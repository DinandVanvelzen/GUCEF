/*
 *  gucefDRN_TestApp: GUCEF test application for the DRN module
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

#include <exception>

#ifndef GUCEF_CORE_H
#include "gucefCORE.h"
#define GUCEF_CORE_H
#endif /* GUCEF_CORE_H ? */

#ifndef GUCEF_CORE_CDATATABLEVALUES_H
#include "gucefCORE_CDataTableValues.h"
#define GUCEF_CORE_CDATATABLEVALUES_H
#endif /* GUCEF_CORE_CDATATABLEVALUES_H ? */

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_MT_CACTIVEOBJECT_H
#include "gucefMT_CActiveObject.h"
#define GUCEF_MT_CACTIVEOBJECT_H
#endif /* GUCEF_MT_CACTIVEOBJECT_H ? */

#ifndef GUCEF_DRN_H
#include "gucefDRN.h"
#define GUCEF_DRN_H
#endif /* GUCEF_DRN_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestDRNTestUtils.h"
#include "TestDRNGroup.h"
#include "TestDRNAllocationCounter.h"
#include "TestDRNTablePublisher.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;
using namespace DRNTEST;

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)
#define REQUIRE_TRUE(t)  do { if ( !(t) ) { ::GUCEF::TESTFW::CTestContext::Instance().RecordFailure( #t, __FILE__, __LINE__ ); throw std::exception(); } } while ( 0 )

namespace {

typedef GUCEF::map< CORE::CVariant, CORE::CVariant::VariantVector > TRowMap;

/*-------------------------------------------------------------------------*/

UInt64
NowMs( void )
{
    return DRN::CDRNClock::NowInMicroseconds() / 1000;
}

/*-------------------------------------------------------------------------*/

/** A page: an id key, a name, a decimal amount, a timestamp, a flag, a ratio, a date and a hidden sort key */
DRN::CDRNPublishedTableSchema
PageSchema( void )
{
    DRN::CDRNPublishedTableSchema schema;
    DRN::CDRNPublishedColumn id( "id", CORE::CDataTableColumnDef( "id", CORE::DATATABLE_TYPE_INT64, false ) );
    DRN::CDRNPublishedColumn name( "name", CORE::CDataTableColumnDef( "name", CORE::DATATABLE_TYPE_UTF8 ) );
    name.title = "Name";
    name.width = 120;
    DRN::CDRNPublishedColumn amount( "amount", CORE::CDataTableColumnDef::Decimal( "amount", 18, 6 ) );
    amount.hints.Set( "alignment", CORE::CVariant( "right" ) );
    amount.hints.Set( "decimals", CORE::CVariant( "2" ) );
    amount.hints.Set( "prefix", CORE::CVariant( "$" ) );
    DRN::CDRNPublishedColumn when( "when", CORE::CDataTableColumnDef::Timestamp( "when", CORE::DATATABLE_TIMEUNIT_MICROSECONDS, true ) );
    DRN::CDRNPublishedColumn flag( "flag", CORE::CDataTableColumnDef( "flag", CORE::DATATABLE_TYPE_BOOL ) );
    DRN::CDRNPublishedColumn ratio( "ratio", CORE::CDataTableColumnDef( "ratio", CORE::DATATABLE_TYPE_FLOAT64 ) );
    DRN::CDRNPublishedColumn day( "day", CORE::CDataTableColumnDef( "day", CORE::DATATABLE_TYPE_DATE ) );
    DRN::CDRNPublishedColumn order( "order", CORE::CDataTableColumnDef( "sortKey", CORE::DATATABLE_TYPE_INT32 ) );
    order.hidden = true;
    schema.columns.push_back( name );
    schema.columns.push_back( id );
    schema.columns.push_back( amount );
    schema.columns.push_back( when );
    schema.columns.push_back( flag );
    schema.columns.push_back( ratio );
    schema.columns.push_back( day );
    schema.columns.push_back( order );
    schema.keyColumn = 1;
    schema.defaultSortColumn = 7;
    return schema;
}

/*-------------------------------------------------------------------------*/

/** The cells of row n of the page, in schema order without the key */
CORE::CVariant::VariantVector
PageCells( Int64 n, const CORE::CString& suffix = CORE::CString::Empty )
{
    CORE::CVariant::VariantVector cells;
    cells.push_back( CORE::CVariant( "row " + CORE::ToString( n ) + suffix ) );
    CORE::CDecimal amount;
    CORE::CDecimal::FromUnscaled( n * 1000000 + 250000, 6, amount );
    cells.push_back( CORE::CDataTableValues::Decimal( amount ) );
    cells.push_back( 0 == n % 5 ? CORE::CVariant() : CORE::CDataTableValues::Timestamp( CORE::CDateTime( 2026, 1, 2, 3, 4, 5, 0, 0 ), CORE::DATATABLE_TIMEUNIT_MICROSECONDS ) );
    cells.push_back( CORE::CVariant( 0 == n % 2 ) );
    cells.push_back( CORE::CVariant( n * 0.5 ) );
    cells.push_back( CORE::CDataTableValues::Date( CORE::CDate( 2026, 3, 1 ) ) );
    cells.push_back( CORE::CVariant( (Int32) ( 1000 - n ) ) );
    return cells;
}

/*-------------------------------------------------------------------------*/

/** Every row of a table by key, the cells in data table schema order without the key */
bool
ReadAll( CORE::CIDataTable& table, TRowMap& rows )
{
    rows.clear();
    CORE::CDataTableScanPtr scan;
    CORE::CDataTableQuery query;
    if ( CORE::DATATABLE_STATUS_OK != table.Scan( query, scan ) || scan.IsNULL() )
        return false;
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == scan->NextBatch( batch ) && !batch.IsNULL() )
    {
        for ( UInt32 r=0; r<batch->GetRowCount(); ++r )
        {
            CORE::CVariant key;
            batch->GetValue( r, 0, key, false );
            CORE::CVariant::VariantVector& cells = rows[ key ];
            cells.resize( batch->GetColumnCount() - 1 );
            for ( UInt32 c=1; c<batch->GetColumnCount(); ++c )
                batch->GetValue( r, c, cells[ c-1 ], false );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Records RowsChangedEvent and SchemaChangedEvent of a table */
class CTableRecorder : public CORE::CObserver
{
    public:
    UInt32 rowEvents;
    UInt32 schemaEvents;
    GUCEF::vector< CORE::CVariant > keys;
    CTableRecorder( void ) : CORE::CObserver(), rowEvents( 0 ), schemaEvents( 0 ), keys() {}
    virtual ~CTableRecorder() { SignalUpcomingObserverDestruction(); }
    protected:
    virtual void OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventid, CORE::CICloneable* eventdata = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CIDataTable::RowsChangedEvent == eventid && GUCEF_NULL != eventdata )
        {
            ++rowEvents;
            const GUCEF::vector< CORE::CVariant >& changed = static_cast< CORE::TDataTableRowsChangedEventData* >( eventdata )->GetData();
            keys.insert( keys.end(), changed.begin(), changed.end() );
        }
        else
        if ( DRN::CDRNPublishedTable::SchemaChangedEvent == eventid )
        {
            ++schemaEvents;
        }
    }
};

/*-------------------------------------------------------------------------*/

class CPageArrived : public CTestCondition
{
    public:
    DRN::CDRNSession& m_session;
    UInt64 m_revision;
    CPageArrived( DRN::CDRNSession& session, UInt64 revision ) : m_session( session ), m_revision( revision ) {}
    virtual bool IsMet( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        DRN::CDRNDataGrid* grid = m_session.FindGrid( "page" );
        if ( GUCEF_NULL == grid )
            return false;
        DRN::CDRNPublishedTable table( *grid, DRN::CDRNPublishedTableSettings() );
        return table.GetPageRevision() >= m_revision && table.IsConsistent();
    }
};

/*-------------------------------------------------------------------------*/

/** A server publishing a page grid, a client subscribed to it */
class CPageWorld
{
    public:

    CGroup group;
    DRN::CDRNDataGrid* grid;
    DRN::CDRNTablePublisher* publisher;

    CPageWorld( const CORE::CString& name, const CORE::CString& channel = "mem" )
        : group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, name, channel ), grid( GUCEF_NULL ), publisher( GUCEF_NULL )
    {}

    ~CPageWorld()
    {
        GUCEF_DELETE publisher;
    }

    bool Start( const DRN::CDRNTablePublisherSettings& settings = DRN::CDRNTablePublisherSettings() )
    {
        if ( !group.Start( 2 ) )
            return false;
        DRN::CDRNDataGridSettings gridSettings;
        gridSettings.name = "page";
        gridSettings.reliable = true;
        gridSettings.deltaAgainstAck = true;
        grid = group[ 0 ].CreateGrid( gridSettings );
        if ( GUCEF_NULL == grid )
            return false;
        publisher = GUCEF_NEW DRN::CDRNTablePublisher( *grid, settings );
        return publisher->IsValid() && publisher->SetSchema( PageSchema() );
    }

    bool Publish( UInt32 count, Int64 changedFrom = -1, Int64 changedTo = -1 )
    {
        DRN::CDRNTablePublisherUpdateScope update( *publisher );
        bool all = update.IsOpen();
        for ( UInt32 n=0; n<count; ++n )
            all = publisher->SetRow( CORE::CVariant( (Int64) n ), PageCells( n, ( (Int64) n >= changedFrom && (Int64) n <= changedTo ) ? CORE::CString( " changed" ) : CORE::CString::Empty ) ) && all;
        return all;
    }

    DRN::CDRNDataGrid* Replica( UInt64 revision )
    {
        CPageArrived arrived( group[ 1 ], revision );
        if ( !PumpUntil( arrived, 15000 ) )
            return GUCEF_NULL;
        return group[ 1 ].FindGrid( "page" );
    }
};

/*-------------------------------------------------------------------------*/

/** The whole table written by the CSV table codec */
bool
ExportCsv( CORE::CIDataTable& table, CORE::CDynamicBuffer& csv )
{
    CORE::CTableCodecRegistry::TTableCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCsvCodec( codec ) || codec.IsNULL() )
        return false;
    csv.SetDataSize( 0 );
    CORE::CDynamicBufferAccess* access = GUCEF_NEW CORE::CDynamicBufferAccess( &csv, false );
    CORE::IOAccessPtr destination = access->CreateSharedPtr();
    CORE::CITableWriterPtr writer;
    CORE::CString error;
    if ( CORE::DATATABLE_STATUS_OK != codec->CreateWriter( destination, table.GetSchema(), CORE::CDataTableWriterOptions(), writer, error ) )
        return false;
    CORE::CDataTableScanPtr scan;
    CORE::CDataTableQuery query;
    if ( CORE::DATATABLE_STATUS_OK != table.Scan( query, scan ) || scan.IsNULL() )
        return false;
    CORE::CDataTableBatchPtr batch;
    while ( CORE::DATATABLE_STATUS_OK == scan->NextBatch( batch ) && !batch.IsNULL() )
    {
        if ( CORE::DATATABLE_STATUS_OK != writer->WriteBatch( *batch ) )
            return false;
    }
    CORE::CDataTableFileSummary summary;
    return CORE::DATATABLE_STATUS_OK == writer->Finish( summary );
}

/*-------------------------------------------------------------------------*/

/** A memory table of the page's data table schema holding rows first..first+count-1 (key first, then the cells) */
bool
FillMemoryTable( CORE::CMemoryDataTable& table, Int64 first, UInt32 count, const CORE::CString& suffix = CORE::CString::Empty )
{
    CORE::CDataTableSchemaPtr schema = table.GetSchema();
    if ( schema.IsNULL() )
        return false;
    CORE::CDataTableBatchBuilder builder( schema, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
    for ( UInt32 i=0; i<count; ++i )
    {
        Int64 n = first + i;
        CORE::CVariant::VariantVector row;
        row.push_back( CORE::CVariant( n ) );
        CORE::CVariant::VariantVector cells = PageCells( n, suffix );
        row.insert( row.end(), cells.begin(), cells.end() );
        if ( !builder.AppendRow( &row[ 0 ], (UInt32) row.size() ) )
            return false;
    }
    CORE::CDataTableBatchPtr batch;
    CORE::CString error;
    if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) )
        return false;
    CORE::CDataTableWriteResult result;
    return CORE::DATATABLE_STATUS_OK == table.Write( *batch, CORE::DATATABLE_WRITE_UPSERT, result );
}

/*-------------------------------------------------------------------------*/

/** Counts the log messages made while it is attached to the log manager */
class CLogLineCounter : public CORE::CILogger
{
    public:

    CLogLineCounter( void ) : CORE::CILogger(), m_lines( 0 ) { CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( this ); }
    virtual ~CLogLineCounter() { CORE::CCoreGlobal::Instance()->GetLogManager().RemoveLogger( this ); }

    UInt32 GetLineCount( void )
    {
        CORE::CCoreGlobal::Instance()->GetLogManager().FlushLogs();
        return m_lines.Load();
    }

    virtual void Log( const CORE::TLogMsgType, const Int32, const CORE::CString&, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { m_lines.FetchAdd( 1 ); }
    virtual void Log( const CORE::TLogMsgType, const Int32, const CORE::CVariantStream&, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { m_lines.FetchAdd( 1 ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType, const Int32, const CORE::CString&, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { m_lines.FetchAdd( 1 ); }
    virtual void LogWithoutFormatting( const CORE::TLogMsgType, const Int32, const CORE::CVariantStream&, const UInt32, const CORE::CTimestamp& ) GUCEF_VIRTUAL_OVERRIDE { m_lines.FetchAdd( 1 ); }
    virtual void FlushLog( void ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual void SetMinimalLogLevel( const Int32 ) GUCEF_VIRTUAL_OVERRIDE {}
    virtual Int32 GetMinimalLogLevel( void ) const GUCEF_VIRTUAL_OVERRIDE { return CORE::LOGLEVEL_EVERYTHING; }

    private:

    MT::CTAtomic< UInt32 > m_lines;
};

/*-------------------------------------------------------------------------*/

/** Writes a value to a grid as it is, without a publisher: what a hostile or broken member could put there */
bool
WriteRaw( DRN::CDRNDataGrid& grid, const CORE::CVariant& key, const CORE::CVariantStream& stream )
{
    CORE::CVariant value;
    value.LinkTo( stream.GetBuffer().GetConstBufferPtr(), stream.GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
    return grid.SetValue( key, value );
}

/*-------------------------------------------------------------------------*/

/** A schema of the given number of INT8 columns, the first being the key */
CORE::CVariantStream*
WideSchema( UInt32 columns )
{
    DRN::CDRNPublishedTableSchema schema;
    for ( UInt32 c=0; c<columns; ++c )
        schema.columns.push_back( DRN::CDRNPublishedColumn( CORE::ToString( c ), CORE::CDataTableColumnDef( CORE::ToString( c ), CORE::DATATABLE_TYPE_INT8, 0 != c ) ) );
    schema.keyColumn = 0;
    CORE::CVariantStream* stream = GUCEF_NEW CORE::CVariantStream();
    stream->GetBuffer().SetBufferSize( columns * 128, false );     // grown once: the stream grows by what each value needs
    schema.Encode( *stream );
    return stream;
}

/*-------------------------------------------------------------------------*/

/** Pulses a pulse generator on a thread of its own, which is its driver thread, until it is stopped */
class CTablePulser : public MT::CActiveObject
{
    public:

    explicit CTablePulser( const CORE::PulseGeneratorPtr& pulseGenerator )
        : MT::CActiveObject(), m_pulseGenerator( pulseGenerator ), m_running( false ), m_stop( false )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_pulseGenerator->SetPulseDriverThreadId( MT::GetCurrentTaskID() );
        return true;
    }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_running.Store( true );
        while ( !m_stop.Load() )
        {
            m_pulseGenerator->ForceDirectPulse();
            MT::ThreadDelay( 1 );
        }
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    bool IsRunning( void ) const { return m_running.Load(); }
    void Stop( void ) { m_stop.Store( true ); }

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    MT::CTAtomic< bool > m_running;
    MT::CTAtomic< bool > m_stop;
};

/*-------------------------------------------------------------------------*/

/** Opens an update of a publisher on a thread of its own, sets a row in it and keeps it open for a while */
class CUpdateHolder : public MT::CActiveObject
{
    public:

    CUpdateHolder( DRN::CDRNTablePublisher& publisher, UInt32 holdMs )
        : MT::CActiveObject(), m_publisher( publisher ), m_holdMs( holdMs ), holding( false ), ending( false ), done( false ), succeeded( false )
    {}

    virtual bool OnThreadStart( void* ) GUCEF_VIRTUAL_OVERRIDE { return true; }

    virtual bool OnThreadCycle( void* ) GUCEF_VIRTUAL_OVERRIDE
    {
        bool ok = false;
        {
            DRN::CDRNTablePublisherUpdateScope update( m_publisher );
            ok = update.IsOpen() && m_publisher.SetRow( CORE::CVariant( (Int64) 100 ), PageCells( 100 ) );
            holding.Store( true );
            MT::ThreadDelay( m_holdMs );
            ending.Store( true );
            ok = update.EarlyEnd() && ok;
        }
        succeeded.Store( ok );
        done.Store( true );
        return true;
    }

    virtual const MT::CILockable* AsLockable( void ) const GUCEF_VIRTUAL_OVERRIDE { return GUCEF_NULL; }

    private:

    DRN::CDRNTablePublisher& m_publisher;
    UInt32 m_holdMs;

    public:

    MT::CTAtomic< bool > holding;
    MT::CTAtomic< bool > ending;            /**< set right before the update ends */
    MT::CTAtomic< bool > done;
    MT::CTAtomic< bool > succeeded;
};

/*-------------------------------------------------------------------------*/

/** Waits until the thread of the active object ended */
static void
WaitForActiveObject( MT::CActiveObject& activeObject )
{
    UInt32 waited = 0;
    while ( activeObject.IsActive() && waited++ < 10000 )
        MT::PrecisionDelay( 1 );
}

/*-------------------------------------------------------------------------*/

/** Records the longest time between two pulses of a pulse generator */
class CPulseGapRecorder : public CORE::CObserver
{
    public:

    MT::CTAtomic< UInt32 > pulses;
    MT::CTAtomic< UInt64 > lastPulseUs;
    MT::CTAtomic< UInt64 > maxGapUs;

    CPulseGapRecorder( void ) : CORE::CObserver(), pulses( 0 ), lastPulseUs( 0 ), maxGapUs( 0 ) {}
    virtual ~CPulseGapRecorder() { SignalUpcomingObserverDestruction(); }

    void Restart( void )
    {
        lastPulseUs.Store( DRN::CDRNClock::NowInMicroseconds() );
        maxGapUs.Store( 0 );
    }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CPulseGenerator::PulseEvent != eventid )
            return;
        pulses.FetchAdd( 1 );
        UInt64 now = DRN::CDRNClock::NowInMicroseconds();
        UInt64 last = lastPulseUs.Load();
        if ( 0 != last && now > last && now - last > maxGapUs.Load() )
            maxGapUs.Store( now - last );
        lastPulseUs.Store( now );
    }
};

/*-------------------------------------------------------------------------*/

/** Takes the lock it is given in every pulse it gets, as an application object that calls the node from a pulse */
class CPulseLockTaker : public CORE::CObserver
{
    public:

    MT::CTAtomic< bool > waiting;

    explicit CPulseLockTaker( MT::CMutex& lockToTake ) : CORE::CObserver(), waiting( false ), m_lockToTake( lockToTake ) {}
    virtual ~CPulseLockTaker() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CPulseGenerator::PulseEvent != eventid )
            return;
        waiting.Store( true );
        MT::CScopeMutex lock( m_lockToTake );
        waiting.Store( false );
    }

    private:

    MT::CMutex& m_lockToTake;
};

/*-------------------------------------------------------------------------*/

/**
 *  Calls PublishFrom() once from within a pulse of the node's own pulse generator, which the node pulses under the
 *  node lock outside any update of the publisher, once the pulse of the table that another thread pulses waits for
 *  that lock
 */
class CPublishFromNodePulse : public CORE::CObserver
{
    public:

    MT::CTAtomic< bool > done;
    MT::CTAtomic< bool > published;
    MT::CTAtomic< bool > tablePulseWaited;
    MT::CTAtomic< UInt64 > tookMs;

    CPublishFromNodePulse( DRN::CDRNTablePublisher& publisher, CORE::CIDataTable& table, CPulseLockTaker& taker )
        : CORE::CObserver(), done( false ), published( false ), tablePulseWaited( false ), tookMs( 0 ), m_publisher( publisher ), m_table( table ), m_taker( taker ), m_started( false )
    {}
    virtual ~CPublishFromNodePulse() { SignalUpcomingObserverDestruction(); }

    protected:

    virtual void OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( CORE::CPulseGenerator::PulseEvent != eventid || m_started )
            return;
        m_started = true;
        UInt32 waited = 0;
        while ( !m_taker.waiting.Load() && waited++ < 2000 )
            MT::ThreadDelay( 1 );
        tablePulseWaited.Store( m_taker.waiting.Load() );
        UInt64 start = NowMs();
        published.Store( m_publisher.PublishFrom( m_table, CORE::CDataTableQuery() ) );
        tookMs.Store( NowMs() - start );
        done.Store( true );
    }

    private:

    DRN::CDRNTablePublisher& m_publisher;
    CORE::CIDataTable& m_table;
    CPulseLockTaker& m_taker;
    bool m_started;
};

/*-------------------------------------------------------------------------*/

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformDRNTablePublisherTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING DRN TABLE PUBLISHER TESTS" );

    GUCEF_TESTFW_SUITE_SCOPE( "gucefDRN.TablePublisher" );

    GUCEF_TESTFW_TESTCASE( "Published tables: the schema arrives typed with the key column as primary key, titles, widths, hidden columns and hints" )
    try
    {
        CPageWorld world( "pub-schema" );
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 10 ) );
        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        ASSERT_TRUE( table.IsValid() && table.IsReadOnly() );
        DRN::CDRNPublishedTableSchema published;
        ASSERT_TRUE( table.GetPublishedSchema( published ) && published == PageSchema() );
        CORE::CDataTableSchemaPtr schema = table.GetSchema();
        REQUIRE_TRUE( !schema.IsNULL() && 8 == schema->GetColumnCount() );
        ASSERT_TRUE( schema->GetColumn( 0 ).name == "id" && schema->GetColumn( 1 ).name == "name" );
        ASSERT_TRUE( schema->GetColumn( 2 ).GetTypeText() == PageSchema().columns[ 2 ].def.GetTypeText() );
        ASSERT_TRUE( schema->GetColumn( 3 ).GetTypeText() == PageSchema().columns[ 3 ].def.GetTypeText() );
        CORE::CValueList hints;
        ASSERT_TRUE( table.GetColumnHints( 2, hints ) && hints.GetValue( "prefix" ).AsString() == "$" && hints.GetValue( "id" ).AsString() == "amount" );
        ASSERT_TRUE( table.GetColumnHints( 1, hints ) && hints.GetValue( "title" ).AsString() == "Name" && hints.GetValue( "width" ).AsInt32() == 120 );
        ASSERT_TRUE( table.GetColumnHints( 7, hints ) && hints.GetValue( "hidden" ).AsBool() );
        CORE::CDataTableDefinition plainDefinition;
        CORE::CMemoryDataTable plain( plainDefinition );
        ASSERT_FALSE( plain.GetColumnHints( 0, hints ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: values of every type arrive typed, nulls as nulls; a cell that does not fit is refused, or shown as null and counted" )
    try
    {
        CPageWorld world( "pub-rows" );
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 20 ) );

        // a name where an integer belongs: refused by the check
        CORE::CVariant::VariantVector wrong = PageCells( 30 );
        wrong[ 6 ] = CORE::CVariant( "not a number" );
        ASSERT_FALSE( world.publisher->SetRow( CORE::CVariant( (Int64) 30 ), wrong ) );
        ASSERT_TRUE( 1 == world.publisher->GetTypeMismatchCount() );

        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        TRowMap rows;
        REQUIRE_TRUE( ReadAll( table, rows ) );
        ASSERT_TRUE( 20 == rows.size() );
        const CORE::CVariant::VariantVector& row7 = rows[ CORE::CVariant( (Int64) 7 ) ];
        REQUIRE_TRUE( 7 == row7.size() );
        ASSERT_TRUE( row7[ 0 ].AsString() == "row 7" );
        CORE::CDecimal expected;
        CORE::CDecimal::FromUnscaled( 7250000, 6, expected );
        CORE::CDecimal arrivedAmount;
        ASSERT_TRUE( row7[ 1 ].IsDecimal() && !CORE::DecimalStatusIsError( row7[ 1 ].GetDecimal( arrivedAmount ) ) && arrivedAmount.ToString() == expected.ToString() );
        ASSERT_TRUE( row7[ 3 ].AsBool() == false && row7[ 4 ].AsFloat64() == 3.5 && row7[ 6 ].AsInt32() == 993 );
        const CORE::CVariant::VariantVector& row5 = rows[ CORE::CVariant( (Int64) 5 ) ];
        ASSERT_TRUE( !row5[ 2 ].IsInitialized() || row5[ 2 ].IsNULL() );
        ASSERT_TRUE( 0 == table.GetMismatchedValueCount() );

        // without the check the value goes out, the replica shows null and counts it
        DRN::CDRNTablePublisherSettings unchecked;
        unchecked.checkRows = false;
        CPageWorld forced( "pub-forced" );
        REQUIRE_TRUE( forced.Start( unchecked ) );
        {
            DRN::CDRNTablePublisherUpdateScope update( *forced.publisher );
            ASSERT_TRUE( forced.publisher->SetRow( CORE::CVariant( (Int64) 1 ), wrong ) );
        }
        forced.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* forcedReplica = forced.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != forcedReplica );
        DRN::CDRNPublishedTable forcedTable( *forcedReplica, DRN::CDRNPublishedTableSettings() );
        REQUIRE_TRUE( ReadAll( forcedTable, rows ) );
        REQUIRE_TRUE( 1 == rows.size() );
        const CORE::CVariant& forcedCell = rows.begin()->second[ 6 ];
        ASSERT_TRUE( !forcedCell.IsInitialized() || forcedCell.IsNULL() );
        ASSERT_TRUE( 1 == forcedTable.GetMismatchedValueCount() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: republishing sends no row, 3 changed of 1000 send 3 rows and #meta, a removed row goes; #meta counts and scalars" )
    try
    {
        CPageWorld world( "pub-diff" );
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 1000 ) );
        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        CTableRecorder recorder;
        recorder.SubscribeTo( &table );
        PumpFor( 50 );

        // the same rows again: nothing written, no revision
        UInt64 bytesBefore = world.grid->GetBytesSent();
        REQUIRE_TRUE( world.Publish( 1000 ) );
        PumpFor( 100 );
        ASSERT_TRUE( 1 == world.publisher->GetPageRevision() && bytesBefore == world.grid->GetBytesSent() && 0 == recorder.rowEvents );

        // 3 changed rows and a scalar
        {
            DRN::CDRNTablePublisherUpdateScope update( *world.publisher );
            for ( UInt32 n=0; n<1000; ++n )
                world.publisher->SetRow( CORE::CVariant( (Int64) n ), PageCells( n, n >= 500 && n < 503 ? CORE::CString( " changed" ) : CORE::CString::Empty ) );
            world.publisher->SetScalar( "total", CORE::CVariant( "1234.56" ) );
        }
        ASSERT_TRUE( 2 == world.publisher->GetPageRevision() );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 2 ) );
        PumpFor( 50 );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Published table diff: " << recorder.rowEvents << " row events, " << recorder.keys.size() << " keys" );
        ASSERT_TRUE( 1 == recorder.rowEvents && 3 == recorder.keys.size() );
        DRN::CDRNPublishedTableMeta meta;
        ASSERT_TRUE( table.GetMeta( meta ) && 2 == meta.pageRevision && 1000 == meta.rowCount && 3 == meta.changedRowCount );
        CORE::CVariant total;
        ASSERT_TRUE( table.GetScalar( "total", total ) && total.AsString() == "1234.56" );

        // a removed row disappears and is listed
        recorder.keys.clear();
        {
            DRN::CDRNTablePublisherUpdateScope update( *world.publisher );
            ASSERT_TRUE( world.publisher->RemoveRow( CORE::CVariant( (Int64) 42 ) ) );
        }
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 3 ) );
        PumpFor( 50 );
        TRowMap rows;
        REQUIRE_TRUE( ReadAll( table, rows ) );
        ASSERT_TRUE( 999 == rows.size() && rows.find( CORE::CVariant( (Int64) 42 ) ) == rows.end() );
        ASSERT_TRUE( 1 == recorder.keys.size() && recorder.keys[ 0 ] == CORE::CVariant( (Int64) 42 ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: an update over several ticks is inconsistent until whole, RowsChangedEvent comes once; maxDeferMs releases it" )
    try
    {
        CPageWorld world( "pub-ticks" );
        world.group.sessionSettings.maxBytesPerTickPerLink = 2000;
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 200 ) );
        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        CTableRecorder recorder;
        recorder.SubscribeTo( &table );

        REQUIRE_TRUE( world.Publish( 200, 0, 199 ) );
        bool sawInconsistent = false;
        UInt64 start = NowMs();
        while ( table.GetPageRevision() < 2 && NowMs() - start < 15000 )
        {
            PumpFor( 1 );
            if ( !table.IsConsistent() )
                sawInconsistent = true;
        }
        CPageArrived arrived( world.group[ 1 ], 2 );
        REQUIRE_TRUE( PumpUntil( arrived, 15000 ) );
        PumpFor( 50 );
        ASSERT_TRUE( sawInconsistent );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Published table over ticks: " << recorder.rowEvents << " row events, " << recorder.keys.size() << " keys, inconsistent seen " << sawInconsistent );
        ASSERT_TRUE( 1 == recorder.rowEvents && 200 == recorder.keys.size() );

        // without holding back: events as the rows come
        DRN::CDRNPublishedTableSettings eager;
        eager.deferUntilConsistent = false;
        DRN::CDRNPublishedTable eagerTable( *replica, eager );
        CTableRecorder eagerRecorder;
        eagerRecorder.SubscribeTo( &eagerTable );
        REQUIRE_TRUE( world.Publish( 200 ) );
        CPageArrived arrivedAgain( world.group[ 1 ], 3 );
        REQUIRE_TRUE( PumpUntil( arrivedAgain, 15000 ) );
        PumpFor( 50 );
        ASSERT_TRUE( eagerRecorder.rowEvents > 1 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: limits: an oversize row and reserved keys are refused and counted, a newer #schema is UNSUPPORTED; no observer means no subscription at the grid" )
    try
    {
        DRN::CDRNTablePublisherSettings small;
        small.maxValueBytes = 1200;
        CPageWorld world( "pub-limits" );
        REQUIRE_TRUE( world.Start( small ) );
        CORE::CVariant::VariantVector big = PageCells( 1 );
        CORE::CString longName;
        for ( UInt32 i=0; i<200; ++i )
            longName += "0123456789";
        big[ 0 ] = CORE::CVariant( longName );
        ASSERT_FALSE( world.publisher->SetRow( CORE::CVariant( (Int64) 1 ), big ) );
        ASSERT_TRUE( 1 == world.publisher->GetOversizeRowCount() );
        ASSERT_FALSE( world.publisher->SetRow( CORE::CVariant( "#mine" ), PageCells( 1 ) ) );
        ASSERT_TRUE( 1 == world.publisher->GetReservedKeyCount() );

        DRN::CDRNPublishedTable table( *world.grid, DRN::CDRNPublishedTableSettings() );
        ASSERT_TRUE( table.IsValid() && 0 == table.GetGridSubscriptionCount() );
        CTableRecorder recorder;
        recorder.SubscribeTo( &table );
        ASSERT_TRUE( 1 == table.GetGridSubscriptionCount() );
        recorder.UnsubscribeAllFromObserver();
        ASSERT_TRUE( 0 == table.GetGridSubscriptionCount() );

        // a #schema of a version this reader does not know
        CORE::CVariantStream newer;
        newer << (UInt8) ( DRN::CDRNPublishedTableSchema::EncodingVersion + 1 ) << (UInt32) 1;
        CORE::CVariant value;
        value.LinkTo( newer.GetBuffer().GetConstBufferPtr(), newer.GetDataSize(), GUCEF_DATATYPE_BINARY_BLOB );
        ASSERT_TRUE( world.grid->SetValue( CORE::CVariant( DRN::CDRNTablePublisher::SchemaKey ), value ) );
        ASSERT_TRUE( table.IsUnsupported() && !table.IsValid() );
        CORE::CDataTableScanPtr scan;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_UNSUPPORTED == table.Scan( CORE::CDataTableQuery(), scan ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: a late subscriber gets the whole table, consistent; publishing 10,000 rows of 20 columns with 1% changed sends only those rows" )
    try
    {
        CPageWorld world( "pub-late" );
        REQUIRE_TRUE( world.Start() );
        for ( UInt32 r=0; r<3; ++r )
            REQUIRE_TRUE( world.Publish( 300, r * 10, r * 10 + 4 ) );
        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 3 );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        TRowMap rows;
        REQUIRE_TRUE( ReadAll( table, rows ) );
        ASSERT_TRUE( 300 == rows.size() && table.IsConsistent() );
        ASSERT_TRUE( rows[ CORE::CVariant( (Int64) 22 ) ][ 0 ].AsString() == "row 22 changed" );

        // 10,000 rows of 20 columns, then 1 % changed
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "pub-perf", "mem" );
        REQUIRE_TRUE( group.Start( 2 ) );
        DRN::CDRNDataGridSettings gridSettings;
        gridSettings.name = "wide";
        gridSettings.reliable = true;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( gridSettings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        DRN::CDRNTablePublisher publisher( *grid, DRN::CDRNTablePublisherSettings() );
        DRN::CDRNPublishedTableSchema schema;
        schema.columns.push_back( DRN::CDRNPublishedColumn( "k", CORE::CDataTableColumnDef( "k", CORE::DATATABLE_TYPE_INT64, false ) ) );
        for ( UInt32 c=1; c<20; ++c )
            schema.columns.push_back( DRN::CDRNPublishedColumn( "c" + CORE::ToString( c ), CORE::CDataTableColumnDef( "c" + CORE::ToString( c ), CORE::DATATABLE_TYPE_INT64 ) ) );
        REQUIRE_TRUE( publisher.SetSchema( schema ) );
        CORE::CVariant::VariantVector cells( 19 );
        UInt64 publishUs[ 2 ] = { 0, 0 };
        UInt64 bytes[ 2 ] = { 0, 0 };
        for ( UInt32 round=0; round<2; ++round )
        {
            UInt64 bytesBefore = grid->GetBytesSent();
            UInt64 start = DRN::CDRNClock::NowInMicroseconds();
            {
                DRN::CDRNTablePublisherUpdateScope update( publisher );
                for ( UInt32 n=0; n<10000; ++n )
                {
                    for ( UInt32 c=0; c<19; ++c )
                        cells[ c ] = CORE::CVariant( (Int64) ( n * 100 + c + ( 1 == round && 0 == n % 100 ? 7 : 0 ) ) );
                    publisher.SetRow( CORE::CVariant( (Int64) n ), cells );
                }
            }
            publishUs[ round ] = DRN::CDRNClock::NowInMicroseconds() - start;
            if ( 0 == round )
                group[ 1 ].Subscribe( "wide" );
            UInt64 waitStart = NowMs();
            while ( NowMs() - waitStart < 30000 )
            {
                PumpFor( 10 );
                DRN::CDRNDataGrid* wide = group[ 1 ].FindGrid( "wide" );
                if ( GUCEF_NULL != wide && DRN::CDRNPublishedTable( *wide, DRN::CDRNPublishedTableSettings() ).GetPageRevision() == round + 1 )
                    break;
            }
            PumpFor( 100 );
            bytes[ round ] = grid->GetBytesSent() - bytesBefore;
        }
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Published table of 10,000 rows of 20 columns: publishing all " << publishUs[ 0 ] / 1000 << " ms, " << bytes[ 0 ] << " bytes; with 1% changed " <<
                   publishUs[ 1 ] / 1000 << " ms, " << bytes[ 1 ] << " bytes" );
        ASSERT_TRUE( bytes[ 1 ] > 0 && bytes[ 1 ] * 20 < bytes[ 0 ] );
        if ( !IsSanitizerBuild() )
            ASSERT_TRUE( publishUs[ 1 ] < 5000000 );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: PublishFrom() publishes the rows of a scanned table; rows gone from the table are removed, a projection without a column is refused" )
    try
    {
        CPageWorld world( "pub-from" );
        REQUIRE_TRUE( world.Start() );
        CORE::CDataTableDefinition definition;
        definition.name = "source";
        REQUIRE_TRUE( PageSchema().ToDataTableSchema( definition.schema ) );
        CORE::CMemoryDataTable source( definition );
        REQUIRE_TRUE( FillMemoryTable( source, 0, 30 ) );

        // outside an update PublishFrom() opens one of its own
        ASSERT_TRUE( world.publisher->PublishFrom( source, CORE::CDataTableQuery() ) );
        ASSERT_TRUE( 30 == world.publisher->GetRowCount() && 1 == world.publisher->GetPageRevision() );
        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != replica );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        TRowMap rows;
        ASSERT_TRUE( ReadAll( table, rows ) && 30 == rows.size() );
        ASSERT_TRUE( rows[ CORE::CVariant( (Int64) 7 ) ] == PageCells( 7 ) );

        // rows 10..29 only, five of them changed: the rest goes, inside the caller's update
        CORE::CMemoryDataTable smaller( definition );
        REQUIRE_TRUE( FillMemoryTable( smaller, 10, 5, " changed" ) );
        REQUIRE_TRUE( FillMemoryTable( smaller, 15, 15 ) );
        {
            DRN::CDRNTablePublisherUpdateScope update( *world.publisher );
            ASSERT_TRUE( world.publisher->PublishFrom( smaller, CORE::CDataTableQuery() ) );
        }
        ASSERT_TRUE( 20 == world.publisher->GetRowCount() && 2 == world.publisher->GetPageRevision() );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 2 ) );
        ASSERT_TRUE( ReadAll( table, rows ) && 20 == rows.size() );
        ASSERT_TRUE( rows.find( CORE::CVariant( (Int64) 3 ) ) == rows.end() );
        ASSERT_TRUE( rows[ CORE::CVariant( (Int64) 12 ) ] == PageCells( 12, " changed" ) );
        DRN::CDRNPublishedTableMeta published;
        ASSERT_TRUE( table.GetMeta( published ) && 15 == published.changedRowCount && 20 == published.rowCount );

        // a query that leaves out a published column cannot give the rows
        CORE::CDataTableQuery partial;
        partial.Select( "id" ).Select( "name" );
        ASSERT_FALSE( world.publisher->PublishFrom( smaller, partial ) );
        ASSERT_TRUE( 2 == world.publisher->GetPageRevision() );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: the CSV export of a published table opened through the drn database client (format = published) equals that of a memory table with the same rows (TPUB 8)" )
    try
    {
        CPageWorld world( "pub-csv" );
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 25 ) );
        world.group[ 1 ].Subscribe( "page" );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 1 ) );

        DRN::CDRNSessionRegistry& registry = DRN::CDRNGlobal::Instance()->GetSessionRegistry();
        REQUIRE_TRUE( registry.Register( world.group[ 1 ] ) );
        COMCORE::CIDatabaseClientPtr client = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "drn" );
        REQUIRE_TRUE( !client.IsNULL() );
        COMCORE::CDatabaseClientSettings settings;
        settings.backendName = "drn";
        settings.database = "pub-csv";
        settings.backendOptions.Set( "page.format", "published" );
        bool opened = client->Open( settings );
        registry.Unregister( world.group[ 1 ] );
        REQUIRE_TRUE( opened );

        CORE::CDataTableDefinition definition;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == client->GetTableDefinition( "page", definition ) );
        ASSERT_TRUE( 8 == definition.schema.GetColumnCount() && "id" == definition.schema.GetColumn( 0 ).name );
        CORE::CIDataTablePtr published;
        REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == client->OpenTable( "page", published ) && !published.IsNULL() );
        ASSERT_TRUE( published->IsReadOnly() );
        CORE::CDataTableSchemaPtr publishedSchema = published->GetSchema();
        REQUIRE_TRUE( !publishedSchema.IsNULL() && 8 == publishedSchema->GetColumnCount() );

        CORE::CDataTableDefinition memoryDefinition;
        memoryDefinition.name = "page";
        memoryDefinition.schema = *publishedSchema;
        CORE::CMemoryDataTable memory( memoryDefinition );
        REQUIRE_TRUE( FillMemoryTable( memory, 0, 25 ) );

        CORE::CDynamicBuffer publishedCsv;
        CORE::CDynamicBuffer memoryCsv;
        REQUIRE_TRUE( ExportCsv( *published, publishedCsv ) );
        REQUIRE_TRUE( ExportCsv( memory, memoryCsv ) );
        ASSERT_TRUE( publishedCsv.GetDataSize() > 25 * 20 );
        ASSERT_TRUE( publishedCsv.GetDataSize() == memoryCsv.GetDataSize() &&
                     0 == memcmp( publishedCsv.GetConstBufferPtr(), memoryCsv.GetConstBufferPtr(), publishedCsv.GetDataSize() ) );

        // without the option the same grid is the key/value table of the grid
        COMCORE::CIDatabaseClientPtr plainClient = COMCORE::CComCoreGlobal::Instance()->GetDatabaseClientFactory().Create( "drn" );
        REQUIRE_TRUE( registry.Register( world.group[ 1 ] ) );
        settings.backendOptions.Clear();
        opened = plainClient->Open( settings );
        registry.Unregister( world.group[ 1 ] );
        REQUIRE_TRUE( opened );
        CORE::CIDataTablePtr plain;
        REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == plainClient->OpenTable( "page", plain ) && !plain.IsNULL() );
        ASSERT_TRUE( DRN::CDRNDataGridTable::KeyColumnName == plain->GetSchema()->GetColumn( 0 ).name &&
                     DRN::CDRNDataGridTable::ValueColumnName == plain->GetSchema()->GetColumn( 1 ).name );
        plain.Unlink();
        published.Unlink();
        plainClient->Close();
        client->Close();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: a #schema of more columns than a published table takes is refused; rows that do not decode stop at their first missing value, are counted and logged once" )
    try
    {
        CGroup group( DRN::DRN_TOPOLOGY_CLIENT_SERVER, "pub-hostile", "mem" );
        REQUIRE_TRUE( group.Start( 1 ) );
        DRN::CDRNDataGridSettings gridSettings;
        gridSettings.name = "evil";
        gridSettings.reliable = true;
        DRN::CDRNDataGrid* grid = group[ 0 ].CreateGrid( gridSettings );
        REQUIRE_TRUE( GUCEF_NULL != grid );

        // 65536 columns: refused when read, the table has no schema
        CORE::CVariantStream* huge = WideSchema( 65536 );
        REQUIRE_TRUE( WriteRaw( *grid, CORE::CVariant( DRN::CDRNTablePublisher::SchemaKey ), *huge ) );
        GUCEF_DELETE huge;
        const UInt32 rowCount = 200;
        for ( UInt32 r=0; r<rowCount; ++r )
        {
            // version and revision only: no cells at all
            CORE::CVariantStream row;
            row << (UInt8) 1 << (UInt64) 1;
            REQUIRE_TRUE( WriteRaw( *grid, CORE::CVariant( (Int64) r ), row ) );
        }
        {
            DRN::CDRNPublishedTable table( *grid, DRN::CDRNPublishedTableSettings() );
            REQUIRE_TRUE( !table.IsValid() );
            CORE::CDataTableScanPtr scan;
            UInt64 start = NowMs();
            REQUIRE_TRUE( CORE::DATATABLE_STATUS_NOT_FOUND == table.Scan( CORE::CDataTableQuery(), scan ) );
            ASSERT_TRUE( NowMs() - start < 2000 );
        }
        ASSERT_TRUE( DRN::CDRNPublishedTableSchema::MaxColumnCount >= 256 );
        DRN::CDRNPublishedTableSchema tooWide;
        for ( UInt32 c=0; c<=DRN::CDRNPublishedTableSchema::MaxColumnCount; ++c )
            tooWide.columns.push_back( DRN::CDRNPublishedColumn( CORE::ToString( c ), CORE::CDataTableColumnDef( CORE::ToString( c ), CORE::DATATABLE_TYPE_INT8, 0 != c ) ) );
        CORE::CString error;
        ASSERT_FALSE( tooWide.IsValid( error ) );

        // the most columns a table takes: every row ends at its first value, one log line for all of them
        CORE::CVariantStream* widest = WideSchema( DRN::CDRNPublishedTableSchema::MaxColumnCount );
        REQUIRE_TRUE( WriteRaw( *grid, CORE::CVariant( DRN::CDRNTablePublisher::SchemaKey ), *widest ) );
        GUCEF_DELETE widest;
        {
            DRN::CDRNPublishedTable table( *grid, DRN::CDRNPublishedTableSettings() );
            REQUIRE_TRUE( table.IsValid() );
            CLogLineCounter logLines;
            UInt32 linesBefore = logLines.GetLineCount();
            CORE::CDataTableScanPtr scan;
            UInt64 start = NowMs();
            REQUIRE_TRUE( CORE::DATATABLE_STATUS_OK == table.Scan( CORE::CDataTableQuery(), scan ) );
            UInt64 took = NowMs() - start;
            CORE::CDataTableBatchPtr batch;
            UInt32 rows = 0;
            while ( CORE::DATATABLE_STATUS_OK == scan->NextBatch( batch ) && !batch.IsNULL() )
                rows += batch->GetRowCount();
            ASSERT_FALSE( table.IsConsistent() );
            UInt32 lines = logLines.GetLineCount() - linesBefore;
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Published tables: " << rowCount << " rows that do not decode against " << DRN::CDRNPublishedTableSchema::MaxColumnCount <<
                       " columns: scan " << took << " ms, " << lines << " log lines, " << table.GetUndecodableRowCount() << " counted" );
            ASSERT_TRUE( 0 == rows );
            ASSERT_TRUE( rowCount == table.GetUndecodableRowCount() );
            ASSERT_TRUE( lines <= 2 );
        }
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: the rows are decoded without the node lock; a node on a thread of its own keeps pulsing while a scan decodes" )
    try
    {
        CPageWorld world( "pub-unlocked" );
        world.group.nodeSettings.ownThread = true;
        REQUIRE_TRUE( world.group.Start( 1 ) );
        DRN::CDRNDataGridSettings gridSettings;
        gridSettings.name = "big";
        gridSettings.reliable = true;
        DRN::CDRNDataGrid* grid = world.group[ 0 ].CreateGrid( gridSettings );
        REQUIRE_TRUE( GUCEF_NULL != grid );
        DRN::CDRNTablePublisher publisher( *grid, DRN::CDRNTablePublisherSettings() );
        DRN::CDRNPublishedTableSchema schema;
        schema.columns.push_back( DRN::CDRNPublishedColumn( "k", CORE::CDataTableColumnDef( "k", CORE::DATATABLE_TYPE_INT64, false ) ) );
        for ( UInt32 c=1; c<20; ++c )
            schema.columns.push_back( DRN::CDRNPublishedColumn( "c" + CORE::ToString( c ), CORE::CDataTableColumnDef( "c" + CORE::ToString( c ), CORE::DATATABLE_TYPE_INT64 ) ) );
        REQUIRE_TRUE( publisher.SetSchema( schema ) );
        CORE::CVariant::VariantVector cells( 19 );
        {
            DRN::CDRNTablePublisherUpdateScope update( publisher );
            for ( UInt32 n=0; n<20000; ++n )
            {
                for ( UInt32 c=0; c<19; ++c )
                    cells[ c ] = CORE::CVariant( (Int64) ( n * 100 + c ) );
                publisher.SetRow( CORE::CVariant( (Int64) n ), cells );
            }
        }

        CPulseGapRecorder recorder;
        recorder.SubscribeTo( world.group.nodes[ 0 ]->GetPulseGenerator().GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        UInt64 start = NowMs();
        while ( recorder.pulses.Load() < 20 && NowMs() - start < 2000 )
            MT::ThreadDelay( 5 );
        REQUIRE_TRUE( recorder.pulses.Load() >= 20 );

        DRN::CDRNPublishedTableSettings settings;
        settings.scanChunkKeyCount = 256;
        DRN::CDRNPublishedTable table( *grid, settings );
        REQUIRE_TRUE( table.IsValid() );
        recorder.Restart();
        UInt64 decodeStart = DRN::CDRNClock::NowInMicroseconds();
        ASSERT_TRUE( table.IsConsistent() );         // decodes every row
        UInt64 decodeUs = DRN::CDRNClock::NowInMicroseconds() - decodeStart;
        UInt64 maxGapUs = recorder.maxGapUs.Load();
        recorder.UnsubscribeAllFromObserver();
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Published tables: decoding 20,000 rows of 20 columns took " << decodeUs / 1000 << " ms, the longest pause of the node's pulses " << maxGapUs / 1000 << " ms" );
        ASSERT_TRUE( 20000 == table.GetCachedRowCount() );
        ASSERT_TRUE( maxGapUs * 4 < decodeUs );
        CORE::CDataTableScanPtr scan;
        ASSERT_TRUE( CORE::DATATABLE_STATUS_OK == table.Scan( CORE::CDataTableQuery(), scan ) );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: a scan reuses the copy of the table for snapshotCacheMs; the decoded rows are dropped after decodeCacheIdleMs without use" )
    try
    {
        CPageWorld world( "pub-cache" );
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 20 ) );

        DRN::CDRNPublishedTableSettings cachedSettings;
        cachedSettings.snapshotCacheMs = 60000;
        DRN::CDRNPublishedTable cached( *world.grid, cachedSettings );
        DRN::CDRNPublishedTable fresh( *world.grid, DRN::CDRNPublishedTableSettings() );
        TRowMap rows;
        REQUIRE_TRUE( ReadAll( cached, rows ) && 20 == rows.size() );
        REQUIRE_TRUE( ReadAll( fresh, rows ) && 20 == rows.size() );
        REQUIRE_TRUE( world.Publish( 20, 3, 3 ) );
        ASSERT_TRUE( ReadAll( cached, rows ) && rows[ CORE::CVariant( (Int64) 3 ) ] == PageCells( 3 ) );
        ASSERT_TRUE( ReadAll( fresh, rows ) && rows[ CORE::CVariant( (Int64) 3 ) ] == PageCells( 3, " changed" ) );

        // the decoded rows go when nobody used them for decodeCacheIdleMs
        DRN::CDRNPublishedTableSettings idleSettings;
        idleSettings.decodeCacheIdleMs = 100;
        DRN::CDRNPublishedTable idle( *world.grid, idleSettings );
        REQUIRE_TRUE( ReadAll( idle, rows ) && 20 == rows.size() );
        ASSERT_TRUE( 20 == idle.GetCachedRowCount() );
        UInt64 start = NowMs();
        while ( 0 != idle.GetCachedRowCount() && NowMs() - start < 3000 )
            PumpFor( 10 );
        ASSERT_TRUE( 0 == idle.GetCachedRowCount() );
        ASSERT_TRUE( ReadAll( idle, rows ) && 20 == rows.size() && rows[ CORE::CVariant( (Int64) 3 ) ] == PageCells( 3, " changed" ) );

        // with the default the decoded rows stay
        ASSERT_TRUE( 20 == fresh.GetCachedRowCount() );
        DRN::CDRNPublishedTableSettings defaults;
        ASSERT_TRUE( 0 == defaults.snapshotCacheMs && 0 != defaults.decodeCacheIdleMs );
        CORE::CDataNode node( "DRNPublishedTableSettings" );
        ASSERT_TRUE( idleSettings.SaveConfig( node ) );
        DRN::CDRNPublishedTableSettings loaded;
        ASSERT_TRUE( loaded.LoadConfig( node ) && 100 == loaded.decodeCacheIdleMs );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: PublishFrom() inside an update refuses a table whose pulse generator another thread pulses; outside the update it publishes it" )
    try
    {
        CPageWorld world( "pub-from-thread" );
        REQUIRE_TRUE( world.Start() );
        CORE::CDataTableDefinition definition;
        definition.name = "source";
        REQUIRE_TRUE( PageSchema().ToDataTableSchema( definition.schema ) );
        CORE::PulseGeneratorPtr tablePulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        CORE::CMemoryDataTable source( definition, GUCEF_NULL, tablePulseGenerator );
        REQUIRE_TRUE( FillMemoryTable( source, 0, 30 ) );
        CTablePulser pulser( tablePulseGenerator );
        REQUIRE_TRUE( pulser.Activate( GUCEF_NULL, 0, 0 ) );
        UInt32 waited = 0;
        while ( !pulser.IsRunning() && waited++ < 5000 )
            MT::ThreadDelay( 1 );
        REQUIRE_TRUE( pulser.IsRunning() );

        // the update holds the node lock; a scan of that table subscribes to the pulse generator of the other thread
        {
            DRN::CDRNTablePublisherUpdateScope update( *world.publisher );
            REQUIRE_TRUE( update.IsOpen() );
            ASSERT_FALSE( world.publisher->PublishFrom( source, CORE::CDataTableQuery() ) );
        }
        ASSERT_TRUE( 0 == world.publisher->GetRowCount() );

        // outside the update the rows are read first, then an update of its own takes them
        ASSERT_TRUE( world.publisher->PublishFrom( source, CORE::CDataTableQuery() ) );
        ASSERT_TRUE( 30 == world.publisher->GetRowCount() );

        pulser.Stop();
        WaitForActiveObject( pulser );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: PublishFrom() from within a pulse of the node, which holds the node lock outside an update, refuses a table whose pulse generator another thread pulses: no deadlock" )
    try
    {
        CPageWorld world( "pub-from-node-pulse" );
        world.group.nodeSettings.ownThread = true;
        REQUIRE_TRUE( world.Start() );
        CORE::CDataTableDefinition definition;
        definition.name = "source";
        REQUIRE_TRUE( PageSchema().ToDataTableSchema( definition.schema ) );
        CORE::PulseGeneratorPtr tablePulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        CORE::CMemoryDataTable source( definition, GUCEF_NULL, tablePulseGenerator );
        REQUIRE_TRUE( FillMemoryTable( source, 0, 30 ) );

        // the pulses of the table take the node lock, as an object of the application that calls the node from them
        CPulseLockTaker taker( world.group.nodes[ 0 ]->GetLock() );
        taker.SubscribeTo( tablePulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        CTablePulser pulser( tablePulseGenerator );
        REQUIRE_TRUE( pulser.Activate( GUCEF_NULL, 0, 0 ) );
        UInt32 waited = 0;
        while ( !pulser.IsRunning() && waited++ < 5000 )
            MT::ThreadDelay( 1 );
        REQUIRE_TRUE( pulser.IsRunning() );

        CPublishFromNodePulse caller( *world.publisher, source, taker );
        caller.SubscribeTo( world.group.nodes[ 0 ]->GetPulseGenerator().GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        UInt64 start = NowMs();
        while ( !caller.done.Load() && NowMs() - start < 120000 )
            MT::ThreadDelay( 5 );
        caller.UnsubscribeAllFromObserver();
        REQUIRE_TRUE( caller.done.Load() );
        ASSERT_TRUE( caller.tablePulseWaited.Load() );
        ASSERT_TRUE( caller.tookMs.Load() < 5000 );
        ASSERT_FALSE( caller.published.Load() );
        ASSERT_TRUE( 0 == world.publisher->GetRowCount() );

        // a thread that holds no lock publishes it
        ASSERT_TRUE( world.publisher->PublishFrom( source, CORE::CDataTableQuery() ) );
        ASSERT_TRUE( 30 == world.publisher->GetRowCount() );

        pulser.Stop();
        WaitForActiveObject( pulser );
        taker.UnsubscribeAllFromObserver();
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: an update that another thread has open does not count as this thread's: PublishFrom() reads its rows, waits for that update and publishes" )
    try
    {
        CPageWorld world( "pub-from-other-update" );
        REQUIRE_TRUE( world.Start() );
        CORE::CDataTableDefinition definition;
        definition.name = "source";
        REQUIRE_TRUE( PageSchema().ToDataTableSchema( definition.schema ) );
        CORE::PulseGeneratorPtr tablePulseGenerator = CORE::CPulseGenerator::CreateSharedObj();
        CORE::CMemoryDataTable source( definition, GUCEF_NULL, tablePulseGenerator );
        REQUIRE_TRUE( FillMemoryTable( source, 0, 30 ) );
        CTablePulser pulser( tablePulseGenerator );
        REQUIRE_TRUE( pulser.Activate( GUCEF_NULL, 0, 0 ) );

        CUpdateHolder holder( *world.publisher, 300 );
        REQUIRE_TRUE( holder.Activate( GUCEF_NULL, 0, 0 ) );
        UInt32 waited = 0;
        while ( ( !holder.holding.Load() || !pulser.IsRunning() ) && waited++ < 5000 )
            MT::ThreadDelay( 1 );
        REQUIRE_TRUE( holder.holding.Load() && pulser.IsRunning() );

        // this thread is in no update: the other thread's update ends first, then this one publishes the whole table
        const bool published = world.publisher->PublishFrom( source, CORE::CDataTableQuery() );
        const bool holderEnded = holder.ending.Load();
        WaitForActiveObject( holder );
        ASSERT_TRUE( published );
        ASSERT_TRUE( holderEnded && holder.done.Load() && holder.succeeded.Load() );
        ASSERT_TRUE( 30 == world.publisher->GetRowCount() );

        pulser.Stop();
        WaitForActiveObject( pulser );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Published tables: SetRow, RemoveRow and PublishRows outside an update are an update of their own; BeginUpdate keeps what is not published yet" )
    try
    {
        CPageWorld world( "pub-outside" );
        REQUIRE_TRUE( world.Start() );
        REQUIRE_TRUE( world.Publish( 3 ) );
        world.group[ 1 ].Subscribe( "page" );
        DRN::CDRNDataGrid* replica = world.Replica( 1 );
        REQUIRE_TRUE( GUCEF_NULL != replica );

        // a row outside an update: a revision of its own, the replica stays consistent through the next update
        ASSERT_TRUE( world.publisher->SetRow( CORE::CVariant( (Int64) 0 ), PageCells( 0, " outside" ) ) );
        ASSERT_TRUE( 2 == world.publisher->GetPageRevision() );
        {
            DRN::CDRNTablePublisherUpdateScope update( *world.publisher );
            ASSERT_TRUE( world.publisher->SetRow( CORE::CVariant( (Int64) 1 ), PageCells( 1, " inside" ) ) );
        }
        ASSERT_TRUE( 3 == world.publisher->GetPageRevision() );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 3 ) );
        DRN::CDRNPublishedTable table( *replica, DRN::CDRNPublishedTableSettings() );
        TRowMap rows;
        ASSERT_TRUE( ReadAll( table, rows ) && 3 == rows.size() );
        ASSERT_TRUE( rows[ CORE::CVariant( (Int64) 0 ) ][ 0 ].AsString() == "row 0 outside" );
        ASSERT_TRUE( rows[ CORE::CVariant( (Int64) 1 ) ][ 0 ].AsString() == "row 1 inside" );

        // a removal and a whole row set outside an update
        ASSERT_TRUE( world.publisher->RemoveRow( CORE::CVariant( (Int64) 2 ) ) );
        ASSERT_TRUE( 4 == world.publisher->GetPageRevision() );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 4 ) );
        DRN::CDRNTablePublisher::TKeyedRows set;
        for ( Int64 n=5; n<8; ++n )
            set.push_back( DRN::CDRNTablePublisher::TKeyedRow( CORE::CVariant( n ), PageCells( n ) ) );
        ASSERT_TRUE( world.publisher->PublishRows( set ) );
        ASSERT_TRUE( 5 == world.publisher->GetPageRevision() );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 5 ) );
        ASSERT_TRUE( ReadAll( table, rows ) && 3 == rows.size() && rows.find( CORE::CVariant( (Int64) 5 ) ) != rows.end() );

        // a scalar set between updates goes with the next update, even one without row changes
        world.publisher->SetScalar( "note", CORE::CVariant( "between" ) );
        {
            DRN::CDRNTablePublisherUpdateScope update( *world.publisher );
        }
        ASSERT_TRUE( 6 == world.publisher->GetPageRevision() );
        REQUIRE_TRUE( GUCEF_NULL != world.Replica( 6 ) );
        CORE::CVariant note;
        ASSERT_TRUE( table.GetScalar( "note", note ) && note.AsString() == "between" );
    }
    catch ( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
