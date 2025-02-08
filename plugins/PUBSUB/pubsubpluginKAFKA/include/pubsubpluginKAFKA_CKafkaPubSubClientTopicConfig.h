/*
 *  pubsubpluginKAFKA: Generic GUCEF COMCORE plugin for providing pubsub via Kafka
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

#ifndef PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPICCONFIG_H
#define PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPICCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_KAFKA_MACROS_H
#include "pubsubpluginKAFKA_macros.h"
#define PUBSUBPLUGIN_KAFKA_MACROS_H
#endif /* PUBSUBPLUGIN_KAFKA_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace KAFKA {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CKafkaPubSubClient;

/**
 *  Standard pub-sub client config with some custom specifcs added for this
 *  specific pub-sub backend
 */
class PUBSUBPLUGIN_KAFKA_PLUGIN_PRIVATE_CPP CKafkaPubSubClientTopicConfig : public PUBSUB::CPubSubClientTopicConfig ,
                                                                            public CORE::CTSharedObjCreator< CKafkaPubSubClientTopicConfig, MT::CMutex >
{
    public:

    typedef CORE::CString::StringMap   StringMap;
    
    StringMap kafkaProducerTopicConfigSettings;
    StringMap kafkaConsumerTopicConfigSettings;
    CORE::CString consumerModeStartOffset;
    bool useKafkaMsgHeadersForConsumerFiltering;
    std::string kafkaMsgHeaderUsedForFiltering;
    CORE::CString::StringVector kafkaMsgValuesUsedForFiltering;    /**< The values that are used for filtering out messages received from Kafka. They will right away count as successfully handled */
    bool addProducerHostnameAsKafkaMsgHeader;                      /**< If enabled the hostname of the producer will be added as a kafka message header */
    CORE::CString prefixToAddForMetaDataKvPairs;
    CORE::CString prefixToAddForKvPairs;
    bool stripPrefixForMetaDataKvPairs;                            /**< If enabled the prefix will be stripped from the meta-data key value pairs before they are added to the message */
    bool stripPrefixForKvPairs;                                    /**< If enabled the prefix will be stripped from the key value pairs before they are added to the message */
    CORE::Int32 maxKafkaErrorsToBeHealthy;                         /**< The maximum number of errors that can occur before the client is considered unhealthy */
    bool addReadyToTransmitTimeStampAsKafkaMetaDataMsgHeader;      /**< If enabled the time at which the message was ready to be transmitted will be added as metadata to the message */
    std::string readyToTransmitTimeStampMetaDataKey;               /**< The key to use for the ready to transmit timestamp metadata if enabled via addReadyToTransmitTimeStampAsKafkaMetaDataMsgHeader */
    
    CKafkaPubSubClientTopicConfig( void );
    
    CKafkaPubSubClientTopicConfig( const CKafkaPubSubClientTopicConfig& src );

    CKafkaPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig );

    virtual ~CKafkaPubSubClientTopicConfig() GUCEF_VIRTUAL_OVERRIDE;

    CKafkaPubSubClientTopicConfig& operator=( const PUBSUB::CPubSubClientTopicConfig& src );

    CKafkaPubSubClientTopicConfig& operator=( const CKafkaPubSubClientTopicConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CKafkaPubSubClientTopicConfig::TSharedPtrType     CKafkaPubSubClientTopicConfigPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAFKA */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTTOPICCONFIG_H ? */
