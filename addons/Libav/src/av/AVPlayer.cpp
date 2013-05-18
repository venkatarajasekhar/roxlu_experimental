
extern "C" {
#  include <libavutil/avutil.h>
#  include <libavutil/imgutils.h>
}

#include <image/Image.h> // temp
#include <roxlu/Roxlu.h>
#include <roxlu/core/Log.h>
#include <roxlu/core/Utils.h>
#include <av/AVPlayer.h>


// ----------------------------------------------

// ----------------------------------------------

void avplayer_thread(void* user) {

  AVPlayer& player = *static_cast<AVPlayer*>(user);

  bool is_eof = false;
  bool must_stop = false;
  SwsContext* sws = player.sws;
  AVFrame* av_frame = NULL;
  AVPlayerFrame* player_frame = NULL;
  AVDecoderFrame* decoder_frame = NULL;
  std::vector<AVPlayerFrame*> free_frames;
  std::vector<AVPlayerFrame*> work_frames;

  while(true) {
    
    free_frames.clear();

    if(!player.getFreeFrames(free_frames)) {
      rx_sleep_millis(100); // @TODO check if this is ok...
      continue;
    }

    // decode frames (can be audio, video, subtitle etc..)
    work_frames.clear();
    for(std::vector<AVPlayerFrame*>::iterator it = free_frames.begin(); it != free_frames.end(); ++it) {
      player_frame = *it;
      decoder_frame = player.dec.decodeFrame(is_eof);
 
      if(!decoder_frame) {
         break;
      }

      if(is_eof) {
        must_stop = true;
        break;
      }
      
      switch(decoder_frame->type) {

        case AV_TYPE_VIDEO: {
          
          av_frame = decoder_frame->frame;      
          player_frame->is_free = false;
          player_frame->decoder_frame = decoder_frame;
          player_frame->nbytes = player.nbytes_video_frame;
                    
          if(sws) {
            int h = sws_scale(sws, av_frame->data, av_frame->linesize, 0, 
                              av_frame->height, player_frame->pic.data, 
                              player_frame->pic.linesize);

            if(h != av_frame->height) {
              RX_ERROR(ERR_AVP_SWSCALE_FAILED);
              player_frame->is_free = true;
              break;
            }

            player_frame->data = (unsigned char*)player_frame->pic.data[0];
          }
          else {
            player_frame->data = (unsigned char*)av_frame->data[0];
          }

          work_frames.push_back(player_frame);                    
          break;
        } // AV_TYPE_VIDEO

        default: {
          RX_ERROR(ERR_AVP_THREAD_UNHANDLED_TYPE);
          break;
        }
      };

    } // end decode frame loop

    // copy the decoded frames 
    if(work_frames.size()) {
      player.lock();
      std::copy(work_frames.begin(), work_frames.end(), std::back_inserter(player.decoded_frames));
      player.unlock();
    }

    player.lock();
    must_stop = player.must_stop;
    player.unlock();

    if(must_stop) {
      RX_VERBOSE("MUST STOP PLAING!!!!!!");
      break;
    }
    
  }  // end thread loop

  player.freeFrames();
  player.decoded_frames.clear();
  player.must_stop = false;
}

// ----------------------------------------------

AVPlayerFrame::AVPlayerFrame() 
  :decoder_frame(NULL)
  ,data(NULL)
  ,is_free(true)
  ,nbytes(0)
{
}

AVPlayerFrame::~AVPlayerFrame() {
  RX_VERBOSE("CLEAN ALL MEMORY USED BY THE AVPLAYERFRAME");
}


// ----------------------------------------------

AVPlayer::AVPlayer() 
  :state(AVP_STATE_NONE)
  ,num_frames_to_allocate(0)
  ,time_started(0)
  ,nbytes_video_frame(0)
  ,sws(NULL)
  ,must_stop(false)
{
  uv_mutex_init(&mutex);
}

AVPlayer::~AVPlayer() {
  RX_ERROR("DELETE AVDECODER FRAMES - JOIN THREAD");
  state = AVP_STATE_NONE;
  uv_mutex_destroy(&mutex);

  if(sws) {
    sws_freeContext(sws);
    sws = NULL;
  }
}

bool AVPlayer::open(std::string filename, bool datapath, AVPlayerSettings cfg, int numPreAllocateFrames) {
  num_frames_to_allocate = numPreAllocateFrames;
  settings = cfg;

  if(!dec.open(filename, datapath)) {
    return false;
  }

  if(settings.out_pixel_format == AV_PIX_FMT_NONE) {
    settings.out_pixel_format = getPixelFormat();
  }

  nbytes_video_frame = avpicture_get_size(settings.out_pixel_format, getWidth(), getHeight());

  dec.print();

  RX_VERBOSE("Move this initialize call into the thread function OR play(), or create a `initialize()` and `shutdown()` which resets everything to the state the AVPlayer had when just created ");
  if(!initializeSWS()) {
    return false;
  }

  if(!allocateFrames()) {
    return false;
  }

  // @TODO check if we can use YUV as format on MAC else another format which is more suited 
  // on windows + make sure the settings.out_pixel_format matches
  gl_surface.setup(dec.getWidth(), dec.getHeight()); 

  return true;
}

bool AVPlayer::allocateFrames() {
  
  if(needsToConvertPixelFormat()) {
    // allocate frames + a buffer for SWS to put the converted pixels in.    
    if(!sws) {
      RX_ERROR(ERR_AVP_PREALLOC_NO_SWS);
      return false;
    }

    bool err = false;
    for(int i = 0; i < num_frames_to_allocate; ++i) {
      AVPlayerFrame* f = new AVPlayerFrame();
      frames.push_back(f);

      int r = avpicture_alloc(&f->pic, settings.out_pixel_format, getWidth(), getHeight()); 
      if(r < 0) {
        RX_ERROR(ERR_AVP_PREALLOC_PIC);
        err = true;
        break;
      }

      if(err) {
        deleteFrames();
        return false;
      }
    }
  }
  else {

    for(int i = 0; i < num_frames_to_allocate; ++i) {
      AVPlayerFrame* f = new AVPlayerFrame();
      frames.push_back(f);
    }

  }

  return true;
}

bool AVPlayer::play() {

  if(state == AVP_STATE_PLAY) {
    RX_ERROR(ERR_AVP_ALREADY_PLAYING);
    return false;
  }

  state = AVP_STATE_PLAY;

  time_started = millis();

  uv_thread_create(&thread, avplayer_thread, this);
  
  return true;
}

bool AVPlayer::stop() {
  state = AVP_STATE_NONE;
  time_started = 0; // @TODO -> move to a new function `shutdown()`

  lock();
  {
    must_stop = true;
  }
  unlock();

  return true;
}

void AVPlayer::update() {
  if(state != AVP_STATE_PLAY) {
    return ;
  }
}

void AVPlayer::draw(int x, int y, int w, int h) {

  if(state != AVP_STATE_PLAY) {
    return;
  }

  // check if there is a frame in the queue.
  AVPlayerFrame* f = NULL;
  lock();
  if(decoded_frames.size()) {
    f = decoded_frames[0];
  }
  unlock();

  // we got a frame, check if we need to display it
  if(f) { 

    uint64_t time_playing = millis() - time_started;

    if(f->decoder_frame->pts <= time_playing) {

      if(f->decoder_frame->type == AV_TYPE_VIDEO) {
        gl_surface.setPixels((unsigned char*)f->data, f->nbytes);

        lock();
        {
          f->is_free = true;
          decoded_frames.erase(decoded_frames.begin());
        }
        unlock();

      }
      else {
        RX_VERBOSE("We got a frame which is not a video frame... we need to handle this!!");
      }
    }
  }

  if(time_started) {
    gl_surface.draw(x, y, w, h);
  }
}

bool AVPlayer::initializeSWS() {

  if(sws) {
    RX_ERROR(ERR_AVP_ALREADY_SWS);
    return false;
  }

  sws = sws_getContext(getWidth(), getHeight(), getPixelFormat(), 
                       getWidth(), getHeight(), settings.out_pixel_format, 
                       SWS_FAST_BILINEAR, NULL, NULL, NULL);

  if(!sws) {
    RX_ERROR(ERR_AVP_ALLOC_SWS);
    return false;
  }

  return true;
}

bool AVPlayer::needsToConvertPixelFormat() {
  return (settings.out_pixel_format != AV_PIX_FMT_NONE && dec.getPixelFormat() != settings.out_pixel_format);
}

bool AVPlayer::getFreeFrames(std::vector<AVPlayerFrame*>& result) {
  lock();
  for(std::vector<AVPlayerFrame*>::iterator it = frames.begin(); it != frames.end(); ++it) {
    AVPlayerFrame* f = *it;
    if(f->is_free) {
      result.push_back(f);
    }
  }
  unlock();

  return result.size();
}

void AVPlayer::deleteFrames() {
  for(std::vector<AVPlayerFrame*>::iterator it = frames.begin(); it != frames.end(); ++it) {
    delete *it;
  }
  frames.clear();
}

void AVPlayer::freeFrames() {
  lock();
  for(std::vector<AVPlayerFrame*>::iterator it = frames.begin(); it != frames.end(); ++it) {
    AVPlayerFrame* f = *it;
    f->is_free = true;
  }
  unlock();
}


