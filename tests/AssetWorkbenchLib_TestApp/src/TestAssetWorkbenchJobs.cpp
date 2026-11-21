/*
 *  AssetWorkbenchLib_TestApp: tests of the AssetWorkbenchLib module
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
#include <Ogre.h>

#include "gucef_test_framework.h"

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CGLOBPATTERN_H
#include "gucefCORE_CGlobPattern.h"
#define GUCEF_CORE_CGLOBPATTERN_H
#endif /* GUCEF_CORE_CGLOBPATTERN_H ? */

#ifndef GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#include "gucefASSETSTORE_CLocalAssetStoreService.h"
#define GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CLOCALASSETSTORESERVICE_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#include "gucefASSETSTORE_CAssetPipelineOperations.h"
#define GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETPIPELINEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#include "gucefASSETSTORE_CAssetServiceOperations.h"
#define GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H
#endif /* GUCEF_ASSETSTORE_CASSETSERVICEOPERATIONS_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#include "gucefASSETSTORE_CAssetLibrarySyncPlan.h"
#define GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H
#endif /* GUCEF_ASSETSTORE_CASSETLIBRARYSYNCPLAN_H ? */

#ifndef GUCEF_GUI_CLAYOUTLINT_H
#include "gucefGUI_CLayoutLint.h"
#define GUCEF_GUI_CLAYOUTLINT_H
#endif /* GUCEF_GUI_CLAYOUTLINT_H ? */

#include "AssetWorkbenchLib.h"
#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestFakeAssetStoreService.h"
#include "TestAssetWorkbenchJobs.h"

/*-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------*/
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  The pipeline of the tests, in the place of the converter: packs "alpha" and "beta" of N assets "models/m<i>.fbx", and "restricted",
 *  whose licence forbids the conversion. A run reports progress, can be slowed (to cancel it), fails the assets of 'failing', and keeps
 *  what it did in a cache: a second run does not do it again (cached) unless it reimports. Every request is kept, for the tests to read.
 */
class CPipelineHook : public ASSETSTORE::CIAssetConverterHook
{
    public:

    MT::CMutex lock;
    GUCEF::vector< ASSETSTORE::CAssetPipelineRequest > runs;
    GUCEF::vector< ASSETSTORE::CAssetPipelineRequest > plans;
    CORE::CString::StringSet failing;
    CORE::CString::StringSet cache;
    UInt32 assetDelayInMs;
    UInt32 assetCount;

    CPipelineHook( void ) : ASSETSTORE::CIAssetConverterHook(), lock(), runs(), plans(), failing(), cache(), assetDelayInMs( 0 ), assetCount( 4 ) {}

    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "test pipeline"; }
    virtual CORE::CString GetProfileName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "test"; }
    virtual bool CanConvert( const ASSETSTORE::CAssetRecord& ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual ASSETSTORE::TAssetStoreStatus ConvertPack( const ASSETSTORE::CAssetConversionRequest&, ASSETSTORE::TAssetRecordVector&, ASSETSTORE::CAssetConversionOutcome& ) GUCEF_VIRTUAL_OVERRIDE { return ASSETSTORE::ASSETSTORE_STATUS_UNSUPPORTED; }
    virtual bool IsConversionStale( const ASSETSTORE::CAssetConversionRecord&, CORE::CString& ) const GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool SupportsPipelineOperation( const CORE::CString& operation ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return ASSETSTORE::CAssetPipelineOperations::ConvertPlan == operation || ASSETSTORE::CAssetPipelineOperations::ConvertPack == operation ||
               ASSETSTORE::CAssetPipelineOperations::CookPack == operation || ASSETSTORE::CAssetPipelineOperations::ValidatePack == operation;
    }

    void GetScope( const ASSETSTORE::CAssetPipelineRequest& request, CORE::CString::StringVector& paths ) const
    {
        for ( UInt32 i=1; i<=assetCount; ++i )
        {
            const CORE::CString path = "models/m" + CORE::UInt32ToString( i ) + ".fbx";
            bool in = request.assets.empty();
            for ( size_t g=0; g<request.assets.size() && !in; ++g )
                in = request.assets[ g ] == path || CORE::CGlobPattern::IsMatch( request.assets[ g ], path, false );
            if ( in )
                paths.push_back( path );
        }
    }

    UInt32 GetRunCount( void )
    {
        MT::CScopeMutex scope( lock );
        return (UInt32) runs.size();
    }

    bool GetRun( UInt32 index, ASSETSTORE::CAssetPipelineRequest& request )
    {
        MT::CScopeMutex scope( lock );
        if ( index >= runs.size() )
            return false;
        request = runs[ index ];
        return true;
    }

    virtual ASSETSTORE::TAssetStoreStatus RunPipelineOperation( const ASSETSTORE::CAssetPipelineRequest& request ,
                                                                ASSETSTORE::CIAssetConversionObserver* observer ,
                                                                ASSETSTORE::CAssetServiceDocument& result       ) GUCEF_VIRTUAL_OVERRIDE
    {
        result.Clear();
        result.document.SetName( "result" );
        result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
        CORE::CString::StringVector paths;
        GetScope( request, paths );
        result.document.SetAttribute( "operation", request.operation );
        result.document.SetAttribute( "packId", request.packId );

        if ( ASSETSTORE::CAssetPipelineOperations::ConvertPlan == request.operation )
        {
            {
                MT::CScopeMutex scope( lock );
                plans.push_back( request );
            }
            CORE::CDataNode* licence = result.document.AddChild( "licence", GUCEF_DATATYPE_OBJECT );
            const bool allowed = "restricted" != request.packId;
            licence->SetAttribute( "conversionAllowed", allowed );
            if ( !allowed )
                licence->SetAttribute( "reason", "the licence of the pack does not allow a conversion" );
            CORE::CString::StringVector restrictions;
            restrictions.push_back( "no machine learning" );
            licence->SetValuesOfChildByName( "restrictions", restrictions );
            UInt32 imports = 0, cached = 0;
            CORE::CDataNode* items = result.document.AddChild( "items", GUCEF_DATATYPE_ARRAY );
            for ( size_t i=0; i<paths.size(); ++i )
            {
                MT::CScopeMutex scope( lock );
                const bool hit = !request.reimport && cache.find( request.packId + ":" + paths[ i ] ) != cache.end();
                CORE::CDataNode* item = items->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                item->SetAttribute( "assetPath", paths[ i ] );
                item->SetAttribute( "action", hit ? "cached" : ( allowed ? "import" : "refused" ) );
                item->SetAttribute( "reason", hit ? "unchanged" : ( request.reimport ? "forced" : "new" ) );
                item->SetAttribute( "detail", CORE::CString::Empty );
                if ( hit )
                    ++cached;
                else
                if ( allowed )
                    ++imports;
            }
            CORE::CDataNode* counts = result.document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
            counts->SetAttribute( "import", imports );
            counts->SetAttribute( "cached", cached );
            counts->SetAttribute( "cook", (UInt32) 0 );
            counts->SetAttribute( "refused", allowed ? (UInt32) 0 : (UInt32) paths.size() );
            counts->SetAttribute( "failed", (UInt32) 0 );
            counts->SetAttribute( "validate", (UInt32) 0 );
            counts->SetAttribute( "noIntermediate", (UInt32) 0 );
            counts->SetAttribute( "filteredOut", (UInt32) ( assetCount - paths.size() ) );
            result.document.SetAttribute( "lastRunAssetTimeMs", (UInt32) 0 );
            return ASSETSTORE::ASSETSTORE_STATUS_OK;
        }

        {
            MT::CScopeMutex scope( lock );
            runs.push_back( request );
        }
        UInt32 converted = 0, cached = 0, failed = 0;
        CORE::CDataNode* assets = result.document.AddChild( "assets", GUCEF_DATATYPE_ARRAY );
        CORE::CDataNode* failures = result.document.AddChild( "failures", GUCEF_DATATYPE_ARRAY );
        const bool indexOnly = request.arguments.GetAttributeValueOrChildValueByName( "indexOnly" ).AsBool( false, true );
        for ( size_t i=0; i<paths.size() && !indexOnly; ++i )
        {
            if ( GUCEF_NULL != observer && observer->IsCancelRequested() )
            {
                result.message = "cancelled";
                return ASSETSTORE::ASSETSTORE_STATUS_CANCELLED;
            }
            if ( assetDelayInMs > 0 )
                MT::PrecisionDelay( assetDelayInMs );
            const CORE::CString key = request.packId + ":" + paths[ i ];
            CORE::CString state;
            {
                MT::CScopeMutex scope( lock );
                if ( failing.find( paths[ i ] ) != failing.end() )
                    state = "failed";
                else
                if ( !request.reimport && cache.find( key ) != cache.end() )
                    state = "cached";
                else
                {
                    state = "converted";
                    cache.insert( key );
                }
            }
            CORE::CDataNode* asset = assets->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            asset->SetAttribute( "assetPath", paths[ i ] );
            asset->SetAttribute( "state", state );
            asset->SetAttribute( "durationMs", assetDelayInMs );
            if ( "failed" == state )
            {
                ++failed;
                CORE::CDataNode* failure = failures->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
                failure->SetAttribute( "assetPath", paths[ i ] );
                failure->SetAttribute( "source", "test importer" );
                failure->SetAttribute( "message", "the file cannot be read" );
            }
            else
            if ( "cached" == state )
                ++cached;
            else
                ++converted;
            if ( GUCEF_NULL != observer )
                observer->OnConversionProgress( (UInt32) i + 1, (UInt32) paths.size(), paths[ i ] );
        }
        CORE::CDataNode* counts = result.document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        counts->SetAttribute( "converted", converted );
        counts->SetAttribute( "cached", cached );
        counts->SetAttribute( "failed", failed );
        result.document.SetAttribute( "durationMs", (UInt32) 5 );
        result.document.SetAttribute( "intermediateFolder", "intermediate/" + request.packId );
        return ASSETSTORE::ASSETSTORE_STATUS_OK;
    }
};

void
Pump( UInt32 times = 1 )
{
    for ( UInt32 i=0; i<times; ++i )
    {
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::PrecisionDelay( 3 );
    }
}

/** A service of this process with the pipeline of the tests */
class CServiceFixture
{
    public:

    CPipelineHook hook;
    ASSETSTORE::CAssetStoreServiceConfig config;
    ASSETSTORE::CLocalAssetStoreService* service;

    CServiceFixture( void ) : hook(), config(), service( GUCEF_NULL )
    {
        config.primaryStore.database.backendName = "none";
        config.loadDownloadQueue = false;
        config.startDownloads = false;
        service = GUCEF_NEW ASSETSTORE::CLocalAssetStoreService( config );
        service->SetConverterHook( &hook );
        service->Open();
    }

    ~CServiceFixture() { service->Close(); GUCEF_DELETE service; }

    /** Starts a pipeline job; the id of the job */
    CORE::CString Start( const ASSETSTORE::CAssetPipelineRequest& request )
    {
        ASSETSTORE::CAssetServiceResult result;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != service->StartPipelineJob( CORE::CString::Empty, request, result ) )
            return CORE::CString::Empty;
        return result.id;
    }

    bool WaitForState( const CORE::CString& jobId, ASSETSTORE::TAssetJobState state, ASSETSTORE::CAssetJobEntry& job, UInt32 maxMs = 8000 )
    {
        for ( UInt32 waited=0; waited<maxMs; waited+=5 )
        {
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == service->GetJob( jobId, job ) && job.state == state )
                return true;
            Pump();
        }
        return ASSETSTORE::ASSETSTORE_STATUS_OK == service->GetJob( jobId, job ) && job.state == state;
    }

    bool WaitForEnd( const CORE::CString& jobId, ASSETSTORE::CAssetJobEntry& job, UInt32 maxMs = 8000 )
    {
        for ( UInt32 waited=0; waited<maxMs; waited+=5 )
        {
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == service->GetJob( jobId, job ) && job.IsEnded() )
                return true;
            Pump();
        }
        return ASSETSTORE::ASSETSTORE_STATUS_OK == service->GetJob( jobId, job ) && job.IsEnded();
    }

    /** The id of a job of the service other than those known */
    CORE::CString FindOtherJob( const CORE::CString& known )
    {
        ASSETSTORE::CAssetJobPage page;
        service->ListJobs( CORE::CString::Empty, 100, page );
        for ( size_t i=0; i<page.entries.size(); ++i )
            if ( page.entries[ i ].jobId != known && page.entries[ i ].kind == "pipeline" )
                return page.entries[ i ].jobId;
        return CORE::CString::Empty;
    }
};

ASSETSTORE::CAssetPipelineRequest
MakeConvert( const CORE::CString& packId )
{
    ASSETSTORE::CAssetPipelineRequest request;
    request.operation = ASSETSTORE::CAssetPipelineOperations::ConvertPack;
    request.packId = packId;
    return request;
}

ASSETSTORE::CAssetJobEntry
MakeServiceJob( const CORE::CString& jobId, const CORE::CString& kind, ASSETSTORE::TAssetJobState state )
{
    ASSETSTORE::CAssetJobEntry job;
    job.jobId = jobId;
    job.kind = kind;
    job.name = jobId;
    job.state = state;
    job.createdAtMs = 1000;
    job.updatedAtMs = 2500;
    return job;
}

UInt64 g_fakeNow = 0;
UInt64 FakeClock( void* ) { return g_fakeNow; }

/** The text of the flags a pipeline request carries, to compare with what an action is meant to do */
CORE::CString
FlagsOf( const ASSETSTORE::CAssetPipelineRequest& request )
{
    CORE::CString flags = request.operation;
    static const char* const names[] = { "indexOnly", "reindex", "analyse", "reanalyse", "rethumbnail", "retryFailed", "noCook" };
    for ( UInt32 i=0; i<7; ++i )
        if ( request.arguments.GetAttributeValueOrChildValueByName( names[ i ] ).AsBool( false, true ) )
            flags += CORE::CString( "+" ) + names[ i ];
    if ( request.reimport )
        flags += "+reimport";
    return flags;
}

/** The shell with the asset sources and the pipeline of the tests, in a window */
class CJobsFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CWorkbenchShell* shell;
    CPipelineHook hook;
    CORE::CString dir;
    bool ready;

    explicit CJobsFixture( const CORE::CString& name )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), shell( GUCEF_NULL ), hook(), dir(), ready( false )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        const CORE::CString storeDir = dir + "/store";
        CORE::CreateDirs( storeDir );
        CAssetSourcesSettings settings;
        settings.primaryStore.locations[ 0 ].vfsRoot = "avjobs/" + name;
        settings.primaryStore.locations[ 0 ].fileSystemPath = storeDir;
        settings.primaryStore.database.database = "avjobs/" + name + "/catalogue";
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        ready = shell->Initialize( *context, GUCEF_NULL, MakeCleanTestDir( name + "_cache" ) ) && shell->InitializeSources( settings ) && GUCEF_NULL != shell->GetSources();
        if ( ready )
            shell->GetSources()->SetConverterHook( &hook );
    }

    ~CJobsFixture()
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        contextPtr.Unlink();
        mounter.UnmountAll();
    }

    IMAGE::TPixelMapPtr Frames( UInt32 count )
    {
        IMAGE::TPixelMapPtr pixels;
        for ( UInt32 i=0; i<count; ++i )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            shell->Update( 5 );
            window->RenderFrame( &pixels );
            MT::PrecisionDelay( 3 );
        }
        return pixels;
    }

    ASSETSTORE::CIAssetStoreService& Service( void ) { return *shell->GetSources()->GetService(); }

    CORE::CString Start( const ASSETSTORE::CAssetPipelineRequest& request )
    {
        ASSETSTORE::CAssetServiceResult result;
        if ( ASSETSTORE::ASSETSTORE_STATUS_OK != Service().StartPipelineJob( CORE::CString::Empty, request, result ) )
            return CORE::CString::Empty;
        return result.id;
    }

    bool WaitForEnd( const CORE::CString& jobId, UInt32 maxFrames = 800 )
    {
        ASSETSTORE::CAssetJobEntry job;
        for ( UInt32 i=0; i<maxFrames; ++i )
        {
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == Service().GetJob( jobId, job ) && job.IsEnded() )
                return true;
            Frames( 1 );
        }
        return false;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS: the jobs                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestRegistryAndFilters( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Jobs: an operation of the workbench is a job of the list only when it ran long enough (300 ms), with its progress, its cancel and its end; the quick ones leave nothing" )
    try
    {
        CWorkbenchJobRegistry registry;
        g_fakeNow = 1000;
        registry.SetClock( &FakeClock, GUCEF_NULL );
        ASSERT_TRUE( 300 == registry.GetVisibleAfterMs() );

        // a quick one: not in the list while it runs, gone when it ends
        const UInt32 quick = registry.Begin( "open", CORE::CString::Empty, "Opening a.png", "a.png", true );
        ASSETSTORE::TAssetJobEntryVector jobs;
        registry.GetJobs( jobs );
        ASSERT_TRUE( jobs.empty() );
        g_fakeNow += 120;
        registry.End( quick, ASSETSTORE::ASSETJOB_STATE_DONE, "done" );
        registry.GetJobs( jobs );
        ASSERT_TRUE( jobs.empty() );

        // a long one shows after 300 ms, with progress; cancel is asked, then it ends
        const UInt32 slow = registry.Begin( "index", "analyse", "Analysing pack", "packs/alpha", true );
        g_fakeNow += 299;
        registry.GetJobs( jobs );
        ASSERT_TRUE( jobs.empty() );
        g_fakeNow += 2;
        registry.SetProgress( slow, 3, 12 );
        registry.GetJobs( jobs );
        ASSERT_TRUE( 1 == jobs.size() && "local:" + CORE::UInt32ToString( slow ) == jobs[ 0 ].jobId && "index" == jobs[ 0 ].kind && "analyse" == jobs[ 0 ].subKind );
        ASSERT_TRUE( ASSETSTORE::ASSETJOB_STATE_RUNNING == jobs[ 0 ].state && 3 == jobs[ 0 ].itemsDone && 12 == jobs[ 0 ].itemsTotal && jobs[ 0 ].fraction > 0.24f && jobs[ 0 ].fraction < 0.26f );
        ASSERT_TRUE( "packs/alpha" == jobs[ 0 ].scope && jobs[ 0 ].IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) && !jobs[ 0 ].IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );
        ASSERT_TRUE( registry.RequestCancel( slow ) && registry.IsCancelRequested( slow ) && !registry.RequestCancel( slow + 100 ) );
        g_fakeNow += 1000;
        registry.End( slow, ASSETSTORE::ASSETJOB_STATE_CANCELLED, "cancelled" );
        jobs.clear();
        registry.GetJobs( jobs );
        ASSERT_TRUE( 1 == jobs.size() && jobs[ 0 ].IsEnded() && ASSETSTORE::ASSETJOB_STATE_CANCELLED == jobs[ 0 ].state && jobs[ 0 ].IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );
        ASSERT_TRUE( jobs[ 0 ].updatedAtMs - jobs[ 0 ].createdAtMs >= 1300 && 0 != jobs[ 0 ].createdAtMs );
        ASSERT_TRUE( registry.Remove( jobs[ 0 ].jobId ) );
        jobs.clear();
        registry.GetJobs( jobs );
        ASSERT_TRUE( jobs.empty() );

        // an operation that announces a long duration is a job at once; the list of the ended ones is bounded
        const UInt32 announced = registry.Begin( "presentation", CORE::CString::Empty, "Opening pack", "packs/beta", false, 5000 );
        registry.GetJobs( jobs );
        ASSERT_TRUE( 1 == jobs.size() && !jobs[ 0 ].IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) );
        registry.End( announced, ASSETSTORE::ASSETJOB_STATE_DONE, "done" );
        for ( UInt32 i=0; i<(UInt32) CWorkbenchJobRegistry::MaxEndedJobs + 20; ++i )
        {
            const UInt32 number = registry.Begin( "thumbnail", CORE::CString::Empty, "t", "t", false, 1000 );
            registry.End( number, ASSETSTORE::ASSETJOB_STATE_DONE, "kept" );
        }
        jobs.clear();
        registry.GetJobs( jobs );
        ASSERT_TRUE( jobs.size() <= (size_t) CWorkbenchJobRegistry::MaxEndedJobs );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Jobs: the filters by state group and by kind, the kinds the list shows, and the counts of the status area" )
    try
    {
        ASSETSTORE::TAssetJobEntryVector jobs;
        jobs.push_back( MakeServiceJob( "pipeline:pj1", "pipeline", ASSETSTORE::ASSETJOB_STATE_RUNNING ) );
        jobs.back().subKind = "convert";
        jobs.push_back( MakeServiceJob( "pipeline:pj2", "pipeline", ASSETSTORE::ASSETJOB_STATE_QUEUED ) );
        jobs.back().subKind = "cook";
        jobs.push_back( MakeServiceJob( "pipeline:pj3", "pipeline", ASSETSTORE::ASSETJOB_STATE_FAILED ) );
        jobs.back().subKind = "convert";
        jobs.push_back( MakeServiceJob( "ingest:i1", "ingest", ASSETSTORE::ASSETJOB_STATE_DONE ) );
        jobs.push_back( MakeServiceJob( "dl1", "download", ASSETSTORE::ASSETJOB_STATE_PAUSED ) );
        jobs.push_back( MakeServiceJob( "convert:cv1", "conversion", ASSETSTORE::ASSETJOB_STATE_CANCELLED ) );
        jobs.push_back( MakeServiceJob( "local:1", "index", ASSETSTORE::ASSETJOB_STATE_FINISHING ) );
        jobs.back().stateText = "writing";

        ASSERT_TRUE( "convert" == CWorkbenchJobFilter::DisplayKindOf( jobs[ 0 ] ) && "ingest" == CWorkbenchJobFilter::DisplayKindOf( jobs[ 3 ] ) && "writing" == CWorkbenchJobFilter::StateTextOf( jobs[ 6 ] ) );
        const char* const groups[] = { "all", "active", "running", "queued", "failed", "done", "cancelled", "finished" };
        const UInt32 expected[] = { 7, 4, 2, 1, 1, 1, 1, 3 };
        for ( UInt32 g=0; g<8; ++g )
        {
            CWorkbenchJobFilter filter;
            ASSERT_TRUE( CWorkbenchJobFilter::ParseStateGroup( groups[ g ], filter.states ) && CORE::CString( groups[ g ] ) == CWorkbenchJobFilter::GetStateGroupName( filter.states ) );
            UInt32 matches = 0;
            for ( size_t i=0; i<jobs.size(); ++i )
                if ( filter.Matches( jobs[ i ] ) )
                    ++matches;
            ASSERT_TRUE( expected[ g ] == matches );
        }
        CWorkbenchJobFilter byKind;
        byKind.kind = "convert";
        UInt32 converts = 0;
        for ( size_t i=0; i<jobs.size(); ++i )
            if ( byKind.Matches( jobs[ i ] ) )
                ++converts;
        ASSERT_TRUE( 2 == converts );
        CWorkbenchJobFilter both;
        both.kind = "convert";
        both.states = CWorkbenchJobFilter::STATES_FAILED;
        ASSERT_TRUE( !both.Matches( jobs[ 0 ] ) && both.Matches( jobs[ 2 ] ) );
        CWorkbenchJobFilter::EStateGroup unknown;
        ASSERT_FALSE( CWorkbenchJobFilter::ParseStateGroup( "nonsense", unknown ) );

        const CWorkbenchJobSummary summary = CWorkbenchJobSummary::Count( jobs );
        ASSERT_TRUE( 3 == summary.running && 1 == summary.queued && 1 == summary.failed && 1 == summary.done && 1 == summary.cancelled && 7 == summary.GetTotal() );
        ASSERT_TRUE( "Jobs: 3 running, 1 queued, 1 failed" == summary.ToText() );
        ASSERT_TRUE( "Jobs: 3 run, 1 wait, 1 failed" == summary.ToShortText() );
        ASSERT_TRUE( "Jobs: idle" == CWorkbenchJobSummary().ToShortText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestListFromService( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Jobs: one list for the service in this process and the workbench: scope, kind, result and times of a run of the pipeline; the transfer queue is the view of the jobs of the service" )
    try
    {
        CServiceFixture fixture;
        CWorkbenchJobRegistry registry;
        registry.SetVisibleAfterMs( 0 );
        CAssetStoreTransferModel model( *fixture.service );
        model.SetLocalJobs( &registry );

        const UInt32 localOpen = registry.Begin( "open", CORE::CString::Empty, "Opening a.fbx", "a.fbx", true );
        const CORE::CString jobId = fixture.Start( MakeConvert( "alpha" ) );
        ASSERT_TRUE( !jobId.IsNULLOrEmpty() );
        ASSETSTORE::CAssetJobEntry done;
        ASSERT_TRUE( fixture.WaitForEnd( jobId, done ) && ASSETSTORE::ASSETJOB_STATE_DONE == done.state );

        ASSETSTORE::TAssetJobEntryVector jobs;
        model.GetJobs( jobs );
        ASSERT_TRUE( 2 == jobs.size() );
        const ASSETSTORE::CAssetJobEntry* pipeline = jobs[ 0 ].jobId == jobId ? &jobs[ 0 ] : &jobs[ 1 ];
        const ASSETSTORE::CAssetJobEntry* local = jobs[ 0 ].jobId == jobId ? &jobs[ 1 ] : &jobs[ 0 ];
        ASSERT_TRUE( "pipeline" == pipeline->kind && "convert" == pipeline->subKind && "alpha" == pipeline->scope && "alpha" == pipeline->packId );
        ASSERT_TRUE( "4 converted" == pipeline->resultText && 4 == pipeline->itemsDone && 4 == pipeline->itemsTotal && "intermediate/alpha" == pipeline->folder );
        ASSERT_TRUE( pipeline->createdAtMs > 0 && pipeline->updatedAtMs >= pipeline->createdAtMs );
        ASSERT_TRUE( "open" == local->kind && local->jobId == "local:" + CORE::UInt32ToString( localOpen ) );
        ASSERT_TRUE( model.IsJobActionAvailable( local->jobId, ASSETSTORE::ASSETJOB_ACTION_CANCEL ) && model.IsJobActionAvailable( jobId, ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );

        // the transfer queue is the same list, filtered to the work of the service
        GUI::CITransferQueueModel::TEntryVector entries;
        model.GetEntries( entries );
        ASSERT_TRUE( 1 == entries.size() && entries[ 0 ].id == jobId );
        const CWorkbenchJobSummary summary = model.GetSummary();
        ASSERT_TRUE( 1 == summary.running && 1 == summary.done );

        // cancel and remove of a local job through the list; remove of a job of the service
        ASSERT_TRUE( model.ControlJob( local->jobId, ASSETSTORE::ASSETJOB_ACTION_CANCEL ) && registry.IsCancelRequested( localOpen ) );
        registry.End( localOpen, ASSETSTORE::ASSETJOB_STATE_CANCELLED, "cancelled" );
        ASSERT_TRUE( model.ControlJob( "local:" + CORE::UInt32ToString( localOpen ), ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );
        ASSERT_TRUE( model.ControlJob( jobId, ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );
        model.GetJobs( jobs );
        ASSERT_TRUE( jobs.empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Jobs: a service that is not in process (the server): its jobs are mirrored, resumable by id, with the new fields; the events update the list, the actions are sent to the server" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        service.SetInProcess( false );
        service.SetDeferAsync( true );
        ASSETSTORE::CAssetJobPage page;
        ASSETSTORE::CAssetJobEntry server = MakeServiceJob( "pipeline:pj7", "pipeline", ASSETSTORE::ASSETJOB_STATE_FAILED );
        server.subKind = "convert";
        server.scope = "alpha";
        server.resultText = "3 converted, 1 failed";
        server.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY, true );
        server.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RESUME, true );
        page.entries.push_back( server );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListJobs, ASSETSTORE::ASSETSTORE_STATUS_OK, &page );
        ASSETSTORE::CAssetServiceResult accepted;
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ControlJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &accepted );

        CAssetStoreTransferModel model( service );
        ASSETSTORE::TAssetJobEntryVector jobs;
        model.GetJobs( jobs );
        ASSERT_TRUE( jobs.empty() && 1 == service.GetPendingCallCount() );
        service.CompletePendingCalls();
        model.Update();
        model.GetJobs( jobs );
        ASSERT_TRUE( 1 == jobs.size() && "convert" == CWorkbenchJobFilter::DisplayKindOf( jobs[ 0 ] ) && "3 converted, 1 failed" == jobs[ 0 ].resultText && "alpha" == jobs[ 0 ].scope );
        ASSERT_TRUE( 1 == model.GetSummary().failed );
        ASSETSTORE::CAssetJobEntry found;
        ASSERT_TRUE( model.GetJob( "pipeline:pj7", found ) && !model.GetJob( "pipeline:none", found ) );

        // an action is sent with the id of the job of the server
        ASSERT_TRUE( model.ControlJob( "pipeline:pj7", ASSETSTORE::ASSETJOB_ACTION_RESUME ) );
        ASSETSTORE::CAssetServiceCall call;
        service.CompletePendingCalls();
        ASSERT_TRUE( service.GetLastCall( ASSETSTORE::CAssetServiceOperations::ControlJob, call ) && "pipeline:pj7" == call.GetText( "jobId" ) && "resume" == call.GetText( "action" ) );
        ASSERT_TRUE( model.ControlJob( "pipeline:pj7", ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
        service.CompletePendingCalls();
        ASSERT_TRUE(  service.GetLastCall( ASSETSTORE::CAssetServiceOperations::ControlJob, call ) && "retry" == call.GetText( "action" ) );

        // the job the server changed is taken from the event; a job of the workbench is listed next to it
        server.state = ASSETSTORE::ASSETJOB_STATE_DONE;
        server.resultText = "4 converted";
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, server );
        CWorkbenchJobRegistry registry;
        registry.SetVisibleAfterMs( 0 );
        model.SetLocalJobs( &registry );
        registry.Begin( "thumbnail", CORE::CString::Empty, "Thumbnail", "packs/alpha", false );
        model.GetJobs( jobs );
        ASSERT_TRUE( 2 == jobs.size() && ASSETSTORE::ASSETJOB_STATE_DONE == jobs[ 0 ].state && "4 converted" == jobs[ 0 ].resultText && "thumbnail" == jobs[ 1 ].kind );
        ASSERT_TRUE( !model.ControlJob( "local:99", ASSETSTORE::ASSETJOB_ACTION_CANCEL ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestCancelResumeRetry( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Jobs: a run that is cancelled goes on where it stopped (resume), a run with failed assets does those alone (retry); both are new jobs of the service" )
    try
    {
        CServiceFixture fixture;
        fixture.hook.assetDelayInMs = 60;
        fixture.hook.assetCount = 8;
        CAssetStoreTransferModel model( *fixture.service );

        const CORE::CString first = fixture.Start( MakeConvert( "alpha" ) );
        ASSETSTORE::CAssetJobEntry job;
        for ( UInt32 i=0; i<400 && ( ASSETSTORE::ASSETSTORE_STATUS_OK != fixture.service->GetJob( first, job ) || job.itemsDone < 2 ); ++i )
            Pump();
        ASSERT_TRUE( job.itemsDone >= 2 && ASSETSTORE::ASSETJOB_STATE_RUNNING == job.state && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) );
        ASSERT_FALSE( job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RESUME ) );
        ASSERT_TRUE( model.ControlJob( first, ASSETSTORE::ASSETJOB_ACTION_CANCEL ) );
        ASSERT_TRUE( fixture.WaitForState( first, ASSETSTORE::ASSETJOB_STATE_CANCELLED, job ) && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RESUME ) && !job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
        const UInt32 doneBefore = (UInt32) fixture.hook.cache.size();
        ASSERT_TRUE( doneBefore >= 2 && doneBefore < 8 );

        // resume: a new job with the same request, the cache keeps what was done
        fixture.hook.assetDelayInMs = 0;
        ASSERT_TRUE( model.ControlJob( first, ASSETSTORE::ASSETJOB_ACTION_RESUME ) );
        const CORE::CString second = fixture.FindOtherJob( first );
        ASSERT_TRUE( !second.IsNULLOrEmpty() );
        ASSERT_TRUE( fixture.WaitForEnd( second, job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state );
        CORE::CString expectedText = CORE::UInt32ToString( 8 - doneBefore ) + " converted, " + CORE::UInt32ToString( doneBefore ) + " cached";
        ASSERT_TRUE( job.scope == "alpha" && job.resultText == expectedText );
        ASSERT_TRUE( 2 == fixture.hook.GetRunCount() );

        // retry of the failed assets: only those, flagged
        fixture.hook.failing.insert( "models/m3.fbx" );
        fixture.hook.failing.insert( "models/m5.fbx" );
        const CORE::CString third = fixture.Start( MakeConvert( "beta" ) );
        ASSERT_TRUE( fixture.WaitForEnd( third, job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY ) && !job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RESUME ) && "partly failed" == job.stateText && CWorkbenchJobFilter::IsFailed( job ) );
        ASSERT_TRUE( "6 converted, 2 failed" == job.resultText );
        fixture.hook.failing.clear();
        const UInt32 runsBefore = fixture.hook.GetRunCount();
        ASSERT_TRUE( model.ControlJob( third, ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
        CORE::CString fourth;
        for ( UInt32 i=0; i<400 && fourth.IsNULLOrEmpty(); ++i )
        {
            ASSETSTORE::CAssetJobPage page;
            fixture.service->ListJobs( CORE::CString::Empty, 100, page );
            for ( size_t p=0; p<page.entries.size(); ++p )
                if ( page.entries[ p ].jobId != first && page.entries[ p ].jobId != second && page.entries[ p ].jobId != third )
                    fourth = page.entries[ p ].jobId;
            Pump();
        }
        ASSERT_TRUE( !fourth.IsNULLOrEmpty() && fixture.WaitForEnd( fourth, job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state );
        ASSETSTORE::CAssetPipelineRequest retried;
        ASSERT_TRUE( fixture.hook.GetRunCount() == runsBefore + 1 && fixture.hook.GetRun( runsBefore, retried ) );
        ASSERT_TRUE( 2 == retried.assets.size() && "models/m3.fbx" == retried.assets[ 0 ] && "models/m5.fbx" == retried.assets[ 1 ] && "beta" == retried.packId );
        ASSERT_TRUE( retried.arguments.GetAttributeValueOrChildValueByName( "retryFailed" ).AsBool( false, true ) && "2 converted" == job.resultText );

        // a job that is done is not taken up again
        ASSERT_FALSE( model.ControlJob( fourth, ASSETSTORE::ASSETJOB_ACTION_RETRY ) || model.IsJobActionAvailable( fourth, ASSETSTORE::ASSETJOB_ACTION_RESUME ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

namespace {

/** Takes its time in Prepare() in small steps and watches for cancellation */
class CSlowLoadJob : public CIAssetLoadJob
{
    public:

    UInt32 prepareInMs;

    explicit CSlowLoadJob( UInt32 time ) : CIAssetLoadJob(), prepareInMs( time ) {}

    virtual bool Prepare( CAssetLoadControl& control ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( UInt32 waited=0; waited<prepareInMs; waited+=10 )
        {
            if ( control.IsCancelled() )
                return false;
            MT::PrecisionDelay( 10 );
        }
        return true;
    }

    virtual EStepResult Step( void ) GUCEF_VIRTUAL_OVERRIDE { return STEP_DONE; }
    virtual void Discard( void ) GUCEF_VIRTUAL_OVERRIDE {}
};

}

static void
TestEveryLongOperationIsAJob( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Jobs: every long operation of the workbench registers as a job: open, index, presentation and thumbnail with their kinds and cancel, the work of the service through the list; short ones leave no job" )
    try
    {
        GUI::CActivityService activity;
        CWorkbenchJobRegistry registry;
        registry.SetActivityService( &activity );
        registry.SetVisibleAfterMs( 60 );
        CAssetLoader loader( &activity );
        loader.SetJobRegistry( &registry, "open" );

        // the operations the workbench starts: (what the shell tells the loader before Begin())
        struct SOperation { const char* kind; const char* subKind; const char* scope; };
        const SOperation operations[] = { { "open", "", "packs/a/model.fbx" }, { "index", "", "packs/a" }, { "index", "analyse", "packs/a" }, { "presentation", "", "packs/a" } };
        for ( UInt32 o=0; o<4; ++o )
        {
            loader.DescribeNext( operations[ o ].kind, operations[ o ].subKind, operations[ o ].scope );
            TAssetLoadJobPtr job( GUCEF_NEW CSlowLoadJob( 220 ) );
            loader.Begin( job, CORE::CString( "Doing " ) + operations[ o ].kind );
            for ( UInt32 i=0; i<600 && loader.IsBusy(); ++i )
            {
                loader.Update();
                MT::PrecisionDelay( 5 );
                activity.Update();
            }
            ASSERT_TRUE( !loader.IsBusy() );
        }
        ASSETSTORE::TAssetJobEntryVector jobs;
        registry.GetJobs( jobs );
        ASSERT_TRUE( 4 == jobs.size() );
        for ( UInt32 o=0; o<4; ++o )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "JobsTest: job " << o << " kind=" << jobs[ o ].kind << " sub=" << jobs[ o ].subKind << " scope=" << jobs[ o ].scope << " state=" << (Int32) jobs[ o ].state );
            ASSERT_TRUE( CORE::CString( operations[ o ].kind ) == jobs[ o ].kind && CORE::CString( operations[ o ].subKind ) == jobs[ o ].subKind && CORE::CString( operations[ o ].scope ) == jobs[ o ].scope );
            ASSERT_TRUE( ASSETSTORE::ASSETJOB_STATE_DONE == jobs[ o ].state && jobs[ o ].updatedAtMs - jobs[ o ].createdAtMs >= 200 );
        }

        // a quick open leaves nothing; a cancelled one is a cancelled job
        loader.DescribeNext( "open", CORE::CString::Empty, "quick" );
        loader.Begin( TAssetLoadJobPtr( GUCEF_NEW CSlowLoadJob( 0 ) ), "Opening quick" );
        for ( UInt32 i=0; i<200 && loader.IsBusy(); ++i )
        {
            loader.Update();
            MT::PrecisionDelay( 2 );
        }
        loader.DescribeNext( "open", CORE::CString::Empty, "long" );
        loader.Begin( TAssetLoadJobPtr( GUCEF_NEW CSlowLoadJob( 5000 ) ), "Opening long" );
        for ( UInt32 i=0; i<200; ++i )
        {
            MT::PrecisionDelay( 2 );
            activity.Update();
            loader.Update();
        }
        jobs.clear();
        registry.GetJobs( jobs );
        ASSERT_TRUE( 5 == jobs.size() && "long" == jobs[ 4 ].scope && !jobs[ 4 ].IsEnded() );
        // the indicator links to the job, the list cancels through the activity
        GUI::CActivityService::TActivityInfoVector shown;
        activity.GetShownOperations( shown );
        ASSERT_TRUE( 1 == shown.size() && registry.FindByActivity( shown[ 0 ].id ) == CWorkbenchJobRegistry::NumberOfId( jobs[ 4 ].jobId ) );
        ASSERT_TRUE( registry.RequestCancel( CWorkbenchJobRegistry::NumberOfId( jobs[ 4 ].jobId ) ) && activity.IsCancelRequested( shown[ 0 ].id ) );
        activity.Update();
        for ( UInt32 i=0; i<400 && loader.IsBusy(); ++i )
        {
            loader.Update();
            MT::PrecisionDelay( 5 );
            activity.Update();
        }
        jobs.clear();
        registry.GetJobs( jobs );
        ASSERT_TRUE( 5 == jobs.size() && ASSETSTORE::ASSETJOB_STATE_CANCELLED == jobs[ 4 ].state );

        // the kinds of the list name every one of them (the work of the service has its own kinds: download, ingest, conversion, pipeline)
        CORE::CString::StringVector kinds;
        CWorkbenchJobFilter::GetKnownKinds( kinds );
        const char* const needed[] = { "open", "index", "analyse", "presentation", "thumbnail", "convert", "cook", "validate", "download", "ingest", "conversion" };
        for ( UInt32 n=0; n<11; ++n )
        {
            bool known = false;
            for ( size_t k=0; k<kinds.size() && !known; ++k )
                known = kinds[ k ] == needed[ n ];
            ASSERT_TRUE( known );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS: the bulk conversion                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestBulkSettings( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Bulk conversion: the settings become the request of the pipeline (operation, globs of the scope, targets, profile, flags), are validated, and survive being saved as a template" )
    try
    {
        CBulkConversionSettings settings;
        CORE::CString reason;
        ASSERT_FALSE( settings.IsValid( reason ) || reason.IsNULLOrEmpty() );
        settings.packId = "alpha";
        ASSERT_TRUE( settings.IsValid( reason ) && "convert_pack" == settings.GetOperation() );

        ASSETSTORE::CAssetPipelineRequest request;
        ASSERT_TRUE( settings.MakeRequest( "alpha", CORE::CString::StringVector(), false, request ) );
        ASSERT_TRUE( "convert_pack" == request.operation && "alpha" == request.packId && request.assets.empty() && 1 == request.cookTargets.size() && "gucef-ogre3d" == request.cookTargets[ 0 ] && !request.reimport );
        ASSERT_TRUE( "convert_pack" == FlagsOf( request ) );

        // the plan asks the plan operation with the same scope and flags
        settings.force = true;
        settings.cookProfile = "mobile";
        settings.analyse = true;
        ASSERT_TRUE( settings.MakeRequest( "alpha", CORE::CString::StringVector(), true, request ) );
        ASSERT_TRUE( "convert_plan" == request.operation && request.reimport && 1 == request.profiles.size() && "mobile" == request.profiles[ 0 ] && "convert_plan+analyse+reimport" == FlagsOf( request ) );

        // intermediate only: the cook is left out; validate only and cook only are other operations
        CBulkConversionSettings intermediate;
        intermediate.packId = "alpha";
        intermediate.ogreRuntime = false;
        ASSERT_TRUE( intermediate.IsValid( reason ) && intermediate.MakeRequest( "alpha", CORE::CString::StringVector(), false, request ) && request.cookTargets.empty() && "convert_pack+noCook" == FlagsOf( request ) );
        intermediate.intermediate = false;
        ASSERT_FALSE( intermediate.IsValid( reason ) );
        CBulkConversionSettings validate;
        validate.packId = "alpha";
        validate.validateOnly = true;
        ASSERT_TRUE( validate.MakeRequest( "alpha", CORE::CString::StringVector(), false, request ) && "validate_pack" == request.operation && request.cookTargets.empty() );
        CBulkConversionSettings cook;
        cook.packId = "alpha";
        cook.cookOnly = true;
        cook.force = true;
        ASSERT_TRUE( cook.MakeRequest( "alpha", CORE::CString::StringVector(), false, request ) && "cook_pack" == request.operation && !request.reimport && 1 == request.cookTargets.size() );
        cook.validateOnly = true;
        ASSERT_FALSE( cook.IsValid( reason ) );

        // the scopes: a folder is a glob, a selection a list, the filter narrows both
        CBulkConversionSettings folder;
        folder.scopeKind = CBulkConversionSettings::ScopeFolder;
        folder.packId = "alpha";
        ASSERT_FALSE( folder.IsValid( reason ) );
        folder.folder = "models/props/";
        CORE::CString::StringVector globs;
        folder.GetScopeGlobs( globs );
        ASSERT_TRUE( folder.IsValid( reason ) && 1 == globs.size() && "models/props/*" == globs[ 0 ] );
        folder.assetFilter = "*.fbx";
        ASSERT_TRUE( folder.MakeRequest( "alpha", globs, false, request ) && 1 == request.assets.size() && "models/props/*.fbx" == request.assets[ 0 ] );
        CBulkConversionSettings selection;
        selection.scopeKind = CBulkConversionSettings::ScopeSelection;
        selection.packId = "alpha";
        ASSERT_FALSE( selection.IsValid( reason ) );
        selection.assets.push_back( "models/a.fbx" );
        selection.assets.push_back( "textures/a.png" );
        selection.assetFilter = "models/*";
        selection.GetScopeGlobs( globs );
        ASSERT_TRUE( selection.IsValid( reason ) && selection.MakeRequest( "alpha", globs, false, request ) && 1 == request.assets.size() && "models/a.fbx" == request.assets[ 0 ] );
        selection.assetFilter = "*.tga";
        ASSERT_FALSE( selection.MakeRequest( "alpha", globs, false, request ) );
        CBulkConversionSettings search;
        search.scopeKind = CBulkConversionSettings::ScopeSavedSearch;
        ASSERT_FALSE( search.IsValid( reason ) );
        search.savedSearchId = "s1";
        ASSERT_TRUE( search.IsValid( reason ) && !search.Describe().IsNULLOrEmpty() );

        // a template: saved under its name in the preferences, written to the settings and read back
        CBulkConversionSettings named = selection;
        named.name = "Weekly props";
        named.assetFilter = "models/*";
        named.cookProfile = "desktop";
        named.force = true;
        CWorkbenchPreferences preferences;
        ASSERT_FALSE( preferences.SetBulkTemplate( CBulkConversionSettings() ) );
        ASSERT_TRUE( preferences.SetBulkTemplate( named ) );
        named.force = false;
        named.name = "Cook all";
        named.cookOnly = true;
        ASSERT_TRUE( preferences.SetBulkTemplate( named ) );
        CBulkConversionSettings renamed = named;
        renamed.name = "Weekly props";
        renamed.cookProfile = "console";
        ASSERT_TRUE( preferences.SetBulkTemplate( renamed ) && 2 == preferences.bulkTemplates.size() );
        CORE::CDataNode section( "AssetWorkbench" );
        preferences.Save( section );
        CWorkbenchPreferences loaded;
        loaded.Load( section );
        CORE::CString::StringVector names;
        loaded.GetBulkTemplateNames( names );
        ASSERT_TRUE( 2 == names.size() && "Weekly props" == names[ 0 ] && "Cook all" == names[ 1 ] );
        CBulkConversionSettings back;
        ASSERT_TRUE( loaded.GetBulkTemplate( "Weekly props", back ) && "console" == back.cookProfile && back.assetFilter == "models/*" && 2 == back.assets.size() && !back.force );
        ASSERT_TRUE( loaded.GetBulkTemplate( "Cook all", back ) && back.cookOnly && CBulkConversionSettings::ScopeSelection == back.scopeKind );
        ASSERT_TRUE( loaded.RemoveBulkTemplate( "Cook all" ) && !loaded.GetBulkTemplate( "Cook all", back ) && 1 == loaded.bulkTemplates.size() );
        for ( UInt32 i=0; i<(UInt32) CWorkbenchPreferences::MaxBulkTemplates + 5; ++i )
        {
            CBulkConversionSettings many = named;
            many.name = "T" + CORE::UInt32ToString( i );
            preferences.SetBulkTemplate( many );
        }
        ASSERT_TRUE( preferences.bulkTemplates.size() == (size_t) CWorkbenchPreferences::MaxBulkTemplates );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestBulkPlanAndRun( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Bulk conversion: the plan (per asset action, reason and work, the counts, the licence refusal), the start as a job of the service with progress, the notice when it ends, and the report per asset" )
    try
    {
        CServiceFixture fixture;
        CBulkConversionController controller( *fixture.service );

        CBulkConversionSettings settings;
        settings.packId = "alpha";
        ASSERT_TRUE( controller.RequestPlan( settings ) && !controller.IsPlanning() );
        const CBulkConversionPlan& plan = controller.GetPlan();
        ASSERT_TRUE( plan.valid && plan.conversionAllowed && 4 == plan.importCount && 0 == plan.cachedCount && 4 == plan.GetWorkCount() && 4 == plan.items.size() );
        ASSERT_TRUE( "import" == plan.items[ 0 ].action && "new" == plan.items[ 0 ].reason && "models/m1.fbx" == plan.items[ 0 ].assetPath && "import and export" == CBulkConversionPlan::DescribeWork( plan.items[ 0 ] ) );
        ASSERT_TRUE( plan.CanStart() && controller.CanStart() && plan.ToSummaryText().HasSubstr( "4 to import" ) == 0 && 1 == plan.restrictions.size() );
        ASSETSTORE::CAssetPipelineRequest planned;
        ASSERT_TRUE( 1 == fixture.hook.plans.size() && "convert_plan" == fixture.hook.plans[ 0 ].operation && 1 == fixture.hook.plans[ 0 ].cookTargets.size() );

        // a pack whose licence forbids the conversion is refused with the reason, and cannot be started
        CBulkConversionSettings restricted;
        restricted.packId = "restricted";
        ASSERT_TRUE( controller.RequestPlan( restricted ) );
        ASSERT_TRUE( !controller.GetPlan().conversionAllowed && !controller.CanStart() && controller.GetPlan().ToSummaryText().HasSubstr( "Refused: " ) == 0 && controller.GetPlan().ToSummaryText().HasSubstr( "does not allow" ) > 0 );
        ASSERT_TRUE( 0 == controller.Start() && !controller.GetProblem().IsNULLOrEmpty() && 0 == fixture.hook.GetRunCount() );

        // the plan of a folder shows what the scope narrows to
        CBulkConversionSettings folder;
        folder.scopeKind = CBulkConversionSettings::ScopeFolder;
        folder.packId = "alpha";
        folder.folder = "models";
        ASSERT_TRUE( controller.RequestPlan( folder ) && 4 == controller.GetPlan().importCount );
        CBulkConversionSettings selection;
        selection.scopeKind = CBulkConversionSettings::ScopeSelection;
        selection.packId = "alpha";
        selection.assets.push_back( "models/m2.fbx" );
        ASSERT_TRUE( controller.RequestPlan( selection ) && 1 == controller.GetPlan().importCount && 1 == controller.GetPlan().items.size() && 3 == controller.GetPlan().filteredOutCount );

        // start: a job of the service; progress and the end are the job's; the notice and the per asset report follow
        fixture.hook.assetDelayInMs = 30;
        ASSERT_TRUE( controller.RequestPlan( settings ) );
        class CNoticeRecorder : public CORE::CObserver
        {
            public:
            UInt32 count;
            CORE::CString last;
            CNoticeRecorder( CORE::CNotifier* notifier ) : CORE::CObserver(), count( 0 ), last() { SubscribeTo( notifier, CBulkConversionController::NotificationEvent ); }
            virtual ~CNoticeRecorder() { UnsubscribeAllFromObserver(); }
            virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& id, CORE::CICloneable* data ) GUCEF_VIRTUAL_OVERRIDE
            {
                if ( id != CBulkConversionController::NotificationEvent )
                    return;
                const CORE::TCloneableString* text = static_cast< const CORE::TCloneableString* >( data );
                if ( GUCEF_NULL != text )
                    last = text->GetData();
                ++count;
            }
        } notices( &controller );
        ASSERT_TRUE( 1 == controller.Start() && 1 == controller.GetJobIds().size() );
        const CORE::CString jobId = controller.GetJobIds()[ 0 ];
        ASSETSTORE::CAssetJobEntry job;
        bool sawProgress = false;
        for ( UInt32 i=0; i<600 && !( ASSETSTORE::ASSETSTORE_STATUS_OK == fixture.service->GetJob( jobId, job ) && job.IsEnded() ); ++i )
        {
            if ( job.itemsDone > 0 && job.itemsDone < job.itemsTotal )
                sawProgress = true;
            Pump();
        }
        ASSERT_TRUE( sawProgress && ASSETSTORE::ASSETJOB_STATE_DONE == job.state && "4 converted" == job.resultText && "alpha" == job.scope );
        Pump( 60 );
        ASSERT_TRUE( 1 == notices.count && notices.last.HasSubstr( "alpha" ) >= 0 && notices.last.HasSubstr( "done" ) >= 0 && 1 == controller.GetNotifications().size() );

        // what was done is kept: a plan now shows it as up to date and has no work; forcing makes it work again
        ASSERT_TRUE( controller.RequestPlan( settings ) && 4 == controller.GetPlan().cachedCount && 0 == controller.GetPlan().importCount && !controller.CanStart() && 0 == controller.Start() );
        settings.force = true;
        ASSERT_TRUE( controller.RequestPlan( settings ) && 4 == controller.GetPlan().importCount && controller.GetPlan().items[ 0 ].reason == "forced" );

        // the report of the job, per asset
        ASSETSTORE::CAssetServiceDocument result;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == fixture.service->GetJobResult( jobId, CORE::CString::Empty, CORE::CString::Empty, 0, result ) );
        GUI::CReportViewForm::TEntryVector entries;
        ASSERT_TRUE( 4 == CWorkbenchJobReport::BuildEntries( result, entries ) && 5 == entries.size() );
        ASSERT_TRUE( entries[ 0 ].message.HasSubstr( "4 converted" ) >= 0 && "models/m1.fbx" == entries[ 1 ].asset && "converted" == entries[ 1 ].message && GUI::CReportViewForm::SEVERITY_INFO == entries[ 1 ].severity );

        // the failed assets are errors in the report and the failed job of the retry names them
        fixture.hook.failing.insert( "models/m4.fbx" );
        CBulkConversionSettings beta;
        beta.packId = "beta";
        ASSERT_TRUE( controller.RequestPlan( beta ) && 1 == controller.Start() );
        const CORE::CString betaJob = controller.GetJobIds().back();
        ASSERT_TRUE( fixture.WaitForEnd( betaJob, job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state && job.IsActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY ) );
        Pump( 60 );
        ASSERT_TRUE( 2 == controller.GetNotifications().size() && controller.GetNotifications()[ 1 ].HasSubstr( "failed" ) >= 0 );
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == fixture.service->GetJobResult( betaJob, CORE::CString::Empty, CORE::CString::Empty, 0, result ) );
        CORE::CString::StringVector failed;
        CWorkbenchJobReport::GetFailedAssets( result, failed );
        ASSERT_TRUE( 1 == failed.size() && "models/m4.fbx" == failed[ 0 ] );
        CWorkbenchJobReport::BuildEntries( result, entries );
        UInt32 errors = 0;
        for ( size_t e=0; e<entries.size(); ++e )
            if ( GUI::CReportViewForm::SEVERITY_FAILURE == entries[ e ].severity )
                ++errors;
        ASSERT_TRUE( 2 == errors );
        fixture.hook.failing.clear();
        const UInt32 jobsBefore = (UInt32) controller.GetJobIds().size();
        ASSERT_TRUE( controller.RetryFailedOfJob( betaJob ) && controller.GetJobIds().size() == jobsBefore + 1 );
        ASSERT_TRUE( fixture.WaitForEnd( controller.GetJobIds().back(), job ) && ASSETSTORE::ASSETJOB_STATE_DONE == job.state );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Bulk conversion: against a service that is not in process the plan and the start are asked and answered later, the job is the server's and is followed by its id" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        service.SetInProcess( false );
        service.SetDeferAsync( true );
        ASSETSTORE::CAssetServiceDocument planAnswer;
        planAnswer.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
        planAnswer.document.SetName( "result" );
        planAnswer.document.SetAttribute( "packId", "alpha" );
        CORE::CDataNode* licence = planAnswer.document.AddChild( "licence", GUCEF_DATATYPE_OBJECT );
        licence->SetAttribute( "conversionAllowed", true );
        CORE::CDataNode* counts = planAnswer.document.AddChild( "counts", GUCEF_DATATYPE_OBJECT );
        counts->SetAttribute( "import", (UInt32) 5 );
        counts->SetAttribute( "cached", (UInt32) 2 );
        counts->SetAttribute( "cook", (UInt32) 1 );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::RunPipelineQuery, ASSETSTORE::ASSETSTORE_STATUS_OK, &planAnswer );
        ASSETSTORE::CAssetServiceResult started;
        started.id = "pipeline:pj42";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &started );

        CBulkConversionController controller( service );
        CBulkConversionSettings settings;
        settings.packId = "alpha";
        ASSERT_TRUE( controller.RequestPlan( settings ) && controller.IsPlanning() && !controller.CanStart() );
        service.CompletePendingCalls();
        controller.Update();
        ASSERT_TRUE( !controller.IsPlanning() && 5 == controller.GetPlan().importCount && 2 == controller.GetPlan().cachedCount && controller.CanStart() );
        ASSERT_TRUE( 1 == controller.Start() && controller.GetJobIds().empty() );
        ASSETSTORE::CAssetServiceCall call;
        service.CompletePendingCalls();
        ASSERT_TRUE( service.GetLastCall( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, call ) );
        ASSETSTORE::CAssetPipelineRequest sent;
        ASSERT_TRUE( call.GetRecord( "request", sent ) && "convert_pack" == sent.operation && "alpha" == sent.packId && !call.commandId.IsNULLOrEmpty() );
        service.CompletePendingCalls();
        controller.Update();
        ASSERT_TRUE( 1 == controller.GetJobIds().size() && "pipeline:pj42" == controller.GetJobIds()[ 0 ] );

        // the job of the server is followed when it ends, also when the client found it by its id after a reconnect
        controller.TrackJob( "pipeline:pj99" );
        ASSETSTORE::CAssetJobEntry end = MakeServiceJob( "pipeline:pj99", "pipeline", ASSETSTORE::ASSETJOB_STATE_DONE );
        end.scope = "beta";
        end.subKind = "convert";
        end.resultText = "7 converted";
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, end );
        ASSERT_TRUE( 1 == controller.GetNotifications().size() && controller.GetNotifications()[ 0 ].HasSubstr( "7 converted" ) >= 0 );
        ASSERT_TRUE( controller.CancelJob( "pipeline:pj42" ) );
        service.CompletePendingCalls();
        ASSERT_TRUE(  service.GetLastCall( ASSETSTORE::CAssetServiceOperations::ControlJob, call ) && "cancel" == call.GetText( "action" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS: the redo actions                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
TestRedoActions( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Redo: every action carries exactly its flags (and no other), names what it invalidates and keeps, and the local ones map to the redo request of the index" )
    try
    {
        struct SExpect { CWorkbenchRedoActions::ERedoAction action; const char* id; const char* flags; const char* flag; bool local; };
        const SExpect expected[] =
        {
            { CWorkbenchRedoActions::REDO_REINDEX,     "reindex",     "convert_pack+indexOnly+reindex",                     "--reindex",     true },
            { CWorkbenchRedoActions::REDO_REANALYSE,   "reanalyse",   "convert_pack+indexOnly+analyse+reanalyse",           "--reanalyse",   true },
            { CWorkbenchRedoActions::REDO_REIMPORT,    "reimport",    "convert_pack+reimport",                              "--reimport",    false },
            { CWorkbenchRedoActions::REDO_RECONVERT,   "reconvert",   "cook_pack",                                          "--cook-only",   false },
            { CWorkbenchRedoActions::REDO_RETHUMBNAIL, "rethumbnail", "convert_pack+indexOnly+rethumbnail",                 "--rethumbnail", true }
        };
        CORE::CString::StringVector globs;
        globs.push_back( "models/*" );
        for ( UInt32 i=0; i<5; ++i )
        {
            CWorkbenchRedoActions::ERedoAction parsed = CWorkbenchRedoActions::REDO_COUNT;
            ASSERT_TRUE( CWorkbenchRedoActions::Parse( expected[ i ].id, parsed ) && parsed == expected[ i ].action );
            ASSERT_TRUE( CWorkbenchRedoActions::ParseActionId( CWorkbenchRedoActions::GetActionId( parsed ), parsed ) && parsed == expected[ i ].action );
            const CWorkbenchRedoActions::SRedoInfo& info = CWorkbenchRedoActions::GetInfo( expected[ i ].action );
            ASSERT_TRUE( CORE::CString( expected[ i ].flag ) == info.flag && 0 != info.invalidates[ 0 ] && 0 != info.keeps[ 0 ] && 0 != info.menuText[ 0 ] );
            ASSETSTORE::CAssetPipelineRequest request;
            CWorkbenchRedoActions::MakePipelineRequest( expected[ i ].action, "alpha", globs, request );
            ASSERT_TRUE( CORE::CString( expected[ i ].flags ) == FlagsOf( request ) && "alpha" == request.packId && 1 == request.assets.size() );
            ASSERT_TRUE( ( expected[ i ].local ) == CWorkbenchRedoActions::IsLocal( expected[ i ].action ) );
            ASSETCONVERTER::CPackRedoRequest redo;
            ASSERT_TRUE( expected[ i ].local == CWorkbenchRedoActions::MakeLocalRedo( expected[ i ].action, redo ) );
            if ( expected[ i ].local )
            {
                // exactly one flag of the local request is on, the one of the action
                const UInt32 on = ( redo.reindex ? 1 : 0 ) + ( redo.reanalyse ? 1 : 0 ) + ( redo.rethumbnail ? 1 : 0 ) + ( redo.reimport ? 1 : 0 ) + ( redo.retryFailed ? 1 : 0 ) + ( redo.verifyContent ? 1 : 0 );
                ASSERT_TRUE( 1 == on && ( CWorkbenchRedoActions::REDO_REINDEX != expected[ i ].action || redo.reindex ) );
            }
            ASSERT_TRUE( CWorkbenchRedoActions::MakeConfirmation( expected[ i ].action, "the pack alpha", 12 ).HasSubstr( "12 assets" ) > 0 );
        }
        CWorkbenchRedoActions::ERedoAction none;
        ASSERT_FALSE( CWorkbenchRedoActions::Parse( "redoeverything", none ) || CWorkbenchRedoActions::ParseActionId( "Redo:nothing", none ) );

        // through the service: counters of the flags the pipeline saw, per action (nothing else is invalidated)
        CServiceFixture fixture;
        for ( UInt32 i=0; i<5; ++i )
        {
            ASSETSTORE::CAssetPipelineRequest request;
            CWorkbenchRedoActions::MakePipelineRequest( expected[ i ].action, "alpha", CORE::CString::StringVector(), request );
            const CORE::CString jobId = fixture.Start( request );
            ASSETSTORE::CAssetJobEntry job;
            ASSERT_TRUE( !jobId.IsNULLOrEmpty() && fixture.WaitForEnd( jobId, job ) );
            ASSERT_TRUE( CORE::CString( CWorkbenchRedoActions::GetJobSubKind( expected[ i ].action ) ) == job.subKind && "alpha" == job.scope );
            ASSETSTORE::CAssetPipelineRequest seen;
            ASSERT_TRUE( fixture.hook.GetRun( i, seen ) && CORE::CString( expected[ i ].flags ) == FlagsOf( seen ) );
        }
        UInt32 indexRuns = 0, reimports = 0, cooks = 0;
        for ( UInt32 i=0; i<fixture.hook.GetRunCount(); ++i )
        {
            ASSETSTORE::CAssetPipelineRequest seen;
            fixture.hook.GetRun( i, seen );
            if ( seen.arguments.GetAttributeValueOrChildValueByName( "indexOnly" ).AsBool( false, true ) )
                ++indexRuns;
            if ( seen.reimport )
                ++reimports;
            if ( "cook_pack" == seen.operation )
                ++cooks;
        }
        ASSERT_TRUE( 3 == indexRuns && 1 == reimports && 1 == cooks );
        // an index run converts nothing: the cache of the hook only holds what the import and convert actions did
        ASSERT_TRUE( 4 == fixture.hook.cache.size() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS: the pages in a window                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

static GUI::CWidget*
RootOf( GUI::CFormEx* form )
{
    return GUCEF_NULL != form ? form->GetRootWidget() : GUCEF_NULL;
}

/** The layout lint over a page at the sizes of the window, 0 problems at warning level and above */
static bool
LintPage( CJobsFixture& fixture, GUI::CFormEx* form, const char* title )
{
    const Int32 widths[] = { 0, 1000, 1600 };
    const Int32 heights[] = { 0, 700, 900 };
    bool clean = true;
    for ( UInt32 size=0; size<3; ++size )
    {
        fixture.context->SetLogicalResolution( widths[ size ], heights[ size ] );
        fixture.Frames( 3 );
        GUI::CLayoutLintOptions options;
        options.root = RootOf( form );
        GUI::TLayoutLintFindingVector findings;
        GUI::CLayoutLint lint;
        lint.Run( *fixture.context, options, findings );
        const UInt32 problems = GUI::CLayoutLint::CountAtLeast( findings, GUI::LAYOUTLINTSEVERITY_WARNING );
        if ( 0 != problems )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Jobs lint " << title << " at " << widths[ size ] << "x" << heights[ size ] << ": " << GUI::CLayoutLint::Report( findings, 20 ) );
            clean = false;
        }
    }
    fixture.context->SetLogicalResolution( 0, 0 );
    return clean;
}

static void
TestJobsPage( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Jobs page: the list of the service and of the workbench with the columns, the filters, the run history, cancel, resume and retry from the list, the report of a finished job, the status summary and the layout lint" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CJobsFixture fixture( "jobs_page" );
        ASSERT_TRUE( fixture.ready );
        if ( !fixture.ready )
            return;
        CWorkbenchShell& shell = *fixture.shell;
        shell.GetJobRegistry().SetVisibleAfterMs( 0 );

        // several jobs: done, done with failed assets, cancelled, running, local
        const CORE::CString done = fixture.Start( MakeConvert( "alpha" ) );
        ASSERT_TRUE( fixture.WaitForEnd( done ) );
        fixture.hook.failing.insert( "models/m2.fbx" );
        const CORE::CString failed = fixture.Start( MakeConvert( "beta" ) );
        ASSERT_TRUE( fixture.WaitForEnd( failed ) );
        fixture.hook.failing.clear();
        fixture.hook.assetDelayInMs = 120;
        fixture.hook.assetCount = 12;
        ASSETSTORE::CAssetPipelineRequest cookRequest = MakeConvert( "gamma" );
        cookRequest.operation = ASSETSTORE::CAssetPipelineOperations::CookPack;
        const CORE::CString cancelled = fixture.Start( cookRequest );
        for ( UInt32 i=0; i<400; ++i )
        {
            ASSETSTORE::CAssetJobEntry probe;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == fixture.Service().GetJob( cancelled, probe ) && probe.itemsDone >= 1 )
                break;
            fixture.Frames( 1 );
        }
        ASSETSTORE::CAssetServiceResult control;
        ASSERT_TRUE( ASSETSTORE::ASSETSTORE_STATUS_OK == fixture.Service().ControlJob( CORE::CString::Empty, cancelled, ASSETSTORE::ASSETJOB_ACTION_CANCEL, control ) );
        ASSERT_TRUE( fixture.WaitForEnd( cancelled ) );
        ASSETSTORE::CAssetPipelineRequest validateRequest = MakeConvert( "delta" );
        validateRequest.operation = ASSETSTORE::CAssetPipelineOperations::ValidatePack;
        fixture.hook.assetCount = 40;           // long enough to still run when the list is read, whatever the speed of the renderer
        const CORE::CString running = fixture.Start( validateRequest );
        const UInt32 localJob = shell.GetJobRegistry().Begin( "index", "analyse", "Analysing pack alpha", "packs/alpha", true );
        shell.GetJobRegistry().SetProgress( localJob, 40, 100 );
        fixture.Frames( 6 );

        ASSERT_TRUE( shell.ShowJobs() );
        CWorkbenchJobsForm* form = shell.GetJobsForm();
        ASSERT_TRUE( GUCEF_NULL != form && form->IsOpen() );
        fixture.Frames( 4 );
        form->Refresh();
        ASSERT_TRUE( 5 == form->GetShownJobCount() );
        ASSERT_TRUE( "Kind" == CWorkbenchJobsForm::GetColumnTitle( CWorkbenchJobsForm::COLUMN_KIND ) && "Duration" == CWorkbenchJobsForm::GetColumnTitle( CWorkbenchJobsForm::COLUMN_DURATION ) );

        // the newest job first; the columns say kind, scope, state, progress, times, duration and result
        ASSETSTORE::CAssetJobEntry row;
        ASSERT_TRUE( form->GetShownJob( 0, row ) && "local:" + CORE::UInt32ToString( localJob ) == row.jobId );
        ASSERT_TRUE( "analyse" == form->GetCellText( 0, CWorkbenchJobsForm::COLUMN_KIND ) && "packs/alpha" == form->GetCellText( 0, CWorkbenchJobsForm::COLUMN_SCOPE ) );
        ASSERT_TRUE( "running" == form->GetCellText( 0, CWorkbenchJobsForm::COLUMN_STATE ) && "40 %" == form->GetCellText( 0, CWorkbenchJobsForm::COLUMN_PROGRESS ) );
        UInt32 doneRow = 99, failedRow = 99, cancelledRow = 99;
        for ( UInt32 r=0; r<form->GetShownJobCount(); ++r )
        {
            form->GetShownJob( r, row );
            if ( row.jobId == done )
                doneRow = r;
            if ( row.jobId == failed )
                failedRow = r;
            if ( row.jobId == cancelled )
                cancelledRow = r;
        }
        ASSERT_TRUE( doneRow < 5 && failedRow < 5 && cancelledRow < 5 );
        ASSERT_TRUE( "convert" == form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_KIND ) && "alpha" == form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_SCOPE ) && "done" == form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_STATE ) );
        ASSERT_TRUE( "100 %" == form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_PROGRESS ) && "4 converted" == form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_RESULT ) );
        ASSERT_TRUE( !form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_STARTED ).IsNULLOrEmpty() && !form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_ENDED ).IsNULLOrEmpty() && !form->GetCellText( doneRow, CWorkbenchJobsForm::COLUMN_DURATION ).IsNULLOrEmpty() );
        ASSERT_TRUE( "partly failed" == form->GetCellText( failedRow, CWorkbenchJobsForm::COLUMN_STATE ) && "3 converted, 1 failed" == form->GetCellText( failedRow, CWorkbenchJobsForm::COLUMN_RESULT ) );
        ASSERT_TRUE( "cook" == form->GetCellText( cancelledRow, CWorkbenchJobsForm::COLUMN_KIND ) && "cancelled" == form->GetCellText( cancelledRow, CWorkbenchJobsForm::COLUMN_STATE ) );
        ASSERT_TRUE( "0 s" != CWorkbenchJobsForm::FormatDuration( 0 ) && "950 ms" == CWorkbenchJobsForm::FormatDuration( 950 ) && "1.5 s" == CWorkbenchJobsForm::FormatDuration( 1500 ) && "2 min 5 s" == CWorkbenchJobsForm::FormatDuration( 125000 ) );

        // the summary of the status area
        shell.RefreshJobSummary();
        const CWorkbenchJobSummary summary = shell.GetJobSummary();
        ASSERT_TRUE( 2 == summary.running && 1 == summary.done && 1 == summary.failed && 1 == summary.cancelled );
        ASSERT_TRUE( GUCEF_NULL != shell.GetJobSummaryButton() );
        ASSERT_TRUE( summary.ToShortText() == shell.GetJobSummaryButton()->GetProperty( GUI::EWidgetProperty_Text ) );

        // the filters: state groups, kind, the history
        form->SetStateGroup( CWorkbenchJobFilter::STATES_FAILED );
        ASSERT_TRUE( 1 == form->GetShownJobCount() );
        form->SetStateGroup( CWorkbenchJobFilter::STATES_FINISHED );
        ASSERT_TRUE( 3 == form->GetShownJobCount() );
        form->SetStateGroup( CWorkbenchJobFilter::STATES_ACTIVE );
        ASSERT_TRUE( 2 == form->GetShownJobCount() );
        form->SetStateGroup( CWorkbenchJobFilter::STATES_ALL );
        form->SetKindFilter( "analyse" );
        ASSERT_TRUE( 1 == form->GetShownJobCount() );
        form->SetKindFilter( "cook" );
        ASSERT_TRUE( 1 == form->GetShownJobCount() );
        form->SetKindFilter( CORE::CString::Empty );
        ASSERT_TRUE( 5 == form->GetShownJobCount() );

        // the buttons follow the job: retry for the failed one, resume for the cancelled, cancel for the one that runs, a report for those that ended
        ASSERT_TRUE( form->SelectJob( failed ) && form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_RETRY ) && !form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_RESUME ) && !form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) && form->IsReportAvailable() );
        ASSERT_TRUE( form->SelectJob( done ) && !form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_RETRY ) && !form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_RESUME ) && form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_REMOVE ) );
        ASSERT_TRUE( form->SelectJob( "local:" + CORE::UInt32ToString( localJob ) ) && form->IsActionEnabled( ASSETSTORE::ASSETJOB_ACTION_CANCEL ) && !form->IsReportAvailable() && !form->ShowReport() );
        ASSERT_TRUE( form->CancelSelected() && shell.GetJobRegistry().IsCancelRequested( localJob ) );
        shell.GetJobRegistry().End( localJob, ASSETSTORE::ASSETJOB_STATE_CANCELLED, "cancelled" );
        fixture.hook.assetDelayInMs = 0;
        fixture.hook.assetCount = 4;
        form->Refresh();

        // the report of the failed job in the report view: a row for the run and for each asset, the failures as errors
        ASSERT_TRUE( form->SelectJob( failed ) && form->ShowReport() );
        GUI::CReportViewForm* report = form->GetReportForm();
        ASSERT_TRUE( GUCEF_NULL != report && failed == form->GetReportJobId() && 6 <= report->GetEntryCount() && 2 == report->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE ) );
        fixture.Frames( 3 );
        DumpPicture( fixture.Frames( 3 ), "f158_report" );

        // retry from the list: a new job of the service for the failed asset alone; resume of the cancelled one
        ASSERT_TRUE( form->SelectJob( failed ) && form->RetrySelected() );
        form->Refresh();
        ASSERT_TRUE( 6 == form->GetShownJobCount() || 5 < form->GetShownJobCount() );
        ASSERT_TRUE( form->SelectJob( cancelled ) && form->ResumeSelected() );
        fixture.Frames( 40 );
        form->Refresh();
        report->Hide();
        DumpPicture( fixture.Frames( 3 ), "f158_jobs" );

        // the transfer queue shows the jobs of the service and not the operations of the workbench
        GUI::CITransferQueueModel::TEntryVector entries;
        shell.GetSources()->GetTransferModel()->GetEntries( entries );
        for ( size_t e=0; e<entries.size(); ++e )
            ASSERT_FALSE( CWorkbenchJobRegistry::IsLocalId( entries[ e ].id ) );
        ASSERT_TRUE( LintPage( fixture, form, "Jobs" ) );
        form->Close();
        ASSERT_FALSE( form->IsOpen() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestBulkPanel( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Bulk panel: scope, targets and options, the plan view with its numbers, start and progress, the notice at the end, retry, the templates and the layout lint" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CJobsFixture fixture( "bulk_panel" );
        ASSERT_TRUE( fixture.ready );
        if ( !fixture.ready )
            return;
        CWorkbenchShell& shell = *fixture.shell;
        ASSERT_TRUE( shell.ShowJobs() );
        ASSERT_TRUE( shell.ShowBulkConversion() );
        CBulkConversionForm* form = shell.GetBulkConversionForm();
        ASSERT_TRUE( GUCEF_NULL != form && form->IsOpen() && GUCEF_NULL != shell.GetBulkConversionController() );
        fixture.Frames( 3 );

        CBulkConversionSettings settings;
        settings.scopeKind = CBulkConversionSettings::ScopeSelection;
        settings.packId = "alpha";
        settings.assets.push_back( "models/m1.fbx" );
        settings.assets.push_back( "models/m2.fbx" );
        settings.assets.push_back( "models/m3.fbx" );
        settings.cookProfile = "desktop";
        settings.assetFilter = "models/*";
        form->SetSettings( settings );
        const CBulkConversionSettings read = form->GetSettings();
        ASSERT_TRUE( CBulkConversionSettings::ScopeSelection == read.scopeKind && "alpha" == read.packId && 3 == read.assets.size() && "desktop" == read.cookProfile && "models/*" == read.assetFilter && read.intermediate && read.ogreRuntime && !read.force );

        // the plan first: numbers per action and the assets of the scope; start is only allowed after it
        ASSERT_FALSE( form->IsStartEnabled() );
        ASSERT_TRUE( form->PlanNow() );
        ASSERT_TRUE( 3 == form->GetPlanRowCount() && form->GetPlanSummaryText().HasSubstr( "3 to import" ) == 0 && form->IsStartEnabled() );
        ASSERT_TRUE( "models/m1.fbx" == form->GetPlanCellText( 0, 0 ) && "import" == form->GetPlanCellText( 0, 1 ) && "new" == form->GetPlanCellText( 0, 2 ) && "import and export" == form->GetPlanCellText( 0, 3 ) );
        fixture.Frames( 3 );
        DumpPicture( fixture.Frames( 3 ), "f158_bulk_plan" );

        // a pack whose licence forbids it is refused with the reason
        CBulkConversionSettings restricted;
        restricted.packId = "restricted";
        form->SetSettings( restricted );
        ASSERT_TRUE( form->PlanNow() && !form->IsStartEnabled() && form->GetPlanSummaryText().HasSubstr( "Refused" ) == 0 && !form->StartNow() );
        form->SetSettings( settings );
        ASSERT_TRUE( form->PlanNow() && form->IsStartEnabled() );

        // start: the job is the service's; the form follows its progress, then the notice
        fixture.hook.assetDelayInMs = 150;
        ASSERT_TRUE( form->StartNow() && !form->GetCurrentJobId().IsNULLOrEmpty() );
        const CORE::CString jobId = form->GetCurrentJobId();
        bool sawProgress = false;
        for ( UInt32 i=0; i<800; ++i )
        {
            fixture.Frames( 1 );
            form->Update( 300 );
            if ( form->GetJobStatusText().HasSubstr( " of 3 assets" ) > 0 && form->GetJobStatusText().HasSubstr( "running" ) > 0 )
                sawProgress = true;
            ASSETSTORE::CAssetJobEntry job;
            if ( ASSETSTORE::ASSETSTORE_STATUS_OK == fixture.Service().GetJob( jobId, job ) && job.IsEnded() )
                break;
        }
        fixture.Frames( 12 );
        form->Update( 400 );
        ASSERT_TRUE( sawProgress && form->GetJobStatusText().HasSubstr( "done" ) > 0 && form->GetJobStatusText().HasSubstr( "3 converted" ) > 0 );
        ASSERT_TRUE( form->GetNoticeText().HasSubstr( "done" ) >= 0 && 1 == shell.GetBulkConversionController()->GetNotifications().size() );
        ASSERT_TRUE( fixture.hook.GetRunCount() == 1 );
        ASSETSTORE::CAssetPipelineRequest sent;
        ASSERT_TRUE( fixture.hook.GetRun( 0, sent ) && 3 == sent.assets.size() && 1 == sent.cookTargets.size() && 1 == sent.profiles.size() );
        DumpPicture( fixture.Frames( 3 ), "f158_bulk_done" );

        // templates: saved in the preferences by name, loaded back into the panel, deleted
        bool changed = false;
        class CChangedRecorder : public CORE::CObserver
        {
            public:
            bool* flag;
            CChangedRecorder( CORE::CNotifier* notifier, bool* target ) : CORE::CObserver(), flag( target ) { SubscribeTo( notifier, CBulkConversionForm::TemplatesChangedEvent ); }
            virtual ~CChangedRecorder() { UnsubscribeAllFromObserver(); }
            virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) GUCEF_VIRTUAL_OVERRIDE { *flag = true; }
        } recorder( form, &changed );
        ASSERT_FALSE( form->SaveTemplate( CORE::CString::Empty ) );
        ASSERT_TRUE( form->SaveTemplate( "Weekly props" ) && changed );
        CBulkConversionSettings other;
        other.packId = "beta";
        other.validateOnly = true;
        form->SetSettings( other );
        ASSERT_TRUE( form->GetSettings().validateOnly && form->LoadTemplate( "Weekly props" ) && !form->GetSettings().validateOnly && "alpha" == form->GetSettings().packId && 3 == form->GetSettings().assets.size() );
        ASSERT_FALSE( form->LoadTemplate( "Nothing" ) );
        ASSERT_TRUE( form->DeleteTemplate( "Weekly props" ) && !form->LoadTemplate( "Weekly props" ) );
        form->SetSettings( settings );

        ASSERT_TRUE( LintPage( fixture, form, "BulkConversion" ) );
        form->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestReprocessActions( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Reprocess actions: the words, the options of the console and the requests of the service: the asset form with assets, the plan, the caller" )
    try
    {
        CWorkbenchReprocessActions::EReprocessAction action = CWorkbenchReprocessActions::REPROCESS_PLAN;
        ASSERT_TRUE( CWorkbenchReprocessActions::Parse( "intermediate", action ) && CWorkbenchReprocessActions::REPROCESS_INTERMEDIATE == action );
        ASSERT_TRUE( CWorkbenchReprocessActions::Parse( "all", action ) && CWorkbenchReprocessActions::REPROCESS_ALL == action );
        ASSERT_TRUE( CWorkbenchReprocessActions::ParseActionId( "Reprocess:redownload", action ) && CWorkbenchReprocessActions::REPROCESS_REDOWNLOAD == action );
        ASSERT_FALSE( CWorkbenchReprocessActions::Parse( "nonsense", action ) );

        CORE::CString::StringVector words;
        words.push_back( "Content/SM_*.uasset" );
        words.push_back( "--failed" );
        words.push_back( "--kinds" );
        words.push_back( "static-model,skeletal-model" );
        words.push_back( "--targets" );
        words.push_back( "ogre" );
        words.push_back( "--no-cook" );
        words.push_back( "--accept-errors" );
        ASSETSTORE::CAssetReprocessRequest options;
        CORE::CString problem;
        ASSERT_TRUE( CWorkbenchReprocessActions::ParseOptions( words, options, problem ) );
        ASSERT_TRUE( 1 == options.assets.size() && 2 == options.onlyStatuses.size() && 2 == options.kinds.size() && 1 == options.targets.size() && !options.cook && options.acceptErrors );
        ASSETSTORE::CAssetPipelineRequest request;
        CWorkbenchReprocessActions::MakePipelineRequest( CWorkbenchReprocessActions::REPROCESS_INTERMEDIATE, "pk1", options, request );
        ASSERT_TRUE( ASSETSTORE::CAssetPipelineOperations::AssetRegenerateIntermediate == request.operation && "pk1" == request.packId );
        const ASSETSTORE::CAssetReprocessRequest back = ASSETSTORE::CAssetReprocessRequest::FromRequest( request );
        ASSERT_TRUE( "workbench" == back.caller && !back.machine && 1 == back.assets.size() && !back.cook );
        CWorkbenchReprocessActions::MakePipelineRequest( CWorkbenchReprocessActions::REPROCESS_REDOWNLOAD, "pk1", ASSETSTORE::CAssetReprocessRequest(), request );
        ASSERT_TRUE( ASSETSTORE::CAssetPipelineOperations::PackRedownloadOriginal == request.operation );
        CWorkbenchReprocessActions::MakePipelineRequest( CWorkbenchReprocessActions::REPROCESS_PLAN, "pk1", ASSETSTORE::CAssetReprocessRequest(), request );
        ASSERT_TRUE( ASSETSTORE::CAssetPipelineOperations::ReprocessPlan == request.operation );
        words.clear();
        words.push_back( "--steps" );
        ASSERT_FALSE( CWorkbenchReprocessActions::ParseOptions( words, options, problem ) );
        ASSERT_TRUE( problem.HasSubstr( "--steps" ) >= 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
TestRedoInShell( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Redo in the workbench: the menu items and the console commands exist, and each action is a job of the jobs list with its kind" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CJobsFixture fixture( "redo_shell" );
        ASSERT_TRUE( fixture.ready );
        if ( !fixture.ready )
            return;
        CWorkbenchShell& shell = *fixture.shell;
        for ( UInt32 i=0; i<(UInt32) CWorkbenchRedoActions::REDO_COUNT; ++i )
        {
            const CORE::CString actionId = CWorkbenchRedoActions::GetActionId( (CWorkbenchRedoActions::ERedoAction) i );
            ASSERT_TRUE( shell.GetMenuPathOfAction( actionId ).HasSubstr( "Jobs/Re-" ) == 0 );
        }
        ASSERT_TRUE( "Jobs/Jobs overview..." == shell.GetMenuPathOfAction( "Jobs" ) && "Jobs/Bulk conversion..." == shell.GetMenuPathOfAction( "BulkConversion" ) );
        ASSERT_FALSE( shell.RedoPack( CWorkbenchRedoActions::REDO_REIMPORT ) );     // no pack is selected
        ASSERT_TRUE( shell.ExecuteAction( "Jobs" ) && shell.GetJobsForm()->IsOpen() );

        UInt32 expectedJobs = 0;
        const char* const kinds[] = { "index", "analyse", "convert", "cook", "thumbnail" };
        for ( UInt32 i=0; i<(UInt32) CWorkbenchRedoActions::REDO_COUNT; ++i )
        {
            ASSERT_TRUE( shell.RedoStorePack( (CWorkbenchRedoActions::ERedoAction) i, "alpha", CORE::CString::StringVector() ) );
            ++expectedJobs;
        }
        for ( UInt32 i=0; i<400 && fixture.hook.GetRunCount() < expectedJobs; ++i )
            fixture.Frames( 1 );
        ASSERT_TRUE( expectedJobs == fixture.hook.GetRunCount() );
        fixture.Frames( 10 );
        shell.GetJobsForm()->Refresh();
        ASSERT_TRUE( expectedJobs == shell.GetJobsForm()->GetShownJobCount() );
        UInt32 matched = 0;
        for ( UInt32 r=0; r<shell.GetJobsForm()->GetShownJobCount(); ++r )
            for ( UInt32 k=0; k<5; ++k )
                if ( shell.GetJobsForm()->GetCellText( r, CWorkbenchJobsForm::COLUMN_KIND ) == kinds[ k ] )
                    ++matched;
        ASSERT_TRUE( expectedJobs == matched );
        shell.GetJobsForm()->Close();

        // the reprocessing of a pack of the store: the menu items, no pack without a selection, a pack the store does not have is refused
        for ( UInt32 i=0; i<(UInt32) CWorkbenchReprocessActions::REPROCESS_COUNT; ++i )
        {
            const CORE::CString actionId = CWorkbenchReprocessActions::GetActionId( (CWorkbenchReprocessActions::EReprocessAction) i );
            ASSERT_TRUE( 0 == shell.GetMenuPathOfAction( actionId ).HasSubstr( "Jobs/Reprocess pack: " ) );
        }
        CORE::CString message;
        ASSERT_FALSE( shell.ReprocessPack( CWorkbenchReprocessActions::REPROCESS_INTERMEDIATE, CORE::CString::Empty, ASSETSTORE::CAssetReprocessRequest(), message ) );
        ASSERT_TRUE( message.HasSubstr( "There is no pack" ) >= 0 );
        ASSERT_FALSE( shell.ReprocessPack( CWorkbenchReprocessActions::REPROCESS_TARGET, "alpha", ASSETSTORE::CAssetReprocessRequest(), message ) );
        ASSERT_TRUE( message.HasSubstr( "was refused" ) >= 0 );
        ASSERT_FALSE( shell.ExecuteAction( "Reprocess:plan" ) );

    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS: the library sync controller                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A sync job entry as the service gives it */
ASSETSTORE::CAssetJobEntry
MakeSyncJob( const CORE::CString& jobId, const CORE::CString& storeId, bool plan, ASSETSTORE::TAssetJobState state )
{
    ASSETSTORE::CAssetJobEntry job = MakeServiceJob( jobId, "pipeline", state );
    job.subKind = "sync";
    job.providerId = storeId;
    job.scope = storeId + ( plan ? CORE::CString( ", plan" ) : CORE::CString::Empty );
    return job;
}

/** The result of a sync job: the plan with its counts and storage */
ASSETSTORE::CAssetServiceDocument
MakeSyncResult( void )
{
    ASSETSTORE::CAssetLibrarySyncPlan plan;
    plan.storeId = "fab";
    plan.mode = "plan";
    plan.listed = 10;
    plan.listingComplete = true;
    plan.AddCount( "missing", 4 );
    plan.AddCount( "stale", 1 );
    plan.AddCount( "current", 5 );
    plan.queuedItems = 5;
    plan.queuedDownloads = 6;
    plan.estimatedBytes = 3ULL * 1024ULL * 1024ULL * 1024ULL;
    plan.storageFreeBytes = 500ULL * 1024ULL * 1024ULL * 1024ULL;
    plan.storageVerdict = "fits";
    ASSETSTORE::CAssetServiceDocument result;
    result.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
    result.document.SetName( "result" );
    result.document.SetNodeType( GUCEF_DATATYPE_OBJECT );
    CORE::CDataNodeSerializableSettings settings;
    plan.Serialize( *result.document.AddChild( "plan", GUCEF_DATATYPE_OBJECT ), settings );
    return result;
}

/** Counts the events of the controller */
class CSyncControllerRecorder : public CORE::CObserver
{
    public:
    UInt32 plans;
    UInt32 notices;
    UInt32 started;
    explicit CSyncControllerRecorder( CORE::CNotifier* notifier ) : CORE::CObserver(), plans( 0 ), notices( 0 ), started( 0 )
    {
        SubscribeTo( notifier, CLibrarySyncController::PlanReadyEvent );
        SubscribeTo( notifier, CLibrarySyncController::NotificationEvent );
        SubscribeTo( notifier, CLibrarySyncController::JobStartedEvent );
    }
    virtual ~CSyncControllerRecorder() { UnsubscribeAllFromObserver(); }
    virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& id, CORE::CICloneable* ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CLibrarySyncController::PlanReadyEvent == id ) ++plans;
        if ( CLibrarySyncController::NotificationEvent == id ) ++notices;
        if ( CLibrarySyncController::JobStartedEvent == id ) ++started;
    }
};

/** The spec of the last start of a pipeline job the fake service was asked for */
bool
LastSyncSpec( ASSETSTORETEST::CFakeAssetStoreService& service, ASSETSTORE::CAssetLibrarySyncSpec& spec )
{
    ASSETSTORE::CAssetServiceCall call;
    ASSETSTORE::CAssetPipelineRequest request;
    if ( !service.GetLastCall( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, call ) || !call.GetRecord( "request", request ) ||
         ASSETSTORE::CAssetPipelineOperations::SyncLibrary != request.operation )
        return false;
    spec = ASSETSTORE::CAssetLibrarySyncSpec::FromRequest( request );
    return true;
}

}

/*-------------------------------------------------------------------------*/

static void
TestLibrarySyncController( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Library sync controller: the stores with their sign in, a plan as a dry run job with its counts, a sync started at once with its progress from the job events, the notice at the end, cancel and retry; a refusal says why" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        service.SetInProcess( true );
        ASSETSTORE::CAssetStoreInfoPage stores;
        ASSETSTORE::CAssetStoreInfo fab;
        fab.storeId = "fab";
        fab.displayName = "Fab";
        fab.signedIn = true;
        stores.entries.push_back( fab );
        ASSETSTORE::CAssetStoreInfo humble;
        humble.storeId = "humble";
        humble.displayName = "Humble";
        stores.entries.push_back( humble );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListStores, ASSETSTORE::ASSETSTORE_STATUS_OK, &stores );
        ASSETSTORE::CAssetJobPage jobs;
        ASSETSTORE::CAssetJobEntry running = MakeSyncJob( "pipeline:pj3", "humble", false, ASSETSTORE::ASSETJOB_STATE_RUNNING );
        running.itemsDone = 1;
        running.itemsTotal = 4;
        jobs.entries.push_back( running );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListJobs, ASSETSTORE::ASSETSTORE_STATUS_OK, &jobs );

        CLibrarySyncController controller( service );
        CSyncControllerRecorder recorder( &controller );
        ASSERT_TRUE( controller.RefreshStores() && 2 == controller.GetStoreCount() );
        const CLibrarySyncController::SStoreState* fabState = controller.FindStore( "fab" );
        const CLibrarySyncController::SStoreState* humbleState = controller.FindStore( "humble" );
        ASSERT_TRUE( GUCEF_NULL != fabState && GUCEF_NULL != humbleState && fabState->signedIn && !humbleState->signedIn && "Fab" == fabState->displayName );
        // a sync that runs on the service is followed
        ASSERT_TRUE( humbleState->running && "pipeline:pj3" == humbleState->runJobId && 1 == humbleState->itemsDone && 4 == humbleState->itemsTotal );

        // the plan: a dry run job of the store with the profile of the store; its counts come with the result when it ends
        ASSETSTORE::CAssetServiceResult planStarted;
        planStarted.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
        planStarted.id = "pipeline:pj7";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &planStarted );
        const ASSETSTORE::CAssetServiceDocument planResult = MakeSyncResult();
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::GetJobResult, ASSETSTORE::ASSETSTORE_STATUS_OK, &planResult );
        ASSERT_TRUE( controller.RequestPlan( "fab" ) && fabState->planning && "pipeline:pj7" == fabState->planJobId && 1 == recorder.started );
        ASSETSTORE::CAssetLibrarySyncSpec spec;
        ASSERT_TRUE( LastSyncSpec( service, spec ) && spec.IsPlanOnly() && spec.useStoreProfile && "fab" == spec.storeId && spec.itemIds.empty() && !spec.manual );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, MakeSyncJob( "pipeline:pj7", "fab", true, ASSETSTORE::ASSETJOB_STATE_DONE ) );
        ASSERT_TRUE( !fabState->planning && fabState->planned && 1 == recorder.plans );
        ASSERT_TRUE( 4 == fabState->missing && 1 == fabState->stale && 5 == fabState->current && 0 == fabState->unavailable && 6 == fabState->queuedDownloads );
        ASSERT_TRUE( "3.0 GiB" == CLibrarySyncController::FormatBytes( fabState->estimatedBytes ) && "500.0 GiB" == CLibrarySyncController::FormatBytes( fabState->freeBytes ) && "fits" == fabState->storageVerdict );

        // the sync: started at once, followed by the job events, the notice at the end
        ASSETSTORE::CAssetServiceResult runStarted;
        runStarted.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
        runStarted.id = "pipeline:pj8";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &runStarted );
        ASSERT_TRUE( controller.StartSync( "fab" ) && fabState->running && "pipeline:pj8" == fabState->runJobId );
        ASSERT_TRUE( LastSyncSpec( service, spec ) && !spec.IsPlanOnly() && spec.useStoreProfile );
        ASSETSTORE::CAssetJobEntry progress = MakeSyncJob( "pipeline:pj8", "fab", false, ASSETSTORE::ASSETJOB_STATE_RUNNING );
        progress.itemsDone = 2;
        progress.itemsTotal = 6;
        progress.fraction = 2.0f / 6.0f;
        progress.stateText = "3 of 6 queued, 1 in flight; last queued: Iron Sword";
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, progress );
        ASSERT_TRUE( 2 == fabState->itemsDone && 6 == fabState->itemsTotal && fabState->progressText.HasSubstr( "Iron Sword" ) > 0 );
        ASSETSTORE::CAssetServiceResult controlled;
        controlled.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
        controlled.id = "pipeline:pj8";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ControlJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &controlled );
        ASSERT_TRUE( controller.Cancel( "fab" ) );
        ASSETSTORE::CAssetServiceCall call;
        ASSERT_TRUE( service.GetLastCall( ASSETSTORE::CAssetServiceOperations::ControlJob, call ) && "cancel" == call.GetText( "action" ) && "pipeline:pj8" == call.GetText( "jobId" ) );
        ASSETSTORE::CAssetJobEntry ended = MakeSyncJob( "pipeline:pj8", "fab", false, ASSETSTORE::ASSETJOB_STATE_DONE );
        ended.itemsDone = 6;
        ended.itemsTotal = 6;
        ended.SetActionAvailable( ASSETSTORE::ASSETJOB_ACTION_RETRY, true );
        ended.resultText = "4 missing, 1 stale; 5 ingested, 0 revalidated, 1 failed";
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, ended );
        ASSERT_TRUE( !fabState->running && !fabState->planned && fabState->retryAvailable && fabState->lastResult.HasSubstr( "5 ingested" ) > 0 );
        ASSERT_TRUE( 1 == recorder.notices && 1 == controller.GetNotifications().size() && controller.GetNotifications()[ 0 ].HasSubstr( "Fab" ) > 0 );

        // retry of the failed items of the run, a download of a selection
        ASSERT_TRUE( controller.RetryFailed( "fab" ) && service.GetLastCall( ASSETSTORE::CAssetServiceOperations::ControlJob, call ) && "retry" == call.GetText( "action" ) );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, ended );
        CORE::CString::StringVector items;
        items.push_back( "item-1" );
        items.push_back( "item-2" );
        ASSERT_TRUE( controller.DownloadItems( "fab", items ) && LastSyncSpec( service, spec ) && 2 == spec.itemIds.size() && "item-2" == spec.itemIds[ 1 ] && !spec.IsPlanOnly() && spec.manual );
        ASSERT_FALSE( controller.DownloadItems( "fab", CORE::CString::StringVector() ) );

        // a start the service refuses says why on the row
        ASSETSTORE::CAssetServiceResult refused;
        refused.status = ASSETSTORE::ASSETSTORE_STATUS_REFUSED;
        refused.message = "a run of the library sync downloads and is started by the devices with the owner role only";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, ASSETSTORE::ASSETSTORE_STATUS_REFUSED, &refused );
        ASSERT_FALSE( controller.StartSync( "humble" ) );
        ASSERT_TRUE( humbleState->problem.HasSubstr( "owner role" ) > 0 && controller.GetProblem() == humbleState->problem );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Library sync controller: against a service that is not in process the stores, the start and the plan are asked and answered later; a sync that runs already is followed" )
    try
    {
        ASSETSTORETEST::CFakeAssetStoreService service;
        service.SetInProcess( false );
        service.SetDeferAsync( true );
        ASSETSTORE::CAssetStoreInfoPage stores;
        ASSETSTORE::CAssetStoreInfo fab;
        fab.storeId = "fab";
        fab.displayName = "Fab";
        fab.signedIn = true;
        stores.entries.push_back( fab );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListStores, ASSETSTORE::ASSETSTORE_STATUS_OK, &stores );
        ASSETSTORE::CAssetJobPage jobs;
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListJobs, ASSETSTORE::ASSETSTORE_STATUS_OK, &jobs );

        CLibrarySyncController controller( service );
        ASSERT_TRUE( controller.RefreshStores() && 0 == controller.GetStoreCount() && controller.IsWaiting() );
        service.CompletePendingCalls();
        controller.Update();
        ASSERT_TRUE( 1 == controller.GetStoreCount() && !controller.IsWaiting() );

        ASSETSTORE::CAssetServiceResult planStarted;
        planStarted.status = ASSETSTORE::ASSETSTORE_STATUS_OK;
        planStarted.id = "pipeline:pj21";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, ASSETSTORE::ASSETSTORE_STATUS_OK, &planStarted );
        const ASSETSTORE::CAssetServiceDocument planResult = MakeSyncResult();
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::GetJobResult, ASSETSTORE::ASSETSTORE_STATUS_OK, &planResult );
        ASSERT_TRUE( controller.RequestPlan( "fab" ) && controller.FindStore( "fab" )->planJobId.IsNULLOrEmpty() );
        service.CompletePendingCalls();
        controller.Update();
        ASSERT_TRUE( "pipeline:pj21" == controller.FindStore( "fab" )->planJobId );
        service.Raise( ASSETSTORE::CIAssetStoreService::JobChangedEvent, MakeSyncJob( "pipeline:pj21", "fab", true, ASSETSTORE::ASSETJOB_STATE_DONE ) );
        ASSERT_TRUE( !controller.FindStore( "fab" )->planned && controller.IsWaiting() );
        service.CompletePendingCalls();
        controller.Update();
        ASSERT_TRUE( controller.FindStore( "fab" )->planned && 4 == controller.FindStore( "fab" )->missing );

        // the service says a sync of the store runs already: that one is followed
        ASSETSTORE::CAssetServiceResult already;
        already.status = ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS;
        already.id = "pipeline:pj20";
        already.message = "A sync of the store fab runs already: pipeline:pj20";
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::StartPipelineJob, ASSETSTORE::ASSETSTORE_STATUS_ALREADY_EXISTS, &already );
        ASSETSTORE::CAssetJobPage runningJobs;
        runningJobs.entries.push_back( MakeSyncJob( "pipeline:pj20", "fab", false, ASSETSTORE::ASSETJOB_STATE_RUNNING ) );
        service.SetAnswer( ASSETSTORE::CAssetServiceOperations::ListJobs, ASSETSTORE::ASSETSTORE_STATUS_OK, &runningJobs );
        ASSERT_TRUE( controller.StartSync( "fab" ) );
        for ( UInt32 round=0; round<3; ++round )
        {
            service.CompletePendingCalls();
            controller.Update();
        }
        ASSERT_TRUE( controller.FindStore( "fab" )->running && "pipeline:pj20" == controller.FindStore( "fab" )->runJobId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchJobTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchJobs" );

    TestRegistryAndFilters();
    TestListFromService();
    TestCancelResumeRetry();
    TestEveryLongOperationIsAJob();
    TestBulkSettings();
    TestBulkPlanAndRun();
    TestLibrarySyncController();
    TestRedoActions();
    TestJobsPage();
    TestBulkPanel();
    TestReprocessActions();
    TestRedoInShell();
}
