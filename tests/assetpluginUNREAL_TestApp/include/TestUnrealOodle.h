/*
 *  assetpluginUNREAL_TestApp: Tests of the Oodle provider, the compressed buffer reader and the capability list
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

#ifndef GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALOODLE_H
#define GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALOODLE_H

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void PerformUnrealOodleTests( void );

/** The path of the oodle tool for the tests: the environment variable, next to the test executable, else the build folder of the tool; empty when none exists */
GUCEF::CORE::CString FindOodleToolForTests( void );

/**
 *  Builds a compressed buffer of method 3 whose blocks are "stored" streams of the tool (two bytes 0xCC 0x06 and the raw bytes), the
 *  form the tests of the tool use; the block size is 1 << blockLog2. The header checksum is right.
 */
void BuildOodleStoredBuffer( const GUCEF::vector< GUCEF::UInt8 >& raw, GUCEF::UInt32 blockLog2, GUCEF::vector< GUCEF::UInt8 >& out );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETPLUGIN_UNREAL_TESTAPP_TESTUNREALOODLE_H ? */
