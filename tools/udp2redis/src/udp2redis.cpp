/*
 *  Udp2Redis: service which pushes UDP packets into Redis
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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#include "udp2redis.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    #include <winsock2.h>
#endif

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_QUEUE_SEND_DEFAULT_BATCH_SIZE                 25

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Udp2RedisChannel::Udp2RedisChannel()
    : CORE::CTaskConsumer()
    , m_channelSettings()
    , m_redisReconnectTimer( GUCEF_NULL )
    , m_redisContext( GUCEF_NULL )
    , m_udpSocket( GUCEF_NULL )
    , m_redisMsgQueueOverflowQueue()
    , m_redisOptions()
    , m_redisReadFlag( false )
    , m_redisWriteFlag( false )
    , m_redisTimeoutFlag( false )
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_redisErrorReplies( 0 )

{GUCEF_TRACE;

    memset( &m_redisOptions, 0, sizeof(m_redisOptions) );
}

/*-------------------------------------------------------------------------*/

Udp2RedisChannel::Udp2RedisChannel( const Udp2RedisChannel& src )
    : CORE::CTaskConsumer()
    , m_channelSettings( src.m_channelSettings )
    , m_redisReconnectTimer( GUCEF_NULL )
    , m_redisContext( src.m_redisContext )
    , m_udpSocket( GUCEF_NULL )
    , m_redisMsgQueueOverflowQueue( src.m_redisMsgQueueOverflowQueue )
    , m_redisOptions( src.m_redisOptions )
    , m_redisReadFlag( src.m_redisReadFlag )
    , m_redisWriteFlag( src.m_redisWriteFlag )
    , m_redisTimeoutFlag( src.m_redisTimeoutFlag )
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_redisErrorReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2RedisChannel::~Udp2RedisChannel()
{GUCEF_TRACE;

    delete m_redisReconnectTimer;
    m_redisReconnectTimer = GUCEF_NULL;

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    delete m_redisOptions.command_timeout;
    m_redisOptions.command_timeout = GUCEF_NULL;
    m_redisOptions.connect_timeout = GUCEF_NULL;

    delete m_udpSocket;
    m_udpSocket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // Register UDP socket event handlers
    TEventCallback callback( this, &Udp2RedisChannel::OnUDPSocketError );
    SubscribeTo( m_udpSocket                              ,
                 COMCORE::CUDPSocket::UDPSocketErrorEvent ,
                 callback                                 );
    TEventCallback callback2( this, &Udp2RedisChannel::OnUDPSocketClosed );
    SubscribeTo( m_udpSocket                               ,
                 COMCORE::CUDPSocket::UDPSocketClosedEvent ,
                 callback2                                 );
    TEventCallback callback3( this, &Udp2RedisChannel::OnUDPSocketOpened );
    SubscribeTo( m_udpSocket                               ,
                 COMCORE::CUDPSocket::UDPSocketOpenedEvent ,
                 callback3                                 );
    TEventCallback callback4( this, &Udp2RedisChannel::OnUDPPacketsRecieved );
    SubscribeTo( m_udpSocket                                 ,
                 COMCORE::CUDPSocket::UDPPacketsRecievedEvent ,
                 callback4                                   );
    TEventCallback callback5( this, &Udp2RedisChannel::OnRedisReconnectTimer );
    SubscribeTo( m_redisReconnectTimer          ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback5                      );
    TEventCallback callback6( this, &Udp2RedisChannel::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback6                      );
}

/*-------------------------------------------------------------------------*/

Udp2RedisChannel::ChannelSettings::ChannelSettings( void )
    : redisAddress()
    , channelStreamName()
    , udpInterface()
    , udpMulticastToJoin()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2RedisChannel::ChannelSettings::ChannelSettings( const ChannelSettings& src )
    : redisAddress( src.redisAddress )
    , channelStreamName( src.channelStreamName )
    , udpInterface( src.udpInterface )
    , udpMulticastToJoin( src.udpMulticastToJoin )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2RedisChannel::ChannelSettings&
Udp2RedisChannel::ChannelSettings::operator=( const ChannelSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        redisAddress = src.redisAddress;
        channelStreamName = src.channelStreamName;
        udpInterface = src.udpInterface;
        udpMulticastToJoin = src.udpMulticastToJoin;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisChannel::LoadConfig( const ChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
Udp2RedisChannel::GetRedisErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisErrorReplies = m_redisErrorReplies;
        m_redisErrorReplies = 0;
        return m_redisErrorReplies;
    }
    else
        return m_redisErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::CString
Udp2RedisChannel::GetType( void ) const
{GUCEF_TRACE;

    return "Udp2RedisChannel";
}

/*-------------------------------------------------------------------------*/

Udp2RedisChannel::ChannelMetrics::ChannelMetrics( void )
    : udpBytesReceived( 0 )
    , udpMessagesReceived( 0 )
    , redisMessagesTransmitted( 0 )
    , redisTransmitOverflowQueueSize( 0 )
    , redisErrorReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_metrics.udpBytesReceived = m_udpSocket->GetBytesReceived( true );
    m_metrics.udpMessagesReceived = m_udpSocket->GetNrOfDataReceivedEvents( true );
    m_metrics.redisTransmitOverflowQueueSize = (CORE::UInt32) m_redisMsgQueueOverflowQueue.size();
    m_metrics.redisErrorReplies = GetRedisErrorRepliesCounter( true );
}

/*-------------------------------------------------------------------------*/

const Udp2RedisChannel::ChannelMetrics&
Udp2RedisChannel::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnUDPSocketError( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventID  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CUDPSocket::TSocketErrorEventData* eData = static_cast< COMCORE::CUDPSocket::TSocketErrorEventData* >( eventData );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel: UDP Socket experienced error " + CORE::Int32ToString( eData->GetData() ) );
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnUDPSocketClosed( CORE::CNotifier* notifier   ,
                                     const CORE::CEvent& eventID ,
                                     CORE::CICloneable* evenData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel: UDP Socket has been closed" );
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnUDPSocketOpened( CORE::CNotifier* notifier   ,
                                     const CORE::CEvent& eventID ,
                                     CORE::CICloneable* evenData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel: UDP Socket has been opened" );
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisAddReadEvent( void* privdata )
{GUCEF_TRACE;

    Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( privdata );
    if ( GUCEF_NULL != thisObj )
    {
        thisObj->m_redisReadFlag = true;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisDelReadEvent( void* privdata )
{GUCEF_TRACE;

    Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( privdata );
    if ( GUCEF_NULL != thisObj )
    {
        thisObj->m_redisReadFlag = false;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisAddWriteEvent( void* privdata )
{GUCEF_TRACE;

    Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( privdata );
    if ( GUCEF_NULL != thisObj )
    {
        thisObj->m_redisWriteFlag = true;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisDelWriteEvent( void* privdata )
{GUCEF_TRACE;

    Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( privdata );
    if ( GUCEF_NULL != thisObj )
    {
        thisObj->m_redisWriteFlag = false;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisCleanupEvent( void* privdata )
{GUCEF_TRACE;

    Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( privdata );
    if ( GUCEF_NULL != thisObj )
    {
        thisObj->m_redisContext = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisScheduleTimerEvent( void* privdata, struct timeval tv )
{GUCEF_TRACE;

    Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( privdata );
    if ( GUCEF_NULL != thisObj )
    {
        //thisObj->m_redisReadFlag = false;
        //thisObj->m_redisWriteFlag = false;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisASyncReply( redisAsyncContext* context ,
                                     redisReply* reply          )
{GUCEF_TRACE;

    switch ( reply->type )
    {
        case REDIS_REPLY_INTEGER:
        {
            break;
        }
        case REDIS_REPLY_DOUBLE:
        {
            break;
        }
        case REDIS_REPLY_STRING:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:OnRedisASyncReply(" + CORE::PointerToString( context ) +
                    "): Redis replied with: \"" + CORE::CString( reply->str ) + "\"" );
            break;
        }
        case REDIS_REPLY_ARRAY:
        {
            break;
        }
        case REDIS_REPLY_ERROR:
        {
            ++m_redisErrorReplies;

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisASyncReply(" + CORE::PointerToString( context ) +
                    "): Redis replied with error: \"" + CORE::CString( reply->str ) + "\"" );
            break;
        }
        default:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisASyncReply(" + CORE::PointerToString( context ) +
                    "): Redis replied with unknown reply type " + CORE::Int32ToString( reply->type ) );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisASyncVoidReply( redisAsyncContext* context ,
                                         void *reply                ,
                                         void *privdata             )
{GUCEF_TRACE;

    if ( context != GUCEF_NULL && context->data != GUCEF_NULL )
    {
        Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( context->data );

        if ( context == thisObj->m_redisContext )
        {
            redisReply* rReply = static_cast< redisReply* >( reply );
            if ( GUCEF_NULL != rReply )
            {
                thisObj->OnRedisASyncReply( context, rReply );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:OnRedisASyncReply: Triggered by old context " + CORE::PointerToString( context ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisASyncConnect( const struct redisAsyncContext* context ,
                                       int status                              )
{GUCEF_TRACE;

    if ( context != GUCEF_NULL && context->data != GUCEF_NULL )
    {
        Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( context->data );

        if ( context == thisObj->m_redisContext )
        {
            if ( status == REDIS_OK )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisASyncConnect(" + CORE::PointerToString( context ) + "): Connected to Redis" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisASyncConnect(" + CORE::PointerToString( context ) +
                        "): Failed to connect to Redis due to an error ( Code=" + CORE::Int32ToString( context->err ) + ", Str=\"" + CORE::CString( context->errstr ) +
                        "\" ), attempting to reconnect" );

                redisAsyncContext* disconnectedContext = thisObj->m_redisContext;
                thisObj->m_redisContext = GUCEF_NULL;
                thisObj->m_redisReconnectTimer->SetEnabled( true );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:OnRedisASyncConnect: Triggered by old context " + CORE::PointerToString( context ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisASyncDisconnect( const struct redisAsyncContext* context ,
                                          int status                              )
{GUCEF_TRACE;

    if ( context != GUCEF_NULL && context->data != GUCEF_NULL )
    {
        Udp2RedisChannel* thisObj = static_cast< Udp2RedisChannel* >( context->data );

        if ( context == thisObj->m_redisContext )
        {
            if ( status == REDIS_OK )
            {
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisASyncDisconnect(" + CORE::PointerToString( context ) + "): Disconnected from Redis as requested" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisASyncDisconnect(" + CORE::PointerToString( context ) +
                        "): Disconnected from Redis due to an error ( Code=" + CORE::Int32ToString( context->err ) + ", Str=\"" + CORE::CString( context->errstr ) +
                        "\" ), attempting to reconnect" );

                redisAsyncContext* disconnectedContext = thisObj->m_redisContext;
                thisObj->m_redisContext = GUCEF_NULL;
                thisObj->m_redisReconnectTimer->SetEnabled( true );
            }
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:OnRedisASyncDisconnect: Triggered by old context " + CORE::PointerToString( context ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnRedisReconnectTimer( CORE::CNotifier* notifier   ,
                                         const CORE::CEvent& eventID ,
                                         CORE::CICloneable* evenData )
{GUCEF_TRACE;

    m_redisReconnectTimer->SetEnabled( false );
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnRedisReconnectTimer(" + CORE::PointerToString( m_redisContext ) + "): Trying to reconnect" );
    RedisConnect();
}

/*-------------------------------------------------------------------------*/

int
Udp2RedisChannel::RedisSend( const TPacketEntryVector& udpPackets , 
                             CORE::UInt32 packetCount             )
{GUCEF_TRACE;

    int errCode = REDIS_OK;
    int retCode = 0;
    for ( CORE::UInt32 i=0; i<packetCount; ++i )
    {
        const CORE::CDynamicBuffer& buffer = udpPackets[ i ].dataBuffer.GetData();
        retCode = redisAsyncCommand( m_redisContext,
                                     &OnRedisASyncVoidReply,
                                     this,
                                     m_redisStreamSendCmd.C_String(),
                                     buffer.GetConstBufferPtr(),
                                     buffer.GetDataSize() );
        if ( retCode != REDIS_OK )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:RedisSend: Failed executing async send command" );
            errCode = retCode;
        }
        #ifdef GUCEF_DEBUG_MODE
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "Udp2RedisChannel:RedisSend: Successfully executed async send command" );
        }
        #endif
    }
    return errCode;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisChannel::SendQueuedPackagesIfAny( void )
{GUCEF_TRACE;

    TPacketEntryVector packetBundle;
    size_t i=0;
    TDynamicBufferQueue::iterator b = m_redisMsgQueueOverflowQueue.begin();
    TDynamicBufferQueue::iterator e = m_redisMsgQueueOverflowQueue.begin();
    while ( i < GUCEF_QUEUE_SEND_DEFAULT_BATCH_SIZE && i < m_redisMsgQueueOverflowQueue.size() )
    {
        TPacketEntry entry;
        entry.dataBuffer.LinkTo( &m_redisMsgQueueOverflowQueue.at( i ) );
        packetBundle.push_back( entry );
        ++i; ++e;    
    }

    if ( !packetBundle.empty() )
    {
        if ( RedisSend( packetBundle, (CORE::UInt32) packetBundle.size() ) )
        {
            m_redisMsgQueueOverflowQueue.erase( b, e );
            return true;
        }
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisChannel::AddToOverflowQueue( const TPacketEntryVector& udpPackets ,
                                      CORE::UInt32 packetCount             )
{GUCEF_TRACE;

    for ( CORE::UInt32 i=0; i<packetCount; ++i )
    {
        m_redisMsgQueueOverflowQueue.push_back( udpPackets[ i ].dataBuffer.GetData() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnUDPPacketsRecieved( CORE::CNotifier* notifier   ,
                                        const CORE::CEvent& eventID ,
                                        CORE::CICloneable* evenData )
{GUCEF_TRACE;

    COMCORE::CUDPSocket::UDPPacketsRecievedEventData* udpPacketData = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( evenData );
    if ( GUCEF_NULL != udpPacketData )
    {
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = udpPacketData->GetData();

        #ifdef GUCEF_DEBUG_MODE
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel: UDP Socket received " + CORE::UInt32ToString( data.packetsReceived ) + " packets" );
        #endif

        if ( SendQueuedPackagesIfAny() )
        {
            int retCode = RedisSend( data.packets, data.packetsReceived );
            if ( retCode == REDIS_ERR )
                AddToOverflowQueue( data.packets, data.packetsReceived ); 
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel: UDP Socket has a data received event but no data was provided" );
    }
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisChannel::RedisConnect( void )
{
    delete m_redisOptions.command_timeout;
    m_redisOptions.command_timeout = GUCEF_NULL;
    m_redisOptions.connect_timeout = GUCEF_NULL;

    memset( &m_redisOptions, 0, sizeof(m_redisOptions) );
    REDIS_OPTIONS_SET_TCP( &m_redisOptions, m_channelSettings.redisAddress.GetHostname().C_String(), m_channelSettings.redisAddress.GetPortInHostByteOrder() );
    struct timeval* timeoutSetting = new struct timeval;
    timeoutSetting->tv_sec = 10;
    timeoutSetting->tv_usec = 0;
    m_redisOptions.command_timeout = timeoutSetting;
    m_redisOptions.connect_timeout = timeoutSetting;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:RedisConnect: Connecting to Redis on " + m_channelSettings.redisAddress.HostnameAndPortAsString() );

    redisAsyncContext* rContext = redisAsyncConnectWithOptions( &m_redisOptions );
	if ( rContext == GUCEF_NULL )
    {
		GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:RedisConnect: Failed to create Redis context" );
        m_redisReconnectTimer->SetEnabled( true );
        return false;
	}

    // Allow ourselves to find this object again from the C callbacks
    rContext->data = this;
    rContext->ev.data = this;
    // Hook up to the event callbacks into hiredis since we also act as the app engine
    rContext->ev.addRead = &OnRedisAddReadEvent;
    rContext->ev.addWrite = &OnRedisAddWriteEvent;
    rContext->ev.delRead = &OnRedisDelReadEvent;
    rContext->ev.delWrite = &OnRedisDelWriteEvent;
    rContext->ev.cleanup = &OnRedisCleanupEvent;
    rContext->ev.scheduleTimer = &OnRedisScheduleTimerEvent;

	if ( rContext->err != REDIS_OK )
    {
		GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:RedisConnect(" + CORE::PointerToString( rContext ) +
                            "): Failed to create Redis context: Error " + CORE::Int32ToString( rContext->err ) + " : " + CORE::CString( rContext->errstr ) );
        m_redisReconnectTimer->SetEnabled( true );
        return false;
	}

    // Set the stream publish command formatting here which will remain constant for this session
    m_redisStreamSendCmd = "XADD " + m_channelSettings.channelStreamName + " * UDP %b";

    redisAsyncSetConnectCallback( rContext, &OnRedisASyncConnect );
    redisAsyncSetDisconnectCallback( rContext, &OnRedisASyncDisconnect );

    m_redisContext = rContext;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:RedisConnect(" + CORE::PointerToString( rContext ) + "): Successfully created a Redis context" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisChannel::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

	m_udpSocket = new COMCORE::CUDPSocket( GetPulseGenerator(), false );
    m_redisReconnectTimer = new CORE::CTimer( GetPulseGenerator(), 10 );
    m_metricsTimer = new CORE::CTimer( GetPulseGenerator(), 1000 );

    RegisterEventHandlers();

    // Setup connection to Redis and open the UDP port.
    // Note that if there is an error here we will just keep on trying
    RedisConnect();
    m_udpSocket->SetMaxUpdatesPerCycle( 10 );
    m_udpSocket->SetAutoReOpenOnError( true );
    if ( m_udpSocket->Open( m_channelSettings.udpInterface.GetFirstIPv4Address() ) )
    {
		GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:OnTaskStart: Successfully opened UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );

        ChannelSettings::HostAddressVector::iterator m = m_channelSettings.udpMulticastToJoin.begin();
        while ( m != m_channelSettings.udpMulticastToJoin.end() )
        {
            const COMCORE::CHostAddress& multicastAddr = (*m);
            if ( m_udpSocket->Join( multicastAddr.GetFirstIPv4Address() ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2RedisChannel:OnTaskStart: Successfully to joined multicast " + multicastAddr.HostnameAndPortAsString() +
                        " for UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnTaskStart: Failed to join multicast " + multicastAddr.HostnameAndPortAsString() +
                        " for UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
            }
            ++m;
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2RedisChannel:OnTaskStart: Failed to open UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Udp2RedisChannel::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // Poll the Redis socket etc,..
    if ( GUCEF_NULL != m_redisContext && m_redisWriteFlag )
        redisAsyncHandleWrite( m_redisContext );
    if ( GUCEF_NULL != m_redisContext && m_redisReadFlag )
        redisAsyncHandleRead( m_redisContext );
    //if ( GUCEF_NULL != m_redisContext )
    //    redisAsyncHandleTimeout( m_redisContext );

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/

void
Udp2RedisChannel::OnTaskEnded( CORE::CTaskPtr task ,
                               bool wasForced      )
{GUCEF_TRACE;

    redisAsyncDisconnect( m_redisContext );
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisInfoResource::RestApiUdp2RedisInfoResource( Udp2Redis* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisInfoResource::~RestApiUdp2RedisInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiUdp2RedisInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                         CORE::CDataNode& output             ,
                                         const CORE::CString& representation ,
                                         const CORE::CString& params         )
{GUCEF_TRACE;

    output.SetName( "info" );
    output.SetAttribute( "application", "udp2redis" );
    output.SetAttribute( "buildDateTime", __TIMESTAMP__ );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", "false" );
    #else
    output.SetAttribute( "isReleaseBuild", "true" );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisConfigResource::RestApiUdp2RedisConfigResource( Udp2Redis* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2RedisConfigResource::~RestApiUdp2RedisConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiUdp2RedisConfigResource::Serialize( const CORE::CString& resourcePath   ,
                                           CORE::CDataNode& output             ,
                                           const CORE::CString& representation ,
                                           const CORE::CString& params         )
{GUCEF_TRACE;

    if ( m_appConfig )
    {
        const CORE::CValueList& loadedConfig = m_app->GetAppConfig();
        return loadedConfig.SaveConfig( output );
    }

    const CORE::CDataNode& globalConfig = m_app->GetGlobalConfig();
    output.Copy( globalConfig );
    return true;
}

/*-------------------------------------------------------------------------*/

Udp2Redis::Udp2Redis( void )
    : CORE::CObserver()
    , m_udpStartPort()
    , m_channelCount()
    , m_redisStreamStartChannelID()
    , m_redisStreamName()
    , m_redisHost()
    , m_redisPort()
    , m_channels()
    , m_channelSettings()
    , m_httpServer()
    , m_httpRouter()
    , m_appConfig()
    , m_globalConfig()
    , m_metricsTimer()
    , m_transmitMetrics( true )
{GUCEF_TRACE;

    TEventCallback callback1( this, &Udp2Redis::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );
}

/*-------------------------------------------------------------------------*/

Udp2Redis::~Udp2Redis()
{GUCEF_TRACE;

    m_httpServer.Close();
}

/*-------------------------------------------------------------------------*/

bool
Udp2Redis::Start( void )
{GUCEF_TRACE;

    bool errorOccured = false;
    m_channels.resize( m_channelCount );
    for ( size_t m=0; m<m_channels.size(); ++m )
    {
        m_channels[ m ] = Udp2RedisChannelPtr( new Udp2RedisChannel() );
    }

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

    CORE::UInt16 udpPort = m_udpStartPort;
    CORE::Int32 channelId = m_redisStreamStartChannelID;
    Udp2RedisChannelVector::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        Udp2RedisChannelPtr& channel = (*i);
        ChannelSettingsMap::iterator n = m_channelSettings.find( channelId );
        if ( n != m_channelSettings.end() )
        {
            const Udp2RedisChannel::ChannelSettings& channelSettings = (*n).second;
            if ( !channel->LoadConfig( channelSettings ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2Redis:Start: Failed to set channel settings on channel " + CORE::Int32ToString( channelId ) );
                errorOccured = true;
                break;
            }

            if ( !threadPool->StartTask( channel ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2Redis:Start: Failed to start task (dedicated thread) for channel " + CORE::Int32ToString( channelId ) );
                errorOccured = true;
                break;
            }
        }

        ++channelId;
        ++i;
    }

    if ( !errorOccured && m_transmitMetrics )
    {
        m_metricsTimer.SetInterval( 1000 );
        m_metricsTimer.SetEnabled( true );
    }

    if ( !errorOccured )
    {
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2Redis: Opening REST API" );
        return m_httpServer.Listen();
    }
    return errorOccured;
}

/*-------------------------------------------------------------------------*/

bool
Udp2Redis::LoadConfig( const CORE::CValueList& appConfig   ,
                       const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    m_transmitMetrics = CORE::StringToBool( appConfig.GetValueAlways( "TransmitMetrics", "true" ) );

    m_udpStartPort = CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "UdpStartPort", "20000" ) ) );
    m_channelCount = CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "ChannelCount", "1" ) ) );
    m_redisStreamStartChannelID = CORE::StringToInt32( CORE::ResolveVars(  appConfig.GetValueAlways( "RedisStreamStartChannelID", "1" ) ) );
    m_redisStreamName = CORE::ResolveVars( appConfig.GetValueAlways( "RedisStreamName", "udp-ingress-ch{channelID}" ) );
    m_redisHost = CORE::ResolveVars( appConfig.GetValueAlways( "RedisHost", "127.0.0.1" ) );
    m_redisPort = CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "RedisPort", "6379" ) ) );

    CORE::UInt16 udpPort = m_udpStartPort;
    CORE::Int32 maxChannelId = m_redisStreamStartChannelID + m_channelCount;
    for ( CORE::Int32 channelId = m_redisStreamStartChannelID; channelId < maxChannelId; ++channelId )
    {
        Udp2RedisChannel::ChannelSettings& channelSettings = m_channelSettings[ channelId ];

        channelSettings.redisAddress.SetHostname( m_redisHost );
        channelSettings.redisAddress.SetPortInHostByteOrder( m_redisPort );

        CORE::CString settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".RedisStreamName";
        CORE::CVariant settingValue = appConfig.GetValueAlways( settingName );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.channelStreamName = CORE::ResolveVars( settingValue.AsString().ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) ) );
        }
        else
        {
            // Use the auto naming and numbering scheme based on a single template name instead
            channelSettings.channelStreamName = CORE::ResolveVars( m_redisStreamName.ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) ) );
        }

        settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".UdpInterface";
        settingValue = appConfig.GetValueAlways( settingName );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.udpInterface.SetHostnameAndPort( settingValue );
        }
        else
        {
            // Use the auto numbering scheme instead
            channelSettings.udpInterface.SetPortInHostByteOrder( udpPort );
        }

        settingName = "ChannelSetting." + CORE::Int32ToString( channelId ) + ".Multicast.Join";
        CORE::CValueList::TVariantVector settingValues = appConfig.GetValueVectorAlways( settingName );
        CORE::CValueList::TVariantVector::iterator n = settingValues.begin();
        while ( n != settingValues.end() )
        {
            CORE::CString settingValue = (*n).AsString();
            COMCORE::CHostAddress multicastAddress( settingValue );
            channelSettings.udpMulticastToJoin.push_back( multicastAddress );
            ++n;
        }

        ++udpPort;
    }

    m_appConfig = appConfig;
    m_globalConfig.Copy( globalConfig );

    m_httpServer.SetPort( CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "RestApiPort", "10000" ) ) ) );

    m_httpRouter.SetResourceMapping( "/info", RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new RestApiUdp2RedisInfoResource( this ) )  );
    m_httpRouter.SetResourceMapping( "/config/appargs", RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new RestApiUdp2RedisConfigResource( this, true ) )  );
    m_httpRouter.SetResourceMapping( "/config", RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new RestApiUdp2RedisConfigResource( this, false ) )  );
    m_httpRouter.SetResourceMapping(  CORE::ResolveVars( appConfig.GetValueAlways( "RestBasicHealthUri", "/health/basic" ) ), RestApiUdp2RedisInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );

    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
Udp2Redis::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::Int32 channelId = m_redisStreamStartChannelID;
    Udp2RedisChannelVector::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        const Udp2RedisChannel::ChannelMetrics& metrics = (*i)->GetMetrics();
        CORE::CString metricPrefix = "udp2redis.ch" + CORE::Int32ToString( channelId ) + ".";

        GUCEF_METRIC_COUNT( metricPrefix + "redisMessagesTransmitted", metrics.redisMessagesTransmitted, 1.0f );
        GUCEF_METRIC_GAUGE( metricPrefix + "redisTransmitOverflowQueueSize", metrics.redisTransmitOverflowQueueSize, 1.0f );
        GUCEF_METRIC_COUNT( metricPrefix + "udpBytesReceived", metrics.udpBytesReceived, 1.0f );
        GUCEF_METRIC_COUNT( metricPrefix + "udpMessagesReceived", metrics.udpMessagesReceived, 1.0f );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
Udp2Redis::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
Udp2Redis::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/
