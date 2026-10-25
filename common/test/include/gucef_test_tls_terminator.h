/*
 *  gucef common header: header-only TLS terminator for tests
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
 *  Test support: a TLS server that puts TLS in front of a plain TCP server of the test, so that the servers that
 *  exist for tests (the HTTP servers, the web socket server, a fake OpenID provider) can be reached over https and
 *  wss. It listens on a local port, does the TLS side of every connection on a thread of its own and relays the
 *  plaintext to the port of the server behind it.
 *
 *  It uses the OpenSSL of the system, loaded when it starts (like the TLS provider of gucefCOMCORE does), so it needs
 *  no OpenSSL development files either; without the library Start() is false and a test skips what needs it.
 *  The certificate and key are those of gucef_test_tls_server.h (CTlsTestCerts). This is test code, not a TLS server
 *  for applications: the TLS server of the framework is a later phase of the TLS work.
 *
 *    GUCEF::TESTFW::CTlsTestTerminator terminator;
 *    GUCEF_TESTFW_SKIP_IF( !terminator.Start( certs.GetPath( "good.crt" ), certs.GetPath( "good.key" ), plainPort ), "no TLS server library" );
 *    ... connect to https://localhost:<terminator.GetPort()> ...
 *
 *  Linux (BSD sockets, pthreads, poll) only; elsewhere Start() is false.
 */

#ifndef GUCEF_TEST_TLS_TERMINATOR_H
#define GUCEF_TEST_TLS_TERMINATOR_H

#ifdef __cplusplus

#if defined( __linux__ )

  #include <string.h>
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

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

namespace GUCEF {
namespace TESTFW {

/*-------------------------------------------------------------------------*/

/** The entry points of OpenSSL that a TLS server needs, looked up by name in the libraries of the system */
struct STlsTerminatorApi
{
    int ( *OPENSSL_init_ssl )( unsigned long long opts, const void* settings );
    const void* ( *TLS_server_method )( void );
    void* ( *SSL_CTX_new )( const void* method );
    void ( *SSL_CTX_free )( void* ctx );
    int ( *SSL_CTX_use_certificate_chain_file )( void* ctx, const char* file );
    int ( *SSL_CTX_use_PrivateKey_file )( void* ctx, const char* file, int type );
    void* ( *SSL_new )( void* ctx );
    void ( *SSL_free )( void* ssl );
    int ( *SSL_set_fd )( void* ssl, int fd );
    int ( *SSL_accept )( void* ssl );
    int ( *SSL_read )( void* ssl, void* buf, int num );
    int ( *SSL_write )( void* ssl, const void* buf, int num );
    int ( *SSL_pending )( const void* ssl );
    int ( *SSL_get_error )( const void* ssl, int ret );
    int ( *SSL_shutdown )( void* ssl );
    void ( *ERR_clear_error )( void );

    STlsTerminatorApi( void ) { memset( this, 0, sizeof( STlsTerminatorApi ) ); }
};

/*-------------------------------------------------------------------------*/

class CTlsTestTerminator
{
    public:

    CTlsTestTerminator( void )
        : m_api()
        , m_ctx( NULL )
        , m_listenFd( -1 )
        , m_port( 0 )
        , m_backendPort( 0 )
        , m_running( false )
        , m_abortWithoutCloseNotify( false )
        , m_acceptThreadStarted( false )
        , m_liveConnections( 0 )
        , m_acceptedConnections( 0 )
        , m_lock()
    {
    }

    ~CTlsTestTerminator()
    {
        Stop();
    }

    /** Listens on a free local port and relays to the plain server on backendPort. @return false without the OpenSSL library or the certificate */
    bool Start( const CORE::CString& certificateFile, const CORE::CString& keyFile, UInt16 backendPort )
    {
        #if defined( __linux__ )
        Stop();
        if ( !LoadApi() )
            return false;

        m_ctx = m_api.SSL_CTX_new( m_api.TLS_server_method() );
        if ( NULL == m_ctx )
            return false;
        if ( 1 != m_api.SSL_CTX_use_certificate_chain_file( m_ctx, certificateFile.C_String() ) ||
             1 != m_api.SSL_CTX_use_PrivateKey_file( m_ctx, keyFile.C_String(), 1 /* SSL_FILETYPE_PEM */ ) )
        {
            m_api.SSL_CTX_free( m_ctx );
            m_ctx = NULL;
            return false;
        }

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
        m_backendPort = backendPort;

        m_running = true;
        if ( 0 != pthread_create( &m_acceptThread, NULL, &CTlsTestTerminator::AcceptThreadMain, this ) )
        {
            m_running = false;
            return false;
        }
        m_acceptThreadStarted = true;
        return true;
        #else
        return false;
        #endif
    }

    void Stop( void )
    {
        #if defined( __linux__ )
        m_running = false;
        if ( m_acceptThreadStarted )
        {
            pthread_join( m_acceptThread, NULL );
            m_acceptThreadStarted = false;
        }

        // the connections end at their next look at the flag
        for ( int i=0; i<500 && GetLiveConnections() > 0; ++i )
            MT::ThreadDelay( 10 );

        if ( m_listenFd >= 0 )
        {
            ::close( m_listenFd );
            m_listenFd = -1;
        }
        if ( NULL != m_ctx )
        {
            m_api.SSL_CTX_free( m_ctx );
            m_ctx = NULL;
        }
        #endif
        m_port = 0;
    }

    /** The port to connect to for TLS */
    UInt16 GetPort( void ) const { return m_port; }

    /** With true a connection that ends is cut at the TCP level without the TLS close notification, as an attacker or a fault would: the data may be truncated */
    void SetAbortWithoutCloseNotify( bool abortWithoutCloseNotify ) { m_abortWithoutCloseNotify = abortWithoutCloseNotify; }

    /** How many connections were accepted so far */
    UInt32 GetAcceptedConnections( void ) { MT::CScopeMutex lock( m_lock ); return m_acceptedConnections; }

    private:

    CTlsTestTerminator( const CTlsTestTerminator& src );
    CTlsTestTerminator& operator=( const CTlsTestTerminator& src );

    UInt32 GetLiveConnections( void ) { MT::CScopeMutex lock( m_lock ); return m_liveConnections; }

    #if defined( __linux__ )

    struct SConnection
    {
        CTlsTestTerminator* owner;
        int clientFd;
    };

    bool LoadApi( void )
    {
        if ( NULL != m_api.SSL_CTX_new )
            return true;

        void* ssl = CORE::LoadModuleDynamicly( "libssl.so.3" );
        if ( NULL == ssl )
            ssl = CORE::LoadModuleDynamicly( "libssl.so.1.1" );
        if ( NULL == ssl )
            return false;
        void* crypto = CORE::LoadModuleDynamicly( "libcrypto.so.3" );
        if ( NULL == crypto )
            crypto = CORE::LoadModuleDynamicly( "libcrypto.so.1.1" );
        if ( NULL == crypto )
            return false;

        STlsTerminatorApi api;
        api.OPENSSL_init_ssl = (int ( * )( unsigned long long, const void* )) Look( ssl, "OPENSSL_init_ssl" );
        api.TLS_server_method = (const void* ( * )( void )) Look( ssl, "TLS_server_method" );
        api.SSL_CTX_new = (void* ( * )( const void* )) Look( ssl, "SSL_CTX_new" );
        api.SSL_CTX_free = (void ( * )( void* )) Look( ssl, "SSL_CTX_free" );
        api.SSL_CTX_use_certificate_chain_file = (int ( * )( void*, const char* )) Look( ssl, "SSL_CTX_use_certificate_chain_file" );
        api.SSL_CTX_use_PrivateKey_file = (int ( * )( void*, const char*, int )) Look( ssl, "SSL_CTX_use_PrivateKey_file" );
        api.SSL_new = (void* ( * )( void* )) Look( ssl, "SSL_new" );
        api.SSL_free = (void ( * )( void* )) Look( ssl, "SSL_free" );
        api.SSL_set_fd = (int ( * )( void*, int )) Look( ssl, "SSL_set_fd" );
        api.SSL_accept = (int ( * )( void* )) Look( ssl, "SSL_accept" );
        api.SSL_read = (int ( * )( void*, void*, int )) Look( ssl, "SSL_read" );
        api.SSL_write = (int ( * )( void*, const void*, int )) Look( ssl, "SSL_write" );
        api.SSL_pending = (int ( * )( const void* )) Look( ssl, "SSL_pending" );
        api.SSL_get_error = (int ( * )( const void*, int )) Look( ssl, "SSL_get_error" );
        api.SSL_shutdown = (int ( * )( void* )) Look( ssl, "SSL_shutdown" );
        api.ERR_clear_error = (void ( * )( void )) Look( crypto, "ERR_clear_error" );

        if ( NULL == api.OPENSSL_init_ssl || NULL == api.TLS_server_method || NULL == api.SSL_CTX_new || NULL == api.SSL_CTX_free ||
             NULL == api.SSL_CTX_use_certificate_chain_file || NULL == api.SSL_CTX_use_PrivateKey_file || NULL == api.SSL_new || NULL == api.SSL_free ||
             NULL == api.SSL_set_fd || NULL == api.SSL_accept || NULL == api.SSL_read || NULL == api.SSL_write || NULL == api.SSL_pending ||
             NULL == api.SSL_get_error || NULL == api.SSL_shutdown || NULL == api.ERR_clear_error )
            return false;

        api.OPENSSL_init_ssl( 0, NULL );
        m_api = api;
        return true;
    }

    static void* Look( void* library, const char* name )
    {
        return CORE::GetFunctionAddress( library, name, 0 ).objPtr;
    }

    static void* AcceptThreadMain( void* arg )
    {
        CTlsTestTerminator* self = static_cast< CTlsTestTerminator* >( arg );
        self->AcceptLoop();
        return NULL;
    }

    void AcceptLoop( void )
    {
        while ( m_running )
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

            SConnection* connection = new SConnection;
            connection->owner = this;
            connection->clientFd = clientFd;
            {
                MT::CScopeMutex lock( m_lock );
                ++m_liveConnections;
                ++m_acceptedConnections;
            }
            pthread_t thread;
            pthread_attr_t attributes;
            pthread_attr_init( &attributes );
            pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );
            if ( 0 != pthread_create( &thread, &attributes, &CTlsTestTerminator::ConnectionThreadMain, connection ) )
            {
                ::close( clientFd );
                delete connection;
                MT::CScopeMutex lock( m_lock );
                --m_liveConnections;
            }
            pthread_attr_destroy( &attributes );
        }
    }

    static void* ConnectionThreadMain( void* arg )
    {
        SConnection* connection = static_cast< SConnection* >( arg );
        CTlsTestTerminator* owner = connection->owner;
        int clientFd = connection->clientFd;
        delete connection;

        // the library writes to the socket of a client that may be gone: that must not take the process down
        sigset_t pipeSignal;
        sigemptyset( &pipeSignal );
        sigaddset( &pipeSignal, SIGPIPE );
        pthread_sigmask( SIG_BLOCK, &pipeSignal, NULL );

        owner->Relay( clientFd );
        MT::CScopeMutex lock( owner->m_lock );
        --owner->m_liveConnections;
        return NULL;
    }

    /** the TLS side of one connection and the plain side to the server behind, until either ends */
    void Relay( int clientFd )
    {
        int one = 1;
        ::setsockopt( clientFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof( one ) );
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        ::setsockopt( clientFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );

        void* ssl = m_api.SSL_new( m_ctx );
        if ( NULL == ssl )
        {
            ::close( clientFd );
            return;
        }
        m_api.SSL_set_fd( ssl, clientFd );
        m_api.ERR_clear_error();
        if ( 1 != m_api.SSL_accept( ssl ) )
        {
            // the client did not like the certificate, or never spoke TLS
            m_api.ERR_clear_error();
            m_api.SSL_free( ssl );
            ::close( clientFd );
            return;
        }
        timeout.tv_sec = 0;
        ::setsockopt( clientFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof( timeout ) );

        int backendFd = ::socket( AF_INET, SOCK_STREAM, 0 );
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        addr.sin_port = htons( m_backendPort );
        if ( backendFd < 0 || 0 != ::connect( backendFd, (struct sockaddr*) &addr, sizeof( addr ) ) )
        {
            if ( backendFd >= 0 )
                ::close( backendFd );
            m_api.SSL_shutdown( ssl );
            m_api.SSL_free( ssl );
            ::close( clientFd );
            return;
        }
        ::setsockopt( backendFd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof( one ) );

        char buffer[ 16384 ];
        bool done = false;
        while ( !done && m_running )
        {
            struct pollfd fds[ 2 ];
            fds[ 0 ].fd = clientFd;
            fds[ 0 ].events = POLLIN;
            fds[ 0 ].revents = 0;
            fds[ 1 ].fd = backendFd;
            fds[ 1 ].events = POLLIN;
            fds[ 1 ].revents = 0;

            // what OpenSSL has already decrypted is not visible to poll
            bool pending = m_api.SSL_pending( ssl ) > 0;
            if ( !pending && ::poll( fds, 2, 50 ) <= 0 )
                continue;

            if ( pending || ( fds[ 0 ].revents & ( POLLIN | POLLHUP | POLLERR ) ) )
            {
                m_api.ERR_clear_error();
                int got = m_api.SSL_read( ssl, buffer, (int) sizeof( buffer ) );
                if ( got > 0 )
                {
                    if ( !WriteAll( backendFd, buffer, got ) )
                        done = true;
                }
                else
                {
                    int error = m_api.SSL_get_error( ssl, got );
                    if ( 2 != error && 3 != error )     // anything but "again": the client is done
                        done = true;
                }
            }

            if ( !done && ( fds[ 1 ].revents & ( POLLIN | POLLHUP | POLLERR ) ) )
            {
                ssize_t got = ::recv( backendFd, buffer, sizeof( buffer ), 0 );
                if ( got <= 0 )
                    done = true;
                else
                {
                    int written = 0;
                    while ( written < got )
                    {
                        m_api.ERR_clear_error();
                        int result = m_api.SSL_write( ssl, buffer + written, (int) got - written );
                        if ( result <= 0 )
                        {
                            done = true;
                            break;
                        }
                        written += result;
                    }
                }
            }
        }

        m_api.ERR_clear_error();
        if ( !m_abortWithoutCloseNotify )
            m_api.SSL_shutdown( ssl );
        m_api.SSL_free( ssl );
        ::close( backendFd );
        ::close( clientFd );
    }

    static bool WriteAll( int fd, const char* data, int size )
    {
        int written = 0;
        while ( written < size )
        {
            ssize_t result = ::send( fd, data + written, size - written, MSG_NOSIGNAL );
            if ( result <= 0 )
                return false;
            written += (int) result;
        }
        return true;
    }

    pthread_t m_acceptThread;

    #endif /* Linux ? */

    STlsTerminatorApi m_api;
    void* m_ctx;
    int m_listenFd;
    UInt16 m_port;
    UInt16 m_backendPort;
    volatile bool m_running;
    volatile bool m_abortWithoutCloseNotify;
    bool m_acceptThreadStarted;
    UInt32 m_liveConnections;
    UInt32 m_acceptedConnections;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_TLS_TERMINATOR_H ? */
