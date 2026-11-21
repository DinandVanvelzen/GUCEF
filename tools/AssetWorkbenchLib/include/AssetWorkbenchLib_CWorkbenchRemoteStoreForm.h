/*
 *  AssetWorkbenchLib: the settings page Remote store: finding, pairing with and connecting to a remote asset store service
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHREMOTESTOREFORM_H
#define ASSETWORKBENCHLIB_CWORKBENCHREMOTESTOREFORM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GUI_CSETTINGSPAGEFORM_H
#include "gucefGUI_CSettingsPageForm.h"
#define GUCEF_GUI_CSETTINGSPAGEFORM_H
#endif /* GUCEF_GUI_CSETTINGSPAGEFORM_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#include "AssetWorkbenchLib_CWorkbenchPreferences.h"
#define ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHPREFERENCES_H ? */

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H
#include "AssetWorkbenchLib_CWorkbenchStoreConnection.h"
#define ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H
#endif /* ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H ? */

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
 *  The settings page Remote store: what the owner needs to use the asset store service of a server (the k3s service) from the workbench.
 *  It is the view of a CWorkbenchStoreConnection and holds no connection logic of its own:
 *
 *    - The state of the connection in words (disconnected, discovering, pairing, connecting, connected, offline) and the error in plain words.
 *    - The services that were found on the local network (the list; choosing one makes its address the address) and Find again.
 *    - The address (host:port of the session listener; worker.lan:47400 by default, kept in the preferences) and the switch for the look
 *      on the local network, which is on by default.
 *    - Connect and Disconnect.
 *    - The pairing: the one-time code that the server shows (a secret input: the code is taken out of the field when Pair is pressed, the field is
 *      emptied and the code is never shown again, logged or kept) and Pair, which pairs and then connects.
 *    - What the service is (its name, version, the roles of this computer, what it offers) while connected.
 *
 *  Widgets (by name): StateText, ErrorText, Services, ServerInfo, EndpointLabel, Endpoint, FindAgain, LanDiscovery, Connect, Disconnect, PairLabel,
 *  PairingCode, Pair, PairHint. The address and the switch are applied to the preferences with Apply or OK of the settings window; the buttons use
 *  what the widgets show at once.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchRemoteStoreForm : public GUI::CSettingsPageForm
{
    public:

    static const CORE::CString PageTitle;                   /**< "Remote store" */

    CWorkbenchRemoteStoreForm( void );

    virtual ~CWorkbenchRemoteStoreForm() GUCEF_VIRTUAL_OVERRIDE;

    /** The preferences the page shows and changes at Apply (not owned) */
    void SetPreferences( CWorkbenchPreferences* preferences );

    /** The connection the page shows and drives (not owned; NULL while the sources are not made) */
    void SetConnection( CWorkbenchStoreConnection* connection );

    /** @name What the buttons do, for tests and for the application */
    ///@{
    bool RequestFind( void );
    bool RequestConnect( void );
    bool RequestDisconnect( void );

    /** Takes the code out of the field (which is emptied) and pairs; false without a code or when something else is going on */
    bool RequestPair( void );
    ///@}

    /** @name What a person reads and types, for tests */
    ///@{
    CORE::CString GetStateText( void );
    CORE::CString GetErrorText( void );
    CORE::CString GetServerInfoText( void );
    CORE::CString GetEndpointText( void );
    bool SetEndpointText( const CORE::CString& endpoint );
    UInt32 GetServiceCount( void ) const;
    CORE::CString GetServiceLine( UInt32 index ) const;
    bool SelectServiceLine( UInt32 index );
    bool SetPairingCode( const CORE::CString& code );
    bool HasPairingCode( void );
    bool IsLanDiscoveryChecked( void );
    ///@}

    virtual CORE::CString GetTitle( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void ReadSettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ApplySettings( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void BuildLayout( CORE::CDataNode& layout ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnPostLayoutLoad( void ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchRemoteStoreForm > TEventCallback;

    void OnFindActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConnectActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnDisconnectActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPairActivated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnServicesSelectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnConnectionChanged( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnFrameStarted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void ShowState( void );
    void ShowServices( void );
    void UseEndpointOfWidgets( void );
    void SetTextIfChanged( const char* widgetName, const CORE::CString& text, CORE::CString& cache );

    CWorkbenchRemoteStoreForm( const CWorkbenchRemoteStoreForm& src );
    CWorkbenchRemoteStoreForm& operator=( const CWorkbenchRemoteStoreForm& src );

    private:

    CWorkbenchPreferences* m_preferences;
    CWorkbenchStoreConnection* m_connection;
    CORE::CString m_shownState;
    CORE::CString m_shownError;
    CORE::CString m_shownInfo;
    CORE::CString::StringVector m_serviceLines;
    UInt32 m_shownStateNumber;
    bool m_showing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHREMOTESTOREFORM_H ? */
