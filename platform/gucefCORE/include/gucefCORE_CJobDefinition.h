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

#ifndef GUCEF_CORE_CJOBDEFINITION_H
#define GUCEF_CORE_CJOBDEFINITION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_JOBSCHEDULERTYPES_H
#include "gucefCORE_JobSchedulerTypes.h"
#define GUCEF_CORE_JOBSCHEDULERTYPES_H
#endif /* GUCEF_CORE_JOBSCHEDULERTYPES_H ? */

#ifndef GUCEF_CORE_CJOBTRIGGER_H
#include "gucefCORE_CJobTrigger.h"
#define GUCEF_CORE_CJOBTRIGGER_H
#endif /* GUCEF_CORE_CJOBTRIGGER_H ? */

#ifndef GUCEF_CORE_CJOBRETRYPOLICY_H
#include "gucefCORE_CJobRetryPolicy.h"
#define GUCEF_CORE_CJOBRETRYPOLICY_H
#endif /* GUCEF_CORE_CJOBRETRYPOLICY_H ? */

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
 *  A job: a task type of the task manager, a payload, a trigger, a lane and policies.
 *
 *  The scheduler adds no work abstraction of its own. A run of a job is a task of the task manager: a task of the type
 *  TaskTypeName, created through the task consumer factory registered under that name, with the task data that the
 *  task data factory of that name deserializes from the Payload (CIDataNodeSerializableTaskData). It runs on the pool
 *  ThreadPoolName (POOL) or is carried out by the thread of the scheduler's main pulse generator (MAIN_THREAD).
 *  Because a job is (task type, payload, trigger, target), jobs can also be submitted as data (CJobScheduler::SubmitJob)
 *  for the task types a scheduler allows.
 *
 *  Serialized form: a CDataNode object with the fields below by their camel case names ("id", "taskTypeName",
 *  "payload" (the payload node as a child), "trigger" (CJobTrigger), "executionTarget" (JobExecutionTargetToString),
 *  "threadPoolName", ...), so definitions can come from settings files.
 */
class GUCEF_CORE_PUBLIC_CPP CJobDefinition : public CIDataNodeSerializable
{
    public:

    static const CString ClassTypeName;

    static const UInt32 MaxJobIdLength = 64;

    CJobDefinition( void );

    virtual ~CJobDefinition() GUCEF_VIRTUAL_OVERRIDE;

    /** Stable id, [a-z][a-z0-9._-]*, at most MaxJobIdLength bytes; never user data (it appears in grid rows) */
    void SetId( const CString& id );
    const CString& GetId( void ) const;

    /** Display title; empty = the id */
    void SetTitle( const CString& title );
    const CString& GetTitle( void ) const;

    void SetDescription( const CString& description );
    const CString& GetDescription( void ) const;

    /** Task type of the runs: the name under which the task consumer factory (and the task data factory, for a payload) is registered with the task manager */
    void SetTaskTypeName( const CString& taskTypeName );
    const CString& GetTaskTypeName( void ) const;

    /** The task data of each run in serialized form, deserialized by the task data factory of the task type (CIDataNodeSerializableTaskData); empty = no task data */
    void SetPayload( const CDataNode& payload );
    const CDataNode& GetPayload( void ) const;

    /** When the job becomes due */
    void SetTrigger( const CJobTrigger& trigger );
    const CJobTrigger& GetTrigger( void ) const;

    /** Serialisation and throttling channel; empty = the default lane */
    void SetLaneName( const CString& laneName );
    const CString& GetLaneName( void ) const;

    /** Where the runs execute */
    void SetExecutionTarget( TJobExecutionTarget executionTarget );
    TJobExecutionTarget GetExecutionTarget( void ) const;

    /** POOL: the pool of the run tasks; empty = the scheduler's run pool (settings RunPoolName) */
    void SetThreadPoolName( const CString& threadPoolName );
    const CString& GetThreadPoolName( void ) const;

    /** 0 highest .. 3 lowest; order of dispatch: priority, due time, enqueue order */
    void SetPriority( UInt32 priority );
    UInt32 GetPriority( void ) const;

    void SetConcurrency( TJobConcurrency concurrency );
    TJobConcurrency GetConcurrency( void ) const;

    /** PARALLEL: the most runs at once */
    void SetMaxParallelRuns( UInt32 maxParallelRuns );
    UInt32 GetMaxParallelRuns( void ) const;

    void SetMissedRunPolicy( TJobMissedRunPolicy missedRunPolicy );
    TJobMissedRunPolicy GetMissedRunPolicy( void ) const;

    /** RUN_EACH: the most catch up runs */
    void SetMaxCatchUpRuns( UInt32 maxCatchUpRuns );
    UInt32 GetMaxCatchUpRuns( void ) const;

    /** An occurrence found later than this after it was due counts as missed; 0 = no maximum */
    void SetMaxLatenessMs( UInt64 maxLatenessMs );
    UInt64 GetMaxLatenessMs( void ) const;

    /** The run task is asked to stop after this run time; 0 = none */
    void SetTimeoutMs( UInt64 timeoutMs );
    UInt64 GetTimeoutMs( void ) const;

    void SetRetryPolicy( const CJobRetryPolicy& retryPolicy );
    const CJobRetryPolicy& GetRetryPolicy( void ) const;

    /** EJobFlags */
    void SetFlags( UInt32 flags );
    UInt32 GetFlags( void ) const;

    /** Control actions remote principals may apply to this job; none by default */
    void SetRemoteActions( const TJobControlActionSet& remoteActions );
    const TJobControlActionSet& GetRemoteActions( void ) const;

    /** Custom action names remote principals may apply to this job; none by default */
    void SetRemoteCustomActions( const CString::StringSet& remoteCustomActions );
    const CString::StringSet& GetRemoteCustomActions( void ) const;

    /** Set by the scheduler */
    void SetOrigin( TJobOrigin origin );
    TJobOrigin GetOrigin( void ) const;

    /** SUBMITTED: the subject of the principal that submitted the job; set by the scheduler */
    void SetSubmittedBy( const CString& submittedBy );
    const CString& GetSubmittedBy( void ) const;

    /** SUBMITTED: when; set by the scheduler */
    void SetSubmittedAtUtcMs( Int64 submittedAtUtcMs );
    Int64 GetSubmittedAtUtcMs( void ) const;
    /** Whether the job id has the form [a-z][a-z0-9._-]* and at most MaxJobIdLength bytes */
    static bool IsValidJobId( const CString& jobId );

    /** Whether the definition can be added: valid id, task type set, valid trigger, sane bounds; the reason otherwise */
    bool IsValid( CString& error ) const;

    virtual bool Serialize( CDataNode& domRootNode                        ,
                            const CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CDataNode& domRootNode                  ,
                              const CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CString m_id;
    CString m_title;
    CString m_description;
    CString m_taskTypeName;
    CDataNode m_payload;
    CJobTrigger m_trigger;
    CString m_laneName;
    TJobExecutionTarget m_executionTarget;
    CString m_threadPoolName;
    UInt32 m_priority;
    TJobConcurrency m_concurrency;
    UInt32 m_maxParallelRuns;
    TJobMissedRunPolicy m_missedRunPolicy;
    UInt32 m_maxCatchUpRuns;
    UInt64 m_maxLatenessMs;
    UInt64 m_timeoutMs;
    CJobRetryPolicy m_retryPolicy;
    UInt32 m_flags;
    TJobControlActionSet m_remoteActions;
    CString::StringSet m_remoteCustomActions;
    TJobOrigin m_origin;
    CString m_submittedBy;
    Int64 m_submittedAtUtcMs;
};

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CJobDefinition > TJobDefinitionVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CJOBDEFINITION_H ? */
