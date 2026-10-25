/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <signal.h>
#include <string.h>

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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_AUTH_H
#include "gucefAUTH.h"
#define GUCEF_AUTH_H
#endif /* GUCEF_AUTH_H ? */

#include "TestPump.h"
#include "TestAccountRef.h"
#include "TestAuthTypes.h"
#include "TestSettings.h"
#include "TestAccountProfile.h"
#include "TestCredentialStore.h"
#include "TestLinkStore.h"
#include "TestDescriptors.h"
#include "TestAuthManager.h"
#include "TestOpenId2Backend.h"
#include "TestUnavailableProviders.h"
#include "TestJwt.h"
#include "TestOAuth2Protocol.h"
#include "TestOAuth2Flows.h"
#include "TestOAuth2Tls.h"
#include "TestOAuth2Verify.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Whether a group of tests runs: all do, unless the environment variable GUCEF_AUTH_TESTS names some */
static bool
RunGroup( const char* name )
{
    const char* selected = getenv( "GUCEF_AUTH_TESTS" );
    return GUCEF_NULL == selected || '\0' == selected[ 0 ] || GUCEF_NULL != strstr( selected, name );
}

/*-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;

    // A peer that goes away in the middle of a handshake (the tests of certificates that are not trusted) must not end the process
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    signal( SIGPIPE, SIG_IGN );
    #endif

    try
    {
        // Initialize systems
        CORE::CCoreGlobal::Instance();
        COMCORE::CComCoreGlobal::Instance();
        AUTH::CAuthGlobal::Instance();

        // The credential store seals with xchacha20-poly1305 and the links are signed with ed25519: both come from codecspluginMONOCYPHER.
        // The link store and the settings are written as json: dstorepluginJSONPARSER
        LoadTestPlugin( "codecspluginMONOCYPHER" );
        JsonCodecAvailable();

        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefAUTH_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF_TESTFW_INIT( "gucefAUTH_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        // The module has no OpenID 2.0 provider of its own: this is the backend of the tests, registered the way a plugin registers one
        CTestOpenId2Backend openId2Backend;
        SetTestOpenId2Backend( &openId2Backend );

        // GUCEF_AUTH_TESTS holds the names of the groups to run, all of them when it is empty
        if ( RunGroup( "AccountRef" ) )     PerformAccountRefTests();
        if ( RunGroup( "AuthTypes" ) )      PerformAuthTypesTests();
        if ( RunGroup( "Settings" ) )       PerformSettingsTests();
        if ( RunGroup( "AccountProfile" ) ) PerformAccountProfileTests();
        if ( RunGroup( "CredentialStore" ) ) PerformCredentialStoreTests();
        if ( RunGroup( "LinkStore" ) )      PerformLinkStoreTests();
        if ( RunGroup( "Descriptors" ) )    PerformDescriptorTests();
        if ( RunGroup( "AuthManager" ) )    PerformAuthManagerTests();
        if ( RunGroup( "UnavailableProviders" ) ) PerformUnavailableProviderTests();

        // The login engine over https with the native HTTP client backend, which has to be the only one there: before the plugin is loaded again
        if ( RunGroup( "OAuth2Tls" ) )      PerformOAuth2TlsTests();

        // The HTTP client for https is the plugin httpclientpluginCURL, as in the shipped game; the group above has loaded it when it ran
        HttpsBackendAvailable();

        if ( RunGroup( "Jwt" ) )            PerformJwtTests();
        if ( RunGroup( "OAuth2Protocol" ) ) PerformOAuth2ProtocolTests();
        if ( RunGroup( "OAuth2Flows" ) )    PerformOAuth2FlowTests();
        if ( RunGroup( "OAuth2Verify" ) )   PerformOAuth2VerifyTests();

        // The HTTP client of the login engine keeps the backend of the plugin it used: it lets go of it before the plugin goes
        AUTH::CAuthGlobal::Instance()->GetOAuth2Engine()->ReleaseHttpBackends();

        // The threads of the pools end here, not at the end of the process: what a TLS library keeps per thread is released when they do
        CORE::CCoreGlobal::Instance()->GetTaskManager().RequestAllThreadsToStop( true, false, 5000 );
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "https" );

        // The backend of the tests goes the way a plugin takes its backend away
        SetTestOpenId2Backend( GUCEF_NULL );

        // The factories of the plugins the tests loaded go away with them, before the global systems do
        CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "test" );

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefAUTH_TestApp_Results.xml" );
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

/*-------------------------------------------------------------------------*/
