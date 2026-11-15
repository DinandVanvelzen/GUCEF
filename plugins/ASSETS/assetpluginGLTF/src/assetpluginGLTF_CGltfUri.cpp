/*
 *  assetpluginGLTF: the URIs of glTF files: percent encoding, decoding and resolution against the folder of the file
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

#include "assetpluginGLTF_CGltfUri.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace GLTF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
IsUnreserved( char c )
{
    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
           '-' == c || '_' == c || '.' == c || '~' == c || '/' == c;
}

Int32
HexValue( char c )
{
    if ( c >= '0' && c <= '9' ) return c - '0';
    if ( c >= 'a' && c <= 'f' ) return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' ) return c - 'A' + 10;
    return -1;
}

/** The parts of a path in order, without empty parts */
void
SplitParts( const CORE::CString& path, CORE::CString::StringVector& parts )
{
    parts.clear();
    CORE::CString part;
    const char* text = path.C_String();
    const UInt32 size = path.ByteSize() > 0 ? path.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<=size; ++i )
    {
        const char c = i < size ? text[ i ] : '/';
        if ( '/' == c || '\\' == c )
        {
            if ( !part.IsNULLOrEmpty() )
                parts.push_back( part );
            part = CORE::CString::Empty;
        }
        else
        {
            part.Append( &c, 1 );
        }
    }
}

CORE::CString
JoinParts( const CORE::CString::StringVector& parts, size_t first = 0 )
{
    CORE::CString path;
    for ( size_t i=first; i<parts.size(); ++i )
    {
        if ( !path.IsNULLOrEmpty() )
            path += "/";
        path += parts[ i ];
    }
    return path;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CGltfUri::Encode( const CORE::CString& path )
{GUCEF_TRACE;

    static const char* digits = "0123456789ABCDEF";
    CORE::CString result;
    const char* text = path.C_String();
    const UInt32 size = path.ByteSize() > 0 ? path.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = text[ i ];
        if ( IsUnreserved( c ) )
        {
            result.Append( &c, 1 );
        }
        else
        {
            const UInt8 byte = (UInt8) c;
            const char escape[ 3 ] = { '%', digits[ byte >> 4 ], digits[ byte & 15 ] };
            result.Append( escape, 3 );
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfUri::Decode( const CORE::CString& uri )
{GUCEF_TRACE;

    CORE::CString result;
    const char* text = uri.C_String();
    const UInt32 size = uri.ByteSize() > 0 ? uri.ByteSize() - 1 : 0;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '%' == text[ i ] && i+2 < size )
        {
            const Int32 high = HexValue( text[ i+1 ] );
            const Int32 low = HexValue( text[ i+2 ] );
            if ( high >= 0 && low >= 0 )
            {
                const char decoded = (char) ( ( high << 4 ) | low );
                result.Append( &decoded, 1 );
                i += 2;
                continue;
            }
        }
        result.Append( &text[ i ], 1 );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CGltfUri::IsDataUri( const CORE::CString& uri )
{GUCEF_TRACE;

    return uri.StartsWith( "data:" );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfUri::Resolve( const CORE::CString& folder, const CORE::CString& uri )
{GUCEF_TRACE;

    CORE::CString::StringVector parts;
    SplitParts( folder, parts );
    CORE::CString::StringVector uriParts;
    SplitParts( Decode( uri ), uriParts );
    for ( size_t i=0; i<uriParts.size(); ++i )
    {
        if ( "." == uriParts[ i ] )
            continue;
        if ( ".." == uriParts[ i ] )
        {
            if ( !parts.empty() )
                parts.pop_back();
            continue;
        }
        parts.push_back( uriParts[ i ] );
    }
    return JoinParts( parts );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGltfUri::MakeRelative( const CORE::CString& folder, const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString::StringVector from;
    CORE::CString::StringVector to;
    SplitParts( folder, from );
    SplitParts( path, to );
    size_t common = 0;
    while ( common < from.size() && common + 1 < to.size() && from[ common ] == to[ common ] )
        ++common;
    CORE::CString relative;
    for ( size_t i=common; i<from.size(); ++i )
        relative += "../";
    relative += JoinParts( to, common );
    return Encode( relative );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GLTF */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
