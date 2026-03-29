/*
 *  gucefPUBSUB_TestApp: GUCEF test application for the PUBSUB module
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_PUBSUB_H
#include "gucefPUBSUB.h"
#define GUCEF_PUBSUB_H
#endif /* GUCEF_PUBSUB_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#include "gucefPUBSUB_CPubSubClientFactory.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTFACTORY_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#include "gucefPUBSUB_CPubSubClientConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTCONFIG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#include "gucefPUBSUB_CPubSubClientTopicConfig.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTTOPICCONFIG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubPlugin_STORAGE_Index.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPubSubPlugin_STORAGEIndexTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pubsubpluginSTORAGE index TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "pubsubpluginSTORAGE_index" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginSTORAGE_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginSTORAGE";
    #else
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginSTORAGE";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginSTORAGE_d";
    #endif

    // If a prior test suite already loaded and kept the STORAGE plugin resident
    // (unload sometimes silently fails), loading it again would call Load() twice
    // on the same DLL and crash. Reuse the already-registered factory type instead.
    PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
    bool ownedPluginLoad = false;
    if ( !factory.IsConstructible( "STORAGE" ) )
    {
        ownedPluginLoad = pluginControl.AddPluginFromDir( firstPluginPath, "pubsubpluginSTORAGE_index_tests", true );
        if ( !ownedPluginLoad )
            ownedPluginLoad = pluginControl.AddPluginFromDir( secondPluginPath, "pubsubpluginSTORAGE_index_tests", true );
    }
    bool pluginAvailable = factory.IsConstructible( "STORAGE" );

    if ( !pluginAvailable )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pubsubpluginSTORAGE binary not available, skipping index tests" );
        return;
    }

    // Test 1: Verify the plugin registers its client type in the PubSub factory
    GUCEF_TESTFW_TESTCASE( "Test 1: Plugin registers \"STORAGE\" client type in PubSubClientFactory" )
        try
        {
            ASSERT_TRUE( factory.IsConstructible( "STORAGE" ) );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Set up a writable VFS root for this test
    CORE::CString nativeTestDir = CORE::RelativePath( "$CURWORKDIR$/storage_index_test" );
    CORE::CreateDirs( nativeTestDir );
    VFS::CVfsGlobal::Instance()->GetVfs().AddRoot( nativeTestDir              ,
                                                    "storage_index_test_root" ,
                                                    true                      );
    // Without a global config load, the VFS never fires VfsInitializationCompletedEvent
    // which the STORAGE topic uses to set m_vfsInitIsComplete. Calling
    // MountAllDelayMountedArchives() sets m_delayedArchiveMountingIsComplete = true so
    // that IsInitialized() returns true when InitializeConnectivity() is called below.
    VFS::CVfsGlobal::Instance()->GetVfs().MountAllDelayMountedArchives();

    // Configure the topic via generic CPubSubClientTopicConfig.
    // STORAGE-specific settings are passed through customConfig so the plugin
    // can read them via LoadCustomConfig(); no backend headers are needed here.
    //
    //   performVfsOpsASync=false + desiredMinimalSerializedBlockSize=1
    //   forces each message to be flushed to its own container file
    //   synchronously inside Publish(), so on return all files are on disk.
    //
    //   The "index" child node tells the STORAGE topic to build a .sidx
    //   sidecar index keyed on the "seqNr" key-value pair (type su64).
    PUBSUB::CPubSubClientTopicConfigPtr topicCfg( new PUBSUB::CPubSubClientTopicConfig() );
    topicCfg->topicName            = "seqnr_topic";
    topicCfg->needPublishSupport   = true;
    topicCfg->needSubscribeSupport = false;

    CORE::CDataNode& customCfg = topicCfg->customConfig;
    customCfg.SetAttribute( CORE::CString("vfsStorageRootPath"),                CORE::CString("seqnr_topic") );
    customCfg.SetAttribute( CORE::CString("performVfsOpsASync"),                false                        );
    customCfg.SetAttribute( CORE::CString("performVfsOpsInDedicatedThread"),    false                        );
    customCfg.SetAttribute( CORE::CString("desiredMinimalSerializedBlockSize"), CORE::UInt32(1)              );

    CORE::CDataNode* indexNode = customCfg.AddChild( "index" );
    if ( GUCEF_NULL != indexNode )
    {
        indexNode->SetAttribute( "keySource",        CORE::CString("k")     );
        indexNode->SetAttribute( "keyName",          CORE::CString("seqNr") );
        indexNode->SetAttribute( "indexType",        CORE::CString("su64")  );
        indexNode->SetAttribute( "samplingInterval", CORE::UInt32(1)        );
    }

    // Create the STORAGE client through the generic factory interface
    PUBSUB::CPubSubClientConfig clientCfg;
    clientCfg.pubsubClientType = "STORAGE";
    clientCfg.topics.push_back( topicCfg );

    PUBSUB::CPubSubClientFactory::TProductPtr client = factory.Create( "STORAGE", clientCfg );

    // Test 2: Client was created
    GUCEF_TESTFW_TESTCASE( "Test 2: Create STORAGE client via factory" )
        try
        {
            ASSERT_FALSE( client.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    if ( client.IsNULL() )
    {
        if ( ownedPluginLoad ) pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_index_tests" );
        return;
    }

    // Obtain topic access through the generic CPubSubClient interface
    PUBSUB::CPubSubClientTopicBasicPtr topic = client->GetOrCreateTopicAccess( "seqnr_topic" );

    // Test 3: Topic access was obtained
    GUCEF_TESTFW_TESTCASE( "Test 3: GetOrCreateTopicAccess returns seqnr_topic" )
        try
        {
            ASSERT_FALSE( topic.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    if ( topic.IsNULL() )
    {
        if ( ownedPluginLoad ) pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_index_tests" );
        return;
    }

    bool connected = topic->InitializeConnectivity( false );

    // Test 4: Connectivity initialized
    GUCEF_TESTFW_TESTCASE( "Test 4: InitializeConnectivity succeeds for seqnr_topic" )
        try
        {
            ASSERT_TRUE( connected );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    if ( !connected )
    {
        if ( ownedPluginLoad ) pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_index_tests" );
        return;
    }

    // Publish 300 messages through the generic CPubSubClientTopic::Publish() interface.
    // Each message carries a "seqNr" key-value pair (UInt64) and a simple text payload.
    PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs;
    msgs.reserve( 300 );
    for ( CORE::UInt64 seqNr = 1; seqNr <= 300; ++seqNr )
    {
        PUBSUB::CBasicPubSubMsg msg;
        msg.GetPrimaryPayload() = CORE::CVariant( CORE::ToString( seqNr ) + ",test_payload" );
        msg.AddKeyValuePair( CORE::CVariant( CORE::CString("seqNr") ), CORE::CVariant( seqNr ) );
        msgs.push_back( msg );
    }

    PUBSUB::CPubSubClientTopic::TPublishActionIdVector actionIds;
    bool published = topic->Publish( actionIds, msgs, false );

    // Test 5: Publish succeeds
    GUCEF_TESTFW_TESTCASE( "Test 5: Publish 300 messages via STORAGE topic" )
        try
        {
            ASSERT_TRUE( published );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: The .sidx index file was written to VFS by the STORAGE backend
    GUCEF_TESTFW_TESTCASE( "Test 6: .sidx index file written to VFS after publish" )
        try
        {
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            // Filename format: idx.<keySource>.<keyName>.<typeCode>.sidx
            bool sidxExists = vfs.FileExists( "seqnr_topic/idx.k.seqNr.su64.sidx" );
            ASSERT_TRUE( sidxExists );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: Validate .sidx binary format via raw VFS read.
    //   Header magic  : "GUCEFIDX" at byte offset 0 (8 bytes)
    //   Footer layout : last 20 bytes =
    //       UInt32 index_entry_count
    //       UInt32 index_entries_offset
    //       UInt32 file_registry_offset
    //       UInt32 file_registry_count
    //       UInt32 footer_magic  (0x47494458)
    GUCEF_TESTFW_TESTCASE( "Test 7: .sidx binary has valid header magic, footer magic, and entry count" )
        try
        {
            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
            CORE::CDynamicBuffer sidxBuf;
            bool loaded = vfs.LoadFile( sidxBuf, "seqnr_topic/idx.k.seqNr.su64.sidx" );
            ASSERT_TRUE( loaded );

            // Minimum size: 8 (magic) + 1 (version) + 4*2 (type+flags) + more
            bool bufferIsExpectedSize = sidxBuf.GetDataSize() >= 28u;
            ASSERT_TRUE( bufferIsExpectedSize );
            if ( !bufferIsExpectedSize )
                return;

            const CORE::UInt8* data = static_cast< const CORE::UInt8* >( sidxBuf.GetConstBufferPtr() );

            // Header magic "GUCEFIDX"
            ASSERT_TRUE( data[0] == 'G' && data[1] == 'U' && data[2] == 'C' && data[3] == 'E' );
            ASSERT_TRUE( data[4] == 'F' && data[5] == 'I' && data[6] == 'D' && data[7] == 'X' );

            CORE::UInt32 fileSize = sidxBuf.GetDataSize();

            // Footer magic at last 4 bytes
            CORE::UInt32 footerMagic = 0;
            ::memcpy( &footerMagic, data + fileSize - 4, sizeof( CORE::UInt32 ) );
            ASSERT_TRUE( footerMagic == 0x47494458u );

            // index_entry_count is at footer offset 0 (20 bytes from end)
            CORE::UInt32 indexEntryCount = 0;
            ::memcpy( &indexEntryCount, data + fileSize - 20, sizeof( CORE::UInt32 ) );
            ASSERT_TRUE( indexEntryCount == 300u );

            // file_registry_count is at footer offset 12 (8 bytes from end, before magic)
            CORE::UInt32 fileRegistryCount = 0;
            ::memcpy( &fileRegistryCount, data + fileSize - 8, sizeof( CORE::UInt32 ) );
            ASSERT_TRUE( fileRegistryCount >= 1u );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    if ( ownedPluginLoad )
        pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_index_tests" );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginSTORAGE index TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
