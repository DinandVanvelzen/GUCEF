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

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#include "hiredis.h"
#include "async.h"
#include "redis++.h"

#ifndef PUBSUBPLUGIN_REDISCLUSTER_REDISNODE_H
#include "pubsubpluginREDISCLUSTER_RedisNode.h"
#define PUBSUBPLUGIN_REDISCLUSTER_REDISNODE_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_REDISNODE_H ? */

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPICREADER_H
#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClientTopicReader.h"
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPICREADER_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPICREADER_H ? */

#ifndef REDISINFO_CREDISCLUSTERKEYCACHE_H
#include "redisinfo_CRedisClusterKeyCache.h"
#define REDISINFO_CREDISCLUSTERKEYCACHE_H
#endif /* REDISINFO_CREDISCLUSTERKEYCACHE_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRedisClusterPubSubClient;

/**
 *  Clustered Redis implementation of the conceptual pub-sub "topic"
 *  For Redis we use streams to implement the topic concept
 */
class PUBSUBPLUGIN_REDISCLUSTER_PLUGIN_PRIVATE_CPP CRedisClusterPubSubClientTopic : public PUBSUB::CPubSubClientTopic  ,
                                                                                    public CORE::CTSharedObjCreator< CRedisClusterPubSubClientTopic, MT::CMutex >
{
    public:

    typedef std::pair< sw::redis::StringView, sw::redis::StringView >      TRedisArgPair;
    typedef GUCEF::vector< TRedisArgPair, gucef_allocator< TRedisArgPair > > TRedisArgs;

    CRedisClusterPubSubClientTopic( CRedisClusterPubSubClient* client );

    virtual ~CRedisClusterPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClient* GetClient( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool InitializeConnectivity( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPublishingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscriptionAtEndOfData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Subscribe( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubBookmark GetCurrentBookmark( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopicName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetMetricFriendlyTopicName( void ) const;
    const CORE::CString& GetMetricsPrefix( void ) const;

    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg, PUBSUB::CPubSubBookmark& bookmark ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RequestSubscriptionMsgArrivalDelay( CORE::UInt32 minDelayInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const CRedisClusterPubSubClientTopicConfig& GetTopicConfig( void ) const;

    class TopicMetrics
    {
        public:

        TopicMetrics( void );

        CORE::UInt32 redisErrorReplies;
        CORE::UInt32 redisTimeouts;
        CORE::UInt32 msgsReceived;
        CORE::Float32 fieldsInMsgsReceivedRatio;
        CORE::UInt32 msgsBytesReceived;
        CORE::UInt32 msgsTransmitted;        
        CORE::Float32 fieldsInMsgsTransmittedRatio;
        CORE::UInt32 msgsInFlight;
    };

    const TopicMetrics& GetMetrics( void ) const;

    CORE::UInt32 GetMsgsInFlightGauge( void ) const;
    CORE::UInt32 GetRedisErrorRepliesCounter( bool resetCounter );
    CORE::UInt32 GetRedisTimeoutsCounter( bool resetCounter );
    CORE::UInt32 GetMsgsTransmittedCounter( bool resetCounter );
    CORE::UInt32 GetFieldsInMsgsTransmittedCounter( bool resetCounter );
    CORE::UInt32 GetMsgsReceivedCounter( bool resetCounter );
    CORE::UInt32 GetFieldsInMsgsReceivedCounter( bool resetCounter );
    CORE::UInt32 GetMsgsBytesReceivedCounter( bool resetCounter );

    bool RedisSendSyncImpl( CORE::UInt64& publishActionId, const sw::redis::StringView& msgId, const TRedisArgs& kvPairs, bool notify );

    bool RedisRead( void );

    CORE::UInt32 CalculateRedisHashSlot( const CORE::CString& keyStr ) const;

    bool GetRedisClusterNodeMap( RedisNodeMap& nodeMap );
    
    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );

    /**
     *  Sets the journal implementation (if any) to be used by the client
     */
    virtual bool SetJournal( PUBSUB::CIPubSubJournalBasicPtr journal ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CIPubSubJournalBasicPtr GetJournal( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void Reconnect( void );

    void RegisterEventHandlers( void );

    void RegisterPulseGeneratorEventHandlers( CORE::PulseGeneratorPtr pulseGenerator );

    bool PrepStorageForReadMsgs( CORE::UInt32 msgCount );

    bool SubscribeImpl( const std::string& readOffset );

    void CleanupRedisReaderThread( void );

    void UpdateIsHealthyStatus( bool newStatus );

    static CORE::CString GenerateMetricsFriendlyTopicName( const CORE::CString& topicName );

    private:

    typedef CORE::CTEventHandlerFunctor< CRedisClusterPubSubClientTopic > TEventCallback;
    
    // Types to read from redis-plus-plus
    typedef std::pair< std::string, std::string > TRedisMsgAttribute;
    typedef GUCEF::vector< TRedisMsgAttribute, gucef_allocator< TRedisMsgAttribute > > TRedisMsgAttributes;
    typedef std::pair< std::string, TRedisMsgAttributes > TRedisMsg;
    typedef GUCEF::vector< TRedisMsg, gucef_allocator< TRedisMsg > > TRedisMsgVector;
    typedef std::pair< const std::string, TRedisMsgAttributes > TStringRedisMsgVectorPair;
    typedef std::unordered_map< std::string, TRedisMsgVector, std::hash< std::string >, std::equal_to< std::string >, gucef_allocator< TStringRedisMsgVectorPair > > TRedisMsgByStream;
    typedef std::insert_iterator< TRedisMsgByStream > TRedisMsgByStreamInserter;

    // Types to implement/hook-up topic interface
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg, gucef_allocator< PUBSUB::CBasicPubSubMsg > > TPubSubMsgsVector;
    typedef std::pair< CORE::CDynamicBuffer, CORE::CDynamicBuffer > TBufferPair;
    typedef GUCEF::vector< TBufferPair, gucef_allocator< TBufferPair > > TBufferVector;

    class RedisTopicVarsForReading
    {
        public:

        TRedisMsgByStream m_redisReadMsgs;
        TPubSubMsgsVector m_pubsubMsgs;
        TMsgsRecievedEventData m_pubsubMsgsRefs;
        TBufferVector m_pubsubMsgAttribs;
        std::string m_readOffset;
        CORE::UInt64 m_currentReceiveActionId;

        MT::CMutex m_lock;

        RedisTopicVarsForReading( void );
    };

    RedisTopicVarsForReading m_readVars;
    CRedisClusterPubSubClient* m_client;
    sw::redis::Pipeline* m_redisPipeline;
    RedisClusterPtr m_redisContext;
    CORE::UInt32 m_redisErrorReplies;
    CORE::UInt32 m_redisTimeoutErrors;
    CORE::UInt32 m_msgsReceived;
    CORE::UInt32 m_fieldsInMsgsReceived;
    CORE::UInt32 m_msgsBytesReceived;
    CORE::UInt32 m_redisMsgsTransmitted;
    CORE::UInt32 m_redisFieldsInMsgsTransmitted;
    CORE::UInt32 m_redisHashSlot;
    CORE::Int32 m_redisMaxXreadCount;
    CORE::UInt32 m_redisXreadBlockTimeoutInMs;
    
    TRedisArgs m_redisMsgArgs;
    COMCORE::CHostAddress m_redisShardHost;
    CORE::CString m_redisShardNodeId;
    CORE::CTimer* m_redisReconnectTimer;
    CRedisClusterPubSubClientTopicConfig m_config;
    PUBSUB::CIPubSubJournalBasicPtr m_journal;
        
    RedisClusterPubSubClientTopicReaderPtr m_readerThread;
    bool m_needToTrackAcks;
    bool m_subscriptionIsAtEndOfData;
    bool m_isSubscribed;
    CORE::Int32 m_maxTotalMsgsInFlight;
    CORE::Int64 m_msgsInFlight;
    CORE::UInt64 m_currentPublishActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
    TopicMetrics m_metrics;
    CORE::CString m_metricFriendlyTopicName;
    CORE::CString m_metricsPrefix;
    mutable bool m_isHealthy;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

typedef CRedisClusterPubSubClientTopic::TSharedPtrType          CRedisClusterPubSubClientTopicPtr;
typedef CRedisClusterPubSubClientTopic::TBasicSharedPtrType     CRedisClusterPubSubClientTopicBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISCLUSTER */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H ? */
