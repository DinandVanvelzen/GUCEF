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

#ifndef ANCESTRYBACKUP_CGEDCOMSUMMARY_H
#define ANCESTRYBACKUP_CGEDCOMSUMMARY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef ANCESTRYBACKUP_CGEDCOMREADER_H
#include "AncestryBackup_CGedcomReader.h"
#define ANCESTRYBACKUP_CGEDCOMREADER_H
#endif /* ANCESTRYBACKUP_CGEDCOMREADER_H ? */

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

/** What a GEDCOM file holds, counted */
struct SGedcomSummary
{
    // header
    std::string sourceSystem;               /**< HEAD.SOUR, for example Ancestry.com Family Trees */
    std::string sourceName;
    std::string sourceVersion;
    std::string gedcomVersion;              /**< HEAD.GEDC.VERS */
    std::string gedcomForm;
    std::string characterSet;               /**< HEAD.CHAR */
    std::string headerDate;
    std::string headerTime;                 /**< HEAD.DATE.TIME, UTC for Ancestry */
    std::string fileName;                   /**< HEAD.FILE */
    std::string treeName;                   /**< HEAD.SOUR._TREE */
    std::string treeId;                     /**< HEAD.SOUR._TREE.RIN: the id the site uses for the tree */

    // records
    UInt32 individuals;
    UInt32 families;
    UInt32 sources;
    UInt32 repositories;
    UInt32 notes;                           /**< records of their own */
    UInt32 mediaObjects;                    /**< records of their own */
    UInt32 submitters;

    // what hangs off the records
    UInt32 individualsWithNotes;
    UInt32 citations;                       /**< SOUR lines that point to a source record, below an individual or a family */
    UInt32 mediaLinks;                      /**< OBJE lines below anything but the file level */
    UInt32 mediaFiles;                      /**< FILE lines below an OBJE */
    UInt32 mediaFilesThatAreUrls;
    UInt32 ancestryIds;                     /**< _APID lines: the Ancestry record a source citation came from */

    std::map< std::string, UInt32 > individualTags;     /**< tags directly below an individual, and how often */
    std::map< std::string, UInt32 > familyTags;
    std::vector< std::string > individualIds;           /**< the ids of the individual records, as in the file */
    std::vector< std::string > mediaFileNames;

    UInt32 lines;
    UInt32 malformedLines;
    bool hasHeader;
    bool hasTrailer;

    SGedcomSummary( void );
};

/**
 *  Counts what is in a GEDCOM file. Used on the file Ancestry exports of a tree to learn what that export holds,
 *  and with that, what it leaves out and has to come from the site.
 */
class CGedcomSummarizer : public CIGedcomHandler
{
    public:

    CGedcomSummarizer( void );

    virtual void OnLine( const SGedcomLine& line ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnMalformedLine( UInt32 lineNumber ) GUCEF_VIRTUAL_OVERRIDE;

    const SGedcomSummary& GetSummary( void ) const;

    /** Readable text, with notes on what is not in the file */
    static CORE::CString Render( const SGedcomSummary& summary );

    private:

    SGedcomSummary m_summary;
    std::vector< std::string > m_tagByLevel;        // the tag that is current on each level
    bool m_currentIndividualHasNote;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CGEDCOMSUMMARY_H ? */
