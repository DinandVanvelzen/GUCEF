/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBYSERVICE_H
#include "gucefMULTIPLAY_CAsyncLobbyService.h"
#define GUCEF_MULTIPLAY_CASYNCLOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CASYNCLOBBYSERVICE_H ? */

#ifndef GUCEF_MULTIPLAY_CASYNCLOBBYLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CAsyncLobbyListingProvider.h"
#define GUCEF_MULTIPLAY_CASYNCLOBBYLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CASYNCLOBBYLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEBROWSER_H
#include "gucefMULTIPLAY_CGameBrowser.h"
#define GUCEF_MULTIPLAY_CGAMEBROWSER_H
#endif /* GUCEF_MULTIPLAY_CGAMEBROWSER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestFakeAsyncLobbyBackend.h"
#include "TestLobbyRecorder.h"
#include "TestAsyncLobby.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
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

/** Counts what a listing provider reports */
class CAsyncListingRecorder : public CORE::CObserver
{
    public:

    UInt32 added;
    UInt32 updated;
    UInt32 removed;
    UInt32 batchesEnded;

    CAsyncListingRecorder( void ) : CORE::CObserver(), added( 0 ), updated( 0 ), removed( 0 ), batchesEnded( 0 ) {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CAsyncListingRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIGameListingProvider::ListingAddedEvent == eventId ) ++added;
        else
        if ( MULTIPLAY::CIGameListingProvider::ListingUpdatedEvent == eventId ) ++updated;
        else
        if ( MULTIPLAY::CIGameListingProvider::ListingRemovedEvent == eventId ) ++removed;
        else
        if ( MULTIPLAY::CIGameListingProvider::ListingsBatchEndedEvent == eventId ) ++batchesEnded;
    }
};

/** Counts how often a browser says its list changed, and what the list was each time */
class CBrowserChangeRecorder : public CORE::CObserver
{
    public:

    UInt32 changes;
    UInt32 settled;
    UInt32 lastCount;
    MULTIPLAY::CGameBrowser* browser;

    CBrowserChangeRecorder( void ) : CORE::CObserver(), changes( 0 ), settled( 0 ), lastCount( 0 ), browser( GUCEF_NULL ) {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CBrowserChangeRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CGameBrowser::BrowserListChangedEvent == eventId )
            ++changes;
        else
        if ( MULTIPLAY::CGameBrowser::BrowserListSettledEvent == eventId )
        {
            ++settled;
            if ( GUCEF_NULL != browser )
                lastCount = browser->GetListingCount();
        }
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static MULTIPLAY::CGameListingEntry
MakeRow( const char* id, const char* name, MULTIPLAY::CGameListingEntry::TKind kind )
{
    MULTIPLAY::CGameListingEntry row;
    row.listingId = id;
    row.name = name;
    row.kind = kind;
    row.players = 1;
    row.maxPlayers = 3;
    row.hostName = "Alice";
    row.backendName = "fake";
    row.extra.Set( MULTIPLAY::CGameListingEntry::ExtraStatusText, "Forming" );
    return row;
}

/*-------------------------------------------------------------------------*/

/** A lobby of three members: Me (boss), Bob who answered, Carol who did not */
static MULTIPLAY::CAsyncLobbyView
MakeView( const char* id )
{
    MULTIPLAY::CAsyncLobbyView view;
    view.state.lobbyId = id;
    view.state.name = "Round one";
    view.state.phase = MULTIPLAY::LOBBYPHASE_OPEN;
    view.state.bossId = "me";
    view.localMemberId = "me";
    const char* ids[ 3 ] = { "me", "bob", "carol" };
    const char* names[ 3 ] = { "Me", "Bob", "Carol" };
    for ( int i=0; i<3; ++i )
    {
        MULTIPLAY::CLobbyMember member;
        member.id = ids[ i ];
        member.name = names[ i ];
        member.slot = i;
        member.ready = i < 2;
        member.statusText = i < 2 ? "Accepted" : "Invited";
        view.state.members.push_back( member );
        MULTIPLAY::CLobbySlot slot;
        slot.index = (UInt32) i;
        slot.team = (UInt32) i + 1;
        slot.kind = MULTIPLAY::LOBBYSLOTKIND_HUMAN;
        slot.memberId = ids[ i ];
        view.state.slots.push_back( slot );
    }
    view.presentation.leaveText = "Cancel game";
    view.presentation.canLeave = true;
    view.presentation.canKick = true;
    view.presentation.note = "Nobody can vote yet";
    return view;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FailureOf( const CORE::CFutureResult& future )
{
    CORE::CTaskPtr task = future.GetResult( 0 );
    return task.IsNULL() ? CORE::CString( "no task" ) : task->GetTaskStatusExtraInfo();
}

/*-------------------------------------------------------------------------*/

void
PerformAsyncLobbyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING async lobby TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "AsyncLobby" );

    GUCEF_TESTFW_TESTCASE( "Test: The listing provider reports the rows of the backend that pass the filter and follows their changes" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            backend.rows.push_back( MakeRow( "a", "Alpha", MULTIPLAY::CGameListingEntry::KIND_LOBBY ) );
            backend.rows.push_back( MakeRow( "b", "Beta", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS ) );
            MULTIPLAY::CAsyncLobbyListingProvider provider( backend, "fake-rows" );
            CAsyncListingRecorder recorder;
            recorder.SubscribeTo( &provider );
            ASSERT_TRUE( "fake-rows" == provider.GetProviderName() && !provider.IsActive() );

            MULTIPLAY::CGameListingFilter filter;
            ASSERT_TRUE( provider.Start( filter ) && provider.IsActive() );
            ASSERT_TRUE( 2 == provider.GetListingCount() && 2 == recorder.added );
            MULTIPLAY::CGameListingEntry row;
            ASSERT_TRUE( provider.GetListing( "a", row ) && "Alpha" == row.name && "Forming" == row.extra.GetValueAlways( MULTIPLAY::CGameListingEntry::ExtraStatusText ).AsString() );

            // a row changes, one goes, one comes
            backend.rows[ 0 ].players = 2;
            backend.rows.erase( backend.rows.begin() + 1 );
            backend.rows.push_back( MakeRow( "c", "Gamma", MULTIPLAY::CGameListingEntry::KIND_LOBBY ) );
            backend.ChangeRows();
            ASSERT_TRUE( 2 == provider.GetListingCount() && 3 == recorder.added && 1 == recorder.updated && 1 == recorder.removed );
            ASSERT_TRUE( provider.GetListing( "a", row ) && 2 == row.players );
            ASSERT_FALSE( provider.GetListing( "b", row ) );

            // Refresh asks the backend
            ASSERT_TRUE( provider.Refresh() && 1 == backend.refreshes );

            // stopped, it forgets the rows and ignores the backend
            provider.Stop();
            ASSERT_TRUE( !provider.IsActive() && 0 == provider.GetListingCount() );
            backend.ChangeRows();
            ASSERT_TRUE( 0 == provider.GetListingCount() );

            // the filter of the start is applied to the rows
            MULTIPLAY::CGameListingFilter lobbies;
            lobbies.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY;
            ASSERT_TRUE( provider.Start( lobbies ) );
            ASSERT_TRUE( 2 == provider.GetListingCount() );
            MULTIPLAY::CGameListingFilter text;
            text.text = "gam";
            ASSERT_TRUE( provider.Start( text ) && 1 == provider.GetListingCount() && provider.GetListing( "c", row ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A browser shows the rows of the backend like those of any provider" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            backend.rows.push_back( MakeRow( "a", "Alpha", MULTIPLAY::CGameListingEntry::KIND_LOBBY ) );
            MULTIPLAY::CGameBrowser browser;
            browser.SetPingEnabled( false );
            CORE::CTSharedPtr< MULTIPLAY::CAsyncLobbyListingProvider, MT::CMutex > shared( GUCEF_NEW MULTIPLAY::CAsyncLobbyListingProvider( backend, "fake" ) );
            browser.AddProvider( MULTIPLAY::CIGameListingProviderPtr( shared ) );
            ASSERT_TRUE( browser.Start() );
            ASSERT_TRUE( 1 == browser.GetListingCount() );
            backend.rows.push_back( MakeRow( "b", "Beta", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS ) );
            backend.ChangeRows();
            ASSERT_TRUE( 2 == browser.GetListingCount() );
            MULTIPLAY::CGameListingEntry row;
            ASSERT_TRUE( browser.GetListing( "b", row ) && -1 == row.pingMs && "fake" == row.backendName );
            browser.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Hundreds of rows that arrive together reach the browser as one set, not one rebuild of the list per row, and a change of one row among them is still told" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            for ( UInt32 i=0; i<500; ++i )
            {
                const CORE::CString id = "row-" + CORE::UInt32ToString( i );
                backend.rows.push_back( MakeRow( id.C_String(), id.C_String(), i % 3 ? MULTIPLAY::CGameListingEntry::KIND_LOBBY : MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS ) );
            }
            MULTIPLAY::CGameBrowser browser;
            browser.SetPingEnabled( false );
            CORE::CTSharedPtr< MULTIPLAY::CAsyncLobbyListingProvider, MT::CMutex > shared( GUCEF_NEW MULTIPLAY::CAsyncLobbyListingProvider( backend, "fake" ) );
            CAsyncListingRecorder providerEvents;
            providerEvents.SubscribeTo( shared.GetPointerAlways() );
            CBrowserChangeRecorder browserEvents;
            browserEvents.browser = &browser;
            browserEvents.SubscribeTo( &browser );
            browser.AddProvider( MULTIPLAY::CIGameListingProviderPtr( shared ) );
            ASSERT_TRUE( browser.Start() );
            ASSERT_TRUE( 500 == browser.GetListingCount() );
            // the provider still reports each row; the browser took the 500 rows as one set: each listing is told once and the set is settled once, with the whole list in it
            ASSERT_TRUE( 500 == providerEvents.added && 1 == providerEvents.batchesEnded );
            ASSERT_TRUE( 500 == browserEvents.changes && 1 == browserEvents.settled && 500 == browserEvents.lastCount );

            // one row changes, one goes: still told, as one set
            const UInt32 before = browserEvents.settled;
            const UInt32 changesBefore = browserEvents.changes;
            backend.rows[ 7 ].name = "Renamed";
            backend.rows.erase( backend.rows.begin() + 8 );
            backend.ChangeRows();
            ASSERT_TRUE( 499 == browser.GetListingCount() );
            ASSERT_TRUE( browserEvents.settled == before + 1 && browserEvents.changes == changesBefore + 2 );
            MULTIPLAY::CGameListingEntry row;
            ASSERT_TRUE( browser.GetListing( "row-7", row ) && "Renamed" == row.name );
            ASSERT_FALSE( browser.GetListing( "row-8", row ) );
            // nothing changed: nothing is told
            backend.ChangeRows();
            ASSERT_TRUE( browserEvents.settled == before + 1 && browserEvents.changes == changesBefore + 2 );
            browser.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The service connects when the backend is available and says why when it is not" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
            ASSERT_TRUE( "fake" == service.GetServiceName() && !service.IsConnected() );
            MULTIPLAY::CLobbyServiceCapabilities capabilities = service.GetCapabilities();
            ASSERT_TRUE( capabilities.voting && !capabilities.relay && !capabilities.chat && !capabilities.matchmaking && !capabilities.inProgressSpectating );

            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "me";
            identity.displayName = "Me";
            backend.available = false;
            backend.unavailableReason = "No mailbox is set up yet";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( service.Connect( identity, future ) );
            ASSERT_TRUE( !service.IsConnected() && future.IsReady() && "No mailbox is set up yet" == FailureOf( future ) );

            backend.available = true;
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.Connect( identity, future ) && service.IsConnected() && future.IsReady() );
            service.Disconnect();
            ASSERT_FALSE( service.IsConnected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Browse gives the rows that pass the filter" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            backend.rows.push_back( MakeRow( "a", "Alpha", MULTIPLAY::CGameListingEntry::KIND_LOBBY ) );
            backend.rows.push_back( MakeRow( "b", "Beta", MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS ) );
            MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "me";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.Connect( identity, future ) );

            MULTIPLAY::CGameListingFilter filter;
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.Browse( filter, future ) );
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( MULTIPLAY::CGameListingList::TryGetFromFuture( future, list ) && 2 == list.listings.size() );

            filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS;
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.Browse( filter, future ) );
            ASSERT_TRUE( MULTIPLAY::CGameListingList::TryGetFromFuture( future, list ) && 1 == list.listings.size() && "b" == list.listings[ 0 ].listingId );

            // a service that is not connected browses nothing
            service.Disconnect();
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( service.Browse( filter, future ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Creating a lobby hands the params with the invitations to the backend and gives the lobby, a refusal gives its reason" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "me";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.Connect( identity, future ) );

            MULTIPLAY::CLobbyCreateParams params;
            params.name = "Round one";
            params.maxPlayers = 3;
            params.invitations.push_back( MULTIPLAY::CLobbyInvitation( "Bob", "bob@example.org" ) );
            params.invitations.push_back( MULTIPLAY::CLobbyInvitation( "Carol", "carol@example.org", "abcd" ) );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.CreateLobby( params, future ) );
            MULTIPLAY::CILobbyPtr lobby;
            ASSERT_TRUE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby ) && !lobby.IsNULL() );
            ASSERT_TRUE( "made-1" == lobby->GetState().lobbyId && "Me" == lobby->GetState().members[ 0 ].name && lobby->IsBoss() );
            ASSERT_TRUE( 2 == backend.lastCreate.invitations.size() && "carol@example.org" == backend.lastCreate.invitations[ 1 ].contact && "abcd" == backend.lastCreate.invitations[ 1 ].code );

            // the invitations survive the data tree of the params
            CORE::CDataNode tree;
            ASSERT_TRUE( params.SaveConfig( tree ) );
            MULTIPLAY::CLobbyCreateParams loaded;
            ASSERT_TRUE( loaded.LoadConfig( tree ) && params.invitations == loaded.invitations );

            backend.refusal = "The mailbox is not working";
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( service.CreateLobby( params, future ) );
            ASSERT_TRUE( "The mailbox is not working" == FailureOf( future ) );
            lobby = MULTIPLAY::CILobbyPtr();
            ASSERT_FALSE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Joining opens the lobby of the id, the same object while it is held, spectating first when asked" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            backend.lobbies[ "g1" ] = MakeView( "g1" );
            MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "me";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.Connect( identity, future ) );

            MULTIPLAY::CILobbyPtr first, second;
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.JoinLobby( "g1", CORE::CString::Empty, false, future ) );
            ASSERT_TRUE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, first ) && !first.IsNULL() );
            ASSERT_TRUE( "Round one" == first->GetState().name && 3 == first->GetState().members.size() && 0 == backend.calls.size() );
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.JoinLobby( "g1", CORE::CString::Empty, false, future ) );
            ASSERT_TRUE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, second ) && first.GetPointerAlways() == second.GetPointerAlways() );

            // released, the next join is a new object; as a spectator the backend is asked first
            service.ReleaseLobby( "g1" );
            first = MULTIPLAY::CILobbyPtr();
            second = MULTIPLAY::CILobbyPtr();
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service.JoinLobby( "g1", CORE::CString::Empty, true, future ) );
            ASSERT_TRUE( backend.Called( "spectate g1 yes" ) && MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, first ) );

            // an id the backend does not know
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( service.JoinLobby( "nope", CORE::CString::Empty, false, future ) );
            ASSERT_TRUE( FailureOf( future ).Length() > 0 );

            // a spectator that is refused makes the join fail with the reason
            backend.refusal = "You cannot watch this game";
            future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            service.ReleaseLobby( "g1" );
            ASSERT_FALSE( service.JoinLobby( "g1", CORE::CString::Empty, true, future ) );
            ASSERT_TRUE( "You cannot watch this game" == FailureOf( future ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lobby shows the snapshot of the backend, the presentation and what changed" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            backend.lobbies[ "g1" ] = MakeView( "g1" );
            MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
            MULTIPLAY::CILobbyPtr lobby = service.OpenLobby( "g1" );
            ASSERT_TRUE( !lobby.IsNULL() && service.OpenLobby( "nope" ).IsNULL() );
            CLobbyRecorder recorder;
            recorder.Watch( lobby );

            ASSERT_TRUE( "me" == lobby->GetLocalMemberId() && lobby->IsBoss() );
            ASSERT_TRUE( "Accepted" == lobby->GetState().members[ 1 ].statusText && "Invited" == lobby->GetState().members[ 2 ].statusText );
            MULTIPLAY::CLobbyPresentation presentation = lobby->GetPresentation();
            ASSERT_TRUE( "Cancel game" == presentation.leaveText && presentation.canKick && "Nobody can vote yet" == presentation.note );

            // Carol answers
            MULTIPLAY::CAsyncLobbyView view = MakeView( "g1" );
            view.state.members[ 2 ].ready = true;
            view.state.members[ 2 ].statusText = "Accepted";
            view.presentation.note = "Everybody answered";
            backend.ChangeLobby( "g1", view );
            ASSERT_TRUE( 1 == recorder.stateChanges && lobby->GetState().members[ 2 ].ready && "Everybody answered" == lobby->GetPresentation().note );

            // a vote starts, then ends
            view.state.activeVote.id = "v1";
            view.state.activeVote.what = "forcestart";
            view.state.activeVote.proposer = "bob";
            view.state.activeVote.yes = 1;
            view.state.activeVote.needed = 2;
            backend.ChangeLobby( "g1", view );
            ASSERT_TRUE( 1 == recorder.votesStarted && "v1" == recorder.lastVote.id && 0 == recorder.votesEnded );
            view.state.activeVote.Clear();
            view.lastVotePassed = true;
            backend.ChangeLobby( "g1", view );
            ASSERT_TRUE( 1 == recorder.votesEnded && recorder.lastVotePassed );

            // the lobby is over: told once
            view.closed = true;
            view.closedReason = "Cancelled by Alice";
            backend.ChangeLobby( "g1", view );
            backend.ChangeLobby( "g1", view );
            ASSERT_TRUE( 1 == recorder.closed && "Cancelled by Alice" == recorder.closeReason );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The requests of a lobby are actions of the backend, a refusal is told with its reason" )
        try
        {
            CFakeAsyncLobbyBackend backend;
            backend.lobbies[ "g1" ] = MakeView( "g1" );
            MULTIPLAY::CAsyncLobbyService service( backend, "fake" );
            MULTIPLAY::CILobbyPtr lobby = service.OpenLobby( "g1" );
            CLobbyRecorder recorder;
            recorder.Watch( lobby );

            ASSERT_TRUE( lobby->SetReady( true ) && backend.Called( "answer g1 yes" ) );
            ASSERT_TRUE( lobby->SetSpectator( true ) && backend.Called( "spectate g1 yes" ) );
            ASSERT_TRUE( lobby->Kick( "carol" ) && backend.Called( "kick g1 carol" ) );
            ASSERT_TRUE( lobby->RequestStart() && backend.Called( "start g1" ) );
            ASSERT_TRUE( lobby->ForceStart() && backend.Called( "forcestart g1" ) );
            ASSERT_TRUE( lobby->Vote( "v1", false ) && backend.Called( "vote g1 v1 no" ) );
            ASSERT_TRUE( lobby->Invite( "Dave", "dave@example.org", "ff" ) && backend.Called( "invite g1 Dave dave@example.org ff" ) );
            ASSERT_TRUE( lobby->Leave() && backend.Called( "leave g1" ) );
            ASSERT_TRUE( 0 == recorder.refusals );

            // what the kind of lobby has no meaning for is refused without asking the backend
            const size_t calls = backend.calls.size();
            ASSERT_FALSE( lobby->RequestSlot( 2 ) );
            ASSERT_TRUE( 1 == recorder.refusals && "slot" == recorder.refusedWhat && !recorder.refusedReason.IsNULLOrEmpty() );
            ASSERT_FALSE( lobby->SetFaction( "x" ) );
            ASSERT_FALSE( lobby->SetColour( "red" ) );
            ASSERT_FALSE( lobby->TransferBoss( "bob" ) );
            ASSERT_FALSE( lobby->SetSlotKind( 1, MULTIPLAY::LOBBYSLOTKIND_AI, "ai" ) );
            ASSERT_FALSE( lobby->SetSetting( "map", "ice" ) );
            ASSERT_FALSE( lobby->ProposeSetting( "map", "ice" ) );
            ASSERT_TRUE( 7 == recorder.refusals && calls == backend.calls.size() );

            // the backend refuses
            backend.refusal = "Only the players who accepted can propose this.";
            ASSERT_FALSE( lobby->ForceStart() );
            ASSERT_TRUE( 8 == recorder.refusals && "forceStart" == recorder.refusedWhat && "Only the players who accepted can propose this." == recorder.refusedReason );
            ASSERT_FALSE( lobby->SetReady( false ) );
            ASSERT_TRUE( 9 == recorder.refusals && "ready" == recorder.refusedWhat );
            ASSERT_FALSE( lobby->Kick( "bob" ) );
            ASSERT_TRUE( "kick" == recorder.refusedWhat );
            ASSERT_FALSE( lobby->Leave() );
            ASSERT_TRUE( "leave" == recorder.refusedWhat );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}
