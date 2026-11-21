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

#include "gucefLabLink_NodeReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* NodeStateAvailable = "available";
const char* NodeStateThrottled = "throttled";
const char* NodeStateBusy = "busy";
const char* NodeStatePaused = "paused";
const char* NodeStateDraining = "draining";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The text with control characters as spaces, cut to the limit */
static CORE::CString
Clean( const CORE::CString& text )
{GUCEF_TRACE;

    char buffer[ MaxReportTextLength + 1 ];
    UInt32 length = text.Length();
    if ( length > MaxReportTextLength )
    {
        // not in the middle of a UTF-8 character
        length = MaxReportTextLength;
        while ( length > 0 && 0x80 == ( (unsigned char) text[ length ] & 0xC0 ) )
            --length;
    }
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = text[ i ];
        buffer[ i ] = ( (unsigned char) c < 0x20 || 0x7f == (unsigned char) c ) ? ' ' : c;
    }
    buffer[ length ] = '\0';
    return CORE::CString( buffer, length );
}

/*-------------------------------------------------------------------------*/

bool
NodeReportToMessage( const SNodeReport& report, COMCORE::CRpcMessage& message )
{GUCEF_TRACE;

    bool ok = true;
    ok = message.SetString( 1, report.machineId ) && ok;
    ok = message.SetString( 2, report.instanceId ) && ok;
    ok = message.SetString( 3, report.bootId ) && ok;
    ok = message.SetUInt64( 22, report.startedInUnixSecs ) && ok;
    ok = message.SetString( 4, report.name ) && ok;
    ok = message.SetString( 5, report.os ) && ok;
    ok = message.SetString( 6, report.arch ) && ok;
    ok = message.SetString( 7, report.agentVersion ) && ok;
    ok = message.SetUInt64( 8, report.uptimeInSecs ) && ok;
    ok = message.SetBool( 9, report.ephemeral ) && ok;
    ok = message.SetString( 10, report.nodeState ) && ok;
    ok = message.SetString( 11, report.presence ) && ok;
    ok = message.SetUInt32( 12, report.idleInSecs ) && ok;
    ok = message.SetUInt32( 13, report.cpuCores ) && ok;
    ok = message.SetUInt32( 14, report.cpuLoadPercent ) && ok;
    ok = message.SetUInt64( 15, report.memTotalInMiB ) && ok;
    ok = message.SetUInt64( 16, report.memAvailableInMiB ) && ok;
    ok = message.SetUInt64( 17, report.diskFreeInMiB ) && ok;

    for ( size_t i=0; i<report.gpus.size() && i<MaxReportGpus; ++i )
    {
        COMCORE::CRpcMessage* gpu = message.AddMessage( 18 );
        if ( GUCEF_NULL == gpu )
        {
            ok = false;
            break;
        }
        gpu->SetString( 1, report.gpus[ i ].name );
        gpu->SetString( 2, report.gpus[ i ].vendor );
        gpu->SetUInt32( 3, report.gpus[ i ].vramTotalInMiB );
        gpu->SetUInt32( 4, report.gpus[ i ].vramFreeInMiB );
        gpu->SetUInt32( 5, report.gpus[ i ].utilizationPercent );
    }
    for ( size_t i=0; i<report.hosts.size() && i<MaxReportHosts; ++i )
    {
        COMCORE::CRpcMessage* host = message.AddMessage( 19 );
        if ( GUCEF_NULL == host )
        {
            ok = false;
            break;
        }
        host->SetString( 1, report.hosts[ i ].name );
        host->SetString( 2, report.hosts[ i ].state );
        host->SetString( 3, report.hosts[ i ].detail );
    }
    for ( size_t i=0; i<report.workloads.size() && i<MaxReportWorkloads; ++i )
    {
        COMCORE::CRpcMessage* workload = message.AddMessage( 20 );
        if ( GUCEF_NULL == workload )
        {
            ok = false;
            break;
        }
        workload->SetString( 1, report.workloads[ i ].id );
        workload->SetString( 2, report.workloads[ i ].kind );
        workload->SetString( 3, report.workloads[ i ].state );
        workload->SetString( 4, report.workloads[ i ].endpoint );
        workload->SetString( 5, report.workloads[ i ].model );
        workload->SetUInt32( 6, report.workloads[ i ].idleSlots );
    }
    for ( size_t i=0; i<report.capabilities.size() && i<MaxReportCapabilities; ++i )
        ok = message.Add( 21, CORE::CVariant( report.capabilities[ i ] ) ) && ok;
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
NodeReportFromMessage( const COMCORE::CRpcMessage& message, SNodeReport& report )
{GUCEF_TRACE;

    report = SNodeReport();
    report.machineId = Clean( message.GetString( 1 ) );
    if ( report.machineId.IsNULLOrEmpty() )
        return false;
    report.instanceId = Clean( message.GetString( 2 ) );
    report.bootId = Clean( message.GetString( 3 ) );
    report.startedInUnixSecs = message.GetUInt64( 22 );
    report.name = Clean( message.GetString( 4 ) );
    report.os = Clean( message.GetString( 5 ) );
    report.arch = Clean( message.GetString( 6 ) );
    report.agentVersion = Clean( message.GetString( 7 ) );
    report.uptimeInSecs = message.GetUInt64( 8 );
    report.ephemeral = message.GetBool( 9 );
    report.nodeState = Clean( message.GetString( 10 ) );
    if ( report.nodeState.IsNULLOrEmpty() )
        report.nodeState = NodeStateAvailable;
    report.presence = Clean( message.GetString( 11 ) );
    report.idleInSecs = message.GetUInt32( 12 );
    report.cpuCores = message.GetUInt32( 13 );
    report.cpuLoadPercent = message.GetUInt32( 14 );
    if ( report.cpuLoadPercent > 100 )
        report.cpuLoadPercent = 100;
    report.memTotalInMiB = message.GetUInt64( 15 );
    report.memAvailableInMiB = message.GetUInt64( 16 );
    report.diskFreeInMiB = message.GetUInt64( 17 );

    const UInt32 gpuCount = message.GetCount( 18 );
    for ( UInt32 i=0; i<gpuCount && i<MaxReportGpus; ++i )
    {
        const COMCORE::CRpcMessage* gpu = message.GetMessageAt( 18, i );
        if ( GUCEF_NULL == gpu )
            continue;
        SGpuInfo info;
        info.name = Clean( gpu->GetString( 1 ) );
        info.vendor = Clean( gpu->GetString( 2 ) );
        info.vramTotalInMiB = gpu->GetUInt32( 3 );
        info.vramFreeInMiB = gpu->GetUInt32( 4 );
        info.utilizationPercent = gpu->GetUInt32( 5 );
        if ( info.utilizationPercent > 100 )
            info.utilizationPercent = 100;
        report.gpus.push_back( info );
    }
    const UInt32 hostCount = message.GetCount( 19 );
    for ( UInt32 i=0; i<hostCount && i<MaxReportHosts; ++i )
    {
        const COMCORE::CRpcMessage* host = message.GetMessageAt( 19, i );
        if ( GUCEF_NULL == host )
            continue;
        SHostInfo info;
        info.name = Clean( host->GetString( 1 ) );
        info.state = Clean( host->GetString( 2 ) );
        info.detail = Clean( host->GetString( 3 ) );
        report.hosts.push_back( info );
    }
    const UInt32 workloadCount = message.GetCount( 20 );
    for ( UInt32 i=0; i<workloadCount && i<MaxReportWorkloads; ++i )
    {
        const COMCORE::CRpcMessage* workload = message.GetMessageAt( 20, i );
        if ( GUCEF_NULL == workload )
            continue;
        SWorkloadInfo info;
        info.id = Clean( workload->GetString( 1 ) );
        info.kind = Clean( workload->GetString( 2 ) );
        info.state = Clean( workload->GetString( 3 ) );
        info.endpoint = Clean( workload->GetString( 4 ) );
        info.model = Clean( workload->GetString( 5 ) );
        info.idleSlots = workload->GetUInt32( 6 );
        report.workloads.push_back( info );
    }
    const UInt32 capabilityCount = message.GetCount( 21 );
    for ( UInt32 i=0; i<capabilityCount && i<MaxReportCapabilities; ++i )
    {
        CORE::CVariant capability;
        if ( message.GetAt( 21, i, capability ) )
            report.capabilities.push_back( Clean( capability.AsString() ) );
    }
    return true;
}

CORE::CString
EncodeReportDetails( const SNodeReport& report )
{GUCEF_TRACE;

    CORE::CString text;
    for ( size_t i=0; i<report.gpus.size() && i<MaxReportGpus; ++i )
    {
        const SGpuInfo& gpu = report.gpus[ i ];
        text += "g\t" + Clean( gpu.name ) + "\t" + Clean( gpu.vendor ) + "\t" + CORE::ToString( gpu.vramTotalInMiB ) + "\t" +
                CORE::ToString( gpu.vramFreeInMiB ) + "\t" + CORE::ToString( gpu.utilizationPercent ) + "\n";
    }
    for ( size_t i=0; i<report.hosts.size() && i<MaxReportHosts; ++i )
        text += "h\t" + Clean( report.hosts[ i ].name ) + "\t" + Clean( report.hosts[ i ].state ) + "\t" + Clean( report.hosts[ i ].detail ) + "\n";
    for ( size_t i=0; i<report.workloads.size() && i<MaxReportWorkloads; ++i )
    {
        const SWorkloadInfo& workload = report.workloads[ i ];
        text += "w\t" + Clean( workload.id ) + "\t" + Clean( workload.kind ) + "\t" + Clean( workload.state ) + "\t" + Clean( workload.endpoint ) + "\t" +
                Clean( workload.model ) + "\t" + CORE::ToString( workload.idleSlots ) + "\n";
    }
    for ( size_t i=0; i<report.capabilities.size() && i<MaxReportCapabilities; ++i )
        text += "c\t" + Clean( report.capabilities[ i ] ) + "\n";
    return text;
}

/*-------------------------------------------------------------------------*/

void
DecodeReportDetails( const CORE::CString& details, SNodeReport& report )
{GUCEF_TRACE;

    const CORE::CString::StringVector lines = details.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        const CORE::CString::StringVector fields = lines[ i ].ParseElements( '\t', true );
        if ( fields.empty() )
            continue;
        if ( fields[ 0 ] == "g" && fields.size() >= 6 && report.gpus.size() < MaxReportGpus )
        {
            SGpuInfo gpu;
            gpu.name = fields[ 1 ];
            gpu.vendor = fields[ 2 ];
            gpu.vramTotalInMiB = CORE::StringToUInt32( fields[ 3 ], 0 );
            gpu.vramFreeInMiB = CORE::StringToUInt32( fields[ 4 ], 0 );
            gpu.utilizationPercent = CORE::StringToUInt32( fields[ 5 ], 0 );
            report.gpus.push_back( gpu );
        }
        else
        if ( fields[ 0 ] == "h" && fields.size() >= 4 && report.hosts.size() < MaxReportHosts )
        {
            SHostInfo host;
            host.name = fields[ 1 ];
            host.state = fields[ 2 ];
            host.detail = fields[ 3 ];
            report.hosts.push_back( host );
        }
        else
        if ( fields[ 0 ] == "w" && fields.size() >= 7 && report.workloads.size() < MaxReportWorkloads )
        {
            SWorkloadInfo workload;
            workload.id = fields[ 1 ];
            workload.kind = fields[ 2 ];
            workload.state = fields[ 3 ];
            workload.endpoint = fields[ 4 ];
            workload.model = fields[ 5 ];
            workload.idleSlots = CORE::StringToUInt32( fields[ 6 ], 0 );
            report.workloads.push_back( workload );
        }
        else
        if ( fields[ 0 ] == "c" && fields.size() >= 2 && report.capabilities.size() < MaxReportCapabilities )
            report.capabilities.push_back( fields[ 1 ] );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
