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

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

#ifndef GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#include "gucefCOMCORE_CTlsProviderRegistry.h"
#define GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H
#endif /* GUCEF_COMCORE_CTLSPROVIDERREGISTRY_H ? */

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
 *  A TCP client connection that can be protected with TLS. It owns a CTCPClientSocket and raises the events of a
 *  CTCPConnection itself, carrying plaintext: a consumer of CTCPClientSocket changes the type of its member and
 *  the call that connects, its event subscriptions and the event ids stay.
 *
 *  Everything happens on the thread of the pulse generator given to the constructor, driven by the pulses of the
 *  socket: nothing here creates a thread or pumps a pulse generator. A synchronous caller runs its own pulse
 *  generator as it does with the socket, an asynchronous one does that in a pool task.
 *
 *  Without settings the connection is plain TCP and the events of the socket are passed on (http, ws). With settings
 *  (ConnectTo() or, on an already connected socket, StartTls()) the handshake runs on the pulses and:
 *  - ConnectedEvent is raised after the handshake, so code written for plain TCP that sends its request on
 *    ConnectedEvent works unchanged. Data sent before that is refused (Send() is false).
 *  - HandshakeCompletedEvent (data: CTlsSessionInfo) comes just before it.
 *  - DataRecievedEvent and DataSentEvent have plaintext, the ones of the socket are consumed here.
 *  - A failure raises TlsErrorEvent (data: CTlsError) and then SocketErrorEvent: for a failure of the socket (DNS,
 *    connect) with the code of the socket, for a TLS failure with GUCEF_COMCORE_TLS_ERRORCODE_BASE + the error class.
 *    No plaintext is ever delivered after a failure. Then the connection closes and raises DisconnectedEvent.
 *  - A close of the connection without TLS close notification while established ends in DisconnectedEvent with
 *    GetSessionInfo().peerClosedCleanly false: the stream may be truncated.
 *
 *  An observer that wants to destroy the connection from inside one of its events does that with
 *  ScheduleForDestruction() like for every notifier: the object is deleted when the delivery of the event is over.
 *  NotifyObservers() reports that and this class stops touching itself then, like the socket does.
 */
class GUCEF_COMCORE_EXPORT_CPP CTlsClientConnection : public CTCPConnection
{
    public:

    static const CORE::CEvent HandshakeCompletedEvent;      /**< data: TTlsSessionInfoEventData */
    static const CORE::CEvent TlsErrorEvent;                /**< data: TTlsErrorEventData */

    /**
     *  What is read from the socket in one pulse at most, and in one recv call: the SetMaxRead() of the socket. The socket reads
     *  1 KiB blocks and drains everything that is there unless told otherwise, which for TLS (records of up to 16 KiB) is a large number
     *  of recv calls. Measured with a download of 16 MiB over loopback (median of 5, the OpenSSL provider): 1 KiB: 99 ms wall and
     *  28 ms CPU, 4 KiB: 164 ms and 160 ms, 16 KiB: 248 ms and 71 ms, 64 KiB: 29 ms and 23 ms. A value is also the limit per pulse, so
     *  one of the size of a TLS record is slower than the default, and 64 KiB is the best of these. Plain connections keep the default.
     */
    static const UInt32 TlsReadBlockSize = 64 * 1024;

    static void RegisterEvents( void );

    public:

    CTlsClientConnection( CORE::PulseGeneratorPtr pulseGenerator ,
                          bool blocking                          );

    explicit CTlsClientConnection( bool blocking );

    virtual ~CTlsClientConnection() GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Connects to the host. With settings the connection is protected by TLS and the host is the name the
     *  certificate is checked for (unless the settings give another). Without, it is plain TCP.
     *  With 'blocking' the call cycles the socket until the connection, and the handshake, are done or have failed
     *  (or the handshake timeout of the settings expired).
     *  @return false if the connection could not be started. When the reason is a TLS one, GetLastError() has it
     *          and no event was raised; failures of the socket are reported by its events as usual
     */
    bool ConnectTo( const CORE::CString& host        ,
                    UInt16 port                      ,
                    const CTlsSettings* settings     ,
                    bool blocking = false            );

    /** Plain TCP, see the variant with settings */
    bool ConnectTo( const CORE::CString& host        ,
                    UInt16 port                      ,
                    bool blocking = false            );

    /**
     *  Upgrades a connected plain connection to TLS, for example after the CONNECT of a proxy was answered. What
     *  arrives after the call is ciphertext. HandshakeCompletedEvent is raised when done, ConnectedEvent is not
     *  raised again.
     *  @param serverName   the name the certificate is checked for, SNI
     *  @param leftover     ciphertext that was already read from the socket as plaintext, behind the last line of the previous protocol
     *  @return false if there is no connected plain connection or no provider is available (GetLastError() says)
     */
    bool StartTls( const CTlsSettings& settings      ,
                   const CORE::CString& serverName   ,
                   const void* leftover = GUCEF_NULL ,
                   UInt32 leftoverSize = 0           );

    /** Whether the connection is protected by TLS or about to be: settings were given */
    bool IsTlsRequested( void ) const;

    /** Whether the handshake is done and the connection is open */
    bool IsEstablished( void ) const;

    /** What was negotiated, empty before the handshake completed */
    const CTlsSessionInfo& GetSessionInfo( void ) const;

    /** The reason of the last TLS failure of this connection, no error when there was none */
    const CTlsError& GetLastError( void ) const;

    /** Which provider registry is asked for a provider, the one of CComCoreGlobal by default. For tests that bring their own */
    void SetProviderRegistry( CTlsProviderRegistry* registry );

    virtual CIPv4Address GetRemoteIP( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetRemoteHostName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt16 GetRemoteTCPPort( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Sends close_notify first, then closes the socket */
    virtual void Close( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Plaintext. With TLS only after the handshake */
    virtual bool Send( const void* dataSource ,
                       const UInt32 dataSize  ) GUCEF_VIRTUAL_OVERRIDE;

    /** Ciphertext waiting in the socket to be sent */
    UInt32 GetBufferedDataToSendInBytes( void ) const;

    virtual bool SetUseTcpSendCoalescing( bool coaleseData ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool GetUseTcpSendCoalescing( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** What went over the wire, so for TLS including the overhead of the protocol */
    virtual UInt32 GetBytesReceived( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual UInt32 GetBytesTransmitted( bool resetCounter ) GUCEF_VIRTUAL_OVERRIDE;

    void SetMaxUpdatesPerCycle( UInt32 maxUpdates );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CTlsClientConnection > TEventCallback;

    CTlsClientConnection( const CTlsClientConnection& src );
    CTlsClientConnection& operator=( const CTlsClientConnection& src );

    void RegisterEventHandlers( void );

    void OnHandshakeTimer( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void OnTcpConnecting( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void OnTcpConnected( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void OnTcpDataReceived( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnTcpDataSent( CORE::CNotifier* notifier    ,
                        const CORE::CEvent& eventId  ,
                        CORE::CICloneable* eventData );

    void OnTcpDisconnected( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnTcpSocketError( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void ResetTlsState( void );
    bool SelectProvider( void );
    bool BeginHandshake( const void* leftover, UInt32 leftoverSize );
    bool SendSessionOutput( void );
    bool ProcessSession( void );
    bool FailTls( const CTlsError& error );
    bool OnSocketData( const CORE::CDynamicBuffer& data );
    bool OnSocketDisconnected( void );
    bool OnSocketError( CORE::CICloneable* eventData );
    void StopHandshakeTimer( void );
    void WaitForHandshake( void );
    CORE::CString GetRemoteDescription( void ) const;

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CTCPClientSocket* m_socket;             /**< not a member: it must outlive a notification that was started by it and ended in the destruction of this object */
    CORE::CTimer* m_handshakeTimer;         /**< same */
    CTlsProviderRegistry* m_registry;
    TITlsProviderPtr m_provider;
    TTlsSessionPtr m_session;
    CTlsSettings m_settings;
    CORE::CString m_serverName;
    CTlsError m_error;
    CTlsSessionInfo m_emptyInfo;
    CORE::CDynamicBuffer m_cipherOut;       /**< what the session produced for the peer, reused for every flush */
    CORE::CDynamicBuffer m_plainIn;         /**< what the session decrypted, reused for every delivery */
    bool m_blockingConnect;
    bool m_tlsRequested;
    bool m_upgraded;                        /**< TLS started with StartTls on a connected plain connection */
    bool m_established;
    bool m_failureReported;
    bool m_localClose;
    UInt32 m_generation;                    /**< changes whenever the connection is closed or started again, so code after an event can tell */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */
