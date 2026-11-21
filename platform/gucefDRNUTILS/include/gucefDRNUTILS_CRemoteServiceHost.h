/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOST_H
#define GUCEF_DRNUTILS_CREMOTESERVICEHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CISERVICERESOURCESOURCE_H
#include "gucefCORE_CIServiceResourceSource.h"
#define GUCEF_CORE_CISERVICERESOURCESOURCE_H
#endif /* GUCEF_CORE_CISERVICERESOURCESOURCE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CTATOMIC_H
#include "gucefMT_CTAtomic.h"
#define GUCEF_MT_CTATOMIC_H
#endif /* GUCEF_MT_CTATOMIC_H ? */

#ifndef GUCEF_AUTH_ICREDENTIALSTORE_H
#include "gucefAUTH_ICredentialStore.h"
#define GUCEF_AUTH_ICREDENTIALSTORE_H
#endif /* GUCEF_AUTH_ICREDENTIALSTORE_H ? */

#ifndef GUCEF_COMCORE_CRPCHOST_H
#include "gucefCOMCORE_CRpcHost.h"
#define GUCEF_COMCORE_CRPCHOST_H
#endif /* GUCEF_COMCORE_CRPCHOST_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#include "gucefDRN_CIDRNResourceProvider.h"
#define GUCEF_DRN_CIDRNRESOURCEPROVIDER_H
#endif /* GUCEF_DRN_CIDRNRESOURCEPROVIDER_H ? */

#ifndef GUCEF_DRN_CDRNBANLIST_H
#include "gucefDRN_CDRNBanList.h"
#define GUCEF_DRN_CDRNBANLIST_H
#endif /* GUCEF_DRN_CDRNBANLIST_H ? */

#ifndef GUCEF_DRN_CDRNSECURITYPOLICY_H
#include "gucefDRN_CDRNSecurityPolicy.h"
#define GUCEF_DRN_CDRNSECURITYPOLICY_H
#endif /* GUCEF_DRN_CDRNSECURITYPOLICY_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEHOSTSETTINGS_H
#include "gucefDRNUTILS_CRemoteServiceHostSettings.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEHOSTSETTINGS_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOSTSETTINGS_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESERVICEPRINCIPAL_H
#include "gucefDRNUTILS_CRemoteServicePrincipal.h"
#define GUCEF_DRNUTILS_CREMOTESERVICEPRINCIPAL_H
#endif /* GUCEF_DRNUTILS_CREMOTESERVICEPRINCIPAL_H ? */

#ifndef GUCEF_DRNUTILS_CREMOTESESSIONCACHE_H
#include "gucefDRNUTILS_CRemoteSessionCache.h"
#define GUCEF_DRNUTILS_CREMOTESESSIONCACHE_H
#endif /* GUCEF_DRNUTILS_CREMOTESESSIONCACHE_H ? */

#ifndef GUCEF_DRNUTILS_CIREMOTESESSIONEXTENSION_H
#include "gucefDRNUTILS_CIRemoteSessionExtension.h"
#define GUCEF_DRNUTILS_CIREMOTESESSIONEXTENSION_H
#endif /* GUCEF_DRNUTILS_CIREMOTESESSIONEXTENSION_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#include "gucefDRNUTILS_CDeviceRegistry.h"
#define GUCEF_DRNUTILS_CDEVICEREGISTRY_H
#endif /* GUCEF_DRNUTILS_CDEVICEREGISTRY_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEACCESSRULES_H
#include "gucefDRNUTILS_CDeviceAccessRules.h"
#define GUCEF_DRNUTILS_CDEVICEACCESSRULES_H
#endif /* GUCEF_DRNUTILS_CDEVICEACCESSRULES_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H
#include "gucefDRNUTILS_CDeviceDrnAccessPolicy.h"
#define GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H
#endif /* GUCEF_DRNUTILS_CDEVICEDRNACCESSPOLICY_H ? */

#ifndef GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H
#include "gucefDRNUTILS_CPairingWindowRequest.h"
#define GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H
#endif /* GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H ? */

#ifndef GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#include "gucefDRNUTILS_CCommandOutcomeCache.h"
#define GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H
#endif /* GUCEF_DRNUTILS_CCOMMANDOUTCOMECACHE_H ? */

#ifndef GUCEF_COMCORE_CSERVICEADVERTISER_H
#include "gucefCOMCORE_CServiceAdvertiser.h"
#define GUCEF_COMCORE_CSERVICEADVERTISER_H
#endif /* GUCEF_COMCORE_CSERVICEADVERTISER_H ? */

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CDevicePairingHost;

/**
 *  The host of a remote service (gucefASSETSTORE_RemoteNetworking_Design.md 3 to 7): the DRN node of the session
 *  listener on a thread of its own (ownThread) with the service's static key and signed device tokens, the device
 *  registry, the ban list, the DRN join policy and the per-method RPC access rules over the registry, the RPC host on
 *  the drn listener, the client session cache with an event ring per session, the pairing listener of an open pairing
 *  window (CDevicePairingHost), the session layer gucef.drnutils.session and the device administration
 *  gucef.drnutils.devices.Devices. A product registers its own services, access rules and session extensions:
 *
 *      host.RegisterService( contract, "Catalogue", &catalogue );           // RPC_EXECUTION_NOTIFICATION_THREAD
 *      host.GetAccessRules().AddRule( "gucef.assetstore.Catalogue/*", "viewer,curator,admin" );
 *      host.AddSessionExtension( &uploads );
 *
 *  and in a handler GetCallPrincipal( call, principal ) gives the device, its roles and kind from the registry and the
 *  client session of the call; PublishEvent( principal.sessionId, event ) appends to the session's event ring.
 *  Keys: the static x25519 key, the token issuer's Ed25519 seed and the service id come from the settings (tests),
 *  else from the credential store (provider "<serviceName>.host", made and stored at the first start), else they are
 *  made per run (logged: clients must pair again after every start).
 *  Threading: everything runs on the pulse generator given at construction: the RPC handlers, the events of the RPC
 *  host and its endpoints, the timer. The access policies run on the node's thread (they read the registry and the
 *  session cache, which have locks of their own). PublishEvent() may be called from any thread: the event is written
 *  to its stream at once on the pulse thread, at the next tick elsewhere. Lock order: the node lock, then the endpoint
 *  locks, then this host's event lock; nothing here waits for the pulse thread under the node lock.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServiceHost : public CORE::CObservingNotifier ,
                                                    public CORE::CIServiceResourceSource
{
    public:

    static const char* const KeyProviderSuffix;     /**< ".host": the credential store provider is serviceName + this */
    static const char* const LocalAdmin;            /**< "local-admin": who acted when the service itself was asked */

    /** What the event ring of a session holds */
    struct GUCEF_DRNUTILS_PUBLIC_CPP SRingState
    {
        UInt64 nextEventSeq;
        UInt64 retainedFrom;
        UInt64 retainedBytes;
        UInt32 retainedCount;

        SRingState( void );
    };

    CRemoteServiceHost( const CORE::PulseGeneratorPtr& pulseGenerator, const CRemoteServiceHostSettings& settings );
    virtual ~CRemoteServiceHost() GUCEF_VIRTUAL_OVERRIDE;

    /** Where the registry (devices.json) and the bans (bans.txt) are kept; empty (the default): in memory only */
    void SetStateDirectory( const CORE::CString& stateDirectory );

    /** The credential store of the keys; not owned, GUCEF_NULL for none */
    void SetCredentialStore( AUTH::ICredentialStore* store );

    /** Keys, registry, bans, the node and its listener, the RPC host. False with the reason; nothing listens then */
    bool Start( CORE::CString& error );

    /**
     *  The service drains (design 5.5): SERVICE_STOPPING to every session with the expected time back (0: the setting),
     *  the pairing window closed, Hello and the product's mutations refused UNAVAILABLE DRAINING with retryAfterMs (the
     *  listener stays open: stopping it would end the accepted links too); once no call or drain hold is open or the
     *  drain time is over, every RPC session is ended with GOODBYE
     */
    void BeginDrain( const CORE::CString& reason, UInt32 expectedBackSeconds = 0 );
    bool IsDraining( void ) const;

    /** Every RPC session of the drain was ended */
    bool IsDrainComplete( void ) const;

    /**
     *  Work of a product that a drain waits for (a mutation in progress, an answer the product still owes): every
     *  AddDrainHold() is followed by one ReleaseDrainHold(). A drain ends early only when no hold is taken and no call of
     *  the host's own endpoints is open; at the drain time it ends anyway (the RPC layer counts the calls a peer made of
     *  this side only while they are linked to the product, so the product says what it is doing). Any thread
     */
    void AddDrainHold( void );
    void ReleaseDrainHold( void );
    UInt32 GetDrainHoldCount( void ) const;

    /** Answers a call UNAVAILABLE DRAINING {retryAfterMs} while draining (for the mutations of a product); true when it did */
    bool RefuseIfDraining( COMCORE::CRpcServerCall& call ) const;

    /** Ends everything: sessions, the pairing listener, the RPC host, the listener, the node; saves the registry */
    void Stop( void );

    bool IsRunning( void ) const;

    UInt16 GetSessionPort( void ) const;
    UInt16 GetPairingPort( void ) const;

    /** The endpoints clients are told: the setting, else tcp://127.0.0.1:<port> */
    CORE::CString::StringVector GetSessionEndpoints( void ) const;
    CORE::CString::StringVector GetPairingEndpoints( void ) const;

    /** Random per Start(): a client that sees another one knows the service restarted */
    const CORE::CString& GetHostInstanceId( void ) const;
    const CORE::CString& GetServiceId( void ) const;
    const UInt8* GetStaticPublicKey( void ) const;
    CORE::CString GetStaticPublicKeyHex( void ) const;
    const UInt8* GetIssuerPublicKey( void ) const;

    /** Milliseconds since the node's thread last pulsed the node; 0 while the host does not run */
    UInt64 GetNodeHeartbeatAgeMs( void ) const;

    DRN::CDRNNode* GetNode( void ) const;
    DRN::CDRNSession* GetDrnSession( void ) const;
    COMCORE::CRpcHost* GetRpcHost( void ) const;
    COMCORE::CRpcServiceSetPtr GetServiceSet( void ) const;
    const CORE::PulseGeneratorPtr& GetPulseGenerator( void ) const;

    // ---- for the components of a service ------------------------------------------------------------------------

    /** A service of a product on every RPC session; before or after Start() */
    bool RegisterService( const COMCORE::CRpcContractPtr& contract, const CORE::CString& serviceName,
                          COMCORE::CIRpcServiceImpl* impl, COMCORE::TRpcExecutionMode mode = COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD );

    CDeviceAccessRules& GetAccessRules( void );
    CDeviceDrnAccessPolicy& GetDrnAccessPolicy( void );

    /**
     *  A share of the host: files (or any resources) that the paired devices with one of the roles read through the DRN resource
     *  facility of the session of the host, and that a VFS mounts with CRemoteShareMount. The provider (a CDRN resource provider, for a
     *  tree of files CDRNFileSystemAccessResourceProvider) is served under the name; the roles are those of the devices as the registry
     *  has them at every call ('*': any device with a role; none: nobody); the decision (not owned, may be NULL: then nothing is readable)
     *  has the last word on reading, browsing and listing (CIRemoteShareDecision).
     *  The registration is the host's: it is applied now when the host runs, and again by every Start(), which makes a new DRN session;
     *  the share stays until UnregisterShare(). False, with the reason, for a name that cannot be a provider name or is a share already.
     *  Call from the thread that starts and stops the host.
     */
    bool RegisterShare( const CORE::CString& name, const DRN::CIDRNResourceProviderPtr& provider, const CORE::CString::StringSet& roles,
                        CIRemoteShareDecision* decision, CORE::CString& error );

    /** Takes the share away: no new calls reach the decision and the provider, calls under way finish. False for a name that is no share */
    bool UnregisterShare( const CORE::CString& name );

    bool HasShare( const CORE::CString& name ) const;

    /** Whether the share is served by the DRN session of the running host now */
    bool IsShareServed( const CORE::CString& name ) const;

    /** Not owned; removed before it goes */
    void AddSessionExtension( CIRemoteSessionExtension* extension );
    void RemoveSessionExtension( CIRemoteSessionExtension* extension );

    /** Who makes the call: false when the call does not come from an ACTIVE device */
    bool GetCallPrincipal( const COMCORE::CRpcServerCall& call, CRemoteServicePrincipal& principal ) const;

    /** The client session of a call (its control or an attached channel), NULL for none */
    CRemoteSessionPtr GetCallSession( const COMCORE::CRpcServerCall& call ) const;

    /** An empty gucef.drnutils.session Event of the kind */
    COMCORE::CRpcMessage NewEvent( const CORE::CString& kind ) const;

    /** Appends the event to the session's ring (eventSeq and atUtcMs are set) and writes it to its stream; its sequence, 0 for an unknown session. Any thread */
    UInt64 PublishEvent( const CORE::CString& sessionId, const COMCORE::CRpcMessage& event );

    /** The same to every session */
    void PublishEventToAll( const COMCORE::CRpcMessage& event );

    SRingState GetRingState( const CORE::CString& sessionId ) const;

    // ---- LAN discovery (setting lanDiscovery) --------------------------------------------------------------------

    /**
     *  Public properties a product adds to the announcement of the service on the local network, for example a
     *  feature list; any time, an announcement that is out is updated. Keys that suggest a secret are refused by the
     *  announcement (COMCORE::CServiceAnnouncement::IsForbiddenPropertyKey())
     */
    void SetLanDiscoveryProperties( const CORE::CValueList& properties );

    /**
     *  The announcement of the service: the service type, display name and TTL of lanDiscoveryAdvertiser, the service
     *  id as the instance id, the session endpoint with the address the announcement is heard from
     *  (tcp://0.0.0.0:<port>) followed by the configured sessionEndpoints, the protocol version, the key fingerprint
     *  and the pairing port as the settings say, and the product's properties. Nothing secret, no product version
     */
    void MakeLanAnnouncement( COMCORE::CServiceAnnouncement& announcement ) const;

    /** Whether the service is announced on the local network now: lanDiscovery on, running, not draining */
    bool IsAdvertisingOnLan( void ) const;

    /** The advertiser while the host runs with lanDiscovery on, else NULL */
    COMCORE::CServiceAdvertiser* GetLanAdvertiser( void ) const;

    // ---- administration (the Devices service and the product's admin endpoint) ---------------------------------

    bool OpenPairingWindow( const CPairingWindowRequest& request, const CORE::CString& openedBy, CORE::CString& windowId,
                            CORE::CString& code, UInt64& expiresUtcMs, CORE::CString& error );
    bool ClosePairingWindow( const CORE::CString& reason );
    bool IsPairingWindowOpen( void ) const;
    bool ConfirmPairing( const CORE::CString& deviceId, const CORE::CString& fingerprint, bool approve, const CORE::CString& by, CORE::CString& error );
    void GetPendingPairings( CDeviceRegistry::TDeviceRecordVector& pending ) const;
    CDevicePairingHost* GetPairingHost( void ) const;

    /** REVOKED in the registry, bans of the device id and key, its links and sessions closed, a waiting pairing declined */
    bool RevokeDevice( const CORE::CString& deviceId, const CORE::CString& reason, const CORE::CString& by, CORE::CString& error );
    bool SetDeviceRoles( const CORE::CString& deviceId, const CORE::CString::StringSet& roles, const CORE::CString& by, CORE::CString& error );
    bool RenameDevice( const CORE::CString& deviceId, const CORE::CString& name, const CORE::CString& by, CORE::CString& error );

    /** Closes a client session without grace (its RPC sessions end with GOODBYE) */
    bool CloseSession( const CORE::CString& sessionId, const CORE::CString& by, CORE::CString& error );

    /** A device token for the device as the registry has it now (its tokenExpiresUtcMs is set); empty when it is not ACTIVE */
    CORE::CString IssueToken( CDeviceRecord& device, UInt64& expiresUtcMs ) const;

    /** The security policy of the session listener as a document: what a client gets at pairing */
    void GetSessionPolicyDocument( CORE::CDataNode& policy ) const;

    /** The policy of the session listener, from the settings */
    DRN::CDRNSecurityPolicy MakeSessionSecurityPolicy( void ) const;

    /** The policy of every pairing listener and pairing client: fixed, so both sides have the same hash before they share anything */
    static DRN::CDRNSecurityPolicy MakePairingSecurityPolicy( void );

    /** Saves the registry (in memory: nothing) */
    void SaveRegistry( void );

    CDeviceRegistry& GetDeviceRegistry( void );
    CRemoteSessionCache& GetSessionCache( void );
    DRN::CDRNBanList& GetBanList( void );
    const CRemoteServiceHostSettings& GetSettings( void ) const;

    /** Values for an info page: state, ports, devices, sessions, rings (no names, no secrets) */
    void GetInfo( CORE::CValueList& info ) const;

    /** The devices and the sessions as documents (Devices/List, Devices/ListSessions, admin commands) */
    void GetDevicesDocument( CORE::CDataNode& devices ) const;
    void GetSessionsDocument( CORE::CDataNode& sessions ) const;

    /** The static x25519 private key in hexadecimal; for the pairing listener only */
    const AUTH::CSecretString& GetStaticPrivateKeyHex( void ) const;

    /** Wall clock UTC and monotonic milliseconds as the host uses them */
    static UInt64 NowUtcMs( void );
    static UInt64 NowMs( void );

    // CIServiceResourceSource
    virtual CORE::CString GetResourceSourceName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetResourceCounters( CORE::CValueList& counters ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteServiceHost > TEventCallback;
    typedef COMCORE::CTRpcMethodHandler< CRemoteServiceHost > THandler;
    typedef GUCEF::vector< CIRemoteSessionExtension* > TExtensionVector;

    struct SEndpointBinding
    {
        CORE::CString sessionId;
        bool attached;
    };
    typedef GUCEF::map< COMCORE::CRpcEndpoint*, SEndpointBinding > TEndpointBindingMap;
    typedef GUCEF::map< COMCORE::CRpcServerCall*, CORE::CString > TEventCallMap;

    struct SShare
    {
        DRN::CIDRNResourceProviderPtr provider;
        CORE::CString::StringSet roles;
        CIRemoteShareDecision* decision;
        bool served;
    };
    typedef GUCEF::map< CORE::CString, SShare > TShareMap;

    CRemoteServiceHost( const CRemoteServiceHost& src );                /**< not implemented */
    CRemoteServiceHost& operator=( const CRemoteServiceHost& src );     /**< not implemented */

    bool LoadKeys( CORE::CString& error );

    // gucef.drnutils.session.Session
    COMCORE::TRpcHandlerResult OnHello( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnResume( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnAttach( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnEvents( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnResync( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnGoodbye( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnRenewToken( COMCORE::CRpcServerCall& call );

    // gucef.drnutils.devices.Devices
    COMCORE::TRpcHandlerResult OnDevicesList( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesRevoke( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesSetRoles( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesRename( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesOpenPairingWindow( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesClosePairingWindow( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesListPendingPairings( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesConfirmPairing( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesListSessions( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnDevicesCloseSession( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult RunDeviceCommand( COMCORE::CRpcServerCall& call, UInt32 commandIdField, Int32 command );

    void OnTick( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEndpointCreated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEndpointClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcSuspended( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcResumed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcGap( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEventsWritable( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEventsCancelled( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void BindEndpoint( const UInt8* rpcSessionId, const CORE::CString& sessionId, bool attached );
    void UnbindEndpoint( COMCORE::CRpcEndpoint* endpoint, SEndpointBinding& binding, bool& found );
    void DrainEvents( const CRemoteSessionPtr& session );
    void DetachEventsCall( const CRemoteSessionPtr& session, const COMCORE::CRpcStatus* finishWith );
    void CloseClientSession( const CRemoteSessionPtr& session, const CORE::CString& reason, bool closeRpc );
    void SuspendClientSession( const CRemoteSessionPtr& session, const CORE::CString& reason );
    void NotifyClosed( const TRemoteSessionVector& sessions, const CORE::CString& reason );
    void CloseDeviceLinks( const CORE::CString& deviceId );
    void FinishDrain( void );

    /** While draining: whether every client session with an open event stream wrote its SERVICE_STOPPING event */
    bool AreStopEventsWritten( void ) const;
    void StartLanDiscovery( void );
    void StopLanDiscovery( void );

    /** Serves a share on the DRN session of the running host; false when the session refuses the name */
    bool ServeShare( const CORE::CString& name, SShare& share );
    void ServeShares( void );
    void FillExtensions( const CRemoteSessionPtr& session, bool resync, CORE::CDataNode& extensions ) const;
    UInt64 GetUsedBytes( void ) const;
    bool IsOnPulseThread( void ) const;
    bool FindEndpointPeer( const COMCORE::CRpcEndpoint& endpoint, UInt32& peerId ) const;
    CORE::CString GetPeerAddress( UInt32 peerId ) const;
    void FailWithDetail( COMCORE::CRpcServerCall& call, COMCORE::TRpcStatusCode code, const char* detailCode, const CORE::CString& message ) const;
    bool IsAdmin( const CRemoteServicePrincipal& principal ) const;

    CORE::PulseGeneratorPtr m_pulse;
    CRemoteServiceHostSettings m_settings;
    CORE::CString m_stateDirectory;
    AUTH::ICredentialStore* m_credentials;
    AUTH::CSecretString m_staticPrivateHex;
    UInt8 m_staticPublic[ 32 ];
    UInt8 m_issuerSeed[ 32 ];
    UInt8 m_issuerPublic[ 32 ];
    CORE::CString m_serviceId;
    CORE::CString m_hostInstanceId;
    CDeviceRegistry m_registry;
    CRemoteSessionCache m_sessions;
    CDeviceAccessRules* m_accessRules;                  /**< owned by m_accessRulesPtr, which the RPC host shares */
    COMCORE::CIRpcAccessPolicyPtr m_accessRulesPtr;
    CDeviceDrnAccessPolicy m_drnPolicy;
    DRN::CDRNBanList m_bans;
    DRN::CDRNNode* m_node;
    DRN::CDRNSession* m_drnSession;
    COMCORE::CIRpcTransportListenerPtr m_listener;
    COMCORE::CRpcHost* m_rpcHost;
    COMCORE::CRpcServiceSetPtr m_services;
    COMCORE::CRpcServiceImpl m_sessionService;
    COMCORE::CRpcServiceImpl m_devicesService;
    CDevicePairingHost* m_pairing;
    COMCORE::CServiceAdvertiser* m_lanAdvertiser;
    CORE::CValueList m_lanProperties;
    CCommandOutcomeCache m_commands;
    TExtensionVector m_extensions;
    TShareMap m_shares;
    mutable MT::CMutex m_shareLock;
    TEndpointBindingMap m_endpoints;
    TEventCallMap m_eventCalls;
    CORE::CTimer m_timer;
    MT::CTAtomic< bool > m_pendingDrain;
    mutable MT::CMutex m_eventLock;                     /**< the event rings and the events calls */
    UInt64 m_lastRegistrySaveMs;
    UInt64 m_drainDeadlineMs;
    GUCEF::map< CORE::CString, UInt64 > m_drainStopEvents;  /**< client session id to the eventSeq of its SERVICE_STOPPING event; under m_eventLock */
    UInt32 m_drainHolds;                                /**< under m_eventLock */
    UInt32 m_drainExpectedBackSeconds;
    UInt16 m_sessionPort;
    bool m_running;
    bool m_draining;
    bool m_drainFinished;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICEHOST_H ? */
