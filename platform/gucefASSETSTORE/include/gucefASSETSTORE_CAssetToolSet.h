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

#ifndef GUCEF_ASSETSTORE_CASSETTOOLSET_H
#define GUCEF_ASSETSTORE_CASSETTOOLSET_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CJOBSCHEDULER_H
#include "gucefCORE_CJobScheduler.h"
#define GUCEF_CORE_CJOBSCHEDULER_H
#endif /* GUCEF_CORE_CJOBSCHEDULER_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#include "gucefASSETSTORE_CAssetToolContext.h"
#define GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLCONTEXT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMERFACTORY_H
#include "gucefASSETSTORE_CAssetToolJobTaskConsumerFactory.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMERFACTORY_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKCONSUMERFACTORY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATAFACTORY_H
#include "gucefASSETSTORE_CAssetToolJobTaskDataFactory.h"
#define GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATAFACTORY_H
#endif /* GUCEF_ASSETSTORE_CASSETTOOLJOBTASKDATAFACTORY_H ? */

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

/**
 *  The asset tool set: the MCP tools, resources and policy of the asset store service (section 7 and 8 of the asset
 *  tools design), written only against CIAssetStoreService, so an MCP client sees the same behaviour with the
 *  in-process service (CLocalAssetStoreService, the standalone host tools/AssetToolsMcp) and with a remote proxy.
 *  Any host registers it into its COM::CMcpServer:
 *
 *      ASSETSTORE::CAssetToolSet toolSet( service, settings );
 *      toolSet.AddSecret( databasePassword );           // every secret the process knows
 *      toolSet.RegisterInto( server, error );
 *      ...
 *      toolSet.Unregister( 5000 );                       // closes the call gate, waits for the calls inside
 *
 *  RegisterInto() makes the job scheduler of the set ("assettools.<name>", lanes assets.conversion, assets.validation
 *  and assets.ingest of one each) on the pulse generator of the server, registers the task type of its jobs as
 *  submittable, makes the job bridge, registers every tool of CAssetToolCatalogue, the resources, the access policy
 *  (CAssetToolsAccessPolicy) and the server instructions, and starts watching the exposure file (re-registering the
 *  tools when it changes, which announces the list change) and the vocabulary of the store (store_search takes its
 *  enumerations from it).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetToolSet : public CORE::CObservingNotifier
{
    public:

    typedef CORE::CTEventHandlerFunctor< CAssetToolSet > TEventCallback;

    /** The instructions for the model (server/discover) */
    static const CORE::CString Instructions;

    CAssetToolSet( const TAssetStoreServicePtr& service, const CAssetToolSetSettings& settings );

    virtual ~CAssetToolSet() GUCEF_VIRTUAL_OVERRIDE;

    /** Registers the set into the server; false with the reason when it cannot (nothing stays registered then) */
    bool RegisterInto( COM::CMcpServer& server, CORE::CString& error );

    /** Takes the set out of the server: no call enters any more, the calls inside are cancelled and waited for */
    void Unregister( UInt32 timeoutMs = 5000 );

    bool IsRegistered( void ) const;

    /** A secret the process knows, for the output filter (the database password, store tokens, a bearer token) */
    bool AddSecret( const CORE::CString& secret );

    bool RemoveSecret( const CORE::CString& secret );

    CAssetToolContext& GetContext( void );

    COM::CMcpJobBridge* GetJobBridge( void );

    CORE::JobSchedulerPtr GetScheduler( void ) const;

    /** The task type name of the jobs of the set */
    const CORE::CString& GetTaskTypeName( void ) const;

    /** Looks at the exposure file now (the timer does it every exposureCheckIntervalMs); true when it changed */
    bool CheckExposure( void );

    /** The names of the tools of the set, in the order of tools/list */
    static void GetToolNames( CORE::CString::StringVector& names );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CAssetToolSet( const CAssetToolSet& src );              /**< not implemented */
    CAssetToolSet& operator=( const CAssetToolSet& src );   /**< not implemented */

    bool RegisterTools( CORE::CString& error );
    bool RegisterResources( CORE::CString& error );
    bool StartJobs( CORE::CString& error );
    void StopJobs( void );
    void RefreshVocabulary( void );

    void OnExposureTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCatalogueChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CAssetToolContext m_context;
    CORE::CString m_taskTypeName;
    CAssetToolJobTaskConsumerFactory m_taskFactory;
    CAssetToolJobTaskDataFactory m_taskDataFactory;
    COM::CMcpServer* m_server;
    CORE::JobSchedulerPtr m_scheduler;
    COM::CMcpJobBridge* m_bridge;
    CORE::CTimer* m_exposureTimer;
    GUCEF::vector< COM::CIMcpToolPtr > m_tools;
    CORE::CString::StringVector m_resourceUris;
    CORE::CString::StringVector m_templateNames;
    bool m_hasVocabulary;
    CAssetVocabularyInfo m_vocabulary;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETTOOLSET_H ? */
