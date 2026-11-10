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

#include <ctime>
#include <cstdio>
#include <cstdlib>

#include "AncestryBackup_CDnaMatches.h"

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

SDnaMatch::SDnaMatch( void )
    : provider( "ancestry" ), testId(), matchId(), name(), initials(), sex(), sharedCm( 0.0 ), sharedSegments( 0 ), longestSegmentCm( 0.0 ), percentShared(), predictedRelationship()
    , relationshipGroup(), predictionSource(), parentSide(), clusterCode(), meiosis( 0 ), ignored( false ), hasTree(), treeSize(), matchDate(), tags(), notes(), profileUrl(), extra()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::BuildListUrl( const std::string& testId, UInt32 page )
{GUCEF_TRACE;

    std::string url = "https://www.ancestry.com/api/dna/matches/" + testId + "/matches";
    if ( page > 1 )
        url += "?currentPage=" + std::to_string( (unsigned long long) page );
    return url;
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::BuildRelationshipsUrl( const std::string& testId )
{GUCEF_TRACE;

    return "https://www.ancestry.com/api/dna/matches/" + testId + "/relationships/data";
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::BuildProfilesUrl( const std::string& testId )
{GUCEF_TRACE;

    return "https://www.ancestry.com/api/dna/matches/" + testId + "/clusters/profileData";
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::BuildIdsBody( const std::vector< std::string >& ids )
{GUCEF_TRACE;

    std::string body = "{\"matchSampleIds\":[";
    for ( size_t i=0; i<ids.size(); ++i )
        body += ( i == 0 ? "\"" : ",\"" ) + ids[ i ] + "\"";
    return body + "]}";
}

/*-------------------------------------------------------------------------*/

bool
CDnaMatches::LooksLikeSampleId( const std::string& text )
{GUCEF_TRACE;

    if ( text.size() != 36 )
        return false;
    for ( size_t i=0; i<36; ++i )
    {
        const char c = text[ i ];
        const bool dash = i == 8 || i == 13 || i == 18 || i == 23;
        const bool hex = ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'f' ) || ( c >= 'A' && c <= 'F' );
        if ( dash ? c != '-' : !hex )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::DateOf( UInt64 unixEpochInMs )
{GUCEF_TRACE;

    const time_t seconds = (time_t) ( unixEpochInMs / 1000 );
    struct tm parts;
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    gmtime_s( &parts, &seconds );
    #else
    gmtime_r( &seconds, &parts );
    #endif
    char text[ 16 ];
    strftime( text, sizeof( text ), "%Y-%m-%d", &parts );
    return text;
}

/*-------------------------------------------------------------------------*/

static std::string
Upper( std::string text )
{
    for ( size_t i=0; i<text.size(); ++i )
        text[ i ] = (char) toupper( text[ i ] );
    return text;
}

/*-------------------------------------------------------------------------*/

bool
CDnaMatches::ParseListPage( const CORE::CDataNode& document, const std::string& testId, SDnaMatchPage& page )
{GUCEF_TRACE;

    page = SDnaMatchPage();
    const CORE::CDataNode* list = document.Find( "matchList" );
    if ( GUCEF_NULL == list )
        return false;

    page.currentPage = (UInt32) document.GetAttributeValue( "currentPage" ).AsUInt64( 0 );
    page.totalPages = (UInt32) document.GetAttributeValue( "totalPages" ).AsUInt64( 0 );
    page.matchCount = (UInt32) document.GetAttributeValue( "matchCount" ).AsUInt64( 0 );
    page.isLastPage = document.GetAttributeValue( "isLastPage" ).AsBool( false );

    for ( CORE::CDataNode::const_iterator i = list->ConstBegin(); i != list->ConstEnd(); ++i )
    {
        const CORE::CDataNode& node = *(*i);
        SDnaMatch match;
        match.testId = testId;
        match.matchId = Upper( node.GetAttributeValue( "sampleId" ).AsString().STL_String() );
        if ( match.matchId.empty() )
            continue;
        match.ignored = node.GetAttributeValue( "ignored" ).AsBool( false );
        match.clusterCode = node.GetAttributeValue( "matchClusterCode" ).AsString().STL_String();
        const UInt64 created = node.GetAttributeValue( "createdDate" ).AsUInt64( 0 );
        match.matchDate = created > 0 ? DateOf( created ) : std::string();

        const CORE::CDataNode* relationship = node.Find( "relationship" );
        if ( GUCEF_NULL != relationship )
        {
            match.sharedCm = relationship->GetAttributeValue( "sharedCentimorgans" ).AsFloat64( 0.0 );
            match.sharedSegments = (UInt32) relationship->GetAttributeValue( "numSharedSegments" ).AsUInt64( 0 );
            match.meiosis = (UInt32) relationship->GetAttributeValue( "meiosis" ).AsUInt64( 0 );
        }

        // the tags of the user: tag id and what the site keeps with it, such as the side of the family it worked out
        const CORE::CDataNode* tags = node.Find( "tags" );
        if ( GUCEF_NULL != tags )
        {
            for ( UInt32 t=0; t<tags->GetAttCount(); ++t )
            {
                const CORE::CDataNode::TKeyValuePair* tag = tags->GetAttribute( t );
                if ( GUCEF_NULL == tag )
                    continue;
                const std::string value = tag->second.AsString().STL_String();
                if ( value.empty() )
                    continue;
                match.tags += ( match.tags.empty() ? "" : ";" ) + tag->first.STL_String() + "=" + value;
            }
        }
        match.profileUrl = "https://www.ancestry.com/discoveryui-matches/compare/" + testId + "/with/" + match.matchId;
        page.matches.push_back( match );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static SDnaMatch*
Find( std::vector< SDnaMatch >& matches, const std::string& id )
{
    const std::string wanted = Upper( id );
    for ( size_t i=0; i<matches.size(); ++i )
        if ( matches[ i ].matchId == wanted )
            return &matches[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CDnaMatches::MergeRelationships( const CORE::CDataNode& document, std::vector< SDnaMatch >& matches )
{GUCEF_TRACE;

    for ( CORE::CDataNode::const_iterator i = document.ConstBegin(); i != document.ConstEnd(); ++i )
    {
        const CORE::CDataNode& node = *(*i);
        SDnaMatch* match = Find( matches, node.GetName().STL_String() );
        if ( GUCEF_NULL == match )
            continue;
        match->predictedRelationship = node.GetAttributeValue( "assignment" ).AsString().STL_String();
        match->predictionSource = node.GetAttributeValue( "predictionSource" ).AsString().STL_String();
        match->relationshipGroup = node.GetAttributeValue( "relationshipGroup" ).AsString().STL_String();
        match->percentShared = node.GetAttributeValue( "percentDna" ).AsString().STL_String();
        std::string side = node.GetAttributeValue( "parentSideFlag" ).AsString().STL_String();
        for ( size_t c=0; c<side.size(); ++c )
            side[ c ] = (char) tolower( side[ c ] );
        match->parentSide = side;
        if ( match->sharedCm == 0.0 )
            match->sharedCm = node.GetAttributeValue( "sharedCm" ).AsFloat64( 0.0 );
        if ( match->sharedSegments == 0 )
            match->sharedSegments = (UInt32) node.GetAttributeValue( "numSharedSegments" ).AsUInt64( 0 );
    }
}

/*-------------------------------------------------------------------------*/

void
CDnaMatches::MergeProfiles( const CORE::CDataNode& document, std::vector< SDnaMatch >& matches )
{GUCEF_TRACE;

    for ( CORE::CDataNode::const_iterator i = document.ConstBegin(); i != document.ConstEnd(); ++i )
    {
        const CORE::CDataNode& node = *(*i);
        SDnaMatch* match = Find( matches, node.GetName().STL_String() );
        if ( GUCEF_NULL == match )
            continue;
        match->name = node.GetAttributeValue( "matchName" ).AsString().STL_String();
        match->initials = node.GetAttributeValue( "matchNameInitials" ).AsString().STL_String();
        match->sex = node.GetAttributeValue( "displayGender" ).AsString().STL_String();
    }
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::CsvField( const std::string& text )
{GUCEF_TRACE;

    if ( text.find_first_of( ",\"\r\n" ) == std::string::npos )
        return text;
    std::string quoted = "\"";
    for ( size_t i=0; i<text.size(); ++i )
    {
        if ( text[ i ] == '"' )
            quoted += '"';
        quoted += text[ i ];
    }
    return quoted + "\"";
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::CsvHeader( void )
{GUCEF_TRACE;

    return "provider,test_id,match_id,name,initials,sex,shared_cm,shared_segments,longest_segment_cm,percent_shared,predicted_relationship,relationship_group,prediction_source,"
           "parent_side,cluster_code,meiosis,ignored,has_tree,tree_size,match_date,tags,notes,profile_url";
}

/*-------------------------------------------------------------------------*/

static std::string
Number( double value )
{
    char text[ 32 ];
    if ( value == (double) (long long) value )
        snprintf( text, sizeof( text ), "%lld", (long long) value );
    else
        snprintf( text, sizeof( text ), "%.2f", value );
    return text;
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::ToCsvRow( const SDnaMatch& m )
{GUCEF_TRACE;

    return CsvField( m.provider ) + "," + CsvField( m.testId ) + "," + CsvField( m.matchId ) + "," + CsvField( m.name ) + "," + CsvField( m.initials ) + "," + CsvField( m.sex ) + "," +
           Number( m.sharedCm ) + "," + std::to_string( (unsigned long long) m.sharedSegments ) + "," + ( m.longestSegmentCm > 0.0 ? Number( m.longestSegmentCm ) : std::string() ) + "," +
           CsvField( m.percentShared ) + "," + CsvField( m.predictedRelationship ) + "," + CsvField( m.relationshipGroup ) + "," + CsvField( m.predictionSource ) + "," + CsvField( m.parentSide ) + "," +
           CsvField( m.clusterCode ) + "," + std::to_string( (unsigned long long) m.meiosis ) + "," + ( m.ignored ? "true" : "false" ) + "," + CsvField( m.hasTree ) + "," + CsvField( m.treeSize ) + "," +
           CsvField( m.matchDate ) + "," + CsvField( m.tags ) + "," + CsvField( m.notes ) + "," + CsvField( m.profileUrl );
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::ToJsonLine( const SDnaMatch& m )
{GUCEF_TRACE;

    #define J(x) CUrlTemplate::EscapeJson( (x).c_str() ).STL_String()
    return "{\"provider\":\"" + J( m.provider ) + "\",\"testId\":\"" + J( m.testId ) + "\",\"matchId\":\"" + J( m.matchId ) + "\",\"name\":\"" + J( m.name ) + "\",\"initials\":\"" + J( m.initials ) +
           "\",\"sex\":\"" + J( m.sex ) + "\",\"sharedCm\":" + Number( m.sharedCm ) + ",\"sharedSegments\":" + std::to_string( (unsigned long long) m.sharedSegments ) +
           ",\"longestSegmentCm\":" + Number( m.longestSegmentCm ) + ",\"percentShared\":\"" + J( m.percentShared ) + "\",\"predictedRelationship\":\"" + J( m.predictedRelationship ) +
           "\",\"relationshipGroup\":\"" + J( m.relationshipGroup ) + "\",\"predictionSource\":\"" + J( m.predictionSource ) + "\",\"parentSide\":\"" + J( m.parentSide ) +
           "\",\"clusterCode\":\"" + J( m.clusterCode ) + "\",\"meiosis\":" + std::to_string( (unsigned long long) m.meiosis ) + ",\"ignored\":" + ( m.ignored ? "true" : "false" ) +
           ",\"hasTree\":\"" + J( m.hasTree ) + "\",\"treeSize\":\"" + J( m.treeSize ) + "\",\"matchDate\":\"" + J( m.matchDate ) + "\",\"tags\":\"" + J( m.tags ) +
           "\",\"notes\":\"" + J( m.notes ) + "\",\"profileUrl\":\"" + J( m.profileUrl ) + "\"}";
    #undef J
}


/*-------------------------------------------------------------------------*/

bool
CDnaMatches::FromJsonLine( const CORE::CDataNode& d, SDnaMatch& m )
{GUCEF_TRACE;

    m = SDnaMatch();
    m.matchId = d.GetAttributeValue( "matchId" ).AsString().STL_String();
    if ( m.matchId.empty() )
        return false;
    m.provider = d.GetAttributeValue( "provider" ).AsString().STL_String();
    m.testId = d.GetAttributeValue( "testId" ).AsString().STL_String();
    m.name = d.GetAttributeValue( "name" ).AsString().STL_String();
    m.initials = d.GetAttributeValue( "initials" ).AsString().STL_String();
    m.sex = d.GetAttributeValue( "sex" ).AsString().STL_String();
    m.sharedCm = d.GetAttributeValue( "sharedCm" ).AsFloat64( 0.0 );
    m.sharedSegments = (UInt32) d.GetAttributeValue( "sharedSegments" ).AsUInt64( 0 );
    m.longestSegmentCm = d.GetAttributeValue( "longestSegmentCm" ).AsFloat64( 0.0 );
    m.percentShared = d.GetAttributeValue( "percentShared" ).AsString().STL_String();
    m.predictedRelationship = d.GetAttributeValue( "predictedRelationship" ).AsString().STL_String();
    m.relationshipGroup = d.GetAttributeValue( "relationshipGroup" ).AsString().STL_String();
    m.predictionSource = d.GetAttributeValue( "predictionSource" ).AsString().STL_String();
    m.parentSide = d.GetAttributeValue( "parentSide" ).AsString().STL_String();
    m.clusterCode = d.GetAttributeValue( "clusterCode" ).AsString().STL_String();
    m.meiosis = (UInt32) d.GetAttributeValue( "meiosis" ).AsUInt64( 0 );
    m.ignored = d.GetAttributeValue( "ignored" ).AsBool( false );
    m.hasTree = d.GetAttributeValue( "hasTree" ).AsString().STL_String();
    m.treeSize = d.GetAttributeValue( "treeSize" ).AsString().STL_String();
    m.matchDate = d.GetAttributeValue( "matchDate" ).AsString().STL_String();
    m.tags = d.GetAttributeValue( "tags" ).AsString().STL_String();
    m.notes = d.GetAttributeValue( "notes" ).AsString().STL_String();
    m.profileUrl = d.GetAttributeValue( "profileUrl" ).AsString().STL_String();
    return true;
}

/*-------------------------------------------------------------------------*/

std::string
CDnaMatches::FindTestId( const std::string& text )
{GUCEF_TRACE;

    static const char* const marker = "/api/dna/matches/";
    size_t pos = 0;
    while ( ( pos = text.find( marker, pos ) ) != std::string::npos )
    {
        pos += strlen( marker );
        if ( pos + 36 <= text.size() )
        {
            const std::string candidate = text.substr( pos, 36 );
            if ( LooksLikeSampleId( candidate ) )
                return Upper( candidate );
        }
    }
    return std::string();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
