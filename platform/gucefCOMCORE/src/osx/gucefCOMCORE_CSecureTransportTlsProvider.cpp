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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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

#include "gucefCOMCORE_CSecureTransportTlsSession.h"

#include "gucefCOMCORE_CSecureTransportTlsProvider.h"

// Secure Transport is deprecated by Apple in favour of Network.framework, which cannot be used as a filter over a
// byte stream and has no equivalent of what these sessions need. It is still shipped and supported by the system.
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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CSecureTransportTlsProvider::ProviderName = "securetransport";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
ReadWholeFile( const CORE::CString& path, std::string& content )
{GUCEF_TRACE;

    content.clear();
    FILE* file = ::fopen( path.C_String(), "rb" );
    if ( GUCEF_NULL == file )
        return false;
    char buffer[ 8192 ];
    size_t got = 0;
    while ( ( got = ::fread( buffer, 1, sizeof( buffer ), file ) ) > 0 )
        content.append( buffer, got );
    ::fclose( file );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Appends the certificates of PEM text to the array. @return whether the text held at least one */
static bool
AppendPemCertificates( const char* pem, size_t length, CFMutableArrayRef out )
{GUCEF_TRACE;

    CFDataRef data = ::CFDataCreate( kCFAllocatorDefault, (const UInt8*) pem, (CFIndex) length );
    if ( GUCEF_NULL == data )
        return false;

    SecExternalFormat format = kSecFormatPEMSequence;
    SecExternalItemType itemType = kSecItemTypeCertificate;
    CFArrayRef items = GUCEF_NULL;
    OSStatus status = ::SecItemImport( data, GUCEF_NULL, &format, &itemType, 0, GUCEF_NULL, GUCEF_NULL, &items );
    ::CFRelease( data );
    if ( noErr != status || GUCEF_NULL == items )
        return false;

    bool any = false;
    for ( CFIndex i=0; i<::CFArrayGetCount( items ); ++i )
    {
        CFTypeRef item = ::CFArrayGetValueAtIndex( items, i );
        if ( ::CFGetTypeID( item ) == ::SecCertificateGetTypeID() )
        {
            ::CFArrayAppendValue( out, item );
            any = true;
        }
    }
    ::CFRelease( items );
    return any;
}


/*-------------------------------------------------------------------------*/

/**
 *  The system imports a PKCS#8 private key (BEGIN PRIVATE KEY) when it is an RSA key, but not when it is an EC key,
 *  which is what OpenSSL 3 writes for every kind of key. An EC key is imported in the form of SEC1 (BEGIN EC PRIVATE KEY),
 *  so the key inside the PKCS#8 wrapping is taken out and written in that form, with the curve it belongs to.
 */
static bool
ReadDerLength( const std::string& der, size_t& pos, size_t& length )
{
    if ( pos >= der.size() )
        return false;
    unsigned char first = (unsigned char) der[ pos++ ];
    if ( first < 0x80 )
    {
        length = first;
        return true;
    }
    size_t count = first & 0x7F;
    if ( 0 == count || count > 4 || pos + count > der.size() )
        return false;
    length = 0;
    for ( size_t i=0; i<count; ++i )
        length = ( length << 8 ) | (unsigned char) der[ pos++ ];
    return true;
}

static void
AppendDerLength( std::string& out, size_t length )
{
    if ( length < 0x80 )
        out.push_back( (char) length );
    else
    if ( length < 0x100 )
    {
        out.push_back( (char) 0x81 );
        out.push_back( (char) length );
    }
    else
    {
        out.push_back( (char) 0x82 );
        out.push_back( (char) ( length >> 8 ) );
        out.push_back( (char) ( length & 0xFF ) );
    }
}

static bool
Base64Decode( const std::string& text, std::string& out )
{
    out.clear();
    unsigned int accumulator = 0;
    int bits = 0;
    for ( size_t i=0; i<text.size(); ++i )
    {
        char c = text[ i ];
        int value = -1;
        if ( c >= 'A' && c <= 'Z' ) value = c - 'A';
        else if ( c >= 'a' && c <= 'z' ) value = c - 'a' + 26;
        else if ( c >= '0' && c <= '9' ) value = c - '0' + 52;
        else if ( '+' == c ) value = 62;
        else if ( '/' == c ) value = 63;
        else if ( '=' == c ) break;
        else continue;     // line breaks and the like
        accumulator = ( accumulator << 6 ) | (unsigned int) value;
        bits += 6;
        if ( bits >= 8 )
        {
            bits -= 8;
            out.push_back( (char) ( ( accumulator >> bits ) & 0xFF ) );
        }
    }
    return !out.empty();
}

static std::string
Base64EncodeLines( const std::string& data )
{
    static const char* table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    size_t lineLength = 0;
    for ( size_t i=0; i<data.size(); i+=3 )
    {
        unsigned int chunk = (unsigned char) data[ i ] << 16;
        if ( i+1 < data.size() ) chunk |= (unsigned char) data[ i+1 ] << 8;
        if ( i+2 < data.size() ) chunk |= (unsigned char) data[ i+2 ];
        out.push_back( table[ ( chunk >> 18 ) & 63 ] );
        out.push_back( table[ ( chunk >> 12 ) & 63 ] );
        out.push_back( i+1 < data.size() ? table[ ( chunk >> 6 ) & 63 ] : '=' );
        out.push_back( i+2 < data.size() ? table[ chunk & 63 ] : '=' );
        lineLength += 4;
        if ( lineLength >= 64 )
        {
            out.push_back( '\n' );
            lineLength = 0;
        }
    }
    if ( lineLength > 0 )
        out.push_back( '\n' );
    return out;
}

/** @return false when the text is not a PKCS#8 key with an EC key inside, sec1Pem is then not touched */
static bool
ConvertPkcs8EcKeyToSec1Pem( const std::string& pem, std::string& sec1Pem )
{GUCEF_TRACE;

    const char* begin = "-----BEGIN PRIVATE KEY-----";
    const char* end = "-----END PRIVATE KEY-----";
    size_t start = pem.find( begin );
    size_t stop = pem.find( end );
    if ( std::string::npos == start || std::string::npos == stop || stop < start )
        return false;

    std::string der;
    if ( !Base64Decode( pem.substr( start + ::strlen( begin ), stop - start - ::strlen( begin ) ), der ) )
        return false;

    // PrivateKeyInfo ::= SEQUENCE { INTEGER version, SEQUENCE { OID algorithm, OID curve }, OCTET STRING privateKey }
    size_t pos = 0, length = 0;
    if ( der.empty() || 0x30 != (unsigned char) der[ pos++ ] || !ReadDerLength( der, pos, length ) )
        return false;
    if ( pos + 3 > der.size() || 0x02 != (unsigned char) der[ pos ] )
        return false;
    pos += 2 + (unsigned char) der[ pos + 1 ];      // the version
    if ( pos >= der.size() || 0x30 != (unsigned char) der[ pos++ ] || !ReadDerLength( der, pos, length ) )
        return false;
    const size_t algorithmEnd = pos + length;
    if ( algorithmEnd > der.size() )
        return false;

    // id-ecPublicKey: 1.2.840.10045.2.1
    static const unsigned char ecPublicKeyOid[] = { 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01 };
    if ( pos + sizeof( ecPublicKeyOid ) > algorithmEnd || 0 != ::memcmp( der.data() + pos, ecPublicKeyOid, sizeof( ecPublicKeyOid ) ) )
        return false;
    pos += sizeof( ecPublicKeyOid );

    // the curve: named by an OID, or given in full (what LibreSSL writes), in which case it is recognised by its prime field
    if ( pos >= algorithmEnd )
        return false;
    std::string curveOid;
    if ( 0x06 == (unsigned char) der[ pos ] )
    {
        size_t curveLength = (unsigned char) der[ pos + 1 ];
        if ( pos + 2 + curveLength > algorithmEnd )
            return false;
        curveOid = der.substr( pos, 2 + curveLength );
    }
    else
    if ( 0x30 == (unsigned char) der[ pos ] )
    {
        // ECParameters ::= SEQUENCE { INTEGER version, SEQUENCE { OID prime-field, INTEGER prime }, ... }
        size_t ppos = pos + 1, plength = 0;
        if ( !ReadDerLength( der, ppos, plength ) || ppos + plength > algorithmEnd || 0x02 != (unsigned char) der[ ppos ] )
            return false;
        ppos += 2 + (unsigned char) der[ ppos + 1 ];                         // the version
        if ( ppos >= algorithmEnd || 0x30 != (unsigned char) der[ ppos++ ] || !ReadDerLength( der, ppos, plength ) )
            return false;
        if ( ppos >= algorithmEnd || 0x06 != (unsigned char) der[ ppos ] )
            return false;
        ppos += 2 + (unsigned char) der[ ppos + 1 ];                         // the OID of the field type
        if ( ppos + 2 >= algorithmEnd || 0x02 != (unsigned char) der[ ppos++ ] || !ReadDerLength( der, ppos, plength ) || ppos + plength > algorithmEnd )
            return false;
        std::string prime = der.substr( ppos, plength );
        if ( !prime.empty() && 0 == prime[ 0 ] )
            prime.erase( 0, 1 );

        static const unsigned char p256Prefix[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01 };
        static const unsigned char oidP256[] = { 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07 };
        static const unsigned char oidP384[] = { 0x06, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x22 };
        static const unsigned char oidP521[] = { 0x06, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x23 };
        if ( 32 == prime.size() && 0 == ::memcmp( prime.data(), p256Prefix, sizeof( p256Prefix ) ) )
            curveOid.assign( (const char*) oidP256, sizeof( oidP256 ) );
        else
        if ( 48 == prime.size() && (unsigned char) prime[ 0 ] == 0xFF && (unsigned char) prime[ 15 ] == 0xFF && (unsigned char) prime[ 47 ] == 0xFF )
            curveOid.assign( (const char*) oidP384, sizeof( oidP384 ) );
        else
        if ( 66 == prime.size() && 0x01 == (unsigned char) prime[ 0 ] && (unsigned char) prime[ 65 ] == 0xFF )
            curveOid.assign( (const char*) oidP521, sizeof( oidP521 ) );
        else
            return false;       // a curve that is not a standard one: left to the system to refuse
    }
    else
    {
        return false;
    }
    pos = algorithmEnd;

    // the ECPrivateKey that is in the octet string
    if ( pos >= der.size() || 0x04 != (unsigned char) der[ pos++ ] || !ReadDerLength( der, pos, length ) || pos + length > der.size() )
        return false;
    std::string inner = der.substr( pos, length );

    size_t ipos = 0, ilength = 0;
    if ( inner.empty() || 0x30 != (unsigned char) inner[ ipos++ ] || !ReadDerLength( inner, ipos, ilength ) )
        return false;
    std::string body = inner.substr( ipos, ilength );

    // version and the key itself stay, the parameters are put in when they are not there
    size_t bpos = 0;
    if ( body.size() < 3 || 0x02 != (unsigned char) body[ 0 ] )
        return false;
    bpos = 2 + (unsigned char) body[ 1 ];
    if ( bpos >= body.size() || 0x04 != (unsigned char) body[ bpos ] )
        return false;
    size_t keyLength = 0, kpos = bpos + 1;
    if ( !ReadDerLength( body, kpos, keyLength ) || kpos + keyLength > body.size() )
        return false;
    size_t afterKey = kpos + keyLength;

    // the parameters of the key are those of the curve by name, wherever they were and however they were written
    std::string newBody = body.substr( 0, afterKey );
    size_t rest = afterKey;
    if ( rest < body.size() && 0xA0 == (unsigned char) body[ rest ] )
    {
        size_t apos = rest + 1, alength = 0;
        if ( !ReadDerLength( body, apos, alength ) || apos + alength > body.size() )
            return false;
        rest = apos + alength;
    }
    newBody.push_back( (char) 0xA0 );
    AppendDerLength( newBody, curveOid.size() );
    newBody += curveOid;
    newBody += body.substr( rest );             // the public key, when it is there

    std::string sec1;
    sec1.push_back( (char) 0x30 );
    AppendDerLength( sec1, newBody.size() );
    sec1 += newBody;

    sec1Pem = "-----BEGIN EC PRIVATE KEY-----\n" + Base64EncodeLines( sec1 ) + "-----END EC PRIVATE KEY-----\n";
    return true;
}


/*-------------------------------------------------------------------------*/

/** The label that identifies a key by its public part: the same for a private key and for the public key in a certificate */
static CFDataRef
CopyKeyLabel( SecKeyRef key )
{GUCEF_TRACE;

    if ( GUCEF_NULL == key )
        return GUCEF_NULL;
    CFDictionaryRef attributes = ::SecKeyCopyAttributes( key );
    if ( GUCEF_NULL == attributes )
        return GUCEF_NULL;
    CFDataRef label = (CFDataRef) ::CFDictionaryGetValue( attributes, kSecAttrApplicationLabel );
    if ( GUCEF_NULL != label )
        ::CFRetain( label );
    ::CFRelease( attributes );
    return label;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
GetFileStamp( const CORE::CString& path )
{GUCEF_TRACE;

    return CORE::GetFileModificationTime( path ).ToIso8601DateTimeString( false, true ) + "/" + CORE::ToString( CORE::FileSize( path ) );
}

/*-------------------------------------------------------------------------*/

/** The CA certificates the settings add to the trust of the system. NULL when there are none, a retained array otherwise */
static CFArrayRef
LoadAnchors( const CTlsSettings& settings, CTlsError& error, bool& haveAny )
{GUCEF_TRACE;

    haveAny = false;
    CFMutableArrayRef anchors = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );

    const CORE::CString& caFile = settings.GetCaBundlePath();
    if ( !caFile.IsNULLOrEmpty() )
    {
        // A trust anchor that was asked for and cannot be read is an error: it must never turn into "trust nothing was asked" or "skip verification"
        std::string pem;
        if ( !ReadWholeFile( caFile, pem ) || !AppendPemCertificates( pem.data(), pem.size(), anchors ) )
        {
            error = CTlsError( TLSERR_SETTINGS, "cannot load the CA file \"" + caFile + "\"" );
            ::CFRelease( anchors );
            return GUCEF_NULL;
        }
        haveAny = true;
    }

    const CORE::CString& caDir = settings.GetCaDir();
    if ( !caDir.IsNULLOrEmpty() )
    {
        DIR* dir = ::opendir( caDir.C_String() );
        if ( GUCEF_NULL == dir )
        {
            error = CTlsError( TLSERR_SETTINGS, "cannot read the CA directory \"" + caDir + "\"" );
            ::CFRelease( anchors );
            return GUCEF_NULL;
        }
        struct dirent* entry = GUCEF_NULL;
        while ( GUCEF_NULL != ( entry = ::readdir( dir ) ) )
        {
            if ( '.' == entry->d_name[ 0 ] )
                continue;
            std::string pem;
            CORE::CString path = CORE::CombinePath( caDir, entry->d_name );
            if ( ReadWholeFile( path, pem ) && AppendPemCertificates( pem.data(), pem.size(), anchors ) )
                haveAny = true;
        }
        ::closedir( dir );
        if ( !haveAny )
        {
            error = CTlsError( TLSERR_SETTINGS, "the CA directory \"" + caDir + "\" holds no certificates" );
            ::CFRelease( anchors );
            return GUCEF_NULL;
        }
    }

    const CORE::CString& caData = settings.GetCaBundleData();
    if ( !caData.IsNULLOrEmpty() )
    {
        if ( !AppendPemCertificates( caData.C_String(), caData.Length(), anchors ) )
        {
            error = CTlsError( TLSERR_SETTINGS, "the CA certificates given as data cannot be read" );
            ::CFRelease( anchors );
            return GUCEF_NULL;
        }
        haveAny = true;
    }

    // The trust that the system is configured with: like with OpenSSL, the files and the directory that the environment names are trusted
    // as well, they come on top of the keychains of the system
    if ( settings.GetUseSystemTrust() )
    {
        const char* envFile = ::getenv( "SSL_CERT_FILE" );
        if ( GUCEF_NULL != envFile && '\0' != envFile[ 0 ] )
        {
            std::string pem;
            if ( ReadWholeFile( envFile, pem ) && AppendPemCertificates( pem.data(), pem.size(), anchors ) )
                haveAny = true;
        }
        const char* envDir = ::getenv( "SSL_CERT_DIR" );
        if ( GUCEF_NULL != envDir && '\0' != envDir[ 0 ] )
        {
            DIR* dir = ::opendir( envDir );
            if ( GUCEF_NULL != dir )
            {
                struct dirent* entry = GUCEF_NULL;
                while ( GUCEF_NULL != ( entry = ::readdir( dir ) ) )
                {
                    std::string pem;
                    if ( '.' != entry->d_name[ 0 ] && ReadWholeFile( CORE::CombinePath( envDir, entry->d_name ), pem ) &&
                         AppendPemCertificates( pem.data(), pem.size(), anchors ) )
                        haveAny = true;
                }
                ::closedir( dir );
            }
        }
    }

    if ( !haveAny )
    {
        ::CFRelease( anchors );
        return GUCEF_NULL;
    }
    return anchors;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSecureTransportTlsProvider::CSecureTransportTlsProvider( void )
    : ITlsProvider()
    , m_serverIdentity()
    , m_keychain( GUCEF_NULL )
    , m_keychains()
    , m_keychainPaths()
    , m_verifyTimeOverride( 0 )
    , m_lock()
{GUCEF_TRACE;

    m_serverIdentity.chain = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CSecureTransportTlsProvider::~CSecureTransportTlsProvider()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_serverIdentity.chain )
        ::CFRelease( (CFTypeRef) m_serverIdentity.chain );
    for ( size_t i=0; i<m_keychains.size(); ++i )
    {
        ::SecKeychainDelete( (SecKeychainRef) m_keychains[ i ] );
        ::CFRelease( (CFTypeRef) m_keychains[ i ] );
    }
    for ( size_t i=0; i<m_keychainPaths.size(); ++i )
        ::unlink( m_keychainPaths[ i ].C_String() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSecureTransportTlsProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsProvider::IsAvailable( CORE::CString& whyNot ) const
{GUCEF_TRACE;

    // Part of the operating system
    whyNot.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CSecureTransportTlsProvider::GetCapabilities( void ) const
{GUCEF_TRACE;

    return TLSCAP_ALPN | TLSCAP_CUSTOM_TRUST | TLSCAP_SERVER;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSecureTransportTlsProvider::GetVersionInfo( void ) const
{GUCEF_TRACE;

    return "Secure Transport (macOS Security.framework), TLS 1.2";
}

/*-------------------------------------------------------------------------*/

void
CSecureTransportTlsProvider::SetVerificationTimeForTesting( Int64 unixTime )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_verifyTimeOverride = unixTime;
}

/*-------------------------------------------------------------------------*/

TTlsSessionPtr
CSecureTransportTlsProvider::CreateClientSession( const CTlsSettings& settings    ,
                                                  const CORE::CString& serverName ,
                                                  CTlsError& error                )
{GUCEF_TRACE;

    error.Clear();

    if ( TLSVERSION_1_3 == settings.GetMinVersion() )
    {
        error = CTlsError( TLSERR_SETTINGS, "the minimum version TLS 1.3 cannot be met: Secure Transport speaks TLS 1.2 at most" );
        return TTlsSessionPtr();
    }
    if ( TLSVERSION_ANY != settings.GetMaxVersion() && settings.GetMaxVersion() < TLSVERSION_1_2 )
    {
        error = CTlsError( TLSERR_SETTINGS, "there is no version of TLS older than 1.2 to use" );
        return TTlsSessionPtr();
    }

    bool haveCustomTrust = false;
    CFArrayRef anchors = LoadAnchors( settings, error, haveCustomTrust );
    if ( error.IsError() )
        return TTlsSessionPtr();
    if ( !haveCustomTrust && !settings.GetUseSystemTrust() && settings.IsVerificationEnabled() )
    {
        error = CTlsError( TLSERR_SETTINGS, "verification is on, the trust of the system is not to be used and no CA is given: nothing can be trusted" );
        return TTlsSessionPtr();
    }

    Int64 verifyTime = 0;
    {
        MT::CScopeMutex lock( m_lock );
        verifyTime = m_verifyTimeOverride;
    }

    CSecureTransportTlsSession* session = GUCEF_NEW CSecureTransportTlsSession( settings, serverName, (void*) anchors, verifyTime );
    TTlsSessionPtr sessionPtr( session );
    if ( GUCEF_NULL != anchors )
        ::CFRelease( anchors );         // the session holds its own reference
    if ( !session->Initialize( error ) )
        return TTlsSessionPtr();
    return sessionPtr;
}

/*-------------------------------------------------------------------------*/

bool
CSecureTransportTlsProvider::EnsureKeychain( CTlsError& error )
{GUCEF_TRACE;

    // A keychain of our own for every key: they are imported into it and never into the keychains of the user, and a key that
    // was imported before cannot be told from the one that is imported now when they share a keychain
    static UInt32 counter = 0;
    CORE::CString path = CORE::CombinePath( CORE::TempDir(), "gucef-tls-" + CORE::ToString( (UInt32) ::getpid() ) + "-" + CORE::ToString( ++counter ) + ".keychain" );
    const char* password = "gucef";

    SecKeychainRef keychain = GUCEF_NULL;
    OSStatus status = ::SecKeychainCreate( path.C_String(), (UInt32) ::strlen( password ), password, false, GUCEF_NULL, &keychain );
    if ( noErr != status || GUCEF_NULL == keychain )
    {
        error = CTlsError( TLSERR_INTERNAL, "cannot create a keychain for the key of the server (status " + CORE::ToString( (Int32) status ) + ")" );
        return false;
    }

    m_keychain = (void*) keychain;
    m_keychains.push_back( (void*) keychain );
    m_keychainPaths.push_back( path );
    return true;
}

/*-------------------------------------------------------------------------*/

void*
CSecureTransportTlsProvider::AcquireServerIdentity( const CTlsServerSettings& settings, CTlsError& error )
{GUCEF_TRACE;

    const CORE::CString stamp = settings.GetCertificateChainPath() + "|" + settings.GetPrivateKeyPath() + "|" +
                                GetFileStamp( settings.GetCertificateChainPath() ) + "|" + GetFileStamp( settings.GetPrivateKeyPath() );

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL != m_serverIdentity.chain && m_serverIdentity.key == stamp )
    {
        ::CFRetain( (CFTypeRef) m_serverIdentity.chain );
        return m_serverIdentity.chain;
    }

    std::string certPem, keyPem;
    if ( !ReadWholeFile( settings.GetCertificateChainPath(), certPem ) )
    {
        error = CTlsError( TLSERR_SETTINGS, "cannot read the certificate file \"" + settings.GetCertificateChainPath() + "\"" );
        return GUCEF_NULL;
    }
    if ( !ReadWholeFile( settings.GetPrivateKeyPath(), keyPem ) )
    {
        error = CTlsError( TLSERR_SETTINGS, "cannot read the private key file \"" + settings.GetPrivateKeyPath() + "\"" );
        return GUCEF_NULL;
    }

    CFMutableArrayRef certificates = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    if ( !AppendPemCertificates( certPem.data(), certPem.size(), certificates ) )
    {
        error = CTlsError( TLSERR_SETTINGS, "the certificate file \"" + settings.GetCertificateChainPath() + "\" holds no certificate" );
        ::CFRelease( certificates );
        return GUCEF_NULL;
    }

    if ( !EnsureKeychain( error ) )
    {
        ::CFRelease( certificates );
        return GUCEF_NULL;
    }

    // A keychain locks itself, for example when the computer sleeps: nobody is there to type the password, we know it
    ::SecKeychainUnlock( (SecKeychainRef) m_keychain, (UInt32) ::strlen( "gucef" ), "gucef", true );

    // The key goes into the keychain of ours, the identity is the pairing of the certificate with it
    std::string sec1Pem;
    if ( ConvertPkcs8EcKeyToSec1Pem( keyPem, sec1Pem ) )
        keyPem = sec1Pem;
    CFDataRef keyData = ::CFDataCreate( kCFAllocatorDefault, (const UInt8*) keyPem.data(), (CFIndex) keyPem.size() );
    SecItemImportExportKeyParameters keyParams;
    ::memset( &keyParams, 0, sizeof( keyParams ) );
    keyParams.version = SEC_KEY_IMPORT_EXPORT_PARAMS_VERSION;
    keyParams.flags = kSecKeyNoAccessControl;

    CFArrayRef importedKeys = GUCEF_NULL;
    // The system tells the formats of PEM keys apart when it is not told which one this is
    SecExternalFormat format = kSecFormatUnknown;
    SecExternalItemType itemType = kSecItemTypePrivateKey;
    OSStatus status = ::SecItemImport( keyData, GUCEF_NULL, &format, &itemType, 0, &keyParams, (SecKeychainRef) m_keychain, &importedKeys );
    ::CFRelease( keyData );

    // The key has to be the one of the certificate. The keychain keeps the keys of earlier servers, and finding the key of the
    // certificate in it is not the same as the key of this file being that key: the identity would be made with another key
    bool keyMatches = false;
    {
        SecCertificateRef leafCert = (SecCertificateRef) ::CFArrayGetValueAtIndex( certificates, 0 );
        SecKeyRef certKey = ::SecCertificateCopyKey( leafCert );
        CFDataRef certLabel = CopyKeyLabel( certKey );
        if ( GUCEF_NULL != certKey )
            ::CFRelease( certKey );
        if ( noErr == status && GUCEF_NULL != importedKeys && ::CFArrayGetCount( importedKeys ) > 0 && GUCEF_NULL != certLabel )
        {
            CFDataRef keyLabel = CopyKeyLabel( (SecKeyRef) ::CFArrayGetValueAtIndex( importedKeys, 0 ) );
            keyMatches = GUCEF_NULL != keyLabel && ::CFEqual( keyLabel, certLabel );
            if ( GUCEF_NULL != keyLabel )
                ::CFRelease( keyLabel );
        }
        if ( GUCEF_NULL != certLabel )
            ::CFRelease( certLabel );
    }
    if ( GUCEF_NULL != importedKeys )
        ::CFRelease( importedKeys );
    if ( noErr == status && !keyMatches )
    {
        error = CTlsError( TLSERR_SETTINGS, "the private key \"" + settings.GetPrivateKeyPath() + "\" does not belong to the certificate \"" + settings.GetCertificateChainPath() + "\"" );
        ::CFRelease( certificates );
        return GUCEF_NULL;
    }
    if ( noErr != status )
    {
        error = CTlsError( TLSERR_SETTINGS, "cannot use the private key file \"" + settings.GetPrivateKeyPath() + "\" (status " + CORE::ToString( (Int32) status ) + ")" );
        ::CFRelease( certificates );
        return GUCEF_NULL;
    }

    SecIdentityRef identity = GUCEF_NULL;
    SecCertificateRef leaf = (SecCertificateRef) ::CFArrayGetValueAtIndex( certificates, 0 );
    status = ::SecIdentityCreateWithCertificate( (CFTypeRef) m_keychain, leaf, &identity );
    if ( noErr != status || GUCEF_NULL == identity )
    {
        error = CTlsError( TLSERR_SETTINGS, "the private key \"" + settings.GetPrivateKeyPath() + "\" does not belong to the certificate \"" + settings.GetCertificateChainPath() + "\"" );
        ::CFRelease( certificates );
        return GUCEF_NULL;
    }

    CFMutableArrayRef chain = ::CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
    ::CFArrayAppendValue( chain, identity );
    for ( CFIndex i=1; i<::CFArrayGetCount( certificates ); ++i )
        ::CFArrayAppendValue( chain, ::CFArrayGetValueAtIndex( certificates, i ) );
    ::CFRelease( identity );
    ::CFRelease( certificates );

    if ( GUCEF_NULL != m_serverIdentity.chain )
        ::CFRelease( (CFTypeRef) m_serverIdentity.chain );
    m_serverIdentity.chain = (void*) chain;
    m_serverIdentity.key = stamp;

    ::CFRetain( chain );
    return (void*) chain;
}

/*-------------------------------------------------------------------------*/

TTlsSessionPtr
CSecureTransportTlsProvider::CreateServerSession( const CTlsServerSettings& settings ,
                                                  CTlsError& error                   )
{GUCEF_TRACE;

    error.Clear();

    if ( !settings.IsEnabled() )
    {
        error = CTlsError( TLSERR_SETTINGS, "a TLS server needs a certificate file and a private key file" );
        return TTlsSessionPtr();
    }
    if ( TLSVERSION_1_3 == settings.GetMinVersion() )
    {
        error = CTlsError( TLSERR_SETTINGS, "the minimum version TLS 1.3 cannot be met: Secure Transport speaks TLS 1.2 at most" );
        return TTlsSessionPtr();
    }

    void* chain = AcquireServerIdentity( settings, error );
    if ( GUCEF_NULL == chain )
        return TTlsSessionPtr();

    CSecureTransportTlsSession* session = GUCEF_NEW CSecureTransportTlsSession( settings, chain );
    TTlsSessionPtr sessionPtr( session );
    ::CFRelease( (CFTypeRef) chain );   // the session holds its own reference
    if ( !session->Initialize( error ) )
        return TTlsSessionPtr();
    return sessionPtr;
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
