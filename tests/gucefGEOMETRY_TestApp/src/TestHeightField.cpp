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

#ifndef GUCEF_GEOMETRY_CTHEIGHTFIELD_H
#include "gucefGEOMETRY_CTHeightField.h"
#define GUCEF_GEOMETRY_CTHEIGHTFIELD_H
#endif /* GUCEF_GEOMETRY_CTHEIGHTFIELD_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestHeightField.h"

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
using namespace SpatialReference;
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The heightfield and its brute force twin: the triangle list of the same heights built here from the stored numbers */
template < typename T >
struct SField
{
    GUCEF::vector< T > heights;
    UInt32 width;
    UInt32 depth;
    CTPoint3D< T > origin;
    T cellX;
    T cellZ;
    CTHeightField< T > field;
    GUCEF::vector< CTTriangle3D< T > > triangles;

    CTPoint3D< T > Vertex( UInt32 i, UInt32 j ) const
    {
        return CTPoint3D< T >( origin.x + (T) i * cellX, origin.y + heights[ (size_t) j * width + i ], origin.z + (T) j * cellZ );
    }
};

template < typename T >
static void
BuildField( CRandom& random, UInt32 width, UInt32 depth, SField< T >& f, bool rough )
{
    f.width = width;
    f.depth = depth;
    f.origin = CTPoint3D< T >( (T) random.Uniform( -5.0, 5.0 ), (T) random.Uniform( -2.0, 2.0 ), (T) random.Uniform( -5.0, 5.0 ) );
    f.cellX = (T) random.Uniform( 0.4, 1.7 );
    f.cellZ = (T) random.Uniform( 0.4, 1.7 );
    f.heights.clear();
    // smooth hills plus noise
    const Float64 fx = random.Uniform( 0.1, 0.5 );
    const Float64 fz = random.Uniform( 0.1, 0.5 );
    for ( UInt32 j=0; j<depth; ++j )
        for ( UInt32 i=0; i<width; ++i )
            f.heights.push_back( (T) ( 3.0 * sin( fx * i ) * cos( fz * j ) + ( rough ? random.Uniform( -1.5, 1.5 ) : 0.0 ) ) );
    f.field.Set( &f.heights[ 0 ], width, depth, f.origin, f.cellX, f.cellZ );
    // the triangles of every cell: ( i, j ), ( i, j + 1 ), ( i + 1, j + 1 ) and ( i, j ), ( i + 1, j + 1 ), ( i + 1, j )
    f.triangles.clear();
    for ( UInt32 j=0; j+1<depth; ++j )
        for ( UInt32 i=0; i+1<width; ++i )
        {
            f.triangles.push_back( CTTriangle3D< T >( f.Vertex( i, j ), f.Vertex( i, j + 1 ), f.Vertex( i + 1, j + 1 ) ) );
            f.triangles.push_back( CTTriangle3D< T >( f.Vertex( i, j ), f.Vertex( i + 1, j + 1 ), f.Vertex( i + 1, j ) ) );
        }
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
RayCastSweep( const char* name, UInt64 seed, UInt32 rays, bool rough )
{
    CRandom random( seed );
    SField< T > f;
    BuildField( random, 41, 37, f, rough );
    const CTAxisAlignedBox3D< T > bounds = f.field.GetBounds();
    UInt32 hits = 0;
    UInt32 misses = 0;
    for ( UInt32 n=0; n<rays; ++n )
    {
        // origins above, inside and beside the field, directions mostly downward
        const T ox = (T) random.Uniform( (Float64) bounds.min.x - 4.0, (Float64) bounds.max.x + 4.0 );
        const T oy = (T) random.Uniform( (Float64) bounds.min.y - 2.0, (Float64) bounds.max.y + 8.0 );
        const T oz = (T) random.Uniform( (Float64) bounds.min.z - 4.0, (Float64) bounds.max.z + 4.0 );
        P3 direction = P3( 0, 0, 0 );
        if ( random.Chance( 0.04 ) )
            direction = P3( 0, -1, 0 );                                  // straight down
        else
            direction = P3( random.Uniform( -1.0, 1.0 ), random.Uniform( -1.0, 0.3 ), random.Uniform( -1.0, 1.0 ) );
        D3 unit;
        if ( !D3::TryCreate( V3( direction.x, direction.y, direction.z ), unit ) )
            continue;
        CTRay3D< T > ray( CTPoint3D< T >( ox, oy, oz ), CTDirectionVector3D< T >( CTVector3D< T >( (T) unit.AsVector().x, (T) unit.AsVector().y, (T) unit.AsVector().z ) ) );
        const T maxDistance = random.Chance( 0.2 ) ? (T) random.Uniform( 1.0, 15.0 ) : (T) 1000.0;
        SRayCastResult< T > hit;
        const bool found = f.field.RayCast( ray, maxDistance, hit );
        // brute force: the closest hit over all triangles with the same triangle routine
        bool expectedFound = false;
        SRayCastResult< T > expected;
        Int32 expectedIndex = -1;
        for ( size_t t=0; t<f.triangles.size(); ++t )
        {
            SRayCastResult< T > candidate;
            if ( f.triangles[ t ].RayCast( ray, maxDistance, candidate ) && ( !expectedFound || candidate.distance < expected.distance ) )
            {
                expectedFound = true;
                expected = candidate;
                expectedIndex = (Int32) t;
            }
        }
        if ( found != expectedFound )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the heightfield " << ( found ? "hits" : "misses" ) << " and the mesh " << ( expectedFound ? "hits" : "misses" ) << ", seed " << seed << " ray " << n );
            return false;
        }
        if ( found )
        {
            ++hits;
            if ( hit.distance != expected.distance )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": hit distance " << hit.distance << " against " << expected.distance << ", seed " << seed << " ray " << n );
                return false;
            }
            // the same triangle unless two triangles are hit at the same distance
            if ( hit.featureIndex != expectedIndex )
            {
                SRayCastResult< T > other;
                if ( !f.triangles[ (size_t) hit.featureIndex ].RayCast( ray, maxDistance, other ) || other.distance != expected.distance )
                    return false;
            }
        }
        else
            ++misses;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "HeightField: " << name << ": " << rays << " rays against " << f.triangles.size() << " triangles, " << hits << " hits and " << misses << " misses equal the mesh" );
    return hits > 0 && misses > 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
ClosestPointSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    SField< T > f;
    BuildField( random, 29, 33, f, true );
    const CTAxisAlignedBox3D< T > bounds = f.field.GetBounds();
    for ( UInt32 n=0; n<count; ++n )
    {
        const CTPoint3D< T > p( (T) random.Uniform( (Float64) bounds.min.x - 6.0, (Float64) bounds.max.x + 6.0 ), (T) random.Uniform( (Float64) bounds.min.y - 6.0, (Float64) bounds.max.y + 6.0 ), (T) random.Uniform( (Float64) bounds.min.z - 6.0, (Float64) bounds.max.z + 6.0 ) );
        const CTPoint3D< T > c = f.field.ClosestPoint( p );
        T best = CTNumericTraits< T >::Max();
        for ( size_t t=0; t<f.triangles.size(); ++t )
        {
            const T d = ( p - f.triangles[ t ].ClosestPoint( p ) ).GetSquaredLength();
            best = d < best ? d : best;
        }
        const T got = ( p - c ).GetSquaredLength();
        if ( got != best )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": squared distance " << got << " against " << best << ", seed " << seed << " point " << n );
            return false;
        }
        if ( f.field.GetSquaredDistance( p ) != best )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Heights at the vertices exact; between them the barycentric interpolation of the triangle that contains the position */
template < typename T >
static bool
HeightSweep( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    SField< T > f;
    BuildField( random, 23, 19, f, true );
    // every vertex: the exact stored height plus the origin
    for ( UInt32 j=0; j<f.depth; ++j )
        for ( UInt32 i=0; i<f.width; ++i )
        {
            T h = (T) 0;
            const CTPoint3D< T > v = f.Vertex( i, j );
            if ( !f.field.TryGetHeight( v.x, v.z, h ) || h != v.y )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the height at vertex ( " << i << ", " << j << " ) is " << h << " instead of " << v.y );
                return false;
            }
        }
    const CTAxisAlignedBox3D< T > bounds = f.field.GetBounds();
    for ( UInt32 n=0; n<count; ++n )
    {
        const Float64 x = random.Uniform( (Float64) bounds.min.x, (Float64) bounds.max.x );
        const Float64 z = random.Uniform( (Float64) bounds.min.z, (Float64) bounds.max.z );
        T h;
        if ( !f.field.TryGetHeight( (T) x, (T) z, h ) )
            return false;
        // the triangle that contains the position by the plane through its vertices, in Float64
        Float64 expected = 0.0;
        bool found = false;
        for ( size_t t=0; t<f.triangles.size() && !found; ++t )
        {
            const CTTriangle3D< T >& tri = f.triangles[ t ];
            const Float64 ax = (Float64) tri.a.x, az = (Float64) tri.a.z;
            const Float64 bx = (Float64) tri.b.x, bz = (Float64) tri.b.z;
            const Float64 cx = (Float64) tri.c.x, cz = (Float64) tri.c.z;
            const Float64 det = ( bz - cz ) * ( ax - cx ) + ( cx - bx ) * ( az - cz );
            const Float64 l1 = ( ( bz - cz ) * ( (Float64) (T) x - cx ) + ( cx - bx ) * ( (Float64) (T) z - cz ) ) / det;
            const Float64 l2 = ( ( cz - az ) * ( (Float64) (T) x - cx ) + ( ax - cx ) * ( (Float64) (T) z - cz ) ) / det;
            const Float64 l3 = 1.0 - l1 - l2;
            if ( l1 >= -1.0e-9 && l2 >= -1.0e-9 && l3 >= -1.0e-9 )
            {
                expected = l1 * (Float64) tri.a.y + l2 * (Float64) tri.b.y + l3 * (Float64) tri.c.y;
                found = true;
            }
        }
        const Float64 tolerance = 1.0e-5 * ( sizeof( T ) == 4 ? 1.0 : 1.0e-6 ) + 1.0e-9;
        if ( !found || fabs( (Float64) h - expected ) > tolerance )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": interpolated height " << h << " against " << expected << ", seed " << seed << " point " << n );
            return false;
        }
    }
    // outside the footprint there is no height
    T h;
    if ( f.field.TryGetHeight( bounds.min.x - (T) 0.5, bounds.min.z, h ) || f.field.TryGetHeight( bounds.min.x, bounds.max.z + (T) 0.5, h ) )
        return false;
    CTDirectionVector3D< T > normal;
    if ( !f.field.TryGetNormal( (T) ( 0.5 * ( (Float64) bounds.min.x + (Float64) bounds.max.x ) ), (T) ( 0.5 * ( (Float64) bounds.min.z + (Float64) bounds.max.z ) ), normal ) || !( normal.AsVector().y > (T) 0 ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
SpecialCases( void )
{
    // too small or invalid
    CTHeightField< Float64 > field;
    const Float64 one[ 2 ] = { 0, 0 };
    if ( field.Set( one, 2, 1, CTPoint3D< Float64 >(), 1.0, 1.0 ) || !field.IsEmpty() )
        return false;
    const Float64 four[ 4 ] = { 0, 1, 2, 3 };
    if ( field.Set( four, 2, 2, CTPoint3D< Float64 >(), 0.0, 1.0 ) )
        return false;
    CTRay3D< Float64 > down( CTPoint3D< Float64 >( 0.25, 10, 0.25 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 0, -1, 0 ) ) );
    SRayCastResult< Float64 > hit;
    if ( field.RayCast( down, 100.0, hit ) )
        return false;
    // a flat 2 by 2: the ray down hits at the height, the closest point is the projection
    const Float64 flat[ 4 ] = { 2, 2, 2, 2 };
    if ( !field.Set( flat, 2, 2, CTPoint3D< Float64 >(), 1.0, 1.0 ) )
        return false;
    if ( !field.RayCast( down, 100.0, hit ) || fabs( hit.distance - 8.0 ) > 1.0e-12 || hit.normal.y <= 0.0 )
        return false;
    // beyond the maximum distance
    if ( field.RayCast( down, 7.0, hit ) )
        return false;
    // a ray parallel to the surface and above it
    CTRay3D< Float64 > level( CTPoint3D< Float64 >( -1, 3, 0.5 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 1, 0, 0 ) ) );
    if ( field.RayCast( level, 100.0, hit ) )
        return false;
    // a ray along the surface just inside the footprint: it lies in the plane of the triangles, no proper hit is required, only no crash
    CTRay3D< Float64 > inside( CTPoint3D< Float64 >( -1, 2, 0.5 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 1, 0, 0 ) ) );
    field.RayCast( inside, 100.0, hit );
    const CTPoint3D< Float64 > c = field.ClosestPoint( CTPoint3D< Float64 >( 0.5, 7, 0.5 ) );
    if ( fabs( c.y - 2.0 ) > 1.0e-12 || fabs( c.x - 0.5 ) > 1.0e-12 )
        return false;
    // an external array, read through the pointer and refitted after a change
    GUCEF::vector< Float64 > heights;
    for ( UInt32 i=0; i<100; ++i )
        heights.push_back( 0.0 );
    CTHeightField< Float64 > external;
    if ( !external.SetExternal( &heights[ 0 ], 10, 10, CTPoint3D< Float64 >(), 1.0, 1.0 ) )
        return false;
    CTRay3D< Float64 > probe( CTPoint3D< Float64 >( 5.5, 10, 5.5 ), CTDirectionVector3D< Float64 >( CTVector3D< Float64 >( 0, -1, 0 ) ) );
    if ( !external.RayCast( probe, 100.0, hit ) || fabs( hit.distance - 10.0 ) > 1.0e-12 )
        return false;
    for ( UInt32 i=0; i<100; ++i )
        heights[ i ] = 3.0;
    external.Refit();
    if ( !external.RayCast( probe, 100.0, hit ) || fabs( hit.distance - 7.0 ) > 1.0e-12 )
        return false;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformHeightFieldTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "HeightField" );

    GUCEF_TESTFW_TESTCASE( "HeightField: ray casts equal the closest hit over all triangles of the mesh built from the same heights" )
    try
    {
        ASSERT_TRUE( ( RayCastSweep< Float64 >( "Float64 smooth", 8101, ScaledCount( 10000 ), false ) ) );
        ASSERT_TRUE( ( RayCastSweep< Float64 >( "Float64 rough", 8102, ScaledCount( 10000 ), true ) ) );
        ASSERT_TRUE( ( RayCastSweep< Float32 >( "Float32 rough", 8103, ScaledCount( 10000 ), true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "HeightField: closest points equal the closest point over all triangles" )
    try
    {
        ASSERT_TRUE( ( ClosestPointSweep< Float64 >( "Float64", 8104, ScaledCount( 3000 ) ) ) );
        ASSERT_TRUE( ( ClosestPointSweep< Float32 >( "Float32", 8105, ScaledCount( 3000 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "HeightField: heights at the cell corners are exact, between them the interpolation of the triangle; no height outside" )
    try
    {
        ASSERT_TRUE( ( HeightSweep< Float64 >( "Float64", 8106, ScaledCount( 3000 ) ) ) );
        ASSERT_TRUE( ( HeightSweep< Float32 >( "Float32", 8107, ScaledCount( 3000 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "HeightField: invalid input, a flat field, a ray beyond the maximum distance, an external array with Refit" )
    try
    {
        ASSERT_TRUE( SpecialCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
