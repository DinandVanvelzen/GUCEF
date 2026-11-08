/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include "gucefCOMCORE_CRpcEndpointSettings.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRpcEndpointSettings::CRpcEndpointSettings( void )
    : CORE::CIConfigurable()
    , nodeName()
    , limits()
    , retentionMaxFrames( 8192 )
    , retentionMaxBytes( 16 * 1024 * 1024 )
    , sendQueueMaxBytes( 16 * 1024 * 1024 )
    , suspendedOverflow( RPC_SUSPENDEDOVERFLOW_REFUSE_NEW )
    , requireSecureTransport( false )
    , enableReflection( true )
    , preferPeerContracts( false )
    , clockSkewAllowanceMs( 2000 )
    , closeTimeoutMs( 5000 )
    , durableResultRetentionMs( 24 * 60 * 60 * 1000 )
    , maxDurableRequestsPerSubject( 1024 )
    , maxGetContractPerSession( 16 )
    , maxOpenAssemblies( 16 )
    , reconnectInitialDelayMs( 500 )
    , reconnectMaxDelayMs( 30000 )
    , reconnectFactor( 2 )
    , autoReconnect( true )
    , tickIntervalMs( 10 )
    , useTransportProfile( true )
    , profile()
    , threadPoolName( "gucef.rpc" )
    , threadPool()
    , contractRegistry( GUCEF_NULL )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpointSettings::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "nodeName", nodeName );
    config.SetAttribute( "maxFrameSize", limits.maxFrameSize );
    config.SetAttribute( "maxMessageSize", limits.maxMessageSize );
    config.SetAttribute( "receiveWindowBytes", limits.receiveWindowBytes );
    config.SetAttribute( "initialStreamCredit", limits.initialStreamCredit );
    config.SetAttribute( "maxConcurrentCalls", limits.maxConcurrentCalls );
    config.SetAttribute( "maxBindings", limits.maxBindings );
    config.SetAttribute( "resumeGraceMs", limits.resumeGraceMs );
    config.SetAttribute( "retentionMaxFrames", retentionMaxFrames );
    config.SetAttribute( "retentionMaxBytes", retentionMaxBytes );
    config.SetAttribute( "sendQueueMaxBytes", sendQueueMaxBytes );
    config.SetAttribute( "suspendedOverflow", RPC_SUSPENDEDOVERFLOW_DROP_OLDEST == suspendedOverflow ? "DROP_OLDEST" : "REFUSE_NEW" );
    config.SetAttribute( "requireSecureTransport", requireSecureTransport );
    config.SetAttribute( "enableReflection", enableReflection );
    config.SetAttribute( "preferPeerContracts", preferPeerContracts );
    config.SetAttribute( "clockSkewAllowanceMs", clockSkewAllowanceMs );
    config.SetAttribute( "closeTimeoutMs", closeTimeoutMs );
    config.SetAttribute( "durableResultRetentionMs", durableResultRetentionMs );
    config.SetAttribute( "maxDurableRequestsPerSubject", maxDurableRequestsPerSubject );
    config.SetAttribute( "autoReconnect", autoReconnect );
    config.SetAttribute( "reconnectInitialDelayMs", reconnectInitialDelayMs );
    config.SetAttribute( "reconnectMaxDelayMs", reconnectMaxDelayMs );
    config.SetAttribute( "threadPoolName", threadPoolName );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRpcEndpointSettings::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    nodeName = config.GetAttributeValueOrChildValueByName( "nodeName", nodeName ).AsString( nodeName );
    limits.maxFrameSize = config.GetAttributeValueOrChildValueByName( "maxFrameSize" ).AsUInt32( limits.maxFrameSize );
    limits.maxMessageSize = config.GetAttributeValueOrChildValueByName( "maxMessageSize" ).AsUInt32( limits.maxMessageSize );
    limits.receiveWindowBytes = config.GetAttributeValueOrChildValueByName( "receiveWindowBytes" ).AsUInt32( limits.receiveWindowBytes );
    limits.initialStreamCredit = config.GetAttributeValueOrChildValueByName( "initialStreamCredit" ).AsUInt32( limits.initialStreamCredit );
    limits.maxConcurrentCalls = config.GetAttributeValueOrChildValueByName( "maxConcurrentCalls" ).AsUInt32( limits.maxConcurrentCalls );
    limits.maxBindings = config.GetAttributeValueOrChildValueByName( "maxBindings" ).AsUInt32( limits.maxBindings );
    limits.resumeGraceMs = config.GetAttributeValueOrChildValueByName( "resumeGraceMs" ).AsUInt32( limits.resumeGraceMs );
    retentionMaxFrames = config.GetAttributeValueOrChildValueByName( "retentionMaxFrames" ).AsUInt32( retentionMaxFrames );
    retentionMaxBytes = config.GetAttributeValueOrChildValueByName( "retentionMaxBytes" ).AsUInt32( retentionMaxBytes );
    sendQueueMaxBytes = config.GetAttributeValueOrChildValueByName( "sendQueueMaxBytes" ).AsUInt32( sendQueueMaxBytes );
    CORE::CString overflow = config.GetAttributeValueOrChildValueByName( "suspendedOverflow" ).AsString();
    if ( "DROP_OLDEST" == overflow )
        suspendedOverflow = RPC_SUSPENDEDOVERFLOW_DROP_OLDEST;
    else if ( "REFUSE_NEW" == overflow )
        suspendedOverflow = RPC_SUSPENDEDOVERFLOW_REFUSE_NEW;
    requireSecureTransport = config.GetAttributeValueOrChildValueByName( "requireSecureTransport" ).AsBool( requireSecureTransport );
    enableReflection = config.GetAttributeValueOrChildValueByName( "enableReflection" ).AsBool( enableReflection );
    preferPeerContracts = config.GetAttributeValueOrChildValueByName( "preferPeerContracts" ).AsBool( preferPeerContracts );
    clockSkewAllowanceMs = config.GetAttributeValueOrChildValueByName( "clockSkewAllowanceMs" ).AsUInt32( clockSkewAllowanceMs );
    closeTimeoutMs = config.GetAttributeValueOrChildValueByName( "closeTimeoutMs" ).AsUInt32( closeTimeoutMs );
    durableResultRetentionMs = config.GetAttributeValueOrChildValueByName( "durableResultRetentionMs" ).AsUInt32( durableResultRetentionMs );
    maxDurableRequestsPerSubject = config.GetAttributeValueOrChildValueByName( "maxDurableRequestsPerSubject" ).AsUInt32( maxDurableRequestsPerSubject );
    autoReconnect = config.GetAttributeValueOrChildValueByName( "autoReconnect" ).AsBool( autoReconnect );
    reconnectInitialDelayMs = config.GetAttributeValueOrChildValueByName( "reconnectInitialDelayMs" ).AsUInt32( reconnectInitialDelayMs );
    reconnectMaxDelayMs = config.GetAttributeValueOrChildValueByName( "reconnectMaxDelayMs" ).AsUInt32( reconnectMaxDelayMs );
    threadPoolName = config.GetAttributeValueOrChildValueByName( "threadPoolName", threadPoolName ).AsString( threadPoolName );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcEndpointSettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcEndpointSettings";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
