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

#ifndef GUCEF_CORE_CLONEABLES_H
#include "cloneables.h"
#define GUCEF_CORE_CLONEABLES_H
#endif /* GUCEF_CORE_CLONEABLES_H ? */

#ifndef GUCEF_GUI_CROUTEDEVENTDATA_H
#include "gucefGUI_CRoutedEventData.h"
#define GUCEF_GUI_CROUTEDEVENTDATA_H
#endif /* GUCEF_GUI_CROUTEDEVENTDATA_H ? */

#ifndef GUCEF_GUI_CFORMLAYOUT_H
#include "gucefGUI_CFormLayout.h"
#define GUCEF_GUI_CFORMLAYOUT_H
#endif /* GUCEF_GUI_CFORMLAYOUT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CLISTBOX_H
#include "gucefGUI_CListBox.h"
#define GUCEF_GUI_CLISTBOX_H
#endif /* GUCEF_GUI_CLISTBOX_H ? */

#ifndef GUCEF_GUI_CCOMBOBOX_H
#include "gucefGUI_CCombobox.h"
#define GUCEF_GUI_CCOMBOBOX_H
#endif /* GUCEF_GUI_CCOMBOBOX_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GAMEGUI_UTILS_H
#include "gucefGAMEGUI_Utils.h"
#define GUCEF_GAMEGUI_UTILS_H
#endif /* GUCEF_GAMEGUI_UTILS_H ? */

#include "gucefGAMEGUI_CLobbyForm.h"

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

const CORE::CEvent CLobbyForm::LaunchEvent = "GUCEF::GAMEGUI::CLobbyForm::LaunchEvent";
const CORE::CEvent CLobbyForm::LeftEvent = "GUCEF::GAMEGUI::CLobbyForm::LeftEvent";

namespace {

const char* const MenuOpenSlot = "Open slot";
const char* const MenuCloseSlot = "Close slot";
const char* const MenuAddAi = "Add AI";
const char* const MenuKick = "Kick";
const char* const MenuMakeBoss = "Make boss";
const char* const MenuBan = "Kick and ban";
const char* const MenuReport = "Report player";

const Int32 ContentTop = WindowContentTop;
const Int32 FooterHeight = WindowFooterHeight;
const Int32 SpectatorsHeight = 70;

/** The window is nine tenths of the screen within these limits; narrower than the first it has one column, from the second on the settings get more room */
const Int32 MinWindowWidth = 640;
const Int32 MaxWindowWidth = 2400;
const Int32 MaxWindowHeight = 1100;
const Int32 NarrowBelowWidth = 760;
const Int32 WideFromWidth = 1700;

/** Two UDims added: "0.29,-84" and "0,68" are "0.29,-16" */
CORE::CString
AddUDim( const CORE::CString& a, const CORE::CString& b )
{
    GUI::CUDim first, second;
    GUI::CUDim::FromString( a, first );
    GUI::CUDim::FromString( b, second );
    return GUI::CUDim( first.GetRelativeValue() + second.GetRelativeValue(), first.GetPixelValue() + second.GetPixelValue() ).ToString();
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CLobbyForm::RegisterEvents( void )
{GUCEF_TRACE;

    LaunchEvent.Initialize();
    LeftEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CLobbyForm::CLobbyForm( void )
    : GUI::CFormEx()
    , m_lobby()
    , m_chatService( GUCEF_NULL )
    , m_chat( GUCEF_NULL )
    , m_settingsPanel( GUCEF_NULL )
    , m_votePrompt( GUCEF_NULL )
    , m_slotMenu( GUCEF_NULL )
    , m_menuSlot( 0 )
    , m_menuMember()
    , m_aiType( "ai" )
    , m_reportReason( "Reported from the lobby" )
    , m_countdown( -1 )
    , m_tier( TIER_NORMAL )
    , m_subscribedToFrames( false )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLobbyForm::CLobbyForm( GUI::CIGUIContext* context )
    : GUI::CFormEx( context )
    , m_lobby()
    , m_chatService( GUCEF_NULL )
    , m_chat( GUCEF_NULL )
    , m_settingsPanel( GUCEF_NULL )
    , m_votePrompt( GUCEF_NULL )
    , m_slotMenu( GUCEF_NULL )
    , m_menuSlot( 0 )
    , m_menuMember()
    , m_aiType( "ai" )
    , m_reportReason( "Reported from the lobby" )
    , m_countdown( -1 )
    , m_tier( TIER_NORMAL )
    , m_subscribedToFrames( false )
    , m_updating( false )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CLobbyForm::~CLobbyForm()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    GUCEF_DELETE m_chat;
    m_chat = GUCEF_NULL;
    GUCEF_DELETE m_settingsPanel;
    m_settingsPanel = GUCEF_NULL;
    GUCEF_DELETE m_votePrompt;
    m_votePrompt = GUCEF_NULL;
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != m_slotMenu && GUCEF_NULL != context )
        context->DestroyWidget( m_slotMenu );
    m_slotMenu = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::AddFooter( CORE::CDataNode& window )
{GUCEF_TRACE;

    // the status and the buttons: a strip along the bottom edge that nothing else uses
    CORE::CDataNode* footer = GUI::CFormLayout::AddWidgetNode( window, "Panel", "ButtonRow", "0,1 1," + CORE::Int32ToString( -( FooterHeight + 1 ) ), "1,-2 0," + CORE::Int32ToString( FooterHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *footer, GUI::EWidgetProperty_BackgroundColour, "18,20,26,255" );
    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *footer, "Label", "Status", "0,20 0,8", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, GUI::EWidgetProperty_TextColour, "255,110,110,255" );
    CORE::CDataNode* leave = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Leave", "1,-120 1,-52", "0,100 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *leave, GUI::EWidgetProperty_Text, "Leave" );
    CORE::CDataNode* voteStart = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "VoteStart", "1,-250 1,-52", "0,120 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *voteStart, GUI::EWidgetProperty_Text, "Vote start" );
    CORE::CDataNode* forceStart = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "ForceStart", "1,-380 1,-52", "0,120 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *forceStart, GUI::EWidgetProperty_Text, "Force start" );
    CORE::CDataNode* start = GUI::CFormLayout::AddWidgetNode( *footer, "Button", "Start", "1,-500 1,-52", "0,110 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *start, GUI::EWidgetProperty_Text, "Start" );
    GUI::CFormLayout::SetWidgetNodeProperty( *start, GUI::EWidgetProperty_StyleClass, "Primary" );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::AddPlayerControls( CORE::CDataNode& parent, const CORE::CString& x, const CORE::CString& y, const CORE::CString& half )
{GUCEF_TRACE;

    // 'x' and 'y' are the corner of the controls, 'half' the width of the left half in the scale and pixels of a UDim ("0.29,-10")
    // Faction and colour on a line, ready and spectate on the next
    CORE::CDataNode* factionLabel = GUI::CFormLayout::AddWidgetNode( parent, "Label", "FactionLabel", x + " " + y, "0,64 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *factionLabel, GUI::EWidgetProperty_Text, "Faction" );
    GUI::CFormLayout::SetWidgetNodeProperty( *factionLabel, GUI::EWidgetProperty_VTextAlign, "Center" );
    GUI::CFormLayout::AddWidgetNode( parent, "Combobox", "Faction", AddUDim( x, "0,68" ) + " " + y, half + " 0,32" );
    CORE::CDataNode* colourLabel = GUI::CFormLayout::AddWidgetNode( parent, "Label", "ColourLabel", AddUDim( x, AddUDim( half, "0,84" ) ) + " " + y, "0,56 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *colourLabel, GUI::EWidgetProperty_Text, "Colour" );
    GUI::CFormLayout::SetWidgetNodeProperty( *colourLabel, GUI::EWidgetProperty_VTextAlign, "Center" );
    GUI::CFormLayout::AddWidgetNode( parent, "Combobox", "Colour", AddUDim( x, AddUDim( half, "0,144" ) ) + " " + y, half + " 0,32" );
    CORE::CDataNode* ready = GUI::CFormLayout::AddWidgetNode( parent, "Checkbox", "Ready", x + " " + AddUDim( y, "0,36" ), "0,110 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *ready, GUI::EWidgetProperty_Text, "Ready" );
    CORE::CDataNode* spectate = GUI::CFormLayout::AddWidgetNode( parent, "Checkbox", "Spectate", AddUDim( x, "0,120" ) + " " + AddUDim( y, "0,36" ), "0,150 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spectate, GUI::EWidgetProperty_Text, "Spectate" );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    const CORE::CString top = CORE::Int32ToString( ContentTop );
    const CORE::CString columnsTop = CORE::Int32ToString( ContentTop + 36 );

    // most of the screen, in the middle of it, within limits: the player can move it and fold it up
    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "LobbyForm", "0.5,0 0.5,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Title, "Lobby" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_HAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_VAlign, "Center" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MinSize, "0,640 0,480" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_MaxSize, "0," + CORE::Int32ToString( MaxWindowWidth ) + " 0," + CORE::Int32ToString( MaxWindowHeight ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, GUI::EWidgetProperty_Clip, "true" );

    if ( TIER_NARROW == m_tier )
    {
        // one column that scrolls as a whole: the countdown and the vote, the slots, the local player, the settings, the spectators, the chat
        CORE::CDataNode* body = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "Body", "0,0 0," + top, "1,0 1," + CORE::Int32ToString( -( FooterHeight + ContentTop + 8 ) ) );
        GUI::CFormLayout::SetWidgetNodeProperty( *body, GUI::EWidgetProperty_BottomPadding, "12" );
        CORE::CDataNode* list = GUI::CFormLayout::AddWidgetNode( *body, "VBox", "BodyList", "0,0 0,0", "1,0 0,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_AutoHeight, "true" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Padding, "16" );
        GUI::CFormLayout::SetWidgetNodeProperty( *list, GUI::EWidgetProperty_Spacing, "10" );

        CORE::CDataNode* countdown = GUI::CFormLayout::AddWidgetNode( *list, "Label", "Countdown", "0,0 0,0", "1,0 0,28" );
        GUI::CFormLayout::SetWidgetNodeProperty( *countdown, GUI::EWidgetProperty_TextColour, "255,220,120,255" );
        CORE::CDataNode* vote = GUI::CFormLayout::AddWidgetNode( *list, "Label", "Vote", "0,0 0,0", "1,0 0,28" );
        GUI::CFormLayout::SetWidgetNodeProperty( *vote, GUI::EWidgetProperty_TextColour, "255,220,120,255" );
        CORE::CDataNode* slots = GUI::CFormLayout::AddWidgetNode( *list, "GridView", "Slots", "0,0 0,0", "1,0 0,200" );
        GUI::CFormLayout::SetWidgetNodeProperty( *slots, GUI::EWidgetProperty_Columns, "#\nTeam\nPlayer\nFaction\nColour\nReady\nPing" );
        CORE::CDataNode* player = GUI::CFormLayout::AddWidgetNode( *list, "Panel", "PlayerRow", "0,0 0,0", "1,0 0,72" );
        AddPlayerControls( *player, "0,0", "0,0", "0.5,-73" );
        CORE::CDataNode* settingsLabel = GUI::CFormLayout::AddWidgetNode( *list, "Label", "SettingsLabel", "0,0 0,0", "1,0 0,24" );
        GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, GUI::EWidgetProperty_Text, "Game settings" );
        GUI::CFormLayout::SetWidgetNodeProperty( *settingsLabel, GUI::EWidgetProperty_TextColour, "150,172,214,255" );
        CORE::CDataNode* holder = GUI::CFormLayout::AddWidgetNode( *list, "VBox", "SettingsHolder", "0,0 0,0", "1,0 0,0" );
        GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_AutoHeight, "true" );
        GUI::CFormLayout::AddWidgetNode( *list, "Listbox", "Spectators", "0,0 0,0", "1,0 0," + CORE::Int32ToString( SpectatorsHeight ) );
        GUI::CFormLayout::AddWidgetNode( *list, "Panel", "ChatHolder", "0,0 0,0", "1,0 0,300" );
        AddFooter( *window );
        return;
    }

    // two columns, the left one a part of the window, in a very wide window less of it so the settings have room for columns of groups
    const bool wide = TIER_WIDE == m_tier;
    const CORE::CString leftWidth = wide ? "0.45,-30" : "0.58,-30";
    const CORE::CString rightPosition = wide ? "0.45,10" : "0.58,10";
    const CORE::CString rightWidth = wide ? "0.55,-30" : "0.42,-30";
    const CORE::CString half = wide ? "0.225,-90" : "0.29,-90";

    // how the start goes on the left, a vote on the right
    CORE::CDataNode* countdown = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Countdown", "0,20 0," + top, leftWidth + " 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *countdown, GUI::EWidgetProperty_TextColour, "255,220,120,255" );
    CORE::CDataNode* vote = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Vote", rightPosition + " 0," + top, rightWidth + " 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *vote, GUI::EWidgetProperty_TextColour, "255,220,120,255" );

    // the left column: the slots, the local player, the chat
    CORE::CDataNode* slots = GUI::CFormLayout::AddWidgetNode( *window, "GridView", "Slots", "0,20 0," + columnsTop, leftWidth + " 0.3,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *slots, GUI::EWidgetProperty_Columns, "#\nTeam\nPlayer\nFaction\nColour\nReady\nPing" );
    AddPlayerControls( *window, "0,20", "0.3," + CORE::Int32ToString( ContentTop + 44 ), half );
    GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ChatHolder", "0,20 0.3," + CORE::Int32ToString( ContentTop + 124 ), leftWidth + " 0.7," + CORE::Int32ToString( -( ContentTop + 124 + FooterHeight + 8 ) ) );

    // the right column: the game settings in a scroll panel, the spectators below them
    CORE::CDataNode* holder = GUI::CFormLayout::AddWidgetNode( *window, "ScrollPanel", "SettingsHolder", rightPosition + " 0," + columnsTop,
                                                                rightWidth + " 1," + CORE::Int32ToString( -( ContentTop + 36 + SpectatorsHeight + 10 + FooterHeight + 8 ) ) );
    GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_BackgroundColour, "28,31,38,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_BorderColour, "58,64,78,255" );
    GUI::CFormLayout::SetWidgetNodeProperty( *holder, GUI::EWidgetProperty_BottomPadding, "4" );
    GUI::CFormLayout::AddWidgetNode( *window, "Listbox", "Spectators", rightPosition + " 1," + CORE::Int32ToString( -( SpectatorsHeight + FooterHeight + 8 ) ), rightWidth + " 0," + CORE::Int32ToString( SpectatorsHeight ) );

    AddFooter( *window );
}

/*-------------------------------------------------------------------------*/

CLobbyForm::TTier
CLobbyForm::GetTierForScreen( void )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL == context )
        return TIER_NORMAL;
    Int32 width = (Int32) ( (Float32) context->GetLogicalWidth() * 0.92f );
    Int32 minimum = (Int32) context->GetLogicalWidth() < MinWindowWidth ? (Int32) context->GetLogicalWidth() : MinWindowWidth;
    if ( width < minimum )
        width = minimum;
    if ( width > MaxWindowWidth )
        width = MaxWindowWidth;
    return width < NarrowBelowWidth ? TIER_NARROW : ( width >= WideFromWidth ? TIER_WIDE : TIER_NORMAL );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::LoadForCurrentSize( void )
{GUCEF_TRACE;

    m_tier = GetTierForScreen();
    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    return LoadLayout( layout );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::UpdateLayoutMode( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() || GetTierForScreen() == m_tier )
        return;

    // the layout of another size of window: the lobby shown comes back as it was
    const bool wasShown = IsVisible();
    if ( !LoadForCurrentSize() )
        return;
    if ( wasShown )
        Show();
    else
        Hide();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::Build( void )
{GUCEF_TRACE;

    if ( !LoadForCurrentSize() )
        return false;
    Hide();

    // the layout follows the size of the screen: one column that scrolls when the window is narrow, more room for the settings when it is wide
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && !m_subscribedToFrames )
    {
        TEventCallback callback( this, &CLobbyForm::OnFrameStarted );
        SubscribeTo( context, GUI::CGUIContext::FrameStartedEvent, callback );
        m_subscribedToFrames = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
    UpdateLayoutMode();
    Refresh();
    return Show();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnPreLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPreLayoutLoad();

    // these live in widgets of the layout that goes away
    GUCEF_DELETE m_chat;
    m_chat = GUCEF_NULL;
    GUCEF_DELETE m_settingsPanel;
    m_settingsPanel = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnPostLayoutLoad( void )
{GUCEF_TRACE;

    GUI::CFormEx::OnPostLayoutLoad();

    TEventCallback startCallback( this, &CLobbyForm::OnStartActivated );
    TEventCallback forceStartCallback( this, &CLobbyForm::OnForceStartActivated );
    TEventCallback leaveCallback( this, &CLobbyForm::OnLeaveActivated );
    GUI::CWidget* button = GetWidget( "Start" );
    if ( GUCEF_NULL != button )
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, startCallback );
    button = GetWidget( "VoteStart" );
    if ( GUCEF_NULL != button )
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, startCallback );
    button = GetWidget( "ForceStart" );
    if ( GUCEF_NULL != button )
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, forceStartCallback );
    button = GetWidget( "Leave" );
    if ( GUCEF_NULL != button )
        SubscribeTo( button, GUI::CWidget::ActivatedEvent, leaveCallback );

    GUI::CWidget* control = GetWidget( "Faction" );
    if ( GUCEF_NULL != control )
    {
        TEventCallback callback( this, &CLobbyForm::OnFactionChanged );
        SubscribeTo( control, GUI::CWidget::ValueChangedEvent, callback );
    }
    control = GetWidget( "Colour" );
    if ( GUCEF_NULL != control )
    {
        TEventCallback callback( this, &CLobbyForm::OnColourChanged );
        SubscribeTo( control, GUI::CWidget::ValueChangedEvent, callback );
    }
    control = GetWidget( "Ready" );
    if ( GUCEF_NULL != control )
    {
        TEventCallback callback( this, &CLobbyForm::OnReadyChanged );
        SubscribeTo( control, GUI::CWidget::ValueChangedEvent, callback );
    }
    control = GetWidget( "Spectate" );
    if ( GUCEF_NULL != control )
    {
        TEventCallback callback( this, &CLobbyForm::OnSpectateChanged );
        SubscribeTo( control, GUI::CWidget::ValueChangedEvent, callback );
    }

    GUI::CWidget* slots = GetWidget( "Slots" );
    if ( GUCEF_NULL != slots )
    {
        TEventCallback activatedCallback( this, &CLobbyForm::OnSlotsItemActivated );
        SubscribeTo( slots, GUI::CWidget::ItemActivatedEvent, activatedCallback );
        TEventCallback clickCallback( this, &CLobbyForm::OnSlotsClicked );
        SubscribeTo( slots, GUI::CWidget::MouseClickEvent, clickCallback );

        GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Slots" );
        if ( GUCEF_NULL != grid )
        {
            GUCEF::vector< Int32 > widths;
            widths.push_back( 30 );     // slot
            widths.push_back( 64 );     // team
            widths.push_back( 140 );    // player
            widths.push_back( 100 );    // faction
            widths.push_back( 84 );     // colour
            widths.push_back( 72 );     // ready
            widths.push_back( 62 );     // latency
            grid->SetIntListProperty( GUI::EWidgetProperty_ColumnWidths, widths );
            grid->SetBoolProperty( GUI::EWidgetProperty_FitColumns, true );
        }
    }

    m_settingsPanel = GUCEF_NEW CLobbySettingsPanelForm();
    GUI::CWidget* settingsHolder = GetWidget( "SettingsHolder" );
    if ( GUCEF_NULL != settingsHolder )
        m_settingsPanel->Build( settingsHolder );

    m_chat = GUCEF_NEW CChatForm();
    GUI::CWidget* chatHolder = GetWidget( "ChatHolder" );
    if ( GUCEF_NULL != chatHolder )
        m_chat->Build( chatHolder );
    m_chat->SetService( m_chatService );

    if ( GUCEF_NULL == m_votePrompt )
    {
        m_votePrompt = GUCEF_NEW CVotePromptForm( GetContext() );
        m_votePrompt->Build();
    }

    ApplyLobbyToChildren();
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::ApplyLobbyToChildren( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_settingsPanel )
        m_settingsPanel->SetLobby( m_lobby );
    if ( GUCEF_NULL != m_votePrompt )
        m_votePrompt->SetLobby( m_lobby );
    if ( GUCEF_NULL != m_chat )
        m_chat->SetSingleChannel( m_lobby.IsNULL() ? CORE::CString::Empty : m_lobby->GetState().chatChannel );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::SetLobby( MULTIPLAY::CILobbyPtr lobby )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() )
        UnsubscribeFrom( m_lobby.GetPointerAlways() );

    m_lobby = lobby;
    m_countdown = -1;
    if ( !m_lobby.IsNULL() )
    {
        CORE::CNotifier* lobbyNotifier = m_lobby.GetPointerAlways();
        TEventCallback stateCallback( this, &CLobbyForm::OnLobbyStateChanged );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::LobbyStateChangedEvent, stateCallback );
        TEventCallback countdownCallback( this, &CLobbyForm::OnLobbyCountdown );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::CountdownEvent, countdownCallback );
        TEventCallback voteCallback( this, &CLobbyForm::OnLobbyVoteChanged );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::VoteStartedEvent, voteCallback );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::VoteEndedEvent, voteCallback );
        TEventCallback launchCallback( this, &CLobbyForm::OnLobbyLaunch );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::LaunchEvent, launchCallback );
        TEventCallback closedCallback( this, &CLobbyForm::OnLobbyClosed );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::LobbyClosedEvent, closedCallback );
        TEventCallback refusedCallback( this, &CLobbyForm::OnLobbyRequestRefused );
        SubscribeTo( lobbyNotifier, MULTIPLAY::CILobby::RequestRefusedEvent, refusedCallback );
    }

    ApplyLobbyToChildren();
    SetLabelText( "Status", CORE::CString::Empty );
    Refresh();
}

/*-------------------------------------------------------------------------*/

const MULTIPLAY::CILobbyPtr&
CLobbyForm::GetLobby( void ) const
{GUCEF_TRACE;

    return m_lobby;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::SetChatService( MULTIPLAY::CIChatService* chatService )
{GUCEF_TRACE;

    m_chatService = chatService;
    if ( GUCEF_NULL != m_chat )
        m_chat->SetService( m_chatService );
}

/*-------------------------------------------------------------------------*/

MULTIPLAY::CIChatService*
CLobbyForm::GetChatService( void ) const
{GUCEF_TRACE;

    return m_chatService;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::SetAiType( const CORE::CString& aiType )
{GUCEF_TRACE;

    m_aiType = aiType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyForm::GetAiType( void ) const
{GUCEF_TRACE;

    return m_aiType;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::SetReportReason( const CORE::CString& reason )
{GUCEF_TRACE;

    m_reportReason = reason;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyForm::GetReportReason( void ) const
{GUCEF_TRACE;

    return m_reportReason;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::ReportPlayer( const CORE::CString& memberId, const CORE::CString& reason )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return false;
    const MULTIPLAY::CLobbyMember* member = m_lobby->GetState().FindMember( memberId );
    CORE::CString name = GUCEF_NULL != member ? member->name : memberId;
    if ( !m_lobby->ReportPlayer( memberId, reason ) )
    {
        SetLabelText( "Status", "Reports are not taken here" );
        return false;
    }
    SetLabelText( "Status", "Reported " + name );
    return true;
}

/*-------------------------------------------------------------------------*/

CChatForm*
CLobbyForm::GetChatForm( void ) const
{GUCEF_TRACE;

    return m_chat;
}

/*-------------------------------------------------------------------------*/

CLobbySettingsPanelForm*
CLobbyForm::GetSettingsPanel( void ) const
{GUCEF_TRACE;

    return m_settingsPanel;
}

/*-------------------------------------------------------------------------*/

CVotePromptForm*
CLobbyForm::GetVotePrompt( void ) const
{GUCEF_TRACE;

    return m_votePrompt;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::SetLabelText( const char* widgetName, const CORE::CString& text )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( widgetName );
    if ( GUCEF_NULL != label )
        label->SetProperty( GUI::EWidgetProperty_Text, text );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::SetEnabled( const char* widgetName, bool enabled )
{GUCEF_TRACE;

    GUI::CWidget* widget = GetWidget( widgetName );
    if ( GUCEF_NULL != widget )
        widget->SetEnabled( enabled );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::ShowSlots( void )
{GUCEF_TRACE;

    GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Slots" );
    if ( GUCEF_NULL == grid )
        return;

    GUI::CGridView::TRowVector rows;
    if ( !m_lobby.IsNULL() )
    {
        const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
        for ( UInt32 i=0; i<(UInt32)state.slots.size(); ++i )
        {
            const MULTIPLAY::CLobbySlot& slot = state.slots[ i ];
            const MULTIPLAY::CLobbyMember* member = slot.memberId.IsNULLOrEmpty() ? GUCEF_NULL : state.FindMember( slot.memberId );

            GUI::CGridView::TStringVector cells;
            cells.resize( COLUMN_COUNT );
            cells[ COLUMN_SLOT ] = CORE::UInt32ToString( slot.index + 1 );
            cells[ COLUMN_TEAM ] = CORE::UInt32ToString( slot.team + 1 );
            if ( !slot.memberId.IsNULLOrEmpty() )
            {
                cells[ COLUMN_PLAYER ] = GetMemberName( state, slot.memberId );
                if ( slot.memberId == state.bossId )
                    cells[ COLUMN_PLAYER ] += " (boss)";
            }
            else
            if ( MULTIPLAY::LOBBYSLOTKIND_AI == slot.kind )
                cells[ COLUMN_PLAYER ] = "AI: " + slot.aiType;
            else
            if ( MULTIPLAY::LOBBYSLOTKIND_CLOSED == slot.kind )
                cells[ COLUMN_PLAYER ] = "(closed)";
            else
                cells[ COLUMN_PLAYER ] = "(open)";
            cells[ COLUMN_FACTION ] = slot.faction;
            cells[ COLUMN_COLOUR ] = slot.colour;
            if ( GUCEF_NULL != member )
            {
                cells[ COLUMN_READY ] = member->ready ? "Yes" : "";
                cells[ COLUMN_LATENCY ] = FormatPing( member->latencyMs );
            }
            rows.push_back( cells );
        }
    }

    Int32 selected = grid->GetSelectedRow();
    bool wasUpdating = m_updating;
    m_updating = true;
    grid->SetRows( rows );
    if ( selected >= 0 && selected < (Int32) rows.size() )
        grid->SetSelectedRow( selected );
    m_updating = wasUpdating;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::ShowSpectators( void )
{GUCEF_TRACE;

    GUI::CListbox* list = GetWidgetAs< GUI::CListbox >( "Spectators" );
    if ( GUCEF_NULL == list )
        return;

    GUI::CListbox::TStringVector names;
    if ( !m_lobby.IsNULL() )
    {
        const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
        for ( UInt32 i=0; i<(UInt32)state.members.size(); ++i )
        {
            const MULTIPLAY::CLobbyMember& member = state.members[ i ];
            if ( !member.IsSpectator() )
                continue;
            CORE::CString line = member.name;
            CORE::CString ping = FormatPing( member.latencyMs );
            if ( !ping.IsNULLOrEmpty() )
                line += "  " + ping;
            names.push_back( line );
        }
    }
    list->SetItems( names );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::ShowControls( void )
{GUCEF_TRACE;

    bool wasUpdating = m_updating;
    m_updating = true;

    GUI::CCombobox* faction = GetWidgetAs< GUI::CCombobox >( "Faction" );
    GUI::CCombobox* colour = GetWidgetAs< GUI::CCombobox >( "Colour" );
    GUI::CWidget* ready = GetWidget( "Ready" );
    GUI::CWidget* spectate = GetWidget( "Spectate" );

    bool isMember = false;
    bool isPlayer = false;
    bool isBoss = false;
    bool open = false;
    bool countingDown = false;
    bool voteRuns = false;

    if ( !m_lobby.IsNULL() )
    {
        const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
        const MULTIPLAY::CLobbySettingsSchema& schema = m_lobby->GetSchema();
        const MULTIPLAY::CLobbyMember* me = state.FindMember( m_lobby->GetLocalMemberId() );
        const MULTIPLAY::CLobbySlot* mySlot = state.FindSlotOfMember( m_lobby->GetLocalMemberId() );

        isMember = GUCEF_NULL != me;
        isPlayer = isMember && !me->IsSpectator();
        isBoss = isMember && m_lobby->IsBoss();
        open = MULTIPLAY::LOBBYPHASE_OPEN == state.phase;
        countingDown = MULTIPLAY::LOBBYPHASE_COUNTDOWN == state.phase;
        voteRuns = state.activeVote.IsActive();

        GUI::CWidget* root = GetRootWidget();
        if ( GUCEF_NULL != root )
            root->SetProperty( GUI::EWidgetProperty_Title, state.name.IsNULLOrEmpty() ? CORE::CString( "Lobby" ) : state.name );

        if ( GUCEF_NULL != faction )
        {
            GUI::CCombobox::TStringVector items;
            faction->GetListItems( items );
            if ( items != schema.factions )
                faction->SetListItems( schema.factions );
            Int32 index = -1;
            if ( GUCEF_NULL != mySlot )
                for ( UInt32 i=0; i<(UInt32)schema.factions.size(); ++i )
                    if ( schema.factions[ i ] == mySlot->faction )
                        index = (Int32) i;
            faction->SetSelectedIndex( index );
        }
        if ( GUCEF_NULL != colour )
        {
            GUI::CCombobox::TStringVector items;
            colour->GetListItems( items );
            if ( items != schema.colours )
                colour->SetListItems( schema.colours );
            Int32 index = -1;
            if ( GUCEF_NULL != mySlot )
                for ( UInt32 i=0; i<(UInt32)schema.colours.size(); ++i )
                    if ( schema.colours[ i ] == mySlot->colour )
                        index = (Int32) i;
            colour->SetSelectedIndex( index );
        }
        if ( GUCEF_NULL != ready )
            ready->SetBoolProperty( GUI::EWidgetProperty_Checked, isMember && me->ready );
        if ( GUCEF_NULL != spectate )
            spectate->SetBoolProperty( GUI::EWidgetProperty_Checked, isMember && me->IsSpectator() );

        if ( GUCEF_NULL != m_chat && m_chat->GetSingleChannel() != state.chatChannel )
            m_chat->SetSingleChannel( state.chatChannel );
        GUI::CWidget* chatHolder = GetWidget( "ChatHolder" );
        if ( GUCEF_NULL != chatHolder )
            chatHolder->SetVisibility( !state.chatChannel.IsNULLOrEmpty() );
    }
    else
    {
        if ( GUCEF_NULL != ready )
            ready->SetBoolProperty( GUI::EWidgetProperty_Checked, false );
        if ( GUCEF_NULL != spectate )
            spectate->SetBoolProperty( GUI::EWidgetProperty_Checked, false );
    }

    bool hasFactions = !m_lobby.IsNULL() && !m_lobby->GetSchema().factions.empty();
    bool hasColours = !m_lobby.IsNULL() && !m_lobby->GetSchema().colours.empty();
    SetEnabled( "Faction", isPlayer && open && hasFactions );
    SetEnabled( "Colour", isPlayer && open && hasColours );
    SetEnabled( "Ready", isPlayer && ( open || countingDown ) );
    SetEnabled( "Spectate", isMember && open );
    SetEnabled( "Start", isBoss && open );
    SetEnabled( "ForceStart", isBoss && open );
    SetEnabled( "VoteStart", isPlayer && !isBoss && open && !voteRuns );
    SetEnabled( "Leave", !m_lobby.IsNULL() );

    m_updating = wasUpdating;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::ShowTexts( void )
{GUCEF_TRACE;

    CORE::CString countdown;
    CORE::CString vote;
    if ( !m_lobby.IsNULL() )
    {
        const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
        switch ( state.phase )
        {
            case MULTIPLAY::LOBBYPHASE_OPEN:
                countdown = "Waiting for the players";
                break;
            case MULTIPLAY::LOBBYPHASE_COUNTDOWN:
                countdown = m_countdown >= 0 ? "The game starts in " + CORE::Int32ToString( m_countdown ) + " s" : CORE::CString( "The game is about to start" );
                break;
            case MULTIPLAY::LOBBYPHASE_LAUNCHING:
                countdown = "Launching the game";
                break;
            case MULTIPLAY::LOBBYPHASE_IN_GAME:
                countdown = "The game is in progress";
                break;
            default:
                countdown = "The lobby is closed";
                break;
        }
        if ( state.activeVote.IsActive() )
        {
            vote = "Vote: " + CVotePromptForm::DescribeVote( state.activeVote, GetMemberName( state, state.activeVote.proposer ), &m_lobby->GetSchema() ) +
                   "  (" + CVotePromptForm::DescribeTally( state.activeVote ) + ")";
        }
    }
    SetLabelText( "Countdown", countdown );
    SetLabelText( "Vote", vote );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::Refresh( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() )
        return;
    ShowSlots();
    ShowSpectators();
    ShowControls();
    ShowTexts();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::RequestStart( void )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return false;
    SetLabelText( "Status", CORE::CString::Empty );
    return m_lobby->RequestStart();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::ForceStart( void )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return false;
    SetLabelText( "Status", CORE::CString::Empty );
    return m_lobby->ForceStart();
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::Leave( void )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return false;
    MULTIPLAY::CILobbyPtr lobby = m_lobby;
    bool left = lobby->Leave();
    TLeftEventData reason( CORE::CString::Empty );
    NotifyObservers( LeftEvent, &reason );
    return left;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbyForm::GetCountdownText( void )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( "Countdown" );
    return GUCEF_NULL != label ? label->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbyForm::GetStatusText( void )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( "Status" );
    return GUCEF_NULL != label ? label->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbyForm::GetVoteText( void )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( "Vote" );
    return GUCEF_NULL != label ? label->GetProperty( GUI::EWidgetProperty_Text ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

Int32
CLobbyForm::GetCountdown( void ) const
{GUCEF_TRACE;

    return m_countdown;
}

/*-------------------------------------------------------------------------*/

Int32
CLobbyForm::FindRowOfMember( const CORE::CString& memberId ) const
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return -1;
    const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
    for ( UInt32 i=0; i<(UInt32)state.slots.size(); ++i )
        if ( state.slots[ i ].memberId == memberId )
            return (Int32) i;
    return -1;
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnSlotActivated( Int32 row )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return;
    const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
    if ( row < 0 || row >= (Int32) state.slots.size() )
        return;
    const MULTIPLAY::CLobbySlot& slot = state.slots[ row ];
    if ( slot.IsFree() )
    {
        SetLabelText( "Status", CORE::CString::Empty );
        m_lobby->RequestSlot( slot.index );
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OpenSlotMenu( Int32 x, Int32 y, Int32 row )
{GUCEF_TRACE;

    GUI::CGUIContext* context = GetGUIContext();
    if ( m_lobby.IsNULL() || GUCEF_NULL == context )
        return;
    const MULTIPLAY::CLobbyState& state = m_lobby->GetState();
    if ( row < 0 || row >= (Int32) state.slots.size() )
        return;
    const MULTIPLAY::CLobbySlot& slot = state.slots[ row ];
    bool boss = m_lobby->IsBoss() && MULTIPLAY::LOBBYPHASE_OPEN == state.phase;

    // Everyone reports another player, in any phase; the boss manages the open lobby
    GUI::CPopupMenu::TStringVector items;
    if ( !slot.memberId.IsNULLOrEmpty() )
    {
        if ( slot.memberId == m_lobby->GetLocalMemberId() )
            return;
        if ( boss )
        {
            items.push_back( MenuKick );
            items.push_back( MenuBan );
            items.push_back( MenuMakeBoss );
        }
        items.push_back( MenuReport );
    }
    else
    if ( boss )
    {
        if ( MULTIPLAY::LOBBYSLOTKIND_OPEN != slot.kind )
            items.push_back( MenuOpenSlot );
        if ( MULTIPLAY::LOBBYSLOTKIND_CLOSED != slot.kind )
            items.push_back( MenuCloseSlot );
        if ( MULTIPLAY::LOBBYSLOTKIND_AI != slot.kind )
            items.push_back( MenuAddAi );
    }
    if ( items.empty() )
        return;

    if ( GUCEF_NULL == m_slotMenu )
    {
        m_slotMenu = static_cast< GUI::CPopupMenu* >( context->CreateWidget( "PopupMenu", "LobbyForm#Menu", GUCEF_NULL ) );
        if ( GUCEF_NULL == m_slotMenu )
            return;
        TEventCallback callback( this, &CLobbyForm::OnSlotMenuActivated );
        SubscribeTo( m_slotMenu, GUI::CWidget::ItemActivatedEvent, callback );
    }
    m_menuSlot = slot.index;
    m_menuMember = slot.memberId;
    m_slotMenu->SetItems( items );
    m_slotMenu->ShowAt( x, y );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnSlotMenuItem( const CORE::CString& item )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return;
    SetLabelText( "Status", CORE::CString::Empty );
    if ( MenuOpenSlot == item )
        m_lobby->SetSlotKind( m_menuSlot, MULTIPLAY::LOBBYSLOTKIND_OPEN, CORE::CString::Empty );
    else
    if ( MenuCloseSlot == item )
        m_lobby->SetSlotKind( m_menuSlot, MULTIPLAY::LOBBYSLOTKIND_CLOSED, CORE::CString::Empty );
    else
    if ( MenuAddAi == item )
        m_lobby->SetSlotKind( m_menuSlot, MULTIPLAY::LOBBYSLOTKIND_AI, m_aiType );
    else
    if ( MenuKick == item )
        m_lobby->Kick( m_menuMember );
    else
    if ( MenuMakeBoss == item )
        m_lobby->TransferBoss( m_menuMember );
    else
    if ( MenuBan == item )
        m_lobby->Ban( m_menuMember );
    else
    if ( MenuReport == item )
        ReportPlayer( m_menuMember, m_reportReason );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnFrameStarted( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    UpdateLayoutMode();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLobbyStateChanged( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_lobby.IsNULL() )
        return;
    if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN != m_lobby->GetState().phase )
        m_countdown = -1;
    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLobbyCountdown( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    m_countdown = static_cast< const MULTIPLAY::CILobby::TCountdownEventData* >( eventData )->GetData();
    ShowTexts();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLobbyVoteChanged( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Refresh();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLobbyLaunch( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    NotifyObservers( LaunchEvent, eventData );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLobbyClosed( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const MULTIPLAY::CILobby::TLobbyClosedEventData* closed = static_cast< const MULTIPLAY::CILobby::TLobbyClosedEventData* >( eventData );
    SetLabelText( "Status", "The lobby closed: " + closed->GetData() );
    Refresh();
    TLeftEventData reason( closed->GetData() );
    NotifyObservers( LeftEvent, &reason );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLobbyRequestRefused( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    const MULTIPLAY::CLobbyRefusalEventData* refusal = static_cast< const MULTIPLAY::CLobbyRefusalEventData* >( eventData );
    SetLabelText( "Status", "Not allowed (" + refusal->what + "): " + refusal->reason );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnSlotMenuActivated( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData || GUCEF_NULL == m_slotMenu )
        return;
    GUI::CItemEventData* item = static_cast< GUI::CItemEventData* >( eventData );
    GUI::CGUIContext* context = GetGUIContext();
    if ( GUCEF_NULL != context && context->GetPopup() == m_slotMenu )
        context->ClosePopup();
    OnSlotMenuItem( item->item );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnStartActivated( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    RequestStart();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnForceStartActivated( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ForceStart();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnLeaveActivated( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    Leave();
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnReadyChanged( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updating || m_lobby.IsNULL() )
        return;
    SetLabelText( "Status", CORE::CString::Empty );
    m_lobby->SetReady( static_cast< GUI::CWidget* >( notifier )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnSpectateChanged( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_updating || m_lobby.IsNULL() )
        return;
    SetLabelText( "Status", CORE::CString::Empty );
    m_lobby->SetSpectator( static_cast< GUI::CWidget* >( notifier )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnFactionChanged( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ApplyChoice( true );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnColourChanged( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    ApplyChoice( false );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::ApplyChoice( bool isFaction )
{GUCEF_TRACE;

    if ( m_updating || m_lobby.IsNULL() )
        return;
    SetLabelText( "Status", CORE::CString::Empty );

    const MULTIPLAY::CLobbySettingsSchema& schema = m_lobby->GetSchema();
    const CORE::CString::StringVector& options = isFaction ? schema.factions : schema.colours;
    GUI::CCombobox* combo = GetWidgetAs< GUI::CCombobox >( isFaction ? "Faction" : "Colour" );
    Int32 selected = GUCEF_NULL != combo ? combo->GetSelectedIndex() : -1;
    if ( selected >= 0 && selected < (Int32) options.size() )
    {
        if ( isFaction )
            m_lobby->SetFaction( options[ selected ] );
        else
            m_lobby->SetColour( options[ selected ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnSlotsItemActivated( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CItemEventData* item = static_cast< GUI::CItemEventData* >( eventData );
    if ( GUI::EVENTPHASE_TUNNEL != item->phase )
        OnSlotActivated( item->index );
}

/*-------------------------------------------------------------------------*/

void
CLobbyForm::OnSlotsClicked( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;
    GUI::CPointerEventData* pointer = static_cast< GUI::CPointerEventData* >( eventData );
    if ( GUI::MOUSEBUTTON_RIGHT == pointer->button && GUI::EVENTPHASE_TUNNEL != pointer->phase )
    {
        GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Slots" );
        OpenSlotMenu( pointer->x, pointer->y, GUCEF_NULL != grid ? grid->GetRowAt( pointer->localY ) : -1 );
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLobbyForm::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GAMEGUI::CLobbyForm";
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
