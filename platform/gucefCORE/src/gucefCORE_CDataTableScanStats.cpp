/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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


#include "gucefCORE_CDataTableScanStats.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDataTableScanStats::CDataTableScanStats( void )
    : CICloneable()
    , filesListed( 0 )
    , filesPruned( 0 )
    , filesOpened( 0 )
    , filesSkipped( 0 )
    , footerCacheHits( 0 )
    , rowGroupsTotal( 0 )
    , rowGroupsPrunedByStatistics( 0 )
    , rowGroupsPrunedByDictionary( 0 )
    , pagesSkipped( 0 )
    , pagesInPlace( 0 )
    , pagesDecoded( 0 )
    , alignmentCopies( 0 )
    , bytesRead( 0 )
    , rowsScanned( 0 )
    , rowsSelected( 0 )
    , batchesDelivered( 0 )
    , poolMisses( 0 )
    , valueErrors( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableScanStats::CDataTableScanStats( const CDataTableScanStats& src )
    : CICloneable( src )
    , filesListed( src.filesListed )
    , filesPruned( src.filesPruned )
    , filesOpened( src.filesOpened )
    , filesSkipped( src.filesSkipped )
    , footerCacheHits( src.footerCacheHits )
    , rowGroupsTotal( src.rowGroupsTotal )
    , rowGroupsPrunedByStatistics( src.rowGroupsPrunedByStatistics )
    , rowGroupsPrunedByDictionary( src.rowGroupsPrunedByDictionary )
    , pagesSkipped( src.pagesSkipped )
    , pagesInPlace( src.pagesInPlace )
    , pagesDecoded( src.pagesDecoded )
    , alignmentCopies( src.alignmentCopies )
    , bytesRead( src.bytesRead )
    , rowsScanned( src.rowsScanned )
    , rowsSelected( src.rowsSelected )
    , batchesDelivered( src.batchesDelivered )
    , poolMisses( src.poolMisses )
    , valueErrors( src.valueErrors )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableScanStats::~CDataTableScanStats()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDataTableScanStats&
CDataTableScanStats::operator=( const CDataTableScanStats& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        filesListed = src.filesListed;
        filesPruned = src.filesPruned;
        filesOpened = src.filesOpened;
        filesSkipped = src.filesSkipped;
        footerCacheHits = src.footerCacheHits;
        rowGroupsTotal = src.rowGroupsTotal;
        rowGroupsPrunedByStatistics = src.rowGroupsPrunedByStatistics;
        rowGroupsPrunedByDictionary = src.rowGroupsPrunedByDictionary;
        pagesSkipped = src.pagesSkipped;
        pagesInPlace = src.pagesInPlace;
        pagesDecoded = src.pagesDecoded;
        alignmentCopies = src.alignmentCopies;
        bytesRead = src.bytesRead;
        rowsScanned = src.rowsScanned;
        rowsSelected = src.rowsSelected;
        batchesDelivered = src.batchesDelivered;
        poolMisses = src.poolMisses;
        valueErrors = src.valueErrors;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CDataTableScanStats&
CDataTableScanStats::operator+=( const CDataTableScanStats& other )
{GUCEF_TRACE;

    filesListed += other.filesListed;
    filesPruned += other.filesPruned;
    filesOpened += other.filesOpened;
    filesSkipped += other.filesSkipped;
    footerCacheHits += other.footerCacheHits;
    rowGroupsTotal += other.rowGroupsTotal;
    rowGroupsPrunedByStatistics += other.rowGroupsPrunedByStatistics;
    rowGroupsPrunedByDictionary += other.rowGroupsPrunedByDictionary;
    pagesSkipped += other.pagesSkipped;
    pagesInPlace += other.pagesInPlace;
    pagesDecoded += other.pagesDecoded;
    alignmentCopies += other.alignmentCopies;
    bytesRead += other.bytesRead;
    rowsScanned += other.rowsScanned;
    rowsSelected += other.rowsSelected;
    batchesDelivered += other.batchesDelivered;
    poolMisses += other.poolMisses;
    valueErrors += other.valueErrors;
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CDataTableScanStats::operator==( const CDataTableScanStats& other ) const
{GUCEF_TRACE;

    return filesListed == other.filesListed &&
           filesPruned == other.filesPruned &&
           filesOpened == other.filesOpened &&
           filesSkipped == other.filesSkipped &&
           footerCacheHits == other.footerCacheHits &&
           rowGroupsTotal == other.rowGroupsTotal &&
           rowGroupsPrunedByStatistics == other.rowGroupsPrunedByStatistics &&
           rowGroupsPrunedByDictionary == other.rowGroupsPrunedByDictionary &&
           pagesSkipped == other.pagesSkipped &&
           pagesInPlace == other.pagesInPlace &&
           pagesDecoded == other.pagesDecoded &&
           alignmentCopies == other.alignmentCopies &&
           bytesRead == other.bytesRead &&
           rowsScanned == other.rowsScanned &&
           rowsSelected == other.rowsSelected &&
           batchesDelivered == other.batchesDelivered &&
           poolMisses == other.poolMisses &&
           valueErrors == other.valueErrors;
}

/*-------------------------------------------------------------------------*/

void
CDataTableScanStats::Clear( void )
{GUCEF_TRACE;

    filesListed = 0;
    filesPruned = 0;
    filesOpened = 0;
    filesSkipped = 0;
    footerCacheHits = 0;
    rowGroupsTotal = 0;
    rowGroupsPrunedByStatistics = 0;
    rowGroupsPrunedByDictionary = 0;
    pagesSkipped = 0;
    pagesInPlace = 0;
    pagesDecoded = 0;
    alignmentCopies = 0;
    bytesRead = 0;
    rowsScanned = 0;
    rowsSelected = 0;
    batchesDelivered = 0;
    poolMisses = 0;
    valueErrors = 0;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CDataTableScanStats::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CDataTableScanStats( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
