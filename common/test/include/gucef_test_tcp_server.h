/*
 *  gucef common header: header-only scripted TCP server for tests, with TLS
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
 *  Test support: a TCP server on a local port that runs a conversation on a thread of its own per connection, the way a
 *  test of a client of a line protocol (mail, FTP, ...) wants it: read a line, write a line, upgrade to TLS in the middle of
 *  the conversation, wait, drop the connection. It is the counterpart of the pulse driven client under test, which runs on the
 *  thread of the test, so everything here is blocking code with timeouts that look at the stop flag of the server.
 *
 *  A test derives from CTestTcpServer and implements Serve(). TLS (implicit from the first byte, or STARTTLS style with
 *  CTestServerConnection::StartTls()) uses the OpenSSL of the system, loaded when it is needed, with the certificate and key
 *  of gucef_test_tls_server.h (CTlsTestCerts); without the library the server cannot do TLS and a test skips what needs it.
 *
 *    class CMyServer : public GUCEF::TESTFW::CTestTcpServer
 *    {
 *        virtual void Serve( GUCEF::TESTFW::CTestServerConnection& connection )
 *        {
 *            connection.WriteLine( "220 hello" );
 *            GUCEF::CORE::CString line;
 *            while ( connection.ReadLine( line, 5000 ) ) { ... }
 *        }
 *    };
 *
 *  Linux (BSD sockets, pthreads, poll) only; elsewhere Start() is false.
 */

#ifndef GUCEF_TEST_TCP_SERVER_H
#define GUCEF_TEST_TCP_SERVER_H

#ifdef __cplusplus

#if defined( __linux__ )

  #include <string.h>
  #include <time.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <errno.h>
  #include <poll.h>
  #include <pthread.h>
  #include <signal.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>

#endif /* Linux ? */

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#include "gucef_test_tls_terminator.h"
#define GUCEF_TEST_TLS_TERMINATOR_H
#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

class CTestTcpServer;

/** One accepted connection, as the conversation of a test sees it */
class CTestServerConnection
{
    public:

    CTestServerConnection( CTestTcpServer* server, int fd, void* tlsContext, const STlsTerminatorApi* api )
        : m_server( server )
        , m_fd( fd )
        , m_ssl( NULL )
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

    bool IsTls( void ) const { return NULL != m_ssl; }

    /** Bytes that are received and not read yet: nonzero when the peer sent more than the conversation asked for */
    UInt32 GetUnreadSize( void ) const { return (UInt32)( m_buffer.size() - m_start ); }

    /** Reads up to the line feed, the CR in front of it is dropped. @return false on a timeout (the bytes stay), a close, or a line longer than 'maxLength' */
    bool ReadLine( CORE::CString& line, UInt32 timeoutInMs, UInt32 maxLength = 4 * 1024 * 1024 );

    /** Reads exactly 'count' bytes */
    bool ReadBytes( UInt32 count, CORE::CDynamicBuffer& bytes, UInt32 timeoutInMs );

    /** Reads what an SMTP DATA command sends: up to the line with a single dot, unstuffed, the terminator not included. Lines end with CR LF as received */
    bool ReadDotTerminated( CORE::CDynamicBuffer& data, UInt32 timeoutInMs, UInt32 maxSize = 128 * 1024 * 1024 );

    bool Write( const void* data, UInt32 size );

    /** The text and CR LF */
    bool WriteLine( const CORE::CString& line );

    bool WriteText( const CORE::CString& text );

    /**
     *  Speaks TLS from now on, as the server: the handshake with the certificate of the server. For the answer of STARTTLS (write the
     *  220 first). @return false when TLS is not available or the handshake failed
     */
    bool StartTls( void );

    void Close( void );

    /** Whether a stop of the server is requested: loops of the conversation look at it */
    bool IsServerStopping( void ) const;

    /** Sleeps, in slices, until the time is up or the server stops */
    void Sleep( UInt32 timeInMs );

    private:

    CTestServerConnection( const CTestServerConnection& src );
    CTestServerConnection& operator=( const CTestServerConnection& src );

    bool Fill( UInt32 timeoutInMs );

    private:

    CTestTcpServer* m_server;
    int m_fd;
    void* m_ssl;
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
        : m_listenFd( -1 )
        , m_port( 0 )
        , m_running( false )
        , m_threadStarted( false )
        , m_implicitTls( false )
        , m_api()
        , m_tlsContext( NULL )
        , m_liveConnections( 0 )
        , m_acceptedConnections( 0 )
        , m_finishedConnections( 0 )
        , m_lock()
        , m_changeCount( 0 )
    {
        #if defined( __linux__ )
        pthread_mutex_init( &m_changeMutex, NULL );
        pthread_condattr_t attributes;
        pthread_condattr_init( &attributes );
        pthread_condattr_setclock( &attributes, CLOCK_MONOTONIC );
        pthread_cond_init( &m_changeCondition, &attributes );
        pthread_condattr_destroy( &attributes );
        #endif
    }

    virtual ~CTestTcpServer()
    {
        // the conversation of a derived class uses its members: Stop() has to be called by the destructor of the derived class too
        Stop();
        #if defined( __linux__ )
        pthread_cond_destroy( &m_changeCondition );
        pthread_mutex_destroy( &m_changeMutex );
        #endif
    }

    /** Give the certificate and key (PEM files) before Start() when the server does TLS */
    void SetTlsFiles( const CORE::CString& certificateFile, const CORE::CString& keyFile )
    {
        m_certificateFile = certificateFile;
        m_keyFile = keyFile;
    }

    /** TLS from the first byte of every connection, before Serve() is called. Needs the files */
    void SetImplicitTls( bool implicitTls ) { m_implicitTls = implicitTls; }

    /** Whether the system has what TLS needs and the files were given */
    bool IsTlsAvailable( void ) const { return NULL != m_tlsContext; }

    /** Listens on a free local port and starts accepting. @return false when that is not possible, or TLS was asked for and is not available */
    bool Start( void )
    {
        #if defined( __linux__ )
        Stop();

        if ( !m_certificateFile.IsNULLOrEmpty() )
        {
            if ( !LoadTlsServerApi( m_api ) )
                return false;
            m_tlsContext = m_api.SSL_CTX_new( m_api.TLS_server_method() );
            if ( NULL == m_tlsContext )
                return false;
            if ( 1 != m_api.SSL_CTX_use_certificate_chain_file( m_tlsContext, m_certificateFile.C_String() ) ||
                 1 != m_api.SSL_CTX_use_PrivateKey_file( m_tlsContext, m_keyFile.C_String(), 1 ) )
            {
                m_api.SSL_CTX_free( m_tlsContext );
                m_tlsContext = NULL;
                return false;
            }
        }
        else
        if ( m_implicitTls )
            return false;

        m_listenFd = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( m_listenFd < 0 )
            return false;
        int one = 1;
        ::setsockopt( m_listenFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ) );
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        addr.sin_port = 0;
        if ( 0 != ::bind( m_listenFd, (struct sockaddr*) &addr, sizeof( addr ) ) || 0 != ::listen( m_listenFd, 16 ) )
        {
            ::close( m_listenFd );
            m_listenFd = -1;
            return false;
        }
        socklen_t length = sizeof( addr );
        ::getsockname( m_listenFd, (struct sockaddr*) &addr, &length );
        m_port = ntohs( addr.sin_port );

        SetRunning( true );
        if ( 0 != pthread_create( &m_acceptThread, NULL, &CTestTcpServer::AcceptThreadMain, this ) )
        {
            SetRunning( false );
            return false;
        }
        m_threadStarted = true;
        return true;
        #else
        return false;
        #endif
    }

    void Stop( void )
    {
        #if defined( __linux__ )
        SetRunning( false );
        if ( m_threadStarted )
        {
            pthread_join( m_acceptThread, NULL );
            m_threadStarted = false;
        }
        // the conversations end at their next look at the flag
        for ( int i=0; i<600 && GetLiveConnections() > 0; ++i )
            MT::ThreadDelay( 10 );
        if ( m_listenFd >= 0 )
        {
            ::close( m_listenFd );
            m_listenFd = -1;
        }
        if ( NULL != m_tlsContext )
        {
            m_api.SSL_CTX_free( m_tlsContext );
            m_tlsContext = NULL;
        }
        #endif
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
        #if defined( __linux__ )
        struct timespec deadline;
        ::clock_gettime( CLOCK_MONOTONIC, &deadline );
        deadline.tv_sec += timeoutInMs / 1000;
        deadline.tv_nsec += (long) ( timeoutInMs % 1000 ) * 1000000L;
        if ( deadline.tv_nsec >= 1000000000L )
        {
            deadline.tv_sec += 1;
            deadline.tv_nsec -= 1000000000L;
        }
        for ( ;; )
        {
            // the count is read before the predicate: a change in between makes the wait return at once
            const UInt32 seen = GetChangeCount();
            if ( predicate() )
                return true;
            if ( !WaitForChange( seen, deadline ) )
                return predicate();
        }
        #else
        return predicate();
        #endif
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
        #if defined( __linux__ )
        pthread_mutex_lock( &m_changeMutex );
        ++m_changeCount;
        pthread_cond_broadcast( &m_changeCondition );
        pthread_mutex_unlock( &m_changeMutex );
        #endif
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

    #if defined( __linux__ )

    UInt32 GetChangeCount( void )
    {
        pthread_mutex_lock( &m_changeMutex );
        const UInt32 count = m_changeCount;
        pthread_mutex_unlock( &m_changeMutex );
        return count;
    }

    /** @return false when the deadline passed without a change after 'seen' */
    bool WaitForChange( UInt32 seen, const struct timespec& deadline )
    {
        bool changed = true;
        pthread_mutex_lock( &m_changeMutex );
        while ( m_changeCount == seen )
        {
            if ( ETIMEDOUT == pthread_cond_timedwait( &m_changeCondition, &m_changeMutex, &deadline ) )
            {
                changed = m_changeCount != seen;
                break;
            }
        }
        pthread_mutex_unlock( &m_changeMutex );
        return changed;
    }

    struct SConnectionStart
    {
        CTestTcpServer* owner;
        int fd;
    };

    static void* AcceptThreadMain( void* arg )
    {
        static_cast< CTestTcpServer* >( arg )->AcceptLoop();
        return NULL;
    }

    void AcceptLoop( void )
    {
        while ( IsRunning() )
        {
            struct pollfd pfd;
            pfd.fd = m_listenFd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            if ( ::poll( &pfd, 1, 50 ) <= 0 || 0 == ( pfd.revents & POLLIN ) )
                continue;
            int clientFd = ::accept( m_listenFd, NULL, NULL );
            if ( clientFd < 0 )
                continue;

            SConnectionStart* start = new SConnectionStart;
            start->owner = this;
            start->fd = clientFd;
            {
                MT::CScopeMutex lock( m_lock );
                ++m_liveConnections;
                ++m_acceptedConnections;
            }
            pthread_t thread;
            pthread_attr_t attributes;
            pthread_attr_init( &attributes );
            pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );
            if ( 0 != pthread_create( &thread, &attributes, &CTestTcpServer::ConnectionThreadMain, start ) )
            {
                ::close( clientFd );
                delete start;
                MT::CScopeMutex lock( m_lock );
                --m_liveConnections;
            }
            pthread_attr_destroy( &attributes );
        }
    }

    static void* ConnectionThreadMain( void* arg )
    {
        SConnectionStart* start = static_cast< SConnectionStart* >( arg );
        CTestTcpServer* owner = start->owner;
        int fd = start->fd;
        delete start;

        // a write to a client that is gone must not take the process down
        sigset_t pipeSignal;
        sigemptyset( &pipeSignal );
        sigaddset( &pipeSignal, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &pipeSignal, NULL );

        int one = 1;
        ::setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof( one ) );
        {
            CTestServerConnection connection( owner, fd, owner->m_tlsContext, &owner->m_api );
            bool proceed = true;
            if ( owner->m_implicitTls )
                proceed = connection.StartTls();
            if ( proceed )
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
        return NULL;
    }

    pthread_t m_acceptThread;

    #endif /* Linux ? */

    /** The flag is read by the threads of the conversations while the owner clears it: an atomic access */
    bool IsRunning( void ) const
    {
        #if defined( __linux__ )
        return __atomic_load_n( &m_running, __ATOMIC_ACQUIRE );
        #else
        return m_running;
        #endif
    }

    void SetRunning( bool running )
    {
        #if defined( __linux__ )
        __atomic_store_n( &m_running, running, __ATOMIC_RELEASE );
        #else
        m_running = running;
        #endif
    }

    int m_listenFd;
    UInt16 m_port;
    volatile bool m_running;
    bool m_threadStarted;
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
    #if defined( __linux__ )
    pthread_mutex_t m_changeMutex;
    pthread_cond_t m_changeCondition;
    #endif
};

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::IsServerStopping( void ) const
{
    return m_server->IsStopping();
}

/*-------------------------------------------------------------------------*/

inline void
CTestServerConnection::Sleep( UInt32 timeInMs )
{
    for ( UInt32 waited=0; waited<timeInMs && !IsServerStopping(); waited += 10 )
        MT::ThreadDelay( 10 );
}

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::Fill( UInt32 timeoutInMs )
{
    #if defined( __linux__ )
    if ( m_closed )
        return false;
    // what OpenSSL has decrypted already is not visible to poll
    if ( NULL != m_ssl && m_api->SSL_pending( m_ssl ) > 0 )
        timeoutInMs = 0;

    UInt32 waited = 0;
    UInt32 rounds = 0;
    for ( ;; )
    {
        // every wait has a deadline: the time asked for, and a count of rounds for the cases in which the wait does not advance
        if ( IsServerStopping() || ++rounds > 100000 )
            return false;
        struct pollfd pfd;
        pfd.fd = m_fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        const UInt32 slice = timeoutInMs - waited < 50 ? timeoutInMs - waited : 50;
        const bool pending = NULL != m_ssl && m_api->SSL_pending( m_ssl ) > 0;
        if ( !pending )
        {
            int polled = ::poll( &pfd, 1, (int) slice );
            if ( polled < 0 && EINTR != errno )
            {
                m_closed = true;
                return false;
            }
            if ( polled <= 0 || 0 == ( pfd.revents & ( POLLIN | POLLHUP | POLLERR ) ) )
            {
                waited += slice;
                if ( waited >= timeoutInMs )
                    return false;
                continue;
            }
        }

        char chunk[ 16384 ];
        int got = 0;
        if ( NULL != m_ssl )
        {
            m_api->ERR_clear_error();
            got = m_api->SSL_read( m_ssl, chunk, (int) sizeof( chunk ) );
            if ( got <= 0 )
            {
                int error = m_api->SSL_get_error( m_ssl, got );
                if ( 2 == error || 3 == error )
                    continue;
                m_closed = true;
                return false;
            }
        }
        else
        {
            got = (int) ::recv( m_fd, chunk, sizeof( chunk ), 0 );
            if ( got <= 0 )
            {
                m_closed = true;
                return false;
            }
        }
        m_buffer.insert( m_buffer.end(), chunk, chunk + got );
        return true;
    }
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::ReadLine( CORE::CString& line, UInt32 timeoutInMs, UInt32 maxLength )
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

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::ReadBytes( UInt32 count, CORE::CDynamicBuffer& bytes, UInt32 timeoutInMs )
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

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::ReadDotTerminated( CORE::CDynamicBuffer& data, UInt32 timeoutInMs, UInt32 maxSize )
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

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::Write( const void* data, UInt32 size )
{
    #if defined( __linux__ )
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
        if ( NULL != m_ssl )
        {
            m_api->ERR_clear_error();
            int result = m_api->SSL_write( m_ssl, bytes + written, (int)( size - written > 16384 ? 16384 : size - written ) );
            if ( result <= 0 )
            {
                int error = m_api->SSL_get_error( m_ssl, result );
                if ( 2 == error || 3 == error )
                    continue;
                m_closed = true;
                return false;
            }
            written += (UInt32) result;
        }
        else
        {
            ssize_t result = ::send( m_fd, bytes + written, size - written, MSG_NOSIGNAL );
            if ( result <= 0 )
            {
                if ( result < 0 && ( EAGAIN == errno || EINTR == errno ) )
                    continue;
                m_closed = true;
                return false;
            }
            written += (UInt32) result;
        }
    }
    return true;
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::WriteLine( const CORE::CString& line )
{
    return WriteText( line + "\r\n" );
}

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::WriteText( const CORE::CString& text )
{
    return Write( text.C_String(), text.ByteSize() > 0 ? text.ByteSize() - 1 : 0 );
}

/*-------------------------------------------------------------------------*/

inline bool
CTestServerConnection::StartTls( void )
{
    #if defined( __linux__ )
    if ( NULL == m_tlsContext || NULL != m_ssl || m_closed )
        return false;
    // whatever the client sent in plain text before it started the handshake is not part of it
    void* ssl = m_api->SSL_new( m_tlsContext );
    if ( NULL == ssl )
        return false;
    m_api->SSL_set_fd( ssl, m_fd );
    m_api->ERR_clear_error();
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    ::setsockopt( m_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
    if ( 1 != m_api->SSL_accept( ssl ) )
    {
        m_api->ERR_clear_error();
        m_api->SSL_free( ssl );
        m_closed = true;
        ::close( m_fd );
        m_fd = -1;
        return false;
    }
    timeout.tv_sec = 0;
    ::setsockopt( m_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );
    m_ssl = ssl;
    return true;
    #else
    return false;
    #endif
}

/*-------------------------------------------------------------------------*/

inline void
CTestServerConnection::Close( void )
{
    #if defined( __linux__ )
    if ( NULL != m_ssl )
    {
        m_api->ERR_clear_error();
        m_api->SSL_shutdown( m_ssl );
        m_api->SSL_free( m_ssl );
        m_ssl = NULL;
    }
    if ( m_fd >= 0 )
    {
        ::close( m_fd );
        m_fd = -1;
    }
    #endif
    m_closed = true;
}

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_TCP_SERVER_H ? */
