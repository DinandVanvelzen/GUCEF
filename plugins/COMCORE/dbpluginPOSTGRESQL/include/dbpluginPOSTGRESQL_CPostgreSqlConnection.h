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

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_DATATABLETYPES_H
#include "gucefCORE_DataTableTypes.h"
#define GUCEF_CORE_DATATABLETYPES_H
#endif /* GUCEF_CORE_DATATABLETYPES_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlParameters.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLPARAMETERS_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlResult.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLRESULT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct pg_conn;

namespace GUCEF {
namespace DBPLUGIN {
namespace POSTGRESQL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The one connection (PGconn) of a client, shared with its tables. A libpq connection serves one statement at a
 *  time: every user holds GetLock() around a statement or a group of statements that belong together, and the calls
 *  marked "with the lock held" are only made so. The session runs in UTC, with ISO dates, exact float text, hex bytea
 *  and UTF-8 (SetUpSession). Notices of the server go to the log, never to the console.
 *
 *  A lost connection is made again before the next statement (PQreset). A transaction of the client that was open
 *  when it was lost is gone: every change then fails with IO_ERROR until the transaction is ended, so no change is
 *  applied outside the transaction it was meant for.
 */
class DBPLUGIN_POSTGRESQL_PUBLIC_CPP CPostgreSqlConnection
{
    public:

    CPostgreSqlConnection( void );

    /** Closes the connection */
    ~CPostgreSqlConnection();

    /** Connects and sets up the session; IO_ERROR with the reason of libpq when it cannot */
    CORE::TDataTableStatus Open( const CPostgreSqlSettings& settings, CORE::CString& error );

    void Close( void );

    bool IsOpen( void ) const;

    /** Held around every statement or group of statements; recursive */
    MT::CMutex& GetLock( void ) const;

    const CPostgreSqlSettings& GetSettings( void ) const;

    /** The version of the server as libpq gives it (140017 for 14.17), 0 when not connected */
    UInt32 GetServerVersion( void ) const;

    /** With the lock held: runs a statement with parameters; the rows, if any, in the result */
    CORE::TDataTableStatus Execute( const CORE::CString& sql, const CPostgreSqlParameters& parameters, CPostgreSqlResult& result, CORE::CString& error );

    /** With the lock held: runs a statement without parameters whose rows are not wanted */
    CORE::TDataTableStatus Execute( const CORE::CString& sql, CORE::CString& error );

    /**
     *  With the lock held: starts a unit of work that changes data and is applied whole or not at all. Outside the
     *  transaction of the client a transaction of its own (BEGIN), inside it a savepoint, so that a failed change does
     *  not end the transaction of the client. Every BeginWork() is paired with an EndWork().
     */
    CORE::TDataTableStatus BeginWork( CORE::CString& error );

    /** With the lock held: applies (commit) or drops the unit of work of BeginWork() */
    CORE::TDataTableStatus EndWork( bool commit, CORE::CString& error );

    /** With the lock held: the transaction of the client, see CIDatabaseClient */
    CORE::TDataTableStatus BeginTransaction( CORE::CString& error );
    CORE::TDataTableStatus CommitTransaction( CORE::CString& error );
    CORE::TDataTableStatus RollbackTransaction( CORE::CString& error );

    bool IsInTransaction( void ) const;

    /** The status of an SQLSTATE: CONFLICT for unique violations and serialization failures, TYPE_MISMATCH for data errors, ... */
    static CORE::TDataTableStatus StatusOfSqlState( const char* sqlState );

    private:

    CPostgreSqlConnection( const CPostgreSqlConnection& src );              /**< not implemented */
    CPostgreSqlConnection& operator=( const CPostgreSqlConnection& src );   /**< not implemented */

    CORE::TDataTableStatus EnsureConnected( CORE::CString& error );
    CORE::TDataTableStatus SetUpSession( CORE::CString& error );
    CORE::TDataTableStatus CheckResult( struct pg_result* result, const CORE::CString& sql, CORE::CString& error );

    struct pg_conn* m_connection;
    mutable MT::CMutex m_lock;
    CPostgreSqlSettings m_settings;
    bool m_inTransaction;
    bool m_transactionLost;
    UInt32 m_workDepth;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CPostgreSqlConnection, MT::CMutex > CPostgreSqlConnectionPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQL */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCONNECTION_H ? */
