/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_COMCORE_TESTDATABASECLIENTTRANSACTIONAL_H
#define GUCEF_COMCORE_TESTDATABASECLIENTTRANSACTIONAL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CDATABASECLIENTBASE_H
#include "gucefCOMCORE_CDatabaseClientBase.h"
#define GUCEF_COMCORE_CDATABASECLIENTBASE_H
#endif /* GUCEF_COMCORE_CDATABASECLIENTBASE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A database backend for the tests with GUCEF_DATABASE_CAPABILITY_TRANSACTIONS, held in memory: its tables are
 *  CMemoryDataTable objects. BeginTransaction() takes a copy of the rows of every table and of the set of tables;
 *  RollbackTransaction() puts both back into the same table objects, CommitTransaction() drops the copy. So every
 *  change made through the client and its tables while a transaction is open is undone by a rollback, whichever thread
 *  made it. Records the thread of the last transaction call.
 */
class CTestTransactionalDatabaseClient : public GUCEF::COMCORE::CDatabaseClientBase
{
    public:

    static const GUCEF::CORE::CString BackendName;

    CTestTransactionalDatabaseClient( void );

    virtual ~CTestTransactionalDatabaseClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Open( const GUCEF::COMCORE::CDatabaseClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    /** Rolls back an open transaction */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsOpen( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const GUCEF::CORE::CString& GetBackendName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus ListTables( GUCEF::CORE::CString::StringSet& tableNames ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus GetTableDefinition( const GUCEF::CORE::CString& tableName, GUCEF::CORE::CDataTableDefinition& definition ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus OpenTable( const GUCEF::CORE::CString& tableName, GUCEF::CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus CreateTable( const GUCEF::CORE::CDataTableDefinition& definition, GUCEF::CORE::CIDataTablePtr& table ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus DropTable( const GUCEF::CORE::CString& tableName, bool deleteData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus BeginTransaction( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus CommitTransaction( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual GUCEF::CORE::TDataTableStatus RollbackTransaction( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsInTransaction( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The thread the last Begin, Commit or Rollback ran on, 0 before the first */
    GUCEF::TThreadId GetThreadOfLastTransactionCall( void ) const;

    /** Transactions committed and rolled back so far */
    GUCEF::UInt32 GetCommitCount( void ) const;
    GUCEF::UInt32 GetRollbackCount( void ) const;

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CIDataTablePtr > TTableMap;
    typedef GUCEF::vector< GUCEF::CORE::CDataTableBatchPtr > TBatchVector;
    typedef GUCEF::map< GUCEF::CORE::CString, TBatchVector > TRowsMap;
    typedef GUCEF::map< GUCEF::TThreadId, GUCEF::CORE::CString > TErrorMap;

    GUCEF::CORE::TDataTableStatus Fail( GUCEF::CORE::TDataTableStatus status, const GUCEF::CORE::CString& message );
    GUCEF::CORE::TDataTableStatus Succeed( void );
    GUCEF::CORE::TDataTableStatus RestoreLocked( void );
    void RecordTransactionCall( void );

    mutable GUCEF::MT::CMutex m_lock;
    TTableMap m_tables;
    TTableMap m_tablesAtBegin;
    TRowsMap m_rowsAtBegin;
    TErrorMap m_errors;
    GUCEF::TThreadId m_lastTransactionThread;
    GUCEF::UInt32 m_commits;
    GUCEF::UInt32 m_rollbacks;
    bool m_inTransaction;
    bool m_open;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTDATABASECLIENTTRANSACTIONAL_H ? */
