/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNNodeSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNNodeSettings::ClassTypeName = "GUCEF::DRN::CDRNNodeSettings";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNNodeSettings::CDRNNodeSettings( void )
    : CORE::CIConfigurable()
    , nodeId( GenerateNodeId() )
    , serviceName( "drn" )
    , listenEndpoints()
    , relayEnabled( false )
    , relayToken()
    , relayMaxBytesPerSecondPerRealm( 0 )
    , echoIntervalMs( 250 )
    , latencyUpdateIntervalMs( 1000 )
    , handshakeTimeoutMs( 5000 )
    , linkTimeoutMs( 10000 )
    , connectTimeoutMs( 2000 )
    , ackDelayMs( 5 )
    , pulseIntervalMs( 5 )
    , ownThread( false )
    , firstSequence( 1 )
    , auth()
    , securityPolicy()
    , staticPrivateKey()
    , staticKeyFile()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNodeSettings::CDRNNodeSettings( const CDRNNodeSettings& src )
    : CORE::CIConfigurable( src )
    , nodeId( src.nodeId )
    , serviceName( src.serviceName )
    , listenEndpoints( src.listenEndpoints )
    , relayEnabled( src.relayEnabled )
    , relayToken( src.relayToken )
    , relayMaxBytesPerSecondPerRealm( src.relayMaxBytesPerSecondPerRealm )
    , echoIntervalMs( src.echoIntervalMs )
    , latencyUpdateIntervalMs( src.latencyUpdateIntervalMs )
    , handshakeTimeoutMs( src.handshakeTimeoutMs )
    , linkTimeoutMs( src.linkTimeoutMs )
    , connectTimeoutMs( src.connectTimeoutMs )
    , ackDelayMs( src.ackDelayMs )
    , pulseIntervalMs( src.pulseIntervalMs )
    , ownThread( src.ownThread )
    , firstSequence( src.firstSequence )
    , auth( src.auth )
    , securityPolicy( src.securityPolicy )
    , staticPrivateKey( src.staticPrivateKey )
    , staticKeyFile( src.staticKeyFile )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNodeSettings::~CDRNNodeSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNNodeSettings&
CDRNNodeSettings::operator=( const CDRNNodeSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        nodeId = src.nodeId;
        serviceName = src.serviceName;
        listenEndpoints = src.listenEndpoints;
        relayEnabled = src.relayEnabled;
        relayToken = src.relayToken;
        relayMaxBytesPerSecondPerRealm = src.relayMaxBytesPerSecondPerRealm;
        echoIntervalMs = src.echoIntervalMs;
        latencyUpdateIntervalMs = src.latencyUpdateIntervalMs;
        handshakeTimeoutMs = src.handshakeTimeoutMs;
        linkTimeoutMs = src.linkTimeoutMs;
        connectTimeoutMs = src.connectTimeoutMs;
        ackDelayMs = src.ackDelayMs;
        pulseIntervalMs = src.pulseIntervalMs;
        ownThread = src.ownThread;
        firstSequence = src.firstSequence;
        auth = src.auth;
        securityPolicy = src.securityPolicy;
        staticPrivateKey = src.staticPrivateKey;
        staticKeyFile = src.staticKeyFile;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNNodeSettings::GenerateNodeId( void )
{GUCEF_TRACE;

    UInt8 bytes[ 8 ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, sizeof( bytes ) ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DRNNodeSettings: No randomness for a node id" );
        return CORE::CString::Empty;
    }
    return CORE::Base16Encode( bytes, sizeof( bytes ) );
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "nodeId", nodeId ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "serviceName", serviceName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "relayEnabled", relayEnabled ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "relayToken", relayToken ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "relayMaxBytesPerSecondPerRealm", relayMaxBytesPerSecondPerRealm ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "echoIntervalMs", echoIntervalMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "latencyUpdateIntervalMs", latencyUpdateIntervalMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "handshakeTimeoutMs", handshakeTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "linkTimeoutMs", linkTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "connectTimeoutMs", connectTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "ackDelayMs", ackDelayMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "pulseIntervalMs", pulseIntervalMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "ownThread", ownThread ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "firstSequence", firstSequence ) && totalSuccess;
    if ( !staticPrivateKey.IsNULLOrEmpty() )
        totalSuccess = cfg.SetAttribute( "staticPrivateKey", staticPrivateKey ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "staticKeyFile", staticKeyFile ) && totalSuccess;

    CORE::CDataNode* listenNode = cfg.FindOrAddChild( "ListenEndpoints" );
    totalSuccess = GUCEF_NULL != listenNode && listenEndpoints.SaveConfig( *listenNode ) && totalSuccess;
    CORE::CDataNode* authNode = cfg.FindOrAddChild( "Auth" );
    totalSuccess = GUCEF_NULL != authNode && auth.SaveConfig( *authNode ) && totalSuccess;
    CORE::CDataNode* policyNode = cfg.FindOrAddChild( "SecurityPolicy" );
    totalSuccess = GUCEF_NULL != policyNode && securityPolicy.SaveConfig( *policyNode ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNNodeSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    nodeId = cfg.GetAttributeValueOrChildValueByName( "nodeId" ).AsString( nodeId, true );
    serviceName = cfg.GetAttributeValueOrChildValueByName( "serviceName" ).AsString( serviceName, true );
    relayEnabled = cfg.GetAttributeValueOrChildValueByName( "relayEnabled" ).AsBool( relayEnabled, true );
    relayToken = cfg.GetAttributeValueOrChildValueByName( "relayToken" ).AsString( relayToken, true );
    relayMaxBytesPerSecondPerRealm = cfg.GetAttributeValueOrChildValueByName( "relayMaxBytesPerSecondPerRealm" ).AsUInt32( relayMaxBytesPerSecondPerRealm, true );
    echoIntervalMs = cfg.GetAttributeValueOrChildValueByName( "echoIntervalMs" ).AsUInt32( echoIntervalMs, true );
    latencyUpdateIntervalMs = cfg.GetAttributeValueOrChildValueByName( "latencyUpdateIntervalMs" ).AsUInt32( latencyUpdateIntervalMs, true );
    handshakeTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "handshakeTimeoutMs" ).AsUInt32( handshakeTimeoutMs, true );
    linkTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "linkTimeoutMs" ).AsUInt32( linkTimeoutMs, true );
    connectTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "connectTimeoutMs" ).AsUInt32( connectTimeoutMs, true );
    ackDelayMs = cfg.GetAttributeValueOrChildValueByName( "ackDelayMs" ).AsUInt32( ackDelayMs, true );
    pulseIntervalMs = cfg.GetAttributeValueOrChildValueByName( "pulseIntervalMs" ).AsUInt32( pulseIntervalMs, true );
    ownThread = cfg.GetAttributeValueOrChildValueByName( "ownThread" ).AsBool( ownThread, true );
    firstSequence = cfg.GetAttributeValueOrChildValueByName( "firstSequence" ).AsUInt32( firstSequence, true );
    if ( 0 == firstSequence )
        firstSequence = 1;
    staticPrivateKey = cfg.GetAttributeValueOrChildValueByName( "staticPrivateKey" ).AsString( staticPrivateKey, true );
    staticKeyFile = cfg.GetAttributeValueOrChildValueByName( "staticKeyFile" ).AsString( staticKeyFile, true );

    const CORE::CDataNode* listenNode = cfg.FindChild( "ListenEndpoints" );
    if ( GUCEF_NULL != listenNode )
        listenEndpoints.LoadConfig( *listenNode );
    const CORE::CDataNode* authNode = cfg.FindChild( "Auth" );
    if ( GUCEF_NULL != authNode )
        auth.LoadConfig( *authNode );
    const CORE::CDataNode* policyNode = cfg.FindChild( "SecurityPolicy" );
    if ( GUCEF_NULL != policyNode )
        securityPolicy.LoadConfig( *policyNode );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNNodeSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
