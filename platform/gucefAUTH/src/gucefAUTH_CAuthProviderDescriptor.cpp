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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAuthProviderDescriptor.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SCapabilityName
{
    UInt32 flag;
    const char* name;
};

static const SCapabilityName g_capabilityNames[] =
{
    { AUTHCAP_BROWSER, "browser" },
    { AUTHCAP_DEVICE_CODE, "device-code" },
    { AUTHCAP_SILENT, "silent" },
    { AUTHCAP_SERVER_VERIFIABLE, "server-verifiable" },
    { AUTHCAP_REFRESHABLE, "refreshable" },
    { AUTHCAP_REQUIRES_CLIENT_ID, "requires-client-id" }
};

/*-------------------------------------------------------------------------*/

static const char*
ProtocolToText( TAuthProtocol protocol )
{GUCEF_TRACE;

    switch ( protocol )
    {
        case AUTHPROTOCOL_OAUTH2 : return "oauth2";
        case AUTHPROTOCOL_OPENID2 : return "openid2";
        case AUTHPROTOCOL_PLATFORM : return "platform";
        default : return "oidc";
    }
}

/*-------------------------------------------------------------------------*/

static TAuthProtocol
TextToProtocol( const CORE::CString& text, TAuthProtocol defaultProtocol )
{GUCEF_TRACE;

    CORE::CString lower = text.Lowercase();
    if ( "oidc" == lower )
        return AUTHPROTOCOL_OIDC;
    if ( "oauth2" == lower )
        return AUTHPROTOCOL_OAUTH2;
    if ( "openid2" == lower )
        return AUTHPROTOCOL_OPENID2;
    if ( "platform" == lower )
        return AUTHPROTOCOL_PLATFORM;
    return defaultProtocol;
}

/*-------------------------------------------------------------------------*/

static const char*
IdScopeToText( TAuthIdScope scope )
{GUCEF_TRACE;

    switch ( scope )
    {
        case AUTHIDSCOPE_PER_ORGANISATION : return "organisation";
        case AUTHIDSCOPE_PER_APP : return "app";
        default : return "global";
    }
}

/*-------------------------------------------------------------------------*/

static TAuthIdScope
TextToIdScope( const CORE::CString& text, TAuthIdScope defaultScope )
{GUCEF_TRACE;

    CORE::CString lower = text.Lowercase();
    if ( "global" == lower )
        return AUTHIDSCOPE_GLOBAL;
    if ( "organisation" == lower )
        return AUTHIDSCOPE_PER_ORGANISATION;
    if ( "app" == lower )
        return AUTHIDSCOPE_PER_APP;
    return defaultScope;
}

/*-------------------------------------------------------------------------*/

CAuthProviderDescriptor::CAuthProviderDescriptor( void )
    : CORE::CIDataNodeSerializable()
    , name()
    , displayName()
    , accountProviderId()
    , protocol( AUTHPROTOCOL_OIDC )
    , idScope( AUTHIDSCOPE_GLOBAL )
    , capabilities( AUTHCAP_NONE )
    , discoveryUrl()
    , issuer()
    , authorizationEndpoint()
    , tokenEndpoint()
    , deviceAuthorizationEndpoint()
    , userInfoEndpoint()
    , jwksUri()
    , revocationEndpoint()
    , defaultScopes()
    , subjectClaim( "sub" )
    , subjectTemplate()
    , displayNameClaim()
    , avatarClaim()
    , publicClient( true )
    , usesPkce( true )
    , clientSecretNeeded( false )
    , jsonRequestBodies( false )
    , clientAuthMethod( "post" )
    , tokenVerifyEndpoint()
    , tokenVerifyStyle()
    , requiredParameters()
    , defaultParameters()
    , authorizationParameters()
    , supportsLoginHint( false )
    , loginHintParameter( "login_hint" )
    , loginHintClaims()
    , formerNames()
    , notes()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderDescriptor::CAuthProviderDescriptor( const CAuthProviderDescriptor& src )
    : CORE::CIDataNodeSerializable( src )
    , name( src.name )
    , displayName( src.displayName )
    , accountProviderId( src.accountProviderId )
    , protocol( src.protocol )
    , idScope( src.idScope )
    , capabilities( src.capabilities )
    , discoveryUrl( src.discoveryUrl )
    , issuer( src.issuer )
    , authorizationEndpoint( src.authorizationEndpoint )
    , tokenEndpoint( src.tokenEndpoint )
    , deviceAuthorizationEndpoint( src.deviceAuthorizationEndpoint )
    , userInfoEndpoint( src.userInfoEndpoint )
    , jwksUri( src.jwksUri )
    , revocationEndpoint( src.revocationEndpoint )
    , defaultScopes( src.defaultScopes )
    , subjectClaim( src.subjectClaim )
    , subjectTemplate( src.subjectTemplate )
    , displayNameClaim( src.displayNameClaim )
    , avatarClaim( src.avatarClaim )
    , publicClient( src.publicClient )
    , usesPkce( src.usesPkce )
    , clientSecretNeeded( src.clientSecretNeeded )
    , jsonRequestBodies( src.jsonRequestBodies )
    , clientAuthMethod( src.clientAuthMethod )
    , tokenVerifyEndpoint( src.tokenVerifyEndpoint )
    , tokenVerifyStyle( src.tokenVerifyStyle )
    , requiredParameters( src.requiredParameters )
    , defaultParameters( src.defaultParameters )
    , authorizationParameters( src.authorizationParameters )
    , supportsLoginHint( src.supportsLoginHint )
    , loginHintParameter( src.loginHintParameter )
    , loginHintClaims( src.loginHintClaims )
    , formerNames( src.formerNames )
    , notes( src.notes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderDescriptor::~CAuthProviderDescriptor()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderDescriptor&
CAuthProviderDescriptor::operator=( const CAuthProviderDescriptor& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        name = src.name;
        displayName = src.displayName;
        accountProviderId = src.accountProviderId;
        protocol = src.protocol;
        idScope = src.idScope;
        capabilities = src.capabilities;
        discoveryUrl = src.discoveryUrl;
        issuer = src.issuer;
        authorizationEndpoint = src.authorizationEndpoint;
        tokenEndpoint = src.tokenEndpoint;
        deviceAuthorizationEndpoint = src.deviceAuthorizationEndpoint;
        userInfoEndpoint = src.userInfoEndpoint;
        jwksUri = src.jwksUri;
        revocationEndpoint = src.revocationEndpoint;
        defaultScopes = src.defaultScopes;
        subjectClaim = src.subjectClaim;
        subjectTemplate = src.subjectTemplate;
        displayNameClaim = src.displayNameClaim;
        avatarClaim = src.avatarClaim;
        publicClient = src.publicClient;
        usesPkce = src.usesPkce;
        clientSecretNeeded = src.clientSecretNeeded;
        jsonRequestBodies = src.jsonRequestBodies;
        clientAuthMethod = src.clientAuthMethod;
        tokenVerifyEndpoint = src.tokenVerifyEndpoint;
        tokenVerifyStyle = src.tokenVerifyStyle;
        requiredParameters = src.requiredParameters;
        defaultParameters = src.defaultParameters;
        authorizationParameters = src.authorizationParameters;
        supportsLoginHint = src.supportsLoginHint;
        loginHintParameter = src.loginHintParameter;
        loginHintClaims = src.loginHintClaims;
        formerNames = src.formerNames;
        notes = src.notes;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderDescriptor::IsWellFormed( void ) const
{GUCEF_TRACE;

    if ( name.IsNULLOrEmpty() || displayName.IsNULLOrEmpty() || !CAccountRef::IsValidProvider( accountProviderId ) )
        return false;
    if ( AUTHPROTOCOL_PLATFORM == protocol )
        return true;
    if ( AUTHPROTOCOL_OPENID2 == protocol )
        return !authorizationEndpoint.IsNULLOrEmpty();
    return !discoveryUrl.IsNULLOrEmpty() ||
           ( !tokenEndpoint.IsNULLOrEmpty() && ( !authorizationEndpoint.IsNULLOrEmpty() || !deviceAuthorizationEndpoint.IsNULLOrEmpty() ) );
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderDescriptor::ExpandTemplate( const CORE::CString& text                  ,
                                         const CORE::CString::StringMap& primary    ,
                                         const CORE::CString::StringMap* fallback   ,
                                         bool encodeValues                          ,
                                         CORE::CString& expanded                    )
{GUCEF_TRACE;

    expanded.Clear();
    CORE::CString result;
    const char* chars = text.C_String();
    UInt32 length = GetByteLength( text );
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( '}' == chars[ i ] )
            return false;
        if ( '{' != chars[ i ] )
        {
            result += chars[ i ];
            continue;
        }
        UInt32 end = i + 1;
        while ( end < length && '}' != chars[ end ] )
            ++end;
        if ( end >= length || end == i + 1 )
            return false;
        CORE::CString key( chars + i + 1, end - i - 1 );
        CORE::CString::StringMap::const_iterator found = primary.find( key );
        CORE::CString value;
        if ( found != primary.end() )
        {
            value = (*found).second;
        }
        else
        if ( GUCEF_NULL != fallback && fallback->find( key ) != fallback->end() )
        {
            value = (*fallback->find( key )).second;
        }
        else
        {
            return false;
        }
        if ( value.IsNULLOrEmpty() )
            return false;
        result += encodeValues ? CAccountRef::EncodeSubject( value ) : value;
        i = end;
    }
    expanded = result;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthProviderDescriptor::GetTemplateNames( const CORE::CString& text, CORE::CString::StringVector& names )
{GUCEF_TRACE;

    names.clear();
    const char* chars = text.C_String();
    UInt32 length = GetByteLength( text );
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( '{' != chars[ i ] )
            continue;
        UInt32 end = i + 1;
        while ( end < length && '}' != chars[ end ] )
            ++end;
        if ( end >= length )
            return;
        if ( end > i + 1 )
            names.push_back( CORE::CString( chars + i + 1, end - i - 1 ) );
        i = end;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderDescriptor::ComposeSubject( const CORE::CString::StringMap& claims     ,
                                         const CORE::CString::StringMap& parameters ,
                                         CORE::CString& subject                     ) const
{GUCEF_TRACE;

    subject.Clear();
    CORE::CString templateText = subjectTemplate;
    if ( templateText.IsNULLOrEmpty() )
    {
        if ( subjectClaim.IsNULLOrEmpty() )
            return false;
        templateText = "{";
        templateText += subjectClaim;
        templateText += '}';
    }
    return ExpandTemplate( templateText, claims, &parameters, true, subject );
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderDescriptor::MakeAccountRef( const CORE::CString::StringMap& claims     ,
                                         const CORE::CString::StringMap& parameters ,
                                         CAccountRef& accountRef                    ) const
{GUCEF_TRACE;

    accountRef = CAccountRef();
    CORE::CString subject;
    if ( !ComposeSubject( claims, parameters, subject ) )
        return false;
    CAccountRef made( accountProviderId, subject );
    if ( !made.IsValid() )
        return false;
    accountRef = made;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderDescriptor::Serialize( CORE::CDataNode& domRootNode                        ,
                                    const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "name", name );
    domRootNode.SetAttribute( "displayName", displayName );
    domRootNode.SetAttribute( "accountProviderId", accountProviderId );
    domRootNode.SetAttribute( "protocol", CORE::CString( ProtocolToText( protocol ) ) );
    domRootNode.SetAttribute( "idScope", CORE::CString( IdScopeToText( idScope ) ) );
    CORE::CString::StringVector capabilityNames;
    for ( size_t i=0; i<sizeof( g_capabilityNames ) / sizeof( g_capabilityNames[ 0 ] ); ++i )
        if ( 0 != ( capabilities & g_capabilityNames[ i ].flag ) )
            capabilityNames.push_back( g_capabilityNames[ i ].name );
    domRootNode.SetValuesOfChildByName( "capabilities", capabilityNames );
    domRootNode.SetAttribute( "discoveryUrl", discoveryUrl );
    domRootNode.SetAttribute( "issuer", issuer );
    domRootNode.SetAttribute( "authorizationEndpoint", authorizationEndpoint );
    domRootNode.SetAttribute( "tokenEndpoint", tokenEndpoint );
    domRootNode.SetAttribute( "deviceAuthorizationEndpoint", deviceAuthorizationEndpoint );
    domRootNode.SetAttribute( "userInfoEndpoint", userInfoEndpoint );
    domRootNode.SetAttribute( "jwksUri", jwksUri );
    domRootNode.SetAttribute( "revocationEndpoint", revocationEndpoint );
    domRootNode.SetValuesOfChildByName( "defaultScopes", defaultScopes );
    domRootNode.SetAttribute( "subjectClaim", subjectClaim );
    domRootNode.SetAttribute( "subjectTemplate", subjectTemplate );
    domRootNode.SetAttribute( "displayNameClaim", displayNameClaim );
    domRootNode.SetAttribute( "avatarClaim", avatarClaim );
    domRootNode.SetAttribute( "publicClient", publicClient );
    domRootNode.SetAttribute( "usesPkce", usesPkce );
    domRootNode.SetAttribute( "clientSecretNeeded", clientSecretNeeded );
    domRootNode.SetAttribute( "jsonRequestBodies", jsonRequestBodies );
    domRootNode.SetAttribute( "clientAuthMethod", clientAuthMethod );
    domRootNode.SetAttribute( "tokenVerifyEndpoint", tokenVerifyEndpoint );
    domRootNode.SetAttribute( "tokenVerifyStyle", tokenVerifyStyle );
    domRootNode.SetValuesOfChildByName( "requiredParameters", requiredParameters );
    CORE::CDataNode* defaults = domRootNode.AddChild( "defaultParameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != defaults )
    {
        CORE::CString::StringMap::const_iterator i = defaultParameters.begin();
        while ( i != defaultParameters.end() )
        {
            defaults->SetAttribute( (*i).first, (*i).second );
            ++i;
        }
    }
    CORE::CDataNode* authParams = domRootNode.AddChild( "authorizationParameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != authParams )
    {
        CORE::CString::StringMap::const_iterator a = authorizationParameters.begin();
        while ( a != authorizationParameters.end() )
        {
            authParams->SetAttribute( (*a).first, (*a).second );
            ++a;
        }
    }
    domRootNode.SetAttribute( "supportsLoginHint", supportsLoginHint );
    domRootNode.SetAttribute( "loginHintParameter", loginHintParameter );
    domRootNode.SetValuesOfChildByName( "loginHintClaims", loginHintClaims );
    domRootNode.SetValuesOfChildByName( "formerNames", formerNames );
    domRootNode.SetAttribute( "notes", notes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderDescriptor::Deserialize( const CORE::CDataNode& domRootNode                  ,
                                      const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    CAuthProviderDescriptor loaded;
    loaded.name = domRootNode.GetAttributeValueOrChildValueByName( "name" ).AsString( domRootNode.GetName(), true );
    loaded.displayName = domRootNode.GetAttributeValueOrChildValueByName( "displayName" ).AsString( loaded.name, true );
    loaded.accountProviderId = domRootNode.GetAttributeValueOrChildValueByName( "accountProviderId" ).AsString( loaded.name, true );
    loaded.protocol = TextToProtocol( domRootNode.GetAttributeValueOrChildValueByName( "protocol" ).AsString( "oidc", true ), AUTHPROTOCOL_OIDC );
    loaded.idScope = TextToIdScope( domRootNode.GetAttributeValueOrChildValueByName( "idScope" ).AsString( "global", true ), AUTHIDSCOPE_GLOBAL );
    CORE::CString::StringVector capabilityNames;
    domRootNode.GetValuesOfChildByName( "capabilities", capabilityNames );
    for ( size_t n=0; n<capabilityNames.size(); ++n )
        for ( size_t i=0; i<sizeof( g_capabilityNames ) / sizeof( g_capabilityNames[ 0 ] ); ++i )
            if ( capabilityNames[ n ] == g_capabilityNames[ i ].name )
                loaded.capabilities |= g_capabilityNames[ i ].flag;
    loaded.discoveryUrl = domRootNode.GetAttributeValueOrChildValueByName( "discoveryUrl" ).AsString( CORE::CString::Empty, true );
    loaded.issuer = domRootNode.GetAttributeValueOrChildValueByName( "issuer" ).AsString( CORE::CString::Empty, true );
    loaded.authorizationEndpoint = domRootNode.GetAttributeValueOrChildValueByName( "authorizationEndpoint" ).AsString( CORE::CString::Empty, true );
    loaded.tokenEndpoint = domRootNode.GetAttributeValueOrChildValueByName( "tokenEndpoint" ).AsString( CORE::CString::Empty, true );
    loaded.deviceAuthorizationEndpoint = domRootNode.GetAttributeValueOrChildValueByName( "deviceAuthorizationEndpoint" ).AsString( CORE::CString::Empty, true );
    loaded.userInfoEndpoint = domRootNode.GetAttributeValueOrChildValueByName( "userInfoEndpoint" ).AsString( CORE::CString::Empty, true );
    loaded.jwksUri = domRootNode.GetAttributeValueOrChildValueByName( "jwksUri" ).AsString( CORE::CString::Empty, true );
    loaded.revocationEndpoint = domRootNode.GetAttributeValueOrChildValueByName( "revocationEndpoint" ).AsString( CORE::CString::Empty, true );
    domRootNode.GetValuesOfChildByName( "defaultScopes", loaded.defaultScopes );
    loaded.subjectClaim = domRootNode.GetAttributeValueOrChildValueByName( "subjectClaim" ).AsString( "sub", true );
    loaded.subjectTemplate = domRootNode.GetAttributeValueOrChildValueByName( "subjectTemplate" ).AsString( CORE::CString::Empty, true );
    loaded.displayNameClaim = domRootNode.GetAttributeValueOrChildValueByName( "displayNameClaim" ).AsString( CORE::CString::Empty, true );
    loaded.avatarClaim = domRootNode.GetAttributeValueOrChildValueByName( "avatarClaim" ).AsString( CORE::CString::Empty, true );
    loaded.publicClient = domRootNode.GetAttributeValueOrChildValueByName( "publicClient" ).AsBool( true, true );
    loaded.usesPkce = domRootNode.GetAttributeValueOrChildValueByName( "usesPkce" ).AsBool( true, true );
    loaded.clientSecretNeeded = domRootNode.GetAttributeValueOrChildValueByName( "clientSecretNeeded" ).AsBool( false, true );
    loaded.jsonRequestBodies = domRootNode.GetAttributeValueOrChildValueByName( "jsonRequestBodies" ).AsBool( false, true );
    loaded.clientAuthMethod = domRootNode.GetAttributeValueOrChildValueByName( "clientAuthMethod" ).AsString( "post", true );
    loaded.tokenVerifyEndpoint = domRootNode.GetAttributeValueOrChildValueByName( "tokenVerifyEndpoint" ).AsString( CORE::CString::Empty, true );
    loaded.tokenVerifyStyle = domRootNode.GetAttributeValueOrChildValueByName( "tokenVerifyStyle" ).AsString( CORE::CString::Empty, true );
    domRootNode.GetValuesOfChildByName( "requiredParameters", loaded.requiredParameters );
    const CORE::CDataNode* defaults = domRootNode.FindChild( "defaultParameters" );
    if ( GUCEF_NULL != defaults )
    {
        CORE::CDataNode::TAttributeMap::const_iterator i = defaults->AttributeBegin();
        while ( i != defaults->AttributeEnd() )
        {
            loaded.defaultParameters[ (*i).first ] = (*i).second.AsString();
            ++i;
        }
    }
    const CORE::CDataNode* authParams = domRootNode.FindChild( "authorizationParameters" );
    if ( GUCEF_NULL != authParams )
    {
        CORE::CDataNode::TAttributeMap::const_iterator a = authParams->AttributeBegin();
        while ( a != authParams->AttributeEnd() )
        {
            loaded.authorizationParameters[ (*a).first ] = (*a).second.AsString();
            ++a;
        }
    }
    loaded.supportsLoginHint = domRootNode.GetAttributeValueOrChildValueByName( "supportsLoginHint" ).AsBool( false, true );
    loaded.loginHintParameter = domRootNode.GetAttributeValueOrChildValueByName( "loginHintParameter" ).AsString( "login_hint", true );
    domRootNode.GetValuesOfChildByName( "loginHintClaims", loaded.loginHintClaims );
    domRootNode.GetValuesOfChildByName( "formerNames", loaded.formerNames );
    loaded.notes = domRootNode.GetAttributeValueOrChildValueByName( "notes" ).AsString( CORE::CString::Empty, true );
    if ( !loaded.IsWellFormed() )
        return false;
    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthProviderDescriptor::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthProviderDescriptor( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthProviderDescriptor::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthProviderDescriptor";
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
