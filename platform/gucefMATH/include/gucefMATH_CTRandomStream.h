/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CTRANDOMSTREAM_H
#define GUCEF_MATH_CTRANDOMSTREAM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <iterator>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

#ifndef GUCEF_MATH_CTRADIAN_H
#include "gucefMATH_CTRadian.h"
#define GUCEF_MATH_CTRADIAN_H
#endif /* GUCEF_MATH_CTRADIAN_H ? */

#ifndef GUCEF_MATH_QUATERNIONS_H
#include "gucefMATH_quaternions.h"
#define GUCEF_MATH_QUATERNIONS_H
#endif /* GUCEF_MATH_QUATERNIONS_H ? */

#ifndef GUCEF_MATH_SAMPLEWARPING_H
#include "gucefMATH_SampleWarping.h"
#define GUCEF_MATH_SAMPLEWARPING_H
#endif /* GUCEF_MATH_SAMPLEWARPING_H ? */

#ifndef GUCEF_MATH_CPCG32_H
#include "gucefMATH_CPcg32.h"
#define GUCEF_MATH_CPCG32_H
#endif /* GUCEF_MATH_CPCG32_H ? */

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#include "gucefMATH_CXoshiro256StarStar.h"
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H
#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */

#ifndef GUCEF_MATH_CSPLITMIX64_H
#include "gucefMATH_CSplitMix64.h"
#define GUCEF_MATH_CSPLITMIX64_H
#endif /* GUCEF_MATH_CSPLITMIX64_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Distributions on top of a bit generator (section 4.9 of the design). Distinct from CORE::CRandom, which stays the
 *  entropy source: a random stream is a value type that owns its generator (CPcg32, CXoshiro256StarStar or
 *  CSplitMix64; any class with NextUInt32() and NextUInt64()), has no global state, copies by value (a copy continues
 *  with the same numbers: save and replay) and is used by one thread at a time. Realtime-safe: nothing allocates.
 *
 *  - Integer and uniform outputs ( NextUInt32/64, NextFloat32/64, Range of integers, Below, Bool, Shuffle ) are bit
 *    identical on all platforms. Outputs that use log, sin, cos, sqrt of a non trivial argument or pow ( Normal,
 *    Exponential, the circle, sphere, cone and rotation functions ) are not, because the C library functions differ.
 *  - Float kinds are Float32 and Float64. The functions without arguments that return vectors exist twice: the plain
 *    name produces Float32 ( rng.InUnitSphere() ), the name with the suffix As takes the kind as template argument
 *    ( rng.InUnitSphereAs< Float64 >() ; in a template write rng.template InUnitSphereAs< Float64 >() ). The others
 *    deduce the kind from their arguments.
 *  - Rejection loops ( Normal, Range of integers ) are capped at 64 rounds so that the time is bounded; the cap is
 *    reached with a probability below 1e-40.
 *  - The predefined kinds ( Float32, Float64 ) of the three generators are exported instantiations compiled without
 *    floating-point contraction.
 */
template < typename TGenerator >
class GUCEF_MATH_TEMPLATE_PUBLIC CTRandomStream
{
    public:

    typedef TGenerator TGeneratorType;

    /** A stream over a default constructed generator */
    CTRandomStream( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_generator()
    {
    }

    /** A stream that continues the given generator ( copied ) */
    explicit CTRandomStream( const TGenerator& generator ) GUCEF_MATH_REALTIME_CONTEXT
        : m_generator( generator )
    {
    }

    GUCEF_MATH_FORCE_INLINE TGenerator& GetGenerator( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_generator; }
    GUCEF_MATH_FORCE_INLINE const TGenerator& GetGenerator( void ) const GUCEF_MATH_REALTIME_CONTEXT { return m_generator; }

    /*---------------------------------------------------------------------*/
    /*  Bits and uniform numbers                                            */
    /*---------------------------------------------------------------------*/

    /** 32 random bits */
    GUCEF_MATH_FORCE_INLINE UInt32 NextUInt32( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_generator.NextUInt32(); }

    /** 64 random bits */
    GUCEF_MATH_FORCE_INLINE UInt64 NextUInt64( void ) GUCEF_MATH_REALTIME_CONTEXT { return m_generator.NextUInt64(); }

    /** Uniform in [0, 1): ( bits >> 8 ) * 2^-24, so 1 is never returned and the largest value is 1 - 2^-24 */
    GUCEF_MATH_FORCE_INLINE Float32 NextFloat32( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return (Float32) ( m_generator.NextUInt32() >> 8 ) * 5.9604644775390625e-8f;
    }

    /** Uniform in [0, 1) with 53 random bits: 1 is never returned */
    GUCEF_MATH_FORCE_INLINE Float64 NextFloat64( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return (Float64) ( m_generator.NextUInt64() >> 11 ) * 1.1102230246251565404236316680908203125e-16;
    }

    /*---------------------------------------------------------------------*/
    /*  Ranges                                                              */
    /*---------------------------------------------------------------------*/

    /** Uniform in [0, bound) without modulo bias (Lemire's multiply-shift with rejection); 0 for a bound of 0 */
    UInt32 Below( UInt32 bound ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( bound == 0 )
            return 0;
        UInt32 x = m_generator.NextUInt32();
        UInt64 product = (UInt64) x * bound;
        UInt32 low = (UInt32) product;
        if ( low < bound )
        {
            const UInt32 threshold = ( 0U - bound ) % bound;
            for ( UInt32 round=0; low < threshold && round < 64; ++round )
            {
                x = m_generator.NextUInt32();
                product = (UInt64) x * bound;
                low = (UInt32) product;
            }
        }
        return (UInt32) ( product >> 32 );
    }

    /** Uniform integer in [min, max], both included, without modulo bias; min and max are swapped when reversed */
    Int32 Range( Int32 min, Int32 max ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( max < min )
        {
            const Int32 swapped = min;
            min = max;
            max = swapped;
        }
        const UInt32 span = (UInt32) max - (UInt32) min + 1U;      // 0 when the range covers all of Int32
        if ( span == 0 )
            return (Int32) m_generator.NextUInt32();
        return (Int32) ( (UInt32) min + Below( span ) );
    }

    /** Uniform in [min, max) for the float kinds: min + ( max - min ) u */
    template < typename T >
    T Range( const T& min, const T& max ) GUCEF_MATH_REALTIME_CONTEXT;

    /** true with the given probability ( p <= 0 never, p >= 1 always ) */
    template < typename T >
    GUCEF_MATH_FORCE_INLINE bool Bool( const T& probability ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return DrawUnit( (const T*) GUCEF_NULL ) < probability;
    }

    /*---------------------------------------------------------------------*/
    /*  Scalar distributions                                                */
    /*---------------------------------------------------------------------*/

    /** Normal distribution (Marsaglia polar method): mean + standardDeviation * z; 'mean' if the rejection cap is hit */
    template < typename T >
    T Normal( const T& mean, const T& standardDeviation ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Exponential distribution with the given rate ( mean 1 / rate ): -ln( 1 - u ) / rate; 0 for a rate <= 0 */
    template < typename T >
    T Exponential( const T& rate ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Points and directions                                               */
    /*---------------------------------------------------------------------*/

    /** Uniform in the unit disk ( Float32 ) */
    GUCEF_MATH_FORCE_INLINE TFloat32Vector2D InUnitCircle( void ) GUCEF_MATH_REALTIME_CONTEXT { return InUnitCircleAs< Float32 >(); }
    template < typename T > CTVector2D< T > InUnitCircleAs( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Uniform on the unit circle ( Float32 ) */
    GUCEF_MATH_FORCE_INLINE TFloat32Vector2D OnUnitCircle( void ) GUCEF_MATH_REALTIME_CONTEXT { return OnUnitCircleAs< Float32 >(); }
    template < typename T > CTVector2D< T > OnUnitCircleAs( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Uniform in the unit ball: radius <= 1 ( Float32 ) */
    GUCEF_MATH_FORCE_INLINE TFloat32Vector3D InUnitSphere( void ) GUCEF_MATH_REALTIME_CONTEXT { return InUnitSphereAs< Float32 >(); }
    template < typename T > CTVector3D< T > InUnitSphereAs( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Uniform on the unit sphere: z uniform in [-1, 1] ( Float32 ) */
    GUCEF_MATH_FORCE_INLINE TFloat32Vector3D OnUnitSphere( void ) GUCEF_MATH_REALTIME_CONTEXT { return OnUnitSphereAs< Float32 >(); }
    template < typename T > CTVector3D< T > OnUnitSphereAs( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /** A unit vector uniform in the cone around 'direction' with the given half angle ( 0 = the direction itself, pi = the sphere ) */
    template < typename T >
    CTVector3D< T > InCone( const CTDirectionVector3D< T >& direction, const CTRadian< T >& halfAngle ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Uniform in the axis aligned box between the corners 'min' and 'max' ( each component as Range ) */
    template < typename T >
    CTVector3D< T > InBox( const CTVector3D< T >& min, const CTVector3D< T >& max ) GUCEF_MATH_REALTIME_CONTEXT;

    /** A unit vector uniform on the hemisphere around 'normal' */
    template < typename T >
    CTVector3D< T > OnHemisphere( const CTDirectionVector3D< T >& normal ) GUCEF_MATH_REALTIME_CONTEXT;

    /** A unit vector on the hemisphere around 'normal' with density cos( theta ) / pi; the mean of cos( theta ) is 2 / 3 */
    template < typename T >
    CTVector3D< T > CosineWeightedHemisphere( const CTDirectionVector3D< T >& normal ) GUCEF_MATH_REALTIME_CONTEXT;

    /** A uniformly distributed unit quaternion ( Shoemake 1992 ); Float32 */
    GUCEF_MATH_FORCE_INLINE CTQuaternion< Float32 > RandomRotation( void ) GUCEF_MATH_REALTIME_CONTEXT { return RandomRotationAs< Float32 >(); }
    template < typename T > CTQuaternion< T > RandomRotationAs( void ) GUCEF_MATH_REALTIME_CONTEXT;

    /*---------------------------------------------------------------------*/
    /*  Choice and permutation                                              */
    /*---------------------------------------------------------------------*/

    /**
     *  The index of a random element, chosen with probability proportional to its weight (linear scan; use CTAliasTable
     *  for many draws from the same weights). Negative and NaN weights count as 0; 0 when all weights are 0 or count is 0.
     */
    template < typename T >
    UInt32 Pick( const T* weights, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT;

    /** Uniformly random permutation of [first, last) (Fisher-Yates); realtime-safe when element assignment is */
    template < typename TIterator >
    void Shuffle( TIterator first, TIterator last ) GUCEF_MATH_REALTIME_CONTEXT
    {
        typedef typename std::iterator_traits< TIterator >::value_type TElement;
        typedef typename std::iterator_traits< TIterator >::difference_type TDifference;
        const TDifference count = last - first;
        for ( TDifference i = count - 1; i > 0; --i )
        {
            const TDifference j = (TDifference) Below( (UInt32) ( i + 1 ) );
            TElement temp( *( first + i ) );
            *( first + i ) = *( first + j );
            *( first + j ) = temp;
        }
    }

    /** Streams with the same generator state are equal */
    friend bool operator==( const CTRandomStream& a, const CTRandomStream& b ) { return a.m_generator == b.m_generator; }
    friend bool operator!=( const CTRandomStream& a, const CTRandomStream& b ) { return !( a.m_generator == b.m_generator ); }

    private:

    GUCEF_MATH_FORCE_INLINE Float32 DrawUnit( const Float32* ) GUCEF_MATH_REALTIME_CONTEXT { return NextFloat32(); }
    GUCEF_MATH_FORCE_INLINE Float64 DrawUnit( const Float64* ) GUCEF_MATH_REALTIME_CONTEXT { return NextFloat64(); }

    TGenerator m_generator;
};

/*-------------------------------------------------------------------------*/

/** The default random stream: PCG-XSH-RR 64/32 */
typedef CTRandomStream< CPcg32 > CRandomStream;

/** Random stream over xoshiro256** (64 bit output, jumps for parallel streams) */
typedef CTRandomStream< CXoshiro256StarStar > CXoshiroRandomStream;

/** Random stream over SplitMix64 */
typedef CTRandomStream< CSplitMix64 > CSplitMixRandomStream;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T ) GUCEF_MATH_STATIC_ASSERT( CTNumericTraits< T >::IsFloatingPoint, Random_stream_distributions_require_a_floating_point_kind )

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTRandomStream< TGenerator >::Range( const T& min, const T& max ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    return min + ( max - min ) * DrawUnit( (const T*) GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTRandomStream< TGenerator >::Normal( const T& mean, const T& standardDeviation ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    T u = T( 0 );
    T s = T( 0 );
    for ( UInt32 round=0; round < 64; ++round )
    {
        u = T( 2 ) * DrawUnit( (const T*) GUCEF_NULL ) - T( 1 );
        const T v = T( 2 ) * DrawUnit( (const T*) GUCEF_NULL ) - T( 1 );
        s = u * u + v * v;
        if ( s < T( 1 ) && s != T( 0 ) )
        {
            const T factor = (T) sqrt( -2.0 * log( (Float64) s ) / (Float64) s );
            return mean + standardDeviation * u * factor;
        }
    }
    return mean;
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE T
CTRandomStream< TGenerator >::Exponential( const T& rate ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    if ( !( rate > T( 0 ) ) )
        return T( 0 );
    const Float64 u = (Float64) DrawUnit( (const T*) GUCEF_NULL );
    return (T) ( -log( 1.0 - u ) / (Float64) rate );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTRandomStream< TGenerator >::InUnitCircleAs( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const T u = DrawUnit( (const T*) GUCEF_NULL );
    const T v = DrawUnit( (const T*) GUCEF_NULL );
    return UniformDisk< T >( u, v );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector2D< T >
CTRandomStream< TGenerator >::OnUnitCircleAs( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    return UniformCircle< T >( DrawUnit( (const T*) GUCEF_NULL ) );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTRandomStream< TGenerator >::OnUnitSphereAs( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const T u = DrawUnit( (const T*) GUCEF_NULL );
    const T v = DrawUnit( (const T*) GUCEF_NULL );
    return UniformSphere< T >( u, v );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTRandomStream< TGenerator >::InUnitSphereAs( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const CTVector3D< T > direction = OnUnitSphereAs< T >();
    // the radius of a uniform point in a ball has the cumulative distribution r^3
    T radius = (T) pow( (Float64) DrawUnit( (const T*) GUCEF_NULL ), 1.0 / 3.0 );
    if ( radius > T( 1 ) )
        radius = T( 1 );
    return CTVector3D< T >( direction.x * radius, direction.y * radius, direction.z * radius );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTRandomStream< TGenerator >::InCone( const CTDirectionVector3D< T >& direction, const CTRadian< T >& halfAngle ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const T u = DrawUnit( (const T*) GUCEF_NULL );
    const T v = DrawUnit( (const T*) GUCEF_NULL );
    const T cosMax = (T) cos( (Float64) halfAngle.GetValue() );
    return OrientAroundNormal< T >( UniformCone< T >( u, v, cosMax ), direction.AsVector() );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTRandomStream< TGenerator >::InBox( const CTVector3D< T >& min, const CTVector3D< T >& max ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const T x = min.x + ( max.x - min.x ) * DrawUnit( (const T*) GUCEF_NULL );
    const T y = min.y + ( max.y - min.y ) * DrawUnit( (const T*) GUCEF_NULL );
    const T z = min.z + ( max.z - min.z ) * DrawUnit( (const T*) GUCEF_NULL );
    return CTVector3D< T >( x, y, z );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTRandomStream< TGenerator >::OnHemisphere( const CTDirectionVector3D< T >& normal ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const T u = DrawUnit( (const T*) GUCEF_NULL );
    const T v = DrawUnit( (const T*) GUCEF_NULL );
    return OrientAroundNormal< T >( UniformHemisphere< T >( u, v ), normal.AsVector() );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTVector3D< T >
CTRandomStream< TGenerator >::CosineWeightedHemisphere( const CTDirectionVector3D< T >& normal ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const T u = DrawUnit( (const T*) GUCEF_NULL );
    const T v = DrawUnit( (const T*) GUCEF_NULL );
    return OrientAroundNormal< T >( CosineHemisphere< T >( u, v ), normal.AsVector() );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE CTQuaternion< T >
CTRandomStream< TGenerator >::RandomRotationAs( void ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    const Float64 u1 = (Float64) DrawUnit( (const T*) GUCEF_NULL );
    const Float64 u2 = 6.28318530717958647692528676655900577 * (Float64) DrawUnit( (const T*) GUCEF_NULL );
    const Float64 u3 = 6.28318530717958647692528676655900577 * (Float64) DrawUnit( (const T*) GUCEF_NULL );
    const Float64 a = sqrt( 1.0 - u1 );
    const Float64 b = sqrt( u1 );
    return CTQuaternion< T >::FromXYZW( (T) ( a * sin( u2 ) ), (T) ( a * cos( u2 ) ), (T) ( b * sin( u3 ) ), (T) ( b * cos( u3 ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename TGenerator >
template < typename T >
GUCEF_MATH_PRECISE_INLINE UInt32
CTRandomStream< TGenerator >::Pick( const T* weights, UInt32 count ) GUCEF_MATH_REALTIME_CONTEXT
{
    GUCEF_MATH_RANDOMSTREAM_FLOAT_ONLY( T );
    Float64 total = 0.0;
    UInt32 lastPositive = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( weights[ i ] > T( 0 ) )
        {
            total = total + (Float64) weights[ i ];
            lastPositive = i;
        }
    }
    if ( !( total > 0.0 ) )
        return 0;
    const Float64 target = (Float64) DrawUnit( (const T*) GUCEF_NULL ) * total;
    Float64 accumulated = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( weights[ i ] > T( 0 ) )
        {
            accumulated = accumulated + (Float64) weights[ i ];
            if ( target < accumulated )
                return i;
        }
    }
    return lastPositive;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The precision-sensitive member templates for the predefined kinds, per generator: X-macro over the generator and
 *  the kind ( gucefMATH_RandomInstantiations.cpp instantiates them ). The members that only compare or shift are inline.
 */
#define GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, generator, kind, vector2, vector3, quaternion )                                                     \
    prefix kind CTRandomStream< generator >::Range< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                     \
    prefix kind CTRandomStream< generator >::Normal< kind >( const kind&, const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                    \
    prefix kind CTRandomStream< generator >::Exponential< kind >( const kind& ) GUCEF_MATH_REALTIME_CONTEXT;                                            \
    prefix vector2 CTRandomStream< generator >::InUnitCircleAs< kind >( void ) GUCEF_MATH_REALTIME_CONTEXT;                                               \
    prefix vector2 CTRandomStream< generator >::OnUnitCircleAs< kind >( void ) GUCEF_MATH_REALTIME_CONTEXT;                                               \
    prefix vector3 CTRandomStream< generator >::InUnitSphereAs< kind >( void ) GUCEF_MATH_REALTIME_CONTEXT;                                              \
    prefix vector3 CTRandomStream< generator >::OnUnitSphereAs< kind >( void ) GUCEF_MATH_REALTIME_CONTEXT;                                              \
    prefix vector3 CTRandomStream< generator >::InCone< kind >( const CTDirectionVector3D< kind >&, const CTRadian< kind >& ) GUCEF_MATH_REALTIME_CONTEXT; \
    prefix vector3 CTRandomStream< generator >::InBox< kind >( const vector3&, const vector3& ) GUCEF_MATH_REALTIME_CONTEXT;                            \
    prefix vector3 CTRandomStream< generator >::OnHemisphere< kind >( const CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;                 \
    prefix vector3 CTRandomStream< generator >::CosineWeightedHemisphere< kind >( const CTDirectionVector3D< kind >& ) GUCEF_MATH_REALTIME_CONTEXT;    \
    prefix quaternion CTRandomStream< generator >::RandomRotationAs< kind >( void ) GUCEF_MATH_REALTIME_CONTEXT;                                          \
    prefix UInt32 CTRandomStream< generator >::Pick< kind >( const kind*, UInt32 ) GUCEF_MATH_REALTIME_CONTEXT;

#define GUCEF_MATH_CTRANDOMSTREAM_PREDEFINED( prefix )                                                                                                  \
    GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, CPcg32, Float32, TFloat32Vector2D, TFloat32Vector3D, TFloat32Quaternion )                                 \
    GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, CPcg32, Float64, TFloat64Vector2D, TFloat64Vector3D, TFloat64Quaternion )                                 \
    GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, CXoshiro256StarStar, Float32, TFloat32Vector2D, TFloat32Vector3D, TFloat32Quaternion )                    \
    GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, CXoshiro256StarStar, Float64, TFloat64Vector2D, TFloat64Vector3D, TFloat64Quaternion )                    \
    GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, CSplitMix64, Float32, TFloat32Vector2D, TFloat32Vector3D, TFloat32Quaternion )                            \
    GUCEF_MATH_CTRANDOMSTREAM_MEMBERS( prefix, CSplitMix64, Float64, TFloat64Vector2D, TFloat64Vector3D, TFloat64Quaternion )

#ifdef GUCEF_MATH_EXTERN_TEMPLATES_ENABLED
GUCEF_MATH_CTRANDOMSTREAM_PREDEFINED( GUCEF_MATH_EXTERN_TEMPLATE )
#endif /* GUCEF_MATH_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CTRANDOMSTREAM_H ? */
