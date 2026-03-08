/*
 *  gucef common header: provides header based platform wide facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MEMORYMANAGERLOADER_VARS_H
#define GUCEF_MEMORYMANAGERLOADER_VARS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MEMORYMANAGERLOADER_TYPES_H
#include "gucef_MemoryManagerLoader_types.h"
#define GUCEF_MEMORYMANAGERLOADER_TYPES_H
#endif /* GUCEF_MEMORYMANAGERLOADER_TYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) )

static TFP_MEMMAN_Initialize    fp_MEMMAN_Initialize    = 0;
static TFP_MEMMAN_Shutdown      fp_MEMMAN_Shutdown      = 0;
static TFP_MEMMAN_DumpLogReport fp_MEMMAN_DumpLogReport = 0;
static TFP_MEMMAN_SetLogFile    fp_MEMMAN_SetLogFile    = 0;
static TFP_MEMMAN_SetLogAlways  fp_MEMMAN_SetLogAlways  = 0;
static TFP_MEMMAN_CleanLogFile  fp_MEMMAN_CleanLogFile  = 0;

/*-------------------------------------------------------------------------*/

static void*       g_memoryManagerModulePtr = NULL;
static void*       g_dynLoadMutex           = NULL;
static const char* MemoryLeakFinderLib      = "MemoryLeakFinder_d.dll";

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_DumpMemoryAllocations      fp_MEMMAN_DumpMemoryAllocations      = 0;
static TFP_MEMMAN_SetExhaustiveTesting       fp_MEMMAN_SetExhaustiveTesting       = 0;
static TFP_MEMMAN_SetPaddingSize             fp_MEMMAN_SetPaddingSize             = 0;
static TFP_MEMMAN_BreakOnAllocation          fp_MEMMAN_BreakOnAllocation          = 0;
static TFP_MEMMAN_BreakOnDeallocation        fp_MEMMAN_BreakOnDeallocation        = 0;
static TFP_MEMMAN_BreakOnReallocation        fp_MEMMAN_BreakOnReallocation        = 0;
static TFP_MEMMAN_ValidateKnownAllocPtr      fp_MEMMAN_ValidateKnownAllocPtr      = 0;
static TFP_MEMMAN_ValidateKnownAllocBlock    fp_MEMMAN_ValidateKnownAllocBlock    = 0;
static TFP_MEMMAN_ValidateAccessibility      fp_MEMMAN_ValidateAccessibility      = 0;
static TFP_MEMMAN_ValidateChunk              fp_MEMMAN_ValidateChunk              = 0;
static TFP_MEMMAN_ValidatePendingDestructor  fp_MEMMAN_ValidatePendingDestructor  = 0;
static TFP_MEMMAN_ValidateFinishedDestructor fp_MEMMAN_ValidateFinishedDestructor = 0;
static TFP_MEMMAN_AllocateMemory             fp_MEMMAN_AllocateMemory             = 0;
static TFP_MEMMAN_DeAllocateMemory           fp_MEMMAN_DeAllocateMemory           = 0;
static TFP_MEMMAN_DeAllocateMemoryEx         fp_MEMMAN_DeAllocateMemoryEx         = 0;
static TFP_MEMMAN_SetOwner                   fp_MEMMAN_SetOwner                   = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_SysAllocString         fp_MEMMAN_SysAllocString         = 0;
static TFP_MEMMAN_SysAllocStringByteLen  fp_MEMMAN_SysAllocStringByteLen  = 0;
static TFP_MEMMAN_SysAllocStringLen      fp_MEMMAN_SysAllocStringLen      = 0;
static TFP_MEMMAN_SysFreeString          fp_MEMMAN_SysFreeString          = 0;
static TFP_MEMMAN_SysReAllocString       fp_MEMMAN_SysReAllocString       = 0;
static TFP_MEMMAN_SysReAllocStringLen    fp_MEMMAN_SysReAllocStringLen    = 0;

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_CallstackScopeBegin  fp_MEMMAN_CallstackScopeBegin  = 0;
static TFP_MEMMAN_CallstackScopeEnd    fp_MEMMAN_CallstackScopeEnd    = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_ExclusiveLockCreated    fp_MEMMAN_ExclusiveLockCreated    = 0;
static TFP_MEMMAN_ExclusiveLockObtained   fp_MEMMAN_ExclusiveLockObtained   = 0;
static TFP_MEMMAN_ExclusiveLockReleased   fp_MEMMAN_ExclusiveLockReleased   = 0;
static TFP_MEMMAN_ExclusiveLockAbandoned  fp_MEMMAN_ExclusiveLockAbandoned  = 0;
static TFP_MEMMAN_ExclusiveLockDestroy    fp_MEMMAN_ExclusiveLockDestroy    = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_VARS_H ? */
