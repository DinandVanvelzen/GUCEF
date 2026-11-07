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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#include "gucefCORE_CDataNodeSerializableSettings.h"
#define GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H
#endif /* GUCEF_CORE_CDATANODESERIALIZABLESETTINGS_H ? */

#ifndef GUCEF_CORE_CMFILEACCESS_H
#include "CMFileAccess.h"
#define GUCEF_CORE_CMFILEACCESS_H
#endif /* GUCEF_CORE_CMFILEACCESS_H ? */

#ifndef GUCEF_CORE_CJSONSYNTAXVALIDATOR_H
#include "gucefCORE_CJsonSyntaxValidator.h"
#define GUCEF_CORE_CJSONSYNTAXVALIDATOR_H
#endif /* GUCEF_CORE_CJSONSYNTAXVALIDATOR_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#include "tablecodecpluginPARQUET_ByteOrder.h"
#define TABLECODECPLUGIN_PARQUET_BYTEORDER_H
#endif /* TABLECODECPLUGIN_PARQUET_BYTEORDER_H ? */

#include "tablecodecpluginPARQUET_CParquetFileMetaData.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CParquetFileMetaData::TableSchemaKey = "gucef.table.schema";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SChunkOrder
{
    UInt64 start;
    UInt64 end;
    UInt32 index;

    bool operator<( const SChunkOrder& other ) const { return start < other.start; }
};

typedef GUCEF::vector< SChunkOrder > TChunkOrderVector;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

SParquetChunkLocation::SParquetChunkLocation( void )
    : start( 0 )
    , length( 0 )
    , status( CORE::DATATABLE_STATUS_OK )
    , problem()
{
}

/*-------------------------------------------------------------------------*/

CParquetTableSchemaCache::CParquetTableSchemaCache( void )
    : m_lock()
    , m_schemas()
    , m_order()
    , m_hits( 0 )
    , m_misses( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CParquetTableSchemaCache::~CParquetTableSchemaCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CParquetTableSchemaCache::Find( const CORE::CString& text, CORE::CDataTableSchemaPtr& schema ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TSchemaMap::const_iterator i = m_schemas.find( text );
    if ( i == m_schemas.end() )
    {
        ++m_misses;
        return false;
    }
    schema = (*i).second;
    ++m_hits;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CParquetTableSchemaCache::Put( const CORE::CString& text, const CORE::CDataTableSchemaPtr& schema )
{GUCEF_TRACE;

    if ( schema.IsNULL() )
        return;
    MT::CScopeMutex lock( m_lock );
    TSchemaMap::iterator i = m_schemas.find( text );
    if ( i != m_schemas.end() )
    {
        // put by another thread meanwhile: either is the same schema
        (*i).second = schema;
        return;
    }
    while ( m_order.size() >= MaxEntries )
    {
        m_schemas.erase( m_order.front() );
        m_order.erase( m_order.begin() );
    }
    m_schemas[ text ] = schema;
    m_order.push_back( text );
}

/*-------------------------------------------------------------------------*/

UInt32
CParquetTableSchemaCache::GetEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_schemas.size();
}

/*-------------------------------------------------------------------------*/

UInt64
CParquetTableSchemaCache::GetHitCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_hits;
}

/*-------------------------------------------------------------------------*/

UInt64
CParquetTableSchemaCache::GetMissCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_misses;
}

/*-------------------------------------------------------------------------*/

CParquetFileMetaData::CParquetFileMetaData( void )
    : CORE::CITableFileMetaData()
    , m_footer()
    , m_footerSize( 0 )
    , m_fileSize( 0 )
    , m_dataEnd( 0 )
    , m_options()
    , m_metaData()
    , m_parsedBytes( 0 )
    , m_columns()
    , m_schema()
    , m_rowGroupInfos()
    , m_chunkLocations()
    , m_keyValues()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CParquetFileMetaData::~CParquetFileMetaData()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetFileMetaData::Read( CORE::CIOAccess& source                    ,
                            const SParquetReadOptions& options         ,
                            CORE::CDataTableBufferPool& pool           ,
                            CORE::CITableFileMetaDataPtr& metaData     ,
                            CORE::CString& error                       ,
                            CParquetTableSchemaCache* schemaCache      )
{GUCEF_TRACE;

    metaData.Unlink();
    const UInt64 fileSize = source.GetSize();
    if ( fileSize < 12 )
    {
        error = "The file has " + CORE::ToString( fileSize ) + " bytes, too few for a parquet file";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    UInt8 tail[ 8 ];
    if ( 0 != source.Setpos( fileSize - 8 ) || 8 != source.Read( tail, 1, 8 ) )
    {
        error = "The end of the file cannot be read";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    if ( 0 == memcmp( tail + 4, "PARE", 4 ) )
    {
        error = "The file has an encrypted footer (PARE), encryption is not supported";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    if ( 0 != memcmp( tail + 4, "PAR1", 4 ) )
    {
        error = "The file does not end with the parquet magic PAR1";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    const UInt32 footerSize = LoadLE32( tail );
    if ( 0 == footerSize || (UInt64) footerSize > fileSize - 12 )
    {
        error = "The footer length " + CORE::ToString( footerSize ) + " does not fit a file of " + CORE::ToString( fileSize ) + " bytes";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    if ( (UInt64) footerSize > options.maxFooterBytes )
    {
        error = "The footer of " + CORE::ToString( footerSize ) + " bytes is larger than maxFooterBytes (" + CORE::ToString( options.maxFooterBytes ) + ")";
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    }

    UInt8 head[ 4 ];
    if ( 0 != source.Setpos( 0 ) || 4 != source.Read( head, 1, 4 ) )
    {
        error = "The start of the file cannot be read";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    if ( 0 != memcmp( head, "PAR1", 4 ) )
    {
        error = "The file does not start with the parquet magic PAR1";
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }

    CORE::CTSharedPtr< CParquetFileMetaData, MT::CMutex > result( GUCEF_NEW CParquetFileMetaData() );
    CParquetFileMetaData& m = *result;
    CORE::TDataTableStatus status = pool.Acquire( footerSize, m.m_footer );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        error = "No buffer for a footer of " + CORE::ToString( footerSize ) + " bytes";
        return status;
    }
    const UInt64 footerStart = fileSize - 8 - footerSize;
    if ( 0 != source.Setpos( footerStart ) || footerSize != source.Read( m.m_footer->GetData(), 1, footerSize ) )
    {
        error = "The footer cannot be read";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    m.m_footer->SetDataSize( footerSize );
    m.m_footerSize = footerSize;
    m.m_fileSize = fileSize;
    m.m_dataEnd = footerStart;
    m.m_options = options;

    status = m.Parse( error, schemaCache );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    metaData = CORE::CITableFileMetaDataPtr( result );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetFileMetaData::Parse( CORE::CString& error, CParquetTableSchemaCache* schemaCache )
{GUCEF_TRACE;

    CThriftCompactReader reader( static_cast< const UInt8* >( m_footer->GetData() ), m_footerSize );
    SParquetParseBudget budget( m_options.maxMetaDataBytes );
    if ( !CParquetMetaDataSerializer::ReadFileMetaData( reader, m_metaData, budget ) )
    {
        error = "The footer is not valid: " + reader.GetErrorMessage();
        return reader.GetStatus();
    }
    m_parsedBytes = budget.usedObjectBytes;
    if ( m_metaData.hasEncryptionAlgorithm )
    {
        error = "The file is encrypted, encryption is not supported";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }

    CORE::CDataTableSchema schema;
    CORE::TDataTableStatus status = CParquetSchemaMapping::MapFileSchema( m_metaData, m_columns, schema, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    // key values as the file holds them; a key without a value is an empty text
    for ( size_t i=0; i<m_metaData.keyValueMetaData.size(); ++i )
    {
        const SParquetKeyValue& kv = m_metaData.keyValueMetaData[ i ];
        m_keyValues.Set( kv.key.ToString(), CORE::CVariant( kv.hasValue ? kv.value.ToString() : CORE::CString::Empty ) );
    }
    ApplyTableSchema( schema, schemaCache );
    m_schema = CORE::CreateDataTableSchemaPtr( schema );

    // row groups: the chunks of every leaf, where they lie, and their statistics
    const UInt32 columnCount = (UInt32) m_columns.size();
    UInt64 totalRows = 0;
    m_rowGroupInfos.resize( m_metaData.rowGroups.size() );
    m_chunkLocations.resize( m_metaData.rowGroups.size() * (size_t) columnCount );
    for ( size_t g=0; g<m_metaData.rowGroups.size(); ++g )
    {
        const SParquetRowGroup& rowGroup = m_metaData.rowGroups[ g ];
        if ( rowGroup.columns.size() != columnCount )
        {
            error = "Row group " + CORE::ToString( (UInt32) g ) + " has " + CORE::ToString( (UInt32) rowGroup.columns.size() ) +
                    " column chunks for a schema of " + CORE::ToString( columnCount ) + " columns";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        if ( (UInt64) rowGroup.numRows > GUCEF_UINT64MAX - totalRows )
        {
            error = "The row counts of the row groups overflow";
            return CORE::DATATABLE_STATUS_CORRUPT_DATA;
        }
        totalRows += (UInt64) rowGroup.numRows;

        CORE::CDataTableRowGroupInfo& info = m_rowGroupInfos[ g ];
        info.Clear();
        info.rowCount = (UInt64) rowGroup.numRows;
        info.columnStats.resize( columnCount );

        TChunkOrderVector order;
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            const SParquetColumnChunk& chunk = rowGroup.columns[ c ];
            const SParquetColumnInfo& column = m_columns[ c ];
            SParquetChunkLocation& location = m_chunkLocations[ g * columnCount + c ];
            CParquetSchemaMapping::BuildColumnStats( column, chunk, rowGroup.numRows, info.columnStats[ c ] );

            if ( chunk.hasFilePath )
            {
                location.status = CORE::DATATABLE_STATUS_UNSUPPORTED;
                location.problem = "the column chunk is in another file (" + chunk.filePath.ToString() + ")";
                continue;
            }
            if ( chunk.isEncrypted || !chunk.hasMetaData )
            {
                location.status = CORE::DATATABLE_STATUS_UNSUPPORTED;
                location.problem = "the column chunk is encrypted or has no metadata";
                continue;
            }
            const SParquetColumnMetaData& meta = chunk.metaData;
            if ( column.supported && meta.type != column.physicalType )
            {
                location.status = CORE::DATATABLE_STATUS_CORRUPT_DATA;
                location.problem = "the column chunk has physical type " + CORE::ToString( meta.type ) + " where the schema says " + CORE::ToString( column.physicalType );
                continue;
            }
            UInt64 start = (UInt64) meta.dataPageOffset;
            if ( meta.hasDictionaryPageOffset && meta.dictionaryPageOffset > 0 && meta.dictionaryPageOffset < meta.dataPageOffset )
                start = (UInt64) meta.dictionaryPageOffset;
            const UInt64 length = (UInt64) meta.totalCompressedSize;
            location.start = start;
            location.length = length;
            if ( 0 == rowGroup.numRows )
            {
                // nothing of an empty row group is read; writers give its chunks any offsets (pyarrow: 0)
                location.length = 0;
                continue;
            }
            if ( start < 4 || length > m_dataEnd || start > m_dataEnd - length )
            {
                location.status = CORE::DATATABLE_STATUS_CORRUPT_DATA;
                location.problem = "the column chunk at byte " + CORE::ToString( start ) + " of " + CORE::ToString( length ) +
                                   " bytes lies outside the data of the file";
                continue;
            }
            if ( (UInt64) meta.dataPageOffset >= start + length )
            {
                location.status = CORE::DATATABLE_STATUS_CORRUPT_DATA;
                location.problem = "the first data page lies outside the column chunk";
                continue;
            }
            SChunkOrder entry;
            entry.start = start;
            entry.end = start + length;
            entry.index = c;
            order.push_back( entry );
        }

        // chunks of a row group never share bytes in a valid file
        std::sort( order.begin(), order.end() );
        for ( size_t i=1; i<order.size(); ++i )
        {
            if ( order[ i ].start < order[ i - 1 ].end )
            {
                SParquetChunkLocation& a = m_chunkLocations[ g * columnCount + order[ i - 1 ].index ];
                SParquetChunkLocation& b = m_chunkLocations[ g * columnCount + order[ i ].index ];
                a.status = b.status = CORE::DATATABLE_STATUS_CORRUPT_DATA;
                a.problem = b.problem = "column chunks of the row group overlap";
            }
        }

        // the leading sort columns that are ascending with nulls last, the order the table layer knows
        if ( rowGroup.hasSortingColumns )
        {
            for ( size_t s=0; s<rowGroup.sortingColumns.size(); ++s )
            {
                const SParquetSortingColumn& sorting = rowGroup.sortingColumns[ s ];
                if ( sorting.columnIdx < 0 || (UInt32) sorting.columnIdx >= columnCount || sorting.descending || sorting.nullsFirst )
                    break;
                info.sortedOn.push_back( (UInt32) sorting.columnIdx );
            }
        }
    }
    if ( totalRows != (UInt64) m_metaData.numRows )
    {
        error = "The file states " + CORE::ToString( (UInt64) m_metaData.numRows ) + " rows where its row groups hold " + CORE::ToString( totalRows );
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
CParquetFileMetaData::ApplyTableSchema( CORE::CDataTableSchema& schema, CParquetTableSchemaCache* schemaCache ) const
{GUCEF_TRACE;

    if ( !m_keyValues.HasKey( TableSchemaKey ) )
        return;

    // the text is taken from the file: it must be UTF-8 before a JSON parser and the string classes see it
    for ( size_t i=0; i<m_metaData.keyValueMetaData.size(); ++i )
    {
        const SParquetKeyValue& kv = m_metaData.keyValueMetaData[ i ];
        if ( kv.key.Equals( TableSchemaKey ) && !CParquetSchemaMapping::IsValidUtf8( kv.value.data, kv.value.size ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "tablecodecpluginPARQUET: the " << TableSchemaKey << " metadata of a file is not UTF-8, it is ignored" );
            return;
        }
    }
    const CORE::CString text = m_keyValues.GetValueAlways( TableSchemaKey ).AsString();

    // the files of a table carry the same text: parsed once when there is a cache
    CORE::CDataTableSchemaPtr parsedSchema;
    if ( GUCEF_NULL == schemaCache || !schemaCache->Find( text, parsedSchema ) )
    {
        CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
        if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) || json.IsNULL() )
            return;

        // untrusted text: only strict JSON of a bounded nesting reaches the JSON codec, which is lenient and parses
        // whatever it is given; a table schema nests a few levels, 32 leaves ample room
        const UInt32 textLength = (UInt32) strlen( text.C_String() );
        CORE::CString syntaxError;
        if ( !CORE::CJsonSyntaxValidator::IsValid( text.C_String(), textLength, 32, &syntaxError ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "tablecodecpluginPARQUET: the " << TableSchemaKey << " metadata of a file is not JSON, it is ignored: " << syntaxError );
            return;
        }
        CORE::CMFileAccess access( text.C_String(), textLength );
        CORE::CDataNode root;
        CORE::CDataTableSchema parsed;
        CORE::CDataNodeSerializableSettings settings;
        if ( !json->BuildDataTree( &root, &access ) || !parsed.Deserialize( root, settings ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "tablecodecpluginPARQUET: the " << TableSchemaKey << " metadata of a file cannot be read, it is ignored" );
            return;
        }
        parsedSchema = CORE::CreateDataTableSchemaPtr( parsed );
        if ( GUCEF_NULL != schemaCache )
            schemaCache->Put( text, parsedSchema );
    }
    const CORE::CDataTableSchema& tableSchema = *parsedSchema;

    // used only when it describes the same columns, as this codec writes it
    if ( tableSchema.GetColumnCount() != schema.GetColumnCount() )
        return;
    for ( UInt32 i=0; i<schema.GetColumnCount(); ++i )
    {
        const CORE::CDataTableColumnDef& fileColumn = schema.GetColumn( i );
        const CORE::CDataTableColumnDef& tableColumn = tableSchema.GetColumn( i );
        if ( fileColumn.name != tableColumn.name || !fileColumn.IsSameType( tableColumn ) || fileColumn.nullable != tableColumn.nullable )
            return;
    }
    CORE::CDataTableSchema combined;
    for ( UInt32 i=0; i<schema.GetColumnCount(); ++i )
    {
        CORE::CDataTableColumnDef column = schema.GetColumn( i );
        column.description = tableSchema.GetColumn( i ).description;
        column.encodingHint = tableSchema.GetColumn( i ).encodingHint;
        combined.AddColumn( column );
    }
    CORE::CString::StringVector names;
    for ( size_t k=0; k<tableSchema.GetPrimaryKey().size(); ++k )
        names.push_back( tableSchema.GetColumn( tableSchema.GetPrimaryKey()[ k ] ).name );
    combined.SetPrimaryKey( names );
    names.clear();
    for ( size_t k=0; k<tableSchema.GetSortKey().size(); ++k )
        names.push_back( tableSchema.GetColumn( tableSchema.GetSortKey()[ k ] ).name );
    combined.SetSortKey( names );
    combined.SetSchemaVersion( tableSchema.GetSchemaVersion() );
    schema = combined;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableSchemaPtr&
CParquetFileMetaData::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

UInt64
CParquetFileMetaData::GetRowCount( void ) const
{GUCEF_TRACE;

    return (UInt64) m_metaData.numRows;
}

/*-------------------------------------------------------------------------*/

UInt32
CParquetFileMetaData::GetRowGroupCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_metaData.rowGroups.size();
}

/*-------------------------------------------------------------------------*/

bool
CParquetFileMetaData::GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const
{GUCEF_TRACE;

    if ( rowGroup >= m_rowGroupInfos.size() )
    {
        info.Clear();
        return false;
    }
    info = m_rowGroupInfos[ rowGroup ];
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableRowGroupInfo*
CParquetFileMetaData::GetRowGroupInfoPtr( UInt32 rowGroup ) const
{GUCEF_TRACE;

    return rowGroup < m_rowGroupInfos.size() ? &m_rowGroupInfos[ rowGroup ] : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const SParquetChunkLocation*
CParquetFileMetaData::GetChunkLocation( UInt32 rowGroup, UInt32 column ) const
{GUCEF_TRACE;

    const size_t columnCount = m_columns.size();
    if ( rowGroup >= m_metaData.rowGroups.size() || column >= columnCount )
        return GUCEF_NULL;
    return &m_chunkLocations[ rowGroup * columnCount + column ];
}

/*-------------------------------------------------------------------------*/

UInt64
CParquetFileMetaData::GetMemoryFootprint( void ) const
{GUCEF_TRACE;

    UInt64 bytes = sizeof( CParquetFileMetaData ) + ( m_footer.IsNULL() ? 0 : m_footer->GetAllocationSize() ) + m_parsedBytes;
    bytes += m_columns.size() * ( sizeof( SParquetColumnInfo ) + 64 );
    bytes += m_chunkLocations.size() * sizeof( SParquetChunkLocation );
    for ( size_t g=0; g<m_rowGroupInfos.size(); ++g )
        bytes += sizeof( CORE::CDataTableRowGroupInfo ) + m_rowGroupInfos[ g ].columnStats.size() * ( sizeof( CORE::CDataTableColumnStats ) + 32 );
    return bytes;
}

/*-------------------------------------------------------------------------*/

void
CParquetFileMetaData::GetKeyValueMetaData( CORE::CValueList& keyValues ) const
{GUCEF_TRACE;

    keyValues = m_keyValues;
}

/*-------------------------------------------------------------------------*/

bool
CParquetFileMetaData::CanVerifySource( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CParquetFileMetaData::VerifySource( CORE::CIOAccess& source ) const
{GUCEF_TRACE;

    // what Read() reads: the size, the leading magic, the footer and its length and magic. When all are the same, the
    // footer would parse into this object again, whatever else changed in the file (the footer locates every page)
    if ( m_footer.IsNULL() || source.GetSize() != m_fileSize || m_fileSize < (UInt64) m_footerSize + 12 )
        return false;

    UInt8 head[ 4 ];
    if ( 0 != source.Setpos( 0 ) || 4 != source.Read( head, 1, 4 ) || 0 != memcmp( head, "PAR1", 4 ) )
        return false;

    // compared in pieces, no allocation; usually one piece, a footer is a few KiB
    const UInt8* footer = static_cast< const UInt8* >( m_footer->GetData() );
    if ( 0 != source.Setpos( m_dataEnd ) )
        return false;
    UInt8 piece[ 8192 ];
    UInt32 compared = 0;
    while ( compared < m_footerSize )
    {
        const UInt32 wanted = m_footerSize - compared < (UInt32) sizeof( piece ) ? m_footerSize - compared : (UInt32) sizeof( piece );
        if ( wanted != source.Read( piece, 1, wanted ) || 0 != memcmp( piece, footer + compared, wanted ) )
            return false;
        compared += wanted;
    }
    UInt8 tail[ 8 ];
    return 8 == source.Read( tail, 1, 8 ) && m_footerSize == LoadLE32( tail ) && 0 == memcmp( tail + 4, "PAR1", 4 );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace PARQUET */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
