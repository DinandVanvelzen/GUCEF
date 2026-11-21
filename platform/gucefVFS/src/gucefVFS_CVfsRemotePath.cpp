/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#include <string.h>

#include "gucefVFS_CVfsRemotePath.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CVfsRemotePath::GetByteCount( const CORE::CString& text )
{GUCEF_TRACE;

    const UInt32 byteSize = text.ByteSize();
    return byteSize > 0 ? byteSize - 1 : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemotePath::Normalize( const CORE::CString& path )
{GUCEF_TRACE;

    const char* data = path.C_String();
    const UInt32 byteSize = GetByteCount( path );
    if ( GUCEF_NULL == data || 0 == byteSize )
        return CORE::CString::Empty;

    GUCEF::vector< char > result;
    result.reserve( byteSize );
    for ( UInt32 i=0; i<byteSize; ++i )
    {
        char c = data[ i ];
        if ( '\\' == c )
            c = '/';
        if ( '/' == c && ( result.empty() || '/' == result[ result.size() - 1 ] ) )
            continue;
        result.push_back( c );
    }
    while ( !result.empty() && '/' == result[ result.size() - 1 ] )
        result.pop_back();
    if ( result.empty() )
        return CORE::CString::Empty;
    return CORE::CString( &result[ 0 ], (UInt32) result.size() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemotePath::ParentOf( const CORE::CString& path )
{GUCEF_TRACE;

    const char* data = path.C_String();
    const UInt32 byteSize = GetByteCount( path );
    for ( UInt32 i=byteSize; i>0; --i )
    {
        if ( '/' == data[ i-1 ] )
            return i > 1 ? CORE::CString( data, i-1 ) : CORE::CString::Empty;
    }
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemotePath::LeafOf( const CORE::CString& path )
{GUCEF_TRACE;

    const char* data = path.C_String();
    const UInt32 byteSize = GetByteCount( path );
    for ( UInt32 i=byteSize; i>0; --i )
    {
        if ( '/' == data[ i-1 ] )
            return i < byteSize ? CORE::CString( data + i, byteSize - i ) : CORE::CString::Empty;
    }
    return path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsRemotePath::Join( const CORE::CString& dirPath, const CORE::CString& name )
{GUCEF_TRACE;

    if ( dirPath.IsNULLOrEmpty() )
        return name;
    if ( name.IsNULLOrEmpty() )
        return dirPath;
    return dirPath + '/' + name;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemotePath::IsSameOrBelow( const CORE::CString& dirPath, const CORE::CString& path )
{GUCEF_TRACE;

    if ( dirPath.IsNULLOrEmpty() )
        return true;
    const UInt32 dirBytes = GetByteCount( dirPath );
    const UInt32 pathBytes = GetByteCount( path );
    if ( pathBytes < dirBytes )
        return false;
    if ( 0 != memcmp( path.C_String(), dirPath.C_String(), dirBytes ) )
        return false;
    return pathBytes == dirBytes || '/' == path.C_String()[ dirBytes ];
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
