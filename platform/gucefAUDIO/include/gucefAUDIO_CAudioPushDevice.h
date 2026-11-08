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

#ifndef GUCEF_AUDIO_CAUDIOPUSHDEVICE_H
#define GUCEF_AUDIO_CAUDIOPUSHDEVICE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICE_H
#include "gucefAUDIO_CAudioDevice.h"
#define GUCEF_AUDIO_CAUDIODEVICE_H
#endif /* GUCEF_AUDIO_CAUDIODEVICE_H ? */

#ifndef GUCEF_AUDIO_CAUDIOBUFFER_H
#include "gucefAUDIO_CAudioBuffer.h"
#define GUCEF_AUDIO_CAUDIOBUFFER_H
#endif /* GUCEF_AUDIO_CAUDIOBUFFER_H ? */

#ifndef GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#include "gucefAUDIO_CAudioDeviceParams.h"
#define GUCEF_AUDIO_CAUDIODEVICEPARAMS_H
#endif /* GUCEF_AUDIO_CAUDIODEVICEPARAMS_H ? */

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

class CAudioPushDeviceTask;

/**
 *  Base for devices of backends that take audio with a blocking write or give it with a blocking read (ALSA, OSS,
 *  a file, a socket): the device runs the loop on a dedicated task of a thread pool (DefaultThreadPoolName unless
 *  told otherwise). For playback the loop asks the callback for a period of audio and hands it to the backend, for
 *  capture it reads a period from the backend and hands it to the callback. The thread is therefore one the task
 *  manager knows, and a backend only supplies the write or the read: it never makes a thread of its own.
 *
 *  A backend implements OnPushStart(), PushFrames() and OnPushStop(), and InterruptPush() when its write or read can
 *  wait for longer than a period. A backend that can capture also implements SupportsCapture() and PullFrames(). It
 *  calls Stop() in its own destructor: by the time this destructor runs the backend part of the object is gone.
 *
 *  The device thread can run with real-time scheduling (MT::CRealtimeSchedulingScope) for the duration of the loop:
 *  see SetRealtimePriority() and the device option RealtimePriorityOptionName. Not getting it is not an error, the
 *  device then runs at normal priority and IsRealtimeSchedulingActive() tells the owner, who may report it.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioPushDevice : public CAudioDevice
{
    public:

    /** The pool the device threads run on when nothing else is asked for */
    static const CString DefaultThreadPoolName;

    /**
     *  "realtimePriority": the device option (CAudioDeviceParams::GetOptions()) that asks for real-time scheduling of
     *  the device thread at the given priority, 1..99 on Linux and Android (any value above 0 selects MMCSS on Windows).
     *  0, the default, keeps normal scheduling. Read by ApplyDeviceOptions()
     */
    static const CString RealtimePriorityOptionName;

    enum EPushResult
    {
        PUSHRESULT_OK,                  /**< playback: the frames are with the backend; capture: the frames were read */
        PUSHRESULT_UNDERRUN_RECOVERED,  /**< playback: the backend ran dry and recovered, the frames are with it, a gap was heard; capture: the backend overran (audio was lost) and recovered, the frames were read after the gap */
        PUSHRESULT_DEVICE_LOST          /**< the backend is gone and will not take or give audio again */
    };
    typedef enum EPushResult TPushResult;

    CAudioPushDevice( const CAudioDeviceInfo& info                      ,
                      IAudioDeviceCallback* callback                    ,
                      const CAudioFormat& format                        ,
                      UInt32 periodSizeInFrames                         ,
                      const CString& threadPoolName = DefaultThreadPoolName );

    virtual ~CAudioPushDevice() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Readies the backend and starts the device thread. Capture only when the backend SupportsCapture() */
    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Ends the device thread and stops the backend. When this returns the callback is not running and will not be called again */
    virtual bool Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** False once stopped, and also after the device was lost */
    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetPeriodSizeInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CAudioPushDevice* AsPushDevice( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Playback: times the backend ran dry since the device was made */
    UInt64 GetUnderrunCount( void ) const;

    /**
     *  Capture: times the backend overran (lost audio) and recovered since the device was made. The count goes up
     *  before the callback gets the block that follows the gap, so a callback that reads it in OnCaptureAudio() sees
     *  the new count with exactly that block
     */
    UInt64 GetOverrunCount( void ) const;

    /** Frames handed to the backend (playback) or taken from it (capture) since the device was made */
    UInt64 GetTotalFramesPushed( void ) const;

    const CString& GetThreadPoolName( void ) const;

    /**
     *  Asks for real-time scheduling of the device thread at the given priority from the next Start() on, 0 (the
     *  default) for normal scheduling. The priority is limited to the range of the system
     */
    void SetRealtimePriority( UInt32 priority );

    UInt32 GetRealtimePriority( void ) const;

    /**
     *  Takes the options of the device parameters that every push device understands: RealtimePriorityOptionName.
     *  Backends call it with the parameters the device was opened with. Options that are not there keep their value
     */
    void ApplyDeviceOptions( const CAudioDeviceParams& params );

    /**
     *  True while the device thread runs with real-time scheduling. Known once the callback was called for the first
     *  time after Start(); false when not asked for, not permitted (no right to it) or not supported, and after the loop ended
     */
    bool IsRealtimeSchedulingActive( void ) const;

    /**
     *  The error of the operating system for the last request for real-time scheduling that was refused (EPERM on
     *  Linux without the right to it, see MT::CRealtimeSchedulingScope), 0 when it was granted or not asked for
     */
    Int32 GetRealtimeSchedulingError( void ) const;

    protected:

    /** Readies the backend to take audio. Called on the thread that calls Start(), false makes Start() fail */
    virtual bool OnPushStart( void ) = 0;

    /**
     *  Hands 'frameCount' frames in GetFormat() to the backend, waiting for as long as the backend needs room for
     *  them: this is what paces the device. Called on the device thread only.
     */
    virtual TPushResult PushFrames( const void* frames, UInt32 frameCount ) = 0;

    /** True when the backend can record: Start() then accepts AUDIODIRECTION_CAPTURE. False by default */
    virtual bool SupportsCapture( void ) const;

    /**
     *  Capture: reads exactly 'frameCount' frames in GetFormat() from the backend into 'frames', waiting for as long
     *  as the backend needs to record them: this is what paces a capture device. An overrun that the backend recovered
     *  from is PUSHRESULT_UNDERRUN_RECOVERED with the frames read after it. Called on the device thread only.
     *  PUSHRESULT_DEVICE_LOST by default
     */
    virtual TPushResult PullFrames( void* frames, UInt32 frameCount );

    /**
     *  Called by Stop(), from another thread than the device thread, while PushFrames() or PullFrames() may be waiting:
     *  make it return soon. Nothing by default, the call then returns within a period, which is what most backends do.
     */
    virtual void InterruptPush( void );

    /** Stops the backend once the device thread is done, on the thread that calls Stop(), also after the device was lost */
    virtual void OnPushStop( void ) = 0;

    /**
     *  Called on the device thread before the first PushFrames(), for backends that need per thread set up (COM on
     *  Windows, thread priority, ...). False is treated as a lost device. Nothing by default.
     */
    virtual bool OnDeviceThreadStart( void );

    /** Called on the device thread after the last PushFrames(), to undo OnDeviceThreadStart(). Nothing by default */
    virtual void OnDeviceThreadEnd( void );

    private:

    friend class CAudioPushDeviceTask;

    /** The loop of the device thread */
    void RunPushLoop( void );

    /** The loop of a playback device: render, push */
    void RunPlaybackLoop( IAudioDeviceCallback* callback );

    /** The loop of a capture device: pull, deliver */
    void RunCaptureLoop( IAudioDeviceCallback* callback );

    CAudioPushDevice( const CAudioPushDevice& src );            /**< not implemented */
    CAudioPushDevice& operator=( const CAudioPushDevice& src ); /**< not implemented */

    private:

    typedef CORE::CTSharedPtr< CAudioPushDeviceTask, MT::CMutex > TAudioPushDeviceTaskPtr;

    CAudioFormat m_format;
    UInt32 m_periodSize;
    CString m_threadPoolName;
    CAudioBuffer m_periodBuffer;
    TAudioPushDeviceTaskPtr m_task;
    bool m_started;
    MT::CTAtomic< bool > m_stopRequested;
    MT::CTAtomic< bool > m_loopRunning;
    MT::CTAtomic< UInt64 > m_underruns;
    MT::CTAtomic< UInt64 > m_overruns;
    MT::CTAtomic< UInt64 > m_totalFrames;
    MT::CTAtomic< UInt32 > m_realtimePriority;
    MT::CTAtomic< bool > m_realtimeActive;
    MT::CTAtomic< Int32 > m_realtimeError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CAUDIOPUSHDEVICE_H ? */
