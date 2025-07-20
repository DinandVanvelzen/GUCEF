/*
 *  pubsub2storage: service which transfers between pubsub and storage
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

#include <deque>

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "CIConfigurable.h"
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

#ifndef GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#include "gucefCORE_CDynamicBufferSwap.h"
#define GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSWAP_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#include "gucefPUBSUB_CPubSubClient.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgContainerBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */

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

#ifndef GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H
#include "gucefWEB_CTConfigurableMapHttpServerResource.h"
#define GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTCONFIGURABLEMAPHTTPSERVERRESOURCE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class ChannelSettings : public CORE::CIConfigurable
{
    public:

    typedef PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector TTopicConfigVector;
    enum EChannelMode : CORE::Int32
    {
        CHANNELMODE_UNKNOWN = 0 ,

        CHANNELMODE_PUBSUB_TO_STORAGE = 1,
        CHANNELMODE_STORAGE_TO_PUBSUB = 2
    };
    typedef enum EChannelMode TChannelMode;

    ChannelSettings( void );
    ChannelSettings( const ChannelSettings& src );
    ChannelSettings& operator=( const ChannelSettings& src );

    PUBSUB::CPubSubClientConfig pubsubClientConfig;
    PUBSUB::CPubSubMsgBinarySerializerOptions pubsubBinarySerializerOptions;
    CORE::UInt32 desiredMinimalSerializedBlockSize;
    CORE::UInt32 desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs;    
    CORE::CString vfsStorageRootPath;
    CORE::CString vfsFileExtention;
    CORE::CString encodeCodecFamily;
    CORE::CString encodeCodecName;
    CORE::CString decodeCodecFamily;
    CORE::CString decodeCodecName;
    CORE::Int32 channelId;
    CORE::UInt32 ticketRefillOnBusyCycle;
    bool performPubSubInDedicatedThread;
    bool applyThreadCpuAffinity;
    CORE::UInt32 cpuAffinityForDedicatedPubSubThread;
    CORE::UInt32 cpuAffinityForMainChannelThread;
    bool collectMetrics;
    TChannelMode mode;
    bool subscribeWithoutBookmarkIfNoneIsPersisted;
    bool autoPushAfterStartupIfStorageToPubSub;
    CORE::CDateTime youngestStoragePubSubMsgFileToLoad;
    CORE::CDateTime oldestStoragePubSubMsgFileToLoad;


    PUBSUB::CPubSubClientTopicConfigPtr GetTopicConfig( const CORE::CString& topicName );

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& tree ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

class CIPubSubBookmarkPersistance
{
    public:

    virtual bool GetPersistedBookmark( CORE::Int32 channelId              , 
                                       const CORE::CString& topicName     , 
                                       PUBSUB::CPubSubBookmark& bookmark ) = 0;
};

/*-------------------------------------------------------------------------*/

class CPubSubClientChannel : public CORE::CTaskConsumer
{
    public:

    typedef CORE::CTEventHandlerFunctor< CPubSubClientChannel > TEventCallback;
    typedef enum ChannelSettings::EChannelMode TChannelMode;

    CPubSubClientChannel( CIPubSubBookmarkPersistance* persistance );
    virtual ~CPubSubClientChannel();

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool LoadConfig( const ChannelSettings& channelSettings );

    const ChannelSettings& GetChannelSettings( void ) const;

    bool ConnectPubSubClient( bool reset );

    bool DisconnectPubSubClient( bool destroyClient = false );

    CORE::CDynamicBufferSwap& GetSerializedMsgBuffers( void );

    private:

    void RegisterEventHandlers( void );

    void RegisterTopicEventHandlers( PUBSUB::CPubSubClientTopic& topic );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnPubSubClientReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData );

    void
    OnPubSubTopicMsgsReceived( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    bool TransmitNextPubSubMsgBuffer( void );

    void OnStoredPubSubMsgTransmissionFailure( const CORE::CDateTime& firstMsgDt );
    
    private:

    CPubSubClientChannel( const CPubSubClientChannel& src ); // not implemented

    class TopicLink
    {
        public:

        typedef std::map< CORE::UInt64, PUBSUB::CIPubSubMsg::TNoLockSharedPtr >    TUInt64ToIPubSubMsgNoLockSharedPtrMap;

        PUBSUB::CPubSubClientTopicBasicPtr topic;                                       /**< the actual backend topic access object */ 
        PUBSUB::CPubSubClientTopic::TPublishActionIdVector currentPublishActionIds;     /**< temp placeholder to help prevent allocations per invocation */         
        TUInt64ToIPubSubMsgNoLockSharedPtrMap inFlightMsgs;

        TopicLink( void );
        TopicLink( PUBSUB::CPubSubClientTopicBasicPtr t );
        
        void AddInFlightMsgs( const PUBSUB::CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                              const PUBSUB::CPubSubClientTopic::TIPubSubMsgSPtrVector& msgs              );

        void AddInFlightMsgs( const PUBSUB::CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                              const PUBSUB::CPubSubClientTopic::TPubSubMsgsRefVector& msgs               );
    };

    typedef std::vector< TopicLink > TopicVector;
    typedef MT::CTMailBox< CORE::UInt32 > TBufferMailbox;

    PUBSUB::CPubSubClientPtr m_pubsubClient;
    TopicVector m_topics;
    ChannelSettings m_channelSettings;
    TBufferMailbox m_mailbox;
    TBufferMailbox::TMailVector m_bulkMail;
    CORE::CTimer* m_metricsTimer;
    CORE::CTimer* m_pubsubClientReconnectTimer;
    CORE::CDynamicBufferSwap m_buffers;
    CORE::CDynamicBuffer* m_msgReceiveBuffer;
    CORE::CDateTime m_lastWriteBlockCompletion;    
    PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex m_msgOffsetIndex;
    CIPubSubBookmarkPersistance* m_persistance;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPubSubClientChannel, MT::CMutex > CPubSubClientChannelPtr;

/*-------------------------------------------------------------------------*/

class CStorageChannel : public CORE::CTaskConsumer ,
                        public CIPubSubBookmarkPersistance
{
    public:

    typedef CORE::CTEventHandlerFunctor< CStorageChannel > TEventCallback;
    typedef enum ChannelSettings::EChannelMode TChannelMode;

    class StorageToPubSubRequest : public CORE::CIConfigurable
    {
        public:

        CORE::CDateTime startDt;
        CORE::CDateTime endDt;
        CORE::CString::StringSet vfsPubSubMsgContainersToPush;

        StorageToPubSubRequest( void );
        StorageToPubSubRequest( const CORE::CDateTime& startDt, const CORE::CDateTime& endDt );
        StorageToPubSubRequest( const StorageToPubSubRequest& src );

        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
        virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;
        virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;
    };
    typedef std::deque< StorageToPubSubRequest > StorageToPubSubRequestDeque;

    CStorageChannel();
    CStorageChannel( const CStorageChannel& src );
    virtual ~CStorageChannel() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool WaitForTaskToFinish( CORE::Int32 timeoutInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetPersistedBookmark( CORE::Int32 channelId              , 
                                       const CORE::CString& topicName     , 
                                       PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetLastPersistedMsgAttributes( CORE::Int32 channelId          , 
                                                const CORE::CString& topicName , 
                                                CORE::CVariant& msgId          , 
                                                CORE::CDateTime& msgDt         );

    virtual bool GetLastPersistedMsgAttributesWithOffset( CORE::Int32 channelId          , 
                                                          const CORE::CString& topicName , 
                                                          CORE::CVariant& msgId          , 
                                                          CORE::CDateTime& msgDt         ,
                                                          CORE::UInt32 lastFileOffset    ,
                                                          bool& fileExistedButHasIssue   );

    bool
    GetPathsToPubSubStorageFiles( const CORE::CDateTime& startDt  ,
                                  const CORE::CDateTime& endDt    ,
                                  CORE::CString::StringSet& files ) const;

    bool AddStorageToPubSubRequest( const StorageToPubSubRequest& request );

    void OnUnableToFullFillStorageToPubSubRequest( const StorageToPubSubRequest& failedRequest );
    
    bool LoadConfig( const ChannelSettings& channelSettings );

    const ChannelSettings& GetChannelSettings( void ) const;

    class ChannelMetrics
    {
        public:

        ChannelMetrics( void );

        CORE::UInt32 udpBytesReceived;
        CORE::UInt32 udpPacketsReceived;
        CORE::UInt32 redisMessagesTransmitted;
        CORE::UInt32 redisPacketsInMsgsTransmitted;
        CORE::UInt32 redisPacketsInMsgsRatio;
        CORE::UInt32 redisTransmitQueueSize;
        CORE::UInt32 redisErrorReplies;
    };

    const ChannelMetrics& GetMetrics( void ) const;

    private:

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void RegisterEventHandlers( void );

    CORE::CString GetPathToLastWrittenPubSubStorageFile( CORE::UInt32 lastOffset ) const;

    bool StoreNextReceivedPubSubBuffer( void );

    bool ProcessNextStorageToPubSubRequest( void );

    bool LoadStorageFile( const CORE::CString& vfsPath       ,
                          CORE::CDynamicBuffer& targetBuffer );

    bool GetStartAndEndFromContainerFilename( const CORE::CString& fullPath ,
                                              CORE::CDateTime& startDt      ,
                                              CORE::CDateTime& endDt        ) const;

    private:
    
    ChannelSettings m_channelSettings;
    CORE::CTimer* m_metricsTimer;
    ChannelMetrics m_metrics;
    CPubSubClientChannelPtr m_pubsubClient;
    CORE::CDynamicBuffer* m_msgReceiveBuffer;
    CORE::CString m_vfsFilePostfix;
    CORE::CVariant m_lastPersistedMsgId;
    CORE::CDateTime m_lastPersistedMsgDt;
    CORE::Float32 m_encodeSizeRatio;
    StorageToPubSubRequestDeque m_storageToPubSubRequests;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CStorageChannel, MT::CMutex > CStorageChannelPtr;

/*-------------------------------------------------------------------------*/

class PubSub2Storage;

class RestApiPubSub2StorageInfoResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPubSub2StorageInfoResource( PubSub2Storage* app );

    virtual ~RestApiPubSub2StorageInfoResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    PubSub2Storage* m_app;
};

/*-------------------------------------------------------------------------*/

class RestApiPubSub2StorageConfigResource : public WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPubSub2StorageConfigResource( PubSub2Storage* app, bool appConfig );

    virtual ~RestApiPubSub2StorageConfigResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDeserializeState Deserialize( const CORE::CString& resourcePath   ,
                                           const CORE::CDataNode& input        ,
                                           const CORE::CString& representation ,
                                           bool isDeltaUpdateOnly              ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    PubSub2Storage* m_app;
    bool m_appConfig;
};

/*-------------------------------------------------------------------------*/

class PubSub2Storage : public CORE::CObserver ,
                       public CORE::CIConfigurable
{
    public:

    PubSub2Storage( void );
    virtual ~PubSub2Storage();

    bool Start( void );

    bool SetStandbyMode( bool newMode );

    bool IsGlobalStandbyEnabled( void ) const;

    bool LoadConfig( const CORE::CValueList& appConfig );

    const CORE::CValueList& GetAppConfig( void ) const;

    const CORE::CDataNode& GetGlobalConfig( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    static const CORE::CDateTime& GetAppCompileDateTime( void );

    private:

    typedef CORE::CTEventHandlerFunctor< PubSub2Storage > TEventCallback;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    private:

    typedef std::map< CORE::Int32, ChannelSettings > ChannelSettingsMap;
    typedef std::map< CORE::Int32, CStorageChannelPtr > StorageChannelMap;
    typedef std::map< CORE::CString, CORE::CDataNode::TConstDataNodeSet > TChannelCfgMap;

    bool m_isInStandby;
    bool m_globalStandbyEnabled;
    CORE::UInt16 m_udpStartPort;
    CORE::UInt16 m_channelCount;
    CORE::Int32 m_storageStartChannelID;
    CORE::CString m_redisStreamName;
    CORE::CString m_redisHost;
    CORE::UInt16 m_redisPort;
    StorageChannelMap m_channels;
    ChannelSettingsMap m_channelSettings;
    ChannelSettings m_templateChannelSettings;
    WEB::CHTTPServer m_httpServer;
    WEB::CDefaultHTTPServerRouter m_httpRouter;
    CORE::CValueList m_appConfig;
    CORE::CDataNode m_globalConfig;
    CORE::CTimer m_metricsTimer;
    bool m_transmitMetrics;
};

/*-------------------------------------------------------------------------*/
