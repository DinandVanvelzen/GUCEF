/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#include "pluginglueEOS_CEosPlatformSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
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

CEosPlatformSettings::CEosPlatformSettings( void )
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
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosPlatformSettings::CEosPlatformSettings( const CEosPlatformSettings& src )
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
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosPlatformSettings::~CEosPlatformSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosPlatformSettings&
CEosPlatformSettings::operator=( const CEosPlatformSettings& src )
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
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::operator==( const CEosPlatformSettings& other ) const
{GUCEF_TRACE;

    return IsSameProduct( other ) && encryptionKey == other.encryptionKey && productName == other.productName &&
           productVersion == other.productVersion && cacheDirectory == other.cacheDirectory;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::IsSameProduct( const CEosPlatformSettings& other ) const
{GUCEF_TRACE;

    return clientId == other.clientId && clientSecret == other.clientSecret && productId == other.productId &&
           sandboxId == other.sandboxId && deploymentId == other.deploymentId;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::IsConfigured( void ) const
{GUCEF_TRACE;

    return !clientId.IsNULLOrEmpty() && !clientSecret.IsEmpty() && !productId.IsNULLOrEmpty() && !sandboxId.IsNULLOrEmpty() && !deploymentId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::IsWellFormed( void ) const
{GUCEF_TRACE;

    return encryptionKey.IsEmpty() || IsHex64( encryptionKey.Reveal() );
}

/*-------------------------------------------------------------------------*/

void
CEosPlatformSettings::FillPlatformConfig( SEosPlatformConfig& config ) const
{GUCEF_TRACE;

    config.productName = productName.C_String();
    config.productVersion = productVersion.C_String();
    config.productId = productId.C_String();
    config.sandboxId = sandboxId.C_String();
    config.deploymentId = deploymentId.C_String();
    config.clientId = clientId.C_String();
    config.clientSecret = clientSecret.Reveal().C_String();
    config.encryptionKey = encryptionKey.Reveal().C_String();
    config.cacheDirectory = cacheDirectory.C_String();
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::LoadAuthSection( const CORE::CDataNode& authSection )
{GUCEF_TRACE;

    const CORE::CDataNode* section = &authSection;
    if ( authSection.GetName() != "Auth" )
    {
        const CORE::CDataNode* child = authSection.FindChild( "Auth" );
        if ( GUCEF_NULL != child )
            section = child;
    }
    const CORE::CDataNode* providers = section->FindChild( "Providers" );
    if ( GUCEF_NULL == providers )
        return false;
    const CORE::CDataNode* epic = providers->FindChild( "epic" );
    if ( GUCEF_NULL == epic )
        return false;
    return LoadConfig( *epic );
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

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
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatformSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    const CORE::CDataNode* parameters = config.FindChild( "Parameters" );

    CEosPlatformSettings loaded( *this );
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

    if ( !loaded.IsWellFormed() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "EosPlatformSettings: The settings are not well formed (the encryptionKey has to be 64 hex digits), they are not used" );
        return false;
    }

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosPlatformSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::EOS::CEosPlatformSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
