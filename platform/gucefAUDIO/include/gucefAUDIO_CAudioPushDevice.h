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
 *  Base for playback devices of backends that take audio with a blocking write (ALSA, OSS, a file, a socket):
 *  the device runs the loop that asks the callback for a period of audio and hands it to the backend, on a
 *  dedicated task of a thread pool (DefaultThreadPoolName unless told otherwise). The thread is therefore one the
 *  task manager knows, and a backend only supplies the write: it never makes a thread of its own.
 *
 *  A backend implements OnPushStart(), PushFrames() and OnPushStop(), and InterruptPush() when its write can wait
 *  for longer than a period. It calls Stop() in its own destructor: by the time this destructor runs the backend
 *  part of the object is gone.
 */
class GUCEF_AUDIO_PUBLIC_CPP CAudioPushDevice : public CAudioDevice
{
    public:

    /** The pool the device threads run on when nothing else is asked for */
    static const CString DefaultThreadPoolName;

    enum EPushResult
    {
        PUSHRESULT_OK,                  /**< the frames are with the backend */
        PUSHRESULT_UNDERRUN_RECOVERED,  /**< the backend ran dry and recovered, the frames are with it, a gap was heard */
        PUSHRESULT_DEVICE_LOST          /**< the backend is gone and will not take audio again */
    };
    typedef enum EPushResult TPushResult;

    CAudioPushDevice( const CAudioDeviceInfo& info                      ,
                      IAudioDeviceCallback* callback                    ,
                      const CAudioFormat& format                        ,
                      UInt32 periodSizeInFrames                         ,
                      const CString& threadPoolName = DefaultThreadPoolName );

    virtual ~CAudioPushDevice() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CAudioFormat& GetFormat( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Readies the backend and starts the device thread. Playback only */
    virtual bool Start( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Ends the device thread and stops the backend. When this returns the callback is not running and will not be called again */
    virtual bool Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** False once stopped, and also after the device was lost */
    virtual bool IsStarted( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetPeriodSizeInFrames( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CAudioPushDevice* AsPushDevice( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Times the backend ran dry since the device was made */
    UInt64 GetUnderrunCount( void ) const;

    /** Frames handed to the backend since the device was made */
    UInt64 GetTotalFramesPushed( void ) const;

    const CString& GetThreadPoolName( void ) const;

    protected:

    /** Readies the backend to take audio. Called on the thread that calls Start(), false makes Start() fail */
    virtual bool OnPushStart( void ) = 0;

    /**
     *  Hands 'frameCount' frames in GetFormat() to the backend, waiting for as long as the backend needs room for
     *  them: this is what paces the device. Called on the device thread only.
     */
    virtual TPushResult PushFrames( const void* frames, UInt32 frameCount ) = 0;

    /**
     *  Called by Stop(), from another thread than the device thread, while PushFrames() may be waiting: make it
     *  return soon. Nothing by default, PushFrames() then returns within a period, which is what most backends do.
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
    MT::CTAtomic< UInt64 > m_totalFrames;
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
