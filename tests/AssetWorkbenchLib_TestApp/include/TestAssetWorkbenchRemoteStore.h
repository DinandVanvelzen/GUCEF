/*
 *  AssetWorkbenchLib_TestApp: tests of the AssetWorkbenchLib module
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

#ifndef ASSETWORKBENCHLIB_TESTAPP_TESTREMOTESTORE_H
#define ASSETWORKBENCHLIB_TESTAPP_TESTREMOTESTORE_H

/**
 *  The connection of the workbench to a remote asset store service, against a real service host on the loopback interface of the test process:
 *  finding the service, pairing with a code (and a wrong code refused in words), connecting, a dropped link and the reconnect, the search through the
 *  remote service, the sources tab on it and the way back to the store of this process, the page Remote store, the console commands and the menu
 */
void PerformAssetWorkbenchRemoteStoreTests( void );

void PerformAssetWorkbenchRemoteThumbnailTests( void );

/** The files of the remote store on the shares of the service: the thread of the interface that never waits, the settings and cache of the previous version, a keep offline that is refused */
void PerformAssetWorkbenchRemoteMigrationTests( void );

/** The Sources tab on a remote store and the opening of what it lists: an item of an online store, a pack or an asset of the library */
void PerformAssetWorkbenchRemoteOpenTests( void );

#endif /* ASSETWORKBENCHLIB_TESTAPP_TESTREMOTESTORE_H ? */
