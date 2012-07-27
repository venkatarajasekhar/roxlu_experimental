#include "AV.h"
#include <algorithm>

// TESTING WITH AUDIO
static float t, tincr, tincr2;
static void get_audio_frame(int16_t* samples, int frame_size) {
	int i,j,v;
	int16_t* q;
	q = samples;
	for(j = 0; j < frame_size; ++j) {
		v = (int)(sin(t) * 1000);
		q[j*2] = v;
		q[j*2+1] = v;
		t += tincr;
		tincr += tincr2;
	}
}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

AVContext::AVContext()
	:c(NULL)
	,of(NULL)
	,vs(NULL)
	,vbuf(NULL)
	,vframe(NULL)
	,tmp_vframe(NULL)
	,sws(NULL)
	,vsize(0)
	,as(NULL)
	,abuf(NULL)
	,asize(0)
	,abit_rate(0)
	,asample_rate(0)
	,asample_fmt(AV_SAMPLE_FMT_S16)
	
	// testing
	,atest_samples(NULL)
	,atest_frame_size(0)
	,atest_time(0)
{
}


AV::AV()
	:src_w(0)
	,src_h(0)
	,can_add_frames(false)
	
{
	av_register_all();
	avformat_network_init();
}

AV::~AV() {
	closeVideo(ct);
}



/**
 * @param 	int srcW		Source width (size of input image)
 * @param	int srcH		Source height
 */
bool AV::open(const int srcW, const int srcH) {
	src_w = srcW;
	src_h = srcH;
	
	// setup libav internals.
	if(!setupAV()) {
		printf("Cannot setup libav internals.\n");
		return false;
	}

	// setup SWS we use to convert RGB to YUV
	if(!setupSWS()) {
		printf("Cannot setup sws\n");
		return false;
	}	
	
	can_add_frames = true;
	return true;
}


// add a new video frame
bool AV::addVideoFrame(unsigned char* pixels) {
	if(!can_add_frames) {
		return false;
	}
	
	// Fill and resize our input image to the destination size
	avpicture_fill((AVPicture*)ct.tmp_vframe, pixels, PIX_FMT_RGB24, src_w, src_h);
	int out_h = sws_scale(
		ct.sws
		,(const uint8_t* const*)((AVPicture*)ct.tmp_vframe)->data // when single planar you can just pass like this!
		,ct.tmp_vframe->linesize
		,0
		,src_h
		,ct.vframe->data
		,ct.vframe->linesize
	);


	// create packet to encode
	AVPacket packet;
	av_init_packet(&packet);
	
	if(ct.vs->codec->coded_frame->pts != AV_NOPTS_VALUE) {
	    packet.pts = av_rescale_q(
						 ct.vs->codec->coded_frame->pts
						,ct.vs->codec->time_base
						,ct.vs->time_base
		);
	}

	
	if(ct.vs->codec->coded_frame->key_frame) {
		packet.flags |= AV_PKT_FLAG_KEY;
	}
	
	packet.stream_index = ct.vs->index;
	packet.data = ct.vbuf;
	packet.size = ct.vsize;
	
	// TEST-----------
	packet.pts += t;
	t += tincr;
	// TEST------------

	
	int result = 0;
	int encode_result = avcodec_encode_video2(ct.vs->codec, &packet, ct.vframe, &result);
	if(encode_result != 0) {
		printf("Can't encode the video. result: %d\n", encode_result);
		return false;
	}
	
	printf("Encoded video pts: %lld\n", ct.vs->codec->coded_frame->pts);
	if(av_write_frame(ct.c, &packet) != 0) {
	//if(av_interleaved_write_frame(ct.c, &packet) != 0) {
		printf("Error while writing out a frame.\n");
		return false;
	}
	
	//printf("out_h: %d\n", out_h);
	return true;
}

	/*
	int avcodec_fill_audio_frame	(	
	
	AVFrame * 	frame,
	int 	nb_channels,
	enum AVSampleFormat 	sample_fmt,
	const uint8_t * 	buf,
	int 	buf_size,
	int 	align	 
)	
	*/

bool AV::addTestAudioFrame() {
	AVCodecContext* c = ct.as->codec;
	AVPacket packet = {0}; // data and size must be '0' (allocation is done for you :> )
	AVFrame* frame = avcodec_alloc_frame(); 
	int got_packet = 0;
	
	av_init_packet(&packet);
	get_audio_frame(ct.atest_samples, ct.atest_frame_size);
	
	frame->nb_samples = ct.atest_frame_size;
	
	avcodec_fill_audio_frame(
			frame
			,c->channels
			,c->sample_fmt
			,(uint8_t*)ct.atest_samples
			,ct.atest_frame_size * av_get_bytes_per_sample(c->sample_fmt) * c->channels
			,1
	);
	
	avcodec_encode_audio2(c, &packet, frame, &got_packet);
	printf("%d\n", packet.size);
	if(!got_packet) {
		return false;
	}
	
	packet.stream_index = ct.as->index;

	//if(av_interleaved_write_frame(ct.c, &packet) != 0) {
	if(av_write_frame(ct.c, &packet) != 0) {
		printf("Cannot write frame.\n");
		return false;
	}
	return true;
}


/**
 *
 * buffer is a float array like: 
 * 
 * 	- float buffer[nsamples * nchannels];
 * 
 *
 */
bool AV::addAudioFrame(unsigned char* buffer, int nsamples, int nchannels) {
	
	AVCodecContext* c = ct.as->codec;
	AVPacket packet = {0}; // data and size must be '0' (allocation is done for you :> )
	AVFrame* frame = avcodec_alloc_frame(); 
	int got_packet = 0;
	
	
	// BUFFER HANDLING
	int samples_stored = av_audio_fifo_write(ct.afifo, (void**)&buffer, nsamples);
	if(samples_stored != nsamples) {
		return false;
	}
	int nstored = av_audio_fifo_size(ct.afifo);
	if(nstored < c->frame_size) {
		return false;
	}

	av_init_packet(&packet);
	int use_nsamples = c->frame_size; 
	frame->nb_samples = use_nsamples; // <-- important, must be set  before avcodec_fill_audio_frame
	
	// GET DATA FROM BUFFER
	int num_bytes = av_samples_get_buffer_size(NULL, c->channels, use_nsamples, c->sample_fmt, 0);
	uint8_t* my_buffer = (uint8_t*)av_malloc(num_bytes);
	uint8_t** my_ptr = &my_buffer;
	int nread = av_audio_fifo_read(ct.afifo, (void**)my_ptr, use_nsamples);
	if(nread != use_nsamples) {
		printf("We only read: %d but we wanted to read %d samples.\n", nread, use_nsamples);
		av_free(my_buffer);
		return false;
	}
	
	// FILL
	int fill_result = avcodec_fill_audio_frame(
		 frame
		,c->channels
		,c->sample_fmt
		,(uint8_t*)my_buffer
		,num_bytes
		,1
	);
	
	if(fill_result != 0) {
		char buf[1024];
		av_strerror(fill_result, buf, 1024);
		printf("av error: %s\n",buf);
		av_free(my_buffer);
		return false;
	}

	// ENCODE
	int enc_result = avcodec_encode_audio2(c, &packet, frame, &got_packet);
	packet.stream_index = ct.as->index;
	if(!got_packet) {
		av_free(my_buffer);
		return false;
	}
	if(enc_result < 0) {
		char buf[1024];
		av_strerror(enc_result, buf, 1024);
		printf("av error: %s\n",buf);
	}

	
	// TEST-----------
	//packet.pts += t;
	//t += tincr;
	packet.pts =  ct.vs->codec->coded_frame->pts;
	printf("In audio: %lld\n", ct.vs->codec->coded_frame->pts);
	// TEST------------

	// WRITE
	if(av_write_frame(ct.c, &packet) != 0) {
	//if(av_interleaved_write_frame(ct.c, &packet) != 0) {
		printf("Cannot write audio frame.\n");
		av_free(my_buffer);
		return false;
	}

	av_free(my_buffer);
	return true;
}


bool AV::close() {
	av_write_trailer(ct.c);
	can_add_frames = false; // after saving we can't add any more frames.
}


// Private
// -----------------------------------------------------------------------------

// setup SWS we use for image conversion
bool AV::setupSWS() {
	if(ct.vs == NULL) {
		printf("Call setupSWS after calling setupAV!\n");
		return false;
	}
	
	ct.sws = sws_getContext(
		 src_w						// input w
		,src_h						// input h
		,PIX_FMT_RGB24				// input format
		,ct.vs->codec->width		// output w
		,ct.vs->codec->height		// output h
		,ct.vs->codec->pix_fmt		// output format	
		,SWS_BICUBIC				// scaling method
		,NULL
		,NULL
		,NULL
	);
	
	if(ct.sws == NULL) {
		return false;
	}
	
	return true;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TESING WITH INTERRUPT HANDLING
static int interrupt_cb(void *ctx) 
{ 
    AVFormatContext* formatContext = reinterpret_cast<AVFormatContext*>(ctx);
	printf("interrupt cb.\n");
    return 0;
}
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


// setup libAV related structs.
bool AV::setupAV() {
	// Create output format 
	ct.of = av_guess_format(NULL, "roxlu.flv", NULL);
	if(!ct.of) {
		printf("Cannot create flv AVOutputFormat\n");
		return false;
	}
	
	// Create the main libav context.
	ct.c = avformat_alloc_context();
	if(!ct.c) {
		printf("Cannot allocate the AVFormatContext\n");
		return false;
	}
	
	ct.c->debug = 3;
	
	// Tell our main context what format to use.
	ct.c->oformat = ct.of;
	
	// Set the output type for our main context.
	const char* output_filename = "tcp://127.0.0.1:6666";
//	const char* output_filename = "test.flv";
	snprintf(ct.c->filename, sizeof(ct.c->filename), "%s", output_filename);
	

	
	// add the video stream.
	ct.vs = addVideoStream(ct, ct.of->video_codec);
	if(!ct.vs) {
		printf("Cannot create video stream: %d.\n", ct.of->video_codec);
		return false;
	}
	
	// open the video stream and init encoder
	if(!openVideo(ct)) {
		printf("Cannot open video stream.\n");
		return false;
	}
	
	// set the metadata for the stream: @todo make this a variable
	av_dict_set(&ct.c->metadata, "streamName", "video_test", 0);
	
	// add audio stream
	bool use_mp3 = true;
	if(!use_mp3) {
		ct.asample_fmt = AV_SAMPLE_FMT_S16;
		ct.abit_rate = 64000;
		ct.asample_rate = 8000;
		ct.as = addAudioStream(ct, CODEC_ID_SPEEX);
	}
	else {
		ct.asample_fmt = AV_SAMPLE_FMT_S16;
		ct.abit_rate = 5000;
		ct.asample_rate = 44100;
		ct.as = addAudioStream(ct, CODEC_ID_MP3);
	}
	
	if(!ct.as) {
		printf("Cannot create audio stream.\n");
		return false;
	}
	
	if(!openAudio(ct)) {
		printf("Cannot open audio stream.\n");
		return false;
	}
	
	// tmp testing with debug
	// ++++++++++++++++++++++++++
		ct.c->interrupt_callback.callback = interrupt_cb;
		ct.c->interrupt_callback.opaque = ct.c;
		//AVFormatContext* formatContext = libffmpeg::avformat_alloc_context( );
		//formatContext->interrupt_callback.callback = interrupt_cb;
		//formatContext->interrupt_callback.opaque = formatContext;
	// ++++++++++++++++++++++++++++
	// end temp testing with debug
	
	
	// show some info
	av_dump_format(ct.c, 0, output_filename, 1);
	
	// open the output file if needed
	if(!(ct.of->flags & AVFMT_NOFILE)) {
		if(avio_open(&ct.c->pb, output_filename, AVIO_FLAG_WRITE) < 0) {
			printf("Cannot open: %s\n", output_filename);
			return false;
		}
	}
	
	// write header.
	avformat_write_header(ct.c, NULL);
	
	return true;
}

/******************************************************************************/
/**  								A U D I O 								  */
/******************************************************************************/
AVStream* AV::addAudioStream(AVContext& context, enum CodecID codecID) {
	AVCodec* codec = NULL;
	AVStream* st = NULL;
	AVCodecContext* c = NULL;
	
	codec = avcodec_find_encoder(codecID);
	if(!codec) {
		printf("Cannot find the audio codec with ID: %d\n", codecID);
		return NULL;
	}
	
	st = avformat_new_stream(ct.c, codec); // we create a stream, so we don't have to allocate the codeccontext
	if(!st) {
		printf("Cannot create stream for audio.\n");
		return NULL;
	}
	st->codec->debug = 3; /// @todo debug
	c = st->codec;
	c->sample_fmt = context.asample_fmt; // @todo make a setter
	c->bit_rate = context.abit_rate;
	c->sample_rate = context.asample_rate;
	//c->bit_rate = 16000;
	//c->sample_rate = 8000;
	c->channels = 2;
	c->bit_rate_tolerance = 1; // http://libav-users.943685.n4.nabble.com/avcodec-open-fail-in-some-system-td3320470.html
	c->channel_layout = 0; // http://libav-users.943685.n4.nabble.com/avcodec-open-fail-in-some-system-td3320470.html  

	if(c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) {
		printf("Variable frame size! not implemented yet\n");
		::exit(0);
	}
	
	if(context.c->oformat->flags & AVFMT_GLOBALHEADER) {
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}

	// Experimental: http://libav-users.943685.n4.nabble.com/Libav-user-FFmpeg-x264-settings-for-zero-delay-encoding-td4655209.html
	av_opt_set(c->priv_data, "preset", "ultrafast", 0);
	av_opt_set(c->priv_data, "tune", "zerolatency", 0);
	//> av_opt_set(codecContex->priv_data, "preset", "ultrafast", 0); 
	//> av_opt_set(codecContex->priv_data, "tune", "zerolatency", 0); 
	

	return st;
}

bool AV::openAudio(AVContext& context) {
	AVCodecContext* c = NULL;
	c = context.as->codec;
	
	if(avcodec_open2(c, NULL, NULL) < 0) {
		printf("Cannot open audio codec.\n");
		return false;
	}
	

	printf("Creating a buffer: %d\n", c->frame_size);
	context.afifo = av_audio_fifo_alloc(c->sample_fmt, c->channels, c->frame_size); // c->frame_size is only set after avcodec_open2 is called
	if(!context.afifo) {
		printf("Cannot create audio buffer.\n");
		return false;
	}

	
	// create test samples
	// +++++++++++++++++++++++++++++++
	t = 0;
	tincr = 2 * M_PI * 710.0 / c->sample_rate;
	tincr2 = 2 * M_PI * 710.0 / c->sample_rate / c->sample_rate;
	tincr = 0.001;
	if(c->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) {
		printf("Audio codec has variable frame size.\n");
		context.atest_frame_size = 1000;
	}
	else {
		context.atest_frame_size = c->frame_size;
	}
	printf("Audio frame_size = %d\n", context.atest_frame_size);
	printf("Audio sizeof samples: %d\n", context.atest_frame_size * av_get_bytes_per_sample(c->sample_fmt) * c->channels); 

	context.atest_samples = (int16_t*) av_malloc(
		context.atest_frame_size 
		* av_get_bytes_per_sample(c->sample_fmt) 
		* c->channels
	);
	// +++++++++++++++++++++++++++++++	
	
	//int asize =  c->frame_size * av_get_bytes_per_sample(c->sample_fmt) * c->channels; ;
	/*
	int aalloc = 0;
	int asize = c->frame_size;
	while(aalloc < asize) {
		aalloc = std::max<int>(2 * aalloc, 1024);
		printf("A: %d\n", aalloc); 
	}
	ct.asize = aalloc;
	ct.abuf = (int16_t*)av_malloc(ct.asize);
	*/

	
		
	return true;
}



/******************************************************************************/
/**  								V I D E O 								  */
/******************************************************************************/

AVStream* AV::addVideoStream(AVContext& context, enum CodecID codecID) {
	AVCodec* codec = NULL;
	AVStream* stream = NULL;
	
	codec = avcodec_find_encoder(codecID);
	if(!codec) {
		printf("Cannot find codec with ID: %d\n", codecID);
		return NULL;
	}
	
	stream = avformat_new_stream(context.c, codec);
	if(!stream) {
		printf("Cannot create stream.\n");
		return NULL;
	}
	
	// sample params
	stream->codec->bit_rate = 400000;
	stream->codec->width = src_w; //  @todo change this to output width
	stream->codec->height = src_h; //  @todo change this to output height
	stream->codec->time_base.den = 30; 
	stream->codec->time_base.num = 1;
	stream->codec->gop_size = 30; // still necessary (?)
	stream->codec->codec_id = context.of->video_codec;
	stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;
	stream->codec->pix_fmt = PIX_FMT_YUV420P;
	stream->codec->max_b_frames = 0; //  @todo testing.
	
	//if(stream->codec->codec_id == CODEC_ID_MPEG2VIDEO
	if(codecID == CODEC_ID_MPEG2VIDEO) {
		stream->codec->max_b_frames = 2;
	}
	
	if(codecID == CODEC_ID_MPEG1VIDEO) {
		stream->codec->mb_decision = 2;
	}
	
	// separate stream headers? for output muxer
	if(context.c->oformat->flags & AVFMT_GLOBALHEADER) {
		stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	
	return stream;
}

bool AV::openVideo(AVContext& context) {
	// open the encoder.
	if(avcodec_open2(context.vs->codec, NULL, NULL) < 0) {
		printf("Cannot open the encoder.\n");
		return false;
	}
	
	context.vbuf = NULL;
	//if(!(context.c->oformat->flags & AVFMT_RAWPICTURE)) {
	//	printf("Allocating video buffer when it's not a raw picture (?)\n");
		context.vsize = 200000;
		context.vbuf = (uint8_t*) av_malloc(context.vsize);
	//}
	
	// allocate encoded raw image.
	context.vframe = allocFrame(context.vs->codec->pix_fmt, context.vs->codec->width, context.vs->codec->height); // we could use "w" and "h", this seems more versatile
	if(!context.vframe) {
		printf("Cannot allocate memory for vframe.\n");
		return false;
	}
	
	// allocate RGB image (used for conversion)
	//  context.vs->codec->width, context.vs->codec->height); // we could use "w" and "h", this seems more versatile
	context.tmp_vframe = allocFrame(PIX_FMT_RGB24, src_w, src_h);
	if(!context.tmp_vframe) {
		printf("Cannot allocate memory for tmp_vframe we need for conversion.\n");
		return false;
	}	
	return true;
}

void AV::closeVideo(AVContext& context) {
	avcodec_close(context.vs->codec);
	av_free(context.vframe->data[0]);
	av_free(context.tmp_vframe->data[0]);
	av_free(context.vbuf);
}


AVFrame* AV::allocFrame(enum PixelFormat pixFormat, int width, int height) {
	AVFrame* pic = NULL;
	uint8_t* buf = NULL;
	int size = 0;
	
	pic = avcodec_alloc_frame();
	if(!pic) {
		printf("Cannot allocate frame.\n");
		return NULL;
	}
	
	// @todo figure out who should maintain ownership of "buf". I think the AVContext
	// @BBB-work on #libav-devel says "yes libav frees it as you close the decoder"
	size = avpicture_get_size(pixFormat, width, height);
	buf = (uint8_t*) av_malloc(size); // memory leak here? who owns this?
	
	avpicture_fill((AVPicture*)pic, buf, pixFormat, width, height);
	
	return pic;
}


