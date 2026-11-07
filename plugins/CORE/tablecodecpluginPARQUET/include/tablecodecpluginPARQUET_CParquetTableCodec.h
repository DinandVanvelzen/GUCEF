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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_MACROS_H
#include "tablecodecpluginPARQUET_macros.h"
#define TABLECODECPLUGIN_PARQUET_MACROS_H
#endif /* TABLECODECPLUGIN_PARQUET_MACROS_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#include "tablecodecpluginPARQUET_CParquetFileMetaData.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETFILEMETADATA_H ? */

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
 *  The parquet table codec, type "parquet", file extension parquet: a GUCEF implementation of the subset of Apache
 *  Parquet in section 13 of the data tables design (no Arrow, no Thrift library). Shared; every member is reentrant.
 *
 *  Read options (the CValueList of ReadMetaData() and CreateReader()): maxFooterBytes, maxMetaDataBytes, maxPageBytes,
 *  maxChunkBytes, ioCoalesceGapBytes and validateUtf8, see SParquetReadOptions. Writer options: CDataTableWriterOptions;
 *  the compression is a codec of the CompressionCodec family named after the parquet codec ("gzip", ...).
 */
class TABLECODECPLUGIN_PARQUET_PUBLIC_CPP CParquetTableCodec : public CORE::CITableCodec
{
    public:

    static const char* const TypeName;

    CParquetTableCodec( void );

    virtual ~CParquetTableCodec() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetFileExtensions( CORE::CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ReadMetaData( CORE::CIOAccess& source, const CORE::CValueList& options,
                                                 CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The metadata must be NULL (it is read then) or have been made by this codec */
    virtual CORE::TDataTableStatus CreateReader( const CORE::IOAccessPtr& source, const CORE::CITableFileMetaDataPtr& metaData,
                                                 const CORE::CValueList& options, CORE::CITableReaderPtr& reader, CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus CreateWriter( const CORE::IOAccessPtr& destination, const CORE::CDataTableSchemaPtr& schema,
                                                 const CORE::CDataTableWriterOptions& options, CORE::CITableWriterPtr& writer, CORE::CString& error ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The table schemas of the footers read by this codec, parsed once per text */
    const CParquetTableSchemaCache& GetTableSchemaCache( void ) const;

    private:

    CParquetTableCodec( const CParquetTableCodec& src );                /**< not implemented: Clone() makes a new codec */
    CParquetTableCodec& operator=( const CParquetTableCodec& src );     /**< not implemented */

    mutable CParquetTableSchemaCache m_tableSchemaCache;
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

#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H ? */
