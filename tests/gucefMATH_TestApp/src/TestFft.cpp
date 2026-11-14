/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#include <math.h>
#include <stdlib.h>
#include <new>
#if defined( __linux__ )
#include <pthread.h>
#endif

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_CREALFFT_H
#include "gucefMATH_CRealFft.h"
#define GUCEF_MATH_CREALFFT_H
#endif /* GUCEF_MATH_CREALFFT_H ? */

#ifndef GUCEF_MATH_CCOMPLEXFFT_H
#include "gucefMATH_CComplexFft.h"
#define GUCEF_MATH_CCOMPLEXFFT_H
#endif /* GUCEF_MATH_CCOMPLEXFFT_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#include "TestFftHashing.h"
#include "TestFft.h"

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
//      ALLOCATION COUNTING                                                //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Counts the calls of operator new of the counting thread while armed. The replacement is for the whole test
 *  application and forwards to malloc; the RealtimeSanitizer build additionally catches every malloc.
 */

static volatile bool g_countAllocations = false;
static volatile UInt32 g_allocationCount = 0;
#if defined( __linux__ )
static pthread_t g_countingThread;
#endif

static void
CountAllocation( void )
{
    if ( !g_countAllocations )
        return;
    #if defined( __linux__ )
    if ( 0 == pthread_equal( pthread_self(), g_countingThread ) )
        return;
    #endif
    ++g_allocationCount;
}

#if defined( __cplusplus ) && ( __cplusplus >= 201103L )
  #define FFTTEST_NEW_SPEC
  #define FFTTEST_DELETE_SPEC noexcept
#else
  #define FFTTEST_NEW_SPEC throw( std::bad_alloc )
  #define FFTTEST_DELETE_SPEC throw()
#endif

void* operator new( std::size_t size ) FFTTEST_NEW_SPEC
{
    CountAllocation();
    void* memory = malloc( 0 == size ? 1 : size );
    if ( GUCEF_NULL == memory )
        throw std::bad_alloc();
    return memory;
}

void* operator new[]( std::size_t size ) FFTTEST_NEW_SPEC
{
    CountAllocation();
    void* memory = malloc( 0 == size ? 1 : size );
    if ( GUCEF_NULL == memory )
        throw std::bad_alloc();
    return memory;
}

void operator delete( void* memory ) FFTTEST_DELETE_SPEC { free( memory ); }
void operator delete[]( void* memory ) FFTTEST_DELETE_SPEC { free( memory ); }
#if defined( __cplusplus ) && ( __cplusplus >= 201402L )
void operator delete( void* memory, std::size_t ) FFTTEST_DELETE_SPEC { free( memory ); }
void operator delete[]( void* memory, std::size_t ) FFTTEST_DELETE_SPEC { free( memory ); }
#endif

static void
StartCounting( void )
{
    #if defined( __linux__ )
    g_countingThread = pthread_self();
    #endif
    g_allocationCount = 0;
    g_countAllocations = true;
}

static UInt32
StopCounting( void )
{
    g_countAllocations = false;
    return g_allocationCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PINNED HASHES                                                      //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  FNV-1a 64 bit hashes of the outputs of the real FFT as it was in gucefAUDIO before the move to gucefMATH, captured
 *  with the signals and the layout of TestFftHashing.h. Index: FftHashing::HashIndex( size index, precision, mode ).
 */
static const UInt64 PinnedHashes[ 56 ] =
{
    0xaa7deba905d4b702ULL, 0x3a9ad61203aff452ULL, 0xaa7deba905d4b702ULL, 0x3a9ad61203aff452ULL,
    0xe979a8211e070db5ULL, 0x0a7e74394aed2595ULL, 0xe979a8211e070db5ULL, 0x0a7e74394aed2595ULL,
    0x9aaa042f3abe7e6aULL, 0x18e890468552e8f0ULL, 0x9aaa042f3abe7e6aULL, 0x18e890468552e8f0ULL,
    0xd8d3c9a71d4f3526ULL, 0x7913897c6e577e39ULL, 0xd8d3c9a71d4f3526ULL, 0x7913897c6e577e39ULL,
    0x745bee0ec9a60419ULL, 0x2e540dc0c9dac80bULL, 0x745bee0ec9a60419ULL, 0x2e540dc0c9dac80bULL,
    0xd9a95da7986bc7deULL, 0x2603fdf7e84ae7aaULL, 0xd9a95da7986bc7deULL, 0x2603fdf7e84ae7aaULL,
    0x0b4989ab0913655aULL, 0x8cb0abbb9034381eULL, 0x0b4989ab0913655aULL, 0x8cb0abbb9034381eULL,
    0xc0e9b19a3f03a2f7ULL, 0x504da564e056bdedULL, 0xc0e9b19a3f03a2f7ULL, 0x504da564e056bdedULL,
    0xbe472c69a20b7338ULL, 0x371f326306752654ULL, 0xbe472c69a20b7338ULL, 0x371f326306752654ULL,
    0x9906ce0f023821b9ULL, 0x32e725a28256ecadULL, 0x9906ce0f023821b9ULL, 0x32e725a28256ecadULL,
    0xdb83fb2040454f37ULL, 0xd456cf2a72174c98ULL, 0xdb83fb2040454f37ULL, 0xd456cf2a72174c98ULL,
    0xb2c399cfc4c34f64ULL, 0x00beb35831ca9b21ULL, 0xb2c399cfc4c34f64ULL, 0x00beb35831ca9b21ULL,
    0x269d9ab2ea52e73eULL, 0x807291404c6d8f82ULL, 0x269d9ab2ea52e73eULL, 0x807291404c6d8f82ULL,
    0x908d791375db958fULL, 0x7d2272181f370506ULL, 0x908d791375db958fULL, 0x7d2272181f370506ULL
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef GUCEF::vector< Float64 > TDoubles;
typedef MATH::CTComplex< Float64 > TComplex64;
typedef MATH::CTComplex< Float32 > TComplex32;

static const Float64 Pi = 3.14159265358979323846;

/** Calls marked as a real time context: in a RealtimeSanitizer build any allocation or lock in them is reported */
template< typename TFft, typename T >
static bool
ForwardRealtime( TFft& fft, const T* input, T* output ) GUCEF_MATH_REALTIME_CONTEXT
{
    return fft.Forward( input, output );
}

template< typename TFft, typename T >
static bool
InverseRealtime( TFft& fft, const T* input, T* output ) GUCEF_MATH_REALTIME_CONTEXT
{
    return fft.Inverse( input, output );
}

static Float64
NextValue( MATH::CPcg32& random )
{
    return (Float64) random.NextUInt32() / 2147483648.0 - 1.0;
}

/** Direct DFT of real input, bins 0 to N/2, real and imaginary part alternating */
static TDoubles
DirectRealDft( const TDoubles& x )
{
    const size_t size = x.size();
    TDoubles spectrum( size + 2 );
    for ( size_t k = 0; k <= size / 2; ++k )
    {
        Float64 re = 0.0;
        Float64 im = 0.0;
        for ( size_t n = 0; n < size; ++n )
        {
            const Float64 angle = 2.0 * Pi * (Float64) ( ( k * n ) % size ) / (Float64) size;
            re += x[ n ] * cos( angle );
            im -= x[ n ] * sin( angle );
        }
        spectrum[ 2 * k ] = re;
        spectrum[ 2 * k + 1 ] = im;
    }
    return spectrum;
}

template< typename T >
static Float64
LargestError( const GUCEF::vector< T >& computed, const TDoubles& reference, size_t length )
{
    Float64 largestError = 0.0;
    Float64 largest = 0.0;
    for ( size_t i = 0; i < length; ++i )
    {
        const Float64 e = fabs( (Float64) computed[ i ] - reference[ i ] );
        if ( e > largestError ) largestError = e;
        if ( fabs( reference[ i ] ) > largest ) largest = fabs( reference[ i ] );
    }
    return largest > 0.0 ? largestError / largest : largestError;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
PerformIdentityTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Fft: MATH::CRealFft reproduces every pinned hash of the gucefAUDIO transform (sizes 64 to 4096, both precisions, in and out of place)" )
    {
        UInt64 hashes[ 56 ];
        FftHashing::ComputeAllHashes< MATH::CRealFft >( hashes );
        UInt32 mismatches = 0;
        for ( UInt32 i = 0; i < 56; ++i )
        {
            if ( hashes[ i ] != PinnedHashes[ i ] )
            {
                ++mismatches;
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Fft: hash " << i << " differs from the pinned value" );
            }
        }
        ASSERT_TRUE( 0 == mismatches );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

template< typename T >
static bool
CheckRealProperties( UInt32 size, Float64 roundTripTolerance, Float64 parsevalTolerance, Float64 exactTolerance )
{
    MATH::CRealFft fft;
    if ( !fft.Prepare( size ) )
        return false;
    const size_t spectrumLength = (size_t) size + 2;
    GUCEF::vector< T > spectrum( spectrumLength );
    GUCEF::vector< T > back( size );
    bool ok = true;

    // impulse: a flat spectrum of 1
    GUCEF::vector< T > impulse( size, (T) 0 );
    impulse[ 0 ] = (T) 1;
    ok = ok && ForwardRealtime( fft, &impulse[ 0 ], &spectrum[ 0 ] );
    for ( size_t k = 0; k <= size / 2; ++k )
        ok = ok && fabs( (Float64) spectrum[ 2 * k ] - 1.0 ) <= exactTolerance && fabs( (Float64) spectrum[ 2 * k + 1 ] ) <= exactTolerance;

    // sine of amplitude A on bin k: magnitude A N / 2
    const UInt32 bin = size / 8 + 1;
    const Float64 amplitude = 0.8;
    GUCEF::vector< T > sine( size );
    for ( UInt32 n = 0; n < size; ++n )
        sine[ n ] = (T) ( amplitude * sin( 2.0 * Pi * (Float64) ( ( (UInt64) bin * n ) % size ) / (Float64) size ) );
    ok = ok && ForwardRealtime( fft, &sine[ 0 ], &spectrum[ 0 ] );
    const Float64 magnitude = sqrt( (Float64) spectrum[ 2 * bin ] * spectrum[ 2 * bin ] + (Float64) spectrum[ 2 * bin + 1 ] * spectrum[ 2 * bin + 1 ] );
    ok = ok && fabs( magnitude - amplitude * size / 2.0 ) <= exactTolerance * size;

    // seeded signal: Parseval, round trip, in place equals out of place, imaginary parts of DC and Nyquist are 0
    MATH::CPcg32 random( 100 + size, 4 );
    GUCEF::vector< T > x( size );
    Float64 energy = 0.0;
    Float64 largest = 0.0;
    for ( UInt32 n = 0; n < size; ++n )
    {
        x[ n ] = (T) NextValue( random );
        energy += (Float64) x[ n ] * x[ n ];
        if ( fabs( (Float64) x[ n ] ) > largest ) largest = fabs( (Float64) x[ n ] );
    }
    ok = ok && ForwardRealtime( fft, &x[ 0 ], &spectrum[ 0 ] );
    ok = ok && 0 == spectrum[ 1 ] && 0 == spectrum[ size + 1 ];
    Float64 spectral = 0.0;
    for ( size_t k = 0; k <= size / 2; ++k )
    {
        const Float64 power = (Float64) spectrum[ 2 * k ] * spectrum[ 2 * k ] + (Float64) spectrum[ 2 * k + 1 ] * spectrum[ 2 * k + 1 ];
        spectral += ( 0 == k || size / 2 == k ) ? power : 2.0 * power;
    }
    ok = ok && fabs( spectral / size / energy - 1.0 ) <= parsevalTolerance;

    ok = ok && InverseRealtime( fft, &spectrum[ 0 ], &back[ 0 ] );
    Float64 worst = 0.0;
    for ( UInt32 n = 0; n < size; ++n )
        if ( fabs( (Float64) back[ n ] - (Float64) x[ n ] ) > worst ) worst = fabs( (Float64) back[ n ] - (Float64) x[ n ] );
    ok = ok && worst / largest <= roundTripTolerance;

    GUCEF::vector< T > inPlace( spectrumLength, (T) 0 );
    for ( UInt32 n = 0; n < size; ++n )
        inPlace[ n ] = x[ n ];
    ok = ok && ForwardRealtime( fft, &inPlace[ 0 ], &inPlace[ 0 ] );
    for ( size_t i = 0; i < spectrumLength; ++i )
        ok = ok && inPlace[ i ] == spectrum[ i ];
    ok = ok && InverseRealtime( fft, &inPlace[ 0 ], &inPlace[ 0 ] );
    for ( UInt32 n = 0; n < size; ++n )
        ok = ok && inPlace[ n ] == back[ n ];

    // the definition, where it is affordable
    if ( size <= 1024 )
    {
        TDoubles xd( size );
        for ( UInt32 n = 0; n < size; ++n )
            xd[ n ] = (Float64) x[ n ];
        ok = ok && LargestError( spectrum, DirectRealDft( xd ), spectrumLength ) <= roundTripTolerance * 5.0;
    }
    return ok;
}

static void
PerformRealPropertyTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Fft: real transform properties for sizes 16 to 2^16 in Float64 (round trip 1e-14) and Float32 (1e-6)" )
    {
        for ( UInt32 size = 16; size <= 65536; size *= 2 )
        {
            const bool ok64 = CheckRealProperties< Float64 >( size, 1e-14, 1e-12, 1e-12 );
            const bool ok32 = CheckRealProperties< Float32 >( size, 1e-6, 1e-5, 2e-6 );
            if ( !ok64 || !ok32 )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Fft: real properties failed for size " << size << ( ok64 ? "" : " in Float64" ) << ( ok32 ? "" : " in Float32" ) );
            ASSERT_TRUE( ok64 );
            ASSERT_TRUE( ok32 );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fft: the extended range holds, 2^20 round trips and sizes outside 16 to 2^20 or not a power of two are refused" )
    {
        MATH::CRealFft fft;
        ASSERT_TRUE( 0 == fft.GetSize() && 0 == fft.GetBinCount() && 0 == fft.GetSpectrumLength() );
        ASSERT_FALSE( fft.Prepare( 8 ) );
        ASSERT_FALSE( fft.Prepare( 0 ) );
        ASSERT_FALSE( fft.Prepare( 24 ) );
        ASSERT_FALSE( fft.Prepare( 2097152 ) );
        ASSERT_TRUE( 0 == fft.GetSize() );
        ASSERT_TRUE( fft.Prepare( 16 ) && 16 == fft.GetSize() && 9 == fft.GetBinCount() && 18 == fft.GetSpectrumLength() );
        ASSERT_FALSE( fft.Prepare( 100 ) );
        ASSERT_TRUE( 16 == fft.GetSize() );                 // a refused size leaves the object as it was

        ASSERT_TRUE( fft.Prepare( 1048576, true, false ) );
        GUCEF::vector< Float32 > x( 1048576 );
        GUCEF::vector< Float32 > spectrum( 1048578 );
        GUCEF::vector< Float32 > back( 1048576 );
        MATH::CPcg32 random( 5, 5 );
        for ( size_t n = 0; n < x.size(); ++n )
            x[ n ] = (Float32) NextValue( random );
        ASSERT_TRUE( fft.Forward( &x[ 0 ], &spectrum[ 0 ] ) && fft.Inverse( &spectrum[ 0 ], &back[ 0 ] ) );
        Float64 worst = 0.0;
        for ( size_t n = 0; n < x.size(); ++n )
            if ( fabs( (Float64) back[ n ] - x[ n ] ) > worst ) worst = fabs( (Float64) back[ n ] - x[ n ] );
        ASSERT_TRUE( worst < 2e-6 );
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fft: an unprepared path or a null pointer returns false and writes nothing" )
    {
        MATH::CRealFft fft;
        Float32 in32[ 16 ] = { 0 };
        Float32 out32[ 18 ];
        Float64 in64[ 16 ] = { 0 };
        Float64 out64[ 18 ];
        for ( int i = 0; i < 18; ++i ) { out32[ i ] = 7.0f; out64[ i ] = 7.0; }
        ASSERT_FALSE( fft.Forward( in32, out32 ) || fft.Forward( in64, out64 ) || fft.Inverse( out32, in32 ) || fft.Inverse( out64, in64 ) );

        ASSERT_TRUE( fft.Prepare( 16, true, false ) );
        ASSERT_TRUE( fft.Forward( in32, out32 ) );
        ASSERT_FALSE( fft.Forward( in64, out64 ) );
        ASSERT_TRUE( 7.0 == out64[ 0 ] && 7.0 == out64[ 17 ] );
        ASSERT_FALSE( fft.Forward( static_cast< const Float32* >( GUCEF_NULL ), out32 ) );
        ASSERT_FALSE( fft.Inverse( out32, static_cast< Float32* >( GUCEF_NULL ) ) );
        ASSERT_TRUE( fft.Prepare( 16, false, true ) );
        ASSERT_FALSE( fft.Forward( in32, out32 ) );                     // preparing again replaces the paths
        ASSERT_TRUE( fft.Forward( in64, out64 ) );

        MATH::CComplexFft complexFft;
        MATH::CTComplex< Float64 > c[ 8 ];
        ASSERT_FALSE( complexFft.Forward( c, c ) );
        ASSERT_FALSE( complexFft.Prepare( 6 ) || complexFft.Prepare( 1 ) || complexFft.Prepare( 0 ) );
        ASSERT_TRUE( complexFft.Prepare( 8, false, true ) && 8 == complexFft.GetSize() );
        ASSERT_TRUE( complexFft.Forward( c, c ) );
        MATH::CTComplex< Float32 > c32[ 8 ];
        ASSERT_FALSE( complexFft.Forward( c32, c32 ) || complexFft.Inverse( c32, c32 ) );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

static void
PerformAllocationTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Fft: Forward and Inverse do not allocate (real and complex, both precisions), and the counter does count" )
    {
        MATH::CRealFft real;
        MATH::CComplexFft complex;
        ASSERT_TRUE( real.Prepare( 1024 ) && complex.Prepare( 512 ) );
        GUCEF::vector< Float32 > in32( 1026, 0.25f ), out32( 1026 );
        GUCEF::vector< Float64 > in64( 1026, 0.25 ), out64( 1026 );
        GUCEF::vector< TComplex32 > c32( 512, TComplex32( 1.0f, 2.0f ) );
        GUCEF::vector< TComplex64 > c64( 512, TComplex64( 1.0, 2.0 ) );

        StartCounting();
        bool ok = ForwardRealtime( real, &in32[ 0 ], &out32[ 0 ] ) && InverseRealtime( real, &out32[ 0 ], &in32[ 0 ] );
        ok = ok && ForwardRealtime( real, &in64[ 0 ], &out64[ 0 ] ) && InverseRealtime( real, &out64[ 0 ], &in64[ 0 ] );
        ok = ok && ForwardRealtime( complex, &c32[ 0 ], &c32[ 0 ] ) && InverseRealtime( complex, &c32[ 0 ], &c32[ 0 ] );
        ok = ok && ForwardRealtime( complex, &c64[ 0 ], &c64[ 0 ] ) && InverseRealtime( complex, &c64[ 0 ], &c64[ 0 ] );
        const UInt32 allocations = StopCounting();
        ASSERT_TRUE( ok );
        ASSERT_TRUE( 0 == allocations );

        // A direct call of the allocation function: unlike a new-expression the compiler may not elide it (C++14 allows
        // removing an unused new-expression, which clang does in optimised builds)
        StartCounting();
        void* probe = ::operator new( 16 );
        const UInt32 counted = StopCounting();
        ::operator delete( probe );
        ASSERT_TRUE( 1 == counted );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

template< typename T >
static bool
CheckComplexFft( UInt32 size, Float64 tolerance )
{
    typedef MATH::CTComplex< T > TC;
    MATH::CComplexFft fft;
    if ( !fft.Prepare( size ) )
        return false;
    MATH::CPcg32 random( 900 + size, 6 );
    GUCEF::vector< TC > x( size ), spectrum( size ), back( size );
    for ( UInt32 n = 0; n < size; ++n )
        x[ n ] = TC( (T) NextValue( random ), (T) NextValue( random ) );

    bool ok = ForwardRealtime( fft, &x[ 0 ], &spectrum[ 0 ] );
    // the definition in Float64: X[k] = sum x[n] exp( -2 pi i k n / N )
    Float64 largest = 0.0;
    Float64 worst = 0.0;
    for ( UInt32 k = 0; k < size; ++k )
    {
        Float64 re = 0.0, im = 0.0;
        for ( UInt32 n = 0; n < size; ++n )
        {
            const Float64 angle = -2.0 * Pi * (Float64) ( ( (UInt64) k * n ) % size ) / (Float64) size;
            re += (Float64) x[ n ].re * cos( angle ) - (Float64) x[ n ].im * sin( angle );
            im += (Float64) x[ n ].re * sin( angle ) + (Float64) x[ n ].im * cos( angle );
        }
        const Float64 e = sqrt( ( re - spectrum[ k ].re ) * ( re - spectrum[ k ].re ) + ( im - spectrum[ k ].im ) * ( im - spectrum[ k ].im ) );
        if ( e > worst ) worst = e;
        if ( sqrt( re * re + im * im ) > largest ) largest = sqrt( re * re + im * im );
    }
    ok = ok && worst <= tolerance * largest;

    ok = ok && InverseRealtime( fft, &spectrum[ 0 ], &back[ 0 ] );
    Float64 roundTrip = 0.0;
    for ( UInt32 n = 0; n < size; ++n )
    {
        const Float64 e = fabs( (Float64) back[ n ].re - x[ n ].re ) + fabs( (Float64) back[ n ].im - x[ n ].im );
        if ( e > roundTrip ) roundTrip = e;
    }
    ok = ok && roundTrip <= tolerance * 4.0;

    // in place gives the same bits
    GUCEF::vector< TC > inPlace = x;
    ok = ok && ForwardRealtime( fft, &inPlace[ 0 ], &inPlace[ 0 ] );
    for ( UInt32 n = 0; n < size; ++n )
        ok = ok && inPlace[ n ] == spectrum[ n ];
    ok = ok && InverseRealtime( fft, &inPlace[ 0 ], &inPlace[ 0 ] );
    for ( UInt32 n = 0; n < size; ++n )
        ok = ok && inPlace[ n ] == back[ n ];
    return ok;
}

static void
PerformComplexFftTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_TESTCASE( "Fft: CComplexFft equals the direct DFT for every size 2 to 256 within 1e-12 (Float64), round trips and works in place" )
    {
        for ( UInt32 size = 2; size <= 256; size *= 2 )
        {
            const bool ok = CheckComplexFft< Float64 >( size, 1e-12 );
            if ( !ok )
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Fft: complex transform failed for size " << size );
            ASSERT_TRUE( ok );
        }
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fft: CComplexFft in Float32 within 2e-5, impulse and shift properties, size 2^16 round trip" )
    {
        for ( UInt32 size = 2; size <= 256; size *= 2 )
            ASSERT_TRUE( ( CheckComplexFft< Float32 >( size, 2e-5 ) ) );

        MATH::CComplexFft fft;
        ASSERT_TRUE( fft.Prepare( 64 ) );
        GUCEF::vector< TComplex64 > x( 64, TComplex64() ), spectrum( 64 );
        x[ 3 ] = TComplex64( 1.0, 0.0 );
        ASSERT_TRUE( fft.Forward( &x[ 0 ], &spectrum[ 0 ] ) );
        bool ok = true;
        for ( UInt32 k = 0; k < 64; ++k )
        {
            const Float64 angle = -2.0 * Pi * (Float64) ( 3 * k % 64 ) / 64.0;       // a shifted impulse is a pure phase ramp
            ok = ok && fabs( spectrum[ k ].re - cos( angle ) ) < 1e-14 && fabs( spectrum[ k ].im - sin( angle ) ) < 1e-14;
        }
        ASSERT_TRUE( ok );

        ASSERT_TRUE( fft.Prepare( 65536 ) );
        MATH::CPcg32 random( 8, 8 );
        GUCEF::vector< TComplex64 > big( 65536 ), bigSpectrum( 65536 ), bigBack( 65536 );
        for ( size_t n = 0; n < big.size(); ++n )
            big[ n ] = TComplex64( NextValue( random ), NextValue( random ) );
        ASSERT_TRUE( fft.Forward( &big[ 0 ], &bigSpectrum[ 0 ] ) && fft.Inverse( &bigSpectrum[ 0 ], &bigBack[ 0 ] ) );
        Float64 worst = 0.0;
        for ( size_t n = 0; n < big.size(); ++n )
            if ( ( big[ n ] - bigBack[ n ] ).GetMagnitude() > worst ) worst = ( big[ n ] - bigBack[ n ] ).GetMagnitude();
        ASSERT_TRUE( worst < 1e-13 );
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/

void
PerformFftTests( void )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "COMMENCING Fft TESTS" );

    PerformIdentityTests();
    PerformRealPropertyTests();
    PerformAllocationTests();
    PerformComplexFftTests();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Fft TESTS COMPLETED" );
}

/*-------------------------------------------------------------------------*/
