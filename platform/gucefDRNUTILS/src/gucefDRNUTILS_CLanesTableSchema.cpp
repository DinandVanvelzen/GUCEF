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

#include "gucefDRNUTILS_CLanesTableSchema.h"

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

const char* CLanesTableSchema::TableName = "lanes";
const char* CLanesTableSchema::KeyColumnName = "lane";

namespace {

struct SColumn
{
    const char* name;
    CORE::TDataTableLogicalType type;
};

const SColumn g_columns[] =
{
    { "lane", CORE::DATATABLE_TYPE_UTF8 },
    { "mode", CORE::DATATABLE_TYPE_UTF8 },
    { "state", CORE::DATATABLE_TYPE_UTF8 },
    { "inFlight", CORE::DATATABLE_TYPE_UINT32 },
    { "maxConcurrent", CORE::DATATABLE_TYPE_UINT32 },
    { "queuedP0", CORE::DATATABLE_TYPE_UINT32 },
    { "queuedP1", CORE::DATATABLE_TYPE_UINT32 },
    { "queuedP2", CORE::DATATABLE_TYPE_UINT32 },
    { "queuedP3", CORE::DATATABLE_TYPE_UINT32 },
    { "waitingFor", CORE::DATATABLE_TYPE_UTF8 },
    { "waitingUntilUtc", CORE::DATATABLE_TYPE_TIMESTAMP },
    { "unavailableReason", CORE::DATATABLE_TYPE_UTF8 },
    { "heldBy", CORE::DATATABLE_TYPE_UTF8 },
    { "revision", CORE::DATATABLE_TYPE_UINT64 }
};

}

/*-------------------------------------------------------------------------*/

const char*
CLanesTableSchema::GetColumnName( EColumn column )
{GUCEF_TRACE;

    if ( column < 0 || column >= COLUMN_COUNT )
        return "";
    return g_columns[ column ].name;
}

/*-------------------------------------------------------------------------*/

void
CLanesTableSchema::GetColumnNames( CORE::CString::StringVector& columnNames )
{GUCEF_TRACE;

    columnNames.clear();
    for ( Int32 i=0; i<COLUMN_COUNT; ++i )
        columnNames.push_back( g_columns[ i ].name );
}

/*-------------------------------------------------------------------------*/

void
CLanesTableSchema::GetSchema( CORE::CDataTableSchema& schema )
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
