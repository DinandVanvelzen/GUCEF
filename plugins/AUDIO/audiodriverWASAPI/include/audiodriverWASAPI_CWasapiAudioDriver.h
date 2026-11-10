/*
 *  audiodriverWASAPI: gucefAUDIO driver plugin for WASAPI on Windows
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

#ifndef GUCEF_AUDIODRIVER_WASAPI_CWASAPIAUDIODRIVER_H
#define GUCEF_AUDIODRIVER_WASAPI_CWASAPIAUDIODRIVER_H

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

#ifndef GUCEF_AUDIODRIVER_WASAPI_MACROS_H
#include "audiodriverWASAPI_macros.h"
#define GUCEF_AUDIODRIVER_WASAPI_MACROS_H
#endif /* GUCEF_AUDIODRIVER_WASAPI_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

// the COM interfaces, so the Windows headers stay out of this header
struct IAudioClient;
struct IAudioRenderClient;

namespace GUCEF {
namespace AUDIODRIVER {
namespace WASAPI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An open WASAPI render endpoint in shared mode with event driven buffering. PushFrames() waits on the event
 *  until the endpoint buffer has room, which paces the device thread that CAudioPushDevice runs. The device thread
 *  joins the COM multithreaded apartment for its life. A buffer found empty when audio is written is an underrun,
 *  an invalidated endpoint (unplugged, disabled, format change) is a lost device.
 */
class CWasapiAudioDevice : public AUDIO::CAudioPushDevice
{
    public:

    CWasapiAudioDevice( const AUDIO::CAudioDeviceInfo& info      ,
                        AUDIO::IAudioDeviceCallback* callback    ,
                        const AUDIO::CAudioFormat& format        ,
                        CORE::UInt32 periodSizeInFrames          ,
                        CORE::UInt32 bufferSizeInFrames          ,
                        IAudioClient* client                     ,
                        IAudioRenderClient* renderClient         ,
                        void* bufferEvent                        );

    virtual ~CWasapiAudioDevice() GUCEF_VIRTUAL_OVERRIDE;

    /** The endpoint buffer: audio written now is heard after about this many frames */
    virtual CORE::UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnDeviceThreadStart( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TPushResult PushFrames( const void* frames, CORE::UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void InterruptPush( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnDeviceThreadEnd( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWasapiAudioDevice( const CWasapiAudioDevice& src );            /**< not implemented */
    CWasapiAudioDevice& operator=( const CWasapiAudioDevice& src ); /**< not implemented */

    private:

    IAudioClient* m_client;
    IAudioRenderClient* m_renderClient;
    void* m_bufferEvent;
    CORE::UInt32 m_bufferSizeInFrames;
    CORE::UInt32 m_bytesPerFrame;
    bool m_comInitializedOnDeviceThread;
    bool m_hasWritten;
};

/*-------------------------------------------------------------------------*/

/**
 *  The "wasapi" driver. Devices are WASAPI endpoint ids, the default render endpoint (console role) when none is
 *  named. The endpoint mixes in its own format; Windows converts from the format asked for (rate, channels,
 *  float or integer samples), so the device gets the format that was asked for.
 *
 *  Options for OpenDevice(): "bufferMs" (default 40) is the endpoint buffer length asked for.
 */
class CWasapiAudioDriver : public AUDIO::CAudioDriver
{
    public:

    CWasapiAudioDriver( void );

    virtual ~CWasapiAudioDriver() GUCEF_VIRTUAL_OVERRIDE;

    /** True when the endpoint services can be reached, which is what the plugin checks before it registers the driver */
    bool IsAvailable( void ) const;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWasapiAudioDriver( const CWasapiAudioDriver& src );            /**< not implemented */
    CWasapiAudioDriver& operator=( const CWasapiAudioDriver& src ); /**< not implemented */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WASAPI */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_WASAPI_CWASAPIAUDIODRIVER_H ? */
