/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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
#include <deque>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_WEB_CJSONRPCCLIENT_H
#include "gucefWEB_CJsonRpcClient.h"
#define GUCEF_WEB_CJSONRPCCLIENT_H
#endif /* GUCEF_WEB_CJSONRPCCLIENT_H ? */

#ifndef GUCEF_WEB_CJSONRPCSERVER_H
#include "gucefWEB_CJsonRpcServer.h"
#define GUCEF_WEB_CJSONRPCSERVER_H
#endif /* GUCEF_WEB_CJSONRPCSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestJsonRpcWebSocket.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

static bool
EnsurePluginLoaded( const CORE::CString& pluginName )
{
    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
}

static bool
JsonAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return true;
    EnsurePluginLoaded( "dstorepluginJSONPARSER" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false );
}

/** Answers the methods used in the examples of the specification */
class CSpecMethods : public COM::CIJsonRpcRequestHandler
{
    public:
    CSpecMethods( void ) : deferredCalls( 0 ), throwOnNext( false ) {}

    virtual COM::EJsonRpcRequestResult OnJsonRpcRequest( COM::CJsonRpcEndpoint& endpoint     ,
                                                         const COM::SJsonRpcRequest& request ,
                                                         COM::SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( throwOnNext )
            throw std::exception();

        if ( request.method == "subtract" && request.hasParams )
        {
            Int32 minuend = 0;
            Int32 subtrahend = 0;
            if ( GUCEF_DATATYPE_ARRAY == request.params.GetNodeType() )
            {
                CORE::CDataNode::const_iterator i = request.params.ConstBegin();
                minuend = (*i)->GetValue().AsInt32();
                ++i;
                subtrahend = (*i)->GetValue().AsInt32();
            }
            else
            {
                minuend = request.params.GetAttributeValue( "minuend" ).AsInt32();
                subtrahend = request.params.GetAttributeValue( "subtrahend" ).AsInt32();
            }
            response.result.SetValue( CORE::CVariant( (Int32) ( minuend - subtrahend ) ) );
            response.result.SetNodeType( response.result.GetValue().GetTypeId() );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "sum" && request.hasParams )
        {
            Int32 total = 0;
            for ( CORE::CDataNode::const_iterator i = request.params.ConstBegin(); i != request.params.ConstEnd(); ++i )
                total += (*i)->GetValue().AsInt32();
            response.result.SetValue( CORE::CVariant( total ) );
            response.result.SetNodeType( response.result.GetValue().GetTypeId() );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "get_data" )
        {
            response.result.SetNodeType( GUCEF_DATATYPE_ARRAY );
            response.result.AddValueAsChild( CORE::CVariant( CORE::CString( "hello" ) ) );
            response.result.AddValueAsChild( CORE::CVariant( (Int32) 5 ) );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "echo" )
        {
            response.result = request.params;
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "later" )
        {
            ++deferredCalls;
            deferredId = request.id;
            return COM::JSONRPC_REQUEST_DEFERRED;
        }
        if ( request.method == "silent" )
        {
            return COM::JSONRPC_REQUEST_DEFERRED;   // and never answered
        }
        if ( request.method == "fail" )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = "Server error";
            response.error.hasData = true;
            response.error.data.SetNodeType( GUCEF_DATATYPE_OBJECT );
            response.error.data.SetAttribute( "reason", "because" );
            return COM::JSONRPC_REQUEST_HANDLED;
        }

        response.isError = true;
        response.error.code = COM::JSONRPC_ERROR_METHOD_NOT_FOUND;
        response.error.message = "Method not found";
        return COM::JSONRPC_REQUEST_HANDLED;
    }

    UInt32 deferredCalls;
    CORE::CVariant deferredId;
    bool throwOnNext;
};

/** Collects the events of an endpoint */
class CRpcRecorder : public CORE::CObserver
{
    public:
    CRpcRecorder( void ) : responses( 0 ), notifications( 0 ) {}

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata        ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == COM::CJsonRpcEndpoint::ResponseEvent )
        {
            responseList.push_back( static_cast< COM::TJsonRpcResponseEventData* >( eventdata )->GetData() );
            ++responses;
        }
        else
        if ( eventid == COM::CJsonRpcEndpoint::NotificationEvent )
        {
            notificationList.push_back( static_cast< COM::TJsonRpcNotificationEventData* >( eventdata )->GetData() );
            ++notifications;
        }
    }

    UInt32 responses;
    UInt32 notifications;
    GUCEF::vector< COM::SJsonRpcResponse > responseList;
    GUCEF::vector< COM::SJsonRpcNotification > notificationList;
};

/** Remembers what was sent */
class CCaptureTransport : public COM::CIJsonRpcTransport
{
    public:
    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        sent.push_back( messageText );
        return true;
    }
    GUCEF::vector< CORE::CString > sent;
};

/** Pumps only the given pulse generator until the counter reaches the expected value */
static bool
PumpUntilCount( CORE::PulseGeneratorPtr pulseGenerator, const UInt32& counter, UInt32 expected, UInt32 timeoutInMs )
{
    UInt64 startTicks = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( counter < expected )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
            return false;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

static bool
PumpUntilCount( const UInt32& counter, UInt32 expected, UInt32 timeoutInMs = 5000 )
{
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    UInt64 startTicks = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( counter < expected )
    {
        if ( ( MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
            return false;
        pulseGenerator->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
PerformJsonRpcWebSocketTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING JSON-RPC WEB SOCKET TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "JsonRpcWebSocket" );

    if ( !JsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "JSON-RPC WEB SOCKET TESTS skipped: the json codec is not available" );
        return;
    }

    // Client and server over real web sockets
    GUCEF_TESTFW_TESTCASE( "Test: JSON-RPC client and server over web sockets" )
        try
        {
            if ( !EnsurePluginLoaded( "codecspluginSTBRUMMEHASH" ) && !WEB::CWebSocketHandshake::IsAvailable() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "sha1-raw codec not available, JSON-RPC over web sockets skipped" );
            }
            else
            {
                WEB::CWebSocketServer wsServer;
                ASSERT_TRUE( wsServer.Listen( 0 ) );
                const UInt16 port = wsServer.GetHttpServer().GetBoundPort();
                WEB::CJsonRpcServer rpcServer( wsServer );
                CSpecMethods methods;
                rpcServer.RegisterMethod( "sum", &methods );
                rpcServer.RegisterMethod( "subtract", &methods );
                rpcServer.RegisterMethod( "get_data", &methods );
                rpcServer.RegisterMethod( "fail", &methods );
                CRpcRecorder serverRecorder;
                (void) serverRecorder;

                WEB::CWebSocketClient wsClient;
                WEB::CJsonRpcClient rpcClient;
                rpcClient.AttachWebSocketClient( wsClient );
                CRpcRecorder clientRecorder;
                clientRecorder.SubscribeTo( &rpcClient );
                ASSERT_TRUE( wsClient.Connect( "ws://127.0.0.1:" + CORE::UInt32ToString( port ) + "/rpc" ) );
                UInt32 pumped = 0;
                while ( !wsClient.IsConnected() && pumped++ < 5000 )
                {
                    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                ASSERT_TRUE( wsClient.IsConnected() );
                pumped = 0;
                while ( 0 == rpcServer.GetSessionCount() && pumped++ < 5000 )
                {
                    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                ASSERT_TRUE( 1 == rpcServer.GetSessionCount() );

                // a call, answered over the wire
                CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
                params.AddValueAsChild( CORE::CVariant( (Int32) 19 ) );
                params.AddValueAsChild( CORE::CVariant( (Int32) 23 ) );
                COM::SJsonRpcResponse response;
                ASSERT_TRUE( rpcClient.CallAndWait( "sum", &params, response, 5000 ) );
                ASSERT_TRUE( !response.isError && 42 == response.result.GetValue().AsInt32() );

                // errors and unknown methods
                ASSERT_TRUE( rpcClient.CallAndWait( "fail", GUCEF_NULL, response, 5000 ) );
                ASSERT_TRUE( response.isError && !response.isLocalError && -32000 == response.error.code );
                ASSERT_TRUE( rpcClient.CallAndWait( "nonsense", GUCEF_NULL, response, 5000 ) );
                ASSERT_TRUE( response.isError && -32601 == response.error.code );

                // the server notifies its clients and the client the server
                CORE::CDataNode news( GUCEF_DATATYPE_OBJECT );
                news.SetAttribute( "headline", "hello" );
                ASSERT_TRUE( 1 == rpcServer.BroadcastNotification( "news", &news ) );
                ASSERT_TRUE( PumpUntilCount( clientRecorder.notifications, 1 ) );
                ASSERT_TRUE( clientRecorder.notificationList[ 0 ].params.GetAttributeValue( "headline" ).AsString() == "hello" );

                ASSERT_TRUE( rpcClient.Notify( "ping", GUCEF_NULL ) );
                struct SServerNotificationCounter : public CORE::CObserver
                {
                    SServerNotificationCounter( void ) : count( 0 ) {}
                    virtual void OnNotify( CORE::CNotifier*, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
                    {
                        if ( eventid == WEB::CJsonRpcServer::NotificationEvent )
                        {
                            ++count;
                            method = static_cast< WEB::TJsonRpcServerNotificationEventData* >( eventdata )->GetData().notification.method;
                        }
                    }
                    UInt32 count;
                    CORE::CString method;
                } serverNotifications;
                serverNotifications.SubscribeTo( &rpcServer );
                ASSERT_TRUE( rpcClient.Notify( "ping", GUCEF_NULL ) );
                ASSERT_TRUE( PumpUntilCount( serverNotifications.count, 1 ) );
                ASSERT_TRUE( serverNotifications.method == "ping" );

                // the server calls the client, which does not handle calls
                WEB::CWebSocketServer::TConnectionVector connections;
                wsServer.GetConnections( connections );
                ASSERT_TRUE( 1 == connections.size() );
                COM::CJsonRpcEndpoint* session = connections.empty() ? GUCEF_NULL : rpcServer.GetEndpoint( connections[ 0 ] );
                ASSERT_TRUE( GUCEF_NULL != session );
                if ( GUCEF_NULL != session )
                {
                    CRpcRecorder sessionRecorder;
                    sessionRecorder.SubscribeTo( session );
                    UInt64 callId = 0;
                    ASSERT_TRUE( session->Call( "clientside", GUCEF_NULL, callId ) );
                    ASSERT_TRUE( PumpUntilCount( sessionRecorder.responses, 1 ) );
                    ASSERT_TRUE( 1 == sessionRecorder.responses && -32601 == sessionRecorder.responseList[ 0 ].error.code );
                }

                // a call in flight when the connection goes away is failed
                UInt64 hangingId = 0;
                ASSERT_TRUE( rpcClient.Call( "silent", GUCEF_NULL, hangingId, 600000 ) );
                UInt32 responsesBefore = clientRecorder.responses;
                wsClient.Disconnect();
                ASSERT_TRUE( PumpUntilCount( clientRecorder.responses, responsesBefore + 1 ) );
                ASSERT_TRUE( clientRecorder.responseList.back().isLocalError );
                ASSERT_TRUE( COM::JSONRPC_ERROR_TRANSPORT_CLOSED == clientRecorder.responseList.back().error.code );
                ASSERT_TRUE( PumpUntilCount( serverNotifications.count, 1 ) );
                pumped = 0;
                while ( 0 != rpcServer.GetSessionCount() && pumped++ < 5000 )
                {
                    CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                ASSERT_TRUE( 0 == rpcServer.GetSessionCount() );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The sessions of a JSON-RPC server run on the pulse generator of its web socket server, so CallAndWait() on a session works on that thread" )
        try
        {
            if ( !EnsurePluginLoaded( "codecspluginSTBRUMMEHASH" ) && !WEB::CWebSocketHandshake::IsAvailable() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "sha1-raw codec not available, JSON-RPC over web sockets skipped" );
            }
            else
            {
                // everything on a pulse generator of its own, which this thread drives: not the application's
                CORE::PulseGeneratorPtr serverPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
                WEB::CWebSocketServer wsServer( serverPulseGenerator );
                ASSERT_TRUE( wsServer.GetPulseGenerator() == serverPulseGenerator );
                ASSERT_TRUE( wsServer.Listen( 0 ) );
                const UInt16 port = wsServer.GetHttpServer().GetBoundPort();
                WEB::CJsonRpcServer rpcServer( wsServer );

                WEB::CWebSocketClient wsClient( serverPulseGenerator );
                WEB::CJsonRpcClient rpcClient( serverPulseGenerator );
                CSpecMethods methods;
                rpcClient.SetRequestHandler( &methods );
                rpcClient.AttachWebSocketClient( wsClient );
                ASSERT_TRUE( wsClient.Connect( "ws://127.0.0.1:" + CORE::UInt32ToString( port ) + "/rpc" ) );
                UInt32 pumped = 0;
                while ( ( !wsClient.IsConnected() || 0 == rpcServer.GetSessionCount() ) && pumped++ < 5000 )
                {
                    serverPulseGenerator->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
                ASSERT_TRUE( 1 == rpcServer.GetSessionCount() );

                WEB::CWebSocketServer::TConnectionVector connections;
                wsServer.GetConnections( connections );
                ASSERT_TRUE( 1 == connections.size() );
                if ( 1 == connections.size() )
                {
                    COM::CJsonRpcEndpoint* session = rpcServer.GetEndpoint( connections[ 0 ] );
                    ASSERT_TRUE( GUCEF_NULL != session );
                    if ( GUCEF_NULL != session )
                    {
                        ASSERT_TRUE( session->GetPulseGenerator() == serverPulseGenerator );

                        // the server calls its client and waits on this thread, which drives the pulse generator of both
                        CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
                        params.AddValueAsChild( CORE::CVariant( (Int32) 40 ) );
                        params.AddValueAsChild( CORE::CVariant( (Int32) 2 ) );
                        COM::SJsonRpcResponse response;
                        ASSERT_TRUE( session->CallAndWait( "sum", &params, response, 5000 ) );
                        ASSERT_TRUE( !response.isError && 42 == response.result.GetValue().AsInt32() );
                    }
                }
                wsClient.Close();
                for ( UInt32 i=0; i<50; ++i )
                {
                    serverPulseGenerator->ForceDirectPulse();
                    MT::ThreadDelay( 1 );
                }
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The JSON-RPC client times out calls on the pulse generator it is given" )
        try
        {
            CORE::PulseGeneratorPtr globalPulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

            // given at construction
            {
                CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
                localPulseGenerator->SetPulseDriverThreadId( MT::GetCurrentTaskID() );
                WEB::CJsonRpcClient client( localPulseGenerator );
                ASSERT_TRUE( client.GetPulseGenerator() == localPulseGenerator );
                CCaptureTransport transport;
                client.SetTransport( &transport );
                CRpcRecorder recorder;
                recorder.SubscribeTo( &client );

                UInt64 callId = 0;
                ASSERT_TRUE( client.Call( "silent", GUCEF_NULL, callId, 50 ) );
                ASSERT_TRUE( PumpUntilCount( localPulseGenerator, recorder.responses, 1, 3000 ) );
                if ( 1 == recorder.responses )
                {
                    ASSERT_TRUE( recorder.responseList[ 0 ].isLocalError && COM::JSONRPC_ERROR_CALL_TIMEOUT == recorder.responseList[ 0 ].error.code );
                }
                client.SetTransport( GUCEF_NULL );
            }

            // set afterwards: the timer moves along
            {
                CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
                localPulseGenerator->SetPulseDriverThreadId( MT::GetCurrentTaskID() );
                WEB::CJsonRpcClient client;
                ASSERT_TRUE( client.GetPulseGenerator() == globalPulseGenerator );
                client.SetPulseGenerator( localPulseGenerator );
                ASSERT_TRUE( client.GetPulseGenerator() == localPulseGenerator );
                CCaptureTransport transport;
                client.SetTransport( &transport );
                CRpcRecorder recorder;
                recorder.SubscribeTo( &client );

                UInt64 callId = 0;
                ASSERT_TRUE( client.Call( "silent", GUCEF_NULL, callId, 50 ) );
                ASSERT_TRUE( PumpUntilCount( localPulseGenerator, recorder.responses, 1, 3000 ) );
                client.SetTransport( GUCEF_NULL );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL JSON-RPC WEB SOCKET TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
