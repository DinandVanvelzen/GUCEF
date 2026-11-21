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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gucefCORE_macros.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
  #include <unistd.h>
  #include <sys/utsname.h>
  #include <sys/statvfs.h>
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#endif

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"            /* GetEnv() */
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_Probes.h"
#include "gucefLabLink_PlatformDefaults.h"
#include "gucefLabLink_WindowsHost.h"

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

CORE::CString
MachineName( const SSettings& settings )
{GUCEF_TRACE;

    if ( !settings.machineName.IsNULLOrEmpty() )
        return settings.machineName;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    char name[ 256 ];
    if ( 0 == ::gethostname( name, sizeof( name ) - 1 ) )
    {
        name[ sizeof( name ) - 1 ] = '\0';
        return CORE::CString( name ).Lowercase();
    }
    return "machine";
    #else
    return CORE::GetEnv( "COMPUTERNAME" ).Lowercase();
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
OsName( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    return "linux";
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return "windows";
    #else
    return "other";
    #endif
}

/*-------------------------------------------------------------------------*/

CORE::CString
ArchName( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    struct utsname info;
    if ( 0 == ::uname( &info ) )
        return info.machine;
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return WindowsArchName( NativeProcessorArchitecture() );
    #endif
    return "unknown";
}

/*-------------------------------------------------------------------------*/

UInt64
UptimeInSecs( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    CORE::CDynamicBuffer content;
    if ( content.LoadContentFromFile( "/proc/uptime" ) && content.GetDataSize() > 0 )
    {
        const CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
        return (UInt64) ::strtoull( text.C_String(), GUCEF_NULL, 10 );
    }
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (UInt64) ( ::GetTickCount64() / 1000 );
    #endif
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
BootId( void )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    CORE::CDynamicBuffer content;
    if ( content.LoadContentFromFile( "/proc/sys/kernel/random/boot_id" ) && content.GetDataSize() > 0 )
        return StripText( CORE::CString( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() ) );
    #endif
    // the moment of the boot, to the minute: the same for as long as the machine runs
    const UInt64 now = NowInUnixSecs();
    const UInt64 uptime = UptimeInSecs();
    return "boot-" + CORE::ToString( ( now > uptime ? now - uptime : 0 ) / 60 );
}

/*-------------------------------------------------------------------------*/

UInt64
FreeDiskInMiB( const CORE::CString& path )
{GUCEF_TRACE;

    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX )
    struct statvfs info;
    if ( 0 == ::statvfs( path.IsNULLOrEmpty() ? "/" : path.C_String(), &info ) )
        return ( (UInt64) info.f_bavail * (UInt64) info.f_frsize ) / ( 1024 * 1024 );
    #elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    ULARGE_INTEGER freeToCaller;
    if ( ::GetDiskFreeSpaceExA( path.IsNULLOrEmpty() ? GUCEF_NULL : path.C_String(), &freeToCaller, GUCEF_NULL, GUCEF_NULL ) )
        return (UInt64) ( freeToCaller.QuadPart / ( 1024 * 1024 ) );
    #endif
    return 0;
}

/*-------------------------------------------------------------------------*/

/** The text of a field of nvidia-smi output, the digits of it; 0 when there are none (such as "[N/A]") */
static UInt32
Digits( const CORE::CString& text )
{GUCEF_TRACE;

    const CORE::CString stripped = StripText( text );
    if ( stripped.IsNULLOrEmpty() || stripped[ 0 ] < '0' || stripped[ 0 ] > '9' )
        return 0;
    return CORE::StringToUInt32( stripped, 0 );
}

/*-------------------------------------------------------------------------*/

bool
ProbeNvidiaGpus( vector< SGpuInfo >& gpus )
{GUCEF_TRACE;

    gpus.clear();
    CORE::CProcess::TStringVector args;
    args.push_back( "--query-gpu=name,memory.total,memory.free,utilization.gpu" );
    args.push_back( "--format=csv,noheader,nounits" );
    std::string output;
    Int32 exitCode = -1;
    if ( !RunProgram( "nvidia-smi", args, CORE::CProcess::TStringMap(), 5000, output, exitCode ) || 0 != exitCode )
        return false;

    const CORE::CString::StringVector lines = CORE::CString( output.c_str() ).ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size() && gpus.size() < MaxReportGpus; ++i )
    {
        const CORE::CString::StringVector fields = StripText( lines[ i ] ).ParseElements( ',', true );
        if ( fields.size() < 4 )
            continue;
        SGpuInfo gpu;
        gpu.name = StripText( fields[ 0 ] );
        gpu.vendor = "nvidia";
        gpu.vramTotalInMiB = Digits( fields[ 1 ] );
        gpu.vramFreeInMiB = Digits( fields[ 2 ] );
        gpu.utilizationPercent = Digits( fields[ 3 ] );
        if ( !gpu.name.IsNULLOrEmpty() )
            gpus.push_back( gpu );
    }
    return !gpus.empty();
}

/*-------------------------------------------------------------------------*/

void
DeriveCapabilities( SNodeReport& report )
{GUCEF_TRACE;

    UInt32 bestVramInMiB = 0;
    for ( size_t i=0; i<report.gpus.size(); ++i )
        if ( report.gpus[ i ].vramTotalInMiB > bestVramInMiB )
            bestVramInMiB = report.gpus[ i ].vramTotalInMiB;

    report.capabilities.clear();
    report.capabilities.push_back( "mcp" );
    if ( bestVramInMiB >= 2048 || report.memTotalInMiB >= 4096 )
    {
        report.capabilities.push_back( "stt" );
        report.capabilities.push_back( "tts" );
    }
    if ( bestVramInMiB >= 6144 )
        report.capabilities.push_back( "llm" );
    else
    if ( report.memTotalInMiB >= 16384 )
        report.capabilities.push_back( "llm-cpu" );
    if ( bestVramInMiB >= 8192 )
        report.capabilities.push_back( "image" );
}

/*-------------------------------------------------------------------------*/

CResourceProbe::CResourceProbe( void )
    : m_cpuDataPoint( CORE::CreateCpuDataPoint() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CResourceProbe::~CResourceProbe()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_cpuDataPoint )
    {
        CORE::FreeCpuDataPoint( m_cpuDataPoint );
        m_cpuDataPoint = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CResourceProbe::Probe( const CORE::CString& diskPath, SNodeReport& report )
{GUCEF_TRACE;

    CORE::TCpuStats* cpuStats = GUCEF_NULL;
    if ( GUCEF_NULL != m_cpuDataPoint && OSWRAP_TRUE == CORE::GetCpuStats( m_cpuDataPoint, &cpuStats ) && GUCEF_NULL != cpuStats )
    {
        report.cpuCores = cpuStats->logicalCpuCount;
        const Float64 load = cpuStats->cpuUsePercentage;
        report.cpuLoadPercent = load < 0 ? 0 : ( load > 100 ? 100 : (UInt32) ( load + 0.5 ) );
    }

    CORE::TGlobalMemoryUsageInfo memory;
    ::memset( &memory, 0, sizeof( memory ) );
    if ( OSWRAP_TRUE == CORE::GetGlobalMemoryUsage( &memory ) )
    {
        report.memTotalInMiB = memory.totalPhysicalMemoryInBytes / ( 1024 * 1024 );
        report.memAvailableInMiB = memory.availablePhysicalMemoryInBytes / ( 1024 * 1024 );
    }
    report.diskFreeInMiB = FreeDiskInMiB( diskPath );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
