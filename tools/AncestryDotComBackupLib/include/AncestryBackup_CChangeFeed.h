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

#ifndef ANCESTRYBACKUP_CCHANGEFEED_H
#define ANCESTRYBACKUP_CCHANGEFEED_H

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
#include <set>

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

/** One change in the tree, as the site's change feed lists it */
struct SChangeEvent
{
    std::string action;             /**< add_fact, edit_fact, add_citation and so on */
    std::string time;               /**< as the site gives it: 2026-10-16T21:27:48Z */
    UInt64 timeInMs;                /**< since the unix epoch, UTC */
    std::string personId;
    std::string factId;
    std::string factType;
    std::string referenceId;
    std::string authorId;
    bool personNoLongerExists;

    SChangeEvent( void ) : action(), time(), timeInMs( 0 ), personId(), factId(), factType(), referenceId(), authorId(), personNoLongerExists( false ) {}
};

/** A page of the feed */
struct SChangePage
{
    std::vector< SChangeEvent > events;     /**< newest first */
    std::string nextCursor;                 /**< what to ask for the page after this one, empty when there is none */
};

/**
 *  The change feed of a tree, the list of what was added, edited and removed with when and by whom. It is the cheap way
 *  to learn what happened since the last time: the feed is read from the newest end until it reaches a change that is
 *  older than the last one that was seen.
 */
class CChangeFeed
{
    public:

    /** The address of a page. The cursor of the previous page is what makes it the next page, empty for the first one */
    static std::string BuildUrl( const std::string& treeId, UInt32 limit, const std::string& cursor );

    static bool ParsePage( const CORE::CDataNode& document, SChangePage& page );

    /** 2026-10-16T21:27:48Z, with or without fractions of a second, to milliseconds since the epoch. false if it is not a time */
    static bool ParseTime( const std::string& text, UInt64& unixEpochInMs );

    /** The date and time of a GEDCOM header: 16 Oct 2026 and 23:14:06, taken to be UTC. The time may be empty. */
    static bool ParseGedcomDateTime( const std::string& date, const std::string& time, UInt64& unixEpochInMs );

    /** The reverse, in the form the site uses */
    static std::string FormatTime( UInt64 unixEpochInMs );

    /** The people the changes are about, each once, in order of first appearance */
    static std::vector< std::string > GetChangedPeople( const std::vector< SChangeEvent >& events );

    /** One line of text for the journal of changes */
    static std::string ToJournalLine( const SChangeEvent& event );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CCHANGEFEED_H ? */
