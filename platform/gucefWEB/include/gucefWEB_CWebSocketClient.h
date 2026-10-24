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

#ifndef GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#include "CTCPClientSocket.h"
#define GUCEF_COMCORE_CTCPCLIENTSOCKET_H
#endif /* GUCEF_COMCORE_CTCPCLIENTSOCKET_H ? */

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
 *  Web socket (RFC 6455) client with a message oriented API for plain ws:// connections.
 *  (wss:// requires TLS which is not available in this module)
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
     *  Starts connecting to the given ws:// url, for example ws://localhost:9222/devtools/page/ABC?x=y
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

    private:

    CWebSocketClient( const CWebSocketClient& src );
    CWebSocketClient& operator=( const CWebSocketClient& src );

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventid               ,
                           CORE::CICloneable* eventdata /* = NULL */ ) GUCEF_VIRTUAL_OVERRIDE;

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

    COMCORE::CTCPClientSocket m_socket;
    EState m_state;
    CORE::CString m_host;
    UInt16 m_port;
    CORE::CString m_resource;
    CORE::CString m_clientKey;
    TStringVector m_offeredSubProtocols;
    CORE::CString m_subProtocol;
    std::string m_handshakeBuffer;
    std::string m_upgradeRequest;
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
