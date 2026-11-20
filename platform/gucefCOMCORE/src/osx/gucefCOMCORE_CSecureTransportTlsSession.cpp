/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#include <string.h>
#include <time.h>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <Security/SecureTransport.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CSecureTransportTlsSession.h"

// See the provider: Secure Transport is deprecated but shipped, and the only way to filter a byte stream
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

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

struct SSuiteName
{
    UInt16 suite;
    const char* name;
};

/**
 *  The suites that are used: forward secrecy and authenticated encryption first, the rest of what a mail server of
 *  some age still needs after it. Nothing with RC4, 3DES, DES, MD5, export strength, null encryption or anonymous
 *  key exchange. The order is the order of preference.
 */
static const SSuiteName AllowedSuites[] =
{
    { 0xC02C, "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384" },
    { 0xC02B, "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256" },
    { 0xC030, "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384" },
    { 0xC02F, "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256" },
    { 0xCCA9, "TLS_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256" },
    { 0xCCA8, "TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256" },
    { 0x009F, "TLS_DHE_RSA_WITH_AES_256_GCM_SHA384" },
    { 0x009E, "TLS_DHE_RSA_WITH_AES_128_GCM_SHA256" },
    { 0xC024, "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384" },
    { 0xC023, "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256" },
    { 0xC028, "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384" },
    { 0xC027, "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256" },
    { 0xC00A, "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA" },
    { 0xC009, "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA" },
    { 0xC014, "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA" },
    { 0xC013, "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA" },
    { 0x006B, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA256" },
    { 0x0067, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA256" },
    { 0x0039, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA" },
    { 0x0033, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA" },
    { 0x009D, "TLS_RSA_WITH_AES_256_GCM_SHA384" },
    { 0x009C, "TLS_RSA_WITH_AES_128_GCM_SHA256" },
    { 0x003D, "TLS_RSA_WITH_AES_256_CBC_SHA256" },
    { 0x003C, "TLS_RSA_WITH_AES_128_CBC_SHA256" },
    { 0x0035, "TLS_RSA_WITH_AES_256_CBC_SHA" },
    { 0x002F, "TLS_RSA_WITH_AES_128_CBC_SHA" }
};

/*-------------------------------------------------------------------------*/

static CORE::CString
SuiteToName( UInt32 suite )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<sizeof( AllowedSuites ) / sizeof( AllowedSuites[ 0 ] ); ++i )
        if ( AllowedSuites[ i ].suite == suite )
            return AllowedSuites[ i ].name;
    return "TLS_SUITE_0x" + CORE::ToString( suite );
}

/*-------------------------------------------------------------------------*/

static CFStringRef
MakeCFString( const CORE::CString& text )
{GUCEF_TRACE;

    return ::CFStringCreateWithBytes( kCFAllocatorDefault, (const UInt8*) text.C_String(), (CFIndex) text.Length(), kCFStringEncodingUTF8, false );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FromCFString( CFStringRef text )
{GUCEF_TRACE;

    if ( GUCEF_NULL == text )
        return CORE::CString::Empty;
    char buffer[ 1024 ];
    if ( ::CFStringGetCString( text, buffer, (CFIndex) sizeof( buffer ), kCFStringEncodingUTF8 ) )
        return buffer;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

static OSStatus
ReadFunction( SSLConnectionRef connection, void* data, size_t* length )
{
    return (OSStatus) static_cast< CSecureTransportTlsSession* >( const_cast< void* >( connection ) )->OnRead( data, length );
}

/*-------------------------------------------------------------------------*/

static OSStatus
WriteFunction( SSLConnectionRef connection, const void* data, size_t* length )
{
    return (OSStatus) static_cast< CSecureTransportTlsSession* >( const_cast< void* >( connection ) )->OnWrite( data, length );
}

/*-------------------------------------------------------------------------*/

/** The time of a certificate as unix time, false when the certificate does not tell */
static bool
GetCertificateTime( SecCertificateRef cert, bool notAfter, Int64& unixTime )
{GUCEF_TRACE;

    CFStringRef oid = notAfter ? kSecOIDX509V1ValidityNotAfter : kSecOIDX509V1ValidityNotBefore;
    const void* keys[] = { oid };
    CFArrayRef keyList = ::CFArrayCreate( kCFAllocatorDefault, keys, 1, &kCFTypeArrayCallBacks );
    CFDictionaryRef values = ::SecCertificateCopyValues( cert, keyList, GUCEF_NULL );
    ::CFRelease( keyList );
    if ( GUCEF_NULL == values )
        return false;

    bool found = false;
    CFDictionaryRef entry = (CFDictionaryRef) ::CFDictionaryGetValue( values, oid );
    if ( GUCEF_NULL != entry )
    {
        CFTypeRef value = ::CFDictionaryGetValue( entry, kSecPropertyKeyValue );
        double absoluteTime = 0;
        if ( GUCEF_NULL != value && ::CFGetTypeID( value ) == ::CFNumberGetTypeID() &&
             ::CFNumberGetValue( (CFNumberRef) value, kCFNumberDoubleType, &absoluteTime ) )
        {
            unixTime = (Int64) ( absoluteTime + kCFAbsoluteTimeIntervalSince1970 );
            found = true;
        }
    }
    ::CFRelease( values );
    return found;
}

/*-------------------------------------------------------------------------*/

static bool
HasSubjectAltName( SecCertificateRef cert )
{GUCEF_TRACE;

    const void* keys[] = { kSecOIDSubjectAltName };
    CFArrayRef keyList = ::CFArrayCreate( kCFAllocatorDefault, keys, 1, &kCFTypeArrayCallBacks );
    CFDictionaryRef values = ::SecCertificateCopyValues( cert, keyList, GUCEF_NULL );
    ::CFRelease( keyList );
    if ( GUCEF_NULL == values )
        return true;        // not knowing is not a reason to say there is none
    bool has = GUCEF_NULL != ::CFDictionaryGetValue( values, kSecOIDSubjectAltName );
    ::CFRelease( values );
    return has;
}


/*-------------------------------------------------------------------------*/

/** Whether the certificate has an extended key usage extension at all */
static bool
HasExtendedKeyUsage( SecCertificateRef cert )
{GUCEF_TRACE;

    const void* keys[] = { kSecOIDExtendedKeyUsage };
    CFArrayRef keyList = ::CFArrayCreate( kCFAllocatorDefault, keys, 1, &kCFTypeArrayCallBacks );
    CFDictionaryRef values = ::SecCertificateCopyValues( cert, keyList, GUCEF_NULL );
    ::CFRelease( keyList );
    if ( GUCEF_NULL == values )
        return true;        // not knowing is not a reason to relax anything
    bool has = GUCEF_NULL != ::CFDictionaryGetValue( values, kSecOIDExtendedKeyUsage );
    ::CFRelease( values );
    return has;
}

/*-------------------------------------------------------------------------*/

/** A name of a certificate (DNS name, possibly with one wildcard as the whole of the first label) against the name that is asked for */
static bool
DnsNameMatches( const CORE::CString& pattern, const CORE::CString& name )
{GUCEF_TRACE;

    const CORE::CString lowerPattern = pattern.Lowercase();
    const CORE::CString lowerName = name.Lowercase();
    if ( lowerPattern.Length() > 2 && '*' == lowerPattern[ 0 ] && '.' == lowerPattern[ 1 ] )
    {
        // the wildcard is the whole first label, it does not match the empty label or more than one label
        Int32 dot = lowerName.HasChar( '.' );
        if ( dot <= 0 )
            return false;
        return lowerName.CutChars( (UInt32) dot, true ) == lowerPattern.CutChars( 1, true );
    }
    return lowerPattern == lowerName;
}

/** Whether the subject alternative names of the certificate have the name: the common name is never used */
static bool
SubjectAltNamesHave( SecCertificateRef cert, const CORE::CString& name, bool isIpAddress )
{GUCEF_TRACE;

    const void* keys[] = { kSecOIDSubjectAltName };
    CFArrayRef keyList = ::CFArrayCreate( kCFAllocatorDefault, keys, 1, &kCFTypeArrayCallBacks );
    CFDictionaryRef values = ::SecCertificateCopyValues( cert, keyList, GUCEF_NULL );
    ::CFRelease( keyList );
    if ( GUCEF_NULL == values )
        return false;

    bool found = false;
    CFDictionaryRef entry = (CFDictionaryRef) ::CFDictionaryGetValue( values, kSecOIDSubjectAltName );
    CFArrayRef names = GUCEF_NULL != entry ? (CFArrayRef) ::CFDictionaryGetValue( entry, kSecPropertyKeyValue ) : GUCEF_NULL;
    if ( GUCEF_NULL != names && ::CFGetTypeID( names ) == ::CFArrayGetTypeID() )
    {
        for ( CFIndex i=0; i<::CFArrayGetCount( names ) && !found; ++i )
        {
            CFDictionaryRef item = (CFDictionaryRef) ::CFArrayGetValueAtIndex( names, i );
            CFStringRef label = (CFStringRef) ::CFDictionaryGetValue( item, kSecPropertyKeyLabel );
            CFTypeRef value = ::CFDictionaryGetValue( item, kSecPropertyKeyValue );
            if ( GUCEF_NULL == label || GUCEF_NULL == value || ::CFGetTypeID( value ) != ::CFStringGetTypeID() )
                continue;
            const CORE::CString labelText = FromCFString( label );
            const CORE::CString valueText = FromCFString( (CFStringRef) value );
            if ( isIpAddress )
                found = labelText.HasSubstr( "IP", true ) >= 0 && valueText == name;
            else
                found = labelText.HasSubstr( "DNS", true ) >= 0 && DnsNameMatches( valueText, name );
        }
    }
    ::CFRelease( values );
    return found;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
UnixTimeToText( Int64 unixTime )
{GUCEF_TRACE;

    time_t t = (time_t) unixTime;
    struct tm utc;
    ::gmtime_r( &t, &utc );
    char buffer[ 64 ];
    ::strftime( buffer, sizeof( buffer ), "%Y-%m-%d %H:%M:%S UTC", &utc );
    return buffer;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GetSubjectText( SecCertificateRef cert )
{GUCEF_TRACE;

    CFStringRef summary = ::SecCertificateCopySubjectSummary( cert );
    if ( GUCEF_NULL == summary )
        return CORE::CString::Empty;
    CORE::CString text = "/CN=" + FromCFString( summary );
    ::CFRelease( summary );
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSecureTransportTlsSession::CSecureTransportTlsSession( const CTlsSettings& settings    ,
                                                        const CORE::CString& serverName ,
                                                        void* anchors                   ,
                                                        Int64 verifyTimeOverride        )
    : ITlsSession()
    , m_settings( settings )
    , m_serverSettings()
    , m_isServer( false )
    , m_serverName( serverName )
    , m_isIpAddress( IsIpAddressLiteral( serverName ) )
    , m_anchors( anchors )
    , m_chain( GUCEF_NULL )
    , m_verifyTimeOverride( verifyTimeOverride )
    , m_context( GUCEF_NULL )
    , m_ciphertextIn()
    , m_ciphertextInPos( 0 )
    , m_ciphertextOut()
    , m_plaintextOut()
    , m_tlsState( TLSSTATE_IDLE )
    , m_error()
    , m_info()
    , m_wasEstablished( false )
    , m_closeSent( false )
    , m_startTicks( 0 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_anchors )
        ::CFRetain( (CFTypeRef) m_anchors );
    m_info.providerName = "securetransport";
    m_info.providerVersion = "Secure Transport (macOS Security.framework), TLS 1.2";
}

/*-------------------------------------------------------------------------*/

CSecureTransportTlsSession::CSecureTransportTlsSession( const CTlsServerSettings& settings ,
                                                        void* chain                        )
    : ITlsSession()
    , m_settings()
    , m_serverSettings( settings )
    , m_isServer( true )
    , m_serverName()
    , m_isIpAddress( false )
    , m_anchors( GUCEF_NULL )
    , m_chain( chain )
    , m_verifyTimeOverride( 0 )
    , m_context( GUCEF_NULL )
    , m_ciphertextIn()
    , m_ciphertextInPos( 0 )
    , m_ciphertextOut()
    , m_plaintextOut()
    , m_tlsState( TLSSTATE_IDLE )
    , m_error()
    , m_info()
    , m_wasEstablished( false )
    , m_closeSent( false )
    , m_startTicks( 0 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_chain )
        ::CFRetain( (CFTypeRef) m_chain );
    m_info.providerName = "securetransport";
    m_info.providerVersion = "Secure Transport (macOS Security.framework), TLS 1.2";
}

/*-------------------------------------------------------------------------*/

CSecureTransportTlsSession::~CSecureTransportTlsSession()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_context )
    {
        // A session that is closed in an orderly way can be resumed later by the system
        if ( m_wasEstablished && !m_closeSent && TLSSTATE_FAILED != m_tlsState )
            ::SSLClose( (SSLContextRef) m_context );
        ::CFRelease( (CFTypeRef) m_context );
        m_context = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_anchors )
        ::CFRelease( (CFTypeRef) m_anchors );
    if ( GUCEF_NULL != m_chain )
        ::CFRelease( (CFTypeRef) m_chain );
}

/*-------------------------------------------------------------------------*/

Int32
CSecureTransportTlsSession::OnRead( void* data, size_t* length )
{GUCEF_TRACE;

    const UInt32 available = m_ciphertextIn.GetDataSize() - m_ciphertextInPos;
    const size_t wanted = *length;
    if ( 0 == available )
    {
        *length = 0;
        return errSSLWouldBlock;
    }

    const size_t count = wanted < available ? wanted : (size_t) available;
    ::memcpy( data, m_ciphertextIn.GetConstBufferPtr( m_ciphertextInPos ), count );
    m_ciphertextInPos += (UInt32) count;
    if ( m_ciphertextInPos == m_ciphertextIn.GetDataSize() )
    {
        m_ciphertextIn.SetDataSize( 0 );
        m_ciphertextInPos = 0;
    }
    *length = count;
    return count < wanted ? errSSLWouldBlock : noErr;
}

/*-------------------------------------------------------------------------*/

Int32
CSecureTransportTlsSession::OnWrite( const void* data, size_t* length )
{GUCEF_TRACE;

    m_ciphertextOut.Append( data, (UInt32) *length );
    return noErr;
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsSession::Initialize( CTlsError& error )
{GUCEF_TRACE;

    SSLContextRef context = ::SSLCreateContext( kCFAllocatorDefault, m_isServer ? kSSLServerSide : kSSLClientSide, kSSLStreamType );
    if ( GUCEF_NULL == context )
    {
        error = CTlsError( TLSERR_INTERNAL, "cannot create the Secure Transport context" );
        return false;
    }
    m_context = (void*) context;

    OSStatus status = ::SSLSetIOFuncs( context, ReadFunction, WriteFunction );
    if ( noErr == status )
        status = ::SSLSetConnection( context, this );

    // TLS 1.2 is both the floor of the policy and the ceiling of Secure Transport
    if ( noErr == status )
        status = ::SSLSetProtocolVersionMin( context, kTLSProtocol12 );
    if ( noErr == status )
        status = ::SSLSetProtocolVersionMax( context, kTLSProtocol12 );

    if ( noErr == status )
    {
        size_t supportedCount = 0;
        ::SSLGetNumberSupportedCiphers( context, &supportedCount );
        std::vector< SSLCipherSuite > supported( supportedCount );
        if ( supportedCount > 0 )
            ::SSLGetSupportedCiphers( context, &supported[ 0 ], &supportedCount );

        std::vector< SSLCipherSuite > enabled;
        for ( UInt32 i=0; i<sizeof( AllowedSuites ) / sizeof( AllowedSuites[ 0 ] ); ++i )
            for ( size_t n=0; n<supportedCount; ++n )
                if ( supported[ n ] == AllowedSuites[ i ].suite )
                {
                    enabled.push_back( supported[ n ] );
                    break;
                }
        if ( enabled.empty() )
        {
            error = CTlsError( TLSERR_INTERNAL, "Secure Transport offers none of the cipher suites that the policy allows" );
            return false;
        }
        status = ::SSLSetEnabledCiphers( context, &enabled[ 0 ], enabled.size() );
    }

    const CORE::CString::StringVector* alpnList = m_isServer ? &m_serverSettings.GetAlpnProtocols() : &m_settings.GetAlpnProtocols();
    if ( noErr == status && !alpnList->empty() )
    {
        CFMutableArrayRef protocols = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
        for ( CORE::CString::StringVector::const_iterator i = alpnList->begin(); i != alpnList->end(); ++i )
        {
            CFStringRef protocol = MakeCFString( *i );
            ::CFArrayAppendValue( protocols, protocol );
            ::CFRelease( protocol );
        }
        status = ::SSLSetALPNProtocols( context, protocols );
        ::CFRelease( protocols );
    }

    if ( noErr == status && m_isServer )
    {
        status = ::SSLSetCertificate( context, (CFArrayRef) m_chain );
    }
    else
    if ( noErr == status )
    {
        // We evaluate the trust of the certificate of the server ourselves when it arrives
        status = ::SSLSetSessionOption( context, kSSLSessionOptionBreakOnServerAuth, true );

        if ( noErr == status && !m_serverName.IsNULLOrEmpty() && !m_isIpAddress )
            status = ::SSLSetPeerDomainName( context, m_serverName.C_String(), m_serverName.Length() );

        if ( noErr == status && m_settings.GetAllowResumption() && m_settings.GetSessionCacheSize() > 0 )
        {
            // The system resumes a session it knows the id of
            CORE::CString peerId = m_settings.GetContextKey() + "#" + m_serverName;
            status = ::SSLSetPeerID( context, peerId.C_String(), peerId.Length() );
        }
    }

    if ( noErr != status )
    {
        error = CTlsError( TLSERR_INTERNAL, "cannot set up the Secure Transport context: " + DescribeStatus( status ) );
        return false;
    }

    m_plaintextOut.SetBufferSize( PlaintextReadBlockSize, false );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSecureTransportTlsSession::DescribeStatus( Int32 status )
{GUCEF_TRACE;

    CFStringRef text = ::SecCopyErrorMessageString( (OSStatus) status, GUCEF_NULL );
    CORE::CString description = FromCFString( text );
    if ( GUCEF_NULL != text )
        ::CFRelease( text );
    if ( description.IsNULLOrEmpty() )
        description = "OSStatus";
    return description + " (" + CORE::ToString( status ) + ")";
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsSession::Start( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context || TLSSTATE_IDLE != m_tlsState )
        return false;

    m_tlsState = TLSSTATE_HANDSHAKING;
    m_startTicks = MT::PrecisionTickCount();
    Advance();
    return TLSSTATE_FAILED != m_tlsState;
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::FailWith( ETlsErrorClass errorClass, const CORE::CString& message )
{GUCEF_TRACE;

    m_error.errorClass = errorClass;
    m_error.message = message;
    m_tlsState = TLSSTATE_FAILED;
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::FailWithStatus( Int32 status, const char* whileDoing )
{GUCEF_TRACE;

    m_tlsState = TLSSTATE_FAILED;
    m_error.providerCode = status;
    m_error.message = DescribeStatus( status ) + " during " + whileDoing;

    switch ( status )
    {
        case errSSLPeerProtocolVersion:
        case errSSLNegotiation:
        {
            m_error.errorClass = TLSERR_PROTOCOL_VERSION;
            break;
        }
        case errSSLClosedAbort:
        case errSSLClosedGraceful:
        {
            m_error.errorClass = m_wasEstablished ? TLSERR_TRUNCATED : TLSERR_PEER_CLOSED;
            break;
        }
        case errSSLClosedNoNotify:
        {
            m_error.errorClass = TLSERR_TRUNCATED;
            break;
        }
        case errSSLCertExpired:
        case errSSLPeerCertExpired:
        {
            m_error.errorClass = TLSERR_CERT_EXPIRED;
            break;
        }
        case errSSLCertNotYetValid:
        {
            m_error.errorClass = TLSERR_CERT_NOT_YET_VALID;
            break;
        }
        case errSSLHostNameMismatch:
        {
            m_error.errorClass = TLSERR_CERT_HOSTNAME_MISMATCH;
            break;
        }
        case errSSLUnknownRootCert:
        case errSSLNoRootCert:
        case errSSLXCertChainInvalid:
        case errSSLPeerUnknownCA:
        {
            m_error.errorClass = TLSERR_CERT_UNTRUSTED;
            break;
        }
        case errSSLBadCert:
        case errSSLPeerBadCert:
        case errSSLPeerUnsupportedCert:
        case errSSLPeerCertRevoked:
        {
            m_error.errorClass = errSSLPeerCertRevoked == status ? TLSERR_CERT_REVOKED : TLSERR_CERT_OTHER;
            break;
        }
        case errSSLFatalAlert:
        case errSSLPeerHandshakeFail:
        case errSSLPeerAccessDenied:
        case errSSLPeerDecryptError:
        case errSSLPeerInsufficientSecurity:
        case errSSLPeerInternalError:
        case errSSLPeerUserCancelled:
        case errSSLPeerNoRenegotiation:
        case errSSLPeerDecodeError:
        case errSSLPeerExportRestriction:
        case errSSLPeerUnexpectedMsg:
        case errSSLPeerBadRecordMac:
        case errSSLPeerDecryptionFail:
        case errSSLPeerRecordOverflow:
        case errSSLPeerDecompressFail:
        {
            m_error.errorClass = TLSERR_PROTOCOL_ALERT;
            break;
        }
        default:
        {
            m_error.errorClass = m_wasEstablished ? TLSERR_PROTOCOL_ALERT : TLSERR_HANDSHAKE_FAILURE;
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::ClassifyTrustFailure( void* trustRef, const CORE::CString& reason )
{GUCEF_TRACE;

    SecTrustRef trust = (SecTrustRef) trustRef;
    m_tlsState = TLSSTATE_FAILED;
    CORE::CString text = reason;

    SecCertificateRef leaf = ::SecTrustGetCertificateCount( trust ) > 0 ? ::SecTrustGetCertificateAtIndex( trust, 0 ) : GUCEF_NULL;
    if ( GUCEF_NULL != leaf )
        m_error.peerSubject = GetSubjectText( leaf );

    // Which kind of fault it is: first judge the chain and the validity alone, without the name
    bool chainOk = false;
    {
        SecPolicyRef basic = ::SecPolicyCreateBasicX509();
        SecTrustSetPolicies( trust, basic );
        ::CFRelease( basic );
        CFErrorRef basicError = GUCEF_NULL;
        chainOk = ::SecTrustEvaluateWithError( trust, &basicError );
        if ( GUCEF_NULL != basicError )
            ::CFRelease( basicError );
    }

    if ( chainOk )
    {
        m_error.errorClass = ( GUCEF_NULL != leaf && !HasSubjectAltName( leaf ) ) ? TLSERR_CERT_NO_SAN : TLSERR_CERT_HOSTNAME_MISMATCH;
        m_error.message = text;
        return;
    }

    Int64 now = 0 != m_verifyTimeOverride ? m_verifyTimeOverride : (Int64) ::time( GUCEF_NULL );
    Int64 notAfter = 0, notBefore = 0;
    if ( GUCEF_NULL != leaf && GetCertificateTime( leaf, true, notAfter ) && now > notAfter )
    {
        m_error.errorClass = TLSERR_CERT_EXPIRED;
        text += ", expired on " + UnixTimeToText( notAfter );
    }
    else
    if ( GUCEF_NULL != leaf && GetCertificateTime( leaf, false, notBefore ) && now < notBefore )
    {
        m_error.errorClass = TLSERR_CERT_NOT_YET_VALID;
        text += ", valid from " + UnixTimeToText( notBefore );
    }
    else
    {
        m_error.errorClass = TLSERR_CERT_UNTRUSTED;
    }
    m_error.message = text;
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsSession::EvaluateServerTrust( void )
{GUCEF_TRACE;

    if ( !m_settings.IsVerificationEnabled() )
        return true;

    SecTrustRef trust = GUCEF_NULL;
    OSStatus status = ::SSLCopyPeerTrust( (SSLContextRef) m_context, &trust );
    if ( noErr != status || GUCEF_NULL == trust )
    {
        FailWith( TLSERR_HANDSHAKE_FAILURE, "the server did not present a certificate" );
        return false;
    }

    // The name has to be in the subject alternative names: the common name is never used
    CFStringRef host = m_serverName.IsNULLOrEmpty() ? GUCEF_NULL : MakeCFString( m_serverName );
    SecPolicyRef policy = ::SecPolicyCreateSSL( true, host );
    if ( GUCEF_NULL != host )
        ::CFRelease( host );
    ::SecTrustSetPolicies( trust, policy );
    ::CFRelease( policy );

    if ( GUCEF_NULL != m_anchors )
    {
        ::SecTrustSetAnchorCertificates( trust, (CFArrayRef) m_anchors );
        ::SecTrustSetAnchorCertificatesOnly( trust, !m_settings.GetUseSystemTrust() );
    }
    else
    if ( !m_settings.GetUseSystemTrust() )
    {
        // Nothing can be trusted, which fails below
        CFArrayRef none = ::CFArrayCreate( kCFAllocatorDefault, GUCEF_NULL, 0, &kCFTypeArrayCallBacks );
        ::SecTrustSetAnchorCertificates( trust, none );
        ::SecTrustSetAnchorCertificatesOnly( trust, true );
        ::CFRelease( none );
    }

    if ( 0 != m_verifyTimeOverride )
    {
        CFDateRef date = ::CFDateCreate( kCFAllocatorDefault, (CFAbsoluteTime) m_verifyTimeOverride - kCFAbsoluteTimeIntervalSince1970 );
        ::SecTrustSetVerifyDate( trust, date );
        ::CFRelease( date );
    }

    CFErrorRef evaluationError = GUCEF_NULL;
    bool trusted = ::SecTrustEvaluateWithError( trust, &evaluationError );

    // The policy of the system for TLS servers has a requirement that is not one of TLS: the certificate must say "serverAuth" in an
    // extended key usage extension. A certificate that has none (private authorities, equipment) is what OpenSSL accepts as well, and
    // the rest of the policy is the one of any other certificate. Only for a certificate which has no such extension at all, the chain
    // is verified without the TLS policy and the name is matched with the subject alternative names here
    if ( !trusted && ::SecTrustGetCertificateCount( trust ) > 0 && !HasExtendedKeyUsage( ::SecTrustGetCertificateAtIndex( trust, 0 ) ) )
    {
        SecCertificateRef leaf = ::SecTrustGetCertificateAtIndex( trust, 0 );
        SecPolicyRef basic = ::SecPolicyCreateBasicX509();
        ::SecTrustSetPolicies( trust, basic );
        ::CFRelease( basic );
        CFErrorRef basicError = GUCEF_NULL;
        const bool chainTrusted = ::SecTrustEvaluateWithError( trust, &basicError );
        if ( GUCEF_NULL != basicError )
            ::CFRelease( basicError );
        if ( chainTrusted && ( m_serverName.IsNULLOrEmpty() || SubjectAltNamesHave( leaf, m_serverName, m_isIpAddress ) ) )
        {
            if ( GUCEF_NULL != evaluationError )
            {
                ::CFRelease( evaluationError );
                evaluationError = GUCEF_NULL;
            }
            trusted = true;
        }
    }
    if ( trusted )
    {
        SecCertificateRef leaf = ::SecTrustGetCertificateCount( trust ) > 0 ? ::SecTrustGetCertificateAtIndex( trust, 0 ) : GUCEF_NULL;
        if ( GUCEF_NULL != leaf )
            m_info.peerSubject = GetSubjectText( leaf );
        ::CFRelease( trust );
        return true;
    }

    CORE::CString reason = "certificate verification failed";
    if ( GUCEF_NULL != evaluationError )
    {
        CFStringRef description = ::CFErrorCopyDescription( evaluationError );
        CORE::CString text = FromCFString( description );
        if ( GUCEF_NULL != description )
            ::CFRelease( description );
        if ( !text.IsNULLOrEmpty() )
            reason = text;
        m_error.verifyCode = (Int32) ::CFErrorGetCode( evaluationError );
        ::CFRelease( evaluationError );
    }
    ClassifyTrustFailure( trust, reason );
    ::CFRelease( trust );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::OnEstablished( void )
{GUCEF_TRACE;

    SSLContextRef context = (SSLContextRef) m_context;
    m_wasEstablished = true;
    m_tlsState = TLSSTATE_ESTABLISHED;

    SSLProtocol protocol = kSSLProtocolUnknown;
    ::SSLGetNegotiatedProtocolVersion( context, &protocol );
    m_info.protocolVersion = kTLSProtocol12 == protocol ? "TLSv1.2" : "TLS";

    SSLCipherSuite cipher = 0;
    if ( noErr == ::SSLGetNegotiatedCipher( context, &cipher ) )
        m_info.cipherName = SuiteToName( (UInt32) cipher );

    CFArrayRef alpn = GUCEF_NULL;
    if ( noErr == ::SSLCopyALPNProtocols( context, &alpn ) && GUCEF_NULL != alpn )
    {
        if ( ::CFArrayGetCount( alpn ) > 0 )
            m_info.alpnProtocol = FromCFString( (CFStringRef) ::CFArrayGetValueAtIndex( alpn, 0 ) );
        ::CFRelease( alpn );
    }

    if ( m_info.peerSubject.IsNULLOrEmpty() )
    {
        SecTrustRef trust = GUCEF_NULL;
        if ( noErr == ::SSLCopyPeerTrust( context, &trust ) && GUCEF_NULL != trust )
        {
            if ( ::SecTrustGetCertificateCount( trust ) > 0 )
                m_info.peerSubject = GetSubjectText( ::SecTrustGetCertificateAtIndex( trust, 0 ) );
            ::CFRelease( trust );
        }
    }

    UInt64 ticksPerSecond = MT::PrecisionTimerResolution();
    if ( 0 != ticksPerSecond )
        m_info.handshakeDurationInMs = (UInt32) ( ( MT::PrecisionTickCount() - m_startTicks ) * 1000 / ticksPerSecond );
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::ReadApplicationData( void )
{GUCEF_TRACE;

    SSLContextRef context = (SSLContextRef) m_context;
    while ( TLSSTATE_ESTABLISHED == m_tlsState || TLSSTATE_CLOSING == m_tlsState )
    {
        if ( m_plaintextOut.GetUnusedBufferSize() < PlaintextReadBlockSize )
            m_plaintextOut.SetBufferSize( m_plaintextOut.GetDataSize() + PlaintextReadBlockSize, false );

        size_t processed = 0;
        OSStatus status = ::SSLRead( context, m_plaintextOut.GetBufferPtr( m_plaintextOut.GetDataSize() ), PlaintextReadBlockSize, &processed );
        if ( processed > 0 )
            m_plaintextOut.SetDataSize( m_plaintextOut.GetDataSize() + (UInt32) processed );

        if ( noErr == status )
        {
            if ( 0 == processed )
                return;
            continue;
        }
        if ( errSSLWouldBlock == status )
            return;
        if ( errSSLClosedGraceful == status )
        {
            // close_notify of the peer: the end of the data is a proper one
            m_info.peerClosedCleanly = true;
            m_tlsState = TLSSTATE_CLOSED;
            return;
        }
        FailWithStatus( status, "reading" );
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::Advance( void )
{GUCEF_TRACE;

    SSLContextRef context = (SSLContextRef) m_context;

    while ( TLSSTATE_HANDSHAKING == m_tlsState )
    {
        OSStatus status = ::SSLHandshake( context );
        if ( noErr == status )
        {
            OnEstablished();
            break;
        }
        if ( errSSLWouldBlock == status )
            return;
        if ( errSSLServerAuthCompleted == status )
        {
            if ( !EvaluateServerTrust() )
            {
                // Tell the server why, then stop
                ::SSLClose( context );
                m_closeSent = true;
                return;
            }
            continue;
        }
        if ( errSSLClientCertRequested == status )
            continue;       // there is no certificate of ours to give: carry on without

        FailWithStatus( status, "the handshake" );
        return;
    }

    if ( TLSSTATE_ESTABLISHED == m_tlsState || TLSSTATE_CLOSING == m_tlsState )
        ReadApplicationData();
}

/*-------------------------------------------------------------------------*/

ETlsState
CSecureTransportTlsSession::PushCiphertext( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context || ( TLSSTATE_HANDSHAKING != m_tlsState && TLSSTATE_ESTABLISHED != m_tlsState && TLSSTATE_CLOSING != m_tlsState ) )
        return m_tlsState;

    m_ciphertextIn.Append( data, size );
    Advance();
    return m_tlsState;
}

/*-------------------------------------------------------------------------*/

ETlsState
CSecureTransportTlsSession::PushPlaintext( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( TLSSTATE_ESTABLISHED != m_tlsState )
        return m_tlsState;

    const char* bytes = static_cast< const char* >( data );
    UInt32 offset = 0;
    while ( offset < size )
    {
        size_t processed = 0;
        OSStatus status = ::SSLWrite( (SSLContextRef) m_context, bytes + offset, size - offset, &processed );
        offset += (UInt32) processed;
        if ( noErr != status && errSSLWouldBlock != status )
        {
            FailWithStatus( status, "writing" );
            return m_tlsState;
        }
        if ( 0 == processed )
        {
            FailWith( TLSERR_INTERNAL, "Secure Transport did not take in the data to send" );
            return m_tlsState;
        }
    }
    return m_tlsState;
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsSession::PullCiphertext( CORE::CDynamicBuffer& out )
{GUCEF_TRACE;

    UInt32 available = m_ciphertextOut.GetDataSize();
    if ( 0 == available )
        return false;
    out.Append( m_ciphertextOut.GetConstBufferPtr(), available );
    m_ciphertextOut.SetDataSize( 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsSession::PullPlaintext( CORE::CDynamicBuffer& out )
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
CSecureTransportTlsSession::GetState( void ) const
{GUCEF_TRACE;

    return m_tlsState;
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsSession::InitiateClose( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_context || m_closeSent )
        return;

    // Established, or closed by the peer after it was: our close_notify is the answer to theirs
    if ( TLSSTATE_ESTABLISHED == m_tlsState || ( TLSSTATE_CLOSED == m_tlsState && m_wasEstablished ) )
    {
        ::SSLClose( (SSLContextRef) m_context );    // queues the close_notify, the answer of the peer is not waited for here
        m_closeSent = true;
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
CSecureTransportTlsSession::GetError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

const CTlsSessionInfo&
CSecureTransportTlsSession::GetInfo( void ) const
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

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */
