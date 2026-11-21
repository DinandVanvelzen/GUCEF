/*
 *  AssetWorkbenchLib: the settings page Remote store: finding, pairing with and connecting to a remote asset store service
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

#ifndef GUCEF_GUI_CSECRETINPUT_H
#include "gucefGUI_CSecretInput.h"
#define GUCEF_GUI_CSECRETINPUT_H
#endif /* GUCEF_GUI_CSECRETINPUT_H ? */

#ifndef GUCEF_CORE_SECUREMEMORY_H
#include "gucefCORE_SecureMemory.h"
#define GUCEF_CORE_SECUREMEMORY_H
#endif /* GUCEF_CORE_SECUREMEMORY_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "AssetWorkbenchLib_CWorkbenchRemoteStoreForm.h"

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

const CORE::CString CWorkbenchRemoteStoreForm::PageTitle = "Remote store";

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

CWorkbenchRemoteStoreForm::CWorkbenchRemoteStoreForm( void )
    : GUI::CSettingsPageForm()
    , m_preferences( GUCEF_NULL )
    , m_connection( GUCEF_NULL )
    , m_shownState()
    , m_shownError()
    , m_shownInfo()
    , m_serviceLines()
    , m_shownStateNumber( 99 )
    , m_showing( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWorkbenchRemoteStoreForm::~CWorkbenchRemoteStoreForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUI::CSecretInput* code = GetWidgetAs< GUI::CSecretInput >( "PairingCode" );
    if ( GUCEF_NULL != code )
        code->ClearValue();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::SetPreferences( CWorkbenchPreferences* preferences )
{GUCEF_TRACE;

    m_preferences = preferences;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::SetConnection( CWorkbenchStoreConnection* connection )
{GUCEF_TRACE;

    if ( connection == m_connection )
        return;
    if ( GUCEF_NULL != m_connection )
        UnsubscribeFrom( m_connection );
    m_connection = connection;
    if ( GUCEF_NULL != m_connection )
    {
        TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnConnectionChanged );
        SubscribeTo( m_connection, CWorkbenchStoreConnection::StateChangedEvent, callback );
        SubscribeTo( m_connection, CWorkbenchStoreConnection::ServicesChangedEvent, callback );
    }
    ShowServices();
    ShowState();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchRemoteStoreForm::GetTitle( void ) const
{GUCEF_TRACE;

    return PageTitle;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* page = GUI::CFormLayout::AddWidgetNode( layout, "Panel", "RemoteStoreSettings", "0,0 0,0", "1,0 1,0" );

    // what the connection is doing, and what went wrong
    CORE::CDataNode* state = AddGroup( *page, "StateGroup", "Store in use", "0,16 0,6", "1,-32 0,96" );
    CORE::CDataNode* stateText = GUI::CFormLayout::AddWidgetNode( *state, "Label", "StateText", "0,10 0,26", "1,-20 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *stateText, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* errorText = GUI::CFormLayout::AddWidgetNode( *state, "Label", "ErrorText", "0,10 0,64", "1,-20 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *errorText, GUI::EWidgetProperty_WordWrap, "true" );
    GUI::CFormLayout::SetWidgetNodeProperty( *errorText, GUI::EWidgetProperty_TextColour, "232,140,140,255" );

    // the service: what was found, what it is, the address and the buttons
    CORE::CDataNode* service = AddGroup( *page, "ServiceGroup", "Store service", "0,16 0,110", "1,-32 0,198" );
    GUI::CFormLayout::AddWidgetNode( *service, "Listbox", "Services", "0,10 0,28", "0.5,-15 0,90" );
    CORE::CDataNode* info = GUI::CFormLayout::AddWidgetNode( *service, "Textbox", "ServerInfo", "0.5,5 0,28", "0.5,-15 0,90" );
    GUI::CFormLayout::SetWidgetNodeProperty( *info, GUI::EWidgetProperty_WordWrap, "true" );
    CORE::CDataNode* endpointLabel = GUI::CFormLayout::AddWidgetNode( *service, "Label", "EndpointLabel", "0,10 0,126", "0,76 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *endpointLabel, GUI::EWidgetProperty_Text, "Address" );
    GUI::CFormLayout::AddWidgetNode( *service, "Editbox", "Endpoint", "0,90 0,122", "1,-100 0,30" );
    CORE::CDataNode* connect = GUI::CFormLayout::AddWidgetNode( *service, "Button", "Connect", "0,10 0,160", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *connect, GUI::EWidgetProperty_Text, "Connect" );
    GUI::CFormLayout::SetWidgetNodeProperty( *connect, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* disconnect = GUI::CFormLayout::AddWidgetNode( *service, "Button", "Disconnect", "0,128 0,160", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *disconnect, GUI::EWidgetProperty_Text, "Disconnect" );
    CORE::CDataNode* find = GUI::CFormLayout::AddWidgetNode( *service, "Button", "FindAgain", "0,246 0,160", "0,110 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *find, GUI::EWidgetProperty_Text, "Find again" );
    CORE::CDataNode* lan = GUI::CFormLayout::AddWidgetNode( *service, "Checkbox", "LanDiscovery", "0,372 0,162", "1,-382 0,26" );
    GUI::CFormLayout::SetWidgetNodeProperty( *lan, GUI::EWidgetProperty_Text, "Look on the network" );

    // the pairing: the one-time code of the server
    CORE::CDataNode* pair = AddGroup( *page, "PairGroup", "Pair this computer", "0,16 0,316", "1,-32 0,84" );
    CORE::CDataNode* pairLabel = GUI::CFormLayout::AddWidgetNode( *pair, "Label", "PairLabel", "0,10 0,30", "0,100 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *pairLabel, GUI::EWidgetProperty_Text, "Pairing code" );
    GUI::CFormLayout::AddWidgetNode( *pair, "SecretInput", "PairingCode", "0,112 0,26", "1,-250 0,30" );
    CORE::CDataNode* pairButton = GUI::CFormLayout::AddWidgetNode( *pair, "Button", "Pair", "1,-130 0,26", "0,120 0,30" );
    GUI::CFormLayout::SetWidgetNodeProperty( *pairButton, GUI::EWidgetProperty_Text, "Pair" );
    CORE::CDataNode* hint = GUI::CFormLayout::AddWidgetNode( *pair, "Label", "PairHint", "0,10 0,58", "1,-20 0,22" );
    GUI::CFormLayout::SetWidgetNodeProperty( *hint, GUI::EWidgetProperty_Text, "Open pairing on the server, then type the code it shows. The code is not kept." );
    GUI::CFormLayout::SetWidgetNodeProperty( *hint, GUI::EWidgetProperty_TextColour, "176,182,196,255" );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CSettingsPageForm::OnPostLayoutLoad();

    const char* const buttons[] = { "FindAgain", "Connect", "Disconnect", "Pair" };
    for ( UInt32 i=0; i<4; ++i )
    {
        GUI::CWidget* widget = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL == widget )
            continue;
        if ( 0 == i )
        {
            TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnFindActivated );
            SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
        }
        else
        if ( 1 == i )
        {
            TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnConnectActivated );
            SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
        }
        else
        if ( 2 == i )
        {
            TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnDisconnectActivated );
            SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
        }
        else
        {
            TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnPairActivated );
            SubscribeTo( widget, GUI::CWidget::ActivatedEvent, callback );
        }
    }
    GUI::CWidget* services = GetWidget( "Services" );
    if ( GUCEF_NULL != services )
    {
        TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnServicesSelectionChanged );
        SubscribeTo( services, GUI::CListbox::SelectionChangedEvent, callback );
    }
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context )
    {
        TEventCallback callback( this, &CWorkbenchRemoteStoreForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::SetTextIfChanged( const char* widgetName, const CORE::CString& text, CORE::CString& cache )
{GUCEF_TRACE;

    if ( text == cache )
        return;
    cache = text;
    SetWidgetProperty( widgetName, GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::ShowState( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() || m_showing )
        return;
    m_showing = true;
    CORE::CString state, error, info;
    UInt32 number = (UInt32) CWorkbenchStoreConnection::STATE_DISCONNECTED;
    bool paired = false;
    bool busy = false;
    if ( GUCEF_NULL == m_connection )
        state = "The connection to a remote store is not ready: the sources of the workbench are not made yet.";
    else
    {
        state = m_connection->GetStateText();
        error = m_connection->GetErrorText();
        info = m_connection->IsConnected() ? m_connection->GetServerSummary() : CORE::CString( "Not connected." );
        number = (UInt32) m_connection->GetState();
        paired = m_connection->IsPaired();
        busy = m_connection->IsBusy();
    }
    SetTextIfChanged( "StateText", state, m_shownState );
    SetTextIfChanged( "ErrorText", error, m_shownError );
    SetTextIfChanged( "ServerInfo", info, m_shownInfo );
    if ( number != m_shownStateNumber )
    {
        m_shownStateNumber = number;
        const bool connected = GUCEF_NULL != m_connection && m_connection->IsConnected();
        const bool connecting = CWorkbenchStoreConnection::STATE_CONNECTING == number;
        SetWidgetProperty( "Connect", GUI::EWidgetProperty_Enabled, CORE::BoolToString( GUCEF_NULL != m_connection && !connected && !connecting && CWorkbenchStoreConnection::STATE_PAIRING != number ) );
        SetWidgetProperty( "Disconnect", GUI::EWidgetProperty_Enabled, CORE::BoolToString( connected || connecting ) );
        SetWidgetProperty( "Pair", GUI::EWidgetProperty_Enabled, CORE::BoolToString( GUCEF_NULL != m_connection && CWorkbenchStoreConnection::STATE_PAIRING != number ) );
        SetWidgetProperty( "FindAgain", GUI::EWidgetProperty_Enabled, CORE::BoolToString( GUCEF_NULL != m_connection && CWorkbenchStoreConnection::STATE_DISCOVERING != number ) );
        SetWidgetProperty( "PairHint", GUI::EWidgetProperty_Text, paired ? CORE::CString( "This computer is paired. Pair again only when the server asks for it. The code is not kept." ) :
                                                                         CORE::CString( "Open pairing on the server, then type the code it shows. The code is not kept." ) );
    }
    (void) busy;
    m_showing = false;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::ShowServices( void )
{GUCEF_TRACE;

    m_serviceLines.clear();
    if ( GUCEF_NULL != m_connection )
    {
        const CWorkbenchStoreConnection::TDiscoveredVector& services = m_connection->GetDiscoveredServices();
        for ( size_t i=0; i<services.size(); ++i )
            m_serviceLines.push_back( CWorkbenchStoreConnection::DescribeService( services[ i ] ) );
    }
    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Services" );
    if ( GUCEF_NULL == list )
        return;
    CReadingSettingsScope readingSettings( *this );
    GUI::CListbox::TStringVector lines;
    for ( size_t i=0; i<m_serviceLines.size(); ++i )
        lines.push_back( m_serviceLines[ i ] );
    list->SetItems( lines );
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::ReadSettings( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_preferences )
    {
        CReadingSettingsScope readingSettings( *this );
        SetWidgetProperty( "Endpoint", GUI::EWidgetProperty_Text, m_preferences->remoteEndpoint );
        SetWidgetProperty( "LanDiscovery", GUI::EWidgetProperty_Checked, CORE::BoolToString( m_preferences->remoteLanDiscovery ) );
    }
    ShowServices();
    m_shownStateNumber = 99;
    ShowState();
    SetHasChanges( false );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::ApplySettings( void )
{GUCEF_TRACE;

    const CORE::CString endpoint = CWorkbenchStoreConnection::NormaliseEndpoint( GetWidgetProperty( "Endpoint", GUI::EWidgetProperty_Text ) );
    const bool lan = CORE::StringToBool( GetWidgetProperty( "LanDiscovery", GUI::EWidgetProperty_Checked ), true );
    if ( GUCEF_NULL != m_preferences )
    {
        m_preferences->remoteEndpoint = endpoint;
        m_preferences->remoteLanDiscovery = lan;
    }
    if ( GUCEF_NULL != m_connection )
    {
        CWorkbenchStoreConnection::SSettings settings = m_connection->GetSettings();
        settings.endpoint = endpoint;
        settings.lanDiscovery = lan;
        if ( GUCEF_NULL != m_preferences )
            settings.pairingPort = m_preferences->remotePairingPort;
        m_connection->SetSettings( settings );
    }
    SetHasChanges( false );
    NotifyObservers( SettingsAppliedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::UseEndpointOfWidgets( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_connection )
        return;
    const CORE::CString endpoint = CWorkbenchStoreConnection::NormaliseEndpoint( GetWidgetProperty( "Endpoint", GUI::EWidgetProperty_Text ) );
    if ( endpoint != m_connection->GetEndpoint() )
        m_connection->SetEndpoint( endpoint );
    CWorkbenchStoreConnection::SSettings settings = m_connection->GetSettings();
    const bool lan = CORE::StringToBool( GetWidgetProperty( "LanDiscovery", GUI::EWidgetProperty_Checked ), true );
    if ( lan != settings.lanDiscovery )
    {
        settings.lanDiscovery = lan;
        m_connection->SetSettings( settings );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::RequestFind( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_connection )
        return false;
    UseEndpointOfWidgets();
    const bool started = m_connection->Discover();
    ShowState();
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::RequestConnect( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_connection )
        return false;
    UseEndpointOfWidgets();
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Remote store: Connect" );
    const bool started = m_connection->Connect();
    ShowState();
    return started;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::RequestDisconnect( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_connection )
        return false;
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Remote store: Disconnect" );
    m_connection->Disconnect();
    ShowState();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::RequestPair( void )
{GUCEF_TRACE;

    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "PairingCode" );
    if ( GUCEF_NULL == m_connection || GUCEF_NULL == field )
        return false;
    UseEndpointOfWidgets();
    // the code goes from the field to the secret string, the copy in a text is wiped and the field is emptied
    AUTH::CSecretString code;
    if ( field->HasValue() )
    {
        CORE::CSecureBuffer buffer = field->GetValue();
        CORE::CString text;
        buffer.CopyToString( text );
        code.Set( text );
        CORE::SecureWipeString( text );
    }
    field->ClearValue();
    GUCEF_USER_LOG( CORE::LOGLEVEL_NORMAL, "Remote store: Pair" );
    const bool started = m_connection->Pair( code );
    code.Clear();
    ShowState();
    return started;
}

/*-------------------------------------------------------------------------*/

CORE::CString CWorkbenchRemoteStoreForm::GetStateText( void ) { ShowState(); return m_shownState; }
CORE::CString CWorkbenchRemoteStoreForm::GetErrorText( void ) { ShowState(); return m_shownError; }
CORE::CString CWorkbenchRemoteStoreForm::GetServerInfoText( void ) { ShowState(); return m_shownInfo; }

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchRemoteStoreForm::GetEndpointText( void )
{GUCEF_TRACE;

    return GetWidgetProperty( "Endpoint", GUI::EWidgetProperty_Text );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::SetEndpointText( const CORE::CString& endpoint )
{GUCEF_TRACE;

    // as if it was typed: the page has a change to apply
    const bool set = SetWidgetProperty( "Endpoint", GUI::EWidgetProperty_Text, endpoint );
    SetHasChanges( true );
    return set;
}

/*-------------------------------------------------------------------------*/

UInt32
CWorkbenchRemoteStoreForm::GetServiceCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_serviceLines.size();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWorkbenchRemoteStoreForm::GetServiceLine( UInt32 index ) const
{GUCEF_TRACE;

    return index < (UInt32) m_serviceLines.size() ? m_serviceLines[ index ] : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::SelectServiceLine( UInt32 index )
{GUCEF_TRACE;

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Services" );
    if ( GUCEF_NULL == list || index >= (UInt32) m_serviceLines.size() )
        return false;
    list->SetSelectedIndex( (Int32) index );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::SetPairingCode( const CORE::CString& code )
{GUCEF_TRACE;

    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "PairingCode" );
    return GUCEF_NULL != field && field->SetValue( code );
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::HasPairingCode( void )
{GUCEF_TRACE;

    GUI::CSecretInput* field = GetWidgetAs< GUI::CSecretInput >( "PairingCode" );
    return GUCEF_NULL != field && field->HasValue();
}

/*-------------------------------------------------------------------------*/

bool
CWorkbenchRemoteStoreForm::IsLanDiscoveryChecked( void )
{GUCEF_TRACE;

    return CORE::StringToBool( GetWidgetProperty( "LanDiscovery", GUI::EWidgetProperty_Checked ), true );
}

/*-------------------------------------------------------------------------*/

void CWorkbenchRemoteStoreForm::OnFindActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { RequestFind(); }
void CWorkbenchRemoteStoreForm::OnConnectActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { RequestConnect(); }
void CWorkbenchRemoteStoreForm::OnDisconnectActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { RequestDisconnect(); }
void CWorkbenchRemoteStoreForm::OnPairActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) { RequestPair(); }

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::OnServicesSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( IsReadingSettings() || GUCEF_NULL == m_connection )
        return;
    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Services" );
    const Int32 selected = GUCEF_NULL != list ? list->GetSelectedIndex() : -1;
    if ( selected >= 0 && m_connection->SelectService( (UInt32) selected ) )
    {
        CReadingSettingsScope readingSettings( *this );
        SetWidgetProperty( "Endpoint", GUI::EWidgetProperty_Text, m_connection->GetEndpoint() );
        SetHasChanges( true );
    }
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::OnConnectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CWorkbenchStoreConnection::ServicesChangedEvent == eventId )
        ShowServices();
    m_shownStateNumber = 99;
    ShowState();
}

/*-------------------------------------------------------------------------*/

void
CWorkbenchRemoteStoreForm::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // the text of a countdown (the next attempt) changes between the events of the connection
    if ( GUCEF_NULL != m_connection && CWorkbenchStoreConnection::STATE_CONNECTING == m_connection->GetState() )
        ShowState();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWorkbenchRemoteStoreForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CWorkbenchRemoteStoreForm";
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
