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

#ifndef GUCEF_CORE_CSCHEDULEDJOBINFO_H
#define GUCEF_CORE_CSCHEDULEDJOBINFO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

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
 *  Snapshot of one scheduled job as the task manager shows it next to its pools, threads and tasks
 *  (CTaskManager::GetAllScheduledJobInfo()): which scheduler, which task type, where its runs execute (a pool or the
 *  main thread) and how it stands. Detail of the job stays with CJobScheduler::GetSnapshot().
 */
class GUCEF_CORE_PUBLIC_CPP CScheduledJobInfo : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    CScheduledJobInfo( void );

    virtual ~CScheduledJobInfo() GUCEF_VIRTUAL_OVERRIDE;

    /** Host name of the scheduler */
    void SetSchedulerName( const CString& schedulerName );
    const CString& GetSchedulerName( void ) const;

    void SetJobId( const CString& jobId );
    const CString& GetJobId( void ) const;

    void SetTitle( const CString& title );
    const CString& GetTitle( void ) const;

    void SetTaskTypeName( const CString& taskTypeName );
    const CString& GetTaskTypeName( void ) const;

    void SetLaneName( const CString& laneName );
    const CString& GetLaneName( void ) const;

    void SetState( TJobState state );
    TJobState GetState( void ) const;

    void SetExecutionTarget( TJobExecutionTarget executionTarget );
    TJobExecutionTarget GetExecutionTarget( void ) const;

    /** POOL: the pool of the runs; MAIN_THREAD: the scheduler's main thread pool */
    void SetThreadPoolName( const CString& threadPoolName );
    const CString& GetThreadPoolName( void ) const;

    void SetOrigin( TJobOrigin origin );
    TJobOrigin GetOrigin( void ) const;

    /** 0 = none */
    void SetNextDueUtcMs( Int64 nextDueUtcMs );
    Int64 GetNextDueUtcMs( void ) const;

    void SetActiveRunCount( UInt32 activeRunCount );
    UInt32 GetActiveRunCount( void ) const;

    void SetLastRunId( UInt64 lastRunId );
    UInt64 GetLastRunId( void ) const;

    void SetLastRunState( TJobRunState lastRunState );
    TJobRunState GetLastRunState( void ) const;

    void SetLastRunEndedUtcMs( Int64 lastRunEndedUtcMs );
    Int64 GetLastRunEndedUtcMs( void ) const;

    /** EJobFlags of the job's definition */
    void SetFlags( UInt32 flags );
    UInt32 GetFlags( void ) const;

    /**
     *  JOBFLAG_HIDDEN: the job is shown in the process (the task manager overview) and never leaves it: remote views
     *  (gucefDRNUTILS: TaskManager/ListScheduledJobs, Watch, the scheduledjobs grid) leave it out
     */
    bool IsHidden( void ) const;
    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString m_schedulerName;
    CString m_jobId;
    CString m_title;
    CString m_taskTypeName;
    CString m_laneName;
    TJobState m_state;
    TJobExecutionTarget m_executionTarget;
    CString m_threadPoolName;
    TJobOrigin m_origin;
    Int64 m_nextDueUtcMs;
    UInt32 m_activeRunCount;
    UInt64 m_lastRunId;
    TJobRunState m_lastRunState;
    Int64 m_lastRunEndedUtcMs;
    UInt32 m_flags;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CScheduledJobInfo > TScheduledJobInfoVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSCHEDULEDJOBINFO_H ? */
