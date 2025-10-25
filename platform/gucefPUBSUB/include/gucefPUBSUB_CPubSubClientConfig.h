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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#define GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#include "gucefPUBSUB_CPubSubClientFeatures.h"    
#define GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFEATURES_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef GUCEF_PUBSUB_CIPUBSUBBOOKMARKPERSISTENCE_H
#include "gucefPUBSUB_CIPubSubBookmarkPersistence.h"
#define GUCEF_PUBSUB_CIPUBSUBBOOKMARKPERSISTENCE_H
#endif /* GUCEF_PUBSUB_CIPUBSUBBOOKMARKPERSISTENCE_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBJOURNALCONFIG_H
#include "gucefPUBSUB_CPubSubJournalConfig.h"
#define GUCEF_PUBSUB_CPUBSUBJOURNALCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBJOURNALCONFIG_H ? */

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
 *  Class providing construction configuration options for pub-sub clients
 */
class GUCEF_PUBSUB_EXPORT_CPP CPubSubClientConfig : public CORE::CIConfigurable
{
    public:

    typedef GUCEF::vector< CPubSubClientTopicConfigPtr > TPubSubClientTopicConfigPtrVector;
    typedef COMCORE::CHostAddress::THostAddressVector  THostAddressVector;
    
    // The following are runtime and config time available:

    CORE::CString pubsubClientType;                 /**< the string form type name of the pub-sub client */
    CPubSubClientFeatures desiredFeatures;          /**< desired feature set of the client */ 
    CORE::CDataNode customConfig;                   /**< client type specific configuration, if any */    
    CORE::UInt32 reconnectDelayInMs;                /**< minimal time to wait before attempting a reconnect if the client supports auto reconnects */
    CORE::Int64 maxTotalMsgsInFlight;               /**< desired max nr of messages in transit, -1 is infinite */
    THostAddressVector remoteAddresses;             /**< primary addresses of the remote host(s) if applicable */
    TPubSubClientTopicConfigPtrVector topics;       /**< config related to specific topics you want the client to deal with, explicitly configured per topic */
    CPubSubClientTopicConfigPtr defaultTopicConfig; /**< config used as the default topic config when creating new topic access */
    CORE::CString metricsPrefix;                    /**< metric key prefix to apply if metrics are enabled */
    CORE::CString pubsubIdPrefix;                   /**< unique but consistent prefix for the pubsub instance, can be used for persistance addressability */
    CPubSubJournalConfig journalConfig;             /**< config for the journal to use, if any */   
    
    // The following are runtime available:

    CORE::PulseGeneratorPtr pulseGenerator;                 /**< dedicated pulse generator to use for anything client related that uses a pulse generator directly, if any */
    CORE::PulseGeneratorPtr topicPulseGenerator;            /**< default dedicated pulse generator to use for anything topic related that uses a pulse generator directly, if any */
    TIPubSubBookmarkPersistencePtr bookmarkPersistence;     /**< peristance facility for bookmarks */
    CIPubSubJournalBasicPtr journal;                        /**< journal to use, if any */

    CPubSubClientConfig( void );

    CPubSubClientConfig( const CPubSubClientConfig& src );

    virtual ~CPubSubClientConfig();

    CPubSubClientConfig& operator=( const CPubSubClientConfig& src );

    /**
     *      @param cfg the data node structure where you'd like to serialize the config to
     *      @return wheter storing the config was successfull
     */
    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *      @param cfg pertinent node in the config document from which to load the config
     *      @return success or failure to load all required settings correctly from the given config
     */
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPubSubClientConfig, MT::CMutex > CPubSubClientConfigPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PUBSUB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H ? */

