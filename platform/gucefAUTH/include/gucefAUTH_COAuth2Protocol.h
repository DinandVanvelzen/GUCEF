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

#ifndef GUCEF_AUTH_COAUTH2PROTOCOL_H
#define GUCEF_AUTH_COAUTH2PROTOCOL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#include "gucefAUTH_CAuthProviderDescriptor.h"
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */

#ifndef GUCEF_AUTH_CAUTHSETTINGS_H
#include "gucefAUTH_CAuthSettings.h"
#define GUCEF_AUTH_CAUTHSETTINGS_H
#endif /* GUCEF_AUTH_CAUTHSETTINGS_H ? */

#ifndef GUCEF_AUTH_CAUTHOPTIONS_H
#include "gucefAUTH_CAuthOptions.h"
#define GUCEF_AUTH_CAUTHOPTIONS_H
#endif /* GUCEF_AUTH_CAUTHOPTIONS_H ? */

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

/** The parts of an OpenID Connect discovery document (RFC 8414, OpenID Connect Discovery) the engine uses */
class GUCEF_AUTH_EXPORT_CPP COidcMetadata
{
    public:

    CORE::CString issuer;
    CORE::CString authorizationEndpoint;
    CORE::CString tokenEndpoint;
    CORE::CString deviceAuthorizationEndpoint;
    CORE::CString userInfoEndpoint;
    CORE::CString jwksUri;
    CORE::CString revocationEndpoint;

    COidcMetadata( void );

    /**
     *  Reads the document. The issuer of the document has to be the expected one (a template with {tid} is matched the
     *  way a token issuer is; the {tenantid} that some providers leave in the document of the shared tenants is accepted)
     *  and every endpoint has to be acceptable (https, or loopback http when allowed).
     */
    bool Parse( const void* data, UInt32 size, const CORE::CString& expectedIssuer, bool insecureLoopbackAllowed );
};

/*-------------------------------------------------------------------------*/

/** What a provider with its settings comes to: the values the flows work with */
class GUCEF_AUTH_EXPORT_CPP COAuth2Resolved
{
    public:

    CORE::CString providerName;
    CORE::CString clientId;                     /**< the client id of a login */
    CSecretString clientSecret;
    CORE::CString::StringVector acceptedClientIds;  /**< what a server accepts as the audience of a proof: clientId and the further client ids of the settings; empty accepts nothing */
    CORE::CString::StringVector scopes;
    CORE::CString issuer;                       /**< placeholders filled in, except {tid} */
    CORE::CString discoveryUrl;
    CORE::CString authorizationEndpoint;
    CORE::CString tokenEndpoint;
    CORE::CString deviceAuthorizationEndpoint;
    CORE::CString userInfoEndpoint;
    CORE::CString jwksUri;
    CORE::CString revocationEndpoint;
    CORE::CString tokenVerifyEndpoint;
    CORE::CString::StringMap parameters;        /**< the defaults of the descriptor with the settings on top */
    CORE::CString redirectMode;                 /**< "loopback", "custom-scheme" or "manual" */
    UInt16 loopbackPort;
    CORE::CString loopbackPath;
    CORE::CString redirectUri;
    CORE::CString relayBase;                    /**< OpenID 2.0 only: the normalized address of the relay page (the parameter relayUrl), empty when the loopback address is the realm */
    CORE::CString relayRealm;                   /**< the origin of the relay with a slash: the realm the provider shows the player */
    bool wantsDiscovery;                        /**< the discovery document should be fetched and the values resolved again */

    COAuth2Resolved( void );
};

/*-------------------------------------------------------------------------*/

/** The answer of a token endpoint or a device authorization endpoint */
class GUCEF_AUTH_EXPORT_CPP COAuth2TokenResponse
{
    public:

    bool hasAccessToken;
    CSecretString accessToken;
    CSecretString refreshToken;
    CSecretString idToken;
    CORE::CString tokenType;
    bool hasExpiresIn;
    UInt64 expiresInSeconds;
    CORE::CString scope;
    CORE::CString error;                        /**< the error code of the provider, empty when there is none */
    CORE::CString errorDescription;
    CSecretString deviceCode;
    CORE::CString userCode;
    CORE::CString verificationUri;
    CORE::CString verificationUriComplete;
    bool hasInterval;
    UInt32 interval;                            /**< seconds */

    COAuth2TokenResponse( void );
};

/*-------------------------------------------------------------------------*/

/** The rules of OAuth 2.0, OpenID Connect and the device grant as functions without state */
class GUCEF_AUTH_EXPORT_CPP COAuth2Protocol
{
    public:

    static const UInt32 CodeVerifierBytes = 32;     /**< 32 random bytes, 43 characters of base64url */
    static const UInt32 RandomTokenBytes = 32;      /**< state and nonce */

    /** Random bytes of the operating system as base64url */
    static bool MakeRandomToken( UInt32 byteCount, CORE::CString& token );

    /** A code verifier: 43 to 128 characters of the unreserved set, from the secure random source */
    static bool MakeCodeVerifier( CORE::CString& verifier );

    /** base64url( SHA-256( verifier ) ), the S256 method */
    static bool MakeCodeChallenge( const CORE::CString& verifier, CORE::CString& challenge );

    /** Whether the verifier is 43 to 128 characters of A-Z a-z 0-9 - . _ ~ */
    static bool IsValidCodeVerifier( const CORE::CString& verifier );

    /**
     *  Works out the endpoints and the other values from the descriptor, the settings of the application and, when it is
     *  there, the discovery document. A value in the settings wins, then the discovery document when the settings name
     *  one themselves, then the descriptor, then the discovery document of the descriptor. False, with the reason as
     *  text without secrets, when a placeholder has no value or an endpoint is not acceptable.
     */
    static bool Resolve( const CAuthProviderDescriptor& descriptor  ,
                         const CAuthProviderSettings& settings      ,
                         const COidcMetadata* discovered            ,
                         bool insecureLoopbackAllowed               ,
                         COAuth2Resolved& resolved                  ,
                         CORE::CString& error                       );

    /** The scopes of the provider with the ones the login asks for, once each */
    static void MergeScopes( const CORE::CString::StringVector& base, const CORE::CString::StringVector& added, CORE::CString::StringVector& scopes );

    /** The longest login hint that is sent or remembered, in bytes (an e-mail address is at most 254) */
    static const UInt32 MaxLoginHintBytes = 254;

    /** Whether the text can be the name of an account to prefill: not empty, at most MaxLoginHintBytes and no control characters or line breaks */
    static bool IsAcceptableLoginHint( const CORE::CString& loginHint );

    /**
     *  The name of the account for the next login, from the claims of a verified login: the first of the claims the descriptor
     *  lists (loginHintClaims) that holds an acceptable value. Empty (and false) for a provider that takes no login hint
     */
    static bool FindLoginHint( const CAuthProviderDescriptor& descriptor, const CORE::CString::StringMap& claims, CORE::CString& loginHint );

    /**
     *  The url the browser is sent to for an authorization code login. The login hint is added under the parameter name of the
     *  descriptor when the descriptor says the provider takes one and the hint is acceptable; for the other providers it is left out
     */
    static CORE::CString BuildAuthorizationUrl( const CAuthProviderDescriptor& descriptor ,
                                                const COAuth2Resolved& resolved           ,
                                                const CORE::CString& redirectUri          ,
                                                const CORE::CString& state                ,
                                                const CORE::CString& nonce                ,
                                                const CORE::CString& codeChallenge        ,
                                                const CORE::CString& loginHint            );

    /** The parameters that go with a client at the token endpoint: the client id, and the secret when it goes in the body */
    static void AddClientParameters( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, CORE::CString::StringMap& parameters );

    /** The Authorization header value of the Basic client authentication; empty when the descriptor sends the secret in the body or there is none */
    static bool MakeBasicAuthorization( const CAuthProviderDescriptor& descriptor, const COAuth2Resolved& resolved, CSecretString& headerValue );

    /** The body of a request in the style of the descriptor: a form, or a JSON object with camel case names */
    static CORE::CString MakeBody( const CAuthProviderDescriptor& descriptor, const CORE::CString::StringMap& parameters, CORE::CString& contentType );

    /** Reads the answer of a token or device authorization endpoint. False when it is not a JSON object */
    static bool ParseTokenResponse( const void* data, UInt32 size, UInt32 maxSize, COAuth2TokenResponse& response );

    /** The reason a login or a refresh fails for an error code of a provider */
    static TAuthFailureReason ReasonForError( const CORE::CString& errorCode );

    /** Whether the tenant of a token fits the tenant setting of the application: common, organizations, consumers or a tenant id */
    static bool IsTenantAllowed( const CORE::CString& tenantSetting, const CORE::CString& tenantId );

    /** The tenant of personal Microsoft accounts */
    static const CORE::CString ConsumersTenantId;

    private:

    COAuth2Protocol( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_COAUTH2PROTOCOL_H ? */
