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

#ifndef GUCEF_MEMORYMANAGERLOADER_TYPES_H
#define GUCEF_MEMORYMANAGERLOADER_TYPES_H

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

#ifndef GUCEF_PLATFORM_H
#include "gucef_platform.h"
#define GUCEF_PLATFORM_H
#endif /* GUCEF_PLATFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*
 *      Possible allocation/deallocation types.
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
#define MM_OLE_FREE       11

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
 *  OLE Memory tracking function pointer types
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
 *  Platform callstack tracing function pointer types
 */

typedef void ( *TFP_MEMMAN_CallstackScopeBegin )( const char *file, int line );
typedef void ( *TFP_MEMMAN_CallstackScopeEnd )( void );

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_CALLSTACK_TRACING ) && defined( GUCEF_USE_PLATFORM_CALLSTACK_TRACING ) ? */
#if defined( GUCEF_USE_PLATFORM_LOCK_TRACER )

/*-------------------------------------------------------------------------*/

/*
 *  Platform lock tracing function pointer types
 */

typedef void ( *TFP_MEMMAN_ExclusiveLockCreated )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockObtained )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockReleased )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockAbandoned )( void* lockId );
typedef void ( *TFP_MEMMAN_ExclusiveLockDestroy )( void* lockId );

/*-------------------------------------------------------------------------*/

#endif /* defined( GUCEF_USE_PLATFORM_LOCK_TRACER ) ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MEMORYMANAGERLOADER_TYPES_H ? */
