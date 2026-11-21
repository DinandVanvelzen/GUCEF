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

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H
#define ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H

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

#ifndef ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H
#include "AssetWorkbenchLib_CBulkConversionController.h"
#define ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONCONTROLLER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#include "AssetWorkbenchLib_CAssetStoreTransferModel.h"
#define ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H
#endif /* ASSETWORKBENCHLIB_CASSETSTORETRANSFERMODEL_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#include "AssetWorkbenchLib_CWorkbenchPreferences.h"
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */

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
 *  The panel of the bulk conversion (feedback item F8): choose the scope (selection, folder, pack, group of the resource view, saved
 *  search), the targets (the intermediate with its sidecar, the Ogre runtime files, a cook profile) and the options (force reimport,
 *  validate only, cook only, an asset filter, analyse), see the plan (what the pipeline would do per asset, with the reason and the
 *  work, and the licence refusal), start the job on the service, and follow it: the progress of the job, cancel, resume after an
 *  interruption, retry of the failed assets, and the notice when it is done. Named settings are kept as templates in the
 *  preferences. The job list of the workbench has the same job; the results open from there.
 *
 *  Widgets (by name): ScopeKind, Pack, ScopeDetail, Intermediate, OgreRuntime, CookProfile, Force, ValidateOnly, CookOnly, Analyse,
 *  AssetFilter, Templates, TemplateName, SaveTemplate, LoadTemplate, DeleteTemplate, Plan, Start, CancelJob, ResumeJob, RetryFailed,
 *  Close, PlanItems (the grid), PlanSummary, JobStatus, Notice.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CBulkConversionForm : public GUI::CFormEx
{
    public:

    typedef CORE::CTEventHandlerFunctor< CBulkConversionForm > TEventCallback;

    static const CORE::CEvent ClosedEvent;                  /**< no data */
    static const CORE::CEvent TemplatesChangedEvent;        /**< no data: a template was saved or deleted; the application keeps the preferences */
    static void RegisterEvents( void );

    enum EDefaults
    {
        RefreshIntervalInMs = 250
    };

    CBulkConversionForm( void );

    explicit CBulkConversionForm( GUI::CIGUIContext* context );

    virtual ~CBulkConversionForm() GUCEF_VIRTUAL_OVERRIDE;

    bool Build( void );

    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** The controller of the service (not owned); the job list (not owned) gives the progress; the preferences the templates (not owned) */
    void SetController( CBulkConversionController* controller );
    void SetModel( CAssetStoreTransferModel* model );
    void SetPreferences( CWorkbenchPreferences* preferences );

    /** Puts the settings in the widgets; the assets of a selection or group stay with the form */
    void SetSettings( const CBulkConversionSettings& settings );

    /** The settings as the widgets say them now */
    CBulkConversionSettings GetSettings( void ) const;

    /*
     *  What the buttons do
     */

    bool PlanNow( void );
    bool StartNow( void );
    bool CancelJob( void );
    bool ResumeJob( void );
    bool RetryFailed( void );
    bool SaveTemplate( const CORE::CString& name );
    bool LoadTemplate( const CORE::CString& name );
    bool DeleteTemplate( const CORE::CString& name );

    /** Lets frame time pass: takes the answers of the controller and brings the progress up to date; true when the form was refreshed */
    bool Update( UInt32 deltaInMs );

    /** The job whose progress the form shows: the last one the controller started */
    CORE::CString GetCurrentJobId( void ) const;

    /*
     *  What is shown
     */

    CORE::CString GetPlanSummaryText( void ) const;
    CORE::CString GetJobStatusText( void ) const;
    CORE::CString GetNoticeText( void ) const;
    UInt32 GetPlanRowCount( void ) const;
    CORE::CString GetPlanCellText( UInt32 row, UInt32 column ) const;
    bool IsStartEnabled( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    CBulkConversionForm( const CBulkConversionForm& src );
    CBulkConversionForm& operator=( const CBulkConversionForm& src );

    void BuildLayout( CORE::CDataNode& layout );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlanReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnModelChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSettingChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPlanActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStartActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCancelActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResumeActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRetryActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSaveActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnLoadActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDeleteActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void ShowChoices( void );
    void ShowTemplates( void );
    void ShowPlan( void );
    void ShowJob( void );
    void ShowButtons( void );
    CORE::CString GetText( const char* widgetName ) const;
    bool GetChecked( const char* widgetName ) const;
    void SetText( const char* widgetName, const CORE::CString& text );
    void SetChecked( const char* widgetName, bool checked );

    private:

    CBulkConversionController* m_controller;
    CAssetStoreTransferModel* m_model;
    CWorkbenchPreferences* m_preferences;
    CORE::CString::StringVector m_assets;
    CORE::CString m_groupId;
    GUCEF::vector< GUCEF::vector< CORE::CString > > m_planRows;
    CORE::CString m_planText;
    CORE::CString m_jobText;
    CORE::CString m_notice;
    UInt32 m_waitedInMs;
    bool m_open;
    bool m_updatingWidgets;
    bool m_dirty;
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

#endif /* ASSETWORKBENCHLIB_CBULKCONVERSIONFORM_H ? */
