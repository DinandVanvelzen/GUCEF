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

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetStoreProviderOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPROVIDEROPERATIONTASKCONSUMER_H ? */
#ifndef GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CPrimaryAssetStoreOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CPRIMARYASSETSTOREOPERATIONTASKCONSUMER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETANALYSISTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetAnalysisTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETANALYSISTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETANALYSISTASKCONSUMER_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CWEBSESSIONSIGNINTASKCONSUMER_H
#include "gucefASSETSTORE_CWebSessionSignInTaskConsumer.h"
#define GUCEF_ASSETSTORE_CWEBSESSIONSIGNINTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CWEBSESSIONSIGNINTASKCONSUMER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADTASKCONSUMER_H
#include "gucefASSETSTORE_CAssetStoreDownloadTaskConsumer.h"
#define GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREDOWNLOADTASKCONSUMER_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */
#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H
#include "gucefASSETSTORE_CLocalAssetStoreServiceOperationTaskConsumer.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICEOPERATIONTASKCONSUMER_H ? */
/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CModule;
class CAssetStoreRegistry;
class CAssetContentAnalyzerRegistry;

/**
 *  The global systems of gucefASSETSTORE, created on first use after the globals of the modules it depends on and
 *  destroyed when the module unloads. It owns, made in this order: the factory of the store providers (store plugins
 *  register their concrete factories in it), the registry of the stores, the registry of the content analysers with the
 *  built in texture analyser (analyser plugins add theirs), and the factories of the task types of the module, which it
 *  registers with the task manager. The module is listed for plugin link-back, so store and analyser plugins find it.
 *
 *  It also owns the factory of the implementations of the asset store service (CIAssetStoreService) by implementation
 *  id, with the in-process implementation registered as "local" (CLocalAssetStoreService). An implementation that
 *  reaches the store of a server registers its concrete factory under its own id ("remote") when it is built and
 *  loaded, and unregisters it before it goes; an application chooses the implementation by id from its settings
 *  (CreateService()).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreGlobal
{
    public:

    static CAssetStoreGlobal* Instance( void );

    /** The concrete factories of the store providers by provider id */
    CAssetStoreProviderFactory& GetProviderFactory( void );

    /** The stores the application works with */
    CAssetStoreRegistry& GetRegistry( void );

    /** The content analysers of the ingest pipeline */
    CAssetContentAnalyzerRegistry& GetContentAnalyzerRegistry( void );

    /** The concrete factories of the implementations of the asset store service by implementation id ("local", "remote") */
    CAssetStoreServiceFactory& GetServiceFactory( void );

    /**
     *  Makes the implementation of the asset store service that config.implementationId names (the local one for an empty id);
     *  null with the reason in 'error' when no implementation of that id is registered in this build
     */
    TAssetStoreServicePtr CreateService( const CAssetStoreServiceConfig& config, CORE::CString& error );

    /**
     *  The credential stores of the web session sign ins (CWebSessionAssetStoreProvider::ConfigureWebSignIn() uses them
     *  when it is given none), set by the application before the store plugins make their providers: the session store
     *  (a key file store, so a session is used again without the keyring code) and the store of saved credentials (the
     *  keyring code store, section 12.2 of the design). Not owned, they must outlive the providers; GUCEF_NULL for none
     */
    void SetWebSignInStores( AUTH::ICredentialStore* sessionStore, AUTH::ICredentialStore* credentialStore );
    AUTH::ICredentialStore* GetWebSessionStore( void ) const;
    AUTH::ICredentialStore* GetWebCredentialStore( void ) const;

    private:

    friend class CModule;

    static void Deinstance( void );

    void Initialize( void );

    CAssetStoreGlobal( void );
    ~CAssetStoreGlobal();
    CAssetStoreGlobal( const CAssetStoreGlobal& src );              /**< not implemented */
    CAssetStoreGlobal& operator=( const CAssetStoreGlobal& src );   /**< not implemented */

    private:

    CAssetStoreProviderFactory* m_providerFactory;
    CAssetStoreRegistry* m_registry;
    CAssetContentAnalyzerRegistry* m_analyzerRegistry;
    TAssetStoreProviderOperationTaskConsumerFactory m_providerOperationFactory;
    TAssetStoreDownloadTaskConsumerFactory m_downloadTaskFactory;
    TPrimaryAssetStoreOperationTaskConsumerFactory m_primaryStoreOperationFactory;
    TAssetAnalysisTaskConsumerFactory m_analysisTaskFactory;
    TWebSessionSignInTaskConsumerFactory m_webSignInTaskFactory;
    TLocalAssetStoreServiceOperationTaskConsumerFactory m_serviceOperationFactory;
    CAssetStoreServiceFactory* m_serviceFactory;
    TLocalAssetStoreServiceFactory m_localServiceFactory;
    AUTH::ICredentialStore* m_webSessionStore;
    AUTH::ICredentialStore* m_webCredentialStore;
    mutable MT::CMutex m_webStoreLock;

    static MT::CMutex g_dataLock;
    static CAssetStoreGlobal* g_instance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */
