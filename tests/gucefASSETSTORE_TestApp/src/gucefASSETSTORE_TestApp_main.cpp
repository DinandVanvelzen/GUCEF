/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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
#include <stdlib.h>

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
#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */
#ifndef GUCEF_ASSETSTORE_H
#include "gucefASSETSTORE.h"
#define GUCEF_ASSETSTORE_H
#endif /* GUCEF_ASSETSTORE_H ? */
#ifndef TESTASSETSTORESUPPORT_H
#include "TestAssetStoreSupport.h"
#define TESTASSETSTORESUPPORT_H
#endif /* TESTASSETSTORESUPPORT_H ? */
#include "TestPackStorageLayout.h"
#include "TestPackIndex.h"
#include "TestNormalisation.h"
#include "TestDownloads.h"
#include "TestLibraryFileSystemAccess.h"
#include "TestImageCache.h"
#include "TestPrimaryStore.h"
#include "TestConversion.h"
#include "TestWebSignIn.h"
#include "TestAssetStoreService.h"
#include "TestAssetToolsMcp.h"
#include "TestAssetStoreRemoteService.h"
#include "TestRemoteAssetFiles.h"
#include "TestAssetStorePluginLoader.h"
#include "TestLibrarySync.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    using namespace GUCEF;
    try
    {
        CORE::CString logFilename = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( logFilename, "gucefASSETSTORE_TestApp_Log.txt" );
        CORE::CFileAccess logFileAccess( logFilename, "w" );

        CORE::CStdLogger logger( logFileAccess );
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

        CORE::CPlatformNativeConsoleLogger consoleOut;
        if ( GUCEF_APP_TYPE == GUCEF_APP_TYPE_CONSOLE )
            CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleOut.GetLogger() );

        CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();

        GUCEF_TESTFW_INIT( "gucefASSETSTORE_TestApp" );
        GUCEF_TESTFW_SET_CALLBACK( TESTFW::TestFrameworkLogCallback, GUCEF_NULL );

        // the module, the codecs and the database backends
        ASSETSTORE::CAssetStoreGlobal::Instance();
        ASSETSTORETEST::SetUpTestEnvironment();

        // GUCEF_TEST_ASSETTOOLS_ONLY=1 runs the suites of the asset tool set only (repeated runs of those suites);
        // GUCEF_TEST_ASSETSTORE_REMOTE_ONLY=1 the suites of the remote service, =faults its fault suite, =live its live smoke suite
        const char* toolsOnly = ::getenv( "GUCEF_TEST_ASSETTOOLS_ONLY" );
        const char* remoteOnly = ::getenv( "GUCEF_TEST_ASSETSTORE_REMOTE_ONLY" );
        // GUCEF_TEST_ASSETSTORE_SYNC_ONLY=1 runs the suites of the library sync only
        const char* syncOnly = ::getenv( "GUCEF_TEST_ASSETSTORE_SYNC_ONLY" );
        const CORE::CString remoteSelection = GUCEF_NULL != remoteOnly ? CORE::CString( remoteOnly ) : CORE::CString::Empty;
        if ( GUCEF_NULL != syncOnly && '1' == syncOnly[ 0 ] )
        {
            PerformLibrarySyncUnitTests();
            PerformLibrarySyncVfsTablesTests();
            PerformLibrarySyncPostgreSqlTests();
        }
        else
        if ( "live" == remoteSelection )
        {
            PerformAssetStoreRemoteLiveSmokeTests();
        }
        else
        if ( "faults" == remoteSelection )
        {
            PerformAssetStoreRemoteFaultTests();
            PerformAssetStoreRemoteSignInRoleTests();
        }
        else
        if ( "files" == remoteSelection )
        {
            PerformAssetStoreBlockCacheTests();
            PerformAssetStoreRemoteFilesTests();
            PerformAssetStoreRemoteFilesGateTests();
        }
        else
        if ( "1" == remoteSelection )
        {
            PerformAssetStoreRemoteUnitTests();
            PerformAssetStoreRemoteServiceVfsTablesTests();
            PerformAssetStoreRemoteServicePostgreSqlTests();
            PerformAssetStoreRemoteFaultTests();
            PerformAssetStoreRemoteSignInRoleTests();
            PerformAssetStoreBlockCacheTests();
            PerformAssetStoreRemoteFilesTests();
            PerformAssetStoreRemoteFilesGateTests();
            PerformAssetStoreRemoteLiveSmokeTests();
        }
        else
        if ( GUCEF_NULL != toolsOnly && '1' == toolsOnly[ 0 ] )
        {
            PerformAssetToolsMcpFakeTests();
            PerformAssetToolsMcpLocalTests();
        }
        else
        {
            PerformPackStorageLayoutTests();
            PerformNormalisationTests();
            PerformDownloadTests();
            PerformLibraryFileSystemAccessTests();
            PerformImageCacheTests();
            PerformAssetStorePluginLoaderTests();
            PerformPrimaryStoreVfsTablesTests();
            PerformPrimaryStorePostgreSqlTests();
            PerformPackIndexVfsTablesTests();
            PerformPackIndexPostgreSqlTests();
            PerformConversionVfsTablesTests();
            PerformConversionPostgreSqlTests();
            PerformWebSignInTests();
            PerformAssetStoreServiceUnitTests();
            PerformAssetStoreServiceVfsTablesTests();
            PerformAssetStoreServicePostgreSqlTests();
            PerformLibrarySyncUnitTests();
            PerformLibrarySyncVfsTablesTests();
            PerformLibrarySyncPostgreSqlTests();
            PerformAssetStoreRemoteUnitTests();
            PerformAssetStoreRemoteServiceVfsTablesTests();
            PerformAssetStoreRemoteServicePostgreSqlTests();
            PerformAssetStoreRemoteFaultTests();
            PerformAssetStoreRemoteSignInRoleTests();
            PerformAssetStoreBlockCacheTests();
            PerformAssetStoreRemoteFilesTests();
            PerformAssetStoreRemoteFilesGateTests();
            PerformAssetStoreRemoteLiveSmokeTests();
            PerformAssetToolsMcpFakeTests();
            PerformAssetToolsMcpLocalTests();
        }

        CORE::CLogStreamScope::FlushLogs();

        CORE::CString xmlPath = CORE::RelativePath( "$CURWORKDIR$" );
        CORE::AppendToPath( xmlPath, "gucefASSETSTORE_TestApp_Results.xml" );
        GUCEF_TESTFW_WRITE_RESULTS( xmlPath.C_String() );
        ASSETSTORETEST::TearDownTestEnvironment();
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
