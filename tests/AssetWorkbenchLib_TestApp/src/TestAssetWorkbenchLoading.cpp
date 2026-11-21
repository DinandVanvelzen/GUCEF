/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#include <string.h>

#include <Ogre.h>

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#include "AssetWorkbenchLib.h"
#include "AssetWorkbenchLib_CAssetLoader.h"

#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchAssets.h"
#include "TestAssetWorkbenchLoading.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::ASSETWORKBENCH;

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

UInt64
NowInMs( void )
{
    return MT::PrecisionTickCount() * 1000 / MT::PrecisionTimerResolution();
}

/** A job that takes its time: the preparation sleeps in small steps and watches for cancellation; the pieces are counted */
class CFakeSlowJob : public CIAssetLoadJob
{
    public:

    UInt32 prepareInMs;
    UInt32 pieces;
    UInt32 pieceInMs;
    bool progress;
    volatile bool prepareStarted;
    volatile bool prepareFinished;
    volatile bool sawCancel;
    TThreadId prepareThread;
    UInt32 stepsMade;
    bool discarded;

    CFakeSlowJob( UInt32 prepareTime, UInt32 pieceCount = 2, UInt32 pieceTime = 0, bool reportProgress = true )
        : CIAssetLoadJob(), prepareInMs( prepareTime ), pieces( pieceCount ), pieceInMs( pieceTime ), progress( reportProgress )
        , prepareStarted( false ), prepareFinished( false ), sawCancel( false ), prepareThread( 0 ), stepsMade( 0 ), discarded( false )
    {}

    virtual bool ReportsProgress( void ) const { return progress; }

    virtual bool Prepare( CAssetLoadControl& control )
    {
        prepareThread = MT::GetCurrentThreadID();
        prepareStarted = true;
        for ( UInt32 waited=0; waited<prepareInMs; waited+=10 )
        {
            if ( control.IsCancelled() )
            {
                sawCancel = true;
                prepareFinished = true;
                return false;
            }
            MT::PrecisionDelay( 10 );
            control.SetProgress( (Float32) waited / (Float32) prepareInMs );
        }
        prepareFinished = true;
        return true;
    }

    virtual EStepResult Step( void )
    {
        ++stepsMade;
        if ( pieceInMs > 0 )
            MT::PrecisionDelay( pieceInMs );
        return stepsMade >= pieces ? STEP_DONE : STEP_MORE;
    }

    virtual void Discard( void ) { discarded = true; }
};

class CFinishedRecorder : public CORE::CObserver
{
    public:

    UInt32 count;
    UInt32 lastId;
    GUCEF::vector< UInt32 > outcomes;
    CAssetLoader* loader;

    explicit CFinishedRecorder( CAssetLoader& source )
        : CORE::CObserver(), count( 0 ), lastId( 0 ), outcomes(), loader( &source )
    {
        typedef CORE::CTEventHandlerFunctor< CFinishedRecorder > TCallback;
        TCallback callback( this, &CFinishedRecorder::OnFinished );
        SubscribeTo( &source, CAssetLoader::LoadFinishedEvent, callback );
    }

    virtual ~CFinishedRecorder() { UnsubscribeAllFromObserver(); }

    void OnFinished( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* data )
    {
        ++count;
        const CORE::TCloneableUInt32* id = static_cast< const CORE::TCloneableUInt32* >( data );
        lastId = GUCEF_NULL != id ? id->GetData() : 0;
        outcomes.push_back( (UInt32) loader->GetLastOutcome() );
    }
};

/** The workbench in a window of the tests with a folder of assets mounted */
class CLoadingFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    GUIDRIVEROGRE::COgreWindowContext* window;
    CAssetMounter mounter;
    CAssetCapture* capture;
    CWorkbenchShell* shell;
    CORE::CString dir;
    bool ready;
    UInt32 maxFrameInMs;

    explicit CLoadingFixture( const CORE::CString& name )
        : contextPtr(), context( GUCEF_NULL ), window( GUCEF_NULL ), mounter(), capture( GUCEF_NULL ), shell( GUCEF_NULL ), dir(), ready( false ), maxFrameInMs( 0 )
    {
        if ( GetMainTestWindow().IsNULL() )
            return;
        window = AsOgreWindow( GetMainTestWindow() );
        dir = MakeCleanTestDir( name );
        const bool written = WriteTestFile( dir + "/model.gltf", BuildTestGltf() ) && WriteTestPng( dir + "/checker.png", 8, 8 ) &&
                             WriteTestPng( dir + "/pic.png", 8, 4 ) && WriteTestFile( dir + "/broken.png", "this is not a picture" );
        if ( !written || !mounter.Mount( "avload", dir ) )
            return;
        contextPtr = CreateWorkbenchContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        capture = GUCEF_NEW CAssetCapture( window );
        if ( !capture->Initialize( 64 ) )
            return;
        shell = GUCEF_NEW CWorkbenchShell( mounter );
        ready = shell->Initialize( *context, capture, MakeCleanTestDir( name + "_cache" ) );
    }

    ~CLoadingFixture()
    {
        if ( GUCEF_NULL != shell )
        {
            shell->Shutdown();
            GUCEF_DELETE shell;
        }
        contextPtr.Unlink();
        if ( GUCEF_NULL != capture )
        {
            capture->Shutdown();
            GUCEF_DELETE capture;
        }
        mounter.UnmountAll();
    }

    /** One frame, as the application does it; the time it took is kept when it is the longest so far */
    void Frame( void )
    {
        const UInt64 start = NowInMs();
        shell->Update( 5 );
        window->RenderFrame( GUCEF_NULL );
        const UInt32 took = (UInt32) ( NowInMs() - start );
        if ( took > maxFrameInMs )
            maxFrameInMs = took;
    }

    /** Frames, one every few milliseconds like a window that waits for the display, until the condition holds or the time is up; the time it took */
    template< typename TCondition >
    UInt32 FramesUntil( TCondition condition, UInt32 timeoutInMs )
    {
        const UInt64 start = NowInMs();
        while ( NowInMs() - start < timeoutInMs )
        {
            Frame();
            if ( condition() )
                break;
            MT::PrecisionDelay( 4 );
        }
        return (UInt32) ( NowInMs() - start );
    }
};

struct SNotOpening
{
    CWorkbenchShell* shell;
    bool operator()( void ) const { return !shell->IsOpening(); }
};

struct SIndicatorShown
{
    CWorkbenchShell* shell;
    bool operator()( void ) const { return shell->GetActivityIndicator()->IsIndicatorShown(); }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAssetWorkbenchLoadingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbench.Loading" );

    GUCEF_TESTFW_TESTCASE( "Loading: The preparation runs on another thread than the caller and the pieces on the caller; Run() does both on the caller; a failed preparation is a failure" )
    try
    {
        CAssetLoader loader( GUCEF_NULL );
        CFinishedRecorder finished( loader );
        CFakeSlowJob* raw = GUCEF_NEW CFakeSlowJob( 100, 3 );
        TAssetLoadJobPtr job( raw );
        CORE::CFutureResult future = loader.Begin( job, "Fake" );
        ASSERT_TRUE( future.HasAFuture() && loader.IsBusy() );
        UInt32 guard = 0;
        while ( loader.IsBusy() && guard++ < 1000 )
        {
            loader.Update();
            MT::PrecisionDelay( 5 );
        }
        ASSERT_TRUE( !loader.IsBusy() && CAssetLoader::OUTCOME_SUCCEEDED == loader.GetLastOutcome() && 1 == finished.count );
        ASSERT_TRUE( raw->prepareThread != MT::GetCurrentThreadID() );
        ASSERT_TRUE( 3 == raw->stepsMade && 3 == loader.GetLastStepCount() && !raw->discarded );

        CFakeSlowJob* local = GUCEF_NEW CFakeSlowJob( 30, 2 );
        TAssetLoadJobPtr localJob( local );
        ASSERT_TRUE( loader.Run( localJob, "Fake, here" ) );
        ASSERT_TRUE( local->prepareThread == MT::GetCurrentThreadID() && 2 == local->stepsMade );

        // a job that cannot prepare: the problem is the job's, nothing is stepped, the job is let go of
        class CFailingJob : public CFakeSlowJob
        {
            public:
            CFailingJob( void ) : CFakeSlowJob( 10, 2 ) {}
            virtual bool Prepare( CAssetLoadControl& control ) { CFakeSlowJob::Prepare( control ); return false; }
            virtual CORE::CString GetProblem( void ) const { return "No such file"; }
        };
        CFailingJob* failing = GUCEF_NEW CFailingJob();
        TAssetLoadJobPtr failingJob( failing );
        loader.Begin( failingJob, "Failing" );
        guard = 0;
        while ( loader.IsBusy() && guard++ < 1000 )
        {
            loader.Update();
            MT::PrecisionDelay( 5 );
        }
        ASSERT_TRUE( CAssetLoader::OUTCOME_FAILED == loader.GetLastOutcome() && "No such file" == loader.GetLastProblem() );
        ASSERT_TRUE( 0 == failing->stepsMade && failing->discarded );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Loading: The pieces of the renderer keep to the budget of a frame: one piece per frame when a piece takes the budget, several when they are small" )
    try
    {
        CAssetLoader loader( GUCEF_NULL );
        CFakeSlowJob* big = GUCEF_NEW CFakeSlowJob( 10, 6, 15 );
        TAssetLoadJobPtr bigJob( big );
        loader.Begin( bigJob, "Big pieces" );
        UInt32 updates = 0;
        while ( loader.IsBusy() && updates < 2000 )
        {
            const UInt32 before = big->stepsMade;
            loader.Update( 8 );
            if ( big->stepsMade > before )
                ASSERT_TRUE( 1 == big->stepsMade - before );
            ++updates;
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == loader.GetLastOutcome() && 6 == big->stepsMade );

        CFakeSlowJob* small = GUCEF_NEW CFakeSlowJob( 10, 6, 0 );
        TAssetLoadJobPtr smallJob( small );
        loader.Begin( smallJob, "Small pieces" );
        updates = 0;
        UInt32 stepsInOneUpdate = 0;
        while ( loader.IsBusy() && updates++ < 2000 )
        {
            const UInt32 before = small->stepsMade;
            loader.Update( 50 );
            if ( small->stepsMade - before > stepsInOneUpdate )
                stepsInOneUpdate = small->stepsMade - before;
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == loader.GetLastOutcome() && stepsInOneUpdate > 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Loading: A job that takes 2 s keeps the frames under 100 ms, the indicator is there within 300 ms, the progress shows and the result is made when it is ready" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CLoadingFixture fixture( "loading_slow" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        for ( UInt32 i=0; i<6; ++i )
            fixture.Frame();
        fixture.maxFrameInMs = 0;
        CFinishedRecorder finished( shell.GetLoader() );

        CFakeSlowJob* raw = GUCEF_NEW CFakeSlowJob( 2000, 2 );
        TAssetLoadJobPtr job( raw );
        const UInt64 begun = NowInMs();
        ASSERT_TRUE( shell.GetLoader().Begin( job, "Opening slow.fbx" ).HasAFuture() );
        ASSERT_TRUE( shell.IsOpening() );
        SIndicatorShown shown = { &shell };
        const UInt32 shownAfter = fixture.FramesUntil( shown, 1000 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Loading: The indicator was there after " << shownAfter << " ms" );
        ASSERT_TRUE( shell.GetActivityIndicator()->IsIndicatorShown() );
        ASSERT_TRUE( shownAfter >= 290 && shownAfter <= 360 );
        ASSERT_TRUE( "Opening slow.fbx" == shell.GetActivityIndicator()->GetWidget( "Text" )->GetProperty( GUI::EWidgetProperty_Text ) );
        ASSERT_TRUE( shell.GetActivityIndicator()->GetWidget( "Cancel" )->IsVisible() );
        ASSERT_TRUE( GUI::CURSORSHAPE_PROGRESS == fixture.context->GetSystemCursorShape() );

        // later the bar has moved on: determinate, the progress of the job
        SNotOpening idle = { &shell };
        fixture.FramesUntil( idle, 1000 );
        GUI::CProgressBar* bar = shell.GetActivityIndicator()->GetWidgetAs< GUI::CProgressBar >( "Bar" );
        ASSERT_TRUE( GUCEF_NULL != bar && bar->GetFraction() > 0.2f && bar->GetFraction() < 0.99f );
        fixture.FramesUntil( idle, 5000 );
        const UInt32 total = (UInt32) ( NowInMs() - begun );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Loading: Done after " << total << " ms, the longest frame took " << fixture.maxFrameInMs << " ms" );
        ASSERT_TRUE( !shell.IsOpening() && CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() && 1 == finished.count );
        ASSERT_TRUE( total >= 1990 && 2 == raw->stepsMade );
        // the interface kept drawing: no frame took longer than the budget of the rule
        ASSERT_TRUE( fixture.maxFrameInMs < 100 );
        fixture.Frame();
        fixture.Frame();
        ASSERT_FALSE( shell.GetActivityIndicator()->IsIndicatorShown() );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == fixture.context->GetSystemCursorShape() );
        #ifdef GUCEF_DEBUG_MODE
        ASSERT_TRUE( 0 == shell.GetActivityService().GetStallCount() );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Loading: A result that arrives after something else was opened is dropped: the old job never makes anything, the new one does" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CLoadingFixture fixture( "loading_stale" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frame();
        CFinishedRecorder finished( shell.GetLoader() );

        CFakeSlowJob* oldJob = GUCEF_NEW CFakeSlowJob( 700, 2 );
        CFakeSlowJob* newJob = GUCEF_NEW CFakeSlowJob( 60, 2 );
        TAssetLoadJobPtr oldPtr( oldJob ), newPtr( newJob );
        shell.GetLoader().Begin( oldPtr, "Old" );
        const UInt32 oldRequest = shell.GetLoader().GetCurrentRequest();
        for ( UInt32 i=0; i<10; ++i )
        {
            fixture.Frame();
            MT::PrecisionDelay( 10 );
        }
        shell.GetLoader().Begin( newPtr, "New" );
        ASSERT_TRUE( shell.GetLoader().GetCurrentRequest() != oldRequest );
        SNotOpening idle = { &shell };
        fixture.FramesUntil( idle, 3000 );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() && 2 == newJob->stepsMade );
        // the old one is told to stop and let go of when it has: nothing of it is stepped
        UInt32 guard = 0;
        while ( 0 == shell.GetLoader().GetDroppedCount() && guard++ < 200 )
        {
            fixture.Frame();
            MT::PrecisionDelay( 10 );
        }
        ASSERT_TRUE( 1 == shell.GetLoader().GetDroppedCount() );
        ASSERT_TRUE( 0 == oldJob->stepsMade && oldJob->discarded && oldJob->sawCancel );
        // the old request ended as cancelled when the new one began, the new one succeeded
        ASSERT_TRUE( 2 == finished.count && CAssetLoader::OUTCOME_CANCELLED == finished.outcomes[ 0 ] && CAssetLoader::OUTCOME_SUCCEEDED == finished.outcomes[ 1 ] );
        ASSERT_TRUE( 0 == shell.GetActivityService().GetActiveCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Loading: The cancel button of the indicator stops the job in the pool, nothing is made, and the indicator and the cursor go away" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CLoadingFixture fixture( "loading_cancel" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frame();
        CFinishedRecorder finished( shell.GetLoader() );

        CFakeSlowJob* raw = GUCEF_NEW CFakeSlowJob( 2000, 2 );
        TAssetLoadJobPtr job( raw );
        shell.GetLoader().Begin( job, "Opening cancel.fbx" );
        SIndicatorShown shown = { &shell };
        fixture.FramesUntil( shown, 1000 );
        ASSERT_TRUE( shell.GetActivityIndicator()->IsIndicatorShown() );
        const UInt64 clicked = NowInMs();

        {
            GUI::CWidget* cancel = shell.GetActivityIndicator()->GetWidget( "Cancel" );
            ASSERT_TRUE( GUCEF_NULL != cancel && cancel->IsVisible() );
            shell.GetActivityService().RequestCancel( shell.GetLoader().GetCurrentActivity() );
        }
        fixture.Frame();
        fixture.Frame();
        ASSERT_FALSE( shell.IsOpening() );
        ASSERT_TRUE( 1 == finished.count && CAssetLoader::OUTCOME_CANCELLED == shell.GetLoader().GetLastOutcome() );
        // the preparation sees it within a few of its small steps
        UInt32 guard = 0;
        while ( !raw->prepareFinished && guard++ < 100 )
            MT::PrecisionDelay( 10 );
        ASSERT_TRUE( raw->prepareFinished && raw->sawCancel && NowInMs() - clicked < 600 );
        guard = 0;
        while ( !raw->discarded && guard++ < 100 )
        {
            fixture.Frame();
            MT::PrecisionDelay( 10 );
        }
        ASSERT_TRUE( raw->discarded && 0 == raw->stepsMade );
        fixture.Frame();
        ASSERT_FALSE( shell.GetActivityIndicator()->IsIndicatorShown() );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == fixture.context->GetSystemCursorShape() );
        // the workbench opens the next thing as if nothing happened
        CFakeSlowJob* next = GUCEF_NEW CFakeSlowJob( 20, 1 );
        TAssetLoadJobPtr nextJob( next );
        shell.GetLoader().Begin( nextJob, "Next" );
        SNotOpening idle = { &shell };
        fixture.FramesUntil( idle, 2000 );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() && 1 == next->stepsMade );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Loading: A model and a picture opened in the background give the same views as when they are opened on the calling thread, the pieces are spread over frames, and a picture that cannot be read is reported with the previous asset kept" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CLoadingFixture fixture( "loading_real" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frame();
        SNotOpening idle = { &shell };

        // the model: on the calling thread first
        ASSERT_TRUE( shell.OpenAsset( "/avload/model.gltf" ) );
        const SMeshStatistics syncStats = shell.GetModelView().GetStatistics();
        const CORE::CString syncVertices = shell.GetInspector()->GetValue( "Statistics@Vertices" );
        const CORE::CString syncTriangles = shell.GetInspector()->GetValue( "Statistics@Triangles" );
        COrbitController::SVector3 syncCenter;
        Float32 syncRadius = 0.0f;
        shell.GetModelView().GetBoundingSphere( syncCenter, syncRadius );
        const size_t syncClips = shell.GetTimeline()->GetClips().size();
        shell.CloseAsset();
        ASSERT_TRUE( ASSETKIND_OTHER == shell.GetShownKind() );

        // then in the background
        ASSERT_TRUE( shell.OpenAssetAsync( "/avload/model.gltf" ).HasAFuture() );
        ASSERT_TRUE( shell.IsOpening() );
        fixture.FramesUntil( idle, 10000 );
        ASSERT_TRUE( !shell.IsOpening() && CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( shell.GetLoader().GetLastStepCount() >= 2 );
        ASSERT_TRUE( ASSETKIND_MODEL == shell.GetShownKind() && "avload/model.gltf" == shell.GetShownPath() && shell.GetModelView().IsLoaded() );
        const SMeshStatistics& asyncStats = shell.GetModelView().GetStatistics();
        ASSERT_TRUE( syncStats.vertexCount == asyncStats.vertexCount && syncStats.triangleCount == asyncStats.triangleCount );
        ASSERT_TRUE( syncStats.subMeshCount == asyncStats.subMeshCount && syncStats.boneCount == asyncStats.boneCount && syncStats.materialCount == asyncStats.materialCount );
        ASSERT_TRUE( syncVertices == shell.GetInspector()->GetValue( "Statistics@Vertices" ) && syncTriangles == shell.GetInspector()->GetValue( "Statistics@Triangles" ) );
        COrbitController::SVector3 asyncCenter;
        Float32 asyncRadius = 0.0f;
        shell.GetModelView().GetBoundingSphere( asyncCenter, asyncRadius );
        ASSERT_TRUE( fabsf( syncRadius - asyncRadius ) < 0.0001f && fabsf( syncCenter.x - asyncCenter.x ) < 0.0001f && fabsf( syncCenter.y - asyncCenter.y ) < 0.0001f );
        ASSERT_TRUE( syncClips == shell.GetTimeline()->GetClips().size() && 0 == shell.GetCenterTab() );
        ASSERT_TRUE( asyncStats.vertexCount == TestGltfVertexCount );
        // the model is drawn
        for ( UInt32 i=0; i<4; ++i )
            fixture.Frame();
        IMAGE::TPixelMapPtr stagePixels;
        ASSERT_TRUE( shell.GetStage().ReadPixels( stagePixels ) && !stagePixels.IsNULL() );

        // the picture
        ASSERT_TRUE( shell.OpenAsset( "/avload/pic.png" ) );
        const CTextureView::SInfo syncInfo = shell.GetTextureView().GetInfo();
        shell.CloseAsset();
        ASSERT_TRUE( shell.OpenAssetAsync( "/avload/pic.png" ).HasAFuture() );
        fixture.FramesUntil( idle, 10000 );
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() );
        ASSERT_TRUE( ASSETKIND_IMAGE == shell.GetShownKind() && shell.GetTextureView().IsLoaded() );
        const CTextureView::SInfo& asyncInfo = shell.GetTextureView().GetInfo();
        ASSERT_TRUE( syncInfo.width == asyncInfo.width && syncInfo.height == asyncInfo.height && syncInfo.formatName == asyncInfo.formatName );
        ASSERT_TRUE( 8 == asyncInfo.width && 4 == asyncInfo.height && 1 == shell.GetCenterTab() );
        ASSERT_TRUE( GUCEF_NULL != shell.GetImageView()->GetImage() );

        // a picture that cannot be decoded: reported, and the picture that was shown stays
        const UInt32 entriesBefore = shell.GetReport()->GetEntryCount();
        ASSERT_TRUE( shell.OpenAssetAsync( "/avload/broken.png" ).HasAFuture() );
        fixture.FramesUntil( idle, 10000 );
        ASSERT_TRUE( CAssetLoader::OUTCOME_FAILED == shell.GetLoader().GetLastOutcome() );
        // the report takes what the log has at the next frames
        fixture.Frame();
        fixture.Frame();
        ASSERT_TRUE( ASSETKIND_IMAGE == shell.GetShownKind() && "avload/pic.png" == shell.GetShownPath() );
        ASSERT_TRUE( shell.GetReport()->GetEntryCount() > entriesBefore );
        ASSERT_TRUE( 0 == shell.GetActivityService().GetActiveCount() );

        // what is not an asset is refused at once
        ASSERT_TRUE( shell.OpenAssetAsync( "/avload/nothing.txt" ).HasNoFuture() && !shell.IsOpening() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Loading: a model whose texture is present opens without a warning or an error, on the calling thread and in the background; a model whose texture is missing reports it" )
    try
    {
        if ( GetMainTestWindow().IsNULL() )
            GUCEF_TESTFW_SKIP( GetNoWindowReason().C_String() );
        CLoadingFixture fixture( "loading_clean" );
        ASSERT_TRUE( fixture.ready );
        CWorkbenchShell& shell = *fixture.shell;
        fixture.Frame();
        SNotOpening idle = { &shell };
        GUI::CReportViewForm* report = shell.GetReport();
        const UInt32 errorsBefore = report->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE );
        const UInt32 warningsBefore = report->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING );

        // the texture is in the folder of the model: nothing to say
        ASSERT_TRUE( shell.OpenAsset( "/avload/model.gltf" ) );
        for ( UInt32 i=0; i<4; ++i )
            fixture.Frame();
        ASSERT_TRUE( errorsBefore == report->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE ) && warningsBefore == report->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING ) );
        shell.CloseAsset();
        ASSERT_TRUE( shell.OpenAssetAsync( "/avload/model.gltf" ).HasAFuture() );
        fixture.FramesUntil( idle, 10000 );
        for ( UInt32 i=0; i<4; ++i )
            fixture.Frame();
        ASSERT_TRUE( CAssetLoader::OUTCOME_SUCCEEDED == shell.GetLoader().GetLastOutcome() && ASSETKIND_MODEL == shell.GetShownKind() );
        ASSERT_TRUE( errorsBefore == report->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE ) && warningsBefore == report->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING ) );

        // the texture is not there: that is said, in both ways of opening
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/nomap/model.gltf", BuildTestGltf() ) && fixture.mounter.Mount( "avnomap", fixture.dir + "/nomap" ) );
        shell.CloseAsset();
        ASSERT_TRUE( shell.OpenAsset( "/avnomap/model.gltf" ) );
        for ( UInt32 i=0; i<4; ++i )
            fixture.Frame();
        const UInt32 problemsSync = report->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE ) + report->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING );
        ASSERT_TRUE( problemsSync > errorsBefore + warningsBefore );
        shell.CloseAsset();
        // another folder: what Ogre has loaded already is not reported again
        ASSERT_TRUE( WriteTestFile( fixture.dir + "/nomap2/model.gltf", BuildTestGltf() ) && fixture.mounter.Mount( "avnomap2", fixture.dir + "/nomap2" ) );
        ASSERT_TRUE( shell.OpenAssetAsync( "/avnomap2/model.gltf" ).HasAFuture() );
        fixture.FramesUntil( idle, 10000 );
        for ( UInt32 i=0; i<4; ++i )
            fixture.Frame();
        ASSERT_TRUE( report->GetEntryCount( GUI::CReportViewForm::SEVERITY_FAILURE ) + report->GetEntryCount( GUI::CReportViewForm::SEVERITY_WARNING ) > problemsSync );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
