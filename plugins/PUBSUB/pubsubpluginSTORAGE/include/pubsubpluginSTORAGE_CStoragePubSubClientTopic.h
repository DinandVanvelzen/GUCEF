/*
 *  pubsubpluginSTORAGE: Generic GUCEF plugin for PUBSUB module functionality 
 *                       providing a storage interface
 *
 *  Copyright (C) 1998 - 2022.  Dinand Vanvelzen
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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#include "gucefCORE_CDynamicBufferSwap.h"
#define GUCEF_CORE_CDYNAMICBUFFERSWAP_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERSWAP_H ? */

#ifndef GUCEF_CORE_CTBASICSHAREDPTROBJOPERATORS_H
#include "gucefCORE_CTBasicSharedPtrObjOperators.h"
#define GUCEF_CORE_CTBASICSHAREDPTROBJOPERATORS_H
#endif /* GUCEF_CORE_CTBASICSHAREDPTROBJOPERATORS_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CIPUBSUBBOOKMARKPERSISTENCE_H
#include "gucefPUBSUB_CIPubSubBookmarkPersistence.h"
#define GUCEF_PUBSUB_CIPUBSUBBOOKMARKPERSISTENCE_H
#endif /* GUCEF_PUBSUB_CIPUBSUBBOOKMARKPERSISTENCE_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginSTORAGE_CStoragePubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTVFSTASK_H
#include "pubsubpluginSTORAGE_CStoragePubSubClientTopicVfsTask.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTVFSTASK_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTVFSTASK_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXWRITER_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexWriter.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXWRITER_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXWRITER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CStoragePubSubClient;

/**
 *
 *  Storage requests are handled in a phased approach to allow some degree of interweaving
 *  requests. Managing storage -> pubsub requests is broken down into the following phases:
 *      Phase 1: Obtain a listing of container files that match the request range
 *      Phase 2: Process each container file of a request and translate to 'received' pubsub messages
 *  In addition there is a Phase 0 specific to persistent storage requests. Such requests never go away after
 *  data has been served but rather revert to a holding area where we wait for more data that matches the request
 *  Once data is found that matches the persitent request it starts progressing through the phases again.
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStoragePubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                          public CORE::CTSharedObjCreator< CStoragePubSubClientTopic, MT::CMutex >
{
    public:

    typedef GUCEF::vector< CORE::UInt32, gucef_allocator< CORE::UInt32 > >    UInt32Vector;

    class CStorageBookmarkInfo
    {
        public:

        UInt8 bookmarkFormatVersion;
        mutable UInt8 doneWithFile;
        mutable CORE::UInt32 msgIndex;
        mutable CORE::UInt32 offsetInFile;
        CORE::CString vfsFilePath;

        inline bool operator<( const CStorageBookmarkInfo& other ) const
        {
            if ( vfsFilePath == other.vfsFilePath )
                return msgIndex < other.msgIndex;
            else
                return vfsFilePath < other.vfsFilePath;
        }

        inline bool operator==( const CStorageBookmarkInfo& other ) const
        {
            return vfsFilePath == other.vfsFilePath && msgIndex == other.msgIndex;
        }

        inline CStorageBookmarkInfo& operator=( const CStorageBookmarkInfo& src )
        {
            if ( &src != this )
            {
                bookmarkFormatVersion = src.bookmarkFormatVersion;
                doneWithFile          = src.doneWithFile;
                msgIndex              = src.msgIndex;
                offsetInFile          = src.offsetInFile;
                vfsFilePath           = src.vfsFilePath;
            }
            return *this;
        }

        inline bool IsEmpty( void ) const
        {
            return 0 == bookmarkFormatVersion;
        }

        inline CStorageBookmarkInfo( const CORE::CString& vfsPath = CORE::CString::Empty )
            : bookmarkFormatVersion( 1 )
            , doneWithFile( 0 )
            , msgIndex( 0 )
            , offsetInFile( 0 )
            , vfsFilePath( vfsPath )
        {}

        inline CStorageBookmarkInfo( const CStorageBookmarkInfo& src )
            : bookmarkFormatVersion( src.bookmarkFormatVersion )
            , doneWithFile( src.doneWithFile )
            , msgIndex( src.msgIndex )
            , offsetInFile( src.offsetInFile )
            , vfsFilePath( src.vfsFilePath )
        {}
    };
    
    class CContainerRangeInfo : public CStorageBookmarkInfo ,
                                public CORE::CTSharedObjCreator< CContainerRangeInfo, MT::CNoLock >
    {
        public:

        mutable bool hasEndDelimiter;
        mutable CORE::UInt32 lastMsgIndex;
        mutable CORE::UInt32 lastOffsetInFile;

        CORE::UInt64 minActionId;
        CORE::UInt64 maxActionId;

        bool operator<( const CContainerRangeInfo& other ) const;
        bool operator==( const CContainerRangeInfo& other ) const;
        CContainerRangeInfo& operator=( const CContainerRangeInfo& src );
        CContainerRangeInfo& operator=( const CStorageBookmarkInfo& src );

        CContainerRangeInfo( const CORE::CString& vfsFilePath = CORE::CString::Empty );
        CContainerRangeInfo( const CContainerRangeInfo& src );
        CContainerRangeInfo( const CStorageBookmarkInfo& src );
    };

    typedef CORE::CTBasicSharedPtr< CContainerRangeInfo, MT::CNoLock >              TCContainerRangeInfoPtr;
    typedef CORE::CTBasicSharedPtrObjOperators< CContainerRangeInfo, MT::CNoLock >  TCContainerRangeInfoReference;
    typedef GUCEF::set< TCContainerRangeInfoReference >                               TCContainerRangeInfoReferenceSet;

    CStoragePubSubClientTopic( CStoragePubSubClient* client           ,
                               CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CStoragePubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClient* GetClient( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool InitializeConnectivity( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscriptionAtEndOfData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPublishingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Subscribe( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual bool SubscribeStartingAtMsgId( const CORE::CVariant& msgIdBookmark );

    virtual bool SubscribeStartingAtMsgDateTime( const CORE::CDateTime& msgDtBookmark );

    virtual bool SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    bool SubscribeStartingAtBookmarkInfo( const CStorageBookmarkInfo& bookmarkInfo );

    /**
     *  Convenience overload: uses the sidecar index to translate a logical key value into
     *  a storage bookmark and then calls SubscribeStartingAtBookmarkInfo().
     *
     *  @param keyNameWithPrefix  "mk:<keyName>" for metadata keys, "k:<keyName>" for KV-pair keys.
     *  @param startKeyValue      First key value to deliver (lower-bound, inclusive).
     */
    bool SubscribeStartingAtKeyValue( const CORE::CString& keyNameWithPrefix ,
                                      CORE::UInt64         startKeyValue     );

    /**
     *  Initiates an out-of-band replay of stored messages starting at the position described by startBookmark.
     *  The replayed messages are delivered via the normal MsgsRecievedEvent and then routed back to the requesting side.
     *  The replayRequestId, requestingSide, and requestingTopic are stored so that OnOutOfBandReplayComplete() can be
     *  signalled when the replay subscription reaches end-of-data.
     *  startBookmark must be BOOKMARK_TYPE_INDEX_KEY_VALUE; endBookmark BOOKMARK_TYPE_NOT_INITIALIZED means "to end of data".
     */
    virtual CORE::CFutureResult RequestReplay( const PUBSUB::CPubSubBookmark& startBookmark   ,
                                               const PUBSUB::CPubSubBookmark& endBookmark     ,
                                               CORE::UInt64                   replayRequestId ,
                                               PUBSUB::CPubSubClientSide*     requestingSide  ,
                                               PUBSUB::CPubSubClientTopic*    requestingTopic ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubBookmark GetCurrentBookmark( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopicName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    const CORE::CString& GetMetricFriendlyTopicName( void ) const;

    virtual bool Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TIPubSubMsgConstRawPtrVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TIPubSubMsgRawPtrVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TIPubSubMsgSPtrVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;    
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TPubSubMsgsRefVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool DeriveBookmarkFromMsg( const PUBSUB::CIPubSubMsg& msg, PUBSUB::CPubSubBookmark& bookmark ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CStoragePubSubClientTopicConfig& config );

    template <class C>
    bool LoadConfigType( const C& config );

    virtual void Shutdown( void );

    /**
     *  Sets the journal implementation (if any) to be used by the client
     */
    virtual bool SetJournal( PUBSUB::CIPubSubJournalBasicPtr journal ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CIPubSubJournalBasicPtr GetJournal( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;

    class TopicMetrics
    {
        public:

        TopicMetrics( void );      

        CORE::UInt32 queuedReadyToReadBuffers;
        CORE::UInt32 smallestBufferSizeInBytes;
        CORE::UInt32 largestBufferSizeInBytes;
        CORE::UInt32 msgsLoadedFromStorage;
        CORE::UInt32 msgBytesLoadedFromStorage;
        CORE::UInt32 msgsNotifiedAsReceived;
        CORE::UInt32 msgsWrittenToStorage;
        CORE::UInt32 msgBytesWrittenToStorage;
        CORE::UInt32 storageCorruptionDetections;
        CORE::UInt32 storageDeserializationFailures;
    };

    const TopicMetrics& GetMetrics( void ) const;

    const CStoragePubSubClientTopicConfig& GetTopicConfig( void ) const;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void OnVfsFileCreated( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventid  ,
                           CORE::CICloneable* eventdata );

    void OnVfsFileRenamed( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void OnVfsDirRenamed( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );
    
    void OnVfsArchiveMounted( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventid  ,
                              CORE::CICloneable* eventdata );

    typedef CORE::CTEventHandlerFunctor< CStoragePubSubClientTopic > TEventCallback;

    // Types to implement/hook-up topic interface
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg > TPubSubMsgsVector;

    class StorageToPubSubRequest : public CORE::CIConfigurable
    {
        public:

        CORE::CDateTime startDt;
        CORE::CDateTime endDt;
        TCContainerRangeInfoReferenceSet vfsPubSubMsgContainersToPush;
        TCContainerRangeInfoReferenceSet vfsPubSubMsgContainersPushed;
        TCContainerRangeInfoReferenceSet vfsPubSubMsgContainersTransmitted;
        bool okIfZeroContainersAreFound;
        bool isPersistentRequest;

        StorageToPubSubRequest( void );

        StorageToPubSubRequest( const CORE::CDateTime& startDt  , 
                                const CORE::CDateTime& endDt    , 
                                bool okIfZeroContainersAreFound ,
                                bool isPersistentRequest        );
        
        StorageToPubSubRequest( const StorageToPubSubRequest& src );

        virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
        virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;
        virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;
    };
    typedef std::deque< StorageToPubSubRequest, gucef_allocator< StorageToPubSubRequest > > StorageToPubSubRequestDeque;
    
    typedef GUCEF::vector< bool, gucef_allocator< bool > > TBoolVector;
    
    class StorageBufferMetaData
    {
        public:

        PUBSUB::CPubSubMsgContainerBinarySerializer::TMsgOffsetIndex msgOffsetIndex;
        PUBSUB::CPubSubMsgContainerBinarySerializer::TBasicPubSubMsgVector msgs;
        TMsgsRecievedEventData pubsubMsgsRefs;
        CORE::UInt32 msgsInFlight;
        CORE::UInt32 lastMsgTransmittedIndex;
        CORE::UInt32 ackdMsgCount;
        TBoolVector msgAcks;        
        CORE::UInt32 lastAckdMsgIndex;
        CORE::CDateTime lastAckdMsgTime;
        TPublishActionIdVector actionIds;         
        StorageToPubSubRequest* linkedRequest;
        TCContainerRangeInfoReference linkedRequestEntry;
        bool isReleased;
        bool isBeingWritten;

        void Clear( void );

        StorageBufferMetaData( void );
    };
    typedef GUCEF::map< CORE::CDynamicBuffer*, StorageBufferMetaData > TStorageBufferMetaDataMap;

    static bool SyncBookmarkInfoToBookmark( const CStorageBookmarkInfo& info  , 
                                            PUBSUB::CPubSubBookmark& bookmark );

    static bool SyncBookmarkToBookmarkInfo( const PUBSUB::CPubSubBookmark& bookmark ,
                                            CStorageBookmarkInfo& info              );
    
    void RegisterEventHandlers( CORE::PulseGeneratorPtr pulseGenerator );

    void RegisterPulseGeneratorEventHandlers( CORE::PulseGeneratorPtr pulseGenerator );

    bool SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config );

    static CORE::CString GenerateMetricsFriendlyTopicName( const CORE::CString& topicName );
    static CORE::CString GenerateDefaultVfsStorageContainerFileExt( void );

    CORE::CString GenerateVfsStorageContainerFileExt( void );

    template < typename T >
    bool PublishViaMsgPtrs( TPublishActionIdVector& publishActionIds           , 
                            const GUCEF::vector< T, gucef_allocator< T > >& msgs , 
                            bool notify                                        );

    void NotifyPublishResults( void );
    
    bool AcknowledgeReceiptImpl( const CStorageBookmarkInfo& bookmark ,
                                 StorageBufferMetaData* metaData      );

    bool GetBookmarkForReceiveActionId( CORE::UInt64 receiveActionId      , 
                                        PUBSUB::CPubSubBookmark& bookmark ) const;

    bool GetBookmarkInfoForReceiveActionId( CORE::UInt64 receiveActionId       , 
                                            StorageBufferMetaData** metaData   ,
                                            CStorageBookmarkInfo& bookmarkInfo );
    
    bool GetStorageBufferMetaDataPtrForReceiveActionId( CORE::UInt64 receiveActionId     , 
                                                        StorageBufferMetaData** metaData );

    bool GetStorageBufferMetaDataPtrForVfsContainerPath( const CORE::CString& vfsContainerPath , 
                                                         bool onlyCurrentlyLoaded              ,
                                                         StorageBufferMetaData** metaData      );

    bool
    GetPathsToPubSubStorageFiles( const CORE::CDateTime& startDt           ,
                                  const CORE::CDateTime& endDt             ,
                                  TCContainerRangeInfoReferenceSet& files  ) const;

    bool AddStorageToPubSubRequest( const StorageToPubSubRequest& request );

    bool SubscribeToVfsTopicPathEvents( void );

    void OnUnableToFullFillStorageToPubSubRequest( const StorageToPubSubRequest& failedRequest );

    bool StoreNextReceivedPubSubBuffer( bool onlyStoreIfBufferHasContent );

    bool ProcessNextPubSubRequestRelatedFile( void );

    bool LocateFilesForStorageToPubSubRequest( void );

    bool LoadStorageFile( const CORE::CString& vfsPath       ,
                          CORE::CDynamicBuffer& targetBuffer ,
                          bool& failureIsOk                  );

    bool GetTimestampsFromContainerFilename( const CORE::CString& fullPath     ,
                                             CORE::CDateTime& firstMsgDt       ,
                                             CORE::CDateTime& lastMsgDt        ,
                                             CORE::CDateTime& containerStartDt ) const;

    CORE::CDynamicBufferSwap& GetSerializedMsgBuffers( void );
    
    bool TransmitNextPubSubMsgBuffer( void );

    bool TransmitNextPubSubMsgBatch( CORE::CDynamicBuffer* readBuffer      ,
                                     StorageBufferMetaData* bufferMetaData );

    void ProgressRequest( StorageBufferMetaData* bufferMetaData ,
                          bool isTransmitted                    ,
                          bool isAcked                          );

    void ProgressRequest( StorageToPubSubRequest* request ,
                          bool isAcked                    );

    void OnStoredPubSubMsgTransmissionFailure( const CORE::CDateTime& firstMsgDt );

    void
    OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void
    OnNoAckRetransmitCheckCycle( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );
    
    void
    OnSyncVfsOpsTimerCycle( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void
    OnBufferContentTimeWindowCheckCycle( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData );

    friend class CStoragePubSubClientTopicVfsTask;

    void PerformASyncVfsWork( void );
    void OnEndOfASyncVfsWork( void );

    bool BeginVfsOps( void );
    bool StopVfsOps( void );

    bool FinalizeWriteBuffer( StorageBufferMetaData* bufferMetaData, CORE::UInt32 bufferOffset, bool onlyIfBufferHasContent );
    bool FinalizeWriteBuffer( bool onlyIfBufferHasContent );

    void AddPublishActionIdsToNotify( const TPublishActionIdVector& publishActionIds, bool success );

    void AddVfsFileToSubscription( const CORE::CString& vfsFilePath );

    CORE::CString ResolveVfsRootPath( void ) const;

    CORE::CString ResolveVfsStorageRootPathForFullyAckdContainers( void ) const;

    bool WasContainerMoved( const CORE::CString& vfsFilePath ) const;

    void UpdateIsHealthyStatus( bool newStatus );
    
    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );

    void
    OnVfsInitializationCompleted( CORE::CNotifier* notifier    ,
                                  const CORE::CEvent& eventId  ,
                                  CORE::CICloneable* eventData );
                  
    CORE::UInt32 GetMsgsLoadedFromStorageCounter( bool resetCounter );
    CORE::UInt32 GetMsgBytesLoadedFromStorageCounter( bool resetCounter );
    CORE::UInt32 GetMsgsWrittenToStorageCounter( bool resetCounter );
    CORE::UInt32 GetMsgsNotifiedAsReceivedCounter( bool resetCounter );
    CORE::UInt32 GetMsgBytesWrittenToStorageCounter( bool resetCounter );
    CORE::UInt32 GetStorageCorruptionDetectionCounter( bool resetCounter );
    CORE::UInt32 GetStorageDeserializationFailuresCounter( bool resetCounter );

    protected:

    CStoragePubSubClient* m_client;
    TPubSubMsgsVector m_pubsubMsgs;
    CStoragePubSubClientTopicConfig m_config;
    PUBSUB::CIPubSubJournalBasicPtr m_journal;
    CORE::CTimer* m_syncVfsOpsTimer;    
    CORE::CTimer* m_reconnectTimer;
    CORE::CTimer* m_noAckRetransmitTimer;
    CORE::CTimer* m_bufferContentTimeWindowCheckTimer;
    MT::CMutex m_lock;
    CORE::UInt64 m_currentPublishActionId;
    CORE::UInt64 m_currentReceiveActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
    TopicMetrics m_metrics;
    CORE::CString m_metricFriendlyTopicName;
    mutable bool m_isHealthy;
    bool m_needToTrackAcks;
    bool m_subscriptionIsAtEndOfData;
    bool m_vfsInitIsComplete;
    CORE::Int64 m_maxTotalMsgsInFlight;

    CORE::CDynamicBuffer* m_currentReadBuffer;
    CORE::CDynamicBuffer* m_currentWriteBuffer;
    CStorageBookmarkInfo m_currentBookmarkInfo;
    PUBSUB::CPubSubBookmark m_currentBookmark;
    PUBSUB::TIPubSubBookmarkPersistenceBasicPtr m_pubsubBookmarkPersistence;
    CORE::CString m_vfsFilePostfix;
    CORE::CString m_vfsRootPath;
    CORE::CString m_vfsStorageRootPathForFullyAckdContainers;
    CORE::CVariant m_lastPersistedMsgId;
    CORE::CDateTime m_lastPersistedMsgDt;
    CORE::Float32 m_encodeSizeRatio;
    StorageToPubSubRequestDeque m_stage0StorageToPubSubRequests;  // <- persistent requests, holding area
    StorageToPubSubRequestDeque m_stage1StorageToPubSubRequests;  // <- requests that need to be matched to files
    StorageToPubSubRequestDeque m_stage2StorageToPubSubRequests;  // <- requests that need their files loaded into buffers to serve the request
    TCContainerRangeInfoReferenceSet m_completedContainers;
   
    CORE::CDynamicBufferSwap m_buffers;
    CORE::CDateTime m_lastWriteBlockCompletion;    
    TStorageBufferMetaDataMap m_storageBufferMetaData;
    CStoragePubSubClientTopicVfsTaskPtr m_vfsOpsThread;

    CORE::UInt32 m_msgsLoadedFromStorage;
    CORE::UInt32 m_msgBytesLoadedFromStorage;
    CORE::UInt32 m_msgsNotifiedAsReceived;
    CORE::UInt32 m_msgsWrittenToStorage;
    CORE::UInt32 m_msgBytesWrittenToStorage;
    CORE::UInt32 m_storageCorruptionDetections;
    CORE::UInt32 m_storageDeserializationFailures;

    typedef GUCEF::map< CORE::CString, CStoragePubSubIndexWriter > TIndexWriterMap;
    TIndexWriterMap m_indexWriters;   /**< keyed by index filename, e.g. "idx.mk.fix_seq_num.su64.sidx" */
};

/*-------------------------------------------------------------------------*/

typedef CStoragePubSubClientTopic::TSharedPtrType        CStoragePubSubClientTopicPtr;
typedef CStoragePubSubClientTopic::TBasicSharedPtrType   CStoragePubSubClientTopicBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPIC_H ? */
