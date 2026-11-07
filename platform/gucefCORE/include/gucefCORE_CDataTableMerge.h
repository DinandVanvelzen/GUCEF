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

#ifndef GUCEF_CORE_CDATATABLEMERGE_H
#define GUCEF_CORE_CDATATABLEMERGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

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

class CDataTableBufferPool;

/**
 *  The row level work of a copy-on-write partition change (section 7.2 of the data tables design), shared by every
 *  backend that rewrites a partition: CMemoryDataTable, CFileDataTable and the VFS backend. Batches given and made
 *  here hold every column of the schema, in schema order, without a selection ("full batches").
 *  Reentrant: all state is in the arguments.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableMerge
{
    public:

    typedef GUCEF::vector< CDataTableBatchPtr > TBatchVector;

    /**
     *  The selected rows of a batch as a full batch of the schema. Columns are matched by name when the batch has a
     *  schema, by schema column index otherwise, and must have the type of the schema column; a column the batch lacks
     *  is null and must be nullable. A batch of the schema's columns in order without a selection shares its buffers.
     *  TYPE_MISMATCH, with the column named, for anything else.
     */
    static TDataTableStatus Normalize( const CDataTableSchemaPtr& schema    ,
                                       const CDataTableBatch& rows          ,
                                       CDataTableBufferPool& pool           ,
                                       CDataTableBatchPtr& normalized       ,
                                       CString& error                       );

    /**
     *  The rows of a partition after a write: the existing rows combined with the incoming rows per mode. INSERT adds
     *  the rows and refuses, with CONFLICT, a primary key that is already present or given twice; UPSERT (a primary
     *  key is required, INVALID_ARGUMENT otherwise) replaces rows with the same key, the last incoming row of a key
     *  winning, and counts them in rowsReplaced; REPLACE_PARTITIONS keeps only the incoming rows. The result is sorted
     *  stably by the sort key (ascending, nulls last) and cut into batches of at most rowGroupRowCount rows.
     */
    static TDataTableStatus Merge( const CDataTableSchemaPtr& schema    ,
                                   const TBatchVector& existing         ,
                                   const TBatchVector& incoming         ,
                                   TDataTableWriteMode mode             ,
                                   UInt32 rowGroupRowCount              ,
                                   CDataTableBufferPool& pool           ,
                                   TBatchVector& result                 ,
                                   UInt64& rowsReplaced                 ,
                                   CString& error                       );

    /**
     *  The canonical form of a partition (Compact): sorted by the sort key, duplicates of the primary key removed with
     *  the last in order winning (counted in rowsRemoved), in batches of rowGroupRowCount rows.
     */
    static TDataTableStatus Canonicalize( const CDataTableSchemaPtr& schema ,
                                          const TBatchVector& batches       ,
                                          UInt32 rowGroupRowCount           ,
                                          CDataTableBufferPool& pool        ,
                                          TBatchVector& result              ,
                                          UInt64& rowsRemoved               ,
                                          CString& error                    );

    /**
     *  The rows of full batches that do not match the query (resolved against the schema of the batches); the rows
     *  that match are counted in rowsRemoved. Batches without a matching row are kept as they are.
     */
    static TDataTableStatus RemoveMatchingRows( const CDataTableResolvedQuery& query ,
                                                const TBatchVector& batches          ,
                                                CDataTableBufferPool& pool           ,
                                                TBatchVector& result                 ,
                                                UInt64& rowsRemoved                  ,
                                                CString& error                       );

    /** Exact statistics of every column of a full batch: min and max of the non-null (and non-NaN) values, null and value counts */
    static void ComputeStats( const CDataTableBatch& batch, CDataTableRowGroupInfo& info );

    /** Orders two cells of columns of the same type: values ascending, NaN after numbers, nulls last */
    static Int32 CompareCells( const CDataTableColumnBatch& left, UInt32 leftRow, const CDataTableColumnBatch& right, UInt32 rightRow );

    private:

    CDataTableMerge( void );                                        /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEMERGE_H ? */
