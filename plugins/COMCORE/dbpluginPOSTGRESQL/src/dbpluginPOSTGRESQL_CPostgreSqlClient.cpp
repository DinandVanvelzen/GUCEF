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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDSTORECODECREGISTRY_H
#include "CDStoreCodecRegistry.h"
#define GUCEF_CORE_CDSTORECODECREGISTRY_H
#endif /* GUCEF_CORE_CDSTORECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSql.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLDATATABLE_H
#include "dbpluginPOSTGRESQL_CPostgreSqlDataTable.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLDATATABLE_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLDATATABLE_H ? */

#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CPostgreSqlClient::BackendName = "postgresql";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

struct SModuleState
{
    MT::CMutex lock;
    GUCEF::map< CORE::CString, UInt32 > liveObjects;
};

SModuleState&
GetModuleState( void )
{
    static SModuleState state;
    return state;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CatalogueName( const CPostgreSqlConnection& connection )
{
    return CPostgreSqlSql::QualifiedName( connection.GetSettings().schema, CPostgreSqlSql::CatalogueTableName );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CPostgreSqlClient::OnObjectCreated( const CORE::CString& description )
{GUCEF_TRACE;

    SModuleState& state = GetModuleState();
    MT::CScopeMutex lock( state.lock );
    ++state.liveObjects[ description ];
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlClient::OnObjectDestroyed( const CORE::CString& description )
{GUCEF_TRACE;

    SModuleState& state = GetModuleState();
    MT::CScopeMutex lock( state.lock );
    GUCEF::map< CORE::CString, UInt32 >::iterator i = state.liveObjects.find( description );
    if ( i != state.liveObjects.end() && 0 == --(*i).second )
        state.liveObjects.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlClient::GetLiveObjects( CORE::CString::StringVector& descriptions )
{GUCEF_TRACE;

    descriptions.clear();
    SModuleState& state = GetModuleState();
    MT::CScopeMutex lock( state.lock );
    GUCEF::map< CORE::CString, UInt32 >::const_iterator i = state.liveObjects.begin();
    while ( i != state.liveObjects.end() )
    {
        for ( UInt32 n=0; n<(*i).second; ++n )
            descriptions.push_back( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CPostgreSqlClient::CPostgreSqlClient( void )
    : COMCORE::CDatabaseClientBase()
    , m_lock()
    , m_connection()
    , m_readOnly( false )
    , m_errors()
{GUCEF_TRACE;

    OnObjectCreated( "database client" );
}

/*-------------------------------------------------------------------------*/

CPostgreSqlClient::~CPostgreSqlClient()
{GUCEF_TRACE;

    WaitForAsyncOperations();
    CloseConnection( false );
    OnObjectDestroyed( "database client" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_errors[ MT::GetCurrentThreadID() ] = message;
    }
    CORE::TCloneableString eventData( message );
    const_cast< CPostgreSqlClient* >( this )->NotifyObserversFromThread( DatabaseErrorEvent, &eventData );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TErrorMap::iterator i = m_errors.find( MT::GetCurrentThreadID() );
    if ( i != m_errors.end() )
        m_errors.erase( i );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPostgreSqlClient::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TErrorMap::const_iterator i = m_errors.find( MT::GetCurrentThreadID() );
    return i != m_errors.end() ? (*i).second : CORE::CString::Empty;
}

/*-------------------------------------------------------------------------*/

CPostgreSqlConnectionPtr
CPostgreSqlClient::GetConnection( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_connection;
}

/*-------------------------------------------------------------------------*/

CPostgreSqlConnectionPtr
CPostgreSqlClient::GetOpenConnection( CORE::TDataTableStatus& status ) const
{GUCEF_TRACE;

    CPostgreSqlConnectionPtr connection = GetConnection();
    if ( connection.IsNULL() )
        status = Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" );
    else
        status = CORE::DATATABLE_STATUS_OK;
    return connection;
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlClient::Open( const COMCORE::CDatabaseClientSettings& settings )
{GUCEF_TRACE;

    // not Close(): OpenAsync() runs this on a pool thread, as one of the operations Close() waits for
    CloseConnection( true );

    CORE::CString error;
    if ( !settings.Validate( error ) )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
        return false;
    }
    if ( settings.backendName != BackendName )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The settings are for backend \"" + settings.backendName + "\", not \"" + BackendName + "\"" );
        return false;
    }
    CPostgreSqlSettings postgreSqlSettings;
    if ( !postgreSqlSettings.LoadFrom( settings, error ) )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
        return false;
    }

    CPostgreSqlConnectionPtr connection( GUCEF_NEW CPostgreSqlConnection() );
    CORE::TDataTableStatus status = connection->Open( postgreSqlSettings, error );
    if ( CORE::DATATABLE_STATUS_OK == status )
    {
        MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        const CORE::CString& schema = postgreSqlSettings.schema;
        if ( !settings.readOnly && postgreSqlSettings.createSchema )
            status = connection->Execute( "CREATE SCHEMA IF NOT EXISTS " + CPostgreSqlSql::QuoteIdentifier( schema ), error );
        if ( CORE::DATATABLE_STATUS_OK == status )
        {
            CPostgreSqlParameters parameters;
            parameters.AddText( schema );
            CPostgreSqlResult found;
            status = connection->Execute( "SELECT 1 FROM pg_catalog.pg_namespace WHERE nspname = $1::text", parameters, found, error );
            if ( CORE::DATATABLE_STATUS_OK == status && 0 == found.GetRowCount() )
            {
                status = CORE::DATATABLE_STATUS_NOT_FOUND;
                error = "The schema \"" + schema + "\" does not exist in the database (backend option createSchema makes it)";
            }
        }
        if ( CORE::DATATABLE_STATUS_OK == status && !settings.readOnly )
        {
            status = connection->Execute( "CREATE TABLE IF NOT EXISTS " + CatalogueName( *connection ) +
                                          " (\"name\" text COLLATE \"C\" PRIMARY KEY, \"definition\" text NOT NULL)", error );
        }
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        connection->Close();
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "PostgreSqlClient: opening " << postgreSqlSettings.Describe() << " failed: " << error );
        Fail( status, error );
        return false;
    }

    {
        MT::CScopeMutex lock( m_lock );
        m_connection = connection;
        m_readOnly = settings.readOnly;
    }
    SetSettings( settings );
    Succeed();
    NotifyObserversFromThread( DatabaseOpenedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlClient::Close( void )
{GUCEF_TRACE;

    WaitForAsyncOperations();
    CloseConnection( true );
}

/*-------------------------------------------------------------------------*/

void
CPostgreSqlClient::CloseConnection( bool notify )
{GUCEF_TRACE;

    CPostgreSqlConnectionPtr connection;
    {
        MT::CScopeMutex lock( m_lock );
        connection = m_connection;
        m_connection.Unlink();
    }
    if ( connection.IsNULL() )
        return;
    // tables still held keep the connection object, closed: their calls then fail
    connection->Close();
    if ( notify )
        NotifyObserversFromThread( DatabaseClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlClient::IsOpen( void ) const
{GUCEF_TRACE;

    return !GetConnection().IsNULL();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPostgreSqlClient::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

UInt32
CPostgreSqlClient::GetCapabilities( void ) const
{GUCEF_TRACE;

    UInt32 capabilities = GUCEF_DATABASE_CAPABILITY_PREDICATE_PUSHDOWN | GUCEF_DATABASE_CAPABILITY_ASYNC |
                          GUCEF_DATABASE_CAPABILITY_TRANSACTIONS | GUCEF_DATABASE_CAPABILITY_INDEXES;
    MT::CScopeMutex lock( m_lock );
    if ( !m_readOnly )
        capabilities |= GUCEF_DATABASE_CAPABILITY_WRITE;
    return capabilities;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::ListTables( CORE::CString::StringSet& tableNames )
{GUCEF_TRACE;

    tableNames.clear();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    CORE::CString error;
    CPostgreSqlResult names;
    {
        MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = connection->Execute( "SELECT \"name\" FROM " + CatalogueName( *connection ), CPostgreSqlParameters(), names, error );
    }
    // a database opened read only before any table was made has no catalogue
    if ( CORE::DATATABLE_STATUS_NOT_FOUND == status )
        return Succeed();
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Listing the tables failed: " + error );
    for ( UInt32 r=0; r<names.GetRowCount(); ++r )
        tableNames.insert( CORE::CString( names.GetValue( r, 0 ), names.GetLength( r, 0 ) ) );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::LoadDefinition( CPostgreSqlConnection& connection, const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) const
{GUCEF_TRACE;

    CORE::CString error;
    CPostgreSqlParameters parameters;
    parameters.AddText( tableName );
    CPostgreSqlResult found;
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    {
        MT::CScopeMutex lock( connection.GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = connection.Execute( "SELECT \"definition\" FROM " + CatalogueName( connection ) + " WHERE \"name\" = $1::text", parameters, found, error );
    }
    if ( CORE::DATATABLE_STATUS_NOT_FOUND == status || ( CORE::DATATABLE_STATUS_OK == status && 0 == found.GetRowCount() ) )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\" in schema \"" + connection.GetSettings().schema + "\"" );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Reading the definition of table \"" + tableName + "\" failed: " + error );

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) )
        return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "Table definitions are JSON and no JSON DStore codec is registered" );

    CORE::CDynamicBuffer content;
    content.CopyFrom( found.GetLength( 0, 0 ), found.GetValue( 0, 0 ) );
    CORE::CDynamicBufferAccess input( content );
    CORE::CDataNode tree;
    if ( !json->BuildDataTree( &tree, &input ) )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The definition of table \"" + tableName + "\" is not valid JSON" );
    const CORE::CDataNode* root = tree.FindChild( "DataTableDefinition" );
    if ( GUCEF_NULL == root )
        root = &tree;

    CORE::CDataTableDefinition loaded;
    if ( !loaded.Deserialize( *root, CORE::CDataNodeSerializableSettings() ) || !loaded.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The definition of table \"" + tableName + "\" is not a valid table definition: " + error );
    if ( loaded.name != tableName )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The definition recorded for table \"" + tableName + "\" is of table \"" + loaded.name + "\"" );
    definition = loaded;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = LoadDefinition( *connection, tableName, definition );
    return CORE::DATATABLE_STATUS_OK == status ? Succeed() : status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    table.Unlink();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CDataTableDefinition definition;
    status = LoadDefinition( *connection, tableName, definition );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    table = CORE::CIDataTablePtr( GUCEF_NEW CPostgreSqlDataTable( connection, definition, connection->GetSettings().readOnly, GetSettings().pulseGenerator ) );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::CreateTableLocked( CPostgreSqlConnection& connection, const CORE::CDataTableDefinition& definition, CORE::CString& error ) const
{GUCEF_TRACE;

    const CORE::CString& schema = connection.GetSettings().schema;
    CPostgreSqlParameters nameParameters;
    nameParameters.AddText( definition.name );
    CPostgreSqlResult found;
    CORE::TDataTableStatus status = connection.Execute( "SELECT 1 FROM " + CatalogueName( connection ) + " WHERE \"name\" = $1::text", nameParameters, found, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( found.GetRowCount() > 0 )
    {
        error = "The table \"" + definition.name + "\" exists";
        return CORE::DATATABLE_STATUS_ALREADY_EXISTS;
    }

    CORE::CString sql;
    if ( !CPostgreSqlSql::MakeCreateTable( definition, schema, sql, error ) )
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    status = connection.Execute( sql, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    // a table of the name that was there already is taken over when it has every column of the definition
    CPostgreSqlParameters columnParameters;
    columnParameters.AddText( schema );
    columnParameters.AddText( definition.name );
    CPostgreSqlResult columns;
    status = connection.Execute( "SELECT column_name FROM information_schema.columns WHERE table_schema = $1::text AND table_name = $2::text",
                                 columnParameters, columns, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CString::StringSet present;
    for ( UInt32 r=0; r<columns.GetRowCount(); ++r )
        present.insert( CORE::CString( columns.GetValue( r, 0 ), columns.GetLength( r, 0 ) ) );
    for ( UInt32 c=0; c<definition.schema.GetColumnCount(); ++c )
    {
        if ( present.find( definition.schema.GetColumn( c ).name ) == present.end() )
        {
            error = "A table \"" + definition.name + "\" exists in the schema without the column \"" + definition.schema.GetColumn( c ).name + "\"";
            return CORE::DATATABLE_STATUS_SCHEMA_MISMATCH;
        }
    }

    for ( UInt32 i=0; i<(UInt32) definition.indexes.size(); ++i )
    {
        status = connection.Execute( CPostgreSqlSql::MakeCreateIndex( definition, definition.indexes[ i ], schema ), error );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) )
    {
        error = "Table definitions are JSON and no JSON DStore codec is registered";
        return CORE::DATATABLE_STATUS_UNSUPPORTED;
    }
    CORE::CDataNode tree;
    CORE::CDynamicBuffer content;
    {
        CORE::CDynamicBufferAccess output( &content, false );
        if ( !definition.Serialize( tree, CORE::CDataNodeSerializableSettings() ) || !json->StoreDataTree( &tree, &output ) )
        {
            error = "The definition of table \"" + definition.name + "\" could not be serialized";
            return CORE::DATATABLE_STATUS_FAILED;
        }
    }
    CPostgreSqlParameters recordParameters;
    recordParameters.AddText( definition.name );
    if ( 0 == recordParameters.AddText( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) )
    {
        error = "The definition of table \"" + definition.name + "\" holds a NUL character";
        return CORE::DATATABLE_STATUS_INVALID_ARGUMENT;
    }
    CPostgreSqlResult recorded;
    return connection.Execute( "INSERT INTO " + CatalogueName( connection ) + " (\"name\", \"definition\") VALUES ($1::text, $2::text)",
                               recordParameters, recorded, error );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    table.Unlink();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( connection->GetSettings().readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database is open read only" );

    CORE::CString error;
    if ( !definition.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    if ( definition.partitioning.IsPartitioned() )
        return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "Table \"" + definition.name + "\": the postgresql backend has no partitioned tables" );
    if ( definition.name == CPostgreSqlSql::CatalogueTableName || !CPostgreSqlSql::IsUsableIdentifier( definition.name ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The table name \"" + definition.name + "\" cannot be used in PostgreSQL" );

    {
        MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = connection->BeginWork( error );
        if ( CORE::DATATABLE_STATUS_OK == status )
        {
            status = CreateTableLocked( *connection, definition, error );
            CORE::CString endError;
            const CORE::TDataTableStatus endStatus = connection->EndWork( CORE::DATATABLE_STATUS_OK == status, endError );
            if ( CORE::DATATABLE_STATUS_OK == status && CORE::DATATABLE_STATUS_OK != endStatus )
            {
                status = endStatus;
                error = endError;
            }
        }
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Creating table \"" + definition.name + "\" failed: " + error );

    table = CORE::CIDataTablePtr( GUCEF_NEW CPostgreSqlDataTable( connection, definition, false, GetSettings().pulseGenerator ) );
    Succeed();
    CORE::TCloneableString eventData( definition.name );
    NotifyObserversFromThread( TableCreatedEvent, &eventData );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::DropTable( const CORE::CString& tableName, bool deleteData )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    if ( connection->GetSettings().readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database is open read only" );

    CORE::CString error;
    {
        MT::CScopeMutex lock( connection->GetLock(), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = connection->BeginWork( error );
        if ( CORE::DATATABLE_STATUS_OK == status )
        {
            CPostgreSqlParameters parameters;
            parameters.AddText( tableName );
            CPostgreSqlResult deleted;
            status = connection->Execute( "DELETE FROM " + CatalogueName( *connection ) + " WHERE \"name\" = $1::text", parameters, deleted, error );
            if ( CORE::DATATABLE_STATUS_OK == status && 0 == deleted.GetAffectedRowCount() )
            {
                status = CORE::DATATABLE_STATUS_NOT_FOUND;
                error = "No table \"" + tableName + "\"";
            }
            if ( CORE::DATATABLE_STATUS_OK == status && deleteData )
                status = connection->Execute( "DROP TABLE IF EXISTS " + CPostgreSqlSql::QualifiedName( connection->GetSettings().schema, tableName ), error );
            CORE::CString endError;
            const CORE::TDataTableStatus endStatus = connection->EndWork( CORE::DATATABLE_STATUS_OK == status, endError );
            if ( CORE::DATATABLE_STATUS_OK == status && CORE::DATATABLE_STATUS_OK != endStatus )
            {
                status = endStatus;
                error = endError;
            }
        }
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, "Dropping table \"" + tableName + "\" failed: " + error );

    Succeed();
    CORE::TCloneableString eventData( tableName );
    NotifyObserversFromThread( TableDroppedEvent, &eventData );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::BeginTransaction( void )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CString error;
    status = connection->BeginTransaction( error );
    return CORE::DATATABLE_STATUS_OK == status ? Succeed() : Fail( status, error );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::CommitTransaction( void )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CString error;
    status = connection->CommitTransaction( error );
    return CORE::DATATABLE_STATUS_OK == status ? Succeed() : Fail( status, error );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CPostgreSqlClient::RollbackTransaction( void )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CPostgreSqlConnectionPtr connection = GetOpenConnection( status );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    CORE::CString error;
    status = connection->RollbackTransaction( error );
    return CORE::DATATABLE_STATUS_OK == status ? Succeed() : Fail( status, error );
}

/*-------------------------------------------------------------------------*/

bool
CPostgreSqlClient::IsInTransaction( void ) const
{GUCEF_TRACE;

    CPostgreSqlConnectionPtr connection = GetConnection();
    return !connection.IsNULL() && connection->IsInTransaction();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPostgreSqlClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DBPLUGIN::POSTGRESQL::CPostgreSqlClient";
    return classTypeName;
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
