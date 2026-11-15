/*
 *  pluginglueOGRE_TestApp: Test application for the Ogre glue modules (pluginglueOGRE, OgrePlugin_GucefVFS, OgreCodec_GucefIMAGE)
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

#include "TestSupport.h"

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

CORE::CString
GetTestWorkDir( void )
{GUCEF_TRACE;

    CORE::CString dir = CORE::RelativePath( "$CURWORKDIR$" );
    CORE::AppendToPath( dir, "pluginglueOGRE_TestApp_Work" );
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

CORE::CString
ReadTestFile( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "rb" );
    if ( !file.IsValid() )
        return CORE::CString();

    CORE::CString content;
    char buffer[ 1025 ];
    UInt32 got = 0;
    while ( ( got = file.Read( buffer, 1, sizeof( buffer ) - 1 ) ) > 0 )
    {
        buffer[ got ] = '\0';
        content += buffer;
    }
    return content;
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
