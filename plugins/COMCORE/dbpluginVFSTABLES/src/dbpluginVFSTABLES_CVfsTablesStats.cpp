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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "dbpluginVFSTABLES_CVfsTablesStats.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsTablesStats::CVfsTablesStats( void )
    : directoryListings( 0 )
    , listingCacheHits( 0 )
    , fileProbes( 0 )
    , filesOpened( 0 )
    , footerReads( 0 )
    , footerCacheHits( 0 )
    , footerCacheEvictions( 0 )
    , footerCacheStale( 0 )
    , filesReplaced( 0 )
    , replaceRetries( 0 )
    , filesDeleted( 0 )
    , temporaryFilesRemoved( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsTablesStats::Clear( void )
{GUCEF_TRACE;

    *this = CVfsTablesStats();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
