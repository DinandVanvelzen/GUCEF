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

#ifndef ANCESTRYBACKUP_CGRAMPSEXPORTER_H
#define ANCESTRYBACKUP_CGRAMPSEXPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#include <string>
#include <vector>

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

/** What to convert and how */
struct SGrampsExportOptions
{
    CORE::CString gedcomPath;               /**< the GEDCOM file to convert */
    CORE::CString mediaDir;                 /**< where the downloaded files are, named <media id>.<extension> */
    CORE::CString outputPath;               /**< the Gramps XML file that is written */
    std::string mediaBasePath;              /**< what Gramps takes as its base media path. Default: the absolute path of mediaDir */
    std::string homePerson;                 /**< id of the person in the GEDCOM file that is the home person, default: the first one in the file */
    std::string researcherName;
    std::string researcherEmail;
    std::string treeId;                     /**< the id of the tree at the site, used for a link to every person. Default: the one in the header of the GEDCOM file */

    SGrampsExportOptions( void ) : gedcomPath(), mediaDir(), outputPath(), mediaBasePath(), homePerson(), researcherName(), researcherEmail(), treeId() {}
};

/** What came out of a conversion */
struct SGrampsExportResult
{
    UInt32 people;
    UInt32 families;
    UInt32 events;
    UInt32 sources;
    UInt32 citations;
    UInt32 repositories;
    UInt32 places;
    UInt32 notes;
    UInt32 mediaObjects;
    UInt32 mediaFilesPresent;               /**< media objects which have their file in the media directory */
    UInt32 mediaFilesMissing;               /**< media objects of which the file was not downloaded, they are tagged */
    UInt32 datesExact;                      /**< dates which Gramps can compute with */
    UInt32 datesAsText;                     /**< dates which were not understood and are kept as text */
    UInt32 danglingReferences;              /**< references to records which are not in the file, left out */
    UInt64 bytesWritten;
    std::string gedcomTime;                 /**< the time of the export that was converted */
    std::string homePerson;                 /**< the xref of the home person that was used */
    std::vector< std::string > warnings;
    std::vector< std::string > personIds;   /**< the ids of the people that were converted, in the order of the file */

    SGrampsExportResult( void ) : people( 0 ), families( 0 ), events( 0 ), sources( 0 ), citations( 0 ), repositories( 0 ), places( 0 ), notes( 0 ), mediaObjects( 0 ), mediaFilesPresent( 0 ), mediaFilesMissing( 0 ), datesExact( 0 ), datesAsText( 0 ), danglingReferences( 0 ), bytesWritten( 0 ), gedcomTime(), homePerson(), warnings(), personIds() {}
};

/**
 *  Turns the GEDCOM export of an Ancestry.com tree and the media that was downloaded into a Gramps XML file (version 1.7.2,
 *  validates against the DTD and RelaxNG schema of Gramps) which Gramps takes in as a tree of its own: people with their names
 *  and events, families, places as a hierarchy, sources with their repositories, citations, notes and media linked to the people
 *  and events they belong to, with the cropped region of a profile picture.
 *
 *  The same GEDCOM file always gives the same file: handles and ids come from the ids Ancestry uses or from what the object holds,
 *  so converting a newer export gives a file in which what did not change is the same. Import it into an empty Gramps tree; Gramps
 *  gives new handles to what it imports into a tree which has people in it, so importing again into the same tree adds again.
 *
 *  Nothing is lost on the way: what has no place in Gramps (the tags the site added for itself) goes into attributes, and
 *  the GEDCOM file itself stays in the data of the tree.
 */
class CGrampsExporter
{
    public:

    static bool Export( const SGrampsExportOptions& options, SGrampsExportResult& result );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CGRAMPSEXPORTER_H ? */
