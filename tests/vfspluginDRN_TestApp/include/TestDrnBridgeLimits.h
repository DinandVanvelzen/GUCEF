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

#ifndef GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGELIMITS_H
#define GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGELIMITS_H

/**
 *  The bridge under the limits of the serving side: one namespace client and one handle budget per (session, member) for all mounts of
 *  it, a request budget shared first come first served, retries of what the serving side still refuses: suite "limits"
 */
void PerformDrnBridgeLimitTests( void );

#endif /* GUCEF_VFSPLUGIN_DRN_TESTAPP_TESTDRNBRIDGELIMITS_H ? */
