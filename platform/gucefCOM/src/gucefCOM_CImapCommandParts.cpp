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


#ifndef GUCEF_COM_CIMAPRESPONSE_H
#include "gucefCOM_CImapResponse.h"
#define GUCEF_COM_CIMAPRESPONSE_H
#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */

#include "gucefCOM_CImapCommandParts.h"

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

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CImapCommandParts::CImapCommandParts( void )
    : texts()
    , literals()
{GUCEF_TRACE;

    texts.push_back( CORE::CString() );
}

/*-------------------------------------------------------------------------*/

void
CImapCommandParts::Clear( void )
{GUCEF_TRACE;

    texts.clear();
    literals.clear();
    texts.push_back( CORE::CString() );
}

/*-------------------------------------------------------------------------*/

void
CImapCommandParts::AddWord( const char* text )
{GUCEF_TRACE;

    texts.back() += text;
}

/*-------------------------------------------------------------------------*/

bool
CImapCommandParts::Quote( const CORE::CString& text, CORE::CString& quoted )
{GUCEF_TRACE;

    const char* p = text.C_String();
    const UInt32 size = ByteLength( text );
    GUCEF::vector< char > out;
    out.push_back( '"' );
    for ( UInt32 i=0; i<size; ++i )
    {
        const UInt8 c = (UInt8) p[ i ];
        // only printable ASCII is quoted: anything else is a literal
        if ( c < 0x20 || c >= 0x7F )
            return false;
        if ( '"' == c || '\\' == c )
            out.push_back( '\\' );
        out.push_back( (char) c );
    }
    out.push_back( '"' );
    quoted = CORE::CString( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapCommandParts::AddString( const CORE::CString& utf8Text )
{GUCEF_TRACE;

    if ( CMimeCodec::HasLineBreakOrNul( utf8Text ) )
        return false;
    CORE::CString quoted;
    if ( Quote( utf8Text, quoted ) )
    {
        texts.back() += quoted;
        return true;
    }
    AddLiteral( utf8Text.C_String(), ByteLength( utf8Text ) );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImapCommandParts::AddLiteral( const void* data, UInt32 size )
{GUCEF_TRACE;

    texts.back() += "{" + CORE::UInt32ToString( size ) + "}";
    CORE::CDynamicBuffer literal;
    if ( size > 0 )
        literal.CopyFrom( size, data );
    literals.push_back( literal );
    texts.push_back( CORE::CString() );
}

/*-------------------------------------------------------------------------*/

bool
CImapCommandParts::AddMailbox( const CORE::CString& utf8Name )
{GUCEF_TRACE;

    CORE::CString wire;
    if ( !CImapMailboxName::Encode( utf8Name, wire ) )
        return false;
    CORE::CString quoted;
    if ( !Quote( wire, quoted ) )
        return false;
    texts.back() += quoted;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImapCommandParts::Append( const CImapCommandParts& other )
{GUCEF_TRACE;

    texts.back() += other.texts[ 0 ];
    for ( size_t i=0; i<other.literals.size(); ++i )
    {
        literals.push_back( other.literals[ i ] );
        texts.push_back( other.texts[ i + 1 ] );
    }
}

/*-------------------------------------------------------------------------*/

bool
CImapCommandParts::HasLiterals( void ) const
{GUCEF_TRACE;

    return !literals.empty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapCommandParts::ToLogText( void ) const
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<texts.size(); ++i )
        text += texts[ i ];
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CImapCommandParts::IsValidSequenceSet( const CORE::CString& set )
{GUCEF_TRACE;

    const char* p = set.C_String();
    const UInt32 size = ByteLength( set );
    if ( 0 == size || size > 1000 )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        if ( !( ( c >= '0' && c <= '9' ) || ':' == c || ',' == c || '*' == c ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
