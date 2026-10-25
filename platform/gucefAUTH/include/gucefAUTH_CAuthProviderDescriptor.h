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

#ifndef GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H
#define GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_AUTH_CACCOUNTREF_H
#include "gucefAUTH_CAccountRef.h"
#define GUCEF_AUTH_CACCOUNTREF_H
#endif /* GUCEF_AUTH_CACCOUNTREF_H ? */

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

/** The protocol family a descriptor driven provider speaks */
enum EAuthProtocol
{
    AUTHPROTOCOL_OIDC = 0,          /**< OpenID Connect: discovery, id token with signature, authorization code with PKCE or device code */
    AUTHPROTOCOL_OAUTH2 = 1,        /**< OAuth 2.0 without id token: the account follows from a profile request with the access token */
    AUTHPROTOCOL_OPENID2 = 2,       /**< OpenID 2.0: a positive assertion checked at the provider with check_authentication; needs the profile of the provider (IOpenId2ProviderProfile), which a plugin registers */
    AUTHPROTOCOL_PLATFORM = 3       /**< a session of the platform or an SDK, no endpoints: the provider is a plugin */
};

typedef enum EAuthProtocol TAuthProtocol;

/*-------------------------------------------------------------------------*/

/**
 *  What is known about an identity provider, as data: where its endpoints are, which claims hold the account, how the
 *  subject of the account reference is made, how far its ids reach and what a registered app has to supply. The
 *  engine that speaks OAuth 2.0 / OIDC (the next phase of the module) works from these records; adding a provider
 *  that follows the standards is adding a record, in code or in the configuration.
 *
 *  Endpoint values and the issuer may hold placeholders in braces, {tenant}, {region}, {poolId}, {poolDomain},
 *  {instanceId}, {graphVersion}: the values the application supplies in the Parameters of the provider's settings
 *  (requiredParameters names them, defaultParameters holds the defaults). The issuer may also name a claim of the
 *  token, {tid}, which the engine fills in from the token it validates.
 *
 *  subjectTemplate makes the subject of the account reference from claims and parameters, for example "{tid}:{oid}";
 *  empty means the value of subjectClaim. The values are encoded with CAccountRef::EncodeSubject() one by one.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthProviderDescriptor : public CORE::CIDataNodeSerializable
{
    public:

    CORE::CString name;                         /**< the provider name: registry key, factory key and key in the settings */
    CORE::CString displayName;                  /**< for a "Log in with ..." button */
    CORE::CString accountProviderId;            /**< the namespace of the account references it makes, the provider part */
    TAuthProtocol protocol;
    TAuthIdScope idScope;
    UInt32 capabilities;                        /**< EAuthProviderCaps flags */

    CORE::CString discoveryUrl;                 /**< OIDC discovery document; the endpoints below are used when it is empty and override what it says when set */
    CORE::CString issuer;
    CORE::CString authorizationEndpoint;
    CORE::CString tokenEndpoint;
    CORE::CString deviceAuthorizationEndpoint;
    CORE::CString userInfoEndpoint;
    CORE::CString jwksUri;
    CORE::CString revocationEndpoint;

    CORE::CString::StringVector defaultScopes;

    CORE::CString subjectClaim;                 /**< "sub" */
    CORE::CString subjectTemplate;
    CORE::CString displayNameClaim;
    CORE::CString avatarClaim;

    bool publicClient;                          /**< no confidential secret in the application */
    bool usesPkce;                              /**< authorization code with PKCE S256 */
    bool clientSecretNeeded;                    /**< the token endpoint wants a client secret even from an installed application (not confidential) */
    bool jsonRequestBodies;                     /**< the endpoints take JSON bodies and not forms */
    CORE::CString clientAuthMethod;             /**< how a client secret goes to the token endpoint: "post" (in the body, the default) or "basic" (HTTP Basic) */
    CORE::CString tokenVerifyEndpoint;          /**< where a server checks an access token that is the proof of a login that has no id token; may hold placeholders */
    CORE::CString tokenVerifyStyle;             /**< "facebook-debug-token" or "amazon-tokeninfo": how that check is made and read; empty when the proof is an id token */

    CORE::CString::StringVector requiredParameters; /**< what the application has to supply besides the client id */
    CORE::CString::StringMap defaultParameters;
    CORE::CString::StringMap authorizationParameters;   /**< parameters the provider wants on every authorization request besides the standard ones, for example access_type=offline */
    bool supportsLoginHint;                     /**< the authorization endpoint takes a parameter that prefills the account name; false for the providers that have none (nothing unsupported is ever sent) */
    CORE::CString loginHintParameter;           /**< the name of that parameter, "login_hint" in OpenID Connect */
    CORE::CString::StringVector loginHintClaims;    /**< the claims of a verified login, first one found wins, that name the account at the provider for the next login: email, preferred_username */
    CORE::CString::StringVector formerNames;    /**< earlier names of the provider: settings of an application that still use one are read as those of this provider (CAuthManager applies CAuthSettings::RenameProvider() for each) */
    CORE::CString notes;                        /**< what to know when registering an app at the provider */

    CAuthProviderDescriptor( void );

    CAuthProviderDescriptor( const CAuthProviderDescriptor& src );

    virtual ~CAuthProviderDescriptor();

    CAuthProviderDescriptor& operator=( const CAuthProviderDescriptor& src );

    /** A name, a display name and a valid account provider id, and for the protocols that have endpoints a discovery url or a token endpoint with an authorization or device authorization endpoint (the authorization endpoint alone for OpenID 2.0) */
    bool IsWellFormed( void ) const;

    /**
     *  Replaces every {name} in the text by its value, from the primary map and then the fallback map. False when a name
     *  has no value or a brace is not closed; expanded is empty then. With encodeValues each value goes through
     *  CAccountRef::EncodeSubject().
     */
    static bool ExpandTemplate( const CORE::CString& text                   ,
                                const CORE::CString::StringMap& primary     ,
                                const CORE::CString::StringMap* fallback    ,
                                bool encodeValues                           ,
                                CORE::CString& expanded                     );

    /** The names between braces in the text, in order */
    static void GetTemplateNames( const CORE::CString& text, CORE::CString::StringVector& names );

    /** The subject of the account reference from the claims of a token (or profile) and the parameters of the settings */
    bool ComposeSubject( const CORE::CString::StringMap& claims        ,
                         const CORE::CString::StringMap& parameters    ,
                         CORE::CString& subject                        ) const;

    /** The account reference of the claims; false when a claim is missing or the subject is not a valid one */
    bool MakeAccountRef( const CORE::CString::StringMap& claims        ,
                         const CORE::CString::StringMap& parameters    ,
                         CAccountRef& accountRef                       ) const;

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHPROVIDERDESCRIPTOR_H ? */
