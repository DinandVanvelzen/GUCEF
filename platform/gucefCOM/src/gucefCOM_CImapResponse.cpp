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

#ifndef GUCEF_COM_CIMAPVALUEPARSER_H
#include "gucefCOM_CImapValueParser.h"
#define GUCEF_COM_CIMAPVALUEPARSER_H
#endif /* GUCEF_COM_CIMAPVALUEPARSER_H ? */

#include "gucefCOM_CImapResponse.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
TrimSpaces( const CORE::CString& text )
{
    return text.Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

bool
IsStatusWord( const CORE::CString& word )
{
    return word.Equals( "OK", false ) || word.Equals( "NO", false ) || word.Equals( "BAD", false ) ||
           word.Equals( "PREAUTH", false ) || word.Equals( "BYE", false );
}

/*-------------------------------------------------------------------------*/

/** After the status word: [CODE arguments] text */
void
ParseStatusRest( CImapValueParser& parser, CImapResponse& response )
{
    parser.SkipSpaces();
    if ( '[' == parser.Peek() )
    {
        parser.Advance();
        CORE::CString inside;
        GUCEF::vector< char > chars;
        UInt32 depth = 0;
        while ( !parser.AtEnd() )
        {
            const char c = parser.Peek();
            if ( ']' == c && 0 == depth )
                break;
            if ( '(' == c ) ++depth;
            if ( ')' == c && depth > 0 ) --depth;
            chars.push_back( c );
            parser.Advance();
        }
        parser.Advance();       // the closing bracket
        if ( !chars.empty() )
            inside = CMimeCodec::SanitizeUtf8( &chars[ 0 ], (UInt32) chars.size() );
        Int32 space = inside.HasChar( ' ' );
        if ( space > 0 )
        {
            response.code = inside.SubstrToIndex( (UInt32) space, true ).Uppercase();
            response.codeArguments = TrimSpaces( inside.SubstrFromRange( (UInt32) space + 1, inside.Length() ) );
        }
        else
            response.code = inside.Uppercase();
    }
    response.text = TrimSpaces( parser.ReadRest() );
}

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapResponse::CImapResponse( void )
    : kind( KIND_NONE )
    , tag()
    , isStatus( false )
    , status()
    , code()
    , codeArguments()
    , text()
    , hasNumber( false )
    , number( 0 )
    , keyword()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapResponse::EndsWithLiteral( const CORE::CString& line, UInt32& literalSize, bool& nonSynchronizing )
{GUCEF_TRACE;

    const char* p = line.C_String();
    const UInt32 size = ByteLength( line );
    if ( size < 3 || '}' != p[ size - 1 ] )
        return false;
    UInt32 i = size - 2;
    nonSynchronizing = false;
    if ( '+' == p[ i ] )
    {
        nonSynchronizing = true;
        if ( 0 == i )
            return false;
        --i;
    }
    UInt32 digits = 0;
    UInt64 count = 0;
    UInt32 multiplier = 1;
    while ( p[ i ] >= '0' && p[ i ] <= '9' && digits < 11 )
    {
        count += (UInt64)( p[ i ] - '0' ) * multiplier;
        multiplier *= 10;
        ++digits;
        if ( 0 == i )
            return false;
        --i;
    }
    if ( 0 == digits || '{' != p[ i ] )
        return false;
    // a size above 32 bits is refused by the caller, which checks it against its limit
    literalSize = count > 0xFFFFFFFFull ? 0xFFFFFFFFu : (UInt32) count;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapResponse::Parse( const void* bytes, UInt32 size, UInt32 maxValues, CImapResponse& response, CORE::CString& error )
{GUCEF_TRACE;

    response = CImapResponse();
    error.Clear();
    CImapValueParser parser( bytes, size, CImapValueParser::DefaultMaxDepth, maxValues );

    if ( 0 == size )
    {
        error = "An empty response";
        return false;
    }
    const char* data = static_cast< const char* >( bytes );

    if ( '+' == data[ 0 ] )
    {
        response.kind = KIND_CONTINUATION;
        parser.Advance();
        response.text = TrimSpaces( parser.ReadRest() );
        return true;
    }

    if ( '*' == data[ 0 ] )
    {
        response.kind = KIND_UNTAGGED;
        parser.Advance();
        parser.SkipSpaces();

        CORE::CString word = parser.ReadWord();
        if ( word.IsNULLOrEmpty() )
        {
            error = "An untagged response without content";
            return false;
        }

        // "* 3 EXISTS"
        bool allDigits = true;
        for ( UInt32 i=0; i<ByteLength( word ); ++i )
            if ( word.C_String()[ i ] < '0' || word.C_String()[ i ] > '9' )
                allDigits = false;
        if ( allDigits )
        {
            if ( ByteLength( word ) > 10 )
            {
                error = "A number in a response is too large";
                return false;
            }
            UInt64 value = 0;
            for ( UInt32 i=0; i<ByteLength( word ); ++i )
                value = value * 10 + (UInt64)( word.C_String()[ i ] - '0' );
            if ( value > 0xFFFFFFFFull )
            {
                error = "A number in a response is too large";
                return false;
            }
            response.hasNumber = true;
            response.number = (UInt32) value;
            word = parser.ReadWord();
            if ( word.IsNULLOrEmpty() )
            {
                error = "An untagged response has no keyword";
                return false;
            }
        }

        response.keyword = word.Uppercase();
        if ( !response.hasNumber && IsStatusWord( response.keyword ) )
        {
            response.isStatus = true;
            response.status = response.keyword;
            ParseStatusRest( parser, response );
            return true;
        }
        if ( !parser.ParseValuesUntilEnd( response.data ) )
        {
            error = parser.GetError();
            return false;
        }
        return true;
    }

    // tagged: the tag, the status, the rest
    response.kind = KIND_TAGGED;
    response.tag = parser.ReadWord();
    CORE::CString status = parser.ReadWord();
    if ( response.tag.IsNULLOrEmpty() || !( status.Equals( "OK", false ) || status.Equals( "NO", false ) || status.Equals( "BAD", false ) ) )
    {
        error = "A response that is not tagged, untagged or a continuation";
        return false;
    }
    response.isStatus = true;
    response.status = status.Uppercase();
    ParseStatusRest( parser, response );
    return true;
}

/*-------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
