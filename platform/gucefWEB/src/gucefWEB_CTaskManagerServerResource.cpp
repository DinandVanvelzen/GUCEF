/*
 *  gucefWEB: GUCEF module providing Web application functionality 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#include "gucefWEB_CTaskManagerServerResource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTaskManagerServerResource::CThreadPoolMetaData::CThreadPoolMetaData( void )
    : threadPoolInfo( GUCEF_NULL )
    , threadPoolInfoRsc()
    , threadPoolInfoLastUpdate( CORE::CDateTime::PastMax ) 
    , allTaskInfo()
    , allTaskInfoRsc()
    , allThreadInfo()
    , allThreadInfoRsc()
    , lock( GUCEF_NULL )
{GUCEF_TRACE;
    
    threadPoolInfoRsc = CDataNodeSerializableHttpServerResourcePtr( GUCEF_NEW CDataNodeSerializableHttpServerResource() );
    threadPoolInfoRsc->LinkTo( GUCEF_NULL, threadPoolInfo, lock ); 
    threadPoolInfoRsc->SetIsCreateSupported( false );
    threadPoolInfoRsc->SetIsDeserializeSupported( false, false );
    threadPoolInfoRsc->SetIsDeserializeSupported( false, true );
    allTaskInfoRsc = TTaskInfoMapRscPtr( GUCEF_NEW TTaskInfoMapRsc() );
    allTaskInfoRsc->LinkTo( "tasks", "taskId", GUCEF_NULL, &allTaskInfo, lock, false, true );   
    allThreadInfoRsc = TThreadInfoMapRscPtr( GUCEF_NEW TThreadInfoMapRsc() );
    allThreadInfoRsc->LinkTo( "threads", "threadId", GUCEF_NULL, &allThreadInfo, lock, false, true );   
    allThreadInfoRsc->SetIsCreateSupported( false );
    allThreadInfoRsc->SetIsDeserializeSupported( false, false );
    allThreadInfoRsc->SetIsDeserializeSupported( false, true );
}

/*-------------------------------------------------------------------------*/

void 
CTaskManagerServerResource::CThreadPoolMetaData::LinkTo( MT::CILockable* newLock, CORE::CThreadPoolInfo* poolInfo )
{GUCEF_TRACE;
    
    lock = newLock;
    threadPoolInfo = poolInfo;
    threadPoolInfoRsc->LinkTo( GUCEF_NULL, threadPoolInfo, lock );   
    allThreadInfoRsc->LinkTo( "threads", "threadId", GUCEF_NULL, &allThreadInfo, lock, false, true );
    allTaskInfoRsc->LinkTo( "tasks", "taskId", GUCEF_NULL, &allTaskInfo, lock, false, true );
}

/*-------------------------------------------------------------------------*/

CTaskManagerServerResource::CTaskManagerServerResource( void )
    : CORE::CTSGNotifier()
    , m_taskManagerInfo()
    , m_taskManagerInfoRsc()
    , m_threadPoolInfoMap()
    , m_threadPoolInfoMapRsc()
    , m_threadPoolMetaDataMap()
    , m_globalTaskIndex()
    , m_globalTaskIndexRsc()
    , m_globalThreadIndex()
    , m_globalThreadIndexRsc()
    , m_globalTaskDataTemplates()
    , m_globalTaskDataTemplatesRsc()
    , m_router( GUCEF_NULL )
    , m_rootPath( "/v1/taskmanager/" )
    , m_rwLock( true )
{GUCEF_TRACE;

    LinkResources();
    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CTaskManagerServerResource::CTaskManagerServerResource( const CTaskManagerServerResource& src )
    : CORE::CTSGNotifier( src )
    , m_taskManagerInfo( src.m_taskManagerInfo )
    , m_taskManagerInfoRsc()
    , m_threadPoolInfoMap( src.m_threadPoolInfoMap )
    , m_threadPoolInfoMapRsc()
    , m_threadPoolMetaDataMap( src.m_threadPoolMetaDataMap )
    , m_globalTaskIndex()
    , m_globalTaskIndexRsc()
    , m_globalThreadIndex()
    , m_globalThreadIndexRsc()
    , m_globalTaskDataTemplates( src.m_globalTaskDataTemplates )
    , m_globalTaskDataTemplatesRsc()
    , m_router( src.m_router )
    , m_rootPath( src.m_rootPath )
    , m_rwLock( src.m_rwLock )
{GUCEF_TRACE;

    LinkResources();
    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::LinkResources( void )
{GUCEF_TRACE;

    m_taskManagerInfoRsc = CDataNodeSerializableHttpServerResourcePtr( GUCEF_NEW CDataNodeSerializableHttpServerResource() );
    m_taskManagerInfoRsc->LinkTo( GUCEF_NULL, &m_taskManagerInfo, &m_rwLock );    
    m_taskManagerInfoRsc->SetIsCreateSupported( false );
    m_taskManagerInfoRsc->SetIsDeserializeSupported( false, false );
    m_taskManagerInfoRsc->SetIsDeserializeSupported( false, true );
    m_threadPoolInfoMapRsc = TThreadPoolInfoMapRscPtr( GUCEF_NEW TThreadPoolInfoMapRsc() );
    m_threadPoolInfoMapRsc->LinkTo( "threadpools", "threadPoolName", GUCEF_NULL, &m_threadPoolInfoMap, &m_rwLock, false, true );
    m_threadPoolInfoMapRsc->SetIsCreateSupported( false );
    m_threadPoolInfoMapRsc->SetIsDeserializeSupported( false, false );
    m_threadPoolInfoMapRsc->SetIsDeserializeSupported( false, true );
    m_globalTaskIndexRsc = TTaskInfoRawPtrMapRscPtr( GUCEF_NEW TTaskInfoRawPtrMapRsc() );
    m_globalTaskIndexRsc->LinkTo( "tasks", "taskId", GUCEF_NULL, &m_globalTaskIndex, &m_rwLock, false, true );
    m_globalTaskIndexRsc->SetIsCreateSupported( false );
    m_globalTaskIndexRsc->SetIsDeserializeSupported( false, false );
    m_globalTaskIndexRsc->SetIsDeserializeSupported( false, true );
    m_globalThreadIndexRsc = TThreadInfoRawPtrMapRscPtr( GUCEF_NEW TThreadInfoRawPtrMapRsc() );
    m_globalThreadIndexRsc->LinkTo( "threads", "threadId", GUCEF_NULL, &m_globalThreadIndex, &m_rwLock, false, true );
    m_globalThreadIndexRsc->SetIsCreateSupported( false );
    m_globalThreadIndexRsc->SetIsDeserializeSupported( false, false );
    m_globalThreadIndexRsc->SetIsDeserializeSupported( false, true ); 
    m_globalTaskData = CTaskDataCollectionServerResourcePtr( GUCEF_NEW CTaskDataCollectionServerResource() );
    m_globalTaskData->SetIsCreateSupported( true );
    m_globalTaskData->SetIsDeserializeSupported( false, false );
    m_globalTaskData->SetIsDeserializeSupported( false, true );
    m_globalTaskDataTemplatesRsc = TStringToDataNodeSerializableTaskDataPtrMapRscPtr( GUCEF_NEW TStringToDataNodeSerializableTaskDataPtrMapRsc() );
    m_globalTaskDataTemplatesRsc->LinkTo( "templates", "taskTypeName", GUCEF_NULL, &m_globalTaskDataTemplates, &m_rwLock, false, true );
    m_globalTaskDataTemplatesRsc->SetIsCreateSupported( false );
    m_globalTaskDataTemplatesRsc->SetIsDeserializeSupported( false, false );
    m_globalTaskDataTemplatesRsc->SetIsDeserializeSupported( false, true );
}

/*-------------------------------------------------------------------------*/

CTaskManagerServerResource::~CTaskManagerServerResource( void )
{GUCEF_TRACE;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::RegisterEventHandlers( void )
{GUCEF_TRACE;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    
    TEventCallback callback( this, &CTaskManagerServerResource::OnThreadPoolCreation );
    SubscribeTo( &taskManager, CORE::CTaskManager::ThreadPoolCreatedEvent, callback );

    TEventCallback callback2( this, &CTaskManagerServerResource::OnThreadPoolDestruction );
    SubscribeTo( &taskManager, CORE::CTaskManager::ThreadPoolDestructionEvent, callback2 ); 

    TEventCallback callback3( this, &CTaskManagerServerResource::OnThreadPoolUnregistered );
    SubscribeTo( &taskManager, CORE::CTaskManager::ThreadPoolUnregisteredEvent, callback3 );     
    
    TEventCallback callback4( this, &CTaskManagerServerResource::OnGlobalTaskConsumerFactoryRegistered );
    SubscribeTo( &taskManager, CORE::CTaskManager::GlobalTaskConsumerFactoryRegisteredEvent, callback4 );

    TEventCallback callback5( this, &CTaskManagerServerResource::OnGlobalTaskConsumerFactoryUnregistered );
    SubscribeTo( &taskManager, CORE::CTaskManager::GlobalTaskConsumerFactoryUnregisteredEvent, callback5 );

    TEventCallback callback6( this, &CTaskManagerServerResource::OnGlobalTaskDataFactoryRegistered );
    SubscribeTo( &taskManager, CORE::CTaskManager::GlobalTaskDataFactoryRegisteredEvent, callback6 );

    TEventCallback callback7( this, &CTaskManagerServerResource::OnGlobalTaskDataFactoryUnregistered );
    SubscribeTo( &taskManager, CORE::CTaskManager::GlobalTaskDataFactoryUnregisteredEvent, callback7 );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::SetTaskManagerRootPath( const CString& rootPath )
{GUCEF_TRACE;

    MT::CScopeWriterLock writeLock( m_rwLock );
    m_rootPath = rootPath;
}

/*-------------------------------------------------------------------------*/

CString
CTaskManagerServerResource::GetTaskManagerRootPath( void ) const
{GUCEF_TRACE;

    MT::CScopeReaderLock readLock( m_rwLock );
    return m_rootPath;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTaskManagerServerResource::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwLock.WriterStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTaskManagerServerResource::Unlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwLock.WriterStop() );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTaskManagerServerResource::ReadOnlyLock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwLock.ReaderStart( lockWaitTimeoutInMs ) );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus 
CTaskManagerServerResource::ReadOnlyUnlock( void ) const
{GUCEF_TRACE;

    return MT::CReadWriteLock::RwLockStateToLockStatus( m_rwLock.ReaderStop() );
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnGlobalTaskConsumerFactoryRegistered( CORE::CNotifier* notifier    ,
                                                                   const CORE::CEvent& eventId  ,
                                                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CTaskManager::GlobalTaskConsumerFactoryRegisteredEventData* eData = static_cast< CORE::CTaskManager::GlobalTaskConsumerFactoryRegisteredEventData* >( eventData );
    if ( GUCEF_NULL != eData )
    {
        const CORE::CString& factoryType = eData->GetData();        
        
        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );
        m_taskManagerInfo.GetTaskConsumerFactoryTypes().insert( factoryType );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnGlobalTaskConsumerFactoryUnregistered( CORE::CNotifier* notifier    ,
                                                                     const CORE::CEvent& eventId  ,
                                                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CTaskManager::GlobalTaskConsumerFactoryUnregisteredEventData* eData = static_cast< CORE::CTaskManager::GlobalTaskConsumerFactoryUnregisteredEventData* >( eventData );
    if ( GUCEF_NULL != eData )
    {
        const CORE::CString& factoryType = eData->GetData();        
        
        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );
        m_taskManagerInfo.GetTaskConsumerFactoryTypes().erase( factoryType );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnGlobalTaskDataFactoryRegistered( CORE::CNotifier* notifier    ,
                                                               const CORE::CEvent& eventId  ,
                                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CTaskManager::GlobalTaskDataFactoryRegisteredEventData* eData = static_cast< CORE::CTaskManager::GlobalTaskDataFactoryRegisteredEventData* >( eventData );
    if ( GUCEF_NULL != eData )
    {
        const CORE::CString& factoryType = eData->GetData();        
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();

        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );
        m_taskManagerInfo.GetTaskDataFactoryTypes().insert( factoryType );                
        m_globalTaskDataTemplates[ factoryType ] = taskManager.CreateCustomTaskDataForTaskTypeIfAvailable( factoryType );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnGlobalTaskDataFactoryUnregistered( CORE::CNotifier* notifier    ,
                                                                 const CORE::CEvent& eventId  ,
                                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CTaskManager::GlobalTaskDataFactoryRegisteredEventData* eData = static_cast< CORE::CTaskManager::GlobalTaskDataFactoryRegisteredEventData* >( eventData );
    if ( GUCEF_NULL != eData )
    {
        const CORE::CString& factoryType = eData->GetData();        
        
        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );
        m_taskManagerInfo.GetTaskDataFactoryTypes().erase( factoryType );
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnThreadPoolCreation( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventId  ,
                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_router )
    {
        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

        if ( GUCEF_NULL != m_router )
        {        
            CORE::CTaskManager::ThreadPoolCreatedEventData* eData = static_cast< CORE::CTaskManager::ThreadPoolCreatedEventData* >( eventData );
            if ( GUCEF_NULL != eData )
            {
                const CORE::CString& poolName = eData->GetData();        
                UpdateThreadPoolInfo( poolName );   
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnThreadPoolDestruction( CORE::CNotifier* notifier    ,
                                                     const CORE::CEvent& eventId  ,
                                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_router )
    {
        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

        if ( GUCEF_NULL != m_router )
        {
            const CORE::CString& poolName = static_cast< CORE::CTaskManager::ThreadPoolDestructionEventData* >( eventData )->GetData();
                    
            m_router->RemoveResourceMapping( m_rootPath + "threadpools/" + poolName );
            m_threadPoolMetaDataMap.erase( poolName );
            m_threadPoolInfoMap.erase( poolName );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnThreadPoolUnregistered( CORE::CNotifier* notifier    ,
                                                      const CORE::CEvent& eventId  ,
                                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_router )
    {
        MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

        if ( GUCEF_NULL != m_router )
        {
            const CORE::CString& poolName = static_cast< CORE::CTaskManager::ThreadPoolUnregisteredEventData* >( eventData )->GetData();
                    
            m_router->RemoveResourceMapping( m_rootPath + "threadpools/" + poolName );
            m_threadPoolMetaDataMap.erase( poolName );
            m_threadPoolInfoMap.erase( poolName );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnThreadUpdateEvent( CORE::CNotifier* notifier    ,
                                                 const CORE::CEvent& eventId  ,
                                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
    {        
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        UInt32 threadId = static_cast< CORE::CThreadPool::TThreadUpdateEventData* >( eventData )->GetData();    
        
        CORE::CThreadInfo threadInfo;
        CORE::CString threadPoolName;
        if ( taskManager.GetThreadInfo( threadId, threadInfo, threadPoolName ) )
        {
            MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

            TThreadPoolMetaDataMap::iterator i = m_threadPoolMetaDataMap.find( threadPoolName );
            if ( i == m_threadPoolMetaDataMap.end() )
            {
                UpdateThreadPoolInfo( threadPoolName );
                i = m_threadPoolMetaDataMap.find( threadPoolName );
            }
            if ( i != m_threadPoolMetaDataMap.end() )
            {
                CThreadPoolMetaData& threadPoolMetaData = (*i).second;                          
                CORE::CThreadInfo* retainedThreadInfo = &( threadPoolMetaData.allThreadInfo[ threadId ] = threadInfo );
                m_globalThreadIndex[ threadId ] = retainedThreadInfo;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CTaskManagerServerResource::OnTaskUpdateEvent( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != eventData )
    {        
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        UInt32 taskId = static_cast< CORE::CThreadPool::TTaskUpdateEventData* >( eventData )->GetData();    
        
        CORE::CTaskInfo taskInfo;
        CORE::CString threadPoolName;
        if ( taskManager.GetTaskInfo( taskId, taskInfo, threadPoolName, false, GUCEF_NULL ) )
        {
            MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

            TThreadPoolMetaDataMap::iterator i = m_threadPoolMetaDataMap.find( threadPoolName );
            if ( i == m_threadPoolMetaDataMap.end() )
            {
                UpdateThreadPoolInfo( threadPoolName );
                i = m_threadPoolMetaDataMap.find( threadPoolName );
            }
            if ( i != m_threadPoolMetaDataMap.end() )
            {
                CThreadPoolMetaData& threadPoolMetaData = (*i).second;                          
                CORE::CTaskInfo* retainedTaskInfo = &( threadPoolMetaData.allTaskInfo[ taskId ] = taskInfo );
                m_globalTaskIndex[ taskId ] = retainedTaskInfo;
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerServerResource::ConnectHttpRouting( CIHTTPServerRouter& webRouter )
{GUCEF_TRACE;

    // "/v1/taskmanager"                                            <- TaskManagerInfo obj
    // "/v1/taskmanager/tasks/"                                     <- task id index Or list of TaskInfo objects
    // "/v1/taskmanager/threads/"                                   <- thread id index Or list of ThreadInfo objects 
    // "/v1/taskmanager/threadpools/"                               <- thread pool name index Or list of ThreadPoolInfo obj
    // "/v1/taskmanager/threadpools/default/"                       <- ThreadPoolInfo obj
    // "/v1/taskmanager/threadpools/default/threads/"               <- thread id index Or list of ThreadInfo objects specific to the given thread pool
    // "/v1/taskmanager/threadpools/default/tasks/"                 <- task id index Or list of TaskInfo objects specific to the given thread pool
    // "/v1/taskmanager/taskdata/templates/"                        <- default objects of serializable task data
    // "/v1/taskmanager/taskdata/                                   <- endpoint for posting data of new tasks

    m_router = &webRouter;
    if ( GUCEF_NULL != m_router )
    {
        bool totalSuccess = true;
        totalSuccess = m_router->SetResourceMapping( m_rootPath, m_taskManagerInfoRsc ) && totalSuccess; 
        totalSuccess = m_router->SetResourceMapping( m_rootPath + "tasks", m_globalTaskIndexRsc ) && totalSuccess;
        totalSuccess = m_router->SetResourceMapping( m_rootPath + "threads", m_globalThreadIndexRsc ) && totalSuccess;
        totalSuccess = m_router->SetResourceMapping( m_rootPath + "threadpools", m_threadPoolInfoMapRsc ) && totalSuccess;        
        totalSuccess = m_router->SetResourceMapping( m_rootPath + "taskdata", m_globalTaskData ) && totalSuccess;
        totalSuccess = m_router->SetResourceMapping( m_rootPath + "taskdata/templates", m_globalTaskDataTemplatesRsc ) && totalSuccess;
        return totalSuccess && UpdateAllInfo();
    }
    
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManagerServerResource::DisconnectHttpRouting( CIHTTPServerRouter& webRouter )
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = m_router->RemoveResourceMapping( m_taskManagerInfoRsc ) && totalSuccess; 
    totalSuccess = m_router->RemoveResourceMapping( m_globalTaskIndexRsc ) && totalSuccess; 
    totalSuccess = m_router->RemoveResourceMapping( m_globalThreadIndexRsc ) && totalSuccess; 
    totalSuccess = m_router->RemoveResourceMapping( m_threadPoolInfoMapRsc ) && totalSuccess;     
    totalSuccess = m_router->RemoveResourceMapping( m_globalTaskData ) && totalSuccess;     
    totalSuccess = m_router->RemoveResourceMapping( m_globalTaskDataTemplatesRsc ) && totalSuccess;     
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CTaskManagerServerResource::UpdateThreadPoolInfo( const CString& poolName )
{GUCEF_TRACE;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    CORE::ThreadPoolPtr threadPool = taskManager.GetThreadPool( poolName );

    MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

    CORE::CThreadPoolInfo& threadPoolInfo = m_threadPoolInfoMap[ poolName ];
    CThreadPoolMetaData& threadPoolMetaData = m_threadPoolMetaDataMap[ poolName ];    
    
    // always relink to extenal items
    // we pass the same rw lock to protect the overall DOM structure flow
    threadPoolMetaData.LinkTo( &m_rwLock, &threadPoolInfo ); 
    m_router->SetResourceMapping( m_rootPath + "threadpools/" + poolName, threadPoolMetaData.threadPoolInfoRsc ); 
    m_router->SetResourceMapping( m_rootPath + "threadpools/" + poolName + "/tasks", threadPoolMetaData.allTaskInfoRsc ); 
    m_router->SetResourceMapping( m_rootPath + "threadpools/" + poolName + "/threads", threadPoolMetaData.allThreadInfoRsc ); 
    
    if ( !threadPool.IsNULL() )
    {
        TEventCallback threadUpdateCallback( this, &CTaskManagerServerResource::OnThreadUpdateEvent );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::ThreadStartedEvent, threadUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::ThreadPausedEvent, threadUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::ThreadResumedEvent, threadUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::ThreadKilledEvent, threadUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::ThreadFinishedEvent, threadUpdateCallback );

        TEventCallback taskUpdateCallback( this, &CTaskManagerServerResource::OnTaskUpdateEvent );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskQueuedEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskStartupEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskStartupFailedEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskStartedEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskPausedEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskResumedEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskKilledEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskStoppedEvent, taskUpdateCallback );
        SubscribeTo( threadPool.GetPointerAlways(), CORE::CThreadPool::TaskFinishedEvent, taskUpdateCallback );

        bool totalSuccess = true;
        totalSuccess = threadPool->GetInfo( threadPoolInfo ) && totalSuccess; 
        totalSuccess = threadPool->GetAllTaskInfo( threadPoolMetaData.allTaskInfo, false, GUCEF_NULL ) && totalSuccess; 
        totalSuccess = threadPool->GetAllThreadInfo( threadPoolMetaData.allThreadInfo ) && totalSuccess; 

        if ( totalSuccess )
            threadPoolMetaData.threadPoolInfoLastUpdate = CORE::CDateTime::NowUTCDateTime();     

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool 
CTaskManagerServerResource::UpdateAllInfo( void )
{GUCEF_TRACE;

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    bool totalSuccess = true;

    MT::CScopeWriterLock writeLock( m_rwLock, GUCEF_MT_LONG_LOCK_TIMEOUT );

    totalSuccess = taskManager.GetInfo( m_taskManagerInfo ) && totalSuccess;   
    totalSuccess = taskManager.GetAllThreadPoolInfo( m_threadPoolInfoMap ) && totalSuccess;  
    
    TThreadPoolInfoMap::iterator i = m_threadPoolInfoMap.begin();
    while ( i != m_threadPoolInfoMap.end() )
    {
        totalSuccess = UpdateThreadPoolInfo( (*i).first ) && totalSuccess;
        ++i;
    }
    
    return totalSuccess;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
