/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRNUTILS_CRemoteServiceConnectionSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteServiceConnectionSettings::CRemoteServiceConnectionSettings( void )
    : CORE::CIConfigurable()
    , clientKind( "client" )
    , clientVersion()
    , capabilities()
    , wantedContracts()
    , bulkContracts()
    , bulkChannels( 0 )
    , hostOverride()
    , connectTimeoutMs( 20000 )
    , linkTimeoutMs( 15000 )
    , echoIntervalMs( 1000 )
    , resumeGraceMs( 600000 )
    , controlWindowBytes( 1024 * 1024 )
    , controlMaxMessageBytes( 4 * 1024 * 1024 )
    , controlSendQueueBytes( 4 * 1024 * 1024 )
    , bulkWindowBytes( 8 * 1024 * 1024 )
    , bulkMaxMessageBytes( 1024 * 1024 )
    , bulkSendQueueBytes( 16 * 1024 * 1024 )
    , autoReconnect( true )
    , reconnectFirstMaxMs( 250 )
    , reconnectBaseMs( 500 )
    , reconnectMaxMs( 30000 )
    , reconnectStableResetMs( 60000 )
    , wakeDetectMs( 5000 )
    , clockSkewNoticeMs( 2000 )
    , openEventStream( true )
    , tickIntervalMs( 10 )
    , lanDiscovery( true )
    , lanDiscoveryFinder()
{GUCEF_TRACE;

    lanDiscoveryFinder.serviceType = "_gucef-service._tcp";
}

/*-------------------------------------------------------------------------*/

namespace {

const char* const KnownKeys[] = { "clientKind", "clientVersion", "capabilities", "wantedContracts", "bulkContracts", "bulkChannels", "hostOverride", "connectTimeoutMs", "linkTimeoutMs",
                                  "echoIntervalMs", "resumeGraceMs", "controlWindowBytes", "controlMaxMessageBytes", "controlSendQueueBytes", "bulkWindowBytes", "bulkMaxMessageBytes",
                                  "bulkSendQueueBytes", "autoReconnect", "reconnectFirstMaxMs", "reconnectBaseMs", "reconnectMaxMs", "reconnectStableResetMs", "wakeDetectMs",
                                  "clockSkewNoticeMs", "openEventStream", "tickIntervalMs", "lanDiscovery", "lanDiscoveryFinder", GUCEF_NULL };

bool
IsKnownKey( const CORE::CString& key )
{GUCEF_TRACE;

    for ( UInt32 i=0; GUCEF_NULL != KnownKeys[ i ]; ++i )
    {
        if ( key == KnownKeys[ i ] )
            return true;
    }
    return false;
}

void
SaveList( CORE::CDataNode& config, const char* name, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CDataNode* list = config.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( UInt32 i=0; i<values.size(); ++i )
        list->AddValueAsChild( CORE::CVariant( values[ i ] ) );
}

/** A list from the children of the named child, or from a comma separated attribute; false when neither is there */
bool
LoadList( const CORE::CDataNode& config, const char* name, CORE::CString::StringVector& values )
{GUCEF_TRACE;

    const CORE::CDataNode* child = config.FindChild( name );
    if ( GUCEF_NULL != child && child->GetNrOfChildNodes() > 0 )
    {
        values.clear();
        for ( CORE::CDataNode::const_iterator i = child->ConstBegin(); i != child->ConstEnd(); ++i )
        {
            CORE::CString value = (*i)->GetValue().AsString().Trim( true ).Trim( false );
            if ( !value.IsNULLOrEmpty() )
                values.push_back( value );
        }
        return true;
    }
    CORE::CVariant text = config.GetAttributeValueOrChildValueByName( name );
    if ( text.IsNULLOrEmpty() )
        return GUCEF_NULL != child;
    values.clear();
    CORE::CString::StringVector parts = text.AsString().ParseElements( ',', false );
    for ( UInt32 i=0; i<parts.size(); ++i )
    {
        CORE::CString value = parts[ i ].Trim( true ).Trim( false );
        if ( !value.IsNULLOrEmpty() )
            values.push_back( value );
    }
    return true;
}

}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnectionSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "clientKind", clientKind );
    config.SetAttribute( "clientVersion", clientVersion );
    SaveList( config, "capabilities", capabilities );
    SaveList( config, "wantedContracts", wantedContracts );
    SaveList( config, "bulkContracts", bulkContracts );
    config.SetAttribute( "bulkChannels", bulkChannels );
    config.SetAttribute( "hostOverride", hostOverride );
    config.SetAttribute( "connectTimeoutMs", connectTimeoutMs );
    config.SetAttribute( "linkTimeoutMs", linkTimeoutMs );
    config.SetAttribute( "echoIntervalMs", echoIntervalMs );
    config.SetAttribute( "resumeGraceMs", resumeGraceMs );
    config.SetAttribute( "controlWindowBytes", controlWindowBytes );
    config.SetAttribute( "controlMaxMessageBytes", controlMaxMessageBytes );
    config.SetAttribute( "controlSendQueueBytes", controlSendQueueBytes );
    config.SetAttribute( "bulkWindowBytes", bulkWindowBytes );
    config.SetAttribute( "bulkMaxMessageBytes", bulkMaxMessageBytes );
    config.SetAttribute( "bulkSendQueueBytes", bulkSendQueueBytes );
    config.SetAttribute( "autoReconnect", autoReconnect );
    config.SetAttribute( "reconnectFirstMaxMs", reconnectFirstMaxMs );
    config.SetAttribute( "reconnectBaseMs", reconnectBaseMs );
    config.SetAttribute( "reconnectMaxMs", reconnectMaxMs );
    config.SetAttribute( "reconnectStableResetMs", reconnectStableResetMs );
    config.SetAttribute( "wakeDetectMs", wakeDetectMs );
    config.SetAttribute( "clockSkewNoticeMs", clockSkewNoticeMs );
    config.SetAttribute( "openEventStream", openEventStream );
    config.SetAttribute( "tickIntervalMs", tickIntervalMs );
    config.SetAttribute( "lanDiscovery", lanDiscovery );
    CORE::CDataNode* finder = config.AddChild( "lanDiscoveryFinder", GUCEF_DATATYPE_OBJECT );
    lanDiscoveryFinder.SaveConfig( *finder );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceConnectionSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    // a key that is not a setting is a mistake of the configuration: refused, nothing is taken
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = config.AttributeBegin(); a != config.AttributeEnd(); ++a )
    {
        if ( !IsKnownKey( (*a).first ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceConnectionSettings: the setting \"" << (*a).first << "\" is not known" );
            return false;
        }
    }
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        if ( !IsKnownKey( (*c)->GetName() ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceConnectionSettings: the setting \"" << (*c)->GetName() << "\" is not known" );
            return false;
        }
    }

    // taken on a copy and kept only when all of it is usable
    CRemoteServiceConnectionSettings loaded( *this );
    loaded.clientKind = config.GetAttributeValueOrChildValueByName( "clientKind" ).AsString( clientKind, true );
    loaded.clientVersion = config.GetAttributeValueOrChildValueByName( "clientVersion" ).AsString( clientVersion, true );
    LoadList( config, "capabilities", loaded.capabilities );
    LoadList( config, "wantedContracts", loaded.wantedContracts );
    LoadList( config, "bulkContracts", loaded.bulkContracts );
    loaded.bulkChannels = config.GetAttributeValueOrChildValueByName( "bulkChannels" ).AsUInt32( bulkChannels, true );
    loaded.hostOverride = config.GetAttributeValueOrChildValueByName( "hostOverride" ).AsString( hostOverride, true );
    loaded.connectTimeoutMs = config.GetAttributeValueOrChildValueByName( "connectTimeoutMs" ).AsUInt32( connectTimeoutMs, true );
    loaded.linkTimeoutMs = config.GetAttributeValueOrChildValueByName( "linkTimeoutMs" ).AsUInt32( linkTimeoutMs, true );
    loaded.echoIntervalMs = config.GetAttributeValueOrChildValueByName( "echoIntervalMs" ).AsUInt32( echoIntervalMs, true );
    loaded.resumeGraceMs = config.GetAttributeValueOrChildValueByName( "resumeGraceMs" ).AsUInt32( resumeGraceMs, true );
    loaded.controlWindowBytes = config.GetAttributeValueOrChildValueByName( "controlWindowBytes" ).AsUInt32( controlWindowBytes, true );
    loaded.controlMaxMessageBytes = config.GetAttributeValueOrChildValueByName( "controlMaxMessageBytes" ).AsUInt32( controlMaxMessageBytes, true );
    loaded.controlSendQueueBytes = config.GetAttributeValueOrChildValueByName( "controlSendQueueBytes" ).AsUInt32( controlSendQueueBytes, true );
    loaded.bulkWindowBytes = config.GetAttributeValueOrChildValueByName( "bulkWindowBytes" ).AsUInt32( bulkWindowBytes, true );
    loaded.bulkMaxMessageBytes = config.GetAttributeValueOrChildValueByName( "bulkMaxMessageBytes" ).AsUInt32( bulkMaxMessageBytes, true );
    loaded.bulkSendQueueBytes = config.GetAttributeValueOrChildValueByName( "bulkSendQueueBytes" ).AsUInt32( bulkSendQueueBytes, true );
    loaded.autoReconnect = config.GetAttributeValueOrChildValueByName( "autoReconnect" ).AsBool( autoReconnect, true );
    loaded.reconnectFirstMaxMs = config.GetAttributeValueOrChildValueByName( "reconnectFirstMaxMs" ).AsUInt32( reconnectFirstMaxMs, true );
    loaded.reconnectBaseMs = config.GetAttributeValueOrChildValueByName( "reconnectBaseMs" ).AsUInt32( reconnectBaseMs, true );
    loaded.reconnectMaxMs = config.GetAttributeValueOrChildValueByName( "reconnectMaxMs" ).AsUInt32( reconnectMaxMs, true );
    loaded.reconnectStableResetMs = config.GetAttributeValueOrChildValueByName( "reconnectStableResetMs" ).AsUInt32( reconnectStableResetMs, true );
    loaded.wakeDetectMs = config.GetAttributeValueOrChildValueByName( "wakeDetectMs" ).AsUInt32( wakeDetectMs, true );
    loaded.clockSkewNoticeMs = config.GetAttributeValueOrChildValueByName( "clockSkewNoticeMs" ).AsUInt32( clockSkewNoticeMs, true );
    loaded.openEventStream = config.GetAttributeValueOrChildValueByName( "openEventStream" ).AsBool( openEventStream, true );
    loaded.tickIntervalMs = config.GetAttributeValueOrChildValueByName( "tickIntervalMs" ).AsUInt32( tickIntervalMs, true );
    loaded.lanDiscovery = config.GetAttributeValueOrChildValueByName( "lanDiscovery" ).AsBool( lanDiscovery, true );
    const CORE::CDataNode* finder = config.FindChild( "lanDiscoveryFinder" );
    if ( GUCEF_NULL != finder && !loaded.lanDiscoveryFinder.LoadConfig( *finder ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceConnectionSettings: the setting \"lanDiscoveryFinder\" has problems" );
        return false;
    }
    if ( loaded.bulkChannels > 2 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceConnectionSettings: bulkChannels is 0, 1 or 2" );
        return false;
    }

    *this = loaded;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteServiceConnectionSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteServiceConnectionSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
