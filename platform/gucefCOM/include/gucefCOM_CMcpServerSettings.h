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

#ifndef GUCEF_COM_CMCPSERVERSETTINGS_H
#define GUCEF_COM_CMCPSERVERSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

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
 *  The configuration of a CMcpServer, section "McpServer". In config: the members by the same name as attributes or
 *  child values, the AEAD codec types comma separated, the transports as the child "Transports".
 */
class GUCEF_COM_PUBLIC_CPP CMcpServerSettings : public CORE::CIConfigurable
{
    public:

    static const CORE::CString ConfigSectionName;       /**< "McpServer" */

    CORE::CString serverName;                   /**< _meta serverInfo name, "gucef-mcp-server" by default */
    CORE::CString serverTitle;
    CORE::CString serverVersion;                /**< "1.0.0" by default */
    CORE::CString instructions;                 /**< for the model, in server/discover and initialize; empty: none */
    bool includeServerInfo;                     /**< write _meta serverInfo on every modern result, default true */

    bool enableTools;                           /**< default true; a disabled registry answers -32601 */
    bool enableResources;
    bool enablePrompts;
    bool supportLegacy;                         /**< serve 2025-11-25 clients that open with initialize, default true */

    UInt32 discoverTtlMs;                       /**< default 3600000 */
    UInt32 listTtlMs;                           /**< default 60000 */
    UInt32 pageSize;                            /**< default 100 */

    UInt32 maxMessageBytes;                     /**< default 4194304 */
    UInt32 maxJsonDepth;                        /**< default 64 */
    UInt32 maxInFlightPerChannel;               /**< default 16 */
    UInt32 maxInFlightTotal;                    /**< default 64 */
    UInt32 toolCallsPerSecond;                  /**< per principal, default 10; 0: no limit */
    UInt32 toolCallBurst;                       /**< default 20 */

    UInt32 progressMinIntervalMs;               /**< default 100 */
    UInt32 maxRequestDurationMs;                /**< default 300000, a tool can set its own */
    UInt32 cancelGraceMs;                       /**< default 2000 */

    UInt32 requestStateTtlMs;                   /**< default 600000 */
    CORE::CString requestStateKeyFile;          /**< empty: a random key per process */
    CORE::CString requestStateProtection;       /**< "auto" (default): AEAD when an AEAD codec is registered, else "hmac-sha256"; or "aead" */
    CORE::CString::StringVector requestStateAeadCodecs;   /**< the AEAD codec types tried in order */

    UInt32 maxSubscriptionsPerChannel;          /**< default 8 */
    UInt32 maxSubscriptionsTotal;               /**< default 64 */
    UInt32 maxResourceSubscriptionsPerListen;   /**< default 256 */
    UInt32 sseKeepAliveMs;                      /**< default 15000, for the Streamable HTTP transport */

    bool validateStructuredOutput;              /**< default true */
    CORE::CString threadPoolName;               /**< default "gucef.mcp" */
    bool stopOnStdinClosed;                     /**< default true, for the stdio transport */

    CORE::CDataNode transports;                 /**< the "Transports" child: one { type, ... } entry per transport */

    CMcpServerSettings( void );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPSERVERSETTINGS_H ? */
