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

#include "pubsubpluginKAFKA_CKafkaPubSubClient.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CKafkaPubSubClient::TypeName = "Kafka";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CKafkaPubSubClient::CKafkaPubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_kafkaProducerConf( GUCEF_NULL )
    , m_kafkaProducer( GUCEF_NULL )
    , m_config()
    , m_metricsTimer( GUCEF_NULL )
    , m_topicMap()
    , m_kafkaErrorReplies( 0 ) 
    , m_isHealthy( true )
    , m_lock()
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClient: Failed to load config at construction" );
    }

    SetupBasedOnConfig();
}

/*-------------------------------------------------------------------------*/

CKafkaPubSubClient::~CKafkaPubSubClient()
{GUCEF_TRACE;

    Clear();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    // first shut down all topics
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->Shutdown();
        ++i;
    }
    
    // now clean up the associated resources
    i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second.Unlink();
        ++i;
    }
    m_topicMap.clear();

    GUCEF_DELETE m_kafkaProducer;
    m_kafkaProducer = GUCEF_NULL;
    GUCEF_DELETE m_kafkaProducerConf;
    m_kafkaProducerConf = GUCEF_NULL;    
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    return SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                       bool includeTopics                        )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    m_config.pulseGenerator = newPulseGenerator;
    
    if ( GUCEF_NULL != m_metricsTimer )
    {
        m_metricsTimer->SetPulseGenerator( newPulseGenerator );
    }

    if ( includeTopics )
    {
        m_config.topicPulseGenerator = m_config.pulseGenerator;

        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            (*i).second->SetPulseGenerator( newPulseGenerator );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

CKafkaPubSubClientConfig& 
CKafkaPubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

CORE::ThreadPoolPtr 
CKafkaPubSubClient::GetThreadPool( void )
{GUCEF_TRACE;

    return m_threadPool;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
{GUCEF_TRACE;
                     
    features.supportsBinaryPayloads = true;               // Kafka supports this natively
    features.supportsPerMsgIds = true;                    // Kafka supports this natively
    features.supportsPrimaryPayloadPerMsg = true;         // This is the primary method for conveying data
    features.supportsAbsentPrimaryPayloadPerMsg = false;  // With Kafka the primary payload is not optional
    features.supportsKeyValueSetPerMsg = true;            // Kafka supports adding attibutes beyond the primary payload
    features.supportsDuplicateKeysPerMsg = true;          // Kafka not care about duplicate header keys, they are just "fields". Some client libraries might have issues with lookups though
    features.supportsMultiHostSharding = true;            // Kafka supports a cluster of brokers with multiple replicated partitions
    features.supportsPublishing = true;                   // We support being a Kafka producer in this plugin
    features.supportsSubscribing = true;                  // We support being a Kafka consumer in this plugin
    features.supportsMetrics = true;                      // We support metrics in this plugin
    features.supportsAutoReconnect = true;                // RdKafka adds auto reconnect out of the box
    features.supportsSubscriberMsgReceivedAck = true;     // Normal Kafka behaviour
    features.supportsAutoMsgReceivedAck = true;           // This is configurable on the Kafka client library
    features.supportsAbsentMsgReceivedAck = true;         // supported if you wish to 'peek' only without removing messages but can become problematic in various scenarios
    features.supportsAckUsingLastMsgInBatch = true;       // This is supported via how you trigger offset commits and is standard practice 
    features.supportsAckUsingBookmark = true;             // This is supported via how you trigger offset commits
    features.supportsBookmarkingConcept = true;           // Bookmarks are a native Kafka concept
    features.supportsSubscribingUsingBookmark = true;     // We provide limited support for client-side provided bookmarks
    features.supportsServerSideBookmarkPersistance = true;// The natural way to interact with Kafka is to let it manage bookmarks server-side
    features.supportsAutoBookmarking = true;              // Kafka keeps bookmarks for a given client id Kafka-side and supports auto commit in RdKafka
    features.supportsMsgIdBasedBookmark = false;          // Kafka does not support finding a conclusive offset based on a message key, there could be many across many partitions making it useless
    features.supportsMsgIndexBasedBookmark = true;        // Offsets (index) is the native Kafka "bookmark"method and thus preferred
    features.supportsTopicIndexBasedBookmark = true;      // Offsets (index) is the native Kafka "bookmark"method and thus preferred
    features.supportsMsgDateTimeBasedBookmark = true;     // We support this via code that converts the DateTime to offsets
    features.supportsDerivingBookmarkFromMsg = true;      // Supported via a BSOD on the message's index field currently
    features.supportsDiscoveryOfAvailableTopics = true;   // Implemented via RdKafka producer metadata api
    features.supportsGlobPatternTopicNames = true;
    features.supportsSubscriptionMsgArrivalDelayRequests = true;    // We support a backoff of the consume() event processing
    features.supportsSubscriptionEndOfDataEvent = false;  // @TODO: needs work
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CKafkaPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                       CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    if ( topicConfig.IsNULL() )
        return PUBSUB::CPubSubClientTopicPtr();
    
    PUBSUB::CPubSubClientTopicBasicPtr topicAccess;
    {
        MT::CScopeMutex lock( m_lock );

        // Check to see if this logical/conceptual 'topic' represents multiple pattern matched Kafka topics
        if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
             topicConfig->topicName.HasChar( '*' ) > -1               )
        {
            PubSubClientTopicSet allTopicAccess;
            if ( CreateMultiTopicAccess( topicConfig, allTopicAccess, pulseGenerator ) && !allTopicAccess.empty() )
            {
                // Caller should really use the CreateMultiTopicAccess() variant
                topicAccess = *(allTopicAccess.begin());
            }
        }
        else
        {
            CKafkaPubSubClientTopicPtr kafkaTopicAccess = ( GUCEF_NEW CKafkaPubSubClientTopic( this ) )->CreateSharedPtr();
            if ( kafkaTopicAccess->LoadConfig( *topicConfig ) )
            {
                topicAccess = kafkaTopicAccess;
                m_topicMap[ topicConfig->topicName ] = kafkaTopicAccess;

                ConfigureJournal( topicAccess, topicConfig );
                PUBSUB::CIPubSubJournalBasicPtr journal = topicAccess->GetJournal();
                if ( !journal.IsNULL() && topicConfig->journalConfig.useJournal )
                    journal->AddTopicCreatedJournalEntry();

                RegisterTopicEventHandlers( kafkaTopicAccess );
            }
            else
            {
                kafkaTopicAccess->Shutdown();
                kafkaTopicAccess.Unlink();
            }
        }
    }

    if ( !topicAccess.IsNULL() )
    {
        TopicAccessCreatedEventData eData( topicConfig->topicName );
        NotifyObservers( TopicAccessCreatedEvent, &eData );
    }

    return topicAccess;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr 
CKafkaPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        return (*i).second;
    }
    return PUBSUB::CPubSubClientTopicBasicPtr();
}

/*-------------------------------------------------------------------------*/

void 
CKafkaPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;
        
    MT::CObjectScopeLock lock( this );

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicAccess.insert( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::GetMultiTopicAccess( const CORE::CString& topicName    ,
                                         PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;

    // Check to see if this logical/conceptual 'topic' name represents multiple pattern matched Redis Streams
    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicName.HasChar( '*' ) > -1                           )
    {
        // We create the actual topic objects from the wildcard glob pattern topic which is used
        // as a template. As such we need to match the pattern again to find the various topics that could have been spawned
        bool matchesFound = false;
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            if ( (*i).first.WildcardEquals( topicName, '*', true ) )
            {
                topicAccess.insert( (*i).second );
                matchesFound = true;
            }
            ++i;
        }
        return matchesFound;
    }
    else
    {
        TTopicMap::iterator i = m_topicMap.find( topicName );
        if ( i != m_topicMap.end() )
        {
            topicAccess.insert( (*i).second );
            return true;
        }
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::GetMultiTopicAccess( const CORE::CString::StringSet& topicNames ,
                                         PubSubClientTopicSet& topicAccess          )
{GUCEF_TRACE;

    bool totalSuccess = true;
    CORE::CString::StringSet::const_iterator i = topicNames.begin();
    while ( i != topicNames.end() )
    {
        totalSuccess = GetMultiTopicAccess( (*i), topicAccess ) && totalSuccess;
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::AutoCreateMultiTopicAccess( const TTopicConfigPtrToStringSetMap& topicsToCreate ,
                                                PubSubClientTopicSet& topicAccess                   ,
                                                CORE::PulseGeneratorPtr pulseGenerator              )
{GUCEF_TRACE;

    CORE::UInt32 newTopicAccessCount = 0;
    bool totalSuccess = true;
    {
        MT::CObjectScopeLock lock( this );

        TTopicConfigPtrToStringSetMap::const_iterator m = topicsToCreate.begin();
        while ( m != topicsToCreate.end() )
        {
            PUBSUB::CPubSubClientTopicConfigPtr templateTopicConfig( ((*m).first) );
            if ( !templateTopicConfig.IsNULL() ) 
            {
                const CORE::CString::StringSet& topicNameList = (*m).second;

                CORE::CString::StringSet::const_iterator i = topicNameList.begin();
                while ( i != topicNameList.end() )
                {
                    CKafkaPubSubClientTopicConfigPtr topicConfig = CKafkaPubSubClientTopicConfig::CreateSharedObj();
                    topicConfig->LoadConfig( *templateTopicConfig.GetPointerAlways() ); 
                    topicConfig->topicName = (*i);

                    CKafkaPubSubClientTopicPtr tAccess;
                    {
                        MT::CObjectScopeLock lock( this );

                        tAccess = ( GUCEF_NEW CKafkaPubSubClientTopic( this ) )->CreateSharedPtr();
                        if ( tAccess->LoadConfig( *topicConfig ) )
                        {
                            m_topicMap[ topicConfig->topicName ] = tAccess;                            
                            topicAccess.insert( tAccess );
                            m_config.topics.push_back( topicConfig );
                            ++newTopicAccessCount;

                            ConfigureJournal( tAccess, topicConfig );
                            PUBSUB::CIPubSubJournalBasicPtr journal = tAccess->GetJournal();
                            if ( !journal.IsNULL() && topicConfig->journalConfig.useJournal )
                                journal->AddTopicCreatedJournalEntry();

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created topic \"" +
                                    topicConfig->topicName + "\" based on template config \"" + templateTopicConfig->topicName + "\"" );
                        }
                        else
                        {
                            tAccess.Unlink();
                            totalSuccess = false;

                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Failed to load config for topic \"" +
                                    topicConfig->topicName + "\" based on template config \"" + templateTopicConfig->topicName + "\"" );
                        }
                    }
                    ++i;
                }
            }
            ++m;
        }
    }

    if ( newTopicAccessCount > 0 )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created " +
            CORE::ToString( newTopicAccessCount ) + " topics based on template configs" );

        TopicsAccessAutoCreatedEventData eData( topicAccess );
        NotifyObservers( TopicsAccessAutoCreatedEvent, &eData );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::AutoCreateMultiTopicAccess( CKafkaPubSubClientTopicConfigPtr templateTopicConfig ,
                                                const CORE::CString::StringSet& topicNameList        ,
                                                PubSubClientTopicSet& topicAccess                    ,
                                                CORE::PulseGeneratorPtr pulseGenerator               )
{GUCEF_TRACE;

    TTopicConfigPtrToStringSetMap topicToCreate;
    topicToCreate[ templateTopicConfig ] = topicNameList;
    return AutoCreateMultiTopicAccess( topicToCreate, topicAccess, pulseGenerator );
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::CreateMultiTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                            PubSubClientTopicSet& topicAccess               ,
                                            CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicConfig->topicName.HasChar( '*' ) > -1               )
    {
        CORE::CString::StringSet topicNameList;
        CORE::CString::StringSet globPatternFilters;
        
        globPatternFilters.insert( topicConfig->topicName );
        
        if ( BeginTopicDiscovery( globPatternFilters ) )
        {
            //m_streamIndexingTimer->SetEnabled( true );

            if ( !topicNameList.empty() )
                return AutoCreateMultiTopicAccess( topicConfig, topicNameList, topicAccess, pulseGenerator );
            return true; // Since its pattern based potential creation at a later time also counts as success
        }
        return false;
    }
    else
    {
        PUBSUB::CPubSubClientTopicBasicPtr tAccess = CreateTopicAccess( topicConfig, pulseGenerator );
        if ( !tAccess.IsNULL() )
        {
            topicAccess.insert( tAccess );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        CKafkaPubSubClientTopicPtr topicAccess = (*i).second;
        m_topicMap.erase( i );

        TopicAccessDestroyedEventData eData( topicName );
        NotifyObservers( TopicAccessDestroyedEvent, &eData );
        
        topicAccess->Shutdown();
        topicAccess.Unlink();        
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CKafkaPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr 
CKafkaPubSubClient::GetTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        if ( topicName == (*i)->topicName )
        {
            return (*i);
        }
        ++i;
    }
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr 
CKafkaPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CKafkaPubSubClientTopicConfigPtr newTopicConfig = CKafkaPubSubClientTopicConfig::CreateSharedObj();
    if ( !newTopicConfig.IsNULL() && 
         !m_config.defaultTopicConfig.IsNULL() && 
         newTopicConfig->LoadConfig( *m_config.defaultTopicConfig ) )
    {
        newTopicConfig->topicName = topicName;
        m_config.topics.push_back( newTopicConfig );
        return newTopicConfig;
    }
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

bool 
CKafkaPubSubClient::BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;

    try
    {
        RdKafka::Metadata* metaDataInfo = GUCEF_NULL;
        RdKafka::ErrorCode errorCode = m_kafkaProducer->metadata( true, GUCEF_NULL, &metaDataInfo, 10000 );
        if ( RdKafka::ERR_NO_ERROR == errorCode )
        {
                
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClient:BeginTopicDiscovery: Failed to obtain metadata from Kafka" );
        }
        
        delete metaDataInfo;
        metaDataInfo = GUCEF_NULL;

        return true;
    }
    catch ( const std::exception& )
    {
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        topicNameList.insert( (*i)->topicName );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicNameList.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKafkaPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::SetupKafkaMetaDataAccess( void )
{GUCEF_TRACE;

    if ( m_config.desiredFeatures.supportsDiscoveryOfAvailableTopics )
    {
        // The library wants the addresses as a csv list on its config obj
        // we convert and prep as such
        CORE::CString csvKafkaBrokerList;
        PUBSUB::CPubSubClientConfig::THostAddressVector::const_iterator h = m_config.remoteAddresses.begin();
        while ( h != m_config.remoteAddresses.end() )
        {
            // The RdKafka library will re-resolve DNSs on reconnects so we should feed it DNSs
            csvKafkaBrokerList += (*h).HostnameAndPortAsString() + ',';
            ++h;
        }

        std::string errStr;

        RdKafka::Conf* kafkaConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        kafkaConf->set( "metadata.broker.list", csvKafkaBrokerList, errStr );

        CKafkaPubSubClientConfig::StringMap::const_iterator m = m_config.kafkaProducerGlobalConfigSettings.begin();
        while ( m != m_config.kafkaProducerGlobalConfigSettings.end() )
        {
            if (RdKafka::Conf::CONF_OK != kafkaConf->set((*m).first, (*m).second, errStr ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClient:LoadConfig: Failed to set Kafka Producer global config entry \"" +
                    (*m).first + "\"=\"" + (*m).second + "\", error message: " + errStr );
                ++m_kafkaErrorReplies;
                return false;
            }
            ++m;
        }
        GUCEF_DELETE m_kafkaProducerConf;
        m_kafkaProducerConf = kafkaConf;

        RdKafka::Producer* producer = RdKafka::Producer::create( m_kafkaProducerConf, errStr );
        if ( producer == GUCEF_NULL )
        {
            GUCEF_ERROR_LOG(CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClient:LoadConfig: Failed to create Kafka producer, error message: " + errStr );
            ++m_kafkaErrorReplies;
            return false;
        }
        m_kafkaProducer = producer;
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient:LoadConfig: Successfully created Kafka producer" );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::SetupBasedOnConfig( void )
{GUCEF_TRACE;

    if ( m_config.pulseGenerator.IsNULL() )
    {
        // Use the global pulse generator.
        // NOT what you want if you want thread isolation or basically any time you are not
        // writing a single threaded app
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient:SetupBasedOnConfig: No pulseGenerator provided, will fall back to global one" );
        m_config.pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    }
        
    if ( m_config.desiredFeatures.supportsMetrics )
    {
        m_metricsTimer = GUCEF_NEW CORE::CTimer( m_config.pulseGenerator, 1000 );
        m_metricsTimer->SetEnabled( m_config.desiredFeatures.supportsMetrics );
    }

    RegisterEventHandlers();

    if ( m_config.remoteAddresses.empty() )
    {
		GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "KafkaPubSubClient:SetupBasedOnConfig: No remote addresses have been provided" );
        return false;
    }

    m_config.metricsPrefix += "kafka.";

    SetupKafkaMetaDataAccess();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    Clear();
    
    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    CKafkaPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        MT::CScopeMutex lock( m_lock );
        m_config = parsedCfg;
        return SetupBasedOnConfig();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg  )
{GUCEF_TRACE;

    Clear();
    
    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    CKafkaPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        MT::CScopeMutex lock( m_lock );
        m_config = parsedCfg;
        return SetupBasedOnConfig();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( IsConnected() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):Disconnect: Beginning topic disconnect" );
        
        bool totalSuccess = true;
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            totalSuccess = (*i).second->Disconnect() && totalSuccess;
            ++i;
        }

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):Disconnect: Finished topic disconnect" );

        return totalSuccess;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::Connect( bool reset )
{GUCEF_TRACE;

    // check if there is any work to do
    // in the case of a reset we always do all the work
    if ( !reset && IsConnectedAndSubscribedAsNeeded() )
        return true;

    if ( reset )
    {
        // Only reset the already connected connections if we are doing a reset
        // otherwise treat it as an additive connection attempt for topics
        if ( !Disconnect() )
            return false;
    }
    
    MT::CScopeMutex lock( m_lock );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):Connect: Beginning topic connect" );
        
    bool totalSuccess = true;
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        if ( !(*i).second->IsConnected() )
        {
            if ( (*i).second->IsSubscribingSupported() )
            {
                totalSuccess = (*i).second->Subscribe() && totalSuccess;
            }
        }
        ++i;
    }

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient(" + CORE::PointerToString( this ) + "):Connect: Finished topic connect" );

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::IsConnected( void ) const 
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( m_topicMap.empty() )
        return false;
    
    bool fullyConnected = true;
    TTopicMap::const_iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        fullyConnected = (*i).second->IsConnected() && fullyConnected;
        ++i;
    }
    return fullyConnected;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::IsHealthy( void ) const 
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( m_topicMap.empty() )
        return true;
    
    // Aggregate the health status of all topics
    bool fullyHealthy = true;
    TTopicMap::const_iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        fullyHealthy = (*i).second->IsHealthy() && fullyHealthy;
        ++i;
    }
    
    // Notify if there was a change in status
    if ( fullyHealthy != m_isHealthy )
    {
        m_isHealthy = fullyHealthy;        

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient:IsHealthy: overall health is now Ok" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "KafkaPubSubClient:IsHealthy: overall health status is now unhealthy" );         
        }

        lock.EarlyUnlock();
        THealthStatusChangeEventData eData( fullyHealthy ); 
        NotifyObservers( HealthStatusChangeEvent, &eData );         
    }
    
    return fullyHealthy;
}

/*-------------------------------------------------------------------------*/

bool
CKafkaPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_metricsTimer )
    {
        TEventCallback callback( this, &CKafkaPubSubClient::OnMetricsTimerCycle );
        SubscribeTo( m_metricsTimer                 ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
}

/*-------------------------------------------------------------------------*/

void 
CKafkaPubSubClient::RegisterTopicEventHandlers( CKafkaPubSubClientTopicPtr& topic )
{GUCEF_TRACE;

    if ( !topic.IsNULL() )
    {
        TEventCallback callback( this, &CKafkaPubSubClient::OnTopicHealthStatusChange );
        SubscribeTo( topic.GetPointerAlways()                         ,
                     CKafkaPubSubClientTopic::HealthStatusChangeEvent ,
                     callback                                         );
    }
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // (Re)determine the aggregate health status
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CKafkaPubSubClient::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        const CKafkaPubSubClientTopic::TopicMetrics& metrics = (*i).second->GetMetrics();
        const CORE::CString& topicName = (*i).second->GetTopicName();

        GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaErrorReplies", metrics.kafkaErrorReplies, 1.0f );
        GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaConnectionErrors", metrics.kafkaConnectionErrors, 1.0f );

        if ( m_config.desiredFeatures.supportsPublishing )
        {
            GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesTransmitted", metrics.kafkaMessagesTransmitted, 1.0f );
            GUCEF_METRIC_GAUGE( m_config.metricsPrefix + topicName + ".kafkaTransmitQueueSize", metrics.kafkaTransmitQueueSize, 1.0f );
            GUCEF_METRIC_GAUGE( m_config.metricsPrefix + topicName + ".kafkaTransmitOverflowQueueSize", metrics.kafkaTransmitOverflowQueueSize, 1.0f );
        }
        if ( m_config.desiredFeatures.supportsSubscribing )
        {
            GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceived", metrics.kafkaMessagesReceived, 1.0f );
            GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesFiltered", metrics.kafkaMessagesFiltered, 1.0f );

            if ( metrics.hasKafkaMessagesReceiveLag )
            {
                GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceiveLag.min", metrics.kafkaMessagesReceiveLagMin, 1.0f );
                GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceiveLag.avg", metrics.kafkaMessagesReceiveLagAvg, 1.0f );
                GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceiveLag.max", metrics.kafkaMessagesReceiveLagMax, 1.0f );
            }
            if ( metrics.hasKafkaMessagesReceiveCommitLag )
            {
                GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceiveCommitLag.min", metrics.kafkaMessagesReceiveCommitLagMin, 1.0f );
                GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceiveCommitLag.avg", metrics.kafkaMessagesReceiveCommitLagAvg, 1.0f );
                GUCEF_METRIC_COUNT( m_config.metricsPrefix + topicName + ".kafkaMessagesReceiveCommitLag.max", metrics.kafkaMessagesReceiveCommitLagMax, 1.0f );
            }
        }

        ++i;
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CKafkaPubSubClient::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CKafkaPubSubClient::Unlock( void ) const 
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CKafkaPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::KAFKA::CKafkaPubSubClient";
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
