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

#ifndef GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H
#include "gucefPUBSUB_CPubSubFlowRouter.h"
#define GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H
#endif /* GUCEF_PUBSUB_CPUBSUBFLOWROUTER_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "MockPubSubClient.h"
#include "TestPubSubClientSide.h"

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
PerformPubSubClientSideTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING CPubSubClientSide TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "CPubSubClientSide" );

    // Test 1: Construction with just a side ID
    GUCEF_TESTFW_TESTCASE( "Test 1: CPubSubClientSide construction with sideId" )
        try
        {
            PUBSUB::CPubSubClientSide side( "testSide" );
            ASSERT_TRUE( side.GetSideId() == "testSide" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: GetSideId returns the ID given at construction
    GUCEF_TESTFW_TESTCASE( "Test 2: GetSideId matches construction ID" )
        try
        {
            PUBSUB::CPubSubClientSide side( "myUniqueSideId" );
            ASSERT_TRUE( side.GetSideId() == "myUniqueSideId" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 3: GetCurrentUnderlyingPubSubClient returns null before setup
    GUCEF_TESTFW_TESTCASE( "Test 3: GetCurrentUnderlyingPubSubClient null before setup" )
        try
        {
            PUBSUB::CPubSubClientSide side( "testSide" );
            PUBSUB::CPubSubClientPtr client = side.GetCurrentUnderlyingPubSubClient();
            ASSERT_TRUE( client.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 4: IsConnected returns false before setup
    GUCEF_TESTFW_TESTCASE( "Test 4: IsConnected is false before any setup" )
        try
        {
            PUBSUB::CPubSubClientSide side( "testSide" );
            ASSERT_FALSE( side.IsConnected() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 5: LoadConfig with minimal valid settings
    GUCEF_TESTFW_TESTCASE( "Test 5: LoadConfig with minimal valid settings" )
        try
        {
            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK";

            bool loaded = side.LoadConfig( settings );
            ASSERT_TRUE( loaded );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 6: GetSideSettings returns settings after LoadConfig
    GUCEF_TESTFW_TESTCASE( "Test 6: GetSideSettings returns loaded settings" )
        try
        {
            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK";

            side.LoadConfig( settings );

            PUBSUB::CPubSubSideChannelSettingsPtr retrieved = side.GetSideSettings();
            ASSERT_FALSE( retrieved.IsNULL() );
            ASSERT_TRUE( retrieved->pubsubClientConfig.pubsubClientType == "MOCK" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 7: PerformPubSubClientSetup creates the mock client when registered
    GUCEF_TESTFW_TESTCASE( "Test 7: PerformPubSubClientSetup creates mock client" )
        try
        {
            PUBSUB::CMockPubSubClient::RegisterInFactory();

            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK";
            side.LoadConfig( settings );

            bool setupOk = side.PerformPubSubClientSetup( false );
            ASSERT_TRUE( setupOk );

            PUBSUB::CPubSubClientPtr client = side.GetCurrentUnderlyingPubSubClient();
            ASSERT_FALSE( client.IsNULL() );

            if ( !client.IsNULL() )
            {
                ASSERT_TRUE( client->GetType() == "MOCK" );
            }

            PUBSUB::CMockPubSubClient::UnregisterFromFactory();
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 8: PerformPubSubClientSetup with unregistered type fails gracefully
    GUCEF_TESTFW_TESTCASE( "Test 8: PerformPubSubClientSetup fails gracefully for unknown type" )
        try
        {
            // Make sure mock is NOT registered
            PUBSUB::CMockPubSubClient::UnregisterFromFactory();

            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK_UNREGISTERED_TYPE";
            side.LoadConfig( settings );

            // Setup should fail if type is not registered
            bool setupOk = side.PerformPubSubClientSetup( false );
            ASSERT_FALSE( setupOk );

            PUBSUB::CPubSubClientPtr client = side.GetCurrentUnderlyingPubSubClient();
            ASSERT_TRUE( client.IsNULL() );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 9: IsHealthy after successful setup with mock
    GUCEF_TESTFW_TESTCASE( "Test 9: IsHealthy after PerformPubSubClientSetup with mock" )
        try
        {
            PUBSUB::CMockPubSubClient::RegisterInFactory();

            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK";
            side.LoadConfig( settings );

            bool setupOk = side.PerformPubSubClientSetup( false );
            ASSERT_TRUE( setupOk );

            // The mock client reports healthy, so the side should be healthy too
            ASSERT_TRUE( side.IsHealthy() );

            PUBSUB::CMockPubSubClient::UnregisterFromFactory();
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 10: GetPubSubClientSupportedFeatures after setup
    GUCEF_TESTFW_TESTCASE( "Test 10: GetPubSubClientSupportedFeatures after setup" )
        try
        {
            PUBSUB::CMockPubSubClient::RegisterInFactory();

            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK";
            side.LoadConfig( settings );
            side.PerformPubSubClientSetup( false );

            PUBSUB::CPubSubClientFeatures features;
            bool gotFeatures = side.GetPubSubClientSupportedFeatures( features );
            ASSERT_TRUE( gotFeatures );
            // Mock declares publishing and subscribing support
            ASSERT_TRUE( features.supportsPublishing );
            ASSERT_TRUE( features.supportsSubscribing );

            PUBSUB::CMockPubSubClient::UnregisterFromFactory();
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 11: PerformPubSubClientSetup with hardReset=true
    GUCEF_TESTFW_TESTCASE( "Test 11: PerformPubSubClientSetup with hardReset=true" )
        try
        {
            PUBSUB::CMockPubSubClient::RegisterInFactory();

            PUBSUB::CPubSubClientSide side( "testSide" );

            PUBSUB::CPubSubSideChannelSettingsPtr settings = PUBSUB::CPubSubSideChannelSettings::CreateSharedObj();
            settings->pubsubClientConfig.pubsubClientType = "MOCK";
            side.LoadConfig( settings );

            ASSERT_TRUE( side.PerformPubSubClientSetup( false ) );
            // Hard reset should also succeed
            ASSERT_TRUE( side.PerformPubSubClientSetup( true ) );

            PUBSUB::CPubSubClientPtr client = side.GetCurrentUnderlyingPubSubClient();
            ASSERT_FALSE( client.IsNULL() );

            PUBSUB::CMockPubSubClient::UnregisterFromFactory();
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 12: Construction with flowRouter pointer
    GUCEF_TESTFW_TESTCASE( "Test 12: CPubSubClientSide construction with flowRouter pointer" )
        try
        {
            PUBSUB::CPubSubFlowRouter router;
            PUBSUB::CPubSubClientSide side( "testSide", &router );
            ASSERT_TRUE( side.GetSideId() == "testSide" );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL CPubSubClientSide TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
