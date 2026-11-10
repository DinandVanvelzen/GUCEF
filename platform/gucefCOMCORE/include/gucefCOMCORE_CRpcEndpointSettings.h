/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H
#define GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#ifndef GUCEF_COMCORE_RPCPROTOCOL_H
#include "gucefCOMCORE_RpcProtocol.h"
#define GUCEF_COMCORE_RPCPROTOCOL_H
#endif /* GUCEF_COMCORE_RPCPROTOCOL_H ? */

#ifndef GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H
#include "gucefCOMCORE_CRpcTransportProfile.h"
#define GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H
#endif /* GUCEF_COMCORE_CRPCTRANSPORTPROFILE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

class CRpcContractRegistry;

/*-------------------------------------------------------------------------*/

/**
 *  Settings of an RPC endpoint (gucefCOMCORE RPC design 10.3; limits 14.4; reliability 7; resume 8). Saved and loaded
 *  as configuration; contractRegistry and threadPool are given by code, not configuration. Every limit accepts 0 for
 *  "no maximum" where a maximum is not a safety requirement.
 */
class GUCEF_COMCORE_EXPORT_CPP CRpcEndpointSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString nodeName;                     /**< informational, sent in HELLO */
    SRpcLimits limits;                          /**< what this side announces */
    UInt32 retentionMaxFrames;                  /**< 8192 */
    UInt32 retentionMaxBytes;                   /**< 16 MB */
    UInt32 sendQueueMaxBytes;                   /**< 16 MB */
    TRpcSuspendedOverflow suspendedOverflow;    /**< REFUSE_NEW */
    bool requireSecureTransport;                /**< refuse transports that are not isSecure */
    bool enableReflection;                      /**< serve gucef.rpc.Reflection (default true) */
    bool preferPeerContracts;                   /**< encode with the peer's exact contract version (generic tools) */
    UInt32 clockSkewAllowanceMs;                /**< provider-side deadline check, 2000 */
    UInt32 closeTimeoutMs;                      /**< graceful Close() waits this long for open calls */
    UInt32 durableResultRetentionMs;            /**< DURABLE results kept this long, 24 h */
    UInt32 maxDurableRequestsPerSubject;        /**< 1024 */
    UInt32 maxGetContractPerSession;            /**< 16 */
    UInt32 maxOpenAssemblies;                   /**< fragmented messages being joined, 16 */
    UInt32 reconnectInitialDelayMs;             /**< 500 */
    UInt32 reconnectMaxDelayMs;                 /**< 30000 */
    UInt32 reconnectFactor;                     /**< 2 */
    bool autoReconnect;                         /**< reconnect by itself when it has a remote URI (Connect()) */
    UInt32 tickIntervalMs;                      /**< timers: acks, retransmissions, deadlines, keepalive; 10 */
    bool useTransportProfile;                   /**< take the timing of the transport's profile */
    CRpcTransportProfile profile;               /**< used when useTransportProfile is false, or to override it */
    CORE::CString threadPoolName;               /**< RPC_EXECUTION_THREAD_POOL handlers, "gucef.rpc" */
    CORE::ThreadPoolPtr threadPool;             /**< given by code; takes precedence over threadPoolName */
    CRpcContractRegistry* contractRegistry;     /**< given by code; NULL = CComCoreGlobal's */

    CRpcEndpointSettings( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CRPCENDPOINTSETTINGS_H ? */
