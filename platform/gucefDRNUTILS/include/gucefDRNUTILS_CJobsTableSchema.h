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

#ifndef GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H
#define GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATATABLESCHEMA_H
#include "gucefCORE_CDataTableSchema.h"
#define GUCEF_CORE_CDATATABLESCHEMA_H
#endif /* GUCEF_CORE_CDATATABLESCHEMA_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The jobs grid of a scheduler host (contract JobRow without allowedActions, which depend on the viewer); default sort jobId.
 *  Column names equal the field names of the contract, so the RPC rows and the grids describe one model.
 *  Times are timestamp(ms,utc); enums are their names as text. Grid name: <prefix>/jobs.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CJobsTableSchema
{
    public:

    enum EColumn
    {
        COLUMN_JOB_ID = 0,
        COLUMN_TITLE,
        COLUMN_LANE,
        COLUMN_STATE,
        COLUMN_PRIORITY,
        COLUMN_TRIGGER,
        COLUMN_NEXT_DUE_UTC,
        COLUMN_WAITING_FOR,
        COLUMN_WAITING_UNTIL_UTC,
        COLUMN_LAST_RUN_ID,
        COLUMN_LAST_RUN_STATE,
        COLUMN_LAST_RUN_ENDED_UTC,
        COLUMN_RUN_COUNT,
        COLUMN_FAILURE_COUNT,
        COLUMN_CONSECUTIVE_FAILURES,
        COLUMN_REVISION,
        COLUMN_TASK_TYPE,
        COLUMN_EXECUTION_TARGET,
        COLUMN_THREAD_POOL,
        COLUMN_ORIGIN,
        COLUMN_SUBMITTED_BY,
        COLUMN_UNAVAILABLE_REASON,
        COLUMN_ACTIVE_RUNS,
        COLUMN_COUNT
    };

    static const char* TableName;       /**< "jobs" */
    static const char* KeyColumnName;   /**< "jobId" */

    static const char* GetColumnName( EColumn column );

    static void GetColumnNames( CORE::CString::StringVector& columnNames );

    /** The schema of the grid: the columns in EColumn order, primary key jobId (not nullable, every other column nullable) */
    static void GetSchema( CORE::CDataTableSchema& schema );

    private:

    CJobsTableSchema( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CJOBSTABLESCHEMA_H ? */
