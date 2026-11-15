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

#ifndef GUCEF_DRNUTILS_CREMOTESERVICECHANNEL_H
#define GUCEF_DRNUTILS_CREMOTESERVICECHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#ifndef GUCEF_DRN_CDRNSESSION_H
#include "gucefDRN_CDRNSession.h"
#define GUCEF_DRN_CDRNSESSION_H
#endif /* GUCEF_DRN_CDRNSESSION_H ? */

#ifndef GUCEF_DRN_DRNPROTOCOL_H
#include "gucefDRN_DRNProtocol.h"
#define GUCEF_DRN_DRNPROTOCOL_H
#endif /* GUCEF_DRN_DRNPROTOCOL_H ? */

#ifndef GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#include "gucefDRNUTILS_CDeviceCredentials.h"
#define GUCEF_DRNUTILS_CDEVICECREDENTIALS_H
#endif /* GUCEF_DRNUTILS_CDEVICECREDENTIALS_H ? */

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

class CRemoteServiceConnection;

/**
 *  One link of a client to its remote service (gucefASSETSTORE_RemoteNetworking_Design.md 3.3): a DRN node of its own
 *  on a thread of its own that joins the service's session with the pinned static key and the device token, a drn RPC
 *  transport over the member link and an RPC endpoint. When the link is lost the endpoint's RPC session is suspended and
 *  the next StartJoin() puts it on a new member link (CRpcEndpoint::Rebind(): case A); an RPC session that ended (the
 *  service's GOODBYE, or it was unknown at the service) gets a new endpoint. Refusals of the link are kept with their
 *  DRN reason, so that permanent ones (banned, another key, the token not accepted, another policy) end the attempts.
 *  The connection (CRemoteServiceConnection) owns its channels: the control channel and the bulk channels.
 *  Threading: lives on the notification pulse generator; Step() runs from the connection's tick there.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CRemoteServiceChannel : public CORE::CObservingNotifier
{
    public:

    enum EChannelState
    {
        CHANNEL_IDLE = 0 ,      /**< nothing started, or shut down */
        CHANNEL_JOINING ,       /**< the DRN link and the session join are under way */
        CHANNEL_BINDING ,       /**< the RPC session is being set up or resumed over the member link */
        CHANNEL_READY ,         /**< the RPC session is established and the wanted contracts are bound */
        CHANNEL_DOWN ,          /**< the link is lost or the join failed; StartJoin() again */
        CHANNEL_REFUSED         /**< the service refused the link for a reason that a retry does not cure */
    };
    typedef enum EChannelState TChannelState;

    struct GUCEF_DRNUTILS_PUBLIC_CPP SSettings
    {
        CORE::CString nodeName;
        UInt32 linkTimeoutMs;
        UInt32 echoIntervalMs;
        UInt32 resumeGraceMs;
        UInt32 receiveWindowBytes;
        UInt32 maxMessageBytes;
        UInt32 sendQueueBytes;
        CORE::CString::StringVector wantedContracts;   /**< bound at the start (major 1) */

        SSettings( void );
    };

    /** "IDLE", "JOINING", "BINDING", "READY", "DOWN", "REFUSED" */
    static const char* ChannelStateToString( TChannelState state );

    /** Whether a DRN link reason is one a retry does not cure */
    static bool IsPermanentRefusal( DRN::TDRNLinkReason reason );

    CRemoteServiceChannel( const CORE::PulseGeneratorPtr& pulseGenerator, UInt32 index, CRemoteServiceConnection* owner );
    virtual ~CRemoteServiceChannel() GUCEF_VIRTUAL_OVERRIDE;

    /** Joins (or joins again) with the credentials; 'hostOverride' "host:port" first when not empty. False with the reason */
    bool StartJoin( const CDeviceCredentials& credentials, const SSettings& settings, const CORE::CString& hostOverride, CORE::CString& error );

    /** Advances the steps; from the connection's tick */
    void Step( void );

    TChannelState GetState( void ) const;
    UInt32 GetIndex( void ) const;

    /** The endpoint of the RPC session (NULL before the first link) */
    COMCORE::CRpcEndpointPtr GetEndpoint( void ) const;

    /** Since the last StartJoin(): the RPC session was resumed over the new link (case A) */
    bool WasRpcResumed( void ) const;

    /** Since the last StartJoin() or ClearNewRpcSession(): the RPC session is new (a first one, a gap, unknown at the service) */
    bool HasNewRpcSession( void ) const;
    void ClearNewRpcSession( void );

    /** The RPC session ended (GOODBYE, an abort, closed) since the last StartJoin() */
    bool IsRpcClosed( void ) const;

    /** The last refusal or close of the link: its DRN reason and text */
    DRN::TDRNLinkReason GetLastLinkReason( void ) const;
    const CORE::CString& GetLastLinkText( void ) const;

    /** Drops the link without Goodbye, as a lost network would (tests, a detected sleep) */
    void DropLink( void );

    /** Ends the RPC session (graceful: GOODBYE) and stops the node */
    void Shutdown( bool graceful );

    /** The smoothed round trip time of the link in microseconds; false without a link */
    bool GetRoundTripTime( UInt32& rttUs ) const;

    /** Counters: links made, RPC resumes (case A), new RPC sessions */
    UInt32 GetLinkCount( void ) const;
    UInt32 GetRpcResumeCount( void ) const;
    UInt32 GetNewRpcSessionCount( void ) const;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** The transport the RPC session runs over, for each member link; the default asks the owning connection (a decorator in tests) */
    virtual COMCORE::CIRpcTransportPtr WrapTransport( const COMCORE::CIRpcTransportPtr& transport );

    private:

    typedef CORE::CTEventHandlerFunctor< CRemoteServiceChannel > TEventCallback;

    CRemoteServiceChannel( const CRemoteServiceChannel& src );              /**< not implemented */
    CRemoteServiceChannel& operator=( const CRemoteServiceChannel& src );   /**< not implemented */

    void StepJoining( void );
    void StepBinding( void );
    void StepReady( void );
    void ReleaseEndpoint( bool close );
    void OnLinkEnded( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnJoinFailed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcResumed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcGap( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnRpcClosed( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    CORE::PulseGeneratorPtr m_pulse;
    UInt32 m_index;
    CRemoteServiceConnection* m_owner;
    SSettings m_settings;
    CORE::CString m_drnSessionName;
    DRN::CDRNNode* m_node;
    DRN::CDRNSession* m_session;
    COMCORE::CIRpcTransportPtr m_transport;
    COMCORE::CRpcEndpointPtr m_endpoint;
    TChannelState m_state;
    bool m_rpcResumed;
    bool m_rpcNewSession;
    bool m_awaitingRebind;          /**< Rebind() was asked: READY only once the endpoint told how it went (resumed, or a new session) */
    bool m_rpcClosed;
    bool m_joinFailed;
    UInt64 m_stepStartedMs;
    DRN::TDRNLinkReason m_lastReason;
    CORE::CString m_lastText;
    UInt32 m_links;
    UInt32 m_rpcResumes;
    UInt32 m_newRpcSessions;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CREMOTESERVICECHANNEL_H ? */
