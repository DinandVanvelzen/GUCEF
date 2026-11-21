/*
 *  AssetWorkbenchLib_TestApp: Test application for the asset workbench library (AssetWorkbenchLib)
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

#include "DVCPPOSWRAP.h"
#include "TestAssetWorkbenchSupport.h"
#include "TestAssetWorkbenchBasic.h"
#include "TestAssetWorkbenchRender.h"
#include "TestAssetWorkbenchShell.h"
#include "TestAssetWorkbenchViews.h"
#include "TestAssetWorkbenchSources.h"
#include "TestAssetWorkbenchSignIn.h"
#include "TestAssetWorkbenchOpen.h"
#include "TestAssetWorkbenchLoading.h"
#include "TestAssetWorkbenchParticles.h"
#include "TestAssetWorkbenchPackPresentation.h"
#include "TestAssetWorkbenchCookedOutput.h"
#include "TestAssetWorkbenchResourceView.h"
#include "TestAssetWorkbenchPackIndexSource.h"
#include "TestAssetWorkbenchServiceAdapters.h"
#include "TestAssetWorkbenchJobs.h"
#include "TestAssetWorkbenchSettings.h"
#include "TestAssetWorkbenchRemoteStore.h"
#include "TestAssetWorkbenchServerSettings.h"

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
        CORE::AppendToPath( logFilename, "AssetWorkbenchLib_TestApp_Log.txt" );
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

        // the pack indexes of the packs the tests open are kept in a folder of the tests, which starts empty
        {
            CORE::CString cache = CORE::RelativePath( "$CURWORKDIR$" );
            CORE::AppendToPath( cache, "AssetWorkbenchLib_TestApp_Work" );
            CORE::AppendToPath( cache, "PackIndexCache" );
            if ( CORE::DirExists( cache ) )
                CORE::DeleteDir( cache, true );
            CORE::CreateDirs( cache );
            CORE::SetEnv( CORE::CString( "GUCEF_PACKINDEX_CACHE" ), cache );
        }

        GUCEF_TESTFW_INIT( "AssetWorkbenchLib_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        {
            // the Ogre support library, the plugins and the drivers; every test case below is skipped when this is not possible
            GUCEF_TESTFW_SUITE_SCOPE( "AssetWorkbenchLib.Setup" );
            GUCEF_TESTFW_TESTCASE( "Setup: the support library, the Ogre plugins, the window backend and the GUI driver load" )
            try
            {
                GUCEF_TESTFW_ASSERT_TRUE( SetUpWorkbenchTests() );
            }
            catch ( ... )
            {
                GUCEF_TESTFW_ERRORHERE;
            }
            GUCEF_TESTFW_TESTCASE_END
        }

        // GUCEF_TEST_ONLY_VIEWS=1 runs only the suite of the views and the arrangement of the window, GUCEF_TEST_REPEAT_VIEWS=<n> runs it n times
        // (to see that it does not depend on the order or the timing of the frames)
        const CORE::CString repeatText = CORE::CString( GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_REPEAT_VIEWS" ).c_str() );
        const UInt32 viewRepeats = CORE::StringToUInt32( repeatText, 1 ) > 0 ? CORE::StringToUInt32( repeatText, 1 ) : 1;
        const bool onlyViews = !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_VIEWS" ).empty();
        // GUCEF_TEST_ONLY_SETTINGS=1 runs only the suite of the settings, the console, the help and the tooltips, GUCEF_TEST_REPEAT_SETTINGS=<n> n times
        const CORE::CString settingsRepeatText = CORE::CString( GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_REPEAT_SETTINGS" ).c_str() );
        const UInt32 settingsRepeats = CORE::StringToUInt32( settingsRepeatText, 1 ) > 0 ? CORE::StringToUInt32( settingsRepeatText, 1 ) : 1;
        const bool onlySettings = !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_SETTINGS" ).empty();
        // GUCEF_TEST_ONLY_PACKS=1 runs only the suite of the presentation of packs, GUCEF_TEST_REPEAT_PACKS=<n> n times
        const CORE::CString packsRepeatText = CORE::CString( GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_REPEAT_PACKS" ).c_str() );
        const UInt32 packsRepeats = CORE::StringToUInt32( packsRepeatText, 1 ) > 0 ? CORE::StringToUInt32( packsRepeatText, 1 ) : 1;
        const bool onlyPacks = !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_PACKS" ).empty();
        // GUCEF_TEST_ONLY_JOBS=1 runs only the suite of the jobs, the bulk conversion and the redo actions, GUCEF_TEST_REPEAT_JOBS=<n> n times; GUCEF_TEST_ONLY_SOURCES=1 only the sources
        const CORE::CString jobsRepeatText = CORE::CString( GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_REPEAT_JOBS" ).c_str() );
        const UInt32 jobsRepeats = CORE::StringToUInt32( jobsRepeatText, 1 ) > 0 ? CORE::StringToUInt32( jobsRepeatText, 1 ) : 1;
        const bool onlyJobs = !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_JOBS" ).empty();
        const bool onlySources = !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_SOURCES" ).empty();
        // GUCEF_TEST_ONLY_REMOTE=1 runs only the suite of the connection to a remote asset store service
        const bool onlyRemote = !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_REMOTE" ).empty();
        // GUCEF_TEST_ONLY_REMOTE_THUMBS=1 runs the case of the pictures of a remote store only
        if ( !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_REMOTE_THUMBS" ).empty() )
            PerformAssetWorkbenchRemoteThumbnailTests();
        else
        // GUCEF_TEST_ONLY_REMOTE_MIGRATION=1 runs the cases of the files of the remote store on the shares of the service
        if ( !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_REMOTE_MIGRATION" ).empty() )
            PerformAssetWorkbenchRemoteMigrationTests();
        else
        // GUCEF_TEST_ONLY_SERVER_SETTINGS=1 runs the cases of the settings of a remote server
        if ( !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_SERVER_SETTINGS" ).empty() )
            PerformAssetWorkbenchServerSettingsTests();
        else
        // GUCEF_TEST_ONLY_REMOTE_OPEN=1 runs the cases of the Sources tab on a remote store and of opening what it lists
        if ( !GUCEF::TESTFW::GetEnvVar( "GUCEF_TEST_ONLY_REMOTE_OPEN" ).empty() )
            PerformAssetWorkbenchRemoteOpenTests();
        else
        if ( onlyRemote )
            PerformAssetWorkbenchRemoteStoreTests();
        else
        if ( onlyJobs )
        {
            for ( UInt32 repeat=0; repeat<jobsRepeats; ++repeat )
                PerformAssetWorkbenchJobTests();
        }
        else
        if ( onlySources )
            PerformAssetWorkbenchSourcesTests();
        else
        if ( onlyPacks )
        {
            for ( UInt32 repeat=0; repeat<packsRepeats; ++repeat )
                {
                    PerformAssetWorkbenchPackPresentationTests();
                    PerformAssetWorkbenchResourceViewTests();
                    PerformAssetWorkbenchPackIndexSourceTests();
                    PerformAssetWorkbenchCookedOutputTests();
                }
        }
        else
        if ( onlySettings )
        {
            for ( UInt32 repeat=0; repeat<settingsRepeats; ++repeat )
                PerformAssetWorkbenchSettingsTests();
        }
        else
        if ( !onlyViews )
        {
            PerformAssetWorkbenchBasicTests();
            PerformAssetWorkbenchRenderTests();
            PerformAssetWorkbenchShellTests();
            PerformAssetWorkbenchViewTests();
            PerformAssetWorkbenchParticleTests();
            PerformAssetWorkbenchPackPresentationTests();
            PerformAssetWorkbenchResourceViewTests();
            PerformAssetWorkbenchPackIndexSourceTests();
            PerformAssetWorkbenchCookedOutputTests();
            PerformAssetWorkbenchServiceAdapterTests();
            PerformAssetWorkbenchJobTests();
            PerformAssetWorkbenchSourcesTests();
            PerformAssetWorkbenchSignInTests();
            PerformAssetWorkbenchOpenTests();
            PerformAssetWorkbenchLoadingTests();
            PerformAssetWorkbenchSettingsTests();
            PerformAssetWorkbenchRemoteStoreTests();
            PerformAssetWorkbenchServerSettingsTests();
        }
        for ( UInt32 repeat=onlyViews ? 0 : 1; !onlySettings && !onlyPacks && !onlyJobs && !onlySources && !onlyRemote && repeat<viewRepeats; ++repeat )
            PerformAssetWorkbenchViewTests();
        TearDownWorkbenchTests();


        CORE::CLogStreamScope::FlushLogs();
        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "AssetWorkbenchLib_TestApp_Results.xml" );
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
