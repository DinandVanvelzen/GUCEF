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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CUDPDISCOVERYCODEC_H
#include "gucefCOM_CUdpDiscoveryCodec.h"
#define GUCEF_COM_CUDPDISCOVERYCODEC_H
#endif /* GUCEF_COM_CUDPDISCOVERYCODEC_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefCOM_CUdpDiscoveryClientBase.h"

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

const UInt16 CUdpDiscoveryClientBase::DefaultPort = 47800;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/**
 *  An advertiser does not always know the address it is reached at, it can give 0.0.0.0 or no host instead:
 *  the receiver puts in the address the beacon came from
 */
void
ResolveEndpointHosts( CORE::CString::StringVector& endpoints, const CORE::CString& sourceHost )
{GUCEF_TRACE;

    for ( size_t i=0; i<endpoints.size(); ++i )
    {
        CORE::CString& endpoint = endpoints[ i ];
        Int32 schemeEnd = endpoint.HasSubstr( "://", true );
        UInt32 hostStart = schemeEnd >= 0 ? (UInt32) schemeEnd + 3 : 0;
        CORE::CString rest = endpoint.CutChars( hostStart, true, 0 );
        if ( rest.StartsWith( "0.0.0.0" ) )
        {
            endpoint = endpoint.SubstrToIndex( hostStart, true ) + sourceHost + rest.CutChars( 7, true, 0 );
        }
        else
        if ( rest.StartsWith( ":" ) )
        {
            endpoint = endpoint.SubstrToIndex( hostStart, true ) + sourceHost + rest;
        }
    }
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CUdpDiscoveryClientBase::CUdpDiscoveryClientBase( const CORE::CString& defaultClientName      ,
                                                  const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CDiscoveryClientBase( defaultClientName, pulseGenerator )
    , m_socket( GetPulseGenerator(), false )
    , m_port( DefaultPort )
    , m_extraTargets()
    , m_destinations()
    , m_sendBuffer( CUdpDiscoveryCodec::MaxBeaconSizeInBytes, true )
    , m_nextAnnounceTimes()
    , m_sendErrorReported( false )
{GUCEF_TRACE;

    m_socket.SetNrOfReceiveBuffers( 4, 2048 );
    RegisterSocketEventHandlers();
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::RegisterSocketEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback callback( this, &CUdpDiscoveryClientBase::OnSocketPackets );
    SubscribeTo( &m_socket                                ,
                 COMCORE::CUDPSocket::UDPPacketsRecievedEvent ,
                 callback                                 );
}

/*-------------------------------------------------------------------------*/

CUdpDiscoveryClientBase::~CUdpDiscoveryClientBase()
{GUCEF_TRACE;

    UnsubscribeFrom( &m_socket );
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::Shutdown( void )
{GUCEF_TRACE;

    StopAll();
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::SetPort( UInt16 port )
{GUCEF_TRACE;

    if ( port != m_port )
    {
        m_port = port;
        if ( m_socket.IsActive() )
        {
            m_socket.Close();
            EnsureSocketIsOpen();
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt16
CUdpDiscoveryClientBase::GetPort( void ) const
{GUCEF_TRACE;

    return m_port;
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::AddExtraTarget( const COMCORE::CIPv4Address& target )
{GUCEF_TRACE;

    m_extraTargets.push_back( target );
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::ClearExtraTargets( void )
{GUCEF_TRACE;

    m_extraTargets.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CUdpDiscoveryClientBase::JoinOnAllInterfaces( COMCORE::CUDPSocket& socket, const COMCORE::CIPv4Address& group )
{GUCEF_TRACE;

    UInt32 joinCount = 0;
    COMCORE::CINetworkInterface::TIPv4InfoVector ipInfo;
    if ( COMCORE::CComCoreGlobal::Instance()->GetCom().GetAllNetworkInterfaceIPInfo( ipInfo ) )
    {
        COMCORE::CIPv4Address::TIPv4AddressSet joined;
        for ( size_t i=0; i<ipInfo.size(); ++i )
        {
            COMCORE::CIPv4Address interfaceAddress( ipInfo[ i ].ip.GetAddress(), 0 );
            if ( 0 == interfaceAddress.GetAddress() || !joined.insert( interfaceAddress ).second )
                continue;
            if ( socket.JoinOnInterface( group, interfaceAddress ) )
                ++joinCount;
        }
    }
    if ( 0 == joinCount && socket.JoinOnInterface( group, COMCORE::CIPv4Address::Any ) )
        ++joinCount;
    return joinCount;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::IsActive( void ) const
{GUCEF_TRACE;

    return m_socket.IsActive();
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::EnsureSocketIsOpen( void )
{GUCEF_TRACE;

    if ( m_socket.IsActive() )
        return true;

    if ( !OpenSocket( m_socket, m_port ) )
    {
        ReportDiscoveryError( "Unable to open the discovery socket on port " + CORE::UInt16ToString( m_port ) );
        return false;
    }

    m_destinations.clear();
    GetDestinations( m_destinations );
    m_sendErrorReported = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::SendBeacon( const CORE::CDynamicBuffer& beacon )
{GUCEF_TRACE;

    if ( !EnsureSocketIsOpen() )
        return false;

    UInt32 sent = 0;
    UInt32 attempts = 0;
    for ( int list=0; list<2; ++list )
    {
        const TIPv4AddressVector& targets = 0 == list ? m_destinations : m_extraTargets;
        for ( size_t i=0; i<targets.size(); ++i )
        {
            ++attempts;
            Int32 result = m_socket.SendPacketTo( targets[ i ], beacon.GetConstBufferPtr(), (UInt16) beacon.GetDataSize() );
            if ( result == (Int32) beacon.GetDataSize() )
                ++sent;
            else
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "UdpDiscoveryClient(" << GetDiscoveryClientName() << "): Failed to send a beacon to " << targets[ i ].AddressAndPortAsString() );
        }
    }

    if ( 0 == sent && !m_sendErrorReported )
    {
        // Only once, a network that is not there tends to stay away for a while
        m_sendErrorReported = true;
        ReportDiscoveryError( "None of the " + CORE::UInt32ToString( attempts ) + " destinations of the discovery beacons could be reached" );
        return false;
    }
    if ( sent > 0 )
        m_sendErrorReported = false;
    return sent > 0;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::Announce( const COMCORE::CIDiscoveryEntry& entry, UInt64 nowInMs )
{GUCEF_TRACE;

    UInt32 ttl = entry.GetTtlInSecs();
    m_nextAnnounceTimes[ entry.GetEntryKey() ] = nowInMs + ( ttl > 0 ? ( ttl * (UInt64) 1000 ) / 3 : (UInt64) 1000 );

    if ( !CUdpDiscoveryCodec::EncodeEntry( CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE, entry, m_sendBuffer ) )
    {
        ReportDiscoveryError( "The entry " + entry.GetEntryKey() + " does not fit in a discovery beacon" );
        return false;
    }
    return SendBeacon( m_sendBuffer );
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::OnBrowseStarted( const CORE::CString& serviceType )
{GUCEF_TRACE;

    if ( !CUdpDiscoveryCodec::EncodeQuery( serviceType, m_sendBuffer ) )
        return false;
    return SendBeacon( m_sendBuffer );
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::OnAdvertised( const COMCORE::CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    // The socket also has to be open to hear queries, even when this first announcement does not get out
    if ( !EnsureSocketIsOpen() )
        return false;
    Announce( *entry, GetTimeInMs() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::OnWithdrawn( const COMCORE::CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    m_nextAnnounceTimes.erase( entry->GetEntryKey() );
    if ( !m_socket.IsActive() )
        return true;
    if ( !CUdpDiscoveryCodec::EncodeEntry( CUdpDiscoveryCodec::BEACONKIND_GOODBYE, *entry, m_sendBuffer ) )
        return false;
    return SendBeacon( m_sendBuffer );
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::OnIdle( void )
{GUCEF_TRACE;

    m_socket.Close();
    m_destinations.clear();
    m_nextAnnounceTimes.clear();
    m_sendErrorReported = false;
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::OnHousekeeping( UInt64 nowInMs )
{GUCEF_TRACE;

    TIDiscoveryEntryPtrVector advertised;
    GetAdvertisedEntries( advertised );
    for ( size_t i=0; i<advertised.size(); ++i )
    {
        TAnnounceTimeMap::iterator n = m_nextAnnounceTimes.find( advertised[ i ]->GetEntryKey() );
        if ( n == m_nextAnnounceTimes.end() || (*n).second <= nowInMs )
            Announce( *advertised[ i ], nowInMs );
    }
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet )
{GUCEF_TRACE;

    if ( !CUdpDiscoveryCodec::IsBeacon( packet.GetConstBufferPtr(), packet.GetDataSize() ) )
        return true;

    CUdpDiscoveryCodec::TBeaconKind kind = CUdpDiscoveryCodec::BEACONKIND_QUERY;
    COMCORE::CDiscoveryEntry entry;
    if ( !CUdpDiscoveryCodec::Decode( packet.GetConstBufferPtr(), packet.GetDataSize(), kind, entry ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "UdpDiscoveryClient(" << GetDiscoveryClientName() << "): Ignoring a malformed beacon from " << source.AddressAndPortAsString() );
        return true;
    }

    switch ( kind )
    {
        case CUdpDiscoveryCodec::BEACONKIND_QUERY:
        {
            // Everyone browsing hears the answer, it goes to the same destinations as the rest
            const CORE::CString& serviceType = entry.GetServiceType();
            bool anyServiceType = serviceType == COMCORE::CIDiscoveryClient::AnyServiceType;
            UInt64 nowInMs = GetTimeInMs();
            TIDiscoveryEntryPtrVector advertised;
            GetAdvertisedEntries( advertised );
            for ( size_t i=0; i<advertised.size(); ++i )
            {
                if ( anyServiceType || advertised[ i ]->GetServiceType() == serviceType )
                    Announce( *advertised[ i ], nowInMs );
            }
            return true;
        }
        case CUdpDiscoveryCodec::BEACONKIND_ANNOUNCE:
        {
            CORE::CString sourceHost = source.AddressAsString();
            entry.SetHost( COMCORE::CHostAddress( source, sourceHost ) );
            ResolveEndpointHosts( entry.GetEndpoints(), sourceHost );
            return ProcessHeardEntry( entry );
        }
        case CUdpDiscoveryCodec::BEACONKIND_GOODBYE:
        {
            CORE::CString instanceId;
            entry.GetApplicationInstanceId( instanceId );
            if ( instanceId.IsNULLOrEmpty() )
                instanceId = entry.GetInstanceName();
            return ProcessGoodbye( entry.GetServiceType(), instanceId );
        }
        default:
            return true;
    }
}

/*-------------------------------------------------------------------------*/

void
CUdpDiscoveryClientBase::OnSocketPackets( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == eventData )
        return;

    const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
    for ( UInt32 i=0; i<data.packetsReceived; ++i )
    {
        const COMCORE::CUDPSocket::TPacketEntry& packet = data.packets[ i ];
        if ( !OnPacket( packet.sourceAddress, packet.dataBuffer.GetData() ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !COMCORE::CDiscoveryClientBase::LoadConfig( config ) )
        return false;

    SetPort( config.GetAttributeValueOrChildValueByName( "port" ).AsUInt16( m_port, true ) );

    CORE::CString extraTargets = config.GetAttributeValueOrChildValueByName( "extraTargets" ).AsString( CORE::CString::Empty, true );
    if ( !extraTargets.IsNULLOrEmpty() )
    {
        m_extraTargets.clear();
        CORE::CString::StringVector targets = extraTargets.ParseElements( ',', false );
        for ( size_t i=0; i<targets.size(); ++i )
        {
            COMCORE::CHostAddress target( targets[ i ].Trim( true ).Trim( false ) );
            if ( target.GetIPv4Addresses().empty() )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "UdpDiscoveryClient(" << GetDiscoveryClientName() << "): Unable to resolve the extra target " << targets[ i ] );
                continue;
            }
            m_extraTargets.push_back( target.GetFirstIPv4Address() );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUdpDiscoveryClientBase::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !COMCORE::CDiscoveryClientBase::SaveConfig( config ) )
        return false;

    config.SetAttribute( "port", m_port );

    CORE::CString extraTargets;
    for ( size_t i=0; i<m_extraTargets.size(); ++i )
    {
        if ( i > 0 )
            extraTargets += ',';
        extraTargets += m_extraTargets[ i ].AddressAndPortAsString();
    }
    config.SetAttribute( "extraTargets", extraTargets );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
