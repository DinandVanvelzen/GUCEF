/*
 *  gucefVFS_TestApp: GUCEF test application for the VFS module
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

#ifndef GUCEF_VFS_TESTAPP_TESTVFSDIGEST_H
#define GUCEF_VFS_TESTAPP_TESTVFSDIGEST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The digest of a file with the name of its algorithm (CVfsDigest, CVfsDigestHasher, CArchive::GetFileDigest(),
 *  CVFS::GetFileDigest()), the result value of the file system access layer that tells that data is not here yet,
 *  and the call result of the VFS (CVfsCallResult)
 */
void
PerformVfsDigestTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_TESTAPP_TESTVFSDIGEST_H ? */
