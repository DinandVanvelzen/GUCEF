/*
 *  gucefMULTIPLAY_EosTestApp: Application for testing the Epic Online Services plugins of gucefMULTIPLAY, gucefDRN and gucefAUTH
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLobbyServiceListingProvider.h"
#define GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H ? */

#include "TestEosCommon.h"
#include "TestEosLobby.h"

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::EOS;
using namespace GUCEF::MULTIPLAYPLUGIN::EOS;
using namespace EosFake;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosLobbyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS lobby service TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosLobby" );

    GUCEF_TESTFW_TESTCASE( "Test: Connect needs the SDK, a platform, a login and an account that fits, and says which is missing" )
        try
        {
            // Without the SDK of the build and without tables of functions
            {
                CEosLobbyService service;
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                MULTIPLAY::CPlayerIdentity identity;
                ASSERT_FALSE( service.Connect( identity, future ) );
                ASSERT_TRUE( future.IsReady() );
                ASSERT_TRUE( FailureOf( future ).HasSubstr( "not part of this build" ) >= 0 );
                ASSERT_FALSE( service.IsConnected() );
                // and the calls say that there is no connection, they do not hang
                CORE::CFutureResult browse( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_FALSE( service.Browse( MULTIPLAY::CGameListingFilter(), browse ) );
                ASSERT_TRUE( browse.IsReady() );
                CORE::CFutureResult create( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_FALSE( service.CreateLobby( CEosFixture::MakeParams(), create ) );
                ASSERT_TRUE( create.IsReady() );
                CORE::CFutureResult join( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_FALSE( service.JoinLobby( "abc", "", false, join ) );
                ASSERT_TRUE( join.IsReady() );
            }

            CEosFixture fixture( 1, false );
            CEosPeerContext& player = fixture[ 0 ];
            PLUGINGLUE::EOS::SEosP2PShim p2p = player.peer.P2PShim();
            player.service.SetShim( player.peer.LobbyShim(), &p2p );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

            // the SDK is not there
            player.peer.sdkAvailable = false;
            ASSERT_FALSE( player.service.Connect( player.identity, future ) );
            ASSERT_TRUE( FailureOf( future ).HasSubstr( "not part of this build" ) >= 0 );
            player.peer.sdkAvailable = true;

            // the platform cannot be made
            player.peer.platformOk = false;
            player.peer.platformError = "fake: wrong sandbox";
            ASSERT_FALSE( player.service.Connect( player.identity, future ) );
            ASSERT_TRUE( FailureOf( future ).HasSubstr( "platform is not initialised" ) >= 0 && FailureOf( future ).HasSubstr( "fake: wrong sandbox" ) >= 0 );
            player.peer.platformOk = true;

            // nobody is logged in
            ASSERT_FALSE( player.service.Connect( player.identity, future ) );
            ASSERT_TRUE( FailureOf( future ).HasSubstr( "Not logged in" ) >= 0 );
            ASSERT_TRUE( 0 == PLUGINGLUE::EOS::CEosGlue::Instance()->GetPlatformCount() );

            // logged in, as another Epic account than the identity says
            ASSERT_TRUE( player.Login() );
            player.identity.accounts.SetAllowMultipleValues( false );
            player.identity.accounts.Set( "epic", "0000000000000000000000000000dead" );
            ASSERT_FALSE( player.service.Connect( player.identity, future ) );
            ASSERT_TRUE( FailureOf( future ).HasSubstr( "another Epic account" ) >= 0 );

            // and now it works
            player.identity.accounts.Set( "epic", player.peer.epicAccountId );
            ASSERT_TRUE( player.service.Connect( player.identity, future ) && PumpUntilReady( future ) );
            ASSERT_TRUE( player.service.IsConnected() && player.service.GetLocalUser() == player.peer.puid );
            ASSERT_TRUE( 1 == player.serviceRecorder.connected );
            CORE::CTaskPtr task = future.GetResult( 0 );
            const MULTIPLAY::CPlayerIdentity* known = CORE::CICloneable::CloneableAs< MULTIPLAY::CPlayerIdentity >( task->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != known && known->accounts.GetValueAlways( "epic" ).AsString() == player.peer.epicAccountId && known->accounts.GetValueAlways( "eos" ).AsString() == player.peer.puid );

            player.service.Disconnect();
            ASSERT_FALSE( player.service.IsConnected() );
            ASSERT_TRUE( 1 == player.serviceRecorder.disconnected );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The capabilities: voting, relay and spectating in progress; no chat, no matchmaking" )
        try
        {
            CEosLobbyService service;
            MULTIPLAY::CLobbyServiceCapabilities capabilities = service.GetCapabilities();
            ASSERT_TRUE( capabilities.voting && capabilities.relay && capabilities.inProgressSpectating );
            ASSERT_FALSE( capabilities.chat );
            ASSERT_FALSE( capabilities.matchmaking );
            ASSERT_TRUE( service.GetServiceName() == "eos" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A lobby is made hidden and empty and published with its data in one update, the owner is the boss" )
        try
        {
            CEosFixture fixture( 1 );
            REQUIRE_TRUE( fixture[ 0 ].service.IsConnected() );
            MULTIPLAY::CLobbyCreateParams params = CEosFixture::MakeParams( "Dust duel" );
            MULTIPLAY::CILobbyPtr lobby = fixture[ 0 ].Create( params );
            REQUIRE_TRUE( !lobby.IsNULL() );
            ASSERT_TRUE( 1 == fixture[ 0 ].serviceRecorder.lobbyJoined );

            // created invite only and without anything, and then published
            ASSERT_TRUE( EOSLOBBYPERM_INVITE_ONLY == fixture[ 0 ].peer.lastCreate.permission );
            ASSERT_TRUE( 1 == fixture[ 0 ].peer.createCalls && 1 == fixture[ 0 ].peer.updateCalls );
            ASSERT_TRUE( fixture[ 0 ].peer.lastCreate.enableJoinById && !fixture[ 0 ].peer.lastCreate.disableHostMigration );
            ASSERT_TRUE( fixture[ 0 ].peer.lastCreate.bucketId == "gucef-testgame" );
            ASSERT_TRUE( 4 + 8 == fixture[ 0 ].peer.lastCreate.maxMembers );

            SFakeLobby* fake = fixture.world.FindLobby( LobbyIdOf( lobby ) );
            REQUIRE_TRUE( GUCEF_NULL != fake );
            ASSERT_TRUE( EOSLOBBYPERM_PUBLIC_ADVERTISED == fake->permission );
            ASSERT_TRUE( fake->owner == fixture[ 0 ].peer.puid && 1 == fake->members.size() );
            CEosLobbySummary summary;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::DecodeSummary( fake->attributes, fake->owner, summary, reason ) );
            ASSERT_TRUE( summary.name == "Dust duel" && summary.gameId == "testgame" && summary.gameVersion == "1.0" && 4 == summary.maxPlayers && 1 == summary.players );
            ASSERT_TRUE( summary.stamp == fixture[ 0 ].peer.puid && summary.hostName == "Player 0" && !summary.passworded );

            // a member attribute of the owner: its name
            CORE::CString name;
            ASSERT_TRUE( CEosLobbyCodec::GetString( fake->members[ 0 ].attributes, CEosLobbyCodec::MemberKeyName, name ) && "Player 0" == name );

            // the state: the owner is the boss and the only member
            const MULTIPLAY::CLobbyState& state = lobby->GetState();
            ASSERT_TRUE( state.name == "Dust duel" && MULTIPLAY::LOBBYPHASE_OPEN == state.phase );
            ASSERT_TRUE( state.bossId == fixture[ 0 ].peer.puid && lobby->IsBoss() && lobby->GetLocalMemberId() == fixture[ 0 ].peer.puid );
            ASSERT_TRUE( 1 == state.members.size() && MULTIPLAY::LOBBYMEMBERROLE_BOSS == state.members[ 0 ].role );
            ASSERT_TRUE( state.GetSetting( "map" ) == "dust" && state.GetSetting( "speed" ) == "3" );
            ASSERT_TRUE( 4 == state.slots.size() && state.slots[ 0 ].memberId == fixture[ 0 ].peer.puid );
            ASSERT_TRUE( lobby->GetSchema().settings.size() == params.schema.settings.size() && lobby->GetSchema().uniqueColours );
            ASSERT_TRUE( Eos( lobby ).IsOwner() && Eos( lobby ).IsReady() && Eos( lobby ).IsMember() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The permission of a lobby follows the settings: public, presence, invite only, and a lobby that is not public is not listed" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings settings = CEosFixture::MakeFastSettings();
            settings.permission = "presence";
            settings.presenceEnabled = true;
            fixture[ 0 ].service.SetSettings( settings );
            MULTIPLAY::CILobbyPtr presence = fixture[ 0 ].Create( CEosFixture::MakeParams( "Presence" ) );
            REQUIRE_TRUE( !presence.IsNULL() );
            ASSERT_TRUE( EOSLOBBYPERM_JOIN_VIA_PRESENCE == fixture.world.FindLobby( LobbyIdOf( presence ) )->permission );
            ASSERT_TRUE( fixture[ 0 ].peer.lastCreate.presenceEnabled );

            settings.permission = "";
            settings.presenceEnabled = false;
            fixture[ 0 ].service.SetSettings( settings );
            MULTIPLAY::CLobbyCreateParams privateParams = CEosFixture::MakeParams( "Private" );
            privateParams.isPublic = false;
            MULTIPLAY::CILobbyPtr hidden = fixture[ 0 ].Create( privateParams );
            REQUIRE_TRUE( !hidden.IsNULL() );
            ASSERT_TRUE( EOSLOBBYPERM_INVITE_ONLY == fixture.world.FindLobby( LobbyIdOf( hidden ) )->permission );

            // neither is found by a search of the other player
            MULTIPLAY::CGameListingFilter ofTheGame;
            ofTheGame.gameId = "testgame";
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( Browse( fixture[ 1 ], ofTheGame, list ) );
            ASSERT_TRUE( 0 == list.listings.size() );

            // a lobby of a service that does not name a permission is public
            MULTIPLAY::CILobbyPtr open = fixture[ 1 ].Create( CEosFixture::MakeParams( "Open" ) );
            REQUIRE_TRUE( !open.IsNULL() );
            ASSERT_TRUE( Browse( fixture[ 0 ], ofTheGame, list ) );
            ASSERT_TRUE( 1 == list.listings.size() && list.listings[ 0 ].name == "Open" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Browsing finds the lobbies with their data and takes the filters: name, full, password, game, version, kind and the bucket" )
        try
        {
            CEosFixture fixture( 4 );
            MULTIPLAY::CLobbyCreateParams alpha = CEosFixture::MakeParams( "Alpha ridge" );
            MULTIPLAY::CLobbyCreateParams beta = CEosFixture::MakeParams( "Beta basin" );
            beta.password = "open sesame";
            beta.maxPlayers = 2;
            MULTIPLAY::CLobbyCreateParams other = CEosFixture::MakeParams( "Other game" );
            other.gameId = "othergame";

            MULTIPLAY::CILobbyPtr a = fixture[ 0 ].Create( alpha );
            MULTIPLAY::CILobbyPtr b = fixture[ 1 ].Create( beta );
            MULTIPLAY::CILobbyPtr c = fixture[ 2 ].Create( other );
            REQUIRE_TRUE( !a.IsNULL() && !b.IsNULL() && !c.IsNULL() );
            ASSERT_TRUE( fixture.world.FindLobby( LobbyIdOf( c ) )->bucketId == "gucef-othergame" );

            // the bucket of a game is the one of the filter
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( Browse( fixture[ 3 ], filter, list ) );
            ASSERT_TRUE( fixture[ 3 ].peer.lastSearchBucket == "gucef-testgame" );
            ASSERT_TRUE( 2 == list.listings.size() );
            const MULTIPLAY::CGameListingEntry* found = GUCEF_NULL;
            for ( size_t i=0; i<list.listings.size(); ++i )
                if ( list.listings[ i ].name == "Beta basin" )
                    found = &list.listings[ i ];
            REQUIRE_TRUE( GUCEF_NULL != found );
            ASSERT_TRUE( found->listingId == LobbyIdOf( b ) && found->gameId == "testgame" && found->gameVersion == "1.0" );
            ASSERT_TRUE( found->passworded && 2 == found->maxPlayers && 1 == found->players && found->hostName == "Player 1" );
            ASSERT_TRUE( found->backendName == "eos" && MULTIPLAY::CGameListingEntry::KIND_LOBBY == found->kind );
            ASSERT_TRUE( found->extra.GetValueAlways( "lobbyService" ).AsString() == "eos" && found->extra.GetValueAlways( "ownerPuid" ).AsString() == fixture[ 1 ].peer.puid );
            ASSERT_TRUE( 1 == found->endpoints.size() && found->endpoints[ 0 ].HasSubstr( "eos-p2p://" + fixture[ 1 ].peer.puid + "/" ) == 0 );
            ASSERT_TRUE( found->extra.GetValueAlways( "set.map" ).AsString() == "dust" );

            // the name is found anywhere in it, whatever the case
            filter.text = "RIDGE";
            ASSERT_TRUE( Browse( fixture[ 3 ], filter, list ) );
            ASSERT_TRUE( 1 == list.listings.size() && list.listings[ 0 ].name == "Alpha ridge" );
            filter.text.Clear();

            // no password
            filter.noPassword = true;
            ASSERT_TRUE( Browse( fixture[ 3 ], filter, list ) );
            ASSERT_TRUE( 1 == list.listings.size() && list.listings[ 0 ].name == "Alpha ridge" );
            filter.noPassword = false;

            // not full: fill the lobby with two places
            MULTIPLAY::CILobbyPtr guest = fixture[ 3 ].Join( LobbyIdOf( b ), "open sesame" );
            REQUIRE_TRUE( !guest.IsNULL() );
            CMemberCount two( b, 2 );
            ASSERT_TRUE( PumpUntil( two ) );
            PumpFor( 60 );
            filter.notFull = true;
            ASSERT_TRUE( Browse( fixture[ 2 ], filter, list ) );
            bool betaListed = false;
            for ( size_t i=0; i<list.listings.size(); ++i )
                betaListed = betaListed || list.listings[ i ].name == "Beta basin";
            ASSERT_TRUE( 1 == list.listings.size() && !betaListed );
            filter.notFull = false;

            // the version of the game, the kind
            filter.gameVersion = "2.0";
            ASSERT_TRUE( Browse( fixture[ 2 ], filter, list ) );
            ASSERT_TRUE( 0 == list.listings.size() );
            filter.gameVersion.Clear();
            filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS;
            ASSERT_TRUE( Browse( fixture[ 2 ], filter, list ) );
            ASSERT_TRUE( 0 == list.listings.size() );
            filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY;
            ASSERT_TRUE( Browse( fixture[ 2 ], filter, list ) );
            ASSERT_TRUE( 2 == list.listings.size() );

            // a search of another bucket finds the lobby of that bucket
            filter = MULTIPLAY::CGameListingFilter();
            filter.gameId = "othergame";
            ASSERT_TRUE( Browse( fixture[ 3 ], filter, list ) );
            ASSERT_TRUE( 1 == list.listings.size() && list.listings[ 0 ].name == "Other game" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Joining: the member is accepted by the owner, both see each other, the events say who came" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CMemberCount guestSees( pair.guest, 2 );
            ASSERT_TRUE( PumpUntil( guestSees ) );

            const MULTIPLAY::CLobbyState& hostState = pair.host->GetState();
            const MULTIPLAY::CLobbyState& guestState = pair.guest->GetState();
            ASSERT_TRUE( hostState.bossId == fixture[ 0 ].peer.puid && guestState.bossId == fixture[ 0 ].peer.puid );
            ASSERT_TRUE( !pair.guest->IsBoss() && pair.host->IsBoss() );
            const MULTIPLAY::CLobbyMember* member = guestState.FindMember( fixture[ 1 ].peer.puid );
            REQUIRE_TRUE( GUCEF_NULL != member );
            ASSERT_TRUE( "Player 1" == member->name && MULTIPLAY::LOBBYMEMBERROLE_PLAYER == member->role && !member->ready );
            ASSERT_TRUE( 1 == hostState.members[ 1 ].joinOrder && member->slot >= 0 );
            ASSERT_TRUE( 1 == pair.hostEvents.joined.size() && pair.hostEvents.joined[ 0 ] == fixture[ 1 ].peer.puid );
            ASSERT_TRUE( pair.hostEvents.HasPath( "members" ) );
            ASSERT_TRUE( 1 == fixture[ 1 ].serviceRecorder.lobbyJoined );
            ASSERT_TRUE( Eos( pair.guest ).IsReady() && !Eos( pair.guest ).IsOwner() );
            ASSERT_TRUE( pair.guest->GetSchema().settings.size() == hostState.settings.GetKeyCount() );
            ASSERT_TRUE( 0 == pair.hostEvents.wrongThreadEvents && 0 == pair.guestEvents.wrongThreadEvents );

            // the member sees the same settings as the owner
            ASSERT_TRUE( guestState.GetSetting( "map" ) == "dust" && guestState.GetSetting( "speed" ) == "3" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Ready flags travel as member attributes and show for everybody; the countdown stops when somebody is not ready any more" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CMemberCount guestSees( pair.guest, 2 );
            ASSERT_TRUE( PumpUntil( guestSees ) );

            ASSERT_TRUE( pair.guest->SetReady( true ) );
            CMemberReady hostSeesReady( pair.host, fixture[ 1 ].peer.puid, true );
            CMemberReady guestSeesReady( pair.guest, fixture[ 1 ].peer.puid, true );
            ASSERT_TRUE( PumpUntil( hostSeesReady ) && PumpUntil( guestSeesReady ) );
            ASSERT_TRUE( pair.guestEvents.HasPath( "members" ) );

            // the flag is a member attribute of the guest, the owner reads it
            SFakeLobby* fake = fixture.world.FindLobby( LobbyIdOf( pair.host ) );
            CORE::CString text;
            ASSERT_TRUE( CEosLobbyCodec::GetString( fake->members[ 1 ].attributes, CEosLobbyCodec::MemberKeyReady, text ) && text.HasSubstr( ":1" ) > 0 );

            // the owner is ready and starts: the countdown runs on every member
            ASSERT_TRUE( pair.host->SetReady( true ) );
            ASSERT_TRUE( pair.host->RequestStart() );
            CPhaseIs counting( pair.guest, MULTIPLAY::LOBBYPHASE_COUNTDOWN );
            ASSERT_TRUE( PumpUntil( counting ) );
            ASSERT_TRUE( !pair.guestEvents.countdowns.empty() && !pair.hostEvents.countdowns.empty() && 2 == pair.hostEvents.countdowns[ 0 ] );

            // somebody gets unready: the countdown is cancelled
            ASSERT_TRUE( pair.guest->SetReady( false ) );
            CPhaseIs open( pair.host, MULTIPLAY::LOBBYPHASE_OPEN );
            CPhaseIs guestOpen( pair.guest, MULTIPLAY::LOBBYPHASE_OPEN );
            ASSERT_TRUE( PumpUntil( open ) && PumpUntil( guestOpen ) );
            ASSERT_TRUE( -1 == pair.hostEvents.countdowns.back() );
            PumpFor( 40 );
            ASSERT_TRUE( -1 == pair.guestEvents.countdowns.back() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
