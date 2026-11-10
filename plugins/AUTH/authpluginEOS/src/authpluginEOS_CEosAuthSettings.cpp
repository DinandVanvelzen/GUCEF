/*
 *  authpluginEOS: gucefAUTH plugin for logging in with Epic Online Services
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

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "authpluginEOS_CEosAuthSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A text setting: the Parameters child first, then the node itself; the value that is there is the default */
static CORE::CString
ReadText( const CORE::CDataNode& config, const CORE::CDataNode* parameters, const char* name, const CORE::CString& current, bool resolveVariables )
{GUCEF_TRACE;

    CORE::CString value = current;
    if ( GUCEF_NULL != parameters )
        value = parameters->GetAttributeValueOrChildValueByName( name ).AsString( value, resolveVariables );
    value = config.GetAttributeValueOrChildValueByName( name ).AsString( value, resolveVariables );
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

/*-------------------------------------------------------------------------*/

static bool
IsHex64( const CORE::CString& text )
{GUCEF_TRACE;

    if ( AUTH::GetByteLength( text ) != 64 )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<64; ++i )
    {
        char c = chars[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' ) ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEosAuthSettings::CEosAuthSettings( void )
    : CORE::CIConfigurable()
    , clientId()
    , clientSecret()
    , productId()
    , sandboxId()
    , deploymentId()
    , encryptionKey()
    , productName( "GUCEF" )
    , productVersion( "1.0" )
    , cacheDirectory()
    , useConnect( true )
    , createConnectUser( true )
    , deletePersistentAuthOnLogout( true )
    , tickIntervalInMs( DefaultTickIntervalInMs )
    , silentTimeoutInMs( DefaultSilentTimeoutInMs )
    , m_engineSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosAuthSettings::CEosAuthSettings( const CEosAuthSettings& src )
    : CORE::CIConfigurable( src )
    , clientId( src.clientId )
    , clientSecret( src.clientSecret )
    , productId( src.productId )
    , sandboxId( src.sandboxId )
    , deploymentId( src.deploymentId )
    , encryptionKey( src.encryptionKey )
    , productName( src.productName )
    , productVersion( src.productVersion )
    , cacheDirectory( src.cacheDirectory )
    , useConnect( src.useConnect )
    , createConnectUser( src.createConnectUser )
    , deletePersistentAuthOnLogout( src.deletePersistentAuthOnLogout )
    , tickIntervalInMs( src.tickIntervalInMs )
    , silentTimeoutInMs( src.silentTimeoutInMs )
    , m_engineSettings( src.m_engineSettings )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosAuthSettings::~CEosAuthSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosAuthSettings&
CEosAuthSettings::operator=( const CEosAuthSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        clientId = src.clientId;
        clientSecret = src.clientSecret;
        productId = src.productId;
        sandboxId = src.sandboxId;
        deploymentId = src.deploymentId;
        encryptionKey = src.encryptionKey;
        productName = src.productName;
        productVersion = src.productVersion;
        cacheDirectory = src.cacheDirectory;
        useConnect = src.useConnect;
        createConnectUser = src.createConnectUser;
        deletePersistentAuthOnLogout = src.deletePersistentAuthOnLogout;
        tickIntervalInMs = src.tickIntervalInMs;
        silentTimeoutInMs = src.silentTimeoutInMs;
        m_engineSettings = src.m_engineSettings;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthSettings::operator==( const CEosAuthSettings& other ) const
{GUCEF_TRACE;

    return clientId == other.clientId && clientSecret == other.clientSecret && productId == other.productId && sandboxId == other.sandboxId &&
           deploymentId == other.deploymentId && encryptionKey == other.encryptionKey && productName == other.productName &&
           productVersion == other.productVersion && cacheDirectory == other.cacheDirectory && useConnect == other.useConnect &&
           createConnectUser == other.createConnectUser && deletePersistentAuthOnLogout == other.deletePersistentAuthOnLogout &&
           tickIntervalInMs == other.tickIntervalInMs && silentTimeoutInMs == other.silentTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthSettings::IsConfigured( void ) const
{GUCEF_TRACE;

    return !clientId.IsNULLOrEmpty() && !clientSecret.IsEmpty() && !productId.IsNULLOrEmpty() && !sandboxId.IsNULLOrEmpty() && !deploymentId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthSettings::IsWellFormed( void ) const
{GUCEF_TRACE;

    return ( encryptionKey.IsEmpty() || IsHex64( encryptionKey.Reveal() ) ) && tickIntervalInMs > 0;
}

/*-------------------------------------------------------------------------*/

PLUGINGLUE::EOS::CEosPlatformSettings
CEosAuthSettings::GetPlatformSettings( void ) const
{GUCEF_TRACE;

    PLUGINGLUE::EOS::CEosPlatformSettings settings;
    settings.clientId = clientId;
    settings.clientSecret = clientSecret;
    settings.productId = productId;
    settings.sandboxId = sandboxId;
    settings.deploymentId = deploymentId;
    settings.encryptionKey = encryptionKey;
    settings.productName = productName;
    settings.productVersion = productVersion;
    settings.cacheDirectory = cacheDirectory;
    return settings;
}

/*-------------------------------------------------------------------------*/

AUTH::CAuthProviderSettings
CEosAuthSettings::GetEngineSettings( void ) const
{GUCEF_TRACE;

    AUTH::CAuthProviderSettings settings( m_engineSettings );
    settings.clientId = clientId;
    settings.clientSecret = clientSecret;
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    // The keys of CAuthProviderSettings in the form it writes them
    config.SetAttribute( "clientId", clientId );
    if ( !clientSecret.IsEmpty() )
        config.SetAttribute( "clientSecret", clientSecret.Reveal() );

    CORE::CDataNode* parameters = config.AddChild( "Parameters", GUCEF_DATATYPE_OBJECT );
    if ( GUCEF_NULL == parameters )
        return false;

    parameters->SetAttribute( "productId", productId );
    parameters->SetAttribute( "sandboxId", sandboxId );
    parameters->SetAttribute( "deploymentId", deploymentId );
    if ( !encryptionKey.IsEmpty() )
        parameters->SetAttribute( "encryptionKey", encryptionKey.Reveal() );
    parameters->SetAttribute( "productName", productName );
    parameters->SetAttribute( "productVersion", productVersion );
    parameters->SetAttribute( "cacheDirectory", cacheDirectory );
    parameters->SetAttribute( "useConnect", useConnect );
    parameters->SetAttribute( "createConnectUser", createConnectUser );
    parameters->SetAttribute( "deletePersistentAuthOnLogout", deletePersistentAuthOnLogout );
    parameters->SetAttribute( "tickIntervalInMs", tickIntervalInMs );
    parameters->SetAttribute( "silentTimeoutInMs", silentTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosAuthSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* parameters = config.FindChild( "Parameters" );

    CEosAuthSettings loaded( *this );
    loaded.clientId = ReadText( config, parameters, "clientId", clientId, false );
    CORE::CString secret = ReadText( config, parameters, "clientSecret", CORE::CString::Empty, false );
    if ( !secret.IsNULLOrEmpty() )
        loaded.clientSecret.Set( secret );
    loaded.productId = ReadText( config, parameters, "productId", productId, false );
    loaded.sandboxId = ReadText( config, parameters, "sandboxId", sandboxId, false );
    loaded.deploymentId = ReadText( config, parameters, "deploymentId", deploymentId, false );
    CORE::CString key = ReadText( config, parameters, "encryptionKey", CORE::CString::Empty, false );
    if ( !key.IsNULLOrEmpty() )
        loaded.encryptionKey.Set( key );
    loaded.productName = ReadText( config, parameters, "productName", productName, false );
    loaded.productVersion = ReadText( config, parameters, "productVersion", productVersion, false );
    loaded.cacheDirectory = ReadText( config, parameters, "cacheDirectory", cacheDirectory, true );
    loaded.useConnect = ReadBool( config, parameters, "useConnect", useConnect );
    loaded.createConnectUser = ReadBool( config, parameters, "createConnectUser", createConnectUser );
    loaded.deletePersistentAuthOnLogout = ReadBool( config, parameters, "deletePersistentAuthOnLogout", deletePersistentAuthOnLogout );
    loaded.tickIntervalInMs = ReadUInt32( config, parameters, "tickIntervalInMs", tickIntervalInMs );
    loaded.silentTimeoutInMs = ReadUInt32( config, parameters, "silentTimeoutInMs", silentTimeoutInMs );

    if ( !loaded.IsWellFormed() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosAuthSettings: The settings are not well formed (the encryptionKey has to be 64 hex digits, tickIntervalInMs more than 0), they are not used" );
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
CEosAuthSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::EOS::CEosAuthSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
