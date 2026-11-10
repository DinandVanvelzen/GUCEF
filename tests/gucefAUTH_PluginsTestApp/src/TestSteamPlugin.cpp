/*
 *  gucefAUTH_PluginsTestApp: Application for testing the gucefAUTH plugins
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

#include <string.h>

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#include "authpluginSTEAM_CSteamAuthProvider.h"
#define GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_CSTEAMAUTHPROVIDER_H ? */

#ifndef GUCEF_AUTHPLUGIN_STEAM_H
#include "authpluginSTEAM.h"
#define GUCEF_AUTHPLUGIN_STEAM_H
#endif /* GUCEF_AUTHPLUGIN_STEAM_H ? */

#include "TestPluginsCommon.h"
#include "TestFakeSteam.h"
#include "TestFakeHttp.h"
#include "TestSteamPlugin.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::AUTHPLUGIN::STEAM;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const TestWebApiKey = "STEAMKEY-9f3c1d77e2b04a65-DO-NOT-LOG";

/** Settings in the form CAuthManager hands them over: the keys of the plugin in Parameters */
static CORE::CDataNode
MakeSteamNode( const char* identity, UInt32 intervalInMs = 5 )
{GUCEF_TRACE;

    CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
    CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    parameters->SetAttribute( "ticketIdentity", identity );
    parameters->SetAttribute( "callbackIntervalInMs", intervalInMs );
    return node;
}

/*-------------------------------------------------------------------------*/

static bool
Configure( CSteamAuthProvider& provider, const char* identity = "unit-test-service" )
{GUCEF_TRACE;

    return provider.LoadConfig( MakeSteamNode( identity ) );
}

/*-------------------------------------------------------------------------*/

//  A fake flat API of Steam: the functions of steam_api_flat.h that the runtime binds, as they behave

struct SFakeCallback
{
    Int32 id;
    GUCEF::vector< UInt8 > payload;
};

struct SFlatState
{
    bool running;
    Int32 initResult;
    UInt32 initCalls;
    UInt32 shutdownCalls;
    UInt32 frames;
    bool manualDispatchInit;
    bool wrongPipe;
    bool wrongSelf;
    UInt64 steamId;
    bool loggedOn;
    UInt32 nextTicket;
    GUCEF::vector< CORE::CString > identities;
    GUCEF::vector< UInt32 > cancelled;
    GUCEF::vector< SFakeCallback > queue;
    GUCEF::vector< SFakeCallback > dueNextFrame;
    UInt32 freed;
    bool withUserV023;
    bool withUserV019;
    bool withWebApi;
    bool withInitFlat;
    UInt32 beginResult;
    UInt64 lastBeginId;
    UInt64 lastEndId;
    UInt32 lastSessionArgs;
    char userMarker;
    char friendsMarker;

    SFlatState( void ) { Reset(); }

    void Reset( void )
    {
        running = true; initResult = 0; initCalls = 0; shutdownCalls = 0; frames = 0; manualDispatchInit = false; wrongPipe = false; wrongSelf = false;
        steamId = 76561198000000042ULL; loggedOn = true; nextTicket = 500; identities.clear(); cancelled.clear(); queue.clear(); dueNextFrame.clear(); freed = 0;
        withUserV023 = false; withUserV019 = true; withWebApi = true; withInitFlat = true; beginResult = 0; lastBeginId = 0; lastEndId = 0; lastSessionArgs = 0;
    }
};

static SFlatState g_flat;
static const Int32 FlatPipe = 77;

static bool FlatIsSteamRunning( void ) { return g_flat.running; }

static Int32 FlatInitFlat( char* message )
{
    ++g_flat.initCalls;
    if ( 0 != g_flat.initResult )
        strcpy( message, "fake: Steam says no" );
    return g_flat.initResult;
}

static void FlatShutdown( void ) { ++g_flat.shutdownCalls; }
static Int32 FlatGetPipe( void ) { return FlatPipe; }
static void FlatManualInit( void ) { g_flat.manualDispatchInit = true; }

static void FlatRunFrame( Int32 pipe )
{
    if ( pipe != FlatPipe )
        g_flat.wrongPipe = true;
    ++g_flat.frames;
    for ( size_t i=0; i<g_flat.dueNextFrame.size(); ++i )
        g_flat.queue.push_back( g_flat.dueNextFrame[ i ] );
    g_flat.dueNextFrame.clear();
}

static bool FlatGetNext( Int32 pipe, CSteamRuntime::TCallbackMsg* message )
{
    if ( pipe != FlatPipe )
        g_flat.wrongPipe = true;
    if ( g_flat.queue.empty() )
        return false;
    message->userHandle = 1;
    message->callbackId = g_flat.queue.front().id;
    message->param = g_flat.queue.front().payload.empty() ? GUCEF_NULL : &g_flat.queue.front().payload[ 0 ];
    message->paramSize = (Int32) g_flat.queue.front().payload.size();
    return true;
}

static void FlatFree( Int32 pipe )
{
    ++g_flat.freed;
    if ( !g_flat.queue.empty() )
        g_flat.queue.erase( g_flat.queue.begin() );
}

static void* FlatUser( void ) { return &g_flat.userMarker; }
static void* FlatFriends( void ) { return &g_flat.friendsMarker; }

static UInt64 FlatGetSteamId( void* user ) { if ( user != &g_flat.userMarker ) g_flat.wrongSelf = true; return g_flat.steamId; }
static bool FlatBLoggedOn( void* user ) { return g_flat.loggedOn; }

static UInt32 FlatWebApiTicket( void* user, const char* identity )
{
    if ( user != &g_flat.userMarker )
        g_flat.wrongSelf = true;
    g_flat.identities.push_back( identity );
    UInt32 handle = g_flat.nextTicket++;

    // Another callback comes first, which the runtime has to hand back and skip
    SFakeCallback noise;
    noise.id = 703;
    noise.payload.assign( 16, 0xAA );
    g_flat.dueNextFrame.push_back( noise );

    SFakeCallback answer;
    answer.id = CSteamRuntime::TicketForWebApiCallbackId;
    UInt8 ticket[ 8 ];
    UInt32 ticketSize = CFakeSteam::TicketBytes( handle, ticket );
    Int32 result = 1;
    Int32 size = (Int32) ticketSize;
    answer.payload.resize( 12 + ticketSize );
    memcpy( &answer.payload[ 0 ], &handle, 4 );
    memcpy( &answer.payload[ 4 ], &result, 4 );
    memcpy( &answer.payload[ 8 ], &size, 4 );
    memcpy( &answer.payload[ 12 ], ticket, ticketSize );
    g_flat.dueNextFrame.push_back( answer );
    return handle;
}

static UInt32 FlatSessionTicket( void* user, void* ticket, Int32 capacity, UInt32* size, const void* identity )
{
    g_flat.lastSessionArgs = 5;
    UInt32 handle = g_flat.nextTicket++;
    *size = CFakeSteam::TicketBytes( handle, static_cast< UInt8* >( ticket ) );
    return handle;
}

static void FlatCancel( void* user, UInt32 ticket ) { g_flat.cancelled.push_back( ticket ); }
static Int32 FlatBegin( void* user, const void* ticket, Int32 size, UInt64 id ) { g_flat.lastBeginId = id; return (Int32) g_flat.beginResult; }
static void FlatEnd( void* user, UInt64 id ) { g_flat.lastEndId = id; }
static const char* FlatPersona( void* friends ) { return "Flat Persona"; }

struct SSymbol
{
    const char* name;
    void* address;
};

static void*
ResolveFlat( void* context, const char* name )
{
    #define FLATSYM( n, f ) if ( 0 == strcmp( name, n ) ) return reinterpret_cast< void* >( &f );
    if ( g_flat.withInitFlat )
    {
        FLATSYM( "SteamAPI_InitFlat", FlatInitFlat )
    }
    FLATSYM( "SteamAPI_Shutdown", FlatShutdown )
    FLATSYM( "SteamAPI_IsSteamRunning", FlatIsSteamRunning )
    FLATSYM( "SteamAPI_GetHSteamPipe", FlatGetPipe )
    FLATSYM( "SteamAPI_ManualDispatch_Init", FlatManualInit )
    FLATSYM( "SteamAPI_ManualDispatch_RunFrame", FlatRunFrame )
    FLATSYM( "SteamAPI_ManualDispatch_GetNextCallback", FlatGetNext )
    FLATSYM( "SteamAPI_ManualDispatch_FreeLastCallback", FlatFree )
    if ( g_flat.withUserV023 )
    {
        FLATSYM( "SteamAPI_SteamUser_v023", FlatUser )
    }
    if ( g_flat.withUserV019 )
    {
        FLATSYM( "SteamAPI_SteamUser_v019", FlatUser )
    }
    FLATSYM( "SteamAPI_SteamFriends_v017", FlatFriends )
    FLATSYM( "SteamAPI_ISteamUser_GetSteamID", FlatGetSteamId )
    FLATSYM( "SteamAPI_ISteamUser_BLoggedOn", FlatBLoggedOn )
    if ( g_flat.withWebApi )
    {
        FLATSYM( "SteamAPI_ISteamUser_GetAuthTicketForWebApi", FlatWebApiTicket )
    }
    FLATSYM( "SteamAPI_ISteamUser_GetAuthSessionTicket", FlatSessionTicket )
    FLATSYM( "SteamAPI_ISteamUser_CancelAuthTicket", FlatCancel )
    FLATSYM( "SteamAPI_ISteamUser_BeginAuthSession", FlatBegin )
    FLATSYM( "SteamAPI_ISteamUser_EndAuthSession", FlatEnd )
    FLATSYM( "SteamAPI_ISteamFriends_GetPersonaName", FlatPersona )
    #undef FLATSYM
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Json( const char* steamId, const char* result )
{
    return CORE::CString( "{\"response\":{\"params\":{\"result\":\"" ) + result + "\",\"steamid\":\"" + steamId + "\",\"ownersteamid\":\"" + steamId + "\",\"vacbanned\":false,\"publisherbanned\":false}}}";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformSteamPluginTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Steam plugin TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "SteamPlugin" );

    GUCEF_TESTFW_TESTCASE( "Test: The provider is steam, with the login in the browser, silent and verifiable, and makes the accounts of the namespace of its descriptor" )
        try
        {
            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( provider.GetProviderName() == "steam" );
            ASSERT_TRUE( ( AUTH::AUTHCAP_BROWSER | AUTH::AUTHCAP_SILENT | AUTH::AUTHCAP_SERVER_VERIFIABLE ) == provider.GetCapabilities() );
            ASSERT_TRUE( provider.GetClassTypeName() == "GUCEF::AUTHPLUGIN::STEAM::CSteamAuthProvider" );

            // the descriptor of the plugin makes the reference for a SteamID64, which is what the login with the client makes too
            AUTH::CAuthProviderDescriptor descriptor = CSteamOpenId2Profile::MakeDescriptor();
            ASSERT_TRUE( descriptor.IsWellFormed() && "steam" == descriptor.name && "Steam" == descriptor.displayName && AUTH::AUTHPROTOCOL_OPENID2 == descriptor.protocol );
            ASSERT_TRUE( provider.GetDescriptor().name == descriptor.name && provider.GetDescriptor().accountProviderId == descriptor.accountProviderId );
            CORE::CString::StringMap claims;
            claims[ "steamid" ] = "76561198000000001";
            AUTH::CAccountRef fromDescriptor;
            ASSERT_TRUE( descriptor.MakeAccountRef( claims, CORE::CString::StringMap(), fromDescriptor ) );
            ASSERT_TRUE( fromDescriptor.ToString() == "steam:76561198000000001" );
            ASSERT_TRUE( fromDescriptor == AUTH::CAccountRef( CSteamOpenId2Profile::AccountNamespace, "76561198000000001" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Not available while Steam is not running or the identity is not set, and then not tried again before the retry interval" )
        try
        {
            CFakeSteam steam;
            steam.initOk = false;
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_FALSE( provider.IsClientAvailable() );
            ASSERT_TRUE( 1 == steam.initCalls );

            // The next question does not ask Steam again within the retry interval
            ASSERT_FALSE( provider.IsClientAvailable() );
            ASSERT_TRUE( 1 == steam.initCalls );

            // Steam started in the meantime: after the interval it is tried again
            CORE::CDataNode node = MakeSteamNode( "unit-test-service" );
            node.FindChild( "Parameters" )->SetAttribute( "retryIntervalInMs", (UInt32) 20 );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            steam.initOk = true;
            MT::ThreadDelay( 60 );
            ASSERT_TRUE( provider.IsClientAvailable() );
            ASSERT_TRUE( 2 == steam.initCalls );

            // No identity for the web api, nothing to ask a ticket for
            CFakeSteam steam2;
            CSteamAuthProvider noIdentity( steam2.MakeShim() );
            ASSERT_FALSE( noIdentity.IsClientAvailable() );
            // The session tickets do not need one
            CORE::CDataNode sessionNode( "steam", GUCEF_DATATYPE_OBJECT );
            sessionNode.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT )->SetAttribute( "ticketMode", "session" );
            ASSERT_TRUE( noIdentity.LoadConfig( sessionNode ) );
            ASSERT_TRUE( noIdentity.IsClientAvailable() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A silent login returns at once, ends with the account, the persona and the ticket as proof, and the events come on the pulse thread" )
        try
        {
            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( future.HasAFuture() );
            // The call did not wait for Steam
            ASSERT_FALSE( future.IsReady() );
            ASSERT_TRUE( 1 == provider.GetPendingTicketCount() );
            ASSERT_TRUE( 1 == steam.identities.size() && steam.identities[ 0 ] == "unit-test-service" );

            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "steam:76561198000000001" );
            ASSERT_TRUE( result.providerName == "steam" );
            ASSERT_TRUE( result.displayName == "Fake Persona" );
            ASSERT_TRUE( result.proof.kind == AUTH::CAuthProof::KindPlatformTicket );
            ASSERT_TRUE( result.proof.data.Reveal() == CFakeSteam::TicketHex( 100 ) );
            ASSERT_TRUE( result.proof.data.Reveal() == "1400deadbeef0164" );
            ASSERT_TRUE( 0 == provider.GetPendingTicketCount() );
            ASSERT_TRUE( 1 == provider.GetIssuedTicketCount() );

            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_SUCCEEDED, 1, 1000 ) );
            ASSERT_TRUE( 1 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );
            ASSERT_TRUE( recorder.records[ 0 ].threadId == MT::GetCurrentTaskID() );
            ASSERT_TRUE( recorder.records[ 0 ].result.accountRef == result.accountRef );

            // The timer stopped with the last ticket: no more frames are asked for
            UInt32 frames = steam.frames;
            PumpFor( 60 );
            ASSERT_TRUE( frames == steam.frames );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: BeginLogin is the same login without a browser" )
        try
        {
            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            AUTH::CAuthOptions options;
            options.preferBrowser = true;
            ASSERT_TRUE( provider.BeginLogin( options, future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "steam:76561198000000001" );
            ASSERT_TRUE( result.proof.kind == AUTH::CAuthProof::KindPlatformTicket );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Steam not running: the silent login does not start, the interactive one fails with a reason" )
        try
        {
            CFakeSteam steam;
            steam.initOk = false;
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( provider.TrySilentLogin( silent ) );
            ASSERT_FALSE( silent.HasAFuture() );
            PumpFor( 20 );
            ASSERT_TRUE( 0 == recorder.records.size() );

            CORE::CFutureResult interactive( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            AUTH::CAuthOptions options;
            ASSERT_TRUE( provider.BeginLogin( options, interactive ) );
            // The login has ended already, as failed: a future that has nothing to wait for
            ASSERT_TRUE( interactive.IsReady() && interactive.HasNoFuture() );
            ASSERT_TRUE( PumpUntilReady( interactive, 1000 ) );
            AUTH::CAuthResult none;
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( interactive, none ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            const CProviderRecorder::SRecord* failed = recorder.Last( CProviderRecorder::KIND_FAILED );
            ASSERT_TRUE( GUCEF_NULL != failed );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == failed->failure.reason );
            ASSERT_TRUE( failed->failure.message.HasSubstr( "Steam client is not running" ) >= 0 );

            // Steam without a connection to its servers, and a user nobody knows
            CFakeSteam offline;
            offline.loggedOn = false;
            CSteamAuthProvider offlineProvider( offline.MakeShim() );
            ASSERT_TRUE( Configure( offlineProvider ) );
            CORE::CFutureResult offlineFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( offlineProvider.TrySilentLogin( offlineFuture ) );
            CFakeSteam nobody;
            nobody.steamId = 0;
            CSteamAuthProvider nobodyProvider( nobody.MakeShim() );
            ASSERT_TRUE( Configure( nobodyProvider ) );
            ASSERT_FALSE( nobodyProvider.TrySilentLogin( offlineFuture ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A ticket that Steam refuses ends the login as failed, a lost connection as a network failure" )
        try
        {
            CFakeSteam steam;
            steam.answerResult = 2; // k_EResultFail
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            AUTH::CAuthResult none;
            ASSERT_FALSE( AUTH::CAuthResult::TryGetFromFuture( future, none ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_INTERNAL == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 0 == provider.GetIssuedTicketCount() );

            steam.answerResult = 3; // k_EResultNoConnection
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 2, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NETWORK == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );

            // Steam does not create the ticket at all
            CFakeSteam none2;
            CSteamAuthProvider session( none2.MakeShim() );
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT )->SetAttribute( "ticketMode", "session" );
            ASSERT_TRUE( session.LoadConfig( node ) );
            none2.sessionTicketFails = true;
            CProviderRecorder sessionRecorder;
            sessionRecorder.SubscribeTo( &session );
            CORE::CFutureResult sessionFuture( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( session.TrySilentLogin( sessionFuture ) );
            ASSERT_TRUE( sessionFuture.IsReady() );
            ASSERT_TRUE( sessionRecorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A ticket that does not come ends as a timeout and is cancelled in Steam" )
        try
        {
            CFakeSteam steam;
            steam.answerTickets = false;
            CSteamAuthProvider provider( steam.MakeShim() );
            CORE::CDataNode node = MakeSteamNode( "unit-test-service" );
            node.FindChild( "Parameters" )->SetAttribute( "ticketTimeoutInMs", (UInt32) 80 );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_TIMEOUT == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( steam.WasCancelled( 100 ) );
            ASSERT_TRUE( 0 == provider.GetPendingTicketCount() );
            ASSERT_TRUE( 0 == provider.GetIssuedTicketCount() );

            // The answer that comes too late is dropped
            steam.answerTickets = true;
            PumpFor( 40 );
            ASSERT_TRUE( 0 == recorder.Count( CProviderRecorder::KIND_SUCCEEDED ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cancelling the future ends the login as cancelled and cancels the ticket" )
        try
        {
            CFakeSteam steam;
            steam.answerTickets = false;
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( Configure( provider ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( future.RequestCancellation() );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_CANCELLED == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( steam.WasCancelled( 100 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Tickets are cancelled at logout, when a new one replaces them and when the provider goes" )
        try
        {
            CFakeSteam steam;
            AUTH::CAccountRef account( CSteamOpenId2Profile::AccountNamespace, "76561198000000001" );
            {
                CSteamAuthProvider provider( steam.MakeShim() );
                ASSERT_TRUE( Configure( provider ) );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( provider.TrySilentLogin( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
                ASSERT_TRUE( 1 == provider.GetIssuedTicketCount() );
                ASSERT_FALSE( steam.WasCancelled( 100 ) );

                ASSERT_TRUE( provider.Logout( account, AUTH::CSecretString() ) );
                ASSERT_TRUE( steam.WasCancelled( 100 ) );
                ASSERT_TRUE( 0 == provider.GetIssuedTicketCount() );
                // Logging out twice is fine
                ASSERT_TRUE( provider.Logout( account, AUTH::CSecretString() ) );
                ASSERT_TRUE( 1 == steam.cancelled.size() );

                // A second login of the same account replaces the first ticket
                ASSERT_TRUE( provider.TrySilentLogin( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
                ASSERT_TRUE( provider.TrySilentLogin( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
                ASSERT_TRUE( 1 == provider.GetIssuedTicketCount() );
                ASSERT_TRUE( steam.WasCancelled( 101 ) );
                ASSERT_FALSE( steam.WasCancelled( 102 ) );
                steam.shutdownCalls = 0;
            }
            // The provider that goes cancels what is left, and shuts the API down it started
            ASSERT_TRUE( steam.WasCancelled( 102 ) );
            ASSERT_TRUE( 1 == steam.shutdownCalls );

            // An application that has Steam in use for itself keeps it
            CFakeSteam shared;
            {
                CSteamAuthProvider provider( shared.MakeShim() );
                CORE::CDataNode node = MakeSteamNode( "unit-test-service" );
                node.FindChild( "Parameters" )->SetAttribute( "ownsSteamApi", false );
                ASSERT_TRUE( provider.LoadConfig( node ) );
                ASSERT_TRUE( provider.IsClientAvailable() );
            }
            ASSERT_TRUE( 0 == shared.shutdownCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Session tickets are used when the runtime has no ticket for the web api, or when the settings say so" )
        try
        {
            CFakeSteam steam;
            steam.hasWebApi = false;
            CSteamAuthProvider provider( steam.MakeShim() );
            // auto mode, no identity needed because there is no web api ticket
            ASSERT_TRUE( provider.IsClientAvailable() );
            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            // At once: the ticket is there when Steam returns
            ASSERT_TRUE( future.IsReady() );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.proof.data.Reveal() == CFakeSteam::TicketHex( 100 ) );
            ASSERT_TRUE( 0 == steam.identities.size() );

            // mode webapi on a runtime without it
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "ticketMode", "webapi" );
            parameters->SetAttribute( "ticketIdentity", "unit-test-service" );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_FALSE( provider.IsClientAvailable() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With callbacks dispatched by the application the plugin dispatches none and takes the answer from the application" )
        try
        {
            CFakeSteam steam;
            steam.answerTickets = false;
            CSteamAuthProvider provider( steam.MakeShim() );
            CORE::CDataNode node = MakeSteamNode( "unit-test-service" );
            node.FindChild( "Parameters" )->SetAttribute( "callbackMode", "external" );
            ASSERT_TRUE( provider.LoadConfig( node ) );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            PumpFor( 40 );
            ASSERT_TRUE( 0 == steam.frames );
            ASSERT_FALSE( future.IsReady() );

            // An answer for a ticket nobody asked for is not an error
            SSteamTicketResponse stray;
            stray.ticketHandle = 9999;
            stray.result = SteamResultOk;
            stray.ticketSize = CFakeSteam::TicketBytes( 9999, stray.ticket );
            provider.OnTicketResponse( stray );
            ASSERT_FALSE( future.IsReady() );

            SSteamTicketResponse answer;
            answer.ticketHandle = 100;
            answer.result = SteamResultOk;
            answer.ticketSize = CFakeSteam::TicketBytes( 100, answer.ticket );
            provider.OnTicketResponse( answer );
            ASSERT_TRUE( future.IsReady() );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            ASSERT_TRUE( result.proof.data.Reveal() == CFakeSteam::TicketHex( 100 ) );
            ASSERT_TRUE( 0 == steam.frames );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Settings: defaults, round trip with the secret, the form CAuthManager hands over, and modes that are not known are refused" )
        try
        {
            CSteamAuthSettings defaults;
            ASSERT_TRUE( defaults.ticketMode == "auto" && defaults.callbackMode == "manual" && defaults.verifyMode == "webapi" );
            ASSERT_TRUE( defaults.ownsSteamApi && !defaults.allowInsecureHttp );
            ASSERT_TRUE( defaults.webApiBaseUrl == "https://partner.steam-api.com" );
            ASSERT_TRUE( 10000 == defaults.ticketTimeoutInMs && 50 == defaults.callbackIntervalInMs && 5000 == defaults.retryIntervalInMs );
            ASSERT_TRUE( defaults.webApiKey.IsEmpty() && defaults.IsWellFormed() );

            CSteamAuthSettings filled;
            filled.runtimeLibrary = "/opt/steam/libsteam_api.so";
            filled.ticketIdentity = "my-service";
            filled.ticketMode = "session";
            filled.callbackMode = "external";
            filled.ownsSteamApi = false;
            filled.ticketTimeoutInMs = 1234;
            filled.callbackIntervalInMs = 7;
            filled.retryIntervalInMs = 99;
            filled.appId = "480";
            filled.webApiKey.Set( TestWebApiKey );
            filled.webApiKeyPlacement = "header";
            filled.webApiBaseUrl = "https://example.invalid";
            filled.webApiTimeoutInMs = 4321;
            filled.allowInsecureHttp = true;
            filled.verifyMode = "peer";

            CORE::CDataNode saved( "steam", GUCEF_DATATYPE_OBJECT );
            ASSERT_TRUE( filled.SaveConfig( saved ) );
            CSteamAuthSettings loaded;
            ASSERT_TRUE( loaded.LoadConfig( saved ) );
            ASSERT_TRUE( loaded == filled );
            ASSERT_TRUE( loaded.webApiKey.Reveal() == TestWebApiKey );
            ASSERT_TRUE( loaded.ticketIdentity == "my-service" );

            // As json, with the parameters nested the way the Auth section has them
            ASSERT_TRUE( JsonCodecAvailable() );
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            CORE::CDynamicBuffer buffer;
            CORE::CDynamicBufferAccess output( buffer );
            ASSERT_TRUE( codec->StoreDataTree( &saved, &output ) );
            CORE::CDynamicBufferAccess input( buffer );
            CORE::CDataNode parsed;
            ASSERT_TRUE( codec->BuildDataTree( &parsed, &input ) );
            CSteamAuthSettings fromJson;
            ASSERT_TRUE( fromJson.LoadConfig( parsed ) );
            ASSERT_TRUE( fromJson == filled );

            // The form the manager hands a provider: the node of CAuthProviderSettings with the plugin's keys in Parameters
            AUTH::CAuthSettings authSettings;
            AUTH::CAuthProviderSettings providerSettings;
            providerSettings.parameters[ "ticketIdentity" ] = "from-manager";
            providerSettings.parameters[ "webApiKey" ] = TestWebApiKey;
            providerSettings.parameters[ "appId" ] = "730";
            authSettings.SetProviderSettings( "steam", providerSettings );
            CFakeSteam steam;
            AUTH::CAuthManager manager;
            manager.SetProviderFactory( GUCEF_NULL );
            TSteamAuthProviderFactory steamFactory;
            AUTH::CIAuthProviderPtr providerPtr = steamFactory.Create();
            static_cast< CSteamAuthProvider* >( providerPtr.GetPointerAlways() )->SetShim( steam.MakeShim() );
            ASSERT_TRUE( manager.AddProvider( providerPtr ) );
            manager.SetSettings( authSettings );
            CSteamAuthProvider* managed = static_cast< CSteamAuthProvider* >( manager.GetProvider( "steam" ) );
            ASSERT_TRUE( GUCEF_NULL != managed );
            CSteamAuthSettings managedSettings = managed->GetSteamSettings();
            ASSERT_TRUE( managedSettings.ticketIdentity == "from-manager" && managedSettings.appId == "730" );
            ASSERT_TRUE( managedSettings.webApiKey.Reveal() == TestWebApiKey );
            // Steam needs no client id: it is offered when it is available and enabled
            ASSERT_TRUE( manager.IsProviderOffered( "steam" ) );
            manager.RemoveProvider( "steam" );

            // A mode that is not known is refused as a whole and the settings stay as they were
            CSteamAuthProvider provider( steam.MakeShim() );
            ASSERT_TRUE( Configure( provider, "kept" ) );
            CORE::CDataNode bad = MakeSteamNode( "changed" );
            bad.FindChild( "Parameters" )->SetAttribute( "ticketMode", "carrier-pigeon" );
            ASSERT_FALSE( provider.LoadConfig( bad ) );
            ASSERT_TRUE( provider.GetSteamSettings().ticketIdentity == "kept" );
            CORE::CDataNode badVerify = MakeSteamNode( "changed" );
            badVerify.FindChild( "Parameters" )->SetAttribute( "verifyMode", "guess" );
            ASSERT_FALSE( provider.LoadConfig( badVerify ) );
            // an empty value is the same as no value: it does not clear a setting
            CORE::CDataNode emptyValue = MakeSteamNode( "" );
            ASSERT_TRUE( provider.LoadConfig( emptyValue ) );
            ASSERT_TRUE( provider.GetSteamSettings().ticketIdentity == "kept" );
            CORE::CDataNode badPlacement = MakeSteamNode( "changed" );
            badPlacement.FindChild( "Parameters" )->SetAttribute( "webApiKeyPlacement", "cookie" );
            ASSERT_FALSE( provider.LoadConfig( badPlacement ) );
            CORE::CDataNode badInterval = MakeSteamNode( "changed", 0 );
            ASSERT_FALSE( provider.LoadConfig( badInterval ) );
            ASSERT_TRUE( provider.GetSteamSettings().ticketIdentity == "kept" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Verify at the Steam Web API: the request, the answers that are accepted and every kind of answer that is not" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );

            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "ticketIdentity", "unit-test-service" );
            parameters->SetAttribute( "appId", "480" );
            parameters->SetAttribute( "webApiKey", TestWebApiKey );
            parameters->SetAttribute( "webApiBaseUrl", server.GetBaseUrl() );
            parameters->SetAttribute( "allowInsecureHttp", true );
            parameters->SetAttribute( "webApiTimeoutInMs", (UInt32) 3000 );
            ASSERT_TRUE( provider.LoadConfig( node ) );

            AUTH::CAuthProof proof( AUTH::CAuthProof::KindPlatformTicket, "1400deadbeef0164" );
            AUTH::CAuthResult result;

            // A good answer
            server.SetResponse( 200, Json( "76561198000000007", "OK" ) );
            ASSERT_TRUE( provider.Verify( proof, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "steam:76561198000000007" );
            ASSERT_TRUE( result.providerName == "steam" );
            ASSERT_TRUE( 1 == server.GetRequestCount() );

            // The request has the method, the path and the parameters of the Web API; the key is in it, as the API wants
            CORE::CString line = server.GetLastRequestLine();
            ASSERT_TRUE( line.StartsWith( "GET /ISteamUserAuth/AuthenticateUserTicket/v1/?" ) );
            ASSERT_TRUE( line.HasSubstr( "key=" ) > 0 && line.HasSubstr( TestWebApiKey ) > 0 );
            ASSERT_TRUE( line.HasSubstr( "appid=480" ) > 0 );
            ASSERT_TRUE( line.HasSubstr( "ticket=1400deadbeef0164" ) > 0 );
            ASSERT_TRUE( line.HasSubstr( "identity=unit-test-service" ) > 0 );

            // result is not OK
            server.SetResponse( 200, Json( "76561198000000007", "Invalid" ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            // the error shape of the API
            server.SetResponse( 200, "{\"response\":{\"error\":{\"errorcode\":101,\"errordesc\":\"Invalid ticket\"}}}" );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            // no steamid, a steamid that is not a number, one that is 0, one that is too long
            server.SetResponse( 200, "{\"response\":{\"params\":{\"result\":\"OK\"}}}" );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 200, Json( "7656119800000000x", "OK" ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 200, Json( "0", "OK" ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 200, Json( "765611980000000071234567", "OK" ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            // not json, json of the wrong shape, an empty answer
            server.SetResponse( 200, "<html>maintenance</html>", "text/html" );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 200, "[1,2,3]" );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 200, "" );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            // a status that is not 200 does not count, whatever the body says
            server.SetResponse( 403, Json( "76561198000000007", "OK" ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 500, Json( "76561198000000007", "OK" ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            // an answer over the size limit
            server.SetLargeResponse( 70000 );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            server.SetResponse( 200, Json( "76561198000000007", "OK" ) );
            ASSERT_TRUE( provider.Verify( proof, result ) );

            // Proofs that are not tickets do not reach the network
            UInt32 requests = server.GetRequestCount();
            ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindOidcIdToken, "1400deadbeef0164" ), result ) );
            ASSERT_FALSE( provider.Verify( AUTH::CAuthProof(), result ) );
            ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, "not hex" ), result ) );
            ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, "abc" ), result ) );
            ASSERT_FALSE( provider.Verify( AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, "14&key=other" ), result ) );
            ASSERT_TRUE( requests == server.GetRequestCount() );

            // Nobody is listening
            server.Stop();
            ASSERT_FALSE( provider.Verify( proof, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With the key in the header it is not in the url, not in the request line and not in the log of a request that failed" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            server.SetResponse( 200, Json( "76561198000000009", "OK" ) );

            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "appId", "480" );
            parameters->SetAttribute( "webApiKey", TestWebApiKey );
            parameters->SetAttribute( "webApiKeyPlacement", "header" );
            parameters->SetAttribute( "webApiBaseUrl", server.GetBaseUrl() );
            parameters->SetAttribute( "allowInsecureHttp", true );
            parameters->SetAttribute( "webApiTimeoutInMs", (UInt32) 2000 );
            ASSERT_TRUE( provider.LoadConfig( node ) );

            CLogCapture capture;
            AUTH::CAuthProof proof( AUTH::CAuthProof::KindPlatformTicket, "1400deadbeef0164" );
            AUTH::CAuthResult result;
            ASSERT_TRUE( provider.Verify( proof, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "steam:76561198000000009" );
            ASSERT_TRUE( server.GetLastRequestLine().HasSubstr( "key=" ) < 0 && server.GetLastRequestLine().HasSubstr( "appid=480" ) > 0 );
            ASSERT_TRUE( server.GetLastRequestText().Lowercase().HasSubstr( "x-webapi-key: " ) > 0 );
            ASSERT_TRUE( server.GetLastRequestText().HasSubstr( TestWebApiKey ) > 0 );

            // A request that does not reach the server is logged by the client of the web module with its url: the key is not in it
            server.Stop();
            ASSERT_FALSE( provider.Verify( proof, result ) );
            ASSERT_TRUE( capture.Text().HasSubstr( "HttpClientRequestRunner" ) >= 0 );
            ASSERT_TRUE( capture.Text().HasSubstr( TestWebApiKey ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Verify refuses to send the key over a connection it is not allowed to, and without the key and the app id" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            server.SetResponse( 200, Json( "76561198000000007", "OK" ) );

            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            AUTH::CAuthProof proof( AUTH::CAuthProof::KindPlatformTicket, "1400deadbeef0164" );
            AUTH::CAuthResult result;

            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "appId", "480" );
            parameters->SetAttribute( "webApiBaseUrl", server.GetBaseUrl() );

            // no key
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );

            // a key, http, and no permission for http
            parameters->SetAttribute( "webApiKey", TestWebApiKey );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            ASSERT_TRUE( 0 == server.GetRequestCount() );

            // the permission, but no app id
            CSteamAuthProvider noApp( steam.MakeShim() );
            CORE::CDataNode noAppNode( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* noAppParameters = noAppNode.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            noAppParameters->SetAttribute( "webApiKey", TestWebApiKey );
            noAppParameters->SetAttribute( "webApiBaseUrl", server.GetBaseUrl() );
            noAppParameters->SetAttribute( "allowInsecureHttp", true );
            ASSERT_TRUE( noApp.LoadConfig( noAppNode ) );
            ASSERT_FALSE( noApp.Verify( proof, result ) );
            ASSERT_TRUE( 0 == server.GetRequestCount() );
            parameters->SetAttribute( "allowInsecureHttp", true );

            // user information in the url, and another scheme
            parameters->SetAttribute( "appId", "480" );
            parameters->SetAttribute( "webApiBaseUrl", "http://user@127.0.0.1:" + CORE::UInt32ToString( server.GetPort() ) );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            parameters->SetAttribute( "webApiBaseUrl", "ftp://127.0.0.1" );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            ASSERT_TRUE( 0 == server.GetRequestCount() );

            // everything in order
            parameters->SetAttribute( "webApiBaseUrl", server.GetBaseUrl() + "/" );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_TRUE( provider.Verify( proof, result ) );
            // no identity set: none is sent
            ASSERT_TRUE( server.GetLastRequestLine().HasSubstr( "identity=" ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The peer check opens a session for the claimed account, closes it and takes nothing else for good" )
        try
        {
            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT )->SetAttribute( "verifyMode", "peer" );
            ASSERT_TRUE( provider.LoadConfig( node ) );

            AUTH::CAuthProof proof( AUTH::CAuthProof::KindPlatformTicket, "1400deadbeef0164" );
            AUTH::CAuthResult result;
            result.accountRef = AUTH::CAccountRef( CSteamOpenId2Profile::AccountNamespace, "76561198000000555" );
            ASSERT_TRUE( provider.Verify( proof, result ) );
            ASSERT_TRUE( result.accountRef.ToString() == "steam:76561198000000555" );
            ASSERT_TRUE( 1 == steam.begun.size() && 76561198000000555ULL == steam.begun[ 0 ] );
            ASSERT_TRUE( 1 == steam.ended.size() && 76561198000000555ULL == steam.ended[ 0 ] );
            ASSERT_TRUE( 8 == steam.lastBeginTicket.size() && 0x14 == steam.lastBeginTicket[ 0 ] && 0xEF == steam.lastBeginTicket[ 5 ] );

            // Steam refuses the ticket for that account
            steam.beginResult = 1;
            result.accountRef = AUTH::CAccountRef( CSteamOpenId2Profile::AccountNamespace, "76561198000000555" );
            ASSERT_FALSE( provider.Verify( proof, result ) );
            ASSERT_TRUE( 1 == steam.ended.size() );

            // no claim, an account of another provider, a ticket that is not hex
            steam.beginResult = 0;
            AUTH::CAuthResult noClaim;
            ASSERT_FALSE( provider.Verify( proof, noClaim ) );
            AUTH::CAuthResult otherClaim;
            otherClaim.accountRef = AUTH::CAccountRef( AUTH::CAuthProviderIds::Google, "12345" );
            ASSERT_FALSE( provider.Verify( proof, otherClaim ) );
            ASSERT_FALSE( provider.VerifyPeerTicket( AUTH::CAccountRef( CSteamOpenId2Profile::AccountNamespace, "76561198000000555" ), AUTH::CAuthProof( AUTH::CAuthProof::KindPlatformTicket, "xyz" ), result ) );
            ASSERT_TRUE( 2 == steam.begun.size() );

            // Steam not there
            CFakeSteam off;
            off.initOk = false;
            CSteamAuthProvider offProvider( off.MakeShim() );
            ASSERT_FALSE( offProvider.VerifyPeerTicket( AUTH::CAccountRef( CSteamOpenId2Profile::AccountNamespace, "76561198000000555" ), proof, result ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The key, the ticket and the identity are never logged, not in a login, not in a verification and not in a failure" )
        try
        {
            ASSERT_TRUE( JsonCodecAvailable() );
            CFakeHttpServer server;
            ASSERT_TRUE( server.Start() );
            server.SetResponse( 200, Json( "76561198000000007", "OK" ) );

            CLogCapture capture;
            CFakeSteam steam;
            CSteamAuthProvider provider( steam.MakeShim() );
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "ticketIdentity", "unit-test-service" );
            parameters->SetAttribute( "callbackIntervalInMs", (UInt32) 5 );
            parameters->SetAttribute( "appId", "480" );
            parameters->SetAttribute( "webApiKey", TestWebApiKey );
            parameters->SetAttribute( "webApiBaseUrl", server.GetBaseUrl() );
            parameters->SetAttribute( "allowInsecureHttp", true );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );

            CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            AUTH::CAuthResult result;
            ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
            AUTH::CAuthResult verified;
            ASSERT_TRUE( provider.Verify( result.proof, verified ) );

            // failures of all kinds
            server.SetResponse( 200, "{\"response\":{\"error\":{\"errorcode\":101,\"errordesc\":\"Invalid ticket\"}}}" );
            ASSERT_FALSE( provider.Verify( result.proof, verified ) );
            steam.answerResult = 2;
            ASSERT_TRUE( provider.TrySilentLogin( future ) );
            ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            provider.Logout( result.accountRef, AUTH::CSecretString() );

            CORE::CString logged = capture.Text();
            ASSERT_TRUE( logged.HasSubstr( "SteamAuthProvider" ) >= 0 );
            ASSERT_TRUE( logged.HasSubstr( TestWebApiKey ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "STEAMKEY" ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( CFakeSteam::TicketHex( 100 ) ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "deadbeef" ) < 0 );
            for ( size_t i=0; i<recorder.records.size(); ++i )
            {
                ASSERT_TRUE( recorder.records[ i ].failure.message.HasSubstr( TestWebApiKey ) < 0 );
                ASSERT_TRUE( recorder.records[ i ].failure.message.HasSubstr( "deadbeef" ) < 0 );
            }

            // The result does not give its proof away when it is serialized
            CORE::CDataNode serialized( "result", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNodeSerializableSettings serializerSettings;
            ASSERT_TRUE( result.Serialize( serialized, serializerSettings ) );
            CORE::CDynamicBuffer dump;
            CORE::CDynamicBufferAccess dumpAccess( dump );
            CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
            ASSERT_TRUE( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( codec ) );
            ASSERT_TRUE( codec->StoreDataTree( &serialized, &dumpAccess ) );
            CORE::CString text( dump.AsConstTypePtr< char >(), dump.GetDataSize() );
            ASSERT_TRUE( text.HasSubstr( "deadbeef" ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The binding to the flat API of the runtime: names, versions of the interface, initialization, callbacks, tickets" )
        try
        {
            // The runtime of an SDK that has the version 19 accessors only and a ticket for the web api
            g_flat.Reset();
            CSteamRuntime runtime;
            ASSERT_FALSE( runtime.IsBound() );
            ASSERT_TRUE( runtime.Bind( &ResolveFlat, GUCEF_NULL ) );
            ASSERT_TRUE( runtime.IsBound() && runtime.HasWebApiTicket() );

            TSteamShim shim;
            ASSERT_TRUE( runtime.FillShim( shim ) );
            char error[ 128 ];
            error[ 0 ] = 0;

            // not initialized: nothing works yet
            ASSERT_FALSE( shim.isInitialized( shim.context ) );
            ASSERT_TRUE( 0 == shim.getSteamId( shim.context ) );
            ASSERT_TRUE( SteamInvalidTicketHandle == shim.requestWebApiTicket( shim.context, "x" ) );

            // Steam is not running
            g_flat.running = false;
            ASSERT_FALSE( shim.init( shim.context, error, sizeof( error ) ) );
            ASSERT_TRUE( 0 == strcmp( error, "The Steam client is not running" ) );
            ASSERT_TRUE( 0 == g_flat.initCalls );
            // running, and the API says no: its message comes through
            g_flat.running = true;
            g_flat.initResult = 2;
            ASSERT_FALSE( shim.init( shim.context, error, sizeof( error ) ) );
            ASSERT_TRUE( 0 == strcmp( error, "fake: Steam says no" ) );
            // and then it works
            g_flat.initResult = 0;
            ASSERT_TRUE( shim.init( shim.context, error, sizeof( error ) ) );
            ASSERT_TRUE( shim.init( shim.context, error, sizeof( error ) ) );
            ASSERT_TRUE( 2 == g_flat.initCalls );
            ASSERT_TRUE( g_flat.manualDispatchInit && shim.isInitialized( shim.context ) );

            ASSERT_TRUE( 76561198000000042ULL == shim.getSteamId( shim.context ) );
            ASSERT_TRUE( shim.isLoggedOn( shim.context ) );
            char persona[ 32 ];
            ASSERT_TRUE( shim.getPersonaName( shim.context, persona, sizeof( persona ) ) );
            ASSERT_TRUE( 0 == strcmp( persona, "Flat Persona" ) );
            // a buffer that is too small gets a text that fits
            char tiny[ 5 ];
            ASSERT_TRUE( shim.getPersonaName( shim.context, tiny, sizeof( tiny ) ) );
            ASSERT_TRUE( 0 == strcmp( tiny, "Flat" ) );

            // a ticket for the web api: the answer comes with a frame, the other callback is skipped and freed
            UInt32 handle = shim.requestWebApiTicket( shim.context, "flat-identity" );
            ASSERT_TRUE( 500 == handle && 1 == g_flat.identities.size() && g_flat.identities[ 0 ] == "flat-identity" );
            SSteamTicketResponse response;
            ASSERT_FALSE( shim.pollTicketResponse( shim.context, &response ) );
            shim.runFrame( shim.context );
            ASSERT_TRUE( shim.pollTicketResponse( shim.context, &response ) );
            ASSERT_TRUE( response.ticketHandle == 500 && SteamResultOk == response.result && 8 == response.ticketSize );
            ASSERT_TRUE( 0xDE == response.ticket[ 2 ] && 0xEF == response.ticket[ 5 ] );
            ASSERT_FALSE( shim.pollTicketResponse( shim.context, &response ) );
            ASSERT_TRUE( 2 == g_flat.freed && g_flat.queue.empty() );

            // a callback that is too short, and one that claims more ticket than it holds
            SFakeCallback shortCallback;
            shortCallback.id = CSteamRuntime::TicketForWebApiCallbackId;
            shortCallback.payload.assign( 6, 0 );
            g_flat.dueNextFrame.push_back( shortCallback );
            SFakeCallback liar;
            liar.id = CSteamRuntime::TicketForWebApiCallbackId;
            liar.payload.assign( 12 + 4, 0x11 );
            UInt32 liarHandle = 77; Int32 liarResult = 1; Int32 liarSize = 100000;
            memcpy( &liar.payload[ 0 ], &liarHandle, 4 );
            memcpy( &liar.payload[ 4 ], &liarResult, 4 );
            memcpy( &liar.payload[ 8 ], &liarSize, 4 );
            g_flat.dueNextFrame.push_back( liar );
            shim.runFrame( shim.context );
            ASSERT_TRUE( shim.pollTicketResponse( shim.context, &response ) );
            ASSERT_TRUE( 77 == response.ticketHandle && 4 == response.ticketSize );
            ASSERT_FALSE( shim.pollTicketResponse( shim.context, &response ) );

            // session ticket: five arguments are passed
            UInt8 buffer[ SteamMaxTicketSize ];
            UInt32 size = 0;
            UInt32 sessionHandle = shim.requestSessionTicket( shim.context, buffer, sizeof( buffer ), &size );
            ASSERT_TRUE( SteamInvalidTicketHandle != sessionHandle && 8 == size && 5 == g_flat.lastSessionArgs );

            // peer check, cancel
            ASSERT_TRUE( SteamBeginAuthSessionOk == shim.beginAuthSession( shim.context, buffer, (Int32) size, 4711 ) );
            shim.endAuthSession( shim.context, 4711 );
            ASSERT_TRUE( 4711 == g_flat.lastBeginId && 4711 == g_flat.lastEndId );
            shim.cancelTicket( shim.context, 500 );
            shim.cancelTicket( shim.context, SteamInvalidTicketHandle );
            ASSERT_TRUE( 1 == g_flat.cancelled.size() && 500 == g_flat.cancelled[ 0 ] );

            // the interface of every call is the one the accessor gave, the pipe the one of the API
            ASSERT_FALSE( g_flat.wrongSelf || g_flat.wrongPipe );

            shim.shutdown( shim.context );
            ASSERT_TRUE( 1 == g_flat.shutdownCalls && !shim.isInitialized( shim.context ) );
            // after the shutdown nothing reaches the API
            ASSERT_TRUE( SteamInvalidTicketHandle == shim.requestWebApiTicket( shim.context, "x" ) );
            runtime.SetOwnsApi( false );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The binding takes the newest version of the accessors, goes without optional functions and refuses a runtime that lacks what it needs" )
        try
        {
            g_flat.Reset();
            g_flat.withUserV023 = true;
            g_flat.withUserV019 = false;
            CSteamRuntime newest;
            ASSERT_TRUE( newest.Bind( &ResolveFlat, GUCEF_NULL ) );
            newest.SetOwnsApi( false );

            // an older SDK without the ticket for the web api: the session ticket is the way
            g_flat.Reset();
            g_flat.withWebApi = false;
            CSteamRuntime older;
            ASSERT_TRUE( older.Bind( &ResolveFlat, GUCEF_NULL ) );
            ASSERT_FALSE( older.HasWebApiTicket() );
            TSteamShim shim;
            ASSERT_TRUE( older.FillShim( shim ) );
            ASSERT_TRUE( GUCEF_NULL == shim.requestWebApiTicket && GUCEF_NULL != shim.requestSessionTicket );
            older.SetOwnsApi( false );

            // no accessor of the user at all, and no InitFlat
            g_flat.Reset();
            g_flat.withUserV019 = false;
            CSteamRuntime noUser;
            ASSERT_FALSE( noUser.Bind( &ResolveFlat, GUCEF_NULL ) );
            ASSERT_FALSE( noUser.IsBound() );
            TSteamShim none;
            ASSERT_FALSE( noUser.FillShim( none ) );
            ASSERT_TRUE( GUCEF_NULL == none.init );
            g_flat.Reset();
            g_flat.withInitFlat = false;
            CSteamRuntime noInit;
            ASSERT_FALSE( noInit.Bind( &ResolveFlat, GUCEF_NULL ) );
            ASSERT_FALSE( noInit.Bind( GUCEF_NULL, GUCEF_NULL ) );

            // a library that is not there is not available, and says why
            g_flat.Reset();
            CSteamRuntime missing;
            ASSERT_FALSE( missing.Load( "libsteam_api_that_does_not_exist" ) );
            ASSERT_FALSE( missing.IsBound() );
            CORE::CString::StringVector names = CSteamRuntime::GetDefaultLibraryNames();
            ASSERT_TRUE( 1 == names.size() && names[ 0 ].HasSubstr( "steam_api" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A provider on the flat API of the fake runtime logs in end to end" )
        try
        {
            g_flat.Reset();
            CSteamRuntime runtime;
            ASSERT_TRUE( runtime.Bind( &ResolveFlat, GUCEF_NULL ) );
            TSteamShim shim;
            ASSERT_TRUE( runtime.FillShim( shim ) );
            {
                CSteamAuthProvider provider( shim );
                ASSERT_TRUE( Configure( provider, "flat-service" ) );
                ASSERT_TRUE( provider.IsClientAvailable() );
                CORE::CFutureResult future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
                ASSERT_TRUE( provider.TrySilentLogin( future ) );
                ASSERT_TRUE( PumpUntilReady( future, 2000 ) );
                AUTH::CAuthResult result;
                ASSERT_TRUE( AUTH::CAuthResult::TryGetFromFuture( future, result ) );
                ASSERT_TRUE( result.accountRef.ToString() == "steam:76561198000000042" );
                ASSERT_TRUE( result.displayName == "Flat Persona" );
                ASSERT_TRUE( result.proof.data.Reveal() == "1400deadbeef01f4" );
                ASSERT_TRUE( g_flat.identities.size() == 1 && g_flat.identities[ 0 ] == "flat-service" );
            }
            // the ticket was cancelled when the provider went
            ASSERT_TRUE( 1 == g_flat.cancelled.size() && 500 == g_flat.cancelled[ 0 ] );
            runtime.SetOwnsApi( false );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Built without the Steam runtime the plugin registers and reports that it is not available" )
        try
        {
            // The provider of the plugin as an application gets it: no table of functions, the runtime library is looked for
            CSteamAuthProvider provider;
            CORE::CDataNode node( "steam", GUCEF_DATATYPE_OBJECT );
            CORE::CDataNode* parameters = node.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
            parameters->SetAttribute( "ticketIdentity", "unit-test-service" );
            parameters->SetAttribute( "runtimeLibrary", "libsteam_api_that_does_not_exist" );
            ASSERT_TRUE( provider.LoadConfig( node ) );
            ASSERT_FALSE( provider.IsClientAvailable() );

            CORE::CFutureResult silent( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_FALSE( provider.TrySilentLogin( silent ) );
            ASSERT_FALSE( silent.HasAFuture() );

            CProviderRecorder recorder;
            recorder.SubscribeTo( &provider );
            CORE::CFutureResult interactive( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );
            ASSERT_TRUE( provider.BeginLogin( AUTH::CAuthOptions(), interactive ) );
            ASSERT_TRUE( PumpUntilReady( interactive, 1000 ) );
            ASSERT_TRUE( recorder.PumpUntil( CProviderRecorder::KIND_FAILED, 1, 1000 ) );
            ASSERT_TRUE( AUTH::AUTHFAIL_NOT_AVAILABLE == recorder.Last( CProviderRecorder::KIND_FAILED )->failure.reason );
            ASSERT_TRUE( recorder.Last( CProviderRecorder::KIND_FAILED )->failure.message.HasSubstr( "cannot be loaded" ) >= 0 );

            // Registration in a factory of its own and in the one of the process
            AUTH::CAuthProviderFactory factory;
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
            ASSERT_TRUE( RegisterSteamAuthProviders( factory ) );
            ASSERT_TRUE( factory.IsConstructible( "steam" ) );
            ASSERT_FALSE( RegisterSteamAuthProviders( factory ) );
            AUTH::CIAuthProviderPtr made = factory.Create( "steam" );
            ASSERT_FALSE( made.IsNULL() );
            ASSERT_TRUE( made->GetProviderName() == "steam" );
            made.Unlink();
            UnregisterSteamAuthProviders( factory );
            ASSERT_FALSE( factory.IsConstructible( "steam" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
