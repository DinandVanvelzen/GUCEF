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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLTABLEREADER_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLTABLEREADER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEREADER_H
#include "gucefCORE_CITableReader.h"
#define GUCEF_CORE_CITABLEREADER_H
#endif /* GUCEF_CORE_CITABLEREADER_H ? */

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
namespace CORE { class CDataTableBatchBuilder; }
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Reads the one row group of a scan of a table: OpenRowGroup() runs the SELECT of the columns the scan asks for, with
 *  the predicates of the query as its WHERE clause, and keeps the rows of the server; ReadNextBatch() turns them into
 *  batches. The rows of a scan are held in memory while it is read: the predicates and the limit of the query bound
 *  them. Used by one thread at a time.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlTableReader : public CORE::CITableReader
{
    public:

    CPostgreSqlTableReader( const CPostgreSqlConnectionPtr& connection             ,
                            const CORE::CDataTableDefinition& definition           ,
                            const CORE::CDataTableSchemaPtr& schema                ,
                            const CORE::CDataTableResolvedQuery& query             ,
                            const CORE::CITableFileMetaDataPtr& metaData           );

    virtual ~CPostgreSqlTableReader() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CITableFileMetaDataPtr& GetMetaData( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OpenRowGroup( UInt32 rowGroup, const CORE::CDataTableReadRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ReadNextBatch( CORE::CDataTableBatchPtr& batch ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CPostgreSqlTableReader( const CPostgreSqlTableReader& src );              /**< not implemented */
    CPostgreSqlTableReader& operator=( const CPostgreSqlTableReader& src );   /**< not implemented */

    CPostgreSqlConnectionPtr m_connection;
    CORE::CDataTableDefinition m_definition;
    CORE::CDataTableSchemaPtr m_schema;
    CORE::CDataTableResolvedQuery m_query;
    CORE::CITableFileMetaDataPtr m_metaData;
    const CORE::CDataTableReadRequest* m_request;
    CORE::CDataTableSchema::TColumnIndexVector m_projection;
    CORE::CDataTableSchemaPtr m_projectedSchema;
    CORE::CDataTableBatchBuilder* m_builder;
    CPostgreSqlResult m_result;
    UInt32 m_position;
    CORE::CString m_error;
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

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLTABLEREADER_H ? */
