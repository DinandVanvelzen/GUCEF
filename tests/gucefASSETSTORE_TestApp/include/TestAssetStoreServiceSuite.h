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

#ifndef GUCEF_AUTH_CFILECREDENTIALSTORE_H
#include "gucefAUTH_CFileCredentialStore.h"
#define GUCEF_AUTH_CFILECREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CFILECREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CKEYRINGSESSION_H
#include "gucefAUTH_CKeyringSession.h"
#define GUCEF_AUTH_CKEYRINGSESSION_H
#endif /* GUCEF_AUTH_CKEYRINGSESSION_H ? */

#ifndef GUCEF_AUTH_CWEBSESSION_H
#include "gucefAUTH_CWebSession.h"
#define GUCEF_AUTH_CWEBSESSION_H
#endif /* GUCEF_AUTH_CWEBSESSION_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#include "gucefASSETSTORE_CAssetServiceNotice.h"
#define GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICENOTICE_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

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

    /** A second service of the same store for a caller that is a machine learning consumer (the host forces the licence gate on it); null when the implementation has no callers to tell apart */
    virtual GUCEF::ASSETSTORE::TAssetStoreServicePtr MakeLearningClient( void ) { return GUCEF::ASSETSTORE::TAssetStoreServicePtr(); }
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

/**
 *  Keeps what a service raised of the sign in of its stores: the prompts, the prompts that closed, the states of the stores, the
 *  notices, and one text of everything raised (to check that no secret was in it)
 */
class CSignInEventRecorder : public GUCEF::CORE::CObservingNotifier
{
    public:

    typedef GUCEF::CORE::CTEventHandlerFunctor< CSignInEventRecorder > TEventCallback;

    GUCEF::ASSETSTORE::TAssetSignInPromptVector prompts;
    GUCEF::ASSETSTORE::TAssetSignInPromptVector closed;
    GUCEF::ASSETSTORE::TAssetStoreInfoVector states;
    GUCEF::CORE::CString dump;

    explicit CSignInEventRecorder( GUCEF::ASSETSTORE::CIAssetStoreService& service )
        : GUCEF::CORE::CObservingNotifier()
        , prompts()
        , closed()
        , states()
        , dump()
    {
        TEventCallback prompt( this, &CSignInEventRecorder::OnPrompt );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::SignInPromptEvent, prompt );
        TEventCallback done( this, &CSignInEventRecorder::OnClosed );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::SignInPromptClosedEvent, done );
        TEventCallback state( this, &CSignInEventRecorder::OnState );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::StoreStateChangedEvent, state );
        TEventCallback notice( this, &CSignInEventRecorder::OnNotice );
        SubscribeTo( &service, GUCEF::ASSETSTORE::CIAssetStoreService::NoticeEvent, notice );
    }

    virtual ~CSignInEventRecorder()
    {
        UnsubscribeAllFromObserver();
    }

    /** The states of the store in the order they were raised */
    GUCEF::CORE::CString StatesOf( const GUCEF::CORE::CString& storeId ) const
    {
        GUCEF::CORE::CString text;
        for ( size_t i=0; i<states.size(); ++i )
            if ( states[ i ].storeId == storeId )
                text += GUCEF::CORE::CString( GUCEF::ASSETSTORE::AssetStoreSignInStateToString( states[ i ].signInState ) ) + ",";
        return text;
    }

    private:

    static void Append( const GUCEF::CORE::CDataNode& node, GUCEF::CORE::CString& text )
    {
        for ( GUCEF::CORE::CDataNode::TAttributeMap::const_iterator a = node.AttributeBegin(); a != node.AttributeEnd(); ++a )
            text += (*a).first + "=" + (*a).second.AsString() + ";";
        for ( GUCEF::CORE::CDataNode::const_iterator c = node.ConstBegin(); c != node.ConstEnd(); ++c )
            Append( *(*c), text );
    }

    void Note( const GUCEF::CORE::CIDataNodeSerializable& record )
    {
        GUCEF::CORE::CDataNode node( "event", GUCEF_DATATYPE_OBJECT );
        GUCEF::CORE::CDataNodeSerializableSettings settings;
        record.Serialize( node, settings );
        Append( node, dump );
        dump += "\n";
    }

    void OnPrompt( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        const GUCEF::ASSETSTORE::CAssetSignInPrompt* prompt = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetSignInPrompt >( eventData );
        if ( GUCEF_NULL == prompt )
            return;
        prompts.push_back( *prompt );
        Note( *prompt );
    }

    void OnClosed( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        const GUCEF::ASSETSTORE::CAssetSignInPrompt* prompt = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetSignInPrompt >( eventData );
        if ( GUCEF_NULL == prompt )
            return;
        closed.push_back( *prompt );
        Note( *prompt );
    }

    void OnState( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        const GUCEF::ASSETSTORE::CAssetStoreInfo* info = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetStoreInfo >( eventData );
        if ( GUCEF_NULL == info )
            return;
        states.push_back( *info );
        Note( *info );
    }

    void OnNotice( GUCEF::CORE::CNotifier* notifier, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        const GUCEF::ASSETSTORE::CAssetServiceNotice* notice = GUCEF::CORE::CICloneable::CloneableAs< GUCEF::ASSETSTORE::CAssetServiceNotice >( eventData );
        if ( GUCEF_NULL != notice )
            Note( *notice );
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

/** The entry of a store in the list of the stores of the service */
inline bool
ServiceSuiteFindStore( GUCEF::ASSETSTORE::CIAssetStoreService& service, const GUCEF::CORE::CString& storeId, GUCEF::ASSETSTORE::CAssetStoreInfo& store )
{
    GUCEF::ASSETSTORE::CAssetStoreInfoPage page;
    if ( GUCEF::ASSETSTORE::ASSETSTORE_STATUS_OK != service.ListStores( page ) )
        return false;
    for ( size_t i=0; i<page.entries.size(); ++i )
        if ( page.entries[ i ].storeId == storeId )
        {
            store = page.entries[ i ];
            return true;
        }
    return false;
}

/** Pumps until the store has the sign in state (or the time is up); true when it has */
inline bool
ServiceSuiteWaitForStoreState( CAssetStoreServiceSuiteTarget& target, GUCEF::ASSETSTORE::CIAssetStoreService& service, const GUCEF::CORE::CString& storeId,
                               GUCEF::ASSETSTORE::TAssetStoreSignInState state, GUCEF::UInt32 timeoutInMs = 15000 )
{
    GUCEF::ASSETSTORE::CAssetStoreInfo store;
    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs; waited+=10 )
    {
        if ( ServiceSuiteFindStore( service, storeId, store ) && store.signInState == state )
            return true;
        target.Pump( 10 );
    }
    return false;
}

/** Pumps until the recorder holds the number of prompts, or of closed prompts (or the time is up); true when it does */
inline bool
ServiceSuiteWaitForPrompts( CAssetStoreServiceSuiteTarget& target, const CSignInEventRecorder& recorder, size_t prompts, size_t closed, GUCEF::UInt32 timeoutInMs = 15000 )
{
    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs; waited+=10 )
    {
        if ( recorder.prompts.size() >= prompts && recorder.closed.size() >= closed )
            return true;
        target.Pump( 10 );
    }
    return false;
}

/** The prompt of the kind among the open prompts of the service for the store; false when there is none */
inline bool
ServiceSuiteFindPrompt( GUCEF::ASSETSTORE::CIAssetStoreService& service, const GUCEF::CORE::CString& storeId, const GUCEF::CORE::CString& kind, GUCEF::ASSETSTORE::CAssetSignInPrompt& prompt )
{
    GUCEF::ASSETSTORE::CAssetSignInPromptPage page;
    if ( GUCEF::ASSETSTORE::ASSETSTORE_STATUS_OK != service.GetSignInPrompts( page ) )
        return false;
    for ( size_t i=0; i<page.entries.size(); ++i )
        if ( page.entries[ i ].storeId == storeId && page.entries[ i ].kind == kind )
        {
            prompt = page.entries[ i ];
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
    // the stores that sign in with steps of a person: one with its client credentials provisioned, one without, one that signs in on its web site
    CFakeSignInStoreProviderFactory signInFactory( "signstore", false );
    CFakeSignInStoreProviderFactory unprovisionedFactory( "signstore2", false );
    CFakeSignInStoreProviderFactory webFactory( "webstorefake", true );
    CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "signstore", &signInFactory );
    CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "signstore2", &unprovisionedFactory );
    CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "webstorefake", &webFactory );
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

    // what the stores get to sign in on the server: the settings of one store, and the keyring of the saved passwords (a code is set up and the keyring locked)
    const CString keyringCode = "KEYRING-CODE-123";
    const CString keyringDirectory = TestDirectory( "svc_" + label + "_keyring" );
    DeleteDir( keyringDirectory, true );
    CreateDirs( keyringDirectory );
    AUTH::CFileCredentialStore keyringStore( keyringDirectory );
    keyringStore.SetKeyMode( AUTH::CFileCredentialStore::KEYMODE_KEYRING_CODE );
    keyringStore.SetKeyringParameters( 64, 1, 1 );
    AUTH::CKeyringSession keyringSession( &keyringStore );
    const bool keyringReady = AUTH::CFileCredentialStore::IsKeyringAvailable() && AUTH::KEYRING_OK == keyringSession.SetUpCode( AUTH::CSecretString( keyringCode ) );
    if ( keyringReady )
        keyringSession.LockNow();
    config.signIn.keyringSession = keyringReady ? &keyringSession : GUCEF_NULL;
    config.signIn.storeSettings.AddChild( "signstore", GUCEF_DATATYPE_OBJECT )->SetAttribute( "clientIdFile", "/run/secrets/fake/clientId" );
    config.signInPromptSeconds = 4;

    TAssetStoreServicePtr servicePtr = target.MakeService( config );
    CString packAId, packBId, swordId, ingestJobId;

    GUCEF_TESTFW_TESTCASE( ( label + " service: made by the factory, opened, it describes itself; an implementation id without an implementation is refused with the reason" ).C_String() )
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
            // both implementations are in the build; an id without an implementation is refused with the reason
            CAssetStoreServiceConfig unknown;
            unknown.implementationId = "no-such-implementation";
            CString error;
            ASSERT_TRUE( CAssetStoreGlobal::Instance()->CreateService( unknown, error ).IsNULL() && !error.IsNULLOrEmpty() );
            ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetServiceFactory().IsConstructible( CAssetStoreServiceConfig::RemoteImplementationId ) );
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

    GUCEF_TESTFW_TESTCASE( ( label + " service: reads of a file by range (the end, beyond the end) and the stored thumbnails: put, found by the digest of the content and the size, read as a file of the service" ).C_String() )
        try
        {
            CAssetFileSet files;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetAssetFileSet( swordId, files ) && !files.files.empty() );
            const UInt64 size = (UInt64) files.files[ 0 ].sizeBytes;
            CAssetFileBlock whole;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReadFileBlock( files.mainVfsPath, 0, (UInt32) size, whole ) && whole.endOfFile && size == whole.data.GetDataSize() && size == whole.fileSize );
            ASSERT_TRUE( Sha256Of( whole.data ) == files.files[ 0 ].digest );
            CAssetFileBlock tail;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReadFileBlock( files.mainVfsPath, size - 3, 100, tail ) && tail.endOfFile && 3 == tail.data.GetDataSize() && size - 3 == tail.offset );
            CAssetFileBlock atEnd;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReadFileBlock( files.mainVfsPath, size, 16, atEnd ) && atEnd.endOfFile && 0 == atEnd.data.GetDataSize() );
            CAssetFileBlock none;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.ReadFileBlock( base + "/main/nothing.bin", 0, 16, none ) );

            CString::StringVector ids;
            ids.push_back( swordId );
            ids.push_back( "pack:" + packAId );
            CAssetThumbnailPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetThumbnails( ids, 64, page ) && 2 == page.entries.size() && ASSETSTORE_STATUS_NOT_FOUND == page.entries[ 0 ].status );
            CDynamicBuffer png32, png96;
            ASSERT_TRUE( MakePng( 32, 32, false, png32 ) && MakePng( 96, 96, true, png96 ) );
            CAssetThumbnail thumbnail;
            thumbnail.assetId = swordId;
            thumbnail.kind = "thumbnail";
            thumbnail.size = 32;
            thumbnail.key.size = 32;
            thumbnail.key.rendererId = "suite";
            thumbnail.key.rendererVersion = "2";
            thumbnail.encoded = png32;
            CAssetServiceResult put;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.PutThumbnail( "suite-thumb-1", thumbnail, put ) );
            thumbnail.size = 96;
            thumbnail.key.size = 96;
            thumbnail.encoded = png96;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.PutThumbnail( "suite-thumb-2", thumbnail, put ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetThumbnails( ids, 64, page ) && ASSETSTORE_STATUS_OK == page.entries[ 0 ].status && 96 == page.entries[ 0 ].key.size );
            ASSERT_TRUE( "suite" == page.entries[ 0 ].key.rendererId && "2" == page.entries[ 0 ].key.rendererVersion && files.files[ 0 ].digest == page.entries[ 0 ].key.digest );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetThumbnails( ids, 16, page ) && 32 == page.entries[ 0 ].key.size );
            CAssetFileBlock picture;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReadFileBlock( page.entries[ 0 ].vfsPath, 0, 1048576, picture ) && picture.endOfFile && Sha256Of( picture.data ) == Sha256Of( png32 ) );
            CFutureResult future = service.GetThumbnailsAsync( ids, 96 );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, future ) );
            CAssetThumbnailPage asyncPage;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( future, asyncPage ) && 96 == asyncPage.entries[ 0 ].key.size );
            CAssetThumbnail bad;
            bad.assetId = swordId;
            bad.size = 32;
            bad.key.size = 32;
            bad.encoded.CopyFrom( 40, "this is not a picture, it is a text..." );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.PutThumbnail( "suite-thumb-bad", bad, put ) && !put.message.IsNULLOrEmpty() );
            thumbnail.assetId = "no-such-asset";
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.PutThumbnail( "suite-thumb-3", thumbnail, put ) );
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
            // a store that needs no sign in is not signed in to; there are no prompts of a sign in that never began
            CAssetServiceResult signIn;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.BeginSignIn( CString::Empty, "svcstore", signIn ) && !signIn.message.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.BeginSignIn( CString::Empty, "no-such-store", signIn ) );
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

    GUCEF_TESTFW_TESTCASE( ( label + " service: operations of the converter pipeline through the hook, a pipeline job with progress; render, thumbnails and uploads say what is not available" ).C_String() )
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
            // the jobs list says what the job works on and does, when it ended, and a job that is done is not taken up again
            ASSERT_TRUE( "converter-pack" == job.scope && "convert" == job.subKind && "converter-pack" == job.packId && !job.resultText.IsNULLOrEmpty() && job.updatedAtMs >= job.createdAtMs && job.createdAtMs > 0 );
            ASSERT_FALSE( job.IsActionAvailable( ASSETJOB_ACTION_RESUME ) || job.IsActionAvailable( ASSETJOB_ACTION_RETRY ) );
            CAssetServiceResult notRestarted;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != service.ControlJob( CString::Empty, started.id, ASSETJOB_ACTION_RESUME, notRestarted ) );
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
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetThumbnails( ids, 128, thumbnails ) && 1 == thumbnails.entries.size() );
            CAssetServiceResult upload;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.BeginUpload( CString::Empty, "pack", 10, 1, upload ) && !upload.message.IsNULLOrEmpty() );
            CAssetServiceDocument resources;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResourceStatus( resources ) && resources.document.GetAttributeValueOrChildValueByName( "catalogueAvailable" ).AsBool() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: the pack index: the job index-pack and its event, groups and resources in pages, the group filter of the search, the reindex from the storage, the licence gate" ).C_String() )
        try
        {
            hook.SetIndexMode( CTestConverterHook::INDEX_BUILD );

            // before the job the packs have no index
            CAssetPackIndexInfo none;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetPackIndex( packAId, false, none ) );
            CAssetPackGroupPage noGroups;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetResourceGroups( packAId, false, noGroups ) );
            CAssetPackResourcePage noResources;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetResources( packAId, CString::Empty, CString::Empty, CString::Empty, 0, false, noResources ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetPackIndex( "no-such-pack", false, none ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.GetPackIndex( CString::Empty, false, none ) );

            // the job index-pack for one pack: the kind of the job, its event, a repeat with the command id
            CAssetPipelineRequest index;
            index.operation = CAssetPipelineOperations::IndexPack;
            index.packId = packAId;
            CAssetServiceResult started;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StartPipelineJob( "index-a", index, started ) && 0 == started.id.HasSubstr( "pipeline:" ) );
            CAssetServiceResult repeated;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StartPipelineJob( "index-a", index, repeated ) && repeated.id == started.id );
            CAssetJobEntry job;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, started.id, job ) && ASSETJOB_STATE_DONE == job.state );
            ASSERT_TRUE( "index-pack" == job.kind && packAId == job.packId && !job.resultText.IsNULLOrEmpty() );
            target.Pump( 100 );
            ASSERT_TRUE( events.HasChange( ASSETCHANGE_PACK_INDEX_UPDATED, packAId ) );

            // the index of the pack
            CAssetPackIndexInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetPackIndex( packAId, false, info ) );
            ASSERT_TRUE( packAId == info.packId && CAssetPackIndexInfo::StateLoaded == info.state && !info.stale && 3 == info.resourceCount && !info.indexKey.IsNULLOrEmpty() );
            ASSERT_TRUE( info.hasManifest && info.HasLicenceRestriction( CAssetLicence::NoMachineLearning ) && 4 == info.groups.size() );
            ASSERT_TRUE( 2 == info.levels.size() );
            CFutureResult infoFuture = service.GetPackIndexAsync( packAId, false );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, infoFuture ) );
            CAssetPackIndexInfo asyncInfo;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( infoFuture, asyncInfo ) && asyncInfo.indexKey == info.indexKey && asyncInfo.hasManifest && asyncInfo.groups.size() == info.groups.size() );

            // the groups: the fixed order, the function group before its sub-groups
            CAssetPackGroupPage groups;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResourceGroups( packAId, false, groups ) && 4 == groups.entries.size() && groups.revision > 0 );
            ASSERT_TRUE( "static-models" == groups.entries[ 0 ].groupId && groups.entries[ 0 ].subGroup.IsNULLOrEmpty() && 2 == groups.entries[ 0 ].resourceCount );
            ASSERT_TRUE( "meshes" == groups.entries[ 1 ].subGroup && "textures" == groups.entries[ 2 ].groupId && 1 == groups.entries[ 2 ].resourceCount );
            CFutureResult groupFuture = service.GetResourceGroupsAsync( packAId, false );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, groupFuture ) );
            CAssetPackGroupPage asyncGroups;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( groupFuture, asyncGroups ) && 4 == asyncGroups.entries.size() );

            // the resources in pages, of a group and of a sub-group
            CAssetPackResourcePage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResources( packAId, CString::Empty, CString::Empty, CString::Empty, 2, false, page ) );
            ASSERT_TRUE( 2 == page.entries.size() && 3 == page.total && !page.nextPageToken.IsNULLOrEmpty() && page.revision > 0 );
            ASSERT_TRUE( "Meshes/SM_Wep_Axe_01.fbx" == page.entries[ 0 ].path && "static-models" == page.entries[ 0 ].groupId && "meshes" == page.entries[ 0 ].subGroup && "model" == page.entries[ 0 ].kind );
            CAssetPackResourcePage second;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResources( packAId, CString::Empty, CString::Empty, page.nextPageToken, 2, false, second ) );
            ASSERT_TRUE( 1 == second.entries.size() && second.nextPageToken.IsNULLOrEmpty() && "textures" == second.entries[ 0 ].groupId );
            CAssetPackResourcePage textures;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResources( packAId, "textures", CString::Empty, CString::Empty, 0, false, textures ) && 1 == textures.entries.size() );
            CAssetPackResourcePage meshes;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResources( packAId, "static-models", "meshes", CString::Empty, 0, false, meshes ) && 2 == meshes.entries.size() );
            bool swordIsResource = false;
            for ( size_t e=0; e<meshes.entries.size(); ++e )
                swordIsResource = swordIsResource || ( swordId == meshes.entries[ e ].assetId && swordId == meshes.entries[ e ].resourceId );
            ASSERT_TRUE( swordIsResource );
            CFutureResult pageFuture = service.GetResourcesAsync( packAId, "static-models", CString::Empty, CString::Empty, 0, false );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, pageFuture ) );
            CAssetPackResourcePage asyncPage;
            ASSERT_TRUE( CIAssetStoreService::TryGetFutureResult( pageFuture, asyncPage ) && 2 == asyncPage.entries.size() );

            // the group filter of the search
            CAssetSearchQuery query;
            query.pageSize = 100;
            query.groups.insert( "static-models" );
            query.packIds.insert( packAId );
            CAssetSearchResultPage hits;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( query, hits ) && 2 == hits.hits.size() );
            query.groups.clear();
            query.groups.insert( "textures" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( query, hits ) && 1 == hits.hits.size() );
            query.groups.clear();
            query.subGroups.insert( "meshes" );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( query, hits ) && 2 == hits.hits.size() );

            // the licence gate: a pack with noMachineLearning is not there for a machine learning consumer, one without is, and what it gets has no thumbnails
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetPackIndex( packAId, true, none ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetResourceGroups( packAId, true, noGroups ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.GetResources( packAId, CString::Empty, CString::Empty, CString::Empty, 0, true, noResources ) );
            CFutureResult hidden = service.GetPackIndexAsync( packAId, true );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, hidden ) && ASSETSTORE_STATUS_NOT_FOUND == CIAssetStoreService::GetStatusOfFuture( hidden ) );
            CAssetPipelineRequest indexB;
            indexB.operation = CAssetPipelineOperations::IndexPack;
            indexB.packId = packBId;
            CAssetServiceResult startedB;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StartPipelineJob( "index-b", indexB, startedB ) );
            CAssetJobEntry jobB;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, startedB.id, jobB ) && ASSETJOB_STATE_DONE == jobB.state );
            CAssetPackIndexInfo infoB;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetPackIndex( packBId, true, infoB ) && 2 == infoB.resourceCount && 1 == infoB.groups.size() && !infoB.HasLicenceRestriction( CAssetLicence::NoMachineLearning ) );
            // the host decides for a caller of a learning role, whatever the client says
            GUCEF::ASSETSTORE::TAssetStoreServicePtr learnerPtr = target.MakeLearningClient();
            ASSERT_TRUE( service.IsInProcess() || !learnerPtr.IsNULL() );
            if ( !learnerPtr.IsNULL() )
            {
                CIAssetStoreService& learner = *learnerPtr;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == learner.Open() );
                CAssetPackIndexInfo seen;
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == learner.GetPackIndex( packAId, false, seen ) );
                CAssetPackGroupPage seenGroups;
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == learner.GetResourceGroups( packAId, false, seenGroups ) );
                CAssetPackResourcePage seenResources;
                ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == learner.GetResources( packAId, CString::Empty, CString::Empty, CString::Empty, 0, false, seenResources ) );
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == learner.GetPackIndex( packBId, false, seen ) && 2 == seen.resourceCount );
                CAssetSearchQuery everything;
                everything.pageSize = 100;
                CAssetSearchResultPage ownerHits;
                CAssetSearchResultPage learnerHits;
                ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.Search( everything, ownerHits ) && ASSETSTORE_STATUS_OK == learner.Search( everything, learnerHits ) );
                bool ownerSeesA = false;
                bool learnerSeesA = false;
                for ( size_t h=0; h<ownerHits.hits.size(); ++h )
                    ownerSeesA = ownerSeesA || packAId == ownerHits.hits[ h ].packId;
                for ( size_t h=0; h<learnerHits.hits.size(); ++h )
                    learnerSeesA = learnerSeesA || packAId == learnerHits.hits[ h ].packId;
                ASSERT_TRUE( ownerSeesA && !learnerSeesA && !learnerHits.hits.empty() );
                learner.Close();
            }
            CAssetPackResourcePage learning;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetResources( packBId, CString::Empty, CString::Empty, CString::Empty, 0, true, learning ) && 2 == learning.entries.size() );
            for ( size_t e=0; e<learning.entries.size(); ++e )
                ASSERT_FALSE( learning.entries[ e ].hasThumbnail );

            // the job over every pack that has no index, and the reindex from the storage (the rows are written again from the documents)
            CAssetPipelineRequest missing;
            missing.operation = CAssetPipelineOperations::IndexPack;
            missing.arguments.SetName( "arguments" );
            missing.arguments.SetAttribute( "missing", true );
            CAssetServiceResult startedMissing;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.StartPipelineJob( CString::Empty, missing, startedMissing ) );
            CAssetJobEntry jobMissing;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, startedMissing.id, jobMissing ) && ASSETJOB_STATE_DONE == jobMissing.state && !jobMissing.resultText.IsNULLOrEmpty() );
            CAssetServiceResult reindex;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReindexFromStorage( "reindex-1", reindex ) && !reindex.id.IsNULLOrEmpty() );
            CAssetServiceResult reindexAgain;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ReindexFromStorage( "reindex-1", reindexAgain ) && reindexAgain.id == reindex.id );
            CAssetJobEntry jobReindex;
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, service, reindex.id, jobReindex ) && ASSETJOB_STATE_DONE == jobReindex.state && "index-pack" == jobReindex.kind );
            CAssetPackIndexInfo infoAfter;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetPackIndex( packAId, false, infoAfter ) && infoAfter.indexKey == info.indexKey && 3 == infoAfter.resourceCount );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: the sign in of an online store through prompts: an address and a code, a text that is no code, a code the store refuses, the right code, the end of the session, a new sign in, cancel and sign out" ).C_String() )
        try
        {
            TESTFW::CLogTextCapture* capture = GUCEF_NEW TESTFW::CLogTextCapture();
            CSignInEventRecorder recorder( service );

            // the store as the service lists it: how it signs in, signed out, and what the settings gave it
            CAssetStoreInfo store;
            ASSERT_TRUE( ServiceSuiteFindStore( service, "signstore", store ) );
            ASSERT_TRUE( ASSETSTORE_SIGNIN_AUTH_PROVIDER == store.signInKind && ASSETSTORE_SIGNINSTATE_SIGNED_OUT == store.signInState && !store.signedIn );
            CAssetStoreLibraryPage library;
            CAssetStoreQuery query;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == service.QueryLibrary( "signstore", query, library ) );
            {
                TAssetStoreProviderPtr provider;
                ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( "signstore", provider ) );
                CFakeSignInStoreProvider* fake = static_cast< CFakeSignInStoreProvider* >( provider.GetPointerAlways() );
                ASSERT_TRUE( fake->WasConfigured() && fake->IsProvisioned() && "/run/secrets/fake/clientId" == fake->GetClientIdFile() );
            }

            // the sign in begins: the person gets the address to open and the field for the code, as events and as the list of the open prompts
            CAssetServiceResult begun;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( "signin-1", "signstore", begun ) && "signstore" == begun.id );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 2, 0 ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_IN_PROGRESS ) );
            CAssetSignInPrompt urlPrompt, codePrompt;
            ASSERT_TRUE( ServiceSuiteFindPrompt( service, "signstore", "openUrl", urlPrompt ) && ServiceSuiteFindPrompt( service, "signstore", "enterCode", codePrompt ) );
            ASSERT_TRUE( urlPrompt.url == CFakeInteractiveSignIn::LoginUrl() && !urlPrompt.sensitiveAnswer && !urlPrompt.promptId.IsNULLOrEmpty() );
            ASSERT_TRUE( codePrompt.sensitiveAnswer && codePrompt.url.IsNULLOrEmpty() && codePrompt.promptId != urlPrompt.promptId );
            ASSERT_TRUE( urlPrompt.expiresUtcMs > 0 && codePrompt.expiresUtcMs > 0 );
            // the same command id answers the first outcome and does not start another sign in
            CAssetServiceResult repeated;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( "signin-1", "signstore", repeated ) && repeated.replayed );
            // another begin while one runs keeps the one that runs
            CAssetServiceResult again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( CString::Empty, "signstore", again ) );
            CAssetSignInPromptPage open;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSignInPrompts( open ) && 2 == open.entries.size() );

            // a step that is not there, a step that takes no answer, a text that is no code: refused, and the step stays open for the next try
            CAssetServiceResult answered;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.AnswerPrompt( CString::Empty, "no-such-prompt", AUTH::CSecretString( CString( "RIGHTCODE1234" ) ), answered ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.AnswerPrompt( CString::Empty, urlPrompt.promptId, AUTH::CSecretString( CString( "RIGHTCODE1234" ) ), answered ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.AnswerPrompt( CString::Empty, codePrompt.promptId, AUTH::CSecretString( CString( "!! not a code" ) ), answered ) && !answered.message.IsNULLOrEmpty() );
            ASSERT_TRUE( ServiceSuiteFindPrompt( service, "signstore", "enterCode", codePrompt ) );

            // a code that has the shape of one and that the store refuses: the sign in ends as failed with the reason, its prompts close
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.AnswerPrompt( "answer-1", codePrompt.promptId, AUTH::CSecretString( CString( CFakeInteractiveSignIn::WrongCode() ) ), answered ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_FAILED ) );
            ASSERT_TRUE( ServiceSuiteFindStore( service, "signstore", store ) && !store.signedIn && !store.message.IsNULLOrEmpty() );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 2, 2 ) );
            CAssetSignInPromptPage none;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSignInPrompts( none ) && none.entries.empty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.AnswerPrompt( CString::Empty, codePrompt.promptId, AUTH::CSecretString( CString( CFakeInteractiveSignIn::RightCode() ) ), answered ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == service.QueryLibrary( "signstore", query, library ) );

            // a new sign in has new steps; with the right code the store has a session and the library opens
            CAssetServiceResult second;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( "signin-2", "signstore", second ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 4, 2 ) );
            ASSERT_TRUE( ServiceSuiteFindPrompt( service, "signstore", "enterCode", codePrompt ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.AnswerPrompt( "answer-2", codePrompt.promptId, AUTH::CSecretString( CString( CFakeInteractiveSignIn::RightCode() ) ), answered ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_SIGNED_IN ) );
            ASSERT_TRUE( ServiceSuiteFindStore( service, "signstore", store ) && store.signedIn );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 4, 4 ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.QueryLibrary( "signstore", query, library ) );
            CFutureResult asyncAnswer = service.AnswerPromptAsync( CString::Empty, "no-such-prompt", AUTH::CSecretString( CString( "RIGHTCODE1234" ) ) );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, asyncAnswer ) && ASSETSTORE_STATUS_NOT_FOUND == CIAssetStoreService::GetStatusOfFuture( asyncAnswer ) );

            // the store refuses the session (HTTP 401 and no renewal): the state is expired, whatever the provider holds, until a sign in
            {
                TAssetStoreProviderPtr provider;
                ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( "signstore", provider ) );
                static_cast< CFakeSignInStoreProvider* >( provider.GetPointerAlways() )->RefuseSession();
            }
            ASSERT_TRUE( ASSETSTORE_STATUS_SESSION_EXPIRED == service.QueryLibrary( "signstore", query, library ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_EXPIRED ) );
            ASSERT_TRUE( ServiceSuiteFindStore( service, "signstore", store ) && !store.signedIn && store.sessionEnded );

            // the renewal is a new sign in; the session ends once more at the store itself and is renewed the same way
            CAssetServiceResult third;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( CString::Empty, "signstore", third ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 6, 4 ) );
            ASSERT_TRUE( ServiceSuiteFindPrompt( service, "signstore", "enterCode", codePrompt ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.AnswerPrompt( CString::Empty, codePrompt.promptId, AUTH::CSecretString( CString( CFakeInteractiveSignIn::RightCode() ) ), answered ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_SIGNED_IN ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.QueryLibrary( "signstore", query, library ) );
            {
                TAssetStoreProviderPtr provider;
                ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( "signstore", provider ) );
                static_cast< CFakeSignInStoreProvider* >( provider.GetPointerAlways() )->ExpireSession();
            }
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_EXPIRED ) );

            // cancel: the steps close and the store is signed out; with no sign in running it answers that none runs
            CAssetServiceResult fourth;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( CString::Empty, "signstore", fourth ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 8, 6 ) );
            ASSERT_TRUE( ServiceSuiteFindPrompt( service, "signstore", "enterCode", codePrompt ) );
            CAssetServiceResult cancelled;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.CancelSignIn( "cancel-1", "signstore", cancelled ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 8, 8 ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_SIGNED_OUT ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.AnswerPrompt( CString::Empty, codePrompt.promptId, AUTH::CSecretString( CString( CFakeInteractiveSignIn::RightCode() ) ), answered ) );
            CAssetServiceResult nothing;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.CancelSignIn( CString::Empty, "signstore", nothing ) && !nothing.message.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.CancelSignIn( CString::Empty, "no-such-store", nothing ) );

            // a step that nobody answers expires: the sign in ends as failed and the steps close; a new sign in works again
            CAssetServiceResult fifth;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( CString::Empty, "signstore", fifth ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 10, 8 ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_FAILED, 15000 ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 10, 10 ) );
            ASSERT_TRUE( ServiceSuiteFindStore( service, "signstore", store ) && !store.message.IsNULLOrEmpty() );
            CAssetServiceResult sixth;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.BeginSignIn( CString::Empty, "signstore", sixth ) );
            ASSERT_TRUE( ServiceSuiteWaitForPrompts( target, recorder, 12, 10 ) );
            ASSERT_TRUE( ServiceSuiteFindPrompt( service, "signstore", "enterCode", codePrompt ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.AnswerPrompt( CString::Empty, codePrompt.promptId, AUTH::CSecretString( CString( CFakeInteractiveSignIn::RightCode() ) ), answered ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_SIGNED_IN ) );

            // sign out forgets the session
            CAssetServiceResult signedOut;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SignOut( CString::Empty, "signstore", signedOut ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore", ASSETSTORE_SIGNINSTATE_SIGNED_OUT ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_SIGNED_IN == service.QueryLibrary( "signstore", query, library ) );
            ASSERT_TRUE( recorder.StatesOf( "signstore" ).HasSubstr( "inProgress,failed" ) >= 0 && recorder.StatesOf( "signstore" ).HasSubstr( "signedIn" ) >= 0 );

            // no secret was in anything that was raised or logged: not a code that was answered, not a prompt that closed
            target.Pump( 100 );
            const CString shown = recorder.dump;
            ASSERT_TRUE( !shown.IsNULLOrEmpty() && shown.HasSubstr( CFakeInteractiveSignIn::RightCode() ) < 0 && shown.HasSubstr( CFakeInteractiveSignIn::WrongCode() ) < 0 );
            ASSERT_TRUE( capture->GetText().HasSubstr( CFakeInteractiveSignIn::RightCode() ) < 0 && capture->GetText().HasSubstr( CFakeInteractiveSignIn::WrongCode() ) < 0 );
            ASSERT_TRUE( !capture->GetText().IsNULLOrEmpty() );
            GUCEF_DELETE capture;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: a store that cannot sign in here says why: client credentials that are not provisioned, a web site sign in without a browser" ).C_String() )
        try
        {
            CAssetStoreInfo store;
            ASSERT_TRUE( ServiceSuiteFindStore( service, "signstore2", store ) );
            ASSERT_TRUE( ASSETSTORE_SIGNINSTATE_SIGNED_OUT == store.signInState && store.message.HasSubstr( "not provisioned" ) >= 0 );
            CAssetServiceResult begun;
            ASSERT_TRUE( ASSETSTORE_STATUS_UNAVAILABLE == service.BeginSignIn( CString::Empty, "signstore2", begun ) && begun.message.HasSubstr( "not provisioned" ) >= 0 );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "signstore2", ASSETSTORE_SIGNINSTATE_FAILED ) );
            CAssetSignInPromptPage none;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSignInPrompts( none ) && none.entries.empty() );

            // the sign in on a web site needs the browser, which a service of this kind does not have: signed out, with the reason, and no prompt
            ASSERT_TRUE( ServiceSuiteFindStore( service, "webstorefake", store ) );
            ASSERT_TRUE( ASSETSTORE_SIGNIN_WEB_SESSION == store.signInKind && ASSETSTORE_SIGNINSTATE_SIGNED_OUT == store.signInState && store.message.HasSubstr( "browser sidecar" ) >= 0 );
            CAssetServiceResult web;
            ASSERT_TRUE( ASSETSTORE_STATUS_BROWSER_UNAVAILABLE == service.BeginSignIn( CString::Empty, "webstorefake", web ) && web.message.HasSubstr( "browser sidecar" ) >= 0 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetSignInPrompts( none ) && none.entries.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: the account holder hands over the session of their own browser (ImportWebSession): only the store's cookies count, the store is signed in; refusals say why; no cookie value in an event, an answer or the log" ).C_String() )
        try
        {
            TESTFW::CLogTextCapture* capture = GUCEF_NEW TESTFW::CLogTextCapture();
            CSignInEventRecorder recorder( service );
            AUTH::CWebSession handed;
            handed.storeId = "webstorefake";
            handed.capturedAtUtcInMs = AUTH::CWebSession::NowUtcInMs();
            handed.userAgent = "Mozilla/5.0 (X11; Linux x86_64) SuiteChrome/140.0";
            AUTH::CWebSessionCookie cookie;
            cookie.name = "sid";
            cookie.value.Set( "suiteWebSessionValue31" );
            cookie.domain = ".webstore.test";
            cookie.path = "/";
            cookie.secure = true;
            cookie.httpOnly = true;
            handed.cookies.push_back( cookie );
            cookie.name = "elsewhere";
            cookie.value.Set( "suiteForeignValue32" );
            cookie.domain = "elsewhere.test";
            handed.cookies.push_back( cookie );
            AUTH::CSecretString secret;
            ASSERT_TRUE( handed.ToSecret( secret ) );

            CAssetServiceResult imported;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ImportWebSession( "import-web-1", "webstorefake", secret, imported ) );
            ASSERT_TRUE( "webstorefake" == imported.id && imported.message.HasSubstr( "1 cookies" ) >= 0 );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "webstorefake", ASSETSTORE_SIGNINSTATE_SIGNED_IN ) );
            TAssetStoreProviderPtr providerPtr;
            ASSERT_TRUE( CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( "webstorefake", providerPtr ) && !providerPtr.IsNULL() );
            const CFakeSignInStoreProvider* provider = static_cast< const CFakeSignInStoreProvider* >( providerPtr.GetPointerAlways() );
            ASSERT_TRUE( 1 == provider->GetImportedCookieCount() && handed.userAgent == provider->GetImportedUserAgent() );

            // the same command id again: the first outcome, not a second import
            CAssetServiceResult again;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.ImportWebSession( "import-web-1", "webstorefake", secret, again ) && again.replayed );

            // refusals with the reason: an unknown store, a store that does not sign in on a web site, what is not a session, a session without a cookie of the store
            CAssetServiceResult refused;
            ASSERT_TRUE( ASSETSTORE_STATUS_NOT_FOUND == service.ImportWebSession( CString::Empty, "nosuchstore", secret, refused ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_UNSUPPORTED == service.ImportWebSession( CString::Empty, "signstore", secret, refused ) && !refused.message.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.ImportWebSession( CString::Empty, "webstorefake", AUTH::CSecretString( CString( "{\"format\":\"nonsense\"}" ) ), refused ) );
            AUTH::CWebSession foreign;
            cookie.name = "elsewhere";
            cookie.value.Set( "suiteForeignValue32" );
            cookie.domain = "elsewhere.test";
            foreign.cookies.push_back( cookie );
            AUTH::CSecretString foreignSecret;
            ASSERT_TRUE( foreign.ToSecret( foreignSecret ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.ImportWebSession( CString::Empty, "webstorefake", foreignSecret, refused ) && refused.message.HasSubstr( "no cookie" ) >= 0 );

            // the asynchronous form
            CFutureResult importing = service.ImportWebSessionAsync( CString::Empty, "webstorefake", secret );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, importing ) && ASSETSTORE_STATUS_OK == CIAssetStoreService::GetStatusOfFuture( importing ) );

            // signed out again for the cases after this one; no value anywhere
            CAssetServiceResult signedOut;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.SignOut( CString::Empty, "webstorefake", signedOut ) );
            ASSERT_TRUE( ServiceSuiteWaitForStoreState( target, service, "webstorefake", ASSETSTORE_SIGNINSTATE_SIGNED_OUT ) );
            target.Pump( 100 );
            ASSERT_TRUE( recorder.StatesOf( "webstorefake" ).HasSubstr( "signedIn" ) >= 0 );
            ASSERT_TRUE( recorder.dump.HasSubstr( "suiteWebSessionValue31" ) < 0 && recorder.dump.HasSubstr( "suiteForeignValue32" ) < 0 );
            ASSERT_TRUE( capture->GetText().HasSubstr( "suiteWebSessionValue31" ) < 0 && capture->GetText().HasSubstr( "suiteForeignValue32" ) < 0 );
            ASSERT_TRUE( imported.message.HasSubstr( "suiteWebSessionValue31" ) < 0 && refused.message.HasSubstr( "suiteForeignValue32" ) < 0 );
            secret.Clear();
            foreignSecret.Clear();
            handed.Clear();
            foreign.Clear();
            GUCEF_DELETE capture;
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " service: the keyring of the saved store passwords: locked, a wrong code, the right code, locked again; the code is in no log and no event" ).C_String() )
        try
        {
            ASSERT_TRUE( keyringReady );
            TESTFW::CLogTextCapture* capture = GUCEF_NEW TESTFW::CLogTextCapture();
            CSignInEventRecorder recorder( service );
            CAssetServiceInfo info;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetServiceInfo( info ) && "locked" == info.keyringState );
            CAssetServiceResult keyring;
            ASSERT_TRUE( ASSETSTORE_STATUS_INVALID_ARGUMENT == service.UnlockKeyring( CString::Empty, AUTH::CSecretString( CString( "WRONG-KEYRING-CODE" ) ), keyring ) && !keyring.message.IsNULLOrEmpty() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetServiceInfo( info ) && "locked" == info.keyringState );
            // the wrong code counted: the next try is not made at once, so the right one is given after the wait of the session or a lock and unlock of the test
            for ( UInt32 waited=0; waited<10000 && 0 != keyringSession.GetRetryWaitInMs(); waited+=50 )
                target.Pump( 50 );
            CFutureResult unlocking = service.UnlockKeyringAsync( CString::Empty, AUTH::CSecretString( keyringCode ) );
            ASSERT_TRUE( ServiceSuiteWaitForFuture( target, unlocking ) && ASSETSTORE_STATUS_OK == CIAssetStoreService::GetStatusOfFuture( unlocking ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetServiceInfo( info ) && "unlocked" == info.keyringState && info.HasCapability( "keyring" ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.LockKeyring( CString::Empty, keyring ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == service.GetServiceInfo( info ) && "locked" == info.keyringState );
            target.Pump( 100 );
            ASSERT_TRUE( capture->GetText().HasSubstr( keyringCode ) < 0 && capture->GetText().HasSubstr( "WRONG-KEYRING-CODE" ) < 0 && !capture->GetText().IsNULLOrEmpty() );
            ASSERT_TRUE( recorder.dump.HasSubstr( keyringCode ) < 0 && recorder.dump.HasSubstr( "WRONG-KEYRING-CODE" ) < 0 );
            GUCEF_DELETE capture;
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
    CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "signstore" );
    CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "signstore" );
    CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "signstore2" );
    CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "signstore2" );
    CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "webstorefake" );
    CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "webstorefake" );
    CFakeStoreProvider::SetServer( GUCEF_NULL );
    server.Stop();
    if ( ownAnalyzer )
        analyzers.Unregister( analyzer->GetName() );
}

/** The configuration of the restart tests: the library below the base, the state folder, the converter hook; the downloads start or not, the staged ingests resume or not */
inline GUCEF::ASSETSTORE::CAssetStoreServiceConfig
MakeRestartServiceConfig( CAssetStoreServiceSuiteTarget& target, const GUCEF::CORE::CString& base, const GUCEF::CORE::CString& stateDirectory,
                          GUCEF::ASSETSTORE::CIAssetConverterHook* hook, bool startDownloads, bool resumeIngests, GUCEF::UInt32 resumeAttempts )
{
    GUCEF::ASSETSTORE::CAssetStoreServiceConfig config;
    config.primaryStore.locations.clear();
    GUCEF::ASSETSTORE::CAssetStorageLocation main;
    main.locationId = "main";
    main.name = "Main";
    main.vfsRoot = base + "/main";
    config.primaryStore.locations.push_back( main );
    config.primaryStore.defaultLocationId = "main";
    config.primaryStore.analysisConcurrency = 2;
    GUCEF::ASSETSTORE::CAssetSourcePolicy policy;
    policy.sourceId = "restartstore";
    policy.mode = GUCEF::ASSETSTORE::ASSETINGEST_MODE_MANUAL;
    config.primaryStore.SetPolicy( policy );
    config.converterHook = hook;
    config.stateDirectory = stateDirectory;
    config.loadDownloadQueue = true;
    config.startDownloads = startDownloads;
    config.resumeIngestsOnStart = resumeIngests;
    config.ingestResumeAttempts = resumeAttempts;
    target.ConfigureDatabase( config );
    return config;
}

/** The text of a file of the state folder, through the VFS; empty when it is not there */
inline GUCEF::CORE::CString
ReadStateFile( const GUCEF::CORE::CString& name )
{
    GUCEF::CORE::CDynamicBuffer content;
    GUCEF::CORE::CString text;
    if ( ReadVfsFile( GUCEF::ASSETSTORE::CAssetStoreServiceConfig::StateVfsRoot + "/" + name, content ) )
        text.Set( (const char*) content.GetConstBufferPtr(), content.GetDataSize() );
    return text;
}

/**
 *  What a restart of the service must not lose, for a service on the database of the target: the queue of the downloads, the
 *  downloads that are to be ingested and the source policies set through the service (the state folder), the ingests that stayed
 *  staged. Each service of the test is the one process of a run; the next one is its restart.
 */
inline void
RunAssetStoreServiceRestartSuite( CAssetStoreServiceSuiteTarget& target )
{
    using namespace GUCEF;
    using namespace GUCEF::CORE;
    using namespace GUCEF::ASSETSTORE;

    const CString label = target.GetLabel();
    const CString base = "astest/svcrs_" + label;
    MountFreshDirectory( "svcrs_" + label + "_main", base + "/main" );
    MountFreshDirectory( "svcrs_" + label + "_source", base + "/source" );
    const CString stateDirectory = TestDirectory( "svcrs_" + label + "_state" );
    DeleteDir( stateDirectory, true );

    // the analyzer of this suite is the registered one (FailNext() is on it)
    CTestModelAnalyzer* analyzer = GUCEF_NEW CTestModelAnalyzer();
    CAssetContentAnalyzerRegistry& analyzers = CAssetStoreGlobal::Instance()->GetContentAnalyzerRegistry();
    analyzers.Unregister( analyzer->GetName() );
    TAssetContentAnalyzerPtr analyzerPtr( analyzer );
    analyzers.Register( analyzerPtr );

    const CString packStaged = base + "/source/Resume_Pack";
    bool written = WriteVfsFile( packStaged + "/Meshes/SM_Resume_01.fbx", CString( "triangles=301;vertices=201" ) );
    written = WriteVfsFile( packStaged + "/Meshes/SM_Resume_02.fbx", CString( "triangles=311;vertices=211" ) ) && written;

    CFakeStoreServer server;
    CFakeStoreProviderFactory storeFactory( "restartstore" );
    {
        CAssetStoreItem item;
        item.itemId = "rs-item-1";
        item.title = "Restart Crate";
        item.seller = "Test Seller";
        item.listingType = "3d-model";
        item.licenceName = "Standard License";
        CAssetStoreFormat format;
        format.formatId = "fbx";
        format.name = "FBX";
        CDynamicBuffer content;
        content.Append( "triangles=97;vertices=65", 24 );
        server.SetFileContent( "rsf1", content );
        CAssetStoreFile file;
        file.fileId = "rsf1";
        file.name = "SM_Restart_01.fbx";
        file.size = content.GetDataSize();
        file.digestAlgorithm = "sha256";
        file.digest = Sha256Of( content );
        format.files.push_back( file );
        item.formats.push_back( format );
        server.AddItem( item );
    }
    const bool serverStarted = server.Start();
    CFakeStoreProvider::SetServer( &server );
    CAssetStoreGlobal::Instance()->GetProviderFactory().RegisterConcreteFactory( "restartstore", &storeFactory );
    CAssetStoreGlobal::Instance()->GetRegistry().RefreshProviders();

    CTestConverterHook hook;
    CString downloadJobId;

    GUCEF_TESTFW_TESTCASE( ( label + " restart: the queue of the downloads, the download that is to be ingested and a source policy survive the end of the service; the download resumes and its pack is ingested" ).C_String() )
        try
        {
            ASSERT_TRUE( written && serverStarted );
            CAssetStoreServiceConfig first = MakeRestartServiceConfig( target, base, stateDirectory, &hook, false, true, 3 );
            TAssetStoreServicePtr before = target.MakeService( first );
            ASSERT_FALSE( before.IsNULL() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == before->Open() && before->IsCatalogueAvailable() );

            // a policy set while it ran, and a download that does not start (the downloads of this service are held)
            CAssetSourcePolicy policy;
            policy.sourceId = "othersource";
            policy.mode = ASSETINGEST_MODE_AUTOMATIC;
            policy.keep = ASSETKEEP_ORIGINAL;
            CAssetServiceResult policyResult;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == before->SetSourcePolicy( "restart-policy", policy, policyResult ) );
            CAssetDownloadRequest download;
            download.providerId = "restartstore";
            download.itemId = "rs-item-1";
            download.itemTitle = "Restart Crate";
            download.formatId = "fbx";
            download.forIngest = true;
            download.keep = "original";
            download.locationId = "main";
            CAssetServiceResult queued;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == before->EnqueueDownload( "restart-download", download, queued ) && !queued.id.IsNULLOrEmpty() );
            downloadJobId = queued.id;
            CAssetJobEntry job;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == before->GetJob( downloadJobId, job ) && ASSETJOB_STATE_QUEUED == job.state && !job.ingestRequested );

            // the files are in the state folder, not in the library
            ASSERT_TRUE( FileExists( CombinePath( stateDirectory, CAssetStoreServiceConfig::QueueFileName ) ) );
            const CString stateText = ReadStateFile( CAssetStoreServiceConfig::ServiceStateFileName );
            ASSERT_TRUE( stateText.HasSubstr( downloadJobId ) >= 0 && stateText.HasSubstr( "othersource" ) >= 0 && stateText.HasSubstr( "automatic" ) >= 0 );
            before->Close();
            before.Unlink();

            // the restart: the same state folder, the downloads run now
            CAssetStoreServiceConfig second = MakeRestartServiceConfig( target, base, stateDirectory, &hook, true, true, 3 );
            TAssetStoreServicePtr after = target.MakeService( second );
            ASSERT_FALSE( after.IsNULL() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == after->Open() && after->IsCatalogueAvailable() );
            ASSERT_TRUE( ServiceSuiteWaitForJob( target, *after, downloadJobId, job, 60000 ) && ASSETJOB_STATE_DONE == job.state && job.forIngest );
            bool ingested = false;
            for ( UInt32 waited=0; waited<60000 && !ingested; waited+=20 )
            {
                CAssetSearchQuery crates;
                crates.text = "restart crate";
                CAssetSearchResultPage page;
                ingested = ASSETSTORE_STATUS_OK == after->Search( crates, page ) && !page.hits.empty();
                if ( !ingested )
                    target.Pump( 20 );
            }
            ASSERT_TRUE( ingested );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == after->GetJob( downloadJobId, job ) && job.ingestRequested );
            // the policy of the first run is still there
            ASSERT_TRUE( ReadStateFile( CAssetStoreServiceConfig::ServiceStateFileName ).HasSubstr( "othersource" ) >= 0 );
            after->Close();
            after.Unlink();

            // a third start does not ingest the finished download again: it is accounted for in the state file
            CAssetStoreServiceConfig third = MakeRestartServiceConfig( target, base, stateDirectory, &hook, true, true, 3 );
            TAssetStoreServicePtr again = target.MakeService( third );
            ASSERT_FALSE( again.IsNULL() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == again->Open() );
            target.Pump( 300 );
            CAssetJobEntry ingestJob;
            ASSERT_FALSE( ServiceSuiteFindJob( *again, "ingest", ingestJob ) );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == again->GetJob( downloadJobId, job ) && job.ingestRequested );
            again->Close();
            again.Unlink();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( ( label + " restart: an ingest that stayed staged resumes when the service opens, a limited number of starts; with the resume off it is left" ).C_String() )
        try
        {
            // an ingest that fails in the analysis stays staged
            analyzer->FailNext( 1 );
            CAssetStoreServiceConfig held = MakeRestartServiceConfig( target, base, stateDirectory, &hook, false, false, 3 );
            TAssetStoreServicePtr first = target.MakeService( held );
            ASSERT_FALSE( first.IsNULL() );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == first->Open() );
            CAssetIngestRequest request;
            request.sourceId = CPrimaryAssetStoreSettings::DiskSourceId;
            request.contentPath = packStaged;
            request.keep = "original";
            CAssetIngestResult result;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK != first->Ingest( "staged-ingest", request, result ) );
            CAssetServiceStringList pending;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == first->GetPendingIngests( pending ) && 1 == pending.values.size() );
            first->Close();
            first.Unlink();

            // the resume is off: the start leaves it staged
            CAssetStoreServiceConfig off = MakeRestartServiceConfig( target, base, stateDirectory, &hook, false, false, 3 );
            TAssetStoreServicePtr left = target.MakeService( off );
            ASSERT_TRUE( !left.IsNULL() && ASSETSTORE_STATUS_OK == left->Open() );
            target.Pump( 300 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == left->GetPendingIngests( pending ) && 1 == pending.values.size() );
            left->Close();
            left.Unlink();

            // no attempts left: the start leaves it staged as well
            CAssetStoreServiceConfig none = MakeRestartServiceConfig( target, base, stateDirectory, &hook, false, true, 0 );
            TAssetStoreServicePtr noAttempts = target.MakeService( none );
            ASSERT_TRUE( !noAttempts.IsNULL() && ASSETSTORE_STATUS_OK == noAttempts->Open() );
            target.Pump( 300 );
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == noAttempts->GetPendingIngests( pending ) && 1 == pending.values.size() );
            noAttempts->Close();
            noAttempts.Unlink();

            // the resume is on: the ingest runs from the step that failed, and the pack is in the catalogue
            CAssetStoreServiceConfig on = MakeRestartServiceConfig( target, base, stateDirectory, &hook, false, true, 3 );
            TAssetStoreServicePtr resumed = target.MakeService( on );
            ASSERT_TRUE( !resumed.IsNULL() && ASSETSTORE_STATUS_OK == resumed->Open() );
            bool done = false;
            for ( UInt32 waited=0; waited<60000 && !done; waited+=20 )
            {
                done = ASSETSTORE_STATUS_OK == resumed->GetPendingIngests( pending ) && pending.values.empty();
                if ( !done )
                    target.Pump( 20 );
            }
            ASSERT_TRUE( done );
            CAssetSearchQuery query;
            query.text = "resume";
            CAssetSearchResultPage page;
            ASSERT_TRUE( ASSETSTORE_STATUS_OK == resumed->Search( query, page ) && !page.hits.empty() );
            resumed->Close();
            resumed.Unlink();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END;

    CAssetStoreGlobal::Instance()->GetRegistry().RemoveProvider( "restartstore" );
    CAssetStoreGlobal::Instance()->GetProviderFactory().UnregisterConcreteFactory( "restartstore" );
    CFakeStoreProvider::SetServer( GUCEF_NULL );
    server.Stop();
    analyzers.Unregister( analyzer->GetName() );
    UnmountDirectory( "svcrs_" + label + "_source" );
    UnmountDirectory( "svcrs_" + label + "_main" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORETEST */

/*-------------------------------------------------------------------------*/

#endif /* TESTASSETSTORESERVICESUITE_H ? */
