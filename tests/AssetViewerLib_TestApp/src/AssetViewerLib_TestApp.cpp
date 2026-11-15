/*
 *  AssetViewerLib_TestApp: Test application for the asset viewer library (AssetViewerLib)
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

#define GUCEF_TESTFW_IMPL

#ifndef GUCEF_TESTFW_H
#include "gucef_test_framework.h"
#define GUCEF_TESTFW_H
#endif /* GUCEF_TESTFW_H ? */

#ifndef GUCEF_TEST_LOG_CALLBACK_H
#include "gucef_test_log_callback.h"
#define GUCEF_TEST_LOG_CALLBACK_H
#endif /* GUCEF_TEST_LOG_CALLBACK_H ? */

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

#ifndef GUCEF_GUI_H
#include "gucefGUI.h"
#define GUCEF_GUI_H
#endif /* GUCEF_GUI_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#include "gucefASSETSTORE_CAssetStoreGlobal.h"
#define GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREGLOBAL_H ? */


using namespace GUCEF;

#include "TestAssetViewerSupport.h"
#include "TestAssetViewerBasic.h"
#include "TestAssetViewerRender.h"
#include "TestAssetViewerShell.h"
#include "TestAssetViewerSources.h"
#include "TestAssetViewerSignIn.h"
#include "TestAssetViewerOpen.h"
#include "TestAssetViewerParticles.h"
#include "TestAssetViewerServiceAdapters.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    try
    {
        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "AssetViewerLib_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );
        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger console;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( console.GetLogger() );

        // initializes the modules, as every application using them has to do
        VFS::CVfsGlobal::Instance();
        IMAGE::CImageGlobal::Instance();
        GUI::CGuiGlobal::Instance();
        COMCORE::CComCoreGlobal::Instance();
        ASSETSTORE::CAssetStoreGlobal::Instance();

        GUCEF_TESTFW_INIT( "AssetViewerLib_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        {
            // the Ogre support library, the plugins and the drivers; every test case below is skipped when this is not possible
            GUCEF_TESTFW_SUITE_SCOPE( "AssetViewerLib.Setup" );
            GUCEF_TESTFW_TESTCASE( "Setup: the support library, the Ogre plugins, the window backend and the GUI driver load" )
            try
            {
                GUCEF_TESTFW_ASSERT_TRUE( SetUpViewerTests() );
            }
            catch ( ... )
            {
                GUCEF_TESTFW_ERRORHERE;
            }
            GUCEF_TESTFW_TESTCASE_END
        }

        PerformAssetViewerBasicTests();
        PerformAssetViewerRenderTests();
        PerformAssetViewerShellTests();
        PerformAssetViewerParticleTests();
        PerformAssetViewerServiceAdapterTests();
        PerformAssetViewerSourcesTests();
        PerformAssetViewerSignInTests();
        PerformAssetViewerOpenTests();
        TearDownViewerTests();


        CORE::CLogStreamScope::FlushLogs();
        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "AssetViewerLib_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );

        CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();
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
