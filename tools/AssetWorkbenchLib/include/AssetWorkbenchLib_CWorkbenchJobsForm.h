/*
 *  AssetWorkbenchLib: views and controllers of the asset workbench
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H
#define ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

#ifndef GUCEF_GUI_CREPORTVIEWFORM_H
#include "gucefGUI_CReportViewForm.h"
#define GUCEF_GUI_CREPORTVIEWFORM_H
#endif /* GUCEF_GUI_CREPORTVIEWFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#include "AssetWorkbenchLib_CAssetStoreTransferModel.h"
#define ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#endif /* ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#include "AssetWorkbenchLib_CWorkbenchJobFilter.h"
#define ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBFILTER_H ? */

#ifndef ASSETWORKBENCHLIB_MACROS_H
#include "AssetWorkbenchLib_macros.h"
#define ASSETWORKBENCHLIB_MACROS_H
#endif /* ASSETWORKBENCHLIB_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The jobs page of the workbench (the jobs overview of the investment services, on the asset store service): one list of the jobs of
 *  the service the workbench works with, in this process or remote (resumable by id: the jobs are the server's), and of the long
 *  operations of the workbench itself, with the columns kind, scope, state, progress, started, ended, duration and result; the
 *  filters state and kind (the state group "finished" is the run history); the buttons Cancel, Resume, Retry and Remove, which apply
 *  to the selected job as far as its job says they do, Report (the result of a finished job, per asset, in the report view of gucefGUI)
 *  and Open (the folder of the result of a finished job).
 *
 *  The form shows what CAssetStoreTransferModel::GetJobs() lists, read again at most every RefreshIntervalInMs of frame time while the
 *  model said that something changed, and every ActiveRefreshIntervalInMs while a job is active (its duration runs). It reads the
 *  grid in string mode, so a list of some hundreds of jobs is cheap.
 *
 *  Why not CSchedulerForm of gucefDRNUTILSGUI: that form is the view of a scheduler node over the DRN scheduler protocol (its tables and
 *  its CSchedulerClient); the jobs of the asset store service are CAssetJobEntry values with their own actions and events. The form
 *  takes the shape of it (a grid with a tool bar, a status line, details, the run history, a confirm before a command) and the
 *  gucefGUI controls, and keeps the time in UTC as the shared forms do.
 *
 *  Widgets (by name): StateFilter, KindFilter, Cancel, Resume, Retry, Remove, Report, Open, Close, Jobs (the grid), Status, Details.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchJobsForm : public GUI::CFormEx
{
    public:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchJobsForm > TEventCallback;

    static const CORE::CEvent ClosedEvent;                  /**< no data */
    static const CORE::CEvent OpenResultRequestedEvent;     /**< TCloneableString: the id of the job whose result folder was asked for */
    static const CORE::CEvent ReportShownEvent;             /**< TCloneableString: the id of the job whose report is in the report form now */
    static void RegisterEvents( void );

    enum EColumn
    {
        COLUMN_KIND = 0 ,
        COLUMN_SCOPE    ,
        COLUMN_STATE    ,
        COLUMN_PROGRESS ,
        COLUMN_STARTED  ,
        COLUMN_ENDED    ,
        COLUMN_DURATION ,
        COLUMN_RESULT   ,
        COLUMN_COUNT
    };

    enum EDefaults
    {
        RefreshIntervalInMs = 200       ,
        ActiveRefreshIntervalInMs = 500
    };

    CWorkbenchJobsForm( void );

    explicit CWorkbenchJobsForm( GUI::CIGUIContext* context );

    virtual ~CWorkbenchJobsForm() GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the window, hidden */
    bool Build( void );

    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** The list (not owned); it has to outlive the form or be taken away with GUCEF_NULL first */
    void SetModel( CAssetStoreTransferModel* model );

    /** The service whose results the report reads (not owned) */
    void SetService( ASSETSTORE::CIAssetStoreService* service );

    /*
     *  The filters
     */

    void SetStateGroup( CWorkbenchJobFilter::EStateGroup group );

    void SetKindFilter( const CORE::CString& kind );

    const CWorkbenchJobFilter& GetFilter( void ) const;

    /*
     *  The rows
     */

    /** Lets frame time pass, which reads the list again when it is due; false when nothing was read */
    bool Update( UInt32 deltaInMs );

    /** Reads the list and shows it now */
    void Refresh( void );

    UInt32 GetShownJobCount( void ) const;

    bool GetShownJob( UInt32 row, ASSETSTORE::CAssetJobEntry& job ) const;

    /** The text of a cell of a shown row, as the grid shows it */
    CORE::CString GetCellText( UInt32 row, EColumn column ) const;

    bool SelectJob( const CORE::CString& jobId );

    CORE::CString GetSelectedJobId( void ) const;

    /*
     *  What the buttons do, for the selected job
     */

    bool IsActionEnabled( ASSETSTORE::TAssetJobAction action ) const;

    bool CancelSelected( void );
    bool ResumeSelected( void );
    bool RetrySelected( void );
    bool RemoveSelected( void );

    /** Whether the selected job has a report to show (a finished job of the pipeline) */
    bool IsReportAvailable( void ) const;

    /** Reads the result of the selected job and shows it per asset in the report form; false when there is none (the status line says why) */
    bool ShowReport( void );

    /** The report form (made on first use) */
    GUI::CReportViewForm* GetReportForm( void );

    /** The job whose report the form shows */
    const CORE::CString& GetReportJobId( void ) const;

    bool OpenSelectedResult( void );

    /** The status line (what is shown of what is there, the counts) and the details of the selected job */
    CORE::CString GetStatusText( void ) const;
    CORE::CString GetDetailsText( void ) const;

    static CORE::CString FormatDuration( UInt64 milliseconds );
    static CORE::CString FormatTime( Int64 unixMilliseconds );
    static CORE::CString GetColumnTitle( EColumn column );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CWorkbenchJobsForm( const CWorkbenchJobsForm& src );
    CWorkbenchJobsForm& operator=( const CWorkbenchJobsForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    void OnModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFilterChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResumeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRetryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRemoveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnReportActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnOpenActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void ShowChoices( void );
    void ShowRows( void );
    void ShowStatusAndButtons( void );
    bool Control( ASSETSTORE::TAssetJobAction action );
    void TakeReport( const ASSETSTORE::CAssetServiceDocument& result, const CORE::CString& jobId );
    const ASSETSTORE::CAssetJobEntry* FindShown( const CORE::CString& jobId ) const;
    void MakeRow( const ASSETSTORE::CAssetJobEntry& job, GUCEF::vector< CORE::CString >& cells ) const;

    private:

    CAssetStoreTransferModel* m_model;
    ASSETSTORE::CIAssetStoreService* m_service;
    CWorkbenchJobFilter m_filter;
    ASSETSTORE::TAssetJobEntryVector m_shown;
    GUCEF::vector< GUCEF::vector< CORE::CString > > m_rows;
    CORE::CString m_selectedId;
    CORE::CString m_lastOutcome;
    CORE::CString m_reportJobId;
    GUI::CReportViewForm* m_reportForm;
    CORE::CFutureResult m_reportFuture;
    CORE::CString m_reportFutureJobId;
    UInt32 m_waitedInMs;
    bool m_dirty;
    bool m_hasActive;
    bool m_open;
    bool m_updatingWidgets;
    bool m_frameSubscribed;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHJOBSFORM_H ? */
