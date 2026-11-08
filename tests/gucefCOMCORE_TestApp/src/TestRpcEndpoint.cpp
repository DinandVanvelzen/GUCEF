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

#ifndef GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#include "gucefCOMCORE_CRpcRoleAccessPolicy.h"
#define GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H ? */

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#include "gucefCOMCORE_CRpcLoopbackTransportListener.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H ? */

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#include "gucefCOMCORE_CRpcProtocolTask.h"
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */

#ifndef GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#include "gucefCOMCORE_CRpcBuiltInContracts.h"
#define GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H
#endif /* GUCEF_COMCORE_CRPCBUILTINCONTRACTS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifdef GUCEF_LINUX_BUILD
#include <unistd.h>
#endif /* GUCEF_LINUX_BUILD ? */

#include "TestRpcEndpointFixture.h"

#include "TestRpcTransportSuite.h"

#include "TestRpcConformance.h"

#include "TestRpcEndpoint.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

namespace {

const char* g_getPath = "gucef.sample.notes.Notes/Get";
const char* g_savePath = "gucef.sample.notes.Notes/Save";
const char* g_watchPath = "gucef.sample.notes.Notes/Watch";
const char* g_importPath = "gucef.sample.notes.Notes/Import";
const char* g_editPath = "gucef.sample.notes.Notes/Edit";
const char* g_reindexPath = "gucef.sample.notes.Notes/Reindex";

UInt64
TelemetryValueOf( const COMCORE::CRpcEndpointPtr& endpoint, const char* name )
{
    CORE::CDataNode telemetry;
    endpoint->GetTelemetry( telemetry );
    return telemetry.GetAttributeValue( name ).AsUInt64();
}

/* the collectors of the conformance suite */
typedef CRpcEndpointEventRecorder CEndpointEventRecorder;
typedef CRpcStreamCollector CStreamCollector;

/** The conformance suite over loopback pairs: on the test thread's pulse generator, or each side on a task of its own */
class CLoopbackConformanceTransports : public CIRpcConformanceTransports
{
    public:

    explicit CLoopbackConformanceTransports( bool ownThreads ) : CIRpcConformanceTransports(), m_ownThreads( ownThreads ) {}

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return m_ownThreads ? "loopback, protocol on tasks of its own" : "loopback, protocol on the test thread";
    }

    virtual bool MakePair( COMCORE::CIRpcTransportPtr& initiator, COMCORE::CIRpcTransportPtr& acceptor ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::PulseGeneratorPtr pulseGenerator;
        if ( !m_ownThreads )
            pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        return COMCORE::CRpcLoopbackTransport::CreatePair( pulseGenerator, pulseGenerator, initiator, acceptor );
    }

    private:

    bool m_ownThreads;
};

bool
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

bool
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

void
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
    GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: " << text );
}

COMCORE::CRpcMessage
GetRequest( CRpcEndpointFixture& f, UInt64 noteId )
{
    COMCORE::CRpcMessage request = f.NewClientRequest( g_getPath );
    request.SetUInt64( 1, noteId );
    return request;
}

/** Every name of a contract that is at least four characters long (shorter ones occur by chance in binary data) */
void
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

/** Records the thread that delivers the pulses of a pulse generator */
class CPulseThreadRecorder : public CORE::CObserver
{
    public:

    CPulseThreadRecorder( void ) : CORE::CObserver(), m_lock(), m_threadId( 0 ) {}
    virtual ~CPulseThreadRecorder() { UnsubscribeAllFromObserver(); }

    void Watch( CORE::PulseGeneratorPtr pulseGenerator )
    {
        typedef CORE::CTEventHandlerFunctor< CPulseThreadRecorder > TEventCallback;
        TEventCallback cb( this, &CPulseThreadRecorder::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, cb );
    }

    TThreadId GetThreadId( void ) const { MT::CScopeMutex lock( m_lock ); return m_threadId; }

    private:

    void OnPulse( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        MT::CScopeMutex lock( m_lock );
        m_threadId = MT::GetCurrentThreadID();
    }

    mutable MT::CMutex m_lock;
    TThreadId m_threadId;
};

/** A service whose handler makes a nested blocking call on the protocol thread */
class CNestedCallService : public COMCORE::CRpcServiceImpl
{
    public:

    COMCORE::CRpcEndpointPtr endpoint;
    COMCORE::CRpcStatus nestedStatus;
    bool nestedReturned;

    CNestedCallService( void ) : COMCORE::CRpcServiceImpl(), endpoint(), nestedStatus(), nestedReturned( false )
    {
        BindMethod( "Get", GUCEF_NEW COMCORE::CTRpcMethodHandler< CNestedCallService >( this, &CNestedCallService::OnGet ) );
    }

    COMCORE::TRpcHandlerResult OnGet( COMCORE::CRpcServerCall& call )
    {
        COMCORE::CRpcMessage request = call.GetRequest();
        COMCORE::CRpcMessage response;
        nestedReturned = endpoint->Call( g_getPath, request, response, nestedStatus );
        COMCORE::CRpcMessage note = call.NewResponse();
        note.SetUInt64( 1, 1 );
        note.SetString( 2, "outer" );
        call.Reply( note );
        return COMCORE::RPC_HANDLER_DONE;
    }
};

}

#define ASSERT_OK( s )      { COMCORE::CRpcStatus assertStatus_( s ); if ( !assertStatus_.IsOk() ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: line " << __LINE__ << ": " << assertStatus_.ToString() ); } }
#define ASSERT_CODE( c, s ) { COMCORE::CRpcStatus assertStatus_( s ); if ( c != assertStatus_.code ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: line " << __LINE__ << ": expected " << COMCORE::RpcStatusCodeToString( c ) << ", got " << assertStatus_.ToString() ); } }
#define REQUIRE_TRUE( t )   { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: requirement failed: " #t ); return; } }

/*-------------------------------------------------------------------------*/

static void
TestUnaryCalls( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );

    // direct RESPONSE, metadata both ways through the intern table (a second call reuses the interned key)
    COMCORE::CRpcCallOptions options;
    options.metadata.Set( "x-trace", CORE::CVariant( "abc" ) );
    options.metadata.Set( "x-attempt", CORE::CVariant( (UInt32) 1 ) );
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ), options );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "note 5" == result.response.GetString( 2 ) );
    ASSERT_TRUE( "the secret body" == result.response.GetString( 3 ) );
    ASSERT_FALSE( result.wasAccepted );
    ASSERT_TRUE( "abc" == f.service.lastMetadata.GetString( "x-trace" ) );
    ASSERT_TRUE( "1" == f.service.lastMetadata.GetString( "x-attempt" ) );
    f.clientWire->ClearCapture();
    result = f.CallAndPump( g_getPath, GetRequest( f, 6 ), options );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "abc" == f.service.lastMetadata.GetString( "x-trace" ) );
    ASSERT_FALSE( f.clientWire->SentContains( 0, "x-trace", 7 ) );

    // an error status of the handler
    result = f.CallAndPump( g_getPath, GetRequest( f, 404 ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_NOT_FOUND, result.status );
    ASSERT_TRUE( "no note 404" == result.status.message );

    // a method the contract does not have is refused locally
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    ASSERT_FALSE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Delete", ref, status ) );
    ASSERT_FALSE( status.IsOk() );

    // the blocking form on the protocol thread pumps it
    COMCORE::CRpcMessage response;
    ASSERT_TRUE( f.client->Call( g_getPath, GetRequest( f, 7 ), response, status ) );
    ASSERT_OK( status );
    ASSERT_TRUE( 7 == response.GetUInt64( 1 ) );

    // Save carries a nested message
    COMCORE::CRpcMessage save = f.NewClientRequest( g_savePath );
    save.GetMutableMessage( 1 )->SetUInt64( 1, 77 );
    save.GetMutableMessage( 1 )->SetString( 2, "title" );
    result = f.CallAndPump( g_savePath, save );
    ASSERT_OK( result.status );
    ASSERT_TRUE( 77 == result.response.GetUInt64( 1 ) );
    ASSERT_TRUE( 1 == result.response.GetUInt64( 2 ) );

    // sensitive fields are redacted in the logged form, and nothing logged the secret
    CORE::CDataNode form;
    ASSERT_OK( result.status );
    COMCORE::CRpcCallResult note = f.CallAndPump( g_getPath, GetRequest( f, 8 ) );
    note.response.ToDataNode( form, true );
    ASSERT_TRUE( "***" == form.GetAttributeValue( "body" ).AsString() );

    // telemetry
    CORE::CDataNode telemetry;
    f.client->GetTelemetry( telemetry );
    ASSERT_TRUE( telemetry.GetAttributeValue( "framesSent" ).AsUInt64() > 0 );
    ASSERT_TRUE( telemetry.GetAttributeValue( "framesReceived" ).AsUInt64() > 0 );
    ASSERT_TRUE( "ESTABLISHED" == telemetry.GetAttributeValue( "state" ).AsString() );
    if ( "ESTABLISHED" != telemetry.GetAttributeValue( "state" ).AsString() )
        GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: telemetry state " << telemetry.GetAttributeValue( "state" ).AsString() << " endpoint state " << COMCORE::RpcSessionStateToString( f.client->GetState() ) );
}

/*-------------------------------------------------------------------------*/

static void
TestSensitiveNotLogged( void )
{
    TESTFW::CLogTextCapture capture;
    {
        CRpcEndpointFixture f;
        COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
        REQUIRE_TRUE( f.Connect( notes, notes ) );
        COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
        ASSERT_OK( result.status );
        COMCORE::CRpcMessage save = f.NewClientRequest( g_savePath );
        save.GetMutableMessage( 1 )->SetString( 3, "very private text" );
        result = f.CallAndPump( g_savePath, save );
        ASSERT_OK( result.status );
    }
    ASSERT_FALSE( capture.Contains( "the secret body" ) );
    ASSERT_FALSE( capture.Contains( "very private text" ) );
}

/*-------------------------------------------------------------------------*/

static void
TestPeerToPeer( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    CRpcNotesTestService clientService;
    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.serverRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    REQUIRE_TRUE( f.server->RegisterService( notes, "Notes", &f.service ) );
    REQUIRE_TRUE( f.client->RegisterService( notes, "Notes", &clientService ) );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    f.server->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( f.server->Start( f.serverTransport, false ) );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
    UInt64 start = MT::PrecisionTickCount();
    while ( !f.server->IsContractBound( "gucef.sample.notes", 1 ) && ( MT::PrecisionTickCount() - start ) < MT::PrecisionTimerResolution() * 2 )
        f.pulseGenerator->ForceDirectPulse();
    REQUIRE_TRUE( f.server->IsContractBound( "gucef.sample.notes", 1 ) );

    // both directions at the same time, over the one session
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.server->ResolveMethod( g_getPath, ref, status ) );
    COMCORE::CRpcMessage toClient = f.server->NewRequest( ref );
    toClient.SetUInt64( 1, 21 );
    CORE::CFutureResult serverToClient = f.server->CallAsync( ref, toClient );
    CORE::CFutureResult clientToServer = f.client->CallAsync( g_getPath, GetRequest( f, 12 ) );
    ASSERT_TRUE( f.PumpUntilReady( serverToClient, 3000 ) );
    ASSERT_TRUE( f.PumpUntilReady( clientToServer, 3000 ) );
    COMCORE::CRpcCallResult a;
    COMCORE::CRpcCallResult b;
    ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( serverToClient, a ) );
    ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( clientToServer, b ) );
    ASSERT_OK( a.status );
    ASSERT_TRUE( 21 == a.response.GetUInt64( 1 ) );
    ASSERT_OK( b.status );
    ASSERT_TRUE( 12 == b.response.GetUInt64( 1 ) );
    ASSERT_TRUE( 1 == clientService.calls );
    ASSERT_TRUE( 1 == f.service.calls );
    f.client->Close( false );
    f.server->Close( false );
    RpcTestPump( 10 );
}

/*-------------------------------------------------------------------------*/

static void
TestStreams( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.service.watchCount = 200;     // more than the initial credit of 64
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;

    // server stream past the initial credit, in order
    {
        CStreamCollector collector;
        REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
        COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( PumpUntilFlag( f, collector.ended, 5000 ) );
        ASSERT_OK( collector.status );
        ASSERT_TRUE( 200 == collector.messages );
        bool inOrder = collector.indices.size() == 200;
        for ( UInt32 i=0; inOrder && i<collector.indices.size(); ++i )
            inOrder = collector.indices[ i ] == i + 1 && collector.noteIds[ i ] == i + 1;
        ASSERT_TRUE( inOrder );
        ASSERT_TRUE( f.serverWire->CountSent( GUCEF_RPC_FRAME_STREAM_MSG ) >= 200 );
        ASSERT_TRUE( f.clientWire->CountSent( GUCEF_RPC_FRAME_STREAM_CREDIT ) > 0 );
    }

    // resumable server stream: starts after the given index
    {
        CStreamCollector collector;
        COMCORE::CRpcCallOptions options;
        options.hasResumeFromIndex = true;
        options.resumeFromIndex = 195;
        COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch, options );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( PumpUntilFlag( f, collector.ended, 3000 ) );
        ASSERT_TRUE( 5 == collector.messages );
        ASSERT_TRUE( !collector.noteIds.empty() && 196 == collector.noteIds[ 0 ] );
    }

    // client stream: chunks, half close, one response
    {
        REQUIRE_TRUE( f.client->ResolveMethod( g_importPath, ref, status ) );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
        REQUIRE_TRUE( !stream.IsNULL() );
        for ( UInt32 i=0; i<5; ++i )
        {
            COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
            char data[ 100 ];
            memset( data, 'a' + i, sizeof( data ) );
            chunk.SetBytes( 1, data, sizeof( data ) );
            ASSERT_TRUE( stream->Write( chunk ) );
        }
        ASSERT_TRUE( stream->WritesDone() );
        COMCORE::CRpcMessage message;
        ASSERT_FALSE( stream->ReadNext( message, status, 3000 ) );
        ASSERT_OK( status );
        ASSERT_TRUE( stream->IsDone() );
        ASSERT_TRUE( 5 == stream->GetResponse().GetUInt32( 1 ) );
        ASSERT_TRUE( 5 == f.service.importedChunks );
        ASSERT_TRUE( 500 == f.service.importedBytes );
    }

    // bidi stream: echoes through the blocking reader
    {
        REQUIRE_TRUE( f.client->ResolveMethod( g_editPath, ref, status ) );
        COMCORE::CRpcMessage first = f.client->NewRequest( ref );
        first.SetUInt64( 1, 1 );
        first.SetString( 2, "first" );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &first );
        REQUIRE_TRUE( !stream.IsNULL() );
        for ( UInt32 i=2; i<=3; ++i )
        {
            COMCORE::CRpcMessage op = f.client->NewRequest( ref );
            op.SetUInt64( 1, i );
            op.Set( 3, CORE::CVariant( (Int32) ( i * 10 ) ) );
            ASSERT_TRUE( stream->Write( op ) );
        }
        COMCORE::CRpcMessage echo;
        for ( UInt32 i=1; i<=3; ++i )
        {
            ASSERT_TRUE( stream->ReadNext( echo, status, 3000 ) );
            ASSERT_TRUE( i == echo.GetUInt64( 1 ) );
        }
        ASSERT_TRUE( stream->WritesDone() );
        ASSERT_FALSE( stream->ReadNext( echo, status, 3000 ) );
        ASSERT_OK( status );
    }

    // a held server stream cancelled by the caller
    {
        f.service.watchCount = 2;
        f.service.holdWatch = true;
        CStreamCollector collector;
        REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
        COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( PumpUntilCount( f, collector.messages, 2, 3000 ) );
        UInt32 cancelledBefore = f.service.cancelledCalls;
        stream->Cancel( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_CANCELLED, "enough" ) );
        ASSERT_TRUE( PumpUntilCount( f, f.service.cancelledCalls, cancelledBefore + 1, 3000 ) );
        ASSERT_TRUE( stream->IsDone() );
        ASSERT_CODE( COMCORE::RPC_STATUS_CANCELLED, stream->GetStatus() );
        f.service.holdWatch = false;
    }
}

/*-------------------------------------------------------------------------*/

static void
TestAcceptedAndResult( const char* idKind )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    CEndpointEventRecorder events;
    events.Watch( f.client );

    COMCORE::CRpcCallOptions options;
    if ( GUCEF_NULL != idKind )
        options.metadata.Set( "id-kind", CORE::CVariant( idKind ) );
    COMCORE::CRpcMessage request = f.NewClientRequest( g_reindexPath );
    request.SetBool( 1, true );
    CORE::CFutureResult future = f.client->CallAsync( g_reindexPath, request, options );
    ASSERT_TRUE( PumpUntilCount( f, events.accepted, 1, 3000 ) );
    RpcTestPump( 5 );
    ASSERT_FALSE( future.IsReady() );
    ASSERT_TRUE( 1 == f.service.reindexAccepted );

    if ( GUCEF_NULL != idKind )
    {
        COMCORE::CRpcRequestId id;
        if ( 0 == strcmp( idKind, "string" ) )
        {
            ASSERT_TRUE( "job-1" == events.lastAcceptedId );
            id = COMCORE::CRpcRequestId( CORE::CString( "job-1" ) );
        }
        else
        {
            ASSERT_TRUE( "#1001" == events.lastAcceptedId );
            id = COMCORE::CRpcRequestId( (UInt64) 1001 );
        }

        // STATUS_QUERY while it runs
        COMCORE::CRpcRequestStatus requestStatus;
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( f.client->QueryStatus( id, false, requestStatus, status ) );
        ASSERT_OK( status );
        ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_RUNNING == requestStatus.state );

        // an unknown id
        ASSERT_TRUE( f.client->QueryStatus( COMCORE::CRpcRequestId( CORE::CString( "nope" ) ), false, requestStatus, status ) );
        ASSERT_TRUE( COMCORE::RPC_REQUESTSTATE_UNKNOWN == requestStatus.state );
    }

    ASSERT_TRUE( 1 == f.service.CompleteReindex( 42 ) );
    ASSERT_TRUE( f.PumpUntilReady( future, 3000 ) );
    COMCORE::CRpcCallResult result;
    ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
    ASSERT_OK( result.status );
    ASSERT_TRUE( result.wasAccepted );
    ASSERT_TRUE( 42 == result.response.GetUInt64( 1 ) );
    ASSERT_TRUE( 1500 == result.response.GetDuration( 2 ) );
    if ( GUCEF_NULL != idKind && 0 == strcmp( idKind, "string" ) )
        ASSERT_TRUE( COMCORE::CRpcRequestId::KIND_STRING == result.requestId.GetKind() );
    else if ( GUCEF_NULL != idKind )
        ASSERT_TRUE( COMCORE::CRpcRequestId::KIND_UINT64 == result.requestId.GetKind() && 1001 == result.requestId.GetUInt64() );
    else
        ASSERT_FALSE( result.requestId.IsSet() );
    ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_ACCEPTED ) );
    ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_RESULT ) );

    // the same method answering at once: a direct RESPONSE
    request.SetBool( 1, false );
    result = f.CallAndPump( g_reindexPath, request );
    ASSERT_OK( result.status );
    ASSERT_TRUE( !result.wasAccepted && 7 == result.response.GetUInt64( 1 ) );
}

/*-------------------------------------------------------------------------*/

static void
TestCancelRequestById( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    CEndpointEventRecorder events;
    events.Watch( f.client );
    COMCORE::CRpcCallOptions options;
    options.metadata.Set( "id-kind", CORE::CVariant( "number" ) );
    COMCORE::CRpcMessage request = f.NewClientRequest( g_reindexPath );
    request.SetBool( 1, true );
    CORE::CFutureResult future = f.client->CallAsync( g_reindexPath, request, options );
    ASSERT_TRUE( PumpUntilCount( f, events.accepted, 1, 3000 ) );
    COMCORE::CRpcStatus status;
    ASSERT_TRUE( f.client->CancelRequest( COMCORE::CRpcRequestId( (UInt64) 1001 ), status ) );
    ASSERT_OK( status );
    ASSERT_TRUE( f.PumpUntilReady( future, 3000 ) );
    COMCORE::CRpcCallResult result;
    ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_CANCELLED, result.status );
    ASSERT_TRUE( 1 == f.clientWire->CountSent( GUCEF_RPC_FRAME_CANCEL ) );
}

/*-------------------------------------------------------------------------*/

static void
TestReflection( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    f.clientSettings.contractRegistry = &f.clientRegistry;
    f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    REQUIRE_TRUE( f.server->RegisterService( notes, "Notes", &f.service ) );
    f.client->AddWantedContract( "gucef.rpc", 1 );
    REQUIRE_TRUE( f.server->Start( f.serverTransport, false ) );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.rpc", 1, 2000 ) );

    // a generic tool: lists what the provider offers and fetches the schema it does not have
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( "gucef.rpc.Reflection/ListContracts", ref, status ) );
    COMCORE::CRpcMessage list = f.client->NewRequest( ref );
    COMCORE::CRpcMessage response;
    REQUIRE_TRUE( f.client->Call( ref, list, response, status ) );
    ASSERT_OK( status );
    bool foundNotes = false;
    bool foundRpc = false;
    CORE::CDynamicBuffer notesHash;
    for ( UInt32 i=0; i<response.GetCount( 1 ); ++i )
    {
        const COMCORE::CRpcMessage* info = response.GetMessageAt( 1, i );
        if ( "gucef.sample.notes" == info->GetString( 1 ) )
        {
            foundNotes = 1 == info->GetUInt32( 2 ) && 2 == info->GetUInt32( 3 ) && 0 < info->GetCount( 5 );
            info->GetBytes( 4, notesHash );
        }
        if ( "gucef.rpc" == info->GetString( 1 ) )
            foundRpc = true;
    }
    ASSERT_TRUE( foundNotes );
    ASSERT_TRUE( foundRpc );
    ASSERT_TRUE( 32 == notesHash.GetDataSize() && 0 == memcmp( notesHash.GetConstBufferPtr(), notes->GetHash().GetBytes(), 32 ) );

    REQUIRE_TRUE( f.client->ResolveMethod( "gucef.rpc.Reflection/GetContract", ref, status ) );
    COMCORE::CRpcMessage get = f.client->NewRequest( ref );
    get.SetString( 2, "gucef.sample.notes" );
    get.SetUInt32( 3, 1 );
    get.SetUInt32( 4, 2 );
    get.SetBool( 5, true );
    REQUIRE_TRUE( f.client->Call( ref, get, response, status ) );
    ASSERT_OK( status );
    CORE::CDynamicBuffer schema;
    CORE::CDynamicBuffer docs;
    ASSERT_TRUE( response.GetBytes( 1, schema ) );
    response.GetBytes( 2, docs );
    CORE::CString error;
    COMCORE::CRpcContractPtr fetched = COMCORE::CRpcContract::FromSchema( schema.GetConstBufferPtr(), schema.GetDataSize(), error,
                                                                          docs.GetConstBufferPtr(), docs.GetDataSize() );
    ASSERT_TRUE( !fetched.IsNULL() && fetched->GetHash() == notes->GetHash() );

    // by hash; an unknown hash is NOT_FOUND
    get = f.client->NewRequest( ref );
    get.SetBytes( 1, notesHash );
    REQUIRE_TRUE( f.client->Call( ref, get, response, status ) );
    ASSERT_OK( status );
    UInt8 zero[ 32 ];
    memset( zero, 0, sizeof( zero ) );
    get.SetBytes( 1, zero, 32 );
    REQUIRE_TRUE( f.client->Call( ref, get, response, status ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_NOT_FOUND, status );

    // reflection switched off
    CRpcEndpointFixture g;
    g.serverSettings.enableReflection = false;
    REQUIRE_TRUE( g.MakeLoopbackTransports() );
    g.server = COMCORE::CRpcEndpoint::Create( g.pulseGenerator, g.serverSettings );
    g.client = COMCORE::CRpcEndpoint::Create( g.pulseGenerator, g.clientSettings );
    REQUIRE_TRUE( g.server->RegisterService( notes, "Notes", &g.service ) );
    g.client->AddWantedContract( "gucef.rpc", 1 );
    REQUIRE_TRUE( g.server->Start( g.serverTransport, false ) );
    REQUIRE_TRUE( g.client->Start( g.clientTransport, true ) );
    REQUIRE_TRUE( g.PumpUntilState( g.client, COMCORE::RPC_SESSIONSTATE_ESTABLISHED, 2000 ) );
    RpcTestPump( 10 );
    REQUIRE_TRUE( g.client->ResolveMethod( "gucef.rpc.Reflection/ListContracts", ref, status ) );
    list = g.client->NewRequest( ref );
    ASSERT_TRUE( g.client->Call( ref, list, response, status ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_UNIMPLEMENTED, status );
}

/*-------------------------------------------------------------------------*/

static void
TestAccessPolicy( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    f.serverWire->hasPrincipalOverride = true;
    f.serverWire->principalOverride = COMCORE::CRpcPrincipal( "loopback", "reader-1", true );
    f.serverWire->principalOverride.roles.insert( "reader" );
    COMCORE::CRpcRoleAccessPolicy* policy = GUCEF_NEW COMCORE::CRpcRoleAccessPolicy();
    CORE::CString::StringSet readers;
    readers.insert( "reader" );
    CORE::CString::StringSet writers;
    writers.insert( "writer" );
    policy->AddRule( "gucef.sample.notes.Notes/Get", readers );
    policy->AddRule( "gucef.sample.notes.Notes/Save", writers );
    CORE::CTSharedPtr< COMCORE::CRpcRoleAccessPolicy, MT::CMutex > shared( policy );
    COMCORE::CIRpcAccessPolicyPtr policyPtr;
    policyPtr = shared;

    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    REQUIRE_TRUE( f.server->RegisterService( notes, "Notes", &f.service ) );
    f.server->SetAccessPolicy( policyPtr );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( f.server->Start( f.serverTransport, false ) );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );

    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
    ASSERT_OK( result.status );
    COMCORE::CRpcMessage save = f.NewClientRequest( g_savePath );
    result = f.CallAndPump( g_savePath, save );
    ASSERT_CODE( COMCORE::RPC_STATUS_PERMISSION_DENIED, result.status );
    ASSERT_TRUE( 0 == f.service.saves );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == f.client->GetState() );

    // without any role the contract cannot be bound at all
    CRpcEndpointFixture g;
    REQUIRE_TRUE( g.MakeLoopbackTransports() );
    g.serverWire->hasPrincipalOverride = true;
    g.serverWire->principalOverride = COMCORE::CRpcPrincipal( "loopback", "nobody", true );
    g.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    g.server = COMCORE::CRpcEndpoint::Create( g.pulseGenerator, g.serverSettings );
    g.client = COMCORE::CRpcEndpoint::Create( g.pulseGenerator, g.clientSettings );
    REQUIRE_TRUE( g.server->RegisterService( notes, "Notes", &g.service ) );
    g.server->SetAccessPolicy( policyPtr );
    g.client->AddWantedContract( "gucef.sample.notes", 1 );
    REQUIRE_TRUE( g.server->Start( g.serverTransport, false ) );
    REQUIRE_TRUE( g.client->Start( g.clientTransport, true ) );
    RpcTestPump( 50 );
    ASSERT_FALSE( g.client->IsContractBound( "gucef.sample.notes", 1 ) );
    ASSERT_TRUE( 0 == g.service.calls );
}

/*-------------------------------------------------------------------------*/

static void
TestExecutionModes( void )
{
    TThreadId self = MT::GetCurrentThreadID();
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    {
        // the protocol thread (here the test thread, which drives the global pulse generator)
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes, COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
        ASSERT_OK( f.CallAndPump( g_getPath, GetRequest( f, 1 ) ).status );
        ASSERT_TRUE( self == f.service.lastHandlerThread );
    }
    {
        // a thread pool task
        CRpcEndpointFixture f;
        REQUIRE_TRUE( f.Connect( notes, notes, COMCORE::RPC_EXECUTION_THREAD_POOL ) );
        ASSERT_OK( f.CallAndPump( g_getPath, GetRequest( f, 1 ) ).status );
        ASSERT_TRUE( 0 != f.service.lastHandlerThread && self != f.service.lastHandlerThread );
        f.service.watchCount = 100;
        CStreamCollector collector;
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
        COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( PumpUntilFlag( f, collector.ended, 5000 ) );
        ASSERT_TRUE( 100 == collector.messages );
        ASSERT_OK( collector.status );
    }
    {
        // protocol on its own dedicated task (loopback without a pulse generator), handlers mailed to the
        // notification thread: the test thread
        CRpcEndpointFixture f;
        COMCORE::CIRpcTransportPtr a;
        COMCORE::CIRpcTransportPtr b;
        CORE::PulseGeneratorPtr none;
        REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( none, none, a, b ) );
        f.clientTransport = CRpcLossyTransport::Wrap( a, &f.clientWire );
        f.serverTransport = CRpcLossyTransport::Wrap( b, &f.serverWire );
        REQUIRE_TRUE( f.Connect( notes, notes, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) );
        CORE::PulseGeneratorPtr protocol = f.server->GetProtocolPulseGenerator();
        ASSERT_TRUE( !protocol.IsNULL() && protocol != f.pulseGenerator );
        ASSERT_TRUE( !protocol.IsNULL() && self != protocol->GetPulseDriverThreadId() );
        ASSERT_OK( f.CallAndPump( g_getPath, GetRequest( f, 1 ) ).status );
        ASSERT_TRUE( self == f.service.lastHandlerThread );
    }
    {
        // the blocking form from a thread that is not the protocol thread waits on the future; the handler runs on
        // the protocol thread (a handler mailed to the blocked thread itself would wait for its own caller)
        CRpcEndpointFixture f;
        COMCORE::CIRpcTransportPtr a;
        COMCORE::CIRpcTransportPtr b;
        CORE::PulseGeneratorPtr none;
        REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( none, none, a, b ) );
        f.clientTransport = CRpcLossyTransport::Wrap( a, &f.clientWire );
        f.serverTransport = CRpcLossyTransport::Wrap( b, &f.serverWire );
        REQUIRE_TRUE( f.Connect( notes, notes, COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
        COMCORE::CRpcMessage response;
        COMCORE::CRpcStatus status;
        ASSERT_TRUE( f.client->Call( g_getPath, GetRequest( f, 2 ), response, status ) );
        ASSERT_OK( status );
        ASSERT_TRUE( 2 == response.GetUInt64( 1 ) );
        CORE::PulseGeneratorPtr protocol = f.server->GetProtocolPulseGenerator();
        ASSERT_TRUE( !protocol.IsNULL() && protocol->GetPulseDriverThreadId() == f.service.lastHandlerThread );
    }
}

/*-------------------------------------------------------------------------*/

static void
TestProtocolTaskNamesItsThread( void )
{
    // the blocking form and the nested call check compare the pulse generator's driver thread with
    // MT::GetCurrentThreadID(): a dedicated protocol task must report the id of the thread that pulses
    COMCORE::CRpcProtocolTaskPtr task = COMCORE::CRpcProtocolTask::Start( "gucef.rpc" );
    REQUIRE_TRUE( !task.IsNULL() );
    CORE::PulseGeneratorPtr pulseGenerator = task->GetPulseGenerator();
    REQUIRE_TRUE( !pulseGenerator.IsNULL() );
    CPulseThreadRecorder recorder;
    recorder.Watch( pulseGenerator );
    pulseGenerator->RequestPeriodicPulses( &recorder, 1 );
    UInt64 start = MT::PrecisionTickCount();
    while ( 0 == recorder.GetThreadId() && ( MT::PrecisionTickCount() - start ) < 3 * MT::PrecisionTimerResolution() )
        MT::PrecisionDelay( 1 );
    pulseGenerator->RequestStopOfPeriodicUpdates( &recorder );
    TThreadId pulsing = recorder.GetThreadId();
    TThreadId reported = pulseGenerator->GetPulseDriverThreadId();
    if ( pulsing != reported )
        GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: the protocol task pulses on " << pulsing << " but reports driver " << reported );
    ASSERT_TRUE( 0 != pulsing );
    ASSERT_TRUE( pulsing == reported );
    recorder.UnsubscribeAllFromObserver();
    task->Stop();
}

/*-------------------------------------------------------------------------*/

static void
TestNestedBlockingCallRefused( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    CNestedCallService nested;
    f.clientRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.serverRegistry.Register( notes, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
    f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
    f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
    REQUIRE_TRUE( f.server->RegisterService( notes, "Notes", &nested ) );
    REQUIRE_TRUE( f.client->RegisterService( notes, "Notes", &f.service ) );
    f.client->AddWantedContract( "gucef.sample.notes", 1 );
    f.server->AddWantedContract( "gucef.sample.notes", 1 );
    nested.endpoint = f.server;
    REQUIRE_TRUE( f.server->Start( f.serverTransport, false ) );
    REQUIRE_TRUE( f.client->Start( f.clientTransport, true ) );
    REQUIRE_TRUE( f.PumpUntilBound( "gucef.sample.notes", 1, 2000 ) );
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 1 ) );
    ASSERT_OK( result.status );
    ASSERT_TRUE( "outer" == result.response.GetString( 2 ) );
    ASSERT_FALSE( nested.nestedReturned );
    ASSERT_CODE( COMCORE::RPC_STATUS_FAILED_PRECONDITION, nested.nestedStatus );
    nested.endpoint.Unlink();
}

/*-------------------------------------------------------------------------*/

static void
TestBlockingCallWithoutKnownThread( void )
{
    // a pulse generator without a driver and without a known driving thread: the blocking form refuses
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    CORE::PulseGeneratorPtr own = CORE::CPulseGenerator::CreateSharedObj();
    f.pulseGenerator = own;
    own->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( own, own, a, b ) );
    f.clientTransport = CRpcLossyTransport::Wrap( a, &f.clientWire );
    f.serverTransport = CRpcLossyTransport::Wrap( b, &f.serverWire );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    COMCORE::CRpcMessage response;
    COMCORE::CRpcStatus status;
    ASSERT_TRUE( f.client->Call( g_getPath, GetRequest( f, 1 ), response, status ) );
    ASSERT_OK( status );
    own->SetPulseDriverThreadId( 0 );
    ASSERT_FALSE( f.client->Call( g_getPath, GetRequest( f, 1 ), response, status ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_FAILED_PRECONDITION, status );
    own->SetPulseDriverThreadId( MT::GetCurrentThreadID() );
}

/*-------------------------------------------------------------------------*/

static void
TestRequireSecureTransport( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    f.serverSettings.requireSecureTransport = true;
    REQUIRE_TRUE( f.MakeLoopbackTransports() );
    f.serverWire->forceInsecure = true;
    ASSERT_FALSE( f.Connect( notes, notes ) );
    RpcTestPump( 20 );
    ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED != f.client->GetState() );
    ASSERT_TRUE( 0 == f.service.calls );
}

/*-------------------------------------------------------------------------*/

static void
TestGracefulClose( void )
{
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    CEndpointEventRecorder clientEvents;
    CEndpointEventRecorder serverEvents;
    clientEvents.Watch( f.client );
    serverEvents.Watch( f.server );

    // an open call is answered before the session ends
    CORE::CFutureResult future = f.client->CallAsync( g_getPath, GetRequest( f, 9 ) );
    f.client->Close( true );
    ASSERT_TRUE( f.PumpUntilReady( future, 3000 ) );
    COMCORE::CRpcCallResult result;
    ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
    ASSERT_OK( result.status );
    ASSERT_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 3000 ) );
    ASSERT_TRUE( f.PumpUntilState( f.client, COMCORE::RPC_SESSIONSTATE_CLOSED, 3000 ) );
    RpcTestPump( 10 );
    ASSERT_TRUE( 1 == f.clientWire->CountSent( GUCEF_RPC_FRAME_GOODBYE ) );
    ASSERT_TRUE( 1 == clientEvents.closed );
    ASSERT_TRUE( 1 == serverEvents.closed );

    // after the end calls are refused at once
    result = f.CallAndPump( g_getPath, GetRequest( f, 9 ) );
    ASSERT_FALSE( result.status.IsOk() );
}

/*-------------------------------------------------------------------------*/

static int
RunRpcThreadPoolHandlerCall( void )
{
    // nothing in this process used gucefCOMCORE before: the endpoint has to have what its thread pool mode needs
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    if ( notes.IsNULL() )
        return 2;
    TThreadId self = MT::GetCurrentThreadID();
    CRpcEndpointFixture f;
    if ( !f.Connect( notes, notes, COMCORE::RPC_EXECUTION_THREAD_POOL ) )
        return 3;
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 1 ), COMCORE::CRpcCallOptions(), 10000 );
    if ( !result.status.IsOk() )
        return 4;
    if ( 0 == f.service.lastHandlerThread || self == f.service.lastHandlerThread )
        return 5;
    return 0;
}

/*-------------------------------------------------------------------------*/

int
RunRpcThreadPoolHandlerInFreshProcess( const char* resultFile )
{GUCEF_TRACE;

    // the outcome goes into the file: the exit code of a process is not only ours (a sanitizer sets it too)
    int outcome = RunRpcThreadPoolHandlerCall();
    CORE::CString text = CORE::ToString( (Int32) outcome );
    CORE::CFileAccess file( resultFile, "wb" );
    if ( file.IsValid() )
        file.Write( text, false );
    file.Close();
    return outcome;
}

/*-------------------------------------------------------------------------*/

static void
TestThreadPoolHandlerInFreshProcess( void )
{
    #ifdef GUCEF_LINUX_BUILD

    // the call task type comes with gucefCOMCORE's global state: a process whose first use of gucefCOMCORE is an
    // endpoint needs it as much as one that used other parts first
    char exePath[ 4096 ];
    ssize_t exePathLength = readlink( "/proc/self/exe", exePath, sizeof( exePath ) - 1 );
    REQUIRE_TRUE( exePathLength > 0 );
    exePath[ exePathLength ] = '\0';
    CORE::CString resultFile = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), "gucef_rpc_fresh_process_" + CORE::ToString( MT::GetCurrentThreadID() ) + ".txt" );
    CORE::DeleteFile( resultFile );
    CORE::CProcess::TStringVector args;
    args.push_back( GUCEF_COMCORE_TESTAPP_RPC_FRESH_PROCESS_ARG );
    args.push_back( resultFile );
    CORE::CProcess child;
    child.SetIsolateFromParent( true );
    REQUIRE_TRUE( child.Launch( exePath, args ) );
    Int32 exitCode = -1;
    const bool ended = child.WaitForExit( 120000, exitCode );
    if ( !ended )
        child.Terminate( false );
    ASSERT_TRUE( ended );
    CORE::CString outcome;
    ASSERT_TRUE( CORE::LoadTextFileAsString( resultFile, outcome ) );
    if ( "0" != outcome )
        GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "RpcEndpoint test: the call in the fresh process ended with outcome " << outcome << " (2 no contract, 3 no binding, 4 the call failed, 5 not on a pool thread)" );
    ASSERT_TRUE( "0" == outcome );
    CORE::DeleteFile( resultFile );

    #endif /* GUCEF_LINUX_BUILD ? */
}

/*-------------------------------------------------------------------------*/

static void
TestEventsToAnotherThreadWhileItCallsIn( void )
{
    // both protocols run on dedicated tasks, their events go to this thread. The handlers here call into the endpoints
    // (a consumed stream message raises the credit, the provider writes on WritableEvent) while the protocol threads
    // raise the next events for this thread: neither side may wait for the other
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    CRpcEndpointFixture f;
    COMCORE::CIRpcTransportPtr a;
    COMCORE::CIRpcTransportPtr b;
    CORE::PulseGeneratorPtr none;
    REQUIRE_TRUE( COMCORE::CRpcLoopbackTransport::CreatePair( none, none, a, b ) );
    f.clientTransport = CRpcLossyTransport::Wrap( a, &f.clientWire );
    f.serverTransport = CRpcLossyTransport::Wrap( b, &f.serverWire );
    const UInt32 messages = 1500;
    f.service.watchCount = messages;
    REQUIRE_TRUE( f.Connect( notes, notes, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) );
    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;
    REQUIRE_TRUE( f.client->ResolveMethod( g_watchPath, ref, status ) );
    for ( UInt32 round=0; round<4; ++round )
    {
        CStreamCollector collector;
        COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );

        // a lock wait between the threads ends only in the lock timeout (a minute): far beyond this
        UInt64 start = MT::PrecisionTickCount();
        ASSERT_TRUE( PumpUntilFlag( f, collector.ended, 20000 ) );
        UInt64 elapsedMs = ( MT::PrecisionTickCount() - start ) * 1000 / MT::PrecisionTimerResolution();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RpcEndpoint test: stream " << round << " to another thread: " << collector.messages << " messages in " << elapsedMs << " ms" );
        ASSERT_OK( collector.status );
        ASSERT_TRUE( messages == collector.messages );
        if ( !collector.ended )
            return;
    }
}

/*-------------------------------------------------------------------------*/

static void
TestProtectionDowngradeRefused( void )
{
    // a transport that honours protection classes per frame delivers a request of a CONFIDENTIAL method as PUBLIC:
    // the session ends with ABORT PERMISSION_DENIED and the transport hears of it (14.3)
    CRpcEndpointFixture f;
    COMCORE::CRpcContractPtr notes = LoadSampleNotesContract( 1, 2 );
    REQUIRE_TRUE( f.Connect( notes, notes ) );
    CEndpointEventRecorder clientEvents;
    clientEvents.Watch( f.client );
    f.serverWire->tamperProtection = true;
    UInt32 callsBefore = f.service.calls;
    COMCORE::CRpcCallResult result = f.CallAndPump( g_getPath, GetRequest( f, 5 ) );
    ASSERT_FALSE( result.status.IsOk() );
    ASSERT_TRUE( f.PumpUntilState( f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 3000 ) );
    ASSERT_TRUE( callsBefore == f.service.calls );
    ASSERT_TRUE( 1 == f.serverWire->CountSent( GUCEF_RPC_FRAME_ABORT ) );
    ASSERT_TRUE( 1 == f.serverWire->downgradeRefusals );
    ASSERT_TRUE( 0 == f.clientWire->downgradeRefusals );
    ASSERT_TRUE( 1 == TelemetryValueOf( f.server, "protectionDowngrades" ) );
    ASSERT_TRUE( PumpUntilCount( f, clientEvents.closed, 1, 3000 ) );
    ASSERT_CODE( COMCORE::RPC_STATUS_PERMISSION_DENIED, clientEvents.lastClose );
}

/*-------------------------------------------------------------------------*/

static void
TestConformanceOverLoopback( bool ownThreads )
{
    CLoopbackConformanceTransports transports( ownThreads );
    PerformRpcConformanceTests( transports );
}

/*-------------------------------------------------------------------------*/

#define RUN_RPC_CASE( name, call )                                  \
    GUCEF_TESTFW_TESTCASE( "rpc_endpoint: " name )                  \
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
PerformRpcEndpointTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "rpc_endpoint" );

    RUN_RPC_CASE( "unary calls, errors, metadata interning, blocking form, telemetry", TestUnaryCalls() );
    RUN_RPC_CASE( "sensitive fields are not logged", TestSensitiveNotLogged() );
    RUN_RPC_CASE( "peer-to-peer: both sides call each other over one session", TestPeerToPeer() );
    RUN_RPC_CASE( "server, client and bidi streams, credits, resume index, cancel", TestStreams() );
    RUN_RPC_CASE( "ACCEPTED then RESULT, string request id, STATUS_QUERY", TestAcceptedAndResult( "string" ) );
    RUN_RPC_CASE( "ACCEPTED then RESULT, UInt64 request id", TestAcceptedAndResult( "number" ) );
    RUN_RPC_CASE( "ACCEPTED then RESULT without a request id", TestAcceptedAndResult( GUCEF_NULL ) );
    RUN_RPC_CASE( "CancelRequest by request id", TestCancelRequestById() );
    RUN_RPC_CASE( "reflection lists and serves contracts", TestReflection() );
    RUN_RPC_CASE( "access policy per method and per contract", TestAccessPolicy() );
    RUN_RPC_CASE( "handler execution modes and the blocking form across threads", TestExecutionModes() );
    RUN_RPC_CASE( "a dedicated protocol task reports the thread that drives its pulse generator", TestProtocolTaskNamesItsThread() );
    RUN_RPC_CASE( "a nested blocking call inside a handler is refused", TestNestedBlockingCallRefused() );
    RUN_RPC_CASE( "the blocking form without a known protocol thread is refused", TestBlockingCallWithoutKnownThread() );
    RUN_RPC_CASE( "requireSecureTransport refuses an insecure transport", TestRequireSecureTransport() );
    RUN_RPC_CASE( "graceful close with GOODBYE", TestGracefulClose() );
    RUN_RPC_CASE( "a thread pool handler in a process that used nothing of gucefCOMCORE before", TestThreadPoolHandlerInFreshProcess() );
    RUN_RPC_CASE( "events for another thread while its handlers call into the endpoints: no lock waits", TestEventsToAnotherThreadWhileItCallsIn() );
    RUN_RPC_CASE( "a protection class downgrade ends the session and is reported to the transport", TestProtectionDowngradeRefused() );
    TestConformanceOverLoopback( false );
    TestConformanceOverLoopback( true );

    // the tests that inspect frames through the capturing decorator, over loopback on the test thread (gucefCOM_TestApp
    // runs them over tcp)
    CLoopbackConformanceTransports loopback( false );
    PerformRpcTransportSuiteTests( loopback );
}

/*-------------------------------------------------------------------------*/
