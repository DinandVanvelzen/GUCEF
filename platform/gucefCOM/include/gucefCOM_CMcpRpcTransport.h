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

#ifndef GUCEF_COM_CMCPRPCTRANSPORT_H
#define GUCEF_COM_CMCPRPCTRANSPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CRPCSERVICESET_H
#include "gucefCOMCORE_CRpcServiceSet.h"
#define GUCEF_COMCORE_CRPCSERVICESET_H
#endif /* GUCEF_COMCORE_CRPCSERVICESET_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACT_H
#include "gucefCOMCORE_CRpcContract.h"
#define GUCEF_COMCORE_CRPCCONTRACT_H
#endif /* GUCEF_COMCORE_CRPCCONTRACT_H ? */

#ifndef GUCEF_COM_CIMCPTRANSPORT_H
#include "gucefCOM_CIMcpTransport.h"
#define GUCEF_COM_CIMCPTRANSPORT_H
#endif /* GUCEF_COM_CIMCPTRANSPORT_H ? */

#ifndef GUCEF_COM_CMCPCHANNEL_H
#include "gucefCOM_CMcpChannel.h"
#define GUCEF_COM_CMCPCHANNEL_H
#endif /* GUCEF_COM_CMCPCHANNEL_H ? */

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
 *  MCP over RPC (gucefDRN RPC transport design 10): the MCP transport "rpc". Each call of gucef.mcp.Session/Exchange,
 *  a bidi stream of McpMessage { 1 string json }, is one MCP channel of kind stream: a message of the caller is one
 *  JSON-RPC message for the server (size and nesting checks apply), a message the server sends is one stream message.
 *  HALF_CLOSE or CANCEL of the call closes the channel; closing the channel ends the call with OK. Stream credits keep
 *  a slow client to its own channel.
 *
 *  Start() registers the Session service on the RPC service set the application gives (SetServiceSet(), or the one of
 *  a CRpcHost), with RPC_EXECUTION_NOTIFICATION_THREAD: the notification pulse generator of the RPC endpoints must be
 *  the MCP server's, so that the channel sees its messages on the protocol thread of the server.
 *
 *  The principal of a channel is CMcpPrincipal( "rpc", <subject of the RPC principal>, <its isAuthenticated> ) with
 *  the roles of the RPC principal as scopes. The MCP access policy and audit see the RPC peer.
 *
 *  Messages the server sends while the client has no credit wait per channel, up to maxPendingMessages (1024) and
 *  maxPendingBytes (16 MB), 0 for no maximum. A client beyond either does not read what it asked for: its call ends
 *  with RESOURCE_EXHAUSTED and its channel closes at the next pulse of the server's pulse generator; other channels
 *  are not affected.
 */
class GUCEF_COM_PUBLIC_CPP CMcpRpcTransport : public CIMcpTransport
{
    public:

    static const CORE::CString RpcTransportName;        /**< "rpc" */
    static const UInt32 DefaultMaxPendingMessages = 1024;
    static const UInt32 DefaultMaxPendingBytes = 16 * 1024 * 1024;

    /** gucef.mcp 1.0, built in code */
    static COMCORE::CRpcContractPtr GetMcpContract( void );

    CMcpRpcTransport( void );
    virtual ~CMcpRpcTransport() GUCEF_VIRTUAL_OVERRIDE;

    /** Before Start(): where the Session service is registered */
    void SetServiceSet( const COMCORE::CRpcServiceSetPtr& services );

    UInt32 GetChannelCount( void ) const;

    /** Bounds of the messages that wait for the credit of one channel's client; 0 for no maximum (config keys
     *  "maxPendingMessages", "maxPendingBytes") */
    void SetMaxPendingMessages( UInt32 maxMessages );
    UInt32 GetMaxPendingMessages( void ) const;
    void SetMaxPendingBytes( UInt32 maxBytes );
    UInt32 GetMaxPendingBytes( void ) const;

    virtual const CORE::CString& GetTransportName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool Start( CMcpServer& server ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void Stop( bool graceful ) GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetEndpointDescription( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    class CSessionService;
    class CRpcChannel;
    friend class CSessionService;
    friend class CRpcChannel;

    CMcpRpcTransport( const CMcpRpcTransport& src );                /**< not implemented */
    CMcpRpcTransport& operator=( const CMcpRpcTransport& src );     /**< not implemented */

    void OnChannelEnded( CRpcChannel& channel );
    void OnChannelOverflow( CRpcChannel& channel );

    CMcpServer* m_server;
    COMCORE::CRpcServiceSetPtr m_services;
    COMCORE::CRpcContractPtr m_contract;
    CSessionService* m_service;
    bool m_active;
    UInt32 m_maxPendingMessages;
    UInt32 m_maxPendingBytes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPRPCTRANSPORT_H ? */
