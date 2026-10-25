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
#include <thread>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#include "gucefCORE_CUriResourceAccessorFactory.h"
#define GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H
#endif /* GUCEF_CORE_CURIRESOURCEACCESSORFACTORY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CRESOURCEMETADATA_H
#include "gucefCORE_CResourceMetaData.h"
#define GUCEF_CORE_CRESOURCEMETADATA_H
#endif /* GUCEF_CORE_CRESOURCEMETADATA_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_WEB_CWEBGLOBAL_H
#include "gucefWEB_CWebGlobal.h"
#define GUCEF_WEB_CWEBGLOBAL_H
#endif /* GUCEF_WEB_CWEBGLOBAL_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_COMCORE_H
#include "gucefCOMCORE.h"
#define GUCEF_COMCORE_H
#endif /* GUCEF_COMCORE_H ? */

#ifndef GUCEF_WEB_H
#include "gucefWEB.h"
#define GUCEF_WEB_H
#endif /* GUCEF_WEB_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENT_H
#include "gucefWEB_CHTTPClient.h"
#define GUCEF_WEB_CHTTPCLIENT_H
#endif /* GUCEF_WEB_CHTTPCLIENT_H ? */

#ifndef GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#include "gucefWEB_CHttpClientBackendSelection.h"
#include "CTFactory.h"
#define GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H
#endif /* GUCEF_WEB_CHTTPCLIENTBACKENDSELECTION_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_TLS_SERVER_H
#include "gucef_test_tls_server.h"
#define GUCEF_TEST_TLS_SERVER_H
#endif /* GUCEF_TEST_TLS_SERVER_H ? */

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#include "gucef_test_tls_terminator.h"
#define GUCEF_TEST_TLS_TERMINATOR_H
#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */

#ifndef GUCEF_TEST_HTTP_PROXY_H
#include "gucef_test_http_proxy.h"
#define GUCEF_TEST_HTTP_PROXY_H
#endif /* GUCEF_TEST_HTTP_PROXY_H ? */

#ifndef GUCEF_TEST_LOG_CAPTURE_H
#include "gucef_test_log_capture.h"
#define GUCEF_TEST_LOG_CAPTURE_H
#endif /* GUCEF_TEST_LOG_CAPTURE_H ? */

#ifndef GUCEF_TEST_NO_TLS_PROVIDER_H
#include "gucef_test_no_tls_provider.h"
#define GUCEF_TEST_NO_TLS_PROVIDER_H
#endif /* GUCEF_TEST_NO_TLS_PROVIDER_H ? */

#ifndef GUCEF_WEB_CHTTPPROXYENVIRONMENT_H
#include "gucefWEB_CHttpProxyEnvironment.h"
#define GUCEF_WEB_CHTTPPROXYENVIRONMENT_H
#endif /* GUCEF_WEB_CHTTPPROXYENVIRONMENT_H ? */

#include "TestHttpClient.h"

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPREDACTION_H
#include "TestHttpRedaction.h"
#define GUCEF_WEB_TESTAPP_TESTHTTPREDACTION_H
#endif /* GUCEF_WEB_TESTAPP_TESTHTTPREDACTION_H ? */

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

static const UInt16 SCRIPTED_SERVER_PORT = 47600;
static const UInt16 OTHER_ORIGIN_PORT = 47601;
static const UInt16 NOTHING_LISTENS_PORT = 47699;

/*-------------------------------------------------------------------------*/

static CORE::CString
ToCString( const CORE::CDynamicBuffer& buffer )
{
    return CORE::CString( buffer.AsConstTypePtr< char >(), buffer.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** the number of bytes of the text, without the null terminator */
static UInt32
TextByteCount( const CORE::CString& text )
{
    return text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;    // ByteSize includes the null terminator
}

/*-------------------------------------------------------------------------*/

/** compares text and logs the actual text when it differs, so a failure tells what was received */
static bool
SameText( const CORE::CString& actual, const CORE::CString& expected )
{
    if ( actual == expected )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpClient test: expected \"" + expected + "\" but got \"" + actual + "\"" );
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  A HTTP server scripted for testing clients: it answers with exact bytes, sent in pieces on separate
 *  network writes so that headers and bodies arrive split over reads, and it runs on its own thread so that
 *  a client can block the test thread with Execute()
 */
class CScriptedHttpServer : public CORE::CObserver
{
    public:

    CScriptedHttpServer( void )
        : CORE::CObserver()
        , m_pulseGenerator( CORE::CPulseGenerator::CreateSharedObj() )
        , m_socket( m_pulseGenerator, false )
        , m_running( false )
        , m_thread()
        , m_connections()
        , m_lock()
        , m_requestCount( 0 )
        , m_lastCookieHeader()
        , m_lastRequestHeaders()
        , m_secureBaseUrl()
    {
        SubscribeTo( &m_socket );
    }

    virtual ~CScriptedHttpServer()
    {
        Stop();
    }

    bool Start( UInt16 port )
    {
        if ( !m_socket.ListenOnPort( port ) )
            return false;
        m_running = true;
        m_thread = std::thread( [this]() { Run(); } );
        return true;
    }

    void Stop( void )
    {
        m_running = false;
        if ( m_thread.joinable() )
            m_thread.join();
        m_socket.Close();
    }

    /** the url of the https that leads to this server, where /to-secure redirects to */
    void SetSecureBaseUrl( const CORE::CString& url ) { MT::CScopeMutex lock( m_lock ); m_secureBaseUrl = url; }

    UInt32 GetRequestCount( void ) { MT::CScopeMutex lock( m_lock ); return m_requestCount; }
    CORE::CString GetLastCookieHeader( void ) { MT::CScopeMutex lock( m_lock ); return m_lastCookieHeader; }
    CORE::CString GetLastRequestHeaders( void ) { MT::CScopeMutex lock( m_lock ); return m_lastRequestHeaders; }

    /** whether the last request carried a header with the given name, regardless of the case it was written in */
    bool LastRequestHadHeader( const CORE::CString& name )
    {
        return HeaderPresent( GetLastRequestHeaders(), name );
    }

    CORE::CString GetLastRequestHeader( const CORE::CString& name )
    {
        return HeaderValue( GetLastRequestHeaders(), name );
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString name = "CScriptedHttpServer";
        return name;
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( notifier == &m_socket )
        {
            if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventId )
            {
                COMCORE::CTCPServerSocket::TClientConnectedEventData* data = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData );
                COMCORE::CTCPServerConnection* connection = data->GetData().connection;
                SubscribeTo( connection );
                SConnection& state = m_connections[ connection ];
                state = SConnection();
                state.connection = connection;
            }
            else
            if ( COMCORE::CTCPServerSocket::ClientDisconnectedEvent == eventId )
            {
                COMCORE::CTCPServerSocket::TClientDisconnectedEventData* data = static_cast< COMCORE::CTCPServerSocket::TClientDisconnectedEventData* >( eventData );
                m_connections.erase( data->GetData().connectionInfo.connection );
            }
            return;
        }

        if ( COMCORE::CTCPConnection::DataRecievedEvent == eventId )
        {
            GUCEF::map< void*, SConnection >::iterator i = m_connections.find( notifier );
            if ( i == m_connections.end() )
                return;
            COMCORE::CTCPConnection::TDataRecievedEventData* data = static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData );
            (*i).second.request.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
            TryAnswer( (*i).second );
        }
    }

    private:

    struct SConnection
    {
        COMCORE::CTCPServerConnection* connection;
        CORE::CString request;
        GUCEF::vector< CORE::CString > pieces;
        size_t nextPiece;
        bool answered;
        bool closeAfterAnswer;

        SConnection( void ) : connection( GUCEF_NULL ), request(), pieces(), nextPiece( 0 ), answered( false ), closeAfterAnswer( true ) {}
    };

    void Run( void )
    {
        while ( m_running )
        {
            m_pulseGenerator->ForceDirectPulse();
            SendPendingPieces();
            MT::ThreadDelay( 2 );
        }
    }

    /** one piece per connection per round, so the client receives them in separate reads */
    void SendPendingPieces( void )
    {
        GUCEF::vector< COMCORE::CTCPServerConnection* > toClose;
        for ( GUCEF::map< void*, SConnection >::iterator i = m_connections.begin(); i != m_connections.end(); ++i )
        {
            SConnection& state = (*i).second;
            if ( !state.answered || state.nextPiece >= state.pieces.size() )
                continue;
            const CORE::CString& piece = state.pieces[ state.nextPiece++ ];
            if ( !piece.IsNULLOrEmpty() )
                state.connection->Send( piece.C_String(), TextByteCount( piece ) );
            if ( state.nextPiece >= state.pieces.size() && state.closeAfterAnswer )
                toClose.push_back( state.connection );
        }
        for ( size_t c=0; c<toClose.size(); ++c )
            toClose[ c ]->Close();
    }

    static bool HeaderPresent( const CORE::CString& headers, const CORE::CString& name )
    {
        return headers.Lowercase().HasSubstr( ( "\n" + name + ":" ).Lowercase(), true ) >= 0;
    }

    static CORE::CString HeaderValue( const CORE::CString& headers, const CORE::CString& name )
    {
        CORE::CString lowerHeaders = headers.Lowercase();
        CORE::CString lowerName = ( "\n" + name + ":" ).Lowercase();
        Int32 pos = lowerHeaders.HasSubstr( lowerName, true );
        if ( pos < 0 )
            return CORE::CString();
        UInt32 start = (UInt32) pos + lowerName.Length();
        while ( start < headers.Length() && ( ' ' == headers[ start ] || '\t' == headers[ start ] ) )
            ++start;
        Int32 end = headers.HasSubstr( "\r\n", (Int32) start, true );
        return headers.SubstrFromRange( start, end < 0 ? headers.Length() : (UInt32) end );
    }

    /** decodes a chunked request body, returns false while it is incomplete */
    static bool DecodeChunked( const CORE::CString& raw, CORE::CString& body )
    {
        UInt32 pos = 0;
        body.Clear();
        while ( true )
        {
            Int32 lineEnd = raw.HasSubstr( "\r\n", (Int32) pos, true );
            if ( lineEnd < 0 )
                return false;
            const CORE::CString sizeText = raw.SubstrFromRange( pos, (UInt32) lineEnd );    // const: only the const C_String() never returns NULL
            UInt32 size = (UInt32) strtoul( sizeText.C_String(), GUCEF_NULL, 16 );
            pos = (UInt32) lineEnd + 2;
            if ( 0 == size )
                return raw.Length() >= pos + 2;
            if ( raw.Length() < pos + size + 2 )
                return false;
            body += raw.SubstrFromRange( pos, pos + size );
            pos += size + 2;
        }
    }

    void TryAnswer( SConnection& state )
    {
        if ( state.answered )
            return;
        Int32 headerEnd = state.request.HasSubstr( "\r\n\r\n", true );
        if ( headerEnd < 0 )
            return;
        CORE::CString headers = state.request.SubstrToIndex( (UInt32) headerEnd + 2, true );
        CORE::CString rawBody = state.request.CutChars( (UInt32) headerEnd + 4, true );
        CORE::CString body;
        if ( HeaderValue( headers, "Transfer-Encoding" ).HasSubstr( "chunked", true ) >= 0 )
        {
            if ( !DecodeChunked( rawBody, body ) )
                return;
        }
        else
        {
            const CORE::CString contentLengthText = HeaderValue( headers, "Content-Length" );    // const: only the const C_String() never returns NULL
            UInt32 contentLength = (UInt32) strtoul( contentLengthText.C_String(), GUCEF_NULL, 10 );
            if ( rawBody.Length() < contentLength )
                return;
            body = rawBody.SubstrToIndex( contentLength, true );
        }

        Int32 methodEnd = headers.HasChar( ' ', true );
        CORE::CString method = methodEnd < 0 ? headers : headers.SubstrToIndex( (UInt32) methodEnd, true );
        UInt32 pathStart = (UInt32) ( methodEnd + 1 );
        Int32 pathEnd = headers.HasChar( ' ', pathStart, true );
        CORE::CString path = headers.SubstrFromRange( pathStart, pathEnd < 0 ? headers.Length() : (UInt32) pathEnd );
        {
            MT::CScopeMutex lock( m_lock );
            ++m_requestCount;
            m_lastCookieHeader = HeaderValue( headers, "Cookie" );
            m_lastRequestHeaders = headers;
        }

        state.answered = true;
        Route( state, method, path, headers, body );
    }

    static CORE::CString Response( int status, const CORE::CString& extraHeaders, const CORE::CString& body, bool withLength = true )
    {
        char statusLine[ 64 ];
        sprintf( statusLine, "HTTP/1.1 %d X\r\n", status );
        CORE::CString response = statusLine + extraHeaders;
        if ( withLength )
            response += "Content-Length: " + CORE::ToString( TextByteCount( body ) ) + "\r\n";
        return response + "\r\n" + body;
    }

    void Route( SConnection& state, const CORE::CString& method, const CORE::CString& path, const CORE::CString& headers, const CORE::CString& body )
    {
        GUCEF::vector< CORE::CString >& p = state.pieces;
        if ( path == "/hello" )
        {
            // the header arrives in three reads, split in the middle of a line
            CORE::CString response = Response( 200, "Content-Type: text/plain\r\nAccept-Ranges: bytes\r\n", "hello world" );
            p.push_back( response.SubstrFromRange( 0, 10 ) );
            p.push_back( response.SubstrFromRange( 10, 35 ) );
            p.push_back( response.CutChars( 35, true ) );
        }
        else
        if ( path == "/chunked" )
        {
            // the chunks are split inside the size lines and the line ends
            p.push_back( "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n1" );
            p.push_back( "4\r\nThe quick " );
            p.push_back( "brown fox \r" );
            p.push_back( "\n1B;ext=1\r\njumps over the lazy dog!!!!\r\n0\r" );
            p.push_back( "\nTrailer: x\r\n\r\n" );
        }
        else
        if ( path == "/close" )
        {
            // no length, the body ends when the connection is closed
            p.push_back( "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nuntil " );
            p.push_back( "the end" );
        }
        else
        if ( path == "/to-secure" )
        {
            // to the https of the terminator that is in front of this server, when there is one
            CORE::CString target;
            {
                MT::CScopeMutex lock( m_lock );
                target = m_secureBaseUrl;
            }
            p.push_back( Response( 302, "Location: " + target + "/hello\r\n", "secure is over there" ) );
        }
        else
        if ( path == "/redirect/1" )
            p.push_back( Response( 302, "Location: /redirect/2\r\n", "go on" ) );
        else
        if ( path == "/redirect/2" )
            p.push_back( Response( 301, "Location: http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/hello\r\n", "moved" ) );
        else
        if ( path == "/loop" )
            p.push_back( Response( 302, "Location: /loop\r\n", "" ) );
        else
        if ( path == "/headers" )
            p.push_back( Response( 200, "", "ok" ) );
        else
        if ( path == "/to-same-origin" )
            p.push_back( Response( 302, "Location: http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/headers\r\n", "" ) );
        else
        if ( path == "/to-same-origin-relative" )
            p.push_back( Response( 307, "Location: /headers\r\n", "" ) );
        else
        if ( path == "/to-other-origin" )
            p.push_back( Response( 302, "Location: http://127.0.0.1:" + CORE::ToString( OTHER_ORIGIN_PORT ) + "/headers\r\n", "" ) );
        else
        if ( path == "/to-other-origin-307" )
            p.push_back( Response( 307, "Location: http://127.0.0.1:" + CORE::ToString( OTHER_ORIGIN_PORT ) + "/headers\r\n", "" ) );
        else
        if ( path == "/to-other-host" )
            p.push_back( Response( 302, "Location: http://127.0.0.2:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/headers\r\n", "" ) );
        else
        if ( path == "/to-other-origin-and-back" )
            p.push_back( Response( 302, "Location: http://127.0.0.1:" + CORE::ToString( OTHER_ORIGIN_PORT ) + "/back\r\n", "" ) );
        else
        if ( path == "/back" )
            p.push_back( Response( 302, "Location: http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/headers\r\n", "" ) );
        else
        if ( path == "/redirect/invalid" )
            p.push_back( Response( 302, "Location: http:///redirect-target?key=SECRETREDIRECT\r\n", "" ) );
        else
        if ( path == "/see-other" )
            p.push_back( Response( 303, "Location: echo\r\n", "" ) );
        else
        if ( path == "/temporary" )
            p.push_back( Response( 307, "Location: /echo\r\n", "" ) );
        else
        if ( path == "/echo" )
            p.push_back( Response( 200, "", method + "\n" + body ) );
        else
        if ( path == "/setcookie" )
            p.push_back( Response( 200, "Set-Cookie: session=abc; Path=/; HttpOnly\r\nSet-Cookie: theme=dark; Path=/app\r\n", "cookies set" ) );
        else
        if ( path == "/showcookies" || path == "/app/showcookies" )
            p.push_back( Response( 200, "", HeaderValue( headers, "Cookie" ) ) );
        else
        if ( path == "/range" )
        {
            CORE::CString content = "0123456789abcdefghijklmnopqrstuvwxyz";
            const CORE::CString range = HeaderValue( headers, "Range" );    // const: only the const C_String() never returns NULL
            unsigned long first = 0, last = 0;
            if ( 2 == sscanf( range.C_String(), "bytes=%lu-%lu", &first, &last ) )
                p.push_back( Response( 206, "", content.SubstrFromRange( (UInt32) first, (UInt32) ( last + 1 ) ) ) );
            else
            if ( 1 == sscanf( range.C_String(), "bytes=%lu-", &first ) )
                p.push_back( Response( 206, "", content.CutChars( (UInt32) first, true ) ) );
            else
                p.push_back( Response( 200, "", content ) );
        }
        else
        if ( path == "/large" )
        {
            GUCEF::vector< char > contentBytes( 2 * 1024 * 1024, 'x' );
            for ( size_t i=0; i<contentBytes.size(); i += 4096 )
                contentBytes[ i ] = (char) ( 'a' + ( i / 4096 ) % 26 );
            CORE::CString response = Response( 200, "", CORE::CString( &contentBytes[ 0 ], (UInt32) contentBytes.size() ) );
            UInt32 responseLength = response.Length();
            for ( UInt32 i=0; i<responseLength; i += 256 * 1024 )
                p.push_back( response.SubstrFromRange( i, i + 256 * 1024 ) );
        }
        else
        if ( path == "/status/404" )
            p.push_back( Response( 404, "", "not here" ) );
        else
        if ( path == "/head" )
            p.push_back( "HTTP/1.1 200 OK\r\nContent-Length: 100\r\n\r\n" );
        else
        if ( path == "/continue" )
        {
            p.push_back( "HTTP/1.1 100 Continue\r\n\r\n" );
            p.push_back( Response( 200, "", "after continue" ) );
        }
        else
        if ( path == "/slow" )
        {
            // never answers
            state.closeAfterAnswer = false;
        }
        else
            p.push_back( Response( 404, "", "unknown" ) );
    }

    CORE::PulseGeneratorPtr m_pulseGenerator;
    COMCORE::CTCPServerSocket m_socket;
    volatile bool m_running;
    std::thread m_thread;
    GUCEF::map< void*, SConnection > m_connections;
    MT::CMutex m_lock;
    UInt32 m_requestCount;
    CORE::CString m_lastCookieHeader;
    CORE::CString m_lastRequestHeaders;
    CORE::CString m_secureBaseUrl;
};

/*-------------------------------------------------------------------------*/

/** a source of the given text which, like a stream, does not tell its size */
class CStreamSource : public CORE::CIOAccess
{
    public:

    CStreamSource( const CORE::CString& text ) : m_text( text ), m_pos( 0 ) {}

    virtual void Open( void ) GUCEF_VIRTUAL_OVERRIDE { m_pos = 0; }
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual bool Opened( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual UInt32 Read( void* dest, UInt32 esize, UInt32 elements ) GUCEF_VIRTUAL_OVERRIDE
    {
        size_t count = std::min( (size_t) esize * elements, (size_t) TextByteCount( m_text ) - m_pos );
        memcpy( dest, m_text.C_String() + m_pos, count );
        m_pos += count;
        return esize > 0 ? (UInt32) ( count / esize ) : 0;
    }
    virtual UInt64 Tell( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_pos; }
    virtual Int32 Seek( Int64 offset, Int32 origin ) GUCEF_VIRTUAL_OVERRIDE { return -1; }
    virtual char GetChar( void ) GUCEF_VIRTUAL_OVERRIDE { char c = 0; Read( &c, 1, 1 ); return c; }
    virtual bool Eof( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_pos >= TextByteCount( m_text ); }
    virtual bool IsReadable( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual bool IsWriteable( void ) const GUCEF_VIRTUAL_OVERRIDE { return false; }
    virtual bool IsValid( void ) GUCEF_VIRTUAL_OVERRIDE { return true; }
    virtual void Flush( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual UInt64 GetSize( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0; }
    virtual MT::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CStreamSource( m_text ); }

    private:

    CORE::CString m_text;
    size_t m_pos;
};

/*-------------------------------------------------------------------------*/

/** records the events of the classic API */
class CClassicRecorder : public CORE::CObserver
{
    public:

    CClassicRecorder( void ) : events(), body(), contentSize( 0 ), httpCode( 0 ), errorCode( 0 ), finished( false ), disconnected( false ) {}

    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( WEB::CHTTPClient::ConnectingEvent == eventId ) events.push_back( "connecting" );
        else if ( WEB::CHTTPClient::ConnectedEvent == eventId ) events.push_back( "connected" );
        else if ( WEB::CHTTPClient::HTTPContentEvent == eventId )
        {
            events.push_back( "content" );
            WEB::CHTTPClient::THTTPContentEventData* data = static_cast< WEB::CHTTPClient::THTTPContentEventData* >( eventData );
            contentSize = data->GetData().contentSize;
            httpCode = data->GetData().HTTPcode;
        }
        else if ( WEB::CHTTPClient::HTTPDataRecievedEvent == eventId )
        {
            WEB::CHTTPClient::THTTPDataRecievedEventData* data = static_cast< WEB::CHTTPClient::THTTPDataRecievedEventData* >( eventData );
            body += ToCString( data->GetData() );
        }
        else if ( WEB::CHTTPClient::HTTPErrorEvent == eventId )
        {
            events.push_back( "error" );
            errorCode = static_cast< WEB::CHTTPClient::THTTPErrorEventData* >( eventData )->GetData();
        }
        else if ( WEB::CHTTPClient::DisconnectedEvent == eventId ) { events.push_back( "disconnected" ); disconnected = true; }
        else if ( WEB::CHTTPClient::HTTPTransferFinishedEvent == eventId ) { events.push_back( "finished" ); finished = true; }
        else if ( WEB::CHTTPClient::ConnectionErrorEvent == eventId ) { events.push_back( "connectionerror" ); disconnected = true; }
    }

    GUCEF::vector< CORE::CString > events;
    CORE::CString body;
    UInt32 contentSize;
    int httpCode;
    int errorCode;
    bool finished;
    bool disconnected;
};

/*-------------------------------------------------------------------------*/

/** where MakeRequest() points the requests: empty for the scripted server itself, else the base url of a TLS terminator in front of it */
static CORE::CString g_scriptedBaseUrl;

static WEB::CHttpRequestData
MakeRequest( const CORE::CString& path, const CORE::CString& method = "GET" )
{
    WEB::CHttpRequestData request;
    request.SetUrl( g_scriptedBaseUrl.IsNULLOrEmpty() ? "http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + path : g_scriptedBaseUrl + path );
    request.requestType = method;
    return request;
}

/*-------------------------------------------------------------------------*/

/**
 *  A backend which does not use the network: each request is answered at once with a redirect to the
 *  next location of a script, and finally with a 200. It records the URL and headers of every request
 *  so that tests can see what a redirect chain sent where, including https which the test servers cannot do.
 */
class CScriptedRedirectBackend : public WEB::CIHttpClientBackend
{
    public:

    static const CORE::CString BackendName;

    struct SHop
    {
        CORE::CString url;
        WEB::CHttpRequestData::TStringMap headers;
    };

    static GUCEF::vector< CORE::CString > script;       // the Location of the response to each request, none left means 200
    static GUCEF::vector< SHop > hops;

    static bool HopHasHeader( size_t hop, const CORE::CString& name )
    {
        if ( hop >= hops.size() )
            return false;
        for ( WEB::CHttpRequestData::TStringMap::const_iterator i = hops[ hop ].headers.begin(); i != hops[ hop ].headers.end(); ++i )
        {
            if ( (*i).first.Equals( name, false ) )
                return true;
        }
        return false;
    }

    virtual CORE::CString GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE { return BackendName; }

    virtual bool SupportsScheme( const CORE::CString& scheme ) const GUCEF_VIRTUAL_OVERRIDE { return scheme == "http" || scheme == "https"; }

    virtual WEB::CIHttpClientBackendRequestPtr StartRequest( const WEB::CHttpRequestData& request          ,
                                                             const WEB::CHttpClientRequestOptions& options ,
                                                             WEB::CHttpResponseData& response              ,
                                                             WEB::CHttpClientTransferInfo& transferInfo    ,
                                                             CORE::PulseGeneratorPtr pulseGenerator        ,
                                                             WEB::CIHttpClientBackendObserver& observer    ) GUCEF_VIRTUAL_OVERRIDE
    {
        SHop hop;
        hop.url = request.GetUrl();
        hop.headers = request.otherHeaders;
        size_t hopIndex = hops.size();
        hops.push_back( hop );

        if ( hopIndex < script.size() )
        {
            response.statusCode = 302;
            response.location = script[ hopIndex ];
        }
        else
            response.statusCode = 200;
        transferInfo.success = true;
        return WEB::CIHttpClientBackendRequestPtr( GUCEF_NEW CScriptedRedirectRequest() );
    }

    private:

    class CScriptedRedirectRequest : public WEB::CIHttpClientBackendRequest
    {
        public:
        virtual bool IsFinished( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
        virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }
        virtual void Cancel( void ) GUCEF_VIRTUAL_OVERRIDE {}
    };
};

typedef CORE::CTFactory< WEB::CIHttpClientBackend, CScriptedRedirectBackend, MT::CMutex > TScriptedRedirectBackendFactory;
static TScriptedRedirectBackendFactory g_scriptedRedirectBackendFactory;

const CORE::CString CScriptedRedirectBackend::BackendName = "scriptedredirect";
GUCEF::vector< CORE::CString > CScriptedRedirectBackend::script;
GUCEF::vector< CScriptedRedirectBackend::SHop > CScriptedRedirectBackend::hops;

/*-------------------------------------------------------------------------*/

/** the names of the headers a caller puts on a request which are credentials, or hold them */
static void
AddCredentialHeaders( WEB::CHttpRequestData& request )
{
    request.otherHeaders[ "Authorization" ] = "Bearer SECRET-AUTHORIZATION";
    request.otherHeaders[ "Proxy-Authorization" ] = "Basic SECRET-PROXY";
    request.otherHeaders[ "Cookie" ] = "own=SECRET-COOKIE";
    request.otherHeaders[ "X-WebApi-Key" ] = "SECRET-WEBAPI-KEY";
    request.otherHeaders[ "x-auth-token" ] = "SECRET-TOKEN";
    request.otherHeaders[ "X-Session-Id" ] = "SECRET-SESSION";
    request.otherHeaders[ "X-Custom-Safe" ] = "harmless";
}

/*-------------------------------------------------------------------------*/

static bool
ReceivedCredentials( CScriptedHttpServer& server )
{
    return server.LastRequestHadHeader( "Authorization" )       ||
           server.LastRequestHadHeader( "Proxy-Authorization" ) ||
           server.LastRequestHadHeader( "Cookie" )              ||
           server.LastRequestHadHeader( "X-WebApi-Key" )        ||
           server.LastRequestHadHeader( "X-Auth-Token" )        ||
           server.LastRequestHadHeader( "X-Session-Id" );
}

/*-------------------------------------------------------------------------*/

static bool
ReceivedAllCredentials( CScriptedHttpServer& server )
{
    return server.LastRequestHadHeader( "Authorization" )       &&
           server.LastRequestHadHeader( "Proxy-Authorization" ) &&
           server.LastRequestHadHeader( "Cookie" )              &&
           server.LastRequestHadHeader( "X-WebApi-Key" )        &&
           server.LastRequestHadHeader( "X-Auth-Token" )        &&
           server.LastRequestHadHeader( "X-Session-Id" );
}

/*-------------------------------------------------------------------------*/

/** credentials follow a redirect only as long as the origin, being scheme, host and port, stays the same */
static void
PerformRedirectCredentialTests( WEB::CHTTPClient& client         ,
                                const CORE::CString& backendName ,
                                CScriptedHttpServer& server      ,
                                CScriptedHttpServer& otherOrigin )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Credentials are sent along a redirect within the same origin" ).C_String() )
        try
        {
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;

            const char* paths[] = { "/to-same-origin", "/to-same-origin-relative" };
            for ( int i=0; i<2; ++i )
            {
                WEB::CHttpRequestData request = MakeRequest( paths[ i ] );
                AddCredentialHeaders( request );
                ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
                ASSERT_TRUE( 1 == info.redirectCount && SameText( ToCString( response.content ), "ok" ) );
                ASSERT_TRUE( ReceivedAllCredentials( server ) );
                ASSERT_TRUE( SameText( server.GetLastRequestHeader( "Authorization" ), "Bearer SECRET-AUTHORIZATION" ) );
                ASSERT_TRUE( SameText( server.GetLastRequestHeader( "Cookie" ), "own=SECRET-COOKIE" ) );
                ASSERT_TRUE( server.LastRequestHadHeader( "X-Custom-Safe" ) );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Credentials are not sent to another origin after a redirect" ).C_String() )
        try
        {
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;

            // same host, other port: another origin
            const char* paths[] = { "/to-other-origin", "/to-other-origin-307" };
            for ( int i=0; i<2; ++i )
            {
                WEB::CHttpRequestData request = MakeRequest( paths[ i ] );
                AddCredentialHeaders( request );
                ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
                ASSERT_TRUE( 1 == info.redirectCount && SameText( ToCString( response.content ), "ok" ) );
                ASSERT_FALSE( ReceivedCredentials( otherOrigin ) );
                ASSERT_FALSE( otherOrigin.LastRequestHadHeader( "Authorization" ) );
                ASSERT_FALSE( otherOrigin.LastRequestHadHeader( "Proxy-Authorization" ) );
                ASSERT_FALSE( otherOrigin.LastRequestHadHeader( "Cookie" ) );
                ASSERT_FALSE( otherOrigin.LastRequestHadHeader( "X-WebApi-Key" ) );
                ASSERT_FALSE( otherOrigin.LastRequestHadHeader( "X-Auth-Token" ) );
                ASSERT_FALSE( otherOrigin.LastRequestHadHeader( "X-Session-Id" ) );

                // headers which are no credentials are still sent
                ASSERT_TRUE( otherOrigin.LastRequestHadHeader( "X-Custom-Safe" ) );
                ASSERT_TRUE( otherOrigin.LastRequestHadHeader( "Host" ) );
            }

            // other host, same port: another origin
            WEB::CHttpRequestData request = MakeRequest( "/to-other-host" );
            AddCredentialHeaders( request );
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 1 == info.redirectCount && SameText( ToCString( response.content ), "ok" ) );
            ASSERT_FALSE( ReceivedCredentials( server ) );
            ASSERT_TRUE( server.LastRequestHadHeader( "X-Custom-Safe" ) );

            // what was taken away stays away when the redirect leads back to where it came from
            request = MakeRequest( "/to-other-origin-and-back" );
            AddCredentialHeaders( request );
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 2 == info.redirectCount && SameText( ToCString( response.content ), "ok" ) );
            ASSERT_FALSE( ReceivedCredentials( otherOrigin ) );
            ASSERT_FALSE( ReceivedCredentials( server ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Cookies of the jar follow the host, those of the caller the origin" ).C_String() )
        try
        {
            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            client.SetCookieJar( jar );

            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_TRUE( client.Execute( MakeRequest( "/setcookie" ), response ) );
            ASSERT_TRUE( 2 == jar->GetCookieCount() );

            // 127.0.0.2 is another host, nothing which belongs to 127.0.0.1 may arrive there
            WEB::CHttpRequestData request = MakeRequest( "/to-other-host" );
            request.otherHeaders[ "Cookie" ] = "own=SECRET-COOKIE";
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 1 == info.redirectCount );
            ASSERT_FALSE( server.LastRequestHadHeader( "Cookie" ) );

            // another port of the same host: cookies are not scoped by port (RFC 6265), the one of the caller is
            request = MakeRequest( "/to-other-origin" );
            request.otherHeaders[ "Cookie" ] = "own=SECRET-COOKIE";
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 1 == info.redirectCount );
            ASSERT_TRUE( SameText( otherOrigin.GetLastRequestHeader( "Cookie" ), "session=abc" ) );

            // within the origin both are sent
            request = MakeRequest( "/to-same-origin" );
            request.otherHeaders[ "Cookie" ] = "own=SECRET-COOKIE";
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( SameText( server.GetLastRequestHeader( "Cookie" ), "own=SECRET-COOKIE; session=abc" ) );

            client.SetCookieJar( WEB::CHttpCookieJarPtr() );
        }
        catch ( ... )
        {
            client.SetCookieJar( WEB::CHttpCookieJarPtr() );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** the origin rules which do not need a server, including https which the test servers cannot do */
static void
PerformRedirectOriginTests( WEB::CHTTPClient& client )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: Origins are compared by scheme, host and port" )
        try
        {
            ASSERT_TRUE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org", "http", "example.org:80" ) );
            ASSERT_TRUE( WEB::CHttpRequestData::IsSameOrigin( "https", "example.org", "HTTPS", "EXAMPLE.org:443" ) );
            ASSERT_TRUE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org:8080", "http", "example.org:8080" ) );
            ASSERT_TRUE( WEB::CHttpRequestData::IsSameOrigin( "http", "[::1]:8080", "http", "[::1]:8080" ) );
            ASSERT_TRUE( WEB::CHttpRequestData::IsSameOrigin( "http", "user:pw@example.org", "http", "example.org" ) );

            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "https", "example.org", "http", "example.org" ) );          // downgrade
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org", "https", "example.org" ) );          // upgrade
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "https", "example.org:80", "http", "example.org:80" ) );
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org", "http", "example.org:8080" ) );
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org", "http", "www.example.org" ) );
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org", "http", "example.org.evil.org" ) );
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "[::1]:8080", "http", "[::1]:8081" ) );
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "example.org", "http", "example.org:x" ) );
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "", "http", "" ) );

            // the part which looks like a host:port in front of the @ is the user info, the host is what follows it
            ASSERT_FALSE( WEB::CHttpRequestData::IsSameOrigin( "http", "good.example:80@evil.example", "http", "good.example" ) );
            ASSERT_TRUE( WEB::CHttpRequestData::IsSameOrigin( "http", "good.example:80@evil.example", "http", "evil.example" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Credentials are not sent along a redirect from https to http or to another host" )
        try
        {
            WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().RegisterConcreteFactory( CScriptedRedirectBackend::BackendName, &g_scriptedRedirectBackendFactory );
            CORE::CString::StringVector scripted;
            scripted.push_back( CScriptedRedirectBackend::BackendName );
            client.SetPreferredBackends( scripted );

            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;

            // downgrade of the same host
            CScriptedRedirectBackend::hops.clear();
            CScriptedRedirectBackend::script.clear();
            CScriptedRedirectBackend::script.push_back( "http://secure.example/next" );
            WEB::CHttpRequestData request;
            request.SetUrl( "https://secure.example/start" );
            AddCredentialHeaders( request );
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 2 == CScriptedRedirectBackend::hops.size() );
            ASSERT_TRUE( CScriptedRedirectBackend::hops[ 1 ].url == "http://secure.example/next" );
            ASSERT_TRUE( CScriptedRedirectBackend::HopHasHeader( 0, "Authorization" ) && CScriptedRedirectBackend::HopHasHeader( 0, "Cookie" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Authorization" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Proxy-Authorization" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Cookie" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "X-WebApi-Key" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "X-Auth-Token" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "X-Session-Id" ) );
            ASSERT_TRUE( CScriptedRedirectBackend::HopHasHeader( 1, "X-Custom-Safe" ) );

            // upgrade to https and to the default port spelled out stay within or leave the origin as per scheme
            CScriptedRedirectBackend::hops.clear();
            CScriptedRedirectBackend::script.clear();
            CScriptedRedirectBackend::script.push_back( "https://secure.example:443/next" );
            request.SetUrl( "https://secure.example/start" );
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 2 == CScriptedRedirectBackend::hops.size() );
            ASSERT_TRUE( CScriptedRedirectBackend::HopHasHeader( 1, "Authorization" ) && CScriptedRedirectBackend::HopHasHeader( 1, "Cookie" ) &&
                         CScriptedRedirectBackend::HopHasHeader( 1, "X-WebApi-Key" ) );

            CScriptedRedirectBackend::hops.clear();
            CScriptedRedirectBackend::script.clear();
            CScriptedRedirectBackend::script.push_back( "https://secure.example/next" );
            request.SetUrl( "http://secure.example/start" );
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 2 == CScriptedRedirectBackend::hops.size() );
            ASSERT_TRUE( CScriptedRedirectBackend::HopHasHeader( 0, "Authorization" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Authorization" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Cookie" ) );

            // another host, and the user info trick which makes an address look like it is still the first host
            CScriptedRedirectBackend::hops.clear();
            CScriptedRedirectBackend::script.clear();
            CScriptedRedirectBackend::script.push_back( "https://secure.example:443@evil.example/next" );
            request.SetUrl( "https://secure.example/start" );
            ASSERT_TRUE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 2 == CScriptedRedirectBackend::hops.size() );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Authorization" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "Cookie" ) );
            ASSERT_FALSE( CScriptedRedirectBackend::HopHasHeader( 1, "X-WebApi-Key" ) );

            client.SetPreferredBackends( WEB::CHttpClientBackendSelection::GetDefaultPreferredBackends() );
            WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().UnregisterConcreteFactory( CScriptedRedirectBackend::BackendName );
        }
        catch ( ... )
        {
            client.SetPreferredBackends( WEB::CHttpClientBackendSelection::GetDefaultPreferredBackends() );
            WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().UnregisterConcreteFactory( CScriptedRedirectBackend::BackendName );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformUrlAndCookieTests( void )
{
    GUCEF_TESTFW_TESTCASE( "Test: URLs are split into the request fields and put back together" )
        try
        {
            WEB::CHttpRequestData request;
            ASSERT_TRUE( request.SetUrl( "HTTPS://Example.org:8443/a/b.html?x=1&y=2#frag" ) );
            ASSERT_TRUE( request.requestProtocol == "https" );
            ASSERT_TRUE( request.requestHost == "Example.org:8443" );
            ASSERT_TRUE( request.requestUri == "/a/b.html" );
            ASSERT_TRUE( request.requestUriParams == "x=1&y=2" );
            ASSERT_TRUE( request.GetHostName() == "Example.org" && 8443 == request.GetPort() );
            ASSERT_TRUE( request.GetUrl() == "https://Example.org:8443/a/b.html?x=1&y=2" );

            ASSERT_TRUE( request.SetUrl( "http://host" ) );
            ASSERT_TRUE( request.requestUri == "/" && 80 == request.GetPort() && request.GetUrl() == "http://host/" );
            ASSERT_TRUE( request.SetUrl( "https://host?q" ) );
            ASSERT_TRUE( request.requestUri == "/" && request.requestUriParams == "q" && 443 == request.GetPort() );
            ASSERT_TRUE( request.SetUrl( "http://[::1]:8080/x" ) );
            ASSERT_TRUE( request.GetHostName() == "::1" && 8080 == request.GetPort() );
            ASSERT_FALSE( request.SetUrl( "no scheme" ) );
            ASSERT_FALSE( request.SetUrl( "http:///path" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: The cookie jar applies the rules of RFC 6265" )
        try
        {
            WEB::CHttpCookieJar jar;
            ASSERT_TRUE( jar.SetCookieFromHeader( "a=1", "www.example.org", "/docs/page", false ) );
            ASSERT_TRUE( jar.SetCookieFromHeader( "b=2; Domain=.example.org; Path=/", "www.example.org", "/", false ) );
            ASSERT_TRUE( jar.SetCookieFromHeader( "c=3; Path=/docs/deep; Secure", "www.example.org", "/", true ) );
            ASSERT_TRUE( jar.SetCookieFromHeader( "d=4; Max-Age=0", "www.example.org", "/", false ) );     // expired at once
            ASSERT_FALSE( jar.SetCookieFromHeader( "e=5; Domain=other.org", "www.example.org", "/", false ) );
            ASSERT_FALSE( jar.SetCookieFromHeader( "f=6; Domain=org", "www.example.org", "/", false ) );
            ASSERT_FALSE( jar.SetCookieFromHeader( "g=7; Secure", "www.example.org", "/", false ) );     // secure needs https
            ASSERT_FALSE( jar.SetCookieFromHeader( "novalue", "www.example.org", "/", false ) );
            ASSERT_TRUE( 3 == jar.GetCookieCount() );

            // a (host only, default path /docs), b (domain wide), c (secure, /docs/deep)
            ASSERT_TRUE( jar.GetCookieHeaderValue( "www.example.org", "/docs/deep/x", true ) == "c=3; a=1; b=2" );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "www.example.org", "/docs/deep/x", false ) == "a=1; b=2" );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "sub.example.org", "/docs", false ) == "b=2" );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "www.example.org", "/documents", false ) == "b=2" );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "notexample.org", "/", false ).IsNULLOrEmpty() );

            // replacing and expiring
            ASSERT_TRUE( jar.SetCookieFromHeader( "b=22; Domain=example.org; Path=/", "example.org", "/", false ) );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "example.org", "/", false ) == "b=22" );
            ASSERT_TRUE( jar.SetCookieFromHeader( "b=gone; Domain=example.org; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT", "example.org", "/", false ) );
            ASSERT_TRUE( jar.GetCookieHeaderValue( "example.org", "/", false ).IsNULLOrEmpty() );

            // persistent cookies survive a save and load, session cookies are kept as well
            ASSERT_TRUE( jar.SetCookieFromHeader( "p=persist; Expires=Wed, 09 Jun 2100 10:18:14 GMT; HttpOnly", "www.example.org", "/", false ) );
            CORE::CString path = CORE::CombinePath( CORE::TempDir(), "gucefWEB_TestApp_cookies.txt" );
            ASSERT_TRUE( jar.SaveToNetscapeFile( path ) );
            WEB::CHttpCookieJar loaded;
            ASSERT_TRUE( loaded.LoadFromNetscapeFile( path ) );
            ASSERT_TRUE( loaded.GetCookieCount() == jar.GetCookieCount() );
            ASSERT_TRUE( loaded.GetCookieHeaderValue( "www.example.org", "/docs/deep/x", true ) == jar.GetCookieHeaderValue( "www.example.org", "/docs/deep/x", true ) );
            WEB::CHttpCookieJar::TCookieVector cookies;
            loaded.GetAllCookies( cookies );
            bool foundPersistent = false;
            for ( size_t i=0; i<cookies.size(); ++i )
            {
                if ( cookies[ i ].name == "p" )
                {
                    foundPersistent = true;
                    ASSERT_TRUE( cookies[ i ].httpOnly && cookies[ i ].isPersistent && 2100 == cookies[ i ].expires.GetYear() );
                }
            }
            ASSERT_TRUE( foundPersistent );
            loaded.RemoveSessionCookies();
            ASSERT_TRUE( 1 == loaded.GetCookieCount() );
            CORE::DeleteFile( path );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/** loads a plugin from the module directory, the debug build of it is tried first in debug builds */
static bool
LoadHttpClientPlugin( const CORE::CString& pluginName )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName;
    #else
    CORE::CString firstPluginPath = "$MODULEDIR$/" + pluginName;
    CORE::CString secondPluginPath = "$MODULEDIR$/" + pluginName + "_d";
    #endif

    // a group of its own so that it can be unloaded without taking the plugins of other tests with it
    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( firstPluginPath, pluginName + "_tests", true ) )
        return true;
    return pluginControl.AddPluginFromDir( secondPluginPath, pluginName + "_tests", true );
}

/*-------------------------------------------------------------------------*/

/** runs the tests which apply to every backend, with the client using only the given backend */
static void
PerformBackendTests( WEB::CHTTPClient& client, const CORE::CString& backendName, CScriptedHttpServer& server, CScriptedHttpServer& otherOrigin )
{GUCEF_TRACE;

    CORE::CString::StringVector preferred;
    preferred.push_back( backendName );
    client.SetPreferredBackends( preferred );

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The backend is chosen for http" ).C_String() )
        try
        {
            ASSERT_TRUE( client.GetBackendNameForScheme( "http" ) == backendName );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Execute reads responses split over reads, chunked and close delimited" ).C_String() )
        try
        {
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_TRUE( client.Execute( MakeRequest( "/hello" ), client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 200 == response.statusCode && SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( response.contentType == "text/plain" );
            ASSERT_TRUE( info.success && info.backendName == backendName && 0 == info.redirectCount );
            ASSERT_TRUE( info.bytesReceived > 11 && info.bytesSent > 0 );

            WEB::CHttpResponseData chunked;
            ASSERT_TRUE( client.Execute( MakeRequest( "/chunked" ), chunked ) );
            ASSERT_TRUE( 200 == chunked.statusCode && SameText( ToCString( chunked.content ), "The quick brown fox jumps over the lazy dog!!!!" ) );

            WEB::CHttpResponseData closed;
            ASSERT_TRUE( client.Execute( MakeRequest( "/close" ), closed ) );
            ASSERT_TRUE( SameText( ToCString( closed.content ), "until the end" ) );

            WEB::CHttpResponseData interim;
            ASSERT_TRUE( client.Execute( MakeRequest( "/continue" ), interim ) );
            ASSERT_TRUE( 200 == interim.statusCode && SameText( ToCString( interim.content ), "after continue" ) );

            WEB::CHttpResponseData head;
            ASSERT_TRUE( client.Execute( MakeRequest( "/head", "HEAD" ), head ) );
            ASSERT_TRUE( 200 == head.statusCode && 0 == head.content.GetDataSize() );

            WEB::CHttpResponseData notFound;
            ASSERT_TRUE( client.Execute( MakeRequest( "/status/404" ), notFound ) );     // a response is a success
            ASSERT_TRUE( 404 == notFound.statusCode && SameText( ToCString( notFound.content ), "not here" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Redirects are followed and rewritten as required" ).C_String() )
        try
        {
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_TRUE( client.Execute( MakeRequest( "/redirect/1" ), client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 200 == response.statusCode && SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( 2 == info.redirectCount && info.effectiveUrl.HasSubstr( "/hello", true ) > 0 );

            // without following the redirect itself is the response, including its body
            WEB::CHttpClientRequestOptions noRedirects = client.GetDefaultOptions();
            noRedirects.followRedirects = false;
            WEB::CHttpResponseData redirect;
            ASSERT_TRUE( client.Execute( MakeRequest( "/redirect/1" ), noRedirects, redirect ) );
            ASSERT_TRUE( 302 == redirect.statusCode && redirect.location == "/redirect/2" && SameText( ToCString( redirect.content ), "go on" ) );

            // endless redirects end in an error
            WEB::CHttpClientRequestOptions fewRedirects = client.GetDefaultOptions();
            fewRedirects.maxRedirects = 3;
            ASSERT_FALSE( client.Execute( MakeRequest( "/loop" ), fewRedirects, response, &info ) );
            ASSERT_TRUE( 3 == info.redirectCount );

            // 303 continues as GET without the body, 307 repeats the POST with its body
            WEB::CHttpRequestData post = MakeRequest( "/see-other", "POST" );
            post.content.CopyFrom( 4, "data" );
            WEB::CHttpResponseData seeOther;
            ASSERT_TRUE( client.Execute( post, seeOther ) );
            ASSERT_TRUE( SameText( ToCString( seeOther.content ), "GET\n" ) );

            WEB::CHttpRequestData temporary = MakeRequest( "/temporary", "POST" );
            temporary.content.CopyFrom( 4, "data" );
            WEB::CHttpResponseData repeated;
            ASSERT_TRUE( client.Execute( temporary, repeated ) );
            ASSERT_TRUE( SameText( ToCString( repeated.content ), "POST\ndata" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Cookies from responses are sent with later requests" ).C_String() )
        try
        {
            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            client.SetCookieJar( jar );

            WEB::CHttpResponseData response;
            ASSERT_TRUE( client.Execute( MakeRequest( "/setcookie" ), response ) );
            ASSERT_TRUE( 2 == jar->GetCookieCount() );

            ASSERT_TRUE( client.Execute( MakeRequest( "/showcookies" ), response ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "session=abc" ) );
            ASSERT_TRUE( client.Execute( MakeRequest( "/app/showcookies" ), response ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "theme=dark; session=abc" ) );

            // a cookie header given by the caller is combined with the jar
            WEB::CHttpRequestData withCookie = MakeRequest( "/showcookies" );
            withCookie.otherHeaders[ "Cookie" ] = "own=1";
            ASSERT_TRUE( client.Execute( withCookie, response ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "own=1; session=abc" ) );

            client.SetCookieJar( WEB::CHttpCookieJarPtr() );
            ASSERT_TRUE( client.Execute( MakeRequest( "/showcookies" ), response ) );
            ASSERT_TRUE( 0 == response.content.GetDataSize() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Ranges, streamed downloads and streamed uploads" ).C_String() )
        try
        {
            WEB::CHttpClientRequestOptions range = client.GetDefaultOptions();
            range.hasRange = true;
            range.rangeStart = 10;
            range.rangeLength = 5;
            WEB::CHttpResponseData partial;
            ASSERT_TRUE( client.Execute( MakeRequest( "/range" ), range, partial ) );
            ASSERT_TRUE( 206 == partial.statusCode && SameText( ToCString( partial.content ), "abcde" ) );
            range.rangeLength = -1;
            range.rangeStart = 30;
            ASSERT_TRUE( client.Execute( MakeRequest( "/range" ), range, partial ) );
            ASSERT_TRUE( SameText( ToCString( partial.content ), "uvwxyz" ) );

            // the body goes to the sink, not into the response
            CORE::CDynamicBuffer downloaded;
            WEB::CHttpClientRequestOptions toSink = client.GetDefaultOptions();
            toSink.responseBodySink = CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( &downloaded, false ) );
            WEB::CHttpResponseData large;
            ASSERT_TRUE( client.Execute( MakeRequest( "/large" ), toSink, large ) );
            ASSERT_TRUE( 0 == large.content.GetDataSize() );
            ASSERT_TRUE( 2 * 1024 * 1024 == downloaded.GetDataSize() );
            ASSERT_TRUE( 'a' == downloaded.AsConstTypePtr< char >()[ 0 ] && 'b' == downloaded.AsConstTypePtr< char >()[ 4096 ] && 'x' == downloaded.AsConstTypePtr< char >()[ 1 ] );

            // a body of unknown size is sent chunked
            WEB::CHttpClientRequestOptions upload = client.GetDefaultOptions();
            upload.requestBody = CORE::IOAccessPtr( GUCEF_NEW CStreamSource( "streamed request body" ) );
            WEB::CHttpResponseData echo;
            ASSERT_TRUE( client.Execute( MakeRequest( "/echo", "PUT" ), upload, echo ) );
            ASSERT_TRUE( SameText( ToCString( echo.content ), "PUT\nstreamed request body" ) );

            // and one of known size with a Content-Length
            CORE::CDynamicBuffer source;
            source.CopyFrom( 10, "known size" );
            upload.requestBody = CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( source ) );
            ASSERT_TRUE( client.Execute( MakeRequest( "/echo", "POST" ), upload, echo ) );
            ASSERT_TRUE( SameText( ToCString( echo.content ), "POST\nknown size" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): Timeouts and connection failures are reported" ).C_String() )
        try
        {
            WEB::CHttpClientRequestOptions quick = client.GetDefaultOptions();
            quick.totalTimeoutInMs = 300;
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_FALSE( client.Execute( MakeRequest( "/slow" ), quick, response, &info ) );
            ASSERT_TRUE( info.timedOut && !info.success && info.durationInMs >= 300 && info.durationInMs < 5000 );

            WEB::CHttpRequestData nowhere;
            nowhere.SetUrl( "http://127.0.0.1:" + CORE::ToString( NOTHING_LISTENS_PORT ) + "/" );
            ASSERT_FALSE( client.Execute( nowhere, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( !info.success && !info.errorDescription.IsNULLOrEmpty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): ExecuteAsync carries out requests on a thread pool and returns futures" ).C_String() )
        try
        {
            client.SetThreadPoolName( "HttpClientTestPool" );

            // several at the same time
            GUCEF::vector< CORE::CFutureResult > futures;
            for ( int i=0; i<5; ++i )
                futures.push_back( client.ExecuteAsync( MakeRequest( i % 2 ? "/hello" : "/chunked" ) ) );
            for ( size_t i=0; i<futures.size(); ++i )
            {
                CORE::CTaskPtr task = futures[ i ].GetResult( 10000 );
                WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( task );
                ASSERT_TRUE( GUCEF_NULL != data );
                if ( GUCEF_NULL != data )
                {
                    ASSERT_TRUE( data->transferInfo.success && 200 == data->response.statusCode );
                    ASSERT_TRUE( SameText( ToCString( data->response.content ), ( i % 2 ? "hello world" : "The quick brown fox jumps over the lazy dog!!!!" ) ) );
                }
            }

            // cancelling the future cancels the request
            CORE::CFutureResult slow = client.ExecuteAsync( MakeRequest( "/slow" ) );
            MT::ThreadDelay( 200 );
            ASSERT_TRUE( slow.RequestCancellation() );
            CORE::CTaskPtr task = slow.GetResult( 10000 );
            WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( task );
            ASSERT_TRUE( GUCEF_NULL != data && data->transferInfo.wasCancelled && !data->transferInfo.success );

            client.SetThreadPool( CORE::ThreadPoolPtr() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): The classic API raises its events as before" ).C_String() )
        try
        {
            CClassicRecorder recorder;
            recorder.SubscribeTo( &client );
            ASSERT_TRUE( client.Get( "http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/hello" ) );
            for ( int i=0; i<5000 && !recorder.finished; ++i )
            {
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( recorder.finished && recorder.disconnected );
            ASSERT_TRUE( SameText( recorder.body, "hello world" ) );
            ASSERT_TRUE( 11 == recorder.contentSize && 200 == recorder.httpCode );
            ASSERT_TRUE( recorder.events.size() >= 5 );
            if ( recorder.events.size() >= 5 )
            {
                ASSERT_TRUE( recorder.events[ 0 ] == "connecting" && recorder.events[ 1 ] == "connected" && recorder.events[ 2 ] == "content" );
                ASSERT_TRUE( recorder.events[ recorder.events.size()-2 ] == "disconnected" && recorder.events.back() == "finished" );
            }

            // an error status is reported as such
            CClassicRecorder errorRecorder;
            errorRecorder.SubscribeTo( &client );
            recorder.UnsubscribeAllFromObserver();
            ASSERT_TRUE( client.Get( "127.0.0.1", SCRIPTED_SERVER_PORT, "/status/404" ) );
            for ( int i=0; i<5000 && !errorRecorder.disconnected; ++i )
            {
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( 404 == errorRecorder.errorCode && !errorRecorder.finished );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    PerformHttpRedactionBackendTests( client, backendName, NOTHING_LISTENS_PORT, SCRIPTED_SERVER_PORT );
    PerformRedirectCredentialTests( client, backendName, server, otherOrigin );
}

/*-------------------------------------------------------------------------*/

/** the http and https schemes are served by the URI resource accessors of the WEB module */
static void
PerformUriAccessorTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The http URI resource accessor reads, describes, writes and deletes resources" )
        try
        {
            CORE::CUriResourceAccessorFactory& factory = CORE::CCoreGlobal::Instance()->GetUriResourceAccessorFactory();
            ASSERT_TRUE( factory.IsConstructible( "http" ) && factory.IsConstructible( "https" ) );

            CORE::CUriResourceAccessorPtr accessor = factory.Create( "http" );
            ASSERT_FALSE( accessor.IsNULL() );
            if ( accessor.IsNULL() )
                return;

            CORE::CString base = "http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT );

            // whole resources
            CORE::CDynamicBuffer content;
            ASSERT_TRUE( accessor->GetResource( CORE::CUri( base + "/hello" ), content ) );
            ASSERT_TRUE( SameText( ToCString( content ), "hello world" ) );
            CORE::CDynamicBuffer missing;
            ASSERT_FALSE( accessor->GetResource( CORE::CUri( base + "/status/404" ), missing ) );

            // parts of resources
            CORE::CDynamicBuffer part;
            ASSERT_TRUE( accessor->GetPartialResource( CORE::CUri( base + "/range" ), 10, 5, part ) );
            ASSERT_TRUE( SameText( ToCString( part ), "abcde" ) );
            part.Clear();
            ASSERT_TRUE( accessor->GetPartialResource( CORE::CUri( base + "/range" ), 30, -1, part ) );
            ASSERT_TRUE( SameText( ToCString( part ), "uvwxyz" ) );

            // read access to a resource
            CORE::IOAccessPtr access;
            ASSERT_TRUE( accessor->GetResourceAccess( CORE::CUri( base + "/hello" ), access, CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_READ ) );
            ASSERT_FALSE( access.IsNULL() );
            if ( !access.IsNULL() )
            {
                char text[ 32 ] = { 0 };
                ASSERT_TRUE( 11 == access->Read( text, 1, 31 ) );
                ASSERT_TRUE( SameText( text, "hello world" ) );
            }
            ASSERT_FALSE( accessor->GetResourceAccess( CORE::CUri( base + "/hello" ), access, CORE::CUriResourceAccessor::URI_RESOURCEACCESS_MODE_WRITE ) );

            // meta data
            CORE::CResourceMetaData metaData;
            ASSERT_TRUE( accessor->GetResourceMetaData( CORE::CUri( base + "/hello" ), metaData ) );
            ASSERT_TRUE( metaData.hasResourceSizeInBytes && 11 == metaData.resourceSizeInBytes );
            ASSERT_TRUE( metaData.hasMimeType && metaData.mimeType == "text/plain" );
            ASSERT_TRUE( metaData.hasName && metaData.name == "hello" );

            // writing: the server answers a PUT to /echo with success and a DELETE as well
            CORE::CDynamicBuffer upload;
            upload.CopyFrom( 4, "data" );
            ASSERT_TRUE( accessor->UpdateResource( CORE::CUri( base + "/echo" ), upload ) );
            ASSERT_TRUE( accessor->CreateResource( CORE::CUri( base + "/echo" ), upload ) );
            ASSERT_TRUE( accessor->DeleteResource( CORE::CUri( base + "/echo" ) ) );
            ASSERT_FALSE( accessor->DeleteResource( CORE::CUri( base + "/status/404" ) ) );

            // what HTTP cannot do
            ASSERT_FALSE( accessor->UpdatePartialResource( CORE::CUri( base + "/echo" ), upload ) );
            ASSERT_FALSE( accessor->IsACollectionResource( CORE::CUri( base + "/" ) ) );
            CORE::CUriResourceAccessor::CUriResourceAccessorOperations ops;
            ASSERT_TRUE( accessor->GetSupportedOperations( CORE::CUri( base + "/hello" ), ops ) );
            ASSERT_TRUE( ops.getResource && ops.getPartialResource && ops.getResourceMetaData && ops.deleteResource && !ops.collectionResolution );

            // other schemes are refused
            ASSERT_FALSE( accessor->GetResource( CORE::CUri( "file:///etc/hostname" ), missing ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/**
 *  A https server on the loopback interface for the tests of the https support, nothing of the internet is involved.
 *  It is the openssl command line tool (s_server) with a throwaway self-signed certificate, so the tests skip when the tool
 *  is not there. The server answers the first connection with a fixed response and writes what the client sent to a file.
 */
class CLocalTlsServer
{
    public:

    CLocalTlsServer( void )
        : m_process()
        , m_requestLog()
    {
        m_process.SetIsolateFromParent( true );
    }

    ~CLocalTlsServer()
    {
        Stop();
    }

    /** whether the tools needed are there: the openssl command line tool, a shell and timeout */
    static bool ToolsAvailable( void )
    {
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
        return RunTool( "openssl", "version" ) && RunTool( "sh", "-c true" ) && RunTool( "timeout", "--foreground 1 true" );
        #else
        return false;
        #endif
    }

    /** a self-signed certificate for localhost and 127.0.0.1, or for the given name only */
    static bool MakeCertificate( const CORE::CString& dir, const CORE::CString& baseName, const CORE::CString& commonName, bool withAlternativeNames )
    {
        CORE::CProcess::TStringVector args;
        args.push_back( "req" );
        args.push_back( "-x509" );
        args.push_back( "-newkey" );
        args.push_back( "ec" );
        args.push_back( "-pkeyopt" );
        args.push_back( "ec_paramgen_curve:prime256v1" );
        args.push_back( "-nodes" );
        args.push_back( "-days" );
        args.push_back( "2" );
        args.push_back( "-keyout" );
        args.push_back( dir + "/" + baseName + ".key" );
        args.push_back( "-out" );
        args.push_back( dir + "/" + baseName + ".crt" );
        args.push_back( "-subj" );
        args.push_back( "/CN=" + commonName );
        if ( withAlternativeNames )
        {
            args.push_back( "-addext" );
            args.push_back( "subjectAltName=DNS:localhost,IP:127.0.0.1" );
        }
        CORE::CProcess process;
        process.SetIsolateFromParent( true );
        Int32 exitCode = -1;
        return process.Launch( "openssl", args ) && process.WaitForExit( 30000, exitCode ) && 0 == exitCode &&
               CORE::FileExists( dir + "/" + baseName + ".crt" ) && CORE::FileExists( dir + "/" + baseName + ".key" );
    }

    bool Start( const CORE::CString& dir, UInt16 port, const CORE::CString& baseName, const CORE::CString& responseBody )
    {
        CORE::CString portText = CORE::ToString( port );
        CORE::CString response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " + CORE::ToString( (UInt32) responseBody.Length() ) +
                                 "\r\nConnection: close\r\n\r\n" + responseBody;
        m_requestLog = dir + "/" + portText + ".request";
        CORE::CString responseFile = dir + "/" + portText + ".response";
        CORE::CString script = dir + "/" + portText + ".sh";
        CORE::DeleteFile( m_requestLog );
        CORE::CDynamicBuffer responseData;
        responseData.CopyFrom( response.Length(), response.C_String() );
        if ( !CORE::WriteFileAtomically( responseFile, responseData ) )
            return false;

        // The response is given to the client as soon as it is connected, the sleep keeps the input open until the client is done.
        // Stopping the script takes the whole group of processes down with it
        CORE::CString text = "#!/bin/sh\n"
                             "trap 'kill 0' TERM INT\n"
                             "( cat \"" + responseFile + "\"; sleep 30 ) | timeout --foreground 120 openssl s_server -accept " + portText +
                             " -cert \"" + dir + "/" + baseName + ".crt\" -key \"" + dir + "/" + baseName + ".key\""
                             " > \"" + m_requestLog + "\" 2> \"" + dir + "/" + portText + ".err\" &\n"
                             "wait\n";
        CORE::CDynamicBuffer scriptData;
        scriptData.CopyFrom( text.Length(), text.C_String() );
        if ( !CORE::WriteFileAtomically( script, scriptData ) )
            return false;
        CORE::CProcess::TStringVector args;
        args.push_back( script );
        if ( !m_process.Launch( "sh", args, CORE::CString(), CORE::CProcess::TStringMap(), true ) )
            return false;

        // The server tells that it is listening
        for ( UInt32 i=0; i<250; ++i )
        {
            CORE::CString log;
            if ( CORE::LoadTextFileAsString( m_requestLog, log ) && log.HasSubstr( "ACCEPT", true ) >= 0 )
                return true;
            if ( !m_process.IsRunning() )
                return false;
            MT::ThreadDelay( 20 );
        }
        return false;
    }

    void Stop( void )
    {
        m_process.Terminate( true, 2000 );
    }

    /** what the server received so far, the connection details of the tool included */
    CORE::CString GetReceived( void ) const
    {
        CORE::CString log;
        CORE::LoadTextFileAsString( m_requestLog, log );
        return log;
    }

    private:

    static bool RunTool( const CORE::CString& tool, const CORE::CString& arguments )
    {
        CORE::CProcess process;
        process.SetIsolateFromParent( true );
        Int32 exitCode = -1;
        return process.Launch( tool, CORE::CProcess::SplitCommandLine( arguments ) ) && process.WaitForExit( 10000, exitCode ) && 0 == exitCode;
    }

    CORE::CProcess m_process;
    CORE::CString m_requestLog;
};

/*-------------------------------------------------------------------------*/

/** a POST of a form like the one that checks an OpenID 2.0 assertion */
static bool
PostCheckAuthentication( WEB::CHTTPClient& client, const WEB::CHttpClientRequestOptions& options, UInt16 port, WEB::CHttpResponseData& response, WEB::CHttpClientTransferInfo& info )
{
    static const CORE::CString body = "openid.assoc_handle=1234567890&openid.mode=check_authentication&openid.ns=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0&openid.sig=ab%2Bc%2FdE%3D";
    WEB::CHttpRequestData request;
    request.requestType = "POST";
    request.resourceRepresentations.push_back( "application/json" );
    request.contentRepresentation = "application/x-www-form-urlencoded";
    request.content.CopyFrom( body.Length(), body.C_String() );
    request.SetUrl( "https://localhost:" + CORE::ToString( port ) + "/openid/login" );
    return client.Execute( request, options, response, &info );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
ContentAsString( const WEB::CHttpResponseData& response )
{
    return CORE::CString( static_cast< const char* >( response.content.GetConstBufferPtr() ), response.content.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

/** https requests through a backend against a server of our own: trusted and untrusted certificates, both without the internet */
static void
PerformLocalHttpsTests( WEB::CHTTPClient& client, const CORE::CString& backendName )
{GUCEF_TRACE;

    CORE::CString::StringVector preferred;
    preferred.push_back( backendName );
    client.SetPreferredBackends( preferred );

    static const UInt16 TRUSTED_PORT = 47610;
    static const UInt16 UNTRUSTED_PORT = 47611;
    static const UInt16 ENVIRONMENT_PORT = 47612;
    static const UInt16 WRONGNAME_PORT = 47613;
    static const UInt16 OTHERBUNDLE_PORT = 47614;
    static const CORE::CString answer = "ns:http://specs.openid.net/auth/2.0\nis_valid:true\n";

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): https works end to end with a trusted certificate and an untrusted one is refused" ).C_String() )
        try
        {
            ASSERT_TRUE( WEB::CHttpClientRequestOptions().verifyPeer );
            if ( !CLocalTlsServer::ToolsAvailable() )
                GUCEF_TESTFW_SKIP( "the openssl command line tool, sh or timeout is not available" );

            CORE::CString dir = CORE::TempDir() + "/gucefWEB_https_" + CORE::ToString( (UInt32) ( MT::PrecisionTickCount() % 1000000 ) );
            ASSERT_TRUE( CORE::CreatePathDirectories( dir + "/" ) );
            ASSERT_TRUE( CLocalTlsServer::MakeCertificate( dir, "good", "localhost", true ) );
            ASSERT_TRUE( CLocalTlsServer::MakeCertificate( dir, "wrongname", "wrong.example", false ) );

            WEB::CHttpClientRequestOptions options = client.GetDefaultOptions();
            options.connectTimeoutInMs = 5000;
            options.totalTimeoutInMs = 15000;

            // Trusted through the option caBundlePath
            {
                CLocalTlsServer tlsServer;
                ASSERT_TRUE( tlsServer.Start( dir, TRUSTED_PORT, "good", answer ) );
                WEB::CHttpClientRequestOptions trusting = options;
                trusting.caBundlePath = dir + "/good.crt";
                WEB::CHttpResponseData response;
                WEB::CHttpClientTransferInfo info;
                bool sent = PostCheckAuthentication( client, trusting, TRUSTED_PORT, response, info );
                if ( !sent )
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "HttpClient test: the https request failed: " << info.errorDescription );
                ASSERT_TRUE( sent );
                ASSERT_TRUE( 200 == response.statusCode );
                ASSERT_TRUE( info.backendName == backendName );
                ASSERT_TRUE( SameText( ContentAsString( response ), answer ) );

                // what the server saw: the form as a POST with the content type of a form and the body untouched
                CORE::CString received = tlsServer.GetReceived();
                ASSERT_TRUE( received.HasSubstr( "POST /openid/login HTTP/1.1", true ) >= 0 );
                ASSERT_TRUE( received.HasSubstr( "application/x-www-form-urlencoded", false ) >= 0 );
                ASSERT_TRUE( received.HasSubstr( "openid.mode=check_authentication&openid.ns=http%3A%2F%2Fspecs.openid.net%2Fauth%2F2.0&openid.sig=ab%2Bc%2FdE%3D", true ) >= 0 );
                tlsServer.Stop();
            }

            // The same certificate is not trusted when nothing says so: the system bundle does not know it. The check is on by default
            {
                CLocalTlsServer tlsServer;
                ASSERT_TRUE( tlsServer.Start( dir, UNTRUSTED_PORT, "good", answer ) );
                WEB::CHttpResponseData response;
                WEB::CHttpClientTransferInfo info;
                ASSERT_FALSE( PostCheckAuthentication( client, options, UNTRUSTED_PORT, response, info ) );
                ASSERT_FALSE( info.success );
                ASSERT_TRUE( WEB::HTTPERRORCLASS_TLS == info.errorClass );
                ASSERT_TRUE( 0 == response.content.GetDataSize() );
                tlsServer.Stop();
            }

            // A bundle without that certificate in it does not trust it either, and a bundle that cannot be read is a failure, not a pass
            {
                CLocalTlsServer tlsServer;
                ASSERT_TRUE( tlsServer.Start( dir, OTHERBUNDLE_PORT, "good", answer ) );
                WEB::CHttpClientRequestOptions other = options;
                other.caBundlePath = dir + "/wrongname.crt";
                WEB::CHttpResponseData response;
                WEB::CHttpClientTransferInfo info;
                ASSERT_FALSE( PostCheckAuthentication( client, other, OTHERBUNDLE_PORT, response, info ) );
                ASSERT_TRUE( WEB::HTTPERRORCLASS_TLS == info.errorClass );
                other.caBundlePath = dir + "/does-not-exist.crt";
                ASSERT_FALSE( PostCheckAuthentication( client, other, OTHERBUNDLE_PORT, response, info ) );
                ASSERT_TRUE( WEB::HTTPERRORCLASS_TLS == info.errorClass );
                tlsServer.Stop();
            }

            // The environment variable of OpenSSL names the bundle when the options do not
            {
                CORE::CString previous = CORE::GetEnv( "SSL_CERT_FILE" );
                CLocalTlsServer tlsServer;
                ASSERT_TRUE( tlsServer.Start( dir, ENVIRONMENT_PORT, "good", answer ) );
                CORE::SetEnv( "SSL_CERT_FILE", dir + "/good.crt" );
                WEB::CHttpResponseData response;
                WEB::CHttpClientTransferInfo info;
                bool sent = PostCheckAuthentication( client, options, ENVIRONMENT_PORT, response, info );
                CORE::SetEnv( "SSL_CERT_FILE", previous );
                ASSERT_TRUE( sent );
                ASSERT_TRUE( SameText( ContentAsString( response ), answer ) );
                tlsServer.Stop();
            }

            // A trusted certificate for another name is refused: the name of the host is checked as well
            {
                CLocalTlsServer tlsServer;
                ASSERT_TRUE( tlsServer.Start( dir, WRONGNAME_PORT, "wrongname", answer ) );
                WEB::CHttpClientRequestOptions trusting = options;
                trusting.caBundlePath = dir + "/wrongname.crt";
                WEB::CHttpResponseData response;
                WEB::CHttpClientTransferInfo info;
                ASSERT_FALSE( PostCheckAuthentication( client, trusting, WRONGNAME_PORT, response, info ) );
                ASSERT_TRUE( WEB::HTTPERRORCLASS_TLS == info.errorClass );
                tlsServer.Stop();
            }

            CORE::DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( ( "Test (" + backendName + "): failures are classified: connect and name lookup" ).C_String() )
        try
        {
            WEB::CHttpClientRequestOptions options = client.GetDefaultOptions();
            options.connectTimeoutInMs = 3000;
            options.totalTimeoutInMs = 8000;
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            WEB::CHttpRequestData request;

            // nothing listens on that port of the loopback interface
            request.SetUrl( "https://127.0.0.1:1/" );
            ASSERT_FALSE( client.Execute( request, options, response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_CONNECT == info.errorClass );

            // the reserved top level domain .invalid never resolves; a resolver without a network may take its time to say so
            request.SetUrl( "https://host.invalid/" );
            ASSERT_FALSE( client.Execute( request, options, response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_DNS == info.errorClass || WEB::HTTPERRORCLASS_TIMEOUT == info.errorClass );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/** a condition for the tests of the proxy environment: the variables are put back as they were */
class CEnvironmentScope
{
    public:

    CEnvironmentScope( void ) : m_names(), m_values() {}

    ~CEnvironmentScope()
    {
        for ( size_t i=0; i<m_names.size(); ++i )
            CORE::SetEnv( m_names[ i ], m_values[ i ] );
    }

    void Set( const CORE::CString& name, const CORE::CString& value )
    {
        m_names.push_back( name );
        m_values.push_back( CORE::GetEnv( name ) );
        CORE::SetEnv( name, value );
    }

    private:

    GUCEF::vector< CORE::CString > m_names;
    GUCEF::vector< CORE::CString > m_values;
};

/*-------------------------------------------------------------------------*/

static void
PerformProxyEnvironmentTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Test: The proxy of the environment is read the way other tools do, with NO_PROXY and without proxying this computer" )
        try
        {
            CEnvironmentScope scope;
            scope.Set( "HTTPS_PROXY", "" );
            scope.Set( "https_proxy", "" );
            scope.Set( "HTTP_PROXY", "" );
            scope.Set( "http_proxy", "" );
            scope.Set( "NO_PROXY", "" );
            scope.Set( "no_proxy", "" );
            scope.Set( "REQUEST_METHOD", "" );

            WEB::CHttpProxyEnvironment::TProxy proxy;
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );

            scope.Set( "HTTPS_PROXY", "http://user:pa%ss@proxy.example:3128" );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );
            ASSERT_TRUE( proxy.host == "proxy.example" && 3128 == proxy.port && proxy.userName == "user" && proxy.password == "pa%ss" );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::GetBasicAuthorization( proxy ) == "Basic dXNlcjpwYSVzcw==" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "http", "example.com", 80, proxy ) );     // another variable

            // the lower case variable is used when there is no upper case one
            scope.Set( "HTTPS_PROXY", "" );
            scope.Set( "https_proxy", "proxy.example" );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );
            ASSERT_TRUE( proxy.host == "proxy.example" && 1080 == proxy.port && proxy.userName.IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::GetBasicAuthorization( proxy ).IsNULLOrEmpty() );

            // this computer is never proxied
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "localhost", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "127.0.0.1", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "127.9.9.9", 443, proxy ) );

            // NO_PROXY: names and their subdomains, addresses, networks, ports and everything
            scope.Set( "NO_PROXY", "internal.example, .corp.example,10.0.0.0/8,192.168.1.5,special.example:8443" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "internal.example", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "www.internal.example", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "a.corp.example", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "corp.example", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "10.20.30.40", 443, proxy ) );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "https", "11.20.30.40", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "192.168.1.5", 443, proxy ) );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "https", "192.168.1.6", 443, proxy ) );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "special.example", 8443, proxy ) );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "https", "special.example", 443, proxy ) );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "https", "notinternal.example", 443, proxy ) );     // a name is not a suffix of text
            scope.Set( "NO_PROXY", "*" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );
            scope.Set( "NO_PROXY", "" );

            // proxies that cannot be used are not used
            scope.Set( "https_proxy", "socks5://proxy.example:1080" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );
            scope.Set( "https_proxy", "http://:80" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );
            scope.Set( "https_proxy", "http://proxy.example:notaport" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "https", "example.com", 443, proxy ) );

            // HTTP_PROXY in upper case is not trusted in a CGI program
            scope.Set( "HTTP_PROXY", "http://evil.example:80" );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "http", "example.com", 80, proxy ) && proxy.host == "evil.example" );
            scope.Set( "REQUEST_METHOD", "GET" );
            ASSERT_FALSE( WEB::CHttpProxyEnvironment::Lookup( "http", "example.com", 80, proxy ) );
            scope.Set( "http_proxy", "http://ok.example:80" );
            ASSERT_TRUE( WEB::CHttpProxyEnvironment::Lookup( "http", "example.com", 80, proxy ) && proxy.host == "ok.example" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

/** what a test of https needs: certificates, a TLS terminator in front of the scripted server, and the CA in the options */
struct SHttpsFixture
{
    TESTFW::CTlsTestCerts certs;
    TESTFW::CTlsTestTerminator terminator;
    bool ready;
    CORE::CString whyNot;

    SHttpsFixture( void ) : certs(), terminator(), ready( false ), whyNot()
    {
        CORE::CString noProvider;
        if ( !TESTFW::CTlsTestCerts::IsToolAvailable() )
            whyNot = "the openssl command line tool is not available";
        else
        if ( !certs.IsReady() )
            whyNot = "the test certificates could not be made";
        else
        if ( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( noProvider ) )
            whyNot = "this system has no TLS provider: " + noProvider;
        else
        if ( !terminator.Start( certs.GetPath( "named.crt" ), certs.GetPath( "named.key" ), SCRIPTED_SERVER_PORT ) )
            whyNot = "the TLS terminator did not start";
        else
            ready = true;
    }

    CORE::CString BaseUrl( void ) { return "https://localhost:" + CORE::ToString( terminator.GetPort() ); }

    /** the default options of the client with the test authority as the only one that is trusted */
    WEB::CHttpClientRequestOptions TrustingOptions( WEB::CHTTPClient& client )
    {
        WEB::CHttpClientRequestOptions options = client.GetDefaultOptions();
        options.caBundlePath = certs.GetPath( "ca.crt" );
        options.connectTimeoutInMs = 10000;
        options.totalTimeoutInMs = 30000;
        return options;
    }
};

#define GUCEF_HTTPS_FIXTURE_REQUIRED( fixture ) GUCEF_TESTFW_SKIP_IF( !( fixture ).ready, ( fixture ).whyNot.C_String() )

/*-------------------------------------------------------------------------*/

/** the native backend carries https through the TLS layer: the tests of the backends again, over a TLS terminator in front of the scripted server */
static void
PerformNativeHttpsTests( WEB::CHTTPClient& client, CScriptedHttpServer& server )
{GUCEF_TRACE;

    CORE::CString::StringVector preferred;
    preferred.push_back( "native" );
    client.SetPreferredBackends( preferred );

    SHttpsFixture fixture;
    WEB::CHttpClientRequestOptions savedDefaults = client.GetDefaultOptions();
    if ( fixture.ready )
    {
        client.SetDefaultOptions( fixture.TrustingOptions( client ) );
        g_scriptedBaseUrl = fixture.BaseUrl();
        server.SetSecureBaseUrl( fixture.BaseUrl() );
    }

    GUCEF_TESTFW_TESTCASE( "Test (native https): Execute reads responses split over reads, chunked and close delimited" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_TRUE( client.Execute( MakeRequest( "/hello" ), client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 200 == response.statusCode && SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( info.success && info.backendName == "native" && 0 == info.redirectCount );
            ASSERT_TRUE( info.effectiveUrl.StartsWith( "https://localhost:" ) );
            ASSERT_TRUE( info.bytesReceived > 11 && info.bytesSent > 0 );

            WEB::CHttpResponseData chunked;
            ASSERT_TRUE( client.Execute( MakeRequest( "/chunked" ), chunked ) );
            ASSERT_TRUE( 200 == chunked.statusCode && SameText( ToCString( chunked.content ), "The quick brown fox jumps over the lazy dog!!!!" ) );

            WEB::CHttpResponseData closed;
            ASSERT_TRUE( client.Execute( MakeRequest( "/close" ), closed ) );
            ASSERT_TRUE( SameText( ToCString( closed.content ), "until the end" ) );

            WEB::CHttpResponseData interim;
            ASSERT_TRUE( client.Execute( MakeRequest( "/continue" ), interim ) );
            ASSERT_TRUE( 200 == interim.statusCode && SameText( ToCString( interim.content ), "after continue" ) );

            WEB::CHttpResponseData head;
            ASSERT_TRUE( client.Execute( MakeRequest( "/head", "HEAD" ), head ) );
            ASSERT_TRUE( 200 == head.statusCode && 0 == head.content.GetDataSize() );

            WEB::CHttpResponseData notFound;
            ASSERT_TRUE( client.Execute( MakeRequest( "/status/404" ), notFound ) );
            ASSERT_TRUE( 404 == notFound.statusCode && SameText( ToCString( notFound.content ), "not here" ) );

            // the server saw the name of the host as the Host header and a request that is a plain HTTP/1.1 request
            ASSERT_TRUE( server.GetLastRequestHeader( "Host" ) == "localhost:" + CORE::ToString( fixture.terminator.GetPort() ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): Redirects between http and https are followed in both directions and the credentials do not follow" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            // https to http: the redirects of the scripted server end at its plain port
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            WEB::CHttpRequestData secureStart = MakeRequest( "/redirect/1" );
            ASSERT_TRUE( client.Execute( secureStart, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( 200 == response.statusCode && SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( 2 == info.redirectCount && info.effectiveUrl.StartsWith( "http://127.0.0.1:" ) );

            // http to https
            WEB::CHttpRequestData plainStart;
            plainStart.SetUrl( "http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/to-secure" );
            WEB::CHttpResponseData toSecure;
            WEB::CHttpClientTransferInfo secureInfo;
            ASSERT_TRUE( client.Execute( plainStart, client.GetDefaultOptions(), toSecure, &secureInfo ) );
            ASSERT_TRUE( 200 == toSecure.statusCode && SameText( ToCString( toSecure.content ), "hello world" ) );
            ASSERT_TRUE( 1 == secureInfo.redirectCount && secureInfo.effectiveUrl.StartsWith( "https://localhost:" ) );

            // a credential given for the https origin is not sent to the http one
            WEB::CHttpRequestData withCredentials = MakeRequest( "/to-same-origin" );
            withCredentials.otherHeaders[ "Authorization" ] = "Bearer SECRETTOKEN";
            WEB::CHttpResponseData stripped;
            ASSERT_TRUE( client.Execute( withCredentials, stripped ) );
            ASSERT_TRUE( 200 == stripped.statusCode );
            ASSERT_FALSE( server.LastRequestHadHeader( "Authorization" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): Ranges, large downloads to a sink and streamed uploads" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            WEB::CHttpClientRequestOptions range = client.GetDefaultOptions();
            range.hasRange = true;
            range.rangeStart = 10;
            range.rangeLength = 5;
            WEB::CHttpResponseData partial;
            ASSERT_TRUE( client.Execute( MakeRequest( "/range" ), range, partial ) );
            ASSERT_TRUE( 206 == partial.statusCode && SameText( ToCString( partial.content ), "abcde" ) );

            CORE::CDynamicBuffer downloaded;
            WEB::CHttpClientRequestOptions toSink = client.GetDefaultOptions();
            toSink.responseBodySink = CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( &downloaded, false ) );
            WEB::CHttpResponseData large;
            ASSERT_TRUE( client.Execute( MakeRequest( "/large" ), toSink, large ) );
            ASSERT_TRUE( 2 * 1024 * 1024 == downloaded.GetDataSize() );
            ASSERT_TRUE( 'a' == downloaded.AsConstTypePtr< char >()[ 0 ] && 'b' == downloaded.AsConstTypePtr< char >()[ 4096 ] && 'x' == downloaded.AsConstTypePtr< char >()[ 1 ] );

            WEB::CHttpClientRequestOptions upload = client.GetDefaultOptions();
            upload.requestBody = CORE::IOAccessPtr( GUCEF_NEW CStreamSource( "streamed request body" ) );
            WEB::CHttpResponseData echo;
            ASSERT_TRUE( client.Execute( MakeRequest( "/echo", "PUT" ), upload, echo ) );
            ASSERT_TRUE( SameText( ToCString( echo.content ), "PUT\nstreamed request body" ) );

            CORE::CDynamicBuffer source;
            source.CopyFrom( 10, "known size" );
            upload.requestBody = CORE::IOAccessPtr( GUCEF_NEW CORE::CDynamicBufferAccess( source ) );
            ASSERT_TRUE( client.Execute( MakeRequest( "/echo", "POST" ), upload, echo ) );
            ASSERT_TRUE( SameText( ToCString( echo.content ), "POST\nknown size" ) );

            // a form like the one of the check of an OpenID assertion, as the login of Steam does it
            static const CORE::CString form = "openid.assoc_handle=1234567890&openid.mode=check_authentication&openid.sig=ab%2Bc%2FdE%3D";
            WEB::CHttpRequestData post = MakeRequest( "/echo", "POST" );
            post.contentRepresentation = "application/x-www-form-urlencoded";
            post.content.CopyFrom( form.Length(), form.C_String() );
            WEB::CHttpResponseData posted;
            ASSERT_TRUE( client.Execute( post, posted ) );
            ASSERT_TRUE( SameText( ToCString( posted.content ), "POST\n" + form ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): Cookies and several requests at the same time on a thread pool" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            WEB::CHttpCookieJarPtr jar( GUCEF_NEW WEB::CHttpCookieJar() );
            client.SetCookieJar( jar );
            WEB::CHttpResponseData response;
            ASSERT_TRUE( client.Execute( MakeRequest( "/setcookie" ), response ) );
            ASSERT_TRUE( client.Execute( MakeRequest( "/showcookies" ), response ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "session=abc" ) );
            client.SetCookieJar( WEB::CHttpCookieJarPtr() );

            client.SetThreadPoolName( "HttpsClientTestPool" );
            GUCEF::vector< CORE::CFutureResult > futures;
            for ( int i=0; i<6; ++i )
                futures.push_back( client.ExecuteAsync( MakeRequest( i % 2 ? "/hello" : "/chunked" ) ) );
            for ( size_t i=0; i<futures.size(); ++i )
            {
                CORE::CTaskPtr task = futures[ i ].GetResult( 20000 );
                WEB::CHttpClientRequestTaskData* data = WEB::CHTTPClient::GetRequestTaskData( task );
                ASSERT_TRUE( GUCEF_NULL != data );
                if ( GUCEF_NULL != data )
                {
                    ASSERT_TRUE( data->transferInfo.success && 200 == data->response.statusCode );
                    ASSERT_TRUE( SameText( ToCString( data->response.content ), ( i % 2 ? "hello world" : "The quick brown fox jumps over the lazy dog!!!!" ) ) );
                }
            }
            client.SetThreadPool( CORE::ThreadPoolPtr() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): The classic API raises its events as before" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            CClassicRecorder recorder;
            recorder.SubscribeTo( &client );
            ASSERT_TRUE( client.Get( fixture.BaseUrl() + "/hello" ) );
            for ( int i=0; i<10000 && !recorder.finished; ++i )
            {
                CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
                MT::ThreadDelay( 1 );
            }
            ASSERT_TRUE( recorder.finished && recorder.disconnected );
            ASSERT_TRUE( SameText( recorder.body, "hello world" ) );
            ASSERT_TRUE( 200 == recorder.httpCode );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): Timeouts, refused connections, untrusted servers and a close without close notification are classified, and nothing secret is logged" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            TESTFW::CLogTextCapture capture;

            WEB::CHttpClientRequestOptions quick = client.GetDefaultOptions();
            quick.totalTimeoutInMs = 500;
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            ASSERT_FALSE( client.Execute( MakeRequest( "/slow" ), quick, response, &info ) );
            ASSERT_TRUE( info.timedOut && !info.success && info.durationInMs >= 500 && info.durationInMs < 5000 );

            // nothing listens
            WEB::CHttpRequestData nowhere;
            nowhere.SetUrl( "https://127.0.0.1:1/" );
            ASSERT_FALSE( client.Execute( nowhere, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_CONNECT == info.errorClass );

            // a name that does not exist
            WEB::CHttpRequestData unknown;
            unknown.SetUrl( "https://host.invalid/" );
            ASSERT_FALSE( client.Execute( unknown, client.GetDefaultOptions(), response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_DNS == info.errorClass || WEB::HTTPERRORCLASS_TIMEOUT == info.errorClass );

            // the certificate is not trusted when the options do not name its authority, the user info and the query of the URL stay out of every text
            WEB::CHttpClientRequestOptions untrusting = client.GetDefaultOptions();
            untrusting.caBundlePath.Clear();
            WEB::CHttpRequestData secret;
            secret.SetUrl( "https://localhost:" + CORE::ToString( fixture.terminator.GetPort() ) + "/hello?key=SECRETKEY" );
            ASSERT_FALSE( client.Execute( secret, untrusting, response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_TLS == info.errorClass );
            ASSERT_TRUE( info.errorDescription.HasSubstr( "not trusted" ) >= 0 );
            ASSERT_TRUE( info.errorDescription.HasSubstr( "SECRET" ) < 0 );
            WEB::CHttpRequestData withUserInfo;
            withUserInfo.SetUrl( "https://user:SECRETPASSWORD@localhost:" + CORE::ToString( fixture.terminator.GetPort() ) + "/hello?key=SECRETKEY" );
            ASSERT_FALSE( client.Execute( withUserInfo, untrusting, response, &info ) );
            ASSERT_TRUE( info.errorDescription.HasSubstr( "SECRET" ) < 0 );
            ASSERT_TRUE( capture.GetText().HasSubstr( "SECRET" ) < 0 );

            // verification off: the same server answers, with a warning
            untrusting.verifyPeer = false;
            ASSERT_TRUE( client.Execute( MakeRequest( "/hello" ), untrusting, response, &info ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( capture.Contains( "TLS verification is DISABLED for localhost:" + CORE::ToString( fixture.terminator.GetPort() ) ) );

            // a cut at the TCP level: a response with a length is complete and fine, one that ends with the connection is not
            fixture.terminator.SetAbortWithoutCloseNotify( true );
            WEB::CHttpResponseData sized;
            ASSERT_TRUE( client.Execute( MakeRequest( "/hello" ), sized ) );
            ASSERT_TRUE( SameText( ToCString( sized.content ), "hello world" ) );
            WEB::CHttpResponseData truncated;
            ASSERT_FALSE( client.Execute( MakeRequest( "/close" ), client.GetDefaultOptions(), truncated, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_CONNECT == info.errorClass );
            ASSERT_TRUE( info.errorDescription.HasSubstr( "close notification" ) >= 0 );
            fixture.terminator.SetAbortWithoutCloseNotify( false );
        }
        catch ( ... )
        {
            fixture.terminator.SetAbortWithoutCloseNotify( false );
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): The proxy of the options carries https as a tunnel, a proxy that refuses is an error and a plain request gets the absolute URL" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            TESTFW::CTestHttpProxy proxy;
            ASSERT_TRUE( proxy.Start( fixture.terminator.GetPort() ) );

            WEB::CHttpClientRequestOptions viaProxy = client.GetDefaultOptions();
            viaProxy.proxyHost = "127.0.0.1";
            viaProxy.proxyPort = proxy.GetPort();
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            WEB::CHttpRequestData request;
            request.SetUrl( "https://tls-target.test:443/hello" );
            ASSERT_TRUE( client.Execute( request, viaProxy, response, &info ) );
            ASSERT_TRUE( 200 == response.statusCode && SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( proxy.GetRequestLines().HasSubstr( "CONNECT tls-target.test:443 HTTP/1.1" ) >= 0 );
            ASSERT_TRUE( server.GetLastRequestHeader( "Host" ) == "tls-target.test:443" );
            ASSERT_TRUE( server.GetLastRequestHeaders().StartsWith( "GET /hello HTTP/1.1" ) );      // the origin form, not the absolute URL

            // a request that is not for the host of the certificate fails the check of the name, also through a tunnel
            WEB::CHttpRequestData otherName;
            otherName.SetUrl( "https://other.example:443/hello" );
            ASSERT_FALSE( client.Execute( otherName, viaProxy, response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_TLS == info.errorClass );

            // http through the same proxy: the whole URL is asked for, the proxy answers
            proxy.ClearRecording();
            WEB::CHttpRequestData plain;
            plain.SetUrl( "http://tls-target.test:80/hello" );
            ASSERT_TRUE( client.Execute( plain, viaProxy, response, &info ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "proxied" ) );
            ASSERT_TRUE( proxy.GetRequestLines().HasSubstr( "GET http://tls-target.test:80/hello HTTP/1.1" ) >= 0 );

            // a proxy that refuses
            TESTFW::CTestHttpProxy refusing;
            ASSERT_TRUE( refusing.Start( fixture.terminator.GetPort(), 403 ) );
            viaProxy.proxyPort = refusing.GetPort();
            ASSERT_FALSE( client.Execute( request, viaProxy, response, &info ) );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_CONNECT == info.errorClass );
            ASSERT_TRUE( info.errorDescription.HasSubstr( "403" ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test (native https): The proxy of the environment is used for hosts that are not this computer, NO_PROXY and the option switch it off" )
        GUCEF_HTTPS_FIXTURE_REQUIRED( fixture );
        try
        {
            TESTFW::CTestHttpProxy proxy;
            ASSERT_TRUE( proxy.Start( fixture.terminator.GetPort() ) );

            CEnvironmentScope scope;
            scope.Set( "NO_PROXY", "" );
            scope.Set( "no_proxy", "" );
            scope.Set( "HTTPS_PROXY", "http://proxyuser:proxypass@127.0.0.1:" + CORE::ToString( proxy.GetPort() ) );
            scope.Set( "https_proxy", "" );

            WEB::CHttpClientRequestOptions options = client.GetDefaultOptions();
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            WEB::CHttpRequestData request;
            request.SetUrl( "https://tls-target.test:443/hello" );
            ASSERT_TRUE( client.Execute( request, options, response, &info ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "hello world" ) );
            ASSERT_TRUE( proxy.GetRequestLines().HasSubstr( "CONNECT tls-target.test:443 HTTP/1.1" ) >= 0 );
            ASSERT_TRUE( proxy.GetLastProxyAuthorization() == "Basic " + CORE::Base64Encode( "proxyuser:proxypass" ) );

            // this computer does not go through it, so the proxy sees nothing of this request
            proxy.ClearRecording();
            ASSERT_TRUE( client.Execute( MakeRequest( "/hello" ), options, response, &info ) );
            ASSERT_TRUE( proxy.GetRequestLines().IsNULLOrEmpty() );

            // NO_PROXY names the host: the request goes there directly, where the name does not exist
            scope.Set( "NO_PROXY", "tls-target.test" );
            ASSERT_FALSE( client.Execute( request, options, response, &info ) );
            ASSERT_TRUE( proxy.GetRequestLines().IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_DNS == info.errorClass || WEB::HTTPERRORCLASS_TIMEOUT == info.errorClass );

            // and the option does the same
            scope.Set( "NO_PROXY", "" );
            options.useEnvironmentProxy = false;
            ASSERT_FALSE( client.Execute( request, options, response, &info ) );
            ASSERT_TRUE( proxy.GetRequestLines().IsNULLOrEmpty() );

            // http gets the variable of http
            scope.Set( "HTTP_PROXY", "http://127.0.0.1:" + CORE::ToString( proxy.GetPort() ) );
            scope.Set( "http_proxy", "" );
            scope.Set( "REQUEST_METHOD", "" );
            options.useEnvironmentProxy = true;
            WEB::CHttpRequestData plain;
            plain.SetUrl( "http://tls-target.test:80/hello" );
            ASSERT_TRUE( client.Execute( plain, options, response, &info ) );
            ASSERT_TRUE( SameText( ToCString( response.content ), "proxied" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    g_scriptedBaseUrl.Clear();
    client.SetDefaultOptions( savedDefaults );
}

/*-------------------------------------------------------------------------*/

void
PerformHttpClientTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING HTTP client TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "HttpClient" );

    PerformUrlAndCookieTests();

    CScriptedHttpServer server;
    bool serverStarted = server.Start( SCRIPTED_SERVER_PORT );
    CScriptedHttpServer otherOrigin;
    bool otherOriginStarted = otherOrigin.Start( OTHER_ORIGIN_PORT );

    WEB::CHTTPClient client;

    GUCEF_TESTFW_TESTCASE( "Test: Backends are selected by scheme" )
        try
        {
            ASSERT_TRUE( serverStarted );
            ASSERT_TRUE( otherOriginStarted );
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            // WinHTTP is part of the operating system and comes before the native backend in the default order
            ASSERT_TRUE( client.GetBackendNameForScheme( "http" ) == "winhttp" );
            ASSERT_TRUE( client.GetBackendNameForScheme( "https" ) == "winhttp" );
            #else
            ASSERT_TRUE( client.GetBackendNameForScheme( "http" ) == "native" );
            #endif

            // https needs a backend from a plugin, without one the request fails with an explanation
            if ( client.GetBackendNameForScheme( "https" ).IsNULLOrEmpty() )
            {
                WEB::CHttpRequestData request;
                request.SetUrl( "https://127.0.0.1:1/" );
                WEB::CHttpResponseData response;
                WEB::CHttpClientTransferInfo info;
                ASSERT_FALSE( client.Execute( request, client.GetDefaultOptions(), response, &info ) );
                ASSERT_TRUE( info.errorDescription.HasSubstr( "https", true ) >= 0 );
            }
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    GUCEF_TESTFW_TESTCASE( "Test: Without a TLS provider and without a plugin there is no https, which the client reports and classifies, and http is not affected" )
        try
        {
            // The system without OpenSSL: the providers are taken out of the registry for the duration of the test
            TESTFW::CNoTlsProviderScope noTls;

            ASSERT_TRUE( WEB::CHTTPClient::IsSchemeSupported( "http" ) );
            ASSERT_TRUE( WEB::CHTTPClient::IsSchemeSupported( "HTTP" ) );
            ASSERT_FALSE( WEB::CHTTPClient::IsSchemeSupported( "https" ) );
            ASSERT_FALSE( WEB::CHTTPClient::IsSchemeSupported( "gopher" ) );
            ASSERT_TRUE( SameText( WEB::CHTTPClient::DescribeAvailableBackends(), "native" ) );
            ASSERT_TRUE( client.GetBackendNameForScheme( "https" ).IsNULLOrEmpty() );
            ASSERT_TRUE( WEB::CHTTPClient::DescribeUnsupportedScheme( "https" ).HasSubstr( "no HTTPS support:", true ) >= 0 );
            ASSERT_TRUE( WEB::CHTTPClient::DescribeUnsupportedScheme( "gopher" ).HasSubstr( "gopher", true ) >= 0 );

            WEB::CHttpRequestData request;
            request.SetUrl( "https://127.0.0.1:1/" );
            WEB::CHttpResponseData response;
            WEB::CHttpClientTransferInfo info;
            bool executed = client.Execute( request, client.GetDefaultOptions(), response, &info );

            // the plain http of the same client works as ever
            WEB::CHttpRequestData plainRequest;
            plainRequest.SetUrl( "http://127.0.0.1:" + CORE::ToString( SCRIPTED_SERVER_PORT ) + "/hello" );
            WEB::CHttpResponseData plainResponse;
            WEB::CHttpClientTransferInfo plainInfo;
            bool plainExecuted = client.Execute( plainRequest, client.GetDefaultOptions(), plainResponse, &plainInfo );

            ASSERT_FALSE( executed );
            ASSERT_TRUE( WEB::HTTPERRORCLASS_NO_BACKEND == info.errorClass );
            ASSERT_TRUE( SameText( WEB::HttpTransferErrorClassToString( info.errorClass ), "no-backend" ) );
            ASSERT_TRUE( info.errorDescription.HasSubstr( "no HTTPS support:", true ) >= 0 );
            ASSERT_TRUE( plainExecuted );
            ASSERT_TRUE( 200 == plainResponse.statusCode );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: With a TLS provider the native backend carries https" )
        CORE::CString whyNot;
        GUCEF_TESTFW_SKIP_IF( !COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().IsAnyAvailable( whyNot ), "this system has no TLS provider" );
        try
        {
            ASSERT_TRUE( WEB::CHTTPClient::IsSchemeSupported( "https" ) );
            ASSERT_TRUE( WEB::CHTTPClient::IsSchemeSupported( "HTTPS" ) );
            ASSERT_TRUE( client.GetBackendNameForScheme( "https" ) == "native" );
            ASSERT_TRUE( SameText( WEB::CHTTPClient::DescribeAvailableBackends(), "native (https)" ) );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif /* GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN ? */

    PerformBackendTests( client, "native", server, otherOrigin );

    PerformProxyEnvironmentTests();
    PerformLocalHttpsTests( client, "native" );
    PerformNativeHttpsTests( client, server );

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    // WinHTTP is an operating system facility, no plugin needed. Windows build unverified
    if ( WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "winhttp" ) )
        PerformBackendTests( client, "winhttp", server, otherOrigin );
    #endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ? */

    PerformRedirectOriginTests( client );

    PerformUriAccessorTests();

    // The curl backend comes from a plugin, when it is available the same tests run with it
    client.SetPreferredBackends( WEB::CHttpClientBackendSelection::GetDefaultPreferredBackends() );
    bool curlPluginLoaded = LoadHttpClientPlugin( "httpclientpluginCURL" ) && WEB::CWebGlobal::Instance()->GetHttpClientBackendFactory().IsConstructible( "curl" );

    GUCEF_TESTFW_TESTCASE( "Test: The curl backend plugin provides https and is preferred by default" )
        try
        {
            ASSERT_TRUE( curlPluginLoaded );
            ASSERT_TRUE( client.GetBackendNameForScheme( "http" ) == "curl" );
            ASSERT_TRUE( client.GetBackendNameForScheme( "https" ) == "curl" );
            ASSERT_TRUE( WEB::CHTTPClient::IsSchemeSupported( "https" ) );
            ASSERT_TRUE( WEB::CHTTPClient::IsSchemeSupported( "HTTPS" ) );
            ASSERT_TRUE( WEB::CHTTPClient::DescribeAvailableBackends().HasSubstr( "curl (https)", true ) >= 0 );
            ASSERT_TRUE( WEB::CHTTPClient::DescribeAvailableBackends().HasSubstr( "native", true ) >= 0 );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    if ( curlPluginLoaded )
    {
        PerformBackendTests( client, "curl", server, otherOrigin );

        PerformLocalHttpsTests( client, "curl" );

        // The backend object of the plugin has to be gone before its code is unloaded
        CORE::CString::StringVector nativeOnly;
        nativeOnly.push_back( "native" );
        client.SetPreferredBackends( nativeOnly );
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "httpclientpluginCURL_tests" );
    }

    otherOrigin.Stop();
    server.Stop();

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL HTTP client TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
