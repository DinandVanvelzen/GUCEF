/*
 *  tablecodecpluginXLSX: GUCEF table codec plugin for Microsoft Excel (.xlsx) workbooks
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

#ifndef TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#include "tablecodecpluginXLSX_CXlsxReadOptions.h"
#define TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXREADOPTIONS_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H
#include "tablecodecpluginXLSX_CXlsxSchemaResolver.h"
#define TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXSCHEMARESOLVER_H ? */

#ifndef TABLECODECPLUGIN_XLSX_CXLSXTABLEREADER_H
#include "tablecodecpluginXLSX_CXlsxTableReader.h"
#define TABLECODECPLUGIN_XLSX_CXLSXTABLEREADER_H
#endif /* TABLECODECPLUGIN_XLSX_CXLSXTABLEREADER_H ? */

#include "tablecodecpluginXLSX_CXlsxTableCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace TABLECODECPLUGIN {
namespace XLSX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* const CXlsxTableCodec::TypeName = "xlsx";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CXlsxTableCodec::CXlsxTableCodec( void )
    : CORE::CITableCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CXlsxTableCodec::~CXlsxTableCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CXlsxTableCodec::GetTypeName( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CXlsxTableCodec::GetName( void ) const
{GUCEF_TRACE;

    return "Microsoft Excel workbook (xlsx, read only)";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CXlsxTableCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*-------------------------------------------------------------------------*/

CORE::TVersion
CXlsxTableCodec::GetVersion( void ) const
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
CXlsxTableCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    // read only; the source is read from its start (the types are inferred: the file does not state them as a table)
    return GUCEF_TABLECODEC_CAPABILITY_READ | GUCEF_TABLECODEC_CAPABILITY_RANDOM_ACCESS;
}

/*-------------------------------------------------------------------------*/

void
CXlsxTableCodec::GetFileExtensions( CORE::CString::StringSet& extensions ) const
{GUCEF_TRACE;

    extensions.insert( "xlsx" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxTableCodec::ReadMetaData( CORE::CIOAccess& source, const CORE::CValueList& options,
                               CORE::CITableFileMetaDataPtr& metaData, CORE::CString& error ) const
{GUCEF_TRACE;

    metaData.Unlink();
    SXlsxReadOptions readOptions;
    if ( !readOptions.Parse( options, error ) )
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    return CXlsxSchemaResolver::Resolve( source, readOptions, metaData, error );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxTableCodec::CreateReader( const CORE::IOAccessPtr& source, const CORE::CITableFileMetaDataPtr& metaData,
                               const CORE::CValueList& options, CORE::CITableReaderPtr& reader, CORE::CString& error ) const
{GUCEF_TRACE;

    reader.Unlink();
    if ( source.IsNULL() )
    {
        error = "No source to read from";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    SXlsxReadOptions readOptions;
    if ( !readOptions.Parse( options, error ) )
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;

    CORE::CITableFileMetaDataPtr fileMetaData = metaData;
    if ( fileMetaData.IsNULL() )
    {
        CORE::IOAccessPtr mutableSource = source;
        CORE::TDataTableStatus status = CXlsxSchemaResolver::Resolve( *mutableSource, readOptions, fileMetaData, error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    else
    if ( fileMetaData->GetSchema().IsNULL() || 0 == fileMetaData->GetSchema()->GetColumnCount() )
    {
        error = "The file metadata has no schema";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    #ifdef GUCEF_RTTI_IS_SUPPORTED
    if ( GUCEF_NULL == dynamic_cast< const CXlsxFileMetaData* >( fileMetaData.GetPointerAlways() ) )
    {
        error = "The file metadata was not made by the xlsx codec";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    #endif

    reader = CORE::CITableReaderPtr( GUCEF_NEW CXlsxTableReader( source, fileMetaData, readOptions ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CXlsxTableCodec::CreateWriter( const CORE::IOAccessPtr& destination, const CORE::CDataTableSchemaPtr& schema,
                               const CORE::CDataTableWriterOptions& options, CORE::CITableWriterPtr& writer, CORE::CString& error ) const
{GUCEF_TRACE;

    writer.Unlink();
    error = "The xlsx codec only reads workbooks: it does not write them";
    return CORE::DATATABLE_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CXlsxTableCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CXlsxTableCodec();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace XLSX */
}; /* namespace TABLECODECPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
