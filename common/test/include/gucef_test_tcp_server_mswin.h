/*
 *  gucef common header: Windows implementation of the scripted TCP server for tests
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

/*
 *  The counterpart of the BSD sockets and pthreads implementation in gucef_test_tcp_server.h, with the same classes and the
 *  same behavior, for Windows: Winsock sockets, WSAPoll and Win32 threads. It is included by gucef_test_tcp_server.h and
 *  not to be included directly.
 *
 *  TLS is not part of it: the TLS server of the tests needs the OpenSSL of the system which is loaded by name on Linux.
 *  Without it IsTlsAvailable() is false and Start() refuses a server that was given a certificate, as on a Linux system
 *  without OpenSSL, and a test skips what needs TLS.
 */

#ifndef GUCEF_TEST_TCP_SERVER_MSWIN_H
#define GUCEF_TEST_TCP_SERVER_MSWIN_H

#if defined( _WIN32 ) && defined( __cplusplus )

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>

#if defined( _MSC_VER )
#pragma comment( lib, "ws2_32.lib" )
#endif

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#include "gucef_test_tls_terminator.h"
#define GUCEF_TEST_TLS_TERMINATOR_H
#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/** Starts Winsock for the process: the calls are counted by Windows so every user may call it */
inline bool
TestSocketsStartup( void )
{
    WSADATA data;
    return 0 == ::WSAStartup( MAKEWORD( 2, 2 ), &data );
}

/*-------------------------------------------------------------------------*/

class CTestTcpServer;

/** One accepted connection, as the conversation of a test sees it */
class CTestServerConnection
{
    public:

    CTestServerConnection( CTestTcpServer* server, SOCKET socketHandle, void* tlsContext, const STlsTerminatorApi* api )
        : m_server( server )
        , m_socket( socketHandle )
        , m_tlsContext( tlsContext )
        , m_api( api )
        , m_buffer()
        , m_start( 0 )
        , m_closed( false )
    {
    }

    ~CTestServerConnection()
    {
        Close();
    }

    /** Whether the other side is gone, or this side closed */
    bool IsClosed( void ) const { return m_closed; }

    bool IsTls( void ) const { return false; }

    /** Bytes that are received and not read yet: nonzero when the peer sent more than the conversation asked for */
    UInt32 GetUnreadSize( void ) const { return (UInt32)( m_buffer.size() - m_start ); }

    /** Reads up to the line feed, the CR in front of it is dropped. @return false on a timeout (the bytes stay), a close, or a line longer than 'maxLength' */
    bool ReadLine( CORE::CString& line, UInt32 timeoutInMs, UInt32 maxLength = 4 * 1024 * 1024 )
    {
        size_t scanned = m_start;
        UInt32 waited = 0;
        for ( ;; )
        {
            for ( ; scanned < m_buffer.size(); ++scanned )
            {
                if ( '\n' == m_buffer[ scanned ] )
                {
                    size_t end = scanned;
                    if ( end > m_start && '\r' == m_buffer[ end - 1 ] )
                        --end;
                    line = end > m_start ? CORE::CString( &m_buffer[ m_start ], (UInt32)( end - m_start ) ) : CORE::CString();
                    m_start = scanned + 1;
                    if ( m_start == m_buffer.size() )
                    {
                        m_buffer.clear();
                        m_start = 0;
                    }
                    return true;
                }
            }
            if ( m_buffer.size() - m_start > maxLength )
                return false;
            if ( waited >= timeoutInMs )
                return false;
            const UInt32 slice = timeoutInMs - waited < 200 ? timeoutInMs - waited : 200;
            if ( !Fill( slice ) )
            {
                if ( m_closed || IsServerStopping() )
                    return false;
                waited += slice;
            }
        }
    }

    /** Reads exactly 'count' bytes */
    bool ReadBytes( UInt32 count, CORE::CDynamicBuffer& bytes, UInt32 timeoutInMs )
    {
        UInt32 waited = 0;
        while ( m_buffer.size() - m_start < count )
        {
            if ( waited >= timeoutInMs )
                return false;
            const UInt32 slice = timeoutInMs - waited < 200 ? timeoutInMs - waited : 200;
            if ( !Fill( slice ) )
            {
                if ( m_closed || IsServerStopping() )
                    return false;
                waited += slice;
            }
        }
        bytes.Clear();
        if ( count > 0 )
            bytes.CopyFrom( count, &m_buffer[ m_start ] );
        m_start += count;
        if ( m_start == m_buffer.size() )
        {
            m_buffer.clear();
            m_start = 0;
        }
        return true;
    }

    /** Reads what an SMTP DATA command sends: up to the line with a single dot, unstuffed, the terminator not included. Lines end with CR LF as received */
    bool ReadDotTerminated( CORE::CDynamicBuffer& data, UInt32 timeoutInMs, UInt32 maxSize = 128 * 1024 * 1024 )
    {
        data.Clear();
        data.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
        CORE::CString line;
        for ( ;; )
        {
            if ( !ReadLine( line, timeoutInMs ) )
                return false;
            if ( 2 == line.ByteSize() && '.' == line.C_String()[ 0 ] )
                return true;
            const char* p = line.C_String();
            UInt32 size = line.ByteSize() > 0 ? line.ByteSize() - 1 : 0;
            if ( size > 1 && '.' == p[ 0 ] && '.' == p[ 1 ] )
            {
                ++p;
                --size;
            }
            if ( data.GetDataSize() + size + 2 > maxSize )
                return false;
            data.Append( p, size, true );
            data.Append( "\r\n", 2, true );
        }
    }

    bool Write( const void* data, UInt32 size )
    {
        if ( m_closed )
            return false;
        const char* bytes = static_cast< const char* >( data );
        UInt32 written = 0;
        UInt32 attempts = 0;
        while ( written < size )
        {
            if ( ++attempts > 1000000 || IsServerStopping() )
            {
                m_closed = true;
                return false;
            }
            const int result = ::send( m_socket, bytes + written, (int)( size - written ), 0 );
            if ( result <= 0 )
            {
                if ( result < 0 && WSAEWOULDBLOCK == ::WSAGetLastError() )
                    continue;
                m_closed = true;
                return false;
            }
            written += (UInt32) result;
        }
        return true;
    }

    /** The text and CR LF */
    bool WriteLine( const CORE::CString& line )
    {
        return WriteText( line + "\r\n" );
    }

    bool WriteText( const CORE::CString& text )
    {
        return Write( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
    }

    /** Not available on Windows, see the header comment: the server has no TLS */
    bool StartTls( void )
    {
        return false;
    }

    void Close( void )
    {
        if ( INVALID_SOCKET != m_socket )
        {
            ::closesocket( m_socket );
            m_socket = INVALID_SOCKET;
        }
        m_closed = true;
    }

    /** Whether a stop of the server is requested: loops of the conversation look at it */
    bool IsServerStopping( void ) const;

    /** Sleeps, in slices, until the time is up or the server stops */
    void Sleep( UInt32 timeInMs )
    {
        for ( UInt32 waited=0; waited<timeInMs && !IsServerStopping(); waited += 10 )
            MT::ThreadDelay( 10 );
    }

    private:

    CTestServerConnection( const CTestServerConnection& src );
    CTestServerConnection& operator=( const CTestServerConnection& src );

    bool Fill( UInt32 timeoutInMs )
    {
        if ( m_closed )
            return false;

        UInt32 waited = 0;
        UInt32 rounds = 0;
        for ( ;; )
        {
            // every wait has a deadline: the time asked for, and a count of rounds for the cases in which the wait does not advance
            if ( IsServerStopping() || ++rounds > 100000 )
                return false;
            WSAPOLLFD pfd;
            pfd.fd = m_socket;
            pfd.events = POLLRDNORM;
            pfd.revents = 0;
            const UInt32 slice = timeoutInMs - waited < 50 ? timeoutInMs - waited : 50;
            const int polled = ::WSAPoll( &pfd, 1, (int) slice );
            if ( SOCKET_ERROR == polled )
            {
                m_closed = true;
                return false;
            }
            if ( polled <= 0 || 0 == ( pfd.revents & ( POLLRDNORM | POLLHUP | POLLERR ) ) )
            {
                waited += slice;
                if ( waited >= timeoutInMs )
                    return false;
                continue;
            }

            char chunk[ 16384 ];
            const int got = ::recv( m_socket, chunk, (int) sizeof( chunk ), 0 );
            if ( got <= 0 )
            {
                m_closed = true;
                return false;
            }
            m_buffer.insert( m_buffer.end(), chunk, chunk + got );
            return true;
        }
    }

    CTestTcpServer* m_server;
    SOCKET m_socket;
    void* m_tlsContext;
    const STlsTerminatorApi* m_api;
    GUCEF::vector< char > m_buffer;
    size_t m_start;
    bool m_closed;
};

/*-------------------------------------------------------------------------*/

/** A server on a local port with a conversation per connection */
class CTestTcpServer
{
    public:

    CTestTcpServer( void )
        : m_listenSocket( INVALID_SOCKET )
        , m_port( 0 )
        , m_running( 0 )
        , m_acceptThread( NULL )
        , m_implicitTls( false )
        , m_api()
        , m_tlsContext( NULL )
        , m_liveConnections( 0 )
        , m_acceptedConnections( 0 )
        , m_finishedConnections( 0 )
        , m_lock()
        , m_changeCount( 0 )
    {
        ::InitializeCriticalSection( &m_changeSection );
        ::InitializeConditionVariable( &m_changeCondition );
    }

    virtual ~CTestTcpServer()
    {
        // the conversation of a derived class uses its members: Stop() has to be called by the destructor of the derived class too
        Stop();
        ::DeleteCriticalSection( &m_changeSection );
    }

    /** Give the certificate and key (PEM files) before Start() when the server does TLS */
    void SetTlsFiles( const CORE::CString& certificateFile, const CORE::CString& keyFile )
    {
        m_certificateFile = certificateFile;
        m_keyFile = keyFile;
    }

    /** TLS from the first byte of every connection, before Serve() is called. Needs the files */
    void SetImplicitTls( bool implicitTls ) { m_implicitTls = implicitTls; }

    /** Whether the system has what TLS needs and the files were given: never on Windows */
    bool IsTlsAvailable( void ) const { return NULL != m_tlsContext; }

    /** Listens on a free local port and starts accepting. @return false when that is not possible, or TLS was asked for and is not available */
    bool Start( void )
    {
        Stop();

        // TLS is not available here, see the header comment
        if ( !m_certificateFile.IsNULLOrEmpty() || m_implicitTls )
            return false;

        if ( !TestSocketsStartup() )
            return false;

        m_listenSocket = ::socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
        if ( INVALID_SOCKET == m_listenSocket )
            return false;
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        addr.sin_port = 0;
        if ( 0 != ::bind( m_listenSocket, (struct sockaddr*) &addr, sizeof( addr ) ) || 0 != ::listen( m_listenSocket, 16 ) )
        {
            ::closesocket( m_listenSocket );
            m_listenSocket = INVALID_SOCKET;
            return false;
        }
        int length = (int) sizeof( addr );
        ::getsockname( m_listenSocket, (struct sockaddr*) &addr, &length );
        m_port = ntohs( addr.sin_port );

        SetRunning( true );
        m_acceptThread = ::CreateThread( NULL, 0, &CTestTcpServer::AcceptThreadMain, this, 0, NULL );
        if ( NULL == m_acceptThread )
        {
            SetRunning( false );
            return false;
        }
        return true;
    }

    void Stop( void )
    {
        SetRunning( false );
        if ( NULL != m_acceptThread )
        {
            ::WaitForSingleObject( m_acceptThread, INFINITE );
            ::CloseHandle( m_acceptThread );
            m_acceptThread = NULL;
        }
        // the conversations end at their next look at the flag
        for ( int i=0; i<600 && GetLiveConnections() > 0; ++i )
            MT::ThreadDelay( 10 );
        if ( INVALID_SOCKET != m_listenSocket )
        {
            ::closesocket( m_listenSocket );
            m_listenSocket = INVALID_SOCKET;
        }
        m_port = 0;
    }

    UInt16 GetPort( void ) const { return m_port; }

    bool IsStopping( void ) const { return !IsRunning(); }

    UInt32 GetAcceptedConnections( void ) { MT::CScopeMutex lock( m_lock ); return m_acceptedConnections; }

    UInt32 GetLiveConnections( void ) { MT::CScopeMutex lock( m_lock ); return m_liveConnections; }

    /** The number of conversations that are over: Serve() returned and the connection is closed */
    UInt32 GetFinishedConnections( void ) { MT::CScopeMutex lock( m_lock ); return m_finishedConnections; }

    /**
     *  The conversation runs on a thread of its own, so what it did in reaction to the last bytes of the client (the QUIT of a
     *  client that closes right after it, ...) is not there yet when the client call returns. A test that looks at it waits for
     *  it with this: it blocks, without polling and without a fixed sleep, until the predicate holds or the time is up.
     *  The predicate is a functor with 'bool operator()( void ) const' that reads the state of the server the way its getters
     *  do; whatever changes that state calls SignalChanged() after the change. @return the predicate, evaluated one last time at the timeout
     */
    template< class TPredicate >
    bool WaitFor( const TPredicate& predicate, UInt32 timeoutInMs )
    {
        const ULONGLONG deadline = ::GetTickCount64() + timeoutInMs;
        for ( ;; )
        {
            // the count is read before the predicate: a change in between makes the wait return at once
            const UInt32 seen = GetChangeCount();
            if ( predicate() )
                return true;
            const ULONGLONG now = ::GetTickCount64();
            if ( now >= deadline || !WaitForChange( seen, (DWORD)( deadline - now ) ) )
                return predicate();
        }
    }

    /** Waits until 'count' conversations are over, see WaitFor() */
    bool WaitForFinishedConnections( UInt32 count, UInt32 timeoutInMs )
    {
        return WaitFor( CFinishedAtLeast( this, count ), timeoutInMs );
    }

    protected:

    /** Wakes the threads in WaitFor(): call it after the state that a predicate looks at has changed (and the lock for it is released) */
    void SignalChanged( void )
    {
        ::EnterCriticalSection( &m_changeSection );
        ++m_changeCount;
        ::WakeAllConditionVariable( &m_changeCondition );
        ::LeaveCriticalSection( &m_changeSection );
    }

    /** The conversation, on the thread of the connection. Returns when it is over; the connection is closed after it */
    virtual void Serve( CTestServerConnection& connection ) = 0;

    private:

    CTestTcpServer( const CTestTcpServer& src );
    CTestTcpServer& operator=( const CTestTcpServer& src );

    struct CFinishedAtLeast
    {
        CFinishedAtLeast( CTestTcpServer* server, UInt32 count ) : m_server( server ), m_count( count ) {}
        bool operator()( void ) const { return m_server->GetFinishedConnections() >= m_count; }
        CTestTcpServer* m_server;
        UInt32 m_count;
    };

    UInt32 GetChangeCount( void )
    {
        ::EnterCriticalSection( &m_changeSection );
        const UInt32 count = m_changeCount;
        ::LeaveCriticalSection( &m_changeSection );
        return count;
    }

    /** @return false when the time passed without a change after 'seen' */
    bool WaitForChange( UInt32 seen, DWORD timeoutInMs )
    {
        const ULONGLONG deadline = ::GetTickCount64() + timeoutInMs;
        bool changed = true;
        ::EnterCriticalSection( &m_changeSection );
        while ( m_changeCount == seen )
        {
            const ULONGLONG now = ::GetTickCount64();
            if ( now >= deadline || !::SleepConditionVariableCS( &m_changeCondition, &m_changeSection, (DWORD)( deadline - now ) ) )
            {
                changed = m_changeCount != seen;
                break;
            }
        }
        ::LeaveCriticalSection( &m_changeSection );
        return changed;
    }

    struct SConnectionStart
    {
        CTestTcpServer* owner;
        SOCKET socketHandle;
    };

    static DWORD WINAPI AcceptThreadMain( LPVOID arg )
    {
        static_cast< CTestTcpServer* >( arg )->AcceptLoop();
        return 0;
    }

    void AcceptLoop( void )
    {
        while ( IsRunning() )
        {
            WSAPOLLFD pfd;
            pfd.fd = m_listenSocket;
            pfd.events = POLLRDNORM;
            pfd.revents = 0;
            if ( ::WSAPoll( &pfd, 1, 50 ) <= 0 || 0 == ( pfd.revents & POLLRDNORM ) )
                continue;
            SOCKET clientSocket = ::accept( m_listenSocket, NULL, NULL );
            if ( INVALID_SOCKET == clientSocket )
                continue;

            SConnectionStart* start = new SConnectionStart;
            start->owner = this;
            start->socketHandle = clientSocket;
            {
                MT::CScopeMutex lock( m_lock );
                ++m_liveConnections;
                ++m_acceptedConnections;
            }
            HANDLE thread = ::CreateThread( NULL, 0, &CTestTcpServer::ConnectionThreadMain, start, 0, NULL );
            if ( NULL == thread )
            {
                ::closesocket( clientSocket );
                delete start;
                MT::CScopeMutex lock( m_lock );
                --m_liveConnections;
            }
            else
            {
                // the thread cleans up after itself
                ::CloseHandle( thread );
            }
        }
    }

    static DWORD WINAPI ConnectionThreadMain( LPVOID arg )
    {
        SConnectionStart* start = static_cast< SConnectionStart* >( arg );
        CTestTcpServer* owner = start->owner;
        SOCKET socketHandle = start->socketHandle;
        delete start;

        BOOL noDelay = TRUE;
        ::setsockopt( socketHandle, IPPROTO_TCP, TCP_NODELAY, (const char*) &noDelay, sizeof( noDelay ) );
        {
            CTestServerConnection connection( owner, socketHandle, owner->m_tlsContext, &owner->m_api );
            owner->Serve( connection );
        }
        {
            MT::CScopeMutex lock( owner->m_lock );
            ++owner->m_finishedConnections;
        }
        owner->SignalChanged();

        // the last use of the server: Stop() returns, and the server may be destroyed, when no connection is live
        MT::CScopeMutex lock( owner->m_lock );
        --owner->m_liveConnections;
        return 0;
    }

    /** The flag is read by the threads of the conversations while the owner clears it: an atomic access */
    bool IsRunning( void ) const
    {
        return 0 != ::InterlockedCompareExchange( const_cast< volatile LONG* >( &m_running ), 0, 0 );
    }

    void SetRunning( bool running )
    {
        ::InterlockedExchange( &m_running, running ? 1 : 0 );
    }

    SOCKET m_listenSocket;
    UInt16 m_port;
    volatile LONG m_running;
    HANDLE m_acceptThread;
    bool m_implicitTls;
    CORE::CString m_certificateFile;
    CORE::CString m_keyFile;
    STlsTerminatorApi m_api;
    void* m_tlsContext;
    UInt32 m_liveConnections;
    UInt32 m_acceptedConnections;
    UInt32 m_finishedConnections;
    MT::CMutex m_lock;
    UInt32 m_changeCount;
    CRITICAL_SECTION m_changeSection;
    CONDITION_VARIABLE m_changeCondition;
};

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::IsServerStopping( void ) const
{
    return m_server->IsStopping();
}

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* _WIN32 and C++ ? */

#endif /* GUCEF_TEST_TCP_SERVER_MSWIN_H ? */
