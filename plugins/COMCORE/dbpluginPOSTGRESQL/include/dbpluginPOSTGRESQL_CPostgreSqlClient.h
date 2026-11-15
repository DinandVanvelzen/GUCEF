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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTBASE_H
#include "gucefCOMCORE_CDatabaseClientBase.h"
#define GUCEF_COMCORE_CDATABASECLIENTBASE_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTBASE_H ? */

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
 *  The "postgresql" database backend: data tables as tables of a PostgreSQL database, through libpq. General purpose,
 *  not tied to one application. Settings: CPostgreSqlSettings.
 *
 *  The tables of a client are those it made: their definitions (with the index hints) are kept as JSON in the table
 *  "_gucef_tables" of the schema (made at the first read-write open), which needs the JSON DStore codec at run time.
 *  CreateTable makes the table with its columns, primary key and the indexes of the hints, records the definition and
 *  takes over a table of the name that exists already when it has every column of the definition (a table dropped
 *  earlier without its data); partitioned definitions are refused with UNSUPPORTED. DropTable forgets the definition,
 *  and with deleteData drops the table as well.
 *
 *  One connection per client, shared with its tables; statements are serialized on it. Transactions as
 *  CIDatabaseClient describes them: the connection is in the transaction, so every change through the client and its
 *  tables belongs to it, also the creation and the dropping of tables. The async calls run the sync calls on a pool
 *  (CDatabaseClientBase).
 *
 *  Capabilities: WRITE (not for a read only open), PREDICATE_PUSHDOWN, ASYNC, TRANSACTIONS, INDEXES.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlClient : public COMCORE::CDatabaseClientBase
{
    public:

    static const CORE::CString BackendName;

    CPostgreSqlClient( void );

    /** Waits for the async calls that are queued or running, then closes */
    virtual ~CPostgreSqlClient() GUCEF_VIRTUAL_OVERRIDE;

    /** Connects; a read-write open makes the schema (createSchema) and the catalogue table when they are not there */
    virtual bool Open( const COMCORE::CDatabaseClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    /** Rolls back an open transaction and disconnects */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus ListTables( CORE::CString::StringSet& tableNames ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus DropTable( const CORE::CString& tableName, bool deleteData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus BeginTransaction( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus CommitTransaction( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::TDataTableStatus RollbackTransaction( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInTransaction( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The connection of the open client, NULL when it is not open */
    CPostgreSqlConnectionPtr GetConnection( void ) const;

    /** Clients and tables of the module that are alive, described for the log of an unload that finds them */
    static void GetLiveObjects( CORE::CString::StringVector& descriptions );

    /** Bookkeeping of the module: a client or table came or went */
    static void OnObjectCreated( const CORE::CString& description );
    static void OnObjectDestroyed( const CORE::CString& description );

    private:

    typedef GUCEF::map< TThreadId, CORE::CString > TErrorMap;

    CPostgreSqlClient( const CPostgreSqlClient& src );              /**< not implemented */
    CPostgreSqlClient& operator=( const CPostgreSqlClient& src );   /**< not implemented */

    CORE::TDataTableStatus Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const;
    CORE::TDataTableStatus Succeed( void ) const;
    CPostgreSqlConnectionPtr GetOpenConnection( CORE::TDataTableStatus& status ) const;
    void CloseConnection( bool notify );
    CORE::TDataTableStatus LoadDefinition( CPostgreSqlConnection& connection, const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) const;
    CORE::TDataTableStatus CreateTableLocked( CPostgreSqlConnection& connection, const CORE::CDataTableDefinition& definition, CORE::CString& error ) const;

    mutable MT::CMutex m_lock;
    CPostgreSqlConnectionPtr m_connection;
    bool m_readOnly;
    mutable TErrorMap m_errors;             /**< per thread */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDatabaseClient, CPostgreSqlClient, MT::CMutex > TPostgreSqlClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */
