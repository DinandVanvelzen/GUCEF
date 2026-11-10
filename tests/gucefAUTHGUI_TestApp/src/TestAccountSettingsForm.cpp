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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefAUTHGUI_TestCommon.h"
#include "TestRecordingBrowser.h"
#include "TestAccountFixture.h"

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
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformAccountSettingsFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefAUTHGUI.AccountSettings" );

    GUCEF_TESTFW_TESTCASE( "AccountSettings: With no manager and no profile the page is usable: a guest, and the line that says nothing is set up" )
    try
    {
        CAccountFixture fixture( 1280, 960, false, false );
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        ASSERT_TRUE( "Account" == page.GetTitle() );
        ASSERT_TRUE( fixture.Open() );

        ASSERT_TRUE( GUCEF_NULL == page.GetAuthManager() && GUCEF_NULL == page.GetProfile() );
        ASSERT_TRUE( "Guest" == page.GetStateText() );
        GUI::CWidget* hint = page.GetWidget( "NoProviders" );
        ASSERT_TRUE( GUCEF_NULL != hint && "No sign-in services are set up in this build. You play as a guest." == hint->GetProperty( "Text" ) );
        ASSERT_TRUE( GUCEF_NULL == page.GetWidget( "Login0" ) && GUCEF_NULL != page.GetWidget( "NoAccounts" ) );
        ASSERT_TRUE( !page.HasProfileFields() && GUCEF_NULL == page.GetWidget( "DisplayName" ) && GUCEF_NULL == page.GetWidget( "ProfileId" ) );
        ASSERT_TRUE( !page.HasAutoLoginBox() && page.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( page.GetDisplayNameText().IsNULLOrEmpty() && !page.SetDisplayNameText( "Nobody" ) && !page.SetAutoLogin( true ) );

        // nothing to apply, nothing to log in with
        ASSERT_FALSE( fixture.settings.HasChanges() || IsWidgetEnabled( fixture.settings, "Apply" ) );
        ASSERT_TRUE( fixture.settings.Apply() );
        ASSERT_FALSE( page.Login( "anything" ) || page.Unlink( AUTH::CAccountRef( "google", "1" ) ) || page.OpenDeviceCodePage( "anything" ) );
        ASSERT_TRUE( page.GetErrorText().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: With a manager that offers nothing the page says so and has no check box" )
    try
    {
        CAccountFixture fixture( 1280, 960, true, false );
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        ASSERT_TRUE( fixture.Open() );
        ASSERT_TRUE( &fixture.manager == page.GetAuthManager() && "Guest" == page.GetStateText() );
        ASSERT_TRUE( GUCEF_NULL != page.GetWidget( "NoProviders" ) && !page.HasAutoLoginBox() && !page.HasProfileFields() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: A profile shows its name and its id read only, the name is applied through the profile and Cancel puts it back" )
    try
    {
        CAccountFixture fixture( 1280, 960, false, true );
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        CEventCount changed( &fixture.profile, CTestProfile::ChangedEvent() );
        ASSERT_TRUE( fixture.Open() );

        ASSERT_TRUE( page.HasProfileFields() && "Alice" == page.GetDisplayNameText() && fixture.profile.id == page.GetProfileIdText() );
        ASSERT_TRUE( page.GetWidget( "ProfileId" )->GetBoolProperty( "ReadOnly" ) && !page.GetWidget( "DisplayName" )->GetBoolProperty( "ReadOnly" ) );
        ASSERT_TRUE( AUTHGUI::CAccountSettingsForm::MaxDisplayNameLength == (UInt32) page.GetWidget( "DisplayName" )->GetIntProperty( "MaxLength" ) );
        ASSERT_TRUE( "Guest" == page.GetStateText() && GUCEF_NULL != page.GetWidget( "NoProviders" ) );
        ASSERT_FALSE( fixture.settings.HasChanges() || IsWidgetEnabled( fixture.settings, "Apply" ) );

        // typing is a change that waits for Apply
        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "Bob", false );
        ASSERT_TRUE( "Bob" == page.GetDisplayNameText() && "Alice" == fixture.profile.name && 0 == fixture.profile.renames );
        ASSERT_TRUE( page.HasChanges() && fixture.settings.HasChanges() && IsWidgetEnabled( fixture.settings, "Apply" ) );
        ClickWidget( fixture.context, fixture.settings.GetWidget( "Apply" ) );
        ASSERT_TRUE( "Bob" == fixture.profile.name && 1 == fixture.profile.renames && 1 == changed.count );
        ASSERT_FALSE( page.HasChanges() || IsWidgetEnabled( fixture.settings, "Apply" ) );
        fixture.Settle();
        ASSERT_TRUE( "Bob" == page.GetDisplayNameText() && !page.HasChanges() );

        // the same name again is no change of the profile
        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "Bob", false );
        ASSERT_TRUE( fixture.settings.Apply() && 1 == fixture.profile.renames );

        // an empty name is not a name, a profile that refuses is an error: the change stays
        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "   ", false );
        ASSERT_FALSE( fixture.settings.Apply() );
        ASSERT_TRUE( "Enter a name" == page.GetErrorText() && "Bob" == fixture.profile.name && page.HasChanges() );
        ASSERT_TRUE( "Enter a name" == page.GetWidget( "Error" )->GetProperty( "Text" ) );
        fixture.profile.refuseNames = true;
        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "Refused", false );
        ASSERT_FALSE( fixture.settings.Apply() );
        ASSERT_TRUE( "The name could not be saved" == page.GetErrorText() && "Bob" == fixture.profile.name && page.HasChanges() );
        fixture.profile.refuseNames = false;
        ASSERT_TRUE( fixture.settings.Apply() && "Refused" == fixture.profile.name && page.GetErrorText().IsNULLOrEmpty() );

        // Cancel puts the name of the profile back and closes
        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "Half typed", false );
        fixture.settings.Cancel();
        ASSERT_TRUE( !fixture.settings.IsVisible() );
        ASSERT_TRUE( fixture.Open() );
        ASSERT_TRUE( "Refused" == page.GetDisplayNameText() && !page.HasChanges() );
        PressKey( fixture.context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( !fixture.settings.IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: The name changed elsewhere shows at once when nothing was typed, what was typed stays" )
    try
    {
        CAccountFixture fixture( 1280, 960, false, true );
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        ASSERT_TRUE( fixture.Open() );

        fixture.profile.ChangeElsewhere( "Carol" );
        fixture.Settle();
        ASSERT_TRUE( "Carol" == page.GetDisplayNameText() && !page.HasChanges() );

        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "Dave", false );
        fixture.profile.ChangeElsewhere( "Eve" );
        fixture.Settle();
        ASSERT_TRUE( "Dave" == page.GetDisplayNameText() && page.HasChanges() );
        ASSERT_TRUE( fixture.settings.Apply() && "Dave" == fixture.profile.name );

        // a profile that goes away takes its fields with it
        {
            CTestProfile temporary;
            page.SetProfile( &temporary );
            fixture.Settle();
            ASSERT_TRUE( page.HasProfileFields() && "Alice" == page.GetDisplayNameText() );
        }
        fixture.Settle();
        ASSERT_TRUE( GUCEF_NULL == page.GetProfile() && !page.HasProfileFields() && "Guest" == page.GetStateText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: A tool without a profile of its own keeps the name in the settings of the manager" )
    try
    {
        CAccountFixture fixture( 1280, 960, true, false );
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        AUTH::CAuthSettingsProfile settingsProfile( &fixture.manager );
        CEventCount changed( &settingsProfile, AUTH::CAuthSettingsProfile::ProfileChangedEvent );
        page.SetProfile( &settingsProfile );
        ASSERT_TRUE( fixture.Open() );
        ASSERT_TRUE( page.HasProfileFields() && page.GetDisplayNameText().IsNULLOrEmpty() && fixture.signer.GetKeyId() == page.GetProfileIdText() );

        // no name yet is no name to keep
        ASSERT_FALSE( fixture.settings.HasChanges() );
        TypeInto( fixture.context, page.GetWidget( "DisplayName" ), "Tool user", false );
        ASSERT_TRUE( fixture.settings.Apply() );
        ASSERT_TRUE( "Tool user" == fixture.manager.GetSettings().displayName && 1 == changed.count );

        // the name is part of the config of the manager, which the tool saves
        CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( fixture.manager.GetSettings().SaveConfig( config ) );
        AUTH::CAuthSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) && "Tool user" == loaded.displayName );

        page.SetProfile( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: It offers the providers the manager offers, the hint is gone, the check box has the setting of the manager" )
    try
    {
        AUTH::CAuthProviderDescriptor descriptor;
        descriptor.name = "fakebrowser";
        descriptor.displayName = "Fake Browser";
        descriptor.accountProviderId = "fake";
        descriptor.protocol = AUTH::AUTHPROTOCOL_PLATFORM;
        REQUIRE_TRUE( AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().RegisterDescriptor( descriptor ) );
        {
            CAccountFixture fixture( 1280, 960, true, true );
            REQUIRE_TRUE( fixture.IsReady() );
            AUTHGUI::CAccountSettingsForm& page = *fixture.page;
            ASSERT_TRUE( GUCEF_NULL != fixture.AddProvider( "fakebrowser" ) && GUCEF_NULL != fixture.AddProvider( "plainprov" ) );
            ASSERT_TRUE( fixture.Open() );
            CORE::CString::StringVector shown;
            page.GetShownProviders( shown );
            ASSERT_TRUE( 2 == shown.size() && "fakebrowser" == shown[ 0 ] && "plainprov" == shown[ 1 ] );
            ASSERT_TRUE( "Log in with Fake Browser" == page.GetWidget( "Login0" )->GetProperty( "Text" ) && "Log in with Plainprov" == page.GetWidget( "Login1" )->GetProperty( "Text" ) );
            ASSERT_TRUE( GUCEF_NULL == page.GetWidget( "NoProviders" ) && AUTHGUI::CAccountsPanel::LOGINPROGRESS_IDLE == page.GetProgress( "fakebrowser" ) );
            ASSERT_TRUE( "Guest" == page.GetStateText() && GUCEF_NULL != page.GetWidget( "NoAccounts" ) );
            ASSERT_TRUE( page.HasAutoLoginBox() && page.GetAutoLogin() );
            ASSERT_TRUE( page.HasProfileFields() );
        }
        AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "fakebrowser" );
    }
    catch ( ... )
    {
        ERRORHERE;
        AUTH::CAuthGlobal::Instance()->GetDescriptorRegistry().UnregisterDescriptor( "fakebrowser" );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: A login with the browser: waiting with Cancel, done, linked, then Unlink; no token shows" )
    try
    {
        CAccountFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        REQUIRE_TRUE( GUCEF_NULL != google );
        google->mode = CFakeLoginProvider::MODE_BROWSER;
        google->accountNamespace = "google";
        google->subject = "109876543210";
        ASSERT_TRUE( fixture.Open() );

        ClickWidget( fixture.context, page.GetWidget( "Login0" ) );
        ASSERT_TRUE( 1 == google->beginCount && google->lastOptions.linkToProfile );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_WAITING_FOR_BROWSER == page.GetProgress( "google" ) );
        ASSERT_TRUE( GUCEF_NULL != page.GetWidget( "Cancel0" ) && !IsWidgetEnabled( page, "Login0" ) && page.GetWidget( "Status0" )->GetProperty( "Text" ).HasSubstr( "Waiting" ) >= 0 );
        ASSERT_FALSE( page.Login( "google" ) );

        // the person is told how long the login waits, and can cancel it
        ASSERT_TRUE( "Waiting, up to 10 min" == page.GetWidget( "Status0" )->GetProperty( "Text" ) );

        // Cancel ends the login that waits
        ClickWidget( fixture.context, page.GetWidget( "Cancel0" ) );
        fixture.Settle();
        PumpFor( 10 );
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_IDLE == page.GetProgress( "google" ) && GUCEF_NULL == page.GetWidget( "Cancel0" ) );
        google->HonourCancel();
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_IDLE == page.GetProgress( "google" ) && page.GetErrorText().IsNULLOrEmpty() && "Guest" == page.GetStateText() );

        // again, now to the end
        ClickWidget( fixture.context, page.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( 2 == google->beginCount && google->Succeed() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_DONE == page.GetProgress( "google" ) && "Linked: Google" == page.GetStateText() );
        ASSERT_TRUE( "Logged in" == page.GetWidget( "Status0" )->GetProperty( "Text" ) && GUCEF_NULL != page.GetWidget( "Unlink0" ) && GUCEF_NULL == page.GetWidget( "NoAccounts" ) );
        ASSERT_TRUE( "Google: 109876543210" == page.GetWidget( "AccountText0" )->GetProperty( "Text" ) );

        // the page is a settings page: a login is no change that waits for Apply
        ASSERT_FALSE( page.HasChanges() || IsWidgetEnabled( fixture.settings, "Apply" ) );

        ClickWidget( fixture.context, page.GetWidget( "Unlink0" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTH::AUTHLOGINSTATE_GUEST == fixture.manager.GetLoginState() && "Guest" == page.GetStateText() );
        ASSERT_TRUE( GUCEF_NULL == page.GetWidget( "Unlink0" ) && GUCEF_NULL != page.GetWidget( "NoAccounts" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: The button to forget remembered account names is there only while a name is remembered, and forgets it at once; the name itself is never shown" )
    try
    {
        CAccountFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        REQUIRE_TRUE( GUCEF_NULL != google );
        google->mode = CFakeLoginProvider::MODE_BROWSER;
        google->accountNamespace = "google";
        google->subject = "109876543210";
        google->loginHint = "someone.private@example.invalid";
        ASSERT_TRUE( fixture.Open() );

        // nothing is remembered yet: nothing to forget, nothing new on the page
        ASSERT_FALSE( page.HasForgetLoginHintsButton() || page.ForgetLoginHints() );

        ClickWidget( fixture.context, page.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( google->Succeed() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_DONE == page.GetProgress( "google" ) );
        ASSERT_TRUE( fixture.manager.HasLoginHints() );

        // the button shows, with its own words, and no widget says the name
        ASSERT_TRUE( page.HasForgetLoginHintsButton() );
        ASSERT_TRUE( "Forget remembered account names" == page.GetWidget( "ForgetHints" )->GetProperty( "Text" ) );
        ASSERT_TRUE( page.GetWidget( "Status0" )->GetProperty( "Text" ).HasSubstr( "someone.private" ) < 0 && page.GetWidget( "AccountText0" )->GetProperty( "Text" ).HasSubstr( "someone.private" ) < 0 );
        ASSERT_FALSE( page.HasChanges() );

        // pressing it forgets everything and the button goes; a login is not forgotten with it
        ClickWidget( fixture.context, page.GetWidget( "ForgetHints" ) );
        fixture.Settle();
        ASSERT_FALSE( fixture.manager.HasLoginHints() );
        ASSERT_FALSE( page.HasForgetLoginHintsButton() );
        ASSERT_TRUE( "Linked: Google" == page.GetStateText() && GUCEF_NULL != page.GetWidget( "Unlink0" ) );
        ASSERT_FALSE( page.ForgetLoginHints() );

        // the next login remembers again, unless the setting says not
        ClickWidget( fixture.context, page.GetWidget( "Unlink0" ) );
        fixture.Settle();
        AUTH::CAuthSettings settings = fixture.manager.GetSettings();
        settings.rememberLoginHints = false;
        fixture.manager.SetSettings( settings );
        ClickWidget( fixture.context, page.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( google->Succeed() );
        fixture.Settle();
        ASSERT_FALSE( fixture.manager.HasLoginHints() || page.HasForgetLoginHintsButton() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: A device code: the page and the code in read only fields, Open opens the page, then done" )
    try
    {
        CAccountFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        CFakeLoginProvider* microsoft = fixture.AddProvider( "microsoft" );
        REQUIRE_TRUE( GUCEF_NULL != microsoft );
        microsoft->mode = CFakeLoginProvider::MODE_DEVICE_CODE;
        microsoft->accountNamespace = "microsoft";
        page.SetPreferDeviceCode( true );
        ASSERT_TRUE( fixture.Open() );
        CRecordingBrowser browser;

        ClickWidget( fixture.context, page.GetWidget( "Login0" ) );
        fixture.Settle();
        ASSERT_TRUE( true == microsoft->lastOptions.preferDeviceCode );
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_SHOWING_DEVICE_CODE == page.GetProgress( "microsoft" ) );
        GUI::CWidget* code = page.GetWidget( "Code0" );
        GUI::CWidget* url = page.GetWidget( "Url0" );
        ASSERT_TRUE( GUCEF_NULL != code && GUCEF_NULL != url && "WDJB-MJHT" == code->GetProperty( "Text" ) && "https://example.com/activate" == url->GetProperty( "Text" ) );
        ASSERT_TRUE( code->GetBoolProperty( "ReadOnly" ) && url->GetBoolProperty( "ReadOnly" ) && GUCEF_NULL != page.GetWidget( "Cancel0" ) );
        ClickWidget( fixture.context, page.GetWidget( "Open0" ) );
        ASSERT_TRUE( 1 == browser.urls.size() && browser.urls[ 0 ].HasSubstr( "WDJB-MJHT" ) >= 0 );
        browser.accept = false;
        ClickWidget( fixture.context, page.GetWidget( "Open0" ) );
        ASSERT_TRUE( page.GetErrorText().HasSubstr( "browser" ) >= 0 );

        ASSERT_TRUE( microsoft->Succeed() );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_DONE == page.GetProgress( "microsoft" ) && GUCEF_NULL == page.GetWidget( "Code0" ) );
        ASSERT_TRUE( "Linked: Microsoft" == page.GetStateText() || "Linked: Fake" == page.GetStateText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: A failed login says why, a provider that cannot start says so, the other providers go on" )
    try
    {
        CAccountFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        CFakeLoginProvider* google = fixture.AddProvider( "google" );
        CFakeLoginProvider* amazon = fixture.AddProvider( "amazon" );
        CFakeLoginProvider* openId2 = fixture.AddProvider( "fakeopenid" );
        REQUIRE_TRUE( GUCEF_NULL != google && GUCEF_NULL != amazon && GUCEF_NULL != openId2 );
        google->mode = CFakeLoginProvider::MODE_FAIL;
        amazon->mode = CFakeLoginProvider::MODE_REFUSE;
        openId2->mode = CFakeLoginProvider::MODE_INSTANT;
        openId2->accountNamespace = "fakeopenid";
        ASSERT_TRUE( fixture.Open() );
        CORE::CString::StringVector shown;
        page.GetShownProviders( shown );
        ASSERT_TRUE( 3 == shown.size() );

        ASSERT_TRUE( page.Login( "google" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_FAILED == page.GetProgress( "google" ) );
        ASSERT_TRUE( page.GetErrorText().HasSubstr( "access denied" ) >= 0 && page.GetWidget( "Error" )->GetProperty( "Text" ) == page.GetErrorText() );

        ASSERT_FALSE( page.Login( "amazon" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_FAILED == page.GetProgress( "amazon" ) && !page.GetErrorText().IsNULLOrEmpty() );

        ASSERT_TRUE( page.Login( "fakeopenid" ) );
        fixture.Settle();
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_DONE == page.GetProgress( "fakeopenid" ) && page.GetErrorText().IsNULLOrEmpty() );
        ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_FAILED == page.GetProgress( "google" ) && "Linked: Fakeopenid" == page.GetStateText() );

        // Cancel (the settings were left) shows no error either
        fixture.settings.Cancel();
        ASSERT_TRUE( page.GetErrorText().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: The check box is a value of the page: Apply gives it to the manager and tells, Cancel puts it back, a login does not take it away" )
    try
    {
        CAccountFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        AUTHGUI::CAccountSettingsForm& page = *fixture.page;
        CFakeLoginProvider* openId2 = fixture.AddProvider( "fakeopenid" );
        REQUIRE_TRUE( GUCEF_NULL != openId2 );
        openId2->accountNamespace = "fakeopenid";
        CEventCount autoLoginChanges( &page, AUTHGUI::CAccountSettingsForm::AutoLoginChangedEvent );
        ASSERT_TRUE( fixture.Open() );
        ASSERT_TRUE( page.HasAutoLoginBox() && page.GetAutoLogin() && fixture.manager.GetSettings().autoLogin );

        // clicking is a change that waits
        ClickWidget( fixture.context, page.GetWidget( "AutoLogin" ) );
        ASSERT_TRUE( !page.GetAutoLogin() && fixture.manager.GetSettings().autoLogin );
        ASSERT_TRUE( page.HasChanges() && IsWidgetEnabled( fixture.settings, "Apply" ) );

        // a login in between makes the page again: the box stays as the person set it
        ASSERT_TRUE( page.Login( "fakeopenid" ) );
        fixture.Settle();
        ASSERT_TRUE( "Linked: Fakeopenid" == page.GetStateText() && !page.GetAutoLogin() && page.HasChanges() && fixture.manager.GetSettings().autoLogin );

        ASSERT_TRUE( fixture.settings.Apply() );
        ASSERT_TRUE( !fixture.manager.GetSettings().autoLogin && 1 == autoLoginChanges.count && !page.HasChanges() );
        CORE::CDataNode config( "Auth", GUCEF_DATATYPE_OBJECT );
        ASSERT_TRUE( fixture.manager.GetSettings().SaveConfig( config ) );
        AUTH::CAuthSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( config ) && !loaded.autoLogin );

        // applied again without a change: nothing is told
        page.SetAutoLogin( false );
        ASSERT_FALSE( page.HasChanges() );
        ASSERT_TRUE( page.SetAutoLogin( true ) && fixture.settings.Apply() && fixture.manager.GetSettings().autoLogin && 2 == autoLoginChanges.count );

        // Cancel: the setting of the manager is what the box shows
        ASSERT_TRUE( page.SetAutoLogin( false ) );
        fixture.settings.Cancel();
        ASSERT_TRUE( fixture.Open() && page.GetAutoLogin() && fixture.manager.GetSettings().autoLogin && 2 == autoLoginChanges.count );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: A manager that goes away is let go, and a login under way when the page gets the manager is waited for" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        REQUIRE_TRUE( GUCEF_NULL != context );
        GUI::CSettingsForm settings( context );
        AUTHGUI::CAccountSettingsForm* page = static_cast< AUTHGUI::CAccountSettingsForm* >( settings.AddPage( "Account" ) );
        REQUIRE_TRUE( GUCEF_NULL != page && settings.Build() );
        {
            CAccountFixture inner;
            REQUIRE_TRUE( inner.IsReady() );
            CFakeLoginProvider* google = inner.AddProvider( "google" );
            REQUIRE_TRUE( GUCEF_NULL != google );
            google->mode = CFakeLoginProvider::MODE_BROWSER;
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( inner.manager.Login( "google", AUTH::CAuthOptions(), future ) );
            page->SetAuthManager( &inner.manager );
            ASSERT_TRUE( AUTHGUI::CAccountsPanel::LOGINPROGRESS_WAITING_FOR_BROWSER == page->GetProgress( "google" ) );
            ASSERT_TRUE( &inner.manager == page->GetAuthManager() );
            google->Succeed();
            PumpFor( 30 );
            inner.manager.RemoveProvider( "google" );
            page->SetAuthManager( GUCEF_NULL );
        }
        ASSERT_TRUE( GUCEF_NULL == page->GetAuthManager() );
        ASSERT_TRUE( settings.Open() && "Guest" == page->GetStateText() );

        // a manager that dies while the page has it
        {
            AUTH::CAuthManager* temporary = new AUTH::CAuthManager();
            page->SetAuthManager( temporary );
            delete temporary;
        }
        ASSERT_TRUE( GUCEF_NULL == page->GetAuthManager() );
        settings.Cancel();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AccountSettings: Several pages: Open with a title puts that page in front, an unknown title leaves the front page" )
    try
    {
        CAccountFixture fixture;
        REQUIRE_TRUE( fixture.IsReady() );
        GUI::CSettingsForm other( fixture.context );
        GUI::CSettingsPageForm* general = other.AddPage( "General" );
        AUTHGUI::CAccountSettingsForm* account = static_cast< AUTHGUI::CAccountSettingsForm* >( other.AddPage( "Account" ) );
        REQUIRE_TRUE( GUCEF_NULL != general && GUCEF_NULL != account );
        account->SetAuthManager( &fixture.manager );
        ASSERT_TRUE( other.Build() );
        ASSERT_TRUE( other.Open() && "General" == other.GetSelectedPageTitle() );
        other.Cancel();
        ASSERT_TRUE( other.Open( "Account" ) && "Account" == other.GetSelectedPageTitle() );
        other.Cancel();
        ASSERT_TRUE( other.Open( "Nothing like it" ) && "Account" == other.GetSelectedPageTitle() );
        ASSERT_FALSE( other.SelectPage( "Nothing like it" ) );
        ASSERT_TRUE( other.SelectPage( "General" ) && "General" == other.GetSelectedPageTitle() );
        other.Cancel();
        account->SetAuthManager( GUCEF_NULL );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
