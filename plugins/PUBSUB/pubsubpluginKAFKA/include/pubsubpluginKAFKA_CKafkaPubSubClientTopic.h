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

#ifndef PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPIC_H

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

#ifndef PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginKAFKA_CKafkaPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#include "rdkafka.h"
#include "rdkafkacpp.h"

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CKafkaPubSubClient;

/**
 *  KAFKA implementation of the conceptual pub-sub "topic"
 */
class PUBSUBPLUGIN_KAFKA_PLUGIN_PRIVATE_CPP CKafkaPubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                      public CORE::CTSharedObjCreator< CKafkaPubSubClientTopic, MT::CMutex > ,
                                                                      private RdKafka::EventCb           ,
                                                                      private RdKafka::DeliveryReportCb  ,
                                                                      private RdKafka::ConsumeCb         ,
                                                                      private RdKafka::RebalanceCb       ,
                                                                      private RdKafka::OffsetCommitCb
{
    public:

    static const std::string KafkaMsgHeader_ProducerHostname;
    static const CORE::CString DefaultOffsetResetValue;

    CKafkaPubSubClientTopic( CKafkaPubSubClient* client );

    virtual ~CKafkaPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

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

    virtual bool SubscribeStartingAtTopicIndex( const CORE::CVariant& msgIndexBookmark );
    
    virtual bool SubscribeStartingAtMsgDateTime( const CORE::CDateTime& msgDtBookmark );

    virtual bool SubscribeStartingAtMsgDateTime( const CORE::CVariant& msgDtBookmark );

    virtual bool SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubBookmark GetCurrentBookmark( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopicName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg, PUBSUB::CPubSubBookmark& bookmark ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool RequestSubscriptionMsgArrivalDelay( CORE::UInt32 minDelayInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CKafkaPubSubClientTopicConfig& config );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );

    CORE::UInt32 GetKafkaErrorRepliesCounter( bool resetCounter );

    CORE::UInt32 GetKafkaConnectionErrorsCounter( bool resetCounter );

    CORE::UInt32 GetKafkaMsgsTransmittedCounter( bool resetCounter );

    CORE::UInt32 GetKafkaMsgsReceivedCounter( bool resetCounter );

    CORE::UInt32 GetKafkaMsgsFilteredCounter( bool resetCounter );

    bool UpdateKafkaMsgsReceiveLag( void );
    
    class TopicMetrics
    {
        public:

        TopicMetrics( void );

        CORE::UInt32 kafkaMessagesTransmitted;
        CORE::UInt32 kafkaTransmitQueueSize;
        CORE::UInt32 kafkaTransmitOverflowQueueSize;

        CORE::UInt32 kafkaMessagesReceived;
        
        bool hasKafkaMessagesReceiveLag;
        CORE::UInt64 kafkaMessagesReceiveLagMin;
        CORE::UInt64 kafkaMessagesReceiveLagAvg;
        CORE::UInt64 kafkaMessagesReceiveLagMax;
        bool hasKafkaMessagesReceiveCommitLag;
        CORE::UInt64 kafkaMessagesReceiveCommitLagMin;
        CORE::UInt64 kafkaMessagesReceiveCommitLagAvg;
        CORE::UInt64 kafkaMessagesReceiveCommitLagMax;

        CORE::UInt32 kafkaMessagesFiltered;        

        CORE::UInt32 kafkaErrorReplies;
        CORE::UInt32 kafkaConnectionErrors;
    };

    const TopicMetrics& GetMetrics( void ) const;

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    #pragma pack(push, 1)  // No structure packing, we want to use this as a BSOB interpretation
    struct SPartitionOffset
    {
        CORE::Int32 partitionId;
        CORE::Int64 partitionOffset;
    };
    typedef struct SPartitionOffset TPartitionOffset;
    #pragma pack(pop)

    typedef GUCEF::vector< TPartitionOffset > TPartitionOffsetVector;
    typedef GUCEF::map< CORE::Int32, TPartitionOffset > TPartitionOffsetMap;

    static bool VariantToPartitionOffsets( const CORE::CVariant& indexBookmarkBlob, TPartitionOffsetMap& offsets );
    static bool VariantToPartitionOffset( const CORE::CVariant& partitionOffsetBlob, TPartitionOffset& offset );

    const CORE::CString& GetConsumerConfigSetting( const CORE::CString& keyName, const CORE::CString& defaultValue = CORE::CString::Empty ) const;
    Int64 GetConsumerConfigSettingAsInt64( const CORE::CString& keyName, Int64 defaultValue ) const;

    static void RdKafkaLogCallback( const rd_kafka_t* rk , 
                                    int level            ,
                                    const char* fac      , 
                                    const char* buf      );

    static int RdKafkaStatsCallback( rd_kafka_t *rk  ,
                                     char *json      ,
 				                     size_t json_len ,
						             void *opaque    );

    void RegisterEventHandlers( void );

    void Clear( void );

    void PrepStorageForReadMsgs( CORE::UInt32 msgCount );

    bool SetupBasedOnConfig( void );

    bool CommitConsumerOffsets( bool useAsyncCommit = true );

    bool AcknowledgeReceipt( const TPartitionOffset& offset );

    CORE::Int64 ConvertKafkaConsumerStartOffset( const CORE::CString& startOffsetDescription ,
                                                 CORE::Int32 partitionId                     ,
                                                 CORE::Int32 timeoutInMs                     );

    CORE::Int64 ConvertKafkaConsumerStartOffset( CORE::Int64 startOffsetDescription ,
                                                 CORE::Int32 partitionId            ,
                                                 CORE::Int32 timeoutInMs            );

    static CORE::CString ConvertKafkaConsumerStartOffset( CORE::Int64 offset );

    static const std::string& MsgStatusToString( RdKafka::Message::Status status );

    void LinkReceivedMsg( RdKafka::Message& message, CORE::UInt32 msgIndex );

    void UpdateIsHealthyStatus( bool newStatus );

    virtual void event_cb( RdKafka::Event& event ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void dr_cb( RdKafka::Message& message ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void consume_cb( RdKafka::Message& message, void* opaque ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void rebalance_cb( RdKafka::KafkaConsumer* consumer                  ,
                               RdKafka::ErrorCode err                            ,
                               std::vector<RdKafka::TopicPartition*>& partitions ) GUCEF_VIRTUAL_OVERRIDE;

  /**
   * @brief Set offset commit callback for use with consumer groups
   *
   * The results of automatic or manual offset commits will be scheduled
   * for this callback and is served by RdKafka::KafkaConsumer::consume().
   *
   * If no partitions had valid offsets to commit this callback will be called
   * with \p err == ERR__NO_OFFSET which is not to be considered an error.
   *
   * The \p offsets list contains per-partition information:
   *   - \c topic      The topic committed
   *   - \c partition  The partition committed
   *   - \c offset:    Committed offset (attempted)
   *   - \c err:       Commit error
   */
    virtual void offset_commit_cb( RdKafka::ErrorCode err                         ,
                                   std::vector<RdKafka::TopicPartition*>& offsets ) GUCEF_VIRTUAL_OVERRIDE;

    bool ProcessRdKafkaMessage( RdKafka::Message& message, CORE::UInt32 msgIndex, bool& isFiltered );

    bool ProcessMsgAcks( void );

    void CleanupMsgAcks( void );

    bool RetrieveKafkaCommitedOffsets( void );

    bool QueryKafkaIfSubscriptionIsAtEndOfData( bool& isSubscriptionAtEndOfData ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< CKafkaPubSubClientTopic > TEventCallback;
   
    // Types to implement/hook-up topic interface
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg, gucef_allocator< PUBSUB::CBasicPubSubMsg > >                      TPubSubMsgsVector;
    typedef GUCEF::vector< RdKafka::Message*, gucef_allocator< RdKafka::Message* > >                                  TRdKafkaMsgPtrVector;
    typedef std::pair< CORE::CDynamicBuffer, CORE::CDynamicBuffer >     TBufferPair;
    typedef GUCEF::vector< TBufferPair, gucef_allocator< TBufferPair > >                                              TBufferVector;
    typedef GUCEF::set< CORE::Int64, std::less< CORE::Int64 >, gucef_allocator< CORE::Int64 >  >                      TInt64Set;
    typedef GUCEF::map< CORE::Int32, CORE::Int64 >                        TInt32ToInt64Map;
    typedef GUCEF::map< CORE::Int32, TInt64Set >                          TInt32ToInt64SetMap;
    typedef std::vector< RdKafka::TopicPartition* >                                                                 TRdKafkaTopicPartitionPtrVector;

    CKafkaPubSubClient* m_client;
    CORE::CTimer* m_metricsTimer;
    TPubSubMsgsVector m_pubsubMsgs;
    TMsgsRecievedEventData m_pubsubMsgsRefs;
    TBufferVector m_pubsubMsgAttribs;
    TRdKafkaMsgPtrVector m_rdKafkaMsgs;
    CKafkaPubSubClientTopicConfig m_config;
    RdKafka::Conf* m_kafkaProducerTopicConf;
    RdKafka::Conf* m_kafkaConsumerTopicConf;
    RdKafka::Conf* m_kafkaProducerConf;
    RdKafka::Conf* m_kafkaConsumerConf;
    RdKafka::Producer* m_kafkaProducer;
    RdKafka::Topic* m_kafkaProducerTopic;
    RdKafka::KafkaConsumer* m_kafkaConsumer;
    TInt32ToInt64Map m_kafkaCommitedConsumerOffsets;
    CORE::UInt32 m_kafkaErrorReplies;
    CORE::UInt32 m_kafkaConnectionErrors;
    CORE::UInt32 m_kafkaMsgsTransmitted;
    CORE::UInt32 m_kafkaMessagesReceived;
    CORE::UInt32 m_kafkaMessagesFiltered;
    std::string m_producerHostname;
    bool m_firstPartitionAssignment;
    TInt32ToInt64Map m_consumerReadOffsets;
    TInt32ToInt64Map m_consumerAckdOffsets;
    TInt32ToInt64SetMap m_receivedMsgAcks;
    CORE::UInt64 m_tickCountAtLastOffsetCommit;
    CORE::UInt64 m_tickCountAtConsumeDelayRequest;
    CORE::UInt32 m_requestedConsumeDelayInMs;
    bool m_msgsAckdSinceLastOffsetCommit;
    bool m_consumerOffsetWaitsForExplicitMsgAck;
    CORE::UInt64 m_currentPublishActionId;
    CORE::UInt64 m_currentReceiveActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
    CORE::Int32 m_maxTotalMsgsInFlight;
    TopicMetrics m_metrics;
    bool m_shouldBeConnected;
    bool m_isSubscribed;
    bool m_isSubscriptionAtEndOfData;
    mutable bool m_isHealthy;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------*/

typedef CKafkaPubSubClientTopic::TSharedPtrType CKafkaPubSubClientTopicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAFKA */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPIC_H ? */
