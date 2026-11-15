/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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
#include <string.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_MATH_FRACTIONTYPES_H
#include "gucefMATH_fractiontypes.h"
#define GUCEF_MATH_FRACTIONTYPES_H
#endif /* GUCEF_MATH_FRACTIONTYPES_H ? */

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

#include "TestConvexReference.h"
#include "TestPredicates.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace QueryReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef TMathBigFraction F;

/** The exact value of a Float64 as a fraction */
static F
Exact( Float64 value )
{
    F result( 0, 1 );
    CTNumericTraits< F >::FromFloat64( value, CORE::ROUNDINGMODE_UNNECESSARY, result );
    return result;
}

/** The exact value of an integer as a fraction */
static F
Exact( Int64 value )
{
    CORE::CBigInteger n;
    CORE::CBigInteger::FromInt64( value, n );
    return F( n, CORE::CBigInteger::One() );
}

/** The exact value of a fixed-point number: raw / 2^F */
template < typename TStorage, UInt32 FractionBits >
static F
Exact( const CTFixedPoint< TStorage, FractionBits >& value )
{
    CORE::CBigInteger n;
    CORE::CBigInteger::FromInt64( (Int64) value.GetRaw(), n );
    CORE::CBigInteger d = CORE::CBigInteger::One();
    d <<= FractionBits;
    return F( n, d );
}

static F Exact( Int32 value ) { return Exact( (Int64) value ); }
static F Exact( Float32 value ) { return Exact( (Float64) value ); }

static Int32
SignOf( const F& value )
{
    return CTNumericTraits< F >::IsNegative( value ) ? -1 : ( value == F( 0, 1 ) ? 0 : 1 );
}

/*-------------------------------------------------------------------------*/

/*
 *  The reference determinants on exact fractions, written for the test with the explicit Leibniz and cofactor forms
 *  (independent of the evaluation of the predicates)
 */
template < typename T >
static Int32
ReferenceOrient2D( const CTPoint2D< T >& a, const CTPoint2D< T >& b, const CTPoint2D< T >& c )
{
    const F ax = Exact( a.x ), ay = Exact( a.y ), bx = Exact( b.x ), by = Exact( b.y ), cx = Exact( c.x ), cy = Exact( c.y );
    /* det [ [ ax, ay, 1 ], [ bx, by, 1 ], [ cx, cy, 1 ] ] */
    return SignOf( ax * by - ax * cy - ay * bx + ay * cx + bx * cy - by * cx );
}

template < typename T >
static Int32
ReferenceOrient3D( const CTPoint3D< T >& a, const CTPoint3D< T >& b, const CTPoint3D< T >& c, const CTPoint3D< T >& d )
{
    /* det [ b - a; c - a; d - a ] by the rule of Sarrus */
    const F m[ 3 ][ 3 ] = { { Exact( b.x ) - Exact( a.x ), Exact( b.y ) - Exact( a.y ), Exact( b.z ) - Exact( a.z ) },
                            { Exact( c.x ) - Exact( a.x ), Exact( c.y ) - Exact( a.y ), Exact( c.z ) - Exact( a.z ) },
                            { Exact( d.x ) - Exact( a.x ), Exact( d.y ) - Exact( a.y ), Exact( d.z ) - Exact( a.z ) } };
    return SignOf( m[ 0 ][ 0 ] * m[ 1 ][ 1 ] * m[ 2 ][ 2 ] + m[ 0 ][ 1 ] * m[ 1 ][ 2 ] * m[ 2 ][ 0 ] + m[ 0 ][ 2 ] * m[ 1 ][ 0 ] * m[ 2 ][ 1 ]
                 - m[ 0 ][ 2 ] * m[ 1 ][ 1 ] * m[ 2 ][ 0 ] - m[ 0 ][ 0 ] * m[ 1 ][ 2 ] * m[ 2 ][ 1 ] - m[ 0 ][ 1 ] * m[ 1 ][ 0 ] * m[ 2 ][ 2 ] );
}

/** The determinant of a 3x3 fraction matrix (Sarrus) */
static F
Det3( const F ( &m )[ 3 ][ 3 ] )
{
    return m[ 0 ][ 0 ] * m[ 1 ][ 1 ] * m[ 2 ][ 2 ] + m[ 0 ][ 1 ] * m[ 1 ][ 2 ] * m[ 2 ][ 0 ] + m[ 0 ][ 2 ] * m[ 1 ][ 0 ] * m[ 2 ][ 1 ]
         - m[ 0 ][ 2 ] * m[ 1 ][ 1 ] * m[ 2 ][ 0 ] - m[ 0 ][ 0 ] * m[ 1 ][ 2 ] * m[ 2 ][ 1 ] - m[ 0 ][ 1 ] * m[ 1 ][ 0 ] * m[ 2 ][ 2 ];
}

template < typename T >
static Int32
ReferenceInCircle( const CTPoint2D< T >& a, const CTPoint2D< T >& b, const CTPoint2D< T >& c, const CTPoint2D< T >& d )
{
    /* rows ( p - d, |p - d|^2 ): positive when d is inside the circle of counter-clockwise a, b, c */
    const CTPoint2D< T >* p[ 3 ] = { &a, &b, &c };
    F m[ 3 ][ 3 ];
    for ( UInt32 r=0; r<3; ++r )
    {
        m[ r ][ 0 ] = Exact( p[ r ]->x ) - Exact( d.x );
        m[ r ][ 1 ] = Exact( p[ r ]->y ) - Exact( d.y );
        m[ r ][ 2 ] = m[ r ][ 0 ] * m[ r ][ 0 ] + m[ r ][ 1 ] * m[ r ][ 1 ];
    }
    return SignOf( Det3( m ) );
}

template < typename T >
static Int32
ReferenceInSphere( const CTPoint3D< T >& a, const CTPoint3D< T >& b, const CTPoint3D< T >& c, const CTPoint3D< T >& d, const CTPoint3D< T >& e )
{
    /*
     *  The lifted orientation: the sphere through a, b, c, d contains e when e lies below the paraboloid lift of the
     *  others; with the orientation of Orient3D this is det of the rows ( p - e, |p - e|^2 ) expanded along the first
     *  column, negated (the convention of CTPredicates: positive inside for Orient3D( a, b, c, d ) > 0)
     */
    const CTPoint3D< T >* p[ 4 ] = { &a, &b, &c, &d };
    F rows[ 4 ][ 4 ];
    for ( UInt32 r=0; r<4; ++r )
    {
        rows[ r ][ 0 ] = Exact( p[ r ]->x ) - Exact( e.x );
        rows[ r ][ 1 ] = Exact( p[ r ]->y ) - Exact( e.y );
        rows[ r ][ 2 ] = Exact( p[ r ]->z ) - Exact( e.z );
        rows[ r ][ 3 ] = rows[ r ][ 0 ] * rows[ r ][ 0 ] + rows[ r ][ 1 ] * rows[ r ][ 1 ] + rows[ r ][ 2 ] * rows[ r ][ 2 ];
    }
    F det( 0, 1 );
    for ( UInt32 r=0; r<4; ++r )
    {
        F minor[ 3 ][ 3 ];
        UInt32 mr = 0;
        for ( UInt32 s=0; s<4; ++s )
        {
            if ( s == r )
                continue;
            for ( UInt32 k=0; k<3; ++k )
                minor[ mr ][ k ] = rows[ s ][ k + 1 ];
            ++mr;
        }
        const F term = rows[ r ][ 0 ] * Det3( minor );
        det = 0 == r % 2 ? det + term : det - term;
    }
    return -SignOf( det );
}

/*-------------------------------------------------------------------------*/

/** The classic near-degenerate grid: exact for every point, while the naive Float64 evaluation errs on some (recorded) */
static bool
ClassicGrid( UInt32& naiveErrors )
{
    typedef CTPoint2D< Float64 > P2;
    const P2 q( 12.0, 12.0 );
    const P2 r( 24.0, 24.0 );
    UInt32 wrong = 0;
    naiveErrors = 0;
    for ( UInt32 i=0; i<256; ++i )
        for ( UInt32 j=0; j<256; ++j )
        {
            const P2 p( 0.5 + ldexp( (Float64) i, -53 ), 0.5 + ldexp( (Float64) j, -53 ) );
            const Int32 exact = ReferenceOrient2D( p, q, r );
            if ( CTPredicates< Float64 >::Orient2D( p, q, r ) != exact )
                ++wrong;
            const Float64 naive = ( p.x - r.x ) * ( q.y - r.y ) - ( p.y - r.y ) * ( q.x - r.x );
            const Int32 naiveSign = naive > 0.0 ? 1 : ( naive < 0.0 ? -1 : 0 );
            if ( naiveSign != exact )
                ++naiveErrors;
        }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: classic grid: 65536 points, " << wrong << " wrong signs from Orient2D, " << naiveErrors << " wrong from naive Float64 evaluation" );
    return 0 == wrong;
}

/*-------------------------------------------------------------------------*/

/** Cocircular and cospherical inputs (sign 0), scaled by powers of two and moved, and one-ulp perturbations against the exact reference */
static bool
CocircularCospherical( void )
{
    typedef CTPoint2D< Float64 > P2;
    typedef CTPoint3D< Float64 > P3;
    bool ok = true;
    const P2 circle[ 8 ] = { P2( 5, 0 ), P2( 3, 4 ), P2( 0, 5 ), P2( -4, 3 ), P2( -5, 0 ), P2( -3, -4 ), P2( 4, -3 ), P2( 0, -5 ) };
    const P3 sphere[ 10 ] = { P3( 1, 2, 2 ), P3( -2, 1, 2 ), P3( 2, -2, 1 ), P3( 0, 0, 3 ), P3( 3, 0, 0 ), P3( 0, -3, 0 ), P3( -1, -2, -2 ), P3( 2, 2, -1 ), P3( -2, 2, 1 ), P3( 1, -2, 2 ) };
    for ( Int32 scale=-30; scale<=30 && ok; scale+=10 )
    {
        const Float64 factor = ldexp( 1.0, scale );
        const Float64 shift = ldexp( 3.0, scale + 2 );
        for ( UInt32 i=0; i<8 && ok; ++i )
        {
            const P2 a( circle[ i ].x * factor + shift, circle[ i ].y * factor - shift );
            const P2 b( circle[ ( i + 1 ) % 8 ].x * factor + shift, circle[ ( i + 1 ) % 8 ].y * factor - shift );
            const P2 c( circle[ ( i + 3 ) % 8 ].x * factor + shift, circle[ ( i + 3 ) % 8 ].y * factor - shift );
            const P2 d( circle[ ( i + 5 ) % 8 ].x * factor + shift, circle[ ( i + 5 ) % 8 ].y * factor - shift );
            ok = 0 == CTPredicates< Float64 >::InCircle( a, b, c, d );
            /* one ulp towards and away from the centre ( shift, -shift ) along a coordinate where d is off the centre */
            const bool alongX = d.x != shift;
            const P2 inward = alongX ? P2( nextafter( d.x, shift ), d.y ) : P2( d.x, nextafter( d.y, -shift ) );
            const P2 outward = alongX ? P2( nextafter( d.x, d.x > shift ? 1.0e300 : -1.0e300 ), d.y ) : P2( d.x, nextafter( d.y, d.y > -shift ? 1.0e300 : -1.0e300 ) );
            ok = ok && CTPredicates< Float64 >::InCircle( a, b, c, inward ) == ReferenceInCircle( a, b, c, inward ) && CTPredicates< Float64 >::InCircle( a, b, c, outward ) == ReferenceInCircle( a, b, c, outward );
            ok = ok && 0 != ReferenceInCircle( a, b, c, inward );
        }
        for ( UInt32 i=0; i<10 && ok; ++i )
        {
            P3 p[ 5 ];
            for ( UInt32 k=0; k<5; ++k )
                p[ k ] = P3( sphere[ ( i + 2 * k ) % 10 ].x * factor + shift, sphere[ ( i + 2 * k ) % 10 ].y * factor, sphere[ ( i + 2 * k ) % 10 ].z * factor - shift );
            if ( 0 == ReferenceOrient3D( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ] ) )
                continue;
            ok = 0 == CTPredicates< Float64 >::InSphere( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], p[ 4 ] );
            const P3 nudged( nextafter( p[ 4 ].x, 0.0 ), p[ 4 ].y, p[ 4 ].z );
            ok = ok && CTPredicates< Float64 >::InSphere( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], nudged ) == ReferenceInSphere( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], nudged );
            ok = ok && CTPredicates< Float64 >::Orient3D( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ] ) == ReferenceOrient3D( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ] );
        }
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: cocircular or cospherical case failed at scale 2^" << scale );
    }
    /* the orientation conventions on simple cases */
    ok = ok && 1 == CTPredicates< Float64 >::Orient2D( P2( 0, 0 ), P2( 1, 0 ), P2( 0, 1 ) );
    ok = ok && 1 == CTPredicates< Float64 >::Orient3D( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 0, 1, 0 ), P3( 0, 0, 1 ) );
    ok = ok && 1 == CTPredicates< Float64 >::InCircle( P2( 0, 0 ), P2( 1, 0 ), P2( 0, 1 ), P2( 0.25, 0.25 ) );
    ok = ok && 1 == CTPredicates< Float64 >::InSphere( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 0, 1, 0 ), P3( 0, 0, 1 ), P3( 0.25, 0.25, 0.25 ) );
    ok = ok && -1 == CTPredicates< Float64 >::InSphere( P3( 0, 0, 0 ), P3( 1, 0, 0 ), P3( 0, 1, 0 ), P3( 0, 0, 1 ), P3( 2, 2, 2 ) );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Near-degenerate inputs of a kind built from integer draws: collinear, coplanar, cocircular-ish, with a one unit nudge, against the fraction reference */
template < typename T >
static bool
KindSweep( const char* name, UInt64 seed, UInt32 count, Int64 range, T ( *make )( Int64 ) )
{
    MATH::CSplitMix64 random( seed );
    UInt32 bad = 0;
    UInt32 zeros = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        Int64 v[ 15 ];
        for ( UInt32 k=0; k<15; ++k )
        {
            const UInt64 draw = random.NextUInt64();
            v[ k ] = (Int64) ( draw % (UInt64) ( 2 * range + 1 ) ) - range;
        }
        /* collinear and coplanar points: a + t d with small integer t, nudged by one unit every other time */
        const Int64 nudge = ( i % 2 ) ? 1 : 0;
        const Int64 dx = v[ 3 ] / 8;
        const Int64 dy = v[ 4 ] / 8;
        const Int64 dz = v[ 5 ] / 8;
        const Int64 ex = v[ 6 ] / 8;
        const Int64 ey = v[ 7 ] / 8;
        const Int64 ez = v[ 8 ] / 8;
        const CTPoint2D< T > a2( make( v[ 0 ] ), make( v[ 1 ] ) );
        const CTPoint2D< T > b2( make( v[ 0 ] + dx ), make( v[ 1 ] + dy ) );
        const CTPoint2D< T > c2( make( v[ 0 ] + 3 * dx ), make( v[ 1 ] + 3 * dy + nudge ) );
        const CTPoint2D< T > d2( make( v[ 9 ] ), make( v[ 10 ] ) );
        const CTPoint3D< T > a3( make( v[ 0 ] ), make( v[ 1 ] ), make( v[ 2 ] ) );
        const CTPoint3D< T > b3( make( v[ 0 ] + dx ), make( v[ 1 ] + dy ), make( v[ 2 ] + dz ) );
        const CTPoint3D< T > c3( make( v[ 0 ] + ex ), make( v[ 1 ] + ey ), make( v[ 2 ] + ez ) );
        const CTPoint3D< T > d3( make( v[ 0 ] + dx + 2 * ex ), make( v[ 1 ] + dy + 2 * ey ), make( v[ 2 ] + dz + 2 * ez + nudge ) );
        const CTPoint3D< T > e3( make( v[ 11 ] ), make( v[ 12 ] ), make( v[ 13 ] ) );
        const Int32 got[ 4 ] = { CTPredicates< T >::Orient2D( a2, b2, c2 ), CTPredicates< T >::Orient3D( a3, b3, c3, d3 ), CTPredicates< T >::InCircle( a2, b2, d2, c2 ),
                                 CTPredicates< T >::InSphere( a3, b3, c3, e3, d3 ) };
        const Int32 expected[ 4 ] = { ReferenceOrient2D( a2, b2, c2 ), ReferenceOrient3D( a3, b3, c3, d3 ), ReferenceInCircle( a2, b2, d2, c2 ), ReferenceInSphere( a3, b3, c3, e3, d3 ) };
        for ( UInt32 k=0; k<4; ++k )
        {
            zeros += 0 == expected[ k ] ? 1 : 0;
            if ( got[ k ] != expected[ k ] )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: " << name << " predicate " << k << " gave " << got[ k ] << " expected " << expected[ k ] << ", seed " << seed << " case " << i );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: " << name << " seed " << seed << ": " << 4 * count << " signs, " << zeros << " exactly zero, " << bad << " wrong" );
    return 0 == bad && zeros > count / 2;
}

static Int32 MakeInt32( Int64 v ) { return (Int32) v; }
static Int64 MakeInt64( Int64 v ) { return v; }
static TFixed32_32 MakeFixed32_32( Int64 v ) { return TFixed32_32::FromRaw( v ); }
static TFixed16_16 MakeFixed16_16( Int64 v ) { return TFixed16_16::FromRaw( (Int32) v ); }
static Float64 MakeFloat64( Int64 v ) { return ldexp( (Float64) v, -30 ); }
static Float32 MakeFloat32( Int64 v ) { return (Float32) v; }

/*-------------------------------------------------------------------------*/

/** The share of random cases that the Float64 filters decide without the exact stage */
static bool
FilterEfficiency( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 decided[ 4 ] = { 0, 0, 0, 0 };
    for ( UInt32 i=0; i<count; ++i )
    {
        Float64 c[ 15 ];
        for ( UInt32 k=0; k<15; ++k )
        {
            const Float64 value = random.Uniform( -1.0, 1.0 );
            c[ k ] = value;
        }
        Int32 sign = 0;
        const Float64 c6[ 6 ] = { c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], c[ 4 ], c[ 5 ] };
        const Float64 c8[ 8 ] = { c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], c[ 4 ], c[ 5 ], c[ 6 ], c[ 7 ] };
        const Float64 c12[ 12 ] = { c[ 0 ], c[ 1 ], c[ 2 ], c[ 3 ], c[ 4 ], c[ 5 ], c[ 6 ], c[ 7 ], c[ 8 ], c[ 9 ], c[ 10 ], c[ 11 ] };
        decided[ 0 ] += PredicateFilterOrient2D( c6, sign ) ? 1 : 0;
        decided[ 1 ] += PredicateFilterOrient3D( c12, sign ) ? 1 : 0;
        decided[ 2 ] += PredicateFilterInCircle( c8, sign ) ? 1 : 0;
        decided[ 3 ] += PredicateFilterInSphere( c, sign ) ? 1 : 0;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: the Float64 filter decided " << decided[ 0 ] << " Orient2D, " << decided[ 1 ] << " Orient3D, " << decided[ 2 ] << " InCircle and "
               << decided[ 3 ] << " InSphere of " << count << " random cases each" );
    bool ok = true;
    for ( UInt32 k=0; k<4; ++k )
        ok = ok && (Float64) decided[ k ] > 0.99 * (Float64) count;
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Random near-degenerate Float64 inputs (points near a line and a plane) against the fraction reference */
static bool
NearDegenerateFloat64( UInt64 seed, UInt32 count )
{
    typedef CTPoint2D< Float64 > P2;
    typedef CTPoint3D< Float64 > Q3;
    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const P3 a = random.Point( 100.0 );
        const V3 d = random.Vector( 10.0 );
        const V3 e = random.Vector( 10.0 );
        const Float64 t = random.Uniform( -3.0, 3.0 );
        const Float64 s = random.Uniform( -3.0, 3.0 );
        const Float64 tiny = ldexp( random.Uniform( -1.0, 1.0 ), -40 );
        const P3 b = a + d;
        const P3 c = a + d * t + V3( tiny, -tiny, tiny );
        const P3 p = a + d * t + e * s + V3( -tiny, tiny, tiny );
        const Q3 q = random.Point( 100.0 );
        const P2 a2( a.x, a.y );
        const P2 b2( b.x, b.y );
        const P2 c2( c.x, c.y );
        const P2 q2( q.x, q.y );
        const Int32 got[ 4 ] = { CTPredicates< Float64 >::Orient2D( a2, b2, c2 ), CTPredicates< Float64 >::Orient3D( a, b, a + e, p ), CTPredicates< Float64 >::InCircle( a2, b2, q2, c2 ),
                                 CTPredicates< Float64 >::InSphere( a, b, a + e, q, p ) };
        const Int32 expected[ 4 ] = { ReferenceOrient2D( a2, b2, c2 ), ReferenceOrient3D( a, b, a + e, p ), ReferenceInCircle( a2, b2, q2, c2 ), ReferenceInSphere( a, b, a + e, q, p ) };
        for ( UInt32 k=0; k<4; ++k )
        {
            if ( got[ k ] != expected[ k ] )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: near-degenerate Float64 predicate " << k << " gave " << got[ k ] << " expected " << expected[ k ] << ", seed " << seed << " case " << i );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: near-degenerate Float64 seed " << seed << ": " << 4 * count << " signs, " << bad << " wrong" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  Float64 inputs whose coordinates mix magnitudes 2^-500 to 2^500 (products of differences under- or overflow the
 *  static filter): every sign against the fraction reference, and the underflow case of the review
 */
static bool
MixedMagnitudeFloat64( UInt64 seed, UInt32 count )
{
    typedef CTPoint2D< Float64 > P2;
    typedef CTPoint3D< Float64 > Q3;
    /* a = ( 0, -2^-445, 2^200 ), b = ( 2^-540, 0, 1 ), c = ( 2^-445, 2^-540, 0 ), d = 0: the rounded products underflow to a wrong sign */
    const Q3 ua( 0.0, -ldexp( 1.0, -445 ), ldexp( 1.0, 200 ) );
    const Q3 ub( ldexp( 1.0, -540 ), 0.0, 1.0 );
    const Q3 uc( ldexp( 1.0, -445 ), ldexp( 1.0, -540 ), 0.0 );
    const Q3 ud( 0.0, 0.0, 0.0 );
    bool ok = -1 == ReferenceOrient3D( ua, ub, uc, ud ) && -1 == CTPredicates< Float64 >::Orient3D( ua, ub, uc, ud );
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: the underflow case of Orient3D gave " << CTPredicates< Float64 >::Orient3D( ua, ub, uc, ud ) );

    CRandom random( seed );
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        Float64 c[ 15 ];
        for ( UInt32 k=0; k<15; ++k )
        {
            const Float64 mantissa = random.Uniform( -1.0, 1.0 );
            const Int32 exponent = random.Index( 1001 ) - 500;
            const Int32 zero = random.Index( 8 );
            c[ k ] = 0 == zero ? 0.0 : ldexp( mantissa, exponent );
        }
        /* every other case: the points share most coordinates, so the differences are the tiny ones */
        if ( 0 == i % 2 )
        {
            for ( UInt32 k=3; k<15; ++k )
                c[ k ] = c[ k % 3 ] + c[ k ];
        }
        const P2 a2( c[ 0 ], c[ 1 ] ), b2( c[ 3 ], c[ 4 ] ), c2( c[ 6 ], c[ 7 ] ), d2( c[ 9 ], c[ 10 ] );
        const Q3 a3( c[ 0 ], c[ 1 ], c[ 2 ] ), b3( c[ 3 ], c[ 4 ], c[ 5 ] ), c3( c[ 6 ], c[ 7 ], c[ 8 ] ), d3( c[ 9 ], c[ 10 ], c[ 11 ] ), e3( c[ 12 ], c[ 13 ], c[ 14 ] );
        const Int32 got[ 4 ] = { CTPredicates< Float64 >::Orient2D( a2, b2, c2 ), CTPredicates< Float64 >::Orient3D( a3, b3, c3, d3 ), CTPredicates< Float64 >::InCircle( a2, b2, c2, d2 ),
                                 CTPredicates< Float64 >::InSphere( a3, b3, c3, d3, e3 ) };
        const Int32 expected[ 4 ] = { ReferenceOrient2D( a2, b2, c2 ), ReferenceOrient3D( a3, b3, c3, d3 ), ReferenceInCircle( a2, b2, c2, d2 ), ReferenceInSphere( a3, b3, c3, d3, e3 ) };
        for ( UInt32 k=0; k<4; ++k )
        {
            if ( got[ k ] != expected[ k ] )
            {
                ++bad;
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: mixed magnitude predicate " << k << " gave " << got[ k ] << " expected " << expected[ k ] << ", seed " << seed << " case " << i );
            }
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: mixed magnitudes 2^-500 .. 2^500 seed " << seed << ": " << 4 * count << " signs, " << bad << " wrong" );
    return ok && 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  Cocircular and cospherical integer points from Pythagorean triples (radius 5k: ( 5k, 0 ), ( 3k, 4k ), ( -4k, 3k ),
 *  ( 0, -5k ); radius 3k: ( k, 2k, 2k ) and its sign and order variants), moved by 'offset': exactly 0, and the sign
 *  of the fraction reference for the fourth or fifth point moved by one unit inwards and outwards
 */
template < typename T >
static bool
PythagoreanCases( const char* name, T ( *make )( Int64 ), Int64 k, Int64 offset )
{
    typedef CTPoint2D< T > P2;
    typedef CTPoint3D< T > Q3;
    UInt32 bad = 0;
    const P2 a2( make( offset + 5 * k ), make( offset ) );
    const P2 b2( make( offset + 3 * k ), make( offset + 4 * k ) );
    const P2 c2( make( offset - 4 * k ), make( offset + 3 * k ) );
    const Q3 a3( make( offset + k ), make( offset + 2 * k ), make( offset + 2 * k ) );
    const Q3 b3( make( offset + 2 * k ), make( offset - k ), make( offset + 2 * k ) );
    const Q3 c3( make( offset + 2 * k ), make( offset + 2 * k ), make( offset - k ) );
    const Q3 d3( make( offset - 2 * k ), make( offset + k ), make( offset + 2 * k ) );
    const bool positive = CTPredicates< T >::Orient3D( a3, b3, c3, d3 ) > 0;
    for ( Int64 nudge=-1; nudge<=1; ++nudge )
    {
        const P2 d2( make( offset ), make( offset - 5 * k + nudge ) );
        const Q3 e3( make( offset - k ), make( offset - 2 * k ), make( offset - 2 * k + nudge ) );
        const Int32 inCircle = CTPredicates< T >::InCircle( a2, b2, c2, d2 );
        const Int32 inSphere = positive ? CTPredicates< T >::InSphere( a3, b3, c3, d3, e3 ) : CTPredicates< T >::InSphere( b3, a3, c3, d3, e3 );
        const Int32 expectedCircle = ReferenceInCircle( a2, b2, c2, d2 );
        const Int32 expectedSphere = positive ? ReferenceInSphere( a3, b3, c3, d3, e3 ) : ReferenceInSphere( b3, a3, c3, d3, e3 );
        /* on the circle and the sphere for no nudge; one unit towards the centre is inside (+1), away outside (-1) */
        const Int32 geometric = 0 == nudge ? 0 : ( nudge > 0 ? 1 : -1 );
        if ( inCircle != expectedCircle || inSphere != expectedSphere || expectedCircle != geometric || expectedSphere != geometric )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: Pythagorean " << name << " k " << k << " offset " << offset << " nudge " << nudge << ": InCircle " << inCircle << " (expected "
                             << expectedCircle << "), InSphere " << inSphere << " (expected " << expectedSphere << ")" );
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** Near-coplanar fixed-point points far from the origin with small differences (the CInt128 path of Orient3D) against fractions */
static bool
FixedOffsetOrient3D( UInt64 seed, UInt32 count )
{
    typedef CTPoint3D< TFixed32_32 > Q3;
    MATH::CSplitMix64 random( seed );
    UInt32 bad = 0;
    UInt32 zeros = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        Int64 v[ 12 ];
        for ( UInt32 k=0; k<12; ++k )
        {
            const UInt64 draw = random.NextUInt64();
            v[ k ] = (Int64) ( draw % ( (UInt64) 1 << 38 ) ) - ( (Int64) 1 << 37 );
        }
        const UInt64 offsetDraw = random.NextUInt64();
        const Int64 offset = (Int64) ( offsetDraw % ( (UInt64) 1 << 61 ) ) - ( (Int64) 1 << 60 );
        const Int64 nudge = (Int64) ( i % 3 ) - 1;
        const Q3 a( TFixed32_32::FromRaw( offset + v[ 0 ] ), TFixed32_32::FromRaw( offset + v[ 1 ] ), TFixed32_32::FromRaw( offset + v[ 2 ] ) );
        const Q3 b( TFixed32_32::FromRaw( offset + v[ 3 ] ), TFixed32_32::FromRaw( offset + v[ 4 ] ), TFixed32_32::FromRaw( offset + v[ 5 ] ) );
        const Q3 c( TFixed32_32::FromRaw( offset + v[ 6 ] ), TFixed32_32::FromRaw( offset + v[ 7 ] ), TFixed32_32::FromRaw( offset + v[ 8 ] ) );
        /* d = a + ( b - a ) + ( c - a ) (coplanar) plus a nudge of one raw unit */
        const Q3 d( TFixed32_32::FromRaw( offset + v[ 3 ] + v[ 6 ] - v[ 0 ] ), TFixed32_32::FromRaw( offset + v[ 4 ] + v[ 7 ] - v[ 1 ] ),
                    TFixed32_32::FromRaw( offset + v[ 5 ] + v[ 8 ] - v[ 2 ] + nudge ) );
        const Int32 got = CTPredicates< TFixed32_32 >::Orient3D( a, b, c, d );
        const Int32 expected = ReferenceOrient3D( a, b, c, d );
        zeros += 0 == expected ? 1 : 0;
        if ( got != expected )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: fixed-point Orient3D at offset gave " << got << " expected " << expected << ", seed " << seed << " case " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Predicates: fixed-point Orient3D near 2^60 seed " << seed << ": " << count << " signs, " << zeros << " exactly zero, " << bad << " wrong" );
    return 0 == bad && zeros > count / 4;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformPredicatesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Predicates" );

    GUCEF_TESTFW_TESTCASE( "Predicates: Orient2D gives the exact sign on the classic near-degenerate grid, where naive Float64 evaluation errs" )
    try
    {
        UInt32 naiveErrors = 0;
        ASSERT_TRUE( ClassicGrid( naiveErrors ) );
        ASSERT_TRUE( naiveErrors > 0 );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Predicates: InCircle and InSphere are 0 on cocircular and cospherical inputs at scales 2^-30 to 2^30, exact for one-ulp nudges" )
    try
    {
        ASSERT_TRUE( CocircularCospherical() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Predicates: integer, fixed-point and float kinds are exact on near-degenerate inputs against fraction evaluation" )
    try
    {
        ASSERT_TRUE( KindSweep< Int32 >( "Int32", 1101, ScaledCount( 2000 ), (Int64) 1 << 28, MakeInt32 ) );
        ASSERT_TRUE( KindSweep< Int64 >( "Int64", 1102, ScaledCount( 2000 ), (Int64) 1 << 61, MakeInt64 ) );
        ASSERT_TRUE( KindSweep< Int64 >( "Int64 small", 1103, ScaledCount( 2000 ), (Int64) 1 << 36, MakeInt64 ) );
        ASSERT_TRUE( KindSweep< TFixed32_32 >( "TFixed32_32", 1104, ScaledCount( 2000 ), (Int64) 1 << 40, MakeFixed32_32 ) );
        ASSERT_TRUE( KindSweep< TFixed32_32 >( "TFixed32_32 wide", 1105, ScaledCount( 2000 ), (Int64) 1 << 60, MakeFixed32_32 ) );
        ASSERT_TRUE( KindSweep< TFixed16_16 >( "TFixed16_16", 1106, ScaledCount( 2000 ), (Int64) 1 << 28, MakeFixed16_16 ) );
        ASSERT_TRUE( KindSweep< Float64 >( "Float64", 1107, ScaledCount( 2000 ), (Int64) 1 << 50, MakeFloat64 ) );
        ASSERT_TRUE( KindSweep< Float32 >( "Float32", 1108, ScaledCount( 2000 ), (Int64) 1 << 21, MakeFloat32 ) );
        ASSERT_TRUE( NearDegenerateFloat64( 1109, ScaledCount( 10000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Predicates: Float64 inputs mixing 2^-500 and 2^500 (filter underflow and overflow) are exact against fraction evaluation" )
    try
    {
        ASSERT_TRUE( MixedMagnitudeFloat64( 1111, ScaledCount( 4000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Predicates: Pythagorean cocircular and cospherical integer and fixed-point inputs (stack integer InCircle and InSphere) and offset Orient3D" )
    try
    {
        const Int64 ks[ 5 ] = { 1, 12345, ( (Int64) 1 << 20 ) + 3, ( (Int64) 1 << 40 ) + 7, ( (Int64) 1 << 57 ) + 1 };
        const Int64 offsets[ 3 ] = { 0, -( (Int64) 1 << 59 ), ( (Int64) 1 << 59 ) + 12345 };
        for ( UInt32 i=0; i<5; ++i )
        {
            for ( UInt32 j=0; j<3; ++j )
            {
                ASSERT_TRUE( PythagoreanCases< Int64 >( "Int64", MakeInt64, ks[ i ], offsets[ j ] ) );
                ASSERT_TRUE( PythagoreanCases< TFixed32_32 >( "TFixed32_32", MakeFixed32_32, ks[ i ], offsets[ j ] ) );
            }
        }
        ASSERT_TRUE( PythagoreanCases< Int32 >( "Int32", MakeInt32, 12345, 1000 ) );
        ASSERT_TRUE( FixedOffsetOrient3D( 1112, ScaledCount( 4000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Predicates: the fraction kind evaluates directly and agrees with the Float64 predicates on the same values" )
    try
    {
        typedef CTPoint3D< TMathBigFraction > PF;
        CRandom random( 1111 );
        bool ok = true;
        for ( UInt32 i=0; i<500 && ok; ++i )
        {
            CTPoint3D< Float64 > p[ 5 ];
            PF q[ 5 ];
            for ( UInt32 k=0; k<5; ++k )
            {
                p[ k ] = random.Point( 10.0 );
                q[ k ] = PF( Exact( p[ k ].x ), Exact( p[ k ].y ), Exact( p[ k ].z ) );
            }
            ok = CTPredicates< TMathBigFraction >::Orient3D( q[ 0 ], q[ 1 ], q[ 2 ], q[ 3 ] ) == CTPredicates< Float64 >::Orient3D( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ] );
            ok = ok && CTPredicates< TMathBigFraction >::InSphere( q[ 0 ], q[ 1 ], q[ 2 ], q[ 3 ], q[ 4 ] ) == CTPredicates< Float64 >::InSphere( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ], p[ 4 ] );
            const CTPoint2D< TMathBigFraction > r[ 4 ] = { CTPoint2D< TMathBigFraction >( q[ 0 ].x, q[ 0 ].y ), CTPoint2D< TMathBigFraction >( q[ 1 ].x, q[ 1 ].y ),
                                                          CTPoint2D< TMathBigFraction >( q[ 2 ].x, q[ 2 ].y ), CTPoint2D< TMathBigFraction >( q[ 3 ].x, q[ 3 ].y ) };
            const CTPoint2D< Float64 > s[ 4 ] = { CTPoint2D< Float64 >( p[ 0 ].x, p[ 0 ].y ), CTPoint2D< Float64 >( p[ 1 ].x, p[ 1 ].y ), CTPoint2D< Float64 >( p[ 2 ].x, p[ 2 ].y ), CTPoint2D< Float64 >( p[ 3 ].x, p[ 3 ].y ) };
            ok = ok && CTPredicates< TMathBigFraction >::Orient2D( r[ 0 ], r[ 1 ], r[ 2 ] ) == CTPredicates< Float64 >::Orient2D( s[ 0 ], s[ 1 ], s[ 2 ] );
            ok = ok && CTPredicates< TMathBigFraction >::InCircle( r[ 0 ], r[ 1 ], r[ 2 ], r[ 3 ] ) == CTPredicates< Float64 >::InCircle( s[ 0 ], s[ 1 ], s[ 2 ], s[ 3 ] );
        }
        ASSERT_TRUE( ok );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Predicates: the Float64 filter decides more than 99 % of 10^6 random cases per predicate without the exact stage" )
    try
    {
        ASSERT_TRUE( FilterEfficiency( 1121, ScaledCount( 1000000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
