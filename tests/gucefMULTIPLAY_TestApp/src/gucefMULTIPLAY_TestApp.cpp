/*
 *  gucefMULTIPLAY_TestApp: Application for testing gucefMULTIPLAY
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

#define GUCEF_TESTFW_IMPL

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#include "gucefCORE_CPlatformNativeConsoleLogger.h"
#define GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H
#endif /* GUCEF_CORE_CPLATFORMNATIVECONSOLELOGGER_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_MULTIPLAY_H
#include "gucefMULTIPLAY.h"
#define GUCEF_MULTIPLAY_H
#endif /* GUCEF_MULTIPLAY_H ? */

#ifndef GUCEF_MULTIPLAY_TESTAPP_TESTPASSIVEURLSERVERLISTPROVIDER_H
#include "TestPassiveUrlServerListProvider.h"
#include "TestServerList.h"
#include "TestListings.h"
#include "TestLobby.h"
#include "TestIdentity.h"
#include "TestAuthIdentity.h"
#include "TestIrcChat.h"
#include "TestGucefLobby.h"
#include "TestMatchModel.h"
#include "TestMatchDecisions.h"
#include "TestMatchDraft.h"
#include "TestMatchContainer.h"
#include "TestMatchBook.h"
#include "TestMatchAlerts.h"
#include "TestAsyncLobby.h"
#define GUCEF_MULTIPLAY_TESTAPP_TESTPASSIVEURLSERVERLISTPROVIDER_H
#endif /* GUCEF_MULTIPLAY_TESTAPP_TESTPASSIVEURLSERVERLISTPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    try
    {
        // Initialize systems
        CORE::CCoreGlobal::Instance();
        COMCORE::CComCoreGlobal::Instance();

        // DRN links are encrypted by default: x25519 and the ciphers come from codecspluginMONOCYPHER
        {
            #ifdef GUCEF_DEBUG_MODE
            const char* cryptoPlugins[ 2 ] = { "$MODULEDIR$/codecspluginMONOCYPHER_d", "$MODULEDIR$/codecspluginMONOCYPHER" };
            #else
            const char* cryptoPlugins[ 2 ] = { "$MODULEDIR$/codecspluginMONOCYPHER", "$MODULEDIR$/codecspluginMONOCYPHER_d" };
            #endif
            CORE::CPluginControl& pluginControl = CORE::CCoreGlobal::Instance()->GetPluginControl();
            if ( !pluginControl.AddPluginFromDir( cryptoPlugins[ 0 ], "test", true ) )
                pluginControl.AddPluginFromDir( cryptoPlugins[ 1 ], "test", true );
        }

        // As a child process of the tests the test app can be a lobby server and nothing else
        if ( argc > 2 && 0 == strcmp( argv[ 1 ], GUCEF_MULTIPLAY_TESTAPP_LOBBYSERVER_ONLY_ARG ) )
            return RunLobbyServerOnly( argv[ 2 ] );
        SetTestAppExecutable( argv[ 0 ] );

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefMULTIPLAY_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefMULTIPLAY_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        PerformPassiveUrlServerListProviderTests();
        PerformServerListTests();
        PerformListingTests();
        PerformLobbyTests();
        PerformIdentityTests();
        PerformAuthIdentityTests();
        PerformIrcChatTests();
        PerformGucefLobbyTests();
        PerformGucefLobbyAuthTests();
        PerformMatchModelTests();
        PerformMatchDecisionTests();
        PerformMatchDraftTests();
        PerformMatchContainerTests();
        PerformMatchBookTests();
        PerformMatchAlertTests();
        PerformAsyncLobbyTests();

        // The factories of the plugins the tests loaded go away with them, before the global systems do
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "test" );

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefMULTIPLAY_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 10-12-2006
        - Dinand: Added this comment section
- 2026
        - Ported to the reporting test framework, with passive url server list provider tests

---------------------------------------------------------------------------*/
