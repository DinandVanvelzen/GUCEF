/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>

#ifndef GUCEF_MT_H
#include "gucefMT.h"
#define GUCEF_MT_H
#endif /* GUCEF_MT_H ? */

#ifdef GUCEF_MSWIN_BUILD
  #include <process.h>
#else
  #include <unistd.h>
#endif

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_H
#include "gucefVFS.h"
#define GUCEF_VFS_H
#endif /* GUCEF_VFS_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestRemoteArchiveFixture.h"

#include "TestVfsAsyncTeardown.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** A precondition: when it does not hold the case ends here, so that the lines after it do not meet what it should have made */
#define REQUIRE(t)      do { if ( !(t) ) { GUCEF_TESTFW_ASSERT_TRUE(t); throw std::runtime_error( "a precondition of the case does not hold" ); } } while (0)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The calls that are inside the remote place when the process ends: the pool of the remote operations has four threads */
const UInt32 RunningCalls = 4;

/**
 *  What the case leaves for the check at the end of the process: the results of the calls that were under way when the
 *  process ended. It is never freed: the check runs after the systems of GUCEF were taken down.
 */
struct STeardownState
{
    CORE::CFutureResult* futures[ RunningCalls ];
    VFS::CVfsAsyncResultPtr* results[ RunningCalls ];
    CORE::CString markerPath;
    CORE::CString folderPath;                   /**< the folder of the place, which nothing else removes */
    bool unloadSeen;                            /**< the archive was unloaded by the VFS while it went */
    UInt32 callsInDelayAtUnload;                /**< the calls that were still inside the place at that moment */
    STeardownState( void ) : markerPath(), folderPath(), unloadSeen( false ), callsInDelayAtUnload( 0 ) {}
};

STeardownState* g_state = GUCEF_NULL;
CRemoteTestEnv* g_env = GUCEF_NULL;

/** Called when the VFS unloads the test archive: by then it has waited for the calls that were under way, so no call is inside the place */
void
OnArchiveUnload( void )
{
    if ( GUCEF_NULL == g_state || GUCEF_NULL == g_env )
        return;
    g_state->unloadSeen = true;
    g_state->callsInDelayAtUnload = g_env->Backend().GetCallsInDelay();
}

void
WriteMarker( const char* verdict )
{
    // plain file calls: the logging of GUCEF has been taken down by the time this runs
    if ( GUCEF_NULL == g_state || g_state->markerPath.IsNULLOrEmpty() )
        return;
    FILE* marker = fopen( g_state->markerPath.C_String(), "w" );
    if ( GUCEF_NULL != marker )
    {
        fputs( verdict, marker );
        fclose( marker );
    }
}

/**
 *  Runs when the process ends, after the VFS was taken down (it is registered before the VFS is made, so that it comes after
 *  the one of the VFS: the functions run in the reverse order of their registration). The VFS waited for the calls that
 *  were under way: when it unloaded the archive no call was inside the place any more, and each call, which was inside the
 *  archive when the process started to end, had delivered its answer.
 */
void
CheckAfterVfsDestruction( void )
{
    if ( GUCEF_NULL == g_state )
        return;

    bool good = g_state->unloadSeen && 0 == g_state->callsInDelayAtUnload;
    for ( UInt32 i=0; i<RunningCalls; ++i )
    {
        const VFS::CVfsAsyncResult* result = g_state->results[ i ]->GetPointerAlways();
        if ( GUCEF_NULL == result || !result->result.IsSuccess() )
            good = false;
    }

    WriteMarker( good ? "verified" : "failed" );

    // the folder of the place is not left behind: it is a folder of the tests, below the scratch directory
    if ( !g_state->folderPath.IsNULLOrEmpty() && g_state->folderPath.HasSubstr( "gucefvfs-blockcache-" ) >= 0 )
        CORE::DeleteDir( g_state->folderPath, true );

    if ( !good )
        _exit( 3 );
}

/** Registers the check at the start of the process, before anything of GUCEF is made */
struct SRegisterCheck
{
    SRegisterCheck( void ) { atexit( &CheckAfterVfsDestruction ); }
};

SRegisterCheck g_registerCheck;

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsAsyncTeardownTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsAsyncTeardown" );

    GUCEF_TESTFW_TESTCASE( "The VFS goes with calls under way: the process ends while four calls wait for a slow remote place; the VFS waits for them, and the check at the end of the process finds that no call was inside the place when the archive was unloaded and that each delivered its answer (it ends the process with status 3 and a marker file that says 'failed' when not)" )
    try
    {
        typedef CRemoteTestEnv E;
        E* env = GUCEF_NEW E( "gucefvfs-async-td" );          // never deleted: the VFS goes with the process
        REQUIRE( env->Setup() );
        g_env = env;
        CTestRemoteArchive::SetUnloadHook( &OnArchiveUnload );
        VFS::CVFS& vfs = env->Vfs();
        env->Backend().SetDelayMs( 300 );

        STeardownState* state = GUCEF_NEW STeardownState();
        state->folderPath = env->GetCacheFolder();
        const char* scratch = getenv( "GUCEF_TEST_SCRATCH_DIR" );
        if ( GUCEF_NULL != scratch )
        {
            state->markerPath = scratch;
            state->markerPath += "/gucefVFS_TestApp_async_teardown_marker.txt";
            remove( state->markerPath.C_String() );
        }

        VFS::CVfsAsyncResultPtr result;
        state->futures[ 0 ] = GUCEF_NEW CORE::CFutureResult( vfs.GetFileMetaDataAsync( env->VfsPath( E::MidFile ), result ) );
        state->results[ 0 ] = GUCEF_NEW VFS::CVfsAsyncResultPtr( result );
        state->futures[ 1 ] = GUCEF_NEW CORE::CFutureResult( vfs.GetFileMetaDataAsync( env->VfsPath( E::BigFile ), result ) );
        state->results[ 1 ] = GUCEF_NEW VFS::CVfsAsyncResultPtr( result );
        state->futures[ 2 ] = GUCEF_NEW CORE::CFutureResult( vfs.GetFileListAsync( env->VfsPath( "docs" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, result ) );
        state->results[ 2 ] = GUCEF_NEW VFS::CVfsAsyncResultPtr( result );
        state->futures[ 3 ] = GUCEF_NEW CORE::CFutureResult( vfs.GetDirListAsync( env->VfsPath( "data" ), false, false, CORE::CString::EmptyStringSet, GUCEF_UINT32MAX, result ) );
        state->results[ 3 ] = GUCEF_NEW VFS::CVfsAsyncResultPtr( result );
        for ( UInt32 w=0; w<3000 && env->Backend().GetStatCount() + env->Backend().GetListCount() < RunningCalls; ++w )
            MT::ThreadDelay( 1 );
        REQUIRE( env->Backend().GetStatCount() + env->Backend().GetListCount() >= RunningCalls );

        // all of them are inside the place, none is done: the process ends now
        for ( UInt32 i=0; i<RunningCalls; ++i )
            ASSERT_FALSE( state->futures[ i ]->IsReady() );
        g_state = state;
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}
