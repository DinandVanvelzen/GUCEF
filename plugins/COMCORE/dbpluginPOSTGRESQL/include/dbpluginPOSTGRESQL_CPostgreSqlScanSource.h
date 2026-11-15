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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSCANSOURCE_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSCANSOURCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLESCANSOURCE_H
#include "gucefCORE_CIDataTableScanSource.h"
#define GUCEF_CORE_CIDATATABLESCANSOURCE_H
#endif /* GUCEF_CORE_CIDATATABLESCANSOURCE_H ? */

#ifndef GUCEF_CORE_CDATATABLEDEFINITION_H
#include "gucefCORE_CDataTableDefinition.h"
#define GUCEF_CORE_CDATATABLEDEFINITION_H
#endif /* GUCEF_CORE_CDATATABLEDEFINITION_H ? */

#ifndef GUCEF_CORE_CDATATABLEQUERY_H
#include "gucefCORE_CDataTableQuery.h"
#define GUCEF_CORE_CDATATABLEQUERY_H
#endif /* GUCEF_CORE_CDATATABLEQUERY_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a CDataTableScan of a PostgreSQL table reads: one "file", the result of the query on the server, read by a
 *  CPostgreSqlTableReader. A query that matches nothing lists no file and sends nothing to the server.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlScanSource : public CORE::CIDataTableScanSource
{
    public:

    CPostgreSqlScanSource( const CPostgreSqlConnectionPtr& connection     ,
                           const CORE::CDataTableDefinition& definition   ,
                           const CORE::CDataTableSchemaPtr& schema        ,
                           const CORE::CDataTableResolvedQuery& query     );

    virtual ~CPostgreSqlScanSource() GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus NextFile( CORE::CDataTableScanFile& file ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OpenFile( const CORE::CDataTableScanFile& file, CORE::CITableReaderPtr& reader ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPostgreSqlScanSource( const CPostgreSqlScanSource& src );              /**< not implemented */
    CPostgreSqlScanSource& operator=( const CPostgreSqlScanSource& src );   /**< not implemented */

    CPostgreSqlConnectionPtr m_connection;
    CORE::CDataTableDefinition m_definition;
    CORE::CDataTableSchemaPtr m_schema;
    CORE::CDataTableResolvedQuery m_query;
    CORE::CITableFileMetaDataPtr m_metaData;
    bool m_listed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSCANSOURCE_H ? */
