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

#ifndef GUCEF_WEB_CMCPHTTPTRANSPORTSETTINGS_H
#define GUCEF_WEB_CMCPHTTPTRANSPORTSETTINGS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CICONFIGURABLE_H
#include "gucefCORE_CIConfigurable.h"
#define GUCEF_CORE_CICONFIGURABLE_H
#endif /* GUCEF_CORE_CICONFIGURABLE_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_WEB_MACROS_H
#include "gucefWEB_macros.h"
#define GUCEF_WEB_MACROS_H
#endif /* GUCEF_WEB_MACROS_H ? */

#ifndef GUCEF_COMCORE_CTLSSERVERSETTINGS_H
#include "gucefCOMCORE_CTlsServerSettings.h"
#define GUCEF_COMCORE_CTLSSERVERSETTINGS_H
#endif /* GUCEF_COMCORE_CTLSSERVERSETTINGS_H ? */

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

/**
 *  The settings of CMcpStreamableHttpTransport, an entry of the Transports list of the MCP server settings with
 *  type "streamable-http". The keys of the authenticator (bearerTokenFile, tokenLabel, scopes for "static-bearer") are
 *  read from the same entry by the authenticator.
 */
class GUCEF_WEB_PUBLIC_CPP CMcpHttpTransportSettings : public CORE::CIConfigurable
{
    public:

    CORE::CString bindAddress;                      /**< dotted IPv4, default 127.0.0.1; another than a loopback address needs TLS or allowNonLoopback */
    UInt16 port;                                    /**< 0 (the default) picks a free port, see CMcpStreamableHttpTransport::GetBoundPort() */
    CORE::CString path;                             /**< the MCP endpoint, default "/mcp" */
    CORE::CString::StringVector allowedOrigins;     /**< origins ("http://localhost:3000") whose requests are allowed; empty: no request with an Origin */
    CORE::CString::StringVector allowedHosts;       /**< host names a request may name in Host; empty: localhost, 127.0.0.1 and [::1]. Always with the bound port */
    CORE::CString authenticator;                    /**< the name of the authenticator in the factory of CWebGlobal, default "static-bearer" */
    UInt32 maxBodyBytes;                            /**< 0 (the default): the maxMessageBytes of the MCP server */
    UInt32 maxConnections;                          /**< default 32 */
    UInt32 sseKeepAliveMs;                          /**< a comment on open streams this often; 0 (the default): the sseKeepAliveMs of the MCP server */
    UInt32 headerTimeoutMs;                         /**< default 10000, see CHttpServerSettings */
    UInt32 bodyTimeoutMs;                           /**< default 30000 */
    UInt32 idleTimeoutMs;                           /**< default 60000 */
    bool allowNonLoopback;                          /**< default false: true allows another than a loopback address without TLS, plain HTTP on the network */
    COMCORE::CTlsServerSettings tls;                /**< HTTPS when a certificate and key are given: certificateChainPath, privateKeyPath and the other keys of CTlsServerSettings in the same entry */

    CMcpHttpTransportSettings( void );

    CMcpHttpTransportSettings( const CMcpHttpTransportSettings& src );

    virtual ~CMcpHttpTransportSettings() GUCEF_VIRTUAL_OVERRIDE;

    CMcpHttpTransportSettings& operator=( const CMcpHttpTransportSettings& src );

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_WEB_CMCPHTTPTRANSPORTSETTINGS_H ? */
