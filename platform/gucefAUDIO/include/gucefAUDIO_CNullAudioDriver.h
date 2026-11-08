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

#ifndef GUCEF_AUDIO_IAUDIOSOURCE_H
#include "gucefAUDIO_IAudioSource.h"
#define GUCEF_AUDIO_IAUDIOSOURCE_H
#endif /* GUCEF_AUDIO_IAUDIOSOURCE_H ? */

#ifndef GUCEF_AUDIO_CLINEARRESAMPLER_H
#include "gucefAUDIO_CLinearResampler.h"
#define GUCEF_AUDIO_CLINEARRESAMPLER_H
#endif /* GUCEF_AUDIO_CLINEARRESAMPLER_H ? */

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
 *
 *  A capture device records silence, or what a source gives once one is set with SetCaptureSource(), so that a test
 *  or a program can feed it the audio a microphone would.
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
     *  Plays 'frameCount' frames: asks the callback for audio (or hands it the recorded audio: silence, or what the
     *  capture source gives) as a real device would, in pieces of the period size. Does nothing when the device is not
     *  started. Returns the number of frames made.
     */
    UInt64 Render( UInt64 frameCount );

    /**
     *  Capture devices only: what Render() records from now on. The source is read on the thread that calls Render(),
     *  converted to the format of the device (sample type, channels and, through a linear resampler, rate); once it
     *  has nothing more the device records silence. An empty pointer goes back to recording silence. Call it when no
     *  Render() runs: everything the conversion needs is made here, Render() does not allocate. False, and the source
     *  that was set before stays, for a playback device or a format that cannot be converted to the device format
     */
    bool SetCaptureSource( const TAudioSourcePtr& source );

    const TAudioSourcePtr& GetCaptureSource( void ) const;

    /** Everything rendered so far, when 'keepOutput' was set */
    const CAudioBuffer& GetRenderedAudio( void ) const;

    void ClearRenderedAudio( void );

    UInt64 GetTotalFramesRendered( void ) const;

    /** Makes the device report that it is gone, as a device that is unplugged does */
    void SimulateDeviceLoss( void );

    private:

    /** Fills the first 'frameCount' frames of m_scratch from the capture source, silence after its end */
    void ReadCaptureSource( UInt32 frameCount );

    CNullAudioDevice( const CNullAudioDevice& src );            /**< not implemented */
    CNullAudioDevice& operator=( const CNullAudioDevice& src ); /**< not implemented */

    private:

    CAudioFormat m_format;
    UInt32 m_periodSize;
    bool m_keepOutput;
    bool m_started;
    UInt64 m_totalFrames;
    CAudioBuffer m_rendered;
    CAudioBuffer m_scratch;

    TAudioSourcePtr m_captureSource;
    IAudioSource* m_captureSourceRaw;           /**< m_captureSource without the shared pointer, for Render() */
    bool m_captureConverts;                     /**< the source format differs from the device format */
    bool m_captureRemixes;                      /**< the channels differ: m_captureMatrix applies */
    bool m_captureResamples;                    /**< the rates differ: m_captureResampler applies */
    CAudioBuffer m_captureRead;                 /**< a period of the source in its own format */
    GUCEF::vector< Float32 > m_captureFloat;    /**< a period of the source as floats in its own channels */
    GUCEF::vector< Float32 > m_capturePending;  /**< source frames as floats in the device channels, not yet delivered */
    UInt32 m_capturePendingOffset;
    UInt32 m_capturePendingFrames;
    GUCEF::vector< Float32 > m_captureOut;      /**< a period in the device format as floats */
    GUCEF::vector< Float32 > m_captureMatrix;
    CLinearResampler m_captureResampler;
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
