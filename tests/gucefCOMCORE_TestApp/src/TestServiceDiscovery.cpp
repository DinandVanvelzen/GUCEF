/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the communication module
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#include "gucefCOMCORE_CDiscoveryClientBase.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#include "gucefCOMCORE_CDiscoveryClientFactory.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYANSWERLIMITER_H
#include "gucefCOMCORE_CDiscoveryAnswerLimiter.h"
#define GUCEF_COMCORE_CDISCOVERYANSWERLIMITER_H
#endif /* GUCEF_COMCORE_CDISCOVERYANSWERLIMITER_H ? */

#ifndef GUCEF_COMCORE_CSERVICEADVERTISER_H
#include "gucefCOMCORE_CServiceAdvertiser.h"
#define GUCEF_COMCORE_CSERVICEADVERTISER_H
#endif /* GUCEF_COMCORE_CSERVICEADVERTISER_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_UNIQUE_H
#include "gucef_test_unique.h"
#define GUCEF_TEST_UNIQUE_H
#endif /* GUCEF_TEST_UNIQUE_H ? */

#include "TestCode_Pump.h"

#include "TestServiceDiscovery.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::COMCORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* BusServiceType = "_gucef-bustest._tcp";

class CTestBusDiscoveryClient;
typedef GUCEF::set< CTestBusDiscoveryClient* > TBusClientSet;

/** Every bus client that exists; the clients look at each other through it */
TBusClientSet&
GetBus( void )
{
    static TBusClientSet bus;
    return bus;
}

MT::CMutex&
GetBusLock( void )
{
    static MT::CMutex lock;
    return lock;
}

/**
 *  A discovery client without a network: a browse hears what the other bus clients advertise (the query and its answers
 *  in one step, on the thread of the browser), an advertisement or a goodbye reaches the browsing bus clients that run
 *  on the same pulse generator
 */
class CTestBusDiscoveryClient : public CDiscoveryClientBase
{
    public:

    static const CORE::CString TypeName;

    CTestBusDiscoveryClient( void )
        : CDiscoveryClientBase( TypeName )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( GetBusLock() );
        GetBus().insert( this );
    }

    virtual ~CTestBusDiscoveryClient()
    {GUCEF_TRACE;

        StopAll();
        MT::CScopeMutex lock( GetBusLock() );
        GetBus().erase( this );
    }

    bool Hear( const CIDiscoveryEntry& entry )
    {GUCEF_TRACE;

        return ProcessHeardEntry( entry );
    }

    bool HearGoodbye( const CORE::CString& serviceType, const CORE::CString& instanceId )
    {GUCEF_TRACE;

        return ProcessGoodbye( serviceType, instanceId );
    }

    void GetOwnEntries( TIDiscoveryEntryPtrVector& entries ) const
    {GUCEF_TRACE;

        GetAdvertisedEntries( entries );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CTestBusDiscoveryClient";
        return classTypeName;
    }

    protected:

    static CDiscoveryEntry AsHeard( const CIDiscoveryEntry& entry )
    {GUCEF_TRACE;

        CDiscoveryEntry heard( entry );
        heard.SetHost( CHostAddress( CIPv4Address( "127.0.0.1", 0 ), "127.0.0.1" ) );
        CORE::CString::StringVector& endpoints = heard.GetEndpoints();
        for ( size_t i=0; i<endpoints.size(); ++i )
            endpoints[ i ] = endpoints[ i ].ReplaceSubstr( "0.0.0.0", "127.0.0.1" );
        return heard;
    }

    virtual bool OnBrowseStarted( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        TIDiscoveryEntryPtrVector answers;
        {
            MT::CScopeMutex lock( GetBusLock() );
            TBusClientSet::iterator i = GetBus().begin();
            while ( i != GetBus().end() )
            {
                if ( (*i) != this )
                    (*i)->GetOwnEntries( answers );
                ++i;
            }
        }
        for ( size_t n=0; n<answers.size(); ++n )
        {
            if ( serviceType == AnyServiceType || answers[ n ]->GetServiceType() == serviceType )
                if ( !ProcessHeardEntry( AsHeard( *answers[ n ] ) ) )
                    return true;
        }
        return true;
    }

    void GetPeersOnMyPulse( TBusClientSet& peers )
    {GUCEF_TRACE;

        MT::CScopeMutex lock( GetBusLock() );
        TBusClientSet::iterator i = GetBus().begin();
        while ( i != GetBus().end() )
        {
            if ( (*i) != this && (*i)->GetPulseGenerator() == GetPulseGenerator() )
                peers.insert( *i );
            ++i;
        }
    }

    virtual bool OnAdvertised( const CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        TBusClientSet peers;
        GetPeersOnMyPulse( peers );
        TBusClientSet::iterator i = peers.begin();
        while ( i != peers.end() )
        {
            (*i)->Hear( AsHeard( *entry ) );
            ++i;
        }
        return true;
    }

    virtual bool OnWithdrawn( const CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        CORE::CString instanceId;
        entry->GetApplicationInstanceId( instanceId );
        TBusClientSet peers;
        GetPeersOnMyPulse( peers );
        TBusClientSet::iterator i = peers.begin();
        while ( i != peers.end() )
        {
            (*i)->HearGoodbye( entry->GetServiceType(), instanceId );
            ++i;
        }
        return true;
    }
};

const CORE::CString CTestBusDiscoveryClient::TypeName = "test-bus";

typedef CORE::CTFactory< CIDiscoveryClient, CTestBusDiscoveryClient, MT::CMutex > TTestBusDiscoveryClientFactory;
TTestBusDiscoveryClientFactory g_testBusDiscoveryClientFactory;

/*-------------------------------------------------------------------------*/

/** Registers the bus client type for the life of the object */
class CBusTypeScope
{
    public:

    CBusTypeScope( void )
    {GUCEF_TRACE;

        CComCoreGlobal::Instance()->GetDiscoveryClientFactory().RegisterConcreteFactory( CTestBusDiscoveryClient::TypeName, &g_testBusDiscoveryClientFactory );
    }

    ~CBusTypeScope()
    {GUCEF_TRACE;

        CComCoreGlobal::Instance()->GetDiscoveryClientFactory().UnregisterConcreteFactory( CTestBusDiscoveryClient::TypeName );
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the candidate events of a finder and keeps the candidate of the last one */
class CCandidateRecorder : public CORE::CObserver
{
    public:

    UInt32 found;
    UInt32 updated;
    UInt32 lost;
    CServiceCandidate last;

    CCandidateRecorder( void )
        : CORE::CObserver()
        , found( 0 )
        , updated( 0 )
        , lost( 0 )
        , last()
    {GUCEF_TRACE;

    }

    virtual ~CCandidateRecorder()
    {GUCEF_TRACE;

        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CCandidateRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( CServiceFinder::CandidateFoundEvent == eventId )
            ++found;
        else if ( CServiceFinder::CandidateUpdatedEvent == eventId )
            ++updated;
        else if ( CServiceFinder::CandidateLostEvent == eventId )
            ++lost;
        else
            return;
        if ( GUCEF_NULL != eventData )
            last = static_cast< TServiceCandidateEventData* >( eventData )->GetData();
    }
};

/*-------------------------------------------------------------------------*/

CServiceAnnouncement
MakeAnnouncement( const CORE::CString& instanceId, UInt16 port )
{GUCEF_TRACE;

    CServiceAnnouncement announcement;
    announcement.serviceType = BusServiceType;
    announcement.instanceId = instanceId;
    announcement.displayName = "Test service " + instanceId;
    announcement.endpoints.push_back( CServiceAnnouncement::MakeEndpoint( "tcp", CORE::CString::Empty, port ) );
    announcement.endpoints.push_back( "tcp://worker.lan:" + CORE::ToString( port ) );
    announcement.protocolVersion = "1.2";
    announcement.keyFingerprint = "ABCD-EFGH-JKMN-PQRS-TVWX";
    announcement.properties.Set( "pairingPort", "47401" );
    return announcement;
}

/*-------------------------------------------------------------------------*/

CServiceFinderSettings
MakeBusFinderSettings( void )
{GUCEF_TRACE;

    CServiceFinderSettings settings;
    settings.serviceType = BusServiceType;
    settings.SetClientTypes( CTestBusDiscoveryClient::TypeName );
    return settings;
}

/*-------------------------------------------------------------------------*/

CServiceAdvertiserSettings
MakeBusAdvertiserSettings( void )
{GUCEF_TRACE;

    CServiceAdvertiserSettings settings;
    settings.serviceType = BusServiceType;
    settings.SetClientTypes( CTestBusDiscoveryClient::TypeName );
    return settings;
}

/*-------------------------------------------------------------------------*/

/** Pulses the given pulse generator on this thread for the given time */
void
PumpOwn( const CORE::PulseGeneratorPtr& pulseGenerator, UInt32 durationInMs )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulse( pulseGenerator );
    for ( UInt32 waited=0; waited<durationInMs; ++waited )
    {
        pulse->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
}

bool
PumpOwnUntilAtLeast( const CORE::PulseGeneratorPtr& pulseGenerator, const UInt32& counter, UInt32 target, UInt32 timeoutInMs )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulse( pulseGenerator );
    for ( UInt32 waited=0; waited<timeoutInMs && counter < target; ++waited )
    {
        pulse->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return counter >= target;
}

/*-------------------------------------------------------------------------*/

/** Counts the packet events of a socket */
class CPacketCounter : public CORE::CObserver
{
    public:

    UInt32 packets;

    CPacketCounter( void ) : CORE::CObserver(), packets( 0 ) {}
    virtual ~CPacketCounter() { UnsubscribeAllFromObserver(); }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CPacketCounter";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        if ( CUDPSocket::UDPPacketsRecievedEvent == eventId && GUCEF_NULL != eventData )
            packets += static_cast< CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData().packetsReceived;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformServiceDiscoveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ServiceDiscovery" );

    GUCEF_TESTFW_TESTCASE( "Test: Service types follow _<name>._tcp or _<name>._udp" )
        try
        {
            ASSERT_TRUE( CServiceAnnouncement::IsValidServiceType( "_gucef-assetstore._tcp" ) );
            ASSERT_TRUE( CServiceAnnouncement::IsValidServiceType( "_gucef-game._udp" ) );
            ASSERT_TRUE( CServiceAnnouncement::IsValidServiceType( "_X1._TCP" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "gucef-assetstore._tcp" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "_gucef-assetstore._sctp" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "_._tcp" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "_-leading._tcp" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "_trailing-._tcp" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "_with space._tcp" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsValidServiceType( "_" + CORE::CString( "abcdefghij" ) + "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghijabcd._tcp" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An announcement survives a discovery entry, its reserved attributes and its port" )
        try
        {
            CServiceAnnouncement announcement = MakeAnnouncement( "svc-1", 47400 );
            CORE::CString reason;
            ASSERT_TRUE( announcement.IsValid( reason ) );

            CDiscoveryEntry entry;
            announcement.ToEntry( entry );
            ASSERT_TRUE( "1.2" == entry.GetAttributes().GetValueAlways( "pv" ).AsString() );
            ASSERT_TRUE( "ABCD-EFGH-JKMN-PQRS-TVWX" == entry.GetAttributes().GetValueAlways( "kfp" ).AsString() );
            ASSERT_TRUE( "tcp://0.0.0.0:47400" == entry.GetEndpoints()[ 0 ] );

            CServiceAnnouncement back;
            ASSERT_TRUE( back.FromEntry( entry ) );
            ASSERT_TRUE( back.HasSameContent( announcement ) );
            ASSERT_TRUE( back.properties.GetKeyCount() == 1 );
            ASSERT_TRUE( "1" == back.GetProtocolMajor() );
            ASSERT_TRUE( 47400 == back.GetPort( "tcp" ) );
            ASSERT_TRUE( 0 == back.GetPort( "udp" ) );
            ASSERT_TRUE( back.GetKey() == CIDiscoveryEntry::MakeEntryKey( BusServiceType, "svc-1" ) );

            // an entry without an instance id is not an announcement
            CDiscoveryEntry anonymous;
            anonymous.SetServiceType( BusServiceType );
            anonymous.SetInstanceName( "no id" );
            ASSERT_FALSE( back.FromEntry( anonymous ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An announcement that cannot be sent is refused: type, id, TTL, endpoints, secrets, reserved keys and size" )
        try
        {
            CORE::CString reason;
            CServiceAnnouncement a = MakeAnnouncement( "svc-1", 47400 );
            a.serviceType = "assetstore";
            ASSERT_FALSE( a.IsValid( reason ) );

            a = MakeAnnouncement( "", 47400 );
            ASSERT_FALSE( a.IsValid( reason ) );

            a = MakeAnnouncement( "svc-1", 47400 );
            a.ttlInSecs = 1;
            ASSERT_FALSE( a.IsValid( reason ) );

            a = MakeAnnouncement( "svc-1", 47400 );
            for ( UInt32 i=0; i<CServiceAnnouncement::MaxEndpoints; ++i )
                a.endpoints.push_back( "tcp://10.0.0." + CORE::ToString( i ) + ":1" );
            ASSERT_FALSE( a.IsValid( reason ) );

            const char* secretKeys[] = { "password", "adminPassword", "sessionToken", "SECRET", "api_key", "privateKey", "credentials" };
            for ( UInt32 i=0; i<sizeof( secretKeys ) / sizeof( secretKeys[ 0 ] ); ++i )
            {
                a = MakeAnnouncement( "svc-1", 47400 );
                a.properties.Set( secretKeys[ i ], "x" );
                ASSERT_FALSE( a.IsValid( reason ) );
                ASSERT_TRUE( CServiceAnnouncement::IsForbiddenPropertyKey( secretKeys[ i ] ) );
            }
            ASSERT_FALSE( CServiceAnnouncement::IsForbiddenPropertyKey( "pairingPort" ) );
            ASSERT_FALSE( CServiceAnnouncement::IsForbiddenPropertyKey( "serverKey" ) );

            a = MakeAnnouncement( "svc-1", 47400 );
            a.properties.Set( "pv", "9" );
            ASSERT_FALSE( a.IsValid( reason ) );

            a = MakeAnnouncement( "svc-1", 47400 );
            CORE::CString big;
            for ( UInt32 i=0; i<150; ++i )
                big += "0123456789";
            a.properties.Set( "big", big );
            ASSERT_TRUE( a.GetEncodedSizeInBytes() > CServiceAnnouncement::MaxEncodedSizeInBytes );
            ASSERT_FALSE( a.IsValid( reason ) );
            ASSERT_FALSE( reason.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The settings of announcements, advertisers, finders, filters and limiters survive a data tree" )
        try
        {
            CServiceAnnouncement a = MakeAnnouncement( "svc-cfg", 47400 );
            CORE::CDataNode aTree( "Announcement", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( a.SaveConfig( aTree ) );
            CServiceAnnouncement aBack;
            ASSERT_TRUE( aBack.LoadConfig( aTree ) );
            ASSERT_TRUE( aBack.HasSameContent( a ) && aBack.ttlInSecs == a.ttlInSecs );

            CServiceAdvertiserSettings adv;
            adv.serviceType = "_gucef-assetstore._tcp";
            adv.instanceId = "inst-7";
            adv.displayName = "Asset store";
            adv.ttlInSecs = 12;
            CORE::CDataNode* client = adv.clientConfig.AddChild( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
            client->SetAttribute( "type", "broadcast" );
            client->SetAttribute( "port", "47800" );
            CORE::CDataNode advTree( "Advertiser", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( adv.SaveConfig( advTree ) );
            CServiceAdvertiserSettings advBack;
            ASSERT_TRUE( advBack.LoadConfig( advTree ) );
            ASSERT_TRUE( advBack.serviceType == adv.serviceType && advBack.instanceId == adv.instanceId && advBack.displayName == adv.displayName && 12 == advBack.ttlInSecs );
            const CORE::CDataNode* clientBack = advBack.clientConfig.FindChild( "DiscoveryClient" );
            ASSERT_TRUE( GUCEF_NULL != clientBack && "broadcast" == clientBack->GetAttributeValue( "type" ).AsString() && 47800 == clientBack->GetAttributeValue( "port" ).AsUInt32() );
            ASSERT_TRUE( CDiscoveryClientList::NamesClients( advBack.clientConfig ) );

            CServiceFinderSettings fin;
            fin.serviceType = "_gucef-assetstore._tcp";
            fin.findTimeoutMs = 1234;
            fin.findMinCandidates = 3;
            fin.threadPoolName = "discovery";
            fin.SetClientTypes( "broadcast,multicast" );
            fin.filter.protocolMajor = "2";
            fin.filter.keyFingerprint = "ABCD";
            fin.filter.requireEndpoint = false;
            fin.filter.requiredProperties.Set( "role", "primary" );
            CORE::CDataNode finTree( "Finder", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( fin.SaveConfig( finTree ) );
            CServiceFinderSettings finBack;
            ASSERT_TRUE( finBack.LoadConfig( finTree ) );
            ASSERT_TRUE( finBack.serviceType == fin.serviceType && 1234 == finBack.findTimeoutMs && 3 == finBack.findMinCandidates && "discovery" == finBack.threadPoolName );
            ASSERT_TRUE( "broadcast,multicast" == finBack.clientConfig.GetAttributeValue( "discoveryClients" ).AsString() );
            ASSERT_TRUE( "2" == finBack.filter.protocolMajor && "ABCD" == finBack.filter.keyFingerprint && !finBack.filter.requireEndpoint );
            ASSERT_TRUE( finBack.filter.requiredProperties.HasKeyAndValue( CORE::CString( "role" ), CORE::CString( "primary" ) ) );

            CDiscoveryAnswerLimiter limiter;
            limiter.SetAnswersPerSourcePerMinute( 7 );
            limiter.SetAnswerBurstPerSource( 2 );
            limiter.SetAnswersPerSecond( 0 );
            limiter.SetAmplificationFactor( 3 );
            limiter.SetMaxTrackedSources( 16 );
            CORE::CDataNode limTree( "Limits", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( limiter.SaveConfig( limTree ) );
            CDiscoveryAnswerLimiter limBack;
            ASSERT_TRUE( limBack.LoadConfig( limTree ) );
            ASSERT_TRUE( 7 == limBack.GetAnswersPerSourcePerMinute() && 2 == limBack.GetAnswerBurstPerSource() && 0 == limBack.GetAnswersPerSecond()
                         && 3 == limBack.GetAmplificationFactor() && 16 == limBack.GetMaxTrackedSources() );

            // the defaults of a service: broadcast and multicast
            CServiceFinderSettings defaults;
            ASSERT_TRUE( 2 == defaults.defaultClientTypes.size() && "broadcast" == defaults.defaultClientTypes[ 0 ] && "multicast" == defaults.defaultClientTypes[ 1 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The answer limiter: a burst per source, refill over time, a process limit, forged sources and the amplification budget" )
        try
        {
            CDiscoveryAnswerLimiter limiter;
            limiter.SetAnswerBurstPerSource( 3 );
            limiter.SetAnswersPerSourcePerMinute( 60 );
            limiter.SetAnswersPerSecond( 0 );
            CIPv4Address asker( "192.168.1.20", 50000 );
            UInt64 now = 1000000;

            // the burst, then nothing until the bucket fills again at one answer per second
            ASSERT_TRUE( limiter.AdmitQuery( asker, now ) );
            ASSERT_TRUE( limiter.AdmitQuery( asker, now ) );
            ASSERT_TRUE( limiter.AdmitQuery( asker, now ) );
            ASSERT_FALSE( limiter.AdmitQuery( asker, now ) );
            ASSERT_FALSE( limiter.AdmitQuery( asker, now + 500 ) );
            ASSERT_TRUE( limiter.AdmitQuery( asker, now + 1001 ) );
            ASSERT_FALSE( limiter.AdmitQuery( asker, now + 1002 ) );
            // another port of the same address is the same source
            ASSERT_FALSE( limiter.AdmitQuery( CIPv4Address( "192.168.1.20", 50001 ), now + 1003 ) );
            // another address has its own bucket
            ASSERT_TRUE( limiter.AdmitQuery( CIPv4Address( "192.168.1.21", 50000 ), now + 1003 ) );
            // a long silence fills the bucket to the burst, not beyond
            UInt32 admitted = 0;
            for ( UInt32 i=0; i<10; ++i )
                if ( limiter.AdmitQuery( asker, now + 600000 ) ) ++admitted;
            ASSERT_TRUE( 3 == admitted );
            CDiscoveryAnswerLimiter::TStats stats = limiter.GetStats();
            ASSERT_TRUE( 3 + 1 + 1 + 3 == stats.admitted );
            ASSERT_TRUE( 4 + 7 == stats.refusedSourceRate );

            // sources that no single asker can have
            ASSERT_FALSE( CDiscoveryAnswerLimiter::IsAcceptableSource( CIPv4Address( "192.168.1.20", 0 ) ) );
            ASSERT_FALSE( CDiscoveryAnswerLimiter::IsAcceptableSource( CIPv4Address( "0.0.0.0", 5000 ) ) );
            ASSERT_FALSE( CDiscoveryAnswerLimiter::IsAcceptableSource( CIPv4Address( "255.255.255.255", 5000 ) ) );
            ASSERT_FALSE( CDiscoveryAnswerLimiter::IsAcceptableSource( CIPv4Address( "239.255.47.80", 5000 ) ) );
            ASSERT_FALSE( CDiscoveryAnswerLimiter::IsAcceptableSource( CIPv4Address( "240.0.0.1", 5000 ) ) );
            ASSERT_TRUE( CDiscoveryAnswerLimiter::IsAcceptableSource( CIPv4Address( "127.0.0.1", 5000 ) ) );
            ASSERT_FALSE( limiter.AdmitQuery( CIPv4Address( "224.0.0.251", 5353 ), now ) );
            ASSERT_TRUE( 1 == limiter.GetStats().refusedSource );

            // the process limit holds across forged sources
            CDiscoveryAnswerLimiter process;
            process.SetAnswersPerSecond( 5 );
            admitted = 0;
            for ( UInt32 i=0; i<100; ++i )
                if ( process.AdmitQuery( CIPv4Address( 0x0A000001 + i, CIPv4Address::HostByteOrderToNetworkByteOrder( (UInt16) 4000 ) ), now ) ) ++admitted;
            ASSERT_TRUE( 5 == admitted );
            ASSERT_TRUE( 95 == process.GetStats().refusedGlobalRate );
            ASSERT_TRUE( process.AdmitQuery( CIPv4Address( "10.1.1.1", 4000 ), now + 1000 ) );

            // the table of sources stays within its bound
            CDiscoveryAnswerLimiter bounded;
            bounded.SetMaxTrackedSources( 4 );
            bounded.SetAnswersPerSecond( 0 );
            for ( UInt32 i=0; i<50; ++i )
                bounded.AdmitQuery( CIPv4Address( 0x0A000001 + i, CIPv4Address::HostByteOrderToNetworkByteOrder( (UInt16) 4000 ) ), now + i );
            ASSERT_TRUE( 4 == bounded.GetTrackedSourceCount() );

            // the budget: as large as the query, unbounded for a verified source or a factor of 0
            ASSERT_TRUE( 1400 == limiter.GetAnswerBudget( 1400, false ) );
            ASSERT_TRUE( 20 == limiter.GetAnswerBudget( 20, false ) );
            ASSERT_TRUE( 0xFFFFFFFF == limiter.GetAnswerBudget( 20, true ) );
            limiter.SetAmplificationFactor( 0 );
            ASSERT_TRUE( 0xFFFFFFFF == limiter.GetAnswerBudget( 20, false ) );

            // with 0 nothing is limited
            CDiscoveryAnswerLimiter open;
            open.SetAnswersPerSourcePerMinute( 0 );
            open.SetAnswersPerSecond( 0 );
            for ( UInt32 i=0; i<100; ++i )
                ASSERT_TRUE( open.AdmitQuery( asker, now ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A finder finds what an advertiser announces, sees its update and its withdrawal" )
        try
        {
            CBusTypeScope busType;
            CServiceAdvertiser advertiser( MakeBusAdvertiserSettings() );
            CServiceFinder finder( MakeBusFinderSettings() );
            CCandidateRecorder recorder;
            recorder.SubscribeTo( &finder );

            ASSERT_TRUE( advertiser.Advertise( MakeAnnouncement( "svc-1", 47400 ) ) );
            ASSERT_TRUE( advertiser.IsAdvertising() );
            ASSERT_TRUE( 1 == advertiser.GetDiscoveryClients().size() );

            ASSERT_TRUE( finder.Start() );
            ASSERT_TRUE( 1 == recorder.found && 1 == finder.GetCandidateCount() );
            CServiceCandidate candidate;
            ASSERT_TRUE( finder.GetCandidate( "svc-1", candidate ) );
            ASSERT_TRUE( "tcp://127.0.0.1:47400" == candidate.GetFirstEndpoint() );
            ASSERT_TRUE( "1.2" == candidate.announcement.protocolVersion );
            ASSERT_TRUE( "ABCD-EFGH-JKMN-PQRS-TVWX" == candidate.announcement.keyFingerprint );
            ASSERT_TRUE( "47401" == candidate.announcement.properties.GetValueAlways( "pairingPort" ).AsString() );
            ASSERT_TRUE( "127.0.0.1" == candidate.host.GetFirstAddressAsString() );
            ASSERT_TRUE( 1 == candidate.sourceClientNames.size() );

            // an update with the same content sends nothing new
            ASSERT_TRUE( advertiser.Update( MakeAnnouncement( "svc-1", 47400 ) ) );
            ASSERT_TRUE( 0 == recorder.updated );
            CServiceAnnouncement moved = MakeAnnouncement( "other-id-is-ignored", 47410 );
            ASSERT_TRUE( advertiser.Update( moved ) );
            ASSERT_TRUE( 1 == recorder.updated && "svc-1" == recorder.last.announcement.instanceId );
            ASSERT_TRUE( 47410 == recorder.last.announcement.GetPort( "tcp" ) );

            ASSERT_TRUE( advertiser.Withdraw() );
            ASSERT_FALSE( advertiser.IsAdvertising() );
            ASSERT_TRUE( 1 == recorder.lost && 0 == finder.GetCandidateCount() );
            ASSERT_FALSE( advertiser.Withdraw() );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The advertiser fills what the announcement leaves empty from its settings and makes an instance id once" )
        try
        {
            CBusTypeScope busType;
            CServiceAdvertiserSettings settings = MakeBusAdvertiserSettings();
            settings.displayName = "From the config";
            settings.ttlInSecs = 9;
            CServiceAdvertiser advertiser( settings );

            CServiceAnnouncement bare;
            bare.endpoints.push_back( "tcp://0.0.0.0:47400" );
            ASSERT_TRUE( advertiser.Advertise( bare ) );
            const CServiceAnnouncement& made = advertiser.GetAnnouncement();
            ASSERT_TRUE( BusServiceType == made.serviceType && "From the config" == made.displayName && 9 == made.ttlInSecs );
            ASSERT_TRUE( 32 == made.instanceId.Length() );
            CORE::CString firstId = made.instanceId;
            ASSERT_TRUE( advertiser.Withdraw() );
            ASSERT_TRUE( advertiser.Advertise( bare ) );
            ASSERT_TRUE( firstId == advertiser.GetAnnouncement().instanceId );

            // what cannot be sent is not advertised at all
            CServiceAnnouncement secret = bare;
            secret.properties.Set( "accessToken", "abc" );
            ASSERT_FALSE( advertiser.Advertise( secret ) );
            ASSERT_FALSE( advertiser.IsAdvertising() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A candidate heard by two clients is one candidate, lost only when both lost it" )
        try
        {
            CServiceFinder finder;
            CServiceFinderSettings settings;
            settings.serviceType = BusServiceType;
            finder.SetSettings( settings );
            CBusTypeScope busType;
            CIDiscoveryClientPtr clientA = CComCoreGlobal::Instance()->GetDiscoveryClientFactory().Create( CTestBusDiscoveryClient::TypeName );
            CIDiscoveryClientPtr clientB = CComCoreGlobal::Instance()->GetDiscoveryClientFactory().Create( CTestBusDiscoveryClient::TypeName );
            CTestBusDiscoveryClient* a = static_cast< CTestBusDiscoveryClient* >( clientA.GetPointerAlways() );
            CTestBusDiscoveryClient* b = static_cast< CTestBusDiscoveryClient* >( clientB.GetPointerAlways() );
            a->SetDiscoveryClientName( "a" );
            b->SetDiscoveryClientName( "b" );
            ASSERT_TRUE( finder.AddDiscoveryClient( clientA ) && finder.AddDiscoveryClient( clientB ) );
            CCandidateRecorder recorder;
            recorder.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );
            ASSERT_TRUE( 2 == finder.GetDiscoveryClients().size() );

            CDiscoveryEntry entry;
            MakeAnnouncement( "svc-2", 47400 ).ToEntry( entry );
            ASSERT_TRUE( a->Hear( entry ) );
            ASSERT_TRUE( b->Hear( entry ) );
            ASSERT_TRUE( 1 == recorder.found && 0 == recorder.updated && 1 == finder.GetCandidateCount() );
            CServiceCandidate candidate;
            ASSERT_TRUE( finder.GetCandidate( "svc-2", candidate ) && 2 == candidate.sourceClientNames.size() );

            ASSERT_TRUE( a->HearGoodbye( BusServiceType, "svc-2" ) );
            ASSERT_TRUE( 0 == recorder.lost && 1 == finder.GetCandidateCount() );
            ASSERT_TRUE( b->HearGoodbye( BusServiceType, "svc-2" ) );
            ASSERT_TRUE( 1 == recorder.lost && 0 == finder.GetCandidateCount() );

            // another service type and an entry without an instance id are no candidates
            CDiscoveryEntry other;
            MakeAnnouncement( "svc-3", 1 ).ToEntry( other );
            other.SetServiceType( "_gucef-other._tcp" );
            a->Hear( other );
            CDiscoveryEntry anonymous;
            anonymous.SetServiceType( BusServiceType );
            anonymous.SetInstanceName( "anonymous" );
            anonymous.SetTtlInSecs( 30 );
            a->Hear( anonymous );
            ASSERT_TRUE( 0 == finder.GetCandidateCount() && 1 == recorder.found );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The filter keeps only matching candidates: fingerprint, protocol major, properties, an endpoint; a non-matching announcement cannot displace a candidate" )
        try
        {
            CServiceFinder finder;
            CServiceFinderSettings settings;
            settings.serviceType = BusServiceType;
            settings.filter.keyFingerprint = "abcd-efgh-jkmn-pqrs-tvwx";
            settings.filter.protocolMajor = "1";
            finder.SetSettings( settings );
            CBusTypeScope busType;
            CIDiscoveryClientPtr client = CComCoreGlobal::Instance()->GetDiscoveryClientFactory().Create( CTestBusDiscoveryClient::TypeName );
            CTestBusDiscoveryClient* bus = static_cast< CTestBusDiscoveryClient* >( client.GetPointerAlways() );
            finder.AddDiscoveryClient( client );
            CCandidateRecorder recorder;
            recorder.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );

            CDiscoveryEntry entry;
            CServiceAnnouncement wrongKey = MakeAnnouncement( "spoof", 47400 );
            wrongKey.keyFingerprint = "ZZZZ-ZZZZ-ZZZZ-ZZZZ-ZZZZ";
            wrongKey.ToEntry( entry );
            bus->Hear( entry );
            CServiceAnnouncement wrongMajor = MakeAnnouncement( "future", 47400 );
            wrongMajor.protocolVersion = "2.0";
            wrongMajor.ToEntry( entry );
            bus->Hear( entry );
            CServiceAnnouncement noEndpoint = MakeAnnouncement( "silent", 47400 );
            noEndpoint.endpoints.clear();
            noEndpoint.ToEntry( entry );
            bus->Hear( entry );
            ASSERT_TRUE( 0 == recorder.found );

            MakeAnnouncement( "mine", 47400 ).ToEntry( entry );
            bus->Hear( entry );
            ASSERT_TRUE( 1 == recorder.found && "mine" == recorder.last.announcement.instanceId );

            // an announcement with the key of the candidate that does not match (a spoofer with another key fingerprint)
            // neither replaces nor removes it
            CServiceAnnouncement spoofed = MakeAnnouncement( "mine", 47999 );
            spoofed.keyFingerprint = "ZZZZ";
            spoofed.ToEntry( entry );
            bus->Hear( entry );
            ASSERT_TRUE( 0 == recorder.lost && 0 == recorder.updated && 1 == finder.GetCandidateCount() );
            CServiceCandidate kept;
            ASSERT_TRUE( finder.GetCandidate( "mine", kept ) && 47400 == kept.announcement.GetPort( "tcp" ) );

            // a new filter that refuses it does remove it
            CServiceCandidateFilter stricter = settings.filter;
            stricter.requiredProperties.Set( "role", "primary" );
            finder.SetFilter( stricter );
            ASSERT_TRUE( 1 == recorder.lost && 0 == finder.GetCandidateCount() );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A candidate that is not heard again within its TTL is lost, on the pulse generator of the finder" )
        try
        {
            CORE::PulseGeneratorPtr pulse = CORE::CPulseGenerator::CreateSharedObj();
            CBusTypeScope busType;
            CServiceFinder finder( pulse, MakeBusFinderSettings() );
            CCandidateRecorder recorder;
            recorder.SubscribeTo( &finder );
            ASSERT_TRUE( finder.Start() );
            ASSERT_TRUE( 1 == finder.GetDiscoveryClients().size() );
            CIDiscoveryClientPtr client = finder.GetDiscoveryClients()[ 0 ];
            CTestBusDiscoveryClient* bus = static_cast< CTestBusDiscoveryClient* >( client.GetPointerAlways() );
            ASSERT_TRUE( bus->GetPulseGenerator() == pulse );

            CServiceAnnouncement shortLived = MakeAnnouncement( "short", 47400 );
            shortLived.ttlInSecs = 3;
            CDiscoveryEntry entry;
            shortLived.ToEntry( entry );
            ASSERT_TRUE( bus->Hear( entry ) );
            ASSERT_TRUE( 1 == recorder.found );
            PumpOwn( pulse, 1500 );
            ASSERT_TRUE( 0 == recorder.lost );
            ASSERT_TRUE( PumpOwnUntilAtLeast( pulse, recorder.lost, 1, 4000 ) );
            ASSERT_TRUE( 0 == finder.GetCandidateCount() );
            finder.Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: FindOnce finds on the calling thread and ends early, or gives up after its time" )
        try
        {
            CBusTypeScope busType;
            CServiceAdvertiser advertiser( MakeBusAdvertiserSettings() );
            ASSERT_TRUE( advertiser.Advertise( MakeAnnouncement( "svc-once", 47400 ) ) );

            CServiceFinderSettings settings = MakeBusFinderSettings();
            settings.findTimeoutMs = 3000;
            CServiceFinder::TServiceCandidateVector candidates;
            UInt64 start = CDiscoveryClientBase::GetTimeInMs();
            ASSERT_TRUE( CServiceFinder::FindOnce( settings, candidates ) );
            ASSERT_TRUE( CDiscoveryClientBase::GetTimeInMs() - start < 2000 );
            ASSERT_TRUE( 1 == candidates.size() && "svc-once" == candidates[ 0 ].announcement.instanceId );

            // nothing of this type: false after the time, nothing found
            settings.serviceType = "_gucef-nothing._tcp";
            settings.findTimeoutMs = 300;
            candidates.clear();
            start = CDiscoveryClientBase::GetTimeInMs();
            ASSERT_FALSE( CServiceFinder::FindOnce( settings, candidates ) );
            ASSERT_TRUE( CDiscoveryClientBase::GetTimeInMs() - start >= 300 );
            ASSERT_TRUE( candidates.empty() );

            // with a minimum of 0 it looks the whole time and gives what it found
            settings.serviceType = BusServiceType;
            settings.findMinCandidates = 0;
            settings.findTimeoutMs = 200;
            ASSERT_TRUE( CServiceFinder::FindOnce( settings, candidates ) );
            ASSERT_TRUE( 1 == candidates.size() );

            // a type that does not exist: no client, no look
            settings.SetClientTypes( "no-such-client" );
            candidates.clear();
            ASSERT_FALSE( CServiceFinder::FindOnce( settings, candidates ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: FindOnceAsync finds as a task on a thread pool and its future carries the candidates" )
        try
        {
            CBusTypeScope busType;
            CServiceAdvertiser advertiser( MakeBusAdvertiserSettings() );
            ASSERT_TRUE( advertiser.Advertise( MakeAnnouncement( "svc-async", 47400 ) ) );

            CServiceFinderSettings settings = MakeBusFinderSettings();
            settings.findTimeoutMs = 3000;
            CORE::CFutureResult future = CServiceFinder::FindOnceAsync( settings );
            ASSERT_TRUE( future.HasAFuture() );
            for ( UInt32 waited=0; waited<5000 && !future.IsReady(); ++waited )
                MT::ThreadDelay( 1 );
            ASSERT_TRUE( future.IsReady() );
            CServiceFinder::TServiceCandidateVector candidates;
            bool found = false;
            ASSERT_TRUE( CServiceFinder::GetFindOnceResult( future, candidates, found ) );
            ASSERT_TRUE( found && 1 == candidates.size() && "svc-async" == candidates[ 0 ].announcement.instanceId );

            // a future of something else is no result
            CORE::CFutureResult failed( CORE::TASKSTATUS_QUEUEING_FAILED );
            ASSERT_FALSE( CServiceFinder::GetFindOnceResult( failed, candidates, found ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A discovery client moves to another pulse generator only while it is idle" )
        try
        {
            CTestBusDiscoveryClient client;
            CORE::PulseGeneratorPtr pulse = CORE::CPulseGenerator::CreateSharedObj();
            ASSERT_TRUE( client.GetPulseGenerator() == CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            ASSERT_TRUE( client.Browse( BusServiceType ) );
            ASSERT_FALSE( client.SetPulseGenerator( pulse ) );
            ASSERT_TRUE( client.StopBrowse( BusServiceType ) );
            ASSERT_TRUE( client.SetPulseGenerator( pulse ) );
            ASSERT_TRUE( client.GetPulseGenerator() == pulse );
            ASSERT_FALSE( client.SetPulseGenerator( CORE::PulseGeneratorPtr() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A UDP socket moved to another pulse generator is no longer driven by the old one" )
        try
        {
            CORE::PulseGeneratorPtr oldPulse = CORE::CPulseGenerator::CreateSharedObj();
            CORE::PulseGeneratorPtr newPulse = CORE::CPulseGenerator::CreateSharedObj();
            CUDPSocket receiver( oldPulse, false );
            receiver.SetPulseGenerator( newPulse );
            CPacketCounter counter;
            counter.SubscribeTo( &receiver, CUDPSocket::UDPPacketsRecievedEvent );
            UInt16 port = TESTFW::FindFreeUdpPort( 47831 );
            ASSERT_TRUE( receiver.Open( CIPv4Address( "127.0.0.1", port ) ) );

            CUDPSocket sender( false );
            ASSERT_TRUE( sender.Open( (UInt16) 0 ) );
            const char data[] = "moved";
            ASSERT_TRUE( 5 == sender.SendPacketTo( CIPv4Address( "127.0.0.1", port ), data, 5 ) );
            MT::ThreadDelay( 20 );

            // the old pulse generator does not reach the socket any more
            PumpOwn( oldPulse, 50 );
            ASSERT_TRUE( 0 == counter.packets );
            ASSERT_TRUE( PumpOwnUntilAtLeast( newPulse, counter.packets, 1, 2000 ) );
            receiver.Close();
            sender.Close();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
