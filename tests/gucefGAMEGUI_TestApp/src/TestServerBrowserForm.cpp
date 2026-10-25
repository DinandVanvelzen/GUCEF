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
#include "TestListingProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the events of the browser form with the listing they carry */
class CBrowserFormRecorder : public CORE::CObserver
{
    public:

    UInt32 joins;
    UInt32 spectates;
    UInt32 creates;
    UInt32 directs;
    MULTIPLAY::CGameListingEntry lastJoin;
    MULTIPLAY::CGameListingEntry lastSpectate;

    typedef CORE::CTEventHandlerFunctor< CBrowserFormRecorder > TEventCallback;

    CBrowserFormRecorder( CORE::CNotifier* form )
        : CORE::CObserver(), joins( 0 ), spectates( 0 ), creates( 0 ), directs( 0 ), lastJoin(), lastSpectate()
    {
        TEventCallback joinCallback( this, &CBrowserFormRecorder::OnJoinRequested );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::JoinRequestedEvent, joinCallback );
        TEventCallback spectateCallback( this, &CBrowserFormRecorder::OnSpectateRequested );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::SpectateRequestedEvent, spectateCallback );
        TEventCallback createCallback( this, &CBrowserFormRecorder::OnCreateRequested );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::CreateRequestedEvent, createCallback );
        TEventCallback directCallback( this, &CBrowserFormRecorder::OnDirectConnectRequested );
        SubscribeTo( form, GAMEGUI::CServerBrowserForm::DirectConnectRequestedEvent, directCallback );
    }

    virtual ~CBrowserFormRecorder() { UnsubscribeAllFromObserver(); }

    void OnJoinRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++joins;
        lastJoin = *static_cast< MULTIPLAY::CGameListingEntry* >( eventData );
    }

    void OnSpectateRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++spectates;
        lastSpectate = *static_cast< MULTIPLAY::CGameListingEntry* >( eventData );
    }

    void OnCreateRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++creates;
    }

    void OnDirectConnectRequested( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        ++directs;
    }
};

/*-------------------------------------------------------------------------*/

static MULTIPLAY::CGameListingEntry
MakeListing( const char* id                                    ,
             const char* name                                  ,
             MULTIPLAY::CGameListingEntry::TKind kind          ,
             UInt32 players                                    ,
             UInt32 maxPlayers                                 ,
             const char* map                                   ,
             const char* mode                                  ,
             bool passworded                                   ,
             Int32 pingMs                                      )
{
    MULTIPLAY::CGameListingEntry listing;
    listing.listingId = id;
    listing.name = name;
    listing.kind = kind;
    listing.gameId = "testgame";
    listing.gameVersion = "1.0";
    listing.mapName = map;
    listing.modeName = mode;
    listing.players = players;
    listing.maxPlayers = maxPlayers;
    listing.passworded = passworded;
    listing.hostName = "Host";
    listing.endpoints.push_back( CORE::CString( "udp://127.0.0.1:" ) + id );
    listing.backendName = "test-listings";
    listing.pingMs = pingMs;
    return listing;
}

/*-------------------------------------------------------------------------*/

/** A browser with a test provider, ping measuring off */
class CBrowserFixture
{
    public:

    TTestListingProviderFactory factory;        /**< the providers go back to it, so it outlives the browser */
    MULTIPLAY::CGameBrowser browser;
    CTestListingProvider* provider;

    CBrowserFixture( void )
        : factory(), browser(), provider( GUCEF_NULL )
    {
        browser.SetPingEnabled( false );
        MULTIPLAY::CIGameListingProviderPtr providerPtr = factory.Create();
        provider = static_cast< CTestListingProvider* >( providerPtr.GetPointerAlways() );
        browser.AddProvider( providerPtr );
        browser.Start();
    }

    ~CBrowserFixture() { browser.Stop(); }

    void AddFourListings( void )
    {
        provider->Put( MakeListing( "1", "Alpha", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 2, 4, "dust", "ffa", false, 30 ) );
        provider->Put( MakeListing( "2", "bravo", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS, 6, 8, "ice", "tdm", true, -1 ) );
        provider->Put( MakeListing( "3", "Charlie", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 4, 4, "dust", "tdm", false, 120 ) );
        provider->Put( MakeListing( "4", "delta", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 1, 2, "ice", "ffa", true, 60 ) );
    }
};

/*-------------------------------------------------------------------------*/

static CORE::CString
NameAt( GUI::CGridView* grid, UInt32 row )
{
    return grid->GetText( GAMEGUI::CServerBrowserForm::COLUMN_NAME, row );
}

/*-------------------------------------------------------------------------*/

void
PerformServerBrowserFormTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "gucefGAMEGUI.ServerBrowser" );

    GUCEF_TESTFW_TESTCASE( "ServerBrowser: Rows appear, update and go as the browser's list changes" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CBrowserFixture fixture;
        GAMEGUI::CServerBrowserForm form( context );
        ASSERT_TRUE( form.Open() && form.IsVisible() );
        form.SetBrowser( &fixture.browser );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid );
        if ( GUCEF_NULL == grid )
            return;
        ASSERT_TRUE( 0 == grid->GetRowCount() && 8 == grid->GetColumnCount() );
        ASSERT_TRUE( "0 games" == form.GetWidget( "Status" )->GetProperty( "Text" ) );

        fixture.AddFourListings();
        ASSERT_TRUE( 4 == grid->GetRowCount() && 4 == form.GetListedEntries().size() );
        ASSERT_TRUE( "4 games" == form.GetWidget( "Status" )->GetProperty( "Text" ) );

        // the cells of a row: name, kind, players, map, mode, ping, lock, source
        ASSERT_TRUE( "Alpha" == NameAt( grid, 0 ) && "Lobby" == grid->GetText( 1, 0 ) && "2/4" == grid->GetText( 2, 0 ) );
        ASSERT_TRUE( "dust" == grid->GetText( 3, 0 ) && "ffa" == grid->GetText( 4, 0 ) && "30 ms" == grid->GetText( 5, 0 ) );
        ASSERT_TRUE( grid->GetText( 6, 0 ).IsNULLOrEmpty() && "test-listings" == grid->GetText( 7, 0 ) );
        ASSERT_TRUE( "bravo" == NameAt( grid, 1 ) && "In progress" == grid->GetText( 1, 1 ) && grid->GetText( 5, 1 ).IsNULLOrEmpty() && "Locked" == grid->GetText( 6, 1 ) );

        // a listing that changes shows the change in its row
        fixture.provider->Put( MakeListing( "1", "Alpha", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 3, 4, "ice", "ffa", false, 45 ) );
        ASSERT_TRUE( 4 == grid->GetRowCount() && "3/4" == grid->GetText( 2, 0 ) && "ice" == grid->GetText( 3, 0 ) && "45 ms" == grid->GetText( 5, 0 ) );

        // a new one joins the order, a gone one leaves it
        fixture.provider->Put( MakeListing( "5", "Echo", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 1, 4, "dust", "ffa", false, 10 ) );
        ASSERT_TRUE( 5 == grid->GetRowCount() && "Echo" == NameAt( grid, 4 ) );
        fixture.provider->Drop( "2" );
        ASSERT_TRUE( 4 == grid->GetRowCount() && "Charlie" == NameAt( grid, 1 ) );

        // the row that was chosen stays chosen while the list moves
        ASSERT_TRUE( form.SelectListing( "3" ) );
        MULTIPLAY::CGameListingEntry selected;
        ASSERT_TRUE( form.GetSelectedListing( selected ) && "3" == selected.listingId );
        fixture.provider->Put( MakeListing( "0", "Aardvark", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 1, 4, "dust", "ffa", false, 10 ) );
        ASSERT_TRUE( form.GetSelectedListing( selected ) && "3" == selected.listingId && 2 == grid->GetSelectedRow() );

        // no browser, nothing to show
        form.SetBrowser( GUCEF_NULL );
        ASSERT_TRUE( 0 == grid->GetRowCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "ServerBrowser: The listings sort by a column, again the other way, the unknown pings last" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CBrowserFixture fixture;
        fixture.AddFourListings();
        GAMEGUI::CServerBrowserForm form( context );
        form.SetBrowser( &fixture.browser );
        ASSERT_TRUE( form.Open() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid );
        if ( GUCEF_NULL == grid )
            return;

        // by name, ignoring case, to begin with
        ASSERT_TRUE( GAMEGUI::CServerBrowserForm::COLUMN_NAME == form.GetSortColumn() && form.IsSortAscending() );
        ASSERT_TRUE( "Alpha" == NameAt( grid, 0 ) && "bravo" == NameAt( grid, 1 ) && "Charlie" == NameAt( grid, 2 ) && "delta" == NameAt( grid, 3 ) );
        ASSERT_TRUE( 0 == grid->GetIntProperty( "SortColumn" ) && grid->GetBoolProperty( "SortAscending" ) );

        // a click on the title of the ping column
        ClickColumnTitle( context, grid, GAMEGUI::CServerBrowserForm::COLUMN_PING );
        ASSERT_TRUE( GAMEGUI::CServerBrowserForm::COLUMN_PING == form.GetSortColumn() && form.IsSortAscending() );
        ASSERT_TRUE( "Alpha" == NameAt( grid, 0 ) && "delta" == NameAt( grid, 1 ) && "Charlie" == NameAt( grid, 2 ) && "bravo" == NameAt( grid, 3 ) );
        ASSERT_TRUE( 5 == grid->GetIntProperty( "SortColumn" ) );

        // again: the other way round, the unknown ping stays last
        ClickColumnTitle( context, grid, GAMEGUI::CServerBrowserForm::COLUMN_PING );
        ASSERT_TRUE( !form.IsSortAscending() && !grid->GetBoolProperty( "SortAscending" ) );
        ASSERT_TRUE( "Charlie" == NameAt( grid, 0 ) && "delta" == NameAt( grid, 1 ) && "Alpha" == NameAt( grid, 2 ) && "bravo" == NameAt( grid, 3 ) );

        // players, by number
        form.SetSort( GAMEGUI::CServerBrowserForm::COLUMN_PLAYERS, false );
        ASSERT_TRUE( "bravo" == NameAt( grid, 0 ) && "Charlie" == NameAt( grid, 1 ) && "Alpha" == NameAt( grid, 2 ) && "delta" == NameAt( grid, 3 ) );

        // the kind, ties by name
        form.SetSort( GAMEGUI::CServerBrowserForm::COLUMN_KIND, true );
        ASSERT_TRUE( "Alpha" == NameAt( grid, 0 ) && "Charlie" == NameAt( grid, 1 ) && "delta" == NameAt( grid, 2 ) && "bravo" == NameAt( grid, 3 ) );

        // the lock
        form.SetSort( GAMEGUI::CServerBrowserForm::COLUMN_LOCK, false );
        ASSERT_TRUE( "bravo" == NameAt( grid, 0 ) && "delta" == NameAt( grid, 1 ) );

        // a new listing lands in the order of the sort
        form.SetSort( GAMEGUI::CServerBrowserForm::COLUMN_NAME, true );
        fixture.provider->Put( MakeListing( "6", "Bravado", MULTIPLAY::CGameListingEntry::KIND_LOBBY, 1, 4, "dust", "ffa", false, 10 ) );
        ASSERT_TRUE( "Bravado" == NameAt( grid, 1 ) && "bravo" == NameAt( grid, 2 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ServerBrowser: The filter bar filters by text, kind, free places, password and ping" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CBrowserFixture fixture;
        fixture.AddFourListings();
        GAMEGUI::CServerBrowserForm form( context );
        form.SetBrowser( &fixture.browser );
        ASSERT_TRUE( form.Open() );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid && 4 == grid->GetRowCount() );
        if ( GUCEF_NULL == grid )
            return;

        // text: found in the name, map, mode or host, whatever the case
        TypeInto( context, form.GetWidget( "FilterText" ), "ALP", false );
        form.GetWidget( "FilterText" )->ChangeValueByUser( "Text", "ALP" );
        ASSERT_TRUE( 1 == grid->GetRowCount() && "Alpha" == NameAt( grid, 0 ) );
        ASSERT_TRUE( "ALP" == form.GetFilter().text );
        ASSERT_TRUE( "1 game (3 hidden by the filter)" == form.GetWidget( "Status" )->GetProperty( "Text" ) );
        form.GetWidget( "FilterText" )->ChangeValueByUser( "Text", "ice" );
        ASSERT_TRUE( 2 == grid->GetRowCount() && "bravo" == NameAt( grid, 0 ) && "delta" == NameAt( grid, 1 ) );
        form.GetWidget( "FilterText" )->ChangeValueByUser( "Text", "" );
        ASSERT_TRUE( 4 == grid->GetRowCount() );

        // kind, picked from the list of the combo box
        ASSERT_TRUE( PickItem( context, form.GetWidget( "FilterKind" ), 1 ) );
        ASSERT_TRUE( MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY == form.GetFilter().kind );
        ASSERT_TRUE( 3 == grid->GetRowCount() && "Alpha" == NameAt( grid, 0 ) && "Charlie" == NameAt( grid, 1 ) && "delta" == NameAt( grid, 2 ) );
        ASSERT_TRUE( PickItem( context, form.GetWidget( "FilterKind" ), 2 ) );
        ASSERT_TRUE( MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS == form.GetFilter().kind );
        ASSERT_TRUE( 1 == grid->GetRowCount() && "bravo" == NameAt( grid, 0 ) );
        ASSERT_TRUE( PickItem( context, form.GetWidget( "FilterKind" ), 0 ) );
        ASSERT_TRUE( 4 == grid->GetRowCount() );

        // full lobbies and passworded ones, by the check boxes
        ClickWidget( context, form.GetWidget( "HideFull" ) );
        ASSERT_TRUE( form.GetFilter().notFull && 3 == grid->GetRowCount() && "Alpha" == NameAt( grid, 0 ) && "bravo" == NameAt( grid, 1 ) && "delta" == NameAt( grid, 2 ) );
        ClickWidget( context, form.GetWidget( "HideLocked" ) );
        ASSERT_TRUE( form.GetFilter().noPassword && 1 == grid->GetRowCount() && "Alpha" == NameAt( grid, 0 ) );
        ClickWidget( context, form.GetWidget( "HideFull" ) );
        ClickWidget( context, form.GetWidget( "HideLocked" ) );
        ASSERT_TRUE( !form.GetFilter().notFull && !form.GetFilter().noPassword && 4 == grid->GetRowCount() );

        // the ping limit: 0 is none, an unknown ping passes
        form.GetWidget( "MaxPing" )->ChangeValueByUser( "Value", "50" );
        ASSERT_TRUE( 50 == form.GetFilter().maxPingMs && 2 == grid->GetRowCount() && "Alpha" == NameAt( grid, 0 ) && "bravo" == NameAt( grid, 1 ) );
        form.GetWidget( "MaxPing" )->ChangeValueByUser( "Value", "100" );
        ASSERT_TRUE( 3 == grid->GetRowCount() );
        form.GetWidget( "MaxPing" )->ChangeValueByUser( "Value", "0" );
        ASSERT_TRUE( -1 == form.GetFilter().maxPingMs && 4 == grid->GetRowCount() );

        // a filter given in code shows in the bar
        MULTIPLAY::CGameListingFilter filter;
        filter.text = "dust";
        filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY;
        filter.notFull = true;
        filter.maxPingMs = 40;
        form.SetFilter( filter );
        ASSERT_TRUE( 1 == grid->GetRowCount() && "Alpha" == NameAt( grid, 0 ) );
        ASSERT_TRUE( "dust" == form.GetWidget( "FilterText" )->GetProperty( "Text" ) && 1 == form.GetWidgetAs< GUI::CCombobox >( "FilterKind" )->GetSelectedIndex() );
        ASSERT_TRUE( form.GetWidget( "HideFull" )->GetBoolProperty( "Checked" ) && !form.GetWidget( "HideLocked" )->GetBoolProperty( "Checked" ) && 40 == form.GetWidget( "MaxPing" )->GetIntProperty( "Value" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ServerBrowser: The buttons follow the selected listing and the events carry the listing" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        CBrowserFixture fixture;
        fixture.AddFourListings();
        GAMEGUI::CServerBrowserForm form( context );
        CBrowserFormRecorder recorder( &form );
        ASSERT_TRUE( form.Open() );
        form.SetBrowser( &fixture.browser );
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid && 4 == grid->GetRowCount() );
        if ( GUCEF_NULL == grid )
            return;

        // nothing chosen: no joining, no spectating; creating, direct connecting and refreshing are always there
        ASSERT_TRUE( !IsWidgetEnabled( form, "Join" ) && !IsWidgetEnabled( form, "Spectate" ) );
        ASSERT_TRUE( IsWidgetEnabled( form, "Create" ) && IsWidgetEnabled( form, "DirectConnect" ) && IsWidgetEnabled( form, "Refresh" ) );
        ClickWidget( context, form.GetWidget( "Join" ) );
        ASSERT_TRUE( 0 == recorder.joins );

        // a lobby with room: both
        ClickRow( context, grid, 0 );
        ASSERT_TRUE( IsWidgetEnabled( form, "Join" ) && IsWidgetEnabled( form, "Spectate" ) && form.CanJoin() && form.CanSpectate() );

        // a game in progress can only be watched
        ClickRow( context, grid, 1 );
        ASSERT_TRUE( !IsWidgetEnabled( form, "Join" ) && IsWidgetEnabled( form, "Spectate" ) );

        // a full lobby: watched too, not joined
        ClickRow( context, grid, 2 );
        ASSERT_TRUE( !IsWidgetEnabled( form, "Join" ) && IsWidgetEnabled( form, "Spectate" ) );

        // the events carry the listing of the row
        ClickRow( context, grid, 0 );
        ClickWidget( context, form.GetWidget( "Join" ) );
        ASSERT_TRUE( 1 == recorder.joins && "1" == recorder.lastJoin.listingId && "Alpha" == recorder.lastJoin.name && 2 == recorder.lastJoin.players );
        ASSERT_TRUE( 1 == recorder.lastJoin.endpoints.size() && "udp://127.0.0.1:1" == recorder.lastJoin.endpoints[ 0 ] );
        ClickRow( context, grid, 1 );
        ClickWidget( context, form.GetWidget( "Spectate" ) );
        ASSERT_TRUE( 1 == recorder.spectates && "2" == recorder.lastSpectate.listingId && MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS == recorder.lastSpectate.kind );
        ClickWidget( context, form.GetWidget( "Create" ) );
        ClickWidget( context, form.GetWidget( "DirectConnect" ) );
        ASSERT_TRUE( 1 == recorder.creates && 1 == recorder.directs );

        // a double click joins; for a game in progress it watches; a full lobby does nothing
        DoubleClickRow( context, grid, 3 );
        ASSERT_TRUE( 2 == recorder.joins && "4" == recorder.lastJoin.listingId );
        DoubleClickRow( context, grid, 1 );
        ASSERT_TRUE( 2 == recorder.spectates && "2" == recorder.lastSpectate.listingId && 2 == recorder.joins );
        DoubleClickRow( context, grid, 2 );
        ASSERT_TRUE( 2 == recorder.spectates && 2 == recorder.joins );

        // refreshing asks the providers again
        UInt32 refreshes = fixture.provider->GetRefreshCount();
        ClickWidget( context, form.GetWidget( "Refresh" ) );
        ASSERT_TRUE( refreshes + 1 == fixture.provider->GetRefreshCount() );

        // when the chosen listing goes the buttons go with it
        ClickRow( context, grid, 0 );
        ASSERT_TRUE( IsWidgetEnabled( form, "Join" ) );
        fixture.provider->Drop( "1" );
        ASSERT_TRUE( !IsWidgetEnabled( form, "Join" ) && !IsWidgetEnabled( form, "Spectate" ) && !form.Join() && !form.Spectate() );

        // without a browser there is nothing to refresh
        form.SetBrowser( GUCEF_NULL );
        ASSERT_TRUE( !IsWidgetEnabled( form, "Refresh" ) && !form.RefreshListings() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ServerBrowser: The handlers are subscribed to the widgets once, and the form lets go of a browser that goes away" )
    try
    {
        GUI::TGuiContextPtr contextPtr = CreateFormContext();
        GUI::CGUIContext* context = ContextOf( contextPtr );
        ASSERT_TRUE( GUCEF_NULL != context );
        if ( GUCEF_NULL == context )
            return;

        GAMEGUI::CServerBrowserForm form( context );
        ASSERT_TRUE( form.Open() );

        // a button each, a filter control each, the three events of the list
        const UInt32 widgetSubscriptions = GetEventSubscriptionCount( form );
        ASSERT_TRUE( 13 == widgetSubscriptions );
        ASSERT_TRUE( form.Open() && widgetSubscriptions == GetEventSubscriptionCount( form ) );

        CBrowserFixture* fixture = GUCEF_NEW CBrowserFixture();
        form.SetBrowser( &fixture->browser );
        ASSERT_TRUE( &fixture->browser == form.GetBrowser() && widgetSubscriptions + 1 == GetEventSubscriptionCount( form ) );
        fixture->AddFourListings();
        GUI::CGridView* grid = form.GetWidgetAs< GUI::CGridView >( "Listings" );
        ASSERT_TRUE( GUCEF_NULL != grid && 4 == grid->GetRowCount() );

        // the browser goes away under the form
        GUCEF_DELETE fixture;
        fixture = GUCEF_NULL;
        ASSERT_TRUE( GUCEF_NULL == form.GetBrowser() && widgetSubscriptions == GetEventSubscriptionCount( form ) );
        ASSERT_TRUE( GUCEF_NULL != grid && 0 == grid->GetRowCount() && !IsWidgetEnabled( form, "Refresh" ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
