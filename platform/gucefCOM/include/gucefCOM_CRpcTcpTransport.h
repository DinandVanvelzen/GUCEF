/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CRPCTCPTRANSPORT_H
#define GUCEF_COM_CRPCTCPTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#include "gucefCOMCORE_CTlsClientConnection.h"
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#include "gucefCOMCORE_CRpcProtocolTask.h"
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRpcTcpTransportListener;

/**
 *  The RPC transport over TCP (gucefCOMCORE RPC design 12.2): a byte stream with a preface and length prefixed
 *  frames. Both sides first send the preface, the bytes "GUCEFRPC" and the framing version 1; then every frame is a
 *  varint length followed by the frame. A peer whose preface does not match (an HTTP request, another protocol) or
 *  whose frame is larger than allowed is disconnected before anything else is read: the first frame may be up to
 *  maxPreHelloFrameSize (the HELLO with its offers), every later one up to maxFrameSize.
 *
 *  The transport is reliable and ordered. TLS is optional: with SetTlsSettings() (or useTls in the configuration)
 *  Connect() protects the connection with TLS and verifies the certificate of the server for the host of the URI;
 *  a connection accepted by a listener with TLS settings is protected likewise. A protected transport reports
 *  isSecure, which an endpoint with requireSecureTransport needs. The principal is "tcp:<address>:<port>", not
 *  authenticated: TLS authenticates the server to the client, not the client. Connect() refuses an address that is
 *  not a loopback address unless TLS is used or allowNonLoopback is set.
 *
 *  Threading (CIRpcTransport): the socket runs on the transport's pulse generator, which is the endpoint's protocol
 *  thread or, without one, a dedicated task of the task manager (CRpcProtocolTask). Every event is raised there.
 *  SendFrame() and Close() take a leaf lock only: the bytes and the close request are handed to the protocol thread,
 *  which writes them at once when it is the caller, else at its next pulse.
 *
 *  Configuration keys: maxFrameSize (16384), maxPreHelloFrameSize (65536), allowNonLoopback (false),
 *  threadPoolName ("gucef.rpc.tcp"), useTls (false) with the keys of CTlsSettings (caBundlePath, serverName,
 *  minVersion, ...) in the same node.
 */
class GUCEF_COM_PUBLIC_CPP CRpcTcpTransport : public COMCORE::CIRpcTransport
{
    public:

    static const CORE::CString TransportType;           /**< "tcp" */
    static const UInt8 Preface[ 9 ];                    /**< "GUCEFRPC" and the framing version 1 */
    static const UInt32 DefaultMaxFrameSize = 16384;
    static const UInt32 DefaultMaxPreHelloFrameSize = 65536;

    CRpcTcpTransport( void );
    virtual ~CRpcTcpTransport() GUCEF_VIRTUAL_OVERRIDE;

    void SetMaxFrameSize( UInt32 maxFrameSize );
    void SetMaxPreHelloFrameSize( UInt32 maxFrameSize );
    void SetAllowNonLoopback( bool allowNonLoopback );

    /** Connect() uses TLS with these settings. Before Connect() */
    void SetTlsSettings( const COMCORE::CTlsSettings& settings );

    /** Connect() uses plain TCP again. Before Connect() */
    void ClearTlsSettings( void );

    /** Whether the connection is protected by TLS, or Connect() will protect it */
    bool IsTls( void ) const;

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Connect( const CORE::CUri& remote ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendFrame( const void* frame, UInt32 size, COMCORE::TRpcProtectionClass protection ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const COMCORE::CRpcTransportProperties& GetProperties( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const COMCORE::CRpcPrincipal& GetPeerPrincipal( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CUri GetRemoteUri( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The lock of the notifier: the events are raised on the protocol thread while observers subscribe and
     *  unsubscribe from other threads (an endpoint subscribes from the thread that starts it). Taken while an event
     *  is delivered, before the locks of the observers (the endpoint lock); never taken under m_lock
     */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether the host part is a loopback address: 127.0.0.0/8 or "localhost" */
    static bool IsLoopbackHost( const CORE::CString& host );

    /** Writes the varint length of a frame and the frame: the framing on the wire */
    static void WriteFramed( CORE::CDynamicBuffer& target, const void* frame, UInt32 size );

    private:
    friend class CRpcTcpTransportListener;

    CRpcTcpTransport( const CRpcTcpTransport& src );                /**< not implemented */
    CRpcTcpTransport& operator=( const CRpcTcpTransport& src );     /**< not implemented */

    /**
     *  The listener's side: a connection the server socket accepted, on the listener's pulse generator.
     *  onOwnTask: the pulse generator is the listener's own protocol task's, driven by a task delegator
     */
    void Adopt( COMCORE::CTCPServerConnection* connection, const CORE::PulseGeneratorPtr& pulseGenerator, const CORE::CUri& remote, bool onOwnTask );

    /** The listener's side: the server socket closed or dropped the connection */
    void ConnectionGone( void );

    bool EnsurePulseGenerator( void );
    void SubscribeToPulses( void );
    void OnSocketConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketData( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ProcessIncoming( const UInt8* data, UInt32 size );
    void FlushOutbound( void );
    void Disconnect( const COMCORE::CRpcStatus& why );
    bool IsProtocolThread( void ) const;
    bool WriteToSocket( const void* data, UInt32 size );

    mutable MT::CMutex m_lock;                  /**< leaf: the outbound bytes and the close request */
    CORE::CDynamicBuffer m_outbound;
    CORE::CDynamicBuffer m_sending;             /**< protocol thread: the bytes being written, swapped with m_outbound */
    bool m_flushing;
    bool m_closeRequested;
    COMCORE::CTlsClientConnection* m_client;
    COMCORE::CTCPServerConnection* m_serverConnection;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    COMCORE::CRpcProtocolTaskPtr m_task;
    CORE::CObserver m_observer;
    CORE::CDynamicBuffer m_inbound;
    bool m_prefaceReceived;
    UInt64 m_framesReceived;
    UInt64 m_framesSent;
    bool m_connected;
    bool m_disconnectRaised;
    bool m_pulsesRequested;
    bool m_onOwnTask;                           /**< the pulse generator is a protocol task's of this transport or its listener */
    mutable COMCORE::CRpcTransportProperties m_properties;
    COMCORE::CRpcPrincipal m_principal;
    CORE::CUri m_remoteUri;
    UInt32 m_maxFrameSize;
    UInt32 m_maxPreHelloFrameSize;
    bool m_allowNonLoopback;
    bool m_useTls;
    COMCORE::CTlsSettings m_tlsSettings;
    CORE::CString m_threadPoolName;
    mutable MT::CMutex m_notifierLock;          /**< see Lock() */
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTSharedPtr< CRpcTcpTransport, MT::CMutex > CRpcTcpTransportPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CRPCTCPTRANSPORT_H ? */
