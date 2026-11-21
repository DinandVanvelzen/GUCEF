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

#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGECONFORMANCE_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGECONFORMANCE_H

/**
 *  Runs the archive conformance suite of gucefVFS_TestApp on the bridge archive, mounted over a loopback share of the reference tree:
 *  once through the drn:// form of the source with the metadata cache on, once through a connection registered in code with the cache
 *  off and a share that knows the digests of its files, and once with a client node that has no thread of its own, which the thread that
 *  makes the calls pumps.
 */
void
PerformDrnBridgeConformanceTests( bool withCache = true, bool withoutCache = true, bool withPumpedClient = true );

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGECONFORMANCE_H ? */
