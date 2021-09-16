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

#ifndef PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTCONFIG_H
#define PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CPUBSUBCLIENTCONFIG_H
#include "gucefCOMCORE_CPubSubClientConfig.h"
#define GUCEF_COMCORE_CPUBSUBCLIENTCONFIG_H
#endif /* GUCEF_COMCORE_CPUBSUBCLIENTCONFIG_H ? */

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
class PUBSUBPLUGIN_KAFKA_PLUGIN_PRIVATE_CPP CKafkaPubSubClientConfig : public COMCORE::CPubSubClientConfig
{
    public:

    typedef std::map< CORE::CString, CORE::CString >   StringMap;

    StringMap kafkaProducerGlobalConfigSettings;
    StringMap kafkaConsumerGlobalConfigSettings;
    StringMap kafkaConsumerDefaultTopicConfigSettings;
    
    CKafkaPubSubClientConfig( void );
    
    CKafkaPubSubClientConfig( const COMCORE::CPubSubClientConfig& genericConfig );

    virtual ~CKafkaPubSubClientConfig() GUCEF_VIRTUAL_OVERRIDE;

    CKafkaPubSubClientConfig& operator=( const COMCORE::CPubSubClientConfig& src );

    CKafkaPubSubClientConfig& operator=( const CKafkaPubSubClientConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace KAFKA */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_KAFKA_CKAFKAPUBSUBCLIENTCONFIG_H ? */
