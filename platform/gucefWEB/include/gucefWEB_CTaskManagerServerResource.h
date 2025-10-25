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

#ifndef GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H
#define GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CREADWRITELOCK_H
#include "gucefMT_CReadWriteLock.h"
#define GUCEF_MT_CREADWRITELOCK_H
#endif /* GUCEF_MT_CREADWRITELOCK_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTASKMANAGERINFO_H
#include "gucefCORE_CTaskManagerInfo.h"
#define GUCEF_CORE_CTASKMANAGERINFO_H
#endif /* GUCEF_CORE_CTASKMANAGERINFO_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOLINFO_H
#include "gucefCORE_CThreadPoolInfo.h"
#define GUCEF_CORE_CTHREADPOOLINFO_H
#endif /* GUCEF_CORE_CTHREADPOOLINFO_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_WEB_CIHTTPSERVERROUTER_H
#include "gucefWEB_CIHTTPServerRouter.h"
#define GUCEF_WEB_CIHTTPSERVERROUTER_H
#endif /* GUCEF_WEB_CIHTTPSERVERROUTER_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#include "gucefWEB_CTDataNodeSerializableMapHttpServerResource.h"
#define GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CDATANODESERIALIZABLEHTTPSERVERRESOURCE_H
#include "gucefWEB_CDataNodeSerializableHttpServerResource.h"
#define GUCEF_WEB_CDATANODESERIALIZABLEHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDATANODESERIALIZABLEHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTASKDATACOLLECTIONSERVERRESOURCE_H
#include "gucefWEB_CTaskDataCollectionServerResource.h"
#define GUCEF_WEB_CTASKDATACOLLECTIONSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTASKDATACOLLECTIONSERVERRESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Convenient utility class for hooking up a web interface for accessing the GU Platform task manager
 */
class GUCEF_WEB_PUBLIC_CPP CTaskManagerServerResource : public CORE::CTSGNotifier 
{
    public:

    bool ConnectHttpRouting( CIHTTPServerRouter& webRouter );

    bool DisconnectHttpRouting( CIHTTPServerRouter& webRouter );

    CTaskManagerServerResource( void );
    CTaskManagerServerResource( const CTaskManagerServerResource& src );
    virtual ~CTaskManagerServerResource() GUCEF_VIRTUAL_OVERRIDE;

    void SetTaskManagerRootPath( const CString& rootPath );

    CString GetTaskManagerRootPath( void ) const;

    bool UpdateAllInfo( void );

    bool UpdateThreadPoolInfo( const CString& poolName );

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;
    
    private:

    typedef CORE::CTEventHandlerFunctor< CTaskManagerServerResource > TEventCallback;

    void LinkResources( void );
    
    void RegisterEventHandlers( void );

    virtual void OnThreadPoolCreation( CORE::CNotifier* notifier                 ,
                                       const CORE::CEvent& eventId               ,
                                       CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnThreadPoolDestruction( CORE::CNotifier* notifier                 ,
                                          const CORE::CEvent& eventId               ,
                                          CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnThreadPoolUnregistered( CORE::CNotifier* notifier                 ,
                                           const CORE::CEvent& eventId               ,
                                           CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnGlobalTaskConsumerFactoryRegistered( CORE::CNotifier* notifier                 ,
                                                        const CORE::CEvent& eventId               ,
                                                        CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnGlobalTaskConsumerFactoryUnregistered( CORE::CNotifier* notifier                 ,
                                                          const CORE::CEvent& eventId               ,
                                                          CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnGlobalTaskDataFactoryRegistered( CORE::CNotifier* notifier                 ,
                                                    const CORE::CEvent& eventId               ,
                                                    CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnGlobalTaskDataFactoryUnregistered( CORE::CNotifier* notifier                 ,
                                                      const CORE::CEvent& eventId               ,
                                                      CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnThreadUpdateEvent( CORE::CNotifier* notifier                 ,
                                      const CORE::CEvent& eventId               ,
                                      CORE::CICloneable* eventData = GUCEF_NULL );

    virtual void OnTaskUpdateEvent( CORE::CNotifier* notifier                 ,
                                    const CORE::CEvent& eventId               ,
                                    CORE::CICloneable* eventData = GUCEF_NULL );

    private:
    
    typedef CORE::CTaskManager::TTaskInfoMap                                            TTaskInfoMap;    
    typedef CTDataNodeSerializableMapHttpServerResource< UInt32, CORE::CTaskInfo >      TTaskInfoMapRsc;
    typedef CORE::CTSharedPtr< TTaskInfoMapRsc, MT::CMutex >                            TTaskInfoMapRscPtr;
    typedef CORE::CTaskManager::TThreadInfoMap                                          TThreadInfoMap;
    typedef CTDataNodeSerializableMapHttpServerResource< UInt32, CORE::CThreadInfo >    TThreadInfoMapRsc;
    typedef CORE::CTSharedPtr< TThreadInfoMapRsc, MT::CMutex >                          TThreadInfoMapRscPtr;
    typedef CORE::CTaskManager::TThreadPoolInfoMap                                      TThreadPoolInfoMap;
    typedef GUCEF::map< CString, CORE::CIDataNodeSerializableTaskDataBasicPtr >         TStringToDataNodeSerializableTaskDataPtrMap;
    typedef CTDataNodeSerializableMapHttpServerResource< CString, CORE::CIDataNodeSerializableTaskDataBasicPtr >    TStringToDataNodeSerializableTaskDataPtrMapRsc;
    typedef CORE::CTSharedPtr< TStringToDataNodeSerializableTaskDataPtrMapRsc, MT::CMutex >                         TStringToDataNodeSerializableTaskDataPtrMapRscPtr;

    class CThreadPoolMetaData
    {
        public:

        CORE::CThreadPoolInfo* threadPoolInfo;
        CDataNodeSerializableHttpServerResourcePtr threadPoolInfoRsc;
        CORE::CDateTime threadPoolInfoLastUpdate;
        TTaskInfoMap allTaskInfo;
        TTaskInfoMapRscPtr allTaskInfoRsc;
        TThreadInfoMap allThreadInfo;
        TThreadInfoMapRscPtr allThreadInfoRsc; 
        MT::CILockable* lock;

        CThreadPoolMetaData( void );

        void LinkTo( MT::CILockable* lock, CORE::CThreadPoolInfo* threadPoolInfo );
    };

    typedef CTDataNodeSerializableMapHttpServerResource< CORE::CString, CORE::CThreadPoolInfo >     TThreadPoolInfoMapRsc;
    typedef CORE::CTSharedPtr< TThreadPoolInfoMapRsc, MT::CMutex >                                  TThreadPoolInfoMapRscPtr;
    typedef GUCEF::map< CORE::CString, CThreadPoolMetaData >                                        TThreadPoolMetaDataMap; 
    typedef GUCEF::map< UInt32, CORE::CThreadInfo* >                                                TThreadInfoRawPtrMap;    
    typedef CTDataNodeSerializableMapHttpServerResource< UInt32, CORE::CThreadInfo* >               TThreadInfoRawPtrMapRsc;
    typedef CORE::CTSharedPtr< TThreadInfoRawPtrMapRsc, MT::CMutex >                                TThreadInfoRawPtrMapRscPtr;
    typedef GUCEF::map< UInt32, CORE::CTaskInfo* >                                                  TTaskInfoRawPtrMap;    
    typedef CTDataNodeSerializableMapHttpServerResource< UInt32, CORE::CTaskInfo* >                 TTaskInfoRawPtrMapRsc;
    typedef CORE::CTSharedPtr< TTaskInfoRawPtrMapRsc, MT::CMutex >                                  TTaskInfoRawPtrMapRscPtr;

    CORE::CTaskManagerInfo m_taskManagerInfo;
    CDataNodeSerializableHttpServerResourcePtr m_taskManagerInfoRsc;
    TThreadPoolInfoMap m_threadPoolInfoMap;
    TThreadPoolInfoMapRscPtr m_threadPoolInfoMapRsc;
    TThreadPoolMetaDataMap m_threadPoolMetaDataMap;
    TTaskInfoRawPtrMap m_globalTaskIndex;
    TTaskInfoRawPtrMapRscPtr m_globalTaskIndexRsc;
    TThreadInfoRawPtrMap m_globalThreadIndex;
    TThreadInfoRawPtrMapRscPtr m_globalThreadIndexRsc;
    CTaskDataCollectionServerResourcePtr m_globalTaskData;
    TStringToDataNodeSerializableTaskDataPtrMap m_globalTaskDataTemplates;
    TStringToDataNodeSerializableTaskDataPtrMapRscPtr m_globalTaskDataTemplatesRsc;
    CIHTTPServerRouter* m_router;
    CString m_rootPath;
    MT::CReadWriteLock m_rwLock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CTASKMANAGERSERVERRESOURCE_H ? */
