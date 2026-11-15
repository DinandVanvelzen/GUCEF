/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTASSETSTORESERVICESUITE_H
#define TESTASSETSTORESERVICESUITE_H

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

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#include "gucefASSETSTORE_CAssetContentAnalyzerRegistry.h"
#define GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETCONTENTANALYZERREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */

#ifndef TESTMODELANALYZER_H
#include "TestModelAnalyzer.h"
#define TESTMODELANALYZER_H
#endif /* TESTMODELANALYZER_H ? */

#ifndef TESTFAKECONVERTERHOOK_H
#include "TestFakeConverterHook.h"
#define TESTFAKECONVERTERHOOK_H
#endif /* TESTFAKECONVERTERHOOK_H ? */

#ifndef TESTFAKESTORESERVER_H
#include "TestFakeStoreServer.h"
#define TESTFAKESTORESERVER_H
#endif /* TESTFAKESTORESERVER_H ? */

#ifndef TESTFAKESTOREPROVIDER_H
#include "TestFakeStoreProvider.h"
#define TESTFAKESTOREPROVIDER_H
#endif /* TESTFAKESTOREPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ASSETSTORETEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a run of the equivalence suite of the asset store service works with (section 10.2 of the service design): the
 *  implementation under test. The suite prepares the configuration (the storage locations, the downloads, the converter
 *  hook) and asks the target to complete it with its database and to make the service. The in-process target makes a
 *  CLocalAssetStoreService through the factory of the module; a remote target makes a service of the server side over
 *  the same configuration and connects a client to it, so every body below runs unchanged against both.
 */
class CAssetStoreServiceSuiteTarget
{
    public:

    virtual ~CAssetStoreServiceSuiteTarget() {}

    /** The label of the run, in the names of the test cases and the folders of the run */
    virtual GUCEF::CORE::CString GetLabel( void ) const = 0;

    /** Gives the configuration its database and its credential store */
    virtual void ConfigureDatabase( GUCEF::ASSETSTORE::CAssetStoreServiceConfig& config ) = 0;

    /** The service of the configuration, not opened yet; null when it cannot be made */
    virtual GUCEF::ASSETSTORE::TAssetStoreServicePtr MakeService( const GUCEF::ASSETSTORE::CAssetStoreServiceConfig& config ) = 0;

    /** Pulses what delivers the events of the service, for the time */
    virtual void Pump( GUCEF::UInt32 durationInMs ) = 0;
};

/*-------------------------------------------------------------------------*/

/** Counts the events of a service and keeps the last data of the job and catalogue events */
class CServiceEventRecorder : public GUCEF::CORE::CObservingNotifier
{
    public:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CServiceEventRecorder > TEventCallback;

    GUCEF::UInt32 catalogueChanges;
    GUCEF::UInt32 jobChanges;
    GUCEF::UInt32 jobRemovals;
    GUCEF::ASSETSTORE::TAssetCatalogueChangeVector changes;
    GUCEF::ASSETSTORE::CAssetJobEntry lastJob;
    GUCEF::ASSETSTORE::CAssetJobEntry lastRemovedJob;

    explicit CServiceEventRecorder( GUCEF::ASSETSTORE::CIAssetStoreService& service )
        : GUCEF::CORE::CObservingNotifier()
        , catalogueChanges( 0 )
        , jobChanges( 0 )
        , jobRemovals( 0 )
        , changes()
        , lastJob()
        , lastRemovedJob()
    {
        TEventCallback changed( this, &CServiceEventRecorder::OnCatalogueChanged );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::CatalogueChangedEvent, changed );
        TEventCallback job( this, &CServiceEventRecorder::OnJobChanged );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::JobChangedEvent, job );
        TEventCallback removed( this, &CServiceEventRecorder::OnJobRemoved );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::JobRemovedEvent, removed );
    }

    virtual ~CServiceEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    bool HasChange( GUCEF::ASSETSTORE::TAssetCatalogueChangeKind kind, const GUCEF::CORE::CString& id ) const
    {
        for ( size_t i=0; i<changes.size(); ++i )
            if ( changes[ i ].kind == kind && ( id.IsNULLOrEmpty() || ( !changes[ i ].ids.empty() && changes[ i ].ids[ 0 ] == id ) ) )
                return true;
        return false;
    }

    private:

    void OnCatalogueChanged( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        ++catalogueChanges;
        const GUCEF::ASSETSTORE::CAssetCatalogueChange* change = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetCatalogueChange >( eventData );
        if ( GUCEF_NULL != change )
            changes.push_back( *change );
    }

    void OnJobChanged( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        ++jobChanges;
        const GUCEF::ASSETSTORE::CAssetJobEntry* job = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetJobEntry >( eventData );
        if ( GUCEF_NULL != job )
            lastJob = *job;
    }

    void OnJobRemoved( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        ++jobRemovals;
        const GUCEF::ASSETSTORE::CAssetJobEntry* job = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetJobEntry >( eventData );
        if ( GUCEF_NULL != job )
            lastRemovedJob = *job;
    }
};

/*-------------------------------------------------------------------------*/

/** The converter of the suite: the test converter with two operations of the pipeline, a query and a job that reports progress */
class CServiceSuiteConverterHook : public CTestConverterHook
{
    public:

    CServiceSuiteConverterHook( void ) : CTestConverterHook() {}

    virtual bool SupportsPipelineOperation( const GUCEF::CORE::CString& operation ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return GUCEF::ASSETSTORE::CAssetPipelineOperations::PackList == operation || GUCEF::ASSETSTORE::CAssetPipelineOperations::ConvertPack == operation;
    }

    virtual GUCEF::ASSETSTORE::TAssetStoreStatus RunPipelineOperation( const GUCEF::ASSETSTORE::CAssetPipelineRequest& request ,
                                                                       GUCEF::ASSETSTORE::CIAssetConversionObserver* observer ,
                                                                       GUCEF::ASSETSTORE::CAssetServiceDocument& result       ) GUCEF_VIRTUAL_OVERRIDE
    {
        result.Clear();
        result.document.SetName( "result" );
        if ( GUCEF::ASSETSTORE::CAssetPipelineOperations::PackList == request.operation )
        {
            result.document.SetAttribute( "packs", (GUCEF::UInt32) 1 );
            result.document.SetAttribute( "text", request.arguments.GetAttributeValueOrChildValueByName( "text" ).AsString() );
            return GUCEF::ASSETSTORE::ASSETSTORE_STATUS_OK;
        }
        if ( GUCEF::ASSETSTORE::CAssetPipelineOperations::ConvertPack == request.operation )
        {
            for ( GUCEF::UInt32 i=1; i<=2; ++i )
            {
                if ( GUCEF_NULL != observer && observer->IsCancelRequested() )
                    return GUCEF::ASSETSTORE::ASSETSTORE_STATUS_CANCELLED;
                if ( GUCEF_NULL != observer )
                    observer->OnConversionProgress( i, 2, "asset" + GUCEF::CORE::ToString( i ) );
            }
            result.document.SetAttribute( "converted", (GUCEF::UInt32) 2 );
            result.document.SetAttribute( "packId", request.packId );
            return GUCEF::ASSETSTORE::ASSETSTORE_STATUS_OK;
        }
        return CTestConverterHook::RunPipelineOperation( request, observer, result );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Pumps until the job of the service has ended (or the time is up); true when it has */
inline bool
ServiceSuiteWaitForJob( CAssetStoreServiceSuiteTarget& target, GUCEF::ASSETSTORE::CIAssetStoreService& service, const GUCEF::CORE::CString& jobId,
                        GUCEF::ASSETSTORE::CAssetJobEntry& job, GUCEF::UInt32 timeoutInMs = 30000 )
{
    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs; waited+=10 )
    {
        if ( GUCEF::ASSETSTORE::ASSETSTORE_STATUS_OK == service.GetJob( jobId, job ) && job.IsEnded() )
            return true;
        target.Pump( 10 );
    }
    return false;
}

/** Pumps until a future of the service is ready; true when it is */
inline bool
ServiceSuiteWaitForFuture( CAssetStoreServiceSuiteTarget& target, const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutInMs = 30000 )
{
    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs; waited+=5 )
    {
        if ( future.IsReady() || future.HasNoFuture() )
            return true;
        target.Pump( 5 );
    }
    return false;
}

/** The first job of the kind in the list of the service that has the pack, or of any pack for an empty id */
inline bool
ServiceSuiteFindJob( GUCEF::ASSETSTORE::CIAssetStoreService& service, const GUCEF::CORE::CString& kind, GUCEF::ASSETSTORE::CAssetJobEntry& job )
{
    GUCEF::ASSETSTORE::CAssetJobPage page;
    if ( GUCEF::ASSETSTORE::ASSETSTORE_STATUS_OK != service.ListJobs( GUCEF::CORE::CString::Empty, GUCEF::ASSETSTORE::CIAssetStoreService::MaxPageEntries, page ) )
        return false;
    for ( size_t i=0; i<page.entries.size(); ++i )
        if ( page.entries[ i ].kind == kind )
        {
            job = page.entries[ i ];
            return true;
        }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  The equivalence suite: the same bodies for every implementation of CIAssetStoreService, every method group exercised
 *  in its synchronous and its asynchronous form where both matter. Results of operations that an implementation does
 *  not carry out are checked to be UNSUPPORTED with a reason, never a silent success.
 */
inline void
RunAssetStoreServiceSuite( CAssetStoreServiceSuiteTarget& target )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;
    using namespace GUCEF::ASSETSTORE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CString label = target.GetLabel();
    const CString base = "astest/svc_" + label;
    MountFreshDirectory( "svc_" + label + "_main", base + "/main" );
    MountFreshDirectory( "svc_" + label + "_mirror", base + "/mirror" );
    MountFreshDirectory( "svc_" + label + "_source", base + "/source" );

    // the analyser of the test models, as the primary store tests have it
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    const bool ownAnalyzer = !analyzers.IsRegistered( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    if ( ownAnalyzer )
        analyzers.Register( analyzerPtr );

    // the packs on disk
    const CString packA = base + "/source/Medieval_Weapons";
    const CString packB = base + "/source/Barrels";
    bool written = WriteVfsFile( packA + "/pack.json", CString( "{ \"title\": \"Medieval Weapons\", \"author\": \"Forge Works\", \"tags\": [ \"sword\", \"medieval\" ], "
                                                               "\"licence\": { \"name\": \"CC BY 4.0\", \"restrictions\": [ \"noMachineLearning\" ] } }" ) );
    written = WriteVfsFile( packA + "/Meshes/SM_Wep_Sword_01.fbx", CString( "triangles=1200;vertices=900;rigged=0;animated=0" ) ) && written;
    written = WriteVfsFile( packA + "/Meshes/SM_Wep_Axe_01.fbx", CString( "triangles=60000;vertices=40000;rigged=0" ) ) && written;
    CDynamicBuffer png;
    written = MakePng( 64, 32, false, png ) && WriteVfsFile( packA + "/Textures/T_Sword_D.png", png ) && written;
    written = WriteVfsFile( packB + "/SM_Barrel_01.fbx", CString( "triangles=400;vertices=300" ) ) && written;
    written = WriteVfsFile( packB + "/SM_Barrel_02.fbx", CString( "triangles=450;vertices=320" ) ) && written;

    // an online store on the loopback interface
    CFakeStoreServer server;
    CFakeStoreProviderFactory storeFactory( "svcstore" );
    {
        CAssetStoreItem item;
        item.itemId = "svc-item-1";
        item.title = "Service Crate";
        item.seller = "Test Seller";
        item.listingType = "3d-model";
        item.licenceName = "Standard License";
        CAssetStoreFormat format;
        format.formatId = "fbx";
        format.name = "FBX";
        CDynamicBuffer content;
        content.Append( "triangles=96;vertices=64", 24 );
        server.SetFileContent( "svcf1", content );
        CAssetStoreFile file;
        file.fileId = "svcf1";
        file.name = "SM_Crate_01.fbx";
        file.size = content.GetDataSize();
        file.digestAlgorithm = "sha256";
        file.digest = Sha256Of( content );
        format.files.push_back( file );
        item.formats.push_back( format );
        server.AddItem( item );
    }
    const bool serverStarted = server.Start();
    CFakeStoreProvider::SetServer( &server );
    CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "svcstore", &storeFactory );
    CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();

    // the configuration of the run
    CServiceSuiteConverterHook hook;
    CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = base + "/main";
    config.primaryStore.locations.push_back( main );
    CAssetStorageLocation mirror;
    mirror.locationId = "mirror";
    mirror.name = "Mirror";
    mirror.vfsRoot = base + "/mirror";
    config.primaryStore.locations.push_back( mirror );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.analysisConcurrency = 2;
    CAssetSourcePolicy policy;
    policy.sourceId = "svcstore";
    policy.mode = ASSETINGEST_MODE_MANUAL;
    config.primaryStore.SetPolicy( policy );
    config.loadDownloadQueue = false;
    config.converterHook = &hook;
    target.ConfigureDatabase( config );

    TAssetStoreServicePtr servicePtr = target.MakeService( config );
    CString packAId, packBId, swordId, ingestJobId;

    GUCEF_TESTFW_TESTCASE( ( label + " service: made by the factory, opened, it describes itself; an implementation that is not built is refused with the reason" ).C_String() )
        try
        {
            ASSERT_TRUE( written && serverStarted );
            ASSERT_FALSE( servicePtr.IsNULL() );
            if ( !servicePtr.IsNULL() )
            {
                CIAssetStoreService& service = *servicePtr;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Open() );
                ASSERT_TRUE( service.IsOpen() && service.IsCatalogueAvailable() && service.GetStatusMessage().IsNULLOrEmpty() );
                CAssetServiceInfo info;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetServiceInfo( info ) );
                ASSERT_TRUE( info.implementationId == service.GetType() && info.inProcess == service.IsInProcess() );
                ASSERT_TRUE( 2 == info.locations.size() && info.locations[ 0 ].online && info.HasCapability( "catalogue" ) && info.HasCapability( "conversion" ) );
                ASSERT_TRUE( info.HasCapability( "pipeline" ) && !info.hasRenderer && info.converterName == hook.GetName() );
                bool hasStore = false;
                for ( size_t i=0; i<info.stores.size(); ++i )
                    hasStore = hasStore || "svcstore" == info.stores[ i ].storeId;
                ASSERT_TRUE( hasStore && 0 == info.catalogueRevision );
                CFutureResult future = service.GetServiceInfoAsync();
                ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
                CAssetServiceInfo asyncInfo;
                ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncInfo ) && asyncInfo.locations.size() == info.locations.size() );
            }
            CAssetStoreServiceConfig remote;
            remote.implementationId = CAssetStoreServiceConfig::RemoteImplementationId;
            CString error;
            ASSERT_TRUE( CAssetStoreGlobal::Instance()->CreateService( remote, error ).IsNULL() && "remote service not available in this build" == error );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    if ( servicePtr.IsNULL() || !servicePtr->IsCatalogueAvailable() )
    {
        CFakeStoreProvider::SetServer( GUCEF_NULL );
        server.Stop();
        return;
    }
    CIAssetStoreService& service = *servicePtr;
    CServiceEventRecorder events( service );

    GUCEF_TESTFW_TESTCASE( ( label + " service: ingests on the calling thread and as a future, duplicates, command ids, the change journal and its events" ).C_String() )
        try
        {
            CAssetIngestRequest request;
            request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
            request.contentPath = packA;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Ingest( "ingest-a", request, result ) && !result.packId.IsNULLOrEmpty() );
            packAId = result.packId;
            // a repeat with the command id answers the first outcome and ingests nothing
            CAssetIngestResult repeat;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Ingest( "ingest-a", request, repeat ) && repeat.packId == packAId && repeat.ingestId == result.ingestId );

            CAssetIngestRequest requestB = request;
            requestB.contentPath = packB;
            requestB.keep = "both";
            CFutureResult future = service.IngestAsync( CString::Empty, requestB );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future, 60000 ) );
            CAssetIngestResult asyncResult;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncResult ) && ASSETSTORE_STATUS_OK == asyncResult.status );
            packBId = asyncResult.packId;

            // the same pack again is an outcome, not a failure
            CFutureResult duplicate = service.IngestAsync( CString::Empty, request );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, duplicate, 60000 ) );
            CAssetIngestResult duplicateResult;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( duplicate, duplicateResult ) && ASSETSTORE_STATUS_DUPLICATE == duplicateResult.status );
            target.Pump( 100 );

            CAssetCatalogueChangePage changes;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetCatalogueChanges( 0, 0, changes ) && changes.revision >= 2 );
            bool addedA = false, addedB = false;
            for ( size_t i=0; i<changes.entries.size(); ++i )
            {
                addedA = addedA || ( ASSETCHANGE_PACK_ADDED == changes.entries[ i ].kind && !changes.entries[ i ].ids.empty() && changes.entries[ i ].ids[ 0 ] == packAId );
                addedB = addedB || ( ASSETCHANGE_PACK_ADDED == changes.entries[ i ].kind && !changes.entries[ i ].ids.empty() && changes.entries[ i ].ids[ 0 ] == packBId );
            }
            ASSERT_TRUE( addedA && addedB );
            CAssetCatalogueChangePage none;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetCatalogueChanges( changes.revision, 0, none ) && none.entries.empty() );
            ASSERT_TRUE( events.HasChange( ASSETCHANGE_PACK_ADDED, packAId ) && events.jobChanges > 0 );
            ASSERT_TRUE( ServiceSuiteFindJob( service, "ingest", events.lastJob ) && ASSETJOB_STATE_DONE == events.lastJob.state );
            ingestJobId = events.lastJob.jobId;
            CAssetServiceStringList pending;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetPendingIngests( pending ) && pending.values.empty() );
            CAssetServiceStringList roots;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ListImportRoots( roots ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: search with filters and facets in both forms, suggestions and the vocabulary" ).C_String() )
        try
        {
            CAssetSearchQuery all;
            all.pageSize = 100;
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( all, page ) && page.hits.size() >= 5 && page.revision >= 2 );
            for ( size_t i=0; i<page.hits.size(); ++i )
                if ( "SM_Wep_Sword_01" == page.hits[ i ].name )
                    swordId = page.hits[ i ].assetId;
            ASSERT_FALSE( swordId.IsNULLOrEmpty() );
            ASSERT_TRUE( !page.facets.empty() );
            CFutureResult future = service.SearchAsync( all );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
            CAssetSearchResultPage asyncPage;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncPage ) && asyncPage.total == page.total && asyncPage.hits.size() == page.hits.size() );

            CAssetSearchQuery text;
            text.text = "sword";
            text.pageSize = 100;
            CAssetSearchResultPage swords;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( text, swords ) && swords.HasHit( swordId ) );
            CAssetSearchQuery models;
            models.kinds.insert( "static-model" );
            models.minTriangles = 1000;
            models.pageSize = 100;
            CAssetSearchResultPage big;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( models, big ) && big.HasHit( swordId ) && big.hits.size() < page.hits.size() );

            CAssetServiceStringList suggestions;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Suggest( "swo", 10, suggestions ) );
            bool hasSword = false;
            for ( size_t i=0; i<suggestions.values.size(); ++i )
                hasSword = hasSword || "sword" == suggestions.values[ i ];
            ASSERT_TRUE( hasSword );
            CFutureResult suggestFuture = service.SuggestAsync( "swo", 10 );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, suggestFuture ) );
            CAssetServiceStringList asyncSuggestions;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( suggestFuture, asyncSuggestions ) && asyncSuggestions.values == suggestions.values );

            CAssetVocabularyInfo vocabulary;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetVocabulary( vocabulary ) && !vocabulary.kinds.empty() && !vocabulary.categories.empty() );
            ASSERT_TRUE( 5 == vocabulary.restrictionFlags.size() && vocabulary.mappingCountsBySource.find( "svcstore" ) != vocabulary.mappingCountsBySource.end() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: saved searches page, save with a command id once, run and delete; a command id is bound to its operation" ).C_String() )
        try
        {
            CAssetSavedSearchPage first;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSavedSearches( CString::Empty, 10, first ) && 10 == first.entries.size() && !first.nextPageToken.IsNULLOrEmpty() );
            UInt64 seen = first.entries.size();
            CString token = first.nextPageToken;
            while ( !token.IsNULLOrEmpty() )
            {
                CAssetSavedSearchPage next;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSavedSearches( token, 10, next ) );
                seen += next.entries.size();
                token = next.nextPageToken;
                if ( next.entries.empty() )
                    break;
            }
            ASSERT_TRUE( seen == first.total && first.total >= 25 );

            CAssetSavedSearch search;
            search.searchId = "user-svc-swords";
            search.name = "Swords";
            search.query.text = "sword";
            search.sortOrder = 10000;
            CAssetServiceResult saved;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SaveSearch( "save-1", search, saved ) && !saved.replayed && "user-svc-swords" == saved.id );
            CAssetServiceResult again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SaveSearch( "save-1", search, again ) && again.replayed );
            CAssetServiceResult reused;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.DeleteSavedSearch( "save-1", search.searchId, reused ) );
            CAssetSearchResultPage results;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunSavedSearch( search.searchId, 0, results ) && results.HasHit( swordId ) );
            CFutureResult future = service.DeleteSavedSearchAsync( "delete-1", search.searchId );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) && ASSETSTORE_STATUS_OK == CIAssetStoreService::GetStatusOfFuture( future ) );
            CAssetSavedSearch invalid;
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.SaveSearch( CString::Empty, invalid, refused ) && !refused.message.IsNULLOrEmpty() );
            target.Pump( 50 );
            ASSERT_TRUE( events.HasChange( ASSETCHANGE_SAVED_SEARCHES, search.searchId ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: packs, assets in pages, one asset, relations, the file set, the licence and the ingest history; errors" ).C_String() )
        try
        {
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetPack( packAId, pack ) && "Medieval Weapons" == pack.title );
            CAssetRecordPage firstPage;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetAssets( packAId, CString::Empty, 2, firstPage ) && 2 == firstPage.entries.size() && firstPage.total >= 3 );
            CAssetRecordPage secondPage;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetAssets( packAId, firstPage.nextPageToken, 2, secondPage ) && firstPage.total == firstPage.entries.size() + secondPage.entries.size() );
            CAssetRecord sword;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetAsset( swordId, sword ) && "SM_Wep_Sword_01" == sword.name && sword.packId == packAId );
            CAssetRelationPage relations;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetRelations( swordId, CString::Empty, 0, relations ) );
            CAssetFileSet files;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetAssetFileSet( swordId, files ) && ASSETSTORE_STATUS_OK == files.mainStatus && vfs.FileExists( files.mainVfsPath ) );
            ASSERT_TRUE( !files.files.empty() && files.files[ 0 ].path == files.mainVfsPath && !files.files[ 0 ].digest.IsNULLOrEmpty() && files.totalBytes > 0 );
            CAssetLicence licence;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetLicence( packAId, licence ) && licence.HasRestriction( CAssetLicence::NoMachineLearning ) );
            CAssetLicence byId;
            ASSERT_TRUE( licence.licenceId.IsNULLOrEmpty() || ( ASSETSTORE_STATUS_OK == service.GetLicence( licence.licenceId, byId ) && byId.licenceClass == licence.licenceClass ) );
            CAssetIngestHistoryPage history;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetIngestHistory( packAId, CString::Empty, 0, history ) && !history.entries.empty() );
            CFutureResult future = service.GetPackAsync( packBId );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
            CAssetPackRecord asyncPack;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncPack ) && asyncPack.packId == packBId );

            CAssetPackRecord missing;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetPack( "no-such-pack", missing ) );
            CAssetRecord missingAsset;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetAsset( "no-such-asset", missingAsset ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.GetAsset( CString::Empty, missingAsset ) );
            CFutureResult failed = service.GetPackAsync( "no-such-pack" );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, failed ) && ASSETSTORE_STATUS_NOT_FOUND == CIAssetStoreService::GetStatusOfFuture( failed ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: files of the service by path: their kind, size and digest, a folder listing and blocks of a file" ).C_String() )
        try
        {
            CAssetFileSet files;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetAssetFileSet( swordId, files ) );
            CString::StringVector paths;
            paths.push_back( files.mainVfsPath );
            paths.push_back( base + "/main" );
            paths.push_back( base + "/main/nothing.bin" );
            CAssetFileStatPage stats;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StatFiles( paths, stats ) && 3 == stats.entries.size() );
            ASSERT_TRUE( "file" == stats.entries[ 0 ].role && "directory" == stats.entries[ 1 ].role && "missing" == stats.entries[ 2 ].role );
            ASSERT_TRUE( !files.files.empty() && stats.entries[ 0 ].digest == files.files[ 0 ].digest && stats.entries[ 0 ].sizeBytes == files.files[ 0 ].sizeBytes );
            const Int32 slash = files.mainVfsPath.HasChar( '/', false );
            const CString folder = slash > 0 ? files.mainVfsPath.SubstrToIndex( (UInt32) slash, true ) : CString::Empty;
            CAssetFileStatPage listing;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ListDirectory( folder, CString::Empty, 0, listing ) && listing.entries.size() >= 2 );
            CAssetFileBlock block;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReadFileBlock( files.mainVfsPath, 0, 9, block ) && 9 == block.data.GetDataSize() && !block.endOfFile );
            ASSERT_TRUE( 0 == memcmp( block.data.GetConstBufferPtr(), "triangles", 9 ) );
            CAssetFileBlock rest;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReadFileBlock( files.mainVfsPath, 10, 1024, rest ) && rest.endOfFile && rest.fileSize == (UInt64) stats.entries[ 0 ].sizeBytes );
            CAssetFileBlock beyond;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.ReadFileBlock( files.mainVfsPath, rest.fileSize + 1, 16, beyond ) );
            CFutureResult future = service.StatFilesAsync( paths );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
            CAssetFileStatPage asyncStats;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncStats ) && 3 == asyncStats.entries.size() && asyncStats.entries[ 0 ].digest == stats.entries[ 0 ].digest );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: storage locations with their state, a refresh and a verification in both forms" ).C_String() )
        try
        {
            CAssetLocationPage locations;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetLocations( locations ) && 2 == locations.entries.size() && locations.entries[ 0 ].online && locations.entries[ 1 ].online );
            CAssetLocationPage refreshed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RefreshLocationStates( refreshed ) && 2 == refreshed.entries.size() );
            CAssetVerificationReport report;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.VerifyLocation( CString::Empty, "main", report ) && report.IsIntact() && report.filesChecked > 0 );
            CFutureResult future = service.VerifyLocationAsync( CString::Empty, "main" );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
            CAssetVerificationReport asyncReport;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncReport ) && asyncReport.filesChecked == report.filesChecked );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: curation of the vocabulary changes it once per command; what is not carried out says so" ).C_String() )
        try
        {
            const UInt32 before = events.catalogueChanges;
            CAssetServiceResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SetSynonym( "syn-1", "blade", "sword", result ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SetCategoryMapping( "map-1", "svcstore", "3D/Props/Crates", "props", CString::Empty, "model", result ) );
            CAssetCategoryRule rule;
            rule.ruleId = "svc-rule-crate";
            rule.matchOn = "keyword";
            rule.pattern = "crate";
            rule.categoryId = "props";
            rule.priority = 10;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SetCategoryRule( "rule-1", rule, result ) && "svc-rule-crate" == result.id );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SetCategoryRule( "rule-1", rule, result ) && result.replayed );
            target.Pump( 50 );
            ASSERT_TRUE( events.catalogueChanges == before + 3 && events.HasChange( ASSETCHANGE_VOCABULARY, CString::Empty ) );
            CAssetServiceResult unsupported;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.RemoveCategoryRule( CString::Empty, "svc-rule-crate", unsupported ) && !unsupported.message.IsNULLOrEmpty() );
            CAssetPackRecord changes;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.UpdatePackMetadata( CString::Empty, changes, 0, unsupported ) && !unsupported.message.IsNULLOrEmpty() );
            CAssetServiceDocument preview;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.CountRuleMatches( rule, preview ) && !preview.message.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: a conversion is a job of the list with its events and its result; jobs are removed through the service" ).C_String() )
        try
        {
            CAssetConversionJobRequest request;
            request.packId = packBId;
            CAssetServiceResult accepted;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RequestConversion( "convert-1", request, accepted ) && 0 == accepted.id.HasSubstr( "convert:" ) );
            CAssetJobEntry job;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, accepted.id, job ) && ASSETJOB_STATE_DONE == job.state && "conversion" == job.kind );
            ASSERT_TRUE( job.IsActionAvailable( ASSETJOB_ACTION_REMOVE ) && !job.IsActionAvailable( ASSETJOB_ACTION_CANCEL ) );
            target.Pump( 50 );
            ASSERT_TRUE( events.HasChange( ASSETCHANGE_PACK_UPDATED, packBId ) );
            CAssetServiceDocument summary;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetJobResult( accepted.id, CString::Empty, CString::Empty, 0, summary ) && GUCEF_NULL != summary.document.FindChild( "job" ) );

            CAssetJobPage jobs;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ListJobs( CString::Empty, 0, jobs ) && jobs.total >= 3 );
            CAssetServiceResult removed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ControlJob( CString::Empty, accepted.id, ASSETJOB_ACTION_REMOVE, removed ) );
            target.Pump( 50 );
            ASSERT_TRUE( events.lastRemovedJob.jobId == accepted.id );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetJob( accepted.id, job ) );
            CAssetServiceResult unknown;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.ControlJob( CString::Empty, "no-such-job", ASSETJOB_ACTION_CANCEL, unknown ) );
            CAssetServiceResult ingestRemoved;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ControlJob( CString::Empty, ingestJobId, ASSETJOB_ACTION_REMOVE, ingestRemoved ) );
            CFutureResult future = service.ListJobsAsync( CString::Empty, 0 );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
            CAssetJobPage asyncJobs;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncJobs ) && asyncJobs.total + 2 == jobs.total );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: an online store: its library, an item, a download for the primary store that the service ingests" ).C_String() )
        try
        {
            CAssetStoreInfoPage stores;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ListStores( stores ) );
            bool found = false;
            for ( size_t i=0; i<stores.entries.size(); ++i )
                found = found || ( "svcstore" == stores.entries[ i ].storeId && !stores.entries[ i ].displayName.IsNULLOrEmpty() );
            ASSERT_TRUE( found );
            TAssetStoreProviderPtr provider;
            ASSERT_TRUE( service.GetStoreProvider( "svcstore", provider ) && !service.GetStoreProvider( "no-such-store", provider ) );
            CAssetStoreQuery query;
            CAssetStoreLibraryPage library;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.QueryLibrary( "svcstore", query, library ) && 1 == library.items.size() );
            CAssetStoreItem item;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetStoreItem( "svcstore", "svc-item-1", item ) && "Service Crate" == item.title && 1 == item.formats.size() );
            CFutureResult itemFuture = service.GetStoreItemAsync( "svcstore", "svc-item-1" );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, itemFuture ) );
            CAssetStoreItem asyncItem;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( itemFuture, asyncItem ) && asyncItem.itemId == item.itemId );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.QueryLibrary( "no-such-store", query, library ) );

            CAssetDownloadRequest download;
            download.providerId = "svcstore";
            download.itemId = "svc-item-1";
            download.itemTitle = "Service Crate";
            download.formatId = "fbx";
            download.forIngest = true;
            download.keep = "original";
            download.locationId = "mirror";
            CAssetServiceResult queued;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.EnqueueDownload( "download-1", download, queued ) && !queued.id.IsNULLOrEmpty() );
            CAssetJobEntry job;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, queued.id, job, 60000 ) && ASSETJOB_STATE_DONE == job.state && job.forIngest );
            bool ingested = false;
            for ( UInt32 waited=0; waited<60000 && !ingested; waited+=20 )
            {
                CAssetSearchQuery crates;
                crates.text = "crate";
                CAssetSearchResultPage page;
                ingested = ASSETSTORE_STATUS_OK == service.Search( crates, page ) && !page.hits.empty();
                if ( !ingested )
                    target.Pump( 20 );
            }
            ASSERT_TRUE( ingested );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetJob( queued.id, job ) && job.ingestRequested );

            CAssetServiceResult signOut;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SignOut( CString::Empty, "svcstore", signOut ) );
            CAssetServiceResult signIn;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.BeginSignIn( CString::Empty, "svcstore", signIn ) && !signIn.message.IsNULLOrEmpty() );
            CAssetSignInPromptPage prompts;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSignInPrompts( prompts ) && prompts.entries.empty() );
            CAssetThumbnail picture;
            ASSERT_FALSE( service.TryGetListingImage( "http://127.0.0.1:1/none.png", 128, picture ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: operations of the converter pipeline through the hook, a pipeline job with progress; render, thumbnails, uploads and the keyring say what is not available" ).C_String() )
        try
        {
            CAssetPipelineRequest list;
            list.operation = CAssetPipelineOperations::PackList;
            list.arguments.SetAttribute( "text", "weapons" );
            CAssetServiceDocument listed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RunPipelineQuery( list, listed ) && 1 == listed.document.GetAttributeValueOrChildValueByName( "packs" ).AsUInt32() );
            ASSERT_TRUE( "weapons" == listed.document.GetAttributeValueOrChildValueByName( "text" ).AsString() );
            CFutureResult listFuture = service.RunPipelineQueryAsync( list );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, listFuture ) && ASSETSTORE_STATUS_OK == CIAssetStoreService::GetStatusOfFuture( listFuture ) );

            CAssetPipelineRequest plan;
            plan.operation = CAssetPipelineOperations::ConvertPlan;
            CAssetServiceDocument planned;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.RunPipelineQuery( plan, planned ) && !planned.message.IsNULLOrEmpty() );
            CAssetPipelineRequest convert;
            convert.operation = CAssetPipelineOperations::ConvertPack;
            convert.packId = "converter-pack";
            CAssetServiceDocument wrongWay;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.RunPipelineQuery( convert, wrongWay ) );
            CAssetServiceResult started;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StartPipelineJob( "pipeline-1", convert, started ) && 0 == started.id.HasSubstr( "pipeline:" ) );
            CAssetJobEntry job;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, started.id, job ) && ASSETJOB_STATE_DONE == job.state && 2 == job.itemsDone && 2 == job.itemsTotal );
            CAssetServiceDocument result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetJobResult( started.id, "summary", CString::Empty, 0, result ) && 2 == result.document.GetAttributeValueOrChildValueByName( "converted" ).AsUInt32() );
            CAssetServiceResult removed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_REMOVE, removed ) );
            CAssetPipelineRequest validate;
            validate.operation = CAssetPipelineOperations::ValidatePack;
            CAssetServiceResult notStarted;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.StartPipelineJob( CString::Empty, validate, notStarted ) && !notStarted.message.IsNULLOrEmpty() );

            CAssetPipelineRequest sheet;
            sheet.operation = CAssetPipelineOperations::RenderSheet;
            CAssetServiceDocument rendered;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.RenderSheet( sheet, rendered ) && !rendered.message.IsNULLOrEmpty() );
            CString::StringVector ids;
            ids.push_back( swordId );
            CAssetThumbnailPage thumbnails;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.GetThumbnails( ids, 128, thumbnails ) );
            CAssetServiceResult upload;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.BeginUpload( CString::Empty, "pack", 10, 1, upload ) && !upload.message.IsNULLOrEmpty() );
            CAssetServiceResult keyring;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.UnlockKeyring( CString::Empty, AUTH::CSecretString( CString( "code" ) ), keyring ) );
            CAssetServiceDocument resources;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResourceStatus( resources ) && resources.document.GetAttributeValueOrChildValueByName( "catalogueAvailable" ).AsBool() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: a pack is removed through the service; an unknown call is refused" ).C_String() )
        try
        {
            CAssetServiceResult removed;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.RemovePack( "remove-b", packBId, false, removed ) );
            CAssetPackRecord pack;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetPack( packBId, pack ) );
            target.Pump( 50 );
            ASSERT_TRUE( events.HasChange( ASSETCHANGE_PACK_REMOVED, packBId ) );
            CAssetServiceCall call( "Catalogue/NoSuchOperation" );
            CICloneable* result = GUCEF_NULL;
            CString message;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.Dispatch( call, result, message ) && GUCEF_NULL == result && !message.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    // the end of the run
    servicePtr->Close();
    CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "svcstore" );
    CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "svcstore" );
    CFakeStoreProvider::SetServer( GUCEF_NULL );
    server.Stop();
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTASSETSTORESERVICESUITE_H ? */
