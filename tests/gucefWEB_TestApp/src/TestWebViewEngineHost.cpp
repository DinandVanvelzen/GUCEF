/*
 *  gucefWEB_TestApp: tests of the gucefWEB module, its web views
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

#include <math.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#include "TestWebViewSupport.h"
#include "TestWebViewEventLog.h"
#include "TestFakeWebView.h"
#include "TestFakeWebViewEngine.h"
#include "TestWebViewSuites.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::WEB;
using namespace GUCEF::WEBVIEWTEST;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

/** An engine built for another version of the interface */
class COldInterfaceEngine : public CFakeWebViewEngine
{
    public:

    COldInterfaceEngine( void ) : CFakeWebViewEngine( "OldInterface" ) {}

    virtual UInt32 GetInterfaceVersion( void ) const GUCEF_VIRTUAL_OVERRIDE { return IWebViewEngine::InterfaceVersion + 1; }
};

/** A fake engine of another name */
TWebViewEnginePtr
MakeFakeEngine( const CORE::CString& name, UInt32 capabilities = 0 )
{
    CFakeWebViewEngine* engine = GUCEF_NEW CFakeWebViewEngine( name );
    if ( 0 != capabilities )
        engine->SetCapabilities( capabilities );
    return TWebViewEnginePtr( engine );
}

CFakeWebView*
AsFake( const TWebViewPtr& view )
{
    return static_cast< CFakeWebView* >( const_cast< TWebViewPtr& >( view ).GetPointerAlways() );
}

/** Restores the module settings when it goes */
class CGlobalSettingsScope
{
    public:

    CGlobalSettingsScope( void ) : m_saved( CWebViewGlobal::Instance()->GetSettings() ) {}

    ~CGlobalSettingsScope() { CWebViewGlobal::Instance()->SetSettings( m_saved ); }

    private:

    CWebViewGlobalSettings m_saved;
};

}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformWebViewEngineHostTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "WebView.EngineHost" );

    GUCEF_TESTFW_TESTCASE( "EngineHost: no thread until the first view; then one dedicated thread of the task manager on the pool WebView.<engine>" )
    try
    {
        CFakeWebViewEngine engine( "LazyStart" );
        ASSERT_TRUE( engine.GetHost().IsNULL() && !engine.WasEngineThreadStarted() && !engine.IsRunning() );
        ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( "WebView.LazyStart" ).IsNULL() );
        TWebViewPtr view = CreateTestView( engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() && 1 == engine.GetViewCount() );
        ASSERT_TRUE( engine.WasEngineThreadStarted() && engine.IsRunning() );
        ASSERT_TRUE( engine.GetStartedThreadId() != MT::GetCurrentThreadID() && 0 != engine.GetStartedThreadId() );
        ASSERT_TRUE( engine.GetHost()->GetEngineThreadId() == engine.GetStartedThreadId() );
        ASSERT_FALSE( engine.IsEngineThread() );
        ASSERT_FALSE( CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool( "WebView.LazyStart" ).IsNULL() );
        ASSERT_TRUE( CString( "Fake" ) != engine.GetEngineName() && "LazyStart" == view->GetEngineName() );
        ASSERT_TRUE( AwaitOk( view->Close() ) );
        ASSERT_TRUE( view->IsClosed() && 0 == engine.GetViewCount() );
        ASSERT_TRUE( engine.Shutdown( 5000 ) );
        ASSERT_FALSE( engine.IsRunning() );
        ASSERT_TRUE( WEBVIEWERROR_SHUTTING_DOWN == AwaitError( engine.CreateViewAsync( MakeTestSettings() ) ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: commands of a view are carried out in the order they were posted, fire and forget ones included" )
    try
    {
        CFakeWebViewEngine engine( "Ordering" );
        TWebViewPtr view = CreateTestView( engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        for ( Int32 i=0; i<300; ++i )
            view->InjectInput( CWebViewInputEvent::PointerMove( i, 1000 - i ) );
        ASSERT_TRUE( AwaitOk( view->SetZoom( 1.5f ) ) );         // after every move
        GUCEF::vector< CWebViewInputEvent > recorded = AsFake( view )->GetRecordedInput();
        ASSERT_TRUE( 300 == recorded.size() );
        bool ordered = true;
        for ( UInt32 i=0; i<(UInt32)recorded.size(); ++i )
            if ( (Int32) i != recorded[ i ].x || 1000 - (Int32) i != recorded[ i ].y )
                ordered = false;
        ASSERT_TRUE( ordered );
        ASSERT_TRUE( 1.5f == AsFake( view )->GetZoom() );
        ASSERT_TRUE( engine.GetHost()->GetExecutedCommandCount() >= 302 );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: events are notified on the engine thread; a relay hands them, in order, to the thread that relays" )
    try
    {
        CFakeWebViewEngine engine( "Events" );
        TWebViewPtr view = CreateTestView( engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        CTestEventLog direct;
        direct.Watch( view.GetPointerAlways() );
        CWebViewEventRelay relay;
        ASSERT_TRUE( relay.Attach( view ) );
        CTestEventLog relayed;
        relayed.Watch( &relay );

        ASSERT_TRUE( CFakeWebSite::Url( "/landing" ) == AwaitText( view->Navigate( CFakeWebSite::Url( "/redirect" ) ) ) );
        ASSERT_TRUE( direct.WaitFor( IWebView::LoadFinishedEvent ) );
        ASSERT_TRUE( direct.AllOnThread( engine.GetStartedThreadId() ) );
        ASSERT_TRUE( 0 == relayed.Total() );                       // nothing until the owner relays
        ASSERT_TRUE( relay.HasEvents() );
        const UInt32 count = relay.Relay();
        ASSERT_TRUE( count == direct.Total() && count == relayed.Total() );
        ASSERT_TRUE( relayed.AllOnThread( MT::GetCurrentThreadID() ) );
        ASSERT_TRUE( direct.Names() == relayed.Names() );

        CORE::CString::StringSet loadEvents;
        loadEvents.insert( "LoadStartedEvent" );
        loadEvents.insert( "NavigationRequestedEvent" );
        loadEvents.insert( "LoadCommittedEvent" );
        loadEvents.insert( "TitleChangedEvent" );
        loadEvents.insert( "LoadFinishedEvent" );
        CORE::CString::StringVector order = relayed.Names( loadEvents );
        CORE::CString::StringVector expected;
        expected.push_back( "NavigationRequestedEvent" );   // the application's navigation, by the policy
        expected.push_back( "LoadStartedEvent" );
        expected.push_back( "NavigationRequestedEvent" );   // the redirect
        expected.push_back( "LoadCommittedEvent" );
        expected.push_back( "TitleChangedEvent" );
        expected.push_back( "LoadFinishedEvent" );
        ASSERT_TRUE( order == expected );
        CString committed;
        ASSERT_TRUE( relayed.GetData< CString >( IWebView::LoadCommittedEvent, 0, committed ) && CFakeWebSite::Url( "/landing" ) == committed );

        relay.Detach();
        ASSERT_FALSE( relay.HasEvents() );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: a thread that must not wait (the GUI thread) is refused a wait; the engine threads are such threads" )
    try
    {
        CFakeWebViewEngine engine( "NoWait" );
        TWebViewPtr view = CreateTestView( engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        ASSERT_TRUE( CWebViewGlobal::Instance()->MayWaitOnCurrentThread() );
        CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), true );
        CORE::CFutureResult future = view->SetZoom( 1.25f );
        CWebViewCommandResult result;
        const UInt64 start = MT::PrecisionTickCount();
        ASSERT_FALSE( CWebViewCommandResult::Await( future, 5000, result ) );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == result.error );
        ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 1000.0 );   // refused, not waited
        CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), false );
        ASSERT_TRUE( AwaitOk( future ) );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), false );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: shutdown with views open closes them on the engine thread, fails what is pending and ends the thread" )
    try
    {
        CFakeWebViewEngine engine( "Shutdown" );
        engine.SetLoopMs( 300 );
        TWebViewPtr first = CreateTestView( engine, MakeTestSettings() );
        TWebViewPtr second = CreateTestView( engine, MakeTestSettings( "other" ) );
        ASSERT_TRUE( !first.IsNULL() && !second.IsNULL() && 2 == engine.GetViewCount() );
        CTestEventLog log;
        log.Watch( first.GetPointerAlways() );
        log.Watch( second.GetPointerAlways() );
        CORE::CFutureResult busy = first->Navigate( CFakeWebSite::Url( "/loop" ) );
        CORE::CFutureResult queued = second->Navigate( CFakeWebSite::Url( "/" ) );
        ASSERT_TRUE( engine.Shutdown( 5000 ) );
        ASSERT_TRUE( busy.IsReady() && queued.IsReady() );
        ASSERT_TRUE( first->IsClosed() && second->IsClosed() );
        ASSERT_TRUE( 2 == log.Count( IWebView::ClosedEvent ) );
        ASSERT_FALSE( engine.IsRunning() );
        ASSERT_TRUE( 0 == engine.GetViewCount() );
        ASSERT_TRUE( WEBVIEWERROR_VIEW_CLOSED == AwaitError( first->Navigate( CFakeWebSite::Url( "/" ) ) ) );
        ASSERT_TRUE( engine.Shutdown( 5000 ) );                    // twice is fine
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: the watchdog finds an engine thread that does not answer, fails its futures and reports it once" )
    try
    {
        CGlobalSettingsScope restore;
        CWebViewGlobalSettings settings = CWebViewGlobal::Instance()->GetSettings();
        settings.engineWatchdogMs = 200;
        CWebViewGlobal::Instance()->SetSettings( settings );

        TWebViewEnginePtr enginePtr = MakeFakeEngine( "Watchdog" );
        CFakeWebViewEngine& engine = *static_cast< CFakeWebViewEngine* >( enginePtr.GetPointerAlways() );
        engine.SetLoopMs( 1500 );
        ASSERT_TRUE( CWebViewGlobal::Instance()->GetEngineRegistry().Register( enginePtr ) );
        TWebViewPtr view = CreateTestView( engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        CTestEventLog engineLog;
        engineLog.WatchEvent( &engine, IWebViewEngine::EngineStalledEvent );
        CTestEventLog viewLog;
        viewLog.Watch( view.GetPointerAlways() );

        ASSERT_TRUE( 0 == CWebViewGlobal::Instance()->CheckEngineWatchdogs() );
        CORE::CFutureResult stuck = view->Navigate( CFakeWebSite::Url( "/loop" ) );
        CORE::CFutureResult waiting = view->SetZoom( 2.0f );
        MT::PrecisionDelay( 450 );
        ASSERT_TRUE( 1 == CWebViewGlobal::Instance()->CheckEngineWatchdogs() );
        ASSERT_TRUE( engine.IsStalled() );
        ASSERT_TRUE( WEBVIEWERROR_ENGINE_STALLED == AwaitError( stuck, 100 ) );
        ASSERT_TRUE( WEBVIEWERROR_ENGINE_STALLED == AwaitError( waiting, 100 ) );
        ASSERT_TRUE( 1 == engineLog.Count( IWebViewEngine::EngineStalledEvent ) );
        ASSERT_TRUE( 1 == viewLog.Count( IWebView::EngineStalledEvent ) );
        ASSERT_TRUE( 1 == CWebViewGlobal::Instance()->CheckEngineWatchdogs() );       // still stalled, reported once
        ASSERT_TRUE( 1 == engineLog.Count( IWebViewEngine::EngineStalledEvent ) );
        ASSERT_TRUE( WEBVIEWERROR_VIEW_CLOSED == AwaitError( view->Reload(), 100 ) );   // recovery: a new view
        CWebViewEngineSelection selection;
        CORE::CString::StringVector order;
        order.push_back( "Watchdog" );
        CWebViewGlobal::Instance()->FindEngine( 0, order, selection );
        ASSERT_TRUE( !selection.candidates.empty() && CWebViewEngineSelection::OUTCOME_STALLED == selection.candidates[ 0 ].outcome );

        ASSERT_TRUE( CWebViewGlobal::Instance()->GetEngineRegistry().Unregister( "Watchdog" ) );
        engine.Shutdown( 5000 );                                    // the page lets go of the thread after its loop
    }
    catch ( ... )
    {
        CWebViewGlobal::Instance()->GetEngineRegistry().Unregister( "Watchdog" );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: an idle engine thread that wakes only when woken answers a command at once" )
    try
    {
        CGlobalSettingsScope restore;
        CWebViewGlobalSettings settings = CWebViewGlobal::Instance()->GetSettings();
        settings.engineIdleWakeMs = 0;
        CWebViewGlobal::Instance()->SetSettings( settings );
        CFakeWebViewEngine engine( "Idle" );
        TWebViewPtr view = CreateTestView( engine, MakeTestSettings() );
        ASSERT_TRUE( !view.IsNULL() );
        MT::PrecisionDelay( 300 );
        const UInt32 executed = engine.GetHost()->GetExecutedCommandCount();
        MT::PrecisionDelay( 200 );
        ASSERT_TRUE( executed == engine.GetHost()->GetExecutedCommandCount() );        // nothing happens while idle
        const UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( AwaitOk( view->SetFocus( true ) ) );
        ASSERT_TRUE( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < 250.0 );
        AwaitOk( view->Close() );
        ASSERT_TRUE( engine.Shutdown( 5000 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineHost: deferred decisions are answered once, refused after the time out, unknown ids are refused" )
    try
    {
        CFakeWebViewEngine engine( "Decisions" );
        CWebViewSettings settings = MakeTestSettings();
        settings.policy.permissions = WEBVIEWPERMISSIONS_ASK;
        settings.decisionTimeoutMs = 150;
        TWebViewPtr view = CreateTestView( engine, settings );
        ASSERT_TRUE( !view.IsNULL() );
        CTestEventLog log;
        log.Watch( view.GetPointerAlways() );
        ASSERT_TRUE( AwaitOk( view->Navigate( CFakeWebSite::Url( "/permissions" ) ) ) );
        ASSERT_TRUE( log.WaitFor( IWebView::PermissionRequestedEvent, 2 ) );
        CWebViewPermissionRequest first, second;
        ASSERT_TRUE( log.GetData< CWebViewPermissionRequest >( IWebView::PermissionRequestedEvent, 0, first ) );
        ASSERT_TRUE( log.GetData< CWebViewPermissionRequest >( IWebView::PermissionRequestedEvent, 1, second ) );
        ASSERT_TRUE( WEBVIEWDECISION_ASK == first.decision && 0 != first.decisionId && first.decisionId != second.decisionId );
        ASSERT_TRUE( WEBVIEWPERMISSION_NOTIFICATIONS == first.kind && WEBVIEWPERMISSION_GEOLOCATION == second.kind );
        ASSERT_TRUE( AwaitOk( view->ResolveDecision( first.decisionId, true ) ) );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->ResolveDecision( first.decisionId, true ) ) );   // once
        MT::PrecisionDelay( 400 );
        CORE::CString::StringVector answers = AsFake( view )->GetAnswers();
        ASSERT_TRUE( 2 == answers.size() && "permission:allowed" == answers[ 0 ] && "permission:timedOut" == answers[ 1 ] );
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->ResolveDecision( second.decisionId, true ) ) );  // timed out already
        ASSERT_TRUE( WEBVIEWERROR_INVALID_ARGUMENT == AwaitError( view->ResolveDecision( 9999, false ) ) );
        AwaitOk( view->Close() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "EngineRegistry: engines by name in registration order, duplicates and other interface versions refused, events" )
    try
    {
        CWebViewEngineRegistry registry;
        CTestEventLog log;
        log.WatchEvent( &registry, CWebViewEngineRegistry::EngineRegisteredEvent );
        log.WatchEvent( &registry, CWebViewEngineRegistry::EngineUnregisteredEvent );
        ASSERT_TRUE( registry.Register( MakeFakeEngine( "B" ) ) );
        ASSERT_TRUE( registry.Register( MakeFakeEngine( "A" ) ) );
        ASSERT_TRUE( registry.Register( MakeFakeEngine( "C" ) ) );
        ASSERT_FALSE( registry.Register( MakeFakeEngine( "a" ) ) );          // names are compared without case
        ASSERT_FALSE( registry.Register( TWebViewEnginePtr( GUCEF_NEW COldInterfaceEngine() ) ) );
        ASSERT_FALSE( registry.Register( TWebViewEnginePtr() ) );
        ASSERT_TRUE( 3 == registry.GetEngineCount() && 3 == log.Count( CWebViewEngineRegistry::EngineRegisteredEvent ) );
        CORE::CString::StringVector names = registry.GetEngineNames();
        ASSERT_TRUE( "B" == names[ 0 ] && "A" == names[ 1 ] && "C" == names[ 2 ] );
        CORE::CString::StringVector preferred;
        preferred.push_back( "C" );
        preferred.push_back( "Missing" );
        CWebViewEngineRegistry::TEngineVector ordered = registry.GetEnginesInOrder( preferred );
        ASSERT_TRUE( 3 == ordered.size() && "C" == ordered[ 0 ]->GetEngineName() && "B" == ordered[ 1 ]->GetEngineName() && "A" == ordered[ 2 ]->GetEngineName() );
        ASSERT_TRUE( !registry.GetEngine( "a" ).IsNULL() && registry.GetEngine( "Missing" ).IsNULL() );
        ASSERT_TRUE( registry.Unregister( "A" ) && !registry.Unregister( "A" ) );
        ASSERT_TRUE( 1 == log.Count( CWebViewEngineRegistry::EngineUnregisteredEvent ) && 2 == registry.GetEngineCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "FindEngine: the first available engine with the capabilities, in the order asked, with the reasons of the others" )
    try
    {
        CWebViewEngineRegistry& registry = CWebViewGlobal::Instance()->GetEngineRegistry();
        typedef CWebViewCapabilities C;
        TWebViewEnginePtr basic = MakeFakeEngine( "SelBasic", C::CAP_OFFSCREEN_FRAMES | C::CAP_INJECT_POINTER );
        TWebViewEnginePtr full = MakeFakeEngine( "SelFull" );
        TWebViewEnginePtr gone = MakeFakeEngine( "SelGone" );
        static_cast< CFakeWebViewEngine* >( gone.GetPointerAlways() )->SetAvailable( false );
        TWebViewEnginePtr devTools = MakeFakeEngine( "ChromeDevTools" );
        ASSERT_TRUE( registry.Register( basic ) && registry.Register( full ) && registry.Register( gone ) && registry.Register( devTools ) );

        CORE::CString::StringVector order;
        order.push_back( "SelMissing" );
        order.push_back( "SelGone" );
        order.push_back( "ChromeDevTools" );
        order.push_back( "SelBasic" );
        order.push_back( "SelFull" );
        CWebViewEngineSelection selection;
        ASSERT_TRUE( CWebViewGlobal::Instance()->FindEngine( C::CAP_COOKIES | C::CAP_SCRIPT_RESULT, order, selection ) );
        ASSERT_TRUE( "SelFull" == selection.engine->GetEngineName() );
        ASSERT_TRUE( "SelMissing" == selection.candidates[ 0 ].engineName && CWebViewEngineSelection::OUTCOME_NOT_LOADED == selection.candidates[ 0 ].outcome );
        ASSERT_TRUE( "SelGone" == selection.candidates[ 1 ].engineName && CWebViewEngineSelection::OUTCOME_NOT_AVAILABLE == selection.candidates[ 1 ].outcome );
        ASSERT_TRUE( "ChromeDevTools" == selection.candidates[ 2 ].engineName && CWebViewEngineSelection::OUTCOME_NOT_AVAILABLE == selection.candidates[ 2 ].outcome );
        ASSERT_TRUE( "SelBasic" == selection.candidates[ 3 ].engineName && CWebViewEngineSelection::OUTCOME_CAPABILITY_MISSING == selection.candidates[ 3 ].outcome );
        ASSERT_TRUE( ( C::CAP_COOKIES | C::CAP_SCRIPT_RESULT ) == selection.candidates[ 3 ].missingCapabilities );
        ASSERT_TRUE( CWebViewEngineSelection::OUTCOME_CHOSEN == selection.candidates[ 4 ].outcome );
        ASSERT_TRUE( selection.Describe().HasSubstr( "SelBasic: capability missing, it lacks Cookies,ScriptResult" ) >= 0 );

        // what the basic engine has: it comes before the full one in that order
        ASSERT_TRUE( CWebViewGlobal::Instance()->FindEngine( C::CAP_OFFSCREEN_FRAMES, order, selection ) && "SelBasic" == selection.engine->GetEngineName() );

        // the last resort engine only when the settings allow it
        CGlobalSettingsScope restore;
        CWebViewGlobalSettings settings = CWebViewGlobal::Instance()->GetSettings();
        settings.allowLastResortEngines = true;
        CWebViewGlobal::Instance()->SetSettings( settings );
        ASSERT_TRUE( CWebViewGlobal::Instance()->FindEngine( C::CAP_OFFSCREEN_FRAMES, order, selection ) && "ChromeDevTools" == selection.engine->GetEngineName() );

        ASSERT_FALSE( CWebViewGlobal::Instance()->FindEngine( C::CAP_GPU_FRAMES, order, selection ) );
        ASSERT_TRUE( selection.engine.IsNULL() );

        registry.Unregister( "SelBasic" );
        registry.Unregister( "SelFull" );
        registry.Unregister( "SelGone" );
        registry.Unregister( "ChromeDevTools" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
