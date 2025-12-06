/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTASKDELEGATOR_H
#include "gucefCORE_CTaskDelegator.h"
#define GUCEF_CORE_CTASKDELEGATOR_H
#endif /* GUCEF_CORE_CTASKDELEGATOR_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefCORE_CTaskConsumer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CTaskConsumer::TaskKilledEvent = "GUCEF::CORE::CTaskConsumer::TaskKilledEvent";
const CEvent CTaskConsumer::TaskStartupEvent = "GUCEF::CORE::CTaskConsumer::TaskStartupEvent";
const CEvent CTaskConsumer::TaskStartedEvent = "GUCEF::CORE::CTaskConsumer::TaskStartedEvent";
const CEvent CTaskConsumer::TaskStartupFailedEvent = "GUCEF::CORE::CTaskConsumer::TaskStartupFailedEvent";
const CEvent CTaskConsumer::TaskPausedEvent = "GUCEF::CORE::CTaskConsumer::TaskPausedEvent";
const CEvent CTaskConsumer::TaskResumedEvent = "GUCEF::CORE::CTaskConsumer::TaskResumedEvent";
const CEvent CTaskConsumer::TaskFinishedEvent = "GUCEF::CORE::CTaskConsumer::TaskFinishedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CTaskConsumer::RegisterEvents( void )
{GUCEF_TRACE;

    TaskKilledEvent.Initialize();
    TaskStartupEvent.Initialize();
    TaskStartedEvent.Initialize();
    TaskStartupFailedEvent.Initialize();
    TaskPausedEvent.Initialize();
    TaskResumedEvent.Initialize();
    TaskFinishedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTaskConsumer::CTaskConsumer( void )
    : CTSGNotifier( PulseGeneratorPtr(), true, false )
    , m_currentTask()
    , m_threadPool()
    , m_delegator()
    , m_ownedByThreadPool( false )
    , m_inPhasedSetup( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTaskConsumer::~CTaskConsumer()
{GUCEF_TRACE;

    try
    {
        SignalUpcomingDestruction();    
    }
    // we should not get exceptions here, the below is mainly for defensive coding as a lesser evil
    catch ( const timeout_exception& )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "TaskConsumer:Destructor: encountered timeout exception" );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( LOGLEVEL_NORMAL, "TaskConsumer:Destructor: encountered std exception " + CString( e.what() ) );
    }
}

/*-------------------------------------------------------------------------*/

void 
CTaskConsumer::SetIsInPhasedSetup( bool inPhasedSetup )
{GUCEF_TRACE;

    if ( inPhasedSetup )
    {
        CTaskPtr currentTask = m_currentTask;
        if ( !currentTask.IsNULL() )
        {
            currentTask->SetTaskStatus( TTaskStatus::TASKSTATUS_SETUP );
        }
    }
    m_inPhasedSetup = inPhasedSetup;    
}

/*-------------------------------------------------------------------------*/

bool 
CTaskConsumer::GetIsInPhasedSetup( void ) const
{GUCEF_TRACE;

    return m_inPhasedSetup;
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::SetIsOwnedByThreadPool( bool ownedByThreadPool )
{GUCEF_TRACE;

    m_ownedByThreadPool = ownedByThreadPool;
}

/*-------------------------------------------------------------------------*/

bool
CTaskConsumer::IsOwnedByThreadPool( void ) const
{GUCEF_TRACE;

    return m_ownedByThreadPool;
}

/*-------------------------------------------------------------------------*/

CTaskConsumer::TTaskDelegatorBasicPtr
CTaskConsumer::GetTaskDelegator( void )
{GUCEF_TRACE;

    return m_delegator;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskConsumer::GetDelegatorThreadId( void ) const
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    return !delegator.IsNULL() ? delegator->GetThreadID() : 0;
}

/*-------------------------------------------------------------------------*/

void 
CTaskConsumer::RequestTaskCycleDelayInMs( UInt32 requestedDelayInMs )
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    if ( !delegator.IsNULL() )
        delegator->RequestTaskCycleDelayInMs( requestedDelayInMs );
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::SetCurrentTask( CTaskPtr newTask )
{GUCEF_TRACE;

    m_currentTask = newTask;
}

/*-------------------------------------------------------------------------*/

CTaskPtr
CTaskConsumer::GetCurrentTask( void ) const
{GUCEF_TRACE;

    return m_currentTask;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskConsumer::GetCurrentTaskId( void ) const
{GUCEF_TRACE;

    CTaskPtr task = m_currentTask;
    if ( !task.IsNULL() )
    {
        return task->GetTaskId();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CTaskConsumer::WaitForTaskToFinish( Int32 timeoutInMs )
{GUCEF_TRACE;

    ThreadPoolPtr threadPool = m_threadPool;
    return !threadPool.IsNULL() ? threadPool->WaitForTaskToFinish( GetCurrentTaskId(), timeoutInMs ) : false;
}

/*-------------------------------------------------------------------------*/

const CString&
CTaskConsumer::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString typeName = "GUCEF::CORE::CTaskConsumer";
    return typeName;
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::SetTaskDelegator( const TTaskDelegatorBasicPtr& delegator )
{GUCEF_TRACE;

    m_delegator = delegator;
    if ( !m_delegator.IsNULL() )
    {
        SetPulseGenerator( m_delegator->GetPulseGenerator() );
    }
    else
    {
        SetPulseGenerator( PulseGeneratorPtr() );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::SetThreadPool( const ThreadPoolPtr& threadPool )
{GUCEF_TRACE;

    m_threadPool = threadPool;
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::OnTaskStarted( CTaskPtr task )
{GUCEF_TRACE;

    NotifyObservers( TaskStartedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CTaskConsumer::OnTaskStart( CTaskPtr task )
{GUCEF_TRACE;

    return NotifyObservers( TaskStartupEvent );
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::OnTaskStartupFailed( CTaskPtr task )
{GUCEF_TRACE;

    NotifyObservers( TaskStartupFailedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::OnTaskPaused( CTaskPtr task  ,
                             bool wasForced )
{GUCEF_TRACE;

    NotifyObservers( TaskPausedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::OnTaskResumed( CTaskPtr task )
{GUCEF_TRACE;

    NotifyObservers( TaskResumedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::OnTaskEnded( CTaskPtr task  ,
                            bool wasForced )
{GUCEF_TRACE;

    NotifyObservers( TaskFinishedEvent );
}

/*-------------------------------------------------------------------------*/

void
CTaskConsumer::OnTaskEnding( CTaskPtr task     ,
                             bool willBeForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CTaskConsumer::SetCpuAffinityMask( UInt32 affinityMaskSize ,
                                   void* affinityMask      )
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    if ( !delegator.IsNULL() )
    {
        return delegator->SetCpuAffinityMask( affinityMaskSize, affinityMask );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskConsumer::SetCpuAffinityByCpuId( UInt32 cpuId )
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    if ( !delegator.IsNULL() )
    {
        return delegator->SetCpuAffinityByCpuId( cpuId );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskConsumer::IsDeactivationRequested( void ) const
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    if ( !delegator.IsNULL() )
    {
        return delegator->IsDeactivationRequested();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskConsumer::IsActive( void ) const
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    if ( !delegator.IsNULL() )
    {
        return delegator->IsActive();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskConsumer::RequestTaskToStop( bool waitOnStop )
{GUCEF_TRACE;

    CTaskPtr task = m_currentTask;
    if ( !task.IsNULL() )
    {
        TTaskDelegatorBasicPtr delegator = m_delegator;
        if ( !delegator.IsNULL() )
        {
            ThreadPoolPtr threadPool = delegator->GetThreadPool();
            if ( !threadPool.IsNULL() )
            {
                return threadPool->RequestTaskToStop( task->GetTaskId(), waitOnStop );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskConsumer::HasTaskData( void ) const
{GUCEF_TRACE;

    TTaskDelegatorBasicPtr delegator = m_delegator;
    if ( !delegator.IsNULL() )
    {
        return delegator->HasTaskData( GetCurrentTaskId() );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskConsumer::SetTaskStatus( TTaskStatus newStatus )
{GUCEF_TRACE;

    CTaskPtr task = m_currentTask;
    if ( !task.IsNULL() )
    {
        task->SetTaskStatus( newStatus );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

TTaskStatus 
CTaskConsumer::GetTaskStatus( void ) const
{GUCEF_TRACE;

    CTaskPtr task = m_currentTask;
    if ( !task.IsNULL() )
    {
        return task->GetTaskStatus();
    }
    return TTaskStatus::TASKSTATUS_UNDEFINED;
}

/*-------------------------------------------------------------------------*/

ThreadPoolPtr
CTaskConsumer::GetThreadPool( void ) const
{GUCEF_TRACE;

    return m_threadPool;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
