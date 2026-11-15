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

#ifndef GUCEF_DRN_CDRNGLOBAL_H
#include "gucefDRN_CDRNGlobal.h"
#define GUCEF_DRN_CDRNGLOBAL_H
#endif /* GUCEF_DRN_CDRNGLOBAL_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CDRNSESSIONREGISTRY_H
#include "gucefDRN_CDRNSessionRegistry.h"
#define GUCEF_DRN_CDRNSESSIONREGISTRY_H
#endif /* GUCEF_DRN_CDRNSESSIONREGISTRY_H ? */

#ifndef GUCEF_DRN_CDRNDATAGRID_H
#include "gucefDRN_CDRNDataGrid.h"
#define GUCEF_DRN_CDRNDATAGRID_H
#endif /* GUCEF_DRN_CDRNDATAGRID_H ? */

#ifndef GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#include "gucefDRN_CDRNPublishedTable.h"
#define GUCEF_DRN_CDRNPUBLISHEDTABLE_H
#endif /* GUCEF_DRN_CDRNPUBLISHEDTABLE_H ? */

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

#ifndef GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H
#include "gucefDRNUTILS_CPoolsTableSchema.h"
#define GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CTHREADSTABLESCHEMA_H
#include "gucefDRNUTILS_CThreadsTableSchema.h"
#define GUCEF_DRNUTILS_CTHREADSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CTHREADSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H
#include "gucefDRNUTILS_CTasksTableSchema.h"
#define GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULEDJOBSTABLESCHEMA_H
#include "gucefDRNUTILS_CScheduledJobsTableSchema.h"
#define GUCEF_DRNUTILS_CSCHEDULEDJOBSTABLESCHEMA_H
#endif /* GUCEF_DRNUTILS_CSCHEDULEDJOBSTABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERSNAPSHOTTABLE_H
#include "gucefDRNUTILS_CSchedulerSnapshotTable.h"
#define GUCEF_DRNUTILS_CSCHEDULERSNAPSHOTTABLE_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERSNAPSHOTTABLE_H ? */

#include "gucefDRNUTILS_CDrnUtilsTableBinder.h"

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

/** Subscribes to the published grid of the session (when it is not the local member's own) and opens it as a table */
static bool
OpenPublishedGrid( const CORE::CString& sessionName, const CORE::CString& gridName, CORE::CIDataTablePtr& table, CORE::CString& error )
{GUCEF_TRACE;

    DRN::CDRNSession* session = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Find( sessionName );
    if ( GUCEF_NULL == session )
    {
        error = "no DRN session \"" + sessionName + "\" is registered in the session registry of gucefDRN";
        return false;
    }
    DRN::CDRNDataGrid* grid = session->FindGrid( gridName );
    if ( GUCEF_NULL == grid )
    {
        error = "the session has no grid \"" + gridName + "\" (not published, or not announced yet)";
        return false;
    }
    if ( !grid->IsOrigin() )
        session->Subscribe( gridName, grid->GetOwnerPeerId() );
    // typed by its #schema once that arrived; the table follows the grid (SchemaChangedEvent, RowsChangedEvent)
    table = CORE::CIDataTablePtr( GUCEF_NEW DRN::CDRNPublishedTable( *grid, DRN::CDRNPublishedTableSettings() ) );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnUtilsTableBinder::GetGridName( const CORE::CString& gridPrefix, const CORE::CString& hostName, const CORE::CString& tableName )
{GUCEF_TRACE;

    CORE::CString prefix = gridPrefix;
    if ( prefix.IsNULLOrEmpty() )
        prefix = "gucef.drnutils/" + hostName;
    return prefix + "/" + tableName;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsTableBinder::OpenSchedulerTables( const CORE::CString& drnSessionName ,
                                           const CORE::CString& gridPrefix     ,
                                           CSchedulerTableSet& tables          ,
                                           CORE::CString& error                )
{GUCEF_TRACE;

    tables.Clear();
    DRN::CDRNSession* session = DRN::CDRNGlobal::Instance()->GetSessionRegistry().Find( drnSessionName );
    CORE::CString hostName;
    if ( gridPrefix.IsNULLOrEmpty() )
    {
        error = "the grid prefix is needed to find the grids of a remote host";
        return false;
    }
    CORE::CIDataTablePtr jobs, lanes, runs;
    if ( GUCEF_NULL == session ||
         !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CJobsTableSchema::TableName ), jobs, error ) ||
         !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CLanesTableSchema::TableName ), lanes, error ) ||
         !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CRunsTableSchema::TableName ), runs, error ) )
    {
        if ( GUCEF_NULL == session )
            error = "no DRN session \"" + drnSessionName + "\" is registered in the session registry of gucefDRN";
        return false;
    }
    tables.SetJobs( jobs );
    tables.SetLanes( lanes );
    tables.SetRuns( runs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsTableBinder::OpenLocalSchedulerTables( CISchedulerSource& source                         ,
                                                const CORE::PulseGeneratorPtr& eventPulseGenerator ,
                                                CSchedulerTableSet& tables                        ,
                                                CORE::CString& error                              )
{GUCEF_TRACE;

    tables.Clear();
    tables.SetJobs( CORE::CIDataTablePtr( GUCEF_NEW CSchedulerSnapshotTable( source, CSchedulerSnapshotTable::TABLE_JOBS, 200, eventPulseGenerator ) ) );
    tables.SetLanes( CORE::CIDataTablePtr( GUCEF_NEW CSchedulerSnapshotTable( source, CSchedulerSnapshotTable::TABLE_LANES, 200, eventPulseGenerator ) ) );
    tables.SetRuns( CORE::CIDataTablePtr( GUCEF_NEW CSchedulerSnapshotTable( source, CSchedulerSnapshotTable::TABLE_RUNS, 200, eventPulseGenerator ) ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDrnUtilsTableBinder::OpenTaskManagerTables( const CORE::CString& drnSessionName ,
                                             const CORE::CString& gridPrefix     ,
                                             CTaskManagerTableSet& tables        ,
                                             CORE::CString& error                )
{GUCEF_TRACE;

    tables.Clear();
    if ( gridPrefix.IsNULLOrEmpty() )
    {
        error = "the grid prefix is needed to find the grids of a remote host";
        return false;
    }
    CORE::CString hostName;
    CORE::CIDataTablePtr pools, threads, tasks, scheduledJobs;
    if ( !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CPoolsTableSchema::TableName ), pools, error ) ||
         !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CThreadsTableSchema::TableName ), threads, error ) ||
         !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CTasksTableSchema::TableName ), tasks, error ) ||
         !OpenPublishedGrid( drnSessionName, GetGridName( gridPrefix, hostName, CScheduledJobsTableSchema::TableName ), scheduledJobs, error ) )
        return false;
    tables.SetPools( pools );
    tables.SetThreads( threads );
    tables.SetTasks( tasks );
    tables.SetScheduledJobs( scheduledJobs );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
