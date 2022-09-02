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

#ifndef GUCEF_DYNNEWOFF_H 
#define GUCEF_DYNNEWOFF_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CONFIG_H
#include "gucef_config.h"
#define GUCEF_CONFIG_H
#endif /* GUCEF_CONFIG_H ? */

#if defined( GUCEF_USE_MEMORY_LEAK_CHECKER ) && defined( GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER )

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *      Undefine all symbols defined by the Memory Manager.  This is important when including 
 *      header files that also overwrite the new/delete operators to avoid circular definitions
 *      that will result in unexpected behavior and syntax errors from the compiler.
 *
 *      As a rule always include this file BEFORE any STL headers !!!.
 */

#undef malloc
#undef calloc
#undef realloc
#undef free

#ifdef GUCEF_MEMCHECK_OLEAPI

#undef SysAllocString
#undef SysAllocStringByteLen
#undef SysAllocStringLen
#undef SysFreeString
#undef SysReAllocString
#undef SysReAllocStringLen

#endif /* GUCEF_MEMCHECK_OLEAPI ? */

#ifdef __cplusplus
  #undef new
  #undef delete
#endif /* __cplusplus ? */

#include <malloc.h>
#include <stdlib.h> 

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_USE_MEMORY_LEAK_CHECKER && GUCEF_USE_PLATFORM_MEMORY_LEAK_CHECKER? */

/* 
 *  We still need to provide the GUCEF memory check macros to prevent compilation errors
 *  However we turn them into no-ops
 */
#undef GUCEF_CHECKALLOCPTR
#undef GUCEF_CHECKMEM
#undef GUCEF_CHECKMEMSEG
#define GUCEF_CHECKALLOCPTR( addr )
#define GUCEF_CHECKMEM( addr, size )
#define GUCEF_CHECKMEMSEG( addr, chunk, size )

/*
#define MEMMAN_LazyLoadMemoryManager()
#define MEMMAN_malloc( file, line, size )
#define MEMMAN_calloc( file, line, num, sz )
#define MEMMAN_realloc( file, line, ptr, sz )
#define MEMMAN_free( file, line, ptr )
#define MEMMAN_SysAllocString( file, line, wcharStr )
#define MEMMAN_SysAllocStringByteLen( file, line, psz, len )
#define MEMMAN_SysAllocStringLen( file, line, strIn, ui )
#define MEMMAN_SysFreeString( file, line, bstrString )
#define MEMMAN_SysReAllocString( file, line, pbstr, psz )
#define MEMMAN_SysReAllocStringLen( file, line, pbstr, psz, len )
#define fp_MEMMAN_Initialize()
#define fp_MEMMAN_DumpMemoryAllocations()
#define fp_MEMMAN_DumpLogReport()
#define fp_MEMMAN_ValidateKnownAllocPtr( addr, file, line  )
#define fp_MEMMAN_Validate( addr, size, file, line )
#define fp_MEMMAN_ValidateChunk( addr, chunk, size, file, line )
*/

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DYNNEWOFF_H ? */
