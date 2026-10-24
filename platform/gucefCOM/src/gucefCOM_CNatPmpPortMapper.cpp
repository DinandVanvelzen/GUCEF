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

#include <stdlib.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_NETWORKBYTEORDER_H
#include "gucefCOM_NetworkByteOrder.h"
#define GUCEF_COM_NETWORKBYTEORDER_H
#endif /* GUCEF_COM_NETWORKBYTEORDER_H ? */

#include "gucefCOM_CNatPmpPortMapper.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <winsock2.h>
  #include <windows.h>
  #include <iphlpapi.h>
#endif

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

const CORE::CString CNatPmpPortMapper::MapperName = "natpmp";
const UInt16 CNatPmpPortMapper::GatewayPort = 5351;

static const UInt32 TimerIntervalInMs = 25;
static const UInt32 PcpAttemptsBeforeFallback = 2;

static const UInt8 PcpVersion = 2;
static const UInt8 PcpOpcodeMap = 1;
static const UInt8 PcpResponseBit = 0x80;
static const UInt8 PcpResultUnsupportedVersion = 1;
static const UInt32 PcpMapRequestSize = 60;
static const UInt32 PcpMapResponseSize = 60;

static const UInt8 NatPmpVersion = 0;
static const UInt8 NatPmpOpcodeExternalAddress = 0;
static const UInt8 NatPmpOpcodeMapUdp = 1;
static const UInt8 NatPmpOpcodeMapTcp = 2;
static const UInt8 NatPmpResponseBit = 0x80;
static const UInt16 NatPmpResultUnsupportedVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char*
PcpResultToString( UInt8 result )
{GUCEF_TRACE;

    switch ( result )
    {
        case 0: return "success";
        case 1: return "unsupported version";
        case 2: return "not authorized";
        case 3: return "malformed request";
        case 4: return "unsupported opcode";
        case 5: return "unsupported option";
        case 6: return "malformed option";
        case 7: return "network failure";
        case 8: return "no resources";
        case 9: return "unsupported protocol";
        case 10: return "user exceeded quota";
        case 11: return "cannot provide external";
        case 12: return "address mismatch";
        case 13: return "excessive remote peers";
        default: return "unknown result";
    }
}

/*-------------------------------------------------------------------------*/

const char*
NatPmpResultToString( UInt16 result )
{GUCEF_TRACE;

    switch ( result )
    {
        case 0: return "success";
        case 1: return "unsupported version";
        case 2: return "not authorized or refused";
        case 3: return "network failure";
        case 4: return "out of resources";
        case 5: return "unsupported opcode";
        default: return "unknown result";
    }
}

/*-------------------------------------------------------------------------*/

UInt32
ReadUInt32At( const UInt8* data )
{GUCEF_TRACE;

    return ( (UInt32) data[ 0 ] << 24 ) | ( (UInt32) data[ 1 ] << 16 ) | ( (UInt32) data[ 2 ] << 8 ) | (UInt32) data[ 3 ];
}

/*-------------------------------------------------------------------------*/

UInt16
ReadUInt16At( const UInt8* data )
{GUCEF_TRACE;

    return (UInt16)( ( data[ 0 ] << 8 ) | data[ 1 ] );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CNatPmpPortMapper::CNatPmpPortMapper( void )
    : COMCORE::CIPortMapper()
    , m_socket( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), false )
    , m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), TimerIntervalInMs )
    , m_gateway()
    , m_gatewayConfigured( false )
    , m_usePcp( true )
    , m_protocolMode( PROTOCOLMODE_UNKNOWN )
    , m_initialIntervalInMs( 250 )
    , m_maxAttempts( 4 )
    , m_externalAddress()
    , m_externalAddressKnown( false )
    , m_requests()
    , m_mappings()
    , m_nonceCounter( 0 )
{GUCEF_TRACE;

    m_socket.SetNrOfReceiveBuffers( 4, 128 );
    SubscribeTo( &m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CNatPmpPortMapper::CNatPmpPortMapper( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CIPortMapper()
    , m_socket( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator, false )
    , m_timer( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator, TimerIntervalInMs )
    , m_gateway()
    , m_gatewayConfigured( false )
    , m_usePcp( true )
    , m_protocolMode( PROTOCOLMODE_UNKNOWN )
    , m_initialIntervalInMs( 250 )
    , m_maxAttempts( 4 )
    , m_externalAddress()
    , m_externalAddressKnown( false )
    , m_requests()
    , m_mappings()
    , m_nonceCounter( 0 )
{GUCEF_TRACE;

    m_socket.SetNrOfReceiveBuffers( 4, 128 );
    SubscribeTo( &m_socket );
    SubscribeTo( &m_timer );
}

/*-------------------------------------------------------------------------*/

CNatPmpPortMapper::~CNatPmpPortMapper()
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    UnsubscribeAllFromObserver();

    for ( size_t i=0; i<m_requests.size(); ++i )
        m_requests[ i ].source.Cancel();
    m_requests.clear();
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

UInt64
CNatPmpPortMapper::GetTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::GetDefaultGateway( COMCORE::CIPv4Address& gateway )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

    // Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT, the addresses as hex of the
    // 32 bit value as it is in memory, which is network byte order
    CORE::CFileAccess routes( "/proc/net/route", "rb" );
    if ( !routes.IsValid() )
        return false;

    CORE::CString line = CORE::CString::ReadLine( &routes );     // the header
    while ( !routes.Eof() )
    {
        line = CORE::CString::ReadLine( &routes );
        CORE::CString::StringVector fields = line.ReplaceChar( '\t', ' ' ).ParseElements( ' ', false );
        if ( fields.size() < 4 )
            continue;
        UInt32 destination = (UInt32) strtoul( fields[ 1 ].C_String(), GUCEF_NULL, 16 );
        UInt32 gatewayAddress = (UInt32) strtoul( fields[ 2 ].C_String(), GUCEF_NULL, 16 );
        UInt32 flags = (UInt32) strtoul( fields[ 3 ].C_String(), GUCEF_NULL, 16 );
        const UInt32 routeIsUp = 0x1;
        const UInt32 routeHasGateway = 0x2;
        if ( 0 == destination && 0 != gatewayAddress && ( flags & routeIsUp ) && ( flags & routeHasGateway ) )
        {
            gateway = COMCORE::CIPv4Address( gatewayAddress, COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( GatewayPort ) );
            return true;
        }
    }
    return false;

    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

    // Loaded when needed so the module does not have to link with iphlpapi
    typedef DWORD ( WINAPI* TGetBestRoutePtr )( DWORD, DWORD, PMIB_IPFORWARDROW );
    HMODULE iphlpapi = ::LoadLibraryA( "iphlpapi.dll" );
    if ( GUCEF_NULL == iphlpapi )
        return false;
    TGetBestRoutePtr getBestRoute = (TGetBestRoutePtr) ::GetProcAddress( iphlpapi, "GetBestRoute" );
    bool found = false;
    if ( GUCEF_NULL != getBestRoute )
    {
        MIB_IPFORWARDROW row;
        memset( &row, 0, sizeof( row ) );
        if ( NO_ERROR == getBestRoute( 0, 0, &row ) && 0 != row.dwForwardNextHop )
        {
            gateway = COMCORE::CIPv4Address( (UInt32) row.dwForwardNextHop, COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( GatewayPort ) );
            found = true;
        }
    }
    ::FreeLibrary( iphlpapi );
    return found;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CNatPmpPortMapper::SetGateway( const COMCORE::CIPv4Address& gateway )
{GUCEF_TRACE;

    m_gateway = gateway;
    if ( 0 == m_gateway.GetPort() )
        m_gateway.SetPortInHostByteOrder( GatewayPort );
    m_gatewayConfigured = true;
    m_protocolMode = PROTOCOLMODE_UNKNOWN;
    m_externalAddressKnown = false;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::GetGateway( COMCORE::CIPv4Address& gateway ) const
{GUCEF_TRACE;

    if ( m_gatewayConfigured )
    {
        gateway = m_gateway;
        return true;
    }
    return GetDefaultGateway( gateway );
}

/*-------------------------------------------------------------------------*/

void
CNatPmpPortMapper::SetUsePcp( bool usePcp )
{GUCEF_TRACE;

    m_usePcp = usePcp;
}

/*-------------------------------------------------------------------------*/

void
CNatPmpPortMapper::SetRetransmission( UInt32 initialIntervalInMs, UInt32 maxAttempts )
{GUCEF_TRACE;

    m_initialIntervalInMs = initialIntervalInMs > 0 ? initialIntervalInMs : 1;
    m_maxAttempts = maxAttempts > 0 ? maxAttempts : 1;
}

/*-------------------------------------------------------------------------*/

UInt32
CNatPmpPortMapper::GetMappingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_mappings.size();
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::IsAvailable( void ) const
{GUCEF_TRACE;

    COMCORE::CIPv4Address gateway;
    return GetGateway( gateway );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CNatPmpPortMapper::GetMapperName( void ) const
{GUCEF_TRACE;

    return MapperName;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::EnsureReady( void )
{GUCEF_TRACE;

    if ( !m_gatewayConfigured )
    {
        if ( !GetDefaultGateway( m_gateway ) )
        {
            TPortMapperErrorEventData eData( "There is no default gateway to ask for port mappings" );
            NotifyObservers( PortMapperErrorEvent, &eData );
            return false;
        }
    }
    if ( !m_socket.IsActive() && !m_socket.Open( (UInt16) 0 ) )
    {
        TPortMapperErrorEventData eData( "Unable to open the socket to talk to the gateway" );
        NotifyObservers( PortMapperErrorEvent, &eData );
        return false;
    }
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNatPmpPortMapper::FillPcpClientAddress( UInt8* address ) const
{GUCEF_TRACE;

    // An IPv4 address goes into PCP as an IPv4-mapped IPv6 address: ::ffff:a.b.c.d
    memset( address, 0, 16 );
    address[ 10 ] = 0xFF;
    address[ 11 ] = 0xFF;

    UInt32 clientAddress = COMCORE::CIPv4Address::Loopback.GetAddress();
    UInt32 gatewayHostOrder = m_gateway.GetAddressInHostByteOrder();
    if ( 0x7F != ( gatewayHostOrder >> 24 ) )
    {
        // The address of the interface that is on the network of the gateway
        COMCORE::CINetworkInterface::TIPv4InfoVector ipInfo;
        if ( COMCORE::CComCoreGlobal::Instance()->GetCom().GetAllNetworkInterfaceIPInfo( ipInfo ) )
        {
            for ( size_t i=0; i<ipInfo.size(); ++i )
            {
                UInt32 ip = ipInfo[ i ].ip.GetAddress();
                UInt32 mask = ipInfo[ i ].subnet.GetAddress();
                if ( 0 != ip && ( ip & mask ) == ( m_gateway.GetAddress() & mask ) )
                {
                    clientAddress = ip;
                    break;
                }
            }
        }
    }
    memcpy( address + 12, &clientAddress, 4 );
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::SendRequest( TRequest& request )
{GUCEF_TRACE;

    UInt8 packet[ PcpMapRequestSize ];
    UInt32 size = 0;
    memset( packet, 0, sizeof( packet ) );

    if ( request.usesPcp )
    {
        // PCP has no request for the external address alone, a map request for this is only used in NAT-PMP
        packet[ 0 ] = PcpVersion;
        packet[ 1 ] = PcpOpcodeMap;
        UInt32 lifetime = REQUESTKIND_UNMAP == request.kind ? 0 : request.leaseInSecs;
        packet[ 4 ] = (UInt8)( lifetime >> 24 );
        packet[ 5 ] = (UInt8)( lifetime >> 16 );
        packet[ 6 ] = (UInt8)( lifetime >> 8 );
        packet[ 7 ] = (UInt8)( lifetime );
        FillPcpClientAddress( packet + 8 );
        memcpy( packet + 24, request.nonce, 12 );
        packet[ 36 ] = COMCORE::CPortMappingResult::PROTOCOL_TCP == request.protocol ? 6 : 17;
        packet[ 40 ] = (UInt8)( request.internalPort >> 8 );
        packet[ 41 ] = (UInt8)( request.internalPort );
        packet[ 42 ] = (UInt8)( request.externalPort >> 8 );
        packet[ 43 ] = (UInt8)( request.externalPort );
        packet[ 54 ] = 0xFF;    // no preference for the external address: ::ffff:0.0.0.0
        packet[ 55 ] = 0xFF;
        size = PcpMapRequestSize;
    }
    else
    if ( REQUESTKIND_EXTERNAL_ADDRESS == request.kind )
    {
        packet[ 0 ] = NatPmpVersion;
        packet[ 1 ] = NatPmpOpcodeExternalAddress;
        size = 2;
    }
    else
    {
        packet[ 0 ] = NatPmpVersion;
        packet[ 1 ] = COMCORE::CPortMappingResult::PROTOCOL_TCP == request.protocol ? NatPmpOpcodeMapTcp : NatPmpOpcodeMapUdp;
        packet[ 4 ] = (UInt8)( request.internalPort >> 8 );
        packet[ 5 ] = (UInt8)( request.internalPort );
        UInt16 externalPort = REQUESTKIND_UNMAP == request.kind ? 0 : request.externalPort;
        packet[ 6 ] = (UInt8)( externalPort >> 8 );
        packet[ 7 ] = (UInt8)( externalPort );
        UInt32 lifetime = REQUESTKIND_UNMAP == request.kind ? 0 : request.leaseInSecs;
        packet[ 8 ] = (UInt8)( lifetime >> 24 );
        packet[ 9 ] = (UInt8)( lifetime >> 16 );
        packet[ 10 ] = (UInt8)( lifetime >> 8 );
        packet[ 11 ] = (UInt8)( lifetime );
        size = 12;
    }

    ++request.attempts;
    request.nextSendInMs = GetTimeInMs() + request.intervalInMs;
    request.intervalInMs *= 2;

    return m_socket.SendPacketTo( m_gateway, packet, (UInt16) size ) == (Int32) size;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::QueueRequest( TRequest& request, CORE::CFutureResult* future )
{GUCEF_TRACE;

    request.attempts = 0;
    request.intervalInMs = m_initialIntervalInMs;
    request.awaitsExternalAddress = false;
    request.hasFuture = GUCEF_NULL != future;
    request.result.protocol = request.protocol;
    request.result.internalPort = request.internalPort;
    request.result.mapperName = MapperName;

    // A nonce that is unique for this mapper: a counter, the clock and some randomness
    ++m_nonceCounter;
    UInt64 now = MT::PrecisionTickCount();
    for ( int i=0; i<4; ++i )
        request.nonce[ i ] = (UInt8)( m_nonceCounter >> ( 24 - 8*i ) );
    for ( int i=0; i<4; ++i )
        request.nonce[ 4+i ] = (UInt8)( now >> ( 24 - 8*i ) );
    for ( int i=0; i<4; ++i )
        request.nonce[ 8+i ] = (UInt8)( rand() & 0xFF );

    if ( GUCEF_NULL != future )
        *future = request.source.GetFuture();

    m_requests.push_back( request );
    SendRequest( m_requests.back() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNatPmpPortMapper::EnsureExternalAddressRequest( void )
{GUCEF_TRACE;

    if ( m_externalAddressKnown )
        return;
    for ( size_t i=0; i<m_requests.size(); ++i )
    {
        if ( REQUESTKIND_EXTERNAL_ADDRESS == m_requests[ i ].kind && !m_requests[ i ].usesPcp )
            return;
    }

    TRequest addressRequest;
    addressRequest.kind = REQUESTKIND_EXTERNAL_ADDRESS;
    addressRequest.protocol = COMCORE::CPortMappingResult::PROTOCOL_UDP;
    addressRequest.internalPort = 0;
    addressRequest.externalPort = 0;
    addressRequest.leaseInSecs = 0;
    addressRequest.usesPcp = false;
    addressRequest.isRenewal = false;
    QueueRequest( addressRequest, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::RequestMapping( TProtocol protocol            ,
                                   UInt16 internalPort           ,
                                   UInt16 externalPort           ,
                                   UInt32 leaseInSecs            ,
                                   CORE::CFutureResult& future   )
{GUCEF_TRACE;

    if ( 0 == internalPort || !EnsureReady() )
        return false;

    TRequest request;
    request.kind = REQUESTKIND_MAP;
    request.protocol = protocol;
    request.internalPort = internalPort;
    request.externalPort = externalPort;
    request.leaseInSecs = leaseInSecs > 0 ? leaseInSecs : 7200;
    request.usesPcp = m_usePcp && PROTOCOLMODE_NATPMP != m_protocolMode;
    request.isRenewal = false;

    // NAT-PMP map answers do not have the external address, so it is asked for first when not known yet
    if ( !request.usesPcp )
        EnsureExternalAddressRequest();
    return QueueRequest( request, &future );
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::RemoveMapping( TProtocol protocol            ,
                                  UInt16 internalPort           ,
                                  CORE::CFutureResult& future   )
{GUCEF_TRACE;

    if ( 0 == internalPort || !EnsureReady() )
        return false;

    TRequest request;
    request.kind = REQUESTKIND_UNMAP;
    request.protocol = protocol;
    request.internalPort = internalPort;
    request.externalPort = 0;
    request.leaseInSecs = 0;
    request.usesPcp = m_usePcp && PROTOCOLMODE_NATPMP != m_protocolMode;
    request.isRenewal = false;

    // No more renewals for it, whatever the gateway says
    for ( size_t i=0; i<m_mappings.size(); ++i )
    {
        if ( m_mappings[ i ].protocol == protocol && m_mappings[ i ].internalPort == internalPort )
        {
            m_mappings.erase( m_mappings.begin() + i );
            break;
        }
    }
    return QueueRequest( request, &future );
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::GetExternalAddress( CORE::CFutureResult& future )
{GUCEF_TRACE;

    if ( !EnsureReady() )
        return false;

    TRequest request;
    request.kind = REQUESTKIND_EXTERNAL_ADDRESS;
    request.protocol = COMCORE::CPortMappingResult::PROTOCOL_UDP;
    request.internalPort = 0;
    request.externalPort = 0;
    request.leaseInSecs = 0;
    request.usesPcp = false;
    request.isRenewal = false;
    return QueueRequest( request, &future );
}

/*-------------------------------------------------------------------------*/

CNatPmpPortMapper::TMapping*
CNatPmpPortMapper::FindMapping( TProtocol protocol, UInt16 internalPort )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_mappings.size(); ++i )
    {
        if ( m_mappings[ i ].protocol == protocol && m_mappings[ i ].internalPort == internalPort )
            return &m_mappings[ i ];
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::CompleteAddressWaiters( bool addressKnown )
{GUCEF_TRACE;

    size_t i = 0;
    while ( i < m_requests.size() )
    {
        if ( m_requests[ i ].awaitsExternalAddress )
        {
            TRequest request = m_requests[ i ];
            m_requests.erase( m_requests.begin() + i );
            if ( addressKnown )
            {
                COMCORE::CIPv4Address address( m_externalAddress );
                address.SetPortInHostByteOrder( request.result.externalPort );
                request.result.externalAddress = address;
            }
            if ( request.hasFuture )
                request.source.Complete( request.result.Clone(), true );
            continue;
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::OnRequestSucceeded( size_t index )
{GUCEF_TRACE;

    TRequest request = m_requests[ index ];
    m_requests.erase( m_requests.begin() + index );
    UInt64 now = GetTimeInMs();

    if ( REQUESTKIND_EXTERNAL_ADDRESS == request.kind )
    {
        if ( request.hasFuture )
            request.source.Complete( request.result.Clone(), true );
        return CompleteAddressWaiters( true );
    }

    if ( REQUESTKIND_UNMAP == request.kind )
    {
        if ( request.hasFuture )
            request.source.Complete( request.result.Clone(), true );
        return true;
    }

    // A mapping was made or renewed: keep it alive from now on
    TMapping* mapping = FindMapping( request.protocol, request.internalPort );
    bool isNew = GUCEF_NULL == mapping;
    if ( isNew )
    {
        m_mappings.push_back( TMapping() );
        mapping = &m_mappings.back();
        mapping->protocol = request.protocol;
        mapping->internalPort = request.internalPort;
    }
    mapping->externalPort = request.result.externalPort;
    mapping->leaseInSecs = request.result.leaseInSecs;
    mapping->renewAtInMs = now + ( request.result.leaseInSecs * (UInt64) 1000 ) / 2;
    mapping->expiresAtInMs = now + request.result.leaseInSecs * (UInt64) 1000;
    mapping->renewing = false;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "NatPmpPortMapper: " << ( request.isRenewal ? "Renewed" : "Made" ) << " the mapping of "
        << COMCORE::CPortMappingResult::ProtocolToString( request.protocol ) << " port " << request.internalPort << " to external port "
        << request.result.externalPort << " for " << request.result.leaseInSecs << " seconds" );

    if ( !request.isRenewal )
    {
        if ( !request.usesPcp && !m_externalAddressKnown )
        {
            // Pending until the external address request is answered
            bool addressRequestPending = false;
            for ( size_t i=0; i<m_requests.size(); ++i )
                if ( REQUESTKIND_EXTERNAL_ADDRESS == m_requests[ i ].kind ) { addressRequestPending = true; break; }
            if ( addressRequestPending )
            {
                request.awaitsExternalAddress = true;
                m_requests.push_back( request );
                return NotifyObservers( MappingCreatedEvent, &request.result );
            }
        }
        if ( request.hasFuture )
            request.source.Complete( request.result.Clone(), true );
        return NotifyObservers( MappingCreatedEvent, &request.result );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::OnRequestFailed( size_t index, const CORE::CString& reason )
{GUCEF_TRACE;

    TRequest request = m_requests[ index ];
    m_requests.erase( m_requests.begin() + index );

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "NatPmpPortMapper: A request to the gateway " << m_gateway.AddressAndPortAsString() << " failed: " << reason );

    if ( request.hasFuture )
        request.source.Fail( CORE::TTaskStatus::TASKSTATUS_STARTUP_FAILED, reason );

    if ( REQUESTKIND_EXTERNAL_ADDRESS == request.kind )
        CompleteAddressWaiters( false );

    if ( request.isRenewal )
    {
        for ( size_t i=0; i<m_mappings.size(); ++i )
        {
            if ( m_mappings[ i ].protocol == request.protocol && m_mappings[ i ].internalPort == request.internalPort )
            {
                COMCORE::CPortMappingResult lost;
                lost.protocol = request.protocol;
                lost.internalPort = request.internalPort;
                lost.externalPort = m_mappings[ i ].externalPort;
                lost.mapperName = MapperName;
                if ( m_externalAddressKnown )
                {
                    lost.externalAddress = m_externalAddress;
                    lost.externalAddress.SetPortInHostByteOrder( lost.externalPort );
                }
                m_mappings.erase( m_mappings.begin() + i );
                if ( !NotifyObservers( MappingLostEvent, &lost ) ) return false;
                break;
            }
        }
    }

    TPortMapperErrorEventData eData( reason );
    return NotifyObservers( PortMapperErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::OnPcpResponse( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    if ( size < 24 || 0 == ( data[ 1 ] & PcpResponseBit ) )
        return true;

    UInt8 opcode = (UInt8)( data[ 1 ] & ~PcpResponseBit );
    UInt8 result = data[ 3 ];
    if ( PcpOpcodeMap != opcode || size < PcpMapResponseSize )
    {
        // An error answer may come without the opcode data, it is about the requests in flight
        if ( PcpResultUnsupportedVersion != result )
            return true;
    }

    if ( PcpResultUnsupportedVersion == result )
    {
        // A PCP gateway that does not take NAT-PMP: what went out as NAT-PMP goes again as PCP. There is no
        // PCP request for the external address alone, those get the address learned from the mappings.
        m_protocolMode = PROTOCOLMODE_PCP;
        size_t i = 0;
        while ( i < m_requests.size() )
        {
            TRequest& request = m_requests[ i ];
            if ( request.usesPcp || request.awaitsExternalAddress )
            {
                ++i;
                continue;
            }
            if ( REQUESTKIND_EXTERNAL_ADDRESS == request.kind )
            {
                if ( m_externalAddressKnown )
                {
                    request.result.externalAddress = m_externalAddress;
                    if ( !OnRequestSucceeded( i ) ) return false;
                }
                else
                {
                    if ( !OnRequestFailed( i, "The gateway only speaks PCP, which has no request for the external address alone" ) ) return false;
                }
                continue;
            }
            request.usesPcp = true;
            request.attempts = 0;
            request.intervalInMs = m_initialIntervalInMs;
            SendRequest( request );
            ++i;
        }
        return true;
    }

    const UInt8* nonce = data + 24;
    for ( size_t i=0; i<m_requests.size(); ++i )
    {
        TRequest& request = m_requests[ i ];
        if ( !request.usesPcp || request.awaitsExternalAddress || 0 != memcmp( request.nonce, nonce, 12 ) )
            continue;

        m_protocolMode = PROTOCOLMODE_PCP;
        if ( 0 != result )
            return OnRequestFailed( i, CORE::CString( "The gateway refused the PCP request: " ) + PcpResultToString( result ) );

        UInt32 lifetime = ReadUInt32At( data + 4 );
        UInt16 externalPort = ReadUInt16At( data + 42 );
        UInt32 externalAddress = 0;
        memcpy( &externalAddress, data + 56, 4 );

        request.result.externalPort = externalPort;
        request.result.leaseInSecs = lifetime;
        if ( 0 != externalAddress )
        {
            m_externalAddress = COMCORE::CIPv4Address( externalAddress, 0 );
            m_externalAddressKnown = true;
            request.result.externalAddress = COMCORE::CIPv4Address( externalAddress, COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( externalPort ) );
        }
        return OnRequestSucceeded( i );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::OnNatPmpResponse( const UInt8* data, UInt32 size )
{GUCEF_TRACE;

    if ( size < 8 || 0 == ( data[ 1 ] & NatPmpResponseBit ) )
        return true;

    UInt8 opcode = (UInt8)( data[ 1 ] & ~NatPmpResponseBit );
    UInt16 result = ReadUInt16At( data + 2 );

    if ( NatPmpResultUnsupportedVersion == result )
    {
        // The answer of a NAT-PMP only gateway to a PCP request
        m_protocolMode = PROTOCOLMODE_NATPMP;
        for ( size_t i=0; i<m_requests.size(); ++i )
        {
            TRequest& request = m_requests[ i ];
            if ( request.usesPcp )
            {
                request.usesPcp = false;
                request.attempts = 0;
                request.intervalInMs = m_initialIntervalInMs;
                SendRequest( request );
            }
        }

        // NAT-PMP map answers do not have the external address
        EnsureExternalAddressRequest();
        return true;
    }

    if ( NatPmpOpcodeExternalAddress == opcode )
    {
        if ( size < 12 )
            return true;
        for ( size_t i=0; i<m_requests.size(); ++i )
        {
            TRequest& request = m_requests[ i ];
            if ( request.usesPcp || REQUESTKIND_EXTERNAL_ADDRESS != request.kind )
                continue;

            m_protocolMode = PROTOCOLMODE_NATPMP == m_protocolMode || !m_usePcp ? PROTOCOLMODE_NATPMP : m_protocolMode;
            if ( 0 != result )
                return OnRequestFailed( i, CORE::CString( "The gateway refused the external address request: " ) + NatPmpResultToString( result ) );

            UInt32 externalAddress = 0;
            memcpy( &externalAddress, data + 8, 4 );
            m_externalAddress = COMCORE::CIPv4Address( externalAddress, 0 );
            m_externalAddressKnown = true;
            request.result.externalAddress = m_externalAddress;
            return OnRequestSucceeded( i );
        }
        return true;
    }

    if ( NatPmpOpcodeMapUdp == opcode || NatPmpOpcodeMapTcp == opcode )
    {
        if ( size < 16 )
            return true;
        TProtocol protocol = NatPmpOpcodeMapTcp == opcode ? COMCORE::CPortMappingResult::PROTOCOL_TCP : COMCORE::CPortMappingResult::PROTOCOL_UDP;
        UInt16 internalPort = ReadUInt16At( data + 8 );
        UInt16 externalPort = ReadUInt16At( data + 10 );
        UInt32 lifetime = ReadUInt32At( data + 12 );

        for ( size_t i=0; i<m_requests.size(); ++i )
        {
            TRequest& request = m_requests[ i ];
            if ( request.usesPcp || request.awaitsExternalAddress || REQUESTKIND_EXTERNAL_ADDRESS == request.kind ||
                 request.protocol != protocol || request.internalPort != internalPort )
                continue;

            m_protocolMode = PROTOCOLMODE_NATPMP;
            if ( 0 != result )
                return OnRequestFailed( i, CORE::CString( "The gateway refused the NAT-PMP request: " ) + NatPmpResultToString( result ) );

            request.result.externalPort = externalPort;
            request.result.leaseInSecs = lifetime;
            if ( m_externalAddressKnown )
                request.result.externalAddress = COMCORE::CIPv4Address( m_externalAddress.GetAddress(), COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( externalPort ) );
            return OnRequestSucceeded( i );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet )
{GUCEF_TRACE;

    // Only the gateway has a say
    if ( source.GetAddress() != m_gateway.GetAddress() || source.GetPort() != m_gateway.GetPort() )
        return true;

    const UInt8* data = static_cast< const UInt8* >( packet.GetConstBufferPtr() );
    UInt32 size = packet.GetDataSize();
    if ( size < 2 )
        return true;

    if ( PcpVersion == data[ 0 ] )
        return OnPcpResponse( data, size );
    if ( NatPmpVersion == data[ 0 ] )
        return OnNatPmpResponse( data, size );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::OnTimer( void )
{GUCEF_TRACE;

    UInt64 now = GetTimeInMs();
    bool needsExternalAddress = false;

    size_t i = 0;
    while ( i < m_requests.size() )
    {
        TRequest& request = m_requests[ i ];
        if ( request.awaitsExternalAddress || request.nextSendInMs > now )
        {
            ++i;
            continue;
        }

        // A gateway that does not answer PCP may still answer NAT-PMP, try that before giving up
        UInt32 maxAttempts = request.usesPcp && PROTOCOLMODE_UNKNOWN == m_protocolMode ? PcpAttemptsBeforeFallback : m_maxAttempts;
        if ( request.attempts >= maxAttempts )
        {
            if ( request.usesPcp && PROTOCOLMODE_PCP != m_protocolMode )
            {
                request.usesPcp = false;
                request.attempts = 0;
                request.intervalInMs = m_initialIntervalInMs;
                SendRequest( request );
                needsExternalAddress = needsExternalAddress || REQUESTKIND_MAP == request.kind;
                ++i;
                continue;
            }
            if ( !OnRequestFailed( i, "The gateway " + m_gateway.AddressAndPortAsString() + " did not answer" ) )
                return false;
            continue;
        }
        SendRequest( request );
        ++i;
    }

    // NAT-PMP map answers do not have the external address
    if ( needsExternalAddress )
        EnsureExternalAddressRequest();

    // Renewals at half the lease
    for ( size_t m=0; m<m_mappings.size(); ++m )
    {
        TMapping& mapping = m_mappings[ m ];
        if ( !mapping.renewing && mapping.renewAtInMs <= now )
        {
            mapping.renewing = true;
            TRequest request;
            request.kind = REQUESTKIND_MAP;
            request.protocol = mapping.protocol;
            request.internalPort = mapping.internalPort;
            request.externalPort = mapping.externalPort;
            request.leaseInSecs = mapping.leaseInSecs;
            request.usesPcp = m_usePcp && PROTOCOLMODE_NATPMP != m_protocolMode;
            request.isRenewal = true;
            QueueRequest( request, GUCEF_NULL );
        }
    }

    if ( m_requests.empty() && m_mappings.empty() )
        m_timer.SetEnabled( false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CNatPmpPortMapper::OnNotify( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_socket )
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent == eventId && GUCEF_NULL != eventData )
        {
            const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
            for ( UInt32 i=0; i<data.packetsReceived; ++i )
            {
                if ( !OnPacket( data.packets[ i ].sourceAddress, data.packets[ i ].dataBuffer.GetData() ) )
                    return;
            }
        }
        return;
    }

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId )
        OnTimer();
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString gateway = config.GetAttributeValueOrChildValueByName( "gateway" ).AsString( CORE::CString::Empty, true );
    if ( !gateway.IsNULLOrEmpty() )
    {
        COMCORE::CHostAddress gatewayHost;
        if ( gateway.HasChar( ':' ) >= 0 )
            gatewayHost.SetHostnameAndPort( gateway );
        else
            gatewayHost.SetHostnameAndPort( gateway, GatewayPort );
        if ( gatewayHost.GetIPv4Addresses().empty() )
            return false;
        SetGateway( gatewayHost.GetFirstIPv4Address() );
    }
    m_usePcp = config.GetAttributeValueOrChildValueByName( "usePcp" ).AsBool( m_usePcp, true );
    SetRetransmission( config.GetAttributeValueOrChildValueByName( "initialRetransmitIntervalInMs" ).AsUInt32( m_initialIntervalInMs, true ),
                       config.GetAttributeValueOrChildValueByName( "maxAttempts" ).AsUInt32( m_maxAttempts, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CNatPmpPortMapper::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( m_gatewayConfigured )
        config.SetAttribute( "gateway", m_gateway.AddressAndPortAsString() );
    config.SetAttribute( "usePcp", m_usePcp );
    config.SetAttribute( "initialRetransmitIntervalInMs", m_initialIntervalInMs );
    config.SetAttribute( "maxAttempts", m_maxAttempts );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CNatPmpPortMapper::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CNatPmpPortMapper";
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
