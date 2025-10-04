/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2008.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#define GUCEF_CORE_CTHREADPOOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTABSTRACTFACTORY_H
#include "CTAbstractFactory.h"
#define GUCEF_CORE_CTABSTRACTFACTORY_H
#endif /* GUCEF_CORE_CTABSTRACTFACTORY_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H
#include "gucefCORE_CGenericCallbackTaskConsumer.h"
#define GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H
#endif /* GUCEF_CORE_CGENERICCALLBACKTASKCONSUMER_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

#ifndef GUCEF_CORE_CTHREADPOOLINFO_H
#include "gucefCORE_CThreadPoollnfo.h"
#define GUCEF_CORE_CTHREADPOOLINFO_H
#endif /* GUCEF_CORE_CTHREADPOOLINFO_H ? */

#ifndef GUCEF_CORE_CTHREADINFO_H
#include "gucefCORE_CThreadlnfo.h"
#define GUCEF_CORE_CTHREADINFO_H
#endif /* GUCEF_CORE_CTHREADINFO_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

#ifndef GUCEF_CORE_CTASKINFO_H
#include "gucefCORE_CTaskInfo.h"
#define GUCEF_CORE_CTASKINFO_H
#endif /* GUCEF_CORE_CTASKINFO_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CDEFERREDTASK_H
#include "gucefCORE_CDeferredTask.h"
#define GUCEF_CORE_CDEFERREDTASK_H
#endif /* GUCEF_CORE_CDEFERREDTASK_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

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

class CASync;
class CTaskDelegator;
class CSingleTaskDelegator;

/*-------------------------------------------------------------------------*/

/**
 *  The thread pool provides centralized management of tasks and more importantly
 *  the threads that execute those tasks. A task is defined as the logic you wish to
 *  have executed in a separate thread. Instead of creating the thread yourself you
 *  create a task consumer and then decide when your task needs to be executed. The
 *  thread pool will deal with threads themselves. This separation of concerns allows
 *  for optimal thread reuse and scheduling. 
 *  Threads have a global effect on your process and as such should be tracked globally 
 *  within your process which is what the task manager does. To enforce this only the 
 *  task manager is allowed to make thread pools
 */
class GUCEF_CORE_PUBLIC_CPP CThreadPool : public CTSGNotifier ,
                                          public CTSharedPtrCreator< CThreadPool, MT::CMutex >
{
    public:

    static const CString ClassTypeName;

    static const CEvent ThreadKilledEvent;
    static const CEvent ThreadStartedEvent;
    static const CEvent ThreadPausedEvent;
    static const CEvent ThreadResumedEvent;
    static const CEvent ThreadFinishedEvent;

    static const CEvent TaskQueuedEvent;
    static const CEvent TaskStartupEvent;
    static const CEvent TaskStartedEvent;
    static const CEvent TaskStartupFailedEvent;
    static const CEvent TaskKilledEvent;
    static const CEvent TaskStoppedEvent;
    static const CEvent TaskPausedEvent;
    static const CEvent TaskResumedEvent;
    static const CEvent TaskFinishedEvent;

    static void RegisterEvents( void );

    public:
    
    typedef typename CTSharedPtrCreator< CThreadPool, MT::CMutex >::TBasicSharedPtrType     ThreadPoolPtr;
    typedef typename CTSharedPtrCreator< CThreadPool, MT::CMutex >::TSharedPtrType          ThreadPoolTypedPtr;
    typedef CTask::TIntegerTypeUsedForTaskId                                                TIntegerTypeUsedForTaskId;
    typedef CTFactoryBase< CTaskConsumer, MT::CMutex >                                      TTaskConsumerFactory;
    typedef CTFactoryBase< CIDataNodeSerializableTaskData, MT::CMutex >                     TTaskDataFactory;
    typedef GUCEF::map< UInt32, CTaskInfo >                                                 TTaskInfoMap;
    typedef GUCEF::map< UInt32, CThreadInfo >                                               TThreadInfoMap;
    typedef GUCEF::vector< TIntegerTypeUsedForTaskId >                                      TTaskIdVector;
    typedef GUCEF::vector< UInt32 >                                                         TThreadIdVector;

    typedef TCloneableUInt32                                    TThreadKilledEventData;
    typedef TCloneableUInt32                                    TThreadStartedEventData;
    typedef TCloneableUInt32                                    TThreadPausedEventData;
    typedef TCloneableUInt32                                    TThreadResumedEventData;
    typedef TCloneableUInt32                                    TThreadFinishedEventData;
    typedef TCloneableUInt32                                    TThreadUpdateEventData;
    
    typedef TCloneableUInt32                                    TTaskQueuedEventData;
    typedef TCloneableUInt32                                    TTaskStartupEventData;
    typedef TCloneableUInt32                                    TTaskStartedEventData;
    typedef TCloneableUInt32                                    TTaskStartupFailedEventData;
    typedef TCloneableUInt32                                    TTaskKilledEventData;
    typedef TCloneableUInt32                                    TTaskStoppedEventData;
    typedef TCloneableUInt32                                    TTaskPausedEventData;
    typedef TCloneableUInt32                                    TTaskResumedEventData;
    typedef TCloneableUInt32                                    TTaskFinishedEventData;
    typedef TCloneableUInt32                                    TTaskUpdateEventData;

    /**
     *  Queues a task for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param assumeOwnershipOfTaskData    Whether the taskData given (if any) needs a private copy 
     *                                      or whether the task manager can assume ownership.
     *                                      Note that task data ownership is only transferred in the event of a successful queueing of the task
     *                                      In the case of an error the caller retains ownership at all times, facilitating a retry.
     */
    CFutureResult QueueTask( const CString& taskType                ,
                             CICloneable* taskData = GUCEF_NULL     ,
                             CObserver* taskObserver = GUCEF_NULL   ,
                             bool assumeOwnershipOfTaskData = false );

    /**
     *  Queues a task, in the form of a function callback, for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param f    The callback function pointer                                  
     */    
    template< typename R >
    CFutureResult QueueCallbackTask( R (*f)() );

    /**
     *  Queues a task, in the form of a function callback, for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation   
     */    
    template< typename R, typename A1 >
    CFutureResult QueueCallbackTask( R (*f)(A1), A1 a1 );

    /**
     *  Queues a task, in the form of a function callback, for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation
     *  @param a2   param 2 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A1, typename A2 >
    CFutureResult QueueCallbackTask( R (*f)(A1, A2), A1 a1, A2 a2 );

    /**
     *  Queues a task, in the form of a function callback, for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation
     *  @param a2   param 2 to be given to the function at time of deferred invocation
     *  @param a3   param 3 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A1, typename A2, typename A3 >
    CFutureResult QueueCallbackTask( R (*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 );

    /**
     *  Queues a task, in the form of a function callback, for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param f    The callback function pointer
     *  @param a1   param 1 to be given to the function at time of deferred invocation
     *  @param a2   param 2 to be given to the function at time of deferred invocation
     *  @param a3   param 3 to be given to the function at time of deferred invocation
     *  @param a4   param 4 to be given to the function at time of deferred invocation
     */ 
    template< typename R, typename A1, typename A2, typename A3, typename A4 >
    CFutureResult QueueCallbackTask( R (*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 );

    /**
     *  Queues a task as the next task to execute after a given task, in the form of a function callback.
     *  Queueing work in this manner allows for multiplexing the work with other work since the follow-up work will go to the back of the line again in the queue
     *  once the pre-req task has been completed, it will subsequently be picked up for execution, as soon as a thread is available per queue order,
     *  to execute it.
     * 
     *  Note that the completed prerequisite task is a mandatory first parameter input to the callback
     *  Also note that until all tasks in a task chain have reached completion all those tasks are considered 'in-use' since they
     *  are part of a greater whole
     *
     *  @param prereqTask       The prerequisite task who's completion will trigger the task formulated with this call
     *  @param f                The callback function pointer                                  
     */    
    template< typename R >
    CFutureResult QueueNextCallbackTask( CTaskPtr prereqTask, R (*f)( CTaskPtr ) );

    /**
     *  Immediately starts executing a task using the task
     *  information provided. Based on the provided information
     *  a task consumer will be constructed to actually carry out the task
     */
    CFutureResult StartTask( const CString& taskType                ,
                             CICloneable* taskData = GUCEF_NULL     ,
                             bool assumeOwnershipOfTaskData = false );

    /**
     *  Checks if a task of the given type already exists, if yes nothing new happens
     *  If no a new task will be started right away
     */
    CFutureResult StartTaskIfNoneExists( const CString& taskType                ,
                                         CICloneable* taskData = GUCEF_NULL     ,
                                         bool assumeOwnershipOfTaskData = false );

    /**
     *  Checks if a task of the given type already exists, if yes nothing new happens
     *  If no a new task will be started right away
     */
    CFutureResult StartTaskIfNoneExists( const CString& taskType   ,
                                         const CDataNode& taskData );

    /**
     *  Immediately starts executing a task using the task consumer provided.
     *
     *  Note that any task that was setup using SetupTask() still requires to be started via a call to
     *  StartTask()
     */
    CFutureResult StartTaskWithConsumer( CTaskConsumerPtr task                  ,
                                         CICloneable* taskData = GUCEF_NULL     ,
                                         bool assumeOwnershipOfTaskData = false ); 
    
    /**
     *  Same as other StartTask() variant except it will construct task data from the given DOM
     *  This requires a task data factory to be registered to construct a relevant data object
     */
    CFutureResult StartTask( const CString& taskType                ,
                             const CDataNode& taskData              ,
                             bool assumeOwnershipOfTaskData = false );

    /**
     *  Performs setup for a singular task (thread association) but does not start the task yet
     *  starts executing the task via an additional call to StartTask()
     *
     *  A singular task is a task where the task consumer is not expected to be reused for other tasks using a queueing mechanism.
     *  Its intended for tasks which are of a more singular and persistent nature, in essence representing a thread that is dedicated to a specific task.
     *  Albeit without directly interacting with the thread and allowing the thread pool to trace the thread lifecycle.
     *
     *  If you have an interdependency between threads and their associated tasks you 
     *  might need the thread to already be available and linked to the consumer without actually
     *  starting the consumer just to perform initial setup between said tasks.
     *  A typical case is needing a PulseGenerator during setup, which is provided by the to-be-associated 
     *  delegator.
     *
     *  This functionality allows you to break out thread association into an independent step
     */
    CFutureResult SetupSingularTask( CTaskConsumerPtr taskConsumer          ,
                                     CICloneable* taskData = GUCEF_NULL     ,
                                     bool assumeOwnershipOfTaskData = false );

    bool PauseTask( const UInt32 taskID                 ,
                    const bool force                    ,
                    const bool okIfTaskIsUnknown = true );

    bool ResumeTask( const UInt32 taskID                  ,
                     const bool okIfTaskIsUnknown = false );

    bool RequestTaskToStop( const UInt32 taskId           , 
                            bool callerShouldWait         ,
                            bool okIfTaskIsUnknown = true );

    bool RequestTaskToStop( CTaskConsumerPtr taskConsumer ,
                            bool callerShouldWait         );

    bool WaitForTaskToFinish( const UInt32 taskId, Int32 timeoutInMs );

    bool WaitForTaskToFinish( CTaskConsumerPtr taskConsumer, Int32 timeoutInMs );

    bool WaitForAllTasksToFinish( Int32 timeoutInMs );

    bool WaitForThreadToFinish( const UInt32 threadId, Int32 timeoutInMs );

    bool WaitForAllThreadsToFinish( Int32 timeoutInMs );

    bool KillThreadWorkingOnTask( const UInt32 taskID );

    void SetNrOfWorkerThreadsToLogicalCPUs( const UInt32 coreFactor );

    void SetDesiredMaxTotalNrOfThreads( const Int32 nrOfThreads );

    Int32 GetDesiredMaxTotalNrOfThreads( void ) const;

    void SetDesiredMinNrOfWorkerThreads( const UInt32 nrOfThreads );

    UInt32 GetDesiredMinNrOfWorkerThreads( void ) const;

    UInt32 GetActiveNrOfThreads( void ) const;

    UInt32 GetActiveNrOfDedicatedThreads( void ) const;

    UInt32 GetActiveNrOfWorkerThreads( void ) const;

    UInt32 GetNrOfQueuedTasks( void ) const;

    /**
     *  Obtains a list of all task ids for all tasks that are currently actively associated with the thread pool
     *  Note that this is a snapshot in time and the task ids can change at any time
     */
    void GetAllTaskIdsForTasksWithWork( TTaskIdVector& taskIds, bool includeQueued ) const;

    /**
     *  Obtains a list of all thread ids that are currently actively associated with the thread pool
     *  Note that this is a snapshot in time and the thread ids can change at any time
     */
    void GetAllCurrentThreadIds( TThreadIdVector& threadIds ) const;

    /**
     *  Requests all tasks to stop and optionally waits for them to finish up to the specified timeout
     *  Note that if you do not specify to no longer accept new work the thread pool will continue to accept new work potentially
     *  and as such the 'stop' would only apply to the tasks known at the time of the beginning of the call
     *  Note that you cannot perform a waitOnStop if the calling thread is one of the threads being stopped, that is disallowed to prevent deadlocks
     *  Returns false if a timeout occurred
     */
    bool RequestAllTasksToStop( bool waitOnStop, bool acceptNewWork, Int32 timeoutInMsPerTask );

    /**
     *  Requests all threads to stop and optionally waits for them to finish up to the specified timeout
     *  Note that if you do not specify to no longer accept new work the thread pool will continue to accept new work potentially
     *  and as such the 'stop' would only apply to the threads known at the time of the beginning of the call
     *  Note that you cannot perform a waitOnStop if the calling thread is one of the threads being stopped, that is disallowed to prevent deadlocks
     *  Returns false if a timeout occurred
     */
    bool RequestAllThreadsToStop( bool waitOnStop, bool acceptNewWork, Int32 timeoutInMsPerThread );

    void SetAllowMainApplicationThreadToPickUpQueuedTasks( bool allowAppThreadToWork );

    bool GetAllowMainApplicationThreadToPickUpQueuedTasks( void ) const;

    /**
     *  Instead of threads managed by the thread pools always doing all the work you can also use this member
     *  function to 'pitch in' and help out in consuming work from some other calling thread.
     *  Do note that this will block the calling thread for the duration of the queued task so use this wisely.
     *
     *  @param maxTasks the max nr of tasks you wish to consume and handle using the calling thread which will be blocked for the entire duration
     *  @return returns the number of tasks that were carried which should be between 0-maxTasks
     */
    UInt32 CarryOutQueuedTasksIfAny( UInt32 maxTasks );

    void RegisterTaskConsumerFactory( const CString& taskType       ,
                                      TTaskConsumerFactory* factory );

    void UnregisterTaskConsumerFactory( const CString& taskType );

    void GetAllRegisteredTaskConsumerFactoryTypes( CString::StringSet& taskTypes ) const;

    void RegisterTaskDataFactory( const CString& taskType   ,
                                  TTaskDataFactory* factory );

    void UnregisterTaskDataFactory( const CString& taskType );

    bool IsTaskOfTaskTypeExecutable( const CString& taskType ) const;
    
    bool IsCustomTaskDataForTaskTypeSerializable( const CString& taskType ) const;

    CIDataNodeSerializableTaskDataBasicPtr CreateCustomTaskDataForTaskTypeIfAvailable( const CString& taskType ) const;
    
    void GetAllRegisteredTaskDataFactoryTypes( CString::StringSet& taskTypes ) const;

    bool TaskOfTypeExists( const CString& taskType               ,
                           UInt32* taskIdIfExists = GUCEF_NULL   , 
                           UInt32* threadIdIfExists = GUCEF_NULL ,
                           TTaskStatus* taskStatus = GUCEF_NULL  ) const;
    
    virtual const CString& GetClassTypeName( void ) const  GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Obtains a snapshot in time of which task which what id is being processed by a thread with the given thread id
     *  Note that since this is information that can change at any time this is for informational/debugging (display?) purposes only
     *
     *  @param taskId set to 0 by default/failure and set to the relevant task id on success
     */
    bool GetTaskIdForThreadId( const UInt32 threadId ,
                               UInt32& taskId        ) const;

    /**
     *  Obtains a snapshot in time of which thread with a given thread id is processing a task with the given task id, if any
     *  Note that since this is information that can change at any time this is for informational/debugging (display?) purposes only
     *
     *  @param threadId set to 0 by default/failure and set to the relevant thread id on success 
     */
    bool GetThreadIdForTaskId( const UInt32 taskId ,
                               UInt32& threadId    ) const;    
    
    /**
     *  Obtains a coherent snapshot of thread pool information in a single lock protected call
     */
    bool GetInfo( CThreadPoolInfo& info ) const;

    /**
     *  Obtains a coherent snapshot of task information in a single lock protected call
     */
    bool GetTaskInfo( UInt32 taskId                                                                , 
                      CTaskInfo& info                                                              ,  
                      bool obtainTaskDataCopyIfPossible                                            ,
                      const CDataNodeSerializableSettings* taskDataSerializerSettings = GUCEF_NULL ) const;

    /**
     *  Obtains a coherent snapshot of all task information in a single lock protected call
     */
    bool GetAllTaskInfo( TTaskInfoMap& info                                                           ,
                         bool obtainTaskDataCopyIfPossible                                            ,
                         const CDataNodeSerializableSettings* taskDataSerializerSettings = GUCEF_NULL ) const;

    /**
     *  Obtains a coherent snapshot of thread information in a single lock protected call
     */
    bool GetThreadInfo( UInt32 threadId, CThreadInfo& info ) const;

    /**
     *  Obtains a coherent snapshot of all thread information in a single lock protected call
     */
    bool GetAllThreadInfo( TThreadInfoMap& info ) const;

    /**
     *  If the task with the given id was provided with any 'work' data and said data is serializable this can be used to 
     *  obtain a copy of said data
     */
    bool GetSerializedTaskDataCopy( UInt32 taskId                                           ,
                                    CDataNode& domNode                                      ,
                                    const CDataNodeSerializableSettings& serializerSettings ) const;

    const CString& GetThreadPoolName( void ) const;
    
    virtual ~CThreadPool() GUCEF_VIRTUAL_OVERRIDE;
    
    protected:

    virtual void OnPumpedNotify( CNotifier* notifier           ,
                                 const CEvent& eventid         ,
                                 CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CTaskDelegator;

    bool GetQueuedTask( CTaskPtr& task );

    void TaskCleanup( CTaskPtr task );

    void OnTaskStartup( CTaskPtr task );
    void OnTaskStarted( CTaskPtr task );
    void OnTaskStartupFailed( CTaskPtr task );
    void OnTaskKilled( CTaskPtr task );
    void OnTaskStopped( CTaskPtr task );
    void OnTaskPaused( CTaskPtr task );
    void OnTaskResumed( CTaskPtr task );
    void OnTaskFinished( CTaskPtr task );

    private:
    friend class CASync;

    CTaskPtr PrepTaskObj( const CString& taskType                ,
                          CICloneable* taskData = GUCEF_NULL     ,
                          CObserver* taskObserver = GUCEF_NULL   ,
                          bool assumeOwnershipOfTaskData = false );

    CFutureResult StartTask( CTaskPtr task );

    CFutureResult QueueTask( CTaskPtr task );

    private:
    friend class CTaskManager;

    CThreadPool( PulseGeneratorPtr threadPoolPulseContext , 
                 const CString& poolName                  );

    private:

    typedef CTAbstractFactory< CString, CTaskConsumer, MT::CMutex > TAbstractTaskConsumerFactory;
    typedef CTAbstractFactory< CString, CIDataNodeSerializableTaskData, MT::CMutex > TAbstractTaskDataFactory;
    typedef MT::CTMailBox< CTaskPtr > TTaskMailbox;
    typedef GUCEF::map< TIntegerTypeUsedForTaskId, CTaskConsumerPtr > TTaskConsumerMap;
    typedef CTBasicSharedPtr< CTaskDelegator, MT::CMutex >  TTaskDelegatorBasicPtr; 
    typedef GUCEF::set< TTaskDelegatorBasicPtr > TTaskDelegatorSet;
    typedef std::pair< const TIntegerTypeUsedForTaskId, CTaskPtr > TTaskIdIntAndTaskPtrPair;
    typedef GUCEF::map< TIntegerTypeUsedForTaskId, CTaskPtr >    TTaskId2TaskPtrMap;
    typedef std::deque< CTaskPtr > TTaskPtrDequeue;
    typedef GUCEF::set< CTaskConsumerPtr > CTaskConsumerPtrSet;
    typedef std::pair< const CString, CTaskConsumerPtrSet > TStringAndTaskConsumerSetPair;
    typedef GUCEF::map< CString, CTaskConsumerPtrSet > TStringToTaskConsumerSetMap;

    void EnforceDesiredNrOfThreads( Int32 desiredMaxTotalNrOfThreads   ,
                                    UInt32 desiredMinNrOfWorkerThreads ,
                                    bool gracefullEnforcement          );

    /**
     *  For task consumers that are not owned by the thread pool there is the issue of an independent lifecycle
     *  of the task consumer object which may invalidate references to it.
     *  As such this function is provided to eliminate any such references
     *
     *  If the consumer has an in-progress task that task will be aborted/erased
     */
    void RemoveConsumer( const CTaskConsumer* taskConsumer );

    void RemoveDelegator( CNotifier* notifier );

    TTaskDelegatorBasicPtr GetDelegatorForThreadId( const UInt32 threadId ) const;

    void SubscribeToTaskConsumerEvents( CTaskConsumerPtr& taskConsumer );

    bool GetOrCreateTaskConsumerOfType( const CString& taskConsumerType ,
                                        CTaskConsumerPtr& taskConsumer  );

    bool GetOrCreateTaskObj( CTaskPtr& outTaskObj );

    TTaskDelegatorBasicPtr GetDelegatorForTaskId( TIntegerTypeUsedForTaskId taskID ,
                                                  bool& taskExists                 ) const;

    CFutureResult SetupSingularTaskImpl( CTaskPtr task );

    CFutureResult SetupSingularTaskImpl( CTaskConsumerPtr taskConsumer  ,
                                         CICloneable* taskData          ,
                                         bool assumeOwnershipOfTaskData ,
                                         CDataNode* taskDataDom         ,
                                         bool consumerWasCreatedByPool  );

    CThreadPool( const CThreadPool& src );                   /**< not supported */
    CThreadPool& operator=( const CThreadPool& src );        /**< not supported */

    private:

    CString m_poolName;
    TAbstractTaskConsumerFactory m_consumerFactory;
    TAbstractTaskDataFactory m_taskDataFactory;
    Int32 m_desiredMaxTotalNrOfThreads;
    UInt32 m_desiredMinNrOfWorkerThreads;
    TTaskMailbox m_taskQueue;
    TTaskId2TaskPtrMap m_inUseTaskObjs;
    TTaskPtrDequeue m_freeTaskObjs;
    TStringToTaskConsumerSetMap m_freeTaskConsumers;
    TTaskDelegatorSet m_taskDedicatedDelegators;
    TTaskDelegatorSet m_taskGenericDelegators;    
    bool m_acceptNewWork;
    bool m_allowAppThreadToWork;
};

/*-------------------------------------------------------------------------*/

typedef CThreadPool::ThreadPoolPtr          ThreadPoolPtr;
typedef CThreadPool::ThreadPoolTypedPtr     ThreadPoolTypedPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename R >
CFutureResult
CThreadPool::QueueCallbackTask( R (*f)() )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;

    CFutureResult future = QueueTask( CGenericCallbackTaskConsumer::TaskType ,
                                      taskData                               ,
                                      GUCEF_NULL                             ,
                                      true                                   );
    return future;
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1 >
CFutureResult
CThreadPool::QueueCallbackTask( R (*f)(A1), A1 a1 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;

    CFutureResult future = QueueTask( CGenericCallbackTaskConsumer::TaskType ,
                                      taskData                               ,
                                      GUCEF_NULL                             ,
                                      true                                   );
    return future;
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2 >
CFutureResult
CThreadPool::QueueCallbackTask( R (*f)(A1, A2), A1 a1, A2 a2 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1, a2 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;

    CFutureResult future = QueueTask( CGenericCallbackTaskConsumer::TaskType ,
                                      taskData                               ,
                                      GUCEF_NULL                             ,
                                      true                                   );
    return future;
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3 >
CFutureResult
CThreadPool::QueueCallbackTask( R (*f)(A1, A2, A3), A1 a1, A2 a2, A3 a3 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1, a2, a3 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;

    CFutureResult future = QueueTask( CGenericCallbackTaskConsumer::TaskType ,
                                      taskData                               ,
                                      GUCEF_NULL                             ,
                                      true                                   );
    return future;
}

/*-------------------------------------------------------------------------*/

template< typename R, typename A1, typename A2, typename A3, typename A4 >
CFutureResult
CThreadPool::QueueCallbackTask( R (*f)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4 )
{GUCEF_TRACE;

    CICloneable* taskData = CDeferredTask::Construct( f, a1, a2, a3, a4 );
    if GUCEF_PREDICT_FALSE( GUCEF_NULL == taskData )
        return TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED;

    CFutureResult future = QueueTask( CGenericCallbackTaskConsumer::TaskType ,
                                      taskData                               ,
                                      GUCEF_NULL                             ,
                                      true                                   );
    return future;
}

/*-------------------------------------------------------------------------*/

template< typename R >
CFutureResult
CThreadPool::QueueNextCallbackTask( CTaskPtr prereqTask, R (*f)( CTaskPtr ) )
{GUCEF_TRACE;

    if ( prereqTask.IsNULL() )
        return TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE;

    CTaskPtr task = PrepTaskObj( CGenericCallbackTaskConsumer::TaskType ,
                                 GUCEF_NULL                             ,
                                 GUCEF_NULL                             ,
                                 true                                   );
    if ( !task->IsTaskInErrorState() )
    {
        if ( prereqTask->SetNextTask( task ) )
        {
            CFutureResult future = QueueCallbackTask( f, prereqTask );
            return future;
        }
    }
    return task;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTHREADPOOL_H ? */
