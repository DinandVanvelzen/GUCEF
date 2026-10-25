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


#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

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

class CLaunches : public CCondition
{
    public:
    GUCEF::vector< CLobbyRecorder* > recorders;
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( size_t i=0; i<recorders.size(); ++i )
            if ( recorders[ i ]->launches < 1 )
                return false;
        return true;
    }
};

class CUInt32AtLeast : public CCondition
{
    public:
    const UInt32& counter;
    UInt32 value;
    CUInt32AtLeast( const UInt32& c, UInt32 v ) : counter( c ), value( v ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return counter >= value; }
};

class CSessionsActiveWith : public CCondition
{
    public:
    GUCEF::vector< DRN::CDRNSession* > sessions;
    UInt32 members;
    explicit CSessionsActiveWith( UInt32 m ) : sessions(), members( m ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( size_t i=0; i<sessions.size(); ++i )
            if ( !sessions[ i ]->IsActive() || sessions[ i ]->GetMemberCount() != members )
                return false;
        return true;
    }
};

class CGridHas : public CCondition
{
    public:
    DRN::CDRNSession& session;
    CORE::CString gridName;
    UInt32 owner;
    CORE::CVariant key;
    CORE::CVariant expected;
    CGridHas( DRN::CDRNSession& s, const CORE::CString& g, UInt32 o, const CORE::CVariant& k, const CORE::CVariant& e ) : session( s ), gridName( g ), owner( o ), key( k ), expected( e ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNDataGrid* grid = session.FindGrid( gridName, owner );
        if ( GUCEF_NULL == grid || !grid->IsSnapshotComplete() )
            return false;
        CORE::CVariant value;
        return grid->GetValue( key, value ) && DRN::CDRNVariantCodec::IsSame( value, expected );
    }
};

/** The endpoint of a launch with the user of this process named, as a process with several users needs */
static DRN::CDRNEndpoint
LocalEndpoint( const CORE::CString& endpoint, const CORE::CString& localUser )
{
    DRN::CDRNEndpoint result;
    result.FromString( endpoint + "?local=" + localUser );
    return result;
}

/** The game: one DRN node and session per member, made from the launch descriptions the lobby gave */
class CEosGame
{
    public:

    GUCEF::vector< DRN::CDRNNode* > nodes;
    GUCEF::vector< DRN::CDRNSession* > sessions;
    CORE::CString failedStep;

    ~CEosGame()
    {
        for ( size_t i=0; i<nodes.size(); ++i )
        {
            nodes[ i ]->Stop();
            GUCEF_DELETE nodes[ i ];
        }
    }

    /** The host (or the sequencer) first, then the others */
    bool Start( const GUCEF::vector< const MULTIPLAY::CLaunchDescription* >& launches, bool waitForAll = true )
    {
        GUCEF::vector< const MULTIPLAY::CLaunchDescription* > order;
        for ( size_t i=0; i<launches.size(); ++i )
            if ( launches[ i ]->IsLocalMemberHost() )
                order.push_back( launches[ i ] );
        for ( size_t i=0; i<launches.size(); ++i )
            if ( !launches[ i ]->IsLocalMemberHost() )
                order.push_back( launches[ i ] );
        if ( order.empty() || !order[ 0 ]->IsLocalMemberHost() )
        {
            failedStep = "no host";
            return false;
        }

        for ( size_t i=0; i<order.size(); ++i )
        {
            const MULTIPLAY::CLaunchDescription& launch = *order[ i ];
            bool host = 0 == i;
            bool peerToPeer = MULTIPLAY::GAMETOPOLOGY_PEER_TO_PEER == launch.topology;

            DRN::CDRNNodeSettings nodeSettings;
            nodeSettings.echoIntervalMs = 50;
            nodeSettings.pulseIntervalMs = 1;
            launch.ApplyToNodeSettings( nodeSettings );
            DRN::CDRNAuthSettings auth;
            launch.MakeAuthSettings( auth );
            if ( host || peerToPeer )
            {
                const MULTIPLAY::CLaunchMember* self = launch.FindMember( launch.localMemberId );
                if ( GUCEF_NULL == self || self->endpoints.empty() )
                {
                    failedStep = "no endpoint of the member";
                    return false;
                }
                nodeSettings.listenEndpoints.Add( LocalEndpoint( self->endpoints[ 0 ], launch.localMemberId ) );
            }
            DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
            nodes.push_back( node );
            if ( !node->SetSettings( nodeSettings ) || !node->Start() )
            {
                failedStep = "node";
                return false;
            }
            DRN::CDRNSession* session = node->CreateSession();
            sessions.push_back( session );
            DRN::CDRNSessionSettings sessionSettings;
            launch.MakeSessionSettings( sessionSettings );
            if ( host )
            {
                if ( !session->Host( sessionSettings ) )
                {
                    failedStep = "host";
                    return false;
                }
            }
            else
            {
                CORE::CString::StringVector hostEndpoints;
                launch.GetHostEndpoints( hostEndpoints );
                DRN::CDRNEndpointList list;
                for ( size_t e=0; e<hostEndpoints.size(); ++e )
                    list.Add( LocalEndpoint( hostEndpoints[ e ], launch.localMemberId ) );
                if ( !session->Join( list, sessionSettings, &auth ) )
                {
                    failedStep = "join";
                    return false;
                }
            }
        }
        if ( !waitForAll )
            return true;
        CSessionsActiveWith active( (UInt32) order.size() );
        active.sessions = sessions;
        if ( !PumpUntil( active, 20000 ) )
        {
            failedStep = "membership";
            return false;
        }
        return true;
    }
};

static bool
DecodeKey( const CORE::CString& hex, UInt8* key )
{
    return DRN::CDRNCrypto::FromHex( hex, key, DRN::CDRNCrypto::KeySize );
}

/** Reads a token of a launch: the session, the user and the role it is for */
static bool
ReadToken( const MULTIPLAY::CLaunchDescription& launch, CORE::CString& session, CORE::CString& user, CORE::CString& role, UInt64& expiry, CORE::CDynamicBuffer& policyHash )
{
    UInt8 publicKey[ DRN::CDRNCrypto::KeySize ];
    if ( !DecodeKey( launch.serverPublicKey, publicKey ) )
        return false;
    GUCEF::vector< UInt8 > raw( launch.token.ByteSize() + 8 );
    UInt32 used = 0;
    if ( !CORE::Base64Decode( launch.token, &raw[ 0 ], (UInt32) raw.size(), used ) )
        return false;
    CORE::CDynamicBuffer userKey;
    return DRN::CDRNTokenAuthenticator::ParseSignedToken( publicKey, &raw[ 0 ], used, session, user, role, expiry, userKey, policyHash );
}

/** Three players in a lobby, ready to start */
struct SThree
{
    SPair pair;
    MULTIPLAY::CILobbyPtr third;
    CLobbyRecorder thirdEvents;
};

static bool
MakeThree( CEosFixture& fixture, SThree& three, const MULTIPLAY::CLobbyCreateParams& params )
{
    if ( !MakePair( fixture, three.pair, params ) )
        return false;
    three.third = fixture[ 2 ].Join( LobbyIdOf( three.pair.host ) );
    if ( three.third.IsNULL() )
        return false;
    three.thirdEvents.SubscribeTo( three.third.GetPointerAlways() );
    CMemberCount all( three.pair.host, 3 );
    CMemberCount allAtThird( three.third, 3 );
    return PumpUntil( all ) && PumpUntil( allAtThird );
}

static bool
StartIt( CEosFixture& fixture, SThree& three )
{
    if ( !three.pair.guest->SetReady( true ) || !three.third->SetReady( true ) )
        return false;
    CMemberReady guestReady( three.pair.host, fixture[ 1 ].peer.puid, true );
    CMemberReady thirdReady( three.pair.host, fixture[ 2 ].peer.puid, true );
    if ( !PumpUntil( guestReady ) || !PumpUntil( thirdReady ) )
        return false;
    if ( !three.pair.host->SetReady( true ) )
        return false;
    CMemberReady hostReady( three.pair.host, fixture[ 0 ].peer.puid, true );
    if ( !PumpUntil( hostReady ) )
        return false;
    if ( !three.pair.host->RequestStart() )
        return false;
    CLaunches launches;
    launches.recorders.push_back( &three.pair.hostEvents );
    launches.recorders.push_back( &three.pair.guestEvents );
    launches.recorders.push_back( &three.thirdEvents );
    return PumpUntil( launches, 10000 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosLobbyLaunchTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS lobby launch TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosLobbyLaunch" );

    GUCEF_TESTFW_TESTCASE( "Test: The start of a game gives every member its launch description with the endpoint of the host and a token that is for that member and signed by the lobby" )
        try
        {
            CEosFixture fixture( 3 );
            SThree three;
            REQUIRE_TRUE( MakeThree( fixture, three, CEosFixture::MakeParams() ) );
            CORE::CString lobbyId = LobbyIdOf( three.pair.host );
            REQUIRE_TRUE( StartIt( fixture, three ) );

            ASSERT_TRUE( 1 == three.pair.hostEvents.launches && 1 == three.pair.guestEvents.launches && 1 == three.thirdEvents.launches );
            ASSERT_TRUE( MULTIPLAY::LOBBYPHASE_IN_GAME == three.pair.host->GetState().phase );
            CPhaseIs inGame( three.pair.guest, MULTIPLAY::LOBBYPHASE_IN_GAME );
            ASSERT_TRUE( PumpUntil( inGame ) );

            const MULTIPLAY::CLaunchDescription& hostLaunch = three.pair.hostEvents.lastLaunch;
            const MULTIPLAY::CLaunchDescription& guestLaunch = three.pair.guestEvents.lastLaunch;
            const MULTIPLAY::CLaunchDescription& thirdLaunch = three.thirdEvents.lastLaunch;
            CORE::CString hostId = fixture[ 0 ].peer.puid;
            CORE::CString guestId = fixture[ 1 ].peer.puid;

            // what they all share
            ASSERT_TRUE( guestLaunch.gameId == "testgame" && guestLaunch.lobbyId == lobbyId && guestLaunch.sessionName == "eos-" + lobbyId );
            ASSERT_TRUE( MULTIPLAY::GAMETOPOLOGY_CLIENT_SERVER == guestLaunch.topology && guestLaunch.serverMemberId == hostId );
            ASSERT_TRUE( 3 == guestLaunch.members.size() && 4 == guestLaunch.slots.size() && guestLaunch.settings.GetValueAlways( "map" ).AsString() == "dust" );
            CORE::CString socket = CEosLobbyCodec::MakeSocketName( "gg", lobbyId );
            const MULTIPLAY::CLaunchMember* hostMember = guestLaunch.FindMember( hostId );
            ASSERT_TRUE( GUCEF_NULL != hostMember && 1 == hostMember->endpoints.size() && hostMember->endpoints[ 0 ] == "eos-p2p://" + hostId + "/" + socket );
            CORE::CString::StringVector hostEndpoints;
            ASSERT_TRUE( guestLaunch.GetHostEndpoints( hostEndpoints ) && 1 == hostEndpoints.size() && hostEndpoints[ 0 ] == hostMember->endpoints[ 0 ] );
            ASSERT_TRUE( hostLaunch.IsLocalMemberHost() && !guestLaunch.IsLocalMemberHost() && !thirdLaunch.IsLocalMemberHost() );
            ASSERT_TRUE( guestLaunch.localMemberId == guestId && thirdLaunch.localMemberId == fixture[ 2 ].peer.puid && !guestLaunch.spectator );

            // the key of the lobby, and a policy that wants signed tokens
            ASSERT_TRUE( 64 == guestLaunch.serverPublicKey.Length() && guestLaunch.serverPublicKey == thirdLaunch.serverPublicKey && guestLaunch.serverPublicKey == hostLaunch.serverPublicKey );
            ASSERT_TRUE( guestLaunch.securityPolicy.requireSignedTokens && guestLaunch.securityPolicy.requireTokenPolicyHash );

            // each token is for its member, this session, a role, and the policy of the game
            UInt8 policyHash[ DRN::CDRNSecurityPolicy::PolicyHashSize ];
            guestLaunch.securityPolicy.GetPolicyHash( policyHash );
            CORE::CString session, user, role;
            UInt64 expiry = 0;
            CORE::CDynamicBuffer tokenPolicy;
            ASSERT_TRUE( ReadToken( guestLaunch, session, user, role, expiry, tokenPolicy ) );
            ASSERT_TRUE( session == guestLaunch.sessionName && user == guestId && role == "player" && expiry > DRN::CDRNTokenAuthenticator::GetUnixTimeNow() );
            ASSERT_TRUE( DRN::CDRNSecurityPolicy::PolicyHashSize == tokenPolicy.GetDataSize() && 0 == memcmp( tokenPolicy.GetConstBufferPtr(), policyHash, sizeof( policyHash ) ) );
            CORE::CDynamicBuffer thirdPolicy;
            ASSERT_TRUE( ReadToken( thirdLaunch, session, user, role, expiry, thirdPolicy ) && user == fixture[ 2 ].peer.puid && role == "player" );
            ASSERT_TRUE( guestLaunch.token != thirdLaunch.token );
            CORE::CDynamicBuffer hostPolicy;
            ASSERT_TRUE( ReadToken( hostLaunch, session, user, role, expiry, hostPolicy ) && user == hostId && role == "host" );

            // a token of another lobby is not a token of this one
            UInt8 otherKey[ DRN::CDRNCrypto::KeySize ];
            UInt8 otherPublic[ DRN::CDRNCrypto::KeySize ];
            ASSERT_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( otherKey, otherPublic ) );
            GUCEF::vector< UInt8 > raw( guestLaunch.token.ByteSize() + 8 );
            UInt32 used = 0;
            ASSERT_TRUE( CORE::Base64Decode( guestLaunch.token, &raw[ 0 ], (UInt32) raw.size(), used ) );
            CORE::CDynamicBuffer ignoredKey, ignoredPolicy;
            ASSERT_FALSE( DRN::CDRNTokenAuthenticator::ParseSignedToken( otherPublic, &raw[ 0 ], used, session, user, role, expiry, ignoredKey, ignoredPolicy ) );

            // nothing of it is in the lobby that everybody can read, and nothing of the key
            SFakeLobby* fake = fixture.world.FindLobby( lobbyId );
            ASSERT_FALSE( IsInAnyAttribute( *fake, guestLaunch.token ) || IsInAnyAttribute( *fake, thirdLaunch.token ) || IsInAnyAttribute( *fake, hostLaunch.token ) );
            ASSERT_TRUE( 0 == fake->attributes.size() || !IsInAnyAttribute( *fake, "token" ) );

            // the sockets of the game are reachable by the members only
            CEosP2PRouter* router = fixture[ 0 ].platform->GetP2PRouter();
            CEosP2PSocket* gameSocket = router->FindSocket( socket );
            REQUIRE_TRUE( GUCEF_NULL != gameSocket );
            ASSERT_TRUE( CEosP2PSocket::ACCEPT_LIST == gameSocket->GetAcceptPolicy() );
            ASSERT_TRUE( gameSocket->IsRemoteAllowed( guestId ) && gameSocket->IsRemoteAllowed( fixture[ 2 ].peer.puid ) && !gameSocket->IsRemoteAllowed( "00112233445566778899aabbccdd77" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The game runs over the launch: three members make a DRN session over the P2P channel, and a grid replicates, on a network that loses packets" )
        try
        {
            REQUIRE_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CEosFixture fixture( 3 );
            SThree three;
            REQUIRE_TRUE( MakeThree( fixture, three, CEosFixture::MakeParams() ) );
            REQUIRE_TRUE( StartIt( fixture, three ) );
            fixture.world.network.lossRate = 0.1;
            fixture.world.network.duplicateRate = 0.05;
            fixture.world.network.reorderRate = 0.1;
            fixture.world.network.reorderMs = 10;

            GUCEF::vector< const MULTIPLAY::CLaunchDescription* > launches;
            launches.push_back( &three.pair.hostEvents.lastLaunch );
            launches.push_back( &three.pair.guestEvents.lastLaunch );
            launches.push_back( &three.thirdEvents.lastLaunch );
            CEosGame game;
            ASSERT_TRUE( game.Start( launches ) );
            ASSERT_TRUE( game.failedStep.IsNULLOrEmpty() && 3 == game.sessions.size() );

            DRN::CDRNDataGridSettings settings;
            settings.name = "world";
            settings.authority = DRN::DRN_AUTHORITY_OWNER;
            settings.sendRateHz = 30;
            DRN::CDRNDataGrid* grid = game.sessions[ 0 ]->CreateGrid( settings );
            REQUIRE_TRUE( GUCEF_NULL != grid );
            grid->SetValue( "tick", CORE::CVariant( (UInt32) 0 ) );
            game.sessions[ 1 ]->Subscribe( "world" );
            game.sessions[ 2 ]->Subscribe( "world" );
            UInt32 index = grid->GetKeyIndex( "tick" );
            for ( UInt32 i=1; i<=200; ++i )
            {
                grid->SetValueByIndex( index, CORE::CVariant( i ) );
                PumpFor( 2 );
            }
            CGridHas firstSees( *game.sessions[ 1 ], "world", 1, "tick", CORE::CVariant( (UInt32) 200 ) );
            CGridHas secondSees( *game.sessions[ 2 ], "world", 1, "tick", CORE::CVariant( (UInt32) 200 ) );
            ASSERT_TRUE( PumpUntil( firstSees, 20000 ) && PumpUntil( secondSees, 20000 ) );
            ASSERT_TRUE( fixture.world.packetsDropped > 0 && fixture.world.largestPacket <= 1170 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Somebody that is not in the lobby cannot reach the game, or the control socket that hands out tokens, and a member with a token of its own making is refused by the game" )
        try
        {
            REQUIRE_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CEosFixture fixture( 4 );
            SThree three;
            REQUIRE_TRUE( MakeThree( fixture, three, CEosFixture::MakeParams() ) );
            REQUIRE_TRUE( StartIt( fixture, three ) );
            CORE::CString lobbyId = LobbyIdOf( three.pair.host );
            CORE::CString gameSocket = CEosLobbyCodec::MakeSocketName( "gg", lobbyId );
            CORE::CString controlSocket = CEosLobbyCodec::MakeSocketName( "gc", lobbyId );
            CEosP2PRouter* hostRouter = fixture[ 0 ].platform->GetP2PRouter();
            REQUIRE_TRUE( GUCEF_NULL != hostRouter && GUCEF_NULL != hostRouter->FindSocket( gameSocket ) && GUCEF_NULL != hostRouter->FindSocket( controlSocket ) );

            // the fourth player is logged in and is no member
            CEosPeerContext& outsider = fixture[ 3 ];
            ASSERT_TRUE( outsider.platform->GetLoginState().IsLoggedIn() );
            CEosP2PRouter* outsiderRouter = outsider.platform->GetP2PRouter();
            REQUIRE_TRUE( GUCEF_NULL != outsiderRouter );
            UInt32 tick = outsider.platform->AddTickDemand( 2 );
            CEosP2PSocket* toGame = outsiderRouter->AcquireSocket( gameSocket );
            CEosP2PSocket* toControl = outsiderRouter->AcquireSocket( controlSocket );
            UInt32 refusedBefore = hostRouter->FindSocket( gameSocket )->GetRefusedConnectionCount();
            UInt32 controlRefusedBefore = hostRouter->FindSocket( controlSocket )->GetRefusedConnectionCount();
            ASSERT_TRUE( toGame->SendPacket( fixture[ 0 ].peer.puid, "hello", 5, EOSRELIABILITY_RELIABLE_ORDERED ) );
            ASSERT_TRUE( toControl->SendPacket( fixture[ 0 ].peer.puid, "GTQ1", 4, EOSRELIABILITY_RELIABLE_ORDERED ) );
            PumpFor( 150 );
            ASSERT_TRUE( refusedBefore + 1 == hostRouter->FindSocket( gameSocket )->GetRefusedConnectionCount() );
            ASSERT_TRUE( controlRefusedBefore + 1 == hostRouter->FindSocket( controlSocket )->GetRefusedConnectionCount() );
            ASSERT_FALSE( hostRouter->FindSocket( gameSocket )->IsConnectedTo( outsider.peer.puid ) || hostRouter->FindSocket( controlSocket )->IsConnectedTo( outsider.peer.puid ) );

            // a member of the lobby: it may ask for its token, but a game that is joined with a token that is not signed by the lobby is refused
            MULTIPLAY::CLaunchDescription forged = three.pair.guestEvents.lastLaunch;
            UInt8 forgedKey[ DRN::CDRNCrypto::KeySize ];
            UInt8 forgedPublic[ DRN::CDRNCrypto::KeySize ];
            ASSERT_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( forgedKey, forgedPublic ) );
            UInt8 policyHash[ DRN::CDRNSecurityPolicy::PolicyHashSize ];
            forged.securityPolicy.GetPolicyHash( policyHash );
            forged.token = DRN::CDRNTokenAuthenticator::IssueSignedToken( forgedKey, forged.sessionName, forged.localMemberId, "player",
                                                                          DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600, GUCEF_NULL, policyHash );
            ASSERT_FALSE( forged.token.IsNULLOrEmpty() );

            GUCEF::vector< const MULTIPLAY::CLaunchDescription* > launches;
            launches.push_back( &three.pair.hostEvents.lastLaunch );
            launches.push_back( &forged );
            CEosGame game;
            ASSERT_TRUE( game.Start( launches, false ) );
            PumpFor( 1500 );
            ASSERT_TRUE( game.sessions[ 0 ]->GetMemberCount() <= 1 );
            ASSERT_FALSE( game.sessions[ 1 ]->IsActive() );

            outsiderRouter->ReleaseSocket( toGame );
            outsiderRouter->ReleaseSocket( toControl );
            outsider.platform->RemoveTickDemand( tick );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A spectator joins the game in progress and gets the game, a player cannot join it" )
        try
        {
            CEosFixture fixture( 4 );
            SThree three;
            REQUIRE_TRUE( MakeThree( fixture, three, CEosFixture::MakeParams() ) );
            REQUIRE_TRUE( StartIt( fixture, three ) );
            CORE::CString lobbyId = LobbyIdOf( three.pair.host );

            // a player is refused
            MULTIPLAY::CILobbyPtr player = fixture[ 3 ].Join( lobbyId );
            ASSERT_TRUE( player.IsNULL() );
            ASSERT_TRUE( fixture[ 3 ].reason.HasSubstr( "progress" ) >= 0 || fixture[ 3 ].reason.HasSubstr( "started" ) >= 0 || fixture[ 3 ].reason.HasSubstr( "spectat" ) >= 0 );
            CEosMembers stillThree( fixture.world, lobbyId, 3 );
            ASSERT_TRUE( PumpUntil( stillThree, 4000 ) );

            // a spectator is let in, and gets the launch with a token of its own
            CLobbyRecorder spectatorEvents;
            fixture[ 3 ].serviceRecorder.watcher = &spectatorEvents;
            MULTIPLAY::CILobbyPtr spectator = fixture[ 3 ].Join( lobbyId, CORE::CString::Empty, true );
            fixture[ 3 ].serviceRecorder.watcher = GUCEF_NULL;
            REQUIRE_TRUE( !spectator.IsNULL() );
            CUInt32AtLeast launched( spectatorEvents.launches, 1 );
            ASSERT_TRUE( PumpUntil( launched, 10000 ) );
            const MULTIPLAY::CLaunchDescription& launch = spectatorEvents.lastLaunch;
            ASSERT_TRUE( launch.spectator && launch.localMemberId == fixture[ 3 ].peer.puid && launch.sessionName == "eos-" + lobbyId );
            CORE::CString session, user, role;
            UInt64 expiry = 0;
            CORE::CDynamicBuffer policy;
            ASSERT_TRUE( ReadToken( launch, session, user, role, expiry, policy ) && user == fixture[ 3 ].peer.puid && role == "spectator" );

            // and the game socket of the host lets it in now
            CEosP2PSocket* gameSocket = fixture[ 0 ].platform->GetP2PRouter()->FindSocket( CEosLobbyCodec::MakeSocketName( "gg", lobbyId ) );
            REQUIRE_TRUE( GUCEF_NULL != gameSocket );
            ASSERT_TRUE( gameSocket->IsRemoteAllowed( fixture[ 3 ].peer.puid ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A game cannot be started without EOS P2P: the start is refused with the reason and the lobby is open again" )
        try
        {
            CEosFixture fixture( 2, false );
            for ( UInt32 i=0; i<2; ++i )
            {
                REQUIRE_TRUE( fixture[ i ].Login() );
                // no P2P table: the lobby interface alone
                fixture[ i ].service.SetShim( fixture[ i ].peer.LobbyShim(), GUCEF_NULL );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                REQUIRE_TRUE( fixture[ i ].service.Connect( fixture[ i ].identity, future ) && PumpUntilReady( future, 3000 ) );
            }
            SPair pair;
            REQUIRE_TRUE( MakePair( fixture, pair, CEosFixture::MakeParams() ) );
            pair.guest->SetReady( true );
            CMemberReady guestReady( pair.host, fixture[ 1 ].peer.puid, true );
            ASSERT_TRUE( PumpUntil( guestReady ) );
            pair.host->SetReady( true );
            CMemberReady hostReady( pair.host, fixture[ 0 ].peer.puid, true );
            ASSERT_TRUE( PumpUntil( hostReady ) );
            ASSERT_TRUE( pair.host->ForceStart() );
            CUInt32AtLeast refusedStart( pair.hostEvents.refusals, 1 );
            ASSERT_TRUE( PumpUntil( refusedStart, 8000 ) );
            ASSERT_TRUE( pair.hostEvents.HasRefusal( "start" ) );
            ASSERT_TRUE( pair.hostEvents.refusalReasons[ 0 ].HasSubstr( "P2P" ) >= 0 );
            CPhaseIs open( pair.host, MULTIPLAY::LOBBYPHASE_OPEN );
            CPhaseIs openAtGuest( pair.guest, MULTIPLAY::LOBBYPHASE_OPEN );
            ASSERT_TRUE( PumpUntil( open ) && PumpUntil( openAtGuest ) );
            ASSERT_TRUE( 0 == pair.hostEvents.launches && 0 == pair.guestEvents.launches );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
