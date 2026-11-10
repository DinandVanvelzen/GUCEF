/*
 *  audiodriverWEBAUDIO: gucefAUDIO driver plugin for the Web Audio API in browsers (Emscripten)
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

#ifndef GUCEF_AUDIODRIVER_WEBAUDIO_CWEBAUDIODRIVER_H
#define GUCEF_AUDIODRIVER_WEBAUDIO_CWEBAUDIODRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#include "gucefAUDIO_CAudioDriver.h"
#define GUCEF_AUDIO_CAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#include "gucefAUDIO_CAudioDevice.h"
#define GUCEF_AUDIO_CAUDIODEVICE_H
#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

#ifndef GUCEF_AUDIODRIVER_WEBAUDIO_MACROS_H
#include "audiodriverWEBAUDIO_macros.h"
#define GUCEF_AUDIODRIVER_WEBAUDIO_MACROS_H
#endif /* GUCEF_AUDIODRIVER_WEBAUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace WEBAUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A Web Audio context that plays what the device callback renders. The browser asks for audio from its main thread
 *  (a script processor node), so this is a callback device: no thread of ours is involved, which also makes it work
 *  in builds without threads. All calls into the page run on the main browser thread, also when made from a worker.
 *
 *  Browsers keep a new audio context suspended until the user interacts with the page; the device resumes it at the
 *  first key press, click or touch after Start(), until then the callback is simply not called.
 */
class CWebAudioDevice : public AUDIO::CAudioDevice
{
    public:

    CWebAudioDevice( const AUDIO::CAudioDeviceInfo& info      ,
                     AUDIO::IAudioDeviceCallback* callback    ,
                     const AUDIO::CAudioFormat& format        ,
                     CORE::UInt32 periodSizeInFrames          ,
                     CORE::Int32 contextId                    );

    virtual ~CWebAudioDevice() GUCEF_VIRTUAL_OVERRIDE;

    virtual const AUDIO::CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::UInt32 GetPeriodSizeInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The script processor buffers a period, the browser about one more */
    virtual CORE::UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Called by the page for each buffer it plays: the frames as interleaved floats, NULL when stopped (silence) */
    const float* Render( CORE::UInt32 frameCount );

    private:

    CWebAudioDevice( const CWebAudioDevice& src );            /**< not implemented */
    CWebAudioDevice& operator=( const CWebAudioDevice& src ); /**< not implemented */

    private:

    AUDIO::CAudioFormat m_format;
    CORE::UInt32 m_periodSize;
    CORE::Int32 m_contextId;
    AUDIO::CAudioBuffer m_renderBuffer;
    MT::CTAtomic< bool > m_started;
};

/*-------------------------------------------------------------------------*/

/**
 *  The "webaudio" driver. A page has one output, so there is one device, "default". The format is float32 at the
 *  rate of the audio context (the one asked for when the browser allows it) with the channels asked for (2 when
 *  none is asked for).
 *
 *  Options for OpenDevice(): "bufferFrames" (default 1024) is the script processor buffer, a power of 2 from 256
 *  to 16384. Smaller is quicker but asks more of the page.
 */
class CWebAudioDriver : public AUDIO::CAudioDriver
{
    public:

    CWebAudioDriver( void );

    virtual ~CWebAudioDriver() GUCEF_VIRTUAL_OVERRIDE;

    /** True when the browser has the Web Audio API */
    static bool IsAvailable( void );

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWebAudioDriver( const CWebAudioDriver& src );            /**< not implemented */
    CWebAudioDriver& operator=( const CWebAudioDriver& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEBAUDIO */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_WEBAUDIO_CWEBAUDIODRIVER_H ? */
