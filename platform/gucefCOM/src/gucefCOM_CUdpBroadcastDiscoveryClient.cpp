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

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefCOM_CUdpBroadcastDiscoveryClient.h"

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

const CORE::CString CUdpBroadcastDiscoveryClient::TypeName = "broadcast";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUdpBroadcastDiscoveryClient::CUdpBroadcastDiscoveryClient( void )
    : CUdpDiscoveryClientBase( TypeName, CORE::PulseGeneratorPtr() )
    , m_useLimitedBroadcast( true )
    , m_useDirectedBroadcasts( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUdpBroadcastDiscoveryClient::CUdpBroadcastDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CUdpDiscoveryClientBase( TypeName, pulseGenerator )
    , m_useLimitedBroadcast( true )
    , m_useDirectedBroadcasts( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUdpBroadcastDiscoveryClient::~CUdpBroadcastDiscoveryClient()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CUdpBroadcastDiscoveryClient::SetUseLimitedBroadcast( bool useLimitedBroadcast )
{GUCEF_TRACE;

    m_useLimitedBroadcast = useLimitedBroadcast;
}

/*-------------------------------------------------------------------------*/

void
CUdpBroadcastDiscoveryClient::SetUseDirectedBroadcasts( bool useDirectedBroadcasts )
{GUCEF_TRACE;

    m_useDirectedBroadcasts = useDirectedBroadcasts;
}

/*-------------------------------------------------------------------------*/

bool
CUdpBroadcastDiscoveryClient::OpenSocket( COMCORE::CUDPSocket& socket, UInt16 port )
{GUCEF_TRACE;

    socket.SetAllowBroadcast( true );
    return socket.Open( port );
}

/*-------------------------------------------------------------------------*/

void
CUdpBroadcastDiscoveryClient::GetDestinations( TIPv4AddressVector& destinations ) const
{GUCEF_TRACE;

    UInt16 portInNetworkByteOrder = COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( GetPort() );
    if ( m_useLimitedBroadcast )
        destinations.push_back( COMCORE::CIPv4Address( 0xFFFFFFFF, portInNetworkByteOrder ) );

    if ( !m_useDirectedBroadcasts )
        return;

    COMCORE::CINetworkInterface::TIPv4InfoVector ipInfo;
    if ( !COMCORE::CComCoreGlobal::Instance()->GetCom().GetAllNetworkInterfaceIPInfo( ipInfo ) )
        return;

    for ( size_t i=0; i<ipInfo.size(); ++i )
    {
        UInt32 address = ipInfo[ i ].ip.GetAddress();
        UInt32 mask = ipInfo[ i ].subnet.GetAddress();
        UInt32 addressInHostByteOrder = ipInfo[ i ].ip.GetAddressInHostByteOrder();

        // No broadcast on loopback or unconfigured interfaces, and a host route has no broadcast address
        if ( 0 == address || 0x7F == ( addressInHostByteOrder >> 24 ) || 0xFFFFFFFF == mask )
            continue;

        // Both are in network byte order, the bitwise operations do not care
        UInt32 broadcast = address | ~mask;
        COMCORE::CIPv4Address destination( broadcast, portInNetworkByteOrder );

        bool known = false;
        for ( size_t n=0; n<destinations.size(); ++n )
        {
            if ( destinations[ n ] == destination )
            {
                known = true;
                break;
            }
        }
        if ( !known )
            destinations.push_back( destination );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUdpBroadcastDiscoveryClient::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !CUdpDiscoveryClientBase::LoadConfig( config ) )
        return false;

    m_useLimitedBroadcast = config.GetAttributeValueOrChildValueByName( "useLimitedBroadcast" ).AsBool( m_useLimitedBroadcast, true );
    m_useDirectedBroadcasts = config.GetAttributeValueOrChildValueByName( "useDirectedBroadcasts" ).AsBool( m_useDirectedBroadcasts, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpBroadcastDiscoveryClient::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !CUdpDiscoveryClientBase::SaveConfig( config ) )
        return false;

    config.SetAttribute( "useLimitedBroadcast", m_useLimitedBroadcast );
    config.SetAttribute( "useDirectedBroadcasts", m_useDirectedBroadcasts );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUdpBroadcastDiscoveryClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CUdpBroadcastDiscoveryClient";
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
