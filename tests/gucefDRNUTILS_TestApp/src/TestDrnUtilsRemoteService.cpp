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

#include <string.h>

#include "TestDrnUtilsRemoteService.h"

#ifndef GUCEF__TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF__TEST_FRAMEWORK_H
#endif /* GUCEF__TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_GUCEFCORE_H
#endif /* GUCEF_GUCEFCORE_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CSERVICERESOURCEWATCH_H
#include "gucefCORE_CServiceResourceWatch.h"
#define GUCEF_CORE_CSERVICERESOURCEWATCH_H
#endif /* GUCEF_CORE_CSERVICERESOURCEWATCH_H ? */

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
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/** The connection with its events collected */
class CTestClient
{
    public:

    DRNUTILS::CRemoteServiceConnection connection;
    CEventCollector events;

    CTestClient( const CORE::PulseGeneratorPtr& pulse, const DRNUTILS::CRemoteServiceConnectionSettings& settings )
        : connection( pulse, settings ), events()
    {
        connection.Subscribe( &events );
    }

    ~CTestClient()
    {
        connection.Unsubscribe( &events );
    }
};

/*-------------------------------------------------------------------------*/

COMCORE::CRpcMessage
ApplyRequest( DRNUTILS::CRemoteServiceConnection& connection, const char* commandId, const char* value )
{GUCEF_TRACE;

    COMCORE::CRpcMessage request = connection.NewRequest( "gucef.drnutils.test.remote.Commands/Apply" );
    request.SetString( 1, commandId );
    request.SetString( 2, value );
    return request;
}

/*-------------------------------------------------------------------------*/

/** Publishes n events of a kind to a session and pumps until the client has seen them all (or the time is up) */
void
PublishEvents( CRemoteWorld& world, CTestClient& client, UInt32 count )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<count; ++i )
    {
        COMCORE::CRpcMessage event = world.host->NewEvent( "TEST" );
        event.Add( 4, CORE::CVariant( CORE::ToString( i ) ) );
        world.host->PublishEvent( client.connection.GetSessionId(), event );
    }
}

/*-------------------------------------------------------------------------*/

class CEventCount : public CCondition
{
    public:

    const CEventCollector& events;
    UInt32 count;

    CEventCount( const CEventCollector& e, UInt32 c ) : events( e ), count( c ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return events.sequences.size() >= count; }
};

/*-------------------------------------------------------------------------*/

/** A blocking call through the async form, pumped until it ends */
COMCORE::CRpcCallResult
CallPumped( DRNUTILS::CRemoteServiceConnection& connection, const CORE::CString& path, const COMCORE::CRpcMessage& request, UInt32 channel = 0, UInt32 timeoutMs = 10000 )
{GUCEF_TRACE;

    COMCORE::CRpcCallResult result;
    CORE::CFutureResult future = connection.CallAsync( path, request, COMCORE::CRpcCallOptions(), channel );
    CFutureReady ready( future );
    if ( !PumpUntil( ready, timeoutMs ) )
    {
        result.status.Set( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, "test timeout" );
        return result;
    }
    COMCORE::CRpcEndpoint::TryGetCallResult( future, result );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
SessionIdOf( DRNUTILS::CRemoteServiceConnection& connection, UInt8* sid )
{GUCEF_TRACE;

    COMCORE::CRpcEndpointPtr endpoint = connection.GetEndpoint( 0 );
    return !endpoint.IsNULL() && endpoint->GetSessionId( sid );
}

/*-------------------------------------------------------------------------*/

CORE::CString
Sha256Hex( const CORE::CDynamicBuffer& data )
{GUCEF_TRACE;

    UInt8 digest[ 32 ];
    DRN::CDRNCrypto::Sha256( data.GetConstBufferPtr(), data.GetDataSize(), digest );
    return DRN::CDRNCrypto::ToHex( digest, 32 ).Lowercase();
}

/*-------------------------------------------------------------------------*/

/** The fault-injection results, written to the log in one block at the end of the suite */
CORE::CString::StringVector g_faultLines;

void
NoteFault( const CORE::CString& line )
{GUCEF_TRACE;

    g_faultLines.push_back( line );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteService fault injection: " << line );
}

/*-------------------------------------------------------------------------*/

bool
LoadJsonFile( const CORE::CString& path, CORE::CDataNode& tree )
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !RpcTestJsonCodecAvailable() || !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) || codec.IsNULL() )
        return false;
    tree.Clear();
    return codec->BuildDataTree( &tree, path );
}

}

/*-------------------------------------------------------------------------*/

void
PerformDrnUtilsRemoteServiceTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "DrnUtilsRemoteService" );

    GUCEF_TESTFW_TESTCASE( "RS-T0 the contracts of the kit: built in code, registered, equal to their JSON authoring sources" )
    try
    {
        COMCORE::CRpcContractPtr session = DRNUTILS::CRemoteSessionContracts::GetContract();
        COMCORE::CRpcContractPtr devices = DRNUTILS::CDeviceContracts::GetContract();
        REQUIRE_TRUE( !session.IsNULL() );
        REQUIRE_TRUE( !devices.IsNULL() );
        ASSERT_TRUE( GUCEF_NULL != session->FindType( "Event" ) );
        GUCEF_TESTFW_REQUIRE_ENV( rpcDirEnv, "GUCEF_DRNUTILS_RPC_DIR" );
        GUCEF_TESTFW_SKIP_IF( !RpcTestJsonCodecAvailable(), "the JSON codec plugin is not available" );
        const char* files[ 2 ] = { "gucef.drnutils.session.json", "gucef.drnutils.devices.json" };
        COMCORE::CRpcContractPtr built[ 2 ] = { session, devices };
        for ( UInt32 i=0; i<2; ++i )
        {
            CORE::CDataNode tree;
            REQUIRE_TRUE( LoadJsonFile( CORE::CombinePath( rpcDirEnv.c_str(), files[ i ] ), tree ) );
            CORE::CString error;
            COMCORE::CRpcContractPtr authored = COMCORE::CRpcContract::LoadFromDataNode( tree, error );
            if ( authored.IsNULL() )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RS-T0: " << files[ i ] << ": " << error );
            REQUIRE_TRUE( !authored.IsNULL() );
            if ( !( authored->GetHash() == built[ i ]->GetHash() ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RS-T0: " << files[ i ] << " hash " << authored->GetHash().ToHexString() << " built " << built[ i ]->GetHash().ToHexString() );
            ASSERT_TRUE( authored->GetHash() == built[ i ]->GetHash() );
        }
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T1 pairing over loopback with the one-time code: the device is registered and active, the credentials pin the static key, Hello opens a session" )
    try
    {
        CRemoteWorld world( "t1" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        bool paired = world.Pair( "viewer", "viewer", credentials, error );
        if ( !paired )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RS-T1: " << error );
        REQUIRE_TRUE( paired );
        ASSERT_TRUE( credentials.IsComplete() );
        ASSERT_TRUE( credentials.serverStaticKeyHex == world.host->GetStaticPublicKeyHex() );
        DRNUTILS::CDeviceRecord record;
        ASSERT_TRUE( world.host->GetDeviceRegistry().Find( credentials.deviceId, record ) );
        ASSERT_TRUE( record.IsActive() );
        ASSERT_TRUE( record.HasRole( "viewer" ) );
        ASSERT_TRUE( !world.host->IsPairingWindowOpen() );     // one pairing per window by default

        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        ASSERT_TRUE( !client.connection.GetSessionId().IsNULLOrEmpty() );
        ASSERT_TRUE( client.connection.GetDeviceId() == credentials.deviceId );
        ASSERT_TRUE( 1 == world.host->GetSessionCache().GetActiveCount() );
        client.connection.Close( true );
        PumpFor( 100 );
        ASSERT_TRUE( 0 == world.host->GetSessionCache().GetCount() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T2 wrong pairing codes: counted, the address banned after 3, the window closed after 5" )
    try
    {
        CRemoteWorld world( "t2" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CPairingWindowRequest window;
        window.deviceKind = "viewer";
        window.roles.insert( "viewer" );
        CORE::CString windowId, code, error;
        UInt64 expires = 0;
        REQUIRE_TRUE( world.host->OpenPairingWindow( window, "test", windowId, code, expires, error ) );
        CORE::CString wrong = "0000-0000-0000-0000";
        if ( wrong == code )
            wrong = "1111-1111-1111-1111";
        DRNUTILS::CDeviceCredentials credentials;
        DRNUTILS::CDevicePairingClient::TPairingFailure failure = DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_NONE;
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( !world.PairWithCode( wrong, credentials, error, &failure ) );
            ASSERT_TRUE( DRNUTILS::CDevicePairingClient::PAIRINGFAILURE_WRONG_CODE == failure );
        }
        PumpFor( 50 );
        ASSERT_TRUE( 3 == world.host->GetPairingHost()->GetFailedProofCount() );
        ASSERT_TRUE( world.host->GetPairingHost()->GetBans().IsBanned( DRN::DRN_BAN_ADDRESS, "127.0.0.1" ) );
        // banned: the right code no longer helps from this address
        ASSERT_TRUE( !world.PairWithCode( code, credentials, error, &failure ) );
        NoteFault( "three wrong pairing codes -> the address is banned from the pairing listener; the right code is then refused too" );

        // a window closes after maxFailedProofs (no ban in between)
        world.StopHost();
        world.settings.banAfterFailedProofs = 100;
        world.settings.maxFailedProofs = 5;
        REQUIRE_TRUE( world.StartHost() );
        REQUIRE_TRUE( world.host->OpenPairingWindow( window, "test", windowId, code, expires, error ) );
        for ( UInt32 i=0; i<5; ++i )
            world.PairWithCode( wrong, credentials, error, &failure );
        PumpFor( 50 );
        ASSERT_TRUE( !world.host->IsPairingWindowOpen() );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T3 a window that grants admin waits for the owner's fingerprint confirmation (ACCEPTED, then RESULT); a declined pairing leaves no device" )
    try
    {
        CRemoteWorld world( "t3" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        bool paired = world.Pair( "viewer", "viewer,admin", credentials, error );
        if ( !paired )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RS-T3: " << error );
        REQUIRE_TRUE( paired );
        DRNUTILS::CDeviceRecord record;
        ASSERT_TRUE( world.host->GetDeviceRegistry().Find( credentials.deviceId, record ) );
        ASSERT_TRUE( record.IsActive() && record.HasRole( "admin" ) );
        ASSERT_TRUE( 0 == world.host->GetPairingHost()->GetPendingCount() );

        // the admin device sees the devices; the window is closed
        DRNUTILS::CRemoteServiceConnectionSettings adminSettings = world.ClientSettings();
        adminSettings.wantedContracts.push_back( DRNUTILS::CDeviceContracts::ContractName );
        CTestClient admin( world.pulse, adminSettings );
        REQUIRE_TRUE( admin.connection.Connect( credentials, error ) );
        COMCORE::CRpcMessage list = admin.connection.NewRequest( DRNUTILS::CDeviceContracts::ListPath );
        COMCORE::CRpcCallResult result = CallPumped( admin.connection, DRNUTILS::CDeviceContracts::ListPath, list );
        ASSERT_TRUE( result.status.IsOk() );
        admin.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T4 Hello and the event stream: events arrive once and in order; a viewer is refused the device administration" )
    try
    {
        CRemoteWorld world( "t4" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        PumpFor( 100 );
        PublishEvents( world, client, 10 );
        CEventCount ten( client.events, 10 );
        ASSERT_TRUE( PumpUntil( ten, 5000 ) );
        ASSERT_TRUE( 10 == client.events.sequences.size() );
        ASSERT_TRUE( client.events.InOrderWithoutDuplicates() );
        ASSERT_TRUE( 10 == client.connection.GetLastEventSeq() );
        COMCORE::CRpcMessage list = client.connection.NewRequest( DRNUTILS::CDeviceContracts::ListPath );
        COMCORE::CRpcCallResult result = CallPumped( client.connection, DRNUTILS::CDeviceContracts::ListPath, list );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == result.status.code || COMCORE::RPC_STATUS_FAILED_PRECONDITION == result.status.code || COMCORE::RPC_STATUS_NOT_FOUND == result.status.code );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T5 case A: the proxy cuts the link; the client relinks, the same RPC session resumes, a pending call completes and no event is lost or repeated" )
    try
    {
        CRemoteWorld world( "t5" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        PumpFor( 100 );
        UInt8 before[ GUCEF_RPC_SESSIONID_SIZE ];
        REQUIRE_TRUE( SessionIdOf( client.connection, before ) );
        PublishEvents( world, client, 3 );
        CEventCount three( client.events, 3 );
        ASSERT_TRUE( PumpUntil( three, 3000 ) );

        COMCORE::CRpcMessage slow = client.connection.NewRequest( "gucef.drnutils.test.remote.Commands/Slow" );
        slow.SetUInt32( 1, 1500 );
        CORE::CFutureResult pending = client.connection.CallAsync( "gucef.drnutils.test.remote.Commands/Slow", slow );
        PumpFor( 100 );
        const UInt64 cutAt = RemoteTestNowMs();
        world.proxy.Cut();
        PublishEvents( world, client, 4 );      // written while the link is down
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_RECONNECTING, 3000 ) );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 10000 ) );
        const UInt64 backAt = RemoteTestNowMs();
        CFutureReady ready( pending );
        ASSERT_TRUE( PumpUntil( ready, 5000 ) );
        COMCORE::CRpcCallResult result;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( pending, result ) && result.status.IsOk() );
        CEventCount seven( client.events, 7 );
        ASSERT_TRUE( PumpUntil( seven, 5000 ) );
        ASSERT_TRUE( 7 == client.events.sequences.size() );
        ASSERT_TRUE( client.events.InOrderWithoutDuplicates() );
        UInt8 after[ GUCEF_RPC_SESSIONID_SIZE ];
        REQUIRE_TRUE( SessionIdOf( client.connection, after ) );
        ASSERT_TRUE( 0 == memcmp( before, after, sizeof( before ) ) );
        ASSERT_TRUE( client.connection.GetRpcResumeCount() >= 1 );
        ASSERT_TRUE( 0 == client.connection.GetSessionReplaceCount() );
        NoteFault( "proxy cut mid-call -> case A: the same RPC session resumed after " + CORE::ToString( backAt - cutAt ) + " ms; the pending call completed, 7 of 7 events once and in order" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T6 black-holed path: the link timeout ends the link, reconnects wait, and the session comes back when the path does" )
    try
    {
        CRemoteWorld world( "t6" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        PumpFor( 200 );
        const UInt64 holeAt = RemoteTestNowMs();
        world.proxy.SetBlackHole( true );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_RECONNECTING, 6000 ) );
        const UInt64 detectedAt = RemoteTestNowMs();
        PumpFor( 1500 );
        ASSERT_TRUE( DRNUTILS::REMOTECONNECTION_CONNECTED != client.connection.GetState() );
        world.proxy.SetBlackHole( false );
        world.proxy.Cut();      // the held connections go; new ones pass
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 15000 ) );
        const UInt64 backAt = RemoteTestNowMs();
        ASSERT_TRUE( detectedAt - holeAt >= 1000 );
        NoteFault( "black-holed path -> the client noticed after " + CORE::ToString( detectedAt - holeAt ) + " ms (link timeout 1500 ms), connected again "
                   + CORE::ToString( backAt - detectedAt - 1500 ) + " ms after the path was restored; RPC resumes " + CORE::ToString( client.connection.GetRpcResumeCount() )
                   + ", new sessions " + CORE::ToString( client.connection.GetSessionReplaceCount() ) );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T7 case B and C: the RPC session is lost while the client session lives: Session/Resume replays the ring; beyond the ring RESUME_GAP and Resync" )
    try
    {
        CRemoteWorld world( "t7" );
        world.settings.eventRetentionEvents = 8;
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        DRNUTILS::CRemoteServiceConnectionSettings settings = world.ClientSettings();
        settings.resumeGraceMs = 1000;      // the client's RPC session gives up quickly; the service keeps the client session
        CTestClient client( world.pulse, settings );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        PumpFor( 100 );
        CORE::CString sessionId = client.connection.GetSessionId();

        // B: three events while away, within the ring
        world.proxy.SetRefuseNew( true );
        world.proxy.Cut();
        PublishEvents( world, client, 3 );
        PumpFor( 2500 );
        world.proxy.SetRefuseNew( false );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 15000 ) );
        CEventCount three( client.events, 3 );
        ASSERT_TRUE( PumpUntil( three, 5000 ) );
        ASSERT_TRUE( client.connection.GetSessionId() == sessionId );
        ASSERT_TRUE( client.connection.GetSessionResumeCount() >= 1 );
        ASSERT_TRUE( 3 == client.events.sequences.size() && client.events.InOrderWithoutDuplicates() );
        NoteFault( "RPC session lost (client grace 1 s, path refused 2.5 s) -> case B: Session/Resume of the same client session, 3 of 3 events replayed from the ring" );

        // C: twenty events while away, the ring holds eight
        world.proxy.SetRefuseNew( true );
        world.proxy.Cut();
        PumpFor( 200 );
        PublishEvents( world, client, 20 );
        PumpFor( 2300 );
        world.proxy.SetRefuseNew( false );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 15000 ) );
        class CResynced : public CCondition
        {
            public:
            CTestClient& c;
            explicit CResynced( CTestClient& client ) : c( client ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return c.events.resyncs > 0; }
        };
        CResynced resynced( client );
        ASSERT_TRUE( PumpUntil( resynced, 5000 ) );
        ASSERT_TRUE( 1 == client.connection.GetResyncCount() );
        ASSERT_TRUE( 23 == client.connection.GetLastEventSeq() );
        ASSERT_TRUE( client.events.InOrderWithoutDuplicates() );
        NoteFault( "20 events while away with a ring of 8 -> case C: RESUME_GAP, Resync, the stream again from the current sequence" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T8 command idempotency: a repeated command id answers the kept outcome (replayed), another request under it is COMMAND_ID_REUSED; case D: after a host restart a repeat is answered from the command journal" )
    try
    {
        CRemoteWorld world( "t8" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        const char* path = "gucef.drnutils.test.remote.Commands/Apply";
        COMCORE::CRpcCallResult first = CallPumped( client.connection, path, ApplyRequest( client.connection, "c-1", "alpha" ) );
        REQUIRE_TRUE( first.status.IsOk() );
        ASSERT_TRUE( !first.response.GetBool( 2 ) );
        const UInt64 revision = first.response.GetUInt64( 3 );
        COMCORE::CRpcCallResult again = CallPumped( client.connection, path, ApplyRequest( client.connection, "c-1", "alpha" ) );
        ASSERT_TRUE( again.status.IsOk() && again.response.GetBool( 2 ) && again.response.GetUInt64( 3 ) == revision );
        COMCORE::CRpcCallResult reused = CallPumped( client.connection, path, ApplyRequest( client.connection, "c-1", "beta" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == reused.status.code );
        ASSERT_TRUE( DRNUTILS::CRemoteServiceDetailCodes::CommandIdReused == DRNUTILS::CRemoteServiceDetailCodes::GetCode( reused.status.details ) );
        COMCORE::CRpcCallResult bad = CallPumped( client.connection, path, ApplyRequest( client.connection, "bad id!", "x" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_INVALID_ARGUMENT == bad.status.code );
        ASSERT_TRUE( 1 == world.service->applied );
        CORE::CString hostInstance = client.connection.GetHostInstanceId();
        CORE::CString sessionId = client.connection.GetSessionId();

        // case D: the service restarts; the client gets a new session and its repeat is answered from the journal
        world.StopHost();
        REQUIRE_TRUE( world.StartHost() );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_RECONNECTING, 8000 ) );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 20000 ) );
        ASSERT_TRUE( client.connection.GetHostInstanceId() != hostInstance );
        ASSERT_TRUE( client.connection.GetSessionId() != sessionId );
        ASSERT_TRUE( client.connection.GetSessionReplaceCount() >= 1 );
        ASSERT_TRUE( client.events.replaced >= 1 );
        COMCORE::CRpcCallResult afterRestart = CallPumped( client.connection, path, ApplyRequest( client.connection, "c-1", "alpha" ) );
        ASSERT_TRUE( afterRestart.status.IsOk() && afterRestart.response.GetBool( 2 ) && afterRestart.response.GetUInt64( 3 ) == revision );
        ASSERT_TRUE( 1 == world.service->replayedFromJournal );
        ASSERT_TRUE( 0 == world.service->applied );
        NoteFault( "host restarted (new host instance) -> case D: a new client session (SessionReplacedEvent), the repeated mutation answered from the command journal" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T9 upload staging over an attached bulk channel: blocks written over the bulk link, the link cut mid-upload, the rest sent from the received ranges, digests checked at the commit" )
    try
    {
        CRemoteWorld world( "t9" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings( 1 ) );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        class CBulkReady : public CCondition
        {
            public:
            CTestClient& c;
            explicit CBulkReady( CTestClient& client ) : c( client ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return c.connection.IsBulkChannelReady( 1 ); }
        };
        CBulkReady bulkReady( client );
        REQUIRE_TRUE( PumpUntil( bulkReady, 10000 ) );
        ASSERT_TRUE( 1 == world.host->GetSessionCache().GetAttachedCount() );

        // a file of 1 MiB and a small one; the second is announced with a wrong digest
        CORE::CDynamicBuffer big;
        big.SetDataSize( 1024 * 1024 );
        for ( UInt32 i=0; i<big.GetDataSize(); ++i )
            static_cast< UInt8* >( big.GetBufferPtr() )[ i ] = (UInt8) ( ( i * 7 ) & 0xFF );
        CORE::CDynamicBuffer small;
        small.CopyFrom( 11, "small file!" );
        COMCORE::CRpcMessage begin = client.connection.NewRequest( "gucef.drnutils.test.remote.Uploads/Begin" );
        begin.SetString( 1, "pack" );
        begin.Add( 2, CORE::CVariant( "data/big.bin" ) );
        begin.Add( 3, CORE::CVariant( (UInt64) big.GetDataSize() ) );
        begin.Add( 4, CORE::CVariant( Sha256Hex( big ) ) );
        begin.Add( 2, CORE::CVariant( "data/small.txt" ) );
        begin.Add( 3, CORE::CVariant( (UInt64) small.GetDataSize() ) );
        begin.Add( 4, CORE::CVariant( Sha256Hex( small ) ) );
        COMCORE::CRpcCallResult begun = CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Begin", begin );
        REQUIRE_TRUE( begun.status.IsOk() && begun.response.GetBool( 3 ) );
        const CORE::CString uploadId = begun.response.GetString( 1 );
        const UInt32 block = 64 * 1024;
        UInt32 written = 0;
        for ( UInt32 offset=0; offset<big.GetDataSize() / 2; offset += block )
        {
            COMCORE::CRpcMessage write = client.connection.NewRequest( "gucef.drnutils.test.remote.Uploads/Write", 1 );
            write.SetString( 1, uploadId );
            write.SetUInt32( 2, 0 );
            write.SetUInt64( 3, offset );
            write.SetBytes( 4, static_cast< const UInt8* >( big.GetConstBufferPtr() ) + offset, block );
            COMCORE::CRpcCallResult wrote = CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Write", write, 1 );
            ASSERT_TRUE( wrote.status.IsOk() && wrote.response.GetBool( 3 ) );
            ASSERT_TRUE( "bulk" == wrote.response.GetString( 6 ) );
            ++written;
        }
        world.proxy.Cut();
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 15000 ) );
        PumpFor( 200 );
        REQUIRE_TRUE( PumpUntil( bulkReady, 10000 ) );

        // what arrived, then the rest
        COMCORE::CRpcMessage query = client.connection.NewRequest( "gucef.drnutils.test.remote.Uploads/Query" );
        query.SetString( 1, uploadId );
        COMCORE::CRpcCallResult queried = CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Query", query );
        REQUIRE_TRUE( queried.status.IsOk() && queried.response.GetBool( 3 ) );
        DRNUTILS::CUploadRangeSet received;
        CORE::CVariant ranges;
        queried.response.GetAt( 5, 0, ranges );
        received.FromString( ranges.AsString() );
        DRNUTILS::CUploadRangeSet::TRangeVector missing;
        received.GetMissing( big.GetDataSize(), missing );
        ASSERT_TRUE( !missing.empty() );
        for ( UInt32 m=0; m<missing.size(); ++m )
        {
            for ( UInt64 offset=missing[ m ].offset; offset<missing[ m ].offset + missing[ m ].length; offset += block )
            {
                UInt64 length = missing[ m ].offset + missing[ m ].length - offset;
                if ( length > block )
                    length = block;
                COMCORE::CRpcMessage write = client.connection.NewRequest( "gucef.drnutils.test.remote.Uploads/Write", 1 );
                write.SetString( 1, uploadId );
                write.SetUInt32( 2, 0 );
                write.SetUInt64( 3, offset );
                write.SetBytes( 4, static_cast< const UInt8* >( big.GetConstBufferPtr() ) + offset, (UInt32) length );
                COMCORE::CRpcCallResult wrote = CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Write", write, 1 );
                ASSERT_TRUE( wrote.status.IsOk() && wrote.response.GetBool( 3 ) );
            }
        }
        // the small file with a byte changed: its digest does not match
        CORE::CDynamicBuffer tampered( small );
        static_cast< UInt8* >( tampered.GetBufferPtr() )[ 0 ] = 'S';
        COMCORE::CRpcMessage writeSmall = client.connection.NewRequest( "gucef.drnutils.test.remote.Uploads/Write", 1 );
        writeSmall.SetString( 1, uploadId );
        writeSmall.SetUInt32( 2, 1 );
        writeSmall.SetUInt64( 3, 0 );
        writeSmall.SetBytes( 4, tampered.GetConstBufferPtr(), tampered.GetDataSize() );
        ASSERT_TRUE( CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Write", writeSmall, 1 ).status.IsOk() );
        COMCORE::CRpcMessage commit = client.connection.NewRequest( "gucef.drnutils.test.remote.Uploads/Commit" );
        commit.SetString( 1, uploadId );
        COMCORE::CRpcCallResult committed = CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Commit", commit );
        ASSERT_TRUE( committed.status.IsOk() && !committed.response.GetBool( 3 ) );
        ASSERT_TRUE( DRNUTILS::CRemoteServiceDetailCodes::UploadDigestMismatch == committed.response.GetString( 2 ) );
        // sent again right
        writeSmall.SetBytes( 4, small.GetConstBufferPtr(), small.GetDataSize() );
        ASSERT_TRUE( CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Write", writeSmall, 1 ).status.IsOk() );
        committed = CallPumped( client.connection, "gucef.drnutils.test.remote.Uploads/Commit", commit );
        ASSERT_TRUE( committed.status.IsOk() && committed.response.GetBool( 3 ) );
        ASSERT_TRUE( 0 == world.service->controlWrites );
        ASSERT_TRUE( world.service->bulkWrites > written );
        NoteFault( "upload of 1 MiB over the bulk channel cut after " + CORE::ToString( written ) + " blocks -> both links back, the rest sent from Query's ranges ("
                   + CORE::ToString( (UInt32) missing.size() ) + " missing ranges), a tampered file refused UPLOAD_DIGEST_MISMATCH, then committed" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T10 drain: SERVICE_STOPPING with the hint, a new Hello refused DRAINING, the drain waits for a running call and ends when it is done, GOODBYE; the client waits for the hint and opens a new session at the restarted host" )
    try
    {
        // The drain time is long on purpose: the drain must end because the running call finished, not because time ran out
        CRemoteWorld world( "t10" );
        world.settings.drainSeconds = 60;
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );

        // a call that runs until the test lets it go (delayMs 0: held); the drain starts once the host has it
        COMCORE::CRpcMessage slow = client.connection.NewRequest( "gucef.drnutils.test.remote.Commands/Slow" );
        slow.SetUInt32( 1, 0 );
        CORE::CFutureResult running = client.connection.CallAsync( "gucef.drnutils.test.remote.Commands/Slow", slow );
        class CHeld : public CCondition
        {
            public:
            CRemoteWorld& w;
            explicit CHeld( CRemoteWorld& world ) : w( world ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return 1 == w.service->GetDelayedCount() && 1 == w.host->GetDrainHoldCount(); }
        };
        CHeld held( world );
        REQUIRE_TRUE( PumpUntil( held, 10000 ) );
        world.host->BeginDrain( "update", 2 );

        class CStopping : public CCondition
        {
            public:
            CTestClient& c;
            explicit CStopping( CTestClient& client ) : c( client ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
            {
                for ( UInt32 i=0; i<c.events.kinds.size(); ++i )
                    if ( c.events.kinds[ i ] == DRNUTILS::CRemoteSessionContracts::EventKindServiceStopping )
                        return true;
                return false;
            }
        };
        CStopping stopping( client );
        ASSERT_TRUE( PumpUntil( stopping, 10000 ) );
        COMCORE::CRpcCallResult refused = CallPumped( client.connection, "gucef.drnutils.test.remote.Commands/Apply", ApplyRequest( client.connection, "d-1", "x" ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_UNAVAILABLE == refused.status.code );
        ASSERT_TRUE( DRNUTILS::CRemoteServiceDetailCodes::Draining == DRNUTILS::CRemoteServiceDetailCodes::GetCode( refused.status.details ) );

        // the held call keeps the drain open: no GOODBYE, the connection stays up
        ASSERT_TRUE( !world.host->IsDrainComplete() );
        ASSERT_TRUE( !running.IsReady() );
        ASSERT_TRUE( DRNUTILS::REMOTECONNECTION_CONNECTED == client.connection.GetState() );

        // the call finishes: its answer arrives, then the drain ends at once (long before its drain time)
        const UInt64 releasedAt = RemoteTestNowMs();
        const UInt32 stateIndex = (UInt32) client.events.states.size();
        ASSERT_TRUE( 1 == world.service->ReleaseHeld() );
        CFutureReady runningReady( running );
        ASSERT_TRUE( PumpUntil( runningReady, 10000 ) );
        COMCORE::CRpcCallResult runningResult;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( running, runningResult ) && runningResult.status.IsOk() && runningResult.response.GetBool( 1 ) );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_RECONNECTING, 10000 ) );
        ASSERT_TRUE( world.host->IsDrainComplete() );
        UInt32 reconnectingIndex = 0;
        const UInt64 goneAt = client.events.StateEnteredAt( DRNUTILS::REMOTECONNECTION_RECONNECTING, stateIndex, &reconnectingIndex );
        ASSERT_TRUE( 0 != goneAt );
        ASSERT_TRUE( goneAt - releasedAt < (UInt64) world.settings.drainSeconds * 1000 / 2 );

        // the first attempt waits for the hint of SERVICE_STOPPING (2 s) plus its spread, counted from the transition
        UInt32 waited = client.connection.GetNextAttemptDelayMs();
        ASSERT_TRUE( waited >= 2000 );
        world.StopHost();
        REQUIRE_TRUE( world.StartHost() );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 30000 ) );
        const UInt64 backAt = client.events.StateEnteredAt( DRNUTILS::REMOTECONNECTION_CONNECTED, reconnectingIndex + 1 );
        ASSERT_TRUE( 0 != backAt );
        ASSERT_TRUE( backAt >= goneAt + 2000 );
        // one stop, told once: the event of the stream (the reconnect reached the restarted host, not the draining one)
        ASSERT_TRUE( 1 == client.events.stopping );
        NoteFault( "drain -> SERVICE_STOPPING (2 s), mutations refused DRAINING, the drain held by a running call and ended "
                   + CORE::ToString( goneAt - releasedAt ) + " ms after it finished, GOODBYE; the client waited " + CORE::ToString( waited )
                   + " ms (hint 2000 + jitter) and was back " + CORE::ToString( backAt - goneAt ) + " ms after the GOODBYE with a new session" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T10b drain time: a call that does not finish holds the drain until its drain time, then GOODBYE ends it" )
    try
    {
        CRemoteWorld world( "t10b" );
        world.settings.drainSeconds = 1;
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );

        COMCORE::CRpcMessage slow = client.connection.NewRequest( "gucef.drnutils.test.remote.Commands/Slow" );
        slow.SetUInt32( 1, 0 );
        CORE::CFutureResult running = client.connection.CallAsync( "gucef.drnutils.test.remote.Commands/Slow", slow );
        class CHeld : public CCondition
        {
            public:
            CRemoteWorld& w;
            explicit CHeld( CRemoteWorld& world ) : w( world ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return 1 == w.service->GetDelayedCount() && 1 == w.host->GetDrainHoldCount(); }
        };
        CHeld held( world );
        REQUIRE_TRUE( PumpUntil( held, 10000 ) );
        const UInt64 drainAt = RemoteTestNowMs();
        world.host->BeginDrain( "update", 2 );

        // never released: the drain ends at its drain time, not before
        CFutureReady runningReady( running );
        ASSERT_TRUE( PumpUntil( runningReady, 20000 ) );
        const UInt64 endedAt = RemoteTestNowMs();
        ASSERT_TRUE( world.host->IsDrainComplete() );
        ASSERT_TRUE( endedAt - drainAt >= 1000 );
        COMCORE::CRpcCallResult runningResult;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( running, runningResult ) && !runningResult.status.IsOk() );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_RECONNECTING, 10000 ) );
        NoteFault( "drain with a call that never finishes -> GOODBYE at the drain time (" + CORE::ToString( endedAt - drainAt ) + " ms), the call ended "
                   + runningResult.status.ToString() );
        client.connection.Close( false );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;


    GUCEF_TESTFW_TESTCASE( "RS-T15 drain with an event stream that cannot take the stop event yet: the drain waits until the stream wrote SERVICE_STOPPING, then ends the session" )
    try
    {
        // the stream of this client has used up its credit when the drain begins: the stop event waits in the ring. The
        // drain time is long, so the drain can only end because the event was written, not because time ran out
        CRemoteWorld world( "t15" );
        world.settings.drainSeconds = 60;
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        DRNUTILS::CRemoteServiceConnectionSettings clientSettings = world.ClientSettings();
        clientSettings.openEventStream = false;
        CTestClient client( world.pulse, clientSettings );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );

        COMCORE::CRpcMessage eventsRequest = client.connection.NewRequest( DRNUTILS::CRemoteSessionContracts::EventsPath );
        eventsRequest.SetUInt64( 1, 0 );
        COMCORE::CIRpcStreamPtr stream = client.connection.OpenStream( DRNUTILS::CRemoteSessionContracts::EventsPath, &eventsRequest );
        REQUIRE_TRUE( !stream.IsNULL() );
        stream->SetManualCredit( true );
        CStreamRecorder recorder;
        recorder.Watch( stream );
        PumpFor( 200 );

        // the 256 messages of the stream's initial credit, consumed without granting more
        const UInt32 initialCredit = 256;
        PublishEvents( world, client, initialCredit );
        CCounterAtLeast all( recorder.count, initialCredit );
        REQUIRE_TRUE( PumpUntil( all, 20000 ) );
        PumpFor( 200 );
        REQUIRE_TRUE( initialCredit == recorder.count );

        world.host->BeginDrain( "update", 2 );
        PumpFor( 1000 );
        // no credit: the stop event is not written yet, and the drain waits for it instead of ending the session
        ASSERT_TRUE( !( world.host->IsDrainComplete() ) );
        ASSERT_TRUE( !( recorder.ended ) );
        ASSERT_TRUE( initialCredit == recorder.count );

        // credit again: the stop event goes out, then the drain ends at once (long before its drain time)
        const UInt64 grantedAt = RemoteTestNowMs();
        stream->GrantCredit( 16 );
        class CStopSeen : public CCondition
        {
            public:
            CStreamRecorder& r;
            explicit CStopSeen( CStreamRecorder& recorder ) : r( recorder ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
            {
                for ( UInt32 i=0; i<r.messages.size(); ++i )
                    if ( r.messages[ i ].GetString( 2 ) == DRNUTILS::CRemoteSessionContracts::EventKindServiceStopping )
                        return true;
                return false;
            }
        };
        CStopSeen stopSeen( recorder );
        ASSERT_TRUE( PumpUntil( stopSeen, 10000 ) );
        class CDrained : public CCondition
        {
            public:
            CRemoteWorld& w;
            explicit CDrained( CRemoteWorld& world ) : w( world ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return w.host->IsDrainComplete(); }
        };
        CDrained drained( world );
        ASSERT_TRUE( PumpUntil( drained, 10000 ) );
        const UInt64 drainedAt = RemoteTestNowMs();
        ASSERT_TRUE( drainedAt - grantedAt < (UInt64) world.settings.drainSeconds * 1000 / 2 );
        NoteFault( "drain while the event stream had no credit -> the drain waited for the stop event; it was written "
                   "after the credit came and the drain ended " + CORE::ToString( drainedAt - grantedAt ) + " ms after the credit" );
        client.connection.Close( false );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T16 a client without an event stream is told of the drain by the refusal of its Hello: ServiceStoppingEvent once, also over repeated refusals" )
    try
    {
        CRemoteWorld world( "t16" );
        world.settings.drainSeconds = 60;
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        DRNUTILS::CRemoteServiceConnectionSettings clientSettings = world.ClientSettings();
        clientSettings.openEventStream = false;
        CTestClient client( world.pulse, clientSettings );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );

        // no event stream: nothing holds the drain, the session ends with GOODBYE and the client comes back to a host
        // that still drains (it is not stopped here), so its Hello is refused with DRAINING
        world.host->BeginDrain( "update", 1 );
        class CStopped : public CCondition
        {
            public:
            CTestClient& c;
            explicit CStopped( CTestClient& client ) : c( client ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return c.events.stopping > 0; }
        };
        CStopped stopped( client );
        ASSERT_TRUE( PumpUntil( stopped, 15000 ) );
        ASSERT_TRUE( world.host->IsDrainComplete() );
        ASSERT_TRUE( DRNUTILS::REMOTECONNECTION_RECONNECTING == client.connection.GetState() );

        // further refusals of the same drain are the same stop
        const UInt32 attempts = client.connection.GetReconnectAttemptCount();
        class CMoreAttempts : public CCondition
        {
            public:
            CTestClient& c;
            UInt32 since;
            CMoreAttempts( CTestClient& client, UInt32 attempts ) : c( client ), since( attempts ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return c.connection.GetReconnectAttemptCount() >= since + 2; }
        };
        CMoreAttempts more( client, attempts );
        ASSERT_TRUE( PumpUntil( more, 20000 ) );
        ASSERT_TRUE( 1 == client.events.stopping );

        // back at a host that serves: connected, and a later drain is told again
        world.StopHost();
        REQUIRE_TRUE( world.StartHost() );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 30000 ) );
        world.host->BeginDrain( "update", 1 );
        class CStoppedTwice : public CCondition
        {
            public:
            CTestClient& c;
            explicit CStoppedTwice( CTestClient& client ) : c( client ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return c.events.stopping >= 2; }
        };
        CStoppedTwice stoppedTwice( client );
        ASSERT_TRUE( PumpUntil( stoppedTwice, 15000 ) );
        NoteFault( "drain with a client without an event stream -> GOODBYE, its Hello refused DRAINING, ServiceStoppingEvent once over "
                   + CORE::ToString( client.connection.GetReconnectAttemptCount() - attempts ) + " further refusals, and again for the next drain" );
        client.connection.Close( false );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T11 a revoked device is refused although its token is unexpired; the client fails without a retry storm" )
    try
    {
        CRemoteWorld world( "t11" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        REQUIRE_TRUE( world.host->RevokeDevice( credentials.deviceId, "lost", "test", error ) );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_FAILED, 10000 ) );
        UInt32 attempts = client.connection.GetReconnectAttemptCount();
        PumpFor( 2000 );
        ASSERT_TRUE( client.connection.GetReconnectAttemptCount() == attempts );
        ASSERT_TRUE( 0 == world.host->GetSessionCache().GetCount() );

        // a new connection with the same unexpired token is refused at the link
        CTestClient again( world.pulse, world.ClientSettings() );
        ASSERT_TRUE( !again.connection.Connect( credentials, error ) );
        ASSERT_TRUE( DRNUTILS::REMOTECONNECTION_FAILED == again.connection.GetState() );
        NoteFault( "device revoked while connected -> links closed BANNED, the client FAILED after " + CORE::ToString( attempts ) + " attempts and made none after; a new connection is refused" );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T12 reconnect backoff: refused connections are retried with growing jittered delays within the cap, not in a tight loop" )
    try
    {
        CRemoteWorld world( "t12" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestClient client( world.pulse, world.ClientSettings() );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        UInt32 before = client.connection.GetReconnectAttemptCount();
        world.proxy.SetRefuseNew( true );
        world.proxy.Cut();
        UInt32 maxDelay = 0;
        const UInt64 startedAt = RemoteTestNowMs();
        while ( RemoteTestNowMs() - startedAt < 5000 )
        {
            PumpFor( 20 );
            UInt32 delay = client.connection.GetNextAttemptDelayMs();
            if ( delay > maxDelay )
                maxDelay = delay;
        }
        UInt32 attempts = client.connection.GetReconnectAttemptCount() - before;
        ASSERT_TRUE( attempts >= 2 );
        ASSERT_TRUE( attempts <= 25 );
        ASSERT_TRUE( maxDelay <= 2000 );
        world.proxy.SetRefuseNew( false );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 15000 ) );
        NoteFault( "service unreachable for 5 s -> " + CORE::ToString( attempts ) + " attempts, the longest wait " + CORE::ToString( maxDelay ) + " ms (cap 2000 in the test)" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T13 wake detection: a jump of the wall clock past the monotonic clock drops the link and reconnects at once" )
    try
    {
        CRemoteWorld world( "t13" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials;
        CORE::CString error;
        REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials, error ) );
        CTestShiftedUtcClock clock;
        CTestClient client( world.pulse, world.ClientSettings() );
        client.connection.SetClocks( &clock, GUCEF_NULL );
        REQUIRE_TRUE( client.connection.Connect( credentials, error ) );
        PumpFor( 100 );
        clock.offsetMs += 60000;
        const UInt64 at = RemoteTestNowMs();
        class CWoke : public CCondition
        {
            public:
            CTestClient& c;
            explicit CWoke( CTestClient& client ) : c( client ) {}
            virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE { return c.connection.GetWakeDetectionCount() > 0; }
        };
        CWoke woke( client );
        ASSERT_TRUE( PumpUntil( woke, 2000 ) );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_RECONNECTING, 3000 ) );
        ASSERT_TRUE( world.WaitFor( client.connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 10000 ) );
        NoteFault( "wall clock 60 s ahead of the monotonic clock (a sleep) -> links dropped and connected again in " + CORE::ToString( RemoteTestNowMs() - at ) + " ms" );
        client.connection.Close( true );
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "RS-T14 soak-lite: three clients through repeated cuts; sessions, rings, staging, threads and descriptors stay within their bounds" )
    try
    {
        CRemoteWorld world( "t14" );
        REQUIRE_TRUE( world.StartHost() );
        DRNUTILS::CDeviceCredentials credentials[ 3 ];
        CORE::CString error;
        for ( UInt32 i=0; i<3; ++i )
            REQUIRE_TRUE( world.Pair( "viewer", "viewer", credentials[ i ], error ) );
        CTestClient* clients[ 3 ];
        for ( UInt32 i=0; i<3; ++i )
        {
            clients[ i ] = GUCEF_NEW CTestClient( world.pulse, world.ClientSettings( 0 == i ? 1 : 0 ) );
            REQUIRE_TRUE( clients[ i ]->connection.Connect( credentials[ i ], error ) );
        }
        PumpFor( 300 );
        CORE::SServiceProcessResources first;
        CORE::CServiceResourceWatch::ReadProcessResources( first );
        const UInt32 cycles = 8;
        UInt32 recovered = 0;
        for ( UInt32 cycle=0; cycle<cycles; ++cycle )
        {
            for ( UInt32 i=0; i<3; ++i )
                PublishEvents( world, *clients[ i ], 5 );
            PumpFor( 100 );
            world.proxy.Cut();
            bool all = true;
            for ( UInt32 i=0; i<3; ++i )
                all = world.WaitFor( clients[ i ]->connection, DRNUTILS::REMOTECONNECTION_CONNECTED, 15000 ) && all;
            if ( all )
                ++recovered;
        }
        PumpFor( 500 );
        for ( UInt32 i=0; i<3; ++i )
            PublishEvents( world, *clients[ i ], 5 );
        for ( UInt32 i=0; i<3; ++i )
        {
            CEventCount all( clients[ i ]->events, 5 * ( cycles + 1 ) );
            if ( !PumpUntil( all, 10000 ) )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RS-T14: client " << i << " has " << clients[ i ]->events.sequences.size() << " events, last " << clients[ i ]->connection.GetLastEventSeq()
                                 << ", ring next " << world.host->GetRingState( clients[ i ]->connection.GetSessionId() ).nextEventSeq << ", state " << DRNUTILS::RemoteConnectionStateToString( clients[ i ]->connection.GetState() ) );
        }
        CORE::SServiceProcessResources last;
        CORE::CServiceResourceWatch::ReadProcessResources( last );
        ASSERT_TRUE( cycles == recovered );
        ASSERT_TRUE( 3 == world.host->GetSessionCache().GetCount() );
        UInt32 events = 0;
        for ( UInt32 i=0; i<3; ++i )
        {
            ASSERT_TRUE( clients[ i ]->events.InOrderWithoutDuplicates() );
            events += (UInt32) clients[ i ]->events.sequences.size();
        }
        ASSERT_TRUE( events == 3 * 5 * ( cycles + 1 ) );
        ASSERT_TRUE( world.host->GetRingState( clients[ 0 ]->connection.GetSessionId() ).retainedCount <= world.settings.eventRetentionEvents );
        ASSERT_TRUE( 0 == world.service->staging.GetOpenCount() );
        ASSERT_TRUE( last.threadCount <= first.threadCount + 4 );
        ASSERT_TRUE( last.openDescriptors <= first.openDescriptors + 16 );
        NoteFault( "soak-lite: 3 clients (one with a bulk channel), " + CORE::ToString( cycles ) + " cuts, " + CORE::ToString( recovered ) + " full recoveries, "
                   + CORE::ToString( events ) + " events once and in order; threads " + CORE::ToString( first.threadCount ) + " -> " + CORE::ToString( last.threadCount )
                   + ", descriptors " + CORE::ToString( first.openDescriptors ) + " -> " + CORE::ToString( last.openDescriptors ) + ", rss MB "
                   + CORE::ToString( (UInt32) ( first.rssBytes / ( 1024 * 1024 ) ) ) + " -> " + CORE::ToString( (UInt32) ( last.rssBytes / ( 1024 * 1024 ) ) ) );
        for ( UInt32 i=0; i<3; ++i )
        {
            clients[ i ]->connection.Close( true );
            GUCEF_DELETE clients[ i ];
        }
    }
    catch ( ::GUCEF::TESTFW::STestSkipRequest& )
    {
        throw;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    for ( UInt32 i=0; i<g_faultLines.size(); ++i )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RemoteService fault injection summary " << ( i+1 ) << ": " << g_faultLines[ i ] );
}

/*-------------------------------------------------------------------------*/
