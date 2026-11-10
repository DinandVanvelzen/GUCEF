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

#ifndef ANCESTRYBACKUP_CEXPORTFINDER_H
#define ANCESTRYBACKUP_CEXPORTFINDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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

/** A file which looks like the GEDCOM export of a tree */
struct SExportCandidate
{
    CORE::CString path;
    UInt64 modifiedAtInMs;          /**< since the unix epoch, UTC */
    UInt64 sizeInBytes;

    SExportCandidate( void ) : path(), modifiedAtInMs( 0 ), sizeInBytes( 0 ) {}
};

/**
 *  Finds GEDCOM exports of Ancestry that were saved recently, normally in the downloads directory
 *  of the user's browser. Nothing here touches the site.
 */
class CExportFinder
{
    public:

    /** The directories exports are looked for in if none are given: the downloads directory */
    static std::vector< CORE::CString > GetDefaultDirs( void );

    /**
     *  Does this file look like a GEDCOM file written by Ancestry? Only the first part of the file is read:
     *  the header has to be there and has to name Ancestry as the program that wrote the file.
     */
    static bool LooksLikeAncestryExport( const CORE::CString& path );

    /**
     *  Looks for exports modified within the maximum age. Directories are not searched deeper than the
     *  directory itself. A zip file that holds an export is not opened, it is reported through 'zipsSeen'.
     *  @param nowInMs      the current time, since the unix epoch, UTC
     *  @param candidates   receives what was found, newest first
     *  @param zipsSeen     receives recent zip files whose name suggests a GEDCOM export, if not null
     */
    static void FindRecent( const std::vector< CORE::CString >& dirs   ,
                            UInt64 nowInMs                              ,
                            UInt64 maxAgeInMs                           ,
                            std::vector< SExportCandidate >& candidates ,
                            std::vector< CORE::CString >* zipsSeen = GUCEF_NULL );

    /**
     *  Recent zip files which hold a GEDCOM file written by Ancestry, newest first. The name of the file is not trusted,
     *  the zip is opened to see what is in it.
     */
    static bool ZipHoldsAncestryExport( const CORE::CString& zipPath );

    static void FindRecentZips( const std::vector< CORE::CString >& dirs, UInt64 nowInMs, UInt64 maxAgeInMs, std::vector< CORE::CString >& zips );

    /** 20261015-173045 for the given time in UTC, used to name the copies of exports */
    static CORE::CString FormatTimestamp( UInt64 unixEpochInMs );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANCESTRYBACKUP */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ANCESTRYBACKUP_CEXPORTFINDER_H ? */
