/*
 *  gucefGAMEGUI: GUCEF module providing prebaked GUI forms for rapid game development
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CEDITBOX_H
#include "gucefGUI_CEditbox.h"
#define GUCEF_GUI_CEDITBOX_H
#endif /* GUCEF_GUI_CEDITBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CSPINNER_H
#include "gucefGUI_CSpinner.h"
#define GUCEF_GUI_CSPINNER_H
#endif /* GUCEF_GUI_CSPINNER_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CCreateLobbyForm.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GAMEGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CCreateLobbyForm::CreateRequestedEvent = "GUCEF::GAMEGUI::CCreateLobbyForm::CreateRequestedEvent";
const CORE::CEvent CCreateLobbyForm::CancelledEvent = "GUCEF::GAMEGUI::CCreateLobbyForm::CancelledEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCreateLobbyRequest::CCreateLobbyRequest( void )
    : CORE::CICloneable()
    , params()
    , serviceName()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCreateLobbyRequest::CCreateLobbyRequest( const CCreateLobbyRequest& src )
    : CORE::CICloneable( src )
    , params( src.params )
    , serviceName( src.serviceName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCreateLobbyRequest::~CCreateLobbyRequest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCreateLobbyRequest&
CCreateLobbyRequest::operator=( const CCreateLobbyRequest& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        params = src.params;
        serviceName = src.serviceName;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CCreateLobbyRequest::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CCreateLobbyRequest( *this );
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::RegisterEvents( void )
{GUCEF_TRACE;

    CreateRequestedEvent.Initialize();
    CancelledEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CCreateLobbyForm::CCreateLobbyForm( void )
    : GUI::CFormEx()
    , m_template()
    , m_serviceNames()
    , m_settingsPanel( GUCEF_NULL )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_template.maxPlayers = 8;
    m_template.allowSpectators = true;
}

/*-------------------------------------------------------------------------*/

CCreateLobbyForm::CCreateLobbyForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_template()
    , m_serviceNames()
    , m_settingsPanel( GUCEF_NULL )
    , m_open( false )
{GUCEF_TRACE;

    RegisterEvents();
    m_template.maxPlayers = 8;
    m_template.allowSpectators = true;
}

/*-------------------------------------------------------------------------*/

CCreateLobbyForm::~CCreateLobbyForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_settingsPanel;
    m_settingsPanel = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "CreateLobbyForm", "0.5,-340 0.5,-350", "0,680 0,700" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, "Title", "Create a lobby" );

    CORE::CDataNode* nameLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "NameLabel", "0,20 0,52", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *nameLabel, "Text", "Name" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Name", "0,20 0,76", "1,-40 0,32" );

    CORE::CDataNode* passwordLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "PasswordLabel", "0,20 0,116", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *passwordLabel, "Text", "Password, if players need one to join" );
    GUI::CFormLayout::AddWidgetNode( *window, "Editbox", "Password", "0,20 0,140", "1,-40 0,32" );

    CORE::CDataNode* maxLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "MaxPlayersLabel", "0,20 0,180", "0,200 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxLabel, "Text", "Most players" );
    CORE::CDataNode* max = GUI::CFormLayout::AddWidgetNode( *window, "Spinner", "MaxPlayers", "0,20 0,204", "0,200 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, "Min", "1" );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, "Max", CORE::UInt32ToString( MaxPlayersLimit ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, "Step", "1" );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, "Value", "8" );
    CORE::CDataNode* spectators = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "Spectators", "0,240 0,204", "0,300 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spectators, "Text", "Spectators may watch" );

    CORE::CDataNode* whereLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "WhereLabel", "0,20 0,244", "0,200 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *whereLabel, "Text", "Where" );
    CORE::CDataNode* where = GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "Where", "0,20 0,268", "0,200 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *where, "Items", "LAN\nInternet" );
    GUI::CFormLayout::SetWidgetNodeProperty( *where, "SelectedIndex", "0" );
    CORE::CDataNode* serviceLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "ServiceLabel", "0,240 0,244", "0,300 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *serviceLabel, "Text", "Lobby service" );
    GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "Service", "0,240 0,268", "0,300 0,32" );

    CORE::CDataNode* settingsLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "SettingsLabel", "0,20 0,316", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, "Text", "Game settings" );
    GUI::CFormLayout::AddWidgetNode( *window, "Panel", "SettingsHolder", "0,20 0,344", "1,-40 1,-456" );

    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Error", "0,20 1,-100", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, "TextColour", "255,110,110,255" );

    CORE::CDataNode* create = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Create", "1,-252 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *create, "Text", "Create" );
    GUI::CFormLayout::SetWidgetNodeProperty( *create, "StyleClass", "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Cancel", "1,-128 1,-56", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, "Text", "Cancel" );
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    m_open = false;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnPreLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPreLayoutLoad();

    // the panel lives in a widget of this layout, what the player set stays
    if ( GUCEF_NULL != m_settingsPanel )
    {
        m_settingsPanel->GetValues( m_template.initialSettings );
        GUCEF_DELETE m_settingsPanel;
        m_settingsPanel = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    const char* buttons[] = { "Create", "Cancel" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL != button )
            SubscribeTo( button, GUI::CWidget::ActivatedEvent );
    }
    const char* fields[] = { "Name", "Password" };
    for ( UInt32 i=0; i<2; ++i )
    {
        GUI::CWidget* field = GetWidget( fields[ i ] );
        if ( GUCEF_NULL != field )
            SubscribeTo( field, GUI::CEditbox::TextEnteredEvent );
    }
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
        SubscribeTo( root, GUI::CWidget::NavigationEvent );

    m_settingsPanel = GUCEF_NEW CLobbySettingsPanelForm();
    m_settingsPanel->SetSchema( m_template.schema );
    GUI::CWidget* holder = GetWidget( "SettingsHolder" );
    if ( GUCEF_NULL != holder )
        m_settingsPanel->Build( holder );

    ShowServiceNames();
    ShowTemplate();
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::ShowServiceNames( void )
{GUCEF_TRACE;

    GUI::CCombobox* service = GetWidgetAs< GUI::CCombobox >( "Service" );
    if ( GUCEF_NULL == service )
        return;

    CORE::CString::StringVector names = m_serviceNames;
    if ( names.empty() )
    {
        MULTIPLAY::CLobbyServiceFactory::TKeySet keys;
        MULTIPLAY::CMultiplayGlobal::Instance()->GetLobbyServiceFactory().ObtainKeySet( keys );
        for ( MULTIPLAY::CLobbyServiceFactory::TKeySet::const_iterator i = keys.begin(); i != keys.end(); ++i )
            names.push_back( (*i) );
    }
    service->SetListItems( names );
    service->SetSelectedIndex( names.empty() ? -1 : 0 );
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::ShowTemplate( void )
{GUCEF_TRACE;

    GUI::CWidget* name = GetWidget( "Name" );
    if ( GUCEF_NULL != name )
        name->SetProperty( "Text", m_template.name );
    GUI::CWidget* password = GetWidget( "Password" );
    if ( GUCEF_NULL != password )
        password->SetProperty( "Text", m_template.password );
    GUI::CWidget* max = GetWidget( "MaxPlayers" );
    if ( GUCEF_NULL != max )
        max->SetProperty( "Value", CORE::UInt32ToString( m_template.maxPlayers ) );
    GUI::CWidget* spectators = GetWidget( "Spectators" );
    if ( GUCEF_NULL != spectators )
        spectators->SetBoolProperty( "Checked", m_template.allowSpectators );
    GUI::CCombobox* where = GetWidgetAs< GUI::CCombobox >( "Where" );
    if ( GUCEF_NULL != where )
        where->SetSelectedIndex( m_template.isPublic ? 1 : 0 );
    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );

    if ( GUCEF_NULL != m_settingsPanel )
    {
        CORE::CValueList start;
        m_template.GetStartSettings( start );
        m_settingsPanel->SetValues( start );
    }
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::SetTemplate( const MULTIPLAY::CLobbyCreateParams& params )
{GUCEF_TRACE;

    m_template = params;
    if ( GUCEF_NULL == GetRootWidget() )
        return;
    if ( GUCEF_NULL != m_settingsPanel )
        m_settingsPanel->SetSchema( m_template.schema );
    ShowTemplate();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CLobbyCreateParams&
CCreateLobbyForm::GetTemplate( void ) const
{GUCEF_TRACE;

    return m_template;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::SetServiceNames( const CORE::CString::StringVector& serviceNames )
{GUCEF_TRACE;

    m_serviceNames = serviceNames;
    ShowServiceNames();
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CCreateLobbyForm::GetServiceNames( void ) const
{GUCEF_TRACE;

    return m_serviceNames;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCreateLobbyForm::GetServiceName( void )
{GUCEF_TRACE;

    GUI::CCombobox* service = GetWidgetAs< GUI::CCombobox >( "Service" );
    if ( GUCEF_NULL == service )
        return CORE::CString::Empty;
    Int32 selected = service->GetSelectedIndex();
    GUI::CCombobox::TStringVector items;
    service->GetListItems( items );
    if ( selected < 0 || selected >= (Int32) items.size() )
        return CORE::CString::Empty;
    return items[ selected ];
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::SelectService( const CORE::CString& serviceName )
{GUCEF_TRACE;

    GUI::CCombobox* service = GetWidgetAs< GUI::CCombobox >( "Service" );
    if ( GUCEF_NULL == service )
        return false;
    GUI::CCombobox::TStringVector items;
    service->GetListItems( items );
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
        if ( items[ i ] == serviceName )
            return service->SetSelectedIndex( (Int32) i );
    return false;
}

/*-------------------------------------------------------------------------*/

CLobbySettingsPanelForm*
CCreateLobbyForm::GetSettingsPanel( void ) const
{GUCEF_TRACE;

    return m_settingsPanel;
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::GetCreateParams( MULTIPLAY::CLobbyCreateParams& params )
{GUCEF_TRACE;

    GUI::CWidget* error = GetWidget( "Error" );
    GUI::CWidget* name = GetWidget( "Name" );
    CORE::CString lobbyName = GUCEF_NULL != name ? TrimText( name->GetProperty( "Text" ) ) : CORE::CString::Empty;
    if ( lobbyName.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( "Text", "Give the lobby a name" );
        return false;
    }
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );

    params = m_template;
    params.name = lobbyName;

    GUI::CWidget* password = GetWidget( "Password" );
    params.password = GUCEF_NULL != password ? password->GetProperty( "Text" ) : CORE::CString::Empty;

    GUI::CWidget* max = GetWidget( "MaxPlayers" );
    if ( GUCEF_NULL != max )
    {
        Int32 value = max->GetIntProperty( "Value" );
        params.maxPlayers = value < 1 ? 1 : (UInt32) value;
    }
    GUI::CWidget* spectators = GetWidget( "Spectators" );
    if ( GUCEF_NULL != spectators )
        params.allowSpectators = spectators->GetBoolProperty( "Checked" );
    GUI::CCombobox* where = GetWidgetAs< GUI::CCombobox >( "Where" );
    if ( GUCEF_NULL != where )
        params.isPublic = 1 == where->GetSelectedIndex();

    if ( GUCEF_NULL != m_settingsPanel )
    {
        CORE::CValueList values;
        m_settingsPanel->GetValues( values );
        params.initialSettings = values;
    }

    // a template with slots of its own gets as many as there are players
    if ( !params.slots.empty() )
    {
        while ( params.slots.size() > params.maxPlayers )
            params.slots.pop_back();
        while ( params.slots.size() < params.maxPlayers )
        {
            MULTIPLAY::CLobbySlot slot;
            slot.index = (UInt32) params.slots.size();
            slot.team = slot.index;
            params.slots.push_back( slot );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;

    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( "Text", CORE::CString::Empty );

    m_open = true;
    bool shown = ShowModal();
    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* name = GetWidget( "Name" );
    if ( shown && GUCEF_NULL != context && GUCEF_NULL != name )
        context->SetFocus( name );
    return shown;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::Close( void )
{GUCEF_TRACE;

    m_open = false;
    Hide();
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::IsOpen( void ) const
{GUCEF_TRACE;

    return m_open;
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::Create( void )
{GUCEF_TRACE;

    CCreateLobbyRequest request;
    if ( !GetCreateParams( request.params ) )
        return false;
    request.serviceName = GetServiceName();
    Close();
    NotifyObservers( CreateRequestedEvent, &request );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::Cancel( void )
{GUCEF_TRACE;

    Close();
    NotifyObservers( CancelledEvent );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CCreateLobbyForm::GetErrorText( void )
{GUCEF_TRACE;

    GUI::CWidget* error = GetWidget( "Error" );
    return GUCEF_NULL != error ? error->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnNotify( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUI::CWidget::ActivatedEvent == eventId )
    {
        if ( notifier == GetWidget( "Create" ) )
            Create();
        else
        if ( notifier == GetWidget( "Cancel" ) )
            Cancel();
        return;
    }
    if ( GUI::CEditbox::TextEnteredEvent == eventId )
    {
        if ( m_open )
            Create();
        return;
    }
    if ( GUI::CWidget::NavigationEvent == eventId && GUCEF_NULL != eventData )
    {
        GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
        if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
        {
            navigation->handled = true;
            Cancel();
        }
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CCreateLobbyForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CCreateLobbyForm";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GAMEGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
