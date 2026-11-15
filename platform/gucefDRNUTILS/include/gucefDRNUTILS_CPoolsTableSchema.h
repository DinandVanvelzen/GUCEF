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

#ifndef GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H
#define GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H

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
 *  The pools grid of a task manager host (contract PoolRow); default sort name.
 *  Column names equal the field names of the contract, so the RPC rows and the grids describe one model.
 *  Times are timestamp(ms,utc); enums are their names as text. Grid name: <prefix>/pools.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CPoolsTableSchema
{
    public:

    enum EColumn
    {
        COLUMN_NAME = 0,
        COLUMN_ACTIVE_WORKERS,
        COLUMN_DESIRED_MIN_WORKERS,
        COLUMN_DESIRED_MAX_THREADS,
        COLUMN_DEDICATED_THREADS,
        COLUMN_QUEUED,
        COLUMN_RUNNING,
        COLUMN_STARTED_PER_SECOND,
        COLUMN_FINISHED_PER_SECOND,
        COLUMN_FAILED_PER_SECOND,
        COLUMN_CANCELLED_PER_SECOND,
        COLUMN_AVG_QUEUE_WAIT_MS,
        COLUMN_AVG_RUN_MS,
        COLUMN_UTILISATION_PERCENT,
        COLUMN_STARTED_TOTAL,
        COLUMN_FINISHED_TOTAL,
        COLUMN_FAILED_TOTAL,
        COLUMN_CANCELLED_TOTAL,
        COLUMN_SCHEDULED_JOBS,
        COLUMN_COUNT
    };

    static const char* TableName;       /**< "pools" */
    static const char* KeyColumnName;   /**< "name" */

    static const char* GetColumnName( EColumn column );

    static void GetColumnNames( CORE::CString::StringVector& columnNames );

    /** The schema of the grid: the columns in EColumn order, primary key name (not nullable, every other column nullable) */
    static void GetSchema( CORE::CDataTableSchema& schema );

    private:

    CPoolsTableSchema( void );      /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CPOOLSTABLESCHEMA_H ? */
