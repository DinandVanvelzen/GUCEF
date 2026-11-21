/*
 *  gucefASSETSTORE_TestApp: tests of the gucefASSETSTORE module
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

#ifndef TESTREMOTEASSETFILES_H
#define TESTREMOTEASSETFILES_H

/** The block cache of the files of a remote service on its own: blocks, a restart, a damaged block, the size limit with pins */
void PerformAssetStoreBlockCacheTests( void );

/**
 *  The files of a remote service on the client (the VFS archive "assetstore" over a client of a host on the loopback): every path of the
 *  service opens through the VFS, reads fetch the blocks they touch, a slow link with read ahead, cancel, offline, a damaged cache, the digest of the
 *  catalogue, downloads and keep offline with a link that drops, the stored thumbnails and the licence gate of the files for the roles
 */
void PerformAssetStoreRemoteFilesTests( void );

/** The licence gate of the files and thumbnails that a host serves: the owner devices, a viewer, the machine role (mcp) and a learning role */
void PerformAssetStoreRemoteFilesGateTests( void );

#endif /* TESTREMOTEASSETFILES_H ? */
