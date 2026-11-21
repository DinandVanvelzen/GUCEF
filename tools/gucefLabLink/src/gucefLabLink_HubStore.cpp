/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CPLUGINMETADATA_H
#include "gucefCORE_CPluginMetaData.h"
#define GUCEF_CORE_CPLUGINMETADATA_H
#endif /* GUCEF_CORE_CPLUGINMETADATA_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCH_H
#include "gucefCORE_CDataTableBatch.h"
#define GUCEF_CORE_CDATATABLEBATCH_H
#endif /* GUCEF_CORE_CDATATABLEBATCH_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef GUCEF_CORE_CDATATABLEROWCURSOR_H
#include "gucefCORE_CDataTableRowCursor.h"
#define GUCEF_CORE_CDATATABLEROWCURSOR_H
#endif /* GUCEF_CORE_CDATATABLEROWCURSOR_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITERESULT_H
#include "gucefCORE_CDataTableWriteResult.h"
#define GUCEF_CORE_CDATATABLEWRITERESULT_H
#endif /* GUCEF_CORE_CDATATABLEWRITERESULT_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "gucefLabLink_HubStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const Int64 SchemaVersion = 1;
static const UInt32 RowsPerWrite = 500;

static const char* NodesColumns =
    "machine_id:utf8!,name:utf8,os:utf8,arch:utf8,agent_version:utf8,verified:bool,address:utf8,first_seen:int64,last_seen:int64,liveness:utf8,"
    "node_state:utf8,presence:utf8,instance_id:utf8,started_at:int64,boot_id:utf8,uptime_secs:int64,ephemeral:bool,idle_secs:int64,cpu_cores:int64,"
    "cpu_load:int64,mem_total_mib:int64,mem_available_mib:int64,disk_free_mib:int64,details:utf8";
static const char* EventsColumns = "event_id:utf8!,ts:int64!,machine_id:utf8!,kind:utf8,detail:utf8";
static const char* VersionColumns = "version:int64!,applied:int64";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A table definition: columns as "name:type" (utf8, int64, bool; a '!' after the type: not nullable), the primary key, index hints "name=column+column;..." */
static CORE::CDataTableDefinition
MakeDefinition( const char* name, const char* columns, const char* primaryKey, const char* indexes )
{GUCEF_TRACE;

    CORE::CDataTableDefinition definition;
    definition.name = name;
    const CORE::CString::StringVector columnSpecs = CORE::CString( columns ).ParseElements( ',', false );
    for ( size_t c=0; c<columnSpecs.size(); ++c )
    {
        const CORE::CString columnName = columnSpecs[ c ].SubstrToChar( ':', true );
        CORE::CString typeText = columnSpecs[ c ].SubstrToChar( ':', false );
        bool nullable = true;
        if ( typeText.HasChar( '!' ) >= 0 )
        {
            nullable = false;
            typeText = typeText.RemoveChar( '!' );
        }
        CORE::TDataTableLogicalType type = CORE::DATATABLE_TYPE_UTF8;
        if ( typeText == "int64" )
            type = CORE::DATATABLE_TYPE_INT64;
        else if ( typeText == "bool" )
            type = CORE::DATATABLE_TYPE_BOOL;
        definition.schema.AddColumn( CORE::CDataTableColumnDef( columnName, type, nullable ) );
    }
    definition.schema.SetPrimaryKey( CORE::CString( primaryKey ).ParseElements( ',', false ) );
    const CORE::CString::StringVector indexSpecs = CORE::CString( indexes ).ParseElements( ';', false );
    for ( size_t i=0; i<indexSpecs.size(); ++i )
    {
        CORE::CDataTableIndexDef index;
        index.name = indexSpecs[ i ].SubstrToChar( '=', true );
        index.columns = indexSpecs[ i ].SubstrToChar( '=', false ).ParseElements( '+', false );
        definition.indexes.push_back( index );
    }
    return definition;
}

/*-------------------------------------------------------------------------*/

/** Loads a plugin next to the program, by its name or by the name of its debug build */
static bool
LoadPlugin( const CORE::CString& moduleName, const char* pluginType )
{GUCEF_TRACE;

    CORE::CPluginControl& plugins = CORE::CCoreGlobal::Instance()->GetPluginControl();
    CORE::CPluginMetaData metaData;
    metaData.SetFullModulePath( CORE::RelativePath( "$MODULEDIR$" ) );
    metaData.SetPluginType( pluginType );
    metaData.SetModuleFilename( moduleName );
    if ( plugins.AddPluginMetaData( metaData, "gucefLabLink-hub", true ) )
        return true;
    metaData.SetModuleFilename( moduleName + "_d" );
    return plugins.AddPluginMetaData( metaData, "gucefLabLink-hub", true );
}

/*-------------------------------------------------------------------------*/

static Int64
Timestamp( UInt64 unixSecs )
{
    return (Int64) unixSecs;
}

/*-------------------------------------------------------------------------*/

static CORE::CString
PaddedSeconds( UInt64 unixSecs )
{GUCEF_TRACE;

    CORE::CString text = CORE::ToString( unixSecs );
    while ( text.Length() < 12 )
        text = "0" + text;
    return text;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHubStore::CHubStore( const SSettings& settings )
    : m_settings( settings )
    , m_client()
    , m_nodes()
    , m_events()
    , m_version()
    , m_mountName()
    , m_eventCounter( 0 )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CHubStore::~CHubStore()
{GUCEF_TRACE;

    Close();
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::IsOpen( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_client.IsNULL() && m_client->IsOpen();
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::SetUpBackend( COMCORE::CDatabaseClientSettings& databaseSettings, CORE::CString& problem )
{GUCEF_TRACE;

    // the codec of the table definitions, which both backends keep as JSON
    if ( !LoadPlugin( "dstorepluginJSONPARSER", "GucefDataStoreCodecPlugin" ) )
    {
        problem = "the plugin dstorepluginJSONPARSER cannot be loaded";
        return false;
    }

    if ( m_settings.dbType == "vfstables" )
    {
        if ( !LoadPlugin( "tablecodecpluginPARQUET", "GucefTableCodecPlugin" ) )
        {
            problem = "the plugin tablecodecpluginPARQUET cannot be loaded";
            return false;
        }
        if ( !LoadPlugin( "dbpluginVFSTABLES", "GucefGenericPlugin" ) )
        {
            problem = "the plugin dbpluginVFSTABLES cannot be loaded";
            return false;
        }
        const CORE::CString directory = m_settings.dbDirectory.IsNULLOrEmpty() ? CORE::CombinePath( m_settings.stateDir, "db" ) : m_settings.dbDirectory;
        if ( !CORE::CreateDirs( CORE::CombinePath( directory, "db" ) ) )
        {
            problem = "the directory " + CORE::CombinePath( directory, "db" ) + " cannot be made";
            return false;
        }
        // one mount for every directory: a process can have several hubs, as the tests do
        static UInt32 mountCounter = 0;
        m_mountName = "gucefLabLinkDb" + CORE::ToString( ++mountCounter );
        const CORE::CString mountPoint = "lablink-db-" + CORE::ToString( mountCounter );
        if ( !VFS::CVfsGlobal::Instance()->GetVfs().AddRoot( directory, m_mountName, true, false, mountPoint ) )
        {
            problem = "the directory " + directory + " cannot be mounted";
            m_mountName.Clear();
            return false;
        }
        databaseSettings.backendName = "vfstables";
        databaseSettings.database = mountPoint + "/db";
        return true;
    }

    if ( m_settings.dbType == "postgresql" )
    {
        if ( !LoadPlugin( "dbpluginPOSTGRESQL", "GucefGenericPlugin" ) )
        {
            problem = "the plugin dbpluginPOSTGRESQL cannot be loaded";
            return false;
        }
        databaseSettings.backendName = "postgresql";
        databaseSettings.database = m_settings.dbName;
        databaseSettings.backendOptions.SetAllowMultipleValues( false );
        if ( !m_settings.dbHost.IsNULLOrEmpty() ) databaseSettings.backendOptions.Set( "host", CORE::CVariant( m_settings.dbHost ) );
        if ( 0 != m_settings.dbPort ) databaseSettings.backendOptions.Set( "port", CORE::CVariant( CORE::ToString( m_settings.dbPort ) ) );
        if ( !m_settings.dbUser.IsNULLOrEmpty() ) databaseSettings.backendOptions.Set( "user", CORE::CVariant( m_settings.dbUser ) );
        if ( !m_settings.dbSslMode.IsNULLOrEmpty() ) databaseSettings.backendOptions.Set( "sslMode", CORE::CVariant( m_settings.dbSslMode ) );
        if ( !m_settings.dbSchema.IsNULLOrEmpty() ) databaseSettings.backendOptions.Set( "schema", CORE::CVariant( m_settings.dbSchema ) );
        if ( !m_settings.dbPasswordFile.IsNULLOrEmpty() ) databaseSettings.backendOptions.Set( "passwordFile", CORE::CVariant( m_settings.dbPasswordFile ) );
        databaseSettings.backendOptions.Set( "applicationName", CORE::CVariant( "gucefLabLink" ) );
        return true;
    }

    problem = "the database type \"" + m_settings.dbType + "\" is not known (vfstables, postgresql, none)";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::EnsureTables( CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CDataTableDefinition definitions[ 3 ] = {
        MakeDefinition( "schema_version", VersionColumns, "version", "" ),
        MakeDefinition( "nodes", NodesColumns, "machine_id", "" ),
        MakeDefinition( "node_events", EventsColumns, "event_id", "by_node=machine_id;by_time=ts" ) };
    CORE::CIDataTablePtr* tables[ 3 ] = { &m_version, &m_nodes, &m_events };

    for ( int d=0; d<3; ++d )
    {
        CORE::TDataTableStatus status = m_client->CreateTable( definitions[ d ], *tables[ d ] );
        if ( CORE::DATATABLE_STATUS_ALREADY_EXISTS == status )
            status = m_client->OpenTable( definitions[ d ].name, *tables[ d ] );
        if ( CORE::DATATABLE_STATUS_OK != status || tables[ d ]->IsNULL() )
        {
            problem = "the table " + definitions[ d ].name + " cannot be made: " + m_client->GetLastErrorMessage();
            return false;
        }
    }

    // a database made by a newer version is not touched
    TRowVector versions;
    if ( !ReadRows( m_version, CORE::CDataTableQuery(), versions, problem ) )
        return false;
    Int64 newest = 0;
    for ( size_t i=0; i<versions.size(); ++i )
        if ( versions[ i ][ "version" ].AsInt64() > newest )
            newest = versions[ i ][ "version" ].AsInt64();
    if ( newest > SchemaVersion )
    {
        problem = "the database was made by a newer version of gucefLabLink (schema " + CORE::ToString( newest ) + ")";
        return false;
    }
    if ( newest < SchemaVersion )
    {
        TRowVector rows( 1 );
        rows[ 0 ][ "version" ] = CORE::CVariant( SchemaVersion );
        rows[ 0 ][ "applied" ] = CORE::CVariant( Timestamp( NowInUnixSecs() ) );
        if ( !WriteRows( m_version, rows, problem ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::Open( CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_client.IsNULL() && m_client->IsOpen() )
        return true;
    if ( m_settings.dbType == "none" )
    {
        problem = "the database is switched off (--db none)";
        return false;
    }

    COMCORE::CDatabaseClientSettings databaseSettings;
    if ( !SetUpBackend( databaseSettings, problem ) )
    {
        CloseUnlocked();
        return false;
    }
    m_client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( databaseSettings.backendName );
    if ( m_client.IsNULL() )
    {
        problem = "there is no database backend \"" + databaseSettings.backendName + "\"";
        CloseUnlocked();
        return false;
    }
    if ( !m_client->Open( databaseSettings ) )
    {
        problem = "the database cannot be opened: " + m_client->GetLastErrorMessage();
        CloseUnlocked();
        return false;
    }
    if ( !EnsureTables( problem ) )
    {
        CloseUnlocked();
        return false;
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "gucefLabLink: The nodes are kept in the " << databaseSettings.backendName << " database " << databaseSettings.database );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHubStore::CloseUnlocked( void )
{GUCEF_TRACE;

    m_nodes.Unlink();
    m_events.Unlink();
    m_version.Unlink();
    if ( !m_client.IsNULL() )
    {
        m_client->Close();
        m_client.Unlink();
    }
    if ( !m_mountName.IsNULLOrEmpty() )
    {
        VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( m_mountName );
        m_mountName.Clear();
    }
}

/*-------------------------------------------------------------------------*/

void
CHubStore::Close( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CloseUnlocked();
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::WriteRows( CORE::CIDataTablePtr table, const TRowVector& rows, CORE::CString& problem )
{GUCEF_TRACE;

    const CORE::CDataTableSchemaPtr schema = table->GetSchema();
    const UInt32 columnCount = schema->GetColumnCount();
    GUCEF::vector< CORE::CVariant > values( columnCount );
    for ( size_t first=0; first<rows.size(); first+=RowsPerWrite )
    {
        CORE::CDataTableBatchBuilder builder( schema, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
        const size_t last = ( first + RowsPerWrite < rows.size() ) ? first + RowsPerWrite : rows.size();
        for ( size_t r=first; r<last; ++r )
        {
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                TRow::const_iterator cell = rows[ r ].find( schema->GetColumn( c ).name );
                values[ c ] = ( cell == rows[ r ].end() ) ? CORE::CVariant() : (*cell).second;
            }
            if ( !builder.AppendRow( &values[ 0 ], columnCount ) )
            {
                problem = "a row does not fit the table: " + CORE::CString( CORE::DataTableStatusToString( builder.GetLastStatus() ) );
                return false;
            }
        }
        CORE::CDataTableBatchPtr batch;
        CORE::CString error;
        if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) )
        {
            problem = "the rows cannot be put together: " + error;
            return false;
        }
        CORE::CDataTableWriteResult result;
        const CORE::TDataTableStatus status = table->Write( *batch, CORE::DATATABLE_WRITE_UPSERT, result );
        if ( CORE::DATATABLE_STATUS_OK != status || CORE::DATATABLE_STATUS_OK != result.GetOverallStatus() )
        {
            problem = "writing failed: " + table->GetLastErrorMessage();
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::ReadRows( CORE::CIDataTablePtr table, const CORE::CDataTableQuery& query, TRowVector& rows, CORE::CString& problem )
{GUCEF_TRACE;

    rows.clear();
    CORE::CDataTableScanPtr scan;
    CORE::TDataTableStatus status = table->Scan( query, scan );
    if ( CORE::DATATABLE_STATUS_OK != status || scan.IsNULL() )
    {
        problem = "reading failed: " + table->GetLastErrorMessage();
        return false;
    }
    for ( ;; )
    {
        CORE::CDataTableBatchPtr batch;
        status = scan->NextBatch( batch );
        if ( CORE::DATATABLE_STATUS_END == status )
            break;
        if ( CORE::DATATABLE_STATUS_OK != status )
        {
            problem = "reading failed: " + scan->GetLastErrorMessage();
            return false;
        }
        if ( batch.IsNULL() )
            continue;
        const CORE::CDataTableSchemaPtr& schema = batch->GetSchema();
        const UInt32 columnCount = batch->GetColumnCount();
        CORE::CString::StringVector names( columnCount );
        for ( UInt32 c=0; c<columnCount; ++c )
            names[ c ] = schema->GetColumn( batch->GetSchemaColumnIndex( c ) ).name;
        CORE::CDataTableRowCursor cursor( batch );
        while ( CORE::DATATABLE_STATUS_OK == cursor.Next() )
        {
            TRow row;
            for ( UInt32 c=0; c<columnCount; ++c )
            {
                CORE::CVariant value;
                if ( cursor.GetValue( c, value, false ) )
                    row[ names[ c ] ] = value;
            }
            rows.push_back( row );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::DeleteWhere( CORE::CIDataTablePtr table, const CORE::CDataTableQuery& query, CORE::CString& problem )
{GUCEF_TRACE;

    CORE::CDataTableWriteResult result;
    const CORE::TDataTableStatus status = table->DeleteRows( query, result );
    if ( CORE::DATATABLE_STATUS_OK != status || CORE::DATATABLE_STATUS_OK != result.GetOverallStatus() )
    {
        problem = "deleting failed: " + table->GetLastErrorMessage();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static const char*
LivenessText( ENodeLiveness liveness )
{
    return NODELIVENESS_ONLINE == liveness ? "online" : ( NODELIVENESS_OFFLINE == liveness ? "offline" : "unknown" );
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::SaveNodes( const vector< SNodeEntry >& nodes, CORE::CString& problem )
{GUCEF_TRACE;

    if ( nodes.empty() )
        return true;
    MT::CScopeMutex lock( m_lock );
    if ( m_nodes.IsNULL() )
    {
        problem = "the database is not open";
        return false;
    }
    TRowVector rows;
    for ( size_t i=0; i<nodes.size(); ++i )
    {
        const SNodeEntry& entry = nodes[ i ];
        const SNodeReport& report = entry.report;
        TRow row;
        row[ "machine_id" ] = CORE::CVariant( report.machineId );
        row[ "name" ] = CORE::CVariant( report.name );
        row[ "os" ] = CORE::CVariant( report.os );
        row[ "arch" ] = CORE::CVariant( report.arch );
        row[ "agent_version" ] = CORE::CVariant( report.agentVersion );
        row[ "verified" ] = CORE::CVariant( entry.verified );
        row[ "address" ] = CORE::CVariant( entry.address );
        row[ "first_seen" ] = CORE::CVariant( Timestamp( entry.firstSeenInUnixSecs ) );
        row[ "last_seen" ] = CORE::CVariant( Timestamp( entry.lastSeenInUnixSecs ) );
        row[ "liveness" ] = CORE::CVariant( CORE::CString( LivenessText( entry.liveness ) ) );
        row[ "node_state" ] = CORE::CVariant( report.nodeState );
        row[ "presence" ] = CORE::CVariant( report.presence );
        row[ "instance_id" ] = CORE::CVariant( report.instanceId );
        row[ "started_at" ] = CORE::CVariant( Timestamp( report.startedInUnixSecs ) );
        row[ "boot_id" ] = CORE::CVariant( report.bootId );
        row[ "uptime_secs" ] = CORE::CVariant( Timestamp( report.uptimeInSecs ) );
        row[ "ephemeral" ] = CORE::CVariant( report.ephemeral );
        row[ "idle_secs" ] = CORE::CVariant( (Int64) report.idleInSecs );
        row[ "cpu_cores" ] = CORE::CVariant( (Int64) report.cpuCores );
        row[ "cpu_load" ] = CORE::CVariant( (Int64) report.cpuLoadPercent );
        row[ "mem_total_mib" ] = CORE::CVariant( (Int64) report.memTotalInMiB );
        row[ "mem_available_mib" ] = CORE::CVariant( (Int64) report.memAvailableInMiB );
        row[ "disk_free_mib" ] = CORE::CVariant( (Int64) report.diskFreeInMiB );
        row[ "details" ] = CORE::CVariant( EncodeReportDetails( report ) );
        rows.push_back( row );
    }
    return WriteRows( m_nodes, rows, problem );
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::LoadNodes( vector< SNodeEntry >& nodes, CORE::CString& problem )
{GUCEF_TRACE;

    nodes.clear();
    MT::CScopeMutex lock( m_lock );
    if ( m_nodes.IsNULL() )
    {
        problem = "the database is not open";
        return false;
    }
    TRowVector rows;
    if ( !ReadRows( m_nodes, CORE::CDataTableQuery(), rows, problem ) )
        return false;
    for ( size_t i=0; i<rows.size(); ++i )
    {
        TRow& row = rows[ i ];
        SNodeEntry entry;
        SNodeReport& report = entry.report;
        report.machineId = row[ "machine_id" ].AsString();
        if ( report.machineId.IsNULLOrEmpty() )
            continue;
        report.name = row[ "name" ].AsString();
        report.os = row[ "os" ].AsString();
        report.arch = row[ "arch" ].AsString();
        report.agentVersion = row[ "agent_version" ].AsString();
        entry.verified = row[ "verified" ].AsBool();
        entry.address = row[ "address" ].AsString();
        entry.firstSeenInUnixSecs = (UInt64) row[ "first_seen" ].AsInt64();
        entry.lastSeenInUnixSecs = (UInt64) row[ "last_seen" ].AsInt64();
        const CORE::CString liveness = row[ "liveness" ].AsString();
        entry.liveness = liveness == "online" ? NODELIVENESS_ONLINE : ( liveness == "offline" ? NODELIVENESS_OFFLINE : NODELIVENESS_UNKNOWN );
        report.nodeState = row[ "node_state" ].AsString();
        report.presence = row[ "presence" ].AsString();
        report.instanceId = row[ "instance_id" ].AsString();
        report.startedInUnixSecs = (UInt64) row[ "started_at" ].AsInt64();
        report.bootId = row[ "boot_id" ].AsString();
        report.uptimeInSecs = (UInt64) row[ "uptime_secs" ].AsInt64();
        report.ephemeral = row[ "ephemeral" ].AsBool();
        report.idleInSecs = (UInt32) row[ "idle_secs" ].AsInt64();
        report.cpuCores = (UInt32) row[ "cpu_cores" ].AsInt64();
        report.cpuLoadPercent = (UInt32) row[ "cpu_load" ].AsInt64();
        report.memTotalInMiB = (UInt64) row[ "mem_total_mib" ].AsInt64();
        report.memAvailableInMiB = (UInt64) row[ "mem_available_mib" ].AsInt64();
        report.diskFreeInMiB = (UInt64) row[ "disk_free_mib" ].AsInt64();
        DecodeReportDetails( row[ "details" ].AsString(), report );
        entry.lastPersistedInUnixSecs = entry.lastSeenInUnixSecs;
        nodes.push_back( entry );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::DeleteNodes( const CORE::CString::StringVector& machineIds, CORE::CString& problem )
{GUCEF_TRACE;

    if ( machineIds.empty() )
        return true;
    MT::CScopeMutex lock( m_lock );
    if ( m_nodes.IsNULL() )
    {
        problem = "the database is not open";
        return false;
    }
    CORE::CVariant::VariantVector values;
    for ( size_t i=0; i<machineIds.size(); ++i )
        values.push_back( CORE::CVariant( machineIds[ i ] ) );
    CORE::CDataTableQuery query;
    query.WhereIn( "machine_id", values );
    return DeleteWhere( m_nodes, query, problem );
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::AppendEvents( const vector< SNodeEvent >& events, CORE::CString& problem )
{GUCEF_TRACE;

    if ( events.empty() )
        return true;
    MT::CScopeMutex lock( m_lock );
    if ( m_events.IsNULL() )
    {
        problem = "the database is not open";
        return false;
    }
    TRowVector rows;
    for ( size_t i=0; i<events.size(); ++i )
    {
        // time first, so the keys sort by time; the counter keeps two events of one second apart
        TRow row;
        row[ "event_id" ] = CORE::CVariant( PaddedSeconds( events[ i ].unixSecs ) + "-" + events[ i ].machineId + "-" + CORE::ToString( ++m_eventCounter ) );
        row[ "ts" ] = CORE::CVariant( Timestamp( events[ i ].unixSecs ) );
        row[ "machine_id" ] = CORE::CVariant( events[ i ].machineId );
        row[ "kind" ] = CORE::CVariant( events[ i ].kind );
        row[ "detail" ] = CORE::CVariant( events[ i ].detail );
        rows.push_back( row );
    }
    return WriteRows( m_events, rows, problem );
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::LoadEvents( const CORE::CString& machineId, vector< SNodeEvent >& events, CORE::CString& problem )
{GUCEF_TRACE;

    events.clear();
    MT::CScopeMutex lock( m_lock );
    if ( m_events.IsNULL() )
    {
        problem = "the database is not open";
        return false;
    }
    CORE::CDataTableQuery query;
    query.WhereEquals( "machine_id", CORE::CVariant( machineId ) );
    TRowVector rows;
    if ( !ReadRows( m_events, query, rows, problem ) )
        return false;

    // oldest first; the key starts with the time
    GUCEF::map< CORE::CString, SNodeEvent > sorted;
    for ( size_t i=0; i<rows.size(); ++i )
    {
        SNodeEvent event;
        event.unixSecs = (UInt64) rows[ i ][ "ts" ].AsInt64();
        event.machineId = rows[ i ][ "machine_id" ].AsString();
        event.kind = rows[ i ][ "kind" ].AsString();
        event.detail = rows[ i ][ "detail" ].AsString();
        sorted[ rows[ i ][ "event_id" ].AsString() ] = event;
    }
    for ( GUCEF::map< CORE::CString, SNodeEvent >::const_iterator i = sorted.begin(); i != sorted.end(); ++i )
        events.push_back( (*i).second );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHubStore::PurgeEvents( UInt64 unixNowInSecs, UInt32 retentionInDays, UInt32 maxEvents, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_events.IsNULL() )
    {
        problem = "the database is not open";
        return false;
    }

    // by age
    if ( retentionInDays > 0 && unixNowInSecs > (UInt64) retentionInDays * 86400 )
    {
        CORE::CDataTableQuery old;
        old.WhereRange( "ts", CORE::CVariant( (Int64) 0 ), true, CORE::CVariant( Timestamp( unixNowInSecs - (UInt64) retentionInDays * 86400 ) ), false );
        if ( !DeleteWhere( m_events, old, problem ) )
            return false;
    }

    // by count: the times of what is left, the oldest above the limit go
    if ( maxEvents > 0 )
    {
        CORE::CDataTableQuery all;
        all.Select( "ts" );
        TRowVector rows;
        if ( !ReadRows( m_events, all, rows, problem ) )
            return false;
        if ( rows.size() > maxEvents )
        {
            GUCEF::vector< Int64 > times;
            for ( size_t i=0; i<rows.size(); ++i )
                times.push_back( rows[ i ][ "ts" ].AsInt64() );
            std::sort( times.begin(), times.end() );
            const Int64 cutoff = times[ rows.size() - maxEvents ];
            CORE::CDataTableQuery excess;
            excess.WhereRange( "ts", CORE::CVariant( (Int64) 0 ), true, CORE::CVariant( cutoff ), false );
            if ( !DeleteWhere( m_events, excess, problem ) )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
