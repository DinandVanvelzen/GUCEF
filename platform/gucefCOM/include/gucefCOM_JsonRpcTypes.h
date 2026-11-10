/*
 *  gucefCOM: GUCEF module providing communication functionality
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

#ifndef GUCEF_COM_JSONRPCTYPES_H
#define GUCEF_COM_JSONRPCTYPES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTCLONEABLEOBJ_H
#include "CTCloneableObj.h"
#define GUCEF_CORE_CTCLONEABLEOBJ_H
#endif /* GUCEF_CORE_CTCLONEABLEOBJ_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Predefined error codes of the JSON-RPC 2.0 specification plus the ones this implementation reports locally */
enum EJsonRpcErrorCode
{
    JSONRPC_ERROR_PARSE             = -32700 ,
    JSONRPC_ERROR_INVALID_REQUEST   = -32600 ,
    JSONRPC_ERROR_METHOD_NOT_FOUND  = -32601 ,
    JSONRPC_ERROR_INVALID_PARAMS    = -32602 ,
    JSONRPC_ERROR_INTERNAL          = -32603 ,

    // reported by the endpoint itself in the response to a call, never sent over the wire
    JSONRPC_ERROR_CALL_TIMEOUT      = -32001 ,
    JSONRPC_ERROR_TRANSPORT_CLOSED  = -32002
};

/**
 *  The protocol dialect. Strict is JSON-RPC 2.0: every message has "jsonrpc":"2.0".
 *  Lenient is for protocols that are JSON-RPC like but leave that field out, such as the Chrome DevTools Protocol.
 *  Lenient writes no version field and accepts messages with and without it.
 */
enum EJsonRpcDialect
{
    JSONRPC_DIALECT_STRICT_2_0 ,
    JSONRPC_DIALECT_LENIENT
};

struct SJsonRpcError
{
    Int32 code;
    CORE::CString message;
    CORE::CDataNode data;           /**< optional extra information, check hasData. Scalar data is in the value of the node */
    bool hasData;

    SJsonRpcError( void ) : code( 0 ), message(), data(), hasData( false ) {}
};

/** What came back for a call */
struct SJsonRpcResponse
{
    CORE::CVariant id;
    bool isError;
    bool isLocalError;              /**< the error was generated here (timeout, transport lost), the peer never answered */
    CORE::CDataNode result;         /**< the result, a node with a value for scalars, an object or array node otherwise */
    SJsonRpcError error;
    CORE::CDataNode envelope;       /**< the complete received message, for protocol specific extra fields such as a session id */

    SJsonRpcResponse( void ) : id(), isError( false ), isLocalError( false ), result(), error(), envelope() {}
};

/** A call made by the peer which is to be answered */
struct SJsonRpcRequest
{
    CORE::CVariant id;
    CORE::CString method;
    CORE::CDataNode params;
    bool hasParams;
    CORE::CDataNode envelope;       /**< the complete received message, for protocol specific extra fields such as a session id */

    SJsonRpcRequest( void ) : id(), method(), params(), hasParams( false ), envelope() {}
};

/** A message from the peer which expects no answer */
struct SJsonRpcNotification
{
    CORE::CString method;
    CORE::CDataNode params;
    bool hasParams;
    CORE::CDataNode envelope;

    SJsonRpcNotification( void ) : method(), params(), hasParams( false ), envelope() {}
};

typedef CORE::CTCloneableObj< SJsonRpcResponse >        TJsonRpcResponseEventData;
typedef CORE::CTCloneableObj< SJsonRpcNotification >    TJsonRpcNotificationEventData;

/**
 *  How a message is carried. The JSON-RPC layer does not care whether that is a web socket, HTTP, a pipe or
 *  something in memory: it hands complete JSON text messages to this interface to be sent. Incoming messages
 *  are given to the endpoint by the party which owns the transport, see CJsonRpcEndpoint::ProcessIncoming()
 */
class GUCEF_COM_PUBLIC_CPP CIJsonRpcTransport
{
    public:

    CIJsonRpcTransport( void ) {}

    virtual ~CIJsonRpcTransport() {}

    /** Sends one complete message, return false if it could not be sent */
    virtual bool SendMessageText( const CORE::CString& messageText ) = 0;
};

class CJsonRpcEndpoint;

/** What a request handler did with a request */
enum EJsonRpcRequestResult
{
    JSONRPC_REQUEST_HANDLED  ,      /**< the response is filled in and will be sent right away */
    JSONRPC_REQUEST_DEFERRED        /**< the answer comes later: the handler calls CJsonRpcEndpoint::SendResponse() */
};

/**
 *  Handles the calls made by the peer. To answer with an error fill in 'response' with isError set,
 *  to answer with a result set response.result. The id is taken care of.
 */
class GUCEF_COM_PUBLIC_CPP CIJsonRpcRequestHandler
{
    public:

    CIJsonRpcRequestHandler( void ) {}

    virtual ~CIJsonRpcRequestHandler() {}

    virtual EJsonRpcRequestResult OnJsonRpcRequest( CJsonRpcEndpoint& endpoint    ,
                                                    const SJsonRpcRequest& request ,
                                                    SJsonRpcResponse& response     ) = 0;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_JSONRPCTYPES_H ? */
