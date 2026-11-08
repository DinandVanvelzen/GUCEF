/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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
#include <algorithm>
#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#include "gucefCOMCORE_CRpcRoleAccessPolicy.h"
#define GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H
#endif /* GUCEF_COMCORE_CRPCROLEACCESSPOLICY_H ? */

#ifndef GUCEF_COM_CCOMGLOBAL_H
#include "gucefCOM_CComGlobal.h"
#define GUCEF_COM_CCOMGLOBAL_H
#endif /* GUCEF_COM_CCOMGLOBAL_H ? */

#ifndef GUCEF_COM_CMCPRPCTRANSPORT_H
#include "gucefCOM_CMcpRpcTransport.h"
#define GUCEF_COM_CMCPRPCTRANSPORT_H
#endif /* GUCEF_COM_CMCPRPCTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPRPCRELAY_H
#include "gucefCOM_CMcpRpcRelay.h"
#define GUCEF_COM_CMCPRPCRELAY_H
#endif /* GUCEF_COM_CMCPRPCRELAY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

// the RPC test support of gucefCOMCORE_TestApp: the sample contract, its test service, the endpoint fixture and the
// capturing transport decorator
#include "TestRpcSupport.h"
#include "TestRpcNotesService.h"
#include "TestRpcLossyTransport.h"
#include "TestRpcEndpointFixture.h"
#include "TestRpcConformance.h"

// the MCP test support of gucefCOM_TestApp: the test tool and the JSON-RPC request helpers
#include "TestMcpSupport.h"

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNLossyChannel.h"
#include "TestDRNRpcTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

/*-------------------------------------------------------------------------*/

const char* g_getPath = "gucef.sample.notes.Notes/Get";
const char* g_savePath = "gucef.sample.notes.Notes/Save";
const char* g_watchPath = "gucef.sample.notes.Notes/Watch";
const char* g_importPath = "gucef.sample.notes.Notes/Import";
const char* g_editPath = "gucef.sample.notes.Notes/Edit";
const char* g_reindexPath = "gucef.sample.notes.Notes/Reindex";
const char* g_listTagsPath = "gucef.sample.notes.PublicIndex/ListTags";

typedef CORE::CTSharedPtr< DRN::CDRNRpcTransport, MT::CMutex >            CDRNRpcTransportPtr;
typedef CORE::CTSharedPtr< DRN::CDRNRpcTransportListener, MT::CMutex >    CDRNRpcTransportListenerPtr;

/*-------------------------------------------------------------------------*/

/** Counts the messages of one class a session hands it */
class CClassProbe : public DRN::CIDRNSessionComponent
{
    public:
    UInt8 msgClass;
    UInt32 received;
    explicit CClassProbe( UInt8 probedClass ) : DRN::CIDRNSessionComponent(), msgClass( probedClass ), received( 0 ) {}
    virtual UInt8 GetMessageClass( void ) const GUCEF_VIRTUAL_OVERRIDE { return msgClass; }
    virtual void OnSessionMessage( DRN::CDRNSession&, DRN::CDRNPeerLink&, UInt32, const DRN::CDRNLinkMessage& ) GUCEF_VIRTUAL_OVERRIDE { ++received; }
    virtual void OnSessionTick( DRN::CDRNSession&, UInt64 ) GUCEF_VIRTUAL_OVERRIDE {}
};

/*-------------------------------------------------------------------------*/

UInt32
GetNodeCounter( DRN::CDRNNode& node, const char* name )
{
    CORE::CDataNode telemetry;
    node.GetTelemetry( telemetry );
    return telemetry.GetAttributeValue( name ).AsUInt32( 0, true );
}

/*-------------------------------------------------------------------------*/

UInt64
ElapsedMicros( UInt64 startTicks )
{
    UInt64 ticksPerUs = MT::PrecisionTimerResolution() / 1000000;
    if ( 0 == ticksPerUs )
        ticksPerUs = 1;
    return ( MT::PrecisionTickCount() - startTicks ) / ticksPerUs;
}

/*-------------------------------------------------------------------------*/

/** Pulses the global pulse generator until the flag is set, no pause between the pulses (latency measurements) */
bool
SpinUntilFlag( const bool& flag, UInt32 timeoutMs )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 start = MT::PrecisionTickCount();
    while ( !flag )
    {
        pulseGenerator->ForceDirectPulse();
        if ( ElapsedMicros( start ) > (UInt64) timeoutMs * 1000 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
Median( GUCEF::vector< UInt64 >& values )
{
    if ( values.empty() )
        return 0;
    std::sort( values.begin(), values.end() );
    return values[ values.size() / 2 ];
}

/*-------------------------------------------------------------------------*/

/** The PublicIndex service of the sample contract: ListTags streams NoteEvents whose titles carry a marker */
class CPublicIndexService : public COMCORE::CRpcServiceImpl
{
    public:

    typedef COMCORE::CTRpcMethodHandler< CPublicIndexService > THandler;

    UInt32 eventCount;
    CORE::CString marker;

    CPublicIndexService( void )
        : COMCORE::CRpcServiceImpl(), eventCount( 3 ), marker( "PUBLICMARKER" )
    {
        BindMethod( "ListTags", GUCEF_NEW THandler( this, &CPublicIndexService::OnListTags ) );
    }

    COMCORE::TRpcHandlerResult OnListTags( COMCORE::CRpcServerCall& call )
    {
        for ( UInt32 i=0; i<eventCount; ++i )
        {
            COMCORE::CRpcMessage event = call.NewResponse();
            event.GetMutableMessage( 1 )->SetUInt64( 1, i + 1 );
            event.GetMutableMessage( 1 )->SetString( 2, marker + CORE::ToString( 1000 + i ) );
            call.Write( event );
        }
        call.Finish();
        return COMCORE::RPC_HANDLER_DONE;
    }
};

/*-------------------------------------------------------------------------*/

/** Collects what a client stream delivers */
class CStreamCollector : public CORE::CObserver
{
    public:

    UInt32 messages;
    GUCEF::vector< UInt64 > noteIds;
    GUCEF::vector< CORE::CString > titles;
    bool ended;
    COMCORE::CRpcStatus status;

    CStreamCollector( void ) : CORE::CObserver(), messages( 0 ), noteIds(), titles(), ended( false ), status() {}
    virtual ~CStreamCollector() { UnsubscribeAllFromObserver(); }

    void Watch( const COMCORE::CIRpcStreamPtr& stream )
    {
        typedef CORE::CTEventHandlerFunctor< CStreamCollector > TEventCallback;
        COMCORE::CIRpcStreamPtr s( stream );
        TEventCallback message( this, &CStreamCollector::OnMessage );
        TEventCallback end( this, &CStreamCollector::OnStatus );
        SubscribeTo( s.GetPointerAlways(), COMCORE::CIRpcStream::MessageEvent, message );
        SubscribeTo( s.GetPointerAlways(), COMCORE::CIRpcStream::StatusEvent, end );
    }

    private:

    void OnMessage( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        ++messages;
        const COMCORE::CRpcMessage& message = static_cast< COMCORE::CRpcStreamMessageEventData* >( eventData )->message;
        const COMCORE::CRpcMessage* note = message.GetMessage( 1 );
        noteIds.push_back( GUCEF_NULL != note ? note->GetUInt64( 1 ) : 0 );
        titles.push_back( GUCEF_NULL != note ? note->GetString( 2 ) : CORE::CString::Empty );
    }

    void OnStatus( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        ended = true;
        if ( GUCEF_NULL != eventData )
            status = *static_cast< COMCORE::CRpcStatus* >( eventData );
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the events of an RPC transport */
class CTransportEvents : public CORE::CObserver
{
    public:

    UInt32 connected;
    UInt32 disconnected;
    UInt32 frames;
    COMCORE::CRpcStatus lastStatus;
    TThreadId eventThread;

    CTransportEvents( void ) : CORE::CObserver(), connected( 0 ), disconnected( 0 ), frames( 0 ), lastStatus(), eventThread( 0 ) {}
    virtual ~CTransportEvents() { UnsubscribeAllFromObserver(); }

    void Watch( COMCORE::CIRpcTransport& transport )
    {
        typedef CORE::CTEventHandlerFunctor< CTransportEvents > TEventCallback;
        TEventCallback connectedCb( this, &CTransportEvents::OnConnected );
        TEventCallback disconnectedCb( this, &CTransportEvents::OnDisconnected );
        TEventCallback frameCb( this, &CTransportEvents::OnFrame );
        SubscribeTo( &transport, COMCORE::CIRpcTransport::ConnectedEvent, connectedCb );
        SubscribeTo( &transport, COMCORE::CIRpcTransport::DisconnectedEvent, disconnectedCb );
        SubscribeTo( &transport, COMCORE::CIRpcTransport::FrameReceivedEvent, frameCb );
    }

    private:

    void OnConnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++connected; eventThread = MT::GetCurrentThreadID(); }
    void OnFrame( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { ++frames; eventThread = MT::GetCurrentThreadID(); }
    void OnDisconnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        eventThread = MT::GetCurrentThreadID();
        if ( GUCEF_NULL != eventData )
            lastStatus = *static_cast< COMCORE::CRpcStatus* >( eventData );
        ++disconnected;
    }
};

/*-------------------------------------------------------------------------*/

/** The disconnect of an RPC transport seen from another thread */
class CDisconnectWatch : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > disconnected;
    MT::CTAtomic< TThreadId > eventThread;

    CDisconnectWatch( void ) : CORE::CObserver(), disconnected( 0 ), eventThread( 0 ) {}
    virtual ~CDisconnectWatch() { UnsubscribeAllFromObserver(); }

    void Watch( COMCORE::CIRpcTransport& transport )
    {
        typedef CORE::CTEventHandlerFunctor< CDisconnectWatch > TEventCallback;
        TEventCallback disconnectedCb( this, &CDisconnectWatch::OnDisconnected );
        SubscribeTo( &transport, COMCORE::CIRpcTransport::DisconnectedEvent, disconnectedCb );
    }

    private:

    void OnDisconnected( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        eventThread.Store( MT::GetCurrentThreadID() );
        disconnected.Store( disconnected.Load() + 1 );
    }
};

/*-------------------------------------------------------------------------*/

/** Sends every REQUEST frame as PUBLIC whatever its method requires: a sender that downgrades */
class CDowngradingTransport : public CRpcLossyTransport
{
    public:

    bool downgradeRequests;

    explicit CDowngradingTransport( const COMCORE::CIRpcTransportPtr& inner ) : CRpcLossyTransport( inner ), downgradeRequests( false ) {}

    virtual bool SendFrame( const void* frame, UInt32 size, COMCORE::TRpcProtectionClass protection ) GUCEF_VIRTUAL_OVERRIDE
    {
        COMCORE::SRpcFrameHeader header;
        if ( downgradeRequests && COMCORE::CRpcFrameCodec::ParseHeader( static_cast< const UInt8* >( frame ), size, header ) &&
             GUCEF_RPC_FRAME_REQUEST == header.type )
            protection = COMCORE::RPC_PROTECTION_PUBLIC;
        return CRpcLossyTransport::SendFrame( frame, size, protection );
    }
};

/*-------------------------------------------------------------------------*/

/** Keeps the text of every log message while it is attached to the log manager */
class CLogCapture : public CORE::CILogger
{
    public:

    CLogCapture( void ) : CORE::CILogger(), m_text(), m_lock() { CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this ); }
    virtual ~CLogCapture() { CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this ); }

    CORE::CString GetText( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        MT::CScopeMutex lock( m_lock );
        return m_text;
    }

    virtual void Log( const CORE::TLogMsgType, const Int32, const CORE::CString& logMessage, const TSystemThreadId, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE
    {
        MT::CScopeMutex lock( m_lock );
        m_text += logMessage;
        m_text += "\n";
    }
    virtual void Log( const CORE::TLogMsgType type, const Int32 level, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    { Log( type, level, logMessage.ToString(), threadId, timestamp ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType type, const Int32 level, const CORE::CString& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    { Log( type, level, logMessage, threadId, timestamp ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType type, const Int32 level, const CORE::CVariantStream& logMessage, const TSystemThreadId threadId, const CORE::CTimestamp& timestamp ) GUCEF_VIRTUAL_OVERRIDE
    { Log( type, level, logMessage.ToString(), threadId, timestamp ); }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    CORE::CString m_text;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

bool
Contains( const CORE::CDynamicBuffer& buffer, const CORE::CString& text )
{
    const UInt8* data = static_cast< const UInt8* >( buffer.GetConstBufferPtr() );
    UInt32 size = buffer.GetDataSize();
    UInt32 length = text.Length();
    if ( 0 == length || size < length )
        return false;
    for ( UInt32 i=0; i+length<=size; ++i )
        if ( 0 == memcmp( data + i, text.C_String(), length ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  A DRN session of nodes on the global pulse generator (CGroup) with an RPC session between two of its members:
 *  the RPC endpoint fixture of gucefCOMCORE_TestApp (registries, endpoints, the notes test service, the pump helpers)
 *  over drn transports, the server side accepted by a CDRNRpcTransportListener. Both endpoint transports are wrapped
 *  in the capturing decorator.
 */
class CRpcDrnWorld : public CORE::CObserver
{
    public:

    CGroup group;
    CRpcEndpointFixture f;
    DRN::CDRNRpcTransportSettings transportSettings;
    CDRNRpcTransportListenerPtr listener;
    CDRNRpcTransportPtr clientDrn;
    COMCORE::CRpcContractPtr contract;
    CPublicIndexService publicIndex;
    UInt32 accepted;
    bool startServerOnAccept;
    bool downgradeClient;
    CDowngradingTransport* clientDowngrader;
    COMCORE::CIRpcTransportPtr lastAccepted;
    COMCORE::TRpcExecutionMode mode;
    CDisconnectWatch* clientWatch;              /**< subscribed to the client transport before anything flows on it */
    bool captureFrames;                         /**< both endpoints over the capturing decorator (off for measurements) */

    CRpcDrnWorld( DRN::TDRNTopology topology, const char* name, const char* channel = "tcp", const char* params = "" )
        : CORE::CObserver(), group( topology, name, channel, params ), f(), transportSettings(), listener(), clientDrn(), contract()
        , publicIndex(), accepted( 0 ), startServerOnAccept( true ), downgradeClient( false ), clientDowngrader( GUCEF_NULL )
        , lastAccepted(), mode( COMCORE::RPC_EXECUTION_CALLING_THREAD ), clientWatch( GUCEF_NULL ), captureFrames( true )
    {
        // tests run without the native console logger: endpoints keep their timers per pulse
        f.clientSettings.nodeName = "drn-client";
        f.serverSettings.nodeName = "drn-server";
    }

    virtual ~CRpcDrnWorld()
    {
        UnsubscribeAllFromObserver();
        if ( !listener.IsNULL() )
            listener->StopListening();
    }

    bool Start( UInt32 members = 2 )
    {
        return group.Start( members );
    }

    /** The endpoints: the server side on member serverIndex accepts through a listener, the client connects from clientIndex */
    bool Connect( UInt32 clientIndex = 1, UInt32 serverIndex = 0, bool waitForBinding = true )
    {
        if ( !Listen( serverIndex ) )
            return false;
        if ( !ConnectClient( clientIndex, serverIndex ) )
            return false;
        return !waitForBinding || f.PumpUntilBound( "gucef.sample.notes", 1, 5000 );
    }

    bool Listen( UInt32 serverIndex )
    {
        if ( contract.IsNULL() )
        {
            contract = LoadSampleNotesContract( 1, 2 );
            if ( contract.IsNULL() )
                return false;
            f.clientRegistry.Register( contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        }
        f.server = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.serverSettings );
        if ( !f.server->RegisterService( contract, "Notes", &f.service, mode ) ||
             !f.server->RegisterService( contract, "PublicIndex", &publicIndex, mode ) )
            return false;
        listener = CDRNRpcTransportListenerPtr( GUCEF_NEW DRN::CDRNRpcTransportListener( group[ serverIndex ], transportSettings ) );
        typedef CORE::CTEventHandlerFunctor< CRpcDrnWorld > TEventCallback;
        TEventCallback acceptedCb( this, &CRpcDrnWorld::OnAccepted );
        SubscribeTo( listener.GetPointerAlways(), COMCORE::CIRpcTransportListener::TransportAcceptedEvent, acceptedCb );
        return listener->Listen( CORE::CUri() );
    }

    bool ConnectClient( UInt32 clientIndex, UInt32 serverIndex )
    {
        f.client = COMCORE::CRpcEndpoint::Create( f.pulseGenerator, f.clientSettings );
        f.client->AddWantedContract( "gucef.sample.notes", 1 );
        clientDrn = CDRNRpcTransportPtr( GUCEF_NEW DRN::CDRNRpcTransport( group[ clientIndex ], group[ serverIndex ].GetLocalPeerId(), transportSettings ) );
        if ( !clientDrn->Connect( CORE::CUri() ) )
            return false;
        if ( GUCEF_NULL != clientWatch )
            clientWatch->Watch( *clientDrn );
        COMCORE::CIRpcTransportPtr inner;
        inner = clientDrn;
        if ( downgradeClient )
        {
            CORE::CTSharedPtr< CDowngradingTransport, MT::CMutex > downgrader( GUCEF_NEW CDowngradingTransport( inner ) );
            clientDowngrader = downgrader.GetPointerAlways();
            f.clientWire = clientDowngrader;
            f.clientTransport = downgrader;
        }
        else
        if ( captureFrames )
        {
            f.clientTransport = CRpcLossyTransport::Wrap( inner, &f.clientWire );
        }
        else
        {
            f.clientTransport = inner;
        }
        return f.client->Start( f.clientTransport, true );
    }

    /** Closes both endpoints from this thread */
    void CloseEndpoints( void )
    {
        if ( !f.client.IsNULL() )
            f.client->Close( false );
        if ( !f.server.IsNULL() )
            f.server->Close( false );
    }

    /** A unary call through the async API, pumped */
    COMCORE::CRpcCallResult Call( const char* path, UInt64 noteId, UInt32 timeoutMs = 5000 )
    {
        COMCORE::CRpcMessage request = f.NewClientRequest( path );
        request.SetUInt64( 1, noteId );
        return f.CallAndPump( path, request, COMCORE::CRpcCallOptions(), timeoutMs );
    }

    UInt32 ServerPeer( UInt32 serverIndex = 0 ) { return group[ serverIndex ].GetLocalPeerId(); }

    private:

    void OnAccepted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        ++accepted;
        COMCORE::CRpcTransportAcceptedEventData* data = static_cast< COMCORE::CRpcTransportAcceptedEventData* >( eventData );
        lastAccepted = data->transport;
        if ( !startServerOnAccept )
            return;
        if ( captureFrames )
            f.serverTransport = CRpcLossyTransport::Wrap( data->transport, &f.serverWire );
        else
            f.serverTransport = data->transport;
        f.server->Start( f.serverTransport, false );
    }
};

/*-------------------------------------------------------------------------*/

/** Unary, errors, server, client and bidi streams, ACCEPTED + RESULT and a fragmented message over a world */
void
RunCallKinds( CRpcDrnWorld& w, const char* label )
{
    COMCORE::CRpcCallResult result = w.Call( g_getPath, 5 );
    if ( !result.status.IsOk() )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC test: Get over " << label << ": " << result.status.ToString() );
    ASSERT_TRUE( result.status.IsOk() && "note 5" == result.response.GetString( 2 ) );
    result = w.Call( g_getPath, 404 );
    ASSERT_TRUE( COMCORE::RPC_STATUS_NOT_FOUND == result.status.code );

    COMCORE::CRpcMessage save = w.f.NewClientRequest( g_savePath );
    save.GetMutableMessage( 1 )->SetUInt64( 1, 77 );
    save.GetMutableMessage( 1 )->SetString( 2, "title" );
    result = w.f.CallAndPump( g_savePath, save );
    ASSERT_TRUE( result.status.IsOk() && 77 == result.response.GetUInt64( 1 ) );

    COMCORE::CRpcMethodRef ref;
    COMCORE::CRpcStatus status;

    // server stream
    {
        CStreamCollector collector;
        REQUIRE_TRUE( w.f.client->ResolveMethod( g_watchPath, ref, status ) );
        COMCORE::CRpcMessage watch = w.f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = w.f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( RpcTestPumpUntil( collector.ended, 5000 ) );
        ASSERT_TRUE( collector.status.IsOk() && 3 == collector.messages );
    }

    // client stream
    {
        REQUIRE_TRUE( w.f.client->ResolveMethod( g_importPath, ref, status ) );
        UInt64 chunksBefore = w.f.service.importedChunks;
        COMCORE::CIRpcStreamPtr stream = w.f.client->OpenStream( ref, GUCEF_NULL );
        REQUIRE_TRUE( !stream.IsNULL() );
        for ( UInt32 i=0; i<5; ++i )
        {
            COMCORE::CRpcMessage chunk = w.f.client->NewRequest( ref );
            char data[ 100 ];
            memset( data, 'a' + i, sizeof( data ) );
            chunk.SetBytes( 1, data, sizeof( data ) );
            ASSERT_TRUE( stream->Write( chunk ) );
        }
        ASSERT_TRUE( stream->WritesDone() );
        COMCORE::CRpcMessage message;
        ASSERT_FALSE( stream->ReadNext( message, status, 5000 ) );
        ASSERT_TRUE( status.IsOk() && w.f.service.importedChunks == chunksBefore + 5 );
    }

    // bidi stream, with a message far over the transport's frame size (RPC fragments it)
    {
        REQUIRE_TRUE( w.f.client->ResolveMethod( g_editPath, ref, status ) );
        COMCORE::CRpcMessage first = w.f.client->NewRequest( ref );
        first.SetUInt64( 1, 1 );
        first.SetString( 2, "first" );
        COMCORE::CIRpcStreamPtr stream = w.f.client->OpenStream( ref, &first );
        REQUIRE_TRUE( !stream.IsNULL() );
        CORE::CString big;
        for ( UInt32 i=0; i<6000; ++i )
            big += "0123456789";
        COMCORE::CRpcMessage op = w.f.client->NewRequest( ref );
        op.SetUInt64( 1, 2 );
        op.SetString( 2, big );
        ASSERT_TRUE( stream->Write( op ) );
        COMCORE::CRpcMessage echo;
        ASSERT_TRUE( stream->ReadNext( echo, status, 5000 ) && 1 == echo.GetUInt64( 1 ) );
        ASSERT_TRUE( stream->ReadNext( echo, status, 10000 ) && 2 == echo.GetUInt64( 1 ) && big == echo.GetString( 2 ) );
        ASSERT_TRUE( stream->WritesDone() );
        ASSERT_FALSE( stream->ReadNext( echo, status, 5000 ) );
        ASSERT_TRUE( status.IsOk() );
    }

    // ACCEPTED, then RESULT
    {
        COMCORE::CRpcMessage reindex = w.f.NewClientRequest( g_reindexPath );
        reindex.SetBool( 1, true );
        COMCORE::CRpcCallOptions options;
        options.metadata.Set( "id-kind", CORE::CVariant( "string" ) );
        CORE::CFutureResult future = w.f.client->CallAsync( g_reindexPath, reindex, options );
        UInt32 acceptedBefore = w.f.service.reindexAccepted;
        ASSERT_TRUE( PumpUntil( w.f.service.reindexAccepted, acceptedBefore + 1, 5000 ) );
        PumpFor( 20 );
        ASSERT_TRUE( 1 == w.f.service.CompleteReindex( 42 ) );
        ASSERT_TRUE( w.f.PumpUntilReady( future, 5000 ) );
        COMCORE::CRpcCallResult reindexResult;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( future, reindexResult ) );
        ASSERT_TRUE( reindexResult.status.IsOk() && reindexResult.wasAccepted && 42 == reindexResult.response.GetUInt64( 1 ) );
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC test: every call kind passed over " << label );
}

/*-------------------------------------------------------------------------*/

/** Signed device tokens: a host that checks them and one device key per user */
class CTokenIssuer
{
    public:

    UInt8 issuerSeed[ 32 ];
    UInt8 issuerPublic[ 32 ];

    CTokenIssuer( void ) { DRN::CDRNCrypto::GenerateSignatureKeyPair( issuerSeed, issuerPublic ); }

    void ConfigureHost( DRN::CDRNNodeSettings& settings ) const
    {
        settings.auth.authenticatorType = "token";
        settings.auth.SetSetting( "issuerPublicKey", DRN::CDRNCrypto::ToHex( issuerPublic, 32 ) );
    }

    DRN::CDRNAuthSettings Credentials( const char* userId, const char* role, const UInt8* devicePublicKey ) const
    {
        DRN::CDRNAuthSettings credentials;
        credentials.authenticatorType = "token";
        credentials.SetSetting( "token", DRN::CDRNTokenAuthenticator::IssueSignedToken( issuerSeed, "rpc-token", userId, role,
                                DRN::CDRNTokenAuthenticator::GetUnixTimeNow() + 600, devicePublicKey ) );
        return credentials;
    }
};

/*-------------------------------------------------------------------------*/

/** A member joins the group's session with credentials of its own; the index of its session */
bool
JoinWithCredentials( CGroup& group, const DRN::CDRNAuthSettings& credentials, UInt32& index, bool pinHostKey = false )
{
    DRN::CDRNNode* node = GUCEF_NEW DRN::CDRNNode();
    DRN::CDRNNodeSettings settings = group.nodeSettings;
    settings.auth = credentials;
    if ( !node->SetSettings( settings ) )
    {
        GUCEF_DELETE node;
        return false;
    }
    group.nodes.push_back( node );
    group.endpoints.push_back( LoopbackEndpoint( group.channelType, NextTestPort(), group.params ) );
    group.sessions.push_back( node->CreateSession() );
    if ( !node->Listen( group.endpoints.back() ) )
        return false;
    index = (UInt32) group.sessions.size() - 1;
    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = group.sessionSettings.sessionName;
    DRN::CDRNEndpoint hostEndpoint = group.endpoints[ 0 ];
    if ( pinHostKey )
        hostEndpoint.GetParams().Set( "key", CORE::CVariant( group.nodes[ 0 ]->GetPublicKeyString() ) );
    if ( !group.sessions.back()->Join( DRN::CDRNEndpointList( hostEndpoint ), joinSettings, &credentials ) )
        return false;
    return group.WaitForMembership();
}

/*-------------------------------------------------------------------------*/

/** Makes blocking calls on a thread of its own: an application thread that is not the protocol thread */
class CBlockingCaller : public MT::CActiveObject
{
    public:

    CBlockingCaller( const COMCORE::CRpcEndpointPtr& endpoint, UInt32 callCount )
        : MT::CActiveObject(), m_endpoint( endpoint ), m_callCount( callCount ), m_succeeded( 0 ), m_done( false ), m_threadId( 0 )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_threadId.Store( MT::GetCurrentThreadID() );
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        if ( m_endpoint->ResolveMethod( g_getPath, ref, status ) )
        {
            for ( UInt32 i=0; i<m_callCount; ++i )
            {
                COMCORE::CRpcMessage request = m_endpoint->NewRequest( ref );
                request.SetUInt64( 1, 100 + i );
                COMCORE::CRpcMessage response;
                if ( m_endpoint->Call( ref, request, response, status ) && status.IsOk() && response.GetUInt64( 1 ) == 100 + i )
                    m_succeeded.Store( m_succeeded.Load() + 1 );
            }
        }
        m_done.Store( true );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsDone( void ) const { return m_done.Load(); }
    UInt32 GetSucceeded( void ) const { return m_succeeded.Load(); }
    TThreadId GetThreadId( void ) const { return m_threadId.Load(); }

    private:

    COMCORE::CRpcEndpointPtr m_endpoint;
    UInt32 m_callCount;
    MT::CTAtomic< UInt32 > m_succeeded;
    MT::CTAtomic< bool > m_done;
    MT::CTAtomic< TThreadId > m_threadId;
};

/*-------------------------------------------------------------------------*/

/** Stops an RPC listener on a thread of its own once it is told to, as an application thread that lets its server go */
class CListenerStopper : public MT::CActiveObject
{
    public:

    explicit CListenerStopper( const CDRNRpcTransportListenerPtr& listener )
        : MT::CActiveObject(), m_listener( listener ), m_go( false ), m_done( false )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( !m_go.Load() )
            MT::PrecisionDelay( 0 );
        m_listener->StopListening();
        m_done.Store( true );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    void Go( void ) { m_go.Store( true ); }
    bool IsDone( void ) const { return m_done.Load(); }

    private:

    CDRNRpcTransportListenerPtr m_listener;
    MT::CTAtomic< bool > m_go;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

/** Keeps the MCP audit records of a server */
class CMcpAuditCapture : public CORE::CObserver
{
    public:

    GUCEF::vector< COM::CMcpAuditRecord > records;

    explicit CMcpAuditCapture( COM::CMcpServer& server ) : CORE::CObserver(), records()
    {
        typedef CORE::CTEventHandlerFunctor< CMcpAuditCapture > TCallback;
        TCallback onAudit( this, &CMcpAuditCapture::OnAudit );
        SubscribeTo( &server, COM::CMcpServer::AuditEvent, onAudit );
    }

    virtual ~CMcpAuditCapture() { UnsubscribeAllFromObserver(); }

    void OnAudit( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        const COM::TMcpAuditEventData* data = static_cast< const COM::TMcpAuditEventData* >( eventData );
        if ( GUCEF_NULL != data )
            records.push_back( data->GetData() );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The conformance suite of gucefCOMCORE_TestApp over drn: a DRN session of two members on the channel type, the
 *  initiator a transport of the member, the acceptor what the host's listener accepts when the member opens the channel
 */
class CDRNConformanceTransports : public CIRpcConformanceTransports ,
                                  public CORE::CObserver
{
    public:

    CDRNConformanceTransports( const char* channel, const char* params, bool ownThread, const char* name )
        : CIRpcConformanceTransports(), CORE::CObserver(), m_group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-conformance", channel, params )
        , m_listener(), m_lock(), m_accepted(), m_name( name ), m_started( false )
    {
        m_group.nodeSettings.ownThread = ownThread;
        m_started = m_group.Start( 2 );
        if ( !m_started )
            return;
        m_listener = CDRNRpcTransportListenerPtr( GUCEF_NEW DRN::CDRNRpcTransportListener( m_group[ 0 ] ) );
        typedef CORE::CTEventHandlerFunctor< CDRNConformanceTransports > TEventCallback;
        TEventCallback accepted( this, &CDRNConformanceTransports::OnAccepted );
        SubscribeTo( m_listener.GetPointerAlways(), COMCORE::CIRpcTransportListener::TransportAcceptedEvent, accepted );
        m_started = m_listener->Listen( CORE::CUri() );
    }

    virtual ~CDRNConformanceTransports()
    {
        UnsubscribeAllFromObserver();
        if ( !m_listener.IsNULL() )
            m_listener->StopListening();
        MT::CScopeMutex lock( m_lock );
        m_accepted.Unlink();
    }

    virtual const char* GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_name.C_String(); }

    virtual bool MakePair( COMCORE::CIRpcTransportPtr& initiator, COMCORE::CIRpcTransportPtr& acceptor ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_started )
            return false;
        CDRNRpcTransportPtr transport( GUCEF_NEW DRN::CDRNRpcTransport( m_group[ 1 ], m_group[ 0 ].GetLocalPeerId() ) );
        if ( !transport->Connect( CORE::CUri() ) )
            return false;
        initiator = transport;
        UInt64 start = MT::PrecisionTickCount();
        while ( MT::PrecisionTickCount() - start < 5 * MT::PrecisionTimerResolution() )
        {
            {
                MT::CScopeMutex lock( m_lock );
                if ( !m_accepted.IsNULL() )
                {
                    acceptor = m_accepted;
                    m_accepted.Unlink();
                    return true;
                }
            }
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::PrecisionDelay( 1 );
        }
        return false;
    }

    private:

    void OnAccepted( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        if ( GUCEF_NULL == eventData )
            return;
        MT::CScopeMutex lock( m_lock );
        m_accepted = static_cast< COMCORE::CRpcTransportAcceptedEventData* >( eventData )->transport;
    }

    CGroup m_group;
    CDRNRpcTransportListenerPtr m_listener;
    MT::CMutex m_lock;
    COMCORE::CIRpcTransportPtr m_accepted;
    CORE::CString m_name;
    bool m_started;
};

/*-------------------------------------------------------------------------*/

/** Rejoins a member whose session closed, with the given credentials; true once it is a member again */
bool
Rejoin( CGroup& group, UInt32 index, const DRN::CDRNAuthSettings* credentials )
{
    DRN::CDRNSessionSettings joinSettings;
    joinSettings.sessionName = group.sessionSettings.sessionName;
    if ( !group.sessions[ index ]->Join( DRN::CDRNEndpointList( group.endpoints[ 0 ] ), joinSettings, credentials ) )
        return false;
    return group.WaitForMembership();
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNRpcTransportTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN RPC TRANSPORT TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.RpcTransport" );

    GUCEF_TESTFW_TESTCASE( "RPC class: messages of DRN_MSGCLASS_RPC reach the session component, a message of an unknown class is counted (B3)" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-class" );
        REQUIRE_TRUE( group.Start( 2 ) );
        CClassProbe probe( DRN::DRN_MSGCLASS_RPC );
        REQUIRE_TRUE( group[ 0 ].RegisterComponent( &probe ) );

        UInt8 payload[ 4 ] = { 1, 2, 3, 4 };
        UInt32 hostPeer = group[ 0 ].GetLocalPeerId();
        ASSERT_TRUE( group[ 1 ].SendToMember( hostPeer, DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RPC, DRN::DRNRpcOpcode( 1, false ), payload, 4 ) );
        ASSERT_TRUE( PumpUntil( probe.received, 1, 3000 ) );

        // a class no node knows: dropped, but counted
        UInt32 before = GetNodeCounter( *group.nodes[ 0 ], "unhandledMessages" );
        DRN::CDRNPeerLinkPtr link = group[ 1 ].GetLinkToMember( hostPeer );
        REQUIRE_TRUE( !link.IsNULL() );
        ASSERT_TRUE( link->Send( DRN::DRN_LANE_RELIABLE_ORDERED, 40, 1, payload, 4 ) );
        link->Flush();
        PumpFor( 200 );
        ASSERT_TRUE( GetNodeCounter( *group.nodes[ 0 ], "unhandledMessages" ) == before + 1 );

        // a known class without a component in the session: counted too
        group[ 0 ].UnregisterComponent( &probe );
        ASSERT_TRUE( group[ 1 ].SendToMember( hostPeer, DRN::DRN_LANE_RELIABLE_ORDERED, DRN::DRN_MSGCLASS_RPC, DRN::DRNRpcOpcode( 1, false ), payload, 4 ) );
        PumpFor( 200 );
        ASSERT_TRUE( GetNodeCounter( *group.nodes[ 0 ], "unhandledMessages" ) == before + 2 );
        ASSERT_TRUE( 1 == probe.received );
        ASSERT_TRUE( group.nodes[ 0 ]->GetUnhandledMessageCount() == before + 2 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Shared secret: the proof is bound to the transcript of its handshake, a proof relayed into another handshake is refused (B18)" )
    try
    {
        DRN::CDRNAuthSettings authSettings;
        authSettings.authenticatorType = DRN::CDRNSharedSecretAuthenticator::MethodName;
        authSettings.SetSetting( "secret", CORE::CVariant( "lan password" ) );
        DRN::CIDRNAuthenticatorPtr authenticator = authSettings.CreateAuthenticator();
        REQUIRE_TRUE( !authenticator.IsNULL() );

        DRN::CDRNAuthContext context;
        context.serviceName = "drn";
        context.initiatorNodeId = "initiator";
        context.acceptorNodeId = "acceptor";
        for ( UInt32 i=0; i<DRN::DRN_CHALLENGE_NONCE_SIZE; ++i )
            context.nonce[ i ] = (UInt8) i;
        for ( UInt32 i=0; i<DRN::DRN_HANDSHAKE_HASH_SIZE; ++i )
            context.transcriptHash[ i ] = (UInt8) ( 0x40 + i );

        CORE::CDynamicBuffer proof;
        REQUIRE_TRUE( authenticator->CreateProof( context, proof ) );
        const UInt8* proofBytes = static_cast< const UInt8* >( proof.GetConstBufferPtr() );
        DRN::CDRNAuthResult result;
        ASSERT_TRUE( authenticator->VerifyProof( context, proofBytes, proof.GetDataSize(), result ) );

        // the same nonce, names and service, another handshake: what a relay in the middle would try
        DRN::CDRNAuthContext relayed = context;
        relayed.transcriptHash[ 0 ] ^= 0x01;
        ASSERT_FALSE( authenticator->VerifyProof( relayed, proofBytes, proof.GetDataSize(), result ) );

        DRN::CDRNAuthContext otherNonce = context;
        otherNonce.nonce[ 3 ] ^= 0x80;
        ASSERT_FALSE( authenticator->VerifyProof( otherNonce, proofBytes, proof.GetDataSize(), result ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Transport: the factories make \"drn\" transports and listeners; a URI names session and member; endpoint calls over it" )
    try
    {
        COMCORE::CIRpcTransportPtr made = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportFactory().Create( "drn" );
        REQUIRE_TRUE( !made.IsNULL() );
        ASSERT_TRUE( "drn" == made->GetTransportType() );
        ASSERT_FALSE( made->SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ) );
        COMCORE::CIRpcTransportListenerPtr madeListener = COMCORE::CComCoreGlobal::Instance()->GetRpcTransportListenerFactory().Create( "drn" );
        REQUIRE_TRUE( !madeListener.IsNULL() );
        ASSERT_FALSE( madeListener->SetPulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() ) );

        // unknown session: refused with a reason
        ASSERT_FALSE( made->Connect( CORE::CUri( "drn://no-such-session/1" ) ) );

        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-uri" );
        REQUIRE_TRUE( w.Start( 2 ) );

        // the listener by URI on the host's session (the registry knows a session by its name, which both members
        // share, so one at a time is registered)
        DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
        REQUIRE_TRUE( registry.Register( w.group[ 0 ] ) );
        REQUIRE_TRUE( w.Listen( 0 ) );
        w.listener->StopListening();
        REQUIRE_TRUE( madeListener->Listen( CORE::CUri( "drn://rpc-uri" ) ) );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC listener URI: " << madeListener->GetListenUri().ToUriInStringForm() );
        ASSERT_TRUE( madeListener->IsListening() && "rpc-uri" == madeListener->GetListenUri().GetAuthorityHost() );
        madeListener->StopListening();
        REQUIRE_TRUE( w.listener->Listen( CORE::CUri() ) );

        // the endpoint connects by URI: the factory makes the drn transport, which refuses the protocol task the
        // endpoint offers (its thread is its node's, known once Connect() found the session) and brings its own
        registry.Unregister( w.group[ 0 ] );
        REQUIRE_TRUE( registry.Register( w.group[ 1 ] ) );
        CORE::CUri remote( "drn://rpc-uri/" + CORE::ToString( w.ServerPeer() ) );
        w.f.client = COMCORE::CRpcEndpoint::Create( w.f.pulseGenerator, w.f.clientSettings );
        w.f.client->AddWantedContract( "gucef.sample.notes", 1 );
        REQUIRE_TRUE( w.f.client->Connect( remote ) );
        COMCORE::CIRpcTransportPtr byUri = w.f.client->GetTransport();
        REQUIRE_TRUE( !byUri.IsNULL() && "drn" == byUri->GetTransportType() );
        ASSERT_TRUE( byUri->IsConnected() && byUri->GetRemoteUri().ToUriInStringForm() == remote.ToUriInStringForm() );
        ASSERT_TRUE( byUri->GetPulseGenerator() == w.group.nodes[ 1 ]->GetPulseGenerator() );
        REQUIRE_TRUE( w.f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) );
        COMCORE::CRpcCallResult result = w.Call( g_getPath, 9 );
        ASSERT_TRUE( result.status.IsOk() && "note 9" == result.response.GetString( 2 ) );
        ASSERT_TRUE( 1 == w.accepted );
        registry.Unregister( w.group[ 1 ] );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Transport: every call kind over drn, CLIENT_SERVER on mem, tcp, udp and lossy udp, PEER_TO_PEER on tcp" )
    try
    {
        const char* channels[] = { "mem", "tcp", "udp", "lossy-udp" };
        const char* params[] = { "", "", "", "?loss=0.1&reorder=0.1" };
        for ( UInt32 c=0; c<4; ++c )
        {
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-kinds", channels[ c ], params[ c ] );
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            ASSERT_TRUE( w.f.client->GetTransport()->GetProperties().isSecure );
            ASSERT_TRUE( "drn" == w.f.client->GetTransport()->GetProperties().profileName );
            RunCallKinds( w, channels[ c ] );
        }

        // peer to peer: two members that are not the sequencer, over their direct link
        CRpcDrnWorld p2p( DRN::DRN_TOPOLOGY_PEER_TO_PEER, "rpc-p2p" );
        REQUIRE_TRUE( p2p.Start( 3 ) );
        REQUIRE_TRUE( p2p.Connect( 2, 1 ) );
        RunCallKinds( p2p, "peer to peer tcp" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Transport: both members call each other in one RPC session" )
    try
    {
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-both" );
        REQUIRE_TRUE( w.Start( 2 ) );
        REQUIRE_TRUE( w.Listen( 0 ) );
        CRpcNotesTestService clientService;
        w.f.client = COMCORE::CRpcEndpoint::Create( w.f.pulseGenerator, w.f.clientSettings );
        w.f.serverRegistry.Register( w.contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        REQUIRE_TRUE( w.f.client->RegisterService( w.contract, "Notes", &clientService ) );
        w.f.server->AddWantedContract( "gucef.sample.notes", 1 );
        w.f.client->AddWantedContract( "gucef.sample.notes", 1 );
        w.clientDrn = CDRNRpcTransportPtr( GUCEF_NEW DRN::CDRNRpcTransport( w.group[ 1 ], w.ServerPeer() ) );
        REQUIRE_TRUE( w.clientDrn->Connect( CORE::CUri() ) );
        COMCORE::CIRpcTransportPtr inner;
        inner = w.clientDrn;
        w.f.clientTransport = CRpcLossyTransport::Wrap( inner, &w.f.clientWire );
        REQUIRE_TRUE( w.f.client->Start( w.f.clientTransport, true ) );
        REQUIRE_TRUE( w.f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) );

        COMCORE::CRpcMessage request = w.f.NewClientRequest( g_getPath );
        request.SetUInt64( 1, 3 );
        CORE::CFutureResult toServer = w.f.client->CallAsync( g_getPath, request );
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        UInt64 waited = 0;
        while ( !w.f.server->ResolveMethod( g_getPath, ref, status ) && waited++ < 3000 )
            PumpFor( 1 );
        REQUIRE_TRUE( w.f.server->ResolveMethod( g_getPath, ref, status ) );
        COMCORE::CRpcMessage back = w.f.server->NewRequest( ref );
        back.SetUInt64( 1, 4 );
        CORE::CFutureResult toClient = w.f.server->CallAsync( ref, back );
        ASSERT_TRUE( w.f.PumpUntilReady( toServer, 5000 ) && w.f.PumpUntilReady( toClient, 5000 ) );
        COMCORE::CRpcCallResult r1, r2;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( toServer, r1 ) && r1.status.IsOk() && "note 3" == r1.response.GetString( 2 ) );
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( toClient, r2 ) && r2.status.IsOk() && "note 4" == r2.response.GetString( 2 ) );
        ASSERT_TRUE( 1 == clientService.calls );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Principal: subject, roles and device key from a signed token, protection attributes; an anonymous link gives an unauthenticated principal (T1)" )
    try
    {
        CTokenIssuer issuer;
        UInt8 deviceSeed[ 32 ];
        UInt8 devicePublic[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( deviceSeed, devicePublic ) );

        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-token" );
        issuer.ConfigureHost( w.group.nodeSettings );
        REQUIRE_TRUE( w.Start( 1 ) );
        UInt32 member = 0;
        REQUIRE_TRUE( JoinWithCredentials( w.group, issuer.Credentials( "alice", "viewer, editor", devicePublic ), member ) );
        REQUIRE_TRUE( w.Connect( member, 0 ) );
        REQUIRE_TRUE( !w.f.server.IsNULL() );
        const COMCORE::CRpcPrincipal& principal = w.f.server->GetPeerPrincipal();
        ASSERT_TRUE( "drn" == principal.transport );
        ASSERT_TRUE( "drn:alice" == principal.subject );
        ASSERT_TRUE( principal.isAuthenticated );
        ASSERT_TRUE( 2 == principal.roles.size() && principal.HasRole( "viewer" ) && principal.HasRole( "editor" ) );
        ASSERT_TRUE( principal.attributes.GetValueAlways( "drn.publicKey" ).AsString() == DRN::CDRNCrypto::ToHex( devicePublic, 32 ) );
        ASSERT_TRUE( principal.attributes.GetValueAlways( "drn.session" ).AsString() == "rpc-token" );
        ASSERT_TRUE( principal.attributes.GetValueAlways( "drn.peerId" ).AsUInt32() == w.group[ member ].GetLocalPeerId() );
        ASSERT_TRUE( principal.attributes.GetValueAlways( "drn.protected" ).AsBool() );
        ASSERT_FALSE( principal.attributes.GetValueAlways( "drn.cipher" ).AsString().IsNULLOrEmpty() );
        ASSERT_TRUE( w.f.server->GetTransport()->GetProperties().isSecure );

        // the member sees the host by its node; the member did not pin the host's key, so the node id is only the
        // host's own word: not authenticated
        const COMCORE::CRpcPrincipal& hostPrincipal = w.f.client->GetPeerPrincipal();
        ASSERT_TRUE( hostPrincipal.subject.HasSubstr( "drn-node:" ) == 0 );
        ASSERT_FALSE( hostPrincipal.isAuthenticated );
        ASSERT_FALSE( hostPrincipal.attributes.GetValueAlways( "drn.publicKey" ).AsString().IsNULLOrEmpty() );
        DRN::CDRNPeerLinkPtr unpinnedLink = w.group[ member ].GetLinkToMember( w.group[ 0 ].GetLocalPeerId() );
        ASSERT_TRUE( !unpinnedLink.IsNULL() && !unpinnedLink->HasPinnedKey() );

        // a member that joined with the host's key pinned sees an authenticated host
        CRpcDrnWorld pinned( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-pinned" );
        issuer.ConfigureHost( pinned.group.nodeSettings );
        REQUIRE_TRUE( pinned.Start( 1 ) );
        UInt32 pinnedMember = 0;
        REQUIRE_TRUE( JoinWithCredentials( pinned.group, issuer.Credentials( "alice", "viewer", devicePublic ), pinnedMember, true ) );
        REQUIRE_TRUE( pinned.Connect( pinnedMember, 0 ) );
        DRN::CDRNPeerLinkPtr pinnedLink = pinned.group[ pinnedMember ].GetLinkToMember( pinned.group[ 0 ].GetLocalPeerId() );
        ASSERT_TRUE( !pinnedLink.IsNULL() && pinnedLink->HasPinnedKey() );
        const COMCORE::CRpcPrincipal& pinnedHost = pinned.f.client->GetPeerPrincipal();
        ASSERT_TRUE( pinnedHost.isAuthenticated && pinnedHost.subject.HasSubstr( "drn-node:" ) == 0 );
        ASSERT_TRUE( pinnedHost.attributes.GetValueAlways( "drn.publicKey" ).AsString() == pinned.group.nodes[ 0 ]->GetPublicKeyString() );

        // anonymous links: the subject is the node, not authenticated
        CRpcDrnWorld anon( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-anon" );
        REQUIRE_TRUE( anon.Start( 2 ) );
        REQUIRE_TRUE( anon.Connect() );
        const COMCORE::CRpcPrincipal& anonymous = anon.f.server->GetPeerPrincipal();
        ASSERT_FALSE( anonymous.isAuthenticated );
        ASSERT_TRUE( anonymous.subject.HasSubstr( "drn-node:" ) == 0 );
        ASSERT_TRUE( anonymous.roles.empty() );

        // a session established with the token principal cannot be resumed over the anonymous one (8.2)
        ASSERT_FALSE( principal.MayResumeAs( anonymous ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Access policy per method: a viewer device is refused a command method with PERMISSION_DENIED and allowed reads (T2)" )
    try
    {
        CTokenIssuer issuer;
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-token" );
        issuer.ConfigureHost( w.group.nodeSettings );
        REQUIRE_TRUE( w.Start( 1 ) );
        UInt32 viewer = 0;
        REQUIRE_TRUE( JoinWithCredentials( w.group, issuer.Credentials( "victor", "viewer", GUCEF_NULL ), viewer ) );

        CORE::CTSharedPtr< COMCORE::CRpcRoleAccessPolicy, MT::CMutex > policy( GUCEF_NEW COMCORE::CRpcRoleAccessPolicy() );
        CORE::CString::StringSet readers;
        readers.insert( "viewer" );
        readers.insert( "editor" );
        CORE::CString::StringSet editors;
        editors.insert( "editor" );
        policy->AddRule( "gucef.sample.notes.Notes/Get", readers );
        policy->AddRule( "gucef.sample.notes.Notes/Save", editors );
        COMCORE::CIRpcAccessPolicyPtr policyPtr;
        policyPtr = policy;
        REQUIRE_TRUE( w.Listen( 0 ) );
        w.f.server->SetAccessPolicy( policyPtr );
        REQUIRE_TRUE( w.ConnectClient( viewer, 0 ) );
        REQUIRE_TRUE( w.f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) );

        COMCORE::CRpcCallResult result = w.Call( g_getPath, 12 );
        ASSERT_TRUE( result.status.IsOk() );
        UInt32 savesBefore = w.f.service.saves;
        COMCORE::CRpcMessage save = w.f.NewClientRequest( g_savePath );
        save.GetMutableMessage( 1 )->SetString( 2, "not allowed" );
        result = w.f.CallAndPump( g_savePath, save );
        ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == result.status.code );
        ASSERT_TRUE( savesBefore == w.f.service.saves );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED == w.f.client->GetState() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Protection: with public items a capture shows a public method's response in clear (MAC packets), no confidential payload and no request; without them nothing in clear (T3)" )
    try
    {
        for ( UInt32 allow=0; allow<2; ++allow )
        {
            CDRNLossyChannel::ClearCapturedPackets();
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-capture", "lossy-tcp", "?capture=1" );
            w.group.nodeSettings.securityPolicy.allowPublicItems = 1 == allow;
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            ASSERT_TRUE( w.f.client->GetTransport()->GetProperties().hasFrameProtectionClasses );

            // a confidential answer, a public stream whose request names what it asks for
            COMCORE::CRpcCallResult result = w.Call( g_getPath, 31 );
            ASSERT_TRUE( result.status.IsOk() && "the secret body" == result.response.GetString( 3 ) );
            COMCORE::CRpcMethodRef ref;
            COMCORE::CRpcStatus status;
            REQUIRE_TRUE( w.f.client->ResolveMethod( g_listTagsPath, ref, status ) );
            COMCORE::CRpcMessage request = w.f.client->NewRequest( ref );
            request.Add( 1, CORE::CVariant( "REQUESTMARKER" ) );
            CStreamCollector collector;
            COMCORE::CIRpcStreamPtr stream = w.f.client->OpenStream( ref, &request );
            REQUIRE_TRUE( !stream.IsNULL() );
            collector.Watch( stream );
            ASSERT_TRUE( RpcTestPumpUntil( collector.ended, 5000 ) );
            ASSERT_TRUE( collector.status.IsOk() && 3 == collector.messages && "PUBLICMARKER1000" == collector.titles[ 0 ] );
            PumpFor( 50 );

            bool publicSeen = false;
            bool secretSeen = false;
            bool requestSeen = false;
            UInt32 macPackets = 0;
            const CDRNLossyChannel::TPacketVector& packets = CDRNLossyChannel::GetCapturedPackets();
            for ( UInt32 p=0; p<packets.size(); ++p )
            {
                publicSeen = publicSeen || Contains( packets[ p ], "PUBLICMARKER" );
                secretSeen = secretSeen || Contains( packets[ p ], "the secret body" ) || Contains( packets[ p ], "note 31" );
                requestSeen = requestSeen || Contains( packets[ p ], "REQUESTMARKER" );
                const UInt8* bytes = static_cast< const UInt8* >( packets[ p ].GetConstBufferPtr() );
                if ( DRN::CDRNPacketProtection::IsMacPacket( bytes, packets[ p ].GetDataSize() ) )
                    ++macPackets;
            }
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC protection capture, public items " << ( 1 == allow ) << ": " << packets.size() << " packets, " << macPackets << " MAC" );
            ASSERT_TRUE( ( 1 == allow ) == publicSeen );
            ASSERT_TRUE( ( 1 == allow ) == ( macPackets > 0 ) );
            ASSERT_FALSE( secretSeen );
            ASSERT_FALSE( requestSeen );
        }
        CDRNLossyChannel::ClearCapturedPackets();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Protection: a confidential method's frame that arrives in a public packet ends the RPC session (ABORT PERMISSION_DENIED) with an anomaly report (T4)" )
    try
    {
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-downgrade" );
        w.group.nodeSettings.securityPolicy.allowPublicItems = true;
        w.downgradeClient = true;
        REQUIRE_TRUE( w.Start( 2 ) );
        REQUIRE_TRUE( w.Connect() );
        COMCORE::CRpcCallResult result = w.Call( g_getPath, 1 );
        ASSERT_TRUE( result.status.IsOk() );

        DRN::CDRNPeerLinkPtr serverLink = w.group[ 0 ].GetLinkToMember( w.group[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !serverLink.IsNULL() );
        UInt64 refusalsBefore = serverLink->GetProtectionDowngradeCount();
        UInt32 callsBefore = w.f.service.calls;
        w.clientDowngrader->downgradeRequests = true;
        result = w.Call( g_getPath, 2, 3000 );
        ASSERT_FALSE( result.status.IsOk() );
        ASSERT_TRUE( w.f.PumpUntilState( w.f.server, COMCORE::RPC_SESSIONSTATE_CLOSED, 3000 ) );
        ASSERT_TRUE( callsBefore == w.f.service.calls );

        // the server endpoint runs over the capturing decorator, which counts the refusal and passes it on to the drn
        // transport: that one reports it through the member link
        ASSERT_TRUE( GUCEF_NULL != w.f.serverWire && 1 == w.f.serverWire->downgradeRefusals );
        ASSERT_TRUE( serverLink->GetProtectionDowngradeCount() == refusalsBefore + 1 );

        // the DRN link itself stays: only the RPC session ended
        ASSERT_TRUE( serverLink->IsOperational() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Sensitive data: a sensitive method's payload is never logged and a default recording holds no RPC frame (T5)" )
    try
    {
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-sensitive" );
        REQUIRE_TRUE( w.Start( 2 ) );
        REQUIRE_TRUE( w.Connect() );
        ASSERT_TRUE( 0 == ( DRN::CDRNSessionRecorder::DefaultClassMask & ( 1u << DRN::DRN_MSGCLASS_RPC ) ) );

        CORE::CString recordingPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( recordingPath, "drn_rpc_recording.bin" );
        CORE::CString fullRecordingPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( fullRecordingPath, "drn_rpc_recording_all.bin" );
        DRN::CDRNSessionRecorder recorder;
        DRN::CDRNSessionRecorder fullRecorder;
        REQUIRE_TRUE( recorder.Start( w.group[ 0 ], recordingPath ) );
        CORE::CString logText;
        {
            CLogCapture capture;
            REQUIRE_TRUE( fullRecorder.Start( w.group[ 1 ], fullRecordingPath, DRN::CDRNSessionRecorder::AllClassesMask ) );
            COMCORE::CRpcMessage save = w.f.NewClientRequest( g_savePath );
            save.GetMutableMessage( 1 )->SetString( 2, "SENSITIVETITLE" );
            save.GetMutableMessage( 1 )->SetString( 3, "SENSITIVEBODY" );
            COMCORE::CRpcCallResult result = w.f.CallAndPump( g_savePath, save );
            ASSERT_TRUE( result.status.IsOk() );
            PumpFor( 20 );
            logText = capture.GetText();
        }
        recorder.Stop();
        fullRecorder.Stop();
        ASSERT_TRUE( logText.HasSubstr( "SENSITIVEBODY" ) < 0 );
        ASSERT_TRUE( logText.HasSubstr( "SENSITIVETITLE" ) < 0 );
        ASSERT_TRUE( logText.HasSubstr( "RPC" ) >= 0 || logText.HasSubstr( "Rpc" ) >= 0 );
        ASSERT_TRUE( logText.HasSubstr( "records the RPC message class" ) >= 0 );

        CORE::CDynamicBuffer recorded;
        CORE::CDynamicBuffer recordedAll;
        ASSERT_TRUE( recorded.LoadContentFromFile( recordingPath ) );
        ASSERT_TRUE( recordedAll.LoadContentFromFile( fullRecordingPath ) );
        ASSERT_FALSE( Contains( recorded, "SENSITIVEBODY" ) );
        ASSERT_TRUE( Contains( recordedAll, "SENSITIVEBODY" ) );
        CORE::DeleteFile( recordingPath );
        CORE::DeleteFile( fullRecordingPath );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Resume: the member's link dies, it rejoins with a new peer id and rebinds; both RPC seq counters continue and pending work completes once (T7)" )
    try
    {
        CTokenIssuer issuer;
        UInt8 deviceSeed[ 32 ];
        UInt8 devicePublic[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( deviceSeed, devicePublic ) );
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-resume" );
        issuer.ConfigureHost( w.group.nodeSettings );
        w.group.nodeSettings.linkTimeoutMs = 2000;
        REQUIRE_TRUE( w.Start( 1 ) );
        UInt32 member = 0;
        DRN::CDRNAuthSettings credentials = issuer.Credentials( "alice", "editor", devicePublic );
        REQUIRE_TRUE( JoinWithCredentials( w.group, credentials, member ) );

        // the server side through a CRpcHost, which keeps the suspended session and routes the resume HELLO
        w.contract = LoadSampleNotesContract( 1, 2 );
        REQUIRE_TRUE( !w.contract.IsNULL() );
        w.f.clientRegistry.Register( w.contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        COMCORE::CRpcHostSettings hostSettings;
        hostSettings.endpointSettings = w.f.serverSettings;
        COMCORE::CRpcHost host( w.f.pulseGenerator, hostSettings );
        REQUIRE_TRUE( host.RegisterService( w.contract, "Notes", &w.f.service ) );
        w.listener = CDRNRpcTransportListenerPtr( GUCEF_NEW DRN::CDRNRpcTransportListener( w.group[ 0 ] ) );
        COMCORE::CIRpcTransportListenerPtr listenerPtr;
        listenerPtr = w.listener;
        REQUIRE_TRUE( host.AddListener( listenerPtr ) );
        REQUIRE_TRUE( w.listener->Listen( CORE::CUri() ) );
        REQUIRE_TRUE( w.ConnectClient( member, 0 ) );
        REQUIRE_TRUE( w.f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) );
        REQUIRE_TRUE( 1 == host.GetEndpointCount() );
        GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
        host.GetEndpoints( endpoints );
        COMCORE::CRpcEndpointPtr server = endpoints[ 0 ];
        COMCORE::CRpcCallResult warm = w.Call( g_getPath, 1 );
        ASSERT_TRUE( warm.status.IsOk() );

        // pending work: an unanswered unary call, accepted work, a held server stream
        w.f.service.holdWatch = true;
        w.f.service.watchCount = 2;
        COMCORE::CRpcMessage held = w.f.NewClientRequest( g_getPath );
        held.SetUInt64( 1, 999 );
        COMCORE::CRpcCallOptions longDeadline;
        longDeadline.deadlineMs = 60000;
        CORE::CFutureResult heldCall = w.f.client->CallAsync( g_getPath, held, longDeadline );
        COMCORE::CRpcMessage reindex = w.f.NewClientRequest( g_reindexPath );
        reindex.SetBool( 1, true );
        COMCORE::CRpcCallOptions acceptedOptions;
        acceptedOptions.metadata.Set( "id-kind", CORE::CVariant( "number" ) );
        CORE::CFutureResult acceptedCall = w.f.client->CallAsync( g_reindexPath, reindex, acceptedOptions );
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( w.f.client->ResolveMethod( g_watchPath, ref, status ) );
        COMCORE::CRpcMessage watch = w.f.client->NewRequest( ref );
        CStreamCollector collector;
        COMCORE::CIRpcStreamPtr stream = w.f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( PumpUntil( collector.messages, 2, 5000 ) );
        ASSERT_TRUE( PumpUntil( w.f.service.reindexAccepted, 1, 5000 ) );
        PumpFor( 100 );
        UInt32 heldCount = w.f.service.GetHeldCount();
        ASSERT_TRUE( 2 == heldCount );
        UInt64 clientSeqBefore = w.f.client->GetNextSendSeq();
        UInt64 serverSeqBefore = server->GetNextSendSeq();
        UInt32 oldPeerId = w.group[ member ].GetLocalPeerId();

        // the link dies without a goodbye
        DRN::CDRNPeerLinkPtr memberLink = w.group[ member ].GetLinkToMember( w.ServerPeer() );
        REQUIRE_TRUE( !memberLink.IsNULL() );
        memberLink->Close( DRN::DRN_LINKREASON_CHANNEL_LOST, "test: link killed" );
        ASSERT_TRUE( w.f.PumpUntilState( w.f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 5000 ) );
        ASSERT_TRUE( w.f.PumpUntilState( server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 5000 ) );

        // rejoin: a new peer id, a new member link, a new RPC channel
        REQUIRE_TRUE( Rejoin( w.group, member, &credentials ) );
        ASSERT_TRUE( oldPeerId != w.group[ member ].GetLocalPeerId() );
        CDRNRpcTransportPtr rebound( GUCEF_NEW DRN::CDRNRpcTransport( w.group[ member ], w.ServerPeer() ) );
        REQUIRE_TRUE( rebound->Connect( CORE::CUri() ) );
        COMCORE::CIRpcTransportPtr reboundPtr;
        reboundPtr = rebound;
        REQUIRE_TRUE( w.f.client->Rebind( reboundPtr ) );
        ASSERT_TRUE( w.f.PumpUntilState( w.f.client, COMCORE::RPC_SESSIONSTATE_ESTABLISHED, 5000 ) );
        ASSERT_TRUE( w.f.PumpUntilState( server, COMCORE::RPC_SESSIONSTATE_ESTABLISHED, 5000 ) );
        ASSERT_TRUE( 1 == host.GetEndpointCount() );

        // the pending work completes, nothing twice
        ASSERT_TRUE( 1 == w.f.service.CompleteReindex( 5 ) );
        w.f.service.FinishHeldStreams( COMCORE::CRpcStatus::Ok );
        ASSERT_TRUE( w.f.PumpUntilReady( acceptedCall, 5000 ) );
        ASSERT_TRUE( w.f.PumpUntilReady( heldCall, 5000 ) );
        ASSERT_TRUE( RpcTestPumpUntil( collector.ended, 5000 ) );
        COMCORE::CRpcCallResult acceptedResult;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( acceptedCall, acceptedResult ) && acceptedResult.status.IsOk() && 5 == acceptedResult.response.GetUInt64( 1 ) );
        COMCORE::CRpcCallResult heldResult;
        ASSERT_TRUE( COMCORE::CRpcEndpoint::TryGetCallResult( heldCall, heldResult ) && heldResult.status.IsOk() );
        ASSERT_TRUE( collector.status.IsOk() && 2 == collector.messages );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC resume: client next seq " << clientSeqBefore << " -> " << w.f.client->GetNextSendSeq() <<
                   ", server next seq " << serverSeqBefore << " -> " << server->GetNextSendSeq() );
        COMCORE::CRpcCallResult after = w.Call( g_getPath, 2 );
        ASSERT_TRUE( after.status.IsOk() );
        ASSERT_TRUE( w.f.client->GetNextSendSeq() > clientSeqBefore && server->GetNextSendSeq() > serverSeqBefore );
        CORE::CDataNode telemetry;
        w.f.client->GetTelemetry( telemetry );
        ASSERT_TRUE( 1 == telemetry.GetAttributeValueOrChildValueByName( "resumes" ).AsUInt32( 0, true ) );
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Resume: refused for another device key under the same user id (T8)" )
    try
    {
        CTokenIssuer issuer;
        UInt8 seedA[ 32 ], publicA[ 32 ], seedB[ 32 ], publicB[ 32 ];
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( seedA, publicA ) );
        REQUIRE_TRUE( DRN::CDRNCrypto::GenerateSignatureKeyPair( seedB, publicB ) );
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-token" );
        issuer.ConfigureHost( w.group.nodeSettings );
        REQUIRE_TRUE( w.Start( 1 ) );
        UInt32 member = 0;
        REQUIRE_TRUE( JoinWithCredentials( w.group, issuer.Credentials( "alice", "editor", publicA ), member ) );

        w.contract = LoadSampleNotesContract( 1, 2 );
        REQUIRE_TRUE( !w.contract.IsNULL() );
        w.f.clientRegistry.Register( w.contract, COMCORE::CRpcContractRegistry::CONTRACTSOURCE_FILE );
        COMCORE::CRpcHostSettings hostSettings;
        hostSettings.endpointSettings = w.f.serverSettings;
        COMCORE::CRpcHost host( w.f.pulseGenerator, hostSettings );
        REQUIRE_TRUE( host.RegisterService( w.contract, "Notes", &w.f.service ) );
        w.listener = CDRNRpcTransportListenerPtr( GUCEF_NEW DRN::CDRNRpcTransportListener( w.group[ 0 ] ) );
        COMCORE::CIRpcTransportListenerPtr listenerPtr;
        listenerPtr = w.listener;
        REQUIRE_TRUE( host.AddListener( listenerPtr ) && w.listener->Listen( CORE::CUri() ) );
        REQUIRE_TRUE( w.ConnectClient( member, 0 ) );
        REQUIRE_TRUE( w.f.PumpUntilBound( "gucef.sample.notes", 1, 5000 ) );
        GUCEF::vector< COMCORE::CRpcEndpointPtr > endpoints;
        host.GetEndpoints( endpoints );
        REQUIRE_TRUE( 1 == endpoints.size() );
        COMCORE::CRpcEndpointPtr server = endpoints[ 0 ];

        DRN::CDRNPeerLinkPtr memberLink = w.group[ member ].GetLinkToMember( w.ServerPeer() );
        REQUIRE_TRUE( !memberLink.IsNULL() );
        memberLink->Close( DRN::DRN_LINKREASON_CHANNEL_LOST, "test: link killed" );
        ASSERT_TRUE( w.f.PumpUntilState( server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 5000 ) );
        ASSERT_TRUE( w.f.PumpUntilState( w.f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 5000 ) );

        // the same user on another device
        DRN::CDRNAuthSettings otherDevice = issuer.Credentials( "alice", "editor", publicB );
        REQUIRE_TRUE( Rejoin( w.group, member, &otherDevice ) );
        CDRNRpcTransportPtr rebound( GUCEF_NEW DRN::CDRNRpcTransport( w.group[ member ], w.ServerPeer() ) );
        REQUIRE_TRUE( rebound->Connect( CORE::CUri() ) );
        COMCORE::CIRpcTransportPtr reboundPtr;
        reboundPtr = rebound;
        REQUIRE_TRUE( w.f.client->Rebind( reboundPtr ) );
        PumpFor( 500 );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_ESTABLISHED != w.f.client->GetState() );
        ASSERT_TRUE( COMCORE::RPC_SESSIONSTATE_SUSPENDED == server->GetState() );
        CORE::CDataNode telemetry;
        server->GetTelemetry( telemetry );
        ASSERT_TRUE( 1 == telemetry.GetAttributeValueOrChildValueByName( "refusedResumes" ).AsUInt32( 0, true ) );
        host.Stop();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Frame sizes: udp frames stay within the link's unfragmented payload and DRN fragments none of them; tcp frames are 16 KB (T9)" )
    try
    {
        {
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-udp-size", "udp" );
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            DRN::CDRNPeerLinkPtr link = w.group[ 1 ].GetLinkToMember( w.ServerPeer() );
            DRN::CDRNPeerLinkPtr serverLink = w.group[ 0 ].GetLinkToMember( w.group[ 1 ].GetLocalPeerId() );
            REQUIRE_TRUE( !link.IsNULL() && !serverLink.IsNULL() );
            UInt32 maxFrame = w.f.client->GetTransport()->GetProperties().maxFrameSize;
            ASSERT_TRUE( maxFrame == link->GetMaxUnfragmentedPayload() && maxFrame < 16384 );
            UInt64 fragmentedBefore = link->GetStats().fragmentedMessages + serverLink->GetStats().fragmentedMessages;
            RunCallKinds( w, "udp, frame size check" );
            ASSERT_TRUE( fragmentedBefore == link->GetStats().fragmentedMessages + serverLink->GetStats().fragmentedMessages );
            GUCEF::vector< CRpcLossyTransport::SFrame > frames;
            w.f.clientWire->GetSent( frames );
            UInt32 largest = 0;
            for ( UInt32 i=0; i<frames.size(); ++i )
                largest = frames[ i ].bytes.GetDataSize() > largest ? frames[ i ].bytes.GetDataSize() : largest;
            ASSERT_TRUE( largest <= maxFrame && largest > maxFrame / 2 );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC frame sizes over udp: maximum " << maxFrame << ", largest sent " << largest );
        }
        {
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-tcp-size", "tcp" );
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            ASSERT_TRUE( 16384 == w.f.client->GetTransport()->GetProperties().maxFrameSize );
            DRN::CDRNPeerLinkPtr link = w.group[ 1 ].GetLinkToMember( w.ServerPeer() );
            REQUIRE_TRUE( !link.IsNULL() );
            ASSERT_TRUE( 16777154 == link->GetMaxUnfragmentedPayload() );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Old peer: a member whose node announces protocol 2.0 gives Connect() UNAVAILABLE at once (T10)" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-old" );
        group.nodeSettings.protocolMinor = 0;
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNPeerLinkPtr link = group[ 1 ].GetLinkToMember( group[ 0 ].GetLocalPeerId() );
        REQUIRE_TRUE( !link.IsNULL() && 0 == link->GetRemoteProtocolMinor() );
        DRN::CDRNRpcTransport transport( group[ 1 ], group[ 0 ].GetLocalPeerId() );
        UInt64 start = MT::PrecisionTickCount();
        ASSERT_FALSE( transport.Connect( CORE::CUri() ) );
        ASSERT_TRUE( ElapsedMicros( start ) < 100000 );
        ASSERT_TRUE( COMCORE::RPC_STATUS_UNAVAILABLE == transport.GetConnectStatus().code );
        ASSERT_TRUE( transport.GetConnectStatus().message.HasSubstr( "has no RPC" ) >= 0 );

        // a current node sees 2.1
        CGroup current( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-new" );
        REQUIRE_TRUE( current.Start( 2 ) );
        DRN::CDRNPeerLinkPtr currentLink = current[ 1 ].GetLinkToMember( current[ 0 ].GetLocalPeerId() );
        DRN::CDRNPeerLinkPtr hostLink = current[ 0 ].GetLinkToMember( current[ 1 ].GetLocalPeerId() );
        REQUIRE_TRUE( !currentLink.IsNULL() && !hostLink.IsNULL() );
        ASSERT_TRUE( DRN::DRN_PROTOCOL_VERSION_MINOR == currentLink->GetRemoteProtocolMinor() && DRN::DRN_PROTOCOL_VERSION_MINOR == hostLink->GetRemoteProtocolMinor() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Direct links only: in CLIENT_SERVER a client's transport toward another client fails Connect() (T11); an unprotected link is refused" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-direct" );
        REQUIRE_TRUE( group.Start( 3 ) );
        DRN::CDRNRpcTransport transport( group[ 1 ], group[ 2 ].GetLocalPeerId() );
        ASSERT_FALSE( transport.Connect( CORE::CUri() ) );
        ASSERT_TRUE( COMCORE::RPC_STATUS_UNAVAILABLE == transport.GetConnectStatus().code );
        DRN::CDRNRpcTransport self( group[ 1 ], group[ 1 ].GetLocalPeerId() );
        ASSERT_FALSE( self.Connect( CORE::CUri() ) );
        DRN::CDRNRpcTransport toHost( group[ 1 ], group[ 0 ].GetLocalPeerId() );
        ASSERT_TRUE( toHost.Connect( CORE::CUri() ) );

        CGroup plain( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-plain" );
        plain.nodeSettings.securityPolicy.requireEncryption = false;
        plain.nodeSettings.securityPolicy.ciphers.clear();
        REQUIRE_TRUE( plain.Start( 2 ) );
        DRN::CDRNPeerLinkPtr plainLink = plain[ 1 ].GetLinkToMember( plain[ 0 ].GetLocalPeerId() );
        REQUIRE_TRUE( !plainLink.IsNULL() );
        if ( !plainLink->IsProtected() )
        {
            DRN::CDRNRpcTransport refused( plain[ 1 ], plain[ 0 ].GetLocalPeerId() );
            ASSERT_FALSE( refused.Connect( CORE::CUri() ) );
            ASSERT_TRUE( COMCORE::RPC_STATUS_PERMISSION_DENIED == refused.GetConnectStatus().code );
            DRN::CDRNRpcTransportSettings allowPlain;
            allowPlain.requireProtectedLink = false;
            DRN::CDRNRpcTransport allowed( plain[ 1 ], plain[ 0 ].GetLocalPeerId(), allowPlain );
            ASSERT_TRUE( allowed.Connect( CORE::CUri() ) );
            ASSERT_FALSE( allowed.GetProperties().isSecure || allowed.GetProperties().hasFrameProtectionClasses );
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC test: the plain group got a protected link, the unprotected case is not checked" );
            ASSERT_TRUE( false );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Lifecycle: Close() is followed by DisconnectedEvent on the node thread, the peer's transport disconnects; member leave and session close suspend the RPC session" )
    try
    {
        {
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-close" );
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            CTransportEvents clientEvents;
            CTransportEvents serverEvents;
            clientEvents.Watch( *w.clientDrn );
            REQUIRE_TRUE( !w.lastAccepted.IsNULL() );
            serverEvents.Watch( *w.lastAccepted );
            w.clientDrn->Close();
            ASSERT_TRUE( 0 == clientEvents.disconnected );
            ASSERT_FALSE( w.clientDrn->IsConnected() );
            ASSERT_FALSE( w.clientDrn->SendFrame( "x", 1, COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
            ASSERT_TRUE( PumpUntil( clientEvents.disconnected, 1, 3000 ) );
            ASSERT_TRUE( PumpUntil( serverEvents.disconnected, 1, 3000 ) );
            ASSERT_TRUE( clientEvents.eventThread == MT::GetCurrentThreadID() );
            ASSERT_TRUE( COMCORE::RPC_STATUS_UNAVAILABLE == serverEvents.lastStatus.code );
            PumpFor( 50 );
            ASSERT_TRUE( 1 == clientEvents.disconnected && 1 == serverEvents.disconnected );
        }
        {
            // the member leaves: the host's endpoint suspends; the member's session closed: its endpoint suspends
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-leave" );
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            w.group[ 1 ].Leave();
            ASSERT_TRUE( w.f.PumpUntilState( w.f.client, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 3000 ) );
            ASSERT_TRUE( w.f.PumpUntilState( w.f.server, COMCORE::RPC_SESSIONSTATE_SUSPENDED, 3000 ) );
        }
        {
            // the session object goes while the transport lives on: it is disconnected and stays harmless
            CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-destroy" );
            REQUIRE_TRUE( w.Start( 2 ) );
            REQUIRE_TRUE( w.Connect() );
            CTransportEvents events;
            events.Watch( *w.clientDrn );
            w.group.nodes[ 1 ]->DestroySession( &w.group[ 1 ] );
            w.group.sessions[ 1 ] = w.group.nodes[ 1 ]->CreateSession();
            ASSERT_TRUE( PumpUntil( events.disconnected, 1, 3000 ) );
            ASSERT_FALSE( w.clientDrn->IsConnected() );
            ASSERT_FALSE( w.clientDrn->SendFrame( "x", 1, COMCORE::RPC_PROTECTION_CONFIDENTIAL ) );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Listener: maxSessionsPerMember refuses the channel beyond it; a channel nobody takes is closed for the member" )
    try
    {
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-limits" );
        w.transportSettings.maxSessionsPerMember = 1;
        REQUIRE_TRUE( w.Start( 2 ) );
        REQUIRE_TRUE( w.Connect() );

        // a second RPC session of the same member
        CDRNRpcTransportPtr second( GUCEF_NEW DRN::CDRNRpcTransport( w.group[ 1 ], w.ServerPeer() ) );
        REQUIRE_TRUE( second->Connect( CORE::CUri() ) );
        ASSERT_TRUE( second->GetChannel() != w.clientDrn->GetChannel() );
        CTransportEvents events;
        events.Watch( *second );
        COMCORE::CRpcEndpointPtr extra = COMCORE::CRpcEndpoint::Create( w.f.pulseGenerator, w.f.clientSettings );
        COMCORE::CIRpcTransportPtr secondPtr;
        secondPtr = second;
        REQUIRE_TRUE( extra->Start( secondPtr, true ) );
        ASSERT_TRUE( PumpUntil( events.disconnected, 1, 3000 ) );
        ASSERT_TRUE( 1 == w.listener->GetRefusedCount() );
        ASSERT_TRUE( 1 == w.accepted );
        extra->Close( false );

        // nobody keeps the accepted transport
        w.startServerOnAccept = false;
        w.transportSettings.maxSessionsPerMember = 0;
        w.listener->SetSettings( w.transportSettings );
        CDRNRpcTransportPtr third( GUCEF_NEW DRN::CDRNRpcTransport( w.group[ 1 ], w.ServerPeer() ) );
        REQUIRE_TRUE( third->Connect( CORE::CUri() ) );
        CTransportEvents thirdEvents;
        thirdEvents.Watch( *third );
        COMCORE::CRpcEndpointPtr orphan = COMCORE::CRpcEndpoint::Create( w.f.pulseGenerator, w.f.clientSettings );
        COMCORE::CIRpcTransportPtr thirdPtr;
        thirdPtr = third;
        REQUIRE_TRUE( orphan->Start( thirdPtr, true ) );
        ASSERT_TRUE( PumpUntil( w.accepted, 2, 3000 ) );
        w.lastAccepted.Unlink();
        ASSERT_TRUE( PumpUntil( thirdEvents.disconnected, 1, 3000 ) );
        orphan->Close( false );

        // the first session is unaffected
        COMCORE::CRpcCallResult result = w.Call( g_getPath, 6 );
        ASSERT_TRUE( result.status.IsOk() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "MCP over drn: tools/list and a tool call through CMcpRpcRelay over gucef.mcp.Session/Exchange; the MCP audit names the device (T12)" )
    try
    {
        if ( !McpTestJsonAvailable() )
            GUCEF_TESTFW_SKIP( "no JSON codec plugin" );

        CTokenIssuer issuer;
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-token" );
        issuer.ConfigureHost( w.group.nodeSettings );
        REQUIRE_TRUE( w.Start( 1 ) );
        UInt32 member = 0;
        REQUIRE_TRUE( JoinWithCredentials( w.group, issuer.Credentials( "operator", "mcp", GUCEF_NULL ), member ) );

        // the MCP server on the pulse generator the endpoints notify on (the Session service runs there)
        COM::CMcpServer mcpServer( w.f.pulseGenerator );
        CORE::CString error;
        mcpServer.GetToolRegistry().Register( COM::CIMcpToolPtr( GUCEF_NEW CMcpTestTool( "echo", CMcpTestTool::ECHO ) ), error );
        CMcpAuditCapture audit( mcpServer );
        COMCORE::CRpcHostSettings hostSettings;
        hostSettings.endpointSettings = w.f.serverSettings;
        COMCORE::CRpcHost host( w.f.pulseGenerator, hostSettings );
        COM::CIMcpTransportPtr transportPtr = COM::CComGlobal::Instance()->GetMcpTransportFactory().Create( "rpc" );
        REQUIRE_TRUE( !transportPtr.IsNULL() );
        COM::CMcpRpcTransport* mcpTransport = static_cast< COM::CMcpRpcTransport* >( transportPtr.GetPointerAlways() );
        mcpTransport->SetServiceSet( host.GetServiceSet() );
        mcpServer.AddTransport( transportPtr );
        REQUIRE_TRUE( mcpServer.Start() );

        w.listener = CDRNRpcTransportListenerPtr( GUCEF_NEW DRN::CDRNRpcTransportListener( w.group[ 0 ] ) );
        COMCORE::CIRpcTransportListenerPtr listenerPtr;
        listenerPtr = w.listener;
        REQUIRE_TRUE( host.AddListener( listenerPtr ) && w.listener->Listen( CORE::CUri() ) );

        w.f.client = COMCORE::CRpcEndpoint::Create( w.f.pulseGenerator, w.f.clientSettings );
        w.f.client->AddWantedContract( "gucef.mcp", 1 );
        w.clientDrn = CDRNRpcTransportPtr( GUCEF_NEW DRN::CDRNRpcTransport( w.group[ member ], w.ServerPeer() ) );
        REQUIRE_TRUE( w.clientDrn->Connect( CORE::CUri() ) );
        w.f.clientTransport = w.clientDrn;
        REQUIRE_TRUE( w.f.client->Start( w.f.clientTransport, true ) );
        REQUIRE_TRUE( w.f.PumpUntilBound( "gucef.mcp", 1, 5000 ) );

        COM::CMcpRpcRelay relay;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( relay.Open( w.f.client, status ) );
        ASSERT_TRUE( relay.SendMessage( McpTestRequest( "1", "tools/list" ) ) );
        CORE::CString answer;
        ASSERT_TRUE( relay.ReadMessage( answer, status, 5000 ) );
        ASSERT_TRUE( answer.HasSubstr( "\"echo\"" ) >= 0 );
        ASSERT_TRUE( relay.SendMessage( McpTestRequest( "2", "tools/call", "\"name\":\"echo\",\"arguments\":{\"text\":\"over drn\"}" ) ) );
        ASSERT_TRUE( relay.ReadMessage( answer, status, 5000 ) );
        ASSERT_TRUE( answer.HasSubstr( "over drn" ) >= 0 );
        ASSERT_TRUE( 1 == mcpTransport->GetChannelCount() );
        PumpFor( 20 );

        bool sawDevice = false;
        for ( UInt32 i=0; i<audit.records.size(); ++i )
        {
            if ( "tools/call" == audit.records[ i ].method )
                sawDevice = "drn:operator" == audit.records[ i ].subject && audit.records[ i ].isAuthenticated && "rpc" == audit.records[ i ].transport;
        }
        ASSERT_TRUE( sawDevice );

        relay.Close();
        UInt32 waited = 0;
        while ( 0 != mcpTransport->GetChannelCount() && waited++ < 2000 )
            PumpFor( 1 );
        ASSERT_TRUE( 0 == mcpTransport->GetChannelCount() );
        w.f.client->Close( false );
        PumpFor( 20 );
        host.Stop();
        mcpServer.Stop( false );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    // the shared conformance suite of the RPC design over drn. Last: it opens a suite of its own, and the test
    // framework does not nest suites, so cases of this suite after it would not be recorded
    {
        const char* channels[] = { "mem", "tcp", "udp", "lossy-udp", "tcp" };
        const char* params[] = { "", "", "", "?loss=0.05&reorder=0.05", "" };
        const bool ownThreads[] = { false, false, false, false, true };
        const char* names[] = { "drn over mem", "drn over tcp", "drn over udp", "drn over lossy udp", "drn over tcp, nodes on threads of their own" };
        for ( UInt32 c=0; c<5; ++c )
        {
            CDRNConformanceTransports transports( channels[ c ], params[ c ], ownThreads[ c ], names[ c ] );
            PerformRpcConformanceTests( transports );
        }
    }

    PerformDRNRpcTransportPerformanceTests();
    PerformDRNRpcTransportThreadingTests();
}

/*-------------------------------------------------------------------------*/

/** T13 over nodes with threads of their own, handlers in the given execution mode */
void
PerformDRNRpcTransportPerformanceTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN RPC TRANSPORT PERFORMANCE TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.RpcTransportPerformance" );

    GUCEF_TESTFW_TESTCASE( "Latency: a unary call over drn beside a raw DRN application message round trip on the same encrypted tcp link (T6)" )
    try
    {
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-latency" );
        w.captureFrames = false;
        REQUIRE_TRUE( w.Start( 2 ) );
        REQUIRE_TRUE( w.Connect() );
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( w.f.client->ResolveMethod( g_getPath, ref, status ) );

        // the raw round trip: the host echoes application messages
        class CEcho : public CORE::CObserver
        {
            public:
            DRN::CDRNSession* host;
            bool answered;
            CEcho( void ) : CORE::CObserver(), host( GUCEF_NULL ), answered( false ) {}
            virtual ~CEcho() { UnsubscribeAllFromObserver(); }
            void OnHost( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* eventData )
            {
                // sent at once, as the RPC transport sends an answer
                const DRN::CDRNAppMessage& message = static_cast< DRN::CDRNAppMessageEventData* >( eventData )->GetData();
                host->SendAppMessage( message.fromPeer, DRN::DRN_LANE_RELIABLE_ORDERED, message.payload );
                DRN::CDRNPeerLinkPtr link = host->GetLinkToMember( message.fromPeer );
                if ( !link.IsNULL() )
                    link->Flush();
            }
            void OnMember( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* ) { answered = true; }
        };
        CEcho echo;
        echo.host = &w.group[ 0 ];
        typedef CORE::CTEventHandlerFunctor< CEcho > TEchoCallback;
        TEchoCallback onHost( &echo, &CEcho::OnHost );
        TEchoCallback onMember( &echo, &CEcho::OnMember );
        echo.SubscribeTo( &w.group[ 0 ], DRN::CDRNSession::AppMessageEvent, onHost );
        echo.SubscribeTo( &w.group[ 1 ], DRN::CDRNSession::AppMessageEvent, onMember );

        DRN::CDRNPeerLinkPtr memberLink = w.group[ 1 ].GetLinkToMember( w.ServerPeer() );
        REQUIRE_TRUE( !memberLink.IsNULL() );
        // GUCEF_TEST_RPC_LATENCY_ROUNDS gives more rounds, for profiling
        UInt32 rounds = 300;
        const CORE::CString roundsText = CORE::GetEnv( "GUCEF_TEST_RPC_LATENCY_ROUNDS" );
        if ( !roundsText.IsNULLOrEmpty() )
            rounds = CORE::StringToUInt32( roundsText, rounds );
        GUCEF::vector< UInt64 > raw;
        GUCEF::vector< UInt64 > rpc;
        UInt8 payload[ 24 ];
        memset( payload, 7, sizeof( payload ) );
        COMCORE::CRpcMessage request = w.f.client->NewRequest( ref );
        request.SetUInt64( 1, 8 );
        for ( UInt32 i=0; i<rounds + 20; ++i )
        {
            echo.answered = false;
            UInt64 start = MT::PrecisionTickCount();
            REQUIRE_TRUE( w.group[ 1 ].SendAppMessage( w.ServerPeer(), DRN::DRN_LANE_RELIABLE_ORDERED, payload, sizeof( payload ) ) );
            memberLink->Flush();
            REQUIRE_TRUE( SpinUntilFlag( echo.answered, 3000 ) );
            UInt64 rawUs = ElapsedMicros( start );

            start = MT::PrecisionTickCount();
            CORE::CFutureResult future = w.f.client->CallAsync( ref, request );
            CORE::PulseGeneratorPtr pulseGenerator = w.f.pulseGenerator;
            while ( !future.IsReady() && ElapsedMicros( start ) < 3000000 )
                pulseGenerator->ForceDirectPulse();
            UInt64 rpcUs = ElapsedMicros( start );
            REQUIRE_TRUE( future.IsReady() );
            if ( i >= 20 )
            {
                raw.push_back( rawUs );
                rpc.push_back( rpcUs );
            }
        }
        UInt64 rawMedian = Median( raw );
        UInt64 rpcMedian = Median( rpc );
        GUCEF_LOG( CORE::LOGLEVEL_CRITICAL, "DRN RPC latency (" << rounds << " rounds, encrypted tcp loopback, one thread): raw application message round trip median " <<
                   rawMedian << " us, unary RPC call median " << rpcMedian << " us, difference " << ( (Int64) rpcMedian - (Int64) rawMedian ) << " us" );
        ASSERT_TRUE( rpcMedian > 0 && rawMedian > 0 );
        ASSERT_TRUE( rpcMedian < rawMedian * 10 + 5000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Performance: unary latency and pipelined throughput over drn (tcp), RpcPerfUnary of the conformance suite" )
    try
    {
        const bool ownThreads[] = { false, true };
        const char* names[] = { "drn over tcp", "drn over tcp, nodes on threads of their own" };
        for ( UInt32 c=0; c<2; ++c )
        {
            CDRNConformanceTransports transports( "tcp", "", ownThreads[ c ], names[ c ] );
            SRpcUnaryFigures figures;
            ASSERT_TRUE( RpcPerfUnary( transports, RpcPerfCalls( 2000 ), RpcPerfCalls( 10000 ), figures ) );
            ASSERT_TRUE( figures.medianUs > 0.0 && figures.callsPerSecond > 0.0 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

}

/*-------------------------------------------------------------------------*/

static void
RunThreadingCase( COMCORE::TRpcExecutionMode mode, bool withStream )
{
        // the watch subscribes on the test thread while the node threads run: the transport's notifier has a lock
        CDisconnectWatch events;
        CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-threads" );
        w.clientWatch = &events;
        w.group.nodeSettings.ownThread = true;
        w.mode = mode;
        REQUIRE_TRUE( w.Start( 2 ) );
        REQUIRE_TRUE( w.Connect() );
        TThreadId testThread = MT::GetCurrentThreadID();
        TThreadId serverNodeThread = w.group.nodes[ 0 ]->GetPulseGenerator()->GetPulseDriverThreadId();
        TThreadId clientNodeThread = w.group.nodes[ 1 ]->GetPulseGenerator()->GetPulseDriverThreadId();
        ASSERT_TRUE( 0 != serverNodeThread && testThread != serverNodeThread && testThread != clientNodeThread );
        ASSERT_TRUE( w.f.client->GetTransport()->GetPulseGenerator() == w.group.nodes[ 1 ]->GetPulseGenerator() );

        // asynchronous calls from this thread, which is not the protocol thread
        for ( UInt32 i=0; i<20; ++i )
        {
            COMCORE::CRpcCallResult result = w.Call( g_getPath, i + 1 );
            ASSERT_TRUE( result.status.IsOk() && i + 1 == result.response.GetUInt64( 1 ) );
        }
        TThreadId handlerThread = w.f.service.lastHandlerThread;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRN RPC threads: test " << testThread << ", server node " << serverNodeThread << ", client node " << clientNodeThread << ", handler " << handlerThread );
        ASSERT_TRUE( ( COMCORE::RPC_EXECUTION_CALLING_THREAD == w.mode ) ? ( serverNodeThread == handlerThread ) : ( testThread == handlerThread ) );

        // blocking calls from this thread: it waits on the future, the node thread does the work (a handler on
        // the notification thread needs this thread, so it is called from another thread only)
        if ( COMCORE::RPC_EXECUTION_CALLING_THREAD == w.mode )
        {
            for ( UInt32 i=0; i<10; ++i )
            {
                COMCORE::CRpcMessage request = w.f.NewClientRequest( g_getPath );
                request.SetUInt64( 1, 50 + i );
                COMCORE::CRpcMessage response;
                COMCORE::CRpcStatus status;
                ASSERT_TRUE( w.f.client->Call( g_getPath, request, response, status ) && status.IsOk() && 50 + i == response.GetUInt64( 1 ) );
            }
        }

        // blocking calls from two other threads while this one keeps the notification thread going
        CBlockingCaller callerA( w.f.client, 25 );
        CBlockingCaller callerB( w.f.client, 25 );
        REQUIRE_TRUE( callerA.Activate( GUCEF_NULL, 0, 0 ) && callerB.Activate( GUCEF_NULL, 0, 0 ) );
        UInt64 start = MT::PrecisionTickCount();
        while ( ( !callerA.IsDone() || !callerB.IsDone() ) && ElapsedMicros( start ) < 30000000 )
            PumpFor( 1 );
        ASSERT_TRUE( callerA.IsDone() && callerB.IsDone() );
        ASSERT_TRUE( 25 == callerA.GetSucceeded() && 25 == callerB.GetSucceeded() );
        UInt32 waited = 0;
        while ( ( callerA.IsActive() || callerB.IsActive() ) && waited++ < 5000 )
            MT::PrecisionDelay( 1 );

        // a server stream, its events on the notification thread
        if ( withStream )
        {
        CStreamCollector collector;
        COMCORE::CRpcMethodRef ref;
        COMCORE::CRpcStatus status;
        REQUIRE_TRUE( w.f.client->ResolveMethod( g_watchPath, ref, status ) );
        COMCORE::CRpcMessage watch = w.f.client->NewRequest( ref );
        COMCORE::CIRpcStreamPtr stream = w.f.client->OpenStream( ref, &watch );
        REQUIRE_TRUE( !stream.IsNULL() );
        collector.Watch( stream );
        ASSERT_TRUE( RpcTestPumpUntil( collector.ended, 5000 ) );
        ASSERT_TRUE( collector.status.IsOk() && 3 == collector.messages );
        }

        // closing from this thread: the disconnect comes on the node thread
        w.f.client->Close( true );
        ASSERT_TRUE( w.f.PumpUntilState( w.f.client, COMCORE::RPC_SESSIONSTATE_CLOSED, 5000 ) );
        start = MT::PrecisionTickCount();
        while ( 0 == events.disconnected.Load() && ElapsedMicros( start ) < 3000000 )
            PumpFor( 1 );
        ASSERT_TRUE( 1 == events.disconnected.Load() && clientNodeThread == events.eventThread.Load() );
        w.CloseEndpoints();
}

/*-------------------------------------------------------------------------*/

/**
 *  Nodes on threads of their own with calls in flight from another thread: the application's thread destroys the
 *  session of the server while a third thread stops the RPC listener of that session. The RPC component of the
 *  session goes with it (OnSessionDestroyed()) and the listener lets go of the component (StopListening()): both
 *  have to happen under the node lock, one after the other
 */
static bool
RunSessionDestroyRound( UInt32 round )
{
    CRpcDrnWorld w( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "rpc-destroy-threads" );
    w.group.nodeSettings.ownThread = true;
    if ( !w.Start( 2 ) || !w.Connect() )
        return false;

    CBlockingCaller caller( w.f.client, 1000 );
    CListenerStopper stopper( w.listener );
    if ( !caller.Activate( GUCEF_NULL, 0, 0 ) || !stopper.Activate( GUCEF_NULL, 0, 0 ) )
        return false;
    PumpFor( 2 + round % 5 );

    stopper.Go();
    w.group.nodes[ 0 ]->DestroySession( &w.group[ 0 ] );
    w.group.sessions[ 0 ] = w.group.nodes[ 0 ]->CreateSession();

    // the calls in flight end once the client's endpoint is closed
    UInt64 start = MT::PrecisionTickCount();
    while ( !stopper.IsDone() && ElapsedMicros( start ) < 10000000 )
        PumpFor( 1 );
    PumpFor( 20 );
    w.CloseEndpoints();
    start = MT::PrecisionTickCount();
    while ( !caller.IsDone() && ElapsedMicros( start ) < 30000000 )
        PumpFor( 1 );
    UInt32 waited = 0;
    while ( ( caller.IsActive() || stopper.IsActive() ) && waited++ < 5000 )
        MT::PrecisionDelay( 1 );
    return stopper.IsDone() && caller.IsDone() && !w.listener->IsListening();
}

/*-------------------------------------------------------------------------*/

void
PerformDRNRpcTransportThreadingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN RPC TRANSPORT THREADING TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.RpcTransportThreads" );

    GUCEF_TESTFW_TESTCASE( "Threading: nodes on threads of their own; async and blocking calls from the application thread and from other threads, a server stream; handlers on the node thread (T13)" )
    try
    {
        RunThreadingCase( COMCORE::RPC_EXECUTION_CALLING_THREAD, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    GUCEF_TESTFW_TESTCASE( "Threading: a session destroyed on the application thread while calls are in flight and another thread stops its RPC listener: the component goes under the node lock" )
    try
    {
        for ( UInt32 round=0; round<20; ++round )
            ASSERT_TRUE( RunSessionDestroyRound( round ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;

    // the protocol thread is not the notification thread: server calls and client stream messages cross between them
    GUCEF_TESTFW_TESTCASE( "Threading: as T13 with handlers on the notification thread and a server stream (T13)" )
    try
    {
        RunThreadingCase( COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD, true );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END;
}

/*-------------------------------------------------------------------------*/
