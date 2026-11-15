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

/* An independent cpuid / xgetbv for the operating system support check, behind the same kind of compiler guards */
#if defined( __x86_64__ ) || defined( __i386__ ) || defined( _M_X64 ) || defined( _M_IX86 )
  #if defined( _MSC_VER )
    #include <intrin.h>
    #include <immintrin.h>
    #define GUCEF_CORE_TEST_CPUID_MSVC
  #elif defined( __GNUC__ ) || defined( __clang__ )
    #include <cpuid.h>
    #define GUCEF_CORE_TEST_CPUID_GNUC
  #endif
#endif

#ifndef GUCEF_CORE_DVOSWRAP_H
#include "DVOSWRAP.h"
#define GUCEF_CORE_DVOSWRAP_H
#endif /* GUCEF_CORE_DVOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#include "TestCpuIsaFeatures.h"

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

/** Every feature bit with the flag name Linux uses for it in /proc/cpuinfo (x86 only; GUCEF_NULL elsewhere) */
struct SIsaFeatureFlag
{
    UInt64 feature;
    const char* cpuInfoFlag;
};

const SIsaFeatureFlag AllFeatures[] =
{
    { GUCEF_CPU_ISA_SSE2, "sse2" },
    { GUCEF_CPU_ISA_SSE3, "pni" },              // Linux names SSE3 "Prescott New Instructions"
    { GUCEF_CPU_ISA_SSSE3, "ssse3" },
    { GUCEF_CPU_ISA_SSE41, "sse4_1" },
    { GUCEF_CPU_ISA_SSE42, "sse4_2" },
    { GUCEF_CPU_ISA_POPCNT, "popcnt" },
    { GUCEF_CPU_ISA_AVX, "avx" },
    { GUCEF_CPU_ISA_AVX2, "avx2" },
    { GUCEF_CPU_ISA_FMA3, "fma" },
    { GUCEF_CPU_ISA_F16C, "f16c" },
    { GUCEF_CPU_ISA_BMI1, "bmi1" },
    { GUCEF_CPU_ISA_BMI2, "bmi2" },
    { GUCEF_CPU_ISA_LZCNT, "abm" },             // LZCNT is the "advanced bit manipulation" flag
    { GUCEF_CPU_ISA_AVX512F, "avx512f" },
    { GUCEF_CPU_ISA_NEON, GUCEF_NULL },
    { GUCEF_CPU_ISA_ARM_FP16, GUCEF_NULL },
    { GUCEF_CPU_ISA_ARM_DOTPROD, GUCEF_NULL },
    { GUCEF_CPU_ISA_SVE, GUCEF_NULL },
    { GUCEF_CPU_ISA_WASM_SIMD128, GUCEF_NULL }
};
const UInt32 AllFeatureCount = sizeof( AllFeatures ) / sizeof( AllFeatures[ 0 ] );

const UInt64 AvxStateFeatures = GUCEF_CPU_ISA_AVX | GUCEF_CPU_ISA_AVX2 | GUCEF_CPU_ISA_FMA3 | GUCEF_CPU_ISA_F16C | GUCEF_CPU_ISA_AVX512F;

/*-------------------------------------------------------------------------*/

#if defined( GUCEF_CORE_TEST_CPUID_MSVC ) || defined( GUCEF_CORE_TEST_CPUID_GNUC )

/** Whether the operating system saves the YMM register state, found with cpuid and xgetbv of the test itself */
bool
OperatingSystemSavesAvxState( void )
{
    UInt32 ecx = 0;
    #if defined( GUCEF_CORE_TEST_CPUID_MSVC )
    int info[ 4 ] = { 0, 0, 0, 0 };
    __cpuid( info, 1 );
    ecx = (UInt32) info[ 2 ];
    #else
    unsigned int a = 0, b = 0, c = 0, d = 0;
    if ( 0 == __get_cpuid( 1, &a, &b, &c, &d ) )
        return false;
    ecx = c;
    #endif
    if ( 0 == ( ecx & ( 1u << 27 ) ) )      // OSXSAVE
        return false;
    #if defined( GUCEF_CORE_TEST_CPUID_MSVC )
    const UInt64 xcr0 = (UInt64) _xgetbv( 0 );
    #else
    UInt32 eax = 0, edx = 0;
    __asm__ __volatile__( "xgetbv" : "=a" ( eax ), "=d" ( edx ) : "c" ( 0 ) );
    const UInt64 xcr0 = ( (UInt64) edx << 32 ) | eax;
    #endif
    return 0x06 == ( xcr0 & 0x06 );
}

#endif

/*-------------------------------------------------------------------------*/

#if defined( __linux__ )

/**
 *  The "flags" line of the first processor in /proc/cpuinfo with a space before and after every flag, so a flag is
 *  found as " name ". False when the file or the line is not there.
 */
bool
ReadProcCpuInfoFlags( char* buffer, UInt32 bufferSize )
{
    FILE* file = fopen( "/proc/cpuinfo", "r" );
    if ( GUCEF_NULL == file )
        return false;
    bool found = false;
    char line[ 16384 ];
    while ( !found && GUCEF_NULL != fgets( line, (int) sizeof( line ), file ) )
    {
        if ( 0 != strncmp( line, "flags", 5 ) )
            continue;
        const char* colon = strchr( line, ':' );
        if ( GUCEF_NULL == colon )
            continue;
        UInt32 length = (UInt32) strlen( colon + 1 );
        while ( length > 0 && ( '\n' == colon[ length ] || '\r' == colon[ length ] ) )
            --length;
        if ( length + 3 > bufferSize )
            break;
        buffer[ 0 ] = ' ';
        memcpy( buffer + 1, colon + 1, length );
        buffer[ length + 1 ] = ' ';
        buffer[ length + 2 ] = '\0';
        found = true;
    }
    fclose( file );
    return found;
}

/*-------------------------------------------------------------------------*/

bool
HasCpuInfoFlag( const char* paddedFlags, const char* flag )
{
    char needle[ 64 ];
    const size_t length = strlen( flag );
    if ( length + 3 > sizeof( needle ) )
        return false;
    needle[ 0 ] = ' ';
    memcpy( needle + 1, flag, length );
    needle[ length + 1 ] = ' ';
    needle[ length + 2 ] = '\0';
    return GUCEF_NULL != strstr( paddedFlags, needle );
}

#endif /* __linux__ ? */

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformCpuIsaFeaturesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "CpuIsaFeatures" );

    GUCEF_TESTFW_TESTCASE( "CpuIsaFeatures: only known bits, the same value on every call" )
    try
    {
        const UInt64 features = CORE::GetCpuIsaFeatures();
        ASSERT_TRUE( 0 == ( features & ~GUCEF_CPU_ISA_ALL_KNOWN ) );
        for ( UInt32 i=0; i<100; ++i )
            ASSERT_TRUE( features == CORE::GetCpuIsaFeatures() );

        for ( UInt32 i=0; i<AllFeatureCount; ++i )
        {
            if ( 0 != ( features & AllFeatures[ i ].feature ) )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CpuIsaFeatures: the CPU reports " << CORE::GetCpuIsaFeatureName( AllFeatures[ i ].feature ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuIsaFeatures: every bit has a unique name, other values have none" )
    try
    {
        UInt64 covered = 0;
        for ( UInt32 i=0; i<AllFeatureCount; ++i )
        {
            const char* name = CORE::GetCpuIsaFeatureName( AllFeatures[ i ].feature );
            ASSERT_TRUE( GUCEF_NULL != name && 0 != name[ 0 ] );
            for ( UInt32 j=0; j<i && GUCEF_NULL != name; ++j )
            {
                const char* other = CORE::GetCpuIsaFeatureName( AllFeatures[ j ].feature );
                ASSERT_TRUE( GUCEF_NULL != other && 0 != strcmp( name, other ) );
            }
            covered |= AllFeatures[ i ].feature;
        }
        ASSERT_TRUE( GUCEF_CPU_ISA_ALL_KNOWN == covered );
        ASSERT_TRUE( 0 == strcmp( "SSE2", CORE::GetCpuIsaFeatureName( GUCEF_CPU_ISA_SSE2 ) ) );
        ASSERT_TRUE( 0 == strcmp( "AVX2", CORE::GetCpuIsaFeatureName( GUCEF_CPU_ISA_AVX2 ) ) );
        ASSERT_TRUE( 0 == strcmp( "NEON", CORE::GetCpuIsaFeatureName( GUCEF_CPU_ISA_NEON ) ) );

        // no bit, two bits, and every bit position that is not a feature
        ASSERT_TRUE( GUCEF_NULL == CORE::GetCpuIsaFeatureName( 0 ) );
        ASSERT_TRUE( GUCEF_NULL == CORE::GetCpuIsaFeatureName( GUCEF_CPU_ISA_SSE2 | GUCEF_CPU_ISA_AVX2 ) );
        for ( UInt32 bit=0; bit<64; ++bit )
        {
            const UInt64 value = (UInt64) 1 << bit;
            if ( 0 == ( value & GUCEF_CPU_ISA_ALL_KNOWN ) )
                ASSERT_TRUE( GUCEF_NULL == CORE::GetCpuIsaFeatureName( value ) );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuIsaFeatures: SSE2 on x86-64, the AVX family only with operating system support" )
    #if !defined( GUCEF_CORE_TEST_CPUID_MSVC ) && !defined( GUCEF_CORE_TEST_CPUID_GNUC )
    GUCEF_TESTFW_SKIP( "not an x86 build with cpuid support" );
    #else
    try
    {
        const UInt64 features = CORE::GetCpuIsaFeatures();
        #if defined( __x86_64__ ) || defined( _M_X64 )
        ASSERT_TRUE( 0 != ( features & GUCEF_CPU_ISA_SSE2 ) );
        #endif
        ASSERT_TRUE( 0 == ( features & ( GUCEF_CPU_ISA_NEON | GUCEF_CPU_ISA_ARM_FP16 | GUCEF_CPU_ISA_ARM_DOTPROD | GUCEF_CPU_ISA_SVE | GUCEF_CPU_ISA_WASM_SIMD128 ) ) );

        // every member of the AVX family needs the AVX register state, so AVX itself comes with each of them
        if ( 0 != ( features & ( GUCEF_CPU_ISA_AVX2 | GUCEF_CPU_ISA_FMA3 | GUCEF_CPU_ISA_F16C | GUCEF_CPU_ISA_AVX512F ) ) )
            ASSERT_TRUE( 0 != ( features & GUCEF_CPU_ISA_AVX ) );
        if ( !OperatingSystemSavesAvxState() )
            ASSERT_TRUE( 0 == ( features & AvxStateFeatures ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "CpuIsaFeatures: x86 features agree with the flags of /proc/cpuinfo" )
    #if !defined( __linux__ ) || !( defined( __x86_64__ ) || defined( __i386__ ) )
    GUCEF_TESTFW_SKIP( "not x86 Linux" );
    #else
    char flags[ 16384 ];
    GUCEF_TESTFW_SKIP_IF( !ReadProcCpuInfoFlags( flags, (UInt32) sizeof( flags ) ), "no flags line in /proc/cpuinfo" );
    try
    {
        const UInt64 features = CORE::GetCpuIsaFeatures();
        for ( UInt32 i=0; i<AllFeatureCount; ++i )
        {
            if ( GUCEF_NULL == AllFeatures[ i ].cpuInfoFlag )
                continue;
            const bool reported = 0 != ( features & AllFeatures[ i ].feature );
            const bool listed = HasCpuInfoFlag( flags, AllFeatures[ i ].cpuInfoFlag );
            if ( reported != listed )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CpuIsaFeatures: disagreement with /proc/cpuinfo for " << CORE::GetCpuIsaFeatureName( AllFeatures[ i ].feature ) );
            ASSERT_TRUE( reported == listed );
        }
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
