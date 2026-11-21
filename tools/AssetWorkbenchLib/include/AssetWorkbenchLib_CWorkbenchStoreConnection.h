/*
 *  AssetWorkbenchLib: the connection of the workbench to a remote asset store service: finding, pairing, connecting and staying connected
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

#ifndef ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H
#define ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_COMCORE_CSERVICEFINDER_H
#include "gucefCOMCORE_CServiceFinder.h"
#define GUCEF_COMCORE_CSERVICEFINDER_H
#endif /* GUCEF_COMCORE_CSERVICEFINDER_H ? */

#ifndef GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#include "gucefDRNUTILS_CReconnectBackoff.h"
#define GUCEF_DRNUTILS_CRECONNECTBACKOFF_H
#endif /* GUCEF_DRNUTILS_CRECONNECTBACKOFF_H ? */

#ifndef GUCEF_AUTH_CSECRETSTRING_H
#include "gucefAUTH_CSecretString.h"
#define GUCEF_AUTH_CSECRETSTRING_H
#endif /* GUCEF_AUTH_CSECRETSTRING_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#include "gucefASSETSTORE_CIAssetStoreService.h"
#define GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTORESERVICE_H ? */

#ifndef ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H
#include "AssetWorkbenchLib_CStoreConnectionTaskData.h"
#define ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H
#endif /* ASSETWORKBENCHLIB_CSTORECONNECTIONTASKDATA_H ? */

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
 *  The way from "no remote store" to the workbench working on the asset store service of a server (gucefASSETSTORE_RemoteNetworking_Design.md
 *  3.4, 5 and 5.6), as one object with a state and events, for the settings page, the menu, the console and the command line:
 *
 *  - Finding: a look on the local network for services of the type _gucef-assetstore._tcp (CServiceFinder, on by default, a pool task
 *    that ends after the look). What is heard is a list of candidates to offer, never trust.
 *  - The endpoint: "host:port" of the session listener (worker.lan:47400 by default). A configured endpoint always wins over what was
 *    found; the pinned key of the service decides at the connect, whatever the address.
 *  - Pairing: the one-time code of a pairing window of the service (80 bits, shown once on the server) and the pairing port (47401) of
 *    the same host: a pool task (CDevicePairingClient with a pulse generator of its own). The key of the service is learned and pinned
 *    there, the device key is made, and the credentials (device key, token, pinned key) go to the credential store the application
 *    gave (the key file store of the sign in in the "Auth" folder of the settings of the user): never to a log, a setting or a widget.
 *    The code is held as a secret string, passed to the task and wiped there as soon as it was used.
 *  - Connecting: a CRemoteAssetStoreService is made and opened on a pool task (it waits for the link, the session and the description of
 *    the service while the interface goes on). When it is open ServiceReadyEvent is raised and GetService() has it; the application then
 *    lets the sources, the transfers and the bulk conversion work on it.
 *  - Staying connected: the connection of the service reconnects by itself (case A to E of the design: prompt first attempt, full jitter
 *    backoff, resume, wake detection) and this object follows its state: STATE_OFFLINE while the link is gone (the service stays in
 *    use, the views keep what they showed), STATE_CONNECTED again when it is back. A permanent refusal (another key, a revoked device,
 *    an old token) ends the connection: ServiceLostEvent, the error in words and STATE_DISCONNECTED. A first connect that could not reach
 *    the service is tried again with the same backoff while the person wants the connection (settings.autoRetry).
 *  - Disconnecting: ServiceLostEvent first (the application goes back to the store of this process while the service is still there),
 *    then the service is closed with a goodbye.
 *
 *  The states: STATE_DISCONNECTED, STATE_DISCOVERING (a look on the network is running), STATE_PAIRING, STATE_CONNECTING (also while
 *  waiting to try again), STATE_CONNECTED, STATE_OFFLINE (connected before, the link is down: reconnecting, the cached view is shown).
 *  Everything is called on the thread that drives the pulse generator of the connection; the pool tasks only fill their own data.
 */
class ASSETWORKBENCHLIB_PUBLIC_CPP CWorkbenchStoreConnection : public CORE::CObservingNotifier
{
    public:

    enum EState
    {
        STATE_DISCONNECTED = 0 ,
        STATE_DISCOVERING ,
        STATE_PAIRING ,
        STATE_CONNECTING ,
        STATE_CONNECTED ,
        STATE_OFFLINE
    };
    typedef enum EState TState;

    static const CORE::CEvent StateChangedEvent;        /**< CORE::TCloneableInt32: the new state */
    static const CORE::CEvent ServicesChangedEvent;     /**< no data: the list of the services that were found changed */
    static const CORE::CEvent ServiceReadyEvent;        /**< no data: GetService() is open, the application works on it from now on */
    static const CORE::CEvent ServiceLostEvent;         /**< no data: GetService() goes (disconnect or permanent refusal); still valid while this is raised, so the application can let go of it */
    static const CORE::CEvent PairedEvent;              /**< no data: this computer was paired with a service */
    static void RegisterEvents( void );

    static const CORE::CString DefaultServiceType;      /**< "_gucef-assetstore._tcp" */
    static const CORE::CString DefaultServiceName;      /**< "gucef.assetstore": the DRN service name of the asset store service */
    static const CORE::CString DefaultSessionName;      /**< "assetstore": the DRN session of its session listener */
    static const CORE::CString DefaultSlot;             /**< "assetstore": the account of the credentials in the credential store */

    /** A service that was found on the local network */
    struct SDiscoveredService
    {
        CORE::CString instanceId;
        CORE::CString displayName;
        CORE::CString endpoint;             /**< "host:port" of its session listener, as announced */
        CORE::CString host;
        UInt32 pairingPort;                 /**< 0 when it did not announce one */
        CORE::CString keyFingerprint;       /**< public, proves nothing */

        SDiscoveredService( void );
    };
    typedef GUCEF::vector< SDiscoveredService > TDiscoveredVector;

    /** What the connection is made from; the application sets it from its preferences (CWorkbenchPreferences) */
    struct SSettings
    {
        CORE::CString endpoint;             /**< "host:port" or "tcp://host:port"; empty: the endpoints of the credentials, else a look on the network */
        UInt32 pairingPort;                 /**< the port of the pairing listener on the host of the endpoint; 47401 */
        bool lanDiscovery;                  /**< look on the local network; default true */
        bool autoRetry;                     /**< try a first connect again when the service could not be reached; default true */
        CORE::CString serviceName;
        CORE::CString sessionName;
        CORE::CString credentialsSlot;
        CORE::CString clientKind;           /**< told to the service in the hello and at pairing; "viewer" */
        CORE::CString deviceName;           /**< what the service shows for this computer; made from the computer name when empty */
        UInt32 connectTimeoutMs;            /**< 12000 */
        UInt32 retryBaseMs;                 /**< 500 */
        UInt32 retryMaxMs;                  /**< 30000 */
        COMCORE::CServiceFinderSettings finder;     /**< the look: the service type (default DefaultServiceType), the time (3000) and the clients (default broadcast and multicast) */
        CORE::CDataNode serviceSettings;    /**< further settings of CRemoteAssetStoreServiceSettings (attributes), for tests and unusual sites */

        SSettings( void );
    };

    CWorkbenchStoreConnection( void );

    virtual ~CWorkbenchStoreConnection() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  'store' keeps the credentials of this computer (not owned, has to outlive this object or Shutdown()); NULL: nothing can be paired.
     *  'pulseGenerator' is the notification pulse generator of the service and of the timer, the global one when it is null
     */
    bool Initialize( AUTH::ICredentialStore* store, const CORE::PulseGeneratorPtr& pulseGenerator = CORE::PulseGeneratorPtr() );

    /** Disconnects, waits for a task that runs and lets go of everything */
    void Shutdown( void );

    bool IsInitialized( void ) const;

    /** Another credential store for the pairing (not owned); what is paired is read again */
    void SetCredentialStore( AUTH::ICredentialStore* store );

    void SetSettings( const SSettings& settings );
    const SSettings& GetSettings( void ) const;

    /** The endpoint of the settings, normalised ("tcp://" and white space taken off) */
    void SetEndpoint( const CORE::CString& endpoint );
    const CORE::CString& GetEndpoint( void ) const;

    /** @name What the person asks for */
    ///@{

    /** Looks on the local network (a pool task; STATE_DISCOVERING while it runs and nothing else is going on); false when a look runs or discovery is off */
    bool Discover( void );

    /** Makes a service that was found the endpoint (and its pairing port the pairing port to use); false for an index that is not there */
    bool SelectService( UInt32 index );

    /**
     *  Pairs this computer with the service of the endpoint with the code of its pairing window, then connects (a pool task); false when
     *  something else is going on. The code is taken (and wiped here); the answer is StateChangedEvent, PairedEvent and GetErrorText()
     */
    bool Pair( const AUTH::CSecretString& code, bool connectAfterwards = true );

    /** Connects to the endpoint (when one is given it becomes the endpoint of the settings); false when this computer is not paired or something else is going on */
    bool Connect( const CORE::CString& endpoint = CORE::CString::Empty );

    /** Ends the connection, or the attempts; the store of this process is the one in use again after ServiceLostEvent */
    void Disconnect( void );

    /** Forgets the pairing: the credentials are removed from the store (and the connection ended) */
    bool ForgetPairing( void );
    ///@}

    /** @name What there is to tell */
    ///@{
    TState GetState( void ) const;
    bool IsConnected( void ) const;                 /**< STATE_CONNECTED or STATE_OFFLINE: the service is the one in use */
    bool IsBusy( void ) const;                      /**< a pairing, an opening or a look is running */
    bool IsPaired( void ) const;                    /**< the credential store has complete credentials of this computer */
    static const char* GetStateName( TState state );

    /** The state in words, for a person */
    CORE::CString GetStateText( void ) const;

    /** What went wrong last, in words that say what to do; empty when nothing did */
    const CORE::CString& GetErrorText( void ) const;

    /** The name and version the service gave in its hello, and what it says it is and offers */
    const CORE::CString& GetServerName( void ) const;
    const CORE::CString& GetServerVersion( void ) const;
    const ASSETSTORE::CAssetServiceInfo& GetServiceInfo( void ) const;

    /** Name, version, roles, capabilities, locations and stores as lines of text */
    CORE::CString GetServerSummary( void ) const;

    /** "worker.lan:47400" of the service in use, empty without one */
    const CORE::CString& GetActiveEndpoint( void ) const;

    /** What the sources tab says as the store in use: "worker.lan:47400 (name)" */
    CORE::CString GetStoreLabel( void ) const;

    /** The service in use: while STATE_CONNECTED or STATE_OFFLINE, else null */
    ASSETSTORE::TAssetStoreServicePtr GetService( void ) const;

    const TDiscoveredVector& GetDiscoveredServices( void ) const;

    /** The text of a found service for a list: its name, address and a short form of its key */
    static CORE::CString DescribeService( const SDiscoveredService& service );

    /** The pairing endpoint the next pairing uses: "tcp://host:port", empty when it has to be looked for */
    CORE::CString GetPairingEndpoint( void ) const;

    /** "host" of an endpoint "host:port" or "tcp://host:port" */
    static CORE::CString GetHostOf( const CORE::CString& endpoint );
    static CORE::CString NormaliseEndpoint( const CORE::CString& endpoint );
    ///@}

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CWorkbenchStoreConnection > TEventCallback;

    CWorkbenchStoreConnection( const CWorkbenchStoreConnection& src );              /**< not implemented */
    CWorkbenchStoreConnection& operator=( const CWorkbenchStoreConnection& src );   /**< not implemented */

    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnServiceConnectionState( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void SetState( TState state );
    void Fail( const CORE::CString& text );
    void CheckFind( void );
    void CheckTask( void );
    void CheckRetry( void );
    void FinishPairing( const CStoreConnectionTaskData& data );
    void FinishConnect( CStoreConnectionTaskData& data );
    bool StartConnectTask( void );
    void ReleaseService( void );
    void RefreshPaired( void );
    void ScheduleRetry( void );
    void LoseService( void );
    CORE::CString MakeDeviceName( void ) const;
    static UInt64 NowInMs( void );

    private:

    SSettings m_settings;
    AUTH::ICredentialStore* m_store;
    CORE::PulseGeneratorPtr m_pulse;
    CORE::CTimer* m_timer;
    TState m_state;
    CORE::CString m_error;
    CORE::CString m_serverName;
    CORE::CString m_serverVersion;
    ASSETSTORE::CAssetServiceInfo m_info;
    ASSETSTORE::TAssetStoreServicePtr m_service;
    ASSETSTORE::TAssetStoreServicePtr m_connecting;     /**< the service whose opening runs on a task */
    CORE::CString m_activeEndpoint;
    CORE::CString m_pairingEndpoint;                    /**< of a found service, when it announced its pairing port */
    TDiscoveredVector m_discovered;
    CORE::CFutureResult m_findFuture;
    bool m_finding;
    CORE::CFutureResult m_taskFuture;
    bool m_taskRunning;
    CStoreConnectionTaskData::TKind m_taskKind;
    bool m_connectAfterPair;
    bool m_discardTask;                                 /**< the person disconnected while a task ran: its result is let go of */
    bool m_wanted;                                      /**< the person wants a connection: a first connect that could not reach the service is tried again */
    bool m_retrying;
    UInt64 m_retryAtInMs;
    UInt32 m_retryCount;
    DRNUTILS::CReconnectBackoff m_backoff;
    bool m_paired;
    bool m_wasConnected;
    bool m_initialized;
    bool m_inShutdown;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* ASSETWORKBENCHLIB_CWORKBENCHSTORECONNECTION_H ? */
