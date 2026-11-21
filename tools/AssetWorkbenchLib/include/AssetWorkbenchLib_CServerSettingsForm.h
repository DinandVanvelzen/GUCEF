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

#ifndef ASSETWORKBENCHLIB_CSERVERSETTINGSFORM_H
#define ASSETWORKBENCHLIB_CSERVERSETTINGSFORM_H

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
#ifndef GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#include "gucefGUI_CPropertyInspectorForm.h"
#define GUCEF_GUI_CPROPERTYINSPECTORFORM_H
#endif /* GUCEF_GUI_CPROPERTYINSPECTORFORM_H ? */
#ifndef ASSETWORKBENCHLIB_CSERVERSETTINGSCONTROLLER_H
#include "AssetWorkbenchLib_CServerSettingsController.h"
#define ASSETWORKBENCHLIB_CSERVERSETTINGSCONTROLLER_H
#endif /* ASSETWORKBENCHLIB_CSERVERSETTINGSCONTROLLER_H ? */
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
 *  The settings of the asset store service the workbench is connected to, as a page (gucefASSETSTORE_RemoteService_Design.md 18.5):
 *  a property list (GUI::CPropertyInspectorForm) with a group per group of the settings and a group per online store with the members
 *  of its sync profile, each value with the editor of its type (a check box, a number in its range, a choice, a text) from the
 *  descriptors of the service. The line below the list tells the selected setting: its value, where it comes from (site, overridden,
 *  default), its site value and default, its range and whether a change applies at once or after a restart. Apply sends the edited
 *  value of the selected row, Reset to site value puts the selected setting back to the value of the site settings of the server. When
 *  the server does not let clients change settings, or this device lacks the role, every row is read only and the notice at the top
 *  says why. The keys of the rows of the sync profiles are "sync:<store id>:<member>".
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CServerSettingsForm : public GUI::CFormEx
{
    public:

    typedef CORE::CTEventHandlerFunctor< CServerSettingsForm > TEventCallback;

    static const CORE::CEvent ClosedEvent;          /**< no data */

    static void RegisterEvents( void );

    enum EDefaults
    {
        RefreshIntervalInMs = 100
    };

    CServerSettingsForm( void );

    explicit CServerSettingsForm( GUI::CIGUIContext* context );

    virtual ~CServerSettingsForm() GUCEF_VIRTUAL_OVERRIDE;

    bool Build( void );

    /** Shows the form and reads the settings of the service again */
    bool Open( void );

    void Close( void );

    bool IsOpen( void ) const;

    /** The controller of the service (not owned); GUCEF_NULL when the workbench is not connected to a remote service */
    void SetController( CServerSettingsController* controller );

    /** Puts the cursor on the row of the key; false for a key that is not shown */
    bool SelectKey( const CORE::CString& key );

    /** The key of the row with the cursor, empty when it is on a group or nowhere */
    CORE::CString GetSelectedKey( void ) const;

    /** Edits the value of a row as the editor of the row does (not sent yet); false with the reason for a value that does not fit or a row that is read only */
    bool EditValue( const CORE::CString& key, const CORE::CString& text, CORE::CString* problem = GUCEF_NULL );

    /** Sends the edited value of the selected row; false when there is nothing to send or it cannot be asked */
    bool ApplySelected( void );

    /** Puts the selected setting back to the value of the site settings of the server */
    bool ResetSelected( void );

    bool RefreshNow( void );

    /** Lets frame time pass: takes the answers of the controller and shows the rows again; true when the form was refreshed */
    bool Update( UInt32 deltaInMs );

    /*
     *  What is shown
     */

    /** The keys of the rows, in the order shown */
    CORE::CString::StringVector GetKeys( void ) const;

    /** The value of the row of the key as shown (with an edit that is not sent yet) */
    CORE::CString GetValueText( const CORE::CString& key ) const;

    /** "site", "overridden" or "default"; empty for a row of a sync profile */
    CORE::CString GetSourceText( const CORE::CString& key ) const;

    /** "live" or "restart"; empty for a row of a sync profile */
    CORE::CString GetApplyModeText( const CORE::CString& key ) const;

    bool IsKeyEditable( const CORE::CString& key ) const;

    /** Whether this device may change the settings of the server now */
    bool IsEditingAllowed( void ) const;

    CORE::CString GetNoticeText( void ) const;
    CORE::CString GetDetailsText( void ) const;
    CORE::CString GetOutcomeText( void ) const;

    /** The property list of the form, GUCEF_NULL before it is built */
    GUI::CPropertyInspectorForm* GetPropertyList( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    /** What a row of the list is */
    struct SRow
    {
        CORE::CString key;
        CORE::CString path;                 /**< the path of the value in the tree of the list */
        CORE::CString storeId;              /**< a member of a sync profile: its store */
        CORE::CString member;               /**< a member of a sync profile: its name */
        bool editable;

        SRow( void );
    };
    typedef GUCEF::vector< SRow > TRowVector;

    CServerSettingsForm( const CServerSettingsForm& src );              /**< not implemented */
    CServerSettingsForm& operator=( const CServerSettingsForm& src );   /**< not implemented */

    void BuildLayout( CORE::CDataNode& layout );
    void Rebuild( void );
    void ShowTexts( void );
    void ShowButtons( void );
    void SetText( const char* widgetName, const CORE::CString& text );
    const SRow* FindRow( const CORE::CString& key ) const;
    const SRow* FindRowByPath( const CORE::CString& path ) const;
    const ASSETSTORE::CAssetSettingState* FindState( const CORE::CString& key ) const;
    static GUI::CPropertyRowDescriptor MakeRowDescriptor( const ASSETSTORE::CAssetSettingDescriptor& descriptor );

    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnControllerRead( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnControllerOutcome( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPropertyChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnApplyActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnResetActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRefreshActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnCloseActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnGridSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    private:

    CServerSettingsController* m_controller;
    GUI::CPropertyInspectorForm* m_list;
    TRowVector m_rows;
    CORE::CString::StringMap m_edits;               /**< the edited values not sent yet, by key */
    CORE::CString m_notice;
    CORE::CString m_outcome;
    CORE::CString m_details;
    UInt32 m_waitedInMs;
    bool m_open;
    bool m_dirty;
    bool m_frameSubscribed;
    bool m_rebuilding;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CSERVERSETTINGSFORM_H ? */
