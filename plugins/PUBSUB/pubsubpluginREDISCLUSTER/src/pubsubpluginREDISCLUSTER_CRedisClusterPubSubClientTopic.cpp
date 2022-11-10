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
    , m_redisTransmitQueueSize( 0 )
    , m_redisMsgsTransmitted( 0 )
    , m_redisFieldsInMsgsTransmitted( 0 )
    , m_redisFieldsInMsgsRatio( 0 )
    , m_redisHashSlot( GUCEFMT_UINT32MAX )
    , m_redisXreadCount( 1000 )
    , m_redisXreadBlockTimeoutInMs( 1000 )
    , m_redisReadMsgs()
    , m_redisMsgArgs()
    , m_redisShardHost()
    , m_redisShardNodeId()
    , m_redisReconnectTimer( GUCEF_NULL )
    , m_config()
    , m_readOffset( "0" )
    , m_readerThread()
    , m_needToTrackAcks( true )
    , m_subscriptionIsAtEndOfData( false )
    , m_maxTotalMsgsInFlight( 1000 )
    , m_msgsInFlight( 0 )
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
    , m_metrics()
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

void
CRedisClusterPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    m_client = GUCEF_NULL;

    delete m_redisReconnectTimer;
    m_redisReconnectTimer = GUCEF_NULL;
    
    CleanupRedisReaderThread();
    Disconnect();    
    SignalUpcomingDestruction();
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

    TEventCallback callback( this, &CRedisClusterPubSubClientTopic::OnPulseCycle );
    SubscribeTo( m_client->GetConfig().pulseGenerator.GetPointerAlways() ,
                 CORE::CPulseGenerator::PulseEvent                       ,
                 callback                                                );
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CRedisClusterPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::IsPublishingSupported( void )
{GUCEF_TRACE;

    return m_config.needPublishSupport;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::IsSubscribingSupported( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRedisClusterPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
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

        static const CORE::CString& fieldName = "PRIMARYPAYLOAD";
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
        m_readOffset = m_config.redisXReadDefaultOffset;
        m_redisXreadBlockTimeoutInMs = m_config.redisXReadBlockTimeoutInMs;

        m_metricFriendlyTopicName = GenerateMetricsFriendlyTopicName( m_config.topicName );

        if ( m_config.useTopicLevelMaxTotalMsgsInFlight )
            m_maxTotalMsgsInFlight = m_config.maxTotalMsgsInFlight;
        else
            m_maxTotalMsgsInFlight = m_client->GetConfig().maxTotalMsgsInFlight;
        
        if ( !m_needToTrackAcks )
        {
            // We are in fire and forget mode so max in flight means per notification of received messages
            // for that there is no point in reading more from redis than we are willing to notify
            // as such we take the lesser of the 2
            if ( m_maxTotalMsgsInFlight > 0 )
                m_redisXreadCount = SMALLEST( m_config.redisXReadCount, m_maxTotalMsgsInFlight );
            else
                m_redisXreadCount = m_config.redisXReadCount;
        }
        else
        {
            // We are tracking acks
            // as such we will be waiting on acks as an independent check
            m_redisXreadCount = m_config.redisXReadCount;
        }
        
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRedisClusterPubSubClientTopic::GetMetricFriendlyTopicName( void ) const
{GUCEF_TRACE;

    return m_metricFriendlyTopicName;
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::RedisSendSyncImpl( CORE::UInt64& publishActionId      ,
                                                   const sw::redis::StringView& msgId ,
                                                   const TRedisArgs& kvPairs          ,
                                                   bool notify                        )
{GUCEF_TRACE;

    if ( m_redisContext.IsNULL() || GUCEF_NULL == m_redisPipeline )
        return false;

    static const sw::redis::StringView autoGenMsgId = sw::redis::StringView( "*", 1 );
    const sw::redis::StringView& msgIdToUse = m_config.redisXAddIgnoreMsgId ? autoGenMsgId : msgId;
    bool totalSuccess = true;

    try
    {
        sw::redis::StringView cnSV( m_config.topicName.C_String(), m_config.topicName.ByteSize() );

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
                    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Successfully sent message with " +
                        CORE::ToString( kvPairs.size() ) + " fields. MsgID=" + CORE::ToString( reply.str ) );

                    ++m_redisMsgsTransmitted;
                    m_redisFieldsInMsgsTransmitted += (CORE::UInt32) kvPairs.size();
                    m_redisFieldsInMsgsRatio = (CORE::UInt32) kvPairs.size();       // <- @TODO doesnt look like this was ported correctly
                    break;
                }
                default:
                case REDIS_ERR:
                {
                    totalSuccess = false;
                    ++m_redisErrorReplies;

                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Error sending message with " +
                        CORE::ToString( kvPairs.size() ) + " fields. Error=" + CORE::ToString( reply.str ) );
                    break;
                }
            }
        }

        // We will count the ability (or lack thereof) to send messages as proof of health
        UpdateIsHealthyStatus( totalSuccess );
    }
    catch ( const sw::redis::MovedError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ MovedError (Redirect failed?) . Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );
        Reconnect();
    }
    catch ( const sw::redis::RedirectionError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ RedirectionError (rebalance? node failure?). Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );

        Reconnect();
    }
    catch ( const sw::redis::ReplyError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ Reply error exception: " + e.what() );
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ OOM exception: " + e.what() );
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ exception: " + e.what() );
        Reconnect();
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: exception: " + e.what() );
        Reconnect();
    }

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
        m_pubsubMsgs.clear();
        TPubSubMsgsRefVector& msgRefs = m_pubsubMsgsRefs;
        msgRefs.clear();

        if ( msgCount > m_pubsubMsgs.capacity() )
        {
            // Grow the actual storage, this is allowed to become larger than msgCount to limit
            // the nr of dynamic allocations
            m_pubsubMsgs.reserve( msgCount );
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
    MT::CScopeMutex lock( m_lock );

    try
    {
        if ( m_redisContext.IsNULL() || m_config.topicName.IsNULLOrEmpty() )
            return totalSuccess;

        CORE::UInt64 maxMsgsToRead = m_redisXreadCount;
        if ( m_needToTrackAcks || m_maxTotalMsgsInFlight > 0 )
        {
            if ( m_msgsInFlight >= m_maxTotalMsgsInFlight )
            {
                // We already have too many messages 'in flight' 
                // wait for this number to go down before reading more
                return true;
            }

            maxMsgsToRead = m_maxTotalMsgsInFlight - m_msgsInFlight; 
            if ( m_config.minAvailableInFlightSlotsBeforeRead > maxMsgsToRead )
            {
                // Based on settings lets not take the overhead of another read call to Redis right now
                // wait for some more 'in flight' slots to open up
                return true;
            }
        }    

        sw::redis::StringView topicSV( m_config.topicName.C_String(), m_config.topicName.ByteSize()-1 );
        sw::redis::StringView readOffsetSV( m_readOffset.c_str(), m_readOffset.size() );
        std::chrono::milliseconds readBlockTimeout( m_redisXreadBlockTimeoutInMs );

        TRedisMsgByStreamInserter inserter = std::inserter( m_redisReadMsgs, m_redisReadMsgs.end() );

        if ( maxMsgsToRead > 0 )
            m_redisContext->xread< TRedisMsgByStreamInserter >( topicSV, readOffsetSV, readBlockTimeout, (long long) maxMsgsToRead, inserter );
        else
            m_redisContext->xread< TRedisMsgByStreamInserter >( topicSV, readOffsetSV, readBlockTimeout, inserter );

        // Do we have data from any streams?
        if ( !m_redisReadMsgs.empty() )
        {
            // Since we received messages we cannot be 'at end of data'
            m_subscriptionIsAtEndOfData = false;
            
            // Process messages for every stream
            TRedisMsgByStream::iterator s = m_redisReadMsgs.begin();
            while ( s != m_redisReadMsgs.end() )
            {
                // Get the actual msgs
                TRedisMsgVector& msgs = (*(m_redisReadMsgs.begin())).second;

                // Make sure we have enough storage to construct our generic representations
                PrepStorageForReadMsgs( (CORE::UInt32) msgs.size() );

                // Cycle through the messages received and build the generic representations
                TPubSubMsgsRefVector& msgRefs = m_pubsubMsgsRefs;
                TRedisMsgVector::iterator i = msgs.begin();
                while ( i != msgs.end() )
                {
                    std::string& msgId = (*i).first;
                    TRedisMsgAttributes& msgAttribs = (*i).second;

                    m_pubsubMsgs.push_back( PUBSUB::CBasicPubSubMsg() );
                    PUBSUB::CBasicPubSubMsg& pubsubMsg = m_pubsubMsgs.back();
                    msgRefs.push_back( TPubSubMsgRef() );
                    TPubSubMsgRef& pubsubMsgRef = msgRefs.back();
                    pubsubMsgRef.LinkTo( &pubsubMsg );
                    pubsubMsg.SetOriginClientTopic( CreateSharedPtr() );

                    pubsubMsg.SetReceiveActionId( m_currentReceiveActionId );
                    ++m_currentReceiveActionId;

                    // set basic message properties

                    pubsubMsg.GetMsgId().LinkTo( msgId );
                    CORE::UInt64 unixUtcDt = CORE::StringToUInt64( msgId );
                    pubsubMsg.GetMsgDateTime().FromUnixEpochBasedTicksInMillisecs( unixUtcDt );

                    // set the message attributes

                    if ( msgAttribs.size() > m_pubsubMsgAttribs.size() )
                        m_pubsubMsgAttribs.resize( msgAttribs.size() );
                    pubsubMsg.GetKeyValuePairs().clear();
                    pubsubMsg.GetKeyValuePairs().reserve( msgAttribs.size() );

                    TBufferVector::iterator a = m_pubsubMsgAttribs.begin();
                    TRedisMsgAttributes::iterator n = msgAttribs.begin();
                    while ( n != msgAttribs.end() )
                    {
                        std::string& keyAtt = (*n).first;
                        std::string& valueAtt = (*n).second;

                        CORE::CDynamicBuffer& keyAttBuffer = (*a).first;
                        CORE::CDynamicBuffer& valueAttBuffer = (*a).second;

                        keyAttBuffer.LinkTo( keyAtt );
                        valueAttBuffer.LinkTo( valueAtt );

                        pubsubMsg.GetKeyValuePairs().push_back( PUBSUB::CBasicPubSubMsg::TKeyValuePair() );
                        PUBSUB::CBasicPubSubMsg::TKeyValuePair& kvLink = pubsubMsg.GetKeyValuePairs().back();

                        kvLink.first.LinkTo( keyAttBuffer );
                        kvLink.second.LinkTo( valueAttBuffer );

                        ++n; ++a;
                    }
                    ++i;
                }

                m_readOffset = (*msgs.rbegin()).first;

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

        // We will count the ability to read messages as proof of good health
        UpdateIsHealthyStatus( true );
        
        totalSuccess = true;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ OOM exception: " + e.what() );
    }
    catch ( const sw::redis::MovedError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ MovedError (Redirect failed?) . Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );
        Reconnect();
    }
    catch ( const sw::redis::RedirectionError& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ RedirectionError (rebalance? node failure?). Current slot: " +
                                CORE::ToString( m_redisHashSlot ) + ", new slot: " + CORE::ToString( e.slot() ) + " at node " + e.node().host + ":" + CORE::ToString( e.node().port ) +
                                " exception: " + e.what() );

        Reconnect();
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: Redis++ exception: " + e.what() );
        Reconnect();
        UpdateIsHealthyStatus( false );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):RedisSendSyncImpl: exception: " + e.what() );
        Reconnect();
        UpdateIsHealthyStatus( false );
    }

    if ( totalSuccess )
    {
        // Communicate all the messages received via an event notification
        if ( NotifyObservers( MsgsRecievedEvent, &m_pubsubMsgsRefs ) ) 
        {
            m_msgsInFlight += (CORE::UInt64) m_pubsubMsgsRefs.size();
        }
        else
        {
            return totalSuccess;
        }
    }
    m_redisReadMsgs.clear();
    m_pubsubMsgsRefs.clear();
    m_pubsubMsgAttribs.clear();

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
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Beginning cleanup" );

        RedisClusterPubSubClientTopicReaderPtr redisReader = m_readerThread;
        if ( !redisReader.IsNULL() )
        {
            lock.EarlyUnlock();

            if ( !redisReader->RequestTaskToStop( true ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):Disconnect: Failed to stop reader thread" );
                return false;
            }

            lock.ReLock();
        }

        GUCEF_DELETE m_redisPipeline;
        m_redisPipeline = GUCEF_NULL;

        // the parent client owns the context, we just null it
        m_redisContext.Unlink();

        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Disconnect: Finished cleanup" );
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Disconnect: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Disconnect: Redis++ exception: " + e.what() );        
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Disconnect: exception: " + e.what() );        
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::Reconnect( void )
{GUCEF_TRACE;

    Disconnect();
    GUCEF_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::ToString( this ) + "):RedisReconnect: starting reconnect timer" );
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
            if ( !m_client->GetThreadPool()->RequestTaskToStop( m_readerThread.StaticCast< CORE::CTaskConsumer >(), true ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):CleanupRedisReaderThread: Failed to stop dedicated redis reader thread" );
            }
        }
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):CleanupRedisReaderThread: exception: " + e.what() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::SubscribeImpl( const std::string& readOffset )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    try
    {
        // reset the oed flag, once we read we will see what the new status is
        m_subscriptionIsAtEndOfData = false;

        m_readOffset = readOffset;
        
        // We use blocking "long polling" reads which means we will need a dedicated thread to block until there is data
        // Redis does not support pushing of data directly

        if ( m_readerThread.IsNULL() )
        {
            m_readerThread = RedisClusterPubSubClientTopicReaderPtr( GUCEF_NEW CRedisClusterPubSubClientTopicReader( this ) );
        }
        if ( !m_readerThread.IsNULL() )
        {
            if ( !m_client->GetThreadPool()->StartTask( m_readerThread ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):SubscribeImpl: Failed to start blocking reader thread for async subscription" );
                return false;
            }
        }
        return true;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):SubscribeImpl: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):SubscribeImpl: Redis++ exception: " + e.what() );
        Reconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):SubscribeImpl: exception: " + e.what() );
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
    MT::CScopeMutex lock( m_lock );
    return SubscribeImpl( m_readOffset );
}

/*-------------------------------------------------------------------------*/

bool
CRedisClusterPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

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

    MT::CScopeMutex lock( m_lock );
    return PUBSUB::CPubSubBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_TOPIC_INDEX, CORE::CVariant( m_readOffset ) );
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
CRedisClusterPubSubClientTopic::InitializeConnectivity( void )
{GUCEF_TRACE;

    Disconnect();

    MT::CScopeMutex lock( m_lock );
    try
    {
        m_redisContext = m_client->GetRedisContext();
        if ( m_redisContext.IsNULL() )
            return false;

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
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Connect: Stream \"" + m_config.topicName +
                    "\" hashes to hash slot " + CORE::ToString( m_redisHashSlot ) + " which lives at " + (*i).second.host.HostnameAndPortAsString() + " with node id " + (*i).second.nodeId );
                break;
            }
            ++i;
        }

        if ( m_config.preferDedicatedConnection )
        {
            // Connect to the specific shard used for this channel's stream with a single dedicated connection
            sw::redis::StringView cnSV( m_config.topicName.C_String(), m_config.topicName.Length() );
            GUCEF_DELETE m_redisPipeline;
            m_redisPipeline = GUCEF_NEW sw::redis::Pipeline( m_redisContext->pipeline( cnSV ) );

            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Connect: Successfully created a Redis pipeline. Hash Slot " + CORE::ToString( m_redisHashSlot ) );
        }

        return true;
    }
    catch ( const sw::redis::OomError& e )
    {
		++m_redisErrorReplies;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Connect: Redis++ OOM exception: " + e.what() );
        return false;
    }
    catch ( const sw::redis::Error& e )
    {
		++m_redisErrorReplies;
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Connect: Redis++ exception: " + e.what() );
        m_redisReconnectTimer->SetEnabled( true );
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):Connect: exception: " + e.what() );
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
    catch ( const sw::redis::Error& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: Redis++ exception: " + e.what() );
        ++m_metrics.redisClusterErrorReplies;
        Reconnect();
        return false;
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "RedisClusterPubSubClientTopic(" + CORE::PointerToString( this ) + "):GetRedisClusterNodeMap: exception: " + e.what() );
        Reconnect();
        return false;
    }
}

/*-------------------------------------------------------------------------*/

CRedisClusterPubSubClientTopic::TopicMetrics::TopicMetrics( void )
    : redisClusterErrorReplies( 0 )
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


}

/*-------------------------------------------------------------------------*/

void
CRedisClusterPubSubClientTopic::OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                                            const CORE::CEvent& eventId  ,
                                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( InitializeConnectivity() )
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

 bool
 CRedisClusterPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

bool
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISCLUSTER */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
