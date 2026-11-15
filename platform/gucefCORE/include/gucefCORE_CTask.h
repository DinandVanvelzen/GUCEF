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

#ifndef GUCEF_CORE_CTASK_H
#define GUCEF_CORE_CTASK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CDEFERREDTASK_H
#include "gucefCORE_CDeferredTask.h"
#define GUCEF_CORE_CDEFERREDTASK_H
#endif /* GUCEF_CORE_CDEFERREDTASK_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULERRUNLINK_H
#include "gucefCORE_CJobSchedulerRunLink.h"
#define GUCEF_CORE_CJOBSCHEDULERRUNLINK_H
#endif /* GUCEF_CORE_CJOBSCHEDULERRUNLINK_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

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

class CTaskDelegator;
class CSingleTaskDelegator;
class CThreadPool;
class CTaskManager;
class CASync;
class CFutureResultSource;
template< typename T, class LockType > class CTFreeList;

typedef CTBasicSharedPtr< CThreadPool, MT::CMutex >    CThreadPoolPtr;

/**
 *  class which provides access to all the information used to formulate a 'task'
 *  A task is a unit of work to be executed by a thread using a task consumer to provide the
 *  necessary task specific logic
 */
class GUCEF_CORE_PUBLIC_CPP CTask : public CNotifier ,
                                    public CTSharedObjCreator< CTask, MT::CMutex >
{
    public:

    typedef typename CTSharedObjCreator< CTask, MT::CMutex >::TBasicSharedPtrType    CTaskPtr;
    typedef CTNumericIDGenerator< UInt32, MT::CMutex >                               TTaskIdGenerator;
    typedef TTaskIdGenerator::TNumericID                                             TTaskId;
    typedef TTaskIdGenerator::TIntegerTypeUsedForId                                  TIntegerTypeUsedForTaskId;
    typedef CTBasicSharedPtr< CTaskDelegator, MT::CMutex >                           TTaskDelegatorBasicPtr;
    typedef GUCEF::set< CTaskPtr >                                                   TTaskPtrSet;
    typedef GUCEF::vector< CTaskPtr >                                                TTaskPtrVector;
    typedef GUCEF::vector< TIntegerTypeUsedForTaskId >                               TTaskIdVector;

    CTask( TTaskStatus taskStatus = CORE::TASKSTATUS_UNDEFINED );

    #ifdef GUCEF_MOVE_SEMANTICS_SUPPORTED
    CTask( CTask&& src ) GUCEF_NOEXCEPT;
    #endif

    virtual ~CTask() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The task consumer currently associated with the task if any
     *  This represents a snapshot in time only
     */
    CTaskConsumerPtr GetTaskConsumer( void ) const;

    /**
     *  The thread pool currently associated with the task if any
     *  This represents a snapshot in time only
     */
    CThreadPoolPtr GetThreadPool( void ) const;

    /**
     *  The type name of the task
     *  Used to match against a task consumer capable of executing a task of said type
     */
    const CString& GetTaskType( void ) const;

    /**
     *  The data that was associated with the task at the time of task creation if any
     */
    CICloneable* GetTaskData( void ) const;

    /**
     *  syntactic helper function to check if the task has associated data
     */
    bool HasTaskData( void ) const;

    /**
     *  Historically informative only. Whether the task data was cloned at time of task creation or
     *  whether the task data was directly referenced per task creation instructions.
     *  Intended for diagnostic purposes
     */
    bool HadAssumedOwnershipOfTaskData( void ) const;

    /**
     *  The numerical process-global-unique id that was assigned to the task at creation time
     */
    TIntegerTypeUsedForTaskId GetTaskId( void ) const;

    /**
     *  Returns a snapshot in time status of the task
     *  Expect this to be dynamically updated as the task is executing
     */
    TTaskStatus GetTaskStatus( void ) const;

    /**
     *  Helper function which does the same thing as GetTaskStatus() except adds a string conversion
     */
    CString GetTaskStatusString( void ) const;

    /**
     *  Extra info associated with the current task status
     *  This can be used to provide human readable context on errors or other status changes
     *  Usually set by the task consumer executing the task for diagnostic purposes when entering terminal states
     */
    void SetTaskStatusExtraInfo( const CString& extraInfo );

    /**
     *  Extra info associated with the current task status
     *  This can be used to provide human readable context on errors or other status changes
     *  Usually set by the task consumer executing the task for diagnostic purposes when entering terminal states
     */
    CString GetTaskStatusExtraInfo( void ) const;

    /**
     *  If the task with the given id was provided with any 'work' data AND said data is serializable this can be used to 
     *  obtain a copy of said data. Not all work data is serializable. 
     */
    bool GetSerializedTaskDataCopy( CDataNode& domNode                                      ,
                                    const CDataNodeSerializableSettings& serializerSettings ) const;

    /**
     *  Helper function to get the thread id of the delegator object which is driving the associated task consumer if any
     *  Note that this will be a snapshot in time only. At any moment the task could start or complete changing the thread id
     *  from 0 to something and back. Hence the main utilization of this functionality is just for human consumption, display-only.
     *  It is the operating system's id of the thread (MT::ThreadID()), not the MT::GetCurrentThreadID() identity.
     */
    TSystemThreadId GetThreadId( void ) const;

    bool IsTaskInEndState( void ) const;

    bool IsTaskInErrorState( void ) const;

    bool IsTaskPartOfAChain( void ) const;

    /**
     *  Returns whether the task is the last task, if part of a chain the last in said chain.
     *  Otherwise always true since a task that is not part of a chain is by definition the last task in its own chain of length 1.
     */
    bool IsLastTaskInAChain( void ) const;

    /**
     *  Utility function to have the caller thread wait for the task status to reach an end state
     */
    MT::TLockStatus WaitForTaskToFinish( Int32 timeoutInMs ) const;

    /**
     *  Allows the requester to signal that they wish to cancel the task (chain?)
     *  Note that cancellation is not guaranteed, it depends on the task implementation
     *  and the state of the task at the time of the cancellation request
     */
    bool RequestCancellation( void ) const;

    /**
     *  Marks the task as one for which cancellation was requested. A task thread which ends the task checks this to
     *  end it as cancelled instead of stopped. Cancellation requests made through the thread pool set this,
     *  there is normally no need to call it directly.
     */
    void SetCancellationRequested( bool cancellationRequested );

    bool IsCancellationRequested( void ) const;

    /**
     *  Operator to allow comparison of tasks based on their unique task id
     */
    bool operator==( const CTask& other ) const;

    /**
     *  Operator to allow comparison of tasks based on their unique task id
     */
    bool operator!=( const CTask& other ) const;

    /**
     *  Operator to allow ordering of tasks based on their unique task id
     */
    bool operator<( const CTask& other ) const;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CTaskDelegator;
    friend class CSingleTaskDelegator;
    friend class CTaskConsumer;
    friend class CThreadPool;
    friend class CTaskManager;
    friend class CTFreeList< CTask, MT::CMutex >;     /**< resets task objects for reuse through Clear() */
    friend class CASync;
    friend class CFutureResultSource;

    void SetTaskStatus( TTaskStatus newStatus );

    void SetTaskConsumer( CTaskConsumerPtr taskConsumer );

    void SetTaskData( CICloneable* taskData, bool assumeOwnershipOfTaskData );

    TTaskDelegatorBasicPtr GetDelegator( void ) const;

    bool Init( const CString& taskType         ,
               CTaskConsumerPtr taskConsumer   ,
               CICloneable* taskData           ,
               bool assumedOwnershipOfTaskData ,
               CDataNode* serializedTaskData   ,
               TTaskStatus taskStatus          ,
               ThreadPoolPtr threadPool        );

    /**
     *  Readies the task object for reuse by a next task, as the thread pool's free list does
     *  Everything of the previous use goes, including the task id: the next use gets a new one
     */
    void Clear( void );

    /**
     *  Updates the chain task ids associated with this task
     *  The order of the task ids in the vector represents the execution order of tasks in the chain
     *  All tasks in the chain must have unique ids and the current task id must be part of the provided list
     */
    bool UpdateTaskChainIds( const TTaskIdVector& taskIds );

    static bool ValidateTaskChainIdSequence( const TTaskIdVector& taskIds );

    CTaskPtr GetNextTask( void ) const;

    CTaskPtr GetPriorTask( void ) const;

    CTaskPtr GetFirstTaskInChain( void ) const;

    CTaskPtr GetFirstErrorStateTask( void ) const;

    CTaskPtr GetLastTaskInChain( void ) const;

    bool GetAllTasksInChain( TTaskPtrSet& taskSet ) const;

    bool GetAllTasksInChain( TTaskPtrVector& taskSet ) const;

    bool GetAllUpcomingTasksInChain( TTaskPtrSet& taskSet ) const;

    protected:

    /**
     *  Copy constructor intended for use by derived classes only in legacy allocator scenarios
     */
    CTask( const CTask& src );

    private:

    CTask& operator=( const CTask& src ) GUCEF_DELETED_MEMBER;  /**< copying doesnt make sense */

    private:

    /** Releases what one use of the task holds, shared by Clear() and the destructor */
    void ClearUseState( void );

    /**
     *  Monotonic time (CSystemMonotonicClock, ms) at which the task was queued and at which a thread started it, set by
     *  the thread pool at those transitions; 0 = not (yet). Relaxed atomic stores: the task info reads them from
     *  other threads
     */
    void SetQueuedAtMs( UInt64 queuedAtMs );
    UInt64 GetQueuedAtMs( void ) const;
    void SetStartedAtMs( UInt64 startedAtMs );
    UInt64 GetStartedAtMs( void ) const;

    /**
     *  The run of a scheduled job (CJobScheduler) that this task carries out, set by the thread pool before the task
     *  is queued (CThreadPool::QueueScheduledRunTask()). The link is told when the task ends
     */
    void SetScheduledRun( const CJobSchedulerRunLinkPtr& runLink, const CString& jobId, UInt64 runId );
    bool GetScheduledRun( CJobSchedulerRunLinkPtr& runLink, CString& jobId, UInt64& runId ) const;
    bool IsScheduledRun( void ) const;

    private:

    CICloneable* m_taskData;
    CDataNode m_serializedTaskData; /**< Serialized task data if any, used for serialization of task data to DOM */
    CString m_taskType;
    CThreadPoolPtr m_threadPool;
    CTaskConsumerPtr m_taskConsumer;
    TTaskId m_taskId;
    bool m_assumedOwnershipOfTaskData;  
    bool m_cancellationRequested;
    TTaskStatus m_taskStatus;
    CString m_taskStatusExtraInfo;
    TTaskIdVector m_chainTasks;
    MT::CTAtomic< UInt64 > m_queuedAtMs;
    MT::CTAtomic< UInt64 > m_startedAtMs;
    CJobSchedulerRunLinkPtr m_runLink;      /**< set for a run of a scheduled job only */
    CString m_scheduledJobId;
    UInt64 m_scheduledRunId;
    MT::CMutex m_lock;
};

typedef CTask::CTaskPtr     CTaskPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASK_H ? */
