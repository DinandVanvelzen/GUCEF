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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_GUI_CGUICONTEXT_H
#include "gucefGUI_CGUIContext.h"
#define GUCEF_GUI_CGUICONTEXT_H
#endif /* GUCEF_GUI_CGUICONTEXT_H ? */

#ifndef GUCEF_GUI_CGRIDVIEW_H
#include "gucefGUI_CGridView.h"
#define GUCEF_GUI_CGRIDVIEW_H
#endif /* GUCEF_GUI_CGRIDVIEW_H ? */

#ifndef GUCEF_GUI_CFILESAVEDIALOGFORM_H
#include "gucefGUI_CFileSaveDialogForm.h"
#define GUCEF_GUI_CFILESAVEDIALOGFORM_H
#endif /* GUCEF_GUI_CFILESAVEDIALOGFORM_H ? */

#ifndef GUCEF_GUI_CMESSAGEBOXFORM_H
#include "gucefGUI_CMessageBoxForm.h"
#define GUCEF_GUI_CMESSAGEBOXFORM_H
#endif /* GUCEF_GUI_CMESSAGEBOXFORM_H ? */

#ifndef GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H
#include "gucefGUI_CNativeGridExportTarget.h"
#define GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H
#endif /* GUCEF_GUI_CNATIVEGRIDEXPORTTARGET_H ? */

#ifndef GUCEF_GUI_CTABLECELLFORMATTER_H
#include "gucefGUI_CTableCellFormatter.h"
#define GUCEF_GUI_CTABLECELLFORMATTER_H
#endif /* GUCEF_GUI_CTABLECELLFORMATTER_H ? */

#include "gucefGUI_CGridCsvExportController.h"

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

const UInt32 MaxFileNameBytes = 100;

CString
CountText( UInt64 count, const char* one, const char* many )
{
    return CTableCellFormatter::GroupDigits( CORE::UInt64ToString( count ) ) + " " + ( 1 == count ? one : many );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGridCsvExportController::CGridCsvExportController( CGridView& grid )
    : CORE::CObserver()
    , m_grid( grid )
    , m_options()
    , m_targets()
    , m_dialog( GUCEF_NULL )
    , m_messageBox( GUCEF_NULL )
    , m_mode( EXPORTMODE_NONE )
    , m_exporting( false )
    , m_cancelRequested( false )
    , m_rowsChanged( false )
    , m_target()
    , m_rootName()
    , m_path()
    , m_stream()
    , m_source()
    , m_exporter()
    , m_future( CORE::TASKSTATUS_UNDEFINED )
    , m_frameNotifier( GUCEF_NULL )
    , m_listenedModel()
    , m_lastReport()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGridCsvExportController::~CGridCsvExportController()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    if ( m_exporting )
    {
        // nothing half written is left behind: a task on a pool is waited for, it holds the file
        if ( EXPORTMODE_THREADPOOL == m_mode && !m_future.HasNoFuture() )
            m_future.GetResult();
        m_exporter.Abort();
        if ( !m_target.IsNULL() )
            m_target->Abandon( m_path, m_stream );
        m_exporting = false;
    }
    DestroyOwnedForm( m_dialog );
    m_dialog = GUCEF_NULL;
    DestroyOwnedForm( m_messageBox );
    m_messageBox = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::DestroyOwnedForm( CForm* form )
{GUCEF_TRACE;

    // the forms belong to the context of the grid: a context that went has deleted them already
    CGUIContext* context = m_grid.GetContext();
    if ( GUCEF_NULL != form && GUCEF_NULL != context && context->OwnsForm( form ) )
        context->DestroyForm( form );
}

/*-------------------------------------------------------------------------*/

CGridCsvOptions&
CGridCsvExportController::GetOptions( void )
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

const CGridCsvOptions&
CGridCsvExportController::GetOptions( void ) const
{GUCEF_TRACE;

    return m_options;
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::AddTarget( const TGridExportTargetPtr& target )
{GUCEF_TRACE;

    if ( !target.IsNULL() )
        m_targets.push_back( target );
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::ClearTargets( void )
{GUCEF_TRACE;

    m_targets.clear();
}

/*-------------------------------------------------------------------------*/

const CGridCsvExportController::TGridExportTargetVector&
CGridCsvExportController::GetTargets( void ) const
{GUCEF_TRACE;

    return m_targets;
}

/*-------------------------------------------------------------------------*/

CString
CGridCsvExportController::SanitizeFileName( const CString& name )
{GUCEF_TRACE;

    const CORE::CUtf8String utf8 = CORE::ToUtf8String( name );
    const char* bytes = utf8.C_String();
    const UInt32 count = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    GUCEF::vector< char > safe;     // bytes, UTF-8 sequences kept as they are
    safe.reserve( count + 1 );
    for ( UInt32 i=0; i<count; ++i )
    {
        const unsigned char c = (unsigned char) bytes[ i ];
        const bool forbidden = c < 0x20 || 0x7F == c || '<' == c || '>' == c || ':' == c || '"' == c || '/' == c || '\\' == c || '|' == c || '?' == c || '*' == c;
        safe.push_back( forbidden ? '_' : (char) c );
    }

    // no dots or blanks at the ends (Windows drops them, a leading dot hides the file), at most MaxFileNameBytes and never inside a UTF-8 sequence
    size_t first = 0;
    while ( first < safe.size() && ( '.' == safe[ first ] || ' ' == safe[ first ] ) )
        ++first;
    size_t last = safe.size();
    if ( last - first > MaxFileNameBytes )
    {
        last = first + MaxFileNameBytes;
        while ( last > first && 0x80 == ( (unsigned char) safe[ last ] & 0xC0 ) )
            --last;
    }
    while ( last > first && ( '.' == safe[ last - 1 ] || ' ' == safe[ last - 1 ] ) )
        --last;
    if ( last <= first )
        return "table";
    GUCEF::vector< char > text( safe.begin() + first, safe.begin() + last );
    text.push_back( '\0' );
    return CORE::ToString( CORE::CUtf8String( &text[ 0 ] ) );
}

/*-------------------------------------------------------------------------*/

CString
CGridCsvExportController::GetSuggestedFileName( void ) const
{GUCEF_TRACE;

    CString name = m_options.suggestedFileName;
    if ( name.IsNULLOrEmpty() )
        name = m_grid.GetName();
    if ( name.IsNULLOrEmpty() )
        name = "table";
    name = SanitizeFileName( name );
    if ( !name.Lowercase().EndsWith( ".csv" ) )
        name += ".csv";
    return name;
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExportController::Open( void )
{GUCEF_TRACE;

    if ( m_exporting )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExportController: An export of grid \"" << m_grid.GetName() << "\" is running; a new one is not started" );
        return false;
    }
    CGUIContext* context = m_grid.GetContext();
    if ( GUCEF_NULL == context )
        return false;
    if ( m_targets.empty() )
        m_targets.push_back( ( GUCEF_NEW CNativeGridExportTarget() )->CreateSharedPtr() );

    // a dialog of the targets there are now
    CForm* previous = m_dialog;
    m_dialog = GUCEF_NULL;
    DestroyOwnedForm( previous );
    m_dialog = GUCEF_NEW CFileSaveDialogForm( context );
    context->AdoptForm( m_dialog );
    CFileSystemDialogForm::TOptions& options = m_dialog->GetOptions();
    options.title = "Export as CSV";
    options.acceptLabel = "Export";
    options.confirmOverwrite = true;
    options.initialName = GetSuggestedFileName();
    const CGridPreferences& preferences = m_grid.GetPreferences();
    for ( UInt32 i=0; i<(UInt32)m_targets.size(); ++i )
    {
        const CString name = m_targets[ i ]->GetDisplayName();
        const bool last = !preferences.exportRoot.IsNULLOrEmpty() && preferences.exportRoot == name;
        m_dialog->AddRoot( name, m_targets[ i ]->GetFileSystemAccess(), last && !preferences.exportDirectory.IsNULLOrEmpty() ? preferences.exportDirectory : m_targets[ i ]->GetInitialDirectory() );
        if ( last )
            options.initialRoot = name;
    }
    CString::StringVector patterns;
    patterns.push_back( "*.csv" );
    m_dialog->AddFileType( "CSV file", patterns, "csv" );

    TEventCallback acceptedCallback( this, &CGridCsvExportController::OnDialogAccepted );
    SubscribeTo( m_dialog, CFileSystemDialogForm::AcceptedEvent, acceptedCallback );
    TEventCallback cancelledCallback( this, &CGridCsvExportController::OnDialogCancelled );
    SubscribeTo( m_dialog, CFileSystemDialogForm::CancelledEvent, cancelledCallback );
    return m_dialog->Open();
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExportController::IsDialogOpen( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_dialog && m_dialog->IsOpen();
}

/*-------------------------------------------------------------------------*/

CFileSaveDialogForm*
CGridCsvExportController::GetDialog( void )
{GUCEF_TRACE;

    return m_dialog;
}

/*-------------------------------------------------------------------------*/

CMessageBoxForm*
CGridCsvExportController::GetMessageBox( void )
{GUCEF_TRACE;

    return m_messageBox;
}

/*-------------------------------------------------------------------------*/

TGridExportTargetPtr
CGridCsvExportController::FindTarget( const CORE::IFileSystemAccess* access ) const
{GUCEF_TRACE;

    for ( UInt32 i=0; i<(UInt32)m_targets.size(); ++i )
        if ( m_targets[ i ]->GetFileSystemAccess().GetPointerAlways() == access )
            return m_targets[ i ];
    return TGridExportTargetPtr();
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::OnDialogAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_dialog || m_dialog->GetSelection().empty() )
        return;
    const CFileSystemDialogForm::TSelection& selection = m_dialog->GetSelection().front();

    // the root is matched by its file system, not by its name
    TGridExportTargetPtr target = FindTarget( selection.access.GetPointerAlways() );
    if ( target.IsNULL() )
    {
        Fail( "The place chosen is not one the table can be exported to" );
        return;
    }
    m_grid.SetExportLocation( m_dialog->GetCurrentRootName(), m_dialog->GetCurrentDirectory() );
    m_rootName = selection.rootName;
    ExportTo( target, selection.path );
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::OnDialogCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_grid.NotifyObservers( CGridView::CsvExportCancelledEvent );
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExportController::Fail( const CString& message )
{GUCEF_TRACE;

    CGridCsvExportReport report;
    report.success = false;
    report.rootName = m_rootName;
    report.path = m_path;
    report.message = message;
    m_exporting = false;
    Complete( report );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExportController::ExportTo( const TGridExportTargetPtr& target, const CString& path )
{GUCEF_TRACE;

    if ( m_exporting || target.IsNULL() )
        return false;
    m_target = target;
    m_path = path;
    if ( m_rootName.IsNULLOrEmpty() )
        m_rootName = target->GetDisplayName();
    m_cancelRequested = false;
    m_rowsChanged = false;
    m_mode = EXPORTMODE_NONE;

    CString error;
    if ( !m_options.Validate( error ) )
        return Fail( error );
    m_source = ( GUCEF_NEW CGridExportSource( m_grid, m_options.rawValues ) )->CreateSharedPtr();
    if ( 0 == m_source->GetColumnCount() )
        return Fail( "The table has no columns to export" );
    if ( !m_target->OpenForWriting( m_path, m_stream, error ) )
        return Fail( error );
    m_exporting = true;

    const UInt64 cells = (UInt64) m_source->GetRowCount() * (UInt64) m_source->GetColumnCount();
    if ( m_options.useThreadPool && m_source->IsDetached() )
    {
        m_mode = EXPORTMODE_THREADPOOL;
        m_future = CGridCsvExporter::ExportAsync( m_source, m_stream, m_options );
        if ( m_future.HasNoFuture() )
        {
            m_target->Abandon( m_path, m_stream );
            return Fail( "The export could not be given to a thread" );
        }
        StartListening( false );
        return true;
    }
    if ( cells <= m_options.immediateCellLimit )
    {
        m_mode = EXPORTMODE_IMMEDIATE;
        CGridCsvExportReport report;
        CGridCsvExporter::Export( *m_source, m_stream, m_options, report );
        Complete( report );
        return report.success;
    }

    // in slices per frame: the GUI keeps drawing and the model is read on its own thread
    m_mode = EXPORTMODE_SLICED;
    if ( !m_exporter.Begin( *m_source, m_stream, m_options, error ) )
    {
        CGridCsvExportReport report;
        report.message = error;
        Complete( report );
        return false;
    }
    StartListening( true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::StartListening( bool toModel )
{GUCEF_TRACE;

    CGUIContext* context = m_grid.GetContext();
    if ( GUCEF_NULL != context && m_frameNotifier != context )
    {
        TEventCallback frameCallback( this, &CGridCsvExportController::OnFrameStarted );
        SubscribeTo( context, CGUIContext::FrameStartedEvent, frameCallback );
        m_frameNotifier = context;
    }
    if ( toModel && m_grid.HasModel() )
    {
        m_listenedModel = m_grid.GetModel();
        CORE::CNotifier* model = m_listenedModel.GetPointerAlways();
        TEventCallback insertedCallback( this, &CGridCsvExportController::OnModelRowsChanged );
        SubscribeTo( model, CITableViewModel::RowsInsertedEvent, insertedCallback );
        TEventCallback removedCallback( this, &CGridCsvExportController::OnModelRowsChanged );
        SubscribeTo( model, CITableViewModel::RowsRemovedEvent, removedCallback );
        TEventCallback resetCallback( this, &CGridCsvExportController::OnModelRowsChanged );
        SubscribeTo( model, CITableViewModel::ModelResetEvent, resetCallback );
        TEventCallback columnsCallback( this, &CGridCsvExportController::OnModelRowsChanged );
        SubscribeTo( model, CITableViewModel::ColumnsChangedEvent, columnsCallback );
    }
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::StopListening( void )
{GUCEF_TRACE;

    // the frames stay subscribed (the handler does nothing without an export); the model is let go
    if ( !m_listenedModel.IsNULL() )
    {
        UnsubscribeFrom( *m_listenedModel.GetPointerAlways() );
        m_listenedModel.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::OnModelRowsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_exporting && EXPORTMODE_SLICED == m_mode )
        m_rowsChanged = true;
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( !m_exporting )
        return;
    if ( EXPORTMODE_THREADPOOL == m_mode )
    {
        if ( !m_future.IsReady() )
            return;
        CGridCsvExportReport report;
        if ( !CGridCsvExporter::GetAsyncResult( m_future, report ) )
            report.message = "The thread of the export gave no result";
        Complete( report );
        return;
    }
    if ( EXPORTMODE_SLICED != m_mode )
        return;

    if ( m_rowsChanged || m_cancelRequested )
    {
        CGridCsvExportReport report;
        report.cancelled = true;
        report.rowCount = m_exporter.GetRowsWritten();
        report.columnCount = m_exporter.GetColumnCount();
        report.message = m_rowsChanged ? "The rows of the table changed while it was exported" : "The export was cancelled";
        m_exporter.Abort();
        Complete( report );
        return;
    }

    // a slice of about cellsPerFrame cells
    const UInt32 columns = m_exporter.GetColumnCount() > 0 ? m_exporter.GetColumnCount() : 1;
    UInt32 rows = m_options.cellsPerFrame / columns;
    if ( rows < 1 )
        rows = 1;
    CGridCsvExporter::EStepResult step = CGridCsvExporter::STEP_MORE;
    while ( rows > 0 && CGridCsvExporter::STEP_MORE == step )
    {
        const UInt64 before = m_exporter.GetRowsWritten();
        step = m_exporter.Step( rows );
        const UInt64 written = m_exporter.GetRowsWritten() - before;
        rows = written >= rows ? 0 : rows - (UInt32) written;
        if ( 0 == written )
            break;
    }
    if ( CGridCsvExporter::STEP_MORE == step )
        return;
    CGridCsvExportReport report;
    if ( CGridCsvExporter::STEP_DONE == step )
        m_exporter.Finish( report );
    else
    {
        report.rowCount = m_exporter.GetRowsWritten();
        report.columnCount = m_exporter.GetColumnCount();
        report.message = m_exporter.GetLastError();
    }
    Complete( report );
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::CancelExport( void )
{GUCEF_TRACE;

    if ( !m_exporting )
        return;
    m_cancelRequested = true;
    if ( EXPORTMODE_SLICED == m_mode )
    {
        CGridCsvExportReport report;
        report.cancelled = true;
        report.rowCount = m_exporter.GetRowsWritten();
        report.columnCount = m_exporter.GetColumnCount();
        report.message = "The export was cancelled";
        m_exporter.Abort();
        Complete( report );
    }
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::Complete( CGridCsvExportReport& report )
{GUCEF_TRACE;

    StopListening();
    if ( m_cancelRequested && !report.cancelled )
    {
        report.cancelled = true;
        report.success = false;
        report.message = "The export was cancelled";
    }
    if ( !m_target.IsNULL() && m_exporting )
    {
        // the file the user chose is replaced by a complete one only
        CString error;
        if ( report.success && !report.cancelled )
        {
            if ( !m_target->Commit( m_path, m_stream, error ) )
            {
                report.success = false;
                report.message = error;
            }
        }
        else
            m_target->Abandon( m_path, m_stream );
    }
    m_stream.Unlink();
    m_source.Unlink();
    m_exporting = false;
    m_cancelRequested = false;
    m_rowsChanged = false;

    report.rootName = m_rootName;
    report.path = m_path;
    if ( report.success )
    {
        report.message = "Exported " + CountText( report.rowCount, "row", "rows" ) + " to " + m_path;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExportController: Grid \"" << m_grid.GetName() << "\": " << report.message << " (" << report.byteCount << " bytes)" );
    }
    else
    {
        report.message = "The table could not be exported to " + m_path + ": " + report.message;
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GridCsvExportController: Grid \"" << m_grid.GetName() << "\": " << report.message );
    }
    m_lastReport = report;
    m_rootName.Clear();
    m_target.Unlink();

    CGridView::TGridCsvExportReportEventData eventData( report );
    m_grid.NotifyObservers( CGridView::CsvExportFinishedEvent, &eventData );
    if ( m_options.showResultMessage )
        Tell( report );
}

/*-------------------------------------------------------------------------*/

void
CGridCsvExportController::Tell( const CGridCsvExportReport& report )
{GUCEF_TRACE;

    CGUIContext* context = m_grid.GetContext();
    if ( GUCEF_NULL == context )
        return;
    if ( GUCEF_NULL == m_messageBox )
    {
        m_messageBox = GUCEF_NEW CMessageBoxForm( context );
        context->AdoptForm( m_messageBox );
    }
    m_messageBox->Open( report.success ? "Export as CSV" : "Export failed", report.message, CMessageBoxForm::MESSAGEBOXBUTTONS_OK );
}

/*-------------------------------------------------------------------------*/

bool
CGridCsvExportController::IsExporting( void ) const
{GUCEF_TRACE;

    return m_exporting;
}

/*-------------------------------------------------------------------------*/

CGridCsvExportController::EExportMode
CGridCsvExportController::GetExportMode( void ) const
{GUCEF_TRACE;

    return m_mode;
}

/*-------------------------------------------------------------------------*/

const CGridCsvExportReport&
CGridCsvExportController::GetLastReport( void ) const
{GUCEF_TRACE;

    return m_lastReport;
}

/*-------------------------------------------------------------------------*/

const CString&
CGridCsvExportController::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::GUI::CGridCsvExportController";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
