/*
 *  gucefPUBSUB: GUCEF module providing pub-sub communication facilities
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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <deque>

#ifndef GUCEF_MT_CNOLOCK_H
#include "gucefMT_CNoLock.h"
#define GUCEF_MT_CNOLOCK_H
#endif /* GUCEF_MT_CNOLOCK_H ? */

#ifndef GUCEF_MT_CTMAILBOX_H
#include "gucefMT_CTMailBox.h"
#define GUCEF_MT_CTMAILBOX_H
#endif /* GUCEF_MT_CTMAILBOX_H ? */

#ifndef GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#include "gucefCORE_CTMailboxForSharedCloneables.h"
#define GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H
#endif /* GUCEF_CORE_CTMAILBOXFORSHAREDCLONEABLES_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CIASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBSIDECHANNELSETTINGS_H
#include "gucefPUBSUB_CPubSubSideChannelSettings.h"
#define GUCEF_PUBSUB_CPUBSUBSIDECHANNELSETTINGS_H
#endif /* GUCEF_PUBSUB_CPUBSUBSIDECHANNELSETTINGS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCHANNELSETTINGS_H
#include "gucefPUBSUB_CPubSubChannelSettings.h"
#define GUCEF_PUBSUB_CPUBSUBCHANNELSETTINGS_H
#endif /* GUCEF_PUBSUB_CPUBSUBCHANNELSETTINGS_H ? */

#ifndef GUCEF_PUBSUB_CIPUBSUBJOURNAL_H
#include "gucefPUBSUB_CIPubSubJournal.h"    
#define GUCEF_PUBSUB_CIPUBSUBJOURNAL_H
#endif /* GUCEF_PUBSUB_CIPUBSUBJOURNAL_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CPubSubFlowRouter;

class GUCEF_PUBSUB_EXPORT_CPP CPubSubClientSide : public CORE::CTaskConsumer
{
    public:

    static const CORE::CEvent HealthStatusChangeEvent;                 /**< event msg sent if the health status changes for the side which provides an aggregate health state */
    static const CORE::CEvent PubSubClientInstantiationEvent;          /**< event msg sent when a new pubsub client is instantiated thus invalidating previous references to the underlying client */

    typedef CORE::TCloneableBool            THealthStatusChangeEventData;           /**< boolean flag indicating the health status */
    typedef CPubSubClientPtr                TPubSubClientInstantiationEventData;    /**< shared pointer to the pubsub client */

    typedef CORE::CTEventHandlerFunctor< CPubSubClientSide >                            TEventCallback;
    typedef GUCEF::vector< CPubSubClientSide*, gucef_allocator< CPubSubClientSide* > >    TPubSubClientSideVector;

    class CPubSubClientSideMetrics : public CORE::CTSharedObjCreator< CPubSubClientSideMetrics, MT::CMutex > 
    {
        public:

        typedef typename CORE::CTSharedObjCreator< CPubSubClientSideMetrics, MT::CMutex >::TSharedPtrType PubSubClientSideMetricsPtr;

        CORE::UInt64 publishedMsgsInFlight;
        CORE::UInt64 publishOrAckFailedMsgs;
        CORE::UInt64 lastPublishBatchSize;
        CORE::UInt32 queuedReceiveSuccessAcks;

        bool hasSupportForPublishing;
        bool hasSupportForSubscribing;

        CPubSubClientSideMetrics( void );
        CPubSubClientSideMetrics( const CPubSubClientSideMetrics& src );
        virtual ~CPubSubClientSideMetrics();
    };
    typedef CPubSubClientSideMetrics::PubSubClientSideMetricsPtr    PubSubClientSideMetricsPtr;
    typedef GUCEF::map< CORE::CString, PubSubClientSideMetricsPtr >   StringToPubSubClientSideMetricsMap;

    CPubSubClientSide( const CORE::CString& sideId                ,
                       CPubSubFlowRouter* flowRouter = GUCEF_NULL );

    virtual ~CPubSubClientSide();

    virtual bool OnTaskStart( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnTaskCycle( CORE::CTaskPtr task ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnding( CORE::CTaskPtr task ,
                               bool willBeForced   ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnTaskEnded( CORE::CTaskPtr task ,
                              bool wasForced      ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CPubSubSideChannelSettingsPtr sideSettings );

    CPubSubSideChannelSettingsPtr GetSideSettings( void ) const;

    CORE::CString GetSideId( void ) const;

    bool PerformPubSubClientSetup( bool hardReset = false );

    bool ConnectPubSubClient( bool reset );

    bool DisconnectPubSubClient( bool destroyClient = false );

    bool GetPubSubClientSupportedFeatures( CPubSubClientFeatures& features ) const;

    bool IsRunningInDedicatedThread( void ) const;

    bool HasSubscribersNeedingAcks( void ) const;

    bool PublishMsgs( const CPubSubClientTopic::TPubSubMsgsRefVector& msgs ,
                      CPubSubClientTopic* specificTargetTopic = GUCEF_NULL );

    virtual bool IsHealthy( void ) const;

    /**
     *  Returns whether connectivty is established as applicable for the given backend
     *  It also validates that if the relevant topics are intended for subscriptions purposes that 
     *  an active subscription has been established.
     */
    bool IsConnectedAndSubscribedAsNeeded( void ) const;

    /**
     *  Returns whether connectivty is established as applicable for the given backend
     *  Note that being connected does not imply being subscribed which is logically distinct.
     */
    bool IsConnected( void ) const;

    /**
     *  Returns whether any applicable infrastructure as applicable for the given backend is 
     *  ready for making a connectivity attempt
     */
    bool IsPubSubClientInfraReadyToConnect( void ) const;

    bool AcknowledgeReceipt( CIPubSubMsg::TNoLockSharedPtr& msg );

    /**
     *  Provides access to the current instantiation of the underlying pubsub client for this side
     *  Note that the lifecycle of the client is controlled by the side hence it is possible the underlying instance will be swapped out
     */
    CPubSubClientPtr GetCurrentUnderlyingPubSubClient( void );

    /**
     *  Provides access to the current instantiation of the underlying pubsub client for this side
     *  Note that the lifecycle of the client is controlled by the side hence it is possible the underlying instance will be swapped out
     */
    CPubSubClientTopicBasicPtr GetCurrentUnderlyingPubSubClientTopicByName( const CORE::CString& topicName ) const;

    void SetPerformConnectOnTaskStart( bool performConnectOnStart );

    bool GetCurrentTopicNames( CORE::CString::StringSet& topicNames ) const;

    static void RegisterEvents( void );

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyLock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus ReadOnlyUnlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void RegisterEventHandlers( void );

    void RegisterPubSubClientEventHandlers( CPubSubClientPtr& pubsubClient );

    void RegisterTopicEventHandlers( CPubSubClientPtr& pubsubClient );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnClientHealthStatusChanged( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );

    void
    OnTopicAccessDestroyed( CORE::CNotifier* notifier   ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void
    OnTopicAccessCreated( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void
    OnTopicsAccessAutoCreated( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void
    OnTopicsAccessAutoDestroyed( CORE::CNotifier* notifier    ,
                                 const CORE::CEvent& eventId  ,
                                 CORE::CICloneable* eventData );

    void
    OnPubSubClientReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                       const CORE::CEvent& eventId  ,
                                       CORE::CICloneable* eventData );

    bool ConfigureTopicLink( CPubSubSideChannelSettingsPtr pubSubSideSettings ,
                             CPubSubClientTopicBasicPtr topic                 ,
                             bool reset                                       );

    bool ConnectPubSubClientTopic( CPubSubClientTopic& topic                              ,
                                   const CPubSubClientFeatures& clientFeatures            ,
                                   const CPubSubSideChannelSettingsPtr pubSubSideSettings ,
                                   bool reset                                             );

    template < typename TMsgCollection >
    bool BroadcastPublishMsgsSync( const TMsgCollection& msgs );

    template < typename TMsgCollection >
    bool PublishMsgsSync( const TMsgCollection& msgs              ,
                          CPubSubClientTopic* specificTargetTopic );

    bool ProcessMailbox( void );

    static CORE::CString GetMsgAttributesForLog( const CIPubSubMsg& msg );

    static CORE::CString GenerateMetricsFriendlyTopicName( const CORE::CString& topicName );

    bool GetLatestBookmark( const CPubSubClientTopic& topic ,
                            CPubSubBookmark& bookmark       );

    void DetachFromClient( CPubSubClientPtr pubsubClient );

    typedef CORE::CTMailboxForSharedCloneables< CIPubSubMsg, MT::CNoLock > TPubSubMsgMailbox;

    class TopicLink : public CORE::CTSGNotifier                                ,
                      public CORE::CTSharedObjCreator< TopicLink, MT::CMutex > 
    {
        public:

        TopicLink( void );
        TopicLink( CPubSubClientTopicBasicPtr t );
        virtual ~TopicLink();

        bool ApplySettings( const CPubSubSideChannelSettingsPtr sideSettings );

        void Clear( void );

        /**
         *  Attempts to update the bookmark persistance with the given bookmark
         *  Note that if bookmarking is not supported then this is treated as a fyi no-op
         */
        bool UpdateReceivedMessagesBookmarkAsNeeded( const CIPubSubMsg& msg                  ,
                                                     const CPubSubBookmark& msgBatchBookmark );

        /**
         *  Same functionality as the 'UpdateReceivedMessagesBookmarkAsNeeded' variant that accepts a bookmark except it
         *  has the additional overhead of obtaining the bookmark itself through whatever means are supported.
         *  If you already have the relevant bookmark you should use the other variant instead
         */
        bool UpdateReceivedMessagesBookmarkAsNeeded( const CIPubSubMsg& msg );

        bool FindClosestMsgBatchBookmarkToMsg( const CIPubSubMsg& msg            ,
                                               UInt64& msgBatchBookmarkReceiveId ,
                                               CPubSubBookmark& msgBatchBookmark );

        void CleanupMsgBatchBookmarksUpTo( UInt64 msgBatchBookmarkReceiveId );

        template < typename TMsgCollection >
        bool PublishMsgs( const TMsgCollection& msgs );

        virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

        void SetParentSide( CPubSubClientSide* parentSide );

        void SetFlowRouter( CPubSubFlowRouter* router );

        CORE::UInt64 GetTotalMsgsInFlight( void ) const;

        void SetTopic( CPubSubClientTopicBasicPtr topic );
        
        CPubSubClientTopicBasicPtr GetTopic( void ) const;

        /**
         *  Detaches from the underlying topic implementation without wiping all the logically related topic state
         *  This allows for subsequent relinking with a replacement topic implementation for the same topic and a 'resume'
         *  of normal operations.
         *  Intended for error handling mechanisms where the underlying implementation is 'reset' by wiping it and replacing it with a new one
         */
        void DetachFromTopic( void );

        CORE::CString GetMetricFriendlyTopicName( void ) const;

        void SetJournal( CIPubSubJournalBasicPtr newJournal );

        void SetPubsubBookmarkPersistence( TIPubSubBookmarkPersistenceBasicPtr persistance );
        
        TIPubSubBookmarkPersistenceBasicPtr GetPubsubBookmarkPersistence( void ) const;

        void SetPubsubBookmarkNamespace( const CString& bookmarkNamespace );

        void SetClientFeatures( const CPubSubClientFeatures& clientFeatures );

        bool AcknowledgeReceipt( CIPubSubMsg::TNoLockSharedPtr& msg );

        void SetNeedToTrackInFlightPublishedMsgsForAck( bool isNeeded );

        virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

        private:

        void UpdateTopicMetrics( void );

        void AddInFlightMsgs( const CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                              const CPubSubClientTopic::TIPubSubMsgSPtrVector& msgs              ,
                              bool inFlightDefaultState                                          );

        void AddInFlightMsgs( const CPubSubClientTopic::TPublishActionIdVector& publishActionIds ,
                              const CPubSubClientTopic::TPubSubMsgsRefVector& msgs               ,
                              bool inFlightDefaultState                                          );

        void AddInFlightMsg( CORE::UInt64 publishActionId       ,
                             CIPubSubMsg::TNoLockSharedPtr& msg );

        void
        OnPulseCycle( CORE::CNotifier* notifier    ,
                      const CORE::CEvent& eventId  ,
                      CORE::CICloneable* eventData );

        void
        OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData );

        void
        OnCheckForTimedOutInFlightMessagesTimerCycle( CORE::CNotifier* notifier    ,
                                                      const CORE::CEvent& eventId  ,
                                                      CORE::CICloneable* eventData );
        
        void
        OnPubSubTopicMsgsReceived( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData );

        void
        OnPubSubTopicMsgsPublished( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData );

        void
        OnPubSubTopicMsgsPublishFailure( CORE::CNotifier* notifier    ,
                                         const CORE::CEvent& eventId  ,
                                         CORE::CICloneable* eventData );

        void
        OnPubSubTopicLocalPublishQueueFull( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData );

        bool AcknowledgeReceiptASync( CIPubSubMsg::TNoLockSharedPtr& msg );
        
        bool AcknowledgeReceiptSync( CIPubSubMsg::TNoLockSharedPtr& msg );

        void ProcessAcknowledgeReceiptsMailbox( void );

        void RegisterTopicEventHandlers( CPubSubClientTopicBasicPtr topic );

        template < typename TMsgCollection >
        bool PublishMsgsASync( const TMsgCollection& msgs );

        template < typename TMsgCollection >
        bool PublishMsgsSync( const TMsgCollection& msgs );

        bool PublishMailboxMsgs( void );

        bool RetryPublishFailedMsgs( void );

        private:

        class MsgTrackingEntry
        {
            public:
            CORE::UInt32 retryCount;
            CORE::CDateTime firstPublishAttempt;
            CORE::CDateTime lastPublishAttempt;
            CORE::UInt64 publishActionId;
            CIPubSubMsg::TNoLockSharedPtr msg;
            bool isInFlight;
            bool waitingForInFlightConfirmation;
            bool readyToAckPublishSuccessButAckFailed;

            MsgTrackingEntry( void );
            MsgTrackingEntry( const MsgTrackingEntry& src );
            MsgTrackingEntry( CORE::UInt64 publishActionId, CIPubSubMsg::TNoLockSharedPtr& msg, bool isInFlightState = false );
            MsgTrackingEntry& operator=( const MsgTrackingEntry& src );
        };

        typedef MT::CTMailBox< CIPubSubMsg::TNoLockSharedPtr >                      TPubSubMsgPtrMailbox;
        typedef GUCEF::map< CORE::UInt64, MsgTrackingEntry >                          TUInt64ToMsgTrackingEntryMap;
        typedef GUCEF::map< CORE::UInt64, CPubSubBookmark >                           TUInt64ToBookmarkMap;
        typedef GUCEF::set< CORE::UInt64 >                                            TUInt64Set;
        typedef CORE::CTEventHandlerFunctor< TopicLink >                            TEventCallback;

        CPubSubClientTopicBasicPtr topic;                                       /**< the actual backend topic access object */
        CIPubSubJournalBasicPtr journal;                                        /**< the journal to use for tracing what happened */
        CPubSubClientTopic::TPublishActionIdVector currentPublishActionIds;     /**< temp placeholder to help prevent allocations per invocation */
        TUInt64ToMsgTrackingEntryMap inFlightMsgs;
        TUInt64Set publishFailedMsgs;
        TPubSubMsgPtrMailbox publishAckdMsgsMailbox;
        CORE::Int32 maxASyncPublishMailboxSize;                                 /**< max messages allowed in async publish mailbox when async publishing. <0 means infinite */
        CORE::Int32 maxASyncPublishMailboxSizeDuringAFR;                        /**< max messages allowed in async publish mailbox when diveriting there from a sync send due to 'Awaiting Failure Report'. <0 means infinite */
        CORE::CString metricFriendlyTopicName;
        CORE::CString metricsPrefix;
        PubSubClientSideMetricsPtr metrics;
        CORE::CDateTime lastBookmarkPersistSuccess;
        CORE::Int32 msgsSinceLastBookmarkPersist;
        TUInt64ToBookmarkMap bookmarksOnMsgReceived;
        TPubSubMsgMailbox msgMailbox;
        CPubSubFlowRouter* flowRouter;
        CPubSubClientSide* side;
        CPubSubClientFeatures clientFeatures;
        bool needToTrackInFlightPublishedMsgsForAck;
        bool retryFailedPublishAttempts;
        TIPubSubBookmarkPersistenceBasicPtr pubsubBookmarkPersistence;
        bool awaitingFailureReport;
        CORE::UInt64 totalMsgsInFlight;
        CORE::CString bookmarkNamespace;
        CORE::UInt32 threadIdOfTopicLink;
        CORE::CTimer timedOutInFlightMessagesCheckTimer;
        CORE::CTimer metricsTimer;
    };
    typedef TopicLink::TSharedPtrType  TopicLinkPtr;

    typedef std::pair< const CPubSubClientTopic*, TopicLinkPtr >   TPubSubClientTopicRawPtrAndTopicLinkPair;
    typedef GUCEF::map< CPubSubClientTopic*, TopicLinkPtr, std::less< CPubSubClientTopic* >, gucef_allocator< TPubSubClientTopicRawPtrAndTopicLinkPair > > TopicPtrMap;
    typedef GUCEF::map< CORE::CString, TopicLinkPtr, std::less< CORE::CString >, gucef_allocator< TPubSubClientTopicRawPtrAndTopicLinkPair > >             TopicNameMap;
    typedef GUCEF::set< CPubSubClientTopicBasicPtr, std::less< CPubSubClientTopicBasicPtr >, gucef_allocator< CPubSubClientTopicBasicPtr > >               TopicSet;

    CPubSubClientPtr m_pubsubClient;
    CPubSubClientFeatures m_clientFeatures;
    TIPubSubBookmarkPersistenceBasicPtr m_pubsubBookmarkPersistence;
    CORE::CString m_bookmarkNamespace;
    TopicPtrMap m_topicPtrs;
    TopicNameMap m_topics;
    CPubSubSideChannelSettingsPtr m_sideSettings;
    TPubSubMsgMailbox m_broadcastMailbox;
    CORE::CTimer m_metricsTimer;
    CORE::CTimer m_pubsubClientReconnectTimer;
    CORE::CString m_sideId;
    CORE::UInt32 m_threadIdOfSide;
    CPubSubFlowRouter* m_flowRouter;
    mutable bool m_isHealthy;
    bool m_connectOnTaskStart;
    CIPubSubJournalBasicPtr m_journal;
    MT::CReadWriteLock m_rwdataLock;

    private:

    CPubSubClientSide( const CPubSubClientSide& src ); // not implemented
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPubSubClientSide, MT::CMutex >  CPubSubClientSidePtr;
typedef GUCEF::vector< CPubSubClientSidePtr, gucef_allocator< CPubSubClientSidePtr > >    TPubSubClientSidePtrVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

