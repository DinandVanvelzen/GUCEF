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

#ifndef GUCEF_MEMORYMANAGERLOADER_H
#define GUCEF_MEMORYMANAGERLOADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#ifndef GUCEF_TYPES_H
#include "gucef_types.h"
#define GUCEF_TYPES_H
#endif /* GUCEF_TYPES_H ? */

#ifndef GUCEF_BASICHELPERS_H
#include "gucef_basichelpers.h"
#define GUCEF_BASICHELPERS_H
#endif /* GUCEF_BASICHELPERS_H ? */

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

#ifndef GUCEF_DYNNEWOFF_H 
#include "gucef_dynnewoff.h"      /* Make sure that the new/delete are not declared to avoid circular definitions. */
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"      /* GUCEF platform build configuration */
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

#ifndef GUCEF_CALLCONV_H
#include "gucef_callconv.h"      /* GUCEF platform calling convention macros */
#define GUCEF_CALLCONV_H
#endif /* GUCEF_CALLCONV_H ? */

#ifndef GUCEF_SHAREDMODULE_H
#include "gucef_sharedmodule.h"  /* GUCEF platform macros related to the use of shared modules */
#define GUCEF_SHAREDMODULE_H
#endif /* GUCEF_SHAREDMODULE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*
 *      Posible allocation/deallocation types.
 *
 *      Declared as characters to minimize memory footprint,
 *      char = 1 byte
 *      enum types = int = 32 bits = 8 bytes on standard machines
 */
#undef MM_UNKNOWN
#undef MM_NEW
#undef MM_NEW_ARRAY
#undef MM_MALLOC
#undef MM_CALLOC
#undef MM_REALLOC
#undef MM_DELETE
#undef MM_DELETE_ARRAY
#undef MM_FREE
#undef MM_OLE_ALLOC
#undef MM_OLE_FREE
#define MM_UNKNOWN        0
#define MM_NEW            1
#define MM_PLACEMENT_NEW  2
#define MM_NEW_ARRAY      3
#define MM_MALLOC         4
#define MM_CALLOC         5
#define MM_REALLOC        6
#define MM_DELETE         7
#define MM_DELETE_ARRAY   8
#define MM_FREE           9
#define MM_OLE_ALLOC      10
#define MM_OLE_FREE       10

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */

#define MEMMAN_Int32      __int32

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  General functions to control the behavior of the external module
 */

#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) )

typedef MEMMAN_Int32 ( *TFP_MEMMAN_Initialize )( void );
typedef MEMMAN_Int32 ( *TFP_MEMMAN_Shutdown )( void );
typedef void ( *TFP_MEMMAN_DumpLogReport )( void );
typedef void ( *TFP_MEMMAN_SetLogFile )( const char *file );
typedef void ( *TFP_MEMMAN_CleanLogFile )( unsigned __int32 test );
typedef void ( *TFP_MEMMAN_SetLogAlways )( unsigned __int32 log );

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

/*
 *  Memory allocation and memory validity related functionality
 */

typedef void ( *TFP_MEMMAN_DumpMemoryAllocations )( void );
typedef void ( *TFP_MEMMAN_SetExhaustiveTesting )( unsigned __int32 test );
typedef void ( *TFP_MEMMAN_SetPaddingSize )( unsigned __int32 clean );
typedef void ( *TFP_MEMMAN_BreakOnAllocation )( int alloccount );
typedef void ( *TFP_MEMMAN_BreakOnDeallocation )( void* address );
typedef void ( *TFP_MEMMAN_BreakOnReallocation )( void* address );
typedef void ( *TFP_MEMMAN_ValidateKnownAllocPtr )( const void* address, const char* file, int line );
typedef void ( *TFP_MEMMAN_ValidateKnownAllocBlock )( const void* address, unsigned __int32 blocksize, const char* file, int line );
typedef void ( *TFP_MEMMAN_ValidateAccessibility )( const void* address, unsigned __int32 blocksize, const char* file, int line );
typedef void ( *TFP_MEMMAN_ValidateChunk )( const void* address, const void* chunk, unsigned __int32 blocksize, const char* file, int line );
typedef void ( *TFP_MEMMAN_ValidatePendingDestructor )( const char* file, int line, const void* address, size_t size, const char* typeName );
typedef void ( *TFP_MEMMAN_ValidateFinishedDestructor )( const char* file, int line, const void* address, size_t size, const char* typeName );

/*-------------------------------------------------------------------------*/

/*
 *  Memory tracking functions which are invoked by the memory allocation overrides
 */

typedef void* ( *TFP_MEMMAN_AllocateMemory )( const char *file, int line, size_t size, char allocType, void* address, const char* typeName );
typedef void ( *TFP_MEMMAN_DeAllocateMemory )( void *address, char allocType, const char* typeName );
typedef void ( *TFP_MEMMAN_DeAllocateMemoryEx )( const char *file, int line, void *address, char allocType, const char* typeName );
typedef MEMMAN_Int32 ( *TFP_MEMMAN_SetOwner )( const char *file, int line, const char* typeName );

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

/*-------------------------------------------------------------------------*/

/*
 *  OLE Memory tracking functions which are invoked by the OLE memory allocation overrides
 */

typedef wchar_t* ( *TFP_MEMMAN_SysAllocString )( const char *file, int line, wchar_t* wcharStr );
typedef wchar_t* ( *TFP_MEMMAN_SysAllocStringByteLen )( const char *file, int line, const char* str, unsigned int bufferSize );
typedef wchar_t* ( *TFP_MEMMAN_SysAllocStringLen )( const char *file, int line, const wchar_t* str, unsigned int charsToCopy );
typedef void ( *TFP_MEMMAN_SysFreeString )( const char *file, int line, wchar_t* bstrString );
typedef int ( *TFP_MEMMAN_SysReAllocString )( const char *file, int line, wchar_t** pbstr, const wchar_t* psz );
typedef int ( *TFP_MEMMAN_SysReAllocStringLen )( const char *file, int line, wchar_t** pbstr, const wchar_t* psz, unsigned int len );

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

/*-------------------------------------------------------------------------*/

/*
 *  Platform callstack tracing functions
 */

typedef void ( *TFP_MEMMAN_CallstackScopeBegin )( const char *file, int line );
typedef void ( *TFP_MEMMAN_CallstackScopeEnd )( void );

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

/*
 *  Platform lock tracing functions
 */

typedef void ( *TFP_MEMMAN_ExclusiveLockCreated )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockObtained )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockReleased )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockAbandoned )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockDestroy )( void* lockId );

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) )

static TFP_MEMMAN_Initialize fp_MEMMAN_Initialize = 0;
static TFP_MEMMAN_Shutdown fp_MEMMAN_Shutdown = 0;
static TFP_MEMMAN_DumpLogReport fp_MEMMAN_DumpLogReport = 0;
static TFP_MEMMAN_SetLogFile fp_MEMMAN_SetLogFile = 0;
static TFP_MEMMAN_SetLogAlways fp_MEMMAN_SetLogAlways = 0;
static TFP_MEMMAN_CleanLogFile fp_MEMMAN_CleanLogFile = 0;

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_DumpMemoryAllocations fp_MEMMAN_DumpMemoryAllocations = 0;
static TFP_MEMMAN_SetExhaustiveTesting fp_MEMMAN_SetExhaustiveTesting = 0;
static TFP_MEMMAN_SetPaddingSize fp_MEMMAN_SetPaddingSize = 0;
static TFP_MEMMAN_BreakOnAllocation fp_MEMMAN_BreakOnAllocation = 0;
static TFP_MEMMAN_BreakOnDeallocation fp_MEMMAN_BreakOnDeallocation = 0;
static TFP_MEMMAN_BreakOnReallocation fp_MEMMAN_BreakOnReallocation = 0;
static TFP_MEMMAN_ValidateKnownAllocPtr fp_MEMMAN_ValidateKnownAllocPtr = 0;
static TFP_MEMMAN_ValidateKnownAllocBlock fp_MEMMAN_ValidateKnownAllocBlock = 0;
static TFP_MEMMAN_ValidateAccessibility fp_MEMMAN_ValidateAccessibility = 0;
static TFP_MEMMAN_ValidateChunk fp_MEMMAN_ValidateChunk = 0;
static TFP_MEMMAN_ValidatePendingDestructor fp_MEMMAN_ValidatePendingDestructor = 0;
static TFP_MEMMAN_ValidateFinishedDestructor fp_MEMMAN_ValidateFinishedDestructor = 0;

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_AllocateMemory fp_MEMMAN_AllocateMemory = 0;
static TFP_MEMMAN_DeAllocateMemory fp_MEMMAN_DeAllocateMemory = 0;
static TFP_MEMMAN_DeAllocateMemoryEx fp_MEMMAN_DeAllocateMemoryEx = 0;
static TFP_MEMMAN_SetOwner fp_MEMMAN_SetOwner = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_SysAllocString fp_MEMMAN_SysAllocString = 0;
static TFP_MEMMAN_SysAllocStringByteLen fp_MEMMAN_SysAllocStringByteLen = 0;
static TFP_MEMMAN_SysAllocStringLen fp_MEMMAN_SysAllocStringLen = 0;
static TFP_MEMMAN_SysFreeString fp_MEMMAN_SysFreeString = 0;
static TFP_MEMMAN_SysReAllocString fp_MEMMAN_SysReAllocString = 0;
static TFP_MEMMAN_SysReAllocStringLen fp_MEMMAN_SysReAllocStringLen = 0;

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

/*-------------------------------------------------------------------------*/

static TFP_MEMMAN_CallstackScopeBegin   fp_MEMMAN_CallstackScopeBegin = 0;
static TFP_MEMMAN_CallstackScopeEnd     fp_MEMMAN_CallstackScopeEnd = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

/*
 *  Platform lock tracing functions
 */

static TFP_MEMMAN_ExclusiveLockCreated      fp_MEMMAN_ExclusiveLockCreated = 0;
static TFP_MEMMAN_ExclusiveLockObtained     fp_MEMMAN_ExclusiveLockObtained = 0;
static TFP_MEMMAN_ExclusiveLockReleased     fp_MEMMAN_ExclusiveLockReleased = 0;
static TFP_MEMMAN_ExclusiveLockAbandoned    fp_MEMMAN_ExclusiveLockAbandoned = 0;
static TFP_MEMMAN_ExclusiveLockDestroy      fp_MEMMAN_ExclusiveLockDestroy = 0;

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) )

/*-------------------------------------------------------------------------*/

static void* g_memoryManagerModulePtr = NULL; 
static void* g_dynLoadMutex = NULL;
static const char* MemoryLeakFinderLib = "MemoryLeakFinder_d.dll";

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) )

static MEMMAN_Int32 GUCEF_HIDDEN
MEMMAN_UnloadMemoryManager( void )
{
    /* check to make sure we have actually loaded the library */
    if ( 0 == g_memoryManagerModulePtr || 0 == g_dynLoadMutex ) 
        return 1;
    /* try to get a lock to safely unload */
    if ( WAIT_OBJECT_0 != WaitForSingleObject( (HANDLE) g_dynLoadMutex, 30000 ) ) 
        return 0;
    /* double check module and lock availability after obtaining the lock */
    if ( 0 == g_memoryManagerModulePtr || 0 == g_dynLoadMutex ) 
        return 1;

    if ( GUCEF_NULL != fp_MEMMAN_Shutdown )
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
        /* we have the module loaded and lock availability but we dont have the functions correctly linked */
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
    /* check to make sure we havent already loaded the library */
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

    /* check to see if the module is already loaded elsewhere in the process */
    g_memoryManagerModulePtr = (void*) GetModuleHandleA( MemoryLeakFinderLib );
    if ( 0 == g_memoryManagerModulePtr )
    {
        /* load the library */
        g_memoryManagerModulePtr = (void*) LoadLibrary( MemoryLeakFinderLib );
        if ( 0 == g_memoryManagerModulePtr )
        {
            ReleaseMutex( (HANDLE) g_dynLoadMutex );
            return 0;
        }
    }

    /* load the function pointers */
    fp_MEMMAN_Initialize = (TFP_MEMMAN_Initialize) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_Initialize" );
    fp_MEMMAN_Shutdown = (TFP_MEMMAN_Shutdown) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_Shutdown" );
    fp_MEMMAN_DumpLogReport = (TFP_MEMMAN_DumpLogReport) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DumpLogReport" );
    fp_MEMMAN_SetLogFile = (TFP_MEMMAN_SetLogFile) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetLogFile" );
    fp_MEMMAN_SetLogAlways = (TFP_MEMMAN_SetLogAlways) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetLogAlways" );
    fp_MEMMAN_CleanLogFile = (TFP_MEMMAN_CleanLogFile) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_CleanLogFile" );

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
    
    fp_MEMMAN_DumpMemoryAllocations = (TFP_MEMMAN_DumpMemoryAllocations) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DumpMemoryAllocations" );    
    fp_MEMMAN_SetExhaustiveTesting = (TFP_MEMMAN_SetExhaustiveTesting) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetExhaustiveTesting" );    
    fp_MEMMAN_SetPaddingSize = (TFP_MEMMAN_SetPaddingSize) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetPaddingSize" );    
    fp_MEMMAN_BreakOnAllocation = (TFP_MEMMAN_BreakOnAllocation) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_BreakOnAllocation" );
    fp_MEMMAN_BreakOnDeallocation = (TFP_MEMMAN_BreakOnDeallocation) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_BreakOnDeallocation" );
    fp_MEMMAN_BreakOnReallocation = (TFP_MEMMAN_BreakOnReallocation) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_BreakOnReallocation" );
    fp_MEMMAN_ValidateKnownAllocPtr = (TFP_MEMMAN_ValidateKnownAllocPtr) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateKnownAllocPtr" );
    fp_MEMMAN_ValidateKnownAllocBlock = (TFP_MEMMAN_ValidateKnownAllocBlock) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateKnownAllocBlock" );
    fp_MEMMAN_ValidateAccessibility = (TFP_MEMMAN_ValidateAccessibility) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateAccessibility" );
    fp_MEMMAN_ValidateChunk = (TFP_MEMMAN_ValidateChunk) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateChunk" );
    fp_MEMMAN_ValidatePendingDestructor = (TFP_MEMMAN_ValidatePendingDestructor) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidatePendingDestructor" );
    fp_MEMMAN_ValidateFinishedDestructor = (TFP_MEMMAN_ValidateFinishedDestructor) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ValidateFinishedDestructor" );
    fp_MEMMAN_AllocateMemory = (TFP_MEMMAN_AllocateMemory) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_AllocateMemory" );
    fp_MEMMAN_DeAllocateMemory = (TFP_MEMMAN_DeAllocateMemory) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DeAllocateMemory" );
    fp_MEMMAN_DeAllocateMemoryEx = (TFP_MEMMAN_DeAllocateMemoryEx) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_DeAllocateMemoryEx" );
    fp_MEMMAN_SetOwner = (TFP_MEMMAN_SetOwner) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SetOwner" );

    if ( 0 == fp_MEMMAN_DumpMemoryAllocations      ||
         0 == fp_MEMMAN_SetExhaustiveTesting       ||
         0 == fp_MEMMAN_SetPaddingSize             || 
         0 == fp_MEMMAN_BreakOnAllocation          ||
         0 == fp_MEMMAN_BreakOnDeallocation        ||
         0 == fp_MEMMAN_BreakOnReallocation        ||
         0 == fp_MEMMAN_ValidateKnownAllocPtr      ||
         0 == fp_MEMMAN_ValidateKnownAllocBlock    ||
         0 == fp_MEMMAN_ValidateAccessibility      ||
         0 == fp_MEMMAN_ValidateChunk              ||
         0 == fp_MEMMAN_ValidatePendingDestructor  ||
         0 == fp_MEMMAN_ValidateFinishedDestructor ||
         0 == fp_MEMMAN_AllocateMemory             ||
         0 == fp_MEMMAN_DeAllocateMemory           ||
         0 == fp_MEMMAN_SetOwner                    ) 
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
    #if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

    fp_MEMMAN_SysAllocString = (TFP_MEMMAN_SysAllocString) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysAllocString" );
    fp_MEMMAN_SysAllocStringByteLen = (TFP_MEMMAN_SysAllocStringByteLen) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysAllocStringByteLen" );
    fp_MEMMAN_SysAllocStringLen = (TFP_MEMMAN_SysAllocStringLen) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysAllocStringLen" );
    fp_MEMMAN_SysFreeString = (TFP_MEMMAN_SysFreeString) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysFreeString" );
    fp_MEMMAN_SysReAllocString = (TFP_MEMMAN_SysReAllocString) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysReAllocString" );
    fp_MEMMAN_SysReAllocStringLen = (TFP_MEMMAN_SysReAllocStringLen) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_SysReAllocStringLen" );

    if ( 0 == fp_MEMMAN_SysAllocString ||
         0 == fp_MEMMAN_SysAllocStringByteLen ||
         0 == fp_MEMMAN_SysAllocStringLen ||
         0 == fp_MEMMAN_SysFreeString ||
         0 == fp_MEMMAN_SysReAllocString ||
         0 == fp_MEMMAN_SysReAllocStringLen )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
    #if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

    fp_MEMMAN_CallstackScopeBegin = (TFP_MEMMAN_CallstackScopeBegin) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_CallstackScopeBegin" );
    fp_MEMMAN_CallstackScopeEnd = (TFP_MEMMAN_CallstackScopeEnd) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_CallstackScopeEnd" );

    if ( 0 == fp_MEMMAN_CallstackScopeBegin ||
         0 == fp_MEMMAN_CallstackScopeEnd )
    {
        FreeLibrary( (HMODULE) g_memoryManagerModulePtr );
        g_memoryManagerModulePtr = 0;
        return 0;
    }

    #endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
    #if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

    fp_MEMMAN_ExclusiveLockCreated = (TFP_MEMMAN_ExclusiveLockCreated) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockCreated" );
    fp_MEMMAN_ExclusiveLockObtained = (TFP_MEMMAN_ExclusiveLockObtained) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockObtained" );
    fp_MEMMAN_ExclusiveLockReleased = (TFP_MEMMAN_ExclusiveLockReleased) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockReleased" );
    fp_MEMMAN_ExclusiveLockAbandoned = (TFP_MEMMAN_ExclusiveLockAbandoned) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockAbandoned" );
    fp_MEMMAN_ExclusiveLockDestroy = (TFP_MEMMAN_ExclusiveLockDestroy) GetProcAddress( (HMODULE) g_memoryManagerModulePtr, "MEMMAN_ExclusiveLockDestroy" );

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
    {        
        return 1;
    }
    MEMMAN_UnloadMemoryManager();
    return 0;
}

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
MEMMAN_SetLogFile( const char *file )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_SetLogFile( file );    
}

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ) || ( defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_SetExhaustiveTesting( unsigned __int32 test )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_SetExhaustiveTesting( test );   
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_SetPaddingSize( unsigned __int32 clean )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_SetPaddingSize( clean );
}

/*-------------------------------------------------------------------------*/

inline
MEMMAN_Int32
MEMMAN_SetOwner( const char *file, int line, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        return fp_MEMMAN_SetOwner( file, line, typeName );    
    return 0;
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_malloc( const char *file, int line, size_t size )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ? malloc( size ) : fp_MEMMAN_AllocateMemory( file, line, size, MM_MALLOC, GUCEF_NULL, GUCEF_NULL ) );    
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_placement_new( const char *file, int line, size_t size, void* address, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_AllocateMemory( file, line, size, MM_PLACEMENT_NEW, address, typeName );    
    return address;
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_calloc( const char *file, int line, size_t num, size_t size )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ? calloc( num, size ) : fp_MEMMAN_AllocateMemory( file, line, size*num, MM_CALLOC, NULL, GUCEF_NULL ) );    
}

/*-------------------------------------------------------------------------*/

inline
void*
MEMMAN_realloc( const char *file, int line, void* ptr, size_t size )
{
    return ( 0 == MEMMAN_LazyLoadMemoryManager() ? realloc( ptr, size ) : ( ptr ? fp_MEMMAN_AllocateMemory( file, line, size, MM_REALLOC, ptr, GUCEF_NULL ) : fp_MEMMAN_AllocateMemory( file, line, size, MM_MALLOC, GUCEF_NULL, GUCEF_NULL ) ) );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_free( const char *file, int line, void* ptr )
{
    ( 0 == MEMMAN_LazyLoadMemoryManager() ? free( ptr ) : fp_MEMMAN_DeAllocateMemoryEx( file, line, ptr, MM_FREE, GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ValidateAccessibility( const void* address, unsigned __int32 blocksize, const char* file, int line )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ValidateAccessibility( address, blocksize, file, line );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ValidatePendingDestructor( const char* file, int line, const void* address, size_t blocksize, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ValidatePendingDestructor( file, line, address, blocksize, typeName );
}

/*-------------------------------------------------------------------------*/

inline
void
MEMMAN_ValidateFinishedDestructor( const char* file, int line, const void* address, size_t blocksize, const char* typeName )
{
    if ( 0 != MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ValidateFinishedDestructor( file, line, address, blocksize, typeName );
}

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */
#if ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) )

/*-------------------------------------------------------------------------*/

inline
wchar_t* 
MEMMAN_SysAllocString( const char *file, int line, wchar_t* wcharStr )
{    
    wchar_t* ptr = ( 0 == MEMMAN_LazyLoadMemoryManager() ? SysAllocString( wcharStr ) : fp_MEMMAN_SysAllocString( file, line, wcharStr ) );
    //fp_MEMMAN_DumpMemoryAllocations();
    return ptr;
}

/*-------------------------------------------------------------------------*/

inline
wchar_t* 
MEMMAN_SysAllocStringByteLen( const char *file, int line, const char* str, unsigned int bufferSize )
{
    wchar_t* ptr = ( 0 == MEMMAN_LazyLoadMemoryManager() ? SysAllocStringByteLen( str, bufferSize ) : fp_MEMMAN_SysAllocStringByteLen( file, line, str, bufferSize ) );
    //fp_MEMMAN_DumpMemoryAllocations();
    return ptr;
}

/*-------------------------------------------------------------------------*/

inline
wchar_t* 
MEMMAN_SysAllocStringLen( const char *file, int line, const wchar_t* str, unsigned int charsToCopy )
{
    wchar_t* ptr = ( 0 == MEMMAN_LazyLoadMemoryManager() ? SysAllocStringLen( str, charsToCopy ) : fp_MEMMAN_SysAllocStringLen( file, line, str, charsToCopy ) );
    //fp_MEMMAN_DumpMemoryAllocations();
    return ptr;
}

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_SysFreeString( const char *file, int line, wchar_t* bstrString )
{
    ( 0 == MEMMAN_LazyLoadMemoryManager() ? SysFreeString( bstrString ) : fp_MEMMAN_SysFreeString( file, line, bstrString ) );
    //fp_MEMMAN_DumpMemoryAllocations();
}

/*-------------------------------------------------------------------------*/

inline
int 
MEMMAN_SysReAllocString( const char* file, int line, wchar_t** pbstr, const wchar_t* psz )
{
    int result = ( 0 == MEMMAN_LazyLoadMemoryManager() ? SysReAllocString( pbstr, psz ) : fp_MEMMAN_SysReAllocString( file, line, pbstr, psz ) );
    //fp_MEMMAN_DumpMemoryAllocations();
    return result;
}

/*-------------------------------------------------------------------------*/

inline
int 
MEMMAN_SysReAllocStringLen( const char* file, int line, wchar_t** pbstr, const wchar_t* psz, unsigned int len )
{
    int result = ( 0 == MEMMAN_LazyLoadMemoryManager() ? SysReAllocStringLen( pbstr, psz, len ) : fp_MEMMAN_SysReAllocStringLen( file, line, pbstr, psz, len ) );
    //fp_MEMMAN_DumpMemoryAllocations();
    return result;
}

/*-------------------------------------------------------------------------*/

#endif /* ( defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) && defined( GUCEF_PLATFORM_MEMORY_LEAK_CHECKER_INCLUDES_OLEAPI ) ) ? */
#if defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING )

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_CallstackScopeBegin( const char *file, int line )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_CallstackScopeBegin( file, line ); 
}

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_CallstackScopeEnd( void )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_CallstackScopeEnd();
}

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_ExclusiveLockCreated( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockCreated( lockId ); 
}

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_ExclusiveLockObtained( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockObtained( lockId ); 
}

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_ExclusiveLockReleased( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockReleased( lockId );
}

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_ExclusiveLockAbandoned( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockAbandoned( lockId );
}

/*-------------------------------------------------------------------------*/

inline
void 
MEMMAN_ExclusiveLockDestroy( void* lockId )
{
    if ( 1 == MEMMAN_LazyLoadMemoryManager() )
        fp_MEMMAN_ExclusiveLockDestroy( lockId ); 
}

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */
#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------*/

#undef GUCEF_DYNNEWOFF_H  /* allow the undef header to be included again later */

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER ) ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_H ? */
