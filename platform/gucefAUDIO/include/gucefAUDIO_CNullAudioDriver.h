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

#ifndef GUCEF_AUDIO_CNULLAUDIODRIVER_H
#define GUCEF_AUDIO_CNULLAUDIODRIVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_CAUDIODRIVER_H
#include "gucefAUDIO_CAudioDriver.h"
#define GUCEF_AUDIO_CAUDIODRIVER_H
#endif /* GUCEF_AUDIO_CAUDIODRIVER_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

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
 *  A device that plays to nowhere and records silence. It gives everything the platform drivers give except sound,
 *  so programs and tests run on machines without audio hardware, and since nothing drives it by itself the audio
 *  is produced exactly when Render() is called: deterministic and as fast as the machine can go.
 *
 *  Options for OpenDevice(): "keepOutput" (default false) keeps what the callback rendered so a test can look at it.
 */
class GUCEF_AUDIO_PUBLIC_CPP CNullAudioDevice : public CAudioDevice
{
    public:

    CNullAudioDevice( const CAudioDeviceInfo& info, IAudioDeviceCallback* callback, const CAudioFormat& format, UInt32 periodSizeInFrames, bool keepOutput );

    virtual ~CNullAudioDevice() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetPeriodSizeInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CNullAudioDevice* AsNullDevice( void ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Plays 'frameCount' frames: asks the callback for audio (or hands it silence to record) as a real device would,
     *  in pieces of the period size. Does nothing when the device is not started. Returns the number of frames made.
     */
    UInt64 Render( UInt64 frameCount );

    /** Everything rendered so far, when 'keepOutput' was set */
    const CAudioBuffer& GetRenderedAudio( void ) const;

    void ClearRenderedAudio( void );

    UInt64 GetTotalFramesRendered( void ) const;

    /** Makes the device report that it is gone, as a device that is unplugged does */
    void SimulateDeviceLoss( void );

    private:

    CAudioFormat m_format;
    UInt32 m_periodSize;
    bool m_keepOutput;
    bool m_started;
    UInt64 m_totalFrames;
    CAudioBuffer m_rendered;
    CAudioBuffer m_scratch;
};

/*-------------------------------------------------------------------------*/

class GUCEF_AUDIO_PUBLIC_CPP CNullAudioDriver : public CAudioDriver
{
    public:

    CNullAudioDriver( void );

    virtual ~CNullAudioDriver() GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CString GetDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual Int32 GetPriority( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool EnumerateDevices( TAudioDirection direction, TAudioDeviceInfoVector& devices ) GUCEF_VIRTUAL_OVERRIDE;

    /** Opens a CNullAudioDevice, so the result can be cast to that type for Render() */
    virtual bool OpenDevice( const CAudioDeviceParams& params, IAudioDeviceCallback* callback, TAudioDevicePtr& device ) GUCEF_VIRTUAL_OVERRIDE;

    /** The format a null device has when nothing is asked: 48000 Hz, stereo, float32 */
    static CAudioFormat GetDefaultFormat( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CNULLAUDIODRIVER_H ? */
