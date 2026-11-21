/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include <stdlib.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */
#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */
#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */
#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */
#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */
#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */
#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#include "gucefDRNUTILS_CDrnUtilsGlobal.h"
#define GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H
#endif /* GUCEF_DRNUTILS_CDRNUTILSGLOBAL_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETTEXTUREANALYZER_H
#include "gucefASSETSTORE_CAssetTextureAnalyzer.h"
#define GUCEF_ASSETSTORE_CASSETTEXTUREANALYZER_H
#endif /* GUCEF_ASSETSTORE_CASSETTEXTUREANALYZER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#include "gucefASSETSTORE_CAssetStoreDownloadManager.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADMANAGER_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#include "gucefASSETSTORE_CPrimaryAssetStore.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTORE_H ? */
#include "gucefASSETSTORE_CAssetStoreGlobal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

MT::CMutex CAssetStoreGlobal::g_dataLock;
CAssetStoreGlobal* CAssetStoreGlobal::g_instance = GUCEF_NULL;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreGlobal::CAssetStoreGlobal( void )
    : m_providerFactory( GUCEF_NULL )
    , m_registry( GUCEF_NULL )
    , m_analyzerRegistry( GUCEF_NULL )
    , m_providerOperationFactory()
    , m_downloadTaskFactory()
    , m_primaryStoreOperationFactory()
    , m_analysisTaskFactory()
    , m_serviceOperationFactory()
    , m_remoteFileTransferFactory()
    , m_syncScheduleTaskFactory()
    , m_syncScheduleTaskDataFactory()
    , m_serviceFactory( GUCEF_NULL )
    , m_localServiceFactory()
    , m_remoteServiceFactory()
    , m_webSessionStore( GUCEF_NULL )
    , m_webCredentialStore( GUCEF_NULL )
    , m_webStoreLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreGlobal::Initialize( void )
{GUCEF_TRACE;

    // the modules this one depends on first
    CORE::CCoreGlobal::Instance();
    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();
    COMCORE::CComCoreGlobal::Instance();
    COM::CComGlobal::Instance();
    WEB::CWebGlobal::Instance();
    AUTH::CAuthGlobal::Instance();     // also the events and the task of the web session sign ins (AUTH::CWebSessionSignIn)
    DRN::CDRNGlobal::Instance();
    DRNUTILS::CDrnUtilsGlobal::Instance();

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefASSETSTORE Global systems initializing" );

    CIAssetStoreProvider::RegisterEvents();
    CAssetStoreRegistry::RegisterEvents();
    CAssetStoreDownloadManager::RegisterEvents();
    CPrimaryAssetStore::RegisterEvents();
    CIAssetStoreService::RegisterEvents();

    m_providerFactory = GUCEF_NEW CAssetStoreProviderFactory();
    m_registry = GUCEF_NEW CAssetStoreRegistry( *m_providerFactory );
    m_analyzerRegistry = GUCEF_NEW CAssetContentAnalyzerRegistry();
    m_analyzerRegistry->Register( TAssetContentAnalyzerPtr( GUCEF_NEW CAssetTextureAnalyzer() ) );

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.RegisterTaskConsumerFactory( CAssetStoreProviderOperationTaskConsumer::TaskType, &m_providerOperationFactory );
    taskManager.RegisterTaskConsumerFactory( CAssetStoreDownloadTaskConsumer::TaskType, &m_downloadTaskFactory );
    taskManager.RegisterTaskConsumerFactory( CPrimaryAssetStoreOperationTaskConsumer::TaskType, &m_primaryStoreOperationFactory );
    taskManager.RegisterTaskConsumerFactory( CAssetAnalysisTaskConsumer::TaskType, &m_analysisTaskFactory );
    taskManager.RegisterTaskConsumerFactory( CLocalAssetStoreServiceOperationTaskConsumer::TaskType, &m_serviceOperationFactory );
    taskManager.RegisterTaskConsumerFactory( CRemoteFileTransferTaskConsumer::TaskType, &m_remoteFileTransferFactory );
    taskManager.RegisterTaskConsumerFactory( CAssetLibrarySyncScheduleTaskConsumer::TaskType, &m_syncScheduleTaskFactory );
    taskManager.RegisterTaskDataFactory( CAssetLibrarySyncScheduleTaskConsumer::TaskType, &m_syncScheduleTaskDataFactory );

    // the implementations of the service: the one in this process; another registers itself under its own id
    m_serviceFactory = GUCEF_NEW CAssetStoreServiceFactory();
    m_serviceFactory->RegisterConcreteFactory( CAssetStoreServiceConfig::LocalImplementationId, &m_localServiceFactory );
    m_serviceFactory->RegisterConcreteFactory( CAssetStoreServiceConfig::RemoteImplementationId, &m_remoteServiceFactory );

    CORE::CCoreGlobal::Instance()->GetPluginControl().ListModuleForLinkBack( "gucefASSETSTORE" );
}

/*-------------------------------------------------------------------------*/

CAssetStoreGlobal::~CAssetStoreGlobal()
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefASSETSTORE Global systems shutting down" );

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnlistModuleForLinkBack( "gucefASSETSTORE" );

    CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
    taskManager.UnregisterTaskDataFactory( CAssetLibrarySyncScheduleTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CAssetLibrarySyncScheduleTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CRemoteFileTransferTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CLocalAssetStoreServiceOperationTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CAssetAnalysisTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CPrimaryAssetStoreOperationTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CAssetStoreDownloadTaskConsumer::TaskType );
    taskManager.UnregisterTaskConsumerFactory( CAssetStoreProviderOperationTaskConsumer::TaskType );

    if ( GUCEF_NULL != m_serviceFactory )
        m_serviceFactory->UnregisterAllConcreteFactories();
    GUCEF_DELETE m_serviceFactory;
    m_serviceFactory = GUCEF_NULL;

    // reverse order of creation: the providers go through their factories
    GUCEF_DELETE m_analyzerRegistry;
    m_analyzerRegistry = GUCEF_NULL;
    GUCEF_DELETE m_registry;
    m_registry = GUCEF_NULL;
    if ( GUCEF_NULL != m_providerFactory )
        m_providerFactory->UnregisterAllConcreteFactories();
    GUCEF_DELETE m_providerFactory;
    m_providerFactory = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAssetStoreGlobal*
CAssetStoreGlobal::Instance( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == g_instance )
    {
        MT::CScopeMutex lock( g_dataLock );
        if ( GUCEF_NULL == g_instance )
        {
            g_instance = GUCEF_NEW CAssetStoreGlobal();
            if ( GUCEF_NULL != g_instance )
            {
                g_instance->Initialize();
                atexit( CAssetStoreGlobal::Deinstance );
            }
        }
    }
    return g_instance;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreGlobal::Deinstance( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( g_dataLock );
    GUCEF_DELETE g_instance;
    g_instance = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CAssetStoreProviderFactory&
CAssetStoreGlobal::GetProviderFactory( void )
{GUCEF_TRACE;

    return *m_providerFactory;
}

/*-------------------------------------------------------------------------*/

CAssetStoreRegistry&
CAssetStoreGlobal::GetRegistry( void )
{GUCEF_TRACE;

    return *m_registry;
}

/*-------------------------------------------------------------------------*/

CAssetContentAnalyzerRegistry&
CAssetStoreGlobal::GetContentAnalyzerRegistry( void )
{GUCEF_TRACE;

    return *m_analyzerRegistry;
}

/*-------------------------------------------------------------------------*/

CAssetStoreServiceFactory&
CAssetStoreGlobal::GetServiceFactory( void )
{GUCEF_TRACE;

    return *m_serviceFactory;
}

/*-------------------------------------------------------------------------*/

TAssetStoreServicePtr
CAssetStoreGlobal::CreateService( const CAssetStoreServiceConfig& config, CORE::CString& error )
{GUCEF_TRACE;

    const CORE::CString id = config.implementationId.IsNULLOrEmpty() ? CAssetStoreServiceConfig::LocalImplementationId : config.implementationId;
    if ( !m_serviceFactory->IsConstructible( id ) )
    {
        error = id == CAssetStoreServiceConfig::RemoteImplementationId ? CORE::CString( "remote service not available in this build" )
                                                                       : "no implementation \"" + id + "\" of the asset store service is available in this build";
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAssetStoreGlobal: " << error );
        return TAssetStoreServicePtr();
    }
    TAssetStoreServicePtr service = m_serviceFactory->Create( id, config );
    if ( service.IsNULL() )
        error = "the implementation \"" + id + "\" of the asset store service could not be made";
    return service;
}

void
CAssetStoreGlobal::SetWebSignInStores( AUTH::ICredentialStore* sessionStore, AUTH::ICredentialStore* credentialStore )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_webStoreLock );
    m_webSessionStore = sessionStore;
    m_webCredentialStore = credentialStore;
}

/*-------------------------------------------------------------------------*/

AUTH::ICredentialStore*
CAssetStoreGlobal::GetWebSessionStore( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_webStoreLock );
    return m_webSessionStore;
}

/*-------------------------------------------------------------------------*/

AUTH::ICredentialStore*
CAssetStoreGlobal::GetWebCredentialStore( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_webStoreLock );
    return m_webCredentialStore;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
