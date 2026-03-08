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

static MEMMAN_Int32 GUCEF_HIDDEN
MEMMAN_UnloadMemoryManager( void )
{
    if ( 0 == g_memoryManagerModulePtr || 0 == g_dynLoadMutex )
        return 1;
    if ( WAIT_OBJECT_0 != WaitForSingleObject( (HANDLE) g_dynLoadMutex, 30000 ) )
        return 0;
    if ( 0 == g_memoryManagerModulePtr || 0 == g_dynLoadMutex )
        return 1;

    if ( NULL != fp_MEMMAN_Shutdown )
    {
        MEMMAN_Int32 callResult = fp_MEMMAN_Shutdown();
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
MEMMAN_LazyLoadMemoryManager( void )
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
    fp_MEMMAN_Initialize    = (TFP_MEMMAN_Initialize)    GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_Initialize" );
    fp_MEMMAN_Shutdown      = (TFP_MEMMAN_Shutdown)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_Shutdown" );
    fp_MEMMAN_DumpLogReport = (TFP_MEMMAN_DumpLogReport) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DumpLogReport" );
    fp_MEMMAN_SetLogFile    = (TFP_MEMMAN_SetLogFile)    GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetLogFile" );
    fp_MEMMAN_SetLogAlways  = (TFP_MEMMAN_SetLogAlways)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetLogAlways" );
    fp_MEMMAN_CleanLogFile  = (TFP_MEMMAN_CleanLogFile)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_CleanLogFile" );

    if ( 0 == fp_MEMMAN_Initialize    ||
         0 == fp_MEMMAN_Shutdown      ||
         0 == fp_MEMMAN_DumpLogReport ||
         0 == fp_MEMMAN_SetLogFile    ||
         0 == fp_MEMMAN_SetLogAlways  ||
         0 == fp_MEMMAN_CleanLogFile   )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

    fp_MEMMAN_DumpMemoryAllocations      = (TFP_MEMMAN_DumpMemoryAllocations)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DumpMemoryAllocations" );
    fp_MEMMAN_SetExhaustiveTesting       = (TFP_MEMMAN_SetExhaustiveTesting)       GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetExhaustiveTesting" );
    fp_MEMMAN_SetPaddingSize             = (TFP_MEMMAN_SetPaddingSize)             GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetPaddingSize" );
    fp_MEMMAN_BreakOnAllocation          = (TFP_MEMMAN_BreakOnAllocation)          GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_BreakOnAllocation" );
    fp_MEMMAN_BreakOnDeallocation        = (TFP_MEMMAN_BreakOnDeallocation)        GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_BreakOnDeallocation" );
    fp_MEMMAN_BreakOnReallocation        = (TFP_MEMMAN_BreakOnReallocation)        GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_BreakOnReallocation" );
    fp_MEMMAN_ValidateKnownAllocPtr      = (TFP_MEMMAN_ValidateKnownAllocPtr)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateKnownAllocPtr" );
    fp_MEMMAN_ValidateKnownAllocBlock    = (TFP_MEMMAN_ValidateKnownAllocBlock)    GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateKnownAllocBlock" );
    fp_MEMMAN_ValidateAccessibility      = (TFP_MEMMAN_ValidateAccessibility)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateAccessibility" );
    fp_MEMMAN_ValidateChunk              = (TFP_MEMMAN_ValidateChunk)              GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateChunk" );
    fp_MEMMAN_ValidatePendingDestructor  = (TFP_MEMMAN_ValidatePendingDestructor)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidatePendingDestructor" );
    fp_MEMMAN_ValidateFinishedDestructor = (TFP_MEMMAN_ValidateFinishedDestructor) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateFinishedDestructor" );
    fp_MEMMAN_AllocateMemory             = (TFP_MEMMAN_AllocateMemory)             GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_AllocateMemory" );
    fp_MEMMAN_DeAllocateMemory           = (TFP_MEMMAN_DeAllocateMemory)           GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DeAllocateMemory" );
    fp_MEMMAN_DeAllocateMemoryEx         = (TFP_MEMMAN_DeAllocateMemoryEx)         GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DeAllocateMemoryEx" );
    fp_MEMMAN_SetOwner                   = (TFP_MEMMAN_SetOwner)                   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetOwner" );

    if ( 0 == fp_MEMMAN_DumpMemoryAllocations      ||
         0 == fp_MEMMAN_SetExhaustiveTesting        ||
         0 == fp_MEMMAN_SetPaddingSize              ||
         0 == fp_MEMMAN_BreakOnAllocation           ||
         0 == fp_MEMMAN_BreakOnDeallocation         ||
         0 == fp_MEMMAN_BreakOnReallocation         ||
         0 == fp_MEMMAN_ValidateKnownAllocPtr       ||
         0 == fp_MEMMAN_ValidateKnownAllocBlock     ||
         0 == fp_MEMMAN_ValidateAccessibility       ||
         0 == fp_MEMMAN_ValidateChunk               ||
         0 == fp_MEMMAN_ValidatePendingDestructor   ||
         0 == fp_MEMMAN_ValidateFinishedDestructor  ||
         0 == fp_MEMMAN_AllocateMemory              ||
         0 == fp_MEMMAN_DeAllocateMemory            ||
         0 == fp_MEMMAN_SetOwner                     )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
    #if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

    fp_MEMMAN_SysAllocString        = (TFP_MEMMAN_SysAllocString)        GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysAllocString" );
    fp_MEMMAN_SysAllocStringByteLen = (TFP_MEMMAN_SysAllocStringByteLen) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysAllocStringByteLen" );
    fp_MEMMAN_SysAllocStringLen     = (TFP_MEMMAN_SysAllocStringLen)     GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysAllocStringLen" );
    fp_MEMMAN_SysFreeString         = (TFP_MEMMAN_SysFreeString)         GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysFreeString" );
    fp_MEMMAN_SysReAllocString      = (TFP_MEMMAN_SysReAllocString)      GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysReAllocString" );
    fp_MEMMAN_SysReAllocStringLen   = (TFP_MEMMAN_SysReAllocStringLen)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysReAllocStringLen" );

    if ( 0 == fp_MEMMAN_SysAllocString        ||
         0 == fp_MEMMAN_SysAllocStringByteLen ||
         0 == fp_MEMMAN_SysAllocStringLen     ||
         0 == fp_MEMMAN_SysFreeString         ||
         0 == fp_MEMMAN_SysReAllocString      ||
         0 == fp_MEMMAN_SysReAllocStringLen    )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
    #if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

    fp_MEMMAN_CallstackScopeBegin = (TFP_MEMMAN_CallstackScopeBegin) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_CallstackScopeBegin" );
    fp_MEMMAN_CallstackScopeEnd   = (TFP_MEMMAN_CallstackScopeEnd)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_CallstackScopeEnd" );

    if ( 0 == fp_MEMMAN_CallstackScopeBegin ||
         0 == fp_MEMMAN_CallstackScopeEnd    )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
    #if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

    fp_MEMMAN_ExclusiveLockCreated   = (TFP_MEMMAN_ExclusiveLockCreated)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockCreated" );
    fp_MEMMAN_ExclusiveLockObtained  = (TFP_MEMMAN_ExclusiveLockObtained)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockObtained" );
    fp_MEMMAN_ExclusiveLockReleased  = (TFP_MEMMAN_ExclusiveLockReleased)  GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockReleased" );
    fp_MEMMAN_ExclusiveLockAbandoned = (TFP_MEMMAN_ExclusiveLockAbandoned) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockAbandoned" );
    fp_MEMMAN_ExclusiveLockDestroy   = (TFP_MEMMAN_ExclusiveLockDestroy)   GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockDestroy" );

    if ( 0 == fp_MEMMAN_ExclusiveLockCreated   ||
         0 == fp_MEMMAN_ExclusiveLockObtained  ||
         0 == fp_MEMMAN_ExclusiveLockReleased  ||
         0 == fp_MEMMAN_ExclusiveLockAbandoned ||
         0 == fp_MEMMAN_ExclusiveLockDestroy    )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

    if ( 1 == fp_MEMMAN_Initialize() )
        return 1;

    MEMMAN_UnloadMemoryManager();
    return 0;
}

/*-------------------------------------------------------------------------*/

#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID )

#include <dlfcn.h>

static const char* MemoryLeakFinderLibSo = "libMemoryLeakFinder_d.so";

static MEMMAN_Int32 GUCEF_HIDDEN
MEMMAN_UnloadMemoryManager( void )
{
    if ( NULL == g_memoryManagerModulePtr )
        return 1;

    if ( NULL != fp_MEMMAN_Shutdown )
        fp_MEMMAN_Shutdown();

    dlclose( g_memoryManagerModulePtr );
    g_memoryManagerModulePtr = NULL;
    return 1;
}

/*-------------------------------------------------------------------------*/

static int GUCEF_HIDDEN
MEMMAN_LazyLoadMemoryManager( void )
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
    fp_MEMMAN_Initialize    = (TFP_MEMMAN_Initialize)    dlsym( g_memoryManagerModulePtr, "MEMMAN_Initialize" );
    fp_MEMMAN_Shutdown      = (TFP_MEMMAN_Shutdown)      dlsym( g_memoryManagerModulePtr, "MEMMAN_Shutdown" );
    fp_MEMMAN_DumpLogReport = (TFP_MEMMAN_DumpLogReport) dlsym( g_memoryManagerModulePtr, "MEMMAN_DumpLogReport" );
    fp_MEMMAN_SetLogFile    = (TFP_MEMMAN_SetLogFile)    dlsym( g_memoryManagerModulePtr, "MEMMAN_SetLogFile" );
    fp_MEMMAN_SetLogAlways  = (TFP_MEMMAN_SetLogAlways)  dlsym( g_memoryManagerModulePtr, "MEMMAN_SetLogAlways" );
    fp_MEMMAN_CleanLogFile  = (TFP_MEMMAN_CleanLogFile)  dlsym( g_memoryManagerModulePtr, "MEMMAN_CleanLogFile" );

    if ( NULL == fp_MEMMAN_Initialize    ||
         NULL == fp_MEMMAN_Shutdown      ||
         NULL == fp_MEMMAN_DumpLogReport ||
         NULL == fp_MEMMAN_SetLogFile    ||
         NULL == fp_MEMMAN_SetLogAlways  ||
         NULL == fp_MEMMAN_CleanLogFile   )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

    fp_MEMMAN_DumpMemoryAllocations      = (TFP_MEMMAN_DumpMemoryAllocations)      dlsym( g_memoryManagerModulePtr, "MEMMAN_DumpMemoryAllocations" );
    fp_MEMMAN_SetExhaustiveTesting       = (TFP_MEMMAN_SetExhaustiveTesting)       dlsym( g_memoryManagerModulePtr, "MEMMAN_SetExhaustiveTesting" );
    fp_MEMMAN_SetPaddingSize             = (TFP_MEMMAN_SetPaddingSize)             dlsym( g_memoryManagerModulePtr, "MEMMAN_SetPaddingSize" );
    fp_MEMMAN_BreakOnAllocation          = (TFP_MEMMAN_BreakOnAllocation)          dlsym( g_memoryManagerModulePtr, "MEMMAN_BreakOnAllocation" );
    fp_MEMMAN_BreakOnDeallocation        = (TFP_MEMMAN_BreakOnDeallocation)        dlsym( g_memoryManagerModulePtr, "MEMMAN_BreakOnDeallocation" );
    fp_MEMMAN_BreakOnReallocation        = (TFP_MEMMAN_BreakOnReallocation)        dlsym( g_memoryManagerModulePtr, "MEMMAN_BreakOnReallocation" );
    fp_MEMMAN_ValidateKnownAllocPtr      = (TFP_MEMMAN_ValidateKnownAllocPtr)      dlsym( g_memoryManagerModulePtr, "MEMMAN_ValidateKnownAllocPtr" );
    fp_MEMMAN_ValidateKnownAllocBlock    = (TFP_MEMMAN_ValidateKnownAllocBlock)    dlsym( g_memoryManagerModulePtr, "MEMMAN_ValidateKnownAllocBlock" );
    fp_MEMMAN_ValidateAccessibility      = (TFP_MEMMAN_ValidateAccessibility)      dlsym( g_memoryManagerModulePtr, "MEMMAN_ValidateAccessibility" );
    fp_MEMMAN_ValidateChunk              = (TFP_MEMMAN_ValidateChunk)              dlsym( g_memoryManagerModulePtr, "MEMMAN_ValidateChunk" );
    fp_MEMMAN_ValidatePendingDestructor  = (TFP_MEMMAN_ValidatePendingDestructor)  dlsym( g_memoryManagerModulePtr, "MEMMAN_ValidatePendingDestructor" );
    fp_MEMMAN_ValidateFinishedDestructor = (TFP_MEMMAN_ValidateFinishedDestructor) dlsym( g_memoryManagerModulePtr, "MEMMAN_ValidateFinishedDestructor" );
    fp_MEMMAN_AllocateMemory             = (TFP_MEMMAN_AllocateMemory)             dlsym( g_memoryManagerModulePtr, "MEMMAN_AllocateMemory" );
    fp_MEMMAN_DeAllocateMemory           = (TFP_MEMMAN_DeAllocateMemory)           dlsym( g_memoryManagerModulePtr, "MEMMAN_DeAllocateMemory" );
    fp_MEMMAN_DeAllocateMemoryEx         = (TFP_MEMMAN_DeAllocateMemoryEx)         dlsym( g_memoryManagerModulePtr, "MEMMAN_DeAllocateMemoryEx" );
    fp_MEMMAN_SetOwner                   = (TFP_MEMMAN_SetOwner)                   dlsym( g_memoryManagerModulePtr, "MEMMAN_SetOwner" );

    if ( NULL == fp_MEMMAN_DumpMemoryAllocations      ||
         NULL == fp_MEMMAN_SetExhaustiveTesting        ||
         NULL == fp_MEMMAN_SetPaddingSize              ||
         NULL == fp_MEMMAN_BreakOnAllocation           ||
         NULL == fp_MEMMAN_BreakOnDeallocation         ||
         NULL == fp_MEMMAN_BreakOnReallocation         ||
         NULL == fp_MEMMAN_ValidateKnownAllocPtr       ||
         NULL == fp_MEMMAN_ValidateKnownAllocBlock     ||
         NULL == fp_MEMMAN_ValidateAccessibility       ||
         NULL == fp_MEMMAN_ValidateChunk               ||
         NULL == fp_MEMMAN_ValidatePendingDestructor   ||
         NULL == fp_MEMMAN_ValidateFinishedDestructor  ||
         NULL == fp_MEMMAN_AllocateMemory              ||
         NULL == fp_MEMMAN_DeAllocateMemory            ||
         NULL == fp_MEMMAN_SetOwner                     )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
    #if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

    fp_MEMMAN_CallstackScopeBegin = (TFP_MEMMAN_CallstackScopeBegin) dlsym( g_memoryManagerModulePtr, "MEMMAN_CallstackScopeBegin" );
    fp_MEMMAN_CallstackScopeEnd   = (TFP_MEMMAN_CallstackScopeEnd)   dlsym( g_memoryManagerModulePtr, "MEMMAN_CallstackScopeEnd" );

    if ( NULL == fp_MEMMAN_CallstackScopeBegin ||
         NULL == fp_MEMMAN_CallstackScopeEnd    )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
    #if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

    fp_MEMMAN_ExclusiveLockCreated   = (TFP_MEMMAN_ExclusiveLockCreated)   dlsym( g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockCreated" );
    fp_MEMMAN_ExclusiveLockObtained  = (TFP_MEMMAN_ExclusiveLockObtained)  dlsym( g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockObtained" );
    fp_MEMMAN_ExclusiveLockReleased  = (TFP_MEMMAN_ExclusiveLockReleased)  dlsym( g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockReleased" );
    fp_MEMMAN_ExclusiveLockAbandoned = (TFP_MEMMAN_ExclusiveLockAbandoned) dlsym( g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockAbandoned" );
    fp_MEMMAN_ExclusiveLockDestroy   = (TFP_MEMMAN_ExclusiveLockDestroy)   dlsym( g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockDestroy" );

    if ( NULL == fp_MEMMAN_ExclusiveLockCreated   ||
         NULL == fp_MEMMAN_ExclusiveLockObtained  ||
         NULL == fp_MEMMAN_ExclusiveLockReleased  ||
         NULL == fp_MEMMAN_ExclusiveLockAbandoned ||
         NULL == fp_MEMMAN_ExclusiveLockDestroy    )
    {
        dlclose( g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = NULL;
        return 0;
    }

    #endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

    if ( 1 == fp_MEMMAN_Initialize() )
        return 1;

    MEMMAN_UnloadMemoryManager();
    return 0;
}

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_PLATFORM */

/*-------------------------------------------------------------------------*/

inline
MEMMAN_Int32
MEMMAN_Initialize( void )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ? 0 : fp_MEMMAN_Initialize() );
}

/*-------------------------------------------------------------------------*/

inline
MEMMAN_Int32
MEMMAN_Shutdown( void )
{
    return MEMMAN_UnloadMemoryManager();
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_DumpLogReport( void )
{
    if ( NULL != fp_MEMMAN_DumpLogReport )
        fp_MEMMAN_DumpLogReport();
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_SetLogFile( const char* file )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_SetLogFile( file );
}

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_LOAD_H ? */
