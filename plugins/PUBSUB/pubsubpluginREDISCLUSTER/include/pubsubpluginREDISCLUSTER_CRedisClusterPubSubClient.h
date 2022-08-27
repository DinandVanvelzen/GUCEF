/*
 *  pubsubpluginREDISCLUSTER: Generic GUCEF COMCORE plugin for providing pubsub via Redis
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

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENT_H
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENT_H

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

#include "hiredis.h"
#include "async.h"
#include "redis++.h"

#ifndef PUBSUBPLUGIN_REDISCLUSTER_REDISNODE_H
#include "pubsubpluginREDISCLUSTER_RedisNode.h"
#define PUBSUBPLUGIN_REDISCLUSTER_REDISNODE_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_REDISNODE_H ? */

#ifndef PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H
#include "pubsubpluginREDISCLUSTER_CRedisClusterPubSubClientTopic.h"
#define PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENTTOPIC_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace REDISCLUSTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class PUBSUBPLUGIN_REDISCLUSTER_PLUGIN_PRIVATE_CPP CRedisClusterPubSubClient : public PUBSUB::CPubSubClient
{
    public:

    static const CORE::CString TypeName;
    
    typedef std::map< const PUBSUB::CPubSubClientTopicConfig* , CORE::CString::StringSet > TTopicConfigPtrToStringSetMap;

    CRedisClusterPubSubClient( const PUBSUB::CPubSubClientConfig& config );

    virtual ~CRedisClusterPubSubClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const PUBSUB::CPubSubClientTopicConfig* GetTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopic* CreateTopicAccess( const PUBSUB::CPubSubClientTopicConfig& topicConfig ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopic* GetTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetMultiTopicAccess( const CORE::CString& topicName    ,
                                      PubSubClientTopicSet& topicAccess ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetMultiTopicAccess( const CORE::CString::StringSet& topicNames ,
                                      PubSubClientTopicSet& topicAccess          ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateMultiTopicAccess( const PUBSUB::CPubSubClientTopicConfig& topicConfig ,
                                         PubSubClientTopicSet& topicAccess                    ) GUCEF_VIRTUAL_OVERRIDE;

    bool AutoCreateMultiTopicAccess( const PUBSUB::CPubSubClientTopicConfig& templateTopicConfig ,
                                     const CORE::CString::StringSet& topicNameList                ,
                                     PubSubClientTopicSet& topicAccess                            );

    bool AutoCreateMultiTopicAccess( const TTopicConfigPtrToStringSetMap& topicsToCreate ,
                                     PubSubClientTopicSet& topicAccess                   );

    void AutoDestroyTopicAccess( const CORE::CString::StringSet& topicNames );
    
    virtual void DestroyTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters = CORE::CString::EmptyStringSet ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList ) GUCEF_VIRTUAL_OVERRIDE;
    
    virtual void GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Connect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Disconnect( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsHealthy( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetType( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& tree ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& treeroot ) GUCEF_VIRTUAL_OVERRIDE;

    PUBSUB::CPubSubClientConfig& GetConfig( void );

    CORE::ThreadPoolPtr GetThreadPool( void );
    
    const RedisNodeMap& GetRedisNodeMap( void ) const;

    RedisClusterPtr GetRedisContext( void ) const;

    protected:

    virtual bool Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    bool GetRedisClusterNodeMap( RedisNodeMap& nodeMap );

    bool GetAllGlobPatternTopicNames( CORE::CString::StringSet& allGlobPatternTopicNames );
    
    bool IsStreamIndexingNeeded( void );

    CORE::UInt32 GetRedisClusterErrorRepliesCounter( bool resetCounter );

    const PUBSUB::CPubSubClientTopicConfig* FindTemplateConfigForTopicName( const CORE::CString& topicName ) const;

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void
    OnRedisReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    void
    OnStreamIndexingTimerCycle( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData );

    void
    OnRedisKeyCacheUpdate( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void RegisterEventHandlers( void );

    void RegisterTopicEventHandlers( PUBSUB::CPubSubClientTopic* topic );
    
    CRedisClusterPubSubClient( void ); /**< not implemented */

    private:

    typedef CORE::CTEventHandlerFunctor< CRedisClusterPubSubClient > TEventCallback;
    typedef std::map< CORE::CString, CRedisClusterPubSubClientTopic* > TTopicMap;

    PUBSUB::CPubSubClientConfig m_config;
    RedisNodeMap m_nodeMap;
    RedisClusterPtr m_redisContext;
    CORE::UInt32 m_redisErrorReplies;
    CORE::CTimer* m_metricsTimer;
    CORE::CTimer* m_redisReconnectTimer;
    CORE::CTimer* m_streamIndexingTimer;
    TTopicMap m_topicMap;
    CORE::ThreadPoolPtr m_threadPool;
    mutable bool m_isHealthy;
    MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace REDISCLUSTER */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_REDISCLUSTER_CREDISCLUSTERPUBSUBCLIENT_H ? */
