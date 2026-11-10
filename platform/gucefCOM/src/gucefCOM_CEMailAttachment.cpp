/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include <string.h>

#include "gucefCOM_CEMailAttachment.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEMailAttachment::CEMailAttachment( void )
    : fileName()
    , contentType( "application/octet-stream" )
    , contentId()
    , isInline( false )
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAttachment::CEMailAttachment( const CORE::CString& name, const CORE::CString& type, const void* content, UInt32 contentSize )
    : fileName( name )
    , contentType( type )
    , contentId()
    , isInline( false )
    , data()
{GUCEF_TRACE;

    if ( GUCEF_NULL != content && contentSize > 0 )
        data.CopyFrom( contentSize, content );
}

/*-------------------------------------------------------------------------*/

CEMailAttachment::CEMailAttachment( const CEMailAttachment& src )
    : fileName( src.fileName )
    , contentType( src.contentType )
    , contentId( src.contentId )
    , isInline( src.isInline )
    , data( src.data )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAttachment::~CEMailAttachment()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEMailAttachment&
CEMailAttachment::operator=( const CEMailAttachment& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        fileName = src.fileName;
        contentType = src.contentType;
        contentId = src.contentId;
        isInline = src.isInline;
        data = src.data;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailAttachment::GetSafeFileName( const CORE::CString& fallback ) const
{GUCEF_TRACE;

    return MakeSafeFileName( fileName, fallback );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEMailAttachment::MakeSafeFileName( const CORE::CString& name, const CORE::CString& fallback )
{GUCEF_TRACE;

    const char* p = name.C_String();
    UInt32 size = name.ByteSize() > 0 ? name.ByteSize() - 1 : 0;

    // only what follows the last separator of either kind
    UInt32 start = 0;
    for ( UInt32 i=0; i<size; ++i )
        if ( '/' == p[ i ] || '\\' == p[ i ] )
            start = i + 1;

    GUCEF::vector< char > clean;
    for ( UInt32 i=start; i<size && clean.size()<200; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        if ( c < 0x20 || 0x7F == c || NULL != strchr( ":*?\"<>|", (char) c ) )
            clean.push_back( '_' );
        else
            clean.push_back( (char) c );
    }
    // not in the middle of a UTF-8 sequence
    while ( !clean.empty() && start + clean.size() < size && 0x80 == ( ( (UInt8) p[ start + clean.size() ] ) & 0xC0 ) )
    {
        // the cut was inside a character: drop the lead byte and its continuation bytes
        while ( !clean.empty() && 0x80 == ( ( (UInt8) clean.back() ) & 0xC0 ) )
            clean.pop_back();
        if ( !clean.empty() )
            clean.pop_back();
        break;
    }

    // leading and trailing dots and spaces
    size_t first = 0;
    size_t last = clean.size();
    while ( first < last && ( '.' == clean[ first ] || ' ' == clean[ first ] ) )
        ++first;
    while ( last > first && ( '.' == clean[ last - 1 ] || ' ' == clean[ last - 1 ] ) )
        --last;
    if ( first >= last )
        return fallback;

    CORE::CString result( &clean[ first ], (UInt32)( last - first ) );

    // device names of Windows, with or without extension
    CORE::CString stem = result;
    Int32 dot = stem.HasChar( '.' );
    if ( dot > 0 )
        stem = stem.SubstrToIndex( (UInt32) dot, true );
    CORE::CString upper = stem.Uppercase();
    bool reserved = upper == "CON" || upper == "PRN" || upper == "AUX" || upper == "NUL";
    if ( !reserved && upper.Length() == 4 && ( upper.SubstrToIndex( 3, true ) == "COM" || upper.SubstrToIndex( 3, true ) == "LPT" ) )
        reserved = upper.C_String()[ 3 ] >= '0' && upper.C_String()[ 3 ] <= '9';
    if ( reserved )
        result = "_" + result;
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
