/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#ifndef GUCEF_DRN_TESTAPP_TESTDRNALLOCATIONCOUNTER_H
#define GUCEF_DRN_TESTAPP_TESTDRNALLOCATIONCOUNTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace DRNTEST {

/**
 *  Counts the heap allocations (malloc, calloc, realloc and so operator new) made by the calling thread between
 *  Start and Stop. Available where the C library allows it to be interposed (glibc).
 */
bool IsAllocationCountingAvailable( void );

/** Why allocations are not counted in this build, empty when they are */
const char* GetAllocationCountingUnavailableReason( void );

void StartAllocationCounting( void );

GUCEF::UInt32 StopAllocationCounting( void );

/** Where the first counted allocations came from, one call stack per line (empty where that is not available) */
GUCEF::CORE::CString GetCountedAllocationStacks( void );

}; /* namespace DRNTEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_TESTAPP_TESTDRNALLOCATIONCOUNTER_H ? */
