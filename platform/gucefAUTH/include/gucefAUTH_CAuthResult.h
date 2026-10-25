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

#ifndef GUCEF_AUTH_CAUTHRESULT_H
#define GUCEF_AUTH_CAUTHRESULT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

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

/**
 *  What a third party can check to know that a login happened: an OIDC id token, an OpenID 2.0 assertion, a platform
 *  ticket. The kind says how to check it, the data is the proof as the provider gave it (a compact JWT, a query
 *  string, a base64 ticket). A proof is a credential until it expires, so it is treated like a secret: not logged,
 *  not serialized.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthProof
{
    public:

    static const CORE::CString KindOidcIdToken;         /**< "oidc-id-token": data is a compact JWT */
    static const CORE::CString KindOpenId2Assertion;    /**< "openid2-assertion": data is the query string of the positive assertion */
    static const CORE::CString KindOAuth2AccessToken;   /**< "oauth2-access-token": data is an access token that the provider's own service validates (Facebook debug_token, Amazon tokeninfo) */
    static const CORE::CString KindPlatformTicket;      /**< "platform-ticket": data is a ticket the platform's own service validates */

    CORE::CString kind;
    CSecretString data;

    CAuthProof( void );

    CAuthProof( const CORE::CString& proofKind, const CORE::CString& proofData );

    bool IsEmpty( void ) const;

    /** The SHA-256 of kind, a ':' and the data as 64 hex digits; empty for an empty proof. What link records carry in place of the proof */
    CORE::CString GetDigestHex( void ) const;

    void Clear( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  The outcome of a login or a refresh: who logged in and how to stay logged in. The tokens are secrets that do not
 *  show up in log streams (CSecretString); Serialize() leaves tokens and proof out, they are only stored through an
 *  ICredentialStore.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthResult : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString providerName;             /**< the login mechanism that produced it, for example "google" or "aws-cognito" */
    CAccountRef accountRef;                 /**< who logged in */
    CORE::CString displayName;
    CORE::CString avatarUrl;
    CORE::CString loginHint;                /**< the name of the account at the provider (an e-mail address) to prefill at the next login, from the verified claims; empty for the providers that take none. Personal data: not serialized and never logged */
    CSecretString accessToken;
    CSecretString idToken;
    CSecretString refreshToken;
    UInt64 expiresAtMs;                     /**< of the access token, milliseconds since 1970-01-01 UTC; 0 when it is not known or does not expire */
    CORE::CString::StringVector scopes;     /**< what the provider granted */
    CAuthProof proof;

    CAuthResult( void );

    CAuthResult( const CAuthResult& src );

    virtual ~CAuthResult();

    CAuthResult& operator=( const CAuthResult& src );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see CORE::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

    /** Whether it names a valid account */
    bool IsValid( void ) const;

    /** Whether the access token is past its expiry, with a margin for clock differences and the time a request takes */
    bool IsExpired( UInt64 nowEpochMs, UInt32 skewMs = 0 ) const;

    /** Overwrites the tokens and the proof */
    void WipeSecrets( void );

    /**
     *  The result a finished future carries; false when the future did not finish with a result.
     *  The futures of a login of this module (CAuthManager, CIAuthProvider) finish with a CAuthResult; a future that
     *  finished with other data gives false (checked by class name, no RTTI needed).
     */
    static bool TryGetFromFuture( const CORE::CFutureResult& future, CAuthResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHRESULT_H ? */
