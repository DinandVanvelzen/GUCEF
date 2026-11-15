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

#ifndef GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H
#define GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATATABLE_H
#include "gucefCORE_CIDataTable.h"
#define GUCEF_CORE_CIDATATABLE_H
#endif /* GUCEF_CORE_CIDATATABLE_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_GUI_CFORMEX_H
#include "gucefGUI_CFormEx.h"
#define GUCEF_GUI_CFORMEX_H
#endif /* GUCEF_GUI_CFORMEX_H ? */

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
 *  The runs of one job (the runs table filtered on jobId, the newest run first): progress of running runs, outcome, task id,
 *  execution target and pool; Cancel run for a selected active run, after the confirmation.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CJobRunHistoryForm : public GUI::CFormEx
{
    public:

    static const CORE::CString ClassTypeName;

    static const CORE::CEvent ClosedEvent;          /**< no data */

    static void RegisterEvents( void );

    CJobRunHistoryForm( void );

    CJobRunHistoryForm( GUI::CIGUIContext* context );

    virtual ~CJobRunHistoryForm() GUCEF_VIRTUAL_OVERRIDE;

    bool Build( void );

    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    void SetRunsTable( const CORE::CIDataTablePtr& runsTable );

    void SetJobId( const CORE::CString& jobId );

    const CORE::CString& GetJobId( void ) const;

    /** Not owned */
    void SetClient( DRNUTILS::CSchedulerClient* client );

    void SetTimeDisplay( const CTimeDisplaySettings& settings );

    /** The allowed actions of the host, when the application (or the scheduler form that opens this one) knows them better than the client's last info */
    void SetHostInfo( const DRNUTILS::SSchedulerHostInfo& hostInfo );

    void SelectRun( UInt64 runId );

    UInt64 GetSelectedRunId( void ) const;

    /** The step after the confirmation; false when the run is not active, the host does not allow it or the command was not sent */
    bool CancelSelectedRun( const CORE::CString& reason );

    /** Whether the Cancel run button is enabled: an active run is selected and the host allows cancelling */
    bool IsCancelEnabled( void ) const;

    /** The progress of the selected run as the form shows it: "12 of 40", empty when the run has no total */
    CORE::CString GetProgressText( void ) const;

    /** The progress of the selected run as a percentage, 0 when it has no total */
    Float32 GetProgressPercentage( void ) const;

    const CORE::CString& GetLastCommandText( void ) const;

    CTableGridBinding& GetBinding( void );

    CControlConfirmForm* GetConfirmForm( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CJobRunHistoryForm > TEventCallback;

    void OnInfoReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCommandCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionOrModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelRunActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConfirmed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );
    void BindTable( void );
    void Refresh( void );
    void SetLabelText( const char* widgetName, const CORE::CString& text );
    CORE::CString GetSelectedRunState( void ) const;

    CJobRunHistoryForm( const CJobRunHistoryForm& src );                /**< not implemented */
    CJobRunHistoryForm& operator=( const CJobRunHistoryForm& src );     /**< not implemented */

    CORE::CIDataTablePtr m_runsTable;
    CORE::CString m_jobId;
    DRNUTILS::CSchedulerClient* m_client;
    DRNUTILS::SSchedulerHostInfo m_hostInfo;
    CTimeDisplaySettings m_timeDisplay;
    CTableGridBinding m_binding;
    CControlConfirmForm* m_confirmForm;
    UInt64 m_pendingRunId;
    CORE::CString m_sentCommandId;
    CORE::CString m_pendingText;
    CORE::CString m_lastCommandText;
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

#endif /* GUCEF_DRNUTILSGUI_CJOBRUNHISTORYFORM_H ? */
