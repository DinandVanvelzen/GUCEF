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
    CORE::CString::StringVector kafkaMsgValuesUsedForFiltering;
    bool addProducerHostnameAsKafkaMsgHeader;
    CORE::CString prefixToAddForMetaDataKvPairs;
    CORE::CString prefixToAddForKvPairs;
    bool stripPrefixForMetaDataKvPairs;
    bool stripPrefixForKvPairs;
    CORE::Int32 maxKafkaErrorsToBeHealthy;
    
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
