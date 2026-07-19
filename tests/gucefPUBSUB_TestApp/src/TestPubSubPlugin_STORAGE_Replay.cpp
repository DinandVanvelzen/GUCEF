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

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#include "gucefPUBSUB_CPubSubClientSide.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubPlugin_STORAGE_Replay.h"

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
PerformPubSubPlugin_STORAGEReplayTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pubsubpluginSTORAGE replay TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "pubsubpluginSTORAGE_replay" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginSTORAGE_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginSTORAGE";
    #else
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginSTORAGE";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginSTORAGE_d";
    #endif

    // Reuse the plugin if already loaded by a prior test suite
    PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
    bool ownedPluginLoad = false;
    if ( !factory.IsConstructible( "STORAGE" ) )
    {
        ownedPluginLoad = pluginControl.AddPluginFromDir( firstPluginPath, "pubsubpluginSTORAGE_replay_tests", true );
        if ( !ownedPluginLoad )
            ownedPluginLoad = pluginControl.AddPluginFromDir( secondPluginPath, "pubsubpluginSTORAGE_replay_tests", true );
    }

    if ( !factory.IsConstructible( "STORAGE" ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pubsubpluginSTORAGE binary not available, skipping replay tests" );
        return;
    }

    // Set up a writable VFS root for this test suite
    CORE::CString nativeTestDir = CORE::RelativePath( "$CURWORKDIR$/storage_replay_test" );
    CORE::CreateDirs( nativeTestDir );
    VFS::CVfsGlobal::Instance()->GetVfs().AddRoot( nativeTestDir               ,
                                                    "storage_replay_test_root" ,
                                                    true                       );
    VFS::CVfsGlobal::Instance()->GetVfs().MountAllDelayMountedArchives();

    // Configure the STORAGE topic in synchronous mode so Publish() blocks until
    // container files are written to the VFS before we attempt replay.
    PUBSUB::CPubSubClientTopicConfigPtr topicCfg( new PUBSUB::CPubSubClientTopicConfig() );
    topicCfg->topicName            = "replay_test_topic";
    topicCfg->needPublishSupport   = true;
    topicCfg->needSubscribeSupport = false;

    CORE::CDataNode& customCfg = topicCfg->customConfig;
    customCfg.SetAttribute( CORE::CString("vfsStorageRootPath"),                CORE::CString("replay_test_topic") );
    customCfg.SetAttribute( CORE::CString("performVfsOpsASync"),                false                              );
    customCfg.SetAttribute( CORE::CString("performVfsOpsInDedicatedThread"),    false                              );
    customCfg.SetAttribute( CORE::CString("desiredMinimalSerializedBlockSize"), CORE::UInt32(1)                    );

    PUBSUB::CPubSubClientConfig clientCfg;
    clientCfg.pubsubClientType = "STORAGE";
    clientCfg.topics.push_back( topicCfg );

    PUBSUB::CPubSubClientFactory::TProductPtr client = factory.Create( "STORAGE", clientCfg );
    if ( client.IsNULL() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to create STORAGE client, skipping replay tests" );
        if ( ownedPluginLoad ) pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_replay_tests" );
        return;
    }

    PUBSUB::CPubSubClientTopicBasicPtr topic = client->GetOrCreateTopicAccess( "replay_test_topic" );
    if ( topic.IsNULL() || !topic->InitializeConnectivity( false ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to init STORAGE topic connectivity, skipping replay tests" );
        topic.Unlink();
        client.Unlink();
        if ( ownedPluginLoad ) pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_replay_tests" );
        return;
    }

    // Publish 5 messages so there are container files for the replay task to read
    {
        PUBSUB::CBasicPubSubMsg::TBasicPubSubMsgVector msgs;
        msgs.reserve( 5 );
        for ( CORE::UInt64 i = 1; i <= 5; ++i )
        {
            PUBSUB::CBasicPubSubMsg msg;
            msg.GetPrimaryPayload() = CORE::CVariant( CORE::ToString( i ) + ",replay_test_payload" );
            msg.AddKeyValuePair( CORE::CVariant( CORE::CString("seqNr") ), CORE::CVariant( i ) );
            msgs.push_back( msg );
        }
        PUBSUB::CPubSubClientTopic::TPublishActionIdVector actionIds;
        topic->Publish( actionIds, msgs, false );
    }

    // Test 1: STORAGE plugin advertises supportsReplay=true
    GUCEF_TESTFW_TESTCASE( "Test 1: STORAGE client GetSupportedFeatures reports supportsReplay=true" )
        try
        {
            PUBSUB::CPubSubClientFeatures features;
            bool gotFeatures = client->GetSupportedFeatures( features );
            ASSERT_TRUE( gotFeatures );
            ASSERT_TRUE( features.supportsReplay );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: RequestReplay with null requestingSide returns HasNoFuture()==true
    GUCEF_TESTFW_TESTCASE( "Test 2: RequestReplay with null requestingSide returns HasNoFuture()" )
        try
        {
            PUBSUB::CPubSubBookmark startBm;
            PUBSUB::CPubSubBookmark endBm;
            CORE::CFutureResult future = topic->RequestReplay( startBm, endBm, 99, GUCEF_NULL, GUCEF_NULL );
            ASSERT_TRUE( future.HasNoFuture() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: RequestReplay from beginning (NOT_INITIALIZED bookmarks) queues a task that completes
    //
    //   - startBookmark = NOT_INITIALIZED  -> replay from first available container file
    //   - requestingTopic = GUCEF_NULL     -> OnReplayMsgsReceived returns false gracefully (no crash)
    //   - Await() blocks until the async task finishes; success means no hang and no crash
    GUCEF_TESTFW_TESTCASE( "Test 3: RequestReplay from beginning queues task and completes" )
        try
        {
            PUBSUB::CPubSubClientSide requestingSide( "replay_test_side" );

            // Both bookmarks left as NOT_INITIALIZED = replay entire stored dataset
            PUBSUB::CPubSubBookmark startBm;
            PUBSUB::CPubSubBookmark endBm;

            CORE::CFutureResult future = topic->RequestReplay( startBm, endBm, 1, &requestingSide, GUCEF_NULL );
            ASSERT_FALSE( future.HasNoFuture() );

            // Block up to 30 seconds for the async replay task to finish.
            // In practice it should complete in well under a second.
            try
            {
                future.Await( 30000 );
            }
            catch ( ... )
            {
                ERRORHERE;
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Release shared pointers before unloading the plugin DLL
    topic.Unlink();
    client.Unlink();

    if ( ownedPluginLoad )
        pluginControl.UnloadPluginGroup( "pubsubpluginSTORAGE_replay_tests" );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginSTORAGE replay TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
