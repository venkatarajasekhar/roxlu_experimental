#ifndef ROXLU_CANON_H
#define ROXLU_CANON_H

/*
 
  # Setup build environment (Mac)

  - Download the latestst EDSK from: https://www.didp.canon-europa.com/developer/didp/didp_main.nsf
  - Copy the files `Macintosh/EDSDK/Header/` to `roxlu/extern/include/`
  - Copy the directory `Macintosh/EDSDK/Framework/DPP.framework` to `roxlu/extern/lib/mac/frameworks/`
  - Copy the directory `Macintosh/EDSDK/Framework/EDSDK.framework` to `roxlu/extern/lib/mac/frameworks/`

*/

#include <videocapture/Types.h>
#include <videocapture/edsdk/CanonTypes.h>
#include <videocapture/edsdk/CanonTaskQueue.h>
#include <videocapture/edsdk/CanonTaskProperty.h>
#include <videocapture/edsdk/CanonTaskTakePicture.h>
#include <videocapture/edsdk/CanonTaskDownload.h>
#include <videocapture/edsdk/CanonTaskOpenSession.h>
#include <videocapture/edsdk/CanonTaskCloseSession.h>

#include <EDSDK.h>
#include <EDSDKErrors.h>
#include <EDSDKTypes.h>

#define ERR_CANON_NOT_OPENED "The canon device is not yet opened"

// -----------------------------------------------------------

EdsError EDSCALLBACK canon_handle_object_event(EdsUInt32 event, EdsBaseRef ref, EdsVoid*  user);
EdsError EDSCALLBACK canon_handle_property_event(EdsUInt32 event, EdsUInt32 property, EdsUInt32 param, EdsVoid* user);
EdsError EDSCALLBACK canon_handle_state_event(EdsUInt32 event, EdsUInt32 param, EdsVoid* user);

EdsError EDSCALLBACK canon_download_progress(EdsUInt32 percent, EdsVoid* user, EdsBool* outCancel);                                      /* monitors download progress; set outCancel to true to cancel the download, (EdsVoid*) user, is the task object */

// -----------------------------------------------------------

class Canon {
 public:
  enum State {
    STATE_NONE,
    STATE_OPENED
  };

 public:
  Canon();
  ~Canon();

  // VideoCapture implementation
  int listDevices();
  bool openDevice(int device, VideoCaptureSettings cfg);
  bool closeDevice();
  bool startCapture();
  bool stopCapture();
  void update();

  // Camera control
  bool isLegacy();                                                      /* to support older models - for now we always return false */
  bool lockUI();                                                        /* for cameras earlier than the 30D , the UI must be locked before commands are reissued */
  bool unlockUI();                                                      /* for cameras earlier than the 30D , the UI must be locked before commands are reissued */
  bool openSession();
  bool closeSession();
  bool takePicture();
  bool downloadPicture(EdsDirectoryItemRef item);

  // Properties
  bool getProperty(EdsPropertyID prop);
  bool getCameraName();
  CanonDevice* getDevice();
  EdsCameraRef getCameraRef();

  // State
  Canon::State getState();
  bool initialize();                                                    /* used internally: is called when the session is created and the CanonTaskOpenSession() has been added + the task thread will start soon; */
  bool shutdown();                                                      /* used internally: will be called, when the task queue thread loop will end */

 private:

  State state;
  CanonDevice input_device;
  CanonTaskQueue queue;
  
};

inline bool Canon::isLegacy() {
  return false;
}

inline CanonDevice* Canon::getDevice() {
  return &input_device;
}

inline EdsCameraRef Canon::getCameraRef() {
  return input_device.camera_ref;
}

inline Canon::State Canon::getState() {
  return state;
}
#endif
