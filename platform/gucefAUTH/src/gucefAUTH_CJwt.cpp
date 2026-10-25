/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA512_H
#include "gucefCORE_CSha512.h"
#define GUCEF_CORE_CSHA512_H
#endif /* GUCEF_CORE_CSHA512_H ? */

#ifndef GUCEF_AUTH_CAUTHJSON_H
#include "gucefAUTH_CAuthJson.h"
#define GUCEF_AUTH_CAUTHJSON_H
#endif /* GUCEF_AUTH_CAUTHJSON_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CJwt.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CJwtValidationOptions::DefaultClockSkewInSeconds;
const UInt32 CJwtValidationOptions::MaxClockSkewInSeconds;
const UInt32 CJwt::MaxTokenBytes;
const UInt32 CJwt::MaxHeaderBytes;
const UInt32 CJwt::MaxPayloadBytes;
const UInt32 CJwt::MaxSignatureBytes;
const UInt32 CJwt::MaxKeyIdBytes;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
JwtStatusToString( TJwtStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case JWT_OK :                   return "ok";
        case JWT_MALFORMED :            return "malformed";
        case JWT_TOO_LARGE :            return "too-large";
        case JWT_ALG_NOT_ALLOWED :      return "algorithm-not-allowed";
        case JWT_UNSUPPORTED_HEADER :   return "unsupported-header";
        case JWT_KEY_ID_MISSING :       return "key-id-missing";
        case JWT_KEY_NOT_FOUND :        return "key-not-found";
        case JWT_KEY_MISMATCH :         return "key-mismatch";
        case JWT_NO_VERIFIER :          return "no-verifier";
        case JWT_BAD_SIGNATURE :        return "bad-signature";
        case JWT_CLAIM_MISSING :        return "claim-missing";
        case JWT_ISSUER :               return "issuer";
        case JWT_AUDIENCE :             return "audience";
        case JWT_AUTHORIZED_PARTY :     return "authorized-party";
        case JWT_EXPIRED :              return "expired";
        case JWT_NOT_YET_VALID :        return "not-yet-valid";
        case JWT_ISSUED_IN_FUTURE :     return "issued-in-future";
        case JWT_NONCE :                return "nonce";
        case JWT_AT_HASH :              return "at-hash";
        case JWT_TENANT :               return "tenant";
    }
    return "unknown";
}

/*-------------------------------------------------------------------------*/

CJwtValidationOptions::CJwtValidationOptions( void )
    : issuer()
    , audience()
    , audiences()
    , checkNonce( false )
    , nonce()
    , accessToken()
    , requireAtHash( false )
    , clockSkewInSeconds( DefaultClockSkewInSeconds )
    , nowEpochSeconds( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CJwtValidationOptions::GetAcceptedAudiences( CORE::CString::StringVector& accepted ) const
{GUCEF_TRACE;

    accepted.clear();
    if ( !audience.IsNULLOrEmpty() )
        accepted.push_back( audience );
    for ( size_t i=0; i<audiences.size(); ++i )
        if ( !audiences[ i ].IsNULLOrEmpty() && !ContainsString( accepted, audiences[ i ] ) )
            accepted.push_back( audiences[ i ] );
}

/*-------------------------------------------------------------------------*/

CJwt::CJwt( void )
    : m_header()
    , m_payload()
    , m_signingInput()
    , m_signature()
    , m_algorithm()
    , m_keyId()
    , m_parsed( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwt::CJwt( const CJwt& src )
    : m_header( src.m_header )
    , m_payload( src.m_payload )
    , m_signingInput( src.m_signingInput )
    , m_signature( src.m_signature )
    , m_algorithm( src.m_algorithm )
    , m_keyId( src.m_keyId )
    , m_parsed( src.m_parsed )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwt::~CJwt()
{GUCEF_TRACE;

    // The signing input is the token without its signature, not a secret by itself but it is the token
    m_signingInput.Clear();
}

/*-------------------------------------------------------------------------*/

CJwt&
CJwt::operator=( const CJwt& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_header = src.m_header;
        m_payload = src.m_payload;
        m_signingInput = src.m_signingInput;
        m_signature = src.m_signature;
        m_algorithm = src.m_algorithm;
        m_keyId = src.m_keyId;
        m_parsed = src.m_parsed;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

static bool
IsAllowedAlgorithm( const CORE::CString& algorithm )
{GUCEF_TRACE;

    // Compared as is: "RS256" only, not "rs256", not "none"
    return algorithm == "RS256" || algorithm == "RS384" || algorithm == "RS512";
}

/*-------------------------------------------------------------------------*/

TJwtStatus
CJwt::Parse( const CORE::CString& compact, UInt32 maxTokenBytes )
{GUCEF_TRACE;

    m_parsed = false;
    m_header.Clear();
    m_payload.Clear();
    m_signingInput.Clear();
    m_signature.Clear( true );
    m_algorithm.Clear();
    m_keyId.Clear();

    UInt32 size = GetByteLength( compact );
    if ( size > maxTokenBytes || size > MaxTokenBytes )
        return JWT_TOO_LARGE;
    if ( 0 == size )
        return JWT_MALFORMED;

    // Exactly three parts
    const char* text = compact.C_String();
    UInt32 firstDot = size, secondDot = size;
    UInt32 dotCount = 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '.' == text[ i ] )
        {
            if ( 0 == dotCount )
                firstDot = i;
            else
            if ( 1 == dotCount )
                secondDot = i;
            ++dotCount;
        }
    }
    if ( 2 != dotCount || 0 == firstDot || secondDot == firstDot + 1 )
        return JWT_MALFORMED;

    CORE::CString headerPart = MakeString( text, firstDot );
    CORE::CString payloadPart = MakeString( text + firstDot + 1, secondDot - firstDot - 1 );
    CORE::CString signaturePart = MakeString( text + secondDot + 1, size - secondDot - 1 );

    // The header first: the algorithm decides what else is looked at
    if ( GetByteLength( headerPart ) > ( MaxHeaderBytes * 4 + 2 ) / 3 )
        return JWT_TOO_LARGE;
    CORE::CDynamicBuffer headerBytes;
    if ( !CORE::Base64UrlDecode( headerPart, headerBytes ) || 0 == headerBytes.GetDataSize() )
        return JWT_MALFORMED;
    if ( headerBytes.GetDataSize() > MaxHeaderBytes )
        return JWT_TOO_LARGE;
    CORE::CDataNode headerRoot;
    if ( !CAuthJson::Parse( headerBytes.GetConstBufferPtr(), headerBytes.GetDataSize(), MaxHeaderBytes, 4, true, headerRoot ) )
        return JWT_MALFORMED;
    const CORE::CDataNode& header = CAuthJson::GetDocument( headerRoot );

    CORE::CString algorithm;
    if ( !CAuthJson::GetString( header, "alg", algorithm ) || !IsAllowedAlgorithm( algorithm ) )
        return JWT_ALG_NOT_ALLOWED;
    if ( header.IsAttribute( "crit" ) || GUCEF_NULL != header.FindChild( "crit" ) )
        return JWT_UNSUPPORTED_HEADER;

    CORE::CString keyId;
    if ( !CAuthJson::GetString( header, "kid", keyId ) || keyId.IsNULLOrEmpty() || GetByteLength( keyId ) > MaxKeyIdBytes )
        return JWT_KEY_ID_MISSING;

    // The payload
    if ( GetByteLength( payloadPart ) > ( MaxPayloadBytes * 4 + 2 ) / 3 )
        return JWT_TOO_LARGE;
    CORE::CDynamicBuffer payloadBytes;
    if ( !CORE::Base64UrlDecode( payloadPart, payloadBytes ) || 0 == payloadBytes.GetDataSize() )
        return JWT_MALFORMED;
    if ( payloadBytes.GetDataSize() > MaxPayloadBytes )
        return JWT_TOO_LARGE;
    CORE::CDataNode payloadRoot;
    if ( !CAuthJson::Parse( payloadBytes.GetConstBufferPtr(), payloadBytes.GetDataSize(), MaxPayloadBytes, 6, true, payloadRoot ) )
        return JWT_MALFORMED;

    // The signature
    CORE::CDynamicBuffer signature;
    if ( signaturePart.IsNULLOrEmpty() || !CORE::Base64UrlDecode( signaturePart, signature ) || 0 == signature.GetDataSize() )
        return JWT_MALFORMED;
    if ( signature.GetDataSize() > MaxSignatureBytes )
        return JWT_TOO_LARGE;

    m_header.Copy( header );
    m_payload.Copy( CAuthJson::GetDocument( payloadRoot ) );
    m_signingInput = MakeString( text, secondDot );
    m_signature = signature;
    m_algorithm = algorithm;
    m_keyId = keyId;
    m_parsed = true;
    return JWT_OK;
}

/*-------------------------------------------------------------------------*/

TJwtStatus
CJwt::VerifySignature( const CJwk& key ) const
{GUCEF_TRACE;

    if ( !m_parsed )
        return JWT_MALFORMED;
    // The key has to be the one the token names, and fit the algorithm of the token
    if ( key.keyId != m_keyId || !key.CanVerify( m_algorithm ) )
        return JWT_KEY_MISMATCH;
    if ( !CAuthCrypto::IsRsaAvailable() )
        return JWT_NO_VERIFIER;

    bool valid = CAuthCrypto::VerifyRsaPkcs1( m_algorithm                                 ,
                                              key.modulus.GetConstBufferPtr()             ,
                                              key.modulus.GetDataSize()                   ,
                                              key.exponent.GetConstBufferPtr()            ,
                                              key.exponent.GetDataSize()                  ,
                                              m_signingInput.C_String()                   ,
                                              GetByteLength( m_signingInput )             ,
                                              m_signature.GetConstBufferPtr()             ,
                                              m_signature.GetDataSize()                   );
    return valid ? JWT_OK : JWT_BAD_SIGNATURE;
}

/*-------------------------------------------------------------------------*/

bool
CJwt::MatchIssuer( const CORE::CString& issuerTemplate, const CORE::CString& issuer, CORE::CString& tenantId )
{GUCEF_TRACE;

    tenantId.Clear();
    Int32 at = issuerTemplate.HasSubstr( "{tid}" );
    if ( at < 0 )
        return !issuer.IsNULLOrEmpty() && issuerTemplate == issuer;

    // Compared as bytes: the template and the issuer are both text of the same encoding
    UInt32 templateSize = GetByteLength( issuerTemplate );
    UInt32 issuerSize = GetByteLength( issuer );
    const char* t = issuerTemplate.C_String();
    const char* s = issuer.C_String();

    // The byte position of the placeholder
    UInt32 prefixSize = 0;
    for ( UInt32 i=0; i+5<=templateSize; ++i )
    {
        if ( 0 == memcmp( t + i, "{tid}", 5 ) )
        {
            prefixSize = i;
            break;
        }
    }
    UInt32 suffixSize = templateSize - prefixSize - 5;
    if ( issuerSize <= prefixSize + suffixSize )
        return false;
    if ( 0 != memcmp( s, t, prefixSize ) || 0 != memcmp( s + issuerSize - suffixSize, t + prefixSize + 5, suffixSize ) )
        return false;

    // The tenant: letters, digits and a few separators, no path or query characters that would let an issuer of another place pass
    UInt32 tenantSize = issuerSize - prefixSize - suffixSize;
    for ( UInt32 i=0; i<tenantSize; ++i )
    {
        char c = s[ prefixSize + i ];
        bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c || '_' == c;
        if ( !ok )
            return false;
    }
    tenantId = MakeString( s + prefixSize, tenantSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CJwt::ComputeHalfHash( const CORE::CString& jwsAlgorithm, const CORE::CString& text, CORE::CString& halfHashBase64Url )
{GUCEF_TRACE;

    halfHashBase64Url.Clear();
    UInt8 digest[ CORE::CSha512::DigestSize ];
    UInt32 digestSize = 0;
    UInt32 size = GetByteLength( text );
    if ( jwsAlgorithm == "RS256" )
    {
        CAuthCrypto::Sha256( text.C_String(), size, digest );
        digestSize = CAuthCrypto::DigestSize;
    }
    else
    if ( jwsAlgorithm == "RS384" )
    {
        CORE::CSha512::Hash384( text.C_String(), size, digest );
        digestSize = CORE::CSha512::Digest384Size;
    }
    else
    if ( jwsAlgorithm == "RS512" )
    {
        CORE::CSha512::Hash( text.C_String(), size, digest );
        digestSize = CORE::CSha512::DigestSize;
    }
    else
        return false;
    return CORE::Base64UrlEncode( digest, digestSize / 2, halfHashBase64Url );
}

/*-------------------------------------------------------------------------*/

static bool
SameText( const CORE::CString& a, const CORE::CString& b )
{GUCEF_TRACE;

    UInt32 sizeA = GetByteLength( a );
    return sizeA == GetByteLength( b ) && CAuthCrypto::ConstantTimeEquals( a.C_String(), b.C_String(), sizeA );
}

/*-------------------------------------------------------------------------*/

TJwtStatus
CJwt::ValidateClaims( const CJwtValidationOptions& options, CORE::CString* tenantId ) const
{GUCEF_TRACE;

    if ( !m_parsed )
        return JWT_MALFORMED;
    if ( GUCEF_NULL != tenantId )
        tenantId->Clear();

    UInt64 now = options.nowEpochSeconds;
    if ( 0 == now )
        now = GetEpochTimeInMs() / 1000;
    UInt32 skew = options.clockSkewInSeconds > CJwtValidationOptions::MaxClockSkewInSeconds ? CJwtValidationOptions::MaxClockSkewInSeconds : options.clockSkewInSeconds;

    // Times: exp is required, nbf and iat when present
    Int64 expires = 0;
    if ( !CAuthJson::GetInt64( m_payload, "exp", expires ) )
        return JWT_CLAIM_MISSING;
    if ( expires < 0 || (UInt64) expires + skew <= now )
        return JWT_EXPIRED;

    Int64 notBefore = 0;
    if ( m_payload.IsAttribute( "nbf" ) )
    {
        if ( !CAuthJson::GetInt64( m_payload, "nbf", notBefore ) )
            return JWT_CLAIM_MISSING;
        if ( notBefore > 0 && (UInt64) notBefore > now + skew )
            return JWT_NOT_YET_VALID;
    }

    Int64 issuedAt = 0;
    if ( m_payload.IsAttribute( "iat" ) )
    {
        if ( !CAuthJson::GetInt64( m_payload, "iat", issuedAt ) )
            return JWT_CLAIM_MISSING;
        if ( issuedAt > 0 && (UInt64) issuedAt > now + skew )
            return JWT_ISSUED_IN_FUTURE;
    }

    // Issuer: exact, or the template with the tenant
    CORE::CString issuer;
    if ( !CAuthJson::GetString( m_payload, "iss", issuer ) )
        return JWT_CLAIM_MISSING;
    CORE::CString tenant;
    if ( options.issuer.IsNULLOrEmpty() || !MatchIssuer( options.issuer, issuer, tenant ) )
        return JWT_ISSUER;
    if ( !tenant.IsNULLOrEmpty() )
    {
        // A token that carries its tenant has to agree with its issuer about it
        CORE::CString tenantClaim;
        if ( CAuthJson::GetString( m_payload, "tid", tenantClaim ) && tenantClaim != tenant )
            return JWT_TENANT;
        if ( GUCEF_NULL != tenantId )
            *tenantId = tenant;
    }

    // Audience: a string or a list, it has to contain a client id of ours; with several, or with an authorized party, the authorized
    // party is a client id of ours as well (a token for the web client of a game that an Android client asked for has the web client
    // as audience and the Android client as authorized party). Without a client id of ours nothing is accepted
    CORE::CString::StringVector audiences;
    if ( !CAuthJson::GetStringList( m_payload, "aud", audiences ) || audiences.empty() )
        return JWT_CLAIM_MISSING;
    CORE::CString::StringVector accepted;
    options.GetAcceptedAudiences( accepted );
    bool audienceIsOurs = false;
    for ( size_t i=0; i<audiences.size() && !audienceIsOurs; ++i )
        audienceIsOurs = ContainsString( accepted, audiences[ i ] );
    if ( !audienceIsOurs )
        return JWT_AUDIENCE;
    CORE::CString authorizedParty;
    bool hasAuthorizedParty = m_payload.IsAttribute( "azp" );
    if ( hasAuthorizedParty && !CAuthJson::GetString( m_payload, "azp", authorizedParty ) )
        return JWT_AUTHORIZED_PARTY;
    if ( ( audiences.size() > 1 || hasAuthorizedParty ) && ( authorizedParty.IsNULLOrEmpty() || !ContainsString( accepted, authorizedParty ) ) )
        return JWT_AUTHORIZED_PARTY;

    // Nonce
    if ( options.checkNonce )
    {
        CORE::CString nonce;
        if ( options.nonce.IsNULLOrEmpty() || !CAuthJson::GetString( m_payload, "nonce", nonce ) || !SameText( nonce, options.nonce ) )
            return JWT_NONCE;
    }

    // Access token hash
    if ( !options.accessToken.IsEmpty() )
    {
        CORE::CString atHash;
        if ( !CAuthJson::GetString( m_payload, "at_hash", atHash ) )
        {
            // absent is allowed unless asked for, a claim of the wrong type is not
            if ( options.requireAtHash || m_payload.IsAttribute( "at_hash" ) )
                return JWT_AT_HASH;
        }
        else
        {
            CORE::CString expected;
            if ( !ComputeHalfHash( m_algorithm, options.accessToken.Reveal(), expected ) || !SameText( atHash, expected ) )
                return JWT_AT_HASH;
        }
    }
    return JWT_OK;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CJwt::GetAlgorithm( void ) const
{GUCEF_TRACE;

    return m_algorithm;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CJwt::GetKeyId( void ) const
{GUCEF_TRACE;

    return m_keyId;
}

/*-------------------------------------------------------------------------*/

bool
CJwt::GetClaimString( const CORE::CString& name, CORE::CString& value ) const
{GUCEF_TRACE;

    return CAuthJson::GetString( m_payload, name, value );
}

/*-------------------------------------------------------------------------*/

bool
CJwt::GetClaimInt64( const CORE::CString& name, Int64& value ) const
{GUCEF_TRACE;

    return CAuthJson::GetInt64( m_payload, name, value );
}

/*-------------------------------------------------------------------------*/

void
CJwt::GetClaims( CORE::CString::StringMap& claims ) const
{GUCEF_TRACE;

    CAuthJson::Flatten( m_payload, claims );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
