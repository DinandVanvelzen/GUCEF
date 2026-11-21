/*
 *  assetpluginUNITY_TestApp: Test application for the Unity plugin of the asset converter (assetpluginUNITY)
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

#ifndef GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYPACKINDEXCORPUS_H
#define GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYPACKINDEXCORPUS_H

/**
 *  The pack index on a real package: when the environment variable GUCEF_TEST_ASSET_CORPUS is 1, the package that
 *  GUCEF_TEST_PACKINDEX_UNITYPACKAGE names is indexed (structure and analysis level), the time of the structure level is
 *  logged, a second run does no work, and the counts per group equal the golden file in the folder
 *  GUCEF_TEST_PACKINDEX_GOLDEN (made when absent)
 */
void PerformUnityPackIndexCorpusTests( void );

#endif /* GUCEF_ASSETPLUGIN_UNITY_TESTAPP_TESTUNITYPACKINDEXCORPUS_H ? */
