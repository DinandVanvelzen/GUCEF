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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <map>
#include <vector>
#include <algorithm>

#ifndef GUCEF_DYNNEWOFF_H
#include "gucef_dynnewoff.h"
#define GUCEF_DYNNEWOFF_H
#endif /* GUCEF_DYNNEWOFF_H ? */

#undef GUCEF_USE_CALLSTACK_TRACING
#undef GUCEF_USE_CALLSTACK_PLATFORM_TRACING

#include "gucefDRGUP_CReporter.h"
#include "gucefDRGUP_CMemoryTracker.h"
#include "gucefDRGUP_CLockTracer.h"
#include "gucefDRGUP_callstack.h"

#ifndef GUCEF_MT_CSCOPERWLOCK_H
#include "gucefMT_CScopeRwLock.h"
#define GUCEF_MT_CSCOPERWLOCK_H
#endif /* GUCEF_MT_CSCOPERWLOCK_H ? */

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <io.h>       /* _unlink */
  #define GUCEF_UNLINK _unlink
#else
  #include <unistd.h>
  #define GUCEF_UNLINK unlink
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRGUP {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const s_allocationTypes[] =
{
    "Unknown", "new", "new(addr)", "new[]",
    "malloc", "calloc", "realloc",
    "delete", "delete[]", "free",
    "OLESysAlloc", "OLESysFree"
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CReporter::CReporter( void )
{
}

/*-------------------------------------------------------------------------*/

CReporter::~CReporter( void )
{
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Formatting helpers                                                  //
//                                                                         //
//-------------------------------------------------------------------------*/

/* static */
char*
CReporter::InsertCommas( size_t value )
{
    static char str[ 30 ];
    memset( str, 0, sizeof(str) );
    sprintf( str, "%zu", value );
    if ( strlen( str ) > 3 )
    {
        memmove( &str[ strlen(str)-3 ], &str[ strlen(str)-4 ], 4 );
        str[ strlen(str) - 4 ] = ',';
    }
    if ( strlen( str ) > 7 )
    {
        memmove( &str[ strlen(str)-7 ], &str[ strlen(str)-8 ], 8 );
        str[ strlen(str) - 8 ] = ',';
    }
    if ( strlen( str ) > 11 )
    {
        memmove( &str[ strlen(str)-11 ], &str[ strlen(str)-12 ], 12 );
        str[ strlen(str) - 12 ] = ',';
    }
    return str;
}

/*-------------------------------------------------------------------------*/

/* static */
char*
CReporter::MemorySizeString( size_t size, bool lengthenString )
{
    static char str[ 90 ];
    if ( lengthenString )
    {
        if ( size > (1024*1024) )     sprintf( str, "%10s (%7.2fM)", InsertCommas(size), size / (1024.0 * 1024.0) );
        else if ( size > 1024 )       sprintf( str, "%10s (%7.2fK)", InsertCommas(size), size / 1024.0 );
        else                          sprintf( str, "%10s bytes     ", InsertCommas(size) );
    }
    else
    {
        if ( size > (1024*1024) )     sprintf( str, "%s (%7.2fM)", InsertCommas(size), size / (1024.0 * 1024.0) );
        else if ( size > 1024 )       sprintf( str, "%s (%7.2fK)", InsertCommas(size), size / 1024.0 );
        else                          sprintf( str, "%s bytes     ", InsertCommas(size) );
    }
    return str;
}

/*-------------------------------------------------------------------------*/

/* static */
char*
CReporter::FormatOwnerString( const char* file, int line )
{
    static char str[ 90 ];
    memset( str, 0, sizeof(str) );
    if ( GUCEF_NULL != file )
        sprintf( str, "%s(%05d)", file, line );
    else
        sprintf( str, "?(%05d)", line );
    return str;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Log file management                                                 //
//                                                                         //
//-------------------------------------------------------------------------*/

FILE*
CReporter::OpenLogFile( bool* /*wasCleanedFlag*/ )
{
    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return GUCEF_NULL;

    const char* logPath = tracker->GetConfig().logFilePath;
    if ( GUCEF_NULL == logPath )
        logPath = "GUCEF_memlog.txt";

    return fopen( logPath, "ab" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Record dumping                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CReporter::DumpRecord( FILE* fp, CAllocationRecord* record )
{
    if ( GUCEF_NULL == fp || GUCEF_NULL == record )
        return;

    fprintf( fp, "Total Memory Size : %s\r\n", MemorySizeString( record->reportedSize, false ) );
    fprintf( fp, "Source File       : %s\r\n", GUCEF_NULL != record->sourceFile ? record->sourceFile : "?" );
    fprintf( fp, "Source Line       : %d\r\n", record->sourceLine );
    fprintf( fp, "Allocation Type   : %s\r\n", s_allocationTypes[ (unsigned char) record->allocationType ] );

    if ( GUCEF_NULL != record->parentRecord )
    {
        fprintf( fp, " --- \r\n" );
        fprintf( fp, "Parent Record:\r\n" );
        DumpRecord( fp, record->parentRecord );
        fprintf( fp, " --- \r\n" );
    }

    if ( GUCEF_NULL != record->allocCallstack && record->allocCallstack->items > 0 )
    {
        fprintf( fp, "Allocation Call Stack (GUCEF logical frames):\r\n" );
        for ( UInt32 s = 0; s < record->allocCallstack->items; ++s )
            fprintf( fp, "  %s:%d\r\n", record->allocCallstack->file[s], record->allocCallstack->linenr[s] );
    }

    if ( GUCEF_NULL != record->allocRawCallstack && record->allocRawCallstack->frameCount > 0 )
    {
        fprintf( fp, "Allocation Call Stack (OS actual frames):\r\n" );
        DRGUP_SymbolicateRawCallstack( record->allocRawCallstack, fp, "  " );
    }

    if ( GUCEF_NULL != record->deallocCallstack && record->deallocCallstack->items > 0 )
    {
        fprintf( fp, "Deallocation Call Stack (GUCEF logical frames):\r\n" );
        for ( UInt32 s = 0; s < record->deallocCallstack->items; ++s )
            fprintf( fp, "  %s:%d\r\n", record->deallocCallstack->file[s], record->deallocCallstack->linenr[s] );
    }

    if ( GUCEF_NULL != record->deallocRawCallstack && record->deallocRawCallstack->frameCount > 0 )
    {
        fprintf( fp, "Deallocation Call Stack (OS actual frames):\r\n" );
        DRGUP_SymbolicateRawCallstack( record->deallocRawCallstack, fp, "  " );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Normal reports                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CReporter::DumpLogReport( void )
{
    FILE* fp = OpenLogFile( GUCEF_NULL );
    if ( GUCEF_NULL == fp )
        return;

    DumpLogReportToFile( fp );

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL != tracker && tracker->GetNumAllocations() != 0 )
        DumpMemoryAllocationsToFile( fp );

    fclose( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpLogReportToFile( FILE* fp )
{
    if ( GUCEF_NULL == fp )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );

    time_t t = time( GUCEF_NULL );
    struct tm* timeInfo = localtime( &t );

    UInt32 totalAllocated    = tracker->GetTotalMemoryAllocated();
    UInt32 totalUsed         = tracker->GetTotalMemoryUsed();
    UInt32 totalAllocations  = (UInt32) tracker->GetTotalMemoryAllocations();
    UInt32 peakAllocations   = tracker->GetPeakTotalNumAllocations();
    UInt32 peakAllocation    = tracker->GetPeakMemoryAllocation();
    UInt32 overhead          = tracker->GetTotalOverheadMemoryCost();
    UInt32 peakOverhead      = tracker->GetPeakOverheadMemoryCost();
    UInt32 numAllocations    = tracker->GetNumAllocations();
    UInt32 numBoundsViol     = tracker->GetNumBoundsViolations();

    /* Compute unallocated memory */
    UInt32 unallocated = 0;
    const CMemoryTracker::TRegistry& reg = tracker->GetRegistry();
    CMemoryTracker::TRegistry::const_iterator i = reg.begin();
    while ( i != reg.end() )
    {
        CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL == rec->parentRecord )
            unallocated += (UInt32) rec->reportedSize;
        ++i;
    }

    UInt32 totalDivider = totalAllocated != 0 ? totalAllocated : 1;

    /* Header */
    fprintf( fp, "\r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "*********           Memory report for: %02d/%02d/%04d %02d:%02d:%02d            ********* \r\n",
             timeInfo->tm_mon + 1, timeInfo->tm_mday, timeInfo->tm_year + 1900,
             timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "\r\n" );

    /* Total usage */
    fprintf( fp, "                     T O T A L  M E M O R Y  U S A G E                          \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "           Total Number of Dynamic Allocations: %10s\r\n", InsertCommas( totalAllocations ) );
    fprintf( fp, "      Reported Memory usage to the Application: %s\r\n", MemorySizeString( totalAllocated ) );
    fprintf( fp, "          Actual Memory use by the Application: %s\r\n", MemorySizeString( overhead + totalAllocated ) );
    fprintf( fp, "                      Memory Tracking Overhead: %s\r\n", MemorySizeString( overhead ) );
    fprintf( fp, "\r\n" );

    /* Peak usage */
    fprintf( fp, "                      P E A K  M E M O R Y  U S A G E                           \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "            Peak Number of Dynamic Allocations: %10s\r\n", InsertCommas( peakAllocations ) );
    fprintf( fp, " Peak Reported Memory usage to the application: %s\r\n", MemorySizeString( peakAllocation ) );
    fprintf( fp, "     Peak Actual Memory use by the Application: %s\r\n", MemorySizeString( peakOverhead + peakAllocation ) );
    fprintf( fp, "                 Peak Memory Tracking Overhead: %s\r\n", MemorySizeString( peakOverhead ) );
    fprintf( fp, "\r\n" );

    /* Unused memory */
    fprintf( fp, "                          U N U S E D  M E M O R Y                              \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "  Percentage of Allocated Memory Actually Used: %10.2f %%\r\n",
             (float)(1 - (totalAllocated - totalUsed) / (float) totalDivider) * 100.0f );
    fprintf( fp, "       Percentage of Allocated Memory Not Used: %10.2f %%\r\n",
             (float)(totalAllocated - totalUsed) / (float) totalDivider * 100.0f );
    fprintf( fp, "        Memory Allocated but not Actually Used: %s\r\n", MemorySizeString( totalAllocated - totalUsed ) );
    fprintf( fp, "\r\n" );

    /* Bounds violations */
    UInt32 numMismatches = tracker->GetNumMismatchedDeallocs();
    fprintf( fp, "                      B O U N D S  V I O L A T I O N S                          \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "            Number of Memory Bounds Violations: %10s\r\n", InsertCommas( numBoundsViol ) );
    fprintf( fp, "      Number of Alloc/Dealloc Type Mismatches: %10s\r\n", InsertCommas( numMismatches ) );
    fprintf( fp, "\r\n" );

    /* Memory leaks */
    fprintf( fp, "                           M E M O R Y  L E A K S                               \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "                        Number of Memory Leaks: %10s\r\n", InsertCommas( numAllocations ) );
    fprintf( fp, "                 Amount of Memory Un-Allocated: %s\r\n", MemorySizeString( unallocated ) );
    fprintf( fp, "   Percentage of Allocated Memory Un-Allocated: %10.2f %%\r\n",
             (float)(1 - (totalAllocated - unallocated) / (float) totalDivider) * 100.0f );
    fprintf( fp, "\r\n" );

    /* Lock tracer summary */
    {
        CLockTracer* lockTracer = CLockTracer::Instance();
        if ( GUCEF_NULL != lockTracer )
        {
            CLockTracer::SLockAggregateStats ls;
            lockTracer->GetAggregateStats( ls );

            fprintf( fp, "                  L O C K  T R A C E R  S U M M A R Y                       \r\n" );
            fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
#ifdef GUCEF_MSWIN_BUILD
            fprintf( fp, "                    Total Lock Instances Tracked: %10I64u\r\n", ls.totalLockInstances );
            fprintf( fp, "              Currently Locked (at report time): %10I64u\r\n",  ls.currentlyLockedCount );
            fprintf( fp, "              Total Lock Abandonments (not released): %6I64u\r\n", ls.totalAbandonments );
            fprintf( fp, "        Total Surplus Lock Releases (over-release): %7I64u\r\n", ls.totalSurplusReleases );
#else
            fprintf( fp, "                    Total Lock Instances Tracked: %10llu\r\n", (unsigned long long)ls.totalLockInstances );
            fprintf( fp, "              Currently Locked (at report time): %10llu\r\n",  (unsigned long long)ls.currentlyLockedCount );
            fprintf( fp, "              Total Lock Abandonments (not released): %6llu\r\n", (unsigned long long)ls.totalAbandonments );
            fprintf( fp, "        Total Surplus Lock Releases (over-release): %7llu\r\n", (unsigned long long)ls.totalSurplusReleases );
#endif
            if ( ls.totalAbandonments > 0 )
                fprintf( fp, "  *** WARNING: Abandoned locks detected — possible deadlock risk! ***\r\n" );
            if ( ls.totalSurplusReleases > 0 )
                fprintf( fp, "  *** WARNING: Surplus lock releases detected — logic error in locking! ***\r\n" );
            fprintf( fp, "\r\n" );
        }
    }

    /* Size histogram */
    {
        UInt64 hCounts[ CMemoryTracker::HISTOGRAM_BUCKET_COUNT ];
        UInt64 hBytes [ CMemoryTracker::HISTOGRAM_BUCKET_COUNT ];
        tracker->GetSizeHistogram( hCounts, hBytes );

        static const char* const s_bucketLabels[ CMemoryTracker::HISTOGRAM_BUCKET_COUNT ] =
        {
            "     1 -    16 bytes",
            "    17 -    64 bytes",
            "    65 -   256 bytes",
            "   257 -  1023 bytes",
            "  1 KB -  4095 bytes",
            "  4 KB - 64535 bytes",
            " 64 KB -    1 MB    ",
            "      > 1 MB        "
        };

        fprintf( fp, "                    A L L O C A T I O N  S I Z E  H I S T O G R A M                \r\n" );
        fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
        fprintf( fp, "  %-20s  %12s  %18s\r\n", "Size Range", "Alloc Count", "Total Bytes" );
        fprintf( fp, "  %-20s  %12s  %18s\r\n", "--------------------", "------------", "------------------" );
        for ( UInt32 b = 0; b < CMemoryTracker::HISTOGRAM_BUCKET_COUNT; ++b )
        {
#ifdef GUCEF_MSWIN_BUILD
            fprintf( fp, "  %-20s  %12I64u  %18I64u\r\n", s_bucketLabels[ b ], hCounts[ b ], hBytes[ b ] );
#else
            fprintf( fp, "  %-20s  %12llu  %18llu\r\n", s_bucketLabels[ b ], (unsigned long long)hCounts[ b ], (unsigned long long)hBytes[ b ] );
#endif
        }
        fprintf( fp, "\r\n" );
    }

    fflush( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpMemoryAllocations( void )
{
    FILE* fp = OpenLogFile( GUCEF_NULL );
    if ( GUCEF_NULL == fp )
        return;
    DumpMemoryAllocationsToFile( fp );
    fclose( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpMemoryAllocationsToFile( FILE* fp )
{
    if ( GUCEF_NULL == fp )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );

    const CMemoryTracker::TRegistry& reg = tracker->GetRegistry();

    fprintf( fp, "              C U R R E N T L Y  A L L O C A T E D  M E M O R Y                 \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );

    int cnt = 1;
    CMemoryTracker::TRegistry::const_iterator i = reg.begin();
    while ( i != reg.end() )
    {
        CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL == rec->parentRecord )
        {
            fprintf( fp, "** Allocation # %2d\r\n", cnt++ );
            DumpRecord( fp, rec );
            fprintf( fp, "\r\n" );
        }
        ++i;
    }

    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "\r\n" );

    /* Sub-records (placement new) */
    fprintf( fp, "              C U R R E N T L Y  U S E D  M E M O R Y  S U B - P A R T S        \r\n" );
    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );

    cnt = 1;
    i = reg.begin();
    while ( i != reg.end() )
    {
        CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL != rec->parentRecord )
        {
            fprintf( fp, "** Usage # %2d\r\n", cnt++ );
            DumpRecord( fp, rec );
            fprintf( fp, "\r\n" );
        }
        ++i;
    }

    fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "\r\n" );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      Exception / crash reports                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CReporter::DumpExceptionReport( FILE* fp, void* faultAddress )
{
    if ( GUCEF_NULL == fp )
        return;

    fprintf( fp, "\r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "*********           Exception report for fault address: %p            \r\n", faultAddress );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "\r\n" );

    TCallStack* currentCallstack = GUCEF_NULL;
    DRGUP_GetCallstackForCurrentThread( &currentCallstack );
    if ( GUCEF_NULL != currentCallstack )
    {
        fprintf( fp, "             C U R R E N T  C A L L S T A C K       \r\n" );
        DRGUP_PrintCallstackCopyTo( currentCallstack, fp );
    }

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
    {
        DumpLogReportToFile( fp );
        return;
    }

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );

    CMemoryTracker::TUInt64ToRecordMap nearestDealloc;
    tracker->FindNearestDeallocatedRecords( faultAddress, 10, nearestDealloc );
    if ( !nearestDealloc.empty() )
    {
        fprintf( fp, "             N E A R E S T  D E A L L O C A T E D  M E M O R Y  N O D E S       \r\n" );
        fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
        UInt32 rank = 1;
        CMemoryTracker::TUInt64ToRecordMap::iterator di = nearestDealloc.begin();
        while ( di != nearestDealloc.end() )
        {
            fprintf( fp, "** Distance rank # %d\r\n", rank );
            #ifdef GUCEF_MSWIN_BUILD
            fprintf( fp, "Distance in bytes : %I64u\r\n", di->first );
            #else
            fprintf( fp, "Distance in bytes : %llu\r\n", di->first );
            #endif
            DumpRecord( fp, di->second );
            fprintf( fp, "\r\n" );
            ++di; ++rank;
        }
        fprintf( fp, "\r\n" );
    }

    CMemoryTracker::TUInt64ToRecordMap nearestCurrent;
    tracker->FindNearestCurrentRecords( faultAddress, 10, nearestCurrent );
    if ( !nearestCurrent.empty() )
    {
        fprintf( fp, "                     N E A R E S T  C U R R E N T  M E M O R Y  N O D E S       \r\n" );
        fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
        UInt32 rank = 1;
        CMemoryTracker::TUInt64ToRecordMap::iterator ci = nearestCurrent.begin();
        while ( ci != nearestCurrent.end() )
        {
            fprintf( fp, "** Distance rank # %d\r\n", rank );
            #ifdef GUCEF_MSWIN_BUILD
            fprintf( fp, "Distance in bytes : %I64u\r\n", ci->first );
            #else
            fprintf( fp, "Distance in bytes : %llu\r\n", ci->first );
            #endif
            DumpRecord( fp, ci->second );
            fprintf( fp, "\r\n" );
            ++ci; ++rank;
        }
        fprintf( fp, "\r\n" );
    }

    DumpLogReportToFile( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpExceptionReport( FILE*              fp                        ,
                                UInt32             blocksize                 ,
                                const void*        faultAddress              ,
                                CAllocationRecord* nearestCurrentRecord      ,
                                CAllocationRecord* nearestDeallocatedRecord  ,
                                bool               wouldHaveFitInDeallocated ,
                                bool               validateMemoryUnits       )
{
    if ( GUCEF_NULL == fp )
        return;

    fprintf( fp, "\r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "*********  Exception report for accessing %d bytes at fault address: %p            \r\n", blocksize, faultAddress );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "\r\n" );

    TCallStack* currentCallstack = GUCEF_NULL;
    DRGUP_GetCallstackForCurrentThread( &currentCallstack );
    if ( GUCEF_NULL != currentCallstack )
    {
        fprintf( fp, "             C U R R E N T  C A L L S T A C K       \r\n" );
        DRGUP_PrintCallstackCopyTo( currentCallstack, fp );
    }

    if ( GUCEF_NULL != nearestDeallocatedRecord )
    {
        fprintf( fp, "             N E A R E S T  D E A L L O C A T E D  M E M O R Y  N O D E       \r\n" );
        fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
        const Int8* repAddr = (const Int8*) nearestDeallocatedRecord->reportedAddress;
        const Int8* accAddr = (const Int8*) faultAddress;
        UInt64 dist = accAddr > repAddr ? (UInt64)(accAddr - repAddr) : (UInt64)(repAddr - accAddr);
        #ifdef GUCEF_MSWIN_BUILD
        fprintf( fp, "Distance in bytes : %I64u\r\n", dist );
        #else
        fprintf( fp, "Distance in bytes : %llu\r\n", dist );
        #endif
        DumpRecord( fp, nearestDeallocatedRecord );
        fprintf( fp, "\r\n" );
    }

    if ( GUCEF_NULL != nearestCurrentRecord )
    {
        fprintf( fp, "                     N E A R E S T  C U R R E N T  M E M O R Y  N O D E       \r\n" );
        fprintf( fp, "------------------------------------------------------------------------------- \r\n" );
        const Int8* repAddr = (const Int8*) nearestCurrentRecord->reportedAddress;
        const Int8* accAddr = (const Int8*) faultAddress;
        UInt64 dist = accAddr > repAddr ? (UInt64)(accAddr - repAddr) : (UInt64)(repAddr - accAddr);
        #ifdef GUCEF_MSWIN_BUILD
        fprintf( fp, "Distance in bytes : %I64u\r\n", dist );
        #else
        fprintf( fp, "Distance in bytes : %llu\r\n", dist );
        #endif
        DumpRecord( fp, nearestCurrentRecord );
        fprintf( fp, "\r\n" );
    }

    DumpLogReportToFile( fp );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      New aggregate report methods                                       //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CReporter::DumpCallsiteReport( UInt32 topN )
{
    FILE* fp = OpenLogFile( GUCEF_NULL );
    if ( GUCEF_NULL == fp )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
    {
        fclose( fp );
        return;
    }

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );
    const CMemoryTracker::TCallsiteMap& csmap = tracker->GetCallsiteMap();

    /* Collect and sort by currentLiveBytes descending */
    typedef std::pair< UInt64, const CCallsiteStats* > TSortEntry;
    std::vector< TSortEntry > sorted;
    sorted.reserve( csmap.size() );
    CMemoryTracker::TCallsiteMap::const_iterator it = csmap.begin();
    while ( it != csmap.end() )
    {
        sorted.push_back( TSortEntry( it->second.currentLiveBytes, &it->second ) );
        ++it;
    }
    std::sort( sorted.begin(), sorted.end(), [](const TSortEntry& a, const TSortEntry& b){ return a.first > b.first; } );

    fprintf( fp, "\r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "*********                C A L L S I T E  R E P O R T                 ********* \r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "  %-40s  %8s  %10s  %10s  %10s\r\n",
             "Callsite", "Live", "LiveBytes", "PeakBytes", "TotalBytes" );
    fprintf( fp, "  %-40s  %8s  %10s  %10s  %10s\r\n",
             "----------------------------------------", "--------", "----------", "----------", "----------" );

    UInt32 printed = 0;
    for ( size_t i = 0; i < sorted.size(); ++i )
    {
        if ( topN > 0 && printed >= topN )
            break;
        const CCallsiteStats* cs = sorted[ i ].second;
        char label[ 48 ];
        if ( GUCEF_NULL != cs->sourceFile )
            snprintf( label, sizeof(label), "%s:%u", cs->sourceFile, cs->sourceLine );
        else
            snprintf( label, sizeof(label), "?:%u", cs->sourceLine );
        label[ sizeof(label) - 1 ] = '\0';
#ifdef GUCEF_MSWIN_BUILD
        fprintf( fp, "  %-40s  %8I64u  %10I64u  %10I64u  %10I64u\r\n",
                 label,
                 (unsigned long long)(cs->allocCount - cs->freeCount),
                 cs->currentLiveBytes, cs->peakLiveBytes, cs->totalBytesAllocated );
#else
        fprintf( fp, "  %-40s  %8llu  %10llu  %10llu  %10llu\r\n",
                 label,
                 (unsigned long long)(cs->allocCount - cs->freeCount),
                 (unsigned long long)cs->currentLiveBytes,
                 (unsigned long long)cs->peakLiveBytes,
                 (unsigned long long)cs->totalBytesAllocated );
#endif
        ++printed;
    }
    fprintf( fp, "\r\n" );
    fclose( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpSizeHistogram( void )
{
    FILE* fp = OpenLogFile( GUCEF_NULL );
    if ( GUCEF_NULL == fp )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
    {
        fclose( fp );
        return;
    }

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );

    UInt64 hCounts[ CMemoryTracker::HISTOGRAM_BUCKET_COUNT ];
    UInt64 hBytes [ CMemoryTracker::HISTOGRAM_BUCKET_COUNT ];
    tracker->GetSizeHistogram( hCounts, hBytes );

    static const char* const s_bucketLabels[ CMemoryTracker::HISTOGRAM_BUCKET_COUNT ] =
    {
        "     1 -    16 bytes",
        "    17 -    64 bytes",
        "    65 -   256 bytes",
        "   257 -  1023 bytes",
        "  1 KB -  4095 bytes",
        "  4 KB - 64535 bytes",
        " 64 KB -    1 MB    ",
        "      > 1 MB        "
    };

    fprintf( fp, "\r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "*********           S I Z E  H I S T O G R A M  (lifetime)           ********* \r\n" );
    fprintf( fp, "******************************************************************************* \r\n" );
    fprintf( fp, "  %-20s  %12s  %18s\r\n", "Size Range", "Alloc Count", "Total Bytes" );
    fprintf( fp, "  %-20s  %12s  %18s\r\n", "--------------------", "------------", "------------------" );
    for ( UInt32 b = 0; b < CMemoryTracker::HISTOGRAM_BUCKET_COUNT; ++b )
    {
#ifdef GUCEF_MSWIN_BUILD
        fprintf( fp, "  %-20s  %12I64u  %18I64u\r\n", s_bucketLabels[ b ], hCounts[ b ], hBytes[ b ] );
#else
        fprintf( fp, "  %-20s  %12llu  %18llu\r\n", s_bucketLabels[ b ], (unsigned long long)hCounts[ b ], (unsigned long long)hBytes[ b ] );
#endif
    }
    fprintf( fp, "\r\n" );
    fclose( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpTimeline( const char* path )
{
    if ( GUCEF_NULL == path )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    FILE* fp = fopen( path, "wb" );
    if ( GUCEF_NULL == fp )
        return;

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );
    const CMemoryTracker::TRegistry& reg = tracker->GetRegistry();

    /* Header */
    fprintf( fp, "timestampUs\treportedSize\tfile\tline\tallocationType\r\n" );

    CMemoryTracker::TRegistry::const_iterator i = reg.begin();
    while ( i != reg.end() )
    {
        const CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL == rec->parentRecord )
        {
#ifdef GUCEF_MSWIN_BUILD
            fprintf( fp, "%I64u\t%zu\t%s\t%u\t%d\r\n",
                     rec->allocationTimestampUs,
                     rec->reportedSize,
                     GUCEF_NULL != rec->sourceFile ? rec->sourceFile : "?",
                     rec->sourceLine,
                     (int) rec->allocationType );
#else
            fprintf( fp, "%llu\t%zu\t%s\t%u\t%d\r\n",
                     (unsigned long long) rec->allocationTimestampUs,
                     rec->reportedSize,
                     GUCEF_NULL != rec->sourceFile ? rec->sourceFile : "?",
                     rec->sourceLine,
                     (int) rec->allocationType );
#endif
        }
        ++i;
    }
    fclose( fp );
}

/*-------------------------------------------------------------------------*/

void
CReporter::DumpMassifFormat( const char* path )
{
    if ( GUCEF_NULL == path )
        return;

    CMemoryTracker* tracker = CMemoryTracker::Instance();
    if ( GUCEF_NULL == tracker )
        return;

    FILE* fp = fopen( path, "wb" );
    if ( GUCEF_NULL == fp )
        return;

    MT::CScopeReaderLock readLock( tracker->GetDataLock() );
    const CMemoryTracker::TRegistry& reg = tracker->GetRegistry();

    /* Compute snapshot totals */
    size_t totalLiveBytes = 0;
    size_t numLive = 0;
    CMemoryTracker::TRegistry::const_iterator i = reg.begin();
    while ( i != reg.end() )
    {
        const CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL == rec->parentRecord )
        {
            totalLiveBytes += rec->reportedSize;
            ++numLive;
        }
        ++i;
    }

    /* Massif snapshot header */
    fprintf( fp, "desc: GUCEF MemoryLeakFinder snapshot-at-shutdown\r\n" );
    fprintf( fp, "cmd: (gucef instrumented application)\r\n" );
    fprintf( fp, "time_unit: ms\r\n" );
    fprintf( fp, "\r\n" );
    fprintf( fp, "#-----------\r\n" );
    fprintf( fp, "snapshot=0\r\n" );
    fprintf( fp, "#-----------\r\n" );
    fprintf( fp, "time=0\r\n" );
    fprintf( fp, "mem_heap_B=%zu\r\n", totalLiveBytes );
    fprintf( fp, "mem_heap_extra_B=0\r\n" );
    fprintf( fp, "mem_stacks_B=0\r\n" );
    fprintf( fp, "heap_tree=detailed\r\n" );
    fprintf( fp, "n%zu: %zu (heap allocation)\r\n", numLive, totalLiveBytes );

    /* Per-allocation detail */
    i = reg.begin();
    while ( i != reg.end() )
    {
        const CAllocationRecord* rec = i->second;
        if ( GUCEF_NULL != rec && GUCEF_NULL == rec->parentRecord )
        {
            fprintf( fp, " n0: %zu (", rec->reportedSize );
            if ( GUCEF_NULL != rec->sourceFile )
                fprintf( fp, "%s:%u", rec->sourceFile, rec->sourceLine );
            else
                fprintf( fp, "?" );
            fprintf( fp, ")\r\n" );
        }
        ++i;
    }
    fclose( fp );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRGUP */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
