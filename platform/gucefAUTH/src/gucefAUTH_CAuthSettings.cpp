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

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#ifndef GUCEF_AUTH_COAUTH2PROTOCOL_H
#include "gucefAUTH_COAuth2Protocol.h"
#define GUCEF_AUTH_COAUTH2PROTOCOL_H
#endif /* GUCEF_AUTH_COAUTH2PROTOCOL_H ? */

#ifndef GUCEF_AUTH_COPENID2_H
#include "gucefAUTH_COpenId2.h"
#define GUCEF_AUTH_COPENID2_H
#endif /* GUCEF_AUTH_COPENID2_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefAUTH_CAuthSettings.h"

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

const UInt32 CAuthSettings::MaxLastUsedProviders;
const UInt32 CAuthSettings::MaxLoginHints;
const UInt32 CAuthSettings::DefaultLoginTimeoutInSeconds;
const UInt32 CAuthSettings::MinLoginTimeoutInSeconds;
const UInt32 CAuthSettings::MaxLoginTimeoutInSeconds;
const UInt32 CAuthSettings::DefaultAutoLoginStepTimeoutInMs;
const UInt32 CAuthSettings::DefaultLoginDeadlineSlackInMs;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

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

static bool
TextToIdScope( const CORE::CString& text, TAuthIdScope& scope )
{GUCEF_TRACE;

    CORE::CString lower = text.Lowercase();
    if ( "global" == lower )
        scope = AUTHIDSCOPE_GLOBAL;
    else
    if ( "organisation" == lower || "organization" == lower )
        scope = AUTHIDSCOPE_PER_ORGANISATION;
    else
    if ( "app" == lower )
        scope = AUTHIDSCOPE_PER_APP;
    else
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CAuthProviderSettings::CAuthProviderSettings( void )
    : CORE::CIConfigurable()
    , clientId()
    , clientSecret()
    , audiences()
    , scopes()
    , discoveryUrl()
    , issuer()
    , authorizationEndpoint()
    , tokenEndpoint()
    , deviceAuthorizationEndpoint()
    , userInfoEndpoint()
    , revocationEndpoint()
    , tokenVerifyEndpoint()
    , jwksUri()
    , redirectMode( "loopback" )
    , loopbackPort( 0 )
    , loopbackPath( "/callback" )
    , redirectUri()
    , hasIdScope( false )
    , idScope( AUTHIDSCOPE_GLOBAL )
    , parameters()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderSettings::CAuthProviderSettings( const CAuthProviderSettings& src )
    : CORE::CIConfigurable( src )
    , clientId( src.clientId )
    , clientSecret( src.clientSecret )
    , audiences( src.audiences )
    , scopes( src.scopes )
    , discoveryUrl( src.discoveryUrl )
    , issuer( src.issuer )
    , authorizationEndpoint( src.authorizationEndpoint )
    , tokenEndpoint( src.tokenEndpoint )
    , deviceAuthorizationEndpoint( src.deviceAuthorizationEndpoint )
    , userInfoEndpoint( src.userInfoEndpoint )
    , revocationEndpoint( src.revocationEndpoint )
    , tokenVerifyEndpoint( src.tokenVerifyEndpoint )
    , jwksUri( src.jwksUri )
    , redirectMode( src.redirectMode )
    , loopbackPort( src.loopbackPort )
    , loopbackPath( src.loopbackPath )
    , redirectUri( src.redirectUri )
    , hasIdScope( src.hasIdScope )
    , idScope( src.idScope )
    , parameters( src.parameters )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderSettings::~CAuthProviderSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProviderSettings&
CAuthProviderSettings::operator=( const CAuthProviderSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        clientId = src.clientId;
        clientSecret = src.clientSecret;
        audiences = src.audiences;
        scopes = src.scopes;
        discoveryUrl = src.discoveryUrl;
        issuer = src.issuer;
        authorizationEndpoint = src.authorizationEndpoint;
        tokenEndpoint = src.tokenEndpoint;
        deviceAuthorizationEndpoint = src.deviceAuthorizationEndpoint;
        userInfoEndpoint = src.userInfoEndpoint;
        revocationEndpoint = src.revocationEndpoint;
        tokenVerifyEndpoint = src.tokenVerifyEndpoint;
        jwksUri = src.jwksUri;
        redirectMode = src.redirectMode;
        loopbackPort = src.loopbackPort;
        loopbackPath = src.loopbackPath;
        redirectUri = src.redirectUri;
        hasIdScope = src.hasIdScope;
        idScope = src.idScope;
        parameters = src.parameters;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderSettings::HasClientId( void ) const
{GUCEF_TRACE;

    return !clientId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderSettings::HasAcceptedClientIds( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector clientIds;
    GetAcceptedClientIds( clientIds );
    return !clientIds.empty();
}

/*-------------------------------------------------------------------------*/

void
CAuthProviderSettings::GetAcceptedClientIds( CORE::CString::StringVector& clientIds ) const
{GUCEF_TRACE;

    clientIds.clear();
    CORE::CString own = clientId.Trim( true ).Trim( false );
    if ( !own.IsNULLOrEmpty() )
        clientIds.push_back( own );
    for ( size_t i=0; i<audiences.size(); ++i )
    {
        CORE::CString audience = audiences[ i ].Trim( true ).Trim( false );
        if ( !audience.IsNULLOrEmpty() && !ContainsString( clientIds, audience ) )
            clientIds.push_back( audience );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthProviderSettings::GetParameter( const CORE::CString& name ) const
{GUCEF_TRACE;

    TParameterMap::const_iterator i = parameters.find( name );
    if ( i != parameters.end() )
        return (*i).second;
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "clientId", clientId );
    if ( !clientSecret.IsEmpty() )
        config.SetAttribute( "clientSecret", clientSecret.Reveal() );
    config.SetValuesOfChildByName( "audiences", audiences );
    config.SetValuesOfChildByName( "scopes", scopes );
    config.SetAttribute( "discoveryUrl", discoveryUrl );
    config.SetAttribute( "issuer", issuer );
    config.SetAttribute( "authorizationEndpoint", authorizationEndpoint );
    config.SetAttribute( "tokenEndpoint", tokenEndpoint );
    config.SetAttribute( "deviceAuthorizationEndpoint", deviceAuthorizationEndpoint );
    config.SetAttribute( "userInfoEndpoint", userInfoEndpoint );
    config.SetAttribute( "revocationEndpoint", revocationEndpoint );
    config.SetAttribute( "tokenVerifyEndpoint", tokenVerifyEndpoint );
    config.SetAttribute( "jwksUri", jwksUri );
    config.SetAttribute( "redirectMode", redirectMode );
    config.SetAttribute( "loopbackPort", (UInt32) loopbackPort );
    config.SetAttribute( "loopbackPath", loopbackPath );
    config.SetAttribute( "redirectUri", redirectUri );
    config.SetAttribute( "idScope", CORE::CString( hasIdScope ? IdScopeToText( idScope ) : "" ) );
    CORE::CDataNode* parameterNode = config.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != parameterNode )
    {
        TParameterMap::const_iterator i = parameters.begin();
        while ( i != parameters.end() )
        {
            parameterNode->SetAttribute( (*i).first, (*i).second );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthProviderSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    // The relay of an OpenID 2.0 login (the parameter relayUrl) is checked before anything is taken: an address that is not
    // acceptable refuses the settings, so that a typo never gives a login at an address nobody meant. A configuration has
    // no way to relax this; the setting of the engine that allows another port and loopback http is for the tests of the
    // engine, which hand the address over directly.
    const CORE::CDataNode* relayParameters = config.FindChild( "Parameters" );
    if ( GUCEF_NULL != relayParameters )
    {
        CORE::CString relayUrl = relayParameters->GetAttributeValueOrChildValueByName( "relayUrl" ).AsString( CORE::CString::Empty, true );
        if ( !relayUrl.IsNULLOrEmpty() )
        {
            CORE::CString baseUrl, realm, reason;
            if ( !COpenId2Relay::ParseRelayUrl( relayUrl, false, baseUrl, realm, reason ) )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AuthProviderSettings: the settings are refused, the parameter relayUrl is not acceptable: " << reason );
                return false;
            }
        }
    }

    clientId = config.GetAttributeValueOrChildValueByName( "clientId" ).AsString( clientId, true );
    CORE::CString secret = config.GetAttributeValueOrChildValueByName( "clientSecret" ).AsString( CORE::CString::Empty, true );
    if ( !secret.IsNULLOrEmpty() )
        clientSecret.Set( secret );
    CORE::CString::StringVector listedAudiences;
    if ( config.GetStringListByName( "audiences", listedAudiences ) )
        audiences = listedAudiences;
    if ( GUCEF_NULL != config.FindChild( "scopes" ) )
        config.GetValuesOfChildByName( "scopes", scopes );
    discoveryUrl = config.GetAttributeValueOrChildValueByName( "discoveryUrl" ).AsString( discoveryUrl, true );
    issuer = config.GetAttributeValueOrChildValueByName( "issuer" ).AsString( issuer, true );
    authorizationEndpoint = config.GetAttributeValueOrChildValueByName( "authorizationEndpoint" ).AsString( authorizationEndpoint, true );
    tokenEndpoint = config.GetAttributeValueOrChildValueByName( "tokenEndpoint" ).AsString( tokenEndpoint, true );
    deviceAuthorizationEndpoint = config.GetAttributeValueOrChildValueByName( "deviceAuthorizationEndpoint" ).AsString( deviceAuthorizationEndpoint, true );
    userInfoEndpoint = config.GetAttributeValueOrChildValueByName( "userInfoEndpoint" ).AsString( userInfoEndpoint, true );
    revocationEndpoint = config.GetAttributeValueOrChildValueByName( "revocationEndpoint" ).AsString( revocationEndpoint, true );
    tokenVerifyEndpoint = config.GetAttributeValueOrChildValueByName( "tokenVerifyEndpoint" ).AsString( tokenVerifyEndpoint, true );
    jwksUri = config.GetAttributeValueOrChildValueByName( "jwksUri" ).AsString( jwksUri, true );
    redirectMode = config.GetAttributeValueOrChildValueByName( "redirectMode" ).AsString( redirectMode, true );
    loopbackPort = (UInt16) config.GetAttributeValueOrChildValueByName( "loopbackPort" ).AsUInt32( loopbackPort, true );
    loopbackPath = config.GetAttributeValueOrChildValueByName( "loopbackPath" ).AsString( loopbackPath, true );
    redirectUri = config.GetAttributeValueOrChildValueByName( "redirectUri" ).AsString( redirectUri, true );

    CORE::CString scopeText = config.GetAttributeValueOrChildValueByName( "idScope" ).AsString( CORE::CString::Empty, true );
    hasIdScope = !scopeText.IsNULLOrEmpty() && TextToIdScope( scopeText, idScope );

    const CORE::CDataNode* parameterNode = config.FindChild( "Parameters" );
    if ( GUCEF_NULL != parameterNode )
    {
        parameters.clear();
        CORE::CDataNode::TAttributeMap::const_iterator i = parameterNode->AttributeBegin();
        while ( i != parameterNode->AttributeEnd() )
        {
            parameters[ (*i).first ] = (*i).second.AsString();
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthProviderSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthProviderSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CAuthSettings::CAuthSettings( void )
    : CORE::CIConfigurable()
    , autoLogin( true )
    , showUnavailableProviders( true )
    , appName( "GUCEF" )
    , displayName()
    , credentialDirectory()
    , linkStorePath()
    , autoLoginStepTimeoutInMs( DefaultAutoLoginStepTimeoutInMs )
    , loginTimeoutInSeconds( DefaultLoginTimeoutInSeconds )
    , loginDeadlineSlackInMs( DefaultLoginDeadlineSlackInMs )
    , enabledProviders()
    , lastUsedProviders()
    , rememberLoginHints( true )
    , loginHints()
    , providers()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthSettings::CAuthSettings( const CAuthSettings& src )
    : CORE::CIConfigurable( src )
    , autoLogin( src.autoLogin )
    , showUnavailableProviders( src.showUnavailableProviders )
    , appName( src.appName )
    , displayName( src.displayName )
    , credentialDirectory( src.credentialDirectory )
    , linkStorePath( src.linkStorePath )
    , autoLoginStepTimeoutInMs( src.autoLoginStepTimeoutInMs )
    , loginTimeoutInSeconds( src.loginTimeoutInSeconds )
    , loginDeadlineSlackInMs( src.loginDeadlineSlackInMs )
    , enabledProviders( src.enabledProviders )
    , lastUsedProviders( src.lastUsedProviders )
    , rememberLoginHints( src.rememberLoginHints )
    , loginHints( src.loginHints )
    , providers( src.providers )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthSettings::~CAuthSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthSettings&
CAuthSettings::operator=( const CAuthSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        autoLogin = src.autoLogin;
        showUnavailableProviders = src.showUnavailableProviders;
        appName = src.appName;
        displayName = src.displayName;
        credentialDirectory = src.credentialDirectory;
        linkStorePath = src.linkStorePath;
        autoLoginStepTimeoutInMs = src.autoLoginStepTimeoutInMs;
        loginTimeoutInSeconds = src.loginTimeoutInSeconds;
        loginDeadlineSlackInMs = src.loginDeadlineSlackInMs;
        enabledProviders = src.enabledProviders;
        lastUsedProviders = src.lastUsedProviders;
        rememberLoginHints = src.rememberLoginHints;
        loginHints = src.loginHints;
        providers = src.providers;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

UInt32
CAuthSettings::ClampLoginTimeoutInSeconds( UInt32 seconds )
{GUCEF_TRACE;

    if ( seconds < MinLoginTimeoutInSeconds )
        return MinLoginTimeoutInSeconds;
    if ( seconds > MaxLoginTimeoutInSeconds )
        return MaxLoginTimeoutInSeconds;
    return seconds;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthSettings::GetCredentialDirectory( void ) const
{GUCEF_TRACE;

    if ( !credentialDirectory.IsNULLOrEmpty() )
        return CORE::RelativePath( credentialDirectory );
    CORE::CString path = CORE::RelativePath( "$APPDATADIR$" );
    path = CORE::CombinePath( path, appName );
    return CORE::CombinePath( path, "Auth" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthSettings::GetLinkStorePath( void ) const
{GUCEF_TRACE;

    if ( !linkStorePath.IsNULLOrEmpty() )
        return CORE::RelativePath( linkStorePath );
    return CORE::CombinePath( GetCredentialDirectory(), "links.json" );
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::GetProviderSettings( const CORE::CString& providerName, CAuthProviderSettings& providerSettings ) const
{GUCEF_TRACE;

    TProviderSettingsMap::const_iterator i = providers.find( providerName );
    if ( i == providers.end() )
        return false;
    providerSettings = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthSettings::SetProviderSettings( const CORE::CString& providerName, const CAuthProviderSettings& providerSettings )
{GUCEF_TRACE;

    providers[ providerName ] = providerSettings;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::IsProviderEnabled( const CORE::CString& providerName ) const
{GUCEF_TRACE;

    return enabledProviders.empty() || ContainsString( enabledProviders, providerName );
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::IsProviderOffered( const CORE::CString& providerName, bool providerRequiresClientId ) const
{GUCEF_TRACE;

    if ( !IsProviderEnabled( providerName ) )
        return false;
    if ( !providerRequiresClientId )
        return true;
    TProviderSettingsMap::const_iterator i = providers.find( providerName );
    return i != providers.end() && (*i).second.HasClientId();
}

/*-------------------------------------------------------------------------*/

/** Replaces 'from' by 'to' in the list, keeping the place and not making 'to' appear twice; true when the list changed */
static bool
RenameInList( CORE::CString::StringVector& list, const CORE::CString& from, const CORE::CString& to )
{GUCEF_TRACE;

    bool changed = false;
    CORE::CString::StringVector renamed;
    for ( size_t i=0; i<list.size(); ++i )
    {
        if ( list[ i ] == from )
        {
            changed = true;
            if ( !ContainsString( renamed, to ) && !ContainsString( list, to ) )
                renamed.push_back( to );
        }
        else
            renamed.push_back( list[ i ] );
    }
    if ( changed )
        list = renamed;
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::RenameProvider( const CORE::CString& from, const CORE::CString& to )
{GUCEF_TRACE;

    if ( from.IsNULLOrEmpty() || to.IsNULLOrEmpty() || from == to )
        return false;

    bool found = false;
    TProviderSettingsMap::iterator f = providers.find( from );
    if ( f != providers.end() )
    {
        found = true;
        TProviderSettingsMap::iterator t = providers.find( to );
        if ( t == providers.end() )
        {
            providers[ to ] = (*f).second;
        }
        else
        {
            for ( CAuthProviderSettings::TParameterMap::const_iterator p = (*f).second.parameters.begin(); p != (*f).second.parameters.end(); ++p )
                if ( (*t).second.parameters.find( (*p).first ) == (*t).second.parameters.end() )
                    (*t).second.parameters[ (*p).first ] = (*p).second;
        }
        providers.erase( from );
    }
    if ( RenameInList( enabledProviders, from, to ) )
        found = true;
    if ( RenameInList( lastUsedProviders, from, to ) )
        found = true;
    CORE::CString::StringMap::iterator hint = loginHints.find( from );
    if ( hint != loginHints.end() )
    {
        found = true;
        if ( loginHints.find( to ) == loginHints.end() )
            loginHints[ to ] = (*hint).second;
        loginHints.erase( from );
    }
    return found;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::TouchLastUsed( const CORE::CString& providerName )
{GUCEF_TRACE;

    if ( providerName.IsNULLOrEmpty() )
        return false;
    if ( !lastUsedProviders.empty() && lastUsedProviders[ 0 ] == providerName )
        return false;
    CORE::CString::StringVector updated;
    updated.push_back( providerName );
    for ( size_t i=0; i<lastUsedProviders.size() && updated.size() < MaxLastUsedProviders; ++i )
        if ( lastUsedProviders[ i ] != providerName )
            updated.push_back( lastUsedProviders[ i ] );
    lastUsedProviders = updated;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::GetLoginHint( const CORE::CString& providerName, CORE::CString& loginHint ) const
{GUCEF_TRACE;

    loginHint.Clear();
    if ( !rememberLoginHints )
        return false;
    CORE::CString::StringMap::const_iterator i = loginHints.find( providerName );
    if ( i == loginHints.end() || !COAuth2Protocol::IsAcceptableLoginHint( (*i).second ) )
        return false;
    loginHint = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::SetLoginHint( const CORE::CString& providerName, const CORE::CString& loginHint )
{GUCEF_TRACE;

    if ( !rememberLoginHints || providerName.IsNULLOrEmpty() || !COAuth2Protocol::IsAcceptableLoginHint( loginHint ) )
        return false;
    CORE::CString::StringMap::iterator i = loginHints.find( providerName );
    if ( i != loginHints.end() && (*i).second == loginHint )
        return false;
    loginHints[ providerName ] = loginHint;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::ClearLoginHints( void )
{GUCEF_TRACE;

    const bool had = !loginHints.empty();
    loginHints.clear();
    return had;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::HasLoginHints( void ) const
{GUCEF_TRACE;

    return rememberLoginHints && !loginHints.empty();
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "autoLogin", autoLogin );
    config.SetAttribute( "showUnavailableProviders", showUnavailableProviders );
    config.SetAttribute( "appName", appName );
    if ( !displayName.IsNULLOrEmpty() )
        config.SetAttribute( "displayName", displayName );
    config.SetAttribute( "credentialDirectory", credentialDirectory );
    config.SetAttribute( "linkStorePath", linkStorePath );
    config.SetAttribute( "autoLoginStepTimeoutInMs", autoLoginStepTimeoutInMs );
    config.SetAttribute( "loginTimeoutInSeconds", loginTimeoutInSeconds );
    config.SetAttribute( "loginDeadlineSlackInMs", loginDeadlineSlackInMs );
    config.SetValuesOfChildByName( "enabledProviders", enabledProviders );
    config.SetValuesOfChildByName( "lastUsedProviders", lastUsedProviders );
    config.SetAttribute( "rememberLoginHints", rememberLoginHints );
    if ( rememberLoginHints && !loginHints.empty() )
    {
        CORE::CDataNode* hintsNode = config.AddChild( "LoginHints", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL != hintsNode )
        {
            CORE::CString::StringMap::const_iterator h = loginHints.begin();
            while ( h != loginHints.end() )
            {
                hintsNode->SetAttribute( (*h).first, (*h).second );
                ++h;
            }
        }
    }
    CORE::CDataNode* providersNode = config.AddChild( "Providers", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL != providersNode )
    {
        TProviderSettingsMap::const_iterator i = providers.begin();
        while ( i != providers.end() )
        {
            CORE::CDataNode* providerNode = providersNode->AddChild( (*i).first, GUCEF_DATATYPE_OBJECT );
            if ( GUCEF_NULL != providerNode )
                (*i).second.SaveConfig( *providerNode );
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &config;
    if ( config.GetName() != "Auth" )
    {
        const CORE::CDataNode* child = config.FindChild( "Auth" );
        if ( GUCEF_NULL != child )
            section = child;
    }

    autoLogin = section->GetAttributeValueOrChildValueByName( "autoLogin" ).AsBool( autoLogin, true );
    showUnavailableProviders = section->GetAttributeValueOrChildValueByName( "showUnavailableProviders" ).AsBool( showUnavailableProviders, true );
    appName = section->GetAttributeValueOrChildValueByName( "appName" ).AsString( appName, true );
    displayName = section->GetAttributeValueOrChildValueByName( "displayName" ).AsString( displayName, true );
    credentialDirectory = section->GetAttributeValueOrChildValueByName( "credentialDirectory" ).AsString( credentialDirectory, false );
    linkStorePath = section->GetAttributeValueOrChildValueByName( "linkStorePath" ).AsString( linkStorePath, false );
    autoLoginStepTimeoutInMs = section->GetAttributeValueOrChildValueByName( "autoLoginStepTimeoutInMs" ).AsUInt32( autoLoginStepTimeoutInMs, true );
    loginTimeoutInSeconds = ClampLoginTimeoutInSeconds( section->GetAttributeValueOrChildValueByName( "loginTimeoutInSeconds" ).AsUInt32( loginTimeoutInSeconds, true ) );
    loginDeadlineSlackInMs = section->GetAttributeValueOrChildValueByName( "loginDeadlineSlackInMs" ).AsUInt32( loginDeadlineSlackInMs, true );
    if ( GUCEF_NULL != section->FindChild( "enabledProviders" ) )
        section->GetValuesOfChildByName( "enabledProviders", enabledProviders );
    if ( GUCEF_NULL != section->FindChild( "lastUsedProviders" ) )
    {
        section->GetValuesOfChildByName( "lastUsedProviders", lastUsedProviders );
        if ( lastUsedProviders.size() > MaxLastUsedProviders )
            lastUsedProviders.resize( MaxLastUsedProviders );
    }

    rememberLoginHints = section->GetAttributeValueOrChildValueByName( "rememberLoginHints" ).AsBool( rememberLoginHints, true );
    const CORE::CDataNode* hintsNode = section->FindChild( "LoginHints" );
    if ( GUCEF_NULL != hintsNode )
    {
        loginHints.clear();
        // with the setting off nothing is taken over: what the file still holds is gone with the next save
        if ( rememberLoginHints )
        {
            CORE::CDataNode::TAttributeMap::const_iterator h = hintsNode->AttributeBegin();
            while ( h != hintsNode->AttributeEnd() && loginHints.size() < MaxLoginHints )
            {
                CORE::CString value = (*h).second.AsString();
                if ( !(*h).first.IsNULLOrEmpty() && COAuth2Protocol::IsAcceptableLoginHint( value ) )
                    loginHints[ (*h).first ] = value;
                ++h;
            }
        }
    }
    if ( !rememberLoginHints )
        loginHints.clear();

    const CORE::CDataNode* providersNode = section->FindChild( "Providers" );
    if ( GUCEF_NULL != providersNode )
    {
        providers.clear();
        CORE::CDataNode::const_iterator i = providersNode->ConstBegin();
        while ( i != providersNode->ConstEnd() )
        {
            CAuthProviderSettings providerSettings;
            if ( providerSettings.LoadConfig( *(*i) ) )
                providers[ (*i)->GetName() ] = providerSettings;
            ++i;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthSettings";
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
