/*
 *  gucefGAMEGUI_TestApp: Application for testing gucefGAMEGUI
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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGAMEGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** An identity provider that keeps the identity in memory */
class CTestIdentityProvider : public MULTIPLAY::CIIdentityProvider
{
    public:

    MULTIPLAY::CPlayerIdentity identity;
    bool refuseNames;
    UInt32 renames;

    CTestIdentityProvider( void )
        : MULTIPLAY::CIIdentityProvider(), identity(), refuseNames( false ), renames( 0 )
    {
        identity.userId = "5f2b9a3e-1111-4222-8333-444455556666";
        identity.displayName = "Old name";
    }

    virtual bool GetIdentity( MULTIPLAY::CPlayerIdentity& result ) GUCEF_VIRTUAL_OVERRIDE { result = identity; return true; }

    virtual bool SetDisplayName( const CORE::CString& displayName ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( refuseNames )
            return false;
        identity.displayName = displayName;
        ++renames;
        return true;
    }

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString providerName = "test-identity";
        return providerName;
    }

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestIdentityProvider";
        return classTypeName;
    }
};

/*-------------------------------------------------------------------------*/

/** Records what the profile form tells */
class CProfileRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CProfileRecorder > TEventCallback;

    UInt32 changes;
    UInt32 cancels;
    MULTIPLAY::CPlayerIdentity last;

    CProfileRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), changes( 0 ), cancels( 0 ), last()
    {
        TEventCallback changedCallback( this, &CProfileRecorder::OnProfileChanged );
        SubscribeTo( form, GAMEGUI::CPlayerProfileForm::ProfileChangedEvent, changedCallback );
        TEventCallback cancelCallback( this, &CProfileRecorder::OnCancelled );
        SubscribeTo( form, GAMEGUI::CPlayerProfileForm::CancelledEvent, cancelCallback );
    }

    virtual ~CProfileRecorder() { UnsubscribeAllFromObserver(); }

    void OnProfileChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++changes;
            last = *static_cast< MULTIPLAY::CPlayerIdentity* >( eventData );
        }
    }

    void OnCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++cancels;
    }
};

/*-------------------------------------------------------------------------*/

void
PerformPlayerProfileFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.PlayerProfile" );

    GUCEF_TESTFW_TESTCASE( "PlayerProfile: The name is changed and saved with the identity provider, the id is shown and read only" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CTestIdentityProvider provider;
        GAMEGUI::CPlayerProfileForm form( context );
        CProfileRecorder recorder( &form );
        form.SetIdentityProvider( &provider );
        ASSERT_TRUE( &provider == form.GetIdentityProvider() && "Old name" == form.GetIdentity().displayName );
        ASSERT_TRUE( form.Open() && form.IsOpen() && form.IsVisible() && context->IsModalRoot( form.GetRootWidget() ) );
        ASSERT_TRUE( context->GetFocus() == form.GetWidget( "DisplayName" ) );

        // the name and the id
        ASSERT_TRUE( "Old name" == form.GetDisplayNameText() && "5f2b9a3e-1111-4222-8333-444455556666" == form.GetWidget( "UserId" )->GetProperty( "Text" ) );
        ASSERT_TRUE( form.GetWidget( "UserId" )->GetBoolProperty( "ReadOnly" ) && !form.GetWidget( "DisplayName" )->GetBoolProperty( "ReadOnly" ) );
        ASSERT_TRUE( GAMEGUI::CPlayerProfileForm::MaxDisplayNameLength == (UInt32) form.GetWidget( "DisplayName" )->GetIntProperty( "MaxLength" ) );

        // no name is no profile
        form.SetDisplayNameText( "   " );
        ClickWidget( context, form.GetWidget( "Save" ) );
        ASSERT_TRUE( 0 == recorder.changes && form.IsOpen() && "Enter a name" == form.GetErrorText() && 0 == provider.renames );

        // a new name, with white space around it, saved
        TypeInto( context, form.GetWidget( "DisplayName" ), "  New name ", false );
        ClickWidget( context, form.GetWidget( "Save" ) );
        ASSERT_TRUE( 1 == recorder.changes && !form.IsOpen() && !form.IsVisible() && 1 == provider.renames );
        ASSERT_TRUE( "New name" == provider.identity.displayName && "New name" == recorder.last.displayName && provider.identity.userId == recorder.last.userId );
        ASSERT_TRUE( "New name" == form.GetIdentity().displayName );

        // the same name again changes nothing
        ASSERT_TRUE( form.Open() );
        TypeInto( context, form.GetWidget( "DisplayName" ), "New name", true );
        ASSERT_TRUE( 1 == recorder.changes && 1 == provider.renames && !form.IsOpen() );

        // Cancel puts the saved name back
        ASSERT_TRUE( form.Open() );
        form.SetDisplayNameText( "Half typed" );
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( 1 == recorder.cancels && "New name" == form.GetDisplayNameText() && !form.IsOpen() && 1 == recorder.changes );
        ASSERT_TRUE( form.Open() );
        PressKey( context, INPUT::KEYCODE_ESCAPE );
        ASSERT_TRUE( 2 == recorder.cancels && !form.IsVisible() );

        // the provider changes elsewhere: the form takes it up when it opens
        provider.identity.displayName = "Renamed elsewhere";
        ASSERT_TRUE( form.Open() && "Renamed elsewhere" == form.GetDisplayNameText() );
        form.Close();

        // a provider that does not take the name
        provider.refuseNames = true;
        ASSERT_TRUE( form.Open() );
        form.SetDisplayNameText( "Refused" );
        ASSERT_FALSE( form.Save() );
        ASSERT_TRUE( form.IsOpen() && "The name could not be saved" == form.GetErrorText() && 1 == recorder.changes );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlayerProfile: Without a provider the identity is the one given, and a provider that goes away is let go" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CPlayerProfileForm form( context );
        CProfileRecorder recorder( &form );
        MULTIPLAY::CPlayerIdentity identity;
        identity.userId = "guid-1";
        identity.displayName = "First";
        form.SetIdentity( identity );
        ASSERT_TRUE( form.Open() && "First" == form.GetDisplayNameText() && "guid-1" == form.GetWidget( "UserId" )->GetProperty( "Text" ) );
        form.SetDisplayNameText( "Second" );
        ASSERT_TRUE( form.Save() && 1 == recorder.changes && "Second" == recorder.last.displayName && "guid-1" == recorder.last.userId && "Second" == form.GetIdentity().displayName );

        {
            CTestIdentityProvider provider;
            form.SetIdentityProvider( &provider );
            ASSERT_TRUE( "Old name" == form.GetIdentity().displayName );
        }
        ASSERT_TRUE( GUCEF_NULL == form.GetIdentityProvider() );

        // a form that is not built yet takes the identity and shows it when it is
        GAMEGUI::CPlayerProfileForm lazy( context );
        lazy.SetIdentity( identity );
        ASSERT_TRUE( lazy.Open() && "First" == lazy.GetDisplayNameText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "PlayerProfile: The accounts button and line are there when asked, with the accounts of the identity, and the form only tells the host" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( 1242, 952, true );
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        MULTIPLAY::CPlayerIdentity identity;
        identity.userId = "guid-1";
        identity.displayName = "First";
        GAMEGUI::CPlayerProfileForm form( context );
        CEventCount requests( &form, GAMEGUI::CPlayerProfileForm::AccountsRequestedEvent );
        form.SetIdentity( identity );

        // as it was
        ASSERT_FALSE( form.GetAccountsEnabled() );
        ASSERT_TRUE( form.Open() && GUCEF_NULL == form.GetWidget( "Accounts" ) && GUCEF_NULL == form.GetWidget( "AccountsText" ) );
        ASSERT_TRUE( 300 == form.GetRootWidget()->GetRect().height && form.GetAccountsText().IsNULLOrEmpty() );
        form.Close();

        // with the accounts: the line says guest, the button tells the host
        form.SetAccountsEnabled( true );
        ASSERT_TRUE( form.GetAccountsEnabled() && form.Open() );
        context->FlushChanges();
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Accounts" ) && "Log in / accounts" == form.GetWidget( "Accounts" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 340 == form.GetRootWidget()->GetRect().height && "Guest" == form.GetAccountsText() );
        ClickWidget( context, form.GetWidget( "Accounts" ) );
        ASSERT_TRUE( 1 == requests.count && form.IsOpen() );
        form.Cancel();

        // linked accounts of the identity: the providers, once each
        identity.accounts.SetAllowMultipleValues( true );
        identity.accounts.Set( "google", "1234" );
        identity.accounts.Set( "steam", "7656" );
        form.SetIdentity( identity );
        ASSERT_TRUE( form.Open() );
        ASSERT_TRUE( "Accounts: Google, Steam" == form.GetAccountsText() );

        // the window keeps its controls apart
        form.SetDisplayNameText( "Second" );
        ASSERT_TRUE( form.Save() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
