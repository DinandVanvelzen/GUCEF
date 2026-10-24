/*
 *  gucefWEB_TestApp: GUCEF test application for the WEB module
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

#ifndef GUCEF_WEB_CJSONRPCENDPOINT_H
#include "gucefWEB_CJsonRpcEndpoint.h"
#define GUCEF_WEB_CJSONRPCENDPOINT_H
#endif /* GUCEF_WEB_CJSONRPCENDPOINT_H ? */

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

#include "TestJsonRpc.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

static const UInt16 JSONRPC_TEST_PORT = 47400;

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

/** Parses JSON text for inspecting what an endpoint sent */
static bool
ParseText( const CORE::CString& text, CORE::CDataNode& root )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        return false;
    root.Clear();
    CORE::CDynamicBuffer buffer;
    buffer.CopyFrom( text.ByteSize() - 1, text.C_String() );
    CORE::CDynamicBufferAccess access( buffer );
    return codec->BuildDataTree( &root, &access );
}

/** The id of a message as text: "null" for a null id, "-" if there is none */
/** The number of direct children. CDataNode::GetNrOfChildNodes() counts all descendants */
static UInt32
ChildCount( const CORE::CDataNode& node )
{
    UInt32 count = 0;
    for ( CORE::CDataNode::const_iterator i = node.ConstBegin(); i != node.ConstEnd(); ++i )
        ++count;
    return count;
}

static CORE::CString
IdOf( const CORE::CDataNode& message )
{
    const CORE::CDataNode::TKeyValuePair* id = message.GetAttribute( "id" );
    if ( GUCEF_NULL != id )
        return ( (*id).second.IsNIL() || (*id).second.IsNULL() ) ? CORE::CString( "null" ) : (*id).second.AsString();
    if ( GUCEF_NULL != message.Find( "id" ) )
        return "null";
    return "-";
}

static Int32
ErrorCodeOf( const CORE::CDataNode& message )
{
    const CORE::CDataNode* error = message.Find( "error" );
    if ( GUCEF_NULL == error || GUCEF_NULL == error->GetAttribute( "code" ) )
        return 0;
    return error->GetAttribute( "code" )->second.AsInt32();
}

static bool
HasResultInt( const CORE::CDataNode& message, Int32 expected )
{
    const CORE::CDataNode::TKeyValuePair* result = message.GetAttribute( "result" );
    return GUCEF_NULL != result && (*result).second.AsInt32() == expected;
}

/*-------------------------------------------------------------------------*/

/** Remembers what was sent */
class CCaptureTransport : public WEB::CIJsonRpcTransport
{
    public:
    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        sent.push_back( messageText );
        return true;
    }
    GUCEF::vector< CORE::CString > sent;
};

/**
 *  Connects two endpoints. Messages are delivered right away from within the send call, or when
 *  Flush() is called if 'queued' is set
 */
class CMemoryTransport : public WEB::CIJsonRpcTransport
{
    public:
    CMemoryTransport( void ) : peer( GUCEF_NULL ), queued( false ), connected( true ), messagesSent( 0 ) {}

    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !connected || GUCEF_NULL == peer )
            return false;
        ++messagesSent;
        if ( queued )
            pending.push_back( messageText );
        else
            peer->ProcessIncoming( messageText );
        return true;
    }

    void Flush( void )
    {
        while ( !pending.empty() )
        {
            CORE::CString message = pending.front();
            pending.pop_front();
            peer->ProcessIncoming( message );
        }
    }

    WEB::CJsonRpcEndpoint* peer;
    bool queued;
    bool connected;
    UInt32 messagesSent;
    std::deque< CORE::CString > pending;
};

/** Answers the methods used in the examples of the specification */
class CSpecMethods : public WEB::CIJsonRpcRequestHandler
{
    public:
    CSpecMethods( void ) : deferredCalls( 0 ), throwOnNext( false ) {}

    virtual WEB::EJsonRpcRequestResult OnJsonRpcRequest( WEB::CJsonRpcEndpoint& endpoint     ,
                                                         const WEB::SJsonRpcRequest& request ,
                                                         WEB::SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE
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
            return WEB::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "sum" && request.hasParams )
        {
            Int32 total = 0;
            for ( CORE::CDataNode::const_iterator i = request.params.ConstBegin(); i != request.params.ConstEnd(); ++i )
                total += (*i)->GetValue().AsInt32();
            response.result.SetValue( CORE::CVariant( total ) );
            response.result.SetNodeType( response.result.GetValue().GetTypeId() );
            return WEB::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "get_data" )
        {
            response.result.SetNodeType( GUCEF_DATATYPE_ARRAY );
            response.result.AddValueAsChild( CORE::CVariant( CORE::CString( "hello" ) ) );
            response.result.AddValueAsChild( CORE::CVariant( (Int32) 5 ) );
            return WEB::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "echo" )
        {
            response.result = request.params;
            return WEB::JSONRPC_REQUEST_HANDLED;
        }
        if ( request.method == "later" )
        {
            ++deferredCalls;
            deferredId = request.id;
            return WEB::JSONRPC_REQUEST_DEFERRED;
        }
        if ( request.method == "silent" )
        {
            return WEB::JSONRPC_REQUEST_DEFERRED;   // and never answered
        }
        if ( request.method == "fail" )
        {
            response.isError = true;
            response.error.code = -32000;
            response.error.message = "Server error";
            response.error.hasData = true;
            response.error.data.SetNodeType( GUCEF_DATATYPE_OBJECT );
            response.error.data.SetAttribute( "reason", "because" );
            return WEB::JSONRPC_REQUEST_HANDLED;
        }

        response.isError = true;
        response.error.code = WEB::JSONRPC_ERROR_METHOD_NOT_FOUND;
        response.error.message = "Method not found";
        return WEB::JSONRPC_REQUEST_HANDLED;
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
        if ( eventid == WEB::CJsonRpcEndpoint::ResponseEvent )
        {
            responseList.push_back( static_cast< WEB::TJsonRpcResponseEventData* >( eventdata )->GetData() );
            ++responses;
        }
        else
        if ( eventid == WEB::CJsonRpcEndpoint::NotificationEvent )
        {
            notificationList.push_back( static_cast< WEB::TJsonRpcNotificationEventData* >( eventdata )->GetData() );
            ++notifications;
        }
    }

    UInt32 responses;
    UInt32 notifications;
    GUCEF::vector< WEB::SJsonRpcResponse > responseList;
    GUCEF::vector< WEB::SJsonRpcNotification > notificationList;
};

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
PerformJsonRpcTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING JSON-RPC TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "JsonRpc" );

    if ( !JsonAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "JSON-RPC TESTS skipped: the json codec is not available" );
        return;
    }

    // The examples of section 7 of the JSON-RPC 2.0 specification: what is sent in and what must come back
    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: calls with positional and named parameters" )
        try
        {
            CCaptureTransport transport;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            CORE::CDataNode response;

            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [42, 23], \"id\": 1}" ) );
            ASSERT_TRUE( 1 == transport.sent.size() );
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( response.GetAttributeValue( "jsonrpc" ).AsString() == "2.0" );
            ASSERT_TRUE( HasResultInt( response, 19 ) && IdOf( response ) == "1" );

            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [23, 42], \"id\": 2}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], response ) );
            ASSERT_TRUE( HasResultInt( response, -19 ) && IdOf( response ) == "2" );

            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": {\"subtrahend\": 23, \"minuend\": 42}, \"id\": 3}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 2 ], response ) );
            ASSERT_TRUE( HasResultInt( response, 19 ) && IdOf( response ) == "3" );

            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": {\"minuend\": 42, \"subtrahend\": 23}, \"id\": 4}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 3 ], response ) );
            ASSERT_TRUE( HasResultInt( response, 19 ) && IdOf( response ) == "4" );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: notifications get no response" )
        try
        {
            CCaptureTransport transport;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &endpoint );

            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"update\", \"params\": [1,2,3,4,5]}" ) );
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"foobar\"}" ) );
            ASSERT_TRUE( transport.sent.empty() );
            ASSERT_TRUE( 2 == recorder.notifications );
            if ( 2 == recorder.notifications )
            {
                ASSERT_TRUE( recorder.notificationList[ 0 ].method == "update" );
                ASSERT_TRUE( recorder.notificationList[ 0 ].hasParams && 5 == ChildCount( recorder.notificationList[ 0 ].params ) );
                ASSERT_TRUE( recorder.notificationList[ 1 ].method == "foobar" );
                ASSERT_FALSE( recorder.notificationList[ 1 ].hasParams );
            }
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: error responses" )
        try
        {
            CCaptureTransport transport;
            WEB::CJsonRpcEndpoint endpoint( &transport );
            CSpecMethods methods;
            endpoint.SetRequestHandler( &methods );
            CORE::CDataNode response;

            // non-existent method: the id is echoed with its string type
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"foobar\", \"id\": \"1\"}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( -32601 == ErrorCodeOf( response ) && IdOf( response ) == "1" );

            // invalid json
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"foobar, \"params\": \"bar\", \"baz]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], response ) );
            ASSERT_TRUE( -32700 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // invalid request object
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": 1, \"params\": \"bar\"}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 2 ], response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // params which are not structured
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": \"bar\", \"id\": 8}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 3 ], response ) );
            ASSERT_TRUE( -32602 == ErrorCodeOf( response ) && IdOf( response ) == "8" );

            // an error with data
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"fail\", \"id\": 9}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 4 ], response ) );
            ASSERT_TRUE( -32000 == ErrorCodeOf( response ) );
            const CORE::CDataNode* error = response.Find( "error" );
            ASSERT_TRUE( GUCEF_NULL != error && error->GetAttributeValue( "message" ).AsString() == "Server error" );
            ASSERT_TRUE( GUCEF_NULL != error && GUCEF_NULL != error->Find( "data" ) && error->Find( "data" )->GetAttributeValue( "reason" ).AsString() == "because" );

            // a handler that throws becomes an internal error instead of taking the endpoint down
            methods.throwOnNext = true;
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1], \"id\": 10}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 5 ], response ) );
            ASSERT_TRUE( -32603 == ErrorCodeOf( response ) && IdOf( response ) == "10" );

            // a call when nobody handles calls
            WEB::CJsonRpcEndpoint noHandler( &transport );
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == noHandler.ProcessIncoming( "{\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"id\": 11}" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 6 ], response ) );
            ASSERT_TRUE( -32601 == ErrorCodeOf( response ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Spec examples: batches" )
        try
        {
            CCaptureTransport transport;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetRequestHandler( &methods );
            CORE::CDataNode response;

            // invalid json in a batch
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[ {\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2,4], \"id\": \"1\"}, {\"jsonrpc\": \"2.0\", \"method\" ]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], response ) );
            ASSERT_TRUE( -32700 == ErrorCodeOf( response ) );

            // empty batch
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( response ) && IdOf( response ) == "null" );

            // batch with an invalid entry gives a batch response with an error for it
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[1]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 2 ], response ) );
            ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == response.GetNodeType() && 1 == ChildCount( response ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( **response.ConstBegin() ) );

            // batch of invalid entries
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming( "[1,2,3]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 3 ], response ) );
            ASSERT_TRUE( 3 == ChildCount( response ) );

            // the big example: calls, a notification, an invalid entry and a missing method
            response = CORE::CDataNode();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == endpoint.ProcessIncoming(
                "[ {\"jsonrpc\": \"2.0\", \"method\": \"sum\", \"params\": [1,2,4], \"id\": \"1\"},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"notify_hello\", \"params\": [7]},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"subtract\", \"params\": [42,23], \"id\": \"2\"},"
                "  {\"foo\": \"boo\"},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"foo.get\", \"params\": {\"name\": \"myself\"}, \"id\": \"5\"},"
                "  {\"jsonrpc\": \"2.0\", \"method\": \"get_data\", \"id\": \"9\"} ]" ) );
            ASSERT_TRUE( ParseText( transport.sent[ 4 ], response ) );
            ASSERT_TRUE( 5 == ChildCount( response ) );
            if ( 5 == ChildCount( response ) )
            {
                CORE::CDataNode::const_iterator i = response.ConstBegin();
                const CORE::CDataNode& sum = **i;
                ++i;
                const CORE::CDataNode& subtract = **i;
                ++i;
                const CORE::CDataNode& invalid = **i;
                ++i;
                const CORE::CDataNode& missing = **i;
                ++i;
                const CORE::CDataNode& data = **i;
                ASSERT_TRUE( IdOf( sum ) == "1" && HasResultInt( sum, 7 ) );
                ASSERT_TRUE( IdOf( subtract ) == "2" && HasResultInt( subtract, 19 ) );
                ASSERT_TRUE( IdOf( invalid ) == "null" && -32600 == ErrorCodeOf( invalid ) );
                ASSERT_TRUE( IdOf( missing ) == "5" && -32601 == ErrorCodeOf( missing ) );
                const CORE::CDataNode* resultArray = data.Find( "result" );
                ASSERT_TRUE( IdOf( data ) == "9" && GUCEF_NULL != resultArray && GUCEF_DATATYPE_ARRAY == resultArray->GetNodeType() && 2 == ChildCount( *resultArray ) );
            }

            // only notifications: nothing comes back at all
            size_t sentBefore = transport.sent.size();
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "[ {\"jsonrpc\": \"2.0\", \"method\": \"notify_sum\", \"params\": [1,2,4]}, {\"jsonrpc\": \"2.0\", \"method\": \"notify_hello\", \"params\": [7]} ]" ) );
            ASSERT_TRUE( sentBefore == transport.sent.size() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Calls between two endpoints" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint a( &aToB );
            WEB::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            // parameters, result and the id which ties them together
            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 5 ) );
            params.AddValueAsChild( CORE::CVariant( (Int32) 6 ) );
            UInt64 firstId = 0;
            UInt64 secondId = 0;
            ASSERT_TRUE( a.Call( "sum", &params, firstId ) );
            ASSERT_TRUE( a.Call( "sum", &params, secondId ) );
            ASSERT_TRUE( firstId != secondId && firstId > 0 );
            ASSERT_TRUE( 2 == recorder.responses );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );
            if ( 2 == recorder.responses )
            {
                ASSERT_TRUE( recorder.responseList[ 0 ].id.AsUInt64() == firstId && recorder.responseList[ 1 ].id.AsUInt64() == secondId );
                ASSERT_FALSE( recorder.responseList[ 0 ].isError );
                ASSERT_TRUE( 11 == recorder.responseList[ 0 ].result.GetValue().AsInt32() );
            }

            // errors come back with their code, message and data
            UInt64 failId = 0;
            ASSERT_TRUE( a.Call( "fail", GUCEF_NULL, failId ) );
            ASSERT_TRUE( 3 == recorder.responses );
            if ( 3 == recorder.responses )
            {
                const WEB::SJsonRpcResponse& failed = recorder.responseList[ 2 ];
                ASSERT_TRUE( failed.isError && !failed.isLocalError );
                ASSERT_TRUE( -32000 == failed.error.code && failed.error.message == "Server error" );
                ASSERT_TRUE( failed.error.hasData && failed.error.data.GetAttributeValue( "reason" ).AsString() == "because" );
            }

            // a method nobody knows
            UInt64 unknownId = 0;
            ASSERT_TRUE( a.Call( "nonsense", GUCEF_NULL, unknownId ) );
            ASSERT_TRUE( 4 == recorder.responses && -32601 == recorder.responseList[ 3 ].error.code );

            // structured results survive
            UInt64 dataId = 0;
            ASSERT_TRUE( a.Call( "get_data", GUCEF_NULL, dataId ) );
            ASSERT_TRUE( 5 == recorder.responses );
            if ( 5 == recorder.responses )
            {
                const CORE::CDataNode& result = recorder.responseList[ 4 ].result;
                ASSERT_TRUE( GUCEF_DATATYPE_ARRAY == result.GetNodeType() && 2 == ChildCount( result ) );
                ASSERT_TRUE( (*result.ConstBegin())->GetValue().AsString() == "hello" );
            }

            // invalid parameters are refused locally
            CORE::CDataNode scalar;
            scalar.SetValue( CORE::CVariant( (Int32) 1 ) );
            scalar.SetNodeType( scalar.GetValue().GetTypeId() );
            UInt64 badId = 0;
            ASSERT_FALSE( a.Call( "sum", &scalar, badId ) );
            ASSERT_FALSE( a.Call( "", GUCEF_NULL, badId ) );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Both sides can call each other and send notifications" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methodsOfA;
            CSpecMethods methodsOfB;
            WEB::CJsonRpcEndpoint a( &aToB );
            WEB::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            a.SetRequestHandler( &methodsOfA );
            b.SetRequestHandler( &methodsOfB );
            CRpcRecorder recorderA;
            CRpcRecorder recorderB;
            recorderA.SubscribeTo( &a );
            recorderB.SubscribeTo( &b );

            UInt64 idA = 0;
            UInt64 idB = 0;
            ASSERT_TRUE( a.Call( "get_data", GUCEF_NULL, idA ) );
            ASSERT_TRUE( b.Call( "get_data", GUCEF_NULL, idB ) );
            ASSERT_TRUE( 1 == recorderA.responses && 1 == recorderB.responses );

            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "event", "tick" );
            ASSERT_TRUE( a.Notify( "news", &params ) );
            ASSERT_TRUE( 1 == recorderB.notifications );
            ASSERT_TRUE( 0 == recorderA.notifications );
            if ( 1 == recorderB.notifications )
            {
                ASSERT_TRUE( recorderB.notificationList[ 0 ].method == "news" );
                ASSERT_TRUE( recorderB.notificationList[ 0 ].params.GetAttributeValue( "event" ).AsString() == "tick" );
            }
            ASSERT_FALSE( a.Notify( "", GUCEF_NULL ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Answers arriving later, in other order or never" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint a( &aToB );
            WEB::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            // deferred: the answer is sent later through the endpoint
            UInt64 laterId = 0;
            ASSERT_TRUE( a.Call( "later", GUCEF_NULL, laterId ) );
            ASSERT_TRUE( 0 == recorder.responses && 1 == a.GetPendingCallCount() && 1 == methods.deferredCalls );
            CORE::CDataNode result;
            result.SetValue( CORE::CVariant( CORE::CString( "done" ) ) );
            result.SetNodeType( result.GetValue().GetTypeId() );
            ASSERT_TRUE( b.SendResponse( methods.deferredId, result ) );
            ASSERT_TRUE( 1 == recorder.responses && 0 == a.GetPendingCallCount() );
            ASSERT_TRUE( recorder.responseList[ 0 ].result.GetValue().AsString() == "done" );

            // a response for something that is not pending (late or unknown) is ignored
            ASSERT_TRUE( b.SendResponse( CORE::CVariant( (UInt64) 424242 ), result ) );
            ASSERT_TRUE( 1 == recorder.responses );

            // a queued transport delivers later: the call is pending until then
            aToB.queued = true;
            bToA.queued = true;
            UInt64 queuedId = 0;
            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 1 ) );
            ASSERT_TRUE( a.Call( "sum", &params, queuedId ) );
            ASSERT_TRUE( 1 == a.GetPendingCallCount() );
            aToB.Flush();
            ASSERT_TRUE( 1 == a.GetPendingCallCount() );
            bToA.Flush();
            ASSERT_TRUE( 0 == a.GetPendingCallCount() && 2 == recorder.responses );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Calls that are not answered time out and calls on a lost transport fail" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint a( &aToB );
            WEB::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            UInt64 silentId = 0;
            ASSERT_TRUE( a.Call( "silent", GUCEF_NULL, silentId, 50 ) );
            UInt64 longId = 0;
            ASSERT_TRUE( a.Call( "silent", GUCEF_NULL, longId, 600000 ) );
            ASSERT_TRUE( 2 == a.GetPendingCallCount() );

            UInt32 expired = 0;
            ASSERT_TRUE( a.ExpireTimedOutCalls( &expired ) );
            ASSERT_TRUE( 0 == expired );
            MT::ThreadDelay( 100 );
            ASSERT_TRUE( a.ExpireTimedOutCalls( &expired ) );
            ASSERT_TRUE( 1 == expired && 1 == recorder.responses && 1 == a.GetPendingCallCount() );
            if ( 1 == recorder.responses )
            {
                ASSERT_TRUE( recorder.responseList[ 0 ].isError && recorder.responseList[ 0 ].isLocalError );
                ASSERT_TRUE( WEB::JSONRPC_ERROR_CALL_TIMEOUT == recorder.responseList[ 0 ].error.code );
                ASSERT_TRUE( recorder.responseList[ 0 ].id.AsUInt64() == silentId );
            }

            // the transport goes away
            ASSERT_TRUE( a.FailAllPendingCalls( WEB::JSONRPC_ERROR_TRANSPORT_CLOSED, "gone" ) );
            ASSERT_TRUE( 2 == recorder.responses && 0 == a.GetPendingCallCount() );
            ASSERT_TRUE( WEB::JSONRPC_ERROR_TRANSPORT_CLOSED == recorder.responseList[ 1 ].error.code );
            ASSERT_TRUE( recorder.responseList[ 1 ].id.AsUInt64() == longId );

            // a transport that cannot send fails the call right away and leaves nothing pending
            aToB.connected = false;
            UInt64 failedId = 0;
            ASSERT_FALSE( a.Call( "sum", GUCEF_NULL, failedId ) );
            ASSERT_TRUE( 0 == a.GetPendingCallCount() );

            // no transport at all
            WEB::CJsonRpcEndpoint lonely;
            ASSERT_FALSE( lonely.Call( "sum", GUCEF_NULL, failedId ) );
            ASSERT_FALSE( lonely.Notify( "sum", GUCEF_NULL ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Batch calls" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint a( &aToB );
            WEB::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorderA;
            CRpcRecorder recorderB;
            recorderA.SubscribeTo( &a );
            recorderB.SubscribeTo( &b );

            WEB::CJsonRpcEndpoint::TBatchItemVector items( 3 );
            items[ 0 ].method = "sum";
            items[ 0 ].params = CORE::CDataNode( GUCEF_DATATYPE_ARRAY );
            items[ 0 ].params.AddValueAsChild( CORE::CVariant( (Int32) 20 ) );
            items[ 0 ].params.AddValueAsChild( CORE::CVariant( (Int32) 22 ) );
            items[ 0 ].hasParams = true;
            items[ 1 ].method = "note";
            items[ 1 ].isNotification = true;
            items[ 2 ].method = "get_data";
            ASSERT_TRUE( a.CallBatch( items ) );

            // one batch message, answered by one batch message, handled as individual responses
            ASSERT_TRUE( 1 == aToB.messagesSent && 1 == bToA.messagesSent );
            ASSERT_TRUE( 2 == recorderA.responses && 1 == recorderB.notifications );
            if ( 2 == recorderA.responses )
            {
                ASSERT_TRUE( recorderA.responseList[ 0 ].id.AsUInt64() == items[ 0 ].callId && 42 == recorderA.responseList[ 0 ].result.GetValue().AsInt32() );
                ASSERT_TRUE( recorderA.responseList[ 1 ].id.AsUInt64() == items[ 2 ].callId );
            }
            WEB::CJsonRpcEndpoint::TBatchItemVector noItems;
            ASSERT_FALSE( a.CallBatch( noItems ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: CallAndWait returns the answer" )
        try
        {
            CMemoryTransport aToB;
            CMemoryTransport bToA;
            CSpecMethods methods;
            WEB::CJsonRpcEndpoint a( &aToB );
            WEB::CJsonRpcEndpoint b( &bToA );
            aToB.peer = &b;
            bToA.peer = &a;
            b.SetRequestHandler( &methods );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &a );

            CORE::CDataNode params( GUCEF_DATATYPE_ARRAY );
            params.AddValueAsChild( CORE::CVariant( (Int32) 40 ) );
            params.AddValueAsChild( CORE::CVariant( (Int32) 2 ) );
            WEB::SJsonRpcResponse response;
            ASSERT_TRUE( a.CallAndWait( "sum", &params, response, 2000 ) );
            ASSERT_FALSE( response.isError );
            ASSERT_TRUE( 42 == response.result.GetValue().AsInt32() );
            ASSERT_TRUE( 0 == recorder.responses );       // not delivered as an event as it was returned

            // an error answer is still an answer
            ASSERT_TRUE( a.CallAndWait( "fail", GUCEF_NULL, response, 2000 ) );
            ASSERT_TRUE( response.isError && -32000 == response.error.code );

            // nobody answers: the wait ends with a timeout error
            ASSERT_TRUE( a.CallAndWait( "silent", GUCEF_NULL, response, 100 ) );
            ASSERT_TRUE( response.isError && response.isLocalError && WEB::JSONRPC_ERROR_CALL_TIMEOUT == response.error.code );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Lenient dialect and extra envelope fields" )
        try
        {
            CCaptureTransport transport;
            WEB::CJsonRpcEndpoint endpoint( &transport );
            endpoint.SetDialect( WEB::JSONRPC_DIALECT_LENIENT );
            endpoint.SetOutgoingCallField( "sessionId", CORE::CVariant( CORE::CString( "ABC123" ) ) );
            CRpcRecorder recorder;
            recorder.SubscribeTo( &endpoint );

            // what goes out has no version field but does have the extra field
            UInt64 callId = 0;
            CORE::CDataNode params( GUCEF_DATATYPE_OBJECT );
            params.SetAttribute( "url", "http://example.org/" );
            ASSERT_TRUE( endpoint.Call( "Page.navigate", &params, callId ) );
            ASSERT_TRUE( 1 == transport.sent.size() );
            CORE::CDataNode sent;
            ASSERT_TRUE( ParseText( transport.sent[ 0 ], sent ) );
            ASSERT_TRUE( GUCEF_NULL == sent.GetAttribute( "jsonrpc" ) );
            ASSERT_TRUE( sent.GetAttributeValue( "method" ).AsString() == "Page.navigate" );
            ASSERT_TRUE( sent.GetAttributeValue( "sessionId" ).AsString() == "ABC123" );
            ASSERT_TRUE( GUCEF_NULL != sent.Find( "params" ) );

            // a CDP style answer without version field is accepted and the extra fields are available
            CORE::CString answer = "{\"id\":" + CORE::UInt64ToString( callId ) + ",\"result\":{\"frameId\":\"F1\"},\"sessionId\":\"ABC123\"}";
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( answer ) );
            ASSERT_TRUE( 1 == recorder.responses );
            if ( 1 == recorder.responses )
            {
                ASSERT_TRUE( recorder.responseList[ 0 ].result.GetAttributeValue( "frameId" ).AsString() == "F1" );
                ASSERT_TRUE( recorder.responseList[ 0 ].envelope.GetAttributeValue( "sessionId" ).AsString() == "ABC123" );
            }

            // an event, which is a notification without version field
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_OK == endpoint.ProcessIncoming( "{\"method\":\"Page.loadEventFired\",\"params\":{\"timestamp\":1.5},\"sessionId\":\"ABC123\"}" ) );
            ASSERT_TRUE( 1 == recorder.notifications );

            // fields can be removed again
            endpoint.SetOutgoingCallField( "", CORE::CVariant() );
            ASSERT_TRUE( endpoint.Notify( "x", GUCEF_NULL ) );
            sent = CORE::CDataNode();
            ASSERT_TRUE( ParseText( transport.sent[ 1 ], sent ) );
            ASSERT_TRUE( GUCEF_NULL == sent.GetAttribute( "sessionId" ) );

            // the strict dialect insists on the version field
            CCaptureTransport strictTransport;
            WEB::CJsonRpcEndpoint strict( &strictTransport );
            ASSERT_TRUE( WEB::CJsonRpcEndpoint::PROCESS_INVALID == strict.ProcessIncoming( "{\"method\":\"sum\",\"id\":1}" ) );
            CORE::CDataNode strictResponse;
            ASSERT_TRUE( 1 == strictTransport.sent.size() && ParseText( strictTransport.sent[ 0 ], strictResponse ) );
            ASSERT_TRUE( -32600 == ErrorCodeOf( strictResponse ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

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
                ASSERT_TRUE( wsServer.Listen( JSONRPC_TEST_PORT ) );
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
                ASSERT_TRUE( wsClient.Connect( "ws://127.0.0.1:" + CORE::UInt32ToString( JSONRPC_TEST_PORT ) + "/rpc" ) );
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
                WEB::SJsonRpcResponse response;
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
                WEB::CJsonRpcEndpoint* session = connections.empty() ? GUCEF_NULL : rpcServer.GetEndpoint( connections[ 0 ] );
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
                ASSERT_TRUE( WEB::JSONRPC_ERROR_TRANSPORT_CLOSED == clientRecorder.responseList.back().error.code );
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

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL JSON-RPC TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
