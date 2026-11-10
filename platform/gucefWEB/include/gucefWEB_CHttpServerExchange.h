/*
 *  gucefWEB: GUCEF module providing Web application functionality
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

#ifndef GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#define GUCEF_WEB_CHTTPSERVEREXCHANGE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COMCORE_CHOSTADDRESS_H
#include "CHostAddress.h"
#define GUCEF_COMCORE_CHOSTADDRESS_H
#endif /* GUCEF_COMCORE_CHOSTADDRESS_H ? */

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
class CHttpServerExchange;
typedef CORE::CTSharedPtr< CHttpServerExchange, MT::CMutex > CHttpServerExchangePtr;

/**
 *  One request a CIHttpRequestTakeoverHandler took over, and the way to answer it: a whole response, or a stream
 *  of a close-delimited body (an SSE stream for example) that ends with the connection.
 *
 *  The exchange is made by the server and shared: a handler that answers later keeps GetSharedPtr(). Once the
 *  exchange ended, or its connection closed, every send returns false; the object stays valid while it is held. An
 *  exchange only ever writes to its own connection: exchange ids are never reused, connection objects are, and the
 *  server unlinks an exchange from its connection when the connection closes.
 *
 *  An exchange is used on the thread that drives the pulse generator of its server only, like the server itself.
 */
class GUCEF_WEB_PUBLIC_CPP CHttpServerExchange : public CORE::CTSharedObjCreator< CHttpServerExchange, MT::CMutex >
{
    public:

    enum EState
    {
        EXCHANGE_RECEIVING  ,   /**< the body of the request is still coming in */
        EXCHANGE_OPEN       ,   /**< the handler owns it and has not answered */
        EXCHANGE_STREAMING  ,   /**< the head of a streamed response went out, the body follows */
        EXCHANGE_ENDED      ,   /**< the handler ended it */
        EXCHANGE_CLOSED         /**< the connection closed before the handler ended it */
    };

    virtual ~CHttpServerExchange() GUCEF_VIRTUAL_OVERRIDE;

    CHttpServerExchangePtr GetSharedPtr( void ) const;

    /** Unique for the life of the process, unlike the indices of connections, which are reused */
    UInt64 GetExchangeId( void ) const;

    const COMCORE::CHostAddress& GetRemoteAddress( void ) const;

    EState GetState( void ) const;

    /** The handler can still answer: open or streaming, with the connection up */
    bool IsOpen( void ) const;

    bool IsStreaming( void ) const;

    /**
     *  Sends a whole response and ends the exchange. The connection stays for the next request when keep-alive is
     *  configured and neither the request nor the response asked to close it. A response whose header fields would
     *  break the framing (a CR, LF or NUL in a name or value) is not sent: the client gets a 500 instead.
     *  @return false when the exchange is not open or nothing could be sent
     */
    bool SendResponse( const CHttpResponseData& response );

    /**
     *  Sends the status and the header fields of a streamed response, whose body is delimited by the close of the
     *  connection: Connection: close, no Content-Length and no Transfer-Encoding. The content of 'head' is not sent.
     */
    bool BeginStream( const CHttpResponseData& head );

    /**
     *  A piece of the body of a stream. The bytes the socket did not take yet are bounded by the maxStreamBacklogBytes
     *  of the server settings: beyond it the client is too slow, the server closes the connection and the handler is
     *  told with OnHttpExchangeClosed() on a later pulse.
     */
    bool SendStreamData( const void* data, UInt32 size );

    bool SendStreamData( const CORE::CDynamicBuffer& data );

    /** Ends the stream; the connection closes once the bytes that were sent are out */
    bool EndStream( void );

    /** The bytes the connection has queued that the socket did not take yet */
    UInt32 GetStreamBacklogBytes( void ) const;

    private:

    friend class CHTTPServer;

    CHttpServerExchange( CHTTPServer* server, UInt32 connectionIndex, UInt64 exchangeId, const COMCORE::CHostAddress& remoteAddress );

    CHttpServerExchange( const CHttpServerExchange& src );              /**< not implemented */
    CHttpServerExchange& operator=( const CHttpServerExchange& src );   /**< not implemented */

    CHTTPServer* m_server;
    UInt32 m_connectionIndex;
    UInt64 m_exchangeId;
    COMCORE::CHostAddress m_remoteAddress;
    EState m_state;
    bool m_aborted;             /**< the server gave up on the connection, the close is on its way */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CHTTPSERVEREXCHANGE_H ? */
