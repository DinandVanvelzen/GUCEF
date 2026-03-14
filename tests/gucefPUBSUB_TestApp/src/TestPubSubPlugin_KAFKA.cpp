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

#include "TestPubSubPlugin_KAFKA.h"

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
PerformPubSubPlugin_KAFKATests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING pubsubpluginKAFKA backend TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "pubsubpluginKAFKA" );

    CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();

    #ifdef GUCEF_DEBUG_MODE
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginKAFKA_d";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginKAFKA";
    #else
    CORE::CString firstPluginPath  = "$MODULEDIR$/pubsubpluginKAFKA";
    CORE::CString secondPluginPath = "$MODULEDIR$/pubsubpluginKAFKA_d";
    #endif

    bool pluginLoaded = pluginControl.AddPluginFromDir( firstPluginPath, "pubsubpluginKAFKA_tests", true );
    if ( !pluginLoaded )
        pluginLoaded = pluginControl.AddPluginFromDir( secondPluginPath, "pubsubpluginKAFKA_tests", true );

    if ( !pluginLoaded )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "pubsubpluginKAFKA binary not available, skipping backend tests" );
        return;
    }

    // Test 1: Verify the plugin registers its client type in the PubSub factory
    GUCEF_TESTFW_TESTCASE( "Test 1: Plugin registers \"Kafka\" client type in PubSubClientFactory" )
        try
        {
            PUBSUB::CPubSubClientFactory& factory = PUBSUB::CPubSubGlobal::Instance()->GetPubSubClientFactory();
            ASSERT_TRUE( factory.IsConstructible( "Kafka" ) );
        }
        catch ( ... ) { ERRORHERE; }
    GUCEF_TESTFW_TESTCASE_END

    pluginControl.UnloadPluginGroup( "pubsubpluginKAFKA_tests" );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL pubsubpluginKAFKA backend TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
