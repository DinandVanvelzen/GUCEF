/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestVars.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------*/

void
PerformVarsTests( void )
{
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING variable resolution TESTS" );
    
    GUCEF_TESTFW_SUITE_SCOPE( "Vars" );

    GUCEF_TESTFW_TESTCASE( "Test: Variable resolution" )
    try
    {
        // $CURWORKDIR$ -> Is resolved to the current working directory
        
        CORE::CString currentWorkingDir = CORE::ResolveVars( "$CURWORKDIR$" );
        ASSERT_TRUE( !currentWorkingDir.IsNULLOrEmpty() );
        ASSERT_TRUE( currentWorkingDir.IsFormattingValid() );

        // $MODULEDIR$  -> Is resolved to the directory this module is located in
        
        CORE::CString moduleDir = CORE::ResolveVars( "$MODULEDIR$" );
        ASSERT_TRUE( !moduleDir.IsNULLOrEmpty() );
        ASSERT_TRUE( moduleDir.IsFormattingValid() );

        // $HOSTNAME$  -> Is resolved to the hostname of the system
        
//        CORE::CString hostname = CORE::ResolveVars( "$HOSTNAME$" );
//        ASSERT_TRUE( !hostname.IsNULLOrEmpty() );
//        ASSERT_TRUE( hostname.IsFormattingValid() );

        // $PID$  -> Is resolved to the process ID of the current process
        
        CORE::CString pidStr = CORE::ResolveVars( "$PID$" );
        ASSERT_TRUE( !pidStr.IsNULLOrEmpty() );
        ASSERT_TRUE( pidStr.IsFormattingValid() );
        ASSERT_TRUE( -1 != CORE::StringToInt64( pidStr, -1 ) );

        // $ENVVAR:FOO$ -> Resolves to the environment variable FOO's content
        
        CORE::CString envVarStr = CORE::ResolveVars( "$ENVVAR:__envVarWhichDoesNotExist__$" );
        ASSERT_TRUE( envVarStr.IsNULLOrEmpty() );
        ASSERT_TRUE( envVarStr.IsFormattingValid() );
        
        #if GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX || GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID
          envVarStr = CORE::ResolveVars( "$ENVVAR:PWD$" );
          ASSERT_TRUE( !envVarStr.IsNULLOrEmpty() );
          ASSERT_TRUE( envVarStr.IsFormattingValid() );
          ASSERT_TRUE( CORE::IsPathValid( envVarStr ) );
          ASSERT_TRUE( envVarStr == CORE::GetEnv( "PWD" ) );
        #elif GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN
          envVarStr = CORE::ResolveVars( "$ENVVAR:windir$" );
          ASSERT_TRUE( !envVarStr.IsNULLOrEmpty() );
          ASSERT_TRUE( envVarStr.IsFormattingValid() );
          ASSERT_TRUE( CORE::IsPathValid( envVarStr ) );
          ASSERT_TRUE( envVarStr == CORE::GetEnv( "windir" ) );
        #endif
        
        ASSERT_TRUE( 0 != CORE::SetEnv( "testEnvironmentVariable", "testEnvironmentVariableValue" ) );
        envVarStr = CORE::ResolveVars( "$ENVVAR:testEnvironmentVariable$" );
        ASSERT_TRUE( !envVarStr.IsNULLOrEmpty() );
        ASSERT_TRUE( envVarStr.IsFormattingValid() );
        ASSERT_TRUE( "testEnvironmentVariableValue" == envVarStr );

        ASSERT_TRUE( 0 != CORE::SetEnv( "testEnvironmentVariable_FOO", "FOO" ) );
        ASSERT_TRUE( 0 != CORE::SetEnv( "testEnvironmentVariable_BAR", "BAR" ) );
        envVarStr = CORE::ResolveVars( "$ENVVAR:testEnvironmentVariable_FOO$:$ENVVAR:testEnvironmentVariable_BAR$" );
        ASSERT_TRUE( !envVarStr.IsNULLOrEmpty() );
        ASSERT_TRUE( envVarStr.IsFormattingValid() );
        ASSERT_TRUE( "FOO:BAR" == envVarStr );

    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Test: Variable resolution leaves text without variables as it is and resolves only the variables present" )
    try
    {
        // text without a '$' comes back unchanged, also text that looks like a path or holds variable names without the '$'
        ASSERT_TRUE( CORE::CString::Empty == CORE::ResolveVars( CORE::CString::Empty ) );
        ASSERT_TRUE( "/data/XNYS/2024/2024-01-02/daily_bars.parquet" == CORE::ResolveVars( "/data/XNYS/2024/2024-01-02/daily_bars.parquet" ) );
        ASSERT_TRUE( "TEMPDIR/MODULEDIR:CURWORKDIR" == CORE::ResolveVars( "TEMPDIR/MODULEDIR:CURWORKDIR" ) );
        ASSERT_TRUE( "../relative/path.txt" == CORE::ResolveVars( "../relative/path.txt" ) );

        // a '$' that starts no variable stays, as before
        ASSERT_TRUE( "price$/a$b$" == CORE::ResolveVars( "price$/a$b$" ) );

        // every variable present is resolved, the rest of the text stays
        const CORE::CString cwd = CORE::CurrentWorkingDir();
        const CORE::CString pid = CORE::UInt32ToString( MT::GetProcessID() );
        ASSERT_TRUE( cwd + "/a/" + pid + "/b" == CORE::ResolveVars( "$CURWORKDIR$/a/$PID$/b" ) );
        ASSERT_TRUE( CORE::ModuleDir() + "/x" == CORE::ResolveVars( "$MODULEDIR$/x" ) );
        ASSERT_TRUE( CORE::TempDir() + "|" + CORE::TempDir() == CORE::ResolveVars( "$TEMPDIR$|$TEMPDIR$" ) );

        // an environment variable whose value holds another variable: that one is resolved too
        ASSERT_TRUE( 0 != CORE::SetEnv( "testEnvironmentVariable_NESTED", "$PID$" ) );
        ASSERT_TRUE( "id=" + pid == CORE::ResolveVars( "id=$ENVVAR:testEnvironmentVariable_NESTED$" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    CORE::CLogStreamScope::FlushLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ALL variable resolution TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
