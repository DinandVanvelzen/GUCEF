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

#include "TestQueryReference.h"
#include "TestContainment.h"

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

typedef TFloat64Point3D P;
typedef TFloat64Vector3D V;
typedef TFloat64DirectionVector3D D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** How far inside the outer shape a point is, by the plain definition of the shape: positive inside */
static Float64
SlackIn( const CTAxisAlignedBox3D< Float64 >& box, const P3& p )
{
    return fmin( fmin( fmin( p.x - box.min.x, box.max.x - p.x ), fmin( p.y - box.min.y, box.max.y - p.y ) ), fmin( p.z - box.min.z, box.max.z - p.z ) );
}

static Float64
SlackIn( const CTOrientedBox3D< Float64 >& box, const P3& p )
{
    const V3 local = box.rotation.Conjugate().Rotate( p - box.centre );
    return fmin( fmin( box.halfExtents.x - fabs( local.x ), box.halfExtents.y - fabs( local.y ) ), box.halfExtents.z - fabs( local.z ) );
}

static Float64
SlackIn( const CTSphere< Float64 >& sphere, const P3& p )
{
    return sphere.radius - ( p - sphere.centre ).GetLength();
}

static Float64
SlackIn( const CTFrustum< Float64 >& frustum, const P3& p )
{
    Float64 slack = 1.0e300;
    for ( UInt32 i=0; i<6; ++i )
        slack = fmin( slack, frustum.GetPlane( i ).GetSignedDistance( p ) );
    return slack;
}

static Float64
SlackIn( const CTPlaneBoundedVolume< Float64 >& volume, const P3& p )
{
    Float64 slack = 1.0e300;
    for ( UInt32 i=0; i<volume.GetPlaneCount(); ++i )
        slack = fmin( slack, volume.GetPlane( i ).GetSignedDistance( p ) );
    return slack;
}

/*-------------------------------------------------------------------------*/

/** The outer shape grown about the middle of its bounds, so that a good share of the random inner shapes fit */
template < typename T >
static void
Enlarge( CTAxisAlignedBox3D< T >& box, Float64 factor )
{
    box = CTAxisAlignedBox3D< T >::FromCentreHalfExtents( box.GetCentre(), box.GetHalfExtents() * (T) factor );
}

template < typename T >
static void
Enlarge( CTOrientedBox3D< T >& box, Float64 factor )
{
    box.halfExtents = box.halfExtents * (T) factor;
}

template < typename T >
static void
Enlarge( CTSphere< T >& sphere, Float64 factor )
{
    sphere.radius = sphere.radius * (T) factor;
}

template < typename T >
static CTMatrix4D< T >
GrowthMatrix( const CTPoint3D< T >& c, Float64 factor )
{
    typedef CTMatrix4D< T > M;
    const T f = (T) factor;
    return M::Translation( c.ToPositionVector() ) * M::Scale( CTVector3D< T >( f, f, f ) ) * M::Translation( -c.ToPositionVector() );
}

template < typename T >
static void
Enlarge( CTFrustum< T >& frustum, Float64 factor )
{
    CTPoint3D< T > c;
    for ( UInt32 i=0; i<8; ++i )
        c = c + frustum.GetCorner( i ).ToPositionVector() * (T) 0.125;
    frustum = frustum.Transformed( GrowthMatrix( c, factor ) );
}

template < typename T >
static void
Enlarge( CTPlaneBoundedVolume< T >& volume, Float64 factor )
{
    CTAxisAlignedBox3D< T > bounds;
    CTPoint3D< T > c;
    if ( volume.TryGetBounds( bounds ) )
        c = bounds.GetCentre();
    CTPlaneBoundedVolume< T > grown;
    volume.TryTransformed( GrowthMatrix( c, factor ), grown );
    volume = grown;
}

/*-------------------------------------------------------------------------*/

/** The directions of the faces of the outer shape: along them the extreme points of the inner shape decide its containment */
static void
OuterNormals( const CTAxisAlignedBox3D< Float64 >&, const P3&, GUCEF::vector< V3 >& out )
{
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        V3 unit;
        unit.SetComponent( axis, 1.0 );
        out.push_back( unit );
        out.push_back( unit * -1.0 );
    }
}

static void
OuterNormals( const CTOrientedBox3D< Float64 >& box, const P3&, GUCEF::vector< V3 >& out )
{
    V3 axes[ 3 ];
    box.GetAxes( axes );
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        out.push_back( axes[ axis ] );
        out.push_back( axes[ axis ] * -1.0 );
    }
}

static void
OuterNormals( const CTSphere< Float64 >& sphere, const P3& hint, GUCEF::vector< V3 >& out )
{
    const V3 d = hint - sphere.centre;
    if ( d.GetLength() > 0 )
        out.push_back( d.Normalized() );
}

static void
OuterNormals( const CTFrustum< Float64 >& frustum, const P3&, GUCEF::vector< V3 >& out )
{
    for ( UInt32 i=0; i<6; ++i )
        out.push_back( frustum.GetPlane( i ).normal.AsVector() * -1.0 );
}

static void
OuterNormals( const CTPlaneBoundedVolume< Float64 >& volume, const P3&, GUCEF::vector< V3 >& out )
{
    for ( UInt32 i=0; i<volume.GetPlaneCount(); ++i )
        out.push_back( volume.GetPlane( i ).normal.AsVector() * -1.0 );
}

/** The extreme points of the round inner shapes along the directions, written out for each shape (the polytopes are sampled at their corners) */
static void
AddExtremes( const CTSphere< Float64 >& s, const GUCEF::vector< V3 >& directions, GUCEF::vector< P3 >& out )
{
    for ( size_t i=0; i<directions.size(); ++i )
        out.push_back( s.centre + directions[ i ] * s.radius );
}

static void
AddExtremes( const CTCapsule< Float64 >& c, const GUCEF::vector< V3 >& directions, GUCEF::vector< P3 >& out )
{
    for ( size_t i=0; i<directions.size(); ++i )
    {
        const P3 tip = directions[ i ].Dot( c.end - c.start ) > 0 ? c.end : c.start;
        out.push_back( tip + directions[ i ] * c.radius );
    }
}

static void
AddExtremes( const CTCylinder< Float64 >& c, const GUCEF::vector< V3 >& directions, GUCEF::vector< P3 >& out )
{
    const V3 u = ( c.end - c.start ).Normalized();
    for ( size_t i=0; i<directions.size(); ++i )
    {
        const P3 cap = directions[ i ].Dot( u ) > 0 ? c.end : c.start;
        const V3 lateral = directions[ i ] - u * directions[ i ].Dot( u );
        out.push_back( lateral.GetLength() > 0 ? cap + lateral.Normalized() * c.radius : cap );
    }
}

static void
AddExtremes( const CTCone< Float64 >& c, const GUCEF::vector< V3 >& directions, GUCEF::vector< P3 >& out )
{
    const V3 u = c.axis.AsVector();
    const P3 base = c.apex + u * c.height;
    out.push_back( c.apex );
    for ( size_t i=0; i<directions.size(); ++i )
    {
        const V3 lateral = directions[ i ] - u * directions[ i ].Dot( u );
        out.push_back( lateral.GetLength() > 0 ? base + lateral.Normalized() * c.baseRadius : base );
    }
}

template < typename TShape >
static void
AddExtremes( const TShape&, const GUCEF::vector< V3 >&, GUCEF::vector< P3 >& )
{
}

/**
 *  A containment query against the minimum slack of a dense sampling of the inner shape in the outer one: the query
 *  must say yes when every sample is inside by more than 0.03, no when one is outside by more than 0.03.
 */
template < typename T, typename TOuter, typename TInner >
static bool
ContainmentSweep( const char* name, UInt64 seed, UInt32 count, Float64 growth )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 yes = 0;
    UInt32 no = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TOuter outer;
        TInner inner;
        RandomShape( random, outer );
        RandomShape( random, inner );
        Enlarge( outer, growth );
        const typename SWidened< TOuter >::Type outer64 = Widen( outer );
        const typename SWidened< TInner >::Type inner64 = Widen( inner );
        GUCEF::vector< P3 > samples;
        SampleShape( inner64, random, 400, samples );
        GUCEF::vector< V3 > normals;
        OuterNormals( outer64, inner64.GetBounds().GetCentre(), normals );
        AddExtremes( inner64, normals, samples );
        Float64 slack = 1.0e300;
        for ( size_t s=0; s<samples.size(); ++s )
            slack = fmin( slack, SlackIn( outer64, samples[ s ] ) );
        if ( fabs( slack ) < 0.03 )
            continue;
        const bool expected = slack > 0;
        const bool got = Contains( outer, inner );
        ++( expected ? yes : no );
        if ( got != expected )
        {
            ++bad;
            if ( bad < 4 )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Containment: " << name << " says " << got << " with slack " << slack << ", seed " << seed << " pair " << i );
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Containment: " << name << " seed " << seed << ": " << yes << " contained and " << no << " not contained checked, " << bad << " disagreements" );
    return 0 == bad && yes > count / 100 && no > count / 20;
}

/*-------------------------------------------------------------------------*/

/** The sphere and the box against the points of a sample: contained when all are, and the empty cases */
template < typename T >
static bool
PointSetSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 bad = 0;
    UInt32 yes = 0;
    UInt32 no = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        CTSphere< T > sphere;
        CTAxisAlignedBox3D< T > box;
        RandomShape( random, sphere );
        RandomShape( random, box );
        Enlarge( sphere, 1.6 );
        Enlarge( box, 1.6 );
        CTPoint3D< T > points[ 6 ];
        bool inSphere = true;
        bool inBox = true;
        const CTSphere< Float64 > sphere64 = Widen( sphere );
        const CTAxisAlignedBox3D< Float64 > box64 = Widen( box );
        const UInt32 n = (UInt32) random.Index( 7 );
        for ( UInt32 k=0; k<n; ++k )
        {
            const P3 q = random.Point( 1.5 );
            points[ k ] = ToP< T >( q );
            inSphere = inSphere && SlackIn( sphere64, Widen( points[ k ] ) ) > 0;
            inBox = inBox && SlackIn( box64, Widen( points[ k ] ) ) > 0;
        }
        bool clear = true;
        for ( UInt32 k=0; k<n; ++k )
            clear = clear && fabs( SlackIn( sphere64, Widen( points[ k ] ) ) ) > 1.0e-4 && fabs( SlackIn( box64, Widen( points[ k ] ) ) ) > 1.0e-4;
        if ( !clear )
            continue;
        if ( Contains( sphere, points, n ) != inSphere || Contains( box, points, n ) != inBox )
            ++bad;
        ++( inSphere ? yes : no );
    }
    return 0 == bad && yes > 10 && no > 10;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformContainmentTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Containment" );

    GUCEF_TESTFW_TESTCASE( "Containment: the axis aligned box contains spheres, oriented boxes, capsules, triangles and segments (Float32 and Float64)" )
    try
    {
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTAxisAlignedBox3D< Float32 >, CTSphere< Float32 > >( "aabb-sphere32", 901, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTAxisAlignedBox3D< Float64 >, CTSphere< Float64 > >( "aabb-sphere64", 902, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTAxisAlignedBox3D< Float32 >, CTOrientedBox3D< Float32 > >( "aabb-obb32", 903, 3000, 2.2 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTAxisAlignedBox3D< Float64 >, CTOrientedBox3D< Float64 > >( "aabb-obb64", 904, 3000, 2.2 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTAxisAlignedBox3D< Float32 >, CTCapsule< Float32 > >( "aabb-capsule32", 905, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTAxisAlignedBox3D< Float64 >, CTCapsule< Float64 > >( "aabb-capsule64", 906, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTAxisAlignedBox3D< Float32 >, CTTriangle3D< Float32 > >( "aabb-triangle32", 907, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTAxisAlignedBox3D< Float64 >, CTSegment3D< Float64 > >( "aabb-segment64", 908, 3000, 1.8 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Containment: the oriented box contains spheres, boxes, capsules, triangles and segments (Float32 and Float64)" )
    try
    {
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTOrientedBox3D< Float32 >, CTSphere< Float32 > >( "obb-sphere32", 911, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTOrientedBox3D< Float64 >, CTSphere< Float64 > >( "obb-sphere64", 912, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTOrientedBox3D< Float32 >, CTAxisAlignedBox3D< Float32 > >( "obb-aabb32", 913, 3000, 2.2 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTOrientedBox3D< Float64 >, CTOrientedBox3D< Float64 > >( "obb-obb64", 914, 3000, 2.2 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTOrientedBox3D< Float32 >, CTCapsule< Float32 > >( "obb-capsule32", 915, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTOrientedBox3D< Float64 >, CTTriangle3D< Float64 > >( "obb-triangle64", 916, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTOrientedBox3D< Float32 >, CTSegment3D< Float32 > >( "obb-segment32", 917, 3000, 1.8 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Containment: the sphere contains spheres, boxes and point sets (Float32 and Float64)" )
    try
    {
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTSphere< Float32 >, CTSphere< Float32 > >( "sphere-sphere32", 921, 3000, 2.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTSphere< Float64 >, CTSphere< Float64 > >( "sphere-sphere64", 922, 3000, 2.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTSphere< Float32 >, CTAxisAlignedBox3D< Float32 > >( "sphere-aabb32", 923, 3000, 2.2 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTSphere< Float64 >, CTOrientedBox3D< Float64 > >( "sphere-obb64", 924, 3000, 2.2 ) ) );
        ASSERT_TRUE( PointSetSweep< Float32 >( 925, 4000 ) );
        ASSERT_TRUE( PointSetSweep< Float64 >( 926, 4000 ) );
        const CTSphere< Float64 > sphere( P( 0, 0, 0 ), 1.0 );
        ASSERT_TRUE( Contains( sphere, sphere ) && !Contains( sphere, CTSphere< Float64 >( P( 0.1, 0, 0 ), 1.0 ) ) && !Contains( CTSphere< Float64 >( P( 0, 0, 0 ), -1.0 ), sphere ) );
        ASSERT_TRUE( Contains( sphere, CTSphere< Float64 >( P( 0.5, 0, 0 ), 0.5 ) ) && Contains( sphere, static_cast< const P* >( GUCEF_NULL ), 0 ) );
        ASSERT_FALSE( Contains( sphere, CTAxisAlignedBox3D< Float64 >() ) );
        const CTAxisAlignedBox3D< Float64 > box( P( -1, -1, -1 ), P( 1, 1, 1 ) );
        ASSERT_TRUE( Contains( box, box ) && Contains( box, CTSphere< Float64 >( P( 0, 0, 0 ), 1.0 ) ) && !Contains( box, CTSphere< Float64 >( P( 0, 0, 0 ), 1.0001 ) ) );
        ASSERT_FALSE( Contains( CTAxisAlignedBox3D< Float64 >(), box ) || Contains( CTAxisAlignedBox3D< Float64 >(), CTSphere< Float64 >( P( 0, 0, 0 ), 0.1 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Containment: the frustum contains spheres, boxes, oriented boxes, segments, triangles and capsules (Float32 and Float64)" )
    try
    {
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTFrustum< Float32 >, CTSphere< Float32 > >( "frustum-sphere32", 931, 3000, 1.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTFrustum< Float64 >, CTAxisAlignedBox3D< Float64 > >( "frustum-aabb64", 932, 3000, 1.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTFrustum< Float32 >, CTOrientedBox3D< Float32 > >( "frustum-obb32", 933, 3000, 1.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTFrustum< Float64 >, CTSegment3D< Float64 > >( "frustum-segment64", 934, 3000, 1.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTFrustum< Float32 >, CTTriangle3D< Float32 > >( "frustum-triangle32", 935, 3000, 1.0 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTFrustum< Float64 >, CTCapsule< Float64 > >( "frustum-capsule64", 936, 3000, 1.0 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Containment: the plane-bounded volume contains spheres, boxes, oriented boxes, segments, triangles, capsules, cylinders and cones (Float32 and Float64)" )
    try
    {
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTPlaneBoundedVolume< Float32 >, CTSphere< Float32 > >( "volume-sphere32", 941, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTPlaneBoundedVolume< Float64 >, CTAxisAlignedBox3D< Float64 > >( "volume-aabb64", 942, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTPlaneBoundedVolume< Float32 >, CTOrientedBox3D< Float32 > >( "volume-obb32", 943, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTPlaneBoundedVolume< Float64 >, CTSegment3D< Float64 > >( "volume-segment64", 944, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTPlaneBoundedVolume< Float32 >, CTTriangle3D< Float32 > >( "volume-triangle32", 945, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTPlaneBoundedVolume< Float64 >, CTCapsule< Float64 > >( "volume-capsule64", 946, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float32, CTPlaneBoundedVolume< Float32 >, CTCylinder< Float32 > >( "volume-cylinder32", 947, 3000, 1.8 ) ) );
        ASSERT_TRUE( ( ContainmentSweep< Float64, CTPlaneBoundedVolume< Float64 >, CTCone< Float64 > >( "volume-cone64", 948, 3000, 1.8 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
