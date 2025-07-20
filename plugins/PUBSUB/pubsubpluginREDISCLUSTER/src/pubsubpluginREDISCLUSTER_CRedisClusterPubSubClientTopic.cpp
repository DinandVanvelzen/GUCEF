/*
 *  pubsubpluginREDISCLUSTER: Generic GUCEF COMCORE plugin for providing pubsub via Redis
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

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENT_H
#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClient.h"
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENT_H ? */

#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClientTopic.h"

#include "crc16.h"
#include "redis_cluster.hpp"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF::REDISINFO; 

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace REDISCLUSTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopic::CRedisClusterPubSubClientTopic( CRedisClusterPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CRedisClusterPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_redisPipeline( GUCEF_NULL )
    , m_redisContext()
    , m_redisErrorReplies( 0 )
    , m_redisTimeoutErrors( 0 )
    , m_msgsReceived( 0 )
    , m_fieldsInMsgsReceived( 0 )
    , m_msgsBytesReceived( 0 )
    , m_redisMsgsTransmitted( 0 )
    , m_redisFieldsInMsgsTransmitted( 0 )
    , m_redisHashSlot( GUCEFMT_UINT32MAX )
    , m_redisMaxXreadCount( 1000 )
    , m_redisXreadBlockTimeoutInMs( 1000 )
    , m_redisMsgArgs()
    , m_redisShardHost()
    , m_redisShardNodeId()
    , m_redisReconnectTimer( GUCEF_NULL )
    , m_config()
    , m_journal()
    , m_readerThread()
    , m_needToTrackAcks( true )
    , m_subscriptionIsAtEndOfData( false )
    , m_isSubscribed( false )
    , m_maxTotalMsgsInFlight( 1000 )
    , m_msgsInFlight( 0 )
    , m_currentPublishActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
    , m_metrics()
    , m_metricFriendlyTopicName()
    , m_metricsPrefix()
    , m_isHealthy( true )
    , m_lock()
{GUCEF_TRACE;

    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    if ( client->GetConfig().desiredFeatures.supportsAutoReconnect )
    {
        m_redisReconnectTimer = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), client->GetConfig().reconnectDelayInMs );
    }

    m_needToTrackAcks = m_client->IsTrackingAcksNeeded();

    RegisterEventHandlers();
}

/*-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopic::~CRedisClusterPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopic::RedisTopicVarsForReading::RedisTopicVarsForReading( void )
    : m_redisReadMsgs()
    , m_pubsubMsgs()
    , m_pubsubMsgsRefs()
    , m_pubsubMsgAttribs()
    , m_readOffset( "0" )
    , m_currentReceiveActionId( 1 )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_client = GUCEF_NULL;

    delete m_redisReconnectTimer;
    m_redisReconnectTimer = GUCEF_NULL;
    
    CleanupRedisReaderThread();
    Disconnect();    

    if ( !m_journal.IsNULL() )
    {
        m_journal->AddTopicDestroyedJournalEntry();
        m_journal.Unlink();
    }

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void 
CRedisClusterPubSubClientTopic::RegisterPulseGeneratorEventHandlers( CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    UnsubscribeFrom( GetPulseGenerator().GetPointerAlways() );
    TEventCallback callback( this, &CRedisClusterPubSubClientTopic::OnPulseCycle );
    SubscribeTo( pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent ,
                 callback                         );
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::RegisterEventHandlers( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_redisReconnectTimer )
    {
        TEventCallback callback( this, &CRedisClusterPubSubClientTopic::OnRedisReconnectTimerCycle );
        SubscribeTo( m_redisReconnectTimer          ,
                     CORE::CTimer::TimerUpdateEvent ,
                     callback                       );
    }
    
    CORE::PulseGeneratorPtr topicPulseGenerator = m_client->GetConfig().topicPulseGenerator;
    if ( topicPulseGenerator.IsNULL() )
        topicPulseGenerator = m_client->GetConfig().pulseGenerator;
    
    RegisterPulseGeneratorEventHandlers( topicPulseGenerator );
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CRedisClusterPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;

    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRedisClusterPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    //MT::CScopeMutex lock( m_lock );
    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( 0 == publishActionId )
    {
        publishActionId = m_currentPublishActionId;
        ++m_currentPublishActionId;
    }

    const CORE::CVariant& msgId = msg.GetMsgId();
    sw::redis::StringView idSV( msgId.AsCharPtr(), msgId.ByteSize( false ) );

    const CORE::CVariant& primaryPayload = msg.GetPrimaryPayload();
    const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs = msg.GetKeyValuePairs();
    bool hasNoPrimaryPayload = primaryPayload.IsNULLOrEmpty();

    // We use a data member for argument reference storage so we dont have to keep
    // allocating a new array (vector) on every call to Publish()
    m_redisMsgArgs.clear();
    m_redisMsgArgs.reserve( (size_t) kvPairs.size() + ( hasNoPrimaryPayload ? 0 : 1 ) );

    if ( !hasNoPrimaryPayload )
    {
        // This is really not appropriote use of the redis backend
        // its strongly prefered for the application to specify a proper key name to ensure its non-conflicting

        static const CORE::CString fieldName = "PRIMARYPAYLOAD";
        sw::redis::StringView fnSV( fieldName.C_String(), fieldName.ByteSize()-1 );

        sw::redis::StringView fvSV( primaryPayload.AsCharPtr(), primaryPayload.ByteSize( false ) );

        m_redisMsgArgs.push_back( std::pair< sw::redis::StringView, sw::redis::StringView >( fnSV, fvSV ) );
    }

    PUBSUB::CIPubSubMsg::TKeyValuePairs::const_iterator i = kvPairs.begin();
    while ( i != kvPairs.end() )
    {
        const CORE::CVariant& fieldName = (*i).first;
        sw::redis::StringView fnSV( fieldName.AsCharPtr(), fieldName.ByteSize( false ) );

        const CORE::CVariant& fieldValue = (*i).second;
        sw::redis::StringView fvSV( fieldValue.AsCharPtr(), fieldValue.ByteSize( false ) );

        m_redisMsgArgs.push_back( std::pair< sw::redis::StringView, sw::redis::StringView >( fnSV, fvSV ) );

        ++i;
    }

    return RedisSendSyncImpl( publishActionId, idSV, m_redisMsgArgs, notify );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( Disconnect() )
    {
        m_config = config;

        m_redisHashSlot = CalculateRedisHashSlot( m_config.topicName );
        m_readVars.m_readOffset = m_config.redisXReadDefaultOffset;
        m_redisXreadBlockTimeoutInMs = m_config.redisXReadBlockTimeoutInMs;
        if ( GUCEF_NULL != m_client )
        {
            // sanity check the read timeout
            // note that a connection timeout of 0 means infinite for redis++
            if ( m_client->GetConfig().redisConnectionOptionSocketTimeoutInMs > 0 && m_config.redisXReadBlockTimeoutInMs > m_client->GetConfig().redisConnectionOptionSocketTimeoutInMs )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):LoadConfig: redisXReadBlockTimeoutInMs (" + CORE::ToString( m_config.redisXReadBlockTimeoutInMs ) + 
                        ") at the topic level should be configured to a lower value than the client's overall socket timeout (" +
                    CORE::ToString( m_client->GetConfig().redisConnectionOptionSocketTimeoutInMs ) + "). Will clamp the value." );

                // clamp the value
                m_config.redisXReadBlockTimeoutInMs = (UInt32) m_client->GetConfig().redisConnectionOptionSocketTimeoutInMs - 1;
            }
        }

        m_metricFriendlyTopicName = GenerateMetricsFriendlyTopicName( m_config.topicName );
        
        if ( GUCEF_NULL != m_client )
            m_metricsPrefix = m_client->GetConfig().metricsPrefix + m_metricFriendlyTopicName;
        else
            m_metricsPrefix = m_metricFriendlyTopicName;    

        if ( m_config.useTopicLevelMaxTotalMsgsInFlight )
            m_maxTotalMsgsInFlight = (Int32) SMALLEST( m_config.maxTotalMsgsInFlight, GUCEF_INT32MAX );
        else
            m_maxTotalMsgsInFlight = (Int32) SMALLEST( m_client->GetConfig().maxTotalMsgsInFlight, GUCEF_INT32MAX );

        if ( m_config.minAvailableInFlightSlotsBeforeRead > m_maxTotalMsgsInFlight )
            m_config.minAvailableInFlightSlotsBeforeRead = m_maxTotalMsgsInFlight;    
        
        // No point in reading more from redis than we are willing to notify
        // as such we take the lesser of the 2
        if ( m_maxTotalMsgsInFlight > 0 && m_config.redisXReadCount > 0 )
            m_redisMaxXreadCount = SMALLEST( m_config.redisXReadCount, m_maxTotalMsgsInFlight );
        else
        if ( m_maxTotalMsgsInFlight > 0 )
            m_redisMaxXreadCount = m_maxTotalMsgsInFlight;    
        else
            m_redisMaxXreadCount = m_config.redisXReadCount;

        if ( m_config.useDedicatedReadThread )
        {
            // We use blocking "long polling" reads which means we will need a dedicated thread to block until there is data
            // Redis does not support pushing of data directly
            if ( m_config.needSubscribeSupport && m_readerThread.IsNULL() )
            {
                m_readerThread = ( GUCEF_NEW CRedisClusterPubSubClientTopicReader( this ) )->CreateSharedPtr();
            }

            if ( !m_readerThread.IsNULL() )
            {
                if ( !m_readerThread->IsActive() )
                {
                    if ( !m_client->GetThreadPool()->SetupSingularTask( m_readerThread ) )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):LoadConfig: Failed to start blocking reader thread for async subscription" );
                        return false;
                    }
                }
                else
                {
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):LoadConfig: blocking reader thread for async subscription was already active, no need to activate" );
                }
            }
        } 
        else
        {
            // @TODO: Drive based on a timer or similar instead
            return false; // not yet implemented
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CRedisClusterPubSubClientTopicConfig& 
CRedisClusterPubSubClientTopic::GetTopicConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRedisClusterPubSubClientTopic::GetMetricFriendlyTopicName( void ) const
{GUCEF_TRACE;

    return m_metricFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CRedisClusterPubSubClientTopic::GetMetricsPrefix( void ) const
{GUCEF_TRACE;

    return m_metricsPrefix;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::RedisSendSyncImpl( CORE::UInt64& publishActionId      ,
                                                   const sw::redis::StringView& msgId ,
                                                   const TRedisArgs& kvPairs          ,
                                                   bool notify                        )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() || ( m_config.preferDedicatedConnection && GUCEF_NULL == m_redisPipeline ) )
        return false;

    static const sw::redis::StringView autoGenMsgId = sw::redis::StringView( "*", 1 );
    const sw::redis::StringView& msgIdToUse = m_config.redisXAddIgnoreMsgId ? autoGenMsgId : msgId;
    bool totalSuccess = false;

    try
    {
        sw::redis::StringView cnSV( m_config.topicName.C_String(), m_config.topicName.ByteSize() );

        if ( m_config.preferDedicatedConnection && GUCEF_NULL != m_redisPipeline )
        {
            if ( m_config.autoGenerateRedisAddMinId )
            {
                // the cluster uses local time for its auto-generated message id as such we will use the same but are assuming the cluster is in the same timezone
                CORE::UInt64 maxAgeTimestamp = CORE::CDateTime::NowLocalDateTime().ToUnixEpochBasedTicksInMillisecs() - m_config.maxAgeInMsForMinId;
                m_redisPipeline->dvcustom_xadd_minid( cnSV, msgIdToUse, kvPairs.begin(), kvPairs.end(), maxAgeTimestamp, m_config.redisXAddMaxAgeIsApproximate );
            }
            else
            if ( m_config.redisXAddMaxLen >= 0 )
                m_redisPipeline->xadd( cnSV, msgIdToUse, kvPairs.begin(), kvPairs.end(), m_config.redisXAddMaxLen, m_config.redisXAddMaxLenIsApproximate );
            else
                m_redisPipeline->xadd( cnSV, msgIdToUse, kvPairs.begin(), kvPairs.end() );

            sw::redis::QueuedReplies redisReplies = m_redisPipeline->exec();                                                   
            
            size_t replyCount = redisReplies.size();
            for ( size_t r=0; r<replyCount; ++r )
            {
                redisReply& reply = redisReplies.get( r );

                switch ( reply.integer )
                {
                    case REDIS_OK:
                    {
                        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Successfully sent message with " +
                            CORE::ToString( kvPairs.size() ) + " fields. MsgID=" + CORE::ToString( reply.str ) );

                        ++m_redisMsgsTransmitted;
                        m_redisFieldsInMsgsTransmitted += (CORE::UInt32) kvPairs.size();
                        totalSuccess = totalSuccess && true;
                        break;
                    }
                    default:
                    case REDIS_ERR:
                    {
                        ++m_redisErrorReplies;

                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Error sending message with " +
                            CORE::ToString( kvPairs.size() ) + " fields. Error=" + CORE::ToString( reply.str ) );
                        break;
                    }
                }
            }
        }
        else
        {
            if ( m_config.autoGenerateRedisAddMinId )
            {
                // the cluster uses local time for its auto-generated message id as such we will use the same but are assuming the cluster is in the same timezone
                CORE::UInt64 maxAgeTimestamp = CORE::CDateTime::NowLocalDateTime().ToUnixEpochBasedTicksInMillisecs() - m_config.maxAgeInMsForMinId;
                m_redisContext->dvcustom_xadd_minid( cnSV, msgIdToUse, kvPairs.begin(), kvPairs.end(), maxAgeTimestamp, m_config.redisXAddMaxAgeIsApproximate );
            }
            else
            if ( m_config.redisXAddMaxLen >= 0 )
                m_redisContext->xadd( cnSV, msgIdToUse, kvPairs.begin(), kvPairs.end(), m_config.redisXAddMaxLen, m_config.redisXAddMaxLenIsApproximate );
            else
                m_redisContext->xadd( cnSV, msgIdToUse, kvPairs.begin(), kvPairs.end() );
        }
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        ++m_redisTimeoutErrors;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Redis++ Timeout exception: " + e.what() );        
    }
    catch ( const sw::redis::MovedError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Redis++ MovedError (Redirect failed?) . Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );
        Reconnect();
    }
    catch ( const sw::redis::RedirectionError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Redis++ RedirectionError (rebalance? node failure?). Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );

        Reconnect();
    }
    catch ( const sw::redis::ReplyError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Redis++ Reply error exception: " + e.what() );
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Redis++ OOM exception: " + e.what() );
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: Redis++ exception: " + e.what() );
        Reconnect();
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisSendSyncImpl: exception: " + e.what() );
        Reconnect();
    }

    // We will count the ability (or lack thereof) to send messages as proof of health
    UpdateIsHealthyStatus( totalSuccess );

    if ( notify )
    {
        if ( totalSuccess )
            m_publishSuccessActionIds.push_back( publishActionId );
        else
            m_publishFailureActionIds.push_back( publishActionId );
    }

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::PrepStorageForReadMsgs( CORE::UInt32 msgCount )
{GUCEF_TRACE;

    try
    {
        // reset size, note this does not dealloc the underlying memory
        m_readVars.m_pubsubMsgs.clear();
        TPubSubMsgsRefVector& msgRefs = m_readVars.m_pubsubMsgsRefs;
        msgRefs.clear();

        if ( msgCount > m_readVars.m_pubsubMsgs.capacity() )
        {
            // Grow the actual storage, this is allowed to become larger than msgCount to limit
            // the nr of dynamic allocations
            m_readVars.m_pubsubMsgs.reserve( msgCount );
            msgRefs.reserve( msgCount );
        }
    }
    catch ( const std::bad_alloc& )
    {
        UpdateIsHealthyStatus( false );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool 
CRedisClusterPubSubClientTopic::IsSubscriptionAtEndOfData( void ) const
{GUCEF_TRACE;

    return m_subscriptionIsAtEndOfData;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::RedisRead( void )
{GUCEF_TRACE;

    bool totalSuccess = false;
    MT::CScopeMutex lock( m_readVars.m_lock );

    try
    {
        if ( m_redisContext.IsNULL() || m_config.topicName.IsNULLOrEmpty() )
            return totalSuccess;

        CORE::Int32 maxMsgsToRead = m_redisMaxXreadCount;
        if ( maxMsgsToRead > 0 && m_maxTotalMsgsInFlight > 0 )
        {
            if ( m_needToTrackAcks )
            {
                if ( m_msgsInFlight >= m_maxTotalMsgsInFlight )
                {
                    // We already have too many messages 'in flight' 
                    // wait for this number to go down before reading more
                    return true;
                }

                maxMsgsToRead = m_maxTotalMsgsInFlight - (Int32) m_msgsInFlight; 
                if ( m_config.minAvailableInFlightSlotsBeforeRead > maxMsgsToRead )
                {
                    // Based on settings lets not take the overhead of another read call to Redis right now
                    // wait for some more 'in flight' slots to open up
                    return true;
                }
            }    
        }
        if ( m_redisMaxXreadCount > 0 )
            maxMsgsToRead = SMALLEST( m_redisMaxXreadCount, maxMsgsToRead );

        sw::redis::StringView topicSV( m_config.topicName.C_String(), m_config.topicName.ByteSize()-1 );
        sw::redis::StringView readOffsetSV( m_readVars.m_readOffset.c_str(), m_readVars.m_readOffset.size() );
        std::chrono::milliseconds readBlockTimeout( m_redisXreadBlockTimeoutInMs );

        if ( m_config.preferDedicatedConnection && GUCEF_NULL != m_redisPipeline )
        {
            if ( maxMsgsToRead > 0 )
                m_redisPipeline->xread( topicSV, readOffsetSV, readBlockTimeout, (long long) maxMsgsToRead );
            else
                m_redisPipeline->xread( topicSV, readOffsetSV, readBlockTimeout );            

            sw::redis::QueuedReplies redisReplies = m_redisPipeline->exec();

            size_t replyCount = redisReplies.size();
            for ( size_t r=0; r<replyCount; ++r )
            {
                redisReply& reply = redisReplies.get( r );
                int type = reply.type;
                if ( REDIS_REPLY_ARRAY == type )
                {
                    // Process messages for every stream
                    size_t max = reply.elements;
                    for ( size_t i=0; i<max; ++i )
                    {
                        const struct redisReply* e = reply.element[ i ];
                        int eType = e->type;
                        if ( REDIS_REPLY_ARRAY == eType )
                        {
                            size_t streamResultElements = e->elements;

                            // Do we have data from any streams?
                            char* streamName = GUCEF_NULL;
                            if ( streamResultElements > 0 && REDIS_REPLY_STRING == e->element[ 0 ]->type )
                                streamName = e->element[ 0 ]->str;
                            else
                                return false;

                            if ( streamResultElements > 1 && REDIS_REPLY_ARRAY == e->element[ 1 ]->type )
                            {
                                size_t msgCount = e->element[ 1 ]->elements;
                                if ( msgCount > 0 )
                                {
                                    // Since we received messages we cannot be 'at end of data'
                                    m_subscriptionIsAtEndOfData = false;

                                    // Make sure we have enough storage to construct our generic representations
                                    PrepStorageForReadMsgs( (CORE::UInt32) msgCount );
                                    m_msgsReceived += (UInt32) msgCount;
                                
                                    // Cycle through the messages received and build the generic representations
                                    TPubSubMsgsRefVector& msgRefs = m_readVars.m_pubsubMsgsRefs;
                                    for ( size_t m=0; m<msgCount; ++m )
                                    {
                                        const struct redisReply* msg = e->element[ 1 ]->element[ m ];                                    
                                        if ( msg->elements >= 2 )
                                        {
                                            // prep generic storage object for linkage
                                            m_readVars.m_pubsubMsgs.push_back( PUBSUB::CBasicPubSubMsg() );
                                            PUBSUB::CBasicPubSubMsg& pubsubMsg = m_readVars.m_pubsubMsgs.back();
                                            msgRefs.push_back( &pubsubMsg );
                                            pubsubMsg.SetOriginClientTopic( CreateSharedPtr() );

                                            if ( REDIS_REPLY_STRING == msg->element[ 0 ]->type )
                                            {
                                                // set basic message properties
                                                char* msgIdStr = msg->element[ 0 ]->str;
                                                pubsubMsg.GetMsgId().LinkTo( msgIdStr, (UInt32) msg->element[ 0 ]->len, GUCEF_DATATYPE_ASCII_STRING );
                                                CORE::UInt64 unixUtcDt = pubsubMsg.GetMsgId().AsUInt64();
                                                pubsubMsg.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( unixUtcDt );
                                            }
                                            if ( REDIS_REPLY_ARRAY == msg->element[ 1 ]->type )
                                            {
                                                // set message attributes
                                                size_t msgAttribElements = msg->element[ 1 ]->elements;                                    
                                                size_t msgKvPairs = msgAttribElements / 2;

                                                pubsubMsg.GetKeyValuePairs().clear();
                                                pubsubMsg.GetKeyValuePairs().reserve( msgKvPairs );

                                                for ( size_t k=0; k<msgAttribElements; )
                                                {
                                                    char* keyStr = GUCEF_NULL;                                        
                                                    size_t keySize = 0;
                                                    char* valueStr = GUCEF_NULL;
                                                    size_t valueSize = 0;

                                                    const struct redisReply* key = msg->element[ 1 ]->element[ k ];

                                                    if ( REDIS_REPLY_STRING == key->type )
                                                    {
                                                        keyStr = key->str;
                                                        keySize = key->len;
                                                        ++k;
                                                    }
                                                    else
                                                        return false;

                                                    const struct redisReply* value = msg->element[ 1 ]->element[ k ];

                                                    if ( REDIS_REPLY_STRING == value->type )
                                                    {
                                                        valueStr = value->str;
                                                        valueSize = value->len;
                                                        ++k;
                                                    }
                                                    else
                                                        return false;

                                                    pubsubMsg.GetKeyValuePairs().push_back( PUBSUB::CBasicPubSubMsg::TKeyValuePair() );
                                                    PUBSUB::CBasicPubSubMsg::TKeyValuePair& kvLink = pubsubMsg.GetKeyValuePairs().back();

                                                    kvLink.first.LinkTo( keyStr, (UInt32) keySize, GUCEF_DATATYPE_BINARY_BLOB );
                                                    kvLink.second.LinkTo( valueStr, (UInt32) valueSize, GUCEF_DATATYPE_BINARY_BLOB );

                                                    m_msgsBytesReceived += (UInt32) ( keySize + valueSize );
                                                    
                                                    ++m_fieldsInMsgsReceived;
                                                }
                                            }

                                            pubsubMsg.SetReceiveActionId( m_readVars.m_currentReceiveActionId );

                                            if ( !m_journal.IsNULL() )
                                                m_journal->AddMessageReceivedJournalEntry( m_readVars.m_currentReceiveActionId );

                                            ++m_readVars.m_currentReceiveActionId;
                                        }
                                    }

                                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: read " + 
                                        CORE::ToString( msgCount ) + " messages from topic " + m_config.topicName );
        
                                    // Communicate all the messages received via an event notification
                                    if ( NotifyObservers( MsgsRecievedEvent, &m_readVars.m_pubsubMsgsRefs ) ) 
                                    {
                                        m_msgsInFlight += (CORE::Int64) m_readVars.m_pubsubMsgsRefs.size();

                                        PUBSUB::CBasicPubSubMsg& lastMsg = m_readVars.m_pubsubMsgs.back();
                                        m_readVars.m_readOffset = lastMsg.GetMsgId().AsString();


                                        m_readVars.m_redisReadMsgs.clear();
                                        m_readVars.m_pubsubMsgsRefs.clear();
                                        m_readVars.m_pubsubMsgAttribs.clear();
                                    }
                                    else
                                    {
                                        return totalSuccess;
                                    }
                                }
                                else
                                {
                                    // we have a valid reply but 0 messages
                                    // we are at the end of the stream
                                }
                            }
                            else
                                return false;
                        }
                    }
                }
                else
                {
                    if ( REDIS_REPLY_ERROR == type )
                    {
                        ++m_redisErrorReplies;
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: Error using pipeline to receive messages. Error=" + CORE::ToString( reply.str ) );
                        return false;
                    }
                }
            }
        }
        else
        {
            TRedisMsgByStreamInserter inserter = std::inserter( m_readVars.m_redisReadMsgs, m_readVars.m_redisReadMsgs.end() );

            if ( maxMsgsToRead > 0 )
                m_redisContext->xread< TRedisMsgByStreamInserter >( topicSV, readOffsetSV, readBlockTimeout, (long long) maxMsgsToRead, inserter );
            else
                m_redisContext->xread< TRedisMsgByStreamInserter >( topicSV, readOffsetSV, readBlockTimeout, inserter );

            // Do we have data from any streams?
            if ( !m_readVars.m_redisReadMsgs.empty() )
            {
                // Since we received messages we cannot be 'at end of data'
                m_subscriptionIsAtEndOfData = false;
            
                // Process messages for every stream
                TRedisMsgByStream::iterator s = m_readVars.m_redisReadMsgs.begin();
                while ( s != m_readVars.m_redisReadMsgs.end() )
                {
                    // Get the actual msgs
                    TRedisMsgVector& msgs = (*(m_readVars.m_redisReadMsgs.begin())).second;
                    m_msgsReceived += (UInt32) msgs.size();

                    // Make sure we have enough storage to construct our generic representations
                    PrepStorageForReadMsgs( (CORE::UInt32) msgs.size() );

                    // Cycle through the messages received and build the generic representations
                    TPubSubMsgsRefVector& msgRefs = m_readVars.m_pubsubMsgsRefs;
                    TRedisMsgVector::iterator i = msgs.begin();
                    while ( i != msgs.end() )
                    {
                        std::string& msgId = (*i).first;                        

                        m_readVars.m_pubsubMsgs.push_back( PUBSUB::CBasicPubSubMsg() );
                        PUBSUB::CBasicPubSubMsg& pubsubMsg = m_readVars.m_pubsubMsgs.back();
                        msgRefs.push_back( &pubsubMsg );
                        pubsubMsg.SetOriginClientTopic( CreateSharedPtr() );

                        pubsubMsg.SetReceiveActionId( m_readVars.m_currentReceiveActionId );

                        if ( !m_journal.IsNULL() )
                            m_journal->AddMessageReceivedJournalEntry( m_readVars.m_currentReceiveActionId );

                        ++m_readVars.m_currentReceiveActionId;

                        // set basic message properties

                        pubsubMsg.GetMsgId().LinkTo( msgId );
                        CORE::UInt64 unixUtcDt = CORE::StringToUInt64( msgId );
                        pubsubMsg.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( unixUtcDt );

                        // set the message attributes
                        TRedisMsgAttributes& msgAttribs = (*i).second;
                        if ( msgAttribs.size() > m_readVars.m_pubsubMsgAttribs.size() )
                            m_readVars.m_pubsubMsgAttribs.resize( msgAttribs.size() );
                        pubsubMsg.GetKeyValuePairs().clear();
                        pubsubMsg.GetKeyValuePairs().reserve( msgAttribs.size() );

                        TBufferVector::iterator a = m_readVars.m_pubsubMsgAttribs.begin();
                        TRedisMsgAttributes::iterator n = msgAttribs.begin();
                        while ( n != msgAttribs.end() )
                        {
                            std::string& keyAtt = (*n).first;
                            std::string& valueAtt = (*n).second;

                            CORE::CDynamicBuffer& keyAttBuffer = (*a).first;
                            CORE::CDynamicBuffer& valueAttBuffer = (*a).second;

                            keyAttBuffer.LinkTo( keyAtt );
                            valueAttBuffer.LinkTo( valueAtt );

                            m_msgsBytesReceived += ( keyAttBuffer.GetDataSize() + valueAttBuffer.GetDataSize() );
                            
                            pubsubMsg.GetKeyValuePairs().push_back( PUBSUB::CBasicPubSubMsg::TKeyValuePair() );
                            PUBSUB::CBasicPubSubMsg::TKeyValuePair& kvLink = pubsubMsg.GetKeyValuePairs().back();

                            kvLink.first.LinkTo( keyAttBuffer );
                            kvLink.second.LinkTo( valueAttBuffer );

                            ++m_fieldsInMsgsReceived;
                            ++n; ++a;
                        }
                        ++i;
                    }

                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: read " + 
                        CORE::ToString( m_readVars.m_pubsubMsgsRefs.size() ) + " messages" );
        
                    // Communicate all the messages received via an event notification
                    if ( NotifyObservers( MsgsRecievedEvent, &m_readVars.m_pubsubMsgsRefs ) ) 
                    {
                        m_msgsInFlight += (CORE::Int64) m_readVars.m_pubsubMsgsRefs.size();

                        m_readVars.m_readOffset = (*msgs.rbegin()).first;


                        m_readVars.m_redisReadMsgs.clear();
                        m_readVars.m_pubsubMsgsRefs.clear();
                        m_readVars.m_pubsubMsgAttribs.clear();
                    }
                    else
                    {
                        return totalSuccess;
                    }

                    ++s;
                }
            }
            else
            {
                // xread timeout occured since we returned without any data
                if ( m_config.treatXReadBlockTimeoutAsEndOfDataEvent )
                {   
                    if ( !m_subscriptionIsAtEndOfData )
                    {
                        m_subscriptionIsAtEndOfData = true;

                        if ( GUCEF_NULL != m_client )
                        {
                            if ( m_client->GetConfig().desiredFeatures.supportsSubscriptionEndOfDataEvent )
                            {                         
                                 if ( !NotifyObservers( SubscriptionEndOfDataEvent ) )
                                    return totalSuccess;
                            }
                        }
                    }
                }
            }
        }

        // We will count the ability to read messages as proof of good health
        UpdateIsHealthyStatus( true );
        
        totalSuccess = true;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        ++m_redisTimeoutErrors;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: Redis++ Timeout exception: " + e.what() );        
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: Redis++ OOM exception: " + e.what() );
    }
    catch ( const sw::redis::MovedError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: Redis++ MovedError (Redirect failed?) . Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );
        Reconnect();
    }
    catch ( const sw::redis::RedirectionError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: Redis++ RedirectionError (rebalance? node failure?). Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );

        Reconnect();
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: Redis++ exception: " + e.what() );
        Reconnect();
        UpdateIsHealthyStatus( false );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisRead: exception: " + e.what() );
        Reconnect();
        UpdateIsHealthyStatus( false );
    }

    m_readVars.m_redisReadMsgs.clear();
    m_readVars.m_pubsubMsgsRefs.clear();
    m_readVars.m_pubsubMsgAttribs.clear();

    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::CalculateRedisHashSlot( const CORE::CString& keyStr ) const
{GUCEF_TRACE;

    // The following code is copied from: https://redis.io/topics/cluster-spec

    // Minor changes were made to accept a string object and use platform types
    CORE::UInt32 keylen = keyStr.Length();
    const char* key = keyStr.C_String();

    CORE::UInt32 s, e; /* start-end indexes of { and } */

    /* Search the first occurrence of '{'. */
    for (s = 0; s < keylen; s++)
        if (key[s] == '{') break;

    /* No '{' ? Hash the whole key. This is the base case. */
    if ( s == keylen)
        return sw::redis::crc16( key,(int) keylen) & 16383;

    /* '{' found? Check if we have the corresponding '}'. */
    for (e = s+1; e < keylen; e++)
        if (key[e] == '}') break;

    /* No '}' or nothing between {} ? Hash the whole key. */
    if (e == keylen || e == s+1)
        return sw::redis::crc16(key,keylen) & 16383;

    /* If we are here there is both a { and a } on its right. Hash
     * what is in the middle between { and }. */
    return sw::redis::crc16(key+s+1,e-s-1) & 16383;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    try
    {
        if ( !m_redisContext.IsNULL() )
        {
            RedisClusterPubSubClientTopicReaderPtr redisReader = m_readerThread;
            if ( !redisReader.IsNULL() )
            {
                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Beginning cleanup" );
            
                lock.EarlyUnlock();

                if ( !redisReader->RequestTaskToStop( true ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Failed to stop reader thread" );
                    return false;
                }

                lock.ReLock();
                m_isSubscribed = false;
            }

            GUCEF_DELETE m_redisPipeline;
            m_redisPipeline = GUCEF_NULL;

            // the parent client owns the context, we just null it
            m_redisContext.Unlink();

            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Finished cleanup" );
        }
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        ++m_redisTimeoutErrors;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Redis++ Timeout exception: " + e.what() );        
        return false;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Redis++ exception: " + e.what() );        
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: exception: " + e.what() );        
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::Reconnect( void )
{GUCEF_TRACE;

    Disconnect();
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisReconnect: starting reconnect timer" );
    m_redisReconnectTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::RequestSubscriptionMsgArrivalDelay( CORE::UInt32 minDelayInMs )
{GUCEF_TRACE;

    // We create a copy of the shared pointer to garantuee the lifecycle
    RedisClusterPubSubClientTopicReaderPtr readerThread = m_readerThread;
    if ( !readerThread.IsNULL() )
    {
        readerThread->RequestTaskCycleDelayInMs( minDelayInMs );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::CleanupRedisReaderThread( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    try
    {
        // We use blocking "long polling" reads which means we needed a dedicated thread to block until there is data
        // since Redis does not support pushing of data directly
        // We will now clean up that thread

        if ( !m_readerThread.IsNULL() )
        {
            if ( !m_readerThread->RequestTaskToStop( true ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):CleanupRedisReaderThread: Failed to stop dedicated redis reader thread" );
            }
            m_readerThread.Unlink();
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):CleanupRedisReaderThread: exception: " + e.what() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::SubscribeImpl( const std::string& readOffset )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    try
    {        
        if ( m_redisContext.IsNULL() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: No redis context is available" );
            return false;
        }

        sw::redis::StringView topicNameSV( m_config.topicName.C_String(), m_config.topicName.Length() );
        UInt64 streamLength = m_redisContext->xlen( topicNameSV );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: current stream length for steam with name \"" + 
            m_config.topicName + "\" is " + CORE::ToString( streamLength ) );

        if ( 0 == streamLength )
        {
            m_subscriptionIsAtEndOfData = true;

            if ( GUCEF_NULL != m_client )
            {
                if ( m_client->GetConfig().desiredFeatures.supportsSubscriptionEndOfDataEvent )
                {                         
                    if ( !NotifyObservers( SubscriptionEndOfDataEvent ) )
                        return true;
                }
            }
        }
        else
        {
            // reset the oed flag
            m_subscriptionIsAtEndOfData = false;
        }

        m_readVars.m_readOffset = readOffset;
        
        // We use blocking "long polling" reads which means we will need a dedicated thread to block until there is data
        // Redis does not support pushing of data directly

        if ( !m_readerThread.IsNULL() )
        {
            if ( !m_client->GetThreadPool()->StartTask( m_readerThread ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: Failed to start blocking reader thread for async subscription" );
                return false;
            }
        }
        
        m_isSubscribed = true;
        return true;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        ++m_redisTimeoutErrors;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: Redis++ Timeout exception: " + e.what() );        
        return false;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: Redis++ exception: " + e.what() );
        Reconnect();
        return false;
    }
    catch ( const GUCEF::timeout_exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: timeout exception" );
        Reconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):SubscribeImpl: exception: " + e.what() );
        Reconnect();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    // Since Redis does not store the offset itself we will use whatever was last in-process which if this
    // is the first use means its the default starting offset
    MT::CScopeMutex lock( m_readVars.m_lock );
    return SubscribeImpl( m_readVars.m_readOffset );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_readVars.m_lock );

    switch ( bookmark.GetBookmarkType() )
    {
        // With Redis the default ID format is a Unix Epoch based datetime
        // For streaming Redis the msg ID and offset/index are the same thing
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_ID:
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX:
        case PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX:
        {
            return SubscribeImpl( bookmark.GetBookmarkData().AsString() );
        }
        default:
        {
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark
CRedisClusterPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_readVars.m_lock );
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX, CORE::CVariant( m_readVars.m_readOffset ) );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg    , 
                                                       PUBSUB::CPubSubBookmark& bookmark ) const
{GUCEF_TRACE;

    // The Redis message ID is a unique entry id per topic and as such a topic index
    // we set it on the generic message id property when reading messages
    bookmark.SetBookmarkType( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX );
    bookmark.SetBookmarkData( msg.GetMsgId() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( m_msgsInFlight > 0 )
        --m_msgsInFlight;

    // Does not apply to Redis wrt what this plugin supports. just treat as a no-op fyi
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    if ( m_msgsInFlight > 0 )
        --m_msgsInFlight;

    // Does not apply to Redis wrt what this plugin supports. just treat as a no-op fyi
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_redisContext.IsNULL() && ( !m_config.preferDedicatedConnection || GUCEF_NULL != m_redisPipeline );
}

/*-------------------------------------------------------------------------*/

bool 
CRedisClusterPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_redisContext.IsNULL() && m_isSubscribed && ( !m_config.preferDedicatedConnection || ( GUCEF_NULL != m_redisPipeline ) );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    
    bool newIsHealthyState = true;
    
    // If we are having to reconnect than we are not healthy
    if ( GUCEF_NULL != m_redisReconnectTimer && m_redisReconnectTimer->GetEnabled() )
        newIsHealthyState = false;

    if ( newIsHealthyState != m_isHealthy )
    {
        m_isHealthy = newIsHealthyState;

        if ( m_isHealthy )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic:IsHealthy: Topic " + m_config.topicName + " is now healthy" );
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic:IsHealthy: Topic " + m_config.topicName + " is now unhealthy" );         
        }

        lock.EarlyUnlock();
        THealthStatusChangeEventData eData( newIsHealthyState ); 
        NotifyObservers( HealthStatusChangeEvent, &eData ); 
    }
    
    return newIsHealthyState;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::UpdateIsHealthyStatus( bool newStatus )
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

bool
CRedisClusterPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    if ( reset )
    {
        if ( !Disconnect() )
            return false;
    }

    MT::CScopeMutex lock( m_lock );
    try
    {
        RedisClusterPtr lastRedisContext = m_redisContext;
        m_redisContext = m_client->GetRedisContext();
        if ( m_redisContext.IsNULL() )
            return false;

        if ( lastRedisContext != m_redisContext )
        {
            // The following is not a must-have for connectivity
            const RedisNodeMap& nodeMap = m_client->GetRedisNodeMap();
            RedisNodeMap::const_iterator i = nodeMap.begin();
            while ( i != nodeMap.end() )
            {
                if ( (*i).first > m_redisHashSlot )
                    break;
                if ( m_redisHashSlot >= (*i).first && m_redisHashSlot <= (*i).second.endSlot )
                {
                    m_redisShardHost = (*i).second.host;
                    m_redisShardNodeId = (*i).second.nodeId;
                    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):InitializeConnectivity: Stream \"" + m_config.topicName +
                        "\" hashes to hash slot " + CORE::ToString( m_redisHashSlot ) + " which lives at " + (*i).second.host.HostnameAndPortAsString() + " with node id " + (*i).second.nodeId );
                    break;
                }
                ++i;
            }
        }

        if ( m_config.preferDedicatedConnection )
        {
            if ( reset || GUCEF_NULL == m_redisPipeline )
            {
                // Connect to the specific shard used for this channel's stream with a single dedicated connection
                sw::redis::StringView cnSV( m_config.topicName.C_String(), m_config.topicName.Length() );
                GUCEF_DELETE m_redisPipeline;
                m_redisPipeline = GUCEF_NEW sw::redis::Pipeline( m_redisContext->pipeline( cnSV ) );

                GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):InitializeConnectivity: Successfully created a Redis pipeline. Hash Slot " + CORE::ToString( m_redisHashSlot ) );
            }
        }

        return true;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        ++m_redisTimeoutErrors;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):InitializeConnectivity: Redis++ Timeout exception: " + e.what() );        
        return false;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):InitializeConnectivity: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):InitializeConnectivity: Redis++ exception: " + e.what() );
        m_redisReconnectTimer->SetEnabled( true );
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):InitializeConnectivity: exception: " + e.what() );
        m_redisReconnectTimer->SetEnabled( true );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::GetRedisClusterNodeMap( RedisNodeMap& nodeMap )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() )
        return false;

    try
    {
        CORE::CString clusterCmd( "CLUSTER" );
        CORE::CString nodesParam( "NODES" );

        sw::redis::StringView clusterCmdSV( clusterCmd.C_String(), clusterCmd.Length() );
        sw::redis::StringView nodesParamSV( nodesParam.C_String(), nodesParam.Length() );

        sw::redis::ReplyUPtr reply = m_redisContext->command( clusterCmdSV, nodesParamSV );
        if ( reply )
        {
            if ( REDIS_REPLY_STRING == reply->type )
            {
                CORE::CString allText = reply->str;
                CORE::CString::StringVector lines = allText.ParseElements( '\n', false );

                // First add all the primary nodes
                CORE::CString::StringVector::iterator i = lines.begin();
                while ( i != lines.end() )
                {
                    CORE::CString::StringVector properties = (*i).ParseElements( ' ', true );
                    if ( properties.size() >= 8 )
                    {
                        bool isReplica = properties[ 2 ].HasSubstr( "slave" ) > -1;
                        if ( !isReplica )
                        {
                            CORE::CString& nodeId = properties[ 0 ];
                            CORE::CString ipAndPort = properties[ 1 ].SubstrToChar( '@' );
                            //CORE::CString& parentNodeId = properties[ 3 ];
                            //bool isConnected = properties[ 7 ] == "connected";

                            CORE::Int32 startSlot = -1;
                            CORE::Int32 endSlot = -1;
                            if ( properties.size() >= 9 )
                            {
                                startSlot = CORE::StringToInt32( properties[ 8 ].SubstrToChar( '-', true ), -1 );
                                endSlot = CORE::StringToInt32( properties[ 8 ].SubstrToChar( '-', false ), -1 );
                            }

                            RedisNode& node = nodeMap[ startSlot ];
                            node.nodeId = nodeId;
                            node.host.SetHostnameAndPort( ipAndPort );
                            node.startSlot = startSlot;
                            node.endSlot = endSlot;
                        }
                    }
                    ++i;
                }

            //    // Now add the replicas
            //    i = lines.begin();
            //    while ( i != lines.end() )
            //    {
            //        CORE::CString::StringVector properties = (*i).ParseElements( ' ', false );
            //        if ( properties.size() >= 8 )
            //        {
            //            bool isReplica = properties[ 2 ].HasSubstr( "slave" ) > -1;
            //            if ( isReplica )
            //            {
            //                CORE::CString& nodeId = properties[ 0 ];
            //                CORE::CString ipAndPort = properties[ 1 ].SubstrToChar( '@' );
            //                CORE::CString& parentNodeId = properties[ 3 ];
            //                //bool isConnected = properties[ 7 ] == "connected";

            //                RedisNodeMap::iterator n = nodeMap.begin();
            //                while ( n != nodeMap.end() )
            //                {
            //                    RedisNode* replicaNode = (*n).second.FindReplica( nodeId, parentNodeId, true );
            //                    if ( GUCEF_NULL != replicaNode )
            //                    {
            //                        replicaNode->host.SetHostnameAndPort( ipAndPort );
            //                        break;
            //                    }
            //                    ++n;
            //                }
            //            }
            //        }
            //        ++i;
            //    }
            }
        }
        return true;
    }
    catch ( const sw::redis::TimeoutError& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):GetRedisClusterNodeMap: Redis++ Timeout exception: " + e.what() );
        ++m_redisTimeoutErrors;
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):GetRedisClusterNodeMap: Redis++ exception: " + e.what() );
        ++m_redisErrorReplies;
        Reconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):GetRedisClusterNodeMap: exception: " + e.what() );
        Reconnect();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetMsgsInFlightGauge( void ) const
{GUCEF_TRACE;

    if ( m_msgsInFlight > 0 )
        return (CORE::UInt32) m_msgsInFlight;
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetRedisErrorRepliesCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisErrorReplies = m_redisErrorReplies;
        m_redisErrorReplies = 0;
        return redisErrorReplies;
    }
    else
        return m_redisErrorReplies;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetRedisTimeoutsCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisTimeoutErrors = m_redisTimeoutErrors;
        m_redisTimeoutErrors = 0;
        return redisTimeoutErrors;
    }
    else
        return m_redisTimeoutErrors;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetMsgsTransmittedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisMsgsTransmitted = m_redisMsgsTransmitted;
        m_redisMsgsTransmitted = 0;
        return redisMsgsTransmitted;
    }
    else
        return m_redisMsgsTransmitted;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetFieldsInMsgsTransmittedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisFieldsInMsgsTransmitted = m_redisFieldsInMsgsTransmitted;
        m_redisFieldsInMsgsTransmitted = 0;
        return redisFieldsInMsgsTransmitted;
    }
    else
        return m_redisFieldsInMsgsTransmitted;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetMsgsReceivedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgsReceived = m_msgsReceived;
        m_msgsReceived = 0;
        return msgsReceived;
    }
    else
        return m_msgsReceived;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetMsgsBytesReceivedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 msgsBytesReceived = m_msgsBytesReceived;
        m_msgsBytesReceived = 0;
        return msgsBytesReceived;
    }
    else
        return m_msgsBytesReceived;
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
CRedisClusterPubSubClientTopic::GetFieldsInMsgsReceivedCounter( bool resetCounter )
{GUCEF_TRACE;

    if ( resetCounter )
    {
        CORE::UInt32 redisFieldsInMsgsTransmitted = m_redisFieldsInMsgsTransmitted;
        m_redisFieldsInMsgsTransmitted = 0;
        return redisFieldsInMsgsTransmitted;
    }
    else
        return m_redisFieldsInMsgsTransmitted;
}

/*-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : redisErrorReplies( 0 )
    , redisTimeouts( 0 )
    , msgsReceived( 0 )
    , fieldsInMsgsReceivedRatio( 0.0f )
    , msgsBytesReceived( 0 )
    , msgsTransmitted( 0 )
    , fieldsInMsgsTransmittedRatio( 0.0f )
    , msgsInFlight( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRedisClusterPubSubClientTopic::GenerateMetricsFriendlyTopicName( const CORE::CString& topicName )
{GUCEF_TRACE;

    // Let's avoid non-ASCII stumbling blocks and force the down to ASCII
    CORE::CAsciiString asciiMetricsFriendlyTopicName = topicName.ForceToAscii( '_' );

    // Replace special chars
    static const char disallowedChars[] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '=', '+', ',', '.', '<', '>', '/', '?', '`', '~', '\\', '|', '{', '}', '[', ']', ';', ':', '\'', '\"' };
    asciiMetricsFriendlyTopicName = asciiMetricsFriendlyTopicName.ReplaceChars( disallowedChars, sizeof(disallowedChars)/sizeof(char), '_' );

    // Back to the platform wide string convention format
    CORE::CString metricsFriendlyTopicName = CORE::ToString( asciiMetricsFriendlyTopicName );
    return metricsFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

const CRedisClusterPubSubClientTopic::TopicMetrics&
CRedisClusterPubSubClientTopic::GetMetrics( void ) const
{GUCEF_TRACE;

    return m_metrics;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                                                     const CORE::CEvent& eventId  ,
                                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_metrics.msgsInFlight = GetMsgsInFlightGauge();

    m_metrics.redisErrorReplies = GetRedisErrorRepliesCounter( true );
    m_metrics.redisTimeouts = GetRedisTimeoutsCounter( true );

    m_metrics.msgsTransmitted = GetMsgsTransmittedCounter( true );    
    UInt32 fieldsInMsgsTransmitted = GetFieldsInMsgsTransmittedCounter( true );
    if ( m_metrics.msgsTransmitted > 0 )
        m_metrics.fieldsInMsgsTransmittedRatio = (Float32) ( fieldsInMsgsTransmitted / m_metrics.msgsTransmitted );
    else
        m_metrics.fieldsInMsgsTransmittedRatio = 0.0f;    

    m_metrics.msgsReceived = GetMsgsReceivedCounter( true );    
    m_metrics.msgsBytesReceived = GetMsgsBytesReceivedCounter( true );    
    UInt32 fieldsInMsgsReceived = GetFieldsInMsgsReceivedCounter( true );
    if ( m_metrics.msgsReceived > 0 )
        m_metrics.fieldsInMsgsReceivedRatio = (Float32) ( fieldsInMsgsReceived / m_metrics.msgsReceived );
    else
        m_metrics.fieldsInMsgsReceivedRatio = 0.0f;

}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                                            const CORE::CEvent& eventId  ,
                                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( InitializeConnectivity( true ) )
    {
        m_redisReconnectTimer->SetEnabled( false );
    }
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::OnPulseCycle( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_publishSuccessActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishedEvent, &m_publishSuccessActionEventData ) ) return;
        m_publishSuccessActionIds.clear();
    }
    if ( !m_publishFailureActionIds.empty() )
    {
        if ( !NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData ) ) return;
        m_publishFailureActionIds.clear();
    }
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CRedisClusterPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

 MT::TLockStatus
 CRedisClusterPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRedisClusterPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& 
CRedisClusterPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::REDISCLUSTER::CRedisClusterPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void 
CRedisClusterPubSubClientTopic::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_redisReconnectTimer )
    {
        m_redisReconnectTimer->SetPulseGenerator( newPulseGenerator );
    }
    
    PUBSUB::CPubSubClientTopic::SetPulseGenerator( newPulseGenerator );

    RegisterPulseGeneratorEventHandlers( newPulseGenerator );
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr 
CRedisClusterPubSubClientTopic::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    if ( ( m_config.needSubscribeSupport && !m_config.needPublishSupport ) &&
         !m_readerThread.IsNULL()                                           )
    {
        // If we are only reading from Redis and not publishing anything we can confidently state
        // that the reader thread is where all the action will be. As such to reduce thread contention
        // for the given topic we'd want to use that thread's pulse generator in other topic related management
        // outside the backend as well if possible
        return m_readerThread->GetPulseGenerator();
    }

    if ( GUCEF_NULL != m_client )
        return m_client->GetPulseGenerator();

    return CORE::PulseGeneratorPtr();
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::SetJournal( PUBSUB::CIPubSubJournalBasicPtr journal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_journal = journal;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CIPubSubJournalBasicPtr 
CRedisClusterPubSubClientTopic::GetJournal( void ) const
{GUCEF_TRACE;

    return m_journal;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISCLUSTER */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
