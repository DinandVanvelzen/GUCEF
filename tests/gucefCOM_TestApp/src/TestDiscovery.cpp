/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYMANAGER_H
#include "gucefCOMCORE_CDiscoveryManager.h"
#define GUCEF_COMCORE_CDISCOVERYMANAGER_H
#endif /* GUCEF_COMCORE_CDISCOVERYMANAGER_H ? */

#ifndef GUCEF_COM_CUDPDISCOVERYCODEC_H
#include "gucefCOM_CUdpDiscoveryCodec.h"
#define GUCEF_COM_CUDPDISCOVERYCODEC_H
#endif /* GUCEF_COM_CUDPDISCOVERYCODEC_H ? */

#ifndef GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H
#include "gucefCOM_CUdpBroadcastDiscoveryClient.h"
#define GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CUDPBROADCASTDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H
#include "gucefCOM_CUdpMulticastDiscoveryClient.h"
#define GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CUDPMULTICASTDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CDNSMESSAGE_H
#include "gucefCOM_CDnsMessage.h"
#define GUCEF_COM_CDNSMESSAGE_H
#endif /* GUCEF_COM_CDNSMESSAGE_H ? */

#ifndef GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#include "gucefCOM_CMdnsDiscoveryClient.h"
#define GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CMDNSDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#include "gucefCOM_CSsdpDiscoveryClient.h"
#define GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CSSDPDISCOVERYCLIENT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_SOCKETS_H
#include "gucef_test_sockets.h"
#define GUCEF_TEST_SOCKETS_H
#endif /* GUCEF_TEST_SOCKETS_H ? */

#include "TestCode_Pump.h"

#include "TestDiscovery.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt16 UnicastTestPortA = 47821;
const UInt16 UnicastTestPortB = 47822;
const UInt16 BroadcastTestPort = 47823;
const UInt16 MulticastTestPort = 47824;
const UInt16 MdnsTestPort = 47825;
const UInt16 SsdpTestPort = 47826;
const char* TestServiceType = "_gucef-test._udp";
const char* TestSsdpServiceType = "urn:gucef-org:service:test:1";

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Counts the discovery events of a client and keeps the entry of the last one
 */
class CDiscoveryEventRecorder : public CORE::CObserver
{
    public:

    UInt32 discovered;
    UInt32 updated;
    UInt32 lost;
    UInt32 errors;
    COMCORE::CIDiscoveryEntryPtr lastEntry;

    CDiscoveryEventRecorder( void )
        : CORE::CObserver()
        , discovered( 0 )
        , updated( 0 )
        , lost( 0 )
        , errors( 0 )
        , lastEntry()
    {GUCEF_TRACE;

    }

    virtual ~CDiscoveryEventRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CDiscoveryEventRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMCORE::CIDiscoveryClient::EntryDiscoveredEvent == eventId )
            ++discovered;
        else if ( COMCORE::CIDiscoveryClient::EntryUpdatedEvent == eventId )
            ++updated;
        else if ( COMCORE::CIDiscoveryClient::EntryLostEvent == eventId )
            ++lost;
        else if ( COMCORE::CIDiscoveryClient::DiscoveryErrorEvent == eventId )
            ++errors;
        else
            return;

        if ( GUCEF_NULL != eventData && COMCORE::CIDiscoveryClient::DiscoveryErrorEvent != eventId )
            lastEntry = *static_cast< COMCORE::CDiscoveryEntryEventData* >( eventData );
    }
};

/*-------------------------------------------------------------------------*/

COMCORE::CIDiscoveryEntryPtr
MakeEntry( const CORE::CString& serviceType, const CORE::CString& instanceId, const CORE::CString& name, UInt32 ttl )
{GUCEF_TRACE;

    COMCORE::CDiscoveryEntry* entry = GUCEF_NEW COMCORE::CDiscoveryEntry();
    entry->SetServiceType( serviceType );
    entry->SetApplicationInstanceId( instanceId );
    entry->SetInstanceName( name );
    entry->SetTtlInSecs( ttl );
    entry->GetEndpoints().push_back( "udp://0.0.0.0:7777" );
    entry->GetAttributes().Set( "map", "dust" );
    entry->GetAttributes().Set( "players", "3" );
    return COMCORE::CIDiscoveryEntryPtr( entry );
}

/*-------------------------------------------------------------------------*/

CORE::CString
GetAttribute( const COMCORE::CIDiscoveryEntry& entry, const char* key )
{GUCEF_TRACE;

    return entry.GetAttributes().GetValueAlways( key ).AsString();
}

/*-------------------------------------------------------------------------*/

/** Whether a broadcast can leave this host: there has to be a route for it */
bool
CanBroadcast( UInt16 port )
{GUCEF_TRACE;

    COMCORE::CUDPSocket socket( false );
    socket.SetAllowBroadcast( true );
    if ( !socket.Open( (UInt16) 0 ) )
        return false;
    const char probe[] = "probe";
    bool sent = socket.SendPacketTo( COMCORE::CIPv4Address( 0xFFFFFFFF, COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( port ) ), probe, 5 ) == 5;
    socket.Close();
    return sent;
}


/** What a discovery client holds of the process while it browses or advertises: sockets and requests for pulses */
struct SProcessHold
{
    int sockets;
    UInt32 pulseRequestors;

    SProcessHold( void )
        : sockets( GUCEF::TESTFW::CountOpenSockets() )
        , pulseRequestors( CORE::CCoreGlobal::Instance()->GetPulseGenerator()->GetNrOfPeriodicPulseRequestors() )
    {}

    bool operator==( const SProcessHold& other ) const { return sockets == other.sockets && pulseRequestors == other.pulseRequestors; }
};

/**
 *  A client that is not browsing and not advertising any more holds nothing: its sockets are closed (the group left) and
 *  its timer is off, it hears nothing and says nothing. And it works again when it is used again, as often as it is used.
 */
bool
ClientHoldsNothingWhenIdle( COMCORE::CIDiscoveryClient& client, const CORE::CString& serviceType, bool ( *isActive )( COMCORE::CIDiscoveryClient& ) )
{GUCEF_TRACE;

    const SProcessHold idle;
    for ( UInt32 round=0; round<20; ++round )
    {
        // browsing: it holds a socket
        if ( !client.Browse( serviceType ) || !isActive( client ) )
            return false;
        const SProcessHold browsing;
        if ( browsing.sockets <= idle.sockets && idle.sockets >= 0 )
            return false;
        PumpFor( 5 );

        // browsing and advertising, the browse ends: the advertisement still needs the socket
        COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( serviceType, "inst-idle", "Idle Game", 30 );
        if ( !client.Advertise( entry ) )
            return false;
        client.StopBrowse( serviceType );
        if ( !isActive( client ) )
            return false;

        // nothing left to do: nothing is held
        client.Withdraw( entry );
        if ( isActive( client ) )
            return false;
        if ( !( SProcessHold() == idle ) )
            return false;
        PumpFor( 5 );
        if ( !( SProcessHold() == idle ) )
            return false;
    }
    return SProcessHold() == idle;
}

bool IsBroadcastActive( COMCORE::CIDiscoveryClient& client ) { return static_cast< COM::CUdpBroadcastDiscoveryClient& >( client ).IsActive(); }
bool IsMulticastActive( COMCORE::CIDiscoveryClient& client ) { return static_cast< COM::CUdpMulticastDiscoveryClient& >( client ).IsActive(); }
bool IsMdnsActive( COMCORE::CIDiscoveryClient& client ) { return static_cast< COM::CMdnsDiscoveryClient& >( client ).IsActive(); }
bool IsSsdpActive( COMCORE::CIDiscoveryClient& client ) { return static_cast< COM::CSsdpDiscoveryClient& >( client ).IsActive(); }

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformUdpDiscoveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "UdpDiscovery" );

    GUCEF_TESTFW_TESTCASE( "Test: A beacon carries the whole entry and reads back the same" )
        try
        {
            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestServiceType, "inst-1", "Game One", 45 );
            static_cast< COMCORE::CDiscoveryEntry* >( entry.GetPointerAlways() )->GetEndpoints().push_back( "tcp://10.1.2.3:7778" );
            static_cast< COMCORE::CDiscoveryEntry* >( entry.GetPointerAlways() )->GetAttributes().Set( "players", "4" );

            CORE::CDynamicBuffer beacon;
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::EncodeEntry( COM::CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE, *entry, beacon ) );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::IsBeacon( beacon.GetConstBufferPtr(), beacon.GetDataSize() ) );

            COM::CUdpDiscoveryCodec::TBeaconKind kind = COM::CUdpDiscoveryCodec::BEACONKIND_QUERY;
            COMCORE::CDiscoveryEntry decoded;
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::Decode( beacon.GetConstBufferPtr(), beacon.GetDataSize(), kind, decoded ) );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE == kind );
            ASSERT_TRUE( decoded.GetServiceType() == TestServiceType );
            ASSERT_TRUE( decoded.GetInstanceName() == "Game One" );
            ASSERT_TRUE( 45 == decoded.GetTtlInSecs() );
            CORE::CString instanceId;
            decoded.GetApplicationInstanceId( instanceId );
            ASSERT_TRUE( instanceId == "inst-1" );
            ASSERT_TRUE( 2 == decoded.GetEndpoints().size() );
            ASSERT_TRUE( decoded.GetEndpoints()[ 1 ] == "tcp://10.1.2.3:7778" );
            ASSERT_TRUE( COMCORE::CDiscoveryEntry::AreAttributesEqual( decoded.GetAttributes(), entry->GetAttributes() ) );
            ASSERT_TRUE( 2 == decoded.GetAttributes().GetValueVector( "players" ).size() );

            // A goodbye has TTL 0
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::EncodeEntry( COM::CUdpDiscoveryCodec::BEACONKIND_GOODBYE, *entry, beacon ) );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::Decode( beacon.GetConstBufferPtr(), beacon.GetDataSize(), kind, decoded ) );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::BEACONKIND_GOODBYE == kind );
            ASSERT_TRUE( 0 == decoded.GetTtlInSecs() );

            ASSERT_TRUE( COM::CUdpDiscoveryCodec::EncodeQuery( TestServiceType, beacon ) );
            COMCORE::CDiscoveryEntry query;
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::Decode( beacon.GetConstBufferPtr(), beacon.GetDataSize(), kind, query ) );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::BEACONKIND_QUERY == kind );
            ASSERT_TRUE( query.GetServiceType() == TestServiceType );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Damaged, foreign and oversized beacons are refused" )
        try
        {
            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestServiceType, "inst-1", "Game One", 45 );
            CORE::CDynamicBuffer beacon;
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::EncodeEntry( COM::CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE, *entry, beacon ) );

            COM::CUdpDiscoveryCodec::TBeaconKind kind = COM::CUdpDiscoveryCodec::BEACONKIND_QUERY;
            COMCORE::CDiscoveryEntry decoded;
            for ( UInt32 size=0; size<beacon.GetDataSize(); ++size )
                ASSERT_FALSE( COM::CUdpDiscoveryCodec::Decode( beacon.GetConstBufferPtr(), size, kind, decoded ) );

            const char junk[] = "GET / HTTP/1.1\r\n\r\n";
            ASSERT_FALSE( COM::CUdpDiscoveryCodec::IsBeacon( junk, sizeof( junk ) ) );
            ASSERT_FALSE( COM::CUdpDiscoveryCodec::Decode( junk, sizeof( junk ), kind, decoded ) );

            CORE::CDynamicBuffer badKind( beacon );
            badKind.SetUByte( 5, 9 );
            ASSERT_FALSE( COM::CUdpDiscoveryCodec::Decode( badKind.GetConstBufferPtr(), badKind.GetDataSize(), kind, decoded ) );

            char big[ 2001 ];
            memset( big, 'x', 2000 );
            big[ 2000 ] = '\0';
            static_cast< COMCORE::CDiscoveryEntry* >( entry.GetPointerAlways() )->GetAttributes().Set( "big", CORE::CString( big ) );
            ASSERT_FALSE( COM::CUdpDiscoveryCodec::EncodeEntry( COM::CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE, *entry, beacon ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two clients find each other through unicast targets on loopback, and the goodbye" )
        try
        {
            COM::CUdpBroadcastDiscoveryClient advertiser;
            advertiser.SetDiscoveryClientName( "advertiser" );
            advertiser.SetPort( UnicastTestPortA );
            advertiser.SetUseLimitedBroadcast( false );
            advertiser.SetUseDirectedBroadcasts( false );
            advertiser.AddExtraTarget( COMCORE::CIPv4Address( "127.0.0.1", UnicastTestPortB ) );

            COM::CUdpBroadcastDiscoveryClient browser;
            browser.SetDiscoveryClientName( "browser" );
            browser.SetPort( UnicastTestPortB );
            browser.SetUseLimitedBroadcast( false );
            browser.SetUseDirectedBroadcasts( false );
            browser.AddExtraTarget( COMCORE::CIPv4Address( "127.0.0.1", UnicastTestPortA ) );

            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &browser );

            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestServiceType, "inst-1", "Game One", 30 );
            ASSERT_TRUE( advertiser.Advertise( entry ) );
            PumpFor( 50 );

            // The browser asks, the advertiser answers
            ASSERT_TRUE( browser.Browse( TestServiceType ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.discovered, 1, 5000 ) );
            if ( !recorder.lastEntry.IsNULL() )
            {
                ASSERT_TRUE( recorder.lastEntry->GetInstanceName() == "Game One" );
                ASSERT_TRUE( GetAttribute( *recorder.lastEntry, "map" ) == "dust" );
                ASSERT_TRUE( 1 == recorder.lastEntry->GetEndpoints().size() );
                ASSERT_TRUE( recorder.lastEntry->GetEndpoints()[ 0 ] == "udp://127.0.0.1:7777" );
                COMCORE::CHostAddress host;
                recorder.lastEntry->GetHost( host );
                ASSERT_TRUE( host.GetFirstIPv4Address().AddressAsString() == "127.0.0.1" );
                ASSERT_TRUE( recorder.lastEntry->GetSourceClientName() == "browser" );
            }

            // The advertiser does not report its own entry and the browser sees it only once
            PumpFor( 200 );
            ASSERT_TRUE( 1 == recorder.discovered );

            ASSERT_TRUE( advertiser.Withdraw( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.lost, 1, 5000 ) );

            COMCORE::CIDiscovery::TIDiscoveryEntrySet entries;
            browser.GetDiscoveredEntries( TestServiceType, entries );
            ASSERT_TRUE( entries.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An advertiser announces again before the TTL runs out" )
        try
        {
            COM::CUdpBroadcastDiscoveryClient advertiser;
            advertiser.SetPort( UnicastTestPortA );
            advertiser.SetUseLimitedBroadcast( false );
            advertiser.SetUseDirectedBroadcasts( false );
            advertiser.AddExtraTarget( COMCORE::CIPv4Address( "127.0.0.1", UnicastTestPortB ) );

            COM::CUdpBroadcastDiscoveryClient browser;
            browser.SetPort( UnicastTestPortB );
            browser.SetUseLimitedBroadcast( false );
            browser.SetUseDirectedBroadcasts( false );
            browser.AddExtraTarget( COMCORE::CIPv4Address( "127.0.0.1", UnicastTestPortA ) );

            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &browser );

            ASSERT_TRUE( browser.Browse( TestServiceType ) );
            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestServiceType, "short", "Short TTL", 2 );
            ASSERT_TRUE( advertiser.Advertise( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.discovered, 1, 3000 ) );

            // Three times the TTL: still there thanks to the announcements every TTL/3
            PumpFor( 6000 );
            ASSERT_TRUE( 0 == recorder.lost );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two clients find each other with broadcasts" )
        GUCEF_TESTFW_SKIP_IF( !CanBroadcast( BroadcastTestPort ), "There is no route for a broadcast on this host" );
        try
        {
            COM::CUdpBroadcastDiscoveryClient advertiser;
            advertiser.SetPort( BroadcastTestPort );
            COM::CUdpBroadcastDiscoveryClient browser;
            browser.SetPort( BroadcastTestPort );

            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &browser );

            ASSERT_TRUE( advertiser.Advertise( MakeEntry( TestServiceType, "inst-b", "Broadcast Game", 30 ) ) );
            ASSERT_TRUE( browser.Browse( TestServiceType ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.discovered, 1, 5000 ) );
            if ( !recorder.lastEntry.IsNULL() )
            {
                ASSERT_TRUE( recorder.lastEntry->GetInstanceName() == "Broadcast Game" );
                ASSERT_FALSE( recorder.lastEntry->GetEndpoints().empty() || recorder.lastEntry->GetEndpoints()[ 0 ].HasSubstr( "0.0.0.0" ) >= 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two clients find each other with multicast" )
        bool multicastWorks = false;
        {
            COM::CUdpMulticastDiscoveryClient probe;
            probe.SetPort( MulticastTestPort );
            multicastWorks = probe.Browse( TestServiceType ) && probe.IsActive() && probe.GetJoinedInterfaceCount() > 0;
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The multicast group cannot be joined on this host" );
        try
        {
            COM::CUdpMulticastDiscoveryClient advertiser;
            advertiser.SetPort( MulticastTestPort );
            COM::CUdpMulticastDiscoveryClient browser;
            browser.SetPort( MulticastTestPort );
            ASSERT_TRUE( browser.GetGroup().AddressAsString() == COM::CUdpMulticastDiscoveryClient::DefaultGroup );
            ASSERT_TRUE( 1 == browser.GetMulticastTtl() );

            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &browser );

            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestServiceType, "inst-m", "Multicast Game", 30 );
            ASSERT_TRUE( browser.Browse( TestServiceType ) );
            ASSERT_TRUE( advertiser.Advertise( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.discovered, 1, 5000 ) );
            if ( !recorder.lastEntry.IsNULL() )
                ASSERT_TRUE( recorder.lastEntry->GetInstanceName() == "Multicast Game" );

            ASSERT_TRUE( advertiser.Withdraw( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.lost, 1, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A broadcast client that no longer browses or advertises holds no socket and no timer, and works again when it is used again" )
        GUCEF_TESTFW_SKIP_IF( GUCEF::TESTFW::CountOpenSockets() < 0, "The open sockets of this process cannot be counted on this platform" );
        try
        {
            COM::CUdpBroadcastDiscoveryClient client;
            client.SetPort( BroadcastTestPort );
            client.SetUseLimitedBroadcast( false );
            client.SetUseDirectedBroadcasts( false );
            client.AddExtraTarget( COMCORE::CIPv4Address( "127.0.0.1", UnicastTestPortB ) );
            ASSERT_FALSE( client.IsActive() );
            ASSERT_TRUE( ClientHoldsNothingWhenIdle( client, TestServiceType, &IsBroadcastActive ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A multicast client that no longer browses or advertises holds no socket, has left the group and holds no timer, and works again when it is used again" )
        GUCEF_TESTFW_SKIP_IF( GUCEF::TESTFW::CountOpenSockets() < 0, "The open sockets of this process cannot be counted on this platform" );
        bool multicastWorks = false;
        {
            COM::CUdpMulticastDiscoveryClient probe;
            probe.SetPort( MulticastTestPort );
            multicastWorks = probe.Browse( TestServiceType ) && probe.IsActive() && probe.GetJoinedInterfaceCount() > 0;
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The multicast group cannot be joined on this host" );
        try
        {
            COM::CUdpMulticastDiscoveryClient client;
            client.SetPort( MulticastTestPort );
            ASSERT_TRUE( ClientHoldsNothingWhenIdle( client, TestServiceType, &IsMulticastActive ) );
            ASSERT_TRUE( 0 == client.GetJoinedInterfaceCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The built in discovery clients are created by name from the config of the manager" )
        try
        {
            COMCORE::CDiscoveryClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryClientFactory();
            ASSERT_TRUE( factory.IsConstructible( "broadcast" ) );
            ASSERT_TRUE( factory.IsConstructible( "multicast" ) );
            ASSERT_TRUE( factory.IsConstructible( "mdns" ) );
            ASSERT_TRUE( factory.IsConstructible( "ssdp" ) );

            COMCORE::CDiscoveryManager& manager = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryManager();
            CORE::CDataNode managerNode( "DiscoveryManager" );
            CORE::CDataNode* clientNode = managerNode.AddChild( "DiscoveryClient" );
            clientNode->SetAttribute( "type", "multicast" );
            clientNode->SetAttribute( "name", "test-multicast" );
            clientNode->SetAttribute( "port", MulticastTestPort );
            clientNode->SetAttribute( "multicastTtl", 2 );
            ASSERT_TRUE( manager.LoadConfig( managerNode ) );

            COMCORE::CIDiscoveryClientPtr client = manager.GetDiscoveryClient( "test-multicast" );
            ASSERT_FALSE( client.IsNULL() );
            if ( !client.IsNULL() )
            {
                // the client says what it is, so no RTTI is needed to get at the class
                COM::CUdpMulticastDiscoveryClient* multicast = GUCEF_NULL;
                if ( client->GetClassTypeName() == "GUCEF::COM::CUdpMulticastDiscoveryClient" )
                    multicast = static_cast< COM::CUdpMulticastDiscoveryClient* >( client.GetPointerAlways() );
                ASSERT_TRUE( GUCEF_NULL != multicast );
                if ( GUCEF_NULL != multicast )
                {
                    ASSERT_TRUE( MulticastTestPort == multicast->GetPort() );
                    ASSERT_TRUE( 2 == multicast->GetMulticastTtl() );
                }
            }
            ASSERT_TRUE( manager.UnregisterDiscoveryClient( "test-multicast" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformMdnsDiscoveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "MdnsDiscovery" );

    GUCEF_TESTFW_TESTCASE( "Test: A DNS message with compressed names reads back the same" )
        try
        {
            CORE::CString serviceName = CORE::CString( TestServiceType ) + ".local";
            CORE::CString instanceName = COM::CDnsMessage::MakeName( "My Game.v2", serviceName );
            ASSERT_TRUE( instanceName == "My Game\\.v2._gucef-test._udp.local" );

            COM::CDnsMessage message;
            message.id = 0x1234;
            message.flags = COM::CDnsMessage::FlagResponse | COM::CDnsMessage::FlagAuthoritative;
            message.questions.push_back( COM::CDnsMessage::CQuestion( serviceName, COM::CDnsMessage::RECORDTYPE_PTR ) );
            message.answers.push_back( COM::CDnsMessage::CResourceRecord::MakePtr( serviceName, instanceName, 120 ) );
            message.additionals.push_back( COM::CDnsMessage::CResourceRecord::MakeSrv( instanceName, "host.local", 7777, 120 ) );
            CORE::CString::StringVector texts;
            texts.push_back( "map=dust" );
            texts.push_back( "flag" );
            message.additionals.push_back( COM::CDnsMessage::CResourceRecord::MakeTxt( instanceName, texts, 120 ) );
            message.additionals.push_back( COM::CDnsMessage::CResourceRecord::MakeA( "host.local", COMCORE::CIPv4Address::Loopback.GetAddress(), 120 ) );

            CORE::CDynamicBuffer wire;
            ASSERT_TRUE( message.Serialize( wire ) );

            // The repeated names are pointers: far less than the names written out in full
            ASSERT_TRUE( wire.GetDataSize() < 160 );

            COM::CDnsMessage read;
            ASSERT_TRUE( read.Deserialize( wire.GetConstBufferPtr(), wire.GetDataSize() ) );
            ASSERT_TRUE( 0x1234 == read.id );
            ASSERT_TRUE( read.IsResponse() );
            ASSERT_TRUE( 1 == read.questions.size() && read.questions[ 0 ].name == serviceName );
            ASSERT_TRUE( 1 == read.answers.size() );
            ASSERT_TRUE( read.answers[ 0 ].target == instanceName );
            ASSERT_TRUE( 3 == read.additionals.size() );
            if ( 3 == read.additionals.size() )
            {
                ASSERT_TRUE( 7777 == read.additionals[ 0 ].port );
                ASSERT_TRUE( read.additionals[ 0 ].target == "host.local" );
                ASSERT_TRUE( read.additionals[ 0 ].cacheFlush );
                ASSERT_TRUE( 2 == read.additionals[ 1 ].texts.size() );
                ASSERT_TRUE( COMCORE::CIPv4Address::Loopback.GetAddress() == read.additionals[ 2 ].ipv4AddressInNetworkByteOrder );
            }

            CORE::CString label, rest;
            ASSERT_TRUE( COM::CDnsMessage::SplitFirstLabel( read.answers[ 0 ].target, label, rest ) );
            ASSERT_TRUE( label == "My Game.v2" );
            ASSERT_TRUE( rest == serviceName );
            ASSERT_TRUE( COM::CDnsMessage::NamesAreEqual( "_GUCEF-test._UDP.local", serviceName ) );

            CORE::CValueList attributes;
            COM::CDnsMessage::TextsToAttributes( texts, attributes );
            ASSERT_TRUE( attributes.GetValueAlways( "map" ).AsString() == "dust" );
            ASSERT_TRUE( attributes.HasKey( "flag" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Damaged DNS messages are refused, compression loops included" )
        try
        {
            COM::CDnsMessage message;
            message.questions.push_back( COM::CDnsMessage::CQuestion( "a.b.local", COM::CDnsMessage::RECORDTYPE_A ) );
            message.answers.push_back( COM::CDnsMessage::CResourceRecord::MakeA( "a.b.local", 0x01020304, 10 ) );
            CORE::CDynamicBuffer wire;
            ASSERT_TRUE( message.Serialize( wire ) );

            COM::CDnsMessage read;
            for ( UInt32 size=0; size<wire.GetDataSize(); ++size )
                ASSERT_FALSE( read.Deserialize( wire.GetConstBufferPtr(), size ) );

            // A question whose name points at itself
            const UInt8 loop[] = { 0,0, 0,0, 0,1, 0,0, 0,0, 0,0, 0xC0,12, 0,1, 0,1 };
            ASSERT_FALSE( read.Deserialize( loop, sizeof( loop ) ) );

            // More records than the data can hold
            const UInt8 counts[] = { 0,0, 0x84,0, 0,0, 0xFF,0xFF, 0,0, 0,0 };
            ASSERT_FALSE( read.Deserialize( counts, sizeof( counts ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two mDNS clients find each other, resolve the instance and see the goodbye" )
        bool multicastWorks = false;
        {
            COM::CMdnsDiscoveryClient probe;
            probe.SetPort( MdnsTestPort );
            multicastWorks = probe.Browse( TestServiceType ) && probe.IsActive();
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The mDNS group cannot be joined on this host" );
        try
        {
            COM::CMdnsDiscoveryClient advertiser;
            advertiser.SetPort( MdnsTestPort );
            advertiser.SetHostName( "gucef-test-advertiser" );
            COM::CMdnsDiscoveryClient browser;
            browser.SetPort( MdnsTestPort );
            browser.SetHostName( "gucef-test-browser" );

            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &browser );

            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestServiceType, "inst-mdns", "mDNS Game.v2", 120 );
            ASSERT_TRUE( advertiser.Advertise( entry ) );
            ASSERT_TRUE( browser.Browse( TestServiceType ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.discovered, 1, 8000 ) );
            if ( !recorder.lastEntry.IsNULL() )
            {
                const COMCORE::CIDiscoveryEntry& found = *recorder.lastEntry;
                ASSERT_TRUE( found.GetServiceType() == TestServiceType );
                ASSERT_TRUE( found.GetInstanceName() == "mDNS Game.v2" );
                CORE::CString instanceId;
                found.GetApplicationInstanceId( instanceId );
                ASSERT_TRUE( instanceId == "inst-mdns" );
                ASSERT_TRUE( GetAttribute( found, "map" ) == "dust" );
                ASSERT_FALSE( found.GetAttributes().HasKey( COM::CMdnsDiscoveryClient::InstanceIdTextKey ) );
                ASSERT_FALSE( found.GetEndpoints().empty() );
                if ( !found.GetEndpoints().empty() )
                {
                    ASSERT_TRUE( found.GetEndpoints()[ 0 ].StartsWith( "udp://" ) );
                    ASSERT_TRUE( found.GetEndpoints()[ 0 ].EndsWith( ":7777" ) );
                }
                COMCORE::CHostAddress host;
                found.GetHost( host );
                ASSERT_TRUE( host.GetHostname() == "gucef-test-advertiser" );
            }

            ASSERT_TRUE( advertiser.Withdraw( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.lost, 1, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An mDNS client that no longer browses or advertises holds no socket and no timer, and works again when it is used again" )
        GUCEF_TESTFW_SKIP_IF( GUCEF::TESTFW::CountOpenSockets() < 0, "The open sockets of this process cannot be counted on this platform" );
        bool multicastWorks = false;
        {
            COM::CMdnsDiscoveryClient probe;
            probe.SetPort( MdnsTestPort );
            multicastWorks = probe.Browse( TestServiceType ) && probe.IsActive();
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The mDNS group cannot be joined on this host" );
        try
        {
            COM::CMdnsDiscoveryClient client;
            client.SetPort( MdnsTestPort );
            client.SetHostName( "gucef-test-idle" );
            ASSERT_TRUE( ClientHoldsNothingWhenIdle( client, TestServiceType, &IsMdnsActive ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The mDNS client shares port 5353 with the responder of the system and finds printers it announces" )
        bool opened = false;
        COM::CMdnsDiscoveryClient client;
        CDiscoveryEventRecorder recorder;
        recorder.SubscribeTo( &client );
        opened = client.Browse( "_ipp._tcp" ) && client.IsActive();
        GUCEF_TESTFW_SKIP_IF( !opened, "The mDNS port cannot be used on this host" );
        try
        {
            ASSERT_TRUE( 5353 == client.GetPort() );
            PumpUntilAtLeast( recorder.discovered, 1, 4000 );
            if ( recorder.discovered > 0 && !recorder.lastEntry.IsNULL() )
            {
                ASSERT_TRUE( recorder.lastEntry->GetServiceType() == "_ipp._tcp" );
                ASSERT_FALSE( recorder.lastEntry->GetInstanceName().IsNULLOrEmpty() );
                ASSERT_FALSE( recorder.lastEntry->GetEndpoints().empty() );
                if ( !recorder.lastEntry->GetEndpoints().empty() )
                    ASSERT_TRUE( recorder.lastEntry->GetEndpoints()[ 0 ].StartsWith( "tcp://" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformSsdpDiscoveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "SsdpDiscovery" );

    GUCEF_TESTFW_TESTCASE( "Test: An SSDP client that no longer browses or advertises holds no sockets and no timer, and works again when it is used again" )
        GUCEF_TESTFW_SKIP_IF( GUCEF::TESTFW::CountOpenSockets() < 0, "The open sockets of this process cannot be counted on this platform" );
        bool multicastWorks = false;
        {
            COM::CSsdpDiscoveryClient probe;
            probe.SetPort( SsdpTestPort );
            multicastWorks = probe.Browse( TestSsdpServiceType ) && probe.IsActive();
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The SSDP group cannot be joined on this host" );
        try
        {
            COM::CSsdpDiscoveryClient client;
            client.SetPort( SsdpTestPort );
            ASSERT_TRUE( ClientHoldsNothingWhenIdle( client, TestSsdpServiceType, &IsSsdpActive ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two SSDP clients find each other with M-SEARCH and NOTIFY, and see the byebye" )
        bool multicastWorks = false;
        {
            COM::CSsdpDiscoveryClient probe;
            probe.SetPort( SsdpTestPort );
            multicastWorks = probe.Browse( TestSsdpServiceType ) && probe.IsActive();
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The SSDP group cannot be joined on this host" );
        try
        {
            COM::CSsdpDiscoveryClient advertiser;
            advertiser.SetPort( SsdpTestPort );
            COM::CSsdpDiscoveryClient browser;
            browser.SetPort( SsdpTestPort );

            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &browser );

            COMCORE::CIDiscoveryEntryPtr entry = MakeEntry( TestSsdpServiceType, "inst-ssdp", "SSDP Game", 60 );
            ASSERT_TRUE( browser.Browse( TestSsdpServiceType ) );
            ASSERT_TRUE( advertiser.Advertise( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.discovered, 1, 5000 ) );
            if ( !recorder.lastEntry.IsNULL() )
            {
                const COMCORE::CIDiscoveryEntry& found = *recorder.lastEntry;
                ASSERT_TRUE( found.GetServiceType() == TestSsdpServiceType );
                ASSERT_TRUE( found.GetInstanceName() == "SSDP Game" );
                CORE::CString instanceId;
                found.GetApplicationInstanceId( instanceId );
                ASSERT_TRUE( instanceId == "inst-ssdp" );
                ASSERT_TRUE( 60 == found.GetTtlInSecs() );
                ASSERT_TRUE( GetAttribute( found, "map" ) == "dust" );
                ASSERT_TRUE( 1 == found.GetEndpoints().size() );
                if ( 1 == found.GetEndpoints().size() )
                    ASSERT_TRUE( found.GetEndpoints()[ 0 ].HasSubstr( "0.0.0.0" ) < 0 );
            }

            // A browse that starts later is answered on its M-SEARCH
            COM::CSsdpDiscoveryClient lateBrowser;
            lateBrowser.SetPort( SsdpTestPort );
            CDiscoveryEventRecorder lateRecorder;
            lateRecorder.SubscribeTo( &lateBrowser );
            ASSERT_TRUE( lateBrowser.Browse( TestSsdpServiceType ) );
            ASSERT_TRUE( PumpUntilAtLeast( lateRecorder.discovered, 1, 5000 ) );

            ASSERT_TRUE( advertiser.Withdraw( entry ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.lost, 1, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
