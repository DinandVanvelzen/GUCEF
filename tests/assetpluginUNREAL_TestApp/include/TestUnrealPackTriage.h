/*
 *  assetpluginUNREAL_TestApp: the triage of real packs: every package of the pack folders through the importer, results by failure class
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALPACKTRIAGE_H
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALPACKTRIAGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Runs the importer over every package of the pack folders named by GUCEF_TEST_UNREAL_DIR (folders separated by ';',
 *  read only) and tells per package its classes, its file versions and the result, and per pack the counts by status and
 *  by failure class. Skips without the variable. Options:
 *    GUCEF_TEST_UNREAL_TRIAGE_OUT     a file that receives the table (one line per package, then the totals)
 *    GUCEF_TEST_UNREAL_TRIAGE_ONLY    only the packages whose path holds this text
 *    GUCEF_TEST_UNREAL_IMAGE_PLUGIN   an image codec plugin (path without extension): the pictures the importer writes are
 *                                     decoded the way the texture baker of the pipeline decodes them
 *  The intermediate files go to the work folder of the tests and are removed after each package.
 */
void PerformUnrealPackTriage( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALPACKTRIAGE_H ? */
