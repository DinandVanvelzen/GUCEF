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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFER_H
#include "gucefCORE_CDataTableBuffer.h"
#define GUCEF_CORE_CDATATABLEBUFFER_H
#endif /* GUCEF_CORE_CDATATABLEBUFFER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H
#include "tablecodecpluginPARQUET_CParquetCompression.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETCOMPRESSION_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#include "tablecodecpluginPARQUET_ParquetMetaData.h"
#define TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_PARQUETMETADATA_H ? */

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

class CParquetColumnStage;

/**
 *  Writes a parquet file (section 13.8 of the data tables design). One thread at a time.
 *
 *  Rows are staged per column until the row group is full (rowGroupRowCount rows or rowGroupByteLimit bytes), at
 *  FlushRowGroup() or at Finish(); then every column chunk is encoded: data pages (v1) every pageRowCount rows, a
 *  variable column's page cut early at pageByteLimit, REQUIRED columns without levels, OPTIONAL columns with RLE /
 *  bit-packed definition levels made from the validity, dictionary encoding per the dictionary mode and the column's
 *  encoding hint (the rest of a chunk continues PLAIN once the dictionary passes dictionaryByteLimit), statistics per
 *  chunk and page, compression per the options. Chunks are written one after another; a PLAIN fixed width chunk is
 *  preceded by up to 7 padding bytes so its page values lie at multiples of 8 in the file, which lets readers use the
 *  pages in place. The row group lists its sort columns when the rows are sorted on the schema's sort key.
 *
 *  The footer holds the schema, the row groups, created_by, the column orders (type defined order) and the key value
 *  metadata of the options, plus gucef.table.schema (the table schema as JSON) when a JSON codec is registered.
 *  Output goes through CIOAccess::Write only (no Setpos), which must take every byte.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetTableWriter : public CORE::CITableWriter
{
    public:

    static const char* const CreatedBy;

    CParquetTableWriter( const CORE::IOAccessPtr& destination            ,
                         const CORE::CDataTableSchemaPtr& schema         ,
                         const CORE::CDataTableWriterOptions& options    );

    virtual ~CParquetTableWriter() GUCEF_VIRTUAL_OVERRIDE;

    /** Checks the schema and the options and prepares the compression; the writer is used only after OK */
    CORE::TDataTableStatus Init( CORE::CString& error );

    virtual CORE::TDataTableStatus WriteBatch( const CORE::CDataTableBatch& batch ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus FlushRowGroup( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TDataTableStatus Finish( CORE::CDataTableFileSummary& summary ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Abort( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::vector< CParquetColumnStage* > TColumnStageVector;

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message );
    CORE::TDataTableStatus Output( const void* data, UInt32 size );
    CORE::TDataTableStatus StartFile( void );
    CORE::TDataTableStatus AppendRows( const CORE::CDataTableBatch& batch, const UInt32* selection, UInt32 first, UInt32 count );
    UInt64 GetStagedBytes( void ) const;
    UInt64 EstimateRowBytes( const CORE::CDataTableBatch& batch ) const;
    bool IsStagedSortedOnKey( void ) const;
    void ReleaseStages( void );

    CParquetTableWriter( const CParquetTableWriter& src );                  /**< not implemented */
    CParquetTableWriter& operator=( const CParquetTableWriter& src );       /**< not implemented */

    CORE::IOAccessPtr m_destination;
    CORE::CDataTableSchemaPtr m_schema;
    CORE::CDataTableWriterOptions m_options;
    CParquetCompression m_compression;
    TColumnStageVector m_stages;
    CORE::CDynamicBuffer m_rowGroupBytes;   /**< the encoded RowGroup structures of the footer */
    UInt32 m_rowGroupCount;
    UInt64 m_rowCount;
    UInt64 m_position;
    bool m_started;
    bool m_finished;
    bool m_aborted;
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

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H ? */
