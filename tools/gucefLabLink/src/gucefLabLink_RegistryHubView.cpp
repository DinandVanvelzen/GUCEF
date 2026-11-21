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

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_RegistryHubView.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRegistryHubView::CRegistryHubView( const CNodeRegistry& registry, const CORE::CString& hubName )
    : ILabHubView()
    , m_registry( registry )
    , m_hubName( hubName )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRegistryHubView::~CRegistryHubView()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CRegistryHubView::GetHubName( void ) const
{GUCEF_TRACE;

    return m_hubName;
}

/*-------------------------------------------------------------------------*/

UInt64
CRegistryHubView::GetHubTimeInUnixSecs( void ) const
{GUCEF_TRACE;

    return NowInUnixSecs();
}

/*-------------------------------------------------------------------------*/

void
CRegistryHubView::ToHubNode( const SNodeEntry& entry, SHubNode& node )
{GUCEF_TRACE;

    node.report = entry.report;
    node.verified = entry.verified;
    node.address = entry.address;
    switch ( entry.liveness )
    {
        case NODELIVENESS_ONLINE: node.status = HubNodeOnline; break;
        case NODELIVENESS_OFFLINE: node.status = HubNodeOffline; break;
        default: node.status = HubNodeUnknown; break;
    }
    node.firstSeenInUnixSecs = entry.firstSeenInUnixSecs;
    node.lastSeenInUnixSecs = entry.lastSeenInUnixSecs;
    node.duplicateInstanceRefusals = entry.duplicateInstanceRefusals;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryHubView::GetNodes( GUCEF::vector< SHubNode >& nodes ) const
{GUCEF_TRACE;

    GUCEF::vector< SNodeEntry > entries;
    m_registry.GetAll( entries );
    nodes.clear();
    nodes.resize( entries.size() );
    for ( size_t i=0; i<entries.size(); ++i )
        ToHubNode( entries[ i ], nodes[ i ] );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryHubView::GetNode( const CORE::CString& machineId, SHubNode& node, bool& found ) const
{GUCEF_TRACE;

    SNodeEntry entry;
    found = m_registry.Find( machineId, entry );
    if ( found )
        ToHubNode( entry, node );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRegistryHubView::GetEndpoints( GUCEF::vector< SHubEndpoint >& endpoints ) const
{GUCEF_TRACE;

    // what the workloads of the nodes that are heard from publish; the hub assigns none yet
    endpoints.clear();
    GUCEF::vector< SNodeEntry > entries;
    m_registry.GetAll( entries );
    for ( size_t i=0; i<entries.size(); ++i )
    {
        const SNodeEntry& entry = entries[ i ];
        if ( NODELIVENESS_ONLINE != entry.liveness )
            continue;
        for ( size_t w=0; w<entry.report.workloads.size(); ++w )
        {
            const SWorkloadInfo& workload = entry.report.workloads[ w ];
            if ( workload.endpoint.IsNULLOrEmpty() || workload.state != "running" )
                continue;
            SHubEndpoint endpoint;
            endpoint.kind = workload.kind;
            endpoint.machineId = entry.report.machineId;
            endpoint.node = entry.report.name;
            endpoint.url = workload.endpoint;
            endpoint.model = workload.model;
            endpoint.workloadId = workload.id;
            endpoint.idleSlots = workload.idleSlots;
            endpoints.push_back( endpoint );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
