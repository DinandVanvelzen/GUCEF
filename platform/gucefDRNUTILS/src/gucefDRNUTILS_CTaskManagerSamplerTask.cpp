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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILS_CTaskManagerSamplerTask.h"

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

const CORE::CEvent CTaskManagerSamplerTask::SampleReadyEvent = "GUCEF::DRNUTILS::CTaskManagerSamplerTask::SampleReadyEvent";
const CORE::CString CTaskManagerSamplerTask::TaskType = "gucef.drnutils.sampler";

/*-------------------------------------------------------------------------*/

void
CTaskManagerSamplerTask::RegisterEvents( void )
{GUCEF_TRACE;

    SampleReadyEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTaskManagerSamplerTask::SConfig::SConfig( void )
    : intervalMs( 2000 )
    , taskIntervalMs( 5000 )
    , minIntervalMs( 1000 )
    , options()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CTaskManagerSamplerTask::CTaskManagerSamplerTask( const CITaskManagerSource& source )
    : CORE::CTaskConsumer()
    , m_source( &source )
    , m_lock()
    , m_wake( 0 )
    , m_config()
    , m_first( true )
    , m_lastTaskSampleMs( 0 )
    , m_sampleCount( 0 )
    , m_previous()
    , m_previousWithTasks()
    , m_current()
    , m_latest()
    , m_hasLatest( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CTaskManagerSamplerTask::~CTaskManagerSamplerTask()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSamplerTask::SetConfig( const SConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_config = config;
    const UInt32 minIntervalMs = 0 == m_config.minIntervalMs ? 1 : m_config.minIntervalMs;
    if ( m_config.intervalMs < minIntervalMs )
        m_config.intervalMs = minIntervalMs;
    if ( m_config.taskIntervalMs < m_config.intervalMs )
        m_config.taskIntervalMs = m_config.intervalMs;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSamplerTask::Wake( void )
{GUCEF_TRACE;

    m_wake.Post();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSamplerTask::GetLatest( CTaskManagerSample& sample ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_hasLatest )
        return false;
    sample = m_latest;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CTaskManagerSamplerTask::GetSampleCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_sampleCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CTaskManagerSamplerTask::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSamplerTask::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    m_first = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerSamplerTask::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    SConfig config;
    {
        MT::CScopeMutex lock( m_lock );
        config = m_config;
    }
    if ( !m_first )
    {
        // sleeps until the next sample; a stop request or a new configuration posts the semaphore
        m_wake.WaitOrTimeout( config.intervalMs );
        while ( 0 == m_wake.TryWait() ) {}
    }
    m_first = false;
    if ( IsDeactivationRequested() )
        return true;
    {
        MT::CScopeMutex lock( m_lock );
        config = m_config;
    }

    UInt64 nowMs = m_source->GetMonotonicTimeInMs();
    CTaskManagerSample::SOptions options = config.options;
    bool taskCycle = 0 == m_lastTaskSampleMs || nowMs - m_lastTaskSampleMs >= config.taskIntervalMs;
    if ( !taskCycle )
    {
        options.includeThreads = false;
        options.includeTasks = false;
    }
    else
    if ( options.includeThreads || options.includeTasks )
    {
        m_lastTaskSampleMs = nowMs;
    }

    // no lock held while the source (the task manager) is asked
    m_current.Take( *m_source, m_sampleCount > 0 ? &m_previous : GUCEF_NULL, m_previousWithTasks.hasTasks ? &m_previousWithTasks : GUCEF_NULL, options );
    m_previous = m_current;
    if ( m_current.hasTasks )
        m_previousWithTasks = m_current;
    {
        MT::CScopeMutex lock( m_lock );
        m_latest = m_current;
        m_hasLatest = true;
        ++m_sampleCount;
    }
    NotifyObservers( SampleReadyEvent );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerSamplerTask::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CTaskManagerSamplerTask::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CTaskManagerSamplerTask";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
