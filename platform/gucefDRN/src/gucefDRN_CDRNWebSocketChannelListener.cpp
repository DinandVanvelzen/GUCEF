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

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#include "gucefWEB_CWebSocketServer.h"
#define GUCEF_WEB_CWEBSOCKETSERVER_H
#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#include "gucefDRN_CDRNWebSocketChannel.h"
#define GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H
#endif /* GUCEF_DRN_CDRNWEBSOCKETCHANNEL_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

#include "gucefDRN_CDRNWebSocketChannelListener.h"

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

CDRNWebSocketChannelListener::CDRNWebSocketChannelListener( void )
    : CIDRNChannelListener()
    , m_httpServer( GUCEF_NULL )
    , m_wsServer( GUCEF_NULL )
    , m_pulseGenerator()
    , m_listenEndpoint()
    , m_channels()
    , m_listening( false )
    , m_newConnectionMaxMessageSize( DRN_MAX_STREAM_PACKET_SIZE )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNWebSocketChannelListener::~CDRNWebSocketChannelListener()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    StopListening();

    TChannelSet channels;
    channels.swap( m_channels );
    TChannelSet::iterator i = channels.begin();
    while ( i != channels.end() )
    {
        (*i)->OnListenerGone();
        ++i;
    }

    GUCEF_DELETE m_wsServer;
    m_wsServer = GUCEF_NULL;
    GUCEF_DELETE m_httpServer;
    m_httpServer = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannelListener::SetPulseGenerator( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    m_pulseGenerator = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CDRNWebSocketChannelListener::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannelListener::ApplySecurityPolicy( const CDRNSecurityPolicy& policy )
{GUCEF_TRACE;

    // a connection takes small messages until its peer authenticated, the link raises the limit then
    UInt32 preAuthSize = policy.limits.maxPreAuthPacketSize;
    m_newConnectionMaxMessageSize = 0 != preAuthSize && preAuthSize < DRN_MAX_STREAM_PACKET_SIZE ? preAuthSize : DRN_MAX_STREAM_PACKET_SIZE;
    if ( GUCEF_NULL != m_wsServer )
        m_wsServer->SetMaxMessageSize( m_newConnectionMaxMessageSize );
}

/*-------------------------------------------------------------------------*/

bool
CDRNWebSocketChannelListener::Listen( const CDRNEndpoint& local )
{GUCEF_TRACE;

    StopListening();

    UInt16 port = local.GetPort();
    if ( 0 == port )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNWebSocketChannelListener: The listen endpoint needs a port: " << local.ToString() );
        return false;
    }

    if ( m_pulseGenerator.IsNULL() )
        m_pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    if ( GUCEF_NULL == m_httpServer )
    {
        WEB::CHttpServerSettings settings;
        settings.allowWebSocketUpgrades = true;
        settings.processRequestsAsync = false;
        settings.keepAliveConnections = true;
        settings.maxClientConnections = local.GetParams().GetValueAlways( "maxConnections", CORE::CVariant( (UInt32) 256 ) ).AsUInt32( 256 );
        m_httpServer = GUCEF_NEW WEB::CHTTPServer( m_pulseGenerator, GUCEF_NULL, &settings );
        m_wsServer = GUCEF_NEW WEB::CWebSocketServer( *m_httpServer );
        m_wsServer->SetMaxMessageSize( m_newConnectionMaxMessageSize );
        SubscribeTo( m_wsServer );
    }

    m_listenEndpoint = local;
    m_listening = m_httpServer->ListenOnPort( port );
    if ( !m_listening )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNWebSocketChannelListener: Failed to listen on " << local.ToString() );
        return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNWebSocketChannelListener: Listening on " << local.ToString() );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannelListener::StopListening( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_httpServer && m_listening )
        m_httpServer->Close();
    m_listening = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNWebSocketChannelListener::IsListening( void ) const
{GUCEF_TRACE;

    return m_listening;
}

/*-------------------------------------------------------------------------*/

CDRNEndpoint
CDRNWebSocketChannelListener::GetListenEndpoint( void ) const
{GUCEF_TRACE;

    return m_listenEndpoint;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNWebSocketChannelListener::GetChannelType( void ) const
{GUCEF_TRACE;

    return CDRNEndpoint::ChannelTypeWebSocket;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNWebSocketChannelListener::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNWebSocketChannelListener";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannelListener::OnChannelGone( CDRNWebSocketChannel* channel )
{GUCEF_TRACE;

    m_channels.erase( channel );
}

/*-------------------------------------------------------------------------*/

void
CDRNWebSocketChannelListener::OnNotify( CORE::CNotifier* notifier                 ,
                                        const CORE::CEvent& eventid               ,
                                        CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_wsServer || notifier != m_wsServer || GUCEF_NULL == eventdata )
        return;

    if ( WEB::CWebSocketServer::BinaryMessageEvent == eventid )
    {
        WEB::SWebSocketServerBinaryMessage& message = static_cast< WEB::TWebSocketServerBinaryMessageEventData* >( eventdata )->GetData();
        if ( message.connection.IsNULL() )
            return;
        CDRNWebSocketChannel* channel = static_cast< CDRNWebSocketChannel* >( message.connection->GetUserData() );
        if ( GUCEF_NULL != channel )
            channel->OnPacket( message.data );
    }
    else
    if ( WEB::CWebSocketServer::ClientConnectedEvent == eventid )
    {
        const WEB::TWebSocketConnectionPtr& connection = static_cast< WEB::TWebSocketServerConnectedEventData* >( eventdata )->GetData();
        if ( connection.IsNULL() )
            return;

        CDRNWebSocketChannel* newChannel = GUCEF_NEW CDRNWebSocketChannel( this, connection, m_pulseGenerator );
        m_channels.insert( newChannel );
        CIDRNChannelPtr channel( newChannel );
        CDRNChannelEventData channelData( channel );
        NotifyObservers( ChannelAcceptedEvent, &channelData );
    }
    else
    if ( WEB::CWebSocketServer::ClientDisconnectedEvent == eventid )
    {
        WEB::SWebSocketServerClosedInfo& info = static_cast< WEB::TWebSocketServerClosedEventData* >( eventdata )->GetData();
        if ( info.connection.IsNULL() )
            return;
        CDRNWebSocketChannel* channel = static_cast< CDRNWebSocketChannel* >( info.connection->GetUserData() );
        if ( GUCEF_NULL != channel )
            channel->OnRemoteClosed();
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
