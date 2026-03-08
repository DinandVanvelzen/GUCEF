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

static TFP_DRGUP_Initialize    fp_DRGUP_Initialize    = 0;
static TFP_DRGUP_Shutdown      fp_DRGUP_Shutdown      = 0;
static TFP_DRGUP_DumpLogReport fp_DRGUP_DumpLogReport = 0;
static TFP_DRGUP_SetLogFile    fp_DRGUP_SetLogFile    = 0;
static TFP_DRGUP_SetLogAlways  fp_DRGUP_SetLogAlways  = 0;
static TFP_DRGUP_CleanLogFile  fp_DRGUP_CleanLogFile  = 0;

/*-------------------------------------------------------------------------*/

static void*       g_memoryManagerModulePtr = NULL;
static void*       g_dynLoadMutex           = NULL;
static const char* MemoryLeakFinderLib      = "MemoryLeakFinder_d.dll";

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

static TFP_DRGUP_DumpMemoryAllocations      fp_DRGUP_DumpMemoryAllocations      = 0;
static TFP_DRGUP_SetExhaustiveTesting       fp_DRGUP_SetExhaustiveTesting       = 0;
static TFP_DRGUP_SetPaddingSize             fp_DRGUP_SetPaddingSize             = 0;
static TFP_DRGUP_BreakOnAllocation          fp_DRGUP_BreakOnAllocation          = 0;
static TFP_DRGUP_BreakOnDeallocation        fp_DRGUP_BreakOnDeallocation        = 0;
static TFP_DRGUP_BreakOnReallocation        fp_DRGUP_BreakOnReallocation        = 0;
static TFP_DRGUP_ValidateKnownAllocPtr      fp_DRGUP_ValidateKnownAllocPtr      = 0;
static TFP_DRGUP_ValidateKnownAllocBlock    fp_DRGUP_ValidateKnownAllocBlock    = 0;
static TFP_DRGUP_ValidateAccessibility      fp_DRGUP_ValidateAccessibility      = 0;
static TFP_DRGUP_ValidateChunk              fp_DRGUP_ValidateChunk              = 0;
static TFP_DRGUP_ValidatePendingDestructor  fp_DRGUP_ValidatePendingDestructor  = 0;
static TFP_DRGUP_ValidateFinishedDestructor fp_DRGUP_ValidateFinishedDestructor = 0;
static TFP_DRGUP_AllocateMemory             fp_DRGUP_AllocateMemory             = 0;
static TFP_DRGUP_DeAllocateMemory           fp_DRGUP_DeAllocateMemory           = 0;
static TFP_DRGUP_DeAllocateMemoryEx         fp_DRGUP_DeAllocateMemoryEx         = 0;
static TFP_DRGUP_SetOwner                   fp_DRGUP_SetOwner                   = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

/*-------------------------------------------------------------------------*/

static TFP_DRGUP_SysAllocString         fp_DRGUP_SysAllocString         = 0;
static TFP_DRGUP_SysAllocStringByteLen  fp_DRGUP_SysAllocStringByteLen  = 0;
static TFP_DRGUP_SysAllocStringLen      fp_DRGUP_SysAllocStringLen      = 0;
static TFP_DRGUP_SysFreeString          fp_DRGUP_SysFreeString          = 0;
static TFP_DRGUP_SysReAllocString       fp_DRGUP_SysReAllocString       = 0;
static TFP_DRGUP_SysReAllocStringLen    fp_DRGUP_SysReAllocStringLen    = 0;

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

/*-------------------------------------------------------------------------*/

static TFP_DRGUP_CallstackScopeBegin  fp_DRGUP_CallstackScopeBegin  = 0;
static TFP_DRGUP_CallstackScopeEnd    fp_DRGUP_CallstackScopeEnd    = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

static TFP_DRGUP_ExclusiveLockCreated    fp_DRGUP_ExclusiveLockCreated    = 0;
static TFP_DRGUP_ExclusiveLockObtained   fp_DRGUP_ExclusiveLockObtained   = 0;
static TFP_DRGUP_ExclusiveLockReleased   fp_DRGUP_ExclusiveLockReleased   = 0;
static TFP_DRGUP_ExclusiveLockAbandoned  fp_DRGUP_ExclusiveLockAbandoned  = 0;
static TFP_DRGUP_ExclusiveLockDestroy    fp_DRGUP_ExclusiveLockDestroy    = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_VARS_H ? */
