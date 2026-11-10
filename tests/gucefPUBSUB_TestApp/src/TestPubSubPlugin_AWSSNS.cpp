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

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestPubSubPlugin_AWSSNS.h"

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
PerformPubSubPlugin_AWSSNSTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pubsubpluginAWSSNS backend TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "pubsubpluginAWSSNS" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginAWSSNS_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginAWSSNS";
    #else
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginAWSSNS";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginAWSSNS_d";
    #endif

    bool pluginLoaded = pluginControl.AddPluginFromDir( firstPluginPath, "pubsubpluginAWSSNS_tests", true );
    if ( !pluginLoaded )
        pluginLoaded = pluginControl.AddPluginFromDir( secondPluginPath, "pubsubpluginAWSSNS_tests", true );

    if ( !pluginLoaded )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pubsubpluginAWSSNS binary not available, skipping backend tests" );
        return;
    }

    // Test 1: Verify the plugin registers its client type in the PubSub factory
    GUCEF_TESTFW_TESTCASE( "Test 1: Plugin registers \"AwsSns\" client type in PubSubClientFactory" )
        try
        {
            ASSERT_TRUE( pluginLoaded );
            PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
            ASSERT_TRUE( factory.IsConstructible( "AwsSns" ) );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    // Test 2: A client can be created and tells what SNS can do: publish to topics but not pull messages from them
    GUCEF_TESTFW_TESTCASE( "Test 2: Client supports publishing but not subscribing" )
        try
        {
            PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
            PUBSUB::CPubSubClientConfig cfg;
            cfg.pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
            PUBSUB::CPubSubClientPtr client = factory.Create( "AwsSns", cfg );
            ASSERT_FALSE( client.IsNULL() );
            if ( !client.IsNULL() )
            {
                ASSERT_TRUE( client->GetType() == "AwsSns" );

                PUBSUB::CPubSubClientFeatures features;
                ASSERT_TRUE( client->GetSupportedFeatures( features ) );
                ASSERT_TRUE( features.supportsPublishing );
                ASSERT_FALSE( features.supportsSubscribing );
                ASSERT_TRUE( features.supportsDiscoveryOfAvailableTopics );
                ASSERT_TRUE( features.supportsPrimaryPayloadPerMsg );
                ASSERT_FALSE( features.supportsBookmarkingConcept );
            }
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    pluginControl.UnloadPluginGroup( "pubsubpluginAWSSNS_tests" );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginAWSSNS backend TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
