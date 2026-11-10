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

#ifndef GUCEF_COMCORE_TESTAPP_TESTRPCCONFORMANCE_H
#define GUCEF_COMCORE_TESTAPP_TESTRPCCONFORMANCE_H

/*
 *  The RPC conformance suite (gucefCOMCORE RPC design 16): the same tests over every transport. Header only, so that
 *  the test app of a module with a transport of its own (tcp in gucefCOM, drn in gucefDRN) runs it with its
 *  transports: implement CIRpcConformanceTransports and call PerformRpcConformanceTests().
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#include "gucefCOMCORE_CRpcContractRegistry.h"
#define GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTREGISTRY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRpcSupport.h"

#include "TestRpcNotesService.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Records the endpoint events it is subscribed to */
class CRpcEndpointEventRecorder : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 established;
    GUCEF::UInt32 suspended;
    GUCEF::UInt32 resumed;
    GUCEF::UInt32 gaps;
    GUCEF::UInt32 closed;
    GUCEF::UInt32 accepted;
    GUCEF::UInt32 bound;
    GUCEF::UInt32 boundFailed;
    GUCEF::CORE::CString lastAcceptedId;
    GUCEF::COMCORE::CRpcStatus lastBindStatus;
    GUCEF::COMCORE::CRpcStatus lastClose;
    GUCEF::COMCORE::CRpcSessionGapEventData lastGap;
    GUCEF::UInt32 eventThread;

    CRpcEndpointEventRecorder( void )
        : GUCEF::CORE::CObserver(), established( 0 ), suspended( 0 ), resumed( 0 ), gaps( 0 ), closed( 0 ), accepted( 0 ), bound( 0 )
        , boundFailed( 0 ), lastAcceptedId(), lastBindStatus(), lastClose(), lastGap(), eventThread( 0 )
    {}

    virtual ~CRpcEndpointEventRecorder() { UnsubscribeAllFromObserver(); }

    void Watch( const GUCEF::COMCORE::CRpcEndpointPtr& endpoint )
    {
        typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcEndpointEventRecorder > TEventCallback;
        GUCEF::COMCORE::CRpcEndpointPtr e( endpoint );
        TEventCallback cb( this, &CRpcEndpointEventRecorder::OnEvent );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::SessionEstablishedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::SessionSuspendedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::SessionResumedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::SessionGapEvent, cb );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::SessionClosedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::AcceptedEvent, cb );
        SubscribeTo( e.GetPointerAlways(), GUCEF::COMCORE::CRpcEndpoint::ContractBoundEvent, cb );
    }

    private:

    void OnEvent( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent& eventId, GUCEF::CORE::CICloneable* eventData )
    {
        eventThread = GUCEF::MT::GetCurrentTaskID();
        if ( GUCEF::COMCORE::CRpcEndpoint::SessionEstablishedEvent == eventId ) ++established;
        else if ( GUCEF::COMCORE::CRpcEndpoint::SessionSuspendedEvent == eventId ) ++suspended;
        else if ( GUCEF::COMCORE::CRpcEndpoint::SessionResumedEvent == eventId ) ++resumed;
        else if ( GUCEF::COMCORE::CRpcEndpoint::SessionClosedEvent == eventId )
        {
            ++closed;
            if ( GUCEF_NULL != eventData )
                lastClose = *static_cast< GUCEF::COMCORE::CRpcStatus* >( eventData );
        }
        else if ( GUCEF::COMCORE::CRpcEndpoint::SessionGapEvent == eventId )
        {
            ++gaps;
            if ( GUCEF_NULL != eventData )
                lastGap = *static_cast< GUCEF::COMCORE::CRpcSessionGapEventData* >( eventData );
        }
        else if ( GUCEF::COMCORE::CRpcEndpoint::AcceptedEvent == eventId )
        {
            ++accepted;
            if ( GUCEF_NULL != eventData )
                lastAcceptedId = static_cast< GUCEF::COMCORE::CRpcAcceptedEventData* >( eventData )->requestId.ToString();
        }
        else if ( GUCEF::COMCORE::CRpcEndpoint::ContractBoundEvent == eventId && GUCEF_NULL != eventData )
        {
            GUCEF::COMCORE::CRpcBindingEventData* data = static_cast< GUCEF::COMCORE::CRpcBindingEventData* >( eventData );
            lastBindStatus = data->status;
            if ( data->status.IsOk() )
                ++bound;
            else
                ++boundFailed;
        }
    }
};

/*-------------------------------------------------------------------------*/

/** Collects the messages and the end of a client stream */
class CRpcStreamCollector : public GUCEF::CORE::CObserver
{
    public:

    GUCEF::UInt32 messages;
    GUCEF::vector< GUCEF::UInt64 > indices;
    GUCEF::vector< GUCEF::UInt64 > noteIds;
    bool ended;
    GUCEF::COMCORE::CRpcStatus status;
    GUCEF::UInt32 eventThread;

    CRpcStreamCollector( void ) : GUCEF::CORE::CObserver(), messages( 0 ), indices(), noteIds(), ended( false ), status(), eventThread( 0 ) {}
    virtual ~CRpcStreamCollector() { UnsubscribeAllFromObserver(); }

    void Watch( const GUCEF::COMCORE::CIRpcStreamPtr& stream )
    {
        typedef GUCEF::CORE::CTEventHandlerFunctor< CRpcStreamCollector > TEventCallback;
        GUCEF::COMCORE::CIRpcStreamPtr s( stream );
        TEventCallback message( this, &CRpcStreamCollector::OnMessage );
        TEventCallback end( this, &CRpcStreamCollector::OnStatus );
        SubscribeTo( s.GetPointerAlways(), GUCEF::COMCORE::CIRpcStream::MessageEvent, message );
        SubscribeTo( s.GetPointerAlways(), GUCEF::COMCORE::CIRpcStream::StatusEvent, end );
    }

    private:

    void OnMessage( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        ++messages;
        eventThread = GUCEF::MT::GetCurrentTaskID();
        GUCEF::COMCORE::CRpcStreamMessageEventData* data = static_cast< GUCEF::COMCORE::CRpcStreamMessageEventData* >( eventData );
        indices.push_back( data->streamIndex );
        const GUCEF::COMCORE::CRpcMessage* note = data->message.GetMessage( 1 );
        noteIds.push_back( GUCEF_NULL != note ? note->GetUInt64( 1 ) : data->message.GetUInt64( 1 ) );
    }

    void OnStatus( GUCEF::CORE::CNotifier*, const GUCEF::CORE::CEvent&, GUCEF::CORE::CICloneable* eventData )
    {
        ended = true;
        if ( GUCEF_NULL != eventData )
            status = *static_cast< GUCEF::COMCORE::CRpcStatus* >( eventData );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  What the conformance suite needs of a transport: connected pairs. The endpoints' events go to the global pulse
 *  generator, which the test thread pumps; the transports run where the implementation puts them (their own threads
 *  test the cases in which the protocol thread is not the notification thread).
 */
class CIRpcConformanceTransports
{
    public:

    virtual ~CIRpcConformanceTransports() {}

    /** For the test names: "loopback", "tcp", "drn", ... */
    virtual const char* GetName( void ) const = 0;

    /** A connected pair; the initiator starts the session, the acceptor waits for its HELLO. False when none can be made */
    virtual bool MakePair( GUCEF::COMCORE::CIRpcTransportPtr& initiator, GUCEF::COMCORE::CIRpcTransportPtr& acceptor ) = 0;

    /** Called in every round of the test thread's pumping: for transports that the test thread has to drive */
    virtual void Pump( void ) {}
};

/*-------------------------------------------------------------------------*/

/** Two endpoints over a pair of the transports under test, the notes service on the acceptor */
class CRpcConformanceFixture
{
    public:

    CIRpcConformanceTransports& transports;
    GUCEF::COMCORE::CRpcContractRegistry clientRegistry;
    GUCEF::COMCORE::CRpcContractRegistry serverRegistry;
    GUCEF::COMCORE::CRpcEndpointSettings clientSettings;
    GUCEF::COMCORE::CRpcEndpointSettings serverSettings;
    GUCEF::COMCORE::CRpcEndpointPtr client;
    GUCEF::COMCORE::CRpcEndpointPtr server;
    GUCEF::COMCORE::CIRpcTransportPtr clientTransport;
    GUCEF::COMCORE::CIRpcTransportPtr serverTransport;
    CRpcNotesTestService service;
    GUCEF::COMCORE::CRpcContractPtr notes;
    GUCEF::CORE::PulseGeneratorPtr pulseGenerator;
    GUCEF::UInt32 previousDriver;

    explicit CRpcConformanceFixture( CIRpcConformanceTransports& t )
        : transports( t ), clientRegistry(), serverRegistry(), clientSettings(), serverSettings(), client(), server()
        , clientTransport(), serverTransport(), service(), notes( LoadSampleNotesContract( 1, 2 ) ), pulseGenerator(), previousDriver( 0 )
    {
        pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        previousDriver = pulseGenerator->GetPulseDriverThreadId();
        pulseGenerator->SetPulseDriverThreadId( GUCEF::MT::GetCurrentTaskID() );
        clientSettings.nodeName = "conformance-client";
        serverSettings.nodeName = "conformance-server";
        clientSettings.contractRegistry = &clientRegistry;
        serverSettings.contractRegistry = &serverRegistry;
        clientRegistry.SetCacheDirectory( GUCEF::CORE::CString::Empty );
        serverRegistry.SetCacheDirectory( GUCEF::CORE::CString::Empty );
    }

    ~CRpcConformanceFixture()
    {
        if ( !client.IsNULL() )
            client->Close( false );
        if ( !server.IsNULL() )
            server->Close( false );
        PumpFor( 50 );
        client.Unlink();
        server.Unlink();
        clientTransport.Unlink();
        serverTransport.Unlink();
        PumpFor( 10 );
        pulseGenerator->SetPulseDriverThreadId( previousDriver );
    }

    void PumpOnce( void )
    {
        pulseGenerator->ForceDirectPulse();
        transports.Pump();
    }

    void PumpFor( GUCEF::UInt32 milliseconds )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        do
        {
            PumpOnce();
            GUCEF::MT::PrecisionDelay( 1 );
        }
        while ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) < milliseconds );
    }

    bool PumpUntilFlag( const bool& flag, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( !flag )
        {
            PumpOnce();
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    bool PumpUntilCount( const GUCEF::UInt32& counter, GUCEF::UInt32 target, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( counter < target )
        {
            PumpOnce();
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    /** The handlers may run on another thread: the service's counter is read under its lock */
    bool PumpUntilCancelledCalls( GUCEF::UInt32 target, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( service.GetCancelledCalls() < target )
        {
            PumpOnce();
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    bool PumpUntilReady( const GUCEF::CORE::CFutureResult& future, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( !future.IsReady() )
        {
            PumpOnce();
            if ( future.IsReady() )
                break;
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    bool PumpUntilState( const GUCEF::COMCORE::CRpcEndpointPtr& endpoint, GUCEF::COMCORE::TRpcSessionState state, GUCEF::UInt32 timeoutMs )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::UInt64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000;
        while ( endpoint->GetState() != state )
        {
            PumpOnce();
            if ( ( GUCEF::MT::PrecisionTickCount() - start ) / ( ticksPerMs > 0 ? ticksPerMs : 1 ) > timeoutMs )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        return true;
    }

    /** The pair, both endpoints, the binding of the notes contract */
    bool Connect( GUCEF::COMCORE::TRpcExecutionMode mode )
    {
        if ( notes.IsNULL() || !transports.MakePair( clientTransport, serverTransport ) )
            return false;
        clientRegistry.Register( notes, GUCEF::COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        server = GUCEF::COMCORE::CRpcEndpoint::Create( pulseGenerator, serverSettings );
        client = GUCEF::COMCORE::CRpcEndpoint::Create( pulseGenerator, clientSettings );
        if ( !server->RegisterService( notes, "Notes", &service, mode ) )
            return false;
        client->AddWantedContract( "gucef.sample.notes", 1 );
        if ( !server->Start( serverTransport, false ) || !client->Start( clientTransport, true ) )
            return false;
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        while ( !client->IsContractBound( "gucef.sample.notes", 1 ) )
        {
            PumpOnce();
            if ( GUCEF::MT::PrecisionTickCount() - start > 10 * GUCEF::MT::PrecisionTimerResolution() )
                return false;
            GUCEF::MT::PrecisionDelay( 1 );
        }
        PumpFor( 5 );
        return true;
    }

    GUCEF::COMCORE::CRpcMessage NewClientRequest( const char* path )
    {
        GUCEF::COMCORE::CRpcMethodRef ref;
        GUCEF::COMCORE::CRpcStatus status;
        if ( !client->ResolveMethod( path, ref, status ) )
            return GUCEF::COMCORE::CRpcMessage();
        return client->NewRequest( ref );
    }

    GUCEF::COMCORE::CRpcMessage GetRequest( GUCEF::UInt64 noteId )
    {
        GUCEF::COMCORE::CRpcMessage request = NewClientRequest( "gucef.sample.notes.Notes/Get" );
        request.SetUInt64( 1, noteId );
        return request;
    }

    GUCEF::COMCORE::CRpcCallResult CallAndPump( const char* path, const GUCEF::COMCORE::CRpcMessage& request,
                                                const GUCEF::COMCORE::CRpcCallOptions& options = GUCEF::COMCORE::CRpcCallOptions(),
                                                GUCEF::UInt32 timeoutMs = 10000 )
    {
        GUCEF::COMCORE::CRpcCallResult result;
        GUCEF::CORE::CFutureResult future = client->CallAsync( path, request, options );
        if ( !PumpUntilReady( future, timeoutMs ) )
        {
            result.status.Set( GUCEF::COMCORE::RPC_STATUS_DEADLINE_EXCEEDED, "test timeout" );
            future.RequestCancellation();
            return result;
        }
        GUCEF::COMCORE::CRpcEndpoint::TryGetCallResult( future, result );
        return result;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_RPC_CONFORMANCE_OK( s )       { GUCEF::COMCORE::CRpcStatus conformanceStatus_( s ); if ( !conformanceStatus_.IsOk() ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcConformance (" << transports.GetName() << "): line " << __LINE__ << ": " << conformanceStatus_.ToString() ); } }
#define GUCEF_RPC_CONFORMANCE_REQUIRE( t )  { if ( !( t ) ) { GUCEF_TESTFW_ERRORHERE; GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcConformance (" << transports.GetName() << "): requirement failed: " #t ); return; } }

/*-------------------------------------------------------------------------*/

/** Unary calls: an answer, an error status of the handler, metadata, the blocking form */
inline void
RpcConformanceUnary( CIRpcConformanceTransports& transports )
{
    CRpcConformanceFixture f( transports );
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
    GUCEF::COMCORE::CRpcCallOptions options;
    options.metadata.Set( "x-trace", GUCEF::CORE::CVariant( "abc" ) );
    GUCEF::COMCORE::CRpcCallResult result = f.CallAndPump( "gucef.sample.notes.Notes/Get", f.GetRequest( 5 ), options );
    GUCEF_RPC_CONFORMANCE_OK( result.status );
    GUCEF_TESTFW_ASSERT_TRUE( "note 5" == result.response.GetString( 2 ) );
    GUCEF_TESTFW_ASSERT_TRUE( "abc" == f.service.lastMetadata.GetString( "x-trace" ) );
    result = f.CallAndPump( "gucef.sample.notes.Notes/Get", f.GetRequest( 404 ) );
    GUCEF_TESTFW_ASSERT_TRUE( GUCEF::COMCORE::RPC_STATUS_NOT_FOUND == result.status.code );

    GUCEF::COMCORE::CRpcMessage response;
    GUCEF::COMCORE::CRpcStatus status;
    GUCEF_TESTFW_ASSERT_TRUE( f.client->Call( "gucef.sample.notes.Notes/Get", f.GetRequest( 7 ), response, status ) );
    GUCEF_RPC_CONFORMANCE_OK( status );
    GUCEF_TESTFW_ASSERT_TRUE( 7 == response.GetUInt64( 1 ) );
}

/*-------------------------------------------------------------------------*/

/** A server stream past the initial credit arrives complete and in order; a held one is cancelled by the caller */
inline void
RpcConformanceServerStream( CIRpcConformanceTransports& transports )
{
    CRpcConformanceFixture f( transports );
    f.service.watchCount = 200;
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
    GUCEF::COMCORE::CRpcMethodRef ref;
    GUCEF::COMCORE::CRpcStatus status;
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Watch", ref, status ) );
    {
        CRpcStreamCollector collector;
        GUCEF::COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        GUCEF::COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        GUCEF_RPC_CONFORMANCE_REQUIRE( !stream.IsNULL() );
        collector.Watch( stream );
        GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilFlag( collector.ended, 10000 ) );
        GUCEF_RPC_CONFORMANCE_OK( collector.status );
        bool inOrder = 200 == collector.noteIds.size();
        for ( GUCEF::UInt32 i=0; inOrder && i<collector.noteIds.size(); ++i )
            inOrder = collector.noteIds[ i ] == i + 1;
        GUCEF_TESTFW_ASSERT_TRUE( inOrder );
    }
    {
        f.service.watchCount = 2;
        f.service.holdWatch = true;
        CRpcStreamCollector collector;
        GUCEF::COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        GUCEF::COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        GUCEF_RPC_CONFORMANCE_REQUIRE( !stream.IsNULL() );
        collector.Watch( stream );
        GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilCount( collector.messages, 2, 10000 ) );
        GUCEF::UInt32 cancelledBefore = f.service.GetCancelledCalls();
        stream->Cancel( GUCEF::COMCORE::CRpcStatus( GUCEF::COMCORE::RPC_STATUS_CANCELLED, "enough" ) );
        GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilCancelledCalls( cancelledBefore + 1, 10000 ) );
        GUCEF_TESTFW_ASSERT_TRUE( GUCEF::COMCORE::RPC_STATUS_CANCELLED == stream->GetStatus().code );
        f.service.holdWatch = false;
    }
}

/*-------------------------------------------------------------------------*/

/** A client stream with one response, a bidi stream echoing through the blocking reader */
inline void
RpcConformanceClientAndBidiStreams( CIRpcConformanceTransports& transports )
{
    CRpcConformanceFixture f( transports );
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
    GUCEF::COMCORE::CRpcMethodRef ref;
    GUCEF::COMCORE::CRpcStatus status;
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Import", ref, status ) );
    GUCEF::COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, GUCEF_NULL );
    GUCEF_RPC_CONFORMANCE_REQUIRE( !stream.IsNULL() );
    for ( GUCEF::UInt32 i=0; i<5; ++i )
    {
        GUCEF::COMCORE::CRpcMessage chunk = f.client->NewRequest( ref );
        char data[ 100 ];
        memset( data, 'a' + (int) i, sizeof( data ) );
        chunk.SetBytes( 1, data, sizeof( data ) );
        GUCEF_TESTFW_ASSERT_TRUE( stream->Write( chunk ) );
    }
    GUCEF_TESTFW_ASSERT_TRUE( stream->WritesDone() );
    GUCEF::COMCORE::CRpcMessage message;
    GUCEF_TESTFW_ASSERT_FALSE( stream->ReadNext( message, status, 10000 ) );
    GUCEF_RPC_CONFORMANCE_OK( status );
    GUCEF_TESTFW_ASSERT_TRUE( 5 == stream->GetResponse().GetUInt32( 1 ) );

    GUCEF_RPC_CONFORMANCE_REQUIRE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Edit", ref, status ) );
    GUCEF::COMCORE::CRpcMessage first = f.client->NewRequest( ref );
    first.SetUInt64( 1, 1 );
    first.SetString( 2, "first" );
    stream = f.client->OpenStream( ref, &first );
    GUCEF_RPC_CONFORMANCE_REQUIRE( !stream.IsNULL() );
    for ( GUCEF::UInt32 i=2; i<=3; ++i )
    {
        GUCEF::COMCORE::CRpcMessage op = f.client->NewRequest( ref );
        op.SetUInt64( 1, i );
        GUCEF_TESTFW_ASSERT_TRUE( stream->Write( op ) );
    }
    GUCEF::COMCORE::CRpcMessage echo;
    for ( GUCEF::UInt32 i=1; i<=3; ++i )
    {
        GUCEF_TESTFW_ASSERT_TRUE( stream->ReadNext( echo, status, 10000 ) );
        GUCEF_TESTFW_ASSERT_TRUE( i == echo.GetUInt64( 1 ) );
    }
    GUCEF_TESTFW_ASSERT_TRUE( stream->WritesDone() );
    GUCEF_TESTFW_ASSERT_FALSE( stream->ReadNext( echo, status, 10000 ) );
    GUCEF_RPC_CONFORMANCE_OK( status );
}

/*-------------------------------------------------------------------------*/

/** ACCEPTED with a request id, STATUS_QUERY while it runs, then RESULT */
inline void
RpcConformanceAcceptedAndResult( CIRpcConformanceTransports& transports )
{
    CRpcConformanceFixture f( transports );
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
    CRpcEndpointEventRecorder events;
    events.Watch( f.client );
    GUCEF::COMCORE::CRpcCallOptions options;
    options.metadata.Set( "id-kind", GUCEF::CORE::CVariant( "string" ) );
    GUCEF::COMCORE::CRpcMessage request = f.NewClientRequest( "gucef.sample.notes.Notes/Reindex" );
    request.SetBool( 1, true );
    GUCEF::CORE::CFutureResult future = f.client->CallAsync( "gucef.sample.notes.Notes/Reindex", request, options );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilCount( events.accepted, 1, 10000 ) );
    GUCEF_TESTFW_ASSERT_FALSE( future.IsReady() );

    GUCEF::COMCORE::CRpcRequestStatus requestStatus;
    GUCEF::COMCORE::CRpcStatus status;
    GUCEF::COMCORE::CRpcRequestId id( GUCEF::CORE::CString( "job-1" ) );
    GUCEF_TESTFW_ASSERT_TRUE( f.client->QueryStatus( id, false, requestStatus, status ) );
    GUCEF_RPC_CONFORMANCE_OK( status );
    GUCEF_TESTFW_ASSERT_TRUE( GUCEF::COMCORE::RPC_REQUESTSTATE_RUNNING == requestStatus.state );

    GUCEF_TESTFW_ASSERT_TRUE( 1 == f.service.CompleteReindex( 42 ) );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilReady( future, 10000 ) );
    GUCEF::COMCORE::CRpcCallResult result;
    GUCEF_TESTFW_ASSERT_TRUE( GUCEF::COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
    GUCEF_RPC_CONFORMANCE_OK( result.status );
    GUCEF_TESTFW_ASSERT_TRUE( result.wasAccepted && 42 == result.response.GetUInt64( 1 ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Handlers on the notification thread (this thread) that call into the endpoints while the protocol threads raise
 *  the next events for it: a stream of messages each consumed here, a provider writing here on WritableEvent
 */
inline void
RpcConformanceNotificationThreadHandlers( CIRpcConformanceTransports& transports )
{
    CRpcConformanceFixture f( transports );
    const GUCEF::UInt32 messages = 1000;
    f.service.watchCount = messages;
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) );
    GUCEF::COMCORE::CRpcMethodRef ref;
    GUCEF::COMCORE::CRpcStatus status;
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Watch", ref, status ) );
    for ( GUCEF::UInt32 round=0; round<2; ++round )
    {
        CRpcStreamCollector collector;
        GUCEF::COMCORE::CRpcMessage watch = f.client->NewRequest( ref );
        GUCEF::COMCORE::CIRpcStreamPtr stream = f.client->OpenStream( ref, &watch );
        GUCEF_RPC_CONFORMANCE_REQUIRE( !stream.IsNULL() );
        collector.Watch( stream );
        GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilFlag( collector.ended, 20000 ) );
        GUCEF_RPC_CONFORMANCE_OK( collector.status );
        GUCEF_TESTFW_ASSERT_TRUE( messages == collector.messages );
        if ( !collector.ended )
            return;
    }
}

/*-------------------------------------------------------------------------*/

/** A graceful close: the open call is answered first, GOODBYE ends both sides */
inline void
RpcConformanceGracefulClose( CIRpcConformanceTransports& transports )
{
    CRpcConformanceFixture f( transports );
    GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
    CRpcEndpointEventRecorder clientEvents;
    CRpcEndpointEventRecorder serverEvents;
    clientEvents.Watch( f.client );
    serverEvents.Watch( f.server );
    GUCEF::CORE::CFutureResult future = f.client->CallAsync( "gucef.sample.notes.Notes/Get", f.GetRequest( 9 ) );
    f.client->Close( true );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilReady( future, 10000 ) );
    GUCEF::COMCORE::CRpcCallResult result;
    GUCEF_TESTFW_ASSERT_TRUE( GUCEF::COMCORE::CRpcEndpoint::TryGetCallResult( future, result ) );
    GUCEF_RPC_CONFORMANCE_OK( result.status );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilState( f.server, GUCEF::COMCORE::RPC_SESSIONSTATE_CLOSED, 10000 ) );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilState( f.client, GUCEF::COMCORE::RPC_SESSIONSTATE_CLOSED, 10000 ) );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilCount( clientEvents.closed, 1, 5000 ) );
    GUCEF_TESTFW_ASSERT_TRUE( f.PumpUntilCount( serverEvents.closed, 1, 5000 ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Endpoints and transports torn down with thousands of calls outstanding (design 10.9): the futures let go before the
 *  endpoints (as RpcPerfUnary() does when it gives up), kept beyond them, and the endpoints let go without Close().
 *  Every future completes. GUCEF_RPC_TEARDOWN_ROUNDS repeats the three as often as asked (sanitizer loops)
 */
inline void
RpcConformanceTeardownWithCallsOutstanding( CIRpcConformanceTransports& transports )
{
    // a quarter under the sanitizers, which slow the protocol threads down so much that a drn link with 6000 calls
    // to deliver misses its liveness timeout
    #if defined( __SANITIZE_THREAD__ ) || defined( __SANITIZE_ADDRESS__ )
    const GUCEF::UInt32 calls = 1500;
    #else
    const GUCEF::UInt32 calls = 6000;
    #endif
    GUCEF::UInt32 rounds = GUCEF::CORE::StringToUInt32( GUCEF::CORE::GetEnv( "GUCEF_RPC_TEARDOWN_ROUNDS" ), 1 );
    if ( 0 == rounds )
        rounds = 1;
    GUCEF::UInt32 notReady = 0;
    for ( GUCEF::UInt32 round=0; round<3*rounds; ++round )
    {
        const GUCEF::UInt32 variant = round % 3;
        GUCEF::vector< GUCEF::CORE::CFutureResult > outliving;
        {
            CRpcConformanceFixture f( transports );
            GUCEF_RPC_CONFORMANCE_REQUIRE( f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) );
            GUCEF::COMCORE::CRpcMethodRef ref;
            GUCEF::COMCORE::CRpcStatus status;
            GUCEF_RPC_CONFORMANCE_REQUIRE( f.client->ResolveMethod( "gucef.sample.notes.Notes/Get", ref, status ) );
            GUCEF::COMCORE::CRpcMessage request = f.client->NewRequest( ref );
            request.SetUInt64( 1, 1 );
            GUCEF::COMCORE::CRpcCallOptions options;
            options.deadlineMs = 300000;
            GUCEF::vector< GUCEF::CORE::CFutureResult > futures;
            futures.reserve( calls );
            for ( GUCEF::UInt32 i=0; i<calls; ++i )
            {
                futures.push_back( f.client->CallAsync( ref, request, options ) );
                if ( 0 == i % 1000 )
                    f.PumpOnce();
            }
            f.PumpOnce();
            if ( variant >= 1 )
                outliving = futures;
            if ( 2 == variant )
            {
                f.client.Unlink();
                f.server.Unlink();
            }
            // the futures go first, then the fixture: its endpoints, then its transports
        }
        GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        while ( GUCEF::MT::PrecisionTickCount() - start < GUCEF::MT::PrecisionTimerResolution() / 50 )
        {
            pulseGenerator->ForceDirectPulse();
            transports.Pump();
            GUCEF::MT::PrecisionDelay( 1 );
        }
        for ( GUCEF::UInt32 i=0; i<outliving.size(); ++i )
            if ( !outliving[ i ].IsReady() )
                ++notReady;
    }
    if ( 0 != notReady )
        GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcConformance (" << transports.GetName() << "): " << notReady << " futures did not complete when their endpoint went" );
    GUCEF_TESTFW_ASSERT_TRUE( 0 == notReady );
}

#undef GUCEF_RPC_CONFORMANCE_OK
#undef GUCEF_RPC_CONFORMANCE_REQUIRE

/*-------------------------------------------------------------------------*/

#define GUCEF_RPC_CONFORMANCE_CASE( name, call )                                                \
    GUCEF_TESTFW_TESTCASE( ( prefix + name ).C_String() )                                       \
    try                                                                                         \
    {                                                                                           \
        call;                                                                                   \
    }                                                                                           \
    catch ( ... )                                                                               \
    {                                                                                           \
        GUCEF_TESTFW_ERRORHERE;                                                                 \
    }                                                                                           \
    GUCEF_TESTFW_TESTCASE_END;

/**
 *  The conformance suite over the given transports: unary calls, all stream kinds with credits and cancellation,
 *  ACCEPTED + RESULT with STATUS_QUERY, handlers on the notification thread, graceful close, teardown with thousands of
 *  calls outstanding
 */
inline void
PerformRpcConformanceTests( CIRpcConformanceTransports& transports )
{
    GUCEF::CORE::CString prefix = GUCEF::CORE::CString( "rpc_conformance (" ) + transports.GetName() + "): ";
    GUCEF_TESTFW_SUITE_SCOPE( "rpc_conformance" );

    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "unary calls, error status, metadata, blocking form" ), RpcConformanceUnary( transports ) );
    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "server stream past its credit in order, cancelled by the caller" ), RpcConformanceServerStream( transports ) );
    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "client stream and bidi stream" ), RpcConformanceClientAndBidiStreams( transports ) );
    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "ACCEPTED, STATUS_QUERY, RESULT" ), RpcConformanceAcceptedAndResult( transports ) );
    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "handlers on the notification thread call into the endpoints" ), RpcConformanceNotificationThreadHandlers( transports ) );
    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "graceful close with GOODBYE" ), RpcConformanceGracefulClose( transports ) );
    GUCEF_RPC_CONFORMANCE_CASE( GUCEF::CORE::CString( "endpoints and transports torn down with thousands of calls outstanding" ), RpcConformanceTeardownWithCallsOutstanding( transports ) );
}

#undef GUCEF_RPC_CONFORMANCE_CASE

/*-------------------------------------------------------------------------*/

/** Call counts of the performance measures: a tenth under the sanitizers, which slow everything down many times */
inline GUCEF::UInt32
RpcPerfCalls( GUCEF::UInt32 calls )
{
    #if defined( __SANITIZE_THREAD__ ) || defined( __SANITIZE_ADDRESS__ )
    return calls / 10;
    #else
    return calls;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Figures of RpcPerfUnary() */
struct SRpcUnaryFigures
{
    GUCEF::UInt32 latencyCalls;
    GUCEF::Float64 medianUs;            /**< round trip of one call at a time, from CallAsync() to the ready future */
    GUCEF::Float64 p99Us;
    GUCEF::UInt32 pipelinedCalls;
    GUCEF::Float64 callsPerSecond;      /**< all calls issued at once, until the last future is ready */
    bool complete;

    SRpcUnaryFigures( void ) : latencyCalls( 0 ), medianUs( 0 ), p99Us( 0 ), pipelinedCalls( 0 ), callsPerSecond( 0 ), complete( false ) {}
};

/**
 *  Unary latency and throughput (design 15) over a pair of the transports: the handler runs on the protocol thread,
 *  the test thread issues the calls and waits for the futures
 */
inline bool
RpcPerfUnary( CIRpcConformanceTransports& transports, GUCEF::UInt32 latencyCalls, GUCEF::UInt32 pipelinedCalls, SRpcUnaryFigures& figures )
{
    CRpcConformanceFixture f( transports );
    if ( !f.Connect( GUCEF::COMCORE::RPC_EXECUTION_CALLING_THREAD ) )
        return false;
    GUCEF::COMCORE::CRpcMethodRef ref;
    GUCEF::COMCORE::CRpcStatus status;
    if ( !f.client->ResolveMethod( "gucef.sample.notes.Notes/Get", ref, status ) )
        return false;
    GUCEF::COMCORE::CRpcMessage request = f.client->NewRequest( ref );
    request.SetUInt64( 1, 1 );
    const GUCEF::Float64 ticksPerUs = GUCEF::MT::PrecisionTimerResolution() / 1000000.0;

    // warm up, then one call at a time
    for ( GUCEF::UInt32 i=0; i<50; ++i )
    {
        GUCEF::CORE::CFutureResult future = f.client->CallAsync( ref, request );
        if ( !f.PumpUntilReady( future, 10000 ) )
            return false;
    }
    GUCEF::vector< GUCEF::Float64 > samples;
    samples.reserve( latencyCalls );
    for ( GUCEF::UInt32 i=0; i<latencyCalls; ++i )
    {
        GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
        GUCEF::CORE::CFutureResult future = f.client->CallAsync( ref, request );
        while ( !future.IsReady() )
            f.PumpOnce();
        samples.push_back( ( GUCEF::MT::PrecisionTickCount() - start ) / ticksPerUs );
    }
    std::sort( samples.begin(), samples.end() );
    figures.latencyCalls = latencyCalls;
    figures.medianUs = samples.empty() ? 0.0 : samples[ samples.size() / 2 ];
    figures.p99Us = samples.empty() ? 0.0 : samples[ ( samples.size() * 99 ) / 100 ];

    // pipelined: everything issued at once (the method's own deadline is not what this measures)
    GUCEF::COMCORE::CRpcCallOptions options;
    options.deadlineMs = 300000;
    GUCEF::vector< GUCEF::CORE::CFutureResult > futures;
    futures.reserve( pipelinedCalls );
    GUCEF::UInt64 start = GUCEF::MT::PrecisionTickCount();
    for ( GUCEF::UInt32 i=0; i<pipelinedCalls; ++i )
        futures.push_back( f.client->CallAsync( ref, request, options ) );
    GUCEF::Float64 issueUs = ( GUCEF::MT::PrecisionTickCount() - start ) / ticksPerUs;
    GUCEF::UInt32 done = 0;
    GUCEF::UInt64 limit = 120 * GUCEF::MT::PrecisionTimerResolution();
    while ( done < futures.size() && GUCEF::MT::PrecisionTickCount() - start < limit )
    {
        while ( done < futures.size() && futures[ done ].IsReady() )
            ++done;
        if ( done < futures.size() )
            f.PumpOnce();
    }
    GUCEF::Float64 elapsedUs = ( GUCEF::MT::PrecisionTickCount() - start ) / ticksPerUs;
    figures.pipelinedCalls = pipelinedCalls;
    figures.callsPerSecond = elapsedUs > 0.0 ? pipelinedCalls * 1000000.0 / elapsedUs : 0.0;
    GUCEF::UInt32 ok = 0;
    GUCEF::COMCORE::CRpcStatus firstFailure;
    for ( GUCEF::UInt32 i=0; i<futures.size(); ++i )
    {
        GUCEF::COMCORE::CRpcCallResult result;
        if ( GUCEF::COMCORE::CRpcEndpoint::TryGetCallResult( futures[ i ], result ) && result.status.IsOk() )
            ++ok;
        else if ( firstFailure.IsOk() )
            firstFailure = result.status;
    }
    if ( ok != pipelinedCalls )
        GUCEF_LOG( GUCEF::CORE::LOGLEVEL_CRITICAL, "RpcPerf (" << transports.GetName() << "): " << pipelinedCalls - ok << " pipelined calls failed, the first with " << firstFailure.ToString() );
    figures.complete = ok == pipelinedCalls;
    GUCEF_LOG( GUCEF::CORE::LOGLEVEL_IMPORTANT, "RpcPerf (" << transports.GetName() << "): unary round trip median " << figures.medianUs
               << " us, p99 " << figures.p99Us << " us over " << latencyCalls << " calls; " << pipelinedCalls << " pipelined calls: "
               << figures.callsPerSecond << " calls/s (issued in " << issueUs / 1000.0 << " ms)" );
    return figures.complete;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTAPP_TESTRPCCONFORMANCE_H ? */
