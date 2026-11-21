/*
 *  assetpluginUNREAL_TestApp: Tests of the tagged property reader of the Unreal reader
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALPROPERTIES_H
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALPROPERTIES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_GUCEFCORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_GUCEFCORE_H
#endif /* GUCEF_CORE_GUCEFCORE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void PerformUnrealPropertyTests( void );

/** A package of the versions with properties of every kind in two exports, soft paths and a payload in the trailer (UE5 versions from 1002) */
void BuildRichPackage( GUCEF::Int32 legacy, GUCEF::Int32 ue4, GUCEF::Int32 ue5, GUCEF::vector< GUCEF::UInt8 >& bytes );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALPROPERTIES_H ? */
