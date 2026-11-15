/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
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

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefAUTHGUI_TestCommon.h"
#include "TestLayoutUtils.h"
#include "TestRecordingBrowser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

typedef AUTHGUI::COAuthCodeEntryForm TCodeForm;
typedef AUTHGUI::CSignInSettingsForm TSettingsForm;
typedef AUTHGUI::CSignInSettingsState TState;

const char* const AuthorizeUrl = "https://accounts.example.com/oauth2/authorize?client_id=abc&state=s3cr3t-state";
const char* const TheCode = "4/0AbCdEfGhIjKlMnOpQrStUvWxYz-codeofthetest";
const char* const OtherCode = "4/0ZyXwVuTsRqPoNmLkJiHgFeDcBa-anothercode";
const char* const UserValue = "owner@example.com";
const char* const PasswordValue = "p4ssw0rd-of-the-form-test";

/** A clipboard that lives in memory, in place of the one of the operating system */
class CTestClipboard
{
    public:

    static CORE::CString& Text( void ) { static CORE::CString text; return text; }
    static bool& Available( void ) { static bool available = true; return available; }

    static bool Write( const char* text )
    {
        if ( !Available() )
            return false;
        Text() = text;
        return true;
    }

    static bool Read( char* dest, UInt32 size, UInt32* written )
    {
        if ( !Available() || Text().IsNULLOrEmpty() || Text().Length() + 1 > size )
            return false;
        memcpy( dest, Text().C_String(), Text().Length() );
        dest[ Text().Length() ] = '\0';
        if ( GUCEF_NULL != written )
            *written = Text().Length();
        return true;
    }

    CTestClipboard( void )
    {
        Text().Clear();
        Available() = true;
        GUI::CSecretInput::SetClipboardReader( &CTestClipboard::Read );
        GUI::CSecretInput::SetClipboardWriter( &CTestClipboard::Write );
    }

    ~CTestClipboard()
    {
        GUI::CSecretInput::SetClipboardReader( GUCEF_NULL );
        GUI::CSecretInput::SetClipboardWriter( GUCEF_NULL );
    }
};

/*-------------------------------------------------------------------------*/

/** An engine that takes the code the way the test says, and remembers what it was given */
class CFakeCodeEngine : public AUTH::IAuthFlowEngine
{
    public:

    AUTH::TCodeCheck answer;
    UInt32 count;
    CORE::CString lastText;

    CFakeCodeEngine( void ) : AUTH::IAuthFlowEngine(), answer( AUTH::CODECHECK_ACCEPTED ), count( 0 ), lastText() {}

    virtual bool IsSupported( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool StartInteractive( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAuthOptions& options, AUTH::IAuthFlowSink& sink, CORE::CFutureResultSource& result ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool StartRefresh( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken, AUTH::IAuthFlowSink& sink, CORE::CFutureResultSource& result ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Revoke( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAccountRef& account, const AUTH::CSecretString& refreshToken ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool Verify( const AUTH::CAuthProviderDescriptor& descriptor, const AUTH::CAuthProviderSettings& settings, const AUTH::CAuthProof& proof, AUTH::CAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE { return false; }

    virtual bool SubmitAuthorizationCode( const AUTH::CSecretString& text ) GUCEF_VIRTUAL_OVERRIDE
    {
        AUTH::TCodeCheck check = AUTH::CODECHECK_REFUSED;
        return SubmitAuthorizationCode( text, check );
    }

    virtual bool SubmitAuthorizationCode( const AUTH::CSecretString& text, AUTH::TCodeCheck& check ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++count;
        lastText = text.Reveal();
        check = answer;
        return AUTH::CODECHECK_ACCEPTED == check;
    }
};

/*-------------------------------------------------------------------------*/

class CCodeFormFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    TCodeForm form;
    CFakeCodeEngine engine;

    CCodeFormFixture( UInt32 width = 1280, UInt32 height = 960 )
        : contextPtr( CreateSizedFormContext( width, height, true ) )
        , context( ContextOf( contextPtr ) )
        , form( context )
        , engine()
    {
        form.SetEngine( &engine );
    }

    ~CCodeFormFixture()
    {
        form.SetEngine( GUCEF_NULL );
    }

    void Type( const CORE::CString& field, const CORE::CString& text, bool enter )
    {
        context->FlushChanges();
        GUI::CWidget* widget = form.GetWidget( field );
        if ( GUCEF_NULL == widget )
            return;
        context->SetFocus( widget );
        context->InjectText( text );
        if ( enter )
            PressKey( context, INPUT::KEYCODE_RETURN );
    }

    private:

    CCodeFormFixture( const CCodeFormFixture& src );
    CCodeFormFixture& operator=( const CCodeFormFixture& src );
};

/*-------------------------------------------------------------------------*/

void
CollectFormTexts( GUI::CWidget* widget, CORE::CString::StringVector& texts )
{
    texts.push_back( widget->GetProperty( "Text" ) );
    texts.push_back( widget->GetProperty( "Tooltip" ) );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        CollectFormTexts( children[ i ], texts );
}

/*-------------------------------------------------------------------------*/

bool
AnyFormTextHas( GUI::CWidget* root, const CORE::CString& text )
{
    CORE::CString::StringVector texts;
    CollectFormTexts( root, texts );
    for ( size_t i=0; i<texts.size(); ++i )
        if ( texts[ i ].HasSubstr( text ) >= 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CodeFieldLength( GUI::CForm& form, const char* name )
{
    GUI::CSecretInput* field = form.GetWidgetAs< GUI::CSecretInput >( name );
    return GUCEF_NULL == field ? 0 : field->GetValueLength();
}

/*-------------------------------------------------------------------------*/

/** Everything that is logged while it exists */
class CSignInLogCapture
{
    public:

    CSignInLogCapture( void ) : m_buffer(), m_access( m_buffer ), m_logger( m_access )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );
    }

    ~CSignInLogCapture()
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( &m_logger );
    }

    CORE::CString Text( void )
    {
        CORE::CLogStreamScope::FlushLogs();
        if ( 0 == m_buffer.GetDataSize() )
            return CORE::CString::Empty;
        return CORE::CString( m_buffer.AsConstTypePtr< char >(), m_buffer.GetDataSize() );
    }

    private:

    CORE::CDynamicBuffer m_buffer;
    CORE::CDynamicBufferAccess m_access;
    CORE::CStdLogger m_logger;
};

/*-------------------------------------------------------------------------*/

/** The handler of SaveCredentialsRequestedEvent of an application: takes the values the way the real one does */
class CCredentialTaker : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CCredentialTaker > TEventCallback;

    TSettingsForm& form;
    UInt32 requests;
    AUTH::CSecretString user;
    AUTH::CSecretString password;

    explicit CCredentialTaker( TSettingsForm& theForm )
        : CORE::CObserver(), form( theForm ), requests( 0 ), user(), password()
    {
        TEventCallback callback( this, &CCredentialTaker::OnRequested );
        SubscribeTo( &form, TSettingsForm::SaveCredentialsRequestedEvent, callback );
    }

    virtual ~CCredentialTaker() { UnsubscribeAllFromObserver(); }

    void OnRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++requests;
        form.GetCredential( "username", user );
        form.GetCredential( "password", password );
    }
};

/*-------------------------------------------------------------------------*/

TState
MakeState( bool signedIn, bool running, TState::EKeyringState keyring, bool saved )
{
    TState state;
    state.serviceName = "Example Store";
    state.signedIn = signedIn;
    state.signInRunning = running;
    state.keyringState = keyring;
    state.hasSavedCredentials = saved;
    state.credentialSlots.push_back( "username" );
    state.credentialSlots.push_back( "password" );
    return state;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSignInFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.OAuthCodeEntryForm" );

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: The layout lint over the matrix of sizes, scales and pseudo-localised text stays within the budget of the form" )
        try
        {
            CCodeFormFixture fixture;
            REQUIRE_TRUE( fixture.form.Open( "Example Store", AuthorizeUrl ) );
            ASSERT_TRUE( RunFormLayoutLint( fixture.context, "OAuthCodeEntry" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: Open shows the provider and the address; Copy puts the address on the clipboard, Open in browser hands it to the browser, an address that is not safe is not opened" )
        try
        {
            CTestClipboard clipboard;
            CRecordingBrowser browser;
            CCodeFormFixture fixture;
            TCodeForm& form = fixture.form;
            ASSERT_TRUE( form.Open( "Example Store", AuthorizeUrl ) && form.IsOpen() && form.IsVisible() );
            ASSERT_TRUE( "Example Store" == form.GetProviderDisplayName() && AuthorizeUrl == form.GetAuthorizationUrl() );
            ASSERT_TRUE( form.GetWidget( "Prompt" )->GetProperty( "Text" ).HasSubstr( "Example Store" ) >= 0 );
            ASSERT_TRUE( AuthorizeUrl == form.GetWidget( "Url" )->GetProperty( "Text" ) );
            ASSERT_TRUE( form.GetWidget( "Url" )->GetBoolProperty( "ReadOnly" ) );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidgetAs< GUI::CSecretInput >( "Code" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "OpenUrl" ) && IsWidgetEnabled( form, "CopyUrl" ) && form.CanOpenUrl() );

            ClickWidget( fixture.context, form.GetWidget( "CopyUrl" ) );
            ASSERT_TRUE( AuthorizeUrl == CTestClipboard::Text() && "Address copied." == form.GetMessageText() );
            CTestClipboard::Available() = false;
            ASSERT_FALSE( form.CopyUrl() );
            ASSERT_TRUE( form.GetMessageText().HasSubstr( "Could not copy" ) >= 0 );

            ClickWidget( fixture.context, form.GetWidget( "OpenUrl" ) );
            ASSERT_TRUE( 1 == browser.urls.size() && AuthorizeUrl == browser.urls[ 0 ] );
            browser.accept = false;
            ASSERT_FALSE( form.OpenUrl() );
            ASSERT_TRUE( form.GetMessageText().HasSubstr( "did not open" ) >= 0 );

            // an address with user information, or one that breaks out of a command line, never reaches the browser
            const char* const unsafe[] = { "https://user@accounts.example.com/authorize", "https://accounts.example.com/a b", "ftp://accounts.example.com/x", "https://accounts.example.com/`id`" };
            for ( UInt32 i=0; i<4; ++i )
            {
                ASSERT_TRUE( form.Open( "Example Store", unsafe[ i ] ) );
                ASSERT_FALSE( form.CanOpenUrl() || IsWidgetEnabled( form, "OpenUrl" ) );
                ASSERT_FALSE( form.OpenUrl() );
                ASSERT_TRUE( IsWidgetEnabled( form, "CopyUrl" ) && unsafe[ i ] == form.GetWidget( "Url" )->GetProperty( "Text" ) );
            }
            ASSERT_TRUE( 2 == browser.urls.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: A code is handed to the engine as typed, the field is wiped, taken closes the window with DoneEvent; the page that holds the code goes the same way" )
        try
        {
            CCodeFormFixture fixture;
            TCodeForm& form = fixture.form;
            CEventCount submitted( &form, TCodeForm::CodeSubmittedEvent );
            CEventCount done( &form, TCodeForm::DoneEvent );
            CEventCount cancelled( &form, TCodeForm::CancelledEvent );
            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );

            // nothing typed
            ClickWidget( fixture.context, form.GetWidget( "Submit" ) );
            ASSERT_TRUE( "The code is empty." == form.GetMessageText() && 0 == fixture.engine.count && 0 == submitted.count );

            fixture.Type( "Code", TheCode, false );
            ASSERT_TRUE( CodeFieldLength( form, "Code" ) == CORE::CString( TheCode ).Length() );
            ClickWidget( fixture.context, form.GetWidget( "Submit" ) );
            ASSERT_TRUE( 1 == fixture.engine.count && TheCode == fixture.engine.lastText );
            ASSERT_TRUE( 1 == submitted.count && 1 == done.count && 0 == cancelled.count );
            ASSERT_FALSE( form.IsOpen() || form.IsVisible() );
            ASSERT_TRUE( 0 == CodeFieldLength( form, "Code" ) );

            // Enter in the field submits; the text of a whole page goes to the engine as it is
            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
            const CORE::CString page = CORE::CString( "{\"authorizationCode\":\"" ) + OtherCode + "\"}";
            fixture.Type( "Code", page, true );
            ASSERT_TRUE( 2 == fixture.engine.count && page == fixture.engine.lastText && 2 == done.count );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: Every outcome that is not taken is said in its own words and the window stays open: no login waiting, ambiguous, empty, malformed, no code, refused; then a code that is taken" )
        try
        {
            CCodeFormFixture fixture;
            TCodeForm& form = fixture.form;
            CEventCount done( &form, TCodeForm::DoneEvent );
            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
            const AUTH::TCodeCheck checks[] = { AUTH::CODECHECK_NO_LOGIN_WAITING, AUTH::CODECHECK_AMBIGUOUS, AUTH::CODECHECK_EMPTY, AUTH::CODECHECK_MALFORMED, AUTH::CODECHECK_NO_CODE, AUTH::CODECHECK_REFUSED };
            CORE::CString::StringVector messages;
            for ( UInt32 i=0; i<6; ++i )
            {
                fixture.engine.answer = checks[ i ];
                fixture.Type( "Code", TheCode, false );
                ASSERT_FALSE( form.Submit() );
                ASSERT_TRUE( form.IsOpen() && 0 == done.count );
                ASSERT_TRUE( 0 == CodeFieldLength( form, "Code" ) );
                ASSERT_TRUE( !form.GetMessageText().IsNULLOrEmpty() && form.GetMessageText() == TCodeForm::CheckToText( checks[ i ] ) );
                ASSERT_TRUE( form.GetMessageText() == form.GetWidget( "Message" )->GetProperty( "Text" ) );
                for ( size_t j=0; j<messages.size(); ++j )
                    ASSERT_TRUE( messages[ j ] != form.GetMessageText() );
                messages.push_back( form.GetMessageText() );
            }
            ASSERT_TRUE( TCodeForm::CheckToText( AUTH::CODECHECK_ACCEPTED ).IsNULLOrEmpty() );
            fixture.engine.answer = AUTH::CODECHECK_ACCEPTED;
            fixture.Type( "Code", OtherCode, true );
            ASSERT_TRUE( 1 == done.count && !form.IsOpen() && 7 == fixture.engine.count );

            // the application says more afterwards, and asks again
            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
            ASSERT_TRUE( form.GetMessageText().IsNULLOrEmpty() );
            form.ShowMessage( "The store refused the code.", true );
            ASSERT_TRUE( "The store refused the code." == form.GetWidget( "Message" )->GetProperty( "Text" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: Without an engine the application takes the code in its handler of CodeSubmittedEvent and answers with ShowCheck(); Paste, Cancel and Escape" )
        try
        {
            CTestClipboard clipboard;
            CCodeFormFixture fixture;
            fixture.form.SetEngine( GUCEF_NULL );
            TCodeForm& form = fixture.form;
            CEventCount done( &form, TCodeForm::DoneEvent );
            CEventCount cancelled( &form, TCodeForm::CancelledEvent );

            class CTaker : public CORE::CObserver
            {
                public:
                typedef CORE::CTEventHandlerFunctor< CTaker > TEventCallback;
                TCodeForm& form;
                UInt32 requests;
                AUTH::CSecretString code;
                explicit CTaker( TCodeForm& f ) : CORE::CObserver(), form( f ), requests( 0 ), code()
                {
                    TEventCallback callback( this, &CTaker::OnRequested );
                    SubscribeTo( &form, TCodeForm::CodeSubmittedEvent, callback );
                }
                virtual ~CTaker() { UnsubscribeAllFromObserver(); }
                void OnRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
                {
                    ++requests;
                    form.GetCode( code );
                }
            } taker( form );

            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
            // the clipboard has the code with white space around it, as it is when copied from a page
            CTestClipboard::Text() = CORE::CString( "  " ) + TheCode + "\n";
            ClickWidget( fixture.context, form.GetWidget( "Paste" ) );
            ASSERT_TRUE( CodeFieldLength( form, "Code" ) == CORE::CString( TheCode ).Length() );
            ASSERT_TRUE( form.Submit() );
            ASSERT_TRUE( 1 == taker.requests && TheCode == taker.code.Reveal() && 0 == CodeFieldLength( form, "Code" ) );
            ASSERT_TRUE( form.IsOpen() && 0 == done.count );
            form.ShowCheck( AUTH::CODECHECK_MALFORMED );
            ASSERT_TRUE( form.IsOpen() && form.GetMessageText() == TCodeForm::CheckToText( AUTH::CODECHECK_MALFORMED ) );
            form.ShowCheck( AUTH::CODECHECK_ACCEPTED );
            ASSERT_TRUE( 1 == done.count && !form.IsOpen() );

            // nothing to paste, or too long to be a code
            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
            CTestClipboard::Text().Clear();
            ASSERT_FALSE( form.PasteCode() );
            ASSERT_TRUE( form.GetMessageText().HasSubstr( "Nothing to paste" ) >= 0 );
            CTestClipboard::Text().Clear();
            for ( UInt32 i=0; i<500; ++i )
                CTestClipboard::Text() += "0123456789";
            ASSERT_FALSE( form.PasteCode() || CodeFieldLength( form, "Code" ) > 0 );

            // Cancel and Escape
            ClickWidget( fixture.context, form.GetWidget( "Cancel" ) );
            ASSERT_TRUE( 1 == cancelled.count && !form.IsOpen() );
            REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
            fixture.Type( "Code", TheCode, false );
            PressKey( fixture.context, INPUT::KEYCODE_ESCAPE );
            ASSERT_TRUE( 2 == cancelled.count && !form.IsOpen() && 0 == CodeFieldLength( form, "Code" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: The code is in no text of a widget and in no log, typed, submitted, refused or taken" )
        try
        {
            CORE::CString logged;
            {
                CSignInLogCapture capture;
                CCodeFormFixture fixture;
                TCodeForm& form = fixture.form;
                REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
                fixture.Type( "Code", TheCode, false );
                ASSERT_FALSE( AnyFormTextHas( form.GetRootWidget(), TheCode ) );
                fixture.engine.answer = AUTH::CODECHECK_REFUSED;
                ASSERT_FALSE( form.Submit() );
                ASSERT_FALSE( AnyFormTextHas( form.GetRootWidget(), TheCode ) );
                fixture.Type( "Code", OtherCode, false );
                fixture.engine.answer = AUTH::CODECHECK_ACCEPTED;
                ASSERT_TRUE( form.Submit() );
                logged = capture.Text();
            }
            ASSERT_FALSE( logged.HasSubstr( TheCode ) >= 0 || logged.HasSubstr( OtherCode ) >= 0 );
            // the address carries a state: it is shown, but never logged
            ASSERT_FALSE( logged.HasSubstr( "s3cr3t-state" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "OAuthCodeEntryForm: The window fits from 800 x 600 to 5120 x 1440 and the portrait 600 x 900: in the middle, nothing outside its parent, nothing overlaps" )
        try
        {
            const SScreenSize sizes[] = { { 800, 600 }, { 1024, 768 }, { 1920, 1080 }, { 3440, 1440 }, { 5120, 1440 }, { 600, 900 } };
            for ( UInt32 s=0; s<6; ++s )
            {
                const CORE::CString where = SizeName( sizes[ s ].width, sizes[ s ].height );
                GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, true );
                GUI::CGUIContext* context = ContextOf( contextPtr );
                REQUIRE_TRUE( GUCEF_NULL != context );
                TCodeForm form( context );
                REQUIRE_TRUE( form.Open( "Example Store", AuthorizeUrl ) );
                form.ShowMessage( TCodeForm::CheckToText( AUTH::CODECHECK_AMBIGUOUS ), true );
                context->RenderFrame( 16 );
                context->FlushChanges();
                context->RenderFrame( 16 );
                GUI::CWidget* window = form.GetRootWidget();
                REQUIRE_TRUE( GUCEF_NULL != window );
                GUI::CRect rect = window->GetRect();
                ASSERT_TRUE( Inside( rect, GUI::CRect( 0, 0, (Int32) sizes[ s ].width, (Int32) sizes[ s ].height ) ) );
                ASSERT_TRUE( rect.width >= 400 && rect.width <= 640 );
                Int32 centerX = rect.left + rect.width / 2;
                ASSERT_TRUE( centerX >= (Int32) sizes[ s ].width / 2 - 1 && centerX <= (Int32) sizes[ s ].width / 2 + 1 );
                CORE::CString problem = FindLayoutProblem( window );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the code entry form at " << where << ": " << problem );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                WriteShot( CaptureFrame( context ), "oauthcode-" + where );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.SignInSettingsForm" );

    GUCEF_TESTFW_TESTCASE( "SignInSettingsForm: The layout lint over the matrix of sizes, scales and pseudo-localised text stays within the budget of the form" )
        try
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1280, 960, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            TSettingsForm form( context );
            form.SetState( MakeState( true, false, TState::KEYRING_UNLOCKED, true ) );
            REQUIRE_TRUE( form.Open() && form.IsOpen() );
            ASSERT_TRUE( RunFormLayoutLint( context, "SignInSettings" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SignInSettingsForm: What it shows and what is enabled follows the state: signed out, running, signed in, and the four states of the keyring" )
        try
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1280, 960, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            TSettingsForm form( context );
            form.SetState( MakeState( false, false, TState::KEYRING_NOT_SET_UP, false ) );
            REQUIRE_TRUE( form.Open() && form.IsOpen() );
            ASSERT_TRUE( "Not signed in" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "SignIn" ) && !IsWidgetEnabled( form, "SignOut" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "SaveCredentials" ) && !IsWidgetEnabled( form, "ForgetCredentials" ) );
            ASSERT_TRUE( "Set up keyring code..." == form.GetWidget( "KeyringCode" )->GetProperty( "Text" ) && !IsWidgetEnabled( form, "LockNow" ) );
            ASSERT_TRUE( "No credentials are saved." == form.GetCredentialsNoteText() && form.GetKeyringNoteText().HasSubstr( "No keyring code yet" ) >= 0 );
            ASSERT_TRUE( "Username" == form.GetWidget( "SlotLabel0" )->GetProperty( "Text" ) && "Password" == form.GetWidget( "SlotLabel1" )->GetProperty( "Text" ) );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidgetAs< GUI::CSecretInput >( "Slot0" ) && GUCEF_NULL != form.GetWidgetAs< GUI::CSecretInput >( "Slot1" ) );

            TState state = MakeState( false, true, TState::KEYRING_LOCKED, false );
            state.statusNote = "The session ended.";
            form.SetState( state );
            ASSERT_TRUE( "Signing in... The session ended." == form.GetStatusText() );
            ASSERT_FALSE( IsWidgetEnabled( form, "SignIn" ) || IsWidgetEnabled( form, "SignOut" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "ForgetCredentials" ) && !IsWidgetEnabled( form, "LockNow" ) && "Change keyring code..." == form.GetWidget( "KeyringCode" )->GetProperty( "Text" ) );
            ASSERT_TRUE( form.GetCredentialsNoteText().HasSubstr( "Unlock" ) >= 0 && form.GetKeyringNoteText().HasSubstr( "Locked" ) >= 0 );

            form.SetState( MakeState( true, false, TState::KEYRING_UNLOCKED, true ) );
            ASSERT_TRUE( "Signed in" == form.GetStatusText() && !IsWidgetEnabled( form, "SignIn" ) && IsWidgetEnabled( form, "SignOut" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "LockNow" ) && IsWidgetEnabled( form, "ForgetCredentials" ) );
            ASSERT_TRUE( form.GetCredentialsNoteText().HasSubstr( "are saved" ) >= 0 && form.GetKeyringNoteText().HasSubstr( "Unlocked" ) >= 0 );

            form.SetState( MakeState( false, false, TState::KEYRING_UNAVAILABLE, false ) );
            ASSERT_FALSE( IsWidgetEnabled( form, "SaveCredentials" ) || IsWidgetEnabled( form, "ForgetCredentials" ) || IsWidgetEnabled( form, "KeyringCode" ) || IsWidgetEnabled( form, "LockNow" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "SignIn" ) && form.GetCredentialsNoteText().HasSubstr( "cannot keep" ) >= 0 );

            // a service without credentials has no fields and nothing to save
            TState plain = MakeState( false, false, TState::KEYRING_UNLOCKED, false );
            plain.credentialSlots.clear();
            form.SetState( plain );
            ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Slot0" ) && !IsWidgetEnabled( form, "SaveCredentials" ) && IsWidgetEnabled( form, "SignIn" ) );
            ASSERT_TRUE( form.IsOpen() && form.IsVisible() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SignInSettingsForm: Each button notifies its event once when it is enabled and nothing when it is not; the keyring button sets up or changes the code; Close and Escape" )
        try
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1280, 960, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            TSettingsForm form( context );
            CEventCount signIn( &form, TSettingsForm::SignInRequestedEvent );
            CEventCount signOut( &form, TSettingsForm::SignOutRequestedEvent );
            CEventCount forget( &form, TSettingsForm::ForgetCredentialsRequestedEvent );
            CEventCount setUp( &form, TSettingsForm::SetUpCodeRequestedEvent );
            CEventCount change( &form, TSettingsForm::ChangeCodeRequestedEvent );
            CEventCount lockNow( &form, TSettingsForm::LockNowRequestedEvent );
            CEventCount closed( &form, TSettingsForm::ClosedEvent );
            form.SetState( MakeState( false, false, TState::KEYRING_NOT_SET_UP, false ) );
            REQUIRE_TRUE( form.Open() );
            ClickWidget( context, form.GetWidget( "SignIn" ) );
            ClickWidget( context, form.GetWidget( "SignOut" ) );
            ClickWidget( context, form.GetWidget( "ForgetCredentials" ) );
            ClickWidget( context, form.GetWidget( "LockNow" ) );
            ASSERT_TRUE( 1 == signIn.count && 0 == signOut.count && 0 == forget.count && 0 == lockNow.count );
            ClickWidget( context, form.GetWidget( "KeyringCode" ) );
            ASSERT_TRUE( 1 == setUp.count && 0 == change.count );

            form.SetState( MakeState( true, false, TState::KEYRING_UNLOCKED, true ) );
            ClickWidget( context, form.GetWidget( "SignIn" ) );
            ClickWidget( context, form.GetWidget( "SignOut" ) );
            ClickWidget( context, form.GetWidget( "ForgetCredentials" ) );
            ClickWidget( context, form.GetWidget( "LockNow" ) );
            ClickWidget( context, form.GetWidget( "KeyringCode" ) );
            ASSERT_TRUE( 1 == signIn.count && 1 == signOut.count && 1 == forget.count && 1 == lockNow.count && 1 == setUp.count && 1 == change.count );

            ClickWidget( context, form.GetWidget( "Close" ) );
            ASSERT_TRUE( 1 == closed.count && !form.IsOpen() && !form.IsVisible() );
            REQUIRE_TRUE( form.Open() );
            PressKey( context, INPUT::KEYCODE_ESCAPE );
            ASSERT_TRUE( 2 == closed.count && !form.IsOpen() );
            // closed, the buttons do nothing
            ASSERT_FALSE( form.SignOut() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SignInSettingsForm: Save credentials needs every field; the handler takes the values; the fields are wiped after it; no value is in a widget or in the log" )
        try
        {
            CORE::CString logged;
            {
                CSignInLogCapture capture;
                GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1280, 960, true );
                GUI::CGUIContext* context = ContextOf( contextPtr );
                REQUIRE_TRUE( GUCEF_NULL != context );
                TSettingsForm form( context );
                form.SetState( MakeState( false, false, TState::KEYRING_UNLOCKED, false ) );
                CCredentialTaker taker( form );
                REQUIRE_TRUE( form.Open() );

                ClickWidget( context, form.GetWidget( "SaveCredentials" ) );
                ASSERT_TRUE( "Fill in the username first." == form.GetMessageText() && 0 == taker.requests );
                context->FlushChanges();
                context->SetFocus( form.GetWidget( "Slot0" ) );
                context->InjectText( UserValue );
                ClickWidget( context, form.GetWidget( "SaveCredentials" ) );
                ASSERT_TRUE( "Fill in the password first." == form.GetMessageText() && 0 == taker.requests );
                context->SetFocus( form.GetWidget( "Slot1" ) );
                context->InjectText( PasswordValue );
                ASSERT_FALSE( AnyFormTextHas( form.GetRootWidget(), UserValue ) || AnyFormTextHas( form.GetRootWidget(), PasswordValue ) );
                PressKey( context, INPUT::KEYCODE_RETURN );
                ASSERT_TRUE( 1 == taker.requests && UserValue == taker.user.Reveal() && PasswordValue == taker.password.Reveal() );
                ASSERT_TRUE( 0 == CodeFieldLength( form, "Slot0" ) && 0 == CodeFieldLength( form, "Slot1" ) );
                ASSERT_TRUE( form.GetMessageText().IsNULLOrEmpty() );

                // typed again, then the window closes: nothing stays
                context->SetFocus( form.GetWidget( "Slot1" ) );
                context->InjectText( PasswordValue );
                ASSERT_TRUE( 0 < CodeFieldLength( form, "Slot1" ) );
                form.CloseWindow();
                ASSERT_TRUE( 0 == CodeFieldLength( form, "Slot1" ) );
                logged = capture.Text();
            }
            ASSERT_FALSE( logged.HasSubstr( UserValue ) >= 0 || logged.HasSubstr( PasswordValue ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "SignInSettingsForm: The window fits from 1024 x 768 to 5120 x 1440 and the portrait 900 x 1200: in the middle, nothing outside its parent, nothing overlaps" )
        try
        {
            const SScreenSize sizes[] = { { 1024, 768 }, { 1280, 960 }, { 1920, 1080 }, { 3440, 1440 }, { 5120, 1440 }, { 900, 1200 } };
            for ( UInt32 s=0; s<6; ++s )
            {
                const CORE::CString where = SizeName( sizes[ s ].width, sizes[ s ].height );
                GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, true );
                GUI::CGUIContext* context = ContextOf( contextPtr );
                REQUIRE_TRUE( GUCEF_NULL != context );
                TSettingsForm form( context );
                form.SetState( MakeState( true, false, TState::KEYRING_UNLOCKED, true ) );
                REQUIRE_TRUE( form.Open() );
                form.ShowMessage( "Credentials saved.", false );
                context->RenderFrame( 16 );
                context->FlushChanges();
                context->RenderFrame( 16 );
                GUI::CWidget* window = form.GetRootWidget();
                REQUIRE_TRUE( GUCEF_NULL != window );
                GUI::CRect rect = window->GetRect();
                ASSERT_TRUE( Inside( rect, GUI::CRect( 0, 0, (Int32) sizes[ s ].width, (Int32) sizes[ s ].height ) ) );
                ASSERT_TRUE( rect.width >= 480 && rect.width <= 560 );
                Int32 centerX = rect.left + rect.width / 2;
                ASSERT_TRUE( centerX >= (Int32) sizes[ s ].width / 2 - 1 && centerX <= (Int32) sizes[ s ].width / 2 + 1 );
                CORE::CString problem = FindLayoutProblem( window );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the sign in settings form at " << where << ": " << problem );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                WriteShot( CaptureFrame( context ), "signin-settings-" + where );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
