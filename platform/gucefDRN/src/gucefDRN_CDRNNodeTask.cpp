/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#include "gucefDRN_CDRNNodeTask.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNNodeTask::TaskType = "GUCEF::DRN::CDRNNodeTask";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNNodeTask::CDRNNodeTask( CDRNNode& node )
    : CORE::CTaskConsumer()
    , CORE::CIPulseGeneratorDriver()
    , m_node( &node )
    , m_pulseGenerator()
    , m_previousDriver( GUCEF_NULL )
    , m_startState( STARTSTATE_PENDING )
    , m_startStateChanged( 0 )
    , m_immediatePulseRequested( 0 )
    , m_driverTaskId( 0 )
    GUCEF_SYSTEM_THREAD_ID_MEMBER_INIT( m_driverTaskSystemThreadId, 0 )
    , m_readinessWaiter()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNodeTask::~CDRNNodeTask()
{GUCEF_TRACE;

    ReleasePulseGenerator();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNNodeTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNNodeTask::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNNodeTask";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::SetStartState( UInt32 state )
{GUCEF_TRACE;

    m_startState.Store( state );
    m_startStateChanged.Post();
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeTask::WaitForStartState( UInt32 state, UInt32 timeoutInMs )
{GUCEF_TRACE;

    // every change posts once; a post a waiter before did not take only makes the state be looked at again
    UInt64 deadline = CDRNClock::NowInMicroseconds() + (UInt64) timeoutInMs * 1000;
    while ( m_startState.Load() < state )
    {
        UInt64 now = CDRNClock::NowInMicroseconds();
        if ( now >= deadline )
            break;
        UInt64 leftMs = ( deadline - now + 999 ) / 1000;
        m_startStateChanged.WaitOrTimeout( (UInt32) leftMs );
    }
    return m_startState.Load() >= state;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeTask::WaitUntilStarted( UInt32 timeoutInMs )
{GUCEF_TRACE;

    WaitForStartState( STARTSTATE_WORKING, timeoutInMs );
    return STARTSTATE_WORKING == m_startState.Load();
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeTask::WaitUntilEnded( UInt32 timeoutInMs )
{GUCEF_TRACE;

    return WaitForStartState( STARTSTATE_ENDED, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeTask::IsWorking( void ) const
{GUCEF_TRACE;

    return STARTSTATE_WORKING == m_startState.Load();
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeTask::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( GetTaskDelegator().IsNULL() || !CORE::CTaskConsumer::OnTaskStart( task ) )
        return false;

    // the node keeps working on its own pulse generator: this task drives it while it runs, every pulse under the node
    // lock (CDRNNode::RunThreadCycle()). The delegator's pulse generator, which the delegator pulses itself outside the
    // cycles of this task, does not reach the objects of the node
    m_pulseGenerator = m_node->GetWorkPulseGenerator();
    if ( m_pulseGenerator.IsNULL() )
        return false;
    m_previousDriver = m_pulseGenerator->GetPulseGeneratorDriver();
    MT::SThreadIds ownerIds;
    MT::GetCurrentThreadIds( &ownerIds );
    m_driverTaskId.Store( ownerIds.threadId );
    GUCEF_SYSTEM_THREAD_ID_CODE( m_driverTaskSystemThreadId.Store( ownerIds.systemThreadId ); )
    m_pulseGenerator->SetPulseGeneratorDriver( this );
    m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );

    // this driver ends the node's wait on a pulse request from another thread (RequestImmediatePulse()): the sockets
    // of the node's channels wake the node when they become ready instead of waiting for its next cycle
    m_readinessWaiter = COMCORE::CComCoreGlobal::Instance()->GetSocketReadinessWaiter();
    if ( !m_readinessWaiter.IsNULL() )
        m_readinessWaiter->EnableForPulseGenerator( m_pulseGenerator );

    m_node->OnOwnThreadStarted();

    SetStartState( STARTSTATE_WORKING );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::OnTaskStartupFailed( CORE::CTaskPtr task )
{GUCEF_TRACE;

    ReleasePulseGenerator();
    SetStartState( STARTSTATE_ENDED );
    CORE::CTaskConsumer::OnTaskStartupFailed( task );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeTask::OnTaskCycle( CORE::CTaskPtr /* task */ )
{GUCEF_TRACE;

    // The task keeps the cycle itself: the delegator waits at least its minimal cycle delta between cycles, while a
    // node works about once per millisecond, at once when a busy socket asks for it and when another thread left work
    // for it (CDRNNode::WakeOwnThread()). The loop ends when the task is asked to stop, which the delegator then sees
    // as well.
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    if ( 0 == ticksPerMs )
        ticksPerMs = 1;
    while ( !IsDeactivationRequested() )
    {
        UInt64 cycleStart = MT::PrecisionTickCount();
        m_immediatePulseRequested.Store( 0 );
        m_node->RunThreadCycle();

        if ( 0 != m_immediatePulseRequested.Load() )
            continue;
        if ( ( MT::PrecisionTickCount() - cycleStart ) / ticksPerMs < 1 )
            m_node->WaitForThreadWork( 1 );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::OnTaskEnding( CORE::CTaskPtr task, bool willBeForced )
{GUCEF_TRACE;

    CORE::CTaskConsumer::OnTaskEnding( task, willBeForced );
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    // the task's own thread calls this after its last cycle: from then on it does not touch the node. Another thread
    // calls it when it gave up waiting for the task's thread to end (CActiveObject::Deactivate), while that thread may
    // still be in a cycle: that is not the end of the work for the node
    const TThreadId driverTaskId = m_driverTaskId.Load();
    if ( 0 == driverTaskId || MT::GetCurrentThreadID() == driverTaskId )
    {
        ReleasePulseGenerator();
        SetStartState( STARTSTATE_ENDED );
    }
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::ReleasePulseGenerator( void )
{GUCEF_TRACE;

    // hands the pulse generator back to the delegator that drives it when no task of ours works on it
    if ( !m_pulseGenerator.IsNULL() )
    {
        if ( !m_readinessWaiter.IsNULL() )
            m_readinessWaiter->DisableForPulseGenerator( m_pulseGenerator );

        if ( this == m_pulseGenerator->GetPulseGeneratorDriver() )
            m_pulseGenerator->SetPulseGeneratorDriver( m_previousDriver );
        m_pulseGenerator.Unlink();
    }
    m_previousDriver = GUCEF_NULL;
    m_readinessWaiter.Unlink();
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::RequestImmediatePulse( CORE::CPulseGenerator& /* pulseGenerator */ )
{GUCEF_TRACE;

    m_immediatePulseRequested.Store( 1 );

    // a request of another thread (the socket readiness waiter, a sender) ends the node's wait between its cycles;
    // the node's own thread looks at the flag before it waits
    if ( MT::GetCurrentThreadID() != m_driverTaskId.Load() )
        m_node->WakeOwnThread();
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::RequestPulsesPerImmediatePulseRequest( CORE::CPulseGenerator& /* pulseGenerator */             ,
                                                     const Int32 /* requestedPulsesPerImmediatePulseRequest */ )
{GUCEF_TRACE;

    // every cycle is a pulse already
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::RequestPeriodicPulses( CORE::CPulseGenerator& /* pulseGenerator */ ,
                                     const UInt32 /* pulseDeltaInMilliSecs */    )
{GUCEF_TRACE;

    // the task pulses about once per millisecond, which covers any interval asked for
}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::RequestPulseInterval( CORE::CPulseGenerator& /* pulseGenerator */ ,
                                    const UInt32 /* pulseDeltaInMilliSecs */    )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNNodeTask::RequestStopOfPeriodicUpdates( CORE::CPulseGenerator& /* pulseGenerator */ )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

TThreadId
CDRNNodeTask::GetPulseDriverThreadId( void ) const
{GUCEF_TRACE;

    // 0 until the task's thread started: the delegator's own thread id (GetDelegatorThreadId()) is the operating
    // system's id of the thread, another kind of number than the thread identity users of the pulse generator compare with
    return m_driverTaskId.Load();
}
/*-------------------------------------------------------------------------*/

TSystemThreadId
CDRNNodeTask::GetPulseDriverSystemThreadId( void ) const
{GUCEF_TRACE;

    return GUCEF_SYSTEM_THREAD_ID_OR_THREAD_ID( m_driverTaskSystemThreadId.Load(), m_driverTaskId.Load() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
