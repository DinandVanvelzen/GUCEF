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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

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

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#include "gucefWEB_CWebSocketHandshake.h"
#define GUCEF_WEB_CWEBSOCKETHANDSHAKE_H
#endif /* GUCEF_WEB_CWEBSOCKETHANDSHAKE_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestHttpHeaderParsing.h"

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

/** compares text and logs the actual text when it differs, so a failure tells what was received */
static bool
SameText( const CORE::CString& actual, const CORE::CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpHeaderParsing test: expected \"" << expected << "\" but got \"" << actual << "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** The requests as the handler of the server under test saw them */
class CRequestRecorder
{
    public:

    CRequestRecorder( void ) : m_lock(), m_requests() {}

    void Record( const WEB::CHttpRequestData& request )
    {
        MT::CScopeMutex lock( m_lock );
        m_requests.push_back( WEB::CHttpRequestData() );
        m_requests.back() = request;    // assigned: the copy of a request is tested on its own
    }

    UInt32 GetCount( void )
    {
        MT::CScopeMutex lock( m_lock );
        return (UInt32) m_requests.size();
    }

    bool GetLast( WEB::CHttpRequestData& request )
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_requests.empty() )
            return false;
        request = m_requests.back();
        return true;
    }

    private:

    MT::CMutex m_lock;
    GUCEF::vector< WEB::CHttpRequestData > m_requests;
};

/*-------------------------------------------------------------------------*/

class CRecordingHandler : public WEB::CIHttpServerRequestHandler
{
    public:

    CRecordingHandler( void ) : WEB::CIHttpServerRequestHandler(), m_recorder( GUCEF_NULL ) {}
    virtual ~CRecordingHandler() {}

    void SetRecorder( CRequestRecorder* recorder ) { m_recorder = recorder; }

    virtual bool OnRequest( const WEB::CHttpRequestData& request, WEB::CHttpResponseData& response ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL != m_recorder )
            m_recorder->Record( request );
        response.statusCode = 200;
        response.contentType = "text/plain";
        response.content.CopyFrom( 2, "ok" );
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

    CRequestRecorder* m_recorder;
};

/*-------------------------------------------------------------------------*/

class CRecordingHandlerFactory : public CORE::CTFactory< WEB::CIHttpServerRequestHandler, CRecordingHandler, MT::CMutex >
{
    public:

    typedef CORE::CTFactory< WEB::CIHttpServerRequestHandler, CRecordingHandler, MT::CMutex > TBase;

    explicit CRecordingHandlerFactory( CRequestRecorder* recorder ) : TBase(), m_recorder( recorder ) {}
    virtual ~CRecordingHandlerFactory() {}

    virtual TProductPtr Create( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        TProductPtr product = TBase::Create();
        CRecordingHandler* handler = static_cast< CRecordingHandler* >( product.GetPointerAlways() );
        if ( GUCEF_NULL != handler )
            handler->SetRecorder( m_recorder );
        return product;
    }

    private:

    CRecordingHandlerFactory( const CRecordingHandlerFactory& src );
    CRecordingHandlerFactory& operator=( const CRecordingHandlerFactory& src );

    CRequestRecorder* m_recorder;
};

/*-------------------------------------------------------------------------*/

/** A HTTP server on the loopback interface that runs on a thread of its own, so a client can block the test thread */
class CHeaderTestServer : public MT::CActiveObject
{
    public:

    CHeaderTestServer( void )
        : MT::CActiveObject()
        , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , m_recorder()
        , m_factory( GUCEF_NULL )
        , m_server( GUCEF_NULL )
        , m_port( 0 )
    {}

    virtual ~CHeaderTestServer()
    {
        Stop();
    }

    bool Start( void )
    {
        m_factory = GUCEF_NEW CRecordingHandlerFactory( &m_recorder );
        WEB::CHttpServerSettings settings;
        settings.bindAddress = "127.0.0.1";
        settings.keepAliveConnections = false;
        settings.processRequestsAsync = false;
        settings.allowWebSocketUpgrades = true;
        m_server = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, m_factory, &settings );
        if ( !m_server->ListenOnPort( 0 ) )
            return false;
        m_port = m_server->GetBoundPort();
        return Activate( GUCEF_NULL, 1, 1 );
    }

    void Stop( void )
    {
        Deactivate( false, true );
        if ( GUCEF_NULL != m_server )
        {
            m_server->Close();
            GUCEF_DELETE m_server;
            m_server = GUCEF_NULL;
        }
        if ( GUCEF_NULL != m_factory )
        {
            GUCEF_DELETE m_factory;
            m_factory = GUCEF_NULL;
        }
    }

    UInt16 GetPort( void ) const { return m_port; }
    CRequestRecorder& GetRecorder( void ) { return m_recorder; }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    protected:

    virtual bool OnThreadStart( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* taskdata ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator->ForceDirectPulse();
        return false;
    }

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CRequestRecorder m_recorder;
    CRecordingHandlerFactory* m_factory;
    WEB::CHTTPServer* m_server;
    UInt16 m_port;
};

/*-------------------------------------------------------------------------*/

/** Sends the exact bytes of a request with a plain TCP client and collects what comes back */
class CRawRequestClient : public CORE::CObserver
{
    public:

    CRawRequestClient( UInt16 port, const CORE::CString& request )
        : CORE::CObserver()
        , m_connected( false )
        , m_done( false )
        , m_request( request )
        , m_response()
        , m_client( false )
    {
        SubscribeTo( &m_client );
        m_client.ConnectTo( "127.0.0.1", port );
    }

    virtual ~CRawRequestClient()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "CRawRequestClient";
        return classTypeName;
    }

    /** Pulses until the server closed the connection or an answer is complete enough to be told apart */
    void Wait( UInt32 timeoutInMs )
    {
        for ( UInt32 waited=0; waited<timeoutInMs && !m_done; ++waited )
        {
            CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            MT::ThreadDelay( 1 );
        }
    }

    void Close( void ) { m_client.Close(); }

    bool m_connected;
    bool m_done;
    CORE::CString m_request;
    CORE::CString m_response;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier           ,
                           const CORE::CEvent& eventid         ,
                           CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( COMCORE::CTCPClientSocket::ConnectedEvent == eventid )
        {
            m_connected = true;
            m_client.Send( m_request.C_String(), m_request.ByteSize() - 1 );
        }
        else
        if ( COMCORE::CTCPClientSocket::DataRecievedEvent == eventid )
        {
            COMCORE::CTCPClientSocket::TDataRecievedEventData* data = static_cast< COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
            m_response.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
            if ( m_response.HasSubstr( "\r\n\r\n", 0, true ) >= 0 )
                m_done = true;
        }
        else
        if ( COMCORE::CTCPClientSocket::SocketErrorEvent == eventid || COMCORE::CTCPClientSocket::DisconnectedEvent == eventid )
        {
            m_done = true;
        }
    }

    private:

    COMCORE::CTCPClientSocket m_client;
};

/*-------------------------------------------------------------------------*/

/** Sends a request, waits for the answer and gives the request as the handler saw it. False when the handler never saw one */
static bool
SendRaw( CHeaderTestServer& server, const CORE::CString& rawRequest, WEB::CHttpRequestData& seen, CORE::CString* response = GUCEF_NULL )
{
    UInt32 before = server.GetRecorder().GetCount();
    CRawRequestClient client( server.GetPort(), rawRequest );
    client.Wait( 5000 );
    client.Close();
    if ( GUCEF_NULL != response )
        *response = client.m_response;
    if ( server.GetRecorder().GetCount() <= before )
        return false;
    return server.GetRecorder().GetLast( seen );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
HeaderOf( const WEB::CHttpRequestData& request, const CORE::CString& lowercaseName )
{
    WEB::CHttpRequestData::TStringMap::const_iterator i = request.otherHeaders.find( lowercaseName );
    return i == request.otherHeaders.end() ? CORE::CString( "<absent>" ) : (*i).second;
}

/*-------------------------------------------------------------------------*/

/** The web socket handshake needs the sha1 codec, which comes with a plugin */
static bool
EnsureSha1CodecPlugin( void )
{GUCEF_TRACE;

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

/** The header block of a request line plus the fields given, each field is one line */
static CORE::CString
MakeRequest( const CORE::CString& fields )
{
    return "GET /headers HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n" + fields + "\r\n";
}

/*-------------------------------------------------------------------------*/

void
PerformHttpHeaderParsingTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING HTTP header parsing TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "HttpHeaderParsing" );

    CHeaderTestServer server;
    bool started = server.Start();

    GUCEF_TESTFW_TESTCASE( "Test: The test server for the header parsing tests runs" )
        try
        {
            ASSERT_TRUE( started );
            ASSERT_TRUE( 0 != server.GetPort() );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( !started )
        return;

    GUCEF_TESTFW_TESTCASE( "Test: A copy and a clone of request and response data keep their header fields" )
        try
        {
            WEB::CHttpRequestData request;
            request.requestUri = "/x";
            request.otherHeaders[ "authorization" ] = "Bearer abc.def";
            request.otherHeaders[ "x-words" ] = "one two";
            WEB::CHttpRequestData requestCopy( request );
            ASSERT_TRUE( 2 == requestCopy.otherHeaders.size() );
            ASSERT_TRUE( SameText( HeaderOf( requestCopy, "authorization" ), "Bearer abc.def" ) );
            WEB::CHttpRequestData* requestClone = static_cast< WEB::CHttpRequestData* >( request.Clone() );
            ASSERT_TRUE( GUCEF_NULL != requestClone );
            ASSERT_TRUE( 2 == requestClone->otherHeaders.size() );
            ASSERT_TRUE( SameText( HeaderOf( *requestClone, "x-words" ), "one two" ) );
            GUCEF_DELETE requestClone;

            WEB::CHttpResponseData response;
            response.statusCode = 200;
            response.otherHeaders[ "X-Words" ] = "one two";
            WEB::CHttpResponseData responseCopy( response );
            ASSERT_TRUE( 1 == responseCopy.otherHeaders.size() );
            WEB::CHttpResponseData* responseClone = static_cast< WEB::CHttpResponseData* >( response.Clone() );
            ASSERT_TRUE( GUCEF_NULL != responseClone );
            ASSERT_TRUE( 1 == responseClone->otherHeaders.size() && SameText( responseClone->otherHeaders[ "X-Words" ], "one two" ) );
            GUCEF_DELETE responseClone;
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A request of a real client keeps the spaces inside the values of its header fields" )
        try
        {
            WEB::CHTTPClient client;
            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( "http://127.0.0.1:" + CORE::ToString( server.GetPort() ) + "/real" ) );
            request.requestType = "GET";
            request.otherHeaders[ "Authorization" ] = "Bearer abc.def";
            request.otherHeaders[ "User-Agent" ] = "Mozilla/5.0 (X11; Linux)";
            request.otherHeaders[ "Cookie" ] = "a=1; b=2";
            request.otherHeaders[ "X-Words" ] = "one two  three";
            request.resourceRepresentations.push_back( "text/html" );
            request.resourceRepresentations.push_back( "application/json;q=0.9" );

            WEB::CHttpResponseData response;
            ASSERT_TRUE( client.Execute( request, response ) );
            ASSERT_TRUE( 200 == response.statusCode );

            WEB::CHttpRequestData seen;
            ASSERT_TRUE( server.GetRecorder().GetLast( seen ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "authorization" ), "Bearer abc.def" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "user-agent" ), "Mozilla/5.0 (X11; Linux)" ) );
            ASSERT_TRUE( SameText( seen.transactionID, "a=1; b=2" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-words" ), "one two  three" ) );
            ASSERT_TRUE( 2 == seen.resourceRepresentations.size() );
            ASSERT_TRUE( SameText( seen.resourceRepresentations[ 0 ], "text/html" ) );
            ASSERT_TRUE( SameText( seen.resourceRepresentations[ 1 ], "application/json" ) );
            ASSERT_TRUE( SameText( seen.requestHost, "127.0.0.1:" + CORE::ToString( server.GetPort() ) ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Only the optional whitespace around a value is dropped, spaces and tabs inside it stay" )
        try
        {
            WEB::CHttpRequestData seen;
            ASSERT_TRUE( SendRaw( server, MakeRequest(
                "X-Inner: one two  three\r\n"
                "X-Tabs: a\tb \t c\r\n"
                "X-Padded:    padded value  \t \r\n"
                "X-Tab-Padded:\tvalue\t\r\n"
                "X-Empty:\r\n"
                "X-Blank:    \r\n"
                "X-NoSpace:value\r\n"
                "Authorization: Bearer abc.def\r\n"
                "User-Agent: Mozilla/5.0 (X11; Linux)\r\n"
                "X-Colon: a:b: c\r\n" ), seen ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-inner" ), "one two  three" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-tabs" ), "a\tb \t c" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-padded" ), "padded value" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-tab-padded" ), "value" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-empty" ), "" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-blank" ), "" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-nospace" ), "value" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "authorization" ), "Bearer abc.def" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "user-agent" ), "Mozilla/5.0 (X11; Linux)" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-colon" ), "a:b: c" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Header names are matched without regard to case and whitespace around the name" )
        try
        {
            WEB::CHttpRequestData seen;
            ASSERT_TRUE( SendRaw( server, MakeRequest(
                "X-MiXeD-CaSe: value one\r\n"
                "X-Space-Before-Colon : value two\r\n"
                "no colon on this line\r\n"
                "X-After: value three\r\n" ), seen ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-mixed-case" ), "value one" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-space-before-colon" ), "value two" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-after" ), "value three" ) );
            ASSERT_TRUE( seen.otherHeaders.end() == seen.otherHeaders.find( "nocolononthisline" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Cookie, Accept, Accept-Encoding, Content-Type, If-Match and Host keep their inner spaces or are split on their commas with the whitespace around the elements dropped" )
        try
        {
            WEB::CHttpRequestData seen;
            ASSERT_TRUE( SendRaw( server, MakeRequest(
                "Cookie: a=1; b=2;  c=3 \r\n"
                "Accept: text/html, application/json;q=0.9 ,\ttext/plain ; q=0.5, */*;q=0.1\r\n"
                "Accept-Encoding: gzip, deflate ,br\r\n"
                "Content-Type: application/json; charset=utf-8\r\n"
                "If-Match: \"v 1\", \"v,2\" ,\"v3\"\r\n" ), seen ) );
            ASSERT_TRUE( SameText( seen.transactionID, "a=1; b=2;  c=3" ) );

            ASSERT_TRUE( 4 == seen.resourceRepresentations.size() );
            ASSERT_TRUE( SameText( seen.resourceRepresentations[ 0 ], "text/html" ) );
            ASSERT_TRUE( SameText( seen.resourceRepresentations[ 1 ], "application/json" ) );
            ASSERT_TRUE( SameText( seen.resourceRepresentations[ 2 ], "text/plain" ) );
            ASSERT_TRUE( SameText( seen.resourceRepresentations[ 3 ], "*/*" ) );

            ASSERT_TRUE( 3 == seen.encodingRepresentations.size() );
            ASSERT_TRUE( SameText( seen.encodingRepresentations[ 0 ], "gzip" ) );
            ASSERT_TRUE( SameText( seen.encodingRepresentations[ 1 ], "deflate" ) );
            ASSERT_TRUE( SameText( seen.encodingRepresentations[ 2 ], "br" ) );

            ASSERT_TRUE( SameText( seen.contentRepresentation, "application/json; charset=utf-8" ) );

            // entity tags are quoted strings: spaces and commas inside the quotes belong to the tag
            ASSERT_TRUE( 3 == seen.resourceVersions.size() );
            ASSERT_TRUE( SameText( seen.resourceVersions[ 0 ], "\"v 1\"" ) );
            ASSERT_TRUE( SameText( seen.resourceVersions[ 1 ], "\"v,2\"" ) );
            ASSERT_TRUE( SameText( seen.resourceVersions[ 2 ], "\"v3\"" ) );
            ASSERT_TRUE( SameText( seen.requestHost, "127.0.0.1" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Quoted strings in a value keep their spaces" )
        try
        {
            WEB::CHttpRequestData seen;
            ASSERT_TRUE( SendRaw( server, MakeRequest(
                "X-Quoted:  \"a b  c\" ; note=\"d e\" \r\n"
                "Content-Disposition: form-data; name=\"the field\"; filename=\"my file.txt\"\r\n" ), seen ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-quoted" ), "\"a b  c\" ; note=\"d e\"" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "content-disposition" ), "form-data; name=\"the field\"; filename=\"my file.txt\"" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A folded header field is one value with the fold replaced by a space" )
        try
        {
            WEB::CHttpRequestData seen;
            ASSERT_TRUE( SendRaw( server, MakeRequest(
                "X-Folded: part one\r\n"
                "   part two\r\n"
                "\tpart three  \r\n"
                "X-After-Fold: after\r\n" ), seen ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-folded" ), "part one part two part three" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "x-after-fold" ), "after" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: A web socket upgrade is accepted with a Connection field that is a list, in any case, and keeps the list of sub protocols" )
        GUCEF_TESTFW_SKIP_IF( !EnsureSha1CodecPlugin(), "The sha1 codec for the WebSocket handshake is not available" );
        try
        {
            // the sample handshake of RFC 6455
            const CORE::CString upgradeBase =
                "GET /chat HTTP/1.1\r\n"
                "Host: 127.0.0.1\r\n"
                "Sec-WebSocket-Version: 13\r\n"
                "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n";

            WEB::CHttpRequestData seen;
            CORE::CString response;

            // what a browser sends: more tokens in the Connection field
            SendRaw( server, upgradeBase +
                "Connection: keep-alive, Upgrade\r\n"
                "Upgrade: websocket\r\n"
                "Sec-WebSocket-Protocol: chat, superchat\r\n\r\n", seen, &response );
            ASSERT_TRUE( 0 == response.HasSubstr( "HTTP/1.1 101", 0, true ) );
            ASSERT_TRUE( response.HasSubstr( "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", 0, true ) >= 0 );

            // field values are matched without regard to case
            response.Clear();
            SendRaw( server, upgradeBase +
                "Connection: UPGRADE\r\n"
                "Upgrade: WebSocket\r\n\r\n", seen, &response );
            ASSERT_TRUE( 0 == response.HasSubstr( "HTTP/1.1 101", 0, true ) );

            response.Clear();
            SendRaw( server, upgradeBase +
                "Connection: Upgrade\r\n"
                "Upgrade: websocket\r\n\r\n", seen, &response );
            ASSERT_TRUE( 0 == response.HasSubstr( "HTTP/1.1 101", 0, true ) );

            // a Connection field that does not mention Upgrade is refused
            response.Clear();
            SendRaw( server, upgradeBase +
                "Connection: keep-alive\r\n"
                "Upgrade: websocket\r\n\r\n", seen, &response );
            ASSERT_TRUE( 0 == response.HasSubstr( "HTTP/1.1 400", 0, true ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The sub protocol list of a web socket upgrade reaches the handler with its spaces" )
        try
        {
            // the server of these tests has no web socket handler, so look at how the request is parsed by an ordinary request with the field
            WEB::CHttpRequestData seen;
            ASSERT_TRUE( SendRaw( server, MakeRequest(
                "Sec-WebSocket-Protocol: chat, superchat,  third\r\n"
                "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n" ), seen ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "sec-websocket-protocol" ), "chat, superchat,  third" ) );
            ASSERT_TRUE( SameText( HeaderOf( seen, "sec-websocket-extensions" ), "permessage-deflate; client_max_window_bits" ) );
        }
        catch( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    server.Stop();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL HTTP header parsing TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
