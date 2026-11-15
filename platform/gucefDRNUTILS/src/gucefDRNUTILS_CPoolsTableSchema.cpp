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

#include "gucefDRNUTILS_CPoolsTableSchema.h"

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

const char* CPoolsTableSchema::TableName = "pools";
const char* CPoolsTableSchema::KeyColumnName = "name";

namespace {

struct SColumn
{
    const char* name;
    CORE::TDataTableLogicalType type;
};

const SColumn g_columns[] =
{
    { "name", CORE::DATATABLE_TYPE_UTF8 },
    { "activeWorkers", CORE::DATATABLE_TYPE_UINT32 },
    { "desiredMinWorkers", CORE::DATATABLE_TYPE_UINT32 },
    { "desiredMaxThreads", CORE::DATATABLE_TYPE_INT32 },
    { "dedicatedThreads", CORE::DATATABLE_TYPE_UINT32 },
    { "queued", CORE::DATATABLE_TYPE_UINT32 },
    { "running", CORE::DATATABLE_TYPE_UINT32 },
    { "startedPerSecond", CORE::DATATABLE_TYPE_FLOAT64 },
    { "finishedPerSecond", CORE::DATATABLE_TYPE_FLOAT64 },
    { "failedPerSecond", CORE::DATATABLE_TYPE_FLOAT64 },
    { "cancelledPerSecond", CORE::DATATABLE_TYPE_FLOAT64 },
    { "avgQueueWaitMs", CORE::DATATABLE_TYPE_FLOAT64 },
    { "avgRunMs", CORE::DATATABLE_TYPE_FLOAT64 },
    { "utilisationPercent", CORE::DATATABLE_TYPE_FLOAT32 },
    { "startedTotal", CORE::DATATABLE_TYPE_UINT64 },
    { "finishedTotal", CORE::DATATABLE_TYPE_UINT64 },
    { "failedTotal", CORE::DATATABLE_TYPE_UINT64 },
    { "cancelledTotal", CORE::DATATABLE_TYPE_UINT64 },
    { "scheduledJobs", CORE::DATATABLE_TYPE_UINT32 }
};

}

/*-------------------------------------------------------------------------*/

const char*
CPoolsTableSchema::GetColumnName( EColumn column )
{GUCEF_TRACE;

    if ( column < 0 || column >= COLUMN_COUNT )
        return "";
    return g_columns[ column ].name;
}

/*-------------------------------------------------------------------------*/

void
CPoolsTableSchema::GetColumnNames( CORE::CString::StringVector& columnNames )
{GUCEF_TRACE;

    columnNames.clear();
    for ( Int32 i=0; i<COLUMN_COUNT; ++i )
        columnNames.push_back( g_columns[ i ].name );
}

/*-------------------------------------------------------------------------*/

void
CPoolsTableSchema::GetSchema( CORE::CDataTableSchema& schema )
{GUCEF_TRACE;

    schema.Clear();
    for ( Int32 i=0; i<COLUMN_COUNT; ++i )
    {
        bool nullable = 0 != i;    // the key column (the first) is never null
        if ( CORE::DATATABLE_TYPE_TIMESTAMP == g_columns[ i ].type )
            schema.AddColumn( CORE::CDataTableColumnDef::Timestamp( g_columns[ i ].name, CORE::DATATABLE_TIMEUNIT_MILLISECONDS, true, nullable ) );
        else
            schema.AddColumn( CORE::CDataTableColumnDef( g_columns[ i ].name, g_columns[ i ].type, nullable ) );
    }
    CORE::CString::StringVector key;
    key.push_back( KeyColumnName );
    schema.SetPrimaryKey( key );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
