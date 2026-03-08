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
static const long ALLOC_BODY_SENTINEL    = (long) 0xBAADC0DE;

/* Break option flags stored in breakOptions field */
static const UInt8 BREAK_OPTION_ON_DEALLOC = 0x01;
static const UInt8 BREAK_OPTION_ON_REALLOC = 0x02;

/* Allocation type identifiers — must match the MM_* constants in MemoryManager.h */
typedef char EAllocationType;

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
    long               predefinedBody;      /* body fill value used (for usage stats) */
    bool               hadPlacementChildren;
    TCallStack*        allocCallstack;
    TCallStack*        deallocCallstack;

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
