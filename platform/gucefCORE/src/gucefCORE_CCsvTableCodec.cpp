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

#ifndef GUCEF_CORE_CCSVTABLEOPTIONS_H
#include "gucefCORE_CCsvTableOptions.h"
#define GUCEF_CORE_CCSVTABLEOPTIONS_H
#endif /* GUCEF_CORE_CCSVTABLEOPTIONS_H ? */

#ifndef GUCEF_CORE_CCSVSCHEMARESOLVER_H
#include "gucefCORE_CCsvSchemaResolver.h"
#define GUCEF_CORE_CCSVSCHEMARESOLVER_H
#endif /* GUCEF_CORE_CCSVSCHEMARESOLVER_H ? */

#ifndef GUCEF_CORE_CCSVTABLEREADER_H
#include "gucefCORE_CCsvTableReader.h"
#define GUCEF_CORE_CCSVTABLEREADER_H
#endif /* GUCEF_CORE_CCSVTABLEREADER_H ? */

#ifndef GUCEF_CORE_CCSVTABLEWRITER_H
#include "gucefCORE_CCsvTableWriter.h"
#define GUCEF_CORE_CCSVTABLEWRITER_H
#endif /* GUCEF_CORE_CCSVTABLEWRITER_H ? */

#include "gucefCORE_CCsvTableCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CCsvTableCodec::CCsvTableCodec( void )
    : CITableCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CCsvTableCodec::~CCsvTableCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CString
CCsvTableCodec::GetTypeName( void ) const
{GUCEF_TRACE;

    return "csv";
}

/*-------------------------------------------------------------------------*/

CString
CCsvTableCodec::GetName( void ) const
{GUCEF_TRACE;

    return "CSV (RFC 4180)";
}

/*-------------------------------------------------------------------------*/

CString
CCsvTableCodec::GetCopyright( void ) const
{GUCEF_TRACE;

    return "Copyright (C) Dinand Vanvelzen, Apache License v2";
}

/*-------------------------------------------------------------------------*/

TVersion
CCsvTableCodec::GetVersion( void ) const
{GUCEF_TRACE;

    TVersion version;
    version.major = 1;
    version.minor = 0;
    version.patch = 0;
    version.release = 0;
    return version;
}

/*-------------------------------------------------------------------------*/

UInt32
CCsvTableCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    // The reader needs Setpos() to infer over the whole file and to start again; the file does not state its column types
    return GUCEF_TABLECODEC_CAPABILITY_READ | GUCEF_TABLECODEC_CAPABILITY_WRITE | GUCEF_TABLECODEC_CAPABILITY_RANDOM_ACCESS;
}

/*-------------------------------------------------------------------------*/

void
CCsvTableCodec::GetFileExtensions( CString::StringSet& extensions ) const
{GUCEF_TRACE;

    extensions.insert( "csv" );
    extensions.insert( "tsv" );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableCodec::ReadMetaData( CIOAccess& source, const CValueList& options, CITableFileMetaDataPtr& metaData, CString& error ) const
{GUCEF_TRACE;

    metaData.Unlink();
    SCsvReadOptions readOptions;
    if ( !readOptions.Parse( options, error ) )
        return DATATABLE_STATUS_INVALID_ARGUMENT;

    return CCsvSchemaResolver::Resolve( source, IOAccessPtr(), CCoreGlobal::Instance()->GetDataTableBufferPool(), readOptions, GUCEF_NULL, metaData, error );
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableCodec::CreateReader( const IOAccessPtr& source, const CITableFileMetaDataPtr& metaData, const CValueList& options,
                              CITableReaderPtr& reader, CString& error ) const
{GUCEF_TRACE;

    reader.Unlink();
    if ( source.IsNULL() )
    {
        error = "No source to read from";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    SCsvReadOptions readOptions;
    if ( !readOptions.Parse( options, error ) )
        return DATATABLE_STATUS_INVALID_ARGUMENT;

    CITableFileMetaDataPtr fileMetaData = metaData;
    if ( fileMetaData.IsNULL() )
    {
        IOAccessPtr mutableSource = source;
        TDataTableStatus status = CCsvSchemaResolver::Resolve( *mutableSource, source, CCoreGlobal::Instance()->GetDataTableBufferPool(), readOptions, GUCEF_NULL, fileMetaData, error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }
    else
    if ( fileMetaData->GetSchema().IsNULL() || 0 == fileMetaData->GetSchema()->GetColumnCount() )
    {
        error = "The file metadata has no schema";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    reader = CITableReaderPtr( GUCEF_NEW CCsvTableReader( source, fileMetaData, readOptions ) );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
CCsvTableCodec::CreateWriter( const IOAccessPtr& destination, const CDataTableSchemaPtr& schema, const CDataTableWriterOptions& options,
                              CITableWriterPtr& writer, CString& error ) const
{GUCEF_TRACE;

    writer.Unlink();
    if ( destination.IsNULL() || schema.IsNULL() )
    {
        error = "A destination and a schema are needed";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    if ( 0 == schema->GetColumnCount() )
    {
        error = "A CSV file needs at least one column";
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    for ( UInt32 i = 0; i < schema->GetColumnCount(); ++i )
    {
        if ( DATATABLE_TYPE_UNSUPPORTED == schema->GetColumn( i ).type )
        {
            error = "Column \"" + schema->GetColumn( i ).name + "\" cannot be written";
            return DATATABLE_STATUS_UNSUPPORTED;
        }
    }

    SCsvWriteOptions writeOptions;
    if ( !writeOptions.Parse( options, error ) )
        return DATATABLE_STATUS_INVALID_ARGUMENT;

    writer = CITableWriterPtr( GUCEF_NEW CCsvTableWriter( destination, schema, writeOptions ) );
    return DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CICloneable*
CCsvTableCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CCsvTableCodec();
}

/*-------------------------------------------------------------------------*/

void
CCsvTableCodec::SetSchemaOption( CValueList& options, const CDataTableSchema& schema )
{GUCEF_TRACE;

    options.Delete( "schema" );
    for ( UInt32 i = 0; i < schema.GetColumnCount(); ++i )
    {
        const CDataTableColumnDef& column = schema.GetColumn( i );
        AddCsvOptionValue( options, "schema", column.name + "|" + column.GetTypeText() + ( column.nullable ? "" : "!" ) );
    }
}

/*-------------------------------------------------------------------------*/

void
CCsvTableCodec::AddListOption( CValueList& options, const CString& key, const CString& value )
{GUCEF_TRACE;

    AddCsvOptionValue( options, key, value );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
