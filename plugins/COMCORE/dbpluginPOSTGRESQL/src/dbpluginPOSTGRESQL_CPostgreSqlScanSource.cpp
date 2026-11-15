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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULTMETADATA_H
#include "dbpluginPOSTGRESQL_CPostgreSqlResultMetaData.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULTMETADATA_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULTMETADATA_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLTABLEREADER_H
#include "dbpluginPOSTGRESQL_CPostgreSqlTableReader.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLTABLEREADER_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLTABLEREADER_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlScanSource.h"

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

CPostgreSqlScanSource::CPostgreSqlScanSource( const CPostgreSqlConnectionPtr& connection     ,
                                              const CORE::CDataTableDefinition& definition   ,
                                              const CORE::CDataTableSchemaPtr& schema        ,
                                              const CORE::CDataTableResolvedQuery& query     )
    : CORE::CIDataTableScanSource()
    , m_connection( connection )
    , m_definition( definition )
    , m_schema( schema )
    , m_query( query )
    , m_metaData()
    , m_listed( false )
{GUCEF_TRACE;

    CORE::CTSharedPtr< CPostgreSqlResultMetaData, MT::CMutex > metaData( GUCEF_NEW CPostgreSqlResultMetaData( schema ) );
    m_metaData = CORE::CITableFileMetaDataPtr( metaData );
}

/*-------------------------------------------------------------------------*/

CPostgreSqlScanSource::~CPostgreSqlScanSource()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlScanSource::NextFile( CORE::CDataTableScanFile& file )
{GUCEF_TRACE;

    if ( m_listed || m_query.matchesNothing )
        return CORE::DATATABLE_STATUS_END;
    m_listed = true;
    file.Clear();
    file.path = m_connection->GetSettings().schema + "." + m_definition.name;
    file.fileSize = GUCEF_UINT64MAX;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlScanSource::OpenFile( const CORE::CDataTableScanFile& file, CORE::CITableReaderPtr& reader )
{GUCEF_TRACE;

    reader = CORE::CITableReaderPtr( GUCEF_NEW CPostgreSqlTableReader( m_connection, m_definition, m_schema, m_query, m_metaData ) );
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
