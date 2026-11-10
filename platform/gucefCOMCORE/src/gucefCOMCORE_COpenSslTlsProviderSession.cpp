/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_COpenSslTlsProviderSession.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The bundles of trusted authorities of the distributions, tried when the library has no usable default */
static const char* KnownCaBundleFiles[] =
{
    "/etc/ssl/certs/ca-certificates.crt",                   // Debian, Ubuntu, Gentoo, Arch
    "/etc/pki/tls/certs/ca-bundle.crt",                     // Fedora, RHEL, CentOS
    "/etc/ssl/ca-bundle.pem",                               // openSUSE
    "/etc/ssl/cert.pem",                                    // Alpine, BSD, macOS style
    "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem",    // RHEL family
    GUCEF_NULL
};

/** A TLS record holds up to 16 KiB of data */
static const UInt32 PlaintextReadBlockSize = 16 * 1024;

/** How much of the error queue is kept */
static const int MaxErrorQueueEntries = 8;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** an IPv4 dotted quad or anything with a colon (IPv6) */
static bool
IsIpAddressLiteral( const CORE::CString& name )
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() )
        return false;
    if ( name.HasChar( ':' ) >= 0 )
        return true;

    UInt32 dots = 0;
    for ( UInt32 i=0; i<name.Length(); ++i )
    {
        char c = name[ i ];
        if ( '.' == c )
            ++dots;
        else
        if ( c < '0' || c > '9' )
            return false;
    }
    return 3 == dots;
}

/*-------------------------------------------------------------------------*/

static bool
HasText( const CORE::CString& lowerText, const char* needle )
{GUCEF_TRACE;

    return lowerText.HasSubstr( needle ) >= 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      COpenSslTlsProviderState                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

COpenSslTlsProviderState::COpenSslTlsProviderState( void )
    : api()
    , providerName( "openssl-dynamic" )
    , versionInfo()
    , verifyTimeOverride( 0 )
    , m_lock()
    , m_contexts()
    , m_resumable()
    , m_resumableOrder()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpenSslTlsProviderState::~COpenSslTlsProviderState()
{GUCEF_TRACE;

    for ( TVoidPtrMap::iterator i = m_resumable.begin(); i != m_resumable.end(); ++i )
        if ( GUCEF_NULL != api.SSL_SESSION_free )
            api.SSL_SESSION_free( (*i).second );
    m_resumable.clear();

    for ( TVoidPtrMap::iterator i = m_contexts.begin(); i != m_contexts.end(); ++i )
        if ( GUCEF_NULL != api.SSL_CTX_free )
            api.SSL_CTX_free( (*i).second );     // sessions still alive keep their own reference
    m_contexts.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProviderState::CollectErrors( void ) const
{GUCEF_TRACE;

    CORE::CString errors;
    char buffer[ 256 ];
    for ( int i=0; i<MaxErrorQueueEntries; ++i )
    {
        unsigned long e = api.ERR_get_error();
        if ( 0 == e )
            break;
        api.ERR_error_string_n( e, buffer, sizeof( buffer ) );
        buffer[ sizeof( buffer ) - 1 ] = '\0';
        if ( !errors.IsNULLOrEmpty() )
            errors += " | ";
        errors += buffer;
    }
    api.ERR_clear_error();
    return errors;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProviderState::LoadSystemTrust( void* ctx, CTlsError& error )
{GUCEF_TRACE;

    // SSL_CERT_FILE and SSL_CERT_DIR are honoured by the library itself in the default paths
    CORE::CString envFile = CORE::GetEnv( "SSL_CERT_FILE" );
    CORE::CString envDir = CORE::GetEnv( "SSL_CERT_DIR" );
    if ( !envFile.IsNULLOrEmpty() || !envDir.IsNULLOrEmpty() )
    {
        api.SSL_CTX_set_default_verify_paths( ctx );
        api.ERR_clear_error();
        return true;
    }

    // The paths the library was built with, when they exist on this system
    bool haveDefaults = false;
    if ( GUCEF_NULL != api.X509_get_default_cert_file )
    {
        const char* file = api.X509_get_default_cert_file();
        haveDefaults = GUCEF_NULL != file && CORE::FileExists( file );
    }
    if ( !haveDefaults && GUCEF_NULL != api.X509_get_default_cert_dir )
    {
        const char* dir = api.X509_get_default_cert_dir();
        haveDefaults = GUCEF_NULL != dir && CORE::DirExists( dir );
    }
    if ( haveDefaults )
    {
        api.SSL_CTX_set_default_verify_paths( ctx );
        api.ERR_clear_error();
        return true;
    }

    // Otherwise the bundle of the distribution
    for ( const char** bundle = KnownCaBundleFiles; GUCEF_NULL != *bundle; ++bundle )
    {
        if ( CORE::FileExists( *bundle ) )
        {
            if ( 1 == api.SSL_CTX_load_verify_locations( ctx, *bundle, GUCEF_NULL ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "OpenSslTlsProvider: Using the CA bundle \"" << *bundle << "\"" );
                return true;
            }
            api.ERR_clear_error();
        }
    }

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "OpenSslTlsProvider: No CA bundle of the system was found, only certificates from the settings can be trusted" );
    return true;
}

/*-------------------------------------------------------------------------*/

void*
COpenSslTlsProviderState::CreateContext( const CTlsSettings& settings, CTlsError& error )
{GUCEF_TRACE;

    api.ERR_clear_error();
    void* ctx = api.SSL_CTX_new( api.TLS_client_method() );
    if ( GUCEF_NULL == ctx )
    {
        error = CTlsError( TLSERR_INTERNAL, "SSL_CTX_new failed: " + CollectErrors() );
        return GUCEF_NULL;
    }

    // The floor is TLS 1.2, there is no setting below it
    int minVersion = TLSVERSION_1_3 == settings.GetMinVersion() ? OSSL_TLS1_3_VERSION : OSSL_TLS1_2_VERSION;
    api.SSL_CTX_ctrl( ctx, OSSL_SSL_CTRL_SET_MIN_PROTO_VERSION, minVersion, GUCEF_NULL );
    if ( TLSVERSION_ANY != settings.GetMaxVersion() )
        api.SSL_CTX_ctrl( ctx, OSSL_SSL_CTRL_SET_MAX_PROTO_VERSION, (int) settings.GetMaxVersion(), GUCEF_NULL );

    api.SetOptions( ctx, OSSL_SSL_OP_NO_COMPRESSION | OSSL_SSL_OP_NO_RENEGOTIATION );
    api.SSL_CTX_set_security_level( ctx, 2 );

    // Sessions are kept by the provider, not by the library: a client never looks into the cache of the library
    api.SSL_CTX_ctrl( ctx, OSSL_SSL_CTRL_SET_SESS_CACHE_MODE, settings.GetAllowResumption() ? ( OSSL_SSL_SESS_CACHE_CLIENT | OSSL_SSL_SESS_CACHE_NO_INTERNAL_STORE ) : OSSL_SSL_SESS_CACHE_OFF, GUCEF_NULL );

    if ( !settings.IsVerificationEnabled() )
    {
        api.SSL_CTX_set_verify( ctx, OSSL_SSL_VERIFY_NONE, GUCEF_NULL );
    }
    else
    {
        api.SSL_CTX_set_verify( ctx, OSSL_SSL_VERIFY_PEER, GUCEF_NULL );

        if ( settings.GetUseSystemTrust() )
            LoadSystemTrust( ctx, error );

        bool haveCustomTrust = false;
        const CORE::CString& caFile = settings.GetCaBundlePath();
        if ( !caFile.IsNULLOrEmpty() )
        {
            // A trust anchor that was asked for and cannot be read is an error: it must never turn into "trust nothing was asked" or "skip verification"
            if ( !CORE::FileExists( caFile ) || 1 != api.SSL_CTX_load_verify_locations( ctx, caFile.C_String(), GUCEF_NULL ) )
            {
                error = CTlsError( TLSERR_SETTINGS, "cannot load the CA file \"" + caFile + "\"" );
                api.ERR_clear_error();
                api.SSL_CTX_free( ctx );
                return GUCEF_NULL;
            }
            haveCustomTrust = true;
        }

        const CORE::CString& caDir = settings.GetCaDir();
        if ( !caDir.IsNULLOrEmpty() )
        {
            if ( !CORE::DirExists( caDir ) || 1 != api.SSL_CTX_load_verify_locations( ctx, GUCEF_NULL, caDir.C_String() ) )
            {
                error = CTlsError( TLSERR_SETTINGS, "cannot use the CA directory \"" + caDir + "\"" );
                api.ERR_clear_error();
                api.SSL_CTX_free( ctx );
                return GUCEF_NULL;
            }
            haveCustomTrust = true;
        }

        const CORE::CString& caData = settings.GetCaBundleData();
        if ( !caData.IsNULLOrEmpty() )
        {
            UInt32 added = 0;
            void* bio = api.BIO_new_mem_buf( caData.C_String(), (int) caData.Length() );
            void* store = api.SSL_CTX_get_cert_store( ctx );
            if ( GUCEF_NULL != bio && GUCEF_NULL != store )
            {
                void* cert = GUCEF_NULL;
                while ( GUCEF_NULL != ( cert = api.PEM_read_bio_X509( bio, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL ) ) )
                {
                    if ( 1 == api.X509_STORE_add_cert( store, cert ) )
                        ++added;
                    api.X509_free( cert );
                }
            }
            if ( GUCEF_NULL != bio )
                api.BIO_free( bio );
            api.ERR_clear_error();
            if ( 0 == added )
            {
                error = CTlsError( TLSERR_SETTINGS, "the CA certificates given as data contain no certificate" );
                api.SSL_CTX_free( ctx );
                return GUCEF_NULL;
            }
            haveCustomTrust = true;
        }

        if ( !settings.GetUseSystemTrust() && !haveCustomTrust )
        {
            error = CTlsError( TLSERR_SETTINGS, "the system trust store is not used and no CA certificate was given: nothing could be trusted" );
            api.SSL_CTX_free( ctx );
            return GUCEF_NULL;
        }
    }

    const CTlsSettings::TStringVector& alpn = settings.GetAlpnProtocols();
    if ( !alpn.empty() )
    {
        CORE::CDynamicBuffer wire;
        for ( size_t i=0; i<alpn.size(); ++i )
        {
            UInt32 length = alpn[ i ].Length();
            if ( 0 == length || length > 255 )
                continue;
            UInt8 lengthByte = (UInt8) length;
            wire.Append( &lengthByte, 1 );
            wire.Append( alpn[ i ].C_String(), length );
        }
        if ( wire.GetDataSize() > 0 && 0 != api.SSL_CTX_set_alpn_protos( ctx, static_cast< const unsigned char* >( wire.GetConstBufferPtr() ), wire.GetDataSize() ) )
        {
            error = CTlsError( TLSERR_SETTINGS, "the ALPN protocol list was refused by the library" );
            api.ERR_clear_error();
            api.SSL_CTX_free( ctx );
            return GUCEF_NULL;
        }
    }
    return ctx;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProviderState::GetContextKey( const CTlsSettings& settings )
{GUCEF_TRACE;

    // What the library takes from the environment for the trust of the system is part of what the context is
    CORE::CString key = settings.GetContextKey();
    if ( settings.GetUseSystemTrust() && settings.IsVerificationEnabled() )
        key += "|" + CORE::GetEnv( "SSL_CERT_FILE" ) + "|" + CORE::GetEnv( "SSL_CERT_DIR" );
    return key;
}

/*-------------------------------------------------------------------------*/

void*
COpenSslTlsProviderState::AcquireContext( const CTlsSettings& settings, CTlsError& error )
{GUCEF_TRACE;

    CORE::CString key = GetContextKey( settings );

    MT::CScopeMutex lock( m_lock );
    TVoidPtrMap::iterator i = m_contexts.find( key );
    if ( i != m_contexts.end() )
        return (*i).second;

    void* ctx = CreateContext( settings, error );
    if ( GUCEF_NULL != ctx )
        m_contexts[ key ] = ctx;
    return ctx;
}

/*-------------------------------------------------------------------------*/

void*
COpenSslTlsProviderState::TakeResumableSession( const CORE::CString& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TVoidPtrMap::iterator i = m_resumable.find( key );
    if ( i == m_resumable.end() )
        return GUCEF_NULL;

    void* session = (*i).second;
    m_resumable.erase( i );
    for ( TStringVector::iterator o = m_resumableOrder.begin(); o != m_resumableOrder.end(); ++o )
    {
        if ( (*o) == key )
        {
            m_resumableOrder.erase( o );
            break;
        }
    }
    return session;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderState::StoreResumableSession( const CORE::CString& key, void* session, UInt32 maxEntries )
{GUCEF_TRACE;

    if ( 0 == maxEntries )
    {
        api.SSL_SESSION_free( session );
        return;
    }

    MT::CScopeMutex lock( m_lock );

    TVoidPtrMap::iterator i = m_resumable.find( key );
    if ( i != m_resumable.end() )
    {
        api.SSL_SESSION_free( (*i).second );
        m_resumable.erase( i );
        for ( TStringVector::iterator o = m_resumableOrder.begin(); o != m_resumableOrder.end(); ++o )
        {
            if ( (*o) == key )
            {
                m_resumableOrder.erase( o );
                break;
            }
        }
    }

    m_resumable[ key ] = session;
    m_resumableOrder.push_back( key );

    while ( m_resumableOrder.size() > maxEntries )
    {
        TVoidPtrMap::iterator oldest = m_resumable.find( m_resumableOrder.front() );
        if ( oldest != m_resumable.end() )
        {
            api.SSL_SESSION_free( (*oldest).second );
            m_resumable.erase( oldest );
        }
        m_resumableOrder.erase( m_resumableOrder.begin() );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
COpenSslTlsProviderState::GetContextCount( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_contexts.size();
}

/*-------------------------------------------------------------------------*/

UInt32
COpenSslTlsProviderState::GetResumableSessionCount( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_resumable.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      COpenSslTlsProviderSession                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

COpenSslTlsProviderSession::COpenSslTlsProviderSession( const TOpenSslStatePtr& state    ,
                                                        const CTlsSettings& settings     ,
                                                        const CORE::CString& serverName  )
    : ITlsSession()
    , m_shared( state )
    , m_settings( settings )
    , m_serverName( serverName )
    , m_contextKey()
    , m_isIpAddress( IsIpAddressLiteral( serverName ) )
    , m_ssl( GUCEF_NULL )
    , m_readBio( GUCEF_NULL )
    , m_writeBio( GUCEF_NULL )
    , m_tlsState( TLSSTATE_IDLE )
    , m_error()
    , m_info()
    , m_plaintextOut()
    , m_wasEstablished( false )
    , m_shutdownSent( false )
    , m_sessionKept( false )
    , m_startTicks( 0 )
{GUCEF_TRACE;

    m_info.providerName = m_shared->providerName;
    m_info.providerVersion = m_shared->versionInfo;
}

/*-------------------------------------------------------------------------*/

COpenSslTlsProviderSession::~COpenSslTlsProviderSession()
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_ssl )
        return;

    const SOpenSslApi& api = m_shared->api;
    api.ERR_clear_error();

    // A session is only resumable when the connection was shut down before it is freed: SSL_free marks the session of
    // a connection that was not shut down as not resumable
    if ( m_wasEstablished && TLSSTATE_FAILED != m_tlsState )
    {
        if ( !m_shutdownSent )
        {
            api.SSL_shutdown( m_ssl );
            m_shutdownSent = true;
        }
        KeepSessionForResumption();
    }

    api.SSL_free( m_ssl );     // also frees the two BIOs
    api.ERR_clear_error();
    m_ssl = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProviderSession::GetResumptionKey( void ) const
{GUCEF_TRACE;

    // A session is only ever resumed in the context it was made in: another context may have another idea about what is trusted
    return m_contextKey + "#" + m_serverName;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::KeepSessionForResumption( void )
{GUCEF_TRACE;

    if ( m_sessionKept || !m_settings.GetAllowResumption() || 0 == m_settings.GetSessionCacheSize() )
        return;
    m_sessionKept = true;

    const SOpenSslApi& api = m_shared->api;
    void* session = api.SSL_get1_session( m_ssl );
    if ( GUCEF_NULL == session )
        return;

    // A session that has no ticket (yet) cannot be resumed
    if ( GUCEF_NULL != api.SSL_SESSION_is_resumable && 0 == api.SSL_SESSION_is_resumable( session ) )
    {
        api.SSL_SESSION_free( session );
        return;
    }
    m_shared->StoreResumableSession( GetResumptionKey(), session, m_settings.GetSessionCacheSize() );
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProviderSession::Initialize( CTlsError& error )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;

    m_contextKey = COpenSslTlsProviderState::GetContextKey( m_settings );
    void* ctx = m_shared->AcquireContext( m_settings, error );
    if ( GUCEF_NULL == ctx )
        return false;

    api.ERR_clear_error();
    m_ssl = api.SSL_new( ctx );
    m_readBio = api.BIO_new( api.BIO_s_mem() );
    m_writeBio = api.BIO_new( api.BIO_s_mem() );
    if ( GUCEF_NULL == m_ssl || GUCEF_NULL == m_readBio || GUCEF_NULL == m_writeBio )
    {
        error = CTlsError( TLSERR_INTERNAL, "cannot create the SSL object: " + m_shared->CollectErrors() );
        if ( GUCEF_NULL != m_readBio ) api.BIO_free( m_readBio );
        if ( GUCEF_NULL != m_writeBio ) api.BIO_free( m_writeBio );
        if ( GUCEF_NULL != m_ssl ) api.SSL_free( m_ssl );
        m_readBio = m_writeBio = m_ssl = GUCEF_NULL;
        return false;
    }
    api.SSL_set_bio( m_ssl, m_readBio, m_writeBio );     // the SSL object owns the BIOs from here
    api.SSL_set_connect_state( m_ssl );

    if ( m_settings.IsVerificationEnabled() && !m_serverName.IsNULLOrEmpty() )
    {
        // The name has to be in the subject alternative names: the common name is never used
        if ( m_isIpAddress )
        {
            if ( 1 != api.X509_VERIFY_PARAM_set1_ip_asc( api.SSL_get0_param( m_ssl ), m_serverName.C_String() ) )
            {
                error = CTlsError( TLSERR_SETTINGS, "the address given as server name cannot be verified" );
                api.ERR_clear_error();
                return false;
            }
        }
        else
        {
            api.SSL_set_hostflags( m_ssl, OSSL_X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS | OSSL_X509_CHECK_FLAG_NEVER_CHECK_SUBJECT );
            if ( 1 != api.SSL_set1_host( m_ssl, m_serverName.C_String() ) )
            {
                error = CTlsError( TLSERR_SETTINGS, "the server name cannot be verified" );
                api.ERR_clear_error();
                return false;
            }
        }
    }

    // SNI is sent for names, never for an address
    if ( !m_serverName.IsNULLOrEmpty() && !m_isIpAddress )
        api.SSL_ctrl( m_ssl, OSSL_SSL_CTRL_SET_TLSEXT_HOSTNAME, OSSL_TLSEXT_NAMETYPE_host_name, const_cast< char* >( m_serverName.C_String() ) );

    if ( 0 != m_shared->verifyTimeOverride && GUCEF_NULL != api.X509_VERIFY_PARAM_set_time )
        api.X509_VERIFY_PARAM_set_time( api.SSL_get0_param( m_ssl ), (time_t) m_shared->verifyTimeOverride );

    if ( m_settings.GetAllowResumption() )
    {
        void* resumable = m_shared->TakeResumableSession( GetResumptionKey() );
        if ( GUCEF_NULL != resumable )
        {
            api.SSL_set_session( m_ssl, resumable );        // takes its own reference
            api.SSL_SESSION_free( resumable );              // a session is used once, TLS 1.3 tickets require it
        }
    }
    api.ERR_clear_error();

    m_plaintextOut.SetBufferSize( PlaintextReadBlockSize, false );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProviderSession::Start( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_ssl || TLSSTATE_IDLE != m_tlsState )
        return false;

    m_tlsState = TLSSTATE_HANDSHAKING;
    m_startTicks = MT::PrecisionTickCount();
    Advance();
    return TLSSTATE_FAILED != m_tlsState;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::OnEstablished( void )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;

    m_wasEstablished = true;
    m_tlsState = TLSSTATE_ESTABLISHED;

    const char* version = api.SSL_get_version( m_ssl );
    m_info.protocolVersion = GUCEF_NULL != version ? version : "";
    const void* cipher = api.SSL_get_current_cipher( m_ssl );
    const char* cipherName = GUCEF_NULL != cipher ? api.SSL_CIPHER_get_name( cipher ) : GUCEF_NULL;
    m_info.cipherName = GUCEF_NULL != cipherName ? cipherName : "";

    const unsigned char* alpn = GUCEF_NULL;
    unsigned int alpnLength = 0;
    api.SSL_get0_alpn_selected( m_ssl, &alpn, &alpnLength );
    if ( GUCEF_NULL != alpn && alpnLength > 0 )
        m_info.alpnProtocol.Set( reinterpret_cast< const char* >( alpn ), alpnLength );

    m_info.resumed = 0 != api.SSL_session_reused( m_ssl );

    void* peerCert = api.PeerCertificate( m_ssl );
    if ( GUCEF_NULL != peerCert )
    {
        m_info.peerSubject = GetPeerSubject( peerCert );
        api.X509_free( peerCert );
    }

    UInt64 ticksPerSecond = MT::PrecisionTimerResolution();
    if ( 0 != ticksPerSecond )
        m_info.handshakeDurationInMs = (UInt32) ( ( MT::PrecisionTickCount() - m_startTicks ) * 1000 / ticksPerSecond );
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProviderSession::GetPeerSubject( void* peerCert )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;
    char buffer[ 512 ];
    buffer[ 0 ] = '\0';
    void* name = api.X509_get_subject_name( peerCert );
    if ( GUCEF_NULL != name )
        api.X509_NAME_oneline( name, buffer, (int) sizeof( buffer ) );
    buffer[ sizeof( buffer ) - 1 ] = '\0';
    return buffer;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProviderSession::GetCertificateDate( void* peerCert, bool notAfter )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;
    if ( GUCEF_NULL == api.X509_get0_notAfter || GUCEF_NULL == api.X509_get0_notBefore || GUCEF_NULL == api.ASN1_TIME_print )
        return CORE::CString::Empty;

    const void* time = notAfter ? api.X509_get0_notAfter( peerCert ) : api.X509_get0_notBefore( peerCert );
    void* bio = api.BIO_new( api.BIO_s_mem() );
    if ( GUCEF_NULL == time || GUCEF_NULL == bio )
    {
        if ( GUCEF_NULL != bio )
            api.BIO_free( bio );
        return CORE::CString::Empty;
    }

    char buffer[ 64 ];
    CORE::CString date;
    if ( 1 == api.ASN1_TIME_print( bio, time ) )
    {
        int got = api.BIO_read( bio, buffer, (int) sizeof( buffer ) - 1 );
        if ( got > 0 )
            date.Set( buffer, got );
    }
    api.BIO_free( bio );
    return date;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::FailWith( ETlsErrorClass errorClass, const CORE::CString& message )
{GUCEF_TRACE;

    m_error.errorClass = errorClass;
    m_error.message = message;
    m_tlsState = TLSSTATE_FAILED;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::ClassifyCertificateFailure( long verifyResult )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;

    m_error.verifyCode = (Int32) verifyResult;
    const char* verifyText = api.X509_verify_cert_error_string( verifyResult );
    CORE::CString text = GUCEF_NULL != verifyText ? CORE::CString( verifyText ) : CORE::CString( "certificate verification failed" );

    // The certificate of the peer is not kept as the peer certificate of the session when its verification failed. What
    // the peer sent is, as the first certificate of the chain it sent
    void* peerCert = api.PeerCertificate( m_ssl );
    bool mustFree = GUCEF_NULL != peerCert;
    if ( GUCEF_NULL == peerCert && GUCEF_NULL != api.OPENSSL_sk_num && GUCEF_NULL != api.OPENSSL_sk_value )
    {
        void* chain = api.SSL_get_peer_cert_chain( m_ssl );
        if ( GUCEF_NULL != chain && api.OPENSSL_sk_num( chain ) > 0 )
            peerCert = api.OPENSSL_sk_value( chain, 0 );
    }
    CORE::CString notAfter, notBefore;
    bool hasSan = true;
    if ( GUCEF_NULL != peerCert )
    {
        m_error.peerSubject = GetPeerSubject( peerCert );
        notAfter = GetCertificateDate( peerCert, true );
        notBefore = GetCertificateDate( peerCert, false );
        if ( GUCEF_NULL != api.X509_get_ext_by_NID )
            hasSan = api.X509_get_ext_by_NID( peerCert, OSSL_NID_subject_alt_name, -1 ) >= 0;
        if ( mustFree )
            api.X509_free( peerCert );
    }

    if ( OSSL_X509_V_ERR_CERT_HAS_EXPIRED == verifyResult )
    {
        m_error.errorClass = TLSERR_CERT_EXPIRED;
        if ( !notAfter.IsNULLOrEmpty() )
            text += ", expired on " + notAfter;
    }
    else
    if ( OSSL_X509_V_ERR_CERT_NOT_YET_VALID == verifyResult )
    {
        m_error.errorClass = TLSERR_CERT_NOT_YET_VALID;
        if ( !notBefore.IsNULLOrEmpty() )
            text += ", valid from " + notBefore;
    }
    else
    if ( OSSL_X509_V_ERR_HOSTNAME_MISMATCH == verifyResult || OSSL_X509_V_ERR_IP_ADDRESS_MISMATCH == verifyResult || OSSL_X509_V_ERR_EMAIL_MISMATCH == verifyResult )
        m_error.errorClass = hasSan ? TLSERR_CERT_HOSTNAME_MISMATCH : TLSERR_CERT_NO_SAN;
    else
    if ( OSSL_X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY == verifyResult || OSSL_X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT == verifyResult ||
         OSSL_X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE == verifyResult || OSSL_X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT == verifyResult ||
         OSSL_X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN == verifyResult )
        m_error.errorClass = TLSERR_CERT_UNTRUSTED;
    else
    if ( OSSL_X509_V_ERR_CERT_REVOKED == verifyResult )
        m_error.errorClass = TLSERR_CERT_REVOKED;
    else
        m_error.errorClass = TLSERR_CERT_OTHER;

    m_error.message = text;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::Fail( int sslErrorCode, const char* whileDoing )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;

    CORE::CString errors = m_shared->CollectErrors();
    m_tlsState = TLSSTATE_FAILED;
    m_error.providerCode = sslErrorCode;

    // The verification result says what is wrong with the certificate. It is not read when verification is off: it
    // is then only informational
    long verifyResult = m_settings.IsVerificationEnabled() ? api.SSL_get_verify_result( m_ssl ) : OSSL_X509_V_OK;
    if ( OSSL_X509_V_OK != verifyResult )
    {
        ClassifyCertificateFailure( verifyResult );
        return;
    }

    // Anything else by what the library says. The reason codes of the error queue are laid out differently in 1.1
    // and 3.x so the text is used instead
    CORE::CString lower = errors.Lowercase();
    if ( HasText( lower, "protocol version" ) || HasText( lower, "unsupported protocol" ) || HasText( lower, "no protocols available" ) )
        m_error.errorClass = TLSERR_PROTOCOL_VERSION;
    else
    if ( HasText( lower, "alert" ) )
        m_error.errorClass = TLSERR_PROTOCOL_ALERT;
    else
        m_error.errorClass = m_wasEstablished ? TLSERR_PROTOCOL_ALERT : TLSERR_HANDSHAKE_FAILURE;

    m_error.message = errors.IsNULLOrEmpty() ? CORE::CString( "TLS error " ) + CORE::ToString( sslErrorCode ) + " during " + whileDoing : errors;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::ReadApplicationData( void )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;

    while ( TLSSTATE_ESTABLISHED == m_tlsState || TLSSTATE_CLOSING == m_tlsState )
    {
        // room for one record, read straight into the buffer
        if ( m_plaintextOut.GetUnusedBufferSize() < PlaintextReadBlockSize )
            m_plaintextOut.SetBufferSize( m_plaintextOut.GetDataSize() + PlaintextReadBlockSize, false );

        api.ERR_clear_error();
        int got = api.SSL_read( m_ssl, m_plaintextOut.GetBufferPtr( m_plaintextOut.GetDataSize() ), (int) PlaintextReadBlockSize );
        if ( got > 0 )
        {
            m_plaintextOut.SetDataSize( m_plaintextOut.GetDataSize() + got );
            continue;
        }

        int sslError = api.SSL_get_error( m_ssl, got );
        if ( OSSL_SSL_ERROR_WANT_READ == sslError || OSSL_SSL_ERROR_WANT_WRITE == sslError )
        {
            api.ERR_clear_error();
            return;
        }
        if ( OSSL_SSL_ERROR_ZERO_RETURN == sslError )
        {
            // close_notify of the peer: the end of the data is a proper one
            m_info.peerClosedCleanly = true;
            m_tlsState = TLSSTATE_CLOSED;
            api.ERR_clear_error();
            return;
        }
        Fail( sslError, "reading" );
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::Advance( void )
{GUCEF_TRACE;

    const SOpenSslApi& api = m_shared->api;

    if ( TLSSTATE_HANDSHAKING == m_tlsState )
    {
        api.ERR_clear_error();
        int result = api.SSL_do_handshake( m_ssl );
        if ( 1 == result )
        {
            OnEstablished();
        }
        else
        {
            int sslError = api.SSL_get_error( m_ssl, result );
            if ( OSSL_SSL_ERROR_WANT_READ == sslError || OSSL_SSL_ERROR_WANT_WRITE == sslError )
            {
                api.ERR_clear_error();
                return;
            }
            Fail( sslError, "the handshake" );
            return;
        }
    }

    if ( TLSSTATE_ESTABLISHED == m_tlsState || TLSSTATE_CLOSING == m_tlsState )
        ReadApplicationData();
}

/*-------------------------------------------------------------------------*/

ETlsState
COpenSslTlsProviderSession::PushCiphertext( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_ssl || ( TLSSTATE_HANDSHAKING != m_tlsState && TLSSTATE_ESTABLISHED != m_tlsState && TLSSTATE_CLOSING != m_tlsState ) )
        return m_tlsState;

    const SOpenSslApi& api = m_shared->api;
    const char* bytes = static_cast< const char* >( data );
    UInt32 offset = 0;
    while ( offset < size )
    {
        int written = api.BIO_write( m_readBio, bytes + offset, (int) ( size - offset ) );
        if ( written <= 0 )
        {
            FailWith( TLSERR_INTERNAL, "the library did not take in the received data" );
            return m_tlsState;
        }
        offset += (UInt32) written;
    }

    Advance();
    return m_tlsState;
}

/*-------------------------------------------------------------------------*/

ETlsState
COpenSslTlsProviderSession::PushPlaintext( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( TLSSTATE_ESTABLISHED != m_tlsState )
        return m_tlsState;

    const SOpenSslApi& api = m_shared->api;
    const char* bytes = static_cast< const char* >( data );
    UInt32 offset = 0;
    while ( offset < size )
    {
        api.ERR_clear_error();
        int written = api.SSL_write( m_ssl, bytes + offset, (int) ( size - offset ) );
        if ( written <= 0 )
        {
            Fail( api.SSL_get_error( m_ssl, written ), "writing" );
            return m_tlsState;
        }
        offset += (UInt32) written;
    }
    return m_tlsState;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProviderSession::PullCiphertext( CORE::CDynamicBuffer& out )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_writeBio )
        return false;

    const SOpenSslApi& api = m_shared->api;
    long pending = api.BIO_ctrl( m_writeBio, OSSL_BIO_CTRL_PENDING, 0, GUCEF_NULL );
    if ( pending <= 0 )
        return false;

    UInt32 dataSize = out.GetDataSize();
    out.SetBufferSize( dataSize + (UInt32) pending, false );
    int got = api.BIO_read( m_writeBio, out.GetBufferPtr( dataSize ), (int) pending );
    if ( got <= 0 )
        return false;
    out.SetDataSize( dataSize + (UInt32) got );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProviderSession::PullPlaintext( CORE::CDynamicBuffer& out )
{GUCEF_TRACE;

    UInt32 available = m_plaintextOut.GetDataSize();
    if ( 0 == available )
        return false;

    out.Append( m_plaintextOut.GetConstBufferPtr(), available );
    m_plaintextOut.SetDataSize( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

ETlsState
COpenSslTlsProviderSession::GetState( void ) const
{GUCEF_TRACE;

    return m_tlsState;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProviderSession::InitiateClose( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_ssl || m_shutdownSent )
        return;

    // Established, or closed by the peer after it was: our close_notify is the answer to theirs
    if ( TLSSTATE_ESTABLISHED == m_tlsState || ( TLSSTATE_CLOSED == m_tlsState && m_wasEstablished ) )
    {
        const SOpenSslApi& api = m_shared->api;
        api.ERR_clear_error();
        api.SSL_shutdown( m_ssl );      // queues the close_notify, the answer of the peer is not waited for here
        api.ERR_clear_error();
        m_shutdownSent = true;
        KeepSessionForResumption();
        if ( TLSSTATE_ESTABLISHED == m_tlsState )
            m_tlsState = TLSSTATE_CLOSING;
    }
    else
    if ( TLSSTATE_HANDSHAKING == m_tlsState || TLSSTATE_IDLE == m_tlsState )
    {
        m_tlsState = TLSSTATE_CLOSED;
    }
}

/*-------------------------------------------------------------------------*/

const CTlsError&
COpenSslTlsProviderSession::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

const CTlsSessionInfo&
COpenSslTlsProviderSession::GetInfo( void ) const
{GUCEF_TRACE;

    return m_info;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
