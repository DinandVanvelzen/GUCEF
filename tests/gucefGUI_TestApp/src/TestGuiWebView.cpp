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

#include <string.h>

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_CORE_CLOGSTREAMSCOPE_H
#include "gucefCORE_CLogStreamScope.h"
#define GUCEF_CORE_CLOGSTREAMSCOPE_H
#endif /* GUCEF_CORE_CLOGSTREAMSCOPE_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_WEB_WEBVIEW_H
#include "gucefWEB_WebView.h"
#define GUCEF_WEB_WEBVIEW_H
#endif /* GUCEF_WEB_WEBVIEW_H ? */

#ifndef GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H
#include "gucefGUI_CWebViewInputTranslator.h"
#define GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H
#endif /* GUCEF_GUI_CWEBVIEWINPUTTRANSLATOR_H ? */

#include "gucefGUI_TestAssetFormSupport.h"
#include "TestWebViewSupport.h"
#include "TestFakeWebView.h"
#include "TestFakeWebViewEngine.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

using namespace GUITEST;
using WEBVIEWTEST::CFakeWebViewEngine;
using WEBVIEWTEST::CFakeWebView;
using WEBVIEWTEST::CFakeWebSite;
using WEBVIEWTEST::AwaitOk;
using WEBVIEWTEST::AwaitText;
using WEBVIEWTEST::CreateTestView;
using WEBVIEWTEST::MakeTestSettings;
using WEBVIEWTEST::MakeSignInSettings;

CFakeWebView*
FakeOf( const WEB::TWebViewPtr& view )
{
    return static_cast< CFakeWebView* >( const_cast< WEB::TWebViewPtr& >( view ).GetPointerAlways() );
}

/** Renders frames of the context until the page view shows a frame of its own size, at most a few seconds */
bool
SettlePage( GUI::CGUIContext* context, GUI::CWebPageView* page )
{
    for ( UInt32 i=0; i<600; ++i )
    {
        context->RenderFrame( 16 );
        WEB::TWebViewPtr view = page->GetView();
        if ( view.IsNULL() )
            return false;
        UInt32 w = 0, h = 0, density = 0;
        page->GetBinding().GetImageSize( w, h, density );
        UInt32 rw = 0, rh = 0;
        Float32 rs = 0.0f;
        page->GetRequestedSize( rw, rh, rs );
        UInt32 vw = 0, vh = 0;
        Float32 vs = 0.0f;
        view->GetSize( vw, vh, vs );
        const GUI::CRect& rect = page->GetRect();
        if ( rect.width > 0 && (UInt32) rect.width == rw && (UInt32) rect.height == rh && vw == rw && vh == rh && vs == rs &&
             w == (UInt32) ( rw * rs ) && h == (UInt32) ( rh * rs ) && !page->IsSizeRequestPending() &&
             !view->GetFrameExchange().HasNewFrame() && !view->IsLoading() )
        {
            // one more round: what the page did after its last frame is in
            context->RenderFrame( 16 );
            if ( !view->GetFrameExchange().HasNewFrame() )
                return true;
        }
        MT::PrecisionDelay( 2 );
    }
    return false;
}

/** Renders frames until the fake page has recorded at least 'count' input events */
bool
WaitForInput( GUI::CGUIContext* context, const WEB::TWebViewPtr& view, UInt32 count )
{
    for ( UInt32 i=0; i<500 && FakeOf( view )->GetRecordedInputCount() < count; ++i )
    {
        context->RenderFrame( 16 );
        MT::PrecisionDelay( 2 );
    }
    return FakeOf( view )->GetRecordedInputCount() >= count;
}

/** Waits until the engine thread did the commands posted before: it answers a script after them */
void
SyncWithEngine( const WEB::TWebViewPtr& view )
{
    AwaitText( const_cast< WEB::TWebViewPtr& >( view )->EvaluateScript( "document.title", WEBVIEWTEST::TestWorld() ) );
}

/** Waits until a script finds the text on the page, rendering frames meanwhile */
bool
PageContains( GUI::CGUIContext* context, const WEB::TWebViewPtr& view, const char* text )
{
    for ( UInt32 i=0; i<200; ++i )
    {
        context->RenderFrame( 16 );
        if ( "true" == AwaitText( const_cast< WEB::TWebViewPtr& >( view )->EvaluateScript( WEBVIEWTEST::ContainsTextExpression( text ), WEBVIEWTEST::TestWorld() ) ) )
            return true;
        MT::PrecisionDelay( 2 );
    }
    return false;
}

/** The middle of an element of the fake page in coordinates of the context */
bool
ElementPoint( GUI::CWebPageView* page, const WEB::TWebViewPtr& view, const char* id, Int32& x, Int32& y )
{
    WEB::CWebViewRect rect;
    if ( !FakeOf( view )->GetElementRect( id, rect ) )
        return false;
    x = page->GetRect().left + rect.left + rect.width / 2;
    y = page->GetRect().top + rect.top + rect.height / 2;
    return true;
}

UInt32
CountRecorded( const WEB::TWebViewPtr& view, WEB::TWebViewInputKind kind )
{
    GUCEF::vector< WEB::CWebViewInputEvent > recorded = FakeOf( view )->GetRecordedInput();
    UInt32 count = 0;
    for ( UInt32 i=0; i<(UInt32)recorded.size(); ++i )
        if ( kind == recorded[ i ].kind )
            ++count;
    return count;
}

bool
LastRecorded( const WEB::TWebViewPtr& view, WEB::TWebViewInputKind kind, WEB::CWebViewInputEvent& found )
{
    GUCEF::vector< WEB::CWebViewInputEvent > recorded = FakeOf( view )->GetRecordedInput();
    for ( Int32 i=(Int32)recorded.size()-1; i>=0; --i )
    {
        if ( kind == recorded[ i ].kind )
        {
            found = recorded[ i ];
            return true;
        }
    }
    return false;
}

/** Renders frames until the fake page has recorded at least 'count' input events of a kind: other events may come in between */
bool
WaitForRecorded( GUI::CGUIContext* context, const WEB::TWebViewPtr& view, WEB::TWebViewInputKind kind, UInt32 count )
{
    for ( UInt32 i=0; i<500 && CountRecorded( view, kind ) < count; ++i )
    {
        context->RenderFrame( 16 );
        MT::PrecisionDelay( 2 );
    }
    return CountRecorded( view, kind ) >= count;
}

/** A page view of 320 x 240 at 20,30 of a screen, showing a fake view */
struct SPageFixture
{
    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    CFakeWebViewEngine engine;
    WEB::TWebViewPtr view;
    GUI::CWebPageView* page;

    SPageFixture( void ) : contextPtr(), context( GUCEF_NULL ), engine( "GuiFake" ), view(), page( GUCEF_NULL ) {}

    bool Make( const WEB::CWebViewSettings& settings, const char* path )
    {
        contextPtr = CreateFormContext();
        context = contextPtr.IsNULL() ? GUCEF_NULL : contextPtr->AsGUIContext();
        if ( GUCEF_NULL == context )
            return false;
        GUI::CWidget* screen = MakeScreen( context );
        page = GUI::CWidget::As< GUI::CWebPageView >( context->CreateWidget( "WebPageView", "Page", screen ) );
        if ( GUCEF_NULL == page )
            return false;
        page->SetProperty( "Position", "0,20 0,30" );
        page->SetProperty( "Size", "0,320 0,240" );
        view = CreateTestView( engine, settings );
        if ( view.IsNULL() || ( GUCEF_NULL != path && !AwaitOk( view->Navigate( CFakeWebSite::Url( path ) ) ) ) )
            return false;
        context->FlushChanges();
        return page->Attach( view ) && SettlePage( context, page );
    }

    ~SPageFixture()
    {
        if ( GUCEF_NULL != page )
            page->Detach();
        if ( !view.IsNULL() )
            AwaitOk( view->Close() );
        view.Unlink();
        engine.Shutdown( 5000 );
    }
};

/** The clipboard of the clipboard tests */
CORE::CString g_clipboard;

bool
TestClipboardReader( char* dest, UInt32 size, UInt32* written )
{
    const UInt32 bytes = g_clipboard.IsNULLOrEmpty() ? 0 : g_clipboard.ByteSize() - 1;
    if ( 0 == bytes || size < 2 )
        return false;
    const UInt32 n = bytes < size - 1 ? bytes : size - 1;
    memcpy( dest, g_clipboard.C_String(), n );
    dest[ n ] = '\0';
    if ( GUCEF_NULL != written )
        *written = n;
    return true;
}

bool
TestClipboardWriter( const char* text )
{
    g_clipboard = text;
    return true;
}

/** Answers every decision of the form while it is told, as an owner of a form would */
class CDecisionOwner : public CORE::CObserver
{
    public:

    GUI::CWebViewForm& form;
    bool allow;
    UInt32 answered;

    CDecisionOwner( GUI::CWebViewForm& theForm, bool allowIt ) : CORE::CObserver(), form( theForm ), allow( allowIt ), answered( 0 )
    {
        SubscribeTo( &form, GUI::CWebViewForm::PopupRequestedEvent );
    }

    virtual ~CDecisionOwner() { UnsubscribeAllFromObserver(); }

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUI::CWebViewForm::PopupRequestedEvent != eventId || GUCEF_NULL == eventData )
            return;
        const WEB::CWebViewPopupRequest& request = static_cast< WEB::TWebViewPopupRequestEventData* >( eventData )->GetData();
        if ( form.ResolveDecision( request.decisionId, allow ) )
            ++answered;
    }
};

/** A logger that keeps the messages of the user log */
class CUserLogRecorder : public CORE::CILogger
{
    public:

    CORE::CString text;

    CUserLogRecorder( void ) : CORE::CILogger(), text() {}

    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CString& message, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message ); }
    virtual void Log( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CVariantStream& message, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message.ToString() ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CString& message, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType logMsgType, const Int32, const CORE::CVariantStream& message, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { Keep( logMsgType, message.ToString() ); }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 logLevel ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    void Keep( const CORE::TLogMsgType logMsgType, const CORE::CString& message ) { if ( CORE::LOG_USER == logMsgType ) text += message + "\n"; }
};

/** Calls the presenter from a thread of its own, as a sign-in task would */
class CPresentingThread : public MT::CActiveObject
{
    public:

    WEB::CIWebViewPresenter* presenter;
    WEB::TWebViewPtr view;
    MT::CTAtomic< UInt32 > done;

    CPresentingThread( void ) : MT::CActiveObject(), presenter( GUCEF_NULL ), view(), done( 0 ) {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        WEB::CWebViewPresentation presentation;
        presentation.title = "Sign in";
        presentation.prompt = "Sign in to the test site";
        presenter->Present( view, presentation );
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

/** A form scene for the render tests: a web view form in a screen, its page settled */
class CWebFormScene : public CFormScene
{
    public:

    CFakeWebViewEngine engine;
    WEB::TWebViewPtr view;
    GUI::CWebViewForm form;

    explicit CWebFormScene( GUI::CGUIContext* context ) : CFormScene(), engine( "SceneFake" ), view(), form( context ) {}

    virtual ~CWebFormScene()
    {
        form.Detach();
        if ( !view.IsNULL() )
            AwaitOk( view->Close() );
        view.Unlink();
        engine.Shutdown( 5000 );
    }
};

/** The form with its chrome around the sign in page of the test site */
CFormScene*
BuildBrowserScene( GUI::CGUIContext* context )
{
    CWebFormScene* scene = GUCEF_NEW CWebFormScene( context );
    GUI::CWidget* screen = MakeScreen( context );
    scene->view = CreateTestView( scene->engine, MakeTestSettings( "scene" ) );
    if ( scene->view.IsNULL() || !scene->form.Build( screen ) || !AwaitOk( scene->view->Navigate( CFakeWebSite::Url( "/login" ) ) ) )
    {
        delete scene;
        return GUCEF_NULL;
    }
    scene->form.Attach( scene->view );
    context->FlushChanges();
    SettlePage( context, scene->form.GetPageView() );
    // typed into a field after the first frames were drawn: the image gets only the field, a driver uploads only that part
    AwaitText( scene->view->EvaluateScript( WEBVIEWTEST::FocusAndClearExpression( "#user" ), WEBVIEWTEST::TestWorld() ) );
    scene->view->InjectInput( WEB::CWebViewInputEvent::Text( "tester" ) );
    SyncWithEngine( scene->view );
    SettlePage( context, scene->form.GetPageView() );
    return scene;
}

/** The form as a sign-in shows it: no navigation bar, a prompt, Cancel; the colour page */
CFormScene*
BuildSignInScene( GUI::CGUIContext* context )
{
    CWebFormScene* scene = GUCEF_NEW CWebFormScene( context );
    GUI::CWidget* screen = MakeScreen( context );
    scene->view = CreateTestView( scene->engine, MakeSignInSettings( "scene-signin" ) );
    if ( scene->view.IsNULL() || !scene->form.Build( screen ) || !AwaitOk( scene->view->Navigate( CFakeWebSite::Url( "/colours" ) ) ) )
    {
        delete scene;
        return GUCEF_NULL;
    }
    WEB::CWebViewPresentation presentation;
    presentation.prompt = "Sign in to the test site to continue";
    presentation.showDialogButtons = true;
    scene->form.ApplyPresentation( presentation );
    scene->form.Attach( scene->view );
    context->FlushChanges();
    SettlePage( context, scene->form.GetPageView() );
    return scene;
}

}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/


void
PerformGuiWebViewTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGUI.WebView" );
    WEB::CWebViewGlobal::Instance();

    GUCEF_TESTFW_TESTCASE( "WebViewInputTranslator: every INPUT::KeyCode has a web key code, or an explicit Unidentified" )
    try
    {
        // every key code of gucefINPUT, as the GUI can produce them
        const INPUT::KeyCode inputKeyCodes[] =
        {
            INPUT::KEYCODE_UNASSIGNED,
            INPUT::KEYCODE_ESCAPE,
            INPUT::KEYCODE_1,
            INPUT::KEYCODE_2,
            INPUT::KEYCODE_3,
            INPUT::KEYCODE_4,
            INPUT::KEYCODE_5,
            INPUT::KEYCODE_6,
            INPUT::KEYCODE_7,
            INPUT::KEYCODE_8,
            INPUT::KEYCODE_9,
            INPUT::KEYCODE_0,
            INPUT::KEYCODE_MINUS,
            INPUT::KEYCODE_EQUALS,
            INPUT::KEYCODE_BACK,
            INPUT::KEYCODE_TAB,
            INPUT::KEYCODE_Q,
            INPUT::KEYCODE_W,
            INPUT::KEYCODE_E,
            INPUT::KEYCODE_R,
            INPUT::KEYCODE_T,
            INPUT::KEYCODE_Y,
            INPUT::KEYCODE_U,
            INPUT::KEYCODE_I,
            INPUT::KEYCODE_O,
            INPUT::KEYCODE_P,
            INPUT::KEYCODE_LBRACKET,
            INPUT::KEYCODE_RBRACKET,
            INPUT::KEYCODE_RETURN,
            INPUT::KEYCODE_LCONTROL,
            INPUT::KEYCODE_A,
            INPUT::KEYCODE_S,
            INPUT::KEYCODE_D,
            INPUT::KEYCODE_F,
            INPUT::KEYCODE_G,
            INPUT::KEYCODE_H,
            INPUT::KEYCODE_J,
            INPUT::KEYCODE_K,
            INPUT::KEYCODE_L,
            INPUT::KEYCODE_SEMICOLON,
            INPUT::KEYCODE_APOSTROPHE,
            INPUT::KEYCODE_GRAVE,
            INPUT::KEYCODE_LSHIFT,
            INPUT::KEYCODE_BACKSLASH,
            INPUT::KEYCODE_Z,
            INPUT::KEYCODE_X,
            INPUT::KEYCODE_C,
            INPUT::KEYCODE_V,
            INPUT::KEYCODE_B,
            INPUT::KEYCODE_N,
            INPUT::KEYCODE_M,
            INPUT::KEYCODE_COMMA,
            INPUT::KEYCODE_PERIOD,
            INPUT::KEYCODE_SLASH,
            INPUT::KEYCODE_RSHIFT,
            INPUT::KEYCODE_MULTIPLY,
            INPUT::KEYCODE_LMENU,
            INPUT::KEYCODE_SPACE,
            INPUT::KEYCODE_CAPITAL,
            INPUT::KEYCODE_F1,
            INPUT::KEYCODE_F2,
            INPUT::KEYCODE_F3,
            INPUT::KEYCODE_F4,
            INPUT::KEYCODE_F5,
            INPUT::KEYCODE_F6,
            INPUT::KEYCODE_F7,
            INPUT::KEYCODE_F8,
            INPUT::KEYCODE_F9,
            INPUT::KEYCODE_F10,
            INPUT::KEYCODE_NUMLOCK,
            INPUT::KEYCODE_SCROLL,
            INPUT::KEYCODE_NUMPAD7,
            INPUT::KEYCODE_NUMPAD8,
            INPUT::KEYCODE_NUMPAD9,
            INPUT::KEYCODE_SUBTRACT,
            INPUT::KEYCODE_NUMPAD4,
            INPUT::KEYCODE_NUMPAD5,
            INPUT::KEYCODE_NUMPAD6,
            INPUT::KEYCODE_ADD,
            INPUT::KEYCODE_NUMPAD1,
            INPUT::KEYCODE_NUMPAD2,
            INPUT::KEYCODE_NUMPAD3,
            INPUT::KEYCODE_NUMPAD0,
            INPUT::KEYCODE_DECIMAL,
            INPUT::KEYCODE_LEFTWINKEY,
            INPUT::KEYCODE_RIGHTWINKEY,
            INPUT::KEYCODE_OEM_102,
            INPUT::KEYCODE_F11,
            INPUT::KEYCODE_F12,
            INPUT::KEYCODE_F13,
            INPUT::KEYCODE_F14,
            INPUT::KEYCODE_F15,
            INPUT::KEYCODE_KANA,
            INPUT::KEYCODE_ABNT_C1,
            INPUT::KEYCODE_CONVERT,
            INPUT::KEYCODE_NOCONVERT,
            INPUT::KEYCODE_YEN,
            INPUT::KEYCODE_ABNT_C2,
            INPUT::KEYCODE_NUMPADEQUALS,
            INPUT::KEYCODE_PREVTRACK,
            INPUT::KEYCODE_AT,
            INPUT::KEYCODE_COLON,
            INPUT::KEYCODE_UNDERLINE,
            INPUT::KEYCODE_KANJI,
            INPUT::KEYCODE_STOP,
            INPUT::KEYCODE_AX,
            INPUT::KEYCODE_UNLABELED,
            INPUT::KEYCODE_NEXTTRACK,
            INPUT::KEYCODE_NUMPADENTER,
            INPUT::KEYCODE_RCONTROL,
            INPUT::KEYCODE_MUTE,
            INPUT::KEYCODE_CALCULATOR,
            INPUT::KEYCODE_PLAYPAUSE,
            INPUT::KEYCODE_MEDIASTOP,
            INPUT::KEYCODE_VOLUMEDOWN,
            INPUT::KEYCODE_VOLUMEUP,
            INPUT::KEYCODE_WEBHOME,
            INPUT::KEYCODE_NUMPADCOMMA,
            INPUT::KEYCODE_DIVIDE,
            INPUT::KEYCODE_SYSRQ,
            INPUT::KEYCODE_RMENU,
            INPUT::KEYCODE_PAUSE,
            INPUT::KEYCODE_HOME,
            INPUT::KEYCODE_UP,
            INPUT::KEYCODE_PGUP,
            INPUT::KEYCODE_LEFT,
            INPUT::KEYCODE_RIGHT,
            INPUT::KEYCODE_END,
            INPUT::KEYCODE_DOWN,
            INPUT::KEYCODE_PGDOWN,
            INPUT::KEYCODE_INSERT,
            INPUT::KEYCODE_DELETE,
            INPUT::KEYCODE_LWIN,
            INPUT::KEYCODE_RWIN,
            INPUT::KEYCODE_APPS,
            INPUT::KEYCODE_POWER,
            INPUT::KEYCODE_SLEEP,
            INPUT::KEYCODE_WAKE,
            INPUT::KEYCODE_WEBSEARCH,
            INPUT::KEYCODE_WEBFAVORITES,
            INPUT::KEYCODE_WEBREFRESH,
            INPUT::KEYCODE_WEBSTOP,
            INPUT::KEYCODE_WEBFORWARD,
            INPUT::KEYCODE_WEBBACK,
            INPUT::KEYCODE_MYCOMPUTER,
            INPUT::KEYCODE_MAIL,
            INPUT::KEYCODE_MEDIASELECT
        };
        const UInt32 inputKeyCodeCount = (UInt32) ( sizeof( inputKeyCodes ) / sizeof( inputKeyCodes[ 0 ] ) );
        bool defined = true;
        for ( UInt32 i=0; i<inputKeyCodeCount; ++i )
        {
            const INPUT::KeyCode keyCode = inputKeyCodes[ i ];
            const WEB::TWebKeyCode webKeyCode = GUI::CWebViewInputTranslator::GetWebKeyCode( keyCode );
            const bool explicitlyUnidentified = INPUT::KEYCODE_UNASSIGNED == keyCode || INPUT::KEYCODE_AT == keyCode || INPUT::KEYCODE_COLON == keyCode || INPUT::KEYCODE_UNDERLINE == keyCode || INPUT::KEYCODE_KANJI == keyCode || INPUT::KEYCODE_STOP == keyCode || INPUT::KEYCODE_AX == keyCode || INPUT::KEYCODE_UNLABELED == keyCode;
            if ( (Int32) webKeyCode < 0 || (Int32) webKeyCode >= (Int32) WEB::WEBKEYCODE_COUNT ||
                 ( WEB::WEBKEYCODE_UNIDENTIFIED == webKeyCode ) != explicitlyUnidentified )
            {
                defined = false;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "INPUT key code " << (Int32) keyCode << " gives web key code " << (Int32) webKeyCode );
            }
        }
        ASSERT_TRUE( defined && inputKeyCodeCount > 140 );

        // the values between the key codes, and those out of range, are Unidentified
        bool gapsUnidentified = true;
        for ( Int32 value=-1; value<=(Int32) INPUT::KEYCODE_MAX + 1; ++value )
        {
            bool isKeyCode = false;
            for ( UInt32 i=0; i<inputKeyCodeCount; ++i )
                isKeyCode = isKeyCode || (Int32) inputKeyCodes[ i ] == value;
            if ( !isKeyCode && WEB::WEBKEYCODE_UNIDENTIFIED != GUI::CWebViewInputTranslator::GetWebKeyCode( (INPUT::KeyCode) value ) )
                gapsUnidentified = false;
        }
        ASSERT_TRUE( gapsUnidentified );

        // the physical keys, also of keys gucefINPUT has two names for
        ASSERT_TRUE( WEB::WEBKEYCODE_KEY_Q == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_Q ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_DIGIT0 == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_0 ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_QUOTE == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_APOSTROPHE ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_NUMPAD_ENTER == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_NUMPADENTER ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_ALT_RIGHT == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_RMENU ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_META_LEFT == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_LWIN ) &&
                     WEB::WEBKEYCODE_META_LEFT == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_LEFTWINKEY ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_INTL_BACKSLASH == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_OEM_102 ) );
        ASSERT_TRUE( WEB::WEBKEYCODE_ARROW_LEFT == GUI::CWebViewInputTranslator::GetWebKeyCode( INPUT::KEYCODE_LEFT ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewInputTranslator: key values of the US layout, modifier flags, mouse buttons and held buttons in DOM numbering" )
    try
    {
        WEB::CWebViewInputEvent a = GUI::CWebViewInputTranslator::KeyEvent( INPUT::KEYCODE_A, true, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( WEB::WEBVIEWINPUT_KEY_DOWN == a.kind && WEB::WEBKEYCODE_KEY_A == a.code && a.IsCharacterKey() && CORE::CString( "A" ) == a.keyText );
        ASSERT_TRUE( WEB::WEBMODIFIER_SHIFT == a.modifiers );
        WEB::CWebViewInputEvent capsLock = GUI::CWebViewInputTranslator::KeyEvent( INPUT::KEYCODE_A, false, INPUT::KEYMOD_CAPSLOCK );
        ASSERT_TRUE( WEB::WEBVIEWINPUT_KEY_UP == capsLock.kind && CORE::CString( "A" ) == capsLock.keyText && WEB::WEBMODIFIER_CAPSLOCK == capsLock.modifiers );
        WEB::CWebViewInputEvent tab = GUI::CWebViewInputTranslator::KeyEvent( INPUT::KEYCODE_TAB, true, INPUT::KEYMOD_SHIFT );
        ASSERT_TRUE( WEB::WEBKEYCODE_TAB == tab.code && WEB::WEBNAMEDKEY_TAB == tab.namedKey && !tab.IsCharacterKey() );
        WEB::CWebViewInputEvent unknown = GUI::CWebViewInputTranslator::KeyEvent( INPUT::KEYCODE_AX, true, 0 );
        ASSERT_TRUE( WEB::WEBKEYCODE_UNIDENTIFIED == unknown.code && WEB::WEBNAMEDKEY_UNIDENTIFIED == unknown.namedKey );

        const UInt32 allInput = INPUT::KEYMOD_SHIFT | INPUT::KEYMOD_CTRL | INPUT::KEYMOD_ALT | INPUT::KEYMOD_META | INPUT::KEYMOD_CAPSLOCK | INPUT::KEYMOD_NUMLOCK | INPUT::KEYMOD_SCROLLLOCK;
        const UInt32 allWeb = WEB::WEBMODIFIER_SHIFT | WEB::WEBMODIFIER_CONTROL | WEB::WEBMODIFIER_ALT | WEB::WEBMODIFIER_META | WEB::WEBMODIFIER_CAPSLOCK | WEB::WEBMODIFIER_NUMLOCK | WEB::WEBMODIFIER_SCROLLLOCK;
        ASSERT_TRUE( allWeb == GUI::CWebViewInputTranslator::GetWebModifiers( allInput ) && 0 == GUI::CWebViewInputTranslator::GetWebModifiers( 0 ) );
        ASSERT_TRUE( WEB::WEBMODIFIER_CONTROL == GUI::CWebViewInputTranslator::GetWebModifiers( INPUT::KEYMOD_CTRL ) );

        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_NONE == GUI::CWebViewInputTranslator::GetWebMouseButton( GUI::MOUSEBUTTON_NONE ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_LEFT == GUI::CWebViewInputTranslator::GetWebMouseButton( GUI::MOUSEBUTTON_LEFT ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_RIGHT == GUI::CWebViewInputTranslator::GetWebMouseButton( GUI::MOUSEBUTTON_RIGHT ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_MIDDLE == GUI::CWebViewInputTranslator::GetWebMouseButton( GUI::MOUSEBUTTON_MIDDLE ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_BACK == GUI::CWebViewInputTranslator::GetWebMouseButton( GUI::MOUSEBUTTON_X1 ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_FORWARD == GUI::CWebViewInputTranslator::GetWebMouseButton( GUI::MOUSEBUTTON_X2 ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTON_NONE == GUI::CWebViewInputTranslator::GetWebMouseButton( (GUI::TMouseButton) 99 ) );

        // held buttons: the GUI has a bit per button number, the DOM has right as 2 and middle as 4
        const UInt32 guiLeftMiddle = ( 1u << GUI::MOUSEBUTTON_LEFT ) | ( 1u << GUI::MOUSEBUTTON_MIDDLE );
        ASSERT_TRUE( ( WEB::WEBMOUSEBUTTONFLAG_LEFT | WEB::WEBMOUSEBUTTONFLAG_MIDDLE ) == GUI::CWebViewInputTranslator::GetWebMouseButtonStates( guiLeftMiddle ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTONFLAG_RIGHT == GUI::CWebViewInputTranslator::GetWebMouseButtonStates( 1u << GUI::MOUSEBUTTON_RIGHT ) );
        ASSERT_TRUE( WEB::WEBMOUSEBUTTONFLAG_FORWARD == GUI::CWebViewInputTranslator::GetWebMouseButtonStates( 1u << GUI::MOUSEBUTTON_X2 ) );
        ASSERT_TRUE( 0 == GUI::CWebViewInputTranslator::GetWebMouseButtonStates( 1u << GUI::MOUSEBUTTON_NONE ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: the page is shown at the size of the widget; the view is told the size once; the image is the frame" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/colours" ) );
        UInt32 w = 0, h = 0;
        Float32 s = 0.0f;
        f.view->GetSize( w, h, s );
        ASSERT_TRUE( 320 == w && 240 == h && 1.0f == s );
        ASSERT_TRUE( 1 == f.page->GetSizeRequestCount() );         // created at 320 x 240 already: told once
        UInt32 iw = 0, ih = 0, density = 0;
        f.page->GetBinding().GetImageSize( iw, ih, density );
        ASSERT_TRUE( 320 == iw && 240 == ih && 1 == density );
        ASSERT_TRUE( f.context->HasImage( f.page->GetImageName() ) );

        // the context draws the page: the quadrants of the colour page where the widget is
        IMAGE::TPixelMapPtr frame;
        ASSERT_TRUE( f.context->CaptureFrame( frame ) && !frame.IsNULL() );
        const Int32 probes[ 4 ][ 2 ] = { { 20 + 80, 30 + 60 }, { 20 + 240, 30 + 60 }, { 20 + 80, 30 + 180 }, { 20 + 240, 30 + 180 } };
        for ( UInt32 q=0; q<4; ++q )
        {
            const UInt8* c = CFakeWebSite::QuadrantColours[ q ];
            ASSERT_TRUE( WEBVIEWTEST::IsColour( WEBVIEWTEST::PixelOf( *frame, probes[ q ][ 0 ], probes[ q ][ 1 ] ), c[ 0 ], c[ 1 ], c[ 2 ] ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: a change of a field copies and uploads that part only, not the frame" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/login" ) );
        const UInt64 copiedBefore = f.page->GetBinding().GetCopiedPixelCount();
        const UInt32 registrations = f.page->GetBinding().GetRegistrationCount();
        ASSERT_TRUE( "true" == AwaitText( f.view->EvaluateScript( WEBVIEWTEST::FocusAndClearExpression( "#user" ), WEBVIEWTEST::TestWorld() ) ) );
        f.view->InjectInput( WEB::CWebViewInputEvent::Text( "abc" ) );
        SyncWithEngine( f.view );
        ASSERT_TRUE( SettlePage( f.context, f.page ) );
        const UInt64 copied = f.page->GetBinding().GetCopiedPixelCount() - copiedBefore;
        ASSERT_TRUE( copied > 0 && copied <= 300 * 28 * 2 );        // the field (twice: the focus, then the text), not 320 x 240
        ASSERT_TRUE( registrations == f.page->GetBinding().GetRegistrationCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: pointer, double clicks, wheel, keys and text go to the view in view coordinates; Tab stays in the page" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/login" ) );
        Int32 x = 0, y = 0;
        ASSERT_TRUE( ElementPoint( f.page, f.view, "user", x, y ) );
        const UInt32 before = FakeOf( f.view )->GetRecordedInputCount();
        f.context->InjectMouseMove( x, y );
        f.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        f.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        f.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        f.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        ASSERT_TRUE( WaitForInput( f.context, f.view, before + 5 ) && WaitForRecorded( f.context, f.view, WEB::WEBVIEWINPUT_POINTER_UP, 2 ) );
        WEB::CWebViewInputEvent down;
        ASSERT_TRUE( LastRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_DOWN, down ) );
        ASSERT_TRUE( x - 20 == down.x && y - 30 == down.y && WEB::WEBMOUSEBUTTON_LEFT == down.button && 2 == down.clickCount );
        ASSERT_TRUE( f.context->GetFocus() == f.page );
        ASSERT_TRUE( "user" == FakeOf( f.view )->GetFocusedElement() );

        f.context->InjectMouseWheel( -2 );
        WEB::CWebViewInputEvent wheel;
        ASSERT_TRUE( WaitForRecorded( f.context, f.view, WEB::WEBVIEWINPUT_WHEEL, 1 ) && LastRecorded( f.view, WEB::WEBVIEWINPUT_WHEEL, wheel ) && -2.0f == wheel.wheelNotchesY );

        // text and keys while the page has the focus; Tab moves the focus in the page, not in the GUI
        f.context->InjectText( "h\xC3\xA9" );
        PressKey( f.context, INPUT::KEYCODE_TAB );
        PressKey( f.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( WaitForRecorded( f.context, f.view, WEB::WEBVIEWINPUT_TEXT, 1 ) && WaitForRecorded( f.context, f.view, WEB::WEBVIEWINPUT_KEY_UP, 2 ) );
        WEB::CWebViewInputEvent text;
        ASSERT_TRUE( LastRecorded( f.view, WEB::WEBVIEWINPUT_TEXT, text ) && "h\xC3\xA9" == text.text );
        WEB::CWebViewInputEvent key;
        ASSERT_TRUE( LastRecorded( f.view, WEB::WEBVIEWINPUT_KEY_UP, key ) && WEB::WEBKEYCODE_ESCAPE == key.code && WEB::WEBNAMEDKEY_ESCAPE == key.namedKey );
        ASSERT_TRUE( 2 == CountRecorded( f.view, WEB::WEBVIEWINPUT_KEY_DOWN ) && 2 == CountRecorded( f.view, WEB::WEBVIEWINPUT_KEY_UP ) );
        ASSERT_TRUE( f.context->GetFocus() == f.page );
        ASSERT_TRUE( "password" == FakeOf( f.view )->GetFocusedElement() );

        // F6 is not the page's
        PressKey( f.context, INPUT::KEYCODE_F6 );
        f.context->RenderFrame( 16 );
        ASSERT_TRUE( 2 == CountRecorded( f.view, WEB::WEBVIEWINPUT_KEY_DOWN ) );

        // the pointer leaves the widget: the page hears it
        f.context->InjectMouseMove( 5, 5 );
        ASSERT_TRUE( WaitForInput( f.context, f.view, FakeOf( f.view )->GetRecordedInputCount() ) );
        f.context->RenderFrame( 16 );
        WEB::CWebViewInputEvent leave;
        for ( UInt32 i=0; i<100 && !LastRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_LEAVE, leave ); ++i )
        {
            f.context->RenderFrame( 16 );
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( LastRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_LEAVE, leave ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: a press captures the pointer; a lost capture sends the release and pointer-leave; focus is told to the view" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/login" ) );
        Int32 x = 0, y = 0;
        ASSERT_TRUE( ElementPoint( f.page, f.view, "user", x, y ) );
        f.context->InjectMouseMove( x, y );
        f.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        f.context->InjectMouseMove( 600, 470 );                    // dragged out of the widget: still the page's
        // the press also gives the page the focus: wait for the second move itself, not for a number of events
        ASSERT_TRUE( WaitForInput( f.context, f.view, 3 ) && WaitForRecorded( f.context, f.view, WEB::WEBVIEWINPUT_POINTER_MOVE, 2 ) );
        WEB::CWebViewInputEvent move;
        ASSERT_TRUE( LastRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_MOVE, move ) && 600 - 20 == move.x && 470 - 30 == move.y );
        const UInt32 upsBefore = CountRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_UP );
        f.page->SetVisibility( false );                             // the press ends without a release
        f.context->RenderFrame( 16 );
        for ( UInt32 i=0; i<100 && CountRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_UP ) == upsBefore; ++i )
        {
            f.context->RenderFrame( 16 );
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( upsBefore + 1 == CountRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_UP ) );
        // the hide posted the release, the pointer-leave, the focus command and the focus-out in that order: the engine
        // thread has done all of them only after a command posted later, seeing the release is not enough
        SyncWithEngine( f.view );
        ASSERT_TRUE( CountRecorded( f.view, WEB::WEBVIEWINPUT_POINTER_LEAVE ) >= 1 );
        ASSERT_TRUE( CountRecorded( f.view, WEB::WEBVIEWINPUT_FOCUS_IN ) >= 1 );
        // the hidden widget lost the keyboard focus, and the view is told
        ASSERT_TRUE( f.context->GetFocus() != f.page );
        ASSERT_TRUE( !FakeOf( f.view )->HasPageFocus() && CountRecorded( f.view, WEB::WEBVIEWINPUT_FOCUS_OUT ) >= 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: the pointer shape of the page while over the widget, the context's again after" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/login" ) );
        Int32 x = 0, y = 0;
        ASSERT_TRUE( ElementPoint( f.page, f.view, "submit", x, y ) );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == f.context->GetSystemCursorShape() );
        f.context->InjectMouseMove( x, y );
        for ( UInt32 i=0; i<200 && GUI::CURSORSHAPE_HAND != f.context->GetSystemCursorShape(); ++i )
        {
            f.context->RenderFrame( 16 );
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( GUI::CURSORSHAPE_HAND == f.context->GetSystemCursorShape() );
        ASSERT_TRUE( ElementPoint( f.page, f.view, "user", x, y ) );
        f.context->InjectMouseMove( x, y );
        for ( UInt32 i=0; i<200 && GUI::CURSORSHAPE_TEXT != f.context->GetSystemCursorShape(); ++i )
        {
            f.context->RenderFrame( 16 );
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( GUI::CURSORSHAPE_TEXT == f.context->GetSystemCursorShape() );
        f.context->InjectMouseMove( 600, 5 );
        f.context->RenderFrame( 16 );
        ASSERT_TRUE( GUI::CURSORSHAPE_ARROW == f.context->GetSystemCursorShape() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: resizes are coalesced (one in flight, the newest wins); the render scale is the device scale" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/landing" ) );
        const UInt32 requests = f.page->GetSizeRequestCount();
        const char* sizes[] = { "0,300 0,200", "0,310 0,210", "0,330 0,250", "0,400 0,300" };
        for ( UInt32 i=0; i<4; ++i )
        {
            f.page->SetProperty( "Size", sizes[ i ] );
            f.context->FlushChanges();
        }
        f.context->RenderFrame( 16 );
        ASSERT_TRUE( requests + 1 == f.page->GetSizeRequestCount() );   // the steps between frames were never sent
        ASSERT_TRUE( SettlePage( f.context, f.page ) );
        UInt32 w = 0, h = 0, density = 0;
        Float32 s = 0.0f;
        f.view->GetSize( w, h, s );
        ASSERT_TRUE( 400 == w && 300 == h );
        f.page->GetBinding().GetImageSize( w, h, density );
        ASSERT_TRUE( 400 == w && 300 == h && 1 == density );

        // a context at twice the pixels: the view renders at density 2, the image keeps its logical size
        f.context->SetRenderScale( 2 );
        f.context->FlushChanges();
        ASSERT_TRUE( SettlePage( f.context, f.page ) );
        f.view->GetSize( w, h, s );
        ASSERT_TRUE( 400 == w && 300 == h && 2.0f == s );
        f.page->GetBinding().GetImageSize( w, h, density );
        ASSERT_TRUE( 800 == w && 600 == h && 2 == density );
        Int32 lw = 0, lh = 0;
        ASSERT_TRUE( f.context->GetImageLogicalSize( f.page->GetImageName(), lw, lh ) && 400 == lw && 300 == lh );
        f.context->SetRenderScale( 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebPageView: an engine without a clipboard of its own gets Ctrl+V as text, Ctrl+C as an edit command whose text is copied" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/login" ) );
        GUI::CSecretInput::SetClipboardReader( &TestClipboardReader );
        GUI::CSecretInput::SetClipboardWriter( &TestClipboardWriter );
        g_clipboard = "pasted";
        Int32 x = 0, y = 0;
        ASSERT_TRUE( ElementPoint( f.page, f.view, "user", x, y ) );
        ClickAt( f.context, x, y );
        PressKey( f.context, INPUT::KEYCODE_V, INPUT::KEYMOD_CTRL );
        for ( UInt32 i=0; i<200 && "\"pasted\"" != AwaitText( f.view->EvaluateScript( "fake:value(#user)", WEBVIEWTEST::TestWorld() ) ); ++i )
            f.context->RenderFrame( 16 );
        ASSERT_TRUE( "\"pasted\"" == AwaitText( f.view->EvaluateScript( "fake:value(#user)", WEBVIEWTEST::TestWorld() ) ) );
        ASSERT_TRUE( 0 == CountRecorded( f.view, WEB::WEBVIEWINPUT_KEY_DOWN ) );     // not passed on as keys
        f.view->InjectInput( WEB::CWebViewInputEvent::Text( "-more" ) );
        g_clipboard.Clear();
        PressKey( f.context, INPUT::KEYCODE_C, INPUT::KEYMOD_CTRL );
        for ( UInt32 i=0; i<200 && g_clipboard.IsNULLOrEmpty(); ++i )
        {
            f.context->RenderFrame( 16 );
            MT::PrecisionDelay( 2 );
        }
        ASSERT_TRUE( "pasted-more" == g_clipboard );
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
    }
    catch ( ... )
    {
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewImageBinding: the frames of a view as an image of the context, offscreen, without a widget" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        CFakeWebViewEngine engine( "BindingFake" );
        WEB::TWebViewPtr view = CreateTestView( engine, MakeTestSettings( "binding", 200, 100 ) );
        ASSERT_TRUE( AwaitOk( view->Navigate( CFakeWebSite::Url( "/colours" ) ) ) );
        {
            GUI::CWebViewImageBinding binding;
            CEventRecorder updates( &binding, GUI::CWebViewImageBinding::ImageUpdatedEvent );
            ASSERT_TRUE( binding.Bind( context, "offscreen.page", view ) );
            for ( UInt32 i=0; i<100 && view->GetFrameExchange().HasNewFrame(); ++i )
                context->RenderFrame( 16 );
            ASSERT_TRUE( binding.HasImage() && context->HasImage( "offscreen.page" ) );
            IMAGE::TPixelMapPtr pixels;
            UInt32 revision = 0;
            ASSERT_TRUE( context->ResolveImage( "offscreen.page", pixels, revision ) && 200 == pixels->GetWidthInPixels() && 100 == pixels->GetHeightInPixels() );
            ASSERT_TRUE( WEBVIEWTEST::IsColour( WEBVIEWTEST::PixelOf( *pixels, 150, 75 ), CFakeWebSite::QuadrantColours[ 3 ][ 0 ], CFakeWebSite::QuadrantColours[ 3 ][ 1 ], CFakeWebSite::QuadrantColours[ 3 ][ 2 ] ) );
            ASSERT_TRUE( updates.count >= 1 );
            binding.Unbind();
            ASSERT_FALSE( context->HasImage( "offscreen.page" ) );
        }
        AwaitOk( view->Close() );
        engine.Shutdown( 5000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewForm: chrome follows the page: title, address, security, loading line, progress; failures show the error panel" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        CFakeWebViewEngine engine( "FormFake" );
        WEB::TWebViewPtr view = CreateTestView( engine, MakeSignInSettings( "form" ) );
        ASSERT_TRUE( !view.IsNULL() );
        {
            GUI::CWebViewForm form( context );
            CEventRecorder titles( &form, GUI::CWebViewForm::TitleChangedEvent );
            CEventRecorder loads( &form, GUI::CWebViewForm::LoadStateChangedEvent );
            CEventRecorder failures( &form, GUI::CWebViewForm::LoadFailedEvent );
            ASSERT_TRUE( form.Build( "Browser" ) );
            ASSERT_TRUE( HasWidget( form, "NavigationBar" ) && HasWidget( form, "Address" ) && HasWidget( form, "Security" ) && HasWidget( form, "Page" ) &&
                         HasWidget( form, "StatusBar" ) && HasWidget( form, "LoadingOverlay" ) && HasWidget( form, "ErrorPanel" ) && HasWidget( form, "Cancel" ) );
            ASSERT_TRUE( form.Attach( view ) );
            form.Navigate( CFakeWebSite::Url( "/login?state=secret-state" ) );
            ASSERT_TRUE( SettlePage( context, form.GetPageView() ) );
            ASSERT_TRUE( titles.count >= 1 && "Sign in" == titles.Last< CORE::TCloneableString >()->GetData() );
            ASSERT_TRUE( loads.count >= 2 && !loads.Last< CORE::TCloneableBool >()->GetData() );
            ASSERT_TRUE( "https://app.test/login?state=secret-state" == form.GetWidget( "Address" )->GetProperty( "Text" ) );
            ASSERT_TRUE( form.GetWidget( "Security" )->IsVisible() && "Secure connection" == form.GetWidget( "Security" )->GetProperty( "Tooltip" ) );
            ASSERT_TRUE( "Sign in" == form.GetPageView()->GetProperty( "Tooltip" ) );
            ASSERT_FALSE( form.IsLoadingOverlayShown() || form.IsErrorShown() );
            ASSERT_TRUE( "100" == form.GetWidget( "Progress" )->GetProperty( "Value" ) );
            ASSERT_TRUE( context->GetFocus() == form.GetPageView() );

            // the address of a sign-in shows no query
            form.SetAddressEditable( false );
            ASSERT_TRUE( "https://app.test/login" == form.GetWidget( "Address" )->GetProperty( "Text" ) );

            // a site outside the allow-list: the error panel instead of the page, no Retry
            form.Navigate( "https://elsewhere.example/" );
            for ( UInt32 i=0; i<200 && failures.count == 0; ++i )
            {
                context->RenderFrame( 16 );
                MT::PrecisionDelay( 2 );
            }
            ASSERT_TRUE( 1 == failures.count && form.IsErrorShown() );
            ASSERT_FALSE( form.GetPageView()->IsVisible() || form.GetWidget( "Retry" )->IsVisible() );
            ASSERT_TRUE( form.GetWidget( "ErrorText" )->GetProperty( "Text" ).HasSubstr( "elsewhere.example" ) >= 0 );

            // Back from the error panel: the page again
            ClickWidget( context, form.GetWidget( "ErrorBack" ) );
            context->RenderFrame( 16 );
            ASSERT_FALSE( form.IsErrorShown() );
            ASSERT_TRUE( form.GetPageView()->IsVisible() );
            form.Detach();
        }
        AwaitOk( view->Close() );
        engine.Shutdown( 5000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewForm: F6 between page and chrome, a page tabbing past its end gives the focus to the chrome, Escape on the chrome cancels" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        CFakeWebViewEngine engine( "FocusFake" );
        WEB::TWebViewPtr view = CreateTestView( engine, MakeTestSettings( "focus" ) );
        {
            GUI::CWebViewForm form( context );
            CEventRecorder cancels( &form, GUI::CWebViewForm::CancelledEvent );
            form.SetShowDialogButtons( true );
            ASSERT_TRUE( form.Build( "Browser" ) && form.Attach( view ) );
            AwaitOk( view->Navigate( CFakeWebSite::Url( "/login" ) ) );
            ASSERT_TRUE( SettlePage( context, form.GetPageView() ) );
            ASSERT_TRUE( context->GetFocus() == form.GetPageView() );
            PressKey( context, INPUT::KEYCODE_F6 );
            ASSERT_TRUE( context->GetFocus() == form.GetWidget( "Cancel" ) );
            PressKey( context, INPUT::KEYCODE_F6 );
            ASSERT_TRUE( context->GetFocus() == form.GetPageView() );

            // Escape in the page is the page's
            PressKey( context, INPUT::KEYCODE_ESCAPE );
            ASSERT_TRUE( WaitForInput( context, view, 2 ) );
            ASSERT_TRUE( 0 == cancels.count );

            // Tab past the last element of the page: the chrome gets the focus
            Int32 x = 0, y = 0;
            ASSERT_TRUE( ElementPoint( form.GetPageView(), view, "submit", x, y ) );
            context->InjectMouseMove( x, y );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
            context->RenderFrame( 600 );
            PressKey( context, INPUT::KEYCODE_TAB );
            for ( UInt32 i=0; i<200 && context->GetFocus() == form.GetPageView(); ++i )
            {
                context->RenderFrame( 16 );
                MT::PrecisionDelay( 2 );
            }
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            ASSERT_TRUE( context->GetFocus() == form.GetWidget( "Cancel" ) );

            // Escape on the chrome: cancelled
            PressKey( context, INPUT::KEYCODE_ESCAPE );
            ASSERT_TRUE( 1 == cancels.count );
            ClickWidget( context, form.GetWidget( "Cancel" ) );
            ASSERT_TRUE( 2 == cancels.count );
            form.Detach();
        }
        AwaitOk( view->Close() );
        engine.Shutdown( 5000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewForm: decisions the owner answers while told, and those it leaves to the form, which asks with a message box" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        CFakeWebViewEngine engine( "DecisionFake" );
        WEB::CWebViewSettings settings = MakeTestSettings( "decisions" );
        settings.policy.popups = WEB::WEBVIEWPOPUPS_ASK;
        WEB::TWebViewPtr view = CreateTestView( engine, settings );
        {
            GUI::CWebViewForm form( context );
            ASSERT_TRUE( form.Build( "Browser" ) && form.Attach( view ) );
            {
                CDecisionOwner owner( form, true );
                form.Navigate( CFakeWebSite::Url( "/popup" ) );
                for ( UInt32 i=0; i<300 && CFakeWebSite::Url( "/landing" ) != view->GetUrl(); ++i )
                {
                    context->RenderFrame( 16 );
                    MT::PrecisionDelay( 2 );
                }
                ASSERT_TRUE( 1 == owner.answered && CFakeWebSite::Url( "/landing" ) == view->GetUrl() );
                ASSERT_TRUE( GUCEF_NULL == form.GetOpenPrompt() );
            }

            // nobody answers: the form asks the person
            CEventRecorder popups( &form, GUI::CWebViewForm::PopupRequestedEvent );
            form.Navigate( CFakeWebSite::Url( "/popup" ) );
            for ( UInt32 i=0; i<300 && GUCEF_NULL == form.GetOpenPrompt(); ++i )
            {
                context->RenderFrame( 16 );
                MT::PrecisionDelay( 2 );
            }
            ASSERT_TRUE( 1 == popups.count && GUCEF_NULL != form.GetOpenPrompt() );
            ASSERT_TRUE( form.GetOpenPrompt()->GetWidget( "Text" )->GetProperty( "Text" ).HasSubstr( "https://app.test/landing" ) >= 0 );
            form.GetOpenPrompt()->Choose( GUI::CMessageBoxForm::MESSAGEBOXRESULT_NO );
            context->RenderFrame( 16 );
            ASSERT_TRUE( GUCEF_NULL == form.GetOpenPrompt() && 0 == form.GetPendingPromptCount() );
            // the refusal is done on the engine thread: wait for its answer, then for what it did after it
            for ( UInt32 i=0; i<300 && FakeOf( view )->GetAnswers().size() < 2; ++i )
            {
                context->RenderFrame( 16 );
                MT::PrecisionDelay( 2 );
            }
            SyncWithEngine( view );
            context->RenderFrame( 16 );
            ASSERT_TRUE( CFakeWebSite::Url( "/popup" ) == view->GetUrl() );
            CORE::CString::StringVector answers = FakeOf( view )->GetAnswers();
            ASSERT_TRUE( 2 == answers.size() && "popup:allowed" == answers[ 0 ] && "popup:refused" == answers[ 1 ] );

            // script dialogs one after the other, the prompt with its text line
            form.Navigate( CFakeWebSite::Url( "/dialogs" ) );
            const GUI::CMessageBoxForm::EMessageBoxResult choices[ 3 ] = { GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK, GUI::CMessageBoxForm::MESSAGEBOXRESULT_CANCEL, GUI::CMessageBoxForm::MESSAGEBOXRESULT_OK };
            for ( UInt32 d=0; d<3; ++d )
            {
                for ( UInt32 i=0; i<300 && GUCEF_NULL == form.GetOpenPrompt(); ++i )
                {
                    context->RenderFrame( 16 );
                    MT::PrecisionDelay( 2 );
                }
                ASSERT_TRUE( GUCEF_NULL != form.GetOpenPrompt() );
                if ( GUCEF_NULL == form.GetOpenPrompt() )
                    break;
                if ( 2 == d )
                {
                    ASSERT_TRUE( "suggested" == form.GetOpenPrompt()->GetInputText() );
                    form.GetOpenPrompt()->GetWidget( "Input" )->SetProperty( "Text", "typed" );
                }
                form.GetOpenPrompt()->Choose( choices[ d ] );
                context->RenderFrame( 16 );
            }
            for ( UInt32 i=0; i<300 && FakeOf( view )->GetAnswers().size() < 5; ++i )
            {
                context->RenderFrame( 16 );
                MT::PrecisionDelay( 2 );
            }
            answers = FakeOf( view )->GetAnswers();
            ASSERT_TRUE( 5 == answers.size() && "alert" == answers[ 2 ] && "confirm:refused" == answers[ 3 ] && "prompt:typed" == answers[ 4 ] );
            form.Detach();
        }
        AwaitOk( view->Close() );
        engine.Shutdown( 5000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewForm: the user action log leaves out the keys and text of the page" )
    try
    {
        SPageFixture f;
        ASSERT_TRUE( f.Make( MakeTestSettings(), "/login" ) );
        CUserLogRecorder recorder;
        const bool wasEnabled = CORE::CCoreGlobal::Instance()->GetLogManager().IsUserLogEnabled();
        CORE::CLogStreamScope::FlushLogs();
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &recorder );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetUserLogEnabled( true );
        GUI::CGUIUserActionLog actionLog;
        ASSERT_TRUE( actionLog.Attach( f.contextPtr, "web" ) );
        Int32 x = 0, y = 0;
        ElementPoint( f.page, f.view, "password", x, y );
        ClickAt( f.context, x, y );
        f.context->InjectText( "hunter2secret" );
        PressKey( f.context, INPUT::KEYCODE_Q );
        PressKey( f.context, INPUT::KEYCODE_RETURN );
        f.context->RenderFrame( 16 );
        CORE::CLogStreamScope::FlushLogs();
        actionLog.DetachAll();
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &recorder );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetUserLogEnabled( wasEnabled );
        ASSERT_TRUE( recorder.text.HasSubstr( "hunter2secret" ) < 0 );
        ASSERT_TRUE( recorder.text.HasSubstr( "key Q" ) < 0 );
        ASSERT_TRUE( GUI::CGUIUserActionLog::IsPrivateInput( f.page ) && !GUI::CGUIUserActionLog::IsSecretInput( f.page ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewFormPresenter: a view presented from another thread opens in a window on the GUI thread; Cancel closes it with an event, Dismiss without" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        CFakeWebViewEngine engine( "PresenterFake" );
        WEB::TWebViewPtr view = CreateTestView( engine, MakeSignInSettings( "presenter" ) );
        AwaitOk( view->Navigate( CFakeWebSite::Url( "/login" ) ) );
        {
            GUI::CWebViewFormPresenter presenter( context );
            CEventRecorder closed( &presenter, WEB::CIWebViewPresenter::PresentationClosedEvent );
            ASSERT_FALSE( WEB::CWebViewGlobal::Instance()->MayWaitOnCurrentThread() );     // the GUI thread never waits

            CPresentingThread thread;
            thread.presenter = &presenter;
            thread.view = view;
            ASSERT_TRUE( thread.Activate( GUCEF_NULL, 0, 0 ) );
            for ( UInt32 i=0; i<500 && ( 0 == thread.done.Load() || GUCEF_NULL == presenter.GetForm() ); ++i )
            {
                context->RenderFrame( 16 );
                MT::PrecisionDelay( 2 );
            }
            ASSERT_TRUE( presenter.IsPresenting() && GUCEF_NULL != presenter.GetForm() );
            GUI::CWebViewForm* form = presenter.GetForm();
            ASSERT_TRUE( GUCEF_NULL != form && form->IsVisible() && "Sign in to the test site" == form->GetPrompt() );
            ASSERT_TRUE( form->GetWidget( "Banner" )->IsVisible() && !form->GetWidget( "NavigationBar" )->IsVisible() && form->GetWidget( "Cancel" )->IsVisible() );
            ASSERT_TRUE( "Sign in" == form->GetRootWidget()->GetProperty( "Title" ) );
            ASSERT_TRUE( context->GetFocus() == form->GetPageView() );
            context->FlushChanges();
            const GUI::CRect& window = form->GetRootWidget()->GetRect();
            ASSERT_TRUE( window.width >= 336 && window.width <= 640 * 6 / 10 + 1 );

            presenter.UpdatePrompt( "Solve the puzzle to continue" );
            context->RenderFrame( 16 );
            ASSERT_TRUE( "Solve the puzzle to continue" == form->GetWidget( "Banner" )->GetProperty( "Text" ) );

            ClickWidget( context, form->GetWidget( "Cancel" ) );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            ASSERT_TRUE( 1 == closed.count && GUCEF_NULL == presenter.GetForm() && !presenter.IsPresenting() );

            WEB::CWebViewPresentation presentation;
            presentation.title = "Again";
            ASSERT_TRUE( presenter.Present( view, presentation ) );
            context->RenderFrame( 16 );
            ASSERT_TRUE( GUCEF_NULL != presenter.GetForm() );
            presenter.Dismiss();
            context->RenderFrame( 16 );
            ASSERT_TRUE( GUCEF_NULL == presenter.GetForm() && 1 == closed.count );
            while ( thread.IsActive() )
                MT::PrecisionDelay( 1 );
        }
        ASSERT_TRUE( WEB::CWebViewGlobal::Instance()->MayWaitOnCurrentThread() );
        AwaitOk( view->Close() );
        engine.Shutdown( 5000 );
    }
    catch ( ... )
    {
        WEB::CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), false );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "WebViewForm: the GUI thread keeps drawing while the engine thread is busy; it never waits for the view" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = contextPtr->AsGUIContext();
        CFakeWebViewEngine engine( "BusyFake" );
        engine.SetLoopMs( 600 );
        WEB::TWebViewPtr view = CreateTestView( engine, MakeTestSettings( "busy" ) );
        {
            GUI::CWebViewForm form( context );
            ASSERT_TRUE( form.Build( "Browser" ) && form.Attach( view ) );
            WEB::CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), true );
            form.Navigate( CFakeWebSite::Url( "/loop" ) );
            Float64 slowest = 0.0;
            for ( UInt32 i=0; i<20; ++i )
            {
                const UInt64 start = MT::PrecisionTickCount();
                context->RenderFrame( 16 );
                const Float64 took = MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start );
                if ( took > slowest )
                    slowest = took;
                MT::PrecisionDelay( 10 );
            }
            ASSERT_TRUE( view->IsLoading() || form.IsLoadingOverlayShown() || slowest < 100.0 );
            ASSERT_TRUE( slowest < 100.0 );
            WEB::CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), false );
            form.Detach();
        }
        AwaitOk( view->Close() );
        engine.Shutdown( 5000 );
    }
    catch ( ... )
    {
        WEB::CWebViewGlobal::Instance()->SetThreadMustNotWait( MT::GetCurrentThreadID(), false );
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    // the form drawn by the drivers against the software reference, with a field typed into after the first frames
    RunFormRenderTests( "web view form", &BuildBrowserScene );
    RunFormRenderTests( "web view form for a sign-in", &BuildSignInScene );
}

/*-------------------------------------------------------------------------*/
