/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefGUI_Utils.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
DecodeUtf8( const CString& text, TCodePointVector& codePoints )
{
    codePoints.clear();
    const UInt8* bytes = reinterpret_cast< const UInt8* >( text.C_String() );
    UInt32 size = text.ByteSize();
    if ( size > 0 && 0 == bytes[ size-1 ] )
        --size;     // ByteSize includes the terminator

    UInt32 i = 0;
    while ( i < size )
    {
        UInt8 lead = bytes[ i ];
        UInt32 cp = 0;
        UInt32 extra = 0;
        if ( lead < 0x80 )            { cp = lead; extra = 0; }
        else if ( 0xC0 == ( lead & 0xE0 ) ) { cp = lead & 0x1F; extra = 1; }
        else if ( 0xE0 == ( lead & 0xF0 ) ) { cp = lead & 0x0F; extra = 2; }
        else if ( 0xF0 == ( lead & 0xF8 ) ) { cp = lead & 0x07; extra = 3; }
        else
        {
            codePoints.push_back( 0xFFFD );
            ++i;
            continue;
        }

        bool valid = i + extra < size;
        for ( UInt32 n=1; valid && n<=extra; ++n )
        {
            UInt8 next = bytes[ i + n ];
            if ( 0x80 != ( next & 0xC0 ) )
                valid = false;
            else
                cp = ( cp << 6 ) | ( next & 0x3F );
        }
        if ( !valid )
        {
            codePoints.push_back( 0xFFFD );
            ++i;
            continue;
        }
        codePoints.push_back( cp );
        i += 1 + extra;
    }
}

/*-------------------------------------------------------------------------*/

CString
JoinStringList( const TStringVector& entries )
{
    // one allocation and one copy of every entry: appending to a string one entry at a time costs time in proportion to its length so far
    CString result;
    size_t total = 0;
    for ( size_t i=0; i<entries.size(); ++i )
        total += strlen( entries[ i ].C_String() ) + ( i > 0 ? 1 : 0 );
    if ( 0 == total )
        return result;
    char* buffer = result.Reserve( (UInt32) total + 1 );
    if ( GUCEF_NULL == buffer )
        return result;
    size_t offset = 0;
    for ( size_t i=0; i<entries.size(); ++i )
    {
        if ( i > 0 )
            buffer[ offset++ ] = '\n';
        const size_t size = strlen( entries[ i ].C_String() );
        memcpy( buffer + offset, entries[ i ].C_String(), size );
        offset += size;
    }
    buffer[ offset ] = '\0';
    result.DetermineLength();
    return result;
}

/*-------------------------------------------------------------------------*/

TStringVector
SplitStringList( const CString& value )
{
    TStringVector result;
    if ( value.IsNULLOrEmpty() )
        return result;
    CString::StringVector parts = value.ParseElements( '\n', true );
    for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
        result.push_back( parts[ i ] );
    return result;
}

/*-------------------------------------------------------------------------*/

GUCEF::vector< Int32 >
SplitIntList( const CString& value )
{
    GUCEF::vector< Int32 > result;
    if ( value.IsNULLOrEmpty() )
        return result;
    CString::StringVector parts = value.ParseElements( ',', false );
    for ( UInt32 i=0; i<(UInt32)parts.size(); ++i )
    {
        CString part = parts[ i ].Trim( true ).Trim( false );
        if ( part.IsNULLOrEmpty() )
            continue;
        char* end = GUCEF_NULL;
        long number = strtol( part.C_String(), &end, 10 );
        if ( '\0' == *end )
            result.push_back( (Int32) number );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

CString
JoinIntList( const GUCEF::vector< Int32 >& values )
{
    CString result;
    for ( UInt32 i=0; i<(UInt32)values.size(); ++i )
    {
        if ( i > 0 )
            result += ',';
        result += CORE::Int32ToString( values[ i ] );
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
