/*
 *  assetpluginUNREAL_TestApp: Tests of the forms of the UE 4.25 to 4.27 and UE 5.0 to 5.2 packages and of the classification of packages
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALLEGACYFORMS_H
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALLEGACYFORMS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Synthetic packages of the forms the earlier engine releases write: the mesh description of UE 4.25 to 4.27 in legacy bulk data
 *  compressed with zlib in chunks, texture sources in the same form, the editor bulk data records of UE 5.0 to 5.2 (file versions
 *  1002 to 1008), and the classification of packages that hold nothing to convert (skipped, with the reason) against packages
 *  that fail.
 */
void PerformUnrealLegacyFormTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALLEGACYFORMS_H ? */
