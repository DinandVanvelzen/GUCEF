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


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
