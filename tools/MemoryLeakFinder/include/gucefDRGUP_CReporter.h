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

#ifndef GUCEF_DRGUP_CREPORTER_H
#define GUCEF_DRGUP_CREPORTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#ifndef GUCEF_DRGUP_ETYPES_H
#include "gucefDRGUP_ETypes.h"
#define GUCEF_DRGUP_ETYPES_H
#endif /* GUCEF_DRGUP_ETYPES_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 * All formatting, statistics reporting, and crash/exception report logic.
 *
 * For normal operation: uses the CMemoryTracker singleton under its own
 * reader lock. All output goes through CMemoryTracker::Log() so it reaches
 * the configured log file.
 *
 * For crash-time (SEH) reports: uses direct fprintf() to an already-open
 * FILE* because the log infrastructure may be unstable.
 */
class GUCEF_HIDDEN CReporter
{
    public:

    CReporter( void );
    ~CReporter( void );

    /* ------------------------------------------------------------------ */
    /* Normal reports (acquire reader lock internally)                      */
    /* ------------------------------------------------------------------ */

    /** Write the full statistics summary to the log file */
    void DumpLogReport( void );

    /** Write all currently live allocations to the log file */
    void DumpMemoryAllocations( void );

    /**
     * Write per-callsite aggregated statistics to the log file.
     * Only useful when enableCallsiteProfiling=true in config.
     * @param topN  show the top N callsites by current live bytes; 0=all
     */
    void DumpCallsiteReport( UInt32 topN = 20 );

    /** Write the size histogram table to the log file */
    void DumpSizeHistogram( void );

    /**
     * Write a TSV (tab-separated) timeline of allocation events to a file.
     * Columns: timestampUs, liveBytes, liveCount, file, line
     * Suitable for import into Excel, Python pandas, or any graphing tool.
     * @param path  output file path
     */
    void DumpTimeline( const char* path );

    /**
     * Write a Valgrind massif-compatible snapshot of the current heap state.
     * The output file can be loaded into massif-visualizer or processed with ms_print.
     * This is a snapshot-at-call-time implementation (not periodic).
     * @param path  output file path
     */
    void DumpMassifFormat( const char* path );

    /* ------------------------------------------------------------------ */
    /* Crash-time reports (direct fprintf to supplied FILE*)                */
    /* ------------------------------------------------------------------ */

    /** Write the exception report for a single fault address */
    void DumpExceptionReport( FILE* fp, void* faultAddress );

    /** Write exception report with pre-resolved nearest nodes */
    void DumpExceptionReport( FILE*              fp                        ,
                              UInt32             blocksize                 ,
                              const void*        faultAddress              ,
                              CAllocationRecord* nearestCurrentRecord      ,
                              CAllocationRecord* nearestDeallocatedRecord  ,
                              bool               wouldHaveFitInDeallocated ,
                              bool               validateMemoryUnits       );

    /* ------------------------------------------------------------------ */
    /* Node formatting helpers                                              */
    /* ------------------------------------------------------------------ */
    void DumpRecord( FILE* fp, CAllocationRecord* record );

    private:

    void DumpLogReportToFile( FILE* fp );
    void DumpMemoryAllocationsToFile( FILE* fp );

    FILE* OpenLogFile( bool* wasCleanedFlag );

    /* Formatting helpers */
    static char* InsertCommas( size_t value );
    static char* MemorySizeString( size_t size, bool lengthenString = true );
    static char* FormatOwnerString( const char* file, int line );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRGUP */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/

#endif /* GUCEF_DRGUP_CREPORTER_H ? */
