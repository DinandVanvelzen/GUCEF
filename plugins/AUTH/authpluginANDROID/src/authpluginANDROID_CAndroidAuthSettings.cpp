/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "authpluginANDROID_CAndroidAuthSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A text setting: the Parameters child first, then the node itself; the value that is there is the default */
static CORE::CString
ReadText( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, const CORE::CString& current )
{GUCEF_TRACE;

    CORE::CString value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsString( value, false );
    value = config.GetAttributeValueOrChildValueByName( name ).AsString( value, false );
    return value;
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadUInt32( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, UInt32 current )
{GUCEF_TRACE;

    UInt32 value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsUInt32( value, true );
    value = config.GetAttributeValueOrChildValueByName( name ).AsUInt32( value, true );
    return value;
}

/*-------------------------------------------------------------------------*/

static bool
ReadBool( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, bool current )
{GUCEF_TRACE;

    bool value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsBool( value, true );
    value = config.GetAttributeValueOrChildValueByName( name ).AsBool( value, true );
    return value;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAndroidAuthSettings::CAndroidAuthSettings( void )
    : CORE::CIConfigurable()
    , clientId()
    , clientSecret()
    , useCredentialManager( true )
    , proofKind( "auth-code" )
    , tokenEndpoint( "https://oauth2.googleapis.com/token" )
    , tokenInfoEndpoint( "https://api.amazon.com/auth/o2/tokeninfo" )
    , amazonScopes( "profile" )
    , allowInsecureHttp( false )
    , httpTimeoutInMs( DefaultHttpTimeoutInMs )
    , silentTimeoutInMs( DefaultSilentTimeoutInMs )
    , m_engineSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthSettings::CAndroidAuthSettings( const CAndroidAuthSettings& src )
    : CORE::CIConfigurable( src )
    , clientId( src.clientId )
    , clientSecret( src.clientSecret )
    , useCredentialManager( src.useCredentialManager )
    , proofKind( src.proofKind )
    , tokenEndpoint( src.tokenEndpoint )
    , tokenInfoEndpoint( src.tokenInfoEndpoint )
    , amazonScopes( src.amazonScopes )
    , allowInsecureHttp( src.allowInsecureHttp )
    , httpTimeoutInMs( src.httpTimeoutInMs )
    , silentTimeoutInMs( src.silentTimeoutInMs )
    , m_engineSettings( src.m_engineSettings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthSettings::~CAndroidAuthSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthSettings&
CAndroidAuthSettings::operator=( const CAndroidAuthSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        clientId = src.clientId;
        clientSecret = src.clientSecret;
        useCredentialManager = src.useCredentialManager;
        proofKind = src.proofKind;
        tokenEndpoint = src.tokenEndpoint;
        tokenInfoEndpoint = src.tokenInfoEndpoint;
        amazonScopes = src.amazonScopes;
        allowInsecureHttp = src.allowInsecureHttp;
        httpTimeoutInMs = src.httpTimeoutInMs;
        silentTimeoutInMs = src.silentTimeoutInMs;
        m_engineSettings = src.m_engineSettings;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthSettings::operator==( const CAndroidAuthSettings& other ) const
{GUCEF_TRACE;

    return clientId == other.clientId && clientSecret == other.clientSecret && useCredentialManager == other.useCredentialManager &&
           proofKind == other.proofKind && tokenEndpoint == other.tokenEndpoint && tokenInfoEndpoint == other.tokenInfoEndpoint &&
           amazonScopes == other.amazonScopes && allowInsecureHttp == other.allowInsecureHttp && httpTimeoutInMs == other.httpTimeoutInMs &&
           silentTimeoutInMs == other.silentTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthSettings::IsWellFormed( void ) const
{GUCEF_TRACE;

    return ( proofKind == "auth-code" || proofKind == "id-token" ) && !amazonScopes.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthProviderSettings
CAndroidAuthSettings::GetEngineSettings( void ) const
{GUCEF_TRACE;

    AUTH::CAuthProviderSettings settings( m_engineSettings );
    settings.clientId = clientId;
    settings.clientSecret = clientSecret;
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    // The keys of CAuthProviderSettings in the form it writes them
    config.SetAttribute( "clientId", clientId );
    if ( !clientSecret.IsEmpty() )
        config.SetAttribute( "clientSecret", clientSecret.Reveal() );

    CORE::CDataNode* parameters = config.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == parameters )
        return false;

    parameters->SetAttribute( "useCredentialManager", useCredentialManager );
    parameters->SetAttribute( "proofKind", proofKind );
    parameters->SetAttribute( "tokenEndpoint", tokenEndpoint );
    parameters->SetAttribute( "tokenInfoEndpoint", tokenInfoEndpoint );
    parameters->SetAttribute( "amazonScopes", amazonScopes );
    parameters->SetAttribute( "allowInsecureHttp", allowInsecureHttp );
    parameters->SetAttribute( "httpTimeoutInMs", httpTimeoutInMs );
    parameters->SetAttribute( "silentTimeoutInMs", silentTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* parameters = config.FindChild( "Parameters" );

    CAndroidAuthSettings loaded( *this );
    loaded.clientId = ReadText( config, parameters, "clientId", clientId );
    CORE::CString secret = ReadText( config, parameters, "clientSecret", CORE::CString::Empty );
    if ( !secret.IsNULLOrEmpty() )
        loaded.clientSecret.Set( secret );
    loaded.useCredentialManager = ReadBool( config, parameters, "useCredentialManager", useCredentialManager );
    loaded.proofKind = ReadText( config, parameters, "proofKind", proofKind );
    loaded.tokenEndpoint = ReadText( config, parameters, "tokenEndpoint", tokenEndpoint );
    loaded.tokenInfoEndpoint = ReadText( config, parameters, "tokenInfoEndpoint", tokenInfoEndpoint );
    loaded.amazonScopes = ReadText( config, parameters, "amazonScopes", amazonScopes );
    loaded.allowInsecureHttp = ReadBool( config, parameters, "allowInsecureHttp", allowInsecureHttp );
    loaded.httpTimeoutInMs = ReadUInt32( config, parameters, "httpTimeoutInMs", httpTimeoutInMs );
    loaded.silentTimeoutInMs = ReadUInt32( config, parameters, "silentTimeoutInMs", silentTimeoutInMs );

    if ( !loaded.IsWellFormed() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthSettings: The settings are not well formed (proofKind is auth-code or id-token, amazonScopes is not empty), they are not used" );
        return false;
    }

    // What the engine takes of the node: its endpoint overrides and parameters
    AUTH::CAuthProviderSettings engineSettings;
    engineSettings.LoadConfig( config );
    loaded.m_engineSettings = engineSettings;

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAndroidAuthSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
