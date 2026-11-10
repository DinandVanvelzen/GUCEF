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

#ifndef GUCEF_AUTH_CJWT_H
#define GUCEF_AUTH_CJWT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_CJWK_H
#include "gucefAUTH_CJwk.h"
#define GUCEF_AUTH_CJWK_H
#endif /* GUCEF_AUTH_CJWK_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Why a token was refused */
enum EJwtStatus
{
    JWT_OK = 0,
    JWT_MALFORMED,              /**< not a compact JWS, not base64url, not JSON, a duplicate member, or nested too deep */
    JWT_TOO_LARGE,              /**< over the size limits of the token, the header or the payload */
    JWT_ALG_NOT_ALLOWED,        /**< "none", an HMAC algorithm or anything else but RS256, RS384 and RS512 */
    JWT_UNSUPPORTED_HEADER,     /**< a "crit" header: extensions that are not understood have to be refused */
    JWT_KEY_ID_MISSING,         /**< the header has no key id */
    JWT_KEY_NOT_FOUND,          /**< no key with that key id at the provider (set by whoever looks the key up) */
    JWT_KEY_MISMATCH,           /**< the key is not for the algorithm of the token, or has another key id */
    JWT_NO_VERIFIER,            /**< the signature codec is not there */
    JWT_BAD_SIGNATURE,
    JWT_CLAIM_MISSING,          /**< iss, aud or exp is missing or not of the type it has to be */
    JWT_ISSUER,
    JWT_AUDIENCE,
    JWT_AUTHORIZED_PARTY,
    JWT_EXPIRED,
    JWT_NOT_YET_VALID,
    JWT_ISSUED_IN_FUTURE,
    JWT_NONCE,
    JWT_AT_HASH,
    JWT_TENANT                  /**< the tenant claim does not match the tenant of the issuer */
};
typedef enum EJwtStatus TJwtStatus;

/** A stable lower case name of a status, for logs and messages; never contains token text */
GUCEF_AUTH_EXPORT_CPP const char*
JwtStatusToString( TJwtStatus status );

/*-------------------------------------------------------------------------*/

/** What a token has to be, besides signed: the checks of CJwt::ValidateClaims() */
class GUCEF_AUTH_EXPORT_CPP CJwtValidationOptions
{
    public:

    static const UInt32 DefaultClockSkewInSeconds = 120;
    static const UInt32 MaxClockSkewInSeconds = 600;

    CORE::CString issuer;                   /**< the exact issuer; may hold {tid} for an issuer that names the tenant of the account */
    CORE::CString audience;                 /**< a client id: aud has to contain it, or one of audiences */
    CORE::CString::StringVector audiences;  /**< further client ids, for a server that has the clients of several platforms or projects: aud has to contain at least one id of audience and audiences; with none, nothing is accepted */
    bool checkNonce;
    CORE::CString nonce;                    /**< the nonce of the login; the token has to carry it when checkNonce is set */
    CSecretString accessToken;              /**< when set and the token has an at_hash, it has to be the hash of this access token */
    bool requireAtHash;                     /**< when an access token is given, a token without at_hash is refused */
    UInt32 clockSkewInSeconds;              /**< allowed difference of clocks on exp, nbf and iat, at most MaxClockSkewInSeconds */
    UInt64 nowEpochSeconds;                 /**< the time to validate at; 0 is the clock */

    CJwtValidationOptions( void );

    /** The client ids that count: audience and the entries of audiences, without empty ones and duplicates */
    void GetAcceptedAudiences( CORE::CString::StringVector& accepted ) const;
};

/*-------------------------------------------------------------------------*/

/**
 *  A signed token in the compact serialization of JWS (RFC 7515), the form an OpenID Connect ID token has. The class
 *  parses and checks the shape, verifies the signature with a key it is given and validates the standard claims.
 *  Which key to use is the business of the caller: it looks up the key id (see CJwksCache).
 *
 *  Only RS256, RS384 and RS512 are accepted. The algorithm of the token never selects the key type: "none", the HMAC
 *  algorithms (a public key used as an HMAC secret) and everything else are refused when the token is parsed.
 *  Tokens are secrets: nothing in this class writes token text to a log or a message.
 */
class GUCEF_AUTH_EXPORT_CPP CJwt
{
    public:

    static const UInt32 MaxTokenBytes = 16384;
    static const UInt32 MaxHeaderBytes = 1024;
    static const UInt32 MaxPayloadBytes = 12288;
    static const UInt32 MaxSignatureBytes = 1024;       /**< 8192 bit keys */
    static const UInt32 MaxKeyIdBytes = 256;

    CJwt( void );
    CJwt( const CJwt& src );
    ~CJwt();
    CJwt& operator=( const CJwt& src );

    /** Parses the token and checks its shape and algorithm; the signature is not checked */
    TJwtStatus Parse( const CORE::CString& compact, UInt32 maxTokenBytes = MaxTokenBytes );

    /** Verifies the signature with the key. The key has to be for the algorithm of the token and have the key id of the token */
    TJwtStatus VerifySignature( const CJwk& key ) const;

    /** Validates the claims; with an issuer that has {tid} the tenant of the token is in tenantId */
    TJwtStatus ValidateClaims( const CJwtValidationOptions& options, CORE::CString* tenantId = GUCEF_NULL ) const;

    const CORE::CString& GetAlgorithm( void ) const;

    const CORE::CString& GetKeyId( void ) const;

    bool GetClaimString( const CORE::CString& name, CORE::CString& value ) const;

    bool GetClaimInt64( const CORE::CString& name, Int64& value ) const;

    /** The scalar claims as text, nested ones with dotted names */
    void GetClaims( CORE::CString::StringMap& claims ) const;

    /** Matches the issuer against a template with one {tid}; the text in the place of {tid} is the tenant. A template without {tid} is an exact comparison */
    static bool MatchIssuer( const CORE::CString& issuerTemplate, const CORE::CString& issuer, CORE::CString& tenantId );

    /** The left half of the hash of the access token as base64url, for the algorithm of the token: the at_hash a token has to carry */
    static bool ComputeHalfHash( const CORE::CString& jwsAlgorithm, const CORE::CString& text, CORE::CString& halfHashBase64Url );

    private:

    CORE::CDataNode m_header;
    CORE::CDataNode m_payload;
    CORE::CString m_signingInput;
    CORE::CDynamicBuffer m_signature;
    CORE::CString m_algorithm;
    CORE::CString m_keyId;
    bool m_parsed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CJWT_H ? */
