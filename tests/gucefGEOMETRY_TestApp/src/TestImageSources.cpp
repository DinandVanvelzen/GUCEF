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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_GEOSPACES_H
#include "gucefGEOMETRY_geospaces.h"
#define GUCEF_GEOMETRY_GEOSPACES_H
#endif /* GUCEF_GEOMETRY_GEOSPACES_H ? */

#ifndef GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#include "gucefGEOMETRY_CTImageSourceModel.h"
#define GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H
#endif /* GUCEF_GEOMETRY_CTIMAGESOURCEMODEL_H ? */

#include "TestGeometryReference.h"
#include "TestConvexReference.h"
#include "TestImageSources.h"

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
using ConvexReference::ScaledCount;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A Float64 triple for the reference computations */
struct SRef3
{
    Float64 v[ 3 ];

    SRef3( void ) { v[ 0 ] = v[ 1 ] = v[ 2 ] = 0.0; }
    SRef3( Float64 x, Float64 y, Float64 z ) { v[ 0 ] = x; v[ 1 ] = y; v[ 2 ] = z; }

    template < typename T >
    explicit SRef3( const CTPoint3D< T >& p ) { v[ 0 ] = (Float64) p.x; v[ 1 ] = (Float64) p.y; v[ 2 ] = (Float64) p.z; }

    template < typename T >
    explicit SRef3( const CTVector3D< T >& p ) { v[ 0 ] = (Float64) p.x; v[ 1 ] = (Float64) p.y; v[ 2 ] = (Float64) p.z; }
};

static Float64
Distance( const SRef3& a, const SRef3& b )
{
    const Float64 dx = a.v[ 0 ] - b.v[ 0 ];
    const Float64 dy = a.v[ 1 ] - b.v[ 1 ];
    const Float64 dz = a.v[ 2 ] - b.v[ 2 ];
    return sqrt( dx * dx + dy * dy + dz * dz );
}

/*-------------------------------------------------------------------------*/

/** The reference room: the order of the walls of AddShoebox is -X, +X, -Y, +Y, -Z, +Z */
struct SRefRoom
{
    Float64 lo[ 3 ];
    Float64 hi[ 3 ];

    /** The mirror image of 'p' in wall 'wall', one coordinate at a time */
    SRef3 Mirror( const SRef3& p, UInt32 wall ) const
    {
        SRef3 result = p;
        const UInt32 axis = wall / 2;
        const Float64 plane = 0 == ( wall & 1 ) ? lo[ axis ] : hi[ axis ];
        result.v[ axis ] = 2.0 * plane - p.v[ axis ];
        return result;
    }

    /** The unit normal component of the wall that points into the room */
    SRef3 Normal( UInt32 wall ) const
    {
        SRef3 n;
        n.v[ wall / 2 ] = 0 == ( wall & 1 ) ? 1.0 : -1.0;
        return n;
    }
};

/*-------------------------------------------------------------------------*/

static SRef3
Reflect( const SRef3& d, const SRef3& n )
{
    const Float64 k = 2.0 * ( d.v[ 0 ] * n.v[ 0 ] + d.v[ 1 ] * n.v[ 1 ] + d.v[ 2 ] * n.v[ 2 ] );
    return SRef3( d.v[ 0 ] - k * n.v[ 0 ], d.v[ 1 ] - k * n.v[ 1 ], d.v[ 2 ] - k * n.v[ 2 ] );
}

/*-------------------------------------------------------------------------*/

static SRef3
Unit( const SRef3& from, const SRef3& to )
{
    const Float64 d = Distance( from, to );
    return SRef3( ( to.v[ 0 ] - from.v[ 0 ] ) / d, ( to.v[ 1 ] - from.v[ 1 ] ) / d, ( to.v[ 2 ] - from.v[ 2 ] ) / d );
}

/*-------------------------------------------------------------------------*/

/** The images of the room that the tests expect: 6 of the first order, 30 ordered pairs of the second order */
struct SRefImages
{
    GUCEF::vector< SRef3 > first;
    GUCEF::vector< SRef3 > ordered;                 /**< second order, by ordered pair ( a, b ) with a != b */
    GUCEF::vector< UInt32 > orderedFirst;
    GUCEF::vector< UInt32 > orderedLast;
    GUCEF::vector< SRef3 > distinct;                /**< the second order images without duplicates */

    SRefImages( const SRefRoom& room, const SRef3& source ) : first(), ordered(), orderedFirst(), orderedLast(), distinct()
    {
        for ( UInt32 a=0; a<6; ++a )
            first.push_back( room.Mirror( source, a ) );
        for ( UInt32 a=0; a<6; ++a )
        {
            for ( UInt32 b=0; b<6; ++b )
            {
                if ( a == b )
                    continue;
                const SRef3 image = room.Mirror( room.Mirror( source, a ), b );
                ordered.push_back( image );
                orderedFirst.push_back( a );
                orderedLast.push_back( b );
                bool known = false;
                for ( size_t i=0; i<distinct.size() && !known; ++i )
                    known = Distance( distinct[ i ], image ) < 1e-9;
                if ( !known )
                    distinct.push_back( image );
            }
        }
    }
};

/*-------------------------------------------------------------------------*/

template < typename T >
static void
BuildRoom( CTImageSourceModel< T >& model, const SRefRoom& room )
{
    UInt32 first = 0;
    model.AddShoebox( CTPoint3D< T >( (T) room.lo[ 0 ], (T) room.lo[ 1 ], (T) room.lo[ 2 ] ), CTPoint3D< T >( (T) room.hi[ 0 ], (T) room.hi[ 1 ], (T) room.hi[ 2 ] ), 100, first );
}

/*-------------------------------------------------------------------------*/

static bool
LogReason( int line )
{
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: room path check failed at line " << line );
    return false;
}

/*-------------------------------------------------------------------------*/

/** The reflection law and the geometry of one path of a room, from independent Float64 arithmetic; false with a reason in the log */
template < typename T >
static bool
CheckRoomPath( const SRefRoom& room, const SRef3& source, const SRef3& listener, const SImageSourcePath< T >& path, Float64 tolerance )
{
    if ( path.order < 1 || path.order > 2 )
        return LogReason( __LINE__ );
    SRef3 points[ 4 ];
    UInt32 walls[ 2 ] = { 0, 0 };
    points[ 0 ] = listener;
    // reflectionPoint[ 0 ] is met first from the source, so the listener meets reflectionPoint[ order - 1 ] first
    for ( UInt32 k=0; k<path.order; ++k )
    {
        points[ 1 + k ] = SRef3( path.reflectionPoint[ path.order - 1 - k ] );
        walls[ k ] = path.wallIndex[ path.order - 1 - k ];
        if ( walls[ k ] > 5 || path.wallUserId[ path.order - 1 - k ] != 100 + walls[ k ] )
            return LogReason( __LINE__ );
        // on its wall, inside the room's face
        const UInt32 axis = walls[ k ] / 2;
        const Float64 plane = 0 == ( walls[ k ] & 1 ) ? room.lo[ axis ] : room.hi[ axis ];
        if ( fabs( points[ 1 + k ].v[ axis ] - plane ) > tolerance )
            return LogReason( __LINE__ );
        for ( UInt32 other=0; other<3; ++other )
        {
            // the model widens the wall polygons by its point tolerance of 1e-4: a path past the edge of two walls may be that far outside
            if ( other != axis && ( points[ 1 + k ].v[ other ] < room.lo[ other ] - tolerance - 2.0e-4 || points[ 1 + k ].v[ other ] > room.hi[ other ] + tolerance + 2.0e-4 ) )
                return LogReason( __LINE__ );
        }
    }
    points[ 1 + path.order ] = source;

    // the reflection law at every point
    Float64 length = 0.0;
    for ( UInt32 k=0; k<path.order; ++k )
    {
        // the two reflection points of a path through the edge of two walls coincide: the direction between them is undefined
        if ( Distance( points[ k ], points[ k + 1 ] ) < 1.0e-3 || Distance( points[ k + 1 ], points[ k + 2 ] ) < 1.0e-3 )
            continue;
        const SRef3 incoming = Unit( points[ k ], points[ k + 1 ] );
        const SRef3 outgoing = Unit( points[ k + 1 ], points[ k + 2 ] );
        const SRef3 expected = Reflect( incoming, room.Normal( walls[ k ] ) );
        if ( Distance( expected, outgoing ) > tolerance * 10.0 )
            return LogReason( __LINE__ );
    }
    Float64 slack = 0.0;
    for ( UInt32 k=0; k<path.order + 1; ++k )
    {
        length += Distance( points[ k ], points[ k + 1 ] );
        if ( k > 0 && k < path.order && Distance( points[ k ], points[ k + 1 ] ) < 1.0e-3 )
            slack = 1.0e-3;         // an edge path: the points are moved onto their walls by up to the point tolerance
    }
    if ( fabs( length - (Float64) path.pathLength ) > tolerance * 10.0 * ( 1.0 + length ) + slack )
        return LogReason( __LINE__ );
    if ( fabs( Distance( SRef3( path.position ), listener ) - (Float64) path.pathLength ) > tolerance * 10.0 * ( 1.0 + length ) )
        return LogReason( __LINE__ );
    // the arrival direction points from the listener to the first point of the path
    const SRef3 arrival = Unit( listener, points[ 1 ] );
    return Distance( arrival, SRef3( path.arrivalDirection ) ) <= tolerance * 10.0;
}

/*-------------------------------------------------------------------------*/

/** The paths of a room found equal the analytic ones: 6 images, 18 distinct second order images each found once, every path obeys the reflection law */
template < typename T >
static bool
RoomPaths( const SRefRoom& room, const SRef3& source, const SRef3& listener, Float64 tolerance, const char* name )
{
    CTImageSourceModel< T > model;
    BuildRoom( model, room );
    if ( 6 != model.GetWallCount() )
        return false;
    const CTPoint3D< T > s( (T) source.v[ 0 ], (T) source.v[ 1 ], (T) source.v[ 2 ] );
    const CTPoint3D< T > l( (T) listener.v[ 0 ], (T) listener.v[ 1 ], (T) listener.v[ 2 ] );
    const SRefImages reference( room, source );
    if ( 6 != reference.first.size() || 30 != reference.ordered.size() || 18 != reference.distinct.size() )
        return false;

    // the unvalidated images: 6, then 30 ordered pairs, equal to the reference one by one
    GUCEF::vector< SImageSourcePath< T > > images;
    if ( 36 != model.BuildImages( s, 2, images ) || 36 != images.size() )
        return false;
    for ( UInt32 i=0; i<6; ++i )
    {
        if ( 1 != images[ i ].order || Distance( SRef3( images[ i ].position ), reference.first[ i ] ) > tolerance )
            return false;
    }
    for ( UInt32 i=0; i<30; ++i )
    {
        const SImageSourcePath< T >& image = images[ 6 + i ];
        if ( 2 != image.order || image.wallIndex[ 0 ] != reference.orderedFirst[ i ] || image.wallIndex[ 1 ] != reference.orderedLast[ i ] || Distance( SRef3( image.position ), reference.ordered[ i ] ) > tolerance )
            return false;
    }
    GUCEF::vector< SImageSourcePath< T > > firstOnly;
    if ( 6 != model.BuildImages( s, 1, firstOnly ) )
        return false;

    // the validated paths
    GUCEF::vector< SImageSourcePath< T > > paths;
    const UInt32 count = model.FindPaths( s, l, paths );
    UInt32 firstCount = 0;
    UInt32 secondCount = 0;
    GUCEF::vector< bool > found( reference.distinct.size(), false );
    GUCEF::vector< bool > firstFound( 6, false );
    for ( size_t i=0; i<paths.size(); ++i )
    {
        if ( i > 0 && paths[ i ].pathLength < paths[ i - 1 ].pathLength )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: " << name << ": paths are not sorted by length" );
            return false;
        }
        if ( !CheckRoomPath( room, source, listener, paths[ i ], tolerance ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: " << name << ": path " << i << " of order " << paths[ i ].order << " breaks the reflection law" );
            return false;
        }
        if ( 1 == paths[ i ].order )
        {
            ++firstCount;
            const UInt32 w = paths[ i ].wallIndex[ 0 ];
            if ( firstFound[ w ] || Distance( SRef3( paths[ i ].position ), reference.first[ w ] ) > tolerance )
                return false;
            firstFound[ w ] = true;
        }
        else
        {
            ++secondCount;
            bool matched = false;
            for ( size_t d=0; d<reference.distinct.size(); ++d )
            {
                if ( Distance( SRef3( paths[ i ].position ), reference.distinct[ d ] ) <= tolerance )
                {
                    if ( found[ d ] )
                    {
                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: " << name << ": second order image " << d << " found twice" );
                        return false;
                    }
                    found[ d ] = true;
                    matched = true;
                }
            }
            if ( !matched )
                return false;
        }
    }
    if ( 6 != firstCount || 18 != secondCount || 24 != count )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: " << name << ": " << firstCount << " first order and " << secondCount << " second order paths, expected 6 and 18" );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: room " << room.lo[ 0 ] << " " << room.lo[ 1 ] << " " << room.lo[ 2 ] << " to " << room.hi[ 0 ] << " " << room.hi[ 1 ] << " " << room.hi[ 2 ] <<
                        " source " << source.v[ 0 ] << " " << source.v[ 1 ] << " " << source.v[ 2 ] << " listener " << listener.v[ 0 ] << " " << listener.v[ 1 ] << " " << listener.v[ 2 ] );
        for ( size_t d=0; d<found.size(); ++d )
        {
            if ( !found[ d ] )
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: missing image " << reference.distinct[ d ].v[ 0 ] << " " << reference.distinct[ d ].v[ 1 ] << " " << reference.distinct[ d ].v[ 2 ] );
        }
        return false;
    }
    // first order only
    model.SetMaxOrder( 1 );
    if ( 6 != model.FindPaths( s, l, paths ) )
        return false;
    model.SetMaxOrder( 0 );
    return 1 == model.GetMaxOrder();
}

/*-------------------------------------------------------------------------*/

/** The fixed room of the acceptance list in both kinds */
static bool
ShoeboxPaths( void )
{
    SRefRoom room;
    room.lo[ 0 ] = 0.0; room.lo[ 1 ] = 0.0; room.lo[ 2 ] = 0.0;
    room.hi[ 0 ] = 10.0; room.hi[ 1 ] = 4.0; room.hi[ 2 ] = 6.0;
    const SRef3 source( 2.0, 1.5, 3.5 );
    const SRef3 listener( 7.5, 2.0, 1.0 );
    return RoomPaths< Float64 >( room, source, listener, 1e-9, "shoebox Float64" ) && RoomPaths< Float32 >( room, source, listener, 2e-4, "shoebox Float32" );
}

/*-------------------------------------------------------------------------*/

/** Seeded rooms of random size with the source and the listener inside: always 6 and 18 paths that obey the law */
template < typename T >
static bool
RoomSweep( const char* name, UInt64 seed, UInt32 rooms, Float64 tolerance )
{
    GeometryReference::CSeededSequence random( seed );
    UInt32 bad = 0;
    for ( UInt32 r=0; r<rooms && bad<5; ++r )
    {
        SRefRoom room;
        SRef3 source;
        SRef3 listener;
        for ( UInt32 axis=0; axis<3; ++axis )
        {
            room.lo[ axis ] = random.NextFloat64( -20.0, 20.0 );
            const Float64 size = random.NextFloat64( 3.0, 15.0 );
            room.hi[ axis ] = room.lo[ axis ] + size;
            source.v[ axis ] = room.lo[ axis ] + random.NextFloat64( 0.25, size - 0.25 );
            listener.v[ axis ] = room.lo[ axis ] + random.NextFloat64( 0.25, size - 0.25 );
        }
        if ( !RoomPaths< T >( room, source, listener, tolerance, name ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: " << name << " room " << r << " failed (seed " << seed << ")" );
            ++bad;
        }
    }
    return 0 == bad;
}

/*-------------------------------------------------------------------------*/

template < typename T >
static CTPoint3D< T >
P3( Float64 x, Float64 y, Float64 z )
{
    return CTPoint3D< T >( (T) x, (T) y, (T) z );
}

/*-------------------------------------------------------------------------*/

/** Walls of finite size: a reflection point outside the polygon is rejected, sides, the plane of the source, one sided walls, degenerate polygons */
template < typename T >
static bool
FiniteWalls( Float64 tolerance )
{
    CTPoint3D< T > corners[ 4 ];
    corners[ 0 ] = P3< T >( 0, 0, 0 );
    corners[ 1 ] = P3< T >( 1, 0, 0 );
    corners[ 2 ] = P3< T >( 1, 1, 0 );
    corners[ 3 ] = P3< T >( 0, 1, 0 );
    CTImageSourceWall< T > wall;
    if ( !CTImageSourceWall< T >::FromPolygon( corners, 4, wall, 5 ) || 5 != wall.GetUserId() || !wall.IsBounded() )
        return false;
    // counter-clockwise from +Z: the normal is +Z
    if ( fabs( (Float64) wall.GetPlane().normal.AsVector().z - 1.0 ) > tolerance )
        return false;

    CTImageSourceModel< T > model;
    model.AddWall( wall );
    GUCEF::vector< SImageSourcePath< T > > paths;

    // inside: the reflection point of ( 0.2, 0.3, 2 ) and ( 0.8, 0.6, 3 ) is at 2 / 5 of the way
    if ( 1 != model.FindPaths( P3< T >( 0.2, 0.3, 2 ), P3< T >( 0.8, 0.6, 3 ), paths ) )
        return false;
    if ( Distance( SRef3( paths[ 0 ].reflectionPoint[ 0 ] ), SRef3( 0.2 + 0.6 * 0.4, 0.3 + 0.3 * 0.4, 0.0 ) ) > tolerance || 5 != paths[ 0 ].wallUserId[ 0 ] )
        return false;
    // on the boundary of the polygon the point still counts: the reflection point is ( 1, 0.5, 0 )
    if ( 1 != model.FindPaths( P3< T >( 0.5, 0.5, 2 ), P3< T >( 1.5, 0.5, 2 ), paths ) )
        return false;
    // outside the polygon: rejected
    if ( 0 != model.FindPaths( P3< T >( 0.2, 0.3, 2 ), P3< T >( 5, 0.6, 3 ), paths ) )
        return false;
    if ( 0 != model.FindPaths( P3< T >( -3, 0.3, 2 ), P3< T >( 0.8, 0.6, 3 ), paths ) )
        return false;
    // on opposite sides of the wall: nothing
    if ( 0 != model.FindPaths( P3< T >( 0.2, 0.3, -2 ), P3< T >( 0.8, 0.6, 3 ), paths ) )
        return false;
    // the source in the plane of the wall: nothing
    if ( 0 != model.FindPaths( P3< T >( 0.2, 0.3, 0 ), P3< T >( 0.8, 0.6, 3 ), paths ) )
        return false;
    // both behind the wall: a two sided wall reflects, a one sided wall does not
    if ( 1 != model.FindPaths( P3< T >( 0.2, 0.3, -2 ), P3< T >( 0.8, 0.6, -3 ), paths ) )
        return false;
    model.SetOneSided( true );
    if ( 0 != model.FindPaths( P3< T >( 0.2, 0.3, -2 ), P3< T >( 0.8, 0.6, -3 ), paths ) || 1 != model.FindPaths( P3< T >( 0.2, 0.3, 2 ), P3< T >( 0.8, 0.6, 3 ), paths ) )
        return false;
    model.SetOneSided( false );

    // the other winding flips the front: one sided, the former back side reflects
    CTPoint3D< T > reversed[ 4 ] = { corners[ 3 ], corners[ 2 ], corners[ 1 ], corners[ 0 ] };
    CTImageSourceWall< T > back;
    if ( !CTImageSourceWall< T >::FromPolygon( reversed, 4, back ) || fabs( (Float64) back.GetPlane().normal.AsVector().z + 1.0 ) > tolerance )
        return false;
    CTImageSourceModel< T > backModel;
    backModel.AddWall( back );
    backModel.SetOneSided( true );
    if ( 0 != backModel.FindPaths( P3< T >( 0.2, 0.3, 2 ), P3< T >( 0.8, 0.6, 3 ), paths ) || 1 != backModel.FindPaths( P3< T >( 0.2, 0.3, -2 ), P3< T >( 0.8, 0.6, -3 ), paths ) )
        return false;

    // a wall in a tilted plane: the image is the mirror image in that plane
    CTPoint3D< T > tilted[ 3 ] = { P3< T >( 0, 0, 0 ), P3< T >( 4, 0, 0 ), P3< T >( 0, 3, 4 ) };
    CTImageSourceWall< T > slanted;
    if ( !CTImageSourceWall< T >::FromPolygon( tilted, 3, slanted ) )
        return false;
    // the plane has the normal ( 0, -4, 3 ) / 5 or its opposite; the image of ( 1, 1, 1 ) is at ( 1, 1, 1 ) - 2 d n with d the signed distance
    CTImageSourceModel< T > slantedModel;
    slantedModel.AddWall( slanted );
    GUCEF::vector< SImageSourcePath< T > > images;
    slantedModel.BuildImages( P3< T >( 1, 1, 1 ), 1, images );
    const Float64 d = ( -4.0 * 1.0 + 3.0 * 1.0 ) / 5.0;
    if ( 1 != images.size() || Distance( SRef3( images[ 0 ].position ), SRef3( 1.0, 1.0 - 2.0 * d * ( -4.0 / 5.0 ), 1.0 - 2.0 * d * ( 3.0 / 5.0 ) ) ) > tolerance )
        return false;

    // degenerate polygons leave the output unchanged
    CTImageSourceWall< T > untouched;
    CTPoint3D< T > line[ 3 ] = { P3< T >( 0, 0, 0 ), P3< T >( 1, 0, 0 ), P3< T >( 2, 0, 0 ) };
    if ( CTImageSourceWall< T >::FromPolygon( line, 3, untouched ) || CTImageSourceWall< T >::FromPolygon( corners, 2, untouched ) || CTImageSourceWall< T >::FromPolygon( GUCEF_NULL, 4, untouched ) || untouched.IsBounded() )
        return false;

    // an unbounded wall reflects everywhere
    CTImageSourceModel< T > open;
    open.AddWall( CTImageSourceWall< T >::FromPlane( CTPlane< T >::FromPointNormal( P3< T >( 0, 0, 0 ), CTDirectionVector3D< T >::UnitZ() ) ) );
    return 1 == open.FindPaths( P3< T >( 1000, -500, 2 ), P3< T >( -700, 90, 3 ), paths );
}

/*-------------------------------------------------------------------------*/

/** Walls from linked planes (extents, placement) */
template < typename T >
static bool
WallsFromPlanes( Float64 tolerance )
{
    const CTTransform3D< T > placement( CTVector3D< T >( (T) 0, (T) 0, (T) 5 ), CTQuaternion< T >(), CTVector3D< T >( (T) 1, (T) 1, (T) 1 ) );
    CTGeoPlane< T > plane( placement );
    CTImageSourceWall< T > wall;
    if ( !CTImageSourceWall< T >::FromGeoPlane( plane, wall, 9 ) || wall.IsBounded() || !wall.ContainsPoint( P3< T >( 100, -100, 5 ), (T) tolerance ) )
        return false;
    plane.SetExtents( CTRectangle< T >( CTPoint2D< T >( (T) -1, (T) -1 ), CTPoint2D< T >( (T) 1, (T) 1 ) ) );
    if ( !CTImageSourceWall< T >::FromGeoPlane( plane, wall, 9 ) || !wall.IsBounded() || 9 != wall.GetUserId() )
        return false;
    if ( fabs( (Float64) wall.GetPlane().normal.AsVector().z - 1.0 ) > tolerance || !wall.ContainsPoint( P3< T >( 0.5, 0.5, 5 ), (T) tolerance ) || wall.ContainsPoint( P3< T >( 2, 0, 5 ), (T) tolerance ) || wall.ContainsPoint( P3< T >( 0.5, 0.5, 5.1 ), (T) tolerance ) )
        return false;
    // a reflection through it
    CTImageSourceModel< T > model;
    model.AddWall( wall );
    GUCEF::vector< SImageSourcePath< T > > paths;
    if ( 1 != model.FindPaths( P3< T >( 0, 0, 7 ), P3< T >( 0.5, 0, 6 ), paths ) || 0 != model.FindPaths( P3< T >( 0, 0, 7 ), P3< T >( 8, 0, 6 ), paths ) )
        return false;

    // turned about X by 90 degrees the normal is -Y
    CTGeoPlane< T > turned( CTTransform3D< T >( CTVector3D< T >( (T) 0, (T) 0, (T) 0 ), CTQuaternion< T >::FromAxisAngle( CTDirectionVector3D< T >::UnitX(), CTRadian< T >( (T) 1.57079632679489661923 ) ), CTVector3D< T >( (T) 1, (T) 1, (T) 1 ) ) );
    turned.SetExtents( CTRectangle< T >( CTPoint2D< T >( (T) -1, (T) -1 ), CTPoint2D< T >( (T) 1, (T) 1 ) ) );
    CTImageSourceWall< T > side;
    if ( !CTImageSourceWall< T >::FromGeoPlane( turned, side ) || fabs( (Float64) side.GetPlane().normal.AsVector().y + 1.0 ) > tolerance * 10.0 )
        return false;

    // an empty extents gives no wall
    CTGeoPlane< T > empty( placement );
    empty.SetExtents( CTRectangle< T >() );
    CTImageSourceWall< T > none;
    return !CTImageSourceWall< T >::FromGeoPlane( empty, none );
}

/*-------------------------------------------------------------------------*/

/** The slabs that make the walls of a room solid for the ray caster, half a unit thick outside the room */
template < typename T >
struct SSlabRoom
{
    CTRayCaster< T > caster;
    GUCEF::vector< CTGeoObjectBase< T >* > objects;

    explicit SSlabRoom( const SRefRoom& room ) : caster(), objects()
    {
        const Float64 t = 0.5;
        for ( UInt32 wall=0; wall<6; ++wall )
        {
            const UInt32 axis = wall / 2;
            Float64 mn[ 3 ];
            Float64 mx[ 3 ];
            for ( UInt32 a=0; a<3; ++a )
            {
                mn[ a ] = room.lo[ a ] - t;
                mx[ a ] = room.hi[ a ] + t;
            }
            if ( 0 == ( wall & 1 ) )
            {
                mn[ axis ] = room.lo[ axis ] - t;
                mx[ axis ] = room.lo[ axis ];
            }
            else
            {
                mn[ axis ] = room.hi[ axis ];
                mx[ axis ] = room.hi[ axis ] + t;
            }
            Add( mn, mx, 1000 + wall );
        }
    }

    ~SSlabRoom()
    {
        for ( size_t i=0; i<objects.size(); ++i )
            GUCEF_DELETE objects[ i ];
    }

    CTGeoObjectBase< T >* Add( const Float64* mn, const Float64* mx, UInt64 id )
    {
        const CTAxisAlignedBox3D< T > box( P3< T >( mn[ 0 ], mn[ 1 ], mn[ 2 ] ), P3< T >( mx[ 0 ], mx[ 1 ], mx[ 2 ] ) );
        CTGeoObject< CTAxisAlignedBox3D< T > >* object = GUCEF_NEW CTGeoObject< CTAxisAlignedBox3D< T > >( box, CTTransform3D< T >() );
        object->SetUserId( id );
        objects.push_back( object );
        caster.AddObject( *object );
        caster.Update();
        return object;
    }
};

/*-------------------------------------------------------------------------*/

/** Counts the paths of a given order that use a wall as the first reflection */
template < typename T >
static UInt32
CountPaths( const GUCEF::vector< SImageSourcePath< T > >& paths, UInt32 order, UInt32 firstWall )
{
    UInt32 count = 0;
    for ( size_t i=0; i<paths.size(); ++i )
    {
        if ( paths[ i ].order == order && ( 0xFFFFFFFFU == firstWall || paths[ i ].wallIndex[ 0 ] == firstWall ) )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

/** Logs the step of a failed multi step check */
static bool
Fail( UInt32 step )
{
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: occluder check step " << step << " failed" );
    return false;
}

/*-------------------------------------------------------------------------*/

/** Rejects the pillar of the occluder test */
template < typename T >
class CRejectPillar : public CTGeoRayFilter< T >
{
    public:

    virtual bool Accept( const CTGeoObjectBase< T >& object ) const GUCEF_VIRTUAL_OVERRIDE { return 7 != object.GetUserId(); }
};

/*-------------------------------------------------------------------------*/

/** Occluders: the solid walls of the room do not block their own reflections, a pillar blocks the paths through it, the direct path is optional */
template < typename T >
static bool
Occluders( Float64 tolerance )
{
    SRefRoom room;
    room.lo[ 0 ] = 0.0; room.lo[ 1 ] = 0.0; room.lo[ 2 ] = 0.0;
    room.hi[ 0 ] = 10.0; room.hi[ 1 ] = 4.0; room.hi[ 2 ] = 6.0;
    SSlabRoom< T > slabs( room );
    CTImageSourceModel< T > model;
    BuildRoom( model, room );
    model.SetRayCaster( &slabs.caster );
    model.SetIncludeDirect( true );
    const CTPoint3D< T > source = P3< T >( 2, 2, 2.5 );
    const CTPoint3D< T > listener = P3< T >( 8, 2, 3.5 );
    GUCEF::vector< SImageSourcePath< T > > paths;

    // 1 direct, 6 first order and 18 second order
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources: room with solid walls: " << model.FindPaths( source, listener, paths ) << " paths, first " << ( paths.empty() ? -1 : (Int32) paths[ 0 ].order ) );
    for ( size_t i=0; i<paths.size(); ++i )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ImageSources:   path " << i << " order " << paths[ i ].order << " walls " << (Int32) paths[ i ].wallIndex[ 0 ] << " " << (Int32) paths[ i ].wallIndex[ 1 ] << " length " << paths[ i ].pathLength );
    if ( 25 != model.FindPaths( source, listener, paths ) || 0 != paths[ 0 ].order || fabs( (Float64) paths[ 0 ].pathLength - sqrt( 37.0 ) ) > tolerance )
        return Fail( 1 );
    // the direct path comes out with the source as its position and no walls
    if ( Distance( SRef3( paths[ 0 ].position ), SRef3( source ) ) > tolerance || 0xFFFFFFFFU != paths[ 0 ].wallIndex[ 0 ] )
        return Fail( 2 );
    // without a ray caster the same
    model.SetRayCaster( GUCEF_NULL );
    if ( 25 != model.FindPaths( source, listener, paths ) )
        return Fail( 3 );
    model.SetRayCaster( &slabs.caster );

    // a pillar on the way of the floor reflection ( the floor is wall 2, its reflection point is ( 5, 0, 3 ) ) at the middle of the segment to the listener
    const Float64 mn[ 3 ] = { 6.4, 0.9, 3.15 };
    const Float64 mx[ 3 ] = { 6.6, 1.1, 3.35 };
    CTGeoObjectBase< T >* pillar = slabs.Add( mn, mx, 7 );
    model.FindPaths( source, listener, paths );
    if ( 0 != CountPaths( paths, 1, 2 ) || 5 != CountPaths( paths, 1, 0xFFFFFFFFU ) || 1 != CountPaths( paths, 0, 0xFFFFFFFFU ) )
        return Fail( 4 );
    // second order paths that run through the pillar are gone as well, those that do not remain
    const UInt32 secondWith = CountPaths( paths, 2, 0xFFFFFFFFU );
    if ( secondWith > 18 || secondWith < 10 )
        return Fail( 5 );

    // a filter that rejects the pillar brings everything back, and so does removing it
    CRejectPillar< T > filter;
    model.SetFilter( &filter );
    if ( 25 != model.FindPaths( source, listener, paths ) )
        return Fail( 6 );
    model.SetFilter( GUCEF_NULL );
    slabs.caster.RemoveObject( *pillar );
    slabs.caster.Update();
    if ( 25 != model.FindPaths( source, listener, paths ) )
        return Fail( 7 );

    // a pillar on the direct path blocks only the direct path of the first order paths ( they run below and above it )
    const Float64 dmn[ 3 ] = { 4.9, 1.9, 2.9 };
    const Float64 dmx[ 3 ] = { 5.1, 2.1, 3.1 };
    slabs.Add( dmn, dmx, 8 );
    model.FindPaths( source, listener, paths );
    if ( 0 != CountPaths( paths, 0, 0xFFFFFFFFU ) || 6 != CountPaths( paths, 1, 0xFFFFFFFFU ) )
        return Fail( 8 );
    model.SetIncludeDirect( false );
    model.FindPaths( source, listener, paths );
    return 0 == CountPaths( paths, 0, 0xFFFFFFFFU );
}

/*-------------------------------------------------------------------------*/

/** Settings and simple cases of the model */
template < typename T >
static bool
ModelSettings( void )
{
    CTImageSourceModel< T > model;
    UInt32 first = 99;
    if ( model.AddShoebox( P3< T >( 0, 0, 0 ), P3< T >( 0, 1, 1 ), 0, first ) || 0 != model.GetWallCount() )
        return false;
    if ( !model.AddShoebox( P3< T >( 0, 0, 0 ), P3< T >( 1, 1, 1 ), 40, first ) || 0 != first || 6 != model.GetWallCount() )
        return false;
    if ( !model.AddShoebox( P3< T >( 5, 0, 0 ), P3< T >( 6, 1, 1 ), 50, first ) || 6 != first || 12 != model.GetWallCount() )
        return false;
    model.Clear();
    if ( 0 != model.GetWallCount() )
        return false;
    GUCEF::vector< SImageSourcePath< T > > paths;
    if ( 0 != model.FindPaths( P3< T >( 0, 0, 0 ), P3< T >( 1, 1, 1 ), paths ) )
        return false;
    model.SetMaxOrder( 7 );
    if ( 2 != model.GetMaxOrder() )
        return false;
    model.SetMaxOrder( 0 );
    if ( 1 != model.GetMaxOrder() )
        return false;
    model.SetPointTolerance( (T) -1 );
    model.SetSegmentMargin( (T) -1 );
    return model.GetPointTolerance() > (T) 0 && model.GetSegmentMargin() > (T) 0;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TESTS                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformImageSourcesTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ImageSources" );

    GUCEF_TESTFW_TESTCASE( "ImageSources: a shoebox gives the analytic 6 first order and 18 distinct second order images, and every path obeys the reflection law (1e-9 Float64, 2e-4 Float32)" )
    try
    {
        ASSERT_TRUE( ShoeboxPaths() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImageSources: seeded random rooms always give 6 and 18 paths that obey the reflection law" )
    try
    {
        ASSERT_TRUE( RoomSweep< Float64 >( "rooms Float64", 61001, ScaledCount( 300 ), 1e-9 ) );
        ASSERT_TRUE( RoomSweep< Float32 >( "rooms Float32", 61002, ScaledCount( 200 ), 5e-3 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImageSources: reflection points outside the wall polygon, walls on the wrong side, sources in the plane, one sided walls and degenerate polygons are rejected" )
    try
    {
        ASSERT_TRUE( FiniteWalls< Float64 >( 1e-9 ) );
        ASSERT_TRUE( FiniteWalls< Float32 >( 2e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImageSources: walls from linked planes take their extents and front" )
    try
    {
        ASSERT_TRUE( WallsFromPlanes< Float64 >( 1e-9 ) );
        ASSERT_TRUE( WallsFromPlanes< Float32 >( 1e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImageSources: solid walls do not block their own reflections, occluders block the paths through them, filters and removal restore them" )
    try
    {
        ASSERT_TRUE( Occluders< Float64 >( 1e-9 ) );
        ASSERT_TRUE( Occluders< Float32 >( 2e-4 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ImageSources: model settings are limited and shoeboxes add six walls in a fixed order" )
    try
    {
        ASSERT_TRUE( ModelSettings< Float32 >() );
        ASSERT_TRUE( ModelSettings< Float64 >() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
