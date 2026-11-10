/*
 *  audiodriverAAUDIO: gucefAUDIO driver plugin for AAudio on Android
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

#ifndef GUCEF_AUDIODRIVER_AAUDIO_CAAUDIODRIVER_H
#define GUCEF_AUDIODRIVER_AAUDIO_CAAUDIODRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#include "gucefAUDIO_CAudioDriver.h"
#define GUCEF_AUDIO_CAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#include "gucefAUDIO_CAudioPushDevice.h"
#define GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#endif /* GUCEF_AUDIO_CAUDIOPUSHDEVICE_H ? */

#ifndef GUCEF_AUDIODRIVER_AAUDIO_CAAUDIOAPI_H
#include "audiodriverAAUDIO_CAAudioApi.h"
#define GUCEF_AUDIODRIVER_AAUDIO_CAAUDIOAPI_H
#endif /* GUCEF_AUDIODRIVER_AAUDIO_CAAUDIOAPI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace AAUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An open AAudio output stream. AAudio is used in its blocking write mode: the writes wait until the stream has
 *  room, which paces the device thread that CAudioPushDevice runs, so no AAudio callback thread is involved.
 *  Underruns are counted from the stream's xrun count, a disconnected stream (headphones pulled, route change) is a
 *  lost device.
 */
class CAAudioDevice : public AUDIO::CAudioPushDevice
{
    public:

    CAAudioDevice( const AUDIO::CAudioDeviceInfo& info      ,
                   AUDIO::IAudioDeviceCallback* callback    ,
                   const AUDIO::CAudioFormat& format        ,
                   CORE::UInt32 periodSizeInFrames          ,
                   CORE::UInt32 bufferSizeInFrames          ,
                   AAudioStream* stream                     ,
                   CAAudioApi& api                          );

    virtual ~CAAudioDevice() GUCEF_VIRTUAL_OVERRIDE;

    /** The part of the AAudio buffer in use: audio written now is heard after about this many frames */
    virtual CORE::UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TPushResult PushFrames( const void* frames, CORE::UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAAudioDevice( const CAAudioDevice& src );            /**< not implemented */
    CAAudioDevice& operator=( const CAAudioDevice& src ); /**< not implemented */

    private:

    AAudioStream* m_stream;
    CAAudioApi& m_api;
    CORE::UInt32 m_bufferSizeInFrames;
    CORE::UInt32 m_bytesPerFrame;
    CORE::Int32 m_xruns;
};

/*-------------------------------------------------------------------------*/

/**
 *  The "aaudio" driver. AAudio does not list devices (that takes the Java AudioManager), so there is one device,
 *  "default", which follows the route Android picks. A numeric device id from AudioDeviceInfo.getId() is accepted
 *  as well.
 *
 *  Options for OpenDevice(): "bursts" (default 2) is the number of bursts AAudio buffers, more is safer, fewer is
 *  quicker. "lowLatency" (default true) asks for the low latency path.
 */
class CAAudioDriver : public AUDIO::CAudioDriver
{
    public:

    explicit CAAudioDriver( CAAudioApi& api );

    virtual ~CAAudioDriver() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAAudioDriver( const CAAudioDriver& src );            /**< not implemented */
    CAAudioDriver& operator=( const CAAudioDriver& src ); /**< not implemented */

    private:

    CAAudioApi& m_api;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_AAUDIO_CAAUDIODRIVER_H ? */
