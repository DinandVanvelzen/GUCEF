/*
 *  gucefCORE_TestApp: GUCEF test application for the CORE module
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
#include <string.h>
#include <stdlib.h>
#include <string>

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestOsWrapProcessAndCpu.h"

#if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #include <windows.h>
#elif ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <unistd.h>
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) )

CORE::TProcessId
OwnProcessId( void )
{
    #if ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
    return (CORE::TProcessId) ::GetCurrentProcessId();
    #else
    return (CORE::TProcessId) ::getpid();
    #endif
}

#endif

/** A two core x86 style file as the Linux kernel writes it, with the empty line after the last section */
const char* X86Sample =
    "processor\t: 0\n"
    "vendor_id\t: GenuineIntel\n"
    "cpu family\t: 6\n"
    "model\t\t: 158\n"
    "model name\t: Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz\n"
    "stepping\t: 13\n"
    "cpu MHz\t\t: 3600.000\n"
    "cache size\t: 12288 KB\n"
    "physical id\t: 0\n"
    "siblings\t: 8\n"
    "core id\t\t: 0\n"
    "cpu cores\t: 8\n"
    "apicid\t\t: 0\n"
    "initial apicid\t: 0\n"
    "fpu\t\t: yes\n"
    "fpu_exception\t: yes\n"
    "cpuid level\t: 22\n"
    "wp\t\t: yes\n"
    "flags\t\t: fpu vme sse2 pni ssse3 sse4_1 sse4_2 popcnt avx avx2 fma f16c bmi1 bmi2\n"
    "vmx flags\t: vnmi preemption_timer posted_intr\n"
    "bugs\t\t: spectre_v1 spectre_v2\n"
    "bogomips\t: 7200.00\n"
    "clflush size\t: 64\n"
    "cache_alignment\t: 64\n"
    "address sizes\t: 39 bits physical, 48 bits virtual\n"
    "power management:\n"
    "\n"
    "processor\t: 1\n"
    "vendor_id\t: GenuineIntel\n"
    "model name\t: Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz\n"
    "core id\t\t: 1\n"
    "flags\t\t: fpu sse2\n"
    "\n";

/** An ARM style file: a different capitalisation, a Features line, and a trailing board section that is not a CPU */
const char* ArmSample =
    "processor\t: 0\n"
    "BogoMIPS\t: 108.00\n"
    "Features\t: fp asimd evtstrm crc32 cpuid\n"
    "CPU implementer\t: 0x41\n"
    "CPU part\t: 0xd08\n"
    "\n"
    "processor\t: 1\n"
    "BogoMIPS\t: 108.00\n"
    "Features\t: fp asimd\n"
    "\n"
    "Hardware\t: BCM2711\n"
    "Revision\t: d03114\n"
    "Model\t\t: Raspberry Pi 4 Model B Rev 1.4\n";

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformOsWrapProcessAndCpuTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "OsWrapProcessAndCpu" );

    GUCEF_TESTFW_TESTCASE( "ProcessList: the C form gives the count and every id of the C++ form" )
    try
    {
        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN ) )

        CORE::TProcessId* ids = GUCEF_NULL;
        CORE::UInt32 count = 12345;
        ASSERT_TRUE( OSWRAP_TRUE == CORE::GetProcessList( &ids, &count ) );
        ASSERT_TRUE( GUCEF_NULL != ids );
        ASSERT_TRUE( count > 0 );

        CORE::TProcessIdVector processes;
        ASSERT_TRUE( CORE::GetProcessList( processes ) );
        ASSERT_TRUE( processes.size() > 0 );

        // processes may start or end between the two calls: the C list has to be about as long
        ASSERT_TRUE( count + 64 > processes.size() && processes.size() + 64 > count );

        // every entry of the C list is filled in (the entries used to be written at every second index only), and
        // the process we are is in it, as is every process that exists before and after both calls
        CORE::TProcessId self = OwnProcessId();
        bool foundSelf = false;
        for ( CORE::UInt32 i=0; i<count; ++i )
        {
            CORE::TProcessId id = CORE::GetProcessIdAtIndex( ids, i );
            ASSERT_TRUE( 0 != id );
            if ( id == self )
                foundSelf = true;
        }
        ASSERT_TRUE( foundSelf );

        CORE::UInt32 inBoth = 0;
        for ( size_t p=0; p<processes.size(); ++p )
        {
            for ( CORE::UInt32 i=0; i<count; ++i )
            {
                if ( CORE::GetProcessIdAtIndex( ids, i ) == processes[ p ] )
                {
                    ++inBoth;
                    break;
                }
            }
        }
        ASSERT_TRUE( inBoth + 64 > processes.size() );

        CORE::FreeProcessList( ids );

        // the arguments are checked
        ASSERT_TRUE( OSWRAP_FALSE == CORE::GetProcessList( GUCEF_NULL, &count ) );
        ASSERT_TRUE( OSWRAP_FALSE == CORE::GetProcessList( &ids, GUCEF_NULL ) );

        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuInfo: the fields, the flag lists and the sections of an x86 file" )
    try
    {
        CORE::TLogicalCpuDescriptionVector cpus;
        ASSERT_TRUE( CORE::ParseProcCpuInfo( X86Sample, (CORE::UInt32) strlen( X86Sample ), cpus ) );
        ASSERT_TRUE( 2 == cpus.size() );

        const CORE::CLogicalCpuDescription& a = cpus[ 0 ];
        ASSERT_TRUE( 0 == a.processor );
        ASSERT_TRUE( a.vendorId == "GenuineIntel" );
        ASSERT_TRUE( 6 == a.cpuFamily );
        ASSERT_TRUE( 158 == a.model );
        ASSERT_TRUE( a.modelName == "Intel(R) Core(TM) i7-9700K CPU @ 3.60GHz" );
        ASSERT_TRUE( 13 == a.stepping );
        ASSERT_TRUE( a.cpuMhz > 3599.9f && a.cpuMhz < 3600.1f );
        ASSERT_TRUE( 12288 * 1024 == a.cacheSizeInBytes );
        ASSERT_TRUE( 8 == a.siblings );
        ASSERT_TRUE( 8 == a.cpuCores );
        ASSERT_TRUE( a.fpu && a.fpuException && a.wp );
        ASSERT_TRUE( 22 == a.cpuidLevel );
        ASSERT_TRUE( a.bogomips > 7199.9f && a.bogomips < 7200.1f );
        ASSERT_TRUE( 64 == a.clflushSize );
        ASSERT_TRUE( 64 == a.cacheAlignment );
        ASSERT_TRUE( a.addressSizes == "39 bits physical, 48 bits virtual" );

        // the flags used to stay empty
        ASSERT_TRUE( 14 == a.flags.size() );
        ASSERT_TRUE( a.HasFlag( "sse2" ) && a.HasFlag( "avx2" ) && a.HasFlag( "bmi2" ) && a.HasFlag( "pni" ) );
        ASSERT_FALSE( a.HasFlag( "avx512f" ) );
        ASSERT_FALSE( a.HasFlag( "sse" ) );         // whole words only
        ASSERT_TRUE( 3 == a.vmxFlags.size() && a.vmxFlags.find( "posted_intr" ) != a.vmxFlags.end() );
        ASSERT_TRUE( 2 == a.bugs.size() && a.bugs.find( "spectre_v2" ) != a.bugs.end() );

        // the second section starts fresh: nothing of the first one is left in it
        const CORE::CLogicalCpuDescription& b = cpus[ 1 ];
        ASSERT_TRUE( 1 == b.processor );
        ASSERT_TRUE( 1 == b.coreId );
        ASSERT_TRUE( 2 == b.flags.size() && b.HasFlag( "fpu" ) && b.HasFlag( "sse2" ) );
        ASSERT_TRUE( 0 == b.vmxFlags.size() && 0 == b.bugs.size() );
        ASSERT_TRUE( 0 == b.stepping && 0 == b.cacheSizeInBytes && b.addressSizes.IsNULLOrEmpty() );
        ASSERT_FALSE( b.fpu );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuInfo: a flags line of any length, a last section without an empty line, no CPUs" )
    try
    {
        // a flags line of 12000 characters: every flag is kept and no extra CPU appears (a read buffer of 2048 characters
        // used to cut the line, and the rest of it was taken for a new section)
        std::string text = "processor\t: 0\nflags\t\t:";
        const UInt32 flagCount = 1500;
        for ( UInt32 i=0; i<flagCount; ++i )
        {
            char flag[ 32 ];
            sprintf( flag, " flag%04u", (unsigned int) i );
            text += flag;
        }
        text += "\nmodel name\t: after the long line\n";      // no empty line at the end
        CORE::TLogicalCpuDescriptionVector cpus;
        ASSERT_TRUE( CORE::ParseProcCpuInfo( text.c_str(), (CORE::UInt32) text.size(), cpus ) );
        ASSERT_TRUE( 1 == cpus.size() );
        ASSERT_TRUE( flagCount == cpus[ 0 ].flags.size() );
        ASSERT_TRUE( cpus[ 0 ].HasFlag( "flag0000" ) && cpus[ 0 ].HasFlag( "flag1499" ) );
        ASSERT_TRUE( cpus[ 0 ].modelName == "after the long line" );

        // the results are appended to what is in the list
        ASSERT_TRUE( CORE::ParseProcCpuInfo( X86Sample, (CORE::UInt32) strlen( X86Sample ), cpus ) );
        ASSERT_TRUE( 3 == cpus.size() );

        // nothing to find
        CORE::TLogicalCpuDescriptionVector none;
        ASSERT_FALSE( CORE::ParseProcCpuInfo( GUCEF_NULL, 10, none ) );
        ASSERT_FALSE( CORE::ParseProcCpuInfo( "", 0, none ) );
        ASSERT_FALSE( CORE::ParseProcCpuInfo( "\n\n\n", 3, none ) );
        ASSERT_FALSE( CORE::ParseProcCpuInfo( "nothing here\nand not here: either\n", 33, none ) );
        ASSERT_TRUE( 0 == none.size() );

        // Windows line ends and an unterminated last line
        const char* crlf = "processor : 3\r\nflags : a b\r\n\r\nprocessor : 4\r\nflags : c";
        ASSERT_TRUE( CORE::ParseProcCpuInfo( crlf, (CORE::UInt32) strlen( crlf ), none ) );
        ASSERT_TRUE( 2 == none.size() );
        ASSERT_TRUE( 3 == none[ 0 ].processor && 2 == none[ 0 ].flags.size() && none[ 0 ].HasFlag( "b" ) );
        ASSERT_TRUE( 4 == none[ 1 ].processor && 1 == none[ 1 ].flags.size() && none[ 1 ].HasFlag( "c" ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuInfo: an ARM style file gives its features, and a board section is not a CPU" )
    try
    {
        CORE::TLogicalCpuDescriptionVector cpus;
        ASSERT_TRUE( CORE::ParseProcCpuInfo( ArmSample, (CORE::UInt32) strlen( ArmSample ), cpus ) );
        ASSERT_TRUE( 2 == cpus.size() );
        ASSERT_TRUE( 5 == cpus[ 0 ].flags.size() && cpus[ 0 ].HasFlag( "asimd" ) && cpus[ 0 ].HasFlag( "crc32" ) );
        ASSERT_TRUE( cpus[ 0 ].bogomips > 107.9f && cpus[ 0 ].bogomips < 108.1f );
        ASSERT_TRUE( 2 == cpus[ 1 ].flags.size() );
        ASSERT_TRUE( 1 == cpus[ 1 ].processor );
        ASSERT_TRUE( 0 == cpus[ 1 ].model );        // "Model" of the board is not the numeric 'model' of a CPU
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuInfo: the logical CPUs of this system, with flags on x86 Linux" )
    try
    {
        #if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )

        CORE::TLogicalCpuDescriptionVector cpus;
        ASSERT_TRUE( CORE::GetLogicalCpuDescriptions( cpus ) );
        ASSERT_TRUE( cpus.size() > 0 );

        // every logical CPU the system has is described once; the count of the operating system can be larger in
        // restricted environments (a container with fewer CPUs visible), never smaller
        ASSERT_TRUE( cpus.size() <= CORE::GetLogicalCPUCount() + 0 || CORE::GetLogicalCPUCount() == 0 );

        for ( size_t i=0; i<cpus.size(); ++i )
        {
            ASSERT_TRUE( !cpus[ i ].flags.empty() );
            if ( i > 0 )
                ASSERT_TRUE( cpus[ i ].processor != cpus[ i-1 ].processor );
        }

        #if defined( __x86_64__ ) || defined( __i386__ )
        // the probe and the operating system agree, one flag from each family
        const CORE::UInt64 isa = CORE::GetCpuIsaFeatures();
        ASSERT_TRUE( cpus[ 0 ].HasFlag( "sse2" ) == ( 0 != ( isa & GUCEF_CPU_ISA_SSE2 ) ) );
        ASSERT_TRUE( cpus[ 0 ].HasFlag( "avx2" ) == ( 0 != ( isa & GUCEF_CPU_ISA_AVX2 ) ) );
        ASSERT_TRUE( cpus[ 0 ].HasFlag( "popcnt" ) == ( 0 != ( isa & GUCEF_CPU_ISA_POPCNT ) ) );
        #endif

        #endif
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
