/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CCONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CCONFIGSTORE_H
#endif /* GUCEF_CORE_CCONFIGSTORE_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_COM_CSTATSDCLIENT_H
#include "gucefCOM_CStatsDClient.h"
#define GUCEF_COM_CSTATSDCLIENT_H
#endif /* GUCEF_COM_CSTATSDCLIENT_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestStatsDClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformStatsDClientTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "StatsDClient" );

    GUCEF_TESTFW_TESTCASE( "StatsDClient: A config without a StatsDClient section is a config with nothing for it, not a failed one" )
    try
    {
        COM::CStatsDClient client;
        CORE::CDataNode config( "Config" );
        config.AddChild( "SomethingElse" )->SetAttribute( "value", "1" );
        ASSERT_TRUE( client.LoadConfig( config ) );
        ASSERT_TRUE( client.LoadConfig( CORE::CDataNode() ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StatsDClient: A StatsDClient section without a destination is refused" )
    try
    {
        COM::CStatsDClient client;
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.AddChild( "StatsDClient" );
        section->SetAttribute( "transmit", "false" );
        ASSERT_FALSE( client.LoadConfig( config ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StatsDClient: A StatsDClient section with a destination is taken" )
    try
    {
        COM::CStatsDClient client;
        CORE::CDataNode config( "Config" );
        CORE::CDataNode* section = config.AddChild( "StatsDClient" );
        section->SetAttribute( "statsDestination", "127.0.0.1:8125" );
        section->SetAttribute( "transmit", "false" );
        section->SetAttribute( "statsNamePrefix", "gucefcomtest." );
        ASSERT_TRUE( client.LoadConfig( config ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "StatsDClient: The global config loads for an application that has no StatsDClient section, as the client of the module is there" )
    try
    {
        // The client the module makes for itself is a global configurable: its being there must not fail the load of a config
        // that has nothing for it
        CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();
        const CORE::CString previousBootstrap = configStore.GetBootstrapConfigFile();
        const CORE::CString::StringVector previousConfigFiles = configStore.GetConfigFiles();

        const CORE::CString path = CORE::CombinePath( CORE::RelativePath( "$TEMPDIR$" ), "gucefCOM_TestApp_StatsD.ini" );
        ASSERT_TRUE( CORE::WriteStringAsTextFile( path, "[Other]\n\"value\"=\"1\"\n" ) );
        configStore.SetBootstrapConfigFile( CORE::CString::Empty );
        configStore.SetConfigFile( path, false );

        const bool loaded = configStore.LoadConfig();

        configStore.SetConfigFiles( previousConfigFiles );
        configStore.SetBootstrapConfigFile( previousBootstrap );
        CORE::DeleteFile( path );
        ASSERT_TRUE( loaded );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
