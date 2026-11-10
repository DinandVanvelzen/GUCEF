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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "gucefGAMEGUI_TestCommon.h"
#include "TestListingProvider.h"
#include "TestLobbyFixture.h"
#include "TestFakeAsyncLobbyBackend.h"
#include "gucefMULTIPLAY_CAsyncLobbyListingProvider.h"
#include "gucefMULTIPLAY_CGameBrowser.h"
#include "TestAsyncLobbyForms.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records the events of the list of games and of the lobby form and of the form that creates a lobby */
class CAsyncFormRecorder : public CORE::CObserver
{
    public:

    typedef CORE::CTEventHandlerFunctor< CAsyncFormRecorder > TEventCallback;

    UInt32 joins;
    UInt32 spectates;
    UInt32 creates;
    UInt32 settings;
    UInt32 directs;
    UInt32 lefts;
    UInt32 backs;
    UInt32 createRequests;
    UInt32 cancels;
    UInt32 filterChanges;
    CORE::CString lastGroup;
    MULTIPLAY::CGameListingEntry lastJoin;
    GAMEGUI::CCreateLobbyRequest lastRequest;

    CAsyncFormRecorder( void )
        : CORE::CObserver(), joins( 0 ), spectates( 0 ), creates( 0 ), settings( 0 ), directs( 0 ), lefts( 0 ), backs( 0 ), createRequests( 0 ), cancels( 0 ), filterChanges( 0 ), lastGroup(), lastJoin(), lastRequest()
    {
    }

    virtual ~CAsyncFormRecorder() { UnsubscribeAllFromObserver(); }

    void WatchBrowser( CORE::CNotifier* form )
    {
        TEventCallback join( this, &CAsyncFormRecorder::OnJoin );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::JoinRequestedEvent, join );
        TEventCallback spectate( this, &CAsyncFormRecorder::OnSpectate );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::SpectateRequestedEvent, spectate );
        TEventCallback create( this, &CAsyncFormRecorder::OnCreate );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::CreateRequestedEvent, create );
        TEventCallback direct( this, &CAsyncFormRecorder::OnDirect );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::DirectConnectRequestedEvent, direct );
        TEventCallback setting( this, &CAsyncFormRecorder::OnSettings );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::SettingsRequestedEvent, setting );
        TEventCallback filter( this, &CAsyncFormRecorder::OnFilterChanged );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::FilterChangedEvent, filter );
    }

    void WatchLobby( CORE::CNotifier* form )
    {
        TEventCallback left( this, &CAsyncFormRecorder::OnLeft );
        SubscribeTo( form, GAMEGUI::CLobbyForm::LeftEvent, left );
        TEventCallback back( this, &CAsyncFormRecorder::OnBack );
        SubscribeTo( form, GAMEGUI::CLobbyForm::BackRequestedEvent, back );
    }

    void WatchCreate( CORE::CNotifier* form )
    {
        TEventCallback request( this, &CAsyncFormRecorder::OnCreateRequest );
        SubscribeTo( form, GAMEGUI::CCreateLobbyForm::CreateRequestedEvent, request );
        TEventCallback cancel( this, &CAsyncFormRecorder::OnCancel );
        SubscribeTo( form, GAMEGUI::CCreateLobbyForm::CancelledEvent, cancel );
    }

    void OnJoin( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++joins; lastJoin = *static_cast< MULTIPLAY::CGameListingEntry* >( eventData ); }
    void OnSpectate( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++spectates; }
    void OnCreate( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++creates; }
    void OnDirect( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++directs; }
    void OnSettings( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++settings; }
    void OnFilterChanged( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++filterChanges; lastGroup = static_cast< CORE::TCloneableString* >( eventData )->GetData(); }
    void OnLeft( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++lefts; }
    void OnBack( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++backs; }
    void OnCancel( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++cancels; }
    void OnCreateRequest( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData ) { ++createRequests; lastRequest = *static_cast< GAMEGUI::CCreateLobbyRequest* >( eventData ); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static GAMEGUI::CServerBrowserCapabilities
MakeMailBrowserCapabilities( void )
{
    GAMEGUI::CServerBrowserCapabilities caps;
    caps.showPing = false;
    caps.showLock = false;
    caps.showFullFilter = false;
    caps.showDirectConnect = false;
    caps.showSettings = true;
    caps.showDetail = true;
    caps.showNote = true;
    caps.title = "E-mail games";
    caps.kindColumnTitle = "Status";
    caps.mapColumnTitle = "Galaxy";
    caps.modeColumnTitle = "Decisions";
    caps.sourceColumnTitle = "Host";
    caps.detailColumnTitle = "Turn";
    caps.createText = "New game";
    caps.settingsText = "Mailbox";
    caps.columnWidths.resize( GAMEGUI::CServerBrowserForm::COLUMN_COUNT, 0 );
    caps.columnWidths[ GAMEGUI::CServerBrowserForm::COLUMN_NAME ] = 230;
    caps.columnWidths[ GAMEGUI::CServerBrowserForm::COLUMN_KIND ] = 125;
    caps.columnWidths[ GAMEGUI::CServerBrowserForm::COLUMN_MODE ] = 145;
    caps.columnWidths[ GAMEGUI::CServerBrowserForm::COLUMN_DETAIL ] = 250;
    return caps;
}

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CGameListingEntry
MakeMailRow( const char* id, const char* name, MULTIPLAY::CGameListingEntry::TKind kind, UInt32 players, UInt32 maxPlayers, const char* status, const char* detail, const char* joinText, bool canJoin, bool canSpectate )
{
    MULTIPLAY::CGameListingEntry row;
    row.listingId = id;
    row.name = name;
    row.kind = kind;
    row.players = players;
    row.maxPlayers = maxPlayers;
    row.mapName = "Medium galaxy";
    row.modeName = "Vote";
    row.hostName = "Alice";
    row.backendName = "mail";
    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraStatusText, status );
    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraDetailText, detail );
    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraJoinText, joinText );
    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraCanJoin, canJoin ? "true" : "false" );
    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraCanSpectate, canSpectate ? "true" : "false" );
    return row;
}

/*-------------------------------------------------------------------------*/

/** A lobby of the host in the draft stage: the host, one who introduced themselves and one who has not answered */
static MULTIPLAY::CAsyncLobbyView
MakeDraftView( const char* id )
{
    MULTIPLAY::CAsyncLobbyView view;
    view.state.lobbyId = id;
    view.state.name = "Round one";
    view.state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
    view.state.bossId = "me";
    view.localMemberId = "me";
    const char* ids[ 3 ] = { "me", "bob", "carol" };
    const char* names[ 3 ] = { "Me", "Bob", "Carol" };
    const char* texts[ 3 ] = { "Host", "Introduced", "Waiting for an answer" };
    for ( int i=0; i<3; ++i )
    {
        MULTIPLAY::CLobbyMember member;
        member.id = ids[ i ];
        member.name = names[ i ];
        member.slot = i;
        member.ready = i < 2;
        member.statusText = texts[ i ];
        view.state.members.push_back( member );
        MULTIPLAY::CLobbySlot slot;
        slot.index = (UInt32) i;
        slot.team = (UInt32) i + 1;
        slot.kind = MULTIPLAY::LOBBYSLOTKIND_HUMAN;
        slot.memberId = ids[ i ];
        view.state.slots.push_back( slot );
    }
    MULTIPLAY::CLobbySettingDef policy;
    policy.key = "mail.policy";
    policy.label = "How the table decides";
    policy.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
    policy.options.push_back( "VOTE" );
    policy.options.push_back( "BOSS" );
    policy.defaultValue = "VOTE";
    policy.changePolicy = MULTIPLAY::CLobbySettingDef::CHANGEPOLICY_ANY;
    view.schema.AddSetting( policy );
    view.state.SetSetting( "mail.policy", "VOTE" );
    MULTIPLAY::CLobbyPresentation& p = view.presentation;
    p.showLatency = false;
    p.showFactionAndColour = false;
    p.showForceStart = false;
    p.showVoteStart = false;
    p.showSpectate = false;
    p.showInvite = true;
    p.showReady = false;
    p.settingsEditable = false;
    p.readyText = "Accept";
    p.readyColumnTitle = "Answer";
    p.leaveText = "Cancel game";
    p.kickText = "Remove";
    p.note = "Nobody can vote yet: until everybody has answered, the host alone decides who plays and when the game starts.";
    p.canStart = true;
    p.canLeave = true;
    p.canInvite = true;
    p.canKick = true;
    return view;
}

/*-------------------------------------------------------------------------*/

/** The lobby of an invited player: the host, the player (who has not answered) */
static MULTIPLAY::CAsyncLobbyView
MakeInvitedView( const char* id )
{
    MULTIPLAY::CAsyncLobbyView view = MakeDraftView( id );
    view.localMemberId = "bob";
    view.state.bossId = "me";
    view.state.members[ 1 ].ready = false;
    view.state.members[ 1 ].statusText = "Invited";
    MULTIPLAY::CLobbyPresentation& p = view.presentation;
    p.showInvite = false;
    p.showStart = false;
    p.showReady = true;
    p.showSpectate = true;
    p.leaveText = "Decline";
    p.note = "Accept to play. You can also watch the game without playing.";
    p.showBack = true;
    p.backText = "Back to the list";
    p.canStart = false;
    p.canInvite = false;
    p.canKick = false;
    p.canReady = true;
    p.canSpectate = true;
    return view;
}

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CILobbyPtr
OpenFakeLobby( CFakeAsyncLobbyBackend& backend, MULTIPLAY::CAsyncLobbyService& service, const char* id )
{
    MULTIPLAY::CPlayerIdentity identity;
    identity.userId = "me";
    CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
    service.Connect( identity, future );
    return service.OpenLobby( id );
}

/*-------------------------------------------------------------------------*/

void
PerformAsyncLobbyFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.AsyncLobbyForms" );

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The list of games without ping, lock and direct connect, with the status in words, a detail column and a settings button" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        TTestListingProviderFactory factory;
        MULTIPLAY::CGameBrowser browser;
        browser.SetPingEnabled( false );
        MULTIPLAY::CIGameListingProviderPtr providerPtr = factory.Create();
        CTestListingProvider* provider = static_cast< CTestListingProvider* >( providerPtr.GetPointerAlways() );
        browser.AddProvider( providerPtr );
        browser.Start();
        provider->Put( MakeMailRow( "a", "Round one", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 2, 3, "Invited", "2 answers missing", "Join", true, true ) );
        provider->Put( MakeMailRow( "b", "Long game", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS, 3, 3, "Your turn", "11 h 40 min left", "Open", true, false ) );
        provider->Put( MakeMailRow( "c", "Old game", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 1, 3, "Cancelled", "", "Open", true, false ) );

        GAMEGUI::CServerBrowserForm form( context );
        CAsyncFormRecorder recorder;
        recorder.WatchBrowser( &form );
        form.SetCapabilities( MakeMailBrowserCapabilities() );
        form.SetBrowser( &browser );
        ASSERT_TRUE( form.Open() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid );
        if ( GUCEF_NULL == grid )
            return;

        // seven columns, no ping, no lock
        CORE::CString::StringVector titles = grid->GetStringListProperty( GUI::EWidgetProperty_Columns );
        ASSERT_TRUE( 7 == titles.size() && 7 == grid->GetColumnCount() );
        ASSERT_TRUE( 7 == titles.size() && "Name" == titles[ 0 ] && "Status" == titles[ 1 ] && "Players" == titles[ 2 ] && "Galaxy" == titles[ 3 ] && "Decisions" == titles[ 4 ] && "Host" == titles[ 5 ] && "Turn" == titles[ 6 ] );
        GUCEF::vector< Int32 > shown;
        form.GetShownColumns( shown );
        ASSERT_TRUE( 7 == shown.size() && !form.IsColumnShown( GAMEGUI::CServerBrowserForm::COLUMN_PING ) && !form.IsColumnShown( GAMEGUI::CServerBrowserForm::COLUMN_LOCK ) );

        // the widths of the columns are the capabilities' when it gives some (a source with longer texts), the usual ones for the others
        GUCEF::vector< Int32 > widths;
        form.GetColumnWidths( widths );
        ASSERT_TRUE( 7 == widths.size() && 230 == widths[ 0 ] && 125 == widths[ 1 ] && 145 == widths[ 4 ] && 250 == widths[ 6 ] && 90 == widths[ 2 ] );
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "MaxPing" ) && GUCEF_NULL == form.GetWidget( "HideLocked" ) && GUCEF_NULL == form.GetWidget( "HideFull" ) && GUCEF_NULL == form.GetWidget( "DirectConnect" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Settings" ) && "Mailbox" == form.GetWidget( "Settings" )->GetProperty( "Text" ) && "New game" == form.GetWidget( "Create" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "E-mail games" == form.GetRootWidget()->GetProperty( GUI::EWidgetProperty_Title ) );

        // the note of the source: in a line of its own, a problem stands out, and it stays when the table is made again
        form.SetNote( "No mailbox is set up yet.", true );
        ASSERT_TRUE( "No mailbox is set up yet." == form.GetNoteText() );
        form.SetNote( "The mailbox is set up.", false );
        ASSERT_TRUE( "The mailbox is set up." == form.GetNoteText() && form.GetWidget( "Note" )->GetRect().Bottom() <= grid->GetRect().top );

        // the cells: the status is the words of the listing, the detail its detail (by name to begin with: Long game, Old game, Round one)
        ASSERT_TRUE( 3 == grid->GetRowCount() );
        ASSERT_TRUE( "Round one" == grid->GetText( 0, 2 ) && "Invited" == grid->GetText( 1, 2 ) && "2/3" == grid->GetText( 2, 2 ) && "Medium galaxy" == grid->GetText( 3, 2 ) && "Vote" == grid->GetText( 4, 2 ) && "Alice" == grid->GetText( 5, 2 ) && "2 answers missing" == grid->GetText( 6, 2 ) );
        ASSERT_TRUE( "Long game" == grid->GetText( 0, 0 ) && "Your turn" == grid->GetText( 1, 0 ) && "11 h 40 min left" == grid->GetText( 6, 0 ) );
        ASSERT_TRUE( "Old game" == grid->GetText( 0, 1 ) && "Cancelled" == grid->GetText( 1, 1 ) );

        // the join button says what it does for the row and works for a game in progress; the row says whether it can be watched
        ClickRow( context, grid, 2 );
        ASSERT_TRUE( "Join" == form.GetWidget( "Join" )->GetProperty( "Text" ) && IsWidgetEnabled( form, "Join" ) && IsWidgetEnabled( form, "Spectate" ) );
        ClickRow( context, grid, 0 );
        ASSERT_TRUE( "Open" == form.GetWidget( "Join" )->GetProperty( "Text" ) && IsWidgetEnabled( form, "Join" ) && !IsWidgetEnabled( form, "Spectate" ) );
        ClickWidget( context, form.GetWidget( "Join" ) );
        ASSERT_TRUE( 1 == recorder.joins && "b" == recorder.lastJoin.listingId );
        ClickWidget( context, form.GetWidget( "Settings" ) );
        ASSERT_TRUE( 1 == recorder.settings );
        ClickWidget( context, form.GetWidget( "Create" ) );
        ASSERT_TRUE( 1 == recorder.creates );

        // a click on the title of the status column sorts by the words of the status, the sort column is the status
        ClickColumnTitle( context, grid, 1 );
        ASSERT_TRUE( GAMEGUI::CServerBrowserForm::COLUMN_KIND == form.GetSortColumn() && form.IsSortAscending() );
        ASSERT_TRUE( "Cancelled" == grid->GetText( 1, 0 ) && "Invited" == grid->GetText( 1, 1 ) && "Your turn" == grid->GetText( 1, 2 ) );
        ClickColumnTitle( context, grid, 6 );
        ASSERT_TRUE( GAMEGUI::CServerBrowserForm::COLUMN_DETAIL == form.GetSortColumn() );

        // the filter that was typed stays when the capabilities change, and the table has all its columns again
        MULTIPLAY::CGameListingFilter filter;
        filter.text = "game";
        form.SetFilter( filter );
        ASSERT_TRUE( 2 == grid->GetRowCount() );
        form.SetCapabilities( GAMEGUI::CServerBrowserCapabilities() );
        grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid && 8 == grid->GetColumnCount() && 2 == grid->GetRowCount() && "game" == form.GetFilter().text );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "MaxPing" ) && GUCEF_NULL != form.GetWidget( "DirectConnect" ) && GUCEF_NULL == form.GetWidget( "Settings" ) );
        ASSERT_TRUE( "game" == form.GetWidget( "FilterText" )->GetProperty( "Text" ) );
        browser.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: A list of games played over time puts the games where it is the player's turn first and makes them stand out, filters on them with one click or on the other groups with the same control, counts them in the status line and keeps the order of the player when a column is clicked" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        TTestListingProviderFactory factory;
        MULTIPLAY::CGameBrowser browser;
        browser.SetPingEnabled( false );
        MULTIPLAY::CIGameListingProviderPtr providerPtr = factory.Create();
        CTestListingProvider* provider = static_cast< CTestListingProvider* >( providerPtr.GetPointerAlways() );
        browser.AddProvider( providerPtr );
        browser.Start();

        // 60 games: ten in each group, in the order of the turn, waiting, forming, finished, spectating and a second group of turn, the turns end at different times and two at the same time
        const char* groups[ 5 ] = { "turn", "waiting", "forming", "finished", "spectating" };
        GUCEF::vector< MULTIPLAY::CGameListingEntry > rows;
        for ( UInt32 i=0; i<60; ++i )
        {
            const UInt32 g = i % 5;
            char name[ 32 ];
            sprintf( name, "Game %02u", (unsigned) i );
            MULTIPLAY::CGameListingEntry row = MakeMailRow( ( CORE::CString( "g" ) + CORE::UInt32ToString( i ) ).C_String(), name, 0 == g || 1 == g ? MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS : MULTIPLAY::CGameListingEntry::KIND_LOBBY, 3, 3,
                                                           0 == g ? "Your turn" : ( 1 == g ? "Their turn" : "Forming" ), "", "Open", true, false );
            row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraGroup, groups[ g ] );
            if ( 0 == g )
            {
                row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraAttention, "true" );
                // game 00 and game 55 end at the same time; the later games end earlier: 55 first, then 50, 45, ...
                const UInt32 slot = 55 == i ? 0 : ( 11 - i / 5 );
                row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraDeadline, CORE::UInt32ToString( 1000000 + 1000 * slot ) );
            }
            rows.push_back( row );
        }
        provider->PutBatch( rows );

        GAMEGUI::CServerBrowserCapabilities caps = MakeMailBrowserCapabilities();
        caps.viewLabels.push_back( "All" );                    caps.viewKeys.push_back( "" );
        caps.viewLabels.push_back( "Your turn" );              caps.viewKeys.push_back( "turn" );
        caps.viewLabels.push_back( "Waiting for others" );     caps.viewKeys.push_back( "waiting" );
        caps.viewLabels.push_back( "Forming and invitations" ); caps.viewKeys.push_back( "forming" );
        caps.viewLabels.push_back( "Finished and cancelled" ); caps.viewKeys.push_back( "finished" );
        caps.viewLabels.push_back( "Spectating" );             caps.viewKeys.push_back( "spectating" );
        caps.quickFilterKey = "turn";
        caps.quickFilterText = "Your turn";
        caps.attentionText = "your turn";
        caps.defaultOrderByAttention = true;
        GAMEGUI::CServerBrowserForm form( context );
        CAsyncFormRecorder recorder;
        recorder.WatchBrowser( &form );
        form.SetCapabilities( caps );
        form.SetBrowser( &browser );
        ASSERT_TRUE( form.Open() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid );
        if ( GUCEF_NULL == grid )
            return;

        // the control has the groups as its options, and a one click filter for the first of them is next to it
        GUI::CCombobox* view = form.GetWidgetAs< GUI::CCombobox >( "FilterKind" );
        GUI::CCombobox::TStringVector viewItems;
        ASSERT_TRUE( GUCEF_NULL != view && view->GetListItems( viewItems ) && 6 == viewItems.size() && "Your turn" == viewItems[ 1 ] && "Spectating" == viewItems[ 5 ] );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "FilterQuick" ) && "Your turn (12)" == form.GetWidget( "FilterQuick" )->GetProperty( "Text" ) && !form.GetWidget( "FilterQuick" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        ASSERT_TRUE( form.GetWidget( "FilterText" )->GetRect().Right() <= form.GetWidget( "FilterQuick" )->GetRect().left && form.GetWidget( "FilterQuick" )->GetRect().Right() <= view->GetRect().left );

        // the count in the status line: how many games, how many of them are the player's turn
        ASSERT_TRUE( "60 games, 12 your turn" == form.GetWidget( "Status" )->GetProperty( "Text" ) );

        // the order the list starts with: the games of the player's turn first, the nearest end of the turn first, equal ends by name; then the rest by name
        ASSERT_TRUE( -1 == form.GetSortColumn() && 60 == grid->GetRowCount() );
        ASSERT_TRUE( "Game 55" == grid->GetText( 0, 0 ) && "Game 50" == grid->GetText( 0, 1 ) && "Game 45" == grid->GetText( 0, 2 ) && "Game 05" == grid->GetText( 0, 10 ) && "Game 00" == grid->GetText( 0, 11 ) );
        ASSERT_TRUE( "Game 01" == grid->GetText( 0, 12 ) && "Game 59" == grid->GetText( 0, 59 ) );
        // the rows of the player's turn are emphasized, the others are not
        GUI::CGridView::TInt32Vector emphasized = grid->GetEmphasizedRows();
        ASSERT_TRUE( 12 == emphasized.size() && 0 == emphasized.front() && 11 == emphasized.back() );

        // one click: only the games where it is the player's turn; the box is checked, the option is chosen, the count says it, the filter is told to the application
        ClickWidget( context, form.GetWidget( "FilterQuick" ) );
        ASSERT_TRUE( form.GetWidget( "FilterQuick" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) && 1 == view->GetSelectedIndex() && "turn" == form.GetFilter().group );
        ASSERT_TRUE( 12 == grid->GetRowCount() );
        ASSERT_TRUE( 12 == grid->GetEmphasizedRows().size() );
        ASSERT_TRUE( "12 games, 12 your turn (48 hidden by the filter)" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 1 == recorder.filterChanges && "turn" == recorder.lastGroup );
        for ( UInt32 r=0; r<12; ++r )
            ASSERT_TRUE( "Your turn" == grid->GetText( 1, r ) );
        // the same filter through the control: the box follows
        ASSERT_TRUE( PickItem( context, view, 2 ) );
        ASSERT_TRUE( !form.GetWidget( "FilterQuick" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) && "waiting" == form.GetFilter().group && 12 == grid->GetRowCount() && 0 == grid->GetEmphasizedRows().size() );
        ASSERT_TRUE( "12 games (48 hidden by the filter)" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "waiting" == recorder.lastGroup );
        ASSERT_TRUE( PickItem( context, view, 5 ) );
        ASSERT_TRUE( 12 == grid->GetRowCount() && "spectating" == form.GetFilter().group );
        // back to all with the box: unchecking after checking
        ClickWidget( context, form.GetWidget( "FilterQuick" ) );
        ASSERT_TRUE( 12 == grid->GetRowCount() && "turn" == form.GetFilter().group );
        ClickWidget( context, form.GetWidget( "FilterQuick" ) );
        ASSERT_TRUE( 60 == grid->GetRowCount() );
        ASSERT_TRUE( form.GetFilter().group.IsNULLOrEmpty() );
        ASSERT_TRUE( recorder.lastGroup.IsNULLOrEmpty() );

        // a filter that was kept from the last time is shown when it is set: the option, the box and the rows
        MULTIPLAY::CGameListingFilter kept;
        kept.group = "turn";
        form.SetFilter( kept );
        ASSERT_TRUE( 12 == grid->GetRowCount() && 1 == view->GetSelectedIndex() && form.GetWidget( "FilterQuick" )->GetBoolProperty( GUI::EWidgetProperty_Checked ) );
        ClickWidget( context, form.GetWidget( "FilterQuick" ) );

        // a click on a title of a column is the player's own order: by name here; the rows of the player's turn are still emphasized wherever they went
        ClickColumnTitle( context, grid, 0 );
        ASSERT_TRUE( GAMEGUI::CServerBrowserForm::COLUMN_NAME == form.GetSortColumn() && "Game 00" == grid->GetText( 0, 0 ) && "Game 01" == grid->GetText( 0, 1 ) );
        emphasized = grid->GetEmphasizedRows();
        ASSERT_TRUE( 12 == emphasized.size() && 0 == emphasized.front() && 5 == emphasized[ 1 ] );

        // a change of one game among the many: its row follows (game 01 becomes the player's turn with the nearest end)
        MULTIPLAY::CGameListingEntry changed = MakeMailRow( "g1", "Game 01", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS, 3, 3, "Your turn", "", "Open", true, false );
        changed.extra.Set( MULTIPLAY::CGameListingEntry::ExtraGroup, "turn" );
        changed.extra.Set( MULTIPLAY::CGameListingEntry::ExtraAttention, "true" );
        changed.extra.Set( MULTIPLAY::CGameListingEntry::ExtraDeadline, "1" );
        rows[ 1 ] = changed;
        provider->PutBatch( rows );
        ASSERT_TRUE( 13 == grid->GetEmphasizedRows().size() && "60 games, 13 your turn" == form.GetWidget( "Status" )->GetProperty( "Text" ) );

        // a network list is not changed by any of it: no group, no attention, the kind filter, the order by name
        GAMEGUI::CServerBrowserForm plain( context );
        plain.SetBrowser( &browser );
        ASSERT_TRUE( plain.Open() );
        ASSERT_TRUE( GUCEF_NULL == plain.GetWidget( "FilterQuick" ) && plain.GetWidgetAs< GUI::CCombobox >( "FilterKind" )->GetListItems( viewItems ) && 3 == viewItems.size() && GAMEGUI::CServerBrowserForm::COLUMN_NAME == plain.GetSortColumn() );
        browser.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: A long list of games is filtered, counted and ordered without the work growing with the square of the rows; the time to take 500 rows is logged" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CFakeAsyncLobbyBackend backend;
        const char* groups[ 5 ] = { "turn", "waiting", "forming", "finished", "spectating" };
        for ( UInt32 i=0; i<500; ++i )
        {
            const UInt32 g = i % 5;
            const CORE::CString id = "g" + CORE::UInt32ToString( i );
            MULTIPLAY::CGameListingEntry row = MakeMailRow( id.C_String(), ( "Game " + CORE::UInt32ToString( 1000 + i ) ).C_String(), MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS, 3, 3, 0 == g ? "Your turn" : "Their turn", "", "Open", true, false );
            row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraGroup, groups[ g ] );
            if ( 0 == g )
            {
                row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraAttention, "true" );
                row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraDeadline, CORE::UInt32ToString( 1000000 + ( i * 37 ) % 1000 ) );
            }
            backend.rows.push_back( row );
        }
        MULTIPLAY::CGameBrowser browser;
        browser.SetPingEnabled( false );
        CORE::CTSharedPtr< MULTIPLAY::CAsyncLobbyListingProvider, MT::CMutex > shared( GUCEF_NEW MULTIPLAY::CAsyncLobbyListingProvider( backend, "fake" ) );
        browser.AddProvider( MULTIPLAY::CIGameListingProviderPtr( shared ) );
        GAMEGUI::CServerBrowserCapabilities caps = MakeMailBrowserCapabilities();
        caps.viewLabels.push_back( "All" );        caps.viewKeys.push_back( "" );
        caps.viewLabels.push_back( "Your turn" );  caps.viewKeys.push_back( "turn" );
        caps.quickFilterKey = "turn";
        caps.quickFilterText = "Your turn";
        caps.attentionText = "your turn";
        caps.defaultOrderByAttention = true;
        GAMEGUI::CServerBrowserForm form( context );
        form.SetCapabilities( caps );
        form.SetBrowser( &browser );
        ASSERT_TRUE( form.Open() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        if ( GUCEF_NULL == grid )
            return;

        // the rows arrive: the source to the browser to the list in one go
        const UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( browser.Start() );
        const UInt64 loaded = MT::PrecisionTickCount();
        ASSERT_TRUE( 500 == browser.GetListingCount() && 500 == grid->GetRowCount() && "500 games, 100 your turn" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        // every row of the player's turn is first, and the ends of the turns are in order
        ASSERT_TRUE( 100 == grid->GetEmphasizedRows().size() && 99 == grid->GetEmphasizedRows().back() );
        for ( UInt32 r=0; r<100; ++r )
            ASSERT_TRUE( "Your turn" == grid->GetText( 1, r ) );

        // all the rows change (a refresh that finds every game in another state) and the list is built again
        for ( size_t i=0; i<backend.rows.size(); ++i )
        {
            // the row again, as the source makes it, with another detail
            MULTIPLAY::CGameListingEntry again = MakeMailRow( backend.rows[ i ].listingId.C_String(), backend.rows[ i ].name.C_String(), MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS, 3, 3, 0 == i % 5 ? "Your turn" : "Their turn", "another", "Open", true, false );
            again.extra.Set( MULTIPLAY::CGameListingEntry::ExtraGroup, groups[ i % 5 ] );
            if ( 0 == i % 5 )
            {
                again.extra.Set( MULTIPLAY::CGameListingEntry::ExtraAttention, "true" );
                again.extra.Set( MULTIPLAY::CGameListingEntry::ExtraDeadline, CORE::UInt32ToString( 1000000 + ( (UInt32) i * 37 ) % 1000 ) );
            }
            backend.rows[ i ] = again;
        }
        const UInt64 changeStart = MT::PrecisionTickCount();
        backend.ChangeRows();
        const UInt64 changed = MT::PrecisionTickCount();
        ASSERT_TRUE( "another" == grid->GetText( 6, 400 ) );

        // and the filter and the order alone
        MULTIPLAY::CGameListingFilter filter;
        filter.group = "turn";
        const UInt64 filterStart = MT::PrecisionTickCount();
        form.SetFilter( filter );
        const UInt64 filtered = MT::PrecisionTickCount();
        ASSERT_TRUE( 100 == grid->GetRowCount() );

        const Float64 perMs = (Float64) MT::PrecisionTimerResolution() / 1000.0;
        const Float64 loadMs = (Float64) ( loaded - start ) / perMs;
        const Float64 changeMs = (Float64) ( changed - changeStart ) / perMs;
        const Float64 filterMs = (Float64) ( filtered - filterStart ) / perMs;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AsyncForms: 500 games: the first list took " << loadMs << " ms, a refresh that changed every row " << changeMs << " ms, a change of the filter " << filterMs << " ms" );
        // a generous bound: the point is that it is not seconds (a rebuild for every row of the 500 would be)
        ASSERT_TRUE( loadMs < 3000.0 && changeMs < 3000.0 && filterMs < 3000.0 );
        browser.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The list of 40 games with the player's turn filter on and off is inside the screen, the bar of the filter is in order and the rows of the player's turn are drawn apart from the others, at 1242 x 952, 3440 x 1440 and 3840 x 1080 (pictures with GUCEF_TEST_SHOT_DIR)" )
    try
    {
        struct SSize { UInt32 width; UInt32 height; const char* name; };
        const SSize sizes[ 3 ] = { { 1242, 952, "1242x952" }, { 3440, 1440, "3440x1440" }, { 3840, 1080, "3840x1080" } };
        for ( UInt32 z=0; z<3; ++z )
        {
            GUI::TGuiContextPtr contextPtr = CreateSizedFormContext( sizes[ z ].width, sizes[ z ].height, true );
            GUI::CGUIContext* context = ContextOf( contextPtr );
            ASSERT_TRUE( GUCEF_NULL != context );
            if ( GUCEF_NULL == context )
                return;
            TTestListingProviderFactory factory;
            MULTIPLAY::CGameBrowser browser;
            browser.SetPingEnabled( false );
            MULTIPLAY::CIGameListingProviderPtr providerPtr = factory.Create();
            CTestListingProvider* provider = static_cast< CTestListingProvider* >( providerPtr.GetPointerAlways() );
            browser.AddProvider( providerPtr );
            browser.Start();
            const char* groups[ 5 ] = { "turn", "waiting", "forming", "finished", "spectating" };
            const char* states[ 5 ] = { "Your turn", "Their turn", "Forming", "Finished", "Spectating" };
            const char* details[ 5 ] = { "Your turn, 11 h 40 min left", "Bob, due in 2 h 5 min", "Waiting for 1 answer", "Finished", "Watching" };
            GUCEF::vector< MULTIPLAY::CGameListingEntry > rows;
            for ( UInt32 i=0; i<40; ++i )
            {
                const UInt32 g = ( i * 7 ) % 5;
                MULTIPLAY::CGameListingEntry row = MakeMailRow( ( CORE::CString( "g" ) + CORE::UInt32ToString( i ) ).C_String(), ( CORE::CString( "Game " ) + CORE::UInt32ToString( 100 + i ) + ( 0 == i % 9 ? " with a name that is long enough to be cut where the title ends" : "" ) ).C_String(),
                                                               g < 2 ? MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS : MULTIPLAY::CGameListingEntry::KIND_LOBBY, 2 + i % 4, 6, states[ g ], details[ g ], "Open", true, false );
                row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraGroup, groups[ g ] );
                if ( 0 == g )
                {
                    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraAttention, "true" );
                    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraDeadline, CORE::UInt32ToString( 1000000 + 60000 * i ) );
                }
                rows.push_back( row );
            }
            provider->PutBatch( rows );

            GAMEGUI::CServerBrowserCapabilities caps = MakeMailBrowserCapabilities();
            caps.viewLabels.push_back( "All" );                     caps.viewKeys.push_back( "" );
            caps.viewLabels.push_back( "Your turn" );               caps.viewKeys.push_back( "turn" );
            caps.viewLabels.push_back( "Waiting for others" );      caps.viewKeys.push_back( "waiting" );
            caps.viewLabels.push_back( "Forming and invitations" ); caps.viewKeys.push_back( "forming" );
            caps.viewLabels.push_back( "Finished and cancelled" );  caps.viewKeys.push_back( "finished" );
            caps.viewLabels.push_back( "Spectating" );              caps.viewKeys.push_back( "spectating" );
            caps.quickFilterKey = "turn";
            caps.quickFilterText = "Your turn";
            caps.attentionText = "your turn";
            caps.defaultOrderByAttention = true;
            caps.settingsText = "Mailbox settings";
            GAMEGUI::CServerBrowserForm form( context );
            form.SetCapabilities( caps );
            form.SetBrowser( &browser );
            form.SetNote( "The mailbox is set up and working.", false );
            ASSERT_TRUE( form.Open() );
            for ( UInt32 pass=0; pass<2; ++pass )
            {
                MULTIPLAY::CGameListingFilter filter;
                filter.group = 0 == pass ? "" : "turn";
                form.SetFilter( filter );
                context->RenderFrame( 16 );
                context->FlushChanges();
                context->RenderFrame( 16 );
                GUI::CWidget* window = form.GetRootWidget();
                const GUI::CRect rect = window->GetRect();
                ASSERT_TRUE( rect.left >= 0 && rect.top >= 0 && rect.Right() <= (Int32) sizes[ z ].width && rect.Bottom() <= (Int32) sizes[ z ].height );
                // the bar of the filter: the text, the box, the control, one after the other inside the window; the status line and the buttons below the list
                GUI::CWidget* text = form.GetWidget( "FilterText" );
                GUI::CWidget* quick = form.GetWidget( "FilterQuick" );
                GUI::CWidget* view = form.GetWidget( "FilterKind" );
                ASSERT_TRUE( text->GetRect().Right() <= quick->GetRect().left && quick->GetRect().Right() <= view->GetRect().left && view->GetRect().Right() <= rect.Right() );
                ASSERT_TRUE( text->GetRect().width >= 200 && quick->GetRect().width >= 150 );
                GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
                ASSERT_TRUE( grid->GetRect().Bottom() <= form.GetWidget( "ButtonRow" )->GetRect().top && grid->GetRect().Right() <= rect.Right() );
                ASSERT_TRUE( ( 0 == pass ? 40u : 8u ) == grid->GetRowCount() && ( 0 == pass ? 8u : 8u ) == grid->GetEmphasizedRows().size() );
                // the status line says it, in the footer, inside the window
                GUI::CWidget* status = form.GetWidget( "Status" );
                ASSERT_TRUE( status->GetRect().Right() <= form.GetWidget( "ButtonRow" )->GetRect().Right() );
                ASSERT_TRUE( ( 0 == pass ? CORE::CString( "40 games, 8 your turn" ) : CORE::CString( "8 games, 8 your turn (32 hidden by the filter)" ) ) == status->GetProperty( "Text" ) );
                IMAGE::TPixelMapPtr frame = CaptureFrame( context );
                ASSERT_TRUE( !frame.IsNULL() );
                WriteShot( frame, CORE::CString( "turn-browser-" ) + sizes[ z ].name + ( 0 == pass ? "-all" : "-yourturn" ) );
            }
            browser.Stop();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The lobby of the host shows the presentation of its lobby: no ping, no faction, the words of the lobby, the note and the invite row" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CFakeAsyncLobbyBackend backend;
        backend.lobbies[ "g1" ] = MakeDraftView( "g1" );
        MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
        MULTIPLAY::CILobbyPtr lobby = OpenFakeLobby( backend, service, "g1" );
        ASSERT_TRUE( !lobby.IsNULL() );
        if ( lobby.IsNULL() )
            return;

        GAMEGUI::CLobbyForm form( context );
        CAsyncFormRecorder recorder;
        recorder.WatchLobby( &form );
        form.SetLobby( lobby );
        ASSERT_TRUE( form.Open() );

        // the table: seat, player and the answer
        CORE::CString::StringVector columns;
        form.GetSlotColumns( columns );
        ASSERT_TRUE( 3 == columns.size() && "#" == columns[ 0 ] && "Player" == columns[ 1 ] && "Answer" == columns[ 2 ] );
        GUI::CGridView* slots = form.GetWidgetAs< GUI::CGridView >( "Slots" );
        ASSERT_TRUE( GUCEF_NULL != slots && 3 == slots->GetRowCount() && 3 == slots->GetColumnCount() );
        if ( GUCEF_NULL == slots )
            return;
        ASSERT_TRUE( "Me (boss)" == slots->GetText( 1, 0 ) && "Host" == slots->GetText( 2, 0 ) && "Bob" == slots->GetText( 1, 1 ) && "Introduced" == slots->GetText( 2, 1 ) && "Waiting for an answer" == slots->GetText( 2, 2 ) );

        // the controls: no faction and colour, the words of the lobby, what is there and what can be used
        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Faction" ) && GUCEF_NULL == form.GetWidget( "Colour" ) );
        ASSERT_TRUE( "Accept" == form.GetButtonText( "Ready" ) && !IsWidgetEnabled( form, "Ready" ) && !form.IsControlShown( "Ready" ) );
        ASSERT_TRUE( "Cancel game" == form.GetButtonText( "Leave" ) && IsWidgetEnabled( form, "Leave" ) );
        ASSERT_TRUE( form.IsControlShown( "Start" ) && IsWidgetEnabled( form, "Start" ) && !form.IsControlShown( "ForceStart" ) && !form.IsControlShown( "VoteStart" ) && !form.IsControlShown( "Spectate" ) );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "Nobody can vote yet" ) >= 0 );

        // the settings are shown and fixed: the game is made with them
        ASSERT_TRUE( GUCEF_NULL != form.GetSettingsPanel() && "VOTE" == form.GetSettingsPanel()->GetValue( "mail.policy" ) && !form.GetSettingsPanel()->IsSettingEditable( "mail.policy" ) );

        // the invite row: a player is asked by name, contact and code; the row is emptied when the lobby took it
        ASSERT_TRUE( form.IsControlShown( "InviteRow" ) && form.IsControlShown( "InviteName" ) && form.IsControlShown( "InviteContact" ) && form.IsControlShown( "InviteCode" ) && form.IsControlShown( "InviteAdd" ) && IsWidgetEnabled( form, "InviteAdd" ) );
        form.GetWidget( "InviteName" )->ChangeValueByUser( "Text", "Dave" );
        form.GetWidget( "InviteContact" )->ChangeValueByUser( "Text", "dave@example.org" );
        form.GetWidget( "InviteCode" )->ChangeValueByUser( "Text", "ff" );
        ClickWidget( context, form.GetWidget( "InviteAdd" ) );
        ASSERT_TRUE( backend.Called( "invite g1 Dave dave@example.org ff" ) );
        ASSERT_TRUE( form.GetWidget( "InviteName" )->GetProperty( "Text" ).IsNULLOrEmpty() && form.GetWidget( "InviteContact" )->GetProperty( "Text" ).IsNULLOrEmpty() );

        // a name and a contact are needed; the lobby that refuses says why in the status line
        ASSERT_FALSE( form.Invite( "", "x@example.org", "" ) );
        backend.refusal = "The table is full.";
        ASSERT_FALSE( form.Invite( "Erin", "erin@example.org", "" ) );
        ASSERT_TRUE( form.GetStatusText().HasSubstr( "The table is full." ) >= 0 );
        backend.refusal.Clear();

        // the start and the leave are the actions of the lobby
        ClickWidget( context, form.GetWidget( "Start" ) );
        ASSERT_TRUE( backend.Called( "start g1" ) );
        ClickWidget( context, form.GetWidget( "Leave" ) );
        ASSERT_TRUE( backend.Called( "leave g1" ) && 1 == recorder.lefts );

        // the lobby moves on to a table where the invitees have a say: the controls follow the presentation
        MULTIPLAY::CAsyncLobbyView view = MakeDraftView( "g1" );
        view.presentation.showInvite = false;
        view.presentation.canInvite = false;
        view.presentation.showForceStart = true;
        view.presentation.canForceStart = true;
        view.presentation.forceStartText = "Start without them";
        view.presentation.note = "A decision takes effect when the host plays the first turn.";
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( !form.IsControlShown( "InviteRow" ) && form.IsControlShown( "ForceStart" ) && "Start without them" == form.GetButtonText( "ForceStart" ) && IsWidgetEnabled( form, "ForceStart" ) );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "when the host plays the first turn" ) >= 0 );
        ClickWidget( context, form.GetWidget( "ForceStart" ) );
        ASSERT_TRUE( backend.Called( "forcestart g1" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: A lobby that decides by vote offers the actions as proposals and one that a boss decides offers them to the boss only, with the reason as the tooltip of a control that is off; the lobby is closed with a button of its own" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        // by vote: everybody who accepted proposes, the host included; closing the lobby is a proposal too and does not leave the lobby
        CFakeAsyncLobbyBackend backend;
        MULTIPLAY::CAsyncLobbyView view = MakeDraftView( "g1" );
        view.presentation.showForceStart = true;
        view.presentation.canForceStart = true;
        view.presentation.forceStartText = "Propose to start";
        view.presentation.kickText = "Propose to remove";
        view.presentation.showCancel = true;
        view.presentation.canCancel = true;
        view.presentation.cancelText = "Propose to cancel";
        view.presentation.cancelHint = "The players who accepted vote: more than half must agree.";
        view.presentation.forceStartHint = "The players who accepted vote on starting without the players who did not answer.";
        view.presentation.leaveText = "Close";
        backend.lobbies[ "g1" ] = view;
        MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
        MULTIPLAY::CILobbyPtr lobby = OpenFakeLobby( backend, service, "g1" );
        ASSERT_TRUE( !lobby.IsNULL() );
        if ( lobby.IsNULL() )
            return;
        GAMEGUI::CLobbyForm form( context );
        CAsyncFormRecorder recorder;
        recorder.WatchLobby( &form );
        form.SetLobby( lobby );
        ASSERT_TRUE( form.Open() );
        ASSERT_TRUE( form.IsControlShown( "Cancel" ) && "Propose to cancel" == form.GetButtonText( "Cancel" ) && IsWidgetEnabled( form, "Cancel" ) );
        ASSERT_TRUE( "Propose to start" == form.GetButtonText( "ForceStart" ) && "Close" == form.GetButtonText( "Leave" ) );
        ASSERT_TRUE( form.GetButtonTooltip( "Cancel" ).HasSubstr( "more than half" ) >= 0 && form.GetButtonTooltip( "ForceStart" ).HasSubstr( "vote on starting" ) >= 0 );
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( backend.Called( "cancel g1" ) && 0 == recorder.lefts );
        // a refused proposal is said in the status line
        backend.refusal = "A vote about this is running.";
        ClickWidget( context, form.GetWidget( "Cancel" ) );
        ASSERT_TRUE( form.GetStatusText().HasSubstr( "A vote about this is running." ) >= 0 );
        backend.refusal.Clear();

        // a boss decides: a player who is not the boss sees the control and cannot use it; the tooltip says why
        MULTIPLAY::CAsyncLobbyView boss = MakeInvitedView( "g1" );
        boss.presentation.showForceStart = true;
        boss.presentation.canForceStart = false;
        boss.presentation.forceStartText = "Start now";
        boss.presentation.forceStartHint = "Only the host decides when the game starts without the players who did not answer.";
        boss.presentation.showCancel = false;
        backend.ChangeLobby( "g1", boss );
        ASSERT_TRUE( form.IsControlShown( "ForceStart" ) && !IsWidgetEnabled( form, "ForceStart" ) && "Start now" == form.GetButtonText( "ForceStart" ) );
        ASSERT_TRUE( form.GetButtonTooltip( "ForceStart" ).HasSubstr( "Only the host decides" ) >= 0 );
        ASSERT_TRUE( !form.IsControlShown( "Cancel" ) );

        // for the lobby of a live connection nothing changes: no cancel button, no tooltip
        MULTIPLAY::CAsyncLobbyView plain = MakeDraftView( "g1" );
        backend.ChangeLobby( "g1", plain );
        ASSERT_TRUE( !form.IsControlShown( "Cancel" ) && form.GetButtonTooltip( "Leave" ).IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The lobby of an invited player has Accept, Decline and a refused answer is said in words; an incompatible invitee cannot accept" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CFakeAsyncLobbyBackend backend;
        backend.lobbies[ "g1" ] = MakeInvitedView( "g1" );
        MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
        MULTIPLAY::CILobbyPtr lobby = OpenFakeLobby( backend, service, "g1" );
        ASSERT_TRUE( !lobby.IsNULL() );
        if ( lobby.IsNULL() )
            return;
        GAMEGUI::CLobbyForm form( context );
        CAsyncFormRecorder recorder;
        recorder.WatchLobby( &form );
        form.SetLobby( lobby );
        ASSERT_TRUE( form.Open() );

        ASSERT_TRUE( "Accept" == form.GetButtonText( "Ready" ) && IsWidgetEnabled( form, "Ready" ) && "Decline" == form.GetButtonText( "Leave" ) );
        ASSERT_TRUE( !form.IsControlShown( "Start" ) && !form.IsControlShown( "InviteRow" ) && form.IsControlShown( "Spectate" ) && IsWidgetEnabled( form, "Spectate" ) );
        ASSERT_TRUE( !form.GetWidget( "Ready" )->GetBoolProperty( "Checked" ) );

        // a lobby that is looked at before it is answered has a button to go back to the list: it tells the application and answers nothing
        ASSERT_TRUE( form.IsControlShown( "Ready" ) && form.IsControlShown( "Back" ) && "Back to the list" == form.GetButtonText( "Back" ) && IsWidgetEnabled( form, "Back" ) );
        ClickWidget( context, form.GetWidget( "Back" ) );
        ASSERT_TRUE( 1 == recorder.backs && 0 == recorder.lefts && !backend.Called( "leave g1" ) && !backend.Called( "answer g1 no" ) );

        // the answer
        backend.refusal = "The mailbox is not working.";
        ClickWidget( context, form.GetWidget( "Ready" ) );
        ASSERT_TRUE( backend.Called( "answer g1 yes" ) && form.GetStatusText().HasSubstr( "The mailbox is not working." ) >= 0 );
        backend.refusal.Clear();

        // the answer was given: checked, and it cannot be given again
        MULTIPLAY::CAsyncLobbyView view = MakeInvitedView( "g1" );
        view.state.members[ 1 ].ready = true;
        view.state.members[ 1 ].statusText = "Accepted";
        view.presentation.canReady = false;
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetWidget( "Ready" )->GetBoolProperty( "Checked" ) && !IsWidgetEnabled( form, "Ready" ) );

        // an invitee whose client cannot play the first turn: Accept is disabled, the reason is in the note, spectating and dismissing are offered
        view = MakeInvitedView( "g1" );
        view.state.members[ 1 ].role = MULTIPLAY::LOBBYMEMBERROLE_SPECTATOR;
        view.state.members[ 1 ].statusText = "Watching: this game cannot be played here";
        view.presentation.canReady = false;
        view.presentation.leaveText = "Dismiss";
        view.presentation.note = "You cannot play this game: the settings need a newer version. You can still watch it.";
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( !IsWidgetEnabled( form, "Ready" ) && "Dismiss" == form.GetButtonText( "Leave" ) && form.GetNoteText().HasSubstr( "You cannot play this game" ) >= 0 );
        GUI::CListbox* spectators = form.GetWidgetAs< GUI::CListbox >( "Spectators" );
        ASSERT_TRUE( GUCEF_NULL != spectators && 1 == spectators->GetItems().size() && spectators->GetItems()[ 0 ].HasSubstr( "Bob" ) >= 0 );
        // the lobby closes: cancelled by the host; the form says so and tells that it is over for the player
        view.closed = true;
        view.closedReason = "Cancelled by Alice";
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetStatusText().HasSubstr( "Cancelled by Alice" ) >= 0 && 1 == recorder.lefts );

        // a player who leaves is not told that the lobby closed: the answer is the Leave of the lobby only
        MULTIPLAY::CILobbyPtr second = OpenFakeLobby( backend, service, "g2" );
        backend.lobbies[ "g2" ] = MakeInvitedView( "g2" );
        second = service.OpenLobby( "g2" );
        form.SetLobby( second );
        ClickWidget( context, form.GetWidget( "Leave" ) );
        ASSERT_TRUE( backend.Called( "leave g2" ) && 2 == recorder.lefts );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: A proposal to start now or to remove a player is told in words and voted on through the lobby" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        MULTIPLAY::CLobbyVote force;
        force.id = "v1";
        force.what = "forcestart";
        force.yes = 1;
        force.needed = 2;
        ASSERT_TRUE( "Bob proposes to begin the game now, with the computer in the places of the players who have not answered" == GAMEGUI::CVotePromptForm::DescribeVote( force, "Bob", GUCEF_NULL ) );
        MULTIPLAY::CLobbyVote kick;
        kick.id = "v2";
        kick.what = "kick";
        kick.key = "carol";
        kick.value = "Carol";
        ASSERT_TRUE( "Bob proposes to remove Carol" == GAMEGUI::CVotePromptForm::DescribeVote( kick, "Bob", GUCEF_NULL ) );

        // in a lobby a vote that runs is shown in the line of the vote and asks the player, who answers through the lobby
        CFakeAsyncLobbyBackend backend;
        MULTIPLAY::CAsyncLobbyView view = MakeInvitedView( "g1" );
        view.state.members[ 0 ].ready = true;
        view.state.members[ 1 ].ready = true;
        view.presentation.canReady = false;
        backend.lobbies[ "g1" ] = view;
        MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
        MULTIPLAY::CILobbyPtr lobby = OpenFakeLobby( backend, service, "g1" );
        ASSERT_TRUE( !lobby.IsNULL() );
        if ( lobby.IsNULL() )
            return;
        GAMEGUI::CLobbyForm form( context );
        form.SetLobby( lobby );
        ASSERT_TRUE( form.Open() );
        view.state.activeVote = force;
        view.state.activeVote.proposer = "me";
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetVoteText().HasSubstr( "Me proposes to begin the game now" ) >= 0 && form.GetVotePrompt()->IsOpen() );
        ASSERT_TRUE( form.GetVotePrompt()->Cast( true ) );
        ASSERT_TRUE( backend.Called( "vote g1 v1 yes" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The decisions of the lobby are shown under the note as the votes come in, with a warning when the host did not carry one out; a lobby without decisions shows the note alone" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CFakeAsyncLobbyBackend backend;
        MULTIPLAY::CAsyncLobbyView first = MakeDraftView( "g1" );
        first.presentation.note = "The table decides by vote.";
        backend.lobbies[ "g1" ] = first;
        MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
        MULTIPLAY::CILobbyPtr lobby = OpenFakeLobby( backend, service, "g1" );
        ASSERT_TRUE( !lobby.IsNULL() );
        if ( lobby.IsNULL() )
            return;
        GAMEGUI::CLobbyForm form( context );
        form.SetLobby( lobby );
        ASSERT_TRUE( form.Open() );
        ASSERT_TRUE( "The table decides by vote." == form.GetNoteText() );

        // a proposal with one vote, then the second vote, then the warning
        MULTIPLAY::CLobbyPresentation shown;
        shown.note = "The table decides by vote.";
        shown.voteLines.push_back( "Vote: Bob: Remove Dave from the game. Yes: Bob. No: nobody. Not voted: Alice, Carol. 1 yes of 3, 2 needed. Open." );
        MULTIPLAY::CAsyncLobbyView view = MakeDraftView( "g1" );
        view.presentation = shown;
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "Vote: Bob" ) == 0 && form.GetNoteText().HasSubstr( "Remove Dave from the game. Yes: Bob." ) > 0 && form.GetNoteText().HasSubstr( "The table decides by vote." ) > 0 );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "Warning" ) < 0 );
        view.presentation.voteLines[ 0 ] = "Vote: Bob: Remove Dave from the game. Yes: Bob, Carol. No: nobody. Not voted: Alice. 2 yes of 3, 2 needed. Passed: waiting for the host to carry it out.";
        view.presentation.voteWarning = "The host has not carried out a decision that passed.";
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "Yes: Bob, Carol." ) > 0 && form.GetNoteText().HasSubstr( "Warning: The host has not carried out a decision that passed." ) > 0 );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "Warning" ) < form.GetNoteText().HasSubstr( "The table decides by vote." ) );
        ASSERT_TRUE( form.GetNoteText() == GAMEGUI::CLobbyForm::ComposeNote( view.presentation ) );

        // with no note the decisions stand alone, and with neither the note label is not shown
        view.presentation.note.Clear();
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetNoteText().HasSubstr( "Vote: Bob" ) == 0 && form.GetNoteText().HasSubstr( "The table decides" ) < 0 );
        view.presentation.voteLines.clear();
        view.presentation.voteWarning.Clear();
        backend.ChangeLobby( "g1", view );
        ASSERT_TRUE( form.GetNoteText().IsNULLOrEmpty() && !form.IsControlShown( "Note" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The form that creates a lobby is opaque in both layouts: the list of games behind it does not show through" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;
        const UInt32 widths[] = { 800, 1242, 3440 };
        const UInt32 heights[] = { 600, 952, 1440 };
        for ( UInt32 s=0; s<3; ++s )
        {
            context->SetLogicalResolution( widths[ s ], heights[ s ] );
            GAMEGUI::CCreateLobbyForm form( context );
            ASSERT_TRUE( form.Open() );
            context->RenderFrame( 16 );
            context->FlushChanges();
            context->RenderFrame( 16 );
            ASSERT_TRUE( GUCEF_NULL != form.GetRootWidget() );
            if ( GUCEF_NULL == form.GetRootWidget() )
                continue;
            ASSERT_TRUE( form.GetRootWidget()->GetProperty( GUI::EWidgetProperty_BackgroundColour ).HasSubstr( ",255" ) > 0 );
            GUI::CWidget* general = form.GetWidget( "GeneralColumn" );
            GUI::CWidget* body = form.GetWidget( "Body" );
            ASSERT_TRUE( ( GUCEF_NULL != general && general->GetProperty( GUI::EWidgetProperty_BackgroundColour ).HasSubstr( ",255" ) > 0 ) || ( GUCEF_NULL != body && body->GetProperty( GUI::EWidgetProperty_BackgroundColour ).HasSubstr( ",255" ) > 0 ) );
            ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "SettingsHolder" ) );
            form.Hide();
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: A lobby of a game on a network keeps its columns, its faction and colour and its words" )
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
        form.SetLobby( fixture.alice );
        ASSERT_TRUE( form.Open() );
        CORE::CString::StringVector columns;
        form.GetSlotColumns( columns );
        ASSERT_TRUE( 7 == columns.size() && "Ready" == columns[ 5 ] && "Ping" == columns[ 6 ] );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "Faction" ) && GUCEF_NULL != form.GetWidget( "Colour" ) && "Ready" == form.GetButtonText( "Ready" ) && "Leave" == form.GetButtonText( "Leave" ) );
        ASSERT_TRUE( form.IsControlShown( "Start" ) && form.IsControlShown( "ForceStart" ) && form.IsControlShown( "VoteStart" ) && form.IsControlShown( "Spectate" ) && !form.IsControlShown( "InviteRow" ) );
        ASSERT_TRUE( form.GetNoteText().IsNULLOrEmpty() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "AsyncForms: The form that creates a lobby without password, spectators, place and service, with a row for every player to invite" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CCreateLobbyCapabilities caps;
        caps.showPassword = false;
        caps.showSpectators = false;
        caps.showWhere = false;
        caps.showService = false;
        caps.showInvitees = true;
        caps.title = "New e-mail game";
        caps.nameLabel = "Name of the game";
        caps.playersLabel = "Players";
        caps.createText = "Make the game";
        caps.minPlayers = 2;
        caps.maxPlayers = 6;
        caps.inviteesHint = "Name, mail address and the player code if you have it.";

        MULTIPLAY::CLobbyCreateParams params;
        params.name = "Round one";
        params.maxPlayers = 3;
        MULTIPLAY::CLobbySettingDef mode;
        mode.key = "mail.policy";
        mode.label = "How the table decides";
        mode.type = MULTIPLAY::CLobbySettingDef::TYPE_ENUM;
        mode.options.push_back( "VOTE" );
        mode.options.push_back( "BOSS" );
        mode.defaultValue = "VOTE";
        mode.group = "E-mail";
        params.schema.AddSetting( mode );

        GAMEGUI::CCreateLobbyForm form( context );
        CAsyncFormRecorder recorder;
        recorder.WatchCreate( &form );
        form.SetCapabilities( caps );
        form.SetTemplate( params );
        ASSERT_TRUE( form.Open() );

        ASSERT_TRUE( GUCEF_NULL == form.GetWidget( "Password" ) && GUCEF_NULL == form.GetWidget( "Spectators" ) && GUCEF_NULL == form.GetWidget( "Where" ) && GUCEF_NULL == form.GetWidget( "Service" ) );
        ASSERT_TRUE( GUCEF_NULL != form.GetWidget( "MaxPlayers" ) && "Players" == form.GetWidget( "MaxPlayersLabel" )->GetProperty( "Text" ) && "Name of the game" == form.GetWidget( "NameLabel" )->GetProperty( "Text" ) );
        ASSERT_TRUE( "New e-mail game" == form.GetRootWidget()->GetProperty( GUI::EWidgetProperty_Title ) && "Make the game" == form.GetWidget( "Create" )->GetProperty( "Text" ) );
        ASSERT_TRUE( 2 == form.GetWidget( "MaxPlayers" )->GetIntProperty( GUI::EWidgetProperty_Min ) && 6 == form.GetWidget( "MaxPlayers" )->GetIntProperty( GUI::EWidgetProperty_Max ) );

        // two rows for three players, and the settings of the schema next to them
        ASSERT_TRUE( 2 == form.GetVisibleInviteeRows() );
        // the rows that are not needed are not there at all, with what is in them: a form that was loaded again for another screen size has the same rows
        for ( UInt32 pass=0; pass<2; ++pass )
        {
            ASSERT_TRUE( form.GetWidget( "InviteeRow0" )->IsEffectivelyVisible() && form.GetWidget( "InviteeRow1" )->IsEffectivelyVisible() );
            ASSERT_TRUE( !form.GetWidget( "InviteeRow2" )->IsEffectivelyVisible() && !form.GetWidget( "InviteeName2" )->IsEffectivelyVisible() && !form.GetWidget( "InviteeCode5" )->IsEffectivelyVisible() );
            if ( 0 == pass )
            {
                context->SetLogicalResolution( 1242, 952 );
                context->RenderFrame( 16 );
                context->FlushChanges();
                context->RenderFrame( 16 );
            }
        }
        // a form that was built before it got its template (as an application does, to open it later) has the rows of the template
        {
            GAMEGUI::CCreateLobbyForm late( context );
            late.SetCapabilities( caps );
            ASSERT_TRUE( late.Build() );
            late.SetTemplate( params );
            ASSERT_TRUE( late.Open() );
            context->RenderFrame( 16 );
            context->FlushChanges();
            context->RenderFrame( 16 );
            ASSERT_TRUE( 2 == late.GetVisibleInviteeRows() && late.GetWidget( "InviteeRow1" )->IsEffectivelyVisible() && !late.GetWidget( "InviteeRow2" )->IsEffectivelyVisible() && !late.GetWidget( "InviteeName4" )->IsEffectivelyVisible() );
            late.Close();
        }

        // and the same when the form was closed while the screen changed size, and is opened again
        form.Close();
        context->SetLogicalResolution( 3440, 1440 );
        context->RenderFrame( 16 );
        context->FlushChanges();
        ASSERT_TRUE( form.Open() );
        context->RenderFrame( 16 );
        context->FlushChanges();
        context->RenderFrame( 16 );
        ASSERT_TRUE( form.GetWidget( "InviteeRow1" )->IsEffectivelyVisible() && !form.GetWidget( "InviteeRow2" )->IsEffectivelyVisible() && !form.GetWidget( "InviteeName4" )->IsEffectivelyVisible() );
        context->SetLogicalResolution( 1280, 960 );
        ASSERT_TRUE( GUCEF_NULL != form.GetSettingsPanel() && form.GetSettingsPanel()->GetValue( "mail.policy" ) == "VOTE" );
        form.GetWidget( "MaxPlayers" )->ChangeValueByUser( "Value", "5" );
        ASSERT_TRUE( 4 == form.GetVisibleInviteeRows() );
        form.GetWidget( "MaxPlayers" )->ChangeValueByUser( "Value", "3" );
        ASSERT_TRUE( 2 == form.GetVisibleInviteeRows() );

        ASSERT_TRUE( form.SetInvitee( 0, "Bob", "bob@example.org", "" ) && form.SetInvitee( 1, "Carol", "carol@example.org", "abcd" ) && !form.SetInvitee( 2, "Dave", "dave@example.org", "" ) );
        MULTIPLAY::CLobbyInvitation invitation;
        ASSERT_TRUE( form.GetInvitee( 1, invitation ) && "Carol" == invitation.name && "carol@example.org" == invitation.contact && "abcd" == invitation.code );

        // what the player filled in stays when the capabilities are set again
        form.SetCapabilities( caps );
        ASSERT_TRUE( form.GetInvitee( 0, invitation ) && "Bob" == invitation.name && 2 == form.GetVisibleInviteeRows() );

        // the params: the players to invite, the settings
        ASSERT_TRUE( form.Create() );
        ASSERT_TRUE( 1 == recorder.createRequests && 3 == recorder.lastRequest.params.maxPlayers && "Round one" == recorder.lastRequest.params.name );
        ASSERT_TRUE( 2 == recorder.lastRequest.params.invitations.size() && "bob@example.org" == recorder.lastRequest.params.invitations[ 0 ].contact && "abcd" == recorder.lastRequest.params.invitations[ 1 ].code );
        ASSERT_TRUE( "VOTE" == recorder.lastRequest.params.initialSettings.GetValueAlways( "mail.policy" ).AsString() );

        // a row left empty is not an invitation; the application says what is wrong with the wishes in the error line, which stays until the form opens again
        ASSERT_TRUE( form.Open() );
        ASSERT_TRUE( form.SetInvitee( 1, "", "", "" ) );
        ASSERT_TRUE( form.Create() && 1 == recorder.lastRequest.params.invitations.size() );
        ASSERT_TRUE( form.Open() );
        form.SetErrorText( "Every player besides you needs a name and a mail address." );
        ASSERT_TRUE( form.GetErrorText().HasSubstr( "needs a name" ) >= 0 );
        ASSERT_TRUE( form.Open() && form.GetErrorText().IsNULLOrEmpty() );

        // the default form of a network lobby has none of this
        GAMEGUI::CCreateLobbyForm network( context );
        ASSERT_TRUE( network.Open() );
        ASSERT_TRUE( GUCEF_NULL != network.GetWidget( "Password" ) && GUCEF_NULL != network.GetWidget( "Where" ) && GUCEF_NULL != network.GetWidget( "Service" ) && 0 == network.GetVisibleInviteeRows() );
        ASSERT_FALSE( network.SetInvitee( 0, "x", "y", "" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
