/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOM_CIrcClientSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

CORE::CString
JoinList( const CORE::CString::StringVector& list )
{GUCEF_TRACE;

    CORE::CString joined;
    for ( size_t i=0; i<list.size(); ++i )
    {
        if ( i > 0 )
            joined += ',';
        joined += list[ i ];
    }
    return joined;
}

/*-------------------------------------------------------------------------*/

void
ReadList( const CORE::CDataNode& config, const char* name, CORE::CString::StringVector& list )
{GUCEF_TRACE;

    const CORE::CVariant& value = config.GetAttributeValueOrChildValueByName( name );
    if ( value.IsNULLOrEmpty() )
        return;

    list.clear();
    CORE::CString::StringVector elements = value.AsString( CORE::CString::Empty, true ).ParseElements( ',', false );
    for ( size_t i=0; i<elements.size(); ++i )
    {
        CORE::CString element = elements[ i ].Trim( true ).Trim( false );
        if ( !element.IsNULLOrEmpty() )
            list.push_back( element );
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIrcClientSettings::CIrcClientSettings( void )
    : CORE::CIConfigurable()
    , server()
    , port( 6667 )
    , useWebSocket( false )
    , webSocketPath( "/" )
    , nick()
    , alternativeNicks()
    , userName()
    , realName()
    , saslAccount()
    , saslPassword()
    , wantedCapabilities()
    , autoJoinChannels()
    , pingTimeoutInSecs( 120 )
{GUCEF_TRACE;

    GetDefaultCapabilities( wantedCapabilities );
}

/*-------------------------------------------------------------------------*/

CIrcClientSettings::CIrcClientSettings( const CIrcClientSettings& src )
    : CORE::CIConfigurable( src )
    , server( src.server )
    , port( src.port )
    , useWebSocket( src.useWebSocket )
    , webSocketPath( src.webSocketPath )
    , nick( src.nick )
    , alternativeNicks( src.alternativeNicks )
    , userName( src.userName )
    , realName( src.realName )
    , saslAccount( src.saslAccount )
    , saslPassword( src.saslPassword )
    , wantedCapabilities( src.wantedCapabilities )
    , autoJoinChannels( src.autoJoinChannels )
    , pingTimeoutInSecs( src.pingTimeoutInSecs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcClientSettings::~CIrcClientSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcClientSettings&
CIrcClientSettings::operator=( const CIrcClientSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        server = src.server;
        port = src.port;
        useWebSocket = src.useWebSocket;
        webSocketPath = src.webSocketPath;
        nick = src.nick;
        alternativeNicks = src.alternativeNicks;
        userName = src.userName;
        realName = src.realName;
        saslAccount = src.saslAccount;
        saslPassword = src.saslPassword;
        wantedCapabilities = src.wantedCapabilities;
        autoJoinChannels = src.autoJoinChannels;
        pingTimeoutInSecs = src.pingTimeoutInSecs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CIrcClientSettings::GetDefaultCapabilities( CORE::CString::StringVector& capabilities )
{GUCEF_TRACE;

    capabilities.clear();
    capabilities.push_back( "sasl" );
    capabilities.push_back( "message-tags" );
    capabilities.push_back( "server-time" );
    capabilities.push_back( "echo-message" );
    capabilities.push_back( "account-tag" );
    capabilities.push_back( "batch" );
    capabilities.push_back( "chathistory" );
    capabilities.push_back( "multi-prefix" );
}

/*-------------------------------------------------------------------------*/

bool
CIrcClientSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "server", server );
    config.SetAttribute( "port", port );
    config.SetAttribute( "useWebSocket", useWebSocket );
    config.SetAttribute( "webSocketPath", webSocketPath );
    config.SetAttribute( "nick", nick );
    config.SetAttribute( "alternativeNicks", JoinList( alternativeNicks ) );
    config.SetAttribute( "userName", userName );
    config.SetAttribute( "realName", realName );
    config.SetAttribute( "saslAccount", saslAccount );
    config.SetAttribute( "saslPassword", saslPassword );
    config.SetAttribute( "wantedCapabilities", JoinList( wantedCapabilities ) );
    config.SetAttribute( "autoJoinChannels", JoinList( autoJoinChannels ) );
    config.SetAttribute( "pingTimeoutInSecs", pingTimeoutInSecs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CIrcClientSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    server = config.GetAttributeValueOrChildValueByName( "server" ).AsString( server, true );
    port = config.GetAttributeValueOrChildValueByName( "port" ).AsUInt16( port, true );
    useWebSocket = config.GetAttributeValueOrChildValueByName( "useWebSocket" ).AsBool( useWebSocket, true );
    webSocketPath = config.GetAttributeValueOrChildValueByName( "webSocketPath" ).AsString( webSocketPath, true );
    nick = config.GetAttributeValueOrChildValueByName( "nick" ).AsString( nick, true );
    ReadList( config, "alternativeNicks", alternativeNicks );
    userName = config.GetAttributeValueOrChildValueByName( "userName" ).AsString( userName, true );
    realName = config.GetAttributeValueOrChildValueByName( "realName" ).AsString( realName, true );
    saslAccount = config.GetAttributeValueOrChildValueByName( "saslAccount" ).AsString( saslAccount, true );
    saslPassword = config.GetAttributeValueOrChildValueByName( "saslPassword" ).AsString( saslPassword, true );
    ReadList( config, "wantedCapabilities", wantedCapabilities );
    ReadList( config, "autoJoinChannels", autoJoinChannels );
    pingTimeoutInSecs = config.GetAttributeValueOrChildValueByName( "pingTimeoutInSecs" ).AsUInt32( pingTimeoutInSecs, true );
    return !server.IsNULLOrEmpty() && !nick.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcClientSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CIrcClientSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
