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

#ifndef GUCEF_CORE_CITABLEWRITER_H
#define GUCEF_CORE_CITABLEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#include "gucefCORE_CDataTableColumnStats.h"
#define GUCEF_CORE_CDATATABLECOLUMNSTATS_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNSTATS_H ? */

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
 *  What a writer wrote, from CITableWriter::Finish
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableFileSummary
{
    public:

    typedef GUCEF::vector< CDataTableColumnStats > TColumnStatsVector;

    UInt64 rowCount;
    UInt32 rowGroupCount;
    UInt64 byteCount;                       /**< bytes written, footer included */
    TColumnStatsVector columnStats;         /**< per schema column, over the whole file */

    CDataTableFileSummary( void );

    void Clear( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Writes batches of one schema into one file. Made by CITableCodec::CreateWriter; used by one thread at a time.
 *  A writer destroyed without Finish() aborts: the output is incomplete and the caller removes it.
 */
class GUCEF_CORE_PUBLIC_CPP CITableWriter
{
    public:

    CITableWriter( void );

    virtual ~CITableWriter();

    /** Flushes row groups as they fill */
    virtual TDataTableStatus WriteBatch( const CDataTableBatch& batch ) = 0;

    virtual TDataTableStatus FlushRowGroup( void ) = 0;

    /** Writes the footer; rows, row groups, bytes and per column statistics in the summary */
    virtual TDataTableStatus Finish( CDataTableFileSummary& summary ) = 0;

    /** Stops writing; the output is incomplete */
    virtual void Abort( void ) = 0;

    virtual CString GetLastErrorMessage( void ) const = 0;

    private:

    CITableWriter( const CITableWriter& src );              /**< not implemented */
    CITableWriter& operator=( const CITableWriter& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CITableWriter, MT::CMutex >    CITableWriterPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CITABLEWRITER_H ? */
