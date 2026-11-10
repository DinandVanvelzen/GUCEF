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

#ifndef GUCEF_CORE_CDATATABLEFILTER_H
#define GUCEF_CORE_CDATATABLEFILTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

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

class CDataTableBufferPool;

/**
 *  Exact row filtering (section 5.5 of the data tables design): turns the resolved predicates of a query into a
 *  selection on each batch, with kernels specialized per storage and type: integer and day ranges, float ranges,
 *  bit tests for booleans, bytewise compares for text and binary, and a bitmap over the dictionary for dictionary
 *  columns, so a row test there is one indexed bit read. The first predicate makes the selection, each further one
 *  narrows it in place.
 *
 *  Prepare compiles the predicates once (it allocates); Apply then makes no heap allocation: the selection is a
 *  pooled buffer, and the dictionary bitmap keeps its capacity from one dictionary to the next. The filter holds the
 *  last dictionary it made a bitmap for, so a bitmap is never reused for another dictionary at the same address.
 *  One thread at a time; a scan gives each of its tasks a filter of its own.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableFilter
{
    public:

    CDataTableFilter( void );

    ~CDataTableFilter();

    /**
     *  Compiles the predicates of the query for batches whose projected column i holds schema column
     *  batchColumns[ i ] of the query's schema. INVALID_ARGUMENT, with the error, when a predicate's column is not
     *  among them or a literal is not in the native form of its column.
     */
    TDataTableStatus Prepare( const CDataTableResolvedQuery& query                    ,
                              const CDataTableSchema::TColumnIndexVector& batchColumns ,
                              CString& error                                           );

    /** True when nothing is to be tested: every row matches */
    bool IsEmpty( void ) const;

    /** True when a predicate matches no row: no batch has a selected row */
    bool MatchesNothing( void ) const;

    /**
     *  Narrows the selection of the batch (all rows when it has none) to the rows that match every compiled predicate
     *  the batch does not guarantee (CDataTableBatch::GetGuaranteedPredicates). A batch in which every row matches is
     *  left without a selection. selectedRowCount is the outcome; 0 means the batch is not to be delivered.
     *  SCHEMA_MISMATCH when a column of the batch does not have the storage of its type; errors of the pool.
     */
    TDataTableStatus Apply( CDataTableBatch& batch, CDataTableBufferPool& pool, UInt32& selectedRowCount );

    /** Forgets the compiled predicates and the dictionary it holds */
    void Clear( void );

    private:

    struct SCompiledPredicate;
    typedef GUCEF::vector< SCompiledPredicate* > TCompiledPredicateVector;

    CDataTableFilter( const CDataTableFilter& src );                /**< not implemented */
    CDataTableFilter& operator=( const CDataTableFilter& src );     /**< not implemented */

    TCompiledPredicateVector m_predicates;
    bool m_matchesNothing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEFILTER_H ? */
