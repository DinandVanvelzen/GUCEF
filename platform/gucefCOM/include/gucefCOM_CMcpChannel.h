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

#ifndef GUCEF_COM_CMCPCHANNEL_H
#define GUCEF_COM_CMCPCHANNEL_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_COBSERVINGNOTIFIER_H
#include "CObservingNotifier.h"
#define GUCEF_CORE_COBSERVINGNOTIFIER_H
#endif /* GUCEF_CORE_COBSERVINGNOTIFIER_H ? */

#ifndef GUCEF_COM_CJSONRPCENDPOINT_H
#include "gucefCOM_CJsonRpcEndpoint.h"
#define GUCEF_COM_CJSONRPCENDPOINT_H
#endif /* GUCEF_COM_CJSONRPCENDPOINT_H ? */

#ifndef GUCEF_COM_CMCPPRINCIPAL_H
#include "gucefCOM_CMcpPrincipal.h"
#define GUCEF_COM_CMCPPRINCIPAL_H
#endif /* GUCEF_COM_CMCPPRINCIPAL_H ? */

#ifndef GUCEF_COM_CMCPREQUESTMETA_H
#include "gucefCOM_CMcpRequestMeta.h"
#define GUCEF_COM_CMCPREQUESTMETA_H
#endif /* GUCEF_COM_CMCPREQUESTMETA_H ? */

#ifndef GUCEF_COM_CMCPLEGACYSESSION_H
#include "gucefCOM_CMcpLegacySession.h"
#define GUCEF_COM_CMCPLEGACYSESSION_H
#endif /* GUCEF_COM_CMCPLEGACYSESSION_H ? */

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

class CMcpServer;

/**
 *  One conversation scope of a transport: the stdio process, one TCP connection, one HTTP exchange. It owns a
 *  CJsonRpcEndpoint in the strict dialect with batches and null request ids refused, and is that endpoint's
 *  transport. Transports derive from it and implement SendMessageText(): CMcpStreamChannel writes one line per
 *  message, the HTTP exchange channel a JSON body or SSE events.
 *
 *  A transport makes its channel, hands it to CMcpServer::OpenChannel() and gives it every message it receives
 *  through ProcessIncomingMessage(), on the protocol thread of the server. The channel checks the size and the
 *  nesting depth of a message before it is parsed. A channel is no key for state: the protocol is stateless, except
 *  for the legacy session of a stream channel.
 */
class GUCEF_COM_PUBLIC_CPP CMcpChannel : public CORE::CObservingNotifier ,
                                         public CIJsonRpcTransport       ,
                                         private CIJsonRpcRequestHandler
{
    public:

    enum EKind
    {
        CHANNEL_STREAM      ,   /**< stdio, TCP: many requests, server notifications between them */
        CHANNEL_EXCHANGE        /**< HTTP: one request, notifications scoped to it */
    };

    static const UInt32 DefaultMaxMessageBytes = 4194304;
    static const UInt32 DefaultMaxJsonDepth = 64;

    CMcpChannel( EKind kind, const CMcpPrincipal& principal );

    virtual ~CMcpChannel() GUCEF_VIRTUAL_OVERRIDE;

    EKind GetKind( void ) const;

    const CMcpPrincipal& GetPrincipal( void ) const;

    /** Assigned by the server when the channel opens; 0 before. For logs and audit only */
    UInt64 GetChannelId( void ) const;

    /** Closed by CMcpServer::CloseChannel(), or never opened: nothing is sent from then on */
    bool IsClosed( void ) const;

    bool IsOpen( void ) const;

    /** The 2025-11-25 session of a stream channel, GUCEF_NULL when it has none */
    CMcpLegacySession* GetLegacySession( void );
    const CMcpLegacySession* GetLegacySession( void ) const;

    /**
     *  For the transport: one complete message, without its framing. A message larger than the limit is refused
     *  with -32600 without an id; one nested deeper than the limit with -32700, before it reaches the parser.
     *  @return false when the message was refused before it was parsed
     */
    bool ProcessIncomingMessage( const CORE::CString& messageText );

    /** For the transport: a message that was too large to be kept was skipped; answered with -32600 without an id */
    void ProcessMessageTooLarge( UInt32 messageBytes );

    /** Sends a result response */
    bool SendResult( const CORE::CVariant& id, const CORE::CDataNode& result );

    /** Sends an error response; an empty id is written as null. OnErrorCode() is told the code first */
    bool SendError( const CORE::CVariant& id, Int32 code, const CORE::CString& message, const CORE::CDataNode* data = GUCEF_NULL );

    bool SendNotification( const CORE::CString& method, const CORE::CDataNode* params );

    /** The deepest nesting of objects and arrays in JSON text, counted outside strings; stops counting above maxDepth */
    static UInt32 GetJsonNestingDepth( const char* text, UInt32 size, UInt32 maxDepth );

    /** What the check of a message text before the parser found */
    enum EJsonTextProblem
    {
        JSONTEXT_OK                 ,
        JSONTEXT_TOO_DEEP           ,   /**< nested deeper than the limit: -32700 */
        JSONTEXT_NOT_UTF8           ,   /**< bytes that are no UTF-8: no JSON text, -32700 */
        JSONTEXT_NOT_JSON           ,   /**< no JSON text as RFC 8259 has it (trailing comma, comment, NaN, ...): -32700 */
        JSONTEXT_REPEATED_NAME      ,   /**< a member name occurs twice in one object (as the names decode) */
        JSONTEXT_NUL_CHARACTER      ,   /**< a string holds \u0000, which the parser would cut the string at */
        JSONTEXT_UNPAIRED_SURROGATE     /**< a \u escape of a surrogate that is not part of a pair */
    };

    struct GUCEF_COM_PUBLIC_CPP SJsonTextCheck
    {
        EJsonTextProblem problem;
        bool problemAtTopLevel;     /**< in the envelope object itself (a name, the method, the id), not inside params */
        CORE::CString detail;       /**< the repeated name */
        bool idIsInt64Min;          /**< the id is the number -9223372036854775808, which the parser gives as a double */

        SJsonTextCheck( void );
    };

    /**
     *  One O(n) pass over a message before the parser: the nesting depth, UTF-8, the JSON grammar of RFC 8259, repeated
     *  member names, U+0000 and unpaired surrogates in strings, and the raw text of the id. The vendored parser is
     *  lenient (trailing commas, comments, single quotes, NaN), shadows a repeated name, cuts a string at U+0000 and
     *  turns the smallest Int64 into a double; only the text still tells.
     */
    static void CheckJsonText( const char* text, UInt32 size, UInt32 maxDepth, SJsonTextCheck& result );

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    /** What a message handed to SendMessageText() is */
    enum EOutgoingKind
    {
        OUTGOING_RESULT         ,   /**< a result response, sent with SendResult() */
        OUTGOING_ERROR          ,   /**< an error response, sent with SendError(); OnErrorCode() was told its code */
        OUTGOING_NOTIFICATION   ,   /**< a notification, sent with SendNotification() */
        OUTGOING_ENDPOINT_ERROR     /**< an error response the endpoint wrote itself: a message that is no valid request */
    };

    /** Inside SendMessageText(): what the message is, so that HTTP can tell a final response from a notification */
    EOutgoingKind GetOutgoingKind( void ) const;

    /** Told of every request the endpoint hands on, before the server dispatches it */
    virtual void OnRequestReceived( const SJsonRpcRequest& request );

    /** Sends one complete message: implemented by the transport */
    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE = 0;

    /** Told the code of every error response before it is written, so that HTTP can choose its status */
    virtual void OnErrorCode( Int32 code );

    /**
     *  The checks of a transport on what accompanies a message, done after the _meta of the request was read:
     *  the HTTP header checks of the Streamable HTTP transport. The default accepts everything.
     *  @param meta     the parsed _meta of a modern request; GUCEF_NULL for a legacy request
     *  @return false to refuse the request with errorCode and errorMessage
     */
    virtual bool ValidateRequestHeaders( const SJsonRpcRequest& request ,
                                         const CMcpRequestMeta* meta    ,
                                         EMcpProtocolEra era            ,
                                         Int32& errorCode               ,
                                         CORE::CString& errorMessage    );

    /**
     *  Told once ValidateRequestHeaders() accepted a request: the transport may complete the principal of that one
     *  request with what the checks verified, as TCP does once the token of the request matched. Until then a request
     *  is audited with the principal of the channel. The default changes nothing.
     */
    virtual void CompleteRequestPrincipal( CMcpPrincipal& principal );

    /**
     *  Told when a request of the channel was cancelled by a notifications/cancelled that came on another channel (a
     *  legacy client over HTTP): no response follows, and an exchange channel ends its exchange. The default does nothing
     */
    virtual void OnRequestCancelled( const CORE::CVariant& id );

    /** Told when the server closed the channel */
    virtual void OnChannelClosed( void );

    /**
     *  Whether a request without modern _meta is a 2025-11-25 request. By default: when initialize opened a session on
     *  this stream channel. The HTTP exchange channel says so from the MCP-Protocol-Version header of its request.
     */
    virtual bool IsLegacyRequest( const SJsonRpcRequest& request ) const;

    CJsonRpcEndpoint& GetEndpoint( void );

    /** The size limit of one message that the server gave the channel when it opened; the default before that */
    UInt32 GetMaxMessageBytes( void ) const;

    private:

    friend class CMcpServer;

    CMcpChannel( const CMcpChannel& src );
    CMcpChannel& operator=( const CMcpChannel& src );

    virtual EJsonRpcRequestResult OnJsonRpcRequest( CJsonRpcEndpoint& endpoint    ,
                                                    const SJsonRpcRequest& request ,
                                                    SJsonRpcResponse& response     ) GUCEF_VIRTUAL_OVERRIDE;

    void OnEndpointNotification( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );

    void AttachToServer( CMcpServer* server, UInt64 channelId, UInt32 maxMessageBytes, UInt32 maxJsonDepth );

    void DetachFromServer( void );

    CMcpLegacySession& OpenLegacySession( void );

    EKind m_kind;
    CMcpPrincipal m_principal;
    CJsonRpcEndpoint m_endpoint;
    CMcpServer* m_server;
    UInt64 m_channelId;
    bool m_closed;
    UInt32 m_maxMessageBytes;
    UInt32 m_maxJsonDepth;
    CMcpLegacySession* m_legacySession;
    EOutgoingKind m_outgoingKind;
    SJsonTextCheck m_incomingCheck;         /**< of the message being processed */
};

typedef CORE::CTSharedPtr< CMcpChannel, MT::CMutex > CMcpChannelPtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPCHANNEL_H ? */
