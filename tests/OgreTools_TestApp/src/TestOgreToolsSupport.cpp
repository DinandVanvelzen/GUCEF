/*
 *  OgreTools_TestApp: Test application for the Ogre asset tools and plugins (OgreAssimpConverter, OgreXMLConverter,
 *  OgreMeshUpgrader, OgreCodec_Assimp, OgrePlugin_DotScene)
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

#ifndef GUCEF_CORE_CPROCESS_H
#include "gucefCORE_CProcess.h"
#define GUCEF_CORE_CPROCESS_H
#endif /* GUCEF_CORE_CPROCESS_H ? */

#ifndef GUCEF_TEST_CHILD_PROCESS_H
#include "gucef_test_child_process.h"
#define GUCEF_TEST_CHILD_PROCESS_H
#endif /* GUCEF_TEST_CHILD_PROCESS_H ? */

#include <string.h>

#include "TestOgreToolsSupport.h"

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

static CORE::CString g_testAppPath;

/*-------------------------------------------------------------------------*/

CORE::CString
GetTestWorkDir( void )
{GUCEF_TRACE;

    CORE::CString dir = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( dir, "OgreTools_TestApp_Work" );
    return dir;
}

/*-------------------------------------------------------------------------*/

CORE::CString
MakeCleanTestDir( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString dir = GetTestWorkDir();
    CORE::AppendToPath( dir, name );
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
    CORE::CreatePathDirectories( dir + "/" );
    return dir;
}

/*-------------------------------------------------------------------------*/

bool
WriteTestFile( const CORE::CString& path, const CORE::CString& content )
{GUCEF_TRACE;

    CORE::CreatePathDirectories( path.SubstrToChar( '/', false ) );
    CORE::CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    if ( content.IsNULLOrEmpty() )
        return true;
    return 1 == file.Write( content.C_String(), content.Length(), 1 );
}

/*-------------------------------------------------------------------------*/

UInt64
GetTestFileSize( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "rb" );
    if ( !file.IsValid() )
        return 0;
    return file.GetSize();
}

/*-------------------------------------------------------------------------*/

void
RemoveTestWorkDir( void )
{GUCEF_TRACE;

    const CORE::CString dir = GetTestWorkDir();
    if ( CORE::DirExists( dir ) )
        CORE::DeleteDir( dir, true );
}

/*-------------------------------------------------------------------------*/

void
SetTestAppPath( const char* argv0 )
{GUCEF_TRACE;

    g_testAppPath = GUCEF_NULL == argv0 ? CORE::CString() : CORE::CString( argv0 );

    // a path relative to the working directory has to stay valid when a tool is started in another directory
    if ( GUCEF_NULL != argv0 && '\0' != argv0[ 0 ] )
    {
        const bool isAbsolute = '/' == argv0[ 0 ] || '\\' == argv0[ 0 ] || ':' == argv0[ 1 ];
        if ( !isAbsolute && GUCEF_NULL != strchr( argv0, '/' ) )
            g_testAppPath = CORE::CombinePath( CORE::CurrentWorkingDir(), CORE::CString( '.' == argv0[ 0 ] && '/' == argv0[ 1 ] ? argv0 + 2 : argv0 ) );
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
FindToolExecutable( const CORE::CString& toolName )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    const CORE::CString fileName = toolName + ".exe";
    #else
    const CORE::CString& fileName = toolName;
    #endif

    // The tools are built next to the test application: the directory of its start path, and the directories
    // the modules of this application are in (the executables of a build are in bin, next to lib)
    CORE::CString::StringVector candidates;
    if ( !g_testAppPath.IsNULLOrEmpty() )
        candidates.push_back( CORE::StripFilename( CORE::RelativePath( g_testAppPath ) ) );
    candidates.push_back( CORE::RelativePath( "$MODULEDIR$/../bin" ) );
    candidates.push_back( CORE::RelativePath( "$MODULEDIR$" ) );
    candidates.push_back( CORE::RelativePath( "$CURWORKDIR$" ) );

    for ( size_t i=0; i<candidates.size(); ++i )
    {
        CORE::CString path = candidates[ i ];
        CORE::AppendToPath( path, fileName );
        if ( CORE::FileExists( path ) )
            return path;
    }
    return CORE::CString();
}

/*-------------------------------------------------------------------------*/

bool
RunTool( const CORE::CString& toolName                  ,
         const CORE::CProcess::TStringVector& args      ,
         const CORE::CString& workingDir                ,
         Int32& exitCode                                ,
         CORE::CString& output                          ,
         UInt32 timeoutInMs                             )
{GUCEF_TRACE;

    output.Clear();
    exitCode = -1;

    const CORE::CString exePath = FindToolExecutable( toolName );
    if ( exePath.IsNULLOrEmpty() )
    {
        output = "tool not found: " + toolName;
        return false;
    }

    CORE::CProcess process;
    CORE::CProcess::TStringMap environment = TESTFW::ChildEnvironmentWithOwnExitCode();
    if ( !process.LaunchWithPipes( exePath, args, false, true, true, workingDir, environment ) )
    {
        output = "tool could not be started: " + exePath;
        return false;
    }

    // read what the tool writes while it runs, a tool that fills a pipe nobody reads blocks
    char buffer[ 1024 ];
    UInt32 waited = 0;
    bool ended = false;
    while ( !ended && waited <= timeoutInMs )
    {
        CORE::CProcessPipe* pipes[ 2 ] = { process.GetStdoutPipe(), process.GetStderrPipe() };
        bool gotData = false;
        for ( int i=0; i<2; ++i )
        {
            if ( GUCEF_NULL != pipes[ i ] && pipes[ i ]->WaitForData( 0 ) )
            {
                UInt32 got = pipes[ i ]->ReadSome( buffer, sizeof( buffer ) - 1 );
                if ( got > 0 )
                {
                    buffer[ got ] = '\0';
                    output += buffer;
                    gotData = true;
                }
            }
        }
        if ( !gotData )
        {
            ended = process.WaitForExit( 20, exitCode );
            waited += 20;
        }
    }
    if ( !ended )
    {
        process.Terminate( false );
        output += "\n(tool did not end in time and was terminated)";
        return false;
    }

    // what is left in the pipes
    for ( int i=0; i<2; ++i )
    {
        CORE::CProcessPipe* pipe = 0 == i ? process.GetStdoutPipe() : process.GetStderrPipe();
        while ( GUCEF_NULL != pipe && pipe->WaitForData( 0 ) )
        {
            UInt32 got = pipe->ReadSome( buffer, sizeof( buffer ) - 1 );
            if ( 0 == got )
                break;
            buffer[ got ] = '\0';
            output += buffer;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/
