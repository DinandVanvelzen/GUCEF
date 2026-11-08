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

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#include "gucefCOMCORE_CRpcDurableRequestTable.h"
#define GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H
#endif /* GUCEF_COMCORE_CRPCDURABLEREQUESTTABLE_H ? */

#ifndef GUCEF_COMCORE_CRPCFRAMECODEC_H
#include "gucefCOMCORE_CRpcFrameCodec.h"
#define GUCEF_COMCORE_CRPCFRAMECODEC_H
#endif /* GUCEF_COMCORE_CRPCFRAMECODEC_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRpcEndpointFixture.h"

#include "TestRpcConformance.h"

#include "TestRpcSession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

#define ASSERT_OK( s )      { COMCORE::CRpcStatus assertStatus_( s ); if ( !assertStatus_.IsOk() ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcSession test: line " << __LINE__ << ": " << assertStatus_.ToString() ); } }
#define ASSERT_CODE( c, s ) { COMCORE::CRpcStatus assertStatus_( s ); if ( c != assertStatus_.code ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcSession test: line " << __LINE__ << ": expected " << COMCORE::RpcStatusCodeToString( c ) << ", got " << assertStatus_.ToString() ); } }
#define REQUIRE_TRUE( t )   { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcSession test: requirement failed: " #t ); return; } }

/*-------------------------------------------------------------------------*/

namespace {

const char* g_getPath = "gucef.sample.notes.Notes/Get";
const char* g_watchPath = "gucef.sample.notes.Notes/Watch";
const char* g_importPath = "gucef.sample.notes.Notes/Import";
const char* g_reindexPath = "gucef.sample.notes.Notes/Reindex";
const char* g_savePath = "gucef.sample.notes.Notes/Save";

class CSessionEventCounter : public CORE::CObserver
{
    public:

    UInt32 established;
    UInt32 suspended;
    UInt32 resumed;
    UInt32 gaps;
    UInt32 closed;
    UInt32 accepted;
    COMCORE::CRpcSessionGapEventData lastGap;
    COMCORE::CRpcStatus lastClose;

    CSessionEventCounter( void ) : CORE::CObserver(), established( 0 ), suspended( 0 ), resumed( 0 ), gaps( 0 ), closed( 0 ), accepted( 0 ), lastGap(), lastClose() {}
    virtual ~CSessionEventCounter() { UnsubscribeAllFromObserver(); }

    void Watch( const COMCORE::CRpcEndpointPtr& endpoint )
    {
        typedef CORE::CTEventHandlerFunctor< CSessionEventCounter > TEventCallback;
        COMCORE::CRpcEndpointPtr e( endpoint );
        TEventCallback cb( this, &CSessionEventCounter::OnEvent );
        SubscribeTo( e.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionEstablishedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionSuspendedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionResumedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionGapEvent, cb );
        SubscribeTo( e.GetPointerAlways(), COMCORE::CRpcEndpoint::SessionClosedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), COMCORE::CRpcEndpoint::AcceptedEvent, cb );
    }

    private:

    void OnEvent( CORE::CNotifier*, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( COMCORE::CRpcEndpoint::SessionEstablishedEvent == eventId ) ++established;
        else if ( COMCORE::CRpcEndpoint::SessionSuspendedEvent == eventId ) ++suspended;
        else if ( COMCORE::CRpcEndpoint::SessionResumedEvent == eventId ) ++resumed;
        else if ( COMCORE::CRpcEndpoint::AcceptedEvent == eventId ) ++accepted;
        else if ( COMCORE::CRpcEndpoint::SessionClosedEvent == eventId )
        {
            ++closed;
            if ( GUCEF_NULL != eventData )
                lastClose = *static_cast< COMCORE::CRpcStatus* >( eventData );
        }
        else if ( COMCORE::CRpcEndpoint::SessionGapEvent == eventId )
        {
            ++gaps;
            if ( GUCEF_NULL != eventData )
                lastGap = *static_cast< COMCORE::CRpcSessionGapEventData* >( eventData );
        }
    }
};

bool
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

bool
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

COMCORE::CRpcMessage
GetRequest( CRpcEndpointFixture& f, UInt64 noteId )
{
    COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
    request.SetUInt64( 1, noteId );
    return request;
}

UInt64
TelemetryValue( const COMCORE::CRpcEndpointPtr& endpoint, const char* name )
{
    CORE::CDataNode telemetry;
    endpoint->GetTelemetry( telemetry );
    return telemetry.GetAttributeValue( name ).AsUInt64();
}

/** The seq of the first frame of the type the wire sent; 0 when none */
UInt64
FirstSeqOf( CRpcLossyTransport* wire, UInt8 type )
{
    GUCEF::vector< CRpcLossyTransport::SFrame > frames;
    wire->GetSent( frames );
    for ( UInt32 i=0; i<frames.size(); ++i )
        if ( frames[ i ].type == type )
            return frames[ i ].seq;
    return 0;
}

/** A new loopback pair for both endpoints; the old decorators stay readable through the out parameters */
bool
NewTransports( CRpcEndpointFixture& f, COMCORE::CIRpcTransportPtr& oldClient, COMCORE::CIRpcTransportPtr& oldServer )
{
    oldClient = f.clientTransport;
    oldServer = f.serverTransport;
    f.clientTransport.Unlink();
    f.serverTransport.Unlink();
    return f.MakeLoopbackTransports();
}

}

/*-------------------------------------------------------------------------*/

static void
TestResumeRefusedForOtherPrincipal( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    CSessionEventCounter clientEvents;
    clientEvents.Watch( f.client );
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );

    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    f.serverWire->hasPrincipalOverride = true;
    f.serverWire->principalOverride = COMCORE::CRpcPrincipal( "loopback", "someone-else", true );
    REQUIRE_TRUE( f.server->Rebind( f.serverTransport ) );
    REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    ASSERT_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_CLOSED, 3000 ) );
    ASSERT_TRUE( PumpUntil( f.pulseGenerator, clientEvents.closed, 1, 2000 ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_PERMISSION_DENIED, clientEvents.lastClose );
    RpcTestPump( 10 );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == f.server->GetState() );
    ASSERT_TRUE( 1 == TelemetryValue( f.server, "refusedResumes" ) );
}

/*-------------------------------------------------------------------------*/

static void
TestHostSessionUnknownAndDurable( void )
{
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    CRpcEndpointFixture f;
    COMCORE::CRpcHostSettings hostSettings;
    hostSettings.endpointSettings = f.serverSettings;
    hostSettings.endpointSettings.limits.resumeGraceMs = 100;
    COMCORE::CRpcHost host( f.pulseGenerator, hostSettings );
    REQUIRE_TRUE( host.GetServiceSet()->RegisterService( notes, "Notes", &f.service ) );
    REQUIRE_TRUE( host.Start() );
    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );

    // first session: a DURABLE accepted request, then the caller goes away for good
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    REQUIRE_TRUE( host.AcceptTransport( f.serverTransport ) );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
    ASSERT_TRUE( 1 == host.GetEndpointCount() );
    CSessionEventCounter clientEvents;
    clientEvents.Watch( f.client );
    COMCORE::CRpcCallOptions options;
    options.metadata.Set( "id-kind", CORE::CVariant( "string" ) );
    options.metadata.Set( "durable", CORE::CVariant( "true" ) );
    options.retryIdempotentOnNewSession = false;
    COMCORE::CRpcMessage request = f.NewClientRequest( g_reindexPath );
    request.SetBool( 1, true );
    CORE::CFutureResult future = f.client->CallAsync( g_reindexPath, request, options );
    REQUIRE_TRUE( PumpUntil( f.pulseGenerator, clientEvents.accepted, 1, 3000 ) );

    // the transport goes and the grace period passes: the host forgets the session
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    RpcTestPump( 400 );
    ASSERT_TRUE( 0 == host.GetEndpointCount() );

    // the provider finishes the durable work after its session ended
    ASSERT_TRUE( 1 == f.service.CompleteReindex( 99 ) || host.CompleteDurableResult( COMCORE::CRpcRequestId( CORE::CString( "job-1" ) ), COMCORE::CRpcStatus(), GUCEF_NULL ) );

    // the resume reaches a host that no longer knows the session: SESSION_UNKNOWN, a new session
    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    REQUIRE_TRUE( host.AcceptTransport( f.serverTransport ) );
    REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    REQUIRE_TRUE( PumpUntil( f.pulseGenerator, clientEvents.gaps, 1, 3000 ) );
    ASSERT_TRUE( clientEvents.lastGap.sessionUnknown );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
    ASSERT_TRUE( 1 == host.GetEndpointCount() );

    // the accepted call was not issued again (retryIdempotentOnNewSession off): it ended with the old session; the
    // result is asked for by its id
    ASSERT_TRUE( f.PumpUntilReady( future, 2000 ) );
    COMCORE::CRpcCallResult ended;
    COMCORE::CRpcEndpoint::TryGetCallResult( future, ended );
    ASSERT_CODE( COMCORE::RPC_STATUS_UNAVAILABLE, ended.status );
    COMCORE::CRpcRequestStatus requestStatus;
    COMCORE::CRpcStatus status;
    ASSERT_TRUE( f.client->QueryStatus( COMCORE::CRpcRequestId( CORE::CString( "job-1" ) ), true, requestStatus, status ) );
    ASSERT_OK( status );
    ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_DONE == requestStatus.state );
    ASSERT_TRUE( requestStatus.hasResult );
    if ( COMCORE::RPC_REQUESTSTATE_DONE != requestStatus.state )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: durable request state " << (UInt32) requestStatus.state << " " << requestStatus.status.ToString() );
    if ( requestStatus.hasResult && requestStatus.resultPayload.GetDataSize() > 0 )
    {
        COMCORE::CRpcMessage response( notes, notes->FindType( "ReindexResult" ) );
        COMCORE::CRpcStatus decodeStatus;
        ASSERT_TRUE( response.Decode( requestStatus.resultPayload.GetConstBufferPtr(), requestStatus.resultPayload.GetDataSize(), decodeStatus ) );
        ASSERT_TRUE( 99 == response.GetUInt64( 1 ) );
    }

    // a call on the new session works
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 7 ) );
    ASSERT_OK( result.status );
    f.client->Close( false );
    RpcTestPump( 10 );
    host.Stop();
    RpcTestPump( 10 );
}

/*-------------------------------------------------------------------------*/

static void
TestProtocolViolationsAbort( void )
{
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    {
        // reserved flag bits
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        CSessionEventCounter clientEvents;
        clientEvents.Watch( f.client );
        UInt8 frame[] = { GUCEF_RPC_FRAME_PING, 0x80, 0 };
        ASSERT_TRUE( f.clientWire->SendRaw( frame, sizeof( frame ) ) );
        ASSERT_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 2000 ) );
        ASSERT_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_CLOSED, 2000 ) );
        ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_ABORT ) );
        ASSERT_TRUE( PumpUntil( f.pulseGenerator, clientEvents.closed, 1, 2000 ) );
    }
    {
        // an acknowledgement of a frame never sent
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        COMCORE::CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_ACK, GUCEF_RPC_FRAMEFLAG_ACK, 0, true, 100000 );
        writer.WriteVarUInt32( 1024 * 1024 );
        writer.WriteVarUInt32( 0 );
        ASSERT_TRUE( f.clientWire->SendRaw( frame.GetConstBufferPtr(), frame.GetDataSize() ) );
        ASSERT_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 2000 ) );
        ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_ABORT ) );
    }
    {
        // a frame of a reserved type
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        UInt8 frame[] = { 0x7E, 0, 0 };
        ASSERT_TRUE( f.clientWire->SendRaw( frame, sizeof( frame ) ) );
        ASSERT_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 2000 ) );
    }
}

/*-------------------------------------------------------------------------*/

static void
TestSuspendedRefuseNew( void )
{
    // REFUSE_NEW (the default): calls beyond the retention while suspended are refused at once, nothing is lost
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.clientSettings.retentionMaxFrames = 2;
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    GUCEF::vector< CORE::CFutureResult > futures;
    for ( UInt32 i=0; i<4; ++i )
        futures.push_back( f.client->CallAsync( g_getPath, GetRequest( f, 70 + i ) ) );
    RpcTestPump( 3 );
    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
    REQUIRE_TRUE( f.server->Rebind( f.serverTransport ) );
    REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
    ASSERT_TRUE( PumpFutures( f, futures, 5000 ) );
    UInt32 ok = 0;
    UInt32 refused = 0;
    for ( UInt32 i=0; i<futures.size(); ++i )
    {
        COMCORE::CRpcCallResult result;
        COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result );
        if ( result.status.IsOk() )
            ++ok;
        else if ( COMCORE::RPC_STATUS_UNAVAILABLE == result.status.code )
            ++refused;
    }
    ASSERT_TRUE( 4 == ok + refused );
    ASSERT_TRUE( ok >= 1 && refused >= 1 );
    ASSERT_TRUE( 0 == TelemetryValue( f.client, "droppedFrames" ) );
}

/*-------------------------------------------------------------------------*/

namespace {

/** The first frame of the type the wire sent, empty when none */
CORE::CDynamicBuffer
FirstFrameOf( CRpcLossyTransport* wire, UInt8 type )
{
    GUCEF::vector< CRpcLossyTransport::SFrame > frames;
    wire->GetSent( frames );
    for ( UInt32 i=0; i<frames.size(); ++i )
        if ( frames[ i ].type == type )
            return frames[ i ].bytes;
    return CORE::CDynamicBuffer();
}

/** Resumes the suspended pair over new transports: the endpoints' own resume */
bool
ResumeOverNewTransports( CRpcEndpointFixture& f )
{
    COMCORE::CIRpcTransportPtr oldClient;
    COMCORE::CIRpcTransportPtr oldServer;
    if ( !NewTransports( f, oldClient, oldServer ) || !f.server->Rebind( f.serverTransport ) || !f.client->Rebind( f.clientTransport ) )
        return false;
    return f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_ESTABLISHED, 3000 ) &&
           f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_ESTABLISHED, 3000 );
}

/**
 *  Offers a resume HELLO to the suspended acceptor over a new transport of its own, as a third party would: the
 *  HELLO_ACK it answers with
 */
bool
OfferResumeHello( CRpcEndpointFixture& f, const CORE::CDynamicBuffer& hello, COMCORE::SRpcHelloAck& ack )
{
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    if ( !COMCORE::CRpcLoopbackTransport::CreatePair( f.pulseGenerator, f.pulseGenerator, a, b ) )
        return false;
    CRpcLossyTransport* injectWire = GUCEF_NULL;
    CRpcLossyTransport* acceptWire = GUCEF_NULL;
    COMCORE::CIRpcTransportPtr inject = CRpcLossyTransport::Wrap( a, &injectWire );
    COMCORE::CIRpcTransportPtr accept = CRpcLossyTransport::Wrap( b, &acceptWire );
    if ( !f.server->Rebind( accept ) || !injectWire->SendRaw( hello.GetConstBufferPtr(), hello.GetDataSize() ) )
        return false;
    UInt64 start = MT::PrecisionTickCount();
    while ( 0 == acceptWire->CountSent( GUCEF_RPC_FRAME_HELLO_ACK ) && MT::PrecisionTickCount() - start < 3 * MT::PrecisionTimerResolution() )
    {
        f.pulseGenerator->ForceDirectPulse();
        MT::PrecisionDelay( 1 );
    }
    CORE::CDynamicBuffer frame = FirstFrameOf( acceptWire, GUCEF_RPC_FRAME_HELLO_ACK );
    COMCORE::SRpcFrameHeader header;
    const UInt8* bytes = static_cast< const UInt8* >( frame.GetConstBufferPtr() );
    if ( 0 == frame.GetDataSize() || !COMCORE::CRpcFrameCodec::ParseHeader( bytes, frame.GetDataSize(), header ) )
        return false;
    CORE::CBinaryWireReader reader( bytes + header.bodyOffset, frame.GetDataSize() - header.bodyOffset );
    bool read = COMCORE::CRpcFrameCodec::ReadHelloAck( reader, ack );
    RpcTestPump( 5 );
    return read;
}

}

/*-------------------------------------------------------------------------*/

static void
TestResumeRefusedForBadProofAndReplayedCount( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );

    // a resume that works; its HELLO is kept
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( ResumeOverNewTransports( f ) );
    CORE::CDynamicBuffer usedHello = FirstFrameOf( f.clientWire, GUCEF_RPC_FRAME_HELLO );
    REQUIRE_TRUE( usedHello.GetDataSize() > 0 );

    // the next outage: two forged attempts before the real resume
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    UInt64 refusedBefore = TelemetryValue( f.server, "refusedResumes" );

    // a proof that is not the token's: the HELLO with one byte of its proof changed
    COMCORE::SRpcFrameHeader header;
    const UInt8* bytes = static_cast< const UInt8* >( usedHello.GetConstBufferPtr() );
    REQUIRE_TRUE( COMCORE::CRpcFrameCodec::ParseHeader( bytes, usedHello.GetDataSize(), header ) );
    CORE::CBinaryWireReader reader( bytes + header.bodyOffset, usedHello.GetDataSize() - header.bodyOffset );
    COMCORE::SRpcHello hello;
    REQUIRE_TRUE( COMCORE::CRpcFrameCodec::ReadHello( reader, hello ) && GUCEF_RPC_HELLO_RESUME == hello.mode );
    hello.resumeCount += 5;
    hello.proof[ 0 ] ^= 0x5A;
    CORE::CDynamicBuffer forged;
    CORE::CBinaryWireWriter writer( forged );
    COMCORE::CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO, 0, 0, false, 0 );
    COMCORE::CRpcFrameCodec::WriteHello( writer, hello );
    COMCORE::SRpcHelloAck ack;
    REQUIRE_TRUE( OfferResumeHello( f, forged, ack ) );
    ASSERT_TRUE( GUCEF_RPC_HELLOACK_REFUSED == ack.result );
    ASSERT_CODE( COMCORE::RPC_STATUS_PERMISSION_DENIED, ack.refusal );
    ASSERT_TRUE( ack.refusal.message.HasSubstr( "proof" ) >= 0 );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == f.server->GetState() );

    // the HELLO of the resume before, with its valid proof: a resume count that was used already
    COMCORE::SRpcHelloAck replayAck;
    REQUIRE_TRUE( OfferResumeHello( f, usedHello, replayAck ) );
    ASSERT_TRUE( GUCEF_RPC_HELLOACK_REFUSED == replayAck.result );
    ASSERT_CODE( COMCORE::RPC_STATUS_PERMISSION_DENIED, replayAck.refusal );
    ASSERT_TRUE( replayAck.refusal.message.HasSubstr( "count" ) >= 0 );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == f.server->GetState() );
    ASSERT_TRUE( refusedBefore + 2 == TelemetryValue( f.server, "refusedResumes" ) );

    // the session was kept: its own initiator resumes it
    UInt8 sessionBefore[ 16 ];
    UInt8 sessionAfter[ 16 ];
    REQUIRE_TRUE( f.client->GetSessionId( sessionBefore ) );
    REQUIRE_TRUE( ResumeOverNewTransports( f ) );
    ASSERT_TRUE( f.client->GetSessionId( sessionAfter ) && 0 == memcmp( sessionBefore, sessionAfter, 16 ) );
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 3 ) );
    ASSERT_OK( result.status );
}

/*-------------------------------------------------------------------------*/

static void
TestMetadataInterningAcrossResume( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );

    // a key defined before the outage, another one in a call made while suspended (retransmitted at the resume)
    COMCORE::CRpcCallOptions first;
    first.metadata.Set( "x-before-outage", CORE::CVariant( "one" ) );
    ASSERT_OK( f.CallAndPump( g_getPath, GetRequest( f, 1 ), first ).status );
    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    COMCORE::CRpcCallOptions both;
    both.metadata.Set( "x-before-outage", CORE::CVariant( "one" ) );
    both.metadata.Set( "x-while-suspended", CORE::CVariant( "two" ) );
    CORE::CFutureResult waiting = f.client->CallAsync( g_getPath, GetRequest( f, 2 ), both );
    REQUIRE_TRUE( ResumeOverNewTransports( f ) );
    ASSERT_TRUE( f.PumpUntilReady( waiting, 3000 ) );
    COMCORE::CRpcCallResult result;
    ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( waiting, result ) );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "one" == f.service.lastMetadata.GetString( "x-before-outage" ) );
    ASSERT_TRUE( "two" == f.service.lastMetadata.GetString( "x-while-suspended" ) );

    // after the resume both keys go as their interned index, and both sides still agree on what they stand for
    f.clientWire->ClearCapture();
    f.service.lastMetadata = COMCORE::CRpcMetadata();
    both.metadata.Set( "x-while-suspended", CORE::CVariant( "three" ) );
    result = f.CallAndPump( g_getPath, GetRequest( f, 3 ), both );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "one" == f.service.lastMetadata.GetString( "x-before-outage" ) );
    ASSERT_TRUE( "three" == f.service.lastMetadata.GetString( "x-while-suspended" ) );
    ASSERT_FALSE( f.clientWire->SentContains( 0, "x-before-outage", 15 ) );
    ASSERT_FALSE( f.clientWire->SentContains( 0, "x-while-suspended", 17 ) );
}

/*-------------------------------------------------------------------------*/

static void
TestWindowAndSendQueueLimits( void )
{
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    CORE::CString body;
    for ( UInt32 i=0; i<1000; ++i )
        body += (char) ( 'a' + i % 26 );
    {
        // the receive window of the provider: unacknowledged request bytes stay within it while its answers (and
        // acknowledgements) are held back
        CRpcEndpointFixture f;
        f.serverSettings.limits.receiveWindowBytes = 4096;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        f.serverWire->lossyControlFrames = true;
        f.serverWire->delayMs = 400;
        f.clientWire->ClearCapture();
        GUCEF::vector< CORE::CFutureResult > futures;
        for ( UInt32 i=0; i<12; ++i )
        {
            COMCORE::CRpcMessage save = f.NewClientRequest( g_savePath );
            save.GetMutableMessage( 1 )->SetUInt64( 1, i + 1 );
            save.GetMutableMessage( 1 )->SetString( 3, body );
            futures.push_back( f.client->CallAsync( g_savePath, save ) );
        }
        RpcTestPump( 100 );
        GUCEF::vector< CRpcLossyTransport::SFrame > frames;
        f.clientWire->GetSent( frames );
        UInt32 requestBytes = 0;
        UInt32 requests = 0;
        for ( UInt32 i=0; i<frames.size(); ++i )
            if ( GUCEF_RPC_FRAME_REQUEST == frames[ i ].type )
            {
                requestBytes += frames[ i ].bytes.GetDataSize();
                ++requests;
            }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: within the window " << requests << " requests, " << requestBytes << " bytes" );
        ASSERT_TRUE( requests >= 1 && requests < 12 );
        ASSERT_TRUE( requestBytes <= 4096 );
        f.serverWire->delayMs = 0;
        ASSERT_TRUE( PumpFutures( f, futures, 5000 ) );
        UInt32 ok = 0;
        for ( UInt32 i=0; i<futures.size(); ++i )
        {
            COMCORE::CRpcCallResult result;
            if ( COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result ) && result.status.IsOk() )
                ++ok;
        }
        ASSERT_TRUE( 12 == ok );
    }
    {
        // the send queue behind the window: a call that does not fit in it is refused at once, nothing waits forever
        CRpcEndpointFixture f;
        f.serverSettings.limits.receiveWindowBytes = 4096;
        f.clientSettings.sendQueueMaxBytes = 6000;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        f.serverWire->lossyControlFrames = true;
        f.serverWire->delayMs = 400;
        GUCEF::vector< CORE::CFutureResult > futures;
        UInt32 refusedAtOnce = 0;
        for ( UInt32 i=0; i<20; ++i )
        {
            COMCORE::CRpcMessage save = f.NewClientRequest( g_savePath );
            save.GetMutableMessage( 1 )->SetUInt64( 1, i + 1 );
            save.GetMutableMessage( 1 )->SetString( 3, body );
            CORE::CFutureResult future = f.client->CallAsync( g_savePath, save );
            COMCORE::CRpcCallResult result;
            if ( future.IsReady() && COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) && COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED == result.status.code )
                ++refusedAtOnce;
            futures.push_back( future );
        }
        f.serverWire->delayMs = 0;
        ASSERT_TRUE( PumpFutures( f, futures, 5000 ) );
        UInt32 ok = 0;
        UInt32 exhausted = 0;
        for ( UInt32 i=0; i<futures.size(); ++i )
        {
            COMCORE::CRpcCallResult result;
            COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result );
            if ( result.status.IsOk() )
                ++ok;
            else if ( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED == result.status.code )
                ++exhausted;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: send queue: " << ok << " answered, " << exhausted << " refused (" << refusedAtOnce << " at once)" );
        ASSERT_TRUE( 20 == ok + exhausted );
        ASSERT_TRUE( ok >= 4 && exhausted >= 1 );
        ASSERT_TRUE( refusedAtOnce == exhausted );
    }
}

/*-------------------------------------------------------------------------*/

static void
TestAckDelayOnOneWayTraffic( void )
{
    // one-way traffic: the provider acknowledges once ackEveryFrames (32) frames arrived, else after ackDelayMs (50),
    // never each frame; nothing goes back in between
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    COMCORE::CRpcTransportProfile profile = COMCORE::CRpcTransportProfile::GetProfile( "tcp" );
    profile.ackDelayMs = 50;
    profile.ackEveryFrames = 32;
    profile.keepaliveIntervalMs = 0;
    f.serverSettings.useTransportProfile = false;
    f.serverSettings.profile = profile;
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( g_importPath, ref, status ) );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
    REQUIRE_TRUE( !stream.IsNULL() );
    RpcTestPump( 100 );
    f.serverWire->ClearCapture();

    // ten frames: below ackEveryFrames, so the ACK waits for the delay
    COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
    char data[ 16 ];
    memset( data, 'x', sizeof( data ) );
    chunk.SetBytes( 1, data, sizeof( data ) );
    for ( UInt32 i=0; i<10; ++i )
        ASSERT_TRUE( stream->Write( chunk ) );
    for ( UInt32 i=0; i<3; ++i )
        f.pulseGenerator->ForceDirectPulse();
    UInt32 earlyAcks = f.serverWire->CountSent( GUCEF_RPC_FRAME_ACK );
    RpcTestPump( 150 );
    UInt32 delayedAcks = f.serverWire->CountSent( GUCEF_RPC_FRAME_ACK );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: ACKs for 10 frames: " << earlyAcks << " at once, " << delayedAcks << " after the delay" );
    ASSERT_TRUE( 0 == earlyAcks );
    ASSERT_TRUE( 1 == delayedAcks );

    // forty more at once: past ackEveryFrames the ACK goes at the next pulse, without the delay; the frames that
    // arrived together are acknowledged together
    f.serverWire->ClearCapture();
    for ( UInt32 i=0; i<40; ++i )
        ASSERT_TRUE( stream->Write( chunk ) );
    UInt64 start = MT::PrecisionTickCount();
    for ( UInt32 i=0; i<3; ++i )
        f.pulseGenerator->ForceDirectPulse();
    UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
    UInt32 promptAcks = f.serverWire->CountSent( GUCEF_RPC_FRAME_ACK );
    RpcTestPump( 150 );
    UInt32 acks = f.serverWire->CountSent( GUCEF_RPC_FRAME_ACK );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: ACKs for 40 frames: " << promptAcks << " within " << elapsedMs << " ms, " << acks << " in all" );
    if ( elapsedMs < 50 )
        ASSERT_TRUE( 1 == promptAcks );
    ASSERT_TRUE( acks >= 1 && acks <= 2 );
    ASSERT_TRUE( stream->WritesDone() );
    COMCORE::CRpcMessage message;
    ASSERT_FALSE( stream->ReadNext( message, status, 3000 ) );
    ASSERT_OK( status );
    ASSERT_TRUE( 50 == f.service.importedChunks );
}

/*-------------------------------------------------------------------------*/

static void
TestLostStreamCreditIsSentAgain( void )
{
    // STREAM_CREDIT is a control frame without retransmission: on a transport that loses frames the caller sends it
    // again now and then, else a lost one stalls the stream for good
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    f.clientWire->reportUnreliable = true;
    f.serverWire->reportUnreliable = true;
    f.service.watchCount = 200;
    REQUIRE_TRUE( f.Connect( notes, notes ) );

    // the first two credits are lost: the provider stops after the initial credit (64)
    f.clientWire->dropFrameType = GUCEF_RPC_FRAME_STREAM_CREDIT;
    f.clientWire->dropFrameTypeCount = 2;
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
    CRpcStreamCollector collector;
    COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
    REQUIRE_TRUE( !stream.IsNULL() );
    collector.Watch( stream );
    ASSERT_TRUE( PumpUntil( f.pulseGenerator, collector.messages, 200, 5000 ) );
    ASSERT_TRUE( 0 == f.clientWire->dropFrameTypeCount );
    ASSERT_TRUE( 200 == collector.messages );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: lost credits: " << collector.messages << " messages, " << f.clientWire->CountSent( GUCEF_RPC_FRAME_STREAM_CREDIT ) << " STREAM_CREDIT frames sent" );
}

/*-------------------------------------------------------------------------*/

namespace {

/**
 *  A hand-driven peer: an acceptor endpoint over a loopback pair whose other end the test writes frames into, with a
 *  HELLO first and then session frames with consecutive seqs. For limits a well-behaved peer never reaches.
 */
class CRpcRawPeer
{
    public:

    CORE::PulseGeneratorPtr pulseGenerator;
    TThreadId previousDriver;
    COMCORE::CRpcContractRegistry registry;
    COMCORE::CRpcEndpointSettings settings;
    COMCORE::CIRpcTransportPtr wire;
    COMCORE::CIRpcTransportPtr endpointSide;
    COMCORE::CRpcEndpointPtr endpoint;
    CRpcNotesTestService service;
    CSessionEventCounter events;
    UInt64 nextSeq;

    CRpcRawPeer( void )
        : pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ), previousDriver( 0 ), registry(), settings(), wire()
        , endpointSide(), endpoint(), service(), events(), nextSeq( 1 )
    {
        previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
        registry.SetCacheDirectory( CORE::CString::Empty );
        settings.contractRegistry = &registry;
        settings.nodeName = "endpoint";
    }

    ~CRpcRawPeer()
    {
        if ( !endpoint.IsNULL() )
            endpoint->Close( false );
        RpcTestPump( 10 );
        endpoint.Unlink();
        wire.Unlink();
        endpointSide.Unlink();
        RpcTestPump( 5 );
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
    }

    /** The endpoint exports the notes contract and takes the HELLO of the peer: established */
    bool Start( void )
    {
        if ( !COMCORE::CRpcLoopbackTransport::CreatePair( pulseGenerator, pulseGenerator, wire, endpointSide ) )
            return false;
        endpoint = COMCORE::CRpcEndpoint::Create( pulseGenerator, settings );
        events.Watch( endpoint );
        if ( !endpoint->RegisterService( LoadSampleNotesContract( 1, 2 ), "Notes", &service, COMCORE::RPC_EXECUTION_CALLING_THREAD ) ||
             !endpoint->Start( endpointSide, false ) )
            return false;
        COMCORE::SRpcHello hello;
        hello.nodeName = "raw";
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        COMCORE::CRpcFrameCodec::WriteHeader( writer, GUCEF_RPC_FRAME_HELLO, 0, 0, false, 0 );
        COMCORE::CRpcFrameCodec::WriteHello( writer, hello );
        if ( !wire->SendFrame( frame.GetConstBufferPtr(), frame.GetDataSize(), COMCORE::RPC_PROTECTION_CONFIDENTIAL ) )
            return false;
        RpcTestPump( 20 );
        return COMCORE::RPC_SESSIONSTATE_ESTABLISHED == endpoint->GetState();
    }

    /** A session frame with the next seq */
    bool Send( UInt8 type, UInt8 flags, const CORE::CDynamicBuffer& body )
    {
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        COMCORE::CRpcFrameCodec::WriteHeader( writer, type, flags, nextSeq++, false, 0 );
        writer.WriteBytes( body.GetConstBufferPtr(), body.GetDataSize() );
        return wire->SendFrame( frame.GetConstBufferPtr(), frame.GetDataSize(), COMCORE::RPC_PROTECTION_CONFIDENTIAL );
    }
};

/** A RESULT for a request id nobody asked about, with payloadSize bytes of payload */
CORE::CDynamicBuffer
OrphanResultBody( UInt64 id, UInt32 payloadSize )
{
    CORE::CDynamicBuffer body;
    CORE::CBinaryWireWriter writer( body );
    writer.WriteUInt8( 1 );
    COMCORE::CRpcFrameCodec::WriteRequestId( writer, COMCORE::CRpcRequestId( id ) );
    COMCORE::CRpcFrameCodec::WriteStatus( writer, COMCORE::CRpcStatus::Ok );
    for ( UInt32 i=0; i<payloadSize; ++i )
        writer.WriteUInt8( (UInt8) i );
    return body;
}

}

/*-------------------------------------------------------------------------*/

static void
TestMetadataKeysOfUnsentRequestsAreNotInterned( void )
{
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    {
        // a call refused for its metadata (beyond 8 KB) defined a key the peer never received: the next call with that
        // key used the interned index and the peer ended the session ("unknown interned metadata key 64")
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        COMCORE::CRpcCallOptions big;
        CORE::CString longValue;
        for ( UInt32 i=0; i<9000; ++i )
            longValue += 'v';
        big.metadata.Set( "x-new-key", CORE::CVariant( longValue ) );
        COMCORE::CRpcCallResult refused = f.CallAndPump( g_getPath, GetRequest( f, 5 ), big );
        ASSERT_CODE( COMCORE::RPC_STATUS_INVALID_ARGUMENT, refused.status );
        COMCORE::CRpcCallOptions small;
        small.metadata.Set( "x-new-key", CORE::CVariant( "v" ) );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 6 ), small );
        ASSERT_OK( result.status );
        ASSERT_TRUE( "v" == f.service.lastMetadata.GetString( "x-new-key" ) );
        RpcTestPump( 20 );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == f.server->GetState() );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == f.client->GetState() );
    }
    {
        // a call refused because the retention of the suspended session is full (REFUSE_NEW) likewise
        CRpcEndpointFixture f;
        f.clientSettings.retentionMaxFrames = 1;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        f.clientWire->ForceDisconnect();
        REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
        REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
        CORE::CFutureResult kept = f.client->CallAsync( g_getPath, GetRequest( f, 7 ) );
        COMCORE::CRpcCallOptions keyed;
        keyed.metadata.Set( "x-suspended-key", CORE::CVariant( "first" ) );
        CORE::CFutureResult refused = f.client->CallAsync( g_getPath, GetRequest( f, 8 ), keyed );
        REQUIRE_TRUE( f.PumpUntilReady( refused, 2000 ) );
        COMCORE::CRpcCallResult result;
        COMCORE::CRpcEndpoint::TryGetCallResult( refused, result );
        ASSERT_CODE( COMCORE::RPC_STATUS_UNAVAILABLE, result.status );
        REQUIRE_TRUE( ResumeOverNewTransports( f ) );
        ASSERT_TRUE( f.PumpUntilReady( kept, 3000 ) );
        keyed.metadata.Set( "x-suspended-key", CORE::CVariant( "second" ) );
        result = f.CallAndPump( g_getPath, GetRequest( f, 9 ), keyed );
        ASSERT_OK( result.status );
        ASSERT_TRUE( "second" == f.service.lastMetadata.GetString( "x-suspended-key" ) );
        RpcTestPump( 20 );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == f.server->GetState() );
    }
}

/*-------------------------------------------------------------------------*/

static void
TestAcceptorAnnouncesCreditsAfterResume( void )
{
    // the acceptor's STREAM_CREDIT frames go into the dying link: unless it announces its limits again after the
    // resume (8.3), the caller's client stream stays at its old limit for good
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( g_importPath, ref, status ) );
    COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
    chunk.Set( 1, CORE::CVariant( "x" ) );
    COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
    REQUIRE_TRUE( !stream.IsNULL() );

    f.serverWire->dropFrameType = GUCEF_RPC_FRAME_STREAM_CREDIT;
    f.serverWire->dropFrameTypeCount = 1000000;
    UInt32 before = 0;
    while ( before < 1000 && stream->Write( chunk ) )
    {
        ++before;
        RpcTestPump( 1 );
    }
    RpcTestPump( 50 );
    ASSERT_TRUE( before > 0 && before < 1000 );

    f.clientWire->ForceDisconnect();
    REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
    REQUIRE_TRUE( ResumeOverNewTransports( f ) );
    RpcTestPump( 100 );
    UInt32 after = 0;
    while ( after < 1000 && stream->Write( chunk ) )
    {
        ++after;
        RpcTestPump( 1 );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: client stream writes before the outage " << before << ", after the resume " << after );
    ASSERT_TRUE( after > 0 );
    ASSERT_TRUE( f.serverWire->CountSent( GUCEF_RPC_FRAME_STREAM_CREDIT ) >= 1 );
    ASSERT_TRUE( stream->WritesDone() );
    UInt64 start = MT::PrecisionTickCount();
    while ( !stream->IsDone() && ( MT::PrecisionTickCount() - start ) < 5 * MT::PrecisionTimerResolution() )
        f.pulseGenerator->ForceDirectPulse();
    ASSERT_TRUE( stream->IsDone() );
    ASSERT_OK( stream->GetStatus() );
}

/*-------------------------------------------------------------------------*/

static void
TestControlFramesArrivingPublicAreRefused( void )
{
    // on a transport with protection classes per frame only call payload frames may arrive PUBLIC (14.3): a control
    // frame that does is a downgrade, refused before its ack is applied, and the transport hears of it
    const UInt8 types[] = { GUCEF_RPC_FRAME_PING, GUCEF_RPC_FRAME_STREAM_CREDIT, GUCEF_RPC_FRAME_ACK };
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    for ( UInt32 t=0; t<sizeof( types ); ++t )
    {
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        f.serverWire->tamperProtection = true;
        CORE::CDynamicBuffer frame;
        CORE::CBinaryWireWriter writer( frame );
        COMCORE::CRpcFrameCodec::WriteHeader( writer, types[ t ], 0, 0, GUCEF_RPC_FRAME_ACK == types[ t ], 0 );
        writer.WriteVarUInt64( GUCEF_RPC_FRAME_ACK == types[ t ] ? 1024 * 1024 : 999 );
        writer.WriteVarUInt64( GUCEF_RPC_FRAME_ACK == types[ t ] ? 0 : 1000000 );
        ASSERT_TRUE( f.clientWire->SendRaw( frame.GetConstBufferPtr(), frame.GetDataSize() ) );
        ASSERT_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 2000 ) );
        ASSERT_TRUE( 1 == f.serverWire->downgradeRefusals );
        ASSERT_TRUE( 1 == TelemetryValue( f.server, "protectionDowngrades" ) );
        ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_ABORT ) );
    }
}

/*-------------------------------------------------------------------------*/

static void
TestPeerBeyondMaxBindingsIsAborted( void )
{
    // BIND frames with fresh indices for a contract nobody exports: every refusal was stored, without a bound
    CRpcRawPeer peer;
    peer.settings.limits.maxBindings = 8;
    REQUIRE_TRUE( peer.Start() );
    for ( UInt32 i=1; i<=200; ++i )
    {
        CORE::CDynamicBuffer body;
        CORE::CBinaryWireWriter writer( body );
        writer.WriteVarUInt32( 100 + i );
        writer.WriteString( "no.such.contract" );
        writer.WriteVarUInt32( 1 );
        writer.WriteVarUInt32( 0 );
        COMCORE::CRpcFrameCodec::WriteHash( writer, COMCORE::CRpcContractHash() );
        peer.Send( GUCEF_RPC_FRAME_BIND, 0, body );
        if ( 8 == i )
        {
            RpcTestPump( 10 );
            ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == peer.endpoint->GetState() );
            ASSERT_TRUE( 8 == TelemetryValue( peer.endpoint, "inBindings" ) );
        }
    }
    ASSERT_TRUE( PumpUntil( peer.pulseGenerator, peer.events.closed, 1, 2000 ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, peer.events.lastClose );
    ASSERT_TRUE( TelemetryValue( peer.endpoint, "inBindings" ) <= 8 );
}

/*-------------------------------------------------------------------------*/

static void
TestOrphanResultsAreNotKept( void )
{
    // RESULT frames for request ids no query asked about: every one was kept, with its payload, for good
    CRpcRawPeer peer;
    REQUIRE_TRUE( peer.Start() );
    for ( UInt32 i=1; i<=500; ++i )
    {
        peer.Send( GUCEF_RPC_FRAME_RESULT, GUCEF_RPC_FRAMEFLAG_RESPONDER, OrphanResultBody( 1000000 + i, 1000 ) );
        if ( 0 == i % 100 )
            RpcTestPump( 2 );
    }
    RpcTestPump( 20 );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == peer.endpoint->GetState() );
    ASSERT_TRUE( 500 == TelemetryValue( peer.endpoint, "orphanResults" ) );
    ASSERT_TRUE( 0 == TelemetryValue( peer.endpoint, "orphanResultsHeld" ) );

    // a status query that did not ask for the result again gets none of them with its answer
    UInt64 querySeq = peer.endpoint->GetNextSendSeq();
    CORE::CFutureResult query = peer.endpoint->QueryStatusAsync( COMCORE::CRpcRequestId( (UInt64) 1000001 ), false );
    CORE::CDynamicBuffer body;
    CORE::CBinaryWireWriter writer( body );
    writer.WriteVarUInt64( querySeq );
    COMCORE::CRpcFrameCodec::WriteRequestId( writer, COMCORE::CRpcRequestId( (UInt64) 1000001 ) );
    writer.WriteUInt8( (UInt8) COMCORE::RPC_REQUESTSTATE_DONE );
    writer.WriteVarUInt32( 1000 );
    writer.WriteVarUInt32( 0 );
    writer.WriteString( CORE::CString::Empty );
    peer.Send( GUCEF_RPC_FRAME_STATUS, GUCEF_RPC_FRAMEFLAG_RESPONDER, body );
    for ( UInt32 i=0; i<200 && !query.IsReady(); ++i )
        RpcTestPump( 1 );
    REQUIRE_TRUE( query.IsReady() );
    COMCORE::CRpcRequestStatus answer;
    REQUIRE_TRUE( COMCORE::CRpcEndpoint::TryGetRequestStatus( query, answer ) );
    ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_DONE == answer.state );
    ASSERT_FALSE( answer.hasResult );
}

/*-------------------------------------------------------------------------*/

static void
TestAssembliesStayWithinTheWindow( void )
{
    // first frames of fragmented messages that announce sizes within the maximum message size, but together beyond the
    // receive window (and the maximum message size): each reserved its full size at once
    CRpcRawPeer peer;
    peer.settings.limits.receiveWindowBytes = 1024 * 1024;
    peer.settings.limits.maxMessageSize = 4 * 1024 * 1024;
    REQUIRE_TRUE( peer.Start() );
    const UInt64 announced = 3 * 1024 * 1024 / 2;
    for ( UInt32 i=1; i<=3; ++i )
    {
        CORE::CDynamicBuffer body;
        CORE::CBinaryWireWriter writer( body );
        writer.WriteUInt8( 1 );
        COMCORE::CRpcFrameCodec::WriteRequestId( writer, COMCORE::CRpcRequestId( (UInt64) i ) );
        COMCORE::CRpcFrameCodec::WriteStatus( writer, COMCORE::CRpcStatus::Ok );
        writer.WriteVarUInt64( announced );
        for ( UInt32 b=0; b<100; ++b )
            writer.WriteUInt8( (UInt8) b );
        peer.Send( GUCEF_RPC_FRAME_RESULT, GUCEF_RPC_FRAMEFLAG_RESPONDER | GUCEF_RPC_FRAMEFLAG_MORE, body );
        RpcTestPump( 5 );
        if ( i < 3 )
        {
            ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == peer.endpoint->GetState() );
            ASSERT_TRUE( i * announced == TelemetryValue( peer.endpoint, "openAssemblyBytes" ) );
        }
    }
    ASSERT_TRUE( PumpUntil( peer.pulseGenerator, peer.events.closed, 1, 2000 ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, peer.events.lastClose );
}

/*-------------------------------------------------------------------------*/

static void
TestUnsolicitedContractIsDropped( void )
{
    // a CONTRACT no GET_CONTRACT asked for: it was registered (and written to the disk cache)
    CRpcRawPeer peer;
    REQUIRE_TRUE( peer.Start() );
    COMCORE::CRpcContractBuilder builder( "gucef.test.planted", 1, 0 );
    builder.AddType( "Thing" );
    builder.AddField( "Thing", 1, "id", "uint32" );
    CORE::CString error;
    COMCORE::CRpcContractPtr planted = builder.Build( error );
    REQUIRE_TRUE( !planted.IsNULL() );
    CORE::CDynamicBuffer body;
    CORE::CBinaryWireWriter writer( body );
    COMCORE::CRpcFrameCodec::WriteHash( writer, planted->GetHash() );
    COMCORE::CRpcFrameCodec::WriteStatus( writer, COMCORE::CRpcStatus::Ok );
    writer.WriteSizedBytes( planted->GetSchema() );
    writer.WriteVarUInt32( 0 );
    peer.Send( GUCEF_RPC_FRAME_CONTRACT, 0, body );
    RpcTestPump( 20 );
    ASSERT_TRUE( peer.registry.FindByHash( planted->GetHash() ).IsNULL() );
    ASSERT_TRUE( 1 == TelemetryValue( peer.endpoint, "unsolicitedContracts" ) );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == peer.endpoint->GetState() );
}

/*-------------------------------------------------------------------------*/

static void
TestRequireSecureTransportOnEveryTransport( void )
{
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    {
        // the initiator and the acceptor refuse an insecure transport to resume over, not only at Start()
        CRpcEndpointFixture f;
        f.clientSettings.requireSecureTransport = true;
        f.serverSettings.requireSecureTransport = true;
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        f.clientWire->ForceDisconnect();
        REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
        REQUIRE_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
        COMCORE::CIRpcTransportPtr oldClient;
        COMCORE::CIRpcTransportPtr oldServer;
        REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
        f.clientWire->forceInsecure = true;
        f.serverWire->forceInsecure = true;
        ASSERT_FALSE( f.server->Rebind( f.serverTransport ) );
        ASSERT_FALSE( f.client->Rebind( f.clientTransport ) );
        RpcTestPump( 20 );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == f.client->GetState() );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == f.server->GetState() );
        ASSERT_TRUE( 0 == f.clientWire->CountSent( GUCEF_RPC_FRAME_HELLO ) );

        // a secure one resumes
        REQUIRE_TRUE( ResumeOverNewTransports( f ) );
        ASSERT_OK( f.CallAndPump( g_getPath, GetRequest( f, 1 ) ).status );
    }
    {
        // a host's endpoint refuses a resume HELLO that arrives over an insecure transport; the session stays
        CRpcEndpointFixture f;
        COMCORE::CRpcHostSettings hostSettings;
        hostSettings.endpointSettings = f.serverSettings;
        hostSettings.endpointSettings.requireSecureTransport = true;
        COMCORE::CRpcHost host( f.pulseGenerator, hostSettings );
        REQUIRE_TRUE( host.GetServiceSet()->RegisterService( notes, "Notes", &f.service ) );
        REQUIRE_TRUE( host.Start() );
        f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        REQUIRE_TRUE( f.MakeLoopbackTransports() );
        REQUIRE_TRUE( host.AcceptTransport( f.serverTransport ) );
        f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
        f.client->AddWantedContract( "gucef.sample.notes", 1 );
        REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
        REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
        f.clientWire->ForceDisconnect();
        REQUIRE_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 2000 ) );
        RpcTestPump( 20 );
        COMCORE::CIRpcTransportPtr oldClient;
        COMCORE::CIRpcTransportPtr oldServer;
        REQUIRE_TRUE( NewTransports( f, oldClient, oldServer ) );
        f.serverWire->forceInsecure = true;
        REQUIRE_TRUE( host.AcceptTransport( f.serverTransport ) );
        REQUIRE_TRUE( f.client->Rebind( f.clientTransport ) );
        ASSERT_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_CLOSED, 3000 ) );
        ASSERT_TRUE( 1 == host.GetEndpointCount() );
        f.client.Unlink();
        host.Stop();
        RpcTestPump( 10 );
    }
}

/*-------------------------------------------------------------------------*/

namespace {

/** A loopback transport under another scheme name ("testloopback://<name>" connects to "loopback://<name>") */
class CTestSchemeLoopbackTransport : public CRpcLossyTransport
{
    public:

    explicit CTestSchemeLoopbackTransport( const COMCORE::CIRpcTransportPtr& inner ) : CRpcLossyTransport( inner ) {}

    virtual bool Connect( const CORE::CUri& remote ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CUri loopback( remote );
        loopback.SetScheme( "loopback" );
        return CRpcLossyTransport::Connect( loopback );
    }
};

/**
 *  The transports of the scheme "testloopback": secure loopback transports, or once insecure is set, loopback
 *  transports that report themselves insecure
 */
class CTestLoopbackFactory : public CORE::CTFactoryBase< COMCORE::CIRpcTransport, MT::CMutex >
{
    public:

    static const char* Scheme;
    MT::CTAtomic< UInt32 > insecure;
    MT::CTAtomic< UInt32 > createdInsecure;

    CTestLoopbackFactory( void )
        : CORE::CTFactoryBase< COMCORE::CIRpcTransport, MT::CMutex >(), insecure( 0 ), createdInsecure( 0 ), m_loopback()
    {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        CTestSchemeLoopbackTransport* transport = GUCEF_NEW CTestSchemeLoopbackTransport( m_loopback.Create() );
        if ( 0 != insecure.Load() )
        {
            transport->forceInsecure = true;
            createdInsecure.FetchAdd( 1 );
        }
        GUCEF::CORE::CTSharedPtr< CTestSchemeLoopbackTransport, MT::CMutex > shared( transport );
        COMCORE::CIRpcTransportPtr result;
        result = shared;
        return result;
    }

    virtual CORE::CString GetConcreteClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "CTestLoopbackFactory"; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CTestLoopbackFactory(); }

    private:

    CORE::CTFactory< COMCORE::CIRpcTransport, COMCORE::CRpcLoopbackTransport, MT::CMutex > m_loopback;
};

const char* CTestLoopbackFactory::Scheme = "testloopback";

/**
 *  Hands what its listener accepts to an endpoint inside the accept event, where the first frame is still to come: the
 *  first transport starts the endpoint, later ones are a resume (Rebind())
 */
class CAcceptedTransport : public CORE::CObserver
{
    public:

    COMCORE::CRpcEndpointPtr endpoint;
    COMCORE::CIRpcTransportPtr transport;
    UInt32 acceptedCount;

    CAcceptedTransport( void ) : CORE::CObserver(), endpoint(), transport(), acceptedCount( 0 ) {}
    virtual ~CAcceptedTransport() { UnsubscribeAllFromObserver(); }

    void Watch( COMCORE::CIRpcTransportListenerPtr listener )
    {
        typedef CORE::CTEventHandlerFunctor< CAcceptedTransport > TEventCallback;
        TEventCallback accepted( this, &CAcceptedTransport::OnAccepted );
        SubscribeTo( listener.GetPointerAlways(), COMCORE::CIRpcTransportListener::TransportAcceptedEvent, accepted );
    }

    private:

    void OnAccepted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData || endpoint.IsNULL() )
            return;
        transport = static_cast< COMCORE::CRpcTransportAcceptedEventData* >( eventData )->transport;
        if ( 0 == acceptedCount++ )
            endpoint->Start( transport, false );
        else
            endpoint->Rebind( transport );
    }
};

}

/*-------------------------------------------------------------------------*/

static void
TestRequireSecureTransportOnReconnect( void )
{
    // the endpoint's own reconnect (a remote URI and autoReconnect) refuses an insecure transport as Start() does: the
    // factory of the scheme makes insecure transports here, so the session stays suspended instead of resuming over one
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    TThreadId previousDriver = pulseGenerator->GetPulseDriverThreadId();
    pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    COMCORE::CRpcTransportFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory();
    static CTestLoopbackFactory testFactory;      // its transports may outlive this function on the client's protocol task
    testFactory.insecure.Store( 0 );
    testFactory.createdInsecure.Store( 0 );
    factory.RegisterConcreteFactory( CTestLoopbackFactory::Scheme, &testFactory );

    COMCORE::CIRpcTransportListenerPtr listener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "loopback" );
    REQUIRE_TRUE( !listener.IsNULL() );
    listener->SetPulseGenerator( pulseGenerator );
    CAcceptedTransport accepted;
    accepted.Watch( listener );
    REQUIRE_TRUE( listener->Listen( CORE::CUri( "loopback://fixa-reconnect" ) ) );

    COMCORE::CRpcContractRegistry clientRegistry;
    COMCORE::CRpcContractRegistry serverRegistry;
    clientRegistry.SetCacheDirectory( CORE::CString::Empty );
    serverRegistry.SetCacheDirectory( CORE::CString::Empty );
    clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    COMCORE::CRpcEndpointSettings clientSettings;
    COMCORE::CRpcEndpointSettings serverSettings;
    clientSettings.contractRegistry = &clientRegistry;
    serverSettings.contractRegistry = &serverRegistry;
    clientSettings.requireSecureTransport = true;
    clientSettings.autoReconnect = true;
    clientSettings.reconnectInitialDelayMs = 20;
    clientSettings.reconnectMaxDelayMs = 40;
    CRpcNotesTestService service;
    COMCORE::CRpcEndpointPtr server = COMCORE::CRpcEndpoint::Create( pulseGenerator, serverSettings );
    REQUIRE_TRUE( server->RegisterService( notes, "Notes", &service ) );
    accepted.endpoint = server;
    COMCORE::CRpcEndpointPtr client = COMCORE::CRpcEndpoint::Create( pulseGenerator, clientSettings );
    client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( client->Connect( CORE::CUri( "testloopback://fixa-reconnect" ) ) );
    UInt64 start = MT::PrecisionTickCount();
    while ( 0 == accepted.acceptedCount && MT::PrecisionTickCount() - start < 3 * MT::PrecisionTimerResolution() )
        RpcTestPump( 1 );
    REQUIRE_TRUE( 1 == accepted.acceptedCount );
    COMCORE::CIRpcTransportPtr serverTransport = accepted.transport;
    start = MT::PrecisionTickCount();
    while ( !client->IsContractBound( "gucef.sample.notes", 1 ) && MT::PrecisionTickCount() - start < 3 * MT::PrecisionTimerResolution() )
        RpcTestPump( 1 );
    REQUIRE_TRUE( client->IsContractBound( "gucef.sample.notes", 1 ) );

    // from now on the scheme's factory makes insecure transports; the connection goes
    testFactory.insecure.Store( 1 );
    serverTransport->Close();
    start = MT::PrecisionTickCount();
    while ( testFactory.createdInsecure.Load() < 3 && MT::PrecisionTickCount() - start < 5 * MT::PrecisionTimerResolution() )
        RpcTestPump( 5 );
    RpcTestPump( 100 );
    UInt32 acceptedInsecure = accepted.acceptedCount - 1;     // a reconnect that went through resumes over it
    COMCORE::TRpcSessionState state = client->GetState();
    UInt32 created = testFactory.createdInsecure.Load();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcSession test: reconnect attempts with insecure transports " << created << ", connected " << acceptedInsecure << ", client state " << COMCORE::RpcSessionStateToString( state ) );
    ASSERT_TRUE( created >= 1 );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == state );
    ASSERT_TRUE( 0 == acceptedInsecure );

    client->Close( false );
    server->Close( false );
    RpcTestPump( 20 );
    accepted.UnsubscribeAllFromObserver();
    accepted.endpoint.Unlink();
    accepted.transport.Unlink();
    client.Unlink();
    server.Unlink();
    serverTransport.Unlink();
    listener->StopListening();
    RpcTestPump( 10 );
    factory.UnregisterConcreteFactory( CTestLoopbackFactory::Scheme );
    pulseGenerator->SetPulseDriverThreadId( previousDriver );
}

/*-------------------------------------------------------------------------*/

static void
TestDurableIdsOfUnauthenticatedPrincipals( void )
{
    // an unauthenticated principal ("tcp:<address>:<port>") names a connection: a later connection that gets the same
    // address and port is another party and must not see the DURABLE ids of the first (10.7)
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    COMCORE::CRpcDurableRequestTablePtr table = COMCORE::CRpcDurableRequestTable::Create( 60000, 16 );
    COMCORE::CRpcPrincipal principal;
    principal.transport = "tcp";
    principal.subject = "tcp:127.0.0.1:40000";
    principal.isAuthenticated = false;

    CRpcEndpointFixture first;
    REQUIRE_TRUE( first.MakeLoopbackTransports() );
    first.serverWire->hasPrincipalOverride = true;
    first.serverWire->principalOverride = principal;
    REQUIRE_TRUE( first.Connect( notes, notes, COMCORE::RPC_EXECUTION_CALLING_THREAD, 1, false ) );
    first.server->SetDurableRequestTable( table );
    REQUIRE_TRUE( first.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
    COMCORE::CRpcCallOptions options;
    options.metadata.Set( "id-kind", CORE::CVariant( "string" ) );
    options.metadata.Set( "durable", CORE::CVariant( "true" ) );
    COMCORE::CRpcMessage request = first.NewClientRequest( g_reindexPath );
    request.SetBool( 1, true );
    CORE::CFutureResult future = first.client->CallAsync( g_reindexPath, request, options );
    REQUIRE_TRUE( PumpUntil( first.pulseGenerator, first.service.reindexAccepted, 1, 3000 ) );
    RpcTestPump( 10 );

    // the session that made it knows it
    COMCORE::CRpcRequestStatus requestStatus;
    COMCORE::CRpcStatus status;
    ASSERT_TRUE( first.client->QueryStatus( COMCORE::CRpcRequestId( CORE::CString( "job-1" ) ), false, requestStatus, status ) );
    ASSERT_OK( status );
    ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_UNKNOWN != requestStatus.state );

    // another connection with the same address and port does not
    CRpcEndpointFixture second;
    REQUIRE_TRUE( second.MakeLoopbackTransports() );
    second.serverWire->hasPrincipalOverride = true;
    second.serverWire->principalOverride = principal;
    REQUIRE_TRUE( second.Connect( notes, notes, COMCORE::RPC_EXECUTION_CALLING_THREAD, 1, false ) );
    second.server->SetDurableRequestTable( table );
    REQUIRE_TRUE( second.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
    ASSERT_TRUE( second.client->QueryStatus( COMCORE::CRpcRequestId( CORE::CString( "job-1" ) ), false, requestStatus, status ) );
    ASSERT_OK( status );
    ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_UNKNOWN == requestStatus.state );
    ASSERT_TRUE( second.client->CancelRequest( COMCORE::CRpcRequestId( CORE::CString( "job-1" ) ), status ) );
    ASSERT_TRUE( first.client->QueryStatus( COMCORE::CRpcRequestId( CORE::CString( "job-1" ) ), false, requestStatus, status ) );
    ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_CANCELLED != requestStatus.state && COMCORE::RPC_REQUESTSTATE_UNKNOWN != requestStatus.state );
    first.service.CompleteReindex( 1 );
    first.PumpUntilReady( future, 2000 );
}

/*-------------------------------------------------------------------------*/

#define RUN_RPC_CASE( name, call )                                  \
    GUCEF_TESTFW_TESTCASE( "rpc_session: " name )                   \
    try                                                             \
    {                                                               \
        call;                                                       \
    }                                                               \
    catch ( ... )                                                   \
    {                                                               \
        ERRORHERE;                                                  \
    }                                                               \
    GUCEF_TESTFW_TESTCASE_END;

void
PerformRpcSessionTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_session" );

    RUN_RPC_CASE( "a resume from another principal is refused, the session stays suspended", TestResumeRefusedForOtherPrincipal() );
    RUN_RPC_CASE( "host: SESSION_UNKNOWN after the grace period, durable result from the next session", TestHostSessionUnknownAndDurable() );
    RUN_RPC_CASE( "protocol violations end the session with ABORT", TestProtocolViolationsAbort() );
    RUN_RPC_CASE( "REFUSE_NEW while suspended refuses beyond the retention", TestSuspendedRefuseNew() );
    RUN_RPC_CASE( "a resume with a proof that is not the token's, or a resume count used before, is refused", TestResumeRefusedForBadProofAndReplayedCount() );
    RUN_RPC_CASE( "interned metadata keys survive a resume, also those of calls made while suspended", TestMetadataInterningAcrossResume() );
    RUN_RPC_CASE( "the peer's receive window and the send queue bound what is in flight", TestWindowAndSendQueueLimits() );
    RUN_RPC_CASE( "one-way traffic is acknowledged per ackEveryFrames frames or after ackDelayMs", TestAckDelayOnOneWayTraffic() );
    RUN_RPC_CASE( "a lost STREAM_CREDIT on an unreliable transport is sent again", TestLostStreamCreditIsSentAgain() );
    RUN_RPC_CASE( "metadata keys of a request that is not sent are not interned", TestMetadataKeysOfUnsentRequestsAreNotInterned() );
    RUN_RPC_CASE( "the acceptor announces its stream credits again after a resume", TestAcceptorAnnouncesCreditsAfterResume() );
    RUN_RPC_CASE( "control frames arriving PUBLIC are refused as a downgrade", TestControlFramesArrivingPublicAreRefused() );
    RUN_RPC_CASE( "a peer beyond maxBindings is aborted, refusals are not stored beyond it", TestPeerBeyondMaxBindingsIsAborted() );
    RUN_RPC_CASE( "RESULT frames nobody asked for again are not kept", TestOrphanResultsAreNotKept() );
    RUN_RPC_CASE( "fragmented messages stay within the receive window", TestAssembliesStayWithinTheWindow() );
    RUN_RPC_CASE( "a CONTRACT nothing asked for is dropped", TestUnsolicitedContractIsDropped() );
    RUN_RPC_CASE( "requireSecureTransport holds for every transport of the session", TestRequireSecureTransportOnEveryTransport() );
    RUN_RPC_CASE( "requireSecureTransport holds for the endpoint's own reconnect", TestRequireSecureTransportOnReconnect() );
    RUN_RPC_CASE( "DURABLE ids of an unauthenticated principal belong to its session lineage", TestDurableIdsOfUnauthenticatedPrincipals() );
}

/*-------------------------------------------------------------------------*/
