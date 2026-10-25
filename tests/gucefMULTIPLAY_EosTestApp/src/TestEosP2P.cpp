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

#include "drnchannelpluginEOSP2P_CEosP2PEndpointInfo.h"
#include "drnchannelpluginEOSP2P_CEosP2PChannel.h"

#include "TestEosCommon.h"
#include "TestEosP2P.h"

using namespace GUCEF;
using namespace GUCEF::PLUGINGLUE::EOS;
using namespace GUCEF::DRNPLUGIN::EOSP2P;
using namespace EosFake;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef DRN::CDRNChannelFactory::TProductPtr TChannelPtr;

/** Records what a socket tells */
class CSocketRecorder : public CORE::CObserver
{
    public:

    struct SPacket
    {
        CORE::CString from;
        UInt8 channel;
        GUCEF::vector< UInt8 > data;
    };

    GUCEF::vector< SPacket > packets;
    UInt32 requests;
    UInt32 requestsAccepted;
    UInt32 established;
    UInt32 interrupted;
    UInt32 closed;
    EEosNetworkType lastNetworkType;
    EEosConnectionCloseReason lastCloseReason;
    UInt32 threadId;

    CSocketRecorder( void ) : CORE::CObserver(), packets(), requests( 0 ), requestsAccepted( 0 ), established( 0 ), interrupted( 0 ), closed( 0 ), lastNetworkType( EOSNETWORK_NONE ), lastCloseReason( EOSCLOSE_UNKNOWN ), threadId( 0 ) {}

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        threadId = MT::GetCurrentTaskID();
        if ( CEosP2PSocket::PacketReceivedEvent == eventId )
        {
            const CEosP2PPacketEventData* data = static_cast< const CEosP2PPacketEventData* >( eventData );
            SPacket packet;
            packet.from = data->remoteUser;
            packet.channel = data->channel;
            packet.data.assign( data->data, data->data + data->size );
            packets.push_back( packet );
        }
        else
        {
            const CEosP2PConnectionEventData* data = static_cast< const CEosP2PConnectionEventData* >( eventData );
            if ( CEosP2PSocket::ConnectionRequestEvent == eventId )
            {
                ++requests;
                if ( data->accepted )
                    ++requestsAccepted;
            }
            else
            if ( CEosP2PSocket::ConnectionEstablishedEvent == eventId )
            {
                ++established;
                lastNetworkType = data->networkType;
            }
            else
            if ( CEosP2PSocket::ConnectionInterruptedEvent == eventId )
            {
                ++interrupted;
            }
            else
            if ( CEosP2PSocket::ConnectionClosedEvent == eventId )
            {
                ++closed;
                lastCloseReason = data->closeReason;
            }
        }
    }
};

/*-------------------------------------------------------------------------*/

/** A user that is logged in with its P2P interface, the router of it asked for by name */
class CP2PUser
{
    public:

    CEosPeerContext& context;
    CEosP2PRouter* router;
    UInt32 tickDemand;

    explicit CP2PUser( CEosPeerContext& c ) : context( c ), router( GUCEF_NULL ), tickDemand( 0 )
    {
        if ( context.Login() )
        {
            context.platform->AttachP2P( context.peer.P2PShim() );
            router = context.platform->GetP2PRouter();
            if ( GUCEF_NULL != router )
                router->SetLocalUser( context.peer.puid );
            tickDemand = context.platform->AddTickDemand( 2 );
        }
    }

    ~CP2PUser()
    {
        if ( GUCEF_NULL != context.platform && 0 != tickDemand )
            context.platform->RemoveTickDemand( tickDemand );
    }

    const CORE::CString& Id( void ) const { return context.peer.puid; }
};

/*-------------------------------------------------------------------------*/

class CSocketCondition : public CCondition
{
    public:
    CSocketRecorder& recorder;
    UInt32 packets;
    UInt32 established;
    UInt32 closed;
    CSocketCondition( CSocketRecorder& r, UInt32 p, UInt32 e, UInt32 c ) : recorder( r ), packets( p ), established( e ), closed( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        return recorder.packets.size() >= packets && recorder.established >= established && recorder.closed >= closed;
    }
};

static DRN::CDRNEndpoint
MakeEndpointOf( const CORE::CString& remote, const CORE::CString& socket, const CORE::CString& local, const CORE::CString& extra = CORE::CString::Empty )
{
    DRN::CDRNEndpoint endpoint;
    endpoint.FromString( "eos-p2p://" + remote + "/" + socket + "?local=" + local + extra );
    return endpoint;
}

/*-------------------------------------------------------------------------*/

/** A DRN session over the eos-p2p channel: a node and a session of it */
class CSessionNode
{
    public:

    DRN::CDRNNode node;
    DRN::CDRNSession* session;

    CSessionNode( void ) : node(), session( GUCEF_NULL ) {}
    ~CSessionNode() { node.Stop(); }

    bool Start( const DRN::CDRNNodeSettings& settings )
    {
        if ( !node.SetSettings( settings ) || !node.Start() )
            return false;
        session = node.CreateSession();
        return GUCEF_NULL != session;
    }
};

class CSessionsActive : public CCondition
{
    public:
    GUCEF::vector< DRN::CDRNSession* > sessions;
    UInt32 members;
    CSessionsActive( UInt32 m ) : sessions(), members( m ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        for ( size_t i=0; i<sessions.size(); ++i )
            if ( !sessions[ i ]->IsActive() || sessions[ i ]->GetMemberCount() != members )
                return false;
        return true;
    }
};

class CGridValueIs : public CCondition
{
    public:
    DRN::CDRNSession& session;
    CORE::CString gridName;
    UInt32 owner;
    CORE::CVariant key;
    CORE::CVariant expected;
    CGridValueIs( DRN::CDRNSession& s, const CORE::CString& g, UInt32 o, const CORE::CVariant& k, const CORE::CVariant& e ) : session( s ), gridName( g ), owner( o ), key( k ), expected( e ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNDataGrid* grid = session.FindGrid( gridName, owner );
        if ( GUCEF_NULL == grid || !grid->IsSnapshotComplete() )
            return false;
        CORE::CVariant value;
        return grid->GetValue( key, value ) && DRN::CDRNVariantCodec::IsSame( value, expected );
    }
};

static DRN::CDRNNodeSettings
MakeNodeSettings( void )
{
    DRN::CDRNNodeSettings settings;
    settings.echoIntervalMs = 50;
    settings.pulseIntervalMs = 1;
    return settings;
}

/** Hosts a session on user a and joins it with user b over the channel; the grid of a is replicated to b while the network misbehaves as set */
static bool
ReplicateOverEos( CEosFixture& fixture, const CORE::CString& extraParams, const CORE::CString& socket, CORE::CString& failedStep )
{
    CP2PUser a( fixture[ 0 ] );
    CP2PUser b( fixture[ 1 ] );
    if ( GUCEF_NULL == a.router || GUCEF_NULL == b.router )
    {
        failedStep = "login";
        return false;
    }

    CSessionNode host;
    CSessionNode joiner;
    DRN::CDRNNodeSettings hostSettings = MakeNodeSettings();
    hostSettings.listenEndpoints.FromString( "eos-p2p://" + a.Id() + "/" + socket + "?local=" + a.Id() + "&acceptAny=1" + extraParams );
    if ( !host.Start( hostSettings ) )
    {
        failedStep = "host node";
        return false;
    }
    DRN::CDRNSessionSettings sessionSettings;
    sessionSettings.sessionName = "eos-direct";
    sessionSettings.topology = DRN::DRN_TOPOLOGY_CLIENT_SERVER;
    if ( !host.session->Host( sessionSettings ) )
    {
        failedStep = "host";
        return false;
    }
    if ( !joiner.Start( MakeNodeSettings() ) )
    {
        failedStep = "joiner node";
        return false;
    }
    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = "eos-direct";
    if ( !joiner.session->Join( DRN::CDRNEndpointList( MakeEndpointOf( a.Id(), socket, b.Id(), extraParams ) ), joinSettings ) )
    {
        failedStep = "join";
        return false;
    }
    CSessionsActive active( 2 );
    active.sessions.push_back( host.session );
    active.sessions.push_back( joiner.session );
    if ( !PumpUntil( active, 15000 ) )
    {
        failedStep = "membership";
        return false;
    }

    DRN::CDRNDataGridSettings settings;
    settings.name = "counter";
    settings.authority = DRN::DRN_AUTHORITY_OWNER;
    settings.sendRateHz = 30;
    DRN::CDRNDataGrid* grid = host.session->CreateGrid( settings );
    if ( GUCEF_NULL == grid )
    {
        failedStep = "grid";
        return false;
    }
    grid->SetValue( "v", CORE::CVariant( (UInt32) 0 ) );
    joiner.session->Subscribe( "counter" );
    CGridValueIs subscribed( *joiner.session, "counter", 1, "v", CORE::CVariant( (UInt32) 0 ) );
    if ( !PumpUntil( subscribed, 15000 ) )
    {
        failedStep = "subscribe";
        return false;
    }

    UInt32 index = grid->GetKeyIndex( "v" );
    for ( UInt32 i=1; i<=300; ++i )
    {
        grid->SetValueByIndex( index, CORE::CVariant( i ) );
        PumpFor( 2 );
    }
    CGridValueIs newest( *joiner.session, "counter", 1, "v", CORE::CVariant( (UInt32) 300 ) );
    if ( !PumpUntil( newest, 20000 ) )
    {
        failedStep = "newest value";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformEosP2PTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING EOS P2P TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "EosP2P" );

    GUCEF_TESTFW_TESTCASE( "Test: Socket names, and sockets of a router are shared by name and counted" )
        try
        {
            ASSERT_TRUE( CEosP2PSocket::IsValidSocketName( "game-1_A" ) );
            ASSERT_FALSE( CEosP2PSocket::IsValidSocketName( "" ) || CEosP2PSocket::IsValidSocketName( "has space" ) || CEosP2PSocket::IsValidSocketName( "a/b" ) );
            char tooLong[ 40 ];
            memset( tooLong, 'a', sizeof( tooLong ) );
            tooLong[ 33 ] = '\0';
            ASSERT_FALSE( CEosP2PSocket::IsValidSocketName( tooLong ) );
            tooLong[ 32 ] = '\0';
            ASSERT_TRUE( CEosP2PSocket::IsValidSocketName( tooLong ) );

            CEosFixture fixture( 1, false );
            CP2PUser user( fixture[ 0 ] );
            REQUIRE_TRUE( GUCEF_NULL != user.router );
            ASSERT_TRUE( 0 == user.router->GetSocketCount() );
            CEosP2PSocket* first = user.router->AcquireSocket( "one" );
            CEosP2PSocket* again = user.router->AcquireSocket( "one" );
            CEosP2PSocket* second = user.router->AcquireSocket( "two" );
            ASSERT_TRUE( GUCEF_NULL != first && first == again && second != first && 2 == user.router->GetSocketCount() );
            ASSERT_TRUE( GUCEF_NULL == user.router->AcquireSocket( "not a name" ) );
            ASSERT_TRUE( user.router->FindSocket( "one" ) == first && GUCEF_NULL == user.router->FindSocket( "three" ) );
            user.router->ReleaseSocket( first );
            ASSERT_TRUE( 2 == user.router->GetSocketCount() && user.router->FindSocket( "one" ) == first );
            user.router->ReleaseSocket( again );
            ASSERT_TRUE( 1 == user.router->GetSocketCount() && GUCEF_NULL == user.router->FindSocket( "one" ) );
            user.router->ReleaseSocket( second );
            ASSERT_TRUE( 0 == user.router->GetSocketCount() );
            PumpFor( 10 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Who may connect is the socket's decision: nobody by default, the list, anybody; the refused hear nothing and cost nothing" )
        try
        {
            CEosFixture fixture( 3, false );
            CP2PUser a( fixture[ 0 ] );
            CP2PUser b( fixture[ 1 ] );
            CP2PUser c( fixture[ 2 ] );
            REQUIRE_TRUE( GUCEF_NULL != a.router && GUCEF_NULL != b.router && GUCEF_NULL != c.router );
            CEosP2PSocket* sockA = a.router->AcquireSocket( "game" );
            CEosP2PSocket* sockB = b.router->AcquireSocket( "game" );
            CEosP2PSocket* sockC = c.router->AcquireSocket( "game" );
            CSocketRecorder recA, recB, recC;
            recA.SubscribeTo( sockA );
            recB.SubscribeTo( sockB );
            recC.SubscribeTo( sockC );

            // a socket nobody put a policy on takes nobody
            ASSERT_TRUE( CEosP2PSocket::ACCEPT_NONE == sockB->GetAcceptPolicy() );
            ASSERT_TRUE( sockA->SendPacket( b.Id(), "hello", 5, EOSRELIABILITY_UNRELIABLE_UNORDERED ) );
            PumpFor( 100 );
            ASSERT_TRUE( 1 == sockB->GetRefusedConnectionCount() && 0 == sockB->GetConnectionCount() && recB.packets.empty() );
            ASSERT_TRUE( 1 == recB.requests && 0 == recB.requestsAccepted );

            // the list
            CORE::CString::StringSet allowed;
            allowed.insert( a.Id() );
            sockB->SetAcceptPolicy( CEosP2PSocket::ACCEPT_LIST );
            sockB->SetAllowedRemotes( allowed );
            ASSERT_TRUE( sockB->IsRemoteAllowed( a.Id() ) && !sockB->IsRemoteAllowed( c.Id() ) );
            ASSERT_TRUE( sockA->SendPacket( b.Id(), "hello", 5, EOSRELIABILITY_UNRELIABLE_UNORDERED ) );
            CSocketCondition gotHello( recB, 1, 0, 0 );
            ASSERT_TRUE( PumpUntil( gotHello ) );
            ASSERT_TRUE( recB.packets[ 0 ].from == a.Id() && 5 == recB.packets[ 0 ].data.size() && 0 == memcmp( &recB.packets[ 0 ].data[ 0 ], "hello", 5 ) );
            ASSERT_TRUE( sockB->IsConnectedTo( a.Id() ) && 1 == sockB->GetConnectionCount() && 1 == recB.requestsAccepted );
            ASSERT_TRUE( MT::GetCurrentTaskID() == recB.threadId );

            // and back over the same connection
            ASSERT_TRUE( sockB->SendPacket( a.Id(), "welcome", 7, EOSRELIABILITY_RELIABLE_ORDERED, 3 ) );
            CSocketCondition gotWelcome( recA, 1, 1, 0 );
            ASSERT_TRUE( PumpUntil( gotWelcome ) );
            ASSERT_TRUE( 7 == recA.packets[ 0 ].data.size() && 3 == recA.packets[ 0 ].channel );

            // somebody that is not on the list
            ASSERT_TRUE( sockC->SendPacket( b.Id(), "let me in", 9, EOSRELIABILITY_UNRELIABLE_UNORDERED ) );
            PumpFor( 100 );
            ASSERT_TRUE( 2 == sockB->GetRefusedConnectionCount() && !sockB->IsConnectedTo( c.Id() ) && 1 == recB.packets.size() );

            // anybody
            sockB->SetAcceptPolicy( CEosP2PSocket::ACCEPT_ANY );
            ASSERT_TRUE( sockC->SendPacket( b.Id(), "let me in", 9, EOSRELIABILITY_UNRELIABLE_UNORDERED ) );
            CSocketCondition gotThird( recB, 2, 0, 0 );
            ASSERT_TRUE( PumpUntil( gotThird ) );
            ASSERT_TRUE( sockB->IsConnectedTo( c.Id() ) && 2 == sockB->GetConnectionCount() );

            // allow and disallow
            sockB->DisallowRemote( a.Id() );
            ASSERT_FALSE( sockB->IsRemoteAllowed( a.Id() ) && CEosP2PSocket::ACCEPT_LIST == sockB->GetAcceptPolicy() );
            sockB->AllowRemote( a.Id() );

            recA.UnsubscribeAllFromObserver();
            recB.UnsubscribeAllFromObserver();
            recC.UnsubscribeAllFromObserver();
            a.router->ReleaseSocket( sockA );
            b.router->ReleaseSocket( sockB );
            c.router->ReleaseSocket( sockC );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The number of connections of a socket is limited, a packet has a size limit, and closing a connection is told to the other side" )
        try
        {
            CEosFixture fixture( 3, false );
            CP2PUser a( fixture[ 0 ] );
            CP2PUser b( fixture[ 1 ] );
            CP2PUser c( fixture[ 2 ] );
            CEosP2PSocket* sockA = a.router->AcquireSocket( "game" );
            CEosP2PSocket* sockB = b.router->AcquireSocket( "game" );
            CEosP2PSocket* sockC = c.router->AcquireSocket( "game" );
            CSocketRecorder recA, recB, recC;
            recA.SubscribeTo( sockA );
            recB.SubscribeTo( sockB );
            recC.SubscribeTo( sockC );
            sockB->SetAcceptPolicy( CEosP2PSocket::ACCEPT_ANY );
            sockB->SetMaxConnections( 1 );
            ASSERT_TRUE( 1 == sockB->GetMaxConnections() );

            ASSERT_TRUE( sockA->SendPacket( b.Id(), "one", 3, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition first( recB, 1, 0, 0 );
            ASSERT_TRUE( PumpUntil( first ) );
            ASSERT_TRUE( sockC->SendPacket( b.Id(), "two", 3, EOSRELIABILITY_RELIABLE_ORDERED ) );
            PumpFor( 100 );
            ASSERT_TRUE( 1 == sockB->GetRefusedConnectionCount() && 1 == sockB->GetConnectionCount() && 1 == recB.packets.size() );

            // the limit holds for the connections that this side opens too
            sockB->SetMaxConnections( 1 );
            ASSERT_FALSE( sockB->SendPacket( c.Id(), "out", 3, EOSRELIABILITY_UNRELIABLE_UNORDERED ) );

            // the size limit of the SDK
            GUCEF::vector< UInt8 > big( 1171, 0x5a );
            ASSERT_TRUE( sockA->SendPacket( b.Id(), &big[ 0 ], 1170, EOSRELIABILITY_RELIABLE_ORDERED ) );
            ASSERT_FALSE( sockA->SendPacket( b.Id(), &big[ 0 ], 1171, EOSRELIABILITY_RELIABLE_ORDERED ) );
            ASSERT_FALSE( sockA->SendPacket( b.Id(), GUCEF_NULL, 5, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition largest( recB, 2, 0, 0 );
            ASSERT_TRUE( PumpUntil( largest ) );
            ASSERT_TRUE( 1170 == recB.packets[ 1 ].data.size() && 1170 == fixture.world.largestPacket );
            ASSERT_TRUE( 2 == sockA->GetSentPacketCount() && 2 == sockB->GetReceivedPacketCount() );

            // closing is told: the other side ends it too and the packets in flight do not arrive
            sockA->CloseConnection( b.Id() );
            ASSERT_TRUE( 1 == recA.closed && 0 == sockA->GetConnectionCount() );
            CSocketCondition closed( recB, 2, 0, 1 );
            ASSERT_TRUE( PumpUntil( closed ) );
            ASSERT_TRUE( 0 == sockB->GetConnectionCount() && !sockB->IsConnectedTo( a.Id() ) );

            // and the room is there again
            ASSERT_TRUE( sockC->SendPacket( b.Id(), "three", 5, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition third( recB, 3, 0, 1 );
            ASSERT_TRUE( PumpUntil( third ) );

            recA.UnsubscribeAllFromObserver();
            recB.UnsubscribeAllFromObserver();
            recC.UnsubscribeAllFromObserver();
            a.router->ReleaseSocket( sockA );
            b.router->ReleaseSocket( sockB );
            c.router->ReleaseSocket( sockC );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Unreliable packets get lost, doubled and reordered by the network, reliable ordered ones do not" )
        try
        {
            CEosFixture fixture( 2, false );
            CP2PUser a( fixture[ 0 ] );
            CP2PUser b( fixture[ 1 ] );
            CEosP2PSocket* sockA = a.router->AcquireSocket( "game" );
            CEosP2PSocket* sockB = b.router->AcquireSocket( "game" );
            CSocketRecorder recA, recB;
            recB.SubscribeTo( sockB );
            sockB->SetAcceptPolicy( CEosP2PSocket::ACCEPT_ANY );

            // the connection first, then the weather
            ASSERT_TRUE( sockA->SendPacket( b.Id(), "x", 1, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition up( recB, 1, 0, 0 );
            ASSERT_TRUE( PumpUntil( up ) );
            fixture.world.network.lossRate = 0.3;
            fixture.world.network.duplicateRate = 0.2;
            fixture.world.network.reorderRate = 0.3;
            fixture.world.network.reorderMs = 20;

            for ( UInt32 i=0; i<200; ++i )
            {
                UInt8 value[ 2 ] = { (UInt8) ( i & 0xff ), 0 };
                ASSERT_TRUE( sockA->SendPacket( b.Id(), value, 2, EOSRELIABILITY_UNRELIABLE_UNORDERED, 1 ) );
                if ( 0 == i % 8 )
                    PumpFor( 1 );
            }
            PumpFor( 200 );
            UInt32 unreliable = 0;
            for ( size_t i=1; i<recB.packets.size(); ++i )
                if ( 1 == recB.packets[ i ].channel )
                    ++unreliable;
            ASSERT_TRUE( fixture.world.packetsDropped > 0 && fixture.world.packetsDuplicated > 0 );
            ASSERT_TRUE( unreliable > 60 && unreliable < 260 && unreliable != 200 );

            // reliable ordered: all of them, once, in order, whatever the weather
            recB.packets.clear();
            for ( UInt32 i=0; i<100; ++i )
            {
                UInt8 value[ 2 ] = { (UInt8) i, 7 };
                ASSERT_TRUE( sockA->SendPacket( b.Id(), value, 2, EOSRELIABILITY_RELIABLE_ORDERED, 2 ) );
                if ( 0 == i % 8 )
                    PumpFor( 1 );
            }
            CSocketCondition all( recB, 100, 0, 0 );
            ASSERT_TRUE( PumpUntil( all ) );
            PumpFor( 100 );
            UInt32 ordered = 0;
            bool inOrder = true;
            for ( size_t i=0; i<recB.packets.size(); ++i )
            {
                if ( 2 == recB.packets[ i ].channel )
                {
                    inOrder = inOrder && recB.packets[ i ].data[ 0 ] == (UInt8) ordered;
                    ++ordered;
                }
            }
            ASSERT_TRUE( 100 == ordered && inOrder );

            recB.UnsubscribeAllFromObserver();
            a.router->ReleaseSocket( sockA );
            b.router->ReleaseSocket( sockB );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: When the direct way is blocked the relay carries the connection, and when relaying is not allowed the connection fails and is told" )
        try
        {
            CEosFixture fixture( 2, false );
            CP2PUser a( fixture[ 0 ] );
            CP2PUser b( fixture[ 1 ] );
            CEosP2PSocket* sockA = a.router->AcquireSocket( "game" );
            CEosP2PSocket* sockB = b.router->AcquireSocket( "game" );
            CSocketRecorder recA, recB;
            recA.SubscribeTo( sockA );
            recB.SubscribeTo( sockB );
            sockB->SetAcceptPolicy( CEosP2PSocket::ACCEPT_ANY );

            // direct works
            ASSERT_TRUE( sockA->SendPacket( b.Id(), "direct", 6, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition direct( recA, 0, 1, 0 );
            ASSERT_TRUE( PumpUntil( direct ) );
            ASSERT_TRUE( EOSNETWORK_DIRECT == recA.lastNetworkType && 1 == fixture.world.directConnections );
            sockA->CloseConnection( b.Id() );
            PumpFor( 50 );

            // blocked: through the relay
            fixture.world.network.directBlocked = true;
            ASSERT_TRUE( a.router->SetRelayControl( EOSRELAY_ALLOW_RELAYS ) );
            ASSERT_TRUE( EOSRELAY_ALLOW_RELAYS == a.router->GetRelayControl() );
            recA.established = 0;
            ASSERT_TRUE( sockA->SendPacket( b.Id(), "relayed", 7, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition relayed( recA, 0, 1, 0 );
            ASSERT_TRUE( PumpUntil( relayed ) );
            ASSERT_TRUE( EOSNETWORK_RELAYED == recA.lastNetworkType && 1 == fixture.world.relayedConnections );
            CSocketCondition seen( recB, 2, 0, 0 );
            ASSERT_TRUE( PumpUntil( seen ) );
            sockA->CloseConnection( b.Id() );
            PumpFor( 50 );

            // blocked and no relay: no connection, and the one that asked hears so
            ASSERT_TRUE( a.router->SetRelayControl( EOSRELAY_NO_RELAYS ) );
            recA.closed = 0;
            recA.established = 0;
            UInt32 delivered = fixture.world.packetsDelivered;
            ASSERT_TRUE( sockA->SendPacket( b.Id(), "never", 5, EOSRELIABILITY_RELIABLE_ORDERED ) );
            CSocketCondition failed( recA, 0, 0, 1 );
            ASSERT_TRUE( PumpUntil( failed, 10000 ) );
            ASSERT_TRUE( 0 == recA.established && delivered == fixture.world.packetsDelivered && 0 == sockA->GetConnectionCount() );
            ASSERT_TRUE( fixture.world.refusedConnections >= 1 );

            recA.UnsubscribeAllFromObserver();
            recB.UnsubscribeAllFromObserver();
            a.router->ReleaseSocket( sockA );
            b.router->ReleaseSocket( sockB );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The endpoint of the channel names a peer and a socket, and what is not an endpoint of it is refused with the reason" )
        try
        {
            CEosP2PEndpointInfo info;
            CORE::CString reason;
            DRN::CDRNEndpoint good = MakeEndpointOf( "00112233445566778899aabbccdd10", "game-1", "00112233445566778899aabbccdd11", "&reliability=reliable&maxPacketSize=900&connectTimeoutMs=2500&tickIntervalInMs=4&relay=force" );
            ASSERT_TRUE( info.Parse( good, false, reason ) );
            ASSERT_TRUE( info.remoteUser == "00112233445566778899aabbccdd10" && info.socketName == "game-1" && info.localUser == "00112233445566778899aabbccdd11" );
            ASSERT_TRUE( EOSRELIABILITY_RELIABLE_UNORDERED == info.reliability && 900 == info.maxPacketSize && 2500 == info.connectTimeoutInMs && 4 == info.tickIntervalInMs );
            ASSERT_TRUE( info.hasRelayControl && EOSRELAY_FORCE_RELAYS == info.relayControl );

            CEosP2PEndpointInfo ordered;
            ASSERT_TRUE( ordered.Parse( MakeEndpointOf( "00112233445566778899aabbccdd10", "g", "00112233445566778899aabbccdd11", "&reliability=ordered" ), false, reason ) );
            ASSERT_TRUE( EOSRELIABILITY_RELIABLE_ORDERED == ordered.reliability );

            // defaults
            CEosP2PEndpointInfo defaults;
            ASSERT_TRUE( defaults.Parse( MakeEndpointOf( "00112233445566778899aabbccdd10", "g", "00112233445566778899aabbccdd11" ), false, reason ) );
            ASSERT_TRUE( 1170 == defaults.maxPacketSize && 15000 == defaults.connectTimeoutInMs && 10 == defaults.tickIntervalInMs && !defaults.hasRelayControl && EOSRELIABILITY_UNRELIABLE_UNORDERED == defaults.reliability );

            // listening
            CEosP2PEndpointInfo listening;
            ASSERT_TRUE( listening.Parse( MakeEndpointOf( "*", "game", "00112233445566778899aabbccdd11", "&acceptAny=1&allow=aaa,bbb&maxConnections=5" ), true, reason ) );
            ASSERT_TRUE( listening.acceptAny && 2 == listening.allowedRemotes.size() && 5 == listening.maxConnections );

            // not endpoints of it
            DRN::CDRNEndpoint bad;
            CEosP2PEndpointInfo refused;
            bad.FromString( "eos-p2p://not%20a%20user/game" );
            ASSERT_FALSE( refused.Parse( bad, false, reason ) );
            ASSERT_FALSE( refused.Parse( MakeEndpointOf( "00112233445566778899aabbccdd10", "bad socket name", "x" ), false, reason ) );
            ASSERT_FALSE( reason.IsNULLOrEmpty() );
            ASSERT_FALSE( refused.Parse( MakeEndpointOf( "00112233445566778899aabbccdd10", "game", "x", "&maxPacketSize=2000" ), false, reason ) );
            ASSERT_FALSE( refused.Parse( MakeEndpointOf( "00112233445566778899aabbccdd10", "game", "x", "&reliability=sometimes" ), false, reason ) );
            ASSERT_FALSE( refused.Parse( MakeEndpointOf( "00112233445566778899aabbccdd10", "game", "x", "&relay=maybe" ), false, reason ) );

            DRN::CDRNEndpoint made = CEosP2PEndpointInfo::MakeEndpoint( "00112233445566778899aabbccdd10", "game" );
            ASSERT_TRUE( made.ToString() == "eos-p2p://00112233445566778899aabbccdd10/game" );
            ASSERT_TRUE( CEosP2PEndpointInfo::IsValidUserId( "abc123" ) && !CEosP2PEndpointInfo::IsValidUserId( "ab c" ) && !CEosP2PEndpointInfo::IsValidUserId( "" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The plugin of the channel registers eos-p2p for channels and listeners and takes them away again" )
        try
        {
            DRN::CDRNGlobal* global = DRN::CDRNGlobal::Instance();
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            ASSERT_TRUE( global->GetChannelFactory().IsConstructible( "eos-p2p" ) && global->GetChannelListenerFactory().IsConstructible( "eos-p2p" ) );
            TChannelPtr channel = global->GetChannelFactory().Create( "eos-p2p" );
            ASSERT_TRUE( !channel.IsNULL() && channel->GetChannelType() == "eos-p2p" );
            ASSERT_FALSE( channel->IsConnected() );
            const DRN::CDRNChannelCapabilities& capabilities = channel->GetCapabilities();
            ASSERT_TRUE( capabilities.isDatagram && !capabilities.isReliable && 1170 == capabilities.maxPacketSize );
            channel = TChannelPtr();
            CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "eosp2p" );
            ASSERT_FALSE( global->GetChannelFactory().IsConstructible( "eos-p2p" ) || global->GetChannelListenerFactory().IsConstructible( "eos-p2p" ) );
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            ASSERT_TRUE( global->GetChannelFactory().IsConstructible( "eos-p2p" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A channel that cannot be connected says no: not logged in, bad endpoint, an unknown local user, the pulse generator of another thread" )
        try
        {
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            DRN::CDRNGlobal* global = DRN::CDRNGlobal::Instance();
            CEosFixture fixture( 2, false );

            // nobody is logged in
            TChannelPtr channel = global->GetChannelFactory().Create( "eos-p2p" );
            ASSERT_FALSE( channel->Connect( MakeEndpointOf( fixture[ 1 ].peer.puid, "game", fixture[ 0 ].peer.puid ) ) );

            CP2PUser a( fixture[ 0 ] );
            CP2PUser b( fixture[ 1 ] );
            // a user that is not logged in
            TChannelPtr other = global->GetChannelFactory().Create( "eos-p2p" );
            ASSERT_FALSE( other->Connect( MakeEndpointOf( b.Id(), "game", "00112233445566778899aabbccdd99" ) ) );
            // two users are logged in, none is named
            TChannelPtr ambiguous = global->GetChannelFactory().Create( "eos-p2p" );
            DRN::CDRNEndpoint noLocal;
            noLocal.FromString( "eos-p2p://" + b.Id() + "/game" );
            ASSERT_FALSE( ambiguous->Connect( noLocal ) );
            // not an endpoint of the channel
            TChannelPtr bad = global->GetChannelFactory().Create( "eos-p2p" );
            DRN::CDRNEndpoint badEndpoint;
            badEndpoint.FromString( "eos-p2p://" + b.Id() + "/bad%20name?local=" + a.Id() );
            ASSERT_FALSE( bad->Connect( badEndpoint ) );

            // a pulse generator of its own: the glue ticks on the one of the application
            TChannelPtr threaded = global->GetChannelFactory().Create( "eos-p2p" );
            threaded->SetPulseGenerator( CORE::CPulseGenerator::CreateSharedObj() );
            ASSERT_FALSE( threaded->Connect( MakeEndpointOf( b.Id(), "game", a.Id() ) ) );

            // the one that is named works
            TChannelPtr good = global->GetChannelFactory().Create( "eos-p2p" );
            ASSERT_TRUE( good->Connect( MakeEndpointOf( b.Id(), "game", a.Id() ) ) );
            ASSERT_TRUE( good->IsConnected() && good->GetRemoteEndpoint().ToString().HasSubstr( b.Id() ) >= 0 );
            good->Close();
            ASSERT_FALSE( good->IsConnected() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A DRN session runs over EOS P2P: the handshake and the grid replication work on the channel, on a clean network" )
        try
        {
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CEosFixture fixture( 2, false );
            CORE::CString failed;
            ASSERT_TRUE( ReplicateOverEos( fixture, CORE::CString::Empty, "clean", failed ) );
            ASSERT_TRUE( failed.IsNULLOrEmpty() );
            ASSERT_TRUE( fixture.world.largestPacket <= 1170 && fixture.world.packetsDelivered > 10 );
            ASSERT_TRUE( 1 == fixture.world.directConnections && 0 == fixture.world.relayedConnections );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A DRN session over EOS P2P converges with loss, duplicates and reordering: DRN supplies the reliability" )
        try
        {
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CEosFixture fixture( 2, false );
            fixture.world.network.lossRate = 0.2;
            fixture.world.network.duplicateRate = 0.1;
            fixture.world.network.reorderRate = 0.2;
            fixture.world.network.reorderMs = 15;
            fixture.world.network.latencyMs = 3;
            CORE::CString failed;
            bool converged = ReplicateOverEos( fixture, CORE::CString::Empty, "weather", failed );
            ASSERT_TRUE( converged );
            ASSERT_TRUE( failed.IsNULLOrEmpty() );
            ASSERT_TRUE( fixture.world.packetsDropped > 0 && fixture.world.packetsDuplicated > 0 );
            ASSERT_TRUE( fixture.world.largestPacket <= 1170 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A DRN session over EOS P2P in relay only mode: the relay carries it, and nothing is direct" )
        try
        {
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CEosFixture fixture( 2, false );
            fixture.world.network.directBlocked = true;
            fixture.world.network.relayExtraMs = 4;
            fixture.world.network.lossRate = 0.1;
            fixture.world.network.reorderRate = 0.1;
            fixture.world.network.reorderMs = 10;
            CORE::CString failed;
            ASSERT_TRUE( ReplicateOverEos( fixture, "&relay=force", "relayed", failed ) );
            ASSERT_TRUE( 0 == fixture.world.directConnections && fixture.world.relayedConnections >= 1 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A session that cannot get through, no direct way and no relay, ends with the join failing and nothing delivered" )
        try
        {
            ASSERT_TRUE( LoadTestPlugin( "drnchannelpluginEOSP2P", "eosp2p" ) );
            CEosFixture fixture( 2, false );
            fixture.world.network.directBlocked = true;
            CORE::CString failed;
            ASSERT_FALSE( ReplicateOverEos( fixture, "&relay=none&connectTimeoutMs=800", "blocked", failed ) );
            ASSERT_TRUE( failed == "membership" );
            ASSERT_TRUE( fixture.world.refusedConnections >= 1 && 0 == fixture.world.packetsDelivered );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
