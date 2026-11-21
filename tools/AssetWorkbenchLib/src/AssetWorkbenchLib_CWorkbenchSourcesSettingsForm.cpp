/*
 *  AssetWorkbenchLib: the settings page Sources
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

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#include "gucefASSETSTORE_CAssetStoreRegistry.h"
#define GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREREGISTRY_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#include "gucefASSETSTORE_CIAssetStoreProbe.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROBE_H ? */

#include "AssetWorkbenchLib_CWorkbenchSourcesSettingsForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CWorkbenchSourcesSettingsForm::PluginsLoadRequestedEvent = "GUCEF::ASSETWORKBENCH::CWorkbenchSourcesSettingsForm::PluginsLoadRequestedEvent";
const CORE::CString CWorkbenchSourcesSettingsForm::PageTitle = "Sources";

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

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::RegisterEvents( void )
{GUCEF_TRACE;

    PluginsLoadRequestedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CWorkbenchSourcesSettingsForm::CWorkbenchSourcesSettingsForm( void )
    : GUI::CSettingsPageForm()
    , m_preferences( GUCEF_NULL )
    , m_signIn( GUCEF_NULL )
    , m_pending()
    , m_report()
    , m_serviceNote()
    , m_storeIds()
    , m_selectedStore()
    , m_probeStore()
    , m_probeText()
    , m_storeStatus()
    , m_probeFuture( CORE::TASKSTATUS_UNDEFINED )
    , m_probeRunning( false )
    , m_subscribedToKeyring( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CWorkbenchSourcesSettingsForm::~CWorkbenchSourcesSettingsForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::SetPreferences( CWorkbenchPreferences* preferences )
{GUCEF_TRACE;

    m_preferences = preferences;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::SetSignIn( CWorkbenchSignIn* signIn )
{GUCEF_TRACE;

    m_signIn = signIn;
    CWorkbenchCredentials* credentials = GUCEF_NULL != signIn ? signIn->GetCredentials() : GUCEF_NULL;
    if ( GUCEF_NULL != credentials && !m_subscribedToKeyring )
    {
        m_subscribedToKeyring = true;
        AUTH::CKeyringSession& session = credentials->GetKeyringSession();
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnKeyringStateChanged );
        SubscribeTo( &session, AUTH::CKeyringSession::UnlockedEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::LockedEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeSetUpEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::CodeChangedEvent, callback );
        SubscribeTo( &session, AUTH::CKeyringSession::ForgottenEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::SetPluginReport( const ASSETSTORE::CAssetStorePluginLoadReport& report )
{GUCEF_TRACE;

    m_report = report;
    ShowPluginsStatus();
    FillStores();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::SetServiceNote( const CORE::CString& note )
{GUCEF_TRACE;

    m_serviceNote = note;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetPluginsDirFromWidgets( void )
{GUCEF_TRACE;

    return GetWidgetProperty( "PluginsDir", GUI::EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetNoStoreText( void )
{GUCEF_TRACE;

    return "No online store is installed. The store plugins are not part of the workbench: build them, set the folder that holds them above and press Load now (or restart the workbench).";
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchSourcesSettingsForm::GetStoreCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_storeIds.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetStoreId( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_storeIds.size() ? m_storeIds[ index ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetStoreLine( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_storeIds.size() ? MakeStoreLine( m_storeIds[ index ] ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetSelectedStoreId( void ) const
{GUCEF_TRACE;

    return m_selectedStore;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSourcesSettingsForm::SelectStore( const CORE::CString& storeId )
{GUCEF_TRACE;

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Stores" );
    for ( UInt32 i=0; i<(UInt32) m_storeIds.size(); ++i )
    {
        if ( m_storeIds[ i ] == storeId )
        {
            m_selectedStore = storeId;
            if ( GUCEF_NULL != list )
                list->SetSelectedIndex( (Int32) i );
            ShowStoreDetails();
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSourcesSettingsForm::FindProvider( const CORE::CString& storeId, ASSETSTORE::TAssetStoreProviderPtr& provider ) const
{GUCEF_TRACE;

    return ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().TryGetProvider( storeId, provider ) && !provider.IsNULL();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetSignInMethodText( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return "unknown";
    switch ( provider->GetSignInKind() )
    {
        case ASSETSTORE::ASSETSTORE_SIGNIN_AUTH_PROVIDER:   return "an account of " + provider->GetAuthProviderName();
        case ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION:     return "the web site of the store, in a browser window";
        default:                                            return "none needed";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetKeyringText( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) || ASSETSTORE::ASSETSTORE_SIGNIN_WEB_SESSION != provider->GetSignInKind() )
        return "not used";
    if ( GUCEF_NULL == m_signIn )
        return "not ready";
    switch ( m_signIn->GetKeyringStatus() )
    {
        case AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE:    return "unavailable in this build";
        case AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP:     return "not set up";
        case AUTHGUI::CSignInSettingsState::KEYRING_LOCKED:         return "locked";
        default:                                                    return "unlocked";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::MakeStoreLine( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( !FindProvider( storeId, provider ) )
        return storeId;
    const bool enabled = m_pending.IsStoreEnabled( storeId );
    return provider->GetDisplayName() + "    " + ( !enabled ? CORE::CString( "off" ) : ( provider->IsSignedIn() ? CORE::CString( "signed in" ) : CORE::CString( "not signed in" ) ) );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "SourcesSettings", "0,0 0,0", "1,0 1,0" );

    // the folder of the plugins, with what loading it did
    CORE::CDataNode* plugins = AddGroup( *page, "PluginsGroup", "Folder with the store plugins", "0,16 0,6", "1,-32 0,100" );
    GUI::CFormLayout::AddWidgetNode( *plugins, "Editbox", "PluginsDir", "0,10 0,28", "1,-148 0,30" );
    CORE::CDataNode* load = GUI::CFormLayout::AddWidgetNode( *plugins, "Button", "LoadPlugins", "1,-130 0,28", "0,120 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *load, GUI::EWidgetProperty_Text, "Load now" );
    CORE::CDataNode* pluginsStatus = GUI::CFormLayout::AddWidgetNode( *plugins, "Label", "PluginsStatus", "0,10 0,62", "1,-20 0,36" );
    GUI::CFormLayout::SetWidgetNodeProperty( *pluginsStatus, GUI::EWidgetProperty_WordWrap, "true" );

    // the first run: what there is to do about the keyring
    CORE::CDataNode* keyring = AddGroup( *page, "KeyringGroup", "Keyring of the saved passwords", "0,16 0,112", "1,-32 0,76" );
    CORE::CDataNode* guide = GUI::CFormLayout::AddWidgetNode( *keyring, "Label", "KeyringGuide", "0,10 0,28", "1,-250 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *guide, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* keyringAction = GUI::CFormLayout::AddWidgetNode( *keyring, "Button", "KeyringAction", "1,-232 0,32", "0,222 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *keyringAction, GUI::EWidgetProperty_Text, "Set up the keyring..." );

    // the stores: the list on the left, what is known of the selected one on the right, the result of its test below
    CORE::CDataNode* stores = AddGroup( *page, "StoresGroup", "Online stores", "0,16 0,194", "1,-32 0,236" );
    GUI::CFormLayout::AddWidgetNode( *stores, "Listbox", "Stores", "0,10 0,28", "0.42,-10 0,84" );
    CORE::CDataNode* empty = GUI::CFormLayout::AddWidgetNode( *stores, "Label", "EmptyState", "0,16 0,32", "0.42,-22 0,76" );
    GUI::CFormLayout::SetWidgetNodeProperty( *empty, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *empty, GUI::EWidgetProperty_InputTransparent, "true" );
    CORE::CDataNode* enabled = GUI::CFormLayout::AddWidgetNode( *stores, "Checkbox", "StoreEnabled", "0.42,10 0,28", "0.58,-20 0,26" );
    GUI::CFormLayout::SetWidgetNodeProperty( *enabled, GUI::EWidgetProperty_Text, "Use this store" );
    CORE::CDataNode* info = GUI::CFormLayout::AddWidgetNode( *stores, "Label", "StoreInfo", "0.42,10 0,58", "0.58,-20 0,44" );
    GUI::CFormLayout::SetWidgetNodeProperty( *info, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* signIn = GUI::CFormLayout::AddWidgetNode( *stores, "Button", "StoreSignIn", "0.42,10 0,106", "0.18,-14 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *signIn, GUI::EWidgetProperty_Text, "Sign in..." );
    CORE::CDataNode* test = GUI::CFormLayout::AddWidgetNode( *stores, "Button", "TestConnection", "0.6,4 0,106", "0.4,-14 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *test, GUI::EWidgetProperty_Text, "Test connection" );
    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *stores, "Label", "StoreStatus", "0,10 0,118", "0.42,-10 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* probe = GUI::CFormLayout::AddWidgetNode( *stores, "Textbox", "ProbeResult", "0,10 0,146", "1,-20 0,80" );
    GUI::CFormLayout::SetWidgetNodeProperty( *probe, GUI::EWidgetProperty_WordWrap, "true" );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CSettingsPageForm::OnPostLayoutLoad();

    GUI::CWidget* widget = GetWidget( "LoadPlugins" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnLoadPluginsActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "KeyringAction" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnKeyringActionActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "StoreSignIn" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnStoreSignInActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "TestConnection" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnTestConnectionActivated );
        SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
    }
    widget = GetWidget( "Stores" );
    if ( GUCEF_NULL != widget )
    {
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnStoresSelectionChanged );
        SubscribeTo( widget, GUI::CListbox::SelectionChangedEvent, callback );
    }
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback callback( this, &CWorkbenchSourcesSettingsForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
    ASSETSTORE::CAssetStoreGlobal* stores = ASSETSTORE::CAssetStoreGlobal::Instance();
    TEventCallback providerCallback( this, &CWorkbenchSourcesSettingsForm::OnProviderChanged );
    SubscribeTo( &stores->GetRegistry(), ASSETSTORE::CAssetStoreRegistry::ProviderAddedEvent, providerCallback );
    SubscribeTo( &stores->GetRegistry(), ASSETSTORE::CAssetStoreRegistry::ProviderRemovedEvent, providerCallback );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::FillStores( void )
{GUCEF_TRACE;

    m_storeIds.clear();
    CORE::CString::StringVector ids;
    ASSETSTORE::CAssetStoreGlobal::Instance()->GetRegistry().GetProviderIds( ids );
    for ( size_t i=0; i<ids.size(); ++i )
        if ( ids[ i ] != "primary" )
            m_storeIds.push_back( ids[ i ] );
    bool selectedIsThere = false;
    for ( size_t i=0; i<m_storeIds.size(); ++i )
        selectedIsThere = selectedIsThere || m_storeIds[ i ] == m_selectedStore;
    if ( !selectedIsThere )
        m_selectedStore = m_storeIds.empty() ? CORE::CString::Empty : m_storeIds[ 0 ];

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Stores" );
    if ( GUCEF_NULL != list )
    {
        CReadingSettingsScope readingSettings( *this );
        GUI::CListbox::TStringVector lines;
        Int32 selected = 0;
        for ( size_t i=0; i<m_storeIds.size(); ++i )
        {
            lines.push_back( MakeStoreLine( m_storeIds[ i ] ) );
            if ( m_storeIds[ i ] == m_selectedStore )
                selected = (Int32) i;
        }
        list->SetItems( lines );
        if ( !lines.empty() )
            list->SetSelectedIndex( selected );
    }
    ShowStoreDetails();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetStoreInfoText( void )
{GUCEF_TRACE;

    if ( m_selectedStore.IsNULLOrEmpty() )
        return CORE::CString::Empty;
    return "Sign in with: " + GetSignInMethodText( m_selectedStore ) + "\nKeyring: " + GetKeyringText( m_selectedStore );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetStoreStatusText( void )
{GUCEF_TRACE;

    return m_storeStatus;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetProbeText( void )
{GUCEF_TRACE;

    return m_probeText;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetEmptyStateText( void )
{GUCEF_TRACE;

    return m_storeIds.empty() ? GetNoStoreText() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetKeyringGuideText( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_signIn )
        return "The keyring is ready once the sources of the workbench are made.";
    switch ( m_signIn->GetKeyringStatus() )
    {
        case AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE:
            return "This build cannot keep saved passwords (the encryption plugin is not loaded); you sign in with the browser window.";
        case AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP:
            return "First run: set a code up for the keyring to let the workbench save the passwords of the stores. Without it you sign in with the browser window each time.";
        case AUTHGUI::CSignInSettingsState::KEYRING_LOCKED:
            return "The keyring is locked: unlock it with your code to sign in with the saved passwords.";
        default:
            return "The keyring is unlocked.";
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetPluginsStatusText( void )
{GUCEF_TRACE;

    CORE::CString::StringVector lines;
    m_report.GetLines( lines );
    CORE::CString text;
    for ( size_t i=0; i<lines.size() && i<3; ++i )
        text += ( i > 0 ? CORE::CString( "\n" ) : CORE::CString::Empty ) + lines[ i ];
    if ( lines.size() > 3 )
        text += "\n... " + CORE::ToString( (UInt32) lines.size() - 3 ) + " more lines in the log";
    return text;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::ShowPluginsStatus( void )
{GUCEF_TRACE;

    SetWidgetProperty( "PluginsStatus", GUI::EWidgetProperty_Text, GetPluginsStatusText() );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::ShowKeyring( void )
{GUCEF_TRACE;

    SetWidgetProperty( "KeyringGuide", GUI::EWidgetProperty_Text, GetKeyringGuideText() );
    const AUTHGUI::CSignInSettingsState::EKeyringState state = GUCEF_NULL != m_signIn ? m_signIn->GetKeyringStatus() : AUTHGUI::CSignInSettingsState::KEYRING_UNAVAILABLE;
    const bool action = AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP == state || AUTHGUI::CSignInSettingsState::KEYRING_LOCKED == state;
    SetWidgetProperty( "KeyringAction", GUI::EWidgetProperty_Visible, CORE::BoolToString( action ) );
    SetWidgetProperty( "KeyringAction", GUI::EWidgetProperty_Text, AUTHGUI::CSignInSettingsState::KEYRING_NOT_SET_UP == state ? "Set up the keyring..." : "Unlock the keyring..." );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::ShowStoreDetails( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;
    const bool any = !m_selectedStore.IsNULLOrEmpty();
    ASSETSTORE::TAssetStoreProviderPtr provider;
    const bool found = any && FindProvider( m_selectedStore, provider );
    {
        CReadingSettingsScope readingSettings( *this );
        SetWidgetProperty( "StoreEnabled", GUI::EWidgetProperty_Checked, CORE::BoolToString( any && m_pending.IsStoreEnabled( m_selectedStore ) ) );
    }
    SetWidgetProperty( "StoreEnabled", GUI::EWidgetProperty_Enabled, CORE::BoolToString( found ) );
    SetWidgetProperty( "StoreInfo", GUI::EWidgetProperty_Text, GetStoreInfoText() );
    const bool needsSignIn = found && ASSETSTORE::ASSETSTORE_SIGNIN_NONE != provider->GetSignInKind();
    SetWidgetProperty( "StoreSignIn", GUI::EWidgetProperty_Enabled, CORE::BoolToString( needsSignIn && GUCEF_NULL != m_signIn ) );
    const bool probe = found && GUCEF_NULL != provider->GetProbe();
    SetWidgetProperty( "TestConnection", GUI::EWidgetProperty_Enabled, CORE::BoolToString( probe && !m_probeRunning ) );
    if ( !m_probeRunning )
    {
        // the status of a connection test of this store stays until another store is selected
        const bool probeShown = !m_probeStore.IsNULLOrEmpty() && m_probeStore == m_selectedStore && !m_probeText.IsNULLOrEmpty();
        if ( !found )
            m_storeStatus.Clear();
        else
        if ( !probeShown )
        {
            if ( !m_pending.IsStoreEnabled( m_selectedStore ) )
                m_storeStatus = "The store is switched off.";
            else
            if ( ASSETSTORE::ASSETSTORE_SIGNIN_NONE == provider->GetSignInKind() )
                m_storeStatus = "No sign in needed.";
            else
                m_storeStatus = provider->IsSignedIn() ? CORE::CString( "Signed in." ) : CORE::CString( "Not signed in." );
            if ( !probe )
                m_storeStatus += " This store has no connection test.";
        }
        if ( m_probeStore != m_selectedStore )
            m_probeText.Clear();
    }
    SetWidgetProperty( "StoreStatus", GUI::EWidgetProperty_Text, m_storeStatus );
    SetWidgetProperty( "ProbeResult", GUI::EWidgetProperty_Text, m_probeText );
    // the state of a service in another process is what there is to tell; without one and without a store, what to do
    const CORE::CString empty = m_serviceNote.IsNULLOrEmpty() ? GetEmptyStateText() : m_serviceNote;
    SetWidgetProperty( "EmptyState", GUI::EWidgetProperty_Text, empty );
    SetWidgetProperty( "EmptyState", GUI::EWidgetProperty_Visible, CORE::BoolToString( !empty.IsNULLOrEmpty() ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchSourcesSettingsForm::GetTitle( void ) const
{GUCEF_TRACE;

    return PageTitle;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::ReadSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_preferences )
        m_pending = *m_preferences;
    {
        CReadingSettingsScope readingSettings( *this );
        SetWidgetProperty( "PluginsDir", GUI::EWidgetProperty_Text, m_pending.privatePluginsDir );
    }
    ShowPluginsStatus();
    ShowKeyring();
    FillStores();
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSourcesSettingsForm::ApplySettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_preferences )
        return true;
    m_pending.privatePluginsDir = GetPluginsDirFromWidgets();
    m_preferences->privatePluginsDir = m_pending.privatePluginsDir;
    m_preferences->disabledStores = m_pending.disabledStores;
    SetHasChanges( false );
    NotifyObservers( SettingsAppliedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSourcesSettingsForm::IsStoreEnabled( const CORE::CString& storeId ) const
{GUCEF_TRACE;

    return m_pending.IsStoreEnabled( storeId );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnValueChanged( GUI::CWidget& widget )
{GUCEF_TRACE;

    if ( "StoreEnabled" == widget.GetName() && !m_selectedStore.IsNULLOrEmpty() )
    {
        m_pending.SetStoreEnabled( m_selectedStore, CORE::StringToBool( widget.GetProperty( GUI::EWidgetProperty_Checked ), true ) );
        FillStores();
    }
    GUI::CSettingsPageForm::OnValueChanged( widget );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSourcesSettingsForm::StartProbe( void )
{GUCEF_TRACE;

    ASSETSTORE::TAssetStoreProviderPtr provider;
    if ( m_probeRunning || m_selectedStore.IsNULLOrEmpty() || !FindProvider( m_selectedStore, provider ) || GUCEF_NULL == provider->GetProbe() )
        return false;
    m_probeFuture = provider->RunProbeAsync();
    m_probeRunning = true;
    m_probeStore = m_selectedStore;
    m_probeText.Clear();
    m_storeStatus = "Testing the connection...";
    ShowStoreDetails();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchSourcesSettingsForm::IsProbeRunning( void ) const
{GUCEF_TRACE;

    return m_probeRunning;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::CheckProbe( void )
{GUCEF_TRACE;

    if ( !m_probeRunning || !m_probeFuture.IsReady() )
        return;
    m_probeRunning = false;
    CORE::CTaskPtr task = m_probeFuture.GetResult( 0 );
    m_probeFuture = CORE::CFutureResult( CORE::TASKSTATUS_UNDEFINED );
    const ASSETSTORE::CAssetStoreProbeResult* result = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetStoreProbeResult >( task->GetTaskData() );
    if ( GUCEF_NULL != result && CORE::TASKSTATUS_FINISHED == task->GetTaskStatus() )
    {
        CORE::CString::StringVector lines;
        result->GetLines( lines );
        m_probeText = CORE::CString().Combine( lines, '\n' );
        m_storeStatus = "Connection test: " + CORE::ToString( result->GetPassedCount() ) + " of " + CORE::ToString( (UInt32) result->checks.size() ) + " checks ok.";
    }
    else
    {
        // a probe that could not run: the status of the task says why in words of the store module, no value of the store
        m_probeText = task.IsNULL() ? CORE::CString( "The test could not be started." ) : task->GetTaskStatusExtraInfo();
        m_storeStatus = "The connection test did not run.";
    }
    ShowStoreDetails();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnLoadPluginsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( PluginsLoadRequestedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnKeyringActionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_signIn )
        m_signIn->OpenKeyring();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnStoreSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_signIn && !m_selectedStore.IsNULLOrEmpty() )
        m_signIn->SignIn( m_selectedStore );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnTestConnectionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    StartProbe();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnStoresSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Stores" );
    const Int32 selected = GUCEF_NULL != list ? list->GetSelectedIndex() : -1;
    if ( selected >= 0 && selected < (Int32) m_storeIds.size() && m_storeIds[ (UInt32) selected ] != m_selectedStore )
    {
        m_selectedStore = m_storeIds[ (UInt32) selected ];
        ShowStoreDetails();
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CheckProbe();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnKeyringStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ShowKeyring();
    ShowStoreDetails();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchSourcesSettingsForm::OnProviderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the stores come and go on the thread of the pulse generator; the page is made again on the next frame of the GUI
    FillStores();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchSourcesSettingsForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchSourcesSettingsForm";
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
