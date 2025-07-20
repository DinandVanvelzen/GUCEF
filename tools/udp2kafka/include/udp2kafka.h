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

 #ifndef GUCEF_APP_UDP2KAFKA
 #define GUCEF_APP_UDP2KAFKA

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H
#include "gucefWEB_CDefaultHTTPServerRouter.h"
#define GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H
#endif /* GUCEF_WEB_CDEFAULTHTTPSERVERROUTER_H ? */

#ifndef GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#include "gucefWEB_CCodecBasedHTTPServerResource.h"
#define GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CCODECBASEDHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#include "rdkafkacpp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

class Udp2KafkaChannel : public CORE::CTaskConsumer ,
                         private RdKafka::EventCb ,
                         private RdKafka::DeliveryReportCb ,
                         private RdKafka::ConsumeCb ,
                         private RdKafka::RebalanceCb ,
                         private RdKafka::OffsetCommitCb
{
    public:

    typedef CORE::CTEventHandlerFunctor< Udp2KafkaChannel > TEventCallback;

    enum EChannelMode
    {
        UNDEFINED      = 0 ,
        KAFKA_PRODUCER     ,
        KAFKA_CONSUMER     ,
        KAFKA_PRODUCER_AND_CONSUMER
    };
    typedef enum EChannelMode TChannelMode;

    static const std::string KafkaMsgHeader_UdpOrigin;
    static const std::string KafkaMsgHeader_ProducerHostname;

    Udp2KafkaChannel();
    Udp2KafkaChannel( const Udp2KafkaChannel& src );
    virtual ~Udp2KafkaChannel() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const;

    class ChannelSettings
    {
        public:

        typedef std::vector< COMCORE::CHostAddress > HostAddressVector;
        typedef std::map< CORE::CString, CORE::CString > StringMap;

        ChannelSettings( void );
        ChannelSettings( const ChannelSettings& src );
        ChannelSettings& operator=( const ChannelSettings& src );

        CORE::CString kafkaBrokers;
        CORE::CString channelTopicName;
        COMCORE::CHostAddress udpInterface;
        HostAddressVector udpMulticastToJoin;
        bool collectMetrics;
        CORE::CString metricsPrefix;
        bool wantsTestPackage;
        TChannelMode mode;
        CORE::CString consumerModeStartOffset;
        CORE::CString consumerModeGroupId;
        HostAddressVector consumerModeUdpDestinations;
        StringMap kafkaProducerGlobalConfigSettings;
        StringMap kafkaConsumerGlobalConfigSettings;
        StringMap kafkaProducerTopicConfigSettings;
        StringMap kafkaConsumerTopicConfigSettings;
        std::string kafkaMsgHeaderUsedForFiltering;
        CORE::CVariant::VariantVector kafkaMsgValuesUsedForFiltering;
        bool addUdpOriginKafkaMsgHeader;
        bool addProducerHostnameKafkaMsgHeader;
    };

    bool LoadConfig( const ChannelSettings& channelSettings );

    const ChannelSettings& GetChannelSettings( void ) const;

    class ChannelMetrics
    {
        public:

        ChannelMetrics( void );

        CORE::UInt32 udpBytesReceived;
        CORE::UInt32 udpMessagesReceived;
        CORE::UInt32 kafkaMessagesTransmitted;
        CORE::UInt32 kafkaTransmitQueueSize;
        CORE::UInt32 kafkaTransmitOverflowQueueSize;

        CORE::UInt32 udpBytesTransmitted;
        CORE::UInt32 udpMessagesTransmitted;
        CORE::UInt32 kafkaMessagesReceived;
        CORE::UInt32 kafkaMessagesFiltered;

        CORE::UInt32 kafkaErrorReplies;
    };

    const ChannelMetrics& GetMetrics( void ) const;

    private:

    typedef COMCORE::CUDPSocket::TPacketEntryVector TPacketEntryVector;
    typedef COMCORE::CUDPSocket::TPacketEntry TPacketEntry;

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

    bool UdpTransmit( RdKafka::Message& message );

    void
    OnUDPSocketError( CORE::CNotifier* notifier   ,
                      const CORE::CEvent& eventID ,
                      CORE::CICloneable* evenData );

    void
    OnUDPSocketClosing( CORE::CNotifier* notifier   ,
                        const CORE::CEvent& eventID ,
                        CORE::CICloneable* evenData );

    void
    OnUDPSocketClosed( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData );

    void
    OnUDPSocketOpened( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData );

    void
    OnUDPPacketsRecieved( CORE::CNotifier* notifier   ,
                          const CORE::CEvent& eventID ,
                          CORE::CICloneable* evenData );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    CORE::UInt32 GetKafkaErrorRepliesCounter( bool resetCounter );

    CORE::UInt32 GetKafkaMsgsTransmittedCounter( bool resetCounter );

    CORE::UInt32 GetKafkaMsgsReceivedCounter( bool resetCounter );

    CORE::UInt32 GetKafkaMsgsFilteredCounter( bool resetCounter );

    CORE::Int64 ConvertKafkaConsumerStartOffset( const CORE::CString& startOffsetDescription ,
                                                 CORE::Int32 partitionId                     ,
                                                 CORE::Int32 timeoutInMs                     );

    CORE::Int64 ConvertKafkaConsumerStartOffset( CORE::Int64 startOffsetDescription ,
                                                 CORE::Int32 partitionId            ,
                                                 CORE::Int32 timeoutInMs            );

    static CORE::CString ConvertKafkaConsumerStartOffset( CORE::Int64 offset );
    
    static const std::string& MsgStatusToString( RdKafka::Message::Status status );

    void RegisterEventHandlers( void );
    
    RdKafka::ErrorCode KafkaProduce( const COMCORE::CIPv4Address& sourceAddress ,
                                     const CORE::CDynamicBuffer& udpPacket    );

    bool CommitConsumerOffsets( void );
    
    private:

    typedef std::deque< TPacketEntry > TPacketEntryQueue;
    typedef ChannelSettings::HostAddressVector HostAddressVector;
    typedef std::map< CORE::Int32, CORE::Int64 > TInt32ToInt64Map;

    RdKafka::Conf* m_kafkaProducerConf;
    RdKafka::Conf* m_kafkaConsumerConf;
    RdKafka::Conf* m_kafkaProducerTopicConf;
    RdKafka::Conf* m_kafkaConsumerTopicConf;
    RdKafka::Producer* m_kafkaProducer;
    RdKafka::Topic* m_kafkaProducerTopic;
    RdKafka::KafkaConsumer* m_kafkaConsumer;

    ChannelSettings m_channelSettings;
    GUCEF::COMCORE::CUDPSocket* m_udpSocket;
    TPacketEntryQueue m_kafkaMsgQueueOverflowQueue;
    CORE::CTimer* m_metricsTimer;
    ChannelMetrics m_metrics;
    CORE::UInt32 m_kafkaErrorReplies;
    CORE::UInt32 m_kafkaMsgsTransmitted;
    CORE::UInt32 m_kafkaMessagesReceived;
    CORE::UInt32 m_kafkaMessagesFiltered;
    HostAddressVector m_kafkaBrokers;
    std::string m_producerHostname;
    bool m_firstPartitionAssignment;
    TInt32ToInt64Map m_consumerOffsets;
    CORE::UInt64 m_tickCountAtLastOffsetCommit;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< Udp2KafkaChannel, MT::CMutex > Udp2KafkaChannelPtr;

/*-------------------------------------------------------------------------*/

class Udp2Kafka;

class RestApiUdp2KafkaInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdp2KafkaInfoResource( Udp2Kafka* app );

    virtual ~RestApiUdp2KafkaInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Udp2Kafka* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiUdp2KafkaConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiUdp2KafkaConfigResource( Udp2Kafka* app, bool appConfig );

    virtual ~RestApiUdp2KafkaConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    Udp2Kafka* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class Udp2Kafka : public CORE::CObserver 
{
    public:

    Udp2Kafka( void );
    virtual ~Udp2Kafka();

    bool Start( void );

    bool LoadConfig( const CORE::CValueList& appConfig   ,
                     const CORE::CDataNode& globalConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    private:

    typedef CORE::CTEventHandlerFunctor< Udp2Kafka > TEventCallback;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData ); 

    void
    OnTransmitTestPacketTimerCycle( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData );

    private:

    typedef std::map< CORE::Int32, Udp2KafkaChannel::ChannelSettings > ChannelSettingsMap;
    typedef std::vector< Udp2KafkaChannelPtr > Udp2KafkaChannelVector;

    CORE::UInt16 m_udpStartPort;
    CORE::UInt16 m_channelCount;
    CORE::Int32 m_kafkaTopicStartChannelID;
    CORE::CString m_kafkaTopicName;
    Udp2KafkaChannelVector m_channels;
    ChannelSettingsMap m_channelSettings;
    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;
    CORE::CTimer m_metricsTimer;
    bool m_transmitMetrics;
    COMCORE::CUDPSocket m_testUdpSocket;
    CORE::CTimer m_testPacketTransmitTimer;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_APP_UDP2KAFKA ? */
