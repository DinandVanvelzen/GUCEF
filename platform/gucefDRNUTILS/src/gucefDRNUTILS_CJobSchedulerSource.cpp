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

#ifndef GUCEF_DRNUTILS_CJOBSCHEDULERSOURCERELAY_H
#include "gucefDRNUTILS_CJobSchedulerSourceRelay.h"
#define GUCEF_DRNUTILS_CJOBSCHEDULERSOURCERELAY_H
#endif /* GUCEF_DRNUTILS_CJOBSCHEDULERSOURCERELAY_H ? */

#include "gucefDRNUTILS_CJobSchedulerSource.h"

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

CJobSchedulerSource::CJobSchedulerSource( const CORE::JobSchedulerPtr& scheduler, const CORE::PulseGeneratorPtr& notificationPulseGenerator )
    : CISchedulerSource()
    , m_scheduler( scheduler )
    , m_relay( GUCEF_NEW CJobSchedulerSourceRelay( scheduler, notificationPulseGenerator ) )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJobSchedulerSource::~CJobSchedulerSource()
{GUCEF_TRACE;

    GUCEF_DELETE m_relay;
    m_relay = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::JobSchedulerPtr
CJobSchedulerSource::GetScheduler( void ) const
{GUCEF_TRACE;

    return m_scheduler;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CJobSchedulerSource::GetHostName( void ) const
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        return CORE::CString::Empty;
    return m_scheduler->GetName();
}

/*-------------------------------------------------------------------------*/

UInt64
CJobSchedulerSource::GetHostInstanceId( void ) const
{GUCEF_TRACE;

    return m_scheduler.IsNULL() ? 0 : m_scheduler->GetHostInstanceId();
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSource::GetSnapshot( CORE::CJobSchedulerSnapshot& snapshot, UInt64 changedAfterRevision ) const
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        snapshot.Clear();
    else
        m_scheduler->GetSnapshot( snapshot, changedAfterRevision );
}

/*-------------------------------------------------------------------------*/

bool
CJobSchedulerSource::GetJobDetail( const CORE::CString& jobId, UInt32 recentRuns, bool includeSourceStatus, SSchedulerJobDetail& detail ) const
{GUCEF_TRACE;

    detail = SSchedulerJobDetail();
    if ( m_scheduler.IsNULL() )
        return false;
    CORE::CJobSchedulerSnapshot snapshot;
    m_scheduler->GetSnapshot( snapshot, 0 );
    const CORE::SJobStatus* job = snapshot.FindJob( jobId );
    if ( GUCEF_NULL == job )
        return false;
    detail.job = *job;
    // the snapshot lists the runs newest first (active ones before them)
    const CORE::TJobRunRecordVector& runs = snapshot.GetRuns();
    for ( UInt32 i=0; i<runs.size() && detail.recentRuns.size() < recentRuns; ++i )
        if ( runs[ i ].jobId == jobId )
            detail.recentRuns.push_back( runs[ i ] );
    // the status of the job's work source, read outside every lock of the scheduler (change request B-3)
    if ( includeSourceStatus )
        m_scheduler->GetWorkSourceStatus( jobId, detail.sourceStatus, false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSource::GetDeclaredActions( TSchedulerActionInfoVector& actions ) const
{GUCEF_TRACE;

    actions.clear();
    if ( m_scheduler.IsNULL() )
        return;
    static const CORE::TJobControlAction generic[ 7 ] = { CORE::JOBCONTROLACTION_RUN_NOW, CORE::JOBCONTROLACTION_CANCEL_RUN, CORE::JOBCONTROLACTION_ENABLE,
                                                          CORE::JOBCONTROLACTION_DISABLE, CORE::JOBCONTROLACTION_SET_PRIORITY,
                                                          CORE::JOBCONTROLACTION_SET_LANE_MODE, CORE::JOBCONTROLACTION_REMOVE_JOB };
    for ( UInt32 a=0; a<7; ++a )
    {
        SSchedulerActionInfo info;
        info.action = generic[ a ];
        info.targetKind = CORE::JOBCONTROLACTION_CANCEL_RUN == generic[ a ] ? CORE::JOBCONTROLTARGET_RUN :
                        ( CORE::JOBCONTROLACTION_SET_LANE_MODE == generic[ a ] ? CORE::JOBCONTROLTARGET_LANE : CORE::JOBCONTROLTARGET_JOB );
        info.title = CORE::JobControlActionToString( generic[ a ] );
        actions.push_back( info );
    }
    // custom actions declared by the work sources of the jobs
    CORE::CJobSchedulerSnapshot snapshot;
    m_scheduler->GetSnapshot( snapshot, 0 );
    CORE::CString::StringSet listed;
    const CORE::TJobStatusVector& jobs = snapshot.GetJobs();
    for ( UInt32 j=0; j<jobs.size(); ++j )
    {
        for ( UInt32 c=0; c<jobs[ j ].customActions.size(); ++c )
        {
            const CORE::SJobCustomAction& custom = jobs[ j ].customActions[ c ];
            if ( listed.find( custom.name ) != listed.end() )
                continue;
            listed.insert( custom.name );
            SSchedulerActionInfo info;
            info.action = CORE::JOBCONTROLACTION_CUSTOM;
            info.targetKind = custom.targetKind;
            info.customName = custom.name;
            info.title = custom.title;
            info.argumentNames = custom.argumentNames;
            info.privileged = custom.privileged;
            actions.push_back( info );
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::TJobControlStatus
CJobSchedulerSource::Apply( const CORE::SJobControlCommand& command, CORE::SJobControlResult& result )
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        return CORE::JOBCONTROLSTATUS_INTERNAL;
    return m_scheduler->Control( command, result );
}

/*-------------------------------------------------------------------------*/

void
CJobSchedulerSource::GetSubmittableTaskTypes( CORE::CString::StringSet& taskTypeNames ) const
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        taskTypeNames.clear();
    else
        m_scheduler->GetSubmittableTaskTypes( taskTypeNames );
}

/*-------------------------------------------------------------------------*/

CORE::TJobControlStatus
CJobSchedulerSource::Submit( const CORE::SJobSubmission& submission, CORE::SJobControlResult& result )
{GUCEF_TRACE;

    if ( m_scheduler.IsNULL() )
        return CORE::JOBCONTROLSTATUS_INTERNAL;
    return m_scheduler->SubmitJob( submission, result );
}

/*-------------------------------------------------------------------------*/

CORE::CNotifier*
CJobSchedulerSource::GetChangeNotifier( void )
{GUCEF_TRACE;

    return m_relay;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
