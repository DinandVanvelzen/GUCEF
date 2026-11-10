/*
 *  pubsubpluginAWSSNS: Generic GUCEF COMCORE plugin for providing pubsub via AWS's SNS
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

#ifndef PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginAWSSNS_CAwsSnsPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPICCONFIG_H ? */

#include <aws/sns/model/PublishRequest.h>
#include <aws/sns/model/PublishResult.h>
#include <aws/sns/model/PublishBatchRequest.h>
#include <aws/sns/model/PublishBatchRequestEntry.h>
#include <aws/sns/model/PublishBatchResult.h>
#include <aws/sns/model/MessageAttributeValue.h>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace AWSSNS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CAwsSnsPubSubClient;

/**
 *  AWS SNS implementation of the conceptual pub-sub "topic", messages can be published to the topic
 */
class PUBSUBPLUGIN_AWSSNS_PLUGIN_PRIVATE_CPP CAwsSnsPubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                        public CORE::CTSharedObjCreator< CAwsSnsPubSubClientTopic, MT::CMutex >
{
    public:

    CAwsSnsPubSubClientTopic( CAwsSnsPubSubClient* client );

    virtual ~CAwsSnsPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClient* GetClient( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool InitializeConnectivity( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribed( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsPublishingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsSubscribingSupported( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Subscribe( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubBookmark GetCurrentBookmark( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetTopicName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CIPubSubMsg::TIPubSubMsgConstRawPtrVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( TPublishActionIdVector& publishActionIds, const PUBSUB::CIPubSubMsg::TIPubSubMsgSPtrVector& msgs, bool notify ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const CORE::CVariant& receipt );

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );

    class TopicMetrics
    {
        public:

        TopicMetrics( void );

        CORE::UInt32 snsMessagesPublished;
        CORE::UInt32 snsPublishErrors;
    };

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    void RegisterEventHandlers( void );


    template< class T >
    bool
    AddAttributesToSnsMsg( T& snsMsg                                          , 
                           const PUBSUB::CIPubSubMsg::TKeyValuePairs& kvPairs ,
                           bool addPrefix                                     ,
                           const CORE::CAsciiString& prefixToAdd              ,
                           CORE::UInt32& msgByteSize                          );

    template< class T >
    bool
    TranslateToSnsMsgOfType( T& snsMsg                      , 
                             const PUBSUB::CIPubSubMsg* msg , 
                             CORE::UInt32& msgByteSize      );

    
    bool
    TranslateToSnsMsg( Aws::SNS::Model::PublishRequest& snsMsg ,
                       const PUBSUB::CIPubSubMsg* msg          ,
                       CORE::UInt32& approxMsgByteSize         );

    bool
    TranslateToSnsBatchMsg( Aws::SNS::Model::PublishBatchRequestEntry& snsMsg ,
                            const PUBSUB::CIPubSubMsg* msg                    ,
                            CORE::UInt64 publishActionId                      ,
                            CORE::UInt32& approxMsgByteSize                   );

    bool ApplySnsMessageAttributeNameContraints( CORE::CAsciiString& candidateName );

    private:

    typedef CORE::CTEventHandlerFunctor< CAwsSnsPubSubClientTopic > TEventCallback;
   

    CAwsSnsPubSubClient* m_client;
    CAwsSnsPubSubClientTopicConfig m_config;
    MT::CMutex m_lock;
    Aws::String m_topicArn;
    CORE::UInt32 m_snsMaximumMessageSize;
    TopicMetrics m_metrics;
    PUBSUB::CIPubSubMsg::TIPubSubMsgConstRawPtrVector m_publishBulkMsgRemapStorage;
    CORE::UInt64 m_currentPublishActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
};

/*-------------------------------------------------------------------------*/

typedef CAwsSnsPubSubClientTopic::TSharedPtrType        CAwsSnsPubSubClientTopicPtr;
typedef CAwsSnsPubSubClientTopic::TBasicSharedPtrType   CAwsSnsPubSubClientTopicBasicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSNS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPIC_H ? */
