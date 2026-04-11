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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_PUBSUB_CIPUBSUBMSG_H
#include "gucefPUBSUB_CIPubSubMsg.h"
#define GUCEF_PUBSUB_CIPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CIPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_CIPUBSUBJOURNAL_H
#include "gucefPUBSUB_CIPubSubJournal.h"
#define GUCEF_PUBSUB_CIPUBSUBJOURNAL_H
#endif /* GUCEF_PUBSUB_CIPUBSUBJOURNAL_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

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

class CPubSubClient;
class CPubSubClientSide;
class CPubSubClientTopicConfig;

/**
 *  Class providing the abstract client-side representation of a 'topic' within the publish-subscribe system
 *
 *  A topic is the logical representation of that to which one can publish and subscribe.
 *  The actual nomenclature would depend on the underlying implementation but the pub-sub concept should apply.
 *
 *  About publishActionId:
 *  One of the problemsets when dealing with providing a generic interface to a plethora of different pubsub
 *  style backends is that each of the dependencies have their own limits in what they can or cannot achieve
 *  The ability to use an eventing approach, a mainstay in these libraries, to async be notified of the publish 
 *  success or failure of your earlier publish action is thus something that is hard to standardize in any meaningful way.
 *  In order the allow tracing between the cause and effect, the Publish() action and the eventual resulting 
 *  success or failure indicator on a per-message basis the publishActionId concept was introduced.
 *  It is backend agnostic and simplistic enough that regardless of backend capabilities every backend should be able to
 *  implement support.
 *  You might ask why not just send the originally published messages along with the publish outcome event notifications?
 *  The answer for that is performance and backend simplicity. Often times that would possibly result in putting additional 
 *  message memory management overhead/complexity requirements on all backends. Consider that due to the 'wrapper' nature
 *  of our IPubSubMsg interface the actual storage of said message is abstracted such that often we can link to backend
 *  specific messages/memory storage thus avoiding memory copies. This same benefit though also means that we cannot efficiently
 *  keep the messages around for async notification in backends because the message lifecycle on a Publish() is not managed by 
 *  the backend and is only valid for the duration of the Publish() call. For this reason we'd have to brute force copy messages
 *  in order to utilize them for the notification.
 *  The publishActionId is a way of sidestepping this message lifecycle management problemset while still allowing for async
 *  event messaging with indicators on a per message basis.
 *  Why not support a void* or similar for user-data you ask? Simply because such mechanisms have historically been responsible for
 *  many reliability issues due to invalid pointer access, especially in cases like this where due to the very nature of the abstractions
 *  you have no idea what the lifecycle of the pointer you passed would look like. Using a backend managed integer avoids this problem.
 *  It is mandatory for all backends to support the publishActionId concept. All backends must provide an id per message 
 *  for any message where a publish attempt is/will-be made. All backends shall issue ids in the same order as the messages 
 *  given within the context of multi-message batch operations. As such the message batch index can be matched against the id index 
 *  to get the id per specific message. Backends are mandated to scope the uniqueness of publishActionId on a per-topic basis 
 *  and coupled with the lifecycle of the topic object lifecycle.
 *
 *  How to use publishActionId:
 *  When you Publish() a message the backend will provide you with a publishActionId value. 
 *  You are required to pass 0 as the publishActionId unless you are re-publishing a previously Published message for which the backend
 *  already provided a publishActionId. In such a retry scenario you required to pass the original publishActionId.
 *  At minimum you can use publishActionId to trace your specific message's journey in any relevant error logging of a given backend to help diagnose issues. 
 *  Keep in mind that backends could use async processing so without such an id the ordering of the logs does not always garantee correlation. 
 *  Beyond that you can specify notify=true in the likely case that you do not just want to perform fire-and-forget publishing.
 *  If you did select notify=false note that on Publish() invocation the same id rules apply with the difference that there will be no
 *  further publish success or failure communication via notification. If you are Ok with fire-and-forget publishing using notify=false
 *  will allow for slightly less work to be carried out and thus amounts to better overall efficiency.
 *  With notify=true you will receive MsgsPublishedEvent or MsgsPublishFailureEvent which in periodic batch fashion will notify observers
 *  of the publish action outcomes. The Ids that are passed along with those events are the same ids you were given at the time of Publish() 
 *  invocation. As such you can correlate what happened to the original messages you published. If you want to obtain the original published
 *  messages the calling application can itself manage the message lifecycle such that using the publishActionId the messages are mapped to
 *  said ids across the points in time.
 *  Note that you should not persist the publishActionId out-of-proc as it is a runtime in-memory aspect only with no correlation retained between 
 *  distinct process runs or backend topic lifecycles.
 *
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubClientTopic : public CORE::CTSGNotifier                                       
{
    public:

    typedef CIPubSubMsg::TIPubSubMsgConstRawPtrVector                       TIPubSubMsgConstRawPtrVector;
    typedef CIPubSubMsg::TIPubSubMsgRawPtrVector                            TIPubSubMsgRawPtrVector;
    typedef CIPubSubMsg::TIPubSubMsgSPtrVector                              TIPubSubMsgSPtrVector;
    typedef CIPubSubMsg*                                                                                         TPubSubMsgRef;
    typedef CORE::CTCloneableExpansion< GUCEF::vector< TPubSubMsgRef, gucef_allocator< TPubSubMsgRef > > >         TPubSubMsgsRefVector;    
    typedef UInt64                                                                                               TPublishActionId;
    typedef CORE::CTCloneableExpansion< GUCEF::vector< TPublishActionId, gucef_allocator< TPublishActionId > > >   TPublishActionIdVector;        

    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent DisconnectedEvent;
    static const CORE::CEvent ConnectionErrorEvent;
    static const CORE::CEvent InternalErrorEvent;
    static const CORE::CEvent MsgsRecievedEvent;                /**< event msg sent when the backend has received 1-N messages from the pubsub system as a subscriber */
    static const CORE::CEvent MsgsPublishedEvent;               /**< event msg sent when the backend has successfully published messages */
    static const CORE::CEvent MsgsPublishFailureEvent;          /**< event msg sent when the backend has failed to successfully publish messages */
    static const CORE::CEvent SubscriptionEndOfDataEvent;       /**< event msg sent when all available data as subscribed has been recieved, provided the backend supports this feature */
    static const CORE::CEvent LocalPublishQueueFullEvent;       /**< if the backend supports queuing messages locally then we have to deal with the possibility of said queue reaching a max capacity */
    static const CORE::CEvent PublishThrottleEvent;             /**< if you overload the backend system as a published you may receive a throttle event msg. In such a case you should back off your publish rate if possible */
    static const CORE::CEvent HealthStatusChangeEvent;          /**< event msg sent if the health status changes for the topic */

    typedef TPubSubMsgsRefVector                                    TMsgsRecievedEventData;
    typedef CORE::CTLinkedCloneable< TPublishActionIdVector >       TMsgsPublishFailureEventData;
    typedef CORE::CTLinkedCloneable< TPublishActionIdVector >       TMsgsPublishedEventData;
    typedef CORE::TCloneableBool                                    THealthStatusChangeEventData;   /**< boolean flag indicating the health status */

    static void RegisterEvents( void );

    CPubSubClientTopic( void );

    CPubSubClientTopic( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CPubSubClientTopic();

    virtual CPubSubClient* GetClient( void ) = 0;

    virtual const CPubSubClient* GetClient( void ) const;

    /**
     *  Explicitly calls for initialization of connectivity if relevant for the given backend
     *  The intent is to allow the application to choose when to take the setup cost hit and also provide better error reporting
     *  Backends should be implement such that if no explicit call to InitializeConnectivity() is made and such a call is relevant that 
     *  lazy initialization is used upon subscribe/publish
     */
    virtual bool InitializeConnectivity( bool reset ) = 0;

    virtual bool Subscribe( void ) = 0;                                                /**< attempt to commence subscription. This will subscribe per config and defaults of the given backend */
    virtual bool SubscribeStartingAtBookmark( const CPubSubBookmark& bookmark ) = 0;   /**< attempt to commence subscription starting at the given client-side provided bookmark */

    /**
     *  Allows for making a request to the pub-sub backend for throttling the rate at which new messages are received for the topic
     *  This is an optional feature, not all backends may support it. You can query the backend's supported features and check
     *  the "supportsSubscriberRateThrottling" feature flag to verify whether this feature is supported
     *  Note that a rate that is smaller or equal to 0 is to be interpreted as "no rate limit"
     */
    virtual bool RequestMaxSubscriptionMsgsPerSecRate( CORE::Int64 maxMsgsPerSec );

    /**
     *  Allows for making a request to the pub-sub backend for throttling the rate at which new messages are received for the topic
     *  by pausing arrival of new messages to the eventing system. 
     *  You can query the backend's supported features and check the "supportsSubscriptionMsgArrivalDelayRequests" feature flag to verify whether this feature is supported
     */ 
    virtual bool RequestSubscriptionMsgArrivalDelay( CORE::UInt32 minDelayInMs );

    /**
     *  If a backend has bookmark support then it is required to implement this member function such
     *  that it returns the bookmark for the currently notified batch of messages received with the bookmark
     *  representing the begin position of said batch, meaning a retransmission based on said bookmark as a starting position
     *  would include the entirety of the current batch.
     *  This garantee is true for the duration of syncronous event handling of the 'MsgsRecievedEvent' event
     *  This becomes exta important when performing bookmark management for a backend that does not supporting deriving a bookmark
     *  from a message in which case this becomes the only avenue by which you would know the bookmark to use for a given batch of messages 
     */ 
    virtual CPubSubBookmark GetCurrentBookmark( void ) = 0;

    virtual bool Disconnect( void ) = 0;

    /**
     *  Allows checking if connectivity is currently established to whatever the backend system is
     */
    virtual bool IsConnected( void ) const = 0;

    /**
     *  Allows checking if we have an active subscription
     */
    virtual bool IsSubscribed( void ) const = 0;

    virtual bool IsHealthy( void ) const = 0;

    /**
     *  Provides a snapshot-in-time state of whether the subscription has reached a point where no more data is known to be 
     *  available. Not all backends will support this as it is tied with the 'supportsSubscriptionEndOfDataEvent' feature toggle.
     *  If the feature is supported but we are not subscribed the return value is to be 'false'
     *
     *  Default implementation always returns 'false'
     *  If the feature 'supportsSubscriptionEndOfDataEvent' is supported the backend should provide an overriding implementation 
     */
    virtual bool IsSubscriptionAtEndOfData( void ) const;

    virtual bool IsPublishingSupported( void ) const = 0;

    virtual bool IsSubscribingSupported( void ) const = 0;

    virtual const CORE::CString& GetTopicName( void ) const = 0;

    /**
     *  Utility/convenience member functions that allow common calling patterns
     *  Note that these all call Publish( const CIPubSubMsg& msg )
     *  Specific backend implementations may optionally override these if there is a gain from doing so
     */
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const CBasicPubSubMsg::TBasicPubSubMsgVector& msgs, bool notify );
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TIPubSubMsgConstRawPtrVector& msgs, bool notify );
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TIPubSubMsgRawPtrVector& msgs, bool notify );
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TIPubSubMsgSPtrVector& msgs, bool notify );    
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const TPubSubMsgsRefVector& msgs, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CString& msgId, const CORE::CString& key, const CORE::CString& value, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CString& msgId, const CORE::CDynamicBuffer& payload, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CString& msgId, const CORE::CString& payload, bool notify );    
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CVariant& msgId, const CORE::CVariant& payload, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CString& msgId, const CORE::CString& key, const CORE::CDynamicBuffer& value, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CString& msgId, const CORE::CDynamicBuffer& key, const CORE::CDynamicBuffer& value, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CVariant& msgId, const CORE::CDynamicBuffer& key, const CORE::CDynamicBuffer& value, bool notify );
    virtual bool Publish( TPublishActionId& publishActionId, const CORE::CString& msgId, const CORE::CValueList& kvPairs, bool notify );    

    /**
     *  Publish the given message to the pub-sub backend system using that system's specifics
     *  If the backend does not support publishing messages this operation will always fail
     *
     *  @return success flag with true on success per backend criterea or false on failure
     */
    virtual bool Publish( UInt64& publishActionId, const CIPubSubMsg& msg, bool notify ) = 0;

    /**
     *  Ack that the given message was successfully received and/or handled by the application
     *  If the backend does not support ack'ing messages this operation should be implement as 
     *  a no-op returning success (true) accepting it as an fyi.
     *
     *  If the backend does support bookmarks but cannot derive bookmarks from messages then the 
     *  operation should fail because the wrong variant of 'AcknowledgeReceipt' is being used based on
     *  the advertised backend capabilities. In that case the 'AcknowledgeReceipt' that accepts a 
     *  bookmark should be used instead.
     *
     *  @return success flag with true on success per backend criterea or false on failure     
     */    
    virtual bool AcknowledgeReceipt( const CIPubSubMsg& msg );
    virtual bool AcknowledgeReceipt( const CPubSubBookmark& bookmark );

    /**
     *  If supported by the backend implementation this will set the given bookmark
     *  to information that act for said bookmark indicates locality of this message 
     *  If per the advertised backend features the backend does not support deriving a bookmark from a message
     *  then this operation will always fail for said backend. 
     *  In such a case you should be using the GetCurrentBookmark() mechanic instead if bookmarking as a whole is supported
     */
    virtual bool DeriveBookmarkFromMsg( const CIPubSubMsg& msg, CPubSubBookmark& bookmark ) const;

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const = 0;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) = 0;

    /**
     *  Sets the journal implementation (if any) to be used by the client
     */
    virtual bool SetJournal( CIPubSubJournalBasicPtr journal );

    virtual CIPubSubJournalBasicPtr GetJournal( void ) const;

    /**
     *  Requests that the backend replay stored messages starting at the given bookmark position.
     *  Only relevant for backends that support the 'supportsReplay' feature.
     *
     *  The replay runs asynchronously on a dedicated task thread. The returned CFutureResult
     *  represents the full replay lifecycle; it resolves when all messages have been delivered
     *  or an error occurs. The task consumer calls requestingSide->OnReplayMsgsReceived() for
     *  each batch and requestingSide->OnReplayComplete() when done.
     *
     *  @param startBookmark    Inclusive lower bound. Use BOOKMARK_TYPE_INDEX_KEY_VALUE with key field + value,
     *                          or BOOKMARK_TYPE_NOT_INITIALIZED meaning "from beginning of data".
     *  @param endBookmark      Inclusive upper bound. Use BOOKMARK_TYPE_NOT_INITIALIZED meaning "to end of data".
     *  @param replayRequestId  Router-assigned unique ID for this replay request.
     *  @param requestingSide   Side that requested the replay; will receive replayed messages via OnReplayMsgsReceived.
     *  @param requestingTopic  Topic on requestingSide that should receive the replayed messages.
     *  @return CFutureResult tracking the async replay task; HasNoFuture() == true on failure to initiate.
     */
    virtual CORE::CFutureResult RequestReplay( const CPubSubBookmark& startBookmark   ,
                                               const CPubSubBookmark& endBookmark     ,
                                               CORE::UInt64           replayRequestId ,
                                               CPubSubClientSide*     requestingSide  ,
                                               CPubSubClientTopic*    requestingTopic );

    /**
     *  Called by the persistence plugin (or the router) when the replay for a given replayRequestId
     *  has been completed (all messages up to endKeyValue have been delivered or data is exhausted).
     *
     *  @param replayRequestId  The router-assigned ID of the completed replay request.
     */
    virtual void OnReplayComplete( CORE::UInt64 replayRequestId );

    /**
     *  Helper function that can be used to resolve some common macros in strings
     *  The generic implementation at this level supports the following macros
     *      - {topicName}
     *      - {clientType}
     */
    virtual bool TryResolveMacrosInString( const CORE::CString& testString ,
                                           CORE::CString& resultString     ) const;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPubSubClientTopic, MT::CMutex >         CPubSubClientTopicPtr;
typedef CORE::CTBasicSharedPtr< CPubSubClientTopic, MT::CMutex >    CPubSubClientTopicBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

