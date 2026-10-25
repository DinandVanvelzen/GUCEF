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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

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

/** Waits until the lobby of a member has closed */
class CClosed : public CCondition
{
    public:
    CLobbyRecorder& recorder;
    explicit CClosed( CLobbyRecorder& r ) : recorder( r ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return recorder.closed > 0; }
};

/** Waits until a lobby has a number of refusals */
class CRefused : public CCondition
{
    public:
    CLobbyRecorder& recorder;
    UInt32 count;
    CRefused( CLobbyRecorder& r, UInt32 c ) : recorder( r ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return recorder.refusals >= count; }
};

/** Waits until the lobby of a member is run by it */
class CIsOwner : public CCondition
{
    public:
    GUCEF::MULTIPLAY::CILobbyPtr lobby;
    explicit CIsOwner( GUCEF::MULTIPLAY::CILobbyPtr l ) : lobby( l ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return Eos( lobby ).IsOwner(); }
};

/** Waits until a lobby of EOS has an owner */
class COwnerIs : public CCondition
{
    public:
    EosFake::CFakeEosWorld& world;
    CORE::CString lobbyId;
    CORE::CString owner;
    COwnerIs( EosFake::CFakeEosWorld& w, const CORE::CString& l, const CORE::CString& o ) : world( w ), lobbyId( l ), owner( o ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        SFakeLobby* lobby = world.FindLobby( lobbyId );
        return GUCEF_NULL != lobby && lobby->owner == owner;
    }
};

/** Waits until the lobbies of the world are gone */
class CNoLobbies : public CCondition
{
    public:
    EosFake::CFakeEosWorld& world;
    explicit CNoLobbies( EosFake::CFakeEosWorld& w ) : world( w ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return 0 == world.GetLobbyCount(); }
};


/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosLobbyRulesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS lobby rules TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosLobbyRules" );

    GUCEF_TESTFW_TESTCASE( "Test: The boss changes a setting at once, a setting only the boss changes is refused to a member with the reason" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );

            ASSERT_TRUE( pair.host->SetSetting( "map", "ice" ) );
            CSettingIs hostHas( pair.host, "map", "ice" );
            CSettingIs guestHas( pair.guest, "map", "ice" );
            ASSERT_TRUE( PumpUntil( hostHas ) && PumpUntil( guestHas ) );
            ASSERT_TRUE( pair.hostEvents.HasPath( "settings/map" ) && pair.guestEvents.HasPath( "settings/map" ) );

            // a member cannot: the owner says no and why, and the member hears it
            ASSERT_TRUE( pair.guest->SetSetting( "map", "dust" ) );
            CRefused refused( pair.guestEvents, 1 );
            ASSERT_TRUE( PumpUntil( refused ) );
            ASSERT_TRUE( pair.guestEvents.HasRefusal( "setting" ) && !pair.guestEvents.refusalReasons[ 0 ].IsNULLOrEmpty() );
            ASSERT_TRUE( pair.host->GetState().GetSetting( "map" ) == "ice" );

            // a value that is not one of the options: refused to the boss as well
            ASSERT_TRUE( pair.host->SetSetting( "map", "lava" ) );
            ASSERT_TRUE( pair.hostEvents.HasRefusal( "setting" ) );
            ASSERT_TRUE( pair.host->GetState().GetSetting( "map" ) == "ice" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A setting with the vote policy becomes a vote when a member changes it, it passes with the vote of the boss" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );

            ASSERT_TRUE( pair.guest->SetSetting( "speed", "5" ) );
            ASSERT_TRUE( PumpUntilCount( pair.hostEvents.votesStarted, 1 ) && PumpUntilCount( pair.guestEvents.votesStarted, 1 ) );
            ASSERT_TRUE( pair.host->GetState().activeVote.IsActive() && pair.host->GetState().activeVote.proposer == fixture[ 1 ].peer.puid );
            ASSERT_TRUE( "speed" == pair.host->GetState().activeVote.key && "5" == pair.host->GetState().activeVote.value );
            ASSERT_TRUE( pair.host->GetState().GetSetting( "speed" ) == "3" );

            ASSERT_TRUE( pair.host->Vote( pair.hostEvents.lastVoteId, true ) );
            CSettingIs passed( pair.guest, "speed", "5" );
            ASSERT_TRUE( PumpUntil( passed ) );
            ASSERT_TRUE( PumpUntilCount( pair.hostEvents.votesEnded, 1 ) && PumpUntilCount( pair.guestEvents.votesEnded, 1 ) );
            ASSERT_TRUE( pair.hostEvents.lastVotePassed && pair.guestEvents.lastVotePassed );
            ASSERT_TRUE( !pair.guest->GetState().activeVote.IsActive() && pair.host->GetState().GetSetting( "speed" ) == "5" );

            // and a vote that is voted down changes nothing
            ASSERT_TRUE( pair.guest->ProposeSetting( "speed", "2" ) );
            ASSERT_TRUE( PumpUntilCount( pair.hostEvents.votesStarted, 2 ) );
            ASSERT_TRUE( pair.host->Vote( pair.hostEvents.lastVoteId, false ) );
            ASSERT_TRUE( PumpUntilCount( pair.hostEvents.votesEnded, 2 ) && PumpUntilCount( pair.guestEvents.votesEnded, 2 ) );
            ASSERT_TRUE( !pair.hostEvents.lastVotePassed && !pair.guestEvents.lastVotePassed );
            ASSERT_TRUE( pair.host->GetState().GetSetting( "speed" ) == "5" );

            // one that nobody votes on ends when its time is up
            ASSERT_TRUE( pair.guest->ProposeSetting( "speed", "1" ) );
            ASSERT_TRUE( PumpUntilCount( pair.hostEvents.votesStarted, 3 ) );
            ASSERT_TRUE( PumpUntilCount( pair.hostEvents.votesEnded, 3, 3000 ) && PumpUntilCount( pair.guestEvents.votesEnded, 3, 3000 ) );
            ASSERT_TRUE( !pair.hostEvents.lastVotePassed && pair.host->GetState().GetSetting( "speed" ) == "5" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Slots, factions and colours: the owner applies the rules, the members see the result, a taken colour is refused" )
        try
        {
            CEosFixture fixture( 3 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString hostId = fixture[ 0 ].peer.puid;
            CORE::CString guestId = fixture[ 1 ].peer.puid;

            ASSERT_TRUE( pair.guest->RequestSlot( 3 ) );
            ASSERT_TRUE( pair.host->SetColour( "red" ) );
            ASSERT_TRUE( pair.guest->SetFaction( "core" ) );
            class CSlotOf : public CCondition
            {
                public:
                MULTIPLAY::CILobbyPtr lobby;
                CORE::CString memberId;
                CORE::CString faction;
                CSlotOf( MULTIPLAY::CILobbyPtr l, const CORE::CString& m, const CORE::CString& f ) : lobby( l ), memberId( m ), faction( f ) {}
                virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
                {
                    const MULTIPLAY::CLobbySlot* slot = lobby->GetState().FindSlotOfMember( memberId );
                    return GUCEF_NULL != slot && 3 == slot->index && slot->faction == faction;
                }
            };
            CSlotOf guestSlot( pair.host, guestId, "core" );
            CSlotOf guestSlotAsSeen( pair.guest, guestId, "core" );
            ASSERT_TRUE( PumpUntil( guestSlot ) && PumpUntil( guestSlotAsSeen ) );
            ASSERT_TRUE( pair.hostEvents.HasPath( "slots" ) && pair.guestEvents.HasPath( "slots" ) );
            ASSERT_TRUE( pair.guest->GetState().FindSlotOfMember( hostId )->colour == "red" );

            // red is taken
            ASSERT_TRUE( pair.guest->SetColour( "red" ) );
            CRefused refused( pair.guestEvents, 1 );
            ASSERT_TRUE( PumpUntil( refused ) && pair.guestEvents.HasRefusal( "colour" ) );

            // a spectator has no slot
            ASSERT_TRUE( pair.guest->SetSpectator( true ) );
            class CSpectator : public CCondition
            {
                public:
                MULTIPLAY::CILobbyPtr lobby;
                CORE::CString memberId;
                CSpectator( MULTIPLAY::CILobbyPtr l, const CORE::CString& m ) : lobby( l ), memberId( m ) {}
                virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
                {
                    const MULTIPLAY::CLobbyMember* member = lobby->GetState().FindMember( memberId );
                    return GUCEF_NULL != member && member->IsSpectator() && GUCEF_NULL == lobby->GetState().FindSlotOfMember( memberId );
                }
            };
            CSpectator spectating( pair.host, guestId );
            CSpectator spectatingAsSeen( pair.guest, guestId );
            ASSERT_TRUE( PumpUntil( spectating ) && PumpUntil( spectatingAsSeen ) );
            ASSERT_TRUE( 1 == pair.host->GetState().GetSpectatorCount() && 1 == pair.host->GetState().GetPlayerCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The boss kicks a member with the reason, a member cannot; the kicked one can come back unless it is banned" )
        try
        {
            CEosFixture fixture( 3 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString guestId = fixture[ 1 ].peer.puid;
            CORE::CString lobbyId = LobbyIdOf( pair.host );

            // a member is refused
            ASSERT_TRUE( pair.guest->Kick( fixture[ 0 ].peer.puid ) );
            CRefused refused( pair.guestEvents, 1 );
            ASSERT_TRUE( PumpUntil( refused ) && pair.guestEvents.HasRefusal( "kick" ) && pair.guestEvents.refusalReasons[ 0 ] == "Only the boss kicks" );
            ASSERT_TRUE( 2 == fixture.world.FindLobby( lobbyId )->members.size() );

            // the boss kicks: the member hears why and is out of the lobby of EOS and of the state of the owner
            ASSERT_TRUE( pair.host->Kick( guestId ) );
            CClosed closed( pair.guestEvents );
            ASSERT_TRUE( PumpUntil( closed ) );
            ASSERT_TRUE( 1 == pair.guestEvents.closed && pair.guestEvents.closedReason == "You were kicked" );
            ASSERT_TRUE( !Eos( pair.guest ).IsMember() );
            CMemberCount alone( pair.host, 1 );
            ASSERT_TRUE( PumpUntil( alone ) );
            ASSERT_TRUE( 1 == pair.hostEvents.left.size() && pair.hostEvents.left[ 0 ] == guestId );
            ASSERT_TRUE( 1 == fixture[ 0 ].peer.kickCalls );

            // EOS wants an invitation for whoever was kicked
            pair.guest = MULTIPLAY::CILobbyPtr();
            MULTIPLAY::CILobbyPtr again = fixture[ 1 ].Join( lobbyId );
            ASSERT_TRUE( again.IsNULL() && fixture[ 1 ].reason.HasSubstr( "permission" ) >= 0 );

            // an invitation lets it in again
            CORE::CFutureResult none( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( Eos( pair.host ).InvitePlayer( guestId ) );
            ASSERT_TRUE( PumpUntilCount( fixture[ 1 ].serviceRecorder.invites, 1 ) );
            ASSERT_TRUE( fixture[ 1 ].serviceRecorder.lastInviteLobby == lobbyId && fixture[ 1 ].serviceRecorder.lastInviteSender == fixture[ 0 ].peer.puid );
            again = fixture[ 1 ].Join( lobbyId );
            ASSERT_TRUE( !again.IsNULL() );
            CMemberCount two( pair.host, 2 );
            ASSERT_TRUE( PumpUntil( two ) );

            // a ban is the lobby's own: whoever comes back is put out again, with the reason, also when EOS lets it join
            fixture.world.FindLobby( lobbyId )->rejoinAfterKickRequiresInvite = false;
            CLobbyRecorder againEvents;
            againEvents.SubscribeTo( again.GetPointerAlways() );
            ASSERT_TRUE( pair.host->Ban( guestId ) );
            CMemberCount aloneAgain( pair.host, 1 );
            CClosed againClosed( againEvents );
            ASSERT_TRUE( PumpUntil( aloneAgain ) && PumpUntil( againClosed ) );
            ASSERT_TRUE( againEvents.closedReason == "You are banned from the lobby" );
            again = MULTIPLAY::CILobbyPtr();
            again = fixture[ 1 ].Join( lobbyId );
            ASSERT_TRUE( again.IsNULL() && fixture[ 1 ].reason.HasSubstr( "banned" ) >= 0 );
            CEosMembers onlyHost( fixture.world, lobbyId, 1 );
            ASSERT_TRUE( PumpUntil( onlyHost ) );

            // and a third player is not affected
            MULTIPLAY::CILobbyPtr third = fixture[ 2 ].Join( lobbyId );
            ASSERT_TRUE( !third.IsNULL() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: When the owner leaves EOS makes another member the owner: it becomes the boss and the lobby goes on" )
        try
        {
            CEosFixture fixture( 4 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            MULTIPLAY::CILobbyPtr third = fixture[ 2 ].Join( LobbyIdOf( pair.host ) );
            REQUIRE_TRUE( !third.IsNULL() );
            CLobbyRecorder thirdEvents;
            thirdEvents.SubscribeTo( third.GetPointerAlways() );
            CMemberCount three( pair.guest, 3 );
            CMemberCount threeAtThird( third, 3 );
            ASSERT_TRUE( PumpUntil( three ) && PumpUntil( threeAtThird ) );
            CORE::CString hostId = fixture[ 0 ].peer.puid;
            CORE::CString guestId = fixture[ 1 ].peer.puid;
            CORE::CString lobbyId = LobbyIdOf( pair.host );

            // the owner leaves: EOS makes the first of the others the owner
            ASSERT_TRUE( pair.host->Leave() );
            ASSERT_FALSE( Eos( pair.host ).IsMember() );
            ASSERT_TRUE( 0 == pair.hostEvents.closed );
            COwnerIs ownerIsGuest( fixture.world, lobbyId, guestId );
            ASSERT_TRUE( PumpUntil( ownerIsGuest ) );

            CBossIs guestIsBoss( pair.guest, guestId );
            CBossIs thirdSeesBoss( third, guestId );
            ASSERT_TRUE( PumpUntil( guestIsBoss ) && PumpUntil( thirdSeesBoss ) );
            ASSERT_TRUE( Eos( pair.guest ).IsOwner() && pair.guest->IsBoss() && !third->IsBoss() );
            ASSERT_TRUE( pair.guestEvents.HasPath( "boss" ) && thirdEvents.HasPath( "boss" ) );
            ASSERT_TRUE( 1 == pair.guestEvents.left.size() && pair.guestEvents.left[ 0 ] == hostId );
            CMemberCount two( pair.guest, 2 );
            CMemberCount twoAtThird( third, 2 );
            ASSERT_TRUE( PumpUntil( two ) && PumpUntil( twoAtThird ) );
            ASSERT_TRUE( 0 == thirdEvents.closed );

            // the data of the lobby carries the stamp of the new owner and the old owner is nowhere in the state
            SFakeLobby* fake = fixture.world.FindLobby( lobbyId );
            CEosLobbySummary summary;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::DecodeSummary( fake->attributes, guestId, summary, reason ) );

            // the new boss runs the rules: it changes a setting, the other member sees it, and a newcomer can join
            ASSERT_TRUE( pair.guest->SetSetting( "map", "ice" ) );
            CSettingIs seen( third, "map", "ice" );
            ASSERT_TRUE( PumpUntil( seen ) );
            MULTIPLAY::CILobbyPtr newcomer = fixture[ 3 ].Join( lobbyId );
            ASSERT_TRUE( !newcomer.IsNULL() );
            CMemberCount three2( pair.guest, 3 );
            ASSERT_TRUE( PumpUntil( three2 ) );
            ASSERT_TRUE( newcomer->GetState().GetSetting( "map" ) == "ice" && newcomer->GetState().bossId == guestId );

            // the new owner can still start with the rules of the lobby
            ASSERT_TRUE( third->SetReady( true ) );
            ASSERT_TRUE( pair.guest->SetReady( true ) );
            ASSERT_TRUE( newcomer->SetReady( true ) );
            CMemberReady allReady( pair.guest, fixture[ 3 ].peer.puid, true );
            CMemberReady thirdReady( pair.guest, fixture[ 2 ].peer.puid, true );
            ASSERT_TRUE( PumpUntil( allReady ) && PumpUntil( thirdReady ) );
            ASSERT_TRUE( pair.guest->RequestStart() );
            CPhaseIs counting( third, MULTIPLAY::LOBBYPHASE_COUNTDOWN );
            ASSERT_TRUE( PumpUntil( counting ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The boss hands the lobby to a member with TransferBoss: the member becomes the owner in EOS, the old boss is a member" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString hostId = fixture[ 0 ].peer.puid;
            CORE::CString guestId = fixture[ 1 ].peer.puid;
            CORE::CString lobbyId = LobbyIdOf( pair.host );

            // a member cannot take it
            ASSERT_TRUE( pair.guest->TransferBoss( guestId ) );
            CRefused refused( pair.guestEvents, 1 );
            ASSERT_TRUE( PumpUntil( refused ) && pair.guestEvents.HasRefusal( "transferBoss" ) );

            ASSERT_TRUE( pair.host->TransferBoss( guestId ) );
            COwnerIs ownerIsGuest( fixture.world, lobbyId, guestId );
            ASSERT_TRUE( PumpUntil( ownerIsGuest ) );
            CBossIs guestIsBoss( pair.guest, guestId );
            CBossIs hostSeesBoss( pair.host, guestId );
            CIsOwner guestRuns( pair.guest );
            ASSERT_TRUE( PumpUntil( guestIsBoss ) && PumpUntil( hostSeesBoss ) && PumpUntil( guestRuns ) );
            ASSERT_TRUE( Eos( pair.guest ).IsOwner() && !Eos( pair.host ).IsOwner() && !pair.host->IsBoss() && pair.guest->IsBoss() );
            ASSERT_TRUE( pair.hostEvents.HasPath( "boss" ) );
            ASSERT_TRUE( 1 == fixture[ 0 ].peer.promoteCalls );
            ASSERT_TRUE( 2 == pair.host->GetState().members.size() && 2 == pair.guest->GetState().members.size() );

            // the new boss decides, the old one asks
            ASSERT_TRUE( pair.guest->SetSetting( "map", "ice" ) );
            CSettingIs seen( pair.host, "map", "ice" );
            ASSERT_TRUE( PumpUntil( seen ) );
            pair.hostEvents.Clear();
            ASSERT_TRUE( pair.host->SetSetting( "map", "dust" ) );
            CRefused hostRefused( pair.hostEvents, 1 );
            ASSERT_TRUE( PumpUntil( hostRefused ) && pair.hostEvents.HasRefusal( "setting" ) );

            // a failed hand over leaves everything as it was
            fixture[ 1 ].peer.failures.promote = EOSRESULT_NO_PERMISSION;
            ASSERT_TRUE( pair.guest->TransferBoss( hostId ) );
            CRefused promoteRefused( pair.guestEvents, 2 );
            ASSERT_TRUE( PumpUntil( promoteRefused ) );
            ASSERT_TRUE( Eos( pair.guest ).IsOwner() && pair.guest->GetState().bossId == guestId );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A password: the right one gets in, the wrong one is refused with the reason, nothing of it is in an attribute, the lobby ends with its owner" )
        try
        {
            CEosFixture fixture( 3 );
            MULTIPLAY::CLobbyCreateParams params = CEosFixture::MakeParams( "Secret place" );
            params.password = "open sesame";
            MULTIPLAY::CILobbyPtr host = fixture[ 0 ].Create( params );
            REQUIRE_TRUE( !host.IsNULL() );
            CORE::CString lobbyId = LobbyIdOf( host );
            ASSERT_TRUE( fixture[ 0 ].peer.lastCreate.disableHostMigration );

            // without it: not even tried
            MULTIPLAY::CILobbyPtr none = fixture[ 1 ].Join( lobbyId );
            ASSERT_TRUE( none.IsNULL() && fixture[ 1 ].reason.HasSubstr( "needs a password" ) >= 0 );
            ASSERT_TRUE( 1 == fixture.world.FindLobby( lobbyId )->members.size() );

            // the wrong one
            MULTIPLAY::CILobbyPtr wrong = fixture[ 1 ].Join( lobbyId, "open says me" );
            ASSERT_TRUE( wrong.IsNULL() && fixture[ 1 ].reason.HasSubstr( "password is wrong" ) >= 0 );
            CEosMembers aloneAgain( fixture.world, lobbyId, 1 );
            ASSERT_TRUE( PumpUntil( aloneAgain ) );
            ASSERT_TRUE( 1 == host->GetState().members.size() );
            fixture.world.FindLobby( lobbyId )->rejoinAfterKickRequiresInvite = false;

            // the right one
            MULTIPLAY::CILobbyPtr member = fixture[ 1 ].Join( lobbyId, "open sesame" );
            REQUIRE_TRUE( !member.IsNULL() );
            CMemberCount two( host, 2 );
            ASSERT_TRUE( PumpUntil( two ) );

            // the password is in no attribute, of the lobby or of a member, the salt is
            SFakeLobby* fake = fixture.world.FindLobby( lobbyId );
            ASSERT_FALSE( IsInAnyAttribute( *fake, "open sesame" ) );
            ASSERT_FALSE( IsInAnyAttribute( *fake, "sesame" ) );
            CEosLobbySummary summary;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::DecodeSummary( fake->attributes, fake->owner, summary, reason ) && summary.passworded && 32 == summary.passwordSalt.Length() );

            // a member that does not follow the way in, joins the lobby of EOS directly and says nothing: it is put out
            SEosLobbyShim shim = fixture[ 2 ].peer.LobbyShim();
            REQUIRE_TRUE( shim.joinLobbyById( shim.context, 9001, lobbyId.C_String(), fixture[ 2 ].peer.puid.C_String(), false ) );
            CEosMembers three( fixture.world, lobbyId, 3 );
            ASSERT_TRUE( PumpUntil( three, 1000 ) );
            CEosMembers backToTwo( fixture.world, lobbyId, 2 );
            ASSERT_TRUE( PumpUntil( backToTwo, 3000 ) );
            ASSERT_TRUE( 2 == host->GetState().members.size() );

            // a lobby with a password ends with its owner: EOS does not pass it on, the successor would not know the password
            CLobbyRecorder memberEvents;
            memberEvents.SubscribeTo( member.GetPointerAlways() );
            ASSERT_TRUE( host->Leave() );
            CClosed closed( memberEvents );
            ASSERT_TRUE( PumpUntil( closed ) );
            ASSERT_TRUE( 1 == memberEvents.closed && memberEvents.closedReason == "The host left" );
            CNoLobbies gone( fixture.world );
            ASSERT_TRUE( PumpUntil( gone ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: What another member writes is a request and no more: attributes with the keys of the owner are ignored" )
        try
        {
            CEosFixture fixture( 3 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString lobbyId = LobbyIdOf( pair.host );
            SFakeLobby* fake = fixture.world.FindLobby( lobbyId );
            TEosAttributeVector before = fake->attributes;

            // the member tries to change the lobby: EOS refuses the whole update
            SEosLobbyShim shim = fixture[ 1 ].peer.LobbyShim();
            SEosLobbyUpdateOptions forged;
            forged.lobbyId = lobbyId;
            forged.localUser = fixture[ 1 ].peer.puid;
            forged.setAttributes.push_back( CEosLobbyCodec::MakeInt( CEosLobbyCodec::KeyPhase, MULTIPLAY::LOBBYPHASE_IN_GAME ) );
            forged.setAttributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::KeyName, "Taken over" ) );
            forged.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "n", "Taken over" ) );
            REQUIRE_TRUE( shim.updateLobby( shim.context, 9101, &forged ) );
            PumpFor( 30 );
            ASSERT_TRUE( before.size() == fake->attributes.size() );
            CEosLobbySummary summary;
            CORE::CString reason;
            ASSERT_TRUE( CEosLobbyCodec::DecodeSummary( fake->attributes, fake->owner, summary, reason ) && summary.name == "Fake lobby" && MULTIPLAY::LOBBYPHASE_OPEN == summary.phase );

            // what a member can write is its own attributes: with the keys of the owner they are not read as anything
            SEosLobbyUpdateOptions own;
            own.lobbyId = lobbyId;
            own.localUser = fixture[ 1 ].peer.puid;
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "st_0", "AAAA" ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeInt( "st_n", 1 ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "st_h", "0000000000000000" ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeInt( "la_n", 1 ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "la_0", "AAAA" ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeInt( "ph", MULTIPLAY::LOBBYPHASE_IN_GAME ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "by", fixture[ 1 ].peer.puid ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "rq", "garbage that is no request" ) );
            own.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( "rdy", "no:way" ) );
            REQUIRE_TRUE( shim.updateLobby( shim.context, 9102, &own ) );
            PumpFor( 60 );
            ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_OPEN == pair.host->GetState().phase && MULTIPLAY::LOBBYPHASE_OPEN == pair.guest->GetState().phase );
            ASSERT_TRUE( 2 == pair.host->GetState().members.size() && !pair.host->GetState().FindMember( fixture[ 1 ].peer.puid )->ready );
            ASSERT_TRUE( 0 == pair.hostEvents.launches && 0 == pair.guestEvents.launches && 0 == pair.hostEvents.refusals );
            ASSERT_TRUE( pair.host->GetState().name == "Fake lobby" );

            // a backend that let somebody else write the lobby: the data does not carry the stamp of the owner, nobody takes it
            SEosAttribute stamp = CEosLobbyCodec::MakeString( CEosLobbyCodec::KeyStamp, fixture[ 1 ].peer.puid );
            SEosAttribute name = CEosLobbyCodec::MakeString( CEosLobbyCodec::KeyName, "Forged lobby" );
            REQUIRE_TRUE( fixture.world.ForgeLobbyAttribute( lobbyId, name ) && fixture.world.ForgeLobbyAttribute( lobbyId, stamp ) );
            PumpFor( 60 );
            ASSERT_TRUE( pair.guest->GetState().name == "Fake lobby" );
            MULTIPLAY::CGameListingFilter filter;
            filter.gameId = "testgame";
            MULTIPLAY::CGameListingList list;
            ASSERT_TRUE( Browse( fixture[ 2 ], filter, list ) );
            ASSERT_TRUE( 0 == list.listings.size() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A blob of a lobby that does not hold up is ignored: a hash that does not fit, a piece that is missing, data nested too deep" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString lobbyId = LobbyIdOf( pair.host );
            SFakeLobby* fake = fixture.world.FindLobby( lobbyId );
            REQUIRE_TRUE( GUCEF_NULL != fake );
            CORE::CString host = fixture[ 0 ].peer.puid;

            // the owner's own stamp on data that was damaged on the way
            ASSERT_TRUE( fixture.world.ForgeLobbyAttribute( lobbyId, CEosLobbyCodec::MakeString( "st_h", "0123456789abcdef" ) ) );
            ASSERT_TRUE( fixture.world.ForgeLobbyAttribute( lobbyId, CEosLobbyCodec::MakeInt( CEosLobbyCodec::KeyRevision, 777 ) ) );
            PumpFor( 60 );
            ASSERT_TRUE( 2 == pair.guest->GetState().members.size() && "Fake lobby" == pair.guest->GetState().name );

            // and data nested far deeper than any lobby is
            CORE::CDataNode deep( "st" );
            CORE::CDataNode* node = &deep;
            for ( UInt32 i=0; i<200; ++i )
                node = node->AddChild( "state", GUCEF_DATATYPE_OBJECT );
            TEosAttributeVector blob;
            CORE::CString reason;
            CEosLobbyLimits limits;
            ASSERT_TRUE( CEosLobbyCodec::EncodeBlob( CEosLobbyCodec::BlobState, deep, limits, blob, reason ) );
            for ( size_t i=0; i<blob.size(); ++i )
                fixture.world.ForgeLobbyAttribute( lobbyId, blob[ i ] );
            fixture.world.ForgeLobbyAttribute( lobbyId, CEosLobbyCodec::MakeInt( CEosLobbyCodec::KeyRevision, 778 ) );
            PumpFor( 60 );
            ASSERT_TRUE( 2 == pair.guest->GetState().members.size() && "Fake lobby" == pair.guest->GetState().name );
            ASSERT_TRUE( 0 == pair.guestEvents.closed );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A member that floods the owner with requests is held to the rate limit, the rest is refused with the reason" )
        try
        {
            CEosFixture fixture( 2 );
            MULTIPLAYPLUGIN::EOS::CEosLobbyServiceSettings settings = CEosFixture::MakeFastSettings();
            settings.memberRequestsPerSecond = 2;
            settings.memberRequestBurst = 3;
            fixture.ApplySettings( settings );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            CORE::CString lobbyId = LobbyIdOf( pair.host );

            // requests written straight into the attributes, faster than the member itself would
            SEosLobbyShim shim = fixture[ 1 ].peer.LobbyShim();
            for ( UInt32 i=1; i<=20; ++i )
            {
                SEosLobbyUpdateOptions options;
                options.lobbyId = lobbyId;
                options.localUser = fixture[ 1 ].peer.puid;
                options.setMemberAttributes.push_back( CEosLobbyCodec::MakeString( CEosLobbyCodec::MemberKeyRequest, CEosLobbyCodec::MakeRequest( 100 + i, "col", "red", "" ) ) );
                REQUIRE_TRUE( shim.updateLobby( shim.context, 9200 + i, &options ) );
                PumpFor( 3 );
            }
            PumpFor( 100 );
            // some were taken, the others refused with the reason, and the state stays consistent
            ASSERT_TRUE( 2 == pair.host->GetState().members.size() );
            ASSERT_TRUE( pair.guest->GetState().FindSlotOfMember( fixture[ 1 ].peer.puid )->colour == "red" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Reporting is not supported over EOS and says so; a lobby over EOS has no chat" )
        try
        {
            CEosFixture fixture( 2 );
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            ASSERT_FALSE( pair.guest->ReportPlayer( fixture[ 0 ].peer.puid, "cheating" ) );
            ASSERT_TRUE( pair.guestEvents.HasRefusal( "report" ) );
            ASSERT_FALSE( fixture[ 0 ].service.GetCapabilities().chat );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
