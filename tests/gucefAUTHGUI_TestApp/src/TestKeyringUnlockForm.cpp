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

typedef AUTHGUI::CKeyringUnlockForm TForm;

const char* const RightCode = "keyring code of the form 4b8e";
const char* const WrongCode = "keyring code of the farm 4b8e";
const char* const NewCode = "Another-Keyring-Code-2c61";

/**
 *  A session whose asynchronous operations are, when asked, done on the pool before the call returns: what a fast pool
 *  thread does now and then, every time. The form sees a future that has ended already and still has to wait for the events
 */
class CCompletingKeyringSession : public AUTH::CKeyringSession
{
    public:

    bool completeBeforeReturn;
    UInt32 completedBeforeReturn;

    CCompletingKeyringSession( void )
        : AUTH::CKeyringSession(), completeBeforeReturn( false ), completedBeforeReturn( 0 )
    {
    }

    virtual CORE::CFutureResult UnlockWithCodeAsync( const AUTH::CSecretString& code ) GUCEF_VIRTUAL_OVERRIDE
    {
        return AwaitWhenAsked( AUTH::CKeyringSession::UnlockWithCodeAsync( code ) );
    }

    virtual CORE::CFutureResult SetUpCodeAsync( const AUTH::CSecretString& code ) GUCEF_VIRTUAL_OVERRIDE
    {
        return AwaitWhenAsked( AUTH::CKeyringSession::SetUpCodeAsync( code ) );
    }

    virtual CORE::CFutureResult ChangeCodeAsync( const AUTH::CSecretString& currentCode, const AUTH::CSecretString& newCode ) GUCEF_VIRTUAL_OVERRIDE
    {
        return AwaitWhenAsked( AUTH::CKeyringSession::ChangeCodeAsync( currentCode, newCode ) );
    }

    private:

    CORE::CFutureResult AwaitWhenAsked( const CORE::CFutureResult& future )
    {
        if ( completeBeforeReturn )
        {
            try
            {
                future.Await( 20000 );
            }
            catch ( ... )
            {
                return future;
            }
            // done on the pool, the events of the outcome wait for the next tick of the timer of the session
            if ( future.IsReady() && future.HasAFuture() && !IsBusy() )
                ++completedBeforeReturn;
        }
        return future;
    }
};

/*-------------------------------------------------------------------------*/

/** A keyring store in a directory of its own, sealed with cheap Argon2id parameters, a session of it and the form */
class CKeyringFormFixture
{
    public:

    GUI::TGuiContextPtr contextPtr;
    GUI::CGUIContext* context;
    AUTH::CFileCredentialStore store;
    CCompletingKeyringSession session;
    TForm form;

    CKeyringFormFixture( const char* name, bool setUp, UInt32 width = 1280, UInt32 height = 960 )
        : contextPtr( CreateSizedFormContext( width, height, true ) )
        , context( ContextOf( contextPtr ) )
        , store()
        , session()
        , form( context )
    {
        CORE::CString dir = CORE::CombinePath( CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "authguitemp" ), name );
        if ( CORE::DirExists( dir ) )
            CORE::DeleteDir( dir, true );
        store.SetDirectory( dir );
        store.SetKeyMode( AUTH::CFileCredentialStore::KEYMODE_KEYRING_CODE );
        store.SetKeyringParameters( 64, 1, 1 );
        if ( setUp && AUTH::CFileCredentialStore::IsKeyringAvailable() )
        {
            store.SetUpKeyring( AUTH::CSecretString( RightCode ) );
            store.Store( "fab", "owner", AUTH::CSecretString( "saved password of the form test" ) );
            store.LockKeyring();
        }
        AUTH::CKeyringSettings settings;
        settings.checkIntervalInMs = 5;
        session.SetSettings( settings );
        session.SetStore( &store );
        form.SetSession( &session );
    }

    ~CKeyringFormFixture()
    {
        form.SetSession( GUCEF_NULL );
        session.SetStore( GUCEF_NULL );
        PumpFor( 10 );
    }

    bool IsReady( void ) const
    {
        return GUCEF_NULL != context && AUTH::CFileCredentialStore::IsKeyringAvailable();
    }

    /** The events reach the form, the form shows what it has to */
    void Settle( void )
    {
        PumpFor( 20 );
        context->RenderFrame( 16 );
        context->FlushChanges();
    }

    /** Pumps and draws until the operation of the form is done */
    bool SettleUntilIdle( UInt32 timeoutInMs )
    {
        for ( UInt32 waited=0; waited<timeoutInMs && form.IsBusy(); waited+=5 )
        {
            PumpFor( 5 );
            context->RenderFrame( 5 );
        }
        Settle();
        return !form.IsBusy();
    }

    /** The person types in a field, Enter when asked */
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

    CKeyringFormFixture( const CKeyringFormFixture& src );
    CKeyringFormFixture& operator=( const CKeyringFormFixture& src );
};

/*-------------------------------------------------------------------------*/

void
CollectTexts( GUI::CWidget* widget, CORE::CString::StringVector& texts )
{
    texts.push_back( widget->GetProperty( "Text" ) );
    texts.push_back( widget->GetProperty( "Tooltip" ) );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        CollectTexts( children[ i ], texts );
}

/*-------------------------------------------------------------------------*/

bool
AnyTextHas( GUI::CWidget* root, const CORE::CString& text )
{
    CORE::CString::StringVector texts;
    CollectTexts( root, texts );
    for ( size_t i=0; i<texts.size(); ++i )
        if ( texts[ i ].HasSubstr( text ) >= 0 )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
FieldLength( TForm& form, const char* name )
{
    GUI::CSecretInput* field = form.GetWidgetAs< GUI::CSecretInput >( name );
    return GUCEF_NULL == field ? 0 : field->GetValueLength();
}

/*-------------------------------------------------------------------------*/

CORE::CSecureBuffer
MakeSecret( const char* text )
{
    CORE::CSecureBuffer buffer( 256 );
    buffer.Assign( text, (UInt32) strlen( text ) );
    return buffer;
}

/*-------------------------------------------------------------------------*/

/** An application without a session: it takes the code in its handler of UnlockRequestedEvent */
class CCodeTaker : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CCodeTaker > TEventCallback;

    TForm& form;
    UInt32 requests;
    AUTH::CSecretString code;
    AUTH::CSecretString newCode;

    explicit CCodeTaker( TForm& theForm )
        : CORE::CObserver(), form( theForm ), requests( 0 ), code(), newCode()
    {
        TEventCallback callback( this, &CCodeTaker::OnRequested );
        SubscribeTo( &form, TForm::UnlockRequestedEvent, callback );
    }

    virtual ~CCodeTaker() { UnsubscribeAllFromObserver(); }

    void OnRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++requests;
        form.GetCode( code );
        form.GetNewCode( newCode );
    }
};

/*-------------------------------------------------------------------------*/

/** Everything that is logged while it exists */
class CFormLogCapture
{
    public:

    CFormLogCapture( void ) : m_buffer(), m_access( m_buffer ), m_logger( m_access )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &m_logger );
    }

    ~CFormLogCapture()
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

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformKeyringUnlockFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.KeyringUnlockForm" );

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: The layout lint over the matrix of sizes, scales and pseudo-localised text stays within the budget of the form" )
        try
        {
            CKeyringFormFixture fixture( "lint", true );
            REQUIRE_TRUE( fixture.form.Open() );
            ASSERT_TRUE( RunFormLayoutLint( fixture.context, "KeyringUnlock" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    const bool available = AUTH::CFileCredentialStore::IsKeyringAvailable();

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: Unlock asks for the code; an empty code, a wrong code and the right code; the work runs on the pool and the form shows it; done closes it" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CKeyringFormFixture fixture( "unlock", true );
            REQUIRE_TRUE( fixture.IsReady() );
            TForm& form = fixture.form;
            CEventCount requested( &form, TForm::UnlockRequestedEvent );
            CEventCount done( &form, TForm::DoneEvent );
            CEventCount cancelled( &form, TForm::CancelledEvent );
            ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() );
            ASSERT_TRUE( TForm::MODE_UNLOCK == form.GetMode() );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidgetAs< GUI::CSecretInput >( "Code" ) );
            ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Confirm" ) && GUCEF_NULL == form.GetWidget( "Current" ) && GUCEF_NULL == form.GetWidget( "Strength" ) );
            ASSERT_TRUE( "Unlock" == form.GetWidget( "Submit" )->GetProperty( "Text" ) && "Cancel" == form.GetWidget( "Cancel" )->GetProperty( "Text" ) );
            ASSERT_TRUE( IsWidgetEnabled( form, "Submit" ) );
            ASSERT_TRUE( form.GetMessageText().IsNULLOrEmpty() && form.GetStrengthText().IsNULLOrEmpty() );

            // nothing typed
            ClickWidget( fixture.context, form.GetWidget( "Submit" ) );
            ASSERT_TRUE( "Enter the keyring code." == form.GetMessageText() && 0 == requested.count );

            // a wrong code with Enter: the form waits for the pool, then says so and empties the field
            fixture.Type( "Code", WrongCode, true );
            ASSERT_TRUE( 1 == requested.count );
            ASSERT_TRUE( form.IsBusy() || !form.GetMessageText().IsNULLOrEmpty() );
            if ( form.IsBusy() )
            {
                ASSERT_FALSE( IsWidgetEnabled( form, "Submit" ) );
                ASSERT_TRUE( "Checking the code..." == form.GetMessageText() );
            }
            ASSERT_TRUE( 0 == FieldLength( form, "Code" ) );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( "Wrong code." == form.GetMessageText() && "Wrong code." == form.GetWidget( "Message" )->GetProperty( "Text" ) );
            ASSERT_TRUE( form.IsOpen() && IsWidgetEnabled( form, "Submit" ) && fixture.session.IsLocked() );
            ASSERT_TRUE( 0 == done.count );

            // the right code with the button
            fixture.Type( "Code", RightCode, false );
            ASSERT_TRUE( 0 < FieldLength( form, "Code" ) );
            ClickWidget( fixture.context, form.GetWidget( "Submit" ) );
            ASSERT_TRUE( 2 == requested.count );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( 1 == done.count && 0 == cancelled.count );
            ASSERT_FALSE( form.IsOpen() || form.IsVisible() );
            ASSERT_FALSE( fixture.session.IsLocked() );
            AUTH::CSecretString secret;
            ASSERT_TRUE( fixture.store.Load( "fab", "owner", secret ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: An operation that is done on the pool before Submit looks at its future is no refusal: Submit starts it, the form waits for the outcome and shows it, in every mode" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            // unlock: a wrong code, then the right one
            {
                CKeyringFormFixture fixture( "completed_unlock", true );
                REQUIRE_TRUE( fixture.IsReady() );
                fixture.session.completeBeforeReturn = true;
                TForm& form = fixture.form;
                CEventCount done( &form, TForm::DoneEvent );
                REQUIRE_TRUE( form.Open() && TForm::MODE_UNLOCK == form.GetMode() );

                fixture.Type( "Code", WrongCode, false );
                ASSERT_TRUE( form.Submit() );
                ASSERT_TRUE( 1 == fixture.session.completedBeforeReturn );
                ASSERT_TRUE( form.IsBusy() && "Checking the code..." == form.GetMessageText() );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                ASSERT_TRUE( "Wrong code." == form.GetMessageText() && form.IsOpen() && 0 == done.count );
                ASSERT_TRUE( 1 == fixture.session.GetFailedAttemptCount() && fixture.session.IsLocked() );

                fixture.Type( "Code", RightCode, false );
                ASSERT_TRUE( form.Submit() );
                ASSERT_TRUE( 2 == fixture.session.completedBeforeReturn );
                ASSERT_TRUE( form.IsBusy() );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                ASSERT_TRUE( 1 == done.count && !form.IsOpen() );
                ASSERT_FALSE( fixture.session.IsLocked() );
            }

            // set up, then change the code
            {
                CKeyringFormFixture fixture( "completed_setup", false );
                REQUIRE_TRUE( fixture.IsReady() );
                fixture.session.completeBeforeReturn = true;
                TForm& form = fixture.form;
                CEventCount done( &form, TForm::DoneEvent );
                REQUIRE_TRUE( form.Open( TForm::MODE_SET_UP ) && TForm::MODE_SET_UP == form.GetMode() );
                fixture.Type( "Code", RightCode, false );
                fixture.Type( "Confirm", RightCode, false );
                ASSERT_TRUE( form.Submit() );
                ASSERT_TRUE( 1 == fixture.session.completedBeforeReturn && form.IsBusy() );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                ASSERT_TRUE( 1 == done.count && !form.IsOpen() );
                ASSERT_TRUE( fixture.session.IsSetUp() && !fixture.session.IsLocked() );

                REQUIRE_TRUE( form.Open( TForm::MODE_CHANGE_CODE ) && TForm::MODE_CHANGE_CODE == form.GetMode() );
                fixture.Type( "Current", RightCode, false );
                fixture.Type( "Code", NewCode, false );
                fixture.Type( "Confirm", NewCode, false );
                ASSERT_TRUE( form.Submit() );
                ASSERT_TRUE( 2 == fixture.session.completedBeforeReturn && form.IsBusy() );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                ASSERT_TRUE( 2 == done.count && !form.IsOpen() );
                fixture.session.LockNow();
                ASSERT_TRUE( AUTH::KEYRING_OK == fixture.session.UnlockWithCode( AUTH::CSecretString( NewCode ) ) );
            }

            // a refusal still is one: without a store the future ends without starting, Submit says so and does not wait
            {
                CKeyringFormFixture fixture( "completed_refused", true );
                REQUIRE_TRUE( fixture.IsReady() );
                fixture.session.completeBeforeReturn = true;
                TForm& form = fixture.form;
                REQUIRE_TRUE( form.Open() );
                fixture.session.SetStore( GUCEF_NULL );
                fixture.Type( "Code", RightCode, false );
                ASSERT_FALSE( form.Submit() );
                ASSERT_FALSE( form.IsBusy() );
                ASSERT_TRUE( 0 == fixture.session.completedBeforeReturn );
                ASSERT_TRUE( "The saved credentials could not be opened." == form.GetMessageText() && form.IsOpen() );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: After wrong codes it shows the wait before the next try, counts it down with Submit disabled, and allows a try again when it is over" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CKeyringFormFixture fixture( "wait", true );
            REQUIRE_TRUE( fixture.IsReady() );
            AUTH::CKeyringSettings settings = fixture.session.GetSettings();
            settings.freeAttempts = 1;
            settings.retryDelayInMs = 2500;
            settings.maxRetryDelayInMs = 2500;
            fixture.session.SetSettings( settings );
            TForm& form = fixture.form;
            REQUIRE_TRUE( form.Open() );

            fixture.Type( "Code", WrongCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( "Wrong code." == form.GetMessageText() && !form.IsWaiting() );
            fixture.Type( "Code", WrongCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( form.IsWaiting() );
            ASSERT_TRUE( "Wrong code. Wait 3 seconds before the next try." == form.GetMessageText() );
            ASSERT_FALSE( IsWidgetEnabled( form, "Submit" ) );
            // the button is disabled; asked anyway, nothing starts
            fixture.Type( "Code", RightCode, false );
            ASSERT_FALSE( form.Submit() );
            ASSERT_TRUE( 2 == fixture.session.GetFailedAttemptCount() && fixture.session.IsLocked() );
            // the count down
            bool sawTwo = false;
            for ( UInt32 waited=0; waited<6000 && form.IsWaiting(); waited+=10 )
            {
                PumpFor( 10 );
                fixture.context->RenderFrame( 10 );
                if ( "Wrong code. Wait 2 seconds before the next try." == form.GetMessageText() )
                    sawTwo = true;
            }
            ASSERT_TRUE( sawTwo );
            ASSERT_FALSE( form.IsWaiting() );
            fixture.Settle();
            ASSERT_TRUE( "Wrong code. Try again." == form.GetMessageText() && IsWidgetEnabled( form, "Submit" ) );
            // a form opened during a wait shows it at once
            form.Close();
            fixture.Type( "Code", WrongCode, false );
            REQUIRE_TRUE( form.Open() );
            fixture.Type( "Code", WrongCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            form.Close();
            REQUIRE_TRUE( form.Open() );
            ASSERT_TRUE( form.IsWaiting() && form.GetMessageText().HasSubstr( "before the next try" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: Without a keyring the form sets up a code: the code twice, the minimum length, the strength hint; done unlocks the new keyring" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CKeyringFormFixture fixture( "setup", false );
            REQUIRE_TRUE( fixture.IsReady() );
            TForm& form = fixture.form;
            CEventCount done( &form, TForm::DoneEvent );
            ASSERT_TRUE( form.Open( TForm::MODE_UNLOCK ) );
            ASSERT_TRUE( TForm::MODE_SET_UP == form.GetMode() );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Code" ) && GUCEF_NULL != form.GetWidget( "Confirm" ) && GUCEF_NULL != form.GetWidget( "Strength" ) && GUCEF_NULL != form.GetWidget( "Note" ) );
            ASSERT_TRUE( "Set up" == form.GetWidget( "Submit" )->GetProperty( "Text" ) );
            ASSERT_TRUE( TForm::STRENGTH_NONE == form.GetStrength() && form.GetWidget( "Strength" )->GetProperty( "Text" ).IsNULLOrEmpty() );

            fixture.Type( "Code", "abc", false );
            ASSERT_TRUE( TForm::STRENGTH_TOO_SHORT == form.GetStrength() && "Too short" == form.GetStrengthText() );
            ASSERT_TRUE( form.GetWidget( "Strength" )->GetProperty( "Text" ).HasSubstr( "at least 8" ) >= 0 );
            fixture.Type( "Confirm", "abc", true );
            ASSERT_TRUE( "Use at least 8 characters." == form.GetMessageText() );
            ASSERT_FALSE( fixture.session.IsSetUp() );

            fixture.Type( "Code", "defghijklmnopqrstuvw", false );
            ASSERT_TRUE( TForm::STRENGTH_STRONG == form.GetStrength() && "Strength: Strong" == form.GetWidget( "Strength" )->GetProperty( "Text" ) );
            form.ClearSecrets();
            fixture.Type( "Code", NewCode, false );
            fixture.Type( "Confirm", RightCode, true );
            ASSERT_TRUE( "The two new codes are not the same." == form.GetMessageText() );
            ASSERT_TRUE( 0 == FieldLength( form, "Confirm" ) && 0 < FieldLength( form, "Code" ) );
            fixture.Type( "Confirm", NewCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( 1 == done.count && !form.IsOpen() );
            ASSERT_TRUE( fixture.session.IsSetUp() && !fixture.session.IsLocked() );
            fixture.session.LockNow();
            ASSERT_TRUE( AUTH::KEYRING_OK == fixture.session.UnlockWithCode( AUTH::CSecretString( NewCode ) ) );

            // the hint
            ASSERT_TRUE( TForm::STRENGTH_NONE == TForm::RateCode( MakeSecret( "" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_TOO_SHORT == TForm::RateCode( MakeSecret( "Ab1!" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_WEAK == TForm::RateCode( MakeSecret( "abcdefgh" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_FAIR == TForm::RateCode( MakeSecret( "abcdefg1" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_GOOD == TForm::RateCode( MakeSecret( "abcdefghijkl" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_GOOD == TForm::RateCode( MakeSecret( "abcdEFG12!" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_STRONG == TForm::RateCode( MakeSecret( "abcdefghijk L1" ), 8 ) );
            // code points, not bytes: eight letters with accents are eight
            ASSERT_TRUE( TForm::STRENGTH_TOO_SHORT == TForm::RateCode( MakeSecret( "\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9" ), 8 ) );
            ASSERT_TRUE( TForm::STRENGTH_TOO_SHORT != TForm::RateCode( MakeSecret( "\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9\xc3\xa9" ), 8 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: Change of code: the current code, the new code twice; a wrong current code is a wrong code; done reseals the keyring" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CKeyringFormFixture fixture( "change", true );
            REQUIRE_TRUE( fixture.IsReady() );
            TForm& form = fixture.form;
            CEventCount done( &form, TForm::DoneEvent );
            ASSERT_TRUE( form.Open( TForm::MODE_CHANGE_CODE ) );
            ASSERT_TRUE( TForm::MODE_CHANGE_CODE == form.GetMode() );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Current" ) && GUCEF_NULL != form.GetWidget( "Code" ) && GUCEF_NULL != form.GetWidget( "Confirm" ) );
            ASSERT_TRUE( "Change code" == form.GetWidget( "Submit" )->GetProperty( "Text" ) );

            fixture.Type( "Code", NewCode, false );
            fixture.Type( "Confirm", NewCode, false );
            ASSERT_FALSE( form.Submit() );
            ASSERT_TRUE( "Enter the current code." == form.GetMessageText() );
            fixture.Type( "Current", WrongCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( "Wrong code." == form.GetMessageText() && 0 == done.count && form.IsOpen() );
            ASSERT_TRUE( 0 == FieldLength( form, "Current" ) && 0 == FieldLength( form, "Code" ) );

            fixture.Type( "Current", RightCode, false );
            fixture.Type( "Code", NewCode, false );
            fixture.Type( "Confirm", NewCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( 1 == done.count && !form.IsOpen() );
            fixture.session.LockNow();
            ASSERT_TRUE( AUTH::KEYRING_WRONG_CODE == fixture.session.UnlockWithCode( AUTH::CSecretString( RightCode ) ) );
            ASSERT_TRUE( AUTH::KEYRING_OK == fixture.session.UnlockWithCode( AUTH::CSecretString( NewCode ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: Cancel and Escape close with CancelledEvent and wipe the fields; a tampered keyring shows its own message" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CKeyringFormFixture fixture( "cancel", true );
            REQUIRE_TRUE( fixture.IsReady() );
            TForm& form = fixture.form;
            CEventCount cancelled( &form, TForm::CancelledEvent );
            CEventCount done( &form, TForm::DoneEvent );
            REQUIRE_TRUE( form.Open() );
            fixture.Type( "Code", WrongCode, false );
            GUI::CSecretInput* code = form.GetWidgetAs< GUI::CSecretInput >( "Code" );
            ASSERT_TRUE( GUCEF_NULL != code && code->HasValue() );
            ClickWidget( fixture.context, form.GetWidget( "Cancel" ) );
            ASSERT_TRUE( 1 == cancelled.count && !form.IsOpen() && !form.IsVisible() );
            ASSERT_FALSE( code->HasValue() );

            REQUIRE_TRUE( form.Open() );
            fixture.Type( "Code", WrongCode, false );
            // Caps Lock: a hint on the line above the buttons, gone when it is off again
            fixture.context->InjectKey( INPUT::KEYCODE_A, true, INPUT::KEYMOD_CAPSLOCK );
            fixture.context->InjectKey( INPUT::KEYCODE_A, false, INPUT::KEYMOD_CAPSLOCK );
            ASSERT_TRUE( "Caps Lock is on." == form.GetMessageText() );
            fixture.context->InjectKey( INPUT::KEYCODE_A, true, 0 );
            fixture.context->InjectKey( INPUT::KEYCODE_A, false, 0 );
            ASSERT_TRUE( form.GetMessageText().IsNULLOrEmpty() );
            PressKey( fixture.context, INPUT::KEYCODE_ESCAPE );
            fixture.Settle();
            ASSERT_TRUE( 2 == cancelled.count && !form.IsOpen() );
            ASSERT_TRUE( 0 == done.count && fixture.session.IsLocked() );

            // the data of the keyring changed: the right code says so, apart from a wrong code
            CORE::CString path = CORE::CombinePath( fixture.store.GetDirectory(), AUTH::CFileCredentialStore::KeyringFileName );
            CORE::CDynamicBuffer file;
            REQUIRE_TRUE( file.LoadContentFromFile( path ) );
            static_cast< UInt8* >( file.GetBufferPtr() )[ file.GetDataSize() - 3 ] ^= 0x10;
            REQUIRE_TRUE( file.WriteContentToFile( path, 0, true ) );
            REQUIRE_TRUE( form.Open() );
            fixture.Type( "Code", RightCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( "The saved credentials were changed and cannot be read." == form.GetMessageText() );
            ASSERT_TRUE( form.IsOpen() && 0 == done.count );
            fixture.Type( "Code", WrongCode, true );
            ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
            ASSERT_TRUE( "Wrong code." == form.GetMessageText() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: Without a session the application takes the code in UnlockRequestedEvent and reports with ShowResult()" )
        try
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1280, 960, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            TForm form( context );
            CCodeTaker taker( form );
            CEventCount done( &form, TForm::DoneEvent );
            REQUIRE_TRUE( form.Open( TForm::MODE_CHANGE_CODE ) );
            context->FlushChanges();
            context->SetFocus( form.GetWidget( "Current" ) );
            context->InjectText( RightCode );
            context->SetFocus( form.GetWidget( "Code" ) );
            context->InjectText( NewCode );
            context->SetFocus( form.GetWidget( "Confirm" ) );
            context->InjectText( NewCode );
            ASSERT_TRUE( form.Submit() );
            ASSERT_TRUE( 1 == taker.requests && RightCode == taker.code.Reveal() && NewCode == taker.newCode.Reveal() );
            ASSERT_TRUE( 0 == FieldLength( form, "Current" ) && 0 == FieldLength( form, "Code" ) && 0 == FieldLength( form, "Confirm" ) );
            ASSERT_FALSE( form.IsBusy() );

            form.ShowResult( AUTH::KEYRING_WRONG_CODE, 1200 );
            ASSERT_TRUE( form.IsWaiting() && "Wrong code. Wait 2 seconds before the next try." == form.GetMessageText() );
            ASSERT_FALSE( IsWidgetEnabled( form, "Submit" ) );
            for ( UInt32 waited=0; waited<4000 && form.IsWaiting(); waited+=10 )
            {
                PumpFor( 10 );
                context->RenderFrame( 10 );
            }
            ASSERT_FALSE( form.IsWaiting() );
            form.ShowResult( AUTH::KEYRING_UNAVAILABLE );
            ASSERT_TRUE( "This build cannot open saved credentials." == form.GetMessageText() );
            form.ShowResult( AUTH::KEYRING_OK );
            ASSERT_TRUE( 1 == done.count && !form.IsOpen() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: No code shows in a text of a widget or in the log, typed, submitted, wrong or right" )
        GUCEF_TESTFW_SKIP_IF( !available, "The argon2id or xchacha20-poly1305 codec is not available" );
        try
        {
            CORE::CString logged;
            {
                CFormLogCapture capture;
                CKeyringFormFixture fixture( "secrets", true );
                REQUIRE_TRUE( fixture.IsReady() );
                TForm& form = fixture.form;
                REQUIRE_TRUE( form.Open() );
                fixture.Type( "Code", WrongCode, false );
                fixture.Settle();
                ASSERT_FALSE( AnyTextHas( form.GetRootWidget(), WrongCode ) );
                PressKey( fixture.context, INPUT::KEYCODE_RETURN );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                fixture.Type( "Code", RightCode, false );
                ASSERT_FALSE( AnyTextHas( form.GetRootWidget(), RightCode ) );
                PressKey( fixture.context, INPUT::KEYCODE_RETURN );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                ASSERT_FALSE( fixture.session.IsLocked() );
                REQUIRE_TRUE( form.Open( TForm::MODE_CHANGE_CODE ) );
                fixture.Type( "Current", RightCode, false );
                fixture.Type( "Code", NewCode, false );
                fixture.Type( "Confirm", NewCode, false );
                fixture.Settle();
                ASSERT_FALSE( AnyTextHas( form.GetRootWidget(), RightCode ) || AnyTextHas( form.GetRootWidget(), NewCode ) );
                ASSERT_TRUE( form.Submit() );
                ASSERT_TRUE( fixture.SettleUntilIdle( 20000 ) );
                logged = capture.Text();
            }
            ASSERT_TRUE( logged.Length() > 0 );
            ASSERT_FALSE( logged.HasSubstr( RightCode ) >= 0 );
            ASSERT_FALSE( logged.HasSubstr( WrongCode ) >= 0 );
            ASSERT_FALSE( logged.HasSubstr( NewCode ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "KeyringUnlockForm: Every mode fits from 640 x 480 to 5120 x 1440 and the portrait 600 x 900: in the middle, nothing outside its parent, nothing overlaps" )
        try
        {
            const SScreenSize sizes[] = { { 640, 480 }, { 1024, 768 }, { 1920, 1080 }, { 3440, 1440 }, { 5120, 1440 }, { 600, 900 } };
            const TForm::EMode modes[ 3 ] = { TForm::MODE_UNLOCK, TForm::MODE_SET_UP, TForm::MODE_CHANGE_CODE };
            const char* modeNames[ 3 ] = { "unlock", "setup", "change" };
            for ( UInt32 s=0; s<6; ++s )
            {
                for ( UInt32 m=0; m<3; ++m )
                {
                    const CORE::CString where = SizeName( sizes[ s ].width, sizes[ s ].height );
                    GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, true );
                    GUI::CGUIContext* context = ContextOf( contextPtr );
                    REQUIRE_TRUE( GUCEF_NULL != context );
                    TForm form( context );
                    REQUIRE_TRUE( form.Open( modes[ m ] ) );
                    form.ShowResult( AUTH::KEYRING_WRONG_CODE, 61000 );
                    context->RenderFrame( 16 );
                    context->FlushChanges();
                    context->RenderFrame( 16 );
                    GUI::CWidget* window = form.GetRootWidget();
                    REQUIRE_TRUE( GUCEF_NULL != window );
                    GUI::CRect rect = window->GetRect();
                    ASSERT_TRUE( Inside( rect, GUI::CRect( 0, 0, (Int32) sizes[ s ].width, (Int32) sizes[ s ].height ) ) );
                    ASSERT_TRUE( rect.width >= 360 && rect.width <= 520 );
                    Int32 centerX = rect.left + rect.width / 2;
                    ASSERT_TRUE( centerX >= (Int32) sizes[ s ].width / 2 - 1 && centerX <= (Int32) sizes[ s ].width / 2 + 1 );
                    CORE::CString problem = FindLayoutProblem( window );
                    if ( !problem.IsNULLOrEmpty() )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layouts: the keyring form (" << modeNames[ m ] << ") at " << where << ": " << problem );
                    ASSERT_TRUE( problem.IsNULLOrEmpty() );
                    IMAGE::TPixelMapPtr frame = CaptureFrame( context );
                    WriteShot( frame, CORE::CString( "keyring-" ) + modeNames[ m ] + "-" + where );
                }
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
