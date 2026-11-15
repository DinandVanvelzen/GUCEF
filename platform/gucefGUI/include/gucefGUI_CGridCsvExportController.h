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

#ifndef GUCEF_GUI_CGRIDCSVEXPORTCONTROLLER_H
#define GUCEF_GUI_CGRIDCSVEXPORTCONTROLLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_GUI_CIGRIDEXPORTTARGET_H
#include "gucefGUI_CIGridExportTarget.h"
#define GUCEF_GUI_CIGRIDEXPORTTARGET_H
#endif /* GUCEF_GUI_CIGRIDEXPORTTARGET_H ? */

#ifndef GUCEF_GUI_CGRIDCSVEXPORTER_H
#include "gucefGUI_CGridCsvExporter.h"
#define GUCEF_GUI_CGRIDCSVEXPORTER_H
#endif /* GUCEF_GUI_CGRIDCSVEXPORTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CGridView;
class CForm;
class CFileSaveDialogForm;
class CMessageBoxForm;

/**
 *  The CSV export of a grid as the user starts it (the export button, CGridView::OpenCsvExport()): asks for the file
 *  with a CFileSaveDialogForm in the context of the grid, writes the view of the grid there and tells the user and the
 *  application what came of it. Owned by its grid.
 *
 *  - The dialog has a root per target (AddTarget(); without any the file system of the computer), the file type
 *    "CSV file" (*.csv), the suggested name (GetSuggestedFileName()) and starts where the last export of the grid went.
 *    It asks before a file is replaced. Cancel writes nothing and raises CGridView::CsvExportCancelledEvent.
 *  - The file is written through the target of the root the user chose: a temporary file, moved over the chosen path
 *    only when the export is complete.
 *  - Up to immediateCellLimit cells are written at once; a larger table in slices of cellsPerFrame cells per frame of
 *    the context (FrameStartedEvent), so the GUI keeps drawing; a change of the rows of the model during a sliced export
 *    ends it with an error. With useThreadPool (a model that may be read from another thread) the export is a task of a
 *    pool, whose future is looked at every frame.
 *  - The outcome: CGridView::CsvExportFinishedEvent with the report and, with showResultMessage, a message box.
 *  One export at a time per grid.
 */
class GUCEF_GUI_PUBLIC_CPP CGridCsvExportController : public CORE::CObserver
{
    public:

    typedef GUCEF::vector< TGridExportTargetPtr > TGridExportTargetVector;

    /** How the running or the last export is written */
    enum EExportMode
    {
        EXPORTMODE_NONE = 0     ,
        EXPORTMODE_IMMEDIATE    ,
        EXPORTMODE_SLICED       ,
        EXPORTMODE_THREADPOOL
    };

    explicit CGridCsvExportController( CGridView& grid );

    virtual ~CGridCsvExportController();

    CGridCsvOptions& GetOptions( void );

    const CGridCsvOptions& GetOptions( void ) const;

    /** A place to export to, a root of the dialog */
    void AddTarget( const TGridExportTargetPtr& target );

    void ClearTargets( void );

    /** The targets; the file system of the computer is added at the first Open() when there is none */
    const TGridExportTargetVector& GetTargets( void ) const;

    /** The name the dialog starts with: suggestedFileName of the options, else the name of the grid, else "table"; made safe for a file name, with ".csv" */
    CString GetSuggestedFileName( void ) const;

    /** A name without separators, control characters, the characters Windows does not take in names, leading or trailing dots and blanks; at most 100 bytes */
    static CString SanitizeFileName( const CString& name );

    /** Opens the file save dialog; false when an export is running or the grid has no context */
    bool Open( void );

    bool IsDialogOpen( void ) const;

    /** The dialog of the last Open(), NULL before */
    CFileSaveDialogForm* GetDialog( void );

    /** The message box the outcome is told in, NULL until one was told */
    CMessageBoxForm* GetMessageBox( void );

    /** Exports the view of the grid to a path of a target, without the dialog: at once, in slices or on a pool as for an accepted dialog */
    bool ExportTo( const TGridExportTargetPtr& target, const CString& path );

    bool IsExporting( void ) const;

    /** Stops the running export; nothing is kept of the file. A pool export ends when its task does */
    void CancelExport( void );

    /** How the running or the last export was written */
    EExportMode GetExportMode( void ) const;

    const CGridCsvExportReport& GetLastReport( void ) const;

    virtual const CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CGridCsvExportController > TEventCallback;

    void OnDialogAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDialogCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelRowsChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void StartListening( bool toModel );
    void StopListening( void );
    TGridExportTargetPtr FindTarget( const CORE::IFileSystemAccess* access ) const;
    void Complete( CGridCsvExportReport& report );
    void Tell( const CGridCsvExportReport& report );
    bool Fail( const CString& message );
    void DestroyOwnedForm( CForm* form );

    CGridCsvExportController( const CGridCsvExportController& src );
    CGridCsvExportController& operator=( const CGridCsvExportController& src );

    private:

    CGridView& m_grid;
    CGridCsvOptions m_options;
    TGridExportTargetVector m_targets;
    CFileSaveDialogForm* m_dialog;
    CMessageBoxForm* m_messageBox;

    // the running export
    EExportMode m_mode;
    bool m_exporting;
    bool m_cancelRequested;
    bool m_rowsChanged;
    TGridExportTargetPtr m_target;
    CString m_rootName;
    CString m_path;
    CORE::IOAccessPtr m_stream;
    TGridExportSourcePtr m_source;
    CGridCsvExporter m_exporter;
    CORE::CFutureResult m_future;
    const CORE::CNotifier* m_frameNotifier;     /**< the context subscribed to for frames (compared, never used: the subscription ends with either side) */
    TTableViewModelPtr m_listenedModel;         /**< the model listened to for changes of its rows while a sliced export runs */
    CGridCsvExportReport m_lastReport;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GUI_CGRIDCSVEXPORTCONTROLLER_H ? */
