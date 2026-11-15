/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefCORE_CJobSchedulerHealthCheck.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJobSchedulerHealthCheck::CJobSchedulerHealthCheck( const JobSchedulerPtr& scheduler, UInt64 stallThresholdMs, UInt32 failureWarnCount )
    : CIServiceHealthCheck()
    , m_scheduler( scheduler )
    , m_stallThresholdMs( stallThresholdMs )
    , m_failureWarnCount( failureWarnCount )
    , m_name( "scheduler" )
{GUCEF_TRACE;

    if ( !m_scheduler.IsNULL() )
        m_name = "scheduler:" + m_scheduler->GetName();
}

/*-------------------------------------------------------------------------*/

CJobSchedulerHealthCheck::~CJobSchedulerHealthCheck()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerHealthCheck::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

TServiceHealthState
CJobSchedulerHealthCheck::Check( CString& detail )
{GUCEF_TRACE;

    detail.Clear();
    if ( m_scheduler.IsNULL() )
    {
        detail = "no scheduler";
        return HEALTH_FAIL;
    }

    CJobSchedulerSnapshot snapshot;
    m_scheduler->GetSnapshot( snapshot );
    if ( JOBSCHEDULERSTATE_STOPPED == snapshot.GetHostState() || JOBSCHEDULERSTATE_CREATED == snapshot.GetHostState() )
    {
        detail = "the scheduler is " + CString( JobSchedulerHostStateToString( snapshot.GetHostState() ) );
        return HEALTH_FAIL;
    }

    TServiceHealthState health = HEALTH_PASS;
    Int64 nowUtcMs = snapshot.GetServerTimeUtcMs();
    const TJobStatusVector& jobs = snapshot.GetJobs();
    for ( TJobStatusVector::const_iterator i = jobs.begin(); i != jobs.end(); ++i )
    {
        const SJobStatus& job = (*i);
        if ( JOBSTATE_SCHEDULED == job.state && 0 != job.nextDueUtcMs && nowUtcMs - job.nextDueUtcMs > (Int64) m_stallThresholdMs )
        {
            detail = "job " + job.jobId + " was due " + ToString( nowUtcMs - job.nextDueUtcMs ) + " ms ago and was not dispatched";
            return HEALTH_FAIL;
        }
        if ( job.consecutiveFailures > m_failureWarnCount && HEALTH_PASS == health )
        {
            detail = "job " + job.jobId + " failed " + ToString( job.consecutiveFailures ) + " times in a row";
            health = HEALTH_WARN;
        }
    }
    return health;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
