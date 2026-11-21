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


#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEBASELINE_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEBASELINE_H

/**
 *  Measurements of the bridge as a remote archive that a later phase of performance work starts from: the round trip of a stat, the time to the first byte,
 *  sequential reads cold and from the cache, the listing of a directory of 5000 entries and the memory the index of the block cache holds for 10000
 *  entries. Nothing is asserted: the numbers are logged and written to vfspluginDRN_TestApp_Baseline.txt. Not part of a default run: mode "baseline"
 */
void
PerformDrnBridgeBaseline( void );

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEBASELINE_H ? */
