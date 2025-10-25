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

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#include "gucefPUBSUB_CPubSubClient.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPIC_H
#include "pubsubpluginUDP_CUdpPubSubClientTopic.h"
#define PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_UDP_CMSMQPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTCONFIG_H
#include "pubsubpluginUDP_CUdpPubSubClientConfig.h"
#define PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTCONFIG_H
#endif /* PUBSUBPLUGIN_UDP_CUDPPUBSUBCLIENTCONFIG_H ? */

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

/**
 *  'Client' for approximation of pub-sub using UDP
 *  Acts as an aggregator of UDP sockets through aggregation of UDP 'topics'
 */
class PUBSUBPLUGIN_UDP_PLUGIN_PRIVATE_CPP CUdpPubSubClient : public PUBSUB::CPubSubClient
{
    public:

    static const CORE::CString TypeName; 

    CUdpPubSubClient( const PUBSUB::CPubSubClientConfig& config );

    virtual ~CUdpPubSubClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetOrCreateTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetDefaultTopicConfig( void ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual PUBSUB::CPubSubClientTopicBasicPtr CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig                    ,
                                                                  CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr GetTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Connect( bool reset ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInitialized( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                    bool includeTopics                        ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      @param cfg the data node structure where you'd like to serialize the config to
     *      @return wheter storing the config was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      @param cfg the config storage object where you'd like to serialize the config to
     *      @return wheter storing the config was successfull
     */
    virtual bool SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      @param cfg pertinent node in the config document from which to load the config
     *      @return success or failure to load all required settings correctly from the given config
     */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      @param cfg the config storage object from which to load the config
     *      @return success or failure to load all required settings correctly from the given config
     */
    virtual bool LoadConfig( const PUBSUB::CPubSubClientConfig& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    CUdpPubSubClientConfig& GetConfig( void );

    private:

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnTransmitTestPacketTimerCycle( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData );

    void RegisterEventHandlers( void );
    
    CUdpPubSubClient( void ); /**< not implemented */

    private:

    typedef CORE::CTEventHandlerFunctor< CUdpPubSubClient > TEventCallback;
    typedef GUCEF::map< CORE::CString, CUdpPubSubClientTopicPtr > TTopicMap;

    CUdpPubSubClientConfig m_config;
    CORE::CTimer* m_metricsTimer;
    TTopicMap m_topicMap;
    COMCORE::CUDPSocket* m_testUdpSocket;
    CORE::CTimer* m_testPacketTransmitTimer;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UDP */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H ? */
