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

#ifndef GUCEF_CORE_CDATATABLESCANSTATS_H
#define GUCEF_CORE_CDATATABLESCANSTATS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICLONEABLE_H
#include "CICloneable.h"
#define GUCEF_CORE_CICLONEABLE_H
#endif /* GUCEF_CORE_CICLONEABLE_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Counters of a scan (section 6.4). Event data of CDataTableScan::ScanFinishedEvent.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableScanStats : public CICloneable
{
    public:

    UInt64 filesListed;                   /**< files the source listed */
    UInt64 filesPruned;                   /**< files dropped by partition pruning */
    UInt64 filesOpened;                   /**< files a reader was opened on */
    UInt64 filesSkipped;                  /**< incompatible files skipped (onIncompatibleFile = skip) */
    UInt64 footerCacheHits;               /**< files whose metadata came from a cache */
    UInt64 rowGroupsTotal;                /**< row groups of the opened files */
    UInt64 rowGroupsPrunedByStatistics;   /**< row groups skipped on their statistics */
    UInt64 rowGroupsPrunedByDictionary;   /**< row groups skipped on their dictionaries */
    UInt64 pagesSkipped;                  /**< pages skipped on their statistics */
    UInt64 pagesInPlace;                  /**< pages whose values batches point into without a copy */
    UInt64 pagesDecoded;                  /**< pages decoded or gathered */
    UInt64 alignmentCopies;               /**< pages copied to align their values */
    UInt64 bytesRead;                     /**< bytes read from the inputs */
    UInt64 rowsScanned;                   /**< rows of the delivered row ranges */
    UInt64 rowsSelected;                  /**< rows selected by the filter */
    UInt64 batchesDelivered;              /**< batches handed to the consumer */
    UInt64 poolMisses;                    /**< buffer pool misses during the scan */
    UInt64 valueErrors;                   /**< values a codec turned into nulls (CSV onValueError = null) */

    CDataTableScanStats( void );

    CDataTableScanStats( const CDataTableScanStats& src );

    virtual ~CDataTableScanStats() GUCEF_VIRTUAL_OVERRIDE;

    CDataTableScanStats& operator=( const CDataTableScanStats& src );

    /** Adds the counters of another part of the same scan, for example of one task */
    CDataTableScanStats& operator+=( const CDataTableScanStats& other );

    bool operator==( const CDataTableScanStats& other ) const;

    void Clear( void );

    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLESCANSTATS_H ? */
