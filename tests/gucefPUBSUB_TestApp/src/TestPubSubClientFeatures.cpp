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

#include "TestPubSubClientFeatures.h"

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
PerformPubSubClientFeaturesTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubClientFeatures TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubClientFeatures" );

    // Test 1: Default construction — all flags false
    GUCEF_TESTFW_TESTCASE( "Test 1: Default construction sets all feature flags to false" )
        try
        {
            PUBSUB::CPubSubClientFeatures f;
            ASSERT_FALSE( f.supportsPublishing );
            ASSERT_FALSE( f.supportsSubscribing );
            ASSERT_FALSE( f.supportsSimultaneousPubAndSub );
            ASSERT_FALSE( f.supportsAutoReconnect );
            ASSERT_FALSE( f.supportsSubscriberRateThrottling );
            ASSERT_FALSE( f.supportsSubscriptionMsgArrivalDelayRequests );
            ASSERT_FALSE( f.supportsSubscriptionEndOfDataEvent );
            ASSERT_FALSE( f.supportsBinaryPayloads );
            ASSERT_FALSE( f.supportsMultiHostSharding );
            ASSERT_FALSE( f.supportsPerMsgIds );
            ASSERT_FALSE( f.supportsMsgIndex );
            ASSERT_FALSE( f.supportsPrimaryPayloadPerMsg );
            ASSERT_FALSE( f.supportsAbsentPrimaryPayloadPerMsg );
            ASSERT_FALSE( f.supportsKeyValueSetPerMsg );
            ASSERT_FALSE( f.supportsDuplicateKeysPerMsg );
            ASSERT_FALSE( f.supportsMetaDataKeyValueSetPerMsg );
            ASSERT_FALSE( f.supportsSubscriberMsgReceivedAck );
            ASSERT_FALSE( f.supportsAutoMsgReceivedAck );
            ASSERT_FALSE( f.supportsAbsentMsgReceivedAck );
            ASSERT_FALSE( f.supportsAckUsingLastMsgInBatch );
            ASSERT_FALSE( f.supportsAckUsingBookmark );
            ASSERT_FALSE( f.supportsBookmarkingConcept );
            ASSERT_FALSE( f.supportsSubscribingUsingBookmark );
            ASSERT_FALSE( f.supportsServerSideBookmarkPersistance );
            ASSERT_FALSE( f.supportsAutoBookmarking );
            ASSERT_FALSE( f.supportsMsgIdBasedBookmark );
            ASSERT_FALSE( f.supportsMsgIndexBasedBookmark );
            ASSERT_FALSE( f.supportsTopicIndexBasedBookmark );
            ASSERT_FALSE( f.supportsMsgDateTimeBasedBookmark );
            ASSERT_FALSE( f.supportsDerivingBookmarkFromMsg );
            ASSERT_FALSE( f.supportsDiscoveryOfAvailableTopics );
            ASSERT_FALSE( f.supportsGlobPatternTopicNames );
            ASSERT_FALSE( f.supportsPatternBasedAggregateTopic );
            ASSERT_FALSE( f.supportsMetrics );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: Setting individual flags and reading them back
    GUCEF_TESTFW_TESTCASE( "Test 2: Individual flag set/get" )
        try
        {
            PUBSUB::CPubSubClientFeatures f;
            f.supportsPublishing = true;
            f.supportsSubscribing = true;
            f.supportsBinaryPayloads = true;
            f.supportsBookmarkingConcept = true;
            f.supportsMetrics = true;

            ASSERT_TRUE( f.supportsPublishing );
            ASSERT_TRUE( f.supportsSubscribing );
            ASSERT_TRUE( f.supportsBinaryPayloads );
            ASSERT_TRUE( f.supportsBookmarkingConcept );
            ASSERT_TRUE( f.supportsMetrics );

            // Others should still be false
            ASSERT_FALSE( f.supportsAutoReconnect );
            ASSERT_FALSE( f.supportsPerMsgIds );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: SaveConfig / LoadConfig round-trip
    GUCEF_TESTFW_TESTCASE( "Test 3: SaveConfig/LoadConfig round-trip preserves all flags" )
        try
        {
            PUBSUB::CPubSubClientFeatures original;
            original.supportsPublishing = true;
            original.supportsSubscribing = true;
            original.supportsSimultaneousPubAndSub = true;
            original.supportsBinaryPayloads = true;
            original.supportsPerMsgIds = true;
            original.supportsMsgIndex = true;
            original.supportsPrimaryPayloadPerMsg = true;
            original.supportsKeyValueSetPerMsg = true;
            original.supportsBookmarkingConcept = true;
            original.supportsSubscribingUsingBookmark = true;
            original.supportsMsgIdBasedBookmark = true;
            original.supportsDerivingBookmarkFromMsg = true;
            original.supportsDiscoveryOfAvailableTopics = true;
            original.supportsMetrics = true;

            CORE::CDataNode cfg;
            ASSERT_TRUE( original.SaveConfig( cfg ) );

            PUBSUB::CPubSubClientFeatures loaded;
            ASSERT_TRUE( loaded.LoadConfig( cfg ) );

            ASSERT_TRUE( loaded.supportsPublishing == original.supportsPublishing );
            ASSERT_TRUE( loaded.supportsSubscribing == original.supportsSubscribing );
            ASSERT_TRUE( loaded.supportsSimultaneousPubAndSub == original.supportsSimultaneousPubAndSub );
            ASSERT_TRUE( loaded.supportsBinaryPayloads == original.supportsBinaryPayloads );
            ASSERT_TRUE( loaded.supportsPerMsgIds == original.supportsPerMsgIds );
            ASSERT_TRUE( loaded.supportsMsgIndex == original.supportsMsgIndex );
            ASSERT_TRUE( loaded.supportsBookmarkingConcept == original.supportsBookmarkingConcept );
            ASSERT_TRUE( loaded.supportsSubscribingUsingBookmark == original.supportsSubscribingUsingBookmark );
            ASSERT_TRUE( loaded.supportsMsgIdBasedBookmark == original.supportsMsgIdBasedBookmark );
            ASSERT_TRUE( loaded.supportsDerivingBookmarkFromMsg == original.supportsDerivingBookmarkFromMsg );
            ASSERT_TRUE( loaded.supportsDiscoveryOfAvailableTopics == original.supportsDiscoveryOfAvailableTopics );
            ASSERT_TRUE( loaded.supportsMetrics == original.supportsMetrics );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: GetClassTypeName returns non-empty
    GUCEF_TESTFW_TESTCASE( "Test 4: GetClassTypeName returns non-empty string" )
        try
        {
            PUBSUB::CPubSubClientFeatures f;
            ASSERT_FALSE( f.GetClassTypeName().IsNULLOrEmpty() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubClientFeatures TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
