/*
 *  vfspluginDRN_TestApp: tests of the DRN VFS plugin
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


#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEREMOTECASES_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEREMOTECASES_H

/**
 *  What the bridge adds to the generic remote archive layer, and what happens to it when the link, the session and the share misbehave: the settings,
 *  the identity in the block cache, the open without a handle, digests, hints and the invalidation through the VFS, the statistics, the state of the
 *  link as an event, a thread that never waits on a slow link, a single thread application, failures injected into the link and the share, the cache
 *  after a restart, pins: suite "remotecases"
 */
void
PerformDrnBridgeRemoteCases( void );

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEREMOTECASES_H ? */
