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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTABLECODECREGISTRY_H
#include "gucefCORE_CTableCodecRegistry.h"
#define GUCEF_CORE_CTABLECODECREGISTRY_H
#endif /* GUCEF_CORE_CTABLECODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CITABLECODEC_H
#include "gucefCORE_CITableCodec.h"
#define GUCEF_CORE_CITABLECODEC_H
#endif /* GUCEF_CORE_CITABLECODEC_H ? */

#ifndef GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#include "gucefCORE_CDataTableBatchBuilder.h"
#define GUCEF_CORE_CDATATABLEBATCHBUILDER_H
#endif /* GUCEF_CORE_CDATATABLEBATCHBUILDER_H ? */

#ifndef GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#include "gucefCORE_CDataTableWriterOptions.h"
#define GUCEF_CORE_CDATATABLEWRITEROPTIONS_H
#endif /* GUCEF_CORE_CDATATABLEWRITEROPTIONS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CASYNCGRIDCSVEXPORT_H
#include "gucefGUI_CAsyncGridCsvExport.h"
#define GUCEF_GUI_CASYNCGRIDCSVEXPORT_H
#endif /* GUCEF_GUI_CASYNCGRIDCSVEXPORT_H ? */

#include "gucefGUI_CGridCsvExporter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The delimiter as the CSV codec reads its option: blanks by name, the others as the character */
CString
DelimiterOption( char delimiter )
{
    if ( '\t' == delimiter )
        return "tab";
    if ( ' ' == delimiter )
        return "space";
    CString text;
    text += delimiter;
    return text;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridCsvExporter::CGridCsvExporter( void )
    : m_source( GUCEF_NULL )
    , m_output()
    , m_options()
    , m_schema()
    , m_writer()
    , m_rowCount( 0 )
    , m_nextRow( 0 )
    , m_columnCount( 0 )
    , m_headerPending( false )
    , m_active( false )
    , m_failed( false )
    , m_error()
    , m_cells()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridCsvExporter::~CGridCsvExporter()
{GUCEF_TRACE;

    if ( m_active )
        Abort();
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExporter::Release( void )
{GUCEF_TRACE;

    if ( !m_writer.IsNULL() && m_active )
        m_writer->Abort();
    m_writer.Unlink();
    m_output.Unlink();
    m_schema.Unlink();
    m_source = GUCEF_NULL;
    m_active = false;
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExporter::Fail( const CString& error )
{GUCEF_TRACE;

    m_failed = true;
    m_error = error;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExporter: " << error );
    Release();
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExporter::Begin( CGridExportSource& source, const CORE::IOAccessPtr& output, const CGridCsvOptions& options, CString& error )
{GUCEF_TRACE;

    if ( m_active )
        Abort();
    m_failed = false;
    m_error.Clear();
    m_rowCount = 0;
    m_nextRow = 0;
    m_columnCount = 0;

    if ( !options.Validate( error ) )
    {
        m_error = error;
        m_failed = true;
        return false;
    }
    if ( 0 == source.GetColumnCount() )
    {
        error = "The table has no columns to export";
        m_error = error;
        m_failed = true;
        return false;
    }
    CORE::IOAccessPtr stream( output );
    if ( stream.IsNULL() || !stream->IsValid() || !stream->IsWriteable() )
    {
        error = "The file to export to cannot be written";
        m_error = error;
        m_failed = true;
        return false;
    }

    CORE::CTableCodecRegistry::TTableCodecPtr codec;
    if ( !CORE::CCoreGlobal::Instance()->GetTableCodecRegistry().TryGetCsvCodec( codec ) || codec.IsNULL() )
    {
        error = "The CSV table codec is not registered";
        m_error = error;
        m_failed = true;
        return false;
    }

    // text columns with names of their own: the titles of a grid can be empty or repeat, a schema cannot; the titles are the first row
    const UInt32 columns = source.GetColumnCount();
    CORE::CDataTableSchema schema;
    for ( UInt32 c=0; c<columns; ++c )
        schema.AddColumn( CORE::CDataTableColumnDef( "c" + CORE::UInt32ToString( c + 1 ), CORE::DATATABLE_TYPE_UTF8, true ) );
    m_schema = CORE::CreateDataTableSchemaPtr( schema );

    CORE::CDataTableWriterOptions writerOptions;
    writerOptions.codecOptions.Set( "delimiter", CORE::CVariant( DelimiterOption( options.delimiter ) ) );
    writerOptions.codecOptions.Set( "lineEnd", CORE::CVariant( options.GetLineEndingCodecText() ) );
    writerOptions.codecOptions.Set( "bom", CORE::CVariant( CString( options.byteOrderMark ? "true" : "false" ) ) );
    writerOptions.codecOptions.Set( "header", CORE::CVariant( CString( "false" ) ) );

    CString codecError;
    const CORE::TDataTableStatus status = codec->CreateWriter( output, m_schema, writerOptions, m_writer, codecError );
    if ( CORE::DATATABLE_STATUS_OK != status || m_writer.IsNULL() )
    {
        error = "The CSV writer cannot be made: " + codecError;
        m_error = error;
        m_failed = true;
        m_writer.Unlink();
        m_schema.Unlink();
        return false;
    }

    m_source = &source;
    m_output = output;
    m_options = options;
    m_columnCount = columns;
    m_rowCount = source.GetRowCount();
    m_headerPending = options.includeHeader;
    m_active = true;
    m_cells.resize( columns );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExporter::WriteBatch( UInt32 rows )
{GUCEF_TRACE;

    CORE::CDataTableBatchBuilder builder( m_schema, CORE::CCoreGlobal::Instance()->GetDataTableBufferPool() );

    // the titles first; an empty text is a null, which is an empty field (the codec quotes an empty text)
    if ( m_headerPending )
    {
        const CGridExportSource::TStringVector& titles = m_source->GetTitles();
        for ( UInt32 c=0; c<m_columnCount; ++c )
        {
            CORE::CDataTableColumnBuilder& column = builder.GetColumn( c );
            const bool appended = c < (UInt32) titles.size() && !titles[ c ].IsNULLOrEmpty() ? column.AppendString( titles[ c ] ) : column.AppendNull();
            if ( !appended )
            {
                Fail( "The title of column " + CORE::UInt32ToString( c + 1 ) + " cannot be written" );
                return false;
            }
        }
        m_headerPending = false;
    }

    if ( rows > 0 )
        m_source->PrepareRows( (UInt32) m_nextRow, rows );
    for ( UInt32 r=0; r<rows; ++r )
    {
        const UInt32 row = (UInt32) ( m_nextRow + r );
        if ( !m_source->GetRowTexts( row, m_cells ) )
        {
            // the rows went while the export ran: what is there is written, the rest is not
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExporter: The table has " << row << " rows now, " << m_rowCount << " when the export began" );
            m_rowCount = row;
            rows = r;
            break;
        }
        for ( UInt32 c=0; c<m_columnCount; ++c )
        {
            CORE::CDataTableColumnBuilder& column = builder.GetColumn( c );
            const bool appended = m_cells[ c ].IsNULLOrEmpty() ? column.AppendNull() : column.AppendString( m_cells[ c ] );
            if ( !appended )
            {
                Fail( "Row " + CORE::UInt32ToString( row + 1 ) + ", column " + CORE::UInt32ToString( c + 1 ) + " cannot be written" );
                return false;
            }
        }
    }

    if ( 0 == builder.GetRowCount() )
        return true;
    CORE::CDataTableBatchPtr batch;
    CString error;
    if ( CORE::DATATABLE_STATUS_OK != builder.Build( batch, error ) || batch.IsNULL() )
    {
        Fail( "A batch of rows cannot be made: " + error );
        return false;
    }
    const CORE::TDataTableStatus status = m_writer->WriteBatch( *batch );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        Fail( "Writing failed: " + m_writer->GetLastErrorMessage() );
        return false;
    }
    m_nextRow += rows;
    return true;
}

/*-------------------------------------------------------------------------*/

CGridCsvExporter::EStepResult
CGridCsvExporter::Step( UInt32 maxRows )
{GUCEF_TRACE;

    if ( !m_active )
        return m_failed ? STEP_FAILED : STEP_DONE;
    UInt64 left = m_rowCount > m_nextRow ? m_rowCount - m_nextRow : 0;
    UInt32 rows = maxRows < m_options.batchRowCount ? maxRows : m_options.batchRowCount;
    if ( (UInt64) rows > left )
        rows = (UInt32) left;
    if ( ( rows > 0 || m_headerPending ) && !WriteBatch( rows ) )
        return STEP_FAILED;
    return m_nextRow >= m_rowCount ? STEP_DONE : STEP_MORE;
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExporter::Finish( CGridCsvExportReport& report )
{GUCEF_TRACE;

    report.success = false;
    report.rowCount = m_nextRow;
    report.columnCount = m_columnCount;
    report.byteCount = 0;
    if ( !m_active )
    {
        report.message = m_failed ? m_error : CString( "The export was not started" );
        return false;
    }
    if ( m_headerPending && !WriteBatch( 0 ) )
    {
        report.message = m_error;
        return false;
    }
    CORE::CDataTableFileSummary summary;
    const CORE::TDataTableStatus status = m_writer->Finish( summary );
    if ( CORE::DATATABLE_STATUS_OK != status )
    {
        Fail( "Writing the end of the file failed: " + m_writer->GetLastErrorMessage() );
        report.message = m_error;
        return false;
    }
    m_output->Flush();
    m_active = false;
    Release();
    report.success = true;
    report.rowCount = m_nextRow;
    report.byteCount = summary.byteCount;
    report.message = "Exported " + CORE::UInt64ToString( m_nextRow ) + ( 1 == m_nextRow ? " row" : " rows" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExporter::Abort( void )
{GUCEF_TRACE;

    Release();
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExporter::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

UInt64
CGridCsvExporter::GetRowCount( void ) const
{GUCEF_TRACE;

    return m_rowCount;
}

/*-------------------------------------------------------------------------*/

UInt64
CGridCsvExporter::GetRowsWritten( void ) const
{GUCEF_TRACE;

    return m_nextRow;
}

/*-------------------------------------------------------------------------*/

UInt32
CGridCsvExporter::GetColumnCount( void ) const
{GUCEF_TRACE;

    return m_columnCount;
}

/*-------------------------------------------------------------------------*/

const CString&
CGridCsvExporter::GetLastError( void ) const
{GUCEF_TRACE;

    return m_error;
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExporter::Export( CGridExportSource& source, const CORE::IOAccessPtr& output, const CGridCsvOptions& options, CGridCsvExportReport& report )
{GUCEF_TRACE;

    CGridCsvExporter exporter;
    CString error;
    if ( !exporter.Begin( source, output, options, error ) )
    {
        report.success = false;
        report.message = error;
        return false;
    }
    EStepResult step = STEP_MORE;
    while ( STEP_MORE == step )
        step = exporter.Step( options.batchRowCount );
    if ( STEP_FAILED == step )
    {
        report.success = false;
        report.rowCount = exporter.GetRowsWritten();
        report.columnCount = exporter.GetColumnCount();
        report.message = exporter.GetLastError();
        return false;
    }
    return exporter.Finish( report );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CGridCsvExporter::ExportAsync( const TGridExportSourcePtr& source, const CORE::IOAccessPtr& output, const CGridCsvOptions& options )
{GUCEF_TRACE;

    try
    {
        CGridCsvExportTaskData taskData;
        taskData.source = source;
        taskData.output = output;
        taskData.options = options;

        CORE::CTaskManager& taskManager = CORE::CCoreGlobal::Instance()->GetTaskManager();
        CORE::ThreadPoolPtr threadPool = taskManager.GetOrCreateThreadPool( options.threadPoolName.IsNULLOrEmpty() ? CORE::CTaskManager::DefaultThreadPoolName : options.threadPoolName );
        if ( !threadPool.IsNULL() )
            return threadPool->QueueTask( CAsyncGridCsvExport::TaskType, &taskData );
    }
    catch ( const std::exception& e )
    {
        GUCEF_EXCEPTION_LOG( CORE::LOGLEVEL_IMPORTANT, "GridCsvExporter:ExportAsync: Failed to queue the task: " << e.what() );
    }
    return CORE::CFutureResult( CORE::TASKSTATUS_QUEUEING_FAILED );
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExporter::GetAsyncResult( const CORE::CFutureResult& future, CGridCsvExportReport& report )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() )
        return false;
    const CGridCsvExportTaskData* data = static_cast< const CGridCsvExportTaskData* >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    report = data->report;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
