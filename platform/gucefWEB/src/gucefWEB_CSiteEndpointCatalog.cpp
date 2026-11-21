/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#include <stdio.h>
#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefWEB_CSiteEndpointCatalog.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static bool
Contains( const CORE::CString& text, const char* part )
{
    return !text.IsNULLOrEmpty() && GUCEF_NULL != strstr( text.C_String(), part );
}

/*-------------------------------------------------------------------------*/

static bool
StartsWith( const CORE::CString& text, const char* prefix )
{
    return !text.IsNULLOrEmpty() && 0 == strncmp( text.C_String(), prefix, strlen( prefix ) );
}

/*-------------------------------------------------------------------------*/

static void
AddUnique( CORE::CString::StringVector& list, const CORE::CString& value, size_t maxCount )
{
    if ( list.size() < maxCount && std::find( list.begin(), list.end(), value ) == list.end() )
        list.push_back( value );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
JoinSet( const CORE::CString::StringSet& values )
{
    CORE::CString result;
    for ( CORE::CString::StringSet::const_iterator i = values.begin(); i != values.end(); ++i )
    {
        if ( !result.IsNULLOrEmpty() )
            result += ", ";
        result += *i;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
FormatBytes( UInt64 bytes )
{
    char text[ 64 ];
    if ( bytes >= 1024ULL * 1024ULL )
        snprintf( text, sizeof( text ), "%.1f MiB", bytes / ( 1024.0 * 1024.0 ) );
    else
    if ( bytes >= 1024ULL )
        snprintf( text, sizeof( text ), "%.1f KiB", bytes / 1024.0 );
    else
        snprintf( text, sizeof( text ), "%u B", (unsigned int) bytes );
    return text;
}

/*-------------------------------------------------------------------------*/

static void
RenderShapeList( const char* title, const CORE::CString::StringVector& items, size_t maxItems, CORE::CString& out )
{
    if ( items.empty() )
        return;
    out += CORE::CString( "- " ) + title + ":\n";
    for ( size_t h=0; h<items.size() && h<maxItems; ++h )
        out += "  - `" + items[ h ] + "`\n";
}

/*-------------------------------------------------------------------------*/

static void
RenderShapeBlock( const char* title, const CWebBodyShape& shape, CORE::CString& out )
{
    out += CORE::CString( "- " ) + title + ":\n\n```\n";
    for ( size_t l=0; l<shape.lines.size(); ++l )
        out += shape.lines[ l ] + "\n";
    if ( shape.truncated )
        out += "...\n";
    out += "```\n";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSiteEndpointCatalog::CSiteEndpointCatalog( void )
    : CIWebObservationSink()
    , m_ownRedactor()
    , m_redactor( &m_ownRedactor )
    , m_entryMap()
    , m_entries()
    , m_hostCounts()
    , m_hostInfo()
    , m_ignoredCounts()
    , m_actions()
    , m_actionIndex()
    , m_droppedActions( 0 )
    , m_requestCount( 0 )
    , m_firstTimestamp( 0.0 )
    , m_lastTimestamp( 0.0 )
    , m_stubCount( 0 )
    , m_stubByClass()
    , m_lastRedactedCount( 0 )
    , m_lastRedactedByClass()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CSiteEndpointCatalog::~CSiteEndpointCatalog()
{GUCEF_TRACE;

    Clear();
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::Clear( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_entries.size(); ++i )
        GUCEF_DELETE m_entries[ i ];
    m_entries.clear();
    m_entryMap.clear();
    m_hostCounts.clear();
    m_hostInfo.clear();
    m_ignoredCounts.clear();
    m_actions.clear();
    m_actionIndex.clear();
    m_droppedActions = 0;
    m_requestCount = 0;
    m_firstTimestamp = 0.0;
    m_lastTimestamp = 0.0;
    m_stubCount = 0;
    m_stubByClass.clear();
    m_lastRedactedCount = 0;
    m_lastRedactedByClass.clear();
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::SetRedactor( CSiteDiscoveryRedactor* redactor )
{GUCEF_TRACE;

    m_redactor = GUCEF_NULL != redactor ? redactor : &m_ownRedactor;
}

/*-------------------------------------------------------------------------*/

CSiteDiscoveryRedactor&
CSiteEndpointCatalog::GetRedactor( void ) const
{GUCEF_TRACE;

    return *m_redactor;
}

/*-------------------------------------------------------------------------*/

CSiteEndpointCatalog::EKind
CSiteEndpointCatalog::Classify( const CORE::CString& type, const CORE::CString& mimeType )
{GUCEF_TRACE;

    const CORE::CString mime = mimeType.Lowercase();
    if ( type == "Script" || type == "Stylesheet" || type == "Font" || type == "Ping" || type == "Manifest" ||
         type == "CSPViolationReport" || type == "Preflight" || type == "TextTrack" || type == "SignedExchange" || type == "WebSocket" )
        return KIND_OTHER;
    if ( type == "Image" || type == "Media" || StartsWith( mime, "image/" ) || StartsWith( mime, "audio/" ) ||
         StartsWith( mime, "video/" ) || mime == "application/pdf" )
        return KIND_MEDIA;
    if ( type == "XHR" || type == "Fetch" || Contains( mime, "json" ) || ( Contains( mime, "xml" ) && !Contains( mime, "html" ) ) )
        return KIND_DATA;
    if ( type == "Document" )
        return KIND_PAGE;
    return KIND_OTHER;
}

/*-------------------------------------------------------------------------*/

CSiteEndpointCatalog::EKind
CSiteEndpointCatalog::Classify( const CObservedWebRequest& observation )
{GUCEF_TRACE;

    const EKind kind = Classify( observation.resourceType, observation.mimeType );
    if ( KIND_OTHER == kind && observation.isDownload )
        return KIND_MEDIA;      // a file the site hands out is never noise
    return kind;
}

/*-------------------------------------------------------------------------*/

bool
CSiteEndpointCatalog::NeedsShape( const CObservedWebRequest& observation ) const
{GUCEF_TRACE;

    if ( observation.neverRecorded || observation.isDownload || !Contains( observation.mimeType.Lowercase(), "json" ) )
        return false;
    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( observation.url, parsed ) || m_redactor->IsNeverRecord( parsed ) )
        return false;
    TEndpointMap::const_iterator i = m_entryMap.find( observation.method + " " + parsed.GetKey() );
    if ( i == m_entryMap.end() )
        return true;
    if ( !i->second->hasShape && i->second->shapeTried )
        return false;       // a response was looked at and gave no shape
    const UInt32 maxShapes = m_redactor->GetSettings().maxShapesPerEndpoint;
    return 0 == maxShapes || i->second->shape.mergedCount < maxShapes;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteEndpointCatalog::GetMergedShapeLimit( void ) const
{GUCEF_TRACE;

    return m_redactor->GetSettings().maxShapeLines * 4;
}

/*-------------------------------------------------------------------------*/

bool
CSiteEndpointCatalog::IsNewResponseShape( const CObservedWebRequest& observation, const CWebBodyShape& shape ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( observation.url, parsed ) )
        return false;
    TEndpointMap::const_iterator i = m_entryMap.find( observation.method + " " + parsed.GetKey() );
    if ( i == m_entryMap.end() )
        return !shape.IsEmpty();
    const UInt32 maxShapes = m_redactor->GetSettings().maxShapesPerEndpoint;
    if ( 0 != maxShapes && i->second->shape.mergedCount >= maxShapes )
        return false;
    return shape.AddsToMerged( i->second->shape, GetMergedShapeLimit() );
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::OnWebRequestObserved( const CObservedWebRequest& observation )
{GUCEF_TRACE;

    Add( observation );
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::OnUserActionObserved( const CObservedUserAction& action )
{GUCEF_TRACE;

    AddAction( action );
}

/*-------------------------------------------------------------------------*/

bool
CSiteEndpointCatalog::WantsResponseShape( const CObservedWebRequest& observation ) const
{GUCEF_TRACE;

    return NeedsShape( observation );
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::AddAction( const CObservedUserAction& action )
{GUCEF_TRACE;

    if ( 0 == action.id || m_actionIndex.find( action.id ) != m_actionIndex.end() )
        return;
    const UInt32 maxActions = m_redactor->GetSettings().maxActions;
    if ( 0 != maxActions && m_actions.size() >= maxActions )
    {
        ++m_droppedActions;
        return;
    }
    SActionRecord record;
    record.action = action;
    m_actionIndex[ action.id ] = m_actions.size();
    m_actions.push_back( record );
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::AddStep( UInt32 actionId, const CORE::CString& step )
{GUCEF_TRACE;

    TActionIndexMap::iterator i = m_actionIndex.find( actionId );
    if ( i == m_actionIndex.end() )
        return;
    SActionRecord& record = m_actions[ i->second ];
    ++record.requestCount;
    if ( !record.steps.empty() && record.steps.back() == step )
        return;
    if ( record.steps.size() >= MaxStepsPerAction )
    {
        ++record.droppedSteps;
        return;
    }
    record.steps.push_back( step );
}

/*-------------------------------------------------------------------------*/

bool
CSiteEndpointCatalog::Add( const CObservedWebRequest& observation )
{GUCEF_TRACE;

    if ( !observation.redactedClass.IsNULLOrEmpty() )
    {
        // the stub of a journal line that held a secret: it happened, nothing else is known of it
        ++m_stubCount;
        ++m_stubByClass[ observation.redactedClass ];
        ++m_requestCount;
        if ( m_requestCount == 1 || observation.timestamp < m_firstTimestamp )
            m_firstTimestamp = observation.timestamp;
        if ( observation.timestamp > m_lastTimestamp )
            m_lastTimestamp = observation.timestamp;
        return false;
    }

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( observation.url, parsed ) )
        return false;   // data: and blob: urls and the like

    const CSiteDiscoverySettings& settings = m_redactor->GetSettings();
    const bool neverRecorded = observation.neverRecorded || m_redactor->IsNeverRecord( parsed );
    if ( neverRecorded && !settings.noteNeverRecorded )
        return false;

    ++m_requestCount;
    if ( m_requestCount == 1 || observation.timestamp < m_firstTimestamp )
        m_firstTimestamp = observation.timestamp;
    if ( observation.timestamp > m_lastTimestamp )
        m_lastTimestamp = observation.timestamp;
    ++m_hostCounts[ parsed.host ];
    if ( !neverRecorded )
    {
        SHostInfo& hostInfo = m_hostInfo[ parsed.host ];
        for ( CObservedWebRequest::TStringMap::const_iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
            hostInfo.requestHeaders.insert( h->first );
        for ( size_t c=0; c<observation.cookiesSent.size(); ++c )
            hostInfo.cookiesSent.insert( observation.cookiesSent[ c ] );
        for ( size_t c=0; c<observation.cookiesSet.size(); ++c )
            hostInfo.cookiesSet.insert( observation.cookiesSet[ c ] );
    }

    const EKind kind = Classify( observation );
    if ( kind == KIND_OTHER )
    {
        ++m_ignoredCounts[ observation.resourceType.IsNULLOrEmpty() ? CORE::CString( "Other" ) : observation.resourceType ];
        return false;
    }

    const CORE::CString key = observation.method + " " + parsed.GetKey();
    bool isNew = false;
    CDiscoveredEndpoint* entry = GUCEF_NULL;
    TEndpointMap::iterator i = m_entryMap.find( key );
    if ( i == m_entryMap.end() )
    {
        entry = GUCEF_NEW CDiscoveredEndpoint();
        entry->method = observation.method;
        entry->host = parsed.host;
        entry->pathTemplate = parsed.pathTemplate;
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
    entry->totalBytes += observation.encodedDataLength;
    if ( 0 != observation.actionId )
        AddStep( observation.actionId, key + ( observation.isDownload ? " (download)" : "" ) );

    if ( neverRecorded )
    {
        // that it happened is all there is to know
        entry->neverRecorded = true;
        return isNew;
    }

    if ( !observation.resourceType.IsNULLOrEmpty() )
        entry->resourceTypes.insert( observation.resourceType );
    if ( !observation.mimeType.IsNULLOrEmpty() )
        entry->mimeTypes.insert( observation.mimeType );
    if ( !observation.requestContentType.IsNULLOrEmpty() )
        entry->requestContentTypes.insert( observation.requestContentType );
    for ( CObservedWebRequest::TStringMap::const_iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
        entry->requestHeaderNames.insert( h->first );
    if ( !entry->hasRequestBodyShape && !observation.requestBodyShape.IsEmpty() )
    {
        entry->requestBodyShape = observation.requestBodyShape;
        entry->hasRequestBodyShape = true;
    }
    if ( observation.hasResponseShape || !observation.bodyFile.IsNULLOrEmpty() )
        entry->shapeTried = true;
    if ( observation.hasResponseShape )
    {
        entry->shape.Merge( observation.responseShape, GetMergedShapeLimit() );
        entry->hasShape = true;
    }
    if ( observation.status >= 300 && observation.status <= 399 && !observation.redirectTarget.IsNULLOrEmpty() )
        entry->redirectTargets.insert( CORE::Int32ToString( observation.status ) + " -> " + observation.redirectTarget );
    if ( observation.isDownload )
    {
        ++entry->downloadCount;
        if ( !observation.contentDisposition.IsNULLOrEmpty() )
            entry->downloadDispositions.insert( observation.contentDisposition );
        if ( !observation.mimeType.IsNULLOrEmpty() )
            entry->downloadContentTypes.insert( observation.mimeType );
        if ( !observation.downloadFileName.IsNULLOrEmpty() )
            entry->downloadFileNames.insert( observation.downloadFileName );
        if ( !observation.redirectedFromHosts.empty() )
        {
            CORE::CString chain;
            for ( size_t h=0; h<observation.redirectedFromHosts.size(); ++h )
                chain += observation.redirectedFromHosts[ h ] + " > ";
            entry->downloadRedirectChains.insert( chain + parsed.host );
        }
    }

    for ( size_t p=0; p<parsed.queryParams.size(); ++p )
    {
        const CORE::CString name = CUrlPathTemplate::PercentDecode( parsed.queryParams[ p ].first );
        CORE::CString::StringVector& values = entry->queryParams[ name ];
        if ( settings.recordExampleValues )
            AddUnique( values, CUrlPathTemplate::PercentDecode( m_redactor->RedactQueryValue( name, parsed.queryParams[ p ].second ) ), MaxExampleValues );
    }

    if ( settings.recordEnumQueryValues )
    {
        CObservedWebRequest::TStringPairVector enums = observation.queryEnumValues;
        if ( enums.empty() )
            m_redactor->ExtractEnumQueryValues( parsed, enums );
        for ( size_t e=0; e<enums.size(); ++e )
        {
            CDiscoveredEndpoint::TValueCountMap& counts = entry->queryEnumValues[ enums[ e ].first ];
            CDiscoveredEndpoint::TValueCountMap::iterator found = counts.find( enums[ e ].second );
            if ( found != counts.end() )
                ++found->second;
            else
            if ( counts.size() < CDiscoveredEndpoint::MaxEnumValues )
                counts[ enums[ e ].second ] = 1;
            else
                entry->queryEnumOverflow.insert( enums[ e ].first );
            if ( entry->queryParams.find( enums[ e ].first ) == entry->queryParams.end() )
                entry->queryParams[ enums[ e ].first ];
        }
    }

    if ( settings.recordExampleValues )
    {
        AddUnique( entry->exampleUrls, m_redactor->RedactUrl( observation.url, false ), MaxExampleUrls );
        if ( !observation.postData.IsNULLOrEmpty() )
        {
            CORE::CString example = m_redactor->RedactBody( observation.postData );
            if ( example.Length() > MaxPostDataLength )
                example = example.SubstrToIndex( MaxPostDataLength, true );
            AddUnique( entry->postDataExamples, example, MaxPostDataExamples );
        }
        if ( !observation.bodyFile.IsNULLOrEmpty() )
            AddUnique( entry->bodyFiles, observation.bodyFile, MaxBodyFiles );
    }
    return isNew;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteEndpointCatalog::GetRequestCount( void ) const
{GUCEF_TRACE;

    return m_requestCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteEndpointCatalog::GetEndpointCount( EKind kind ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ]->kind == (Int32) kind )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

const CSiteEndpointCatalog::TEndpointVector&
CSiteEndpointCatalog::GetEntries( void ) const
{GUCEF_TRACE;

    return m_entries;
}

/*-------------------------------------------------------------------------*/

const CSiteEndpointCatalog::TActionVector&
CSiteEndpointCatalog::GetActions( void ) const
{GUCEF_TRACE;

    return m_actions;
}

/*-------------------------------------------------------------------------*/

const CDiscoveredEndpoint*
CSiteEndpointCatalog::FindEntry( const CORE::CString& method, const CORE::CString& url ) const
{GUCEF_TRACE;

    CUrlPathTemplate parsed;
    if ( !CUrlPathTemplate::Parse( url, parsed ) )
        return GUCEF_NULL;
    TEndpointMap::const_iterator i = m_entryMap.find( method + " " + parsed.GetKey() );
    return i == m_entryMap.end() ? GUCEF_NULL : i->second;
}

/*-------------------------------------------------------------------------*/

void
CSiteEndpointCatalog::RenderEntry( const CDiscoveredEndpoint& entry, CORE::CString& out ) const
{GUCEF_TRACE;

    out += "### " + entry.method + " " + entry.host + entry.pathTemplate + "\n\n";
    out += "- requests: " + CORE::UInt32ToString( entry.requestCount ) + ", statuses: ";
    bool first = true;
    for ( CDiscoveredEndpoint::TStatusCountMap::const_iterator s = entry.statusCounts.begin(); s != entry.statusCounts.end(); ++s )
    {
        out += ( first ? "" : ", " ) + CORE::Int32ToString( s->first ) + " (" + CORE::UInt32ToString( s->second ) + ")";
        first = false;
    }
    out += ", transferred: " + FormatBytes( entry.totalBytes ) + "\n";
    if ( entry.neverRecorded )
    {
        out += "- content not recorded: a sign-in, security, credential, checkout or payment request\n\n";
        return;
    }
    if ( !entry.resourceTypes.empty() )
        out += "- resource types: " + JoinSet( entry.resourceTypes ) + "\n";
    if ( !entry.mimeTypes.empty() )
        out += "- content types: " + JoinSet( entry.mimeTypes ) + "\n";
    for ( size_t u=0; u<entry.exampleUrls.size(); ++u )
        out += "- example " + CORE::UInt32ToString( (UInt32) u + 1 ) + ": `" + entry.exampleUrls[ u ] + "`\n";
    if ( !entry.queryParams.empty() )
    {
        out += "- query parameters:\n";
        for ( CDiscoveredEndpoint::TQueryParamMap::const_iterator q = entry.queryParams.begin(); q != entry.queryParams.end(); ++q )
        {
            out += "  - `" + q->first + "`";
            for ( size_t v=0; v<q->second.size(); ++v )
                out += ( v == 0 ? CORE::CString( " e.g. " ) : CORE::CString( ", " ) ) + "`" + q->second[ v ] + "`";
            CDiscoveredEndpoint::TQueryEnumMap::const_iterator enumValues = entry.queryEnumValues.find( q->first );
            if ( enumValues != entry.queryEnumValues.end() )
            {
                out += " values: ";
                bool firstValue = true;
                for ( CDiscoveredEndpoint::TValueCountMap::const_iterator v = enumValues->second.begin(); v != enumValues->second.end(); ++v )
                {
                    out += ( firstValue ? "`" : ", `" ) + v->first + "` (" + CORE::UInt32ToString( v->second ) + ")";
                    firstValue = false;
                }
                if ( entry.queryEnumOverflow.find( q->first ) != entry.queryEnumOverflow.end() )
                    out += ", and other values";
            }
            if ( CWebBodyShape::IsPaginationKey( q->first ) )
                out += " (paging)";
            out += "\n";
        }
    }
    if ( !entry.requestHeaderNames.empty() )
        out += "- request headers: " + JoinSet( entry.requestHeaderNames ) + "\n";
    if ( !entry.requestContentTypes.empty() )
        out += "- request content types: " + JoinSet( entry.requestContentTypes ) + "\n";
    if ( entry.hasRequestBodyShape )
        RenderShapeBlock( "request body fields", entry.requestBodyShape, out );
    for ( size_t p=0; p<entry.postDataExamples.size(); ++p )
        out += "- request body example: `" + entry.postDataExamples[ p ] + "`\n";
    for ( size_t b=0; b<entry.bodyFiles.size(); ++b )
        out += "- saved response: `" + entry.bodyFiles[ b ] + "`\n";
    if ( !entry.redirectTargets.empty() )
        out += "- redirects: " + JoinSet( entry.redirectTargets ) + "\n";
    if ( entry.downloadCount > 0 )
    {
        out += "- downloads: " + CORE::UInt32ToString( entry.downloadCount );
        if ( !entry.downloadDispositions.empty() )
            out += ", disposition: " + JoinSet( entry.downloadDispositions );
        if ( !entry.downloadContentTypes.empty() )
            out += ", content types: " + JoinSet( entry.downloadContentTypes );
        if ( !entry.downloadFileNames.empty() )
            out += ", file names: " + JoinSet( entry.downloadFileNames );
        out += "\n";
        if ( !entry.downloadRedirectChains.empty() )
            out += "- download redirect chain (hosts): " + JoinSet( entry.downloadRedirectChains ) + "\n";
    }
    if ( entry.hasShape )
    {
        RenderShapeList( "paging fields", entry.shape.paginationHints, 50, out );
        RenderShapeList( "urls in the response", entry.shape.urlFields, 12, out );
        RenderShapeBlock( entry.shape.mergedCount > 1 ? ( "response shape (union of " + CORE::UInt32ToString( entry.shape.mergedCount ) + " responses)" ).C_String() : "response shape", entry.shape, out );
    }
    out += "\n";
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteEndpointCatalog::GetRedactedEntryCount( void ) const
{GUCEF_TRACE;

    return m_lastRedactedCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteEndpointCatalog::GetRedactionSummary( void ) const
{GUCEF_TRACE;

    return CSiteDiscoveryRedactor::FormatClassCounts( m_lastRedactedByClass );
}

/*-------------------------------------------------------------------------*/

UInt32
CSiteEndpointCatalog::GetStubCount( void ) const
{GUCEF_TRACE;

    return m_stubCount;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSiteEndpointCatalog::GetStubSummary( void ) const
{GUCEF_TRACE;

    return CSiteDiscoveryRedactor::FormatClassCounts( m_stubByClass );
}

/*-------------------------------------------------------------------------*/

bool
CSiteEndpointCatalog::Guard( const CORE::CString& block, CORE::CString& reason ) const
{GUCEF_TRACE;

    CSiteDiscoveryRedactor::ESecretClass secretClass = CSiteDiscoveryRedactor::SECRET_CLASS_OTHER;
    if ( !m_redactor->FindKnownSecret( block, secretClass ) )
        return true;
    const CORE::CString className = CSiteDiscoveryRedactor::SecretClassToString( secretClass );
    ++m_lastRedactedCount;
    ++m_lastRedactedByClass[ className ];
    reason = "secret class " + className + ", left out " + CORE::UInt32ToString( m_lastRedactedCount );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CSiteEndpointCatalog::RenderReport( CORE::CString& report, const CORE::CString::StringVector& extraSummaryLines ) const
{GUCEF_TRACE;

    m_lastRedactedCount = 0;
    m_lastRedactedByClass.clear();

    // every part is checked on its own, so a part that would hold a secret value is the only one that is left out
    CORE::CString out;
    CORE::CString reason;
    for ( size_t i=0; i<extraSummaryLines.size(); ++i )
    {
        if ( Guard( extraSummaryLines[ i ], reason ) )
            out += extraSummaryLines[ i ] + "\n\n";
        else
            out += "A summary line was left out: it would contain a secret value (" + reason + ").\n\n";
    }

    out += "## Hosts\n\n| host | requests |\n|---|---|\n";
    UInt32 hostIndex = 0;
    for ( TCountMap::const_iterator h = m_hostCounts.begin(); h != m_hostCounts.end(); ++h )
    {
        ++hostIndex;
        const CORE::CString row = "| " + h->first + " | " + CORE::UInt32ToString( h->second ) + " |\n";
        if ( Guard( row, reason ) )
            out += row;
        else
            out += "| (host " + CORE::UInt32ToString( hostIndex ) + " left out: " + reason + ") | " + CORE::UInt32ToString( h->second ) + " |\n";
    }
    out += "\n";

    if ( !m_hostInfo.empty() )
    {
        out += "## Request headers and cookies per host\n\nNames only, never values.\n\n";
        hostIndex = 0;
        for ( THostInfoMap::const_iterator h = m_hostInfo.begin(); h != m_hostInfo.end(); ++h )
        {
            ++hostIndex;
            CORE::CString row = "- `" + h->first + "`: request headers: " + ( h->second.requestHeaders.empty() ? CORE::CString( "none" ) : JoinSet( h->second.requestHeaders ) ) +
                                "; cookies sent: " + ( h->second.cookiesSent.empty() ? CORE::CString( "none" ) : JoinSet( h->second.cookiesSent ) ) +
                                "; cookies set: " + ( h->second.cookiesSet.empty() ? CORE::CString( "none" ) : JoinSet( h->second.cookiesSet ) ) + "\n";
            if ( Guard( row, reason ) )
                out += row;
            else
                out += "- (host " + CORE::UInt32ToString( hostIndex ) + " left out: " + reason + ")\n";
        }
        out += "\n";
    }

    const char* const titles[] = { "Data endpoints (XHR, fetch, JSON and XML)", "Media and documents", "Pages" };
    const EKind kinds[] = { KIND_DATA, KIND_MEDIA, KIND_PAGE };
    GUCEF::vector< UInt8 > leftOut( m_entries.size(), 0 );
    for ( int k=0; k<3; ++k )
    {
        out += CORE::CString( "## " ) + titles[ k ] + "\n\n";
        bool any = false;
        for ( size_t i=0; i<m_entries.size(); ++i )
        {
            if ( m_entries[ i ]->kind != (Int32) kinds[ k ] )
                continue;
            CORE::CString entryText;
            RenderEntry( *m_entries[ i ], entryText );
            if ( Guard( entryText, reason ) )
                out += entryText;
            else
            {
                leftOut[ i ] = 1;
                out += "### Endpoint " + CORE::UInt32ToString( (UInt32) i + 1 ) + " of " + CORE::UInt32ToString( (UInt32) m_entries.size() ) +
                       ": left out\n\n- This endpoint would have contained a secret value that was seen during recording (" + reason + "). Its content is not shown; the other endpoints are complete.\n\n";
            }
            any = true;
        }
        if ( !any )
            out += "None observed.\n\n";
    }

    out += "## Downloads\n\n";
    bool anyDownload = false;
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        const CDiscoveredEndpoint& entry = *m_entries[ i ];
        if ( 0 == entry.downloadCount )
            continue;
        anyDownload = true;
        if ( 0 != leftOut[ i ] )
        {
            out += "- Endpoint " + CORE::UInt32ToString( (UInt32) i + 1 ) + " left out, see above\n";
            continue;
        }
        CORE::CString line = "- `" + entry.GetKey() + "`: " + CORE::UInt32ToString( entry.downloadCount ) + " file(s)";
        if ( !entry.downloadContentTypes.empty() )
            line += ", " + JoinSet( entry.downloadContentTypes );
        if ( !entry.downloadFileNames.empty() )
            line += ", " + JoinSet( entry.downloadFileNames );
        line += ", " + FormatBytes( entry.totalBytes ) + "\n";
        if ( Guard( line, reason ) )
            out += line;
        else
            out += "- Endpoint " + CORE::UInt32ToString( (UInt32) i + 1 ) + " left out: it would contain a secret value (" + reason + ")\n";
    }
    out += anyDownload ? "\n" : "None observed.\n\n";

    out += "## User actions and the requests that followed\n\n";
    bool anyAction = false;
    for ( size_t a=0; a<m_actions.size(); ++a )
    {
        const SActionRecord& record = m_actions[ a ];
        if ( record.steps.empty() )
            continue;
        anyAction = true;
        CORE::CString text = "### Action " + CORE::UInt32ToString( record.action.id ) + ": " + record.action.kind;
        if ( !record.action.label.IsNULLOrEmpty() )
            text += " \"" + record.action.label + "\"";
        if ( !record.action.page.IsNULLOrEmpty() )
            text += " on " + record.action.page;
        text += "\n\n";
        for ( size_t s=0; s<record.steps.size(); ++s )
            text += CORE::UInt32ToString( (UInt32) s + 1 ) + ". " + record.steps[ s ] + "\n";
        if ( record.droppedSteps > 0 )
            text += "- and " + CORE::UInt32ToString( record.droppedSteps ) + " more\n";
        text += "\n";
        if ( Guard( text, reason ) )
            out += text;
        else
            out += "### Action " + CORE::UInt32ToString( record.action.id ) + ": left out\n\n- This action would have contained a secret value that was seen during recording (" + reason +
                   "). Its label, page and steps are not shown.\n\n";
    }
    if ( !anyAction )
        out += "None observed.\n\n";
    if ( m_droppedActions > 0 )
        out += "Actions not kept (maxActions): " + CORE::UInt32ToString( m_droppedActions ) + "\n\n";

    out += "## Not reported (scripts, styles, fonts, tracking)\n\n";
    if ( m_ignoredCounts.empty() )
        out += "None.\n";
    for ( TCountMap::const_iterator i = m_ignoredCounts.begin(); i != m_ignoredCounts.end(); ++i )
    {
        const CORE::CString line = "- " + i->first + ": " + CORE::UInt32ToString( i->second ) + "\n";
        if ( Guard( line, reason ) )
            out += line;
        else
            out += "- (a type left out: " + reason + "): " + CORE::UInt32ToString( i->second ) + "\n";
    }

    // the head, which can name what was left out
    CORE::CString head;
    char seconds[ 64 ];
    snprintf( seconds, sizeof( seconds ), "%.1f", m_lastTimestamp - m_firstTimestamp );
    head += "# Endpoints observed during discovery\n\n";
    head += "Requests observed: " + CORE::UInt32ToString( m_requestCount ) + ", distinct endpoints: " + CORE::UInt32ToString( (UInt32) m_entries.size() ) +
            ", user actions: " + CORE::UInt32ToString( (UInt32) ( m_actions.size() + m_droppedActions ) ) + ", observed over " + seconds + " seconds.\n\n";
    if ( m_lastRedactedCount > 0 )
        head += "Parts of this report left out because they would have contained a secret value that was seen during recording: " + CORE::UInt32ToString( m_lastRedactedCount ) +
                " (" + GetRedactionSummary() + "). Only the class of the secret is named, never its value; every other part is complete.\n\n";
    if ( m_stubCount > 0 )
        head += "Journal lines that were reduced to a stub when they were written, for the same reason: " + CORE::UInt32ToString( m_stubCount ) +
                " (" + GetStubSummary() + "). They are counted in the requests above and not catalogued.\n\n";
    out = head + out;

    // a value that spans two parts is the only thing left that the parts could not see
    if ( m_redactor->ContainsKnownSecret( out ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "SiteEndpointCatalog: The finished report would contain a secret value that was seen during recording, it is refused" );
        report.Clear();
        return false;
    }
    report = out;
    if ( m_lastRedactedCount > 0 )
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_IMPORTANT, "SiteEndpointCatalog: " << m_lastRedactedCount << " part(s) of the report were left out because they would contain a secret value (" << GetRedactionSummary() << ")" );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
