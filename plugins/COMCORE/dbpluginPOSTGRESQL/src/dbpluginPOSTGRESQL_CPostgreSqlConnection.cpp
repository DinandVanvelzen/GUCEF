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

#include <string.h>

#include "libpq-fe.h"

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlConnection.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Notices of the server (NOTICE, WARNING) go to the log instead of the standard error of the process */
void
PostgreSqlNoticeProcessor( void* arg, const char* message )
{
    CORE::CString text( GUCEF_NULL != message ? message : "" );
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "PostgreSqlConnection: server notice: " << text.Trim( false ) );
}

/*-------------------------------------------------------------------------*/

/** The message of libpq without its trailing new line */
CORE::CString
TrimmedMessage( const char* message )
{
    return CORE::CString( GUCEF_NULL != message ? message : "" ).Trim( false );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPostgreSqlConnection::CPostgreSqlConnection( void )
    : m_connection( GUCEF_NULL )
    , m_lock()
    , m_settings()
    , m_inTransaction( false )
    , m_transactionLost( false )
    , m_workDepth( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPostgreSqlConnection::~CPostgreSqlConnection()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

MT::CMutex&
CPostgreSqlConnection::GetLock( void ) const
{GUCEF_TRACE;

    return m_lock;
}

/*-------------------------------------------------------------------------*/

const CPostgreSqlSettings&
CPostgreSqlConnection::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::StatusOfSqlState( const char* sqlState )
{GUCEF_TRACE;

    if ( GUCEF_NULL == sqlState || strlen( sqlState ) < 5 )
        return CORE::DATATABLE_STATUS_FAILED;
    if ( 0 == strcmp( sqlState, "23505" ) || 0 == strcmp( sqlState, "40001" ) || 0 == strcmp( sqlState, "40P01" ) )
        return CORE::DATATABLE_STATUS_CONFLICT;
    if ( 0 == strcmp( sqlState, "42P01" ) || 0 == strcmp( sqlState, "3F000" ) || 0 == strcmp( sqlState, "3D000" ) )
        return CORE::DATATABLE_STATUS_NOT_FOUND;
    if ( 0 == strcmp( sqlState, "42P07" ) || 0 == strcmp( sqlState, "42P06" ) )
        return CORE::DATATABLE_STATUS_ALREADY_EXISTS;
    if ( 0 == strcmp( sqlState, "25006" ) )
        return CORE::DATATABLE_STATUS_READ_ONLY;
    if ( 0 == strcmp( sqlState, "57014" ) )
        return CORE::DATATABLE_STATUS_CANCELLED;
    if ( 0 == strcmp( sqlState, "53200" ) )
        return CORE::DATATABLE_STATUS_OUT_OF_MEMORY;
    if ( 0 == strcmp( sqlState, "54000" ) || 0 == strcmp( sqlState, "54001" ) || 0 == strcmp( sqlState, "54011" ) || 0 == strcmp( sqlState, "54023" ) )
        return CORE::DATATABLE_STATUS_LIMIT_EXCEEDED;
    // data exceptions and the integrity constraints other than unique: a value the column does not take
    if ( '2' == sqlState[ 0 ] && ( '2' == sqlState[ 1 ] || '3' == sqlState[ 1 ] ) )
        return CORE::DATATABLE_STATUS_TYPE_MISMATCH;
    // connection exceptions and the server going down
    if ( ( '0' == sqlState[ 0 ] && '8' == sqlState[ 1 ] ) || ( '5' == sqlState[ 0 ] && '7' == sqlState[ 1 ] ) )
        return CORE::DATATABLE_STATUS_IO_ERROR;
    if ( '4' == sqlState[ 0 ] && '2' == sqlState[ 1 ] )
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    return CORE::DATATABLE_STATUS_FAILED;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::Open( const CPostgreSqlSettings& settings, CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    Close();
    m_settings = settings;

    CORE::CString::StringVector keywords;
    CORE::CString::StringVector values;
    settings.GetConnectionParameters( keywords, values );
    GUCEF::vector< CORE::CUtf8String > keywordText;
    GUCEF::vector< CORE::CUtf8String > valueText;
    for ( UInt32 i=0; i<(UInt32) keywords.size(); ++i )
    {
        keywordText.push_back( CORE::ToUtf8String( keywords[ i ] ) );
        valueText.push_back( CORE::ToUtf8String( values[ i ] ) );
    }
    GUCEF::vector< const char* > keywordPointers;
    GUCEF::vector< const char* > valuePointers;
    for ( UInt32 i=0; i<(UInt32) keywordText.size(); ++i )
    {
        keywordPointers.push_back( keywordText[ i ].C_String() );
        valuePointers.push_back( valueText[ i ].C_String() );
    }
    keywordPointers.push_back( GUCEF_NULL );
    valuePointers.push_back( GUCEF_NULL );

    m_connection = PQconnectdbParams( &keywordPointers[ 0 ], &valuePointers[ 0 ], 1 );

    // the copies of the password are not kept longer than needed
    for ( UInt32 i=0; i<(UInt32) valueText.size(); ++i )
    {
        if ( valueText[ i ].ByteSize() > 1 )
            memset( const_cast< char* >( valueText[ i ].C_String() ), 0, valueText[ i ].ByteSize() - 1 );
    }

    if ( GUCEF_NULL == m_connection )
    {
        error = "libpq could not make a connection object (out of memory)";
        return CORE::DATATABLE_STATUS_OUT_OF_MEMORY;
    }
    if ( CONNECTION_OK != PQstatus( m_connection ) )
    {
        error = "Connecting to " + settings.Describe() + " failed: " + TrimmedMessage( PQerrorMessage( m_connection ) );
        PQfinish( m_connection );
        m_connection = GUCEF_NULL;
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    PQsetNoticeProcessor( m_connection, &PostgreSqlNoticeProcessor, GUCEF_NULL );

    const CORE::TDataTableStatus status = SetUpSession( error );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        PQfinish( m_connection );
        m_connection = GUCEF_NULL;
        return status;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "PostgreSqlConnection: connected to " << settings.Describe() << ", server version " <<
        PQserverVersion( m_connection ) << ", TLS " << ( 0 != PQsslInUse( m_connection ) ? "in use" : "not in use" ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::SetUpSession( CORE::CString& error )
{GUCEF_TRACE;

    PGresult* result = PQexec( m_connection, "SET TimeZone TO 'UTC'; SET DateStyle TO 'ISO, YMD'; SET extra_float_digits TO 3; "
                                             "SET bytea_output TO 'hex'; SET client_encoding TO 'UTF8'" );
    const CORE::TDataTableStatus status = CheckResult( result, "session setup", error );
    PQclear( result );
    return status;
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlConnection::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( GUCEF_NULL != m_connection )
    {
        // an open transaction of the client ends with the session: PostgreSQL rolls it back
        PQfinish( m_connection );
        m_connection = GUCEF_NULL;
    }
    m_inTransaction = false;
    m_transactionLost = false;
    m_workDepth = 0;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlConnection::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    return GUCEF_NULL != m_connection;
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlConnection::GetServerVersion( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    return GUCEF_NULL == m_connection ? 0 : (UInt32) PQserverVersion( m_connection );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::EnsureConnected( CORE::CString& error )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_connection )
    {
        error = "The database is not open";
        return CORE::DATATABLE_STATUS_FAILED;
    }
    if ( CONNECTION_OK == PQstatus( m_connection ) )
        return CORE::DATATABLE_STATUS_OK;

    if ( m_inTransaction )
        m_transactionLost = true;
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "PostgreSqlConnection: the connection to " << m_settings.Describe() << " was lost, connecting again" );
    PQreset( m_connection );
    if ( CONNECTION_OK != PQstatus( m_connection ) )
    {
        error = "The connection to " + m_settings.Describe() + " was lost and cannot be made again: " + TrimmedMessage( PQerrorMessage( m_connection ) );
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    m_workDepth = 0;
    return SetUpSession( error );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::CheckResult( PGresult* result, const CORE::CString& sql, CORE::CString& error )
{GUCEF_TRACE;

    if ( GUCEF_NULL == result )
    {
        error = "The statement could not be sent: " + TrimmedMessage( PQerrorMessage( m_connection ) );
        if ( CONNECTION_OK != PQstatus( m_connection ) && m_inTransaction )
            m_transactionLost = true;
        return CONNECTION_OK != PQstatus( m_connection ) ? CORE::DATATABLE_STATUS_IO_ERROR : CORE::DATATABLE_STATUS_OUT_OF_MEMORY;
    }
    const ExecStatusType status = PQresultStatus( result );
    if ( PGRES_COMMAND_OK == status || PGRES_TUPLES_OK == status )
        return CORE::DATATABLE_STATUS_OK;

    const char* sqlState = PQresultErrorField( result, PG_DIAG_SQLSTATE );
    error = TrimmedMessage( PQresultErrorMessage( result ) );
    if ( CONNECTION_OK != PQstatus( m_connection ) )
    {
        if ( m_inTransaction )
            m_transactionLost = true;
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    return StatusOfSqlState( sqlState );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::Execute( const CORE::CString& sql, const CPostgreSqlParameters& parameters, CPostgreSqlResult& result, CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    result.Reset();
    CORE::TDataTableStatus status = EnsureConnected( error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    const CORE::CUtf8String text = CORE::ToUtf8String( sql );
    PGresult* executed = PQexecParams( m_connection, text.C_String(), (int) parameters.GetCount(), GUCEF_NULL,
                                       parameters.GetValues(), parameters.GetLengths(), parameters.GetFormats(), 0 );
    status = CheckResult( executed, sql, error );
    result.Reset( executed );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::Execute( const CORE::CString& sql, CORE::CString& error )
{GUCEF_TRACE;

    CPostgreSqlResult result;
    return Execute( sql, CPostgreSqlParameters(), result, error );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::BeginWork( CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    CORE::TDataTableStatus status = EnsureConnected( error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( m_transactionLost )
    {
        error = "The connection was lost during the transaction of the client, which is rolled back: end it before changing data again";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    if ( m_inTransaction || m_workDepth > 0 )
        status = Execute( "SAVEPOINT gucef_work_" + CORE::ToString( m_workDepth ), error );
    else
        status = Execute( "BEGIN", error );
    if ( CORE::DATATABLE_STATUS_OK == status )
        ++m_workDepth;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::EndWork( bool commit, CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( 0 == m_workDepth )
    {
        error = "No unit of work is open";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    --m_workDepth;
    if ( m_transactionLost )
    {
        error = "The connection was lost during the transaction of the client, which is rolled back";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    if ( m_inTransaction || m_workDepth > 0 )
    {
        const CORE::CString savepoint = "gucef_work_" + CORE::ToString( m_workDepth );
        if ( commit )
            return Execute( "RELEASE SAVEPOINT " + savepoint, error );
        CORE::CString rollbackError;
        CORE::TDataTableStatus status = Execute( "ROLLBACK TO SAVEPOINT " + savepoint, rollbackError );
        if ( CORE::DATATABLE_STATUS_OK == status )
            status = Execute( "RELEASE SAVEPOINT " + savepoint, rollbackError );
        if ( CORE::DATATABLE_STATUS_OK != status )
            error = rollbackError;
        return status;
    }
    if ( commit )
    {
        const CORE::TDataTableStatus status = Execute( "COMMIT", error );
        return status;
    }
    CORE::CString rollbackError;
    const CORE::TDataTableStatus status = Execute( "ROLLBACK", rollbackError );
    if ( CORE::DATATABLE_STATUS_OK != status )
        error = rollbackError;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::BeginTransaction( CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( m_inTransaction || m_transactionLost )
    {
        error = "A transaction is open already";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    if ( m_workDepth > 0 )
    {
        error = "A change is under way on the connection";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    const CORE::TDataTableStatus status = Execute( "BEGIN", error );
    if ( CORE::DATATABLE_STATUS_OK == status )
        m_inTransaction = true;
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::CommitTransaction( CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( m_transactionLost )
    {
        m_transactionLost = false;
        m_inTransaction = false;
        error = "The connection was lost during the transaction, nothing of it is applied";
        return CORE::DATATABLE_STATUS_IO_ERROR;
    }
    if ( !m_inTransaction )
    {
        error = "No transaction is open";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    // a failed COMMIT ends the transaction as well: PostgreSQL rolls it back
    const CORE::TDataTableStatus status = Execute( "COMMIT", error );
    m_inTransaction = false;
    if ( CORE::DATATABLE_STATUS_OK != status && CONNECTION_OK == PQstatus( m_connection ) && PQTRANS_IDLE != PQtransactionStatus( m_connection ) )
    {
        CORE::CString rollbackError;
        Execute( "ROLLBACK", rollbackError );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlConnection::RollbackTransaction( CORE::CString& error )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    if ( m_transactionLost )
    {
        // the server rolled it back when the session ended
        m_transactionLost = false;
        m_inTransaction = false;
        return CORE::DATATABLE_STATUS_OK;
    }
    if ( !m_inTransaction )
    {
        error = "No transaction is open";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    const CORE::TDataTableStatus status = Execute( "ROLLBACK", error );
    m_inTransaction = false;
    return status;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlConnection::IsInTransaction( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
    return m_inTransaction || m_transactionLost;
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
