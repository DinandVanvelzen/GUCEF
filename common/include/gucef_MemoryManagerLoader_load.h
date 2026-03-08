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

#ifndef GUCEF_MEMORYMANAGERLOADER_LOAD_H
#define GUCEF_MEMORYMANAGERLOADER_LOAD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MEMORYMANAGERLOADER_VARS_H
#include "gucef_MemoryManagerLoader_vars.h"
#define GUCEF_MEMORYMANAGERLOADER_VARS_H
#endif /* GUCEF_MEMORYMANAGERLOADER_VARS_H ? */

#ifndef GUCEF_SHAREDMODULE_H
#include "gucef_sharedmodule.h"
#define GUCEF_SHAREDMODULE_H
#endif /* GUCEF_SHAREDMODULE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) )

/*-------------------------------------------------------------------------*/

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )

/*-------------------------------------------------------------------------*/

static DRGUP_Int32 GUCEF_HIDDEN
DRGUP_UnloadMemoryManager( void )
{
    if ( 0 == g_memoryManagerModulePtr || 0 == g_dynLoadMutex )
        return 1;
    if ( WAIT_OBJECT_0 != WaitForSingleObject( (HANDLE) g_dynLoadMutex, 30000 ) )
        return 0;
    if ( 0 == g_memoryManagerModulePtr || 0 == g_dynLoadMutex )
        return 1;

    if ( NULL != fp_DRGUP_Shutdown )
    {
        DRGUP_Int32 callResult = fp_DRGUP_Shutdown();
        if ( 1 == callResult )
        {
            FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
            g_memoryManagerModulePtr = 0;
            ReleaseMutex( (HANDLE) g_dynLoadMutex );
            g_dynLoadMutex = 0;
            return 1;
        }
        return callResult;
    }
    else
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        ReleaseMutex( (HANDLE) g_dynLoadMutex );
        g_dynLoadMutex = 0;
        return 1;
    }
}

/*-------------------------------------------------------------------------*/

static int GUCEF_HIDDEN
DRGUP_LazyLoadMemoryManager( void )
{
    if ( 0 != g_memoryManagerModulePtr )
        return 1;
    if ( 0 == g_dynLoadMutex )
    {
        g_dynLoadMutex = CreateMutex( NULL, FALSE, NULL );
        if ( 0 == g_dynLoadMutex )
            return 0;
    }
    if ( WAIT_FAILED == WaitForSingleObject( (HANDLE) g_dynLoadMutex, 30000 ) )
        return 0;
    if ( 0 != g_memoryManagerModulePtr )
        return 1;

    g_memoryManagerModulePtr = (void*) GetModuleHandleA( MemoryLeakFinderLib );
    if ( 0 == g_memoryManagerModulePtr )
    {
        g_memoryManagerModulePtr = (void*) LoadLibrary( MemoryLeakFinderLib );
        if ( 0 == g_memoryManagerModulePtr )
        {
            ReleaseMutex( (HANDLE) g_dynLoadMutex );
            return 0;
        }
    }

    /* Resolve the base set of function pointers */
    fp_DRGUP_Initialize    = (TFP_DRGUP_Initialize)    GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_Initialize" );
    fp_DRGUP_Shutdown      = (TFP_DRGUP_Shutdown)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_Shutdown" );
    fp_DRGUP_DumpLogReport = (TFP_DRGUP_DumpLogReport) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_DumpLogReport" );
    fp_DRGUP_SetLogFile    = (TFP_DRGUP_SetLogFile)    GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SetLogFile" );
    fp_DRGUP_SetLogAlways  = (TFP_DRGUP_SetLogAlways)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SetLogAlways" );
    fp_DRGUP_CleanLogFile  = (TFP_DRGUP_CleanLogFile)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_CleanLogFile" );

    if ( 0 == fp_DRGUP_Initialize    ||
         0 == fp_DRGUP_Shutdown      ||
         0 == fp_DRGUP_DumpLogReport ||
         0 == fp_DRGUP_SetLogFile    ||
         0 == fp_DRGUP_SetLogAlways  ||
         0 == fp_DRGUP_CleanLogFile   )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

    fp_DRGUP_DumpMemoryAllocations      = (TFP_DRGUP_DumpMemoryAllocations)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_DumpMemoryAllocations" );
    fp_DRGUP_SetExhaustiveTesting       = (TFP_DRGUP_SetExhaustiveTesting)       GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SetExhaustiveTesting" );
    fp_DRGUP_SetPaddingSize             = (TFP_DRGUP_SetPaddingSize)             GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SetPaddingSize" );
    fp_DRGUP_BreakOnAllocation          = (TFP_DRGUP_BreakOnAllocation)          GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_BreakOnAllocation" );
    fp_DRGUP_BreakOnDeallocation        = (TFP_DRGUP_BreakOnDeallocation)        GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_BreakOnDeallocation" );
    fp_DRGUP_BreakOnReallocation        = (TFP_DRGUP_BreakOnReallocation)        GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_BreakOnReallocation" );
    fp_DRGUP_ValidateKnownAllocPtr      = (TFP_DRGUP_ValidateKnownAllocPtr)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ValidateKnownAllocPtr" );
    fp_DRGUP_ValidateKnownAllocBlock    = (TFP_DRGUP_ValidateKnownAllocBlock)    GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ValidateKnownAllocBlock" );
    fp_DRGUP_ValidateAccessibility      = (TFP_DRGUP_ValidateAccessibility)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ValidateAccessibility" );
    fp_DRGUP_ValidateChunk              = (TFP_DRGUP_ValidateChunk)              GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ValidateChunk" );
    fp_DRGUP_ValidatePendingDestructor  = (TFP_DRGUP_ValidatePendingDestructor)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ValidatePendingDestructor" );
    fp_DRGUP_ValidateFinishedDestructor = (TFP_DRGUP_ValidateFinishedDestructor) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ValidateFinishedDestructor" );
    fp_DRGUP_AllocateMemory             = (TFP_DRGUP_AllocateMemory)             GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_AllocateMemory" );
    fp_DRGUP_DeAllocateMemory           = (TFP_DRGUP_DeAllocateMemory)           GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_DeAllocateMemory" );
    fp_DRGUP_DeAllocateMemoryEx         = (TFP_DRGUP_DeAllocateMemoryEx)         GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_DeAllocateMemoryEx" );
    fp_DRGUP_SetOwner                   = (TFP_DRGUP_SetOwner)                   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SetOwner" );

    if ( 0 == fp_DRGUP_DumpMemoryAllocations      ||
         0 == fp_DRGUP_SetExhaustiveTesting        ||
         0 == fp_DRGUP_SetPaddingSize              ||
         0 == fp_DRGUP_BreakOnAllocation           ||
         0 == fp_DRGUP_BreakOnDeallocation         ||
         0 == fp_DRGUP_BreakOnReallocation         ||
         0 == fp_DRGUP_ValidateKnownAllocPtr       ||
         0 == fp_DRGUP_ValidateKnownAllocBlock     ||
         0 == fp_DRGUP_ValidateAccessibility       ||
         0 == fp_DRGUP_ValidateChunk               ||
         0 == fp_DRGUP_ValidatePendingDestructor   ||
         0 == fp_DRGUP_ValidateFinishedDestructor  ||
         0 == fp_DRGUP_AllocateMemory              ||
         0 == fp_DRGUP_DeAllocateMemory            ||
         0 == fp_DRGUP_SetOwner                     )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
    #if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

    fp_DRGUP_SysAllocString        = (TFP_DRGUP_SysAllocString)        GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SysAllocString" );
    fp_DRGUP_SysAllocStringByteLen = (TFP_DRGUP_SysAllocStringByteLen) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SysAllocStringByteLen" );
    fp_DRGUP_SysAllocStringLen     = (TFP_DRGUP_SysAllocStringLen)     GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SysAllocStringLen" );
    fp_DRGUP_SysFreeString         = (TFP_DRGUP_SysFreeString)         GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SysFreeString" );
    fp_DRGUP_SysReAllocString      = (TFP_DRGUP_SysReAllocString)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SysReAllocString" );
    fp_DRGUP_SysReAllocStringLen   = (TFP_DRGUP_SysReAllocStringLen)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_SysReAllocStringLen" );

    if ( 0 == fp_DRGUP_SysAllocString        ||
         0 == fp_DRGUP_SysAllocStringByteLen ||
         0 == fp_DRGUP_SysAllocStringLen     ||
         0 == fp_DRGUP_SysFreeString         ||
         0 == fp_DRGUP_SysReAllocString      ||
         0 == fp_DRGUP_SysReAllocStringLen    )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
    #if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

    fp_DRGUP_CallstackScopeBegin = (TFP_DRGUP_CallstackScopeBegin) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_CallstackScopeBegin" );
    fp_DRGUP_CallstackScopeEnd   = (TFP_DRGUP_CallstackScopeEnd)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_CallstackScopeEnd" );

    if ( 0 == fp_DRGUP_CallstackScopeBegin ||
         0 == fp_DRGUP_CallstackScopeEnd    )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
    #if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

    fp_DRGUP_ExclusiveLockCreated   = (TFP_DRGUP_ExclusiveLockCreated)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ExclusiveLockCreated" );
    fp_DRGUP_ExclusiveLockObtained  = (TFP_DRGUP_ExclusiveLockObtained)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ExclusiveLockObtained" );
    fp_DRGUP_ExclusiveLockReleased  = (TFP_DRGUP_ExclusiveLockReleased)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ExclusiveLockReleased" );
    fp_DRGUP_ExclusiveLockAbandoned = (TFP_DRGUP_ExclusiveLockAbandoned) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ExclusiveLockAbandoned" );
    fp_DRGUP_ExclusiveLockDestroy   = (TFP_DRGUP_ExclusiveLockDestroy)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "DRGUP_ExclusiveLockDestroy" );

    if ( 0 == fp_DRGUP_ExclusiveLockCreated   ||
         0 == fp_DRGUP_ExclusiveLockObtained  ||
         0 == fp_DRGUP_ExclusiveLockReleased  ||
         0 == fp_DRGUP_ExclusiveLockAbandoned ||
         0 == fp_DRGUP_ExclusiveLockDestroy    )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

    if ( 1 == fp_DRGUP_Initialize() )
        return 1;

    DRGUP_UnloadMemoryManager();
    return 0;
}

/*-------------------------------------------------------------------------*/

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

#include <dlfcn.h>

static const char* MemoryLeakFinderLibSo = "libMemoryLeakFinder_d.so";

static DRGUP_Int32 GUCEF_HIDDEN
DRGUP_UnloadMemoryManager( void )
{
    if ( NULL == g_memoryManagerModulePtr )
        return 1;

    if ( NULL != fp_DRGUP_Shutdown )
        fp_DRGUP_Shutdown();

    dlclose( g_memoryManagerModulePtr );
    g_memoryManagerModulePtr = NULL;
    return 1;
}

/*-------------------------------------------------------------------------*/

static int GUCEF_HIDDEN
DRGUP_LazyLoadMemoryManager( void )
{
    if ( NULL != g_memoryManagerModulePtr )
        return 1;

    /* Check if already loaded in the process (e.g. by another TU) */
    g_memoryManagerModulePtr = dlopen( MemoryLeakFinderLibSo, RTLD_NOLOAD | RTLD_NOW );
    if ( NULL == g_memoryManagerModulePtr )
        g_memoryManagerModulePtr = dlopen( MemoryLeakFinderLibSo, RTLD_NOW | RTLD_GLOBAL );
    if ( NULL == g_memoryManagerModulePtr )
        return 0;

    /* Resolve the base set of function pointers */
    fp_DRGUP_Initialize    = (TFP_DRGUP_Initialize)    dlsym( g_memoryManagerModulePtr, "DRGUP_Initialize" );
    fp_DRGUP_Shutdown      = (TFP_DRGUP_Shutdown)      dlsym( g_memoryManagerModulePtr, "DRGUP_Shutdown" );
    fp_DRGUP_DumpLogReport = (TFP_DRGUP_DumpLogReport) dlsym( g_memoryManagerModulePtr, "DRGUP_DumpLogReport" );
    fp_DRGUP_SetLogFile    = (TFP_DRGUP_SetLogFile)    dlsym( g_memoryManagerModulePtr, "DRGUP_SetLogFile" );
    fp_DRGUP_SetLogAlways  = (TFP_DRGUP_SetLogAlways)  dlsym( g_memoryManagerModulePtr, "DRGUP_SetLogAlways" );
    fp_DRGUP_CleanLogFile  = (TFP_DRGUP_CleanLogFile)  dlsym( g_memoryManagerModulePtr, "DRGUP_CleanLogFile" );

    if ( NULL == fp_DRGUP_Initialize    ||
         NULL == fp_DRGUP_Shutdown      ||
         NULL == fp_DRGUP_DumpLogReport ||
         NULL == fp_DRGUP_SetLogFile    ||
         NULL == fp_DRGUP_SetLogAlways  ||
         NULL == fp_DRGUP_CleanLogFile   )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

    fp_DRGUP_DumpMemoryAllocations      = (TFP_DRGUP_DumpMemoryAllocations)      dlsym( g_memoryManagerModulePtr, "DRGUP_DumpMemoryAllocations" );
    fp_DRGUP_SetExhaustiveTesting       = (TFP_DRGUP_SetExhaustiveTesting)       dlsym( g_memoryManagerModulePtr, "DRGUP_SetExhaustiveTesting" );
    fp_DRGUP_SetPaddingSize             = (TFP_DRGUP_SetPaddingSize)             dlsym( g_memoryManagerModulePtr, "DRGUP_SetPaddingSize" );
    fp_DRGUP_BreakOnAllocation          = (TFP_DRGUP_BreakOnAllocation)          dlsym( g_memoryManagerModulePtr, "DRGUP_BreakOnAllocation" );
    fp_DRGUP_BreakOnDeallocation        = (TFP_DRGUP_BreakOnDeallocation)        dlsym( g_memoryManagerModulePtr, "DRGUP_BreakOnDeallocation" );
    fp_DRGUP_BreakOnReallocation        = (TFP_DRGUP_BreakOnReallocation)        dlsym( g_memoryManagerModulePtr, "DRGUP_BreakOnReallocation" );
    fp_DRGUP_ValidateKnownAllocPtr      = (TFP_DRGUP_ValidateKnownAllocPtr)      dlsym( g_memoryManagerModulePtr, "DRGUP_ValidateKnownAllocPtr" );
    fp_DRGUP_ValidateKnownAllocBlock    = (TFP_DRGUP_ValidateKnownAllocBlock)    dlsym( g_memoryManagerModulePtr, "DRGUP_ValidateKnownAllocBlock" );
    fp_DRGUP_ValidateAccessibility      = (TFP_DRGUP_ValidateAccessibility)      dlsym( g_memoryManagerModulePtr, "DRGUP_ValidateAccessibility" );
    fp_DRGUP_ValidateChunk              = (TFP_DRGUP_ValidateChunk)              dlsym( g_memoryManagerModulePtr, "DRGUP_ValidateChunk" );
    fp_DRGUP_ValidatePendingDestructor  = (TFP_DRGUP_ValidatePendingDestructor)  dlsym( g_memoryManagerModulePtr, "DRGUP_ValidatePendingDestructor" );
    fp_DRGUP_ValidateFinishedDestructor = (TFP_DRGUP_ValidateFinishedDestructor) dlsym( g_memoryManagerModulePtr, "DRGUP_ValidateFinishedDestructor" );
    fp_DRGUP_AllocateMemory             = (TFP_DRGUP_AllocateMemory)             dlsym( g_memoryManagerModulePtr, "DRGUP_AllocateMemory" );
    fp_DRGUP_DeAllocateMemory           = (TFP_DRGUP_DeAllocateMemory)           dlsym( g_memoryManagerModulePtr, "DRGUP_DeAllocateMemory" );
    fp_DRGUP_DeAllocateMemoryEx         = (TFP_DRGUP_DeAllocateMemoryEx)         dlsym( g_memoryManagerModulePtr, "DRGUP_DeAllocateMemoryEx" );
    fp_DRGUP_SetOwner                   = (TFP_DRGUP_SetOwner)                   dlsym( g_memoryManagerModulePtr, "DRGUP_SetOwner" );

    if ( NULL == fp_DRGUP_DumpMemoryAllocations      ||
         NULL == fp_DRGUP_SetExhaustiveTesting        ||
         NULL == fp_DRGUP_SetPaddingSize              ||
         NULL == fp_DRGUP_BreakOnAllocation           ||
         NULL == fp_DRGUP_BreakOnDeallocation         ||
         NULL == fp_DRGUP_BreakOnReallocation         ||
         NULL == fp_DRGUP_ValidateKnownAllocPtr       ||
         NULL == fp_DRGUP_ValidateKnownAllocBlock     ||
         NULL == fp_DRGUP_ValidateAccessibility       ||
         NULL == fp_DRGUP_ValidateChunk               ||
         NULL == fp_DRGUP_ValidatePendingDestructor   ||
         NULL == fp_DRGUP_ValidateFinishedDestructor  ||
         NULL == fp_DRGUP_AllocateMemory              ||
         NULL == fp_DRGUP_DeAllocateMemory            ||
         NULL == fp_DRGUP_SetOwner                     )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
    #if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

    fp_DRGUP_CallstackScopeBegin = (TFP_DRGUP_CallstackScopeBegin) dlsym( g_memoryManagerModulePtr, "DRGUP_CallstackScopeBegin" );
    fp_DRGUP_CallstackScopeEnd   = (TFP_DRGUP_CallstackScopeEnd)   dlsym( g_memoryManagerModulePtr, "DRGUP_CallstackScopeEnd" );

    if ( NULL == fp_DRGUP_CallstackScopeBegin ||
         NULL == fp_DRGUP_CallstackScopeEnd    )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
    #if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

    fp_DRGUP_ExclusiveLockCreated   = (TFP_DRGUP_ExclusiveLockCreated)   dlsym( g_memoryManagerModulePtr, "DRGUP_ExclusiveLockCreated" );
    fp_DRGUP_ExclusiveLockObtained  = (TFP_DRGUP_ExclusiveLockObtained)  dlsym( g_memoryManagerModulePtr, "DRGUP_ExclusiveLockObtained" );
    fp_DRGUP_ExclusiveLockReleased  = (TFP_DRGUP_ExclusiveLockReleased)  dlsym( g_memoryManagerModulePtr, "DRGUP_ExclusiveLockReleased" );
    fp_DRGUP_ExclusiveLockAbandoned = (TFP_DRGUP_ExclusiveLockAbandoned) dlsym( g_memoryManagerModulePtr, "DRGUP_ExclusiveLockAbandoned" );
    fp_DRGUP_ExclusiveLockDestroy   = (TFP_DRGUP_ExclusiveLockDestroy)   dlsym( g_memoryManagerModulePtr, "DRGUP_ExclusiveLockDestroy" );

    if ( NULL == fp_DRGUP_ExclusiveLockCreated   ||
         NULL == fp_DRGUP_ExclusiveLockObtained  ||
         NULL == fp_DRGUP_ExclusiveLockReleased  ||
         NULL == fp_DRGUP_ExclusiveLockAbandoned ||
         NULL == fp_DRGUP_ExclusiveLockDestroy    )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

    if ( 1 == fp_DRGUP_Initialize() )
        return 1;

    DRGUP_UnloadMemoryManager();
    return 0;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM */

/*-------------------------------------------------------------------------*/

inline
DRGUP_Int32
DRGUP_Initialize( void )
{
    return ( 0 == DRGUP_LazyLoadMemoryManager() ? 0 : fp_DRGUP_Initialize() );
}

/*-------------------------------------------------------------------------*/

inline
DRGUP_Int32
DRGUP_Shutdown( void )
{
    return DRGUP_UnloadMemoryManager();
}

/*-------------------------------------------------------------------------*/

inline
void
DRGUP_DumpLogReport( void )
{
    if ( NULL != fp_DRGUP_DumpLogReport )
        fp_DRGUP_DumpLogReport();
}

/*-------------------------------------------------------------------------*/

inline
void
DRGUP_SetLogFile( const char* file )
{
    if ( 0 != DRGUP_LazyLoadMemoryManager() )
        fp_DRGUP_SetLogFile( file );
}

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_LOAD_H ? */
