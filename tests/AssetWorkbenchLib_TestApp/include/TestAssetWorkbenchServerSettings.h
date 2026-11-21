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

#ifndef ASSETWORKBENCHLIB_TESTAPP_TESTSERVERSETTINGS_H
#define ASSETWORKBENCHLIB_TESTAPP_TESTSERVERSETTINGS_H

/**
 *  The settings of the remote asset store service the workbench is connected to (gucefASSETSTORE_RemoteService_Design.md 18.5), against a real
 *  service host on the loopback interface of the test process: the menu item Server/Settings... only while connected, the page with the
 *  settings, their source and apply mode and the sync profiles of the stores, a change sent with Apply and applied by the server, Reset to site
 *  value, the editors refusing a value out of range, the page read only for a device without the owner role and for a server that does not
 *  allow changes, a change by another client shown, and the console commands server settings, server set and server reset
 */
void PerformAssetWorkbenchServerSettingsTests( void );

#endif /* ASSETWORKBENCHLIB_TESTAPP_TESTSERVERSETTINGS_H ? */
