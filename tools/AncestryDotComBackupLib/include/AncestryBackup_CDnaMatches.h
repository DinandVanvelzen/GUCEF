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

#ifndef ANCESTRYBACKUP_CDNAMATCHES_H
#define ANCESTRYBACKUP_CDNAMATCHES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#include <string>
#include <vector>
#include <map>

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ANCESTRYBACKUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One DNA match, in the form in which matches of any testing company can be kept and exchanged: a flat record with
 *  the things every company reports (who, how much DNA is shared, in how many pieces, how related that suggests)
 *  and the things that are particular to one company, kept as they are in 'extra'.
 */
struct SDnaMatch
{
    std::string provider;               /**< ancestry, 23andme, ftdna, gedmatch... */
    std::string testId;                 /**< the test of the person whose matches these are */
    std::string matchId;                /**< the test of the match, at the provider */
    std::string name;                   /**< as the provider shows it, often initials or a user name */
    std::string initials;
    std::string sex;
    double sharedCm;                    /**< shared centimorgans */
    UInt32 sharedSegments;
    double longestSegmentCm;            /**< 0 when unknown */
    std::string percentShared;          /**< as the provider gives it: 3.2, < 1 */
    std::string predictedRelationship;  /**< the label, such as '3rd cousin 1x removed' */
    std::string relationshipGroup;      /**< close family, extended family, distant family... as the provider groups it */
    std::string predictionSource;
    std::string parentSide;             /**< paternal, maternal, both, unknown... */
    std::string clusterCode;
    UInt32 meiosis;
    bool ignored;
    std::string hasTree;                /**< yes, no, private, unknown */
    std::string treeSize;
    std::string matchDate;              /**< when the match appeared, 2015-05-25 */
    std::string tags;                   /**< the tags of the user on this match, as the provider keeps them */
    std::string notes;
    std::string profileUrl;             /**< where to look at the match at the provider */
    std::string extra;                  /**< whatever else the provider said, as JSON */

    SDnaMatch( void );
};

/** A page of the match list */
struct SDnaMatchPage
{
    std::vector< SDnaMatch > matches;
    UInt32 currentPage;
    UInt32 totalPages;
    UInt32 matchCount;                  /**< all matches, 0 when the page does not say */
    bool isLastPage;

    SDnaMatchPage( void ) : matches(), currentPage( 0 ), totalPages( 0 ), matchCount( 0 ), isLastPage( false ) {}
};

/**
 *  The DNA matches of the user at Ancestry: the list that is paged through, and the details that the site gets for 20
 *  matches at a time. The result is kept as a CSV file that opens anywhere, and as the raw answers of the site.
 */
class CDnaMatches
{
    public:

    static std::string BuildListUrl( const std::string& testId, UInt32 page );

    static std::string BuildRelationshipsUrl( const std::string& testId );

    static std::string BuildProfilesUrl( const std::string& testId );

    /** {"matchSampleIds":["...","..."]} */
    static std::string BuildIdsBody( const std::vector< std::string >& ids );

    static bool ParseListPage( const CORE::CDataNode& document, const std::string& testId, SDnaMatchPage& page );

    /** Adds what the relationships answer tells about each match: predicted relationship, side, percent shared */
    static void MergeRelationships( const CORE::CDataNode& document, std::vector< SDnaMatch >& matches );

    /** Adds the names and the sex */
    static void MergeProfiles( const CORE::CDataNode& document, std::vector< SDnaMatch >& matches );

    static std::string CsvHeader( void );

    static std::string ToCsvRow( const SDnaMatch& match );

    /** A field of a CSV file: in quotes when it holds a comma, quote or line end, quotes doubled */
    static std::string CsvField( const std::string& text );

    /** Milliseconds since the epoch to 2015-05-25 */
    static std::string DateOf( UInt64 unixEpochInMs );

    /** The text of the sample ids in the list that are looked for in a journal of discovery, in the form the site uses them */
    static bool LooksLikeSampleId( const std::string& text );

    /** One line of the matches file: the match as JSON */
    static std::string ToJsonLine( const SDnaMatch& match );

    static bool FromJsonLine( const CORE::CDataNode& document, SDnaMatch& match );

    /** Finds the id of the DNA test of the user in a text that holds addresses of the site, such as a journal of discovery. Empty if it is not there. */
    static std::string FindTestId( const std::string& text );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CDNAMATCHES_H ? */
