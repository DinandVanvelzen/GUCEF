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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUDIO_CAudioPushDevice.h"

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
 *  The task that is the device thread: one cycle that runs the push loop until the device stops it.
 *  The loop paces itself through the blocking writes of the backend, so it never returns to the delegator
 *  in between periods and the cycle delays of the delegator do not apply.
 */
class CAudioPushDeviceTask : public CORE::CTaskConsumer
{
    public:

    explicit CAudioPushDeviceTask( CAudioPushDevice* device )
        : CORE::CTaskConsumer()
        , m_device( device )
    {GUCEF_TRACE;
    }

    virtual ~CAudioPushDeviceTask() GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return "GUCEF::AUDIO::CAudioPushDeviceTask";
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        static const CORE::CString typeName = "GUCEF::AUDIO::CAudioPushDeviceTask";
        return typeName;
    }

    virtual bool OnTaskCycle( CORE::CTaskPtr ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        // The device is not touched after the loop returns, which is what Stop() waits for
        m_device->RunPushLoop();
        return true;
    }

    virtual void OnTaskEnded( CORE::CTaskPtr, bool ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
    }

    private:

    CAudioPushDevice* m_device;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CAudioPushDevice::DefaultThreadPoolName = "AudioDevice";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAudioPushDevice::CAudioPushDevice( const CAudioDeviceInfo& info     ,
                                    IAudioDeviceCallback* callback   ,
                                    const CAudioFormat& format       ,
                                    UInt32 periodSizeInFrames        ,
                                    const CString& threadPoolName    )
    : CAudioDevice( info, callback )
    , m_format( format )
    , m_periodSize( periodSizeInFrames > 0 ? periodSizeInFrames : 1024 )
    , m_threadPoolName( threadPoolName.IsNULLOrEmpty() ? DefaultThreadPoolName : threadPoolName )
    , m_periodBuffer( format, periodSizeInFrames > 0 ? periodSizeInFrames : 1024 )
    , m_task()
    , m_started( false )
    , m_stopRequested( false )
    , m_loopRunning( false )
    , m_underruns( 0 )
    , m_totalFrames( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAudioPushDevice::~CAudioPushDevice()
{GUCEF_TRACE;

    // The backend had to stop the device in its own destructor, the hooks it implements are gone by now
    GUCEF_ASSERT( !m_started );
    if ( m_started )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AudioPushDevice: destroyed while started, the backend did not call Stop() in its destructor" );
    }
}

/*-------------------------------------------------------------------------*/

const CAudioFormat&
CAudioPushDevice::GetFormat( void ) const
{GUCEF_TRACE;

    return m_format;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPushDevice::Start( void )
{GUCEF_TRACE;

    if ( m_started )
        return true;
    if ( AUDIODIRECTION_PLAYBACK != GetDirection() || GUCEF_NULL == GetCallback() )
        return false;

    if ( !OnPushStart() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioPushDevice: the backend of device \"" << GetDeviceInfo().GetName() << "\" could not be readied" );
        return false;
    }

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_threadPoolName );
    if ( pool.IsNULL() )
    {
        OnPushStop();
        return false;
    }

    m_stopRequested.Store( false, MT::MEMORDER_RELEASE );
    m_loopRunning.Store( true, MT::MEMORDER_RELEASE );
    m_task = TAudioPushDeviceTaskPtr( GUCEF_NEW CAudioPushDeviceTask( this ) );
    CORE::CFutureResult started = pool->StartTaskWithConsumer( m_task );
    if ( started.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AudioPushDevice: no thread for device \"" << GetDeviceInfo().GetName() << "\" on pool " << m_threadPoolName );
        m_loopRunning.Store( false, MT::MEMORDER_RELEASE );
        m_task.Unlink();
        OnPushStop();
        return false;
    }

    m_started = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPushDevice::Stop( void )
{GUCEF_TRACE;

    if ( !m_started )
        return true;

    m_stopRequested.Store( true, MT::MEMORDER_RELEASE );
    InterruptPush();

    // The loop ends after the period in progress. The wait is generous: a backend whose write hangs without end
    // is broken, and returning while the callback may still run would be worse than a long wait
    UInt32 waitedInMs = 0;
    while ( m_loopRunning.Load( MT::MEMORDER_ACQUIRE ) )
    {
        MT::PrecisionDelay( 1 );
        if ( ++waitedInMs == 10000 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AudioPushDevice: the device thread of \"" << GetDeviceInfo().GetName() << "\" does not end, still waiting" );
        }
    }

    OnPushStop();
    m_task.Unlink();
    m_started = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAudioPushDevice::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started && m_loopRunning.Load( MT::MEMORDER_ACQUIRE );
}

/*-------------------------------------------------------------------------*/

CAudioPushDevice*
CAudioPushDevice::AsPushDevice( void )
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

UInt32
CAudioPushDevice::GetPeriodSizeInFrames( void ) const
{GUCEF_TRACE;

    return m_periodSize;
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioPushDevice::GetUnderrunCount( void ) const
{GUCEF_TRACE;

    return m_underruns.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

UInt64
CAudioPushDevice::GetTotalFramesPushed( void ) const
{GUCEF_TRACE;

    return m_totalFrames.Load( MT::MEMORDER_RELAXED );
}

/*-------------------------------------------------------------------------*/

const CString&
CAudioPushDevice::GetThreadPoolName( void ) const
{GUCEF_TRACE;

    return m_threadPoolName;
}

/*-------------------------------------------------------------------------*/

void
CAudioPushDevice::InterruptPush( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAudioPushDevice::OnDeviceThreadStart( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CAudioPushDevice::OnDeviceThreadEnd( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAudioPushDevice::RunPushLoop( void )
{GUCEF_TRACE;

    IAudioDeviceCallback* callback = GetCallback();
    void* frames = m_periodBuffer.GetDataPtr();

    if ( !OnDeviceThreadStart() )
    {
        callback->OnDeviceLost();
        m_loopRunning.Store( false, MT::MEMORDER_RELEASE );
        return;
    }

    while ( !m_stopRequested.Load( MT::MEMORDER_ACQUIRE ) )
    {
        // whatever the callback does not write is heard as it is, so the period starts as silence
        m_periodBuffer.FillSilence();
        callback->OnRenderAudio( frames, m_periodSize );

        const TPushResult result = PushFrames( frames, m_periodSize );
        if ( PUSHRESULT_DEVICE_LOST == result )
        {
            callback->OnDeviceLost();
            break;
        }
        if ( PUSHRESULT_UNDERRUN_RECOVERED == result )
            m_underruns.FetchAdd( 1, MT::MEMORDER_RELAXED );
        m_totalFrames.FetchAdd( m_periodSize, MT::MEMORDER_RELAXED );
    }

    OnDeviceThreadEnd();

    // the last thing the device thread does with the device
    m_loopRunning.Store( false, MT::MEMORDER_RELEASE );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
