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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#ifndef GUCEF_COM_CIMAPRESPONSE_H
#include "gucefCOM_CImapResponse.h"
#define GUCEF_COM_CIMAPRESPONSE_H
#endif /* GUCEF_COM_CIMAPRESPONSE_H ? */

#include "gucefCOM_CImapSearchCriteria.h"

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

/*-------------------------------------------------------------------------*/

CImapSearchCriteria::CImapSearchCriteria( void )
    : m_criteria()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

#define GUCEF_COM_NEW_CRITERION( kindValue ) \
    SCriterion c; c.kind = kindValue; c.first = 0; c.last = 0; c.year = 0; c.month = 0; c.day = 0;

void CImapSearchCriteria::AddUnseen( void ) { GUCEF_COM_NEW_CRITERION( KIND_UNSEEN ) m_criteria.push_back( c ); }
void CImapSearchCriteria::AddSeen( void ) { GUCEF_COM_NEW_CRITERION( KIND_SEEN ) m_criteria.push_back( c ); }
void CImapSearchCriteria::AddFlagged( void ) { GUCEF_COM_NEW_CRITERION( KIND_FLAGGED ) m_criteria.push_back( c ); }
void CImapSearchCriteria::AddDeleted( void ) { GUCEF_COM_NEW_CRITERION( KIND_DELETED ) m_criteria.push_back( c ); }
void CImapSearchCriteria::AddUidRange( UInt32 first, UInt32 last ) { GUCEF_COM_NEW_CRITERION( KIND_UID ) c.first = first; c.last = last; m_criteria.push_back( c ); }
void CImapSearchCriteria::AddHeader( const CORE::CString& name, const CORE::CString& text ) { GUCEF_COM_NEW_CRITERION( KIND_HEADER ) c.name = name; c.text = text; m_criteria.push_back( c ); }
void CImapSearchCriteria::AddSubject( const CORE::CString& text ) { GUCEF_COM_NEW_CRITERION( KIND_SUBJECT ) c.text = text; m_criteria.push_back( c ); }
void CImapSearchCriteria::AddFrom( const CORE::CString& text ) { GUCEF_COM_NEW_CRITERION( KIND_FROM ) c.text = text; m_criteria.push_back( c ); }
void CImapSearchCriteria::AddSince( Int32 year, Int32 month, Int32 day ) { GUCEF_COM_NEW_CRITERION( KIND_SINCE ) c.year = year; c.month = month; c.day = day; m_criteria.push_back( c ); }

#undef GUCEF_COM_NEW_CRITERION

/*-------------------------------------------------------------------------*/

bool
CImapSearchCriteria::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_criteria.empty();
}

/*-------------------------------------------------------------------------*/

bool
CImapSearchCriteria::Build( CImapCommandParts& parts, CORE::CString& error ) const
{GUCEF_TRACE;

    parts.Clear();
    if ( m_criteria.empty() )
    {
        parts.AddWord( "ALL" );
        return true;
    }

    // a literal in a search needs the character set to be said
    bool needsCharset = false;
    for ( size_t i=0; i<m_criteria.size(); ++i )
    {
        CORE::CString quoted;
        if ( !m_criteria[ i ].text.IsNULLOrEmpty() && !CImapCommandParts::Quote( m_criteria[ i ].text, quoted ) )
            needsCharset = true;
    }
    if ( needsCharset )
        parts.AddWord( "CHARSET UTF-8 " );

    static const char* months[ 12 ] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    for ( size_t i=0; i<m_criteria.size(); ++i )
    {
        const SCriterion& c = m_criteria[ i ];
        if ( i > 0 )
            parts.AddWord( " " );
        switch ( c.kind )
        {
            case KIND_UNSEEN: parts.AddWord( "UNSEEN" ); break;
            case KIND_SEEN: parts.AddWord( "SEEN" ); break;
            case KIND_FLAGGED: parts.AddWord( "FLAGGED" ); break;
            case KIND_DELETED: parts.AddWord( "DELETED" ); break;
            case KIND_UID:
            {
                parts.AddWord( ( "UID " + CORE::UInt32ToString( c.first ) + ":" + ( 0 == c.last ? CORE::CString( "*" ) : CORE::UInt32ToString( c.last ) ) ).C_String() );
                break;
            }
            case KIND_HEADER:
            {
                if ( !CMimeCodec::IsValidHeaderName( c.name ) || !CMimeCodec::IsAscii( c.name.C_String(), ByteLength( c.name ) ) )
                {
                    error = "A header name in the search is not valid";
                    return false;
                }
                parts.AddWord( "HEADER " );
                if ( !parts.AddString( c.name ) )
                {
                    error = "A header name in the search is not valid";
                    return false;
                }
                parts.AddWord( " " );
                if ( !parts.AddString( c.text ) )
                {
                    error = "A text in the search has a line break";
                    return false;
                }
                break;
            }
            case KIND_SUBJECT:
            case KIND_FROM:
            {
                parts.AddWord( KIND_SUBJECT == c.kind ? "SUBJECT " : "FROM " );
                if ( !parts.AddString( c.text ) )
                {
                    error = "A text in the search has a line break";
                    return false;
                }
                break;
            }
            case KIND_SINCE:
            {
                if ( c.month < 1 || c.month > 12 || c.day < 1 || c.day > 31 || c.year < 1970 || c.year > 9999 )
                {
                    error = "The date of the search is not valid";
                    return false;
                }
                parts.AddWord( ( "SINCE " + CORE::Int32ToString( c.day ) + "-" + months[ c.month - 1 ] + "-" + CORE::Int32ToString( c.year ) ).C_String() );
                break;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CImapFetchSpec::CImapFetchSpec( void )
    : uid( true )
    , flags( false )
    , internalDate( false )
    , size( false )
    , envelope( false )
    , bodyStructure( false )
    , sections()
    , markSeen( false )
    , partialOffset( 0 )
    , partialLength( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CImapFetchSpec::Build( CORE::CString& items ) const
{GUCEF_TRACE;

    CORE::CString text;
    #define GUCEF_COM_ADD_ITEM( condition, name ) if ( condition ) { if ( !text.IsNULLOrEmpty() ) text += " "; text += name; }
    GUCEF_COM_ADD_ITEM( uid, "UID" )
    GUCEF_COM_ADD_ITEM( flags, "FLAGS" )
    GUCEF_COM_ADD_ITEM( internalDate, "INTERNALDATE" )
    GUCEF_COM_ADD_ITEM( size, "RFC822.SIZE" )
    GUCEF_COM_ADD_ITEM( envelope, "ENVELOPE" )
    GUCEF_COM_ADD_ITEM( bodyStructure, "BODYSTRUCTURE" )
    #undef GUCEF_COM_ADD_ITEM

    for ( size_t i=0; i<sections.size(); ++i )
    {
        // a section is letters, digits, dots, spaces, parentheses and header names: no brackets, quotes or line breaks
        const char* s = sections[ i ].C_String();
        for ( UInt32 k=0; k<ByteLength( sections[ i ] ); ++k )
        {
            const char c = s[ k ];
            const bool ok = ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) || ( c >= '0' && c <= '9' ) || '.' == c || ' ' == c || '(' == c || ')' == c || '-' == c || '_' == c;
            if ( !ok )
                return false;
        }
        if ( !text.IsNULLOrEmpty() )
            text += " ";
        text += markSeen ? "BODY[" : "BODY.PEEK[";
        text += sections[ i ] + "]";
        if ( 0 == i && partialLength > 0 )
            text += "<" + CORE::UInt32ToString( partialOffset ) + "." + CORE::UInt32ToString( partialLength ) + ">";
    }
    if ( text.IsNULLOrEmpty() )
        return false;
    items = text;
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
