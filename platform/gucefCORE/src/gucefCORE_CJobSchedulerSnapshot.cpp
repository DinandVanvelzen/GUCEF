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

#include "gucefCORE_CJobSchedulerSnapshot.h"

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

CJobSchedulerSnapshot::CJobSchedulerSnapshot( void )
    : m_hostName()
    , m_hostInstanceId( 0 )
    , m_revision( 0 )
    , m_changedAfterRevision( 0 )
    , m_hostState( JOBSCHEDULERSTATE_CREATED )
    , m_serverTimeUtcMs( 0 )
    , m_totalRunsStarted( 0 )
    , m_totalRunsSucceeded( 0 )
    , m_totalRunsFailed( 0 )
    , m_totalRunsCancelled( 0 )
    , m_totalRunsTimedOut( 0 )
    , m_totalRunsSkipped( 0 )
    , m_submittableTaskTypes()
    , m_jobs()
    , m_lanes()
    , m_runs()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const TJobStatusVector&
CJobSchedulerSnapshot::GetJobs( void ) const
{GUCEF_TRACE;

    return m_jobs;
}

/*-------------------------------------------------------------------------*/

TJobStatusVector&
CJobSchedulerSnapshot::GetJobs( void )
{GUCEF_TRACE;

    return m_jobs;
}

/*-------------------------------------------------------------------------*/

const TJobLaneStatusVector&
CJobSchedulerSnapshot::GetLanes( void ) const
{GUCEF_TRACE;

    return m_lanes;
}

/*-------------------------------------------------------------------------*/

TJobLaneStatusVector&
CJobSchedulerSnapshot::GetLanes( void )
{GUCEF_TRACE;

    return m_lanes;
}

/*-------------------------------------------------------------------------*/

const TJobRunRecordVector&
CJobSchedulerSnapshot::GetRuns( void ) const
{GUCEF_TRACE;

    return m_runs;
}

/*-------------------------------------------------------------------------*/

TJobRunRecordVector&
CJobSchedulerSnapshot::GetRuns( void )
{GUCEF_TRACE;

    return m_runs;
}

/*-------------------------------------------------------------------------*/

const SJobStatus*
CJobSchedulerSnapshot::FindJob( const CString& jobId ) const
{GUCEF_TRACE;

    TJobStatusVector::const_iterator i = m_jobs.begin();
    while ( i != m_jobs.end() )
    {
        if ( (*i).jobId == jobId )
            return &(*i);
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const SJobLaneStatus*
CJobSchedulerSnapshot::FindLane( const CString& laneName ) const
{GUCEF_TRACE;

    TJobLaneStatusVector::const_iterator i = m_lanes.begin();
    while ( i != m_lanes.end() )
    {
        if ( (*i).laneName == laneName )
            return &(*i);
        ++i;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::Clear( void )
{GUCEF_TRACE;

    m_hostName = CString();
    m_hostInstanceId = 0;
    m_revision = 0;
    m_changedAfterRevision = 0;
    m_hostState = JOBSCHEDULERSTATE_CREATED;
    m_serverTimeUtcMs = 0;
    m_totalRunsStarted = 0;
    m_totalRunsSucceeded = 0;
    m_totalRunsFailed = 0;
    m_totalRunsCancelled = 0;
    m_totalRunsTimedOut = 0;
    m_totalRunsSkipped = 0;
    m_submittableTaskTypes = CString::StringSet();
    m_jobs.clear();
    m_lanes.clear();
    m_runs.clear();
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetHostName( const CString& hostName )
{GUCEF_TRACE;

    m_hostName = hostName;
}

/*-------------------------------------------------------------------------*/

const CString&
CJobSchedulerSnapshot::GetHostName( void ) const
{GUCEF_TRACE;

    return m_hostName;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetHostInstanceId( UInt64 hostInstanceId )
{GUCEF_TRACE;

    m_hostInstanceId = hostInstanceId;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetHostInstanceId( void ) const
{GUCEF_TRACE;

    return m_hostInstanceId;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetRevision( UInt64 revision )
{GUCEF_TRACE;

    m_revision = revision;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetRevision( void ) const
{GUCEF_TRACE;

    return m_revision;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetChangedAfterRevision( UInt64 changedAfterRevision )
{GUCEF_TRACE;

    m_changedAfterRevision = changedAfterRevision;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetChangedAfterRevision( void ) const
{GUCEF_TRACE;

    return m_changedAfterRevision;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetHostState( TJobSchedulerHostState hostState )
{GUCEF_TRACE;

    m_hostState = hostState;
}

/*-------------------------------------------------------------------------*/

TJobSchedulerHostState
CJobSchedulerSnapshot::GetHostState( void ) const
{GUCEF_TRACE;

    return m_hostState;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetServerTimeUtcMs( Int64 serverTimeUtcMs )
{GUCEF_TRACE;

    m_serverTimeUtcMs = serverTimeUtcMs;
}

/*-------------------------------------------------------------------------*/

Int64
CJobSchedulerSnapshot::GetServerTimeUtcMs( void ) const
{GUCEF_TRACE;

    return m_serverTimeUtcMs;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetTotalRunsStarted( UInt64 totalRunsStarted )
{GUCEF_TRACE;

    m_totalRunsStarted = totalRunsStarted;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetTotalRunsStarted( void ) const
{GUCEF_TRACE;

    return m_totalRunsStarted;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetTotalRunsSucceeded( UInt64 totalRunsSucceeded )
{GUCEF_TRACE;

    m_totalRunsSucceeded = totalRunsSucceeded;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetTotalRunsSucceeded( void ) const
{GUCEF_TRACE;

    return m_totalRunsSucceeded;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetTotalRunsFailed( UInt64 totalRunsFailed )
{GUCEF_TRACE;

    m_totalRunsFailed = totalRunsFailed;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetTotalRunsFailed( void ) const
{GUCEF_TRACE;

    return m_totalRunsFailed;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetTotalRunsCancelled( UInt64 totalRunsCancelled )
{GUCEF_TRACE;

    m_totalRunsCancelled = totalRunsCancelled;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetTotalRunsCancelled( void ) const
{GUCEF_TRACE;

    return m_totalRunsCancelled;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetTotalRunsTimedOut( UInt64 totalRunsTimedOut )
{GUCEF_TRACE;

    m_totalRunsTimedOut = totalRunsTimedOut;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetTotalRunsTimedOut( void ) const
{GUCEF_TRACE;

    return m_totalRunsTimedOut;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetTotalRunsSkipped( UInt64 totalRunsSkipped )
{GUCEF_TRACE;

    m_totalRunsSkipped = totalRunsSkipped;
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSnapshot::GetTotalRunsSkipped( void ) const
{GUCEF_TRACE;

    return m_totalRunsSkipped;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSnapshot::SetSubmittableTaskTypes( const CString::StringSet& submittableTaskTypes )
{GUCEF_TRACE;

    m_submittableTaskTypes = submittableTaskTypes;
}

/*-------------------------------------------------------------------------*/

const CString::StringSet&
CJobSchedulerSnapshot::GetSubmittableTaskTypes( void ) const
{GUCEF_TRACE;

    return m_submittableTaskTypes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
