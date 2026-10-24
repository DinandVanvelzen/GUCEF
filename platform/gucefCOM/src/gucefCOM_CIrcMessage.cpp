/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#include "gucefCOM_CIrcMessage.h"

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

UInt32
ByteLength( const CORE::CString& str )
{GUCEF_TRACE;

    UInt32 byteSize = str.ByteSize();
    return byteSize > 0 ? byteSize - 1 : 0;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CIrcMessage::CIrcMessage( void )
    : CORE::CICloneable()
    , tags()
    , prefix()
    , command()
    , params()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessage::CIrcMessage( const CORE::CString& messageCommand )
    : CORE::CICloneable()
    , tags()
    , prefix()
    , command( messageCommand )
    , params()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessage::CIrcMessage( const CIrcMessage& src )
    : CORE::CICloneable( src )
    , tags( src.tags )
    , prefix( src.prefix )
    , command( src.command )
    , params( src.params )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessage::~CIrcMessage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CIrcMessage&
CIrcMessage::operator=( const CIrcMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        tags = src.tags;
        prefix = src.prefix;
        command = src.command;
        params = src.params;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CIrcMessage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CIrcMessage( *this );
}

/*-------------------------------------------------------------------------*/

void
CIrcMessage::Clear( void )
{GUCEF_TRACE;

    tags.clear();
    prefix.Clear();
    command.Clear();
    params.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcMessage::EscapeTagValue( const CORE::CString& value )
{GUCEF_TRACE;

    const char* str = value.C_String();
    UInt32 size = ByteLength( value );

    // Most values have nothing to escape
    UInt32 n = 0;
    while ( n < size && ';' != str[ n ] && ' ' != str[ n ] && '\\' != str[ n ] && '\r' != str[ n ] && '\n' != str[ n ] )
        ++n;
    if ( n == size )
        return value;

    CORE::CString escaped;
    for ( UInt32 i=0; i<size; ++i )
    {
        switch ( str[ i ] )
        {
            case ';':  escaped += "\\:"; break;
            case ' ':  escaped += "\\s"; break;
            case '\\': escaped += "\\\\"; break;
            case '\r': escaped += "\\r"; break;
            case '\n': escaped += "\\n"; break;
            default:   escaped += str[ i ]; break;
        }
    }
    return escaped;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcMessage::UnescapeTagValue( const CORE::CString& value )
{GUCEF_TRACE;

    const char* str = value.C_String();
    UInt32 size = ByteLength( value );

    // Most values have no escapes
    UInt32 n = 0;
    while ( n < size && '\\' != str[ n ] )
        ++n;
    if ( n == size )
        return value;

    CORE::CString unescaped;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '\\' != str[ i ] )
        {
            unescaped += str[ i ];
            continue;
        }

        // A backslash at the very end is dropped, an unknown escape is the character itself
        if ( ++i >= size )
            break;
        switch ( str[ i ] )
        {
            case ':':  unescaped += ';'; break;
            case 's':  unescaped += ' '; break;
            case '\\': unescaped += '\\'; break;
            case 'r':  unescaped += '\r'; break;
            case 'n':  unescaped += '\n'; break;
            default:   unescaped += str[ i ]; break;
        }
    }
    return unescaped;
}

/*-------------------------------------------------------------------------*/

bool
CIrcMessage::Parse( const CORE::CString& line )
{GUCEF_TRACE;

    Clear();

    const char* str = line.C_String();
    UInt32 size = ByteLength( line );
    while ( size > 0 && ( '\r' == str[ size-1 ] || '\n' == str[ size-1 ] ) )
        --size;

    UInt32 pos = 0;

    // The tags
    if ( pos < size && '@' == str[ pos ] )
    {
        ++pos;
        UInt32 tagsEnd = pos;
        while ( tagsEnd < size && ' ' != str[ tagsEnd ] )
            ++tagsEnd;

        UInt32 tagStart = pos;
        while ( tagStart < tagsEnd )
        {
            UInt32 tagEnd = tagStart;
            while ( tagEnd < tagsEnd && ';' != str[ tagEnd ] )
                ++tagEnd;
            if ( tagEnd > tagStart )
            {
                UInt32 equals = tagStart;
                while ( equals < tagEnd && '=' != str[ equals ] )
                    ++equals;

                TTag tag;
                tag.key.Set( str + tagStart, equals - tagStart );
                if ( equals + 1 < tagEnd )
                {
                    CORE::CString rawValue( str + equals + 1, tagEnd - equals - 1 );
                    tag.value = UnescapeTagValue( rawValue );
                }
                tags.push_back( tag );
            }
            tagStart = tagEnd + 1;
        }
        pos = tagsEnd;
    }

    while ( pos < size && ' ' == str[ pos ] )
        ++pos;

    // The prefix
    if ( pos < size && ':' == str[ pos ] )
    {
        ++pos;
        UInt32 prefixEnd = pos;
        while ( prefixEnd < size && ' ' != str[ prefixEnd ] )
            ++prefixEnd;
        prefix.Set( str + pos, prefixEnd - pos );
        pos = prefixEnd;
        while ( pos < size && ' ' == str[ pos ] )
            ++pos;
    }

    // The command
    UInt32 commandEnd = pos;
    while ( commandEnd < size && ' ' != str[ commandEnd ] )
        ++commandEnd;
    if ( commandEnd == pos )
        return false;
    command.Set( str + pos, commandEnd - pos );
    pos = commandEnd;

    // The parameters
    while ( pos < size )
    {
        while ( pos < size && ' ' == str[ pos ] )
            ++pos;
        if ( pos >= size )
            break;

        if ( ':' == str[ pos ] )
        {
            ++pos;
            params.push_back( CORE::CString() );
            if ( pos < size )
                params.back().Set( str + pos, size - pos );
            break;
        }

        UInt32 paramEnd = pos;
        while ( paramEnd < size && ' ' != str[ paramEnd ] )
            ++paramEnd;
        params.push_back( CORE::CString() );
        params.back().Set( str + pos, paramEnd - pos );
        pos = paramEnd;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcMessage::ToString( void ) const
{GUCEF_TRACE;

    CORE::CString line;
    if ( !tags.empty() )
    {
        line += '@';
        for ( size_t i=0; i<tags.size(); ++i )
        {
            if ( i > 0 )
                line += ';';
            line += tags[ i ].key;
            if ( !tags[ i ].value.IsNULLOrEmpty() )
            {
                line += '=';
                line += EscapeTagValue( tags[ i ].value );
            }
        }
        line += ' ';
    }
    if ( !prefix.IsNULLOrEmpty() )
    {
        line += ':';
        line += prefix;
        line += ' ';
    }
    line += command;
    for ( size_t i=0; i<params.size(); ++i )
    {
        line += ' ';
        const CORE::CString& param = params[ i ];
        bool isLast = i+1 == params.size();
        if ( isLast && ( param.IsNULLOrEmpty() || param.HasChar( ' ' ) >= 0 || param.StartsWith( ":" ) ) )
            line += ':';
        line += param;
    }
    return line;
}

/*-------------------------------------------------------------------------*/

CIrcMessage&
CIrcMessage::AddParam( const CORE::CString& param )
{GUCEF_TRACE;

    params.push_back( param );
    return *this;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CIrcMessage::GetParam( UInt32 index ) const
{GUCEF_TRACE;

    if ( index < params.size() )
        return params[ index ];
    return CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

UInt32
CIrcMessage::GetParamCount( void ) const
{GUCEF_TRACE;

    return (UInt32) params.size();
}

/*-------------------------------------------------------------------------*/

bool
CIrcMessage::HasTag( const CORE::CString& key ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<tags.size(); ++i )
        if ( tags[ i ].key == key )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CIrcMessage::GetTag( const CORE::CString& key, CORE::CString& value ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<tags.size(); ++i )
    {
        if ( tags[ i ].key == key )
        {
            value = tags[ i ].value;
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CIrcMessage::SetTag( const CORE::CString& key, const CORE::CString& value )
{GUCEF_TRACE;

    for ( size_t i=0; i<tags.size(); ++i )
    {
        if ( tags[ i ].key == key )
        {
            tags[ i ].value = value;
            return;
        }
    }
    TTag tag;
    tag.key = key;
    tag.value = value;
    tags.push_back( tag );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcMessage::GetSourceNick( void ) const
{GUCEF_TRACE;

    Int32 bang = prefix.HasChar( '!' );
    if ( bang >= 0 )
        return prefix.SubstrToIndex( (UInt32) bang, true );
    Int32 at = prefix.HasChar( '@' );
    if ( at >= 0 )
        return prefix.SubstrToIndex( (UInt32) at, true );
    return prefix;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcMessage::GetSourceUser( void ) const
{GUCEF_TRACE;

    Int32 bang = prefix.HasChar( '!' );
    if ( bang < 0 )
        return CORE::CString::Empty;
    CORE::CString rest = prefix.CutChars( (UInt32) bang + 1, true, 0 );
    Int32 at = rest.HasChar( '@' );
    return at >= 0 ? rest.SubstrToIndex( (UInt32) at, true ) : rest;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CIrcMessage::GetSourceHost( void ) const
{GUCEF_TRACE;

    Int32 at = prefix.HasChar( '@' );
    if ( at < 0 )
        return CORE::CString::Empty;
    return prefix.CutChars( (UInt32) at + 1, true, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CIrcMessage::IsNumeric( void ) const
{GUCEF_TRACE;

    if ( 3 != ByteLength( command ) )
        return false;
    const char* str = command.C_String();
    return str[ 0 ] >= '0' && str[ 0 ] <= '9' && str[ 1 ] >= '0' && str[ 1 ] <= '9' && str[ 2 ] >= '0' && str[ 2 ] <= '9';
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
