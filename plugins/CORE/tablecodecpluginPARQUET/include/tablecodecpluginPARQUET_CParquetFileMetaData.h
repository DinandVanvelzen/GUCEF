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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#include "gucefCORE_CITableFileMetaData.h"
#define GUCEF_CORE_CITABLEFILEMETADATA_H
#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

#ifndef GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#include "gucefCORE_CDataTableRowGroupInfo.h"
#define GUCEF_CORE_CDATATABLEROWGROUPINFO_H
#endif /* GUCEF_CORE_CDATATABLEROWGROUPINFO_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFER_H
#include "gucefCORE_CDataTableBuffer.h"
#define GUCEF_CORE_CDATATABLEBUFFER_H
#endif /* GUCEF_CORE_CDATATABLEBUFFER_H ? */

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#include "tablecodecpluginPARQUET_CParquetSchemaMapping.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETSCHEMAMAPPING_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETREADOPTIONS_H
#include "tablecodecpluginPARQUET_CParquetReadOptions.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETREADOPTIONS_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETREADOPTIONS_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

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
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Where a column chunk lies in the file, checked once when the footer is read */
struct TABLECODECPLUGIN_PARQUET_PUBLIC_CPP SParquetChunkLocation
{
    UInt64 start;                       /**< the dictionary page, else the first data page */
    UInt64 length;                      /**< total_compressed_size */
    CORE::TDataTableStatus status;      /**< OK, CORRUPT_DATA or UNSUPPORTED for a chunk that cannot be read */
    CORE::CString problem;              /**< why, when the status is not OK */

    SParquetChunkLocation( void );
};

typedef GUCEF::vector< SParquetChunkLocation > TParquetChunkLocationVector;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The table schemas parsed from gucef.table.schema texts: the files of one table carry the same text, so its JSON is
 *  validated and parsed once instead of once per footer. A text is the key, compared in full. Thread safe; the schemas
 *  are shared and immutable. At most MaxEntries texts are kept, the oldest goes first.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetTableSchemaCache
{
    public:

    static const UInt32 MaxEntries = 32;

    CParquetTableSchemaCache( void );
    ~CParquetTableSchemaCache();

    /** The schema parsed from the text before, when it is kept */
    bool Find( const CORE::CString& text, CORE::CDataTableSchemaPtr& schema ) const;

    /** Keeps the schema parsed from the text */
    void Put( const CORE::CString& text, const CORE::CDataTableSchemaPtr& schema );

    UInt32 GetEntryCount( void ) const;
    UInt64 GetHitCount( void ) const;
    UInt64 GetMissCount( void ) const;

    private:

    CParquetTableSchemaCache( const CParquetTableSchemaCache& src );                /**< not implemented */
    CParquetTableSchemaCache& operator=( const CParquetTableSchemaCache& src );     /**< not implemented */

    typedef GUCEF::map< CORE::CString, CORE::CDataTableSchemaPtr > TSchemaMap;
    typedef GUCEF::vector< CORE::CString > TTextVector;

    mutable MT::CMutex m_lock;
    TSchemaMap m_schemas;
    TTextVector m_order;            /**< the texts in the order they were put, for eviction */
    mutable UInt64 m_hits;
    mutable UInt64 m_misses;
};

/*-------------------------------------------------------------------------*/

/**
 *  The parsed footer of a parquet file: immutable once made and shared between threads (the footer cache).
 *
 *  Read footer first: the last 8 bytes give the footer length and the magic PAR1 (PARE is an encrypted footer:
 *  UNSUPPORTED), the first 4 bytes must be PAR1 too, the footer must lie within the file and maxFooterBytes. The parsed
 *  structures point into the footer bytes the object keeps. Row group statistics are made once, here.
 *
 *  Key value metadata are reported as they are. When the file holds gucef.table.schema (the table schema as JSON, written
 *  by this codec) and a JSON codec is registered, the descriptions and encoding hints of the columns, the primary key,
 *  the sort key and the schema version are taken from it, provided its columns match the file's by name and type. With a
 *  CParquetTableSchemaCache a text parsed before is not parsed again.
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetFileMetaData : public CORE::CITableFileMetaData
{
    public:

    /** The key of the table schema in the key value metadata */
    static const char* const TableSchemaKey;

    virtual ~CParquetFileMetaData() GUCEF_VIRTUAL_OVERRIDE;

    /** Reads the footer of the file; the object is made only when the status is OK. The cache is optional */
    static CORE::TDataTableStatus Read( CORE::CIOAccess& source                                 ,
                                        const SParquetReadOptions& options                      ,
                                        CORE::CDataTableBufferPool& pool                        ,
                                        CORE::CITableFileMetaDataPtr& metaData                  ,
                                        CORE::CString& error                                    ,
                                        CParquetTableSchemaCache* schemaCache = GUCEF_NULL      );

    virtual const CORE::CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetKeyValueMetaData( CORE::CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE;

    /** True: the footer bytes are kept */
    virtual bool CanVerifySource( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the source has the size, the leading magic and the footer bytes this object was read from */
    virtual bool VerifySource( CORE::CIOAccess& source ) const GUCEF_VIRTUAL_OVERRIDE;

    inline const SParquetFileMetaData& GetParquetMetaData( void ) const { return m_metaData; }
    inline const TParquetColumnInfoVector& GetColumnInfos( void ) const { return m_columns; }
    inline UInt64 GetFileSize( void ) const { return m_fileSize; }

    /** The first byte of the footer: column chunks end before it */
    inline UInt64 GetDataEnd( void ) const { return m_dataEnd; }

    /** The location of a column chunk; GUCEF_NULL for indices out of range */
    const SParquetChunkLocation* GetChunkLocation( UInt32 rowGroup, UInt32 column ) const;

    /** The row group statistics made from the footer, GUCEF_NULL for a row group out of range */
    const CORE::CDataTableRowGroupInfo* GetRowGroupInfoPtr( UInt32 rowGroup ) const;

    private:

    CParquetFileMetaData( void );
    CParquetFileMetaData( const CParquetFileMetaData& src );                /**< not implemented */
    CParquetFileMetaData& operator=( const CParquetFileMetaData& src );     /**< not implemented */

    CORE::TDataTableStatus Parse( CORE::CString& error, CParquetTableSchemaCache* schemaCache );
    void ApplyTableSchema( CORE::CDataTableSchema& schema, CParquetTableSchemaCache* schemaCache ) const;

    typedef GUCEF::vector< CORE::CDataTableRowGroupInfo > TRowGroupInfoVector;

    CORE::CDataTableBufferPtr m_footer;
    UInt32 m_footerSize;
    UInt64 m_fileSize;
    UInt64 m_dataEnd;
    SParquetReadOptions m_options;
    SParquetFileMetaData m_metaData;
    UInt64 m_parsedBytes;
    TParquetColumnInfoVector m_columns;
    CORE::CDataTableSchemaPtr m_schema;
    TRowGroupInfoVector m_rowGroupInfos;
    TParquetChunkLocationVector m_chunkLocations;      /**< row group major */
    CORE::CValueList m_keyValues;
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

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H ? */
