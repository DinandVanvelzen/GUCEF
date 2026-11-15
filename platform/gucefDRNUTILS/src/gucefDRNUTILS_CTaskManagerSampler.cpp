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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "gucefDRNUTILS_CTaskManagerSampler.h"

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

CTaskManagerSampler::CTaskManagerSampler( const CITaskManagerSource& source, const CORE::CString& poolName )
    : m_source( &source )
    , m_poolName( poolName )
    , m_task( GUCEF_NULL )
    , m_taskPtr()
    , m_samplesOfStoppedTasks( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CTaskManagerSampler::~CTaskManagerSampler()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_task )
    {
        m_task->RequestTaskToStop( false );
        m_task->Wake();
        m_task->WaitForTaskToFinish( 10000 );
    }
    m_task = GUCEF_NULL;
    m_taskPtr.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSampler::IsRunning( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_task;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSampler::Start( const CTaskManagerSamplerTask::SConfig& config, CORE::CTSGNotifier& observer, CORE::CIEventHandlerFunctorBase& callback )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_task )
    {
        Update( config );
        return true;
    }
    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( m_poolName, true );
    if ( pool.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TaskManagerSampler: the pool " << m_poolName << " could not be made" );
        return false;
    }
    m_task = GUCEF_NEW CTaskManagerSamplerTask( *m_source );
    CORE::CTSharedPtr< CTaskManagerSamplerTask, MT::CMutex > owner( m_task );
    m_taskPtr = owner;
    m_task->SetConfig( config );
    observer.SubscribeTo( m_task, CTaskManagerSamplerTask::SampleReadyEvent, callback );
    CORE::CFutureResult future = pool->StartTaskWithConsumer( m_taskPtr );
    if ( future.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TaskManagerSampler: the sampler task could not be started on pool " << m_poolName );
        observer.UnsubscribeFrom( m_task );
        m_task = GUCEF_NULL;
        m_taskPtr.Unlink();
        return false;
    }
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "TaskManagerSampler: sampling every " << config.intervalMs << " ms on pool " << m_poolName );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSampler::Update( const CTaskManagerSamplerTask::SConfig& config )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_task )
        m_task->SetConfig( config );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSampler::Stop( CORE::CTSGNotifier& observer )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_task )
        return;
    observer.UnsubscribeFrom( m_task );
    m_task->RequestTaskToStop( false );
    m_task->Wake();
    // the sampler task never waits for the notification thread, so waiting for it here cannot deadlock
    if ( !m_task->WaitForTaskToFinish( 10000 ) )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TaskManagerSampler: the sampler task did not stop within 10 s" );
    m_samplesOfStoppedTasks += m_task->GetSampleCount();
    m_task = GUCEF_NULL;
    m_taskPtr.Unlink();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSampler::GetLatest( CTaskManagerSample& sample ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_task && m_task->GetLatest( sample );
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskManagerSampler::GetSampleCount( void ) const
{GUCEF_TRACE;

    return m_samplesOfStoppedTasks + ( GUCEF_NULL != m_task ? m_task->GetSampleCount() : 0 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
