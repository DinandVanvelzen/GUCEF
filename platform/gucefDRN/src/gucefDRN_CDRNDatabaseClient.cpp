/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNREPLICATOR_H
#include "gucefDRN_CDRNReplicator.h"
#define GUCEF_DRN_CDRNREPLICATOR_H
#endif /* GUCEF_DRN_CDRNREPLICATOR_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRIDTABLE_H
#include "gucefDRN_CDRNDataGridTable.h"
#define GUCEF_DRN_CDRNDATAGRIDTABLE_H
#endif /* GUCEF_DRN_CDRNDATAGRIDTABLE_H ? */

#include "gucefDRN_CDRNDatabaseClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The logical type named in a backend option, DATATABLE_TYPE_UNSUPPORTED when there is none */
static CORE::TDataTableLogicalType
GetTypeOption( const CORE::CValueList& options, const CORE::CString& name )
{
    CORE::TDataTableLogicalType type = CORE::DATATABLE_TYPE_UNSUPPORTED;
    if ( options.HasKey( name ) )
        CORE::DataTableLogicalTypeFromString( options.GetValueAlways( name ).AsString(), type );
    return type;
}

/*-------------------------------------------------------------------------*/

/** The state of a replica, read under the lock of the node that changes it */
static bool
IsSnapshotWaitOver( CDRNSession& session, const CDRNDataGrid& grid, bool& removed )
{
    MT::CScopeMutex lock( session.GetNode().GetLock() );
    removed = grid.IsRemoved();
    return removed || grid.IsSnapshotComplete();
}

/*-------------------------------------------------------------------------*/

/** Pause while a replica gets its snapshot */
static const UInt32 SnapshotPollIntervalMs = 2;

/** Most a table waits for the hand over of a snapshot that is complete, in milliseconds */
static const UInt32 HandOverWaitMs = 250;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNDatabaseClient::BackendName = "drn";

/*-------------------------------------------------------------------------*/

void
CDRNDatabaseClient::CSessionWatcher::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{GUCEF_TRACE;

    if ( CORE::CNotifier::DestructionEvent == eventid && notifier == m_session && GUCEF_NULL != m_client )
        m_client->OnSessionDestroyed();
}

/*-------------------------------------------------------------------------*/

CDRNDatabaseClient::CDRNDatabaseClient( void )
    : COMCORE::CDatabaseClientBase()
    , m_lock()
    , m_session( GUCEF_NULL )
    , m_attached( false )
    , m_sessionWatcher()
    , m_tableSettings()
    , m_errors()
{GUCEF_TRACE;

    m_sessionWatcher.m_client = this;
}

/*-------------------------------------------------------------------------*/

CDRNDatabaseClient::~CDRNDatabaseClient()
{GUCEF_TRACE;

    // the queued and running async calls call into this object; no events from an object that is going
    WaitForAsyncOperations();
    m_sessionWatcher.m_client = GUCEF_NULL;
    ReleaseSession( false );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_errors[ MT::GetCurrentTaskID() ] = message;
    }
    CORE::TCloneableString eventData( message );
    const_cast< CDRNDatabaseClient* >( this )->NotifyObserversFromThread( DatabaseErrorEvent, &eventData );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_errors.erase( MT::GetCurrentTaskID() );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNDatabaseClient::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TErrorMap::const_iterator i = m_errors.find( MT::GetCurrentTaskID() );
    return i == m_errors.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDatabaseClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNDatabaseClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNDatabaseClient::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

UInt32
CDRNDatabaseClient::GetCapabilities( void ) const
{GUCEF_TRACE;

    UInt32 capabilities = GUCEF_DATABASE_CAPABILITY_PREDICATE_PUSHDOWN | GUCEF_DATABASE_CAPABILITY_ASYNC;
    if ( !GetSettings().readOnly )
        capabilities |= GUCEF_DATABASE_CAPABILITY_WRITE;
    return capabilities;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDatabaseClient::Attach( CDRNSession& session )
{GUCEF_TRACE;

    ReleaseSession( true );
    {
        MT::CScopeMutex lock( m_lock );
        m_session = &session;
        m_attached = true;
    }
    m_sessionWatcher.m_session = &session;
    m_sessionWatcher.SubscribeTo( &session, CDRNSession::SessionClosedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDatabaseClient::SetTableSettings( const CORE::CString& tableName, const CDRNDataGridTableSettings& settings )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_tableSettings[ tableName ] = settings;
}

/*-------------------------------------------------------------------------*/

CDRNDataGridTableSettings
CDRNDatabaseClient::GetTableSettings( const CORE::CString& tableName ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TTableSettingsMap::const_iterator i = m_tableSettings.find( tableName );
        if ( i != m_tableSettings.end() )
            return (*i).second;
    }

    // the defaults of the backend options
    const CORE::CValueList& options = GetSettings().backendOptions;
    CDRNDataGridTableSettings settings;
    settings.readOnly = GetSettings().readOnly;
    if ( options.HasKey( "includeRemoved" ) )
        settings.includeRemoved = options.GetValueAlways( "includeRemoved" ).AsBool( settings.includeRemoved );
    if ( options.HasKey( "scanChunkKeys" ) )
        settings.scanChunkKeyCount = options.GetValueAlways( "scanChunkKeys" ).AsUInt32( settings.scanChunkKeyCount );
    if ( options.HasKey( "snapshotCacheMs" ) )
        settings.snapshotCacheMs = options.GetValueAlways( "snapshotCacheMs" ).AsUInt32( settings.snapshotCacheMs );
    if ( options.HasKey( "writeTimeoutMs" ) )
        settings.writeTimeoutMs = options.GetValueAlways( "writeTimeoutMs" ).AsUInt32( settings.writeTimeoutMs );
    settings.keyType = GetTypeOption( options, tableName + ".keyType" );
    settings.valueType = GetTypeOption( options, tableName + ".valueType" );
    return settings;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDatabaseClient::Open( const COMCORE::CDatabaseClientSettings& settings )
{GUCEF_TRACE;

    if ( settings.backendName != BackendName )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The settings are for backend \"" + settings.backendName + "\", not \"" + BackendName + "\"" );
        return false;
    }

    // an attached session is the database: the settings need not name one
    COMCORE::CDatabaseClientSettings checked = settings;
    CDRNSession* attached = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        if ( m_attached )
            attached = m_session;
    }
    if ( GUCEF_NULL != attached && checked.database.IsNULLOrEmpty() )
        checked.database = attached->GetName().IsNULLOrEmpty() ? CORE::CString( "session" ) : attached->GetName();
    CORE::CString error;
    if ( !checked.Validate( error ) )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
        return false;
    }

    CDRNSession* session = attached;
    if ( GUCEF_NULL == session )
    {
        session = CDRNGlobal::Instance()->GetSessionRegistry().Find( settings.database );
        if ( GUCEF_NULL == session )
        {
            Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No session named \"" + settings.database + "\" is registered: register the session with CDRNGlobal::GetSessionRegistry() or Attach() it" );
            return false;
        }
        // not through Close(), which waits for the async calls and Open may be one of them
        ReleaseSession( true );
        {
            MT::CScopeMutex lock( m_lock );
            m_session = session;
            m_attached = false;
        }
        m_sessionWatcher.m_session = session;
        m_sessionWatcher.SubscribeTo( session, CDRNSession::SessionClosedEvent );
    }

    SetSettings( settings );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "DRNDatabaseClient: opened session \"" << session->GetName() << "\"" << ( settings.readOnly ? " read only" : "" ) );
    Succeed();
    NotifyObserversFromThread( DatabaseOpenedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNDatabaseClient::Close( void )
{GUCEF_TRACE;

    WaitForAsyncOperations();
    ReleaseSession( true );
}

/*-------------------------------------------------------------------------*/

void
CDRNDatabaseClient::ReleaseSession( bool notify )
{GUCEF_TRACE;

    CDRNSession* session = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        session = m_session;
        m_session = GUCEF_NULL;
        m_attached = false;
    }
    if ( GUCEF_NULL == session )
        return;
    m_sessionWatcher.UnsubscribeFrom( *session );
    if ( notify )
        NotifyObserversFromThread( DatabaseClosedEvent );
}

/*-------------------------------------------------------------------------*/

void
CDRNDatabaseClient::OnSessionDestroyed( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_session = GUCEF_NULL;
    m_attached = false;
}

/*-------------------------------------------------------------------------*/

bool
CDRNDatabaseClient::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_session && !GetSettings().backendName.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CDRNSession*
CDRNDatabaseClient::GetOpenSession( CORE::TDataTableStatus& status ) const
{GUCEF_TRACE;

    CDRNSession* session = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        session = m_session;
    }
    status = GUCEF_NULL == session || GetSettings().backendName.IsNULLOrEmpty() ? Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" ) : CORE::DATATABLE_STATUS_OK;
    return CORE::DATATABLE_STATUS_OK == status ? session : GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::ListTables( CORE::CString::StringSet& tableNames )
{GUCEF_TRACE;

    tableNames.clear();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CDRNSession* session = GetOpenSession( status );
    if ( GUCEF_NULL == session )
        return status;

    TDRNItemInfoVector items;
    session->GetReplicator().GetItems( items );
    for ( UInt32 i=0; i<(UInt32) items.size(); ++i )
        if ( CDRNDataItem::ITEMKIND_GRID == items[ i ].kind )
            tableNames.insert( items[ i ].name );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CDRNSession* session = GetOpenSession( status );
    if ( GUCEF_NULL == session )
        return status;
    CDRNDataGrid* grid = session->FindGrid( tableName );
    if ( GUCEF_NULL == grid )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The session has no grid \"" + tableName + "\"" );

    CDRNDataGridTable table( *session, *grid, GetTableSettings( tableName ), GetSettings().pulseGenerator );
    definition = table.GetDefinition();
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    table.Unlink();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CDRNSession* session = GetOpenSession( status );
    if ( GUCEF_NULL == session )
        return status;
    CDRNDataGrid* grid = session->FindGrid( tableName );
    if ( GUCEF_NULL == grid )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The session has no grid \"" + tableName + "\"" );

    // a grid that was announced and is not subscribed yet is subscribed now; the table is there when its snapshot is
    bool removed = false;
    if ( !IsSnapshotWaitOver( *session, *grid, removed ) || removed )
    {
        session->Subscribe( tableName, grid->GetOwnerPeerId() );
        UInt32 timeoutMs = DefaultOpenTimeoutMs;
        if ( GetSettings().backendOptions.HasKey( "openTimeoutMs" ) )
            timeoutMs = GetSettings().backendOptions.GetValueAlways( "openTimeoutMs" ).AsUInt32( timeoutMs );
        const UInt64 start = CDRNClock::NowInMicroseconds();
        while ( !IsSnapshotWaitOver( *session, *grid, removed ) )
        {
            if ( 0 != timeoutMs && CDRNClock::NowInMicroseconds() - start >= (UInt64) timeoutMs * 1000 )
                return Fail( CORE::DATATABLE_STATUS_FAILED, "The snapshot of grid \"" + tableName + "\" did not arrive within " + CORE::UInt32ToString( timeoutMs ) +
                             " ms; OpenTable() does not drive the node: use OpenTableAsync() from the thread that does" );
            MT::ThreadDelay( SnapshotPollIntervalMs );
        }
        if ( removed )
            return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The grid \"" + tableName + "\" went away before its snapshot arrived" );

        // the snapshot is complete in the middle of a tick of the node and handed to the application at the end of a
        // tick: a grid that has keys has had its first update tick then. Grids with keys that have no value stay at
        // tick 0, so the wait for the hand over is short
        const UInt64 settleStart = CDRNClock::NowInMicroseconds();
        while ( 0 == grid->GetUpdateTick() && grid->GetKeyByIndex( 0 ).IsInitialized() && CDRNClock::NowInMicroseconds() - settleStart < (UInt64) HandOverWaitMs * 1000 )
            MT::ThreadDelay( 1 );
    }

    table = CORE::CIDataTablePtr( GUCEF_NEW CDRNDataGridTable( *session, *grid, GetTableSettings( tableName ), GetSettings().pulseGenerator ) );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    table.Unlink();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CDRNSession* session = GetOpenSession( status );
    if ( GUCEF_NULL == session )
        return status;
    if ( GetSettings().readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database was opened read only" );

    const CORE::CDataTableSchema& schema = definition.schema;
    if ( definition.name.IsNULLOrEmpty() || schema.GetColumnCount() < 2 || schema.GetColumn( 0 ).name != CDRNDataGridTable::KeyColumnName ||
         schema.GetColumn( 1 ).name != CDRNDataGridTable::ValueColumnName )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "A grid table has a name and starts with the columns \"key\" and \"value\"" );
    CDRNDataGrid* existing = session->FindGrid( definition.name, session->GetLocalPeerId() );
    if ( GUCEF_NULL != existing && !existing->IsRemoved() )
        return Fail( CORE::DATATABLE_STATUS_ALREADY_EXISTS, "This member publishes a grid \"" + definition.name + "\" already" );

    CDRNDataGridSettings gridSettings;
    gridSettings.name = definition.name;
    gridSettings.keyType = schema.GetColumn( 0 ).type;
    gridSettings.valueType = schema.GetColumn( 1 ).type;
    const CORE::CValueList& options = definition.backendOptions;
    if ( options.HasKey( "authority" ) )
        gridSettings.authority = CDRNDataGridSettings::StringToAuthority( options.GetValueAlways( "authority" ).AsString(), gridSettings.authority );
    if ( options.HasKey( "reliable" ) )
        gridSettings.reliable = options.GetValueAlways( "reliable" ).AsBool( gridSettings.reliable );
    if ( options.HasKey( "sendRateHz" ) )
        gridSettings.sendRateHz = options.GetValueAlways( "sendRateHz" ).AsUInt32( gridSettings.sendRateHz );
    if ( options.HasKey( "capacity" ) )
        gridSettings.capacity = options.GetValueAlways( "capacity" ).AsUInt32( gridSettings.capacity );
    if ( options.HasKey( "notify" ) )
        gridSettings.notify = CDRNDataGridSettings::StringToNotifyMode( options.GetValueAlways( "notify" ).AsString(), gridSettings.notify );
    if ( options.HasKey( "priority" ) )
        gridSettings.priority = options.GetValueAlways( "priority" ).AsFloat32( gridSettings.priority );

    CDRNDataGrid* grid = session->CreateGrid( gridSettings );
    if ( GUCEF_NULL == grid )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "This member may not create the grid \"" + definition.name + "\": a grid with " + CDRNDataGridSettings::AuthorityToString( gridSettings.authority ) +
                     " authority is made by the sequencer, and the session has to be active" );

    table = CORE::CIDataTablePtr( GUCEF_NEW CDRNDataGridTable( *session, *grid, GetTableSettings( definition.name ), GetSettings().pulseGenerator ) );
    CORE::TCloneableString eventData( definition.name );
    NotifyObserversFromThread( TableCreatedEvent, &eventData );
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CDRNDatabaseClient::DropTable( const CORE::CString& tableName, bool /* deleteData */ )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    CDRNSession* session = GetOpenSession( status );
    if ( GUCEF_NULL == session )
        return status;
    if ( GetSettings().readOnly )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database was opened read only" );
    CDRNDataGrid* grid = session->FindGrid( tableName );
    if ( GUCEF_NULL == grid || grid->IsRemoved() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The session has no grid \"" + tableName + "\"" );
    if ( !grid->IsOrigin() )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "Grid \"" + tableName + "\" is published by another member: only the publisher drops it" );
    if ( !session->GetReplicator().RemoveItem( grid ) )
        return Fail( CORE::DATATABLE_STATUS_FAILED, "Grid \"" + tableName + "\" could not be withdrawn" );

    CORE::TCloneableString eventData( tableName );
    NotifyObserversFromThread( TableDroppedEvent, &eventData );
    return Succeed();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
