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

#ifndef GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H
#define GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H

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
 *  The tasks grid of a task manager host (contract TaskRow; never task data); default sort taskId.
 *  Column names equal the field names of the contract, so the RPC rows and the grids describe one model.
 *  Times are timestamp(ms,utc); enums are their names as text. Grid name: <prefix>/tasks.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CTasksTableSchema
{
    public:

    enum EColumn
    {
        COLUMN_TASK_ID = 0,
        COLUMN_POOL,
        COLUMN_TASK_TYPE,
        COLUMN_STATUS,
        COLUMN_EXTRA_INFO,
        COLUMN_QUEUED_UTC,
        COLUMN_STARTED_UTC,
        COLUMN_RUNNING_FOR_MS,
        COLUMN_SYSTEM_THREAD_ID,
        COLUMN_SCHEDULER_NAME,
        COLUMN_JOB_ID,
        COLUMN_RUN_ID,
        COLUMN_COUNT
    };

    static const char* TableName;       /**< "tasks" */
    static const char* KeyColumnName;   /**< "taskId" */

    static const char* GetColumnName( EColumn column );

    static void GetColumnNames( CORE::CString::StringVector& columnNames );

    /** The schema of the grid: the columns in EColumn order, primary key taskId (not nullable, every other column nullable) */
    static void GetSchema( CORE::CDataTableSchema& schema );

    private:

    CTasksTableSchema( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CTASKSTABLESCHEMA_H ? */
