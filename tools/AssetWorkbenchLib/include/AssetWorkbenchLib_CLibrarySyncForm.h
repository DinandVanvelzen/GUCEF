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

#ifndef ASSETWORKBENCHLIB_CLIBRARYSYNCFORM_H
#define ASSETWORKBENCHLIB_CLIBRARYSYNCFORM_H

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

#ifndef ASSETWORKBENCHLIB_CLIBRARYSYNCCONTROLLER_H
#include "AssetWorkbenchLib_CLibrarySyncController.h"
#define ASSETWORKBENCHLIB_CLIBRARYSYNCCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CLIBRARYSYNCCONTROLLER_H ? */

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
 *  The panel of the library sync (library sync design, section 7.4): one row per online store of the service with its sign in, the
 *  counts of its last plan (missing, stale, current, unavailable), the estimate of the download against the free disk, the progress
 *  of its sync and the summary of its last run. For the selected store: Plan (the dry run), Sync now (starts at once, as the owner
 *  decided: no confirmation), Cancel and Retry failed; a progress bar with the counts and the item queued last, and the notice when a
 *  sync ends. The work is the CLibrarySyncController's (not owned); the form only shows it. Built from gucefGUI widgets only, so
 *  every driver shows it.
 *
 *  Widgets (by name): Stores (the grid), Plan, SyncNow, Cancel, RetryFailed, Refresh, Close, Progress (the bar), ProgressText, Notice.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CLibrarySyncForm : public GUI::CFormEx
{
    public:

    typedef CORE::CTEventHandlerFunctor< CLibrarySyncForm > TEventCallback;

    static const CORE::CEvent ClosedEvent;          /**< no data */

    static void RegisterEvents( void );

    /** The columns of the grid */
    enum EColumn
    {
        COLUMN_STORE = 0        ,
        COLUMN_SIGNIN           ,
        COLUMN_MISSING          ,
        COLUMN_STALE            ,
        COLUMN_CURRENT          ,
        COLUMN_UNAVAILABLE      ,
        COLUMN_DOWNLOAD         ,
        COLUMN_FREEDISK         ,
        COLUMN_PROGRESS         ,
        COLUMN_LASTRESULT       ,

        COLUMN_COUNT
    };

    enum EDefaults
    {
        RefreshIntervalInMs = 250
    };

    CLibrarySyncForm( void );

    explicit CLibrarySyncForm( GUI::CIGUIContext* context );

    virtual ~CLibrarySyncForm() GUCEF_VIRTUAL_OVERRIDE;

    bool Build( void );

    /** Shows the form and reads the stores of the service again */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** The controller of the service (not owned) */
    void SetController( CLibrarySyncController* controller );

    /** The store the buttons act on; false for a store that is not listed */
    bool SelectStore( const CORE::CString& storeId );

    CORE::CString GetSelectedStoreId( void ) const;

    /*
     *  What the buttons do, for the selected store
     */

    bool PlanNow( void );
    bool SyncNow( void );
    bool CancelSync( void );
    bool RetryFailed( void );
    bool RefreshNow( void );

    /** Lets frame time pass: takes the answers of the controller and shows the rows again; true when the form was refreshed */
    bool Update( UInt32 deltaInMs );

    /*
     *  What is shown
     */

    UInt32 GetRowCount( void ) const;
    CORE::CString GetCellText( UInt32 row, UInt32 column ) const;
    CORE::CString GetProgressText( void ) const;
    Float32 GetProgressPercent( void ) const;
    CORE::CString GetNoticeText( void ) const;
    bool IsSyncEnabled( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CLibrarySyncForm( const CLibrarySyncForm& src );              /**< not implemented */
    CLibrarySyncForm& operator=( const CLibrarySyncForm& src );   /**< not implemented */

    void BuildLayout( CORE::CDataNode& layout );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnControllerChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlanActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSyncActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRetryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRefreshActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void ShowRows( void );
    void ShowSelected( void );
    void ShowButtons( void );
    void TakeProblem( bool asked );
    void SetText( const char* widgetName, const CORE::CString& text );
    const CLibrarySyncController::SStoreState* GetSelectedState( void ) const;

    private:

    CLibrarySyncController* m_controller;
    GUCEF::vector< GUCEF::vector< CORE::CString > > m_rows;
    CORE::CString::StringVector m_rowStoreIds;
    CORE::CString m_selectedStoreId;
    CORE::CString m_progressText;
    Float32 m_progressPercent;
    CORE::CString m_notice;
    UInt32 m_waitedInMs;
    bool m_open;
    bool m_dirty;
    bool m_frameSubscribed;
    bool m_updatingWidgets;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CLIBRARYSYNCFORM_H ? */
