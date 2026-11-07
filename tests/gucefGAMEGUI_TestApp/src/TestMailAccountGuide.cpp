/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_COM_CEMAILPROVIDERPRESETS_H
#include "gucefCOM_CEMailProviderPresets.h"
#define GUCEF_COM_CEMAILPROVIDERPRESETS_H
#endif /* GUCEF_COM_CEMAILPROVIDERPRESETS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#include "gucefGAMEGUI_CEMailAccountForm.h"
#include "gucefGAMEGUI_TestCommon.h"
#include "TestMatchLayoutChecks.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const char* const GuidePassword = "Zq9-guide-secret-31";

/** Records the events of the mail form that the guide adds, and the data of save and test */
class CGuideRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CGuideRecorder > TEventCallback;

    UInt32 saves;
    UInt32 tests;
    UInt32 signIns;
    UInt32 signOuts;
    CORE::CString signInProvider;
    GAMEGUI::CEMailAccountFormData last;

    explicit CGuideRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), saves( 0 ), tests( 0 ), signIns( 0 ), signOuts( 0 ), signInProvider(), last()
    {
        TEventCallback saved( this, &CGuideRecorder::OnSave );
        SubscribeTo( form, GAMEGUI::CEMailAccountForm::SaveRequestedEvent, saved );
        TEventCallback tested( this, &CGuideRecorder::OnTest );
        SubscribeTo( form, GAMEGUI::CEMailAccountForm::TestRequestedEvent, tested );
        TEventCallback signedIn( this, &CGuideRecorder::OnSignIn );
        SubscribeTo( form, GAMEGUI::CEMailAccountForm::SignInRequestedEvent, signedIn );
        TEventCallback signedOut( this, &CGuideRecorder::OnSignOut );
        SubscribeTo( form, GAMEGUI::CEMailAccountForm::SignOutRequestedEvent, signedOut );
    }

    virtual ~CGuideRecorder() { UnsubscribeAllFromObserver(); }

    void OnSave( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++saves;
        if ( GAMEGUI::CEMailAccountFormData* data = CORE::CICloneable::CloneableAs< GAMEGUI::CEMailAccountFormData >( eventData ) )
            last = *data;
    }

    void OnTest( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++tests;
        if ( GAMEGUI::CEMailAccountFormData* data = CORE::CICloneable::CloneableAs< GAMEGUI::CEMailAccountFormData >( eventData ) )
            last = *data;
    }

    void OnSignIn( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++signIns;
        if ( const CORE::TCloneableString* name = static_cast< const CORE::TCloneableString* >( eventData ) )
            signInProvider = name->GetData();
    }

    void OnSignOut( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { ++signOuts; }
};

CORE::CString::StringVector
Providers( const char* a, const char* b = GUCEF_NULL )
{
    CORE::CString::StringVector list;
    if ( GUCEF_NULL != a )
        list.push_back( a );
    if ( GUCEF_NULL != b )
        list.push_back( b );
    return list;
}

CORE::CString
TextOf( GAMEGUI::CEMailAccountForm& form, const char* name )
{
    GUI::CWidget* widget = form.GetWidget( name );
    return GUCEF_NULL != widget ? widget->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString( "<missing>" );
}

Int32
SelectedOf( GAMEGUI::CEMailAccountForm& form, const char* name )
{
    GUI::CCombobox* choice = form.GetWidgetAs< GUI::CCombobox >( name );
    return GUCEF_NULL != choice ? choice->GetSelectedIndex() : -99;
}

bool
Shows( GAMEGUI::CEMailAccountForm& form, const char* name )
{
    GUI::CWidget* widget = form.GetWidget( name );
    return GUCEF_NULL != widget && widget->IsEffectivelyVisible();
}

/** The problem with the layout of the page that is shown: a control outside the window or two that overlap */
CORE::CString
PageProblem( GAMEGUI::CEMailAccountForm& form )
{
    TMfWidgetVector all;
    MfCollect( form.GetRootWidget(), all );
    TMfWidgetVector controls;
    const GUI::CRect window = form.GetRootWidget()->GetRect();
    for ( size_t i=1; i<all.size(); ++i )
    {
        if ( !all[ i ]->IsEffectivelyVisible() || all[ i ]->GetRect().IsEmpty() || MfIsContainerType( all[ i ]->GetType() ) )
            continue;
        if ( !MfInside( all[ i ]->GetRect(), window ) )
            return "\"" + all[ i ]->GetName() + "\" is outside the window";
        controls.push_back( all[ i ] );
    }
    for ( size_t i=0; i<controls.size(); ++i )
        for ( size_t j=i+1; j<controls.size(); ++j )
            if ( !controls[ i ]->GetRect().Intersection( controls[ j ]->GetRect() ).IsEmpty() )
                return "\"" + controls[ i ]->GetName() + "\" overlaps \"" + controls[ j ]->GetName() + "\"";
    return CORE::CString::Empty;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformMailAccountGuideTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.MailAccountGuide" );

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: the address comes first, the provider is found from its domain and the servers, ports and security are filled in" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        ASSERT_TRUE( form.Open() );

        // nothing typed: the form says where to start
        ASSERT_TRUE( form.GetProviderId() == "custom" && form.GetGuidance().HasSubstr( "Start with your mail address" ) >= 0 );

        // the address is the first field of the window
        GUI::CWidget* address = form.GetWidget( "Address" );
        ASSERT_TRUE( GUCEF_NULL != address && address->GetRect().top <= form.GetWidget( "DisplayName" )->GetRect().top && address->GetRect().top <= form.GetWidget( "Provider" )->GetRect().top );

        TypeInto( context, address, "me@gmail.com", false );
        ASSERT_TRUE( form.GetProviderId() == "gmail" );
        ASSERT_TRUE( TextOf( form, "IncomingHost" ) == "imap.gmail.com" && TextOf( form, "IncomingPort" ) == "993" && 0 == SelectedOf( form, "IncomingSecurity" ) && 0 == SelectedOf( form, "Protocol" ) );
        ASSERT_TRUE( TextOf( form, "OutgoingHost" ) == "smtp.gmail.com" && TextOf( form, "OutgoingPort" ) == "587" && 1 == SelectedOf( form, "OutgoingSecurity" ) );
        ASSERT_TRUE( 0 == SelectedOf( form, "Provider" ) );
        ASSERT_TRUE( form.GetGuidance().HasSubstr( "app password" ) >= 0 && form.GetGuidance().HasSubstr( "2-step verification" ) >= 0 );
        ASSERT_TRUE( TextOf( form, "ServersNote" ).HasSubstr( "Servers filled in for" ) >= 0 );

        // the data carries the provider and the servers that were filled in
        GAMEGUI::CEMailAccountFormData data = form.GetData();
        ASSERT_TRUE( data.providerId == "gmail" && 0 == data.authMode && data.incomingHost == "imap.gmail.com" && 993 == data.incomingPort && 0 == data.incomingSecurity );
        ASSERT_TRUE( data.outgoingHost == "smtp.gmail.com" && 587 == data.outgoingPort && 1 == data.outgoingSecurity && data.useImap );

        // another provider by another address: capitals and a plus tag do not matter
        address->SetProperty( GUI::EWidgetProperty_Text, "Me+redux@Yahoo.COM" );
        ASSERT_TRUE( form.GetProviderId() == "yahoo" && TextOf( form, "IncomingHost" ) == "imap.mail.yahoo.com" && TextOf( form, "OutgoingHost" ) == "smtp.mail.yahoo.com" );
        address->SetProperty( GUI::EWidgetProperty_Text, "x@outlook.com" );
        ASSERT_TRUE( form.GetProviderId() == "microsoft" && TextOf( form, "IncomingHost" ) == "outlook.office365.com" && TextOf( form, "OutgoingHost" ) == "smtp.office365.com" && 1 == SelectedOf( form, "OutgoingSecurity" ) );
        address->SetProperty( GUI::EWidgetProperty_Text, "x@icloud.com" );
        ASSERT_TRUE( form.GetProviderId() == "icloud" && TextOf( form, "IncomingHost" ) == "imap.mail.me.com" );
        address->SetProperty( GUI::EWidgetProperty_Text, "x@web.de" );
        ASSERT_TRUE( form.GetProviderId() == "webde" && TextOf( form, "IncomingHost" ) == "imap.web.de" );
        address->SetProperty( GUI::EWidgetProperty_Text, "x@fastmail.com" );
        ASSERT_TRUE( form.GetProviderId() == "fastmail" );
        address->SetProperty( GUI::EWidgetProperty_Text, "x@gmx.net" );
        ASSERT_TRUE( form.GetProviderId() == "gmx" && TextOf( form, "OutgoingHost" ) == "mail.gmx.net" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: an address of an unknown domain is the custom provider, the typed servers are never overwritten, a picked provider stays, a known domain wins" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        ASSERT_TRUE( form.Open() );
        GUI::CWidget* address = form.GetWidget( "Address" );

        address->SetProperty( GUI::EWidgetProperty_Text, "me@company.example" );
        ASSERT_TRUE( form.GetProviderId() == "custom" && form.GetGuidance().HasSubstr( "server" ) >= 0 );
        form.GetWidget( "IncomingHost" )->SetProperty( GUI::EWidgetProperty_Text, "mail.company.example" );
        form.GetWidget( "OutgoingHost" )->SetProperty( GUI::EWidgetProperty_Text, "smtp.company.example" );
        form.GetWidget( "IncomingPort" )->SetProperty( GUI::EWidgetProperty_Text, "1993" );
        ASSERT_TRUE( TextOf( form, "ServersNote" ).HasSubstr( "fill them in" ) >= 0 );

        // more typing of the address leaves what was typed in the server fields
        address->SetProperty( GUI::EWidgetProperty_Text, "me@other-company.example" );
        ASSERT_TRUE( form.GetProviderId() == "custom" && TextOf( form, "IncomingHost" ) == "mail.company.example" && TextOf( form, "IncomingPort" ) == "1993" && TextOf( form, "OutgoingHost" ) == "smtp.company.example" );

        // an address that is no address yet does not change the servers either
        address->SetProperty( GUI::EWidgetProperty_Text, "me@" );
        ASSERT_TRUE( form.GetProviderId() == "custom" && TextOf( form, "IncomingHost" ) == "mail.company.example" );

        // a hosted domain: the person picks the provider by hand, the choice stays while the domain is unknown
        address->SetProperty( GUI::EWidgetProperty_Text, "me@company.example" );
        ASSERT_TRUE( PickItem( context, form.GetWidget( "Provider" ), 0 ) );
        ASSERT_TRUE( form.GetProviderId() == "gmail" && TextOf( form, "IncomingHost" ) == "imap.gmail.com" );
        address->SetProperty( GUI::EWidgetProperty_Text, "me@company2.example" );
        ASSERT_TRUE( form.GetProviderId() == "gmail" && TextOf( form, "IncomingHost" ) == "imap.gmail.com" );
        // a known domain takes over
        address->SetProperty( GUI::EWidgetProperty_Text, "me@web.de" );
        ASSERT_TRUE( form.GetProviderId() == "webde" && TextOf( form, "IncomingHost" ) == "imap.web.de" );

        // SelectProvider is the same as picking; an unknown id is refused
        ASSERT_TRUE( form.SelectProvider( "fastmail" ) && form.GetProviderId() == "fastmail" && TextOf( form, "OutgoingHost" ) == "smtp.fastmail.com" );
        ASSERT_FALSE( form.SelectProvider( "nonesuch" ) );
        ASSERT_TRUE( form.GetProviderId() == "fastmail" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: the table of providers is data: a table of the application replaces the defaults" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        COM::CEMailProviderPresets table;
        CORE::CDataNode config( "MailProviders" );
        CORE::CDataNode* corp = config.AddChild( "corpmail" );
        corp->SetAttribute( "displayName", "Corp mail" );
        corp->SetAttribute( "domains", "corp.example" );
        corp->SetAttribute( "imapHost", "imap.corp.example" );
        corp->SetAttribute( "smtpHost", "smtp.corp.example" );
        corp->SetAttribute( "smtpPort", (UInt16) 2525 );
        corp->SetAttribute( "passwordHelp", "Use the password of your corp account." );
        ASSERT_TRUE( table.LoadConfig( config ) );
        GAMEGUI::CEMailAccountForm form( context );
        form.SetProviderPresets( &table );
        ASSERT_TRUE( form.Open() );
        form.GetWidget( "Address" )->SetProperty( GUI::EWidgetProperty_Text, "me@corp.example" );
        ASSERT_TRUE( form.GetProviderId() == "corpmail" && TextOf( form, "IncomingHost" ) == "imap.corp.example" && TextOf( form, "OutgoingPort" ) == "2525" );
        ASSERT_TRUE( form.GetGuidance().HasSubstr( "password of your corp account" ) >= 0 );
        ASSERT_TRUE( SelectedOf( form, "Provider" ) == (Int32) ( table.GetCount() - 1 ) || form.GetWidget( "Provider" )->GetProperty( GUI::EWidgetProperty_Items ).HasSubstr( "Corp mail" ) >= 0 );
        GAMEGUI::CEMailAccountFormData data = form.GetData();
        ASSERT_TRUE( data.providerId == "corpmail" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: Sign in with Google or Microsoft only exists when the application has a client id for it, and the form says in plain words what that means" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        CGuideRecorder recorder( &form );
        ASSERT_TRUE( form.Open() );
        context->RenderFrame( 16 );
        GUI::CWidget* address = form.GetWidget( "Address" );

        // no client id: no button, and the text says so
        address->SetProperty( GUI::EWidgetProperty_Text, "me@gmail.com" );
        ASSERT_FALSE( form.IsSignInShown() || Shows( form, "SignIn" ) );
        ASSERT_TRUE( form.GetGuidance().HasSubstr( "Sign-in with Google is not available in this build" ) >= 0 && form.GetGuidance().HasSubstr( "app password" ) >= 0 );
        address->SetProperty( GUI::EWidgetProperty_Text, "me@outlook.com" );
        ASSERT_FALSE( form.IsSignInShown() || Shows( form, "SignIn" ) );
        ASSERT_TRUE( form.GetGuidance().HasSubstr( "Sign-in with Microsoft is not available in this build" ) >= 0 );
        ASSERT_TRUE( Shows( form, "Password" ) );
        form.RequestSignIn();
        ASSERT_TRUE( 0 == recorder.signIns );

        // a client id for Google only: Gmail gets the button, Outlook does not
        form.SetOAuthProviders( Providers( "google" ) );
        ASSERT_FALSE( form.IsSignInShown() || Shows( form, "SignIn" ) );
        address->SetProperty( GUI::EWidgetProperty_Text, "me@gmail.com" );
        ASSERT_TRUE( form.IsSignInShown() && Shows( form, "SignIn" ) && form.GetSignInText() == "Sign in with Google" && TextOf( form, "SignIn" ) == "Sign in with Google" );
        ASSERT_TRUE( form.GetGuidance().HasSubstr( "not available in this build" ) < 0 && form.GetGuidance().HasSubstr( "Sign in with Google" ) >= 0 );

        // the button asks the application to sign in, with the name of the provider
        context->RenderFrame( 16 );
        context->FlushChanges();
        ClickWidget( context, form.GetWidget( "SignIn" ) );
        ASSERT_TRUE( 1 == recorder.signIns && recorder.signInProvider == "google" );

        // Microsoft with its own client id
        form.SetOAuthProviders( Providers( "google", "microsoft" ) );
        address->SetProperty( GUI::EWidgetProperty_Text, "me@hotmail.com" );
        ASSERT_TRUE( form.IsSignInShown() && form.GetSignInText() == "Sign in with Microsoft" && form.GetGuidance().HasSubstr( "not available in this build" ) < 0 );
        form.RequestSignIn();
        ASSERT_TRUE( 2 == recorder.signIns && recorder.signInProvider == "microsoft" );

        // a provider that has no sign-in never has the button, whatever the application can do
        address->SetProperty( GUI::EWidgetProperty_Text, "me@fastmail.com" );
        ASSERT_FALSE( form.IsSignInShown() || Shows( form, "SignIn" ) );
        ASSERT_TRUE( form.GetGuidance().HasSubstr( "Sign-in with" ) < 0 );

        // the button belongs to the account page
        address->SetProperty( GUI::EWidgetProperty_Text, "me@gmail.com" );
        ASSERT_TRUE( Shows( form, "SignIn" ) );
        form.ShowServersPage( true );
        ASSERT_FALSE( Shows( form, "SignIn" ) );
        form.ShowServersPage( false );
        ASSERT_TRUE( Shows( form, "SignIn" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: when the mailbox is signed in to, the password is not asked for and the data says so; signing out and another provider bring the password back" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        CGuideRecorder recorder( &form );
        form.SetOAuthProviders( Providers( "google", "microsoft" ) );
        ASSERT_TRUE( form.Open() );
        context->RenderFrame( 16 );
        form.GetWidget( "Address" )->SetProperty( GUI::EWidgetProperty_Text, "me@gmail.com" );
        GUI::CSecretInput* password = form.GetWidgetAs< GUI::CSecretInput >( "Password" );
        ASSERT_TRUE( GUCEF_NULL != password && Shows( form, "Password" ) && !Shows( form, "SignInState" ) && !Shows( form, "SignOut" ) );
        GAMEGUI::CEMailAccountFormData typed;
        typed.address = "me@gmail.com";
        typed.password = GuidePassword;
        form.SetData( typed );
        ASSERT_TRUE( password->HasValue() );

        form.SetSignedIn( true, "Signed in with Google. The password is not used for this mailbox." );
        ASSERT_TRUE( form.IsSignedIn() && !Shows( form, "Password" ) && !Shows( form, "PasswordLabel" ) && Shows( form, "SignInState" ) && Shows( form, "SignOut" ) );
        ASSERT_TRUE( TextOf( form, "SignInState" ).HasSubstr( "Signed in with Google" ) >= 0 );
        ASSERT_TRUE( form.GetSignInText() == "Sign in with Google again" );
        ASSERT_FALSE( password->HasValue() );
        GAMEGUI::CEMailAccountFormData data = form.GetData();
        ASSERT_TRUE( 1 == data.authMode && data.oauthProvider == "google" && data.password.IsNULLOrEmpty() && data.providerId == "gmail" );
        // the same on the servers page: the strip is on both
        form.ShowServersPage( true );
        ASSERT_TRUE( Shows( form, "SignInState" ) && Shows( form, "SignOut" ) && !Shows( form, "Password" ) );
        form.ShowServersPage( false );

        // a form that is filled from saved data shows the state
        GAMEGUI::CEMailAccountForm second( context );
        ASSERT_TRUE( second.Open() );
        GAMEGUI::CEMailAccountFormData saved;
        saved.address = "me@outlook.com";
        saved.authMode = 1;
        saved.oauthProvider = "microsoft";
        saved.providerId = "microsoft";
        second.SetData( saved );
        ASSERT_TRUE( second.IsSignedIn() && TextOf( second, "SignInState" ).HasSubstr( "Microsoft" ) >= 0 && !Shows( second, "Password" ) );
        ASSERT_TRUE( 1 == second.GetData().authMode && second.GetData().oauthProvider == "microsoft" );

        // the person wants a password instead (the second window is closed: it was opened on top of the first)
        second.Close();
        context->RenderFrame( 16 );
        context->FlushChanges();
        ClickWidget( context, form.GetWidget( "SignOut" ) );
        ASSERT_TRUE( 1 == recorder.signOuts );
        form.SetSignedIn( false, CORE::CString::Empty );
        ASSERT_TRUE( !form.IsSignedIn() && Shows( form, "Password" ) && !Shows( form, "SignInState" ) && 0 == form.GetData().authMode && form.GetData().oauthProvider.IsNULLOrEmpty() );

        // an address of another provider ends the sign-in of the first
        form.SetSignedIn( true, "Signed in with Google." );
        form.GetWidget( "Address" )->SetProperty( GUI::EWidgetProperty_Text, "me@web.de" );
        ASSERT_TRUE( !form.IsSignedIn() && Shows( form, "Password" ) && form.GetProviderId() == "webde" );
        form.SetSignedIn( true, "Signed in with Google." );
        form.SelectProvider( "yahoo" );
        ASSERT_TRUE( form.IsSignedIn() || !Shows( form, "SignOut" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: hostile or malformed input stops Save and Test with a plain sentence that does not repeat it; empty fields are left to the application" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        GAMEGUI::CEMailAccountForm form( context );
        CGuideRecorder recorder( &form );
        ASSERT_TRUE( form.Open() );

        // empty everything: the application says what is missing
        form.RequestSave();
        form.RequestTest();
        ASSERT_TRUE( 1 == recorder.saves && 1 == recorder.tests );

        struct SBad { const char* field; const char* value; bool serversPage; };
        const SBad bad[] = {
            { "Address", "me@gmail.com\r\nBcc: evil@example.org", false }, { "Address", "me@@gmail.com", false }, { "Address", "no address", false },
            { "UserName", "me\r\nRCPT TO:<x@example.org>", false }, { "DisplayName", "Name\r\nBcc: x@example.org", false },
            { "IncomingHost", "imap.example.org\r\nX", true }, { "IncomingHost", "imap example.org", true }, { "OutgoingHost", "smtp.example.org/../x", true },
            { "IncomingPort", "99999", true }, { "OutgoingPort", "abc", true }, { "IncomingPort", "-1", true },
            { "Mailbox", "INBOX\r\nA1 LOGOUT", true }, { "ProcessedFolder", "Games\x01", true }
        };
        UInt32 saves = recorder.saves, tests = recorder.tests;
        for ( size_t i=0; i<sizeof( bad ) / sizeof( bad[ 0 ] ); ++i )
        {
            GAMEGUI::CEMailAccountFormData clean;
            clean.address = "me@example.org";
            form.SetData( clean );
            form.ShowServersPage( false );
            form.GetWidget( bad[ i ].field )->SetProperty( GUI::EWidgetProperty_Text, bad[ i ].value );
            form.SetStatus( CORE::CString::Empty, false );
            form.RequestSave();
            ASSERT_TRUE( saves == recorder.saves );
            ASSERT_FALSE( form.GetStatus().IsNULLOrEmpty() );
            ASSERT_TRUE( form.GetStatus().HasSubstr( "Bcc" ) < 0 && form.GetStatus().HasSubstr( "evil" ) < 0 && form.GetStatus().HasSubstr( "RCPT" ) < 0 && form.GetStatus().HasSubstr( "LOGOUT" ) < 0 );
            ASSERT_TRUE( bad[ i ].serversPage == form.IsServersPageShown() );
            form.RequestTest();
            ASSERT_TRUE( tests == recorder.tests );
        }
        // a field the sign-in button reads is checked too
        form.SetOAuthProviders( Providers( "google" ) );
        GAMEGUI::CEMailAccountFormData gmail;
        gmail.address = "me@gmail.com\r\nBcc: x@example.org";
        form.SetData( gmail );
        const UInt32 signIns = recorder.signIns;
        form.RequestSignIn();
        ASSERT_TRUE( signIns == recorder.signIns );

        // good input goes through
        GAMEGUI::CEMailAccountFormData good;
        good.address = "me+tag@example.org";
        good.displayName = "Dinand";
        good.incomingHost = "imap.example.org";
        good.outgoingHost = "smtp.example.org";
        good.incomingPort = 993;
        good.outgoingPort = 587;
        form.SetData( good );
        form.RequestSave();
        ASSERT_TRUE( saves + 1 == recorder.saves && recorder.last.address == "me+tag@example.org" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: the two pages, the password and the status are on both, nothing is lost by switching, the password is never in a log" )
    try
    {
        CORE::CString logText;
        {
            TESTFW::CLogTextCapture capture;
            GUI::TGuiContextPtr contextPtr = CreateFormContext();
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            GAMEGUI::CEMailAccountForm form( context );
            CGuideRecorder recorder( &form );
            ASSERT_TRUE( form.Open() );
            context->RenderFrame( 16 );
            context->FlushChanges();
            ASSERT_FALSE( form.IsServersPageShown() );
            ASSERT_TRUE( Shows( form, "Address" ) && Shows( form, "Provider" ) && Shows( form, "Guidance" ) && Shows( form, "Password" ) && Shows( form, "Status" ) );
            ASSERT_FALSE( Shows( form, "IncomingHost" ) || Shows( form, "OutgoingHost" ) || Shows( form, "Mailbox" ) || Shows( form, "ProcessedAction" ) );

            GUI::CWidget* address = form.GetWidget( "Address" );
            address->SetProperty( GUI::EWidgetProperty_Text, "me@gmail.com" );
            ClickWidget( context, form.GetWidget( "Password" ) );
            context->InjectText( GuidePassword );
            ASSERT_TRUE( GuidePassword == form.GetData().password );

            ClickWidget( context, form.GetWidget( "PageServers" ) );
            ASSERT_TRUE( form.IsServersPageShown() && Shows( form, "IncomingHost" ) && Shows( form, "Mailbox" ) && Shows( form, "ProcessedAction" ) && Shows( form, "Password" ) && Shows( form, "Status" ) );
            ASSERT_FALSE( Shows( form, "Address" ) || Shows( form, "Guidance" ) || Shows( form, "Provider" ) );
            // the password survived the switch
            context->RenderFrame( 16 );
            ASSERT_TRUE( GuidePassword == form.GetData().password );
            ClickWidget( context, form.GetWidget( "PageAccount" ) );
            ASSERT_TRUE( !form.IsServersPageShown() && Shows( form, "Address" ) && GuidePassword == form.GetData().password );

            form.RequestSave();
            ASSERT_TRUE( 1 == recorder.saves && GuidePassword == recorder.last.password );
            form.Close();
            ASSERT_TRUE( form.GetData().password.IsNULLOrEmpty() );
            logText = capture.GetText();
        }
        ASSERT_TRUE( logText.HasSubstr( "guide-secret" ) < 0 && logText.HasSubstr( "Zq9" ) < 0 && logText.HasSubstr( "me@gmail.com" ) < 0 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "MailAccountGuide: both pages fit and no control overlaps another from 1024 x 768 to the ultrawide sizes, and the pictures are made for a Gmail, a Microsoft (sign-in hidden and available), a custom and an empty form" )
    try
    {
        struct SSize { UInt32 width; UInt32 height; const char* name; };
        const SSize sizes[] = { { 1242, 952, "1242x952" }, { 3440, 1440, "3440x1440" }, { 3840, 1080, "3840x1080" }, { 1024, 768, "1024x768" } };
        struct SCase { const char* name; const char* address; const char* oauth; bool servers; bool signedIn; };
        const SCase cases[] = {
            { "gmail_password", "me@gmail.com", "", false, false },
            { "gmail_servers", "me@gmail.com", "", true, false },
            { "microsoft_hidden", "me@outlook.com", "", false, false },
            { "microsoft_available", "me@outlook.com", "microsoft", false, false },
            { "microsoft_signed_in", "me@outlook.com", "microsoft", false, true },
            { "custom", "me@company.example", "google", false, false },
            { "custom_servers", "me@company.example", "", true, false },
            { "empty", "", "", false, false }
        };
        for ( int s=0; s<4; ++s )
        {
            for ( size_t c=0; c<sizeof( cases ) / sizeof( cases[ 0 ] ); ++c )
            {
                GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ s ].width, sizes[ s ].height, true );
                GUI::CGUIContext* context = ContextOf( contextPtr );
                ASSERT_TRUE( GUCEF_NULL != context );
                if ( GUCEF_NULL == context )
                    return;
                const COM::CEMailProviderPreset* found = GUCEF_NULL;
                GAMEGUI::CEMailAccountForm form( context );
                form.SetPlayerCode( "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef" );
                form.SetOAuthProviders( Providers( cases[ c ].oauth[ 0 ] ? cases[ c ].oauth : GUCEF_NULL ) );
                ASSERT_TRUE( form.Open() );
                GAMEGUI::CEMailAccountFormData data;
                data.address = cases[ c ].address;
                data.displayName = "Dinand";
                if ( !cases[ c ].signedIn )
                    data.password = GuidePassword;
                form.SetData( data );
                if ( COM::CEMailProviderPresets::RECOGNIZE_KNOWN == form.GetProviderPresets().Recognize( cases[ c ].address, found ) )
                    form.SelectProvider( found->id );
                if ( cases[ c ].signedIn )
                    form.SetSignedIn( true, "Signed in with Microsoft. The password is not used for this mailbox." );
                form.ShowServersPage( cases[ c ].servers );
                form.SetStatus( "Fill in the mailbox, test the servers and save.", false );
                context->RenderFrame( 16 );
                context->FlushChanges();
                context->RenderFrame( 16 );
                const GUI::CRect screen( 0, 0, (Int32) sizes[ s ].width, (Int32) sizes[ s ].height );
                ASSERT_TRUE( MfInside( form.GetRootWidget()->GetRect(), screen ) );
                const CORE::CString problem = PageProblem( form );
                if ( !problem.IsNULLOrEmpty() )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Layout: " << problem << " in " << cases[ c ].name << " at " << sizes[ s ].name );
                ASSERT_TRUE( problem.IsNULLOrEmpty() );
                WriteShot( CaptureFrame( context ), CORE::CString( "mail_guide_" ) + cases[ c ].name + "_" + sizes[ s ].name );
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
