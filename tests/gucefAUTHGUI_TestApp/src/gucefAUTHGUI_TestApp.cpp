/*
 *  gucefAUTHGUI_TestApp: Application for testing gucefAUTHGUI
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

/* the test framework is implemented in this source, before anything includes its header */
#define GUCEF_TESTFW_IMPL

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_AUTHGUI_H
#include "gucefAUTHGUI.h"
#define GUCEF_AUTHGUI_H
#endif /* GUCEF_AUTHGUI_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#include "gucefAUTHGUI_TestCommon.h"
#include "TestOpenId2Backend.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        GUCEF::CORE::CString logFilename = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( logFilename, "gucefAUTHGUI_TestApp_Log.txt" );
        GUCEF::CORE::CFileAccess logFileAccess( logFilename, "w" );
        GUCEF::CORE::CStdLogger logger( logFileAccess );
        GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        GUCEF::CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            GUCEF::CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        GUCEF::AUTHGUI::CAuthGuiGlobal::Instance();

        // The module has no OpenID 2.0 provider of its own: this is the backend of the tests, registered the way a plugin registers one
        CTestOpenId2Backend openId2Backend;
        SetTestOpenId2Backend( &openId2Backend );

        GUCEF_TESTFW_INIT( "gucefAUTHGUI_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( GUCEF::TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        GUCEF::CORE::CString xmlPath = GUCEF::CORE::RelativePath( "$CURWORKDIR$" );
        GUCEF::CORE::AppendToPath( xmlPath, "gucefAUTHGUI_TestApp_Results.xml" );

        // the profile key of a login needs the signature codec
        LoadTestPlugin( "codecspluginMONOCYPHER", "codecs" );

        // without an HTTP client for https the login of the OpenID 2.0 backend is shown disabled, with the reason
        PerformAccountOfferNoHttpsTests();

        // the HTTP client for https which the game has: the logins that need it are only offered with it. Not unloaded by the tests, see HttpsBackendAvailable() of the AUTH tests
        LoadTestPlugin( "httpclientpluginCURL", "https" );

        PerformAuthGuiGlobalTests();
        PerformLoginFormTests();
        PerformAccountSettingsFormTests();
        PerformFormLayoutTests();
        PerformAccountOfferTests();

        // the plugins the tests loaded go away before the global systems do
        GUCEF::AUTH::CAuthGlobal::Instance()->GetOAuth2Engine()->ReleaseHttpBackends();
        GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "https" );
        GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "guidriver" );
        GUCEF::CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadPluginGroup( "codecs" );
        SetTestOpenId2Backend( GUCEF_NULL );

        GUCEF::CORE::CLogStreamScope::FlushLogs();

        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        return GUCEF_TESTFW_EXITCODE;
    }
    catch ( ... )
    {
        GUCEF::CORE::ShowErrorMessage( "Unknown exception"                                                                 ,
                                       "Unhandled exception during program execution, the application will now terminate"  );
    }
    return 1;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------*/
