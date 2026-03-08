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

#ifndef GUCEF_MLF_CALLOCATIONRECORD_H
#define GUCEF_MLF_CALLOCATIONRECORD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_MLF_ETYPES_H
#include "gucefMLF_ETypes.h"
#define GUCEF_MLF_ETYPES_H
#endif /* GUCEF_MLF_ETYPES_H ? */

#ifndef GUCEF_CALLSTACK_H
#include "gucefMLF_callstack.h"
#define GUCEF_CALLSTACK_H
#endif /* GUCEF_CALLSTACK_H ? */

#ifndef GUCEF_MLF_MEMORYMANAGER_H
#include "gucefMLF_MemoryManager.h"
#define GUCEF_MLF_MEMORYMANAGER_H
#endif /* GUCEF_MLF_MEMORYMANAGER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MLF {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* Sentinel values — must not change (binary format compatibility) */
static const long ALLOC_PADDING_SENTINEL = (long) 0xDEADC0DE;

/**
 * Body fill pattern for newly-allocated memory.
 * 0xBAADC0DE is recognizable in memory dumps and signals "uninitialized".
 */
static const long ALLOC_BODY_SENTINEL    = (long) 0xBAADC0DE;

/**
 * Body fill pattern written over freed memory in the dealloc ring.
 * This pattern is intentionally recognizable so that use-after-free reads
 * that access this data produce a recognizable crash value.
 * The pattern is FEEDFACE_DEADBEEF (64-bit) stored in repeating long words.
 */
#if defined(_WIN64) || defined(__LP64__) || defined(__x86_64__)
static const long ALLOC_FREED_BODY_SENTINEL = (long) 0xFEEDFACEDEADBEEFLL;
#else
static const long ALLOC_FREED_BODY_SENTINEL = (long) 0xDEADBEEFL;
#endif

/* Break option flags stored in breakOptions field */
static const UInt8 BREAK_OPTION_ON_DEALLOC = 0x01;
static const UInt8 BREAK_OPTION_ON_REALLOC = 0x02;

/* Allocation type identifiers — must match the MM_* constants in MemoryManager.h */
typedef char EAllocationType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * OS-level raw callstack — instruction-pointer addresses captured by
 * RtlCaptureStackBackTrace (Windows) or backtrace() (Linux/Android).
 *
 * Raw addresses are stored (not symbolicated) because symbolication is slow
 * (DbgHelp.dll lock, PDB disk I/O) and should be deferred to report time.
 * Freed by FreeRawCallstack() in callstack.cpp.
 */
struct TRawCallStack
{
    void**  frames;       /* raw instruction-pointer addresses (malloc'd) */
    UInt32  frameCount;   /* number of captured frames */
    UInt32  threadId;     /* ID of the thread that captured this stack */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * Unified allocation record replacing the old MemoryNode + SubMemoryNode pair.
 *
 * Key design decisions:
 *  - sourceFile is a malloc'd char* (no 64-byte cap like the old char[64])
 *  - Placement-new children hang off the parent as a singly-linked list,
 *    eliminating the second hash table (m_hashTableForSubs)
 *  - Allocate()/Release() use raw ::malloc/::free ONLY — never operator new/delete
 *    to avoid re-entering the memory tracking system
 *  - hashNext/hashPrev serve triple duty: hash chain / dealloc ring / free-pool
 */
class GUCEF_HIDDEN CAllocationRecord
{
    public:

    void*              actualAddress;       /* malloc'd block start (includes padding) */
    void*              reportedAddress;     /* pointer handed to the user (past front padding) */
    size_t             actualSize;          /* total malloc'd size incl. padding on both sides */
    size_t             reportedSize;        /* size the caller requested */
    char*              sourceFile;          /* malloc'd copy of __FILE__ — full length, no cap */
    UInt32             sourceLine;
    UInt16             paddingSize;         /* # of long-words of padding on each side */
    EAllocationType    allocationType;      /* MM_NEW, MM_MALLOC, etc. */
    UInt8              breakOptions;        /* BREAK_OPTION_ON_DEALLOC | BREAK_OPTION_ON_REALLOC */
    UInt8              suppressMismatchCheck; /* non-zero: skip alloc/dealloc type mismatch check */
    size_t             guardPageRegionSize; /* 0 = normal malloc; >0 = total VirtualAlloc/mmap region size (guard page backed) */
    long               predefinedBody;      /* body fill value used (for usage stats) */
    bool               hadPlacementChildren;
    UInt64             allocationTimestampUs; /* microseconds since tracker start (QueryPerformanceCounter / clock_gettime) */
    TCallStack*        allocCallstack;      /* GUCEF_TRACE logical callstack at allocation */
    TCallStack*        deallocCallstack;    /* GUCEF_TRACE logical callstack at deallocation */
    TRawCallStack*     allocRawCallstack;   /* OS-level raw frames captured at allocation */
    TRawCallStack*     deallocRawCallstack; /* OS-level raw frames captured at deallocation */

    /* Placement-new children: singly-linked list hanging off the parent */
    CAllocationRecord* placementChildren;   /* first child (null if none) */
    CAllocationRecord* placementSiblingNext;/* next sibling in parent's child list */
    CAllocationRecord* parentRecord;        /* null for a top-level allocation */

    /* Intrusive link pointers — reused for:
     *   (a) hash table chain (active registry)
     *   (b) dealloc ring (recently freed nodes)
     *   (c) free-pool singly-linked list
     */
    CAllocationRecord* hashNext;
    CAllocationRecord* hashPrev;

    /* ------------------------------------------------------------------ */
    /* Lifecycle — must ONLY use raw malloc/free internally                */
    /* ------------------------------------------------------------------ */
    static CAllocationRecord* Allocate( void );
    static void Release( CAllocationRecord* record );

    /* ------------------------------------------------------------------ */
    /* Sentinel / validation                                               */
    /* ------------------------------------------------------------------ */

    /** Fill padding sentinels and body fill. Call after the backing malloc. */
    void InitializeSentinels( long bodyFill );

    /**
     * Like InitializeSentinels but preserves already-written content up to
     * preservedSize bytes (used for realloc expansions).
     */
    void InitializeReallocSentinels( long bodyFill, size_t preservedSize );

    /**
     * Check all padding longs for corruption.
     * @param outViolationCount receives number of corrupted padding words
     * @return true if no violations found
     */
    bool ValidateSentinels( UInt32& outViolationCount ) const;

    /** Returns true when the dealloc type is not legal for this alloc type */
    bool IsDeallocationTypeMismatch( EAllocationType deallocType ) const;

    /** Count body bytes actually written by caller (for usage stats) */
    size_t CountUsedBodyBytes( void ) const;

    /** Set sourceFile from a raw __FILE__ string (copies; strips directory prefix) */
    void SetSourceFile( const char* file );

    private:

    CAllocationRecord( void );
    ~CAllocationRecord( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MLF */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_MLF_CALLOCATIONRECORD_H ? */
