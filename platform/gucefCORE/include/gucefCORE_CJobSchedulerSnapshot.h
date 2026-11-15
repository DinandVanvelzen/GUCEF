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

#ifndef GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H
#define GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CJOBSTATUS_H
#include "gucefCORE_CJobStatus.h"
#define GUCEF_CORE_CJOBSTATUS_H
#endif /* GUCEF_CORE_CJOBSTATUS_H ? */

#ifndef GUCEF_CORE_CJOBLANESTATUS_H
#include "gucefCORE_CJobLaneStatus.h"
#define GUCEF_CORE_CJOBLANESTATUS_H
#endif /* GUCEF_CORE_CJOBLANESTATUS_H ? */

#ifndef GUCEF_CORE_CJOBRUNRECORD_H
#include "gucefCORE_CJobRunRecord.h"
#define GUCEF_CORE_CJOBRUNRECORD_H
#endif /* GUCEF_CORE_CJOBRUNRECORD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Copy of the model of a job scheduler (jobs, lanes, active and recent runs, totals and revision), taken under the
 *  scheduler lock in one step (CJobScheduler::GetSnapshot()). Views, RPC answers and published rows are built from it.
 */
class GUCEF_CORE_PUBLIC_CPP CJobSchedulerSnapshot
{
    public:

    CJobSchedulerSnapshot( void );

    void SetHostName( const CString& hostName );
    const CString& GetHostName( void ) const;

    void SetHostInstanceId( UInt64 hostInstanceId );
    UInt64 GetHostInstanceId( void ) const;

    void SetRevision( UInt64 revision );
    UInt64 GetRevision( void ) const;

    /** 0 = a full snapshot; else only rows changed after this revision are in it */
    void SetChangedAfterRevision( UInt64 changedAfterRevision );
    UInt64 GetChangedAfterRevision( void ) const;

    void SetHostState( TJobSchedulerHostState hostState );
    TJobSchedulerHostState GetHostState( void ) const;

    void SetServerTimeUtcMs( Int64 serverTimeUtcMs );
    Int64 GetServerTimeUtcMs( void ) const;

    void SetTotalRunsStarted( UInt64 totalRunsStarted );
    UInt64 GetTotalRunsStarted( void ) const;

    void SetTotalRunsSucceeded( UInt64 totalRunsSucceeded );
    UInt64 GetTotalRunsSucceeded( void ) const;

    void SetTotalRunsFailed( UInt64 totalRunsFailed );
    UInt64 GetTotalRunsFailed( void ) const;

    void SetTotalRunsCancelled( UInt64 totalRunsCancelled );
    UInt64 GetTotalRunsCancelled( void ) const;

    void SetTotalRunsTimedOut( UInt64 totalRunsTimedOut );
    UInt64 GetTotalRunsTimedOut( void ) const;

    void SetTotalRunsSkipped( UInt64 totalRunsSkipped );
    UInt64 GetTotalRunsSkipped( void ) const;

    void SetSubmittableTaskTypes( const CString::StringSet& submittableTaskTypes );
    const CString::StringSet& GetSubmittableTaskTypes( void ) const;
    const TJobStatusVector& GetJobs( void ) const;
    TJobStatusVector& GetJobs( void );

    const TJobLaneStatusVector& GetLanes( void ) const;
    TJobLaneStatusVector& GetLanes( void );

    /** Active runs first (queued, waiting, running), then the recent ended runs, newest first */
    const TJobRunRecordVector& GetRuns( void ) const;
    TJobRunRecordVector& GetRuns( void );

    const SJobStatus* FindJob( const CString& jobId ) const;
    const SJobLaneStatus* FindLane( const CString& laneName ) const;

    void Clear( void );

    private:

    CString m_hostName;
    UInt64 m_hostInstanceId;
    UInt64 m_revision;
    UInt64 m_changedAfterRevision;
    TJobSchedulerHostState m_hostState;
    Int64 m_serverTimeUtcMs;
    UInt64 m_totalRunsStarted;
    UInt64 m_totalRunsSucceeded;
    UInt64 m_totalRunsFailed;
    UInt64 m_totalRunsCancelled;
    UInt64 m_totalRunsTimedOut;
    UInt64 m_totalRunsSkipped;
    CString::StringSet m_submittableTaskTypes;
    TJobStatusVector m_jobs;
    TJobLaneStatusVector m_lanes;
    TJobRunRecordVector m_runs;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBSCHEDULERSNAPSHOT_H ? */
