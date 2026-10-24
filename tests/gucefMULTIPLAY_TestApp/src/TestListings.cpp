/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVFILEUTILS_H
#include "dvfileutils.h"
#define GUCEF_CORE_DVFILEUTILS_H
#endif /* GUCEF_CORE_DVFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COM_CUDPECHORESPONDER_H
#include "gucefCOM_CUdpEchoResponder.h"
#define GUCEF_COM_CUDPECHORESPONDER_H
#endif /* GUCEF_COM_CUDPECHORESPONDER_H ? */

#ifndef GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#include "gucefMULTIPLAY_CMultiplayGlobal.h"
#define GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H
#endif /* GUCEF_MULTIPLAY_CMULTIPLAYGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLanListingProvider.h"
#define GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CHttpListListingProvider.h"
#define GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CHTTPLISTLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLobbyServiceListingProvider.h"
#define GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLOBBYSERVICELISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#include "gucefMULTIPLAY_CGameAdvertiser.h"
#define GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#endif /* GUCEF_MULTIPLAY_CGAMEADVERTISER_H ? */

#ifndef GUCEF_MULTIPLAY_CGAMEBROWSER_H
#include "gucefMULTIPLAY_CGameBrowser.h"
#define GUCEF_MULTIPLAY_CGAMEBROWSER_H
#endif /* GUCEF_MULTIPLAY_CGAMEBROWSER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPump.h"
#include "TestLobbyService.h"
#include "TestListings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** Records the failure and abandons the test case, for what the rest of it needs */
#define REQUIRE_TRUE(t) do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE( t ); throw CORE::CString( "required: " #t ); } } while ( 0 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

/* Ports of the loopback tests, away from the ones of the COM tests */
static const UInt16 LanPortA = 47910;
static const UInt16 LanPortB = 47911;
static const UInt16 EchoPort = 47912;
static const UInt16 SilentPort = 47913;
static const UInt16 TcpOnlyPort = 47914;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Counts what a listing provider reports */
class CListingRecorder : public CORE::CObserver
{
    public:

    UInt32 added;
    UInt32 updated;
    UInt32 removed;
    UInt32 errors;
    MULTIPLAY::CGameListingEntry last;

    CListingRecorder( void ) : CORE::CObserver(), added( 0 ), updated( 0 ), removed( 0 ), errors( 0 ), last() {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CListingRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CIGameListingProvider::ListingAddedEvent == eventId ) { ++added; last = *static_cast< MULTIPLAY::CGameListingEntry* >( eventData ); }
        else
        if ( MULTIPLAY::CIGameListingProvider::ListingUpdatedEvent == eventId ) { ++updated; last = *static_cast< MULTIPLAY::CGameListingEntry* >( eventData ); }
        else
        if ( MULTIPLAY::CIGameListingProvider::ListingRemovedEvent == eventId ) { ++removed; last = *static_cast< MULTIPLAY::CGameListingEntry* >( eventData ); }
        else
        if ( MULTIPLAY::CIGameListingProvider::ProviderErrorEvent == eventId ) ++errors;
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the changes of a browser list */
class CBrowserRecorder : public CORE::CObserver
{
    public:

    UInt32 added;
    UInt32 updated;
    UInt32 removed;

    CBrowserRecorder( void ) : CORE::CObserver(), added( 0 ), updated( 0 ), removed( 0 ) {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CBrowserRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CGameBrowser::BrowserListChangedEvent != eventId )
            return;
        const MULTIPLAY::CBrowserListChange* change = static_cast< const MULTIPLAY::CBrowserListChange* >( eventData );
        if ( MULTIPLAY::CBrowserListChange::CHANGE_ADDED == change->change ) ++added;
        else
        if ( MULTIPLAY::CBrowserListChange::CHANGE_UPDATED == change->change ) ++updated;
        else ++removed;
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the updates of a ping service */
class CPingRecorder : public CORE::CObserver
{
    public:

    UInt32 known;
    UInt32 unknown;
    MULTIPLAY::CPingUpdate last;

    CPingRecorder( void ) : CORE::CObserver(), known( 0 ), unknown( 0 ), last() {}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CPingRecorder";
        return classTypeName;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( MULTIPLAY::CPingService::PingUpdatedEvent != eventId )
            return;
        last = *static_cast< const MULTIPLAY::CPingUpdate* >( eventData );
        if ( last.rttMs >= 0 )
            ++known;
        else
            ++unknown;
    }
};

/*-------------------------------------------------------------------------*/

/** A provider whose listings the test sets */
class CStaticListingProvider : public MULTIPLAY::CIGameListingProvider
{
    public:

    static const CORE::CString ProviderName;

    CStaticListingProvider( void ) : MULTIPLAY::CIGameListingProvider(), m_active( false ) {}

    virtual bool Start( const MULTIPLAY::CGameListingFilter& filter ) GUCEF_VIRTUAL_OVERRIDE { SetFilter( filter ); m_active = true; return true; }
    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE { m_active = false; }
    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE { return m_active; }
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_active; }
    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE { return ProviderName; }
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CStaticListingProvider";
        return classTypeName;
    }

    void Put( const MULTIPLAY::CGameListingEntry& listing ) { UpsertListing( listing ); }
    void Drop( const CORE::CString& listingId ) { RemoveListing( listingId ); }

    private:

    bool m_active;
};

const CORE::CString CStaticListingProvider::ProviderName = "static-test";
typedef CORE::CTFactory< MULTIPLAY::CIGameListingProvider, CStaticListingProvider, MT::CMutex > TStaticListingProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static MULTIPLAY::CGameListingEntry
MakeListing( const char* id, const char* name, const char* endpoint )
{
    MULTIPLAY::CGameListingEntry listing;
    listing.listingId = id;
    listing.name = name;
    listing.gameId = "testgame";
    listing.gameVersion = "1.0";
    listing.mapName = "dust";
    listing.modeName = "ffa";
    listing.players = 1;
    listing.maxPlayers = 4;
    listing.hostName = "Host";
    if ( GUCEF_NULL != endpoint )
        listing.endpoints.push_back( endpoint );
    return listing;
}

/*-------------------------------------------------------------------------*/

/** The config of a broadcast discovery client that talks to the other port on loopback only */
static void
AddLoopbackClientConfig( CORE::CDataNode& config, UInt16 port, UInt16 otherPort, const char* name )
{
    CORE::CDataNode* client = config.AddChild( "DiscoveryClient", GUCEF_DATATYPE_OBJECT );
    client->SetAttribute( "type", CORE::CString( "broadcast" ) );
    client->SetAttribute( "name", CORE::CString( name ) );
    client->SetAttribute( "port", CORE::ToString( port ) );
    client->SetAttribute( "extraTargets", "127.0.0.1:" + CORE::ToString( otherPort ) );
    client->SetAttribute( "useLimitedBroadcast", CORE::CString( "false" ) );
    client->SetAttribute( "useDirectedBroadcasts", CORE::CString( "false" ) );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
MakeFileUri( const CORE::CString& path )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return "file:///" + path.ReplaceChar( '\\', '/' );
    #else
    return "file://" + path;
    #endif
}

/*-------------------------------------------------------------------------*/

void
PerformListingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING listing TESTS" );
    GUCEF_TESTFW_SUITE_SCOPE( "Listings" );

    MULTIPLAY::CMultiplayGlobal* global = MULTIPLAY::CMultiplayGlobal::Instance();
    TStaticListingProviderFactory staticFactory;
    global->GetListingProviderFactory().RegisterConcreteFactory( CStaticListingProvider::ProviderName, &staticFactory );
    TTestLobbyServiceFactory testServiceFactory;
    global->GetLobbyServiceFactory().RegisterConcreteFactory( CTestLobbyService::ServiceName, &testServiceFactory );

    GUCEF_TESTFW_TESTCASE( "Test: A listing survives a data tree and the attributes of a LAN entry" )
        try
        {
            MULTIPLAY::CGameListingEntry listing = MakeListing( "id-1", "Game One", "udp://127.0.0.1:7777" );
            listing.kind = MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS;
            listing.passworded = true;
            listing.spectators = 2;
            listing.endpoints.push_back( "ws://127.0.0.1:7778/play" );
            listing.extra.Set( "mod", "big" );
            listing.pingMs = 33;

            CORE::CDataNode tree;
            ASSERT_TRUE( listing.Serialize( tree, CORE::CDataNodeSerializableSettings() ) );
            MULTIPLAY::CGameListingEntry copy;
            ASSERT_TRUE( copy.Deserialize( tree, CORE::CDataNodeSerializableSettings() ) );
            ASSERT_TRUE( copy.HasSameContent( listing ) && 33 == copy.pingMs );

            // The LAN attributes, and the extras next to them
            COMCORE::CDiscoveryEntry entry;
            MULTIPLAY::CLanListingProvider::ListingToEntry( listing, MULTIPLAY::CLanListingProvider::DefaultServiceType, entry );
            ASSERT_TRUE( "game" == entry.GetAttributes().GetValueAlways( "kind" ).AsString() );
            ASSERT_TRUE( "1" == entry.GetAttributes().GetValueAlways( "pw" ).AsString() );
            ASSERT_TRUE( "id-1" == entry.GetAttributes().GetValueAlways( "id" ).AsString() );
            MULTIPLAY::CGameListingEntry fromLan;
            ASSERT_TRUE( MULTIPLAY::CLanListingProvider::EntryToListing( entry, fromLan ) );
            ASSERT_TRUE( "id-1" == fromLan.listingId && "Game One" == fromLan.name && fromLan.passworded );
            ASSERT_TRUE( MULTIPLAY::CGameListingEntry::KIND_GAME_IN_PROGRESS == fromLan.kind );
            ASSERT_TRUE( 1 == fromLan.players && 4 == fromLan.maxPlayers && 2 == fromLan.spectators );
            ASSERT_TRUE( "dust" == fromLan.mapName && "ffa" == fromLan.modeName && "Host" == fromLan.hostName );
            ASSERT_TRUE( 2 == fromLan.endpoints.size() && "big" == fromLan.extra.GetValueAlways( "mod" ).AsString() );
            ASSERT_TRUE( 1 == fromLan.extra.GetKeyCount() );

            // Endpoints
            CORE::CString scheme, host, path;
            UInt16 port = 0;
            ASSERT_TRUE( MULTIPLAY::CGameListingEntry::ParseEndpoint( "ws://10.0.0.5:8080/lobby", scheme, host, port, path ) );
            ASSERT_TRUE( "ws" == scheme && "10.0.0.5" == host && 8080 == port && "/lobby" == path );
            ASSERT_FALSE( MULTIPLAY::CGameListingEntry::ParseEndpoint( "nonsense", scheme, host, port, path ) );
            ASSERT_TRUE( "tcp://h:1" == MULTIPLAY::CGameListingEntry::MakeEndpoint( "tcp", "h", 1 ) );
            ASSERT_TRUE( listing.SharesEndpointWith( fromLan ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The filter on text, kind, game, free places, password and ping" )
        try
        {
            MULTIPLAY::CGameListingEntry listing = MakeListing( "id-1", "Friday Night Game", "udp://127.0.0.1:7777" );
            MULTIPLAY::CGameListingFilter filter;
            ASSERT_TRUE( filter.Matches( listing ) );
            filter.text = "friday";
            ASSERT_TRUE( filter.Matches( listing ) );
            filter.text = "DUST";                                   // the map counts too
            ASSERT_TRUE( filter.Matches( listing ) );
            filter.text = "monday";
            ASSERT_FALSE( filter.Matches( listing ) );
            filter.text.Clear();

            filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_GAME_IN_PROGRESS;
            ASSERT_FALSE( filter.Matches( listing ) );
            filter.kind = MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY;
            ASSERT_TRUE( filter.Matches( listing ) );

            filter.gameId = "testgame";
            filter.gameVersion = "2.0";
            ASSERT_FALSE( filter.Matches( listing ) );
            filter.gameVersion = "1.0";
            ASSERT_TRUE( filter.Matches( listing ) );

            filter.notFull = true;
            listing.players = 4;
            ASSERT_FALSE( filter.Matches( listing ) );
            listing.players = 3;
            filter.noPassword = true;
            listing.passworded = true;
            ASSERT_FALSE( filter.Matches( listing ) );
            listing.passworded = false;

            filter.maxPingMs = 100;
            ASSERT_TRUE( filter.Matches( listing ) );                // unknown passes
            listing.pingMs = 150;
            ASSERT_FALSE( filter.Matches( listing ) );
            listing.pingMs = 50;
            ASSERT_TRUE( filter.Matches( listing ) );

            CORE::CDataNode config;
            ASSERT_TRUE( filter.SaveConfig( config ) );
            MULTIPLAY::CGameListingFilter loaded;
            ASSERT_TRUE( loaded.LoadConfig( config ) );
            ASSERT_TRUE( loaded.notFull && loaded.noPassword && 100 == loaded.maxPingMs && "testgame" == loaded.gameId );
            ASSERT_TRUE( MULTIPLAY::CGameListingFilter::KINDFILTER_LOBBY == loaded.kind );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A game advertised on the LAN is listed by the lan provider on loopback, updated and withdrawn" )
        try
        {
            CORE::CDataNode advertiserConfig;
            AddLoopbackClientConfig( advertiserConfig, LanPortA, LanPortB, "advertiser" );
            advertiserConfig.SetAttribute( "ttlInSecs", CORE::CString( "30" ) );
            MULTIPLAY::CGameAdvertiser advertiser;
            ASSERT_TRUE( advertiser.LoadConfig( advertiserConfig ) );

            CORE::CDataNode providerConfig;
            AddLoopbackClientConfig( providerConfig, LanPortB, LanPortA, "browser" );
            MULTIPLAY::CIGameListingProviderPtr provider = MULTIPLAY::CMultiplayGlobal::Instance()->GetListingProviderFactory().Create( "lan" );
            REQUIRE_TRUE( !provider.IsNULL() );
            ASSERT_TRUE( provider->LoadConfig( providerConfig ) );
            CListingRecorder recorder;
            recorder.SubscribeTo( provider.GetPointerAlways() );

            MULTIPLAY::CGameListingEntry listing = MakeListing( "", "LAN Game", "udp://127.0.0.1:7777" );
            listing.extra.Set( "mod", "big" );
            ASSERT_TRUE( advertiser.Advertise( listing ) );
            ASSERT_TRUE( advertiser.IsAdvertising() && !advertiser.GetListing().listingId.IsNULLOrEmpty() );
            ASSERT_TRUE( 1 == advertiser.GetDiscoveryClients().size() );
            PumpFor( 50 );

            ASSERT_TRUE( provider->Start( MULTIPLAY::CGameListingFilter() ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.added, 1, 5000 ) );
            MULTIPLAY::CGameListingEntry found;
            ASSERT_TRUE( provider->GetListing( advertiser.GetListing().listingId, found ) );
            ASSERT_TRUE( "LAN Game" == found.name && "testgame" == found.gameId && "lan" == found.backendName );
            ASSERT_TRUE( 1 == found.endpoints.size() && "udp://127.0.0.1:7777" == found.endpoints[ 0 ] );
            ASSERT_TRUE( "big" == found.extra.GetValueAlways( "mod" ).AsString() );

            ASSERT_TRUE( advertiser.UpdatePlayers( 3, 1 ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.updated, 1, 5000 ) );
            ASSERT_TRUE( provider->GetListing( advertiser.GetListing().listingId, found ) );
            ASSERT_TRUE( 3 == found.players && 1 == found.spectators );

            ASSERT_TRUE( advertiser.Withdraw() );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.removed, 1, 5000 ) );
            ASSERT_TRUE( 0 == provider->GetListingCount() );
            provider->Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The http-list provider lists a server list file and drops what it no longer has" )
        GUCEF_TESTFW_SKIP_IF( !XmlCodecIsAvailable(), "The xml codec plugin is not available" );
        try
        {
            CORE::CString workDir = CORE::CombinePath( CORE::RelativePath( "$CURWORKDIR$" ), "multiplaytemp" );
            CORE::CreateDirs( workDir );
            CORE::CString listPath = CORE::CombinePath( workDir, "listings.xml" );
            REQUIRE_TRUE( CORE::WriteStringAsTextFile( listPath,
                "<ServerList>"
                    "<ServerEntry Name=\"Alpha\" Address=\"10.1.2.4\" Port=\"4000\" game=\"testgame\" map=\"desert\" pl=\"2\" max=\"8\"/>"
                    "<ServerEntry Name=\"Beta\" Address=\"10.1.2.3\" Port=\"4001\"/>"
                "</ServerList>" ) );

            CORE::CDataNode config;
            config.SetAttribute( "url", MakeFileUri( listPath ) );
            config.SetAttribute( "codec", CORE::CString( "xml" ) );
            config.SetAttribute( "endpointScheme", CORE::CString( "tcp" ) );
            MULTIPLAY::CIGameListingProviderPtr provider = MULTIPLAY::CMultiplayGlobal::Instance()->GetListingProviderFactory().Create( "http-list" );
            REQUIRE_TRUE( !provider.IsNULL() );
            ASSERT_TRUE( provider->LoadConfig( config ) );
            CListingRecorder recorder;
            recorder.SubscribeTo( provider.GetPointerAlways() );

            ASSERT_TRUE( provider->Start( MULTIPLAY::CGameListingFilter() ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.added, 2, 10000 ) );
            MULTIPLAY::CGameListingEntry alpha;
            ASSERT_TRUE( provider->GetListing( "http:10.1.2.4:4000", alpha ) );
            ASSERT_TRUE( "Alpha" == alpha.name && "testgame" == alpha.gameId && "desert" == alpha.mapName );
            ASSERT_TRUE( 2 == alpha.players && 8 == alpha.maxPlayers && "http-list" == alpha.backendName );
            ASSERT_TRUE( 1 == alpha.endpoints.size() && "tcp://10.1.2.4:4000" == alpha.endpoints[ 0 ] );
            ASSERT_TRUE( "desert" == alpha.extra.GetValueAlways( "map" ).AsString() );

            REQUIRE_TRUE( CORE::WriteStringAsTextFile( listPath,
                "<ServerList><ServerEntry Name=\"Beta\" Address=\"10.1.2.3\" Port=\"4001\"/></ServerList>" ) );
            ASSERT_TRUE( provider->Refresh() );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.removed, 1, 10000 ) );
            ASSERT_TRUE( 1 == provider->GetListingCount() );
            ASSERT_FALSE( provider->GetListing( "http:10.1.2.4:4000", alpha ) );

            // A list that is not there is an error, not a crash
            MULTIPLAY::CHttpListListingProvider missing;
            CListingRecorder missingRecorder;
            missingRecorder.SubscribeTo( &missing );
            missing.SetUrl( MakeFileUri( CORE::CombinePath( workDir, "missing.xml" ) ) );
            ASSERT_TRUE( missing.Start( MULTIPLAY::CGameListingFilter() ) );
            ASSERT_TRUE( PumpUntilAtLeast( missingRecorder.errors, 1, 10000 ) );
            provider->Stop();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The lobby-service provider lists the lobbies of a service and what an advertiser publishes there" )
        try
        {
            MULTIPLAY::CMultiplayGlobal* multiplay = MULTIPLAY::CMultiplayGlobal::Instance();
            MULTIPLAY::CILobbyServicePtr service = multiplay->GetLobbyServiceFactory().Create( CTestLobbyService::ServiceName );
            REQUIRE_TRUE( !service.IsNULL() );
            MULTIPLAY::CPlayerIdentity identity;
            identity.userId = "host-id";
            identity.displayName = "Host";
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( service->Connect( identity, future ) );

            MULTIPLAY::CLobbyCreateParams params;
            params.name = "Service lobby";
            params.gameId = "testgame";
            params.maxPlayers = 4;
            ASSERT_TRUE( service->CreateLobby( params, future ) );
            MULTIPLAY::CILobbyPtr lobby;
            ASSERT_TRUE( MULTIPLAY::CILobbyService::TryGetLobbyFromFuture( future, lobby ) );

            MULTIPLAY::CGameAdvertiser advertiser;
            ASSERT_TRUE( advertiser.AddLobbyService( service ) );
            ASSERT_TRUE( advertiser.Advertise( MakeListing( "elsewhere-1", "Hosted elsewhere", "tcp://10.9.9.9:4000" ) ) );
            ASSERT_TRUE( advertiser.GetDiscoveryClients().empty() );     // the service replaces the LAN defaults

            MULTIPLAY::CIGameListingProviderPtr provider = multiplay->GetListingProviderFactory().Create( "lobby-service" );
            REQUIRE_TRUE( !provider.IsNULL() );
            static_cast< MULTIPLAY::CLobbyServiceListingProvider* >( provider.GetPointerAlways() )->SetLobbyService( service );
            static_cast< MULTIPLAY::CLobbyServiceListingProvider* >( provider.GetPointerAlways() )->SetRefreshIntervalInSecs( 0 );
            CListingRecorder recorder;
            recorder.SubscribeTo( provider.GetPointerAlways() );
            ASSERT_TRUE( provider->Start( MULTIPLAY::CGameListingFilter() ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.added, 2, 5000 ) );
            MULTIPLAY::CGameListingEntry found;
            ASSERT_TRUE( provider->GetListing( lobby->GetState().lobbyId, found ) && "Service lobby" == found.name );
            ASSERT_TRUE( provider->GetListing( "elsewhere-1", found ) && "tcp://10.9.9.9:4000" == found.endpoints[ 0 ] );

            ASSERT_TRUE( advertiser.Withdraw() );
            ASSERT_TRUE( provider->Refresh() );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.removed, 1, 5000 ) );
            ASSERT_TRUE( 1 == provider->GetListingCount() );

            // Made by name from config
            MULTIPLAY::CLobbyServiceListingProvider byName;
            CORE::CDataNode config;
            config.SetAttribute( "service", CTestLobbyService::ServiceName );
            ASSERT_TRUE( byName.LoadConfig( config ) );
            CListingRecorder byNameRecorder;
            byNameRecorder.SubscribeTo( &byName );
            ASSERT_TRUE( byName.Start( MULTIPLAY::CGameListingFilter() ) );
            ASSERT_FALSE( byName.GetLobbyService().IsNULL() );
            ASSERT_TRUE( 1 == byNameRecorder.errors );                 // that service is not connected
            byName.Stop();

            provider->Stop();
            lobby = MULTIPLAY::CILobbyPtr();
            CTestLobbyService::Reset();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The ping service measures with the first method that answers and falls back to the next" )
        try
        {
            COM::CUdpEchoResponder responder;
            REQUIRE_TRUE( responder.Start( EchoPort ) );

            MULTIPLAY::CPingService pings;
            CORE::CString::StringVector methods;
            methods.push_back( "no-such-method" );
            methods.push_back( "udp-echo" );
            pings.SetMethods( methods );
            pings.SetIntervalInMs( 50 );
            pings.SetTimeoutInMs( 200 );
            pings.SetMaxFailuresPerMethod( 2 );
            CPingRecorder recorder;
            recorder.SubscribeTo( &pings );

            CORE::CString target = "udp://127.0.0.1:" + CORE::ToString( EchoPort );
            ASSERT_TRUE( pings.AddTarget( target ) );
            ASSERT_TRUE( pings.HasTarget( target ) );
            ASSERT_TRUE( "udp-echo" == pings.GetMethod( target ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.known, 3, 5000 ) );
            ASSERT_TRUE( "udp-echo" == recorder.last.method && target == recorder.last.target );
            ASSERT_TRUE( pings.GetRttMs( target ) >= 0 && pings.GetRttMs( target ) < 1000 );

            // The answers stop: the target becomes unknown, shown as nothing
            responder.Stop();
            ASSERT_TRUE( PumpUntilAtLeast( recorder.unknown, 1, 5000 ) );
            ASSERT_TRUE( -1 == pings.GetRttMs( target ) && pings.GetMethod( target ).IsNULLOrEmpty() );
            ASSERT_TRUE( pings.RemoveTarget( target ) && !pings.HasTarget( target ) );

            // A TCP listener with no UDP echo: udp-echo gives up, tcp-connect measures
            COMCORE::CTCPServerSocket tcpServer( false );
            REQUIRE_TRUE( tcpServer.ListenOnPort( TcpOnlyPort ) );
            methods.clear();
            methods.push_back( "udp-echo" );
            methods.push_back( "tcp-connect" );
            pings.SetMethods( methods );
            CORE::CString tcpTarget = "127.0.0.1:" + CORE::ToString( TcpOnlyPort );
            UInt32 knownBefore = recorder.known;
            ASSERT_TRUE( pings.AddTarget( tcpTarget ) );
            ASSERT_TRUE( PumpUntilAtLeast( recorder.known, knownBefore + 1, 10000 ) );
            ASSERT_TRUE( "tcp-connect" == pings.GetMethod( tcpTarget ) && "tcp-connect" == recorder.last.method );
            pings.RemoveAllTargets();
            tcpServer.Close();

            // Nothing answers at all: it stays unknown and nothing is reported
            methods.clear();
            methods.push_back( "udp-echo" );
            pings.SetMethods( methods );
            CORE::CString silent = "127.0.0.1:" + CORE::ToString( SilentPort );
            knownBefore = recorder.known;
            ASSERT_TRUE( pings.AddTarget( silent ) );
            PumpFor( 800 );
            ASSERT_TRUE( knownBefore == recorder.known && -1 == pings.GetRttMs( silent ) );
            ASSERT_FALSE( pings.AddTarget( "not a host" ) );

            // The config
            CORE::CDataNode config;
            config.SetAttribute( "methods", CORE::CString( "tcp-connect, icmp" ) );
            config.SetAttribute( "smoothing", CORE::CString( "0.5" ) );
            ASSERT_TRUE( pings.LoadConfig( config ) );
            ASSERT_TRUE( 2 == pings.GetMethods().size() && "icmp" == pings.GetMethods()[ 1 ] );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The browser merges by id and by endpoint, filters, and pings the listings" )
        try
        {
            COM::CUdpEchoResponder responder;
            REQUIRE_TRUE( responder.Start( EchoPort ) );

            MULTIPLAY::CGameBrowser browser;
            CORE::CDataNode browserConfig;
            CORE::CDataNode* pingConfig = browserConfig.AddChild( "PingService", GUCEF_DATATYPE_OBJECT );
            pingConfig->SetAttribute( "methods", CORE::CString( "udp-echo" ) );
            pingConfig->SetAttribute( "intervalInMs", CORE::CString( "50" ) );
            pingConfig->SetAttribute( "timeoutInMs", CORE::CString( "300" ) );
            CORE::CDataNode* first = browserConfig.AddChild( "ListingProvider", GUCEF_DATATYPE_OBJECT );
            first->SetAttribute( "type", CStaticListingProvider::ProviderName );
            CORE::CDataNode* second = browserConfig.AddChild( "ListingProvider", GUCEF_DATATYPE_OBJECT );
            second->SetAttribute( "type", CStaticListingProvider::ProviderName );
            ASSERT_TRUE( browser.LoadConfig( browserConfig ) );
            REQUIRE_TRUE( 2 == browser.GetProviders().size() );
            MULTIPLAY::CGameBrowser::TProviderVector providers = browser.GetProviders();
            CStaticListingProvider* lan = static_cast< CStaticListingProvider* >( providers[ 0 ].GetPointerAlways() );
            CStaticListingProvider* internet = static_cast< CStaticListingProvider* >( providers[ 1 ].GetPointerAlways() );

            CBrowserRecorder recorder;
            recorder.SubscribeTo( &browser );
            ASSERT_TRUE( browser.Start() );

            CORE::CString echoEndpoint = "udp://127.0.0.1:" + CORE::ToString( EchoPort );
            lan->Put( MakeListing( "x", "Echo game", echoEndpoint.C_String() ) );
            lan->Put( MakeListing( "y", "Second game", "udp://127.0.0.1:1" ) );
            internet->Put( MakeListing( "x-on-the-internet", "Echo game seen twice", echoEndpoint.C_String() ) );   // the same endpoint
            internet->Put( MakeListing( "y", "Second game again", GUCEF_NULL ) );                                    // the same id
            MULTIPLAY::CGameListingEntry locked = MakeListing( "z", "Locked game", "udp://127.0.0.1:2" );
            locked.passworded = true;
            internet->Put( locked );

            ASSERT_TRUE( 3 == browser.GetListingCount() );
            MULTIPLAY::CGameListingEntry merged;
            ASSERT_TRUE( browser.GetListing( "x", merged ) && "Echo game" == merged.name );   // the first provider gives the content
            ASSERT_FALSE( browser.GetListing( "x-on-the-internet", merged ) );
            ASSERT_TRUE( browser.GetListing( "y", merged ) && "Second game" == merged.name );

            // The ping of the echo game arrives
            ASSERT_TRUE( PumpUntilAtLeast( recorder.updated, 1, 5000 ) );
            ASSERT_TRUE( browser.GetListing( "x", merged ) && merged.pingMs >= 0 );

            // Filters
            MULTIPLAY::CGameListingFilter filter;
            filter.noPassword = true;
            browser.SetFilter( filter );
            ASSERT_TRUE( 2 == browser.GetListingCount() && 1 <= recorder.removed );
            filter.text = "echo";
            browser.SetFilter( filter );
            ASSERT_TRUE( 1 == browser.GetListingCount() );
            browser.SetFilter( MULTIPLAY::CGameListingFilter() );
            ASSERT_TRUE( 3 == browser.GetListingCount() );

            // A listing gone from the first provider is still listed by the second
            UInt32 removedBefore = recorder.removed;
            lan->Drop( "x" );
            ASSERT_TRUE( browser.GetListing( "x-on-the-internet", merged ) && 3 == browser.GetListingCount() );
            internet->Drop( "x-on-the-internet" );
            ASSERT_TRUE( 2 == browser.GetListingCount() && recorder.removed > removedBefore );

            browser.Stop();
            ASSERT_TRUE( 0 == browser.GetListingCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CTestLobbyService::Reset();
    global->GetLobbyServiceFactory().UnregisterConcreteFactory( CTestLobbyService::ServiceName );
    global->GetListingProviderFactory().UnregisterConcreteFactory( CStaticListingProvider::ProviderName );
}

/*-------------------------------------------------------------------------*/
