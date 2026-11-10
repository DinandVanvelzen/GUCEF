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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUTH_CAuthProviderRegistry.h"

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

const CORE::CString CAuthProviderRegistry::GoogleName = "google";
const CORE::CString CAuthProviderRegistry::MicrosoftName = "microsoft";
const CORE::CString CAuthProviderRegistry::FacebookName = "facebook";
const CORE::CString CAuthProviderRegistry::AmazonName = "amazon";
const CORE::CString CAuthProviderRegistry::AwsCognitoName = "aws-cognito";
const CORE::CString CAuthProviderRegistry::AwsIdentityCenterName = "aws-identitycenter";
const CORE::CString CAuthProviderRegistry::EpicName = "epic";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeGoogle( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::GoogleName;
    d.displayName = "Google";
    d.accountProviderId = CAuthProviderIds::Google;
    d.protocol = AUTHPROTOCOL_OIDC;
    d.idScope = AUTHIDSCOPE_GLOBAL;
    d.capabilities = AUTHCAP_BROWSER | AUTHCAP_DEVICE_CODE | AUTHCAP_SERVER_VERIFIABLE | AUTHCAP_REFRESHABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.discoveryUrl = "https://accounts.google.com/.well-known/openid-configuration";
    d.issuer = "https://accounts.google.com";
    d.authorizationEndpoint = "https://accounts.google.com/o/oauth2/v2/auth";
    d.tokenEndpoint = "https://oauth2.googleapis.com/token";
    d.deviceAuthorizationEndpoint = "https://oauth2.googleapis.com/device/code";
    d.userInfoEndpoint = "https://openidconnect.googleapis.com/v1/userinfo";
    d.jwksUri = "https://www.googleapis.com/oauth2/v3/certs";
    d.revocationEndpoint = "https://oauth2.googleapis.com/revoke";
    d.defaultScopes.push_back( "openid" );
    d.defaultScopes.push_back( "profile" );
    d.defaultScopes.push_back( "email" );
    d.subjectClaim = "sub";
    d.displayNameClaim = "name";
    d.avatarClaim = "picture";
    d.publicClient = true;
    d.usesPkce = true;
    d.clientSecretNeeded = true;
    d.authorizationParameters[ "access_type" ] = "offline";
    d.supportsLoginHint = true;
    d.loginHintParameter = "login_hint";
    d.loginHintClaims.push_back( "email" );
    d.notes = "Client type Desktop app: loopback redirects on 127.0.0.1 with any port are accepted. Google issues a client secret for it and the token endpoint asks for it, but it is not confidential. "
              "The device flow needs a client of the type TVs and Limited Input devices and allows the scopes openid, email and profile only. Games on Android use Play Games Services with a separate web client id (SDK plugin). "
              "The scope email gives the address of the account in the id token: it is what login_hint prefills at the next login.";
    return d;
}

/*-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeMicrosoft( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::MicrosoftName;
    d.displayName = "Microsoft";
    d.accountProviderId = CAuthProviderIds::Microsoft;
    d.protocol = AUTHPROTOCOL_OIDC;
    d.idScope = AUTHIDSCOPE_GLOBAL;
    d.capabilities = AUTHCAP_BROWSER | AUTHCAP_DEVICE_CODE | AUTHCAP_SERVER_VERIFIABLE | AUTHCAP_REFRESHABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.discoveryUrl = "https://login.microsoftonline.com/{tenant}/v2.0/.well-known/openid-configuration";
    d.issuer = "https://login.microsoftonline.com/{tid}/v2.0";
    d.authorizationEndpoint = "https://login.microsoftonline.com/{tenant}/oauth2/v2.0/authorize";
    d.tokenEndpoint = "https://login.microsoftonline.com/{tenant}/oauth2/v2.0/token";
    d.deviceAuthorizationEndpoint = "https://login.microsoftonline.com/{tenant}/oauth2/v2.0/devicecode";
    d.userInfoEndpoint = "https://graph.microsoft.com/oidc/userinfo";
    d.jwksUri = "https://login.microsoftonline.com/{tenant}/discovery/v2.0/keys";
    d.defaultScopes.push_back( "openid" );
    d.defaultScopes.push_back( "profile" );
    d.defaultScopes.push_back( "offline_access" );
    d.supportsLoginHint = true;
    d.loginHintParameter = "login_hint";
    d.loginHintClaims.push_back( "preferred_username" );
    d.loginHintClaims.push_back( "email" );
    d.subjectClaim = "oid";
    d.subjectTemplate = "{tid}:{oid}";
    d.displayNameClaim = "name";
    d.publicClient = true;
    d.usesPkce = true;
    d.defaultParameters[ "tenant" ] = "common";
    d.notes = "Register as a public client (mobile and desktop application) with the redirect URI http://localhost: the port is ignored when a loopback redirect is matched. "
              "The tenant is common (work, school and personal accounts), organizations, consumers or a tenant id; the issuer of a token names the tenant of the account, so it is checked as that tenant. "
              "The oid claim is the same in every app, personal accounts included, so the account reference is global. The id token carries no picture. "
              "login_hint is the preferred_username of the id token; domain_hint is not sent (add it to authorizationParameters of the descriptor, for example consumers, to skip the account type question).";
    return d;
}

/*-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeFacebook( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::FacebookName;
    d.displayName = "Facebook";
    d.accountProviderId = CAuthProviderIds::Facebook;
    d.protocol = AUTHPROTOCOL_OAUTH2;
    d.idScope = AUTHIDSCOPE_PER_APP;
    d.capabilities = AUTHCAP_BROWSER | AUTHCAP_DEVICE_CODE | AUTHCAP_SERVER_VERIFIABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.authorizationEndpoint = "https://www.facebook.com/{graphVersion}/dialog/oauth";
    d.tokenEndpoint = "https://graph.facebook.com/{graphVersion}/oauth/access_token";
    d.deviceAuthorizationEndpoint = "https://graph.facebook.com/{graphVersion}/device/login";
    d.userInfoEndpoint = "https://graph.facebook.com/{graphVersion}/me?fields=id,name,picture.type(large)";
    d.defaultScopes.push_back( "public_profile" );
    d.subjectClaim = "id";
    d.displayNameClaim = "name";
    d.avatarClaim = "picture.data.url";
    d.publicClient = true;
    d.usesPkce = false;
    d.defaultParameters[ "graphVersion" ] = "v21.0";
    d.tokenVerifyEndpoint = "https://graph.facebook.com/{graphVersion}/debug_token";
    d.tokenVerifyStyle = "facebook-debug-token";
    d.notes = "The user id is scoped to the app: it identifies the same person in two products only when they share one registered app (or the business ids of one Meta business). "
              "The redirect URI must match a registered one exactly, so a loopback login needs a fixed port (loopbackPort) registered with the app; the device flow needs the client token of the app as the parameter clientToken. "
              "A server verifies an access token with debug_token, which needs the app secret and so runs on a server only.";
    d.requiredParameters.push_back( "clientToken" );
    return d;
}

/*-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeAmazon( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::AmazonName;
    d.displayName = "Amazon";
    d.accountProviderId = CAuthProviderIds::Amazon;
    d.protocol = AUTHPROTOCOL_OAUTH2;
    d.idScope = AUTHIDSCOPE_PER_APP;
    d.capabilities = AUTHCAP_BROWSER | AUTHCAP_SERVER_VERIFIABLE | AUTHCAP_REFRESHABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.authorizationEndpoint = "https://www.amazon.com/ap/oa";
    d.tokenEndpoint = "https://api.amazon.com/auth/o2/token";
    d.userInfoEndpoint = "https://api.amazon.com/user/profile";
    d.tokenVerifyEndpoint = "https://api.amazon.com/auth/o2/tokeninfo";
    d.tokenVerifyStyle = "amazon-tokeninfo";
    d.defaultScopes.push_back( "profile" );
    d.subjectClaim = "user_id";
    d.displayNameClaim = "name";
    d.publicClient = true;
    d.usesPkce = true;
    d.notes = "Login with Amazon: the user id (amzn1.account....) comes from the profile endpoint. It is treated as scoped to the security profile until it is confirmed to be wider for the registered profile. "
              "A server verifies an access token by checking its audience with the tokeninfo endpoint before it asks for the profile, so a token issued to another app is refused. "
              "Allowed return URLs are registered with the security profile; check that a loopback address with a fixed port is accepted for it.";
    return d;
}

/*-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeAwsCognito( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::AwsCognitoName;
    d.displayName = "Amazon Cognito";
    d.accountProviderId = CAuthProviderIds::Aws;
    d.protocol = AUTHPROTOCOL_OIDC;
    d.idScope = AUTHIDSCOPE_PER_ORGANISATION;
    d.capabilities = AUTHCAP_BROWSER | AUTHCAP_SERVER_VERIFIABLE | AUTHCAP_REFRESHABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.discoveryUrl = "https://cognito-idp.{region}.amazonaws.com/{poolId}/.well-known/openid-configuration";
    d.issuer = "https://cognito-idp.{region}.amazonaws.com/{poolId}";
    d.authorizationEndpoint = "https://{poolDomain}/oauth2/authorize";
    d.tokenEndpoint = "https://{poolDomain}/oauth2/token";
    d.userInfoEndpoint = "https://{poolDomain}/oauth2/userInfo";
    d.jwksUri = "https://cognito-idp.{region}.amazonaws.com/{poolId}/.well-known/jwks.json";
    d.revocationEndpoint = "https://{poolDomain}/oauth2/revoke";
    d.defaultScopes.push_back( "openid" );
    d.defaultScopes.push_back( "profile" );
    d.supportsLoginHint = true;     // verify against the pool: the managed login of newer pools takes login_hint, the classic hosted UI ignores it
    d.loginHintParameter = "login_hint";
    d.loginHintClaims.push_back( "email" );
    d.loginHintClaims.push_back( "preferred_username" );
    d.subjectClaim = "sub";
    d.subjectTemplate = "{poolId}:{sub}";
    d.displayNameClaim = "name";
    d.avatarClaim = "picture";
    d.publicClient = true;
    d.usesPkce = true;
    d.requiredParameters.push_back( "region" );
    d.requiredParameters.push_back( "poolId" );
    d.requiredParameters.push_back( "poolDomain" );
    d.notes = "App client without a secret, authorization code grant with PKCE. region is for example eu-west-1, poolId the id of the user pool (eu-west-1_AbCdEfGhI) and poolDomain the host of its hosted UI domain without scheme. "
              "Callback URLs are registered and matched exactly (http is allowed for localhost only), so the loopback redirect needs a fixed port (loopbackPort). The hosted UI has no device flow. "
              "The sub claim is unique within the user pool, the account reference names the pool as well. Identity providers federated into the pool (Google, Apple, SAML) appear as accounts of the pool. "
              "login_hint is accepted by the managed login of newer pools and ignored by the classic hosted UI: verify against the pool. The email claim needs the scope email on the app client.";
    return d;
}

/*-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeAwsIdentityCenter( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::AwsIdentityCenterName;
    d.displayName = "AWS IAM Identity Center";
    d.accountProviderId = CAuthProviderIds::Aws;
    d.protocol = AUTHPROTOCOL_OAUTH2;
    d.idScope = AUTHIDSCOPE_PER_ORGANISATION;
    d.capabilities = AUTHCAP_DEVICE_CODE | AUTHCAP_BROWSER | AUTHCAP_REFRESHABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.authorizationEndpoint = "https://oidc.{region}.amazonaws.com/authorize";
    d.tokenEndpoint = "https://oidc.{region}.amazonaws.com/token";
    d.deviceAuthorizationEndpoint = "https://oidc.{region}.amazonaws.com/device_authorization";
    d.subjectClaim = "userId";
    d.subjectTemplate = "{instanceId}:{userId}";
    d.displayNameClaim = "name";
    d.publicClient = true;
    d.usesPkce = true;
    d.jsonRequestBodies = true;
    d.requiredParameters.push_back( "region" );
    d.requiredParameters.push_back( "instanceId" );
    d.notes = "The OIDC service of IAM Identity Center takes JSON bodies. The client id (and secret) come from registering a client with the service in the region of the instance; instanceId is the identity store id (d-...) and the userId the user id in that store. "
              "Confirm against a real instance which token carries the user id and whether the instance issues id tokens; if not, the user id is read from the identity store by a server and the account reference is made there.";
    return d;
}

/*-------------------------------------------------------------------------*/

static CAuthProviderDescriptor
MakeEpic( void )
{GUCEF_TRACE;

    CAuthProviderDescriptor d;
    d.name = CAuthProviderRegistry::EpicName;
    d.displayName = "Epic Games";
    d.accountProviderId = CAuthProviderIds::Epic;
    d.protocol = AUTHPROTOCOL_OIDC;
    d.idScope = AUTHIDSCOPE_GLOBAL;
    d.capabilities = AUTHCAP_BROWSER | AUTHCAP_SERVER_VERIFIABLE | AUTHCAP_REFRESHABLE | AUTHCAP_REQUIRES_CLIENT_ID;
    d.issuer = "https://api.epicgames.dev/epic/oauth/v1";
    d.authorizationEndpoint = "https://www.epicgames.com/id/authorize";
    d.tokenEndpoint = "https://api.epicgames.dev/epic/oauth/v1/token";
    d.userInfoEndpoint = "https://api.epicgames.dev/epic/oauth/v1/userInfo";
    d.jwksUri = "https://api.epicgames.dev/epic/oauth/v1/.well-known/jwks.json";
    d.revocationEndpoint = "https://api.epicgames.dev/epic/oauth/v1/revoke";
    d.defaultScopes.push_back( "basic_profile" );
    d.subjectClaim = "sub";
    d.displayNameClaim = "preferred_username";
    d.publicClient = false;
    d.usesPkce = true;
    d.clientSecretNeeded = true;
    d.clientAuthMethod = "basic";
    d.notes = "Epic Account Services: the sub claim is the Epic account id (32 hex digits), the same in every product. The Product User Id of EOS Connect is scoped to the product and is not used for the account reference. "
              "Epic issues a client id with a client secret for the product; the secret is kept out of shipped games where the product allows it. "
              "Verify the jwks and userInfo paths and the display name claim against the registered product before use.";
    return d;
}

/*-------------------------------------------------------------------------*/

CAuthProviderRegistry::CAuthProviderRegistry( void )
    : CORE::CIConfigurable()
    , m_lock()
    , m_descriptors()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderRegistry::~CAuthProviderRegistry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAuthProviderRegistry::RegisterBuiltIns( void )
{GUCEF_TRACE;

    RegisterDescriptor( MakeGoogle() );
    RegisterDescriptor( MakeMicrosoft() );
    RegisterDescriptor( MakeFacebook() );
    RegisterDescriptor( MakeAmazon() );
    RegisterDescriptor( MakeAwsCognito() );
    RegisterDescriptor( MakeAwsIdentityCenter() );
    RegisterDescriptor( MakeEpic() );
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderRegistry::RegisterDescriptor( const CAuthProviderDescriptor& descriptor )
{GUCEF_TRACE;

    if ( !descriptor.IsWellFormed() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AuthProviderRegistry: The descriptor \"" << descriptor.name << "\" is not well formed" );
        return false;
    }
    MT::CScopeMutex lock( m_lock );
    m_descriptors[ descriptor.name ] = descriptor;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderRegistry::UnregisterDescriptor( const CORE::CString& name )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return 0 != m_descriptors.erase( name );
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderRegistry::GetDescriptor( const CORE::CString& name, CAuthProviderDescriptor& descriptor ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDescriptorMap::const_iterator i = m_descriptors.find( name );
    if ( i == m_descriptors.end() )
        return false;
    descriptor = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderRegistry::HasDescriptor( const CORE::CString& name ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_descriptors.find( name ) != m_descriptors.end();
}

/*-------------------------------------------------------------------------*/

void
CAuthProviderRegistry::GetNames( CORE::CString::StringVector& names ) const
{GUCEF_TRACE;

    names.clear();
    MT::CScopeMutex lock( m_lock );
    TDescriptorMap::const_iterator i = m_descriptors.begin();
    while ( i != m_descriptors.end() )
    {
        names.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAuthProviderRegistry::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_descriptors.size();
}

/*-------------------------------------------------------------------------*/

void
CAuthProviderRegistry::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_descriptors.clear();
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderRegistry::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TDescriptorMap::const_iterator i = m_descriptors.begin();
    while ( i != m_descriptors.end() )
    {
        CORE::CDataNode* child = config.AddChild( (*i).first, GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != child )
            (*i).second.Serialize( *child, CORE::CDataNodeSerializableSettings() );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderRegistry::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    bool allLoaded = true;
    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        CAuthProviderDescriptor descriptor;
        if ( !descriptor.Deserialize( *(*i), CORE::CDataNodeSerializableSettings() ) || !RegisterDescriptor( descriptor ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AuthProviderRegistry: The descriptor \"" << (*i)->GetName() << "\" in the configuration is not usable" );
            allLoaded = false;
        }
        ++i;
    }
    return allLoaded;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthProviderRegistry::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthProviderRegistry";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
