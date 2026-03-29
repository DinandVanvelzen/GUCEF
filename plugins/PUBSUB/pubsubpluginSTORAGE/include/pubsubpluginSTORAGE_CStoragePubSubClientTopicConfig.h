/*
 *  pubsubpluginSTORAGE: Generic GUCEF plugin for PUBSUB module functionality 
 *                       providing a storage interface
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

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPICCONFIG_H
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPICCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <vector>

#ifndef GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgContainerBinarySerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGCONTAINERBINARYSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBMSGSERIALIZER_H
#include "gucefPUBSUB_CPubSubMsgSerializer.h"
#define GUCEF_PUBSUB_CPUBSUBMSGSERIALIZER_H
#endif /* GUCEF_PUBSUB_CPUBSUBMSGSERIALIZER_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#include "pubsubpluginSTORAGE_CStoragePubSubIndexDef.h"
#define PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H
#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBINDEXDEF_H ? */

#ifndef PUBSUBPLUGIN_STORAGE_MACROS_H
#include "pubsubpluginSTORAGE_macros.h"
#define PUBSUBPLUGIN_STORAGE_MACROS_H
#endif /* PUBSUBPLUGIN_STORAGE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace STORAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CStoragePubSubClient;

/**
 *  Standard pub-sub client config with some custom specifcs added for this
 *  specific pub-sub backend
 */
class PUBSUBPLUGIN_STORAGE_PLUGIN_PRIVATE_CPP CStoragePubSubClientTopicConfig : public PUBSUB::CPubSubClientTopicConfig ,
                                                                                public CORE::CTSharedObjCreator< CStoragePubSubClientTopicConfig, MT::CMutex >
{
    public:

    typedef GUCEF::vector< PUBSUB::CPubSubClientTopicConfig, gucef_allocator< PUBSUB::CPubSubClientTopicConfig > > TTopicConfigVector;
    typedef GUCEF::vector< CStoragePubSubIndexDef, gucef_allocator< CStoragePubSubIndexDef > >                    TIndexDefVector;

    PUBSUB::CPubSubMsgBinarySerializerOptions pubsubBinarySerializerOptions;
    PUBSUB::CPubSubMsgSerializerOptions pubsubSerializerOptions;
    CORE::UInt32 desiredNrOfBuffers;
    CORE::UInt32 desiredMinimalSerializedBlockSize;
    CORE::UInt32 desiredMaxTimeToWaitToGrowSerializedBlockSizeInMs;    
    CORE::CString vfsStorageRootPath;
    CORE::CString vfsFileExtention;
    CORE::CString encodeCodecFamily;
    CORE::CString encodeCodecName;
    CORE::CString decodeCodecFamily;
    CORE::CString decodeCodecName;
    CORE::UInt32 ticketRefillOnBusyCycle;
    bool performVfsOpsASync;
    bool performVfsOpsInDedicatedThread;
    bool applyDedicatedVfsOpsThreadCpuAffinity;
    CORE::UInt32 cpuAffinityForDedicatedVfsOpsThread;
    bool collectMetrics;
    bool subscribeWithoutBookmarkIfNoneIsPersisted;
    bool autoPushAfterStartupIfStorageToPubSub;
    bool autoPushAfterStartupIsPersistent;
    CORE::CDateTime youngestStoragePubSubMsgFileToLoad;
    CORE::CDateTime oldestStoragePubSubMsgFileToLoad;
    CORE::Float32 defaultCodecDecodeGrowthRatioExpectation;
    bool bestEffortDeserializeIsAllowed;
    CORE::Int32 maxStorageCorruptionDetectionsToBeHealthy;
    CORE::Int32 maxStorageDeserializationFailuresToBeHealthy;
    bool treatEveryFullfilledRequestAsEODEvent;
    bool deleteContainersWithFullyAckdContent;
    bool moveContainersWithFullyAckdContent;
    CORE::CString vfsStorageRootPathForFullyAckdContainers;
    CORE::UInt32 maxCompletedContainerRefsToRetain;
    CORE::UInt32 nonAckdMsgCheckIntervalInMs;
    CORE::UInt32 maxTimeToWaitForAllMsgBatchAcksInMs;
    bool topicFollowsDirRenames;                                        /**< if true, the topic will follow the dir rename, if false, the topic will remain on the old path and thus functionally be disconnected */
    TIndexDefVector indexDefinitions;                                   /**< optional sidecar index definitions; one .sidx file per entry */

    CStoragePubSubClientTopicConfig( void );

    CStoragePubSubClientTopicConfig( const CStoragePubSubClientTopicConfig& src );
    
    CStoragePubSubClientTopicConfig( const PUBSUB::CPubSubClientTopicConfig& genericConfig );

    virtual ~CStoragePubSubClientTopicConfig() GUCEF_VIRTUAL_OVERRIDE;

    CStoragePubSubClientTopicConfig& operator=( const PUBSUB::CPubSubClientTopicConfig& src );

    CStoragePubSubClientTopicConfig& operator=( const CStoragePubSubClientTopicConfig& src );

    bool LoadConfig( const PUBSUB::CPubSubClientTopicConfig& src );

    virtual bool LoadCustomConfig( const CORE::CDataNode& config );

    virtual bool SaveCustomConfig( CORE::CDataNode& config ) const;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------*/

typedef CStoragePubSubClientTopicConfig::TSharedPtrType     CStoragePubSubClientTopicConfigPtr;     

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace STORAGE */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* PUBSUBPLUGIN_STORAGE_CSTORAGEPUBSUBCLIENTTOPICCONFIG_H ? */
