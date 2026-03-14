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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubClientConfig.h"

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
PerformPubSubClientConfigTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubClientConfig TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubClientConfig" );

    // Test 1: Default construction produces sane state
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction" )
        try
        {
            PUBSUB::CPubSubClientConfig cfg;
            ASSERT_TRUE( cfg.pubsubClientType.IsNULLOrEmpty() );
            ASSERT_TRUE( cfg.reconnectDelayInMs == 0 );
            ASSERT_TRUE( cfg.maxTotalMsgsInFlight == -1 );
            ASSERT_TRUE( cfg.topics.empty() );
            ASSERT_TRUE( cfg.remoteAddresses.empty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: SaveConfig and LoadConfig round-trip for basic fields
    GUCEF_TESTFW_TESTCASE( "Test 2: SaveConfig/LoadConfig round-trip for basic fields" )
        try
        {
            PUBSUB::CPubSubClientConfig cfg;
            cfg.pubsubClientType = "TestBackend";
            cfg.reconnectDelayInMs = 5000;
            cfg.maxTotalMsgsInFlight = 1000;
            cfg.metricsPrefix = "testPrefix";
            cfg.pubsubIdPrefix = "testIdPrefix";

            CORE::CDataNode cfgNode;
            ASSERT_TRUE( cfg.SaveConfig( cfgNode ) );

            PUBSUB::CPubSubClientConfig cfg2;
            ASSERT_TRUE( cfg2.LoadConfig( cfgNode ) );

            ASSERT_TRUE( cfg2.pubsubClientType == cfg.pubsubClientType );
            ASSERT_TRUE( cfg2.reconnectDelayInMs == cfg.reconnectDelayInMs );
            ASSERT_TRUE( cfg2.maxTotalMsgsInFlight == cfg.maxTotalMsgsInFlight );
            ASSERT_TRUE( cfg2.metricsPrefix == cfg.metricsPrefix );
            ASSERT_TRUE( cfg2.pubsubIdPrefix == cfg.pubsubIdPrefix );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: Copy construction preserves all fields
    GUCEF_TESTFW_TESTCASE( "Test 3: Copy construction" )
        try
        {
            PUBSUB::CPubSubClientConfig cfg;
            cfg.pubsubClientType = "CopyTest";
            cfg.reconnectDelayInMs = 3000;
            cfg.maxTotalMsgsInFlight = 500;
            cfg.metricsPrefix = "copyPrefix";

            PUBSUB::CPubSubClientConfig cfg2( cfg );

            ASSERT_TRUE( cfg2.pubsubClientType == cfg.pubsubClientType );
            ASSERT_TRUE( cfg2.reconnectDelayInMs == cfg.reconnectDelayInMs );
            ASSERT_TRUE( cfg2.maxTotalMsgsInFlight == cfg.maxTotalMsgsInFlight );
            ASSERT_TRUE( cfg2.metricsPrefix == cfg.metricsPrefix );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: Assignment operator
    GUCEF_TESTFW_TESTCASE( "Test 4: Assignment operator" )
        try
        {
            PUBSUB::CPubSubClientConfig cfg;
            cfg.pubsubClientType = "AssignTest";
            cfg.reconnectDelayInMs = 1000;
            cfg.maxTotalMsgsInFlight = 200;

            PUBSUB::CPubSubClientConfig cfg2;
            cfg2 = cfg;

            ASSERT_TRUE( cfg2.pubsubClientType == cfg.pubsubClientType );
            ASSERT_TRUE( cfg2.reconnectDelayInMs == cfg.reconnectDelayInMs );
            ASSERT_TRUE( cfg2.maxTotalMsgsInFlight == cfg.maxTotalMsgsInFlight );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: Topic config round-trip
    GUCEF_TESTFW_TESTCASE( "Test 5: Topic config round-trip" )
        try
        {
            PUBSUB::CPubSubClientConfig cfg;
            cfg.pubsubClientType = "TopicTest";

            PUBSUB::CPubSubClientTopicConfigPtr topicCfg( new PUBSUB::CPubSubClientTopicConfig() );
            topicCfg->topicName = "MyTestTopic";
            topicCfg->needSubscribeSupport = true;
            topicCfg->needPublishSupport = false;
            cfg.topics.push_back( topicCfg );

            CORE::CDataNode cfgNode;
            ASSERT_TRUE( cfg.SaveConfig( cfgNode ) );

            PUBSUB::CPubSubClientConfig cfg2;
            ASSERT_TRUE( cfg2.LoadConfig( cfgNode ) );

            ASSERT_TRUE( cfg2.topics.size() == 1 );
            ASSERT_FALSE( cfg2.topics[ 0 ].IsNULL() );
            ASSERT_TRUE( cfg2.topics[ 0 ]->topicName == "MyTestTopic" );
            ASSERT_TRUE( cfg2.topics[ 0 ]->needSubscribeSupport == true );
            ASSERT_TRUE( cfg2.topics[ 0 ]->needPublishSupport == false );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubClientConfig TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
