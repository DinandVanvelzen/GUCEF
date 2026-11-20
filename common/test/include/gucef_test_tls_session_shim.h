/*
 *  gucef common header: header-only support for tests
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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
 *  Test support for macOS: the TLS servers of the tests (gucef_test_tls_terminator.h, gucef_test_tcp_server.h) are made with the
 *  entry points of OpenSSL, which they look up in libssl. macOS has no OpenSSL to look in, so this fills the same table of entry
 *  points with functions that do the TLS of a connection with the TLS server side of the provider that the framework has on that
 *  system (the sessions of gucefCOMCORE), over a socket that is read and written here. The behavior is that of the functions it
 *  replaces for what the tests use: blocking reads and writes on a socket of which the timeouts the test set, "again" as error 2.
 */

#ifndef GUCEF_TEST_TLS_SESSION_SHIM_H
#define GUCEF_TEST_TLS_SESSION_SHIM_H

#if defined( __APPLE__ ) && defined( __cplusplus )

#include <string.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#include "gucefCOMCORE_CTlsProviderRegistry.h"
#define GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#endif /* GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H ? */

namespace GUCEF {
namespace TESTFW {
namespace SHIM {

/** the values the tests look at, those of OpenSSL */
static const int ERROR_WANT_READ = 2;
static const int ERROR_SYSCALL = 5;
static const int ERROR_ZERO_RETURN = 6;
static const int ERROR_SSL = 1;

struct SContext
{
    CORE::CString certificateFile;
    CORE::CString keyFile;
    bool usable;

    SContext( void ) : certificateFile(), keyFile(), usable( true ) {}
};

struct SConnection
{
    SContext* context;
    int fd;
    COMCORE::TTlsSessionPtr session;
    std::vector< char > plaintext;
    size_t plaintextPos;
    int lastError;
    bool closedByPeer;

    SConnection( void ) : context( NULL ), fd( -1 ), session(), plaintext(), plaintextPos( 0 ), lastError( 0 ), closedByPeer( false ) {}
};

inline bool
SendAll( int fd, const char* data, size_t size )
{
    size_t written = 0;
    while ( written < size )
    {
        ssize_t result = ::send( fd, data + written, size - written, 0 );
        if ( result < 0 && ( EINTR == errno ) )
            continue;
        if ( result < 0 && ( EAGAIN == errno || EWOULDBLOCK == errno ) )
        {
            struct pollfd pfd;
            pfd.fd = fd;
            pfd.events = POLLOUT;
            pfd.revents = 0;
            if ( ::poll( &pfd, 1, 5000 ) <= 0 )
                return false;
            continue;
        }
        if ( result <= 0 )
            return false;
        written += (size_t) result;
    }
    return true;
}

/** sends what the session has produced for the peer */
inline bool
Flush( SConnection* connection )
{
    CORE::CDynamicBuffer out;
    if ( !connection->session->PullCiphertext( out ) )
        return true;
    return SendAll( connection->fd, static_cast< const char* >( out.GetConstBufferPtr() ), out.GetDataSize() );
}

inline void
TakePlaintext( SConnection* connection )
{
    CORE::CDynamicBuffer in;
    if ( connection->session->PullPlaintext( in ) )
    {
        const char* bytes = static_cast< const char* >( in.GetConstBufferPtr() );
        connection->plaintext.insert( connection->plaintext.end(), bytes, bytes + in.GetDataSize() );
    }
}

inline int InitSsl( unsigned long long, const void* ) { return 1; }
inline const void* ServerMethod( void ) { static int method = 0; return &method; }
inline void* ContextNew( const void* ) { return new SContext(); }
inline void ContextFree( void* ctx ) { delete static_cast< SContext* >( ctx ); }
inline void ClearError( void ) {}

inline int
ContextUseCertificateChainFile( void* ctx, const char* file )
{
    SContext* context = static_cast< SContext* >( ctx );
    context->certificateFile = file;
    return CORE::FileExists( context->certificateFile ) ? 1 : 0;
}

/** the key is the last piece: the pair is tried at once, so a pair that cannot be used is found when the context is made, as with OpenSSL */
inline int
ContextUsePrivateKeyFile( void* ctx, const char* file, int )
{
    SContext* context = static_cast< SContext* >( ctx );
    context->keyFile = file;
    CORE::CString whyNot;
    COMCORE::TITlsProviderPtr provider = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().GetBestAvailable( COMCORE::TLSCAP_SERVER, whyNot );
    if ( provider.IsNULL() )
        return 0;
    COMCORE::CTlsServerSettings settings;
    settings.SetCertificateChainPath( context->certificateFile );
    settings.SetPrivateKeyPath( context->keyFile );
    COMCORE::CTlsError error;
    return provider->CreateServerSession( settings, error ).IsNULL() ? 0 : 1;
}

inline void*
SslNew( void* ctx )
{
    SConnection* connection = new SConnection();
    connection->context = static_cast< SContext* >( ctx );
    return connection;
}

inline void SslFree( void* ssl ) { delete static_cast< SConnection* >( ssl ); }

inline int
SslSetFd( void* ssl, int fd )
{
    static_cast< SConnection* >( ssl )->fd = fd;
    return 1;
}

inline int
SslAccept( void* ssl )
{
    SConnection* connection = static_cast< SConnection* >( ssl );
    CORE::CString whyNot;
    COMCORE::TITlsProviderPtr provider = COMCORE::CComCoreGlobal::Instance()->GetTlsProviderRegistry().GetBestAvailable( COMCORE::TLSCAP_SERVER, whyNot );
    if ( provider.IsNULL() )
        return -1;
    COMCORE::CTlsServerSettings settings;
    settings.SetCertificateChainPath( connection->context->certificateFile );
    settings.SetPrivateKeyPath( connection->context->keyFile );
    COMCORE::CTlsError error;
    connection->session = provider->CreateServerSession( settings, error );
    if ( connection->session.IsNULL() || !connection->session->Start() )
        return -1;

    char chunk[ 16384 ];
    while ( COMCORE::TLSSTATE_ESTABLISHED != connection->session->GetState() )
    {
        if ( COMCORE::TLSSTATE_FAILED == connection->session->GetState() || COMCORE::TLSSTATE_CLOSED == connection->session->GetState() )
        {
            Flush( connection );        // the alert for the client
            return -1;
        }
        ssize_t got = ::recv( connection->fd, chunk, sizeof( chunk ), 0 );     // the timeout of the socket is the one of the handshake
        if ( got < 0 && EINTR == errno )
            continue;
        if ( got <= 0 )
            return -1;
        connection->session->PushCiphertext( chunk, (UInt32) got );
        if ( !Flush( connection ) )
            return -1;
    }
    Flush( connection );
    TakePlaintext( connection );
    return 1;
}

inline int
SslPending( const void* ssl )
{
    const SConnection* connection = static_cast< const SConnection* >( ssl );
    return (int) ( connection->plaintext.size() - connection->plaintextPos );
}

inline int
SslRead( void* ssl, void* buf, int num )
{
    SConnection* connection = static_cast< SConnection* >( ssl );
    for ( ;; )
    {
        const size_t available = connection->plaintext.size() - connection->plaintextPos;
        if ( available > 0 )
        {
            const size_t count = available < (size_t) num ? available : (size_t) num;
            ::memcpy( buf, &connection->plaintext[ connection->plaintextPos ], count );
            connection->plaintextPos += count;
            if ( connection->plaintextPos == connection->plaintext.size() )
            {
                connection->plaintext.clear();
                connection->plaintextPos = 0;
            }
            return (int) count;
        }
        if ( connection->closedByPeer || COMCORE::TLSSTATE_CLOSED == connection->session->GetState() )
        {
            connection->lastError = ERROR_ZERO_RETURN;
            return 0;
        }

        char chunk[ 16384 ];
        ssize_t got = ::recv( connection->fd, chunk, sizeof( chunk ), 0 );
        if ( got < 0 && EINTR == errno )
            continue;
        if ( got < 0 && ( EAGAIN == errno || EWOULDBLOCK == errno ) )
        {
            connection->lastError = ERROR_WANT_READ;
            return -1;
        }
        if ( got <= 0 )
        {
            connection->closedByPeer = true;
            connection->lastError = ERROR_SYSCALL;
            return 0;
        }
        connection->session->PushCiphertext( chunk, (UInt32) got );
        Flush( connection );
        TakePlaintext( connection );
        if ( COMCORE::TLSSTATE_FAILED == connection->session->GetState() )
        {
            connection->lastError = ERROR_SSL;
            return -1;
        }
        if ( connection->plaintext.size() == connection->plaintextPos && COMCORE::TLSSTATE_CLOSED != connection->session->GetState() )
        {
            // only a part of a record, or protocol messages: more has to come
            connection->lastError = ERROR_WANT_READ;
            return -1;
        }
    }
}

inline int
SslWrite( void* ssl, const void* buf, int num )
{
    SConnection* connection = static_cast< SConnection* >( ssl );
    if ( COMCORE::TLSSTATE_ESTABLISHED != connection->session->PushPlaintext( buf, (UInt32) num ) )
    {
        connection->lastError = ERROR_SSL;
        return -1;
    }
    if ( !Flush( connection ) )
    {
        connection->lastError = ERROR_SYSCALL;
        return -1;
    }
    return num;
}

inline int
SslGetError( const void* ssl, int )
{
    return static_cast< const SConnection* >( ssl )->lastError;
}

inline int
SslShutdown( void* ssl )
{
    SConnection* connection = static_cast< SConnection* >( ssl );
    if ( !connection->session.IsNULL() )
    {
        connection->session->InitiateClose();
        Flush( connection );
    }
    return 1;
}

} /* namespace SHIM */
} /* namespace TESTFW */
} /* namespace GUCEF */

#endif /* __APPLE__ && __cplusplus ? */

#endif /* GUCEF_TEST_TLS_SESSION_SHIM_H ? */
