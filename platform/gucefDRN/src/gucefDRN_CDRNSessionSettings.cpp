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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefDRN_CDRNSessionSettings.h"

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

const CORE::CString CDRNSessionSettings::ClassTypeName = "GUCEF::DRN::CDRNSessionSettings";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNSessionSettings::CDRNSessionSettings( void )
    : CORE::CIConfigurable()
    , sessionName()
    , topology( DRN_TOPOLOGY_CLIENT_SERVER )
    , maxMembers( 0 )
    , advertisedEndpoints()
    , joinTimeoutMs( 10000 )
    , maxBytesPerTickPerLink( 0 )
    , publishLatency( false )
    , latencyPublishIntervalMs( 1000 )
    , securityPolicy()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSessionSettings::CDRNSessionSettings( const CDRNSessionSettings& src )
    : CORE::CIConfigurable( src )
    , sessionName( src.sessionName )
    , topology( src.topology )
    , maxMembers( src.maxMembers )
    , advertisedEndpoints( src.advertisedEndpoints )
    , joinTimeoutMs( src.joinTimeoutMs )
    , maxBytesPerTickPerLink( src.maxBytesPerTickPerLink )
    , publishLatency( src.publishLatency )
    , latencyPublishIntervalMs( src.latencyPublishIntervalMs )
    , securityPolicy( src.securityPolicy )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSessionSettings::~CDRNSessionSettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSessionSettings&
CDRNSessionSettings::operator=( const CDRNSessionSettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        sessionName = src.sessionName;
        topology = src.topology;
        maxMembers = src.maxMembers;
        advertisedEndpoints = src.advertisedEndpoints;
        joinTimeoutMs = src.joinTimeoutMs;
        maxBytesPerTickPerLink = src.maxBytesPerTickPerLink;
        publishLatency = src.publishLatency;
        latencyPublishIntervalMs = src.latencyPublishIntervalMs;
        securityPolicy = src.securityPolicy;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNSessionSettings::TopologyToString( TDRNTopology topology )
{GUCEF_TRACE;

    return DRN_TOPOLOGY_PEER_TO_PEER == topology ? "PEER_TO_PEER" : "CLIENT_SERVER";
}

/*-------------------------------------------------------------------------*/

TDRNTopology
CDRNSessionSettings::StringToTopology( const CORE::CString& topology, TDRNTopology defaultTopology )
{GUCEF_TRACE;

    CORE::CString lower = topology.Lowercase();
    if ( lower == "peer_to_peer" || lower == "p2p" )
        return DRN_TOPOLOGY_PEER_TO_PEER;
    if ( lower == "client_server" )
        return DRN_TOPOLOGY_CLIENT_SERVER;
    return defaultTopology;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionSettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "sessionName", sessionName ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "topology", TopologyToString( topology ) ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxMembers", maxMembers ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "joinTimeoutMs", joinTimeoutMs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "maxBytesPerTickPerLink", maxBytesPerTickPerLink ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "publishLatency", publishLatency ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "latencyPublishIntervalMs", latencyPublishIntervalMs ) && totalSuccess;
    CORE::CDataNode* endpointsNode = cfg.FindOrAddChild( "AdvertisedEndpoints" );
    totalSuccess = GUCEF_NULL != endpointsNode && advertisedEndpoints.SaveConfig( *endpointsNode ) && totalSuccess;
    CORE::CDataNode* policyNode = cfg.FindOrAddChild( "SecurityPolicy" );
    totalSuccess = GUCEF_NULL != policyNode && securityPolicy.SaveConfig( *policyNode ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSessionSettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    sessionName = cfg.GetAttributeValueOrChildValueByName( "sessionName" ).AsString( sessionName, true );
    topology = StringToTopology( cfg.GetAttributeValueOrChildValueByName( "topology" ).AsString( TopologyToString( topology ), true ), topology );
    maxMembers = cfg.GetAttributeValueOrChildValueByName( "maxMembers" ).AsUInt32( maxMembers, true );
    joinTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "joinTimeoutMs" ).AsUInt32( joinTimeoutMs, true );
    maxBytesPerTickPerLink = cfg.GetAttributeValueOrChildValueByName( "maxBytesPerTickPerLink" ).AsUInt32( maxBytesPerTickPerLink, true );
    publishLatency = cfg.GetAttributeValueOrChildValueByName( "publishLatency" ).AsBool( publishLatency, true );
    latencyPublishIntervalMs = cfg.GetAttributeValueOrChildValueByName( "latencyPublishIntervalMs" ).AsUInt32( latencyPublishIntervalMs, true );
    const CORE::CDataNode* endpointsNode = cfg.FindChild( "AdvertisedEndpoints" );
    if ( GUCEF_NULL != endpointsNode )
        advertisedEndpoints.LoadConfig( *endpointsNode );
    const CORE::CDataNode* policyNode = cfg.FindChild( "SecurityPolicy" );
    if ( GUCEF_NULL != policyNode )
        securityPolicy.LoadConfig( *policyNode );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSessionSettings::GetClassTypeName( void ) const
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
