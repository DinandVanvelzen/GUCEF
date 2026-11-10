/*
 *  audiodriverALSA: gucefAUDIO driver plugin for ALSA on Linux
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

#ifndef GUCEF_AUDIODRIVER_ALSA_CALSAAUDIODRIVER_H
#define GUCEF_AUDIODRIVER_ALSA_CALSAAUDIODRIVER_H

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

#ifndef GUCEF_AUDIODRIVER_ALSA_CALSAAPI_H
#include "audiodriverALSA_CAlsaApi.h"
#define GUCEF_AUDIODRIVER_ALSA_CALSAAPI_H
#endif /* GUCEF_AUDIODRIVER_ALSA_CALSAAPI_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIODRIVER {
namespace ALSA {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An open ALSA playback device. The writes block until ALSA has room, which paces the device thread that
 *  CAudioPushDevice runs. Underruns and suspends are recovered and counted, any other write error is a lost device.
 */
class CAlsaAudioDevice : public AUDIO::CAudioPushDevice
{
    public:

    CAlsaAudioDevice( const AUDIO::CAudioDeviceInfo& info      ,
                      AUDIO::IAudioDeviceCallback* callback    ,
                      const AUDIO::CAudioFormat& format        ,
                      CORE::UInt32 periodSizeInFrames          ,
                      CORE::UInt32 bufferSizeInFrames          ,
                      snd_pcm_t* pcm                           ,
                      CAlsaApi& api                            );

    virtual ~CAlsaAudioDevice() GUCEF_VIRTUAL_OVERRIDE;

    /** The ALSA buffer: audio written now is heard after about this many frames */
    virtual CORE::UInt32 GetLatencyInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnPushStart( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TPushResult PushFrames( const void* frames, CORE::UInt32 frameCount ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPushStop( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAlsaAudioDevice( const CAlsaAudioDevice& src );            /**< not implemented */
    CAlsaAudioDevice& operator=( const CAlsaAudioDevice& src ); /**< not implemented */

    private:

    snd_pcm_t* m_pcm;
    CAlsaApi& m_api;
    CORE::UInt32 m_bufferSizeInFrames;
    CORE::UInt32 m_bytesPerFrame;
};

/*-------------------------------------------------------------------------*/

/**
 *  The "alsa" driver. Devices are ALSA PCM names ("default", "hw:0,0", "plughw:1,3", ...), "default" when none is named.
 *
 *  Options for OpenDevice(): "periods" (default 4) is the number of periods in the ALSA buffer.
 */
class CAlsaAudioDriver : public AUDIO::CAudioDriver
{
    public:

    explicit CAlsaAudioDriver( CAlsaApi& api );

    virtual ~CAlsaAudioDriver() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool EnumerateDevices( AUDIO::TAudioDirection direction, AUDIO::TAudioDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OpenDevice( const AUDIO::CAudioDeviceParams& params, AUDIO::IAudioDeviceCallback* callback, AUDIO::TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** Agrees on the hardware parameters, as close to what is asked as the device allows */
    bool Negotiate( snd_pcm_t* pcm                         ,
                    const AUDIO::CAudioDeviceParams& params ,
                    AUDIO::CAudioFormat& format             ,
                    CORE::UInt32& periodSizeInFrames        ,
                    CORE::UInt32& bufferSizeInFrames        );

    CAlsaAudioDriver( const CAlsaAudioDriver& src );            /**< not implemented */
    CAlsaAudioDriver& operator=( const CAlsaAudioDriver& src ); /**< not implemented */

    private:

    CAlsaApi& m_api;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ALSA */
}; /* namespace AUDIODRIVER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIODRIVER_ALSA_CALSAAUDIODRIVER_H ? */
