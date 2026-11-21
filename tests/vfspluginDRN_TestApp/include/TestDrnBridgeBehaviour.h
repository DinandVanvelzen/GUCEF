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

#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEBEHAVIOUR_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEBEHAVIOUR_H

/**
 *  The cases of the bridge archive beyond the conformance suite: what happens to an open file when the share changes, when the link is lost and
 *  comes back, the pool of handles, the cost of lists in calls, readers on several threads, mounting through the VFS, and the refusals.
 */
void
PerformDrnBridgeBehaviourTests( void );

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGEBEHAVIOUR_H ? */
