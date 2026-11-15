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

#include "gucefDRNUTILS_CRemoteServiceHostSettings.h"

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

namespace {

const char* const KnownKeys[] = { "serviceName", "serviceVersion", "drnSessionName", "sessionPort", "pairingPort", "bindAddress", "sessionEndpoints", "pairingEndpoints", "linkTimeoutMs", "echoIntervalMs", "handshakeTimeoutMs", "maxPendingHandshakes", "maxHandshakesPerAddressPerMinute", "tokenLifetimeDays", "renewBeforeDays", "resumeGraceSeconds", "maxSessions", "maxSessionsPerDevice", "maxBulkLinksPerSession", "eventRetentionEvents", "eventRetentionBytes", "sessionBudgetBytes", "controlWindowBytes", "controlMaxMessageBytes", "controlSendQueueBytes", "bulkWindowBytes", "bulkMaxMessageBytes", "bulkSendQueueBytes", "maxConcurrentCalls", "pairingDefaultSeconds", "pairingMaxSeconds", "maxFailedProofs", "banAfterFailedProofs", "banMinutes", "confirmationTimeoutSeconds", "requireConfirmationForRoles", "adminRoles", "ownerUserId", "drainSeconds", "expectedBackSeconds", "tickIntervalMs", "staticPrivateKeyHex", "issuerSeedHex", "lanDiscovery", "lanDiscoveryAdvertiser", "lanDiscoveryKeyFingerprint", "lanDiscoveryPairingPort", "lanDiscoveryProtocolVersion", GUCEF_NULL };

/*-------------------------------------------------------------------------*/

void
SaveList( CORE::CDataNode& config, const char* name, const CORE::CString::StringVector& values )
{GUCEF_TRACE;

    CORE::CDataNode* list = config.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( UInt32 i=0; i<values.size(); ++i )
        list->AddValueAsChild( CORE::CVariant( values[ i ] ) );
}

/*-------------------------------------------------------------------------*/

void
SaveList( CORE::CDataNode& config, const char* name, const CORE::CString::StringSet& values )
{GUCEF_TRACE;

    CORE::CDataNode* list = config.AddChild( name, GUCEF_DATATYPE_ARRAY );
    for ( CORE::CString::StringSet::const_iterator i = values.begin(); i != values.end(); ++i )
        list->AddValueAsChild( CORE::CVariant( (*i) ) );
}

/*-------------------------------------------------------------------------*/

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

/*-------------------------------------------------------------------------*/

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

}

/*-------------------------------------------------------------------------*/

CRemoteServiceHostSettings::CRemoteServiceHostSettings( void )
    : CORE::CIConfigurable()
    , serviceName( "gucef.service" )
    , serviceVersion( "1.0.0" )
    , drnSessionName( "service" )
    , sessionPort( 0 )
    , pairingPort( 0 )
    , bindAddress( "" )
    , sessionEndpoints(  )
    , pairingEndpoints(  )
    , linkTimeoutMs( 15000 )
    , echoIntervalMs( 1000 )
    , handshakeTimeoutMs( 5000 )
    , maxPendingHandshakes( 64 )
    , maxHandshakesPerAddressPerMinute( 60 )
    , tokenLifetimeDays( 90 )
    , renewBeforeDays( 30 )
    , resumeGraceSeconds( 600 )
    , maxSessions( 64 )
    , maxSessionsPerDevice( 4 )
    , maxBulkLinksPerSession( 2 )
    , eventRetentionEvents( 4096 )
    , eventRetentionBytes( 2097152 )
    , sessionBudgetBytes( 268435456 )
    , controlWindowBytes( 1048576 )
    , controlMaxMessageBytes( 4194304 )
    , controlSendQueueBytes( 4194304 )
    , bulkWindowBytes( 8388608 )
    , bulkMaxMessageBytes( 1048576 )
    , bulkSendQueueBytes( 16777216 )
    , maxConcurrentCalls( 64 )
    , pairingDefaultSeconds( 300 )
    , pairingMaxSeconds( 900 )
    , maxFailedProofs( 5 )
    , banAfterFailedProofs( 3 )
    , banMinutes( 15 )
    , confirmationTimeoutSeconds( 300 )
    , requireConfirmationForRoles(  )
    , adminRoles(  )
    , ownerUserId( "owner" )
    , drainSeconds( 20 )
    , expectedBackSeconds( 30 )
    , tickIntervalMs( 250 )
    , staticPrivateKeyHex( "" )
    , issuerSeedHex( "" )
    , lanDiscovery( true )
    , lanDiscoveryAdvertiser()
    , lanDiscoveryKeyFingerprint( true )
    , lanDiscoveryPairingPort( true )
    , lanDiscoveryProtocolVersion( "" )
{GUCEF_TRACE;

    lanDiscoveryAdvertiser.serviceType = "_gucef-service._tcp";

    requireConfirmationForRoles.insert( "admin" );
    adminRoles.insert( "admin" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteServiceHostSettings::GetPairingSessionName( void ) const
{GUCEF_TRACE;

    return drnSessionName + ".pair";
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHostSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serviceName", serviceName );
    config.SetAttribute( "serviceVersion", serviceVersion );
    config.SetAttribute( "drnSessionName", drnSessionName );
    config.SetAttribute( "sessionPort", sessionPort );
    config.SetAttribute( "pairingPort", pairingPort );
    config.SetAttribute( "bindAddress", bindAddress );
    SaveList( config, "sessionEndpoints", sessionEndpoints );
    SaveList( config, "pairingEndpoints", pairingEndpoints );
    config.SetAttribute( "linkTimeoutMs", linkTimeoutMs );
    config.SetAttribute( "echoIntervalMs", echoIntervalMs );
    config.SetAttribute( "handshakeTimeoutMs", handshakeTimeoutMs );
    config.SetAttribute( "maxPendingHandshakes", maxPendingHandshakes );
    config.SetAttribute( "maxHandshakesPerAddressPerMinute", maxHandshakesPerAddressPerMinute );
    config.SetAttribute( "tokenLifetimeDays", tokenLifetimeDays );
    config.SetAttribute( "renewBeforeDays", renewBeforeDays );
    config.SetAttribute( "resumeGraceSeconds", resumeGraceSeconds );
    config.SetAttribute( "maxSessions", maxSessions );
    config.SetAttribute( "maxSessionsPerDevice", maxSessionsPerDevice );
    config.SetAttribute( "maxBulkLinksPerSession", maxBulkLinksPerSession );
    config.SetAttribute( "eventRetentionEvents", eventRetentionEvents );
    config.SetAttribute( "eventRetentionBytes", eventRetentionBytes );
    config.SetAttribute( "sessionBudgetBytes", sessionBudgetBytes );
    config.SetAttribute( "controlWindowBytes", controlWindowBytes );
    config.SetAttribute( "controlMaxMessageBytes", controlMaxMessageBytes );
    config.SetAttribute( "controlSendQueueBytes", controlSendQueueBytes );
    config.SetAttribute( "bulkWindowBytes", bulkWindowBytes );
    config.SetAttribute( "bulkMaxMessageBytes", bulkMaxMessageBytes );
    config.SetAttribute( "bulkSendQueueBytes", bulkSendQueueBytes );
    config.SetAttribute( "maxConcurrentCalls", maxConcurrentCalls );
    config.SetAttribute( "pairingDefaultSeconds", pairingDefaultSeconds );
    config.SetAttribute( "pairingMaxSeconds", pairingMaxSeconds );
    config.SetAttribute( "maxFailedProofs", maxFailedProofs );
    config.SetAttribute( "banAfterFailedProofs", banAfterFailedProofs );
    config.SetAttribute( "banMinutes", banMinutes );
    config.SetAttribute( "confirmationTimeoutSeconds", confirmationTimeoutSeconds );
    SaveList( config, "requireConfirmationForRoles", requireConfirmationForRoles );
    SaveList( config, "adminRoles", adminRoles );
    config.SetAttribute( "ownerUserId", ownerUserId );
    config.SetAttribute( "drainSeconds", drainSeconds );
    config.SetAttribute( "expectedBackSeconds", expectedBackSeconds );
    config.SetAttribute( "tickIntervalMs", tickIntervalMs );
    config.SetAttribute( "staticPrivateKeyHex", staticPrivateKeyHex );
    config.SetAttribute( "issuerSeedHex", issuerSeedHex );
    config.SetAttribute( "lanDiscovery", lanDiscovery );
    CORE::CDataNode* advertiser = config.AddChild( "lanDiscoveryAdvertiser", GUCEF_DATATYPE_OBJECT );
    lanDiscoveryAdvertiser.SaveConfig( *advertiser );
    config.SetAttribute( "lanDiscoveryKeyFingerprint", lanDiscoveryKeyFingerprint );
    config.SetAttribute( "lanDiscoveryPairingPort", lanDiscoveryPairingPort );
    config.SetAttribute( "lanDiscoveryProtocolVersion", lanDiscoveryProtocolVersion );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteServiceHostSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    // a key that is not a setting is a mistake of the configuration: refused, nothing is taken
    for ( CORE::CDataNode::TAttributeMap::const_iterator a = config.AttributeBegin(); a != config.AttributeEnd(); ++a )
    {
        if ( !IsKnownKey( (*a).first ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceHostSettings: the setting \"" << (*a).first << "\" is not known" );
            return false;
        }
    }
    for ( CORE::CDataNode::const_iterator c = config.ConstBegin(); c != config.ConstEnd(); ++c )
    {
        if ( !IsKnownKey( (*c)->GetName() ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceHostSettings: the setting \"" << (*c)->GetName() << "\" is not known" );
            return false;
        }
    }

    serviceName = config.GetAttributeValueOrChildValueByName( "serviceName" ).AsString( serviceName, true );
    serviceVersion = config.GetAttributeValueOrChildValueByName( "serviceVersion" ).AsString( serviceVersion, true );
    drnSessionName = config.GetAttributeValueOrChildValueByName( "drnSessionName" ).AsString( drnSessionName, true );
    sessionPort = (UInt16) config.GetAttributeValueOrChildValueByName( "sessionPort" ).AsUInt32( sessionPort, true );
    pairingPort = (UInt16) config.GetAttributeValueOrChildValueByName( "pairingPort" ).AsUInt32( pairingPort, true );
    bindAddress = config.GetAttributeValueOrChildValueByName( "bindAddress" ).AsString( bindAddress, true );
    LoadList( config, "sessionEndpoints", sessionEndpoints );
    LoadList( config, "pairingEndpoints", pairingEndpoints );
    linkTimeoutMs = config.GetAttributeValueOrChildValueByName( "linkTimeoutMs" ).AsUInt32( linkTimeoutMs, true );
    echoIntervalMs = config.GetAttributeValueOrChildValueByName( "echoIntervalMs" ).AsUInt32( echoIntervalMs, true );
    handshakeTimeoutMs = config.GetAttributeValueOrChildValueByName( "handshakeTimeoutMs" ).AsUInt32( handshakeTimeoutMs, true );
    maxPendingHandshakes = config.GetAttributeValueOrChildValueByName( "maxPendingHandshakes" ).AsUInt32( maxPendingHandshakes, true );
    maxHandshakesPerAddressPerMinute = config.GetAttributeValueOrChildValueByName( "maxHandshakesPerAddressPerMinute" ).AsUInt32( maxHandshakesPerAddressPerMinute, true );
    tokenLifetimeDays = config.GetAttributeValueOrChildValueByName( "tokenLifetimeDays" ).AsUInt32( tokenLifetimeDays, true );
    renewBeforeDays = config.GetAttributeValueOrChildValueByName( "renewBeforeDays" ).AsUInt32( renewBeforeDays, true );
    resumeGraceSeconds = config.GetAttributeValueOrChildValueByName( "resumeGraceSeconds" ).AsUInt32( resumeGraceSeconds, true );
    maxSessions = config.GetAttributeValueOrChildValueByName( "maxSessions" ).AsUInt32( maxSessions, true );
    maxSessionsPerDevice = config.GetAttributeValueOrChildValueByName( "maxSessionsPerDevice" ).AsUInt32( maxSessionsPerDevice, true );
    maxBulkLinksPerSession = config.GetAttributeValueOrChildValueByName( "maxBulkLinksPerSession" ).AsUInt32( maxBulkLinksPerSession, true );
    eventRetentionEvents = config.GetAttributeValueOrChildValueByName( "eventRetentionEvents" ).AsUInt32( eventRetentionEvents, true );
    eventRetentionBytes = config.GetAttributeValueOrChildValueByName( "eventRetentionBytes" ).AsUInt64( eventRetentionBytes, true );
    sessionBudgetBytes = config.GetAttributeValueOrChildValueByName( "sessionBudgetBytes" ).AsUInt64( sessionBudgetBytes, true );
    controlWindowBytes = config.GetAttributeValueOrChildValueByName( "controlWindowBytes" ).AsUInt32( controlWindowBytes, true );
    controlMaxMessageBytes = config.GetAttributeValueOrChildValueByName( "controlMaxMessageBytes" ).AsUInt32( controlMaxMessageBytes, true );
    controlSendQueueBytes = config.GetAttributeValueOrChildValueByName( "controlSendQueueBytes" ).AsUInt32( controlSendQueueBytes, true );
    bulkWindowBytes = config.GetAttributeValueOrChildValueByName( "bulkWindowBytes" ).AsUInt32( bulkWindowBytes, true );
    bulkMaxMessageBytes = config.GetAttributeValueOrChildValueByName( "bulkMaxMessageBytes" ).AsUInt32( bulkMaxMessageBytes, true );
    bulkSendQueueBytes = config.GetAttributeValueOrChildValueByName( "bulkSendQueueBytes" ).AsUInt32( bulkSendQueueBytes, true );
    maxConcurrentCalls = config.GetAttributeValueOrChildValueByName( "maxConcurrentCalls" ).AsUInt32( maxConcurrentCalls, true );
    pairingDefaultSeconds = config.GetAttributeValueOrChildValueByName( "pairingDefaultSeconds" ).AsUInt32( pairingDefaultSeconds, true );
    pairingMaxSeconds = config.GetAttributeValueOrChildValueByName( "pairingMaxSeconds" ).AsUInt32( pairingMaxSeconds, true );
    maxFailedProofs = config.GetAttributeValueOrChildValueByName( "maxFailedProofs" ).AsUInt32( maxFailedProofs, true );
    banAfterFailedProofs = config.GetAttributeValueOrChildValueByName( "banAfterFailedProofs" ).AsUInt32( banAfterFailedProofs, true );
    banMinutes = config.GetAttributeValueOrChildValueByName( "banMinutes" ).AsUInt32( banMinutes, true );
    confirmationTimeoutSeconds = config.GetAttributeValueOrChildValueByName( "confirmationTimeoutSeconds" ).AsUInt32( confirmationTimeoutSeconds, true );
    { CORE::CString::StringVector list; if ( LoadList( config, "requireConfirmationForRoles", list ) ) { requireConfirmationForRoles.clear(); requireConfirmationForRoles.insert( list.begin(), list.end() ); } }
    { CORE::CString::StringVector list; if ( LoadList( config, "adminRoles", list ) ) { adminRoles.clear(); adminRoles.insert( list.begin(), list.end() ); } }
    ownerUserId = config.GetAttributeValueOrChildValueByName( "ownerUserId" ).AsString( ownerUserId, true );
    drainSeconds = config.GetAttributeValueOrChildValueByName( "drainSeconds" ).AsUInt32( drainSeconds, true );
    expectedBackSeconds = config.GetAttributeValueOrChildValueByName( "expectedBackSeconds" ).AsUInt32( expectedBackSeconds, true );
    tickIntervalMs = config.GetAttributeValueOrChildValueByName( "tickIntervalMs" ).AsUInt32( tickIntervalMs, true );
    staticPrivateKeyHex = config.GetAttributeValueOrChildValueByName( "staticPrivateKeyHex" ).AsString( staticPrivateKeyHex, true );
    issuerSeedHex = config.GetAttributeValueOrChildValueByName( "issuerSeedHex" ).AsString( issuerSeedHex, true );
    lanDiscovery = config.GetAttributeValueOrChildValueByName( "lanDiscovery" ).AsBool( lanDiscovery, true );
    const CORE::CDataNode* advertiser = config.FindChild( "lanDiscoveryAdvertiser" );
    if ( GUCEF_NULL != advertiser && !lanDiscoveryAdvertiser.LoadConfig( *advertiser ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RemoteServiceHostSettings: the setting \"lanDiscoveryAdvertiser\" has problems" );
        return false;
    }
    lanDiscoveryKeyFingerprint = config.GetAttributeValueOrChildValueByName( "lanDiscoveryKeyFingerprint" ).AsBool( lanDiscoveryKeyFingerprint, true );
    lanDiscoveryPairingPort = config.GetAttributeValueOrChildValueByName( "lanDiscoveryPairingPort" ).AsBool( lanDiscoveryPairingPort, true );
    lanDiscoveryProtocolVersion = config.GetAttributeValueOrChildValueByName( "lanDiscoveryProtocolVersion" ).AsString( lanDiscoveryProtocolVersion, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteServiceHostSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CRemoteServiceHostSettings";
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
