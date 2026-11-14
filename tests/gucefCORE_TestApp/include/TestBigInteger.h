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

#ifndef GUCEF_CORE_TESTAPP_TESTBIGINTEGER_H
#define GUCEF_CORE_TESTAPP_TESTBIGINTEGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tests the arbitrary precision CBigInteger: representation and storage, arithmetic, division (Knuth algorithm D),
 *  number theory, two's complement bit operations, text in every base, float, decimal and byte conversions.
 *  Results are checked against the independent base 10^9 reference of TestDecimalReference.h, identities and
 *  published constants. Opt-in benchmarks run when GUCEF_CORE_TESTAPP_BIGINTEGER_BENCHMARK is set.
 */
void
PerformBigIntegerTests( void );

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTBIGINTEGER_H ? */
