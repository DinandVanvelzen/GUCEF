/*
 *  gucefWEB: GUCEF module providing Web application functionality
 *  for standardized protocols
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

#include "gucefWEB_CMcpHttpTransportSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A list as configured, without the empty elements an empty text gives */
static void
DropEmpty( CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CString::StringVector kept;
    for ( size_t i=0; i<values.size(); ++i )
    {
        CORE::CString value = values[ i ].Trim( true ).Trim( false );
        if ( !value.IsNULLOrEmpty() )
            kept.push_back( value );
    }
    values = kept;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
JoinList( const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( size_t i=0; i<values.size(); ++i )
    {
        if ( i > 0 )
            joined += ',';
        joined += values[ i ];
    }
    return joined;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpHttpTransportSettings::CMcpHttpTransportSettings( void )
    : CORE::CIConfigurable()
    , bindAddress( "127.0.0.1" )
    , port( 0 )
    , path( "/mcp" )
    , allowedOrigins()
    , allowedHosts()
    , authenticator( "static-bearer" )
    , maxBodyBytes( 0 )
    , maxConnections( 32 )
    , sseKeepAliveMs( 0 )
    , headerTimeoutMs( 10000 )
    , bodyTimeoutMs( 30000 )
    , idleTimeoutMs( 60000 )
    , allowNonLoopback( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpHttpTransportSettings::CMcpHttpTransportSettings( const CMcpHttpTransportSettings& src )
    : CORE::CIConfigurable( src )
    , bindAddress( src.bindAddress )
    , port( src.port )
    , path( src.path )
    , allowedOrigins( src.allowedOrigins )
    , allowedHosts( src.allowedHosts )
    , authenticator( src.authenticator )
    , maxBodyBytes( src.maxBodyBytes )
    , maxConnections( src.maxConnections )
    , sseKeepAliveMs( src.sseKeepAliveMs )
    , headerTimeoutMs( src.headerTimeoutMs )
    , bodyTimeoutMs( src.bodyTimeoutMs )
    , idleTimeoutMs( src.idleTimeoutMs )
    , allowNonLoopback( src.allowNonLoopback )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpHttpTransportSettings::~CMcpHttpTransportSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpHttpTransportSettings&
CMcpHttpTransportSettings::operator=( const CMcpHttpTransportSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        bindAddress = src.bindAddress;
        port = src.port;
        path = src.path;
        allowedOrigins = src.allowedOrigins;
        allowedHosts = src.allowedHosts;
        authenticator = src.authenticator;
        maxBodyBytes = src.maxBodyBytes;
        maxConnections = src.maxConnections;
        sseKeepAliveMs = src.sseKeepAliveMs;
        headerTimeoutMs = src.headerTimeoutMs;
        bodyTimeoutMs = src.bodyTimeoutMs;
        idleTimeoutMs = src.idleTimeoutMs;
        allowNonLoopback = src.allowNonLoopback;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpTransportSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "bindAddress", CORE::CVariant( bindAddress ) );
    cfg.SetAttribute( "port", (UInt32) port );
    cfg.SetAttribute( "path", CORE::CVariant( path ) );
    cfg.SetAttribute( "allowedOrigins", CORE::CVariant( JoinList( allowedOrigins ) ) );
    cfg.SetAttribute( "allowedHosts", CORE::CVariant( JoinList( allowedHosts ) ) );
    cfg.SetAttribute( "authenticator", CORE::CVariant( authenticator ) );
    cfg.SetAttribute( "maxBodyBytes", maxBodyBytes );
    cfg.SetAttribute( "maxConnections", maxConnections );
    cfg.SetAttribute( "sseKeepAliveMs", sseKeepAliveMs );
    cfg.SetAttribute( "headerTimeoutMs", headerTimeoutMs );
    cfg.SetAttribute( "bodyTimeoutMs", bodyTimeoutMs );
    cfg.SetAttribute( "idleTimeoutMs", idleTimeoutMs );
    cfg.SetAttribute( "allowNonLoopback", allowNonLoopback );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpHttpTransportSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    bindAddress = cfg.GetAttributeValueOrChildValueByName( "bindAddress" ).AsString( bindAddress, true );
    const UInt32 configuredPort = cfg.GetAttributeValueOrChildValueByName( "port" ).AsUInt32( port, true );
    if ( configuredPort > 65535 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpHttpTransportSettings: The port " << configuredPort << " is not valid" );
        return false;
    }
    port = (UInt16) configuredPort;
    path = cfg.GetAttributeValueOrChildValueByName( "path" ).AsString( path, true );
    if ( path.IsNULLOrEmpty() || '/' != path[ 0 ] )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpHttpTransportSettings: The path of the MCP endpoint must start with /" );
        return false;
    }
    CORE::CString::StringVector list;
    if ( cfg.GetStringListByName( "allowedOrigins", list ) )
    {
        DropEmpty( list );
        allowedOrigins = list;
    }
    list.clear();
    if ( cfg.GetStringListByName( "allowedHosts", list ) )
    {
        DropEmpty( list );
        allowedHosts = list;
    }
    authenticator = cfg.GetAttributeValueOrChildValueByName( "authenticator" ).AsString( authenticator, true );
    maxBodyBytes = cfg.GetAttributeValueOrChildValueByName( "maxBodyBytes" ).AsUInt32( maxBodyBytes, true );
    maxConnections = cfg.GetAttributeValueOrChildValueByName( "maxConnections" ).AsUInt32( maxConnections, true );
    sseKeepAliveMs = cfg.GetAttributeValueOrChildValueByName( "sseKeepAliveMs" ).AsUInt32( sseKeepAliveMs, true );
    headerTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "headerTimeoutMs" ).AsUInt32( headerTimeoutMs, true );
    bodyTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "bodyTimeoutMs" ).AsUInt32( bodyTimeoutMs, true );
    idleTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "idleTimeoutMs" ).AsUInt32( idleTimeoutMs, true );
    allowNonLoopback = cfg.GetAttributeValueOrChildValueByName( "allowNonLoopback" ).AsBool( allowNonLoopback, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpHttpTransportSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::WEB::CMcpHttpTransportSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
