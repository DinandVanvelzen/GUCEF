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

#ifndef GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCE_H
#define GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "TestArchiveConformanceFixture.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The archive conformance suite: the behaviour every archive type that can be mounted into the VFS has to show, run
 *  against the archive of a fixture. It sets the fixture up, runs the cases and tears it down again.
 *
 *  What it checks, against the reference tree of the fixture (CArchiveConformanceTree):
 *  - FileExists(), DirExists() and GetFileSize() for present and absent names, and for a file where a directory is
 *    asked for and the other way around
 *  - GetFileList() and GetDirList(): recursive or not, name filters, the path in the entries (the include flag), a
 *    directory of several hundred files, empty directories, absent locations, the forms a location can have
 *  - GetFile(): byte exact content of every file, in a read from start to end and as ranged reads (Setpos() and
 *    Seek() of the returned CIOAccess, Tell(), Eof(), a read past the end), two files open at once, the same file
 *    opened twice, reads from several threads at once, a file above 4 GiB
 *  - GetFileHash(), GetFileModificationTime() and GetFileMetaData(), and the behaviour for a file that is not there
 *  - IsHealthy(), IsConnected(), the watch bookkeeping and the calls after UnloadArchive()
 *  - when the fixture can change the content behind the archive: that a changed, added or removed file shows
 *  - when the archive is writable: storing, overwriting, writing through a handle, appending, deleting, moving files
 *    and creating, moving and deleting directories
 *
 *  What a backend offers beyond the mandatory is reported by the capabilities of its fixture: a case that depends on a
 *  capability the fixture lacks is not run, so a remote archive runs the same suite as the file system archive.
 *
 *  The results go to the test framework: one suite named after the fixture, a test case per behaviour.
 */
void
PerformArchiveConformanceSuite( CArchiveConformanceFixture& fixture );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTARCHIVECONFORMANCE_H ? */
