/*
 *  gucef common header: header-only scripted HTTP proxy for tests
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
 *  Test support: a proxy for tests of the proxy support of HTTP clients. It listens on a local port and
 *  - answers a CONNECT with 200 and then relays the bytes to a server on this computer (whatever host the CONNECT names:
 *    this is how a test reaches a server through a host name that does not resolve), or refuses it with the status it was
 *    given, and
 *  - answers a plain request with an absolute URL ("GET http://host/path") with a fixed response, which tells the client got there.
 *  It records the first line of the requests it saw and the value of the header Proxy-Authorization of the last one.
 *  Linux only (BSD sockets, pthreads, poll); elsewhere Start() is false.
 */

#ifndef GUCEF_TEST_HTTP_PROXY_H
#define GUCEF_TEST_HTTP_PROXY_H

#ifdef __cplusplus

#if defined( __linux__ )

  #include <string.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <poll.h>
  #include <pthread.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>
  #include <arpa/inet.h>

#endif /* Linux ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

class CTestHttpProxy
{
    public:

    CTestHttpProxy( void )
        : m_listenFd( -1 ), m_port( 0 ), m_upstreamPort( 0 ), m_refuseStatus( 0 ), m_running( false ), m_threadStarted( false )
        , m_liveConnections( 0 ), m_requestLines(), m_lastProxyAuthorization(), m_lock()
    {
    }

    ~CTestHttpProxy() { Stop(); }

    /** Listens on a free local port. CONNECTs are relayed to 127.0.0.1:upstreamPort, unless refuseStatus is not 0: then they get that status */
    bool Start( UInt16 upstreamPort, int refuseStatus = 0 )
    {
        #if defined( __linux__ )
        Stop();
        m_upstreamPort = upstreamPort;
        m_refuseStatus = refuseStatus;
        m_listenFd = ::socket( AF_INET, SOCK_STREAM, 0 );
        if ( m_listenFd < 0 )
            return false;
        int one = 1;
        ::setsockopt( m_listenFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ) );
        struct sockaddr_in addr;
        memset( &addr, 0, sizeof( addr ) );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        if ( 0 != ::bind( m_listenFd, (struct sockaddr*) &addr, sizeof( addr ) ) || 0 != ::listen( m_listenFd, 16 ) )
        {
            ::close( m_listenFd );
            m_listenFd = -1;
            return false;
        }
        socklen_t length = sizeof( addr );
        ::getsockname( m_listenFd, (struct sockaddr*) &addr, &length );
        m_port = ntohs( addr.sin_port );
        m_running = true;
        if ( 0 != pthread_create( &m_thread, NULL, &CTestHttpProxy::AcceptMain, this ) )
        {
            m_running = false;
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
        m_running = false;
        if ( m_threadStarted )
        {
            pthread_join( m_thread, NULL );
            m_threadStarted = false;
        }
        for ( int i=0; i<500 && GetLive() > 0; ++i )
            MT::ThreadDelay( 10 );
        if ( m_listenFd >= 0 )
        {
            ::close( m_listenFd );
            m_listenFd = -1;
        }
        #endif
        m_port = 0;
    }

    UInt16 GetPort( void ) const { return m_port; }

    /** the first lines of the requests seen, one per line: "CONNECT tls-target.test:47123 HTTP/1.1" */
    CORE::CString GetRequestLines( void ) { MT::CScopeMutex lock( m_lock ); return m_requestLines; }

    CORE::CString GetLastProxyAuthorization( void ) { MT::CScopeMutex lock( m_lock ); return m_lastProxyAuthorization; }

    void ClearRecording( void ) { MT::CScopeMutex lock( m_lock ); m_requestLines.Clear(); m_lastProxyAuthorization.Clear(); }

    private:

    CTestHttpProxy( const CTestHttpProxy& src );
    CTestHttpProxy& operator=( const CTestHttpProxy& src );

    UInt32 GetLive( void ) { MT::CScopeMutex lock( m_lock ); return m_liveConnections; }

    #if defined( __linux__ )

    struct SConnection
    {
        CTestHttpProxy* owner;
        int fd;
    };

    static void* AcceptMain( void* arg )
    {
        CTestHttpProxy* self = static_cast< CTestHttpProxy* >( arg );
        while ( self->m_running )
        {
            struct pollfd pfd;
            pfd.fd = self->m_listenFd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            if ( ::poll( &pfd, 1, 50 ) <= 0 || 0 == ( pfd.revents & POLLIN ) )
                continue;
            int fd = ::accept( self->m_listenFd, NULL, NULL );
            if ( fd < 0 )
                continue;
            SConnection* connection = new SConnection;
            connection->owner = self;
            connection->fd = fd;
            {
                MT::CScopeMutex lock( self->m_lock );
                ++self->m_liveConnections;
            }
            pthread_t thread;
            pthread_attr_t attributes;
            pthread_attr_init( &attributes );
            pthread_attr_setdetachstate( &attributes, PTHREAD_CREATE_DETACHED );
            if ( 0 != pthread_create( &thread, &attributes, &CTestHttpProxy::ConnectionMain, connection ) )
            {
                ::close( fd );
                delete connection;
                MT::CScopeMutex lock( self->m_lock );
                --self->m_liveConnections;
            }
            pthread_attr_destroy( &attributes );
        }
        return NULL;
    }

    static void* ConnectionMain( void* arg )
    {
        SConnection* connection = static_cast< SConnection* >( arg );
        CTestHttpProxy* owner = connection->owner;
        int fd = connection->fd;
        delete connection;
        owner->Serve( fd );
        ::close( fd );
        MT::CScopeMutex lock( owner->m_lock );
        --owner->m_liveConnections;
        return NULL;
    }

    static bool SendAll( int fd, const char* data, size_t size )
    {
        size_t written = 0;
        while ( written < size )
        {
            ssize_t result = ::send( fd, data + written, size - written, MSG_NOSIGNAL );
            if ( result <= 0 )
                return false;
            written += (size_t) result;
        }
        return true;
    }

    void Serve( int fd )
    {
        // the head of the request
        char buffer[ 16384 ];
        size_t have = 0;
        const char* headEnd = NULL;
        while ( m_running && NULL == headEnd && have < sizeof( buffer ) - 1 )
        {
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLIN;
            pfd.revents = 0;
            if ( ::poll( &pfd, 1, 50 ) <= 0 )
                continue;
            ssize_t got = ::recv( fd, buffer + have, sizeof( buffer ) - 1 - have, 0 );
            if ( got <= 0 )
                return;
            have += (size_t) got;
            buffer[ have ] = '\0';
            headEnd = strstr( buffer, "\r\n\r\n" );
        }
        if ( NULL == headEnd )
            return;

        CORE::CString head( buffer, (UInt32) ( headEnd - buffer ) );
        Int32 lineEnd = head.HasSubstr( "\r\n" );
        CORE::CString firstLine = lineEnd < 0 ? head : head.SubstrToIndex( (UInt32) lineEnd, true );
        {
            MT::CScopeMutex lock( m_lock );
            m_requestLines += firstLine + "\n";
            m_lastProxyAuthorization.Clear();
            Int32 auth = head.Lowercase().HasSubstr( "\r\nproxy-authorization:" );
            if ( auth >= 0 )
            {
                CORE::CString rest = head.CutChars( (UInt32) auth + 2, true );
                Int32 end = rest.HasSubstr( "\r\n" );
                CORE::CString header = end < 0 ? rest : rest.SubstrToIndex( (UInt32) end, true );
                m_lastProxyAuthorization = header.CutChars( (UInt32) header.HasChar( ':' ) + 1, true ).Trim( true ).Trim( false );
            }
        }

        if ( firstLine.StartsWith( "CONNECT " ) )
        {
            if ( 0 != m_refuseStatus )
            {
                char response[ 128 ];
                sprintf( response, "HTTP/1.1 %d Refused\r\nContent-Length: 0\r\n\r\n", m_refuseStatus );
                SendAll( fd, response, strlen( response ) );
                return;
            }

            int upstream = ::socket( AF_INET, SOCK_STREAM, 0 );
            struct sockaddr_in addr;
            memset( &addr, 0, sizeof( addr ) );
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
            addr.sin_port = htons( m_upstreamPort );
            if ( upstream < 0 || 0 != ::connect( upstream, (struct sockaddr*) &addr, sizeof( addr ) ) )
            {
                if ( upstream >= 0 )
                    ::close( upstream );
                static const char bad[] = "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 0\r\n\r\n";
                SendAll( fd, bad, sizeof( bad ) - 1 );
                return;
            }
            int one = 1;
            ::setsockopt( upstream, IPPROTO_TCP, TCP_NODELAY, &one, sizeof( one ) );
            ::setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof( one ) );
            static const char ok[] = "HTTP/1.1 200 Connection established\r\n\r\n";
            SendAll( fd, ok, sizeof( ok ) - 1 );

            // whatever the client sent behind the head already
            size_t consumed = (size_t) ( headEnd - buffer ) + 4;
            if ( have > consumed )
                SendAll( upstream, buffer + consumed, have - consumed );
            Relay( fd, upstream );
            ::close( upstream );
            return;
        }

        static const char answer[] = "HTTP/1.1 200 OK\r\nContent-Length: 7\r\nConnection: close\r\n\r\nproxied";
        SendAll( fd, answer, sizeof( answer ) - 1 );
    }

    void Relay( int a, int b )
    {
        char buffer[ 16384 ];
        bool done = false;
        while ( !done && m_running )
        {
            struct pollfd fds[ 2 ];
            fds[ 0 ].fd = a; fds[ 0 ].events = POLLIN; fds[ 0 ].revents = 0;
            fds[ 1 ].fd = b; fds[ 1 ].events = POLLIN; fds[ 1 ].revents = 0;
            if ( ::poll( fds, 2, 50 ) <= 0 )
                continue;
            for ( int i=0; i<2 && !done; ++i )
            {
                if ( fds[ i ].revents & ( POLLIN | POLLHUP | POLLERR ) )
                {
                    ssize_t got = ::recv( fds[ i ].fd, buffer, sizeof( buffer ), 0 );
                    if ( got <= 0 || !SendAll( 0 == i ? b : a, buffer, (size_t) got ) )
                        done = true;
                }
            }
        }
    }

    pthread_t m_thread;

    #endif /* Linux ? */

    int m_listenFd;
    UInt16 m_port;
    UInt16 m_upstreamPort;
    int m_refuseStatus;
    volatile bool m_running;
    bool m_threadStarted;
    UInt32 m_liveConnections;
    CORE::CString m_requestLines;
    CORE::CString m_lastProxyAuthorization;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __cplusplus ? */

#endif /* GUCEF_TEST_HTTP_PROXY_H ? */
