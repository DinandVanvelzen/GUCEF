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

#ifndef GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H
#define GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H

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

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #define GUCEF_WEB_TEST_RAW_POSIX_SOCKETS
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  What the HTTP server tests of this test app share: a TCP client that sends exactly the bytes a test gives it, in
 *  the writes the test asks for, and collects what comes back, plus the parsing of the responses and SSE events in
 *  what came back. The server and the client run on the application's pulse generator, which the test thread pumps.
 */

/** Drives the application pulse generator for the given time */
inline void
TestHttpPumpFor( GUCEF::UInt32 durationInMs )
{
    GUCEF::CORE::PulseGeneratorPtr pulseGenerator = GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    GUCEF::UInt64 startTicks = GUCEF::MT::PrecisionTickCount();
    GUCEF::Float64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000.0;
    while ( ( GUCEF::MT::PrecisionTickCount() - startTicks ) / ticksPerMs < durationInMs )
    {
        pulseGenerator->ForceDirectPulse();
        GUCEF::MT::ThreadDelay( 1 );
    }
}

/*-------------------------------------------------------------------------*/

/** One response as it came in */
struct STestHttpResponse
{
    GUCEF::Int32 status;
    GUCEF::CORE::CString statusLine;
    GUCEF::CORE::CString head;                  /**< the header block, lines ending in \r\n */
    GUCEF::CORE::CString body;
    bool complete;                              /**< the whole body is in: by its length, or by the close of the connection */

    STestHttpResponse( void ) : status( 0 ), statusLine(), head(), body(), complete( false ) {}

    /** The value of a header, matched without regard to case; empty when it is not there */
    GUCEF::CORE::CString Header( const GUCEF::CORE::CString& name ) const
    {
        GUCEF::CORE::CString::StringVector lines = head.ParseElements( '\n', false );
        for ( size_t i=0; i<lines.size(); ++i )
        {
            GUCEF::CORE::CString line = lines[ i ];
            if ( line.Length() > 0 && '\r' == line[ line.Length()-1 ] )
                line = line.CutChars( 1, false );
            GUCEF::Int32 colon = line.HasChar( ':' );
            if ( colon <= 0 )
                continue;
            if ( line.SubstrToIndex( (GUCEF::UInt32) colon, true ).Trim( true ).Trim( false ).Equals( name, false ) )
                return line.CutChars( (GUCEF::UInt32) colon + 1, true ).Trim( true ).Trim( false );
        }
        return GUCEF::CORE::CString::Empty;
    }

    bool HasHeader( const GUCEF::CORE::CString& name ) const
    {
        GUCEF::CORE::CString lowerHead = "\n" + head.Lowercase();
        return lowerHead.HasSubstr( "\n" + name.Lowercase() + ":" ) >= 0;
    }
};

typedef GUCEF::vector< STestHttpResponse > TTestHttpResponseVector;

/*-------------------------------------------------------------------------*/

/**
 *  Cuts the bytes a client received into responses: a header block, then a body of Content-Length bytes, or for
 *  a response without a length (an SSE stream) all that follows. Interim 1xx responses count as responses.
 */
inline GUCEF::UInt32
TestHttpParseResponses( const GUCEF::CORE::CString& received, bool connectionClosed, TTestHttpResponseVector& responses )
{
    responses.clear();
    const char* text = received.C_String();
    const GUCEF::UInt32 size = received.ByteSize() > 0 ? received.ByteSize() - 1 : 0;
    GUCEF::UInt32 offset = 0;
    while ( offset < size )
    {
        const char* headEnd = GUCEF_NULL;
        for ( GUCEF::UInt32 i=offset; i+3<size; ++i )
        {
            if ( '\r' == text[ i ] && '\n' == text[ i+1 ] && '\r' == text[ i+2 ] && '\n' == text[ i+3 ] )
            {
                headEnd = text + i;
                break;
            }
        }
        if ( GUCEF_NULL == headEnd )
            break;
        STestHttpResponse response;
        GUCEF::CORE::CString block( text + offset, (GUCEF::UInt32) ( headEnd - ( text + offset ) ) );
        GUCEF::Int32 lineEnd = block.HasSubstr( "\r\n" );
        response.statusLine = lineEnd >= 0 ? block.SubstrToIndex( (GUCEF::UInt32) lineEnd, true ) : block;
        response.head = lineEnd >= 0 ? block.CutChars( (GUCEF::UInt32) lineEnd + 2, true ) + "\r\n" : GUCEF::CORE::CString::Empty;
        GUCEF::CORE::CString::StringVector parts = response.statusLine.ParseElements( ' ', false );
        response.status = parts.size() >= 2 ? GUCEF::CORE::StringToInt32( parts[ 1 ] ) : 0;
        offset = (GUCEF::UInt32) ( headEnd - text ) + 4;

        if ( response.status >= 100 && response.status < 200 && 101 != response.status )
        {
            response.complete = true;
            responses.push_back( response );
            continue;
        }
        if ( response.HasHeader( "Content-Length" ) )
        {
            GUCEF::UInt32 length = GUCEF::CORE::StringToUInt32( response.Header( "Content-Length" ) );
            if ( offset + length > size )
            {
                response.body = GUCEF::CORE::CString( text + offset, size - offset );
                responses.push_back( response );
                break;
            }
            response.body = GUCEF::CORE::CString( text + offset, length );
            response.complete = true;
            offset += length;
            responses.push_back( response );
            continue;
        }
        // a body delimited by the close of the connection
        response.body = GUCEF::CORE::CString( text + offset, size - offset );
        response.complete = connectionClosed;
        responses.push_back( response );
        break;
    }
    return (GUCEF::UInt32) responses.size();
}

/*-------------------------------------------------------------------------*/

/** The data of the SSE events in a stream body, in order; comment lines are counted apart */
inline GUCEF::UInt32
TestHttpParseSseEvents( const GUCEF::CORE::CString& body, GUCEF::CORE::CString::StringVector& events, GUCEF::UInt32* comments = GUCEF_NULL )
{
    events.clear();
    if ( GUCEF_NULL != comments )
        *comments = 0;
    GUCEF::CORE::CString data;
    bool hasData = false;
    // only complete lines count: what follows the last line end is still coming in
    const char* text = body.C_String();
    const GUCEF::UInt32 size = body.ByteSize() > 0 ? body.ByteSize() - 1 : 0;
    GUCEF::UInt32 lineStart = 0;
    for ( GUCEF::UInt32 i=0; i<size; ++i )
    {
        if ( '\n' != text[ i ] )
            continue;
        GUCEF::CORE::CString line( text + lineStart, i - lineStart );
        lineStart = i + 1;
        if ( line.Length() > 0 && '\r' == line[ line.Length()-1 ] )
            line = line.CutChars( 1, false );
        if ( line.IsNULLOrEmpty() )
        {
            if ( hasData )
                events.push_back( data );
            data.Clear();
            hasData = false;
            continue;
        }
        if ( ':' == line[ 0 ] )
        {
            if ( GUCEF_NULL != comments )
                ++(*comments);
            continue;
        }
        if ( 0 == line.HasSubstr( "data:", 0, true ) )
        {
            GUCEF::CORE::CString value = line.CutChars( 5, true );
            if ( value.Length() > 0 && ' ' == value[ 0 ] )
                value = value.CutChars( 1, true );
            if ( hasData )
                data += "\n";
            data += value;
            hasData = true;
        }
    }
    return (GUCEF::UInt32) events.size();
}

/*-------------------------------------------------------------------------*/

/**
 *  A TCP client of gucefCOMCORE on the application's pulse generator that sends the exact bytes of a test, in as many
 *  writes as the test makes, and keeps what comes back.
 */
class CTestHttpRawClient : public GUCEF::CORE::CObserver
{
    public:

    CTestHttpRawClient( void )
        : GUCEF::CORE::CObserver()
        , m_connected( false )
        , m_closed( false )
        , m_received()
        , m_client( GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false )
    {
        SubscribeTo( &m_client );
    }

    virtual ~CTestHttpRawClient()
    {
        UnsubscribeAllFromObserver();
        m_client.Close();
    }

    virtual const GUCEF::CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const GUCEF::CORE::CString classTypeName = "CTestHttpRawClient";
        return classTypeName;
    }

    bool Connect( GUCEF::UInt16 port, GUCEF::UInt32 timeoutInMs = 5000 )
    {
        m_connected = false;
        m_closed = false;
        m_received.Clear();
        if ( !m_client.ConnectTo( "127.0.0.1", port, false ) )
            return false;
        return PumpUntilConnected( timeoutInMs );
    }

    /** One write of the bytes, then some pulses so that the server reads them before the next write */
    bool Send( const GUCEF::CORE::CString& bytes, GUCEF::UInt32 settleInMs = 20 )
    {
        return SendBytes( bytes.C_String(), bytes.ByteSize() > 0 ? bytes.ByteSize() - 1 : 0, settleInMs );
    }

    bool SendBytes( const void* data, GUCEF::UInt32 size, GUCEF::UInt32 settleInMs = 20 )
    {
        bool sent = m_client.Send( data, size );
        if ( settleInMs > 0 )
            TestHttpPumpFor( settleInMs );
        return sent;
    }

    void Close( void )
    {
        m_client.Close();
    }

    /** Pulses until the number of complete responses is reached */
    bool WaitForResponses( GUCEF::UInt32 count, GUCEF::UInt32 timeoutInMs = 5000 )
    {
        GUCEF::UInt64 startTicks = GUCEF::MT::PrecisionTickCount();
        GUCEF::Float64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000.0;
        while ( GetCompleteResponseCount() < count )
        {
            if ( ( GUCEF::MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
                return false;
            GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            GUCEF::MT::ThreadDelay( 1 );
        }
        return true;
    }

    /** Pulses until the received text holds the given text */
    bool WaitForText( const GUCEF::CORE::CString& text, GUCEF::UInt32 timeoutInMs = 5000 )
    {
        GUCEF::UInt64 startTicks = GUCEF::MT::PrecisionTickCount();
        GUCEF::Float64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000.0;
        while ( m_received.HasSubstr( text ) < 0 )
        {
            if ( ( GUCEF::MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
                return false;
            GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            GUCEF::MT::ThreadDelay( 1 );
        }
        return true;
    }

    /** Pulses until the server closed the connection */
    bool WaitForClose( GUCEF::UInt32 timeoutInMs = 5000 )
    {
        GUCEF::UInt64 startTicks = GUCEF::MT::PrecisionTickCount();
        GUCEF::Float64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000.0;
        while ( !m_closed )
        {
            if ( ( GUCEF::MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
                return false;
            GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            GUCEF::MT::ThreadDelay( 1 );
        }
        return true;
    }

    GUCEF::UInt32 GetResponses( TTestHttpResponseVector& responses ) const
    {
        return TestHttpParseResponses( m_received, m_closed, responses );
    }

    GUCEF::UInt32 GetCompleteResponseCount( void ) const
    {
        TTestHttpResponseVector responses;
        TestHttpParseResponses( m_received, m_closed, responses );
        GUCEF::UInt32 complete = 0;
        for ( size_t i=0; i<responses.size(); ++i )
            if ( responses[ i ].complete )
                ++complete;
        return complete;
    }

    /** The number of responses that came in, complete or not: responses follow each other by their length */
    GUCEF::UInt32 CountStatusLines( void ) const
    {
        TTestHttpResponseVector responses;
        return TestHttpParseResponses( m_received, m_closed, responses );
    }

    bool IsConnected( void ) const { return m_connected; }
    bool IsClosed( void ) const { return m_closed; }
    const GUCEF::CORE::CString& GetReceived( void ) const { return m_received; }
    void ClearReceived( void ) { m_received.Clear(); }

    protected:

    virtual void OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
                           const GUCEF::CORE::CEvent& eventid               ,
                           GUCEF::CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF::COMCORE::CTCPClientSocket::ConnectedEvent == eventid )
        {
            m_connected = true;
        }
        else
        if ( GUCEF::COMCORE::CTCPClientSocket::DataRecievedEvent == eventid )
        {
            GUCEF::COMCORE::CTCPClientSocket::TDataRecievedEventData* data = static_cast< GUCEF::COMCORE::CTCPClientSocket::TDataRecievedEventData* >( eventdata );
            m_received.Append( data->GetData().AsConstTypePtr< char >(), data->GetData().GetDataSize() );
        }
        else
        if ( GUCEF::COMCORE::CTCPClientSocket::DisconnectedEvent == eventid || GUCEF::COMCORE::CTCPClientSocket::SocketErrorEvent == eventid )
        {
            m_closed = true;
        }
    }

    private:

    bool PumpUntilConnected( GUCEF::UInt32 timeoutInMs )
    {
        GUCEF::UInt64 startTicks = GUCEF::MT::PrecisionTickCount();
        GUCEF::Float64 ticksPerMs = GUCEF::MT::PrecisionTimerResolution() / 1000.0;
        while ( !m_connected && !m_closed )
        {
            if ( ( GUCEF::MT::PrecisionTickCount() - startTicks ) / ticksPerMs > timeoutInMs )
                return false;
            GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator()->ForceDirectPulse();
            GUCEF::MT::ThreadDelay( 1 );
        }
        // the server accepts on its next pulses
        TestHttpPumpFor( 10 );
        return m_connected;
    }

    bool m_connected;
    bool m_closed;
    GUCEF::CORE::CString m_received;
    GUCEF::COMCORE::CTCPClientSocket m_client;
};

/*-------------------------------------------------------------------------*/

#ifdef GUCEF_WEB_TEST_RAW_POSIX_SOCKETS

/**
 *  A plain POSIX socket for what the client of gucefCOMCORE cannot do: end a connection with a reset (SO_LINGER 0),
 *  the way a client that crashes or loses its network ends it.
 */
class CTestHttpResettingClient
{
    public:

    CTestHttpResettingClient( void ) : m_socket( -1 ) {}

    ~CTestHttpResettingClient() { Reset(); }

    bool Connect( GUCEF::UInt16 port )
    {
        m_socket = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( m_socket < 0 )
            return false;
        struct sockaddr_in address;
        ::memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_port = htons( port );
        address.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        if ( 0 != ::connect( m_socket, reinterpret_cast< struct sockaddr* >( &address ), sizeof( address ) ) )
            return false;
        ::fcntl( m_socket, F_SETFL, ::fcntl( m_socket, F_GETFL, 0 ) | O_NONBLOCK );
        TestHttpPumpFor( 10 );
        return true;
    }

    bool Send( const GUCEF::CORE::CString& bytes, GUCEF::UInt32 settleInMs = 20 )
    {
        const GUCEF::UInt32 size = bytes.ByteSize() > 0 ? bytes.ByteSize() - 1 : 0;
        bool sent = (ssize_t) size == ::send( m_socket, bytes.C_String(), size, MSG_NOSIGNAL );
        TestHttpPumpFor( settleInMs );
        return sent;
    }

    /** What arrived so far, without waiting */
    GUCEF::CORE::CString Read( void )
    {
        GUCEF::CORE::CString text;
        char buffer[ 4096 ];
        for ( ;; )
        {
            ssize_t got = ::recv( m_socket, buffer, sizeof( buffer ), 0 );
            if ( got <= 0 )
                break;
            text.Append( buffer, (GUCEF::UInt32) got );
        }
        return text;
    }

    /** Ends the connection with a reset instead of the orderly close */
    void Reset( void )
    {
        if ( m_socket < 0 )
            return;
        struct linger lingerOption;
        lingerOption.l_onoff = 1;
        lingerOption.l_linger = 0;
        ::setsockopt( m_socket, SOL_SOCKET, SO_LINGER, &lingerOption, sizeof( lingerOption ) );
        ::close( m_socket );
        m_socket = -1;
    }

    private:

    int m_socket;
};

#endif /* GUCEF_WEB_TEST_RAW_POSIX_SOCKETS ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_TESTAPP_TESTHTTPRAWCLIENT_H ? */
