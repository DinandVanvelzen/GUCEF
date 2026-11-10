/*
 *  tablecodecpluginPARQUET: GUCEF table codec plugin for Apache Parquet files
 *
 *  Copyright (C) 1998 - 2026.  Dinand Vanvelzen
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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLEREADER_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#include "gucefCORE_CDataTableColumnBatch.h"
#define GUCEF_CORE_CDATATABLECOLUMNBATCH_H
#endif /* GUCEF_CORE_CDATATABLECOLUMNBATCH_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#include "tablecodecpluginPARQUET_CParquetFileMetaData.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H
#include "tablecodecpluginPARQUET_CParquetCompression.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#include "tablecodecpluginPARQUET_CRleBitPackedDecoder.h"
#define TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H
#endif /* TABLECODECPLUGIN_PARQUET_CRLEBITPACKEDDECODER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace PARQUET {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the row groups of one parquet file as batches (section 13.4 of the data tables design). One thread at a time.
 *
 *  OpenRowGroup reads the byte ranges of the projected column chunks, ranges no more than ioCoalesceGapBytes apart with
 *  one Setpos + Read each (under the shared file lock of the request when there is one), into pooled buffers, and decodes
 *  the dictionary pages. EQUALS and IN predicates on a projected column whose data pages are all dictionary encoded
 *  (per encoding_stats) are tested against the dictionary: no literal present prunes the row group (PRUNED).
 *
 *  A batch covers rows [ start, start + batchRowCount ) of the row group. A column whose range lies inside one decoded
 *  page is a slice of that page; a PLAIN, null-free fixed width page whose width equals the batch width is used in place
 *  (the values point into the chunk buffer, or into the decompressed page buffer) when the host is little endian and
 *  the values are aligned, otherwise copied once. Ranges that cross pages are gathered into pooled buffers. Text and
 *  binary dictionary pages deliver dictionary storage over one dictionary per row group, unless the request asks for
 *  materialized dictionaries; numeric dictionary pages are gathered into plain values.
 *
 *  Everything read from the file is checked before it is used: sizes, counts and offsets against the bytes present and
 *  the read options, levels and dictionary indices against their bounds. A file that violates the format gives
 *  CORRUPT_DATA, a feature outside the subset UNSUPPORTED naming it, a bound LIMIT_EXCEEDED.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetTableReader : public CORE::CITableReader
{
    public:

    CParquetTableReader( const CORE::IOAccessPtr& source                 ,
                         const CORE::CITableFileMetaDataPtr& metaData    ,
                         const CParquetFileMetaData& parquetMetaData     ,
                         const SParquetReadOptions& options              );

    virtual ~CParquetTableReader() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ReadNextBatch( CORE::CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** The state of one projected column within the open row group */
    struct SColumnState
    {
        UInt32 fileColumn;
        const SParquetColumnInfo* info;
        const SParquetColumnMetaData* meta;
        CParquetCompression* compression;

        CORE::CDataTableBufferPtr chunkBuffer;
        const UInt8* chunkData;
        UInt32 chunkSize;
        UInt32 chunkPosition;
        UInt64 rowsLoaded;                      /**< rows of the data pages decoded so far */

        bool hasDictionary;
        UInt32 dictionaryCount;
        CORE::CDataTableColumnBatch dictionary; /**< the decoded dictionary values */
        CORE::CDataTableBatchPtr dictionaryBatch;   /**< text and binary: the batch holding the dictionary column */

        CORE::CDataTableColumnBatch page;       /**< the current page, every row of it */
        UInt64 pageFirstRow;
        UInt32 pageRows;
        bool pageLoaded;

        SColumnState( void );
    };

    typedef GUCEF::vector< SColumnState > TColumnStateVector;

    /** A column chunk to read, in file order */
    struct SChunkReadRange
    {
        UInt64 start;
        UInt64 end;
        UInt32 column;

        bool operator<( const SChunkReadRange& other ) const { return start < other.start; }
    };

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message );
    CORE::TDataTableStatus FailColumn( const SColumnState& column, CORE::TDataTableStatus status, const CORE::CString& message );

    CORE::TDataTableStatus ReadChunks( void );
    CORE::TDataTableStatus PrepareCompression( SColumnState& column );
    CORE::TDataTableStatus ReadPage( SColumnState& column, bool dictionaryOnly, bool& gotDataPage );
    CORE::TDataTableStatus GetPageBytes( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData,
                                         UInt32 levelBytes, bool compressed, const UInt8*& bytes, CORE::CDataTableBufferPtr& owner );
    CORE::TDataTableStatus DecodeDictionaryPage( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData );
    CORE::TDataTableStatus DecodeDataPageV1( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData );
    CORE::TDataTableStatus DecodeDataPageV2( SColumnState& column, const SParquetPageHeader& header, const UInt8* pageData );
    CORE::TDataTableStatus DecodeLevels( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                         CORE::CDataTableBufferPtr& validity, UInt32& nullCount );
    CORE::TDataTableStatus DecodeValues( SColumnState& column, Int32 encoding, const UInt8* data, UInt32 size, UInt32 valueCount,
                                         const CORE::CDataTableBufferPtr& validity, UInt32 nullCount,
                                         const CORE::CDataTableBufferPtr& pageOwner, CORE::CDataTableColumnBatch& out );
    CORE::TDataTableStatus DecodePlain( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                        const UInt8* validity, UInt32 nullCount, const CORE::CDataTableBufferPtr& pageOwner,
                                        bool countPage, CORE::CDataTableColumnBatch& out );
    CORE::TDataTableStatus DecodeDictionaryIndices( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                                    const UInt8* validity, UInt32 nullCount, CORE::CDataTableColumnBatch& out );
    CORE::TDataTableStatus DecodeRleBooleans( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                              const UInt8* validity, UInt32 nullCount, CORE::CDataTableColumnBatch& out );
    CORE::TDataTableStatus DecodeVariable( SColumnState& column, const UInt8* data, UInt32 size, UInt32 valueCount,
                                           const UInt8* validity, UInt32 nullCount, CORE::CDataTableColumnBatch& out );
    CORE::TDataTableStatus MaterializeDictionary( const SColumnState& column, const UInt32* indices, UInt32 rowCount,
                                                  const UInt8* validity, CORE::CDataTableColumnBatch& out );
    CORE::TDataTableStatus EnsurePage( SColumnState& column, UInt64 row );
    CORE::TDataTableStatus FillColumn( SColumnState& column, UInt64 firstRow, UInt32 rowCount, CORE::CDataTableColumnBatch& out );
    bool DictionaryMayMatch( void );
    CORE::TDataTableStatus AcquireZeroed( UInt32 bytes, CORE::CDataTableBufferPtr& buffer );
    CORE::TDataTableStatus CheckDecodedSize( const SColumnState& column, UInt64 bytes );

    CParquetTableReader( const CParquetTableReader& src );                  /**< not implemented */
    CParquetTableReader& operator=( const CParquetTableReader& src );       /**< not implemented */

    CORE::IOAccessPtr m_source;
    CORE::CITableFileMetaDataPtr m_metaDataPtr;
    const CParquetFileMetaData& m_metaData;
    SParquetReadOptions m_options;
    CParquetCompression m_compressions[ PARQUET_COMPRESSION_LZ4_RAW + 1 ];
    bool m_compressionReady[ PARQUET_COMPRESSION_LZ4_RAW + 1 ];

    CORE::CDataTableReadRequest m_request;
    CORE::CDataTableBufferPool* m_pool;
    UInt32 m_rowGroup;
    UInt64 m_rowGroupRows;
    UInt64 m_nextRow;
    UInt32 m_batchRowCount;
    bool m_open;
    TColumnStateVector m_columns;
    CORE::CDataTableSchema::TColumnIndexVector m_projection;
    GUCEF::vector< SChunkReadRange > m_ranges;
    CRleBitPackedDecoder m_decoder;
    CORE::CDataTableBufferPtr m_scratch;    /**< dense dictionary indices of the page being decoded */

    bool m_failed;
    CORE::TDataTableStatus m_failedStatus;
    CORE::CString m_lastError;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLEREADER_H ? */
