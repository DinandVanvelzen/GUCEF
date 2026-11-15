/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTGEOMETRYALLOCATIONCOUNTER_H
#define GUCEF_GEOMETRY_TESTAPP_TESTGEOMETRYALLOCATIONCOUNTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF_MACROS_H
#endif /* GUCEF_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Counts the heap allocations (malloc, calloc, realloc and aligned variants, which includes every operator new) made by
 *  the calling thread while armed. It replaces the allocation functions of the C library for the whole test application
 *  (the replacement mechanism that glibc documents) and forwards to the C library, so it coexists with other
 *  replacements of operator new in the test application. Specific to gucefGEOMETRY, so it lives with its test application.
 *
 *  Not available (IsAvailable() is false, the counts stay 0) with a sanitizer that replaces the allocator itself
 *  (AddressSanitizer, ThreadSanitizer, MemorySanitizer, RealtimeSanitizer) and on platforms without glibc: callers skip
 *  their test in that case; the RealtimeSanitizer build proves the same property by reporting any allocation made in
 *  a realtime context.
 */
namespace GeometryAllocationCounter {

/** True when the counter is compiled in */
bool IsAvailable( void );

/** Arms the counter for the calling thread and resets the count */
void StartCounting( void );

/** Disarms the counter and returns the number of allocations made since StartCounting */
GUCEF::UInt32 StopCounting( void );

/** Allocates 16 bytes through malloc, as seen by the counter (not elidable by the compiler); used to prove that the counter counts */
void AllocateProbe( void );

}; /* namespace GeometryAllocationCounter */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTGEOMETRYALLOCATIONCOUNTER_H ? */
