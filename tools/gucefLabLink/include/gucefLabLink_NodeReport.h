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

#ifndef GUCEF_LABLINK_NODEREPORT_H
#define GUCEF_LABLINK_NODEREPORT_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_COMCORE_CRPCMESSAGE_H
#include "gucefCOMCORE_CRpcMessage.h"
#define GUCEF_COMCORE_CRPCMESSAGE_H
#endif /* GUCEF_COMCORE_CRPCMESSAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The node states a report can carry; the presence policy sets them, a node without one reports "available" */
extern const char* NodeStateAvailable;
extern const char* NodeStateThrottled;
extern const char* NodeStateBusy;
extern const char* NodeStatePaused;
extern const char* NodeStateDraining;

/** Limits applied to what a node reports: more is cut off, so a report has a bounded size */
static const GUCEF::UInt32 MaxReportTextLength = 128;
static const GUCEF::UInt32 MaxReportGpus = 8;
static const GUCEF::UInt32 MaxReportHosts = 8;
static const GUCEF::UInt32 MaxReportWorkloads = 32;
static const GUCEF::UInt32 MaxReportCapabilities = 16;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A graphics card of a node */
struct SGpuInfo
{
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString vendor;               /**< "nvidia", "amd", "intel" or empty */
    GUCEF::UInt32 vramTotalInMiB;
    GUCEF::UInt32 vramFreeInMiB;
    GUCEF::UInt32 utilizationPercent;

    SGpuInfo( void ) : name(), vendor(), vramTotalInMiB( 0 ), vramFreeInMiB( 0 ), utilizationPercent( 0 ) {}
};

/** A place a node can run workloads in: "native", "wsl2", "k3s-agent" */
struct SHostInfo
{
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString state;                /**< "ready", "stopped", "needs-reboot", "absent", ... */
    GUCEF::CORE::CString detail;

    SHostInfo( void ) : name(), state(), detail() {}
};

/** A workload a node runs */
struct SWorkloadInfo
{
    GUCEF::CORE::CString id;
    GUCEF::CORE::CString kind;                 /**< "llm", "image", "tts", "stt", "mcp", "generic" */
    GUCEF::CORE::CString state;                /**< "starting", "running", "stopped", "failed" */
    GUCEF::CORE::CString endpoint;
    GUCEF::CORE::CString model;
    GUCEF::UInt32 idleSlots;

    SWorkloadInfo( void ) : id(), kind(), state(), endpoint(), model(), idleSlots( 0 ) {}
};

/** What a node tells the hub about itself every report interval */
struct SNodeReport
{
    GUCEF::CORE::CString machineId;
    GUCEF::CORE::CString instanceId;           /**< per run of the agent: random at every start */
    GUCEF::UInt64 startedInUnixSecs;           /**< when this run of the agent started; the newer run of a machine id wins, an older one is a second instance */
    GUCEF::CORE::CString bootId;               /**< per boot of the machine */
    GUCEF::CORE::CString name;
    GUCEF::CORE::CString os;
    GUCEF::CORE::CString arch;
    GUCEF::CORE::CString agentVersion;
    GUCEF::UInt64 uptimeInSecs;
    bool ephemeral;

    GUCEF::CORE::CString nodeState;            /**< NodeState... */
    GUCEF::CORE::CString presence;             /**< "none", "locked", "idle", "active", "fullscreen", empty when not known */
    GUCEF::UInt32 idleInSecs;

    GUCEF::UInt32 cpuCores;
    GUCEF::UInt32 cpuLoadPercent;
    GUCEF::UInt64 memTotalInMiB;
    GUCEF::UInt64 memAvailableInMiB;
    GUCEF::UInt64 diskFreeInMiB;

    GUCEF::vector< SGpuInfo > gpus;
    GUCEF::vector< SHostInfo > hosts;
    GUCEF::vector< SWorkloadInfo > workloads;
    GUCEF::CORE::CString::StringVector capabilities;    /**< what this node can run: "llm", "image", ... */

    SNodeReport( void )
        : machineId(), instanceId(), startedInUnixSecs( 0 ), bootId(), name(), os(), arch(), agentVersion(), uptimeInSecs( 0 ), ephemeral( false )
        , nodeState( NodeStateAvailable ), presence(), idleInSecs( 0 ), cpuCores( 0 ), cpuLoadPercent( 0 )
        , memTotalInMiB( 0 ), memAvailableInMiB( 0 ), diskFreeInMiB( 0 ), gpus(), hosts(), workloads(), capabilities()
    {}
};

/** Fills the message of the Report request from the report; false when the message does not take it */
bool NodeReportToMessage( const SNodeReport& report, GUCEF::COMCORE::CRpcMessage& message );

/**
 *  Reads a Report request. Text is cut to MaxReportTextLength and lists to their limits, control characters in text
 *  become spaces. @return false when the message has no machine id
 */
bool NodeReportFromMessage( const GUCEF::COMCORE::CRpcMessage& message, SNodeReport& report );

/**
 *  The lists of a report (graphics cards, hosts, workloads, capabilities) as text for one column of storage: one line
 *  per item, tab separated, the kind of item first
 */
GUCEF::CORE::CString EncodeReportDetails( const SNodeReport& report );

/** The inverse of EncodeReportDetails: adds what the text holds to the lists of the report; lines that do not fit are skipped */
void DecodeReportDetails( const GUCEF::CORE::CString& details, SNodeReport& report );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_NODEREPORT_H ? */
