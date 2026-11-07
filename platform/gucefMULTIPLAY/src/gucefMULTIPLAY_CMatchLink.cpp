/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CMATCHID_H
#include "gucefMULTIPLAY_CMatchId.h"
#define GUCEF_MULTIPLAY_CMATCHID_H
#endif /* GUCEF_MULTIPLAY_CMATCHID_H ? */

#include "gucefMULTIPLAY_CMatchLink.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The characters a link may have outside of a query and fragment: letters, digits and a few marks. Nothing that needs decoding */
static bool
IsLinkChar( char c )
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
           '-' == c || '.' == c || '_' == c || '/' == c || ':' == c || '+' == c;
}

/*-------------------------------------------------------------------------*/

static bool
IsSchemeChar( char c )
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '+' == c || '-' == c || '.' == c;
}

/*-------------------------------------------------------------------------*/

static bool
IsHostChar( char c )
{GUCEF_TRACE;

    return ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '-' == c || '.' == c;
}

/*-------------------------------------------------------------------------*/

static bool
IsValidScheme( const CORE::CString& scheme )
{GUCEF_TRACE;

    if ( scheme.IsNULLOrEmpty() || scheme.Length() > 32 )
        return false;
    const char* p = scheme.C_String();
    if ( !( ( p[ 0 ] >= 'a' && p[ 0 ] <= 'z' ) || ( p[ 0 ] >= 'A' && p[ 0 ] <= 'Z' ) ) )
        return false;
    for ( UInt32 i=0; i<scheme.Length(); ++i )
        if ( !IsSchemeChar( p[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
IsValidHost( const CORE::CString& host )
{GUCEF_TRACE;

    if ( host.IsNULLOrEmpty() || host.Length() > 253 )
        return false;
    const char* p = host.C_String();
    for ( UInt32 i=0; i<host.Length(); ++i )
        if ( !IsHostChar( p[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMatchLink::CMatchLink( void )
    : m_customScheme()
    , m_customHost( "match" )
    , m_httpsHosts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchLink::CMatchLink( const CORE::CString& customScheme, const CORE::CString& httpsHost )
    : m_customScheme()
    , m_customHost( "match" )
    , m_httpsHosts()
{GUCEF_TRACE;

    SetCustomScheme( customScheme );
    AddHttpsHost( httpsHost );
}

/*-------------------------------------------------------------------------*/

CMatchLink::CMatchLink( const CMatchLink& src )
    : m_customScheme( src.m_customScheme )
    , m_customHost( src.m_customHost )
    , m_httpsHosts( src.m_httpsHosts )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchLink::~CMatchLink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMatchLink&
CMatchLink::operator=( const CMatchLink& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        m_customScheme = src.m_customScheme;
        m_customHost = src.m_customHost;
        m_httpsHosts = src.m_httpsHosts;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CMatchLink::SetCustomScheme( const CORE::CString& scheme )
{GUCEF_TRACE;

    m_customScheme = IsValidScheme( scheme ) ? scheme.Lowercase() : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

void
CMatchLink::SetCustomHost( const CORE::CString& host )
{GUCEF_TRACE;

    if ( IsValidHost( host ) )
        m_customHost = host.Lowercase();
}

/*-------------------------------------------------------------------------*/

void
CMatchLink::AddHttpsHost( const CORE::CString& host )
{GUCEF_TRACE;

    if ( IsValidHost( host ) )
        m_httpsHosts.push_back( host.Lowercase() );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchLink::GetCustomScheme( void ) const
{GUCEF_TRACE;

    return m_customScheme;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMatchLink::GetCustomHost( void ) const
{GUCEF_TRACE;

    return m_customHost;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchLink::MakeCustomLink( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    if ( m_customScheme.IsNULLOrEmpty() || !CMatchId::IsValid( matchId ) )
        return CORE::CString::Empty;
    return m_customScheme + "://" + m_customHost + "/" + matchId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMatchLink::MakeHttpsLink( const CORE::CString& matchId ) const
{GUCEF_TRACE;

    if ( m_httpsHosts.empty() || !CMatchId::IsValid( matchId ) )
        return CORE::CString::Empty;
    return "https://" + m_httpsHosts[ 0 ] + "/match/" + matchId;
}

/*-------------------------------------------------------------------------*/

bool
CMatchLink::Parse( const CORE::CString& link, CORE::CString& matchId ) const
{GUCEF_TRACE;

    matchId.Clear();
    if ( link.IsNULLOrEmpty() || link.Length() > MaxLinkLength )
        return false;

    // blanks around the link are the way it was handed over, none are allowed inside it
    const char* text = link.C_String();
    UInt32 begin = 0;
    UInt32 end = link.Length();
    while ( begin < end && ( ' ' == text[ begin ] || '\t' == text[ begin ] ) )
        ++begin;
    while ( end > begin && ( ' ' == text[ end - 1 ] || '\t' == text[ end - 1 ] || '\r' == text[ end - 1 ] || '\n' == text[ end - 1 ] ) )
        --end;

    // the query and fragment are not part of what is read, but they may only hold printable characters
    UInt32 pathEnd = end;
    for ( UInt32 i=begin; i<end; ++i )
    {
        if ( '?' == text[ i ] || '#' == text[ i ] )
        {
            pathEnd = i;
            break;
        }
    }
    for ( UInt32 i=begin; i<end; ++i )
    {
        unsigned char c = (unsigned char) text[ i ];
        if ( c <= 0x20 || c >= 0x7F || '%' == c || '\\' == c )
            return false;
    }
    for ( UInt32 i=begin; i<pathEnd; ++i )
        if ( !IsLinkChar( text[ i ] ) )
            return false;

    CORE::CString body( text + begin, pathEnd - begin );
    Int32 schemeEnd = body.HasSubstr( "://" );
    if ( schemeEnd <= 0 )
        return false;
    CORE::CString scheme = body.SubstrFromRange( 0, (UInt32) schemeEnd ).Lowercase();
    CORE::CString rest = body.SubstrFromRange( (UInt32) schemeEnd + 3, body.Length() );

    // the host is up to the first slash: no user info and no port
    Int32 hostEnd = rest.HasChar( '/' );
    if ( hostEnd <= 0 )
        return false;
    CORE::CString host = rest.SubstrFromRange( 0, (UInt32) hostEnd ).Lowercase();
    CORE::CString path = rest.SubstrFromRange( (UInt32) hostEnd, rest.Length() );
    if ( !IsValidHost( host ) )
        return false;
    if ( path.Length() > 1 && '/' == path[ path.Length() - 1 ] )
        path = path.SubstrFromRange( 0, path.Length() - 1 );

    CORE::CString candidate;
    if ( "https" == scheme )
    {
        bool knownHost = false;
        for ( size_t i=0; i<m_httpsHosts.size(); ++i )
            if ( m_httpsHosts[ i ] == host )
                knownHost = true;
        if ( !knownHost || path.Length() <= 7 || 0 != memcmp( path.C_String(), "/match/", 7 ) )
            return false;
        candidate = path.SubstrFromRange( 7, path.Length() );
    }
    else
    if ( !m_customScheme.IsNULLOrEmpty() && scheme == m_customScheme )
    {
        if ( host != m_customHost || path.Length() <= 1 )
            return false;
        candidate = path.SubstrFromRange( 1, path.Length() );
    }
    else
        return false;

    CORE::CString normalized = CMatchId::Normalize( candidate );
    if ( normalized.IsNULLOrEmpty() )
        return false;
    matchId = normalized;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
