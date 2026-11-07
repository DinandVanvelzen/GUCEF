/*
 *  gucefWEB: GUCEF module providing WEB application functionality
 *  Copyright (C) 2002 - 2026.  Dinand Vanvelzen
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

#ifndef GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H
#define GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#include "CTCPServerConnection.h"
#define GUCEF_COMCORE_CTCPSERVERCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPSERVERCONNECTION_H ? */

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPRESPONSEDATA_H
#include "gucefWEB_CHttpResponseData.h"
#define GUCEF_WEB_CHTTPRESPONSEDATA_H
#endif /* GUCEF_WEB_CHTTPRESPONSEDATA_H ? */

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

class CHTTPServer;

/**
 *  Receives the data and the end of a connection which was upgraded to the web socket protocol.
 *  The handler of the upgrade links an implementation to the TCP connection through its user data slot:
 *  \code
 *  connection.SetUserData( static_cast< CIWebSocketConnectionHandler* >( myHandler ) );
 *  \endcode
 *  From then on the HTTP server hands everything that happens to the connection to it. There is no lookup
 *  involved in that: it is a pointer on the connection object itself.
 *  The callbacks are made from the thread which drives the HTTP server's socket.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebSocketConnectionHandler
{
    public:

    CIWebSocketConnectionHandler( void ) {}

    virtual ~CIWebSocketConnectionHandler() {}

    /** Raw bytes received on the connection, not necessarily aligned to frame boundaries */
    virtual void OnWebSocketData( const void* data, UInt32 dataSize ) = 0;

    /** The connection ended, either side may have closed it. The link on the connection is removed afterwards */
    virtual void OnWebSocketClosed( void ) = 0;
};

/*-------------------------------------------------------------------------*/

/**
 *  Interface for a party that takes over HTTP connections which upgrade to the web socket protocol.
 *  The HTTP server performs the opening handshake: it validates the request headers and sends the 101 response.
 *  From then on the connection no longer carries HTTP and data for it goes to the connection's
 *  CIWebSocketConnectionHandler instead of the HTTP request parser.
 *  CWebSocketServer implements this interface.
 *
 *  Callbacks are made from the thread which drives the HTTP server's socket.
 */
class GUCEF_WEB_PUBLIC_CPP CIWebSocketUpgradeHandler
{
    public:

    CIWebSocketUpgradeHandler( void ) {}

    virtual ~CIWebSocketUpgradeHandler() {}

    /**
     *  Called after the HTTP server validated the upgrade request but before it accepts it.
     *  The handler can adjust the 101 response (for example to select a sub protocol) or refuse the upgrade
     *  by returning false in which case the response, which should then have a status code of 400 or above, is sent
     *  and the connection is closed. Origin checks and authentication belong here.
     */
    virtual bool OnWebSocketUpgradeRequested( CHTTPServer& server                 ,
                                              COMCORE::CTCPServerConnection& connection ,
                                              const CHttpRequestData& request    ,
                                              CHttpResponseData& response         ) = 0;

    /**
     *  The 101 response was sent, the connection now speaks the web socket protocol.
     *  The handler takes over the connection and must link a CIWebSocketConnectionHandler to it, see above.
     *  Bytes the client sent behind its upgrade request, in the same read, are handed to that connection handler
     *  right after this returns.
     */
    virtual void OnWebSocketConnectionEstablished( CHTTPServer& server                 ,
                                                   COMCORE::CTCPServerConnection& connection ,
                                                   const CHttpRequestData& request    ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CIWEBSOCKETUPGRADEHANDLER_H ? */
