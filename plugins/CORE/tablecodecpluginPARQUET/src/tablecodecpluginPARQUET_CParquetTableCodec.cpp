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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLEREADER_H
#include "tablecodecpluginPARQUET_CParquetTableReader.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLEREADER_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLEREADER_H ? */

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H
#include "tablecodecpluginPARQUET_CParquetTableWriter.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLEWRITER_H ? */

#include "tablecodecpluginPARQUET_CParquetTableCodec.h"

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

const char* const CParquetTableCodec::TypeName = "parquet";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CParquetTableCodec::CParquetTableCodec( void )
    : CORE::CITableCodec()
    , m_tableSchemaCache()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CParquetTableCodec::~CParquetTableCodec()
{GUCEF_TRACE;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetTableCodec::GetTypeName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetTableCodec::GetName( void ) const
{GUCEF_TRACE;

    return "Apache Parquet";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CParquetTableCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*-------------------------------------------------------------------------*/

CORE::TVersion
CParquetTableCodec::GetVersion( void ) const
{GUCEF_TRACE;

    CORE::TVersion version;
    version.major = 1;
    version.minor = 0;
    version.patch = 0;
    version.release = 0;
    return version;
}

/*-------------------------------------------------------------------------*/

UInt32
CParquetTableCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_TABLECODEC_CAPABILITY_READ | GUCEF_TABLECODEC_CAPABILITY_WRITE | GUCEF_TABLECODEC_CAPABILITY_ROW_GROUPS |
           GUCEF_TABLECODEC_CAPABILITY_STATISTICS | GUCEF_TABLECODEC_CAPABILITY_RANDOM_ACCESS | GUCEF_TABLECODEC_CAPABILITY_EMBEDDED_SCHEMA |
           GUCEF_TABLECODEC_CAPABILITY_IN_PLACE_PAGES;
}

/*-------------------------------------------------------------------------*/

void
CParquetTableCodec::GetFileExtensions( CORE::CString::StringSet& extensions ) const
{GUCEF_TRACE;

    extensions.insert( "parquet" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableCodec::ReadMetaData( CORE::CIOAccess& source, const CORE::CValueList& options,
                                  CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error ) const
{GUCEF_TRACE;

    metaData.Unlink();
    SParquetReadOptions readOptions;
    if ( !readOptions.Parse( options, error ) )
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    return CParquetFileMetaData::Read( source, readOptions, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool(), metaData, error, &m_tableSchemaCache );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableCodec::CreateReader( const CORE::IOAccessPtr& source, const CORE::CITableFileMetaDataPtr& metaData,
                                  const CORE::CValueList& options, CORE::CITableReaderPtr& reader, CORE::CString& error ) const
{GUCEF_TRACE;

    reader.Unlink();
    if ( source.IsNULL() )
    {
        error = "No source to read from";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    SParquetReadOptions readOptions;
    if ( !readOptions.Parse( options, error ) )
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;

    CORE::CITableFileMetaDataPtr fileMetaData = metaData;
    if ( fileMetaData.IsNULL() )
    {
        CORE::IOAccessPtr mutableSource = source;
        CORE::TDataTableStatus status = CParquetFileMetaData::Read( *mutableSource, readOptions, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool(), fileMetaData, error, &m_tableSchemaCache );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }

    // the reader needs the parsed footer of this codec, not only the generic interface
    const CParquetFileMetaData* parquetMetaData = GUCEF_NULL;
    #ifdef GUCEF_RTTI_IS_SUPPORTED
    parquetMetaData = dynamic_cast< const CParquetFileMetaData* >( fileMetaData.GetPointerAlways() );
    if ( GUCEF_NULL == parquetMetaData )
    {
        error = "The file metadata was not made by the parquet codec";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    #else
    // without RTTI the caller is trusted to pass metadata this codec made, as the interface asks
    parquetMetaData = static_cast< const CParquetFileMetaData* >( fileMetaData.GetPointerAlways() );
    #endif

    reader = CORE::CITableReaderPtr( GUCEF_NEW CParquetTableReader( source, fileMetaData, *parquetMetaData, readOptions ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CParquetTableCodec::CreateWriter( const CORE::IOAccessPtr& destination, const CORE::CDataTableSchemaPtr& schema,
                                  const CORE::CDataTableWriterOptions& options, CORE::CITableWriterPtr& writer, CORE::CString& error ) const
{GUCEF_TRACE;

    writer.Unlink();
    CParquetTableWriter* parquetWriter = GUCEF_NEW CParquetTableWriter( destination, schema, options );
    CORE::CITableWriterPtr result( parquetWriter );
    CORE::TDataTableStatus status = parquetWriter->Init( error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    writer = result;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CParquetTableCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CParquetTableCodec();
}

/*-------------------------------------------------------------------------*/

const CParquetTableSchemaCache&
CParquetTableCodec::GetTableSchemaCache( void ) const
{GUCEF_TRACE;

    return m_tableSchemaCache;
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
