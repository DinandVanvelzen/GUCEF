/*
 *  AncestryDotComBackupLib: logic of the Ancestry.com tree backup tool
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

#ifndef ANCESTRYBACKUP_CURLTEMPLATE_H
#include "AncestryBackup_CUrlTemplate.h"
#define ANCESTRYBACKUP_CURLTEMPLATE_H
#endif /* ANCESTRYBACKUP_CURLTEMPLATE_H ? */

#ifndef ANCESTRYBACKUP_CENDPOINTCATALOG_H
#include "AncestryBackup_CEndpointCatalog.h"
#define ANCESTRYBACKUP_CENDPOINTCATALOG_H
#endif /* ANCESTRYBACKUP_CENDPOINTCATALOG_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#include <algorithm>

#include "AncestryBackup_CRequestPolicy.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRequestPolicy::CRequestPolicy( void )
    : m_treeId()
    , m_dnaAllowed( false )
    , m_exportStartAllowed( false )
    , m_dnaPostsEnabled( false )
    , m_requireHttps( true )
    , m_hostSuffixes()
    , m_allowed()
    , m_allowedPosts()
{GUCEF_TRACE;

    m_hostSuffixes.push_back( "ancestry.com" );
    m_hostSuffixes.push_back( "ancestrycdn.com" );
}

/*-------------------------------------------------------------------------*/

void
CRequestPolicy::SetHostSuffixesForTesting( const std::vector< std::string >& suffixes, bool requireHttps )
{GUCEF_TRACE;

    m_hostSuffixes = suffixes;
    m_requireHttps = requireHttps;
}

/*-------------------------------------------------------------------------*/

bool
CRequestPolicy::SetTreeId( const std::string& treeId )
{GUCEF_TRACE;

    m_treeId.clear();
    if ( treeId.empty() || treeId.find_first_not_of( "0123456789" ) != std::string::npos )
        return false;
    m_treeId = treeId;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRequestPolicy::SetDnaAllowed( bool allowed )
{GUCEF_TRACE;

    m_dnaAllowed = allowed;
}

/*-------------------------------------------------------------------------*/

void
CRequestPolicy::SetExportStartAllowed( bool allowed )
{GUCEF_TRACE;

    m_exportStartAllowed = allowed;
}

/*-------------------------------------------------------------------------*/

void
CRequestPolicy::AddAllowed( const std::string& pathTemplate )
{GUCEF_TRACE;

    if ( !pathTemplate.empty() && std::find( m_allowed.begin(), m_allowed.end(), pathTemplate ) == m_allowed.end() )
        m_allowed.push_back( pathTemplate );
}

/*-------------------------------------------------------------------------*/

size_t
CRequestPolicy::GetAllowedCount( void ) const
{GUCEF_TRACE;

    return m_allowed.size();
}

/*-------------------------------------------------------------------------*/

void
CRequestPolicy::AddDefaultAllowed( void )
{GUCEF_TRACE;

    static const char* const defaults[] = {
        // the tree and its people
        "www.ancestry.com/api/treesui-list/trees/{tree}/persons",
        "www.ancestry.com/api/treesui-list/trees/{tree}/persons/count",
        "www.ancestry.com/api/treesui-list/trees/{tree}/personscount",
        "www.ancestry.com/api/treesui-list/trees/{tree}/rootperson",
        "www.ancestry.com/api/treesui-list/trees/{tree}/getTreeName",
        "www.ancestry.com/api/treeviewer/tree/newfamilyview/{tree}",
        "www.ancestry.com/api/treeviewer/tree/{tree}/info",
        // one person
        "www.ancestry.com/family-tree/person/facts/user/{guid}/tree/{tree}/person/{n}/factsglue",
        "www.ancestry.com/family-tree/person/lifestory/user/{guid}/tree/{tree}/person/{n}/lifestoryglue",
        "www.ancestry.com/family-tree/person/factedit/user/{guid}/tree/{tree}/person/{n}/source/{n}",
        "www.ancestry.com/family-tree/person/facts/user/{guid}/tree/{tree}/person/{n}/getfamilymembersourcedata",
        "www.ancestry.com/api/treeviewer/comments/tree/{tree}",
        "www.ancestry.com/api/treeviewer/tree/{tree}/person/{n}/photoCount",
        // what changed
        "www.ancestry.com/tree-change-event/api/change-events",
        // media
        "www.ancestry.com/api/media/viewer/v1/trees/{tree}",
        "www.ancestry.com/api/media/viewer/v1/trees/{tree}/people/{n}",
        "www.ancestry.com/api/media/viewer/v1/trees/{tree}/{guid}/personids",
        "www.ancestry.com/api/media/viewer/v2/trees/{tree}/media",
        "www.ancestry.com/api/media/viewer/v2/trees/{tree}/albums",
        "www.ancestry.com/api/media/viewer/v2/trees/{tree}/albums/count",
        "www.ancestry.com/api/media/viewer/v2/trees/{tree}/stories/list",
        "www.ancestry.com/api/media/viewer/api/media-use/{guid}/metadata",
        "www.ancestry.com/api/media/viewer/api/media/{guid}/attributes",
        "www.ancestry.com/api/media/viewer/api/trees/{tree}/media/{guid}/users",
        "www.ancestry.com/api/media/retrieval/v2/image/namespaces/{n}/media/{guid}",
        "www.ancestry.com/api/media/retrieval/v2/image/namespaces/{n}/media/{guid}.jpg",
        "www.ancestry.com/api/media/retrieval/v2/image/namespaces/{n}/media/*",
        "www.ancestry.com/comments/initWidget",
        // records that are attached to people
        "www.ancestry.com/api/hintsui-api/collection/{n}/record/{n}",
        "www.ancestry.com/api/hintsui-api/collection/{n}/metadata",
        // the GEDCOM export: its state, and its download
        "www.ancestry.com/api/treesui-list/trees/{tree}/export",
        "www.ancestry.com/api/media/retrieval/v2/stream/namespaces/{n}/media/{guid}.ged",
        GUCEF_NULL };
    for ( UInt32 i=0; defaults[ i ] != GUCEF_NULL; ++i )
        AddAllowed( defaults[ i ] );
}

/*-------------------------------------------------------------------------*/

void
CRequestPolicy::AddDnaAllowed( void )
{GUCEF_TRACE;

    m_dnaPostsEnabled = true;

    // reading the DNA matches. The sample id is the id of the DNA test of the user
    static const char* const gets[] = {
        "www.ancestry.com/api/dna/matches/*/matches",
        "www.ancestry.com/api/dna/matches/*/matches/count",
        "www.ancestry.com/api/dna/matches/*/tags/custom",
        "www.ancestry.com/api/dna/matches/*/trees/info",
        "www.ancestry.com/api/dna/matches/*/user/profile",
        "www.ancestry.com/api/dna/matches/*/parents/paternalCluster",
        "www.ancestry.com/api/dna/matches/*/matches/with/*/shared",
        "www.ancestry.com/api/dna/matches/*/relationships/sharedDna/*",
        "www.ancestry.com/api/dna/matches/*/map/locations",
        "www.ancestry.com/api/dna/matches/*/commonAncestors/compare/*/data",
        "www.ancestry.com/dna/insights/api/dnaReport/regions/*",
        "www.ancestry.com/dna/insights/api/dnaReport/journeys/*",
        "www.ancestry.com/dna/insights/api/dnaReport/traits/*",
        "www.ancestry.com/api/uhome/secure/rest/user/dna/summary",
        GUCEF_NULL };
    for ( UInt32 i=0; gets[ i ] != GUCEF_NULL; ++i )
        AddAllowed( gets[ i ] );

    // and the POST requests that read
    static const char* const posts[] = {
        "www.ancestry.com/api/dna/matches/*/relationships/data",
        "www.ancestry.com/api/dna/matches/*/clusters/profileData",
        "www.ancestry.com/api/dna/matches/*/commonAncestors",
        "www.ancestry.com/api/dna/matches/*/trees/inTree",
        GUCEF_NULL };
    for ( UInt32 i=0; posts[ i ] != GUCEF_NULL; ++i )
        if ( std::find( m_allowedPosts.begin(), m_allowedPosts.end(), posts[ i ] ) == m_allowedPosts.end() )
            m_allowedPosts.push_back( posts[ i ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CRequestPolicy::LoadAllowedFile( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !CORE::FileExists( path ) )
        return 0;
    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return 0;
    std::string content( (size_t) file.GetSize(), '\0' );
    if ( !content.empty() && file.Read( &content[ 0 ], 1, (UInt32) content.size() ) != content.size() )
        return 0;

    UInt32 added = 0;
    size_t start = 0;
    while ( start < content.size() )
    {
        size_t end = content.find_first_of( "\r\n", start );
        if ( end == std::string::npos )
            end = content.size();
        std::string line = content.substr( start, end - start );
        start = end + 1;
        const size_t b = line.find_first_not_of( " \t" );
        const size_t e = line.find_last_not_of( " \t" );
        if ( b == std::string::npos || line[ b ] == '#' )
            continue;
        line = line.substr( b, e - b + 1 );
        const size_t before = m_allowed.size();
        AddAllowed( line );
        added += (UInt32) ( m_allowed.size() - before );
    }
    return added;
}

/*-------------------------------------------------------------------------*/

static std::vector< std::string >
Split( const std::string& text, char separator )
{
    std::vector< std::string > parts;
    size_t start = 0;
    while ( start <= text.size() )
    {
        size_t end = text.find( separator, start );
        if ( end == std::string::npos )
            end = text.size();
        parts.push_back( text.substr( start, end - start ) );
        start = end + 1;
    }
    return parts;
}

/*-------------------------------------------------------------------------*/

bool
CRequestPolicy::MatchesAllowed( const std::string& hostAndPathTemplate, const std::string& hostAndRawPath, bool forPost ) const
{GUCEF_TRACE;

    const std::vector< std::string > wanted = Split( hostAndPathTemplate, '/' );
    const std::vector< std::string > raw = Split( hostAndRawPath, '/' );
    const std::vector< std::string >& list = forPost ? m_allowedPosts : m_allowed;
    for ( size_t a=0; a<list.size(); ++a )
    {
        const std::vector< std::string > allowed = Split( list[ a ], '/' );
        if ( allowed.size() != wanted.size() )
            continue;
        bool same = true;
        for ( size_t i=0; i<allowed.size() && same; ++i )
        {
            if ( allowed[ i ] == "{tree}" )
                same = !m_treeId.empty() && i < raw.size() && raw[ i ] == m_treeId;     // the tree this is set up for, and no other
            else
                same = allowed[ i ] == "*" || allowed[ i ] == wanted[ i ];
        }
        if ( same )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

static bool
EndsWith( const std::string& text, const char* suffix )
{
    const size_t n = strlen( suffix );
    return text.size() >= n && text.compare( text.size() - n, n, suffix ) == 0;
}

/*-------------------------------------------------------------------------*/

/** A body that is a list of ids and nothing else: {"matchSampleIds":["GUID",...]}, {"sampleIds":[...]} or a plain list of GUIDs */
static bool
BodyIsOnlyIds( const std::string& body )
{
    if ( body.empty() || body.size() > 256 * 1024 )
        return false;
    std::string b;
    for ( size_t i=0; i<body.size(); ++i )
        if ( body[ i ] != ' ' && body[ i ] != '\n' && body[ i ] != '\r' && body[ i ] != '\t' )
            b += body[ i ];

    std::string inner;
    static const char* const objectStarts[] = { "{\"matchSampleIds\":[", "{\"sampleIds\":[", GUCEF_NULL };
    bool found = false;
    for ( UInt32 i=0; objectStarts[ i ] != GUCEF_NULL && !found; ++i )
    {
        const std::string start = objectStarts[ i ];
        if ( b.compare( 0, start.size(), start ) == 0 && b.size() > start.size() + 2 && b.compare( b.size() - 2, 2, "]}" ) == 0 )
        {
            inner = b.substr( start.size(), b.size() - start.size() - 2 );
            found = true;
        }
    }
    if ( !found && b.size() > 2 && b[ 0 ] == '[' && b[ b.size() - 1 ] == ']' )
    {
        inner = b.substr( 1, b.size() - 2 );
        found = true;
    }
    if ( !found || inner.empty() )
        return false;

    // every item is a quoted GUID: "8-4-4-4-12"
    size_t pos = 0;
    while ( pos < inner.size() )
    {
        if ( inner.size() - pos < 38 || inner[ pos ] != '"' || inner[ pos + 37 ] != '"' )
            return false;
        for ( size_t i=0; i<36; ++i )
        {
            const char c = inner[ pos + 1 + i ];
            const bool dash = i == 8 || i == 13 || i == 18 || i == 23;
            const bool hex = ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' );
            if ( dash ? c != '-' : !hex )
                return false;
        }
        pos += 38;
        if ( pos == inner.size() )
            break;
        if ( inner[ pos ] != ',' )
            return false;
        ++pos;
        if ( pos == inner.size() )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

SRequestVerdict
CRequestPolicy::Check( const std::string& method, const std::string& url, const std::string& body ) const
{GUCEF_TRACE;

    SRequestVerdict verdict;

    const bool isPost = method == "POST";
    if ( method != "GET" && !( isPost && m_dnaPostsEnabled && m_dnaAllowed ) )
    {
        verdict.reason = "only GET requests are ever sent, this is a " + method;
        return verdict;
    }

    SUrlTemplate parsed;
    if ( !CUrlTemplate::Parse( CORE::CString( url.c_str() ), parsed ) )
    {
        verdict.reason = "this is not an absolute url";
        return verdict;
    }
    if ( m_requireHttps && parsed.scheme != "https" )
    {
        verdict.reason = "only https is used";
        return verdict;
    }
    const std::string host = parsed.host.STL_String();
    bool hostOk = false;
    for ( size_t i=0; i<m_hostSuffixes.size() && !hostOk; ++i )
    {
        // the host itself, or a host below it: ancestry.com and www.ancestry.com but not notancestry.com or ancestry.com.evil.example
        std::string hostOnly = host;
        const size_t colon = hostOnly.find( ':' );
        if ( colon != std::string::npos )
            hostOnly = hostOnly.substr( 0, colon );
        hostOk = hostOnly == m_hostSuffixes[ i ] || EndsWith( hostOnly, ( "." + m_hostSuffixes[ i ] ).c_str() );
    }
    if ( !hostOk )
    {
        verdict.reason = "the host " + host + " is not an Ancestry host";
        return verdict;
    }
    if ( CEndpointCatalog::IsCredentialUrl( url ) )
    {
        verdict.reason = "this is a sign in, account or security page, those are never touched";
        return verdict;
    }
    const bool dna = CEndpointCatalog::IsDnaUrl( url );
    if ( dna && !m_dnaAllowed )
    {
        verdict.reason = "this is a DNA page and DNA access was not asked for";
        return verdict;
    }

    // another tree is none of our business, whether it is named in the path or in the query
    if ( !m_treeId.empty() )
    {
        const std::string path = parsed.path.STL_String();
        static const char* const treeSegments[] = { "tree", "trees", GUCEF_NULL };
        for ( UInt32 i=0; treeSegments[ i ] != GUCEF_NULL; ++i )
        {
            const std::string id = CUrlTemplate::FindIdAfterSegment( CORE::CString( path.c_str() ), CORE::CString( treeSegments[ i ] ) ).STL_String();
            if ( !id.empty() && id != m_treeId )
            {
                verdict.reason = "this is about tree " + id + " and not about tree " + m_treeId;
                return verdict;
            }
        }
        for ( size_t q=0; q<parsed.queryParams.size(); ++q )
        {
            const std::string name = CUrlTemplate::PercentDecode( parsed.queryParams[ q ].first ).STL_String();
            if ( name == "treeId" || name == "tid" || name == "tree_id" || name == "treeid" )
            {
                const std::string value = CUrlTemplate::PercentDecode( parsed.queryParams[ q ].second ).STL_String();
                if ( !value.empty() && value != m_treeId )
                {
                    verdict.reason = "this is about tree " + value + " and not about tree " + m_treeId;
                    return verdict;
                }
            }
        }
    }

    if ( isPost && !BodyIsOnlyIds( body ) )
    {
        verdict.reason = "a POST request that reads may only carry a list of ids";
        return verdict;
    }

    // starting an export is a GET too, and it is only allowed when asked for
    for ( size_t q=0; q<parsed.queryParams.size(); ++q )
    {
        if ( parsed.queryParams[ q ].first == "start" && !m_exportStartAllowed )
        {
            verdict.reason = "starting something on the site was not asked for";
            return verdict;
        }
    }

    if ( !MatchesAllowed( std::string( parsed.GetKey().STL_String() ), host + std::string( parsed.path.STL_String() ), isPost ) )
    {
        verdict.reason = "this endpoint is not on the list of requests that are known to only read: " + std::string( parsed.GetKey().STL_String() ) + ( isPost ? " (as a POST)" : "" );
        return verdict;
    }

    verdict.allowed = true;
    return verdict;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
