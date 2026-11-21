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

#ifndef GUCEF_LABLINK_PROBES_H
#define GUCEF_LABLINK_PROBES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#include "gucefLabLink_Settings.h"
#include "gucefLabLink_NodeReport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The name the machine gives itself: the setting, or the host name */
GUCEF::CORE::CString MachineName( const SSettings& settings );

/** "linux", "windows" or "other" */
GUCEF::CORE::CString OsName( void );

/** The processor architecture as the operating system names it, "unknown" when it does not say */
GUCEF::CORE::CString ArchName( void );

/** Seconds since the machine booted; 0 when the operating system does not say */
GUCEF::UInt64 UptimeInSecs( void );

/** Identifies one boot of the machine: the same until the next boot */
GUCEF::CORE::CString BootId( void );

/** Free space of the volume that holds the path, in MiB; 0 when it cannot be read */
GUCEF::UInt64 FreeDiskInMiB( const GUCEF::CORE::CString& path );

/**
 *  The NVIDIA graphics cards of the machine, through nvidia-smi (which comes with the driver). It runs a program,
 *  so it takes a moment: the caller keeps the answer for a while. @return false when there is no nvidia-smi or it fails
 */
bool ProbeNvidiaGpus( GUCEF::vector< SGpuInfo >& gpus );

/**
 *  What a node with these resources can run: "mcp" always; "stt" and "tts" with 2 GiB of video memory or 4 GiB of
 *  free memory; "llm" with 6 GiB of video memory; "llm-cpu" with 16 GiB of free memory and no card for it; "image" with
 *  8 GiB of video memory. The hub decides with them which workloads a node can take; they are a starting point
 */
void DeriveCapabilities( SNodeReport& report );

/** CPU load and memory of the machine; the CPU load is the one since the previous call, the first call gives a measure over a short time */
class CResourceProbe
{
    public:

    CResourceProbe( void );

    ~CResourceProbe();

    /** Fills cores, CPU load, memory and the free disk of the volume of diskPath in the report */
    void Probe( const GUCEF::CORE::CString& diskPath, SNodeReport& report );

    private:

    CResourceProbe( const CResourceProbe& src );
    CResourceProbe& operator=( const CResourceProbe& src );

    GUCEF::CORE::TCpuDataPoint* m_cpuDataPoint;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_LABLINK_PROBES_H ? */
