/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#ifndef GUCEF_LABLINK_WEBJSON_H
#define GUCEF_LABLINK_WEBJSON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefLabLink_HubView.h"
#include "gucefLabLink_WebUsers.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The JSON of the API of the website (version 1). Times are unix seconds on the clock of the hub, sizes MiB:
 *    nodes:     { "hub", "hubTime", "nodes": [ node, ... ] }, online first, then unknown, then offline, by name
 *    node:      { "machineId", "name", "os", "arch", "agentVersion", "status" (online|offline|unknown), "verified",
 *                 "address", "firstSeen", "lastSeen", "uptime", "ephemeral", "nodeState", "presence", "idleSecs",
 *                 "duplicateInstanceRefusals", "resources": { "cpuCores", "cpuLoadPercent", "memTotalMiB",
 *                 "memAvailableMiB", "diskFreeMiB" }, "gpus": [ { "name", "vendor", "vramTotalMiB", "vramFreeMiB",
 *                 "utilizationPercent" } ], "hosts": [ { "name", "state", "detail" } ], "workloads": [ { "id",
 *                 "kind", "state", "endpoint", "model", "idleSlots" } ], "capabilities": [ text, ... ] }
 *    endpoints: { "hub", "hubTime", "endpoints": [ { "kind", "machineId", "node", "url", "model", "workloadId", "idleSlots" } ] }
 *    users:     { "users": [ { "name", "role", "disabled", "created" } ] }, never a password record
 */
class CWebJson
{
    public:

    /** A JSON string, quotes included */
    static GUCEF::CORE::CString Quote( const GUCEF::CORE::CString& text );

    static GUCEF::CORE::CString Node( const SHubNode& node );

    /** Sorts the nodes as the list shows them */
    static GUCEF::CORE::CString Nodes( const GUCEF::CORE::CString& hubName, GUCEF::UInt64 hubTime, GUCEF::vector< SHubNode >& nodes );

    static GUCEF::CORE::CString Endpoints( const GUCEF::CORE::CString& hubName, GUCEF::UInt64 hubTime, const GUCEF::vector< SHubEndpoint >& endpoints );

    static GUCEF::CORE::CString Users( const GUCEF::vector< SWebUser >& users );

    /** { "error": text } */
    static GUCEF::CORE::CString Error( const GUCEF::CORE::CString& text );

    /** Whether node a comes before node b in the list: online, unknown, offline, then by name */
    static bool ListsBefore( const SHubNode& a, const SHubNode& b );

    private:

    CWebJson( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_WEBJSON_H ? */
