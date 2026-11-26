/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#include "gucefCOMCORE_CMacNetworkInterface.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS )

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <map>
#include <vector>

#ifndef GUCEF_MT_COBJECTSCOPELOCK_H
#include "gucefMT_CObjectScopeLock.h"
#define GUCEF_MT_COBJECTSCOPELOCK_H
#endif /* GUCEF_MT_COBJECTSCOPELOCK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The routing socket interface pads every socket address in a message to a multiple of 4 bytes */
#define GUCEF_COMCORE_MAC_SA_SIZE( sa )  ( ( (sa)->sa_len > 0 ) ? ( 1 + ( ( ( (sa)->sa_len ) - 1 ) | ( sizeof( UInt32 ) - 1 ) ) ) : sizeof( UInt32 ) )

/*-------------------------------------------------------------------------*/

/**
 *  Reads the routing socket interface data of the system with sysctl().
 *  The size of the data can change between asking for the size and getting it: that is retried.
 */
static bool
ReadRoutingSysctl( int mib[ 6 ], std::vector< char >& buffer )
{GUCEF_TRACE;

    for ( int attempt=0; attempt<8; ++attempt )
    {
        size_t needed = 0;
        if ( 0 != ::sysctl( mib, 6, GUCEF_NULL, &needed, GUCEF_NULL, 0 ) )
            return false;
        if ( 0 == needed )
        {
            buffer.clear();
            return true;
        }

        buffer.resize( needed + needed / 8 + 512 );
        size_t size = buffer.size();
        if ( 0 == ::sysctl( mib, 6, &buffer[ 0 ], &size, GUCEF_NULL, 0 ) )
        {
            buffer.resize( size );
            return true;
        }
        if ( ENOMEM != errno )
            return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  The IPv4 default gateways of the machine, by the index of the interface that they are reached through.
 *  This is the routing table: the default route is the one with the destination 0.0.0.0 and a gateway.
 */
static bool
GetDefaultGatewaysByInterfaceIndex( std::map< UInt32, CIPv4Address >& gateways )
{GUCEF_TRACE;

    int mib[ 6 ] = { CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_FLAGS, RTF_GATEWAY };
    std::vector< char > buffer;
    if ( !ReadRoutingSysctl( mib, buffer ) )
        return false;

    const char* pos = buffer.empty() ? GUCEF_NULL : &buffer[ 0 ];
    const char* end = pos + buffer.size();
    while ( GUCEF_NULL != pos && ( pos + sizeof( struct rt_msghdr ) ) <= end )
    {
        const struct rt_msghdr* rtm = (const struct rt_msghdr*) pos;
        if ( rtm->rtm_msglen < sizeof( struct rt_msghdr ) || ( pos + rtm->rtm_msglen ) > end )
            break;

        if ( 0 != ( rtm->rtm_flags & RTF_UP ) && 0 != ( rtm->rtm_flags & RTF_GATEWAY ) && 0 == ( rtm->rtm_flags & RTF_IFSCOPE ) )
        {
            // the socket addresses follow the header in the order of the bits in rtm_addrs
            const struct sockaddr* dst = GUCEF_NULL;
            const struct sockaddr* gateway = GUCEF_NULL;
            const struct sockaddr* netmask = GUCEF_NULL;
            const char* sa = pos + sizeof( struct rt_msghdr );
            const char* msgEnd = pos + rtm->rtm_msglen;
            for ( int bit=0; bit<RTAX_MAX && sa < msgEnd; ++bit )
            {
                if ( 0 == ( rtm->rtm_addrs & ( 1 << bit ) ) )
                    continue;
                const struct sockaddr* addr = (const struct sockaddr*) sa;
                if ( RTAX_DST == bit )
                    dst = addr;
                else
                if ( RTAX_GATEWAY == bit )
                    gateway = addr;
                else
                if ( RTAX_NETMASK == bit )
                    netmask = addr;
                sa += GUCEF_COMCORE_MAC_SA_SIZE( addr );
            }

            if ( GUCEF_NULL != dst && GUCEF_NULL != gateway && AF_INET == dst->sa_family && AF_INET == gateway->sa_family )
            {
                const struct sockaddr_in* dstIn = (const struct sockaddr_in*) dst;
                const struct sockaddr_in* gatewayIn = (const struct sockaddr_in*) gateway;

                // the default route has no destination, and a netmask of 0 or none at all
                bool maskIsEmpty = true;
                if ( GUCEF_NULL != netmask && netmask->sa_len > offsetof( struct sockaddr_in, sin_addr ) )
                    maskIsEmpty = 0 == ( (const struct sockaddr_in*) netmask )->sin_addr.s_addr;

                if ( 0 == dstIn->sin_addr.s_addr && maskIsEmpty )
                    gateways[ (UInt32) rtm->rtm_index ] = CIPv4Address( (UInt32) gatewayIn->sin_addr.s_addr, 0 );
            }
        }
        pos += rtm->rtm_msglen;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The 64 bit statistics of every interface that the system has, by the index of the interface */
static bool
GetInterfaceStatistics( std::map< UInt32, struct if_data64 >& statistics )
{GUCEF_TRACE;

    int mib[ 6 ] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0 };
    std::vector< char > buffer;
    if ( !ReadRoutingSysctl( mib, buffer ) )
        return false;

    const char* pos = buffer.empty() ? GUCEF_NULL : &buffer[ 0 ];
    const char* end = pos + buffer.size();
    while ( GUCEF_NULL != pos && ( pos + sizeof( struct if_msghdr ) ) <= end )
    {
        const struct if_msghdr* ifm = (const struct if_msghdr*) pos;
        if ( ifm->ifm_msglen < sizeof( struct if_msghdr ) || ( pos + ifm->ifm_msglen ) > end )
            break;

        if ( RTM_IFINFO2 == ifm->ifm_type && ifm->ifm_msglen >= sizeof( struct if_msghdr2 ) )
        {
            const struct if_msghdr2* ifm2 = (const struct if_msghdr2*) pos;
            statistics[ (UInt32) ifm2->ifm_index ] = ifm2->ifm_data;
        }
        pos += ifm->ifm_msglen;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static void
FillMetricsFromStatistics( const struct if_data64& stats, CNetworkInterfaceMetrics& metrics )
{GUCEF_TRACE;

    // the statistics of the system count all packets, the multicast ones are known separately
    // which gives the unicast and non unicast counts. Broadcast packets are not told apart from multicast ones.
    metrics.transmitLinkSpeedBitsPerSec = (UInt64) stats.ifi_baudrate;
    metrics.hasTransmitLinkSpeedBitsPerSec = true;
    metrics.receiveLinkSpeedBitsPerSec = (UInt64) stats.ifi_baudrate;
    metrics.hasReceiveLinkSpeedBitsPerSec = true;

    metrics.inboundOctets = (UInt64) stats.ifi_ibytes;
    metrics.hasInboundOctets = true;
    metrics.inboundNonUnicastPackets = (UInt64) stats.ifi_imcasts;
    metrics.hasInboundNonUnicastPackets = true;
    metrics.inboundUnicastPackets = stats.ifi_ipackets >= stats.ifi_imcasts ? (UInt64) ( stats.ifi_ipackets - stats.ifi_imcasts ) : 0;
    metrics.hasInboundUnicastPackets = true;
    metrics.inboundDiscardedPackets = (UInt64) stats.ifi_iqdrops;
    metrics.hasInboundDiscardedPackets = true;
    metrics.inboundErroredPackets = (UInt64) stats.ifi_ierrors;
    metrics.hasInboundErroredPackets = true;
    metrics.inboundUnknownProtocolPackets = (UInt64) stats.ifi_noproto;
    metrics.hasInboundUnknownProtocolPackets = true;

    metrics.outboundOctets = (UInt64) stats.ifi_obytes;
    metrics.hasOutboundOctets = true;
    metrics.outboundNonUnicastPackets = (UInt64) stats.ifi_omcasts;
    metrics.hasOutboundNonUnicastPackets = true;
    metrics.outboundUnicastPackets = stats.ifi_opackets >= stats.ifi_omcasts ? (UInt64) ( stats.ifi_opackets - stats.ifi_omcasts ) : 0;
    metrics.hasOutboundUnicastPackets = true;
    metrics.outboundErroredPackets = (UInt64) stats.ifi_oerrors;
    metrics.hasOutboundErroredPackets = true;
}

/*-------------------------------------------------------------------------*/

/** The IPv4 name servers that the system resolver uses. These are not kept per interface, they are the same for all. */
static void
GetSystemDnsServers( CINetworkInterface::THostAddressVector& dnsAddresses )
{GUCEF_TRACE;

    FILE* file = ::fopen( "/etc/resolv.conf", "r" );
    if ( GUCEF_NULL == file )
        return;

    char line[ 512 ];
    while ( GUCEF_NULL != ::fgets( line, sizeof( line ), file ) )
    {
        // the lines that matter look like "nameserver 192.168.1.1"
        const char keyword[] = "nameserver";
        const char* p = line;
        while ( ' ' == *p || '\t' == *p )
            ++p;
        if ( 0 != ::strncmp( p, keyword, sizeof( keyword ) - 1 ) )
            continue;
        p += sizeof( keyword ) - 1;
        if ( ' ' != *p && '\t' != *p )
            continue;
        while ( ' ' == *p || '\t' == *p )
            ++p;

        char address[ 64 ];
        size_t len = 0;
        while ( len < sizeof( address ) - 1 && '\0' != p[ len ] && ' ' != p[ len ] && '\t' != p[ len ] && '\r' != p[ len ] && '\n' != p[ len ] && '#' != p[ len ] )
        {
            address[ len ] = p[ len ];
            ++len;
        }
        address[ len ] = '\0';

        struct in_addr ipv4;
        if ( 1 == ::inet_pton( AF_INET, address, &ipv4 ) )
        {
            CIPv4Address dnsIp( (UInt32) ipv4.s_addr, CIPv4Address::HostByteOrderToNetworkByteOrder( (UInt16) 53 ) );
            dnsAddresses.push_back( CHostAddress( dnsIp, CORE::CString( address ) ) );
        }
    }
    ::fclose( file );
}

/*-------------------------------------------------------------------------*/

/** Runs a command and gives what it wrote to standard output. @return false when it could not be run or did not end well */
static bool
RunCommandForOutput( const char* command, CORE::CString& output )
{GUCEF_TRACE;

    output.Clear();
    FILE* pipe = ::popen( command, "r" );
    if ( GUCEF_NULL == pipe )
        return false;

    char buffer[ 512 ];
    size_t bytesRead = 0;
    while ( ( bytesRead = ::fread( buffer, 1, sizeof( buffer ) - 1, pipe ) ) > 0 )
    {
        buffer[ bytesRead ] = '\0';
        output += buffer;
    }
    const int status = ::pclose( pipe );
    return 0 == status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CMacNetworkInterface::FillExtraInfoFromConfig( void )
{GUCEF_TRACE;

    // The name servers are those of the system
    GetSystemDnsServers( m_dnsAddresses );

    // DHCP: the DHCP client of the system keeps the reply that it got for every interface that it configured, which
    // ipconfig gives. An interface with a fixed address has none and the command fails.
    CORE::CString packet;
    CORE::CString command = "/usr/sbin/ipconfig getpacket " + m_name + " 2>/dev/null";
    if ( RunCommandForOutput( command.C_String(), packet ) && !packet.IsNULLOrEmpty() )
    {
        m_dhcpUsedforIpv4 = true;

        CORE::CString::StringVector lines = packet.RemoveChar( '\r' ).ParseElements( '\n', false );
        CORE::CString::StringVector::iterator i = lines.begin();
        while ( i != lines.end() )
        {
            const CORE::CString line = (*i).Trim( true ).Trim( false );
            ++i;

            // "server_identifier (ip): 192.168.1.1"
            if ( 0 == line.HasSubstr( "server_identifier (ip):", 0, true ) )
            {
                CORE::CString value = line.SubstrToIndex( (UInt32) CORE::CString( "server_identifier (ip):" ).Length(), false ).Trim( true ).Trim( false );
                m_dhcpAddr.SetAddress( value );
            }
        }
    }

    // The default gateway(s) of the interface
    std::map< UInt32, CIPv4Address > gateways;
    if ( GetDefaultGatewaysByInterfaceIndex( gateways ) )
    {
        std::map< UInt32, CIPv4Address >::const_iterator g = gateways.find( m_nicIndex );
        if ( g != gateways.end() )
        {
            m_defGateway = (*g).second;
            m_gatewayList.push_back( (*g).second );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::EnumNetworkAdapters( TINetworkInterfacePtrVector& interfaces )
{GUCEF_TRACE;

    struct ifaddrs* ifaddr = GUCEF_NULL;
    if ( ::getifaddrs( &ifaddr ) == -1 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MacNetworkInterface:EnumNetworkAdapters: getifaddrs failed" );
        return false;
    }

    // The same interface is in the list once for every address it has (IPv4, IPv6, link level):
    // the interface is what we are after so those are grouped by the name of the interface
    typedef std::map< CString, CMacNetworkInterfacePtr > TNicMap;
    TNicMap nicMap;
    std::vector< CMacNetworkInterfacePtr > nicsInOrder;

    for ( struct ifaddrs* ifa = ifaddr; ifa != GUCEF_NULL; ifa = ifa->ifa_next )
    {
        if ( GUCEF_NULL == ifa->ifa_addr || GUCEF_NULL == ifa->ifa_name )
            continue;

        const int family = ifa->ifa_addr->sa_family;
        if ( family != AF_INET && family != AF_INET6 )
            continue;

        if ( 0 != ( ifa->ifa_flags & IFF_LOOPBACK ) )
            continue; // Skip loopback interfaces

        const CString nicName = ifa->ifa_name;
        CMacNetworkInterfacePtr nic;
        TNicMap::iterator n = nicMap.find( nicName );
        if ( n == nicMap.end() )
        {
            nic = CMacNetworkInterfacePtr( GUCEF_NEW CMacNetworkInterface() );
            nic->m_name = nicName;
            nic->m_desc = nicName;
            nic->m_nicIndex = (UInt32) ::if_nametoindex( ifa->ifa_name );
            nicMap[ nicName ] = nic;
            nicsInOrder.push_back( nic );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MacNetworkInterface:EnumNetworkAdapters: found interface \"" + nic->m_name +
                "\" with index " + CORE::ToString( nic->m_nicIndex ) );
        }
        else
        {
            nic = (*n).second;
        }

        if ( AF_INET == family )
        {
            const struct sockaddr_in* addr = (const struct sockaddr_in*) ifa->ifa_addr;
            CIPv4Info ipInfo;
            ipInfo.ip = CIPv4Address( (UInt32) addr->sin_addr.s_addr, 0 );
            if ( GUCEF_NULL != ifa->ifa_netmask )
                ipInfo.subnet = CIPv4Address( (UInt32) ( (const struct sockaddr_in*) ifa->ifa_netmask )->sin_addr.s_addr, 0 );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MacNetworkInterface:EnumNetworkAdapters: interface \"" + nic->m_name + "\" has IPv4 " +
                ipInfo.ip.AddressAsString() + " with subnet mask " + ipInfo.subnet.AddressAsString() );

            // the first IPv4 address is the one that is in use
            if ( 0 == nic->m_curIpAddr.ip.GetAddress() )
                nic->m_curIpAddr = ipInfo;
            nic->m_ipv4Addresses.insert( ipInfo );
        }
    }

    // the link level addresses (MAC) are separate entries in the list: those are for interfaces we have already
    for ( struct ifaddrs* ifa = ifaddr; ifa != GUCEF_NULL; ifa = ifa->ifa_next )
    {
        if ( GUCEF_NULL == ifa->ifa_addr || GUCEF_NULL == ifa->ifa_name || AF_LINK != ifa->ifa_addr->sa_family )
            continue;

        TNicMap::iterator n = nicMap.find( CString( ifa->ifa_name ) );
        if ( n == nicMap.end() )
            continue;

        const struct sockaddr_dl* link = (const struct sockaddr_dl*) ifa->ifa_addr;
        if ( 6 == link->sdl_alen )
        {
            const unsigned char* mac = (const unsigned char*) LLADDR( link );
            char macAddress[ 24 ];
            ::snprintf( macAddress, sizeof( macAddress ), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
            (*n).second->m_macAddrs.insert( CString( macAddress ) );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "MacNetworkInterface:EnumNetworkAdapters: interface \"" + (*n).second->m_name + "\" MAC: " + CString( macAddress ) );
        }
    }

    ::freeifaddrs( ifaddr );
    ifaddr = GUCEF_NULL;

    for ( size_t i=0; i<nicsInOrder.size(); ++i )
    {
        nicsInOrder[ i ]->FillExtraInfoFromConfig();
        interfaces.push_back( nicsInOrder[ i ].StaticCast< CINetworkInterface >() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::GetDefaultGateway( CIPv4Address& gateway )
{GUCEF_TRACE;

    std::map< UInt32, CIPv4Address > gateways;
    if ( !GetDefaultGatewaysByInterfaceIndex( gateways ) || gateways.empty() )
        return false;

    // the map is ordered by interface index: the first one is the one the system set up first
    gateway = (*gateways.begin()).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::GetGlobalMetrics( CNetworkInterfaceMetrics& metrics )
{GUCEF_TRACE;

    std::map< UInt32, struct if_data64 > statistics;
    if ( !GetInterfaceStatistics( statistics ) )
        return false;

    // sum the interfaces that carry traffic to and from the machine
    struct if_data64 total;
    ::memset( &total, 0, sizeof( total ) );
    bool foundAny = false;
    std::map< UInt32, struct if_data64 >::const_iterator i = statistics.begin();
    while ( i != statistics.end() )
    {
        const struct if_data64& stats = (*i).second;
        ++i;

        if ( IFT_LOOP == stats.ifi_type )
            continue;

        total.ifi_ibytes += stats.ifi_ibytes;
        total.ifi_ipackets += stats.ifi_ipackets;
        total.ifi_imcasts += stats.ifi_imcasts;
        total.ifi_iqdrops += stats.ifi_iqdrops;
        total.ifi_ierrors += stats.ifi_ierrors;
        total.ifi_noproto += stats.ifi_noproto;
        total.ifi_obytes += stats.ifi_obytes;
        total.ifi_opackets += stats.ifi_opackets;
        total.ifi_omcasts += stats.ifi_omcasts;
        total.ifi_oerrors += stats.ifi_oerrors;
        foundAny = true;
    }
    if ( !foundAny )
        return false;

    FillMetricsFromStatistics( total, metrics );

    // a link speed is a property of an interface: it has no meaning when added up
    metrics.hasTransmitLinkSpeedBitsPerSec = false;
    metrics.transmitLinkSpeedBitsPerSec = 0;
    metrics.hasReceiveLinkSpeedBitsPerSec = false;
    metrics.receiveLinkSpeedBitsPerSec = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

CMacNetworkInterface::CMacNetworkInterface( void )
    : CINetworkInterface()
    , m_name()
    , m_desc()
    , m_macAddrs()
    , m_priWins()
    , m_secWins()
    , m_defGateway()
    , m_dhcpAddr()
    , m_curIpAddr()
    , m_nicIndex( 0 )
    , m_adapterType( 0 )
    , m_dhcpUsedforIpv4( false )
    , m_dhcpUsedforIpv6( false )
    , m_winsUsed( false )
    , m_dnsAddresses()
    , m_ipv4Addresses()
    , m_gatewayList()
    , m_leaseObtained( 0 )
    , m_leaseExpires( 0 )
    , m_dataLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMacNetworkInterface::~CMacNetworkInterface()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMacNetworkInterface::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CMacNetworkInterface";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMacNetworkInterface::GetCommunicationInterfaceType( void ) const
{GUCEF_TRACE;

    return "network";
}

/*-------------------------------------------------------------------------*/

CString
CMacNetworkInterface::GetAdapterName( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_name;
}

/*-------------------------------------------------------------------------*/

CString
CMacNetworkInterface::GetAdapterDescription( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_desc;
}

/*-------------------------------------------------------------------------*/

UInt32
CMacNetworkInterface::GetNrOfDnsAddresses( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return (UInt32) m_dnsAddresses.size();
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::GetDnsAddresses( THostAddressVector& dnsAddresses )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    THostAddressVector::const_iterator i = m_dnsAddresses.begin();
    while ( i != m_dnsAddresses.end() )
    {
        dnsAddresses.push_back( (*i) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CMacNetworkInterface::GetNrOfIPAddresses( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return (UInt32) m_ipv4Addresses.size();
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::GetIPInfo( TIPv4InfoVector& ipInfo, bool includeUninitialized ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CIPv4Info::TIPv4InfoSet::const_iterator i = m_ipv4Addresses.begin();
    while ( i != m_ipv4Addresses.end() )
    {
        if ( includeUninitialized || 0 != (*i).ip.GetAddress() )
            ipInfo.push_back( (*i) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::IsDhcpUsedForIPv4( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_dhcpUsedforIpv4;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::IsDhcpUsedForIPv6( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_dhcpUsedforIpv6;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CMacNetworkInterface::GetDhcpLeaseObtainedTime( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CORE::CDateTime result;
    result.FromUnixEpochBasedTicksInMillisecs( (UInt64) m_leaseObtained * 1000 );
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CDateTime
CMacNetworkInterface::GetDhcpLeaseExpirationTime( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    CORE::CDateTime result;
    result.FromUnixEpochBasedTicksInMillisecs( (UInt64) m_leaseExpires * 1000 );
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::ReleaseAddress( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::RenewAddress( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::IsWinsUsed( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_winsUsed;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CMacNetworkInterface::GetPrimaryWinsServer( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_priWins;
}

/*-------------------------------------------------------------------------*/

CIPv4Address
CMacNetworkInterface::GetSecondaryWinsServer( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_secWins;
}

/*-------------------------------------------------------------------------*/

UInt32
CMacNetworkInterface::GetOsAdapterIndex( void ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_nicIndex;
}

/*-------------------------------------------------------------------------*/

bool
CMacNetworkInterface::GetMetrics( CNetworkInterfaceMetrics& metrics ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    std::map< UInt32, struct if_data64 > statistics;
    if ( !GetInterfaceStatistics( statistics ) )
        return false;

    std::map< UInt32, struct if_data64 >::const_iterator s = statistics.find( m_nicIndex );
    if ( s == statistics.end() )
        return false;

    FillMetricsFromStatistics( (*s).second, metrics );
    return true;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CMacNetworkInterface::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CMacNetworkInterface::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_dataLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CMacNetworkInterface::Unlock( void ) const
{GUCEF_TRACE;

    return m_dataLock.Unlock();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM == GUCEF_PLATFORM_MACOS ? */
