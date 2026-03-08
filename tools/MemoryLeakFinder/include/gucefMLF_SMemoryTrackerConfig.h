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
 * POD configuration struct replacing all scattered MEMMAN_Set* global state.
 * Call SMemoryTrackerConfig_SetDefaults() to initialize before first use.
 */
struct SMemoryTrackerConfig
{
    UInt32      paddingSize;               /* bytes of sentinel padding on each side; default=4 */
    bool        logAlways;                 /* log every alloc/dealloc to the log file; default=true */
    bool        cleanLogFileOnFirstRun;    /* delete the log file at first write; default=true */
    bool        exhaustiveTesting;         /* run full O(n) integrity scan on every alloc; default=false */
    Int32       breakOnAllocationCount;    /* trigger debugger break on N-th alloc; -1=disabled */
    const char* logFilePath;               /* log file path; default="GUCEF_memlog.txt" */
    UInt32      deallocRingCapacity;       /* max recently-deallocated nodes to retain; default=10000000 */
    bool        enableCallstackCapture;    /* capture callstacks at alloc/dealloc time; default=true */
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
    cfg.paddingSize            = 4;
    cfg.logAlways              = true;
    cfg.cleanLogFileOnFirstRun = true;
    cfg.exhaustiveTesting      = false;
    cfg.breakOnAllocationCount = -1;
    cfg.logFilePath            = "GUCEF_memlog.txt";
    cfg.deallocRingCapacity    = 10000000;
    cfg.enableCallstackCapture = true;
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
