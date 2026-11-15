/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefWEB_CWebViewHostAllowList.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A substring by byte positions, [start, end) */
CString
Bytes( const char* text, UInt32 start, UInt32 end )
{
    CString result;
    if ( end > start )
        result.Set( text + start, end - start );
    return result;
}

bool
IsSchemeChar( char c, bool first )
{
    if ( c >= 'a' && c <= 'z' )
        return true;
    if ( first )
        return false;
    return ( c >= '0' && c <= '9' ) || '+' == c || '-' == c || '.' == c;
}

bool
ParsePort( const CString& text, UInt16& port )
{
    const char* c = text.C_String();
    const UInt32 count = text.IsNULLOrEmpty() ? 0 : text.ByteSize() - 1;
    if ( 0 == count || count > 5 )
        return false;
    UInt32 value = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( c[ i ] < '0' || c[ i ] > '9' )
            return false;
        value = value * 10 + (UInt32) ( c[ i ] - '0' );
    }
    if ( 0 == value || value > 65535 )
        return false;
    port = (UInt16) value;
    return true;
}

}

/*-------------------------------------------------------------------------*/

CWebViewHostAllowList::SEntry::SEntry( void )
    : scheme()
    , host()
    , includeSubdomains( false )
    , port( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewHostAllowList::CWebViewHostAllowList( void )
    : CORE::CIConfigurable()
    , m_entries()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewHostAllowList::~CWebViewHostAllowList()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt16
CWebViewHostAllowList::GetDefaultPort( const CString& scheme )
{GUCEF_TRACE;

    if ( "http" == scheme || "ws" == scheme )
        return 80;
    if ( "https" == scheme || "wss" == scheme )
        return 443;
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::IsIpLiteral( const CString& host )
{GUCEF_TRACE;

    if ( host.IsNULLOrEmpty() )
        return false;
    const char* c = host.C_String();
    if ( '[' == c[ 0 ] )
        return true;
    // dotted IPv4: digits and dots only
    for ( ; '\0' != *c; ++c )
        if ( !( ( *c >= '0' && *c <= '9' ) || '.' == *c ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::NormaliseHost( CString& host )
{GUCEF_TRACE;

    if ( host.IsNULLOrEmpty() )
        return false;
    const char* c = host.C_String();
    UInt32 count = host.ByteSize() - 1;

    if ( '[' == c[ 0 ] )
    {
        // an IPv6 literal: hex digits, ':' and '.' (an embedded IPv4) between brackets
        if ( count < 3 || ']' != c[ count - 1 ] )
            return false;
        GUCEF::vector< char > out;
        out.reserve( count );
        for ( UInt32 i=0; i<count; ++i )
        {
            char ch = c[ i ];
            if ( ch >= 'A' && ch <= 'F' )
                ch = (char) ( ch - 'A' + 'a' );
            const bool inside = i > 0 && i + 1 < count;
            if ( inside && !( ( ch >= '0' && ch <= '9' ) || ( ch >= 'a' && ch <= 'f' ) || ':' == ch || '.' == ch ) )
                return false;
            out.push_back( ch );
        }
        host.Set( &out[ 0 ], (UInt32) out.size() );
        return true;
    }

    // one trailing dot is the root of the name and means the same host
    if ( count > 0 && '.' == c[ count - 1 ] )
        --count;
    if ( 0 == count )
        return false;
    GUCEF::vector< char > out;
    out.reserve( count );
    char previous = '.';
    for ( UInt32 i=0; i<count; ++i )
    {
        char ch = c[ i ];
        if ( (unsigned char) ch >= 0x80 )
            return false;
        if ( ch >= 'A' && ch <= 'Z' )
            ch = (char) ( ch - 'A' + 'a' );
        const bool valid = ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) || '-' == ch || '.' == ch || '_' == ch;
        if ( !valid )
            return false;
        if ( '.' == ch && '.' == previous )
            return false;           // an empty label, also a leading dot
        out.push_back( ch );
        previous = ch;
    }
    host.Set( &out[ 0 ], (UInt32) out.size() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::SplitHostAndPort( const CString& authority, CString& host, UInt16& port, bool& hasPort )
{GUCEF_TRACE;

    hasPort = false;
    port = 0;
    if ( authority.IsNULLOrEmpty() )
        return false;
    const char* c = authority.C_String();
    const UInt32 count = authority.ByteSize() - 1;
    UInt32 hostEnd = count;
    if ( '[' == c[ 0 ] )
    {
        UInt32 close = 0;
        while ( close < count && ']' != c[ close ] )
            ++close;
        if ( close >= count )
            return false;
        hostEnd = close + 1;
        if ( hostEnd < count && ':' != c[ hostEnd ] )
            return false;
    }
    else
    {
        for ( UInt32 i=0; i<count; ++i )
            if ( ':' == c[ i ] )
            {
                hostEnd = i;
                break;
            }
    }
    host = Bytes( c, 0, hostEnd );
    if ( hostEnd < count )
    {
        if ( !ParsePort( Bytes( c, hostEnd + 1, count ), port ) )
            return false;
        hasPort = true;
    }
    return NormaliseHost( host );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::SplitUrl( const CString& url, CString& scheme, CString& host, UInt16& port, bool& hasUserInfo )
{GUCEF_TRACE;

    hasUserInfo = false;
    port = 0;
    if ( url.IsNULLOrEmpty() )
        return false;
    const char* c = url.C_String();
    const UInt32 count = url.ByteSize() - 1;

    // the scheme, made lower case, up to "://"
    UInt32 i = 0;
    GUCEF::vector< char > schemeChars;
    while ( i < count && ':' != c[ i ] )
    {
        char ch = c[ i ];
        if ( ch >= 'A' && ch <= 'Z' )
            ch = (char) ( ch - 'A' + 'a' );
        if ( !IsSchemeChar( ch, 0 == i ) )
            return false;
        schemeChars.push_back( ch );
        ++i;
    }
    if ( schemeChars.empty() || i + 2 >= count || '/' != c[ i + 1 ] || '/' != c[ i + 2 ] )
        return false;
    scheme.Set( &schemeChars[ 0 ], (UInt32) schemeChars.size() );
    i += 3;

    // the authority ends at the path, the query or the fragment
    UInt32 end = i;
    while ( end < count && '/' != c[ end ] && '?' != c[ end ] && '#' != c[ end ] && '\\' != c[ end ] )
        ++end;
    CString authority = Bytes( c, i, end );
    const Int32 at = authority.HasChar( '@', false );
    if ( at >= 0 )
    {
        hasUserInfo = true;
        authority = Bytes( authority.C_String(), (UInt32) at + 1, authority.ByteSize() - 1 );
    }
    bool hasPort = false;
    if ( !SplitHostAndPort( authority, host, port, hasPort ) )
        return false;
    if ( !hasPort )
        port = GetDefaultPort( scheme );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::ParsePattern( const CString& pattern, TEntry& entry )
{GUCEF_TRACE;

    CString text = pattern.Trim( true ).Trim( false );
    if ( text.IsNULLOrEmpty() || text.HasChar( '*' ) >= 0 || text.HasChar( '@' ) >= 0 || text.HasChar( '?' ) >= 0 || text.HasChar( '#' ) >= 0 )
        return false;

    entry = TEntry();
    const Int32 schemeEnd = text.HasSubstr( "://" );
    if ( schemeEnd >= 0 )
    {
        entry.scheme = text.SubstrToIndex( (UInt32) schemeEnd, true ).Lowercase();
        if ( "http" != entry.scheme && "https" != entry.scheme )
            return false;
        text = text.CutChars( (UInt32) schemeEnd + 3, true );
        // a trailing slash is allowed, a path is not
        if ( text.EndsWith( "/" ) )
            text = text.CutChars( 1, false );
    }
    if ( text.HasChar( '/' ) >= 0 || text.HasChar( '\\' ) >= 0 )
        return false;
    if ( text.StartsWith( "." ) )
    {
        entry.includeSubdomains = true;
        text = text.CutChars( 1, true );
    }
    bool hasPort = false;
    UInt16 port = 0;
    CString host;
    if ( !SplitHostAndPort( text, host, port, hasPort ) )
        return false;
    if ( entry.includeSubdomains && IsIpLiteral( host ) )
        return false;
    entry.host = host;
    entry.port = hasPort ? port : 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::AddEntry( const CString& pattern )
{GUCEF_TRACE;

    TEntry entry;
    if ( !ParsePattern( pattern, entry ) )
        return false;
    m_entries.push_back( entry );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::AddEntries( const CString::StringVector& patterns )
{GUCEF_TRACE;

    bool allValid = true;
    for ( CString::StringVector::const_iterator i=patterns.begin(); i!=patterns.end(); ++i )
        if ( !AddEntry( (*i) ) )
            allValid = false;
    return allValid;
}

/*-------------------------------------------------------------------------*/

void
CWebViewHostAllowList::Clear( void )
{GUCEF_TRACE;

    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::IsEmpty( void ) const
{GUCEF_TRACE;

    return m_entries.empty();
}

/*-------------------------------------------------------------------------*/

const CWebViewHostAllowList::TEntryVector&
CWebViewHostAllowList::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

CString::StringVector
CWebViewHostAllowList::GetPatterns( void ) const
{GUCEF_TRACE;

    CString::StringVector patterns;
    for ( TEntryVector::const_iterator i=m_entries.begin(); i!=m_entries.end(); ++i )
    {
        const TEntry& entry = (*i);
        CString pattern;
        if ( !entry.scheme.IsNULLOrEmpty() )
            pattern = entry.scheme + "://";
        if ( entry.includeSubdomains )
            pattern += ".";
        pattern += entry.host;
        if ( 0 != entry.port )
            pattern += ":" + CORE::UInt32ToString( entry.port );
        patterns.push_back( pattern );
    }
    return patterns;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::IsHostAllowed( const CString& scheme, const CString& host, UInt16 port ) const
{GUCEF_TRACE;

    if ( "http" != scheme && "https" != scheme )
        return false;
    const UInt16 effectivePort = 0 == port ? GetDefaultPort( scheme ) : port;
    for ( TEntryVector::const_iterator i=m_entries.begin(); i!=m_entries.end(); ++i )
    {
        const TEntry& entry = (*i);
        if ( !entry.scheme.IsNULLOrEmpty() && entry.scheme != scheme )
            continue;
        const UInt16 entryPort = 0 == entry.port ? GetDefaultPort( scheme ) : entry.port;
        if ( entryPort != effectivePort )
            continue;
        if ( host == entry.host )
            return true;
        if ( entry.includeSubdomains && host.EndsWith( "." + entry.host ) )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::IsUrlAllowed( const CString& url ) const
{GUCEF_TRACE;

    CString scheme, host;
    UInt16 port = 0;
    bool hasUserInfo = false;
    if ( !SplitUrl( url, scheme, host, port, hasUserInfo ) || hasUserInfo )
        return false;
    return IsHostAllowed( scheme, host, port );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    CString::StringVector patterns = GetPatterns();
    return config.SetAttribute( "hosts", CString::Empty.Combine( patterns, ',' ) );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewHostAllowList::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_entries.clear();
    CString::StringVector patterns = config.GetAttributeValueOrChildValueByName( "hosts" ).AsString().ParseElements( ',', false );
    CORE::CDataNode::TVariantVector children = config.GetChildrenValuesByName( "host" );
    for ( CORE::CDataNode::TVariantVector::iterator i=children.begin(); i!=children.end(); ++i )
        patterns.push_back( (*i).AsString() );
    for ( CString::StringVector::iterator i=patterns.begin(); i!=patterns.end(); ++i )
    {
        const CString pattern = (*i).Trim( true ).Trim( false );
        if ( pattern.IsNULLOrEmpty() )
            continue;
        if ( !AddEntry( pattern ) )
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebViewHostAllowList: The host pattern \"" << pattern << "\" is not valid and is left out" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewHostAllowList::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewHostAllowList";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
