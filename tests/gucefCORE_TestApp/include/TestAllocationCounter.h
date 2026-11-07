/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H
#define GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace CORETEST {

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

/**
 *  Whether GetLiveHeapBytes() works in this build: with glibc (2.33 and later) or a sanitizer runtime which keeps the
 *  heap statistics (address and thread sanitizer)
 */
bool IsLiveHeapMeasurementAvailable( void );

/**
 *  The bytes of the heap allocations of the whole process (all threads) which are alive right now.
 *  Meant to compare before and after a workload, to see whether it leaves memory behind
 */
GUCEF::UInt64 GetLiveHeapBytes( void );

}; /* namespace CORETEST */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_TESTAPP_TESTALLOCATIONCOUNTER_H ? */
