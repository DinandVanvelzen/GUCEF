/*
 *  pubsubpluginWEB: Generic GUCEF COMCORE plugin for providing pubsub approximation via the WEB
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

#ifndef PUBSUBPLUGIN_UDP_CWEBPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_UDP_CWEBPUBSUBCLIENTTOPIC_H

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

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgSerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

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

#ifndef GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#include "gucefWEB_CDummyHTTPServerResource.h"
#define GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CDUMMYHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#include "gucefWEB_CTReadableMapIndexHttpServerResource.h"
#define GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTREADABLEMAPINDEXHTTPSERVERRESOURCE_H ? */

#ifndef GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#include "gucefWEB_CTDataNodeSerializableMapHttpServerResource.h"
#define GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H
#endif /* GUCEF_WEB_CTDATANODESERIALIZABLEMAPHTTPSERVERRESOURCE_H ? */

#ifndef PUBSUBPLUGIN_WEB_CWEBPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginWEB_CWebPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_WEB_CWEBPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_WEB_CWEBPUBSUBCLIENTTOPICCONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CWebPubSubClientTopic;
typedef CORE::CTBasicSharedPtr< CWebPubSubClientTopic, MT::CMutex > TBasicWebPubSubClientTopicPtr;

class RestApiPublishedMessagesResource : public GUCEF::WEB::CCodecBasedHTTPServerResource
{
    public:

    RestApiPublishedMessagesResource( TBasicWebPubSubClientTopicPtr& topic );

    virtual ~RestApiPublishedMessagesResource();

    virtual bool Serialize( const CORE::CString& resourcePath   ,
                            CORE::CDataNode& output             ,
                            const CORE::CString& representation ,
                            const CORE::CString& params         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual TDeserializeState Deserialize( const CORE::CString& resourcePath   ,
                                           const CORE::CDataNode& input        ,
                                           const CORE::CString& representation ,
                                           bool isDeltaUpdateOnly              ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    TBasicWebPubSubClientTopicPtr m_topic;
};

/*-------------------------------------------------------------------------*/


class CWebPubSubClient;

/**
 *  
 * About the REST API feature:
 *  This topic can optionally expose a REST API that allows for checking messages via basic request-response over HTTP and posting new messages over HTTP
 *  The basic endpoint structure is as follows: <httpServerRoot>/clients/<clientTypeName>/topics/<topicName>/messages/<publishActionId> 
 *  For the read (get) use-case this allows one to perform spot checks and debugging for low flow-rate messages
 *  In order to reliably view all messages (for a bit, until you run out of resources) you'd have to disable message retention limits which is not advised
 *  As such its intended to be used as described for reads: for performing spot checks and debugging for low flow-rate messages
 *  
 */
class PUBSUBPLUGIN_WEB_PLUGIN_PRIVATE_CPP CWebPubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                  public CORE::CTSharedObjCreator< CWebPubSubClientTopic, MT::CMutex >
{
    public:

    typedef GUCEF::vector< CORE::UInt32 > UInt32Vector;

    CWebPubSubClientTopic( CWebPubSubClient* client );

    virtual ~CWebPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

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

    const CORE::CString& GetMetricFriendlyTopicName( void ) const;

    virtual bool Publish( CORE::UInt64& publishActionId, const PUBSUB::CIPubSubMsg& msg, bool notify ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark ) GUCEF_VIRTUAL_OVERRIDE;
    bool AcknowledgeReceiptImpl( const PUBSUB::CPubSubBookmark& bookmark, CORE::UInt64 receiveActionId );

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void Shutdown( void );
    
    class TopicMetrics
    {
        public:

        TopicMetrics( void );

    };

    const TopicMetrics& GetMetrics( void ) const;

    const CWebPubSubClientTopicConfig& GetTopicConfig( void ) const;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWebPubSubClientTopic > TEventCallback;

    // Types to implement/hook-up topic interface
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg > TPubSubMsgsVector;    

    private:

    void RegisterEventHandlers( void );

    bool RegisterRestApiEndpoints( void );

    bool SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config );

    static CORE::CString GenerateMetricsFriendlyTopicName( const CORE::CString& topicName );

    bool PublishToRestApi( CORE::UInt64& publishActionId                               , 
                           const PUBSUB::CIPubSubMsg& msg                              , 
                           bool notify                                                 , 
                           const PUBSUB::CPubSubClient* originClient                   ,
                           const CORE::CString& originClientType                       ,
                           const PUBSUB::CPubSubClientTopicBasicPtr& originClientTopic );

    void
    OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void
    OnPublishedMsgPrunerTimerCycle( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData );
    
    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );
    
    private:

    static CORE::CString UnknownClientType;

    typedef GUCEF::map< CORE::UInt64, PUBSUB::CIPubSubMsg::TNoLockSharedPtr >                                              TUInt64dToTIPubSubMsgSPtrMap;
    typedef GUCEF::map< const PUBSUB::CPubSubClientTopic*, TUInt64dToTIPubSubMsgSPtrMap >                                  TPubSubClientTopicToUInt64ToIPubSubMsgSPtrVectorMap;
    typedef GUCEF::map< const PUBSUB::CPubSubClient*, TPubSubClientTopicToUInt64ToIPubSubMsgSPtrVectorMap >                TPubSubClientToPubSubClientTopicMsgsMap;
    typedef GUCEF::vector< const PUBSUB::CPubSubClient* >                                                                  TPubSubClientPtrVector;
    typedef GUCEF::WEB::CTReadableMapIndexHttpServerResource< CORE::CString, TPubSubClientPtrVector >                      TClientIndexMap;
    typedef GUCEF::map< CORE::CString, TPubSubClientPtrVector >                                                            TStringToPubSubClientPtrVectorMap;
    typedef GUCEF::vector< PUBSUB::CPubSubClientTopicBasicPtr >                                                            TPubSubClientTopicPtrVector;
    typedef GUCEF::map< CORE::CString, TPubSubClientTopicPtrVector >                                                       TStringToPubSubClientTopicPtrVectorMap;
    typedef GUCEF::WEB::CTReadableMapIndexHttpServerResource< CORE::CString, TPubSubClientTopicPtrVector >                 TClientTopicIndexMap;
    typedef GUCEF::map< CORE::CString, TStringToPubSubClientTopicPtrVectorMap >                                            TStringToStringToPubSubClientTopicPtrVectorMap;            
    typedef GUCEF::WEB::CDummyHTTPServerResource                                                                           TDummyHttpServerResource;
    typedef GUCEF::WEB::CTReadableMapIndexHttpServerResource< CORE::UInt64, PUBSUB::CIPubSubMsg::TNoLockSharedPtr >        TClientTopicMsgsIndexMap;
    typedef GUCEF::WEB::CTDataNodeSerializableMapHttpServerResource< CORE::UInt64, PUBSUB::CIPubSubMsg::TNoLockSharedPtr > TClientTopicMsgsMap;

    CWebPubSubClient* m_client;
    TPubSubMsgsVector m_pubsubMsgs;
    TMsgsRecievedEventData m_pubsubMsgsRefs;
    CWebPubSubClientTopicConfig m_config;  
    CORE::CTimer* m_reconnectTimer;
    MT::CMutex m_lock;
    CORE::UInt64 m_currentPublishActionId;
    CORE::UInt64 m_currentReceiveActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
    TopicMetrics m_metrics;
    CORE::CString m_metricFriendlyTopicName;
    GUCEF::WEB::CHTTPServer m_httpServer;
    GUCEF::WEB::CDefaultHTTPServerRouter m_httpRouter;
    CORE::CTimer* m_publishedMsgPrunerTimer;
    TPubSubClientToPubSubClientTopicMsgsMap m_publishedMsgs;
    TStringToPubSubClientPtrVectorMap m_publishedClientTypes;
    TStringToStringToPubSubClientTopicPtrVectorMap m_publishedTopicNamesPerClientType;
    PUBSUB::CPubSubMsgSerializerOptions m_pubsubSerializerOptions;
};

/*--------------------------------------------------------------------------*/

typedef CWebPubSubClientTopic::TSharedPtrType   CWebPubSubClientTopicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_WEB_CWEBPUBSUBCLIENTTOPIC_H ? */
