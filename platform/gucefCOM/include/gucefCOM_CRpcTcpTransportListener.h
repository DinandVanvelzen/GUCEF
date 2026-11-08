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

#ifndef GUCEF_COM_CRPCTCPTRANSPORTLISTENER_H
#define GUCEF_COM_CRPCTCPTRANSPORTLISTENER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#include "gucefCOMCORE_CIRpcTransportListener.h"
#define GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CIRPCTRANSPORTLISTENER_H ? */

#ifndef GUCEF_COM_CRPCTCPTRANSPORT_H
#include "gucefCOM_CRpcTcpTransport.h"
#define GUCEF_COM_CRPCTCPTRANSPORT_H
#endif /* GUCEF_COM_CRPCTCPTRANSPORT_H ? */

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

/**
 *  Accepts RPC sessions over TCP (gucefCOMCORE RPC design 12.2): a server socket on the listener's pulse generator
 *  (or, without one, a dedicated task of the task manager); every accepted connection becomes a connected
 *  CRpcTcpTransport raised with TransportAcceptedEvent, on that thread. CRpcHost takes it from there (the first frame
 *  must be a HELLO).
 *
 *  Listen( "tcp://127.0.0.1:0" ) picks a free port, see GetListenUri(). A bind address that is not a loopback address
 *  is refused unless allowNonLoopback is set: there is no TLS for this transport.
 *
 *  Configuration keys: maxConnections (64), maxFrameSize (16384), maxPreHelloFrameSize (65536),
 *  allowNonLoopback (false), threadPoolName ("gucef.rpc.tcp").
 */
class GUCEF_COM_PUBLIC_CPP CRpcTcpTransportListener : public COMCORE::CIRpcTransportListener
{
    public:

    static const UInt32 DefaultMaxConnections = 64;

    CRpcTcpTransportListener( void );
    virtual ~CRpcTcpTransportListener() GUCEF_VIRTUAL_OVERRIDE;

    void SetMaxConnections( UInt32 maxConnections );
    void SetMaxFrameSize( UInt32 maxFrameSize );
    void SetAllowNonLoopback( bool allowNonLoopback );
    UInt32 GetConnectionCount( void ) const;

    virtual const CORE::CString& GetTransportType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Listen( const CORE::CUri& local ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void StopListening( void ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsListening( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CUri GetListenUri( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::PulseGeneratorPtr GetPulseGenerator( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  The lock of the notifier: TransportAcceptedEvent is raised on the protocol thread while observers subscribe
     *  and unsubscribe from other threads
     */
    virtual MT::TLockStatus Lock( UInt32 lockWaitTimeoutInMs = GUCEF_MT_DEFAULT_LOCK_TIMEOUT_IN_MS ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual MT::TLockStatus Unlock( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< COMCORE::CTCPServerConnection*, CRpcTcpTransportPtr > TTransportMap;

    CRpcTcpTransportListener( const CRpcTcpTransportListener& src );                /**< not implemented */
    CRpcTcpTransportListener& operator=( const CRpcTcpTransportListener& src );     /**< not implemented */

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    COMCORE::CTCPServerSocket* m_socket;
    TTransportMap m_transports;
    GUCEF::vector< CRpcTcpTransportPtr > m_retired;     /**< gone connections, released at the next pulse: never inside their own events */
    CORE::PulseGeneratorPtr m_pulseGenerator;
    COMCORE::CRpcProtocolTaskPtr m_task;
    CORE::CString m_host;
    bool m_listening;
    UInt32 m_maxConnections;
    UInt32 m_maxFrameSize;
    UInt32 m_maxPreHelloFrameSize;
    bool m_allowNonLoopback;
    CORE::CString m_threadPoolName;
    mutable MT::CMutex m_notifierLock;          /**< see Lock() */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CRPCTCPTRANSPORTLISTENER_H ? */
