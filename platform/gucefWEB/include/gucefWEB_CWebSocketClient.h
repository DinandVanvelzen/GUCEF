/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef GUCEF_WEB_CWEBSOCKETCLIENT_H
#define GUCEF_WEB_CWEBSOCKETCLIENT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <map>
#include <vector>
#include <string>

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#include "gucefCOMCORE_CTlsClientConnection.h"
#define GUCEF_COMCORE_CTLSCLIENTCONNECTION_H
#endif /* GUCEF_COMCORE_CTLSCLIENTCONNECTION_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#include "gucefWEB_CWebSocketMessageParser.h"
#define GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H
#endif /* GUCEF_WEB_CWEBSOCKETMESSAGEPARSER_H ? */

#ifndef GUCEF_WEB_WEBSOCKETTYPES_H
#include "gucefWEB_WebSocketTypes.h"
#define GUCEF_WEB_WEBSOCKETTYPES_H
#endif /* GUCEF_WEB_WEBSOCKETTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Web socket (RFC 6455) client with a message oriented API for ws:// and wss:// connections.
 *  wss:// goes through the TLS layer of gucefCOMCORE (CTlsClientConnection): the certificate of the server
 *  is verified against the trust store of the system, or against what SetTlsSettings() says. When no TLS
 *  provider is available the connect fails with an ErrorEvent that says why, everything else works.
 *
 *  The opening handshake is performed as a HTTP/1.1 upgrade request over the TCP connection. After the
 *  server accepted it the connection is handed to the web socket protocol: Connect() returns immediately,
 *  progress is reported by events. Pings are answered automatically.
 *
 *  Events (data types in gucefWEB_WebSocketTypes.h):
 *    ConnectedEvent    TWebSocketConnectedEventData   the handshake completed, messages can now be sent
 *    TextMessageEvent  TWebSocketTextMessageEventData
 *    BinaryMessageEvent TWebSocketBinaryMessageEventData
 *    ClosedEvent       TWebSocketClosedEventData      the connection ended (also if it never got established)
 *    ErrorEvent        CORE::TCloneableString         a description of what went wrong, followed by ClosedEvent
 *
 *  Requires the sha1-raw codec (codecspluginSTBRUMMEHASH) for validating the server's handshake response.
 *  Not thread-safe: use from the thread which drives the pulse generator of the client.
 */
class GUCEF_WEB_PUBLIC_CPP CWebSocketClient : public CORE::CObservingNotifier
{
    public:

    static const CORE::CEvent ConnectedEvent;
    static const CORE::CEvent TextMessageEvent;
    static const CORE::CEvent BinaryMessageEvent;
    static const CORE::CEvent ClosedEvent;
    static const CORE::CEvent ErrorEvent;

    static void RegisterEvents( void );

    typedef std::vector< CORE::CString >                    TStringVector;
    typedef std::map< CORE::CString, CORE::CString >        TStringMap;

    enum EState
    {
        STATE_DISCONNECTED ,
        STATE_CONNECTING   ,        /**< TCP connection being established */
        STATE_HANDSHAKING  ,        /**< waiting for the server to accept the upgrade */
        STATE_OPEN         ,
        STATE_CLOSING               /**< we sent a close frame and are waiting for the server's */
    };

    public:

    CWebSocketClient( void );

    explicit CWebSocketClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CWebSocketClient();

    /**
     *  Starts connecting to the given ws:// or wss:// url, for example ws://localhost:9222/devtools/page/ABC?x=y
     *  (wss:// uses port 443 unless the url says otherwise and needs a TLS provider, see SetTlsSettings())
     *  @param subProtocols     sub protocols to offer in order of preference
     *  @param extraHeaders     additional headers for the upgrade request such as Origin or Cookie
     *  @return false if the url is not usable or a connection is already in progress
     */
    bool Connect( const CORE::CString& url                           ,
                  const TStringVector& subProtocols = TStringVector() ,
                  const TStringMap& extraHeaders = TStringMap()       );

    /**
     *  Starts the closing handshake: sends a close frame. ClosedEvent follows when the server answered
     *  or the connection dropped. Use Disconnect() to not wait for that.
     */
    bool Close( UInt16 closeCode = WEBSOCKET_CLOSE_NORMAL, const CORE::CString& reason = CORE::CString() );

    /** Drops the connection immediately without a closing handshake */
    void Disconnect( void );

    EState GetState( void ) const;

    bool IsConnected( void ) const;

    /** The sub protocol the server selected, empty if none */
    const CORE::CString& GetSubProtocol( void ) const;

    bool SendText( const CORE::CString& text );

    bool SendBinary( const void* data, UInt32 dataSize );

    bool SendPing( const CORE::CString& payload = CORE::CString() );

    /**
     *  The largest message taken from the server; a larger one closes the connection with 1009.
     *  Can change at any time, call it on the thread that receives once connected.
     */
    void SetMaxMessageSize( UInt32 maxMessageSizeInBytes );

    /**
     *  How wss:// connections are secured: the trust store, the CA to add, the timeout of the handshake. Taken at Connect().
     *  The default verifies the certificate against the trust store of the system. The ALPN protocol "http/1.1" is added
     *  by Connect() since the upgrade is a HTTP/1.1 request.
     */
    void SetTlsSettings( const COMCORE::CTlsSettings& settings );

    const COMCORE::CTlsSettings& GetTlsSettings( void ) const;

    private:

    CWebSocketClient( const CWebSocketClient& src );
    CWebSocketClient& operator=( const CWebSocketClient& src );

    typedef CORE::CTEventHandlerFunctor< CWebSocketClient > TEventCallback;

    void RegisterSocketEventHandlers( void );

    void OnTcpConnected( CORE::CNotifier* notifier    ,
                         const CORE::CEvent& eventId  ,
                         CORE::CICloneable* eventData );

    void OnTcpDisconnected( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    void OnTlsError( CORE::CNotifier* notifier    ,
                     const CORE::CEvent& eventId  ,
                     CORE::CICloneable* eventData );

    void OnTcpSocketError( CORE::CNotifier* notifier    ,
                           const CORE::CEvent& eventId  ,
                           CORE::CICloneable* eventData );

    void OnTcpDataReceived( CORE::CNotifier* notifier    ,
                            const CORE::CEvent& eventId  ,
                            CORE::CICloneable* eventData );

    bool SendFrame( bool fin, UInt8 opcode, const void* data, UInt32 dataSize );

    // Note that notifying observers can result in the observer deleting this object. NotifyObservers() reports that
    // by returning false and in that case nothing may touch the object anymore. The functions below which
    // return a bool pass that on: false means the object is gone and the caller has to return right away.
    void OnSocketConnected( void );

    bool OnSocketData( const CORE::CDynamicBuffer& data );

    void OnSocketDisconnected( void );

    bool ProcessHandshakeResponse( void );

    bool ProcessMessages( const CWebSocketMessageParser::TMessageVector& messages );

    bool FailAndClose( const CORE::CString& description );

    bool ReportClosed( void );

    /** Closes the socket and, once that is done, reports the close */
    bool CloseSocketAndReport( void );

    private:

    COMCORE::CTlsClientConnection m_socket;
    COMCORE::CTlsSettings m_tlsSettings;
    CORE::CString m_tlsFailure;          /**< what the TLS layer said about the failure that is about to be reported */
    EState m_state;
    CORE::CString m_host;
    UInt16 m_port;
    CORE::CString m_resource;
    CORE::CString m_clientKey;
    TStringVector m_offeredSubProtocols;
    CORE::CString m_subProtocol;
    std::string m_handshakeBuffer;
    CORE::CString m_upgradeRequest;
    CWebSocketMessageParser m_parser;
    UInt32 m_maxMessageSize;
    SWebSocketCloseInfo m_closeInfo;
    bool m_closedReported;
    bool m_suppressDisconnectReport;     /**< set while we close the socket ourselves, we report the close afterwards */
    SWebSocketPeerInfo m_peerInfo;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSOCKETCLIENT_H ? */
