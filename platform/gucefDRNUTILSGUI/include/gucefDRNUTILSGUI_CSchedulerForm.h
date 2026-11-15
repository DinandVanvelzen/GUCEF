/*
 *  gucefDRNUTILSGUI: GUCEF module providing the forms that view and control job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILSGUI_CSCHEDULERFORM_H
#define GUCEF_DRNUTILSGUI_CSCHEDULERFORM_H

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

#ifndef GUCEF_DRNUTILS_CSCHEDULERTABLESET_H
#include "gucefDRNUTILS_CSchedulerTableSet.h"
#define GUCEF_DRNUTILS_CSCHEDULERTABLESET_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERTABLESET_H ? */

#ifndef GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#include "gucefDRNUTILS_CSchedulerClient.h"
#define GUCEF_DRNUTILS_CSCHEDULERCLIENT_H
#endif /* GUCEF_DRNUTILS_CSCHEDULERCLIENT_H ? */

#ifndef GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H
#include "gucefDRNUTILSGUI_CTimeDisplaySettings.h"
#define GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H
#endif /* GUCEF_DRNUTILSGUI_CTIMEDISPLAYSETTINGS_H ? */

#ifndef GUCEF_DRNUTILSGUI_CTABLEGRIDBINDING_H
#include "gucefDRNUTILSGUI_CTableGridBinding.h"
#define GUCEF_DRNUTILSGUI_CTABLEGRIDBINDING_H
#endif /* GUCEF_DRNUTILSGUI_CTABLEGRIDBINDING_H ? */

#ifndef GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H
#include "gucefDRNUTILSGUI_CControlConfirmForm.h"
#define GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H
#endif /* GUCEF_DRNUTILSGUI_CCONTROLCONFIRMFORM_H ? */

#ifndef GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H
#include "gucefDRNUTILSGUI_CJobRunHistoryForm.h"
#define GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H
#endif /* GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILSGUI {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The scheduler view of a node: tabs Jobs, Lanes and Runs (grids bound to the tables of a CSchedulerTableSet), a tool
 *  bar with Run now, Cancel run, Enable and Disable, a details panel of the selected job (Scheduler/GetJob) and a status
 *  line (host, revision age, connection, displayed time zone). The buttons follow the allowed actions of GetInfo and of
 *  the selected row; commands go through the CSchedulerClient (never blocking) after the CControlConfirmForm. The
 *  execution target (main thread or pool) and task type of jobs and runs are columns of the grids.
 *  The grids show the tables with CTimeDisplayTableModel: times in the display zone of the time display settings, with the
 *  zone in every cell; cell values, sorting and filters stay UTC.
 *  Pause and resume are not in this version.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CSchedulerForm : public GUI::CFormEx
{
    public:

    static const CORE::CString ClassTypeName;

    static const CORE::CEvent ClosedEvent;          /**< no data */

    static void RegisterEvents( void );

    enum ETab
    {
        TAB_JOBS    = 0 ,
        TAB_LANES   = 1 ,
        TAB_RUNS    = 2
    };
    typedef enum ETab TTab;

    CSchedulerForm( void );

    CSchedulerForm( GUI::CIGUIContext* context );

    virtual ~CSchedulerForm() GUCEF_VIRTUAL_OVERRIDE;

    /** Makes the window, hidden */
    bool Build( void );

    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    void SetTables( const DRNUTILS::CSchedulerTableSet& tables );

    const DRNUTILS::CSchedulerTableSet& GetTables( void ) const;

    /** Not owned; must outlive the form or be taken away with GUCEF_NULL first */
    void SetClient( DRNUTILS::CSchedulerClient* client );

    DRNUTILS::CSchedulerClient* GetClient( void ) const;

    void SetTimeDisplay( const CTimeDisplaySettings& settings );

    const CTimeDisplaySettings& GetTimeDisplay( void ) const;

    void ShowTab( TTab tab );

    TTab GetShownTab( void ) const;

    void SelectJob( const CORE::CString& jobId );

    CORE::CString GetSelectedJobId( void ) const;

    void SelectRun( UInt64 runId );

    UInt64 GetSelectedRunId( void ) const;

    /** Whether the button of the action is enabled for the current selection (allowed actions of the host and the row) */
    bool IsActionEnabled( CORE::TJobControlAction action ) const;

    /** As the buttons do, after confirmation; false when the action is not enabled or the command was not sent */
    bool RunSelectedJobNow( const CORE::CString& reason );
    bool CancelSelectedRun( const CORE::CString& reason );
    bool SetSelectedJobEnabled( bool enabled, const CORE::CString& reason );

    /** The status line: host, state, revision, connection and the displayed time zone */
    CORE::CString GetStatusText( void ) const;

    /** What became of the last command the form sent: "Run now of job-1: done" or "Run now of job-1 was refused: ..." (empty before the first) */
    const CORE::CString& GetLastCommandText( void ) const;

    /**
     *  The details panel: every shown column of the selected job as "Title: text", times in the display zone with their
     *  UTC value, then what Scheduler/GetJob told about the job (CSchedulerClient::RequestJobDetail(), asked when the
     *  selected job changes): description, the actions this viewer may use, the recent runs and the work source's status,
     *  or why the node refused; empty without selection
     */
    CORE::CString GetJobDetailsText( void ) const;

    /** The last answer of Scheduler/GetJob for the selected job; false while none arrived for it */
    bool GetSelectedJobDetail( DRNUTILS::SSchedulerJobDetailAnswer& detail ) const;

    /** The grid bindings, for tests and for a host application that adds its own columns or filters */
    CTableGridBinding& GetJobsBinding( void );
    CTableGridBinding& GetLanesBinding( void );
    CTableGridBinding& GetRunsBinding( void );

    /** The confirmation form every control action asks first; made with the form, with the same context */
    CControlConfirmForm* GetConfirmForm( void );

    /** The runs of the selected job in their own form (made on first use); false when no job is selected or the form cannot be built */
    bool OpenRunHistory( void );

    CJobRunHistoryForm* GetRunHistoryForm( void );

    /** The host info as the form has it: from the last InfoReceivedEvent of the client */
    const DRNUTILS::SSchedulerHostInfo& GetHostInfo( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CSchedulerForm > TEventCallback;

    CSchedulerForm( const CSchedulerForm& src );                /**< not implemented */
    CSchedulerForm& operator=( const CSchedulerForm& src );     /**< not implemented */

    void OnInfoReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCommandCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJobDetailReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionOrModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnActiveTabChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRunNowActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelRunActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEnableActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDisableActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnHistoryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConfirmed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );
    void BindTables( void );
    void Refresh( void );
    void UpdateButtons( void );
    void SetLabelText( const char* widgetName, const CORE::CString& text );

    /** Asks the client for the detail of the selected job when that changed since the last request */
    void RequestSelectedJobDetail( bool force );

    /** Asks for the confirmation of the action on the selected target; the command is sent when it is given */
    bool AskToConfirm( CORE::TJobControlAction action );

    /** The state of the job or the run in the table, empty when there is none */
    CORE::CString GetSelectedJobState( void ) const;
    CORE::CString GetSelectedRunState( void ) const;

    bool IsActionAllowedByHost( CORE::TJobControlAction action ) const;

    /** Builds the command, calls the client (which answers asynchronously) and remembers it for the answer */
    bool SendCommand( CORE::TJobControlAction action, CORE::TJobControlTargetKind targetKind, const CORE::CString& targetId, const CORE::CString& reason );

    CORE::CString DescribeAction( CORE::TJobControlAction action ) const;

    DRNUTILS::CSchedulerTableSet m_tables;
    DRNUTILS::CSchedulerClient* m_client;
    DRNUTILS::SSchedulerHostInfo m_hostInfo;
    CTimeDisplaySettings m_timeDisplay;
    CTableGridBinding m_jobsBinding;
    CTableGridBinding m_lanesBinding;
    CTableGridBinding m_runsBinding;
    CControlConfirmForm* m_confirmForm;
    CJobRunHistoryForm* m_historyForm;
    CORE::TJobControlAction m_pendingAction;
    CORE::TJobControlTargetKind m_pendingTargetKind;
    CORE::CString m_pendingTargetId;
    CORE::CString m_pendingText;            /**< "Run now of job-1": what the pending or last command was */
    CORE::CString m_sentCommandId;
    CORE::CString m_lastCommandText;
    CORE::CString m_detailJobId;            /**< the job of the last RequestJobDetail() */
    CORE::CString m_detailRequestId;        /**< its request id; answers with another id are ignored */
    bool m_haveDetail;
    DRNUTILS::SSchedulerJobDetailAnswer m_detail;
    bool m_open;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILSGUI */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILSGUI_CSCHEDULERFORM_H ? */
