/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#include "gucefCOMCORE_CDiscoveryClientBase.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#include "gucefCOMCORE_CDiscoveryClientFactory.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CIRTTPROBE_H
#include "gucefCOMCORE_CIRttProbe.h"
#define GUCEF_COMCORE_CIRTTPROBE_H
#endif /* GUCEF_COMCORE_CIRTTPROBE_H ? */

#ifndef GUCEF_COMCORE_CIPORTMAPPER_H
#include "gucefCOMCORE_CIPortMapper.h"
#define GUCEF_COMCORE_CIPORTMAPPER_H
#endif /* GUCEF_COMCORE_CIPORTMAPPER_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCode_Pump.h"

#include "TestCode_Discovery.h"

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

/**
 *  A discovery client without a network: the test tells it what it hears
 */
class CTestDiscoveryClient : public CDiscoveryClientBase
{
    public:

    static const CORE::CString TypeName;

    UInt32 browseStarts;
    UInt32 browseStops;
    UInt32 advertisements;
    UInt32 withdrawals;

    CTestDiscoveryClient( void )
        : CDiscoveryClientBase( TypeName )
        , browseStarts( 0 )
        , browseStops( 0 )
        , advertisements( 0 )
        , withdrawals( 0 )
    {GUCEF_TRACE;

    }

    virtual ~CTestDiscoveryClient()
    {GUCEF_TRACE;

        StopAll();
    }

    bool Hear( const CIDiscoveryEntry& entry )
    {GUCEF_TRACE;

        return ProcessHeardEntry( entry );
    }

    bool HearGoodbye( const CORE::CString& serviceType, const CORE::CString& instanceId )
    {GUCEF_TRACE;

        return ProcessGoodbye( serviceType, instanceId );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        static const CORE::CString classTypeName = "CTestDiscoveryClient";
        return classTypeName;
    }

    protected:

    virtual bool OnBrowseStarted( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++browseStarts;
        return true;
    }

    virtual void OnBrowseStopped( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++browseStops;
    }

    virtual bool OnAdvertised( const CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++advertisements;
        return true;
    }

    virtual bool OnWithdrawn( const CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;

        ++withdrawals;
        return true;
    }
};

const CORE::CString CTestDiscoveryClient::TypeName = "test-fake";

typedef CORE::CTFactory< CIDiscoveryClient, CTestDiscoveryClient, MT::CMutex > TTestDiscoveryClientFactory;
TTestDiscoveryClientFactory g_testDiscoveryClientFactory;

/*-------------------------------------------------------------------------*/

/**
 *  Counts the discovery events of a notifier and keeps the entry of the last one
 */
class CDiscoveryEventRecorder : public CORE::CObserver
{
    public:

    UInt32 discovered;
    UInt32 updated;
    UInt32 lost;
    UInt32 browseStarted;
    UInt32 browseStopped;
    UInt32 errors;
    CIDiscoveryEntryPtr lastEntry;

    CDiscoveryEventRecorder( void )
        : CORE::CObserver()
        , discovered( 0 )
        , updated( 0 )
        , lost( 0 )
        , browseStarted( 0 )
        , browseStopped( 0 )
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

        if ( CIDiscoveryClient::EntryDiscoveredEvent == eventId )
            ++discovered;
        else if ( CIDiscoveryClient::EntryUpdatedEvent == eventId )
            ++updated;
        else if ( CIDiscoveryClient::EntryLostEvent == eventId )
            ++lost;
        else if ( CIDiscoveryClient::BrowseStartedEvent == eventId )
            ++browseStarted;
        else if ( CIDiscoveryClient::BrowseStoppedEvent == eventId )
            ++browseStopped;
        else if ( CIDiscoveryClient::DiscoveryErrorEvent == eventId )
            ++errors;
        else
            return;

        if ( GUCEF_NULL != eventData && ( CIDiscoveryClient::EntryDiscoveredEvent == eventId || CIDiscoveryClient::EntryUpdatedEvent == eventId || CIDiscoveryClient::EntryLostEvent == eventId ) )
            lastEntry = *static_cast< CDiscoveryEntryEventData* >( eventData );
    }
};

/*-------------------------------------------------------------------------*/

CDiscoveryEntry
MakeEntry( const CORE::CString& serviceType, const CORE::CString& instanceId, const CORE::CString& name, UInt32 ttl )
{GUCEF_TRACE;

    CDiscoveryEntry entry;
    entry.SetServiceType( serviceType );
    entry.SetApplicationInstanceId( instanceId );
    entry.SetInstanceName( name );
    entry.SetApplicationId( "test-app" );
    entry.SetCapabilities( "chat, lobby" );
    entry.SetTtlInSecs( ttl );
    entry.GetEndpoints().push_back( "udp://10.0.0.1:7777" );
    entry.GetAttributes().Set( "map", "dust" );
    entry.SetHost( CHostAddress( CIPv4Address( "10.0.0.1", 7777 ), "10.0.0.1" ) );
    return entry;
}

/*-------------------------------------------------------------------------*/

/**
 *  A future that the test completes with a port mapping result, as a port mapper would
 */
class CPromise
{
    public:

    CORE::CFutureResultSource source;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDiscoveryTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Discovery" );

    GUCEF_TESTFW_TESTCASE( "Test: An entry is identified by its service type and instance id, its content can be compared" )
        try
        {
            CDiscoveryEntry a = MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 );
            ASSERT_TRUE( a.GetEntryKey() == CIDiscoveryEntry::MakeEntryKey( "_gucef-test._udp", "id-1" ) );

            CDiscoveryEntry noId = MakeEntry( "_gucef-test._udp", "", "Game One", 30 );
            ASSERT_TRUE( noId.GetEntryKey() == CIDiscoveryEntry::MakeEntryKey( "_gucef-test._udp", "Game One" ) );

            CDiscoveryEntry b( a );
            ASSERT_TRUE( a.HasSameContent( b ) );
            b.SetLastSeen( CORE::CDateTime::NowUTCDateTime() );
            b.SetSourceClientName( "elsewhere" );
            ASSERT_TRUE( a.HasSameContent( b ) );

            b.GetAttributes().Set( "map", "inferno" );
            ASSERT_FALSE( a.HasSameContent( b ) );

            CDiscoveryEntry c( a );
            c.GetEndpoints().push_back( "tcp://10.0.0.1:7778" );
            ASSERT_FALSE( a.HasSameContent( c ) );

            CDiscoveryEntry d( static_cast< const CIDiscoveryEntry& >( a ) );
            ASSERT_TRUE( a.HasSameContent( d ) );
            ASSERT_TRUE( d.GetInstanceName() == "Game One" );
            ASSERT_TRUE( d.GetTtlInSecs() == 30 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A client reports what it hears for browsed service types: discovered, updated, lost by goodbye" )
        try
        {
            CTestDiscoveryClient client;
            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &client );

            // Nothing is reported for service types that are not browsed
            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 ) ) );
            ASSERT_TRUE( 0 == recorder.discovered );

            ASSERT_TRUE( client.Browse( "_gucef-test._udp" ) );
            ASSERT_TRUE( client.IsBrowsing( "_gucef-test._udp" ) );
            ASSERT_TRUE( client.IsBrowsing() );
            ASSERT_TRUE( 1 == client.browseStarts );
            ASSERT_TRUE( 1 == recorder.browseStarted );

            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 ) ) );
            ASSERT_TRUE( 1 == recorder.discovered );
            ASSERT_TRUE( !recorder.lastEntry.IsNULL() && recorder.lastEntry->GetSourceClientName() == CTestDiscoveryClient::TypeName );

            // The same again only renews it
            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 ) ) );
            ASSERT_TRUE( 1 == recorder.discovered );
            ASSERT_TRUE( 0 == recorder.updated );

            CDiscoveryEntry changed = MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 );
            changed.GetAttributes().Set( "players", "3" );
            ASSERT_TRUE( client.Hear( changed ) );
            ASSERT_TRUE( 1 == recorder.updated );
            ASSERT_TRUE( recorder.lastEntry->GetAttributes().HasKey( "players" ) );

            CIDiscovery::TIDiscoveryEntrySet entries;
            ASSERT_TRUE( client.GetDiscoveredEntries( "_gucef-test._udp", entries ) );
            ASSERT_TRUE( 1 == entries.size() );

            entries.clear();
            ASSERT_TRUE( client.FindApplication( "test-app", entries ) );
            ASSERT_TRUE( 1 == entries.size() );
            entries.clear();
            ASSERT_TRUE( client.FindCapability( "lobby", entries ) );
            ASSERT_TRUE( 1 == entries.size() );
            entries.clear();
            ASSERT_TRUE( client.FindHost( CHostAddress( CIPv4Address( "10.0.0.1", 0 ), "" ), entries ) );
            ASSERT_TRUE( 1 == entries.size() );

            ASSERT_TRUE( client.HearGoodbye( "_gucef-test._udp", "id-1" ) );
            ASSERT_TRUE( 1 == recorder.lost );
            entries.clear();
            client.GetDiscoveredEntries( "_gucef-test._udp", entries );
            ASSERT_TRUE( entries.empty() );

            // A TTL of 0 is a goodbye as well
            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "id-2", "Game Two", 30 ) ) );
            ASSERT_TRUE( 2 == recorder.discovered );
            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "id-2", "Game Two", 0 ) ) );
            ASSERT_TRUE( 2 == recorder.lost );

            // Stopping the browse forgets what was found
            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "id-3", "Game Three", 30 ) ) );
            ASSERT_TRUE( client.StopBrowse( "_gucef-test._udp" ) );
            ASSERT_TRUE( 3 == recorder.lost );
            ASSERT_TRUE( 1 == recorder.browseStopped );
            ASSERT_FALSE( client.IsBrowsing() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An entry that is not heard again within its TTL is lost" )
        try
        {
            CTestDiscoveryClient client;
            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &client );

            ASSERT_TRUE( client.Browse( "_gucef-test._udp" ) );
            ASSERT_TRUE( client.Hear( MakeEntry( "_gucef-test._udp", "short", "Short Lived", 1 ) ) );
            ASSERT_TRUE( 1 == recorder.discovered );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.lost, 1, 3000 ) );
            CORE::CString lostId;
            ASSERT_TRUE( !recorder.lastEntry.IsNULL() && recorder.lastEntry->GetApplicationInstanceId( lostId ) && lostId == "short" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A client ignores its own advertisements and browsing for * accepts every service type" )
        try
        {
            CTestDiscoveryClient client;
            CDiscoveryEventRecorder recorder;
            recorder.SubscribeTo( &client );

            CIDiscoveryEntryPtr own( GUCEF_NEW CDiscoveryEntry( MakeEntry( "_gucef-test._udp", "me", "Myself", 30 ) ) );
            ASSERT_TRUE( client.Advertise( own ) );
            ASSERT_TRUE( 1 == client.advertisements );
            ASSERT_TRUE( client.Browse( CIDiscoveryClient::AnyServiceType ) );

            ASSERT_TRUE( client.Hear( *own ) );
            ASSERT_TRUE( 0 == recorder.discovered );

            ASSERT_TRUE( client.Hear( MakeEntry( "_other._tcp", "them", "Them", 30 ) ) );
            ASSERT_TRUE( 1 == recorder.discovered );

            ASSERT_TRUE( client.Withdraw( own ) );
            ASSERT_TRUE( 1 == client.withdrawals );
            ASSERT_FALSE( client.Withdraw( own ) );

            // Register and Unregister are the same operations
            ASSERT_TRUE( client.Register( own ) );
            ASSERT_TRUE( 2 == client.advertisements );
            ASSERT_TRUE( client.Unregister( own ) );
            ASSERT_TRUE( 2 == client.withdrawals );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The discovery manager creates clients from config and merges what they find" )
        try
        {
            CComCoreGlobal* comCore = CComCoreGlobal::Instance();
            comCore->GetDiscoveryClientFactory().RegisterConcreteFactory( CTestDiscoveryClient::TypeName, &g_testDiscoveryClientFactory );
            CDiscoveryManager& manager = comCore->GetDiscoveryManager();

            CORE::CDataNode config( "Config" );
            CORE::CDataNode* managerNode = config.AddChild( "DiscoveryManager" );
            CORE::CDataNode* clientA = managerNode->AddChild( "DiscoveryClient" );
            clientA->SetAttribute( "type", CTestDiscoveryClient::TypeName );
            clientA->SetAttribute( "name", "fake-a" );
            CORE::CDataNode* clientB = managerNode->AddChild( "DiscoveryClient" );
            clientB->SetAttribute( "type", CTestDiscoveryClient::TypeName );
            clientB->SetAttribute( "name", "fake-b" );
            ASSERT_TRUE( manager.LoadConfig( config ) );

            CIDiscoveryClientPtr a = manager.GetDiscoveryClient( "fake-a" );
            CIDiscoveryClientPtr b = manager.GetDiscoveryClient( "fake-b" );
            ASSERT_TRUE( !a.IsNULL() && !b.IsNULL() );
            if ( !a.IsNULL() && !b.IsNULL() )
            {
                CTestDiscoveryClient* fakeA = static_cast< CTestDiscoveryClient* >( a.GetPointerAlways() );
                CTestDiscoveryClient* fakeB = static_cast< CTestDiscoveryClient* >( b.GetPointerAlways() );

                CDiscoveryEventRecorder recorder;
                recorder.SubscribeTo( &manager );

                ASSERT_TRUE( manager.Browse( "_gucef-test._udp" ) );
                ASSERT_TRUE( 1 == fakeA->browseStarts && 1 == fakeB->browseStarts );
                ASSERT_TRUE( 1 == recorder.browseStarted );

                CIDiscoveryEntryPtr own( GUCEF_NEW CDiscoveryEntry( MakeEntry( "_gucef-test._udp", "me", "Myself", 30 ) ) );
                ASSERT_TRUE( manager.Advertise( own ) );
                ASSERT_TRUE( 1 == fakeA->advertisements && 1 == fakeB->advertisements );

                // Both clients find the same entry: one discovery
                fakeA->Hear( MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 ) );
                fakeB->Hear( MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 ) );
                ASSERT_TRUE( 1 == recorder.discovered );
                ASSERT_TRUE( 0 == recorder.updated );

                CIDiscovery::TIDiscoveryEntrySet entries;
                manager.GetDiscoveredEntries( "_gucef-test._udp", entries );
                ASSERT_TRUE( 1 == entries.size() );

                // A change heard by one of them is an update
                CDiscoveryEntry changed = MakeEntry( "_gucef-test._udp", "id-1", "Game One", 30 );
                changed.SetInstanceName( "Game One Renamed" );
                fakeA->Hear( changed );
                ASSERT_TRUE( 1 == recorder.updated );

                // Lost only once no client knows it
                fakeA->HearGoodbye( "_gucef-test._udp", "id-1" );
                ASSERT_TRUE( 0 == recorder.lost );
                fakeB->HearGoodbye( "_gucef-test._udp", "id-1" );
                ASSERT_TRUE( 1 == recorder.lost );

                // Unregistering a client forgets what only it knew
                fakeA->Hear( MakeEntry( "_gucef-test._udp", "id-2", "Game Two", 30 ) );
                ASSERT_TRUE( 2 == recorder.discovered );
                ASSERT_TRUE( manager.Withdraw( own ) );
                ASSERT_TRUE( 1 == fakeA->withdrawals && 1 == fakeB->withdrawals );

                CORE::CDataNode saved;
                ASSERT_TRUE( manager.SaveConfig( saved ) );
                CORE::CDataNode* savedManager = saved.FindChild( "DiscoveryManager" );
                ASSERT_TRUE( GUCEF_NULL != savedManager && 2 == savedManager->FindChildrenOfType( "DiscoveryClient" ).size() );

                ASSERT_TRUE( 2 == manager.UnregisterDiscoveryClientsOfType( CTestDiscoveryClient::TypeName ) );
                ASSERT_TRUE( 2 == recorder.lost );
                ASSERT_TRUE( manager.GetDiscoveryClient( "fake-a" ).IsNULL() );

                ASSERT_TRUE( manager.StopBrowse( "_gucef-test._udp" ) );
                ASSERT_TRUE( 1 == recorder.browseStopped );
            }
            manager.UnregisterDiscoveryClient( "fake-a" );
            manager.UnregisterDiscoveryClient( "fake-b" );
            comCore->GetDiscoveryClientFactory().UnregisterConcreteFactory( CTestDiscoveryClient::TypeName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A discovery client type that is not registered is refused by the manager" )
        try
        {
            CDiscoveryManager& manager = CComCoreGlobal::Instance()->GetDiscoveryManager();
            CORE::CDataNode managerNode( "DiscoveryManager" );
            CORE::CDataNode* client = managerNode.AddChild( "DiscoveryClient" );
            client->SetAttribute( "type", "no-such-type" );
            ASSERT_FALSE( manager.LoadConfig( managerNode ) );

            // A config without a section for the manager is not its business
            CORE::CDataNode unrelated( "Something" );
            ASSERT_TRUE( manager.LoadConfig( unrelated ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformNetworkServiceInterfaceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "NetworkServiceInterfaces" );

    GUCEF_TESTFW_TESTCASE( "Test: A port mapping result travels as the result data of a future" )
        try
        {
            CPromise promise;
            CORE::CFutureResult future = promise.source.GetFuture();

            CPortMappingResult result;
            ASSERT_FALSE( CPortMappingResult::TryGetFromFuture( future, result ) );

            CPortMappingResult made;
            made.protocol = CPortMappingResult::PROTOCOL_TCP;
            made.internalPort = 7777;
            made.externalPort = 17777;
            made.externalAddress = CIPv4Address( "203.0.113.5", 17777 );
            made.leaseInSecs = 3600;
            made.mapperName = "test";
            ASSERT_TRUE( promise.source.Complete( made.Clone(), true ) );

            ASSERT_TRUE( CPortMappingResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( CPortMappingResult::PROTOCOL_TCP == result.protocol );
            ASSERT_TRUE( 17777 == result.externalPort );
            ASSERT_TRUE( result.externalAddress.AddressAsString() == "203.0.113.5" );
            ASSERT_TRUE( 3600 == result.leaseInSecs );

            CPromise failing;
            CORE::CFutureResult failed = failing.source.GetFuture();
            ASSERT_TRUE( failing.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, "no gateway" ) );
            ASSERT_FALSE( CPortMappingResult::TryGetFromFuture( failed, result ) );

            // a future that finished well but carries data of another class is no port mapping result
            CPromise foreign;
            CORE::CFutureResult foreignFuture = foreign.source.GetFuture();
            ASSERT_TRUE( foreign.source.Complete( GUCEF_NEW CORE::TCloneableString( "not a mapping" ), true ) );
            ASSERT_TRUE( foreignFuture.IsReady() );
            ASSERT_FALSE( CPortMappingResult::TryGetFromFuture( foreignFuture, result ) );
            ASSERT_TRUE( 17777 == result.externalPort ); // the result is left as it was

            // the type name the check is based on is that of the class
            ASSERT_TRUE( made.IsCloneableType( CPortMappingResult::CloneableTypeName() ) );
            ASSERT_TRUE( GUCEF_NULL != CORE::CICloneable::CloneableAs< CPortMappingResult >( &made ) );
            CORE::TCloneableString plain( "x" );
            ASSERT_TRUE( GUCEF_NULL == CORE::CICloneable::CloneableAs< CPortMappingResult >( &plain ) );
            ASSERT_TRUE( GUCEF_NULL == CORE::CICloneable::CloneableAs< CPortMappingResult >( static_cast< const CORE::CICloneable* >( GUCEF_NULL ) ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The round trip time clock and event data" )
        try
        {
            UInt64 first = CIRttProbe::GetTimeInMicroSecs();
            MT::ThreadDelay( 20 );
            UInt64 second = CIRttProbe::GetTimeInMicroSecs();
            ASSERT_TRUE( second > first );
            ASSERT_TRUE( second - first >= 10000 );

            CRttProbeEventData data( CHostAddress( "127.0.0.1", 80 ), 1234, 7 );
            CORE::CICloneable* clone = data.Clone();
            CRttProbeEventData* copy = static_cast< CRttProbeEventData* >( clone );
            ASSERT_TRUE( 1234 == copy->GetRttInMicroSecs() );
            ASSERT_TRUE( 7 == copy->GetSequence() );
            ASSERT_TRUE( 80 == copy->GetTarget().GetPortInHostByteOrder() );
            GUCEF_DELETE clone;

            // The factories are there for the implementations to register in
            ASSERT_FALSE( CComCoreGlobal::Instance()->GetRttProbeFactory().IsConstructible( "no-such-method" ) );
            ASSERT_FALSE( CComCoreGlobal::Instance()->GetPortMapperFactory().IsConstructible( "no-such-mapper" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
