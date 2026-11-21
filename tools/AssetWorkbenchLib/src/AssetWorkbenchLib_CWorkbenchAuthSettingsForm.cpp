/*
 *  AssetWorkbenchLib: the settings page Authentication
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

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#include "AssetWorkbenchLib_CWorkbenchAuthSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A group on a page: a bordered panel with its caption, whose children are placed by position below the caption (the caption has 26 pixels) */
CORE::CDataNode*
AddGroup( CORE::CDataNode& parent, const char* name, const char* caption, const char* position, const char* size )
{GUCEF_TRACE;

    CORE::CDataNode* group = GUI::CFormLayout::AddWidgetNode( parent, "Panel", name, position, size );
    GUI::CFormLayout::SetWidgetNodeProperty( *group, GUI::EWidgetProperty_BackgroundColour, "0,0,0,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *group, GUI::EWidgetProperty_BorderColour, "74,80,96,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *group, GUI::EWidgetProperty_BorderWidth, "1" );
    CORE::CDataNode* title = GUI::CFormLayout::AddWidgetNode( *group, "Label", CORE::CString( name ) + "Caption", "0,10 0,3", "1,-20 0,22" );
    GUI::CFormLayout::SetWidgetNodeProperty( *title, GUI::EWidgetProperty_Text, caption );
    GUI::CFormLayout::SetWidgetNodeProperty( *title, GUI::EWidgetProperty_TextColour, "176,182,196,255" );
    return group;
}

}

const CORE::CString CWorkbenchAuthSettingsForm::PageTitle = "Authentication";

/*-------------------------------------------------------------------------*/

CWorkbenchAuthSettingsForm::CWorkbenchAuthSettingsForm( void )
    : GUI::CSettingsPageForm()
    , m_signIn( GUCEF_NULL )
    , m_storeIds()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchAuthSettingsForm::~CWorkbenchAuthSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::SetSignIn( CWorkbenchSignIn* signIn )
{GUCEF_TRACE;

    if ( m_signIn == signIn )
        return;
    m_signIn = signIn;
    CWorkbenchCredentials* credentials = GUCEF_NULL != signIn ? signIn->GetCredentials() : GUCEF_NULL;
    if ( GUCEF_NULL != credentials )
    {
        // the state follows the keyring while the page is open
        AUTH::CKeyringSession& session = credentials->GetKeyringSession();
        TEventCallback callback( this, &CWorkbenchAuthSettingsForm::OnKeyringStateChanged );
        SubscribeTo( &session, AUTH::CKeyringSession::UnlockedEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::LockedEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeSetUpEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeChangedEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::ForgottenEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchAuthSettingsForm::GetKeyringStateText( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_signIn )
        return "The sign in is not ready yet: the sources of the workbench are not made.";
    switch ( m_signIn->GetKeyringStatus() )
    {
        case AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE:
            return "This build cannot keep saved passwords: the encryption plugin is not loaded. Signing in with the browser window still works.";
        case AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP:
            return "There is no keyring yet. Set a code up to let the workbench save passwords of the online stores; without it you sign in with the browser window each time.";
        case AUTHGUI::CSignInSettingsState::KEYRING_LOCKED:
            return "The keyring is locked. Unlock it with your code to use the saved passwords.";
        default:
            return "The keyring is unlocked. It locks again after a while without use, or when you lock it.";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchAuthSettingsForm::GetKeyringActionText( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_signIn )
        return CORE::CString::Empty;
    switch ( m_signIn->GetKeyringStatus() )
    {
        case AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP:     return "Set up the keyring...";
        case AUTHGUI::CSignInSettingsState::KEYRING_LOCKED:         return "Unlock the keyring...";
        default:                                                    return CORE::CString::Empty;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchAuthSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return PageTitle;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "AuthSettings", "0,0 0,0", "1,0 1,0" );

    CORE::CDataNode* keyring = AddGroup( *page, "KeyringGroup", "Keyring of the saved passwords", "0,16 0,6", "1,-32 0,160" );
    CORE::CDataNode* state = GUI::CFormLayout::AddWidgetNode( *keyring, "Label", "KeyringState", "0,10 0,28", "1,-20 0,66" );
    GUI::CFormLayout::SetWidgetNodeProperty( *state, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* action = GUI::CFormLayout::AddWidgetNode( *keyring, "Button", "KeyringAction", "0,10 0,100", "0,220 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *action, GUI::EWidgetProperty_Text, "Set up the keyring..." );
    CORE::CDataNode* lock = GUI::CFormLayout::AddWidgetNode( *keyring, "Button", "LockKeyring", "0,240 0,100", "0,130 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *lock, GUI::EWidgetProperty_Text, "Lock now" );
    CORE::CDataNode* change = GUI::CFormLayout::AddWidgetNode( *keyring, "Button", "ChangeCode", "0,380 0,100", "0,190 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *change, GUI::EWidgetProperty_Text, "Change the code..." );

    CORE::CDataNode* store = AddGroup( *page, "StoreGroup", "Sign in settings of an online store", "0,16 0,176", "1,-32 0,84" );
    GUI::CFormLayout::AddWidgetNode( *store, "Combobox", "StoreChoice", "0,10 0,30", "0,320 0,32" );
    CORE::CDataNode* storeButton = GUI::CFormLayout::AddWidgetNode( *store, "Button", "StoreSettings", "0,340 0,30", "0,220 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *storeButton, GUI::EWidgetProperty_Text, "Sign in settings..." );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CSettingsPageForm::OnPostLayoutLoad();

    GUI::CWidget* widget = GetWidget( "KeyringAction" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchAuthSettingsForm::OnKeyringActionActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "LockKeyring" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchAuthSettingsForm::OnLockActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "ChangeCode" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchAuthSettingsForm::OnChangeCodeActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "StoreSettings" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchAuthSettingsForm::OnStoreSettingsActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::ShowState( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;
    SetWidgetProperty( "KeyringState", GUI::EWidgetProperty_Text, GetKeyringStateText() );
    const AUTHGUI::CSignInSettingsState::EKeyringState state = GUCEF_NULL != m_signIn ? m_signIn->GetKeyringStatus() : AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE;
    const CORE::CString actionText = GetKeyringActionText();
    SetWidgetProperty( "KeyringAction", GUI::EWidgetProperty_Visible, CORE::BoolToString( !actionText.IsNULLOrEmpty() ) );
    if ( !actionText.IsNULLOrEmpty() )
        SetWidgetProperty( "KeyringAction", GUI::EWidgetProperty_Text, actionText );
    SetWidgetProperty( "LockKeyring", GUI::EWidgetProperty_Enabled, CORE::BoolToString( AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED == state ) );
    SetWidgetProperty( "ChangeCode", GUI::EWidgetProperty_Enabled, CORE::BoolToString( AUTHGUI::CSignInSettingsState::KEYRING_UNLOCKED == state ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    // the stores that sign in: the online stores of the registry
    m_storeIds.clear();
    CORE::CString::StringVector ids;
    ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().GetProviderIds( ids );
    GUI::CCombobox::TStringVector names;
    for ( size_t i=0; i<ids.size(); ++i )
    {
        ASSETSTORE::TAssetStoreProviderPtr provider;
        if ( ids[ i ] == "primary" || !ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( ids[ i ], provider ) )
            continue;
        m_storeIds.push_back( ids[ i ] );
        names.push_back( provider->GetDisplayName() );
    }
    GUI::CCombobox* stores = GetWidgetAs< GUI::CCombobox >( "StoreChoice" );
    if ( GUCEF_NULL != stores )
    {
        CReadingSettingsScope readingSettings( *this );
        stores->SetListItems( names );
        if ( !names.empty() )
            stores->SetSelectedIndex( 0 );
    }
    SetWidgetProperty( "StoreSettings", GUI::EWidgetProperty_Enabled, CORE::BoolToString( !names.empty() && GUCEF_NULL != m_signIn ) );
    ShowState();
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchAuthSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    // nothing is kept here: the keyring and the sign in settings act at once
    SetHasChanges( false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::OnKeyringActionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_signIn )
        m_signIn->OpenKeyring();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::OnLockActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_signIn )
        m_signIn->LockKeyring();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::OnChangeCodeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_signIn )
        m_signIn->OpenKeyringChangeCode();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::OnStoreSettingsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CCombobox* stores = GetWidgetAs< GUI::CCombobox >( "StoreChoice" );
    const Int32 selected = GUCEF_NULL != stores ? stores->GetSelectedIndex() : -1;
    if ( GUCEF_NULL != m_signIn && selected >= 0 && selected < (Int32) m_storeIds.size() )
        m_signIn->OpenSettings( m_storeIds[ (UInt32) selected ] );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchAuthSettingsForm::OnKeyringStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowState();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchAuthSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchAuthSettingsForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
