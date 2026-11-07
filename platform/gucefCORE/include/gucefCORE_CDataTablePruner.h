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

#ifndef GUCEF_CORE_CDATATABLEPRUNER_H
#define GUCEF_CORE_CDATATABLEPRUNER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#include "gucefCORE_CDataTableRowGroupInfo.h"
#define GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#endif /* GUCEF_CORE_CDATATABLEROWGROUPINFO_H ? */

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
 *  The generic pruning rules of section 5.3 of the data tables design: whether a unit (partition, row group, page)
 *  with the given statistics can hold a row matching a resolved predicate. Used by CDataTableScan for row groups, by
 *  CDataTablePartitionSpec::MayMatch for partitions and by codecs for pages. Reentrant, no allocation.
 *
 *  Statistics and literals are in the native form of 5.2: Int64 for signed integers, decimal mantissas, day counts,
 *  timestamp ticks and booleans (0 / 1), UInt64 for unsigned integers, Float64 for floats, the bytes of text and
 *  binary values, compared bytewise and unsigned. A unit without statistics is never pruned. Inexact bounds are used
 *  as bounds and never for the min == max test; float statistics holding NaN are not used.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTablePruner
{
    public:

    /**
     *  False when no row of a unit with these statistics can match the predicate on the column.
     *  statsColumn is the column the statistics were made for when it differs from the predicate's column (a file
     *  column read as a wider table column, a decimal of a lower scale); GUCEF_NULL when it is the same.
     */
    static bool MayMatch( const CDataTableResolvedPredicate& predicate     ,
                          const CDataTableColumnDef& column                ,
                          const CDataTableColumnStats& stats               ,
                          const CDataTableColumnDef* statsColumn = GUCEF_NULL );

    /**
     *  False when no row of the row group can match the query: an empty row group, or one predicate that MayMatch
     *  refutes. fileColumnOfTableColumn maps the table schema columns of the query to the file columns the statistics
     *  are indexed by, GUCEF_UINT32MAX for a column the file lacks (all null); empty when they are the same columns.
     *  fileSchema gives the file column types when they differ from the table's, GUCEF_NULL otherwise.
     */
    static bool RowGroupMayMatch( const CDataTableResolvedQuery& query                               ,
                                  const CDataTableRowGroupInfo& rowGroup                             ,
                                  const CDataTableSchema::TColumnIndexVector& fileColumnOfTableColumn ,
                                  const CDataTableSchema* fileSchema = GUCEF_NULL                    );

    /**
     *  Orders two values of the native form of the column type: negative, 0 or positive. Floats compare numerically
     *  with NaN after every number; text and binary bytewise. Values that are not of the native form compare equal.
     */
    static Int32 CompareNative( TDataTableLogicalType type, const CVariant& a, const CVariant& b );

    /** The bytes of a text or binary variant: text without its terminator */
    static const char* GetNativeBytes( const CVariant& value, UInt32& byteLength );

    private:

    CDataTablePruner( void );                                       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEPRUNER_H ? */
