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

#ifndef GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#include "gucefWEB_CIHttpServerRequestHandler.h"
#define GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H
#endif /* GUCEF_WEB_CIHTTPSERVERREQUESTHANDLER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

#ifndef GUCEF_WEB_CSSEEVENTWRITER_H
#include "gucefWEB_CSseEventWriter.h"
#define GUCEF_WEB_CSSEEVENTWRITER_H
#endif /* GUCEF_WEB_CSSEEVENTWRITER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#include "gucefWEB_CWebSocketFrameCodec.h"
#define GUCEF_WEB_CWEBSOCKETFRAMECODEC_H
#endif /* GUCEF_WEB_CWEBSOCKETFRAMECODEC_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_TEST_SOCKETS_H
#include "gucef_test_sockets.h"
#define GUCEF_TEST_SOCKETS_H
#endif /* GUCEF_TEST_SOCKETS_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#include "TestHttpRawClient.h"
#define GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#endif /* GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H ? */

#include "TestHttpServerBody.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

/** compares text and logs both when it differs, so a failure tells what was received */
static bool
BodyTestSameText( const CORE::CString& actual, const CORE::CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpServerBody test: expected \"" << expected << "\" but got \"" << actual << "\"" );
    return false;
}

/** The bytes of a buffer as text, also when it is empty */
static CORE::CString
BodyTestText( const CORE::CDynamicBuffer& buffer )
{
    if ( 0 == buffer.GetDataSize() )
        return CORE::CString::Empty;
    return CORE::CString( static_cast< const char* >( buffer.GetConstBufferPtr() ), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** compares counts and logs both when they differ */
static bool
BodyTestSameCount( UInt32 actual, UInt32 expected, const char* what )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpServerBody test: expected " << expected << " " << what << " but got " << actual );
    return false;
}

/*-------------------------------------------------------------------------*/

/** The requests the router handler of the server under test was given */
class CBodyRequestRecorder
{
    public:

    CBodyRequestRecorder( void ) : m_lock(), m_methods(), m_uris(), m_bodies() {}

    void Record( const WEB::CHttpRequestData& request )
    {
        MT::CScopeMutex lock( m_lock );
        m_methods.push_back( request.requestType );
        m_uris.push_back( request.requestUri );
        m_bodies.push_back( BodyTestText( request.content ) );
    }

    UInt32 GetCount( void )
    {
        MT::CScopeMutex lock( m_lock );
        return (UInt32) m_bodies.size();
    }

    CORE::CString GetBody( UInt32 index )
    {
        MT::CScopeMutex lock( m_lock );
        return index < m_bodies.size() ? m_bodies[ index ] : CORE::CString( "<none>" );
    }

    CORE::CString GetUri( UInt32 index )
    {
        MT::CScopeMutex lock( m_lock );
        return index < m_uris.size() ? m_uris[ index ] : CORE::CString( "<none>" );
    }

    void Clear( void )
    {
        MT::CScopeMutex lock( m_lock );
        m_methods.clear();
        m_uris.clear();
        m_bodies.clear();
    }

    private:

    MT::CMutex m_lock;
    CORE::CString::StringVector m_methods;
    CORE::CString::StringVector m_uris;
    CORE::CString::StringVector m_bodies;
};

/*-------------------------------------------------------------------------*/

/** Answers every router request with 200 and the text "got <uri> <body>" */
class CBodyRecordingHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CBodyRecordingHandler( void ) : WEB::CIHttpServerRequestHandler(), m_recorder( GUCEF_NULL ) {}
    virtual ~CBodyRecordingHandler() {}

    void SetRecorder( CBodyRequestRecorder* recorder ) { m_recorder = recorder; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != m_recorder )
            m_recorder->Record( request );
        CORE::CString text = "got " + request.requestUri + " " + BodyTestText( request.content );
        response.statusCode = 200;
        response.contentType = "text/plain";
        response.content.CopyFrom( text.Length(), text.C_String() );
        return true;
    }

    virtual bool OnRead( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnReadMetaData( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnUpdate( const WEB::CHttpRequestData&, bool, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool OnCreate( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE { return OnRequest( request, response ); }
    virtual bool OnDelete( const WEB::CHttpRequestData&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool SetRouterController( WEB::CIHTTPServerRouterController* ) GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual WEB::CIHTTPServerRouterController* GetRouterController( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    CBodyRequestRecorder* m_recorder;
};

/*-------------------------------------------------------------------------*/

class CBodyRecordingHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CBodyRecordingHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CBodyRecordingHandler, MT::CMutex > TBase;

    explicit CBodyRecordingHandlerFactory( CBodyRequestRecorder* recorder ) : TBase(), m_recorder( recorder ) {}
    virtual ~CBodyRecordingHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CBodyRecordingHandler* handler = static_cast< CBodyRecordingHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetRecorder( m_recorder );
        return product;
    }

    private:

    CBodyRecordingHandlerFactory( const CBodyRecordingHandlerFactory& src );
    CBodyRecordingHandlerFactory& operator=( const CBodyRecordingHandlerFactory& src );

    CBodyRequestRecorder* m_recorder;
};

/*-------------------------------------------------------------------------*/

/** A HTTP server on the loopback interface and the application's pulse generator, answering with the recording handler */
class CBodyTestServer
{
    public:

    explicit CBodyTestServer( const WEB::CHttpServerSettings* extraSettings = GUCEF_NULL )
        : m_recorder()
        , m_factory( &m_recorder )
        , m_server( GUCEF_NULL )
    {
        WEB::CHttpServerSettings settings;
        if ( GUCEF_NULL != extraSettings )
            settings = *extraSettings;
        settings.bindAddress = "127.0.0.1";
        m_server = GUCEF_NEW WEB::CHTTPServer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), &m_factory, &settings );
    }

    ~CBodyTestServer()
    {
        m_server->Close();
        GUCEF_DELETE m_server;
    }

    bool Start( void ) { return m_server->ListenOnPort( 0 ); }
    UInt16 GetPort( void ) const { return m_server->GetBoundPort(); }
    WEB::CHTTPServer& GetServer( void ) { return *m_server; }
    CBodyRequestRecorder& GetRecorder( void ) { return m_recorder; }

    private:

    CBodyRequestRecorder m_recorder;
    CBodyRecordingHandlerFactory m_factory;
    WEB::CHTTPServer* m_server;
};

/*-------------------------------------------------------------------------*/

static CORE::CString
PostRequest( const CORE::CString& uri, const CORE::CString& body, const CORE::CString& extraHeaders = CORE::CString::Empty )
{
    return "POST " + uri + " HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: text/plain\r\nContent-Length: " + CORE::ToString( body.Length() ) + "\r\n" + extraHeaders + "\r\n" + body;
}

/*-------------------------------------------------------------------------*/

static bool
EnsureHashCodecPluginLoadedForBodyTests( void )
{
    if ( WEB::CWebSocketHandshake::IsAvailable() )
        return true;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH";
    CORE::CString secondPluginPath = "$MODULEDIR$/codecspluginSTBRUMMEHASH_d";
    #endif

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( !pluginControl.AddPluginFromDir( firstPluginPath, "test", true ) )
        pluginControl.AddPluginFromDir( secondPluginPath, "test", true );
    return WEB::CWebSocketHandshake::IsAvailable();
}

/*-------------------------------------------------------------------------*/

/** Counts the text messages a web socket server received */
class CBodyWsRecorder : public CORE::CObserver
{
    public:

    CBodyWsRecorder( void ) : CORE::CObserver(), texts() {}

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( eventid == WEB::CWebSocketServer::TextMessageEvent )
            texts.push_back( static_cast< WEB::TWebSocketServerTextMessageEventData* >( eventdata )->GetData().text );
    }

    CORE::CString::StringVector texts;
};

/*-------------------------------------------------------------------------*/

static void
PerformRequestIntakeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A keep-alive GET in sync mode is answered with exactly one response" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( "GET /one HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n" ) );
            ASSERT_TRUE( client.WaitForResponses( 1 ) );
            TestHttpPumpFor( 300 );
            ASSERT_TRUE( BodyTestSameCount( client.CountStatusLines(), 1, "status lines" ) );
            ASSERT_TRUE( BodyTestSameCount( server.GetRecorder().GetCount(), 1, "requests at the handler" ) );

            // the connection stays usable for the next request, which is also answered once
            client.ClearReceived();
            ASSERT_TRUE( client.Send( "GET /two HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n" ) );
            ASSERT_TRUE( client.WaitForResponses( 1 ) );
            TestHttpPumpFor( 300 );
            ASSERT_TRUE( BodyTestSameCount( client.CountStatusLines(), 1, "status lines" ) );
            ASSERT_FALSE( client.IsClosed() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A POST whose headers and body arrive in two writes gives the handler the whole body" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( "POST /post HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: text/plain\r\nContent-Length: 11\r\n\r\n", 50 ) );
            ASSERT_TRUE( client.Send( "hello world" ) );
            ASSERT_TRUE( client.WaitForResponses( 1 ) );
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( BodyTestSameCount( server.GetRecorder().GetCount(), 1, "requests at the handler" ) );
            ASSERT_TRUE( BodyTestSameText( server.GetRecorder().GetBody( 0 ), "hello world" ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && 200 == responses[ 0 ].status );
            ASSERT_TRUE( 1 == responses.size() && BodyTestSameText( responses[ 0 ].body, "got /post hello world" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A POST split in the middle of its header block, and one sent byte by byte, reach the handler whole" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                CORE::CString request = PostRequest( "/split", "split body" );
                ASSERT_TRUE( client.Send( request.SubstrToIndex( 30, true ), 50 ) );
                ASSERT_TRUE( client.Send( request.CutChars( 30, true ) ) );
                ASSERT_TRUE( client.WaitForResponses( 1 ) );
                ASSERT_TRUE( 1 == server.GetRecorder().GetCount() && BodyTestSameText( server.GetRecorder().GetBody( 0 ), "split body" ) );
            }
            server.GetRecorder().Clear();
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                CORE::CString request = PostRequest( "/bytes", "byte by byte" );
                for ( UInt32 i=0; i<request.Length(); ++i )
                    client.SendBytes( request.C_String() + i, 1, 3 );
                ASSERT_TRUE( client.WaitForResponses( 1 ) );
                TestHttpPumpFor( 100 );
                ASSERT_TRUE( 1 == server.GetRecorder().GetCount() && BodyTestSameText( server.GetRecorder().GetBody( 0 ), "byte by byte" ) );
                ASSERT_TRUE( BodyTestSameCount( client.CountStatusLines(), 1, "status lines" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Two pipelined requests in one write get two responses in order" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( PostRequest( "/first", "1st" ) + PostRequest( "/second", "2nd" ) ) );
            ASSERT_TRUE( client.WaitForResponses( 2 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 2 == client.GetResponses( responses ) );
            if ( 2 == responses.size() )
            {
                ASSERT_TRUE( BodyTestSameText( responses[ 0 ].body, "got /first 1st" ) );
                ASSERT_TRUE( BodyTestSameText( responses[ 1 ].body, "got /second 2nd" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The log of a server which takes in requests holds no identifier of the path, query or credential of them" )
        try
        {
            TESTFW::CLogTextCapture capture;
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );

            // a request that is handled, one that is malformed and one with a body over the limit
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( "GET /trader/v1/accounts/ABCDEF0123456789ABCDEF0123456789/transactions?token=SECRETQUERY HTTP/1.1\r\nHost: 127.0.0.1\r\n"
                                      "Authorization: Bearer SECRETBEARER\r\nCookie: sid=SECRETCOOKIE\r\nX-Api-Key: SECRETAPIKEY\r\n\r\n" ) );
            ASSERT_TRUE( client.WaitForResponses( 1 ) );
            TestHttpPumpFor( 200 );

            CTestHttpRawClient badClient;
            ASSERT_TRUE( badClient.Connect( server.GetPort() ) );
            ASSERT_TRUE( badClient.Send( "GET /trader/v1/accounts/ABCDEF0123456789ABCDEF0123456789/transactions?token=SECRETQUERY HTTP/1.1\r\nHost: 127.0.0.1\r\n"
                                         "Authorization: Bearer SECRETBEARER\r\nthis line has no colon SECRETCOOKIE\r\n\r\n" ) );
            ASSERT_TRUE( badClient.WaitForResponses( 1 ) );
            TestHttpPumpFor( 200 );

            CORE::CString logged = capture.GetText();
            ASSERT_TRUE( logged.HasSubstr( "ABCDEF0123456789ABCDEF0123456789", true ) < 0 );
            ASSERT_TRUE( logged.HasSubstr( "SECRET", true ) < 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A Content-Length over the limit gets 413 before any body byte is sent, and the connection closes" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( "POST /big HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 999999999999\r\n\r\n" ) );
            ASSERT_TRUE( client.WaitForResponses( 1, 2000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && 413 == responses[ 0 ].status );
            ASSERT_TRUE( client.WaitForClose( 2000 ) );
            ASSERT_TRUE( 0 == server.GetRecorder().GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Expect: 100-continue gets 100 Continue, then the response once the body is in" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( "POST /continue HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 4\r\nExpect: 100-continue\r\n\r\n" ) );
            ASSERT_TRUE( client.WaitForResponses( 1, 2000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && 100 == responses[ 0 ].status );
            ASSERT_TRUE( client.Send( "data" ) );
            ASSERT_TRUE( client.WaitForResponses( 2, 2000 ) );
            ASSERT_TRUE( 2 == client.GetResponses( responses ) && 200 == responses[ 1 ].status );
            ASSERT_TRUE( 2 == responses.size() && BodyTestSameText( responses[ 1 ].body, "got /continue data" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Transfer-Encoding: chunked gets 501, two different Content-Length values get 400" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /chunked HTTP/1.1\r\nHost: 127.0.0.1\r\nTransfer-Encoding: chunked\r\n\r\n4\r\ndata\r\n0\r\n\r\n" ) );
                ASSERT_TRUE( client.WaitForResponses( 1, 2000 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 501 == responses[ 0 ].status );
                ASSERT_TRUE( client.WaitForClose( 2000 ) );
            }
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /two HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 4\r\nContent-Length: 40\r\n\r\ndata" ) );
                ASSERT_TRUE( client.WaitForResponses( 1, 2000 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 400 == responses[ 0 ].status );
                ASSERT_TRUE( client.WaitForClose( 2000 ) );
            }
            ASSERT_TRUE( 0 == server.GetRecorder().GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Bytes behind a web socket upgrade in the same write reach the web socket handler" )
        try
        {
            GUCEF_TESTFW_SKIP_IF( !EnsureHashCodecPluginLoadedForBodyTests(), "the sha1 codec plugin for the web socket handshake is not available" );

            WEB::CHttpServerSettings settings;
            settings.allowWebSocketUpgrades = true;
            CBodyTestServer server( &settings );
            WEB::CWebSocketServer webSocketServer( server.GetServer() );
            CBodyWsRecorder recorder;
            recorder.SubscribeTo( &webSocketServer );
            ASSERT_TRUE( server.Start() );

            CORE::CDynamicBuffer frame;
            ASSERT_TRUE( WEB::CWebSocketFrameCodec::EncodeFrame( true, WEB::CWebSocketFrameCodec::OPCODE_TEXT, "early", 5, true, frame ) );
            CORE::CString upgrade = "GET /ws HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                                    "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n";
            CORE::CDynamicBuffer both;
            both.Append( upgrade.C_String(), upgrade.Length() );
            both.Append( frame.GetConstBufferPtr(), frame.GetDataSize() );

            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.SendBytes( both.GetConstBufferPtr(), both.GetDataSize() ) );
            ASSERT_TRUE( client.WaitForText( "\r\n\r\n", 2000 ) );
            TestHttpPumpFor( 200 );
            ASSERT_TRUE( 0 == client.GetReceived().HasSubstr( "HTTP/1.1 101", 0, true ) );
            ASSERT_TRUE( 1 == recorder.texts.size() && BodyTestSameText( recorder.texts[ 0 ], "early" ) );
            recorder.UnsubscribeAllFromObserver();
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** A takeover handler for the tests: answers at once, later from a timer, as a stream, or refuses on a header */
class CTestTakeoverHandler : public WEB::CIHttpRequestTakeoverHandler ,
                             public CORE::CObserver
{
    public:

    enum EMode
    {
        ANSWER_NOW      ,   /**< answers from inside OnHttpRequest() */
        ANSWER_TIMER    ,   /**< answers from a timer after 'delayMs' */
        ANSWER_HELD     ,   /**< keeps the exchange; the test answers or streams through it */
        STREAM_SSE          /**< begins an SSE stream at once and keeps the exchange */
    };

    explicit CTestTakeoverHandler( EMode handlerMode )
        : WEB::CIHttpRequestTakeoverHandler()
        , CORE::CObserver()
        , mode( handlerMode )
        , delayMs( 100 )
        , headerCalls( 0 )
        , requestCalls( 0 )
        , closedCalls( 0 )
        , sendsAfterClose( 0 )
        , lastBody()
        , lastUri()
        , exchanges()
        , m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), 100 )
    {
        typedef CORE::CTEventHandlerFunctor< CTestTakeoverHandler > TEventCallback;
        TEventCallback callback( this, &CTestTakeoverHandler::OnTimer );
        SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, callback );
    }

    virtual ~CTestTakeoverHandler()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CTestTakeoverHandler";
        return classTypeName;
    }

    virtual EHeaderDecision OnHttpRequestHeaders( WEB::CHTTPServer& server                 ,
                                                  const WEB::CHttpRequestData& request     ,
                                                  const WEB::CHttpServerExchange& exchange ,
                                                  WEB::CHttpResponseData& response         ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++headerCalls;
        if ( request.otherHeaders.end() != request.otherHeaders.find( "x-reject" ) )
        {
            response.statusCode = 403;
            return TAKEOVER_REJECT;
        }
        return TAKEOVER_CONTINUE;
    }

    virtual void OnHttpRequest( WEB::CHTTPServer& server             ,
                                const WEB::CHttpRequestData& request ,
                                WEB::CHttpServerExchange& exchange   ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++requestCalls;
        lastBody = BodyTestText( request.content );
        lastUri = request.requestUri;
        exchanges.push_back( exchange.GetSharedPtr() );
        switch ( mode )
        {
            case ANSWER_NOW:
            {
                Answer( exchange, "taken " + lastBody );
                break;
            }
            case ANSWER_TIMER:
            {
                m_timer.SetInterval( delayMs );
                m_timer.SetEnabled( true );
                break;
            }
            case STREAM_SSE:
            {
                WEB::CHttpResponseData head;
                head.statusCode = 200;
                head.contentType = "text/event-stream";
                head.cacheability = "no-cache";
                exchange.BeginStream( head );
                break;
            }
            default:
                break;
        }
    }

    virtual void OnHttpExchangeClosed( WEB::CHTTPServer& server, WEB::CHttpServerExchange& exchange ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++closedCalls;
        // a send from inside the notification fails as well
        if ( exchange.SendStreamData( "x", 1 ) )
            ++sendsAfterClose;
    }

    static bool Answer( WEB::CHttpServerExchange& exchange, const CORE::CString& text )
    {
        WEB::CHttpResponseData response;
        response.statusCode = 200;
        response.contentType = "text/plain";
        response.content.CopyFrom( text.Length(), text.C_String() );
        return exchange.SendResponse( response );
    }

    EMode mode;
    UInt32 delayMs;
    UInt32 headerCalls;
    UInt32 requestCalls;
    UInt32 closedCalls;
    UInt32 sendsAfterClose;
    CORE::CString lastBody;
    CORE::CString lastUri;
    GUCEF::vector< WEB::CHttpServerExchangePtr > exchanges;

    private:

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        m_timer.SetEnabled( false );
        for ( size_t i=0; i<exchanges.size(); ++i )
            if ( exchanges[ i ]->IsOpen() )
                Answer( *exchanges[ i ], "late " + lastBody );
    }

    CORE::CTimer m_timer;
};

/*-------------------------------------------------------------------------*/

static void
PerformTimeoutTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A header block that trickles in beyond headerTimeoutMs gets 408 and a close" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.headerTimeoutMs = 300;
            CBodyTestServer server( &settings );
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            const char* trickle = "GET /slow HTTP/1.1\r\nHost: 127.0.0.1\r\nX-Slow: ";
            for ( UInt32 i=0; i<(UInt32) strlen( trickle ) && !client.IsClosed(); ++i )
                client.SendBytes( trickle + i, 1, 30 );
            ASSERT_TRUE( client.WaitForClose( 2000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && 408 == responses[ 0 ].status );
            ASSERT_TRUE( 0 == server.GetRecorder().GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A body that does not come in within bodyTimeoutMs gets 408, an idle connection closes without a response" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.bodyTimeoutMs = 300;
            settings.idleTimeoutMs = 300;
            CBodyTestServer server( &settings );
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /x HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 10\r\n\r\nabc" ) );
                ASSERT_TRUE( client.WaitForClose( 2000 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 408 == responses[ 0 ].status );
            }
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.WaitForClose( 2000 ) );
                ASSERT_TRUE( client.GetReceived().IsNULLOrEmpty() );
            }
            ASSERT_TRUE( 0 == server.GetRecorder().GetCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformTakeoverTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: A takeover route answers late from a timer; the response arrives and keep-alive continues" )
        try
        {
            CBodyTestServer server;
            CTestTakeoverHandler handler( CTestTakeoverHandler::ANSWER_TIMER );
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/taken", &handler, 0 ) );
            ASSERT_FALSE( server.GetServer().RegisterRequestTakeoverHandler( "/taken", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );

            // a request behind the taken one waits until the taken one is answered
            ASSERT_TRUE( client.Send( PostRequest( "/taken", "one" ) + PostRequest( "/router", "two" ) ) );
            TestHttpPumpFor( 30 );
            ASSERT_TRUE( 0 == client.GetCompleteResponseCount() );
            ASSERT_TRUE( 1 == handler.requestCalls && 1 == server.GetServer().GetOpenExchangeCount() );
            ASSERT_TRUE( client.WaitForResponses( 2, 3000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 2 == client.GetResponses( responses ) );
            if ( 2 == responses.size() )
            {
                ASSERT_TRUE( BodyTestSameText( responses[ 0 ].body, "late one" ) );
                ASSERT_TRUE( BodyTestSameText( responses[ 1 ].body, "got /router two" ) );
                ASSERT_TRUE( BodyTestSameText( responses[ 0 ].Header( "Connection" ), "keep-alive" ) );
            }
            ASSERT_FALSE( client.IsClosed() );
            ASSERT_TRUE( 0 == server.GetServer().GetOpenExchangeCount() );

            // the same connection is taken over again for the next request
            client.ClearReceived();
            ASSERT_TRUE( client.Send( PostRequest( "/taken", "three" ) ) );
            ASSERT_TRUE( client.WaitForResponses( 1, 3000 ) );
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && BodyTestSameText( responses[ 0 ].body, "late three" ) );
            ASSERT_TRUE( 2 == handler.requestCalls && 2 == handler.headerCalls && 0 == handler.closedCalls );
            ASSERT_TRUE( handler.exchanges.size() == 2 && handler.exchanges[ 0 ]->GetExchangeId() != handler.exchanges[ 1 ]->GetExchangeId() );
            ASSERT_FALSE( handler.exchanges[ 0 ]->SendResponse( WEB::CHttpResponseData() ) );
            ASSERT_TRUE( server.GetServer().UnregisterRequestTakeoverHandler( "/taken" ) );
            ASSERT_FALSE( server.GetServer().UnregisterRequestTakeoverHandler( "/taken" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A takeover answer from inside the callback, a refusal before the body, and a route over the router and the upgrade" )
        try
        {
            CBodyTestServer server;
            CTestTakeoverHandler handler( CTestTakeoverHandler::ANSWER_NOW );
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/now", &handler, 16 ) );
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( PostRequest( "/now", "a" ) + PostRequest( "/now?query=1", "b" ) + PostRequest( "/now/below", "c" ) ) );
                ASSERT_TRUE( client.WaitForResponses( 3 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 3 == client.GetResponses( responses ) );
                if ( 3 == responses.size() )
                {
                    ASSERT_TRUE( BodyTestSameText( responses[ 0 ].body, "taken a" ) );
                    ASSERT_TRUE( BodyTestSameText( responses[ 1 ].body, "taken b" ) );
                    ASSERT_TRUE( BodyTestSameText( responses[ 2 ].body, "got /now/below c" ) );     // not the exact path: the router
                }
            }
            {
                // refused on its headers: the body is never read and the handler never sees the request
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /now HTTP/1.1\r\nHost: 127.0.0.1\r\nX-Reject: yes\r\nContent-Length: 5\r\n\r\n" ) );
                ASSERT_TRUE( client.WaitForResponses( 1 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 403 == responses[ 0 ].status );
                ASSERT_TRUE( client.WaitForClose( 2000 ) );
            }
            {
                // the body limit of the route, after the header checks of the handler
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /now HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: 17\r\n\r\n" ) );
                ASSERT_TRUE( client.WaitForResponses( 1 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 413 == responses[ 0 ].status );
            }
            {
                // a web socket upgrade on the path goes to the handler too
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "GET /now HTTP/1.1\r\nHost: 127.0.0.1\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                                          "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\nSec-WebSocket-Version: 13\r\n\r\n" ) );
                ASSERT_TRUE( client.WaitForResponses( 1 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 200 == responses[ 0 ].status );
            }
            ASSERT_TRUE( BodyTestSameCount( handler.requestCalls, 3, "taken requests" ) );
            ASSERT_TRUE( BodyTestSameCount( handler.headerCalls, 5, "header checks" ) );
            ASSERT_TRUE( 0 == handler.closedCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A streamed response of three SSE events and an end: the client sees three events and the close" )
        try
        {
            CBodyTestServer server;
            CTestTakeoverHandler handler( CTestTakeoverHandler::STREAM_SSE );
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/events", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( PostRequest( "/events", "go" ) ) );
            ASSERT_TRUE( client.WaitForText( "\r\n\r\n" ) );
            ASSERT_TRUE( 1 == handler.exchanges.size() );
            if ( 1 == handler.exchanges.size() )
            {
                WEB::CHttpServerExchangePtr exchange = handler.exchanges[ 0 ];
                ASSERT_TRUE( exchange->IsStreaming() );
                ASSERT_FALSE( CTestTakeoverHandler::Answer( *exchange, "too late" ) );     // a stream is no whole response
                CORE::CDynamicBuffer events;
                ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( events, "{\"n\":1}" ) );
                ASSERT_TRUE( exchange->SendStreamData( events ) );
                events.Clear();
                WEB::CSseEventWriter::AppendComment( events );
                ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( events, "{\"n\":2}" ) );
                ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( events, "{\"n\":3}" ) );
                ASSERT_TRUE( exchange->SendStreamData( events ) );
                TestHttpPumpFor( 50 );
                ASSERT_FALSE( client.IsClosed() );
                ASSERT_TRUE( exchange->EndStream() );
                ASSERT_FALSE( exchange->EndStream() );
                ASSERT_FALSE( exchange->SendStreamData( "x", 1 ) );
                ASSERT_TRUE( client.WaitForClose( 2000 ) );

                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 200 == responses[ 0 ].status && responses[ 0 ].complete );
                if ( 1 == responses.size() )
                {
                    ASSERT_TRUE( BodyTestSameText( responses[ 0 ].Header( "Content-Type" ), "text/event-stream" ) );
                    ASSERT_TRUE( BodyTestSameText( responses[ 0 ].Header( "Connection" ), "close" ) );
                    ASSERT_FALSE( responses[ 0 ].HasHeader( "Content-Length" ) );
                    ASSERT_FALSE( responses[ 0 ].HasHeader( "Transfer-Encoding" ) );
                    CORE::CString::StringVector data;
                    UInt32 comments = 0;
                    ASSERT_TRUE( 3 == TestHttpParseSseEvents( responses[ 0 ].body, data, &comments ) && 1 == comments );
                    if ( 3 == data.size() )
                    {
                        ASSERT_TRUE( BodyTestSameText( data[ 0 ], "{\"n\":1}" ) );
                        ASSERT_TRUE( BodyTestSameText( data[ 2 ], "{\"n\":3}" ) );
                    }
                }
            }
            ASSERT_TRUE( 0 == handler.closedCalls );
            ASSERT_TRUE( 0 == server.GetServer().GetOpenExchangeCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A client that closes during a stream raises OnHttpExchangeClosed() once, and a send after that returns false" )
        try
        {
            CBodyTestServer server;
            CTestTakeoverHandler handler( CTestTakeoverHandler::STREAM_SSE );
            handler.sendsAfterClose = 0;
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/events", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( PostRequest( "/events", "go" ) ) );
                ASSERT_TRUE( client.WaitForText( "\r\n\r\n" ) );
                client.Close();
            }
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 1 == handler.closedCalls && 0 == handler.sendsAfterClose );
            ASSERT_TRUE( 1 == handler.exchanges.size() );
            if ( 1 == handler.exchanges.size() )
            {
                ASSERT_FALSE( handler.exchanges[ 0 ]->IsOpen() );
                ASSERT_TRUE( WEB::CHttpServerExchange::EXCHANGE_CLOSED == handler.exchanges[ 0 ]->GetState() );
                ASSERT_FALSE( handler.exchanges[ 0 ]->SendStreamData( "late", 4 ) );
                ASSERT_FALSE( handler.exchanges[ 0 ]->EndStream() );
            }
            TestHttpPumpFor( 50 );
            ASSERT_TRUE( 1 == handler.closedCalls );
            ASSERT_TRUE( 0 == server.GetServer().GetOpenExchangeCount() );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: An exchange held across a disconnect and a new connection on the same index never writes to the new one" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.maxClientConnections = 1;      // one connection object: the next client gets the same index
            CBodyTestServer server( &settings );
            CTestTakeoverHandler handler( CTestTakeoverHandler::ANSWER_HELD );
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/held", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient first;
                ASSERT_TRUE( first.Connect( server.GetPort() ) );
                ASSERT_TRUE( first.Send( PostRequest( "/held", "first" ) ) );
                TestHttpPumpFor( 30 );
                first.Close();
            }
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 1 == handler.closedCalls );

            CTestHttpRawClient second;
            ASSERT_TRUE( second.Connect( server.GetPort() ) );
            ASSERT_TRUE( second.Send( PostRequest( "/held", "second" ) ) );
            TestHttpPumpFor( 30 );
            ASSERT_TRUE( 2 == handler.exchanges.size() );
            if ( 2 == handler.exchanges.size() )
            {
                ASSERT_TRUE( handler.exchanges[ 0 ]->GetExchangeId() != handler.exchanges[ 1 ]->GetExchangeId() );
                ASSERT_FALSE( CTestTakeoverHandler::Answer( *handler.exchanges[ 0 ], "for the first client" ) );
                TestHttpPumpFor( 50 );
                ASSERT_TRUE( second.GetReceived().IsNULLOrEmpty() );
                ASSERT_TRUE( CTestTakeoverHandler::Answer( *handler.exchanges[ 1 ], "for the second client" ) );
                ASSERT_TRUE( second.WaitForResponses( 1 ) );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == second.GetResponses( responses ) && BodyTestSameText( responses[ 0 ].body, "for the second client" ) );
            }
            ASSERT_TRUE( 1 == handler.closedCalls );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Unregistering a route closes its open exchanges without a callback" )
        try
        {
            CBodyTestServer server;
            CTestTakeoverHandler handler( CTestTakeoverHandler::STREAM_SSE );
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/events", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( PostRequest( "/events", "go" ) ) );
            ASSERT_TRUE( client.WaitForText( "\r\n\r\n" ) );
            ASSERT_TRUE( 1 == server.GetServer().GetOpenExchangeCount() );
            ASSERT_TRUE( server.GetServer().UnregisterRequestTakeoverHandler( "/events" ) );
            ASSERT_TRUE( client.WaitForClose( 2000 ) );
            ASSERT_TRUE( 0 == handler.closedCalls );
            ASSERT_TRUE( 1 == handler.exchanges.size() && !handler.exchanges[ 0 ]->IsOpen() );
            ASSERT_TRUE( 0 == server.GetServer().GetOpenExchangeCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #ifdef GUCEF_WEB_TEST_RAW_POSIX_SOCKETS
    GUCEF_TESTFW_TESTCASE( "Test: A stream to a client that does not read is closed once its backlog passes maxStreamBacklogBytes" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.maxStreamBacklogBytes = 256 * 1024;
            CBodyTestServer server( &settings );
            CTestTakeoverHandler handler( CTestTakeoverHandler::STREAM_SSE );
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/events", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );

            CTestHttpResettingClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( PostRequest( "/events", "go" ), 50 ) );
            ASSERT_TRUE( 1 == handler.exchanges.size() );
            if ( 1 == handler.exchanges.size() )
            {
                WEB::CHttpServerExchangePtr exchange = handler.exchanges[ 0 ];
                CORE::CDynamicBuffer chunk;
                chunk.SetDataSize( 16 * 1024 );
                memset( chunk.GetBufferPtr(), 'z', chunk.GetDataSize() );
                UInt32 sent = 0;
                bool refused = false;
                for ( UInt32 i=0; i<4096 && !refused; ++i )         // up to 64 MiB, far beyond what the socket buffers hold
                {
                    if ( exchange->SendStreamData( chunk ) )
                        ++sent;
                    else
                        refused = true;
                }
                ASSERT_TRUE( refused );
                ASSERT_TRUE( exchange->GetStreamBacklogBytes() <= settings.maxStreamBacklogBytes );
                TestHttpPumpFor( 100 );
                ASSERT_TRUE( 1 == handler.closedCalls );
                ASSERT_FALSE( exchange->IsOpen() );
            }
            ASSERT_TRUE( 0 == server.GetServer().GetOpenExchangeCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif /* GUCEF_WEB_TEST_RAW_POSIX_SOCKETS ? */
}

/*-------------------------------------------------------------------------*/

static void
PerformSseEventWriterTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: SSE events: multi-line data, empty data, event names, comments" )
        try
        {
            CORE::CDynamicBuffer out;
            ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( out, "{\"a\":1}" ) );
            ASSERT_TRUE( BodyTestSameText( BodyTestText( out ), "data: {\"a\":1}\n\n" ) );

            out.Clear();
            ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( out, "one\ntwo\r\nthree\rfour" ) );
            ASSERT_TRUE( BodyTestSameText( BodyTestText( out ), "data: one\ndata: two\ndata: three\ndata: four\n\n" ) );

            out.Clear();
            ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( out, CORE::CString::Empty ) );
            ASSERT_TRUE( BodyTestSameText( BodyTestText( out ), "data: \n\n" ) );

            // a line break at the end of the text is an empty line of data, never the end of the event
            out.Clear();
            ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( out, "x\n\ny" ) );
            ASSERT_TRUE( BodyTestSameText( BodyTestText( out ), "data: x\ndata: \ndata: y\n\n" ) );

            out.Clear();
            ASSERT_TRUE( WEB::CSseEventWriter::AppendEvent( out, "m", "message" ) );
            ASSERT_TRUE( BodyTestSameText( BodyTestText( out ), "event: message\ndata: m\n\n" ) );
            out.Clear();
            ASSERT_FALSE( WEB::CSseEventWriter::AppendEvent( out, "m", "bad\nname" ) );
            ASSERT_FALSE( WEB::CSseEventWriter::AppendEvent( out, "m", "bad:name" ) );
            ASSERT_TRUE( 0 == out.GetDataSize() );

            WEB::CSseEventWriter::AppendComment( out );
            WEB::CSseEventWriter::AppendComment( out, "keep\nalive" );
            ASSERT_TRUE( BodyTestSameText( BodyTestText( out ), ":\n: keep\n: alive\n" ) );

            // the parser of the tests reads back what the writer wrote
            out.Clear();
            WEB::CSseEventWriter::AppendEvent( out, "first" );
            WEB::CSseEventWriter::AppendComment( out );
            WEB::CSseEventWriter::AppendEvent( out, "second\nline" );
            CORE::CString::StringVector events;
            ASSERT_TRUE( 2 == TestHttpParseSseEvents( BodyTestText( out ), events ) );
            ASSERT_TRUE( 2 == events.size() && BodyTestSameText( events[ 1 ], "second\nline" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** Answers with a header value that holds a line break, as a handler that copies client text into a header would */
class CSplittingTakeoverHandler : public WEB::CIHttpRequestTakeoverHandler
{
    public:

    CSplittingTakeoverHandler( void ) : WEB::CIHttpRequestTakeoverHandler(), refusedSends( 0 ) {}

    virtual EHeaderDecision OnHttpRequestHeaders( WEB::CHTTPServer&, const WEB::CHttpRequestData&, const WEB::CHttpServerExchange&, WEB::CHttpResponseData& ) GUCEF_VIRTUAL_OVERRIDE
    {
        return TAKEOVER_CONTINUE;
    }

    virtual void OnHttpRequest( WEB::CHTTPServer& server, const WEB::CHttpRequestData& request, WEB::CHttpServerExchange& exchange ) GUCEF_VIRTUAL_OVERRIDE
    {
        WEB::CHttpResponseData response;
        response.statusCode = 200;
        response.otherHeaders[ "X-Echo" ] = "a\r\nSet-Cookie: stolen=1";
        if ( !exchange.SendResponse( response ) )
            ++refusedSends;
    }

    virtual void OnHttpExchangeClosed( WEB::CHTTPServer&, WEB::CHttpServerExchange& ) GUCEF_VIRTUAL_OVERRIDE {}

    UInt32 refusedSends;
};

/*-------------------------------------------------------------------------*/

/** Sends one request on a new connection and gives the status of the first response, 0 when none came */
static Int32
StatusOfRequest( UInt16 port, const CORE::CString& request, bool* closedAfter = GUCEF_NULL )
{
    CTestHttpRawClient client;
    if ( !client.Connect( port ) )
        return 0;
    client.Send( request );
    client.WaitForResponses( 1, 2000 );
    TTestHttpResponseVector responses;
    client.GetResponses( responses );
    if ( GUCEF_NULL != closedAfter )
        *closedAfter = client.WaitForClose( 1000 );
    return responses.empty() ? 0 : responses[ 0 ].status;
}

/*-------------------------------------------------------------------------*/

static void
PerformHardeningTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Malformed and ambiguous requests are refused: bad Content-Length, duplicate Host, bad request line, oversized and overfull header blocks" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.maxHeaderBytes = 1024;
            settings.maxHeaderCount = 10;
            CBodyTestServer server( &settings );
            ASSERT_TRUE( server.Start() );
            const UInt16 port = server.GetPort();
            bool closed = false;

            const char* badLengths[] = { "+4", "4x", "-1", "0x4", "4 4", "99999999999999999999", "" };
            for ( int i=0; i<7; ++i )
            {
                CORE::CString request = "POST /len HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Length: " + CORE::CString( badLengths[ i ] ) + "\r\n\r\ndata";
                ASSERT_TRUE( 400 == StatusOfRequest( port, request, &closed ) && closed );
            }
            ASSERT_TRUE( 400 == StatusOfRequest( port, "GET / HTTP/1.1\r\nHost: a\r\nHost: b\r\n\r\n", &closed ) && closed );
            ASSERT_TRUE( 400 == StatusOfRequest( port, "POST / HTTP/1.1\r\nHost: a\r\nContent-Type: a/b\r\nContent-Type: c/d\r\nContent-Length: 0\r\n\r\n", &closed ) && closed );
            ASSERT_TRUE( 400 == StatusOfRequest( port, "GARBAGE\r\n\r\n", &closed ) && closed );
            ASSERT_TRUE( 400 == StatusOfRequest( port, "GET / FTP/1.1\r\nHost: a\r\n\r\n", &closed ) && closed );
            ASSERT_TRUE( 501 == StatusOfRequest( port, "POST / HTTP/1.1\r\nHost: a\r\nContent-Length: 4\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n", &closed ) && closed );
            ASSERT_TRUE( 417 == StatusOfRequest( port, "POST / HTTP/1.1\r\nHost: a\r\nExpect: something-else\r\nContent-Length: 1\r\n\r\n", &closed ) && closed );

            CORE::CString big = "GET / HTTP/1.1\r\nHost: a\r\nX-Big: ";
            for ( UInt32 i=0; i<1100; ++i )
                big += "b";
            ASSERT_TRUE( 431 == StatusOfRequest( port, big, &closed ) && closed );

            CORE::CString many = "GET / HTTP/1.1\r\nHost: a\r\n";
            for ( UInt32 i=0; i<11; ++i )
                many += "X-Field-" + CORE::ToString( i ) + ": v\r\n";
            ASSERT_TRUE( 431 == StatusOfRequest( port, many + "\r\n", &closed ) && closed );

            // repeated Content-Length fields with the same value are one length; repeated other fields are joined
            ASSERT_TRUE( 200 == StatusOfRequest( port, "POST /same HTTP/1.1\r\nHost: a\r\nContent-Length: 2\r\nContent-Length: 2\r\n\r\nok", &closed ) );
            ASSERT_TRUE( server.GetRecorder().GetCount() == 1 && BodyTestSameText( server.GetRecorder().GetBody( 0 ), "ok" ) );

            // an HTTP/1.0 request, and one that asks for the close, are answered and closed
            ASSERT_TRUE( 200 == StatusOfRequest( port, "GET /old HTTP/1.0\r\n\r\n", &closed ) && closed );
            ASSERT_TRUE( 200 == StatusOfRequest( port, "GET /close HTTP/1.1\r\nHost: a\r\nConnection: close\r\n\r\n", &closed ) && closed );

            TestHttpPumpFor( 50 );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );
            ASSERT_TRUE( 0 == server.GetServer().GetBufferedRequestBytes() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A response whose header value holds a line break is never written, the client gets 500" )
        try
        {
            WEB::CHttpResponseData splitting;
            splitting.statusCode = 200;
            splitting.otherHeaders[ "X-Echo" ] = "a\r\nSet-Cookie: stolen=1";
            CORE::CDynamicBuffer out;
            ASSERT_FALSE( splitting.Serialize( out ) );
            ASSERT_TRUE( 0 == out.GetDataSize() );
            WEB::CHttpResponseData badName;
            badName.statusCode = 200;
            badName.otherHeaders[ "X Bad" ] = "v";
            ASSERT_FALSE( badName.Serialize( out ) );
            WEB::CHttpResponseData badType;
            badType.statusCode = 200;
            badType.contentType = "text/plain\nX-Injected: 1";
            badType.content.CopyFrom( 1, "x" );
            ASSERT_FALSE( badType.Serialize( out ) );
            WEB::CHttpResponseData fine;
            fine.statusCode = 405;
            fine.allowedMethods.push_back( "POST" );
            ASSERT_TRUE( fine.Serialize( out ) );
            ASSERT_TRUE( 0 == BodyTestText( out ).HasSubstr( "HTTP/1.1 405 Method Not Allowed\r\n", 0, true ) );
            ASSERT_TRUE( BodyTestText( out ).HasSubstr( "\r\nAllow: POST\r\n" ) > 0 );

            CBodyTestServer server;
            CSplittingTakeoverHandler handler;
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/split", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.Send( "GET /split HTTP/1.1\r\nHost: a\r\n\r\n" ) );
            ASSERT_TRUE( client.WaitForResponses( 1 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == client.GetResponses( responses ) && 500 == responses[ 0 ].status );
            ASSERT_TRUE( client.GetReceived().HasSubstr( "stolen" ) < 0 );
            ASSERT_TRUE( 1 == handler.refusedSends );
            ASSERT_TRUE( client.WaitForClose( 1000 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Partial requests whose client goes away leave nothing behind, also after a reset mid-body and mid-stream" )
        try
        {
            CBodyTestServer server;
            CTestTakeoverHandler handler( CTestTakeoverHandler::STREAM_SSE );
            handler.sendsAfterClose = 0;
            ASSERT_TRUE( server.GetServer().RegisterRequestTakeoverHandler( "/events", &handler, 0 ) );
            ASSERT_TRUE( server.Start() );
            {
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /partial HTTP/1.1\r\nHost: a\r\nContent-Le" ) );
                ASSERT_TRUE( server.GetServer().GetBufferedRequestBytes() > 0 );
                client.Close();
            }
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 0 == server.GetServer().GetBufferedRequestBytes() );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );

            #ifdef GUCEF_WEB_TEST_RAW_POSIX_SOCKETS
            {
                CTestHttpResettingClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( "POST /partial HTTP/1.1\r\nHost: a\r\nContent-Length: 100\r\n\r\nonly some" ) );
                ASSERT_TRUE( server.GetServer().GetBufferedRequestBytes() > 0 );
                client.Reset();
            }
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 0 == server.GetServer().GetBufferedRequestBytes() );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );
            {
                CTestHttpResettingClient client;
                ASSERT_TRUE( client.Connect( server.GetPort() ) );
                ASSERT_TRUE( client.Send( PostRequest( "/events", "go" ), 50 ) );
                ASSERT_TRUE( 1 == server.GetServer().GetOpenExchangeCount() );
                client.Reset();
            }
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 1 == handler.closedCalls && 0 == handler.sendsAfterClose );
            ASSERT_TRUE( 0 == server.GetServer().GetOpenExchangeCount() );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );
            #endif /* GUCEF_WEB_TEST_RAW_POSIX_SOCKETS ? */
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: 1000 sequential connections are all answered and leave no state or sockets behind" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            const int socketsBefore = TESTFW::CountOpenSockets();
            UInt32 answered = 0;
            for ( UInt32 i=0; i<1000; ++i )
            {
                CTestHttpRawClient client;
                if ( !client.Connect( server.GetPort() ) )
                    continue;
                client.Send( ( i % 2 ) ? "GET /seq HTTP/1.1\r\nHost: a\r\nConnection: close\r\n\r\n" : CORE::CString( PostRequest( "/seq", "x" ) ), 0 );
                if ( client.WaitForResponses( 1, 2000 ) )
                    ++answered;
                client.Close();
            }
            TestHttpPumpFor( 200 );
            ASSERT_TRUE( BodyTestSameCount( answered, 1000, "answered connections" ) );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );
            ASSERT_TRUE( 0 == server.GetServer().GetBufferedRequestBytes() );
            if ( socketsBefore >= 0 )
            {
                ASSERT_TRUE( BodyTestSameCount( (UInt32) TESTFW::CountOpenSockets(), (UInt32) socketsBefore, "open sockets" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Twenty clients at once, each with three pipelined requests split over writes, all get their answers in order" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            const UInt32 clientCount = 20;
            GUCEF::vector< CTestHttpRawClient* > clients;
            for ( UInt32 i=0; i<clientCount; ++i )
            {
                clients.push_back( GUCEF_NEW CTestHttpRawClient() );
                ASSERT_TRUE( clients.back()->Connect( server.GetPort() ) );
            }
            for ( UInt32 i=0; i<clientCount; ++i )
            {
                CORE::CString requests = PostRequest( "/c" + CORE::ToString( i ), "1" ) + PostRequest( "/c" + CORE::ToString( i ), "2" ) + PostRequest( "/c" + CORE::ToString( i ), "3" );
                const UInt32 half = requests.Length() / 2;
                clients[ i ]->Send( requests.SubstrToIndex( half, true ), 0 );
            }
            TestHttpPumpFor( 20 );
            for ( UInt32 i=0; i<clientCount; ++i )
            {
                CORE::CString requests = PostRequest( "/c" + CORE::ToString( i ), "1" ) + PostRequest( "/c" + CORE::ToString( i ), "2" ) + PostRequest( "/c" + CORE::ToString( i ), "3" );
                const UInt32 half = requests.Length() / 2;
                clients[ i ]->Send( requests.CutChars( half, true ), 0 );
            }
            UInt32 inOrder = 0;
            for ( UInt32 i=0; i<clientCount; ++i )
            {
                clients[ i ]->WaitForResponses( 3, 3000 );
                TTestHttpResponseVector responses;
                if ( 3 == clients[ i ]->GetResponses( responses ) &&
                     responses[ 0 ].body == "got /c" + CORE::ToString( i ) + " 1" &&
                     responses[ 1 ].body == "got /c" + CORE::ToString( i ) + " 2" &&
                     responses[ 2 ].body == "got /c" + CORE::ToString( i ) + " 3" )
                    ++inOrder;
            }
            ASSERT_TRUE( BodyTestSameCount( inOrder, clientCount, "clients answered in order" ) );
            for ( UInt32 i=0; i<clientCount; ++i )
                GUCEF_DELETE clients[ i ];
            TestHttpPumpFor( 100 );
            ASSERT_TRUE( 0 == server.GetServer().GetTrackedConnectionCount() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Header fields RFC 9112 refuses get 400: whitespace before the colon, a bare CR or LF, NUL and other controls, obs-fold, a leading space, a name that is no token" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.Start() );
            const UInt16 port = server.GetPort();
            bool closed = false;

            const char* malformed[] = {
                "POST /f HTTP/1.1\r\nHost: a\r\nContent-Length : 2\r\n\r\nok",            // whitespace between name and colon (5.1)
                "POST /f HTTP/1.1\r\nHost: a\r\nContent-Length\t: 2\r\n\r\nok",
                "POST /f HTTP/1.1\r\nHost: a\r\nX-A: a\nContent-Length: 5\r\nContent-Length: 2\r\n\r\nok",     // a bare LF inside a field
                "POST /f HTTP/1.1\r\nHost: a\r\nX-A: a\rb\r\nContent-Length: 2\r\n\r\nok",   // a bare CR
                "POST /f HTTP/1.1\r\n Content-Length: 2\r\nHost: a\r\n\r\nok",            // whitespace before the first field (2.2)
                "POST /f HTTP/1.1\r\nHost: a\r\nX-A: b\r\n c\r\nContent-Length: 2\r\n\r\nok",  // obs-fold (5.2)
                "POST /f HTTP/1.1\r\nHost: a\r\nX-A: b\r\n\tc\r\nContent-Length: 2\r\n\r\nok",
                "POST /f HTTP/1.1\r\nHost: a\r\nX(A): b\r\nContent-Length: 2\r\n\r\nok",    // no token
                "POST /f HTTP/1.1\r\nHost: a\r\nX@A: b\r\nContent-Length: 2\r\n\r\nok",
                "POST /f HTTP/1.1\r\nHost: a\r\n: b\r\nContent-Length: 2\r\n\r\nok",       // no name
                "POST /f HTTP/1.1\r\nHost: a\r\nNoColonHere\r\nContent-Length: 2\r\n\r\nok",
                "POST /f HTTP/1.1\r\nHost: a\r\nX-A: a\x01" "b\r\nContent-Length: 2\r\n\r\nok",  // controls in a value
                "POST /f HTTP/1.1\r\nHost: a\r\nX-A: a\x7f\r\nContent-Length: 2\r\n\r\nok",
                "POST /f\x01 HTTP/1.1\r\nHost: a\r\nContent-Length: 2\r\n\r\nok" };       // a control in the request line
            for ( UInt32 i=0; i<sizeof( malformed ) / sizeof( malformed[ 0 ] ); ++i )
            {
                const Int32 status = StatusOfRequest( port, malformed[ i ], &closed );
                if ( 400 != status || !closed )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpServerBody test: malformed request " << i << " got " << status << ( closed ? " and a close" : " and no close" ) );
                ASSERT_TRUE( 400 == status && closed );
            }

            // a NUL inside a field name
            {
                const char nulRequest[] = "POST /f HTTP/1.1\r\nHost: a\r\nContent-Le\0ngth: 5\r\nContent-Length: 2\r\n\r\nok";
                CTestHttpRawClient client;
                ASSERT_TRUE( client.Connect( port ) );
                client.SendBytes( nulRequest, sizeof( nulRequest ) - 1 );
                client.WaitForResponses( 1, 2000 );
                TTestHttpResponseVector responses;
                ASSERT_TRUE( 1 == client.GetResponses( responses ) && 400 == responses[ 0 ].status );
            }
            ASSERT_TRUE( 0 == server.GetRecorder().GetCount() );

            // what is allowed stays allowed: a tab in a value, bytes above 0x7F, optional whitespace around the value
            ASSERT_TRUE( 200 == StatusOfRequest( port, "POST /ok HTTP/1.1\r\nHost: a\r\nX-A: a\tb\r\nX-B:  caf\xc3\xa9 \r\nX-C:\r\nContent-Length: 2\r\n\r\nok", &closed ) );
            ASSERT_TRUE( 1 == server.GetRecorder().GetCount() && BodyTestSameText( server.GetRecorder().GetBody( 0 ), "ok" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A connection that sends nothing is closed at headerTimeoutMs from its accept, not at idleTimeoutMs" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.headerTimeoutMs = 300;
            settings.idleTimeoutMs = 20000;
            CBodyTestServer server( &settings );
            ASSERT_TRUE( server.Start() );
            CTestHttpRawClient client;
            ASSERT_TRUE( client.Connect( server.GetPort() ) );
            ASSERT_TRUE( client.WaitForClose( 3000 ) );

            // after a response the idle time of a kept connection applies
            CTestHttpRawClient kept;
            ASSERT_TRUE( kept.Connect( server.GetPort() ) );
            ASSERT_TRUE( kept.Send( "GET /a HTTP/1.1\r\nHost: a\r\n\r\n" ) );
            ASSERT_TRUE( kept.WaitForResponses( 1, 2000 ) );
            ASSERT_FALSE( kept.WaitForClose( 1500 ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: 32 connections that send nothing do not keep the next client from being served" )
        try
        {
            WEB::CHttpServerSettings settings;
            settings.maxClientConnections = 32;
            CBodyTestServer server( &settings );
            ASSERT_TRUE( server.Start() );
            const UInt16 port = server.GetPort();

            GUCEF::vector< CTestHttpRawClient* > silent;
            for ( UInt32 i=0; i<32; ++i )
            {
                CTestHttpRawClient* client = GUCEF_NEW CTestHttpRawClient();
                silent.push_back( client );
                ASSERT_TRUE( client->Connect( port ) );
            }
            TestHttpPumpFor( 200 );

            // far within the header timeout of 10 s
            CTestHttpRawClient legitimate;
            ASSERT_TRUE( legitimate.Connect( port ) );
            ASSERT_TRUE( legitimate.Send( "GET /served HTTP/1.1\r\nHost: a\r\n\r\n" ) );
            ASSERT_TRUE( legitimate.WaitForResponses( 1, 3000 ) );
            TTestHttpResponseVector responses;
            ASSERT_TRUE( 1 == legitimate.GetResponses( responses ) && 200 == responses[ 0 ].status );
            ASSERT_TRUE( server.GetServer().GetTrackedConnectionCount() <= 32 );

            for ( size_t i=0; i<silent.size(); ++i )
                GUCEF_DELETE silent[ i ];
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The server names the pulse generator it runs on" )
        try
        {
            CBodyTestServer server;
            ASSERT_TRUE( server.GetServer().GetPulseGenerator() == CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            CORE::PulseGeneratorPtr own = CORE::CPulseGenerator::CreateSharedObj();
            WEB::CHTTPServer ownServer( own );
            ASSERT_TRUE( ownServer.GetPulseGenerator() == own );
            WEB::CHTTPServer defaultServer;
            ASSERT_TRUE( defaultServer.GetPulseGenerator() == CORE::CCoreGlobal::Instance()->GetPulseGenerator() );
            WEB::CWebSocketServer webSocketServer( ownServer );
            ASSERT_TRUE( webSocketServer.GetPulseGenerator() == own );
            WEB::CWebSocketServer ownedWebSocketServer( own );
            ASSERT_TRUE( ownedWebSocketServer.GetPulseGenerator() == own && ownedWebSocketServer.GetHttpServer().GetPulseGenerator() == own );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformHttpServerBodyTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING HTTP server body and takeover TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "HttpServerBody" );

    PerformRequestIntakeTests();
    PerformTimeoutTests();
    PerformTakeoverTests();
    PerformSseEventWriterTests();
    PerformHardeningTests();
}
