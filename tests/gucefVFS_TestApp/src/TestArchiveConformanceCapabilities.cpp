/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#include "TestArchiveConformanceCapabilities.h"

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CArchiveConformanceCapabilities::CArchiveConformanceCapabilities( void )
    : isWritable( false )
    , supportsDirectoryWatching( false )
    , hasModificationTimes( false )
    , modificationTimeResolutionInMs( 2000 )
    , revisionIsWeak( false )
    , canMutateTree( false )
    , canProvideOutsidePath( false )
    , supportsFileHash( false )
    , fileHashIsMd5( false )
    , digestAlgorithm( GUCEF::VFS::CVfsDigest::ALGORITHM_UNKNOWN )
    , supportsFileMetaData( false )
    , honoursMaxListEntries( false )
    , dirListHonoursIncludePath( false )
    , dirListRecursesIntoSubDirs( false )
    , dirEntriesHaveTrailingSeparator( false )
    , confinesPathsToArchive( false )
    , readHandlesRefuseWrites( false )
    , supportsLargeFiles( false )
    , storesEmptyFiles( false )
    , supportsPartialWrites( false )
    , supportsAtomicMove( false )
    , supportsDirOperations( false )
    , supportsConcurrentCallers( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/
