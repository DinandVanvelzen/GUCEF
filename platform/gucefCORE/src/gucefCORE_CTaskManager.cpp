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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */ 

#ifndef GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#include "gucefMT_CObjectScopeReadOnlyLock.h"
#define GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H
#endif /* GUCEF_MT_COBJECTSCOPEREADONLYLOCK_H ? */ 

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CSINGLETASKDELEGATOR_H
#include "gucefCORE_CSingleTaskDelegator.h"
#define GUCEF_CORE_CSINGLETASKDELEGATOR_H
#endif /* GUCEF_CORE_CSINGLETASKDELEGATOR_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSTDDATANODESERIALIZABLETASKDATA_H
#include "gucefCORE_CStdDataNodeSerializableTaskData.h"
#define GUCEF_CORE_CSTDDATANODESERIALIZABLETASKDATA_H
#endif /* GUCEF_CORE_CSTDDATANODESERIALIZABLETASKDATA_H ? */

#include "gucefCORE_CTaskManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTFactory< CTaskConsumer, CGenericCallbackTaskConsumer, MT::CMutex >     TGenericCallbackTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CEvent CTaskManager::ThreadPoolCreatedEvent = "GUCEF::CORE::CTaskManager::ThreadPoolCreatedEvent";
const CEvent CTaskManager::ThreadPoolDestructionEvent = "GUCEF::CORE::CTaskManager::ThreadPoolDestructionEvent";
const CEvent CTaskManager::ThreadPoolUnregisteredEvent = "GUCEF::CORE::CTaskManager::ThreadPoolUnregisteredEvent";
const CEvent CTaskManager::GlobalTaskConsumerFactoryRegisteredEvent = "GUCEF::CORE::CTaskManager::GlobalTaskConsumerFactoryRegisteredEvent";
const CEvent CTaskManager::GlobalTaskConsumerFactoryUnregisteredEvent = "GUCEF::CORE::CTaskManager::GlobalTaskConsumerFactoryUnregisteredEvent";
const CEvent CTaskManager::GlobalTaskDataFactoryRegisteredEvent = "GUCEF::CORE::CTaskManager::GlobalTaskDataFactoryRegisteredEvent";
const CEvent CTaskManager::GlobalTaskDataFactoryUnregisteredEvent = "GUCEF::CORE::CTaskManager::GlobalTaskDataFactoryUnregisteredEvent";

const CString CTaskManager::ClassTypeName = "GUCEF::CORE::CTaskManager";
const CString CTaskManager::DefaultThreadPoolName = "default";

TGenericCallbackTaskConsumerFactory g_genericCallbackTaskConsumerFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void 
CTaskManager::RegisterEvents( void )
{GUCEF_TRACE;

    ThreadPoolCreatedEvent.Initialize();
    ThreadPoolDestructionEvent.Initialize();
    ThreadPoolUnregisteredEvent.Initialize();
    GlobalTaskConsumerFactoryRegisteredEvent.Initialize();
    GlobalTaskConsumerFactoryUnregisteredEvent.Initialize();
    GlobalTaskDataFactoryRegisteredEvent.Initialize();
    GlobalTaskDataFactoryUnregisteredEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CTaskManager::CTaskManager( void )
    : CTSGNotifier() 
    , m_taskIdGenerator()
    , m_consumerFactory()     
    , m_taskDataFactory()
    , m_threadPools() 
    , m_desiredGlobalNrOfThreads( 0 )
    , m_activeGlobalNrOfThreads( 0 )
{GUCEF_TRACE;

    m_taskIdGenerator = TTaskIdGenerator::CreateSharedObj();

    ThreadPoolPtr defaultPool = ( GUCEF_NEW CThreadPool( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), DefaultThreadPoolName ) )->CreateSharedPtr();
    m_threadPools[ DefaultThreadPoolName ] = defaultPool;

    RegisterEventHandlers();
    RegisterThreadPoolEventHandlers( defaultPool.GetPointerAlways() );

    RegisterTaskConsumerFactory( CGenericCallbackTaskConsumer::TaskType, &g_genericCallbackTaskConsumerFactory );

    ThreadPoolCreatedEventData eData( DefaultThreadPoolName );
    NotifyObserversFromThread( ThreadPoolCreatedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

CTaskManager::~CTaskManager( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_INFINITE_LOCK_TIMEOUT );    
    UnregisterAllTaskConsumerFactories();
    UnregisterAllTaskDataFactories();
    UnregisterAllThreadPools();
    SignalUpcomingDestruction();
    lock.EarlyUnlock();
}

/*-------------------------------------------------------------------------*/

const CString&
CTaskManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::RegisterEventHandlers( void )
{GUCEF_TRACE;

    CGUCEFApplication& app = CORE::CCoreGlobal::Instance()->GetApplication();

    TEventCallback callback( this, &CTaskManager::OnAppShutdown );
    SubscribeTo( &app, CGUCEFApplication::AppShutdownEvent, callback );
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::OnAppShutdown( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::Shutdown( void )
{GUCEF_TRACE;

    RequestAllThreadsToStop( true, false, GUCEF_MT_SUPER_LONG_LOCK_TIMEOUT );
    UnregisterAllTaskDataFactories();
    UnregisterAllTaskConsumerFactories();
    UnregisterAllThreadPools();
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::RegisterThreadPoolEventHandlers( CThreadPool* threadPool )
{GUCEF_TRACE;

    TEventCallback callback( this, &CTaskManager::OnThreadPoolThreadStarted );
    SubscribeTo( threadPool, CThreadPool::ThreadStartedEvent, callback );

    TEventCallback callback2( this, &CTaskManager::OnThreadPoolThreadKilled );
    SubscribeTo( threadPool, CThreadPool::ThreadKilledEvent, callback2 ); 

    TEventCallback callback3( this, &CTaskManager::OnThreadPoolThreadFinished );
    SubscribeTo( threadPool, CThreadPool::ThreadFinishedEvent, callback3 );    

    TEventCallback callback4( this, &CTaskManager::OnThreadPoolDestruction );
    SubscribeTo( threadPool, CNotifier::DestructionEvent, callback4 );

}

/*-------------------------------------------------------------------------*/

void
CTaskManager::OnThreadPoolThreadStarted( CNotifier* notifier    ,
                                         const CEvent& eventId  ,
                                         CICloneable* eventData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );
    ++m_activeGlobalNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::OnThreadPoolThreadKilled( CNotifier* notifier    ,
                                        const CEvent& eventId  ,
                                        CICloneable* eventData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );
    --m_activeGlobalNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::OnThreadPoolThreadFinished( CNotifier* notifier    ,
                                          const CEvent& eventId  ,
                                          CICloneable* eventData )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );
    --m_activeGlobalNrOfThreads;
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::OnThreadPoolDestruction( CNotifier* notifier    ,
                                       const CEvent& eventId  ,
                                       CICloneable* eventData )
{GUCEF_TRACE;

    if ( CThreadPool::ClassTypeName == notifier->GetClassTypeName() )
    {
        CThreadPool* threadPool = static_cast< CThreadPool* >( notifier );
        CString threadPoolName = threadPool->GetThreadPoolName();

        // The threadpool should already be unregistered at this point but just in case lets make double sure
        // no-op if it was already not registered
        UnregisterThreadPool( threadPoolName );

        // Now what we are mainly after: Notify observers of the threadpool's demise by name
        ThreadPoolDestructionEventData eventData( threadPoolName );
        NotifyObserversFromThread( ThreadPoolDestructionEvent, &eventData );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::OnPumpedNotify( CNotifier* notifier    ,
                              const CEvent& eventId  ,
                              CICloneable* eventData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

ThreadPoolPtr 
CTaskManager::GetThreadPool( const CString& threadPoolName ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.find( threadPoolName );
    if ( i != m_threadPools.end() )
    {
        return (*i).second;
    }
    return ThreadPoolPtr();
}

/*-------------------------------------------------------------------------*/

ThreadPoolPtr 
CTaskManager::GetOrCreateThreadPool( const CString& threadPoolName            , 
                                     PulseGeneratorPtr threadPoolPulseContext ,
                                     bool createIfNotExists                   )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::iterator i = m_threadPools.find( threadPoolName );
    if ( i != m_threadPools.end() )
    {
        return (*i).second;
    }
    
    if ( createIfNotExists )
    {
        ThreadPoolPtr newPool = ( GUCEF_NEW CThreadPool( threadPoolPulseContext, threadPoolName ) )->CreateSharedPtr();        
        m_threadPools[ threadPoolName ] = newPool;
        lock.EarlyUnlock();

        RegisterThreadPoolEventHandlers( newPool.GetPointerAlways() );
        
        ThreadPoolCreatedEventData eData( threadPoolName );
        NotifyObserversFromThread( ThreadPoolCreatedEvent, &eData ); 
        return newPool; 
    }

    return ThreadPoolPtr();
}

/*-------------------------------------------------------------------------*/

ThreadPoolPtr 
CTaskManager::GetOrCreateThreadPool( const CString& threadPoolName , 
                                     bool createIfNotExists        )
{GUCEF_TRACE;

    return GetOrCreateThreadPool( threadPoolName, CORE::CCoreGlobal::Instance()->GetPulseGenerator(), createIfNotExists );        
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::UnregisterThreadPool( const CString& threadPoolName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.find( threadPoolName );
    if ( i != m_threadPools.end() )
    {
        m_threadPools.erase( i );
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TaskManager:UnregisterThreadPool: Thread pool with name \"" + threadPoolName + "\" has been unregistered" );

        lock.EarlyUnlock();
        
        ThreadPoolUnregisteredEventData eventData( threadPoolName );
        NotifyObserversFromThread( ThreadPoolUnregisteredEvent, &eventData );
    }
    
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::UnregisterThreadPool( ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    if ( threadPool.IsNULL() )
        return true; // nothing to do
    
    return UnregisterThreadPool( threadPool->GetThreadPoolName() );
}

/*-------------------------------------------------------------------------*/

void 
CTaskManager::UnregisterAllThreadPools( void )
{GUCEF_TRACE;

    // We dont bulk unregister because we want to provide proper notifications per entry
    
    CString::StringSet poolNames;
    GetAllThreadPoolNames( poolNames );

    CString::StringSet::iterator i = poolNames.begin();
    while ( i != poolNames.end() )
    {
        UnregisterThreadPool( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult 
CTaskManager::QueueTask( const CString& threadPoolName  ,
                         const CString& taskType        ,
                         CICloneable* taskData          ,
                         CObserver* taskObserver        ,
                         bool assumeOwnershipOfTaskData )
{GUCEF_TRACE;

    ThreadPoolPtr threadPool = GetOrCreateThreadPool( threadPoolName, false );        
    if ( threadPool.IsNULL() )
    {
        return threadPool->QueueTask( taskType                  ,
                                      taskData                  ,
                                      taskObserver              ,
                                      assumeOwnershipOfTaskData );
    }

    return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CTaskManager::StartOrQueueTask( CIDataNodeSerializableTaskData* taskData ,
                                CObserver* taskObserver                  ,
                                bool assumeOwnershipOfTaskData           )
{GUCEF_TRACE;

    if ( GUCEF_NULL == taskData )
    {
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    }
    
    const CString& threadPoolName = taskData->GetThreadPoolName();
    const CString& taskType = taskData->GetTaskTypeName();

    if ( !IsTaskOfTaskTypeExecutable( taskType, threadPoolName ) )
    {
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "TaskManager:StartOrQueueTask: Task type \"" + taskType + "\" is not an executable task type, aborting" );
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKTYPE_INVALID );
    }

    ThreadPoolPtr threadPool = GetOrCreateThreadPool( threadPoolName, !taskData->GetOnlyUseExistingThreadPool() );        
    if ( !threadPool.IsNULL() )
    {
        // If we can queue we should always queue
        // worker threads will just pick up the work as capacity becomes available
        if ( taskData->GetTaskCanBeQueued() )
        {
            return threadPool->QueueTask( taskType                  ,
                                          taskData                  ,
                                          taskObserver              ,
                                          assumeOwnershipOfTaskData );
        }
        else
        {
            return threadPool->StartTask( taskType                  ,
                                          taskData                  ,
                                          assumeOwnershipOfTaskData );
        }
    }
    else
    {
        if ( taskData->GetOnlyUseExistingThreadPool() )
            return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );
        else
            return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_LIMIT_REACHED );
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult
CTaskManager::StartOrQueueTask( const CDataNode& taskData ,
                                CObserver* taskObserver   )
{GUCEF_TRACE;

    CDataNodeSerializableSettings defaultSerializationSettings;
    CStdDataNodeSerializableTaskData* genericTaskData = GUCEF_NEW CStdDataNodeSerializableTaskData();
    if ( genericTaskData->Deserialize( taskData, defaultSerializationSettings ) )
    {
        CIDataNodeSerializableTaskDataBasicPtr taskDataObj = CreateCustomTaskDataForTaskTypeIfAvailable( genericTaskData->GetTaskTypeName() );
        if ( !taskDataObj.IsNULL() )
        {
            // Place the task data in the dedicated task data object instead of a generic one
            if ( taskDataObj->Deserialize( taskData, defaultSerializationSettings ) )
            {                
                GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TaskManager:StartOrQueueTask: Successfully deserialized custom task properties from provide DOM for task with type " + genericTaskData->GetTaskTypeName() );
                GUCEF_DELETE genericTaskData;                
                return StartOrQueueTask( taskDataObj.GetPointerAlways(), taskObserver, false );
            }
            else
            {
                GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "TaskManager:StartOrQueueTask: Unable to deserialize custom task properties from provide DOM for task with type " + genericTaskData->GetTaskTypeName() );
                GUCEF_DELETE genericTaskData;
                return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
            }
        }
        else
        {
            // No custom task data object is available, lets stick with the generic one
            // Since we allocated the generic one and its in-scope, let the task assume ownsership of the object we created
            GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "TaskManager:StartOrQueueTask: Successfully deserialized generic task properties from provide DOM for task with type " + genericTaskData->GetTaskTypeName() );
            return StartOrQueueTask( genericTaskData, taskObserver, true ); 
        }
    }
    else
    {
        GUCEF_DELETE genericTaskData;
        GUCEF_WARNING_LOG( LOGLEVEL_NORMAL, "TaskManager:StartOrQueueTask: Unable to derive standard task properties from provide DOM" ); 
        return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_TASKDATA_INVALID );
    }
}

/*-------------------------------------------------------------------------*/

CFutureResult 
CTaskManager::StartTask( const CString& threadPoolName ,
                         const CString& taskType       ,
                         CICloneable* taskData         )
{GUCEF_TRACE;

    ThreadPoolPtr threadPool = GetOrCreateThreadPool( threadPoolName, false );        
    if ( !threadPool.IsNULL() )
    {
        return threadPool->StartTask( taskType ,
                                      taskData );
    }
    return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CTaskManager::StartTaskIfNoneExists( const CString& threadPoolName ,
                                     const CString& taskType       ,
                                     CICloneable* taskData         )
{GUCEF_TRACE;

    ThreadPoolPtr threadPool = GetOrCreateThreadPool( threadPoolName, false );        
    if ( !threadPool.IsNULL() )
    {
        return threadPool->StartTaskIfNoneExists( taskType ,
                                                  taskData );
    }
    return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

CFutureResult
CTaskManager::StartTaskIfNoneExists( const CString& threadPoolName ,
                                     const CString& taskType       ,
                                     const CDataNode& taskData     )
{GUCEF_TRACE;

    ThreadPoolPtr threadPool = GetOrCreateThreadPool( threadPoolName, false );        
    if ( !threadPool.IsNULL() )
    {
        return threadPool->StartTaskIfNoneExists( taskType ,
                                                  taskData );
    }
    return CTask::CreateSharedObjWithParam( TTaskStatus::TASKSTATUS_RESOURCE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

UInt32 
CTaskManager::GetGlobalNrOfActiveThreads( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    UInt32 threadCount = 0;
    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        threadCount += (*i).second->GetActiveNrOfThreads();
        ++i;
    }
    return threadCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManager::GetGlobalMinNrOfDesiredWorkerThreads( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    UInt32 threadCount = 0;
    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        threadCount += (*i).second->GetDesiredMinNrOfWorkerThreads();
        ++i;
    }
    return threadCount;
}

/*-------------------------------------------------------------------------*/

UInt32 
CTaskManager::GetGlobalNrOfQueuedTasks( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    UInt32 taskCount = 0;
    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        taskCount += (*i).second->GetNrOfQueuedTasks();
        ++i;
    }
    return taskCount;
}

/*-------------------------------------------------------------------------*/

void 
CTaskManager::GetAllThreadPools( ThreadPoolVector& threadPools ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        threadPools.push_back( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::RequestAllThreadsToStop( bool waitOnStop, bool acceptNewWork, Int32 timeoutInMsPerThread )
{GUCEF_TRACE;

    ThreadPoolVector threadPools;
    GetAllThreadPools( threadPools );

    bool totalSuccess = true;
    ThreadPoolVector::iterator i = threadPools.begin();
    while ( i != threadPools.end() )
    {
        totalSuccess = (*i)->RequestAllThreadsToStop( waitOnStop, acceptNewWork, timeoutInMsPerThread ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::RemoveConsumer( CTaskConsumer* taskConsumer )
{GUCEF_TRACE;

    if ( GUCEF_NULL == taskConsumer )
        return; // nothing to do

    ThreadPoolVector threadPools;
    GetAllThreadPools( threadPools );

    ThreadPoolVector::iterator i = threadPools.begin();
    while ( i != threadPools.end() )
    {
        (*i)->RemoveConsumer( taskConsumer );
        ++i;
    }    
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::RegisterTaskConsumerFactory( const CString& taskType       ,
                                           TTaskConsumerFactory* factory )
{GUCEF_TRACE;

    m_consumerFactory.RegisterConcreteFactory( taskType, factory );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TaskManager: new global consumer factory registered of type " + taskType );    
    GlobalTaskConsumerFactoryRegisteredEventData eData( taskType );
    NotifyObserversFromThread( GlobalTaskConsumerFactoryRegisteredEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::UnregisterTaskConsumerFactory( const CString& taskType )
{GUCEF_TRACE;

    m_consumerFactory.UnregisterConcreteFactory( taskType );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TaskManager: global consumer factory unregistered of type " + taskType );
    GlobalTaskConsumerFactoryUnregisteredEventData eData( taskType );
    NotifyObserversFromThread( GlobalTaskConsumerFactoryUnregisteredEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void 
CTaskManager::UnregisterAllTaskConsumerFactories( void )
{GUCEF_TRACE;

    // We dont bulk unregister because we want to provide proper notifications

    CString::StringSet taskTypes;
    m_consumerFactory.ObtainKeySet( taskTypes );

    CString::StringSet::iterator i = taskTypes.begin();
    while ( i != taskTypes.end() )
    {
        UnregisterTaskConsumerFactory( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::RegisterTaskDataFactory( const CString& taskType   ,
                                       TTaskDataFactory* factory )
{GUCEF_TRACE;

    m_taskDataFactory.RegisterConcreteFactory( taskType, factory );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TaskManager: new global task data factory registered of type " + taskType );
    GlobalTaskDataFactoryRegisteredEventData eData( taskType );
    NotifyObserversFromThread( GlobalTaskDataFactoryRegisteredEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::UnregisterTaskDataFactory( const CString& taskType )
{GUCEF_TRACE;

    m_taskDataFactory.UnregisterConcreteFactory( taskType );
    GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TaskManager: global task data factory unregistered of type " + taskType );
    GlobalTaskDataFactoryUnregisteredEventData eData( taskType );
    NotifyObserversFromThread( GlobalTaskDataFactoryUnregisteredEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void 
CTaskManager::UnregisterAllTaskDataFactories( void )
{GUCEF_TRACE;

    // We dont bulk unregister because we want to provide proper notifications

    CString::StringSet taskTypes;
    m_taskDataFactory.ObtainKeySet( taskTypes );

    CString::StringSet::iterator i = taskTypes.begin();
    while ( i != taskTypes.end() )
    {
        UnregisterTaskDataFactory( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::SetTaskIdRecycleCheckThreshold( UInt32 threshold )
{GUCEF_TRACE;

    m_taskIdGenerator->SetRecycleCheckThreshold( threshold );
}

/*-------------------------------------------------------------------------*/

UInt32
CTaskManager::GetTaskIdRecycleCheckThreshold( void ) const
{GUCEF_TRACE;

    return m_taskIdGenerator->GetRecycleCheckThreshold();
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::TryGetGlobalTaskId( CTask::TTaskId& taskId )
{GUCEF_TRACE;

    try
    {
        //@TODO: gcc does not allow direct assignment, check this
        TTaskIdGenerator::TNumericID tmp = m_taskIdGenerator->GenerateID( true );
        taskId = tmp;
    }
    catch ( const TTaskIdGenerator::EMaximumReached& )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_NORMAL, "TaskManager: Global task id limit reached" );
        return false; // no more task ids available
    }
    return true; // task id successfully generated
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::ReleaseGlobalTaskId( CTask::TTaskId& taskId )
{GUCEF_TRACE;

    m_taskIdGenerator->ReleaseID( &taskId );
}

/*-------------------------------------------------------------------------*/

CTaskConsumerPtr
CTaskManager::CreateTaskConsumer( const CString& taskType )
{GUCEF_TRACE;

    CTaskConsumerPtr taskConsumer( m_consumerFactory.Create( taskType ) );
    return taskConsumer;
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::GetAllRegisteredTaskConsumerFactoryTypes( CORE::CString::StringSet& taskTypes )
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        (*i).second->GetAllRegisteredTaskConsumerFactoryTypes( taskTypes );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManager::GetRegisteredTaskConsumerFactoryTypes( const CString& threadPoolName       ,
                                                     CORE::CString::StringSet& taskTypes )
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::iterator i = m_threadPools.find( threadPoolName );
    if ( i != m_threadPools.end() )
    {
        (*i).second->GetAllRegisteredTaskConsumerFactoryTypes( taskTypes );
    }
}

/*-------------------------------------------------------------------------*/

void 
CTaskManager::GetAllThreadPoolNames( CORE::CString::StringSet& poolNames ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        poolNames.insert( (*i).first );
        ++i;
    }
}
/*-------------------------------------------------------------------------*/

bool 
CTaskManager::GetInfo( CTaskManagerInfo& info ) const
{GUCEF_TRACE;

    // Obtain an overall lock to get a coherent snapshot of threadpool info
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    info.SetActiveGlobalNrOfThreads( m_activeGlobalNrOfThreads );
    info.SetDesiredGlobalNrOfThreads( m_desiredGlobalNrOfThreads );
    m_consumerFactory.ObtainKeySet( info.GetTaskConsumerFactoryTypes() );
    m_taskDataFactory.ObtainKeySet( info.GetTaskDataFactoryTypes() );
    GetAllThreadPoolNames( info.GetThreadPoolNames() );
    info.SetGlobalNrOfQueuedTasks( GetGlobalNrOfQueuedTasks() );

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetTaskIdForThreadId( const UInt32 threadId ,
                                    UInt32& taskId        ) const
{GUCEF_TRACE;

    taskId = 0;
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        if ( (*i).second->GetTaskIdForThreadId( threadId, taskId ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetThreadIdForTaskId( const UInt32 taskId ,
                                    UInt32& threadId    ) const
{GUCEF_TRACE;

    threadId = 0;
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        if ( (*i).second->GetThreadIdForTaskId( taskId, threadId ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetTaskInfo( UInt32 taskId                                             , 
                           CTaskInfo& info                                           ,
                           CString& threadPoolName                                   ,
                           bool obtainTaskDataCopyIfPossible                         ,
                           CDataNodeSerializableSettings* taskDataSerializerSettings ) const
{GUCEF_TRACE;

    info.Clear();
    threadPoolName.Clear();
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        if ( (*i).second->GetTaskInfo( taskId, info, obtainTaskDataCopyIfPossible, taskDataSerializerSettings ) )
        {
            threadPoolName = (*i).first;
            return true;
        }
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetAllTaskInfo( TTaskInfoMap& info                                        ,
                              bool obtainTaskDataCopyIfPossible                         ,
                              CDataNodeSerializableSettings* taskDataSerializerSettings ) const
{GUCEF_TRACE;

    info.clear();
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    bool totalSuccess = true;
    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        totalSuccess = (*i).second->GetAllTaskInfo( info                         , 
                                                    obtainTaskDataCopyIfPossible , 
                                                    taskDataSerializerSettings   ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::GetAllThreadPoolInfo( TThreadPoolInfoMap& info ) const
{GUCEF_TRACE;

    info.clear();
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    bool totalSuccess = true; 
    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        CThreadPoolInfo& poolInfo = info[ (*i).first ];
        totalSuccess = (*i).second->GetInfo( poolInfo ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::GetThreadPoolInfo( const CString& poolName, CThreadPoolInfo& info ) const
{GUCEF_TRACE;

    info.Clear();
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.find( poolName );
    if ( i != m_threadPools.end() )
    {
        if ( (*i).second->GetInfo( info ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetThreadInfo( UInt32 threadId         ,   
                             CThreadInfo& info       ,
                             CString& threadPoolName ) const
{GUCEF_TRACE;

    info.Clear();
    threadPoolName.Clear();
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        if ( (*i).second->GetThreadInfo( threadId, info ) )
        {
            threadPoolName = (*i).first;
            return true;
        }
        ++i;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetAllThreadInfo( TThreadInfoMap& info ) const
{GUCEF_TRACE;

    info.clear();
    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    bool totalSuccess = true;
    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        totalSuccess = (*i).second->GetAllThreadInfo( info ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::IsTaskOfTaskTypeExecutable( const CString& taskType, const CString& threadPoolName ) const
{GUCEF_TRACE;

    if ( threadPoolName.IsNULLOrEmpty() )
    {
        MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

        // First check for per pool task consumer factories
        ThreadPoolMap::const_iterator i = m_threadPools.begin();
        while ( i != m_threadPools.end() )
        {
            if ( (*i).second->IsTaskOfTaskTypeExecutable( taskType ) )
                return true;
            ++i;
        }
        
        lock.EarlyReaderUnlock();

        // If no pool level task data factory is defined, lets see if we have a global one
        return m_consumerFactory.IsConstructible( taskType );
    }
    else
    {
        MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

        // check for the explicit threadpool requested
        ThreadPoolMap::const_iterator i = m_threadPools.find( threadPoolName );
        if ( i != m_threadPools.end() )
        {
            if ( (*i).second->IsTaskOfTaskTypeExecutable( taskType ) )
                return true;
        }
        if ( DefaultThreadPoolName == threadPoolName )
        {
            return m_consumerFactory.IsConstructible( taskType );
        }
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManager::IsCustomTaskDataForTaskTypeSerializable( const CString& taskType, const CString& threadPoolName ) const
{GUCEF_TRACE;

    if ( threadPoolName.IsNULLOrEmpty() )
    {
        MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

        // First check for per pool task consumer factories
        ThreadPoolMap::const_iterator i = m_threadPools.begin();
        while ( i != m_threadPools.end() )
        {
            if ( (*i).second->IsCustomTaskDataForTaskTypeSerializable( taskType ) )
                return true;
            ++i;
        }
        
        lock.EarlyReaderUnlock();

        // If no pool level task data factory is defined, lets see if we have a global one
        return m_taskDataFactory.IsConstructible( taskType );
    }
    else
    {
        MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

        // check for the explicit threadpool requested
        ThreadPoolMap::const_iterator i = m_threadPools.find( threadPoolName );
        if ( i != m_threadPools.end() )
        {
            if ( (*i).second->IsCustomTaskDataForTaskTypeSerializable( taskType ) )
                return true;
        }
        if ( DefaultThreadPoolName == threadPoolName )
        {
            return m_taskDataFactory.IsConstructible( taskType );
        }
        return false;
    }
}

/*-------------------------------------------------------------------------*/

CIDataNodeSerializableTaskDataBasicPtr 
CTaskManager::CreateCustomTaskDataForTaskTypeIfAvailable( const CString& taskType, const CString& threadPoolName ) const
{GUCEF_TRACE;

    CIDataNodeSerializableTaskDataBasicPtr taskData;
    if ( threadPoolName.IsNULLOrEmpty() )
    {
        MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

        // First check for per pool task consumer factories
        ThreadPoolMap::const_iterator i = m_threadPools.begin();
        while ( i != m_threadPools.end() )
        {
            taskData = (*i).second->CreateCustomTaskDataForTaskTypeIfAvailable( taskType );
            if ( !taskData.IsNULL() )
                return taskData;
            ++i;
        }
        
        lock.EarlyReaderUnlock();

        // If no pool level task data factory is defined, lets see if we have a global one
        return m_taskDataFactory.Create( taskType );
    }
    else
    {
        MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

        // check for the explicit threadpool requested
        ThreadPoolMap::const_iterator i = m_threadPools.find( threadPoolName );
        if ( i != m_threadPools.end() )
        {
            taskData = (*i).second->CreateCustomTaskDataForTaskTypeIfAvailable( taskType );
            if ( !taskData.IsNULL() )
                return taskData;
        }
        if ( DefaultThreadPoolName == threadPoolName )
        {
            return m_taskDataFactory.Create( taskType );
        }
        return taskData;
    }
}

/*-------------------------------------------------------------------------*/

bool
CTaskManager::GetSerializedTaskDataCopy( const UInt32 taskId                                     ,
                                         CDataNode& domNode                                      ,
                                         const CDataNodeSerializableSettings& serializerSettings ) const
{GUCEF_TRACE;

    MT::CObjectScopeReadOnlyLock lock( this, GUCEF_MT_LONG_LOCK_TIMEOUT );

    ThreadPoolMap::const_iterator i = m_threadPools.begin();
    while ( i != m_threadPools.end() )
    {
        if ( (*i).second->GetSerializedTaskDataCopy( taskId, domNode, serializerSettings ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
