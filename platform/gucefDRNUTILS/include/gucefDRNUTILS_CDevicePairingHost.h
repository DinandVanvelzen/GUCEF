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

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGHOST_H
#define GUCEF_DRNUTILS_CDEVICEPAIRINGHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

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

#ifndef GUCEF_DRN_CDRNBANLIST_H
#include "gucefDRN_CDRNBanList.h"
#define GUCEF_DRN_CDRNBANLIST_H
#endif /* GUCEF_DRN_CDRNBANLIST_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICERECORD_H
#include "gucefDRNUTILS_CDeviceRecord.h"
#define GUCEF_DRNUTILS_CDEVICERECORD_H
#endif /* GUCEF_DRNUTILS_CDEVICERECORD_H ? */

#ifndef GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H
#include "gucefDRNUTILS_CPairingWindowRequest.h"
#define GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H
#endif /* GUCEF_DRNUTILS_CPAIRINGWINDOWREQUEST_H ? */

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

class CRemoteServiceHost;

/**
 *  The pairing window of a remote service host (gucefASSETSTORE_RemoteNetworking_Design.md 3.4): one window at a time,
 *  with its one-time code (CDevicePairingCrypto: 80 bits, shown once, never logged), the device kind and roles it
 *  grants, how many pairings it allows and its expiry, and its own listener: a second DRN node with the host's static
 *  key, the DRN session "<session>.pair", the shared-secret authenticator with the secret derived from the code (its
 *  proof covers the transcript of the link, so it cannot be relayed onto another link) and an RPC host serving
 *  gucef.drnutils.devices.Pairing. The node exists only while a window is open and while a pairing waits for its
 *  confirmation, and a moment after so the last answer leaves.
 *  Links refused for a wrong code are counted: an address with banAfterFailedProofs of them is banned from the pairing
 *  listener for banMinutes, and maxFailedProofs close the window. A window that grants one of the roles of
 *  requireConfirmationForRoles needs the owner's fingerprint confirmation: Complete answers ACCEPTED (request id = the
 *  device id), the device waits as PENDING_CONFIRMATION, and Confirm() with the fingerprint the client shows completes
 *  it (RESULT with the token) or declines it (ABORTED PAIRING_DECLINED), as does the confirmation timeout.
 *  Lives on the host's pulse generator.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDevicePairingHost : public CORE::CObservingNotifier
{
    public:

    CDevicePairingHost( CRemoteServiceHost& host, const CORE::PulseGeneratorPtr& pulseGenerator );
    virtual ~CDevicePairingHost() GUCEF_VIRTUAL_OVERRIDE;

    /** Opens a window (closing an open one); the code is given once, formatted. False with the rule that is broken */
    bool Open( const CPairingWindowRequest& request, const CORE::CString& openedBy, CORE::CString& windowId,
               CORE::CString& code, UInt64& expiresUtcMs, CORE::CString& error );

    /** Closes the window; pairings that wait for confirmation stay until they are decided or expire */
    void Close( const CORE::CString& reason );

    /** Ends everything at once: the window, the waiting pairings (declined), the node */
    void Shutdown( void );

    bool IsOpen( void ) const;
    const CORE::CString& GetWindowId( void ) const;
    UInt16 GetPort( void ) const;
    UInt32 GetFailedProofCount( void ) const;
    UInt32 GetPendingCount( void ) const;
    UInt32 GetPairedCount( void ) const;

    /** Whether the node of the pairing listener exists */
    bool HasNode( void ) const;

    /** The bans of the pairing listener: addresses with too many wrong codes */
    const DRN::CDRNBanList& GetBans( void ) const;

    /** Decides a pairing that waits; the fingerprint has to be the one of the device's key (any grouping and case) */
    bool Confirm( const CORE::CString& deviceId, const CORE::CString& fingerprint, bool approve,
                  const CORE::CString& by, CORE::CString& error );

    /** The pairings that wait for the owner, as device records (with the fingerprint the owner compares) */
    void GetPending( GUCEF::vector< CDeviceRecord >& pending ) const;

    /** A revoked device: a pairing of it that waits is declined */
    void DeclinePending( const CORE::CString& deviceId, const CORE::CString& reason );

    /** Expiry of the window and of waiting pairings, the end of the node; the host calls it on its tick */
    void OnTick( void );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CDevicePairingHost > TEventCallback;
    typedef COMCORE::CTRpcMethodHandler< CDevicePairingHost > THandler;

    struct SPending
    {
        COMCORE::CRpcServerCallPtr call;
        CORE::CString deviceId;
        CORE::CString address;
        UInt64 expiresAtMs;
        CDeviceRecord device;
    };
    typedef GUCEF::vector< SPending > TPendingVector;
    typedef GUCEF::map< CORE::CString, CORE::CDynamicBuffer > TNonceMap;
    typedef GUCEF::map< CORE::CString, UInt32 > TAddressCountMap;

    CDevicePairingHost( const CDevicePairingHost& src );                /**< not implemented */
    CDevicePairingHost& operator=( const CDevicePairingHost& src );     /**< not implemented */

    bool StartNode( const CORE::CString& authSecretHex, CORE::CString& error );
    void StopNode( void );
    COMCORE::TRpcHandlerResult OnBegin( COMCORE::CRpcServerCall& call );
    COMCORE::TRpcHandlerResult OnComplete( COMCORE::CRpcServerCall& call );
    void OnLinkRefused( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool BuildCompletion( CDeviceRecord& device, COMCORE::CRpcMessage& response, CORE::CString& error );
    CORE::CString GetCallAddress( const COMCORE::CRpcServerCall& call ) const;
    CORE::CString GetCallKey( const COMCORE::CRpcServerCall& call ) const;
    bool NeedsConfirmation( const CPairingWindowRequest& request ) const;

    CRemoteServiceHost& m_host;
    CORE::PulseGeneratorPtr m_pulse;
    DRN::CDRNNode* m_node;
    DRN::CDRNSession* m_session;
    COMCORE::CIRpcTransportListenerPtr m_listener;
    COMCORE::CRpcHost* m_rpcHost;
    COMCORE::CRpcServiceImpl m_pairingService;
    DRN::CDRNBanList m_bans;
    CPairingWindowRequest m_request;
    CORE::CString m_windowId;
    CORE::CString m_openedBy;
    UInt8 m_serverProofKey[ 32 ];
    bool m_open;
    bool m_requireConfirmation;
    UInt32 m_remaining;
    UInt32 m_failedProofs;
    UInt32 m_paired;
    UInt64 m_expiresAtMs;
    UInt64 m_expiresUtcMs;
    UInt64 m_closedAtMs;
    UInt16 m_port;
    TAddressCountMap m_failedByAddress;
    TNonceMap m_nonces;
    TPendingVector m_pending;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGHOST_H ? */
