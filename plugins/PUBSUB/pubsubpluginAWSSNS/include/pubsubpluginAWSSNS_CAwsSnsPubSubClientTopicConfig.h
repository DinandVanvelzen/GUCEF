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

#ifndef PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPICCONFIG_H
#define PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPICCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_AWSSNS_MACROS_H
#include "pubsubpluginAWSSNS_macros.h"
#define PUBSUBPLUGIN_AWSSNS_MACROS_H
#endif /* PUBSUBPLUGIN_AWSSNS_MACROS_H ? */

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

class CAwsSnsPubSubClient;

/**
 *  Standard pub-sub client config with some custom specifcs added for this
 *  specific pub-sub backend
 */
class PUBSUBPLUGIN_AWSSNS_PLUGIN_PRIVATE_CPP CAwsSnsPubSubClientTopicConfig : public PUBSUB::CPubSubClientTopicConfig ,
                                                                              public CORE::CTSharedObjCreator< CAwsSnsPubSubClientTopicConfig, MT::CMutex >
{
    public:

    
    CAwsSnsPubSubClientTopicConfig( void );
    
    CAwsSnsPubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig );

    CAwsSnsPubSubClientTopicConfig( const CAwsSnsPubSubClientTopicConfig& src );

    virtual ~CAwsSnsPubSubClientTopicConfig() GUCEF_VIRTUAL_OVERRIDE;

    CAwsSnsPubSubClientTopicConfig& operator=( const PUBSUB::CPubSubClientTopicConfig& src );

    CAwsSnsPubSubClientTopicConfig& operator=( const CAwsSnsPubSubClientTopicConfig& src );

    bool LoadCustomConfig( const CORE::CDataNode& config );

    bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src );

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    bool createTopicIfMissing;
    bool tryToUsePublishBatch;
    bool addPrefixWhenSendingKvPairs;
    CORE::CAsciiString kvPairPrefixToAddOnSend;
    bool addPrefixWhenSendingMetaDataKvPairs;
    CORE::CAsciiString metaDatakvPairPrefixToAddOnSend;
};

/*-------------------------------------------------------------------------*/

typedef CAwsSnsPubSubClientTopicConfig::TSharedPtrType     CAwsSnsPubSubClientTopicConfigPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AWSSNS */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_AWSSNS_CAWSSNSPUBSUBCLIENTTOPICCONFIG_H ? */
