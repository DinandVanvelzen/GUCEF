/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef GUCEF_AUDIO_IAUDIODEVICECALLBACK_H
#define GUCEF_AUDIO_IAUDIODEVICECALLBACK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIOFORMAT_H
#include "gucefAUDIO_CAudioFormat.h"
#define GUCEF_AUDIO_CAUDIOFORMAT_H
#endif /* GUCEF_AUDIO_CAUDIOFORMAT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  How a device talks to what uses it. The device calls this whenever it needs audio to play (or has audio that
 *  was recorded). The calls come from the thread that the driver runs the device on, which is a real time thread
 *  on most platforms: the implementation must be quick, never wait for a lock that someone may hold for long,
 *  never allocate, never do IO and never log.
 */
class GUCEF_AUDIO_PUBLIC_CPP IAudioDeviceCallback
{
    public:

    virtual ~IAudioDeviceCallback() {}

    /**
     *  Playback: fill 'buffer' with exactly 'frameCount' frames in the format of the device.
     *  Whatever is not written is heard as it is, so write silence when there is nothing to play.
     */
    virtual void OnRenderAudio( void* buffer, UInt32 frameCount ) = 0;

    /** Capture: the device has recorded 'frameCount' frames in its format. The buffer is only valid during the call */
    virtual void OnCaptureAudio( const void* buffer, UInt32 frameCount ) = 0;

    /** The device is gone (unplugged, driver reset) and will not call again. May come from any thread, only note it, do not act on it here */
    virtual void OnDeviceLost( void ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_IAUDIODEVICECALLBACK_H ? */
