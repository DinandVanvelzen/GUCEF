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

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#include "TestHttpRawClient.h"
#define GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#endif /* GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H ? */

#include "TestHttpResponseSizes.h"

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

namespace {

/** the sizes of header plus body that are tried: around the multiples of 1024 bytes */
const UInt32 g_totalSizes[] = { 1023, 1024, 1025, 2047, 2048, 2049, 3072, 4095, 4096, 4097, 8191, 8192, 8193, 16384 };
const size_t g_totalSizeCount = sizeof( g_totalSizes ) / sizeof( g_totalSizes[ 0 ] );

/** the length of the extra header field of a response: no field, a short one and a long one, which moves bytes from the body to the header */
const UInt32 g_padSizes[] = { 0, 350, 1000 };
const size_t g_padSizeCount = sizeof( g_padSizes ) / sizeof( g_padSizes[ 0 ] );

/** Text of the given size, made of the given letter and the next ones so that a lost or moved byte shows */
CORE::CString
SizesText( UInt32 size, char firstLetter, UInt32 letterCount )
{
    if ( 0 == size )
        return CORE::CString::Empty;
    GUCEF::vector< char > buffer( size );
    for ( UInt32 i=0; i<size; ++i )
        buffer[ i ] = (char) ( firstLetter + ( i % letterCount ) );
    return CORE::CString( &buffer[ 0 ], size );
}

/** The body of the given size */
CORE::CString
SizesBodyText( UInt32 size )
{
    return SizesText( size, 'a', 26 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Builds the response whose serialized form, header plus body, is exactly totalSize bytes.
 *  @return false when no body size gives that total, as the header alone is larger
 */
bool
FitResponse( UInt32 totalSize, UInt32 padSize, bool keepAlive, WEB::CHttpResponseData& response, UInt32& bodySize )
{
    Int32 candidate = (Int32) totalSize;
    for ( int attempt=0; attempt<16 && candidate > 0; ++attempt )
    {
        response = WEB::CHttpResponseData();
        response.statusCode = 200;
        response.contentType = "application/json";
        if ( padSize > 0 )
            response.otherHeaders[ "X-Pad" ] = SizesText( padSize, 'p', 1 );
        const CORE::CString body = SizesBodyText( (UInt32) candidate );
        response.content.CopyFrom( (UInt32) candidate, body.C_String() );

        CORE::CDynamicBuffer serialized;
        if ( !response.Serialize( serialized, keepAlive ) )
            return false;
        const Int32 excess = (Int32) serialized.GetDataSize() - (Int32) totalSize;
        if ( 0 == excess )
        {
            bodySize = (UInt32) candidate;
            return true;
        }
        candidate -= excess;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Answers /total/<size>/<pad> with a response of exactly that many bytes of header plus body */
class CSizesHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CSizesHandler( void ) : WEB::CIHttpServerRequestHandler(), m_serverKeepAlive( false ) {}
    virtual ~CSizesHandler() {}

    void SetServerKeepAlive( bool serverKeepAlive ) { m_serverKeepAlive = serverKeepAlive; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        const CORE::CString::StringVector parts = request.requestUri.ParseElements( '/', false );
        UInt32 bodySize = 0;
        if ( parts.size() != 3 || !FitResponse( CORE::StringToUInt32( parts[ 1 ] ), CORE::StringToUInt32( parts[ 2 ] ), m_serverKeepAlive && request.keepConnectionsAlive, response, bodySize ) )
        {
            response = WEB::CHttpResponseData();
            response.statusCode = 400;
        }
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

    bool m_serverKeepAlive;
};

/*-------------------------------------------------------------------------*/

class CSizesHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CSizesHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CSizesHandler, MT::CMutex > TBase;

    explicit CSizesHandlerFactory( bool serverKeepAlive ) : TBase(), m_serverKeepAlive( serverKeepAlive ) {}
    virtual ~CSizesHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CSizesHandler* handler = static_cast< CSizesHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetServerKeepAlive( m_serverKeepAlive );
        return product;
    }

    private:

    CSizesHandlerFactory( const CSizesHandlerFactory& src );
    CSizesHandlerFactory& operator=( const CSizesHandlerFactory& src );

    bool m_serverKeepAlive;
};

/*-------------------------------------------------------------------------*/

/**
 *  A HTTP server on the loopback interface, set up like the server of the asset store tests: requests are handled
 *  in sync mode and it runs on a pulse generator and thread of its own so that a client can block the test thread
 */
class CSizesServer : public MT::CActiveObject
{
    public:

    explicit CSizesServer( bool keepAlive )
        : MT::CActiveObject()
        , m_keepAlive( keepAlive )
        , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , m_factory( keepAlive )
        , m_server( GUCEF_NULL )
        , m_port( 0 )
    {
        WEB::CHttpServerSettings settings;
        settings.bindAddress = "127.0.0.1";
        settings.keepAliveConnections = keepAlive;
        settings.processRequestsAsync = false;
        settings.allowWebSocketUpgrades = false;
        m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, &m_factory, &settings );
    }

    virtual ~CSizesServer()
    {
        Deactivate( false, true );
        m_server->Close();
        GUCEF_DELETE m_server;
    }

    bool Start( void )
    {
        if ( !m_server->ListenOnPort( 0 ) )
            return false;
        m_port = m_server->GetBoundPort();
        return Activate( GUCEF_NULL, 1, 1 );
    }

    UInt16 GetPort( void ) const { return m_port; }
    bool IsKeepAlive( void ) const { return m_keepAlive; }

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator->ForceDirectPulse();
        return false;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    bool m_keepAlive;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CSizesHandlerFactory m_factory;
    WEB::CHTTPServer* m_server;
    UInt16 m_port;
};

/*-------------------------------------------------------------------------*/

/** The size of the body of the response that has the given total size, for the log lines of the results */
UInt32
SizesBodyBytes( UInt32 totalSize, UInt32 padSize, bool keepAlive )
{
    WEB::CHttpResponseData response;
    UInt32 bodySize = 0;
    FitResponse( totalSize, padSize, keepAlive, response, bodySize );
    return bodySize;
}

/*-------------------------------------------------------------------------*/

CORE::CString
SizesUri( UInt32 totalSize, UInt32 padSize )
{
    return "/total/" + CORE::ToString( totalSize ) + "/" + CORE::ToString( padSize );
}

/*-------------------------------------------------------------------------*/

CORE::CString
SizesRequestText( UInt32 totalSize, UInt32 padSize, bool connectionClose )
{
    return "GET " + SizesUri( totalSize, padSize ) + " HTTP/1.1\r\nHost: 127.0.0.1\r\n" + ( connectionClose ? "Connection: close\r\n" : "" ) + "\r\n";
}

/*-------------------------------------------------------------------------*/

/** What a case found: empty when the response arrived whole */
typedef CORE::CString TSizesVerdict;

/** The native HTTP client, the one of the asset store tests: Execute() on the calling thread, the request says Connection: close */
TSizesVerdict
RunNativeClient( CSizesServer& server, UInt32 totalSize, UInt32 padSize )
{
    WEB::CHttpResponseData expected;
    UInt32 bodySize = 0;
    if ( !FitResponse( totalSize, padSize, false, expected, bodySize ) )
        return "SKIP";

    WEB::CHttpRequestData request;
    if ( !request.SetUrl( "http://127.0.0.1:" + CORE::ToString( server.GetPort() ) + SizesUri( totalSize, padSize ) ) )
        return "the request address was refused";
    request.requestType = "GET";

    CORE::CString::StringVector native;
    native.push_back( "native" );
    WEB::CHTTPClient client;
    client.SetPreferredBackends( native );

    WEB::CHttpClientRequestOptions options;
    options.connectTimeoutInMs = 5000;
    options.totalTimeoutInMs = 10000;
    WEB::CHttpResponseData response;
    WEB::CHttpClientTransferInfo info;
    if ( !client.Execute( request, options, response, &info ) || !info.success )
        return "Execute failed: " + info.errorDescription;
    if ( 200 != response.statusCode )
        return "status " + CORE::ToString( response.statusCode );
    if ( response.content.GetDataSize() != bodySize )
        return "body is " + CORE::ToString( response.content.GetDataSize() ) + " bytes, expected " + CORE::ToString( bodySize );
    const CORE::CString body = SizesBodyText( bodySize );
    if ( 0 != memcmp( response.content.GetConstBufferPtr(), body.C_String(), bodySize ) )
        return "body differs from what was sent";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

/** The TCP client of gucefCOMCORE, on the pulse generator of the application, which the test thread pumps */
TSizesVerdict
RunRawClient( CSizesServer& server, UInt32 totalSize, UInt32 padSize, bool connectionClose )
{
    const bool stays = server.IsKeepAlive() && !connectionClose;
    WEB::CHttpResponseData expected;
    UInt32 bodySize = 0;
    if ( !FitResponse( totalSize, padSize, stays, expected, bodySize ) )
        return "SKIP";

    CTestHttpRawClient client;
    if ( !client.Connect( server.GetPort() ) )
        return "no connection";
    client.Send( SizesRequestText( totalSize, padSize, connectionClose ), 0 );
    client.WaitForResponses( 1, 1500 );

    TTestHttpResponseVector responses;
    if ( 1 != client.GetResponses( responses ) )
        return "responses seen: " + CORE::ToString( client.GetResponses( responses ) ) + ", bytes received: " + CORE::ToString( client.GetReceived().Length() ) + ", closed: " + ( client.IsClosed() ? "yes" : "no" );
    if ( !responses[ 0 ].complete )
        return "response incomplete, bytes received: " + CORE::ToString( client.GetReceived().Length() ) + " of " + CORE::ToString( totalSize ) + ", closed: " + ( client.IsClosed() ? "yes" : "no" );
    if ( client.GetReceived().Length() != totalSize )
        return "bytes received: " + CORE::ToString( client.GetReceived().Length() ) + ", expected " + CORE::ToString( totalSize );
    if ( responses[ 0 ].body != SizesBodyText( bodySize ) )
        return "body differs from what was sent";
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_WEB_TEST_RAW_POSIX_SOCKETS

/** A plain blocking POSIX socket, which reads until the server ends the connection: no gucefCOMCORE code in the path of the receiving */
TSizesVerdict
RunPlainSocketClient( CSizesServer& server, UInt32 totalSize, UInt32 padSize )
{
    WEB::CHttpResponseData expected;
    UInt32 bodySize = 0;
    if ( !FitResponse( totalSize, padSize, false, expected, bodySize ) )
        return "SKIP";

    int sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock < 0 )
        return "no socket";
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 0;
    setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
    struct sockaddr_in address;
    memset( &address, 0, sizeof( address ) );
    address.sin_family = AF_INET;
    address.sin_port = htons( server.GetPort() );
    inet_pton( AF_INET, "127.0.0.1", &address.sin_addr );
    if ( 0 != connect( sock, (struct sockaddr*) &address, sizeof( address ) ) )
    {
        close( sock );
        return "no connection";
    }
    const CORE::CString requestText = SizesRequestText( totalSize, padSize, true );
    if ( (ssize_t) requestText.Length() != send( sock, requestText.C_String(), requestText.Length(), 0 ) )
    {
        close( sock );
        return "send failed";
    }

    CORE::CString received;
    char block[ 512 ];
    for ( ;; )
    {
        ssize_t got = recv( sock, block, sizeof( block ), 0 );
        if ( got <= 0 )
            break;
        received.Append( block, (UInt32) got );
    }
    close( sock );

    TTestHttpResponseVector responses;
    if ( received.Length() != totalSize )
        return "bytes received: " + CORE::ToString( received.Length() ) + ", expected " + CORE::ToString( totalSize );
    if ( 1 != TestHttpParseResponses( received, true, responses ) || !responses[ 0 ].complete || responses[ 0 ].body != SizesBodyText( bodySize ) )
        return "the received bytes are not the response that was sent";
    return CORE::CString::Empty;
}

#endif /* GUCEF_WEB_TEST_RAW_POSIX_SOCKETS ? */

/*-------------------------------------------------------------------------*/

enum ESizesClient
{
    SIZESCLIENT_NATIVE = 0,
    SIZESCLIENT_RAW_KEEPALIVE,
    SIZESCLIENT_RAW_CLOSE,
    SIZESCLIENT_PLAIN_SOCKET
};

/** Runs all the sizes with one kind of client against a server, every result is logged. Returns the number of sizes that failed */
UInt32
RunAllSizes( ESizesClient clientKind, const char* clientName, bool serverKeepAlive )
{
    CSizesServer server( serverKeepAlive );
    if ( !server.Start() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpResponseSizes test: the server did not start" );
        return 1;
    }

    UInt32 failures = 0;
    for ( size_t p=0; p<g_padSizeCount; ++p )
    {
        for ( size_t s=0; s<g_totalSizeCount; ++s )
        {
            TSizesVerdict verdict;
            switch ( clientKind )
            {
                case SIZESCLIENT_NATIVE:        verdict = RunNativeClient( server, g_totalSizes[ s ], g_padSizes[ p ] ); break;
                case SIZESCLIENT_RAW_KEEPALIVE: verdict = RunRawClient( server, g_totalSizes[ s ], g_padSizes[ p ], false ); break;
                case SIZESCLIENT_RAW_CLOSE:     verdict = RunRawClient( server, g_totalSizes[ s ], g_padSizes[ p ], true ); break;
                #ifdef GUCEF_WEB_TEST_RAW_POSIX_SOCKETS
                case SIZESCLIENT_PLAIN_SOCKET:  verdict = RunPlainSocketClient( server, g_totalSizes[ s ], g_padSizes[ p ] ); break;
                #endif
                default:                        verdict = "SKIP"; break;
            }

            if ( verdict == "SKIP" )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "HttpResponseSizes SKIP: client=" << clientName << " total=" << g_totalSizes[ s ] << " pad=" << g_padSizes[ p ] << ", no response has that size" );
                continue;
            }
            const char* result = verdict.IsNULLOrEmpty() ? "PASS" : "FAIL";
            const UInt32 bodyBytes = SizesBodyBytes( g_totalSizes[ s ], g_padSizes[ p ], serverKeepAlive );
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "HttpResponseSizes " << result << ": client=" << clientName << " serverKeepAlive=" << ( serverKeepAlive ? "yes" : "no" ) <<
                       " total=" << g_totalSizes[ s ] << " pad=" << g_padSizes[ p ] << " body=" << bodyBytes << ( verdict.IsNULLOrEmpty() ? CORE::CString::Empty : ( " reason: " + verdict ) ) );
            if ( !verdict.IsNULLOrEmpty() )
                ++failures;
        }
    }
    return failures;
}

} /* anonymous namespace */

/*-------------------------------------------------------------------------*/

void
PerformHttpResponseSizeTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The native client reads responses of every size whole from a closing server" )
        try
        {
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_NATIVE, "native", false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The native client reads responses of every size whole from a keep-alive server" )
        try
        {
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_NATIVE, "native", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The TCP client of gucefCOMCORE receives responses of every size whole on keep-alive connections" )
        try
        {
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_RAW_KEEPALIVE, "comcore-keepalive", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The TCP client of gucefCOMCORE receives responses of every size whole when the server closes after the response" )
        try
        {
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_RAW_CLOSE, "comcore-close", true ) );
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_RAW_KEEPALIVE, "comcore-serverclose", false ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #ifdef GUCEF_WEB_TEST_RAW_POSIX_SOCKETS
    GUCEF_TESTFW_TESTCASE( "Test: A plain socket receives responses of every size whole from the server up to the end of the connection" )
        try
        {
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_PLAIN_SOCKET, "plainsocket", false ) );
            ASSERT_TRUE( 0 == RunAllSizes( SIZESCLIENT_PLAIN_SOCKET, "plainsocket", true ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif /* GUCEF_WEB_TEST_RAW_POSIX_SOCKETS ? */

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL HTTP response size TESTS COMPLETED" );
}
