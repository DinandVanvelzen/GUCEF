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
CLobbyForm::BuildLayout( CORE::CDataNode& layout )
{GUCEF_TRACE;

    CORE::CDataNode* window = GUI::CFormLayout::AddWidgetNode( layout, "Window", "LobbyForm", "0.04,0 0.04,0", "0.92,0 0.92,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *window, "Title", "Lobby" );

    // how the start goes
    CORE::CDataNode* countdown = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Countdown", "0,20 0,48", "1,-40 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *countdown, "TextColour", "255,220,120,255" );

    // the slots and the settings
    CORE::CDataNode* slots = GUI::CFormLayout::AddWidgetNode( *window, "GridView", "Slots", "0,20 0,84", "0.6,-20 0.34,0" );
    GUI::CFormLayout::SetWidgetNodeProperty( *slots, "Columns", "Slot\nTeam\nPlayer\nFaction\nColour\nReady\nLatency" );
    GUI::CFormLayout::AddWidgetNode( *window, "Panel", "SettingsHolder", "0.6,10 0,84", "0.4,-30 0.34,0" );

    // the local player, and the spectators
    CORE::CDataNode* factionLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "FactionLabel", "0,20 0.34,96", "0,70 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *factionLabel, "Text", "Faction" );
    GUI::CFormLayout::SetWidgetNodeProperty( *factionLabel, "VTextAlign", "Center" );
    GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "Faction", "0,94 0.34,96", "0,130 0,32" );
    CORE::CDataNode* colourLabel = GUI::CFormLayout::AddWidgetNode( *window, "Label", "ColourLabel", "0,236 0.34,96", "0,60 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *colourLabel, "Text", "Colour" );
    GUI::CFormLayout::SetWidgetNodeProperty( *colourLabel, "VTextAlign", "Center" );
    GUI::CFormLayout::AddWidgetNode( *window, "Combobox", "Colour", "0,300 0.34,96", "0,130 0,32" );
    CORE::CDataNode* ready = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "Ready", "0,440 0.34,96", "0,90 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *ready, "Text", "Ready" );
    CORE::CDataNode* spectate = GUI::CFormLayout::AddWidgetNode( *window, "Checkbox", "Spectate", "0,540 0.34,96", "0,130 0,32" );
    GUI::CFormLayout::SetWidgetNodeProperty( *spectate, "Text", "Spectate" );

    CORE::CDataNode* vote = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Vote", "0,20 0.34,138", "0.6,-20 0,28" );
    GUI::CFormLayout::SetWidgetNodeProperty( *vote, "TextColour", "255,220,120,255" );
    GUI::CFormLayout::AddWidgetNode( *window, "Listbox", "Spectators", "0.6,10 0.34,96", "0.4,-30 0,70" );

    // the chat of the lobby
    GUI::CFormLayout::AddWidgetNode( *window, "Panel", "ChatHolder", "0,20 0.34,176", "1,-40 0.66,-286" );

    // status and buttons
    CORE::CDataNode* status = GUI::CFormLayout::AddWidgetNode( *window, "Label", "Status", "0,20 1,-100", "1,-40 0,24" );
    GUI::CFormLayout::SetWidgetNodeProperty( *status, "TextColour", "255,110,110,255" );
    CORE::CDataNode* leave = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Leave", "1,-136 1,-56", "0,120 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *leave, "Text", "Leave" );
    CORE::CDataNode* voteStart = GUI::CFormLayout::AddWidgetNode( *window, "Button", "VoteStart", "1,-276 1,-56", "0,130 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *voteStart, "Text", "Vote start" );
    CORE::CDataNode* forceStart = GUI::CFormLayout::AddWidgetNode( *window, "Button", "ForceStart", "1,-416 1,-56", "0,130 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *forceStart, "Text", "Force start" );
    CORE::CDataNode* start = GUI::CFormLayout::AddWidgetNode( *window, "Button", "Start", "1,-556 1,-56", "0,130 0,38" );
    GUI::CFormLayout::SetWidgetNodeProperty( *start, "Text", "Start" );
    GUI::CFormLayout::SetWidgetNodeProperty( *start, "StyleClass", "Primary" );
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::Build( void )
{GUCEF_TRACE;

    CORE::CDataNode layout;
    GUI::CFormLayout::InitLayout( layout );
    BuildLayout( layout );
    if ( !LoadLayout( layout ) )
        return false;
    Hide();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLobbyForm::Open( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == GetRootWidget() && !Build() )
        return false;
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

    const char* buttons[] = { "Start", "ForceStart", "VoteStart", "Leave" };
    for ( UInt32 i=0; i<4; ++i )
    {
        GUI::CWidget* button = GetWidget( buttons[ i ] );
        if ( GUCEF_NULL != button )
            SubscribeTo( button, GUI::CWidget::ActivatedEvent );
    }
    const char* controls[] = { "Faction", "Colour", "Ready", "Spectate" };
    for ( UInt32 i=0; i<4; ++i )
    {
        GUI::CWidget* control = GetWidget( controls[ i ] );
        if ( GUCEF_NULL != control )
            SubscribeTo( control, GUI::CWidget::ValueChangedEvent );
    }
    GUI::CWidget* slots = GetWidget( "Slots" );
    if ( GUCEF_NULL != slots )
    {
        SubscribeTo( slots, GUI::CWidget::ItemActivatedEvent );
        SubscribeTo( slots, GUI::CWidget::MouseClickEvent );

        GUI::CGridView* grid = dynamic_cast< GUI::CGridView* >( slots );
        if ( GUCEF_NULL != grid )
        {
            GUCEF::vector< Int32 > widths;
            widths.push_back( 60 );     // slot
            widths.push_back( 60 );     // team
            widths.push_back( 220 );    // player
            widths.push_back( 120 );    // faction
            widths.push_back( 100 );    // colour
            widths.push_back( 70 );     // ready
            widths.push_back( 90 );     // latency
            grid->SetIntListProperty( "ColumnWidths", widths );
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
        SubscribeTo( m_lobby.GetPointerAlways() );

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
        label->SetProperty( "Text", text );
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
            root->SetProperty( "Title", state.name.IsNULLOrEmpty() ? CORE::CString( "Lobby" ) : state.name );

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
            ready->SetBoolProperty( "Checked", isMember && me->ready );
        if ( GUCEF_NULL != spectate )
            spectate->SetBoolProperty( "Checked", isMember && me->IsSpectator() );

        if ( GUCEF_NULL != m_chat && m_chat->GetSingleChannel() != state.chatChannel )
            m_chat->SetSingleChannel( state.chatChannel );
        GUI::CWidget* chatHolder = GetWidget( "ChatHolder" );
        if ( GUCEF_NULL != chatHolder )
            chatHolder->SetVisibility( !state.chatChannel.IsNULLOrEmpty() );
    }
    else
    {
        if ( GUCEF_NULL != ready )
            ready->SetBoolProperty( "Checked", false );
        if ( GUCEF_NULL != spectate )
            spectate->SetBoolProperty( "Checked", false );
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
    return GUCEF_NULL != label ? label->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbyForm::GetStatusText( void )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( "Status" );
    return GUCEF_NULL != label ? label->GetProperty( "Text" ) : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CLobbyForm::GetVoteText( void )
{GUCEF_TRACE;

    GUI::CWidget* label = GetWidget( "Vote" );
    return GUCEF_NULL != label ? label->GetProperty( "Text" ) : CORE::CString::Empty;
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
        SubscribeTo( m_slotMenu, GUI::CWidget::ItemActivatedEvent );
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
CLobbyForm::OnNotify( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventId  ,
                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_lobby.IsNULL() && notifier == m_lobby.GetPointerAlways() )
    {
        if ( MULTIPLAY::CILobby::LobbyStateChangedEvent == eventId )
        {
            if ( MULTIPLAY::LOBBYPHASE_COUNTDOWN != m_lobby->GetState().phase )
                m_countdown = -1;
            Refresh();
        }
        else
        if ( MULTIPLAY::CILobby::CountdownEvent == eventId && GUCEF_NULL != eventData )
        {
            m_countdown = static_cast< const MULTIPLAY::CILobby::TCountdownEventData* >( eventData )->GetData();
            ShowTexts();
        }
        else
        if ( MULTIPLAY::CILobby::VoteStartedEvent == eventId || MULTIPLAY::CILobby::VoteEndedEvent == eventId )
        {
            Refresh();
        }
        else
        if ( MULTIPLAY::CILobby::LaunchEvent == eventId )
        {
            NotifyObservers( LaunchEvent, eventData );
        }
        else
        if ( MULTIPLAY::CILobby::LobbyClosedEvent == eventId && GUCEF_NULL != eventData )
        {
            const MULTIPLAY::CILobby::TLobbyClosedEventData* closed = static_cast< const MULTIPLAY::CILobby::TLobbyClosedEventData* >( eventData );
            SetLabelText( "Status", "The lobby closed: " + closed->GetData() );
            Refresh();
            TLeftEventData reason( closed->GetData() );
            NotifyObservers( LeftEvent, &reason );
        }
        else
        if ( MULTIPLAY::CILobby::RequestRefusedEvent == eventId && GUCEF_NULL != eventData )
        {
            const MULTIPLAY::CLobbyRefusalEventData* refusal = static_cast< const MULTIPLAY::CLobbyRefusalEventData* >( eventData );
            SetLabelText( "Status", "Not allowed (" + refusal->what + "): " + refusal->reason );
        }
        return;
    }

    if ( GUCEF_NULL != m_slotMenu && notifier == m_slotMenu && GUI::CWidget::ItemActivatedEvent == eventId && GUCEF_NULL != eventData )
    {
        GUI::CItemEventData* item = static_cast< GUI::CItemEventData* >( eventData );
        GUI::CGUIContext* context = GetGUIContext();
        if ( GUCEF_NULL != context && context->GetPopup() == m_slotMenu )
            context->ClosePopup();
        OnSlotMenuItem( item->item );
        return;
    }

    if ( GUI::CWidget::ActivatedEvent == eventId )
    {
        if ( notifier == GetWidget( "Start" ) )
            RequestStart();
        else
        if ( notifier == GetWidget( "VoteStart" ) )
            RequestStart();
        else
        if ( notifier == GetWidget( "ForceStart" ) )
            ForceStart();
        else
        if ( notifier == GetWidget( "Leave" ) )
            Leave();
        return;
    }
    if ( GUI::CWidget::ValueChangedEvent == eventId )
    {
        if ( m_updating || m_lobby.IsNULL() )
            return;
        SetLabelText( "Status", CORE::CString::Empty );
        if ( notifier == GetWidget( "Ready" ) )
            m_lobby->SetReady( GetWidget( "Ready" )->GetBoolProperty( "Checked" ) );
        else
        if ( notifier == GetWidget( "Spectate" ) )
            m_lobby->SetSpectator( GetWidget( "Spectate" )->GetBoolProperty( "Checked" ) );
        else
        if ( notifier == GetWidget( "Faction" ) || notifier == GetWidget( "Colour" ) )
        {
            bool isFaction = notifier == GetWidget( "Faction" );
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
        return;
    }
    if ( GUI::CWidget::ItemActivatedEvent == eventId && GUCEF_NULL != eventData && notifier == GetWidget( "Slots" ) )
    {
        GUI::CItemEventData* item = static_cast< GUI::CItemEventData* >( eventData );
        if ( GUI::EVENTPHASE_TUNNEL != item->phase )
            OnSlotActivated( item->index );
        return;
    }
    if ( GUI::CWidget::MouseClickEvent == eventId && GUCEF_NULL != eventData && notifier == GetWidget( "Slots" ) )
    {
        GUI::CPointerEventData* pointer = static_cast< GUI::CPointerEventData* >( eventData );
        if ( GUI::MOUSEBUTTON_RIGHT == pointer->button && GUI::EVENTPHASE_TUNNEL != pointer->phase )
        {
            GUI::CGridView* grid = GetWidgetAs< GUI::CGridView >( "Slots" );
            OpenSlotMenu( pointer->x, pointer->y, GUCEF_NULL != grid ? grid->GetRowAt( pointer->localY ) : -1 );
        }
        return;
    }
    GUI::CFormEx::OnNotify( notifier, eventId, eventData );
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
