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

#ifndef GUCEF_DRNUTILSGUI_CTASKMANAGERFORM_H
#define GUCEF_DRNUTILSGUI_CTASKMANAGERFORM_H

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

#ifndef GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H
#include "gucefDRNUTILS_CTaskManagerTableSet.h"
#define GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERTABLESET_H ? */

#ifndef GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H
#include "gucefDRNUTILS_CTaskManagerClient.h"
#define GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H
#endif /* GUCEF_DRNUTILS_CTASKMANAGERCLIENT_H ? */

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
 *  The task manager view of a node: tabs Pools, Threads, Tasks and Scheduled jobs (which jobs run on the main thread and
 *  which on which pool), Resize pool (two fields next to the button instead of a dialog), Cancel task, Stop task and a sample
 *  interval selector. While open the form keeps the client viewing (CTaskManagerClient::StartViewing()), so the host samples
 *  and publishes: the threads and the tasks are only sampled while their tab is shown. Closing the form (or taking the client
 *  away) ends the viewing. Every control action asks the confirmation first.
 *  Pause and resume of tasks and pools are not in this version.
 */
class GUCEF_DRNUTILSGUI_PUBLIC_CPP CTaskManagerForm : public GUI::CFormEx
{
    public:

    static const CORE::CString ClassTypeName;

    static const CORE::CEvent ClosedEvent;          /**< no data */

    static void RegisterEvents( void );

    enum ETab
    {
        TAB_POOLS           = 0 ,
        TAB_THREADS         = 1 ,
        TAB_TASKS           = 2 ,
        TAB_SCHEDULED_JOBS  = 3
    };
    typedef enum ETab TTab;

    CTaskManagerForm( void );

    CTaskManagerForm( GUI::CIGUIContext* context );

    virtual ~CTaskManagerForm() GUCEF_VIRTUAL_OVERRIDE;

    bool Build( void );

    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    void SetTables( const DRNUTILS::CTaskManagerTableSet& tables );

    const DRNUTILS::CTaskManagerTableSet& GetTables( void ) const;

    /** Not owned; must outlive the form or be taken away with GUCEF_NULL first */
    void SetClient( DRNUTILS::CTaskManagerClient* client );

    DRNUTILS::CTaskManagerClient* GetClient( void ) const;

    void SetTimeDisplay( const CTimeDisplaySettings& settings );

    void SetSampleIntervalMs( UInt32 sampleIntervalMs );

    UInt32 GetSampleIntervalMs( void ) const;

    void ShowTab( TTab tab );

    TTab GetShownTab( void ) const;

    void SelectPool( const CORE::CString& poolName );

    CORE::CString GetSelectedPool( void ) const;

    void SelectTask( UInt32 taskId );

    UInt32 GetSelectedTaskId( void ) const;

    /** Whether the button of the action is enabled for the current selection (capabilities and controllable pools) */
    bool IsActionEnabled( DRNUTILS::TDrnUtilsTaskControlAction action ) const;

    bool ResizeSelectedPool( UInt32 minWorkers, Int32 maxThreads, const CORE::CString& reason );
    bool CancelSelectedTask( const CORE::CString& reason );
    bool StopSelectedTask( const CORE::CString& reason );

    /** Whether the form keeps the client viewing at this moment (it does while it is open) */
    bool IsViewing( void ) const;

    /** The status line: host, whether the form views, the sample interval and the displayed time zone */
    CORE::CString GetStatusText( void ) const;

    const CORE::CString& GetLastCommandText( void ) const;

    const DRNUTILS::STaskManagerHostInfo& GetHostInfo( void ) const;

    CTableGridBinding& GetPoolsBinding( void );
    CTableGridBinding& GetThreadsBinding( void );
    CTableGridBinding& GetTasksBinding( void );
    CTableGridBinding& GetScheduledJobsBinding( void );

    CControlConfirmForm* GetConfirmForm( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTaskManagerForm > TEventCallback;

    void OnInfoReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCommandCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionOrModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnActiveTabChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnIntervalChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResizeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelTaskActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStopTaskActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConfirmed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNavigation( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BuildLayout( CORE::CDataNode& layout );
    void BindTables( void );
    void Refresh( void );
    void UpdateResizeInputs( void );
    void SetLabelText( const char* widgetName, const CORE::CString& text );

    /** Starts (or restarts) the viewing with what the shown tab needs, ends it when the form is not open */
    void UpdateViewing( bool restart );
    void StopViewing( void );

    bool AskToConfirm( DRNUTILS::TDrnUtilsTaskControlAction action );
    bool SendCommand( const DRNUTILS::STaskManagerControlCommand& command, const CORE::CString& text );
    bool ReadResizeInputs( UInt32& minWorkers, Int32& maxThreads, CORE::CString& error ) const;
    CORE::CString GetSelectedTaskPool( void ) const;

    CTaskManagerForm( const CTaskManagerForm& src );                /**< not implemented */
    CTaskManagerForm& operator=( const CTaskManagerForm& src );     /**< not implemented */

    DRNUTILS::CTaskManagerTableSet m_tables;
    DRNUTILS::CTaskManagerClient* m_client;
    DRNUTILS::STaskManagerHostInfo m_hostInfo;
    CTimeDisplaySettings m_timeDisplay;
    CTableGridBinding m_poolsBinding;
    CTableGridBinding m_threadsBinding;
    CTableGridBinding m_tasksBinding;
    CTableGridBinding m_scheduledJobsBinding;
    CControlConfirmForm* m_confirmForm;
    UInt32 m_sampleIntervalMs;
    DRNUTILS::STaskManagerControlCommand m_pendingCommand;
    bool m_hasPendingCommand;
    CORE::CString m_pendingText;
    CORE::CString m_sentCommandId;
    CORE::CString m_lastCommandText;
    bool m_viewing;
    bool m_viewingThreads;
    bool m_viewingTasks;
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

#endif /* GUCEF_DRNUTILSGUI_CTASKMANAGERFORM_H ? */
