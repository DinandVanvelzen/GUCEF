/*
 *  MemoryLeakFinder: GUCEF module providing memory diagnostics
 *  Copyright (C) 2002 - 2024.  Dinand Vanvelzen
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

#ifndef GUCEF_MLF_SMEMORYTRACKERCONFIG_H
#define GUCEF_MLF_SMEMORYTRACKERCONFIG_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      SANITIZER DETECTION MACROS                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 * Detect when the address sanitizer (ASan) is active.
 * When ASan is active: disable sentinel writes/checks and guard pages
 * (they conflict with ASan shadow memory).  Keep OLE tracking, callstacks,
 * lock tracing, and statistics.
 */
#if defined( __SANITIZE_ADDRESS__ )
  #define GUCEF_MLF_ASAN_ACTIVE 1
#elif defined( __has_feature )
  #if __has_feature( address_sanitizer )
    #define GUCEF_MLF_ASAN_ACTIVE 1
  #endif
#endif

/*
 * Detect when memory sanitizer (MSan) is active.
 * When MSan is active: disable body fill patterns (they interfere with MSan
 * shadow tracking).  Keep allocation tracking, callstacks, OLE, lock tracing.
 */
#if defined( __SANITIZE_MEMORY__ )
  #define GUCEF_MLF_MSAN_ACTIVE 1
#elif defined( __has_feature )
  #if __has_feature( memory_sanitizer )
    #define GUCEF_MLF_MSAN_ACTIVE 1
  #endif
#endif

/*
 * Detect when thread sanitizer (TSan) is active.
 * When TSan is active: disable our lock instrumentation (avoid false positives
 * from TSan observing our internal lock operations).  Keep memory tracking,
 * callstacks, OLE.
 */
#if defined( __SANITIZE_THREAD__ )
  #define GUCEF_MLF_TSAN_ACTIVE 1
#elif defined( __has_feature )
  #if __has_feature( thread_sanitizer )
    #define GUCEF_MLF_TSAN_ACTIVE 1
  #endif
#endif

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Configures the action taken when an alloc/dealloc type mismatch is detected
 * (e.g. allocated with new, freed with free).
 *
 * MISMATCH_LOG   — log the mismatch and continue (default; current behaviour)
 * MISMATCH_BREAK — log the mismatch and trigger a debugger break (debug sessions)
 * MISMATCH_ABORT — log the mismatch and call abort() (CI / gate builds)
 */
enum EMismatchResponse
{
    MISMATCH_LOG   = 0,  /* log only (default) */
    MISMATCH_BREAK = 1,  /* log + debugger break */
    MISMATCH_ABORT = 2   /* log + abort() */
};

/**
 * POD configuration struct replacing all scattered MEMMAN_Set* global state.
 * Call SMemoryTrackerConfig_SetDefaults() to initialize before first use.
 */
struct SMemoryTrackerConfig
{
    /* --- Core tracking --- */
    UInt32      paddingSize;               /* bytes of sentinel padding on each side; default=4 */
    bool        logAlways;                 /* log every alloc/dealloc to the log file; default=true */
    bool        cleanLogFileOnFirstRun;    /* delete the log file at first write; default=true */
    bool        exhaustiveTesting;         /* run full O(n) integrity scan on every alloc; default=false */
    Int32       breakOnAllocationCount;    /* trigger debugger break on N-th alloc; -1=disabled */
    const char* logFilePath;               /* log file path; default="GUCEF_memlog.txt" */
    UInt32      deallocRingCapacity;       /* max recently-deallocated nodes to retain; default=10000000 */

    /* --- Callstack --- */
    bool        enableCallstackCapture;    /* capture GUCEF_TRACE callstacks at alloc/dealloc time; default=true */
    bool        enableRawCallstackCapture; /* capture OS-level callstacks (RtlCaptureStackBackTrace/backtrace); default=true */
    UInt32      maxRawCallstackDepth;      /* max frames for OS-level capture; default=32; Win limit=62 */

    /* --- Type mismatch enforcement (Gap 7) --- */
    EMismatchResponse deallocMismatchResponse; /* action on alloc/dealloc type mismatch; default=MISMATCH_LOG */

    /* --- Heap profiling (Gap 6) --- */
    bool        enableCallsiteProfiling;   /* aggregate per-callsite stats (slight lock overhead); default=false */

    /* --- Guard pages (Gap 2) --- */
    bool        useGuardPages;             /* place a PROT_NONE page after each allocation; default=false */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifdef __cplusplus

inline void
SMemoryTrackerConfig_SetDefaults( SMemoryTrackerConfig& cfg )
{
    cfg.paddingSize                = 4;
    cfg.logAlways                  = true;
    cfg.cleanLogFileOnFirstRun     = true;
    cfg.exhaustiveTesting          = false;
    cfg.breakOnAllocationCount     = -1;
    cfg.logFilePath                = "GUCEF_memlog.txt";
    cfg.deallocRingCapacity        = 10000000;
    cfg.enableCallstackCapture     = true;
    cfg.enableRawCallstackCapture  = true;
    cfg.maxRawCallstackDepth       = 32;
    cfg.deallocMismatchResponse    = MISMATCH_LOG;
    cfg.enableCallsiteProfiling    = false;
    cfg.useGuardPages              = false;
}

#endif /* __cplusplus ? */

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

#endif /* GUCEF_MLF_SMEMORYTRACKERCONFIG_H ? */
