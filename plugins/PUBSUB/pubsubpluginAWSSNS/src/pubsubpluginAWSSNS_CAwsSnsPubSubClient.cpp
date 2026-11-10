/*
 *  pubsubpluginAWSSNS: Generic GUCEF COMCORE plugin for providing pubsub via AWS's SNS
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

#include "pubsubpluginAWSSNS_CAwsSnsPubSubClient.h"

#include <aws/sns/model/CreateTopicRequest.h>
#include <aws/sns/model/CreateTopicResult.h>
#include <aws/sns/model/ListTopicsRequest.h>
#include <aws/sns/model/ListTopicsResult.h>
#include <aws/sns/model/GetTopicAttributesRequest.h>
#include <aws/sns/model/GetTopicAttributesResult.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace AWSSNS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAwsSnsPubSubClient::TypeName = "AwsSns";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAwsSnsPubSubClient::CAwsSnsPubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_config( config )
    , m_metricsTimer( GUCEF_NULL )
    , m_topicIndexingTimer( GUCEF_NULL )
    , m_topicMap()
    , m_snsClient( PLUGINGLUE::AWSSDK::CAwsSdkGlobal::Instance()->GetDefaultAwsClientConfig() )
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "AwsSnsPubSubClient: Failed to load config at construction" );
    }

    ConfigureJournal( m_config );

    if ( m_config.pulseGenerator.IsNULL() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient: No pulseGenerator provided, will fall back to global one" );
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
        m_topicIndexingTimer = GUCEF_NEW CORE::CTimer( m_config.pulseGenerator, 100000 );
    }

    m_config.metricsPrefix += "sns.";

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClient::~CAwsSnsPubSubClient()
{GUCEF_TRACE;
    
    GUCEF_DELETE m_metricsTimer;
    m_metricsTimer = GUCEF_NULL;

    GUCEF_DELETE m_topicIndexingTimer;
    m_topicIndexingTimer = GUCEF_NULL;

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
CAwsSnsPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    return SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
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
CAwsSnsPubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
{GUCEF_TRACE;

    features.supportsBinaryPayloads = false;             // A SNS message can include only XML, JSON, and unformatted text, so only text and a limited subset of Unicode
    features.supportsPerMsgIds = true;                   // Supported but system generated only
    features.supportsPrimaryPayloadPerMsg = true;        // This is the primary way to send the payload in SNS, key-value attribute support is merely supplemental
    features.supportsAbsentPrimaryPayloadPerMsg = false; // With SNS the primary payload is not optional
    features.supportsKeyValueSetPerMsg = true;           // Supported in SNS using message attributes, up to 10 per message
    features.supportsMetaDataKeyValueSetPerMsg = true;   // Also carried as message attributes
    features.supportsDuplicateKeysPerMsg = false;        // Since attributes are a map of keys to a value it mandates that every key is unique
    features.supportsMultiHostSharding = true;           // SNS is a managed service which under the covers is spread across hardware/nodes
    features.supportsPublishing = true;                  // SNS supports publishing messages to a topic
    features.supportsSubscribing = false;                // SNS pushes messages to its subscribers (such as SQS queues), a client cannot pull messages from a SNS topic
    features.supportsMetrics = true;                     // We add our own metrics support in this plugin for SNS specific stats
    features.supportsAutoReconnect = true;               // Our plugin adds auto 'reconnect' out of the box by virtue of the AWS SDK
    features.supportsSubscriberMsgReceivedAck = false;   // No subscribing
    features.supportsAutoMsgReceivedAck = false;
    features.supportsAbsentMsgReceivedAck = false;
    features.supportsAckUsingLastMsgInBatch = false;
    features.supportsAckUsingBookmark = false;
    features.supportsBookmarkingConcept = false;
    features.supportsSubscribingUsingBookmark = false;
    features.supportsServerSideBookmarkPersistance = false;
    features.supportsAutoBookmarking = false;
    features.supportsMsgIdBasedBookmark = false;
    features.supportsMsgIndexBasedBookmark = false;
    features.supportsTopicIndexBasedBookmark = false;
    features.supportsMsgDateTimeBasedBookmark = false;
    features.supportsDiscoveryOfAvailableTopics = true;  // All accessable SNS topics can be discovered
    features.supportsGlobPatternTopicNames = true;       // We support pattern matching based topic creation
    features.supportsSubscriptionMsgArrivalDelayRequests = false;
    features.supportsSubscriptionEndOfDataEvent = false;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CAwsSnsPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
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
        CAwsSnsPubSubClientTopicPtr topicAccess;
        {
            MT::CObjectScopeLock lock( this );

            topicAccess = ( GUCEF_NEW CAwsSnsPubSubClientTopic( this ) )->CreateSharedPtr();
            if ( topicAccess->LoadConfig( *topicConfig ) )
            {
                m_topicMap[ topicConfig->topicName ] = topicAccess;

                ConfigureJournal( topicAccess, topicConfig );
                PUBSUB::CIPubSubJournalBasicPtr journal = topicAccess->GetJournal();
                if ( !journal.IsNULL() && topicConfig->journalConfig.useJournal )
                    journal->AddTopicCreatedJournalEntry();

                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::PointerToString( this ) + "):CreateTopicAccess: created topic access for topic \"" + topicConfig->topicName + "\"" );

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
CAwsSnsPubSubClient::GetTopicAccess( const CORE::CString& topicName )
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
CAwsSnsPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
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
CAwsSnsPubSubClient::GetMultiTopicAccess( const CORE::CString& topicName    ,
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
CAwsSnsPubSubClient::GetMultiTopicAccess( const CORE::CString::StringSet& topicNames ,
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
CAwsSnsPubSubClient::AutoCreateMultiTopicAccess( const TTopicConfigPtrToStringSetMap& topicsToCreate ,
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
                    CAwsSnsPubSubClientTopicConfigPtr topicConfig = CAwsSnsPubSubClientTopicConfig::CreateSharedObj();
                    topicConfig->LoadConfig( *templateTopicConfig.GetPointerAlways() ); 
                    topicConfig->topicName = (*i);

                    CAwsSnsPubSubClientTopicPtr tAccess;
                    {
                        MT::CObjectScopeLock lock( this );

                        tAccess = ( GUCEF_NEW CAwsSnsPubSubClientTopic( this ) )->CreateSharedPtr();
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

                            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created topic \"" +
                                    topicConfig->topicName + "\" based on template config \"" + templateTopicConfig->topicName + "\"" );
                        }
                        else
                        {
                            tAccess.Unlink();
                            totalSuccess = false;

                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Failed to load config for topic \"" +
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
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::PointerToString( this ) + "):AutoCreateMultiTopicAccess: Auto created " +
            CORE::ToString( newTopicAccessCount ) + " topics based on template configs" );

        TopicsAccessAutoCreatedEventData eData( topicAccess );
        NotifyObservers( TopicsAccessAutoCreatedEvent, &eData );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::AutoCreateMultiTopicAccess( CAwsSnsPubSubClientTopicConfigPtr templateTopicConfig ,
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
CAwsSnsPubSubClient::CreateMultiTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                             PubSubClientTopicSet& topicAccess               ,
                                             CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    if ( m_config.desiredFeatures.supportsGlobPatternTopicNames &&
         topicConfig->topicName.HasChar( '*' ) > -1               )
    {
        CORE::CString::StringSet topicNameList;
        if ( ListAllTopics( topicConfig->topicName ,
                            topicNameList          ) )
        {
            if ( GUCEF_NULL != m_topicIndexingTimer )
                m_topicIndexingTimer->SetEnabled( true );

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
CAwsSnsPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    {
        MT::CObjectScopeLock lock( this );

        TTopicMap::iterator i = m_topicMap.find( topicName );
        if ( i != m_topicMap.end() )
        {
            CAwsSnsPubSubClientTopicPtr topicAccess = (*i).second;
            m_topicMap.erase( i );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::ToString( this ) + "):DestroyTopicAccess: destroyed topic access for topic \"" + topicName + "\"" );

            topicAccess->Shutdown();
            topicAccess.Unlink();
        }
    }

    TopicAccessDestroyedEventData eData( topicName );
    NotifyObservers( TopicAccessDestroyedEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClient::AutoDestroyTopicAccess( const CORE::CString::StringSet& topicNames )
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
                CAwsSnsPubSubClientTopicPtr tAccess = (*i).second;
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
                    CAwsSnsPubSubClientTopicBasicPtr topicAccess = tAccess.StaticCast< CAwsSnsPubSubClientTopic >();
                    topicAccess->Shutdown();
                    topicAccess.Unlink();
                }
                tAccess.Unlink();

                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::ToString( this ) + "):AutoDestroyTopicAccess: destroyed topic access for topic \"" + topicName + "\"" );

                ++destroyedTopicAccessCount;
                ++t;
            }

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::ToString( this ) + "):AutoDestroyTopicAccess: destroyed topic access for " +
                CORE::ToString( destroyedTopicAccessCount ) + "topics" );
        }
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CAwsSnsPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_config.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CAwsSnsPubSubClient::GetTopicConfig( const CORE::CString& topicName )
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
CAwsSnsPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CAwsSnsPubSubClientTopicConfigPtr newTopicConfig = CAwsSnsPubSubClientTopicConfig::CreateSharedObj();
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
CAwsSnsPubSubClient::GetSnsTopicArnForTopicName( const CORE::CString& topicName ,
                                                 bool createIfMissing           )
{GUCEF_TRACE;

    // A topic can be given by its ARN, in which case there is nothing to look up
    if ( topicName.HasSubstr( "arn:", true ) == 0 )
        return topicName.C_String();

    try
    {
        // SNS has no 'get by name' operation: topic ARNs end with the name of the topic so we look through the list of topics
        Aws::SNS::Model::ListTopicsRequest listRequest;
        while ( true )
        {
            Aws::SNS::Model::ListTopicsOutcome listOutcome = m_snsClient.ListTopics( listRequest );
            if ( !listOutcome.IsSuccess() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: Error listing topics: " + CORE::ToString( listOutcome.GetError().GetMessage() ) );
                break;
            }

            const Aws::Vector< Aws::SNS::Model::Topic >& topics = listOutcome.GetResult().GetTopics();
            Aws::Vector< Aws::SNS::Model::Topic >::const_iterator i = topics.begin();
            while ( i != topics.end() )
            {
                const Aws::String& arn = (*i).GetTopicArn();
                if ( GetTopicNameFromArn( arn ) == topicName.C_String() )
                {
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: Resolved topic name \"" + topicName + "\" to ARN: " + CORE::ToString( arn ) );
                    return arn;
                }
                ++i;
            }

            const Aws::String& nextToken = listOutcome.GetResult().GetNextToken();
            if ( nextToken.empty() )
                break;
            listRequest.SetNextToken( nextToken );
        }

        if ( createIfMissing )
        {
            // creating a topic is idempotent, if it exists already we simply get its ARN back
            Aws::SNS::Model::CreateTopicRequest createRequest;
            createRequest.SetName( topicName.C_String() );
            Aws::SNS::Model::CreateTopicOutcome createOutcome = m_snsClient.CreateTopic( createRequest );
            if ( createOutcome.IsSuccess() )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: Created topic \"" + topicName + "\" with ARN: " + CORE::ToString( createOutcome.GetResult().GetTopicArn() ) );
                return createOutcome.GetResult().GetTopicArn();
            }
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: Error creating topic \"" + topicName + "\": " + CORE::ToString( createOutcome.GetError().GetMessage() ) );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: No topic with name \"" + topicName + "\" was found" );
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, CORE::CString( "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: experienced an exception: " ) + e.what() );
    }
    catch ( ... )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_CRITICAL, "AwsSnsPubSubClient:GetSnsTopicArnForTopicName: experienced an unknown exception, your application may be unstable" );
    }
    return Aws::String();
}

/*-------------------------------------------------------------------------*/

std::string
CAwsSnsPubSubClient::GetTopicNameFromArn( const std::string& topicArn )
{GUCEF_TRACE;

    // A topic ARN looks like arn:aws:sns:region:account-id:topicname
    size_t pos = topicArn.find_last_of( ':' );
    if ( pos != std::string::npos )
        return topicArn.substr( pos + 1 );
    return topicArn;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::ListAllTopics( const CORE::CString::StringSet& globPatternFilters ,
                                    CORE::CString::StringSet& topicNames               )
{GUCEF_TRACE;

    try
    {
        Aws::SNS::Model::ListTopicsRequest listRequest;
        while ( true )
        {
            Aws::SNS::Model::ListTopicsOutcome listOutcome = m_snsClient.ListTopics( listRequest );
            if ( !listOutcome.IsSuccess() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSnsPubSubClient:ListAllTopics: Error listing topics: " + CORE::ToString( listOutcome.GetError().GetMessage() ) );
                return false;
            }

            const Aws::Vector< Aws::SNS::Model::Topic >& topics = listOutcome.GetResult().GetTopics();
            Aws::Vector< Aws::SNS::Model::Topic >::const_iterator i = topics.begin();
            while ( i != topics.end() )
            {
                CORE::CString topicName = GetTopicNameFromArn( (*i).GetTopicArn() );
                if ( !globPatternFilters.empty() )
                {
                    if ( topicName.WildcardEquals( globPatternFilters ) )
                        topicNames.insert( topicName );
                }
                else
                {
                    topicNames.insert( topicName );
                }
                ++i;
            }

            const Aws::String& nextToken = listOutcome.GetResult().GetNextToken();
            if ( nextToken.empty() )
                break;
            listRequest.SetNextToken( nextToken );
        }
        return true;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSnsPubSubClient:ListAllTopics: Exception occured: " + CORE::CString( e.what() ) );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::ListAllTopics( const CORE::CString& globPatternFilter ,
                                    CORE::CString::StringSet& topicNames   )
{GUCEF_TRACE;

    CORE::CString::StringSet filters;
    if ( !globPatternFilter.IsNULLOrEmpty() )
        filters.insert( globPatternFilter );
    return ListAllTopics( filters, topicNames );
}

/*-------------------------------------------------------------------------*/

bool 
CAwsSnsPubSubClient::BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters )
{GUCEF_TRACE;

    TopicDiscoveryEventData topicNames;
    if ( ListAllTopics( globPatternFilters, topicNames ) && !topicNames.empty() )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSnsPubSubClient:BeginTopicDiscovery: obtained list of " +
                CORE::ToString( topicNames.size() ) + " topic names to be used as topic names" );
                
        NotifyObservers( TopicDiscoveryEvent, &topicNames );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
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
CAwsSnsPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicNameList.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

Aws::SNS::SNSClient&
CAwsSnsPubSubClient::GetSnsClient( void )
{GUCEF_TRACE;

    return m_snsClient;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CAwsSnsPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    return m_config.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    cfg = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
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
CAwsSnsPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg  )
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    m_config = cfg;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::Connect( bool reset )
{GUCEF_TRACE;

    if ( !reset && IsConnected() )
        return true;

    if ( !Disconnect() )
        return false;

    // The SNS client works on a per action basis as request-response
    // there is currently no persistent connect aside from a long poll for subscription use cases
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;

    return IsInitialized();
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::IsInitialized( void ) const
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
CAwsSnsPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_metricsTimer )
    {
        TEventCallback callback( this, &CAwsSnsPubSubClient::OnMetricsTimerCycle );
        SubscribeTo( m_metricsTimer                 ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
    if ( GUCEF_NULL != m_topicIndexingTimer )
    {
        TEventCallback callback( this, &CAwsSnsPubSubClient::OnTopicIndexingTimerCycle );
        SubscribeTo( m_topicIndexingTimer           ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
}

/*-------------------------------------------------------------------------*/

void 
CAwsSnsPubSubClient::RegisterTopicEventHandlers( CAwsSnsPubSubClientTopicPtr& topic )
{GUCEF_TRACE;

    if ( !topic.IsNULL() )
    {
        TEventCallback callback( this, &CAwsSnsPubSubClient::OnTopicHealthStatusChange );
        SubscribeTo( topic.GetPointerAlways()                          ,
                     CAwsSnsPubSubClientTopic::HealthStatusChangeEvent ,
                     callback                                          );
    }
}

/*-------------------------------------------------------------------------*/

CAwsSnsPubSubClientTopicConfigPtr
CAwsSnsPubSubClient::FindTemplateConfigForTopicName( const CORE::CString& topicName ) const
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
    return CAwsSnsPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

bool
CAwsSnsPubSubClient::GetAllGlobPatternTopicNames( CORE::CString::StringSet& allGlobPatternTopicNames )
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
CAwsSnsPubSubClient::OnTopicIndexingTimerCycle( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AwsSnsPubSubClient(" + CORE::ToString( this ) + "):OnTopicIndexingTimerCycle: Checking for new topics matching glob pattterns" );

    // First get a new list of all glob patterns we need to match
    CORE::CString::StringSet allGlobPatternTopicNames;
    if( !GetAllGlobPatternTopicNames( allGlobPatternTopicNames ) )
        return;

    // Next we will need to fetch all stream names in the cluster
    CORE::CString::StringSet allMatchingTopicNames;
    if ( ListAllTopics( allGlobPatternTopicNames ,
                        allMatchingTopicNames    ) )
    {
        // Check created topics, filtering the ones we already created
        TTopicMap::iterator t = m_topicMap.begin();
        while ( t != m_topicMap.end() )
        {
            CORE::CString::StringSet::iterator i = allMatchingTopicNames.find( (*t).first );
            if ( i != allMatchingTopicNames.end() )
            {
                allMatchingTopicNames.erase( i );
            }
            ++t;
        }

        // Now we automatically create topic access for all the remaining topics
        if ( !allMatchingTopicNames.empty() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient(" + CORE::ToString( this ) + "):OnTopicIndexingTimerCycle: Found " +
                CORE::ToString( allMatchingTopicNames.size() ) + " new topics that match glob patterns. Will attempt to auto create topics for these" );
        }

        CORE::CString::StringSet::iterator n = allMatchingTopicNames.begin();
        while ( n != allMatchingTopicNames.end() )
        {
            CAwsSnsPubSubClientTopicConfigPtr templateConfig = FindTemplateConfigForTopicName( (*n) );
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
CAwsSnsPubSubClient::TryGetTopicAttributes( const std::string& topicArn          ,
                                            CORE::CString::StringMap& attributes )
{GUCEF_TRACE;

    try
    {
        Aws::SNS::Model::GetTopicAttributesRequest getRequest;
        getRequest.SetTopicArn( topicArn );

        Aws::SNS::Model::GetTopicAttributesOutcome getOutcome = m_snsClient.GetTopicAttributes( getRequest );
        if ( getOutcome.IsSuccess() )
        {
            const Aws::Map< Aws::String, Aws::String >& awsAttributes = getOutcome.GetResult().GetAttributes();
            Aws::Map< Aws::String, Aws::String >::const_iterator i = awsAttributes.begin();
            while ( i != awsAttributes.end() )
            {
                attributes[ (*i).first.c_str() ] = (*i).second.c_str();
                ++i;
            }
            return true;
        }

        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:TryGetTopicAttributes: Error getting topic attributes: " + CORE::ToString( getOutcome.GetError().GetMessage() ) );
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "AwsSnsPubSubClient:TryGetTopicAttributes: exception occured: " + CORE::CString( e.what() ) );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClient::OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                                                const CORE::CEvent& eventId  ,
                                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // (Re)determine the aggregate health status
    IsHealthy();
}

/*-------------------------------------------------------------------------*/

void
CAwsSnsPubSubClient::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
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
CAwsSnsPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::AWSSNS::CAwsSnsPubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSNS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
