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

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H

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

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORT_H
#include "gucefCOMCORE_CIRpcTransport.h"
#define GUCEF_COMCORE_CIRPCTRANSPORT_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORT_H ? */

#ifndef GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#include "gucefCOMCORE_CRpcProtocolTask.h"
#define GUCEF_COMCORE_CRPCPROTOCOLTASK_H
#endif /* GUCEF_COMCORE_CRPCPROTOCOLTASK_H ? */

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

class CRpcLoopbackLink;

/*-------------------------------------------------------------------------*/

/**
 *  The in-process RPC transport "loopback" (gucefCOMCORE RPC design 12.1). A frame is copied into the peer's queue
 *  and delivered on the peer's pulse generator: on that thread at its next pulse, never re-entrantly inside
 *  SendFrame(). Reliable, ordered, secure (in process), latency LOCAL, profile "loopback". Principal
 *  "local:in-process", authenticated, with the roles of the settings.
 *
 *  Connect( "loopback://<name>" ) reaches a CRpcLoopbackTransportListener listening on that name; CreatePair() makes a
 *  connected pair directly. Without a pulse generator a transport runs on a dedicated task of the task manager.
 *
 *  The notifier has a lock of its own (Lock()): an endpoint subscribes on the thread that starts it while the
 *  transport's thread raises events, which it does holding this lock. Lock order: the pulse generator's notification
 *  lock, this lock, the endpoint lock, the queue lock (a leaf). The pulse subscription goes through a member observer,
 *  so subscribing to the pulse generator never holds this lock.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcLoopbackTransport : public CIRpcTransport
{
    public:

    static const CORE::CString TransportType;       /**< "loopback" */

    CRpcLoopbackTransport( void );
    virtual ~CRpcLoopbackTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** Two connected transports, a on pulse generator a, b on b */
    static bool CreatePair( const CORE::PulseGeneratorPtr& pulseGeneratorA, const CORE::PulseGeneratorPtr& pulseGeneratorB,
                            CIRpcTransportPtr& a, CIRpcTransportPtr& b );

    void SetMaxFrameSize( UInt32 maxFrameSize );
    void SetPrincipalRoles( const CORE::CString::StringSet& roles );

    /** For transports made by the factory: the pool of the dedicated task, "gucef.rpc.loopback" */
    void SetThreadPoolName( const CORE::CString& poolName );

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Connect( const CORE::CUri& remote ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConnected( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SendFrame( const void* frame, UInt32 size, TRpcProtectionClass protection ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CRpcTransportProperties& GetProperties( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual const CRpcPrincipal& GetPeerPrincipal( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CUri GetRemoteUri( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The notifier's lock, see the class description */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:
    friend class CRpcLoopbackLink;
    friend class CRpcLoopbackTransportListener;

    struct SQueuedFrame
    {
        CORE::CDynamicBuffer bytes;
        bool isDisconnect;
        bool isConnect;
    };
    typedef GUCEF::vector< SQueuedFrame* > TQueue;

    static void Join( CRpcLoopbackTransport& a, CRpcLoopbackTransport& b, const CORE::CUri& uriOfB, const CORE::CUri& uriOfA );
    void Enqueue( const void* frame, UInt32 size, bool isDisconnect, bool isConnect );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool EnsurePulseGenerator( void );
    void SubscribeToPulses( void );
    void DisconnectFromPeer( bool notifyPeer );
    void ReleaseDelivery( void );

    CRpcLoopbackTransport( const CRpcLoopbackTransport& src );                  /**< not implemented */
    CRpcLoopbackTransport& operator=( const CRpcLoopbackTransport& src );       /**< not implemented */

    mutable MT::CMutex m_notifierLock;     /**< see Lock() */
    mutable MT::CMutex m_queueLock;
    TQueue m_queue;
    TQueue m_free;
    CORE::CTSharedPtr< CRpcLoopbackLink, MT::CMutex > m_link;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CRpcProtocolTaskPtr m_task;
    CORE::CString m_threadPoolName;
    CORE::CObserver m_pulseObserver;
    CRpcTransportProperties m_properties;
    CRpcPrincipal m_principal;
    CORE::CUri m_remoteUri;
    bool m_connected;
    bool m_closed;
    bool m_pulsesRequested;                 /**< periodic pulses asked for while frames wait */
    bool m_deliveryHeld;                    /**< accepted by a listener that did not announce it yet: frames wait */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORT_H ? */
