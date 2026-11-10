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

#ifndef GUCEF_COM_CMCPRPCRELAY_H
#define GUCEF_COM_CMCPRPCRELAY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCENDPOINT_H
#include "gucefCOMCORE_CRpcEndpoint.h"
#define GUCEF_COMCORE_CRPCENDPOINT_H
#endif /* GUCEF_COMCORE_CRPCENDPOINT_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The client side of MCP over RPC (gucefDRN RPC transport design 10): one gucef.mcp.Session/Exchange stream over an
 *  RPC endpoint that wants gucef.mcp 1, carrying JSON-RPC messages as they are. An application connects it to
 *  standard input and output (the stdio framing: one message per line) to offer a remote MCP server to an LLM client
 *  that starts a local process.
 *
 *  Threading: SendMessage() from any thread; ReadMessage() blocks as CIRpcStream::ReadNext() does (on the protocol
 *  thread it pumps that thread).
 */
class GUCEF_COM_PUBLIC_CPP CMcpRpcRelay
{
    public:

    CMcpRpcRelay( void );
    ~CMcpRpcRelay();

    /** Opens the Exchange stream; the endpoint must have bound gucef.mcp 1 */
    bool Open( const COMCORE::CRpcEndpointPtr& endpoint, COMCORE::CRpcStatus& status );

    /** One JSON-RPC message (a line of stdio without its line end) to the server */
    bool SendMessage( const CORE::CString& json );

    /** The next message of the server; false when the stream ended (status) or the time ran out */
    bool ReadMessage( CORE::CString& json, COMCORE::CRpcStatus& status, UInt32 timeoutMs );

    /** Ends the stream: the server closes the channel */
    void Close( void );

    bool IsOpen( void ) const;

    /**
     *  Relays stdio framed text: every complete line of the input is one message; returns the bytes consumed. The
     *  rest of an incomplete line stays for the next call
     */
    UInt32 RelayInput( const char* data, UInt32 size );

    private:

    CMcpRpcRelay( const CMcpRpcRelay& src );                /**< not implemented */
    CMcpRpcRelay& operator=( const CMcpRpcRelay& src );     /**< not implemented */

    COMCORE::CRpcEndpointPtr m_endpoint;
    COMCORE::CRpcMethodRef m_ref;
    COMCORE::CIRpcStreamPtr m_stream;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPRPCRELAY_H ? */
