/*
 *  gucefDRNUTILS_TestApp: GUCEF test application
 *
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

#include "TestDrnUtilsLanDiscovery.h"

#ifndef GUCEF__TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF__TEST_FRAMEWORK_H
#endif /* GUCEF__TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_GUCEFCORE_H
#endif /* GUCEF_GUCEFCORE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_TESTRPCSUPPORT_H
#include "TestRpcSupport.h"
#define GUCEF_TESTRPCSUPPORT_H
#endif /* GUCEF_TESTRPCSUPPORT_H ? */

#ifndef GUCEF_TESTDRNUTILSREMOTEWORLD_H
#include "TestDrnUtilsRemoteWorld.h"
#define GUCEF_TESTDRNUTILSREMOTEWORLD_H
#endif /* GUCEF_TESTDRNUTILSREMOTEWORLD_H ? */

using namespace GUCEF;
using namespace DRNUTILSTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

namespace {

const char* LanServiceType = "_gucef-assetstore._tcp";

/** Counts the candidate events of a finder and keeps the last candidate */
class CLanCandidateCounter : public CORE::CObserver
{
    public:

    UInt32 found;
    UInt32 updated;
    UInt32 lost;
    COMCORE::CServiceCandidate last;

    CLanCandidateCounter( void ) : CORE::CObserver(), found( 0 ), updated( 0 ), lost( 0 ), last() {}
    virtual ~CLanCandidateCounter() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CLanCandidateCounter";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CServiceFinder::CandidateFoundEvent == eventId )
            ++found;
        else if ( COMCORE::CServiceFinder::CandidateUpdatedEvent == eventId )
            ++updated;
        else if ( COMCORE::CServiceFinder::CandidateLostEvent == eventId )
            ++lost;
        else
            return;
        if ( GUCEF_NULL != eventData )
            last = static_cast< COMCORE::TServiceCandidateEventData* >( eventData )->GetData();
    }
};

/** A counter reaching a value, for PumpUntil */
class CAtLeast : public CCondition
{
    public:

    const UInt32& counter;
    UInt32 target;

    CAtLeast( const UInt32& c, UInt32 t ) : counter( c ), target( t ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return counter >= target; }
};

/** Whether a broadcast can leave this host: there has to be a route for it */
bool
CanBroadcastHere( void )
{
    COMCORE::CUDPSocket socket( false );
    socket.SetAllowBroadcast( true );
    if ( !socket.Open( (UInt16) 0 ) )
        return false;
    const char probe[] = "probe";
    bool sent = socket.SendPacketTo( COMCORE::CIPv4Address( 0xFFFFFFFF, COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( (UInt16) 9 ) ), probe, 5 ) == 5;
    socket.Close();
    return sent;
}

/** A "DiscoveryClient" node of the broadcast type that only talks unicast on loopback */
void
AddLoopbackClient( CORE::CDataNode& clientConfig, UInt16 port, UInt16 peerPort )
{
    CORE::CDataNode* client = clientConfig.AddChild( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
    client->SetAttribute( "type", "broadcast" );
    client->SetAttribute( "port", CORE::ToString( port ) );
    client->SetAttribute( "useLimitedBroadcast", "false" );
    client->SetAttribute( "useDirectedBroadcasts", "false" );
    if ( 0 != peerPort )
        client->SetAttribute( "extraTargets", "127.0.0.1:" + CORE::ToString( peerPort ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDrnUtilsLanDiscoveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsLanDiscovery" );

    GUCEF_TESTFW_TESTCASE( "Test: LAN discovery of a host and of a client is on by default, and the settings survive a data tree" )
        try
        {
            DRNUTILS::CRemoteServiceHostSettings defaults;
            ASSERT_TRUE( defaults.lanDiscovery );
            DRNUTILS::CRemoteServiceConnectionSettings clientDefaults;
            ASSERT_TRUE( clientDefaults.lanDiscovery );
            ASSERT_TRUE( "_gucef-service._tcp" == clientDefaults.lanDiscoveryFinder.serviceType );
            ASSERT_TRUE( 2 == clientDefaults.lanDiscoveryFinder.defaultClientTypes.size() );

            // the defaults survive a data tree: saved and read into settings that had it off
            CORE::CDataNode defaultTree( "RemoteServiceHost", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( defaults.SaveConfig( defaultTree ) );
            DRNUTILS::CRemoteServiceHostSettings fromDefaults;
            fromDefaults.lanDiscovery = false;
            fromDefaults.lanDiscoveryKeyFingerprint = false;
            ASSERT_TRUE( fromDefaults.LoadConfig( defaultTree ) );
            ASSERT_TRUE( fromDefaults.lanDiscovery && fromDefaults.lanDiscoveryKeyFingerprint && fromDefaults.lanDiscoveryPairingPort );
            ASSERT_TRUE( "_gucef-service._tcp" == fromDefaults.lanDiscoveryAdvertiser.serviceType && 30 == fromDefaults.lanDiscoveryAdvertiser.ttlInSecs );
            // an empty configuration keeps the defaults: a bare product advertises
            DRNUTILS::CRemoteServiceHostSettings bare;
            ASSERT_TRUE( bare.LoadConfig( CORE::CDataNode( "RemoteServiceHost", GUCEF_DATATYPE_OBJECT ) ) );
            ASSERT_TRUE( bare.lanDiscovery );
            ASSERT_TRUE( "_gucef-service._tcp" == defaults.lanDiscoveryAdvertiser.serviceType );
            ASSERT_TRUE( defaults.lanDiscoveryKeyFingerprint && defaults.lanDiscoveryPairingPort );
            ASSERT_TRUE( defaults.lanDiscoveryProtocolVersion.IsNULLOrEmpty() );

            DRNUTILS::CRemoteServiceHostSettings settings;
            settings.lanDiscovery = false;
            settings.lanDiscoveryAdvertiser.serviceType = LanServiceType;
            settings.lanDiscoveryAdvertiser.displayName = "Asset store";
            settings.lanDiscoveryAdvertiser.ttlInSecs = 12;
            settings.lanDiscoveryAdvertiser.SetClientTypes( "broadcast,multicast" );
            settings.lanDiscoveryKeyFingerprint = false;
            settings.lanDiscoveryPairingPort = false;
            settings.lanDiscoveryProtocolVersion = "2.1";
            CORE::CDataNode tree( "RemoteServiceHost", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( settings.SaveConfig( tree ) );

            DRNUTILS::CRemoteServiceHostSettings back;
            ASSERT_TRUE( back.LoadConfig( tree ) );
            ASSERT_TRUE( !back.lanDiscovery && LanServiceType == back.lanDiscoveryAdvertiser.serviceType && "Asset store" == back.lanDiscoveryAdvertiser.displayName );
            ASSERT_TRUE( 12 == back.lanDiscoveryAdvertiser.ttlInSecs && !back.lanDiscoveryKeyFingerprint && !back.lanDiscoveryPairingPort && "2.1" == back.lanDiscoveryProtocolVersion );
            ASSERT_TRUE( "broadcast,multicast" == back.lanDiscoveryAdvertiser.clientConfig.GetAttributeValue( "discoveryClients" ).AsString() );

            // a mistyped key is still refused
            tree.SetAttribute( "lanDiscovry", "true" );
            DRNUTILS::CRemoteServiceHostSettings refused;
            ASSERT_FALSE( refused.LoadConfig( tree ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A host configured without LAN discovery has no advertiser" )
        try
        {
            CRemoteWorld world( "lan-off" );
            world.settings.lanDiscovery = false;
            ASSERT_TRUE( world.StartHost() );
            ASSERT_TRUE( GUCEF_NULL == world.host->GetLanAdvertiser() );
            ASSERT_FALSE( world.host->IsAdvertisingOnLan() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A host announces itself on the LAN, a finder finds its port, id and key fingerprint, sees product properties and loses it at the drain" )
        try
        {
            UInt16 hostDiscoveryPort = TESTFW::FindFreeUdpPort( 47861 );
            UInt16 finderPort = TESTFW::FindFreeUdpPort( 47862 );

            CRemoteWorld world( "lan-on" );
            world.settings.lanDiscovery = true;
            world.settings.lanDiscoveryAdvertiser.serviceType = LanServiceType;
            world.settings.lanDiscoveryAdvertiser.displayName = "Asset store test";
            world.settings.pairingPort = TESTFW::FindFreeTcpAndUdpPort( 0 );
            AddLoopbackClient( world.settings.lanDiscoveryAdvertiser.clientConfig, hostDiscoveryPort, finderPort );
            ASSERT_TRUE( world.StartHost() );
            ASSERT_TRUE( world.host->IsAdvertisingOnLan() );

            COMCORE::CServiceAnnouncement made;
            world.host->MakeLanAnnouncement( made );
            CORE::CString reason;
            ASSERT_TRUE( made.IsValid( reason ) );

            // a look on a thread of the pool, while this thread drives the host
            COMCORE::CServiceFinderSettings finderSettings;
            finderSettings.serviceType = LanServiceType;
            finderSettings.findTimeoutMs = 5000;
            AddLoopbackClient( finderSettings.clientConfig, finderPort, hostDiscoveryPort );
            CORE::CFutureResult future = COMCORE::CServiceFinder::FindOnceAsync( finderSettings );
            CFutureReady ready( future );
            ASSERT_TRUE( PumpUntil( ready, 8000 ) );
            COMCORE::CServiceFinder::TServiceCandidateVector candidates;
            bool found = false;
            ASSERT_TRUE( COMCORE::CServiceFinder::GetFindOnceResult( future, candidates, found ) );
            ASSERT_TRUE( found && 1 == candidates.size() );
            if ( 1 == candidates.size() )
            {
                const COMCORE::CServiceAnnouncement& heard = candidates[ 0 ].announcement;
                ASSERT_TRUE( heard.instanceId == world.host->GetServiceId() );
                ASSERT_TRUE( "Asset store test" == heard.displayName );
                ASSERT_TRUE( heard.GetPort( "tcp" ) == world.host->GetSessionPort() );
                ASSERT_TRUE( "tcp://127.0.0.1:" + CORE::ToString( world.host->GetSessionPort() ) == heard.endpoints[ 0 ] );
                ASSERT_TRUE( 2 == heard.endpoints.size() && world.settings.sessionEndpoints[ 0 ] == heard.endpoints[ 1 ] );
                ASSERT_TRUE( heard.keyFingerprint == DRNUTILS::CDevicePairingCrypto::Fingerprint( world.host->GetStaticPublicKey() ) );
                ASSERT_FALSE( heard.keyFingerprint.IsNULLOrEmpty() );
                ASSERT_TRUE( "1" == heard.protocolVersion );
                ASSERT_TRUE( CORE::ToString( world.settings.pairingPort ) == heard.properties.GetValueAlways( "pairingPort" ).AsString() );
                // nothing about the product version
                ASSERT_FALSE( heard.properties.HasKey( "serviceVersion" ) );
            }

            // a finder on this thread sees what the product adds, and the drain takes the service away
            COMCORE::CServiceFinder finder( finderSettings );
            CLanCandidateCounter counter;
            counter.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );
            CAtLeast foundOnce( counter.found, 1 );
            ASSERT_TRUE( PumpUntil( foundOnce, 5000 ) );

            CORE::CValueList properties;
            properties.Set( "formats", "gltf,fbx" );
            world.host->SetLanDiscoveryProperties( properties );
            CAtLeast updatedOnce( counter.updated, 1 );
            ASSERT_TRUE( PumpUntil( updatedOnce, 5000 ) );
            ASSERT_TRUE( "gltf,fbx" == counter.last.announcement.properties.GetValueAlways( "formats" ).AsString() );

            // a secret never goes out: the update is refused and the announcement stays as it was
            CORE::CValueList secret;
            secret.Set( "adminToken", "abc" );
            world.host->SetLanDiscoveryProperties( secret );
            ASSERT_FALSE( world.host->GetLanAdvertiser()->GetAnnouncement().properties.HasKey( "adminToken" ) );

            world.host->BeginDrain( "test" );
            ASSERT_FALSE( world.host->IsAdvertisingOnLan() );
            CAtLeast lostOnce( counter.lost, 1 );
            ASSERT_TRUE( PumpUntil( lostOnce, 5000 ) );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A host with the default settings announces itself and a finder with the default settings finds it" )
        GUCEF_TESTFW_SKIP_IF( !CanBroadcastHere(), "There is no route for a broadcast on this host" );
        try
        {
            CRemoteWorld world( "lan-default" );
            ASSERT_TRUE( world.StartHost() );
            ASSERT_TRUE( world.host->IsAdvertisingOnLan() );
            ASSERT_TRUE( "_gucef-service._tcp" == world.host->GetLanAdvertiser()->GetAnnouncement().serviceType );

            COMCORE::CServiceFinderSettings defaults;
            defaults.serviceType = "_gucef-service._tcp";
            COMCORE::CServiceFinder finder( defaults );
            CLanCandidateCounter counter;
            counter.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );
            ASSERT_TRUE( 2 == finder.GetDiscoveryClients().size() );
            CAtLeast foundOnce( counter.found, 1 );
            ASSERT_TRUE( PumpUntil( foundOnce, 5000 ) );
            ASSERT_TRUE( counter.last.announcement.instanceId == world.host->GetServiceId() );
            ASSERT_TRUE( counter.last.announcement.GetPort( "tcp" ) == world.host->GetSessionPort() );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A paired client without an endpoint finds its service on the LAN and connects; another key is not taken; with discovery off it does not connect" )
        try
        {
            UInt16 hostDiscoveryPort = TESTFW::FindFreeUdpPort( 47863 );
            UInt16 spoofPort = TESTFW::FindFreeUdpPort( 47864 );
            UInt16 clientPort = TESTFW::FindFreeUdpPort( 47865 );

            CRemoteWorld world( "lan-connect" );
            world.settings.lanDiscoveryAdvertiser.serviceType = LanServiceType;
            AddLoopbackClient( world.settings.lanDiscoveryAdvertiser.clientConfig, hostDiscoveryPort, clientPort );
            ASSERT_TRUE( world.StartHost() );
            DRNUTILS::CDeviceCredentials credentials;
            CORE::CString error;
            ASSERT_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );

            // a spoofer on the LAN: the same type, another key, an endpoint that leads nowhere
            COMCORE::CServiceAdvertiserSettings spoofSettings;
            spoofSettings.serviceType = LanServiceType;
            AddLoopbackClient( spoofSettings.clientConfig, spoofPort, clientPort );
            COMCORE::CServiceAdvertiser spoofer( spoofSettings );
            COMCORE::CServiceAnnouncement spoof;
            spoof.instanceId = "s-SPOOFEDSERVICE00000";
            spoof.endpoints.push_back( "tcp://127.0.0.1:9" );
            spoof.keyFingerprint = "ZZZZ-ZZZZ-ZZZZ-ZZZZ-ZZZZ";
            ASSERT_TRUE( spoofer.Advertise( spoof ) );

            // the credentials lost their endpoints (a new client, a moved service): no endpoint is configured
            DRNUTILS::CDeviceCredentials bare( credentials );
            bare.sessionEndpoints.clear();
            DRNUTILS::CRemoteServiceConnectionSettings clientSettings = world.ClientSettings();
            clientSettings.hostOverride.Clear();
            clientSettings.lanDiscoveryFinder.serviceType = LanServiceType;
            clientSettings.lanDiscoveryFinder.findMinCandidates = 0;
            clientSettings.lanDiscoveryFinder.findTimeoutMs = 1500;
            CORE::CDataNode* client = clientSettings.lanDiscoveryFinder.clientConfig.AddChild( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
            client->SetAttribute( "type", "broadcast" );
            client->SetAttribute( "port", CORE::ToString( clientPort ) );
            client->SetAttribute( "useLimitedBroadcast", "false" );
            client->SetAttribute( "useDirectedBroadcasts", "false" );
            client->SetAttribute( "extraTargets", "127.0.0.1:" + CORE::ToString( hostDiscoveryPort ) + ",127.0.0.1:" + CORE::ToString( spoofPort ) );

            {
                DRNUTILS::CRemoteServiceConnection connection( world.pulse, clientSettings );
                COMCORE::CServiceFinder::TServiceCandidateVector candidates;
                ASSERT_TRUE( connection.FindLanCandidates( bare, candidates ) );
                ASSERT_TRUE( 1 == candidates.size() );
                if ( 1 == candidates.size() )
                    ASSERT_TRUE( candidates[ 0 ].announcement.instanceId == credentials.serviceId );
                DRNUTILS::CDeviceCredentials extended( bare );
                ASSERT_TRUE( DRNUTILS::CRemoteServiceConnection::AddCandidateEndpoints( extended, candidates ) >= 1 );
                ASSERT_TRUE( 0 == DRNUTILS::CRemoteServiceConnection::AddCandidateEndpoints( extended, candidates ) );

                ASSERT_TRUE( connection.Connect( bare, error ) );
                ASSERT_TRUE( DRNUTILS::REMOTECONNECTION_CONNECTED == connection.GetState() );
                connection.Close();
            }

            // with discovery off a client without an endpoint has nothing to connect to
            clientSettings.lanDiscovery = false;
            {
                DRNUTILS::CRemoteServiceConnection connection( world.pulse, clientSettings );
                COMCORE::CServiceFinder::TServiceCandidateVector candidates;
                ASSERT_FALSE( connection.FindLanCandidates( bare, candidates ) );
                ASSERT_FALSE( connection.Connect( bare, error ) );
            }
            spoofer.Withdraw();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
