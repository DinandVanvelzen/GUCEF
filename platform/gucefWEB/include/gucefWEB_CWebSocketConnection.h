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

#ifndef GUCEF_WEB_CWEBSOCKETCONNECTION_H
#define GUCEF_WEB_CWEBSOCKETCONNECTION_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#include "gucefWEB_CIWebSocketUpgradeHandler.h"
#define GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#endif /* GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

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

class CWebSocketServer;

/**
 *  A client connection of a CWebSocketServer, speaking the web socket protocol.
 *
 *  The object is linked directly to the underlying TCP connection: there is no lookup involved in receiving or
 *  sending, you send on the connection object itself. The TCP connection objects of the server are re-used for
 *  later clients, this object is not: once the connection has ended it stays valid to hold on to but IsOpen()
 *  is false and sending fails, it will never send to another client.
 *
 *  Sending can be done from any thread. Everything else, in particular receiving, happens on the thread
 *  which drives the HTTP server's socket.
 */
class GUCEF_WEB_PUBLIC_CPP CWebSocketConnection : public CIWebSocketConnectionHandler                      ,
                                                  public CORE::CTSharedObjCreator< CWebSocketConnection, MT::CMutex >
{
    public:

    typedef CORE::CTSharedPtr< CWebSocketConnection, MT::CMutex >   TWebSocketConnectionPtr;

    /**
     *  Used by CWebSocketServer: links a new connection object to an upgraded TCP connection
     */
    static TWebSocketConnectionPtr Create( CWebSocketServer& server                    ,
                                           COMCORE::CTCPServerConnection& tcpConnection ,
                                           const SWebSocketPeerInfo& peerInfo           ,
                                           UInt32 maxMessageSize                        );

    CWebSocketConnection( void );

    virtual ~CWebSocketConnection();

    /** Identifies the connection among the connections of the server for as long as it is open */
    UInt32 GetId( void ) const;

    const SWebSocketPeerInfo& GetPeerInfo( void ) const;

    /**
     *  A pointer for whoever works with this connection, for example to link a session object to it
     *  without having to look it up on every message. Never touched by the connection itself.
     */
    void SetUserData( void* userData );

    void* GetUserData( void ) const;

    bool IsOpen( void ) const;

    /**
     *  The largest message this connection takes from the client from now on, starting with the server's
     *  CWebSocketServer::SetMaxMessageSize(). A larger message is refused: the connection is closed with 1009.
     *  The limit is checked when a frame header is read, so a too large message is not collected first.
     *  Call it on the thread that receives (for example from within an event of the server) or before data arrives.
     */
    void SetMaxMessageSize( UInt32 maxMessageSizeInBytes );

    UInt32 GetMaxMessageSize( void ) const;

    bool SendText( const CORE::CString& text );

    bool SendBinary( const void* data, UInt32 dataSize );

    bool SendPing( const CORE::CString& payload = CORE::CString() );

    /**
     *  Starts closing the connection: sends a close frame and closes the connection.
     *  The server reports the end of the connection through its disconnected event
     */
    bool Close( UInt16 closeCode = WEBSOCKET_CLOSE_NORMAL, const CORE::CString& reason = CORE::CString() );

    private:

    friend class CWebSocketServer;

    CWebSocketConnection( const CWebSocketConnection& src );
    CWebSocketConnection& operator=( const CWebSocketConnection& src );

    void Initialize( CWebSocketServer& server                    ,
                     COMCORE::CTCPServerConnection& tcpConnection ,
                     const SWebSocketPeerInfo& peerInfo           ,
                     UInt32 maxMessageSize                        );

    /** Called when the owning server goes away: lets go of everything without reporting events */
    void Detach( void );

    // CIWebSocketConnectionHandler: the HTTP server hands what happens to the TCP connection to us
    virtual void OnWebSocketData( const void* data, UInt32 dataSize ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnWebSocketClosed( void ) GUCEF_VIRTUAL_OVERRIDE;

    bool SendFrame( UInt8 opcode, const void* data, UInt32 dataSize );

    void SendCloseAndDrop( UInt16 sendCode, const SWebSocketCloseInfo& reportedCloseInfo );

    private:

    CWebSocketServer* m_server;
    COMCORE::CTCPServerConnection* m_tcpConnection;
    SWebSocketPeerInfo m_peerInfo;
    CWebSocketMessageParser m_parser;
    SWebSocketCloseInfo m_closeInfo;
    bool m_closeSent;
    void* m_userData;
    mutable MT::CMutex m_lock;
};

typedef CWebSocketConnection::TWebSocketConnectionPtr TWebSocketConnectionPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CWEBSOCKETCONNECTION_H ? */
