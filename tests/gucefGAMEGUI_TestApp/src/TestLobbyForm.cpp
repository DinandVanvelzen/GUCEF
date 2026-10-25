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
#include "TestLobbyFixture.h"
#include "TestChatService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the events of a lobby form */
class CLobbyFormRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CLobbyFormRecorder > TEventCallback;

    UInt32 launches;
    UInt32 lefts;
    MULTIPLAY::CLaunchDescription launch;
    CORE::CString leftReason;

    CLobbyFormRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), launches( 0 ), lefts( 0 ), launch(), leftReason()
    {
        TEventCallback launchCallback( this, &CLobbyFormRecorder::OnLaunch );
        SubscribeTo( form, GAMEGUI::CLobbyForm::LaunchEvent, launchCallback );
        TEventCallback leftCallback( this, &CLobbyFormRecorder::OnLeft );
        SubscribeTo( form, GAMEGUI::CLobbyForm::LeftEvent, leftCallback );
    }

    virtual ~CLobbyFormRecorder() { UnsubscribeAllFromObserver(); }

    void OnLaunch( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++launches;
            launch = *static_cast< MULTIPLAY::CLaunchDescription* >( eventData );
        }
    }

    void OnLeft( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL != eventData )
        {
            ++lefts;
            leftReason = static_cast< GAMEGUI::CLobbyForm::TLeftEventData* >( eventData )->GetData();
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Right click on a row of the slots and choose an entry of the menu that opens */
static bool
ChooseFromSlotMenu( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row, const char* item )
{
    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, x, y );
    ClickAt( context, x, y, GUI::MOUSEBUTTON_RIGHT );
    GUI::CPopupMenu* menu = GAMEGUI::WidgetAs< GUI::CPopupMenu >( context->GetPopup(), "PopupMenu" );
    if ( GUCEF_NULL == menu )
        return false;
    GUI::CPopupMenu::TStringVector items = menu->GetItems();
    for ( UInt32 i=0; i<(UInt32)items.size(); ++i )
    {
        if ( items[ i ] == item )
        {
            context->FlushChanges();
            const GUI::CRect& rect = menu->GetRect();
            ClickAt( context, rect.left + 10, rect.top + (Int32) i * menu->GetRowHeight() + menu->GetRowHeight() / 2 );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static CORE::CString::StringVector
SlotMenuItems( GUI::CGUIContext* context, GUI::CGridView* grid, Int32 row )
{
    CORE::CString::StringVector items;
    Int32 x = 0, y = 0;
    RowPoint( context, grid, row, x, y );
    ClickAt( context, x, y, GUI::MOUSEBUTTON_RIGHT );
    GUI::CPopupMenu* menu = GAMEGUI::WidgetAs< GUI::CPopupMenu >( context->GetPopup(), "PopupMenu" );
    if ( GUCEF_NULL != menu )
    {
        items = menu->GetItems();
        menu->Close();
    }
    return items;
}

/*-------------------------------------------------------------------------*/

void
PerformLobbyFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.Lobby" );

    GUCEF_TESTFW_TESTCASE( "Lobby: The slots, the spectators and the settings of the lobby show, and a player picks faction, colour and ready" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );
        GAMEGUI::CLobbyForm form( context );
        ASSERT_TRUE( form.Open() );
        form.SetLobby( fixture.bob );
        ASSERT_TRUE( fixture.bob.GetPointerAlways() == form.GetLobby().GetPointerAlways() );
        GUI::CGridView* slots = form.GetWidgetAs< GUI::CGridView >( "Slots" );
        ASSERT_TRUE( GUCEF_NULL != slots && 4 == slots->GetRowCount() && 7 == slots->GetColumnCount() );
        if ( GUCEF_NULL == slots )
            return;

        // the title is the name of the lobby; the line above the table tells the lobby waits
        ASSERT_TRUE( "Test lobby" == form.GetRootWidget()->GetProperty( "Title" ) && "Waiting for the players" == form.GetCountdownText() );

        // slot, team, player, faction, colour, ready, latency
        ASSERT_TRUE( "1" == slots->GetText( 0, 0 ) && "1" == slots->GetText( 1, 0 ) && "Alice (boss)" == slots->GetText( 2, 0 ) );
        ASSERT_TRUE( "Bob" == slots->GetText( 2, 1 ) && "Carol" == slots->GetText( 2, 2 ) && "(open)" == slots->GetText( 2, 3 ) );
        ASSERT_TRUE( "4" == slots->GetText( 0, 3 ) && "4" == slots->GetText( 1, 3 ) );
        ASSERT_TRUE( slots->GetText( 5, 1 ).IsNULLOrEmpty() && slots->GetText( 6, 1 ).IsNULLOrEmpty() );
        ASSERT_TRUE( 1 == form.FindRowOfMember( "bob-id" ) && -1 == form.FindRowOfMember( "dave-id" ) );

        // the spectators
        GUI::CListbox* spectators = form.GetWidgetAs< GUI::CListbox >( "Spectators" );
        ASSERT_TRUE( GUCEF_NULL != spectators && 1 == spectators->GetItems().size() && "Dave" == spectators->GetItems()[ 0 ] );

        // latency arrives from the lobby
        fixture.GetHost()->SetMemberLatency( "bob-id", 45 );
        fixture.GetHost()->SetMemberLatency( "dave-id", 80 );
        ASSERT_TRUE( "45 ms" == slots->GetText( 6, 1 ) && "Dave  80 ms" == spectators->GetItems()[ 0 ] );

        // the settings of the lobby, in the embedded panel
        GAMEGUI::CLobbySettingsPanelForm* panel = form.GetSettingsPanel();
        ASSERT_TRUE( GUCEF_NULL != panel && 6 == panel->GetSettingCount() && fixture.bob.GetPointerAlways() == panel->GetLobby().GetPointerAlways() );
        ASSERT_TRUE( panel->GetRootWidget()->GetParentWidget() == form.GetWidget( "SettingsHolder" ) && panel->IsProposalOnly( "speed" ) );

        // faction and colour, from the lists of the schema
        GUI::CCombobox* faction = form.GetWidgetAs< GUI::CCombobox >( "Faction" );
        GUI::CCombobox* colour = form.GetWidgetAs< GUI::CCombobox >( "Colour" );
        GUI::CCombobox::TStringVector items;
        faction->GetListItems( items );
        ASSERT_TRUE( 2 == items.size() && "arm" == items[ 0 ] && "core" == items[ 1 ] && -1 == faction->GetSelectedIndex() );
        items.clear();
        colour->GetListItems( items );
        ASSERT_TRUE( 4 == items.size() && "blue" == items[ 1 ] && -1 == colour->GetSelectedIndex() );
        ASSERT_TRUE( PickItem( context, faction, 1 ) );
        ASSERT_TRUE( "core" == fixture.bob->GetState().FindSlotOfMember( "bob-id" )->faction && "core" == slots->GetText( 3, 1 ) && 1 == faction->GetSelectedIndex() );
        ASSERT_TRUE( PickItem( context, colour, 1 ) );
        ASSERT_TRUE( "blue" == fixture.bob->GetState().FindSlotOfMember( "bob-id" )->colour && "blue" == slots->GetText( 4, 1 ) && 1 == colour->GetSelectedIndex() );

        // another player takes the colour: refused, and the line says so
        fixture.carol->SetColour( "blue" );
        ASSERT_TRUE( "blue" == slots->GetText( 4, 1 ) && slots->GetText( 4, 2 ).IsNULLOrEmpty() );

        // ready: the check box and the table agree, also when another route changes it
        ClickWidget( context, form.GetWidget( "Ready" ) );
        ASSERT_TRUE( fixture.bob->GetState().FindMember( "bob-id" )->ready && "Yes" == slots->GetText( 5, 1 ) && form.GetWidget( "Ready" )->GetBoolProperty( "Checked" ) );
        fixture.bob->SetReady( false );
        ASSERT_TRUE( slots->GetText( 5, 1 ).IsNULLOrEmpty() && !form.GetWidget( "Ready" )->GetBoolProperty( "Checked" ) );
        fixture.carol->SetReady( true );
        ASSERT_TRUE( "Yes" == slots->GetText( 5, 2 ) );

        // spectating: a player becomes a spectator and back
        ClickWidget( context, form.GetWidget( "Spectate" ) );
        ASSERT_TRUE( fixture.bob->GetState().FindMember( "bob-id" )->IsSpectator() && 2 == spectators->GetItems().size() && "(open)" == slots->GetText( 2, 1 ) );
        ASSERT_FALSE( IsWidgetEnabled( form, "Ready" ) || IsWidgetEnabled( form, "Faction" ) );
        ClickWidget( context, form.GetWidget( "Spectate" ) );
        ASSERT_TRUE( !fixture.bob->GetState().FindMember( "bob-id" )->IsSpectator() && 1 == spectators->GetItems().size() );
        ASSERT_TRUE( IsWidgetEnabled( form, "Ready" ) );

        // no lobby: nothing to show and nothing to do
        form.SetLobby( MULTIPLAY::CILobbyPtr() );
        ASSERT_TRUE( 0 == slots->GetRowCount() && form.GetCountdownText().IsNULLOrEmpty() && 0 == spectators->GetItems().size() );
        ASSERT_FALSE( IsWidgetEnabled( form, "Start" ) || IsWidgetEnabled( form, "Leave" ) || IsWidgetEnabled( form, "Ready" ) );
        ASSERT_TRUE( !form.RequestStart() && !form.ForceStart() && !form.Leave() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: The buttons are enabled by the role of the member and the phase of the lobby" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );
        GAMEGUI::CLobbyForm bossForm( context );
        GAMEGUI::CLobbyForm playerForm( context );
        GAMEGUI::CLobbyForm spectatorForm( context );
        ASSERT_TRUE( bossForm.Build() && playerForm.Build() && spectatorForm.Build() );
        bossForm.SetLobby( fixture.alice );
        playerForm.SetLobby( fixture.bob );
        spectatorForm.SetLobby( fixture.dave );

        // an open lobby
        ASSERT_TRUE( IsWidgetEnabled( bossForm, "Start" ) && IsWidgetEnabled( bossForm, "ForceStart" ) && !IsWidgetEnabled( bossForm, "VoteStart" ) );
        ASSERT_TRUE( IsWidgetEnabled( bossForm, "Ready" ) && IsWidgetEnabled( bossForm, "Spectate" ) && IsWidgetEnabled( bossForm, "Faction" ) && IsWidgetEnabled( bossForm, "Colour" ) && IsWidgetEnabled( bossForm, "Leave" ) );
        ASSERT_TRUE( !IsWidgetEnabled( playerForm, "Start" ) && !IsWidgetEnabled( playerForm, "ForceStart" ) && IsWidgetEnabled( playerForm, "VoteStart" ) );
        ASSERT_TRUE( IsWidgetEnabled( playerForm, "Ready" ) && IsWidgetEnabled( playerForm, "Spectate" ) && IsWidgetEnabled( playerForm, "Faction" ) && IsWidgetEnabled( playerForm, "Leave" ) );
        ASSERT_TRUE( !IsWidgetEnabled( spectatorForm, "Start" ) && !IsWidgetEnabled( spectatorForm, "ForceStart" ) && !IsWidgetEnabled( spectatorForm, "VoteStart" ) );
        ASSERT_TRUE( !IsWidgetEnabled( spectatorForm, "Ready" ) && !IsWidgetEnabled( spectatorForm, "Faction" ) && !IsWidgetEnabled( spectatorForm, "Colour" ) );
        ASSERT_TRUE( IsWidgetEnabled( spectatorForm, "Spectate" ) && spectatorForm.GetWidget( "Spectate" )->GetBoolProperty( "Checked" ) && IsWidgetEnabled( spectatorForm, "Leave" ) );

        // a vote that runs: no second vote to start
        ASSERT_TRUE( fixture.AllReady() );
        fixture.bob->RequestStart();
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() && "start" == fixture.alice->GetState().activeVote.what );
        ASSERT_TRUE( !IsWidgetEnabled( playerForm, "VoteStart" ) && IsWidgetEnabled( bossForm, "Start" ) );
        ASSERT_TRUE( fixture.carol->Vote( fixture.alice->GetState().activeVote.id, false ) );
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() && !IsWidgetEnabled( playerForm, "VoteStart" ) );
        ASSERT_TRUE( fixture.alice->Vote( fixture.alice->GetState().activeVote.id, false ) );
        ASSERT_TRUE( !fixture.alice->GetState().activeVote.IsActive() && IsWidgetEnabled( playerForm, "VoteStart" ) );

        // counting down: the start is on its way, the player still can become unready, nothing else changes
        fixture.GetHost()->RequestStart( "alice-id", false );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_COUNTDOWN == fixture.alice->GetState().phase );
        ASSERT_TRUE( !IsWidgetEnabled( bossForm, "Start" ) && !IsWidgetEnabled( bossForm, "ForceStart" ) && !IsWidgetEnabled( playerForm, "VoteStart" ) );
        ASSERT_TRUE( IsWidgetEnabled( bossForm, "Ready" ) && IsWidgetEnabled( playerForm, "Ready" ) );
        ASSERT_TRUE( !IsWidgetEnabled( bossForm, "Spectate" ) && !IsWidgetEnabled( bossForm, "Faction" ) && !IsWidgetEnabled( bossForm, "Colour" ) );
        ASSERT_TRUE( IsWidgetEnabled( bossForm, "Leave" ) );

        // the game runs: the lobby is over for everything but leaving
        PumpFor( 400 );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_IN_GAME == fixture.alice->GetState().phase );
        ASSERT_TRUE( !IsWidgetEnabled( bossForm, "Start" ) && !IsWidgetEnabled( bossForm, "Ready" ) && !IsWidgetEnabled( playerForm, "Ready" ) && !IsWidgetEnabled( bossForm, "Spectate" ) );
        ASSERT_TRUE( IsWidgetEnabled( bossForm, "Leave" ) && IsWidgetEnabled( spectatorForm, "Leave" ) );
        ASSERT_TRUE( "The game is in progress" == bossForm.GetCountdownText() );

        // the boss leaves: the boss passes on, and the new boss has the buttons
        ASSERT_TRUE( fixture.alice->Leave() );
        ASSERT_TRUE( "bob-id" == fixture.bob->GetState().bossId );
        ASSERT_TRUE( !IsWidgetEnabled( bossForm, "Leave" ) || !IsWidgetEnabled( bossForm, "Start" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: Start, force start, vote start, the countdown, its cancellation and the launch" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );
        GAMEGUI::CLobbyForm form( context );
        CLobbyFormRecorder recorder( &form );
        ASSERT_TRUE( form.Open() );
        form.SetLobby( fixture.alice );

        // not everyone is ready: refused, and the line says why
        ClickWidget( context, form.GetWidget( "Start" ) );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_OPEN == fixture.alice->GetState().phase && form.GetStatusText().HasSubstr( "Not allowed", true ) >= 0 );
        ASSERT_TRUE( form.GetStatusText().HasSubstr( "start", true ) >= 0 );

        // the boss is ready with its check box, the others with the lobby
        ClickWidget( context, form.GetWidget( "Ready" ) );
        ASSERT_TRUE( fixture.alice->GetState().FindMember( "alice-id" )->ready );
        ASSERT_TRUE( fixture.bob->SetReady( true ) && fixture.carol->SetReady( true ) );
        ClickWidget( context, form.GetWidget( "Start" ) );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_COUNTDOWN == fixture.alice->GetState().phase );
        ASSERT_TRUE( form.GetStatusText().IsNULLOrEmpty() );
        ASSERT_TRUE( form.GetCountdown() >= 0 && form.GetCountdownText().HasSubstr( "The game starts in", true ) >= 0 );

        // someone becomes unready: the countdown stops
        ASSERT_TRUE( fixture.bob->SetReady( false ) );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_OPEN == fixture.alice->GetState().phase && -1 == form.GetCountdown() && "Waiting for the players" == form.GetCountdownText() );
        ASSERT_TRUE( IsWidgetEnabled( form, "Start" ) && 0 == recorder.launches );

        // a force start does not wait for the readiness
        ClickWidget( context, form.GetWidget( "ForceStart" ) );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_COUNTDOWN == fixture.alice->GetState().phase && form.GetCountdownText().HasSubstr( "starts in", true ) >= 0 );

        // and the launch description arrives, with the members and the token of the boss
        PumpFor( 400 );
        ASSERT_TRUE( 1 == recorder.launches && MULTIPLAY::LOBBYPHASE_IN_GAME == fixture.alice->GetState().phase );
        ASSERT_TRUE( "alice-id" == recorder.launch.localMemberId && "testgame" == recorder.launch.gameId && 4 == recorder.launch.members.size() && !recorder.launch.token.IsNULLOrEmpty() );
        ASSERT_TRUE( "The game is in progress" == form.GetCountdownText() && !recorder.launch.spectator );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: A player starts a vote to start, and the vote shows to everyone" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );
        GAMEGUI::CLobbyForm playerForm( context );
        GAMEGUI::CLobbyForm otherForm( context );
        ASSERT_TRUE( playerForm.Open() );
        ASSERT_TRUE( otherForm.Build() );
        playerForm.SetLobby( fixture.bob );
        otherForm.SetLobby( fixture.carol );
        ASSERT_TRUE( playerForm.GetVoteText().IsNULLOrEmpty() );
        ASSERT_TRUE( fixture.AllReady() );

        ClickWidget( context, playerForm.GetWidget( "VoteStart" ) );
        ASSERT_TRUE( fixture.alice->GetState().activeVote.IsActive() && "start" == fixture.alice->GetState().activeVote.what && "bob-id" == fixture.alice->GetState().activeVote.proposer );
        ASSERT_TRUE( "Vote: Bob proposes to start the game  (Yes 1  No 0  (2 needed))" == playerForm.GetVoteText() );
        ASSERT_TRUE( playerForm.GetVoteText() == otherForm.GetVoteText() );
        ASSERT_TRUE( !IsWidgetEnabled( playerForm, "VoteStart" ) && !IsWidgetEnabled( otherForm, "VoteStart" ) );

        // the proposer does not get a prompt, the other players do
        ASSERT_FALSE( playerForm.GetVotePrompt()->IsOpen() );
        ASSERT_TRUE( otherForm.GetVotePrompt()->IsOpen() && otherForm.GetVotePrompt()->IsVisible() );

        // the other player says yes: the vote passes and the countdown runs
        ASSERT_TRUE( otherForm.GetVotePrompt()->Cast( true ) );
        ASSERT_TRUE( playerForm.GetVoteText().IsNULLOrEmpty() && !otherForm.GetVotePrompt()->IsOpen() );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_COUNTDOWN == fixture.alice->GetState().phase && playerForm.GetCountdownText().HasSubstr( "starts in", true ) >= 0 );
        PumpFor( 400 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: A free slot is taken with a double click, the boss opens, closes and fills slots, kicks and passes the lead" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );

        // a player moves to the free slot
        {
            GAMEGUI::CLobbyForm playerForm( context );
            ASSERT_TRUE( playerForm.Open() );
            playerForm.SetLobby( fixture.bob );
            GUI::CGridView* slots = playerForm.GetWidgetAs< GUI::CGridView >( "Slots" );
            ASSERT_TRUE( 1 == playerForm.FindRowOfMember( "bob-id" ) );
            DoubleClickRow( context, slots, 3 );
            ASSERT_TRUE( 3 == playerForm.FindRowOfMember( "bob-id" ) && "Bob" == slots->GetText( 2, 3 ) && "(open)" == slots->GetText( 2, 1 ) );

            // a slot of another player is not taken
            DoubleClickRow( context, slots, 2 );
            ASSERT_TRUE( 3 == playerForm.FindRowOfMember( "bob-id" ) && "Carol" == slots->GetText( 2, 2 ) );

            // the menu is the boss's
            Int32 x = 0, y = 0;
            RowPoint( context, slots, 1, x, y );
            ClickAt( context, x, y, GUI::MOUSEBUTTON_RIGHT );
            ASSERT_TRUE( GUCEF_NULL == context->GetPopup() );
        }

        GAMEGUI::CLobbyForm bossForm( context );
        ASSERT_TRUE( bossForm.Open() );
        bossForm.SetLobby( fixture.alice );
        GUI::CGridView* slots = bossForm.GetWidgetAs< GUI::CGridView >( "Slots" );
        ASSERT_TRUE( GUCEF_NULL != slots && "(open)" == slots->GetText( 2, 1 ) );

        // an open slot: close it or give it an AI
        CORE::CString::StringVector items = SlotMenuItems( context, slots, 1 );
        ASSERT_TRUE( 2 == items.size() && "Close slot" == items[ 0 ] && "Add AI" == items[ 1 ] );
        ASSERT_TRUE( ChooseFromSlotMenu( context, slots, 1, "Close slot" ) );
        ASSERT_TRUE( MULTIPLAY::LOBBYSLOTKIND_CLOSED == fixture.alice->GetState().FindSlot( 1 )->kind && "(closed)" == slots->GetText( 2, 1 ) );

        // a closed slot: open it, or an AI
        items = SlotMenuItems( context, slots, 1 );
        ASSERT_TRUE( 2 == items.size() && "Open slot" == items[ 0 ] && "Add AI" == items[ 1 ] );
        bossForm.SetAiType( "easy" );
        ASSERT_TRUE( "easy" == bossForm.GetAiType() );
        ASSERT_TRUE( ChooseFromSlotMenu( context, slots, 1, "Add AI" ) );
        ASSERT_TRUE( MULTIPLAY::LOBBYSLOTKIND_AI == fixture.alice->GetState().FindSlot( 1 )->kind && "easy" == fixture.alice->GetState().FindSlot( 1 )->aiType && "AI: easy" == slots->GetText( 2, 1 ) );
        items = SlotMenuItems( context, slots, 1 );
        ASSERT_TRUE( 2 == items.size() && "Open slot" == items[ 0 ] && "Close slot" == items[ 1 ] );
        ASSERT_TRUE( ChooseFromSlotMenu( context, slots, 1, "Open slot" ) );
        ASSERT_TRUE( "(open)" == slots->GetText( 2, 1 ) );

        // a slot with a player: kick, ban, pass the lead or report; the boss's own slot has no menu
        items = SlotMenuItems( context, slots, 2 );
        ASSERT_TRUE( 4 == items.size() && "Kick" == items[ 0 ] && "Kick and ban" == items[ 1 ] && "Make boss" == items[ 2 ] && "Report player" == items[ 3 ] );
        ASSERT_TRUE( SlotMenuItems( context, slots, 0 ).empty() );
        bossForm.SetReportReason( "rude" );
        ASSERT_TRUE( "rude" == bossForm.GetReportReason() );
        ASSERT_TRUE( ChooseFromSlotMenu( context, slots, 2, "Report player" ) );
        CTestLobby* aliceLobby = static_cast< CTestLobby* >( fixture.alice.GetPointerAlways() );
        ASSERT_TRUE( "carol-id" == aliceLobby->lastReportedId && "rude" == aliceLobby->lastReportReason );
        ASSERT_TRUE( "Reported Carol" == bossForm.GetStatusText() );
        ASSERT_TRUE( ChooseFromSlotMenu( context, slots, 2, "Kick" ) );
        ASSERT_TRUE( GUCEF_NULL == fixture.alice->GetState().FindMember( "carol-id" ) && "(open)" == slots->GetText( 2, 2 ) );
        ASSERT_TRUE( ChooseFromSlotMenu( context, slots, 3, "Make boss" ) );
        ASSERT_TRUE( "bob-id" == fixture.alice->GetState().bossId && "Bob (boss)" == slots->GetText( 2, 3 ) );
        ASSERT_FALSE( IsWidgetEnabled( bossForm, "Start" ) );
        ASSERT_TRUE( IsWidgetEnabled( bossForm, "VoteStart" ) );
        // no longer the boss: the menu only reports
        items = SlotMenuItems( context, slots, 3 );
        ASSERT_TRUE( 1 == items.size() && "Report player" == items[ 0 ] );
        ASSERT_TRUE( SlotMenuItems( context, slots, 0 ).empty() && SlotMenuItems( context, slots, 1 ).empty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: Leaving is notified, a closed lobby is notified with the reason" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        fixture.rules.closeWhenHostLeaves = true;
        ASSERT_TRUE( fixture.Setup() );
        GAMEGUI::CLobbyForm bobForm( context );
        GAMEGUI::CLobbyForm aliceForm( context );
        CLobbyFormRecorder bobRecorder( &bobForm );
        CLobbyFormRecorder aliceRecorder( &aliceForm );
        ASSERT_TRUE( bobForm.Open() && aliceForm.Build() );
        bobForm.SetLobby( fixture.bob );
        aliceForm.SetLobby( fixture.alice );

        // a player leaves
        ClickWidget( context, bobForm.GetWidget( "Leave" ) );
        ASSERT_TRUE( 1 == bobRecorder.lefts && bobRecorder.leftReason.IsNULLOrEmpty() );
        ASSERT_TRUE( GUCEF_NULL == fixture.alice->GetState().FindMember( "bob-id" ) && 0 == aliceRecorder.lefts );
        bobForm.SetLobby( MULTIPLAY::CILobbyPtr() );
        ASSERT_FALSE( IsWidgetEnabled( bobForm, "Ready" ) || IsWidgetEnabled( bobForm, "Start" ) || IsWidgetEnabled( bobForm, "Leave" ) );

        // the host leaves a lobby that closes with it
        ASSERT_TRUE( aliceForm.Open() );
        fixture.carol->Leave();
        ASSERT_TRUE( aliceForm.Leave() );
        ASSERT_TRUE( 1 == aliceRecorder.lefts );
        ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_CLOSED == fixture.alice->GetState().phase );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: The lobby that closes under a member tells the member why" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        fixture.rules.closeWhenHostLeaves = true;
        ASSERT_TRUE( fixture.Setup() );
        GAMEGUI::CLobbyForm bobForm( context );
        CLobbyFormRecorder recorder( &bobForm );
        ASSERT_TRUE( bobForm.Open() );
        bobForm.SetLobby( fixture.bob );

        ASSERT_TRUE( fixture.alice->Leave() );
        ASSERT_TRUE( 1 == recorder.lefts && !recorder.leftReason.IsNULLOrEmpty() );
        ASSERT_TRUE( bobForm.GetStatusText() == "The lobby closed: " + recorder.leftReason );
        ASSERT_TRUE( "The lobby is closed" == bobForm.GetCountdownText() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Lobby: The chat of the lobby is the channel the lobby names, on the chat service given" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CLobbyFixture fixture;
        ASSERT_TRUE( fixture.Setup() );

        CTestChatService aliceChat;
        CTestChatService bobChat;
        MULTIPLAY::CPlayerIdentity aliceIdentity;
        aliceIdentity.userId = "alice-id";
        aliceIdentity.displayName = "Alice";
        MULTIPLAY::CPlayerIdentity bobIdentity;
        bobIdentity.userId = "bob-id";
        bobIdentity.displayName = "Bob";
        CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
        ASSERT_TRUE( aliceChat.Connect( aliceIdentity, future ) && bobChat.Connect( bobIdentity, future ) );

        GAMEGUI::CLobbyForm aliceForm( context );
        GAMEGUI::CLobbyForm bobForm( context );
        ASSERT_TRUE( aliceForm.Open() && bobForm.Build() );
        aliceForm.SetChatService( &aliceChat );
        bobForm.SetChatService( &bobChat );
        aliceForm.SetLobby( fixture.alice );
        bobForm.SetLobby( fixture.bob );
        ASSERT_TRUE( &aliceChat == aliceForm.GetChatService() );

        // a lobby without a chat has no chat on show
        ASSERT_FALSE( aliceForm.GetWidget( "ChatHolder" )->IsVisible() );

        // the lobby names its chat: both forms join the channel
        fixture.GetHost()->SetChat( "test-chat", "lobby-1" );
        ASSERT_TRUE( aliceForm.GetWidget( "ChatHolder" )->IsVisible() && "lobby-1" == aliceForm.GetChatForm()->GetSingleChannel() );
        ASSERT_TRUE( 1 == aliceForm.GetChatForm()->GetTabCount() && "lobby-1" == aliceForm.GetChatForm()->GetTabName( 0 ) );
        ASSERT_TRUE( GUCEF_NULL != aliceChat.GetChannel( "lobby-1" ) && GUCEF_NULL != bobChat.GetChannel( "lobby-1" ) );
        ASSERT_TRUE( 2 == aliceChat.GetChannel( "lobby-1" )->GetMembers().size() );

        // a line typed by Alice shows for both
        TypeInto( context, aliceForm.GetChatForm()->GetWidget( "Input" ), "Hello Bob", true );
        ASSERT_TRUE( AnyLineHas( aliceForm.GetChatForm()->GetShownLines(), "Alice: Hello Bob" ) );
        ASSERT_TRUE( AnyLineHas( bobForm.GetChatForm()->GetShownLines(), "Alice: Hello Bob" ) );
        ASSERT_TRUE( bobForm.GetChatForm()->SendText( "Hi Alice" ) );
        ASSERT_TRUE( AnyLineHas( aliceForm.GetChatForm()->GetShownLines(), "Bob: Hi Alice" ) );

        // the chat form of a lobby is the one channel
        ASSERT_TRUE( GUCEF_NULL == aliceForm.GetChatForm()->GetWidget( "ChatTabs" ) && GUCEF_NULL == aliceForm.GetChatForm()->GetWidget( "Close" ) );

        // and no chat again
        fixture.GetHost()->SetChat( "", "" );
        ASSERT_FALSE( aliceForm.GetWidget( "ChatHolder" )->IsVisible() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
