/*
 *  pubsubpluginAWSSQS: Generic GUCEF COMCORE plugin for providing pubsub via AWS's SQS
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

#ifndef GUCEF_PLUGINGLUE_AWSSDK_CAWSSDKGLOBAL_H
#include "pluginglueAWSSDK_CAwsSdkGlobal.h"
#define GUCEF_PLUGINGLUE_AWSSDK_CAWSSDKGLOBAL_H
#endif /* GUCEF_PLUGINGLUE_AWSSDK_CAWSSDKGLOBAL_H ? */

#include "pubsubpluginAWSSQS_CAwsSqsPubSubClient.h"

#include <aws/sqs/model/GetQueueUrlRequest.h>
#include <aws/sqs/model/GetQueueUrlResult.h>
#include <aws/sqs/model/ListQueuesRequest.h>
#include <aws/sqs/model/ListQueuesResult.h>
#include <aws/sqs/model/GetQueueAttributesRequest.h>
#include <aws/sqs/model/GetQueueAttributesResult.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace AWSSQS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAwsSqsPubSubClient::TypeName = "AwsSqs";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAwsSqsPubSubClient::CAwsSqsPubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_config( config )
    , m_metricsTimer( GUCEF_NULL )
    , m_queueIndexingTimer( GUCEF_NULL )
    , m_topicMap()
    , m_sqsClient( PLUGINGLUE::AWSSDK::CAwsSdkGlobal::Instance()->GetDefaultAwsClientConfig() )
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AwsSqsPubSubClient: Failed to load config at construction" );
    }

    ConfigureJournal( m_config );

    if ( m_config.pulseGenerator.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient: No pulseGenerator provided, will fall back to global one" );
        m_config.pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    }

    if ( m_config.topicPulseGenerator.IsNULL() )
        m_config.topicPulseGenerator = m_config.pulseGenerator;

    if ( config.desiredFeatures.supportsMetrics )
    {
        m_metricsTimer = GUCEF_NEW CORE::CTimer( m_config.pulseGenerator, 1000 );
        m_metricsTimer->SetEnabled( config.desiredFeatures.supportsMetrics );
    }
    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames )
    {                                                                      // @TODO: interval
        m_queueIndexingTimer = GUCEF_NEW CORE::CTimer( m_config.pulseGenerator, 100000 );
    }

    m_config.metricsPrefix += "sqs.";

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CAwsSqsPubSubClient::~CAwsSqsPubSubClient()
{GUCEF_TRACE;
    
    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    GUCEF_DELETE m_queueIndexingTimer;
    m_queueIndexingTimer = GUCEF_NULL;

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->Shutdown();
        (*i).second.Unlink();
        ++i;
    }
    m_topicMap.clear();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    return SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                        bool includeTopics                        )
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    
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

PUBSUB::CPubSubClientConfig& 
CAwsSqsPubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
{GUCEF_TRACE;

    features.supportsBinaryPayloads = false;             // Per SendMessage doc: "A message can include only XML, JSON, and unformatted text. The following Unicode characters are allowed", so only text and a tiny subset of Unicode
    features.supportsPerMsgIds = true;                   // Supported but system generated only
    features.supportsPrimaryPayloadPerMsg = true;        // This is the primary way to send the payload in SQS, key-value attribute support is merely supplemental
    features.supportsAbsentPrimaryPayloadPerMsg = false; // With SQS the primary payload is not optional
    features.supportsKeyValueSetPerMsg = true;           // Supported in SQS using message attributes. "Each message can have up to 10 attributes. Message attributes are optional and separate from the message body (however, they are sent alongside it)"
    features.supportsMetaDataKeyValueSetPerMsg = true;   // "Whereas you can use message attributes to attach custom metadata to Amazon SQS messages for your applications, you can use message system attributes to store metadata for other AWS services, such as AWS X-Ray"
    features.supportsDuplicateKeysPerMsg = false;        // Since attributes are a map of keys to a value it mandates that every key is unique
    features.supportsMultiHostSharding = true;           // SQS is a managed service which under the coveres is shared across hardware/nodes
    features.supportsPublishing = true;                  // SQS supports sending messages to the queue
    features.supportsSubscribing = true;                 // SQS supports reading messages from the queue
    features.supportsMetrics = true;                     // We add our own metrics support in this plugin for SQS specific stats
    features.supportsAutoReconnect = true;               // Our plugin adds auto 'reconnect' out of the box by virtue of the AWS SDK
    features.supportsSubscriberMsgReceivedAck = true;    // SQS requires explit messages deletion to signal you are done with the message, this is the ack
    features.supportsAutoMsgReceivedAck = false;         // Since SQS is a queue where you consume the messages: grabbing the message is in a way the ack but this does not really apply
    features.supportsAbsentMsgReceivedAck = false;       // SQS requires explit messages deletion to signal you are done with the message, without that the message will become 'visible' again and retransmitted
    features.supportsAckUsingLastMsgInBatch = false;     // Since SQS is a queue where you consume the messages: this does not apply
    features.supportsAckUsingBookmark = false;           // Since SQS is a queue where you consume the messages: this does not apply
    features.supportsBookmarkingConcept = true;          // Since SQS is a queue where you consume the messages: Your offset is remembered simply due to the nature of a queue
    features.supportsSubscribingUsingBookmark = false;   // Since SQS is a queue where you consume the messages: This is not supported
    features.supportsServerSideBookmarkPersistance = true; // Since SQS is a queue where you consume the messages: Your offset is remembered simply due to the nature of a queue
    features.supportsAutoBookmarking = true;             // Since SQS is a queue where you consume the messages: Your offset is remembered simply due to the nature of a queue
    features.supportsMsgIdBasedBookmark = false;         // Since SQS is a queue where you consume the messages: You cannot provide a msg ID to resume from a given point
    features.supportsMsgIndexBasedBookmark = false;      // Since SQS is a queue where you consume the messages: You cannot provide a msg index to resume from a given point
    features.supportsTopicIndexBasedBookmark = false;    // Since SQS is a queue where you consume the messages: You cannot provide a msg index to resume from a given point
    features.supportsMsgDateTimeBasedBookmark = false;   // Since SQS is a queue where you consume the messages: You cannot provide a datetime to resume from a given point in time
    features.supportsDiscoveryOfAvailableTopics = true;  // All accessable SQS queues can be discovered
    features.supportsGlobPatternTopicNames = true;       // We support pattern matching based topic creation
    features.supportsSubscriptionMsgArrivalDelayRequests = false;
    features.supportsSubscriptionEndOfDataEvent = false;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CAwsSqsPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                        CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    // Check to see if this logical/conceptual 'topic' represents multiple pattern matched Redis Streams
    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicConfig->topicName.HasChar( '*' ) > -1               )
    {
        PubSubClientTopicSet allTopicAccess;
        if ( CreateMultiTopicAccess( topicConfig, allTopicAccess, pulseGenerator ) && !allTopicAccess.empty() )
        {
            // Caller should really use the CreateMultiTopicAccess() variant
            PUBSUB::CPubSubClientTopicBasicPtr tAccess = *allTopicAccess.begin();
            return tAccess;
        }
        return PUBSUB::CPubSubClientTopicPtr();
    }
    else
    {
        CAwsSqsPubSubClientTopicPtr topicAccess;
        {
            MT::CObjectScopeLock lock( this );

            topicAccess = ( GUCEF_NEW CAwsSqsPubSubClientTopic( this ) )->CreateSharedPtr();
            if ( topicAccess->LoadConfig( *topicConfig ) )
            {
                m_topicMap[ topicConfig->topicName ] = topicAccess;

                ConfigureJournal( topicAccess, topicConfig );
                PUBSUB::CIPubSubJournalBasicPtr journal = topicAccess->GetJournal();
                if ( !journal.IsNULL() && topicConfig->journalConfig.useJournal )
                    journal->AddTopicCreatedJournalEntry();

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::PointerToString( this ) + "):CreateTopicAccess: created topic access for topic \"" + topicConfig->topicName + "\"" );

                lock.EarlyUnlock();

                TopicAccessCreatedEventData eData( topicConfig->topicName );
                NotifyObservers( TopicAccessCreatedEvent, &eData );
            }
            else
            {
                topicAccess.Unlink();
            }
        }
        return topicAccess;
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr 
CAwsSqsPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;
        
    MT::CObjectScopeLock lock( this );

    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        return (*i).second;
    }
    return PUBSUB::CPubSubClientTopicBasicPtr();
}

/*-------------------------------------------------------------------------*/

void 
CAwsSqsPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
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
CAwsSqsPubSubClient::GetMultiTopicAccess( const CORE::CString& topicName    ,
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
CAwsSqsPubSubClient::GetMultiTopicAccess( const CORE::CString::StringSet& topicNames ,
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
CAwsSqsPubSubClient::AutoCreateMultiTopicAccess( const TTopicConfigPtrToStringSetMap& topicsToCreate ,
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
                    CAwsSqsPubSubClientTopicConfigPtr topicConfig = CAwsSqsPubSubClientTopicConfig::CreateSharedObj();
                    topicConfig->LoadConfig( *templateTopicConfig.GetPointerAlways() ); 
                    topicConfig->topicName = (*i);

                    CAwsSqsPubSubClientTopicPtr tAccess;
                    {
                        MT::CObjectScopeLock lock( this );

                        tAccess = ( GUCEF_NEW CAwsSqsPubSubClientTopic( this ) )->CreateSharedPtr();
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

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created topic \"" +
                                    topicConfig->topicName + "\" based on template config \"" + templateTopicConfig->topicName + "\"" );
                        }
                        else
                        {
                            tAccess.Unlink();
                            totalSuccess = false;

                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Failed to load config for topic \"" +
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
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created " +
            CORE::ToString( newTopicAccessCount ) + " topics based on template configs" );

        TopicsAccessAutoCreatedEventData eData( topicAccess );
        NotifyObservers( TopicsAccessAutoCreatedEvent, &eData );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::AutoCreateMultiTopicAccess( CAwsSqsPubSubClientTopicConfigPtr templateTopicConfig ,
                                                 const CORE::CString::StringSet& topicNameList         ,
                                                 PubSubClientTopicSet& topicAccess                     ,
                                                 CORE::PulseGeneratorPtr pulseGenerator                )
{GUCEF_TRACE;

    TTopicConfigPtrToStringSetMap topicToCreate;
    topicToCreate[ templateTopicConfig ] = topicNameList;
    return AutoCreateMultiTopicAccess( topicToCreate, topicAccess, pulseGenerator );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::CreateMultiTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                             PubSubClientTopicSet& topicAccess               ,
                                             CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicConfig->topicName.HasChar( '*' ) > -1               )
    {
        CORE::CString::StringSet topicNameList;
        if ( ListAllQueues( topicConfig->topicName ,
                            topicNameList          ) )
        {
            if ( GUCEF_NULL != m_queueIndexingTimer )
                m_queueIndexingTimer->SetEnabled( true );

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
CAwsSqsPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    {
        MT::CObjectScopeLock lock( this );

        TTopicMap::iterator i = m_topicMap.find( topicName );
        if ( i != m_topicMap.end() )
        {
            CAwsSqsPubSubClientTopicPtr topicAccess = (*i).second;
            m_topicMap.erase( i );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::ToString( this ) + "):DestroyTopicAccess: destroyed topic access for topic \"" + topicName + "\"" );

            topicAccess->Shutdown();
            topicAccess.Unlink();
        }
    }

    TopicAccessDestroyedEventData eData( topicName );
    NotifyObservers( TopicAccessDestroyedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::AutoDestroyTopicAccess( const CORE::CString::StringSet& topicNames )
{GUCEF_TRACE;

    PubSubClientTopicSet topicAccess;
    {
        MT::CObjectScopeLock lock( this );

        CORE::CString::StringSet::const_iterator t = topicNames.begin();
        while ( t != topicNames.end() )
        {
            const CORE::CString& topicName = (*t);
            TTopicMap::iterator i = m_topicMap.find( topicName );
            if ( i != m_topicMap.end() )
            {
                CAwsSqsPubSubClientTopicPtr tAccess = (*i).second;
                topicAccess.insert( tAccess );
            }
            ++t;
        }
    }

    if ( !topicAccess.empty() )
    {
        TopicsAccessAutoDestroyedEventData eData( topicAccess );
        NotifyObservers( TopicsAccessAutoDestroyedEvent, &eData );

        {
            // Now that everyone has been duely warned we can proceed with the actual destruction
            MT::CObjectScopeLock lock( this );

            CORE::UInt32 destroyedTopicAccessCount = 0;
            PubSubClientTopicSet::iterator t = topicAccess.begin();
            while ( t != topicAccess.end() )
            {
                PUBSUB::CPubSubClientTopicBasicPtr tAccess = (*t);

                CORE::CString topicName = tAccess->GetTopicName();
                m_topicMap.erase( topicName );                
                {
                    CAwsSqsPubSubClientTopicBasicPtr topicAccess = tAccess.StaticCast< CAwsSqsPubSubClientTopic >();
                    topicAccess->Shutdown();
                    topicAccess.Unlink();
                }
                tAccess.Unlink();

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::ToString( this ) + "):AutoDestroyTopicAccess: destroyed topic access for topic \"" + topicName + "\"" );

                ++destroyedTopicAccessCount;
                ++t;
            }

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::ToString( this ) + "):AutoDestroyTopicAccess: destroyed topic access for " +
                CORE::ToString( destroyedTopicAccessCount ) + "topics" );
        }
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CAwsSqsPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_config.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CAwsSqsPubSubClient::GetTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

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
CAwsSqsPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CAwsSqsPubSubClientTopicConfigPtr newTopicConfig = CAwsSqsPubSubClientTopicConfig::CreateSharedObj();
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

Aws::String
CAwsSqsPubSubClient::GetSqsQueueUrlForQueueName( const CORE::CString& queueName ,
                                                 Aws::SQS::SQSError& errorCode  )
{GUCEF_TRACE;

    try
    {
        Aws::SQS::Model::GetQueueUrlRequest gqu_req;
        gqu_req.SetQueueName( queueName );

        Aws::SQS::Model::GetQueueUrlOutcome gqu_out = m_sqsClient.GetQueueUrl( gqu_req );
        if ( gqu_out.IsSuccess() ) 
        {
            const Aws::String& queueUrl = gqu_out.GetResult().GetQueueUrl();

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient:GetSqsQueueUrlForQueueName: Resolved queue name \"" + queueName + "\" to URL: " + CORE::ToString( queueUrl ) );
            return queueUrl;
        } 
        else 
        {
            errorCode = gqu_out.GetError();
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient:GetSqsQueueUrlForQueueName: Error getting URL for queue name \"" + queueName + "\":" + CORE::ToString( gqu_out.GetError().GetMessage() ) );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSqsPubSubClient:GetSqsQueueUrlForQueueName: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSqsPubSubClient:GetSqsQueueUrlForQueueName: experienced an unknown exception, your application may be unstable" );
    }

    return Aws::String();
}

/*-------------------------------------------------------------------------*/

std::string 
CAwsSqsPubSubClient::GetQueueNameFromUrl( const std::string& queueUrl ) 
{GUCEF_TRACE;

    // Find the last slash in the URL
    size_t pos = queueUrl.find_last_of('/');
    if (pos != std::string::npos) {
        // Extract and return the queue name
        return queueUrl.substr(pos + 1);
    }
    return "";
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::ListAllQueues( const CORE::CString::StringSet& globPatternFilters ,
                                    CORE::CString::StringSet& queueNames               )
{GUCEF_TRACE;

    try
    {
        Aws::SQS::Model::ListQueuesRequest listQueuesRequest;
        Aws::SQS::Model::ListQueuesOutcome listQueuesOutcome = m_sqsClient.ListQueues( listQueuesRequest );

        if ( listQueuesOutcome.IsSuccess() ) 
        {
            const Aws::Vector< Aws::String >& queueUrls = listQueuesOutcome.GetResult().GetQueueUrls();
            Aws::Vector< Aws::String >::const_iterator i = queueUrls.begin();
            while ( i != queueUrls.end() ) 
            {
                const Aws::String& url = (*i);
                CORE::CString queueName = GetQueueNameFromUrl( url );

                if ( !globPatternFilters.empty() )
                {
                    if ( queueName.WildcardEquals( globPatternFilters ) )
                        queueNames.insert( queueName );
                }
                else
                {
                    queueNames.insert( queueName );
                }
                ++i;
            }
            return true;
        } 
        else 
        {

            GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSqsPubSubClient:ListAllQueues: Error listing queues: " +
                    listQueuesOutcome.GetError().GetMessage() );
            return false;
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSqsPubSubClient:ListAllQueues: Exception occured: " + CORE::CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::ListAllQueues( const CORE::CString& globPatternFilter ,
                                    CORE::CString::StringSet& queueNames   )
{GUCEF_TRACE;

    CORE::CString::StringSet filters;
    filters.insert( globPatternFilter );

    return ListAllQueues( filters, queueNames );
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClient::BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;

    TopicDiscoveryEventData topicNames;
    if ( ListAllQueues( globPatternFilters, topicNames ) && !topicNames.empty() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSqsPubSubClient:BeginTopicDiscovery: obtained list of " +
                CORE::ToString( topicNames.size() ) + " queue names to be used as topic names" );
                
        NotifyObservers( TopicDiscoveryEvent, &topicNames );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        topicNameList.insert( (*i)->topicName );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicNameList.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

Aws::SQS::SQSClient&
CAwsSqsPubSubClient::GetSqsClient( void )
{GUCEF_TRACE;

    return m_sqsClient;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CAwsSqsPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    cfg = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    // Try to see if we can properly load the entire config before
    // applying it. If not stick with old config vs corrupt config
    PUBSUB::CPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        //MT::CScopeMutex lock( m_lock );
        m_config = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg  )
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    m_config = cfg;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::Connect( bool reset )
{GUCEF_TRACE;

    if ( !reset && IsConnected() )
        return true;

    if ( !Disconnect() )
        return false;

    // The SQS client works on a per action basis as request-response
    // there is currently no persistent connect aside from a long poll for subscription use cases
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;

    return IsInitialized();
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    if ( PLUGINGLUE::AWSSDK::CAwsSdkGlobal::Instance()->IsIncludedInGlobalBootstrapConfigLoad() )
    {
        if ( PLUGINGLUE::AWSSDK::CAwsSdkGlobal::Instance()->IsGlobalBootstrapConfigLoadInProgress() ||
             PLUGINGLUE::AWSSDK::CAwsSdkGlobal::Instance()->IsGlobalConfigLoadInProgress()           )
        {
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_metricsTimer )
    {
        TEventCallback callback( this, &CAwsSqsPubSubClient::OnMetricsTimerCycle );
        SubscribeTo( m_metricsTimer                 ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
    if ( GUCEF_NULL != m_queueIndexingTimer )
    {
        TEventCallback callback( this, &CAwsSqsPubSubClient::OnQueueIndexingTimerCycle );
        SubscribeTo( m_queueIndexingTimer           ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
}

/*-------------------------------------------------------------------------*/

void 
CAwsSqsPubSubClient::RegisterTopicEventHandlers( CAwsSqsPubSubClientTopicPtr& topic )
{GUCEF_TRACE;

    if ( !topic.IsNULL() )
    {
        TEventCallback callback( this, &CAwsSqsPubSubClient::OnTopicHealthStatusChange );
        SubscribeTo( topic.GetPointerAlways()                          ,
                     CAwsSqsPubSubClientTopic::HealthStatusChangeEvent ,
                     callback                                          );
    }
}

/*-------------------------------------------------------------------------*/

CAwsSqsPubSubClientTopicConfigPtr
CAwsSqsPubSubClient::FindTemplateConfigForTopicName( const CORE::CString& topicName ) const
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );

    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::const_iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        if ( (*i)->topicName.HasChar( '*' ) > -1 )
        {
            if ( topicName.WildcardEquals( (*i)->topicName, '*', true ) )
            {
                return (*i);
            }
        }
        ++i;
    }
    return CAwsSqsPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

bool
CAwsSqsPubSubClient::GetAllGlobPatternTopicNames( CORE::CString::StringSet& allGlobPatternTopicNames )
{GUCEF_TRACE;

    // Check config'd topic
    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_config.topics.begin();
    while ( i != m_config.topics.end() )
    {
        if ( (*i)->topicName.HasChar( '*' ) > -1 )
        {
            allGlobPatternTopicNames.insert( (*i)->topicName );
        }
        ++i;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::OnQueueIndexingTimerCycle( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSqsPubSubClient(" + CORE::ToString( this ) + "):OnQueueIndexingTimerCycle: Checking for new queues matching glob pattterns" );

    // First get a new list of all glob patterns we need to match
    CORE::CString::StringSet allGlobPatternTopicNames;
    if( !GetAllGlobPatternTopicNames( allGlobPatternTopicNames ) )
        return;

    // Next we will need to fetch all stream names in the cluster
    CORE::CString::StringSet allMatchingQueueNames;
    if ( ListAllQueues( allGlobPatternTopicNames ,
                        allMatchingQueueNames    ) )
    {
        // Check created topics, filtering the ones we already created
        TTopicMap::iterator t = m_topicMap.begin();
        while ( t != m_topicMap.end() )
        {
            CORE::CString::StringSet::iterator i = allMatchingQueueNames.find( (*t).first );
            if ( i != allMatchingQueueNames.end() )
            {
                allMatchingQueueNames.erase( i );
            }
            ++t;
        }

        // Now we automatically create topic access for all the remaining topics
        if ( !allMatchingQueueNames.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient(" + CORE::ToString( this ) + "):OnQueueIndexingTimerCycle: Found " +
                CORE::ToString( allMatchingQueueNames.size() ) + " new queues that match glob patterns. Will attempt to auto create topics for these" );
        }

        CORE::CString::StringSet::iterator n = allMatchingQueueNames.begin();
        while ( n != allMatchingQueueNames.end() )
        {
            CAwsSqsPubSubClientTopicConfigPtr templateConfig = FindTemplateConfigForTopicName( (*n) );
            if ( !templateConfig.IsNULL() )
            {
                CORE::CString::StringSet topicNameList;
                topicNameList.insert( (*n) );

                PubSubClientTopicSet topicAccess;

                AutoCreateMultiTopicAccess( templateConfig, topicNameList, topicAccess, m_config.topicPulseGenerator );
            }
            ++n;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClient::IsQueueEmpty( const std::string& queueUrl ) 
{GUCEF_TRACE;

    CORE::Int64 messageCount = -1;
    TryGetNrOfMessagesInQueue( queueUrl, messageCount );
    return 0 == messageCount;         
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClient::TryGetNrOfMessagesInQueue( const std::string& queueUrl ,
                                                CORE::Int64& messageCount   ) 
{GUCEF_TRACE;

    try
    {
        // Create a request to get queue attributes
        Aws::SQS::Model::GetQueueAttributesRequest getQueueAttributesRequest;
        getQueueAttributesRequest.SetQueueUrl( queueUrl );
        getQueueAttributesRequest.AddAttributeNames( Aws::SQS::Model::QueueAttributeName::ApproximateNumberOfMessages );

        // Call GetQueueAttributes
        Aws::SQS::Model::GetQueueAttributesOutcome getQueueAttributesOutcome = m_sqsClient.GetQueueAttributes( getQueueAttributesRequest );

        if ( getQueueAttributesOutcome.IsSuccess() ) 
        {
            const Aws::Map<Aws::SQS::Model::QueueAttributeName, Aws::String>& attributes = getQueueAttributesOutcome.GetResult().GetAttributes();
            auto it = attributes.find( Aws::SQS::Model::QueueAttributeName::ApproximateNumberOfMessages );
            if ( it != attributes.end() ) 
            {
                messageCount = CORE::StringToInt64( it->second, messageCount );
                return true;
            }
        } 
        else 
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient:TryGetNrOfMessagesInQueue: Error getting queue attributes: " +
                getQueueAttributesOutcome.GetError().GetMessage() );
        }
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:TryGetNrOfMessagesInQueue: exception occured: " + CORE::CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSqsPubSubClient::TryGetQueueAttributes( const std::string& queueUrl          ,
                                            CORE::CString::StringMap& attributes ) 
{GUCEF_TRACE;

    try
    {
        // Create a request to get queue attributes
        Aws::SQS::Model::GetQueueAttributesRequest getQueueAttributesRequest;
        getQueueAttributesRequest.SetQueueUrl( queueUrl );
        getQueueAttributesRequest.AddAttributeNames( Aws::SQS::Model::QueueAttributeName::All );

        // Call GetQueueAttributes
        Aws::SQS::Model::GetQueueAttributesOutcome getQueueAttributesOutcome = m_sqsClient.GetQueueAttributes( getQueueAttributesRequest );

        if ( getQueueAttributesOutcome.IsSuccess() ) 
        {
            const Aws::Map<Aws::SQS::Model::QueueAttributeName, Aws::String>& awsAttributes = getQueueAttributesOutcome.GetResult().GetAttributes();

            Aws::Map<Aws::SQS::Model::QueueAttributeName, Aws::String>::const_iterator i = awsAttributes.begin();
            while ( i != awsAttributes.end() )
            {
                Aws::SQS::Model::QueueAttributeName enumName = (*i).first;
                const Aws::String& value = (*i).second;

                switch ( enumName )
                {
                    case Aws::SQS::Model::QueueAttributeName::NOT_SET : attributes[ "NOT_SET" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::All : attributes[ "All" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::Policy : attributes[ "Policy" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::VisibilityTimeout : attributes[ "VisibilityTimeout" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::MaximumMessageSize : attributes[ "MaximumMessageSize" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::MessageRetentionPeriod : attributes[ "MessageRetentionPeriod" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ApproximateNumberOfMessages : attributes[ "ApproximateNumberOfMessages" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ApproximateNumberOfMessagesNotVisible : attributes[ "ApproximateNumberOfMessagesNotVisible" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::CreatedTimestamp : attributes[ "CreatedTimestamp" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::LastModifiedTimestamp : attributes[ "LastModifiedTimestamp" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::QueueArn : attributes[ "QueueArn" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ApproximateNumberOfMessagesDelayed : attributes[ "ApproximateNumberOfMessagesDelayed" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::DelaySeconds : attributes[ "DelaySeconds" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ReceiveMessageWaitTimeSeconds : attributes[ "ReceiveMessageWaitTimeSeconds" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::RedrivePolicy : attributes[ "RedrivePolicy" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::FifoQueue : attributes[ "FifoQueue" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ContentBasedDeduplication : attributes[ "ContentBasedDeduplication" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::KmsMasterKeyId : attributes[ "KmsMasterKeyId" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::KmsDataKeyReusePeriodSeconds : attributes[ "KmsDataKeyReusePeriodSeconds" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::DeduplicationScope : attributes[ "DeduplicationScope" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::FifoThroughputLimit : attributes[ "FifoThroughputLimit" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::RedriveAllowPolicy : attributes[ "RedriveAllowPolicy" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::SqsManagedSseEnabled : attributes[ "SqsManagedSseEnabled" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::SentTimestamp : attributes[ "SentTimestamp" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ApproximateFirstReceiveTimestamp : attributes[ "ApproximateFirstReceiveTimestamp" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::ApproximateReceiveCount : attributes[ "ApproximateReceiveCount" ] = value; break;
                    case Aws::SQS::Model::QueueAttributeName::SenderId : attributes[ "SenderId" ] = value; break;

                    default:
                    {
                        attributes[ CORE::ToString( static_cast< CORE::Int64 >( enumName ) ) ] = value;
                        break;
                    }
                }

                ++i;
            }
            return true;
        } 
        else 
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClient:TryGetQueueAttributes: Error getting queue attributes: " +
                getQueueAttributesOutcome.GetError().GetMessage() );
        }
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "AwsSqsPubSubClientTopic:TryGetQueueAttributes: exception occured: " + CORE::CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // (Re)determine the aggregate health status
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CAwsSqsPubSubClient::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnMetricsTimerCycle( notifier  ,
                                          eventId   ,
                                          eventData );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CAwsSqsPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::AWSSQS::CAwsSqsPubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSQS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
