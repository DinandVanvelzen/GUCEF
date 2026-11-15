/*
 *  gucefGUI: GUCEF module providing a uniform interface towards GUI backends
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

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTER_H
#include "gucefGUI_CGridCsvExporter.h"
#define GUCEF_GUI_CGRIDCSVEXPORTER_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTER_H ? */

#include "gucefGUI_CAsyncGridCsvExport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAsyncGridCsvExport::TaskType = "AsyncGridCsvExport";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridCsvExportTaskData::CGridCsvExportTaskData( void )
    : CORE::CICloneable()
    , source()
    , output()
    , options()
    , report()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridCsvExportTaskData::CGridCsvExportTaskData( const CGridCsvExportTaskData& src )
    : CORE::CICloneable( src )
    , source( src.source )
    , output( src.output )
    , options( src.options )
    , report( src.report )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridCsvExportTaskData::~CGridCsvExportTaskData()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridCsvExportTaskData&
CGridCsvExportTaskData::operator=( const CGridCsvExportTaskData& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        source = src.source;
        output = src.output;
        options = src.options;
        report = src.report;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CGridCsvExportTaskData::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CGridCsvExportTaskData( *this );
}

/*-------------------------------------------------------------------------*/

CAsyncGridCsvExport::CAsyncGridCsvExport( void )
    : CORE::CTaskConsumer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAsyncGridCsvExport::~CAsyncGridCsvExport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAsyncGridCsvExport::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CAsyncGridCsvExport::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    if ( task.IsNULL() )
        return true;
    CGridCsvExportTaskData* data = static_cast< CGridCsvExportTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AsyncGridCsvExport: No task data to work on" );
        return true;
    }
    data->report.Clear();
    if ( data->source.IsNULL() )
    {
        data->report.message = "There is no table to export";
        return true;
    }

    // the steps of CGridCsvExporter::Export(), with a look at the task between batches
    CGridCsvExporter exporter;
    CString error;
    if ( !exporter.Begin( *data->source, data->output, data->options, error ) )
    {
        data->report.message = error;
        return true;
    }
    CGridCsvExporter::EStepResult step = CGridCsvExporter::STEP_MORE;
    while ( CGridCsvExporter::STEP_MORE == step )
    {
        if ( task->IsCancellationRequested() )
        {
            exporter.Abort();
            data->report.cancelled = true;
            data->report.rowCount = exporter.GetRowsWritten();
            data->report.message = "The export was cancelled";
            return true;
        }
        step = exporter.Step( data->options.batchRowCount );
    }
    if ( CGridCsvExporter::STEP_FAILED == step )
    {
        data->report.rowCount = exporter.GetRowsWritten();
        data->report.columnCount = exporter.GetColumnCount();
        data->report.message = exporter.GetLastError();
        return true;
    }
    exporter.Finish( data->report );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAsyncGridCsvExport::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
