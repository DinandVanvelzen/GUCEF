/*
 *  gucefAUTH_TestApp: Application for testing gucefAUTH
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

#ifndef GUCEF_AUTH_TESTAPP_TESTFILES_H
#define GUCEF_AUTH_TESTAPP_TESTFILES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <sys/stat.h>
  #include <unistd.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A directory for a test, below the working directory of the test app, emptied first */
inline GUCEF::CORE::CString
MakeTestDirectory( const GUCEF::CORE::CString& name )
{GUCEF_TRACE;

    GUCEF::CORE::CString dir = GUCEF::CORE::CombinePath( GUCEF::CORE::CombinePath( GUCEF::CORE::RelativePath( "$CURWORKDIR$" ), "authtemp" ), name );
    if ( GUCEF::CORE::DirExists( dir ) )
        GUCEF::CORE::DeleteDir( dir, true );
    return dir;
}

/*-------------------------------------------------------------------------*/

/** The names of the files and directories in a directory */
inline GUCEF::CORE::CString::StringVector
ListDirectory( const GUCEF::CORE::CString& dir )
{GUCEF_TRACE;

    GUCEF::CORE::CString::StringVector names;
    GUCEF::CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( dir ) )
    {
        do
        {
            names.push_back( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return names;
}

/*-------------------------------------------------------------------------*/

inline bool
HasName( const GUCEF::CORE::CString::StringVector& names, const GUCEF::CORE::CString& name )
{GUCEF_TRACE;

    for ( size_t i=0; i<names.size(); ++i )
        if ( names[ i ] == name )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes occur in the file */
inline bool
FileContains( const GUCEF::CORE::CString& path, const GUCEF::CORE::CString& text )
{GUCEF_TRACE;

    GUCEF::CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) || 0 == content.GetDataSize() )
        return false;
    const char* data = content.AsConstTypePtr< char >();
    GUCEF::UInt32 size = content.GetDataSize();
    GUCEF::UInt32 length = text.Length();
    for ( GUCEF::UInt32 i=0; i+length<=size; ++i )
        if ( 0 == memcmp( data + i, text.C_String(), length ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Whether the bytes occur in the buffer */
inline bool
BufferContains( const GUCEF::CORE::CDynamicBuffer& content, const GUCEF::CORE::CString& text )
{GUCEF_TRACE;

    const char* data = content.AsConstTypePtr< char >();
    GUCEF::UInt32 size = content.GetDataSize();
    GUCEF::UInt32 length = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    for ( GUCEF::UInt32 i=0; i+length<=size; ++i )
        if ( 0 == memcmp( data + i, text.C_String(), length ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

/** Flips one bit in the byte at the offset of a file */
inline bool
FlipFileByte( const GUCEF::CORE::CString& path, GUCEF::UInt32 offset )
{GUCEF_TRACE;

    GUCEF::CORE::CDynamicBuffer content;
    if ( !content.LoadContentFromFile( path ) || offset >= content.GetDataSize() )
        return false;
    static_cast< GUCEF::UInt8* >( content.GetBufferPtr() )[ offset ] ^= 0x01;
    return GUCEF::CORE::WriteFileAtomically( path, content );
}

/*-------------------------------------------------------------------------*/

/** Waits for a file to be there, up to the time given */
inline bool
WaitForFile( const GUCEF::CORE::CString& path, GUCEF::UInt32 timeoutInMs )
{GUCEF_TRACE;

    for ( GUCEF::UInt32 waited=0; waited<timeoutInMs; waited+=5 )
    {
        if ( GUCEF::CORE::FileExists( path ) )
            return true;
        GUCEF::MT::PrecisionDelay( 5 );
    }
    return GUCEF::CORE::FileExists( path );
}

/*-------------------------------------------------------------------------*/

/** The value of an environment variable, empty when it is not set */
inline GUCEF::CORE::CString
GetEnvText( const char* name )
{GUCEF_TRACE;

    const char* value = getenv( name );
    return GUCEF_NULL != value ? GUCEF::CORE::CString( value ) : GUCEF::CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )

/** The path of the executable of this process, to start the test application again as a second process */
inline GUCEF::CORE::CString
GetOwnExecutablePath( void )
{GUCEF_TRACE;

    char path[ 4096 ];
    ssize_t length = readlink( "/proc/self/exe", path, sizeof( path ) - 1 );
    if ( length <= 0 )
        return GUCEF::CORE::CString::Empty;
    path[ length ] = '\0';
    return GUCEF::CORE::CString( path );
}

#endif

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_TESTAPP_TESTFILES_H ? */
