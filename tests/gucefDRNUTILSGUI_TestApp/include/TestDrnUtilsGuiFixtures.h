/*
 *  gucefDRNUTILSGUI_TestApp: GUCEF test application for the gucefDRNUTILSGUI module
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

#ifndef GUCEF_DRNUTILSGUI_TESTAPP_TESTFIXTURES_H
#define GUCEF_DRNUTILSGUI_TESTAPP_TESTFIXTURES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                    //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CMEMORYDATATABLE_H
#include "gucefCORE_CMemoryDataTable.h"
#define GUCEF_CORE_CMEMORYDATATABLE_H
#endif /* GUCEF_CORE_CMEMORYDATATABLE_H ? */

#include "gucefDRNUTILSGUI_TestCommon.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A row of a test table: the cells by column name, texts and numbers (numbers also for the timestamps, in milliseconds); other cells are null */
class CTestRow
{
    public:

    GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CString > text;
    GUCEF::map< GUCEF::CORE::CString, GUCEF::Int64 > number;

    CTestRow& Text( const char* column, const GUCEF::CORE::CString& value ) { text[ column ] = value; return *this; }
    CTestRow& Num( const char* column, GUCEF::Int64 value ) { number[ column ] = value; return *this; }
};

typedef GUCEF::vector< CTestRow > TTestRowVector;

/**
 *  A table in memory with the schema of one of the tables of gucefDRNUTILS that can tell its observers that rows changed,
 *  as a table that follows a replicated source does, and that has presentation hints (as the tables the DRN publisher
 *  gives have)
 */
class CTestTable : public GUCEF::CORE::CMemoryDataTable
{
    public:

    CTestTable( const GUCEF::CORE::CDataTableDefinition& definition, GUCEF::CORE::CDataTableBufferPool* pool );

    void RaiseRowsChanged( void );

    /** Sets a presentation hint of a column: "title", "hidden", "width", ... */
    void SetHint( const GUCEF::CORE::CString& column, const GUCEF::CORE::CString& hint, const GUCEF::CORE::CVariant& value );

    virtual bool GetColumnHints( GUCEF::UInt32 columnIndex, GUCEF::CORE::CValueList& hints ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< GUCEF::CORE::CString, GUCEF::map< GUCEF::CORE::CString, GUCEF::CORE::CVariant > > THintMap;
    THintMap m_hints;
};

/** Which table of gucefDRNUTILS */
enum ETestTableKind
{
    TESTTABLE_JOBS = 0, TESTTABLE_LANES, TESTTABLE_RUNS, TESTTABLE_POOLS, TESTTABLE_THREADS, TESTTABLE_TASKS, TESTTABLE_SCHEDULEDJOBS
};

/** A table with the schema of the DRNUTILS table; 'table' holds it, the result points into it. NULL when it could not be made */
CTestTable* MakeTestTable( ETestTableKind kind, GUCEF::CORE::CIDataTablePtr& table );

/** Inserts or replaces rows by their primary key */
bool UpsertTestRows( CTestTable& table, const TTestRowVector& rows );

/** Removes the rows whose text in the column is the given one */
bool DeleteTestRows( CTestTable& table, const GUCEF::CORE::CString& column, const GUCEF::CORE::CString& text );

/*-------------------------------------------------------------------------*/

/** A scheduler client that records what the forms ask instead of calling a node; answers are given by the test */
class CFakeSchedulerClient : public GUCEF::DRNUTILS::CSchedulerClient
{
    public:

    GUCEF::vector< GUCEF::CORE::SJobControlCommand > commands;
    GUCEF::CORE::CString lastCommandId;
    GUCEF::UInt32 infoRequests;
    bool refuseToSend;
    GUCEF::CORE::CString::StringVector detailJobs;          /**< the jobs RequestJobDetail() was called for, in order */
    GUCEF::CORE::CString lastDetailRequestId;

    CFakeSchedulerClient( void )
        : GUCEF::DRNUTILS::CSchedulerClient( GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
        , commands(), lastCommandId(), infoRequests( 0 ), refuseToSend( false ), detailJobs(), lastDetailRequestId(), m_nextId( 1 )
    {
    }

    virtual GUCEF::CORE::CString RequestJobDetail( const GUCEF::CORE::CString& jobId, GUCEF::UInt32 recentRuns, bool includeSourceStatus ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( refuseToSend )
            return GUCEF::CORE::CString::Empty;
        detailJobs.push_back( jobId );
        lastDetailRequestId = "fake-detail-" + GUCEF::CORE::UInt32ToString( m_nextId++ );
        return lastDetailRequestId;
    }

    /** The host answers a GetJob */
    void AnnounceJobDetail( const GUCEF::DRNUTILS::SSchedulerJobDetailAnswer& answer )
    {
        TJobDetailReceivedEventData data( answer );
        NotifyObservers( JobDetailReceivedEvent, &data );
    }

    virtual bool RequestInfo( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++infoRequests;
        return true;
    }

    virtual GUCEF::CORE::CString Control( const GUCEF::CORE::SJobControlCommand& command, bool waitForRun ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( refuseToSend )
            return GUCEF::CORE::CString::Empty;
        commands.push_back( command );
        lastCommandId = "fake-command-" + GUCEF::CORE::UInt32ToString( m_nextId++ );
        return lastCommandId;
    }

    /** The host tells its info, as the answer of GetInfo does */
    void AnnounceInfo( const GUCEF::DRNUTILS::SSchedulerHostInfo& info )
    {
        TInfoReceivedEventData data( info );
        NotifyObservers( InfoReceivedEvent, &data );
    }

    /** The host answers a command */
    void AnnounceOutcome( const GUCEF::CORE::CString& commandId, GUCEF::DRNUTILS::TDrnUtilsCommandStatus status, const GUCEF::CORE::CString& message )
    {
        GUCEF::DRNUTILS::SDrnUtilsCommandOutcome outcome;
        outcome.commandId = commandId;
        outcome.status = status;
        outcome.statusMessage = message;
        TCommandCompletedEventData data( outcome );
        NotifyObservers( CommandCompletedEvent, &data );
    }

    private:

    GUCEF::UInt32 m_nextId;
};

/** The same for the task manager, with the calls that keep a viewing open */
class CFakeTaskManagerClient : public GUCEF::DRNUTILS::CTaskManagerClient
{
    public:

    struct SViewing
    {
        GUCEF::UInt32 sampleIntervalMs;
        bool includeThreads;
        bool includeTasks;
    };

    GUCEF::vector< GUCEF::DRNUTILS::STaskManagerControlCommand > commands;
    GUCEF::vector< SViewing > viewings;
    GUCEF::CORE::CString lastCommandId;
    GUCEF::UInt32 stopViewingCalls;
    GUCEF::UInt32 infoRequests;

    CFakeTaskManagerClient( void )
        : GUCEF::DRNUTILS::CTaskManagerClient( GUCEF::CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
        , commands(), viewings(), lastCommandId(), stopViewingCalls( 0 ), infoRequests( 0 ), m_nextId( 1 )
    {
    }

    virtual bool RequestInfo( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++infoRequests;
        return true;
    }

    virtual GUCEF::CORE::CString Control( const GUCEF::DRNUTILS::STaskManagerControlCommand& command ) GUCEF_VIRTUAL_OVERRIDE
    {
        commands.push_back( command );
        lastCommandId = "fake-task-command-" + GUCEF::CORE::UInt32ToString( m_nextId++ );
        return lastCommandId;
    }

    virtual bool StartViewing( GUCEF::UInt32 sampleIntervalMs, bool includeThreads, bool includeTasks ) GUCEF_VIRTUAL_OVERRIDE
    {
        SViewing viewing;
        viewing.sampleIntervalMs = sampleIntervalMs;
        viewing.includeThreads = includeThreads;
        viewing.includeTasks = includeTasks;
        viewings.push_back( viewing );
        return true;
    }

    virtual void StopViewing( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        ++stopViewingCalls;
    }

    void AnnounceInfo( const GUCEF::DRNUTILS::STaskManagerHostInfo& info )
    {
        TInfoReceivedEventData data( info );
        NotifyObservers( InfoReceivedEvent, &data );
    }

    void AnnounceOutcome( const GUCEF::CORE::CString& commandId, GUCEF::DRNUTILS::TDrnUtilsCommandStatus status, const GUCEF::CORE::CString& message )
    {
        GUCEF::DRNUTILS::SDrnUtilsCommandOutcome outcome;
        outcome.commandId = commandId;
        outcome.status = status;
        outcome.statusMessage = message;
        TCommandCompletedEventData data( outcome );
        NotifyObservers( CommandCompletedEvent, &data );
    }

    private:

    GUCEF::UInt32 m_nextId;
};

/*-------------------------------------------------------------------------*/

/** A scheduler host info that allows the actions of the form (run now, cancel run, enable, disable) */
GUCEF::DRNUTILS::SSchedulerHostInfo MakeSchedulerInfo( const GUCEF::CORE::CString& hostName, bool allowControl );

/** A task manager info with the capabilities and the pools that may be controlled */
GUCEF::DRNUTILS::STaskManagerHostInfo MakeTaskManagerInfo( const GUCEF::CORE::CString& hostName, bool allowControl );

/** The three tables of a scheduler with a few rows: jobs job-1 (SCHEDULED), job-2 (RUNNING), job-3 (DISABLED), a lane and runs of job-1 and job-2 */
struct STestSchedulerTables
{
    GUCEF::CORE::CIDataTablePtr jobsPtr;
    GUCEF::CORE::CIDataTablePtr lanesPtr;
    GUCEF::CORE::CIDataTablePtr runsPtr;
    CTestTable* jobs;
    CTestTable* lanes;
    CTestTable* runs;

    STestSchedulerTables( void ) : jobsPtr(), lanesPtr(), runsPtr(), jobs( GUCEF_NULL ), lanes( GUCEF_NULL ), runs( GUCEF_NULL ) {}

    bool Fill( void );

    GUCEF::DRNUTILS::CSchedulerTableSet GetSet( void ) const;
};

struct STestTaskManagerTables
{
    GUCEF::CORE::CIDataTablePtr poolsPtr;
    GUCEF::CORE::CIDataTablePtr threadsPtr;
    GUCEF::CORE::CIDataTablePtr tasksPtr;
    GUCEF::CORE::CIDataTablePtr scheduledJobsPtr;
    CTestTable* pools;
    CTestTable* threads;
    CTestTable* tasks;
    CTestTable* scheduledJobs;

    STestTaskManagerTables( void ) : poolsPtr(), threadsPtr(), tasksPtr(), scheduledJobsPtr(), pools( GUCEF_NULL ), threads( GUCEF_NULL ), tasks( GUCEF_NULL ), scheduledJobs( GUCEF_NULL ) {}

    bool Fill( void );

    GUCEF::DRNUTILS::CTaskManagerTableSet GetSet( void ) const;
};

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_TESTAPP_TESTFIXTURES_H ? */
