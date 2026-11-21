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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static std::string
PathOf( const std::string& url )
{
    SUrlTemplate parsed;
    const CORE::CString path = CUrlTemplate::Parse( url, parsed ) ? parsed.path : CORE::CString( url );
    return path.Lowercase().STL_String();
}

/*-------------------------------------------------------------------------*/

template< class Container >
static void
CopyToSet( const Container& from, std::set< std::string >& to )
{
    to.clear();
    for ( typename Container::const_iterator i = from.begin(); i != from.end(); ++i )
        to.insert( i->STL_String() );
}

/*-------------------------------------------------------------------------*/

template< class Container >
static void
CopyToVector( const Container& from, std::vector< std::string >& to )
{
    to.clear();
    for ( typename Container::const_iterator i = from.begin(); i != from.end(); ++i )
        to.push_back( i->STL_String() );
}

/*-------------------------------------------------------------------------*/

static const WEB::CSiteDiscoveryRedactor&
GetCredentialRules( void )
{
    static const WEB::CSiteDiscoveryRedactor rules( CEndpointCatalog::MakeSettings( true ) );
    return rules;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEndpointCatalog::CEndpointCatalog( void )
    : WEB::CIWebObservationSink()
    , m_ownRedactor( MakeSettings( false ) )
    , m_catalog()
    , m_treeIds()
    , m_recordDna( false )
    , m_cache()
    , m_entries()
{GUCEF_TRACE;

    m_catalog.SetRedactor( &m_ownRedactor );
}

/*-------------------------------------------------------------------------*/

CEndpointCatalog::~CEndpointCatalog()
{GUCEF_TRACE;

    for ( TEntryCache::iterator i = m_cache.begin(); i != m_cache.end(); ++i )
        delete i->second;
    m_cache.clear();
    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

WEB::CSiteDiscoverySettings
CEndpointCatalog::MakeSettings( bool recordDna )
{GUCEF_TRACE;

    WEB::CSiteDiscoverySettings settings;
    settings.recordExampleValues = true;
    settings.noteNeverRecorded = true;
    if ( !recordDna )
    {
        static const char* const parts[] = { "/dna/", "/dna", "treesdna", "genetic", "discoveryui-matches", "thrulines", GUCEF_NULL };
        for ( UInt32 i=0; parts[ i ] != GUCEF_NULL; ++i )
            settings.neverRecordPathPatterns.push_back( parts[ i ] );
    }
    return settings;
}

/*-------------------------------------------------------------------------*/

CEndpointCatalog::EKind
CEndpointCatalog::Classify( const SObservedRequest& observation )
{GUCEF_TRACE;

    return (EKind) WEB::CSiteEndpointCatalog::Classify( observation.resourceType, observation.mimeType );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsCredentialUrl( const std::string& url )
{GUCEF_TRACE;

    return GetCredentialRules().IsNeverRecordUrl( url );
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

bool
CEndpointCatalog::IsSensitiveUrl( const std::string& url )
{GUCEF_TRACE;

    return IsCredentialUrl( url ) || IsDnaUrl( url );
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::SetRecordDna( bool record )
{GUCEF_TRACE;

    m_recordDna = record;
    m_ownRedactor.SetSettings( MakeSettings( record ) );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsRecordDna( void ) const
{GUCEF_TRACE;

    return m_recordDna;
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsExcluded( const std::string& url ) const
{GUCEF_TRACE;

    return m_catalog.GetRedactor().IsNeverRecordUrl( url );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsSecretHeader( const std::string& name )
{GUCEF_TRACE;

    // credentials and the tokens that tie a request to a login session
    return name == "cookie" || name == "set-cookie" || GetCredentialRules().IsSecretName( name );
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::SetSharedRedactor( WEB::CSiteDiscoveryRedactor* redactor )
{GUCEF_TRACE;

    m_catalog.SetRedactor( GUCEF_NULL != redactor ? redactor : &m_ownRedactor );
}

/*-------------------------------------------------------------------------*/

WEB::CSiteDiscoveryRedactor&
CEndpointCatalog::GetRedactor( void ) const
{GUCEF_TRACE;

    return m_catalog.GetRedactor();
}

/*-------------------------------------------------------------------------*/

WEB::CSiteEndpointCatalog&
CEndpointCatalog::GetGeneric( void )
{GUCEF_TRACE;

    return m_catalog;
}

/*-------------------------------------------------------------------------*/

WEB::CObservedWebRequest
CEndpointCatalog::ToObservation( const SObservedRequest& observation, const SJsonShape* shape )
{GUCEF_TRACE;

    WEB::CObservedWebRequest result;
    result.sequence = observation.sequence;
    result.timestamp = observation.timestamp;
    result.method = observation.method;
    result.url = observation.url;
    result.resourceType = observation.resourceType;
    result.mimeType = observation.mimeType;
    result.status = observation.status;
    result.encodedDataLength = observation.encodedDataLength;
    result.failed = observation.failed;
    result.errorText = observation.errorText;
    for ( std::map< std::string, std::string >::const_iterator h = observation.requestHeaders.begin(); h != observation.requestHeaders.end(); ++h )
        result.requestHeaders[ h->first ] = h->second;
    std::map< std::string, std::string >::const_iterator contentType = observation.requestHeaders.find( "content-type" );
    if ( contentType != observation.requestHeaders.end() )
        result.requestContentType = CORE::CString( contentType->second ).SubstrToChar( ';', true ).Trim( false ).Lowercase();
    result.postData = observation.postData;
    result.bodyFile = observation.bodyFile;
    if ( GUCEF_NULL != shape )
    {
        result.responseShape = *shape;
        result.hasResponseShape = true;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::NeedsShape( const SObservedRequest& observation ) const
{GUCEF_TRACE;

    return !observation.bodyFile.empty() && m_catalog.NeedsShape( ToObservation( observation, GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::MarkShapeTried( const SObservedRequest& observation )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::AddObservation( const WEB::CObservedWebRequest& observation )
{GUCEF_TRACE;

    // the tree id is the id that follows the 'tree' path segment, if the application uses such urls
    SUrlTemplate parsed;
    if ( CUrlTemplate::Parse( observation.url, parsed ) )
    {
        const CORE::CString treeId = CUrlTemplate::FindIdAfterSegment( parsed.path, "tree" );
        if ( !treeId.IsNULLOrEmpty() )
            m_treeIds.insert( treeId.STL_String() );
        const CORE::CString treesId = CUrlTemplate::FindIdAfterSegment( parsed.path, "trees" );
        if ( !treesId.IsNULLOrEmpty() )
            m_treeIds.insert( treesId.STL_String() );
    }
    return m_catalog.Add( observation );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::Add( const SObservedRequest& observation, const SJsonShape* shape )
{GUCEF_TRACE;

    return AddObservation( ToObservation( observation, shape ) );
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::OnWebRequestObserved( const WEB::CObservedWebRequest& observation )
{GUCEF_TRACE;

    AddObservation( observation );
}

/*-------------------------------------------------------------------------*/

void
CEndpointCatalog::OnUserActionObserved( const WEB::CObservedUserAction& action )
{GUCEF_TRACE;

    m_catalog.AddAction( action );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::WantsResponseShape( const WEB::CObservedWebRequest& observation ) const
{GUCEF_TRACE;

    return m_catalog.NeedsShape( observation );
}

/*-------------------------------------------------------------------------*/

bool
CEndpointCatalog::IsNewResponseShape( const WEB::CObservedWebRequest& observation, const WEB::CWebBodyShape& shape ) const
{GUCEF_TRACE;

    return m_catalog.IsNewResponseShape( observation, shape );
}

/*-------------------------------------------------------------------------*/

UInt32
CEndpointCatalog::GetRequestCount( void ) const
{GUCEF_TRACE;

    return m_catalog.GetRequestCount();
}

/*-------------------------------------------------------------------------*/

UInt32
CEndpointCatalog::GetEndpointCount( EKind kind ) const
{GUCEF_TRACE;

    return m_catalog.GetEndpointCount( (WEB::CSiteEndpointCatalog::EKind) kind );
}

/*-------------------------------------------------------------------------*/

SEndpointEntry*
CEndpointCatalog::Refresh( const WEB::CDiscoveredEndpoint& endpoint ) const
{GUCEF_TRACE;

    SEndpointEntry*& entry = m_cache[ &endpoint ];
    if ( GUCEF_NULL == entry )
        entry = new SEndpointEntry();

    entry->method = endpoint.method.STL_String();
    entry->host = endpoint.host.STL_String();
    entry->pathTemplate = endpoint.pathTemplate.STL_String();
    entry->kind = endpoint.kind;
    entry->firstSequence = endpoint.firstSequence;
    entry->requestCount = endpoint.requestCount;
    entry->statusCounts.clear();
    for ( WEB::CDiscoveredEndpoint::TStatusCountMap::const_iterator s = endpoint.statusCounts.begin(); s != endpoint.statusCounts.end(); ++s )
        entry->statusCounts[ s->first ] = s->second;
    CopyToSet( endpoint.resourceTypes, entry->resourceTypes );
    CopyToSet( endpoint.mimeTypes, entry->mimeTypes );
    CopyToVector( endpoint.exampleUrls, entry->exampleUrls );
    entry->queryParams.clear();
    for ( WEB::CDiscoveredEndpoint::TQueryParamMap::const_iterator q = endpoint.queryParams.begin(); q != endpoint.queryParams.end(); ++q )
        CopyToVector( q->second, entry->queryParams[ q->first.STL_String() ] );
    CopyToSet( endpoint.requestHeaderNames, entry->requestHeaderNames );
    CopyToVector( endpoint.postDataExamples, entry->postDataExamples );
    CopyToVector( endpoint.bodyFiles, entry->bodyFiles );
    entry->totalBytes = endpoint.totalBytes;
    entry->hasShape = endpoint.hasShape;
    entry->shapeTried = endpoint.shapeTried;
    entry->shape = endpoint.shape;
    return entry;
}

/*-------------------------------------------------------------------------*/

const std::vector< SEndpointEntry* >&
CEndpointCatalog::GetEntries( void ) const
{GUCEF_TRACE;

    const WEB::CSiteEndpointCatalog::TEndpointVector& endpoints = m_catalog.GetEntries();
    m_entries.clear();
    for ( size_t i=0; i<endpoints.size(); ++i )
        m_entries.push_back( Refresh( *endpoints[ i ] ) );
    return m_entries;
}

/*-------------------------------------------------------------------------*/

const SEndpointEntry*
CEndpointCatalog::FindEntry( const CORE::CString& method, const CORE::CString& url ) const
{GUCEF_TRACE;

    const WEB::CDiscoveredEndpoint* endpoint = m_catalog.FindEntry( method, url );
    return GUCEF_NULL == endpoint ? GUCEF_NULL : Refresh( *endpoint );
}

/*-------------------------------------------------------------------------*/

const std::set< std::string >&
CEndpointCatalog::GetTreeIds( void ) const
{GUCEF_TRACE;

    return m_treeIds;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CEndpointCatalog::RenderReport( void ) const
{GUCEF_TRACE;

    CORE::CString::StringVector extra;
    if ( !m_treeIds.empty() )
    {
        CORE::CString line = "Tree ids seen in urls: ";
        for ( std::set< std::string >::const_iterator i = m_treeIds.begin(); i != m_treeIds.end(); ++i )
            line += CORE::CString( i == m_treeIds.begin() ? "" : ", " ) + CORE::CString( *i );
        extra.push_back( line );
    }
    CORE::CString report;
    m_catalog.RenderReport( report, extra );
    return report;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
