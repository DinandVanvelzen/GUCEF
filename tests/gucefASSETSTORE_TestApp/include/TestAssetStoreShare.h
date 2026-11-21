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

#ifndef TESTASSETSTORESHARE_H
#define TESTASSETSTORESHARE_H

/**
 *  The primary store of the asset store service as a share of the DRN resource facility of its host, mounted by a device through the VFS archive of
 *  type "drn": names, sizes, listings and bytes of the files, the digests of the catalogue, and the same rules of the byte gate as the Files contract
 *  (the machine role, the licence flags of the packs, the learning roles, the pictures a device was given), with the roles taken from the registry at
 *  every call, the share following the state of the locations, and the restrictions kept for the thread of the network node
 */
void PerformAssetStoreShareTests( void );

#endif /* TESTASSETSTORESHARE_H ? */
