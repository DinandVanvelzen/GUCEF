/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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

#ifndef GUCEF_CORE_BROWSERLAUNCHER_H
#include "gucefCORE_BrowserLauncher.h"
#define GUCEF_CORE_BROWSERLAUNCHER_H
#endif /* GUCEF_CORE_BROWSERLAUNCHER_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

#ifndef GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#include "gucefAUTH_CAuthProviderRegistry.h"
#define GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERREGISTRY_H ? */

#ifndef GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#include "gucefAUTH_CMemoryCredentialStore.h"
#define GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H
#endif /* GUCEF_AUTH_CMEMORYCREDENTIALSTORE_H ? */

#ifndef GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#include "gucefAUTH_CLocalIdentityLinkStore.h"
#define GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H
#endif /* GUCEF_AUTH_CLOCALIDENTITYLINKSTORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefAUTHGUI_TestCommon.h"
#include "TestFakeLoginProvider.h"
#include "TestSigner.h"
#include "TestRecordingBrowser.h"
#include "TestLoginFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CFakeLoginProvider::SecretAccessToken = "SECRET-ACCESS-TOKEN-7f3a91c2";
const CORE::CString CFakeLoginProvider::SecretRefreshToken = "SECRET-REFRESH-TOKEN-b04d55e8";
const CORE::CString CFakeLoginProvider::SecretProof = "SECRET-PROOF-d1e6a7c0";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records what the player does with a form */
class CLoginFormRecorder : public CORE::CObserver
{
    public:

    UInt32 closed;
    UInt32 autoLoginChanges;

    typedef CORE::CTEventHandlerFunctor< CLoginFormRecorder > TEventCallback;

    CLoginFormRecorder( CORE::CNotifier* form ) : CORE::CObserver(), closed( 0 ), autoLoginChanges( 0 )
    {
        TEventCallback closedCallback( this, &CLoginFormRecorder::OnClosed );
        SubscribeTo( form, AUTHGUI::CLoginForm::ClosedEvent, closedCallback );
        TEventCallback autoLoginCallback( this, &CLoginFormRecorder::OnAutoLoginChanged );
        SubscribeTo( form, AUTHGUI::CLoginForm::AutoLoginChangedEvent, autoLoginCallback );
    }

    virtual ~CLoginFormRecorder() { UnsubscribeAllFromObserver(); }

    void OnClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++closed;
    }

    void OnAutoLoginChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++autoLoginChanges;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Every text a widget of the form shows or holds */
void
CollectTexts( GUI::CWidget* widget, CORE::CString::StringVector& texts )
{
    texts.push_back( widget->GetProperty( "Text" ) );
    const GUI::CWidget::TWidgetVector& children = widget->GetChildren();
    for ( UInt32 i=0; i<(UInt32)children.size(); ++i )
        CollectTexts( children[ i ], texts );
}

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

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformLoginFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.LoginForm" );

    GUCEF_TESTFW_TESTCASE( "LoginForm: The profile key is needed for links, the plugin for it is loaded" )
    try
    {
        LoadTestPlugin( "codecspluginMONOCYPHER", "codecs" );
        CTestSigner signer;
        ASSERT_TRUE( signer.IsValid() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: It offers the providers the manager offers, named after their descriptors, and a guest has no accounts" )
    try
    {
        // a descriptor gives a provider its name on the button
        AUTH::CAuthProviderDescriptor descriptor;
        descriptor.name = "fakebrowser";
        descriptor.displayName = "Fake Browser";
        descriptor.accountProviderId = "fake";
        descriptor.protocol = AUTH::AUTHPROTOCOL_PLATFORM;
        REQUIRE_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().RegisterDescriptor( descriptor ) );

        {
            CLoginFixture fixture;
            REQUIRE_TRUE( fixture.IsReady() );
            CFakeLoginProvider* browser = fixture.AddProvider( "fakebrowser" );
            CFakeLoginProvider* plain = fixture.AddProvider( "plainprov" );
            CFakeLoginProvider* hidden = fixture.AddProvider( "hidden" );
            REQUIRE_TRUE( GUCEF_NULL != browser && GUCEF_NULL != plain && GUCEF_NULL != hidden );
            // needs a client id the application did not configure: not offered
            hidden->capabilities |= AUTH::AUTHCAP_REQUIRES_CLIENT_ID;

            AUTHGUI::CLoginForm& form = fixture.form;
            ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() );
            CORE::CString::StringVector shown;
            form.GetShownProviders( shown );
            ASSERT_TRUE( 2 == shown.size() && "fakebrowser" == shown[ 0 ] && "plainprov" == shown[ 1 ] );
            ASSERT_TRUE( "Log in with Fake Browser" == form.GetWidget( "Login0" )->GetProperty( "Text" ) );
            ASSERT_TRUE( "Log in with Plainprov" == form.GetWidget( "Login1" )->GetProperty( "Text" ) );
            ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Login2" ) );
            ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_IDLE == form.GetProgress( "fakebrowser" ) );

            // a guest: no accounts, the state line says so, the check box has the setting
            ASSERT_TRUE( "Guest" == form.GetStateText() && "Guest" == form.GetWidget( "State" )->GetProperty( "Text" ) );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "NoAccounts" ) && GUCEF_NULL == form.GetWidget( "Unlink0" ) );
            ASSERT_TRUE( form.GetWidget( "AutoLogin" )->GetBoolProperty( "Checked" ) && form.GetAutoLogin() );

            // the provider gets its client id: it is offered
            AUTH::CAuthSettings settings( fixture.manager.GetSettings() );
            AUTH::CAuthProviderSettings providerSettings;
            providerSettings.clientId = "client-id-of-the-game";
            settings.SetProviderSettings( "hidden", providerSettings );
            fixture.manager.SetSettings( settings );
            form.Refresh();
            form.GetShownProviders( shown );
            ASSERT_TRUE( 3 == shown.size() && "hidden" == shown[ 1 ] && "Log in with Hidden" == form.GetWidget( "Login1" )->GetProperty( "Text" ) );
            ASSERT_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().GetCount() > 0 );
        }

        // without a manager there is nothing to log in with
        {
            GUI::TGuiContextPtr contextPtr = CreateFormContext();
            GUI::CGUIContext* context = ContextOf( contextPtr );
            REQUIRE_TRUE( GUCEF_NULL != context );
            AUTHGUI::CLoginForm form( context );
            ASSERT_TRUE( form.Open() );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "NoProviders" ) && "Guest" == form.GetStateText() );
            ASSERT_FALSE( form.Login( "anything" ) );
        }

        // the names of providers without a descriptor, and of the namespaces of accounts
        ASSERT_TRUE( "Fake Browser" == AUTH::GetProviderDisplayName( "fakebrowser" ) && "Plainprov" == AUTH::GetProviderDisplayName( "plainprov" ) );
        ASSERT_TRUE( "Fake Browser" == AUTH::GetAccountProviderDisplayName( "fake" ) && "Zzz" == AUTH::GetAccountProviderDisplayName( "zzz" ) );
        ASSERT_TRUE( "Fakeopenid" == AUTH::GetProviderDisplayName( "fakeopenid" ) && "Fakeopenid" == AUTH::GetAccountProviderDisplayName( "fakeopenid" ) );
        ASSERT_TRUE( "AWS" == AUTH::GetAccountProviderDisplayName( "aws" ) );
        ASSERT_TRUE( "Google: 1234" == AUTHGUI::CLoginForm::MakeAccountText( AUTH::CAccountRef( "google", "1234" ) ) );
        CORE::CString longText = AUTHGUI::CLoginForm::MakeAccountText( AUTH::CAccountRef( "aws", "eu-west-1_AbCdEfGhI:2f6c9d1e-0000-1111-2222-333344445555-and-then-some" ) );
        ASSERT_TRUE( AUTHGUI::CLoginForm::MaxShownTextLength == longText.Length() && longText.HasSubstr( "..." ) >= 0 );

        AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "fakebrowser" );
    }
    catch ( ... )
    {
        ERRORHERE;
        AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "fakebrowser" );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: A login with the browser: waiting with a Cancel button, then done, the account is linked and can be unlinked" )
    try
    {
        CLoginFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        REQUIRE_TRUE( GUCEF_NULL != google );
        google->mode = CFakeLoginProvider::MODE_BROWSER;
        google->accountNamespace = "google";
        google->subject = "109876543210";
        AUTHGUI::CLoginForm& form = fixture.form;
        CLoginFormRecorder recorder( &form );
        REQUIRE_TRUE( form.Open() );
        ASSERT_TRUE( "Log in with Google" == form.GetWidget( "Login0" )->GetProperty( "Text" ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Cancel0" ) );

        // the button starts the login, the provider asks for the browser, the form waits with a Cancel button
        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        ASSERT_TRUE( 1 == google->beginCount && google->lastOptions.linkToProfile && !google->lastOptions.preferDeviceCode );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_WAITING_FOR_BROWSER == form.GetProgress( "google" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Cancel0" ) && !IsWidgetEnabled( form, "Login0" ) );
        ASSERT_TRUE( form.GetWidget( "Status0" )->GetProperty( "Text" ).HasSubstr( "Waiting" ) >= 0 );
        // pressing it again does not start another
        ASSERT_FALSE( form.Login( "google" ) );
        ASSERT_TRUE( 1 == google->beginCount );

        // the player is done in the browser
        ASSERT_TRUE( google->Succeed() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_DONE == form.GetProgress( "google" ) && "Fake Player" == form.GetProgressMessage( "google" ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Cancel0" ) && IsWidgetEnabled( form, "Login0" ) );
        ASSERT_TRUE( "Logged in" == form.GetWidget( "Status0" )->GetProperty( "Text" ) );
        ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == fixture.manager.GetLoginState() );
        ASSERT_TRUE( "Linked: Google" == form.GetStateText() && "Linked: Google" == form.GetWidget( "State" )->GetProperty( "Text" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Unlink0" ) && GUCEF_NULL == form.GetWidget( "NoAccounts" ) );
        ASSERT_TRUE( "Google: 109876543210" == form.GetWidget( "AccountText0" )->GetProperty( "Text" ) );

        // no token of the login shows anywhere
        ASSERT_FALSE( AnyTextHas( form.GetRootWidget(), CFakeLoginProvider::SecretAccessToken ) );
        ASSERT_FALSE( AnyTextHas( form.GetRootWidget(), CFakeLoginProvider::SecretRefreshToken ) );
        ASSERT_FALSE( AnyTextHas( form.GetRootWidget(), CFakeLoginProvider::SecretProof ) );

        // unlinking removes the account
        ClickWidget( fixture.context, form.GetWidget( "Unlink0" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == fixture.manager.GetLoginState() && "Guest" == form.GetStateText() );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Unlink0" ) && GUCEF_NULL != form.GetWidget( "NoAccounts" ) );
        ASSERT_TRUE( 0 == recorder.closed );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: Cancelling a login that waits ends it, nothing is linked, the button is back" )
    try
    {
        CLoginFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        REQUIRE_TRUE( GUCEF_NULL != google );
        google->mode = CFakeLoginProvider::MODE_BROWSER;
        AUTHGUI::CLoginForm& form = fixture.form;
        REQUIRE_TRUE( form.Open() );

        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_WAITING_FOR_BROWSER == form.GetProgress( "google" ) && google->IsPending() );
        ClickWidget( fixture.context, form.GetWidget( "Cancel0" ) );
        ASSERT_TRUE( google->HonourCancel() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_IDLE == form.GetProgress( "google" ) && form.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Cancel0" ) && IsWidgetEnabled( form, "Login0" ) );
        ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == fixture.manager.GetLoginState() && "Guest" == form.GetStateText() );

        // and it can be tried again
        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( 2 == google->beginCount && AUTHGUI::CLoginForm::LOGINPROGRESS_WAITING_FOR_BROWSER == form.GetProgress( "google" ) );
        ASSERT_TRUE( google->Succeed() );
        fixture.Settle();
        ASSERT_TRUE( AUTH::AUTHLOGINSTATE_LINKED == fixture.manager.GetLoginState() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: A device code is shown with its page in two fields to copy from, Open goes to the page, Cancel ends it" )
    try
    {
        CLoginFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* microsoft = fixture.AddProvider( "microsoft" );
        REQUIRE_TRUE( GUCEF_NULL != microsoft );
        microsoft->mode = CFakeLoginProvider::MODE_DEVICE_CODE;
        microsoft->accountNamespace = "microsoft";
        AUTHGUI::CLoginForm& form = fixture.form;
        form.SetPreferDeviceCode( true );
        REQUIRE_TRUE( form.Open() );
        CRecordingBrowser browser;

        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        ASSERT_TRUE( microsoft->lastOptions.preferDeviceCode );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_SHOWING_DEVICE_CODE == form.GetProgress( "microsoft" ) );
        ASSERT_TRUE( "WDJB-MJHT" == form.GetDeviceCode( "microsoft" ) && "https://example.com/activate" == form.GetDeviceCodeUrl( "microsoft" ) );

        // the page and the code are in fields the player can select and copy, and cannot change
        GUI::CWidget* url = form.GetWidget( "Url0" );
        GUI::CWidget* code = form.GetWidget( "Code0" );
        REQUIRE_TRUE( GUCEF_NULL != url && GUCEF_NULL != code && GUCEF_NULL != form.GetWidget( "Open0" ) && GUCEF_NULL != form.GetWidget( "Cancel0" ) );
        ASSERT_TRUE( "Editbox" == url->GetType() && "Editbox" == code->GetType() && url->GetBoolProperty( "ReadOnly" ) && code->GetBoolProperty( "ReadOnly" ) );
        ASSERT_TRUE( "https://example.com/activate" == url->GetProperty( "Text" ) && "WDJB-MJHT" == code->GetProperty( "Text" ) );
        ASSERT_TRUE( code->GetBoolProperty( "SelectAllOnFocus" ) && url->GetBoolProperty( "SelectAllOnFocus" ) );

        // Open goes to the page, with the code in it
        ClickWidget( fixture.context, form.GetWidget( "Open0" ) );
        ASSERT_TRUE( 1 == browser.urls.size() && "https://example.com/activate?user_code=WDJB-MJHT" == browser.urls[ 0 ] );
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_SHOWING_DEVICE_CODE == form.GetProgress( "microsoft" ) );
        browser.accept = false;
        ASSERT_FALSE( form.OpenDeviceCodePage( "microsoft" ) );
        ASSERT_TRUE( form.GetErrorText().HasSubstr( "browser" ) >= 0 );
        ASSERT_FALSE( form.OpenDeviceCodePage( "nosuchprovider" ) );

        // the player entered the code elsewhere
        ASSERT_TRUE( microsoft->Succeed() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_DONE == form.GetProgress( "microsoft" ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Code0" ) );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Open0" ) );
        ASSERT_TRUE( form.GetDeviceCode( "microsoft" ).IsNULLOrEmpty() );

        // another time, cancelled
        ASSERT_TRUE( fixture.manager.Unlink( AUTH::CAccountRef( "microsoft", "player-1" ) ) );
        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_SHOWING_DEVICE_CODE == form.GetProgress( "microsoft" ) );
        ClickWidget( fixture.context, form.GetWidget( "Cancel0" ) );
        ASSERT_TRUE( microsoft->HonourCancel() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_IDLE == form.GetProgress( "microsoft" ) && GUCEF_NULL == form.GetWidget( "Code0" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: A login that fails says why, in the line at the bottom and red in its row; one that cannot start says so" )
    try
    {
        CLoginFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        CFakeLoginProvider* amazon = fixture.AddProvider( "amazon" );
        REQUIRE_TRUE( GUCEF_NULL != google && GUCEF_NULL != amazon );
        google->mode = CFakeLoginProvider::MODE_FAIL;
        amazon->mode = CFakeLoginProvider::MODE_REFUSE;
        AUTHGUI::CLoginForm& form = fixture.form;
        REQUIRE_TRUE( form.Open() );
        ASSERT_TRUE( "Log in with Amazon" == form.GetWidget( "Login0" )->GetProperty( "Text" ) && "Log in with Google" == form.GetWidget( "Login1" )->GetProperty( "Text" ) );

        ClickWidget( fixture.context, form.GetWidget( "Login1" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_FAILED == form.GetProgress( "google" ) );
        ASSERT_TRUE( form.GetProgressMessage( "google" ).HasSubstr( "access denied" ) >= 0 );
        ASSERT_TRUE( form.GetErrorText().HasSubstr( "Google" ) >= 0 && form.GetErrorText().HasSubstr( "access denied" ) >= 0 );
        ASSERT_TRUE( form.GetWidget( "Error" )->GetProperty( "Text" ) == form.GetErrorText() );
        ASSERT_TRUE( "Failed" == form.GetWidget( "Status1" )->GetProperty( "Text" ) && IsWidgetEnabled( form, "Login1" ) );
        ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == fixture.manager.GetLoginState() );

        // another one cannot start
        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_FAILED == form.GetProgress( "amazon" ) && !form.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( form.GetErrorText().HasSubstr( "Amazon" ) >= 0 );
        ASSERT_TRUE( IsWidgetEnabled( form, "Login0" ) && GUCEF_NULL == form.GetWidget( "Cancel0" ) );

        // a try that works clears the line
        google->mode = CFakeLoginProvider::MODE_INSTANT;
        ClickWidget( fixture.context, form.GetWidget( "Login1" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_DONE == form.GetProgress( "google" ) && form.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_FAILED == form.GetProgress( "amazon" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: The check box is the setting autoLogin of the manager" )
    try
    {
        CLoginFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        REQUIRE_TRUE( GUCEF_NULL != fixture.AddProvider( "google" ) );
        AUTHGUI::CLoginForm& form = fixture.form;
        CLoginFormRecorder recorder( &form );
        REQUIRE_TRUE( form.Open() );
        ASSERT_TRUE( fixture.manager.GetSettings().autoLogin && form.GetWidget( "AutoLogin" )->GetBoolProperty( "Checked" ) );

        ClickWidget( fixture.context, form.GetWidget( "AutoLogin" ) );
        ASSERT_TRUE( !fixture.manager.GetSettings().autoLogin && !form.GetAutoLogin() && 1 == recorder.autoLoginChanges );
        fixture.Settle();
        ASSERT_TRUE( !form.GetWidget( "AutoLogin" )->GetBoolProperty( "Checked" ) );
        ClickWidget( fixture.context, form.GetWidget( "AutoLogin" ) );
        ASSERT_TRUE( fixture.manager.GetSettings().autoLogin && 2 == recorder.autoLoginChanges );

        // by code: only a change is a change, the check box follows
        form.SetAutoLogin( true );
        ASSERT_TRUE( 2 == recorder.autoLoginChanges );
        form.SetAutoLogin( false );
        ASSERT_TRUE( 3 == recorder.autoLoginChanges && !fixture.manager.GetSettings().autoLogin && !form.GetWidget( "AutoLogin" )->GetBoolProperty( "Checked" ) );

        // the setting reaches the provider settings it did not touch
        AUTH::CAuthSettings settings( fixture.manager.GetSettings() );
        AUTH::CAuthProviderSettings providerSettings;
        providerSettings.clientId = "kept";
        settings.SetProviderSettings( "google", providerSettings );
        fixture.manager.SetSettings( settings );
        form.SetAutoLogin( true );
        AUTH::CAuthProviderSettings kept;
        ASSERT_TRUE( fixture.manager.GetSettings().GetProviderSettings( "google", kept ) && "kept" == kept.clientId );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: The window is moved by its title bar and folded, and stays where it was put and folded when the content changes" )
    try
    {
        CLoginFixture fixture( 1242, 952 );
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        REQUIRE_TRUE( GUCEF_NULL != google );
        google->mode = CFakeLoginProvider::MODE_BROWSER;
        AUTHGUI::CLoginForm& form = fixture.form;
        REQUIRE_TRUE( form.Open() );
        fixture.context->FlushChanges();

        GUI::CWindow* window = GUI::CWidget::ToWindow( form.GetRootWidget() );
        REQUIRE_TRUE( GUCEF_NULL != window );
        ASSERT_TRUE( window->IsMovable() && window->IsCollapsible() && !window->IsCollapsed() && !fixture.context->IsModalRoot( window ) );
        const GUI::CRect start = window->GetRect();

        // by the title bar
        fixture.context->InjectMouseMove( start.left + 200, start.top + 10 );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        fixture.context->InjectMouseMove( start.left + 200 - 70, start.top + 10 + 30 );
        fixture.context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        fixture.context->FlushChanges();
        ASSERT_TRUE( window->GetRect().left == start.left - 70 && window->GetRect().top == start.top + 30 );

        // the content changes: the window is made again, it stays where it was put
        ClickWidget( fixture.context, form.GetWidget( "Login0" ) );
        fixture.Settle();
        window = GUI::CWidget::ToWindow( form.GetRootWidget() );
        REQUIRE_TRUE( GUCEF_NULL != window );
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_WAITING_FOR_BROWSER == form.GetProgress( "google" ) && GUCEF_NULL != form.GetWidget( "Cancel0" ) );
        ASSERT_TRUE( window->GetRect().left == start.left - 70 && window->GetRect().top == start.top + 30 && window->GetRect().width == start.width && window->GetRect().height == start.height );

        // folded: it stays folded when the login ends
        window->ToggleCollapsed();
        fixture.context->FlushChanges();
        ASSERT_TRUE( window->IsCollapsed() && window->GetRect().height == window->GetTitleBarHeight() );
        ASSERT_TRUE( google->Succeed() );
        fixture.Settle();
        window = GUI::CWidget::ToWindow( form.GetRootWidget() );
        REQUIRE_TRUE( GUCEF_NULL != window );
        ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_DONE == form.GetProgress( "google" ) );
        ASSERT_TRUE( window->IsCollapsed() );
        ASSERT_TRUE( window->GetRect().top == start.top + 30 );
        ASSERT_TRUE( window->GetRect().left == start.left - 70 );
        window->ToggleCollapsed();
        fixture.context->FlushChanges();
        ASSERT_TRUE( window->GetRect() == GUI::CRect( start.left - 70, start.top + 30, start.width, start.height ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Unlink0" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: A thumb of the scroll bar that is held keeps the position and the drag when the content changes, the wheel scrolls over the buttons" )
    try
    {
        CLoginFixture fixture( 800, 600 );
        REQUIRE_TRUE( fixture.IsReady() );
        for ( UInt32 i=0; i<14; ++i )
            REQUIRE_TRUE( GUCEF_NULL != fixture.AddProvider( "provider" + CORE::UInt32ToString( i ) ) );
        AUTHGUI::CLoginForm& form = fixture.form;
        REQUIRE_TRUE( form.Open() );
        fixture.Settle();
        GUI::CGUIContext* context = fixture.context;
        GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( form.GetWidget( "Body" ) );
        REQUIRE_TRUE( GUCEF_NULL != body && body->IsScrollable() );

        // over a button
        GUI::CWidget* button = form.GetWidget( "Login3" );
        REQUIRE_TRUE( GUCEF_NULL != button );
        context->InjectMouseMove( button->GetRect().left + button->GetRect().width / 2, button->GetRect().top + button->GetRect().height / 2 );
        context->InjectMouseWheel( -1 );
        context->FlushChanges();
        ASSERT_TRUE( body->GetScrollPosition() > 0 );
        body->SetScrollPosition( 0 );
        context->FlushChanges();

        // the thumb held, the content changes
        const GUI::CRect track = body->GetTrackRect();
        const GUI::CRect thumb = body->GetThumbRect();
        const Int32 x = body->GetRect().left + track.left + track.width / 2;
        const Int32 y = body->GetRect().top + thumb.top + thumb.height / 2;
        context->InjectMouseMove( x, y );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, true );
        context->InjectMouseMove( x, y + 20 );
        context->FlushChanges();
        const Int32 dragged = body->GetScrollPosition();
        ASSERT_TRUE( dragged > 0 && body->IsThumbDragged() );
        form.SetAuthManager( &fixture.manager );
        context->RenderFrame( 16 );
        context->RenderFrame( 16 );
        context->InjectMouseMove( x, y + 40 );
        context->FlushChanges();
        GUI::CScrollPanel* now = GUI::CWidget::ToScrollPanel( form.GetWidget( "Body" ) );
        ASSERT_TRUE( GUCEF_NULL != now && now->GetScrollPosition() > dragged );
        context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
        context->RenderFrame( 16 );
        GUI::CScrollPanel* after = GUI::CWidget::ToScrollPanel( form.GetWidget( "Body" ) );
        ASSERT_TRUE( GUCEF_NULL != after && after->GetScrollPosition() > dragged && !after->IsThumbDragged() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: Close and Escape close the window and notify, a form opened again has the state of the manager" )
    try
    {
        CLoginFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        REQUIRE_TRUE( GUCEF_NULL != google );
        AUTHGUI::CLoginForm& form = fixture.form;
        CLoginFormRecorder recorder( &form );
        REQUIRE_TRUE( form.Open() );

        ClickWidget( fixture.context, form.GetWidget( "Close" ) );
        ASSERT_TRUE( !form.IsOpen() && !form.IsVisible() && 1 == recorder.closed );
        form.Close();
        ASSERT_TRUE( 1 == recorder.closed );

        // what happens meanwhile is there when it opens
        google->mode = CFakeLoginProvider::MODE_INSTANT;
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( fixture.manager.Login( "google", AUTH::CAuthOptions(), future ) );
        PumpFor( 30 );
        ASSERT_TRUE( form.Open() && form.IsVisible() );
        fixture.Settle();
        ASSERT_TRUE( "Linked: Fake" == form.GetStateText() && GUCEF_NULL != form.GetWidget( "Unlink0" ) );
        PressKey( fixture.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !form.IsOpen() && !form.IsVisible() && 2 == recorder.closed );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "LoginForm: A manager that goes away is let go, a login under way when the form gets the manager is waited for" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        REQUIRE_TRUE( GUCEF_NULL != context );
        AUTHGUI::CLoginForm form( context );
        {
            CLoginFixture inner;
            REQUIRE_TRUE( inner.IsReady() );
            CFakeLoginProvider* google = inner.AddProvider( "google" );
            REQUIRE_TRUE( GUCEF_NULL != google );
            google->mode = CFakeLoginProvider::MODE_BROWSER;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( inner.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            form.SetAuthManager( &inner.manager );
            ASSERT_TRUE( AUTHGUI::CLoginForm::LOGINPROGRESS_WAITING_FOR_BROWSER == form.GetProgress( "google" ) );
            ASSERT_TRUE( &inner.manager == form.GetAuthManager() );
            google->Succeed();
            PumpFor( 30 );
            inner.manager.RemoveProvider( "google" );
        }
        ASSERT_TRUE( GUCEF_NULL == form.GetAuthManager() );
        ASSERT_TRUE( form.Open() && "Guest" == form.GetStateText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
