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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULTMETADATA_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULTMETADATA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CITABLEFILEMETADATA_H
#include "gucefCORE_CITableFileMetaData.h"
#define GUCEF_CORE_CITABLEFILEMETADATA_H
#endif /* GUCEF_CORE_CITABLEFILEMETADATA_H ? */

#ifndef DBPLUGIN_POSTGRESQL_MACROS_H
#include "dbpluginPOSTGRESQL_macros.h"
#define DBPLUGIN_POSTGRESQL_MACROS_H
#endif /* DBPLUGIN_POSTGRESQL_MACROS_H ? */

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
 *  What a scan knows of the result of a query before it is read: the schema of the table, one row group, no row count
 *  and no statistics (the server pruned with the predicates already)
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlResultMetaData : public CORE::CITableFileMetaData
{
    public:

    explicit CPostgreSqlResultMetaData( const CORE::CDataTableSchemaPtr& schema );

    virtual ~CPostgreSqlResultMetaData() GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CDataTableSchemaPtr& GetSchema( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Not known before the query ran: GUCEF_UINT64MAX */
    virtual UInt64 GetRowCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetRowGroupCount( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** False: there are no statistics to prune with */
    virtual bool GetRowGroupInfo( UInt32 rowGroup, CORE::CDataTableRowGroupInfo& info ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt64 GetMemoryFootprint( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void GetKeyValueMetaData( CORE::CValueList& keyValues ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CORE::CDataTableSchemaPtr m_schema;
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

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULTMETADATA_H ? */
