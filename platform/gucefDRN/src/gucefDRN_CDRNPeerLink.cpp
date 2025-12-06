/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_CSTREAMEREVENTS_H
#include "CStreamerEvents.h"
#define GUCEF_CORE_CSTREAMEREVENTS_H
#endif /* GUCEF_CORE_CSTREAMEREVENTS_H ? */

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#include "CTCPConnection.h"
#define GUCEF_COMCORE_CTCPCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

#ifndef GUCEF_COMCORE_CUDPCHANNEL_H
#include "CUDPChannel.h"
#define GUCEF_COMCORE_CUDPCHANNEL_H
#endif /* GUCEF_COMCORE_CUDPCHANNEL_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNPEERLINKDATA_H
#include "gucefDRN_CDRNPeerLinkData.h"
#define GUCEF_DRN_CDRNPEERLINKDATA_H
#endif /* GUCEF_DRN_CDRNPEERLINKDATA_H ? */

#ifndef GUCEF_COMCORE_CUDPMASTERSOCKET_H
#include "CUDPMasterSocket.h"
#define GUCEF_COMCORE_CUDPMASTERSOCKET_H
#endif /* GUCEF_COMCORE_CUDPMASTERSOCKET_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CIDRNPEERVALIDATOR_H
#include "gucefDRN_CIDRNPeerValidator.h"
#define GUCEF_DRN_CIDRNPEERVALIDATOR_H
#endif /* GUCEF_DRN_CIDRNPEERVALIDATOR_H ? */

#include "gucefDRN_CDRNPeerLink.h"

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

const CORE::CEvent CDRNPeerLink::ConnectedEvent = "GUCEF::DRN::CDRNPeerLink::ConnectedEvent";
const CORE::CEvent CDRNPeerLink::DisconnectedEvent = "GUCEF::DRN::CDRNPeerLink::DisconnectedEvent";
const CORE::CEvent CDRNPeerLink::SocketErrorEvent = "GUCEF::DRN::CDRNPeerLink::SocketErrorEvent";
const CORE::CEvent CDRNPeerLink::LinkCorruptionEvent = "GUCEF::DRN::CDRNPeerLink::LinkCorruptionEvent";
const CORE::CEvent CDRNPeerLink::LinkProtocolMismatchEvent = "GUCEF::DRN::CDRNPeerLink::LinkProtocolMismatchEvent";
const CORE::CEvent CDRNPeerLink::LinkProtocolMatchEvent = "GUCEF::DRN::CDRNPeerLink::LinkProtocolMatchEvent";
const CORE::CEvent CDRNPeerLink::LinkIncompatibleEvent = "GUCEF::DRN::CDRNPeerLink::LinkIncompatibleEvent";
const CORE::CEvent CDRNPeerLink::LinkOperationalForPeerEvent = "GUCEF::DRN::CDRNPeerLink::LinkOperationalForPeerEvent";
const CORE::CEvent CDRNPeerLink::LinkOperationalForUsEvent = "GUCEF::DRN::CDRNPeerLink::LinkOperationalForUsEvent";
const CORE::CEvent CDRNPeerLink::IllegalRequestEvent = "GUCEF::DRN::CDRNPeerLink::IllegalRequestEvent";
const CORE::CEvent CDRNPeerLink::CompatibleServiceEvent = "GUCEF::DRN::CDRNPeerLink::CompatibleServiceEvent";
const CORE::CEvent CDRNPeerLink::IncompatibleServiceEvent = "GUCEF::DRN::CDRNPeerLink::IncompatibleServiceEvent";
const CORE::CEvent CDRNPeerLink::PeerAuthenticationSuccessEvent = "GUCEF::DRN::CDRNPeerLink::PeerAuthenticationSuccessEvent";
const CORE::CEvent CDRNPeerLink::AuthenticationSuccessEvent = "GUCEF::DRN::CDRNPeerLink::AuthenticationSuccessEvent";
const CORE::CEvent CDRNPeerLink::PeerAuthenticationFailureEvent = "GUCEF::DRN::CDRNPeerLink::PeerAuthenticationFailureEvent";
const CORE::CEvent CDRNPeerLink::AuthenticationFailureEvent = "GUCEF::DRN::CDRNPeerLink::AuthenticationFailureEvent";
const CORE::CEvent CDRNPeerLink::PeerListReceivedFromPeerEvent = "GUCEF::DRN::CDRNPeerLink::PeerListReceivedFromPeerEvent";
const CORE::CEvent CDRNPeerLink::StreamListReceivedFromPeerEvent = "GUCEF::DRN::CDRNPeerLink::StreamListReceivedFromPeerEvent";
const CORE::CEvent CDRNPeerLink::DataGroupListReceivedFromPeerEvent = "GUCEF::DRN::CDRNPeerLink::DataGroupListReceivedFromPeerEvent";
const CORE::CEvent CDRNPeerLink::SubscribedToDataGroupEvent = "GUCEF::DRN::CDRNPeerLink::SubscribedToDataGroupEvent";
const CORE::CEvent CDRNPeerLink::SubscribedToDataStreamEvent = "GUCEF::DRN::CDRNPeerLink::SubscribedToDataStreamEvent";
const CORE::CEvent CDRNPeerLink::UnsubscribedFromDataGroupEvent = "GUCEF::DRN::CDRNPeerLink::UnsubscribedFromDataGroupEvent";
const CORE::CEvent CDRNPeerLink::UnsubscribedFromDataStreamEvent = "GUCEF::DRN::CDRNPeerLink::UnsubscribedFromDataStreamEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDRNPeerLink::RegisterEvents( void )
{GUCEF_TRACE;

    ConnectedEvent.Initialize();
    DisconnectedEvent.Initialize();
    SocketErrorEvent.Initialize();
    LinkCorruptionEvent.Initialize();
    LinkProtocolMismatchEvent.Initialize();
    LinkProtocolMatchEvent.Initialize();
    LinkIncompatibleEvent.Initialize();
    LinkOperationalForPeerEvent.Initialize();
    LinkOperationalForUsEvent.Initialize();
    IllegalRequestEvent.Initialize();
    CompatibleServiceEvent.Initialize();
    IncompatibleServiceEvent.Initialize();
    PeerAuthenticationSuccessEvent.Initialize();
    AuthenticationSuccessEvent.Initialize();
    PeerAuthenticationFailureEvent.Initialize();
    AuthenticationFailureEvent.Initialize();
    PeerListReceivedFromPeerEvent.Initialize();
    StreamListReceivedFromPeerEvent.Initialize();
    DataGroupListReceivedFromPeerEvent.Initialize();
    SubscribedToDataGroupEvent.Initialize();
    SubscribedToDataStreamEvent.Initialize();
    UnsubscribedFromDataGroupEvent.Initialize();
    UnsubscribedFromDataStreamEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink::CDRNPeerLink( CDRNNode& parentNode                   ,
                            COMCORE::CTCPConnection& tcpConnection ,
                            COMCORE::CUDPMasterSocket& udpSocket   )
    : m_udpSocket( &udpSocket )           ,
      m_tcpConnection( &tcpConnection )   ,
      m_isAuthenticated( false )          ,
      m_isPeerAuthenticated( false )      ,
      m_linkData( NULL )                  ,
      m_parentNode( &parentNode )         ,
      m_tcpStreamBuffer()                 ,
      m_tcpStreamKeepBytes( 0 )           ,
      m_isLinkOperationalForPeer( false ) ,
      m_isLinkOperationalForUs( false )   ,
      m_peerUDPPortOpened( false )        ,
      m_peerUDPPort( 53457 )              ,
      m_peerTCPConnectBackInfo()
{GUCEF_TRACE;

    assert( m_udpSocket != NULL );
    assert( m_parentNode != NULL );
    assert( m_tcpConnection != NULL );
    RegisterEvents();

    // Verify that we are connected
    if ( tcpConnection.IsActive() )
    {
        m_linkData = new CDRNPeerLinkData( *this );
        SubscribeTo( &tcpConnection );

        // Init the connect back settings based on what we have available
        // Small chance these values allow for a connection to be established but it's
        // the best we have until we get connect back info from the peer itself
        m_peerTCPConnectBackInfo.SetHostname( tcpConnection.GetRemoteHostName() );
        m_peerTCPConnectBackInfo.SetPortInHostByteOrder( tcpConnection.GetRemoteTCPPort() );

        NotifyObservers( ConnectedEvent );
        SendGreetingMessage();
    }
}

/*-------------------------------------------------------------------------*/

CDRNPeerLink::~CDRNPeerLink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::GetPeerConnectBackInfo( CHostAddress& peerHostAddress ,
                                      UInt16& peerUDPPort           ,
                                      bool& isPeerUDPPortOpened     ) const
{GUCEF_TRACE;

    peerHostAddress = m_peerTCPConnectBackInfo;
    peerUDPPort = m_peerUDPPort;
    isPeerUDPPortOpened = m_peerUDPPortOpened;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendGreetingMessage( void )
{GUCEF_TRACE;

    // Compose the greeting message
    char sendBuffer[ 8 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_GREETING, DRN_PROTOCOL_MAYOR_VERION, DRN_PROTOCOL_MINOR_VERION, DRN_PROTOCOL_PATCH_VERION, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 4;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 8, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::CloseLink( void )
{GUCEF_TRACE;

    if ( m_tcpConnection != NULL )
    {
        m_tcpConnection->Close();
        UnsubscribeFrom( m_tcpConnection );
    }
    m_peerUDPPortOpened = false;
    m_isAuthenticated = false;
    m_tcpConnection = NULL;
    m_udpSocket = NULL;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNPeerLink::GetPeerHostName( void ) const
{GUCEF_TRACE;

    if ( NULL != m_tcpConnection )
    {
        return m_tcpConnection->GetRemoteHostName();
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

UInt16
CDRNPeerLink::GetPeerTCPPort( void ) const
{GUCEF_TRACE;

    if ( NULL != m_tcpConnection )
    {
        return m_tcpConnection->GetRemoteTCPPort();
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsUDPPossible( void ) const
{GUCEF_TRACE;

    return m_peerUDPPortOpened             &&
           m_parentNode->IsUDPPortOpened() &&
           m_udpSocket != NULL;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsAuthenticated( void ) const
{GUCEF_TRACE;

    return m_isAuthenticated;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsPeerAuthenticated( void ) const
{GUCEF_TRACE;

    return m_isPeerAuthenticated;
}

/*-------------------------------------------------------------------------*/

COMCORE::CTCPConnection*
CDRNPeerLink::GetTCPConnection( void )
{GUCEF_TRACE;

    return m_tcpConnection;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsOperationalForUs( void ) const
{GUCEF_TRACE;

    return m_isLinkOperationalForUs;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::IsOperationalForPeer( void ) const
{GUCEF_TRACE;

    return m_isLinkOperationalForPeer;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::SendData( const void* dataSource                   ,
                        const UInt16 dataSize                    ,
                        const bool allowUnreliable /* = false */ )
{GUCEF_TRACE;

    if ( allowUnreliable &&
         IsUDPPossible()  )
    {
        // Send the data using UDP
        COMCORE::CIPv4Address ip( m_peerTCPConnectBackInfo.GetFirstAddressAsString() ,
                                  m_peerUDPPort                                      );
        return m_udpSocket->SendPacketTo( ip         ,
                                          dataSource ,
                                          dataSize   ) >= 0;

    }
    else
    if ( m_tcpConnection != NULL )
    {
        // Send the data using TCP
        return m_tcpConnection->Send( dataSource ,
                                      dataSize   );
    }

    return false;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendServiceTypeMessage( void )
{GUCEF_TRACE;

    // Obtain the service name from our parent node
    CORE::CString serviceName;
    CIDRNPeerValidator* validator = m_parentNode->GetPeervalidator();
    if ( NULL != validator )
    {
        serviceName = validator->GetServiceName( *this );
    }

    // Compose the service type message
    CORE::CDynamicBuffer sendBuffer( serviceName.Length() + 5 , true );
    sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
    sendBuffer[ 3 ] = DRN_PEERCOMM_SERVICE;
    sendBuffer[ serviceName.Length() + 4 ] = DRN_TRANSMISSION_END;

    UInt16 payloadSize = (UInt16)serviceName.Length()+1;
    sendBuffer.CopyFrom( 1, 2, &payloadSize );
    sendBuffer.CopyFrom( 4, serviceName.Length(), serviceName.C_String() );

    // Send the service name message
    if ( !SendData( sendBuffer.GetConstBufferPtr()   ,
                    (UInt16)sendBuffer.GetDataSize() ,
                    false                            ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerLinkIncompatible( void )
{GUCEF_TRACE;

    NotifyObservers( LinkIncompatibleEvent );
    CloseLink();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerConnectBackInfo( const char* data      ,
                                     const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the data size
    if ( dataSize > 6 )
    {
        m_peerUDPPortOpened = *(data+1) == 1;
        m_peerUDPPort = *reinterpret_cast< const UInt16* >( data+2 );
        m_peerTCPConnectBackInfo.SetPortInHostByteOrder( *reinterpret_cast< const UInt16* >( data+4 ) );

        CORE::CString hostname;
        hostname.Scan( data+6, dataSize-6 );
        m_peerTCPConnectBackInfo.SetHostname( hostname );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendConnectBackInfo( void )
{GUCEF_TRACE;

    // Check whether we should use the actual settings or the override
    // settings. An override may be done to solve connection problems.
    UInt16 udpPort = 0;
    char udpPortOpen = m_parentNode->IsUDPPortOpened() ? 1 : 0;
    CDRNNode::CHostAddress hostAddress;
    if ( m_parentNode->GetOverrideConnectBackSettings() )
    {
        m_parentNode->GetConnectBackOverride( hostAddress ,
                                              udpPort     );
    }
    else
    {
        m_parentNode->GetListenAddress( hostAddress );
        udpPort = m_parentNode->GetUDPPort();
    }

    // Compose the message
    UInt16 tcpPort = hostAddress.GetPortInHostByteOrder();
    UInt32 hostnameLength = hostAddress.GetHostname().Length();
    CORE::CDynamicBuffer sendBuffer( hostnameLength + 10, true );
    sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
    sendBuffer[ 3 ] = DRN_PEERCOMM_CONNECTBACKINFO;
    sendBuffer[ 4 ] = udpPortOpen;
    sendBuffer.CopyFrom( 5, 2, &udpPort );
    sendBuffer.CopyFrom( 7, 2, &tcpPort );
    sendBuffer.CopyFrom( 9, hostnameLength, hostAddress.GetHostname().C_String() );
    sendBuffer[ hostnameLength + 9 ] = DRN_TRANSMISSION_END;

    UInt16 payloadSize = (UInt16) hostnameLength+6;
    sendBuffer.CopyFrom( 1, 2, &payloadSize );

    // Send the connect back info
    SendData( sendBuffer.GetConstBufferPtr()  ,
             (UInt16)sendBuffer.GetDataSize() ,
             false                            );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendIncompatibleLinkMessage( void )
{GUCEF_TRACE;

    // Compose the message
    char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_INCOMPATIBLE_LINK, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 1;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 5, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendLinkOperationalMessage( void )
{GUCEF_TRACE;

    // Compose the link operational message
    char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_LINK_OPERATIONAL, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 1;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 5, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendAuthenticationRequiredMessage( void )
{GUCEF_TRACE;

    // Compose the link operational message
    char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_AUTHENTICATION_REQUIRED, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 1;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 5, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerServicesCompatible( void )
{GUCEF_TRACE;

    NotifyObservers( CompatibleServiceEvent );

    // Check if a validator mechanism has been provided to the node
    CIDRNPeerValidator* validator = m_parentNode->GetPeervalidator();
    if ( validator != NULL )
    {
        // A validator is available for use
        if ( validator->IsPeerLoginRequired( *this ) )
        {
            // Peer authentication is mandatory
            SendAuthenticationRequiredMessage();
            return;
        }
    }

    // Peer authentication is optional, we can proceed without it
    m_isLinkOperationalForPeer = true;
    SendLinkOperationalMessage();
    NotifyObservers( LinkOperationalForPeerEvent );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerServiceType( const char* data      ,
                                 const UInt32 dataSize )
{GUCEF_TRACE;

    // Obtain the local service name
    CORE::CString localService;
    CIDRNPeerValidator* validator = m_parentNode->GetPeervalidator();
    if ( NULL != validator )
    {
        localService = validator->GetServiceName( *this );
    }

    // Check if the payload contains a service name
    if ( dataSize > 1 )
    {
        // Build the remote service string
        UInt32 serviceStringSize = dataSize - 1;
        CORE::CString remoteService( data+1, serviceStringSize );

        bool isValidService = false;
        if ( NULL != validator )
        {
            isValidService = validator->IsPeerServiceCompatible( *this         ,
                                                                 remoteService );
        }
        else
        {
            // Simply compare the service names
            isValidService = remoteService == localService;
        }

        if ( isValidService )
        {
            // The service names match, the link is compatible
            OnPeerServicesCompatible();
            return;
        }
        else
        {
            // Incompatible: the local service does not match the remote service name.
            // The names have to match for the link to be considered compatible
            NotifyObservers( IncompatibleServiceEvent );
            NotifyObservers( LinkIncompatibleEvent );
            SendIncompatibleLinkMessage();
            CloseLink();
            return;
        }
    }
    else
    if ( localService.Length() > 0 )
    {
        // Incompatible: the local service is not unnamed
        // Both have to be unnamed to be considered as compatible
        NotifyObservers( IncompatibleServiceEvent );
        NotifyObservers( LinkIncompatibleEvent );
        SendIncompatibleLinkMessage();
        CloseLink();
        return;
    }

    //Both services are without a name: always considered compatible
    OnPeerServicesCompatible();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerLinkOperational( void )
{GUCEF_TRACE;

    m_isLinkOperationalForUs = true;
    NotifyObservers( LinkOperationalForUsEvent );

    SendConnectBackInfo();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerDataGroupRequest( void )
{GUCEF_TRACE;

    // Access rights check
    if ( m_isLinkOperationalForPeer )
    {
        // Obtain a list of streams publicized on this link
        CDRNPeerLinkData::TDRNDataGroupList dataGroupList;
        m_linkData->GetPublicizedDataGroups( dataGroupList );

        // Compose the list message
        CORE::CDynamicBuffer streamBuffer( 4, true );
        streamBuffer[ 0 ] = DRN_TRANSMISSION_START;
        streamBuffer[ 3 ] = DRN_PEERCOMM_DATAGROUPLIST;

        // Copy the list into the buffer
        CDRNPeerLinkData::TDRNDataGroupList::iterator i = dataGroupList.begin();
        while ( i != dataGroupList.end() )
        {
            // Copy the string into the buffer including the null terminator
            streamBuffer.Append( (*i)->GetName().C_String(), (*i)->GetName().Length()+1 );
            ++i;
        }

        // Fill in the payload size segment
        UInt16 payloadSize = (UInt16) streamBuffer.GetDataSize() - 3;
        streamBuffer.CopyFrom( 1, 2, &payloadSize );

        // Add the transmission delimiter
        UInt8 delimiter = DRN_TRANSMISSION_END;
        streamBuffer.Append( &delimiter, 1 );

        if ( !SendData( streamBuffer.GetBufferPtr()        ,
                        (UInt16)streamBuffer.GetDataSize() ,
                        false                              ) )
        {
            // Failed to send data, something is very wrong
            CloseLink();
        }
    }
    else
    {
        SendNotAllowed();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerStreamListRequest( void )
{GUCEF_TRACE;

    // Access rights check
    if ( m_isLinkOperationalForPeer )
    {
        // Obtain a list of streams publicized on this link
        CDRNPeerLinkData::TDRNDataStreamList dataStreamList;
        m_linkData->GetPublicizedDataStreams( dataStreamList );

        // Compose the list message
        CORE::CDynamicBuffer streamBuffer( 4, true );
        streamBuffer[ 0 ] = DRN_TRANSMISSION_START;
        streamBuffer[ 3 ] = DRN_PEERCOMM_STREAMLIST;

        // Copy the list into the buffer
        CDRNPeerLinkData::TDRNDataStreamList::iterator i = dataStreamList.begin();
        while ( i != dataStreamList.end() )
        {
            // Copy the string into the buffer including the null terminator
            streamBuffer.Append( (*i)->GetName().C_String(), (*i)->GetName().Length()+1 );
            ++i;
        }

        // Fill in the payload size segment
        UInt16 payloadSize = (UInt16) streamBuffer.GetDataSize() - 3;
        streamBuffer.CopyFrom( 1, 2, &payloadSize );

        // Add the transmission delimiter
        UInt8 delimiter = DRN_TRANSMISSION_END;
        streamBuffer.Append( &delimiter, 1 );

        if ( !SendData( streamBuffer.GetBufferPtr()        ,
                        (UInt16)streamBuffer.GetDataSize() ,
                        false                              ) )
        {
            // Failed to send data, something is very wrong
            CloseLink();
        }
    }
    else
    {
        SendNotAllowed();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerPeerListRequest( void )
{GUCEF_TRACE;

    // Access rights check
    if ( m_isLinkOperationalForPeer )
    {
        // @TODO
    }
    else
    {
        SendNotAllowed();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerStreamListReceived( const char* data      ,
                                        const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the logic flow
    if ( m_isLinkOperationalForUs )
    {
        TStringList streamNameList;

        CORE::CString tmpStr;
        UInt32 i=1;
        while ( i < dataSize )
        {
            tmpStr.Scan( data+i, dataSize-i );
            i += tmpStr.Length();

            if ( tmpStr.Length() > 0 )
            {
                streamNameList.push_back( tmpStr );
            }
            else
            {
                break;
            }
        }

        StreamListReceivedFromPeerEventData eData( &streamNameList );
        NotifyObservers( StreamListReceivedFromPeerEvent, &eData );
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerPeerListReceived( const char* data      ,
                                      const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the logic flow
    if ( m_isLinkOperationalForUs )
    {
        TStringList peerNameList;

        CORE::CString tmpStr;
        UInt32 i=1;
        while ( i < dataSize )
        {
            tmpStr.Scan( data+i, dataSize-i );
            i += tmpStr.Length();

            if ( tmpStr.Length() > 0 )
            {
                peerNameList.push_back( tmpStr );
            }
            else
            {
                break;
            }
        }

        PeerListReceivedFromPeerEventData eData( &peerNameList );
        NotifyObservers( PeerListReceivedFromPeerEvent, &eData );
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerDataGroupListReceived( const char* data      ,
                                           const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the logic flow
    if ( m_isLinkOperationalForUs )
    {
        TStringList dataGroupNameList;

        CORE::CString tmpStr;
        UInt32 i=1;
        while ( i < dataSize )
        {
            tmpStr.Scan( data+i, dataSize-i );
            i += tmpStr.Length();

            if ( tmpStr.Length() > 0 )
            {
                dataGroupNameList.push_back( tmpStr );
            }
            else
            {
                break;
            }
        }

        DataGroupListReceivedFromPeerEventData eData( &dataGroupNameList );
        NotifyObservers( DataGroupListReceivedFromPeerEvent, &eData );
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerDataGroupItemMutation( const char* data             ,
                                           const UInt32 dataSize        ,
                                           const TDataGroupDelta change )
{GUCEF_TRACE;

    // Sanity check on the logic flow
    if ( m_isLinkOperationalForUs )
    {
        // Sanity check on the data size
        if ( dataSize >= 9 )
        {
            UInt16 dataGroupID = *reinterpret_cast< const UInt16* >( data+1 );
            UInt16 itemIDSize = *reinterpret_cast< const UInt16* >( data+3 );

            // Sanity check on the item ID size
            if ( itemIDSize > 0 )
            {
                CORE::CDynamicBuffer itemID;
                CORE::CDynamicBuffer itemValue;

                itemID.LinkTo( data+5, itemIDSize );
                itemValue.LinkTo( data+5+itemIDSize, dataSize-itemIDSize-5 );

                // Try to find the data group, we should only get this type of
                // update if we are already subscribed to the given data group
                CDRNPeerLinkData::TDRNDataGroupPtr dataGroup = m_linkData->GetSubscribedDataGroupWithID( dataGroupID );
                if ( dataGroup )
                {
                    switch ( change )
                    {
                        case DATAGROUPDELTA_ADD :
                        {
                            // First check to make sure no item has already been added with the given ID
                            if ( dataGroup->HasItem( itemID ) )
                            {
                                // An item with the given ID already exists, we will delete
                                // the item first
                                dataGroup->RemoveItem( itemID );
                            }

                            // Add the item to the data group
                            dataGroup->SetItem( itemID    ,
                                                itemValue ,
                                                true      );
                            return;
                        }
                        case DATAGROUPDELTA_UPDATE :
                        {
                            // Overwrite or add the item to the data group
                            dataGroup->SetItem( itemID    ,
                                                itemValue ,
                                                true      );
                            return;
                        }
                        case DATAGROUPDELTA_REMOVE :
                        {
                            // Overwrite or add the item to the data group
                            dataGroup->RemoveItem( itemID );
                            return;
                        }
                        default :
                        {
                            // If we get here you forgot to add support for a new mutation type
                            GUCEF_ASSERT_ALWAYS;
                            return;
                        }
                    }
                }
            }
        }
    }

    // We should not get here
    NotifyObservers( LinkCorruptionEvent );

    // Terminate the link
    CloseLink();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerAuthenticationSuccess( void )
{GUCEF_TRACE;

    // We have successfully authenticated with the peer
    m_isAuthenticated = true;
    NotifyObservers( AuthenticationSuccessEvent );

    if ( !m_isLinkOperationalForPeer )
    {
        // Check if the link is now operational,..
        if ( !m_isPeerAuthenticated )
        {
            // Check if a validator mechanism has been provided to the node
            CIDRNPeerValidator* validator = m_parentNode->GetPeervalidator();
            if ( validator != NULL )
            {
                // A validator is available for use
                if ( validator->IsPeerLoginRequired( *this ) )
                {
                    // It seems the peer still has to authenticate with us
                    // The link is thus not operational yet
                    return;
                }
            }
        }

        // The link is now operational for the peer
        m_isLinkOperationalForPeer = true;
        NotifyObservers( LinkOperationalForPeerEvent );
        SendLinkOperationalMessage();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerAuthenticationFailed( void )
{GUCEF_TRACE;

    m_isAuthenticated = false;
    NotifyObservers( AuthenticationFailureEvent );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendAuthenticationSuccess( void )
{GUCEF_TRACE;

    // Compose the authentication successful message
    char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_AUTHENTICATION_SUCCESS, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 1;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 5, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendAuthenticationFailed( void )
{GUCEF_TRACE;

    // Compose the authentication failed message
    char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_AUTHENTICATION_FAILED, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 1;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 5, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendAuthentication( const CORE::CString& ourAccountName ,
                                  const CORE::CString& ourPassword    )
{GUCEF_TRACE;

    // Compose the authentication message
    CORE::CDynamicBuffer sendBuffer( ourAccountName.Length() + ourPassword.Length() + 6 , true );
    sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
    sendBuffer[ 3 ] = DRN_PEERCOMM_AUTHENTICATION;
    sendBuffer[ ourAccountName.Length() + ourPassword.Length() + 5 ] = DRN_TRANSMISSION_END;

    UInt16 payloadSize = (UInt16) ( 2 + ourAccountName.Length() + ourPassword.Length() );
    sendBuffer.CopyFrom( 1, 2, &payloadSize );
    if ( ourAccountName.Length() > 0 )
    {
        sendBuffer.CopyFrom( 4, ourAccountName.Length()+1, ourAccountName.C_String() );
    }
    else
    {
        sendBuffer[ 4 ] = '\0';
    }
    sendBuffer.CopyFrom( 5 + ourAccountName.Length(), ourPassword.Length(), ourPassword.C_String() );

    // Send the authentication message
    if ( !SendData( sendBuffer.GetConstBufferPtr()   ,
                    (UInt16)sendBuffer.GetDataSize() ,
                    false                            ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerAuthentication( const char* data      ,
                                    const UInt32 dataSize )
{GUCEF_TRACE;

    if ( !m_isLinkOperationalForPeer )
    {
        // Check if a validator mechanism has been provided to the node
        CIDRNPeerValidator* validator = m_parentNode->GetPeervalidator();
        if ( validator != NULL )
        {
            // A validator is available for use,..
            // Sanity check on the payload size
            if ( dataSize > 2 )
            {
                CORE::CString peerAccount;
                CORE::CString peerPassword;

                peerAccount.Scan( data+1, dataSize-1 );
                if ( peerAccount.Length() < dataSize-1 )
                {
                    peerPassword.Set( data+2+peerAccount.Length()     ,
                                      dataSize-2-peerAccount.Length() );
                }

                if ( validator->IsPeerLoginValid( *this        ,
                                                  peerAccount  ,
                                                  peerPassword ) )
                {
                    // The peer has successfully authenticated
                    m_isPeerAuthenticated = true;
                    SendAuthenticationSuccess();
                    NotifyObservers( PeerAuthenticationSuccessEvent );

                    // The link has now become operational for the peer
                    m_isLinkOperationalForPeer = true;
                    SendLinkOperationalMessage();
                    NotifyObservers( LinkOperationalForPeerEvent );
                    return;
                }
                else
                {
                    // Invalid authentication info has been provided by the peer
                    m_isPeerAuthenticated = false;
                    SendAuthenticationFailed();
                    NotifyObservers( PeerAuthenticationFailureEvent );
                    return;
                }
            }
        }
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerAuthenticationRequest( void )
{GUCEF_TRACE;

    if ( !m_isLinkOperationalForUs )
    {
        // Check if a validator mechanism has been provided to the node
        CIDRNPeerValidator* validator = m_parentNode->GetPeervalidator();
        if ( validator != NULL )
        {
            // A validator is available for use
            CORE::CString ourAccount;
            CORE::CString ourPassword;
            if ( validator->GetLoginForPeer( *this       ,
                                             ourAccount  ,
                                             ourPassword ) )
            {
                SendAuthentication( ourAccount  ,
                                    ourPassword );
                return;
            }
        }

        // This is a problem,.. we do not have any login info. This means we
        // cannot send any authentication even though the peer requires it.
        NotifyObservers( AuthenticationFailureEvent );

        // Terminate the link
        CloseLink();
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendNotAllowed( void )
{GUCEF_TRACE;

    // Compose the authentication failed message
    char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_NOT_ALLOWED, DRN_TRANSMISSION_END };
    UInt16 payloadSize = 1;
    memcpy( sendBuffer+1, &payloadSize, 2 );

    if ( !SendData( sendBuffer, 5, false ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendSubscribedToDataGroup( const CORE::CString& groupName ,
										 const UInt16 id                )
{GUCEF_TRACE;

	// Compose the message
    CORE::CDynamicBuffer sendBuffer( groupName.Length() + 7, true );
    sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
    sendBuffer[ 3 ] = DRN_PEERCOMM_SUBSCRIBED_TO_DATAGROUP;
    sendBuffer[ groupName.Length() + 6 ] = DRN_TRANSMISSION_END;
	sendBuffer.CopyFrom( 6, groupName.Length(), groupName.C_String() );
    sendBuffer.CopyFrom( 4, 2, &id );

    UInt16 payloadSize = (UInt16) ( 3 + groupName.Length() );
    sendBuffer.CopyFrom( 1, 2, &payloadSize );

    // Send the authentication message
    if ( !SendData( sendBuffer.GetConstBufferPtr()   ,
                    (UInt16)sendBuffer.GetDataSize() ,
                    false                            ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendSubscribedToDataStream( const CORE::CString& streamName ,
										  const UInt16 id                 )
{GUCEF_TRACE;

	// Compose the message
    CORE::CDynamicBuffer sendBuffer( streamName.Length() + 7, true );
    sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
    sendBuffer[ 3 ] = DRN_PEERCOMM_SUBSCRIBED_TO_DATASTREAM;
    sendBuffer[ streamName.Length() + 6 ] = DRN_TRANSMISSION_END;
	sendBuffer.CopyFrom( 6, streamName.Length(), streamName.C_String() );
    sendBuffer.CopyFrom( 4, 2, &id );

    UInt16 payloadSize = (UInt16) ( 3 + streamName.Length() );
    sendBuffer.CopyFrom( 1, 2, &payloadSize );

    // Send the authentication message
    if ( !SendData( sendBuffer.GetConstBufferPtr()   ,
                    (UInt16)sendBuffer.GetDataSize() ,
                    false                            ) )
    {
        // Failed to send data, something is very wrong
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnSubscribedToPeerDataGroup( const char* data      ,
                                           const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the payload size
    if ( dataSize > 3 )
    {
        // Extract the data group ID
        UInt16 dataGroupID = 0;
        memcpy( &dataGroupID, data+1, 2 );

        // Extract the data group name
        CORE::CString dataGroupName;
        dataGroupName.Scan( data+3, dataSize-3 );

        if ( dataGroupName.Length() > 0 )
        {
            // Apply the subscription in the administration
            TDRNDataGroupPtr dataGroup = m_linkData->AddSubscribedDataGroup( dataGroupName ,
                                                                             dataGroupID   );

            SubscribedToDataGroupEventData eData( dataGroup );
            NotifyObservers( SubscribedToDataGroupEvent, &eData );
        }
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnSubscribedToPeerDataStream( const char* data      ,
                                            const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the payload size
    if ( dataSize > 3 )
    {
        // Extract the data group ID
        UInt16 dataStreamID = 0;
        memcpy( &dataStreamID, data+1, 2 );

        // Extract the data group name
        CORE::CString dataStreamName;
        dataStreamName.Scan( data+3, dataSize-3 );

        if ( dataStreamName.Length() > 0 )
        {
            // Apply the subscription in the administration
            TDRNDataStreamPtr dataStream = m_linkData->AddSubscribedDataStream( dataStreamName ,
                                                                                dataStreamID   );

            SubscribedToDataStreamEventData eData( dataStream );
            NotifyObservers( SubscribedToDataStreamEvent, &eData );
        }
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerSubscribeToDataGroupRequest( const char* data      ,
                                                 const UInt32 dataSize )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForPeer )
    {
        // Sanity check on the payload
        if ( dataSize > 2 )
        {
            // First we extract a name
            CORE::CString dataGroupName;
            dataGroupName.Scan( data+1, dataSize-1 );

            // Sanity check on the name
            if ( dataGroupName.Length() > 0 )
            {
                // Now we try to find the requested item
                CDRNPeerLinkData::TDRNDataGroupPtr dataPtr = m_linkData->GetPublicizedDataGroupWithName( dataGroupName );
                if ( !dataPtr.IsNULL() )
                {
                    // Subscribe the peer to the data group
					SubscribeTo( &(*dataPtr), CDRNDataGroup::ItemChangedEvent );

					// Notify the peer that the action has been successfully completed
					// We also give the peer an ID by which he/she will reference the
					// data group in the future
					UInt16 id;
					m_linkData->GetPublicizedDataGroupID( dataGroupName, id );
					SendSubscribedToDataGroup( dataGroupName, id );

					// Send all the current data group values to the peer so the remote
					// and local data groups are in-sync. This initial dispatch will be send
					// in a reliable fashion regardless of the settings
                    const CORE::CDynamicBuffer* itemID = NULL;
                    const CORE::CDynamicBuffer* itemData = NULL;
                    UInt32 items = dataPtr->GetItemCount();

                    for ( UInt32 i=0; i<items; ++i )
                    {
                        if ( dataPtr->GetIDAndDataAtIndex( i         ,
                                                           &itemID   ,
                                                           &itemData ) )
                        {
                            SendDataGroupItemUpdateToPeer( id                 ,
                                                           *itemID            ,
                                                           *itemData          ,
                                                           false              ,
                                                           DATAGROUPDELTA_ADD );
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Tell the peer his action is not allowed at this time
        SendNotAllowed();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerSubscribeToStreamRequest( const char* data      ,
                                              const UInt32 dataSize )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForPeer )
    {
        // Sanity check on the payload
        if ( dataSize > 2 )
        {
            // First we extract a name
            CORE::CString dataStreamName;
            dataStreamName.Scan( data+1, dataSize-1 );

            // Sanity check on the name
            if ( dataStreamName.Length() > 0 )
            {
                // Now we try to find the requested item
                CDRNPeerLinkData::TDRNDataStreamPtr dataPtr = m_linkData->GetPublicizedDataStreamWithName( dataStreamName );
                if ( !dataPtr.IsNULL() )
                {
                    // Subscribe the peer to the data stream
                    SubscribeTo( &(*dataPtr), CDRNDataStream::DataTransmittedEvent );

					// Notify the peer that the action has been successfully completed
					// We also give the peer an ID by which he/she will reference the
					// data group in the future
					UInt16 id;
					m_linkData->GetPublicizedDataStreamID( dataStreamName, id );
					SendSubscribedToDataStream( dataStreamName, id );
                }
            }
        }
    }
    else
    {
        // Tell the peer his action is not allowed at this time
        SendNotAllowed();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerGreeting( const char* data      ,
                              const UInt32 dataSize )
{GUCEF_TRACE;

    // First we perform a sanity check on the data size since the
    // greeting has a fixed transmission length
    if ( dataSize == 4 )
    {
        // data size checks out,.. we now handle the payload
        char drnProtocolMayorVersion = data[ 1 ];
        char drnProtocolMinorVersion = data[ 2 ];
        char drnProtocolPatchVersion = data[ 3 ];

        if ( ( drnProtocolMayorVersion == DRN_PROTOCOL_MAYOR_VERION ) &&
             ( drnProtocolMinorVersion == DRN_PROTOCOL_MINOR_VERION ) &&
             ( drnProtocolPatchVersion == DRN_PROTOCOL_PATCH_VERION )  )
        {
            // We now know we have a peer that is using a compatible
            // DRN protocol version.
            NotifyObservers( LinkProtocolMatchEvent );
            SendServiceTypeMessage();
            return;
        }
        else
        {
            // The peer is using a different version of the DRN protocol.
            NotifyObservers( LinkProtocolMismatchEvent );
            NotifyObservers( LinkIncompatibleEvent );

            // Notify the peer that we consider the link to be incompatible
            SendIncompatibleLinkMessage();

            // Terminate the link
            CloseLink();
            return;
        }
    }
    else
    {
        // We should not get here
        NotifyObservers( LinkCorruptionEvent );

        // Terminate the link
        CloseLink();
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerIllegalRequest( void )
{GUCEF_TRACE;

    // Simply inform the observers of what has happened
    NotifyObservers( IllegalRequestEvent );
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerStreamDataReceived( const char* data      ,
                                        const UInt32 dataSize )
{GUCEF_TRACE;

    // Sanity check on the program flow logic: the link has to be operational
    // before you can subscribe thus also in order for us to receive stream data
    if ( m_isLinkOperationalForUs )
    {
        // Sanity check on the payload size
        if ( dataSize > 3 )
        {
            // Extract the stream ID
            UInt16 streamID = *reinterpret_cast< const UInt16* >( data+1 );

            // Try to find a stream with the given ID among the stream subscriptions
            TDRNDataStreamPtr dataStream = m_linkData->GetSubscribedDataStreamWithID( streamID );
            if ( dataStream )
            {
                // Pass the data along to the stream's handler
                dataStream->OnDataReceived( data+3     ,
                                            dataSize-3 );
                return;
            }
        }
    }

    // Tell the peer his action is not allowed at this time
    SendNotAllowed();
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnPeerDataReceived( const char* data      ,
                                  const UInt32 dataSize )
{GUCEF_TRACE;

    switch ( data[ 0 ] )
    {
        case DRN_PEERCOMM_GREETING :
        {
            OnPeerGreeting( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_SERVICE :
        {
            OnPeerServiceType( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_INCOMPATIBLE_LINK :
        {
            OnPeerLinkIncompatible();
            return;
        }
        case DRN_PEERCOMM_AUTHENTICATION :
        {
            OnPeerAuthentication( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_AUTHENTICATION_REQUIRED :
        {
            OnPeerAuthenticationRequest();
            return;
        }
        case DRN_PEERCOMM_AUTHENTICATION_FAILED :
        {
            OnPeerAuthenticationFailed();
            return;
        }
        case DRN_PEERCOMM_AUTHENTICATION_SUCCESS :
        {
            OnPeerAuthenticationSuccess();
            return;
        }
        case DRN_PEERCOMM_LINK_OPERATIONAL :
        {
            OnPeerLinkOperational();
            return;
        }
        case DRN_PEERCOMM_DATAGROUP_ITEM_ADDED :
        {
            OnPeerDataGroupItemMutation( data, dataSize, DATAGROUPDELTA_ADD );
            return;
        }
        case DRN_PEERCOMM_DATAGROUP_ITEM_UPDATE :
        {
            OnPeerDataGroupItemMutation( data, dataSize, DATAGROUPDELTA_UPDATE );
            return;
        }
        case DRN_PEERCOMM_DATAGROUP_ITEM_REMOVED :
        {
            OnPeerDataGroupItemMutation( data, dataSize, DATAGROUPDELTA_REMOVE );
            return;
        }
        case DRN_PEERCOMM_STREAM_DATA :
        {
            OnPeerStreamDataReceived( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_NOT_ALLOWED :
        {
            OnPeerIllegalRequest();
            return;
        }
        case DRN_PEERCOMM_PEERLIST_REQUEST :
        {
            OnPeerPeerListRequest();
            return;
        }
        case DRN_PEERCOMM_PEERLIST :
        {
            OnPeerPeerListReceived( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_STREAMLIST_REQUEST :
        {
            OnPeerStreamListRequest();
            return;
        }
        case DRN_PEERCOMM_STREAMLIST :
        {
            OnPeerStreamListReceived( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_DATAGROUPLIST_REQUEST :
        {
            OnPeerDataGroupRequest();
            return;
        }
        case DRN_PEERCOMM_DATAGROUPLIST :
        {
            OnPeerDataGroupListReceived( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_SUBSCRIBE_TO_DATAGROUP_REQUEST :
        {
            OnPeerSubscribeToDataGroupRequest( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_SUBSCRIBE_TO_STREAM_REQUEST :
        {
            OnPeerSubscribeToStreamRequest( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_SUBSCRIBED_TO_DATAGROUP :
        {
            OnSubscribedToPeerDataGroup( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_SUBSCRIBED_TO_DATASTREAM :
        {
            OnSubscribedToPeerDataStream( data, dataSize );
            return;
        }
        case DRN_PEERCOMM_CONNECTBACKINFO :
        {
            OnPeerConnectBackInfo( data, dataSize );
            return;
        }
        default:
        {
            // If we get here then an unexpected value was found as the command
            // This is not something that should happen or something that is allowed
            NotifyObservers( LinkCorruptionEvent );

            // Terminate the link
            CloseLink();
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnUDPChannelEvent( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventid  ,
                                 CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    if ( COMCORE::CUDPChannel::UDPPacketReceivedEvent == eventid )
    {
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnTCPDataReceived( const GUCEF::CORE::CDynamicBuffer& buffer )
{GUCEF_TRACE;

    // When using TCP transmissions can get concatenated. We have to separate these
    // transmissions ourselves using the delimiters specified in our DRN protocol.
    UInt32 dataSize = buffer.GetDataSize();
    UInt32 i = m_tcpStreamKeepBytes;
    if ( i < dataSize )
    {
        if ( buffer[ i ] == DRN_TRANSMISSION_START )
        {
            // Check if the data we received is a segmented transmission
            if ( i != 0 )
            {
                // perform an end delimiter sanity check on the previous transmission
                if ( buffer[ i-1 ] == DRN_TRANSMISSION_END )
                {
                    // Append the received prefix to what we have in our buffer
                    // the data segment belongs to an earlier transmission.
                    m_tcpStreamKeepBytes = 0;
                    m_tcpStreamBuffer.Append( buffer.GetConstBufferPtr() ,
                                              i                          ,
                                              true                       );

                    // We now have a complete transmission in our stream buffer
                    // We will now process the buffer
                    OnPeerDataReceived( static_cast< const char* >( m_tcpStreamBuffer.GetConstBufferPtr() ) ,
                                        m_tcpStreamBuffer.GetDataSize()                                     );

                    // We must now empty the buffer to make room for a new segmented message
                    // We only perform a logical clear to avoid unnecessary reallocations
                    m_tcpStreamBuffer.Clear( true );
                }
                else
                {
                    // We should not get here, the transmission has been corrupted somehow
                    // Even though we have all the data needed we did not find the end
                    // delimiter as expected.
                    NotifyObservers( LinkCorruptionEvent );

                    // Terminate the link
                    CloseLink();
                    return;
                }
            }

            // Sanity check on the buffer size versus payload info offset
            if ( i+2 < dataSize )
            {
                // Get the transmission size
                UInt16 transmissionSize = buffer.AsConstType< UInt16 >( i+1 );

                // Sanity check on the buffer size versus reported payload size
                if ( i+2+transmissionSize < dataSize )
                {
                    // Additional sanity check on the reported transmission size
                    if ( buffer[ i+3+transmissionSize ] == DRN_TRANSMISSION_END )
                    {
                        // The payload segment seems to be all here and the size checks
                        // out. We can now proceed with processing the transmission.
                        OnPeerDataReceived( static_cast< const char* >( buffer.GetConstBufferPtr() )+i+3 ,
                                            transmissionSize                                             );

                        // We must now check to see if there is any data left to be processed.
                        if ( i+4+transmissionSize < dataSize )
                        {
                            // We have more data, it seems multiple transmissions have been
                            // concatenated into a single transmission.
                            GUCEF::CORE::CDynamicBuffer subBuffer;
                            subBuffer.LinkTo( static_cast< const char* >( buffer.GetConstBufferPtr() )+(i+4+transmissionSize) ,
                                              dataSize - (i+4+transmissionSize)                                               );
                            OnTCPDataReceived( subBuffer );
                        }
                    }
                    else
                    {
                        // We should not get here, the transmission has been corrupted somehow
                        // Even though we have all the data needed we did not find the end
                        // delimiter as expected.
                        NotifyObservers( LinkCorruptionEvent );

                        // Terminate the link
                        CloseLink();
                        return;
                    }
                }
                else
                {
                    // We do not have all the data needed to process the transmission
                    // We will place the data we have in the stream buffer. The data can
                    // then be processed when we receive the remainder of the transmission
                    m_tcpStreamKeepBytes = transmissionSize - ( dataSize-i );
                    m_tcpStreamBuffer.Append( buffer.GetConstBufferPtr( i ) ,
                                              dataSize-i                    ,
                                              true                          );
                }
            }
            else
            {
                // if the stream split happened here we are screwed
                // @todo come up with a solution for this
                assert( false );
            }
        }
    }
    else
    {
        // The entire transmission we received is part of a bigger transmission
        // We will append the received data to our stream buffer
        m_tcpStreamKeepBytes -= dataSize;
        m_tcpStreamBuffer.Append( buffer.GetConstBufferPtr() ,
                                  dataSize                   ,
                                  true                       );
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnTCPConnectionEvent( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventid  ,
                                    CORE::CICloneable* eventdata )
{GUCEF_TRACE;


    if ( GUCEF::COMCORE::CTCPConnection::DataRecievedEvent == eventid )
    {
        // Prepare access to the data buffer
        GUCEF::CORE::TLinkedCloneableBuffer* bufferLink = static_cast< GUCEF::CORE::TLinkedCloneableBuffer* >( eventdata );
        const GUCEF::CORE::CDynamicBuffer& buffer = bufferLink->GetData();

        // Call the event handler
        OnTCPDataReceived( buffer );
        return;
    }
    else
    if ( GUCEF::COMCORE::CTCPConnection::DataSentEvent == eventid )
    {
        // No need to do anything
        return;
    }
    else
    if ( GUCEF::COMCORE::CTCPConnection::SocketErrorEvent == eventid )
    {
        NotifyObservers( SocketErrorEvent );
        return;
    }
    else
    if ( GUCEF::COMCORE::CTCPConnection::DisconnectedEvent == eventid )
    {
        NotifyObservers( DisconnectedEvent );
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventid               ,
                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // Call base-class implementation,.. mandatory
    CObservingNotifier::OnNotify( notifier  ,
                                  eventid   ,
                                  eventdata );

    if ( notifier == m_tcpConnection )
    {
        OnTCPConnectionEvent( notifier  ,
                              eventid   ,
                              eventdata );
    }
    else
    if ( notifier == m_udpChannel )
    {
        OnUDPChannelEvent( notifier  ,
                           eventid   ,
                           eventdata );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::RequestPeerList( void )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Send a peer-list-request to the given peer node
        char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_PEERLIST_REQUEST, DRN_TRANSMISSION_END };
        UInt16 payloadSize = 1;
        memcpy( sendBuffer+1, &payloadSize, 2 );

        return SendData( sendBuffer, 5, false );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::RequestStreamList( void )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Send a stream-list-request to the given peer node
        char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_STREAMLIST_REQUEST, DRN_TRANSMISSION_END };
        UInt16 payloadSize = 1;
        memcpy( sendBuffer+1, &payloadSize, 2 );

        return SendData( sendBuffer, 5, false );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::RequestDataGroupList( void )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Send a datagroup-list-request to the given peer node
        char sendBuffer[ 5 ] = { DRN_TRANSMISSION_START, 0, 0, DRN_PEERCOMM_DATAGROUPLIST_REQUEST, DRN_TRANSMISSION_END };
        UInt16 payloadSize = 1;
        memcpy( sendBuffer+1, &payloadSize, 2 );

        return SendData( sendBuffer, 5, false );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::RequestDataGroupSubscription( const CORE::CString& dataGroupName )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Compose the message
        CORE::CDynamicBuffer sendBuffer( dataGroupName.Length() + 5, true );
        sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
        sendBuffer[ 3 ] = DRN_PEERCOMM_SUBSCRIBE_TO_DATAGROUP_REQUEST;
        sendBuffer[ dataGroupName.Length() + 4 ] = DRN_TRANSMISSION_END;
        sendBuffer.CopyFrom( 4, dataGroupName.Length(), dataGroupName.C_String() );

        UInt16 payloadSize = (UInt16) dataGroupName.Length()+1;
        sendBuffer.CopyFrom( 1, 2, &payloadSize );

        // Send the subscription request
        if ( SendData( sendBuffer.GetConstBufferPtr()   ,
                       (UInt16)sendBuffer.GetDataSize() ,
                       false                            ) )
        {
            return true;
        }

        // Failed to send data, something is very wrong
        CloseLink();
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNPeerLink::RequestStreamSubscription( const CORE::CString& dataStreamName )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Compose the message
        CORE::CDynamicBuffer sendBuffer( dataStreamName.Length() + 5, true );
        sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
        sendBuffer[ 3 ] = DRN_PEERCOMM_SUBSCRIBE_TO_STREAM_REQUEST;
        sendBuffer[ dataStreamName.Length() + 4 ] = DRN_TRANSMISSION_END;
        sendBuffer.CopyFrom( 4, dataStreamName.Length(), dataStreamName.C_String() );

        UInt16 payloadSize = (UInt16) dataStreamName.Length()+1;
        sendBuffer.CopyFrom( 1, 2, &payloadSize );

        // Send the subscription request
        if ( SendData( sendBuffer.GetConstBufferPtr()   ,
                       (UInt16)sendBuffer.GetDataSize() ,
                       false                            ) )
        {
            return true;
        }

        // Failed to send data, something is very wrong
        CloseLink();
        return false;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CDRNNode&
CDRNPeerLink::GetParentNode( void )
{GUCEF_TRACE;

    return *m_parentNode;
}

/*-------------------------------------------------------------------------*/

CDRNPeerLinkData&
CDRNPeerLink::GetLinkData( void )
{GUCEF_TRACE;

    return *m_linkData;
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendStreamDataToPeer( const UInt16 id                        ,
                                    const CORE::CDynamicBuffer& data       ,
                                    const bool allowUnreliableTransmission )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Sanity check on the data, no need to send anything if there
        // is nothing to send
        if ( data.GetDataSize() > 0 )
        {
            // Compose the message
            CORE::CDynamicBuffer sendBuffer( data.GetDataSize() + 7, true );
            sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
            sendBuffer[ 3 ] = DRN_PEERCOMM_STREAM_DATA;
            sendBuffer[ data.GetDataSize() + 6 ] = DRN_TRANSMISSION_END;
            sendBuffer.CopyFrom( 4, 2, &id );
            sendBuffer.CopyFrom( 6, data.GetDataSize(), data.GetConstBufferPtr() );

            UInt16 payloadSize = (UInt16) data.GetDataSize()+3;
            sendBuffer.CopyFrom( 1, 2, &payloadSize );

            // Send the stream data
            if ( SendData( sendBuffer.GetConstBufferPtr()   ,
                           (UInt16)sendBuffer.GetDataSize() ,
                           allowUnreliableTransmission      ) )
            {
                return;
            }

            // Failed to send data, something is very wrong
            CloseLink();
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendDataGroupItemUpdateToPeer( const UInt16 id                        ,
                                             const CORE::CDynamicBuffer& itemID     ,
                                             const CORE::CDynamicBuffer& itemValue  ,
                                             const bool allowUnreliableTransmission ,
                                             const TDataGroupDelta deltaChange      )
{GUCEF_TRACE;

    if ( m_isLinkOperationalForUs )
    {
        // Cache some values
        UInt16 itemIDSize = (UInt16) itemID.GetDataSize();
        UInt32 itemValueSize = itemValue.GetDataSize();

        // Sanity check on the data, we do not send anything if there is nothing to send
        // Note that an empty itemValue is allowed
        if ( itemIDSize > 0 )
        {
            // Translate the mutation type into a protocol message
            char protocolCmd = 0;
            switch ( deltaChange )
            {
                case DATAGROUPDELTA_ADD :
                {
                    protocolCmd = DRN_PEERCOMM_DATAGROUP_ITEM_ADDED;
                    break;
                }
                case DATAGROUPDELTA_UPDATE :
                {
                    protocolCmd = DRN_PEERCOMM_DATAGROUP_ITEM_UPDATE;
                    break;
                }
                case DATAGROUPDELTA_REMOVE :
                {
                    protocolCmd = DRN_PEERCOMM_DATAGROUP_ITEM_REMOVED;
                    break;
                }
                default :
                {
                    // If we get here you forgot to add support for a new mutation type
                    GUCEF_ASSERT_ALWAYS;
                    return;
                }
            }

            // Compose the message
            CORE::CDynamicBuffer sendBuffer( itemIDSize + itemValueSize + 9, true );
            sendBuffer[ 0 ] = DRN_TRANSMISSION_START;
            sendBuffer[ 3 ] = protocolCmd;
            sendBuffer[ itemIDSize + itemValueSize + 8 ] = DRN_TRANSMISSION_END;
            sendBuffer.CopyFrom( 4, 2, &id );
            sendBuffer.CopyFrom( 6, 2, &itemIDSize );
            sendBuffer.CopyFrom( 8, itemIDSize, itemID.GetConstBufferPtr() );
            sendBuffer.CopyFrom( 8 + itemIDSize, itemValueSize, itemValue.GetConstBufferPtr() );

            UInt16 payloadSize = (UInt16) ( itemIDSize + itemValueSize + 5 );
            sendBuffer.CopyFrom( 1, 2, &payloadSize );

            // Send the data group item data
            if ( SendData( sendBuffer.GetConstBufferPtr()   ,
                           (UInt16)sendBuffer.GetDataSize() ,
                           allowUnreliableTransmission      ) )
            {
                return;
            }

            // Failed to send data, something is very wrong
            CloseLink();
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CDRNPeerLink::SendDataGroupItemUpdateToPeer( const CDRNDataGroup& dataGroup        ,
                                             const UInt16 id                       ,
                                             const CORE::CDynamicBuffer& itemID    ,
                                             const CORE::CDynamicBuffer& itemValue ,
                                             const TDataGroupDelta deltaChange     )
{GUCEF_TRACE;

    CDRNDataGroup::CDRNDataGroupPropertiesPtr settings = dataGroup.GetGroupProperties();
    bool allowUnreliableTransmission = settings->GetAllowUnreliableTransmission();

    if ( settings->GetEmitEntireGroupOnChange() )
    {
        const CORE::CDynamicBuffer* itemIDValue = NULL;
        const CORE::CDynamicBuffer* itemDataValue = NULL;
        UInt32 items = dataGroup.GetItemCount();

        for ( UInt32 i=0; i<items; ++i )
        {
            if ( dataGroup.GetIDAndDataAtIndex( i              ,
                                                &itemIDValue   ,
                                                &itemDataValue ) )
            {
                TDataGroupDelta change = *itemIDValue == itemID ? deltaChange : DATAGROUPDELTA_UPDATE;
                SendDataGroupItemUpdateToPeer( id                          ,
                                               *itemIDValue                ,
                                               *itemDataValue              ,
                                               allowUnreliableTransmission ,
                                               change                      );
            }
        }
    }
    else
    {
        SendDataGroupItemUpdateToPeer( id                          ,
                                       itemID                      ,
                                       itemValue                   ,
                                       allowUnreliableTransmission ,
                                       deltaChange                 );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
