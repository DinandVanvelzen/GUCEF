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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_COpenSslTlsProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString COpenSslTlsProvider::ProviderName = "openssl-dynamic";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

COpenSslTlsProvider::COpenSslTlsProvider( void )
    : ITlsProvider()
    , m_candidates()
    , m_shared()
    , m_probed( false )
    , m_available( false )
    , m_whyNot()
    , m_lock()
{GUCEF_TRACE;

    GetDefaultLibraryCandidates( m_candidates );
}

/*-------------------------------------------------------------------------*/

COpenSslTlsProvider::COpenSslTlsProvider( const TLibraryCandidateVector& candidates )
    : ITlsProvider()
    , m_candidates( candidates )
    , m_shared()
    , m_probed( false )
    , m_available( false )
    , m_whyNot()
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

COpenSslTlsProvider::~COpenSslTlsProvider()
{GUCEF_TRACE;

    // The libraries stay loaded, see the class description
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProvider::GetDefaultLibraryCandidates( TLibraryCandidateVector& candidates )
{GUCEF_TRACE;

    candidates.clear();

    SLibraryCandidate v3;
    v3.sslName = "libssl.so.3";
    v3.cryptoName = "libcrypto.so.3";
    candidates.push_back( v3 );

    SLibraryCandidate v11;
    v11.sslName = "libssl.so.1.1";
    v11.cryptoName = "libcrypto.so.1.1";
    candidates.push_back( v11 );

    SLibraryCandidate any;
    any.sslName = "libssl.so";
    any.cryptoName = "libcrypto.so";
    candidates.push_back( any );
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProvider::CheckApi( const SOpenSslApi& api, CORE::CString& whyNot )
{GUCEF_TRACE;

    if ( GUCEF_NULL == api.OpenSSL_version_num )
    {
        whyNot = "the version of the library cannot be determined";
        return false;
    }

    unsigned long version = api.OpenSSL_version_num();
    if ( version < OSSL_MIN_VERSION_NUMBER )
    {
        CORE::CString text = GUCEF_NULL != api.OpenSSL_version ? CORE::CString( api.OpenSSL_version( OSSL_OPENSSL_VERSION ) ) : CORE::CString( "an old version" );
        whyNot = "the OpenSSL of this system (" + text + ") is too old, 1.1.1 or newer is needed for TLS 1.3 and the interface used";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProvider::TryLoad( const SLibraryCandidate& candidate, CORE::CString& whyNot ) const
{GUCEF_TRACE;

    // libcrypto first: libssl needs it, and the entry points come from the handles and never from the global namespace
    CORE::CString reason;
    void* cryptoLib = CORE::LoadModuleDynamicly( candidate.cryptoName, reason );
    if ( GUCEF_NULL == cryptoLib )
    {
        whyNot = reason.IsNULLOrEmpty() ? candidate.cryptoName + ": cannot be loaded" : reason;
        return false;
    }
    void* sslLib = CORE::LoadModuleDynamicly( candidate.sslName, reason );
    if ( GUCEF_NULL == sslLib )
    {
        whyNot = reason.IsNULLOrEmpty() ? candidate.sslName + ": cannot be loaded" : reason;
        CORE::UnloadModuleDynamicly( cryptoLib );
        return false;
    }

    TOpenSslStatePtr shared( GUCEF_NEW COpenSslTlsProviderState() );
    CORE::CString missing;
    if ( !shared->api.Resolve( sslLib, cryptoLib, missing ) )
    {
        // the first names are enough to tell what kind of library this is
        CORE::CString::StringVector names = missing.ParseElements( ',', false );
        CORE::CString shown;
        for ( size_t i=0; i<names.size() && i<4; ++i )
            shown += ( i > 0 ? ", " : "" ) + names[ i ].Trim( true ).Trim( false );
        if ( names.size() > 4 )
            shown += " and " + CORE::ToString( (UInt32) names.size() - 4 ) + " more";
        whyNot = candidate.sslName + ": the library lacks " + shown;
        CORE::UnloadModuleDynamicly( sslLib );
        CORE::UnloadModuleDynamicly( cryptoLib );
        return false;
    }

    if ( !CheckApi( shared->api, whyNot ) )
    {
        whyNot = candidate.sslName + ": " + whyNot;
        CORE::UnloadModuleDynamicly( sslLib );
        CORE::UnloadModuleDynamicly( cryptoLib );
        return false;
    }

    // The library found must be the one the name stands for. A program that exports its own OpenSSL can capture the symbols
    // libssl needs from libcrypto; the major version reported by the library tells when the two do not belong together
    unsigned long version = shared->api.versionNumber;
    bool expectV3 = candidate.sslName.HasSubstr( ".so.3" ) >= 0;
    bool expectV11 = candidate.sslName.HasSubstr( ".so.1.1" ) >= 0;
    if ( ( expectV3 && ( version >> 28 ) != 3 ) || ( expectV11 && ( ( version >> 28 ) != 1 || ( ( version >> 20 ) & 0xFF ) != 1 ) ) )
    {
        whyNot = candidate.sslName + ": the library reports OpenSSL " + CORE::ToString( (UInt32) ( version >> 28 ) ) + ", not the version its name stands for";
        CORE::UnloadModuleDynamicly( sslLib );
        CORE::UnloadModuleDynamicly( cryptoLib );
        return false;
    }

    shared->api.OPENSSL_init_ssl( 0, GUCEF_NULL );
    shared->versionInfo = shared->api.OpenSSL_version( OSSL_OPENSSL_VERSION );
    shared->providerName = ProviderName;
    m_shared = shared;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProvider::Probe( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_probed )
        return m_available;
    m_probed = true;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

    CORE::CString reasons;
    for ( TLibraryCandidateVector::const_iterator i = m_candidates.begin(); i != m_candidates.end(); ++i )
    {
        CORE::CString whyNot;
        if ( TryLoad( (*i), whyNot ) )
        {
            m_available = true;
            m_whyNot.Clear();
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OpenSslTlsProvider: Using " << m_shared->versionInfo << " through " << (*i).sslName );
            return true;
        }
        if ( !reasons.IsNULLOrEmpty() )
            reasons += "; ";
        reasons += whyNot;
    }

    m_available = false;
    m_whyNot = reasons.IsNULLOrEmpty() ? CORE::CString( "no OpenSSL library is known to look for" ) : reasons;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "OpenSslTlsProvider: Not available: " << m_whyNot );
    return false;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_WASM_EMSCRIPTEN )

    m_available = false;
    m_whyNot = "the browser provides TLS through fetch and WebSocket, there is no TLS layer under the sockets here";
    return false;

    #else

    m_available = false;
    m_whyNot = "the OpenSSL provider is only used on Linux, the TLS of this platform is not available yet";
    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

bool
COpenSslTlsProvider::IsAvailable( CORE::CString& whyNot ) const
{GUCEF_TRACE;

    bool available = Probe();
    MT::CScopeMutex lock( m_lock );
    whyNot = m_whyNot;
    return available;
}

/*-------------------------------------------------------------------------*/

UInt32
COpenSslTlsProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    UInt32 caps = TLSCAP_ALPN | TLSCAP_RESUMPTION | TLSCAP_CUSTOM_TRUST;
    if ( Probe() )
        caps |= TLSCAP_TLS13;
    return caps;
}

/*-------------------------------------------------------------------------*/

CORE::CString
COpenSslTlsProvider::GetVersionInfo( void ) const
{GUCEF_TRACE;

    if ( !Probe() )
        return CORE::CString::Empty;
    return m_shared->versionInfo;
}

/*-------------------------------------------------------------------------*/

TTlsSessionPtr
COpenSslTlsProvider::CreateClientSession( const CTlsSettings& settings    ,
                                          const CORE::CString& serverName ,
                                          CTlsError& error                )
{GUCEF_TRACE;

    error.Clear();
    if ( !Probe() )
    {
        CORE::CString whyNot;
        IsAvailable( whyNot );
        error = CTlsError( TLSERR_NO_PROVIDER, whyNot );
        return TTlsSessionPtr();
    }

    COpenSslTlsProviderSession* session = GUCEF_NEW COpenSslTlsProviderSession( m_shared, settings, serverName );
    TTlsSessionPtr sessionPtr( session );
    if ( !session->Initialize( error ) )
        return TTlsSessionPtr();
    return sessionPtr;
}

/*-------------------------------------------------------------------------*/

void
COpenSslTlsProvider::SetVerificationTimeForTesting( Int64 unixTime )
{GUCEF_TRACE;

    if ( Probe() )
        m_shared->verifyTimeOverride = unixTime;
}

/*-------------------------------------------------------------------------*/

UInt32
COpenSslTlsProvider::GetContextCount( void ) const
{GUCEF_TRACE;

    return Probe() ? m_shared->GetContextCount() : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
COpenSslTlsProvider::GetResumableSessionCount( void ) const
{GUCEF_TRACE;

    return Probe() ? m_shared->GetResumableSessionCount() : 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
