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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_WEB_CURLPATHTEMPLATE_H
#include "gucefWEB_CUrlPathTemplate.h"
#define GUCEF_WEB_CURLPATHTEMPLATE_H
#endif /* GUCEF_WEB_CURLPATHTEMPLATE_H ? */

#include "gucefLabLink_WebJson.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** "name": */
static CORE::CString
Key( const char* name )
{GUCEF_TRACE;

    return CORE::CString( "\"" ) + name + "\":";
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Text( const char* name, const CORE::CString& value )
{GUCEF_TRACE;

    return Key( name ) + CWebJson::Quote( value );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Number( const char* name, UInt64 value )
{GUCEF_TRACE;

    return Key( name ) + CORE::ToString( value );
}

/*-------------------------------------------------------------------------*/

static CORE::CString
Flag( const char* name, bool value )
{GUCEF_TRACE;

    return Key( name ) + ( value ? "true" : "false" );
}

/*-------------------------------------------------------------------------*/

/** The rank of a status in the list: online first */
static int
RankOf( const CORE::CString& status )
{GUCEF_TRACE;

    if ( status == HubNodeOnline )
        return 0;
    if ( status == HubNodeUnknown )
        return 1;
    return 2;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CORE::CString
CWebJson::Quote( const CORE::CString& text )
{GUCEF_TRACE;

    return "\"" + WEB::CUrlPathTemplate::EscapeJson( text ) + "\"";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebJson::Error( const CORE::CString& text )
{GUCEF_TRACE;

    return "{" + Text( "error", text ) + "}";
}

/*-------------------------------------------------------------------------*/

bool
CWebJson::ListsBefore( const SHubNode& a, const SHubNode& b )
{GUCEF_TRACE;

    const int rankA = RankOf( a.status );
    const int rankB = RankOf( b.status );
    if ( rankA != rankB )
        return rankA < rankB;
    const CORE::CString nameA = a.report.name.Lowercase();
    const CORE::CString nameB = b.report.name.Lowercase();
    if ( nameA != nameB )
        return nameA < nameB;
    return a.report.machineId < b.report.machineId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebJson::Node( const SHubNode& node )
{GUCEF_TRACE;

    const SNodeReport& r = node.report;
    CORE::CString json = "{" + Text( "machineId", r.machineId ) + "," + Text( "name", r.name ) + "," + Text( "os", r.os ) + "," +
                         Text( "arch", r.arch ) + "," + Text( "agentVersion", r.agentVersion ) + "," + Text( "status", node.status ) + "," +
                         Flag( "verified", node.verified ) + "," + Text( "address", node.address ) + "," +
                         Number( "firstSeen", node.firstSeenInUnixSecs ) + "," + Number( "lastSeen", node.lastSeenInUnixSecs ) + "," +
                         Number( "uptime", r.uptimeInSecs ) + "," + Flag( "ephemeral", r.ephemeral ) + "," + Text( "nodeState", r.nodeState ) + "," +
                         Text( "presence", r.presence ) + "," + Number( "idleSecs", r.idleInSecs ) + "," +
                         Number( "duplicateInstanceRefusals", node.duplicateInstanceRefusals ) + ",";
    json += Key( "resources" ) + "{" + Number( "cpuCores", r.cpuCores ) + "," + Number( "cpuLoadPercent", r.cpuLoadPercent ) + "," +
            Number( "memTotalMiB", r.memTotalInMiB ) + "," + Number( "memAvailableMiB", r.memAvailableInMiB ) + "," +
            Number( "diskFreeMiB", r.diskFreeInMiB ) + "},";

    json += Key( "gpus" ) + "[";
    for ( size_t i=0; i<r.gpus.size(); ++i )
    {
        const SGpuInfo& g = r.gpus[ i ];
        json += ( i > 0 ? ",{" : "{" ) + Text( "name", g.name ) + "," + Text( "vendor", g.vendor ) + "," + Number( "vramTotalMiB", g.vramTotalInMiB ) + "," +
                Number( "vramFreeMiB", g.vramFreeInMiB ) + "," + Number( "utilizationPercent", g.utilizationPercent ) + "}";
    }
    json += "]," + Key( "hosts" ) + "[";
    for ( size_t i=0; i<r.hosts.size(); ++i )
    {
        const SHostInfo& h = r.hosts[ i ];
        json += ( i > 0 ? ",{" : "{" ) + Text( "name", h.name ) + "," + Text( "state", h.state ) + "," + Text( "detail", h.detail ) + "}";
    }
    json += "]," + Key( "workloads" ) + "[";
    for ( size_t i=0; i<r.workloads.size(); ++i )
    {
        const SWorkloadInfo& w = r.workloads[ i ];
        json += ( i > 0 ? ",{" : "{" ) + Text( "id", w.id ) + "," + Text( "kind", w.kind ) + "," + Text( "state", w.state ) + "," +
                Text( "endpoint", w.endpoint ) + "," + Text( "model", w.model ) + "," + Number( "idleSlots", w.idleSlots ) + "}";
    }
    json += "]," + Key( "capabilities" ) + "[";
    for ( size_t i=0; i<r.capabilities.size(); ++i )
        json += ( i > 0 ? "," : "" ) + Quote( r.capabilities[ i ] );
    json += "]}";
    return json;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebJson::Nodes( const CORE::CString& hubName, UInt64 hubTime, GUCEF::vector< SHubNode >& nodes )
{GUCEF_TRACE;

    std::sort( nodes.begin(), nodes.end(), &CWebJson::ListsBefore );
    CORE::CString json = "{" + Text( "hub", hubName ) + "," + Number( "hubTime", hubTime ) + "," + Key( "nodes" ) + "[";
    for ( size_t i=0; i<nodes.size(); ++i )
    {
        if ( i > 0 )
            json += ",";
        json += Node( nodes[ i ] );
    }
    json += "]}";
    return json;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebJson::Endpoints( const CORE::CString& hubName, UInt64 hubTime, const GUCEF::vector< SHubEndpoint >& endpoints )
{GUCEF_TRACE;

    CORE::CString json = "{" + Text( "hub", hubName ) + "," + Number( "hubTime", hubTime ) + "," + Key( "endpoints" ) + "[";
    for ( size_t i=0; i<endpoints.size(); ++i )
    {
        const SHubEndpoint& e = endpoints[ i ];
        json += ( i > 0 ? ",{" : "{" ) + Text( "kind", e.kind ) + "," + Text( "machineId", e.machineId ) + "," + Text( "node", e.node ) + "," +
                Text( "url", e.url ) + "," + Text( "model", e.model ) + "," + Text( "workloadId", e.workloadId ) + "," +
                Number( "idleSlots", e.idleSlots ) + "}";
    }
    json += "]}";
    return json;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebJson::Users( const GUCEF::vector< SWebUser >& users )
{GUCEF_TRACE;

    CORE::CString json = "{" + Key( "users" ) + "[";
    for ( size_t i=0; i<users.size(); ++i )
    {
        const SWebUser& u = users[ i ];
        json += ( i > 0 ? ",{" : "{" ) + Text( "name", u.name ) + "," + Text( "role", u.role ) + "," + Flag( "disabled", u.disabled ) + "," +
                Number( "created", u.createdInUnixSecs ) + "}";
    }
    json += "]}";
    return json;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
