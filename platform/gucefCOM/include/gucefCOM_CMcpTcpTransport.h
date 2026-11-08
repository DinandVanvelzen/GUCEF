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

#ifndef GUCEF_COM_CMCPTCPTRANSPORT_H
#define GUCEF_COM_CMCPTCPTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CTCPSERVERSOCKET_H
#include "CTCPServerSocket.h"
#define GUCEF_COMCORE_CTCPSERVERSOCKET_H
#endif /* GUCEF_COMCORE_CTCPSERVERSOCKET_H ? */

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#include "gucefCOM_CIMcpTransport.h"
#define GUCEF_COM_CIMCPTRANSPORT_H
#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPSTREAMCHANNEL_H
#include "gucefCOM_CMcpStreamChannel.h"
#define GUCEF_COM_CMCPSTREAMCHANNEL_H
#endif /* GUCEF_COM_CMCPSTREAMCHANNEL_H ? */

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
 *  MCP over TCP with the framing of stdio: a custom transport over a byte stream, for which the specification
 *  recommends the stdio framing. Every connection is a channel of kind CHANNEL_STREAM with a line buffer and a legacy
 *  session of its own. The server socket runs on the pulse generator of the server and refuses to run on any other.
 *
 *  Binding to the loopback address is not an authentication boundary: every process of the machine, and every web page
 *  through the browser, can connect. The transport therefore defends itself:
 *
 *  - Cross-protocol defence. A browser can make an HTTP request to any local port; its request line and headers
 *    arrive as lines. The first line of a connection that is not a JSON object closes the connection at once, before
 *    a later line (the JSON body an attacker puts behind the headers) is read. A later line that is not a JSON object
 *    closes it as well.
 *  - Authentication. With requireToken, which is on by default, every request carries the token in
 *    _meta["com.vanvelzensoftware.gucef/authToken"] (CMcpMetaKeys::GucefAuthToken), compared in constant time with the
 *    token of the transport. This is per request, so the transport stays stateless. A request without a valid token
 *    is answered with -31003, and the connection closes after three of them. The token is never logged. It comes
 *    from SetToken(), or from the file named by tokenFile; a file that does not exist is made, with a random token
 *    in it and for the owner only from its creation on (CORE::CreateOwnerOnlyFile()); an existing file is only read
 *    when it is a regular file of the user that runs the process that nobody else can access
 *    (CORE::ReadOwnerOnlyFile()), never through a symbolic link. Without requireToken a warning is logged at start:
 *    that is for tests and for a process that is alone on the machine. A request is audited as authenticated, with
 *    the subject "local:tcp-token", only once its token matched; before that its subject is "local:tcp-unverified".
 *  - Connections that hold a place. A connection that has not made a request that passed the checks above within
 *    firstRequestTimeoutMs of its accept is closed. The socket takes one connection more than maxConnections, so a
 *    new client is always accepted; when that makes one too many, the oldest connection without such a request is
 *    closed to make room. When every place has a client that made such a request, the new connection is held: what
 *    it sends waits (up to the message size limit) until a place is free, as the backlog of the socket held it before.
 *  - TLS, optional. With SetTlsSettings() (a certificate and key) every connection is protected by TLS: the client
 *    speaks TLS first and the lines above inside it. TLS that cannot be used makes Start() fail.
 *  - Loopback unless TLS. A bind address that is not a loopback address (127.x.x.x) is refused unless TLS is used,
 *    or allowNonLoopback is set, which logs a warning: plain TCP on the network.
 *
 *  Configuration keys: bindAddress ("127.0.0.1"), port (0: a free one, see GetBoundPort()), maxConnections (8),
 *  requireToken (true), tokenFile (none), allowNonLoopback (false), firstRequestTimeoutMs (10000), and the keys of
 *  CTlsServerSettings (certificateChainPath, privateKeyPath, ...) in the same node.
 */
class GUCEF_COM_PUBLIC_CPP CMcpTcpTransport : public CIMcpTransport
{
    public:

    /** The number of requests without a valid token after which a connection is closed */
    static const UInt32 MaxTokenFailuresPerConnection = 3;

    /** The time a new connection has for a request that passes the checks of the transport */
    static const UInt32 DefaultFirstRequestTimeoutMs = 10000;

    CMcpTcpTransport( void );

    virtual ~CMcpTcpTransport() GUCEF_VIRTUAL_OVERRIDE;

    // settings, before Start()

    void SetBindAddress( const CORE::CString& bindAddress );
    void SetPort( UInt16 port );
    void SetMaxConnections( UInt32 maxConnections );
    void SetRequireToken( bool requireToken );
    void SetTokenFile( const CORE::CString& tokenFile );
    void SetAllowNonLoopback( bool allowNonLoopback );
    void SetFirstRequestTimeoutMs( UInt32 timeoutInMs );

    /** TLS for every connection: settings with a certificate and key turn it on */
    void SetTlsSettings( const COMCORE::CTlsServerSettings& settings );
    const COMCORE::CTlsServerSettings& GetTlsSettings( void ) const;

    /** The token to require, instead of the one of the token file. It is not part of the configuration */
    void SetToken( const CORE::CString& token );

    /** The token in use after Start(), for the application that hands it to its client */
    CORE::CString GetToken( void ) const;

    /** The port in use: that of the settings, or the free one that was picked for port 0. 0 while not active */
    UInt16 GetBoundPort( void ) const;

    UInt32 GetConnectionCount( void ) const;

    virtual const CORE::CString& GetTransportName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Start( CMcpServer& server ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( bool graceful ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CString GetEndpointDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Whether an IPv4 address in dotted text is a loopback address (127.0.0.0/8) */
    static bool IsLoopbackAddress( const CORE::CString& address );

    private:

    class CTcpChannel;
    friend class CTcpChannel;

    struct SConnectionRecord
    {
        COMCORE::CTCPServerConnection* connection;
        CTcpChannel* channel;
        CMcpChannelPtr channelPtr;
        UInt64 connectedAtInMs;
        bool held;                          /**< accepted beyond maxConnections while every place has a verified client: waits for a place */
        CORE::CDynamicBuffer heldBytes;     /**< what a held connection sent, processed once it has a place */

        SConnectionRecord( void ) : connection( GUCEF_NULL ), channel( GUCEF_NULL ), channelPtr(), connectedAtInMs( 0 ), held( false ), heldBytes() {}
    };
    typedef GUCEF::map< COMCORE::CTCPServerConnection*, SConnectionRecord > TConnectionMap;

    CMcpTcpTransport( const CMcpTcpTransport& src );
    CMcpTcpTransport& operator=( const CMcpTcpTransport& src );

    void OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientData( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    void ReleaseHeldConnection( void );

    /** From a channel: the peer must go. The channel closes at the server at once; the connection closes when the read that caused it is done */
    void OnChannelWantsClose( CTcpChannel& channel );

    bool PrepareToken( CORE::CString& problem );

    bool IsTokenValid( const CORE::CString& given ) const;

    void CloseConnection( COMCORE::CTCPServerConnection* connection );

    void Release( void );

    private:

    CMcpServer* m_server;
    COMCORE::CTCPServerSocket* m_socket;
    TConnectionMap m_connections;
    bool m_active;
    CORE::CString m_bindAddress;
    UInt16 m_port;
    UInt32 m_maxConnections;
    bool m_requireToken;
    CORE::CString m_tokenFile;
    bool m_allowNonLoopback;
    COMCORE::CTlsServerSettings m_tlsSettings;
    UInt32 m_firstRequestTimeoutMs;
    UInt64 m_nextTimeoutCheckInMs;
    CORE::PulseGeneratorPtr m_pulseGenerator;
    CORE::CString m_token;
    UInt8 m_tokenDigest[ 32 ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPTCPTRANSPORT_H ? */
