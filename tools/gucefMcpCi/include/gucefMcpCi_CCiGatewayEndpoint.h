/*
 *  gucefMcpCi: MCP server that hands the build matrix of a repository to the lab CI
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

#ifndef GUCEF_MCPCI_CCIGATEWAYENDPOINT_H
#define GUCEF_MCPCI_CCIGATEWAYENDPOINT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFMcpCi {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The gateway a machine in proxy mode sends its calls to, the optional top level object "gateway" of ci.json as
 *  lablink provides it:
 *
 *      "gateway": { "url": "https://host:8792", "caFile": "ca.crt" }
 *
 *  A relative caFile is relative to the directory of ci.json.
 */
class CCiGatewayEndpoint
{
    public:

    GUCEF::CORE::CString url;           /**< the root of the gateway API, without a trailing / */
    GUCEF::CORE::CString caFile;        /**< PEM file with the authorities that vouch for the gateway, empty for the system trust store */

    CCiGatewayEndpoint( void );

    /** Reads the "gateway" object. @return false with the reason when it is there but not usable */
    bool LoadConfig( const GUCEF::CORE::CDataNode& node, GUCEF::CORE::CString& problem );

    /** Makes a relative caFile relative to 'baseDir' */
    void ResolveRelativeTo( const GUCEF::CORE::CString& baseDir );

    bool IsSet( void ) const;

    /**
     *  Whether a gateway URL is acceptable: https, or http to a loopback address only (127.0.0.1, localhost, [::1]),
     *  since the token crosses the network with every request
     */
    static bool IsAcceptableUrl( const GUCEF::CORE::CString& url, GUCEF::CORE::CString& problem );
};

/*-------------------------------------------------------------------------*/

}; /* namespace GUCEFMcpCi */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MCPCI_CCIGATEWAYENDPOINT_H ? */
