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

#ifndef GUCEF_AUTH_TESTAPP_TESTFILES_H
#define GUCEF_AUTH_TESTAPP_TESTFILES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

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

#endif /* GUCEF_AUTH_TESTAPP_TESTFILES_H ? */
