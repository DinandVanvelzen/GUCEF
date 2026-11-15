/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEADVERTISER_H
#include "gucefCOMCORE_CServiceAdvertiser.h"
#define GUCEF_COMCORE_CSERVICEADVERTISER_H
#endif /* GUCEF_COMCORE_CSERVICEADVERTISER_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

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

#ifndef GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#include "gucefCOM_CMdnsDiscoveryClient.h"
#define GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CMDNSDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#include "gucefCOM_CSsdpDiscoveryClient.h"
#define GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#endif /* GUCEF_COM_CSSDPDISCOVERYCLIENT_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestCode_Pump.h"

#include "TestServiceDiscoveryNetwork.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* NetServiceType = "_gucef-nettest._tcp";

/** Counts the candidate events of a finder */
class CCandidateCounter : public CORE::CObserver
{
    public:

    UInt32 found;
    UInt32 updated;
    UInt32 lost;
    COMCORE::CServiceCandidate last;

    CCandidateCounter( void ) : CORE::CObserver(), found( 0 ), updated( 0 ), lost( 0 ), last() {}
    virtual ~CCandidateCounter() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CCandidateCounter";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

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

/*-------------------------------------------------------------------------*/

/** A plain socket that plays an asker or an attacker: counts what it receives */
class CRawPeer : public CORE::CObserver
{
    public:

    COMCORE::CUDPSocket socket;
    UInt32 packets;
    UInt32 bytes;
    UInt32 largest;
    UInt32 announcements;

    CRawPeer( void )
        : CORE::CObserver()
        , socket( false )
        , packets( 0 )
        , bytes( 0 )
        , largest( 0 )
        , announcements( 0 )
    {GUCEF_TRACE;

        socket.SetNrOfReceiveBuffers( 16, 2048 );
        SubscribeTo( &socket, COMCORE::CUDPSocket::UDPPacketsRecievedEvent );
    }

    virtual ~CRawPeer()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
        socket.Close();
    }

    bool Open( void ) { return socket.Open( (UInt16) 0 ); }

    bool Send( UInt16 port, const CORE::CDynamicBuffer& data )
    {GUCEF_TRACE;

        return (Int32) data.GetDataSize() == socket.SendPacketTo( COMCORE::CIPv4Address( "127.0.0.1", port ), data.GetConstBufferPtr(), (UInt16) data.GetDataSize() );
    }

    void Reset( void ) { packets = 0; bytes = 0; largest = 0; announcements = 0; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CRawPeer";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent != eventId || GUCEF_NULL == eventData )
            return;
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
        for ( UInt32 i=0; i<data.packetsReceived; ++i )
        {
            const CORE::CDynamicBuffer& packet = data.packets[ i ].dataBuffer.GetData();
            ++packets;
            bytes += packet.GetDataSize();
            if ( packet.GetDataSize() > largest )
                largest = packet.GetDataSize();
            COM::CUdpDiscoveryCodec::TBeaconKind kind = COM::CUdpDiscoveryCodec::BEACONKIND_QUERY;
            COMCORE::CDiscoveryEntry entry;
            if ( COM::CUdpDiscoveryCodec::Decode( packet.GetConstBufferPtr(), packet.GetDataSize(), kind, entry ) && COM::CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE == kind )
                ++announcements;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** A "DiscoveryClient" node of the broadcast type that only talks unicast on loopback */
void
AddLoopbackClient( CORE::CDataNode& clientConfig, UInt16 port, UInt16 peerPort, const char* name )
{GUCEF_TRACE;

    CORE::CDataNode* client = clientConfig.AddChild( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
    client->SetAttribute( "type", "broadcast" );
    client->SetAttribute( "name", name );
    client->SetAttribute( "port", CORE::ToString( port ) );
    client->SetAttribute( "useLimitedBroadcast", "false" );
    client->SetAttribute( "useDirectedBroadcasts", "false" );
    if ( 0 != peerPort )
        client->SetAttribute( "extraTargets", "127.0.0.1:" + CORE::ToString( peerPort ) );
}

/*-------------------------------------------------------------------------*/

COMCORE::CServiceAnnouncement
MakeServiceAnnouncement( const CORE::CString& instanceId, UInt32 padBytes )
{GUCEF_TRACE;

    COMCORE::CServiceAnnouncement announcement;
    announcement.serviceType = NetServiceType;
    announcement.instanceId = instanceId;
    announcement.displayName = "Net test " + instanceId;
    announcement.endpoints.push_back( COMCORE::CServiceAnnouncement::MakeEndpoint( "tcp", CORE::CString::Empty, 47400 ) );
    announcement.protocolVersion = "1";
    announcement.keyFingerprint = "ABCD-EFGH-JKMN-PQRS-TVWX";
    if ( padBytes > 0 )
    {
        CORE::CString filler;
        for ( UInt32 i=0; i<padBytes; ++i )
            filler += 'x';
        announcement.properties.Set( "filler", filler );
    }
    return announcement;
}

/*-------------------------------------------------------------------------*/

COMCORE::CIDiscoveryEntryPtr
MakeServiceEntry( const CORE::CString& instanceId, UInt32 padBytes )
{GUCEF_TRACE;

    COMCORE::CDiscoveryEntry* entry = GUCEF_NEW COMCORE::CDiscoveryEntry();
    MakeServiceAnnouncement( instanceId, padBytes ).ToEntry( *entry );
    return COMCORE::CIDiscoveryEntryPtr( entry );
}

/*-------------------------------------------------------------------------*/

CORE::CDynamicBuffer
MakeQuery( const CORE::CString& serviceType, UInt32 padTo )
{GUCEF_TRACE;

    CORE::CDynamicBuffer query( 1500, true );
    COM::CUdpDiscoveryCodec::EncodeQuery( serviceType, query, padTo );
    return query;
}

/*-------------------------------------------------------------------------*/

/** A broadcast client that only hears unicast on loopback, with the settings the test asks for */
void
SetUpLoopbackAdvertiser( COM::CUdpBroadcastDiscoveryClient& client, UInt16 port )
{GUCEF_TRACE;

    client.SetPort( port );
    client.SetUseLimitedBroadcast( false );
    client.SetUseDirectedBroadcasts( false );
}

/*-------------------------------------------------------------------------*/

bool
CanBroadcastHere( UInt16 port )
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

/*-------------------------------------------------------------------------*/

/** Advertises with one client type and finds with another instance of it, both through the generic classes */
bool
FindThroughClientType( const CORE::CDataNode& clientNode, const CORE::CString& serviceType, UInt32 timeoutMs )
{GUCEF_TRACE;

    COMCORE::CServiceAdvertiserSettings advertiserSettings;
    advertiserSettings.serviceType = serviceType;
    advertiserSettings.clientConfig.AddChild( clientNode );
    COMCORE::CServiceAdvertiser advertiser( advertiserSettings );

    COMCORE::CServiceFinderSettings finderSettings;
    finderSettings.serviceType = serviceType;
    finderSettings.clientConfig.AddChild( clientNode );
    COMCORE::CServiceFinder finder( finderSettings );
    CCandidateCounter counter;
    counter.SubscribeTo( &finder );

    COMCORE::CServiceAnnouncement announcement = MakeServiceAnnouncement( "per-type", 0 );
    announcement.serviceType = serviceType;
    if ( !advertiser.Advertise( announcement ) || !finder.Start() )
        return false;
    if ( !PumpUntilAtLeast( counter.found, 1, timeoutMs ) )
        return false;
    bool fieldsOk = "per-type" == counter.last.announcement.instanceId && 47400 == counter.last.announcement.GetPort( "" );
    advertiser.Withdraw();
    bool lost = PumpUntilAtLeast( counter.lost, 1, timeoutMs );
    finder.Stop();
    return fieldsOk && lost;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformServiceDiscoveryNetworkTests( void )
{GUCEF_TRACE;

    COM::CComGlobal::Instance();
    GUCEF_TESTFW_SUITE_SCOPE( "ServiceDiscoveryNetwork" );

    GUCEF_TESTFW_TESTCASE( "Test: A finder finds an advertised service on loopback, sees the update and the goodbye" )
        try
        {
            UInt16 advertiserPort = TESTFW::FindFreeUdpPort( 47841 );
            UInt16 finderPort = TESTFW::FindFreeUdpPort( 47842 );

            COMCORE::CServiceAdvertiserSettings advertiserSettings;
            advertiserSettings.serviceType = NetServiceType;
            AddLoopbackClient( advertiserSettings.clientConfig, advertiserPort, finderPort, "advertiser" );
            COMCORE::CServiceAdvertiser advertiser( advertiserSettings );

            COMCORE::CServiceFinderSettings finderSettings;
            finderSettings.serviceType = NetServiceType;
            AddLoopbackClient( finderSettings.clientConfig, finderPort, advertiserPort, "finder" );
            COMCORE::CServiceFinder finder( finderSettings );
            CCandidateCounter counter;
            counter.SubscribeTo( &finder );

            ASSERT_TRUE( advertiser.Advertise( MakeServiceAnnouncement( "svc-net", 0 ) ) );
            PumpFor( 20 );
            ASSERT_TRUE( finder.Start() );
            ASSERT_TRUE( PumpUntilAtLeast( counter.found, 1, 5000 ) );
            ASSERT_TRUE( "svc-net" == counter.last.announcement.instanceId );
            ASSERT_TRUE( "tcp://127.0.0.1:47400" == counter.last.GetFirstEndpoint() );
            ASSERT_TRUE( "ABCD-EFGH-JKMN-PQRS-TVWX" == counter.last.announcement.keyFingerprint );
            ASSERT_TRUE( "1" == counter.last.announcement.protocolVersion );
            ASSERT_TRUE( "127.0.0.1" == counter.last.host.GetFirstAddressAsString() );

            COMCORE::CServiceAnnouncement changed = MakeServiceAnnouncement( "svc-net", 0 );
            changed.displayName = "Renamed";
            ASSERT_TRUE( advertiser.Update( changed ) );
            ASSERT_TRUE( PumpUntilAtLeast( counter.updated, 1, 5000 ) );
            ASSERT_TRUE( "Renamed" == counter.last.announcement.displayName );

            ASSERT_TRUE( advertiser.Withdraw() );
            ASSERT_TRUE( PumpUntilAtLeast( counter.lost, 1, 5000 ) );
            ASSERT_TRUE( 0 == finder.GetCandidateCount() );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The answer to a query goes by unicast to the query socket of the asker, the only way it is heard here" )
        try
        {
            UInt16 advertiserPort = TESTFW::FindFreeUdpPort( 47843 );
            UInt16 finderPort = TESTFW::FindFreeUdpPort( 47844 );

            // the advertiser announces to nobody: only an answer can reach the finder
            COMCORE::CServiceAdvertiserSettings advertiserSettings;
            advertiserSettings.serviceType = NetServiceType;
            AddLoopbackClient( advertiserSettings.clientConfig, advertiserPort, 0, "silent-advertiser" );
            COMCORE::CServiceAdvertiser advertiser( advertiserSettings );
            ASSERT_TRUE( advertiser.Advertise( MakeServiceAnnouncement( "svc-unicast", 0 ) ) );
            COMCORE::CIDiscoveryClientPtr advertiserClientPtr = advertiser.GetDiscoveryClients()[ 0 ];
            COM::CUdpDiscoveryClientBase* advertiserClient = static_cast< COM::CUdpDiscoveryClientBase* >( advertiserClientPtr.GetPointerAlways() );

            COMCORE::CServiceFinderSettings finderSettings;
            finderSettings.serviceType = NetServiceType;
            AddLoopbackClient( finderSettings.clientConfig, finderPort, advertiserPort, "asker" );
            COMCORE::CServiceFinder finder( finderSettings );
            CCandidateCounter counter;
            counter.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );
            COMCORE::CIDiscoveryClientPtr finderClientPtr = finder.GetDiscoveryClients()[ 0 ];
            COM::CUdpDiscoveryClientBase* finderClient = static_cast< COM::CUdpDiscoveryClientBase* >( finderClientPtr.GetPointerAlways() );
            ASSERT_TRUE( finderClient->IsQuerySocketActive() && 0 != finderClient->GetQuerySocketPort() && finderPort != finderClient->GetQuerySocketPort() );

            ASSERT_TRUE( PumpUntilAtLeast( counter.found, 1, 5000 ) );
            ASSERT_TRUE( "svc-unicast" == counter.last.announcement.instanceId );
            ASSERT_TRUE( 1 == advertiserClient->GetAnswerLimiter().GetStats().admitted );

            // a refresh asks again and is answered again
            ASSERT_TRUE( finder.Refresh() );
            PumpFor( 100 );
            ASSERT_TRUE( 2 == advertiserClient->GetAnswerLimiter().GetStats().admitted );
            ASSERT_TRUE( 1 == counter.found && 1 == finder.GetCandidateCount() );

            // no browse: no query socket
            finder.Stop();
            ASSERT_FALSE( finderClient->IsQuerySocketActive() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A candidate whose announcements stop is lost when its TTL runs out" )
        try
        {
            UInt16 finderPort = TESTFW::FindFreeUdpPort( 47845 );
            COMCORE::CServiceFinderSettings finderSettings;
            finderSettings.serviceType = NetServiceType;
            // a browse needs somewhere to send its query: a port where nobody answers
            AddLoopbackClient( finderSettings.clientConfig, finderPort, TESTFW::FindFreeUdpPort( 47855 ), "finder" );
            COMCORE::CServiceFinder finder( finderSettings );
            CCandidateCounter counter;
            counter.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );

            // one announcement with a TTL of 3 seconds, and then silence
            CRawPeer peer;
            ASSERT_TRUE( peer.Open() );
            COMCORE::CDiscoveryEntry entry;
            COMCORE::CServiceAnnouncement shortLived = MakeServiceAnnouncement( "svc-ttl", 0 );
            shortLived.ttlInSecs = 3;
            shortLived.ToEntry( entry );
            CORE::CDynamicBuffer beacon( 1500, true );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::EncodeEntry( COM::CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE, entry, beacon ) );
            ASSERT_TRUE( peer.Send( finderPort, beacon ) );
            ASSERT_TRUE( PumpUntilAtLeast( counter.found, 1, 3000 ) );
            PumpFor( 1500 );
            ASSERT_TRUE( 0 == counter.lost );
            ASSERT_TRUE( PumpUntilAtLeast( counter.lost, 1, 5000 ) );
            ASSERT_TRUE( 0 == finder.GetCandidateCount() );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Malformed, oversized and foreign queries, and a query for every type when that is off, get no answer" )
        try
        {
            UInt16 advertiserPort = TESTFW::FindFreeUdpPort( 47846 );
            COM::CUdpBroadcastDiscoveryClient advertiser;
            SetUpLoopbackAdvertiser( advertiser, advertiserPort );
            advertiser.SetAnswerWildcardQueries( false );
            ASSERT_TRUE( advertiser.Advertise( MakeServiceEntry( "svc-guarded", 0 ) ) );

            CRawPeer peer;
            ASSERT_TRUE( peer.Open() );

            CORE::CDynamicBuffer garbage( 64, true );
            garbage.CopyFrom( 12, "GDSC\x01\x01\xFF\xFFjunk" );
            ASSERT_TRUE( peer.Send( advertiserPort, garbage ) );

            CORE::CDynamicBuffer valid = MakeQuery( NetServiceType, 1400 );
            CORE::CDynamicBuffer truncated( 64, true );
            truncated.CopyFrom( 9, valid.GetConstBufferPtr() );
            ASSERT_TRUE( peer.Send( advertiserPort, truncated ) );

            CORE::CDynamicBuffer oversized = MakeQuery( NetServiceType, 0 );
            oversized.SetDataSize( 1500 );
            ASSERT_TRUE( peer.Send( advertiserPort, oversized ) );

            ASSERT_TRUE( peer.Send( advertiserPort, MakeQuery( "_gucef-other._tcp", 1400 ) ) );
            ASSERT_TRUE( peer.Send( advertiserPort, MakeQuery( COMCORE::CIDiscoveryClient::AnyServiceType, 1400 ) ) );

            CORE::CDynamicBuffer notABeacon( 64, true );
            notABeacon.CopyFrom( 5, "hello" );
            ASSERT_TRUE( peer.Send( advertiserPort, notABeacon ) );

            PumpFor( 300 );
            ASSERT_TRUE( 0 == peer.packets );
            ASSERT_TRUE( 0 == advertiser.GetAnswerLimiter().GetStats().admitted );

            // and the right query is answered, as the control
            ASSERT_TRUE( peer.Send( advertiserPort, valid ) );
            ASSERT_TRUE( PumpUntilAtLeast( peer.announcements, 1, 3000 ) );
            ASSERT_TRUE( 1 == peer.packets );
            advertiser.StopAll();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A source that asks too often is answered only within its burst" )
        try
        {
            UInt16 advertiserPort = TESTFW::FindFreeUdpPort( 47847 );
            COM::CUdpBroadcastDiscoveryClient advertiser;
            SetUpLoopbackAdvertiser( advertiser, advertiserPort );
            CORE::CDataNode limits;
            limits.SetAttribute( "answerBurstPerSource", "3" );
            limits.SetAttribute( "answersPerSourcePerMinute", "1" );
            ASSERT_TRUE( advertiser.LoadConfig( limits ) );
            ASSERT_TRUE( advertiser.Advertise( MakeServiceEntry( "svc-limited", 0 ) ) );

            CRawPeer peer;
            ASSERT_TRUE( peer.Open() );
            CORE::CDynamicBuffer query = MakeQuery( NetServiceType, 1400 );
            for ( UInt32 i=0; i<10; ++i )
            {
                ASSERT_TRUE( peer.Send( advertiserPort, query ) );
                PumpFor( 5 );
            }
            PumpFor( 300 );
            ASSERT_TRUE( 3 == peer.announcements );
            COMCORE::CDiscoveryAnswerLimiter::TStats stats = advertiser.GetAnswerLimiter().GetStats();
            ASSERT_TRUE( 3 == stats.admitted && 7 == stats.refusedSourceRate );
            advertiser.StopAll();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The answers to a query never hold more bytes than the query, unless the asker is a configured peer" )
        try
        {
            UInt16 advertiserPort = TESTFW::FindFreeUdpPort( 47848 );
            COM::CUdpBroadcastDiscoveryClient advertiser;
            SetUpLoopbackAdvertiser( advertiser, advertiserPort );
            CORE::CDataNode limits;
            limits.SetAttribute( "answerBurstPerSource", "100" );
            ASSERT_TRUE( advertiser.LoadConfig( limits ) );
            // three entries of about 400 bytes each
            ASSERT_TRUE( advertiser.Advertise( MakeServiceEntry( "svc-a", 300 ) ) );
            ASSERT_TRUE( advertiser.Advertise( MakeServiceEntry( "svc-b", 300 ) ) );
            ASSERT_TRUE( advertiser.Advertise( MakeServiceEntry( "svc-c", 300 ) ) );

            CRawPeer peer;
            ASSERT_TRUE( peer.Open() );

            // a small query: nothing fits, nothing is sent
            CORE::CDynamicBuffer small = MakeQuery( NetServiceType, 0 );
            ASSERT_TRUE( small.GetDataSize() < 60 );
            ASSERT_TRUE( peer.Send( advertiserPort, small ) );
            PumpFor( 300 );
            ASSERT_TRUE( 0 == peer.packets );
            ASSERT_TRUE( 3 == advertiser.GetAnswerLimiter().GetStats().answersWithheld );

            // a query of 1000 bytes: two answers fit, the third waits for the announcement
            peer.Reset();
            ASSERT_TRUE( peer.Send( advertiserPort, MakeQuery( NetServiceType, 1000 ) ) );
            ASSERT_TRUE( PumpUntilAtLeast( peer.announcements, 2, 3000 ) );
            PumpFor( 200 );
            ASSERT_TRUE( 2 == peer.announcements && peer.bytes <= 1000 && peer.largest > 300 );

            // a full query of 1400 bytes: all three fit
            peer.Reset();
            ASSERT_TRUE( peer.Send( advertiserPort, MakeQuery( NetServiceType, 1400 ) ) );
            ASSERT_TRUE( PumpUntilAtLeast( peer.announcements, 3, 3000 ) );
            PumpFor( 200 );
            ASSERT_TRUE( 3 == peer.announcements && peer.bytes <= 1400 );

            // a peer the configuration names is verified: a small query gets everything
            advertiser.AddExtraTarget( COMCORE::CIPv4Address( "127.0.0.1", TESTFW::FindFreeUdpPort( 47849 ) ) );
            PumpFor( 50 );
            peer.Reset();
            ASSERT_TRUE( peer.Send( advertiserPort, small ) );
            ASSERT_TRUE( PumpUntilAtLeast( peer.announcements, 3, 3000 ) );
            ASSERT_TRUE( peer.bytes > small.GetDataSize() );
            advertiser.StopAll();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The new settings of a UDP discovery client survive a data tree" )
        try
        {
            CORE::CDataNode config;
            config.SetAttribute( "port", "47999" );
            config.SetAttribute( "queryPaddingBytes", "900" );
            config.SetAttribute( "answerWildcardQueries", "false" );
            config.SetAttribute( "answerBurstPerSource", "2" );
            config.SetAttribute( "answersPerSecond", "11" );
            config.SetAttribute( "amplificationFactor", "2" );
            COM::CUdpMulticastDiscoveryClient client;
            ASSERT_TRUE( client.LoadConfig( config ) );
            ASSERT_TRUE( 900 == client.GetQueryPaddingBytes() && !client.GetAnswerWildcardQueries() );
            ASSERT_TRUE( 2 == client.GetAnswerLimiter().GetAnswerBurstPerSource() && 11 == client.GetAnswerLimiter().GetAnswersPerSecond() );

            CORE::CDataNode saved;
            ASSERT_TRUE( client.SaveConfig( saved ) );
            COM::CUdpMulticastDiscoveryClient again;
            ASSERT_TRUE( again.LoadConfig( saved ) );
            ASSERT_TRUE( 47999 == again.GetPort() && 900 == again.GetQueryPaddingBytes() && !again.GetAnswerWildcardQueries() );
            ASSERT_TRUE( 2 == again.GetAnswerLimiter().GetAnswerBurstPerSource() && 2 == again.GetAnswerLimiter().GetAmplificationFactor() );

            // padding beyond a datagram is cut to the largest beacon
            config.SetAttribute( "queryPaddingBytes", "9000" );
            ASSERT_TRUE( again.LoadConfig( config ) );
            ASSERT_TRUE( COM::CUdpDiscoveryCodec::MaxBeaconSizeInBytes == again.GetQueryPaddingBytes() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Advertise and find with real broadcasts" )
        UInt16 port = TESTFW::FindFreeUdpPort( 47850 );
        GUCEF_TESTFW_SKIP_IF( !CanBroadcastHere( port ), "There is no route for a broadcast on this host" );
        try
        {
            CORE::CDataNode client( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
            client.SetAttribute( "type", "broadcast" );
            client.SetAttribute( "port", CORE::ToString( port ) );
            ASSERT_TRUE( FindThroughClientType( client, NetServiceType, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Advertise and find with multicast" )
        UInt16 port = TESTFW::FindFreeUdpPort( 47851 );
        bool multicastWorks = false;
        {
            COM::CUdpMulticastDiscoveryClient probe;
            probe.SetPort( port );
            multicastWorks = probe.Browse( NetServiceType ) && probe.IsActive() && probe.GetJoinedInterfaceCount() > 0;
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The multicast group cannot be joined on this host" );
        try
        {
            CORE::CDataNode client( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
            client.SetAttribute( "type", "multicast" );
            client.SetAttribute( "port", CORE::ToString( port ) );
            ASSERT_TRUE( FindThroughClientType( client, NetServiceType, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Advertise and find with mDNS" )
        UInt16 port = TESTFW::FindFreeUdpPort( 47852 );
        bool multicastWorks = false;
        {
            COM::CMdnsDiscoveryClient probe;
            probe.SetPort( port );
            multicastWorks = probe.Browse( NetServiceType ) && probe.IsActive();
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The mDNS group cannot be joined on this host" );
        try
        {
            CORE::CDataNode client( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
            client.SetAttribute( "type", "mdns" );
            client.SetAttribute( "port", CORE::ToString( port ) );
            client.SetAttribute( "hostName", "gucef-service-test" );
            ASSERT_TRUE( FindThroughClientType( client, NetServiceType, 8000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Advertise and find with SSDP" )
        UInt16 port = TESTFW::FindFreeUdpPort( 47853 );
        bool multicastWorks = false;
        {
            COM::CSsdpDiscoveryClient probe;
            probe.SetPort( port );
            multicastWorks = probe.Browse( NetServiceType ) && probe.IsActive();
        }
        GUCEF_TESTFW_SKIP_IF( !multicastWorks, "The SSDP group cannot be joined on this host" );
        try
        {
            CORE::CDataNode client( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
            client.SetAttribute( "type", "ssdp" );
            client.SetAttribute( "port", CORE::ToString( port ) );
            ASSERT_TRUE( FindThroughClientType( client, NetServiceType, 5000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
