/*
 *  gucefMT: GUCEF module providing multithreading solutions
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

#ifndef GUCEF_CALLSTACK_H
#define GUCEF_CALLSTACK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MACROS_H
#include "gucef_macros.h"
#define GUCEF__MACROS_H
#endif /* GUCEF_MACROS_H ? */

#ifndef GUCEF_MLF_CONFIG_H
#include "gucefMLF_config.h"
#define GUCEF_MLF_CONFIG_H
#endif /* GUCEF_MLF_CONFIG_H ? */

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

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

#ifdef __cplusplus
   extern "C" {
#endif   /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SCallStack
{
    const char** file;
    Int32* linenr;
    UInt64* entryTickCount;
    UInt32 items;
    UInt32 reservedStacksize;
    UInt32 threadid;
    UInt8  staticsAreCopied;
};
typedef struct SCallStack TCallStack;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *      Should be called at the beginning of a function.
 *      Use RAII in C++ to wrap calls to Begin and End
 *
 *      @param file filename of the source file from where this function is called
 *      @param line line number in the caller source file
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_CallstackScopeBegin( const char* file ,
                            Int32 line       );

/*-------------------------------------------------------------------------*/

/**
 *      Should be called at the end of a function scope.
 *      Use RAII in C++ to wrap calls to Begin and End
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_CallstackScopeEnd( void );

/*-------------------------------------------------------------------------*/

/**
 *  Function for obtaining the callstack for the current calling thread
 *  Threadsafety wise the callstack structure's lifespan is only valid during the duration
 *  of the calling function's scope!
 *  If no callstack trace is available the output stack pointer is set to GUCEF_NULL
 */
GUCEF_MLF_PUBLIC_C Int32
MEMMAN_GetCallstackForCurrentThread( TCallStack** outStack );

/*-------------------------------------------------------------------------*/

/**
 *  Same as MEMMAN_GetCallstackForCurrentThread() except it allocates new memory
 *  for the callstack information if available and thus its lifespan is not limited to
 *  the calling function's scope.
 *  Do note that the caller is responsible for deleting the allocated memory when finished
 *  with the data using MEMMAN_FreeCallstackCopy()
 */
GUCEF_MLF_PUBLIC_C Int32
MEMMAN_GetCallstackCopyForCurrentThread( TCallStack** outStack  ,
                                         UInt32 alsoCopyStatics );

/*-------------------------------------------------------------------------*/

/**
 *  Use this to free memory allocated by MEMMAN_GetCallstackCopyForCurrentThread()
 */
GUCEF_MLF_PUBLIC_C
void
MEMMAN_FreeCallstackCopy( TCallStack* stackCopy );

/*-------------------------------------------------------------------------*/

/**
 *      Prints the current call stack to stdout
 */
GUCEF_MLF_PUBLIC_C void
GUCEF_PrintCallstack( void );

/*-------------------------------------------------------------------------*/

/**
 *      Prints the current call stack to stdout
 */
GUCEF_MLF_PUBLIC_C void
GUCEF_PrintCallstackCopy( TCallStack* stackToPrint );


/*-------------------------------------------------------------------------*/

/**
 *      Prints the current call stack to the given file
 */
GUCEF_MLF_PUBLIC_C void
GUCEF_PrintCallstackCopyTo( TCallStack* stackToPrint, FILE* dest );

/*-------------------------------------------------------------------------*/

/**
 *      Dumps the current call stack to the file with the given name
 *
 *      @param filename path and name of the output file.
 */
GUCEF_MLF_PUBLIC_C void
GUCEF_DumpCallstack( const char* filename );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C void
GUCEF_LogStackTo( const char* filename );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C void
GUCEF_LogStackToStdOut( void );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C void
GUCEF_SetStackLogging( const UInt32 logStackBool );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C void
GUCEF_SetStackLoggingInCvsFormat( const UInt32 logAsCvsBool );

/*-------------------------------------------------------------------------*/

typedef void (*TStackPushCallback) ( const char* fileName     ,
                                     const int lineNr         ,
                                     const UInt32 threadId    ,
                                     const UInt32 stackHeight );

GUCEF_MLF_PUBLIC_C void
GUCEF_SetStackPushCallback( TStackPushCallback callback );

/*-------------------------------------------------------------------------*/

typedef void (*TStackPopCallback) ( const char* fileName     ,
                                    const int lineNr         ,
                                    const UInt32 threadId    ,
                                    const UInt32 stackHeight ,
                                    const UInt32 ticksSpent  );

GUCEF_MLF_PUBLIC_C void
GUCEF_SetStackPopCallback( TStackPopCallback callback );

/*-------------------------------------------------------------------------*/

/**
 *  Initializes the call stack utility, should be called before using any of
 *  the functions.
 */
GUCEF_MLF_PUBLIC_C void
GUCEF_InitCallstackUtility( void );

/*-------------------------------------------------------------------------*/

GUCEF_MLF_PUBLIC_C void
GUCEF_ShutdowntCallstackUtility( void );

/*-------------------------------------------------------------------------*/

/*
 * Forward declaration — defined in CAllocationRecord.h.
 * Declared here so the raw-callstack C API can be used without pulling in
 * the full CAllocationRecord header.
 */
#ifdef __cplusplus
struct TRawCallStack;
#else
typedef struct TRawCallStack TRawCallStack;
struct TRawCallStack;
#endif

/**
 * Capture the current thread's OS-level callstack (instruction-pointer frames).
 *
 * Windows : RtlCaptureStackBackTrace — no DbgHelp needed for capture.
 * Linux   : backtrace() from <execinfo.h> (glibc / bionic).
 * Android : same as Linux; _Unwind_Backtrace used as fallback.
 *
 * @param outStack  receives a newly malloc'd TRawCallStack; set to NULL on failure.
 * @param maxDepth  maximum frames to capture (capped at 62 on Windows).
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_CaptureRawCallstack( struct TRawCallStack** outStack, UInt32 maxDepth );

/**
 * Free a TRawCallStack previously allocated by MEMMAN_CaptureRawCallstack().
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_FreeRawCallstack( struct TRawCallStack* stack );

/**
 * Write a symbolicated (human-readable) form of the raw callstack to dest.
 *
 * Symbolication is deferred to report time (not per-alloc) to avoid the
 * performance cost of DbgHelp.dll disk I/O on every allocation.
 *
 * Windows: SymFromAddr + SymGetLineFromAddr64 (DbgHelp.dll; single-threaded,
 *           serialised behind an internal mutex).
 * Linux  : backtrace_symbols() / dladdr() for demangling.
 *
 * @param stack  the raw callstack to symbolicate (may be NULL; prints nothing).
 * @param dest   output FILE* (e.g. log file handle).
 * @param indent prefix string printed before each frame line (may be NULL).
 */
GUCEF_MLF_PUBLIC_C void
MEMMAN_SymbolicateRawCallstack( struct TRawCallStack* stack ,
                                FILE*                 dest  ,
                                const char*           indent );

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
   }
#endif /* __cplusplus */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus
}; /* namespace MLF */
}; /* namespace GUCEF */
#endif /* __cplusplus ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CALLSTACK_H ? */
