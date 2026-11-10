/*
 *  gucefCOM: GUCEF module providing communication implementations
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

#ifndef GUCEF_COM_CMCPREQUESTSTATESEALER_H
#include "gucefCOM_CMcpRequestStateSealer.h"
#define GUCEF_COM_CMCPREQUESTSTATESEALER_H
#endif /* GUCEF_COM_CMCPREQUESTSTATESEALER_H ? */

#include "gucefCOM_CMcpServerSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpServerSettings::ConfigSectionName = "McpServer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpServerSettings::CMcpServerSettings( void )
    : CORE::CIConfigurable()
    , serverName( "gucef-mcp-server" )
    , serverTitle()
    , serverVersion( "1.0.0" )
    , instructions()
    , includeServerInfo( true )
    , enableTools( true )
    , enableResources( true )
    , enablePrompts( true )
    , supportLegacy( true )
    , discoverTtlMs( 3600000 )
    , listTtlMs( 60000 )
    , pageSize( 100 )
    , maxMessageBytes( 4194304 )
    , maxJsonDepth( 64 )
    , maxInFlightPerChannel( 16 )
    , maxInFlightTotal( 64 )
    , toolCallsPerSecond( 10 )
    , toolCallBurst( 20 )
    , progressMinIntervalMs( 100 )
    , maxRequestDurationMs( 300000 )
    , cancelGraceMs( 2000 )
    , requestStateTtlMs( 600000 )
    , requestStateKeyFile()
    , requestStateProtection( "auto" )
    , requestStateAeadCodecs( CMcpRequestStateSealer::DefaultAeadCodecTypes.ParseElements( ',', false ) )
    , maxSubscriptionsPerChannel( 8 )
    , maxSubscriptionsTotal( 64 )
    , maxResourceSubscriptionsPerListen( 256 )
    , sseKeepAliveMs( 15000 )
    , validateStructuredOutput( true )
    , threadPoolName( "gucef.mcp" )
    , stopOnStdinClosed( true )
    , transports( GUCEF_DATATYPE_ARRAY )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CMcpServerSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serverName", CORE::CVariant( serverName ) );
    config.SetAttribute( "serverTitle", CORE::CVariant( serverTitle ) );
    config.SetAttribute( "serverVersion", CORE::CVariant( serverVersion ) );
    config.SetAttribute( "instructions", CORE::CVariant( instructions ) );
    config.SetAttribute( "requestStateKeyFile", CORE::CVariant( requestStateKeyFile ) );
    config.SetAttribute( "requestStateProtection", CORE::CVariant( requestStateProtection ) );
    config.SetAttribute( "threadPoolName", CORE::CVariant( threadPoolName ) );
    config.SetAttribute( "includeServerInfo", includeServerInfo );
    config.SetAttribute( "enableTools", enableTools );
    config.SetAttribute( "enableResources", enableResources );
    config.SetAttribute( "enablePrompts", enablePrompts );
    config.SetAttribute( "supportLegacy", supportLegacy );
    config.SetAttribute( "validateStructuredOutput", validateStructuredOutput );
    config.SetAttribute( "stopOnStdinClosed", stopOnStdinClosed );
    config.SetAttribute( "discoverTtlMs", discoverTtlMs );
    config.SetAttribute( "listTtlMs", listTtlMs );
    config.SetAttribute( "pageSize", pageSize );
    config.SetAttribute( "maxMessageBytes", maxMessageBytes );
    config.SetAttribute( "maxJsonDepth", maxJsonDepth );
    config.SetAttribute( "maxInFlightPerChannel", maxInFlightPerChannel );
    config.SetAttribute( "maxInFlightTotal", maxInFlightTotal );
    config.SetAttribute( "toolCallsPerSecond", toolCallsPerSecond );
    config.SetAttribute( "toolCallBurst", toolCallBurst );
    config.SetAttribute( "progressMinIntervalMs", progressMinIntervalMs );
    config.SetAttribute( "maxRequestDurationMs", maxRequestDurationMs );
    config.SetAttribute( "cancelGraceMs", cancelGraceMs );
    config.SetAttribute( "requestStateTtlMs", requestStateTtlMs );
    config.SetAttribute( "maxSubscriptionsPerChannel", maxSubscriptionsPerChannel );
    config.SetAttribute( "maxSubscriptionsTotal", maxSubscriptionsTotal );
    config.SetAttribute( "maxResourceSubscriptionsPerListen", maxResourceSubscriptionsPerListen );
    config.SetAttribute( "sseKeepAliveMs", sseKeepAliveMs );

    CORE::CString aeadCodecs;
    for ( size_t i=0; i<requestStateAeadCodecs.size(); ++i )
    {
        if ( i > 0 )
            aeadCodecs += ',';
        aeadCodecs += requestStateAeadCodecs[ i ];
    }
    config.SetAttribute( "requestStateAeadCodecs", CORE::CVariant( aeadCodecs ) );

    config.DelChild( "Transports" );
    if ( transports.HasChildren() )
    {
        CORE::CDataNode* node = config.AddChild( transports );
        node->SetName( "Transports" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpServerSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    serverName = config.GetAttributeValueOrChildValueByName( "serverName" ).AsString( serverName, true );
    serverTitle = config.GetAttributeValueOrChildValueByName( "serverTitle" ).AsString( serverTitle, true );
    serverVersion = config.GetAttributeValueOrChildValueByName( "serverVersion" ).AsString( serverVersion, true );
    instructions = config.GetAttributeValueOrChildValueByName( "instructions" ).AsString( instructions, true );
    requestStateKeyFile = config.GetAttributeValueOrChildValueByName( "requestStateKeyFile" ).AsString( requestStateKeyFile, true );
    requestStateProtection = config.GetAttributeValueOrChildValueByName( "requestStateProtection" ).AsString( requestStateProtection, true );
    threadPoolName = config.GetAttributeValueOrChildValueByName( "threadPoolName" ).AsString( threadPoolName, true );
    includeServerInfo = config.GetAttributeValueOrChildValueByName( "includeServerInfo" ).AsBool( includeServerInfo, true );
    enableTools = config.GetAttributeValueOrChildValueByName( "enableTools" ).AsBool( enableTools, true );
    enableResources = config.GetAttributeValueOrChildValueByName( "enableResources" ).AsBool( enableResources, true );
    enablePrompts = config.GetAttributeValueOrChildValueByName( "enablePrompts" ).AsBool( enablePrompts, true );
    supportLegacy = config.GetAttributeValueOrChildValueByName( "supportLegacy" ).AsBool( supportLegacy, true );
    validateStructuredOutput = config.GetAttributeValueOrChildValueByName( "validateStructuredOutput" ).AsBool( validateStructuredOutput, true );
    stopOnStdinClosed = config.GetAttributeValueOrChildValueByName( "stopOnStdinClosed" ).AsBool( stopOnStdinClosed, true );
    discoverTtlMs = config.GetAttributeValueOrChildValueByName( "discoverTtlMs" ).AsUInt32( discoverTtlMs, true );
    listTtlMs = config.GetAttributeValueOrChildValueByName( "listTtlMs" ).AsUInt32( listTtlMs, true );
    pageSize = config.GetAttributeValueOrChildValueByName( "pageSize" ).AsUInt32( pageSize, true );
    maxMessageBytes = config.GetAttributeValueOrChildValueByName( "maxMessageBytes" ).AsUInt32( maxMessageBytes, true );
    maxJsonDepth = config.GetAttributeValueOrChildValueByName( "maxJsonDepth" ).AsUInt32( maxJsonDepth, true );
    maxInFlightPerChannel = config.GetAttributeValueOrChildValueByName( "maxInFlightPerChannel" ).AsUInt32( maxInFlightPerChannel, true );
    maxInFlightTotal = config.GetAttributeValueOrChildValueByName( "maxInFlightTotal" ).AsUInt32( maxInFlightTotal, true );
    toolCallsPerSecond = config.GetAttributeValueOrChildValueByName( "toolCallsPerSecond" ).AsUInt32( toolCallsPerSecond, true );
    toolCallBurst = config.GetAttributeValueOrChildValueByName( "toolCallBurst" ).AsUInt32( toolCallBurst, true );
    progressMinIntervalMs = config.GetAttributeValueOrChildValueByName( "progressMinIntervalMs" ).AsUInt32( progressMinIntervalMs, true );
    maxRequestDurationMs = config.GetAttributeValueOrChildValueByName( "maxRequestDurationMs" ).AsUInt32( maxRequestDurationMs, true );
    cancelGraceMs = config.GetAttributeValueOrChildValueByName( "cancelGraceMs" ).AsUInt32( cancelGraceMs, true );
    requestStateTtlMs = config.GetAttributeValueOrChildValueByName( "requestStateTtlMs" ).AsUInt32( requestStateTtlMs, true );
    maxSubscriptionsPerChannel = config.GetAttributeValueOrChildValueByName( "maxSubscriptionsPerChannel" ).AsUInt32( maxSubscriptionsPerChannel, true );
    maxSubscriptionsTotal = config.GetAttributeValueOrChildValueByName( "maxSubscriptionsTotal" ).AsUInt32( maxSubscriptionsTotal, true );
    maxResourceSubscriptionsPerListen = config.GetAttributeValueOrChildValueByName( "maxResourceSubscriptionsPerListen" ).AsUInt32( maxResourceSubscriptionsPerListen, true );
    sseKeepAliveMs = config.GetAttributeValueOrChildValueByName( "sseKeepAliveMs" ).AsUInt32( sseKeepAliveMs, true );

    CORE::CString::StringVector aeadCodecs;
    if ( config.GetStringListByName( "requestStateAeadCodecs", aeadCodecs ) && !aeadCodecs.empty() )
        requestStateAeadCodecs = aeadCodecs;

    const CORE::CDataNode* transportsNode = config.FindChild( "Transports" );
    if ( GUCEF_NULL != transportsNode )
    {
        transports = *transportsNode;
        transports.SetName( CORE::CString::Empty );
    }

    CMcpRequestStateSealer::EProtection protection;
    return pageSize > 0 && CMcpRequestStateSealer::ParseProtection( requestStateProtection, protection );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpServerSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpServerSettings";
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
