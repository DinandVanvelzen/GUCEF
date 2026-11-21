/*
 *  gucefGUI_TestApp: Application for testing gucefGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GUI_CACTIVITYSERVICE_H
#include "gucefGUI_CActivityService.h"
#define GUCEF_GUI_CACTIVITYSERVICE_H
#endif /* GUCEF_GUI_CACTIVITYSERVICE_H ? */

#ifndef GUCEF_GUI_CACTIVITYSCOPE_H
#include "gucefGUI_CActivityScope.h"
#define GUCEF_GUI_CACTIVITYSCOPE_H
#endif /* GUCEF_GUI_CACTIVITYSCOPE_H ? */

#ifndef GUCEF_GUI_CACTIVITYINDICATORFORM_H
#include "gucefGUI_CActivityIndicatorForm.h"
#define GUCEF_GUI_CACTIVITYINDICATORFORM_H
#endif /* GUCEF_GUI_CACTIVITYINDICATORFORM_H ? */

#include "gucefGUI_TestCommon.h"
#include "gucefGUI_TestAssetFormSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::GUITEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** A clock the test moves */
struct SManualClock
{
    UInt64 now;
    SManualClock( void ) : now( 1000 ) {}
};

UInt64
ReadManualClock( void* userData )
{
    return static_cast< SManualClock* >( userData )->now;
}

/** The service, a context with the indicator in a strip of it, and a clock to move */
class CActivityRig
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    SManualClock clock;
    GUI::CActivityService service;
    GUI::CActivityIndicatorForm* indicator;
    GUI::CWidget* strip;

    CActivityRig( bool manualClock )
        : contextPtr( CreateFormContext() )
        , context( GUCEF_NULL )
        , clock()
        , service()
        , indicator( GUCEF_NULL )
        , strip( GUCEF_NULL )
    {
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return;
        if ( manualClock )
            service.SetClock( &ReadManualClock, &clock );
        GUI::CWidget* screen = MakeScreen( context );
        strip = context->CreateWidget( "Panel", "Strip", screen );
        strip->SetProperty( "Position", "0,0 1,-30" );
        strip->SetProperty( "Size", "1,0 0,30" );
        indicator = GUCEF_NEW GUI::CActivityIndicatorForm( context );
        indicator->Build( strip );
        indicator->SetService( &service );
        service.Attach( context );
        context->RenderFrame( 16 );
    }

    ~CActivityRig()
    {
        service.Detach();
        indicator->SetService( GUCEF_NULL );
        GUCEF_DELETE indicator;
    }

    /** Moves the clock and draws a frame */
    void Advance( UInt32 ms )
    {
        clock.now += ms;
        context->RenderFrame( ms );
    }
};

/** A recorder of the events of the service and of the ids they carry */
class CServiceRecorder : public CORE::CObserver
{
    public:

    UInt32 shown, changed, ended, cancelRequested, activityChanged;
    UInt32 lastCancelId;

    explicit CServiceRecorder( GUI::CActivityService* service )
        : CORE::CObserver(), shown( 0 ), changed( 0 ), ended( 0 ), cancelRequested( 0 ), activityChanged( 0 ), lastCancelId( 0 )
    {
        typedef CORE::CTEventHandlerFunctor< CServiceRecorder > TCallback;
        TCallback shownCallback( this, &CServiceRecorder::OnShown );
        SubscribeTo( service, GUI::CActivityService::OperationShownEvent, shownCallback );
        TCallback changedCallback( this, &CServiceRecorder::OnChanged );
        SubscribeTo( service, GUI::CActivityService::OperationChangedEvent, changedCallback );
        TCallback endedCallback( this, &CServiceRecorder::OnEnded );
        SubscribeTo( service, GUI::CActivityService::OperationEndedEvent, endedCallback );
        TCallback cancelCallback( this, &CServiceRecorder::OnCancel );
        SubscribeTo( service, GUI::CActivityService::CancelRequestedEvent, cancelCallback );
        TCallback activityCallback( this, &CServiceRecorder::OnActivity );
        SubscribeTo( service, GUI::CActivityService::ActivityChangedEvent, activityCallback );
    }

    virtual ~CServiceRecorder() { UnsubscribeAllFromObserver(); }

    void OnShown( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++shown; }
    void OnChanged( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++changed; }
    void OnEnded( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++ended; }
    void OnActivity( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++activityChanged; }
    void OnCancel( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* data )
    {
        ++cancelRequested;
        const CORE::TCloneableUInt32* id = static_cast< const CORE::TCloneableUInt32* >( data );
        if ( GUCEF_NULL != id )
            lastCancelId = id->GetData();
    }
};

/** What the pool task of the thread test does: announces, reports and ends operations of the service */
class CActivityWorkerData : public CORE::CICloneable
{
    public:

    GUI::CActivityService* service;
    UInt32 operations;
    UInt32 finished;

    CActivityWorkerData( void ) : CORE::CICloneable(), service( GUCEF_NULL ), operations( 0 ), finished( 0 ) {}
    virtual ~CActivityWorkerData() {}
    virtual CORE::CICloneable* Clone( void ) const { return GUCEF_NEW CActivityWorkerData( *this ); }
};

class CActivityWorkerTask : public CORE::CTaskConsumer
{
    public:

    static const CORE::CString TaskType;

    virtual bool OnTaskCycle( CORE::CTaskPtr task )
    {
        CActivityWorkerData* data = static_cast< CActivityWorkerData* >( task->GetTaskData() );
        for ( UInt32 i=0; i<data->operations; ++i )
        {
            const UInt32 id = data->service->Begin( "Worker operation", 0, true, true );
            data->service->SetProgress( id, 0.5f );
            data->service->SetLabel( id, "Worker operation, half" );
            data->service->IsCancelRequested( id );
            MT::PrecisionDelay( 2 );
            data->service->End( id );
        }
        data->finished = data->operations;
        return true;
    }
    virtual void OnTaskEnded( CORE::CTaskPtr task, bool wasForced ) {}
    virtual CORE::CString GetType( void ) const { return TaskType; }
};

const CORE::CString CActivityWorkerTask::TaskType = "TestActivityWorker";

typedef CORE::CTFactory< CORE::CTaskConsumer, CActivityWorkerTask, MT::CMutex > TActivityWorkerFactory;

/** A scene of the indicator for the layout lint: a long label, a progress bar, the count and the button */
class CActivityScene : public CFormScene
{
    public:

    GUI::CActivityService service;
    GUI::CActivityIndicatorForm indicator;

    explicit CActivityScene( GUI::CGUIContext* context ) : CFormScene(), service(), indicator( context )
    {
        service.SetThresholdInMs( 0 );
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* strip = context->CreateWidget( "Panel", "Strip", screen );
        strip->SetProperty( "Position", "0,0 1,-30" );
        strip->SetProperty( "Size", "1,0 0,30" );
        indicator.Build( strip );
        indicator.SetService( &service );
        service.Attach( context );
        service.Begin( "Opening a model with a rather long name of the kind that packs ship.fbx", 0, true, true );
        const UInt32 second = service.Begin( "Importing", 0, true, true );
        service.SetProgress( second, 0.4f );
        context->RenderFrame( 16 );
    }

    virtual ~CActivityScene()
    {
        service.Detach();
        indicator.SetService( GUCEF_NULL );
    }
};

CFormScene*
BuildActivityScene( GUI::CGUIContext* context )
{
    return new CActivityScene( context );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformGuiActivityServiceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.ActivityService" );

    GUCEF_TESTFW_TESTCASE( "ActivityService: An operation that ends before the threshold shows nothing: no event, no cursor, no indicator" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        CServiceRecorder events( &rig.service );
        ASSERT_TRUE( 300 == rig.service.GetThresholdInMs() );

        const UInt32 id = rig.service.Begin( "Fast", 0, false, true );
        ASSERT_TRUE( 0 != id && rig.service.IsActive( id ) );
        rig.Advance( 100 );
        rig.Advance( 150 );
        ASSERT_TRUE( 0 == rig.service.GetShownCount() );
        ASSERT_FALSE( rig.indicator->IsIndicatorShown() );
        rig.service.End( id );
        rig.Advance( 400 );
        ASSERT_FALSE( rig.service.IsActive( id ) );
        ASSERT_TRUE( 0 == events.shown && 0 == events.ended && 0 == events.activityChanged );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == rig.context->GetSystemCursorShape() && !rig.service.IsBusyCursorShown() );
        ASSERT_FALSE( rig.indicator->IsIndicatorShown() );
        ASSERT_FALSE( rig.indicator->GetRootWidget()->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: A slow operation is shown at the threshold, with the busy cursor, the label and a bar, and goes away at its end" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        CServiceRecorder events( &rig.service );

        const UInt32 id = rig.service.Begin( "Opening tree.fbx", 0, true, false );
        rig.Advance( 299 );
        ASSERT_TRUE( 0 == rig.service.GetShownCount() && !rig.indicator->IsIndicatorShown() );
        rig.Advance( 1 );
        ASSERT_TRUE( 1 == rig.service.GetShownCount() && 1 == events.shown );
        ASSERT_TRUE( rig.indicator->IsIndicatorShown() && id == rig.indicator->GetDisplayedOperation() );
        ASSERT_TRUE( rig.indicator->GetRootWidget()->IsVisible() );
        ASSERT_TRUE( "Opening tree.fbx" == rig.indicator->GetWidget( "Text" )->GetProperty( GUI::EWidgetProperty_Text ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_PROGRESS == rig.context->GetSystemCursorShape() && rig.service.IsBusyCursorShown() );
        // not cancellable: no button
        ASSERT_FALSE( rig.indicator->GetWidget( "Cancel" )->IsVisible() );

        rig.service.SetProgress( id, 0.25f );
        rig.Advance( 16 );
        ASSERT_TRUE( 1 == events.changed );
        GUI::CProgressBar* bar = rig.indicator->GetWidgetAs< GUI::CProgressBar >( "Bar" );
        ASSERT_TRUE( GUCEF_NULL != bar && bar->GetFraction() > 0.24f && bar->GetFraction() < 0.26f );

        {
            GUI::CActivityInfo info;
            ASSERT_TRUE( rig.service.GetOperation( id, info ) && info.shown && info.determinate && !info.cancellable );
        }

        rig.service.End( id );
        rig.Advance( 16 );
        ASSERT_TRUE( 1 == events.ended && 0 == rig.service.GetShownCount() );
        ASSERT_FALSE( rig.indicator->IsIndicatorShown() );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == rig.context->GetSystemCursorShape() && !rig.service.IsBusyCursorShown() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: The threshold is settable, and 0 shows every operation at the next frame" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        rig.service.SetThresholdInMs( 1000 );
        const UInt32 first = rig.service.Begin( "Slow to show", 0, false, false );
        rig.Advance( 900 );
        ASSERT_TRUE( 0 == rig.service.GetShownCount() );
        rig.Advance( 100 );
        ASSERT_TRUE( 1 == rig.service.GetShownCount() );
        rig.service.End( first );
        rig.service.SetThresholdInMs( 0 );
        rig.service.Begin( "At once", 0, false, false );
        rig.Advance( 1 );
        ASSERT_TRUE( 1 == rig.service.GetShownCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: An operation that announces more than the threshold as its duration is shown at the next frame, not after the threshold" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        CServiceRecorder events( &rig.service );
        const UInt32 id = rig.service.Begin( "Announced long", 2000, false, false );
        rig.Advance( 16 );
        ASSERT_TRUE( 1 == events.shown && 1 == rig.service.GetShownCount() && rig.indicator->IsIndicatorShown() );
        rig.service.End( id );
        // exactly the threshold announced is not "longer than": it waits
        const UInt32 equal = rig.service.Begin( "Announced the threshold", 300, false, false );
        rig.Advance( 16 );
        ASSERT_TRUE( 0 == rig.service.GetShownCount() );
        rig.service.End( equal );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: Cancel: the button asks the service, the event carries the id, the worker sees the flag; an operation that cannot be cancelled refuses" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        CServiceRecorder events( &rig.service );
        const UInt32 fixed = rig.service.Begin( "Cannot cancel", 1000, false, false );
        ASSERT_FALSE( rig.service.RequestCancel( fixed ) );
        rig.service.End( fixed );
        rig.Advance( 16 );
        events.shown = 0;

        const UInt32 id = rig.service.Begin( "Cancel me", 1000, false, true );
        rig.Advance( 16 );
        GUI::CWidget* cancel = rig.indicator->GetWidget( "Cancel" );
        ASSERT_TRUE( rig.indicator->IsIndicatorShown() && cancel->IsVisible() && cancel->IsEnabled() );
        ASSERT_FALSE( rig.service.IsCancelRequested( id ) );
        ClickWidget( rig.context, cancel );
        // the flag is there at once, on any thread; the event follows at the frame
        ASSERT_TRUE( rig.service.IsCancelRequested( id ) );
        rig.Advance( 16 );
        ASSERT_TRUE( 1 == events.cancelRequested && id == events.lastCancelId );
        ASSERT_FALSE( rig.indicator->GetWidget( "Cancel" )->IsEnabled() );
        // asking again does not say it twice
        ASSERT_TRUE( rig.service.RequestCancel( id ) );
        rig.Advance( 16 );
        ASSERT_TRUE( 1 == events.cancelRequested );
        rig.service.End( id );
        rig.Advance( 16 );
        ASSERT_FALSE( rig.service.IsCancelRequested( id ) && rig.service.IsActive( id ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: Concurrent operations: the indicator shows the newest and the count, and falls back when it ends; the jobs list reads them all" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        CServiceRecorder events( &rig.service );
        const UInt32 a = rig.service.Begin( "Alpha", 1000, false, false );
        const UInt32 b = rig.service.Begin( "Beta", 1000, false, false );
        const UInt32 c = rig.service.Begin( "Gamma", 1000, false, false );
        rig.Advance( 16 );
        ASSERT_TRUE( 3 == events.shown && 3 == rig.service.GetShownCount() && 3 == rig.service.GetActiveCount() );
        ASSERT_TRUE( c == rig.indicator->GetDisplayedOperation() );
        ASSERT_TRUE( "Gamma" == rig.indicator->GetWidget( "Text" )->GetProperty( GUI::EWidgetProperty_Text ) );
        ASSERT_TRUE( "3 operations" == rig.indicator->GetWidget( "Count" )->GetProperty( GUI::EWidgetProperty_Text ) );
        GUI::CActivityService::TActivityInfoVector list;
        rig.service.GetShownOperations( list );
        ASSERT_TRUE( 3 == list.size() && "Alpha" == list[ 0 ].label && "Gamma" == list[ 2 ].label );

        rig.service.End( c );
        rig.Advance( 16 );
        ASSERT_TRUE( b == rig.indicator->GetDisplayedOperation() && "2 operations" == rig.indicator->GetWidget( "Count" )->GetProperty( GUI::EWidgetProperty_Text ) );
        rig.service.End( b );
        rig.Advance( 16 );
        ASSERT_TRUE( a == rig.indicator->GetDisplayedOperation() && rig.indicator->GetWidget( "Count" )->GetProperty( GUI::EWidgetProperty_Text ).IsNULLOrEmpty() );
        // the cursor stays busy while one is left
        ASSERT_TRUE( GUI::CURSORSHAPE_PROGRESS == rig.context->GetSystemCursorShape() );
        rig.service.End( a );
        rig.Advance( 16 );
        ASSERT_TRUE( 3 == events.ended && !rig.indicator->IsIndicatorShown() && GUI::CURSORSHAPE_ARROW == rig.context->GetSystemCursorShape() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: An indeterminate operation animates the bar on the frames; the scope announces and ends with its lifetime" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        {
            GUI::CActivityScope scope( &rig.service, "Scoped", 1000 );
            ASSERT_TRUE( 0 != scope.GetId() && rig.service.IsActive( scope.GetId() ) );
            rig.Advance( 16 );
            GUI::CProgressBar* bar = rig.indicator->GetWidgetAs< GUI::CProgressBar >( "Bar" );
            ASSERT_TRUE( GUCEF_NULL != bar );
            const Float32 first = bar->GetFraction();
            rig.Advance( 200 );
            const Float32 second = bar->GetFraction();
            rig.Advance( 400 );
            const Float32 third = bar->GetFraction();
            ASSERT_TRUE( first != second && second != third );
            ASSERT_TRUE( first >= 0.0f && first <= 1.0f && second >= 0.0f && second <= 1.0f && third >= 0.0f && third <= 1.0f );
            scope.SetLabel( "Scoped again" );
            rig.Advance( 16 );
            ASSERT_TRUE( "Scoped again" == rig.indicator->GetWidget( "Text" )->GetProperty( GUI::EWidgetProperty_Text ) );
        }
        rig.Advance( 16 );
        ASSERT_TRUE( 0 == rig.service.GetActiveCount() && !rig.indicator->IsIndicatorShown() );
        // without a service the scope does nothing
        GUI::CActivityScope nothing( GUCEF_NULL, "Nowhere" );
        ASSERT_TRUE( 0 == nothing.GetId() && !nothing.IsCancelRequested() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: On the real clock a slow operation is shown within 300 ms plus a frame, and a fast one never is" )
    try
    {
        CActivityRig rig( false );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        CServiceRecorder events( &rig.service );

        // fast: ends after 120 ms
        const UInt32 fast = rig.service.Begin( "Real fast", 0, false, false );
        UInt64 start = MT::PrecisionTickCount();
        const UInt64 resolution = MT::PrecisionTimerResolution();
        UInt64 last = start;
        while ( ( MT::PrecisionTickCount() - start ) * 1000 / resolution < 120 )
        {
            MT::PrecisionDelay( 8 );
            const UInt64 now = MT::PrecisionTickCount();
            rig.context->RenderFrame( (UInt32) ( ( now - last ) * 1000 / resolution ) );
            last = now;
        }
        rig.service.End( fast );
        rig.context->RenderFrame( 16 );
        ASSERT_TRUE( 0 == events.shown && !rig.indicator->IsIndicatorShown() );

        // slow: when is it shown
        const UInt32 slow = rig.service.Begin( "Real slow", 0, false, false );
        start = MT::PrecisionTickCount();
        last = start;
        UInt64 shownAfter = 0;
        while ( ( MT::PrecisionTickCount() - start ) * 1000 / resolution < 800 )
        {
            MT::PrecisionDelay( 8 );
            const UInt64 now = MT::PrecisionTickCount();
            rig.context->RenderFrame( (UInt32) ( ( now - last ) * 1000 / resolution ) );
            last = now;
            if ( rig.indicator->IsIndicatorShown() )
            {
                shownAfter = ( now - start ) * 1000 / resolution;
                break;
            }
        }
        rig.service.End( slow );
        ASSERT_TRUE( 0 != shownAfter );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ActivityService: The indicator of a slow operation showed after " << shownAfter << " ms" );
        // never before the threshold, and no later than a frame of the loop (8 ms and the work of a frame) plus the scheduling of the machine
        ASSERT_TRUE( shownAfter >= 295 && shownAfter <= 360 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: Operations announced from a thread of a pool are safe: all of them end, and the queue is read on the interface thread only" )
    try
    {
        CActivityRig rig( false );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        rig.service.SetThresholdInMs( 0 );
        CServiceRecorder events( &rig.service );

        TActivityWorkerFactory factory;
        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        taskManager.RegisterTaskConsumerFactory( CActivityWorkerTask::TaskType, &factory );
        CActivityWorkerData data;
        data.service = &rig.service;
        data.operations = 60;
        CORE::ThreadPoolPtr pool = taskManager.GetOrCreateThreadPool( "ActivityTest" );
        ASSERT_FALSE( pool.IsNULL() );
        CORE::CFutureResult future = pool->QueueTask( CActivityWorkerTask::TaskType, &data );
        ASSERT_TRUE( future.HasAFuture() );
        UInt32 guard = 0;
        while ( !future.IsReady() && guard++ < 2000 )
        {
            rig.context->RenderFrame( 4 );
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( future.IsReady() );
        rig.context->RenderFrame( 4 );
        rig.context->RenderFrame( 4 );
        CORE::CTaskPtr task = future.GetResult();
        const CActivityWorkerData* done = static_cast< const CActivityWorkerData* >( task->GetTaskData() );
        ASSERT_TRUE( GUCEF_NULL != done && 60 == done->finished );
        ASSERT_TRUE( 0 == rig.service.GetActiveCount() );
        // every operation that was shown ended: shown and ended events balance, whatever the interleaving was
        ASSERT_TRUE( events.shown == events.ended );
        ASSERT_TRUE( events.shown > 0 );
        taskManager.UnregisterTaskConsumerFactory( CActivityWorkerTask::TaskType );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: The watchdog counts a frame longer than the stall threshold in a debug build and costs nothing in a release build" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        ASSERT_TRUE( 100 == rig.service.GetStallThresholdInMs() );
        rig.service.Begin( "Blocking", 1000, false, false );
        rig.Advance( 16 );
        rig.Advance( 100 );
        ASSERT_TRUE( 0 == rig.service.GetStallCount() );
        rig.Advance( 250 );
        #ifdef GUCEF_DEBUG_MODE
        ASSERT_TRUE( 1 == rig.service.GetStallCount() );
        #else
        ASSERT_TRUE( 0 == rig.service.GetStallCount() );
        #endif
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: The service and the indicator survive each other: either one can be destroyed first" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        GUI::CWidget* screen = MakeScreen( context );
        GUI::CWidget* strip = context->CreateWidget( "Panel", "Strip", screen );
        strip->SetProperty( "Size", "1,0 0,30" );
        GUI::CActivityIndicatorForm indicator( context );
        ASSERT_TRUE( indicator.Build( strip ) );
        {
            GUI::CActivityService service;
            service.SetThresholdInMs( 0 );
            service.Attach( context );
            indicator.SetService( &service );
            service.Begin( "Gone soon", 0, false, false );
            context->RenderFrame( 16 );
            ASSERT_TRUE( indicator.IsIndicatorShown() );
        }
        // the service is gone: the indicator is empty, and the context has its own cursor again
        ASSERT_TRUE( GUCEF_NULL == indicator.GetService() );
        context->RenderFrame( 16 );
        ASSERT_FALSE( indicator.IsIndicatorShown() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ActivityService: The busy cursor stays while a driver sets the shape of the context every frame, and the shape that was set is shown again after" )
    try
    {
        CActivityRig rig( true );
        ASSERT_TRUE( GUCEF_NULL != rig.context );
        rig.context->SetSystemCursorShape( GUI::CURSORSHAPE_HAND );
        ASSERT_TRUE( GUI::CURSORSHAPE_HAND == rig.context->GetSystemCursorShape() );
        rig.service.Begin( "Busy", 1000, false, false );
        rig.Advance( 16 );
        ASSERT_TRUE( rig.context->IsSystemCursorShapeOverridden() && GUI::CURSORSHAPE_PROGRESS == rig.context->GetSystemCursorShape() );
        // what a driver does: another shape, every frame
        rig.context->SetSystemCursorShape( GUI::CURSORSHAPE_TEXT );
        rig.Advance( 16 );
        rig.context->SetSystemCursorShape( GUI::CURSORSHAPE_ARROW );
        ASSERT_TRUE( GUI::CURSORSHAPE_PROGRESS == rig.context->GetSystemCursorShape() );
        rig.service.SetBusyCursorEnabled( false );
        ASSERT_TRUE( !rig.context->IsSystemCursorShapeOverridden() && GUI::CURSORSHAPE_ARROW == rig.context->GetSystemCursorShape() );
        rig.service.SetBusyCursorEnabled( true );
        ASSERT_TRUE( GUI::CURSORSHAPE_PROGRESS == rig.context->GetSystemCursorShape() );
        rig.service.Detach();
        ASSERT_TRUE( !rig.context->IsSystemCursorShapeOverridden() && GUI::CURSORSHAPE_ARROW == rig.context->GetSystemCursorShape() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    RunFormLintBaseline( "ActivityIndicator", &BuildActivityScene );
}

/*-------------------------------------------------------------------------*/
