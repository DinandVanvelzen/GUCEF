/*
 *  dbpluginVFSTABLES_TestApp: tests of the vfstables database backend
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

#include <algorithm>

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

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

#ifndef TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#include "tablecodecpluginPARQUET_CParquetTableCodec.h"
#define TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H
#endif /* TABLECODECPLUGIN_PARQUET_CPARQUETTABLECODEC_H ? */

#ifndef DBPLUGIN_VFSTABLES_H
#include "dbpluginVFSTABLES.h"
#define DBPLUGIN_VFSTABLES_H
#endif /* DBPLUGIN_VFSTABLES_H ? */

#include "TestDataTableScanSupport.h"
#include "TestVfsTablesSupport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace VFSTABLESTEST {

using namespace GUCEF;
using namespace GUCEF::CORE;

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
    if ( pluginControl.AddPluginFromDir( first, "VfsTablesTestPlugins", true ) )
        return true;
    return pluginControl.AddPluginFromDir( second, "VfsTablesTestPlugins", true );
}

void
CollectFiles( const CString& dir, const CString& relativeDir, CString::StringVector& files )
{
    CFileSystemIterator iterator;
    if ( !iterator.FindFirst( dir ) )
        return;
    do
    {
        const CString name = iterator.GetResourceName();
        if ( name == "." || name == ".." )
            continue;
        const CString relative = relativeDir.IsNULLOrEmpty() ? name : relativeDir + "/" + name;
        if ( iterator.IsADirectory() && !iterator.IsSymlink() )
            CollectFiles( CombinePath( dir, name ), relative, files );
        else
            files.push_back( relative );
    }
    while ( iterator.FindNext() );
}

}

/*-------------------------------------------------------------------------*/

void
SetUpTestEnvironment( void )
{
    AddPlugin( "codecspluginZLIB" );
    AddPlugin( "dstorepluginJSONPARSER" );

    // the parquet plugin is linked: its codec goes into the registry as the plugin module would put it there
    CTableCodecRegistry& registry = CCoreGlobal::Instance()->GetTableCodecRegistry();
    if ( !registry.IsRegistered( "parquet" ) )
    {
        CTableCodecRegistry::TTableCodecPtr codec( CTSharedPtr< CITableCodec, MT::CMutex >( GUCEF_NEW TABLECODECPLUGIN::PARQUET::CParquetTableCodec() ) );
        registry.TryRegister( "parquet", codec );
    }

    // the backend module is linked as well: its plugin entry point registers it
    if ( !COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().IsConstructible( "vfstables" ) )
        GUCEFPlugin_Load( 0, GUCEF_NULL );

    VFS::CVfsGlobal::Instance()->GetVfs().MountAllDelayMountedArchives();
}

/*-------------------------------------------------------------------------*/

CString
MountFreshDirectory( const CString& name, const CString& mountPoint, bool writeable )
{
    const CString dir = CombinePath( CombinePath( TempDir(), "dbpluginVFSTABLES_TestApp" ), name );
    if ( DirExists( dir ) )
        DeleteDir( dir, true );
    CreateDirs( dir );
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.UnmountArchiveByName( "VfsTablesTest_" + name );
    vfs.AddRoot( dir, "VfsTablesTest_" + name, writeable, false, mountPoint );
    return dir;
}

/*-------------------------------------------------------------------------*/

void
UnmountDirectory( const CString& name )
{
    VFS::CVfsGlobal::Instance()->GetVfs().UnmountArchiveByName( "VfsTablesTest_" + name );
}

/*-------------------------------------------------------------------------*/

COMCORE::CDatabaseClientSettings
MakeSettings( const CString& database, bool readOnly )
{
    COMCORE::CDatabaseClientSettings settings;
    settings.backendName = "vfstables";
    settings.database = database;
    settings.readOnly = readOnly;
    settings.backendOptions.SetAllowMultipleValues( false );
    // the tests make many files: durability is checked once, separately
    settings.backendOptions.Set( "syncWrites", CVariant( "false" ) );
    return settings;
}

/*-------------------------------------------------------------------------*/

COMCORE::CIDatabaseClientPtr
OpenDatabase( const COMCORE::CDatabaseClientSettings& settings )
{
    COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( settings.backendName );
    if ( client.IsNULL() || !client->Open( settings ) )
        return COMCORE::CIDatabaseClientPtr();
    return client;
}

/*-------------------------------------------------------------------------*/

CDataTableBatchPtr
MakeBatch( const CDataTableSchemaPtr& schema, const TRowVector& rows )
{
    CDataTableBatchBuilder builder( schema, CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( UInt32 r=0; r<(UInt32) rows.size(); ++r )
    {
        if ( !builder.AppendRow( &rows[ r ][ 0 ], (UInt32) rows[ r ].size() ) )
            return CDataTableBatchPtr();
    }
    CDataTableBatchPtr batch;
    CString error;
    if ( DATATABLE_STATUS_OK != builder.Build( batch, error ) )
        return CDataTableBatchPtr();
    return batch;
}

/*-------------------------------------------------------------------------*/

TDataTableStatus
ScanRows( CIDataTable& table, const CDataTableQuery& query, CString::StringVector& rows, CDataTableScanStats* stats, bool async )
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
    if ( GUCEF_NULL != stats )
        scan->GetStats( *stats );
    std::sort( rows.begin(), rows.end() );
    return DATATABLE_STATUS_END == status ? DATATABLE_STATUS_OK : status;
}

/*-------------------------------------------------------------------------*/

void
BruteForceRows( const CDataTableSchemaPtr& schema, const TRowVector& rows, const CDataTableQuery& query, CString::StringVector& expected )
{
    expected.clear();
    TRowVector matching;
    const CDataTableQuery::TPredicateVector& predicates = query.GetPredicates();
    for ( UInt32 r=0; r<(UInt32) rows.size(); ++r )
    {
        bool matches = true;
        for ( UInt32 p=0; p<(UInt32) predicates.size() && matches; ++p )
        {
            const Int32 column = schema->FindColumn( predicates[ p ].columnName );
            matches = column >= 0 && CORETEST::BruteForceMatches( schema->GetColumn( (UInt32) column ), rows[ r ][ column ], predicates[ p ] );
        }
        if ( matches )
            matching.push_back( rows[ r ] );
    }
    if ( matching.empty() )
        return;

    // the projection of the query, as schema columns
    GUCEF::vector< UInt32 > projection;
    const CDataTableQuery::TSelectedColumnVector& selection = query.GetSelection();
    for ( UInt32 s=0; s<(UInt32) selection.size(); ++s )
    {
        if ( selection[ s ].columnName.IsNULLOrEmpty() )
            projection.push_back( selection[ s ].columnIndex );
        else
            projection.push_back( (UInt32) schema->FindColumn( selection[ s ].columnName ) );
    }
    if ( projection.empty() )
    {
        for ( UInt32 c=0; c<schema->GetColumnCount(); ++c )
            projection.push_back( c );
    }

    CDataTableBatchPtr batch = MakeBatch( schema, matching );
    if ( batch.IsNULL() )
        return;
    for ( UInt32 r=0; r<batch->GetRowCount(); ++r )
    {
        CString text;
        for ( UInt32 c=0; c<(UInt32) projection.size(); ++c )
            text += CORETEST::CellText( *batch, r, projection[ c ] ) + "|";
        expected.push_back( text );
    }
    std::sort( expected.begin(), expected.end() );
}

/*-------------------------------------------------------------------------*/

UInt32
CountFilesBelow( const CString& nativeDir )
{
    CString::StringVector files;
    CollectFiles( nativeDir, CString::Empty, files );
    return (UInt32) files.size();
}

/*-------------------------------------------------------------------------*/

CVariant
Day( Int32 year, UInt32 month, UInt32 day )
{
    Int32 days = 0;
    CDataTableValues::DaysFromCivil( year, month, day, days );
    return CDataTableValues::Date( days );
}

/*-------------------------------------------------------------------------*/

bool
WriteNativeFile( const CString& path, const void* data, UInt32 size )
{
    CreateDirs( StripFilename( path ) );
    CFileAccess file( path, "wb" );
    if ( !file.IsValid() )
        return false;
    const bool written = 0 == size || size == file.Write( data, 1, size );
    file.Close();
    return written;
}

}; /* namespace VFSTABLESTEST */

/*-------------------------------------------------------------------------*/
