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

#include "gucefCOM_CUdpMulticastDiscoveryClient.h"

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

const CORE::CString CUdpMulticastDiscoveryClient::TypeName = "multicast";
const CORE::CString CUdpMulticastDiscoveryClient::DefaultGroup = "239.255.47.80";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUdpMulticastDiscoveryClient::CUdpMulticastDiscoveryClient( void )
    : CUdpDiscoveryClientBase( TypeName, CORE::PulseGeneratorPtr() )
    , m_group( DefaultGroup, DefaultPort )
    , m_multicastTtl( 1 )
    , m_allowLoopback( true )
    , m_joinedInterfaceCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUdpMulticastDiscoveryClient::CUdpMulticastDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CUdpDiscoveryClientBase( TypeName, pulseGenerator )
    , m_group( DefaultGroup, DefaultPort )
    , m_multicastTtl( 1 )
    , m_allowLoopback( true )
    , m_joinedInterfaceCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CUdpMulticastDiscoveryClient::~CUdpMulticastDiscoveryClient()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CUdpMulticastDiscoveryClient::SetGroup( const CORE::CString& group )
{GUCEF_TRACE;

    COMCORE::CIPv4Address address;
    if ( !address.SetAddress( group ) || !address.IsMulticast() )
        return false;
    m_group = address;
    return true;
}

/*-------------------------------------------------------------------------*/

const COMCORE::CIPv4Address&
CUdpMulticastDiscoveryClient::GetGroup( void ) const
{GUCEF_TRACE;

    return m_group;
}

/*-------------------------------------------------------------------------*/

void
CUdpMulticastDiscoveryClient::SetMulticastTtl( Int32 ttl )
{GUCEF_TRACE;

    m_multicastTtl = ttl;
}

/*-------------------------------------------------------------------------*/

Int32
CUdpMulticastDiscoveryClient::GetMulticastTtl( void ) const
{GUCEF_TRACE;

    return m_multicastTtl;
}

/*-------------------------------------------------------------------------*/

void
CUdpMulticastDiscoveryClient::SetAllowLoopback( bool allowLoopback )
{GUCEF_TRACE;

    m_allowLoopback = allowLoopback;
}

/*-------------------------------------------------------------------------*/

UInt32
CUdpMulticastDiscoveryClient::GetJoinedInterfaceCount( void ) const
{GUCEF_TRACE;

    return m_joinedInterfaceCount;
}

/*-------------------------------------------------------------------------*/

bool
CUdpMulticastDiscoveryClient::OpenSocket( COMCORE::CUDPSocket& socket, UInt16 port )
{GUCEF_TRACE;

    socket.SetMulticastTTL( m_multicastTtl );
    socket.SetAllowMulticastLoopback( m_allowLoopback );
    if ( !socket.Open( port ) )
        return false;

    m_joinedInterfaceCount = JoinOnAllInterfaces( socket, m_group );
    if ( 0 == m_joinedInterfaceCount )
    {
        ReportDiscoveryError( "Unable to join multicast group " + m_group.AddressAsString() + " on any network interface" );
        socket.Close();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CUdpMulticastDiscoveryClient::GetDestinations( TIPv4AddressVector& destinations ) const
{GUCEF_TRACE;

    COMCORE::CIPv4Address destination( m_group );
    destination.SetPortInHostByteOrder( GetPort() );
    destinations.push_back( destination );
}

/*-------------------------------------------------------------------------*/

bool
CUdpMulticastDiscoveryClient::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !CUdpDiscoveryClientBase::LoadConfig( config ) )
        return false;

    CORE::CString group = config.GetAttributeValueOrChildValueByName( "group" ).AsString( CORE::CString::Empty, true );
    if ( !group.IsNULLOrEmpty() && !SetGroup( group ) )
        return false;

    m_multicastTtl = config.GetAttributeValueOrChildValueByName( "multicastTtl" ).AsInt32( m_multicastTtl, true );
    m_allowLoopback = config.GetAttributeValueOrChildValueByName( "allowLoopback" ).AsBool( m_allowLoopback, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpMulticastDiscoveryClient::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !CUdpDiscoveryClientBase::SaveConfig( config ) )
        return false;

    config.SetAttribute( "group", m_group.AddressAsString() );
    config.SetAttribute( "multicastTtl", m_multicastTtl );
    config.SetAttribute( "allowLoopback", m_allowLoopback );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CUdpMulticastDiscoveryClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CUdpMulticastDiscoveryClient";
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
