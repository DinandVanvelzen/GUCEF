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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#define GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_PUBSUB_MACROS_H
#include "gucefPUBSUB_macros.h"      /* often used gucefCOMCORE macros */
#define GUCEF_PUBSUB_MACROS_H
#endif /* GUCEF_PUBSUB_MACROS_H ? */

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

class GUCEF_PUBSUB_EXPORT_CPP CPubSubClientFeatures : public CORE::CIConfigurable
{
    public:

    bool supportsPublishing;                    /**< not every client may support full pub-sub: Is publishing supported? */
    bool supportsSubscribing;                   /**< not every client may support full pub-sub: Is subscribing supported? */
    bool supportsSimultaneousPubAndSub;         /**< even if publishing AND subscribing is supported that doesnt mean you can use both simultaneously, this flag will tell */
    
    bool supportsAutoReconnect;                 /**< whether the backend is capable of handling disconnects on its own */
    bool supportsSubscriberRateThrottling;      /**< whether the backend is capable of throttling the rate at which messages are received when subscribed */
    bool supportsSubscriptionMsgArrivalDelayRequests; /**< whether the backend is capable of throttling the rate at which messages are sent to the eventing system by x ms */
    bool supportsSubscriptionEndOfDataEvent;    /**< whether the backend is capable of sending an event message when all available data has been received */
    
    bool supportsBinaryPayloads;                /**< if not supported and a binary payload is given it will be converted to base64 */
    bool supportsMultiHostSharding;             /**< if the pub-sub system supports sharding across multiple hosts */
    bool supportsPerMsgIds;                     /**< whether per message IDs are supported inside a topic, if not the msgId param is irrelevant and not used */
    bool supportsMsgIndex;                      /**< whether a message index or offset concept is supported inside a topic, if not the msgIndex param is irrelevant and not used */
    bool supportsPrimaryPayloadPerMsg;          /**< whether the concept of a per message primary payload is supported. If the backend only supports key-value ideally this feature is not used by the app */
    bool supportsAbsentPrimaryPayloadPerMsg;    /**< whether the concept of a per message primary payload is supported but usage is optional. Only relevant if supportsPrimaryPayloadPerMsg=true */
    bool supportsKeyValueSetPerMsg;             /**< whether the concept of a per message set of key-value attributes is supported. If the backend only supports a primary payload ideally this feature is not used by the app */
    bool supportsDuplicateKeysPerMsg;           /**< whether the concept of a per message set of key-value attributes has the constraint of keys being unique */
    bool supportsMetaDataKeyValueSetPerMsg;     /**< whether the concept of a per message set of key-value meta data attributes is supported */

    bool supportsSubscriberMsgReceivedAck;      /**< whether the concept of a per message acknowledglement of receipt is supported for subscribers */
    bool supportsAutoMsgReceivedAck;            /**< only relevant if 'supportsSubscriberMsgReceivedAck'=true : whether you can forgo sending a msg received ack explicitly at the app level, not recomended for garanteed handling */
    bool supportsAbsentMsgReceivedAck;          /**< only relevant if 'supportsSubscriberMsgReceivedAck'=true : whether you can forgo sending a msg received ack auto or otherwise */
    bool supportsAckUsingLastMsgInBatch;        /**< only relevant if 'supportsSubscriberMsgReceivedAck'=true : whether you can ack using just the last message in a batch on behalf of the entire batch */
    bool supportsAckUsingBookmark;              /**< only relevant if 'supportsSubscriberMsgReceivedAck'=true : whether you can use a Bookmark object to ack instead of a message */
    
    bool supportsBookmarkingConcept;            /**< whether the concept of bookmarking where you last were in the pub-sub subscription is supported at all. The other bookmarking related flags are meaningless if this is false */
    bool supportsSubscribingUsingBookmark;      /**< whether it is possible to subscribe using a client provided bookmark as the subscription starting point */
    bool supportsServerSideBookmarkPersistance; /**< whether the implementation supports server-side persistance of bookmarks, meaning no local persistance feature is needed by the client application */
    bool supportsAutoBookmarking;               /**< whether the implementation takes care of remembering where you are wrt pub-sub subscriber consumption without needing an explicit ack call to the backend */
    bool supportsMsgIdBasedBookmark;            /**< whether the implementation can accept a MsgId to setup pub-sub subscriber consumption */
    bool supportsMsgIndexBasedBookmark;         /**< whether the implementation can accept a Msg Index to setup pub-sub subscriber consumption */
    bool supportsTopicIndexBasedBookmark;       /**< whether the implementation can accept a Msg Index to setup pub-sub subscriber consumption */
    bool supportsMsgDateTimeBasedBookmark;      /**< whether the implementation can accept a message DateTime to setup pub-sub subscriber consumption */
    bool supportsDerivingBookmarkFromMsg;       /**< whether the implementation supports deriving a bookmark when given a message that originated from said implementation */
    
    bool supportsDiscoveryOfAvailableTopics;    /**< whether 'discovering' available topic is supported by the implementation */
    bool supportsGlobPatternTopicNames;         /**< only relevant if 'supportsDiscoveryOfAvailableTopics'=true : whether topics can be specified with a glob pattern to be pattern matched and discovered against available topics */
    bool supportsPatternBasedAggregateTopic;    /**< whether the implementation supports representing a variable number of source topics as a singular topic represented by a topic name pattern */
    bool supportsMetrics;
    bool supportsReplay;                        /**< whether the backend supports replaying stored messages starting at a given key value (e.g. for FIX ResendRequest use-cases) */


    CPubSubClientFeatures( void );
    virtual ~CPubSubClientFeatures();

    /**
     *      Attempts to store the given tree in the file
     *      given according to the method of the codec metadata
     *
     *      @param tree the data tree you wish to store
     *      @return wheter storing the tree was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      Attempts to load data from the given file to the
     *      root node given. The root data will be replaced
     *      and any children the node may already have will be deleted.
     *
     *      @param treeroot pointer to the node that is to act as root of the data tree
     *      @return whether building the tree from the given file was successfull.
     */
    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H ? */

