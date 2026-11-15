/*
 *  dbpluginPOSTGRESQL: gucefCOMCORE database backend for PostgreSQL
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#include "gucefCORE_CDataTableBufferPool.h"
#define GUCEF_CORE_CDATATABLEBUFFERPOOL_H
#endif /* GUCEF_CORE_CDATATABLEBUFFERPOOL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#include "dbpluginPOSTGRESQL_CPostgreSqlValues.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLVALUES_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlTableReader.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPostgreSqlTableReader::CPostgreSqlTableReader( const CPostgreSqlConnectionPtr& connection             ,
                                                const CORE::CDataTableDefinition& definition           ,
                                                const CORE::CDataTableSchemaPtr& schema                ,
                                                const CORE::CDataTableResolvedQuery& query             ,
                                                const CORE::CITableFileMetaDataPtr& metaData           )
    : CORE::CITableReader()
    , m_connection( connection )
    , m_definition( definition )
    , m_schema( schema )
    , m_query( query )
    , m_metaData( metaData )
    , m_request( GUCEF_NULL )
    , m_projection()
    , m_projectedSchema()
    , m_builder( GUCEF_NULL )
    , m_result()
    , m_position( 0 )
    , m_error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPostgreSqlTableReader::~CPostgreSqlTableReader()
{GUCEF_TRACE;

    GUCEF_DELETE m_builder;
    m_builder = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CITableFileMetaDataPtr&
CPostgreSqlTableReader::GetMetaData( void ) const
{GUCEF_TRACE;

    return m_metaData;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlTableReader::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlTableReader::OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request )
{GUCEF_TRACE;

    m_request = GUCEF_NULL;
    GUCEF_DELETE m_builder;
    m_builder = GUCEF_NULL;
    m_result.Reset();
    m_position = 0;

    if ( 0 != rowGroup )
    {
        m_error = "Row group " + CORE::ToString( rowGroup ) + " does not exist: a query result is one row group";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    if ( GUCEF_NULL == request.bufferPool )
    {
        m_error = "The read request has no buffer pool";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }

    // the columns the scan asks for, as a schema of their own for the builder
    CORE::CDataTableSchema projected;
    const CORE::CDataTableSchema& schema = *m_schema;
    for ( UInt32 i=0; i<(UInt32) request.projection.size(); ++i )
    {
        if ( request.projection[ i ] >= schema.GetColumnCount() || !projected.AddColumn( schema.GetColumn( request.projection[ i ] ) ) )
        {
            m_error = "The projection names column " + CORE::ToString( request.projection[ i ] ) + ", which cannot be read";
            return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
        }
    }
    m_projection = request.projection;
    m_projectedSchema = CORE::CreateDataTableSchemaPtr( projected );

    CPostgreSqlParameters parameters;
    CORE::CString sql;
    CORE::TDataTableStatus status = CPostgreSqlSql::MakeSelect( m_definition, m_connection->GetSettings().schema, m_projection, m_query, parameters, sql, m_error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    {
        MT::CScopeMutex lock( m_connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = m_connection->Execute( sql, parameters, m_result, m_error );
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        m_error = "Reading table \"" + m_definition.name + "\" failed: " + m_error;
        m_result.Reset();
        return status;
    }
    if ( !m_projection.empty() && m_result.GetColumnCount() != (UInt32) m_projection.size() )
    {
        m_error = "The server returned " + CORE::ToString( m_result.GetColumnCount() ) + " columns for " + CORE::ToString( (UInt32) m_projection.size() );
        m_result.Reset();
        return CORE::DATATABLE_STATUS_CORRUPT_DATA;
    }
    m_builder = GUCEF_NEW CORE::CDataTableBatchBuilder( m_projectedSchema, *request.bufferPool );
    m_request = &request;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlTableReader::ReadNextBatch( CORE::CDataTableBatchPtr& batch )
{GUCEF_TRACE;

    batch.Reset();
    if ( GUCEF_NULL == m_request || GUCEF_NULL == m_builder )
    {
        m_error = "No row group is open";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    const UInt32 rowCount = m_result.GetRowCount();
    if ( m_position >= rowCount )
        return CORE::DATATABLE_STATUS_END;
    UInt32 count = rowCount - m_position;
    if ( count > m_request->batchRowCount )
        count = m_request->batchRowCount;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    const UInt32 columnCount = (UInt32) m_projection.size();
    if ( 0 == columnCount )
    {
        // no column asked for: only the number of rows
        status = m_request->bufferPool->AcquireBatch( batch );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        CORE::CDataTableBatch* writable = batch.GetWritable();
        writable->SetColumnCount( 0 );
        writable->SetSchema( m_schema );
        writable->SetRowCount( count );
    }
    else
    {
        const CORE::CDataTableSchema& projected = *m_projectedSchema;
        for ( UInt32 c=0; c<columnCount; ++c )
        {
            const CORE::CDataTableColumnDef& column = projected.GetColumn( c );
            CORE::CDataTableColumnBuilder& target = m_builder->GetColumn( c );
            for ( UInt32 r=m_position; r<m_position+count; ++r )
            {
                status = CPostgreSqlValues::AppendResultValue( column, m_result.GetValue( r, c ), m_result.GetLength( r, c ), m_result.IsNull( r, c ), target );
                if ( CORE::DATATABLE_STATUS_OK != status )
                {
                    m_error = "Table \"" + m_definition.name + "\", column \"" + column.name + "\": the value of row " + CORE::ToString( r ) +
                              " cannot be held by the column (" + CORE::DataTableStatusToString( status ) + ")";
                    m_builder->Reset();
                    return status;
                }
            }
        }
        status = m_builder->Build( batch, m_error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        CORE::CDataTableBatch* writable = batch.GetWritable();
        writable->SetSchema( m_schema );
        for ( UInt32 c=0; c<columnCount; ++c )
            writable->SetSchemaColumnIndex( c, m_projection[ c ] );
    }

    CORE::CDataTableBatchOrigin origin;
    origin.fileOrdinal = m_request->fileOrdinal;
    origin.rowGroup = 0;
    origin.firstRowInRowGroup = m_position;
    batch.GetWritable()->SetOrigin( origin );
    m_position += count;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
