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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_DRN_CDRNTCPCHANNEL_H
#include "gucefDRN_CDRNTcpChannel.h"
#define GUCEF_DRN_CDRNTCPCHANNEL_H
#endif /* GUCEF_DRN_CDRNTCPCHANNEL_H ? */

#include "gucefDRN_CDRNTcpChannelListener.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNTcpChannelListener::CDRNTcpChannelListener( void )
    : CIDRNChannelListener()
    , m_serverSocket( GUCEF_NULL )
    , m_pulseGenerator()
    , m_listenEndpoint()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNTcpChannelListener::~CDRNTcpChannelListener()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    StopListening();
    GUCEF_DELETE m_serverSocket;
    m_serverSocket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannelListener::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNTcpChannelListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTcpChannelListener::Listen( const CDRNEndpoint& local )
{GUCEF_TRACE;

    StopListening();

    UInt16 port = local.GetPort();
    if ( 0 == port )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTcpChannelListener: The listen endpoint needs a port: " << local.ToString() );
        return false;
    }

    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    if ( GUCEF_NULL == m_serverSocket )
    {
        UInt32 maxConnections = local.GetParams().GetValueAlways( "maxConnections", CORE::CVariant( (UInt32) 256 ) ).AsUInt32( 256 );
        m_serverSocket = GUCEF_NEW COMCORE::CTCPServerSocket( m_pulseGenerator, false, maxConnections );
        SubscribeTo( m_serverSocket );
    }

    m_listenEndpoint = local;
    if ( !m_serverSocket->ListenOnPort( port ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTcpChannelListener: Failed to listen on " << local.ToString() );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNTcpChannelListener: Listening on " << local.ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannelListener::StopListening( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_serverSocket && m_serverSocket->IsActive() )
        m_serverSocket->Close();
}

/*-------------------------------------------------------------------------*/

bool
CDRNTcpChannelListener::IsListening( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_serverSocket && m_serverSocket->IsActive();
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNTcpChannelListener::GetListenEndpoint( void ) const
{GUCEF_TRACE;

    return m_listenEndpoint;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNTcpChannelListener::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeTcp;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNTcpChannelListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNTcpChannelListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNTcpChannelListener::OnNotify( CORE::CNotifier* notifier                 ,
                                  const CORE::CEvent& eventid               ,
                                  CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( notifier != m_serverSocket || GUCEF_NULL == m_serverSocket )
        return;

    if ( COMCORE::CTCPServerSocket::ClientConnectedEvent == eventid )
    {
        if ( GUCEF_NULL == eventdata )
            return;
        COMCORE::CTCPServerSocket::TConnectionInfo& info = static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventdata )->GetData();
        if ( GUCEF_NULL == info.connection )
            return;

        CIDRNChannelPtr channel( GUCEF_NEW CDRNTcpChannel( info.connection, m_pulseGenerator ) );
        CDRNChannelEventData channelData( channel );
        NotifyObservers( ChannelAcceptedEvent, &channelData );
    }
    else
    if ( COMCORE::CTCPServerSocket::ServerSocketErrorEvent == eventid )
    {
        CDRNChannelErrorEventData errorData( "server socket error" );
        NotifyObservers( ListenerErrorEvent, &errorData );
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
