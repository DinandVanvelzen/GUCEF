/*
 *  gucefCOM_TestApp: GUCEF test application for the COM module
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

#ifndef GUCEF_COM_TESTAPP_TESTMCPCLIENTSUPPORT_H
#define GUCEF_COM_TESTAPP_TESTMCPCLIENTSUPPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_CORE_CPROCESSPIPE_H
#include "gucefCORE_CProcessPipe.h"
#define GUCEF_CORE_CPROCESSPIPE_H
#endif /* GUCEF_CORE_CPROCESSPIPE_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "TestMcpSupport.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <winsock2.h>
  #include <windows.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/select.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  What the transport tests of the MCP server share: pipes, a raw TCP client that bypasses everything of GUCEF, a
 *  line buffer, and a client of a child process. Everything here waits with a timeout: a test that does not get its
 *  answer fails and goes on, it never waits for ever.
 */

/*-------------------------------------------------------------------------*/

/** Both ends of an anonymous pipe, as handles of the platform. False when the O/S would not make one */
inline bool
McpTestMakePipe( GUCEF::CORE::CProcessPipe::TNativeHandle& readEnd, GUCEF::CORE::CProcessPipe::TNativeHandle& writeEnd )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    HANDLE read = GUCEF_NULL;
    HANDLE write = GUCEF_NULL;
    if ( !::CreatePipe( &read, &write, GUCEF_NULL, 0 ) )
        return false;
    readEnd = read;
    writeEnd = write;
    return true;
    #else
    int ends[ 2 ] = { -1, -1 };
    if ( 0 != ::pipe( ends ) )
        return false;
    readEnd = ends[ 0 ];
    writeEnd = ends[ 1 ];
    return true;
    #endif
}

/*-------------------------------------------------------------------------*/

/** Text that arrives in pieces, out as lines. A line that has its end is popped without its '\n' */
class CMcpTestLineBuffer
{
    public:

    CMcpTestLineBuffer( void ) : m_text(), m_unparsable( 0 ), m_lines( 0 ) {}

    void Append( const char* data, GUCEF::UInt32 size )
    {
        m_text.Append( data, size );
    }

    bool PopLine( GUCEF::CORE::CString& line )
    {
        const char* text = m_text.C_String();
        const GUCEF::UInt32 textBytes = m_text.ByteSize() > 0 ? m_text.ByteSize() - 1 : 0;      // bytes: Length() counts characters
        const void* end = ::memchr( text, '\n', textBytes );
        if ( GUCEF_NULL == end )
            return false;
        const GUCEF::UInt32 lineLength = (GUCEF::UInt32) ( static_cast< const char* >( end ) - text );
        line = GUCEF::CORE::CString( text, lineLength );
        m_text = GUCEF::CORE::CString( text + lineLength + 1, textBytes - lineLength - 1 );
        ++m_lines;
        GUCEF::CORE::CDataNode node;
        if ( !McpTestParse( line, node ) )
            ++m_unparsable;
        return true;
    }

    /** Lines that were popped and are not JSON */
    GUCEF::UInt32 GetUnparsableCount( void ) const { return m_unparsable; }

    GUCEF::UInt32 GetLineCount( void ) const { return m_lines; }

    GUCEF::UInt32 GetPendingBytes( void ) const { return m_text.ByteSize() > 0 ? m_text.ByteSize() - 1 : 0; }

    private:

    GUCEF::CORE::CString m_text;
    GUCEF::UInt32 m_unparsable;
    GUCEF::UInt32 m_lines;
};

/*-------------------------------------------------------------------------*/

/**
 *  A TCP client made of nothing but the sockets of the O/S, so that what it sends is exactly what the test says:
 *  half lines, HTTP requests, garbage
 */
class CMcpRawTcpClient
{
    public:

    CMcpRawTcpClient( void ) : m_socket( InvalidSocket() ), m_closedByPeer( false ), m_lines() {}

    ~CMcpRawTcpClient() { Close(); }

    bool Connect( GUCEF::UInt16 port )
    {
        Close();
        m_closedByPeer = false;
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        WSADATA wsaData;
        ::WSAStartup( MAKEWORD( 2, 2 ), &wsaData );
        #endif
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        SOCKET created = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( INVALID_SOCKET == created )
            return false;
        m_socket = (GUCEF::Int64) created;
        #else
        int created = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( created < 0 )
            return false;
        m_socket = (GUCEF::Int64) created;
        #endif
        struct sockaddr_in address;
        ::memset( &address, 0, sizeof( address ) );
        address.sin_family = AF_INET;
        address.sin_port = htons( port );
        address.sin_addr.s_addr = htonl( 0x7F000001UL );
        if ( 0 != ::connect( Native(), (struct sockaddr*) &address, sizeof( address ) ) )
        {
            Close();
            return false;
        }
        return true;
    }

    bool IsConnected( void ) const { return InvalidSocket() != m_socket; }

    /** All of the bytes, or false */
    bool Send( const GUCEF::CORE::CString& bytes )
    {
        if ( !IsConnected() )
            return false;
        GUCEF::UInt32 sent = 0;
        const GUCEF::UInt32 total = bytes.ByteSize() > 0 ? bytes.ByteSize() - 1 : 0;      // bytes: Length() counts characters
        while ( sent < total )
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            int result = ::send( Native(), bytes.C_String() + sent, (int) ( total - sent ), 0 );
            #elif defined( MSG_NOSIGNAL )
            ssize_t result = ::send( Native(), bytes.C_String() + sent, total - sent, MSG_NOSIGNAL );
            #else
            ssize_t result = ::send( Native(), bytes.C_String() + sent, total - sent, 0 );
            #endif
            if ( result <= 0 )
                return false;
            sent += (GUCEF::UInt32) result;
        }
        return true;
    }

    bool SendLine( const GUCEF::CORE::CString& line ) { return Send( line + "\n" ); }

    /** Reads what is there, waiting at most the timeout for the first bytes. @return the number of bytes, 0 when nothing came or the peer closed (see IsClosedByPeer()) */
    GUCEF::UInt32 Receive( GUCEF::UInt32 timeoutInMs )
    {
        if ( !IsConnected() || m_closedByPeer )
            return 0;
        fd_set readSet;
        FD_ZERO( &readSet );
        FD_SET( Native(), &readSet );
        struct timeval timeout;
        timeout.tv_sec = (long) ( timeoutInMs / 1000 );
        timeout.tv_usec = (long) ( ( timeoutInMs % 1000 ) * 1000 );
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        int ready = ::select( 0, &readSet, GUCEF_NULL, GUCEF_NULL, &timeout );
        #else
        int ready = ::select( Native() + 1, &readSet, GUCEF_NULL, GUCEF_NULL, &timeout );
        #endif
        if ( ready <= 0 )
            return 0;
        char buffer[ 8192 ];
        #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
        int received = ::recv( Native(), buffer, sizeof( buffer ), 0 );
        #else
        ssize_t received = ::recv( Native(), buffer, sizeof( buffer ), 0 );
        #endif
        if ( received <= 0 )
        {
            m_closedByPeer = true;
            return 0;
        }
        m_lines.Append( buffer, (GUCEF::UInt32) received );
        return (GUCEF::UInt32) received;
    }

    bool IsClosedByPeer( void ) const { return m_closedByPeer; }

    CMcpTestLineBuffer& Lines( void ) { return m_lines; }

    void Close( void )
    {
        if ( InvalidSocket() != m_socket )
        {
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            ::closesocket( Native() );
            #else
            ::close( Native() );
            #endif
            m_socket = InvalidSocket();
        }
    }

    private:

    static GUCEF::Int64 InvalidSocket( void ) { return -1; }

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    SOCKET Native( void ) const { return (SOCKET) m_socket; }
    #else
    int Native( void ) const { return (int) m_socket; }
    #endif

    GUCEF::Int64 m_socket;
    bool m_closedByPeer;
    CMcpTestLineBuffer m_lines;
};

/*-------------------------------------------------------------------------*/

/**
 *  Starts a child process with its three standard streams on pipes and talks to it line by line. The stderr of the
 *  child is drained all the time, so that a child that logs a lot never blocks on a full pipe. The child is
 *  ended when the client goes away.
 */
class CMcpSubprocessClient
{
    public:

    CMcpSubprocessClient( void ) : m_process(), m_stdout(), m_stderrText(), m_stdoutEnded( false ) {}

    ~CMcpSubprocessClient()
    {
        // never leaves a child behind
        m_process.CloseStdinPipe();
        m_process.Terminate( false );
    }

    bool Launch( const GUCEF::CORE::CString& exe, const GUCEF::CORE::CProcess::TStringVector& args, const GUCEF::CORE::CProcess::TStringMap& environment )
    {
        return m_process.LaunchWithPipes( exe, args, true, true, true, GUCEF::CORE::CString(), environment );
    }

    bool SendLine( const GUCEF::CORE::CString& line )
    {
        GUCEF::CORE::CProcessPipe* input = m_process.GetStdinPipe();
        if ( GUCEF_NULL == input || !input->Opened() )
            return false;
        GUCEF::CORE::CString text = line + "\n";
        return 1 == input->Write( text.C_String(), text.ByteSize() - 1, 1 );
    }

    void CloseInput( void ) { m_process.CloseStdinPipe(); }

    /** Takes in what the child wrote, waiting at most the timeout for something on standard output */
    void Pump( GUCEF::UInt32 timeoutInMs )
    {
        char buffer[ 8192 ];
        GUCEF::CORE::CProcessPipe* error = m_process.GetStderrPipe();
        while ( GUCEF_NULL != error && error->WaitForData( 0 ) )
        {
            GUCEF::UInt32 got = error->ReadSome( buffer, sizeof( buffer ) );
            if ( 0 == got )
                break;
            if ( m_stderrText.Length() < 4 * 1024 * 1024 )
                m_stderrText.Append( buffer, got );
        }
        GUCEF::CORE::CProcessPipe* output = m_process.GetStdoutPipe();
        if ( GUCEF_NULL != output && !m_stdoutEnded && output->WaitForData( timeoutInMs ) )
        {
            GUCEF::UInt32 got = output->ReadSome( buffer, sizeof( buffer ) );
            if ( 0 == got )
                m_stdoutEnded = true;
            else
                m_stdout.Append( buffer, got );
        }
        else
        if ( GUCEF_NULL != output && !m_stdoutEnded )
        {
            // nothing came; a child that is gone closes the pipe
        }
    }

    /** The next line of standard output, parsed too. False at the timeout or when the output ended */
    bool ReadLine( GUCEF::CORE::CString& line, GUCEF::UInt32 timeoutInMs )
    {
        GUCEF::UInt32 waited = 0;
        while ( !m_stdout.PopLine( line ) )
        {
            if ( m_stdoutEnded || waited >= timeoutInMs )
                return false;
            Pump( 20 );
            waited += 20;
        }
        return true;
    }

    /** The response with the id, the lines before it (notifications, other responses) are kept in 'others' */
    bool ReadResponse( const GUCEF::CORE::CString& id, GUCEF::CORE::CDataNode& response, GUCEF::vector< GUCEF::CORE::CDataNode >& others, GUCEF::UInt32 timeoutInMs )
    {
        GUCEF::UInt32 waited = 0;
        while ( waited < timeoutInMs )
        {
            GUCEF::CORE::CString line;
            if ( !ReadLine( line, 50 ) )
            {
                waited += 50;
                if ( m_stdoutEnded )
                    return false;
                continue;
            }
            GUCEF::CORE::CDataNode node;
            if ( !McpTestParse( line, node ) )
                continue;
            if ( GUCEF_NULL == node.GetAttribute( "method" ) && GUCEF_NULL != node.GetAttribute( "id" ) && node.GetAttribute( "id" )->second.AsString() == id )
            {
                response = node;
                return true;
            }
            others.push_back( node );
        }
        return false;
    }

    /** Pumps for the given time and returns the response with the id if it came, without waiting for it otherwise */
    bool TryReadResponse( const GUCEF::CORE::CString& id, GUCEF::CORE::CDataNode& response, GUCEF::UInt32 durationInMs )
    {
        GUCEF::vector< GUCEF::CORE::CDataNode > others;
        return ReadResponse( id, response, others, durationInMs );
    }

    bool WaitForExit( GUCEF::UInt32 timeoutInMs, GUCEF::Int32& exitCode )
    {
        GUCEF::UInt32 waited = 0;
        while ( waited < timeoutInMs )
        {
            if ( m_process.WaitForExit( 20, exitCode ) )
                return true;
            Pump( 0 );
            waited += 20;
        }
        return false;
    }

    GUCEF::CORE::CProcess& Process( void ) { return m_process; }

    const GUCEF::CORE::CString& GetStderrText( void ) const { return m_stderrText; }

    CMcpTestLineBuffer& Stdout( void ) { return m_stdout; }

    bool HasStdoutEnded( void ) const { return m_stdoutEnded; }

    private:

    GUCEF::CORE::CProcess m_process;
    CMcpTestLineBuffer m_stdout;
    GUCEF::CORE::CString m_stderrText;
    bool m_stdoutEnded;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_TESTAPP_TESTMCPCLIENTSUPPORT_H ? */
