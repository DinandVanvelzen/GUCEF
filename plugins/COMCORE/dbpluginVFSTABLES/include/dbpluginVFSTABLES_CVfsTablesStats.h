/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESSTATS_H
#define DBPLUGIN_VFSTABLES_CVFSTABLESSTATS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef DBPLUGIN_VFSTABLES_MACROS_H
#include "dbpluginVFSTABLES_macros.h"
#define DBPLUGIN_VFSTABLES_MACROS_H
#endif /* DBPLUGIN_VFSTABLES_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Counters of the storage work of a vfstables database, over all its tables, since it was opened or the counters
 *  were reset. The pruning of one scan is in its CDataTableScanStats; these show what reached the VFS.
 */
class DBPLUGIN_VFSTABLES_PUBLIC_CPP CVfsTablesStats
{
    public:

    UInt64 directoryListings;           /**< directories listed through the VFS */
    UInt64 listingCacheHits;            /**< listings served from the listing cache */
    UInt64 fileProbes;                  /**< existence checks of partition files whose names were built from predicates */
    UInt64 filesOpened;                 /**< partition files opened for reading */
    UInt64 footerReads;                 /**< file metadata read from storage */
    UInt64 footerCacheHits;             /**< file metadata taken from the footer cache */
    UInt64 footerCacheEvictions;        /**< footer cache entries dropped for the budget */
    UInt64 footerCacheStale;            /**< cached footers that no longer matched their file when it was opened, read again */
    UInt64 filesReplaced;               /**< partition and definition files put in place */
    UInt64 replaceRetries;              /**< replaces that were refused and tried again */
    UInt64 filesDeleted;                /**< partition and definition files deleted */
    UInt64 temporaryFilesRemoved;       /**< leftovers of this writer removed at open, and temporary files of failed writes */

    CVfsTablesStats( void );

    void Clear( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESSTATS_H ? */
