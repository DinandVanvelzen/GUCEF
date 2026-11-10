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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include <sstream>
#include <algorithm>

#include "AncestryBackup_CEndpointCatalog.h"

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

static const size_t MaxExampleUrls = 3;
static const size_t MaxExampleValues = 4;
static const size_t MaxPostDataExamples = 2;
static const size_t MaxPostDataLength = 600;
static const size_t MaxBodyFiles = 3;

/*-------------------------------------------------------------------------*/

CEndpointCatalog::CEndpointCatalog( void )
    : m_entryMap()
    , m_entries()
    , m_hostCounts()
    , m_ignoredCounts()
    , m_treeIds()
    , m_recordDna( false )
    , m_requestCount( 0 )
    , m_firstTimestamp( 0.0 )
    , m_lastTimestamp( 0.0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEndpointCatalog::~CEndpointCatalog()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_entries.size(); ++i )
        delete m_entries[ i ];
    m_entries.clear();
    m_entryMap.clear();
}

/*-------------------------------------------------------------------------*/

static bool
Contains( const std::string& text, const char* part )
{
    return text.find( part ) != std::string::npos;
}

/*-------------------------------------------------------------------------*/

CEndpointCatalog::EKind
CEndpointCatalog::Classify( const SObservedRequest& observation )
{GUCEF_TRACE;

    const std::string& type = observation.resourceType;
    const std::string& mime = observation.mimeType;

    if ( type == "Script" || type == "Stylesheet" || type == "Font" || type == "Ping" || type == "Manifest" ||
         type == "CSPViolationReport" || type == "Preflight" || type == "TextTrack" || type == "SignedExchange" || type == "WebSocket" )
        return KIND_OTHER;

    if ( type == "Image" || type == "Media" || mime.compare( 0, 6, "image/" ) == 0 || mime.compare( 0, 6, "audio/" ) == 0 ||
         mime.compare( 0, 6, "video/" ) == 0 || mime == "application/pdf" )
        return KIND_MEDIA;

    if ( type == "XHR" || type == "Fetch" || Contains( mime, "json" ) || Contains( mime, "xml" ) && !Contains( mime, "html" ) )
        return KIND_DATA;

    if ( type == "Document" )
        return KIND_PAGE;

    return KIND_OTHER;
}

/*-------------------------------------------------------------------------*/

static std::string
PathOf( const std::string& url )
{
    SUrlTemplate parsed;
    std::string path = CUrlTemplate::Parse( CORE::CString( url.c_str() ), parsed ) ? std::string( parsed.path.STL_String() ) : url;
    for ( size_t i=0; i<path.size(); ++i )
        path[ i ] = (char) tolower( path[ i ] );
    return path;
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsCredentialUrl( const std::string& url )
{GUCEF_TRACE;

    const std::string path = PathOf( url );
    static const char* const parts[] = { "password", "passwd", "signin", "sign-in", "sign_in", "login", "logon", "credential", "oauth", "/security/", "step-up", "stepup",
                                         "/verify", "authenticat", "/2fa", "/mfa", "/otp", "passcode", "/token", "/account/", "/secret", "/session", GUCEF_NULL };
    for ( UInt32 i=0; parts[ i ] != GUCEF_NULL; ++i )
        if ( path.find( parts[ i ] ) != std::string::npos )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsDnaUrl( const std::string& url )
{GUCEF_TRACE;

    const std::string path = PathOf( url );
    static const char* const parts[] = { "/dna/", "/dna", "treesdna", "genetic", "discoveryui-matches", "thrulines", GUCEF_NULL };
    for ( UInt32 i=0; parts[ i ] != GUCEF_NULL; ++i )
        if ( path.find( parts[ i ] ) != std::string::npos )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::SetRecordDna( bool record )
{GUCEF_TRACE;

    m_recordDna = record;
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsExcluded( const std::string& url ) const
{GUCEF_TRACE;

    return IsCredentialUrl( url ) || ( !m_recordDna && IsDnaUrl( url ) );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsSensitiveUrl( const std::string& url )
{GUCEF_TRACE;

    return IsCredentialUrl( url ) || IsDnaUrl( url );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsSecretHeader( const std::string& name )
{GUCEF_TRACE;

    // credentials and the tokens that tie a request to a login session
    return name == "cookie" || name == "set-cookie" || name == "authorization" || name == "proxy-authorization" || name == "x-api-key" ||
           name.find( "csrf" ) != std::string::npos || name.find( "xsrf" ) != std::string::npos || name.find( "token" ) != std::string::npos;
}

/*-------------------------------------------------------------------------*/

static std::string
EntryKey( const SObservedRequest& observation, SUrlTemplate& parsed )
{
    if ( !CUrlTemplate::Parse( CORE::CString( observation.url.c_str() ), parsed ) )
        return std::string();
    return observation.method + " " + parsed.GetKey().STL_String().c_str();
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::NeedsShape( const SObservedRequest& observation ) const
{GUCEF_TRACE;

    if ( observation.bodyFile.empty() || !Contains( observation.mimeType, "json" ) )
        return false;

    SUrlTemplate parsed;
    const std::string key = EntryKey( observation, parsed );
    TEntryMap::const_iterator i = m_entryMap.find( key );
    return i == m_entryMap.end() || ( !i->second->hasShape && !i->second->shapeTried );
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::MarkShapeTried( const SObservedRequest& observation )
{GUCEF_TRACE;

    SUrlTemplate parsed;
    const std::string key = EntryKey( observation, parsed );
    TEntryMap::iterator i = m_entryMap.find( key );
    if ( i != m_entryMap.end() )
        i->second->shapeTried = true;
}

/*-------------------------------------------------------------------------*/

static void
AddUnique( std::vector< std::string >& list, const std::string& value, size_t maxCount )
{
    if ( list.size() < maxCount && std::find( list.begin(), list.end(), value ) == list.end() )
        list.push_back( value );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::Add( const SObservedRequest& observation, const SJsonShape* shape )
{GUCEF_TRACE;

    ++m_requestCount;
    if ( m_requestCount == 1 || observation.timestamp < m_firstTimestamp )
        m_firstTimestamp = observation.timestamp;
    if ( observation.timestamp > m_lastTimestamp )
        m_lastTimestamp = observation.timestamp;

    SUrlTemplate parsed;
    const std::string key = EntryKey( observation, parsed );
    if ( key.empty() )
        return false;   // data: and blob: urls and the like

    ++m_hostCounts[ parsed.host.STL_String().c_str() ];

    // the tree id is the id that follows the 'tree' path segment, if the application uses such urls
    const CORE::CString treeId = CUrlTemplate::FindIdAfterSegment( parsed.path, "tree" );
    if ( !treeId.IsNULLOrEmpty() )
        m_treeIds.insert( treeId.STL_String().c_str() );
    const CORE::CString treesId = CUrlTemplate::FindIdAfterSegment( parsed.path, "trees" );
    if ( !treesId.IsNULLOrEmpty() )
        m_treeIds.insert( treesId.STL_String().c_str() );

    const EKind kind = Classify( observation );
    if ( kind == KIND_OTHER )
    {
        ++m_ignoredCounts[ observation.resourceType.empty() ? std::string( "Other" ) : observation.resourceType ];
        return false;
    }

    bool isNew = false;
    SEndpointEntry* entry = GUCEF_NULL;
    TEntryMap::iterator i = m_entryMap.find( key );
    if ( i == m_entryMap.end() )
    {
        entry = new SEndpointEntry();
        entry->method = observation.method;
        entry->host = parsed.host.STL_String().c_str();
        entry->pathTemplate = parsed.pathTemplate.STL_String().c_str();
        entry->kind = (Int32) kind;
        entry->firstSequence = observation.sequence;
        m_entryMap[ key ] = entry;
        m_entries.push_back( entry );
        isNew = true;
    }
    else
        entry = i->second;

    ++entry->requestCount;
    ++entry->statusCounts[ observation.status ];
    if ( !observation.resourceType.empty() )
        entry->resourceTypes.insert( observation.resourceType );
    if ( !observation.mimeType.empty() )
        entry->mimeTypes.insert( observation.mimeType );
    AddUnique( entry->exampleUrls, observation.url, MaxExampleUrls );
    entry->totalBytes += observation.encodedDataLength;

    for ( size_t p=0; p<parsed.queryParams.size(); ++p )
        AddUnique( entry->queryParams[ CUrlTemplate::PercentDecode( parsed.queryParams[ p ].first ).STL_String().c_str() ],
                   CUrlTemplate::PercentDecode( parsed.queryParams[ p ].second ).STL_String().c_str(), MaxExampleValues );

    for ( std::map< std::string, std::string >::const_iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
        entry->requestHeaderNames.insert( h->first );

    // what was sent to and received from a sign in or account page stays out of the report, whatever the journal holds
    const bool sensitive = IsExcluded( observation.url );
    if ( !observation.postData.empty() && !sensitive )
        AddUnique( entry->postDataExamples, observation.postData.substr( 0, MaxPostDataLength ), MaxPostDataExamples );

    if ( !observation.bodyFile.empty() && !sensitive )
        AddUnique( entry->bodyFiles, observation.bodyFile, MaxBodyFiles );

    if ( GUCEF_NULL != shape && !entry->hasShape )
    {
        entry->shape = *shape;
        entry->hasShape = true;
    }
    return isNew;
}

/*-------------------------------------------------------------------------*/

UInt32
CEndpointCatalog::GetRequestCount( void ) const
{GUCEF_TRACE;

    return m_requestCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CEndpointCatalog::GetEndpointCount( EKind kind ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ]->kind == (Int32) kind )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

const std::vector< SEndpointEntry* >&
CEndpointCatalog::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

const SEndpointEntry*
CEndpointCatalog::FindEntry( const CORE::CString& method, const CORE::CString& url ) const
{GUCEF_TRACE;

    SUrlTemplate parsed;
    if ( !CUrlTemplate::Parse( url, parsed ) )
        return GUCEF_NULL;
    TEntryMap::const_iterator i = m_entryMap.find( std::string( method.STL_String().c_str() ) + " " + parsed.GetKey().STL_String().c_str() );
    return i == m_entryMap.end() ? GUCEF_NULL : i->second;
}

/*-------------------------------------------------------------------------*/

const std::set< std::string >&
CEndpointCatalog::GetTreeIds( void ) const
{GUCEF_TRACE;

    return m_treeIds;
}

/*-------------------------------------------------------------------------*/

static std::string
JoinSet( const std::set< std::string >& values )
{
    std::string result;
    for ( std::set< std::string >::const_iterator i = values.begin(); i != values.end(); ++i )
        result += ( result.empty() ? "" : ", " ) + *i;
    return result;
}

/*-------------------------------------------------------------------------*/

static std::string
FormatBytes( UInt64 bytes )
{
    std::ostringstream s;
    if ( bytes >= 1024ULL * 1024ULL )
        s << ( bytes / ( 1024.0 * 1024.0 ) ) << " MiB";
    else
    if ( bytes >= 1024ULL )
        s << ( bytes / 1024.0 ) << " KiB";
    else
        s << bytes << " B";
    return s.str();
}

/*-------------------------------------------------------------------------*/

static void
RenderEntry( const SEndpointEntry& entry, bool withShape, std::ostringstream& out )
{
    out << "### " << entry.method << " " << entry.host << entry.pathTemplate << "\n\n";
    out << "- requests: " << entry.requestCount << ", statuses: ";
    bool first = true;
    for ( std::map< Int32, UInt32 >::const_iterator s = entry.statusCounts.begin(); s != entry.statusCounts.end(); ++s )
    {
        out << ( first ? "" : ", " ) << s->first << " (" << s->second << ")";
        first = false;
    }
    out << ", transferred: " << FormatBytes( entry.totalBytes ) << "\n";
    if ( !entry.resourceTypes.empty() )
        out << "- resource types: " << JoinSet( entry.resourceTypes ) << "\n";
    if ( !entry.mimeTypes.empty() )
        out << "- content types: " << JoinSet( entry.mimeTypes ) << "\n";
    for ( size_t u=0; u<entry.exampleUrls.size(); ++u )
        out << "- example " << ( u + 1 ) << ": `" << entry.exampleUrls[ u ] << "`\n";

    if ( !entry.queryParams.empty() )
    {
        out << "- query parameters:\n";
        for ( std::map< std::string, std::vector< std::string > >::const_iterator q = entry.queryParams.begin(); q != entry.queryParams.end(); ++q )
        {
            out << "  - `" << q->first << "`";
            for ( size_t v=0; v<q->second.size(); ++v )
                out << ( v == 0 ? " e.g. " : ", " ) << "`" << q->second[ v ] << "`";
            out << "\n";
        }
    }
    if ( !entry.requestHeaderNames.empty() )
        out << "- request headers: " << JoinSet( entry.requestHeaderNames ) << "\n";
    for ( size_t p=0; p<entry.postDataExamples.size(); ++p )
        out << "- request body example: `" << entry.postDataExamples[ p ] << "`\n";
    for ( size_t b=0; b<entry.bodyFiles.size(); ++b )
        out << "- saved response: `" << entry.bodyFiles[ b ] << "`\n";

    if ( withShape && entry.hasShape )
    {
        if ( !entry.shape.paginationHints.empty() )
        {
            out << "- paging fields:\n";
            for ( size_t h=0; h<entry.shape.paginationHints.size(); ++h )
                out << "  - `" << entry.shape.paginationHints[ h ].STL_String().c_str() << "`\n";
        }
        if ( !entry.shape.urlFields.empty() )
        {
            out << "- urls in the response:\n";
            for ( size_t h=0; h<entry.shape.urlFields.size() && h<12; ++h )
                out << "  - `" << entry.shape.urlFields[ h ].STL_String().c_str() << "`\n";
        }
        out << "- response shape:\n\n```\n";
        for ( size_t l=0; l<entry.shape.lines.size(); ++l )
            out << entry.shape.lines[ l ].STL_String().c_str() << "\n";
        if ( entry.shape.truncated )
            out << "...\n";
        out << "```\n";
    }
    out << "\n";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEndpointCatalog::RenderReport( void ) const
{GUCEF_TRACE;

    std::ostringstream out;
    out << "# Endpoints observed during discovery\n\n";
    out << "Requests observed: " << m_requestCount << ", distinct endpoints: " << m_entries.size() << ", observed over " << ( m_lastTimestamp - m_firstTimestamp ) << " seconds.\n\n";

    if ( !m_treeIds.empty() )
        out << "Tree ids seen in urls: " << JoinSet( m_treeIds ) << "\n\n";

    out << "## Hosts\n\n| host | requests |\n|---|---|\n";
    for ( std::map< std::string, UInt32 >::const_iterator h = m_hostCounts.begin(); h != m_hostCounts.end(); ++h )
        out << "| " << h->first << " | " << h->second << " |\n";
    out << "\n";

    const char* const titles[] = { "Data endpoints (XHR, fetch, JSON and XML)", "Media and documents", "Pages" };
    const EKind kinds[] = { KIND_DATA, KIND_MEDIA, KIND_PAGE };
    for ( int k=0; k<3; ++k )
    {
        out << "## " << titles[ k ] << "\n\n";
        bool any = false;
        for ( size_t i=0; i<m_entries.size(); ++i )
        {
            const SEndpointEntry& entry = *m_entries[ i ];
            if ( entry.kind != (Int32) kinds[ k ] )
                continue;
            RenderEntry( entry, kinds[ k ] == KIND_DATA, out );
            any = true;
        }
        if ( !any )
            out << "None observed.\n\n";
    }

    out << "## Not reported (scripts, styles, fonts, tracking)\n\n";
    if ( m_ignoredCounts.empty() )
        out << "None.\n";
    for ( std::map< std::string, UInt32 >::const_iterator i = m_ignoredCounts.begin(); i != m_ignoredCounts.end(); ++i )
        out << "- " << i->first << ": " << i->second << "\n";

    return CORE::CString( out.str().c_str() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEndpointCatalog::ToJournalLine( const SObservedRequest& observation )
{GUCEF_TRACE;

    std::ostringstream out;
    out << "{\"seq\":" << observation.sequence << ",\"ts\":" << observation.timestamp
        << ",\"method\":\"" << CUrlTemplate::EscapeJson( observation.method.c_str() ).STL_String().c_str() << "\""
        << ",\"url\":\"" << CUrlTemplate::EscapeJson( observation.url.c_str() ).STL_String().c_str() << "\""
        << ",\"type\":\"" << CUrlTemplate::EscapeJson( observation.resourceType.c_str() ).STL_String().c_str() << "\""
        << ",\"status\":" << observation.status
        << ",\"mime\":\"" << CUrlTemplate::EscapeJson( observation.mimeType.c_str() ).STL_String().c_str() << "\""
        << ",\"size\":" << observation.encodedDataLength
        << ",\"failed\":" << ( observation.failed ? "true" : "false" )
        << ",\"error\":\"" << CUrlTemplate::EscapeJson( observation.errorText.c_str() ).STL_String().c_str() << "\""
        << ",\"bodyFile\":\"" << CUrlTemplate::EscapeJson( observation.bodyFile.c_str() ).STL_String().c_str() << "\""
        << ",\"postData\":\"" << CUrlTemplate::EscapeJson( observation.postData.c_str() ).STL_String().c_str() << "\""
        << ",\"requestHeaders\":{";
    bool first = true;
    for ( std::map< std::string, std::string >::const_iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
    {
        out << ( first ? "" : "," ) << "\"" << CUrlTemplate::EscapeJson( h->first.c_str() ).STL_String().c_str() << "\":\"" << CUrlTemplate::EscapeJson( h->second.c_str() ).STL_String().c_str() << "\"";
        first = false;
    }
    out << "}}";
    return CORE::CString( out.str().c_str() );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::FromJournalDocument( const CORE::CDataNode& document, SObservedRequest& observation )
{GUCEF_TRACE;

    observation = SObservedRequest();
    observation.url = document.GetAttributeValue( "url" ).AsString().STL_String().c_str();
    if ( observation.url.empty() )
        return false;

    observation.sequence = (UInt32) document.GetAttributeValue( "seq" ).AsUInt64( 0 );
    observation.timestamp = document.GetAttributeValue( "ts" ).AsFloat64( 0.0 );
    observation.method = document.GetAttributeValue( "method" ).AsString().STL_String().c_str();
    observation.resourceType = document.GetAttributeValue( "type" ).AsString().STL_String().c_str();
    observation.status = (Int32) document.GetAttributeValue( "status" ).AsInt64( 0 );
    observation.mimeType = document.GetAttributeValue( "mime" ).AsString().STL_String().c_str();
    observation.encodedDataLength = document.GetAttributeValue( "size" ).AsUInt64( 0 );
    observation.failed = document.GetAttributeValue( "failed" ).AsBool( false );
    observation.errorText = document.GetAttributeValue( "error" ).AsString().STL_String().c_str();
    observation.bodyFile = document.GetAttributeValue( "bodyFile" ).AsString().STL_String().c_str();
    observation.postData = document.GetAttributeValue( "postData" ).AsString().STL_String().c_str();

    const CORE::CDataNode* headers = document.Find( "requestHeaders" );
    if ( GUCEF_NULL != headers )
    {
        for ( UInt32 i=0; i<headers->GetAttCount(); ++i )
        {
            const CORE::CDataNode::TKeyValuePair* att = headers->GetAttribute( i );
            if ( GUCEF_NULL != att )
                observation.requestHeaders[ att->first.STL_String().c_str() ] = att->second.AsString().STL_String().c_str();
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
