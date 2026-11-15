/*
 *  dbpluginPOSTGRESQL_TestApp: tests of the postgresql database backend
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

#include <stdlib.h>
#include <algorithm>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CDATATABLESCAN_H
#include "gucefCORE_CDataTableScan.h"
#define GUCEF_CORE_CDATATABLESCAN_H
#endif /* GUCEF_CORE_CDATATABLESCAN_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_H
#include "dbpluginPOSTGRESQL.h"
#define DBPLUGIN_POSTGRESQL_H
#endif /* DBPLUGIN_POSTGRESQL_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#include "dbpluginPOSTGRESQL_CPostgreSqlClient.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLCLIENT_H ? */

#ifndef DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#include "dbpluginPOSTGRESQL_CPostgreSqlSettings.h"
#define DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H
#endif /* DBPLUGIN_POSTGRESQL_CPOSTGRESQLSETTINGS_H ? */

#include "TestDataTableSupport.h"
#include "TestDataTableScanSupport.h"
#include "TestPostgreSqlSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace GUCEF::CORE;

namespace POSTGRESQLTEST {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
AddPlugin( const CString& name )
{
    #ifdef GUCEF_DEBUG_MODE
    const CString first = "$MODULEDIR$/" + name + "_d";
    const CString second = "$MODULEDIR$/" + name;
    #else
    const CString first = "$MODULEDIR$/" + name;
    const CString second = "$MODULEDIR$/" + name + "_d";
    #endif
    CPluginControl& pluginControl = CCoreGlobal::Instance()->GetPluginControl();
    if ( pluginControl.AddPluginFromDir( first, "PostgreSqlTestPlugins", true ) )
        return true;
    return pluginControl.AddPluginFromDir( second, "PostgreSqlTestPlugins", true );
}

}

/*-------------------------------------------------------------------------*/

void
SetUpTestEnvironment( void )
{
    AddPlugin( "dstorepluginJSONPARSER" );

    // the backend module is linked: its plugin entry point registers it
    if ( !COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().IsConstructible( "postgresql" ) )
        GUCEFPlugin_Load( 0, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
GetTestServerConnectionString( CString& connectionString )
{
    const char* value = getenv( "GUCEF_TEST_POSTGRESQL" );
    if ( GUCEF_NULL == value || '\0' == value[ 0 ] )
        return false;
    connectionString = value;
    return true;
}

/*-------------------------------------------------------------------------*/

CString
MakeTestSchemaName( void )
{
    CORETEST::CTestRandom random( (UInt64) MT::PrecisionTickCount() ^ ( (UInt64) MT::GetProcessID() << 32 ) );
    return "gucef_test_" + ToString( (UInt32) MT::GetProcessID() ) + "_" + ToString( random.Below( 1000000 ) );
}

/*-------------------------------------------------------------------------*/

COMCORE::CDatabaseClientSettings
MakeServerSettings( const CString& connectionString, const CString& schema, bool readOnly )
{
    COMCORE::CDatabaseClientSettings settings;
    settings.backendName = "postgresql";
    CValueList parsed;
    CString error;
    if ( DBPLUGIN::POSTGRESQL::CPostgreSqlSettings::ParseConnectionString( connectionString, parsed, error ) )
        settings.database = parsed.GetValueAlways( CString( "dbname" ) ).AsString();
    settings.readOnly = readOnly;
    settings.backendOptions.Set( "connectionString", CVariant( connectionString ) );
    settings.backendOptions.Set( "schema", CVariant( schema ) );
    settings.backendOptions.Set( "createSchema", CVariant( "true" ) );
    settings.backendOptions.Set( "applicationName", CVariant( "dbpluginPOSTGRESQL_TestApp" ) );
    settings.backendOptions.Set( "connectTimeout", CVariant( "10" ) );
    return settings;
}

/*-------------------------------------------------------------------------*/

CDataTableDefinition
MakeAllTypesDefinition( const CString& name )
{
    CDataTableDefinition definition;
    definition.name = name;
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    const CDataTableSchemaPtr allTypes = CORETEST::MakeAllTypesSchema( true );
    for ( UInt32 i=0; i<allTypes->GetColumnCount(); ++i )
        definition.schema.AddColumn( allTypes->GetColumn( i ) );
    CString::StringVector key;
    key.push_back( "id" );
    definition.schema.SetPrimaryKey( key );
    return definition;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
ScanRows( CIDataTable& table, const CDataTableQuery& query, CString::StringVector& rows, bool async )
{
    rows.clear();
    CDataTableScanPtr scan;
    TDataTableStatus status = DATATABLE_STATUS_OK;
    if ( async )
    {
        CFutureResult future = table.ScanAsync( query, scan );
        if ( scan.IsNULL() )
            return DATATABLE_STATUS_FAILED;
    }
    else
    {
        status = table.Scan( query, scan );
        if ( DATATABLE_STATUS_OK != status )
            return status;
    }
    status = CORETEST::CollectScanRows( *scan, rows );
    std::sort( rows.begin(), rows.end() );
    return DATATABLE_STATUS_END == status ? DATATABLE_STATUS_OK : status;
}

/*-------------------------------------------------------------------------*/

bool
WaitForFuture( const CFutureResult& future, UInt32 timeoutInMs )
{
    CTaskPtr task = future.GetResult( timeoutInMs );
    return !task.IsNULL() && task->IsTaskInEndState();
}

/*-------------------------------------------------------------------------*/

TTaskStatus
StatusOfFuture( const CFutureResult& future )
{
    CTaskPtr task = future.GetResult();
    return task.IsNULL() ? TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace POSTGRESQLTEST */

/*-------------------------------------------------------------------------*/
