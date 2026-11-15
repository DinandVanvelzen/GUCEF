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

#ifndef GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#include "gucefGEOMETRY_BoundingVolumeFitting.h"
#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#endif /* GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H ? */

#include "TestConvexReference.h"
#include "TestFitting.h"

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
using namespace ConvexReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The sphere through 2, 3 or 4 points written for the test (Cramer's rule on the bisector planes); false when degenerate */
static bool
ReferenceCircumsphere( const P3* points, UInt32 count, P3& centre, Float64& radius )
{
    if ( 2 == count )
    {
        centre = points[ 0 ] + ( points[ 1 ] - points[ 0 ] ) * 0.5;
        radius = ( points[ 1 ] - points[ 0 ] ).GetLength() * 0.5;
        return true;
    }
    /* rows of the linear system 2 ( p_i - p_0 ) . c = |p_i|^2 - |p_0|^2, plus the plane of the triangle for 3 points */
    Float64 m[ 3 ][ 3 ];
    Float64 rhs[ 3 ];
    const V3 origin = points[ 0 ].ToPositionVector();
    for ( UInt32 i=1; i<count; ++i )
    {
        const V3 d = points[ i ] - points[ 0 ];
        m[ i - 1 ][ 0 ] = 2.0 * d.x;
        m[ i - 1 ][ 1 ] = 2.0 * d.y;
        m[ i - 1 ][ 2 ] = 2.0 * d.z;
        rhs[ i - 1 ] = d.Dot( d );
    }
    if ( 3 == count )
    {
        const V3 n = ( points[ 1 ] - points[ 0 ] ).Cross( points[ 2 ] - points[ 0 ] );
        m[ 2 ][ 0 ] = n.x;
        m[ 2 ][ 1 ] = n.y;
        m[ 2 ][ 2 ] = n.z;
        rhs[ 2 ] = 0.0;
    }
    const Float64 det = m[ 0 ][ 0 ] * ( m[ 1 ][ 1 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 1 ] ) - m[ 0 ][ 1 ] * ( m[ 1 ][ 0 ] * m[ 2 ][ 2 ] - m[ 1 ][ 2 ] * m[ 2 ][ 0 ] ) + m[ 0 ][ 2 ] * ( m[ 1 ][ 0 ] * m[ 2 ][ 1 ] - m[ 1 ][ 1 ] * m[ 2 ][ 0 ] );
    if ( fabs( det ) < 1.0e-12 )
        return false;
    Float64 c[ 3 ];
    for ( UInt32 k=0; k<3; ++k )
    {
        Float64 a[ 3 ][ 3 ];
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 s=0; s<3; ++s )
                a[ r ][ s ] = s == k ? rhs[ r ] : m[ r ][ s ];
        c[ k ] = ( a[ 0 ][ 0 ] * ( a[ 1 ][ 1 ] * a[ 2 ][ 2 ] - a[ 1 ][ 2 ] * a[ 2 ][ 1 ] ) - a[ 0 ][ 1 ] * ( a[ 1 ][ 0 ] * a[ 2 ][ 2 ] - a[ 1 ][ 2 ] * a[ 2 ][ 0 ] ) + a[ 0 ][ 2 ] * ( a[ 1 ][ 0 ] * a[ 2 ][ 1 ] - a[ 1 ][ 1 ] * a[ 2 ][ 0 ] ) ) / det;
    }
    centre = P3( c[ 0 ], c[ 1 ], c[ 2 ] ) + origin;
    radius = ( points[ 0 ] - centre ).GetLength();
    return true;
}

/*-------------------------------------------------------------------------*/

/** True when the sphere through the support set exists and contains every point (relative slack 1e-9) */
static bool
EnclosesAll( const GUCEF::vector< P3 >& points, const P3* set, UInt32 count, Float64& radius )
{
    P3 centre;
    if ( !ReferenceCircumsphere( set, count, centre, radius ) )
        return false;
    for ( UInt32 m=0; m<points.size(); ++m )
        if ( ( points[ m ] - centre ).GetLength() > radius * ( 1.0 + 1.0e-9 ) + 1.0e-9 )
            return false;
    return true;
}

/** The smallest enclosing sphere by brute force: the smallest of the spheres over all 2, 3 and 4 point support sets that contains every point */
static Float64
BruteForceMinimumRadius( const GUCEF::vector< P3 >& points )
{
    const UInt32 n = (UInt32) points.size();
    if ( 1 == n )
        return 0.0;
    Float64 best = 1.0e300;
    Float64 radius = 0.0;
    for ( UInt32 i=0; i<n; ++i )
        for ( UInt32 j=i+1; j<n; ++j )
        {
            const P3 pair[ 2 ] = { points[ i ], points[ j ] };
            if ( EnclosesAll( points, pair, 2, radius ) && radius < best )
                best = radius;
            for ( UInt32 k=j+1; k<n; ++k )
            {
                const P3 triple[ 3 ] = { points[ i ], points[ j ], points[ k ] };
                if ( EnclosesAll( points, triple, 3, radius ) && radius < best )
                    best = radius;
                for ( UInt32 l=k+1; l<n; ++l )
                {
                    const P3 quad[ 4 ] = { points[ i ], points[ j ], points[ k ], points[ l ] };
                    if ( EnclosesAll( points, quad, 4, radius ) && radius < best )
                        best = radius;
                }
            }
        }
    return best;
}

/*-------------------------------------------------------------------------*/

/** A random point set of 1 to 12 points: general, collinear, cospherical, with duplicates or coplanar (values drawn into locals) */
static GUCEF::vector< P3 >
RandomFittingSet( CRandom& random, UInt32& kind )
{
    GUCEF::vector< P3 > points;
    const UInt32 count = 1 + (UInt32) random.Index( 12 );
    kind = (UInt32) random.Index( 5 );
    const P3 centre = random.Point( 2.0 );
    const Float64 radius = random.Uniform( 0.5, 2.0 );
    const D3 axis = random.Direction();
    for ( UInt32 i=0; i<count; ++i )
    {
        if ( 0 == kind )
        {
            points.push_back( random.Point( 2.0 ) );
        }
        else if ( 1 == kind )
        {
            /* collinear */
            const Float64 t = random.Uniform( -2.0, 2.0 );
            points.push_back( centre + axis.AsVector() * t );
        }
        else if ( 2 == kind )
        {
            /* cospherical */
            const D3 d = random.Direction();
            points.push_back( centre + d.AsVector() * radius );
        }
        else if ( 3 == kind )
        {
            /* duplicates */
            const P3 p = random.Point( 2.0 );
            points.push_back( p );
            if ( random.Chance( 0.5 ) )
                points.push_back( p );
        }
        else
        {
            /* coplanar on a circle: cocircular */
            const Float64 angle = random.Uniform( 0.0, 6.283185307179586 );
            points.push_back( centre + V3( cos( angle ) * radius, 0.0, sin( angle ) * radius ) );
        }
    }
    if ( points.size() > 12 )
        points.resize( 12 );
    return points;
}

/*-------------------------------------------------------------------------*/

/** Welzl against the brute-force minimum over all support sets for 1000 seeded sets of at most 12 points */
static bool
WelzlSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 kinds[ 5 ] = { 0, 0, 0, 0, 0 };
    Float64 worst = 0.0;
    GUCEF::vector< P3 > scratch;
    for ( UInt32 i=0; i<count; ++i )
    {
        UInt32 kind = 0;
        const GUCEF::vector< P3 > points = RandomFittingSet( random, kind );
        ++kinds[ kind ];
        CTSphere< Float64 > sphere;
        const bool fitted = FitMinimumSphere( &points[ 0 ], (UInt32) points.size(), scratch, seed + i, sphere );
        const Float64 expected = BruteForceMinimumRadius( points );
        bool ok = fitted;
        for ( UInt32 k=0; k<points.size() && ok; ++k )
            ok = ( points[ k ] - sphere.centre ).GetLength() <= sphere.radius * ( 1.0 + 1.0e-9 ) + 1.0e-12;
        const Float64 error = fabs( sphere.radius - expected );
        worst = error > worst ? error : worst;
        ok = ok && error <= 1.0e-9 * ( 1.0 + expected );
        /* Ritter encloses every point with a larger or equal sphere */
        CTSphere< Float64 > ritter;
        ok = ok && FitRitterSphere( &points[ 0 ], (UInt32) points.size(), ritter ) && ritter.radius >= sphere.radius - 1.0e-9;
        for ( UInt32 k=0; k<points.size() && ok; ++k )
            ok = ( points[ k ] - ritter.centre ).GetLength() <= ritter.radius * ( 1.0 + 1.0e-12 ) + 1.0e-12;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: set " << i << " (kind " << kind << ", " << (UInt32) points.size() << " points) radius " << sphere.radius << " expected " << expected << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: Welzl seed " << seed << ": " << count << " sets (" << kinds[ 0 ] << " general, " << kinds[ 1 ] << " collinear, " << kinds[ 2 ] << " cospherical, "
               << kinds[ 3 ] << " with duplicates, " << kinds[ 4 ] << " cocircular), worst error " << worst << ", " << bad << " bad" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/** The fraction kind: exact centre and squared radius, every point inside or on the sphere exactly */
static bool
ExactFractionSpheres( UInt64 seed )
{
    typedef CTPoint3D< TMathBigFraction > PF;
    const TMathBigFraction zero( 0, 1 );
    const TMathBigFraction one( 1, 1 );
    const TMathBigFraction minusOne( -1, 1 );
    const TMathBigFraction half( 1, 2 );
    /* the six axis points of the unit sphere and an interior point: centre 0, squared radius 1 exactly */
    const PF octahedron[ 7 ] = { PF( one, zero, zero ), PF( minusOne, zero, zero ), PF( zero, one, zero ), PF( zero, minusOne, zero ), PF( zero, zero, one ), PF( zero, zero, minusOne ), PF( half, half, zero ) };
    PF scratch[ 12 ];
    PF centre;
    TMathBigFraction squared( 0, 1 );
    bool ok = FitMinimumSphereSquared( octahedron, 7, scratch, seed, centre, squared ) && centre == PF( zero, zero, zero ) && squared == one;

    /* random small integer sets: exact containment and agreement with Float64 */
    CRandom random( seed );
    for ( UInt32 i=0; i<200 && ok; ++i )
    {
        const UInt32 count = 2 + (UInt32) random.Index( 9 );
        PF points[ 12 ];
        P3 wide[ 12 ];
        for ( UInt32 k=0; k<count; ++k )
        {
            const Int32 x = random.Index( 21 ) - 10;
            const Int32 y = random.Index( 21 ) - 10;
            const Int32 z = random.Index( 21 ) - 10;
            points[ k ] = PF( TMathBigFraction( x, 1 ), TMathBigFraction( y, 1 ), TMathBigFraction( z, 1 ) );
            wide[ k ] = P3( x, y, z );
        }
        ok = FitMinimumSphereSquared( points, count, scratch, seed + i, centre, squared );
        UInt32 onSurface = 0;
        for ( UInt32 k=0; k<count && ok; ++k )
        {
            const TMathBigFraction d = ( points[ k ] - centre ).GetSquaredLength();
            ok = d <= squared;
            onSurface += d == squared ? 1 : 0;
        }
        P3 wideScratch[ 12 ];
        CTSphere< Float64 > sphere;
        FitMinimumSphere( wide, count, wideScratch, seed + i, sphere );
        ok = ok && onSurface >= 2 && fabs( sqrt( CTNumericTraits< TMathBigFraction >::ToFloat64( squared ) ) - sphere.radius ) < 1.0e-9;
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: exact fraction set " << i << " failed, seed " << seed );
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Principal component boxes of rotated box point clouds: every point inside, the volume within 1 % of the true box */
template < typename T >
static bool
PcaBoxSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    Float64 worstRatio = 0.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 hx = random.Uniform( 2.0, 3.0 );
        const Float64 hy = random.Uniform( 1.0, 1.8 );
        const Float64 hz = random.Uniform( 0.2, 0.8 );
        const V3 half( hx, hy, hz );
        const P3 centre = random.Point( 5.0 );
        const Q4 rotation = random.Rotation();
        const CTOrientedBox3D< Float64 > truth( centre, rotation, half );
        GUCEF::vector< CTPoint3D< T > > points;
        P3 corners[ 8 ];
        truth.GetCorners( corners );
        for ( UInt32 k=0; k<8; ++k )
            points.push_back( ToP< T >( corners[ k ] ) );
        /* a lattice over the box (a symmetric cloud: its covariance has the box axes as eigenvectors) */
        for ( UInt32 x=0; x<=6; ++x )
            for ( UInt32 y=0; y<=6; ++y )
                for ( UInt32 z=0; z<=6; ++z )
                    points.push_back( ToP< T >( truth.FromLocal( V3( ( x / 3.0 - 1.0 ) * half.x, ( y / 3.0 - 1.0 ) * half.y, ( z / 3.0 - 1.0 ) * half.z ) ) ) );
        CTOrientedBox3D< T > box;
        bool ok = FitOrientedBox( &points[ 0 ], (UInt32) points.size(), box );
        /* a random cloud in the same box: the principal component box encloses every point */
        GUCEF::vector< CTPoint3D< T > > cloud;
        for ( UInt32 k=0; k<300; ++k )
        {
            const Float64 x = random.Uniform( -1.0, 1.0 );
            const Float64 y = random.Uniform( -1.0, 1.0 );
            const Float64 z = random.Uniform( -1.0, 1.0 );
            cloud.push_back( ToP< T >( truth.FromLocal( V3( x * half.x, y * half.y, z * half.z ) ) ) );
        }
        CTOrientedBox3D< T > cloudBox;
        ok = ok && FitOrientedBox( &cloud[ 0 ], (UInt32) cloud.size(), cloudBox );
        for ( UInt32 k=0; k<cloud.size() && ok; ++k )
            ok = Widen( cloudBox ).GetDistance( Widen( cloud[ k ] ) ) <= ( sizeof( T ) == 4 ? 1.0e-3 : 1.0e-8 );
        const Float64 tolerance = sizeof( T ) == 4 ? 1.0e-4 : 1.0e-9;
        for ( UInt32 k=0; k<points.size() && ok; ++k )
            ok = Widen( box ).GetDistance( Widen( points[ k ] ) ) <= tolerance * 10.0;
        const Float64 ratio = (Float64) box.GetVolume() / truth.GetVolume();
        worstRatio = fabs( ratio - 1.0 ) > worstRatio ? fabs( ratio - 1.0 ) : worstRatio;
        ok = ok && fabs( ratio - 1.0 ) <= 0.01;

        /* the hull refinement finds the box itself from its corners */
        const CTConvexHull3D< T > hull = CTConvexHull3D< T >::FromOrientedBox( CTOrientedBox3D< T >( ToP< T >( truth.centre ), ToQ< T >( truth.rotation ), ToV< T >( half ) ) );
        CTOrientedBox3D< T > refined;
        ok = ok && FitOrientedBoxFromHull( hull, refined ) && fabs( (Float64) refined.GetVolume() / truth.GetVolume() - 1.0 ) <= tolerance * 100.0;
        for ( UInt32 k=0; k<hull.GetVertexCount() && ok; ++k )
            ok = Widen( refined ).GetDistance( Widen( hull.GetVertex( k ) ) ) <= tolerance * 10.0;
        if ( !ok )
        {
            ++bad;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: box " << i << " volume ratio " << ratio << ", seed " << seed );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: principal component boxes seed " << seed << ": " << count << " clouds, worst volume ratio difference " << worstRatio << ", " << bad << " bad" );
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

/**
 *  64 points on a tilted circle of radius 1 far from the origin (exactly coplanar before rounding): the smallest sphere
 *  has radius 1 up to the rounding of the coordinates; nearly coplanar support sets must not give a huge circumsphere
 */
template < typename T >
static bool
TiltedCircleSphere( UInt64 seed, Float64 offset, Float64 tolerance )
{
    CTPoint3D< T > points[ 64 ];
    CTPoint3D< T > scratch[ 64 ];
    for ( UInt32 i=0; i<64; ++i )
    {
        const Float64 angle = 6.283185307179586 * i / 64.0;
        const Float64 x = cos( angle );
        const Float64 y = sin( angle ) * 0.6;
        const Float64 z = sin( angle ) * 0.8;
        points[ i ] = CTPoint3D< T >( (T) ( x + offset ), (T) ( y - offset ), (T) ( z + 0.5 * offset ) );
    }
    bool ok = true;
    for ( UInt32 trial=0; trial<20 && ok; ++trial )
    {
        CTSphere< T > sphere;
        ok = FitMinimumSphere( points, 64, scratch, seed + trial, sphere ) && fabs( (Float64) sphere.radius - 1.0 ) <= tolerance;
        for ( UInt32 k=0; k<64 && ok; ++k )
            ok = Widen( points[ k ] ).GetDistance( Widen( sphere.centre ) ) <= (Float64) sphere.radius + tolerance;
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: tilted circle at offset " << offset << " gave radius " << (Float64) sphere.radius << ", seed " << seed + trial );
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** The smallest sphere does not depend on the order of the input: reversed, rotated and shuffled copies give the same bits */
template < typename T >
static bool
WelzlOrderIndependence( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    bool ok = true;
    for ( UInt32 i=0; i<count && ok; ++i )
    {
        const UInt32 n = 4 + (UInt32) random.Index( 60 );
        CTPoint3D< T > points[ 64 ];
        CTPoint3D< T > reordered[ 64 ];
        CTPoint3D< T > scratch[ 64 ];
        const P3 centre = random.Point( 1000.0 );
        for ( UInt32 k=0; k<n; ++k )
        {
            const P3 p = centre + random.Vector( 10.0 );
            points[ k ] = ToP< T >( p );
        }
        CTSphere< T > reference;
        ok = FitMinimumSphere( points, n, scratch, seed + i, reference );
        for ( UInt32 order=0; order<3 && ok; ++order )
        {
            for ( UInt32 k=0; k<n; ++k )
                reordered[ k ] = 0 == order ? points[ n - 1 - k ] : points[ ( k + 17 ) % n ];
            if ( 2 == order )
            {
                for ( UInt32 k=n-1; k>0; --k )
                {
                    const UInt32 j = (UInt32) random.Index( (Int32) k + 1 );
                    const CTPoint3D< T > swap = reordered[ k ];
                    reordered[ k ] = reordered[ j ];
                    reordered[ j ] = swap;
                }
            }
            CTSphere< T > sphere;
            ok = FitMinimumSphere( reordered, n, scratch, seed + i, sphere ) && sphere.centre == reference.centre && sphere.radius == reference.radius;
            if ( !ok )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: the input order " << order << " changed the sphere of set " << i << ", seed " << seed );
        }
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  Principal component boxes of clouds with repeated eigenvalues (a cube lattice: one triple eigenvalue; a 1 x 1 x 3
 *  box lattice: a double one), rotated and far from the origin: the axes in the repeated eigenspace are arbitrary, so
 *  the box contains every point and is at most 3 sqrt( 3 ) times the true volume; the hull refinement finds the box
 */
template < typename T >
static bool
RepeatedEigenvalueBoxes( Float64 offset, Float64 tolerance )
{
    bool ok = true;
    for ( UInt32 kind=0; kind<2 && ok; ++kind )
    {
        const V3 half( 1.0, 1.0, 0 == kind ? 1.0 : 3.0 );
        const CTOrientedBox3D< Float64 > truth( P3( offset, offset, -offset ), Q4::FromAxisAngle( D3::UnitZ(), CTRadian< Float64 >( 0.3 ) ), half );
        GUCEF::vector< CTPoint3D< T > > points;
        for ( UInt32 x=0; x<=6; ++x )
            for ( UInt32 y=0; y<=6; ++y )
                for ( UInt32 z=0; z<=6; ++z )
                    points.push_back( ToP< T >( truth.FromLocal( V3( ( x / 3.0 - 1.0 ) * half.x, ( y / 3.0 - 1.0 ) * half.y, ( z / 3.0 - 1.0 ) * half.z ) ) ) );
        CTOrientedBox3D< T > box;
        ok = FitOrientedBox( &points[ 0 ], (UInt32) points.size(), box ) && box.IsValid();
        for ( UInt32 k=0; k<points.size() && ok; ++k )
            ok = Widen( box ).GetDistance( Widen( points[ k ] ) ) <= tolerance;
        const Float64 ratio = (Float64) box.GetVolume() / truth.GetVolume();
        ok = ok && ratio >= 1.0 - tolerance && ratio <= 5.2;
        const CTConvexHull3D< T > hull = CTConvexHull3D< T >::FromOrientedBox( CTOrientedBox3D< T >( ToP< T >( truth.centre ), ToQ< T >( truth.rotation ), ToV< T >( half ) ) );
        CTOrientedBox3D< T > refined;
        ok = ok && FitOrientedBoxFromHull( hull, refined ) && fabs( (Float64) refined.GetVolume() / truth.GetVolume() - 1.0 ) <= tolerance;
        if ( !ok )
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Fitting: repeated eigenvalue cloud " << kind << " at offset " << offset << ": volume ratio " << ratio );
    }
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformFittingTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Fitting" );

    GUCEF_TESTFW_TESTCASE( "Fitting: Welzl equals the brute-force minimum over all 2, 3 and 4 point support sets, 1000 seeded sets of at most 12 points incl. collinear and cospherical" )
    try
    {
        ASSERT_TRUE( WelzlSweep( 1001, ScaledCount( 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fitting: the smallest sphere is exact for the fraction kind" )
    try
    {
        ASSERT_TRUE( ExactFractionSpheres( 1011 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fitting: principal component boxes contain every point; for rotated box lattices the volume is within 1 % of the true box; the hull refinement finds the box" )
    try
    {
        ASSERT_TRUE( PcaBoxSweep< Float64 >( 1021, ScaledCount( 200 ) ) );
        ASSERT_TRUE( PcaBoxSweep< Float32 >( 1022, ScaledCount( 100 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fitting: Welzl on nearly coplanar sets (a tilted circle at offset 1000 in Float32, 10^6 in Float64) has radius 1; the result does not depend on the input order" )
    try
    {
        ASSERT_TRUE( TiltedCircleSphere< Float32 >( 1031, 1000.0, 2.0e-3 ) );
        ASSERT_TRUE( TiltedCircleSphere< Float64 >( 1032, 1.0e6, 1.0e-6 ) );
        ASSERT_TRUE( TiltedCircleSphere< Float64 >( 1033, 0.0, 1.0e-12 ) );
        ASSERT_TRUE( WelzlOrderIndependence< Float64 >( 1034, ScaledCount( 200 ) ) );
        ASSERT_TRUE( WelzlOrderIndependence< Float32 >( 1035, ScaledCount( 200 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fitting: principal component boxes of cube and cylinder-like lattices (repeated eigenvalues) at offset 1000 contain every point; the hull refinement finds the box" )
    try
    {
        ASSERT_TRUE( RepeatedEigenvalueBoxes< Float64 >( 0.0, 1.0e-9 ) );
        ASSERT_TRUE( RepeatedEigenvalueBoxes< Float64 >( 1000.0, 1.0e-9 ) );
        ASSERT_TRUE( RepeatedEigenvalueBoxes< Float32 >( 0.0, 1.0e-4 ) );
        ASSERT_TRUE( RepeatedEigenvalueBoxes< Float32 >( 1000.0, 1.0e-3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Fitting: empty point sets and a single point" )
    try
    {
        const CTPoint3D< Float64 > single( 1.0, 2.0, 3.0 );
        CTPoint3D< Float64 > scratch[ 1 ];
        CTSphere< Float64 > sphere;
        CTOrientedBox3D< Float64 > box;
        ASSERT_FALSE( FitMinimumSphere( &single, 0, scratch, 1, sphere ) );
        ASSERT_FALSE( FitRitterSphere( &single, 0, sphere ) );
        ASSERT_FALSE( FitOrientedBox( &single, 0, box ) );
        ASSERT_FALSE( FitOrientedBoxFromHull( CTConvexHull3D< Float64 >(), box ) );
        ASSERT_TRUE( FitMinimumSphere( &single, 1, scratch, 1, sphere ) && sphere.centre == single && 0.0 == sphere.radius );
        ASSERT_TRUE( FitOrientedBox( &single, 1, box ) && box.centre == single );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
