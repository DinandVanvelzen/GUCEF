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

#ifndef GUCEF_DRGUP_CCALLSITESTATS_H
#define GUCEF_DRGUP_CCALLSITESTATS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRGUP_ETYPES_H
#include "gucefDRGUP_ETypes.h"
#define GUCEF_DRGUP_ETYPES_H
#endif /* GUCEF_DRGUP_ETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRGUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Per-callsite (file + line) aggregated allocation statistics.
 *
 * Updated on every alloc/dealloc when `enableCallsiteProfiling` is true.
 * Maintained in CMemoryTracker::m_callsiteMap keyed by (file_ptr, line).
 *
 * file pointer is a static string constant from __FILE__ — the pointer
 * value itself is used as a key (pointer identity, not string equality)
 * which is valid because __FILE__ strings have static storage duration.
 */
struct CCallsiteStats
{
    const char* sourceFile;          /* __FILE__ pointer (static storage, not freed) */
    UInt32      sourceLine;          /* __LINE__ value */
    UInt64      allocCount;          /* lifetime total allocations from this site */
    UInt64      freeCount;           /* lifetime total frees matched to this site */
    UInt64      totalBytesAllocated; /* lifetime total bytes allocated from this site */
    UInt64      currentLiveBytes;    /* bytes currently live (allocated - freed) */
    UInt64      peakLiveBytes;       /* historical peak of currentLiveBytes */
    UInt64      peakLiveCount;       /* historical peak of (allocCount - freeCount) */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRGUP */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_DRGUP_CCALLSITESTATS_H ? */
