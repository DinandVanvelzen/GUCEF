/*
 *  gucefCOMCORE_TestApp: GUCEF test application for the COMCORE module
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

/*
 *  The session and endpoint tests of the RPC design's Phase 1 verification items (16.2) that look at the frames on the
 *  wire, through the capturing and fault injecting decorator (CRpcLossyTransport) around a pair of the transports
 *  under test: contract fetch and cache, no names on the wire, minor and major versions, deadlines and cancellation,
 *  resume after a forced disconnect, RESUME_GAP, gaps on a reliable transport, a lossy transport, fragmentation.
 *  Header only, like TestRpcConformance.h: gucefCOMCORE_TestApp runs it over loopback, gucefCOM_TestApp over tcp
 *  (PerformRpcTransportSuiteTests( transports )).
 */

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCTRANSPORTSUITE_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCTRANSPORTSUITE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRpcConformance.h"
#include "TestRpcEndpointFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define RTS_ERRORHERE           GUCEF_TESTFW_ERRORHERE
#define RTS_ASSERT_TRUE(t)      GUCEF_TESTFW_ASSERT_TRUE(t)
#define RTS_ASSERT_FALSE(t)     GUCEF_TESTFW_ASSERT_FALSE(t)
#define RTS_ASSERT_OK( s )      { GUCEF::COMCORE::CRpcStatus assertStatus_( s ); if ( !assertStatus_.IsOk() ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcTransportSuite test: line " << __LINE__ << ": " << assertStatus_.ToString() ); } }
#define RTS_ASSERT_CODE( c, s ) { GUCEF::COMCORE::CRpcStatus assertStatus_( s ); if ( c != assertStatus_.code ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcTransportSuite test: line " << __LINE__ << ": expected " << GUCEF::COMCORE::RpcStatusCodeToString( c ) << ", got " << assertStatus_.ToString() ); } }
#define RTS_REQUIRE_TRUE( t )   { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcTransportSuite test: requirement failed: " #t ); return; } }

namespace RpcTransportSuite {

using namespace GUCEF;

static const char* g_getPath = "gucef.sample.notes.Notes/Get";
static const char* g_savePath = "gucef.sample.notes.Notes/Save";
static const char* g_watchPath = "gucef.sample.notes.Notes/Watch";
static const char* g_importPath = "gucef.sample.notes.Notes/Import";
static const char* g_editPath = "gucef.sample.notes.Notes/Edit";
static const char* g_reindexPath = "gucef.sample.notes.Notes/Reindex";

/*-------------------------------------------------------------------------*/

inline bool
PumpUntilFlag( CRpcEndpointFixture& f, const bool& flag, UInt32 timeoutMs )
{
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( !flag )
    {
        f.pulseGenerator->ForceDirectPulse();
        if ( ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

inline bool
PumpUntilCount( CRpcEndpointFixture& f, const UInt32& counter, UInt32 target, UInt32 timeoutMs )
{
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( counter < target )
    {
        f.pulseGenerator->ForceDirectPulse();
        if ( ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

inline void
DumpFrames( CRpcEndpointFixture& f, const char* why )
{
    GUCEF::vector< CRpcLossyTransport::SFrame > frames;
    f.clientWire->GetSent( frames );
    CORE::CString text = CORE::CString( why ) + ": client sent";
    for ( UInt32 i=0; i<frames.size(); ++i )
        text += " " + CORE::CString( COMCORE::RpcFrameTypeToString( frames[ i ].type ) ) + "#" + CORE::ToString( frames[ i ].seq );
    f.serverWire->GetSent( frames );
    text += "; server sent";
    for ( UInt32 i=0; i<frames.size(); ++i )
        text += " " + CORE::CString( COMCORE::RpcFrameTypeToString( frames[ i ].type ) ) + "#" + CORE::ToString( frames[ i ].seq );
    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcTransportSuite test: " << text );
}

inline void
CollectNames( const COMCORE::CRpcContract& contract, GUCEF::vector< CORE::CString >& names )
{
    names.push_back( contract.GetName() );
    for ( UInt32 t=0; t<contract.GetTypeCount(); ++t )
    {
        const COMCORE::CRpcTypeDescriptor& type = contract.GetTypeAt( t );
        names.push_back( type.GetName() );
        for ( UInt32 f=0; f<type.GetFieldCount(); ++f )
            names.push_back( type.GetFieldAt( f ).GetName() );
    }
    for ( UInt32 e=0; e<contract.GetEnumCount(); ++e )
    {
        names.push_back( contract.GetEnumAt( e ).GetName() );
        for ( UInt32 v=0; v<contract.GetEnumAt( e ).GetValueCount(); ++v )
            names.push_back( contract.GetEnumAt( e ).GetValueAt( v ).name );
    }
    for ( UInt32 s=0; s<contract.GetServiceCount(); ++s )
    {
        names.push_back( contract.GetServiceAt( s ).GetName() );
        for ( UInt32 m=0; m<contract.GetServiceAt( s ).GetMethodCount(); ++m )
            names.push_back( contract.GetServiceAt( s ).GetMethodAt( m ).GetName() );
    }
    for ( UInt32 i=0; i<names.size(); )
    {
        if ( names[ i ].Length() < 4 )
            names.erase( names.begin() + i );
        else
            ++i;
    }
}

inline bool
PumpUntil( CORE::PulseGeneratorPtr pulseGenerator, const UInt32& counter, UInt32 target, UInt32 timeoutMs )
{
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( counter < target )
    {
        pulseGenerator->ForceDirectPulse();
        if ( ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
    return true;
}

inline bool
PumpFutures( CRpcEndpointFixture& f, GUCEF::vector< CORE::CFutureResult >& futures, UInt32 timeoutMs )
{
    UInt64 start = MT::PrecisionTickCount();
    UInt64 ticksPerMs = MT::PrecisionTimerResolution() / 1000;
    while ( true )
    {
        bool all = true;
        for ( UInt32 i=0; i<futures.size() && all; ++i )
            all = futures[ i ].IsReady();
        if ( all )
            return true;
        f.pulseGenerator->ForceDirectPulse();
        if ( ( MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
            return false;
        MT::PrecisionDelay( 1 );
    }
}

inline COMCORE::CRpcMessage
GetRequest( CRpcEndpointFixture& f, UInt64 noteId )
{
    COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
    request.SetUInt64( 1, noteId );
    return request;
}

inline UInt64
TelemetryValue( const COMCORE::CRpcEndpointPtr& endpoint, const char* name )
{
    CORE::CDataNode telemetry;
    endpoint->GetTelemetry( telemetry );
    return telemetry.GetAttributeValue( name ).AsUInt64();
}

inline UInt64
FirstSeqOf( CRpcLossyTransport* wire, UInt8 type )
{
    GUCEF::vector< CRpcLossyTransport::SFrame > frames;
    wire->GetSent( frames );
    for ( UInt32 i=0; i<frames.size(); ++i )
        if ( frames[ i ].type == type )
            return frames[ i ].seq;
    return 0;
}

inline bool
NewTransports( CRpcEndpointFixture& f, COMCORE::CIRpcTransportPtr& oldClient, COMCORE::CIRpcTransportPtr& oldServer )
{
    oldClient = f.clientTransport;
    oldServer = f.serverTransport;
    f.clientTransport.Unlink();
    f.serverTransport.Unlink();
    return f.MakeTransports();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

inline void
TestContractFetchThenCache( CIRpcConformanceTransports& transports )
{
    CORE::CString cacheDir = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), "gucef_rpc_test_cache_" + CORE::ToString( MT::GetCurrentThreadID() ) );
    CORE::DeleteDir( cacheDir, true );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    {
        // the client knows nothing: GET_CONTRACT, CONTRACT, then BIND
        CRpcEndpointFixture f( &transports );
        f.clientRegistry.SetCacheDirectory( cacheDir );
        RTS_REQUIRE_TRUE( f.Connect( notes, COMCORE::CRpcContractPtr() ) );
        RTS_ASSERT_TRUE( 1 == f.clientWire->CountSent( GUCEF_RPC_FRAME_GET_CONTRACT ) );
        RTS_ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_CONTRACT ) );
        RTS_ASSERT_TRUE( 1 == f.clientWire->CountSent( GUCEF_RPC_FRAME_BIND ) );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 3 ) );
        RTS_ASSERT_OK( result.status );
        RTS_ASSERT_TRUE( "note 3" == result.response.GetString( 2 ) );
    }
    {
        // a new process with the same cache: the contract comes from the disk cache by hash, no GET_CONTRACT
        CRpcEndpointFixture f( &transports );
        f.clientRegistry.SetCacheDirectory( cacheDir );
        RTS_REQUIRE_TRUE( f.Connect( notes, COMCORE::CRpcContractPtr() ) );
        RTS_ASSERT_TRUE( 0 == f.clientWire->CountSent( GUCEF_RPC_FRAME_GET_CONTRACT ) );
        RTS_ASSERT_TRUE( 1 == f.clientWire->CountSent( GUCEF_RPC_FRAME_BIND ) );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 4 ) );
        RTS_ASSERT_OK( result.status );
    }
    CORE::DeleteDir( cacheDir, true );
}

/*-------------------------------------------------------------------------*/

inline void
TestNoNamesOnWire( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );
    f.clientWire->ClearCapture();
    f.serverWire->ClearCapture();

    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
    RTS_ASSERT_OK( result.status );
    CRpcStreamCollector collector;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    RTS_REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
    COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
    RTS_REQUIRE_TRUE( !stream.IsNULL() );
    collector.Watch( stream );
    RTS_ASSERT_TRUE( PumpUntilFlag( f, collector.ended, 3000 ) );
    if ( !collector.ended )
        DumpFrames( f, "watch" );

    // names of five characters or more (shorter ones occur by chance; the values sent are not names)
    GUCEF::vector< CORE::CString > names;
    CollectNames( *notes, names );
    UInt32 checked = 0;
    for ( UInt32 i=0; i<names.size(); ++i )
    {
        if ( names[ i ].Length() < 5 )
            continue;
        ++checked;
        bool clientSent = f.clientWire->SentContains( 0, names[ i ].C_String(), names[ i ].ByteSize() - 1 );
        bool serverSent = f.serverWire->SentContains( 0, names[ i ].C_String(), names[ i ].ByteSize() - 1 );
        if ( clientSent || serverSent )
            GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcTransportSuite test: name on the wire after binding: " << names[ i ] );
        RTS_ASSERT_FALSE( clientSent );
        RTS_ASSERT_FALSE( serverSent );
    }
    RTS_ASSERT_TRUE( checked > 20 );
}

/*-------------------------------------------------------------------------*/

inline void
TestMinorInterop( CIRpcConformanceTransports& transports )
{
    {
        // a 1.2 caller against a 1.0 provider: Get works, Import (since 1) is refused without being sent
        CRpcEndpointFixture f( &transports );
        RTS_REQUIRE_TRUE( f.Connect( LoadSampleNotesContract( 1, 0 ), LoadSampleNotesContract( 1, 2 ) ) );
        RTS_ASSERT_TRUE( 0 == f.client->GetPeerContractMinor( "gucef.sample.notes", 1 ) );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
        RTS_ASSERT_OK( result.status );
        RTS_ASSERT_TRUE( "note 5" == result.response.GetString( 2 ) );
        f.clientWire->ClearCapture();
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        bool resolved = f.client->ResolveMethod( g_importPath, ref, status );
        if ( resolved )
        {
            COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
            COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
            if ( !stream.IsNULL() ) RTS_ASSERT_CODE( COMCORE::RPC_STATUS_UNIMPLEMENTED, stream->GetStatus() );
        }
        else
        {
            RTS_ASSERT_CODE( COMCORE::RPC_STATUS_UNIMPLEMENTED, status );
        }
        RpcTestPump( 5 );
        RTS_ASSERT_TRUE( 0 == f.clientWire->CountSent( GUCEF_RPC_FRAME_REQUEST ) );
    }
    {
        // a 1.0 caller against a 1.2 provider: the provider's answer carries a field the caller does not know (price)
        CRpcEndpointFixture f( &transports );
        RTS_REQUIRE_TRUE( f.Connect( LoadSampleNotesContract( 1, 2 ), LoadSampleNotesContract( 1, 0 ) ) );
        RTS_ASSERT_TRUE( 2 == f.client->GetPeerContractMinor( "gucef.sample.notes", 1 ) );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
        RTS_ASSERT_OK( result.status );
        RTS_ASSERT_TRUE( "note 5" == result.response.GetString( 2 ) );
        RTS_ASSERT_TRUE( result.response.GetUnknownFieldCount() > 0 );
    }
}

/*-------------------------------------------------------------------------*/

inline void
TestMajorRefused( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    CRpcEndpointEventRecorder events;
    RTS_REQUIRE_TRUE( f.Connect( LoadSampleNotesContract( 1, 2 ), LoadSampleNotesContract( 2, 0 ), COMCORE::RPC_EXECUTION_CALLING_THREAD, 2, false ) );
    events.Watch( f.client );
    RpcTestPump( 50 );
    RTS_ASSERT_FALSE( f.client->IsContractBound( "gucef.sample.notes", 2 ) );
    RTS_ASSERT_TRUE( 0 == f.clientWire->CountSent( GUCEF_RPC_FRAME_BIND ) );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    RTS_ASSERT_FALSE( f.client->ResolveMethod( g_getPath, ref, status ) );
    COMCORE::CRpcMessage request;
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, request );
    RTS_ASSERT_FALSE( result.status.IsOk() );
    RTS_ASSERT_TRUE( 0 == f.clientWire->CountSent( GUCEF_RPC_FRAME_REQUEST ) );
    RTS_ASSERT_TRUE( 0 == f.service.calls );
    RTS_ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == f.client->GetState() );
}

/*-------------------------------------------------------------------------*/

inline void
TestDeadlinesAndCancellation( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.serverSettings.clockSkewAllowanceMs = 0;
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );

    // the caller's deadline: DEADLINE_EXCEEDED locally, CANCEL to the provider, the held call is cancelled there
    COMCORE::CRpcCallOptions options;
    options.deadlineMs = 100;
    UInt64 start = MT::PrecisionTickCount();
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 999 ), options );
    UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
    RTS_ASSERT_CODE( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, result.status );
    RTS_ASSERT_TRUE( elapsedMs >= 90 && elapsedMs < 2000 );
    RTS_ASSERT_TRUE( PumpUntilCount( f, f.service.cancelledCalls, 1, 2000 ) );

    // the future cancelled by the application
    CORE::CFutureResult future = f.client->CallAsync( g_getPath, GetRequest( f, 999 ) );
    RpcTestPump( 10 );
    future.RequestCancellation();
    RTS_ASSERT_TRUE( PumpUntilCount( f, f.service.cancelledCalls, 2, 2000 ) );

    // the provider checks the deadline on arrival: a request that arrives late is not run
    UInt32 callsBefore = f.service.calls;
    f.clientWire->delayMs = 300;
    options.deadlineMs = 50;
    result = f.CallAndPump( g_getPath, GetRequest( f, 5 ), options );
    RTS_ASSERT_CODE( COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, result.status );
    RpcTestPump( 400 );
    RTS_ASSERT_TRUE( callsBefore == f.service.calls );
    f.clientWire->delayMs = 0;
    result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
    RTS_ASSERT_OK( result.status );
}

/*-------------------------------------------------------------------------*/

inline void
TestResumeContinuesBothCounters( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );
    CRpcEndpointEventRecorder clientEvents;
    CRpcEndpointEventRecorder serverEvents;
    clientEvents.Watch( f.client );
    serverEvents.Watch( f.server );
    UInt8 sessionBefore[ 16 ];
    UInt8 sessionAfter[ 16 ];
    RTS_REQUIRE_TRUE( f.client->GetSessionId( sessionBefore ) );

    // a request the reliable transport swallows, then the transport goes
    CRpcLossyTransport* oldClientWire = f.clientWire;
    oldClientWire->blackHole = true;
    CORE::CFutureResult future = f.client->CallAsync( g_getPath, GetRequest( f, 31 ) );
    RpcTestPump( 3 );
    UInt64 requestSeq = FirstSeqOf( oldClientWire, GUCEF_RPC_FRAME_REQUEST );
    RTS_REQUIRE_TRUE( 0 != requestSeq );
    UInt64 clientNextBefore = f.client->GetNextSendSeq();
    UInt64 serverNextBefore = f.server->GetNextSendSeq();
    oldClientWire->ForceDisconnect();
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    RTS_ASSERT_FALSE( future.IsReady() );

    // a call made while suspended waits for the resume (waitForReady)
    CORE::CFutureResult waiting = f.client->CallAsync( g_getPath, GetRequest( f, 32 ) );

    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    RTS_REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    RTS_REQUIRE_TRUE( f.server->Rebind( f.serverTransport ) );
    RTS_REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    RTS_ASSERT_TRUE( f.PumpUntilReady( future, 3000 ) );
    RTS_ASSERT_TRUE( f.PumpUntilReady( waiting, 3000 ) );
    COMCORE::CRpcCallResult result;
    RTS_ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
    RTS_ASSERT_OK( result.status );
    RTS_ASSERT_TRUE( 31 == result.response.GetUInt64( 1 ) );
    RTS_ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( waiting, result ) );
    RTS_ASSERT_OK( result.status );
    RTS_ASSERT_TRUE( 32 == result.response.GetUInt64( 1 ) );

    // the same session, both seq counters continue, the lost request went exactly once more
    RTS_ASSERT_TRUE( f.client->GetSessionId( sessionAfter ) );
    RTS_ASSERT_TRUE( 0 == memcmp( sessionBefore, sessionAfter, 16 ) );
    RTS_ASSERT_TRUE( f.client->GetNextSendSeq() > clientNextBefore );
    RTS_ASSERT_TRUE( f.server->GetNextSendSeq() > serverNextBefore );
    RTS_ASSERT_TRUE( 1 == f.clientWire->CountSentSeq( requestSeq ) );
    RTS_ASSERT_TRUE( 2 == f.service.calls );
    RTS_ASSERT_TRUE( 1 == clientEvents.resumed && 1 == serverEvents.resumed );
    RTS_ASSERT_TRUE( 0 == clientEvents.established && 0 == clientEvents.gaps );
    RTS_ASSERT_TRUE( TelemetryValue( f.client, "resumeRetransmissions" ) >= 1 );
}

/*-------------------------------------------------------------------------*/

inline void
TestResumeGap( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.clientSettings.suspendedOverflow = COMCORE::RPC_SUSPENDEDOVERFLOW_DROP_OLDEST;
    f.clientSettings.retentionMaxFrames = 2;
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );
    CRpcEndpointEventRecorder clientEvents;
    clientEvents.Watch( f.client );
    UInt8 sessionBefore[ 16 ];
    UInt8 sessionAfter[ 16 ];
    RTS_REQUIRE_TRUE( f.client->GetSessionId( sessionBefore ) );

    f.clientWire->ForceDisconnect();
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );

    // four idempotent calls while suspended; only two frames are kept, the oldest go: a gap the resume reports
    COMCORE::CRpcCallOptions options;
    options.retryIdempotentOnNewSession = true;
    GUCEF::vector< CORE::CFutureResult > futures;
    for ( UInt32 i=0; i<4; ++i )
        futures.push_back( f.client->CallAsync( g_getPath, GetRequest( f, 40 + i ), options ) );
    RpcTestPump( 3 );
    RTS_ASSERT_TRUE( TelemetryValue( f.client, "droppedFrames" ) >= 2 );

    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    RTS_REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    RTS_REQUIRE_TRUE( f.server->Rebind( f.serverTransport ) );
    RTS_REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    RTS_ASSERT_TRUE( PumpFutures( f, futures, 5000 ) );
    for ( UInt32 i=0; i<futures.size(); ++i )
    {
        COMCORE::CRpcCallResult result;
        RTS_ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result ) );
        RTS_ASSERT_OK( result.status );
        RTS_ASSERT_TRUE( 40 + i == result.response.GetUInt64( 1 ) );
    }
    RTS_ASSERT_TRUE( 1 == clientEvents.gaps );
    RTS_ASSERT_FALSE( clientEvents.lastGap.sessionUnknown );
    RTS_ASSERT_TRUE( clientEvents.lastGap.gapToAcceptorFrom > 0 && clientEvents.lastGap.gapToAcceptorTo >= clientEvents.lastGap.gapToAcceptorFrom );
    RTS_ASSERT_TRUE( 0 == clientEvents.resumed );
    RTS_ASSERT_TRUE( f.client->GetSessionId( sessionAfter ) );
    RTS_ASSERT_FALSE( 0 == memcmp( sessionBefore, sessionAfter, 16 ) );
    RTS_ASSERT_TRUE( 4 == f.service.calls );

    // without retryIdempotentOnNewSession the open calls end with RESUME_GAP
    f.clientWire->ForceDisconnect();
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    futures.clear();
    options.retryIdempotentOnNewSession = false;
    for ( UInt32 i=0; i<4; ++i )
        futures.push_back( f.client->CallAsync( g_getPath, GetRequest( f, 50 + i ), options ) );
    RpcTestPump( 3 );
    RTS_REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    RTS_REQUIRE_TRUE( f.server->Rebind( f.serverTransport ) );
    RTS_REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    RTS_ASSERT_TRUE( PumpFutures( f, futures, 5000 ) );
    UInt32 gapped = 0;
    for ( UInt32 i=0; i<futures.size(); ++i )
    {
        COMCORE::CRpcCallResult result;
        COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result );
        if ( COMCORE::RPC_STATUS_RESUME_GAP == result.status.code )
            ++gapped;
        else
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcTransportSuite test: call " << i << " after the gap: " << result.status.ToString() );
    }
    RTS_ASSERT_TRUE( 4 == gapped );
    RTS_ASSERT_TRUE( PumpUntil( f.pulseGenerator, clientEvents.gaps, 2, 2000 ) );
}

/*-------------------------------------------------------------------------*/

inline void
TestGapOnReliableTransport( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );
    CRpcEndpointEventRecorder serverEvents;
    serverEvents.Watch( f.server );

    // one request lost on a transport that claims reliability, the next one arrives: the receiver sees the gap,
    // closes the transport and the resume repairs it
    f.clientWire->blackHole = true;
    CORE::CFutureResult lost = f.client->CallAsync( g_getPath, GetRequest( f, 61 ) );
    RpcTestPump( 2 );
    f.clientWire->blackHole = false;
    CORE::CFutureResult next = f.client->CallAsync( g_getPath, GetRequest( f, 62 ) );
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    RTS_ASSERT_TRUE( TelemetryValue( f.server, "gapsDetected" ) >= 1 );
    RTS_REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );

    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    RTS_REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    RTS_REQUIRE_TRUE( f.server->Rebind( f.serverTransport ) );
    RTS_REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    RTS_ASSERT_TRUE( f.PumpUntilReady( lost, 3000 ) );
    RTS_ASSERT_TRUE( f.PumpUntilReady( next, 3000 ) );
    COMCORE::CRpcCallResult result;
    COMCORE::CRpcEndpoint::TryGetCallResult( lost, result );
    RTS_ASSERT_OK( result.status );
    COMCORE::CRpcEndpoint::TryGetCallResult( next, result );
    RTS_ASSERT_OK( result.status );
    RTS_ASSERT_TRUE( 2 == f.service.calls );
    RTS_ASSERT_TRUE( 1 == serverEvents.resumed );
}

/*-------------------------------------------------------------------------*/

inline void
TestLossyMailLikeTransport( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );

    // the timing of the mail profile, scaled down so the test runs in seconds
    COMCORE::CRpcTransportProfile profile = COMCORE::CRpcTransportProfile::GetProfile( "mail" ).Scaled( 20000 );
    f.clientSettings.useTransportProfile = false;
    f.clientSettings.profile = profile;
    f.serverSettings.useTransportProfile = false;
    f.serverSettings.profile = profile;
    RTS_REQUIRE_TRUE( f.MakeTransports() );
    CRpcLossyTransport* wires[ 2 ] = { f.clientWire, f.serverWire };
    for ( UInt32 w=0; w<2; ++w )
    {
        wires[ w ]->SetSeed( 7 + w );
        wires[ w ]->dropPermille = 100;
        wires[ w ]->duplicatePermille = 50;
        wires[ w ]->reorderPermille = 100;
        wires[ w ]->delayMs = 2;
        wires[ w ]->jitterMs = 10;
        wires[ w ]->profileName = "mail";
        wires[ w ]->lossyControlFrames = true;      // HELLO, ACK and STREAM_CREDIT are lost as well
    }
    RTS_REQUIRE_TRUE( f.Connect( notes, notes, COMCORE::RPC_EXECUTION_CALLING_THREAD, 1, false ) );
    RTS_REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 10000 ) );

    // the method's own deadline (5 s) is not what this test is about: a loaded machine may need longer
    COMCORE::CRpcCallOptions options;
    options.deadlineMs = 60000;
    GUCEF::vector< CORE::CFutureResult > futures;
    for ( UInt32 i=0; i<200; ++i )
        futures.push_back( f.client->CallAsync( g_getPath, GetRequest( f, 1000 + i ), options ) );
    RTS_ASSERT_TRUE( PumpFutures( f, futures, 60000 ) );
    UInt32 ok = 0;
    for ( UInt32 i=0; i<futures.size(); ++i )
    {
        COMCORE::CRpcCallResult result;
        if ( COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result ) && result.status.IsOk() && 1000 + i == result.response.GetUInt64( 1 ) )
            ++ok;
        else if ( ok + 1 == i + 1 )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcTransportSuite test: lossy call " << i << ": " << result.status.ToString() );
    }
    RTS_ASSERT_TRUE( 200 == ok );
    RTS_ASSERT_TRUE( 200 == f.service.calls );          // duplicates never run a handler twice
    UInt64 retransmissions = TelemetryValue( f.client, "retransmissions" ) + TelemetryValue( f.server, "retransmissions" );
    UInt64 duplicates = TelemetryValue( f.client, "duplicateFrames" ) + TelemetryValue( f.server, "duplicateFrames" );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcTransportSuite test: lossy run: " << retransmissions << " retransmissions, " << duplicates << " duplicates" );
    RTS_ASSERT_TRUE( retransmissions > 0 );
    RTS_ASSERT_TRUE( duplicates > 0 );

    // a server stream past the initial credit (64) arrives complete and in order: lost STREAM_CREDIT frames are
    // sent again
    f.service.watchCount = 200;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    RTS_REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
    COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
    RTS_REQUIRE_TRUE( !stream.IsNULL() );
    COMCORE::CRpcMessage event;
    UInt32 received = 0;
    bool inOrder = true;
    while ( stream->ReadNext( event, status, 10000 ) )
    {
        ++received;
        const COMCORE::CRpcMessage* note = event.GetMessage( 1 );
        inOrder = inOrder && GUCEF_NULL != note && received == note->GetUInt64( 1 );
    }
    RTS_ASSERT_OK( status );
    RTS_ASSERT_TRUE( 200 == received );
    RTS_ASSERT_TRUE( inOrder );
}

/*-------------------------------------------------------------------------*/

inline void
TestFragmentation( CIRpcConformanceTransports& transports )
{
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.clientSettings.limits.maxFrameSize = 4096;
    f.serverSettings.limits.maxFrameSize = 4096;
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );

    // one message of 2 MB in 4 KB frames while a small call passes it
    const UInt32 size = 2 * 1024 * 1024;
    CORE::CDynamicBuffer data( size, true );
    for ( UInt32 i=0; i<size; ++i )
        static_cast< UInt8* >( data.GetBufferPtr() )[ i ] = (UInt8) ( i * 31 + 7 );
    data.SetDataSize( size );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    RTS_REQUIRE_TRUE( f.client->ResolveMethod( g_importPath, ref, status ) );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
    RTS_REQUIRE_TRUE( !stream.IsNULL() );
    COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
    chunk.SetBytes( 1, data );
    RTS_ASSERT_TRUE( stream->Write( chunk ) );
    RTS_ASSERT_TRUE( stream->WritesDone() );
    CORE::CFutureResult small = f.client->CallAsync( g_getPath, GetRequest( f, 3 ) );
    bool smallFirst = false;
    UInt64 start = MT::PrecisionTickCount();
    while ( !stream->IsDone() && ( MT::PrecisionTickCount() - start ) < 30 * MT::PrecisionTimerResolution() )
    {
        f.pulseGenerator->ForceDirectPulse();
        if ( small.IsReady() && !stream->IsDone() )
            smallFirst = true;
    }
    RTS_ASSERT_TRUE( stream->IsDone() );
    RTS_ASSERT_OK( stream->GetStatus() );
    RTS_ASSERT_TRUE( 1 == stream->GetResponse().GetUInt32( 1 ) );
    RTS_ASSERT_TRUE( size == f.service.importedBytes );
    RTS_ASSERT_TRUE( f.PumpUntilReady( small, 3000 ) );
    RTS_ASSERT_TRUE( smallFirst );
    RTS_ASSERT_TRUE( f.clientWire->CountSent( GUCEF_RPC_FRAME_CONTINUATION ) >= size / 4096 );
    GUCEF::vector< CRpcLossyTransport::SFrame > frames;
    f.clientWire->GetSent( frames );
    bool withinLimit = true;
    for ( UInt32 i=0; i<frames.size(); ++i )
        withinLimit = withinLimit && frames[ i ].bytes.GetDataSize() <= 4096;
    RTS_ASSERT_TRUE( withinLimit );
}

/*-------------------------------------------------------------------------*/

inline void
TestFragmentationLarge( CIRpcConformanceTransports& transports )
{
    // 10 MB in 4 KB frames in every message kind that fragments: REQUEST (Save), RESPONSE (Get), the caller's and the
    // provider's STREAM_MSG (Edit echoes), RESULT (accepted Reindex)
    CRpcEndpointFixture f( &transports );
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.clientSettings.limits.maxFrameSize = 4096;
    f.serverSettings.limits.maxFrameSize = 4096;
    RTS_REQUIRE_TRUE( f.Connect( notes, notes ) );
    const UInt32 size = 10 * 1024 * 1024;
    CORE::CDynamicBuffer data( size, true );
    for ( UInt32 i=0; i<size; ++i )
        static_cast< UInt8* >( data.GetBufferPtr() )[ i ] = (UInt8) ( i * 31 + 7 );
    data.SetDataSize( size );
    COMCORE::CRpcCallOptions options;
    options.deadlineMs = 120000;

    // REQUEST
    COMCORE::CRpcMessage save = f.NewClientRequest( g_savePath );
    save.GetMutableMessage( 1 )->SetUInt64( 1, 77 );
    save.GetMutableMessage( 1 )->SetString( 2, "large" );
    save.GetMutableMessage( 1 )->SetBytes( 10, data );
    COMCORE::CRpcCallResult result = f.CallAndPump( g_savePath, save, options, 120000 );
    RTS_ASSERT_OK( result.status );
    RTS_ASSERT_TRUE( size == f.service.savedAttachmentBytes );

    // RESPONSE
    f.service.attachmentBytes = size;
    result = f.CallAndPump( g_getPath, GetRequest( f, 78 ), options, 120000 );
    f.service.attachmentBytes = 0;
    RTS_ASSERT_OK( result.status );
    CORE::CVariant attachment;
    RTS_ASSERT_TRUE( result.response.Get( 10, attachment ) && size == attachment.ByteSize( false ) &&
                     0 == memcmp( attachment.AsVoidPtr(), data.GetConstBufferPtr(), size ) );

    // STREAM_MSG of the caller and, echoed, of the provider
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    RTS_REQUIRE_TRUE( f.client->ResolveMethod( g_editPath, ref, status ) );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
    RTS_REQUIRE_TRUE( !stream.IsNULL() );
    COMCORE::CRpcMessage op = f.client->NewRequest( ref );
    op.SetUInt64( 1, 79 );
    op.Set( 3, CORE::CVariant( data.GetConstBufferPtr(), size, GUCEF_DATATYPE_BINARY_BLOB ) );
    RTS_ASSERT_TRUE( stream->Write( op ) );
    COMCORE::CRpcMessage echoed;
    RTS_ASSERT_TRUE( stream->ReadNext( echoed, status, 120000 ) );
    CORE::CVariant cell;
    RTS_ASSERT_TRUE( 79 == echoed.GetUInt64( 1 ) && echoed.Get( 3, cell ) && size == cell.ByteSize( false ) &&
                     0 == memcmp( cell.AsVoidPtr(), data.GetConstBufferPtr(), size ) );
    RTS_ASSERT_TRUE( stream->WritesDone() );
    UInt64 start = MT::PrecisionTickCount();
    while ( !stream->IsDone() && MT::PrecisionTickCount() - start < 30 * MT::PrecisionTimerResolution() )
    {
        f.pulseGenerator->ForceDirectPulse();
        transports.Pump();
    }
    RTS_ASSERT_TRUE( stream->IsDone() );

    // RESULT: the provider answers with a newer type that adds a 10 MB field; the caller keeps it as an unknown field
    COMCORE::CRpcCallOptions accept;
    accept.metadata.Set( "id-kind", CORE::CVariant( "number" ) );
    COMCORE::CRpcMessage reindex = f.NewClientRequest( g_reindexPath );
    reindex.SetBool( 1, true );
    CORE::CFutureResult future = f.client->CallAsync( g_reindexPath, reindex, accept );
    start = MT::PrecisionTickCount();
    while ( 0 == f.service.reindexAccepted && MT::PrecisionTickCount() - start < 10 * MT::PrecisionTimerResolution() )
    {
        f.pulseGenerator->ForceDirectPulse();
        transports.Pump();
    }
    RTS_REQUIRE_TRUE( 1 == f.service.reindexAccepted );
    COMCORE::CRpcContractBuilder builder( "gucef.test.wide", 1, 0 );
    builder.AddType( "WideReindexResult" );
    builder.AddField( "WideReindexResult", 1, "indexed", "uint64" );
    builder.AddField( "WideReindexResult", 3, "report", "bytes" );
    CORE::CString error;
    COMCORE::CRpcContractPtr wide = builder.Build( error );
    RTS_REQUIRE_TRUE( !wide.IsNULL() );
    COMCORE::CRpcMessage wideResult = wide->NewMessage( "WideReindexResult" );
    wideResult.SetUInt64( 1, 80 );
    wideResult.SetBytes( 3, data );
    RTS_ASSERT_TRUE( 1 == f.service.CompleteReindexWith( wideResult ) );
    RTS_ASSERT_TRUE( f.PumpUntilReady( future, 120000 ) );
    COMCORE::CRpcEndpoint::TryGetCallResult( future, result );
    RTS_ASSERT_OK( result.status );
    RTS_ASSERT_TRUE( result.wasAccepted && 80 == result.response.GetUInt64( 1 ) && 1 == result.response.GetUnknownFieldCount() );
    RTS_ASSERT_TRUE( result.response.GetEncodedSize() > size );

    // every frame within the limit, and the large messages did go in fragments
    RTS_ASSERT_TRUE( f.clientWire->CountSent( GUCEF_RPC_FRAME_CONTINUATION ) >= 2 * ( size / 4096 ) );
    RTS_ASSERT_TRUE( f.serverWire->CountSent( GUCEF_RPC_FRAME_CONTINUATION ) >= 3 * ( size / 4096 ) );
    GUCEF::vector< CRpcLossyTransport::SFrame > frames;
    f.serverWire->GetSent( frames );
    bool withinLimit = true;
    for ( UInt32 i=0; i<frames.size(); ++i )
        withinLimit = withinLimit && frames[ i ].bytes.GetDataSize() <= 4096;
    RTS_ASSERT_TRUE( withinLimit );
}

}; /* namespace RpcTransportSuite */

/*-------------------------------------------------------------------------*/

#define GUCEF_RPC_TRANSPORTSUITE_CASE( name, call )                 \
    GUCEF_TESTFW_TESTCASE( ( prefix + name ).C_String() )           \
    try                                                             \
    {                                                               \
        call;                                                       \
    }                                                               \
    catch ( ... )                                                   \
    {                                                               \
        GUCEF_TESTFW_ERRORHERE;                                     \
    }                                                               \
    GUCEF_TESTFW_TESTCASE_END;

inline void
PerformRpcTransportSuiteTests( CIRpcConformanceTransports& transports )
{
    GUCEF::CORE::CString prefix = GUCEF::CORE::CString( "rpc_transport (" ) + transports.GetName() + "): ";
    GUCEF_TESTFW_SUITE_SCOPE( "rpc_transport" );

    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "contract fetched from the peer, then taken from the disk cache" ), RpcTransportSuite::TestContractFetchThenCache( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "no contract names on the wire after binding" ), RpcTransportSuite::TestNoNamesOnWire( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "minor versions interoperate both ways" ), RpcTransportSuite::TestMinorInterop( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "another major is refused without a call being sent" ), RpcTransportSuite::TestMajorRefused( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "deadlines of caller and provider, cancellation" ), RpcTransportSuite::TestDeadlinesAndCancellation( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "resume continues both seq counters, a lost frame goes once more" ), RpcTransportSuite::TestResumeContinuesBothCounters( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "RESUME_GAP after DROP_OLDEST: a new session, idempotent calls issued again" ), RpcTransportSuite::TestResumeGap( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "a gap on a reliable transport suspends, the resume repairs it" ), RpcTransportSuite::TestGapOnReliableTransport( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "lossy, duplicating, reordering mail-like transport: every call once, streams past their credit in order" ), RpcTransportSuite::TestLossyMailLikeTransport( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "fragmentation: a 2 MB message in 4 KB frames, a small call passes it" ), RpcTransportSuite::TestFragmentation( transports ) );
    GUCEF_RPC_TRANSPORTSUITE_CASE( GUCEF::CORE::CString( "fragmentation: 10 MB in REQUEST, RESPONSE, both STREAM_MSG directions and RESULT" ), RpcTransportSuite::TestFragmentationLarge( transports ) );
}

#undef GUCEF_RPC_TRANSPORTSUITE_CASE
#undef RTS_ERRORHERE
#undef RTS_ASSERT_TRUE
#undef RTS_ASSERT_FALSE
#undef RTS_ASSERT_OK
#undef RTS_ASSERT_CODE
#undef RTS_REQUIRE_TRUE

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCTRANSPORTSUITE_H ? */
