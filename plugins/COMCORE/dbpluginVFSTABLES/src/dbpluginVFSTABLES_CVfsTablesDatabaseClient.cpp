/*
 *  dbpluginVFSTABLES: GUCEF database backend over table files in the VFS
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

#ifndef GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#include "gucefCORE_CDataTableAsyncOperation.h"
#define GUCEF_CORE_CDATATABLEASYNCOPERATION_H
#endif /* GUCEF_CORE_CDATATABLEASYNCOPERATION_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#include "dbpluginVFSTABLES_CVfsDataTable.h"
#define DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSDATATABLE_H ? */

#include "dbpluginVFSTABLES_CVfsTablesDatabaseClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DBPLUGIN {
namespace VFSTABLES {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CVfsTablesDatabaseClient::BackendName = "vfstables";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The bookkeeping of the module: the roots open for writing and the live clients and tables */
struct SModuleState
{
    MT::CMutex lock;
    CORE::CString::StringSet writeRoots;
    GUCEF::map< CORE::CString, UInt32 > liveObjects;
};

SModuleState&
GetModuleState( void )
{
    static SModuleState state;
    return state;
}

/** A VFS root in one form, for comparing roots */
CORE::CString
ConformRoot( const CORE::CString& root )
{
    CORE::CString conformed = root.ReplaceChar( '\\', '/' );
    while ( conformed.Length() > 1 && conformed.HasChar( '/', false ) == (Int32) conformed.Length() - 1 )
        conformed = conformed.CutChars( 1, false );
    return conformed;
}

/** The table name in a file name of the definitions directory, empty for other files */
CORE::CString
TableNameOfDefinitionFile( const CORE::CString& fileName )
{
    static const CORE::CString suffix = ".json";
    if ( fileName.Length() <= suffix.Length() || CVfsTablesStore::IsTemporaryFileName( fileName ) )
        return CORE::CString::Empty;
    if ( fileName.CutChars( fileName.Length() - suffix.Length(), true ) != suffix )
        return CORE::CString::Empty;
    const CORE::CString name = fileName.CutChars( suffix.Length(), false );
    return CORE::CDataTableDefinition::IsValidTableName( name ) ? name : CORE::CString::Empty;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CVfsTablesDatabaseClient::OnObjectCreated( const CORE::CString& description )
{GUCEF_TRACE;

    SModuleState& state = GetModuleState();
    MT::CScopeMutex lock( state.lock );
    ++state.liveObjects[ description ];
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesDatabaseClient::OnObjectDestroyed( const CORE::CString& description )
{GUCEF_TRACE;

    SModuleState& state = GetModuleState();
    MT::CScopeMutex lock( state.lock );
    GUCEF::map< CORE::CString, UInt32 >::iterator i = state.liveObjects.find( description );
    if ( i != state.liveObjects.end() && 0 == --(*i).second )
        state.liveObjects.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesDatabaseClient::GetLiveObjects( CORE::CString::StringVector& descriptions )
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

CVfsTablesDatabaseClient::CVfsTablesDatabaseClient( void )
    : COMCORE::CDatabaseClientBase()
    , m_lock()
    , m_store()
    , m_exclusiveRoot()
    , m_errors()
{GUCEF_TRACE;

    OnObjectCreated( "database client" );
}

/*-------------------------------------------------------------------------*/

CVfsTablesDatabaseClient::~CVfsTablesDatabaseClient()
{GUCEF_TRACE;

    // the queued and running async calls call into this object; no events from an object that is going
    WaitForAsyncOperations();
    ReleaseStore( false );
    OnObjectDestroyed( "database client" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::Fail( CORE::TDataTableStatus status, const CORE::CString& message ) const
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_errors[ MT::GetCurrentTaskID() ] = message;
    }
    CORE::TCloneableString eventData( message );
    const_cast< CVfsTablesDatabaseClient* >( this )->NotifyObserversFromThread( DatabaseErrorEvent, &eventData );
    return status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::Succeed( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_errors.erase( MT::GetCurrentTaskID() );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsTablesDatabaseClient::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TErrorMap::const_iterator i = m_errors.find( MT::GetCurrentTaskID() );
    return i == m_errors.end() ? CORE::CString::Empty : (*i).second;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesDatabaseClient::Open( const COMCORE::CDatabaseClientSettings& settings )
{GUCEF_TRACE;

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
    CVfsTablesSettings backendSettings;
    if ( !backendSettings.LoadFromBackendOptions( settings.backendOptions, error ) )
    {
        Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
        return false;
    }

    // an open database goes first; not through Close(), which waits for the async calls and Open may be one of them
    ReleaseStore( true );

    const CORE::CString root = ConformRoot( settings.database );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( !vfs.DirExists( root ) )
    {
        Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "The database root \"" + root + "\" is not a directory of the VFS" );
        return false;
    }
    if ( !settings.readOnly )
    {
        if ( !vfs.IsWriteable( root ) )
        {
            Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database root \"" + root + "\" is not in a writeable mount; open it read only" );
            return false;
        }
        // one writer per root in this process (the single writer rule of the design, 8)
        SModuleState& state = GetModuleState();
        MT::CScopeMutex lock( state.lock );
        if ( state.writeRoots.find( root ) != state.writeRoots.end() )
        {
            lock.EarlyUnlock();
            Fail( CORE::DATATABLE_STATUS_ALREADY_EXISTS, "The database root \"" + root + "\" is open for writing already" );
            return false;
        }
        state.writeRoots.insert( root );
    }

    CORE::ThreadPoolPtr threadPool = settings.threadPool;
    if ( threadPool.IsNULL() )
        threadPool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( settings.threadPoolName, true );

    TVfsTablesStorePtr store( GUCEF_NEW CVfsTablesStore( root, settings.readOnly, backendSettings, settings.bufferPoolMaxBytes, threadPool ) );
    if ( !settings.readOnly )
        store->RemoveLeftoverTemporaryFiles();

    SetSettings( settings );
    {
        MT::CScopeMutex lock( m_lock );
        m_store = store;
        m_exclusiveRoot = settings.readOnly ? CORE::CString::Empty : root;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "VfsTables: opened \"" << root << "\"" << ( settings.readOnly ? " read only" : " for writing" ) <<
               " as writer \"" << backendSettings.writerId << "\"" );
    Succeed();
    NotifyObserversFromThread( DatabaseOpenedEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesDatabaseClient::Close( void )
{GUCEF_TRACE;

    WaitForAsyncOperations();
    ReleaseStore( true );
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesDatabaseClient::ReleaseStore( bool notify )
{GUCEF_TRACE;

    TVfsTablesStorePtr store;
    CORE::CString exclusiveRoot;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_store;
        exclusiveRoot = m_exclusiveRoot;
        m_store.Unlink();
        m_exclusiveRoot.Clear();
    }
    if ( store.IsNULL() )
        return;
    if ( !exclusiveRoot.IsNULLOrEmpty() )
    {
        SModuleState& state = GetModuleState();
        MT::CScopeMutex lock( state.lock );
        state.writeRoots.erase( exclusiveRoot );
    }
    store.Unlink();
    if ( notify )
        NotifyObserversFromThread( DatabaseClosedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesDatabaseClient::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_store.IsNULL();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsTablesDatabaseClient::GetBackendName( void ) const
{GUCEF_TRACE;

    return BackendName;
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsTablesDatabaseClient::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_DATABASE_CAPABILITY_WRITE | GUCEF_DATABASE_CAPABILITY_PARTITIONED_TABLES | GUCEF_DATABASE_CAPABILITY_PREDICATE_PUSHDOWN |
           GUCEF_DATABASE_CAPABILITY_ASYNC | GUCEF_DATABASE_CAPABILITY_SCHEMA_EVOLUTION;
}

/*-------------------------------------------------------------------------*/

TVfsTablesStorePtr
CVfsTablesDatabaseClient::GetOpenStore( CORE::TDataTableStatus& status ) const
{GUCEF_TRACE;

    TVfsTablesStorePtr store;
    {
        MT::CScopeMutex lock( m_lock );
        store = m_store;
    }
    status = store.IsNULL() ? Fail( CORE::DATATABLE_STATUS_FAILED, "The database is not open" ) : CORE::DATATABLE_STATUS_OK;
    return store;
}

/*-------------------------------------------------------------------------*/

TVfsTablesStorePtr
CVfsTablesDatabaseClient::GetStore( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_store;
}

/*-------------------------------------------------------------------------*/

bool
CVfsTablesDatabaseClient::GetStorageStats( CVfsTablesStats& stats ) const
{GUCEF_TRACE;

    TVfsTablesStorePtr store = GetStore();
    if ( store.IsNULL() )
        return false;
    store->GetStats( stats );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CVfsTablesDatabaseClient::ResetStorageStats( void )
{GUCEF_TRACE;

    TVfsTablesStorePtr store = GetStore();
    if ( !store.IsNULL() )
        store->ResetStats();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CVfsTablesDatabaseClient::GetDefinitionPath( TVfsTablesStorePtr store, const CORE::CString& tableName ) const
{GUCEF_TRACE;

    return store->MakePath( CVfsTablesStore::TablesDirName + "/" + tableName + ".json" );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::ListTables( CORE::CString::StringSet& tableNames )
{GUCEF_TRACE;

    tableNames.clear();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    TVfsTablesStorePtr store = GetOpenStore( status );
    if ( store.IsNULL() )
        return status;

    CORE::CString::StringVector files;
    store->ListDirectory( store->MakePath( CVfsTablesStore::TablesDirName ), false, files );
    for ( UInt32 i=0; i<(UInt32) files.size(); ++i )
    {
        const CORE::CString name = TableNameOfDefinitionFile( files[ i ] );
        if ( !name.IsNULLOrEmpty() )
            tableNames.insert( name );
    }
    return Succeed();
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::LoadDefinition( TVfsTablesStorePtr store, const CORE::CString& tableName, CORE::CDataTableDefinition& definition ) const
{GUCEF_TRACE;

    if ( !CORE::CDataTableDefinition::IsValidTableName( tableName ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "\"" + tableName + "\" is not a valid table name" );

    const CORE::CString path = GetDefinitionPath( store, tableName );
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( path ) )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table \"" + tableName + "\" in \"" + store->GetRoot() + "\"" );

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) )
        return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "Table definitions are JSON and no JSON DStore codec is registered" );

    CORE::CDynamicBuffer content;
    if ( !VFS::CVfsGlobal::Instance()->GetVfs().LoadFile( content, path, "rb" ) )
        return Fail( CORE::DATATABLE_STATUS_IO_ERROR, "The definition \"" + path + "\" cannot be read" );
    CORE::CDynamicBufferAccess input( content );
    CORE::CDataNode tree;
    if ( !json->BuildDataTree( &tree, &input ) )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The definition \"" + path + "\" is not valid JSON" );
    const CORE::CDataNode* root = tree.FindChild( "DataTableDefinition" );
    if ( GUCEF_NULL == root )
        root = &tree;

    CORE::CDataTableDefinition loaded;
    CORE::CString error;
    if ( !loaded.Deserialize( *root, CORE::CDataNodeSerializableSettings() ) || !loaded.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The definition \"" + path + "\" is not a valid table definition: " + error );
    if ( loaded.name != tableName )
        return Fail( CORE::DATATABLE_STATUS_CORRUPT_DATA, "The definition \"" + path + "\" is of table \"" + loaded.name + "\"" );
    definition = loaded;
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::StoreDefinition( TVfsTablesStorePtr store, const CORE::CDataTableDefinition& definition ) const
{GUCEF_TRACE;

    CORE::CDStoreCodecRegistry::TDStoreCodecPtr json;
    if ( !CORE::CCoreGlobal::Instance()->GetDStoreCodecRegistry().TryGetJsonCodec( json ) )
        return Fail( CORE::DATATABLE_STATUS_UNSUPPORTED, "Table definitions are JSON and no JSON DStore codec is registered" );

    CORE::CDataNode tree;
    CORE::CDynamicBuffer content;
    {
        CORE::CDynamicBufferAccess output( &content, false );
        if ( !definition.Serialize( tree, CORE::CDataNodeSerializableSettings() ) || !json->StoreDataTree( &tree, &output ) )
            return Fail( CORE::DATATABLE_STATUS_FAILED, "The definition of table \"" + definition.name + "\" could not be serialized" );
    }

    // written as partitions are: a temporary file moved over the definition
    const CORE::CString path = GetDefinitionPath( store, definition.name );
    const CORE::CString temporaryPath = store->MakeTemporaryPath( path );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    bool written = false;
    {
        VFS::TBasicVfsResourcePtr resource = vfs.GetFile( temporaryPath, "wb", true );
        CORE::IOAccessPtr access;
        if ( !resource.IsNULL() )
            access = resource->GetAccess();
        if ( !access.IsNULL() )
        {
            written = content.GetDataSize() == access->Write( content.GetConstBufferPtr(), 1, content.GetDataSize() );
            written = store->FinishTemporaryFile( access ) && written;
        }
    }
    CORE::CString error;
    if ( !written )
    {
        store->RemoveTemporaryFile( temporaryPath );
        return Fail( CORE::DATATABLE_STATUS_IO_ERROR, "The definition \"" + path + "\" could not be written" );
    }
    const CORE::TDataTableStatus status = store->ReplaceFile( temporaryPath, path, error );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        store->RemoveTemporaryFile( temporaryPath );
        return Fail( status, error );
    }
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::LookupCodec( const CORE::CString& codecType, CVfsTablesStore::TTableCodecPtr& codec ) const
{GUCEF_TRACE;

    if ( !CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryLookup( codecType, codec, false ) || codec.IsNULL() )
        return Fail( CORE::DATATABLE_STATUS_NOT_FOUND, "No table codec \"" + codecType + "\" is registered" );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::MakeTable( TVfsTablesStorePtr store, const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table ) const
{GUCEF_TRACE;

    CVfsTablesStore::TTableCodecPtr codec;
    const CORE::TDataTableStatus status = LookupCodec( definition.codecType, codec );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    table = CORE::CIDataTablePtr( GUCEF_NEW CVfsDataTable( store, definition, codec, GetSettings().pulseGenerator ) );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::GetTableDefinition( const CORE::CString& tableName, CORE::CDataTableDefinition& definition )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    TVfsTablesStorePtr store = GetOpenStore( status );
    if ( store.IsNULL() )
        return status;
    status = LoadDefinition( store, tableName, definition );
    return CORE::DATATABLE_STATUS_OK == status ? Succeed() : status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::OpenTable( const CORE::CString& tableName, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    table.Unlink();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    TVfsTablesStorePtr store = GetOpenStore( status );
    if ( store.IsNULL() )
        return status;
    CORE::CDataTableDefinition definition;
    status = LoadDefinition( store, tableName, definition );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;
    status = MakeTable( store, definition, table );
    return CORE::DATATABLE_STATUS_OK == status ? Succeed() : status;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::CreateTable( const CORE::CDataTableDefinition& definition, CORE::CIDataTablePtr& table )
{GUCEF_TRACE;

    table.Unlink();
    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    TVfsTablesStorePtr store = GetOpenStore( status );
    if ( store.IsNULL() )
        return status;
    if ( store->IsReadOnly() )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database \"" + store->GetRoot() + "\" is open read only" );

    CORE::CString error;
    if ( !definition.Validate( error ) )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, error );
    CORE::CString::StringVector firstFields;
    CORE::CString firstSegment;
    definition.partitioning.GetSegmentFields( 0, firstFields );
    if ( definition.partitioning.IsPartitioned() && firstFields.empty() && definition.partitioning.BuildSegment( 0, CORE::CValueList(), firstSegment ) &&
         CVfsTablesStore::TablesDirName == firstSegment )
        return Fail( CORE::DATATABLE_STATUS_INVALID_ARGUMENT, "The path template of table \"" + definition.name + "\" starts in the directory of the table definitions" );
    CVfsTablesStore::TTableCodecPtr codec;
    status = LookupCodec( definition.codecType, codec );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    {
        // the definition is created under the write lock of the table, so two creates of one name cannot both succeed
        MT::CScopeMutex writeLock( store->GetTableWriteLock( definition.name ), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        if ( VFS::CVfsGlobal::Instance()->GetVfs().FileExists( GetDefinitionPath( store, definition.name ) ) )
            return Fail( CORE::DATATABLE_STATUS_ALREADY_EXISTS, "The table \"" + definition.name + "\" exists" );
        status = StoreDefinition( store, definition );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
    }
    status = MakeTable( store, definition, table );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    Succeed();
    CORE::TCloneableString eventData( definition.name );
    NotifyObserversFromThread( TableCreatedEvent, &eventData );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CVfsTablesDatabaseClient::DropTable( const CORE::CString& tableName, bool deleteData )
{GUCEF_TRACE;

    CORE::TDataTableStatus status = CORE::DATATABLE_STATUS_OK;
    TVfsTablesStorePtr store = GetOpenStore( status );
    if ( store.IsNULL() )
        return status;
    if ( store->IsReadOnly() )
        return Fail( CORE::DATATABLE_STATUS_READ_ONLY, "The database \"" + store->GetRoot() + "\" is open read only" );

    CORE::CDataTableDefinition definition;
    status = LoadDefinition( store, tableName, definition );
    if ( CORE::DATATABLE_STATUS_OK != status )
        return status;

    if ( deleteData )
    {
        CORE::CIDataTablePtr table;
        status = MakeTable( store, definition, table );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return status;
        CORE::CDataTableWriteResult result;
        status = table->DeletePartitions( CORE::CDataTableQuery(), result );
        if ( CORE::DATATABLE_STATUS_OK != status )
            return Fail( status, table->GetLastErrorMessage() );
    }

    CORE::CString error;
    {
        MT::CScopeMutex writeLock( store->GetTableWriteLock( tableName ), GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        status = store->DeleteFile( GetDefinitionPath( store, tableName ), error );
    }
    if ( CORE::DATATABLE_STATUS_OK != status )
        return Fail( status, error );

    Succeed();
    CORE::TCloneableString eventData( tableName );
    NotifyObserversFromThread( TableDroppedEvent, &eventData );
    return CORE::DATATABLE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CVfsTablesDatabaseClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFSTABLES */
}; /* namespace DBPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
