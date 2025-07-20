/*
 *  udp2kafka: service which pushes UDP packets into kafka topics
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

#include "udp2kafka.h"

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

const std::string Udp2KafkaChannel::KafkaMsgHeader_UdpOrigin = "UdpOrigin";
const std::string Udp2KafkaChannel::KafkaMsgHeader_ProducerHostname = "ProducerHostname";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

Udp2KafkaChannel::Udp2KafkaChannel()
    : CORE::CTaskConsumer()
    , RdKafka::EventCb()
    , RdKafka::DeliveryReportCb()
    , RdKafka::ConsumeCb()
    , RdKafka::RebalanceCb()
    , m_kafkaProducerConf( GUCEF_NULL )
    , m_kafkaConsumerConf( GUCEF_NULL )
    , m_kafkaProducerTopicConf( GUCEF_NULL )
    , m_kafkaConsumerTopicConf( GUCEF_NULL )
    , m_kafkaProducer( GUCEF_NULL )
    , m_kafkaProducerTopic( GUCEF_NULL )
    , m_kafkaConsumer( GUCEF_NULL )
    , m_channelSettings()
    , m_udpSocket( GUCEF_NULL )
    , m_kafkaMsgQueueOverflowQueue()
    , m_metricsTimer( GUCEF_NULL )
    , m_metrics()
    , m_kafkaErrorReplies( 0 )
    , m_kafkaMsgsTransmitted( 0 )
    , m_kafkaMessagesReceived( 0 )
    , m_kafkaMessagesFiltered( 0 )
    , m_kafkaBrokers()
    , m_producerHostname( CORE::GetHostname() )
    , m_firstPartitionAssignment( true )
    , m_consumerOffsets()
    , m_tickCountAtLastOffsetCommit( 0 )
{GUCEF_TRACE;

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

Udp2KafkaChannel::Udp2KafkaChannel( const Udp2KafkaChannel& src )
    : CORE::CTaskConsumer()
    , RdKafka::EventCb()
    , RdKafka::DeliveryReportCb()
    , RdKafka::ConsumeCb()
    , RdKafka::RebalanceCb()
    , m_kafkaProducerConf( GUCEF_NULL )
    , m_kafkaConsumerConf( GUCEF_NULL )
    , m_kafkaProducerTopicConf( GUCEF_NULL )
    , m_kafkaConsumerTopicConf( GUCEF_NULL )
    , m_kafkaProducer( GUCEF_NULL )
    , m_kafkaProducerTopic( GUCEF_NULL )
    , m_kafkaConsumer( GUCEF_NULL )
    , m_channelSettings( src.m_channelSettings )
    , m_udpSocket( src.m_udpSocket )
    , m_kafkaMsgQueueOverflowQueue()
    , m_metricsTimer( src.m_metricsTimer )
    , m_metrics()
    , m_kafkaErrorReplies( 0 )
    , m_kafkaMsgsTransmitted( 0 )
    , m_kafkaMessagesReceived( 0 )
    , m_kafkaMessagesFiltered( 0 )
    , m_kafkaBrokers( src.m_kafkaBrokers )
    , m_producerHostname( CORE::GetHostname() )
    , m_firstPartitionAssignment( true )
    , m_consumerOffsets()
    , m_tickCountAtLastOffsetCommit( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2KafkaChannel::~Udp2KafkaChannel()
{GUCEF_TRACE;

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;
    
    delete m_udpSocket;
    m_udpSocket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // Register UDP socket event handlers
    TEventCallback callback( this, &Udp2KafkaChannel::OnUDPSocketError );
    SubscribeTo( m_udpSocket                              ,
                 COMCORE::CUDPSocket::UDPSocketErrorEvent ,
                 callback                                 );
    TEventCallback callback2( this, &Udp2KafkaChannel::OnUDPSocketClosed );
    SubscribeTo( m_udpSocket                               ,
                 COMCORE::CUDPSocket::UDPSocketClosedEvent ,
                 callback2                                 );
    TEventCallback callback3( this, &Udp2KafkaChannel::OnUDPSocketClosing );
    SubscribeTo( m_udpSocket                                ,
                 COMCORE::CUDPSocket::UDPSocketClosingEvent ,
                 callback3                                  );
    TEventCallback callback4( this, &Udp2KafkaChannel::OnUDPSocketOpened );
    SubscribeTo( m_udpSocket                               ,
                 COMCORE::CUDPSocket::UDPSocketOpenedEvent ,
                 callback4                                 );
    TEventCallback callback5( this, &Udp2KafkaChannel::OnUDPPacketsRecieved );
    SubscribeTo( m_udpSocket                                  ,
                 COMCORE::CUDPSocket::UDPPacketsRecievedEvent ,
                 callback5                                    );
    
    TEventCallback callback6( this, &Udp2KafkaChannel::OnMetricsTimerCycle );
    SubscribeTo( m_metricsTimer                 ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback6                      );
}

/*-------------------------------------------------------------------------*/

Udp2KafkaChannel::ChannelSettings::ChannelSettings( void )
    : kafkaBrokers( "127.0.0.1:9092" )
    , channelTopicName()
    , udpInterface()
    , udpMulticastToJoin()
    , collectMetrics( false )
    , metricsPrefix()
    , wantsTestPackage( false )
    , mode( Udp2KafkaChannel::EChannelMode::KAFKA_PRODUCER )
    , consumerModeStartOffset( "stored" )
    , consumerModeGroupId( "0" )
    , consumerModeUdpDestinations()
    , kafkaProducerGlobalConfigSettings()
    , kafkaConsumerGlobalConfigSettings()
    , kafkaProducerTopicConfigSettings()
    , kafkaConsumerTopicConfigSettings()
    , kafkaMsgHeaderUsedForFiltering( KafkaMsgHeader_ProducerHostname )
    , kafkaMsgValuesUsedForFiltering()
    , addUdpOriginKafkaMsgHeader( true )
    , addProducerHostnameKafkaMsgHeader( true )
{GUCEF_TRACE;

     kafkaMsgValuesUsedForFiltering.push_back( CORE::GetHostname() );
}

/*-------------------------------------------------------------------------*/

Udp2KafkaChannel::ChannelSettings::ChannelSettings( const ChannelSettings& src )
    : kafkaBrokers( src.kafkaBrokers )
    , channelTopicName( src.channelTopicName )
    , udpInterface( src.udpInterface )
    , udpMulticastToJoin( src.udpMulticastToJoin )
    , collectMetrics( src.collectMetrics )
    , metricsPrefix( src.metricsPrefix )
    , wantsTestPackage( src.wantsTestPackage )
    , mode( src.mode )
    , consumerModeStartOffset( src.consumerModeStartOffset )
    , consumerModeGroupId( src.consumerModeGroupId )
    , consumerModeUdpDestinations( src.consumerModeUdpDestinations )
    , kafkaProducerGlobalConfigSettings( src.kafkaProducerGlobalConfigSettings )
    , kafkaConsumerGlobalConfigSettings( src.kafkaConsumerGlobalConfigSettings )
    , kafkaProducerTopicConfigSettings( src.kafkaProducerTopicConfigSettings )
    , kafkaConsumerTopicConfigSettings( src.kafkaConsumerTopicConfigSettings )
    , kafkaMsgHeaderUsedForFiltering( src.kafkaMsgHeaderUsedForFiltering )
    , kafkaMsgValuesUsedForFiltering( src.kafkaMsgValuesUsedForFiltering )
    , addUdpOriginKafkaMsgHeader( src.addUdpOriginKafkaMsgHeader )
    , addProducerHostnameKafkaMsgHeader( src.addProducerHostnameKafkaMsgHeader )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

Udp2KafkaChannel::ChannelSettings&
Udp2KafkaChannel::ChannelSettings::operator=( const ChannelSettings& src )
{GUCEF_TRACE;

    if ( this != &src )
    {
        kafkaBrokers = src.kafkaBrokers;
        channelTopicName = src.channelTopicName;
        udpInterface = src.udpInterface;
        udpMulticastToJoin = src.udpMulticastToJoin;
        collectMetrics = src.collectMetrics;
        metricsPrefix = src.metricsPrefix;
        wantsTestPackage = src.wantsTestPackage;
        mode = src.mode;
        consumerModeStartOffset = src.consumerModeStartOffset;
        consumerModeGroupId = src.consumerModeGroupId;
        consumerModeUdpDestinations = src.consumerModeUdpDestinations;
        kafkaProducerGlobalConfigSettings = src.kafkaProducerGlobalConfigSettings;
        kafkaProducerTopicConfigSettings = src.kafkaProducerTopicConfigSettings;
        kafkaConsumerGlobalConfigSettings = src.kafkaConsumerGlobalConfigSettings;
        kafkaConsumerTopicConfigSettings = src.kafkaConsumerTopicConfigSettings;
        kafkaMsgHeaderUsedForFiltering = src.kafkaMsgHeaderUsedForFiltering;
        kafkaMsgValuesUsedForFiltering = src.kafkaMsgValuesUsedForFiltering;
        addUdpOriginKafkaMsgHeader = src.addUdpOriginKafkaMsgHeader;
        addProducerHostnameKafkaMsgHeader = src.addProducerHostnameKafkaMsgHeader;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool 
Udp2KafkaChannel::LoadConfig( const ChannelSettings& channelSettings )
{GUCEF_TRACE;

    m_channelSettings = channelSettings;
    return true;
}

/*-------------------------------------------------------------------------*/

const Udp2KafkaChannel::ChannelSettings& 
Udp2KafkaChannel::GetChannelSettings( void ) const
{GUCEF_TRACE;

    return m_channelSettings;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
Udp2KafkaChannel::GetKafkaErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisErrorReplies = m_kafkaErrorReplies;
        m_kafkaErrorReplies = 0;
        return redisErrorReplies;
    }
    else
        return m_kafkaErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
Udp2KafkaChannel::GetKafkaMsgsTransmittedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 kafkaMsgsTransmitted = m_kafkaMsgsTransmitted;
        m_kafkaMsgsTransmitted = 0;
        return kafkaMsgsTransmitted;
    }
    else
        return m_kafkaMsgsTransmitted;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
Udp2KafkaChannel::GetKafkaMsgsReceivedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 kafkaMsgsReceived = m_kafkaMessagesReceived;
        m_kafkaMessagesReceived = 0;
        return kafkaMsgsReceived;
    }
    else
        return m_kafkaMessagesReceived;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
Udp2KafkaChannel::GetKafkaMsgsFilteredCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 kafkaMsgsFiltered = m_kafkaMessagesFiltered;
        m_kafkaMessagesFiltered = 0;
        return kafkaMsgsFiltered;
    }
    else
        return m_kafkaMessagesFiltered;
}

/*-------------------------------------------------------------------------*/

CORE::CString
Udp2KafkaChannel::GetType( void ) const
{GUCEF_TRACE;

    return "Udp2KafkaChannel";
}

/*-------------------------------------------------------------------------*/

Udp2KafkaChannel::ChannelMetrics::ChannelMetrics( void )
    : udpBytesReceived( 0 )
    , udpMessagesReceived( 0 )
    , kafkaMessagesTransmitted( 0 )
    , kafkaTransmitQueueSize( 0 )
    , kafkaTransmitOverflowQueueSize( 0 )
    , udpBytesTransmitted( 0 )
    , udpMessagesTransmitted( 0 )
    , kafkaMessagesReceived( 0 )
    , kafkaMessagesFiltered( 0 )
    , kafkaErrorReplies( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_metrics.kafkaErrorReplies = GetKafkaErrorRepliesCounter( true );
    if ( EChannelMode::KAFKA_PRODUCER == m_channelSettings.mode || EChannelMode::KAFKA_PRODUCER_AND_CONSUMER == m_channelSettings.mode )
    {
        m_metrics.udpBytesReceived = m_udpSocket->GetBytesReceived( true );
        m_metrics.udpMessagesReceived = m_udpSocket->GetNrOfDataReceivedEvents( true );
        m_metrics.kafkaTransmitQueueSize = (CORE::UInt32) m_kafkaProducer->outq_len();
        m_metrics.kafkaTransmitOverflowQueueSize = (CORE::UInt32) m_kafkaMsgQueueOverflowQueue.size();   
        m_metrics.kafkaMessagesTransmitted = GetKafkaMsgsTransmittedCounter( true );
    }
    if ( EChannelMode::KAFKA_CONSUMER == m_channelSettings.mode || EChannelMode::KAFKA_PRODUCER_AND_CONSUMER == m_channelSettings.mode )
    {
        m_metrics.udpBytesTransmitted = m_udpSocket->GetBytesTransmitted( true );
        m_metrics.udpMessagesTransmitted = m_udpSocket->GetNrOfDataSentEvents( true );
        m_metrics.kafkaMessagesReceived = GetKafkaMsgsReceivedCounter( true );
        m_metrics.kafkaMessagesFiltered = GetKafkaMsgsFilteredCounter( true );
    }
}

/*-------------------------------------------------------------------------*/

const Udp2KafkaChannel::ChannelMetrics&
Udp2KafkaChannel::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::OnUDPSocketError( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventID  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CUDPSocket::TSocketErrorEventData* eData = static_cast< COMCORE::CUDPSocket::TSocketErrorEventData* >( eventData );    
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: UDP Socket experienced error " + CORE::Int32ToString( eData->GetData() ) );    
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::OnUDPSocketClosed( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventID  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: UDP Socket has been closed" );
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::OnUDPSocketClosing( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventID  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: UDP Socket is going to close" );

    // Gracefully leave the multicast groups we joined
    ChannelSettings::HostAddressVector::iterator m = m_channelSettings.udpMulticastToJoin.begin();
    while ( m != m_channelSettings.udpMulticastToJoin.end() )
    {
        const COMCORE::CHostAddress& multicastAddr = (*m);
        if ( m_udpSocket->Leave( multicastAddr.GetFirstIPv4Address() ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnUDPSocketClosing: Successfully to left multicast " + multicastAddr.HostnameAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnUDPSocketClosing: Failed to leave multicast " + multicastAddr.HostnameAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
        }
        ++m;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::OnUDPSocketOpened( CORE::CNotifier* notifier   ,
                                     const CORE::CEvent& eventID ,
                                     CORE::CICloneable* evenData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: UDP Socket has been opened" );

    ChannelSettings::HostAddressVector::iterator m = m_channelSettings.udpMulticastToJoin.begin();
    while ( m != m_channelSettings.udpMulticastToJoin.end() )
    {
        const COMCORE::CHostAddress& multicastAddr = (*m);
        if ( m_udpSocket->Join( multicastAddr.GetFirstIPv4Address(), false, true ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnUDPSocketOpened: Successfully joined multicast " + multicastAddr.HostnameAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnUDPSocketOpened: Failed to join multicast " + multicastAddr.HostnameAndPortAsString() +
                    " for UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
        }
        ++m;
    }
}

/*-------------------------------------------------------------------------*/

bool
Udp2KafkaChannel::UdpTransmit( RdKafka::Message& message )
{GUCEF_TRACE;

    if ( message.len() > GUCEFCORE_UINT16MAX )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:consume_cb: Received " + CORE::UInt64ToString( CORE::UInt64( message.len() ) ) + " byte message on topic \"" +          
                m_channelSettings.channelTopicName + " at offset " + CORE::Int64ToString( message.offset() ) + ". This exceeds the max size of a UInt16 and cannot be sent as a UDP packet" );
        return false;
    }

    CORE::UInt32 successCount = 0;
    HostAddressVector::iterator i = m_channelSettings.consumerModeUdpDestinations.begin();
    while ( i != m_channelSettings.consumerModeUdpDestinations.end() )
    {
        if ( 0 < m_udpSocket->SendPacketTo( (*i).GetFirstIPv4Address(), message.payload(), (CORE::UInt16) message.len() ) )
            ++successCount;
        ++i;
    }

    // We will consider a partial transmission success 'good enough' wrt consumer offset commits
    // We only update our own offset record here and dont actually commit yet since we do not want to
    // commit the offset on a per message basis, that would be too much of a performance penalty
    if ( successCount > 0 )
    {
        m_consumerOffsets[ message.partition() ] = message.offset();
    }

    return successCount == m_channelSettings.consumerModeUdpDestinations.size();
}

/*-------------------------------------------------------------------------*/

void 
Udp2KafkaChannel::offset_commit_cb( RdKafka::ErrorCode err                         ,
                                    std::vector<RdKafka::TopicPartition*>& offsets )
{GUCEF_TRACE;

    if ( RdKafka::ERR_NO_ERROR != err && RdKafka::ERR__NO_OFFSET != err )
    {
        std::string errStr = RdKafka::err2str( err );
        CORE::CString commitInfo = "Udp2KafkaChannel:offset_commit_cb: Member ID \"" + m_kafkaConsumer->memberid() + "\": Code: " + errStr + " : ";
        for ( unsigned int i=0; i<offsets.size(); ++i )
        {
            commitInfo += "Topic \"" + offsets[ i ]->topic() + "\" is at partition \"" + CORE::Int32ToString( offsets[ i ]->partition() ).STL_String() + "\" at offset \"" + ConvertKafkaConsumerStartOffset( offsets[ i ]->offset() ).STL_String() + "\". ";
        }
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, commitInfo );
    }
    #ifdef GUCEF_DEBUG_MODE
    else
    {
        std::string errStr = RdKafka::err2str( err );
        CORE::CString commitInfo = "Udp2KafkaChannel:offset_commit_cb: Member ID \"" + m_kafkaConsumer->memberid() + "\": Code: " + errStr + " : ";
        for ( unsigned int i=0; i<offsets.size(); ++i )
        {
            commitInfo += "Topic \"" + offsets[ i ]->topic() + "\" is at partition \"" + CORE::Int32ToString( offsets[ i ]->partition() ).STL_String() + "\" at offset \"" + ConvertKafkaConsumerStartOffset( offsets[ i ]->offset() ).STL_String() + "\". ";
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, commitInfo );
    }
    #endif
}

/*-------------------------------------------------------------------------*/

void 
Udp2KafkaChannel::rebalance_cb( RdKafka::KafkaConsumer* consumer                  ,
                                RdKafka::ErrorCode err                            ,
                                std::vector<RdKafka::TopicPartition*>& partitions )
{GUCEF_TRACE;

    std::string actionStr = "<UNKNOWN>";
    if ( err == RdKafka::ERR__ASSIGN_PARTITIONS ) 
    {
        RdKafka::ErrorCode assignSuccess = consumer->assign( partitions );
        for ( unsigned int i=0; i<partitions.size(); ++i )
        {
            if ( m_firstPartitionAssignment || RdKafka::Topic::OFFSET_INVALID == partitions[ i ]->offset() )
            {
                CORE::Int64 startOffset = ConvertKafkaConsumerStartOffset( m_channelSettings.consumerModeStartOffset, partitions[ i ]->partition(), 10000 );    
                partitions[ i ]->set_offset( startOffset );
                m_consumerOffsets[ partitions[ i ]->partition() ] = startOffset;
            }
        }
        if (  m_firstPartitionAssignment )
            m_firstPartitionAssignment = false;

        assignSuccess = consumer->assign( partitions );
        if ( RdKafka::ERR_NO_ERROR != assignSuccess )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:rebalance_cb: Failed to assign new partitions" );

        RdKafka::ErrorCode ec = consumer->offsets_store( partitions );
        RdKafka::ErrorCode ec2 = consumer->commitSync( this );
        actionStr = "ASSIGN_PARTITIONS";
    } 
    else
    if ( err == RdKafka::ERR__REVOKE_PARTITIONS ) 
    {
        consumer->unassign();
        for ( unsigned int i=0; i<partitions.size(); ++i )
        {
            m_consumerOffsets.erase( partitions[ i ]->partition() );
        }
        actionStr = "REVOKE_PARTITIONS";
    }

    CORE::CString partitionInfo = "Udp2KafkaChannel:rebalance_cb: Member ID \"" + consumer->memberid() + "\": Action " + actionStr + " : ";
    for ( unsigned int i=0; i<partitions.size(); ++i )
    {
        partitionInfo += "Topic \"" + partitions[ i ]->topic() + "\" is at partition \"" + CORE::Int32ToString( partitions[ i ]->partition() ).STL_String() + 
                "\" at offset \"" + ConvertKafkaConsumerStartOffset( partitions[ i ]->offset() ).STL_String() + "\". ";

        int64_t high = 0; int64_t low = 0;
        RdKafka::ErrorCode err = m_kafkaConsumer->get_watermark_offsets( m_channelSettings.channelTopicName, partitions[ i ]->partition(), &low, &high );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel: Offsets for topic \"" + m_channelSettings.channelTopicName + 
                    "\" and partition " + CORE::Int32ToString( partitions[ i ]->partition() ) + ": Low=" + ConvertKafkaConsumerStartOffset( low ) + 
                    ", High=" + ConvertKafkaConsumerStartOffset( high ) );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, partitionInfo );
}

/*-------------------------------------------------------------------------*/

void 
Udp2KafkaChannel::consume_cb( RdKafka::Message& message, void* opaque )
{GUCEF_TRACE;

    switch ( message.err() ) 
    {
        case RdKafka::ERR__TIMED_OUT:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka consume error: TIMED_OUT: " + message.errstr() );
            break;
        }
        case RdKafka::ERR_NO_ERROR:
        {
            #ifdef GUCEF_DEBUG_MODE
            if ( nullptr != message.key() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Udp2KafkaChannel:consume_cb: Received " + CORE::UInt64ToString( CORE::UInt64( message.len() ) ) + " byte message on topic \"" +          
                        m_channelSettings.channelTopicName + "\" with key \"" + *message.key() + "\" and offset " + CORE::Int64ToString( message.offset() ) );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Udp2KafkaChannel:consume_cb: Received " + CORE::UInt64ToString( CORE::UInt64( message.len() ) ) + " byte message on topic \"" +          
                        m_channelSettings.channelTopicName + " without a key and with offset " + CORE::Int64ToString( message.offset() ) );
            }
            #endif
            
            ++m_kafkaMessagesReceived;

            bool isFiltered = false;
            const RdKafka::Headers* headers = message.headers();
            if ( GUCEF_NULL != headers && !m_channelSettings.kafkaMsgHeaderUsedForFiltering.empty() ) 
            {
                std::vector<RdKafka::Headers::Header> hdrs = headers->get( m_channelSettings.kafkaMsgHeaderUsedForFiltering );
                for ( size_t i=0; i<hdrs.size(); ++i ) 
                {
                    const char* hdrValue = hdrs[ i ].value_string();
                    if ( GUCEF_NULL != hdrValue )
                    {
                        CORE::CVariant::VariantVector::iterator v = m_channelSettings.kafkaMsgValuesUsedForFiltering.begin();
                        while ( v != m_channelSettings.kafkaMsgValuesUsedForFiltering.end() )
                        {
                            if ( (*v) == hdrValue )
                            {
                                isFiltered = true;
                                ++m_kafkaMessagesFiltered;

                                // A filtered message also counts as successfully handled
                                // As such we need to update the offset so that its taken into account for a later commit of said offsets
                                m_consumerOffsets[ message.partition() ] = message.offset();

                                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Udp2KafkaChannel:consume_cb: Filtered message on topic \"" +          
                                        m_channelSettings.channelTopicName + " with offset " + CORE::Int64ToString( message.offset() ) );    
                                break;
                            }
                            ++v;
                        }
                    }
                    if ( isFiltered )
                        break;
                }
            }

            if ( !isFiltered )
                UdpTransmit( message );
            break;
        }
        case RdKafka::ERR__PARTITION_EOF:
        {
            // Last message that was available has been read
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Kafka topic \"" + m_channelSettings.channelTopicName + "\" doesnt have any new messages waiting to be consumed" );
            break;
        }
        case RdKafka::ERR__UNKNOWN_PARTITION:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka consume error: UNKNOWN_PARTITION: " + message.errstr() );
            break;
        }
        case RdKafka::ERR__UNKNOWN_TOPIC:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka consume error: UNKNOWN_TOPIC: " + message.errstr() );
            break;
        }
        case RdKafka::ERR__MAX_POLL_EXCEEDED:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka consume error: MAX_POLL_EXCEEDED: " + message.errstr() );
            break;
        }
        default:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka consume error: " + message.errstr() );
            break;
        }
    } 
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::event_cb( RdKafka::Event& event )
{GUCEF_TRACE;

    switch ( event.type() )
    {
        case RdKafka::Event::EVENT_ERROR:
        {
            #ifdef GUCEF_DEBUG_MODE
            if ( RdKafka::ERR__PARTITION_EOF == event.err() )
            {
                // Last message that was available has been read
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Kafka topic \"" + m_channelSettings.channelTopicName + "\" doesnt have any new messages waiting to be consumed" );
                break;
            }
            #endif

            if ( RdKafka::ERR__RESOLVE == event.err() )
            {
                // Per GitHub comment from edenhill on Dec 18, 2018 for issue #2159:
                //  "It will re-resolve the address on each re-connect attempt, but it will not log equal sub-sequent errors."
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to resolve Kafka brokers with setting value \"" + m_channelSettings.kafkaBrokers + "\" for Kafka topic \"" + m_channelSettings.channelTopicName + "\". Wrong DNS?" );
                break;
            }

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka error: " + RdKafka::err2str( event.err() ) + " from event_cb()" );
            break;
        }
        case RdKafka::Event::EVENT_STATS:
        {
	        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Kafka stats: " + event.str() );
	        break;
        }
        case RdKafka::Event::EVENT_LOG:
        {
	        switch ( event.severity() )
            {
                case RdKafka::Event::EVENT_SEVERITY_ALERT:
                case RdKafka::Event::EVENT_SEVERITY_EMERG:
                case RdKafka::Event::EVENT_SEVERITY_CRITICAL:
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
                case RdKafka::Event::EVENT_SEVERITY_ERROR:
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
                case RdKafka::Event::EVENT_SEVERITY_WARNING:
                {
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
                case RdKafka::Event::EVENT_SEVERITY_INFO:
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
                case RdKafka::Event::EVENT_SEVERITY_DEBUG:
                {
                    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }

                case RdKafka::Event::EVENT_SEVERITY_NOTICE:
                default:
                {
                    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
            }
	        break;
        }
        case RdKafka::Event::EVENT_THROTTLE:
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Kafka Throttle event: throttled for " + CORE::Int32ToString( event.throttle_time() ) + "ms by broker " + 
                event.broker_name() + " with ID " + CORE::Int32ToString( event.broker_id() ) );
            break;
        }
        default:
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka event: " + CORE::Int32ToString( (CORE::Int32) event.type() ) + ", with error code " + RdKafka::err2str( event.err() ) );
	        break;
        }
    }
}

/*-------------------------------------------------------------------------*/

const std::string& 
Udp2KafkaChannel::MsgStatusToString( RdKafka::Message::Status status )
{GUCEF_TRACE;

    switch ( status )
    {
        case RdKafka::Message::Status::MSG_STATUS_NOT_PERSISTED:
        {
            static const std::string statusStr = "status:not_persisted";
            return statusStr;
        }
        case RdKafka::Message::Status::MSG_STATUS_POSSIBLY_PERSISTED:
        {
            static const std::string statusStr = "status:possibly_persisted";
            return statusStr;
        }
        case RdKafka::Message::Status::MSG_STATUS_PERSISTED:
        {
            static const std::string statusStr = "status:persisted";
            return statusStr;
        }
        default:
        {
            static const std::string statusStr = "<unknown status>";
            return statusStr;
        }
    }
}

/*-------------------------------------------------------------------------*/

void 
Udp2KafkaChannel::dr_cb( RdKafka::Message& message )
{GUCEF_TRACE;

    if ( message.err() ) 
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Kafka delivery report: error: " + message.errstr() + 
                                                ", on topic: " + message.topic_name() + 
                                                ", key: " + ( message.key() ? (*message.key()) : std::string( "NULL" ) ) + 
                                                ", payload size: " + CORE::ToString( message.len() ).STL_String() +
                                                ", msg has " + MsgStatusToString( message.status() ) );
    }
    else 
    {
        ++m_kafkaMsgsTransmitted;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Kafka delivery report: " + MsgStatusToString( message.status() ) + ": topic: " + message.topic_name() + 
                                                ", partition: " + CORE::ToString( message.partition() ).STL_String() +
                                                ", offset: " + CORE::ToString( message.offset() ).STL_String() +
                                                ", key: " + ( message.key() ? (*message.key()) : std::string( "NULL" ) ) + 
                                                ", payload size: " + CORE::ToString( message.len() ).STL_String() );
    }
}

/*-------------------------------------------------------------------------*/

RdKafka::ErrorCode
Udp2KafkaChannel::KafkaProduce( const COMCORE::CIPv4Address& sourceAddress ,
                                const CORE::CDynamicBuffer& udpPacket    )
{GUCEF_TRACE;

    RdKafka::Headers* headers = 0;
    if ( m_channelSettings.addProducerHostnameKafkaMsgHeader ||
         m_channelSettings.addUdpOriginKafkaMsgHeader         )
    {
        headers = RdKafka::Headers::create();

        if ( m_channelSettings.addUdpOriginKafkaMsgHeader )
            headers->add( KafkaMsgHeader_UdpOrigin, sourceAddress.AddressAndPortAsString() );

        if ( m_channelSettings.addProducerHostnameKafkaMsgHeader )
            headers->add( KafkaMsgHeader_ProducerHostname, m_producerHostname );
    }

    RdKafka::ErrorCode retCode = 
        m_kafkaProducer->dvcustom_produce( m_kafkaProducerTopic, 
                                           RdKafka::Topic::PARTITION_UA,
                                           RdKafka::Producer::RK_MSG_COPY,                       // <- Copy payload, tradeoff against blocking on kafka produce
                                           const_cast< void* >( udpPacket.GetConstBufferPtr() ), // <- MSG_COPY flag will cause buffer to be copied, const cast to avoid copying again
                                           (size_t)udpPacket.GetDataSize(), 
                                           NULL, 0,
                                           headers,
                                           NULL );
    
    if ( retCode != RdKafka::ERR_NO_ERROR )
    {
        // Headers are auto-deleted on success but not on failure!
        delete headers;
    }
    
    switch ( retCode )
    {
        case RdKafka::ERR_NO_ERROR:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:KafkaProduce: Successfully queued packet for transmission" );
            break;
        }
        case RdKafka::ERR__QUEUE_FULL:
        {
            // We dont treat queue full as an error metrics wise. This is an expected and handled scenario
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:KafkaProduce: transmission queue is full" );
            break;
        }
        default:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:KafkaProduce: Kafka error: " + RdKafka::err2str( retCode ) + " from kafkaProducer->produce()" );
            ++m_kafkaErrorReplies;
            break;
        }
    }

    return retCode;
}

/*-------------------------------------------------------------------------*/

void
Udp2KafkaChannel::OnUDPPacketsRecieved( CORE::CNotifier* notifier   ,
                                        const CORE::CEvent& eventID ,
                                        CORE::CICloneable* evenData )
{GUCEF_TRACE;

    if ( EChannelMode::KAFKA_PRODUCER != m_channelSettings.mode && EChannelMode::KAFKA_PRODUCER_AND_CONSUMER != m_channelSettings.mode )
    {
        // If we are not a producer we ignore incoming UDP data
        return;
    }

    COMCORE::CUDPSocket::UDPPacketsRecievedEventData* udpPacketData = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( evenData );
    if ( GUCEF_NULL != udpPacketData )
    {
        const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = udpPacketData->GetData();
        for ( CORE::UInt32 p=0; p<data.packetsReceived; ++p )
        {
            const CORE::CDynamicBuffer& udpPacketBuffer = data.packets[ p ].dataBuffer.GetData();
            const COMCORE::CIPv4Address& sourceAddress = data.packets[ p ].sourceAddress;

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel: UDP Socket received a packet from " + sourceAddress.AddressAndPortAsString() );
        
            if ( GUCEF_NULL != m_kafkaProducer )
            {
                RdKafka::ErrorCode retCode = RdKafka::ERR_NO_ERROR;
                while ( !m_kafkaMsgQueueOverflowQueue.empty() && ( retCode == RdKafka::ERR_NO_ERROR ) )
                {
                    const TPacketEntry& queuedUdpPacketEntry = m_kafkaMsgQueueOverflowQueue.front();
                    retCode = KafkaProduce( queuedUdpPacketEntry.sourceAddress, queuedUdpPacketEntry.dataBuffer.GetData() );
                    if ( retCode == RdKafka::ERR_NO_ERROR )
                    {
                        m_kafkaMsgQueueOverflowQueue.pop_front();
                    }
                    else
                        break;            
                }

                if ( retCode == RdKafka::ERR_NO_ERROR )
                {
                    retCode = retCode = KafkaProduce( sourceAddress, udpPacketBuffer );
                    switch ( retCode )
                    {
                        case RdKafka::ERR__QUEUE_FULL: 
                        {
                            m_kafkaMsgQueueOverflowQueue.push_back( data.packets[ p ] );
                            break;
                        }
                        case RdKafka::ERR_NO_ERROR: 
                        {
                            break;
                        }
                    }
                }
                else
                {
                    m_kafkaMsgQueueOverflowQueue.push_back( data.packets[ p ] );
                }
            }
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: UDP Socket has a data received event but no data was provided" );
    }
}

/*-------------------------------------------------------------------------*/

CORE::Int64
Udp2KafkaChannel::ConvertKafkaConsumerStartOffset( const CORE::CString& startOffsetDescription ,
                                                   CORE::Int32 partitionId                     ,
                                                   CORE::Int32 timeoutInMs                     )
{GUCEF_TRACE;

    CORE::CString testString = startOffsetDescription.Lowercase();
    if ( testString.IsNULLOrEmpty() )
    {
        testString = "stored"; 
    }

    if ( "beginning" == testString )
    {
        return ConvertKafkaConsumerStartOffset( RdKafka::Topic::OFFSET_BEGINNING, partitionId, timeoutInMs );
    }
    if ( "end" == testString )
    {
        return ConvertKafkaConsumerStartOffset( RdKafka::Topic::OFFSET_END, partitionId, timeoutInMs );
    }
    if ( "stored" == testString )
    {
        return ConvertKafkaConsumerStartOffset( RdKafka::Topic::OFFSET_STORED, partitionId, timeoutInMs );
    }

    return ConvertKafkaConsumerStartOffset( CORE::StringToInt64( testString ), partitionId, timeoutInMs );
}

/*-------------------------------------------------------------------------*/

CORE::Int64
Udp2KafkaChannel::ConvertKafkaConsumerStartOffset( CORE::Int64 startOffsetDescription ,
                                                   CORE::Int32 partitionId            ,
                                                   CORE::Int32 timeoutInMs            )
{GUCEF_TRACE;

    if ( RdKafka::Topic::OFFSET_BEGINNING == startOffsetDescription )
    {
        int64_t high = 0; int64_t low = 0;
        RdKafka::ErrorCode err = m_kafkaConsumer->query_watermark_offsets( m_channelSettings.channelTopicName, partitionId, &low, &high, timeoutInMs );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Converted offset description \"BEGINNING\" into offset " + CORE::Int64ToString( low ) + " for partition " + CORE::Int32ToString( partitionId ) );
            return low;
        }

        std::string errStr = RdKafka::err2str( err );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Failed to convert offset description \"BEGINNING\" into offset for partition " + 
                CORE::Int32ToString( partitionId ) + ". ErrorCode : " + errStr );
        return RdKafka::Topic::OFFSET_INVALID;
    }
    if ( RdKafka::Topic::OFFSET_END == startOffsetDescription )
    {
        int64_t high = 0; int64_t low = 0;
        RdKafka::ErrorCode err = m_kafkaConsumer->query_watermark_offsets( m_channelSettings.channelTopicName, partitionId, &low, &high, timeoutInMs );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Converted offset description \"END\" into offset " + CORE::Int64ToString( low ) + " for partition " + CORE::Int32ToString( partitionId ) );
            return high;
        }

        std::string errStr = RdKafka::err2str( err );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Failed to convert offset description \"END\" into offset for partition " + 
                CORE::Int32ToString( partitionId ) + ". ErrorCode : " + errStr );
        return RdKafka::Topic::OFFSET_INVALID;
    }
    if ( RdKafka::Topic::OFFSET_STORED == startOffsetDescription )
    {
        std::vector<RdKafka::TopicPartition*> partitions;
        RdKafka::ErrorCode err = m_kafkaConsumer->assignment( partitions );
        if ( RdKafka::ERR_NO_ERROR != err )
        {
            std::string errStr = RdKafka::err2str( err );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Failed to convert offset description \"STORED\" into offset for partition " + 
                    CORE::Int32ToString( partitionId ) + ". Cannot obtain partition assignment. ErrorCode : " + errStr );
            return RdKafka::Topic::OFFSET_INVALID;
        }
        
        err = m_kafkaConsumer->committed( partitions, timeoutInMs );
        if ( RdKafka::ERR_NO_ERROR != err )
        {
            std::string errStr = RdKafka::err2str( err );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Failed to convert offset description \"STORED\" into offset for partition " + 
                    CORE::Int32ToString( partitionId ) + ". Cannot obtain commited offsets . ErrorCode : " + errStr );
            return RdKafka::Topic::OFFSET_INVALID;
        }

        std::vector<RdKafka::TopicPartition*>::iterator i = partitions.begin(); 
        while ( i != partitions.end() )
        {
            if ( (*i)->partition() == partitionId )
            {
                CORE::Int64 storedOffset = (*i)->offset();
                GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Converted offset description \"STORED\" into offset " + 
                        CORE::Int64ToString( storedOffset ) + " for partition " + CORE::Int32ToString( partitionId ) );
                return storedOffset;
            }      
            ++i;
        }
        

        return RdKafka::Topic::OFFSET_STORED;
    }

    // Assume its already a numeric offset
    return startOffsetDescription;
}

/*-------------------------------------------------------------------------*/

CORE::CString
Udp2KafkaChannel::ConvertKafkaConsumerStartOffset( CORE::Int64 offset )
{GUCEF_TRACE;

    if ( RdKafka::Topic::OFFSET_STORED == offset )
        return "stored";
    if ( RdKafka::Topic::OFFSET_BEGINNING == offset )
        return "beginning";
    if ( RdKafka::Topic::OFFSET_END == offset )
        return "end";
    if ( RdKafka::Topic::OFFSET_INVALID == offset )
        return "invalid";

    return CORE::Int64ToString( offset );    
}

/*-------------------------------------------------------------------------*/

bool
Udp2KafkaChannel::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

	m_udpSocket = new GUCEF::COMCORE::CUDPSocket( GetPulseGenerator(), false );
    m_metricsTimer = new CORE::CTimer( GetPulseGenerator(), 1000 );
    m_metricsTimer->SetEnabled( m_channelSettings.collectMetrics );

    // Set the minimum number of cycles we will go full speed if a single cycle was not enough to handle
    // all the processing. This will cause a bypass of CPU yielding if/when the situation arises.
    // In such a case the thread will run at max speed for a least the below set nr of cycles.
    GetPulseGenerator()->RequestPulsesPerImmediatePulseRequest( 100 );

    RegisterEventHandlers();
            
    CORE::CString::StringVector kafkaBrokerList = m_channelSettings.kafkaBrokers.ParseElements( ',', false );
    CORE::CString::StringVector::iterator i = kafkaBrokerList.begin();
    while ( i != kafkaBrokerList.end() )
    {
        m_kafkaBrokers.push_back( COMCORE::CHostAddress( (*i) ) );
        ++i;
    }
    if ( m_kafkaBrokers.empty() )
        return false;

	if ( TChannelMode::KAFKA_PRODUCER == m_channelSettings.mode || TChannelMode::KAFKA_PRODUCER_AND_CONSUMER == m_channelSettings.mode )
    {
        std::string errStr;
        RdKafka::Conf* kafkaConf = RdKafka::Conf::create( RdKafka::Conf::CONF_GLOBAL );
        kafkaConf->set( "metadata.broker.list", m_channelSettings.kafkaBrokers, errStr );
	    kafkaConf->set( "event_cb", static_cast< RdKafka::EventCb* >( this ), errStr );
	    kafkaConf->set( "dr_cb", static_cast< RdKafka::DeliveryReportCb* >( this ), errStr );
        kafkaConf->set( "rebalance_cb", static_cast< RdKafka::RebalanceCb* >( this ), errStr );
        ChannelSettings::StringMap::iterator m = m_channelSettings.kafkaProducerGlobalConfigSettings.begin();
        while ( m != m_channelSettings.kafkaProducerGlobalConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConf->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka Producer global config entry \"" + 
                        (*m).first + "\"=\"" + (*m).second + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
        }
        delete m_kafkaProducerConf;
        m_kafkaProducerConf = kafkaConf;
        
        RdKafka::Conf* kafkaProducerTopicConfig = RdKafka::Conf::create( RdKafka::Conf::CONF_TOPIC );
        m = m_channelSettings.kafkaProducerTopicConfigSettings.begin();
        while ( m != m_channelSettings.kafkaProducerTopicConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaProducerTopicConfig->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka Producer topic config entry \"" + 
                        (*m).first + "\"=\"" + (*m).second + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
    
        }
        delete m_kafkaProducerTopicConf;
        m_kafkaProducerTopicConf = kafkaProducerTopicConfig;
        
        RdKafka::Producer* producer = RdKafka::Producer::create( m_kafkaProducerConf, errStr );
	    if ( producer == nullptr ) 
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to create Kafka producer for topic \"" + 
                m_channelSettings.channelTopicName + "\", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
	    }
        m_kafkaProducer = producer;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskStart: Successfully created Kafka producer for topic \"" + 
            m_channelSettings.channelTopicName + "\"" );
        
        RdKafka::Topic* topic = RdKafka::Topic::create( m_kafkaProducer, m_channelSettings.channelTopicName, m_kafkaProducerTopicConf, errStr );
	    if ( topic == nullptr ) 
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to obtain Kafka Producer Topic handle for topic \"" + 
                m_channelSettings.channelTopicName + "\", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
	    }
        m_kafkaProducerTopic = topic;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskStart: Successfully created Kafka Producer Topic handle for topic: " + m_channelSettings.channelTopicName );
    }

    if ( TChannelMode::KAFKA_CONSUMER == m_channelSettings.mode || TChannelMode::KAFKA_PRODUCER_AND_CONSUMER == m_channelSettings.mode )
    {
        std::string errStr;
        RdKafka::Conf* kafkaConf = RdKafka::Conf::create( RdKafka::Conf::CONF_GLOBAL );
        kafkaConf->set( "metadata.broker.list", m_channelSettings.kafkaBrokers, errStr );
	    kafkaConf->set( "event_cb", static_cast< RdKafka::EventCb* >( this ), errStr );
	    kafkaConf->set( "dr_cb", static_cast< RdKafka::DeliveryReportCb* >( this ), errStr );
        kafkaConf->set( "rebalance_cb", static_cast< RdKafka::RebalanceCb* >( this ), errStr );
        ChannelSettings::StringMap::iterator m = m_channelSettings.kafkaProducerGlobalConfigSettings.begin();
        while ( m != m_channelSettings.kafkaProducerGlobalConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConf->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka Producer global config entry \"" + 
                        (*m).first + "\"=\"" + (*m).second + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
        }
        delete m_kafkaConsumerConf;
        m_kafkaConsumerConf = kafkaConf;
       
        #ifdef GUCEF_DEBUG_MODE
        m_kafkaConsumerConf->set( "enable.partition.eof", "true", errStr );
        #endif

        RdKafka::Conf* kafkaConsumerTopicConfig = RdKafka::Conf::create( RdKafka::Conf::CONF_TOPIC );
        m = m_channelSettings.kafkaConsumerTopicConfigSettings.begin();
        while ( m != m_channelSettings.kafkaConsumerTopicConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConsumerTopicConfig->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka Producer topic config entry \"" + 
                        (*m).first + "\"=\"" + (*m).second + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
    
        }
        delete m_kafkaConsumerTopicConf;
        m_kafkaConsumerTopicConf = kafkaConsumerTopicConfig;
        
        CORE::CString testString = m_channelSettings.consumerModeStartOffset.Lowercase();
        if ( "beginning" == testString )
        {
            if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerTopicConf->set( "auto.offset.reset", "earliest", errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka consumer topic config's auto.offset.reset, error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
        }
        if ( "end" == testString || "stored" == testString )
        {
            if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerTopicConf->set( "auto.offset.reset", "latest", errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka consumer topic config's auto.offset.reset, error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
        }

        // We dont want the offsets managed by the RdKafka library because that can cause data gaps
        // We want to wait till we successfully transmitted the message on UDP before we commit to stating we processed it.
        // ie garanteed handling. While UDP itself is not reliable, that doesnt mean we should add to the problem here.
        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "enable.auto.commit", "false", errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka consumer global config's enable.auto.commit, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "enable.auto.offset.store", "false", errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka consumer global config's enable.auto.offset.store, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }

        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "default_topic_conf", m_kafkaConsumerTopicConf, errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka consumer default topic config, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "group.id", m_channelSettings.consumerModeGroupId, errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to set Kafka consumer group id to \"" + 
                m_channelSettings.consumerModeGroupId + "\", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
        RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create( m_kafkaConsumerConf, errStr );
	    if ( consumer == nullptr ) 
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to create Kafka consumer, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
	    }
        m_kafkaConsumer = consumer;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskStart: Successfully created Kafka consumer" );

        std::vector< std::string > topics;
        topics.push_back( m_channelSettings.channelTopicName );
        RdKafka::ErrorCode response = m_kafkaConsumer->subscribe( topics );
        if ( RdKafka::ERR_NO_ERROR != response ) 
        {
		    errStr = RdKafka::err2str( response );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to start Kafka Consumer for topic \"" + 
                m_channelSettings.channelTopicName + "\" at offset " + m_channelSettings.consumerModeStartOffset + ", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
    }

    m_udpSocket->SetMaxUpdatesPerCycle( 50 );
    m_udpSocket->SetAutoReOpenOnError( true );
    m_udpSocket->SetAllowBroadcast( true );
    if ( m_udpSocket->Open( m_channelSettings.udpInterface.GetFirstIPv4Address() ) )
    {
		GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskStart: Successfully opened UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel:OnTaskStart: Failed to open UDP socket on " + m_channelSettings.udpInterface.HostnameAndPortAsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
Udp2KafkaChannel::CommitConsumerOffsets( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_kafkaConsumer )
        return false;

    // Now commit the latest offsets
    std::vector<RdKafka::TopicPartition*> partitions;
    RdKafka::ErrorCode err = m_kafkaConsumer->assignment( partitions );
    if ( RdKafka::ERR_NO_ERROR == err )
    {
        // Match the current Topic objects with our simplistic bookkeeping and sync them
        std::vector<RdKafka::TopicPartition*>::iterator p = partitions.begin();
        while ( p != partitions.end() )
        {
            CORE::Int32 partitionId = (*p)->partition();
            TInt32ToInt64Map::iterator o = m_consumerOffsets.find( partitionId );
            if ( o != m_consumerOffsets.end() )
            {
                (*p)->set_offset( (*o).second );
            }
            ++p;
        }

        // Now we actually tell the client library locally about the new offsets
        err = m_kafkaConsumer->offsets_store( partitions );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            // Now we request to send the local offset bookkeeping to Kafka
            err = m_kafkaConsumer->commitAsync( partitions );
            if ( RdKafka::ERR_NO_ERROR == err )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel: Successfully triggered async commit of current offsets" );
                return true;
            }
            else
            {
                std::string errStr = RdKafka::err2str( err );
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Cannot commit consumer offsets: Failed to trigger async commit of current offets. ErrorCode : " + errStr );
                return false;
            }
        }
        else
        {
            std::string errStr = RdKafka::err2str( err );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Cannot commit consumer offsets: Failed to store current offets on local topic objects. ErrorCode : " + errStr );
            return false;
        }
    }
    else
    {
        std::string errStr = RdKafka::err2str( err );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2KafkaChannel: Cannot commit consumer offsets: Failed to obtain current partition assignment. ErrorCode : " + errStr );
        return false;
    }
}

/*-------------------------------------------------------------------------*/
    
bool
Udp2KafkaChannel::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;
                      
    // You are required to periodically call poll() on a producer to trigger queued callbacks if any
    if ( GUCEF_NULL != m_kafkaProducer )
    { 
        int i=0;
        for ( ; i<50; ++i )
        {
            CORE::Int32 opsServed = (CORE::Int32) m_kafkaProducer->poll( 0 );
            if ( 0 == opsServed )
            {
                break;
            }

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Udp2KafkaChannel:OnTaskCycle: poll() on the kafkaProducer served " + 
                    CORE::Int32ToString( opsServed ) + " events");
        }
        if ( i == 50 )
        {
            // We have more work to do. Make sure we dont go to sleep
            GetPulseGenerator()->RequestImmediatePulse();
        }
    }

    if ( GUCEF_NULL != m_kafkaConsumer )
    { 
        int i=0;
        for ( ; i<50; ++i )
        {
            RdKafka::Message* msg = m_kafkaConsumer->consume( 0 );
            int errState = msg->err();
            if ( RdKafka::ERR__TIMED_OUT == errState || RdKafka::ERR__PARTITION_EOF == errState )
            {
                delete msg;
                break;
            }
            
            consume_cb( *msg, GUCEF_NULL );
            delete msg;
        }
        if ( i == 50 )
        {
            // We have more work to do. Make sure we dont go to sleep
            GetPulseGenerator()->RequestImmediatePulse();
        } 

        // Periodically commit our offsets
        // This can slow things down so we dont want to do this too often
        if ( 5000 < GetPulseGenerator()->GetTimeSinceTickCountInMilliSecs( m_tickCountAtLastOffsetCommit ) )
        {
            if ( CommitConsumerOffsets() )
            {
                m_tickCountAtLastOffsetCommit = GetPulseGenerator()->GetTickCount();   
            }
        }
    }

    //if ( m_kafkaErrorReplies > 0 )
    //{
    //    HostAddressVector::iterator h = m_kafkaBrokers.begin();
    //    while ( h != m_kafkaBrokers.end() )
    //    {
    //        (*h).Refresh();
    //        ++h;
    //    }    
    //}

    // We are never 'done' so return false
    return false;
}

/*-------------------------------------------------------------------------*/
    
void
Udp2KafkaChannel::OnTaskEnded( CORE::CTaskPtr task ,
                               bool wasForced      )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskEnded" );

    if ( TChannelMode::KAFKA_PRODUCER == m_channelSettings.mode || TChannelMode::KAFKA_PRODUCER_AND_CONSUMER == m_channelSettings.mode )
    {
        // First stop the influx of new messages
        m_udpSocket->Close();

        // Push what we have left into the Kafka library's producer queue
        RdKafka::ErrorCode retCode = RdKafka::ERR_NO_ERROR;
        while ( !m_kafkaMsgQueueOverflowQueue.empty() && ( retCode == RdKafka::ERR_NO_ERROR ) )
        {
            TPacketEntry& queuedUdpPacketEntry = m_kafkaMsgQueueOverflowQueue.front();
            retCode = KafkaProduce( queuedUdpPacketEntry.sourceAddress, queuedUdpPacketEntry.dataBuffer.GetData() );
            if ( retCode == RdKafka::ERR_NO_ERROR )
            {
                m_kafkaMsgQueueOverflowQueue.pop_front();
            }
            else
                break;            
        }

        int waited = 0;
        int queuedMsgs = m_kafkaProducer->outq_len();
        while ( queuedMsgs > 0 && waited <= 30000 ) 
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskEnded: Waiting on Kafka Producer for topic: \"" + 
                m_channelSettings.channelTopicName + "\" to finish " + CORE::Int32ToString( queuedMsgs ) + " queued messages" );
            
            if ( RdKafka::ERR_NO_ERROR == m_kafkaProducer->flush( 1000 ) )
                break;

            waited += 1000;
            queuedMsgs = m_kafkaProducer->outq_len();
        }
        if ( waited > 30000 )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Udp2KafkaChannel:OnTaskEnded: Timed out waiting on Kafka Producer for topic: \"" + 
                m_channelSettings.channelTopicName + "\" to finish " + CORE::Int32ToString( queuedMsgs ) + " queued messages. Shutdown will continue regardless. Data loss will occur!" );
        }
        
        delete m_kafkaProducer;
        m_kafkaProducer = GUCEF_NULL;
        delete m_kafkaProducerTopic;
        m_kafkaProducerTopic = GUCEF_NULL;
        delete m_kafkaProducerTopicConf;
        m_kafkaProducerTopicConf =  GUCEF_NULL;
        delete m_kafkaProducerConf;
        m_kafkaProducerConf =  GUCEF_NULL;
    }

    if ( TChannelMode::KAFKA_CONSUMER == m_channelSettings.mode || TChannelMode::KAFKA_PRODUCER_AND_CONSUMER == m_channelSettings.mode )
    {
        // Stop the influx of new messages from Kafka
        m_kafkaConsumer->close();

        delete m_kafkaConsumer;
        m_kafkaConsumer = GUCEF_NULL;
        delete m_kafkaConsumerTopicConf;
        m_kafkaConsumerTopicConf =  GUCEF_NULL;
        delete m_kafkaConsumerConf;
        m_kafkaConsumerConf =  GUCEF_NULL;
    }

    delete m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;
    delete m_udpSocket;
    m_udpSocket = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2KafkaInfoResource::RestApiUdp2KafkaInfoResource( Udp2Kafka* app )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2KafkaInfoResource::~RestApiUdp2KafkaInfoResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiUdp2KafkaInfoResource::Serialize( const CORE::CString& resourcePath   ,
                                         CORE::CDataNode& output             ,
                                         const CORE::CString& representation ,
                                         const CORE::CString& params         )
{GUCEF_TRACE;

    output.SetName( "info" );
    output.SetAttribute( "application", "udp2kafka" );
    output.SetAttribute( "buildDateTime", __TIMESTAMP__ );
    #ifdef GUCEF_DEBUG_MODE
    output.SetAttribute( "isReleaseBuild", "false" );
    #else
    output.SetAttribute( "isReleaseBuild", "true" );
    #endif
    return true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2KafkaConfigResource::RestApiUdp2KafkaConfigResource( Udp2Kafka* app, bool appConfig )
    : WEB::CCodecBasedHTTPServerResource()
    , m_app( app )
    , m_appConfig( appConfig )
{GUCEF_TRACE;

    m_allowSerialize = true;
}

/*-------------------------------------------------------------------------*/

RestApiUdp2KafkaConfigResource::~RestApiUdp2KafkaConfigResource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
RestApiUdp2KafkaConfigResource::Serialize( const CORE::CString& resourcePath   ,
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

Udp2Kafka::Udp2Kafka( void )
    : CORE::CObserver()
    , m_udpStartPort( 20000 )
    , m_channelCount( 1 )
    , m_kafkaTopicStartChannelID( 0 )
    , m_kafkaTopicName( "udp-ingress-ch{channelID}" )
    , m_channels()
    , m_channelSettings()
    , m_httpServer()
    , m_httpRouter()
    , m_appConfig()
    , m_globalConfig()
    , m_metricsTimer()
    , m_transmitMetrics( false )
    , m_testUdpSocket( false )
    , m_testPacketTransmitTimer()
{GUCEF_TRACE;

    TEventCallback callback1( this, &Udp2Kafka::OnMetricsTimerCycle );
    SubscribeTo( &m_metricsTimer                ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback1                      );

    TEventCallback callback2( this, &Udp2Kafka::OnTransmitTestPacketTimerCycle );
    SubscribeTo( &m_testPacketTransmitTimer     ,
                 CORE::CTimer::TimerUpdateEvent ,
                 callback2                      );    
}

/*-------------------------------------------------------------------------*/

Udp2Kafka::~Udp2Kafka()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
Udp2Kafka::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::Int32 channelId = m_kafkaTopicStartChannelID;
    Udp2KafkaChannelVector::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        const Udp2KafkaChannel::ChannelMetrics& metrics = (*i)->GetMetrics();
        const Udp2KafkaChannel::ChannelSettings& settings = (*i)->GetChannelSettings();

        GUCEF_METRIC_COUNT( settings.metricsPrefix + "kafkaErrorReplies", metrics.kafkaErrorReplies, 1.0f );

        if ( Udp2KafkaChannel::EChannelMode::KAFKA_PRODUCER == settings.mode || Udp2KafkaChannel::EChannelMode::KAFKA_PRODUCER_AND_CONSUMER == settings.mode )
        {
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "kafkaMessagesTransmitted", metrics.kafkaMessagesTransmitted, 1.0f );
            GUCEF_METRIC_GAUGE( settings.metricsPrefix + "kafkaTransmitQueueSize", metrics.kafkaTransmitQueueSize, 1.0f );
            GUCEF_METRIC_GAUGE( settings.metricsPrefix + "kafkaTransmitOverflowQueueSize", metrics.kafkaTransmitOverflowQueueSize, 1.0f );
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "udpBytesReceived", metrics.udpBytesReceived, 1.0f );
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "udpMessagesReceived", metrics.udpMessagesReceived, 1.0f );
        }
        if ( Udp2KafkaChannel::EChannelMode::KAFKA_CONSUMER == settings.mode || Udp2KafkaChannel::EChannelMode::KAFKA_PRODUCER_AND_CONSUMER == settings.mode )
        {
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "kafkaMessagesReceived", metrics.kafkaMessagesReceived, 1.0f );
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "kafkaMessagesFiltered", metrics.kafkaMessagesFiltered, 1.0f );
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "udpBytesTransmitted", metrics.udpBytesTransmitted, 1.0f );
            GUCEF_METRIC_COUNT( settings.metricsPrefix + "udpMessagesTransmitted", metrics.udpMessagesTransmitted, 1.0f );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
Udp2Kafka::OnTransmitTestPacketTimerCycle( CORE::CNotifier* notifier    ,
                                           const CORE::CEvent& eventId  ,
                                           CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_testUdpSocket.IsActive() )
        if ( !m_testUdpSocket.Open() )
            return;
    
    Udp2KafkaChannelVector::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        const Udp2KafkaChannel::ChannelSettings& settings = (*i)->GetChannelSettings();
        if ( settings.wantsTestPackage )
        {
            m_testUdpSocket.SendPacketTo( settings.udpInterface.GetFirstIPv4Address(), "TEST", 4 );
        }
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
Udp2Kafka::Start( void )
{GUCEF_TRACE;

    bool errorOccured = false;
    m_channels.resize( m_channelCount );
    for ( size_t m=0; m<m_channels.size(); ++m )
    {
        m_channels[ m ] = Udp2KafkaChannelPtr( new Udp2KafkaChannel() );
    }

    CORE::ThreadPoolPtr threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetThreadPool();

    CORE::UInt16 udpPort = m_udpStartPort;
    CORE::Int32 channelId = m_kafkaTopicStartChannelID;
    Udp2KafkaChannelVector::iterator i = m_channels.begin();
    while ( i != m_channels.end() )
    {
        Udp2KafkaChannelPtr& channel = (*i);
        ChannelSettingsMap::iterator n = m_channelSettings.find( channelId );
        if ( n != m_channelSettings.end() )
        {
            const Udp2KafkaChannel::ChannelSettings& channelSettings = (*n).second;
            if ( !channel->LoadConfig( channelSettings ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2Kafka:Start: Failed to set channel settings on channel " + CORE::Int32ToString( channelId ) );
                errorOccured = true;
                break;
            }

            if ( !threadPool->StartTask( channel ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2Kafka:Start: Failed to start task (dedicated thread) for channel " + CORE::Int32ToString( channelId ) );
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
        GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "Udp2Kafka: Opening REST API" );
        return m_httpServer.Listen();
    }
    return errorOccured;
}

/*-------------------------------------------------------------------------*/

bool 
Udp2Kafka::LoadConfig( const CORE::CValueList& appConfig   ,
                       const CORE::CDataNode& globalConfig )
{GUCEF_TRACE;

    m_transmitMetrics = CORE::StringToBool( appConfig.GetValueAlways( "TransmitMetrics", "true" ) );
    
    m_udpStartPort = CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "UdpStartPort", "20000" ) ) );
    m_channelCount = CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "ChannelCount", "1" ) ) );
    m_kafkaTopicStartChannelID = CORE::StringToInt32( CORE::ResolveVars(  appConfig.GetValueAlways( "KafkaTopicStartChannelID", "1" ) ) );
    m_kafkaTopicName = CORE::ResolveVars( appConfig.GetValueAlways( "KafkaTopicName", "udp-ingress-ch{channelID}" ) );
    CORE::CString kafkaBrokers = CORE::ResolveVars( appConfig.GetValueAlways( "KafkaBrokers", "127.0.0.1:9092" ) );

    Udp2KafkaChannel::ChannelSettings::HostAddressVector consumerModeUdpDestinations;
    CORE::CValueList::TVariantVector settingValues = appConfig.GetValueVectorAlways( "ConsumerMode.UdpDestinations" );
    CORE::CValueList::TVariantVector::iterator n = settingValues.begin();
    while ( n != settingValues.end() )
    {
        CORE::CString settingValue = CORE::ResolveVars( (*n) );
        CORE::CString::StringVector splitValues = settingValue.ParseElements( ',', false );
        CORE::CString::StringVector::iterator m = splitValues.begin();
        while ( m != splitValues.end() )
        {
            COMCORE::CHostAddress udpDest( (*m) );
            consumerModeUdpDestinations.push_back( udpDest );
            ++m;
        }
        ++n;
    }

    CORE::UInt16 udpPort = m_udpStartPort;
    CORE::Int32 maxChannelId = m_kafkaTopicStartChannelID + m_channelCount;
    for ( CORE::Int32 channelId = m_kafkaTopicStartChannelID; channelId < maxChannelId; ++channelId )
    {
        Udp2KafkaChannel::ChannelSettings& channelSettings = m_channelSettings[ channelId ];

        channelSettings.kafkaBrokers = kafkaBrokers;
        channelSettings.collectMetrics = m_transmitMetrics;

        CORE::CString settingPrefix = "ChannelSetting." + CORE::Int32ToString( channelId );
        
        CORE::CString settingName = settingPrefix + ".KafkaTopicName";
        CORE::CString settingValue = appConfig.GetValueAlways( settingName ).AsString();
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.channelTopicName = CORE::ResolveVars( settingValue.ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) ) );
        }
        else
        {
            // Use the auto naming and numbering scheme based on a single template name instead
            channelSettings.channelTopicName = CORE::ResolveVars( m_kafkaTopicName.ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) ) );
        }

        settingName = settingPrefix + ".UdpInterface";
        settingValue = CORE::ResolveVars( appConfig.GetValueAlways( settingName ) );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            if ( settingValue.HasChar( ':' ) )
            {
                channelSettings.udpInterface.SetHostnameAndPort( settingValue );
            }
            else
            {
                channelSettings.udpInterface.SetHostname( settingValue );
            }
        }
        else
        {
            // Use the auto numbering scheme instead
            channelSettings.udpInterface.SetPortInHostByteOrder( udpPort );
        }

        // Allow overriding the specific port for a specific channel
        settingName = settingPrefix + ".UdpPort";
        settingValue = CORE::ResolveVars( appConfig.GetValueAlways( settingName ) );
        if ( !settingValue.IsNULLOrEmpty() )
        {
            channelSettings.udpInterface.SetPortInHostByteOrder( CORE::StringToUInt16( settingValue ) );    
        }

        settingName = settingPrefix + ".WantsTestPackage";
        channelSettings.wantsTestPackage = appConfig.GetValueAlways( settingName ).AsBool( channelSettings.wantsTestPackage );

        settingName = settingPrefix + ".MetricsPrefix";
        channelSettings.metricsPrefix = appConfig.GetValueAlways( settingName, "udp2kafka.ch{channelID}." ).AsString( CORE::CString::Empty );
        channelSettings.metricsPrefix = channelSettings.metricsPrefix.ReplaceSubstr( "{channelID}", CORE::Int32ToString( channelId ) );
        channelSettings.metricsPrefix = channelSettings.metricsPrefix.ReplaceSubstr( "{topicName}", channelSettings.channelTopicName );

        settingName = settingPrefix + ".Multicast.Join";
        CORE::CValueList::TVariantVector settingValues = appConfig.GetValueVectorAlways( settingName, ',' );
        CORE::CValueList::TVariantVector::iterator n = settingValues.begin();
        while ( n != settingValues.end() )
        {
            CORE::CString settingValue = (*n).AsString( CORE::CString::Empty, true );
            COMCORE::CHostAddress multicastAddress( settingValue );
            channelSettings.udpMulticastToJoin.push_back( multicastAddress );
            ++n;
        }

        settingName = settingPrefix + ".Mode";
        settingValue = CORE::ResolveVars( appConfig.GetValueAlways( settingName, "producer" ) ).Lowercase();
        if ( settingValue == "consumer" )
        {
            channelSettings.mode = Udp2KafkaChannel::EChannelMode::KAFKA_CONSUMER;
        }
        else
        if ( settingValue == "both" || settingValue == "producer_and_consumer" )
        {
            channelSettings.mode = Udp2KafkaChannel::EChannelMode::KAFKA_PRODUCER_AND_CONSUMER;
        }
        else
        {
            channelSettings.mode = Udp2KafkaChannel::EChannelMode::KAFKA_PRODUCER;
        }

        settingName = settingPrefix + ".ConsumerMode.StartOffset";
        channelSettings.consumerModeStartOffset = CORE::ResolveVars( appConfig.GetValueAlways( settingName, "stored" ) ).Lowercase();

        settingName = settingPrefix + ".ConsumerMode.GroupID";
        channelSettings.consumerModeGroupId = CORE::ResolveVars( appConfig.GetValueAlways( settingName, "0" ) );

        channelSettings.consumerModeUdpDestinations = consumerModeUdpDestinations;
        settingName = settingPrefix + ".ConsumerMode.UdpDestinations";
        settingValues = appConfig.GetValueVectorAlways( settingName, ',' );
        n = settingValues.begin();
        while ( n != settingValues.end() )
        {
            CORE::CString settingValue = CORE::ResolveVars( (*n) );
            COMCORE::CHostAddress udpDest( settingValue );
            channelSettings.consumerModeUdpDestinations.push_back( udpDest );
            ++n;
        }

        settingName = settingPrefix + ".KafkaProducerGlobalConfig.";
        CORE::CValueList::TStringVector kafkaGlobalConfigKeys = appConfig.GetKeysWithWildcardKeyMatch( settingName + '*' );
        CORE::CValueList::TStringVector::iterator m = kafkaGlobalConfigKeys.begin();
        while ( m != kafkaGlobalConfigKeys.end() )
        {
            CORE::CString kafkaGlobalConfigSettingKey = (*m).CutChars( settingName.Length(), true, 0 );
            CORE::CString kafkaGlobalConfigSettingValue = appConfig.GetValueAlways( (*m) );
            if ( !kafkaGlobalConfigSettingKey.IsNULLOrEmpty() && !kafkaGlobalConfigSettingValue.IsNULLOrEmpty() )
            {
                channelSettings.kafkaProducerGlobalConfigSettings[ kafkaGlobalConfigSettingKey ] = kafkaGlobalConfigSettingValue;
            }
            ++m;
        }
        settingName = settingPrefix + ".KafkaConsumerGlobalConfig.";
        kafkaGlobalConfigKeys = appConfig.GetKeysWithWildcardKeyMatch( settingName + '*' );
        m = kafkaGlobalConfigKeys.begin();
        while ( m != kafkaGlobalConfigKeys.end() )
        {
            CORE::CString kafkaGlobalConfigSettingKey = (*m).CutChars( settingName.Length(), true, 0 );
            CORE::CString kafkaGlobalConfigSettingValue = appConfig.GetValueAlways( (*m) );
            if ( !kafkaGlobalConfigSettingKey.IsNULLOrEmpty() && !kafkaGlobalConfigSettingValue.IsNULLOrEmpty() )
            {
                channelSettings.kafkaConsumerGlobalConfigSettings[ kafkaGlobalConfigSettingKey ] = kafkaGlobalConfigSettingValue;
            }
            ++m;
        }

        settingName = settingPrefix + ".KafkaProducerTopicConfig.";
        CORE::CValueList::TStringVector kafkaTopicConfigKeys = appConfig.GetKeysWithWildcardKeyMatch( settingName + '*' );
        m = kafkaTopicConfigKeys.begin();
        while ( m != kafkaTopicConfigKeys.end() )
        {
            CORE::CString kafkaTopicConfigSettingKey = (*m).CutChars( settingName.Length(), true, 0 );
            CORE::CString kafkaTopicConfigSettingValue = appConfig.GetValueAlways( (*m) );
            if ( !kafkaTopicConfigSettingKey.IsNULLOrEmpty() && !kafkaTopicConfigSettingValue.IsNULLOrEmpty() )
            {
                channelSettings.kafkaProducerTopicConfigSettings[ kafkaTopicConfigSettingKey ] = kafkaTopicConfigSettingValue;
            }
            ++m;
        }
        settingName = settingPrefix + ".KafkaConsumerTopicConfig.";
        kafkaTopicConfigKeys = appConfig.GetKeysWithWildcardKeyMatch( settingName + '*' );
        m = kafkaTopicConfigKeys.begin();
        while ( m != kafkaTopicConfigKeys.end() )
        {
            CORE::CString kafkaTopicConfigSettingKey = (*m).CutChars( settingName.Length(), true, 0 );
            CORE::CString kafkaTopicConfigSettingValue = appConfig.GetValueAlways( (*n) );
            if ( !kafkaTopicConfigSettingKey.IsNULLOrEmpty() && !kafkaTopicConfigSettingValue.IsNULLOrEmpty() )
            {
                channelSettings.kafkaConsumerTopicConfigSettings[ kafkaTopicConfigSettingKey ] = kafkaTopicConfigSettingValue;
            }
            ++m;
        }

        settingName = settingPrefix + ".KafkaMsgHeaderFilterValues";
        channelSettings.kafkaMsgValuesUsedForFiltering = appConfig.GetValueVectorAlways( settingName, ',', channelSettings.kafkaMsgValuesUsedForFiltering );

        ++udpPort;
    }

    m_appConfig = appConfig;
    m_globalConfig.Copy( globalConfig );

    m_httpServer.SetPort( CORE::StringToUInt16( CORE::ResolveVars( appConfig.GetValueAlways( "RestApiPort", "10000" ) ) ) );

    m_httpRouter.SetResourceMapping( "/info", RestApiUdp2KafkaInfoResource::THTTPServerResourcePtr( new RestApiUdp2KafkaInfoResource( this ) )  );
    m_httpRouter.SetResourceMapping( "/config/appargs", RestApiUdp2KafkaConfigResource::THTTPServerResourcePtr( new RestApiUdp2KafkaConfigResource( this, true ) )  );
    m_httpRouter.SetResourceMapping( "/config", RestApiUdp2KafkaConfigResource::THTTPServerResourcePtr( new RestApiUdp2KafkaConfigResource( this, false ) )  );
    m_httpRouter.SetResourceMapping(  CORE::ResolveVars( appConfig.GetValueAlways( "RestBasicHealthUri", "/health/basic" ) ), RestApiUdp2KafkaInfoResource::THTTPServerResourcePtr( new WEB::CDummyHTTPServerResource() )  );

    m_httpServer.GetRouterController()->AddRouterMapping( &m_httpRouter, "", "" );

    m_testPacketTransmitTimer.SetInterval( CORE::StringToUInt32( appConfig.GetValueAlways( "TestPacketTransmissionIntervalInMs", "1000" ) ) );
    m_testPacketTransmitTimer.SetEnabled( CORE::StringToBool( appConfig.GetValueAlways( "TransmitTestPackets", "false" ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CValueList&
Udp2Kafka::GetAppConfig( void ) const
{
    return m_appConfig;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataNode&
Udp2Kafka::GetGlobalConfig( void ) const
{
    return m_globalConfig;
}

/*-------------------------------------------------------------------------*/


