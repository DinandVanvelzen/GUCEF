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

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#define GUCEF_CORE_CDATATABLEQUERY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEPREDICATE_H
#include "gucefCORE_CDataTablePredicate.h"
#define GUCEF_CORE_CDATATABLEPREDICATE_H
#endif /* GUCEF_CORE_CDATATABLEPREDICATE_H ? */

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

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

class CDataTableResolvedQuery;

/**
 *  What a scan reads: the projection, the predicates (AND'ed) and how batches are delivered.
 *  The members return the query so calls can be chained: query.Select( "date" ).Select( "close" ).WhereRange( ... ).
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableQuery
{
    public:

    typedef GUCEF::vector< CDataTablePredicate > TPredicateVector;

    /** A projected column, by name or by schema index */
    struct GUCEF_CORE_PUBLIC_CPP SSelectedColumn
    {
        CString columnName;         /**< empty when selected by index */
        UInt32 columnIndex;         /**< GUCEF_UINT32MAX when selected by name */

        SSelectedColumn( void );
    };
    typedef GUCEF::vector< SSelectedColumn > TSelectedColumnVector;

    static const UInt64 NoLimit = GUCEF_UINT64MAX;

    CDataTableQuery( void );

    CDataTableQuery& Select( const CString& columnName );           /**< projection in the order given; none = all columns */
    CDataTableQuery& Select( UInt32 schemaColumnIndex );
    CDataTableQuery& Where( const CDataTablePredicate& predicate );
    CDataTableQuery& WhereEquals( const CString& columnName, const CVariant& value );
    CDataTableQuery& WhereIn( const CString& columnName, const CVariant::VariantVector& values );
    CDataTableQuery& WhereRange( const CString& columnName, const CVariant& low, bool lowInclusive, const CVariant& high, bool highInclusive );
    CDataTableQuery& WhereNull( const CString& columnName, bool isNull );

    CDataTableQuery& SetLimit( UInt64 maxRows );                    /**< default: no limit */
    CDataTableQuery& SetBatchRowCount( UInt32 rows );               /**< default 16384, clamped to 1024 .. 1048576 */
    CDataTableQuery& SetFilterMode( TDataTableFilterMode mode );    /**< FILTER_EXACT (default), FILTER_PRUNE_ONLY */
    CDataTableQuery& SetOrderedDelivery( bool ordered );            /**< default true */
    CDataTableQuery& SetMaterializeDictionaries( bool materialize );/**< default false */
    CDataTableQuery& SetMaxParallelism( UInt32 tasks );             /**< async: 0 = the pool's thread count */
    CDataTableQuery& SetMaxBatchesInFlight( UInt32 batches );       /**< async: 0 = 2 x parallelism */

    /**
     *  Turns names into column indices and literals into the column's native form (section 5.2):
     *  NOT_FOUND for unknown columns, TYPE_MISMATCH for literals of an unrelated type; the error names the predicate.
     */
    bool Resolve( const CDataTableSchema& schema, CDataTableResolvedQuery& resolved, CString& error ) const;

    const TSelectedColumnVector& GetSelection( void ) const;
    const TPredicateVector& GetPredicates( void ) const;
    UInt64 GetLimit( void ) const;
    UInt32 GetBatchRowCount( void ) const;
    TDataTableFilterMode GetFilterMode( void ) const;
    bool GetOrderedDelivery( void ) const;
    bool GetMaterializeDictionaries( void ) const;
    UInt32 GetMaxParallelism( void ) const;
    UInt32 GetMaxBatchesInFlight( void ) const;

    private:

    TSelectedColumnVector m_selection;
    TPredicateVector m_predicates;
    UInt64 m_limit;
    UInt32 m_batchRowCount;
    TDataTableFilterMode m_filterMode;
    bool m_orderedDelivery;
    bool m_materializeDictionaries;
    UInt32 m_maxParallelism;
    UInt32 m_maxBatchesInFlight;
};

/*-------------------------------------------------------------------------*/

/**
 *  A query resolved against a schema: what every backend and the generic scan work from.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableResolvedQuery
{
    public:

    typedef GUCEF::vector< CDataTableResolvedPredicate > TResolvedPredicateVector;

    CDataTableSchemaPtr schema;                         /**< the schema the indices refer to; Resolve sets a shared copy when it is NULL */
    CDataTableSchema::TColumnIndexVector projection;    /**< schema columns in delivery order; every column when the query selected none */
    TResolvedPredicateVector predicates;
    bool matchesNothing;                                /**< a predicate can match no row: the scan delivers nothing */
    TDataTableStatus status;                            /**< outcome of Resolve: OK, NOT_FOUND, TYPE_MISMATCH, INVALID_ARGUMENT or UNSUPPORTED */
    UInt64 limit;
    UInt32 batchRowCount;
    TDataTableFilterMode filterMode;
    bool orderedDelivery;
    bool materializeDictionaries;
    UInt32 maxParallelism;
    UInt32 maxBatchesInFlight;

    CDataTableResolvedQuery( void );

    /** The resolved predicates on the schema column, in query order */
    void GetPredicatesOnColumn( UInt32 schemaColumnIndex, TResolvedPredicateVector& columnPredicates ) const;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */
