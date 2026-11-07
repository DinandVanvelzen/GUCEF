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

#ifndef GUCEF_CORE_CITABLEREADER_H
#define GUCEF_CORE_CITABLEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#include "gucefCORE_CITableFileMetaData.h"
#define GUCEF_CORE_CITABLEFILEMETADATA_H
#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

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
class CDataTableScanStats;

/**
 *  What a reader is asked to deliver from a row group
 */
class GUCEF_CORE_PUBLIC_CPP CDataTableReadRequest
{
    public:

    const CDataTableResolvedQuery* query;                   /**< the predicates the reader may use for dictionary and page pruning and sorted slices, GUCEF_NULL for none */
    CDataTableSchema::TColumnIndexVector projection;        /**< file schema columns to deliver, in delivery order */
    CDataTableSchema::TColumnIndexVector fileColumnOfTableColumn;   /**< per table schema column its file schema column, GUCEF_UINT32MAX when the file lacks it; used to map the query predicates */
    UInt32 batchRowCount;
    TDataTableFilterMode filterMode;
    bool materializeDictionaries;                           /**< text columns delivered as DATATABLE_STORAGE_VARIABLE */
    CDataTableBufferPool* bufferPool;                       /**< where the reader takes its buffers and batches */
    MT::CMutex* sharedFileLock;                             /**< held around every read of the shared input, GUCEF_NULL when the reader owns its input */
    UInt32 fileOrdinal;                                     /**< for the origin of the batches */
    CDataTableScanStats* stats;                             /**< where the reader adds its counters (pages skipped, in place and decoded, alignment copies, bytes read), GUCEF_NULL for none; owned by the task using the reader */

    CDataTableReadRequest( void );
};

/*-------------------------------------------------------------------------*/

/**
 *  Reads the row groups of one file as batches. Made by CITableCodec::CreateReader; used by one thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CITableReader
{
    public:

    CITableReader( void );

    virtual ~CITableReader();

    virtual const CITableFileMetaDataPtr& GetMetaData( void ) const = 0;

    /** Prepares a row group for reading; PRUNED when the reader proves no row matches (dictionary, pages) */
    virtual TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CDataTableReadRequest& request ) = 0;

    /** OK with a batch, END when the row group is done, or an error */
    virtual TDataTableStatus ReadNextBatch( CDataTableBatchPtr& batch ) = 0;

    virtual CString GetLastErrorMessage( void ) const = 0;

    private:

    CITableReader( const CITableReader& src );              /**< not implemented */
    CITableReader& operator=( const CITableReader& src );   /**< not implemented */
};

/*-------------------------------------------------------------------------*/

typedef CTSharedPtr< CITableReader, MT::CMutex >    CITableReaderPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CITABLEREADER_H ? */
