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

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_GEOMETRY_SUPPORTMAPPINGS_H
#include "gucefGEOMETRY_SupportMappings.h"
#define GUCEF_GEOMETRY_SUPPORTMAPPINGS_H
#endif /* GUCEF_GEOMETRY_SUPPORTMAPPINGS_H ? */

#include "TestQueryReference.h"
#include "TestVolumeShapes.h"

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > static Float64 PositionTolerance( void ) { return sizeof( T ) < 8 ? 3.0e-5 : 1.0e-9; }

/*-------------------------------------------------------------------------*/

/**
 *  How far inside the solid the point is, by the definition of the shape written out with plain Float64 arithmetic and no
 *  shared code with the members: positive inside, negative outside, in length units ( up to a constant factor for the cone ).
 */
static Float64
ReferenceMargin( const CTOrientedBox3D< Float64 >& box, const P3& p )
{
    const Q4 inverse = box.rotation.Conjugate();
    const V3 local = inverse.Rotate( p - box.centre );
    return fmin( fmin( box.halfExtents.x - fabs( local.x ), box.halfExtents.y - fabs( local.y ) ), box.halfExtents.z - fabs( local.z ) );
}

static Float64
ReferenceMargin( const CTCylinder< Float64 >& cylinder, const P3& p )
{
    const V3 axis = cylinder.end - cylinder.start;
    const Float64 length = axis.GetLength();
    const V3 u = axis / length;
    const V3 w = p - cylinder.start;
    const Float64 s = w.x * u.x + w.y * u.y + w.z * u.z;
    const Float64 rho = sqrt( fmax( 0.0, w.x * w.x + w.y * w.y + w.z * w.z - s * s ) );
    return fmin( fmin( s, length - s ), cylinder.radius - rho );
}

static Float64
ReferenceMargin( const CTCone< Float64 >& cone, const P3& p )
{
    const V3 u = cone.axis.AsVector();
    const V3 w = p - cone.apex;
    const Float64 s = w.x * u.x + w.y * u.y + w.z * u.z;
    const Float64 rho = sqrt( fmax( 0.0, w.x * w.x + w.y * w.y + w.z * w.z - s * s ) );
    const Float64 slope = cone.baseRadius / cone.height;
    return fmin( fmin( s, cone.height - s ), ( slope * s - rho ) / sqrt( 1.0 + slope * slope ) );
}

/*-------------------------------------------------------------------------*/

/**
 *  Seeded sweeps of the uniform members of one shape kind: Contains against the reference definition, ClosestPoint by
 *  the characterisation of the projection on a convex set (a point of the shape such that no point of the shape lies
 *  beyond it, seen from the query point), support points as the extremes over samples, and bounds that contain the
 *  samples and touch the extremes. Returns the number of failed checks.
 */
template < typename T, typename TShape >
static UInt32
MembersSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = PositionTolerance< T >();
    UInt32 bad = 0;
    UInt32 insideCount = 0;
    UInt32 outsideCount = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TShape shape;
        RandomShape( random, shape );
        const typename SWidened< TShape >::Type shape64 = Widen( shape );
        GUCEF::vector< P3 > samples;
        SampleShape( shape64, random, 300, samples );

        for ( UInt32 k=0; k<10; ++k )
        {
            P3 p;
            if ( 0 == k % 2 )
                p = random.Point( 3.0 );
            else
            {
                const V3 jitter = random.Vector( 0.4 );
                p = samples[ (size_t) random.Index( (Int32) samples.size() ) ] + jitter;
            }
            const CTPoint3D< T > pT = ToP< T >( p );
            const P3 p64 = Widen( pT );

            /* Contains */
            const Float64 margin = ReferenceMargin( shape64, p64 );
            const bool contains = shape.Contains( pT );
            if ( fabs( margin ) > tolerance * 4.0 )
            {
                if ( contains != ( margin > 0 ) )
                    ++bad;
                ++( margin > 0 ? insideCount : outsideCount );
            }

            /* ClosestPoint: in the shape, the nearest of all, and the point itself inside */
            const CTPoint3D< T > closestT = shape.ClosestPoint( pT );
            const P3 closest = Widen( closestT );
            if ( ReferenceMargin( shape64, closest ) < -tolerance * 4.0 )
                ++bad;
            const V3 toPoint = p64 - closest;
            const Float64 distance = toPoint.GetLength();
            if ( !NearTo( (Float64) shape.GetDistance( pT ), distance, tolerance ) || !NearTo( (Float64) shape.GetSquaredDistance( pT ), distance * distance, tolerance * 4.0 ) )
                ++bad;
            if ( margin > tolerance * 4.0 && distance > tolerance )
                ++bad;
            for ( size_t s=0; s<samples.size(); ++s )
            {
                if ( ( samples[ s ] - closest ).Dot( toPoint ) > tolerance * ( 1.0 + distance ) * 4.0 )
                {
                    ++bad;
                    break;
                }
            }
        }

        /* the support point along a direction is in the shape and beyond every sample */
        for ( UInt32 k=0; k<4; ++k )
        {
            const D3 direction = random.Direction();
            const CTPoint3D< T > supportT = shape.GetSupport( ToV< T >( direction.AsVector() ) );
            const P3 support = Widen( supportT );
            if ( ReferenceMargin( shape64, support ) < -tolerance * 4.0 )
                ++bad;
            const Float64 reach = direction.AsVector().Dot( support.ToPositionVector() );
            for ( size_t s=0; s<samples.size(); ++s )
            {
                if ( direction.AsVector().Dot( samples[ s ].ToPositionVector() ) > reach + tolerance * 4.0 )
                {
                    ++bad;
                    break;
                }
            }
        }

        /* the bounds contain the samples and touch the shape on all six sides */
        const CTAxisAlignedBox3D< T > bounds = shape.GetBounds();
        const CTAxisAlignedBox3D< Float64 > bounds64 = Widen( bounds );
        for ( size_t s=0; s<samples.size(); ++s )
        {
            if ( !bounds64.Expanded( tolerance * 4.0 ).Contains( samples[ s ] ) )
            {
                ++bad;
                break;
            }
        }
        for ( UInt32 axis=0; axis<3; ++axis )
        {
            V3 unit;
            unit.SetComponent( axis, 1.0 );
            const P3 high = Widen( shape.GetSupport( ToV< T >( unit ) ) );
            const P3 low = Widen( shape.GetSupport( ToV< T >( unit * -1.0 ) ) );
            if ( !NearTo( bounds64.max.GetComponent( axis ), high.GetComponent( axis ), tolerance * 4.0 ) || !NearTo( bounds64.min.GetComponent( axis ), low.GetComponent( axis ), tolerance * 4.0 ) )
                ++bad;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "VolumeShapes: " << name << " members sweep seed " << seed << ": " << insideCount << " inside and " << outsideCount << " outside points checked, " << bad << " failed checks" );
    return bad + ( insideCount < count ? 1 : 0 ) + ( outsideCount < count ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

static void GetAxisOf( const CTCylinder< Float64 >& c, V3& axis );
static void GetAxisOf( const CTCone< Float64 >& c, V3& axis );

/**
 *  Directions nearly parallel to the axis ( and exactly along it ): the support point must stay on the surface and
 *  maximise the dot product, and the member must be the same function as the support mapping of the convex algorithms.
 */
template < typename T, typename TShape >
static UInt32
NearAxisSupportSweep( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    const Float64 tolerance = PositionTolerance< T >();
    UInt32 bad = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        TShape shape;
        RandomShape( random, shape );
        const typename SWidened< TShape >::Type shape64 = Widen( shape );
        V3 axis;
        GetAxisOf( shape64, axis );
        GUCEF::vector< P3 > samples;
        SampleShape( shape64, random, 300, samples );
        for ( UInt32 k=0; k<8; ++k )
        {
            const Float64 sign = 0 == k % 2 ? 1.0 : -1.0;
            const Float64 sizes[ 4 ] = { 0.0, 1.0e-8, 1.0e-5, 1.0e-2 };
            const Float64 tilt = sizes[ k / 2 ];
            const V3 side = RandomPerpendicular( random, axis );
            const V3 direction = axis * sign + side * tilt;
            const CTVector3D< T > directionT = ToV< T >( direction );
            const CTPoint3D< T > supportT = shape.GetSupport( directionT );
            const P3 support = Widen( supportT );
            const Float64 margin = ReferenceMargin( shape64, support );
            if ( fabs( margin ) > tolerance * 4.0 )
                ++bad;                                      /* off the surface: outside, or inside it */
            const Float64 reach = Widen( directionT ).Dot( support.ToPositionVector() );
            for ( size_t s=0; s<samples.size(); ++s )
            {
                if ( Widen( directionT ).Dot( samples[ s ].ToPositionVector() ) > reach + tolerance * 4.0 )
                {
                    ++bad;
                    break;
                }
            }
            if ( !( CTSupportMapping< TShape >::Make( shape ).GetSupport( directionT ) == supportT ) )
                ++bad;
        }
    }
    return bad;
}

static void GetAxisOf( const CTCylinder< Float64 >& c, V3& axis ) { axis = ( c.end - c.start ).Normalized(); }
static void GetAxisOf( const CTCone< Float64 >& c, V3& axis ) { axis = c.axis.AsVector(); }

/** The volume of the shape by counting random points of its bounds that the Contains member accepts, as a fraction of the volume of the bounds */
template < typename TShape >
static Float64
MonteCarloVolume( const TShape& shape, CRandom& random, UInt32 count )
{
    const CTAxisAlignedBox3D< Float64 > bounds = shape.GetBounds();
    UInt32 inside = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 x = random.Uniform( bounds.min.x, bounds.max.x );
        const Float64 y = random.Uniform( bounds.min.y, bounds.max.y );
        const Float64 z = random.Uniform( bounds.min.z, bounds.max.z );
        if ( shape.Contains( P3( x, y, z ) ) )
            ++inside;
    }
    return bounds.GetVolume() * (Float64) inside / (Float64) count;
}

/** The surface area as the thin shell: ( volume of the shape grown by d minus the volume ) / d for a small d, by counting */
template < typename TShape >
static Float64
ShellArea( const TShape& shape, CRandom& random, UInt32 count, Float64 thickness )
{
    const CTAxisAlignedBox3D< Float64 > bounds = shape.GetBounds().Expanded( thickness );
    UInt32 shell = 0;
    for ( UInt32 i=0; i<count; ++i )
    {
        const Float64 x = random.Uniform( bounds.min.x, bounds.max.x );
        const Float64 y = random.Uniform( bounds.min.y, bounds.max.y );
        const Float64 z = random.Uniform( bounds.min.z, bounds.max.z );
        const P3 p( x, y, z );
        const Float64 d = shape.GetDistance( p );
        if ( d > 0 && d <= thickness )
            ++shell;
    }
    return bounds.GetVolume() * (Float64) shell / (Float64) count / thickness;
}

/*-------------------------------------------------------------------------*/

/** Every point of the wireframe lies on the shape ( distance 0 ) */
template < typename TShape >
static bool
WireframeOnShape( const TShape& shape, UInt32 segmentsPerCircle, UInt32& segmentCount )
{
    GUCEF::vector< CTSegment3D< Float64 > > lines;
    shape.GetWireframeLines( lines, segmentsPerCircle );
    segmentCount = (UInt32) lines.size();
    for ( size_t i=0; i<lines.size(); ++i )
    {
        if ( shape.GetDistance( lines[ i ].start ) > 1.0e-9 || shape.GetDistance( lines[ i ].end ) > 1.0e-9 || shape.GetDistance( lines[ i ].GetMidpoint() ) > 1.0e-9 )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The text and DataNode forms: round trips, failures that leave the value unchanged */
template < typename TShape >
static bool
FormsRoundTrip( const TShape& shape, const char* garbage )
{
    CORE::CString text = shape.ToString();
    TShape fromText;
    if ( !fromText.FromString( text ) || !( fromText == shape ) )
        return false;
    TShape unchanged = shape;
    if ( unchanged.FromString( garbage ) || !( unchanged == shape ) || unchanged.FromString( "" ) || !( unchanged == shape ) )
        return false;
    CORE::CDataNodeSerializableSettings settings;
    CORE::CDataNode node;
    if ( !shape.Serialize( node, settings ) )
        return false;
    TShape fromNode;
    if ( !fromNode.Deserialize( node, settings ) || !( fromNode == shape ) )
        return false;
    CORE::CDataNode empty;
    if ( unchanged.Deserialize( empty, settings ) || !( unchanged == shape ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformVolumeShapesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "VolumeShapes" );

    typedef TFloat64Point3D P;
    typedef TFloat64Vector3D V;
    typedef TFloat64DirectionVector3D D;

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: tolerance defaults, scale, equality" )
    try
    {
        const CTGeometryTolerance< Float32 > t32;
        ASSERT_TRUE( t32.absolute == CTNumericTraits< Float32 >::DefaultAbsoluteTolerance() && t32.relative == CTNumericTraits< Float32 >::DefaultRelativeTolerance() );
        const CTGeometryTolerance< Float64 > t64( 1.0e-9, 1.0e-6, 1.0e-8 );
        ASSERT_TRUE( 1.0e-9 == t64.GetLengthTolerance( 0.0 ) && NearTo( t64.GetLengthTolerance( 1000.0 ), 1.0e-3, 1e-15 ) );
        ASSERT_TRUE( t64.GetLengthTolerance( -1000.0 ) == t64.GetLengthTolerance( 1000.0 ) );
        ASSERT_TRUE( t64.IsZeroLength( 5.0e-4, 1000.0 ) && !t64.IsZeroLength( 5.0e-3, 1000.0 ) && t64.IsZeroLength( -5.0e-4, 1000.0 ) );
        ASSERT_TRUE( t64.LengthsEqual( 1000.0, 1000.0005, 1000.0 ) && !t64.LengthsEqual( 1000.0, 1000.01, 1000.0 ) );
        ASSERT_TRUE( t64 == CTGeometryTolerance< Float64 >( 1.0e-9, 1.0e-6, 1.0e-8 ) && t64 != CTGeometryTolerance< Float64 >() );
        ASSERT_TRUE( CTGeometryTolerance< Float64 >::Default() == CTGeometryTolerance< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: oriented box members, 1000 seeded boxes in Float32 and Float64 against reference definitions and samples" )
    try
    {
        ASSERT_TRUE( 0 == ( MembersSweep< Float32, CTOrientedBox3D< Float32 > >( "obb32", 601, 1000 ) ) );
        ASSERT_TRUE( 0 == ( MembersSweep< Float64, CTOrientedBox3D< Float64 > >( "obb64", 602, 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: cylinder members, 1000 seeded cylinders in Float32 and Float64 against reference definitions and samples" )
    try
    {
        ASSERT_TRUE( 0 == ( MembersSweep< Float32, CTCylinder< Float32 > >( "cylinder32", 603, 1000 ) ) );
        ASSERT_TRUE( 0 == ( MembersSweep< Float64, CTCylinder< Float64 > >( "cylinder64", 604, 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: cone members, 1000 seeded cones in Float32 and Float64 against reference definitions and samples" )
    try
    {
        ASSERT_TRUE( 0 == ( MembersSweep< Float32, CTCone< Float32 > >( "cone32", 605, 1000 ) ) );
        ASSERT_TRUE( 0 == ( MembersSweep< Float64, CTCone< Float64 > >( "cone64", 606, 1000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: volumes and surface areas against closed forms, Monte Carlo counts and the shell of the shape" )
    try
    {
        CRandom random( 611 );
        const CTCylinder< Float64 > cylinder( P( 1, 0, 0 ), P( 1, 3, 0 ), 0.5 );
        ASSERT_TRUE( NearTo( cylinder.GetVolume(), 3.141592653589793 * 0.25 * 3.0, 1e-12 ) && NearTo( cylinder.GetSurfaceArea(), 2.0 * 3.141592653589793 * 0.5 * ( 0.5 + 3.0 ), 1e-12 ) );
        const CTCone< Float64 > cone( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 3.0, 2.0 );
        ASSERT_TRUE( NearTo( cone.GetVolume(), 3.141592653589793 * 4.0 * 3.0 / 3.0, 1e-12 ) && NearTo( cone.GetSurfaceArea(), 3.141592653589793 * 2.0 * ( 2.0 + sqrt( 13.0 ) ), 1e-12 ) );
        const CTOrientedBox3D< Float64 > box( P( 1, 2, 3 ), CTQuaternion< Float64 >::FromAxisAngle( D( V( 1, 1, 0 ) ), CTRadian< Float64 >( 0.7 ) ), V( 1, 2, 3 ) );
        ASSERT_TRUE( NearTo( box.GetVolume(), 48.0, 1e-12 ) && NearTo( box.GetSurfaceArea(), 2.0 * ( 2.0 * 4.0 + 4.0 * 6.0 + 6.0 * 2.0 ), 1e-12 ) );
        ASSERT_TRUE( NearTo( MonteCarloVolume( cylinder, random, 200000 ), cylinder.GetVolume(), 0.03 * cylinder.GetVolume() ) );
        ASSERT_TRUE( NearTo( MonteCarloVolume( cone, random, 200000 ), cone.GetVolume(), 0.03 * cone.GetVolume() ) );
        ASSERT_TRUE( NearTo( MonteCarloVolume( box, random, 200000 ), box.GetVolume(), 0.03 * box.GetVolume() ) );
        ASSERT_TRUE( NearTo( ShellArea( cylinder, random, 400000, 0.01 ), cylinder.GetSurfaceArea(), 0.05 * cylinder.GetSurfaceArea() ) );
        ASSERT_TRUE( NearTo( ShellArea( cone, random, 400000, 0.01 ), cone.GetSurfaceArea(), 0.06 * cone.GetSurfaceArea() ) );
        ASSERT_TRUE( NearTo( ShellArea( box, random, 400000, 0.01 ), box.GetSurfaceArea(), 0.05 * box.GetSurfaceArea() ) );

        /* invalid shapes have no measure */
        ASSERT_TRUE( 0.0 == CTCylinder< Float64 >( P( 0, 0, 0 ), P( 0, 0, 0 ), 1.0 ).GetVolume() && 0.0 == CTCylinder< Float64 >( P( 0, 0, 0 ), P( 0, 1, 0 ), -1.0 ).GetVolume() );
        ASSERT_TRUE( 0.0 == CTCone< Float64 >( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 0.0, 1.0 ).GetVolume() && 0.0 == CTCone< Float64 >( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 1.0, -1.0 ).GetSurfaceArea() );
        ASSERT_TRUE( 0.0 == CTOrientedBox3D< Float64 >( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( -1, 1, 1 ) ).GetVolume() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: degenerate and invalid shapes contain nothing, a zero radius is a segment, flat boxes are valid" )
    try
    {
        const CTCylinder< Float64 > zeroLength( P( 1, 1, 1 ), P( 1, 1, 1 ), 1.0 );
        ASSERT_FALSE( zeroLength.IsValid() || zeroLength.Contains( P( 1, 1, 1 ) ) );
        ASSERT_FALSE( CTCylinder< Float64 >( P( 0, 0, 0 ), P( 0, 1, 0 ), -0.5 ).Contains( P( 0, 0.5, 0 ) ) );
        const CTCylinder< Float64 > wire( P( 0, 0, 0 ), P( 0, 2, 0 ), 0.0 );
        ASSERT_TRUE( wire.IsValid() && wire.Contains( P( 0, 1, 0 ) ) && !wire.Contains( P( 0.001, 1, 0 ) ) && NearTo( wire.GetDistance( P( 3, 1, 0 ) ), 3.0, 1e-12 ) );
        ASSERT_TRUE( zeroLength.ClosestPoint( P( 4, 4, 4 ) ) == P( 1, 1, 1 ) );
        SRayCastResult< Float64 > hit;
        ASSERT_FALSE( wire.RayCast( CTRay3D< Float64 >( P( -3, 1, 0 ), D( V( 1, 0, 0 ) ) ), 10.0, hit ) );
        ASSERT_FALSE( zeroLength.RayCast( CTRay3D< Float64 >( P( -3, 1, 1 ), D( V( 1, 0, 0 ) ) ), 10.0, hit ) );

        const CTCone< Float64 > flatCone( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 0.0, 1.0 );
        ASSERT_FALSE( flatCone.IsValid() || flatCone.Contains( P( 0, 0, 0 ) ) );
        const CTCone< Float64 > needle( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 2.0, 0.0 );
        ASSERT_TRUE( needle.IsValid() && needle.Contains( P( 0, 0, 1 ) ) && !needle.Contains( P( 0.001, 0, 1 ) ) && NearTo( needle.GetDistance( P( 2, 0, 1 ) ), 2.0, 1e-12 ) );
        ASSERT_FALSE( needle.RayCast( CTRay3D< Float64 >( P( -3, 0, 1 ), D( V( 1, 0, 0 ) ) ), 10.0, hit ) );
        ASSERT_TRUE( NearTo( needle.GetDistance( P( 0, 0, 5 ) ), 3.0, 1e-12 ) );
        /* the apex and base centre points are in the cone, the point behind the apex is nearest to the apex */
        const CTCone< Float64 > cone( P( 0, 0, 0 ), D( V( 0, 0, 1 ) ), 2.0, 1.0 );
        ASSERT_TRUE( cone.Contains( P( 0, 0, 0 ) ) && cone.Contains( P( 0, 0, 2 ) ) && cone.Contains( P( 1, 0, 2 ) ) && !cone.Contains( P( 1, 0, 1.9 ) ) );
        ASSERT_TRUE( cone.ClosestPoint( P( 0, 0, -3 ) ) == P( 0, 0, 0 ) );
        ASSERT_TRUE( NearTo( cone.GetDistance( P( 0, 0, -3 ) ), 3.0, 1e-12 ) );

        const CTOrientedBox3D< Float64 > flat( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 0 ) );
        ASSERT_TRUE( flat.IsValid() && flat.Contains( P( 0.5, 0.5, 0 ) ) && !flat.Contains( P( 0.5, 0.5, 0.001 ) ) && 0.0 == flat.GetVolume() && NearTo( flat.GetSurfaceArea(), 8.0, 1e-12 ) );
        const CTOrientedBox3D< Float64 > invalid( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, -1, 1 ) );
        ASSERT_FALSE( invalid.IsValid() || invalid.Contains( P( 0, 0, 0 ) ) );
        ASSERT_FALSE( invalid.RayCast( CTRay3D< Float64 >( P( -3, 0, 0 ), D( V( 1, 0, 0 ) ) ), 10.0, hit ) );
        ASSERT_TRUE( invalid.GetBounds().IsEmpty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: oriented box axes, corners, frame conversion and construction from an axis aligned box" )
    try
    {
        CRandom random( 621 );
        for ( UInt32 i=0; i<500; ++i )
        {
            CTOrientedBox3D< Float64 > box;
            RandomShape( random, box );
            V3 axes[ 3 ];
            box.GetAxes( axes );
            ASSERT_TRUE( NearTo( axes[ 0 ].GetLength(), 1.0, 1e-12 ) && NearTo( axes[ 0 ].Dot( axes[ 1 ] ), 0.0, 1e-12 ) && NearTo( axes[ 1 ].Dot( axes[ 2 ] ), 0.0, 1e-12 ) );
            ASSERT_TRUE( NearTo( axes[ 0 ].Cross( axes[ 1 ] ).Dot( axes[ 2 ] ), 1.0, 1e-12 ) );
            P3 corners[ 8 ];
            box.GetCorners( corners );
            for ( UInt32 c=0; c<8; ++c )
            {
                ASSERT_TRUE( ( corners[ c ] - box.GetCorner( c ) ).GetLength() < 1e-12 );
                const V3 local = box.ToLocal( corners[ c ] );
                ASSERT_TRUE( NearTo( fabs( local.x ), box.halfExtents.x, 1e-12 ) && NearTo( fabs( local.y ), box.halfExtents.y, 1e-12 ) && NearTo( fabs( local.z ), box.halfExtents.z, 1e-12 ) );
                ASSERT_TRUE( ( local.x > 0 ) == ( 0 != ( c & 1 ) ) && ( local.y > 0 ) == ( 0 != ( c & 2 ) ) && ( local.z > 0 ) == ( 0 != ( c & 4 ) ) );
            }
            const P3 p = random.Point( 2.0 );
            ASSERT_TRUE( ( box.FromLocal( box.ToLocal( p ) ) - p ).GetLength() < 1e-12 );
        }
        const CTAxisAlignedBox3D< Float64 > aabb( P( -1, 0, 2 ), P( 3, 4, 8 ) );
        const CTOrientedBox3D< Float64 > converted = CTOrientedBox3D< Float64 >::FromAxisAlignedBox( aabb );
        ASSERT_TRUE( converted.centre == P( 1, 2, 5 ) && converted.halfExtents == V( 2, 2, 3 ) && converted.GetBounds().Equals( aabb ) );
        ASSERT_FALSE( CTOrientedBox3D< Float64 >::FromAxisAlignedBox( CTAxisAlignedBox3D< Float64 >() ).IsValid() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: transforms keep the solid: similarities for all, scales along the axes for the oriented box, nothing for shears" )
    try
    {
        CRandom random( 631 );
        typedef CTMatrix4D< Float64 > M;
        for ( UInt32 i=0; i<300; ++i )
        {
            const Q4 q = random.Rotation();
            const V3 translation = random.Vector( 3.0 );
            const Float64 scale = random.Uniform( 0.5, 2.0 );
            const M similarity = M::FromTRS( translation, q, V3( scale, scale, scale ) );
            CTOrientedBox3D< Float64 > box;
            CTCylinder< Float64 > cylinder;
            CTCone< Float64 > cone;
            RandomShape( random, box );
            RandomShape( random, cylinder );
            RandomShape( random, cone );
            CTOrientedBox3D< Float64 > movedBox;
            CTCylinder< Float64 > movedCylinder;
            CTCone< Float64 > movedCone;
            ASSERT_TRUE( box.TryTransformed( similarity, movedBox ) && cylinder.TryTransformed( similarity, movedCylinder ) && cone.TryTransformed( similarity, movedCone ) );
            ASSERT_TRUE( box.Transformed( similarity ) == movedBox );
            ASSERT_TRUE( NearTo( movedBox.GetVolume(), box.GetVolume() * scale * scale * scale, 1e-9 ) && NearTo( movedCylinder.GetVolume(), cylinder.GetVolume() * scale * scale * scale, 1e-9 ) && NearTo( movedCone.GetVolume(), cone.GetVolume() * scale * scale * scale, 1e-9 ) );
            for ( UInt32 k=0; k<20; ++k )
            {
                const P3 p = random.Point( 3.0 );
                const P3 moved = similarity.TransformPoint( p );
                if ( fabs( ReferenceMargin( box, p ) ) > 1e-6 )
                    ASSERT_TRUE( box.Contains( p ) == movedBox.Contains( moved ) );
                if ( fabs( ReferenceMargin( cylinder, p ) ) > 1e-6 )
                    ASSERT_TRUE( cylinder.Contains( p ) == movedCylinder.Contains( moved ) );
                if ( fabs( ReferenceMargin( cone, p ) ) > 1e-6 )
                    ASSERT_TRUE( cone.Contains( p ) == movedCone.Contains( moved ) );
            }

            /* a scale along the axes of the box: still a box; a reflection too */
            V3 axes[ 3 ];
            box.GetAxes( axes );
            const M axisScale = M::FromTRS( translation, Q4::Identity(), V3( 1, 1, 1 ) ) * M( CTMatrix3D< Float64 >::FromColumns( axes[ 0 ], axes[ 1 ], axes[ 2 ] ) ) *
                                M::Scale( V3( 2.0, 0.5, 1.5 ) ) * M( CTMatrix3D< Float64 >::FromColumns( axes[ 0 ], axes[ 1 ], axes[ 2 ] ).Transposed() );
            CTOrientedBox3D< Float64 > stretched;
            ASSERT_TRUE( box.TryTransformed( axisScale, stretched ) );
            ASSERT_TRUE( NearTo( stretched.GetVolume(), box.GetVolume() * 1.5, 1e-9 ) );
            for ( UInt32 k=0; k<20; ++k )
            {
                const P3 p = random.Point( 3.0 );
                if ( fabs( ReferenceMargin( box, p ) ) > 1e-6 )
                    ASSERT_TRUE( box.Contains( p ) == stretched.Contains( axisScale.TransformPoint( p ) ) );
            }
            const M mirror = M::Scale( V3( -1, 1, 1 ) );
            CTOrientedBox3D< Float64 > mirrored;
            ASSERT_TRUE( box.TryTransformed( mirror, mirrored ) && NearTo( mirrored.GetVolume(), box.GetVolume(), 1e-9 ) );
            ASSERT_TRUE( mirrored.Contains( mirror.TransformPoint( box.centre ) ) );
        }

        /* shears and flattening matrices have no exact image; Transformed then bounds the corners */
        const CTOrientedBox3D< Float64 > box( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( 1, 1, 1 ) );
        M shear;
        shear( 0, 1 ) = 0.5;
        CTOrientedBox3D< Float64 > out( P( 9, 9, 9 ), CTQuaternion< Float64 >::Identity(), V( 9, 9, 9 ) );
        const CTOrientedBox3D< Float64 > before = out;
        ASSERT_FALSE( box.TryTransformed( shear, out ) );
        ASSERT_TRUE( out == before );
        const CTOrientedBox3D< Float64 > bound = box.Transformed( shear );
        for ( UInt32 c=0; c<8; ++c )
            ASSERT_TRUE( bound.GetBounds().Expanded( 1e-9 ).Contains( shear.TransformPoint( box.GetCorner( c ) ) ) );
        ASSERT_FALSE( box.TryTransformed( M::Scale( V3( 1, 0, 1 ) ), out ) );
        const CTCylinder< Float64 > cylinder( P( 0, 0, 0 ), P( 0, 1, 0 ), 1.0 );
        CTCylinder< Float64 > outCylinder;
        ASSERT_FALSE( cylinder.TryTransformed( M::Scale( V3( 2, 1, 1 ) ), outCylinder ) );
        CTCone< Float64 > outCone;
        ASSERT_FALSE( CTCone< Float64 >().TryTransformed( M::Scale( V3( 2, 1, 1 ) ), outCone ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: wireframes lie on the shapes and have the segment counts of the outlines" )
    try
    {
        UInt32 segments = 0;
        const CTOrientedBox3D< Float64 > box( P( 1, 2, 3 ), CTQuaternion< Float64 >::FromAxisAngle( D( V( 0, 1, 1 ) ), CTRadian< Float64 >( 0.4 ) ), V( 1, 0.5, 2 ) );
        ASSERT_TRUE( WireframeOnShape( box, 8, segments ) && 12 == segments );
        const CTCylinder< Float64 > cylinder( P( 0, 0, 0 ), P( 1, 2, 3 ), 0.7 );
        ASSERT_TRUE( WireframeOnShape( cylinder, 12, segments ) && 2 * 12 + 4 == segments );
        ASSERT_TRUE( WireframeOnShape( cylinder, 1, segments ) && 2 * 3 + 4 == segments );
        const CTCone< Float64 > cone( P( 0, 0, 0 ), D( V( 1, 2, 3 ) ), 2.5, 0.9 );
        ASSERT_TRUE( WireframeOnShape( cone, 10, segments ) && 10 + 4 == segments );
        ASSERT_TRUE( WireframeOnShape( cone, 0, segments ) && 3 + 4 == segments );
        GUCEF::vector< CTSegment3D< Float64 > > none;
        CTOrientedBox3D< Float64 >( P( 0, 0, 0 ), CTQuaternion< Float64 >::Identity(), V( -1, 1, 1 ) ).GetWireframeLines( none, 8 );
        ASSERT_TRUE( none.empty() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "VolumeShapes: text and DataNode forms round trip, and failures leave the value unchanged" )
    try
    {
        CRandom random( 641 );
        for ( UInt32 i=0; i<50; ++i )
        {
            CTOrientedBox3D< Float64 > box;
            CTCylinder< Float64 > cylinder;
            CTCone< Float64 > cone;
            RandomShape( random, box );
            RandomShape( random, cylinder );
            RandomShape( random, cone );
            ASSERT_TRUE( FormsRoundTrip( box, "OrientedBox(centre (1, 2, 3), rotation (0, 0, 0, 1))" ) );
            ASSERT_TRUE( FormsRoundTrip( cylinder, "Cylinder(start (1, 2, 3), end (4, 5, 6))" ) );
            ASSERT_TRUE( FormsRoundTrip( cone, "Cone(apex (1, 2, 3), axis (0, 0, 2), height 1, radius 1)" ) );     /* the axis must be a unit direction */
            CTOrientedBox3D< Float32 > box32;
            RandomShape( random, box32 );
            ASSERT_TRUE( FormsRoundTrip( box32, "OrientedBox()" ) );
        }
        CTCylinder< Float64 > parsed;
        ASSERT_TRUE( parsed.FromString( " Cylinder( start ( 0 , 0 , 0 ) , end ( 0 , 1 , 0 ) , radius 2.5 ) " ) && parsed.radius == 2.5 && parsed.end.y == 1.0 );
        ASSERT_TRUE( CORE::CString( "Cylinder(start (0, 0, 0), end (0, 1, 0), radius 2.5)" ) == parsed.ToString() );
        ASSERT_TRUE( CORE::CString( "Cone(apex (0, 0, 0), axis (0, 1, 0), height 2, radius 1)" ) == CTCone< Float64 >( P( 0, 0, 0 ), D( V( 0, 1, 0 ) ), 2.0, 1.0 ).ToString() );
        ASSERT_TRUE( parsed.Equals( parsed ) && !parsed.Equals( CTCylinder< Float64 >() ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
    GUCEF_TESTFW_TESTCASE( "VolumeShapes: cylinder and cone support points stay on the surface and are the support mappings, for directions along and nearly along the axis" )
    try
    {
        ASSERT_TRUE( 0 == ( NearAxisSupportSweep< Float32, CTCylinder< Float32 > >( 651, 1500 ) ) );
        ASSERT_TRUE( 0 == ( NearAxisSupportSweep< Float64, CTCylinder< Float64 > >( 652, 1500 ) ) );
        ASSERT_TRUE( 0 == ( NearAxisSupportSweep< Float32, CTCone< Float32 > >( 653, 1500 ) ) );
        ASSERT_TRUE( 0 == ( NearAxisSupportSweep< Float64, CTCone< Float64 > >( 654, 1500 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
