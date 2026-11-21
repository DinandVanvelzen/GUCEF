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

#ifndef TESTREMOTEASSETSHARES_H
#define TESTREMOTEASSETSHARES_H

/**
 *  The files of a remote service on the client through the share of the host (the VFS archive "drn" mounted by CRemoteAssetShares): the
 *  layout of the shares, files that open through the VFS, blocks that are fetched and read ahead, offline copies, the thread that never
 *  waits, kept files, digests, downloads, the settings and the stored thumbnails
 */
void PerformAssetStoreRemoteSharesTests( void );

#endif /* TESTREMOTEASSETSHARES_H ? */
