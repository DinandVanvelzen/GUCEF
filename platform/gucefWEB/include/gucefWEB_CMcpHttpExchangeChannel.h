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

#ifndef GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H
#define GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COM_CMCPCHANNEL_H
#include "gucefCOM_CMcpChannel.h"
#define GUCEF_COM_CMCPCHANNEL_H
#endif /* GUCEF_COM_CMCPCHANNEL_H ? */

#ifndef GUCEF_WEB_CHTTPREQUESTDATA_H
#include "gucefWEB_CHttpRequestData.h"
#define GUCEF_WEB_CHTTPREQUESTDATA_H
#endif /* GUCEF_WEB_CHTTPREQUESTDATA_H ? */

#ifndef GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#include "gucefWEB_CHttpServerExchange.h"
#define GUCEF_WEB_CHTTPSERVEREXCHANGE_H
#endif /* GUCEF_WEB_CHTTPSERVEREXCHANGE_H ? */

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

class CMcpStreamableHttpTransport;

/**
 *  The MCP channel of one HTTP POST of the Streamable HTTP transport: one request, or one notification.
 *
 *  It answers with a JSON body (application/json, with a Content-Length) when the response is all there is, and
 *  with a request-scoped SSE stream (text/event-stream, closed with the response) as soon as a notification has to
 *  go before the response: progress, or the notifications of subscriptions/listen. Every message is one event of
 *  one "data:" line. The HTTP status follows the error code of the response while the response has not started
 *  (see StatusForErrorCode()); once a stream is open an error is an event like any other.
 *
 *  For modern requests it checks the headers that mirror the body: MCP-Protocol-Version, Mcp-Method, Mcp-Name and
 *  the Mcp-Param-{Name} headers of the x-mcp-header parameters of a tool, with -32020 HeaderMismatch on a missing,
 *  malformed or different header. A request with MCP-Protocol-Version 2025-11-25 and no modern _meta is a legacy
 *  request; its Mcp-* headers are not checked.
 *
 *  Used on the thread of the MCP server, which is the thread of the HTTP server.
 */
class GUCEF_WEB_PUBLIC_CPP CMcpHttpExchangeChannel : public COM::CMcpChannel
{
    public:

    /** The headers of the request this channel needs, taken from the request before its body is processed */
    CMcpHttpExchangeChannel( const COM::CMcpPrincipal& principal          ,
                             CMcpStreamableHttpTransport& transport       ,
                             const CHttpServerExchangePtr& exchange       ,
                             const CHttpRequestData& request              );

    virtual ~CMcpHttpExchangeChannel() GUCEF_VIRTUAL_OVERRIDE;

    /** After the body went through ProcessIncomingMessage(): a body without a request is answered 202 Accepted */
    void FinishIncoming( void );

    /** The connection went away: nothing is sent from now on */
    void DropExchange( void );

    /** Sends an SSE comment on an open stream, to keep it alive through intermediaries */
    bool SendKeepAlive( void );

    bool HasResponded( void ) const;

    bool IsStreaming( void ) const;

    UInt64 GetExchangeId( void ) const;

    /**
     *  The HTTP status of an error response that is sent as a whole response:
     *  -32700, -32600, -32020, -32021, -32022 and a -32602 before the header checks (a malformed _meta) 400;
     *  -32601 404 for a modern request (200 for a legacy one); -31003 403; -31001 429; -31002 503; else 200
     */
    static int StatusForErrorCode( Int32 code, bool afterHeaderChecks, bool legacy );

    /**
     *  The value of an Mcp-Name or Mcp-Param-{Name} header: as it is, or the UTF-8 text in the base64 between
     *  "=?base64?" and "?=". False for text that is not a valid header value, or a sentinel that does not decode
     */
    static bool DecodeHeaderValue( const CORE::CString& headerValue, CORE::CString& decoded );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnErrorCode( Int32 code ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool ValidateRequestHeaders( const COM::SJsonRpcRequest& request ,
                                         const COM::CMcpRequestMeta* meta    ,
                                         COM::EMcpProtocolEra era            ,
                                         Int32& errorCode                    ,
                                         CORE::CString& errorMessage         ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsLegacyRequest( const COM::SJsonRpcRequest& request ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnChannelClosed( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnRequestCancelled( const CORE::CVariant& id ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnRequestReceived( const COM::SJsonRpcRequest& request ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::map< CORE::CString, CORE::CString > TStringMap;

    CMcpHttpExchangeChannel( const CMcpHttpExchangeChannel& src );
    CMcpHttpExchangeChannel& operator=( const CMcpHttpExchangeChannel& src );

    bool CheckParamHeaders( const COM::SJsonRpcRequest& request, Int32& errorCode, CORE::CString& errorMessage );
    bool SendWholeResponse( int status, const CORE::CString& body );
    bool BeginStream( void );
    void Done( void );

    CMcpStreamableHttpTransport* m_transport;
    CHttpServerExchangePtr m_exchange;
    UInt64 m_exchangeId;
    bool m_hasProtocolVersionHeader;
    CORE::CString m_protocolVersionHeader;
    bool m_hasMethodHeader;
    CORE::CString m_methodHeader;
    bool m_hasNameHeader;
    CORE::CString m_nameHeader;
    TStringMap m_paramHeaders;          /**< by the lowercased {Name} of Mcp-Param-{Name} */
    bool m_acceptsJson;
    bool m_acceptsSse;
    bool m_requestReceived;
    bool m_headerChecksDone;
    bool m_legacy;
    bool m_responded;
    bool m_streaming;
    int m_errorStatus;                  /**< the status of the next error response, set by OnErrorCode() */
    int m_forcedStatus;                 /**< a status the header checks decided, which wins over the error code */
    CORE::CDynamicBuffer m_eventBuffer; /**< reused for every event */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CMCPHTTPEXCHANGECHANNEL_H ? */
