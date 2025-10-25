/*
 *  pubsubpluginUDP: Generic GUCEF COMCORE plugin for providing pubsub approximation via UDP
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

#ifndef PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPIC_H
#define PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPIC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPICCONFIG_H
#include "pubsubpluginUDP_CUdpPubSubClientTopicConfig.h"
#define PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPICCONFIG_H
#endif /* PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPICCONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace UDP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CUdpPubSubClient;

class PUBSUBPLUGIN_UDP_PLUGIN_PRIVATE_CPP CUdpPubSubClientTopic : public PUBSUB::CPubSubClientTopic ,
                                                                  public CORE::CTSharedObjCreator< CUdpPubSubClientTopic, MT::CMutex >
{
    public:

    typedef GUCEF::vector< CORE::UInt32 > UInt32Vector;

    CUdpPubSubClientTopic( CUdpPubSubClient* client );

    virtual ~CUdpPubSubClientTopic() GUCEF_VIRTUAL_OVERRIDE;

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

    virtual bool SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Shutdown( void );
    
    class TopicMetrics
    {
        public:

        TopicMetrics( void );

        CORE::UInt32 udpBytesReceived;
        CORE::UInt32 udpPacketsReceived;
        CORE::UInt32 udpBytesSent;
        CORE::UInt32 udpPacketsSent;
    };

    const TopicMetrics& GetMetrics( void ) const;

    const CUdpPubSubClientTopicConfig& GetTopicConfig( void ) const;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnUDPSocketError( CORE::CNotifier* notifier   ,
                      const CORE::CEvent& eventID ,
                      CORE::CICloneable* evenData );

    void
    OnUDPSocketClosed( CORE::CNotifier* notifier   ,
                       const CORE::CEvent& eventID ,
                       CORE::CICloneable* evenData );

    void
    OnUDPSocketClosing( CORE::CNotifier* notifier   ,
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
    
    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual MT::TLockStatus Lock( CORE::UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CUdpPubSubClientTopic > TEventCallback;

    // Types to implement/hook-up topic interface
    typedef GUCEF::vector< PUBSUB::CBasicPubSubMsg > TPubSubMsgsVector;    

    private:

    void RegisterEventHandlers( void );

    void PrepStorageForReadMsgs( CORE::UInt32 msgCount );

    bool SetupToSubscribe( PUBSUB::CPubSubClientTopicConfig& config );

    static CORE::CString GenerateMetricsFriendlyTopicName( const CORE::CString& topicName );
    
    void
    OnPulseCycle( CORE::CNotifier* notifier    ,
                  const CORE::CEvent& eventId  ,
                  CORE::CICloneable* eventData );
    
    private:

    CUdpPubSubClient* m_client;
    TPubSubMsgsVector m_pubsubMsgs;
    TMsgsRecievedEventData m_pubsubMsgsRefs;
    CUdpPubSubClientTopicConfig m_config;  
    MT::CMutex m_lock;
    CORE::UInt64 m_currentPublishActionId;
    CORE::UInt64 m_currentReceiveActionId;
    TPublishActionIdVector m_publishSuccessActionIds;
    TMsgsPublishedEventData m_publishSuccessActionEventData;
    TPublishActionIdVector m_publishFailureActionIds;
    TMsgsPublishFailureEventData m_publishFailureActionEventData;
    TopicMetrics m_metrics;
    CORE::CString m_metricFriendlyTopicName;
    COMCORE::CUDPSocket m_udpSocket;

};

/*--------------------------------------------------------------------------*/

typedef CUdpPubSubClientTopic::TSharedPtrType   CUdpPubSubClientTopicPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UDP */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPIC_H ? */
