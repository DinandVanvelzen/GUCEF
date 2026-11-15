/*
 *  vfspluginUNITYPACKAGE_TestApp: tests of the Unity package VFS plugin
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

#ifndef GUCEF_VFSPLUGIN_UNITYPACKAGE_TESTAPP_TESTUNITYPACKAGE_H
#define GUCEF_VFSPLUGIN_UNITYPACKAGE_TESTAPP_TESTUNITYPACKAGE_H

/**
 *  Tests vfspluginUNITYPACKAGE: Unity packages written with the libarchive writer are mounted via the VFS
 *  as a file, as an entry of a zip archive and, when the environment variable GUCEF_TEST_UNITYPACKAGE
 *  holds the path of a package file, as that real package, which is then only listed and read, never copied.
 */
void
PerformUnityPackageTests( void );

#endif /* GUCEF_VFSPLUGIN_UNITYPACKAGE_TESTAPP_TESTUNITYPACKAGE_H ? */
