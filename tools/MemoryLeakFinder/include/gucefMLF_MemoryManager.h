/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

 /*
  * This Memory Manager software provides the following functionality:
  *             1. Seamless interface.
  *             2. Tracking all memory allocations and deallocations.
  *             3. Reporting memory leaks, unallocated memory.
  *             4. Reporting memory bounds violations.
  *             5. Reporting the percentage of allocated memory that is actually being used.
  *             6. Customizable tracking.
  *
  * The code is self documented, thus reading through this header file should tell you
  * everything that you would ever need to know inorder to use the memory manager.
  */

#ifndef GUCEF_MLF_MEMORYMANAGER_H
#define GUCEF_MLF_MEMORYMANAGER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>   /* size_t */

#ifndef GUCEF_MLF_CONFIG_H
#include "gucefMLF_config.h"
#define GUCEF_MLF_CONFIG_H
#endif /* GUCEF_MLF_CONFIG_H ? */

#ifndef GUCEF_MLF_MACROS_H
#include "gucefMLF_macros.h"      /* module build configuration */
#define GUCEF_MLF_MACROS_H
#endif /* GUCEF_MLF_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
namespace GUCEF {
namespace MLF {
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Prevent C++ name mangling
 */
#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/

/*
 *      User API, these functions can be used to set parameters within the Memory
 *      Manager to control the type and extent of the memory tests that are performed.
 *      Note that it is not necessary to call any of these methods, you will get the default
 *      Memory Manager automatically.
 */

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C __int32
MEMMAN_Initialize( void );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C __int32
MEMMAN_Shutdown( void );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_dumpLogReport():
 *      Dump the log report to the file, this is the same method that is automatically called
 *      upon the programs termination to report all statistical information.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DumpLogReport( void );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_DumpMemoryAllocations():
 *      Report all allocated memory to the log file.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DumpMemoryAllocations( void );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_SetLogFile():
 *      Allows for the log file to be changed from the default.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_SetLogFile( const char *file );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_SetExhaustiveTesting():
 *      This method allows for exhaustive testing.  It has the same functionality as the following
 *      function calls => setLogAlways( true ); setPaddingSize( 1024 );
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_SetExhaustiveTesting( UInt32 test );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_SetLogAlways():
 *      Sets the flag for exhaustive information logging.  All information is sent to the log file.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_SetLogAlways( UInt32 log );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_SetPaddingSize():
 *      Sets the padding size for memory bounds checks.
 *      The default is 4
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_SetPaddingSize( UInt32 size );

/*-------------------------------------------------------------------------*/

/**
 *      MEMAN_CleanLogFile():
 *      Cleans out the log file by deleting it.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_CleanLogFile( UInt32 clean );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_BreakOnAllocation():
 *      Allows you to set a break point on the n-th allocation.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_BreakOnAllocation( int alloccount );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_BreakOnDeallocation():
 *      Sets a flag that will set a break point when the specified memory is deallocated.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_BreakOnDeallocation( void *address );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_BreakOnReallocation():
 *      Sets a flag that will set a break point when the specified memory is reallocated by
 *      using the realloc() method.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_BreakOnReallocation( void *address );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C void
MEMMAN_ValidateKnownAllocPtr( const void* address ,
                              const char *file    ,
                              int line            );

GUCEF_MLF_PUBLIC_C void
MEMMAN_ValidateKnownAllocBlock( const void* address ,
                                UInt32 blocksize    ,
                                const char *file    ,
                                int line            );

GUCEF_MLF_PUBLIC_C void
MEMMAN_ValidateChunk( const void* address ,
                      const void* chunk   ,
                      UInt32 blocksize    ,
                      const char *file    ,
                      int line            );

GUCEF_MLF_PUBLIC_C void
MEMMAN_ValidateAccessibility( const void* address ,
                              UInt32 blocksize    ,
                              const char *file    ,
                              int line            );

GUCEF_MLF_PUBLIC_C void
MEMMAN_ValidatePendingDestructor( const char* file     ,
                                  int line             ,
                                  const void* address  ,
                                  size_t size          ,
                                  const char* typeName );

GUCEF_MLF_PUBLIC_C void
MEMMAN_ValidateFinishedDestructor( const char* file     ,
                                   int line             ,
                                   const void* address  ,
                                   size_t size          ,
                                   const char* typeName );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_SuppressMismatchCheck():
 * Mark a live allocation so that dealloc type mismatch checking is skipped.
 * Use this for intentional cross-allocator patterns (e.g. code that acquires
 * with new and releases with free by design).
 * address must be a currently tracked reported-address.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_SuppressMismatchCheck( void* address );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_LockProtectsRange():
 * Declare that lockId is intended to protect the memory range [address, address+size).
 * At deallocation time, if the allocation overlaps the range and the lock is not
 * currently held, a warning is logged.  This is contract validation, not race detection.
 * Has no effect when TSan is active (GUCEF_MLF_TSAN_ACTIVE).
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_LockProtectsRange( void* lockId, const void* address, size_t size );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_LockUnprotectsRange():
 * Remove the range association previously declared for lockId.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_LockUnprotectsRange( void* lockId );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_DumpCallsiteReport():
 * Write per-callsite aggregated statistics to the log file.
 * Requires enableCallsiteProfiling=true. topN=0 means print all.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DumpCallsiteReport( UInt32 topN );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_DumpSizeHistogram():
 * Write the lifetime allocation size histogram to the log file.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DumpSizeHistogram( void );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_DumpTimeline():
 * Write a TSV timeline of live allocations (timestampUs, size, file, line) to path.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DumpTimeline( const char* path );

/*-------------------------------------------------------------------------*/

/**
 * MEMMAN_DumpMassifFormat():
 * Write a Valgrind massif-format snapshot of the current heap to path.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DumpMassifFormat( const char* path );

/*-------------------------------------------------------------------------*/

/*
 *      Memory Manager specific definitions.  Below are the definitions that make up the
 *      Memory Manager. These functions should not be called by the user directly.
 *      They are called by the redirect versions of new, malloc() ect.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Posible allocation/deallocation types.
 *
 *      Declared as characters to minimize memory footprint,
 *      char = 1 byte
 *      enum types = int = 32 bits = 8 bytes on standard machines
 */
#undef MM_UNKNOWN
#undef MM_NEW
#undef MM_PLACEMENT_NEW
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      MEMMAN_AllocateMemory():
 *      This is the main memory allocation routine, this is called by all of the other
 *      memory allocation routines to allocate and track memory.
 */
GUCEF_MLF_PUBLIC_C void*
MEMMAN_AllocateMemory( const char* file     , 
                       int line             ,
                       size_t size          ,
                       char allocType       ,
                       void* address        ,
                       const char* typeName );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_DeAllocateMemory():
 *      This is the main memory deallocation routine.
 *      This method is used by all of the other de-allocation routines
 *      for de-allocating and tracking memory.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DeAllocateMemory( void* address        ,
                         char allocType       ,
                         const char* typeName );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_DeAllocateMemoryEx():
 *      This is the main memory deallocation routine.
 *      This method is used by all of the other de-allocation routines
 *      for de-allocating and tracking memory.
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_DeAllocateMemoryEx( const char* file     ,
                           int line             ,
                           void* address        ,
                           char allocType       ,
                           const char* typeName );

/*-------------------------------------------------------------------------*/

/**
 *      MEMMAN_SetOwner():
 *      This method is used by the deallocation methods to record the source file and line
 *      number that is requesting the allocation.  Note that it is important to create a
 *      seperate method for deallocation since we can not pass the addition parameters to
 *      the delete methods like we do with the new methods.
 */
GUCEF_MLF_PUBLIC_C __int32
MEMMAN_SetOwner( const char* file     ,
                 int line             ,
                 const char* typeName );

/*-------------------------------------------------------------------------*/

#ifdef MEMCHECK_OLEAPI

/**
 *      Memory check version of the OLE function SysAllocString
 */
GUCEF_MLF_PUBLIC_C
wchar_t*
MEMMAN_SysAllocString( const char* file ,
                       int line         ,
                       const wchar_t*   );

/*-------------------------------------------------------------------------*/

/**
 *      Memory check version of the OLE function SysAllocStringByteLen
 */
GUCEF_MLF_PUBLIC_C
wchar_t*
MEMMAN_SysAllocStringByteLen( const char* file        ,
                              int line                ,
                              const char* str         ,
                              unsigned int bufferSize );

/*-------------------------------------------------------------------------*/

/**
 *      Memory check version of the OLE function SysAllocStringLen
 */
GUCEF_MLF_PUBLIC_C
wchar_t*
MEMMAN_SysAllocStringLen( const char* file         ,
                          int line                 ,
                          const wchar_t* strIn     ,
                          unsigned int charsToCopy );

/*-------------------------------------------------------------------------*/

/**
 *      Memory check version of the OLE function SysFreeString
 */
GUCEF_MLF_PUBLIC_C
void
MEMMAN_SysFreeString( const char *file    ,
                      int line            ,
                      wchar_t* bstrString );

/*-------------------------------------------------------------------------*/

/**
 *      Memory check version of the OLE function SysReAllocString
 */
GUCEF_MLF_PUBLIC_C
void
MEMMAN_SysReAllocString( const char *file    ,
                         int line            ,
                         wchar_t** pbstr     , 
                         const wchar_t* psz  );

/*-------------------------------------------------------------------------*/

/**
 *      Memory check version of the OLE function SysReAllocStringLen
 */
GUCEF_MLF_PUBLIC_C
void
MEMMAN_SysReAllocStringLen( const char *file    ,
                            int line            ,
                            wchar_t** pbstr     , 
                            const wchar_t* psz  ,
                            unsigned int len    );
#endif

/*-------------------------------------------------------------------------*/

/*
 *      End the C++ name mangling protection
 */
#ifdef __cplusplus
};
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace MLF */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MLF_MEMORYMANAGER_H */
