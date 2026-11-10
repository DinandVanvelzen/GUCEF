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

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H
#include "gucefPUBSUB_CPubSubFlowRouterConfig.h"
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H
#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTERCONFIG_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubFlowRouterConfig.h"

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
PerformPubSubFlowRouterConfigTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubFlowRouterConfig TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubFlowRouterConfig" );

    // Test 1: Default construction
    GUCEF_TESTFW_TESTCASE( "Test 1: CPubSubFlowRouterConfig default construction" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;
            ASSERT_TRUE( config.ackStyle == PUBSUB::CPubSubFlowRouterConfig::AllOrNothing );
            ASSERT_TRUE( config.routes.empty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: AckStyle string round-trip for AllOrNothing
    GUCEF_TESTFW_TESTCASE( "Test 2: AckStyle AllOrNothing string round-trip" )
        try
        {
            CORE::CString s = PUBSUB::CPubSubFlowRouterConfig::AckStyleToString( PUBSUB::CPubSubFlowRouterConfig::AllOrNothing );
            ASSERT_FALSE( s.IsNULLOrEmpty() );
            PUBSUB::CPubSubFlowRouterConfig::AckStyle ackStyle2 = PUBSUB::CPubSubFlowRouterConfig::StringToAckStyle( s );
            ASSERT_TRUE( ackStyle2 == PUBSUB::CPubSubFlowRouterConfig::AllOrNothing );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: AckStyle string round-trip for FireAndForget
    GUCEF_TESTFW_TESTCASE( "Test 3: AckStyle FireAndForget string round-trip" )
        try
        {
            CORE::CString s = PUBSUB::CPubSubFlowRouterConfig::AckStyleToString( PUBSUB::CPubSubFlowRouterConfig::FireAndForget );
            ASSERT_FALSE( s.IsNULLOrEmpty() );
            PUBSUB::CPubSubFlowRouterConfig::AckStyle ackStyle2 = PUBSUB::CPubSubFlowRouterConfig::StringToAckStyle( s );
            ASSERT_TRUE( ackStyle2 == PUBSUB::CPubSubFlowRouterConfig::FireAndForget );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: AckStyle string round-trip for AnyOneForAll
    GUCEF_TESTFW_TESTCASE( "Test 4: AckStyle AnyOneForAll string round-trip" )
        try
        {
            CORE::CString s = PUBSUB::CPubSubFlowRouterConfig::AckStyleToString( PUBSUB::CPubSubFlowRouterConfig::AnyOneForAll );
            ASSERT_FALSE( s.IsNULLOrEmpty() );
            PUBSUB::CPubSubFlowRouterConfig::AckStyle ackStyle2 = PUBSUB::CPubSubFlowRouterConfig::StringToAckStyle( s );
            ASSERT_TRUE( ackStyle2 == PUBSUB::CPubSubFlowRouterConfig::AnyOneForAll );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: FindRouteWithFromSideAndToSide returns null on empty config
    GUCEF_TESTFW_TESTCASE( "Test 5: FindRouteWithFromSideAndToSide returns null when no routes" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;
            PUBSUB::CPubSubFlowRouteConfigPtr found = config.FindRouteWithFromSideAndToSide( "sideA", "sideB" );
            ASSERT_TRUE( found.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: FindRouteWithFromSideAndToSide finds existing route
    GUCEF_TESTFW_TESTCASE( "Test 6: FindRouteWithFromSideAndToSide finds matching route" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;

            PUBSUB::CPubSubFlowRouteConfigPtr routeConfig = PUBSUB::CPubSubFlowRouteConfig::CreateSharedObj();
            routeConfig->fromSideId = "sideA";
            routeConfig->toSideId   = "sideB";
            config.routes.push_back( routeConfig );

            PUBSUB::CPubSubFlowRouteConfigPtr found = config.FindRouteWithFromSideAndToSide( "sideA", "sideB" );
            ASSERT_FALSE( found.IsNULL() );
            ASSERT_TRUE( found->fromSideId == "sideA" );
            ASSERT_TRUE( found->toSideId   == "sideB" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: FindRouteWithFromSideAndToSide returns null for non-matching route
    GUCEF_TESTFW_TESTCASE( "Test 7: FindRouteWithFromSideAndToSide returns null for non-match" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;

            PUBSUB::CPubSubFlowRouteConfigPtr routeConfig = PUBSUB::CPubSubFlowRouteConfig::CreateSharedObj();
            routeConfig->fromSideId = "sideA";
            routeConfig->toSideId   = "sideB";
            config.routes.push_back( routeConfig );

            PUBSUB::CPubSubFlowRouteConfigPtr found = config.FindRouteWithFromSideAndToSide( "sideX", "sideY" );
            ASSERT_TRUE( found.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: CPubSubFlowRouterConfig SaveConfig/LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "Test 8: CPubSubFlowRouterConfig SaveConfig/LoadConfig round-trip" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;
            config.ackStyle = PUBSUB::CPubSubFlowRouterConfig::FireAndForget;
            config.minBadHealthDurationBeforeFailoverInMs = 1000;
            config.routeSwitchingTimerIntervalInMs        = 500;

            PUBSUB::CPubSubFlowRouteConfigPtr routeConfig = PUBSUB::CPubSubFlowRouteConfig::CreateSharedObj();
            routeConfig->fromSideId = "sideA";
            routeConfig->toSideId   = "sideB";
            config.routes.push_back( routeConfig );

            CORE::CDataNode cfgNode;
            ASSERT_TRUE( config.SaveConfig( cfgNode ) );

            PUBSUB::CPubSubFlowRouterConfig config2;
            ASSERT_TRUE( config2.LoadConfig( cfgNode ) );

            ASSERT_TRUE( config2.ackStyle == PUBSUB::CPubSubFlowRouterConfig::FireAndForget );
            ASSERT_TRUE( config2.minBadHealthDurationBeforeFailoverInMs == 1000 );
            ASSERT_TRUE( config2.routeSwitchingTimerIntervalInMs        == 500 );
            ASSERT_FALSE( config2.routes.empty() );
            ASSERT_TRUE( config2.routes[ 0 ]->fromSideId == "sideA" );
            ASSERT_TRUE( config2.routes[ 0 ]->toSideId   == "sideB" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: CPubSubFlowRouterConfig copy construction
    GUCEF_TESTFW_TESTCASE( "Test 9: CPubSubFlowRouterConfig copy construction" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;
            config.ackStyle = PUBSUB::CPubSubFlowRouterConfig::AnyOneForAll;
            config.minBadHealthDurationBeforeSpilloverInMs = 2000;

            PUBSUB::CPubSubFlowRouteConfigPtr routeConfig = PUBSUB::CPubSubFlowRouteConfig::CreateSharedObj();
            routeConfig->fromSideId = "s1";
            routeConfig->toSideId   = "s2";
            config.routes.push_back( routeConfig );

            PUBSUB::CPubSubFlowRouterConfig config2( config );
            ASSERT_TRUE( config2.ackStyle == PUBSUB::CPubSubFlowRouterConfig::AnyOneForAll );
            ASSERT_TRUE( config2.minBadHealthDurationBeforeSpilloverInMs == 2000 );
            ASSERT_TRUE( config2.routes.size() == 1 );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: CPubSubFlowRouterConfig Clear
    GUCEF_TESTFW_TESTCASE( "Test 10: CPubSubFlowRouterConfig Clear removes routes" )
        try
        {
            PUBSUB::CPubSubFlowRouterConfig config;
            config.ackStyle = PUBSUB::CPubSubFlowRouterConfig::FireAndForget;

            PUBSUB::CPubSubFlowRouteConfigPtr routeConfig = PUBSUB::CPubSubFlowRouteConfig::CreateSharedObj();
            routeConfig->fromSideId = "sideA";
            routeConfig->toSideId   = "sideB";
            config.routes.push_back( routeConfig );

            ASSERT_FALSE( config.routes.empty() );
            config.Clear();
            ASSERT_TRUE( config.routes.empty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: CPubSubFlowRouteConfig default construction
    GUCEF_TESTFW_TESTCASE( "Test 11: CPubSubFlowRouteConfig default construction" )
        try
        {
            PUBSUB::CPubSubFlowRouteConfig routeConfig;
            ASSERT_TRUE( routeConfig.fromSideId.IsNULLOrEmpty() );
            ASSERT_TRUE( routeConfig.toSideId.IsNULLOrEmpty() );
            ASSERT_TRUE( routeConfig.failoverSideId.IsNULLOrEmpty() );
            ASSERT_TRUE( routeConfig.topicAssociations.empty() );
            ASSERT_TRUE( routeConfig.persistenceSideId.IsNULLOrEmpty() );
            ASSERT_FALSE( routeConfig.persistenceSideTopicsAutoMatchFromSide );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: CPubSubFlowRouteConfig SaveConfig/LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "Test 12: CPubSubFlowRouteConfig SaveConfig/LoadConfig round-trip" )
        try
        {
            PUBSUB::CPubSubFlowRouteConfig routeConfig;
            routeConfig.fromSideId     = "sideFrom";
            routeConfig.toSideId       = "sideTo";
            routeConfig.failoverSideId = "sideFailover";
            routeConfig.persistenceSideId = "sidePersistence";
            routeConfig.toSideTopicsAutoMatchFromSide           = true;
            routeConfig.failoverSideTopicsAutoMatchFromSide     = false;
            routeConfig.persistenceSideTopicsAutoMatchFromSide  = true;

            CORE::CDataNode cfgNode;
            ASSERT_TRUE( routeConfig.SaveConfig( cfgNode ) );

            PUBSUB::CPubSubFlowRouteConfig routeConfig2;
            ASSERT_TRUE( routeConfig2.LoadConfig( cfgNode ) );

            ASSERT_TRUE( routeConfig2.fromSideId      == "sideFrom" );
            ASSERT_TRUE( routeConfig2.toSideId        == "sideTo" );
            ASSERT_TRUE( routeConfig2.failoverSideId  == "sideFailover" );
            ASSERT_TRUE( routeConfig2.persistenceSideId == "sidePersistence" );
            ASSERT_TRUE( routeConfig2.toSideTopicsAutoMatchFromSide          == true );
            ASSERT_TRUE( routeConfig2.failoverSideTopicsAutoMatchFromSide    == false );
            ASSERT_TRUE( routeConfig2.persistenceSideTopicsAutoMatchFromSide == true );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 13: CPubSubFlowRouteConfig IsAnyAutoTopicMatchingNeeded
    GUCEF_TESTFW_TESTCASE( "Test 13: CPubSubFlowRouteConfig IsAnyAutoTopicMatchingNeeded" )
        try
        {
            PUBSUB::CPubSubFlowRouteConfig routeConfig;

            // All false by default — no auto matching needed
            routeConfig.toSideTopicsAutoMatchFromSide         = false;
            routeConfig.failoverSideTopicsAutoMatchFromSide   = false;
            routeConfig.spilloverSideTopicsAutoMatchFromSide  = false;
            routeConfig.deadLetterSideTopicsAutoMatchFromSide = false;
            routeConfig.persistenceSideTopicsAutoMatchFromSide = false;
            ASSERT_FALSE( routeConfig.IsAnyAutoTopicMatchingNeeded() );

            // Enable to-side — auto matching needed
            routeConfig.toSideTopicsAutoMatchFromSide = true;
            ASSERT_TRUE( routeConfig.IsAnyAutoTopicMatchingNeeded() );

            // persistence side alone also triggers it
            PUBSUB::CPubSubFlowRouteConfig persistenceConfig;
            persistenceConfig.persistenceSideTopicsAutoMatchFromSide = true;
            ASSERT_TRUE( persistenceConfig.IsAnyAutoTopicMatchingNeeded() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 14: CPubSubFlowRouteTopicConfig default construction
    GUCEF_TESTFW_TESTCASE( "Test 14: CPubSubFlowRouteTopicConfig default construction" )
        try
        {
            PUBSUB::CPubSubFlowRouteTopicConfig topicConfig;
            ASSERT_TRUE( topicConfig.fromSideTopicName.IsNULLOrEmpty() );
            ASSERT_TRUE( topicConfig.toSideTopicName.IsNULLOrEmpty() );
            ASSERT_TRUE( topicConfig.failoverSideTopicName.IsNULLOrEmpty() );
            ASSERT_TRUE( topicConfig.spilloverSideTopicName.IsNULLOrEmpty() );
            ASSERT_TRUE( topicConfig.deadLetterSideTopicName.IsNULLOrEmpty() );
            ASSERT_TRUE( topicConfig.persistenceSideTopicName.IsNULLOrEmpty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 15: CPubSubFlowRouteTopicConfig SaveConfig/LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "Test 15: CPubSubFlowRouteTopicConfig SaveConfig/LoadConfig round-trip" )
        try
        {
            PUBSUB::CPubSubFlowRouteTopicConfig topicConfig;
            topicConfig.fromSideTopicName       = "topicFrom";
            topicConfig.toSideTopicName         = "topicTo";
            topicConfig.failoverSideTopicName   = "topicFailover";
            topicConfig.spilloverSideTopicName  = "topicSpillover";
            topicConfig.deadLetterSideTopicName = "topicDeadLetter";
            topicConfig.persistenceSideTopicName = "topicPersistence";

            CORE::CDataNode cfgNode;
            ASSERT_TRUE( topicConfig.SaveConfig( cfgNode ) );

            PUBSUB::CPubSubFlowRouteTopicConfig topicConfig2;
            ASSERT_TRUE( topicConfig2.LoadConfig( cfgNode ) );

            ASSERT_TRUE( topicConfig2.fromSideTopicName       == "topicFrom" );
            ASSERT_TRUE( topicConfig2.toSideTopicName         == "topicTo" );
            ASSERT_TRUE( topicConfig2.failoverSideTopicName   == "topicFailover" );
            ASSERT_TRUE( topicConfig2.spilloverSideTopicName  == "topicSpillover" );
            ASSERT_TRUE( topicConfig2.deadLetterSideTopicName == "topicDeadLetter" );
            ASSERT_TRUE( topicConfig2.persistenceSideTopicName == "topicPersistence" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 16: CPubSubFlowRouteConfig FindTopicAssociation not found
    GUCEF_TESTFW_TESTCASE( "Test 16: CPubSubFlowRouteConfig FindTopicAssociation not found" )
        try
        {
            PUBSUB::CPubSubFlowRouteConfig routeConfig;
            PUBSUB::CPubSubFlowRouteTopicConfigPtr found = routeConfig.FindTopicAssociation( "nonExistentTopic", false );
            ASSERT_TRUE( found.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 17: CPubSubFlowRouteConfig FindOrCreateTopicAssociation creates entry
    GUCEF_TESTFW_TESTCASE( "Test 17: CPubSubFlowRouteConfig FindOrCreateTopicAssociation creates entry" )
        try
        {
            PUBSUB::CPubSubFlowRouteConfig routeConfig;
            ASSERT_TRUE( routeConfig.topicAssociations.empty() );

            // FindOrCreateTopicAssociation only creates entries when auto-topic-matching is configured
            // At least one *AutoMatchFromSide flag must be true for IsAnyAutoTopicMatchingNeeded() to return true
            routeConfig.toSideTopicsAutoMatchFromSide = true;

            PUBSUB::CPubSubFlowRouteTopicConfigPtr created = routeConfig.FindOrCreateTopicAssociation( "myTopic", false );
            ASSERT_FALSE( created.IsNULL() );
            ASSERT_FALSE( routeConfig.topicAssociations.empty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubFlowRouterConfig TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
