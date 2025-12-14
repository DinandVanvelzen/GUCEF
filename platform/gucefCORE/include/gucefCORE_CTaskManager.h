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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#define GUCEF_CORE_CTASKMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGERINFO_H
#include "gucefCORE_CTaskManagerInfo.h"
#define GUCEF_CORE_CTASKMANAGERINFO_H
#endif /* GUCEF_CORE_CTASKMANAGERINFO_H ? */

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CIDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLETASKDATA_H ? */

#ifndef GUCEF_CORE_CASYNC_H
#include "gucefCORE_CASync.h"
#define GUCEF_CORE_CASYNC_H
#endif /* GUCEF_CORE_CASYNC_H ? */

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

/*-------------------------------------------------------------------------*/

/**
 *  The task manager provides centralized management of tasks and more importantly
 *  the threads that execute those tasks. A task is defined as the logic you wish to
 *  have executed in a worker thread. Instead of creating the thread yourself you
 *  create a task consumer and then decide when your task needs to be executed. The
 *  task manager will deal with threads themselves. This separation of concerns allows
 *  for optimal thread reuse and scheduling. Threads have a global effect on your process
 *  and as such should be tracked globally within your process which is what the task
 *  manager does.
 *  You can group your threading concerns into distinct thread pools for which the task manager
 *  will provide an aggregated view.
 */
class GUCEF_CORE_PUBLIC_CPP CTaskManager : public CTSGNotifier
{
    public:

    static const CEvent ThreadPoolCreatedEvent;
    static const CEvent ThreadPoolDestructionEvent;
    static const CEvent ThreadPoolUnregisteredEvent;
    static const CEvent GlobalTaskConsumerFactoryRegisteredEvent;
    static const CEvent GlobalTaskConsumerFactoryUnregisteredEvent;
    static const CEvent GlobalTaskDataFactoryRegisteredEvent;
    static const CEvent GlobalTaskDataFactoryUnregisteredEvent;

    typedef TCloneableString    ThreadPoolCreatedEventData;                           /**< name of the thread pool that is created */
    typedef TCloneableString    ThreadPoolDestructionEventData;                       /**< name of the thread pool that is being destroyed */
    typedef TCloneableString    ThreadPoolUnregisteredEventData;                      /**< name of the thread pool that is unregistered */
    typedef TCloneableString    GlobalTaskConsumerFactoryRegisteredEventData;         /**< name of the task consumer factory which is newly registered */
    typedef TCloneableString    GlobalTaskConsumerFactoryUnregisteredEventData;       /**< name of the task consumer factory which is no longer registered */
    typedef TCloneableString    GlobalTaskDataFactoryRegisteredEventData;             /**< name of the task data factory which is newly registered */
    typedef TCloneableString    GlobalTaskDataFactoryUnregisteredEventData;           /**< name of the task data factory which is no longer registered */

    static void RegisterEvents( void );

    public:

    typedef CThreadPool::TTaskConsumerFactory               TTaskConsumerFactory;
    typedef CThreadPool::TTaskDataFactory                   TTaskDataFactory;
    typedef CThreadPool::TTaskInfoMap                       TTaskInfoMap;
    typedef CThreadPool::TThreadInfoMap                     TThreadInfoMap;
    typedef GUCEF::map< CString, CThreadPoolInfo >          TThreadPoolInfoMap;
    typedef GUCEF::vector< ThreadPoolPtr >                  ThreadPoolVector;

    static const CString ClassTypeName;
    static const CString DefaultThreadPoolName; 

    ThreadPoolPtr GetThreadPool( const CString& threadPoolName = DefaultThreadPoolName ) const;

    ThreadPoolPtr GetOrCreateThreadPool( const CString& threadPoolName            , 
                                         PulseGeneratorPtr threadPoolPulseContext ,
                                         bool createIfNotExists = true            );

    ThreadPoolPtr GetOrCreateThreadPool( const CString& threadPoolName , 
                                         bool createIfNotExists = true );

    void GetAllThreadPools( ThreadPoolVector& threadPools ) const;

    bool UnregisterThreadPool( const CString& threadPoolName );

    bool UnregisterThreadPool( ThreadPoolPtr threadPool );

    void UnregisterAllThreadPools( void );
    
    /**
     *  Queues a task for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param assumeOwnershipOfTaskData    Whether the taskData given (if any) needs a private copy 
     *                                      or whether the task manager can assume ownership
     */
    CFutureResult QueueTask( const CString& threadPoolName          ,
                             const CString& taskType                ,
                             CICloneable* taskData = GUCEF_NULL     ,
                             CObserver* taskObserver = GUCEF_NULL   ,
                             bool assumeOwnershipOfTaskData = false );

    /**
     *  Queues a task for execution as soon as a thread is available
     *  to execute it.
     *
     *  @param assumeOwnershipOfTaskData    Whether the taskData given (if any) needs a private copy 
     *                                      or whether the task manager can assume ownership
     */
    CFutureResult StartOrQueueTask( CIDataNodeSerializableTaskData* taskData ,
                                    CObserver* taskObserver = GUCEF_NULL     ,
                                    bool assumeOwnershipOfTaskData = false   );

    /**
     *  Attempts to distill a task description from the given data node based representation
     *  
     */
    CFutureResult StartOrQueueTask( const CDataNode& taskData              ,
                                    CObserver* taskObserver = GUCEF_NULL   );

    /**
     *  Immediately starts executing a task using the task
     *  information provided. Based on the provided information
     *  a task consumer will be constructed to actually carry out the task
     */
    CFutureResult StartTask( const CString& threadPoolName      ,
                             const CString& taskType            ,
                             CICloneable* taskData = GUCEF_NULL );

    /**
     *  Checks if a task of the given type already exists, if yes nothing new happens
     *  If no a new task one would be started right away
     */
    CFutureResult StartTaskIfNoneExists( const CString& threadPoolName      ,
                                         const CString& taskType            ,
                                         CICloneable* taskData = GUCEF_NULL );

    /**
     *  Checks if a task of the given type already exists, if yes nothing new happens
     *  If no a new task one would be started right away
     */
    CFutureResult StartTaskIfNoneExists( const CString& threadPoolName ,
                                         const CString& taskType       ,
                                         const CDataNode& taskData     );

    void RegisterTaskConsumerFactory( const CString& taskType       ,
                                      TTaskConsumerFactory* factory );

    void UnregisterTaskConsumerFactory( const CString& taskType );

    void UnregisterAllTaskConsumerFactories( void );

    void GetAllRegisteredTaskConsumerFactoryTypes( CORE::CString::StringSet& taskTypes );

    void GetRegisteredTaskConsumerFactoryTypes( const CString& threadPoolName       ,
                                                CORE::CString::StringSet& taskTypes );

    void RegisterTaskDataFactory( const CString& taskType   ,
                                  TTaskDataFactory* factory );

    void UnregisterTaskDataFactory( const CString& taskType );

    void UnregisterAllTaskDataFactories( void );

    bool IsTaskOfTaskTypeExecutable( const CString& taskType, const CString& threadPoolName = CString::Empty ) const;
    
    bool IsCustomTaskDataForTaskTypeSerializable( const CString& taskType, const CString& threadPoolName = CString::Empty ) const;

    CIDataNodeSerializableTaskDataBasicPtr CreateCustomTaskDataForTaskTypeIfAvailable( const CString& taskType, const CString& threadPoolName = CString::Empty ) const;
    
    UInt32 GetGlobalNrOfActiveThreads( void ) const;

    UInt32 GetGlobalMinNrOfDesiredWorkerThreads( void ) const;

    UInt32 GetGlobalNrOfQueuedTasks( void ) const;

    bool GetTaskTotals( UInt32& nrOfInUseTasks   ,
                        UInt32& nrOfActiveTasks  ,
                        UInt32& nrOfDormantTasks ,
                        UInt32& nrOfFreeTaskObjs ) const;

    /**
     *  Requests all threads to stop in all thread pools and optionally waits for them to finish up to the specified timeout
     *  Note that if you do not specify to no longer accept new work the threadpool will continue to accept new work potentially
     *  and as such the 'stop' would only apply to the threads known at the time of the beginning of the call
     *  Note that you cannot perform a waitOnStop if the calling thread is one of the threads being stopped, that is disalllowed to prevent deadlocks
     *  Returns false if a timeout occured
     */
    bool RequestAllThreadsToStop( bool waitOnStop, bool acceptNewWork, Int32 timeoutInMsPerThread );

    void GetAllThreadPoolNames( CORE::CString::StringSet& poolNames ) const;

    virtual const CString& GetClassTypeName( void ) const  GUCEF_VIRTUAL_OVERRIDE;

    CTaskConsumerPtr CreateTaskConsumer( const CString& taskType );

    /**
     *  Obtains a coherent snapshot of task manager information in a single lock protected call
     */
    bool GetInfo( CTaskManagerInfo& info ) const;

    /**
     *  Obtains a coherent snapshot of task information in a single lock protected call
     */
    bool GetTaskInfo( UInt32 taskId                                                          , 
                      CTaskInfo& info                                                        , 
                      CString& threadPoolName                                                ,
                      bool obtainTaskDataCopyIfPossible                                      ,
                      CDataNodeSerializableSettings* taskDataSerializerSettings = GUCEF_NULL ) const;

    /**
     *  Obtains a coherent snapshot of all task information in a single lock protected call
     */
    bool GetAllTaskInfo( TTaskInfoMap& info                                                     ,
                         bool obtainTaskDataCopyIfPossible                                      ,
                         CDataNodeSerializableSettings* taskDataSerializerSettings = GUCEF_NULL ) const;

    /**
     *  Obtains a coherent snapshot of thread pool information in a single lock protected call
     */
    bool GetThreadPoolInfo( const CString& poolName, CThreadPoolInfo& info ) const;

    /**
     *  Obtains a coherent snapshot of all threadpool information in a single lock protected call
     */
    bool GetAllThreadPoolInfo( TThreadPoolInfoMap& info ) const;

    /**
     *  Obtains a coherent snapshot of thread information in a single lock protected call
     */
    bool GetThreadInfo( UInt32 threadId         , 
                        CThreadInfo& info       ,
                        CString& threadPoolName ) const;

    /**
     *  Obtains a coherent snapshot of all thread information in a single lock protected call
     */
    bool GetAllThreadInfo( TThreadInfoMap& info ) const;

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
     *  If the task with the given id was provided with any 'work' data and said data is serializable this can be used to 
     *  obtain a copy of said data
     */
    bool GetSerializedTaskDataCopy( const UInt32 taskId                                     ,
                                    CDataNode& domNode                                      ,
                                    const CDataNodeSerializableSettings& serializerSettings ) const;

    /**
     *  Gracefully shuts everything down and unregisters all that was registered
     */    
    void Shutdown( void );

    void SetTaskIdRecycleCheckThreshold( UInt32 threshold );

    UInt32 GetTaskIdRecycleCheckThreshold( void ) const;
    
    protected:

    virtual void OnPumpedNotify( CNotifier* notifier           ,
                                 const CEvent& eventid         ,
                                 CICloneable* eventdata = NULL ) GUCEF_VIRTUAL_OVERRIDE;

    void OnThreadPoolThreadStarted( CNotifier* notifier    ,
                                    const CEvent& eventId  ,
                                    CICloneable* eventData );

    void OnThreadPoolThreadKilled( CNotifier* notifier    ,
                                   const CEvent& eventId  ,
                                   CICloneable* eventData );

    void OnThreadPoolThreadFinished( CNotifier* notifier    ,
                                     const CEvent& eventId  ,
                                     CICloneable* eventData );

    void OnThreadPoolDestruction( CNotifier* notifier    ,
                                  const CEvent& eventId  ,
                                  CICloneable* eventData );

    void OnAppShutdown( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );
    
    private:
    friend class CThreadPool;
    friend class CTask;

    bool TryGetGlobalTaskId( CTask::TTaskId& taskId );

    void ReleaseGlobalTaskId( CTask::TTaskId& taskId );

    private:
    friend class CCoreGlobal;

    CTaskManager( void );

    virtual ~CTaskManager();

    private:

    void RemoveConsumer( CTaskConsumer* taskConsumer );

    void RegisterThreadPoolEventHandlers( CThreadPool* threadPool );

    void RegisterEventHandlers( void );

    CTaskManager( const CTaskManager& src );            /**< not implemented */
    CTaskManager& operator=( const CTaskManager& src ); /**< not implemented */

    private:

    typedef GUCEF::map< CString, ThreadPoolPtr > ThreadPoolMap;
    typedef CTAbstractFactory< CString, CTaskConsumer, MT::CMutex > TAbstractTaskConsumerFactory;
    typedef CTAbstractFactory< CString, CIDataNodeSerializableTaskData, MT::CMutex > TAbstractTaskDataFactory;
    typedef CTask::TTaskIdGenerator TTaskIdGenerator;
    typedef CTEventHandlerFunctor< CTaskManager > TEventCallback;

    TTaskIdGenerator::TNumericIDGeneratorTypedPtr m_taskIdGenerator;
    TAbstractTaskConsumerFactory m_consumerFactory;
    TAbstractTaskDataFactory m_taskDataFactory;
    ThreadPoolMap m_threadPools;
    UInt32 m_desiredGlobalNrOfThreads;
    Int32 m_activeGlobalNrOfThreads;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
