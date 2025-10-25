/*
 *  pubsubpluginMSMQ: Generic GUCEF COMCORE plugin for providing pubsub via Microsoft MSMQ
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

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPIC_H
#include "pubsubpluginMSMQ_CMsmqPubSubClientTopic.h"
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTTOPIC_H ? */

#ifndef PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTCONFIG_H
#include "pubsubpluginMSMQ_CMsmqPubSubClientConfig.h"
#define PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTCONFIG_H
#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENTCONFIG_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace MSMQ {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class PUBSUBPLUGIN_MSMQ_PLUGIN_PRIVATE_CPP CMsmqPubSubClient : public PUBSUB::CPubSubClient
{
    public:

    static const CORE::CString TypeName; 

    CMsmqPubSubClient( const PUBSUB::CPubSubClientConfig& config );

    virtual ~CMsmqPubSubClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetOrCreateTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetDefaultTopicConfig( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig                    ,
                                                                  CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr GetTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void DestroyTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters = CORE::CString::EmptyStringSet ) GUCEF_VIRTUAL_OVERRIDE;
    
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

    CMsmqPubSubClientConfig& GetConfig( void );

    protected:

    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< std::wstring > TWStringVector;

    static bool GetPrivateQueues( const std::wstring& computerName ,
                                  TWStringVector& queuePathNames   );

    static bool GetPrivateQueues( const CORE::CString& computerName           ,
                                  CORE::CString::StringVector& queuePathNames );

    static bool GetLocalPrivateQueues( CORE::CString::StringVector& queuePathNames );

    static bool GetPublicQueues( CORE::CString::StringVector& queueIDs );

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnTopicHealthStatusChange( CORE::CNotifier* notifier   ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );
    
    void RegisterEventHandlers( void );

    void RegisterTopicEventHandlers( CMsmqPubSubClientTopicPtr& topic );
    
    CMsmqPubSubClient( void ); /**< not implemented */

    static CORE::UInt64 GetMsmqComputerMetric( const CORE::CString& metricDescription ,
                                               CORE::UInt32 propId                    ,
                                               CORE::UInt32 propType                  );

    static CORE::UInt64 GetComputerGlobalTotalBytesOfAllMessagesOfAllQueues( void );

    private:

    typedef CORE::CTEventHandlerFunctor< CMsmqPubSubClient > TEventCallback;
    typedef GUCEF::map< CORE::CString, CMsmqPubSubClientTopicPtr > TTopicMap;

    CMsmqPubSubClientConfig m_config;
    CORE::CTimer* m_metricsTimer;
    TTopicMap m_topicMap;
    mutable bool m_isHealthy;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MSMQ */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_MSMQ_CMSMQPUBSUBCLIENT_H ? */
