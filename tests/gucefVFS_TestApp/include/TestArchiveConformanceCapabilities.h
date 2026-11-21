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

#ifndef GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCECAPABILITIES_H
#define GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCECAPABILITIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_VFS_CVFSDIGEST_H
#include "gucefVFS_CVfsDigest.h"
#define GUCEF_VFS_CVFSDIGEST_H
#endif /* GUCEF_VFS_CVFSDIGEST_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the archive behind a conformance fixture can do and where it knowingly deviates from the reference behaviour.
 *  The suite checks a behaviour whenever the fixture says it has it, and checks nothing about a behaviour that depends on a
 *  capability the fixture does not have. All members default to the most conservative value (the least is promised),
 *  a fixture switches on what its archive offers.
 *
 *  The suite makes no assumption about local disk semantics: a remote archive reports its own capabilities and runs the
 *  same cases. Deviations of the reference implementation itself are listed here as capabilities that are off, so they
 *  are visible in one place.
 */
class CArchiveConformanceCapabilities
{
    public:

    CArchiveConformanceCapabilities( void );

    /** Whether the archive accepts writes: the write cases (store, create, move, delete) are run when this is set. Must equal CArchive::IsWriteable() */
    bool isWritable;

    /** Whether IsDirectoryWatchingSupported() is true. The suite checks the bookkeeping of watches, not that events arrive */
    bool supportsDirectoryWatching;

    /** Whether the archive reports real modification times (GetFileModificationTime() changes when a file changes) */
    bool hasModificationTimes;

    /** The finest difference in modification time the archive can show, in milliseconds. The suite waits this long between two versions of a file */
    GUCEF::UInt32 modificationTimeResolutionInMs;

    /**
     *  Whether a change of a file that keeps its size can go unnoticed in the size and modification time (a weak revision).
     *  The suite then does not require that a same-size change shows in them, or in the content read back
     */
    bool revisionIsWeak;

    /** Whether the fixture can change the content behind the archive (CArchiveConformanceFixture::PutFile() and RemoveFile()), which runs the change detection cases */
    bool canMutateTree;

    /** Whether the fixture can make a path that leads out of the archive to something that exists (CArchiveConformanceFixture::GetPathOutsideArchive()) */
    bool canProvideOutsidePath;

    /** Whether GetFileHash() returns a hash of the content (the empty string for every file when not) */
    bool supportsFileHash;

    /** Whether that hash is the MD5 of the content in the form CORE::MD5ToString() gives, a stronger statement than consistency */
    bool fileHashIsMd5;

    /** The algorithm that CArchive::GetFileDigest() names for the files of the archive: ALGORITHM_UNKNOWN for an archive that does not override it (it then gives the GetFileHash() string) */
    GUCEF::VFS::CVfsDigest::EAlgorithm digestAlgorithm;

    /** Whether GetFileMetaData() is implemented (resource exists, size, times) */
    bool supportsFileMetaData;

    /** Whether the lists stop at maxListEntries. When not, the limit is advice the archive ignores */
    bool honoursMaxListEntries;

    /** Whether GetDirList() puts the path in the entries (mount location, archive location) when asked to include the parent directory */
    bool dirListHonoursIncludePath;

    /** Whether a recursive GetDirList() lists the directories at every depth, instead of the first level only */
    bool dirListRecursesIntoSubDirs;

    /** Whether every entry of GetDirList() ends with a path separator */
    bool dirEntriesHaveTrailingSeparator;

    /** Whether FileExists(), GetFile() and the others refuse a path with ".." that leads out of the archive */
    bool confinesPathsToArchive;

    /** Whether the access of a file opened for reading reports that it cannot be written to (a file kept in memory has to be protected from writes too) */
    bool readHandlesRefuseWrites;

    /** Whether the archive handles a file above 4 GiB (the fixture puts one in the tree) */
    bool supportsLargeFiles;

    /** Whether StoreAsFile() with a size of zero makes an empty file */
    bool storesEmptyFiles;

    /** Whether StoreAsFile() writes at an offset into an existing file when asked not to overwrite */
    bool supportsPartialWrites;

    /** Whether MoveFileAtomically() is offered (the default of the archive interface fails) */
    bool supportsAtomicMove;

    /** Whether CreateDir(), DeleteDir() and MoveDir() are offered */
    bool supportsDirOperations;

    /**
     *  Whether several threads may call the archive at once, which the parallel case does. Not when the thread that calls
     *  also has to work what the archive waits for (a remote archive over a node that the calling thread pumps): its calls
     *  work the node while they wait, a second thread waits for that thread, which is not pumping while it waits for the second
     */
    bool supportsConcurrentCallers;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCECAPABILITIES_H ? */
