/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCHOST_H
#define GUCEF_COMCORE_CRPCHOST_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COMCORE_CRPCHOSTSETTINGS_H
#include "gucefCOMCORE_CRpcHostSettings.h"
#define GUCEF_COMCORE_CRPCHOSTSETTINGS_H
#endif /* GUCEF_COMCORE_CRPCHOSTSETTINGS_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The accepting side of many RPC sessions (gucefCOMCORE RPC design 10.5): owns listeners and a service set shared by
 *  all its endpoints, creates an endpoint per new session, keeps suspended endpoints for their grace period and hands
 *  a resume HELLO on a new transport to the session it names (8.2). A resume of a session it no longer knows gets a
 *  new session (SESSION_UNKNOWN). DURABLE results of sessions that ended go to CompleteDurableResult().
 *
 *  Threading: accepted transports are watched on their own protocol thread until their HELLO; events of the host are
 *  raised on its notification pulse generator. The host lets go of an accepted transport at the next pulse of the
 *  transport's own protocol thread, never inside one of the transport's events (a notifier outlives its own
 *  notification): a refused or abandoned transport may have no other holder.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcHost : public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent EndpointCreatedEvent;     /**< CRpcEndpointEventData */
    static const CORE::CEvent EndpointClosedEvent;      /**< CRpcEndpointEventData */

    static void RegisterEvents( void );

    CRpcHost( const CORE::PulseGeneratorPtr& notificationPulseGenerator, const CRpcHostSettings& settings );
    virtual ~CRpcHost() GUCEF_VIRTUAL_OVERRIDE;

    /** Creates and starts the listeners of the settings through CComCoreGlobal's listener factory */
    bool Start( void );
    void Stop( void );

    /** A listener made by the application (drn, a configured instance); listening already or listening later */
    bool AddListener( const CIRpcTransportListenerPtr& listener );

    /** A connected transport whose first frame will be a HELLO */
    bool AcceptTransport( const CIRpcTransportPtr& transport );

    bool RegisterService( const CRpcContractPtr& contract, const CORE::CString& serviceName,
                          CIRpcServiceImpl* impl, TRpcExecutionMode mode = RPC_EXECUTION_CALLING_THREAD );
    CRpcServiceSetPtr GetServiceSet( void ) const;
    void SetAccessPolicy( const CIRpcAccessPolicyPtr& policy );
    CRpcDurableRequestTablePtr GetDurableRequestTable( void ) const;

    /** The outcome of DURABLE work whose session ended (6.5.3), so a later STATUS_QUERY gets it */
    bool CompleteDurableResult( const CRpcRequestId& id, const CRpcStatus& status, const CRpcMessage* response );

    void GetEndpoints( GUCEF::vector< CRpcEndpointPtr >& endpoints ) const;
    UInt32 GetEndpointCount( void ) const;
    CRpcEndpointPtr FindEndpoint( const UInt8* sessionId ) const;
    void GetListenUris( GUCEF::vector< CORE::CUri >& uris ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SPending
    {
        CIRpcTransportPtr transport;
        UInt64 sinceMs;
    };
    typedef GUCEF::vector< SPending > TPendingVector;

    struct SReleased
    {
        CIRpcTransportPtr transport;
        CORE::PulseGeneratorPtr pulseGenerator;
    };
    typedef GUCEF::vector< SReleased > TReleasedVector;
    typedef GUCEF::set< CORE::CPulseGenerator* > TPulseGeneratorSet;

    void OnTransportAccepted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPendingFrame( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPendingDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnEndpointClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnReleasePulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void HandleFirstFrame( const CIRpcTransportPtr& transport, const CRpcFrameEventData& frame );
    void ReleaseOnItsThread( const CIRpcTransportPtr& transport );
    bool TakePending( CORE::CNotifier* notifier, CIRpcTransportPtr& transport );
    CRpcEndpointPtr NewEndpoint( void );
    bool MaySessionStart( const CRpcPrincipal& principal ) const;

    CRpcHost( const CRpcHost& src );                    /**< not implemented */
    CRpcHost& operator=( const CRpcHost& src );         /**< not implemented */

    mutable MT::CMutex m_lock;
    CRpcHostSettings m_settings;
    CRpcServiceSetPtr m_services;
    CIRpcAccessPolicyPtr m_accessPolicy;
    CRpcDurableRequestTablePtr m_durable;
    GUCEF::vector< CIRpcTransportListenerPtr > m_listeners;
    GUCEF::vector< CRpcEndpointPtr > m_endpoints;
    TPendingVector m_pending;
    TReleasedVector m_released;                 /**< accepted transports let go of at the next pulse of their own thread */
    TPulseGeneratorSet m_releasePulseGenerators;    /**< the pulse generators m_releaseObserver is subscribed to */
    CORE::CObserver m_listenerObserver;
    CORE::CObserver m_pendingObserver;
    CORE::CObserver m_endpointObserver;
    CORE::CObserver m_pulseObserver;
    CORE::CObserver m_releaseObserver;
};

/*-------------------------------------------------------------------------*/

/** Event data of the host's endpoint events */
class GUCEF_COMCORE_EXPORT_CPP CRpcEndpointEventData : public CORE::CICloneable
{
    public:

    CRpcEndpointPtr endpoint;

    explicit CRpcEndpointEventData( const CRpcEndpointPtr& rpcEndpoint );
    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCHOST_H ? */
