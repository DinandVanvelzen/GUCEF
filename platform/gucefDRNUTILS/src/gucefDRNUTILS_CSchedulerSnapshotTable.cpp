/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_TASKSTATUS_H
#include "gucefCORE_TaskStatus.h"
#define GUCEF_CORE_TASKSTATUS_H
#endif /* GUCEF_CORE_TASKSTATUS_H ? */

#ifndef GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H
#include "gucefDRNUTILS_CJobsTableSchema.h"
#define GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CLANESTABLESCHEMA_H
#include "gucefDRNUTILS_CLanesTableSchema.h"
#define GUCEF_DRNUTILS_CLANESTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CLANESTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CRUNSTABLESCHEMA_H
#include "gucefDRNUTILS_CRunsTableSchema.h"
#define GUCEF_DRNUTILS_CRUNSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CRUNSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#include "gucefDRNUTILS_CSchedulerRowCodec.h"
#define GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERROWCODEC_H ? */

#include "gucefDRNUTILS_CSchedulerSnapshotTable.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSchedulerSnapshotTable::CSchedulerSnapshotTable( CISchedulerSource& source, ETable table, UInt32 publishedRuns, const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CIDataTable( pulseGenerator )
    , m_source( &source )
    , m_table( table )
    , m_publishedRuns( publishedRuns )
    , m_definition()
    , m_schema()
    , m_rows( GUCEF_NULL )
    , m_filledRevision( 0 )
    , m_filledInstanceId( 0 )
    , m_stale( true )
    , m_following( false )
    , m_fillCount( 0 )
    , m_revisions()
    , m_lastError()
{GUCEF_TRACE;

    switch ( m_table )
    {
        case TABLE_JOBS:
            m_definition.name = CJobsTableSchema::TableName;
            CJobsTableSchema::GetSchema( m_definition.schema );
            break;
        case TABLE_LANES:
            m_definition.name = CLanesTableSchema::TableName;
            CLanesTableSchema::GetSchema( m_definition.schema );
            break;
        default:
            m_definition.name = CRunsTableSchema::TableName;
            CRunsTableSchema::GetSchema( m_definition.schema );
            break;
    }
    m_definition.codecType = "memory";
    m_schema = m_definition.CreateSchemaPtr();
}

/*-------------------------------------------------------------------------*/

CSchedulerSnapshotTable::~CSchedulerSnapshotTable()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    UnsubscribeAllFromObserver();
    GUCEF_DELETE m_rows;
    m_rows = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerSnapshotTable::GetName( void ) const
{GUCEF_TRACE;

    return m_definition.name;
}

/*-------------------------------------------------------------------------*/

const CORE::CDataTableDefinition&
CSchedulerSnapshotTable::GetDefinition( void ) const
{GUCEF_TRACE;

    return m_definition;
}

/*-------------------------------------------------------------------------*/

CORE::CDataTableSchemaPtr
CSchedulerSnapshotTable::GetSchema( void ) const
{GUCEF_TRACE;

    return m_schema;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerSnapshotTable::IsReadOnly( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerSnapshotTable::TakeRows( const CORE::CJobSchedulerSnapshot& snapshot, GUCEF::vector< CORE::CVariant::VariantVector >& rows, TRevisionMap& revisions ) const
{GUCEF_TRACE;

    // the same rows the publisher writes: hidden jobs and their runs are left out, sensitive texts redacted
    CORE::CString::StringSet hidden;
    CORE::CString::StringSet sensitive;
    const CORE::TJobStatusVector& jobs = snapshot.GetJobs();
    for ( UInt32 i=0; i<jobs.size(); ++i )
    {
        if ( 0 != ( jobs[ i ].flags & CORE::JOBFLAG_HIDDEN ) )
            hidden.insert( jobs[ i ].jobId );
        else
        if ( 0 != ( jobs[ i ].flags & CORE::JOBFLAG_SENSITIVE ) )
            sensitive.insert( jobs[ i ].jobId );
    }
    CORE::CVariant::VariantVector cells;
    if ( TABLE_JOBS == m_table )
    {
        for ( UInt32 i=0; i<jobs.size(); ++i )
        {
            if ( hidden.find( jobs[ i ].jobId ) != hidden.end() )
                continue;
            CSchedulerRowCodec::JobToCells( jobs[ i ], cells );
            CORE::CVariant::VariantVector row;
            row.push_back( CORE::CVariant( jobs[ i ].jobId ) );
            row.insert( row.end(), cells.begin(), cells.end() );
            rows.push_back( row );
            revisions[ CORE::CVariant( jobs[ i ].jobId ) ] = jobs[ i ].revision;
        }
    }
    else
    if ( TABLE_LANES == m_table )
    {
        const CORE::TJobLaneStatusVector& lanes = snapshot.GetLanes();
        for ( UInt32 i=0; i<lanes.size(); ++i )
        {
            CSchedulerRowCodec::LaneToCells( lanes[ i ], cells );
            CORE::CVariant::VariantVector row;
            row.push_back( CORE::CVariant( lanes[ i ].laneName ) );
            row.insert( row.end(), cells.begin(), cells.end() );
            rows.push_back( row );
            revisions[ CORE::CVariant( lanes[ i ].laneName ) ] = lanes[ i ].revision;
        }
    }
    else
    {
        const CORE::TJobRunRecordVector& runs = snapshot.GetRuns();
        UInt32 endedRuns = 0;
        for ( UInt32 i=0; i<runs.size(); ++i )
        {
            if ( hidden.find( runs[ i ].jobId ) != hidden.end() )
                continue;
            if ( CORE::JobRunStateIsAnEndState( runs[ i ].state ) )
            {
                if ( endedRuns >= m_publishedRuns )
                    continue;
                ++endedRuns;
            }
            CORE::SJobRunRecord run = runs[ i ];
            run.itemKey.Clear();
            if ( sensitive.find( run.jobId ) != sensitive.end() )
            {
                if ( !run.note.IsNULLOrEmpty() )
                    run.note = CSchedulerRowCodec::RedactedText;
                if ( !run.outcomeMessage.IsNULLOrEmpty() )
                    run.outcomeMessage = CSchedulerRowCodec::RedactedText;
            }
            CSchedulerRowCodec::RunToCells( run, cells );
            CORE::CVariant::VariantVector row;
            row.push_back( CORE::CVariant( run.runId ) );
            row.insert( row.end(), cells.begin(), cells.end() );
            rows.push_back( row );
            revisions[ CORE::CVariant( run.runId ) ] = run.revision;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSchedulerSnapshotTable::Fill( bool force )
{GUCEF_TRACE;

    if ( !force && !m_stale && GUCEF_NULL != m_rows )
        return;
    CORE::CJobSchedulerSnapshot snapshot;
    m_source->GetSnapshot( snapshot, 0 );
    if ( !force && GUCEF_NULL != m_rows && snapshot.GetRevision() == m_filledRevision && snapshot.GetHostInstanceId() == m_filledInstanceId )
    {
        m_stale = false;
        return;
    }
    GUCEF::vector< CORE::CVariant::VariantVector > rows;
    TRevisionMap revisions;
    TakeRows( snapshot, rows, revisions );

    CORE::CMemoryDataTable* table = GUCEF_NEW CORE::CMemoryDataTable( m_definition, GUCEF_NULL, GetPulseGenerator() );
    if ( !rows.empty() )
    {
        CORE::CDataTableBatchBuilder builder( m_schema, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );
        for ( UInt32 r=0; r<rows.size(); ++r )
            builder.AppendRow( &rows[ r ][ 0 ], (UInt32) rows[ r ].size() );
        CORE::CDataTableBatchPtr batch;
        CORE::CString error;
        CORE::CDataTableWriteResult result;
        if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) || batch.IsNULL() ||
             CORE::DATATABLE_STATUS_OK != table->Write( *batch, CORE::DATATABLE_WRITE_INSERT, result ) )
        {
            m_lastError = "the rows of " + m_definition.name + " could not be stored: " + error + " " + table->GetLastErrorMessage();
        }
    }
    // scans handed out earlier keep their batches: the table they came from may go
    GUCEF_DELETE m_rows;
    m_rows = table;
    m_filledRevision = snapshot.GetRevision();
    m_filledInstanceId = snapshot.GetHostInstanceId();
    m_revisions = revisions;
    m_stale = false;
    ++m_fillCount;
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CSchedulerSnapshotTable::Scan( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan )
{GUCEF_TRACE;

    Fill( false );
    return m_rows->Scan( query, scan );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CSchedulerSnapshotTable::ScanAsync( const CORE::CDataTableQuery& query, CORE::CDataTableScanPtr& scan, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    Fill( false );
    return m_rows->ScanAsync( query, scan, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CSchedulerSnapshotTable::Write( const CORE::CDataTableBatch& rows, CORE::TDataTableWriteMode mode, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::DATATABLE_STATUS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CSchedulerSnapshotTable::WriteAsync( const CORE::CDataTableBatchPtr& rows, CORE::TDataTableWriteMode mode, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CSchedulerSnapshotTable::DeleteRows( const CORE::CDataTableQuery& predicates, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::DATATABLE_STATUS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CSchedulerSnapshotTable::DeleteRowsAsync( const CORE::CDataTableQuery& predicates, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CSchedulerSnapshotTable::DeletePartitions( const CORE::CDataTableQuery& partitionFilter, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::DATATABLE_STATUS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CSchedulerSnapshotTable::DeletePartitionsAsync( const CORE::CDataTableQuery& partitionFilter, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CSchedulerSnapshotTable::ListPartitions( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::CDataTablePartitionInfo::TPartitionInfoVector& partitions )
{GUCEF_TRACE;

    Fill( false );
    return m_rows->ListPartitions( partitionFilter, withFileStats, partitions );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CSchedulerSnapshotTable::ListPartitionsAsync( const CORE::CDataTableQuery& partitionFilter, bool withFileStats, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    Fill( false );
    return m_rows->ListPartitionsAsync( partitionFilter, withFileStats, threadPool );
}

/*-------------------------------------------------------------------------*/

CORE::TDataTableStatus
CSchedulerSnapshotTable::Compact( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::CDataTableWriteResult& result )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::DATATABLE_STATUS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CSchedulerSnapshotTable::CompactAsync( const CORE::CDataTableQuery& partitionFilter, bool force, CORE::ThreadPoolPtr threadPool )
{GUCEF_TRACE;

    m_lastError = "the scheduler tables are read only";
    return CORE::CFutureResult( CORE::TASKSTATUS_SETUP_FAILED );
}

/*-------------------------------------------------------------------------*/

void
CSchedulerSnapshotTable::Refresh( void )
{GUCEF_TRACE;

    Fill( true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSchedulerSnapshotTable::GetLastErrorMessage( void ) const
{GUCEF_TRACE;

    return m_lastError;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerSnapshotTable::FollowSource( void )
{GUCEF_TRACE;

    if ( m_following || GUCEF_NULL == m_source->GetChangeNotifier() )
        return;
    m_following = true;
    // the rows as they are now are the base the changes are compared with
    Fill( true );
    TEventCallback changed( this, &CSchedulerSnapshotTable::OnSourceChanged );
    SubscribeTo( m_source->GetChangeNotifier(), CISchedulerSource::ChangedEvent, changed );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerSnapshotTable::Subscribe( CORE::CObserver* observer )
{GUCEF_TRACE;

    bool subscribed = CORE::CIDataTable::Subscribe( observer );
    if ( subscribed )
        FollowSource();
    return subscribed;
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerSnapshotTable::Subscribe( CORE::CObserver* observer, const CORE::CEvent& eventid, CORE::CIEventHandlerFunctorBase* callback )
{GUCEF_TRACE;

    bool subscribed = CORE::CIDataTable::Subscribe( observer, eventid, callback );
    if ( subscribed && RowsChangedEvent == eventid )
        FollowSource();
    return subscribed;
}

/*-------------------------------------------------------------------------*/

void
CSchedulerSnapshotTable::OnSourceChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TRevisionMap before = m_revisions;
    Fill( true );
    GUCEF::vector< CORE::CVariant > keys;
    TRevisionMap::const_iterator i = m_revisions.begin();
    while ( i != m_revisions.end() )
    {
        TRevisionMap::iterator found = before.find( (*i).first );
        if ( found == before.end() || (*found).second != (*i).second )
            keys.push_back( (*i).first );
        if ( found != before.end() )
            before.erase( found );
        ++i;
    }
    for ( i = before.begin(); i != before.end(); ++i )
        keys.push_back( (*i).first );
    if ( keys.empty() )
        return;
    CORE::TDataTableRowsChangedEventData data( keys );
    NotifyObservers( RowsChangedEvent, &data );
}

/*-------------------------------------------------------------------------*/

bool
CSchedulerSnapshotTable::IsFollowingSource( void ) const
{GUCEF_TRACE;

    return m_following;
}

/*-------------------------------------------------------------------------*/

UInt32
CSchedulerSnapshotTable::GetFillCount( void ) const
{GUCEF_TRACE;

    return m_fillCount;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSchedulerSnapshotTable::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRNUTILS::CSchedulerSnapshotTable";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
