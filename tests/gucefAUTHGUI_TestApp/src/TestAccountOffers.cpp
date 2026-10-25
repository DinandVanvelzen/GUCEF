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

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestOpenId2Backend.h"
#include "gucefAUTHGUI_TestCommon.h"
#include "TestAccountFixture.h"
#include "TestLayoutUtils.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A machine with the client of the provider of the OpenID 2.0 backend of the tests, which has had one user: what the backend
 *  detects on the machine, for the line below the button of the login. Apply() makes the backend see the machine as it is now.
 */
class CHintMachine
{
    public:

    bool installed;
    bool running;
    CORE::CString persona;
    CORE::CString account;

    CHintMachine( void ) : installed( true ), running( true ), persona( "Alice P" ), account( "alice" )
    {
        Apply();
    }

    ~CHintMachine()
    {
        TestOpenId2Profile().Reset();
    }

    /** The name of the person is the persona, or the name of the account when there is none */
    void Apply( void )
    {
        CTestOpenId2Profile& profile = TestOpenId2Profile();
        profile.hintInstalled = installed;
        profile.hintRunning = running;
        profile.hintName = persona.IsNULLOrEmpty() ? account : persona;
        profile.ForgetDetectedAccountHint();
    }
};

/*-------------------------------------------------------------------------*/

/** Records what the page says before a login starts, and what the manager knew at that moment */
class CLoginRequestRecorder : public CORE::CObserver
{
    public:

    CORE::CString::StringVector names;
    GUCEF::vector< bool > pendingAtThatMoment;
    AUTH::CAuthManager* manager;

    typedef CORE::CTEventHandlerFunctor< CLoginRequestRecorder > TEventCallback;

    CLoginRequestRecorder( CORE::CNotifier* notifier, const CORE::CEvent& id, AUTH::CAuthManager* observedManager )
        : CORE::CObserver(), names(), pendingAtThatMoment(), manager( observedManager )
    {
        TEventCallback callback( this, &CLoginRequestRecorder::OnLoginRequested );
        SubscribeTo( notifier, id, callback );
    }

    virtual ~CLoginRequestRecorder() { UnsubscribeAllFromObserver(); }

    void OnLoginRequested( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData )
            return;
        const CORE::CString name = static_cast< CORE::TCloneableString* >( eventData )->GetData();
        names.push_back( name );
        pendingAtThatMoment.push_back( GUCEF_NULL != manager && manager->IsLoginPending( name ) );
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
TextOf( AUTHGUI::CAccountSettingsForm& page, const CORE::CString& widgetName )
{
    GUI::CWidget* widget = page.GetWidget( widgetName );
    return GUCEF_NULL == widget ? CORE::CString( "<no such widget>" ) : widget->GetProperty( "Text" );
}

/*-------------------------------------------------------------------------*/

/** The widget number of an unavailable provider in the page */
static CORE::CString
UnavailableNumber( AUTHGUI::CAccountSettingsForm& page, const CORE::CString& providerName )
{
    CORE::CString::StringVector shown;
    page.GetShownUnavailableProviders( shown );
    for ( size_t i=0; i<shown.size(); ++i )
        if ( shown[ i ] == providerName )
            return CORE::UInt32ToString( (UInt32) i );
    return CORE::CString( "none" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A system without a TLS library, for as long as the object lives: the TLS providers of the process are taken out of the registry and put back */
class CScopedNoTlsProviders
{
    public:

    CScopedNoTlsProviders( void )
        : m_providers()
    {
        COMCORE::CTlsProviderRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        registry.GetProviderStatus( m_providers );
        registry.UnregisterAll();
    }

    ~CScopedNoTlsProviders()
    {
        COMCORE::CTlsProviderRegistry& registry = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry();
        for ( size_t i=0; i<m_providers.size(); ++i )
            registry.RegisterProvider( m_providers[ i ].provider, m_providers[ i ].priority );
    }

    private:

    CScopedNoTlsProviders( const CScopedNoTlsProviders& src );
    CScopedNoTlsProviders& operator=( const CScopedNoTlsProviders& src );

    COMCORE::CTlsProviderRegistry::TTlsProviderStatusVector m_providers;
};

/*-------------------------------------------------------------------------*/

void
PerformAccountOfferNoHttpsTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.AccountOffersNoHttps" );

    GUCEF_TESTFW_TESTCASE( "AccountOffersNoHttps: Without an HTTP client for https the login of the OpenID 2.0 backend is shown disabled with the reason" )
    GUCEF_TESTFW_SKIP_IF( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN, "WinHTTP is part of the build on Windows, https needs no plugin there" );
    GUCEF_TESTFW_SKIP_IF( WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" ), "The plugin for https is loaded already" );
    try
    {
        // a system without a TLS library of its own, and without the plugin for https
        CScopedNoTlsProviders noTls;
        REQUIRE_TRUE( !WEB::CHTTPClient::IsSchemeSupported( "https" ) );

        CAccountFixture fixture( 1242, 952, true, true );
        REQUIRE_TRUE( fixture.IsReady() );
        CHintMachine machine;
        fixture.manager.RefreshProviders();
        REQUIRE_TRUE( GUCEF_NULL != fixture.manager.GetProvider( "testopenid" ) );
        machine.Apply();
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        REQUIRE_TRUE( fixture.Open() );

        // nothing is offered, the login of the backend is one of the disabled ones, with the reason as text and as tooltip
        CORE::CString::StringVector shown;
        page.GetShownProviders( shown );
        ASSERT_TRUE( shown.empty() );
        CORE::CString::StringVector unavailable;
        page.GetShownUnavailableProviders( unavailable );
        ASSERT_TRUE( 8 == unavailable.size() );
        const CORE::CString number = UnavailableNumber( page, "testopenid" );
        ASSERT_TRUE( "none" != number );
        GUI::CWidget* button = page.GetWidget( "UnavailableLogin" + number );
        GUI::CWidget* reason = page.GetWidget( "UnavailableReason" + number );
        ASSERT_TRUE( GUCEF_NULL != button && GUCEF_NULL != reason );
        if ( GUCEF_NULL != button && GUCEF_NULL != reason )
        {
            ASSERT_FALSE( button->GetBoolProperty( "Enabled" ) );
            ASSERT_TRUE( "No HTTPS support: no TLS provider is registered on this platform" == reason->GetProperty( "Text" ) );
            ASSERT_TRUE( "No HTTPS support: no TLS provider is registered on this platform" == button->GetProperty( "Tooltip" ) );
        }
        ASSERT_TRUE( "No HTTPS support: no TLS provider is registered on this platform" == page.GetPanel().GetUnavailableText( "testopenid" ) );
        fixture.settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformAccountOfferTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.AccountOffers" );

    GUCEF_TESTFW_TESTCASE( "AccountOffers: The OpenID 2.0 login of the backend is offered with the name the backend detected, the providers without a client id are disabled with the reason and the key as tooltip" )
    try
    {
        CAccountFixture fixture( 1242, 952, true, true );
        REQUIRE_TRUE( fixture.IsReady() );
        CHintMachine machine;
        fixture.manager.RefreshProviders();
        REQUIRE_TRUE( GUCEF_NULL != fixture.manager.GetProvider( "testopenid" ) );
        machine.Apply();
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        REQUIRE_TRUE( fixture.Open() );

        // the one offer, with who the backend says is there, and the line about the client
        CORE::CString::StringVector shown;
        page.GetShownProviders( shown );
        ASSERT_TRUE( 1 == shown.size() && "testopenid" == shown[ 0 ] );
        ASSERT_TRUE( "Log in with TestOpenId (Alice P)" == TextOf( page, "Login0" ) );
        ASSERT_TRUE( "TestOpenId is installed and running" == TextOf( page, "Hint0" ) );
        ASSERT_TRUE( page.GetWidget( "Login0" )->GetBoolProperty( "Enabled" ) );
        ASSERT_TRUE( page.GetPanel().GetLoginButtonText( "testopenid" ) == TextOf( page, "Login0" ) && page.GetPanel().GetHintText( "testopenid" ) == TextOf( page, "Hint0" ) );
        // the offer is the login of the browser: no page of a device code, nothing in progress, nobody linked
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_IDLE == page.GetProgress( "testopenid" ) && "Guest" == page.GetStateText() );
        ASSERT_TRUE( GUCEF_NULL != page.GetWidget( "NoAccounts" ) && "None. You play as a guest." == TextOf( page, "NoAccounts" ) );

        // the others are there, disabled, with why
        CORE::CString::StringVector unavailable;
        page.GetShownUnavailableProviders( unavailable );
        ASSERT_TRUE( 7 == unavailable.size() );
        const char* names[] = { "amazon", "aws-cognito", "aws-identitycenter", "epic", "facebook", "google", "microsoft" };
        for ( UInt32 i=0; i<sizeof( names ) / sizeof( names[ 0 ] ); ++i )
        {
            const CORE::CString number = UnavailableNumber( page, names[ i ] );
            ASSERT_TRUE( "none" != number );
            GUI::CWidget* button = page.GetWidget( "UnavailableLogin" + number );
            GUI::CWidget* reason = page.GetWidget( "UnavailableReason" + number );
            ASSERT_TRUE( GUCEF_NULL != button && GUCEF_NULL != reason );
            if ( GUCEF_NULL == button || GUCEF_NULL == reason )
                continue;
            ASSERT_FALSE( button->GetBoolProperty( "Enabled" ) );
            ASSERT_TRUE( "Log in with " + AUTH::GetProviderDisplayName( names[ i ] ) == button->GetProperty( "Text" ) );
            ASSERT_TRUE( "Not set up in this build: needs a client id" == reason->GetProperty( "Text" ) );
            const CORE::CString tooltip = CORE::CString( "Not set up in this build: needs a client id. Fill in Auth/Providers/" ) + names[ i ] + "/clientId in the configuration";
            ASSERT_TRUE( tooltip == button->GetProperty( "Tooltip" ) && tooltip == reason->GetProperty( "Tooltip" ) );
            ASSERT_TRUE( reason->GetProperty( "Text" ) == page.GetPanel().GetUnavailableText( names[ i ] ) && tooltip == page.GetPanel().GetUnavailableTooltip( names[ i ] ) );
        }
        ASSERT_TRUE( page.GetPanel().GetUnavailableText( "testopenid" ).IsNULLOrEmpty() && page.GetPanel().GetUnavailableTooltip( "nobody" ).IsNULLOrEmpty() );

        // the check box of the start is there: a provider is offered
        ASSERT_TRUE( page.HasAutoLoginBox() );

        // pressing a disabled button does nothing
        CEventCount requests( &page, AUTHGUI::CAccountSettingsForm::LoginRequestedEvent );
        const CORE::CString googleNumber = UnavailableNumber( page, "google" );
        ClickWidget( fixture.context, page.GetWidget( "UnavailableLogin" + googleNumber ) );
        fixture.Settle();
        ASSERT_TRUE( 0 == requests.count && !fixture.manager.IsLoginPending( "google" ) && AUTHGUI::CAccountsPanel::LOGINPROGRESS_IDLE == page.GetProgress( "google" ) );
        ASSERT_FALSE( page.Login( "google" ) && fixture.manager.IsLoginPending( "google" ) );
        fixture.settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountOffers: The line about the client of the provider follows whether it runs, has no name when none is known and is not there without a client" )
    try
    {
        CAccountFixture fixture( 1242, 952, true, true );
        REQUIRE_TRUE( fixture.IsReady() );
        CHintMachine machine;
        fixture.manager.RefreshProviders();
        REQUIRE_TRUE( GUCEF_NULL != fixture.manager.GetProvider( "testopenid" ) );
        machine.Apply();
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;

        // installed, not running
        machine.running = false;
        machine.Apply();
        REQUIRE_TRUE( fixture.Open() );
        ASSERT_TRUE( "Log in with TestOpenId (Alice P)" == TextOf( page, "Login0" ) && "TestOpenId is installed" == TextOf( page, "Hint0" ) );
        fixture.settings.Cancel();

        // a name of a user that is too long is cut off
        machine.persona = "A very long persona name that goes on and on";
        machine.Apply();
        REQUIRE_TRUE( fixture.Open() );
        const CORE::CString longText = TextOf( page, "Login0" );
        ASSERT_TRUE( longText.HasSubstr( "Log in with TestOpenId (A very long perso..." ) == 0 && longText.HasSubstr( "...)" ) > 0 && longText.HasSubstr( "goes on" ) < 0 );
        fixture.settings.Cancel();

        // no persona: the name of the account
        machine.persona = "";
        machine.Apply();
        REQUIRE_TRUE( fixture.Open() );
        ASSERT_TRUE( "Log in with TestOpenId (alice)" == TextOf( page, "Login0" ) && "TestOpenId is installed" == TextOf( page, "Hint0" ) );
        fixture.settings.Cancel();

        // no name at all: the plain button, the line says the client is there
        machine.account = "";
        machine.Apply();
        REQUIRE_TRUE( fixture.Open() );
        ASSERT_TRUE( "Log in with TestOpenId" == TextOf( page, "Login0" ) && "TestOpenId is installed" == TextOf( page, "Hint0" ) );
        fixture.settings.Cancel();

        // no client: the plain button and no line
        machine.installed = false;
        machine.Apply();
        REQUIRE_TRUE( fixture.Open() );
        ASSERT_TRUE( "Log in with TestOpenId" == TextOf( page, "Login0" ) && GUCEF_NULL == page.GetWidget( "Hint0" ) );
        ASSERT_TRUE( page.GetPanel().GetHintText( "testopenid" ).IsNULLOrEmpty() );
        fixture.settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountOffers: With showUnavailableProviders off the disabled providers are not shown, and a provider with a client id is offered and not shown disabled" )
    try
    {
        CAccountFixture fixture( 1242, 952, true, true );
        REQUIRE_TRUE( fixture.IsReady() );
        CHintMachine machine;
        fixture.manager.RefreshProviders();
        machine.Apply();
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;

        AUTH::CAuthSettings settings = fixture.manager.GetSettings();
        AUTH::CAuthProviderSettings google;
        google.clientId = "123-abc.apps.googleusercontent.com";
        settings.SetProviderSettings( "google", google );
        fixture.manager.SetSettings( settings );
        REQUIRE_TRUE( fixture.Open() );
        CORE::CString::StringVector shown, unavailable;
        page.GetShownProviders( shown );
        page.GetShownUnavailableProviders( unavailable );

        // with its client id the provider is offered, when the codecs of its tokens are there; without them it is shown disabled and says so
        const bool codecs = AUTH::CAuthCrypto::IsRsaAvailable() && fixture.manager.IsProviderOffered( "google" );
        if ( codecs )
        {
            ASSERT_TRUE( 2 == shown.size() && 6 == unavailable.size() && "none" == UnavailableNumber( page, "google" ) );
        }
        else
        {
            ASSERT_TRUE( 1 == shown.size() && 7 == unavailable.size() );
            const CORE::CString number = UnavailableNumber( page, "google" );
            ASSERT_TRUE( "none" != number && "Not available on this platform" == TextOf( page, "UnavailableReason" + number ) );
            ASSERT_TRUE( page.GetPanel().GetUnavailableTooltip( "google" ) == "Not available on this platform" );
        }
        // the line about the client is on the row of the backend only
        ASSERT_TRUE( 1 == ( GUCEF_NULL != page.GetWidget( "Hint0" ) ? 1U : 0U ) + ( GUCEF_NULL != page.GetWidget( "Hint1" ) ? 1U : 0U ) );
        fixture.settings.Cancel();

        settings.showUnavailableProviders = false;
        fixture.manager.SetSettings( settings );
        REQUIRE_TRUE( fixture.Open() );
        page.GetShownUnavailableProviders( unavailable );
        ASSERT_TRUE( unavailable.empty() && GUCEF_NULL == page.GetWidget( "UnavailableRow0" ) && GUCEF_NULL == page.GetWidget( "UnavailableLogin0" ) && GUCEF_NULL == page.GetWidget( "UnavailableReason0" ) );
        page.GetShownProviders( shown );
        ASSERT_TRUE( ( codecs ? 2U : 1U ) == shown.size() );
        fixture.settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountOffers: A login that starts from the page is announced with the provider name before the manager is asked, by the panel and by the page" )
    try
    {
        CAccountFixture fixture( 1242, 952, true, true );
        REQUIRE_TRUE( fixture.IsReady() );
        CFakeLoginProvider* alpha = fixture.AddProvider( "alpha" );
        CFakeLoginProvider* beta = fixture.AddProvider( "beta" );
        REQUIRE_TRUE( GUCEF_NULL != alpha && GUCEF_NULL != beta );
        alpha->mode = CFakeLoginProvider::MODE_BROWSER;
        beta->mode = CFakeLoginProvider::MODE_REFUSE;
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        REQUIRE_TRUE( fixture.Open() );
        CLoginRequestRecorder onPage( &page, AUTHGUI::CAccountSettingsForm::LoginRequestedEvent, &fixture.manager );
        CLoginRequestRecorder onPanel( &page.GetPanel(), AUTHGUI::CAccountsPanel::LoginRequestedEvent, &fixture.manager );

        // the button
        ClickWidget( fixture.context, page.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( 1 == onPage.names.size() && "alpha" == onPage.names[ 0 ] && !onPage.pendingAtThatMoment[ 0 ] );
        ASSERT_TRUE( 1 == onPanel.names.size() && "alpha" == onPanel.names[ 0 ] );
        ASSERT_TRUE( 1 == alpha->beginCount && fixture.manager.IsLoginPending( "alpha" ) );

        // a login that waits is not started again and is not announced again
        ASSERT_FALSE( page.Login( "alpha" ) );
        ASSERT_TRUE( 1 == onPage.names.size() && 1 == alpha->beginCount );

        // a login the manager does not start is announced too, and is a failure of the panel
        ASSERT_FALSE( page.Login( "beta" ) );
        ASSERT_TRUE( 2 == onPage.names.size() && "beta" == onPage.names[ 1 ] && AUTHGUI::CAccountsPanel::LOGINPROGRESS_FAILED == page.GetProgress( "beta" ) );

        // a login of a provider nobody has: announced, and refused by the manager
        ASSERT_FALSE( page.Login( "nobody" ) );
        ASSERT_TRUE( 3 == onPage.names.size() && "nobody" == onPage.names[ 2 ] );

        alpha->HonourCancel();
        page.CancelLogin( "alpha" );
        alpha->HonourCancel();
        fixture.Settle();
        fixture.settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountOffers: The page with a hint and with disabled providers keeps its controls apart and inside from 600 x 900 to 3840 x 1080" )
    try
    {
        const SScreenSize sizes[] = { { 1242, 952 }, { 3840, 1080 }, { 600, 900 }, { 800, 600 }, { 1920, 1080 } };
        for ( UInt32 s=0; s<5; ++s )
        {
            const UInt32 screenWidth = sizes[ s ].width;
            const UInt32 screenHeight = sizes[ s ].height;
            const CORE::CString where = SizeName( screenWidth, screenHeight );
            CAccountFixture fixture( screenWidth, screenHeight, true, true );
            REQUIRE_TRUE( fixture.IsReady() );
            CHintMachine machine;
            fixture.manager.RefreshProviders();
            machine.Apply();
            AUTHGUI::CAccountSettingsForm& page = *fixture.page;
            REQUIRE_TRUE( fixture.Open() );

            GUI::CWidget* window = fixture.settings.GetRootWidget();
            ASSERT_TRUE( Inside( window->GetRect(), GUI::CRect( 0, 0, (Int32) screenWidth, (Int32) screenHeight ) ) );
            CORE::CString problem = FindLayoutProblem( window );
            if ( !problem.IsNULLOrEmpty() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AccountOffers: the account page at " << where.C_String() << ": " << problem.C_String() );
            ASSERT_TRUE( problem.IsNULLOrEmpty() );
            IMAGE::TPixelMapPtr frame = CaptureFrame( fixture.context );
            ASSERT_TRUE( OnlyBlackOutside( frame, window->GetRect() ) && CountLit( frame, window->GetRect() ) > 500 );
            WriteShot( frame, "account-offer-guest-" + where );

            // the last of the disabled ones comes into view
            GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( page.GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != body );
            if ( GUCEF_NULL == body )
                return;
            body->SetScrollPosition( body->GetMaxScrollPosition() );
            fixture.context->FlushChanges();
            ASSERT_TRUE( FindLayoutProblem( window ).IsNULLOrEmpty() );
            GUI::CWidget* last = page.GetWidget( "AutoLogin" );
            ASSERT_TRUE( GUCEF_NULL != last && Inside( last->GetRect(), body->GetRect() ) );
            if ( body->IsScrollable() )
                WriteShot( CaptureFrame( fixture.context ), "account-offer-guest-scrolled-" + where );
            body->SetScrollPosition( 0 );
            fixture.context->FlushChanges();

            // the column is as wide as the tab, up to a generous limit, and in the middle
            GUI::CWidget* list = page.GetWidget( "BodyList" );
            ASSERT_TRUE( GUCEF_NULL != list && list->GetRect().width <= 1384 && list->GetRect().width >= 300 );
            ASSERT_TRUE( list->GetRect().width == body->GetRect().width - 16 || list->GetRect().width == 1384 );
            fixture.settings.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountOffers: The wheel scrolls the page over every widget of it, the disabled providers too, and a drag of the thumb survives the page being made again" )
    try
    {
        const SScreenSize sizes[] = { { 800, 600 }, { 1242, 952 }, { 3840, 1080 } };
        for ( UInt32 s=0; s<3; ++s )
        {
            const UInt32 screenWidth = sizes[ s ].width;
            const UInt32 screenHeight = sizes[ s ].height;
            const CORE::CString where = SizeName( screenWidth, screenHeight );
            CAccountFixture fixture( screenWidth, screenHeight, true, true );
            REQUIRE_TRUE( fixture.IsReady() );
            CHintMachine machine;
            fixture.manager.RefreshProviders();
            machine.Apply();
            AUTHGUI::CAccountSettingsForm& page = *fixture.page;
            REQUIRE_TRUE( fixture.Open() );
            GUI::CGUIContext* context = fixture.context;
            GUI::CScrollPanel* body = GUI::CWidget::ToScrollPanel( page.GetWidget( "Body" ) );
            REQUIRE_TRUE( GUCEF_NULL != body && body->IsScrollable() );
            const Int32 maxScroll = body->GetMaxScrollPosition();

            // a notch over each widget in view scrolls the page, whether the widget is enabled or not
            UInt32 over = 0, overDisabled = 0;
            GUCEF::vector< GUI::CWidget* > widgets;
            widgets.push_back( body );
            for ( size_t i=0; i<widgets.size(); ++i )
                for ( size_t c=0; c<widgets[ i ]->GetChildren().size(); ++c )
                    widgets.push_back( widgets[ i ]->GetChildren()[ c ] );
            for ( size_t i=1; i<widgets.size(); ++i )
            {
                GUI::CWidget* widget = widgets[ i ];
                for ( UInt32 position=0; position<3; ++position )
                {
                    body->SetScrollPosition( 0 == position ? 0 : ( 1 == position ? maxScroll / 2 : maxScroll ) );
                    context->FlushChanges();
                    const GUI::CRect seen = widget->GetRect().Intersection( GUI::CRect( body->GetRect().left, body->GetRect().top, body->GetRect().width - body->GetTrackRect().width, body->GetRect().height ) );
                    if ( !widget->IsVisible() || seen.IsEmpty() )
                        continue;
                    GUI::CWidget* hit = context->HitTest( seen.left + seen.width / 2, seen.top + seen.height / 2 );
                    if ( GUCEF_NULL == hit || hit == body || "Editbox" == hit->GetType() )
                        continue;
                    const Int32 before = body->GetScrollPosition();
                    if ( before >= maxScroll )
                        continue;
                    context->InjectMouseMove( seen.left + seen.width / 2, seen.top + seen.height / 2 );
                    context->InjectMouseWheel( -1 );
                    context->FlushChanges();
                    if ( body->GetScrollPosition() <= before )
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AccountOffers: the wheel over \"" << hit->GetName().C_String() << "\" at " << where.C_String() << " scrolls nothing" );
                    ASSERT_TRUE( body->GetScrollPosition() > before );
                    ++over;
                    if ( !hit->IsEffectivelyEnabled() )
                        ++overDisabled;
                    break;
                }
            }
            ASSERT_TRUE( over > 8 && overDisabled > 3 );

            // the thumb is held and the page is made again (a login changed): the position is what it was and the drag goes on
            body->SetScrollPosition( 0 );
            context->FlushChanges();
            context->RenderFrame( 16 );
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
            page.SetAuthManager( &fixture.manager );
            context->RenderFrame( 16 );
            context->RenderFrame( 16 );
            context->InjectMouseMove( x, y + 40 );
            context->FlushChanges();
            GUI::CScrollPanel* now = GUI::CWidget::ToScrollPanel( page.GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != now && now->GetScrollPosition() > dragged );
            context->InjectMouseButton( GUI::MOUSEBUTTON_LEFT, false );
            context->RenderFrame( 16 );
            GUI::CScrollPanel* after = GUI::CWidget::ToScrollPanel( page.GetWidget( "Body" ) );
            ASSERT_TRUE( GUCEF_NULL != after && after->GetScrollPosition() > dragged && !after->IsThumbDragged() );
            fixture.settings.Cancel();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
