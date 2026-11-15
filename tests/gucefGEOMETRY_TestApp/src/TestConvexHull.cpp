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

#ifndef GUCEF_GEOMETRY_CONVEXHULL2D_H
#include "gucefGEOMETRY_ConvexHull2D.h"
#define GUCEF_GEOMETRY_CONVEXHULL2D_H
#endif /* GUCEF_GEOMETRY_CONVEXHULL2D_H ? */

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#include "gucefGEOMETRY_CTQuickHull3D.h"
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H
#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */


#include "TestSpatialReference.h"
#include "TestConvexReference.h"
#include "TestConvexHull.h"

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

typedef TMathBigFraction F;

static Int32 MakeInt32( Int64 g ) { return (Int32) g; }
static Float64 MakeFloat64( Int64 g ) { return ldexp( (Float64) g, -3 ); }
static Float32 MakeFloat32( Int64 g ) { return (Float32) g / 8.0f; }
static TFixed32_32 MakeFixed( Int64 g ) { return TFixed32_32::FromRaw( g * 536870912 ); }

/*-------------------------------------------------------------------------*/

template < typename T >
static F
Cross( const CTPoint2D< T >& a, const CTPoint2D< T >& b, const CTPoint2D< T >& c )
{
    return ( ExactFraction( b.x ) - ExactFraction( a.x ) ) * ( ExactFraction( c.y ) - ExactFraction( a.y ) ) - ( ExactFraction( b.y ) - ExactFraction( a.y ) ) * ( ExactFraction( c.x ) - ExactFraction( a.x ) );
}

template < typename T >
static bool
SamePoint( const CTPoint2D< T >& a, const CTPoint2D< T >& b )
{
    return a.x == b.x && a.y == b.y;
}

/** The squared length of b - a as a fraction */
template < typename T >
static F
SquaredLength( const CTPoint2D< T >& a, const CTPoint2D< T >& b )
{
    const F dx = ExactFraction( b.x ) - ExactFraction( a.x );
    const F dy = ExactFraction( b.y ) - ExactFraction( a.y );
    return dx * dx + dy * dy;
}

/**
 *  The reference hull by gift wrapping with exact cross products: the strictly convex vertices counter-clockwise from the
 *  lowest point by x then y, and with 'keepCollinear' the points on the edges in order along each edge. Duplicates count once, by the lowest index.
 */
template < typename T >
static void
ReferenceHull( const CTPoint2D< T >* points, UInt32 count, bool keepCollinear, GUCEF::vector< UInt32 >& result )
{
    result.clear();
    GUCEF::vector< UInt32 > unique;
    for ( UInt32 i=0; i<count; ++i )
    {
        bool seen = false;
        for ( size_t k=0; k<unique.size() && !seen; ++k )
            seen = SamePoint( points[ unique[ k ] ], points[ i ] );
        if ( !seen )
            unique.push_back( i );
    }
    if ( unique.size() <= 2 )
    {
        // the order is by x then y
        if ( 2 == unique.size() && ( points[ unique[ 1 ] ].x < points[ unique[ 0 ] ].x || ( points[ unique[ 1 ] ].x == points[ unique[ 0 ] ].x && points[ unique[ 1 ] ].y < points[ unique[ 0 ] ].y ) ) )
        {
            const UInt32 swap = unique[ 0 ]; unique[ 0 ] = unique[ 1 ]; unique[ 1 ] = swap;
        }
        result = unique;
        return;
    }
    // the lowest point by x then y
    size_t start = 0;
    for ( size_t k=1; k<unique.size(); ++k )
    {
        const CTPoint2D< T >& p = points[ unique[ k ] ];
        const CTPoint2D< T >& s = points[ unique[ start ] ];
        if ( p.x < s.x || ( p.x == s.x && p.y < s.y ) )
            start = k;
    }
    // all collinear?
    bool collinear = true;
    for ( size_t k=0; k<unique.size() && collinear; ++k )
    {
        // against the two points that are furthest apart on the line through the first two
        collinear = Cross( points[ unique[ 0 ] ], points[ unique[ 1 ] ], points[ unique[ k ] ] ) == F( 0, 1 );
    }
    if ( collinear )
    {
        GUCEF::vector< UInt32 > ordered = unique;
        for ( size_t i=0; i<ordered.size(); ++i )
            for ( size_t j=i+1; j<ordered.size(); ++j )
            {
                const CTPoint2D< T >& a = points[ ordered[ i ] ];
                const CTPoint2D< T >& b = points[ ordered[ j ] ];
                if ( b.x < a.x || ( b.x == a.x && b.y < a.y ) )
                {
                    const UInt32 swap = ordered[ i ]; ordered[ i ] = ordered[ j ]; ordered[ j ] = swap;
                }
            }
        if ( keepCollinear )
            result = ordered;
        else
        {
            result.push_back( ordered.front() );
            result.push_back( ordered.back() );
        }
        return;
    }
    GUCEF::vector< UInt32 > strict;
    size_t current = start;
    do
    {
        strict.push_back( unique[ current ] );
        size_t next = ( current + 1 ) % unique.size();
        for ( size_t k=0; k<unique.size(); ++k )
        {
            if ( k == current )
                continue;
            const F c = Cross( points[ unique[ current ] ], points[ unique[ next ] ], points[ unique[ k ] ] );
            if ( F( 0, 1 ) < F( 0, 1 ) - c || ( c == F( 0, 1 ) && SquaredLength( points[ unique[ current ] ], points[ unique[ next ] ] ) < SquaredLength( points[ unique[ current ] ], points[ unique[ k ] ] ) ) )
                next = k;
        }
        current = next;
    }
    while ( current != start && strict.size() <= unique.size() );
    if ( !keepCollinear )
    {
        result = strict;
        return;
    }
    for ( size_t e=0; e<strict.size(); ++e )
    {
        const UInt32 a = strict[ e ];
        const UInt32 b = strict[ ( e + 1 ) % strict.size() ];
        result.push_back( a );
        GUCEF::vector< UInt32 > between;
        for ( size_t k=0; k<unique.size(); ++k )
        {
            const UInt32 c = unique[ k ];
            if ( c == a || c == b )
                continue;
            if ( Cross( points[ a ], points[ b ], points[ c ] ) == F( 0, 1 ) && SquaredLength( points[ a ], points[ c ] ) < SquaredLength( points[ a ], points[ b ] ) && SquaredLength( points[ b ], points[ c ] ) < SquaredLength( points[ a ], points[ b ] ) )
                between.push_back( c );
        }
        for ( size_t i=0; i<between.size(); ++i )
            for ( size_t j=i+1; j<between.size(); ++j )
                if ( SquaredLength( points[ a ], points[ between[ j ] ] ) < SquaredLength( points[ a ], points[ between[ i ] ] ) )
                {
                    const UInt32 swap = between[ i ]; between[ i ] = between[ j ]; between[ j ] = swap;
                }
        for ( size_t i=0; i<between.size(); ++i )
            result.push_back( between[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
Hull2DSweep( const char* name, UInt64 seed, UInt32 count, T ( *make )( Int64 ) )
{
    CRandom random( seed );
    for ( UInt32 n=0; n<count; ++n )
    {
        const UInt32 pointCount = (UInt32) random.Index( 41 );
        const Int32 range = 1 + random.Index( 8 );
        GUCEF::vector< CTPoint2D< T > > points;
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            // some sets lie on a line, some on a grid
            const Int32 x = random.Index( 2 * range + 1 ) - range;
            const Int32 y = 0 == n % 7 ? 2 * x : random.Index( 2 * range + 1 ) - range;
            points.push_back( CTPoint2D< T >( make( x ), make( y ) ) );
        }
        for ( UInt32 keep=0; keep<2; ++keep )
        {
            GUCEF::vector< UInt32 > hull;
            GUCEF::vector< UInt32 > expected;
            ConvexHull2DIndices( pointCount > 0 ? &points[ 0 ] : static_cast< const CTPoint2D< T >* >( GUCEF_NULL ), pointCount, hull, 1 == keep );
            ReferenceHull( pointCount > 0 ? &points[ 0 ] : static_cast< const CTPoint2D< T >* >( GUCEF_NULL ), pointCount, 1 == keep, expected );
            if ( hull != expected )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": hull of " << pointCount << " points differs from the gift wrapping reference (keep collinear " << keep << "), seed " << seed << " set " << n << ", got " << hull.size() << " expected " << expected.size() );
                return false;
            }
            // contains all points, counter-clockwise, convex
            if ( hull.size() >= 3 )
            {
                for ( size_t e=0; e<hull.size(); ++e )
                {
                    const CTPoint2D< T >& a = points[ hull[ e ] ];
                    const CTPoint2D< T >& b = points[ hull[ ( e + 1 ) % hull.size() ] ];
                    const CTPoint2D< T >& c = points[ hull[ ( e + 2 ) % hull.size() ] ];
                    const F turn = Cross( a, b, c );
                    if ( turn < F( 0, 1 ) || ( 0 == keep && turn == F( 0, 1 ) ) )
                        return false;
                    for ( UInt32 i=0; i<pointCount; ++i )
                        if ( Cross( a, b, points[ i ] ) < F( 0, 1 ) )
                            return false;
                }
            }
            // the wrappers agree
            GUCEF::vector< CTPoint2D< T > > hullPoints;
            ConvexHull2D( pointCount > 0 ? &points[ 0 ] : static_cast< const CTPoint2D< T >* >( GUCEF_NULL ), pointCount, hullPoints, 1 == keep );
            if ( hullPoints.size() != hull.size() )
                return false;
        }
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: 2D " << name << ": " << count << " point sets equal the gift wrapping reference with and without collinear points" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The points of the hull with a thousand points on a circle of integers, and the degenerate sets */
static bool
Hull2DDegenerate( void )
{
    typedef CTPoint2D< Int32 > P;
    GUCEF::vector< UInt32 > hull;
    if ( 0 != ConvexHull2DIndices( static_cast< const P* >( GUCEF_NULL ), 0, hull ) || !hull.empty() )
        return false;
    const P one[ 3 ] = { P( 2, 2 ), P( 2, 2 ), P( 2, 2 ) };
    if ( 1 != ConvexHull2DIndices( one, 3, hull ) || 0 != hull[ 0 ] )
        return false;
    const P line[ 5 ] = { P( 3, 3 ), P( 0, 0 ), P( 1, 1 ), P( 2, 2 ), P( 1, 1 ) };
    if ( 2 != ConvexHull2DIndices( line, 5, hull ) || hull[ 0 ] != 1 || hull[ 1 ] != 0 )
        return false;
    if ( 4 != ConvexHull2DIndices( line, 5, hull, true ) || hull[ 0 ] != 1 || hull[ 1 ] != 2 || hull[ 2 ] != 3 || hull[ 3 ] != 0 )
        return false;
    // a square with the midpoints of its edges and the centre: four corners, or eight points with the collinear ones
    const P square[ 9 ] = { P( 0, 0 ), P( 1, 0 ), P( 2, 0 ), P( 2, 1 ), P( 2, 2 ), P( 1, 2 ), P( 0, 2 ), P( 0, 1 ), P( 1, 1 ) };
    if ( 4 != ConvexHull2DIndices( square, 9, hull ) )
        return false;
    if ( 8 != ConvexHull2DIndices( square, 9, hull, true ) || hull[ 0 ] != 0 || hull[ 1 ] != 1 || hull[ 2 ] != 2 || hull[ 3 ] != 3 )
        return false;
    // not finite points are skipped
    CTPoint2D< Float64 > fp[ 4 ] = { CTPoint2D< Float64 >( 0, 0 ), CTPoint2D< Float64 >( 1, 0 ), CTPoint2D< Float64 >( 0.0 / ( (Float64) hull.size() - (Float64) hull.size() ), 1 ), CTPoint2D< Float64 >( 0, 1 ) };
    if ( 3 != ConvexHull2DIndices( fp, 4, hull ) )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

typedef CTPoint3D< Float64 > TP3;

struct SHullCheck
{
    UInt32 vertices;
    UInt32 edges;
    UInt32 faces;
};

/** Checks the polyhedron of a quickhull build: closed manifold (every directed edge once, its reverse once), outward faces, all points inside, Euler; the counts into 'check' */
template < typename T >
static bool
CheckPolyhedron( const char* name, const CTQuickHull3D< T >& builder, const CTPoint3D< T >* points, UInt32 count, SHullCheck& check )
{
    const CTConvexHull3D< T >& hull = builder.GetHull();
    const GUCEF::vector< UInt32 >& indices = builder.GetVertexIndices();
    check.vertices = hull.GetVertexCount();
    check.faces = hull.GetFaceCount();
    if ( check.vertices != indices.size() || check.faces < 4 )
        return false;
    for ( UInt32 v=0; v<check.vertices; ++v )
        if ( !( hull.GetVertex( v ) == points[ indices[ v ] ] ) )
            return false;
    // the directed edges of the faces
    GUCEF::vector< UInt64 > directed;
    UInt32 corners = 0;
    for ( UInt32 f=0; f<check.faces; ++f )
    {
        const UInt32 size = hull.GetFaceVertexCount( f );
        if ( size < 3 )
            return false;
        for ( UInt32 k=0; k<size; ++k )
        {
            const UInt32 a = hull.GetFaceVertexIndex( f, k );
            const UInt32 b = hull.GetFaceVertexIndex( f, ( k + 1 ) % size );
            directed.push_back( ( (UInt64) a << 32 ) | b );
            ++corners;
        }
    }
    std::sort( directed.begin(), directed.end() );
    for ( size_t i=1; i<directed.size(); ++i )
        if ( directed[ i ] == directed[ i - 1 ] )
            return false;
    for ( size_t i=0; i<directed.size(); ++i )
    {
        const UInt64 reverse = ( ( directed[ i ] & 0xFFFFFFFFULL ) << 32 ) | ( directed[ i ] >> 32 );
        if ( !std::binary_search( directed.begin(), directed.end(), reverse ) )
            return false;
    }
    check.edges = corners / 2;
    if ( (Int32) check.vertices - (Int32) check.edges + (Int32) check.faces != 2 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": V - E + F = " << ( (Int32) check.vertices - (Int32) check.edges + (Int32) check.faces ) << " with V " << check.vertices << " E " << check.edges << " F " << check.faces );
        return false;
    }
    // faces outward and every input point behind or on every face plane
    Float64 scale = 1.0;
    for ( UInt32 i=0; i<count; ++i )
    {
        scale = scale > fabs( (Float64) points[ i ].x ) ? scale : fabs( (Float64) points[ i ].x );
        scale = scale > fabs( (Float64) points[ i ].y ) ? scale : fabs( (Float64) points[ i ].y );
        scale = scale > fabs( (Float64) points[ i ].z ) ? scale : fabs( (Float64) points[ i ].z );
    }
    const Float64 epsilon = 1.0e-12 * scale + 10.0 * builder.GetTolerance();
    for ( UInt32 f=0; f<check.faces; ++f )
    {
        // Newell's normal of the polygon from the Float64 coordinates
        const UInt32 size = hull.GetFaceVertexCount( f );
        Float64 n[ 3 ] = { 0, 0, 0 };
        Float64 c[ 3 ] = { 0, 0, 0 };
        for ( UInt32 k=0; k<size; ++k )
        {
            const CTPoint3D< T >& p = hull.GetVertex( hull.GetFaceVertexIndex( f, k ) );
            const CTPoint3D< T >& q = hull.GetVertex( hull.GetFaceVertexIndex( f, ( k + 1 ) % size ) );
            n[ 0 ] += ( (Float64) p.y - (Float64) q.y ) * ( (Float64) p.z + (Float64) q.z );
            n[ 1 ] += ( (Float64) p.z - (Float64) q.z ) * ( (Float64) p.x + (Float64) q.x );
            n[ 2 ] += ( (Float64) p.x - (Float64) q.x ) * ( (Float64) p.y + (Float64) q.y );
            c[ 0 ] += (Float64) p.x; c[ 1 ] += (Float64) p.y; c[ 2 ] += (Float64) p.z;
        }
        const Float64 length = sqrt( n[ 0 ] * n[ 0 ] + n[ 1 ] * n[ 1 ] + n[ 2 ] * n[ 2 ] );
        if ( !( length > 0.0 ) )
            return false;
        for ( UInt32 a=0; a<3; ++a )
        {
            n[ a ] /= length;
            c[ a ] /= (Float64) size;
        }
        for ( UInt32 i=0; i<count; ++i )
        {
            const Float64 d = n[ 0 ] * ( (Float64) points[ i ].x - c[ 0 ] ) + n[ 1 ] * ( (Float64) points[ i ].y - c[ 1 ] ) + n[ 2 ] * ( (Float64) points[ i ].z - c[ 2 ] );
            if ( d > epsilon )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": point " << i << " is " << d << " in front of face " << f );
                return false;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The sorted vertex triple of a triangle as one key */
static UInt64
TripleKey( UInt32 a, UInt32 b, UInt32 c )
{
    UInt32 t[ 3 ] = { a, b, c };
    std::sort( t, t + 3 );
    return ( (UInt64) t[ 0 ] << 42 ) | ( (UInt64) t[ 1 ] << 21 ) | t[ 2 ];
}

/** True when the point lies within epsilon behind or on every triangle of the brute force hull of the other points */
static bool
InsideBruteForce( const GUCEF::vector< P3 >& others, const P3& p, Float64 epsilon )
{
    if ( others.size() < 4 )
        return false;
    const ConvexReference::SPolytope hull = ConvexReference::BruteForceHull( others );
    if ( hull.triangles.empty() )
        return false;
    for ( size_t t=0; t<hull.triangles.size(); t+=3 )
    {
        const P3& a = hull.vertices[ hull.triangles[ t ] ];
        const V3 normal = ( hull.vertices[ hull.triangles[ t + 1 ] ] - a ).Cross( hull.vertices[ hull.triangles[ t + 2 ] ] - a );
        const Float64 length = normal.GetLength();
        if ( length > 0.0 && normal.Dot( p - a ) / length > epsilon )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Points of a generic position: the faces are triangles and equal the triangles of the brute force hull (all triples tested as faces) */
template < typename T >
static bool
QuickHullGeneric( const char* name, UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    UInt32 largest = 0;
    for ( UInt32 n=0; n<count; ++n )
    {
        const UInt32 pointCount = 4 + (UInt32) random.Index( 47 );
        GUCEF::vector< CTPoint3D< T > > points;
        GUCEF::vector< P3 > wide;
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            const P3 p = random.Point( 1.0 );
            // the same values in both kinds: widen the stored ones
            const CTPoint3D< T > stored( (T) p.x, (T) p.y, (T) p.z );
            points.push_back( stored );
            wide.push_back( P3( (Float64) stored.x, (Float64) stored.y, (Float64) stored.z ) );
        }
        CTQuickHull3D< T > builder;
        if ( QUICKHULL_OK != builder.Build( &points[ 0 ], pointCount ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the build failed with status " << (Int32) builder.GetStatus() << ", seed " << seed << " set " << n );
            return false;
        }
        SHullCheck check;
        if ( !CheckPolyhedron( name, builder, &points[ 0 ], pointCount, check ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": polyhedron check failed, seed " << seed << " set " << n );
            return false;
        }
        largest = largest > check.vertices ? largest : check.vertices;
        // the brute force hull: every triple that has all points on one side
        const ConvexReference::SPolytope reference = ConvexReference::BruteForceHull( wide );
        GUCEF::vector< UInt64 > expected;
        for ( size_t t=0; t<reference.triangles.size(); t+=3 )
            expected.push_back( TripleKey( reference.triangles[ t ], reference.triangles[ t + 1 ], reference.triangles[ t + 2 ] ) );
        GUCEF::vector< UInt64 > actual;
        const CTConvexHull3D< T >& hull = builder.GetHull();
        for ( UInt32 f=0; f<hull.GetFaceCount(); ++f )
        {
            if ( 3 != hull.GetFaceVertexCount( f ) )
                return false;
            actual.push_back( TripleKey( builder.GetVertexIndices()[ hull.GetFaceVertexIndex( f, 0 ) ], builder.GetVertexIndices()[ hull.GetFaceVertexIndex( f, 1 ) ], builder.GetVertexIndices()[ hull.GetFaceVertexIndex( f, 2 ) ] ) );
        }
        std::sort( expected.begin(), expected.end() );
        std::sort( actual.begin(), actual.end() );
        if ( expected != actual )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the faces differ from the brute force hull: " << actual.size() << " against " << expected.size() << ", seed " << seed << " set " << n << " points " << pointCount );
            return false;
        }
        // the faces face outward: the centre of the vertices is behind every face (the volume is positive)
        if ( !( hull.GetVolume() > T( 0 ) ) )
            return false;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: quickhull " << name << ": " << count << " sets of up to 50 points equal the brute force hull, up to " << largest << " vertices" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Lattice subsets, cospherical sets: faces are merged polygons; compared with the brute force hull by planes and by the extreme points */
static bool
QuickHullDegenerateSets( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    for ( UInt32 n=0; n<count + 2; ++n )
    {
        GUCEF::vector< TP3 > points;
        const char* name = "lattice";
        if ( n >= count )
        {
            // all integer points of the sphere x x + y y + z z = 9 (30 points), and the full 3 by 3 by 3 lattice
            name = n == count ? "cospherical" : "lattice 3x3x3";
            for ( Int32 x=-3; x<=3; ++x )
                for ( Int32 y=-3; y<=3; ++y )
                    for ( Int32 z=-3; z<=3; ++z )
                        if ( count == n - 0 ? ( x * x + y * y + z * z == 9 ) : ( x >= -1 && x <= 1 && y >= -1 && y <= 1 && z >= -1 && z <= 1 ) )
                            points.push_back( TP3( x, y, z ) );
        }
        else
        {
            const UInt32 pointCount = 6 + (UInt32) random.Index( 15 );
            for ( UInt32 i=0; i<pointCount; ++i )
                points.push_back( TP3( random.Index( 4 ), random.Index( 4 ), random.Index( 4 ) ) );
        }
        CTQuickHull3D< Float64 > builder;
        const TQuickHullStatus status = builder.Build( &points[ 0 ], (UInt32) points.size() );
        if ( QUICKHULL_OK != status )
        {
            // random lattice subsets can be flat: the lower dimensional results are checked elsewhere
            if ( n < count && ( QUICKHULL_PLANE == status || QUICKHULL_LINE == status || QUICKHULL_POINT == status ) )
                continue;
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: " << name << ": status " << (Int32) status << ", seed " << seed << " set " << n );
            return false;
        }
        SHullCheck check;
        if ( !CheckPolyhedron( name, builder, &points[ 0 ], (UInt32) points.size(), check ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: " << name << ": polyhedron check failed, seed " << seed << " set " << n );
            return false;
        }
        GUCEF::vector< P3 > wide;
        for ( size_t i=0; i<points.size(); ++i )
            wide.push_back( P3( points[ i ].x, points[ i ].y, points[ i ].z ) );
        const ConvexReference::SPolytope reference = ConvexReference::BruteForceHull( wide );
        // every triangle of the brute force hull lies in a face of the build, on the outward side
        const CTConvexHull3D< Float64 >& hull = builder.GetHull();
        for ( size_t t=0; t<reference.triangles.size(); t+=3 )
        {
            bool covered = false;
            for ( UInt32 f=0; f<hull.GetFaceCount() && !covered; ++f )
            {
                const CTPlane< Float64 >& plane = hull.GetFacePlane( f );
                covered = true;
                for ( UInt32 k=0; k<3; ++k )
                    covered = covered && fabs( plane.GetSignedDistance( points[ reference.triangles[ t + k ] ] ) ) < 1.0e-9;
                if ( covered )
                {
                    const P3& a = wide[ reference.triangles[ t ] ];
                    const V3 normal = ( wide[ reference.triangles[ t + 1 ] ] - a ).Cross( wide[ reference.triangles[ t + 2 ] ] - a );
                    covered = normal.Dot( plane.normal.AsVector() ) > 0.0;
                }
            }
            if ( !covered )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: " << name << ": a triangle of the brute force hull is in no face, seed " << seed << " set " << n );
                return false;
            }
        }
        // the vertices are exactly the extreme points: a point is a vertex iff it is not inside the hull of the others
        if ( points.size() <= 30 )
        {
            GUCEF::vector< UInt8 > isVertex( points.size(), 0 );
            for ( size_t v=0; v<builder.GetVertexIndices().size(); ++v )
                isVertex[ builder.GetVertexIndices()[ v ] ] = 1;
            for ( size_t i=0; i<points.size(); ++i )
            {
                // equal points are interchangeable: the group has one vertex when the position is extreme, none otherwise
                UInt32 vertexCopies = 0;
                bool first = true;
                for ( size_t j=0; j<points.size(); ++j )
                    if ( wide[ j ].x == wide[ i ].x && wide[ j ].y == wide[ i ].y && wide[ j ].z == wide[ i ].z )
                    {
                        vertexCopies += isVertex[ j ];
                        if ( j < i )
                            first = false;
                    }
                if ( !first )
                    continue;
                if ( vertexCopies > 1 )
                    return false;
                GUCEF::vector< P3 > others;
                for ( size_t j=0; j<points.size(); ++j )
                    if ( !( wide[ j ].x == wide[ i ].x && wide[ j ].y == wide[ i ].y && wide[ j ].z == wide[ i ].z ) )
                        others.push_back( wide[ j ] );
                const bool inside = InsideBruteForce( others, wide[ i ], 1.0e-9 );
                if ( ( 1 == vertexCopies ) == inside )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: " << name << ": point " << i << " is " << ( inside ? "inside the hull of the others and yet" : "extreme and yet not" ) << " a vertex, seed " << seed << " set " << n );
                    return false;
                }
            }
        }
        if ( count == n )
        {
            // the cospherical set: every point is a vertex
            if ( check.vertices != points.size() )
                return false;
        }
        if ( count + 1 == n )
        {
            // the cube: 8 corners, 12 edges, 6 square faces
            if ( 8 != check.vertices || 12 != check.edges || 6 != check.faces )
                return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

static bool
QuickHullLowerDimensions( void )
{
    CTQuickHull3D< Float64 > builder;
    if ( QUICKHULL_EMPTY != builder.Build( static_cast< const TP3* >( GUCEF_NULL ), 0 ) || builder.GetHull().GetVertexCount() != 0 )
        return false;
    const TP3 one[ 3 ] = { TP3( 1, 2, 3 ), TP3( 1, 2, 3 ), TP3( 1, 2, 3 ) };
    if ( QUICKHULL_POINT != builder.Build( one, 3 ) || 1 != builder.GetHull().GetVertexCount() || builder.GetHull().HasFaces() )
        return false;
    const TP3 line[ 5 ] = { TP3( 0, 0, 0 ), TP3( 1, 1, 1 ), TP3( 3, 3, 3 ), TP3( 2, 2, 2 ), TP3( -1, -1, -1 ) };
    if ( QUICKHULL_LINE != builder.Build( line, 5 ) || 2 != builder.GetHull().GetVertexCount() )
        return false;
    // a planar set: the 2D hull of the square with its centre and the midpoints of its edges
    const TP3 flat[ 9 ] = { TP3( 0, 0, 5 ), TP3( 1, 0, 5 ), TP3( 2, 0, 5 ), TP3( 2, 1, 5 ), TP3( 2, 2, 5 ), TP3( 1, 2, 5 ), TP3( 0, 2, 5 ), TP3( 0, 1, 5 ), TP3( 1, 1, 5 ) };
    if ( QUICKHULL_PLANE != builder.Build( flat, 9 ) || 4 != builder.GetHull().GetVertexCount() || builder.GetHull().HasFaces() )
        return false;
    // a tilted plane
    GUCEF::vector< TP3 > tilted;
    for ( Int32 i=0; i<6; ++i )
        for ( Int32 j=0; j<6; ++j )
            tilted.push_back( TP3( i, j, 2 * i - 3 * j ) );
    if ( QUICKHULL_PLANE != builder.Build( &tilted[ 0 ], (UInt32) tilted.size() ) || 4 != builder.GetHull().GetVertexCount() )
        return false;
    const Float64 zero = 0.0;
    const TP3 bad[ 4 ] = { TP3( 0, 0, 0 ), TP3( 1, 0, 0 ), TP3( 0, 1, 0 ), TP3( 0, 0, zero / zero ) };
    if ( QUICKHULL_NOT_FINITE != builder.Build( bad, 4 ) )
        return false;
    // a tetrahedron and a rebuild with the same engine
    const TP3 tetra[ 4 ] = { TP3( 0, 0, 0 ), TP3( 1, 0, 0 ), TP3( 0, 1, 0 ), TP3( 0, 0, 1 ) };
    if ( QUICKHULL_OK != builder.Build( tetra, 4 ) || 4 != builder.GetHull().GetVertexCount() || 4 != builder.GetHull().GetFaceCount() || 6 != builder.GetEdgeCount() )
        return false;
    if ( fabs( builder.GetHull().GetVolume() - 1.0 / 6.0 ) > 1.0e-12 )
        return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Larger sets: the points of a ball and of a sphere: Euler, all points inside, outward faces */
static bool
QuickHullLarge( UInt64 seed, UInt32 count )
{
    CRandom random( seed );
    for ( UInt32 pass=0; pass<2; ++pass )
    {
        GUCEF::vector< TP3 > points;
        for ( UInt32 i=0; i<count; ++i )
        {
            P3 p = random.Point( 1.0 );
            if ( 1 == pass )
            {
                const Float64 length = p.GetDistance( P3( 0, 0, 0 ) );
                if ( length < 1.0e-6 )
                    continue;
                p = P3( p.x / length, p.y / length, p.z / length );
            }
            points.push_back( TP3( p.x, p.y, p.z ) );
        }
        CTQuickHull3D< Float64 > builder;
        if ( QUICKHULL_OK != builder.Build( &points[ 0 ], (UInt32) points.size() ) )
            return false;
        SHullCheck check;
        if ( !CheckPolyhedron( 0 == pass ? "ball" : "sphere", builder, &points[ 0 ], (UInt32) points.size(), check ) )
            return false;
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ConvexHull: quickhull of " << points.size() << " points of a " << ( 0 == pass ? "ball" : "sphere" ) << ": V " << check.vertices << " E " << check.edges << " F " << check.faces );
    }
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformConvexHullTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "ConvexHull" );

    GUCEF_TESTFW_TESTCASE( "ConvexHull: 2D hull equals gift wrapping with exact cross products, counter-clockwise, convex, collinear points left out or kept" )
    try
    {
        ASSERT_TRUE( Hull2DDegenerate() );
        ASSERT_TRUE( ( Hull2DSweep< Int32 >( "Int32", 6101, ScaledCount( 1500 ), MakeInt32 ) ) );
        ASSERT_TRUE( ( Hull2DSweep< Float64 >( "Float64", 6102, ScaledCount( 1500 ), MakeFloat64 ) ) );
        ASSERT_TRUE( ( Hull2DSweep< Float32 >( "Float32", 6103, ScaledCount( 800 ), MakeFloat32 ) ) );
        ASSERT_TRUE( ( Hull2DSweep< TFixed32_32 >( "TFixed32_32", 6104, ScaledCount( 800 ), MakeFixed ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConvexHull: quickhull 3D of up to 50 points of generic position equals the brute force hull, V - E + F = 2, every point inside" )
    try
    {
        ASSERT_TRUE( ( QuickHullGeneric< Float64 >( "Float64", 6201, ScaledCount( 300 ) ) ) );
        ASSERT_TRUE( ( QuickHullGeneric< Float32 >( "Float32", 6202, ScaledCount( 300 ) ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConvexHull: quickhull 3D of coplanar and cospherical inputs merges faces, keeps exactly the extreme points and covers the brute force hull" )
    try
    {
        ASSERT_TRUE( QuickHullDegenerateSets( 6203, ScaledCount( 150 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConvexHull: quickhull 3D degenerate sets (empty, point, line, plane, not finite) and a tetrahedron" )
    try
    {
        ASSERT_TRUE( QuickHullLowerDimensions() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "ConvexHull: quickhull 3D of thousands of points of a ball and of a sphere is a closed polyhedron with every point inside" )
    try
    {
        ASSERT_TRUE( QuickHullLarge( 6204, ScaledCount( 20000 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
