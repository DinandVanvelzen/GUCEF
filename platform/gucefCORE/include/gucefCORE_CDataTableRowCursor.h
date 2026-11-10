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

#ifndef GUCEF_CORE_CDATATABLEROWCURSOR_H
#define GUCEF_CORE_CDATATABLEROWCURSOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLEBATCHSOURCE_H
#include "gucefCORE_CIDataTableBatchSource.h"
#define GUCEF_CORE_CIDATATABLEBATCHSOURCE_H
#endif /* GUCEF_CORE_CIDATATABLEBATCHSOURCE_H ? */

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

/**
 *  Row by row iteration for Tier 1 users: over the batches of a batch source (a scan: a CDataTableScanPtr converts to
 *  a CDataTableBatchSourcePtr) or over one batch. Only selected rows are visited.
 *
 *  The cursor holds the current batch, so a value linked by GetValue is valid until the following Next().
 *  One thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableRowCursor
{
    public:

    explicit CDataTableRowCursor( const CDataTableBatchSourcePtr& source );

    explicit CDataTableRowCursor( const CDataTableBatchPtr& batch );

    ~CDataTableRowCursor();

    /** Moves to the next selected row, pulling batches as needed: OK, END when there are no more rows, or the status of the source */
    TDataTableStatus Next( void );

    /** Tier 1 access to the current row; false before the first Next(), after the end and for a column out of range */
    bool GetValue( UInt32 projectedColumn, CVariant& out, bool linkIfPossible = true ) const;

    /** The batch of the current row, NULL before the first Next() and after the end */
    const CDataTableBatchPtr& GetCurrentBatch( void ) const;

    /** The row within the current batch */
    UInt32 GetCurrentRow( void ) const;

    /** Rows Next() moved to */
    UInt64 GetRowsVisited( void ) const;

    /** The message of the source when Next() failed */
    CString GetLastErrorMessage( void ) const;

    private:

    CDataTableRowCursor( const CDataTableRowCursor& src );              /**< not implemented */
    CDataTableRowCursor& operator=( const CDataTableRowCursor& src );   /**< not implemented */

    CDataTableBatchSourcePtr m_source;
    CDataTableBatchPtr m_batch;
    CDataTableBatchPtr m_singleBatch;   /**< the batch of a cursor over one batch, until it is visited */
    UInt32 m_position;                  /**< index among the selected rows of the current batch */
    UInt32 m_row;
    UInt64 m_rowsVisited;
    bool m_started;
    bool m_ended;
    TDataTableStatus m_lastStatus;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CDATATABLEROWCURSOR_H ? */
