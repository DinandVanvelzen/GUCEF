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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENT_H
#define GUCEF_PUBSUB_CPUBSUBCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CITYPENAMED_H
#include "CITypeNamed.h"
#define GUCEF_CORE_CITYPENAMED_H
#endif /* GUCEF_CORE_CITYPENAMED_H ? */

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#include "gucefPUBSUB_CPubSubClientFeatures.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#include "gucefPUBSUB_CPubSubClientTopic.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPIC_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#include "gucefPUBSUB_CPubSubClientConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H ? */

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
 
/**
 *  Base class for all pub-sub client implementations
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubClient : public CORE::CTSGNotifier           ,
                                              public virtual CORE::CIConfigurable
{
    public:

    static const CORE::CEvent TopicAccessCreatedEvent;                 /**< occurs when topic access is created programatically or via loading of config requesting the same */
    static const CORE::CEvent TopicAccessDestroyedEvent;               /**< occurs when topic access is removed. You should clean up any outstanding references */
    static const CORE::CEvent TopicsAccessAutoCreatedEvent;            /**< occurs when topic access is automatically created following discovery. Requires the 'supportsDiscoveryOfAvailableTopics' feature to be supported and enabled */
    static const CORE::CEvent TopicsAccessAutoDestroyedEvent;          /**< occurs when topic access is automatically destroyed following discovery. Requires the 'supportsDiscoveryOfAvailableTopics' feature to be supported and enabled */
    static const CORE::CEvent TopicDiscoveryEvent;                     /**< occurs when available topic changes are detected. Requires the 'supportsDiscoveryOfAvailableTopics' feature to be supported and enabled */
    static const CORE::CEvent TopicDiscoveryEndedEvent;                /**< backend signaling that as far as its concerned the discovery of available topics has ended and you should not expect more TopicDiscoveryEvent events. Requires the 'supportsDiscoveryOfAvailableTopics' feature to be supported and enabled */
    static const CORE::CEvent HealthStatusChangeEvent;                 /**< event msg sent if the health status changes for the topic */

    typedef GUCEF::set< CPubSubClientTopicBasicPtr >                  PubSubClientTopicSet;
    typedef CORE::TCloneableString                                  TopicAccessCreatedEventData;            /**< name of the topic is passed as event relevant data */
    typedef CORE::TCloneableString                                  TopicAccessDestroyedEventData;          /**< name of the topic is passed as event relevant data */
    typedef CORE::CTCloneableObj< PubSubClientTopicSet >            TopicsAccessAutoCreatedEventData;       /**< access to the topics is passed as event relevant data */
    typedef CORE::CTCloneableObj< PubSubClientTopicSet >            TopicsAccessAutoDestroyedEventData;     /**< access to the topics is passed as event relevant data */
    typedef CORE::CTCloneableExpansion< CORE::CString::StringSet >  TopicDiscoveryEventData;                /**< list of discovered topics */
    typedef CORE::TCloneableBool                                    THealthStatusChangeEventData;           /**< boolean flag indicating the health status */

    CPubSubClient( void );
    
    CPubSubClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    CPubSubClient( const CPubSubClient& src );

    virtual ~CPubSubClient();

    CPubSubClient& operator=( const CPubSubClient& src );

    virtual bool GetSupportedFeatures( CPubSubClientFeatures& features ) const = 0;

    virtual CPubSubClientTopicBasicPtr CreateTopicAccess( CPubSubClientTopicConfigPtr topicConfig                            ,
                                                          CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() ) = 0;

    /**
     *  Same as the version that takes an entire config except the expectation here is that the topic      
     *  is already configured via a CPubSubClientConfig but not yet instantiated
     *  This would be the typical case when using global app config defined topics and not programatic topic access
     */ 
    virtual CPubSubClientTopicBasicPtr CreateTopicAccess( const CString& topicName                                           ,
                                                          CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual CPubSubClientTopicBasicPtr GetTopicAccess( const CString& topicName ) = 0;

    virtual CPubSubClientTopicBasicPtr GetOrCreateTopicAccess( const CString& topicName                                           ,
                                                               CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual bool GetMultiTopicAccess( CPubSubClientTopicConfigPtr topicConfig ,
                                      PubSubClientTopicSet& topicAccess       );

    virtual bool GetMultiTopicAccess( const CString& topicName          ,
                                      PubSubClientTopicSet& topicAccess );

    virtual bool GetMultiTopicAccess( const CString::StringSet& topicNames ,
                                      PubSubClientTopicSet& topicAccess    );
    
    virtual bool GetOrCreateMultiTopicAccess( CPubSubClientTopicConfigPtr topicConfig                            ,
                                              PubSubClientTopicSet& topicAccess                                  ,
                                              CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual bool GetOrCreateMultiTopicAccess( const CString& topicName                                           ,
                                              PubSubClientTopicSet& topicAccess                                  ,
                                              CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    virtual bool CreateMultiTopicAccess( CPubSubClientTopicConfigPtr topicConfig                            ,
                                         PubSubClientTopicSet& topicAccess                                  ,
                                         CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    /**
     *  Same as the version that takes an entire config except the expectation here is that the topic      
     *  is already configured via a CPubSubClientConfig but not yet instantiated
     *  This would be the typical case when using global app config defined topics and not programatic topic access
     */ 
    virtual bool CreateMultiTopicAccess( const CString& topicName                                           ,
                                         PubSubClientTopicSet& topicAccess                                  ,
                                         CORE::PulseGeneratorPtr pulseGenerator = CORE::PulseGeneratorPtr() );

    /**
     *  Attempts to obtain from the backend the default config for any topic
     */
    virtual CPubSubClientTopicConfigPtr GetDefaultTopicConfig( void ) = 0;

    /**
     *  Attempts to obtain from the backend the config associated with a given topic, if any
     */
    virtual CPubSubClientTopicConfigPtr GetTopicConfig( const CString& topicName ) = 0;

    /**
     *  Attempts to obtain from the backend the config associated with a given topic
     *  If no specific config exists for the given topic a new one will be created using the default topic config as the template
     */
    virtual CPubSubClientTopicConfigPtr GetOrCreateTopicConfig( const CString& topicName ) = 0;

    /**
     *      @param cfg the data node structure where you'd like to serialize the config to
     *      @return wheter storing the config was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& cfg ) const;

    /**
     *      @param cfg the config storage object where you'd like to serialize the config to
     *      @return wheter storing the config was successfull
     */
    virtual bool SaveConfig( CPubSubClientConfig& cfg ) const = 0;

    /**
     *      @param cfg pertinent node in the config document from which to load the config
     *      @return success or failure to load all required settings correctly from the given config
     */
    virtual bool LoadConfig( const CORE::CDataNode& cfg );

    /**
     *      @param cfg the config storage object from which to load the config
     *      @return success or failure to load all required settings correctly from the given config
     */
    virtual bool LoadConfig( const CPubSubClientConfig& cfg ) = 0;

    /**
     *  Attempts to get a list of topics available on the backend regardless of whichever topics are known
     *  due to configuration. Thus for some backends this allows one to 'discover' new topics
     */
    virtual bool BeginTopicDiscovery( const CORE::CString::StringSet& globPatternFilters = CORE::CString::EmptyStringSet );
    
    virtual void GetConfiguredTopicNameList( CString::StringSet& topicNameList ) = 0;

    virtual void GetCreatedTopicAccessNameList( CString::StringSet& topicNameList ) = 0;

    virtual void GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess ) = 0;
    
    virtual void GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess ) const;

    virtual void DestroyTopicAccess( const CString& topicName ) = 0;

    virtual bool Connect( bool reset ) = 0;

    virtual bool Disconnect( void ) = 0;

    /**
     *  Returns whether connectivty is established as applicable for the given backend
     *  It also validates that if the relevant topics are intended for subscriptions purposes that 
     *  an active subscription has been established.
     */
    virtual bool IsConnectedAndSubscribedAsNeeded( void ) const;

    /**
     *  Returns whether connectivty is established as applicable for the given backend
     *  Note that being connected does not imply being subscribed which is logically distinct.
     */
    virtual bool IsConnected( void ) const = 0;

    /**
     *  Returns whether everything is working as far as the code is contextually aware
     *  If anything of any kind is doing recovery, such as a reconnect timer cycle, that would cause
     *  the healthy state to be 'false'
     */
    virtual bool IsHealthy( void ) const = 0;

    /**
     *  Obtains a snapshot-in-time evaluation of the initialization completeness of the 
     *  persistence backend used. Given the abstractions one cannot assume that merely instantiating the
     *  client equals full initialization. There may be dependencies at play which need to be satisfied async.
     */
    virtual bool IsInitialized( void ) const = 0;

    /**
     *  Provides a snapshot-in-time state of whether as an aggregate status for all topics the subscriptions have 
     *  reached a point where no more data is known to be available. Not all backends will support this as it is 
     *  tied with the 'supportsSubscriptionEndOfDataEvent' feature toggle.
     *  If the feature is supported but we are not subscribed the return value is to be 'false'
     *
     *  Default implementation acts as a helper function using the other member functions available to perform the aggregation 
     *  of the snapshot-in-time state. A more efficient version could be implemented by backends if so desired or if a more
     *  transactional/atomic state determination is required.
     */
    virtual bool AreAllSubscriptionsAtEndOfData( void ) const;

    /**
     *  Provide ability to get the textual name of the client type, typically the name under which its registered
     */
    virtual const CString& GetType( void ) const = 0;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    void SetOpaqueUserData( void* opaqueUserData );
    
    void* GetOpaqueUserData( void ) const;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                    bool includeTopics                        );

    /**
     *  Sets the pulse generator for both the default and all currently existing topics
     *  overriding and this decoupling any existing pulse generator said topic might have been using
     *  Use with care as this potentially alters the overall threading model.
     */
    virtual void SetTopicPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator );

    /**
     *  Sets the default pulse generator for newly created topics going forward if no pulseGenerator was specified
     *  at the time of topic creation. 
     *  Use with care as this potentially alters the overall threading model.
     */
    virtual bool SetDefaultTopicPulseGenerator( CORE::PulseGeneratorPtr defaultPulseGenerator );

    /**
     *  Gets the default pulse generator which is set to be used for newly created topics if no pulseGenerator was specified
     *  at the time of topic creation. 
     */
    virtual CORE::PulseGeneratorPtr GetDefaultTopicPulseGenerator( void ) const;

    /**
     *  Sets the journal implementation (if any) to be used by the client
     */
    virtual bool SetJournal( CIPubSubJournalBasicPtr journal );

    virtual CIPubSubJournalBasicPtr GetJournal( void ) const;

    static void RegisterEvents( void );

    protected:

    virtual bool ConfigureJournal( CPubSubClientConfig& clientConfig );
    
    virtual bool ConfigureJournal( CPubSubClientConfigPtr clientConfig );

    virtual bool ConfigureJournal( CPubSubClientTopicBasicPtr topic        ,
                                   CPubSubClientTopicConfigPtr topicConfig );
    
    private:

    void* m_opaqueUserData;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTBasicSharedPtr< CPubSubClient, MT::CMutex > CPubSubClientPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBCLIENT_H ? */

