/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#include "gucefDRNUTILS_CTaskManagerSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTaskManagerSource::CTaskManagerSource( const CORE::CString& hostName )
    : CITaskManagerSource()
    , m_hostName( hostName )
    , m_hostInstanceId( 0 )
{GUCEF_TRACE;

    // random per process start: a viewer sees a restart of the node
    CORE::CRandom::GetRandomBytes( &m_hostInstanceId, sizeof( m_hostInstanceId ) );
    if ( 0 == m_hostInstanceId )
        m_hostInstanceId = 1;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSource::~CTaskManagerSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerSource::GetHostName( void ) const
{GUCEF_TRACE;

    return m_hostName;
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskManagerSource::GetHostInstanceId( void ) const
{GUCEF_TRACE;

    return m_hostInstanceId;
}

/*-------------------------------------------------------------------------*/

Int64
CTaskManagerSource::GetUtcTimeInMs( void ) const
{GUCEF_TRACE;

    return (Int64) CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs();
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskManagerSource::GetMonotonicTimeInMs( void ) const
{GUCEF_TRACE;

    return CORE::CSystemMonotonicClock().GetMonotonicTimeInMs();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::GetPoolCounters( CORE::CTaskManager::TThreadPoolCounterMap& counters ) const
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetTaskManager().GetPoolCounters( counters );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::GetAllThreadPoolInfo( CORE::CTaskManager::TThreadPoolInfoMap& info ) const
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetTaskManager().GetAllThreadPoolInfo( info );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::GetAllThreadInfo( CORE::CTaskManager::TThreadInfoMap& info ) const
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetTaskManager().GetAllThreadInfo( info );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::GetAllTaskInfo( CORE::CTaskManager::TTaskInfoMap& info ) const
{GUCEF_TRACE;

    // never with task data copies: task data is never exposed
    return CORE::CCoreGlobal::Instance()->GetTaskManager().GetAllTaskInfo( info, false );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::GetAllScheduledJobInfo( CORE::TScheduledJobInfoVector& info ) const
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetTaskManager().GetAllScheduledJobInfo( info );
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::ResizePool( const CORE::CString& poolName, UInt32 minWorkers, Int32 maxThreads, CORE::CString& error )
{GUCEF_TRACE;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( poolName );
    if ( pool.IsNULL() )
    {
        error = "no thread pool \"" + poolName + "\"";
        return false;
    }
    pool->SetDesiredMinNrOfWorkerThreads( minWorkers );
    pool->SetDesiredMaxTotalNrOfThreads( maxThreads );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::CancelTask( UInt32 taskId, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CString poolName;
    CORE::CString taskTypeName;
    if ( !GetTaskPoolAndType( taskId, poolName, taskTypeName ) )
    {
        error = "no task " + CORE::ToString( taskId );
        return false;
    }
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( poolName );
    if ( pool.IsNULL() || !pool->RequestTaskCancellation( taskId, false, false ) )
    {
        error = "the cancellation of task " + CORE::ToString( taskId ) + " was refused";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::StopTask( UInt32 taskId, CORE::CString& error )
{GUCEF_TRACE;

    CORE::CString poolName;
    CORE::CString taskTypeName;
    if ( !GetTaskPoolAndType( taskId, poolName, taskTypeName ) )
    {
        error = "no task " + CORE::ToString( taskId );
        return false;
    }
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( poolName );
    if ( pool.IsNULL() || !pool->RequestTaskToStop( taskId, false, false ) )
    {
        error = "the stop request of task " + CORE::ToString( taskId ) + " was refused";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSource::GetTaskPoolAndType( UInt32 taskId, CORE::CString& poolName, CORE::CString& taskTypeName ) const
{GUCEF_TRACE;

    CORE::CTaskInfo info;
    if ( !CORE::CCoreGlobal::Instance()->GetTaskManager().GetTaskInfo( taskId, info, poolName, false ) )
        return false;
    taskTypeName = info.GetTaskTypeName();
    if ( poolName.IsNULLOrEmpty() )
        poolName = info.GetThreadPoolName();
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
