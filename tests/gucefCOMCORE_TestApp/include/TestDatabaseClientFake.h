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

#ifndef GUCEF_COMCORE_TESTDATABASECLIENTFAKE_H
#define GUCEF_COMCORE_TESTDATABASECLIENTFAKE_H

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
 *  A database backend for the tests, held in memory: tables are definitions, the tables it opens have no rows and refuse
 *  every operation on rows. It records on which thread each sync call ran, so the tests can tell sync calls on the calling
 *  thread from async calls on a pool thread, keeps the last error message per calling thread, and can be made slow to
 *  have operations pending.
 */
class CTestFakeDatabaseClient : public GUCEF::COMCORE::CDatabaseClientBase
{
    public:

    static const GUCEF::CORE::CString BackendName;

    /** Milliseconds every table operation takes, to have async operations pending */
    GUCEF::UInt32 delayInMs;

    CTestFakeDatabaseClient( void );

    virtual ~CTestFakeDatabaseClient() GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Open( const GUCEF::COMCORE::CDatabaseClientSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;
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

    /** The id of the thread the last call of the kind ran on: 0 open, 1 list, 2 open table, 3 create, 4 drop */
    GUCEF::TThreadId GetThreadOfLastCall( GUCEF::UInt32 kind ) const;

    GUCEF::UInt32 GetCallCount( void ) const;

    private:

    GUCEF::CORE::TDataTableStatus Fail( GUCEF::CORE::TDataTableStatus status, const GUCEF::CORE::CString& message );
    void Record( GUCEF::UInt32 kind );

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CDataTableDefinition > TDefinitionMap;
    typedef GUCEF::map< GUCEF::TThreadId, GUCEF::CORE::CString > TErrorMap;

    mutable GUCEF::MT::CMutex m_lock;
    TDefinitionMap m_tables;
    TErrorMap m_errors;
    GUCEF::TThreadId m_threads[ 5 ];
    GUCEF::UInt32 m_callCount;
    bool m_open;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_TESTDATABASECLIENTFAKE_H ? */
