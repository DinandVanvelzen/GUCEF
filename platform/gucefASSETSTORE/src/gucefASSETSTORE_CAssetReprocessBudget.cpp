/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#include <string.h>

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */
#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */
#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#include "gucefASSETSTORE_CAssetReprocessBudget.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt64 BytesPerMB = 1024 * 1024;

/** The limit in the file, when it exists and holds one */
bool
ReadLimitFile( const CORE::CString& path, UInt64& bytes )
{GUCEF_TRACE;

    CORE::CString text;
    if ( !CORE::FileExists( path ) || !CORE::LoadTextFileAsString( path, text, true, "\n" ) )
        return false;
    return CAssetReprocessBudget::ParseCgroupMemoryLimit( text, bytes );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetReprocessBudget::CAssetReprocessBudget( void )
    : logicalCores( 1 )
    , memoryLimitBytes( 0 )
    , memoryLimitSource( "unknown" )
    , concurrentJobs( 1 )
    , threadsPerJob( 1 )
    , memoryBudgetMB( 0 )
    , jobsAutomatic( true )
    , threadsAutomatic( true )
    , memoryAutomatic( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessBudget::Compute( const CAssetReprocessSettings& settings, UInt32 cores, UInt64 memoryLimit, const CORE::CString& source )
{GUCEF_TRACE;

    logicalCores = cores > 0 ? cores : 1;
    memoryLimitBytes = memoryLimit;
    memoryLimitSource = source.IsNULLOrEmpty() ? CORE::CString( "unknown" ) : source;

    jobsAutomatic = 0 == settings.maxConcurrentJobs;
    if ( jobsAutomatic )
    {
        concurrentJobs = logicalCores / 4;
        if ( concurrentJobs < 1 )
            concurrentJobs = 1;
        if ( concurrentJobs > 8 )
            concurrentJobs = 8;
    }
    else
        concurrentJobs = settings.maxConcurrentJobs;

    threadsAutomatic = 0 == settings.threadsPerJob;
    if ( threadsAutomatic )
    {
        const UInt32 usable = logicalCores > settings.reservedCores ? logicalCores - settings.reservedCores : 1;
        threadsPerJob = usable / concurrentJobs;
        if ( threadsPerJob < 1 )
            threadsPerJob = 1;
    }
    else
        threadsPerJob = settings.threadsPerJob;

    memoryAutomatic = 0 == settings.memoryBudgetMB;
    if ( memoryAutomatic )
        memoryBudgetMB = ( memoryLimitBytes / BytesPerMB ) * 70 / 100;
    else
        memoryBudgetMB = settings.memoryBudgetMB;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetReprocessBudget::EstimateJobMemoryMB( const CAssetReprocessSettings& settings, bool converts, UInt64 largestAssetBytes )
{GUCEF_TRACE;

    if ( !converts )
        return 0;
    UInt64 estimate = settings.memoryPerJobMB;
    const UInt64 ofLargest = ( largestAssetBytes * settings.memoryLargestAssetFactor + BytesPerMB - 1 ) / BytesPerMB;
    if ( ofLargest > estimate )
        estimate = ofLargest;
    return estimate;
}

/*-------------------------------------------------------------------------*/

void
CAssetReprocessBudget::Describe( CORE::CDataNode& node ) const
{GUCEF_TRACE;

    node.SetAttribute( "logicalCores", logicalCores );
    node.SetAttribute( "memoryLimitMB", memoryLimitBytes / BytesPerMB );
    node.SetAttribute( "memoryLimitSource", memoryLimitSource );
    node.SetAttribute( "concurrentJobs", concurrentJobs );
    node.SetAttribute( "concurrentJobsAutomatic", jobsAutomatic );
    node.SetAttribute( "threadsPerJob", threadsPerJob );
    node.SetAttribute( "threadsPerJobAutomatic", threadsAutomatic );
    node.SetAttribute( "memoryBudgetMB", memoryBudgetMB );
    node.SetAttribute( "memoryBudgetAutomatic", memoryAutomatic );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetReprocessBudget::DescribeText( void ) const
{GUCEF_TRACE;

    return CORE::ToString( concurrentJobs ) + " job(s) x " + CORE::ToString( threadsPerJob ) + " thread(s) of " + CORE::ToString( logicalCores ) + " logical cores, memory budget " +
           ( 0 == memoryBudgetMB ? CORE::CString( "none" ) : CORE::ToString( memoryBudgetMB ) + " MB" ) + " of " +
           ( 0 == memoryLimitBytes ? CORE::CString( "an unknown limit" ) : CORE::ToString( memoryLimitBytes / BytesPerMB ) + " MB" ) + " (" + memoryLimitSource + ")";
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetReprocessBudget::DetectLogicalCores( void )
{GUCEF_TRACE;

    const UInt32 cores = CORE::GetLogicalCPUCount();
    return cores > 0 ? cores : 1;
}

/*-------------------------------------------------------------------------*/

bool
CAssetReprocessBudget::ParseCgroupMemoryLimit( const CORE::CString& text, UInt64& bytes )
{GUCEF_TRACE;

    // the first word of the text: the files end with a line feed
    const char* chars = text.C_String();
    const UInt32 size = text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
    UInt32 start = 0;
    while ( start < size && ( ' ' == chars[ start ] || '\t' == chars[ start ] || '\n' == chars[ start ] || '\r' == chars[ start ] ) )
        ++start;
    UInt32 end = start;
    while ( end < size && chars[ end ] >= '0' && chars[ end ] <= '9' )
        ++end;
    if ( end == start || end - start > 20 )
        return false;
    if ( end < size && ' ' != chars[ end ] && '\t' != chars[ end ] && '\n' != chars[ end ] && '\r' != chars[ end ] )
        return false;
    const CORE::CString value( chars + start, end - start );
    const UInt64 parsed = CORE::StringToUInt64( value, 0 );
    // cgroup v1 says "no limit" with the largest multiple of the page size of a signed 64 bit number
    if ( 0 == parsed || parsed >= ( (UInt64) 1 << 62 ) )
        return false;
    bytes = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetReprocessBudget::ReadCgroupMemoryLimitBytes( const CORE::CString& cgroupRoot, const CORE::CString& selfCgroupFile, CORE::CString& source )
{GUCEF_TRACE;

    // the group of the process: "0::<path>" (v2) and "<n>:memory:<path>" or "<n>:...,memory,...:<path>" (v1)
    CORE::CString v2Path;
    CORE::CString v1Path;
    CORE::CString groups;
    if ( CORE::FileExists( selfCgroupFile ) && CORE::LoadTextFileAsString( selfCgroupFile, groups, true, "\n" ) )
    {
        const CORE::CString::StringVector lines = groups.ParseElements( '\n', false );
        for ( size_t l=0; l<lines.size(); ++l )
        {
            const CORE::CString::StringVector parts = lines[ l ].ParseElements( ':', true );
            if ( parts.size() < 3 )
                continue;
            CORE::CString path = parts[ 2 ];
            for ( size_t p=3; p<parts.size(); ++p )
                path += ":" + parts[ p ];
            path = path.Trim( true ).Trim( false );
            if ( "0" == parts[ 0 ] && parts[ 1 ].IsNULLOrEmpty() )
                v2Path = path;
            else
            {
                const CORE::CString::StringVector controllers = parts[ 1 ].ParseElements( ',', false );
                for ( size_t c=0; c<controllers.size(); ++c )
                    if ( "memory" == controllers[ c ] )
                        v1Path = path;
            }
        }
    }
    UInt64 bytes = 0;
    if ( !v2Path.IsNULLOrEmpty() && "/" != v2Path && ReadLimitFile( cgroupRoot + v2Path + "/memory.max", bytes ) )
    {
        source = "cgroup v2";
        return bytes;
    }
    if ( ReadLimitFile( cgroupRoot + "/memory.max", bytes ) )
    {
        source = "cgroup v2";
        return bytes;
    }
    if ( !v1Path.IsNULLOrEmpty() && "/" != v1Path && ReadLimitFile( cgroupRoot + "/memory" + v1Path + "/memory.limit_in_bytes", bytes ) )
    {
        source = "cgroup v1";
        return bytes;
    }
    if ( ReadLimitFile( cgroupRoot + "/memory/memory.limit_in_bytes", bytes ) )
    {
        source = "cgroup v1";
        return bytes;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CAssetReprocessBudget::DetectMemoryLimitBytes( CORE::CString& source )
{GUCEF_TRACE;

    UInt64 physical = 0;
    CORE::TGlobalMemoryUsageInfo memory;
    memset( &memory, 0, sizeof( memory ) );
    if ( OSWRAP_TRUE == CORE::GetGlobalMemoryUsage( &memory ) )
        physical = memory.totalPhysicalMemoryInBytes;

    UInt64 limit = 0;
    #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
    limit = ReadCgroupMemoryLimitBytes( "/sys/fs/cgroup", "/proc/self/cgroup", source );
    #endif
    if ( limit > 0 && ( 0 == physical || limit <= physical ) )
        return limit;
    if ( physical > 0 )
    {
        source = "machine";
        return physical;
    }
    source = "unknown";
    return 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
