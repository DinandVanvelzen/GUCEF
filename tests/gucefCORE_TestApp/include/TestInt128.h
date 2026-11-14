/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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

#ifndef GUCEF_CORE_TESTAPP_TESTINT128_H
#define GUCEF_CORE_TESTAPP_TESTINT128_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the 128 bit integers: the additive CUInt128 functions (128 by 128 division, 128 by 128 checked multiply,
 *  bitwise helpers, comparison operators) and the signed CInt128, checked against the independent base 10^9
 *  reference and the compiler's native 128 bit type where one exists. Sets a user break when a test fails.
 */
void
PerformInt128Tests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTINT128_H ? */
