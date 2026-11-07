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
#include <vector>
#include <stdexcept>

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

#ifndef GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#include "gucefWEB_CChromeDevToolsClient.h"
#define GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H
#endif /* GUCEF_WEB_CCHROMEDEVTOOLSCLIENT_H ? */

#ifndef GUCEF_WEB_CJSONRPCSERVER_H
#include "gucefWEB_CJsonRpcServer.h"
#define GUCEF_WEB_CJSONRPCSERVER_H
#endif /* GUCEF_WEB_CJSONRPCSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestChromeDevTools.h"

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

static const UInt16 FAKE_CHROME_PORT = 47500;
static const UInt16 NOTHING_LISTENS_PORT = 47599;

/*-------------------------------------------------------------------------*/

static bool
LoadPlugin( const CORE::CString& pluginName )
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
PluginsAvailable( void )
{
    CORE::CDStoreCodecRegistry::TDStoreCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) )
        LoadPlugin( "dstorepluginJSONPARSER" );
    if ( !WEB::CWebSocketHandshake::IsAvailable() )
        LoadPlugin( "codecspluginSTBRUMMEHASH" );
    return CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryLookup( "json", codec, false ) && WEB::CWebSocketHandshake::IsAvailable();
}

static bool
PumpUntilTrue( const bool& flag, UInt32 timeoutInMs = 5000 )
{
    UInt64 start = MT::PrecisionTickCount();
    Float64 ticksPerMs = MT::PrecisionTimerResolution() / 1000.0;
    while ( !flag )
    {
        if ( ( MT::PrecisionTickCount() - start ) / ticksPerMs > timeoutInMs )
            return false;
        CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
        MT::ThreadDelay( 1 );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The discovery interface of the fake browser */
class CFakeChromeHttpHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        CORE::CString port = CORE::UInt32ToString( FAKE_CHROME_PORT );
        CORE::CString body;
        if ( request.requestUri == "/json/version" )
        {
            body = "{\n   \"Browser\": \"Chrome/120.0.0.0\",\n   \"Protocol-Version\": \"1.3\",\n   \"User-Agent\": \"Mozilla/5.0 Fake\",\n"
                   "   \"webSocketDebuggerUrl\": \"ws://127.0.0.1:" + port + "/devtools/browser/B1\"\n}";
        }
        else
        if ( request.requestUri == "/json/list" )
        {
            body = "[ {\n   \"description\": \"\",\n   \"devtoolsFrontendUrl\": \"/devtools/inspector.html?ws=127.0.0.1:" + port + "/devtools/page/P1\",\n"
                   "   \"id\": \"P1\",\n   \"title\": \"First page\",\n   \"type\": \"page\",\n   \"url\": \"http://example.org/\",\n"
                   "   \"webSocketDebuggerUrl\": \"ws://127.0.0.1:" + port + "/devtools/page/P1\"\n}, {\n"
                   "   \"description\": \"\",\n   \"id\": \"W2\",\n   \"title\": \"Worker\",\n   \"type\": \"service_worker\",\n   \"url\": \"http://example.org/sw.js\",\n"
                   "   \"webSocketDebuggerUrl\": \"ws://127.0.0.1:" + port + "/devtools/page/W2\"\n} ]";
        }
        else
        {
            response.statusCode = 404;
            return true;
        }

        response.statusCode = 200;
        response.contentType = "application/json; charset=UTF-8";
        response.content.CopyFrom( body.ByteSize() - 1, body.C_String() );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }
};

typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CFakeChromeHttpHandler, MT::CMutex > TFakeChromeHttpHandlerFactory;

/** The commands the fake browser understands */
class CFakeChromeCommands : public COM::CIJsonRpcRequestHandler
{
    public:

    virtual COM::EJsonRpcRequestResult OnJsonRpcRequest( COM::CJsonRpcEndpoint& endpoint     ,
                                                         const COM::SJsonRpcRequest& request ,
                                                         COM::SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE
    {
        response.result = CORE::CDataNode( GUCEF_DATATYPE_OBJECT );
        CORE::CDataNode& result = response.result;
        const CORE::CString& method = request.method;

        if ( method == "Network.enable" )
        {
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Network.getAllCookies" )
        {
            CORE::CDataNode* cookies = result.AddChild( "cookies", GUCEF_DATATYPE_ARRAY );
            CORE::CDataNode* first = cookies->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            first->SetAttribute( "name", "sessionid" );
            first->SetAttribute( "value", "abc123" );
            first->SetAttribute( "domain", ".example.org" );
            first->SetAttribute( "path", "/" );
            first->SetAttribute( "expires", (Float32) 1900000000.0f );
            first->SetAttribute( "httpOnly", true );
            first->SetAttribute( "secure", true );
            first->SetAttribute( "session", false );
            first->SetAttribute( "sameSite", "Lax" );
            CORE::CDataNode* second = cookies->AddChild( CORE::CString::Empty, GUCEF_DATATYPE_OBJECT );
            second->SetAttribute( "name", "theme" );
            second->SetAttribute( "value", "dark" );
            second->SetAttribute( "domain", "example.org" );
            second->SetAttribute( "path", "/app" );
            second->SetAttribute( "expires", (Float32) -1.0f );
            second->SetAttribute( "httpOnly", false );
            second->SetAttribute( "secure", false );
            second->SetAttribute( "session", true );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Page.navigate" )
        {
            if ( request.params.GetAttributeValue( "url" ).AsString() == "http://broken.invalid/" )
            {
                result.SetAttribute( "frameId", "F1" );
                result.SetAttribute( "errorText", "net::ERR_NAME_NOT_RESOLVED" );
            }
            else
            {
                result.SetAttribute( "frameId", "F1" );

                // the browser reports the page loaded
                CORE::CDataNode eventParams( GUCEF_DATATYPE_OBJECT );
                eventParams.SetAttribute( "timestamp", (Float32) 12.5f );
                endpoint.Notify( "Page.loadEventFired", &eventParams );
            }
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Runtime.evaluate" )
        {
            CORE::CString expression = request.params.GetAttributeValue( "expression" ).AsString();
            CORE::CDataNode* evaluated = result.AddChild( "result", GUCEF_DATATYPE_OBJECT );
            if ( expression == "1+41" )
            {
                evaluated->SetAttribute( "type", "number" );
                evaluated->SetAttribute( "value", (Int32) 42 );
            }
            else
            if ( expression == "'hello'" )
            {
                evaluated->SetAttribute( "type", "string" );
                evaluated->SetAttribute( "value", "hello" );
            }
            else
            if ( expression == "({a:1,b:2})" )
            {
                evaluated->SetAttribute( "type", "object" );
                CORE::CDataNode* value = evaluated->AddChild( "value", GUCEF_DATATYPE_OBJECT );
                value->SetAttribute( "a", (Int32) 1 );
                value->SetAttribute( "b", (Int32) 2 );
            }
            else
            if ( expression == "undefined" )
            {
                evaluated->SetAttribute( "type", "undefined" );
            }
            else
            {
                // a script that throws
                evaluated->SetAttribute( "type", "object" );
                CORE::CDataNode* details = result.AddChild( "exceptionDetails", GUCEF_DATATYPE_OBJECT );
                details->SetAttribute( "text", "Uncaught" );
                CORE::CDataNode* exception = details->AddChild( "exception", GUCEF_DATATYPE_OBJECT );
                exception->SetAttribute( "description", "Error: boom" );
                exception->SetAttribute( "type", "object" );
            }
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Target.createTarget" )
        {
            result.SetAttribute( "targetId", "T2" );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Target.attachToTarget" && request.params.GetAttributeValue( "flatten" ).AsBool( false ) )
        {
            result.SetAttribute( "sessionId", "S9" );
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Network.getResponseBody" )
        {
            if ( request.params.GetAttributeValue( "requestId" ).AsString() == "R1" )
            {
                result.SetAttribute( "body", "aGk=" );
                result.SetAttribute( "base64Encoded", true );
                return COM::JSONRPC_REQUEST_HANDLED;
            }
            response.isError = true;
            response.error.code = -32000;
            response.error.message = "No resource with given identifier found";
            return COM::JSONRPC_REQUEST_HANDLED;
        }
        if ( method == "Debug.sessionOf" )
        {
            // tells which session the command was directed to
            result.SetAttribute( "sessionId", request.envelope.GetAttributeValue( "sessionId" ).AsString() );
            return COM::JSONRPC_REQUEST_HANDLED;
        }

        response.isError = true;
        response.error.code = COM::JSONRPC_ERROR_METHOD_NOT_FOUND;
        response.error.message = "'" + method + "' wasn't found";
        return COM::JSONRPC_REQUEST_HANDLED;
    }
};

/** Collects the events of the client */
class CChromeRecorder : public CORE::CObserver
{
    public:
    CChromeRecorder( void ) : discoveryCompleted( 0 ), discoveryFailed( 0 ), connected( 0 ), disconnected( 0 ), protocolEvents( 0 ) {}

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == WEB::CChromeDevToolsClient::DiscoveryCompletedEvent )
            ++discoveryCompleted;
        else
        if ( eventid == WEB::CChromeDevToolsClient::DiscoveryFailedEvent )
            ++discoveryFailed;
        else
        if ( eventid == WEB::CChromeDevToolsClient::ConnectedEvent )
            ++connected;
        else
        if ( eventid == WEB::CChromeDevToolsClient::DisconnectedEvent )
            ++disconnected;
        else
        if ( eventid == WEB::CChromeDevToolsClient::ProtocolEvent )
        {
            events.push_back( static_cast< WEB::TChromeProtocolEventData* >( eventdata )->GetData() );
            ++protocolEvents;
        }
    }

    UInt32 discoveryCompleted, discoveryFailed, connected, disconnected, protocolEvents;
    GUCEF::vector< WEB::SChromeProtocolEvent > events;
};

/** Counts the pulses a pulse generator gets from threads other than its owner */
class CForeignPulseCounter : public CORE::CObserver
{
    public:

    CForeignPulseCounter( CORE::PulseGeneratorPtr pulseGenerator, UInt32 ownerThreadId )
        : CORE::CObserver()
        , m_ownerThreadId( ownerThreadId )
        , m_foreignPulses( 0 )
        , m_lock()
    {
        TEventCallback callback( this, &CForeignPulseCounter::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
    }

    UInt32 GetForeignPulses( void ) const { MT::CScopeMutex lock( m_lock ); return m_foreignPulses; }

    private:

    typedef CORE::CTEventHandlerFunctor< CForeignPulseCounter > TEventCallback;

    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        MT::CScopeMutex lock( m_lock );
        if ( MT::GetCurrentTaskID() != m_ownerThreadId )
            ++m_foreignPulses;
    }

    UInt32 m_ownerThreadId;
    UInt32 m_foreignPulses;
    MT::CMutex m_lock;
};

/** Uses one of the blocking members of a client on a thread of its own */
class CChromeWaitThread : public MT::CActiveObject
{
    public:

    enum EOperation
    {
        OPERATION_COMMAND   ,
        OPERATION_DISCOVERY ,
        OPERATION_CONNECT
    };

    CChromeWaitThread( WEB::CChromeDevToolsClient& client, EOperation operation, const CORE::CString& webSocketDebuggerUrl )
        : MT::CActiveObject()
        , result( false )
        , m_client( client )
        , m_operation( operation )
        , m_webSocketDebuggerUrl( webSocketDebuggerUrl )
        , m_done( false )
    {}

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        return true;
    }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {GUCEF_TRACE;
        if ( OPERATION_COMMAND == m_operation )
        {
            CORE::CDataNode commandResult;
            result = m_client.SendCommandAndWait( "Network.enable", GUCEF_NULL, commandResult, GUCEF_NULL, 1000 );
        }
        else
        if ( OPERATION_DISCOVERY == m_operation )
        {
            WEB::SChromeDiscoveryResult discovery;
            result = m_client.DiscoverTargetsAndWait( "127.0.0.1", FAKE_CHROME_PORT, discovery, 1000 );
        }
        else
        {
            result = m_client.ConnectAndWait( m_webSocketDebuggerUrl, 1000 );
        }
        m_done.Store( true, MT::MEMORDER_RELEASE );
        return true; // stop cycling
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    /** Runs the calls and waits until the thread is gone. The results may only be read after this returned true */
    bool Run( void )
    {
        if ( !Activate( GUCEF_NULL, 0, 0 ) )
            return false;
        UInt32 waitedInMs = 0;
        while ( !m_done.Load( MT::MEMORDER_ACQUIRE ) && waitedInMs < 15000 )
        {
            MT::PrecisionDelay( 1 );
            ++waitedInMs;
        }
        while ( m_done.Load( MT::MEMORDER_ACQUIRE ) && IsActive() && waitedInMs < 20000 )
        {
            MT::PrecisionDelay( 1 );
            ++waitedInMs;
        }
        return m_done.Load( MT::MEMORDER_ACQUIRE ) && !IsActive();
    }

    bool result;

    private:

    WEB::CChromeDevToolsClient& m_client;
    EOperation m_operation;
    CORE::CString m_webSocketDebuggerUrl;
    MT::CTAtomic< bool > m_done;
};

/*-------------------------------------------------------------------------*/

void
PerformChromeDevToolsTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Chrome DevTools client TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "ChromeDevTools" );

    if ( !PluginsAvailable() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Chrome DevTools client TESTS skipped: the json codec or sha1-raw codec is not available" );
        return;
    }

    // The fake browser: discovery over HTTP and commands over a web socket on the same port
    TFakeChromeHttpHandlerFactory handlerFactory;
    WEB::CHttpServerSettings httpSettings;
    httpSettings.allowWebSocketUpgrades = true;
    httpSettings.processRequestsAsync = false;
    httpSettings.keepAliveConnections = false;
    WEB::CHTTPServer httpServer( &handlerFactory, &httpSettings );
    bool listening = httpServer.ListenOnPort( FAKE_CHROME_PORT );
    WEB::CWebSocketServer wsServer( httpServer );
    WEB::CJsonRpcServer rpcServer( wsServer );
    rpcServer.SetDialect( COM::JSONRPC_DIALECT_LENIENT );
    CFakeChromeCommands commands;
    const char* commandNames[] = { "Network.enable", "Network.getAllCookies", "Page.navigate", "Runtime.evaluate", "Target.createTarget",
                                   "Target.attachToTarget", "Network.getResponseBody", "Debug.sessionOf" };
    for ( UInt32 i=0; i<sizeof( commandNames ) / sizeof( commandNames[ 0 ] ); ++i )
        rpcServer.RegisterMethod( commandNames[ i ], &commands );

    GUCEF_TESTFW_TESTCASE( "Test: Discovery lists the browser and its targets" )
        try
        {
            ASSERT_TRUE( listening );

            WEB::CChromeDevToolsClient client;
            CChromeRecorder recorder;
            recorder.SubscribeTo( &client );

            WEB::SChromeDiscoveryResult result;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", FAKE_CHROME_PORT, result ) );
            ASSERT_TRUE( 1 == recorder.discoveryCompleted && 0 == recorder.discoveryFailed );
            ASSERT_TRUE( result.browser.browser == "Chrome/120.0.0.0" );
            ASSERT_TRUE( result.browser.protocolVersion == "1.3" );
            ASSERT_TRUE( result.browser.webSocketDebuggerUrl.EndsWith( "/devtools/browser/B1" ) );
            ASSERT_TRUE( 2 == result.targets.size() );
            if ( 2 == result.targets.size() )
            {
                ASSERT_TRUE( result.targets[ 0 ].id == "P1" && result.targets[ 0 ].type == "page" && result.targets[ 0 ].title == "First page" );
                ASSERT_TRUE( result.targets[ 0 ].url == "http://example.org/" );
                ASSERT_TRUE( result.targets[ 0 ].webSocketDebuggerUrl.EndsWith( "/devtools/page/P1" ) );
                ASSERT_TRUE( result.targets[ 1 ].type == "service_worker" );
            }

            // a second discovery with the same client works too
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", FAKE_CHROME_PORT, result ) );
            ASSERT_TRUE( 2 == recorder.discoveryCompleted );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Discovery of a browser that is not there fails" )
        try
        {
            WEB::CChromeDevToolsClient client;
            CChromeRecorder recorder;
            recorder.SubscribeTo( &client );

            WEB::SChromeDiscoveryResult result;
            ASSERT_FALSE( client.DiscoverTargetsAndWait( "127.0.0.1", NOTHING_LISTENS_PORT, result, 5000 ) );
            ASSERT_TRUE( 1 == recorder.discoveryFailed && 0 == recorder.discoveryCompleted );
            ASSERT_TRUE( result.targets.empty() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Connect, commands, results and events" )
        try
        {
            WEB::CChromeDevToolsClient client;
            CChromeRecorder recorder;
            recorder.SubscribeTo( &client );

            WEB::SChromeDiscoveryResult discovery;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", FAKE_CHROME_PORT, discovery ) );
            ASSERT_TRUE( !discovery.targets.empty() );
            if ( discovery.targets.empty() )
                throw std::runtime_error( "no targets discovered, the rest of the test cannot run" );
            ASSERT_TRUE( client.ConnectAndWait( discovery.targets[ 0 ].webSocketDebuggerUrl ) );
            ASSERT_TRUE( client.IsConnected() && 1 == recorder.connected );

            // commands without results
            ASSERT_TRUE( client.NetworkEnable() );

            // cookies, with their attributes
            std::vector< WEB::SChromeCookie > cookies;    // std::vector because CChromeDevToolsClient::GetAllCookies() takes one
            ASSERT_TRUE( client.GetAllCookies( cookies ) );
            ASSERT_TRUE( 2 == cookies.size() );
            if ( 2 == cookies.size() )
            {
                ASSERT_TRUE( cookies[ 0 ].name == "sessionid" && cookies[ 0 ].value == "abc123" );
                ASSERT_TRUE( cookies[ 0 ].domain == ".example.org" && cookies[ 0 ].path == "/" );
                ASSERT_TRUE( cookies[ 0 ].httpOnly && cookies[ 0 ].secure && !cookies[ 0 ].session );
                ASSERT_TRUE( cookies[ 0 ].expires > 1000000000.0 && cookies[ 0 ].sameSite == "Lax" );
                ASSERT_TRUE( cookies[ 1 ].name == "theme" && !cookies[ 1 ].httpOnly && !cookies[ 1 ].secure && cookies[ 1 ].session );
                ASSERT_TRUE( cookies[ 1 ].path == "/app" );
            }

            // navigating: the answer and then the event the browser sends about it
            CORE::CString frameId;
            ASSERT_TRUE( client.PageNavigate( "http://example.org/next", &frameId ) );
            ASSERT_TRUE( frameId == "F1" );
            UInt32 pumped = 0;
            while ( recorder.protocolEvents < 1 && pumped++ < 3000 )
            {
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( 1 == recorder.protocolEvents );
            if ( 1 == recorder.protocolEvents )
            {
                ASSERT_TRUE( recorder.events[ 0 ].method == "Page.loadEventFired" );
                ASSERT_TRUE( recorder.events[ 0 ].hasParams && recorder.events[ 0 ].params.GetAttributeValue( "timestamp" ).AsFloat64() > 12.0 );
                ASSERT_TRUE( recorder.events[ 0 ].sessionId.IsNULLOrEmpty() );
            }

            // a navigation that fails reports why
            ASSERT_FALSE( client.PageNavigate( "http://broken.invalid/" ) );

            // evaluating script: numbers, strings, objects, undefined and exceptions
            CORE::CDataNode value;
            CORE::CString exceptionText;
            ASSERT_TRUE( client.RuntimeEvaluate( "1+41", false, value, &exceptionText ) );
            ASSERT_TRUE( 42 == value.GetValue().AsInt32() );
            ASSERT_TRUE( client.RuntimeEvaluate( "'hello'", true, value ) );
            ASSERT_TRUE( value.GetValue().AsString() == "hello" );
            ASSERT_TRUE( client.RuntimeEvaluate( "({a:1,b:2})", false, value ) );
            ASSERT_TRUE( GUCEF_DATATYPE_OBJECT == value.GetNodeType() && 1 == value.GetAttributeValue( "a" ).AsInt32() && 2 == value.GetAttributeValue( "b" ).AsInt32() );
            ASSERT_TRUE( client.RuntimeEvaluate( "undefined", false, value ) );
            ASSERT_TRUE( GUCEF_DATATYPE_NIL == value.GetNodeType() );
            ASSERT_FALSE( client.RuntimeEvaluate( "throw new Error('boom')", false, value, &exceptionText ) );
            ASSERT_TRUE( exceptionText == "Error: boom" );

            // response bodies, and a request the browser does not know
            CORE::CString body;
            bool isBase64 = false;
            ASSERT_TRUE( client.GetResponseBody( "R1", body, isBase64 ) );
            ASSERT_TRUE( body == "aGk=" && isBase64 );
            ASSERT_FALSE( client.GetResponseBody( "unknown", body, isBase64 ) );

            // a command the browser does not know comes back as an error with its code
            CORE::CDataNode unusedResult;
            COM::SJsonRpcError error;
            ASSERT_FALSE( client.SendCommandAndWait( "Nonsense.command", GUCEF_NULL, unusedResult, &error ) );
            ASSERT_TRUE( COM::JSONRPC_ERROR_METHOD_NOT_FOUND == error.code );

            // targets and sessions: new tab, attach, then commands are directed at the session
            CORE::CString targetId;
            CORE::CString sessionId;
            ASSERT_TRUE( client.TargetCreate( "about:blank", targetId ) );
            ASSERT_TRUE( targetId == "T2" );
            ASSERT_TRUE( client.AttachToTarget( targetId, sessionId ) );
            ASSERT_TRUE( sessionId == "S9" );

            CORE::CDataNode sessionResult;
            ASSERT_TRUE( client.SendCommandAndWait( "Debug.sessionOf", GUCEF_NULL, sessionResult ) );
            ASSERT_TRUE( sessionResult.GetAttributeValue( "sessionId" ).AsString().IsNULLOrEmpty() );
            client.SetSessionId( sessionId );
            ASSERT_TRUE( client.SendCommandAndWait( "Debug.sessionOf", GUCEF_NULL, sessionResult ) );
            ASSERT_TRUE( sessionResult.GetAttributeValue( "sessionId" ).AsString() == "S9" );
            client.SetSessionId( CORE::CString::Empty );
            ASSERT_TRUE( client.SendCommandAndWait( "Debug.sessionOf", GUCEF_NULL, sessionResult ) );
            ASSERT_TRUE( sessionResult.GetAttributeValue( "sessionId" ).AsString().IsNULLOrEmpty() );

            // disconnecting
            client.Disconnect();
            ASSERT_FALSE( client.IsConnected() );
            ASSERT_TRUE( 1 == recorder.disconnected );
            ASSERT_FALSE( client.SendCommandAndWait( "Network.enable", GUCEF_NULL, unusedResult, &error ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The client runs on its pulse generator and only waits on the thread that drives it" )
        try
        {
            const UInt32 thisThreadId = MT::GetCurrentTaskID();
            CORE::PulseGeneratorPtr globalPulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

            // the pulse generator is given to the JSON-RPC client the commands go through
            {
                CORE::PulseGeneratorPtr localPulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
                WEB::CChromeDevToolsClient localClient( localPulseGenerator );
                ASSERT_TRUE( localClient.GetPulseGenerator() == localPulseGenerator );
                ASSERT_TRUE( localClient.GetRpcClient().GetPulseGenerator() == localPulseGenerator );
            }

            WEB::CChromeDevToolsClient client;
            CChromeRecorder recorder;
            recorder.SubscribeTo( &client );
            ASSERT_TRUE( client.GetPulseGenerator() == globalPulseGenerator );
            ASSERT_TRUE( client.GetRpcClient().GetPulseGenerator() == globalPulseGenerator );
            ASSERT_TRUE( globalPulseGenerator->GetPulseDriverThreadId() == thisThreadId );

            WEB::SChromeDiscoveryResult discovery;
            ASSERT_TRUE( client.DiscoverTargetsAndWait( "127.0.0.1", FAKE_CHROME_PORT, discovery ) );
            if ( discovery.targets.empty() )
                throw std::runtime_error( "no targets discovered, the rest of the test cannot run" );
            ASSERT_TRUE( client.ConnectAndWait( discovery.targets[ 0 ].webSocketDebuggerUrl ) );

            // another thread may not wait: that would pump this thread's pulse generator
            CForeignPulseCounter foreignPulses( globalPulseGenerator, thisThreadId );
            {
                CChromeWaitThread waiter( client, CChromeWaitThread::OPERATION_COMMAND, CORE::CString::Empty );
                ASSERT_TRUE( waiter.Run() );
                ASSERT_FALSE( waiter.result );
            }
            {
                CChromeWaitThread waiter( client, CChromeWaitThread::OPERATION_DISCOVERY, CORE::CString::Empty );
                ASSERT_TRUE( waiter.Run() );
                ASSERT_FALSE( waiter.result );
                ASSERT_TRUE( 1 == recorder.discoveryCompleted );
            }
            ASSERT_TRUE( client.IsConnected() );
            ASSERT_TRUE( client.NetworkEnable() );      // this thread still can

            client.Disconnect();
            ASSERT_FALSE( client.IsConnected() );
            UInt32 pumped = 0;
            while ( pumped++ < 100 )
            {
                globalPulseGenerator->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            {
                CChromeWaitThread waiter( client, CChromeWaitThread::OPERATION_CONNECT, discovery.targets[ 0 ].webSocketDebuggerUrl );
                ASSERT_TRUE( waiter.Run() );
                ASSERT_FALSE( waiter.result );
                ASSERT_FALSE( client.IsConnected() );
            }
            ASSERT_TRUE( 0 == foreignPulses.GetForeignPulses() );

            // while on this thread the connection is made again
            ASSERT_TRUE( client.ConnectAndWait( discovery.targets[ 0 ].webSocketDebuggerUrl ) );
            ASSERT_TRUE( client.NetworkEnable() );
            client.Disconnect();
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL Chrome DevTools client TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
