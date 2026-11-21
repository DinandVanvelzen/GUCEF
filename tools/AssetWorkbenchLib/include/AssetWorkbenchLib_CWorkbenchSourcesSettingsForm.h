/*
 *  AssetWorkbenchLib: the settings page Sources: the online stores, their sign in and connection test, the keyring guidance and the folder of the store plugins
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSOURCESSETTINGSFORM_H
#define ASSETWORKBENCHLIB_CWORKBENCHSOURCESSETTINGSFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H
#include "gucefASSETSTORE_CAssetStorePluginLoadReport.h"
#define GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREPLUGINLOADREPORT_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#include "AssetWorkbenchLib_CWorkbenchPreferences.h"
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#include "AssetWorkbenchLib_CWorkbenchSignIn.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSIGNIN_H ? */

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
 *  The settings page Sources: what the owner needs to set up and try the online stores.
 *
 *    - The folder of the private store plugins (CWorkbenchPreferences::privatePluginsDir) with the button Load now, which raises
 *      PluginsLoadRequestedEvent for the application to load them (CAssetStorePluginLoader) and to tell the result with SetPluginReport().
 *    - The keyring, for the first run: what there is to do (set the code up, unlock) with the button that opens the window of the sign
 *      in for it; the state follows the keyring.
 *    - A line per online store of the registry (the stores the plugins registered): its name and sign in state; for the selected one
 *      the switch Enabled (kept in the preferences, a store that is off does not show in the sources tab), how it signs in, the state
 *      of the keyring for it, the button Sign in, which starts the sign in of the sources tab, and the button Test connection, which
 *      runs the probe of the store (CIAssetStoreProbe) when it has one and shows the checks without any value of the store.
 *    - The state of the service the stores run in (a remote service: its connection) and, without a store, what to do about that.
 *
 *  Widgets (by name): PluginsDir, LoadPlugins, PluginsStatus, KeyringGuide, KeyringAction, Stores, StoreEnabled, StoreInfo, StoreSignIn,
 *  TestConnection, StoreStatus, ProbeResult, EmptyState.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchSourcesSettingsForm : public GUI::CSettingsPageForm
{
    public:

    static const CORE::CEvent PluginsLoadRequestedEvent;    /**< no data: Load now was pressed, the folder to use is GetPluginsDirFromWidgets() */
    static void RegisterEvents( void );

    static const CORE::CString PageTitle;                   /**< "Sources" */

    CWorkbenchSourcesSettingsForm( void );

    virtual ~CWorkbenchSourcesSettingsForm() GUCEF_VIRTUAL_OVERRIDE;

    /** The preferences the page shows and changes at Apply (not owned) */
    void SetPreferences( CWorkbenchPreferences* preferences );

    /** The sign in of the sources tab whose windows the page opens (not owned; NULL while the sources are not made) */
    void SetSignIn( CWorkbenchSignIn* signIn );

    /** What the load of the plugin folder did, shown in the status line of the folder */
    void SetPluginReport( const ASSETSTORE::CAssetStorePluginLoadReport& report );

    /** A line about the service the stores run in, for example "Remote service: connected"; empty for the local service */
    void SetServiceNote( const CORE::CString& note );

    /** The folder as the edit box has it now */
    CORE::CString GetPluginsDirFromWidgets( void );

    /** The stores of the page: the ids of the registry without the primary store */
    UInt32 GetStoreCount( void ) const;
    CORE::CString GetStoreId( UInt32 index ) const;
    bool SelectStore( const CORE::CString& storeId );
    CORE::CString GetSelectedStoreId( void ) const;

    /** The text of the line of the store in the list */
    CORE::CString GetStoreLine( UInt32 index ) const;

    /** The text a person reads for the selected store, for tests */
    CORE::CString GetStoreInfoText( void );
    CORE::CString GetStoreStatusText( void );
    CORE::CString GetProbeText( void );
    CORE::CString GetEmptyStateText( void );
    CORE::CString GetKeyringGuideText( void );
    CORE::CString GetPluginsStatusText( void );

    /** Whether the store is on as the page has it now (not yet applied) */
    bool IsStoreEnabled( const CORE::CString& storeId ) const;

    /** The text of the empty state: what to do when no online store is installed */
    static CORE::CString GetNoStoreText( void );

    /** The button actions, for tests and for the application: Test connection of the selected store, and whether a probe is running */
    bool StartProbe( void );
    bool IsProbeRunning( void ) const;

    virtual CORE::CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnValueChanged( GUI::CWidget& widget ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchSourcesSettingsForm > TEventCallback;

    void OnLoadPluginsActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyringActionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoreSignInActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnTestConnectionActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnStoresSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnKeyringStateChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnProviderChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void FillStores( void );
    void ShowStoreDetails( void );
    void ShowKeyring( void );
    void ShowPluginsStatus( void );
    void CheckProbe( void );
    CORE::CString MakeStoreLine( const CORE::CString& storeId ) const;
    CORE::CString GetSignInMethodText( const CORE::CString& storeId ) const;
    CORE::CString GetKeyringText( const CORE::CString& storeId ) const;
    bool FindProvider( const CORE::CString& storeId, ASSETSTORE::TAssetStoreProviderPtr& provider ) const;

    CWorkbenchSourcesSettingsForm( const CWorkbenchSourcesSettingsForm& src );
    CWorkbenchSourcesSettingsForm& operator=( const CWorkbenchSourcesSettingsForm& src );

    private:

    CWorkbenchPreferences* m_preferences;
    CWorkbenchSignIn* m_signIn;
    CWorkbenchPreferences m_pending;
    ASSETSTORE::CAssetStorePluginLoadReport m_report;
    CORE::CString m_serviceNote;
    CORE::CString::StringVector m_storeIds;
    CORE::CString m_selectedStore;
    CORE::CString m_probeStore;
    CORE::CString m_probeText;
    CORE::CString m_storeStatus;
    CORE::CFutureResult m_probeFuture;
    bool m_probeRunning;
    bool m_subscribedToKeyring;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHSOURCESSETTINGSFORM_H ? */
