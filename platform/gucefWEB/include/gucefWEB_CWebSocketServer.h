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

#ifndef GUCEF_WEB_CWEBSOCKETSERVER_H
#define GUCEF_WEB_CWEBSOCKETSERVER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <set>
#include <vector>

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CNOTIFIER_H
#include "CNotifier.h"
#define GUCEF_CORE_CNOTIFIER_H
#endif /* GUCEF_CORE_CNOTIFIER_H ? */

#ifndef GUCEF_WEB_CHTTPSERVER_H
#include "gucefWEB_CHTTPServer.h"
#define GUCEF_WEB_CHTTPSERVER_H
#endif /* GUCEF_WEB_CHTTPSERVER_H ? */

#ifndef GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#include "gucefWEB_CIWebSocketUpgradeHandler.h"
#define GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#endif /* GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H ? */

#ifndef GUCEF_WEB_CWEBSOCKETCONNECTION_H
#include "gucefWEB_CWebSocketConnection.h"
#define GUCEF_WEB_CWEBSOCKETCONNECTION_H
#endif /* GUCEF_WEB_CWEBSOCKETCONNECTION_H ? */

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

/** Event data of the web socket server: every event carries the connection it is about */
struct SWebSocketServerTextMessage
{
    TWebSocketConnectionPtr connection;
    CORE::CString text;                                     /**< valid UTF-8 */
};

struct SWebSocketServerBinaryMessage
{
    TWebSocketConnectionPtr connection;
    CORE::CDynamicBuffer data;
};

struct SWebSocketServerClosedInfo
{
    TWebSocketConnectionPtr connection;
    SWebSocketCloseInfo closeInfo;
};

typedef CORE::CTCloneableObj< TWebSocketConnectionPtr >             TWebSocketServerConnectedEventData;
typedef CORE::CTCloneableObj< SWebSocketServerTextMessage >         TWebSocketServerTextMessageEventData;
typedef CORE::CTCloneableObj< SWebSocketServerBinaryMessage >       TWebSocketServerBinaryMessageEventData;
typedef CORE::CTCloneableObj< SWebSocketServerClosedInfo >          TWebSocketServerClosedEventData;

/*-------------------------------------------------------------------------*/

/**
 *  Web socket (RFC 6455) server with a message oriented API.
 *
 *  The HTTP side of the opening handshake is delegated to a CHTTPServer: either one owned by this class
 *  or an existing one which then keeps serving normal HTTP requests on the same port next to web sockets.
 *  Every client is represented by a CWebSocketConnection which you can send on directly. Pings are answered
 *  automatically and the closing handshake is handled for you.
 *
 *  Events (sent from the thread driving the HTTP server, data types are defined above):
 *    ClientConnectedEvent    TWebSocketServerConnectedEventData
 *    TextMessageEvent        TWebSocketServerTextMessageEventData
 *    BinaryMessageEvent      TWebSocketServerBinaryMessageEventData
 *    ClientDisconnectedEvent TWebSocketServerClosedEventData
 *
 *  Requires the sha1-raw codec (codecspluginSTBRUMMEHASH).
 */
class GUCEF_WEB_PUBLIC_CPP CWebSocketServer : public CORE::CNotifier ,
                                              public CIWebSocketUpgradeHandler
{
    public:

    static const CORE::CEvent ClientConnectedEvent;
    static const CORE::CEvent ClientDisconnectedEvent;
    static const CORE::CEvent TextMessageEvent;
    static const CORE::CEvent BinaryMessageEvent;

    static void RegisterEvents( void );

    typedef std::vector< CORE::CString >            TStringVector;
    typedef std::vector< TWebSocketConnectionPtr >  TConnectionVector;

    public:

    /**
     *  Creates a server which owns its own HTTP server. Call Listen() to start accepting clients
     */
    CWebSocketServer( void );

    /**
     *  Attaches to an existing HTTP server which must outlive this object.
     *  The HTTP server must have allowWebSocketUpgrades enabled in its settings.
     */
    explicit CWebSocketServer( CHTTPServer& httpServer );

    virtual ~CWebSocketServer();

    /** Only for a server that owns its HTTP server: start listening on the given port */
    bool Listen( UInt16 port );

    void Close( void );

    UInt16 GetPort( void ) const;

    CHTTPServer& GetHttpServer( void );

    /**
     *  Sub protocols this server supports in order of preference. If the list is not empty a client that
     *  offers sub protocols none of which is supported is refused. An empty list accepts every client without
     *  negotiating a sub protocol.
     */
    void SetSupportedSubProtocols( const TStringVector& subProtocols );

    /**
     *  The largest message a connection takes, for connections made from now on.
     *  A connection can change its own limit at any time with CWebSocketConnection::SetMaxMessageSize().
     */
    void SetMaxMessageSize( UInt32 maxMessageSizeInBytes );

    UInt32 GetMaxMessageSize( void ) const;

    /** Sends to all connected clients, returns the number of clients it was sent to */
    UInt32 BroadcastText( const CORE::CString& text );

    UInt32 BroadcastBinary( const void* data, UInt32 dataSize );

    UInt32 GetConnectionCount( void ) const;

    void GetConnections( TConnectionVector& connections ) const;

    private:

    friend class CWebSocketConnection;

    CWebSocketServer( const CWebSocketServer& src );
    CWebSocketServer& operator=( const CWebSocketServer& src );

    virtual bool OnWebSocketUpgradeRequested( CHTTPServer& server                          ,
                                              COMCORE::CTCPServerConnection& connection    ,
                                              const CHttpRequestData& request              ,
                                              CHttpResponseData& response                  ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnWebSocketConnectionEstablished( CHTTPServer& server                       ,
                                                   COMCORE::CTCPServerConnection& connection ,
                                                   const CHttpRequestData& request           ) GUCEF_VIRTUAL_OVERRIDE;

    // Reporting by the connections. Notifying observers can result in them deleting this server, NotifyObservers()
    // reports that by returning false. These functions pass that on: false means this object is gone and the
    // connection must stop what it is doing right away without touching the server again.
    bool OnConnectionText( const TWebSocketConnectionPtr& connection, const CORE::CString& text );
    bool OnConnectionBinary( const TWebSocketConnectionPtr& connection, const CORE::CDynamicBuffer& data );
    bool OnConnectionClosed( const TWebSocketConnectionPtr& connection, const SWebSocketCloseInfo& closeInfo );

    private:

    typedef std::set< TWebSocketConnectionPtr > TConnectionSet;

    CHTTPServer* m_httpServer;
    bool m_ownsHttpServer;
    TStringVector m_supportedSubProtocols;
    UInt32 m_maxMessageSize;
    TConnectionSet m_connections;           /**< only for enumeration and teardown, not touched when sending or receiving */
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSOCKETSERVER_H ? */
