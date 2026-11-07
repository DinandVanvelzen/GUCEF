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

#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_CFILEDATATABLE_H
#include "gucefCORE_CFileDataTable.h"
#define GUCEF_CORE_CFILEDATATABLE_H
#endif /* GUCEF_CORE_CFILEDATATABLE_H ? */

#ifndef GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#include "gucefCORE_CDataTablePartitionSpec.h"
#define GUCEF_CORE_CDATATABLEPARTITIONSPEC_H
#endif /* GUCEF_CORE_CDATATABLEPARTITIONSPEC_H ? */

#ifndef GUCEF_CORE_CITABLEWRITER_H
#include "gucefCORE_CITableWriter.h"
#define GUCEF_CORE_CITABLEWRITER_H
#endif /* GUCEF_CORE_CITABLEWRITER_H ? */

#ifndef GUCEF_COMCORE_CDATABASETABLERESULT_H
#include "gucefCOMCORE_CDatabaseTableResult.h"
#define GUCEF_COMCORE_CDATABASETABLERESULT_H
#endif /* GUCEF_COMCORE_CDATABASETABLERESULT_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef DBPLUGIN_VFSTABLES_H
#include "dbpluginVFSTABLES.h"
#define DBPLUGIN_VFSTABLES_H
#endif /* DBPLUGIN_VFSTABLES_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#include "dbpluginVFSTABLES_CVfsTablesDatabaseClient.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESDATABASECLIENT_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#include "dbpluginVFSTABLES_CVfsDataTable.h"
#define DBPLUGIN_VFSTABLES_CVFSDATATABLE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSDATATABLE_H ? */

#ifndef DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H
#include "dbpluginVFSTABLES_CVfsTableScanSource.h"
#define DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H
#endif /* DBPLUGIN_VFSTABLES_CVFSTABLESCANSOURCE_H ? */

#if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
  #include <sys/stat.h>
  #include <unistd.h>
#endif

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDataTableScanSupport.h"
#include "TestVfsTablesSupport.h"
#include "TestVfsTablesDatabase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

/** A failed requirement ends the test case: what follows would only fail on its consequences */
struct SRequirementFailed {};
#define REQUIRE_TRUE(t) { const bool requirementMet = ( t ); GUCEF_TESTFW_ASSERT_TRUE( requirementMet ); if ( !requirementMet ) throw SRequirementFailed(); }

using namespace GUCEF;
using namespace GUCEF::CORE;
using namespace VFSTABLESTEST;

namespace {

typedef DBPLUGIN::VFSTABLES::CVfsTablesDatabaseClient   TClient;
typedef DBPLUGIN::VFSTABLES::CVfsDataTable              TVfsTable;
typedef DBPLUGIN::VFSTABLES::CVfsTablesStats            TStorageStats;

/**
 *  Events: region (utf8), day (date), id (int64), value (float64, nullable), note (utf8, nullable).
 *  Primary key region, day, id; sorted on day, id; partitioned as {region}/{year}.<extension>
 */
CDataTableDefinition
MakeEventsDefinition( const CString& name, const CString& codecType = "parquet", UInt32 rowGroupRowCount = 1048576 )
{
    CDataTableDefinition definition;
    definition.name = name;
    definition.codecType = codecType;
    definition.schema.AddColumn( CDataTableColumnDef( "region", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "day", DATATABLE_TYPE_DATE, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "value", DATATABLE_TYPE_FLOAT64, true ) );
    definition.schema.AddColumn( CDataTableColumnDef( "note", DATATABLE_TYPE_UTF8, true ) );
    CString::StringVector key;
    key.push_back( "region" );
    key.push_back( "day" );
    key.push_back( "id" );
    definition.schema.SetPrimaryKey( key );
    CString::StringVector sortKey;
    sortKey.push_back( "day" );
    sortKey.push_back( "id" );
    definition.schema.SetSortKey( sortKey );
    definition.partitioning.AddField( "region", "region", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.AddField( "year", "day", DATATABLE_PARTITION_TRANSFORM_YEAR );
    definition.partitioning.SetPathTemplate( CString( "{region}/{year}." ) + ( "csv" == codecType ? "csv" : "parquet" ) );
    definition.storage.rowGroupRowCount = rowGroupRowCount;
    if ( definition.storage.pageRowCount > rowGroupRowCount )
        definition.storage.pageRowCount = rowGroupRowCount;
    return definition;
}

/** One file per symbol: "{symbol}.parquet" with an id and a price */
CDataTableDefinition
MakePricesDefinition( void )
{
    CDataTableDefinition definition;
    definition.name = "prices";
    definition.codecType = "parquet";
    definition.schema.AddColumn( CDataTableColumnDef( "symbol", DATATABLE_TYPE_UTF8, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
    definition.schema.AddColumn( CDataTableColumnDef( "price", DATATABLE_TYPE_INT64, false ) );
    definition.partitioning.AddField( "symbol", "symbol", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
    definition.partitioning.SetPathTemplate( "{symbol}.parquet" );
    return definition;
}

/** 100 rows of KO with ids 0 .. 99 and prices first .. first + 99: files of the same size for every first price below 10^6 */
TRowVector
MakePrices( Int64 firstPrice )
{
    TRowVector rows;
    for ( Int64 i=0; i<100; ++i )
    {
        TRow row;
        row.push_back( CVariant( CUtf8String( "KO" ) ) );
        row.push_back( CVariant( i ) );
        row.push_back( CVariant( firstPrice + i ) );
        rows.push_back( row );
    }
    return rows;
}

TRow
EventRow( const char* region, Int32 year, UInt32 month, UInt32 day, Int64 id, Float64 value, const char* note )
{
    TRow row;
    row.push_back( CVariant( CUtf8String( region ) ) );
    row.push_back( Day( year, month, day ) );
    row.push_back( CVariant( id ) );
    row.push_back( value != value ? CVariant() : CVariant( value ) );
    row.push_back( GUCEF_NULL == note ? CVariant() : CVariant( CUtf8String( note ) ) );
    return row;
}

/** Events of the regions over the years: one per region, year and id */
TRowVector
MakeEvents( const char* const* regions, UInt32 regionCount, Int32 firstYear, Int32 lastYear, UInt32 idsPerYear, Float64 value )
{
    TRowVector rows;
    for ( UInt32 r=0; r<regionCount; ++r )
        for ( Int32 y=firstYear; y<=lastYear; ++y )
            for ( UInt32 i=0; i<idsPerYear; ++i )
                rows.push_back( EventRow( regions[ r ], y, 1 + i % 12, 1 + i % 28, (Int64) i, value, i % 3 == 0 ? GUCEF_NULL : "n" ) );
    return rows;
}

CIDataTablePtr
CreateTable( COMCORE::CIDatabaseClientPtr& client, const CDataTableDefinition& definition )
{
    CIDataTablePtr table;
    REQUIRE_TRUE( !client.IsNULL() && DATATABLE_STATUS_OK == client->CreateTable( definition, table ) );
    return table;
}

COMCORE::CIDatabaseClientPtr
OpenRequired( const COMCORE::CDatabaseClientSettings& settings )
{
    COMCORE::CIDatabaseClientPtr client = OpenDatabase( settings );
    REQUIRE_TRUE( !client.IsNULL() );
    return client;
}

TDataTableStatus
WriteRows( CIDataTable& table, const TRowVector& rows, TDataTableWriteMode mode, CDataTableWriteResult* result = GUCEF_NULL )
{
    CDataTableWriteResult localResult;
    CDataTableBatchPtr batch = MakeBatch( table.GetSchema(), rows );
    if ( batch.IsNULL() )
        return DATATABLE_STATUS_INVALID_ARGUMENT;
    return table.Write( *batch, mode, GUCEF_NULL != result ? *result : localResult );
}

UInt32
CountRows( CIDataTable& table, const CDataTableQuery& query = CDataTableQuery() )
{
    CString::StringVector rows;
    if ( DATATABLE_STATUS_OK != ScanRows( table, query, rows ) )
        return GUCEF_UINT32MAX;
    return (UInt32) rows.size();
}

TStorageStats
StorageStats( CIDataTablePtr& table )
{
    TStorageStats stats;
    static_cast< TVfsTable* >( table.GetPointerAlways() )->GetStorageStats( stats );
    return stats;
}

void
ResetStorage( CIDataTablePtr& table )
{
    TVfsTable* vfsTable = static_cast< TVfsTable* >( table.GetPointerAlways() );
    vfsTable->Refresh();
    vfsTable->GetStore()->ResetStats();
}

/** Names of the temporary files below a native directory */
UInt32
CountTemporaryFiles( const CString& dir )
{
    UInt32 count = 0;
    CFileSystemIterator iterator;
    if ( !iterator.FindFirst( dir ) )
        return 0;
    do
    {
        const CString name = iterator.GetResourceName();
        if ( name == "." || name == ".." )
            continue;
        if ( iterator.IsADirectory() )
            count += CountTemporaryFiles( CombinePath( dir, name ) );
        else
        if ( name.HasSubstr( ".tmp-" ) > 0 )
            ++count;
    }
    while ( iterator.FindNext() );
    return count;
}

TTaskStatus
AwaitFuture( const CFutureResult& future, UInt32 timeoutMs, CTaskPtr& task )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( !future.IsReady() )
    {
        if ( MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) > timeoutMs )
            return TTaskStatus::TASKSTATUS_UNDEFINED;
        MT::PrecisionDelay( 2 );
    }
    task = future.GetResult( 0 );
    return task.IsNULL() ? TTaskStatus::TASKSTATUS_UNDEFINED : task->GetTaskStatus();
}

/*-------------------------------------------------------------------------*/

/** Whether the writers of CFaultyTableCodec fail at Finish(), after the batches went to the file */
MT::CTAtomic< UInt32 > g_failWrites( 0 );

/** A native directory the writers of CFaultyTableCodec make read only after Finish(), when set */
CString g_readOnlyDirAfterFinish;

/** A writer that writes through another one and fails at the end, as a write cut short would */
class CFaultyTableWriter : public CITableWriter
{
    public:

    explicit CFaultyTableWriter( const CITableWriterPtr& inner ) : CITableWriter(), m_inner( inner ) {}
    virtual TDataTableStatus WriteBatch( const CDataTableBatch& batch ) GUCEF_VIRTUAL_OVERRIDE { return m_inner->WriteBatch( batch ); }
    virtual TDataTableStatus FlushRowGroup( void ) GUCEF_VIRTUAL_OVERRIDE { return m_inner->FlushRowGroup(); }
    virtual TDataTableStatus Finish( CDataTableFileSummary& summary ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 != g_failWrites.Load() )
            return DATATABLE_STATUS_IO_ERROR;
        const TDataTableStatus status = m_inner->Finish( summary );
        #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
        // the directory of the partition stops taking new names: the rename that follows is refused, the file stays writable
        if ( !g_readOnlyDirAfterFinish.IsNULLOrEmpty() )
            ::chmod( g_readOnlyDirAfterFinish.C_String(), 0555 );
        #endif
        return status;
    }
    virtual void Abort( void ) GUCEF_VIRTUAL_OVERRIDE { m_inner->Abort(); }
    virtual CString GetLastErrorMessage( void ) const GUCEF_VIRTUAL_OVERRIDE { return 0 != g_failWrites.Load() ? CString( "injected failure" ) : m_inner->GetLastErrorMessage(); }

    private:

    CITableWriterPtr m_inner;
};

/** The parquet codec under another name, whose writers fail on request: crash injection for the copy-on-write path */
class CFaultyTableCodec : public CITableCodec
{
    public:

    explicit CFaultyTableCodec( const CTableCodecRegistry::TTableCodecPtr& inner ) : CITableCodec(), m_inner( inner ) {}
    virtual CString GetTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "faultyparquet"; }
    virtual CString GetName( void ) const GUCEF_VIRTUAL_OVERRIDE { return "faulty parquet"; }
    virtual CString GetCopyright( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetCopyright(); }
    virtual TVersion GetVersion( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetVersion(); }
    virtual UInt32 GetCapabilities( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_inner->GetCapabilities(); }
    virtual void GetFileExtensions( CString::StringSet& extensions ) const GUCEF_VIRTUAL_OVERRIDE { m_inner->GetFileExtensions( extensions ); }
    virtual TDataTableStatus ReadMetaData( CIOAccess& source, const CValueList& options, CITableFileMetaDataPtr& metaData, CString& error ) const GUCEF_VIRTUAL_OVERRIDE
        { return m_inner->ReadMetaData( source, options, metaData, error ); }
    virtual TDataTableStatus CreateReader( const IOAccessPtr& source, const CITableFileMetaDataPtr& metaData, const CValueList& options,
                                           CITableReaderPtr& reader, CString& error ) const GUCEF_VIRTUAL_OVERRIDE
        { return m_inner->CreateReader( source, metaData, options, reader, error ); }
    virtual TDataTableStatus CreateWriter( const IOAccessPtr& destination, const CDataTableSchemaPtr& schema, const CDataTableWriterOptions& options,
                                           CITableWriterPtr& writer, CString& error ) const GUCEF_VIRTUAL_OVERRIDE
    {
        CITableWriterPtr inner;
        const TDataTableStatus status = m_inner->CreateWriter( destination, schema, options, inner, error );
        if ( DATATABLE_STATUS_OK != status )
            return status;
        writer = CITableWriterPtr( GUCEF_NEW CFaultyTableWriter( inner ) );
        return DATATABLE_STATUS_OK;
    }
    virtual CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NEW CFaultyTableCodec( m_inner ); }

    private:

    CTableCodecRegistry::TTableCodecPtr m_inner;
};

/*-------------------------------------------------------------------------*/

/** Replaces the partitions of the table over and over: each partition holds version v as v % 40 + 1 rows of value v */
class CPartitionWriterThread : public MT::CActiveObject
{
    public:

    CPartitionWriterThread( const CIDataTablePtr& table, UInt32 rounds )
        : MT::CActiveObject(), m_table( table ), m_rounds( rounds ), failures( 0 ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        static const char* const regions[] = { "EU", "US", "APAC" };
        for ( UInt32 v=1; v<=m_rounds; ++v )
        {
            TRowVector rows;
            for ( UInt32 r=0; r<3; ++r )
                for ( UInt32 i=0; i<v % 40 + 1; ++i )
                    rows.push_back( EventRow( regions[ r ], 2020, 1, 1 + i % 28, (Int64) i, (Float64) v, GUCEF_NULL ) );
            if ( DATATABLE_STATUS_OK != WriteRows( *m_table, rows, DATATABLE_WRITE_REPLACE_PARTITIONS ) )
                failures.FetchAdd( 1 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CIDataTablePtr m_table;
    UInt32 m_rounds;
    MT::CTAtomic< UInt32 > failures;
    MT::CTAtomic< UInt32 > done;
};

/** Scans the table until the writer is done; every partition must be one whole version */
class CPartitionReaderThread : public MT::CActiveObject
{
    public:

    CPartitionReaderThread( const CIDataTablePtr& table, MT::CTAtomic< UInt32 >& writerDone, bool async )
        : MT::CActiveObject(), m_table( table ), m_writerDone( writerDone ), m_async( async ), torn( 0 ), scans( 0 ), failures( 0 ), done( 0 ) {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        while ( 0 == m_writerDone.Load() )
        {
            CDataTableQuery query;
            query.Select( "region" ).Select( "value" );
            CDataTableScanPtr scan;
            if ( m_async )
                m_table->ScanAsync( query, scan );
            else
            if ( DATATABLE_STATUS_OK != m_table->Scan( query, scan ) )
            {
                failures.FetchAdd( 1 );
                continue;
            }
            GUCEF::map< CString, GUCEF::map< Int64, UInt32 > > versions;
            TDataTableStatus status = DATATABLE_STATUS_OK;
            while ( DATATABLE_STATUS_OK == status )
            {
                CDataTableBatchPtr batch;
                status = scan->NextBatch( batch );
                if ( DATATABLE_STATUS_OK != status )
                    break;
                for ( UInt32 r=0; r<batch->GetRowCount(); ++r )
                {
                    CVariant region;
                    CVariant value;
                    batch->GetValue( r, 0, region, false );
                    batch->GetValue( r, 1, value, false );
                    ++versions[ region.AsString() ][ (Int64) value.AsFloat64() ];
                }
            }
            if ( DATATABLE_STATUS_END != status )
            {
                failures.FetchAdd( 1 );
                continue;
            }
            for ( GUCEF::map< CString, GUCEF::map< Int64, UInt32 > >::const_iterator i = versions.begin(); i != versions.end(); ++i )
            {
                // one version per partition, with exactly the rows of that version
                if ( 1 != (*i).second.size() || (*(*i).second.begin()).second != (UInt32) ( (*(*i).second.begin()).first % 40 + 1 ) )
                    torn.FetchAdd( 1 );
            }
            scans.FetchAdd( 1 );
        }
        done.Store( 1 );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    CIDataTablePtr m_table;
    MT::CTAtomic< UInt32 >& m_writerDone;
    bool m_async;
    MT::CTAtomic< UInt32 > torn;
    MT::CTAtomic< UInt32 > scans;
    MT::CTAtomic< UInt32 > failures;
    MT::CTAtomic< UInt32 > done;
};

void
WaitForDone( MT::CTAtomic< UInt32 >& done, UInt32 timeoutMs )
{
    const UInt64 start = MT::PrecisionTickCount();
    while ( 0 == done.Load() && MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() - start ) < timeoutMs )
        MT::PrecisionDelay( 5 );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVfsTablesDatabaseTests( void )
{
    GUCEF_TESTFW_SUITE_SCOPE( "VfsTablesDatabase" );

    COMCORE::CDatabaseClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory();
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

    GUCEF_TESTFW_TESTCASE( "Database: the plugin registers \"vfstables\" in the database client factory and takes it out at unload" )
        try
        {
            ASSERT_TRUE( factory.IsConstructible( "vfstables" ) );
            GUCEFPlugin_Unload();
            ASSERT_FALSE( factory.IsConstructible( "vfstables" ) );
            ASSERT_TRUE( 1 == GUCEFPlugin_Load( 0, GUCEF_NULL ) );
            ASSERT_TRUE( factory.IsConstructible( "vfstables" ) );
            ASSERT_TRUE( 0 == GUCEFPlugin_Load( 0, GUCEF_NULL ) );      // a second registration is refused
            COMCORE::CIDatabaseClientPtr client = factory.Create( "vfstables" );
            ASSERT_FALSE( client.IsNULL() );
            if ( !client.IsNULL() )
            {
                ASSERT_TRUE( "vfstables" == client->GetBackendName() );
                ASSERT_TRUE( 0 != ( client->GetCapabilities() & GUCEF_DATABASE_CAPABILITY_PARTITIONED_TABLES ) );
                ASSERT_FALSE( client->IsOpen() );
                CString::StringVector live;
                TClient::GetLiveObjects( live );
                ASSERT_TRUE( 1 == live.size() );
            }
            client.Unlink();
            CString::StringVector live;
            TClient::GetLiveObjects( live );
            ASSERT_TRUE( live.empty() );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: open refuses bad settings, a missing root, writing on a read only mount and a second writer of a root" )
        try
        {
            MountFreshDirectory( "open", "vfstests/open", true );
            MountFreshDirectory( "openro", "vfstests/openro", false );

            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/open", false );
            COMCORE::CIDatabaseClientPtr client = factory.Create( "vfstables" );
            settings.backendName = "other";
            ASSERT_FALSE( client->Open( settings ) );
            settings = MakeSettings( "vfstests/nothere", false );
            ASSERT_FALSE( client->Open( settings ) );
            ASSERT_TRUE( client->GetLastErrorMessage().HasSubstr( "vfstests/nothere" ) >= 0 );
            settings = MakeSettings( "vfstests/openro", false );
            ASSERT_FALSE( client->Open( settings ) );
            settings.readOnly = true;
            ASSERT_TRUE( client->Open( settings ) );
            client->Close();
            ASSERT_FALSE( client->IsOpen() );

            settings = MakeSettings( "vfstests/open", false );
            settings.backendOptions.Set( "footerCacheBytes", CVariant( "lots" ) );
            ASSERT_FALSE( client->Open( settings ) );
            settings = MakeSettings( "vfstests/open", false );
            settings.backendOptions.Set( "onIncompatibleFile", CVariant( "maybe" ) );
            ASSERT_FALSE( client->Open( settings ) );
            settings = MakeSettings( "vfstests/open", false );
            settings.backendOptions.Set( "writerId", CVariant( "" ) );
            ASSERT_FALSE( client->Open( settings ) );

            // one writer per root, readers beside it
            settings = MakeSettings( "vfstests/open/", false );
            ASSERT_TRUE( client->Open( settings ) );
            COMCORE::CIDatabaseClientPtr second = OpenDatabase( MakeSettings( "vfstests/open", false ) );
            ASSERT_TRUE( second.IsNULL() );
            COMCORE::CIDatabaseClientPtr reader = OpenDatabase( MakeSettings( "vfstests/open", true ) );
            ASSERT_FALSE( reader.IsNULL() );
            client->Close();
            second = OpenDatabase( MakeSettings( "vfstests/open", false ) );
            ASSERT_FALSE( second.IsNULL() );
            second.Unlink();
            reader.Unlink();
            client.Unlink();
            UnmountDirectory( "open" );
            UnmountDirectory( "openro" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: tables are created, listed, described, opened and dropped with or without their data" )
        try
        {
            const CString dir = MountFreshDirectory( "tables", "vfstests/tables", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/tables", false ) );
            ASSERT_FALSE( client.IsNULL() );

            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            ASSERT_FALSE( table.IsNULL() );
            CIDataTablePtr again;
            ASSERT_TRUE( DATATABLE_STATUS_ALREADY_EXISTS == client->CreateTable( MakeEventsDefinition( "events" ), again ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->CreateTable( MakeEventsDefinition( ".hidden" ), again ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->CreateTable( MakeEventsDefinition( "other", "nosuchcodec" ), again ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "_tables/events.json" ) ) );

            CString::StringSet names;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->ListTables( names ) );
            ASSERT_TRUE( 1 == names.size() && names.find( "events" ) != names.end() );
            CDataTableDefinition described;
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->GetTableDefinition( "events", described ) );
            ASSERT_TRUE( described.schema == MakeEventsDefinition( "events" ).schema && described.partitioning == MakeEventsDefinition( "events" ).partitioning );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->GetTableDefinition( "missing", described ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->OpenTable( "../events", again ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == client->OpenTable( "_tables/events", again ) );

            static const char* const regions[] = { "EU", "US" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 2, 2020, 2021, 10, 1.0 ), DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "EU/2020.parquet" ) ) && FileExists( CombinePath( dir, "US/2021.parquet" ) ) );

            // another client of the root, read only, opens the table and reads the rows
            COMCORE::CIDatabaseClientPtr reader = OpenRequired( MakeSettings( "vfstests/tables", true ) );
            CIDataTablePtr readTable;
            ASSERT_TRUE( DATATABLE_STATUS_OK == reader->OpenTable( "events", readTable ) );
            ASSERT_TRUE( 40 == CountRows( *readTable ) );
            ASSERT_TRUE( readTable->IsReadOnly() );
            readTable.Unlink();
            reader.Unlink();

            // dropped without its data the files stay, a table of the same definition sees them again
            table.Unlink();
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->DropTable( "events", false ) );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->OpenTable( "events", again ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "EU/2020.parquet" ) ) );
            table = CreateTable( client, MakeEventsDefinition( "events" ) );
            ASSERT_TRUE( 40 == CountRows( *table ) );
            table.Unlink();
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->DropTable( "events", true ) );
            ASSERT_FALSE( FileExists( CombinePath( dir, "EU/2020.parquet" ) ) || FileExists( CombinePath( dir, "US/2021.parquet" ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->ListTables( names ) && names.empty() );
            ASSERT_TRUE( DATATABLE_STATUS_NOT_FOUND == client->DropTable( "events", true ) );
            ASSERT_TRUE( 0 == CountTemporaryFiles( dir ) );
            client.Unlink();
            UnmountDirectory( "tables" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: insert conflicts leave a partition as it was, upsert replaces by key, replace partitions only touches the partitions written" )
        try
        {
            const CString dir = MountFreshDirectory( "modes", "vfstests/modes", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/modes", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            ASSERT_FALSE( table.IsNULL() );

            static const char* const regions[] = { "EU", "US" };
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 2, 2020, 2021, 5, 1.0 ), DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( 4 == result.partitions.size() && 20 == result.GetTotalRowsWritten() );

            // a key that is there fails its partition; the other partition of the write goes ahead
            TRowVector rows;
            rows.push_back( EventRow( "EU", 2020, 2, 2, 1, 9.0, "dup" ) );           // exists (id 1 is on 2020-02-02)
            rows.push_back( EventRow( "US", 2022, 1, 1, 100, 9.0, "new" ) );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == WriteRows( *table, rows, DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( 2 == result.partitions.size() );
            ASSERT_TRUE( DATATABLE_STATUS_CONFLICT == result.partitions[ 0 ].status && "EU/2020.parquet" == result.partitions[ 0 ].path );
            ASSERT_TRUE( DATATABLE_STATUS_OK == result.partitions[ 1 ].status && "US/2022.parquet" == result.partitions[ 1 ].path );
            ASSERT_TRUE( table->GetLastErrorMessage().HasSubstr( "EU/2020.parquet" ) >= 0 );
            ASSERT_TRUE( 21 == CountRows( *table ) );
            ASSERT_TRUE( 0 == CountRows( *table, CDataTableQuery().WhereEquals( "note", CVariant( "dup" ) ) ) );

            // upsert: the last row of a key wins, replaced rows are counted
            rows.clear();
            rows.push_back( EventRow( "EU", 2020, 2, 2, 1, 5.0, "first" ) );
            rows.push_back( EventRow( "EU", 2020, 2, 2, 1, 6.0, "last" ) );
            rows.push_back( EventRow( "EU", 2020, 7, 7, 77, 7.0, "added" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_UPSERT, &result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 1 == result.partitions[ 0 ].rowsReplaced && 6 == result.partitions[ 0 ].rowsWritten );
            CString::StringVector found;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery().Select( "note" ).Select( "value" ).WhereEquals( "id", CVariant( (Int64) 1 ) ).WhereEquals( "region", CVariant( "EU" ) ).WhereRange( "day", Day( 2020, 1, 1 ), true, Day( 2020, 12, 31 ), true ), found ) );
            ASSERT_TRUE( 1 == found.size() && "last|6|" == found[ 0 ] );

            // replace partitions: EU/2021 becomes the two rows given, the other partitions stay
            rows.clear();
            rows.push_back( EventRow( "EU", 2021, 3, 3, 500, 1.5, GUCEF_NULL ) );
            rows.push_back( EventRow( "EU", 2021, 4, 4, 501, 2.5, GUCEF_NULL ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_REPLACE_PARTITIONS, &result ) );
            ASSERT_TRUE( 2 == CountRows( *table, CDataTableQuery().WhereEquals( "region", CVariant( "EU" ) ).WhereRange( "day", Day( 2021, 1, 1 ), true, Day( 2021, 12, 31 ), true ) ) );
            ASSERT_TRUE( 6 == CountRows( *table, CDataTableQuery().WhereEquals( "region", CVariant( "EU" ) ).WhereRange( "day", Day( 2020, 1, 1 ), true, Day( 2020, 12, 31 ), true ) ) );
            ASSERT_TRUE( 11 == CountRows( *table, CDataTableQuery().WhereEquals( "region", CVariant( "US" ) ) ) );

            // a batch that does not fit the schema, upsert without a primary key
            TRowVector bad;
            TRow badRow = EventRow( "EU", 2020, 1, 1, 1, 1.0, GUCEF_NULL );
            bad.push_back( badRow );
            CDataTableSchema otherSchema;
            otherSchema.AddColumn( CDataTableColumnDef( "region", DATATABLE_TYPE_INT64, false ) );
            TRowVector otherRows;
            TRow otherRow;
            otherRow.push_back( CVariant( (Int64) 5 ) );
            otherRows.push_back( otherRow );
            CDataTableBatchPtr otherBatch = MakeBatch( CreateDataTableSchemaPtr( otherSchema ), otherRows );
            ASSERT_TRUE( DATATABLE_STATUS_OK != table->Write( *otherBatch, DATATABLE_WRITE_INSERT, result ) );
            ASSERT_TRUE( 0 == CountTemporaryFiles( dir ) );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "modes" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: the rows equal those of a CMemoryDataTable after the same random writes and deletes, for random queries" )
        try
        {
            MountFreshDirectory( "random", "vfstests/random", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/random", false ) );
            CDataTableDefinition definition = MakeEventsDefinition( "events", "parquet", 7 );
            CIDataTablePtr table = CreateTable( client, definition );
            ASSERT_FALSE( table.IsNULL() );
            CMemoryDataTable reference( definition );

            static const char* const regions[] = { "EU", "US", "APAC", "BRK.B", "eu" };
            CORETEST::CTestRandom random( 20261104 );
            UInt32 comparisons = 0;
            UInt32 mismatches = 0;
            for ( UInt32 round=0; round<30; ++round )
            {
                // a write: new keys by insert or existing and new keys by upsert
                TRowVector rows;
                const UInt32 count = 1 + random.Below( 40 );
                GUCEF::set< CString > keys;
                for ( UInt32 i=0; i<count; ++i )
                {
                    const char* region = regions[ random.Below( 5 ) ];
                    const Int32 year = 2019 + (Int32) random.Below( 4 );
                    const UInt32 month = 1 + random.Below( 12 );
                    const UInt32 day = 1 + random.Below( 28 );
                    const Int64 id = round * 1000 + (Int64) random.Below( 30 );
                    const CString key = CString( region ) + ToString( year ) + ToString( month ) + ToString( day ) + ToString( id );
                    if ( !keys.insert( key ).second )
                        continue;
                    rows.push_back( EventRow( region, year, month, day, id, random.Chance( 10 ) ? std::numeric_limits< Float64 >::quiet_NaN() : (Float64) random.Between( -50, 50 ) * 0.5,
                                              random.Chance( 30 ) ? GUCEF_NULL : ( random.Chance( 50 ) ? "a" : "b" ) ) );
                }
                const TDataTableWriteMode mode = random.Chance( 50 ) ? DATATABLE_WRITE_INSERT : DATATABLE_WRITE_UPSERT;
                CDataTableBatchPtr batch = MakeBatch( table->GetSchema(), rows );
                CDataTableWriteResult vfsResult;
                CDataTableWriteResult memoryResult;
                const TDataTableStatus vfsStatus = table->Write( *batch, mode, vfsResult );
                const TDataTableStatus memoryStatus = reference.Write( *batch, mode, memoryResult );
                ASSERT_TRUE( vfsStatus == memoryStatus );

                // sometimes a delete
                if ( random.Chance( 20 ) )
                {
                    CDataTableQuery remove;
                    remove.Where( CDataTablePredicate( "value", DATATABLE_PREDICATE_LESS, CVariant( (Float64) random.Between( -30, 0 ) ) ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeleteRows( remove, vfsResult ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == reference.DeleteRows( remove, memoryResult ) );
                }

                // queries on partition keys, other columns and both
                for ( UInt32 q=0; q<4; ++q )
                {
                    CDataTableQuery query;
                    if ( random.Chance( 50 ) )
                        query.WhereEquals( "region", CVariant( CUtf8String( regions[ random.Below( 5 ) ] ) ) );
                    else
                    if ( random.Chance( 50 ) )
                    {
                        CVariant::VariantVector set;
                        set.push_back( CVariant( CUtf8String( regions[ random.Below( 5 ) ] ) ) );
                        set.push_back( CVariant( CUtf8String( regions[ random.Below( 5 ) ] ) ) );
                        query.WhereIn( "region", set );
                    }
                    if ( random.Chance( 60 ) )
                    {
                        const Int32 year = 2019 + (Int32) random.Below( 4 );
                        query.WhereRange( "day", Day( year, 1 + random.Below( 12 ), 1 ), random.Chance( 50 ), Day( year + (Int32) random.Below( 2 ), 6, 15 ), random.Chance( 50 ) );
                    }
                    if ( random.Chance( 30 ) )
                        query.Where( CDataTablePredicate( "value", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Float64) random.Between( -20, 20 ) ) ) );
                    if ( random.Chance( 20 ) )
                        query.WhereNull( "note", random.Chance( 50 ) );
                    CString::StringVector vfsRows;
                    CString::StringVector memoryRows;
                    ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, query, vfsRows, GUCEF_NULL, random.Chance( 30 ) ) );
                    ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( reference, query, memoryRows ) );
                    ++comparisons;
                    if ( vfsRows != memoryRows )
                        ++mismatches;
                }
            }
            ASSERT_TRUE( 120 == comparisons && 0 == mismatches );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "random" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: partitions fixed by predicates are found without listing, others pruned while listing, and scans open only the files they need" )
        try
        {
            MountFreshDirectory( "pruning", "vfstests/pruning", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/pruning", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            static const char* const regions[] = { "EU", "US", "APAC" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 3, 2019, 2022, 20, 1.0 ), DATATABLE_WRITE_INSERT ) );

            // region fixed: no listing of the regions; the years of EU listed once, three of four pruned there
            ResetStorage( table );
            CDataTableScanStats stats;
            CString::StringVector rows;
            CDataTableQuery euIn2021;
            euIn2021.WhereEquals( "region", CVariant( "EU" ) ).WhereRange( "day", Day( 2021, 1, 1 ), true, Day( 2021, 12, 31 ), true );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, euIn2021, rows, &stats ) );
            ASSERT_TRUE( 20 == rows.size() );
            ASSERT_TRUE( 1 == stats.filesListed && 1 == stats.filesOpened && 3 == stats.filesPruned );
            TStorageStats storage = StorageStats( table );
            ASSERT_TRUE( 1 == storage.directoryListings && 1 == storage.footerReads && 0 == storage.footerCacheHits );

            // every field fixed: the file names are built, probed and opened, nothing is listed
            ResetStorage( table );
            CVariant::VariantVector twoRegions;
            twoRegions.push_back( CVariant( "EU" ) );
            twoRegions.push_back( CVariant( "US" ) );
            CDataTableQuery oneDay;
            oneDay.WhereIn( "region", twoRegions ).WhereEquals( "day", Day( 2020, 5, 5 ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, oneDay, rows, &stats ) );
            ASSERT_TRUE( 2 == rows.size() && 2 == stats.filesOpened && 0 == stats.filesPruned );
            storage = StorageStats( table );
            ASSERT_TRUE( 0 == storage.directoryListings && 2 == storage.fileProbes && 2 == storage.footerReads );

            // the same again: the footers come from the cache
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, oneDay, rows, &stats ) );
            ASSERT_TRUE( 2 == stats.footerCacheHits );
            storage = StorageStats( table );
            ASSERT_TRUE( 2 == storage.footerReads && 2 == storage.footerCacheHits );

            // a sparse partition: no file, no rows, no error
            CDataTableQuery missing;
            missing.WhereEquals( "region", CVariant( "LATAM" ) ).WhereEquals( "day", Day( 2020, 5, 5 ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, missing, rows, &stats ) );
            ASSERT_TRUE( rows.empty() && 0 == stats.filesListed );

            // a range on the year only: the regions and the years of each region are listed (4), nine files pruned
            ResetStorage( table );
            CDataTableQuery in2022;
            in2022.WhereRange( "day", Day( 2022, 1, 1 ), true, Day( 2023, 1, 1 ), false );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, in2022, rows, &stats ) );
            ASSERT_TRUE( 60 == rows.size() && 3 == stats.filesOpened && 9 == stats.filesPruned );
            storage = StorageStats( table );
            ASSERT_TRUE( 4 == storage.directoryListings );

            // listings are cached: the same query lists nothing again
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, in2022, rows, &stats ) );
            storage = StorageStats( table );
            ASSERT_TRUE( 4 == storage.directoryListings && 4 == storage.listingCacheHits );

            // a write of a new partition reaches the cached listings: no relisting, the new file is found
            TRowVector added;
            added.push_back( EventRow( "LATAM", 2022, 3, 3, 1, 1.0, GUCEF_NULL ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, added, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, in2022, rows, &stats ) );
            ASSERT_TRUE( 61 == rows.size() );
            storage = StorageStats( table );
            ASSERT_TRUE( 5 == storage.directoryListings );      // only the new region's directory

            // with no predicates every file is read, in path order of the key values
            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery(), true, partitions ) );
            ASSERT_TRUE( 13 == partitions.size() );
            ASSERT_TRUE( "APAC/2019.parquet" == partitions[ 0 ].path && "US/2022.parquet" == partitions[ 12 ].path );
            ASSERT_TRUE( partitions[ 0 ].hasFileStats && 20 == partitions[ 0 ].rowCount && partitions[ 0 ].fileSize > 0 );
            ASSERT_TRUE( partitions[ 0 ].keyValues.GetValueAlways( "region" ).AsString() == "APAC" );

            // listings filtered on row group statistics: no file holds an id above 100
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery().Where( CDataTablePredicate( "id", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 100 ) ) ), false, partitions ) );
            ASSERT_TRUE( partitions.empty() );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "pruning" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a scan that holds a partition keeps reading the old file while a write replaces it" )
        try
        {
            MountFreshDirectory( "snapshot", "vfstests/snapshot", true );
            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/snapshot", false );
            settings.backendOptions.Set( "replaceRetryTimeoutMs", CVariant( "500" ) );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( settings );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events", "parquet", 1000 ) );
            TRowVector rows;
            for ( UInt32 i=0; i<5000; ++i )
                rows.push_back( EventRow( "EU", 2020, 1 + i % 12, 1 + i % 28, (Int64) i, 1.0, GUCEF_NULL ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );

            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Scan( CDataTableQuery().Select( "value" ).SetBatchRowCount( 1024 ), scan ) );
            CDataTableBatchPtr batch;
            ASSERT_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( batch ) );
            UInt32 oldRows = batch->GetRowCount();
            UInt32 newValues = 0;

            // replaced while the scan is in its first row group: five row groups of the old file are still to come
            for ( UInt32 i=0; i<(UInt32) rows.size(); ++i )
                rows[ i ][ 3 ] = CVariant( (Float64) 2.0 );
            #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_UPSERT ) );
            #else
            // Windows refuses to rename over a file a reader holds open (sharing violation): the write fails after its
            // retries instead of rewriting the file under the scan
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == WriteRows( *table, rows, DATATABLE_WRITE_UPSERT ) );
            #endif
            TDataTableStatus status = DATATABLE_STATUS_OK;
            while ( DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) )
            {
                oldRows += batch->GetRowCount();
                for ( UInt32 r=0; r<batch->GetRowCount(); ++r )
                {
                    CVariant value;
                    batch->GetValue( r, 0, value );
                    if ( 1.0 != value.AsFloat64() )
                        ++newValues;
                }
            }
            ASSERT_TRUE( DATATABLE_STATUS_END == status );
            ASSERT_TRUE( 5000 == oldRows && 0 == newValues );
            scan.Unlink();
            batch.Reset();
            #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_UPSERT ) );
            #endif
            ASSERT_TRUE( 5000 == CountRows( *table, CDataTableQuery().WhereEquals( "value", CVariant( (Float64) 2.0 ) ) ) );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "snapshot" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: temporary files this writer left behind are removed at open, those of other writers are left, scans never read them" )
        try
        {
            const CString dir = MountFreshDirectory( "leftovers", "vfstests/leftovers", true );
            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/leftovers", false );
            settings.backendOptions.Set( "writerId", CVariant( "WRITER1" ) );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( settings );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            static const char* const regions[] = { "EU" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 10, 1.0 ), DATATABLE_WRITE_INSERT ) );
            table.Unlink();
            client.Unlink();

            // what a write cut short leaves: a partial file next to the partition
            ASSERT_TRUE( WriteNativeFile( CombinePath( dir, "EU/2020.parquet.tmp-WRITER1-7" ), "PAR1partial", 11 ) );
            ASSERT_TRUE( WriteNativeFile( CombinePath( dir, "EU/2021.parquet.tmp-WRITER1-8" ), "PAR1", 4 ) );
            ASSERT_TRUE( WriteNativeFile( CombinePath( dir, "EU/2020.parquet.tmp-WRITER2-3" ), "PAR1other", 9 ) );

            client = OpenRequired( settings );
            ASSERT_FALSE( client.IsNULL() );
            TStorageStats storage;
            static_cast< TClient* >( client.GetPointerAlways() )->GetStorageStats( storage );
            ASSERT_TRUE( 2 == storage.temporaryFilesRemoved );
            ASSERT_FALSE( FileExists( CombinePath( dir, "EU/2020.parquet.tmp-WRITER1-7" ) ) || FileExists( CombinePath( dir, "EU/2021.parquet.tmp-WRITER1-8" ) ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "EU/2020.parquet.tmp-WRITER2-3" ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->OpenTable( "events", table ) );
            CDataTableScanStats stats;
            CString::StringVector rows;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), rows, &stats ) );
            ASSERT_TRUE( 10 == rows.size() && 1 == stats.filesListed );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "leftovers" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a write that fails at the end or whose replace is refused leaves the partition as it was and no temporary file" )
        try
        {
            const CString dir = MountFreshDirectory( "faults", "vfstests/faults", true );
            CTableCodecRegistry& registry = CCoreGlobal::Instance()->GetTableCodecRegistry();
            CTableCodecRegistry::TTableCodecPtr parquet;
            ASSERT_TRUE( registry.TryLookup( "parquet", parquet ) );
            CTableCodecRegistry::TTableCodecPtr faulty( CTSharedPtr< CITableCodec, MT::CMutex >( GUCEF_NEW CFaultyTableCodec( parquet ) ) );
            ASSERT_TRUE( registry.TryRegister( "faultyparquet", faulty ) );

            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/faults", false );
            settings.backendOptions.Set( "replaceRetryTimeoutMs", CVariant( "50" ) );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( settings );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events", "faultyparquet" ) );
            ASSERT_FALSE( table.IsNULL() );
            static const char* const regions[] = { "EU" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 10, 1.0 ), DATATABLE_WRITE_INSERT ) );
            const UInt64 sizeBefore = FileSize( CombinePath( dir, "EU/2020.parquet" ) );

            // the writer fails after the rows went to the temporary file
            g_failWrites.Store( 1 );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 20, 2.0 ), DATATABLE_WRITE_UPSERT, &result ) );
            g_failWrites.Store( 0 );
            ASSERT_TRUE( 1 == result.partitions.size() && DATATABLE_STATUS_IO_ERROR == result.partitions[ 0 ].status );
            ASSERT_TRUE( sizeBefore == FileSize( CombinePath( dir, "EU/2020.parquet" ) ) );
            ASSERT_TRUE( 10 == CountRows( *table, CDataTableQuery().WhereEquals( "value", CVariant( (Float64) 1.0 ) ) ) && 10 == CountRows( *table ) );
            ASSERT_TRUE( 0 == CountTemporaryFiles( dir ) );

            // the replace is refused: a directory is where the partition file of US 2021 goes
            ASSERT_TRUE( CreateDirs( CombinePath( dir, "US/2021.parquet" ) ) );
            TRowVector rows;
            rows.push_back( EventRow( "US", 2021, 1, 1, 1, 1.0, GUCEF_NULL ) );
            rows.push_back( EventRow( "EU", 2021, 1, 1, 1, 1.0, GUCEF_NULL ) );
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == WriteRows( *table, rows, DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( 2 == result.partitions.size() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == result.partitions[ 0 ].status && "EU/2021.parquet" == result.partitions[ 0 ].path );
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == result.partitions[ 1 ].status );
            ASSERT_TRUE( DirExists( CombinePath( dir, "US/2021.parquet" ) ) );
            ASSERT_TRUE( 0 == CountTemporaryFiles( dir ) );
            TStorageStats storage = StorageStats( table );
            ASSERT_TRUE( storage.replaceRetries > 0 );
            DeleteDir( CombinePath( dir, "US/2021.parquet" ), true );

            table.Unlink();
            client.Unlink();
            registry.TryUnregister( "faultyparquet" );
            UnmountDirectory( "faults" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a cached footer is checked against the file at every open: a file of the same size another writer put in place is read with its own footer" )
        try
        {
            const CString dirA = MountFreshDirectory( "staleA", "vfstests/staleA", true );
            const CString dirB = MountFreshDirectory( "staleB", "vfstests/staleB", true );
            COMCORE::CIDatabaseClientPtr writerA = OpenRequired( MakeSettings( "vfstests/staleA", false ) );
            COMCORE::CIDatabaseClientPtr writerB = OpenRequired( MakeSettings( "vfstests/staleB", false ) );
            CIDataTablePtr tableA = CreateTable( writerA, MakePricesDefinition() );
            CIDataTablePtr tableB = CreateTable( writerB, MakePricesDefinition() );
            REQUIRE_TRUE( !tableA.IsNULL() && !tableB.IsNULL() );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == WriteRows( *tableA, MakePrices( 0 ), DATATABLE_WRITE_INSERT ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == WriteRows( *tableB, MakePrices( 1000 ), DATATABLE_WRITE_INSERT ) );
            const CString fileA = CombinePath( dirA, "KO.parquet" );
            const CString fileB = CombinePath( dirB, "KO.parquet" );
            REQUIRE_TRUE( FileSize( fileA ) == FileSize( fileB ) );

            // a read only client beside the writer, as another process would be (deployment of 14.8)
            COMCORE::CDatabaseClientSettings readOnly = MakeSettings( "vfstests/staleA", true );
            readOnly.backendOptions.Set( "listingCacheTtlMs", CVariant( "1" ) );
            COMCORE::CIDatabaseClientPtr reader = OpenRequired( readOnly );
            CIDataTablePtr table;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == reader->OpenTable( "prices", table ) && !table.IsNULL() );
            const CDataTableQuery high = CDataTableQuery().Where( CDataTablePredicate( "price", DATATABLE_PREDICATE_GREATER_OR_EQUAL, CVariant( (Int64) 1000 ) ) );
            const CDataTableQuery low = CDataTableQuery().Where( CDataTablePredicate( "price", DATATABLE_PREDICATE_LESS, CVariant( (Int64) 1000 ) ) );
            ASSERT_TRUE( 0 == CountRows( *table, high ) && 100 == CountRows( *table, low ) );

            // the file as it was: its footer comes from the cache
            const TStorageStats cached = StorageStats( table );
            ASSERT_TRUE( 100 == CountRows( *table, low ) );
            const TStorageStats unchanged = StorageStats( table );
            ASSERT_TRUE( unchanged.footerCacheHits == cached.footerCacheHits + 1 && unchanged.footerReads == cached.footerReads );

            // the other writer puts a file of the same size in place, within the same second: no Refresh()
            REQUIRE_TRUE( MoveFile( fileB, fileA, true ) );
            ASSERT_TRUE( 100 == CountRows( *table, high ) );
            ASSERT_TRUE( 0 == CountRows( *table, low ) );
            const TStorageStats replaced = StorageStats( table );
            ASSERT_TRUE( replaced.footerReads == unchanged.footerReads + 1 );

            table.Unlink();
            reader.Unlink();
            tableA.Unlink();
            tableB.Unlink();
            writerA.Unlink();
            writerB.Unlink();
            UnmountDirectory( "staleA" );
            UnmountDirectory( "staleB" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a file opened again for a scan must be the version the scan planned with, also after another writer put a file of the same size in place" )
        try
        {
            const CString dirA = MountFreshDirectory( "reopenA", "vfstests/reopenA", true );
            const CString dirB = MountFreshDirectory( "reopenB", "vfstests/reopenB", true );
            COMCORE::CIDatabaseClientPtr writerA = OpenRequired( MakeSettings( "vfstests/reopenA", false ) );
            COMCORE::CIDatabaseClientPtr writerB = OpenRequired( MakeSettings( "vfstests/reopenB", false ) );
            CIDataTablePtr tableA = CreateTable( writerA, MakePricesDefinition() );
            CIDataTablePtr tableB = CreateTable( writerB, MakePricesDefinition() );
            REQUIRE_TRUE( !tableA.IsNULL() && !tableB.IsNULL() );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == WriteRows( *tableA, MakePrices( 0 ), DATATABLE_WRITE_INSERT ) );
            REQUIRE_TRUE( DATATABLE_STATUS_OK == WriteRows( *tableB, MakePrices( 1000 ), DATATABLE_WRITE_INSERT ) );
            const CString fileA = CombinePath( dirA, "KO.parquet" );
            const CString fileB = CombinePath( dirB, "KO.parquet" );
            REQUIRE_TRUE( FileSize( fileA ) == FileSize( fileB ) );

            CTableCodecRegistry::TTableCodecPtr parquet;
            REQUIRE_TRUE( CCoreGlobal::Instance()->GetTableCodecRegistry().TryLookup( "parquet", parquet ) );
            DBPLUGIN::VFSTABLES::CVfsTableScanSource::TPartitionFileVector files( 1 );
            files[ 0 ].relativePath = "KO.parquet";
            DBPLUGIN::VFSTABLES::CVfsTableScanSource source( static_cast< TVfsTable* >( tableA.GetPointerAlways() )->GetStore(),
                                                              tableA->GetDefinition(), tableA->GetSchema(), parquet, CValueList(), files );
            CDataTableScanFile file;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == source.NextFile( file ) );
            CITableReaderPtr first;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == source.OpenFile( file, first ) && !first.IsNULL() );
            const CITableFileMetaDataPtr planned = first->GetMetaData();   // what a scan keeps of the file while it plans with it
            first.Unlink();     // the last reader of the file goes: the file closes

            // the same version opens again; another one does not
            CITableReaderPtr again;
            ASSERT_TRUE( DATATABLE_STATUS_OK == source.OpenFile( file, again ) && !again.IsNULL() );
            again.Unlink();
            REQUIRE_TRUE( MoveFile( fileB, fileA, true ) );
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == source.OpenFile( file, again ) );

            tableA.Unlink();
            tableB.Unlink();
            writerA.Unlink();
            writerB.Unlink();
            UnmountDirectory( "reopenA" );
            UnmountDirectory( "reopenB" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    #if ( GUCEF_PLATFORM != GUCEF_PLATFORM_MSWIN )
    GUCEF_TESTFW_TESTCASE( "Database: a replace the file system refuses leaves the partition file untouched, also for a scan reading it: it is never rewritten in place" )
        GUCEF_TESTFW_SKIP_IF( 0 == ::geteuid(), "the super user renames in a read only directory" );
        try
        {
            const CString dir = MountFreshDirectory( "refused", "vfstests/refused", true );
            CTableCodecRegistry& registry = CCoreGlobal::Instance()->GetTableCodecRegistry();
            CTableCodecRegistry::TTableCodecPtr parquet;
            REQUIRE_TRUE( registry.TryLookup( "parquet", parquet ) );
            CTableCodecRegistry::TTableCodecPtr faulty( CTSharedPtr< CITableCodec, MT::CMutex >( GUCEF_NEW CFaultyTableCodec( parquet ) ) );
            REQUIRE_TRUE( registry.TryRegister( "faultyparquet", faulty ) );

            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/refused", false );
            settings.backendOptions.Set( "replaceRetryTimeoutMs", CVariant( "50" ) );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( settings );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events", "faultyparquet", 4 ) );
            REQUIRE_TRUE( !table.IsNULL() );
            static const char* const regions[] = { "EU" };
            REQUIRE_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 40, 1.0 ), DATATABLE_WRITE_INSERT ) );
            const CString partition = CombinePath( dir, "EU/2020.parquet" );
            CDynamicBuffer before;
            REQUIRE_TRUE( before.LoadContentFromFile( partition ) && before.GetDataSize() > 0 );

            // a scan holds the old file
            CDataTableScanPtr scan;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == table->Scan( CDataTableQuery().Select( "value" ), scan ) );
            CDataTableBatchPtr batch;
            REQUIRE_TRUE( DATATABLE_STATUS_OK == scan->NextBatch( batch ) );
            UInt32 rowsRead = batch->GetRowCount();
            UInt32 otherValues = 0;

            // the rename is refused while the partition file itself stays writable
            g_readOnlyDirAfterFinish = CombinePath( dir, "EU" );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_IO_ERROR == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 40, 2.0 ), DATATABLE_WRITE_UPSERT, &result ) );
            g_readOnlyDirAfterFinish.Clear();
            ::chmod( CombinePath( dir, "EU" ).C_String(), 0755 );

            CDynamicBuffer after;
            ASSERT_TRUE( after.LoadContentFromFile( partition ) && after.GetDataSize() == before.GetDataSize() &&
                         0 == memcmp( after.GetConstBufferPtr(), before.GetConstBufferPtr(), before.GetDataSize() ) );
            TDataTableStatus status = DATATABLE_STATUS_OK;
            while ( DATATABLE_STATUS_OK == ( status = scan->NextBatch( batch ) ) )
            {
                rowsRead += batch->GetRowCount();
                for ( UInt32 r=0; r<batch->GetRowCount(); ++r )
                {
                    CVariant value;
                    batch->GetValue( r, 0, value );
                    if ( 1.0 != value.AsFloat64() )
                        ++otherValues;
                }
            }
            ASSERT_TRUE( DATATABLE_STATUS_END == status && 40 == rowsRead && 0 == otherValues );
            scan.Unlink();
            batch.Reset();
            ASSERT_TRUE( 40 == CountRows( *table, CDataTableQuery().WhereEquals( "value", CVariant( (Float64) 1.0 ) ) ) && 40 == CountRows( *table ) );

            table.Unlink();
            client.Unlink();
            registry.TryUnregister( "faultyparquet" );
            UnmountDirectory( "refused" );
        }
        catch ( ... )
        {
            g_readOnlyDirAfterFinish.Clear();
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
    #endif

    GUCEF_TESTFW_TESTCASE( "Database: DeleteRows rewrites or deletes partitions, DeletePartitions takes whole partitions, Compact brings files into the canonical form" )
        try
        {
            const CString dir = MountFreshDirectory( "changes", "vfstests/changes", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/changes", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events", "parquet", 100 ) );
            static const char* const regions[] = { "EU", "US" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 2, 2020, 2021, 10, 1.0 ), DATATABLE_WRITE_INSERT ) );

            // rows of one partition go, a partition left empty is deleted
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeleteRows( CDataTableQuery().WhereEquals( "region", CVariant( "EU" ) ).Where( CDataTablePredicate( "id", DATATABLE_PREDICATE_LESS, CVariant( (Int64) 3 ) ) ), result ) );
            ASSERT_TRUE( 2 == result.partitions.size() && 3 == result.partitions[ 0 ].rowsDeleted && 7 == result.partitions[ 0 ].rowsWritten );
            ASSERT_TRUE( 34 == CountRows( *table ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeleteRows( CDataTableQuery().WhereEquals( "region", CVariant( "US" ) ).WhereRange( "day", Day( 2021, 1, 1 ), true, Day( 2021, 12, 31 ), true ), result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 10 == result.partitions[ 0 ].rowsDeleted && 0 == result.partitions[ 0 ].rowsWritten );
            ASSERT_FALSE( FileExists( CombinePath( dir, "US/2021.parquet" ) ) );

            // partitions by their key values only
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == table->DeletePartitions( CDataTableQuery().WhereEquals( "id", CVariant( (Int64) 5 ) ), result ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->DeletePartitions( CDataTableQuery().WhereEquals( "region", CVariant( "EU" ) ), result ) );
            ASSERT_TRUE( 2 == result.partitions.size() && 7 == result.partitions[ 0 ].rowsDeleted );
            ASSERT_FALSE( FileExists( CombinePath( dir, "EU/2020.parquet" ) ) || FileExists( CombinePath( dir, "EU/2021.parquet" ) ) );
            ASSERT_TRUE( 10 == CountRows( *table ) );

            // files in the layout of the definition are canonical; a foreign file is not
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Compact( CDataTableQuery(), false, result ) );
            ASSERT_TRUE( result.partitions.empty() );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Compact( CDataTableQuery(), true, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && DATATABLE_STATUS_OK == result.partitions[ 0 ].status );

            // a foreign file at a partition path: unsorted, a duplicate key, small row groups, no layout metadata
            CDataTableDefinition foreign = MakeEventsDefinition( "foreign", "parquet", 3 );
            foreign.schema.SetPrimaryKey( CString::StringVector() );
            foreign.partitioning.Clear();
            foreign.schema.SetSortKey( CString::StringVector() );
            CTableCodecRegistry::TTableCodecPtr parquet;
            CCoreGlobal::Instance()->GetTableCodecRegistry().TryLookup( "parquet", parquet );
            {
                CFileDataTable file( CombinePath( dir, "EU/2023.parquet" ), parquet, CValueList(), foreign );
                TRowVector rows;
                rows.push_back( EventRow( "EU", 2023, 5, 5, 3, 3.0, "late" ) );
                rows.push_back( EventRow( "EU", 2023, 1, 1, 1, 1.0, "early" ) );
                rows.push_back( EventRow( "EU", 2023, 5, 5, 3, 4.0, "dup" ) );
                rows.push_back( EventRow( "EU", 2023, 2, 2, 2, 2.0, "mid" ) );
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( file, rows, DATATABLE_WRITE_INSERT ) );
            }
            table->Refresh();
            ASSERT_TRUE( 4 == CountRows( *table, CDataTableQuery().WhereEquals( "region", CVariant( "EU" ) ) ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Compact( CDataTableQuery().WhereEquals( "region", CVariant( "EU" ) ), false, result ) );
            ASSERT_TRUE( 1 == result.partitions.size() && 1 == result.partitions[ 0 ].rowsReplaced && 3 == result.partitions[ 0 ].rowsWritten );
            CString::StringVector rows;
            CDataTableScanPtr ordered;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Scan( CDataTableQuery().Select( "note" ).WhereEquals( "region", CVariant( "EU" ) ), ordered ) );
            ASSERT_TRUE( DATATABLE_STATUS_END == CORETEST::CollectScanRows( *ordered, rows ) );
            ASSERT_TRUE( 3 == rows.size() && "early|" == rows[ 0 ] && "mid|" == rows[ 1 ] && "dup|" == rows[ 2 ] );
            ordered.Unlink();

            // a foreign file whose rows belong elsewhere is refused, not spread
            {
                CFileDataTable file( CombinePath( dir, "US/2019.parquet" ), parquet, CValueList(), foreign );
                TRowVector misplaced;
                misplaced.push_back( EventRow( "EU", 2019, 1, 1, 1, 1.0, GUCEF_NULL ) );
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( file, misplaced, DATATABLE_WRITE_INSERT ) );
            }
            table->Refresh();
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == table->Compact( CDataTableQuery().WhereEquals( "region", CVariant( "US" ) ), true, result ) );
            ASSERT_TRUE( 0 == CountTemporaryFiles( dir ) );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "changes" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a nullable column appended to the table reads as null from the files written before" )
        try
        {
            MountFreshDirectory( "evolution", "vfstests/evolution", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/evolution", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            static const char* const regions[] = { "EU", "US" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 2, 2020, 2020, 5, 1.0 ), DATATABLE_WRITE_INSERT ) );
            table.Unlink();

            // version 2 of the definition appends a nullable column
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->DropTable( "events", false ) );
            CDataTableDefinition second = MakeEventsDefinition( "events" );
            second.schema.AddColumn( CDataTableColumnDef( "source", DATATABLE_TYPE_UTF8, true ) );
            second.schema.SetSchemaVersion( 2 );
            table = CreateTable( client, second );
            ASSERT_FALSE( table.IsNULL() );
            ASSERT_TRUE( 10 == CountRows( *table, CDataTableQuery().WhereNull( "source", true ) ) );

            TRowVector rows;
            TRow row = EventRow( "EU", 2020, 12, 1, 99, 3.0, GUCEF_NULL );
            row.push_back( CVariant( "feed" ) );
            rows.push_back( row );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( 1 == CountRows( *table, CDataTableQuery().WhereEquals( "source", CVariant( "feed" ) ) ) );
            ASSERT_TRUE( 10 == CountRows( *table, CDataTableQuery().WhereNull( "source", true ) ) );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "evolution" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a file the table cannot read fails the scan with its path, or is skipped and counted with onIncompatibleFile = skip" )
        try
        {
            const CString dir = MountFreshDirectory( "incompatible", "vfstests/incompatible", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/incompatible", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            static const char* const regions[] = { "EU" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 5, 1.0 ), DATATABLE_WRITE_INSERT ) );
            table.Unlink();
            client.Unlink();

            // "value" as text in a file at a partition path
            CDataTableDefinition wrong = MakeEventsDefinition( "wrong" );
            wrong.partitioning.Clear();
            CDataTableSchema schema;
            schema.AddColumn( CDataTableColumnDef( "region", DATATABLE_TYPE_UTF8, false ) );
            schema.AddColumn( CDataTableColumnDef( "day", DATATABLE_TYPE_DATE, false ) );
            schema.AddColumn( CDataTableColumnDef( "id", DATATABLE_TYPE_INT64, false ) );
            schema.AddColumn( CDataTableColumnDef( "value", DATATABLE_TYPE_UTF8, true ) );
            wrong.schema = schema;
            CTableCodecRegistry::TTableCodecPtr parquet;
            CCoreGlobal::Instance()->GetTableCodecRegistry().TryLookup( "parquet", parquet );
            {
                CFileDataTable file( CombinePath( dir, "US/2020.parquet" ), parquet, CValueList(), wrong );
                TRowVector rows;
                TRow row;
                row.push_back( CVariant( "US" ) );
                row.push_back( Day( 2020, 1, 1 ) );
                row.push_back( CVariant( (Int64) 1 ) );
                row.push_back( CVariant( "text" ) );
                rows.push_back( row );
                ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( file, rows, DATATABLE_WRITE_INSERT ) );
            }

            client = OpenRequired( MakeSettings( "vfstests/incompatible", true ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->OpenTable( "events", table ) );
            CString::StringVector rows;
            CDataTableScanPtr scan;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Scan( CDataTableQuery(), scan ) );
            ASSERT_TRUE( DATATABLE_STATUS_SCHEMA_MISMATCH == CORETEST::CollectScanRows( *scan, rows ) );
            ASSERT_TRUE( scan->GetLastErrorMessage().HasSubstr( "US/2020.parquet" ) >= 0 );
            scan.Unlink();
            table.Unlink();
            client.Unlink();

            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/incompatible", true );
            settings.backendOptions.Set( "onIncompatibleFile", CVariant( "skip" ) );
            client = OpenRequired( settings );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->OpenTable( "events", table ) );
            CDataTableScanStats stats;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), rows, &stats ) );
            ASSERT_TRUE( 5 == rows.size() && 1 == stats.filesSkipped && 2 == stats.filesListed );

            // a file that is not a table file at all fails also when skipping
            ASSERT_TRUE( WriteNativeFile( CombinePath( dir, "EU/2019.parquet" ), "this is not parquet at all", 26 ) );
            table->Refresh();
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->Scan( CDataTableQuery(), scan ) );
            const TDataTableStatus damaged = CORETEST::CollectScanRows( *scan, rows );
            ASSERT_TRUE( DATATABLE_STATUS_CORRUPT_DATA == damaged );
            scan.Unlink();
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "incompatible" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a read only database reads its tables and refuses every change" )
        try
        {
            MountFreshDirectory( "readonly", "vfstests/readonly", true );
            COMCORE::CIDatabaseClientPtr writer = OpenRequired( MakeSettings( "vfstests/readonly", false ) );
            CIDataTablePtr table = CreateTable( writer, MakeEventsDefinition( "events" ) );
            static const char* const regions[] = { "EU" };
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, MakeEvents( regions, 1, 2020, 2020, 5, 1.0 ), DATATABLE_WRITE_INSERT ) );
            table.Unlink();
            writer.Unlink();

            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/readonly", true ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == client->OpenTable( "events", table ) );
            ASSERT_TRUE( table->IsReadOnly() && 5 == CountRows( *table ) );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == WriteRows( *table, MakeEvents( regions, 1, 2021, 2021, 1, 1.0 ), DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table->DeleteRows( CDataTableQuery(), result ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table->DeletePartitions( CDataTableQuery(), result ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == table->Compact( CDataTableQuery(), true, result ) );
            CIDataTablePtr created;
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == client->CreateTable( MakeEventsDefinition( "other" ), created ) );
            ASSERT_TRUE( DATATABLE_STATUS_READ_ONLY == client->DropTable( "events", true ) );
            ASSERT_TRUE( 5 == CountRows( *table ) );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "readonly" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: hostile key values stay inside the root and apart from each other, values a path cannot hold are refused" )
        try
        {
            const CString parent = MountFreshDirectory( "hostile", "vfstests/hostileparent", true );
            UnmountDirectory( "hostile" );
            const CString dir = CombinePath( parent, "root" );
            CreateDirs( dir );
            vfs.AddRoot( dir, "VfsTablesTest_hostileroot", true, false, "vfstests/hostile" );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/hostile", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            ASSERT_FALSE( table.IsNULL() );

            static const char* const hostile[] = { "..", ".", "...", "../..", "../../outside", "/absolute", "\\windows", "C:", "c:\\x", "con", "CON", "NUL.txt",
                                                   "com1", "LPT9.log", "AUX", "ko", "KO", "Ko", "a/b", "a\\b", "%2F", "%", "%%41", "x.", ".x", " lead", "trail ",
                                                   "\xC3\xA9t\xC3\xA9", "tab\there", "line\nbreak", "*?<>|\"", "BRK.B", "BF-B", "_tables", "_TABLES",
                                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" };
            const UInt32 count = (UInt32) ( sizeof( hostile ) / sizeof( hostile[ 0 ] ) );
            TRowVector rows;
            for ( UInt32 i=0; i<count; ++i )
                rows.push_back( EventRow( hostile[ i ], 2020, 1, 1, (Int64) i, (Float64) i, GUCEF_NULL ) );
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( count == result.partitions.size() );

            // every value reads back from its own partition, and only from it
            UInt32 wrong = 0;
            for ( UInt32 i=0; i<count; ++i )
            {
                CString::StringVector found;
                CDataTableScanStats stats;
                if ( DATATABLE_STATUS_OK != ScanRows( *table, CDataTableQuery().Select( "id" ).WhereEquals( "region", CVariant( CUtf8String( hostile[ i ] ) ) ), found, &stats ) ||
                     1 != found.size() || ToString( i ) + "|" != found[ 0 ] || 1 != stats.filesOpened )
                    ++wrong;
            }
            ASSERT_TRUE( 0 == wrong );
            ASSERT_TRUE( count == CountRows( *table ) );

            // everything is below the root: one file per value, the definition, nothing beside the root
            ASSERT_TRUE( count + 1 == CountFilesBelow( dir ) );
            ASSERT_TRUE( count + 1 == CountFilesBelow( parent ) );

            // a value a path cannot hold: empty, or with a NUL byte
            CDataTableBatchBuilder builder( table->GetSchema(), CCoreGlobal::Instance()->GetDataTableBufferPool() );
            builder.GetColumn( 0 ).AppendUtf8( "A\0B", 3 );
            builder.GetColumn( 1 ).AppendDate( 0 );
            builder.GetColumn( 2 ).AppendInt64( 1000 );
            builder.GetColumn( 3 ).AppendNull();
            builder.GetColumn( 4 ).AppendNull();
            CDataTableBatchPtr withNul;
            CString error;
            ASSERT_TRUE( DATATABLE_STATUS_OK == builder.Build( withNul, error ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == table->Write( *withNul, DATATABLE_WRITE_INSERT, result ) );
            TRowVector empty;
            empty.push_back( EventRow( "", 2020, 1, 1, 1001, 1.0, GUCEF_NULL ) );
            ASSERT_TRUE( DATATABLE_STATUS_INVALID_ARGUMENT == WriteRows( *table, empty, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( count + 1 == CountFilesBelow( parent ) );

            table.Unlink();
            client.Unlink();
            vfs.UnmountArchiveByName( "VfsTablesTest_hostileroot" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a level of more than 65535 partitions is listed whole and in numeric order, a fixed key is found without listing it" )
        try
        {
            const CString dir = MountFreshDirectory( "wide", "vfstests/wide", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/wide", false ) );
            CDataTableDefinition definition;
            definition.name = "wide";
            definition.codecType = "parquet";
            definition.schema.AddColumn( CDataTableColumnDef( "key", DATATABLE_TYPE_INT64, false ) );
            definition.schema.AddColumn( CDataTableColumnDef( "value", DATATABLE_TYPE_INT64, true ) );
            definition.partitioning.AddField( "key", "key", DATATABLE_PARTITION_TRANSFORM_IDENTITY );
            definition.partitioning.SetPathTemplate( "keys/{key}.parquet" );
            CIDataTablePtr table = CreateTable( client, definition );
            ASSERT_FALSE( table.IsNULL() );

            // files are put there directly: listing does not read them
            const UInt32 fileCount = 70000;
            CreateDirs( CombinePath( dir, "keys" ) );
            UInt32 made = 0;
            for ( UInt32 i=1; i<=fileCount; ++i )
                made += WriteNativeFile( CombinePath( dir, "keys/" + ToString( i ) + ".parquet" ), GUCEF_NULL, 0 ) ? 1 : 0;
            ASSERT_TRUE( fileCount == made );

            ResetStorage( table );
            CDataTablePartitionInfo::TPartitionInfoVector partitions;
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery(), false, partitions ) );
            ASSERT_TRUE( fileCount == partitions.size() );
            if ( fileCount == partitions.size() )
                ASSERT_TRUE( "keys/1.parquet" == partitions[ 0 ].path && "keys/2.parquet" == partitions[ 1 ].path && "keys/70000.parquet" == partitions[ fileCount - 1 ].path );
            TStorageStats storage = StorageStats( table );
            ASSERT_TRUE( 1 == storage.directoryListings );

            ResetStorage( table );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery().WhereEquals( "key", CVariant( (Int64) 69999 ) ), false, partitions ) );
            ASSERT_TRUE( 1 == partitions.size() && "keys/69999.parquet" == partitions[ 0 ].path );
            storage = StorageStats( table );
            ASSERT_TRUE( 0 == storage.directoryListings && 1 == storage.fileProbes );
            ASSERT_TRUE( DATATABLE_STATUS_OK == table->ListPartitions( CDataTableQuery().Where( CDataTablePredicate( "key", DATATABLE_PREDICATE_GREATER, CVariant( (Int64) 69990 ) ) ), false, partitions ) );
            ASSERT_TRUE( 10 == partitions.size() );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "wide" );
            DeleteDir( dir, true );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: one write of 2000 partitions, read whole, by key and through async scans" )
        try
        {
            MountFreshDirectory( "many", "vfstests/many", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/many", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events" ) );
            TRowVector rows;
            for ( UInt32 p=0; p<2000; ++p )
            {
                const CString region = "R" + ToString( p );
                for ( UInt32 i=0; i<10; ++i )
                    rows.push_back( EventRow( region.C_String(), 2020, 1 + i, 1, (Int64) i, (Float64) p, GUCEF_NULL ) );
            }
            CDataTableWriteResult result;
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, rows, DATATABLE_WRITE_INSERT, &result ) );
            ASSERT_TRUE( 2000 == result.partitions.size() && 20000 == result.GetTotalRowsWritten() );

            CString::StringVector found;
            CDataTableScanStats stats;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), found, &stats ) );
            ASSERT_TRUE( 20000 == found.size() && 2000 == stats.filesOpened );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery(), found, &stats, true ) );
            ASSERT_TRUE( 20000 == found.size() && 2000 == stats.filesOpened && 2000 == stats.footerCacheHits );

            ResetStorage( table );
            CVariant::VariantVector three;
            three.push_back( CVariant( "R7" ) );
            three.push_back( CVariant( "R1999" ) );
            three.push_back( CVariant( "R500" ) );
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery().WhereIn( "region", three ), found, &stats ) );
            ASSERT_TRUE( 30 == found.size() && 3 == stats.filesOpened );
            TStorageStats storage = StorageStats( table );
            ASSERT_TRUE( 3 == storage.directoryListings );          // the regions are built, the years of each are listed
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "many" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: scans beside a writer see every partition whole, before or after each replacement" )
        try
        {
            MountFreshDirectory( "concurrent", "vfstests/concurrent", true );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( MakeSettings( "vfstests/concurrent", false ) );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events", "parquet", 8 ) );
            ASSERT_FALSE( table.IsNULL() );

            CPartitionWriterThread* writer = GUCEF_NEW CPartitionWriterThread( table, 60 );
            CPartitionReaderThread* readers[ 3 ];
            for ( UInt32 i=0; i<3; ++i )
                readers[ i ] = GUCEF_NEW CPartitionReaderThread( table, writer->done, 2 == i );
            ASSERT_TRUE( writer->Activate() );
            for ( UInt32 i=0; i<3; ++i )
                ASSERT_TRUE( readers[ i ]->Activate() );

            WaitForDone( writer->done, 300000 );
            UInt32 torn = 0;
            UInt32 scans = 0;
            UInt32 failures = writer->failures.Load();
            for ( UInt32 i=0; i<3; ++i )
            {
                WaitForDone( readers[ i ]->done, 60000 );
                readers[ i ]->Deactivate( false, true );
                torn += readers[ i ]->torn.Load();
                scans += readers[ i ]->scans.Load();
                failures += readers[ i ]->failures.Load();
                GUCEF_DELETE readers[ i ];
            }
            ASSERT_TRUE( 1 == writer->done.Load() );
            writer->Deactivate( false, true );
            GUCEF_DELETE writer;
            ASSERT_TRUE( 0 == failures && 0 == torn && scans > 0 );
            GUCEF_LOG( LOGLEVEL_NORMAL, "VfsTables concurrency: " << scans << " scans beside 60 replacements of 3 partitions" );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "concurrent" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: the async calls of the client and its tables complete their futures with their results" )
        try
        {
            MountFreshDirectory( "async", "vfstests/async", true );
            COMCORE::CIDatabaseClientPtr client = factory.Create( "vfstables" );
            CTaskPtr task;
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( client->OpenAsync( MakeSettings( "vfstests/async", false ) ), 20000, task ) );
            ASSERT_TRUE( client->IsOpen() );
            CFutureResult created = client->CreateTableAsync( MakeEventsDefinition( "events" ) );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( created, 20000, task ) );
            CIDataTablePtr table;
            REQUIRE_TRUE( COMCORE::CDatabaseTableResult::TryGetFromFuture( created, table ) && !table.IsNULL() );

            static const char* const regions[] = { "EU", "US" };
            CFutureResult written = table->WriteAsync( MakeBatch( table->GetSchema(), MakeEvents( regions, 2, 2020, 2021, 10, 1.0 ) ), DATATABLE_WRITE_INSERT );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( written, 20000, task ) );
            const CDataTableWriteResult* writeResult = static_cast< const CDataTableWriteResult* >( task->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != writeResult && 40 == writeResult->GetTotalRowsWritten() );
            CFutureResult conflict = table->WriteAsync( MakeBatch( table->GetSchema(), MakeEvents( regions, 1, 2020, 2020, 1, 1.0 ) ), DATATABLE_WRITE_INSERT );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_STARTUP_FAILED == AwaitFuture( conflict, 20000, task ) );
            ASSERT_TRUE( task->GetTaskStatusExtraInfo().HasSubstr( "CONFLICT" ) >= 0 );

            CFutureResult listed = table->ListPartitionsAsync( CDataTableQuery(), true );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( listed, 20000, task ) );
            const CDataTablePartitionInfoList* list = static_cast< const CDataTablePartitionInfoList* >( task->GetTaskData() );
            ASSERT_TRUE( GUCEF_NULL != list && 4 == list->partitions.size() );

            CFutureResult names = client->ListTablesAsync();
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( names, 20000, task ) );
            CString::StringSet tableNames;
            ASSERT_TRUE( COMCORE::CDatabaseTableNamesResult::TryGetFromFuture( names, tableNames ) && 1 == tableNames.size() );

            CString::StringVector rows;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, CDataTableQuery().WhereEquals( "region", CVariant( "US" ) ), rows, GUCEF_NULL, true ) );
            ASSERT_TRUE( 20 == rows.size() );
            table.Unlink();
            CFutureResult dropped = client->DropTableAsync( "events", true );
            ASSERT_TRUE( TTaskStatus::TASKSTATUS_FINISHED == AwaitFuture( dropped, 20000, task ) );
            client.Unlink();
            UnmountDirectory( "async" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Database: a table of CSV files through the built in codec, with durable writes" )
        try
        {
            const CString dir = MountFreshDirectory( "csv", "vfstests/csv", true );
            COMCORE::CDatabaseClientSettings settings = MakeSettings( "vfstests/csv", false );
            settings.backendOptions.Set( "syncWrites", CVariant( "true" ) );
            COMCORE::CIDatabaseClientPtr client = OpenRequired( settings );
            CIDataTablePtr table = CreateTable( client, MakeEventsDefinition( "events", "csv" ) );
            ASSERT_FALSE( table.IsNULL() );
            static const char* const regions[] = { "EU", "US" };
            TRowVector all = MakeEvents( regions, 2, 2020, 2021, 6, 1.5 );
            ASSERT_TRUE( DATATABLE_STATUS_OK == WriteRows( *table, all, DATATABLE_WRITE_INSERT ) );
            ASSERT_TRUE( FileExists( CombinePath( dir, "EU/2020.csv" ) ) );
            CDataTableQuery query;
            query.WhereEquals( "region", CVariant( "US" ) ).WhereNull( "note", false );
            CString::StringVector rows;
            CString::StringVector expected;
            ASSERT_TRUE( DATATABLE_STATUS_OK == ScanRows( *table, query, rows ) );
            BruteForceRows( table->GetSchema(), all, query, expected );
            ASSERT_TRUE( !expected.empty() && rows == expected );
            table.Unlink();
            client.Unlink();
            UnmountDirectory( "csv" );
        }
        catch ( ... )
        {
            ERRORHERE;
        }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
