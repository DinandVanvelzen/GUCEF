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

/*
 *  IMPORTANT: This file must NEVER use operator new/delete or tracked malloc/free.
 *  All memory used internally must go through raw ::malloc / ::free to prevent
 *  recursive re-entry into the memory tracking system.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Turn off all memory tracking macros for this file */
#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#undef GUCEF_USE_CALLSTACK_TRACING
#undef GUCEF_USE_CALLSTACK_PLATFORM_TRACING

#ifndef GUCEF_DRGUP_CALLOCATIONRECORD_H
#include "gucefDRGUP_CAllocationRecord.h"
#define GUCEF_DRGUP_CALLOCATIONRECORD_H
#endif /* GUCEF_DRGUP_CALLOCATIONRECORD_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRGUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAllocationRecord::CAllocationRecord( void )
    : actualAddress( GUCEF_NULL )
    , reportedAddress( GUCEF_NULL )
    , actualSize( 0 )
    , reportedSize( 0 )
    , sourceFile( GUCEF_NULL )
    , sourceLine( 0 )
    , paddingSize( 0 )
    , allocationType( 0 /* MM_UNKNOWN */ )
    , breakOptions( 0 )
    , suppressMismatchCheck( 0 )
    , guardPageRegionSize( 0 )
    , predefinedBody( 0 )
    , hadPlacementChildren( false )
    , allocationTimestampUs( 0 )
    , allocCallstack( GUCEF_NULL )
    , deallocCallstack( GUCEF_NULL )
    , allocRawCallstack( GUCEF_NULL )
    , deallocRawCallstack( GUCEF_NULL )
    , placementChildren( GUCEF_NULL )
    , placementSiblingNext( GUCEF_NULL )
    , parentRecord( GUCEF_NULL )
    , hashNext( GUCEF_NULL )
    , hashPrev( GUCEF_NULL )
{
}

/*-------------------------------------------------------------------------*/

CAllocationRecord::~CAllocationRecord( void )
{
    /* caller is responsible for freeing sourceFile / callstacks before calling Release() */
}

/*-------------------------------------------------------------------------*/

/* static */
CAllocationRecord*
CAllocationRecord::Allocate( void )
{
    /* Use raw ::malloc — NEVER operator new, to avoid re-entrancy */
    void* mem = ::malloc( sizeof( CAllocationRecord ) );
    if ( GUCEF_NULL == mem )
        return GUCEF_NULL;

    /* Placement-construct via memset zero + constructor-like init */
    CAllocationRecord* rec = static_cast< CAllocationRecord* >( mem );
    ::memset( rec, 0, sizeof( CAllocationRecord ) );
    return rec;
}

/*-------------------------------------------------------------------------*/

/* static */
void
CAllocationRecord::Release( CAllocationRecord* record )
{
    if ( GUCEF_NULL == record )
        return;

    /* Free dynamically allocated fields */
    if ( GUCEF_NULL != record->sourceFile )
    {
        ::free( record->sourceFile );
        record->sourceFile = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->allocCallstack )
    {
        DRGUP_FreeCallstackCopy( record->allocCallstack );
        record->allocCallstack = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->deallocCallstack )
    {
        DRGUP_FreeCallstackCopy( record->deallocCallstack );
        record->deallocCallstack = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->allocRawCallstack )
    {
        if ( GUCEF_NULL != record->allocRawCallstack->frames )
            ::free( record->allocRawCallstack->frames );
        ::free( record->allocRawCallstack );
        record->allocRawCallstack = GUCEF_NULL;
    }
    if ( GUCEF_NULL != record->deallocRawCallstack )
    {
        if ( GUCEF_NULL != record->deallocRawCallstack->frames )
            ::free( record->deallocRawCallstack->frames );
        ::free( record->deallocRawCallstack );
        record->deallocRawCallstack = GUCEF_NULL;
    }

    ::free( record );
}

/*-------------------------------------------------------------------------*/

void
CAllocationRecord::SetSourceFile( const char* file )
{
    /* Strip directory prefix (keep only filename) */
    const char* basename = file;
    if ( GUCEF_NULL != file )
    {
        const char* backslash = ::strrchr( file, '\\' );
        const char* slash     = ::strrchr( file, '/' );
        const char* sep = backslash > slash ? backslash : slash;
        if ( GUCEF_NULL != sep )
            basename = sep + 1;
    }

    ::free( sourceFile );
    sourceFile = GUCEF_NULL;

    if ( GUCEF_NULL == basename )
        return;

    size_t len = ::strlen( basename );
    sourceFile = (char*) ::malloc( len + 1 );
    if ( GUCEF_NULL != sourceFile )
    {
        ::memcpy( sourceFile, basename, len + 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CAllocationRecord::InitializeSentinels( long bodyFill )
{
    if ( GUCEF_NULL == actualAddress || 0 == paddingSize )
        return;

    /* Write padding sentinels at the front */
    long* beginning = (long*) actualAddress;
    /* Write padding sentinels at the back */
    long* ending = (long*) ( (char*) actualAddress + actualSize - (size_t)paddingSize * sizeof(long) );

    for ( UInt16 i = 0; i < paddingSize; ++i )
    {
        beginning[ i ] = ALLOC_PADDING_SENTINEL;
        ending[ i ]    = ALLOC_PADDING_SENTINEL;
    }

    /* Fill the body */
    long*        bodyLongs = (long*) reportedAddress;
    size_t       longCount = reportedSize / sizeof(long);
    for ( size_t i = 0; i < longCount; ++i )
        bodyLongs[ i ] = bodyFill;

    /* Fill any remaining bytes */
    char* remainder = (char*) ( bodyLongs + longCount );
    size_t remBytes = reportedSize - longCount * sizeof(long);
    for ( size_t i = 0; i < remBytes; ++i )
        remainder[ i ] = (char) bodyFill;

    predefinedBody = bodyFill;
}

/*-------------------------------------------------------------------------*/

void
CAllocationRecord::InitializeReallocSentinels( long bodyFill, size_t preservedSize )
{
    if ( GUCEF_NULL == actualAddress || 0 == paddingSize )
        return;

    if ( 0 == preservedSize )
    {
        InitializeSentinels( bodyFill );
        return;
    }

    if ( preservedSize == reportedSize )
    {
        /* Same size: just rewrite padding sentinels, content is unchanged */
        long* beginning = (long*) actualAddress;
        long* ending    = (long*) ( (char*) actualAddress + actualSize - (size_t)paddingSize * sizeof(long) );
        for ( UInt16 i = 0; i < paddingSize; ++i )
        {
            beginning[ i ] = ALLOC_PADDING_SENTINEL;
            ending[ i ]    = ALLOC_PADDING_SENTINEL;
        }
        predefinedBody = bodyFill;
        return;
    }

    /* Write back padding sentinels (both ends) */
    long* beginning = (long*) actualAddress;
    long* ending    = (long*) ( (char*) actualAddress + actualSize - (size_t)paddingSize * sizeof(long) );
    for ( UInt16 i = 0; i < paddingSize; ++i )
    {
        beginning[ i ] = ALLOC_PADDING_SENTINEL;
        ending[ i ]    = ALLOC_PADDING_SENTINEL;
    }

    if ( preservedSize < reportedSize )
    {
        /* Block was enlarged — fill the new tail bytes with bodyFill */
        char* newTailStart = (char*) reportedAddress + preservedSize;
        size_t newTailBytes = reportedSize - preservedSize;

        size_t longCount = newTailBytes / sizeof(long);
        long*  lptr      = (long*) newTailStart;
        for ( size_t i = 0; i < longCount; ++i )
            lptr[ i ] = bodyFill;
        char* rem = (char*)( lptr + longCount );
        size_t remBytes = newTailBytes - longCount * sizeof(long);
        for ( size_t i = 0; i < remBytes; ++i )
            rem[ i ] = (char) bodyFill;
    }
    /* else block was shrunk — the padding was already written above, content is intact */

    predefinedBody = bodyFill;
}

/*-------------------------------------------------------------------------*/

bool
CAllocationRecord::ValidateSentinels( UInt32& outViolationCount ) const
{
    outViolationCount = 0;
    if ( GUCEF_NULL == actualAddress || 0 == paddingSize )
        return true;

    const long* beginning = (const long*) actualAddress;
    const long* ending    = (const long*) ( (const char*) actualAddress + actualSize - (size_t)paddingSize * sizeof(long) );

    for ( UInt16 i = 0; i < paddingSize; ++i )
    {
        if ( beginning[ i ] != ALLOC_PADDING_SENTINEL )
            ++outViolationCount;
        if ( ending[ i ] != ALLOC_PADDING_SENTINEL )
            ++outViolationCount;
    }

    return outViolationCount == 0;
}

/*-------------------------------------------------------------------------*/

bool
CAllocationRecord::IsDeallocationTypeMismatch( EAllocationType deallocType ) const
{
    /* Legal alloc->dealloc pairs */
    if ( deallocType == 7 /* MM_DELETE       */ && ( allocationType == 1 /* MM_NEW           */ ||
                                                     allocationType == 2 /* MM_PLACEMENT_NEW */ ) )
        return false;
    if ( deallocType == 8 /* MM_DELETE_ARRAY */ && allocationType == 3 /* MM_NEW_ARRAY       */ )
        return false;
    if ( deallocType == 9 /* MM_FREE         */ && ( allocationType == 4 /* MM_MALLOC  */ ||
                                                     allocationType == 5 /* MM_CALLOC  */ ||
                                                     allocationType == 6 /* MM_REALLOC */ ) )
        return false;
    if ( deallocType == 11 /* MM_OLE_FREE    */ && allocationType == 10 /* MM_OLE_ALLOC      */ )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

size_t
CAllocationRecord::CountUsedBodyBytes( void ) const
{
    if ( GUCEF_NULL == reportedAddress || 0 == reportedSize )
        return 0;

    size_t usedBytes = 0;

    const long* lptr     = (const long*) reportedAddress;
    size_t      longCount = reportedSize / sizeof(long);
    for ( size_t i = 0; i < longCount; ++i )
    {
        if ( lptr[ i ] != predefinedBody )
            usedBytes += sizeof(long);
    }

    const char* cptr   = (const char*)( lptr + longCount );
    size_t      remBytes = reportedSize - longCount * sizeof(long);
    for ( size_t i = 0; i < remBytes; ++i )
    {
        if ( cptr[ i ] != (char) predefinedBody )
            ++usedBytes;
    }

    return usedBytes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRGUP */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
