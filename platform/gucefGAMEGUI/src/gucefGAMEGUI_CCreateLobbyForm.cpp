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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The window is this part of the screen, within these limits (pixels) */
const Float32 WindowScreenPart = 0.9f;
const Int32 MinWindowWidth = 560;
const Int32 MinWindowHeight = 460;
const Int32 MaxWindowWidth = 2000;
const Int32 MaxWindowHeight = 1000;

/** A window narrower than this has one column that scrolls as a whole */
const Int32 NarrowBelowWidth = 700;

/** A window this wide or wider has the general fields in a column of their own width, the settings get the rest (in columns of groups) */
const Int32 WideFromWidth = 1500;
const Int32 WideGeneralWidth = 480;

/** Below the title bar (of the dark theme), and the strip with the error and the buttons along the bottom edge */
const Int32 ContentTop = WindowContentTop;
const Int32 FooterHeight = WindowFooterHeight;

const Int32 LabelHeight = 24;
const Int32 ControlHeight = 32;
const Int32 FieldHeight = LabelHeight + 2 + ControlHeight;

}

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
    , m_tier( TIER_NORMAL )
    , m_open( false )
    , m_subscribedToFrames( false )
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
    , m_tier( TIER_NORMAL )
    , m_open( false )
    , m_subscribedToFrames( false )
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
CCreateLobbyForm::AddGeneralFields( CORE::CDataNode& parent )
{GUCEF_TRACE;

    // every field is a label above its control, in a panel of its own so the pairs stand apart
    CORE::CDataNode* nameField = GUI::CFormLayout::AddWidgetNode( parent, "Panel", "NameField", "0,0 0,0", "1,0 0," + CORE::Int32ToString( FieldHeight ) );
    CORE::CDataNode* nameLabel = GUI::CFormLayout::AddWidgetNode( *nameField, "Label", "NameLabel", "0,0 0,0", "1,0 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *nameLabel, GUI::EWidgetProperty_Text, "Name" );
    GUI::CFormLayout::AddWidgetNode( *nameField, "Editbox", "Name", "0,0 0," + CORE::Int32ToString( LabelHeight + 2 ), "1,0 0," + CORE::Int32ToString( ControlHeight ) );

    CORE::CDataNode* passwordField = GUI::CFormLayout::AddWidgetNode( parent, "Panel", "PasswordField", "0,0 0,0", "1,0 0," + CORE::Int32ToString( FieldHeight ) );
    CORE::CDataNode* passwordLabel = GUI::CFormLayout::AddWidgetNode( *passwordField, "Label", "PasswordLabel", "0,0 0,0", "1,0 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *passwordLabel, GUI::EWidgetProperty_Text, "Password (optional)" );
    GUI::CFormLayout::AddWidgetNode( *passwordField, "Editbox", "Password", "0,0 0," + CORE::Int32ToString( LabelHeight + 2 ), "1,0 0," + CORE::Int32ToString( ControlHeight ) );

    CORE::CDataNode* playersField = GUI::CFormLayout::AddWidgetNode( parent, "Panel", "PlayersField", "0,0 0,0", "1,0 0," + CORE::Int32ToString( FieldHeight ) );
    CORE::CDataNode* maxLabel = GUI::CFormLayout::AddWidgetNode( *playersField, "Label", "MaxPlayersLabel", "0,0 0,0", "1,0 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *maxLabel, GUI::EWidgetProperty_Text, "Most players" );
    CORE::CDataNode* max = GUI::CFormLayout::AddWidgetNode( *playersField, "Spinner", "MaxPlayers", "0,0 0," + CORE::Int32ToString( LabelHeight + 2 ), "0,150 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, GUI::EWidgetProperty_Min, "1" );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, GUI::EWidgetProperty_Max, CORE::UInt32ToString( MaxPlayersLimit ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, GUI::EWidgetProperty_Step, "1" );
    GUI::CFormLayout::SetWidgetNodeProperty( *max, GUI::EWidgetProperty_Value, "8" );
    CORE::CDataNode* spectators = GUI::CFormLayout::AddWidgetNode( parent, "Checkbox", "Spectators", "0,0 0,0", "1,0 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *spectators, GUI::EWidgetProperty_Text, "Spectators may watch" );

    // where the lobby is, and the service that makes it: side by side
    CORE::CDataNode* whereField = GUI::CFormLayout::AddWidgetNode( parent, "Panel", "WhereField", "0,0 0,0", "1,0 0," + CORE::Int32ToString( FieldHeight ) );
    CORE::CDataNode* whereLabel = GUI::CFormLayout::AddWidgetNode( *whereField, "Label", "WhereLabel", "0,0 0,0", "0.5,-6 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *whereLabel, GUI::EWidgetProperty_Text, "Where" );
    CORE::CDataNode* where = GUI::CFormLayout::AddWidgetNode( *whereField, "Combobox", "Where", "0,0 0," + CORE::Int32ToString( LabelHeight + 2 ), "0.5,-6 0," + CORE::Int32ToString( ControlHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *where, GUI::EWidgetProperty_Items, "LAN\nInternet" );
    GUI::CFormLayout::SetWidgetNodeProperty( *where, GUI::EWidgetProperty_SelectedIndex, "0" );
    CORE::CDataNode* serviceLabel = GUI::CFormLayout::AddWidgetNode( *whereField, "Label", "ServiceLabel", "0.5,6 0,0", "0.5,-7 0," + CORE::Int32ToString( LabelHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *serviceLabel, GUI::EWidgetProperty_Text, "Lobby service" );
    GUI::CFormLayout::AddWidgetNode( *whereField, "Combobox", "Service", "0.5,6 0," + CORE::Int32ToString( LabelHeight + 2 ), "0.5,-7 0," + CORE::Int32ToString( ControlHeight ) );
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::AddFooter( CORE::CDataNode& window )
{GUCEF_TRACE;

    // the error and the buttons: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( window, "Panel", "ButtonRow", "0,1 1," + CORE::Int32ToString( -( FooterHeight + 1 ) ), "1,-2 0," + CORE::Int32ToString( FooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );

    CORE::CDataNode* error = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Error", "0,20 0,8", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *error, GUI::EWidgetProperty_TextColour, "255,110,110,255" );

    CORE::CDataNode* create = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Create", "1,-252 1,-52", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *create, GUI::EWidgetProperty_Text, "Create" );
    GUI::CFormLayout::SetWidgetNodeProperty( *create, GUI::EWidgetProperty_StyleClass, "Primary" );
    CORE::CDataNode* cancel = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Cancel", "1,-128 1,-52", "0,112 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *cancel, GUI::EWidgetProperty_Text, "Cancel" );
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    // a window of nine tenths of the screen up to a size that is plenty, in the middle of it
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "CreateLobbyForm", "0.5,0 0.5,0", "0.9,0 0.9,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Create a lobby" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0," + CORE::Int32ToString( MinWindowWidth ) + " 0," + CORE::Int32ToString( MinWindowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0," + CORE::Int32ToString( MaxWindowWidth ) + " 0," + CORE::Int32ToString( MaxWindowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    const CORE::CString bodyTop = CORE::Int32ToString( ContentTop );
    const CORE::CString headerColour = "150,172,214,255";

    if ( TIER_NARROW == m_tier )
    {
        // one column that scrolls as a whole: the general fields, then the game settings
        CORE::CDataNode* body = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "Body", "0,0 0," + bodyTop, "1,0 1," + CORE::Int32ToString( -( FooterHeight + ContentTop + 8 ) ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *body, GUI::EWidgetProperty_BottomPadding, "12" );
        CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *body, "VBox", "BodyList", "0,0 0,0", "1,0 0,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_AutoHeight, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Padding, "16" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Spacing, "12" );

        CORE::CDataNode* generalHeader = GUI::CFormLayout::AddWidgetNode( *list, "Label", "GeneralLabel", "0,0 0,0", "1,0 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *generalHeader, GUI::EWidgetProperty_Text, "General" );
        GUI::CFormLayout::SetWidgetNodeProperty( *generalHeader, GUI::EWidgetProperty_TextColour, headerColour );
        AddGeneralFields( *list );
        CORE::CDataNode* settingsLabel = GUI::CFormLayout::AddWidgetNode( *list, "Label", "SettingsLabel", "0,0 0,0", "1,0 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, GUI::EWidgetProperty_Text, "Game settings" );
        GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, GUI::EWidgetProperty_TextColour, headerColour );
        CORE::CDataNode* holder = GUI::CFormLayout::AddWidgetNode( *list, "VBox", "SettingsHolder", "0,0 0,0", "1,0 0,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_AutoHeight, "true" );
    }
    else
    {
        // two columns: the general fields on the left, the game settings on the right, as much of them as fits and a scroll bar for the rest;
        // a window that is very wide has the general fields in a column of their own width and the groups of the settings side by side
        const bool wide = TIER_WIDE == m_tier;
        const CORE::CString generalSize = wide ? "0," + CORE::Int32ToString( WideGeneralWidth ) : CORE::CString( "0.4,-30" );
        const CORE::CString settingsPosition = wide ? "0," + CORE::Int32ToString( WideGeneralWidth + 40 ) : CORE::CString( "0.4,10" );
        const CORE::CString settingsSize = wide ? "1," + CORE::Int32ToString( -( WideGeneralWidth + 60 ) ) : CORE::CString( "0.6,-30" );
        const CORE::CString columnTop = CORE::Int32ToString( ContentTop + 28 );
        const CORE::CString columnHeight = CORE::Int32ToString( -( FooterHeight + ContentTop + 28 + 8 ) );

        CORE::CDataNode* generalLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "GeneralLabel", "0,20 0," + bodyTop, generalSize + " 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *generalLabel, GUI::EWidgetProperty_Text, "General" );
        GUI::CFormLayout::SetWidgetNodeProperty( *generalLabel, GUI::EWidgetProperty_TextColour, headerColour );
        CORE::CDataNode* general = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "GeneralColumn", "0,20 0," + columnTop, generalSize + " 1," + columnHeight );
        CORE::CDataNode* generalList = GUI::CFormLayout::AddWidgetNode( *general, "VBox", "GeneralList", "0,0 0,0", "1,0 0,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *generalList, GUI::EWidgetProperty_AutoHeight, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *generalList, GUI::EWidgetProperty_Spacing, "14" );
        AddGeneralFields( *generalList );

        CORE::CDataNode* settingsLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "SettingsLabel", settingsPosition + " 0," + bodyTop, settingsSize + " 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, GUI::EWidgetProperty_Text, "Game settings" );
        GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, GUI::EWidgetProperty_TextColour, headerColour );
        CORE::CDataNode* holder = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "SettingsHolder", settingsPosition + " 0," + columnTop, settingsSize + " 1," + columnHeight );
        GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_BackgroundColour, "28,31,38,255" );
        GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_BorderColour, "58,64,78,255" );
        GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_BottomPadding, "4" );
    }

    AddFooter( *window );
}

/*-------------------------------------------------------------------------*/

CCreateLobbyForm::TTier
CCreateLobbyForm::GetTierForScreen( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return TIER_NORMAL;
    Int32 width = (Int32) ( (Float32) context->GetLogicalWidth() * WindowScreenPart );
    if ( width < MinWindowWidth )
        width = MinWindowWidth;
    if ( width > MaxWindowWidth )
        width = MaxWindowWidth;
    return width < NarrowBelowWidth ? TIER_NARROW : ( width >= WideFromWidth ? TIER_WIDE : TIER_NORMAL );
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::LoadForCurrentSize( void )
{GUCEF_TRACE;

    m_tier = GetTierForScreen();
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

bool
CCreateLobbyForm::Build( void )
{GUCEF_TRACE;

    if ( !LoadForCurrentSize() )
        return false;
    Hide();
    m_open = false;

    // the layout follows the size of the screen: one column that scrolls when the window gets narrow
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_subscribedToFrames )
    {
        TEventCallback callback( this, &CCreateLobbyForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
        m_subscribedToFrames = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::UpdateLayoutMode( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() || GetTierForScreen() == m_tier )
        return;

    // what the player filled in goes along into the other layout
    CORE::CString name, password, serviceName, error;
    Int32 maxPlayers = 0, whereIndex = 0;
    bool spectators = true;
    GUI::CWidget* field = GetWidget( "Name" );
    if ( GUCEF_NULL != field )
        name = field->GetProperty( GUI::EWidgetProperty_Text );
    field = GetWidget( "Password" );
    if ( GUCEF_NULL != field )
        password = field->GetProperty( GUI::EWidgetProperty_Text );
    field = GetWidget( "MaxPlayers" );
    if ( GUCEF_NULL != field )
        maxPlayers = field->GetIntProperty( GUI::EWidgetProperty_Value );
    field = GetWidget( "Spectators" );
    if ( GUCEF_NULL != field )
        spectators = field->GetBoolProperty( GUI::EWidgetProperty_Checked );
    GUI::CCombobox* where = GetWidgetAs< GUI::CCombobox >( "Where" );
    if ( GUCEF_NULL != where )
        whereIndex = where->GetSelectedIndex();
    serviceName = GetServiceName();
    error = GetErrorText();

    GUI::CGUIContext* context = GetGUIContext();
    GUI::CWidget* focus = GUCEF_NULL != context ? context->GetFocus() : GUCEF_NULL;
    CORE::CString focusName = GUCEF_NULL != focus ? focus->GetName() : CORE::CString::Empty;

    if ( !LoadForCurrentSize() )
        return;

    field = GetWidget( "Name" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, name );
    field = GetWidget( "Password" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, password );
    field = GetWidget( "MaxPlayers" );
    if ( GUCEF_NULL != field && maxPlayers > 0 )
        field->SetProperty( GUI::EWidgetProperty_Value, CORE::Int32ToString( maxPlayers ) );
    field = GetWidget( "Spectators" );
    if ( GUCEF_NULL != field )
        field->SetBoolProperty( GUI::EWidgetProperty_Checked, spectators );
    where = GetWidgetAs< GUI::CCombobox >( "Where" );
    if ( GUCEF_NULL != where )
        where->SetSelectedIndex( whereIndex );
    SelectService( serviceName );
    field = GetWidget( "Error" );
    if ( GUCEF_NULL != field )
        field->SetProperty( GUI::EWidgetProperty_Text, error );

    if ( m_open )
    {
        ShowModal();
        field = focusName.IsNULLOrEmpty() ? GUCEF_NULL : GetWidget( focusName );
        if ( GUCEF_NULL != context && GUCEF_NULL != field )
            context->SetFocus( field );
    }
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

    GUI::CWidget* button = GetWidget( "Create" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CCreateLobbyForm::OnCreateActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    button = GetWidget( "Cancel" );
    if ( GUCEF_NULL != button )
    {
        TEventCallback callback( this, &CCreateLobbyForm::OnCancelActivated );
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, callback );
    }
    TEventCallback fieldCallback( this, &CCreateLobbyForm::OnFieldTextEntered );
    GUI::CWidget* field = GetWidget( "Name" );
    if ( GUCEF_NULL != field )
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, fieldCallback );
    field = GetWidget( "Password" );
    if ( GUCEF_NULL != field )
        SubscribeTo( field, GUI::CEditbox::TextEnteredEvent, fieldCallback );
    GUI::CWidget* root = GetRootWidget();
    if ( GUCEF_NULL != root )
    {
        TEventCallback callback( this, &CCreateLobbyForm::OnNavigation );
        SubscribeTo( root, GUI::CWidget::NavigationEvent, callback );
    }

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
        name->SetProperty( GUI::EWidgetProperty_Text, m_template.name );
    GUI::CWidget* password = GetWidget( "Password" );
    if ( GUCEF_NULL != password )
        password->SetProperty( GUI::EWidgetProperty_Text, m_template.password );
    GUI::CWidget* max = GetWidget( "MaxPlayers" );
    if ( GUCEF_NULL != max )
        max->SetProperty( GUI::EWidgetProperty_Value, CORE::UInt32ToString( m_template.maxPlayers ) );
    GUI::CWidget* spectators = GetWidget( "Spectators" );
    if ( GUCEF_NULL != spectators )
        spectators->SetBoolProperty( GUI::EWidgetProperty_Checked, m_template.allowSpectators );
    GUI::CCombobox* where = GetWidgetAs< GUI::CCombobox >( "Where" );
    if ( GUCEF_NULL != where )
        where->SetSelectedIndex( m_template.isPublic ? 1 : 0 );
    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, CORE::CString::Empty );

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
    CORE::CString lobbyName = GUCEF_NULL != name ? TrimText( name->GetProperty( GUI::EWidgetProperty_Text ) ) : CORE::CString::Empty;
    if ( lobbyName.IsNULLOrEmpty() )
    {
        if ( GUCEF_NULL != error )
            error->SetProperty( GUI::EWidgetProperty_Text, "Give the lobby a name" );
        return false;
    }
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, CORE::CString::Empty );

    params = m_template;
    params.name = lobbyName;

    GUI::CWidget* password = GetWidget( "Password" );
    params.password = GUCEF_NULL != password ? password->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;

    GUI::CWidget* max = GetWidget( "MaxPlayers" );
    if ( GUCEF_NULL != max )
    {
        Int32 value = max->GetIntProperty( GUI::EWidgetProperty_Value );
        params.maxPlayers = value < 1 ? 1 : (UInt32) value;
    }
    GUI::CWidget* spectators = GetWidget( "Spectators" );
    if ( GUCEF_NULL != spectators )
        params.allowSpectators = spectators->GetBoolProperty( GUI::EWidgetProperty_Checked );
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
    UpdateLayoutMode();

    GUI::CWidget* error = GetWidget( "Error" );
    if ( GUCEF_NULL != error )
        error->SetProperty( GUI::EWidgetProperty_Text, CORE::CString::Empty );

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
    return GUCEF_NULL != error ? error->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        UpdateLayoutMode();
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnCreateActivated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Create();
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnCancelActivated( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Cancel();
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnFieldTextEntered( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_open )
        Create();
}

/*-------------------------------------------------------------------------*/

void
CCreateLobbyForm::OnNavigation( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CNavigationEventData* navigation = static_cast< GUI::CNavigationEventData* >( eventData );
    if ( GUI::NAVIGATION_CANCEL == navigation->direction && GUI::EVENTPHASE_TUNNEL != navigation->phase && m_open )
    {
        navigation->handled = true;
        Cancel();
    }
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
