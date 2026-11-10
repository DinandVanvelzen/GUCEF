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

#ifndef PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENT_H
#define PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <aws/core/Aws.h>
#include <aws/sns/SNSClient.h>

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

#ifndef PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPIC_H
#include "pubsubpluginAWSSNS_CAwsSnsPubSubClientTopic.h"
#define PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPIC_H
#endif /* PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPIC_H ? */

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

class PUBSUBPLUGIN_AWSSNS_PLUGIN_PRIVATE_CPP CAwsSnsPubSubClient : public PUBSUB::CPubSubClient
{
    public:

    static const CORE::CString TypeName; 

    typedef std::map< CAwsSnsPubSubClientTopicConfigPtr , CORE::CString::StringSet > TTopicConfigPtrToStringSetMap;

    CAwsSnsPubSubClient( const PUBSUB::CPubSubClientConfig& config );

    virtual ~CAwsSnsPubSubClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetOrCreateTopicConfig( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicConfigPtr GetDefaultTopicConfig( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig                    ,
                                                                  CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    virtual PUBSUB::CPubSubClientTopicBasicPtr GetTopicAccess( const CORE::CString& topicName ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetMultiTopicAccess( const CORE::CString& topicName    ,
                                      PubSubClientTopicSet& topicAccess ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetMultiTopicAccess( const CORE::CString::StringSet& topicNames ,
                                      PubSubClientTopicSet& topicAccess          ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateMultiTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig                    ,
                                         PubSubClientTopicSet& topicAccess                                  ,
                                         CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ) GUCEF_VIRTUAL_OVERRIDE;

    bool AutoCreateMultiTopicAccess( CAwsSnsPubSubClientTopicConfigPtr templateTopicConfig ,
                                     const CORE::CString::StringSet& topicNameList         ,
                                     PubSubClientTopicSet& topicAccess                     ,
                                     CORE::PulseGeneratorPtr pulseGenerator                );

    bool AutoCreateMultiTopicAccess( const TTopicConfigPtrToStringSetMap& topicsToCreate ,
                                     PubSubClientTopicSet& topicAccess                   ,
                                     CORE::PulseGeneratorPtr pulseGenerator              );

    void AutoDestroyTopicAccess( const CORE::CString::StringSet& topicNames );

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

    PUBSUB::CPubSubClientConfig& GetConfig( void );

    Aws::SNS::SNSClient& GetSnsClient( void );

    /**
     *  Helper function to extract the topic name from a topic ARN
     */
    static std::string GetTopicNameFromArn( const std::string& topicArn );

    /**
     *  Resolves a topic name to the ARN of the topic. A topic name that already is an ARN is returned as is.
     *  SNS topics are found by listing them.
     *  @param createIfMissing creates the topic if it does not exist yet (this is a no-op for a topic that does exist)
     *  @return the ARN or an empty string if the topic could not be resolved
     */
    Aws::String GetSnsTopicArnForTopicName( const CORE::CString& topicName ,
                                            bool createIfMissing           );

    bool TryGetTopicAttributes( const std::string& topicArn          ,
                                CORE::CString::StringMap& attributes );

    CAwsSnsPubSubClientTopicConfigPtr FindTemplateConfigForTopicName( const CORE::CString& topicName ) const;

    bool GetAllGlobPatternTopicNames( CORE::CString::StringSet& allGlobPatternTopicNames );

    bool ListAllTopics( const CORE::CString::StringSet& globPatternFilters ,
                        CORE::CString::StringSet& topicNames               );

    bool ListAllTopics( const CORE::CString& globPatternFilter ,
                        CORE::CString::StringSet& topicNames   );
    
    private:

    void
    OnMetricsTimerCycle( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void
    OnTopicIndexingTimerCycle( CORE::CNotifier* notifier    ,
                              const CORE::CEvent& eventId  ,
                              CORE::CICloneable* eventData );

    void
    OnTopicHealthStatusChange( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void RegisterEventHandlers( void );

    void RegisterTopicEventHandlers( CAwsSnsPubSubClientTopicPtr& topic );
    
    CAwsSnsPubSubClient( void ); /**< not implemented */

    private:

    typedef CORE::CTEventHandlerFunctor< CAwsSnsPubSubClient > TEventCallback;
    typedef std::map< CORE::CString, CAwsSnsPubSubClientTopicPtr > TTopicMap;

    PUBSUB::CPubSubClientConfig m_config;
    CORE::CTimer* m_metricsTimer;
    CORE::CTimer* m_topicIndexingTimer;
    TTopicMap m_topicMap;
    Aws::SNS::SNSClient m_snsClient;    
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSNS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENT_H ? */
