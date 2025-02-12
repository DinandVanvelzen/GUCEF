/*
 *  pubsubpluginKAFKA: Generic GUCEF COMCORE plugin for providing pubsub via Kafka
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

#include <string.h>
#include <math.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H */

#ifndef GUCEF_CORE_METRICSMACROS_H
#include "gucefCORE_MetricsMacros.h"
#define GUCEF_CORE_METRICSMACROS_H
#endif /* GUCEF_CORE_METRICSMACROS_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H */

#ifndef PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENT_H
#include "pubsubpluginKAFKA_CKafkaPubSubClient.h"
#define PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENT_H ? */

#include "pubsubpluginKAFKA_CKafkaPubSubClientTopic.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace KAFKA {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const std::string CKafkaPubSubClientTopic::KafkaMsgHeader_ProducerHostname = "ProducerHostname";
const CORE::CString CKafkaPubSubClientTopic::DefaultOffsetResetValue = "beginning";

/*-------------------------------------------------------------------------*/

CKafkaPubSubClientTopic::CKafkaPubSubClientTopic( CKafkaPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CKafkaPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_metricsTimer( GUCEF_NULL )
    , m_pubsubMsgs()
    , m_pubsubMsgsRefs()
    , m_pubsubMsgAttribs()
    , m_rdKafkaMsgs()
    , m_config()
    , m_kafkaProducerTopicConf( GUCEF_NULL )
    , m_kafkaConsumerTopicConf( GUCEF_NULL )
    , m_kafkaProducerConf( GUCEF_NULL )
    , m_kafkaConsumerConf( GUCEF_NULL )
    , m_kafkaProducer( GUCEF_NULL )
    , m_kafkaProducerTopic( GUCEF_NULL )
    , m_kafkaConsumer( GUCEF_NULL )
    , m_kafkaCommitedConsumerOffsets()
    , m_kafkaErrorReplies( 0 )
    , m_kafkaConnectionErrors( 0 )
    , m_kafkaMsgsTransmitted( 0 )
    , m_kafkaMessagesReceived( 0 )
    , m_kafkaMessagesFiltered( 0 )
    , m_producerHostname( CORE::GetHostname() )
    , m_firstPartitionAssignment( true )
    , m_consumerReadOffsets()
    , m_consumerAckdOffsets()
    , m_receivedMsgAcks()
    , m_tickCountAtLastOffsetCommit( 0 )
    , m_tickCountAtConsumeDelayRequest( 0 )
    , m_requestedConsumeDelayInMs( 0 )
    , m_msgsAckdSinceLastOffsetCommit( false )
    , m_consumerOffsetWaitsForExplicitMsgAck( false )
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
    , m_maxTotalMsgsInFlight( 1000 )
    , m_metrics()
    , m_shouldBeConnected( false )
    , m_isSubscribed( false )
    , m_isSubscriptionAtEndOfData( false )
    , m_isHealthy( true )
    , m_lock()
{GUCEF_TRACE;

    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CKafkaPubSubClientTopic::~CKafkaPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_client = GUCEF_NULL;

    Disconnect();
    Clear();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CKafkaPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    GUCEF_DELETE m_kafkaProducerConf;
    m_kafkaProducerConf = GUCEF_NULL;

    GUCEF_DELETE m_kafkaConsumerConf;
    m_kafkaConsumerConf = GUCEF_NULL;

    GUCEF_DELETE m_kafkaProducerTopicConf;
    m_kafkaProducerTopicConf = GUCEF_NULL;

    GUCEF_DELETE m_kafkaConsumerTopicConf;
    m_kafkaConsumerTopicConf = GUCEF_NULL;

    GUCEF_DELETE m_kafkaProducerTopic;
    m_kafkaProducerTopic = GUCEF_NULL;

    GUCEF_DELETE m_kafkaProducer;
    m_kafkaProducer = GUCEF_NULL;

    GUCEF_DELETE m_kafkaConsumer;
    m_kafkaConsumer = GUCEF_NULL;

    m_currentPublishActionId = 1;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_metricsTimer )
    {
        TEventCallback callback( this, &CKafkaPubSubClientTopic::OnMetricsTimerCycle );
        SubscribeTo( m_metricsTimer                 ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }

    TEventCallback callback2( this, &CKafkaPubSubClientTopic::OnPulseCycle );
    SubscribeTo( m_client->GetConfig().pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent                       ,
                 callback2                                               );
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needSubscribeSupport;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKafkaPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify )
{GUCEF_TRACE;

    RdKafka::ErrorCode retCode = RdKafka::ERR_NO_ERROR;

    MT::CScopeMutex lock( m_lock );

    if ( 0 == publishActionId )
    {
        publishActionId = m_currentPublishActionId;
        ++m_currentPublishActionId;
    }

    // First write any backend auto generated headers per config
    RdKafka::Headers* headers = GUCEF_NULL;
    if ( m_config.addProducerHostnameAsKafkaMsgHeader )
    {
        headers = RdKafka::Headers::create();
        retCode = headers->add( KafkaMsgHeader_ProducerHostname, m_producerHostname );
        if ( retCode != RdKafka::ERR_NO_ERROR )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Publish: Failed to add auto generated Producer Hostname Kafka Msg header: " + RdKafka::err2str( retCode ) +
                ". publishActionId=" + CORE::ToString( publishActionId ).STL_String() + ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ).STL_String() );
        }
    }

    // Next we add the message's meta-data key-value headers if any
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& metaDataKvPairs = msg.GetMetaDataKeyValuePairs();
    if ( !metaDataKvPairs.empty() )
    {
        if ( GUCEF_NULL == headers )
            headers = RdKafka::Headers::create();

        if ( m_config.addReadyToTransmitTimeStampAsKafkaMetaDataMsgHeader )
        {
            CORE::CDateTime nowDt = CORE::CDateTime::NowUTCDateTime();
            retCode = headers->add( m_config.prefixToAddForMetaDataKvPairs + m_config.readyToTransmitTimeStampMetaDataKey, CORE::ToString( nowDt.ToUnixEpochBasedTicksInMillisecs() ) );
            if ( retCode != RdKafka::ERR_NO_ERROR )
            {
                GUCEF_ERROR_LOG(CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Publish: Failed to add auto generated ReadyToTransmitTimeStamp Kafka Msg header: " + CORE::CString( RdKafka::err2str( retCode ) ) +
                    ". publishActionId=" + CORE::ToString( publishActionId ) + ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );
            }
        }

        PUBSUB::CIPubSubMsg::TKeyValuePairs::const_iterator k = metaDataKvPairs.begin();
        while ( k != metaDataKvPairs.end() )
        {
            retCode = headers->add( m_config.prefixToAddForMetaDataKvPairs + (*k).first.AsString(), (*k).second.AsVoidPtr(), (*k).second.ByteSize() );
            if ( retCode != RdKafka::ERR_NO_ERROR )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Publish: Failed to add msg meta-data key \"" + m_config.prefixToAddForMetaDataKvPairs + (*k).first.AsString() +
                    "\" as Kafka Msg header: " + RdKafka::err2str( retCode ) + ". Value=" + (*k).second.AsString() +
                    ". publishActionId=" + CORE::ToString( publishActionId ) + ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );
                return false;
            }
            ++k;
        }
    }

    // Next we add the message's key-value headers if any
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();
    if ( !kvPairs.empty() )
    {
        if ( GUCEF_NULL == headers )
            headers = RdKafka::Headers::create();

        PUBSUB::CIPubSubMsg::TKeyValuePairs::const_iterator k = kvPairs.begin();
        while ( k != kvPairs.end() )
        {
            retCode = headers->add( m_config.prefixToAddForKvPairs + (*k).first.AsString(), (*k).second.AsVoidPtr(), (*k).second.ByteSize() );
            if ( retCode != RdKafka::ERR_NO_ERROR )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Publish: Failed to add msg key-value entry \"" + m_config.prefixToAddForKvPairs + (*k).first.AsString() +
                        "\" as Kafka Msg header: " + RdKafka::err2str( retCode ) + ". Value=" + (*k).second.AsString() +
                        ". publishActionId=" + CORE::ToString( publishActionId ) + ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );
                return false;
            }
            ++k;
        }
    }

    void* msg_opaque = GUCEF_NULL;
    if ( notify )
    {
        // We encode the publish action Id into the passthrough opaque void*
        // This saves us from having to perform another memory allocation
        #ifdef GUCEF_64BIT
        msg_opaque = reinterpret_cast< void* >( publishActionId );
        #else
        if ( publishActionId > GUCEF_MT_UINT32MAX )
            msg_opaque = reinterpret_cast< void* >( publishActionId - GUCEF_MT_UINT32MAX );
        else
            msg_opaque = reinterpret_cast< void* >( static_cast< CORE::UInt32 >( publishActionId ) );
        #endif
    }

    retCode =
        m_kafkaProducer->dvcustom_produce( m_kafkaProducerTopic,
                                           RdKafka::Topic::PARTITION_UA,                               // <- use configured partitioner to keep it config driven
                                           RdKafka::Producer::RK_MSG_COPY,                             // <- Copy payload, tradeoff against blocking on kafka produce
                                           const_cast< void* >( msg.GetPrimaryPayload().AsVoidPtr() ), // <- MSG_COPY flag will cause buffer to be copied, const cast to avoid copying again due to bad constness on library API
                                           (size_t) msg.GetPrimaryPayload().ByteSize( false ),
                                           const_cast< void* >( msg.GetMsgId().AsVoidPtr() ),
                                           (size_t) msg.GetMsgId().ByteSize( false ),
                                           headers,
                                           msg_opaque );

    if ( retCode != RdKafka::ERR_NO_ERROR )
    {
        // Headers are auto-deleted on success but not on failure!
        GUCEF_DELETE headers;
    }

    switch ( retCode )
    {
        case RdKafka::ERR_NO_ERROR:
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Publish: Successfully queued message for transmission with publishActionId " + CORE::ToString( publishActionId ) +
                    ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );
            return true;
        }
        case RdKafka::ERR__QUEUE_FULL:
        {
            // We dont treat queue full as an error metrics wise. This is an expected and handled scenario
            ++m_metrics.kafkaTransmitOverflowQueueSize;
            m_metrics.kafkaTransmitQueueSize = (CORE::UInt32) m_kafkaProducer->outq_len();

            // We will not be getting a delivery report callback on this message so we add it here
            // to messages for which a failure needs to be notified
            m_publishFailureActionIds.push_back( publishActionId );

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Publish: transmission queue is full. publishActionId=" + CORE::ToString( publishActionId ) +
                    ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ) );

            NotifyObservers( LocalPublishQueueFullEvent );
            return false;
        }
        default:
        {
            // We will not be getting a delivery report callback on this message so we add it here
            // to messages for which a failure needs to be notified
            m_publishFailureActionIds.push_back( publishActionId );

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:Publish: Kafka error: " + RdKafka::err2str( retCode ) +
                    " from kafkaProducer->produce(). publishActionId=" + CORE::ToString( publishActionId ).STL_String() +
                    ". receiveActionId=" + CORE::ToString( msg.GetReceiveActionId() ).STL_String() );
            ++m_kafkaErrorReplies;
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    Clear();
    m_config = config;
    return SetupBasedOnConfig();
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::LoadConfig( const CKafkaPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    Clear();
    m_config = config;
    return SetupBasedOnConfig();
}

/*-------------------------------------------------------------------------*/

int
CKafkaPubSubClientTopic::RdKafkaStatsCallback( rd_kafka_t *rk  ,
                                               char *json      ,
                                               size_t json_len ,
						                       void *opaque    )
{GUCEF_TRACE;

    return 0;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::RdKafkaLogCallback( const rd_kafka_t *rk ,
                                             int level            ,
                                             const char *fac      ,
                                             const char *buf      )
{GUCEF_TRACE;

    // When using this approach to getting logs from RdKafka note that this callback is called
    // directly from various internal RdKafka threads

	switch ( level )
    {
        case RdKafka::Event::EVENT_SEVERITY_ALERT:
        case RdKafka::Event::EVENT_SEVERITY_EMERG:
        case RdKafka::Event::EVENT_SEVERITY_CRITICAL:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Kafka log: " + CORE::ToString( fac ) + " : " + CORE::ToString( buf ) );
            break;
        }
        case RdKafka::Event::EVENT_SEVERITY_ERROR:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + CORE::ToString( fac ) + " : " + CORE::ToString( buf ) );
            break;
        }
        case RdKafka::Event::EVENT_SEVERITY_WARNING:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + CORE::ToString( fac ) + " : " + CORE::ToString( buf ) );
            break;
        }
        case RdKafka::Event::EVENT_SEVERITY_INFO:
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + CORE::ToString( fac ) + " : " + CORE::ToString( buf ) );
            break;
        }
        case RdKafka::Event::EVENT_SEVERITY_DEBUG:
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Kafka log: " + CORE::ToString( fac ) + " : " + CORE::ToString( buf ) );
            break;
        }

        case RdKafka::Event::EVENT_SEVERITY_NOTICE:
        default:
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka log: " + CORE::ToString( fac ) + " : " + CORE::ToString( buf ) );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::SetupBasedOnConfig( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return false;

   const CKafkaPubSubClientConfig& clientConfig = m_client->GetConfig();

    if ( clientConfig.desiredFeatures.supportsMetrics )
    {
        if ( GUCEF_NULL == m_metricsTimer )
            m_metricsTimer = GUCEF_NEW CORE::CTimer( clientConfig.pulseGenerator, 1000 );
        m_metricsTimer->SetEnabled( clientConfig.desiredFeatures.supportsMetrics );
    }

    RegisterEventHandlers();

    // The library wants the addresses as a csv list on its config obj
    // we convert and prep as such
    CORE::CString csvKafkaBrokerList;
    PUBSUB::CPubSubClientConfig::THostAddressVector::const_iterator h = clientConfig.remoteAddresses.begin();
    while ( h != clientConfig.remoteAddresses.end() )
    {
        // The RdKafka library will re-resolve DNSs on reconnects so we should feed it DNSs
        csvKafkaBrokerList += (*h).HostnameAndPortAsString() + ',';
        ++h;
    }

	if ( clientConfig.desiredFeatures.supportsPublishing )
    {
        std::string errStr;

        RdKafka::Conf* kafkaConf = RdKafka::Conf::create( RdKafka::Conf::CONF_GLOBAL );
        kafkaConf->set( "metadata.broker.list", csvKafkaBrokerList, errStr );
	    kafkaConf->set( "event_cb", static_cast< RdKafka::EventCb* >( this ), errStr );
	    kafkaConf->set( "dr_cb", static_cast< RdKafka::DeliveryReportCb* >( this ), errStr );
        kafkaConf->set( "rebalance_cb", static_cast< RdKafka::RebalanceCb* >( this ), errStr );
        kafkaConf->set( "offset_commit_cb", static_cast< RdKafka::OffsetCommitCb* >( this ), errStr );

        rd_kafka_conf_set_log_cb( kafkaConf->c_ptr_global(), &RdKafkaLogCallback );

        if ( m_maxTotalMsgsInFlight > 0 )
        {
            std::string maxTotalMsgsInFlightStr = CORE::ToString( m_maxTotalMsgsInFlight );
            kafkaConf->set( "max.in.flight", maxTotalMsgsInFlightStr, errStr );
        }
        if ( GUCEF_NULL != m_client )
        {
            std::string reconnectBackoffMsStr = CORE::ToString( m_client->GetConfig().reconnectDelayInMs );
            kafkaConf->set( "reconnect.backoff.ms", reconnectBackoffMsStr, errStr );
        }

        CKafkaPubSubClientConfig::StringMap::const_iterator m = clientConfig.kafkaProducerGlobalConfigSettings.begin();
        while ( m != clientConfig.kafkaProducerGlobalConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConf->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka Producer global config entry \"" +
                        (*m).first + "\"=\"" + (*m).second + "\" for topic \"" + m_config.topicName + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
        }
        GUCEF_DELETE m_kafkaProducerConf;
        m_kafkaProducerConf = kafkaConf;

        RdKafka::Conf* kafkaProducerTopicConfig = RdKafka::Conf::create( RdKafka::Conf::CONF_TOPIC );
        m = m_config.kafkaProducerTopicConfigSettings.begin();
        while ( m != m_config.kafkaProducerTopicConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaProducerTopicConfig->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka Producer topic config entry \"" +
                        (*m).first + "\"=\"" + (*m).second + "\" for topic \"" + m_config.topicName + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;

        }
        GUCEF_DELETE m_kafkaProducerTopicConf;
        m_kafkaProducerTopicConf = kafkaProducerTopicConfig;

        RdKafka::Producer* producer = RdKafka::Producer::create( m_kafkaProducerConf, errStr );
	    if ( producer == GUCEF_NULL )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to create Kafka producer for topic \"" + m_config.topicName + "\", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
	    }
        m_kafkaProducer = producer;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:LoadConfig: Successfully created Kafka producer for topic \"" + m_config.topicName + "\"" );

        RdKafka::Topic* topic = RdKafka::Topic::create( m_kafkaProducer, m_config.topicName, m_kafkaProducerTopicConf, errStr );
	    if ( topic == GUCEF_NULL )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to obtain Kafka Producer Topic handle for topic \"" +
                m_config.topicName + "\", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
	    }
        m_kafkaProducerTopic = topic;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:LoadConfig: Successfully created Kafka Producer Topic handle for topic: " + m_config.topicName );

    }

    if ( clientConfig.desiredFeatures.supportsSubscribing )
    {
        std::string errStr;

        RdKafka::Conf* kafkaConf = RdKafka::Conf::create( RdKafka::Conf::CONF_GLOBAL );
        kafkaConf->set( "metadata.broker.list", csvKafkaBrokerList, errStr );
	    kafkaConf->set( "event_cb", static_cast< RdKafka::EventCb* >( this ), errStr );
	    kafkaConf->set( "dr_cb", static_cast< RdKafka::DeliveryReportCb* >( this ), errStr );
        kafkaConf->set( "rebalance_cb", static_cast< RdKafka::RebalanceCb* >( this ), errStr );
        kafkaConf->set( "offset_commit_cb", static_cast< RdKafka::OffsetCommitCb* >( this ), errStr );

        rd_kafka_conf_set_log_cb( kafkaConf->c_ptr_global(), &RdKafkaLogCallback );
        rd_kafka_conf_set_stats_cb( kafkaConf->c_ptr_global(), &RdKafkaStatsCallback );

        CKafkaPubSubClientConfig::StringMap::const_iterator m = clientConfig.kafkaConsumerGlobalConfigSettings.begin();
        while ( m != clientConfig.kafkaConsumerGlobalConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConf->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka Consumer global config entry \"" +
                        (*m).first + "\"=\"" + (*m).second + "\" for topic \"" + m_config.topicName + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
        }

        #ifdef GUCEF_DEBUG_MODE
        bool tracePartitionEof = true;
        #else
        bool tracePartitionEof = clientConfig.desiredFeatures.supportsSubscriptionEndOfDataEvent;
        #endif
        kafkaConf->set( "enable.partition.eof", tracePartitionEof ? "true" : "false", errStr );

        if ( m_maxTotalMsgsInFlight > 0 )
        {
            std::string maxTotalMsgsInFlightStr = CORE::ToString( m_maxTotalMsgsInFlight );
            kafkaConf->set( "max.in.flight", maxTotalMsgsInFlightStr, errStr );
        }
        if ( GUCEF_NULL != m_client )
        {
            std::string reconnectBackoffMsStr = CORE::ToString( m_client->GetConfig().reconnectDelayInMs );
            kafkaConf->set( "reconnect.backoff.ms", reconnectBackoffMsStr, errStr );
        }

        GUCEF_DELETE m_kafkaConsumerConf;
        m_kafkaConsumerConf = kafkaConf;

        // Apply default client level topic config as an overlay
        RdKafka::Conf* kafkaConsumerTopicConfig = RdKafka::Conf::create( RdKafka::Conf::CONF_TOPIC );
        m = clientConfig.kafkaConsumerDefaultTopicConfigSettings.begin();
        while ( m != clientConfig.kafkaConsumerDefaultTopicConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConsumerTopicConfig->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClient:LoadConfig: Failed to set Kafka consumer topic config entry based on client wide config \"" +
                        (*m).first + "\"=\"" + (*m).second + "\" KafkaPubSubClientTopic topic \"" + m_config.topicName + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;

        }

        // Now the topic level topic config (if any)
        m = m_config.kafkaConsumerTopicConfigSettings.begin();
        while ( m != m_config.kafkaConsumerTopicConfigSettings.end() )
        {
            if ( RdKafka::Conf::CONF_OK != kafkaConsumerTopicConfig->set( (*m).first, (*m).second, errStr ) )
            {
		        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka consumer topic config entry \"" +
                        (*m).first + "\"=\"" + (*m).second + "\" for topic \"" + m_config.topicName + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;

        }

        GUCEF_DELETE m_kafkaConsumerTopicConf;
        m_kafkaConsumerTopicConf = kafkaConsumerTopicConfig;

        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "default_topic_conf", m_kafkaConsumerTopicConf, errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka consumer default topic config, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }

        bool autoCommit = clientConfig.desiredFeatures.supportsAutoMsgReceivedAck;
        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "enable.auto.commit", CORE::ToString( autoCommit ), errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka consumer global config's enable.auto.commit, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
        if ( RdKafka::Conf::CONF_OK != m_kafkaConsumerConf->set( "enable.auto.offset.store", CORE::ToString( autoCommit ), errStr ) )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka consumer global config's enable.auto.offset.store, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
        m_consumerOffsetWaitsForExplicitMsgAck = !autoCommit;

        // Check for the mandatory group ID property.
        // This may have already been set by the generic custom property setting that occured above
        std::string confValue;
        RdKafka::Conf::ConfResult configResult = m_kafkaConsumerConf->get( "group.id", confValue );
        if ( RdKafka::Conf::ConfResult::CONF_OK != configResult ||
             confValue.empty()                                  ||
             confValue.size() == 0                               )
        {
            if ( !m_config.consumerGroupName.IsNULLOrEmpty() )
            {
                CORE::CString consumerGroupName;
                if ( !TryResolveMacrosInString( m_config.consumerGroupName, consumerGroupName ) )
                {
		            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to resolve macros in consumerGroupName" );
                }

                if ( RdKafka::Conf::CONF_OK == m_kafkaConsumerConf->set( "group.id", consumerGroupName, errStr ) )
                {
		            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:LoadConfig: Set Kafka group.id to \"" +
                        consumerGroupName + "\" for topic " + m_config.topicName );
                }
                else
                {
		            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka consumer group id to \"" +
                        consumerGroupName + "\", error message: " + errStr );
                    ++m_kafkaErrorReplies;
                    return false;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: consumerGroupName is mandatory but not configured. check the config" );
            }
        }

        // Check for the mandatory group member ID property.
        // This may have already been set by the generic custom property setting that occured above
        configResult = m_kafkaConsumerConf->get( "group.instance.id", confValue );
        if ( RdKafka::Conf::ConfResult::CONF_OK != configResult ||
             confValue.empty()                                  ||
             confValue.size() == 0                               )
        {
            if ( !m_config.consumerName.IsNULLOrEmpty() )
            {
                CORE::CString consumerName;
                if ( !TryResolveMacrosInString( m_config.consumerName, consumerName ) )
                {
		            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to resolve macros in consumerName" );
                }

                if ( RdKafka::Conf::CONF_OK == m_kafkaConsumerConf->set( "group.instance.id", consumerName, errStr ) )
                {
		            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:LoadConfig: Set Kafka group.instance.id to \"" +
                        consumerName + "\" for topic " + m_config.topicName );
                }
                else
                {
		            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to set Kafka consumer instance id to \"" +
                        m_config.consumerName + "\", error message: " + errStr );
                    ++m_kafkaErrorReplies;
                    return false;
                }
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: consumerName is mandatory but not configured. check the config" );
            }
        }

        RdKafka::KafkaConsumer* consumer = RdKafka::KafkaConsumer::create( m_kafkaConsumerConf, errStr );
	    if ( consumer == GUCEF_NULL )
        {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:LoadConfig: Failed to create Kafka consumer, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
	    }
        m_kafkaConsumer = consumer;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:LoadConfig: Successfully created Kafka consumer" );
    }

    if ( m_config.useTopicLevelMaxTotalMsgsInFlight )
        m_maxTotalMsgsInFlight = (Int32) SMALLEST( m_config.maxTotalMsgsInFlight, GUCEF_INT32MAX );
    else
        m_maxTotalMsgsInFlight = (Int32) SMALLEST( m_client->GetConfig().maxTotalMsgsInFlight, GUCEF_INT32MAX );

    if ( m_maxTotalMsgsInFlight > 0 )
        PrepStorageForReadMsgs( (UInt32) m_maxTotalMsgsInFlight );

    return true;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::PrepStorageForReadMsgs( CORE::UInt32 msgCount )
{GUCEF_TRACE;

    if ( msgCount > m_rdKafkaMsgs.size() )
    {
        // Add extra slots
        m_rdKafkaMsgs.resize( msgCount );
    }

    if ( msgCount > m_pubsubMsgs.size() )
    {
        // Add extra slots
        m_pubsubMsgs.resize( msgCount );
    }

    // reset size, note this does not dealloc the underlying memory
    TPubSubMsgsRefVector& msgRefs = m_pubsubMsgsRefs;
    msgRefs.clear();

    if ( msgCount > msgRefs.capacity() )
    {
        // Grow the actual storage, this is allowed to become larger than msgCount to limit
        // the nr of dynamic allocations
        msgRefs.reserve( msgCount );
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_shouldBeConnected = false;

    bool totalSuccess = true;

    if ( GUCEF_NULL != m_kafkaConsumer )
    {
        RdKafka::ErrorCode response = m_kafkaConsumer->unsubscribe();
        if ( RdKafka::ERR_NO_ERROR != response )
        {
		    std::string errStr = RdKafka::err2str( response );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:Disconnect: Failed to unsubscribe() Kafka Consumer for topic \"" +
                m_config.topicName + ", error message: " + errStr );
            ++m_kafkaErrorReplies;
            totalSuccess = false;
        }
        else
        {
            m_isSubscribed = false;
        }
        response = m_kafkaConsumer->close();
        if ( RdKafka::ERR_NO_ERROR != response )
        {
		    std::string errStr = RdKafka::err2str( response );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:Disconnect: Failed to close() Kafka Consumer for topic \"" +
                m_config.topicName + ", error message: " + errStr );
            ++m_kafkaErrorReplies;
            totalSuccess = false;
        }
    }

    if ( GUCEF_NULL != m_kafkaProducer )
    {
        RdKafka::ErrorCode response = RdKafka::ERR_NO_ERROR;
        int waited = 0;
        int queuedMsgs = m_kafkaProducer->outq_len();
        while ( queuedMsgs > 0 && waited <= 30000 )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Disconnect: Waiting on Kafka Producer for topic: \"" +
                m_config.topicName + "\" to finish " + CORE::Int32ToString( queuedMsgs ) + " queued messages" );

            response = m_kafkaProducer->flush( 1000 );
            if ( RdKafka::ERR_NO_ERROR == response )
                break;

            waited += 1000;
            queuedMsgs = m_kafkaProducer->outq_len();
        }
        if ( waited > 30000 || RdKafka::ERR_NO_ERROR != response )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Disconnect: Timed out waiting on Kafka Producer for topic: \"" +
                m_config.topicName + "\" to finish " + CORE::Int32ToString( queuedMsgs ) + " queued messages. Shutdown will continue regardless. Data loss will occur!" );
            ++m_kafkaErrorReplies;
            totalSuccess = false;
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::RetrieveKafkaCommitedOffsets( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_kafkaConsumer )
    {
        TRdKafkaTopicPartitionPtrVector partitions;
        RdKafka::ErrorCode err = m_kafkaConsumer->committed( partitions, 10000 );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            bool retrievedKafkaCommitedOffsets = false;
            CORE::CString commitInfo = "KafkaPubSubClientTopic:RetrieveKafkaCommitedOffsets: Group \"" + m_config.consumerGroupName + "\", Member ID \"" + m_kafkaConsumer->memberid() + "\": ";
            if ( !partitions.empty() )
            {
                bool hasErrorPartition = false;
                for ( unsigned int i=0; i<partitions.size(); ++i )
                {
                    RdKafka::TopicPartition* partition = partitions[ i ];
                    if ( GUCEF_NULL != partition )
                    {
                        Int32 partitionId = partition->partition();
                        Int64 partitionOffset = partition->offset();

                        commitInfo += "Topic \"" + CORE::ToString( partition->topic() ) + "\" has partition \"" + CORE::ToString( partitionId ) + "\" with committed offset \"" + CORE::ToString( partitionOffset ) + "\". ";

                        if ( RdKafka::Topic::OFFSET_INVALID != partitionOffset )
                        {
                            m_kafkaCommitedConsumerOffsets[ partitionId ] = partitionOffset;
                        }
                        else
                        {
                            hasErrorPartition = true;
                        }
                    }
                }

                if ( !hasErrorPartition )
                    retrievedKafkaCommitedOffsets = true;
            }
            else
            {
                commitInfo += "has no known server-side commited offsets";
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, commitInfo );

            return retrievedKafkaCommitedOffsets;
        }
        else
        {
            ++m_kafkaErrorReplies;
            std::string errStr = RdKafka::err2str( err );
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:RetrieveKafkaCommitedOffsets: Failed to obtain currently commited offsets for member \"" + m_kafkaConsumer->memberid() + "\". ErrorCode : " + errStr );
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    /*
        Per RdKafka docs:

        The high-level Kafka consumer (KafkaConsumer in C++) will start consuming at the last committed offset by default,
        if there is no previously committed offset for the topic+partition and group it will fall back on the topic configuration property auto.offset.reset which defaults to latest,
        thus starting to consume at the end of the partition (only new messages will be consumed).
    */

    MT::CScopeMutex lock( m_lock );
    m_shouldBeConnected = true;

    if ( !IsConnected() )
        if ( !SetupBasedOnConfig() )
            return false;

    if ( GUCEF_NULL != m_kafkaConsumer )
    {
        // First obtain the currently commited offsets if possible
        // this is optional but desired as it helps with troubleshooting and helps init the stats earlier
        RetrieveKafkaCommitedOffsets();

        // Perform the actual subscribe
        std::vector< std::string > topics;
        topics.push_back( m_config.topicName );
        RdKafka::ErrorCode response = m_kafkaConsumer->subscribe( topics );
        if ( RdKafka::ERR_NO_ERROR == response )
        {
            m_isSubscribed = true;

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:Subscribe: Successfully subscribed Kafka Consumer for topic \"" +
                m_config.topicName + "\" using Kafka server side bookmark as the starting position" );
        }
        else
        {
		    std::string errStr = RdKafka::err2str( response );
            GUCEF_ERROR_LOG(CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:Subscribe: Failed to 'subscribe' Kafka Consumer for topic \"" +
                m_config.topicName + ", error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }

        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::VariantToPartitionOffset( const CORE::CVariant& partitionOffsetBlob, TPartitionOffset& offset )
{GUCEF_TRACE;

    try
    {
        CORE::CDynamicBuffer blob;
        blob.LinkTo( partitionOffsetBlob );
        CORE::UInt32 byteOffset = 0;

        CORE::UInt32 partitionId = blob.AsConstType< CORE::UInt32 >( byteOffset );
        byteOffset += sizeof( CORE::UInt32 );
        offset.partitionId = partitionId;

        offset.partitionOffset = blob.AsConstType< CORE::Int64 >( byteOffset );
        byteOffset += sizeof( CORE::Int64 );

        return true;
    }
    catch ( CORE::CDynamicBuffer::EIllegalCast& )
    {

    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::VariantToPartitionOffsets( const CORE::CVariant& indexBookmarkBlob, TPartitionOffsetMap& offsets )
{GUCEF_TRACE;

    try
    {
        CORE::CDynamicBuffer blob;
        blob.LinkTo( indexBookmarkBlob );

        CORE::UInt32 partitionCount = (CORE::UInt32) blob.GetDataSize() / sizeof( TPartitionOffset );

        CORE::UInt32 offset = 0;
        for ( CORE::UInt32 i=0; i<partitionCount; ++i )
        {
            CORE::UInt32 partitionId = blob.AsConstType< CORE::UInt32 >( offset );
            offset += sizeof( CORE::UInt32 );

            TPartitionOffset& offsetEntry = offsets[ partitionId ];
            offsetEntry.partitionId = partitionId;

            offsetEntry.partitionOffset = blob.AsConstType< CORE::Int64 >( offset );
            offset += sizeof( CORE::Int64 );
        }

        return true;
    }
    catch ( CORE::CDynamicBuffer::EIllegalCast& )
    {

    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::SubscribeStartingAtTopicIndex( const CORE::CVariant& indexBookmark )
{GUCEF_TRACE;

    /*
        Per RdKafka docs:

        To manually set the starting offset for a partition the assign() API allows you to specify the start offset for each partition
        by setting the .offset field in the topic_partition_t element to either an absolute offset (>=0) or one of the logical offsets (BEGINNING, END, STORED, TAIL(..)).
    */

    std::string errStr;
    RdKafka::ErrorCode response;

    MT::CScopeMutex lock( m_lock );

    m_shouldBeConnected = true;

    // First we need to set up RdKafka with the relevant internal stuctures for our topics
    std::vector< std::string > topics;
    topics.push_back( m_config.topicName );
    response = m_kafkaConsumer->subscribe( topics );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to start Kafka Consumer for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    std::vector< RdKafka::TopicPartition* > partitions;

    // Now obtain the relevant partitions for our topic
    response = m_kafkaConsumer->position( partitions );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		m_kafkaConsumer->unsubscribe();
        std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to obtain current partitions for Kafka Consumer for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    // If a BLOB is passed we assume we are trying to set exact dictated offsets per partition
    // Using the same concept as in GetCurrentBookmark()
    if ( indexBookmark.IsBlob() )
    {
        TPartitionOffsetMap offsets;
        if ( !VariantToPartitionOffsets( indexBookmark, offsets ) )
        {
            m_kafkaConsumer->unsubscribe();
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to convert bookmark variant blob into partition offsets for Kafka Consumer for topic: " + m_config.topicName );
            return false;
        }

        // Now locally modify the offsets for the partitions that are still valid
        std::vector< RdKafka::TopicPartition* >::iterator i = partitions.begin();
        while ( i != partitions.end() )
        {
            TPartitionOffsetMap::iterator o = offsets.find( (*i)->partition() );
            if ( o != offsets.end() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Found offset stored in the blob bookmark for partition " +
                    CORE::ToString( (CORE::Int64)(*i)->partition() ) + ". Existing offset " + CORE::ToString( (CORE::Int64)(*i)->offset() ) + " will be replaced by " + CORE::ToString( (*o).second.partitionOffset ) + " for topic: " + m_config.topicName );

                // The offset given could be a numerical alias, we account for that here as well
                // if its truly a numerical offset this will just be pass through
                CORE::Int64 newOffset = ConvertKafkaConsumerStartOffset( (*o).second.partitionOffset ,
                                                                         (*i)->partition()           ,
                                                                         10000                       );

                if ( RdKafka::Topic::OFFSET_INVALID == newOffset )
                {

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to convert numerical offset description for Kafka Consumer for topic \"" +
                        m_config.topicName + " to a valid offset, offset alias=" + CORE::ToString( (*o).second.partitionOffset ) + " partition=" + CORE::ToString( (*i)->partition() ) );
                    ++m_kafkaErrorReplies;

                    // We don't allow for partial success as that would leave us in an unpredictable state
                    m_kafkaConsumer->unsubscribe();
                    return false;
                }

                (*i)->set_offset( (*o).second.partitionOffset );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: No offset was stored in the blob bookmark for partition " +
                    CORE::ToString( (CORE::Int64)(*i)->partition() ) + ". Will use existing offset " + CORE::ToString( (CORE::Int64)(*i)->offset() ) + " instead for topic: " + m_config.topicName );
            }

            ++i;
        }
    }
    else
    {
        CORE::CString startOffsetDescription = indexBookmark.AsString();

        // Now locally modify the offsets for the partitions based on the alias given
        std::vector< RdKafka::TopicPartition* >::iterator i = partitions.begin();
        while ( i != partitions.end() )
        {
            CORE::Int64 startOffset = ConvertKafkaConsumerStartOffset( startOffsetDescription ,
                                                                       (*i)->partition()      ,
                                                                       10000                  );


            if ( RdKafka::Topic::OFFSET_INVALID == startOffset )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to convert start offset description for Kafka Consumer for topic \"" +
                    m_config.topicName + " to a valid offset, startOffsetDescription=" + startOffsetDescription + " partition=" + CORE::ToString( (*i)->partition() ) );
                ++m_kafkaErrorReplies;

                // We don't allow for partial success as that would leave us in an unpredictable state
                m_kafkaConsumer->unsubscribe();
                return false;
            }
            (*i)->set_offset( startOffset );

            ++i;
        }
    }

    // Now persist our now locally modified offsets into RdKafka
    response = m_kafkaConsumer->assign( partitions );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		m_kafkaConsumer->unsubscribe();
        std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to assign() new offsets to current partitions for Kafka Consumer for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    // And finally persist our changes server-side
    if ( !CommitConsumerOffsets() )
    {
        m_kafkaConsumer->unsubscribe();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtTopicIndex: Failed to commit new parition offsets for consumer to Kafka for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::SubscribeStartingAtMsgDateTime( const CORE::CDateTime& msgDtBookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_shouldBeConnected = true;

    std::string errStr;
    RdKafka::ErrorCode response = RdKafka::ERR_NO_ERROR;
    CORE::UInt64 ticksSinceEpoch = msgDtBookmark.ToUnixEpochBasedTicksInMillisecs();

    // First we need to set up RdKafka with the relevant internal stuctures for our topics
    std::vector< std::string > topics;
    topics.push_back( m_config.topicName );
    response = m_kafkaConsumer->subscribe( topics );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to start Kafka Consumer for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    std::vector< RdKafka::TopicPartition* > partitions;

    // Now obtain the relevant partitions for our topic
    response = m_kafkaConsumer->position( partitions );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		m_kafkaConsumer->unsubscribe();
        std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to obtain current partitions for Kafka Consumer for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    // Now locally modify the offsets for the partitions based on the timestamp given
    std::vector< RdKafka::TopicPartition* >::iterator i = partitions.begin();
    while ( i != partitions.end() )
    {
        // IMPORTANT: The API for getting time based offsets is a bit wonkey in that you need to abuse the
        // offset field on input as a ticks since Epoch field. Due to the calling function context the RdKafka library will
        // know you actually filled it with time ticks and not with an absolute offset nr
        (*i)->set_offset( ticksSinceEpoch );
        ++i;
    }

    response = m_kafkaConsumer->offsetsForTimes( partitions, 10000 );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		m_kafkaConsumer->unsubscribe();
        std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to obtain offsetsForTimes() for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    // Now persist our now locally modified offsets into RdKafka
    response = m_kafkaConsumer->assign( partitions );
    if ( RdKafka::ERR_NO_ERROR != response )
    {
		m_kafkaConsumer->unsubscribe();
        std::string errStr = RdKafka::err2str( response );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to assign() new offsets to current partitions for Kafka Consumer for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    // And finally persist our changes server-side
    if ( !CommitConsumerOffsets() )
    {
        m_kafkaConsumer->unsubscribe();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:SubscribeStartingAtMsgDateTime: Failed to commit new parition offsets for consumer to Kafka for topic \"" +
            m_config.topicName + ", error message: " + errStr );
        ++m_kafkaErrorReplies;
        return false;
    }

    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::SubscribeStartingAtMsgDateTime( const CORE::CVariant& msgDtBookmark )
{GUCEF_TRACE;

    CORE::CDateTime msgDt;
    if ( msgDtBookmark.IsInteger() )
    {
        CORE::UInt64 dtInt = msgDtBookmark.AsUInt64( GUCEF_MT_UINT64MAX );
        if ( GUCEF_MT_UINT64MAX != dtInt  )
        {
            msgDt.FromUnixEpochBasedTicksInMillisecs( dtInt );
            return SubscribeStartingAtMsgDateTime( msgDt );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:SubscribeStartingAtBookmark: Failed to use Unix epoch based ticks in millisecs based timestamp. Bookmark=" + msgDtBookmark.AsString() );
        }
    }
    else
    {
        if ( msgDt.FromIso8601DateTimeString( msgDtBookmark.AsString() ) )
        {
            return SubscribeStartingAtMsgDateTime( msgDt );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:SubscribeStartingAtBookmark: Failed to parse ISO 8601 string based timestamp. Bookmark=" + msgDtBookmark.AsString() );
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_shouldBeConnected = true;

    if ( !IsConnected() )
        if ( !SetupBasedOnConfig() )
            return false;

    switch ( bookmark.GetBookmarkType() )
    {
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID:
        {
            // This is not supported in Kafka
            // You can seek a stream based on index (offset) or with some effort get said index based on a timestamp
            // duplicate IDs (keys) are to be expected as they are used for partition routing/sharding purposes
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:SubscribeStartingAtBookmark: Cannot subscribe with a Msg ID based bookmark when using Kafka. Bookmark=" + bookmark.GetBookmarkData().AsString() );
            break;
        }
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_DATETIME:
        {
            return SubscribeStartingAtMsgDateTime( bookmark.GetBookmarkData() );
        }
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX:
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX:
        {
            return SubscribeStartingAtTopicIndex( bookmark.GetBookmarkData() );
        }
        default:
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:SubscribeStartingAtBookmark: Cannot subscribe with the bookmark type given when using Kafka. Type=" + CORE::ToString( (CORE::Int32) bookmark.GetBookmarkType() ) + ". Bookmark=" + bookmark.GetBookmarkData().AsString() );
            break;
        }
    }

    return false;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark
CKafkaPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    try
    {
        MT::CScopeMutex lock( m_lock );

        // In Kafka there is no singular offset or such per topic. Such bookmarks are per partition instead for which a topic represents 1-N of them
        // This poses a problem when trying to represent this as a unified bookmark concept
        // To remedy we encode/serialize all partition IDs and their offsets into a blob bookmark
        // This will achieve the aim, to the extent possible with Kafka, of being able to retain this information client-side as a unified concept

        // Also note that we are not using the Kafka comitted offsets here but rather the current best read position which can be a little ahead of
        // the actual commit to Kafka but it is the most recent ack'd position that we can provide without risk of information loss

        if ( !m_consumerAckdOffsets.empty() )
        {
            CORE::CDynamicBuffer consumerOffsetsBuffer( (UInt32) ( m_consumerAckdOffsets.size() * ( sizeof(CORE::UInt32) + sizeof(CORE::Int64) ) ) );

            CORE::UInt32 byteOffset = 0;
            TInt32ToInt64Map::iterator i = m_consumerAckdOffsets.begin();
            while ( i != m_consumerAckdOffsets.end() )
            {
                *consumerOffsetsBuffer.AsTypePtr< CORE::UInt32 >( byteOffset ) = (*i).first;
                byteOffset += sizeof(CORE::UInt32);
                *consumerOffsetsBuffer.AsTypePtr< CORE::Int64 >( byteOffset ) = (*i).second;
                byteOffset += sizeof(CORE::Int64);

                consumerOffsetsBuffer.SetDataSize( byteOffset );
                ++i;
            }

            return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX, CORE::CVariant( consumerOffsetsBuffer ) );
        }
    }
    catch ( CORE::CDynamicBuffer::EIllegalCast& )
    {
    }

    // Cannot provide a bookmark at this time
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_AVAILABLE );
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::AcknowledgeReceipt( const TPartitionOffset& offset )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TInt64Set& ackdOffsets = m_receivedMsgAcks[ offset.partitionId ];
    ackdOffsets.insert( offset.partitionOffset );
    m_msgsAckdSinceLastOffsetCommit = true;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    TPartitionOffset offset;
    if ( VariantToPartitionOffset( msg.GetMsgIndex(), offset ) )
    {
        return AcknowledgeReceipt( offset );
    }

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:AcknowledgeReceipt: Failed to obtain partition offset from Msg Index field" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    if ( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX == bookmark.GetBookmarkType() )
    {
        TPartitionOffset offset;
        if ( VariantToPartitionOffset( bookmark.GetBookmarkData(), offset ) )
        {
            return AcknowledgeReceipt( offset );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:AcknowledgeReceipt: Failed to obtain partition offset from bookmark data" );
        }
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:AcknowledgeReceipt: Unsupported bookmark type, expected bookmark type is MSG_INDEX" );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );

        // note that connected is a very relative term here, more like prepared to connect as needed if you are a producer
        if ( m_config.needPublishSupport )
            if ( GUCEF_NULL == m_kafkaProducer )
                return false;

        if ( m_config.needSubscribeSupport )
            if ( ( GUCEF_NULL == m_kafkaConsumer ) || !m_isSubscribed )
                return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    bool isSubscribed = false;
    MT::CScopeMutex lock( m_lock );
    isSubscribed = ( GUCEF_NULL != m_kafkaConsumer ) && m_isSubscribed;
    lock.EarlyUnlock();
    return isSubscribed;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    bool newHealthyState = true;

    if ( m_config.maxKafkaErrorsToBeHealthy >= 0 )
    {
        // Current and last metrics cycle error count counts against the max
        if ( m_kafkaErrorReplies + m_metrics.kafkaErrorReplies > (CORE::UInt32) m_config.maxKafkaErrorsToBeHealthy )
            newHealthyState = false;
    }

    // Aside from error count, having to reconnect while we should be connected is also not good
    if ( newHealthyState && m_shouldBeConnected && !IsConnected() )
        newHealthyState = false;

    // See if this is a change from the current persisted state
    if ( newHealthyState != m_isHealthy )
    {
        m_isHealthy = newHealthyState;

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:IsHealthy: Topic " + m_config.topicName + " is now healthy" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:IsHealthy: Topic " + m_config.topicName + " is now unhealthy" );
        }

        THealthStatusChangeEventData eData( newHealthyState );
        NotifyObservers( HealthStatusChangeEvent, &eData );
    }

    return newHealthyState;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    // We dont have any per-topic connectivity setup work that is
    // not tied directly to publishing/subscribing. RdKafka takes care of all that.

    // We can take the oppertunity to perform a settings sanity check though...

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_client )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:InitializeConnectivity: Internal error: Parent client object is missing" );
        return false;
    }

    const CKafkaPubSubClientConfig& clientConfig = m_client->GetConfig();

    if ( clientConfig.remoteAddresses.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:InitializeConnectivity: No Kafka broker addressess have been provided" );
        return false;
    }

    if ( m_config.topicName.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:InitializeConnectivity: No Kafka topic name" );
        return false;
    }

    if ( !m_config.needPublishSupport && !m_config.needSubscribeSupport )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:InitializeConnectivity: topic config claims no need for publishing or subscribing and neither will be available. Dummy mode only" );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CKafkaPubSubClientTopic::GetKafkaErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 kafkaErrorReplies = m_kafkaErrorReplies;
        m_kafkaErrorReplies = 0;
        return kafkaErrorReplies;
    }
    else
        return m_kafkaErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CKafkaPubSubClientTopic::GetKafkaConnectionErrorsCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 kafkaConnectionErrors = m_kafkaConnectionErrors;
        m_kafkaConnectionErrors = 0;
        return kafkaConnectionErrors;
    }
    else
        return m_kafkaConnectionErrors;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CKafkaPubSubClientTopic::GetKafkaMsgsTransmittedCounter( bool resetCounter )
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
CKafkaPubSubClientTopic::GetKafkaMsgsReceivedCounter( bool resetCounter )
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
CKafkaPubSubClientTopic::GetKafkaMsgsFilteredCounter( bool resetCounter )
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

bool
CKafkaPubSubClientTopic::UpdateKafkaMsgsReceiveLag( void )
{GUCEF_TRACE;

    bool totalSuccess = true;

    m_metrics.hasKafkaMessagesReceiveLag = false;
    m_metrics.kafkaMessagesReceiveLagAvg = 0;
    m_metrics.kafkaMessagesReceiveLagMin = 0;
    m_metrics.kafkaMessagesReceiveLagMax = 0;
    m_metrics.hasKafkaMessagesReceiveCommitLag = false;
    m_metrics.kafkaMessagesReceiveCommitLagAvg = 0;
    m_metrics.kafkaMessagesReceiveCommitLagMin = 0;
    m_metrics.kafkaMessagesReceiveCommitLagMax = 0;

    if ( GUCEF_NULL != m_kafkaConsumer )
    {
        RdKafka::ErrorCode response = RdKafka::ERR_NO_ERROR;

        TRdKafkaTopicPartitionPtrVector kafkaPartitions;
        response = m_kafkaConsumer->assignment( kafkaPartitions );
        if ( response != RdKafka::ERR_NO_ERROR )
                return false;

        if ( !kafkaPartitions.empty() )
        {
            CORE::Int64 minLag = GUCEF_INT64MAX;
            CORE::Int64 maxLag = GUCEF_INT64MIN;
            CORE::Int64 lag = 0;

            CORE::Int64 commitMinLag = GUCEF_INT64MAX;
            CORE::Int64 commitMaxLag = GUCEF_INT64MIN;
            CORE::Int64 commitLag = 0;

            TRdKafkaTopicPartitionPtrVector::iterator i = kafkaPartitions.begin();
            while ( i != kafkaPartitions.end() )
            {
                RdKafka::TopicPartition* partition = (*i);
                if ( GUCEF_NULL != partition )
                {
                    int64_t lowOffset = 0;
                    int64_t highOffset = 0;
                    response = m_kafkaConsumer->get_watermark_offsets( partition->topic(), partition->partition(), &lowOffset, &highOffset );
                    if ( response == RdKafka::ERR_NO_ERROR )
                    {
                        if ( RdKafka::Topic::OFFSET_INVALID != highOffset )
                        {
                            // We use the ack'd minimum offsets known to the topic as the earliest most up-to-date carat of where we
                            // are reading wise which can be a little ahead of the read position that was actually commited to Kafka

                            TInt32ToInt64Map::iterator n = m_consumerReadOffsets.find( partition->partition() );
                            if ( n != m_consumerReadOffsets.end() )
                            {
                                CORE::Int64 currentReadCursorOffset = (*n).second;
                                CORE::Int64 currentReadCursorLag = highOffset - currentReadCursorOffset;

                                if ( currentReadCursorLag < minLag )
                                    minLag = currentReadCursorLag;
                                if ( currentReadCursorLag > maxLag )
                                    maxLag = currentReadCursorLag;

                                lag += currentReadCursorLag;

                                TInt32ToInt64Map::iterator m = m_kafkaCommitedConsumerOffsets.find( partition->partition() );
                                if ( m != m_kafkaCommitedConsumerOffsets.end() )
                                {
                                    CORE::Int64 currentReadCursorCommitedOffset = (*m).second;
                                    if ( 0 < currentReadCursorCommitedOffset )
                                    {
                                        CORE::Int64 currentReadCursorCommitedOffsetLag = highOffset - currentReadCursorCommitedOffset;

                                        if ( currentReadCursorCommitedOffsetLag < commitMinLag )
                                            commitMinLag = currentReadCursorCommitedOffsetLag;
                                        if ( currentReadCursorCommitedOffsetLag > commitMaxLag )
                                            commitMaxLag = currentReadCursorCommitedOffsetLag;

                                        commitLag += currentReadCursorCommitedOffsetLag;
                                    }
                                }
                            }
                        }
                    }
                }
                ++i;
            }

            if ( minLag != GUCEF_INT64MAX && maxLag != GUCEF_INT64MIN )
            {
                m_metrics.hasKafkaMessagesReceiveLag = true;
                m_metrics.kafkaMessagesReceiveLagAvg = (CORE::UInt64) round( ( 1.0 * lag ) / ( 1.0 * kafkaPartitions.size() ) );
                m_metrics.kafkaMessagesReceiveLagMin = (CORE::UInt64) minLag;
                m_metrics.kafkaMessagesReceiveLagMax = (CORE::UInt64) maxLag;
            }
            else
                totalSuccess = false;

            if ( commitMinLag != GUCEF_INT64MAX && commitMaxLag != GUCEF_INT64MIN )
            {
                m_metrics.hasKafkaMessagesReceiveCommitLag = true;
                m_metrics.kafkaMessagesReceiveCommitLagAvg = (CORE::UInt64) round( ( 1.0 * commitLag ) / ( 1.0 * kafkaPartitions.size() ) );
                m_metrics.kafkaMessagesReceiveCommitLagMin = (CORE::UInt64) commitMinLag;
                m_metrics.kafkaMessagesReceiveCommitLagMax = (CORE::UInt64) commitMaxLag;
            }
            else
                totalSuccess = false;
        }
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

CKafkaPubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : kafkaMessagesTransmitted( 0 )
    , kafkaTransmitQueueSize( 0 )
    , kafkaTransmitOverflowQueueSize( 0 )
    , kafkaMessagesReceived( 0 )
    , hasKafkaMessagesReceiveLag( false )
    , kafkaMessagesReceiveLagMin( 0 )
    , kafkaMessagesReceiveLagAvg( 0 )
    , kafkaMessagesReceiveLagMax( 0 )
    , hasKafkaMessagesReceiveCommitLag( false )
    , kafkaMessagesReceiveCommitLagMin( 0 )
    , kafkaMessagesReceiveCommitLagAvg( 0 )
    , kafkaMessagesReceiveCommitLagMax( 0 )
    , kafkaMessagesFiltered( 0 )
    , kafkaErrorReplies( 0 )
    , kafkaConnectionErrors( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CKafkaPubSubClientConfig& clientConfig = m_client->GetConfig();

    m_metrics.kafkaErrorReplies = GetKafkaErrorRepliesCounter( true );
    m_metrics.kafkaConnectionErrors = GetKafkaConnectionErrorsCounter( true );

    if ( clientConfig.desiredFeatures.supportsPublishing )
    {
        m_metrics.kafkaTransmitQueueSize = (CORE::UInt32) m_kafkaProducer->outq_len();
        m_metrics.kafkaMessagesTransmitted = GetKafkaMsgsTransmittedCounter( true );
    }
    if ( clientConfig.desiredFeatures.supportsSubscribing )
    {
        UpdateKafkaMsgsReceiveLag();

        m_metrics.kafkaMessagesReceived = GetKafkaMsgsReceivedCounter( true );
        m_metrics.kafkaMessagesFiltered = GetKafkaMsgsFilteredCounter( true );
    }
}

/*-------------------------------------------------------------------------*/

const CKafkaPubSubClientTopic::TopicMetrics&
CKafkaPubSubClientTopic::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CKafkaPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

 MT::TLockStatus
 CKafkaPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CKafkaPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

CORE::Int64
CKafkaPubSubClientTopic::ConvertKafkaConsumerStartOffset( const CORE::CString& startOffsetDescription ,
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
CKafkaPubSubClientTopic::ConvertKafkaConsumerStartOffset( CORE::Int64 startOffsetDescription ,
                                                          CORE::Int32 partitionId            ,
                                                          CORE::Int32 timeoutInMs            )
{GUCEF_TRACE;

    if ( RdKafka::Topic::OFFSET_BEGINNING == startOffsetDescription )
    {
        int64_t high = 0; int64_t low = 0;
        RdKafka::ErrorCode err = m_kafkaConsumer->query_watermark_offsets( m_config.topicName, partitionId, &low, &high, timeoutInMs );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Converted offset description \"BEGINNING\" into offset " + CORE::Int64ToString( low ) + " for partition " + CORE::Int32ToString( partitionId ) );
            return low;
        }

        ++m_kafkaErrorReplies;
        std::string errStr = RdKafka::err2str( err );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Failed to convert offset description \"BEGINNING\" into offset for partition " +
                CORE::Int32ToString( partitionId ) + ". ErrorCode : " + errStr );
        return RdKafka::Topic::OFFSET_INVALID;
    }
    if ( RdKafka::Topic::OFFSET_END == startOffsetDescription )
    {
        int64_t high = 0; int64_t low = 0;
        RdKafka::ErrorCode err = m_kafkaConsumer->query_watermark_offsets( m_config.topicName, partitionId, &low, &high, timeoutInMs );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Converted offset description \"END\" into offset " + CORE::Int64ToString( low ) + " for partition " + CORE::Int32ToString( partitionId ) );
            return high;
        }

        std::string errStr = RdKafka::err2str( err );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Failed to convert offset description \"END\" into offset for partition " +
                CORE::Int32ToString( partitionId ) + ". ErrorCode : " + errStr );
        return RdKafka::Topic::OFFSET_INVALID;
    }
    if ( RdKafka::Topic::OFFSET_STORED == startOffsetDescription )
    {
        std::vector<RdKafka::TopicPartition*> partitions;
        RdKafka::ErrorCode err = m_kafkaConsumer->assignment( partitions );
        if ( RdKafka::ERR_NO_ERROR != err )
        {
            ++m_kafkaErrorReplies;

            std::string errStr = RdKafka::err2str( err );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Failed to convert offset description \"STORED\" into offset for partition " +
                    CORE::Int32ToString( partitionId ) + ". Cannot obtain partition assignment. ErrorCode : " + errStr );
            return RdKafka::Topic::OFFSET_INVALID;
        }

        err = m_kafkaConsumer->committed( partitions, timeoutInMs );
        if ( RdKafka::ERR_NO_ERROR != err )
        {
            ++m_kafkaErrorReplies;

            std::string errStr = RdKafka::err2str( err );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Failed to convert offset description \"STORED\" into offset for partition " +
                    CORE::Int32ToString( partitionId ) + ". Cannot obtain commited offsets . ErrorCode : " + errStr );
            return RdKafka::Topic::OFFSET_INVALID;
        }

        std::vector<RdKafka::TopicPartition*>::iterator i = partitions.begin();
        while ( i != partitions.end() )
        {
            if ( (*i)->partition() == partitionId )
            {
                CORE::Int64 storedOffset = (*i)->offset();
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Converted offset description \"STORED\" into offset " +
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
CKafkaPubSubClientTopic::ConvertKafkaConsumerStartOffset( CORE::Int64 offset )
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

const CORE::CString&
CKafkaPubSubClientTopic::GetConsumerConfigSetting( const CORE::CString& keyName, const CORE::CString& defaultValue ) const
{GUCEF_TRACE;

    CKafkaPubSubClientTopicConfig::StringMap::const_iterator i = m_config.kafkaConsumerTopicConfigSettings.find( keyName );
    if ( i != m_config.kafkaConsumerTopicConfigSettings.end() )
    {
        const CORE::CString& foundValue = (*i).second;
        return foundValue;
    }
    return defaultValue;
}

/*-------------------------------------------------------------------------*/

Int64
CKafkaPubSubClientTopic::GetConsumerConfigSettingAsInt64( const CORE::CString& keyName, Int64 defaultValue ) const
{GUCEF_TRACE;

    CKafkaPubSubClientTopicConfig::StringMap::const_iterator i = m_config.kafkaConsumerTopicConfigSettings.find( keyName );
    if ( i != m_config.kafkaConsumerTopicConfigSettings.end() )
    {
        const CORE::CString& foundValue = (*i).second;
        return CORE::StringToInt64( foundValue, defaultValue );
    }
    return defaultValue;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::offset_commit_cb( RdKafka::ErrorCode err                            ,
                                           std::vector<RdKafka::TopicPartition*>& partitions )
{GUCEF_TRACE;

    if ( RdKafka::ERR_NO_ERROR != err && RdKafka::ERR__NO_OFFSET != err )
    {
        ++m_kafkaErrorReplies;

        std::string errStr = RdKafka::err2str( err );
        CORE::CString commitInfo = "KafkaPubSubClientTopic:offset_commit_cb: Member ID \"" + m_kafkaConsumer->memberid() + "\": Code: " + errStr + " : ";
        for ( unsigned int i=0; i<partitions.size(); ++i )
        {
            commitInfo += "Topic \"" + partitions[ i ]->topic() + "\" is at partition \"" + CORE::Int32ToString( partitions[ i ]->partition() ).STL_String() + "\" at offset \"" + ConvertKafkaConsumerStartOffset( partitions[ i ]->offset() ).STL_String() + "\". ";
        }
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, commitInfo );
    }
    else
    {
        #ifdef GUCEF_DEBUG_MODE
        std::string errStr = RdKafka::err2str( err );
        CORE::CString commitInfo = "KafkaPubSubClientTopic:offset_commit_cb: Member ID \"" + m_kafkaConsumer->memberid() + "\": Code: " + errStr + " : ";
        for ( unsigned int i=0; i<partitions.size(); ++i )
        {
            commitInfo += "Topic \"" + partitions[ i ]->topic() + "\" is at partition \"" + CORE::Int32ToString( partitions[ i ]->partition() ).STL_String() + "\" at offset \"" + ConvertKafkaConsumerStartOffset( partitions[ i ]->offset() ).STL_String() + "\". ";
        }
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, commitInfo );
        #endif

        if ( RdKafka::ERR__NO_OFFSET == err )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: No offsets are stored server-side for this client id. If this is unexpected check your offset storage health and retention policies. Will switch to using a default initial setting" );

            // check our config to see how we want to handle this scenario
            Int64 requestTimeout = GetConsumerConfigSettingAsInt64( "request.timeout.ms", 3000 );
            CORE::CString offsetResetSetting = GetConsumerConfigSetting( "auto.offset.reset", DefaultOffsetResetValue );
            if ( offsetResetSetting.Lowercase() == "stored" )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: The \"auto.offset.reset\" setting is set to \"" + offsetResetSetting + " which is not correct for an invalid/no offset scenario. Overruling to " + DefaultOffsetResetValue );
                offsetResetSetting = DefaultOffsetResetValue;
            }
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: Using offset reset setting of \"" + offsetResetSetting + "\" with request timeout of " + CORE::ToString( requestTimeout ) );

            // Set the desired new offsets
            for ( unsigned int i=0; i<partitions.size(); ++i )
            {
                int64_t partitionOffset = partitions[ i ]->offset();
                if ( RdKafka::Topic::OFFSET_INVALID == partitionOffset                                                ||
                     RdKafka::Topic::OFFSET_INVALID == m_kafkaCommitedConsumerOffsets[ partitions[ i ]->partition() ] )
                {
                    Int64 newPartitionOffset = ConvertKafkaConsumerStartOffset( offsetResetSetting, partitions[ i ]->partition(), (Int32) requestTimeout );
                    m_kafkaCommitedConsumerOffsets[ partitions[ i ]->partition() ] = newPartitionOffset;
                    partitions[ i ]->set_offset( newPartitionOffset );
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: Resetting local offset administration to " + CORE::ToString( newPartitionOffset ) + " for partition " + CORE::ToString( partitions[ i ]->partition() ) );
                }
            }

            if ( CommitConsumerOffsets( false ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: Successfully commited the new offsets" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: CommitConsumerOffsets failed after reset of offsets" );
            }
        }
        else
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            for ( unsigned int i=0; i<partitions.size(); ++i )
            {
                int64_t partitionOffset = partitions[ i ]->offset();
                if ( 0 <= partitionOffset )
                {
                    m_kafkaCommitedConsumerOffsets[ partitions[ i ]->partition() ] = partitionOffset;
                }

            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::rebalance_cb( RdKafka::KafkaConsumer* consumer                  ,
                                       RdKafka::ErrorCode err                            ,
                                       std::vector<RdKafka::TopicPartition*>& partitions )
{GUCEF_TRACE;

    std::string actionStr = "<UNKNOWN>";
    if ( err == RdKafka::ERR__ASSIGN_PARTITIONS )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Assigning partitions" );
        RdKafka::ErrorCode assignSuccess = consumer->assign( partitions );

        // Check if we can use the newly assigned partition's offsets
        bool hasInvalidOffsets = false;
        for ( unsigned int i=0; i<partitions.size(); ++i )
        {
            Int64 paritionOffset = partitions[ i ]->offset();
            if ( RdKafka::Topic::OFFSET_INVALID == paritionOffset )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: We were given OFFSET_INVALID for topic \"" + m_config.topicName + "\"" );
                hasInvalidOffsets = true;
                break;
            }
        }

        // If the newly assigned partition offsets are Ok we will consider those the last commited offsets
        // in such a case no need for an extra call to Kafka
        bool updatedKafkaCommitedConsumerOffsets = false;
        if ( !hasInvalidOffsets )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: We were given offsets for topic \"" + m_config.topicName + "\". Will use those as the new 'commited' offsets" );

            m_kafkaCommitedConsumerOffsets.clear();
            for ( unsigned int i=0; i<partitions.size(); ++i )
            {
                Int32 partitionId = partitions[ i ]->partition();
                Int64 paritionOffset = partitions[ i ]->offset();
                m_kafkaCommitedConsumerOffsets[ partitionId ] = paritionOffset;
                m_consumerAckdOffsets[ partitionId ] = paritionOffset;

                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: partition " +
                    CORE::ToString( partitionId ) + " is now at offset " + CORE::ToString( paritionOffset ) );
            }
            updatedKafkaCommitedConsumerOffsets = true;
        }

        // Check if we can directly obtain our last commited offsets from Kafka if we dont have the values yet
        if ( m_kafkaCommitedConsumerOffsets.empty() )
        {
            if ( RetrieveKafkaCommitedOffsets() && !m_kafkaCommitedConsumerOffsets.empty() )
            {
                hasInvalidOffsets = false;
                updatedKafkaCommitedConsumerOffsets = true;
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Successfully retrieved last commited offsets for topic \"" + m_config.topicName + "\" from Kafka" );

                for ( unsigned int i=0; i<partitions.size(); ++i )
                {
                    Int32 partitionId = partitions[ i ]->partition();
                    Int64 paritionOffset = partitions[ i ]->offset();

                    TInt32ToInt64Map::iterator c = m_kafkaCommitedConsumerOffsets.find( partitionId );
                    if ( c != m_kafkaCommitedConsumerOffsets.end() )
                    {
                        Int64 comittedOffset = (*c).second;
                        partitions[ i ]->set_offset( comittedOffset );
                        m_consumerAckdOffsets[ partitionId ] = comittedOffset;

                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Changing offset for partition " + CORE::ToString( partitionId ) +
                            " from " + CORE::ToString( paritionOffset ) + " to committed value " + CORE::ToString( comittedOffset )   );
                    }
                    else
                    {
                        // this should never happen
                        hasInvalidOffsets = true;
                        partitions[ i ]->set_offset( RdKafka::Topic::OFFSET_INVALID );
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Offset for partition " + CORE::ToString( partitionId ) +
                            " with value " + CORE::ToString( paritionOffset ) + " could not be updated to a committed value since the partition is not available as part of the commited set" );
                    }
                }
            }
            else
            {
                // The Kafka cluster doesnt have our offsets (anymore?)
                // offset retention is controlled through settings such as "offsets.retention.minutes" at the broker level and can be altered at the topic level
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: The Kafka cluster doesnt have a valid commit offset (anymore?) for us for topic \"" + m_config.topicName + "\"." );
            }
        }

        // Any luck yet? If not fall back to locally cached values if we have any
        if ( ( hasInvalidOffsets || m_kafkaCommitedConsumerOffsets.empty() ) && !m_consumerAckdOffsets.empty() )
        {
            // We will need watermarks to bounds check our cached values since its possible they are aged too much and no longer valid
            for ( unsigned int i=0; i<partitions.size(); ++i )
            {
                Int32 partitionId = partitions[ i ]->partition();
                Int64 paritionOffset = partitions[ i ]->offset();

                int64_t high = 0; int64_t low = 0;
                RdKafka::ErrorCode err = m_kafkaConsumer->get_watermark_offsets( m_config.topicName, partitionId, &low, &high );
                if ( RdKafka::ERR_NO_ERROR == err )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: The watermark offsets for topic \"" + m_config.topicName +
                            "\" and partition " + CORE::ToString( partitionId ) + " are: Low=" + CORE::ToString( (Int64)low ) + ", High=" + CORE::ToString( (Int64)high ) );

                    // Does the offset for this partition need fixing? Some may already be Ok
                    if ( RdKafka::Topic::OFFSET_INVALID == paritionOffset )
                    {
                        // Try using the locally available in ram 'last ack'd' offsets
                        TInt32ToInt64Map::iterator a = m_consumerAckdOffsets.find( partitionId );
                        if ( a != m_consumerAckdOffsets.end() )
                        {
                            // We have last ack'd offsets in memory on our end for this partition
                            // in this case its safe to use the last ack'd offsets as long as they are within watermark range. This limits replay volume due to commit lag
                            Int64 lastAckdOffset = (*a).second;

                            if ( RdKafka::Topic::OFFSET_INVALID != lastAckdOffset && lastAckdOffset >= low && lastAckdOffset <= high )
                            {
                                // Great! The offset is in range and thus can be used as a substitute
                                partitions[ i ]->set_offset( lastAckdOffset );
                                m_kafkaCommitedConsumerOffsets[ partitionId ] = lastAckdOffset;
                                updatedKafkaCommitedConsumerOffsets = true;

                                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Kafka has offset " + CORE::ToString( paritionOffset ) + " for topic \"" + m_config.topicName +
                                        "\" and partition " + CORE::ToString( partitions[ i ]->partition() ) + ". We have a last Ack'd offset of " + CORE::ToString( lastAckdOffset ) + " so we will try to resume from that point instead" );
                            }
                            else
                            {
                                hasInvalidOffsets = true;

                                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Kafka has offset " + CORE::ToString( paritionOffset ) + " for topic \"" + m_config.topicName +
                                        "\" and partition " + CORE::ToString( partitions[ i ]->partition() ) + ". We have do NOT have a valid and usable last Ack'd offset: " + CORE::ToString( lastAckdOffset ) );
                            }
                        }
                    }
                }
                else
                {
                    ++m_kafkaErrorReplies;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:rebalance_cb: Unable to obtain low and high watermarks for topic \"" + m_config.topicName +
                            "\" and partition " + CORE::ToString( partitionId ) + ". Without this information our only option is a full reset" );
                }
            }
        }

        // Any luck yet? If not fall back to a reset to default offset values, starting from scratch
        if ( hasInvalidOffsets || m_kafkaCommitedConsumerOffsets.empty() )
        {
            Int64 requestTimeout = GetConsumerConfigSettingAsInt64( "request.timeout.ms", 3000 );

            hasInvalidOffsets = false;
            for ( unsigned int i=0; i<partitions.size(); ++i )
            {
                Int32 partitionId = partitions[ i ]->partition();
                Int64 paritionOffset = partitions[ i ]->offset();
                CORE::Int64 startOffset = (CORE::Int64) RdKafka::Topic::OFFSET_INVALID;

                // Does the offset for this partition need fixing? Some may already be Ok
                if ( RdKafka::Topic::OFFSET_INVALID == paritionOffset )
                {
                    startOffset = ConvertKafkaConsumerStartOffset( DefaultOffsetResetValue, partitionId, (Int32) requestTimeout );
                    if ( RdKafka::Topic::OFFSET_INVALID != startOffset )
                    {
                        partitions[ i ]->set_offset( startOffset );
                        m_kafkaCommitedConsumerOffsets[ partitionId ] = startOffset;
                        m_consumerAckdOffsets[ partitionId ] = startOffset;
                        m_receivedMsgAcks[ partitionId ].clear();
                        updatedKafkaCommitedConsumerOffsets = true;

                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Changing offset for partition " + CORE::ToString( partitionId ) +
                            " from " + CORE::ToString( paritionOffset ) + " to reset value " + CORE::ToString( startOffset ) );
                    }
                    else
                    {
                        // This should not happen
                        hasInvalidOffsets = true;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Offset for partition " + CORE::ToString( partitionId ) +
                            " with offset " + CORE::ToString( paritionOffset ) + " cannot be updated to a reset value since the reset value is invalid: " + CORE::ToString( startOffset ) );
                    }
                }
            }
        }

        // Revalidate
        if ( !hasInvalidOffsets )
        {
            for ( unsigned int i=0; i<partitions.size(); ++i )
            {
                Int32 partitionId = partitions[ i ]->partition();
                Int64 paritionOffset = partitions[ i ]->offset();
                if ( RdKafka::Topic::OFFSET_INVALID == paritionOffset )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: We still have OFFSET_INVALID for partition " + CORE::ToString( partitionId )  + " of topic \"" + m_config.topicName + "\"" );
                    hasInvalidOffsets = true;
                }
            }
        }

        if ( !hasInvalidOffsets && updatedKafkaCommitedConsumerOffsets && !m_kafkaCommitedConsumerOffsets.empty() )
        {
            if ( CommitConsumerOffsets( false ) )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Successfully committed the new offsets" );
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: CommitConsumerOffsets failed" );
            }
        }

        actionStr = "ASSIGN_PARTITIONS";
    }
    else
    if ( err == RdKafka::ERR__REVOKE_PARTITIONS )
    {
        consumer->unassign();
        for ( unsigned int i=0; i<partitions.size(); ++i )
        {
            m_consumerReadOffsets.erase( partitions[ i ]->partition() );
            m_kafkaCommitedConsumerOffsets.erase( partitions[ i ]->partition() );
        }

        actionStr = "REVOKE_PARTITIONS";
    }

    CORE::CString partitionInfo = "KafkaPubSubClientTopic:rebalance_cb: Member ID \"" + consumer->memberid() + "\": Action " + actionStr + " : ";
    for ( unsigned int i=0; i<partitions.size(); ++i )
    {
        partitionInfo += "Topic \"" + partitions[ i ]->topic() + "\" is at partition \"" + CORE::ToString( partitions[ i ]->partition() ).STL_String() +
                "\" at offset \"" + ConvertKafkaConsumerStartOffset( partitions[ i ]->offset() ).STL_String() + "\". ";

        int64_t high = 0; int64_t low = 0;
        RdKafka::ErrorCode err = m_kafkaConsumer->get_watermark_offsets( m_config.topicName, partitions[ i ]->partition(), &low, &high );
        if ( RdKafka::ERR_NO_ERROR == err )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:rebalance_cb: Actual offsets for topic \"" + m_config.topicName +
                    "\" and partition " + CORE::Int32ToString( partitions[ i ]->partition() ) + ": Low=" + ConvertKafkaConsumerStartOffset( low ) +
                    ", High=" + ConvertKafkaConsumerStartOffset( high ) );
        }
        else
        {
            ++m_kafkaErrorReplies;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: Unable to get offsets for topic \"" + m_config.topicName +
                    "\" and partition " + CORE::Int32ToString( partitions[ i ]->partition() ) );
        }
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, partitionInfo );
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::UpdateIsHealthyStatus( bool newStatus )
{GUCEF_TRACE;

    // See if this is a change from the current persisted state
    if ( newStatus != m_isHealthy )
    {
        // Make sure we use the same consistent logic to determine health
        // the caller might only know about one aspect of health
        // The IsHealthy check itself will update the status and perform eventing if needed
        newStatus = IsHealthy();
    }
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::LinkReceivedMsg( RdKafka::Message& message, CORE::UInt32 msgIndex )
{GUCEF_TRACE;

    // Grab a message wrapper from pre-allocated storage
    PUBSUB::CBasicPubSubMsg& msgWrap = m_pubsubMsgs[ msgIndex ];
    msgWrap.Clear();
    msgWrap.SetOriginClientTopic( CreateSharedPtr() );

    msgWrap.SetReceiveActionId( m_currentReceiveActionId );
    ++m_currentReceiveActionId;

    if ( message.key_len() > 0 )
    {
        if ( GUCEF_NULL != message.key() )
        {
            msgWrap.GetMsgId().LinkTo( message.key()->c_str(), (CORE::UInt32) message.key_len(), GUCEF_DATATYPE_UTF8_STRING );
        }
        else
        {
            msgWrap.GetMsgId().LinkTo( message.key_pointer(), (CORE::UInt32) message.key_len(), GUCEF_DATATYPE_BINARY_BLOB );
        }
    }

    CORE::CVariant& msgIndexVar = msgWrap.GetMsgIndex();
    TPartitionOffset* partitionOffset = msgIndexVar.AsBsobPtr< TPartitionOffset >();
    partitionOffset->partitionId = message.partition();
    partitionOffset->partitionOffset = message.offset();
    msgIndexVar.OverrideTypeId( GUCEF_DATATYPE_BINARY_BSOB );

    msgWrap.GetPrimaryPayload().LinkTo( message.payload(), (CORE::UInt32) message.len() );

    RdKafka::MessageTimestamp msgTimestamp = message.timestamp();
    switch ( msgTimestamp.type )
    {
        case RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME:      // Message creation time (source)
        case RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME:  // Message log append time (broker)
        {
            msgWrap.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( (CORE::UInt64 ) msgTimestamp.timestamp );
            break;
        }
        case RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE:    // Timestamp not available
        default:
        {
            msgWrap.GetMsgDateTime() = CORE::CDateTime::NowUTCDateTime();
        }
    }

    // Kafka does not differentiate between meta-data vs non-meta-data key-value pairs
    // as a feature to enhance out-of-the-box compatibility with other backend types we support stripping a prefix that allows detecting
    // of such a differentiation
    const RdKafka::Headers* headers = message.headers();
    if ( GUCEF_NULL != headers )
    {
        std::vector<RdKafka::Headers::Header> hdrs = headers->get_all();
        for ( size_t i=0; i<hdrs.size(); ++i )
        {
            // sadly not available as a reference... dynamic allocation
            CORE::CString headerKeyStr = hdrs[ i ].key();
            CORE::CVariant headerKey;
            CORE::CVariant headerValue( hdrs[ i ].value(), (CORE::UInt32) hdrs[ i ].value_size() );

            if ( !m_config.stripPrefixForMetaDataKvPairs && !m_config.stripPrefixForKvPairs )
            {
                headerKey = headerKeyStr;
                msgWrap.AddKeyValuePair( headerKey, headerValue );
            }
            else
            if ( m_config.stripPrefixForMetaDataKvPairs && 0 == headerKeyStr.HasSubstr( m_config.prefixToAddForMetaDataKvPairs, true ) )
            {
                headerKey = headerKeyStr.CutChars( m_config.prefixToAddForMetaDataKvPairs.Length(), true, 0 );
                msgWrap.AddMetaDataKeyValuePair( headerKey, headerValue );
            }
            else
            if ( m_config.stripPrefixForKvPairs && 0 == headerKeyStr.HasSubstr( m_config.prefixToAddForKvPairs, true ) )
            {
                headerKey = headerKeyStr.CutChars( m_config.prefixToAddForKvPairs.Length(), true, 0 );
                msgWrap.AddKeyValuePair( headerKey, headerValue );
            }
            else
            {
                msgWrap.AddKeyValuePair( headerKey, headerValue );
            }
        }
    }

    // Now that we have prepared our wrapped message let's link it
    // as a reference in the list of received messages we send out
    TPubSubMsgsRefVector& msgRefs = m_pubsubMsgsRefs;
    msgRefs.push_back( &msgWrap );
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::consume_cb( RdKafka::Message& message, void* opaque )
{GUCEF_TRACE;

    bool isFiltered = false;
    ProcessRdKafkaMessage( message, 0, isFiltered );
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::IsSubscriptionAtEndOfData( void ) const
{GUCEF_TRACE;

    if ( m_isSubscribed )
        return m_isSubscriptionAtEndOfData;

    bool isSubscriptionAtEndOfData = m_isSubscriptionAtEndOfData;
    if ( QueryKafkaIfSubscriptionIsAtEndOfData( isSubscriptionAtEndOfData ) )
        return isSubscriptionAtEndOfData;

    return m_isSubscriptionAtEndOfData;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::QueryKafkaIfSubscriptionIsAtEndOfData( bool& isSubscriptionAtEndOfData ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( GUCEF_NULL == m_kafkaConsumer )
        return false;

    if ( !m_consumerReadOffsets.empty() )
    {
        TInt32ToInt64Map::const_iterator i = m_consumerReadOffsets.begin();
        while ( i != m_consumerReadOffsets.end() )
        {
            CORE::Int32 partitionId = (*i).first;
            CORE::Int64 readPosition = (*i).second;

            int64_t lowOffset = 0;
            int64_t highOffset = 0;
            RdKafka::ErrorCode response = m_kafkaConsumer->get_watermark_offsets( m_config.topicName, partitionId, &lowOffset, &highOffset );
            if ( response == RdKafka::ERR_NO_ERROR )
            {
                if ( RdKafka::Topic::OFFSET_INVALID != highOffset   &&
                     RdKafka::Topic::OFFSET_INVALID != readPosition )
                {
                    // We check to see if the read position, the offset of the last message received, has reached the high watermark
                    if ( readPosition < highOffset )
                        return false;
                }
            }
            else
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "KafkaPubSubClientTopic:IsSubscriptionAtEndOfData: Error obtaining watermark offsets. ErrorCode=" + CORE::ToString( response ) );
                return false;
            }
            ++i;
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::ProcessRdKafkaMessage( RdKafka::Message& message ,
                                                CORE::UInt32 msgIndex     ,
                                                bool& isFiltered          )
{GUCEF_TRACE;

    bool success = false;
    switch ( message.err() )
    {
        case RdKafka::ERR__TIMED_OUT:
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Kafka consume error: TIMED_OUT: " + message.errstr() );
            UpdateIsHealthyStatus( false );
            break;
        }
        case RdKafka::ERR_NO_ERROR:
        {
            #ifdef GUCEF_DEBUG_MODE
            if ( GUCEF_NULL != message.key() )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "KafkaPubSubClientTopic:consume_cb: Received " + CORE::UInt64ToString( CORE::UInt64( message.len() ) ) + " byte message on topic \"" +
                        m_config.topicName + "\" with key \"" + *message.key() + "\" and offset " + CORE::Int64ToString( message.offset() ) + " for partition " + CORE::ToString( message.partition() ) );
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "KafkaPubSubClientTopic:consume_cb: Received " + CORE::UInt64ToString( CORE::UInt64( message.len() ) ) + " byte message on topic \"" +
                        m_config.topicName + " without a key and with offset " + CORE::Int64ToString( message.offset() ) + " for partition " + CORE::ToString( message.partition() ) );
            }
            #endif

            ++m_kafkaMessagesReceived;
            m_consumerReadOffsets[ message.partition() ] = message.offset();

            isFiltered = false;
            if ( m_config.useKafkaMsgHeadersForConsumerFiltering )
            {
                const RdKafka::Headers* headers = message.headers();
                if ( GUCEF_NULL != headers && !m_config.kafkaMsgHeaderUsedForFiltering.empty() )
                {
                    std::vector<RdKafka::Headers::Header> hdrs = headers->get( m_config.kafkaMsgHeaderUsedForFiltering );
                    for ( size_t i=0; i<hdrs.size(); ++i )
                    {
                        const char* hdrValue = hdrs[ i ].value_string();
                        if ( GUCEF_NULL != hdrValue )
                        {
                            CORE::CString::StringVector::iterator v = m_config.kafkaMsgValuesUsedForFiltering.begin();
                            while ( v != m_config.kafkaMsgValuesUsedForFiltering.end() )
                            {
                                if ( (*v) == hdrValue )
                                {
                                    isFiltered = true;
                                    ++m_kafkaMessagesFiltered;

                                    // A filtered message also counts as successfully handled
                                    // As such we need to update the offset so that its taken into account for a later commit of said offsets
                                    m_receivedMsgAcks[ message.partition() ].insert( message.offset() );

                                    // Even when using an explicit ack this counts as the ack since the msg will never leave the backend as it was filtered
                                    m_msgsAckdSinceLastOffsetCommit = true;

                                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "KafkaPubSubClientTopic:consume_cb: Filtered message on topic \"" +
                                            m_config.topicName + " with offset " + CORE::Int64ToString( message.offset() ) );
                                    break;
                                }
                                ++v;
                            }
                        }
                        if ( isFiltered )
                            break;
                    }
                }
            }

            if ( !isFiltered )
            {
                if ( !m_consumerOffsetWaitsForExplicitMsgAck )
                {
                    // Right away update the local offsets, don't wait for AcknowledgeReceipt()
                    // we fake ack right away allowing the process to progress towards a commit
                    CORE::Int64& offset = m_consumerAckdOffsets[ message.partition() ];
                    if ( message.offset() > offset )
                    {
                        offset = message.offset();
                        m_msgsAckdSinceLastOffsetCommit = true;  // Since something was recieved we want to set the flag to ensure we start commiting offsets again
                    }
                }

                LinkReceivedMsg( message, msgIndex );
                success = true;
            }

            // We will consider the ability to receive and process messages as healthy
            UpdateIsHealthyStatus( true );
            break;
        }
        case RdKafka::ERR__PARTITION_EOF:
        {
            // Last message that was available has been read
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "KafkaPubSubClientTopic: Kafka topic \"" + m_config.topicName + "\" doesnt have any new messages waiting to be consumed" );

            if ( m_client->GetConfig().desiredFeatures.supportsSubscriptionEndOfDataEvent )
            {
                // We need to see if we have reached EOF across all partitions not just the partition for which the RdKafka event was sent.
                // The signal from Kafka merely triggers a more hollistic check
                if ( IsSubscriptionAtEndOfData() )
                {
                    if ( !NotifyObservers( SubscriptionEndOfDataEvent ) )
                        return success;
                }
            }

            break;
        }
        case RdKafka::ERR__UNKNOWN_PARTITION:
        {
            ++m_kafkaErrorReplies;

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Kafka consume error: UNKNOWN_PARTITION: " + message.errstr() );
            UpdateIsHealthyStatus( false );
            break;
        }
        case RdKafka::ERR__UNKNOWN_TOPIC:
        {
            ++m_kafkaErrorReplies;

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Kafka consume error: UNKNOWN_TOPIC: " + message.errstr() );
            UpdateIsHealthyStatus( false );
            break;
        }
        case RdKafka::ERR__MAX_POLL_EXCEEDED:
        {
            ++m_kafkaErrorReplies;

            GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Kafka consume error: MAX_POLL_EXCEEDED: " + message.errstr() );
            break;
        }
        default:
        {
            ++m_kafkaErrorReplies;

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Kafka consume error: " + message.errstr() );
            UpdateIsHealthyStatus( false );
            break;
        }
    }

    return success;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::event_cb( RdKafka::Event& event )
{GUCEF_TRACE;

    switch ( event.type() )
    {
        case RdKafka::Event::EVENT_ERROR:
        {
            switch (  event.err() )
            {
                #ifdef GUCEF_DEBUG_MODE
                case RdKafka::ERR__PARTITION_EOF:
                {
                    // Last message that was available has been read
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Kafka topic \"" + m_config.topicName + "\" doesnt have any new messages waiting to be consumed" );
                    break;
                }
                #endif
                case RdKafka::ERR__RESOLVE:
                {
                    ++m_kafkaErrorReplies;
                    ++m_kafkaConnectionErrors;

                    // Per GitHub comment from edenhill on Dec 18, 2018 for issue #2159:
                    //  "It will re-resolve the address on each re-connect attempt, but it will not log equal sub-sequent errors."
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:event_cb: Unable to resolve Kafka broker DNS for Kafka topic \"" + m_config.topicName +
                        "\". Wrong DNS? event msg=\"" + event.str() + "\". error as string =\"" + RdKafka::err2str( event.err() ) + "\"" );
                    if ( !NotifyObservers( ConnectionErrorEvent ) ) return;
                    UpdateIsHealthyStatus( false );
                    break;
                }
                case RdKafka::ERR__TRANSPORT:
                {
                    ++m_kafkaErrorReplies;
                    ++m_kafkaConnectionErrors;

                    // Per GitHub comment from edenhill on Dec 18, 2018 for issue #2159:
                    //  "It will re-resolve the address on each re-connect attempt, but it will not log equal sub-sequent errors."
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:event_cb: Unable to establish or retain connection to Kafka brokers for Kafka topic \"" + m_config.topicName +
                        "\". event msg=\"" + event.str() + "\". error as string =\"" + RdKafka::err2str( event.err() ) + "\"" );
                    if ( !NotifyObservers( ConnectionErrorEvent ) ) return;
                    UpdateIsHealthyStatus( false );
                    break;
                }
                default:
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic:event_cb: Kafka error: " + RdKafka::err2str( event.err() ) + " from KafkaEventCallback()" );
                    break;
                }
            }
            break;
        }
        case RdKafka::Event::EVENT_STATS:
        {
	        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Kafka stats: " + event.str() );
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
                    ++m_kafkaErrorReplies;
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
                case RdKafka::Event::EVENT_SEVERITY_ERROR:
                {
                    ++m_kafkaErrorReplies;
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
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
                case RdKafka::Event::EVENT_SEVERITY_DEBUG:
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }

                case RdKafka::Event::EVENT_SEVERITY_NOTICE:
                default:
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka log: " + event.fac() + " : " + event.str() );
                    break;
                }
            }
	        break;
        }
        case RdKafka::Event::EVENT_THROTTLE:
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Kafka Throttle event: throttled for " + CORE::Int32ToString( event.throttle_time() ) + "ms by broker " +
                event.broker_name() + " with ID " + CORE::Int32ToString( event.broker_id() ) );
            NotifyObservers( PublishThrottleEvent );
            break;
        }
        default:
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "Kafka event: " + CORE::Int32ToString( (CORE::Int32) event.type() ) + ", with error code " + RdKafka::err2str( event.err() ) );
	        break;
        }
    }
}

/*-------------------------------------------------------------------------*/

const std::string&
CKafkaPubSubClientTopic::MsgStatusToString( RdKafka::Message::Status status )
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
CKafkaPubSubClientTopic::dr_cb( RdKafka::Message& message )
{GUCEF_TRACE;

    static const std::string NULLstr = "NULL";

    bool notify = GUCEF_NULL != message.msg_opaque();
    CORE::UInt64 publishActionId = 0;

    // We decode the publish action Id from the passthrough opaque void*
    // This saved us from having to perform another memory allocation
    #ifdef GUCEF_64BIT
    publishActionId = reinterpret_cast< CORE::UInt64 >( message.msg_opaque() );
    #else
    publishActionId = reinterpret_cast< CORE::UInt32 >( message.msg_opaque() );
    if ( m_currentPublishActionId > GUCEF_MT_UINT32MAX || publishActionId < m_currentPublishActionId )
    {
        publishActionId += GUCEF_MT_UINT32MAX;
    }
    #endif

    if ( message.err() )
    {
        ++m_kafkaErrorReplies;

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Kafka delivery report: publishActionId: " + CORE::ToString( publishActionId ) +
                                                ". error: \"" + message.errstr() +
                                                "\", on topic: \"" + message.topic_name() +
                                                "\", key: " + ( message.key() ? (*message.key()) : NULLstr ) +
                                                ", payload size: " + CORE::ToString( message.len() ).STL_String() +
                                                ", msg has status: " + MsgStatusToString( message.status() ) );

        if ( notify )
        {
            m_publishFailureActionIds.push_back( publishActionId );
        }
    }
    else
    {
        ++m_kafkaMsgsTransmitted;

        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Kafka delivery report: publishActionId: " + CORE::ToString( publishActionId ) +
                                                ". status: " + MsgStatusToString( message.status() ) +
                                                " , topic: \"" + message.topic_name() +
                                                "\", partition: " + CORE::ToString( (CORE::Int64)message.partition() ).STL_String() +
                                                ", offset: " + CORE::ToString( (CORE::Int64)message.offset() ).STL_String() +
                                                ", key: " + ( message.key() ? (*message.key()) : NULLstr ) +
                                                ", payload size: " + CORE::ToString( message.len() ).STL_String() );

        if ( notify )
        {
            m_publishSuccessActionIds.push_back( publishActionId );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::CommitConsumerOffsets( bool useAsyncCommit )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_kafkaConsumer )
        return false;

    // First get the assigned partitions for the consumer
    std::vector<RdKafka::TopicPartition*> partitions;
    RdKafka::ErrorCode err = m_kafkaConsumer->assignment( partitions );
    if ( RdKafka::ERR_NO_ERROR == err )
    {
        if ( !partitions.empty() )
        {
            // Match the current Topic objects with our simplistic bookkeeping and sync them
            std::vector<RdKafka::TopicPartition*>::iterator p = partitions.begin();
            while ( p != partitions.end() )
            {
                CORE::Int32 partitionId = (*p)->partition();
                CORE::Int64 partitionOffset = (*p)->offset();

                // try to find ackd offsets for the given partition
                // note that if no messages have been recieved and/or ackd yet then this will not yield anything

                TInt32ToInt64Map::iterator o = m_consumerAckdOffsets.find( partitionId );
                if ( o != m_consumerAckdOffsets.end() )
                {
                    CORE::Int64 ackdOffset = (*o).second;

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:CommitConsumerOffsets: Ackd offsets are available for partition " + CORE::ToString( partitionId ) +
                        ". Current offset is " + CORE::ToString( partitionOffset ) + ". Ackd offset is " + CORE::ToString( ackdOffset ) );

                    (*p)->set_offset( ackdOffset );
                }
                else
                {
                    // We dont have any acks for this partition we can use to update the offset
                    // as such fall back to the commited offsets collection
                    TInt32ToInt64Map::iterator o2 = m_kafkaCommitedConsumerOffsets.find( partitionId );
                    if ( o2 != m_kafkaCommitedConsumerOffsets.end() )
                    {
                        CORE::Int64 commitedOffset = (*o2).second;

                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:CommitConsumerOffsets: No ackd offsets are available for partition " + CORE::ToString( partitionId ) +
                            ". Current offset is " + CORE::ToString( partitionOffset ) + ". falling back to local copy of commited offset " + CORE::ToString( commitedOffset ) );

                        (*p)->set_offset( commitedOffset );
                    }
                    else
                    {
                        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic:CommitConsumerOffsets: Cannot commit: No Ack'd or commited offsets are available for partition " + CORE::ToString( partitionId ) );
                        return false;
                    }
                }

                ++p;
            }

            // Now we actually tell the client library locally about the new offsets
            err = m_kafkaConsumer->offsets_store( partitions );
            if ( RdKafka::ERR_NO_ERROR == err )
            {
                // Now we request to send the local offset bookkeeping to Kafka
                if ( useAsyncCommit )
                {
                    err = m_kafkaConsumer->commitAsync( partitions );
                    if ( RdKafka::ERR_NO_ERROR == err )
                    {
                        // Reset our flag so that we do not commit needlessly
                        m_msgsAckdSinceLastOffsetCommit = false;

                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: Successfully triggered async commit of current offsets" );
                        return true;
                    }
                    else
                    {
                        ++m_kafkaErrorReplies;

                        std::string errStr = RdKafka::err2str( err );
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Cannot commit consumer offsets: Failed to trigger async commit of current offets. ErrorCode : " + errStr );
                        UpdateIsHealthyStatus( false );
                        return false;
                    }
                }
                else
                {
                    err = m_kafkaConsumer->commitSync( partitions );
                    if ( RdKafka::ERR_NO_ERROR == err )
                    {
                        // Reset our flag so that we do not commit needlessly
                        m_msgsAckdSinceLastOffsetCommit = false;

                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: Successfully triggered sync commit of current offsets" );
                        return true;
                    }
                    else
                    {
                        ++m_kafkaErrorReplies;

                        std::string errStr = RdKafka::err2str( err );
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Cannot commit consumer offsets: Failed to trigger sync commit of current offets. ErrorCode : " + errStr );
                        UpdateIsHealthyStatus( false );
                        return false;
                    }
                }
            }
            else
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClientTopic: No partitions are currently assigned to the consumer" );

                // Obtain the relevant partitions for our topic
                err = m_kafkaConsumer->position( partitions );

                return true;
            }
        }
        else
        {
            ++m_kafkaErrorReplies;

            std::string errStr = RdKafka::err2str( err );
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Cannot commit consumer offsets: Failed to store current offets on local topic objects. ErrorCode : " + errStr );
            return false;
        }
    }
    else
    {
        ++m_kafkaErrorReplies;

        std::string errStr = RdKafka::err2str( err );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Cannot commit consumer offsets: Failed to obtain current partition assignment. ErrorCode : " + errStr );
        UpdateIsHealthyStatus( false );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::RequestSubscriptionMsgArrivalDelay( CORE::UInt32 minDelayInMs )
{GUCEF_TRACE;

    if ( 0 == m_requestedConsumeDelayInMs )
    {
        m_tickCountAtConsumeDelayRequest = GetPulseGenerator()->GetTickCount();
        m_requestedConsumeDelayInMs = minDelayInMs;
    }
    else
    {
        // We are still working on the last delay
        // We leave the last request timestamp but merge the requests by adding the remnant time to the newly requested time
        CORE::UInt32 remainingMs = (CORE::UInt32) GetPulseGenerator()->GetTimeSinceTickCountInMilliSecs( m_tickCountAtConsumeDelayRequest );
        m_requestedConsumeDelayInMs = minDelayInMs + remainingMs;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::ProcessMsgAcks( void )
{GUCEF_TRACE;

    bool offsetsChanged = false;

    // Acks can come in async with gaps
    // as such we need to match complete acks sequences against offsets we aim to commit
    // this is required to ensure we dont accidentally commit while experiencing a gap, skipping over the gap

    TInt32ToInt64SetMap::iterator n = m_receivedMsgAcks.begin();
    while ( n != m_receivedMsgAcks.end() )
    {
        CORE::Int32 partitionId = (*n).first;
        TInt64Set& ackdOffsets = (*n).second;

        if ( !ackdOffsets.empty() )
        {
            // We need to repeat this process for all partitions belonging to the topic
            TInt32ToInt64Map::iterator p = m_consumerAckdOffsets.find( partitionId );
            if ( p == m_consumerAckdOffsets.end() )
            {
                CORE::Int64 commitedOffset = 0;
                TInt32ToInt64Map::iterator k = m_kafkaCommitedConsumerOffsets.find( partitionId );
                if ( k == m_kafkaCommitedConsumerOffsets.end() )
                {
                    if ( RetrieveKafkaCommitedOffsets() )
                    {
                        k = m_kafkaCommitedConsumerOffsets.find( partitionId );
                    }
                }
                if ( k != m_kafkaCommitedConsumerOffsets.end() )
                {
                    commitedOffset = (*k).second;

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: No ackd offset found for partition " + CORE::ToString( partitionId ) +
                        " using Kafka commited offset as the new ack starting position: " + CORE::ToString( commitedOffset )  );

                    m_consumerAckdOffsets[ partitionId ] = commitedOffset;
                    p = m_consumerAckdOffsets.find( partitionId );
                }
                else
                {
                    // Note that we need the consensus ackd offset not the offsets in the m_receivedMsgAcks collection because there could be race conditions where an earlier offset will
                    // flow into that collection any second. As such we dont want to assume its lowest nr is the lowest nr of all messages in flight
                    // In contrast to the m_consumerAckdOffsets map which is only progressed once the full sequence is validated
                    GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: No Ackd or Kafka commited offsets are found for partition " + CORE::ToString( partitionId ) );
                    ++n;
                    continue;
                }
            }

            if ( p != m_consumerAckdOffsets.end() )
            {
                CORE::Int64& currentOffset = (*p).second;

                TInt64Set::iterator a = ackdOffsets.begin();
                CORE::Int64 ackdOffset = (*a);
                if ( currentOffset == ackdOffset || currentOffset+1 == ackdOffset )
                {
                    // We found the start of the next sequence to commit
                    // let's commit an entire block at once if we can by checking for a continuous sequence
                    ++a;
                    while ( a != ackdOffsets.end() )
                    {
                        CORE::Int64 nextAckdOffset = (*a);
                        if ( ackdOffset+1 == nextAckdOffset )
                            ackdOffset = nextAckdOffset;
                        else
                            break;

                        ++a;
                    }

                    // Now we move the consumer offset to the new position
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Moving ackd commit offset from " +
                        CORE::ToString( currentOffset ) + " to " + CORE::ToString( ackdOffset ) + " for partition " + CORE::ToString( partitionId ) );
                    currentOffset = ackdOffset;
                    offsetsChanged = true;
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClientTopic: Ackd offset " +
                        CORE::ToString( ackdOffset ) + " is too far from current offset " + CORE::ToString( currentOffset ) + " for partition " + CORE::ToString( partitionId ) );
                }
            }
        }
        ++n;
    }

    return offsetsChanged;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::CleanupMsgAcks( void )
{GUCEF_TRACE;

    // Acks which have been committed successfully need to be eliminated from our acks collection
    TInt32ToInt64Map::iterator p = m_consumerAckdOffsets.begin();
    while ( p != m_consumerAckdOffsets.end() )
    {
        CORE::Int32 partitionId = (*p).first;
        CORE::Int64 currentOffset = (*p).second;

        TInt32ToInt64SetMap::iterator n = m_receivedMsgAcks.begin();
        while ( n != m_receivedMsgAcks.end() )
        {
            TInt64Set& ackdOffsets = (*n).second;
            while ( !ackdOffsets.empty() )
            {
                CORE::Int64 ackdOffset = *(ackdOffsets.begin());
                if ( ackdOffset <= currentOffset )
                    ackdOffsets.erase( ackdOffsets.begin() );
                else
                    break;
            }

            ++n;
        }

        ++p;
    }
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_publishFailureActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData ) ) return;
        m_publishFailureActionIds.clear();
    }
    if ( !m_publishSuccessActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishedEvent, &m_publishSuccessActionEventData ) ) return;
        m_publishSuccessActionIds.clear();
    }

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

            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "KafkaPubSubClientTopic:OnPulseCycle: poll() on the kafkaProducer served " +
                    CORE::Int32ToString( opsServed ) + " events");
        }
        if ( i == 50 )
        {
            // We have more work to do. Make sure we dont go to sleep
            GetPulseGenerator()->RequestImmediatePulse();
        }
    }

    if ( GUCEF_NULL != m_kafkaConsumer && m_isSubscribed )
    {
        if ( m_requestedConsumeDelayInMs == 0 ||
             m_requestedConsumeDelayInMs <= GetPulseGenerator()->GetTimeSinceTickCountInMilliSecs( m_tickCountAtConsumeDelayRequest ) )
        {
            m_requestedConsumeDelayInMs = 0;

            UInt32 maxMsgsToRead = 100;
            if ( m_maxTotalMsgsInFlight > 0 )
            {
                maxMsgsToRead = (UInt32) m_maxTotalMsgsInFlight;
                PrepStorageForReadMsgs( maxMsgsToRead );
            }

            // Try to fetch a bunch of messages in one go
            CORE::Int32 msgRead = -1;
            for ( CORE::UInt32 i=0; i<maxMsgsToRead; ++i )
            {
                RdKafka::Message* msg = m_kafkaConsumer->consume( 0 );
                int errState = msg->err();
                if ( RdKafka::ERR__TIMED_OUT == errState )
                {
                    GUCEF_DELETE msg;
                    break;
                }
                if ( RdKafka::ERR__PARTITION_EOF == errState )
                {
                    GUCEF_DELETE msg;

                    // We have a toggle flag to ensure we dont trigger notifications on every check
                    // we only want to notify if the end of data status changed
                    if ( !m_isSubscriptionAtEndOfData )
                    {
                        m_isSubscriptionAtEndOfData = true;
                        if ( GUCEF_NULL != m_client )
                        {
                            if ( m_client->GetConfig().desiredFeatures.supportsSubscriptionEndOfDataEvent )
                            {
                                if ( !NotifyObservers( SubscriptionEndOfDataEvent ) )
                                    return;
                            }
                        }
                    }
                    break;
                }

                m_isSubscriptionAtEndOfData = false;
                bool isFiltered = false;
                if ( ProcessRdKafkaMessage( *msg, msgRead+1, isFiltered ) && !isFiltered )
                {
                    ++msgRead;
                    m_rdKafkaMsgs[ msgRead ] = msg;
                }
            }

            // Communicate all the messages received via an event notification in bulk
            // doing a single notification of the set of messages reduced eventing/routing/etc overhead with minimal extra latency
            if ( !m_pubsubMsgsRefs.empty() )
            {
                if ( !NotifyObservers( MsgsRecievedEvent, &m_pubsubMsgsRefs ) )
                    return;

                m_pubsubMsgsRefs.clear();
            }

            // Cleanup all the messages we received from RdKafka
            for ( CORE::Int32 i=0; i<=msgRead; ++i )
            {
                RdKafka::Message* msg = m_rdKafkaMsgs[ i ];
                GUCEF_DELETE msg;
                m_rdKafkaMsgs[ i ] = GUCEF_NULL;
            }

            if ( msgRead+1 >= m_maxTotalMsgsInFlight )
            {
                // We have more work to do. Make sure we dont go to sleep
                GetPulseGenerator()->RequestImmediatePulse();
            }

            if ( !m_consumerOffsetWaitsForExplicitMsgAck || m_msgsAckdSinceLastOffsetCommit )
            {
                // Periodically commit our offsets
                // This can slow things down so we dont want to do this too often
                if ( 5000 < GetPulseGenerator()->GetTimeSinceTickCountInMilliSecs( m_tickCountAtLastOffsetCommit ) )
                {
                    bool offsetsChanged = ProcessMsgAcks();
                    if ( offsetsChanged )
                    {
                        if ( CommitConsumerOffsets( false ) )
                        {
                            CleanupMsgAcks();

                            m_tickCountAtLastOffsetCommit = GetPulseGenerator()->GetTickCount();
                        }
                    }
                }
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClientTopic::DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg, PUBSUB::CPubSubBookmark& bookmark ) const
{
    // Note that this does not require a lock as data we access is local to the message
    // The code is here because only the backend logic knows which bookmark type can be derived (if any)
    // from a message for the given backend

    // For Kafka we know that we encode the partition offset on the Index property
    if ( msg.GetMsgIndex().ByteSize() > 0 && msg.GetMsgIndex().GetTypeId() == GUCEF_DATATYPE_BINARY_BSOB )
    {
        bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
        bookmark.SetBookmarkData( msg.GetMsgIndex() );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CKafkaPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::KAFKA::CKafkaPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAFKA */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
