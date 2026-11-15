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
#include <limits>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H
#include "gucefGEOMETRY_CTConstrainedDelaunay2D.h"
#define GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H
#endif /* GUCEF_GEOMETRY_CTCONSTRAINEDDELAUNAY2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#include "gucefGEOMETRY_CTPolygonOffsetter.h"
#define GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H ? */

#include "TestConvexReference.h"
#include "TestPolygonReference.h"
#include "TestDelaunay.h"

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
using namespace PolygonReference;
using ConvexReference::ScaledCount;
using ConvexReference::CRawHash;

/*
 *  Functions marked GUCEF_TEST_CONTRACTING are compiled for FMA with floating-point contraction enabled (as in
 *  TestQueryFpContraction.cpp): the constructions of the triangulation, the clipper and the offsetter run in the module,
 *  so calls from contracting code must give the same bits.
 */
#if ( defined( __GNUC__ ) || defined( __clang__ ) ) && ( defined( __x86_64__ ) || defined( __i386__ ) )
  #define GUCEF_TEST_CONTRACTION_SUPPORTED
  #if defined( __clang__ )
    #define GUCEF_TEST_CONTRACTING  __attribute__(( target( "fma" ), noinline ))
    #define GUCEF_TEST_CONTRACT_HERE  _Pragma( "clang fp contract(fast)" )
  #else
    #define GUCEF_TEST_CONTRACTING  __attribute__(( target( "fma" ), optimize( "fp-contract=fast" ), noinline ))
    #define GUCEF_TEST_CONTRACT_HERE
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The canonical triangles of the cocircular grid, pinned: identical for every kind, build, compiler and platform */
static const UInt64 PinnedGridHash = 7255649129618155039ULL;
static const UInt64 PinnedGridConstrainedHash = 5653910484062145327ULL;

/*-------------------------------------------------------------------------*/

/** The squared Float64 distance from p to the segment a-b */
static Float64
SegmentDistanceSquared( Float64 px, Float64 py, Float64 ax, Float64 ay, Float64 bx, Float64 by )
{
    const Float64 dx = bx - ax, dy = by - ay;
    const Float64 length = dx * dx + dy * dy;
    Float64 t = length > 0.0 ? ( ( px - ax ) * dx + ( py - ay ) * dy ) / length : 0.0;
    t = t < 0.0 ? 0.0 : ( t > 1.0 ? 1.0 : t );
    const Float64 ex = ax + t * dx - px, ey = ay + t * dy - py;
    return ex * ex + ey * ey;
}

/*-------------------------------------------------------------------------*/

/** Orders points by x, then y */
template < typename T >
static bool
ReferenceLexLess( const CTPoint2D< T >& a, const CTPoint2D< T >& b )
{
    return a.x < b.x || ( a.x == b.x && a.y < b.y );
}

/**
 *  The convex hull of the points, counter-clockwise without collinear points, by Andrew's monotone chain with the
 *  reference orientation (independent of the module's ConvexHull2D)
 */
template < typename T >
static void
ReferenceHull( GUCEF::vector< CTPoint2D< T > > points, GUCEF::vector< CTPoint2D< T > >& hull )
{
    std::sort( points.begin(), points.end(), ReferenceLexLess< T > );
    hull.clear();
    if ( points.size() < 3 )
    {
        hull = points;
        return;
    }
    hull.resize( 2 * points.size() );
    size_t k = 0;
    for ( size_t i=0; i<points.size(); ++i )
    {
        while ( k >= 2 && ReferenceOrient( hull[ k - 2 ], hull[ k - 1 ], points[ i ] ) <= 0 )
            --k;
        hull[ k++ ] = points[ i ];
    }
    const size_t lower = k + 1;
    for ( size_t i=points.size()-1; i>0; --i )
    {
        while ( k >= lower && ReferenceOrient( hull[ k - 2 ], hull[ k - 1 ], points[ i - 1 ] ) <= 0 )
            --k;
        hull[ k++ ] = points[ i - 1 ];
    }
    hull.resize( k - 1 );
}

/*-------------------------------------------------------------------------*/

/**
 *  Checks a triangulation: counter-clockwise triangles; every directed edge once; V - E + F = 1; every distinct input
 *  point a vertex; the triangles tile the convex hull of the vertices (area, exact as fractions when 'exactArea');
 *  every interior edge that is not constrained locally Delaunay with the exact InCircle (<= 0); and every constraint
 *  present as a chain of constrained edges between its end points through vertices on the segment (exactly on it, or
 *  within 'tolerance' when that is not 0). The predicates and the hull are the test's own references, not the module's.
 */
template < typename T >
static bool
CheckTriangulation( const char* name, const char* context, const CTConstrainedDelaunay2D< T >& cdt, const UInt32* constraints, UInt32 constraintCount, Float64 tolerance, bool exactArea )
{
    typedef CTPoint2D< T > TPoint;
    const GUCEF::vector< UInt32 >& tris = cdt.GetTriangles();
    const GUCEF::vector< TPoint >& vertices = cdt.GetVertices();
    const UInt32 vertexCount = (UInt32) vertices.size();
    // directed edges: key a * count + b -> the opposite vertex
    GUCEF::map< UInt64, UInt32 > opposite;
    GUCEF::vector< UInt8 > used( vertexCount, 0 );
    for ( size_t t=0; t<tris.size(); t+=3 )
    {
        if ( ReferenceOrient( vertices[ tris[ t ] ], vertices[ tris[ t + 1 ] ], vertices[ tris[ t + 2 ] ] ) <= 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": triangle " << t / 3 << " is not counter-clockwise" );
            return false;
        }
        for ( UInt32 k=0; k<3; ++k )
        {
            const UInt64 key = (UInt64) tris[ t + k ] * vertexCount + tris[ t + ( k + 1 ) % 3 ];
            if ( opposite.find( key ) != opposite.end() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": a directed edge is used twice" );
                return false;
            }
            opposite[ key ] = tris[ t + ( k + 2 ) % 3 ];
            used[ tris[ t + k ] ] = 1;
        }
    }
    UInt32 usedCount = 0;
    for ( UInt32 v=0; v<vertexCount; ++v )
    {
        const UInt32 representative = v < cdt.GetVertexCount() ? cdt.GetVertexOfInput( v ) : v;
        if ( representative == v && 0 == used[ v ] )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": vertex " << v << " is not used" );
            return false;
        }
        if ( representative != v && 0 != used[ v ] )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": a duplicate vertex is used" );
            return false;
        }
        usedCount += used[ v ];
    }
    UInt32 edgeCount = 0;
    GUCEF::vector< CTPoint2D< T > > usedPoints;
    for ( GUCEF::map< UInt64, UInt32 >::const_iterator i=opposite.begin(); i!=opposite.end(); ++i )
    {
        const UInt32 a = (UInt32) ( (*i).first / vertexCount ), b = (UInt32) ( (*i).first % vertexCount );
        GUCEF::map< UInt64, UInt32 >::const_iterator twin = opposite.find( (UInt64) b * vertexCount + a );
        if ( twin == opposite.end() || a < b )
            ++edgeCount;
        if ( twin == opposite.end() || cdt.IsConstrained( a, b ) )
            continue;
        if ( ReferenceInCircle( vertices[ a ], vertices[ b ], vertices[ (*i).second ], vertices[ (*twin).second ] ) > 0 )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": edge " << a << " - " << b << " is not locally Delaunay" );
            return false;
        }
    }
    const Int64 euler = (Int64) usedCount - (Int64) edgeCount + (Int64) ( tris.size() / 3 );
    if ( 1 != euler )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": V - E + F = " << euler );
        return false;
    }
    for ( UInt32 v=0; v<vertexCount; ++v )
        if ( 0 != used[ v ] )
            usedPoints.push_back( vertices[ v ] );
    GUCEF::vector< CTPoint2D< T > > hull;
    ReferenceHull( usedPoints, hull );
    if ( exactArea )
    {
        bool positive = false;
        const F total = TrianglesDoubleArea( &vertices[ 0 ], tris, positive );
        const F hullArea = RingDoubleArea( &hull[ 0 ], (UInt32) hull.size() );
        if ( !( total == hullArea ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": the triangles do not tile the convex hull" );
            return false;
        }
    }
    else
    {
        Float64 total = 0.0, hullArea = 0.0;
        typedef MATH::CTNumericTraits< T > TTraits;
        for ( size_t t=0; t<tris.size(); t+=3 )
        {
            const TPoint& a = vertices[ tris[ t ] ];
            const TPoint& b = vertices[ tris[ t + 1 ] ];
            const TPoint& c = vertices[ tris[ t + 2 ] ];
            total += ( TTraits::ToFloat64( b.x ) - TTraits::ToFloat64( a.x ) ) * ( TTraits::ToFloat64( c.y ) - TTraits::ToFloat64( a.y ) ) -
                     ( TTraits::ToFloat64( b.y ) - TTraits::ToFloat64( a.y ) ) * ( TTraits::ToFloat64( c.x ) - TTraits::ToFloat64( a.x ) );
        }
        for ( size_t i=1; i+1<hull.size(); ++i )
            hullArea += ( TTraits::ToFloat64( hull[ i ].x ) - TTraits::ToFloat64( hull[ 0 ].x ) ) * ( TTraits::ToFloat64( hull[ i + 1 ].y ) - TTraits::ToFloat64( hull[ 0 ].y ) ) -
                        ( TTraits::ToFloat64( hull[ i ].y ) - TTraits::ToFloat64( hull[ 0 ].y ) ) * ( TTraits::ToFloat64( hull[ i + 1 ].x ) - TTraits::ToFloat64( hull[ 0 ].x ) );
        if ( fabs( total - hullArea ) > 1e-9 * fabs( hullArea ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": the triangle area " << total << " differs from the hull area " << hullArea );
            return false;
        }
    }

    // constraints as chains of constrained edges through vertices on the segment
    const GUCEF::vector< UInt32 >& constrained = cdt.GetConstrainedEdges();
    GUCEF::vector< GUCEF::vector< UInt32 > > adjacency( vertexCount );
    for ( size_t i=0; i<constrained.size(); i+=2 )
    {
        adjacency[ constrained[ i ] ].push_back( constrained[ i + 1 ] );
        adjacency[ constrained[ i + 1 ] ].push_back( constrained[ i ] );
    }
    typedef MATH::CTNumericTraits< T > TTraits;
    for ( UInt32 c=0; c<constraintCount; ++c )
    {
        const UInt32 u = cdt.GetVertexOfInput( constraints[ 2 * c ] );
        const UInt32 v = cdt.GetVertexOfInput( constraints[ 2 * c + 1 ] );
        if ( u == v )
            continue;
        const TPoint& pu = vertices[ u ];
        const TPoint& pv = vertices[ v ];
        GUCEF::vector< UInt8 > seen( vertexCount, 0 );
        GUCEF::vector< UInt32 > stack( 1, u );
        seen[ u ] = 1;
        bool found = false;
        while ( !stack.empty() && !found )
        {
            const UInt32 w = stack.back();
            stack.pop_back();
            for ( size_t j=0; j<adjacency[ w ].size(); ++j )
            {
                const UInt32 x = adjacency[ w ][ j ];
                if ( 0 != seen[ x ] )
                    continue;
                const TPoint& px = vertices[ x ];
                bool onSegment = false;
                if ( 0.0 == tolerance )
                    onSegment = 0 == ReferenceOrient( pu, pv, px ) && SegmentsMeet( pu, pv, px, px );
                else
                    onSegment = SegmentDistanceSquared( TTraits::ToFloat64( px.x ), TTraits::ToFloat64( px.y ), TTraits::ToFloat64( pu.x ), TTraits::ToFloat64( pu.y ),
                                                        TTraits::ToFloat64( pv.x ), TTraits::ToFloat64( pv.y ) ) <= tolerance * tolerance;
                if ( !onSegment )
                    continue;
                seen[ x ] = 1;
                if ( x == v )
                {
                    found = true;
                    break;
                }
                stack.push_back( x );
            }
        }
        if ( !found )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": constraint " << c << " (" << u << " - " << v << ") is not present" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** FNV-1a of the canonical triangles */
static UInt64
TriangleHash( const GUCEF::vector< UInt32 >& triangles )
{
    CRawHash hash;
    for ( size_t i=0; i<triangles.size(); ++i )
        hash.Add( triangles[ i ] );
    return hash.GetHash();
}

/*-------------------------------------------------------------------------*/

/** The canonical triangles after renaming the vertices through 'map' (rotated to the smallest index, sorted) */
static GUCEF::vector< UInt32 >
Renamed( const GUCEF::vector< UInt32 >& triangles, const GUCEF::vector< UInt32 >& map )
{
    GUCEF::vector< UInt64 > keys;
    for ( size_t t=0; t<triangles.size(); t+=3 )
    {
        UInt32 v[ 3 ] = { map[ triangles[ t ] ], map[ triangles[ t + 1 ] ], map[ triangles[ t + 2 ] ] };
        UInt32 first = 0;
        if ( v[ 1 ] < v[ first ] ) first = 1;
        if ( v[ 2 ] < v[ first ] ) first = 2;
        keys.push_back( ( (UInt64) v[ first ] << 42 ) | ( (UInt64) v[ ( first + 1 ) % 3 ] << 21 ) | (UInt64) v[ ( first + 2 ) % 3 ] );
    }
    std::sort( keys.begin(), keys.end() );
    GUCEF::vector< UInt32 > result;
    for ( size_t i=0; i<keys.size(); ++i )
    {
        result.push_back( (UInt32) ( keys[ i ] >> 42 ) );
        result.push_back( (UInt32) ( ( keys[ i ] >> 21 ) & 0x1FFFFF ) );
        result.push_back( (UInt32) ( keys[ i ] & 0x1FFFFF ) );
    }
    return result;
}

/*-------------------------------------------------------------------------*/

/**
 *  The cocircular grid: 'side' x 'side' grid points (every unit square is cocircular; 'side' odd), triangulated in kind T in the
 *  input order and in a seeded permutation; the two must agree and their hash is returned. With 'constrained' the grid
 *  diagonals and a few axis lines are constraints (they pass through grid points, so no Steiner points arise).
 */
template < typename T >
static bool
CocircularGrid( const char* name, UInt32 side, T ( *make )( G ), bool constrained, UInt64 seed, UInt64& hash )
{
    GUCEF::vector< CTPoint2D< T > > points;
    for ( UInt32 y=0; y<side; ++y )
        for ( UInt32 x=0; x<side; ++x )
            points.push_back( CTPoint2D< T >( make( (G) x ), make( (G) y ) ) );
    GUCEF::vector< UInt32 > constraints;
    if ( constrained )
    {
        const UInt32 last = side - 1;
        const UInt32 lines[] = { 0, last * side + last, last, last * side, side / 2, ( last ) * side + side / 2, ( side / 3 ) * side, ( side / 3 ) * side + last, 0, last };
        constraints.assign( lines, lines + 10 );
    }
    CTConstrainedDelaunay2D< T > cdt;
    const TDelaunayStatus status = cdt.Build( &points[ 0 ], (UInt32) points.size(), constraints.empty() ? GUCEF_NULL : &constraints[ 0 ], (UInt32) constraints.size() / 2 );
    if ( DELAUNAY_OK != status || !CheckTriangulation( name, "grid", cdt, constraints.empty() ? GUCEF_NULL : &constraints[ 0 ], (UInt32) constraints.size() / 2, 0.0, true ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the grid failed, status " << (Int32) status );
        return false;
    }
    if ( cdt.GetTriangleCount() != 2 * ( side - 1 ) * ( side - 1 ) || 0 != cdt.GetSteinerPointCount() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the grid has " << cdt.GetTriangleCount() << " triangles" );
        return false;
    }
    hash = TriangleHash( cdt.GetTriangles() );

    // a permutation of the input gives the same triangles after renaming
    QueryReference::CRandom random( seed );
    GUCEF::vector< UInt32 > permutation( points.size() );
    for ( UInt32 i=0; i<(UInt32) points.size(); ++i )
        permutation[ i ] = i;
    for ( UInt32 i=(UInt32) points.size() - 1; i>0; --i )
    {
        const Int32 j = random.Index( (Int32) i + 1 );
        std::swap( permutation[ i ], permutation[ (UInt32) j ] );
    }
    GUCEF::vector< CTPoint2D< T > > shuffled( points.size() );
    GUCEF::vector< UInt32 > back( points.size() );
    for ( UInt32 i=0; i<(UInt32) points.size(); ++i )
    {
        shuffled[ i ] = points[ permutation[ i ] ];
        back[ i ] = permutation[ i ];
    }
    GUCEF::vector< UInt32 > shuffledConstraints;
    GUCEF::vector< UInt32 > forward( points.size() );
    for ( UInt32 i=0; i<(UInt32) points.size(); ++i )
        forward[ permutation[ i ] ] = i;
    for ( size_t i=0; i<constraints.size(); ++i )
        shuffledConstraints.push_back( forward[ constraints[ i ] ] );
    CTConstrainedDelaunay2D< T > other;
    other.Build( &shuffled[ 0 ], (UInt32) shuffled.size(), shuffledConstraints.empty() ? GUCEF_NULL : &shuffledConstraints[ 0 ], (UInt32) shuffledConstraints.size() / 2 );
    if ( Renamed( other.GetTriangles(), back ) != cdt.GetTriangles() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the grid triangulation depends on the input order, seed " << seed );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** The pinned grid hashes in every kind */
static bool
PinnedGrids( void )
{
    UInt64 hashes[ 5 ] = { 0, 0, 0, 0, 0 };
    UInt64 constrainedHashes[ 5 ] = { 0, 0, 0, 0, 0 };
    bool ok = true;
    ok = CocircularGrid< Int32 >( "Int32", 25, MakeInt32, false, 8001, hashes[ 0 ] ) && ok;
    ok = CocircularGrid< Float64 >( "Float64", 25, MakeFloat64, false, 8002, hashes[ 1 ] ) && ok;
    ok = CocircularGrid< Float32 >( "Float32", 25, MakeFloat32, false, 8003, hashes[ 2 ] ) && ok;
    ok = CocircularGrid< TFixed32_32 >( "TFixed32_32", 25, MakeFixed, false, 8004, hashes[ 3 ] ) && ok;
    ok = CocircularGrid< F >( "TMathBigFraction", 25, MakeFraction, false, 8005, hashes[ 4 ] ) && ok;
    ok = CocircularGrid< Int32 >( "Int32", 25, MakeInt32, true, 8006, constrainedHashes[ 0 ] ) && ok;
    ok = CocircularGrid< Float64 >( "Float64", 25, MakeFloat64, true, 8007, constrainedHashes[ 1 ] ) && ok;
    ok = CocircularGrid< Float32 >( "Float32", 25, MakeFloat32, true, 8008, constrainedHashes[ 2 ] ) && ok;
    ok = CocircularGrid< TFixed32_32 >( "TFixed32_32", 25, MakeFixed, true, 8009, constrainedHashes[ 3 ] ) && ok;
    ok = CocircularGrid< F >( "TMathBigFraction", 25, MakeFraction, true, 8010, constrainedHashes[ 4 ] ) && ok;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: cocircular grid hash " << hashes[ 0 ] << " constrained " << constrainedHashes[ 0 ] );
    for ( UInt32 k=0; k<5; ++k )
    {
        if ( hashes[ k ] != PinnedGridHash || constrainedHashes[ k ] != PinnedGridConstrainedHash )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: kind " << k << " grid hash " << hashes[ k ] << " / " << constrainedHashes[ k ] << " differs from the pinned values" );
            ok = false;
        }
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Seeded random points with random (crossing) constraints */
template < typename T >
static bool
RandomSets( const char* name, UInt64 seed, UInt32 sets, UInt32 pointCount, UInt32 constraintCount, T ( *make )( G ), Float64 range, Float64 tolerance, bool exactArea )
{
    QueryReference::CRandom random( seed );
    CTConstrainedDelaunay2D< T > cdt;
    for ( UInt32 set=0; set<sets; ++set )
    {
        GUCEF::vector< CTPoint2D< T > > points;
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            const Float64 x = random.Uniform( -range, range );
            const Float64 y = random.Uniform( -range, range );
            points.push_back( CTPoint2D< T >( make( (G) floor( x ) ), make( (G) floor( y ) ) ) );
        }
        // some duplicates
        for ( UInt32 i=0; i<pointCount / 50; ++i )
        {
            const Int32 a = random.Index( (Int32) pointCount );
            points.push_back( points[ (UInt32) a ] );
        }
        GUCEF::vector< UInt32 > constraints;
        for ( UInt32 c=0; c<constraintCount; ++c )
        {
            const Int32 a = random.Index( (Int32) points.size() );
            const Int32 b = random.Index( (Int32) points.size() );
            constraints.push_back( (UInt32) a );
            constraints.push_back( (UInt32) b );
        }
        const Float64 start = NowMs();
        const TDelaunayStatus status = cdt.Build( &points[ 0 ], (UInt32) points.size(), constraints.empty() ? GUCEF_NULL : &constraints[ 0 ], constraintCount );
        const Float64 elapsed = NowMs() - start;
        const CORE::CString context = "seed " + CORE::ToString( seed ) + " set " + CORE::ToString( set );
        if ( DELAUNAY_OK != status )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": status " << (Int32) status );
            return false;
        }
        if ( !CheckTriangulation( name, context.C_String(), cdt, constraints.empty() ? GUCEF_NULL : &constraints[ 0 ], constraintCount, tolerance, exactArea ) )
            return false;
        if ( 0 == set )
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: " << name << ": " << points.size() << " points with " << constraintCount << " constraints took " << elapsed << " ms, " << cdt.GetSteinerPointCount() << " Steiner points, " << context );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Small dense sets on a 6 x 6 grid of integer values with up to nine distinct crossing constraints: in the rounding
 *  kinds the crossings often round onto or next to existing vertices, so both constraints are rerouted through that
 *  point and the edge they replace loses its constraint flag. The result must be a constrained Delaunay
 *  triangulation (status OK, every unconstrained edge locally Delaunay, constraints present as chains).
 */
template < typename T >
static bool
DenseCrossings( const char* name, UInt64 seed, UInt32 sets, T ( *make )( G ), G scale, Float64 tolerance )
{
    QueryReference::CRandom random( seed );
    CTConstrainedDelaunay2D< T > cdt;
    UInt32 rerouted = 0;
    for ( UInt32 set=0; set<sets; ++set )
    {
        const Int32 extra = random.Index( 20 );
        const UInt32 pointCount = 6 + (UInt32) extra;
        GUCEF::vector< CTPoint2D< T > > points;
        UInt8 taken[ 36 ] = { 0 };
        while ( points.size() < pointCount )
        {
            const Int32 x = random.Index( 6 );
            const Int32 y = random.Index( 6 );
            if ( 0 != taken[ 6 * y + x ] )
                continue;
            taken[ 6 * y + x ] = 1;
            points.push_back( CTPoint2D< T >( make( (G) x * scale ), make( (G) y * scale ) ) );
        }
        const Int32 drawn = random.Index( 9 );
        const UInt32 constraintCount = 1 + (UInt32) drawn;
        GUCEF::vector< UInt32 > constraints;
        for ( UInt32 c=0; c<constraintCount; ++c )
        {
            const Int32 a = random.Index( (Int32) pointCount );
            const Int32 b = random.Index( (Int32) pointCount );
            bool repeated = a == b;
            for ( size_t i=0; i<constraints.size() && !repeated; i+=2 )
                repeated = ( constraints[ i ] == (UInt32) a && constraints[ i + 1 ] == (UInt32) b ) || ( constraints[ i ] == (UInt32) b && constraints[ i + 1 ] == (UInt32) a );
            if ( repeated )
                continue;
            constraints.push_back( (UInt32) a );
            constraints.push_back( (UInt32) b );
        }
        const UInt32 used = (UInt32) constraints.size() / 2;
        const UInt32* constraintData = constraints.empty() ? GUCEF_NULL : &constraints[ 0 ];
        const TDelaunayStatus status = cdt.Build( &points[ 0 ], pointCount, constraintData, used );
        const CORE::CString context = "seed " + CORE::ToString( seed ) + " set " + CORE::ToString( set );
        if ( DELAUNAY_OK != status && DELAUNAY_DEGENERATE != status )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << context << ": status " << (Int32) status );
            return false;
        }
        if ( DELAUNAY_DEGENERATE == status )
            continue;
        if ( !CheckTriangulation( name, context.C_String(), cdt, constraintData, used, tolerance, true ) )
            return false;
        // more constrained edges than constraints: a constraint was split at a crossing or a vertex
        if ( cdt.GetConstrainedEdges().size() / 2 > used )
            ++rerouted;
    }
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: " << name << ": " << sets << " dense sets with crossing constraints, " << rerouted << " with split constraints" );
    return true;
}

/*-------------------------------------------------------------------------*/

/** Degenerate input, small sets, insertion after the build */
static bool
SmallCases( void )
{
    typedef CTPoint2D< Float64 > P;
    CTConstrainedDelaunay2D< Float64 > cdt;
    bool ok = true;
    const P line[] = { P( 0, 0 ), P( 1, 1 ), P( 2, 2 ), P( 3, 3 ) };
    ok = DELAUNAY_DEGENERATE == cdt.Build( line, 4 ) && 0 == cdt.GetTriangleCount() && ok;
    const P same[] = { P( 1, 1 ), P( 1, 1 ), P( 2, 2 ) };
    ok = DELAUNAY_DEGENERATE == cdt.Build( same, 3 ) && ok;
    ok = DELAUNAY_DEGENERATE == cdt.Build( GUCEF_NULL, 0 ) && ok;
    ok = DELAUNAY_INVALID_INPUT == cdt.Build( GUCEF_NULL, 3 ) && ok;
    const P notFinite[] = { P( 0, 0 ), P( 1, 0 ), P( 0, std::numeric_limits< Float64 >::infinity() ) };
    ok = DELAUNAY_INVALID_INPUT == cdt.Build( notFinite, 3 ) && ok;
    const P triangle[] = { P( 0, 0 ), P( 4, 0 ), P( 0, 4 ) };
    const UInt32 badConstraint[] = { 0, 7 };
    ok = DELAUNAY_INVALID_INPUT == cdt.Build( triangle, 3, badConstraint, 1 ) && ok;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: degenerate or invalid input not reported" );

    // one triangle (clockwise input), then points inside, on an edge, on a hull line beyond the hull, outside
    const P clockwise[] = { P( 0, 0 ), P( 0, 4 ), P( 4, 0 ) };
    bool small = DELAUNAY_OK == cdt.Build( clockwise, 3 ) && 1 == cdt.GetTriangleCount();
    UInt32 v = 0;
    small = DELAUNAY_OK == cdt.InsertPoint( P( 1, 1 ), v ) && 3 == v && 3 == cdt.GetTriangleCount() && small;
    small = DELAUNAY_OK == cdt.InsertPoint( P( 2, 0 ), v ) && 4 == v && small;
    small = DELAUNAY_OK == cdt.InsertPoint( P( 6, 0 ), v ) && 5 == v && small;
    small = DELAUNAY_OK == cdt.InsertPoint( P( -3, -3 ), v ) && 6 == v && small;
    small = DELAUNAY_OK == cdt.InsertPoint( P( 1, 1 ), v ) && 3 == v && 7 == cdt.GetVertexCount() && small;
    small = DELAUNAY_INVALID_INPUT == cdt.InsertPoint( P( std::numeric_limits< Float64 >::quiet_NaN(), 0 ), v ) && small;
    small = CheckTriangulation( "Float64", "inserted points", cdt, GUCEF_NULL, 0, 0.0, true ) && small;
    // a constraint after the build, then a point on it: the halves stay constrained
    const UInt32 diagonal[] = { 6, 1 };
    small = DELAUNAY_OK == cdt.InsertConstraint( 6, 1 ) && cdt.IsConstrained( 6, 1 ) == cdt.HasEdge( 6, 1 ) && small;
    small = CheckTriangulation( "Float64", "constraint after build", cdt, diagonal, 1, 0.0, true ) && small;
    small = DELAUNAY_INVALID_INPUT == cdt.InsertConstraint( 0, 99 ) && small;
    if ( !small )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: the small incremental case failed" );
    ok = small && ok;

    // collinear points on the hull and a constraint through collinear vertices
    const P square[] = { P( 0, 0 ), P( 1, 0 ), P( 2, 0 ), P( 3, 0 ), P( 3, 3 ), P( 0, 3 ), P( 0, 1 ), P( 1, 1 ), P( 2, 2 ) };
    const UInt32 through[] = { 0, 8, 5, 3 };
    bool collinear = DELAUNAY_OK == cdt.Build( square, 9, through, 2 );
    collinear = CheckTriangulation( "Float64", "collinear hull", cdt, through, 2, 0.0, true ) && collinear;
    collinear = cdt.IsConstrained( 0, 7 ) && 1 == cdt.GetSteinerPointCount() && collinear;
    if ( !collinear )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: the collinear case failed" );
    ok = collinear && ok;

    // two crossing constraints in an exact kind: the Steiner point is the exact intersection
    typedef CTPoint2D< F > PF;
    const PF quad[] = { PF( F( 0, 1 ), F( 0, 1 ) ), PF( F( 7, 1 ), F( 1, 1 ) ), PF( F( 6, 1 ), F( 5, 1 ) ), PF( F( 1, 1 ), F( 6, 1 ) ), PF( F( 3, 1 ), F( -2, 1 ) ) };
    const UInt32 crossing[] = { 0, 2, 1, 3 };
    CTConstrainedDelaunay2D< F > exact;
    bool exactOk = DELAUNAY_OK == exact.Build( quad, 5, crossing, 2 ) && 1 == exact.GetSteinerPointCount();
    exactOk = exactOk && CheckTriangulation( "TMathBigFraction", "crossing constraints", exact, crossing, 2, 0.0, true );
    if ( exactOk )
    {
        const PF& s = exact.GetVertices()[ 5 ];
        exactOk = 0 == ReferenceOrient( quad[ 0 ], quad[ 2 ], s ) && 0 == ReferenceOrient( quad[ 1 ], quad[ 3 ], s );
    }
    if ( !exactOk )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: the exact crossing failed" );
    ok = exactOk && ok;
    return ok;
}


/*-------------------------------------------------------------------------*/

/** The outputs whose constructions are rounded: Steiner vertices of crossing constraints, round offsets */
struct SConstructionResults
{
    GUCEF::vector< CTPoint2D< Float64 > > vertices;
    GUCEF::vector< UInt32 > triangles;
    GUCEF::vector< GUCEF::vector< CTPoint2D< Float64 > > > offset;
};

#define GUCEF_TEST_BUILD_CONSTRUCTIONS( points, count, constraints, constraintCount, out )                  \
    {                                                                                                       \
        CTConstrainedDelaunay2D< Float64 > cdt;                                                             \
        cdt.Build( points, count, constraints, constraintCount );                                           \
        out.vertices = cdt.GetVertices();                                                                   \
        out.triangles = cdt.GetTriangles();                                                                 \
        CTPolygonOffsetter< Float64 > offsetter;                                                            \
        offsetter.AddPolygon( points, 40 );                                                                 \
        offsetter.AddPolyline( points + 40, 20 );                                                           \
        offsetter.Execute( 3.25, POLYGONJOIN_ROUND, POLYLINEEND_ROUND, out.offset );                        \
    }

#ifdef GUCEF_TEST_CONTRACTION_SUPPORTED

static void
BuildConstructionsPlain( const CTPoint2D< Float64 >* points, UInt32 count, const UInt32* constraints, UInt32 constraintCount, SConstructionResults& out )
GUCEF_TEST_BUILD_CONSTRUCTIONS( points, count, constraints, constraintCount, out )

/** a * b + c written inline in a contracting function: fused when the compiler contracts */
GUCEF_TEST_CONTRACTING static Float64
ContractedMultiplyAdd( Float64 a, Float64 b, Float64 c )
{
    GUCEF_TEST_CONTRACT_HERE
    return a * b + c;
}

GUCEF_TEST_CONTRACTING static void
BuildConstructionsContracted( const CTPoint2D< Float64 >* points, UInt32 count, const UInt32* constraints, UInt32 constraintCount, SConstructionResults& out )
{
    GUCEF_TEST_CONTRACT_HERE
    GUCEF_TEST_BUILD_CONSTRUCTIONS( points, count, constraints, constraintCount, out )
}

static bool
CpuHasFma( void )
{
    __builtin_cpu_init();
    return 0 != __builtin_cpu_supports( "fma" );
}

/** Seeded points with crossing constraints and an offset, built from plain and from contracting code: bit identical */
static bool
ContractionInvariance( UInt64 seed )
{
    QueryReference::CRandom random( seed );
    GUCEF::vector< CTPoint2D< Float64 > > points;
    for ( UInt32 i=0; i<400; ++i )
    {
        const Float64 x = random.Uniform( -1000.0, 1000.0 );
        const Float64 y = random.Uniform( -1000.0, 1000.0 );
        points.push_back( CTPoint2D< Float64 >( x, y ) );
    }
    GUCEF::vector< UInt32 > constraints;
    for ( UInt32 c=0; c<40; ++c )
    {
        const Int32 a = random.Index( 400 );
        const Int32 b = random.Index( 400 );
        constraints.push_back( (UInt32) a );
        constraints.push_back( (UInt32) b );
    }
    SConstructionResults plain;
    SConstructionResults contracted;
    BuildConstructionsPlain( &points[ 0 ], 400, &constraints[ 0 ], 40, plain );
    BuildConstructionsContracted( &points[ 0 ], 400, &constraints[ 0 ], 40, contracted );
    bool same = plain.triangles == contracted.triangles && plain.vertices.size() == contracted.vertices.size() && plain.offset.size() == contracted.offset.size();
    for ( size_t i=0; same && i<plain.vertices.size(); ++i )
        same = 0 == memcmp( &plain.vertices[ i ].x, &contracted.vertices[ i ].x, sizeof( Float64 ) ) && 0 == memcmp( &plain.vertices[ i ].y, &contracted.vertices[ i ].y, sizeof( Float64 ) );
    for ( size_t r=0; same && r<plain.offset.size(); ++r )
        same = plain.offset[ r ] == contracted.offset[ r ];
    if ( !same )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Delaunay: the constructions differ when called from contracting code, seed " << seed );
    return same && plain.vertices.size() > 400;
}

#endif /* GUCEF_TEST_CONTRACTION_SUPPORTED ? */

/*-------------------------------------------------------------------------*/

void
PerformDelaunayTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "Delaunay" );

    GUCEF_TESTFW_TESTCASE( "Delaunay: degenerate input, incremental points inside, on edges and outside the hull, constraints after the build, exact crossings" )
    try
    {
        ASSERT_TRUE( SmallCases() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delaunay: the cocircular grid gives the same triangles in every kind and input order (pinned hash), with and without constraints" )
    try
    {
        ASSERT_TRUE( PinnedGrids() );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delaunay: seeded points with crossing constraints: locally Delaunay, constraints present, hull tiled, in every kind" )
    try
    {
        ASSERT_TRUE( ( RandomSets< Float64 >( "Float64", 8101, ScaledCount( 40 ), 300, 12, MakeFloat64, 4000.0, 1e-9, true ) ) );
        ASSERT_TRUE( ( RandomSets< Float32 >( "Float32", 8102, ScaledCount( 40 ), 300, 12, MakeFloat32, 4000.0, 1e-3, true ) ) );
        // integer crossings are rounded to the grid: the chains stay within a few grid units of the segments
        ASSERT_TRUE( ( RandomSets< Int32 >( "Int32", 8103, ScaledCount( 40 ), 300, 12, MakeInt32, 4000.0, 2.0, true ) ) );
        ASSERT_TRUE( ( RandomSets< TFixed32_32 >( "TFixed32_32", 8104, ScaledCount( 40 ), 300, 12, MakeFixed, 4000.0, 1e-6, true ) ) );
        ASSERT_TRUE( ( RandomSets< F >( "TMathBigFraction", 8105, ScaledCount( 4 ), 60, 6, MakeFraction, 400.0, 0.0, true ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delaunay: small dense sets with crossing constraints rounded onto or near vertices stay constrained Delaunay (Int32, TFixed32_32)" )
    try
    {
        ASSERT_TRUE( ( DenseCrossings< Int32 >( "Int32", 8111, ScaledCount( 3000 ), MakeInt32, 1, 2.0 ) ) );
        ASSERT_TRUE( ( DenseCrossings< TFixed32_32 >( "TFixed32_32", 8112, ScaledCount( 3000 ), MakeFixed, 8, 1e-6 ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delaunay: Steiner points, clipper and offsetter constructions give the same bits from contracting FMA code" )
    #ifndef GUCEF_TEST_CONTRACTION_SUPPORTED
    GUCEF_TESTFW_SKIP( "contraction test needs GCC or Clang on x86" );
    #else
    GUCEF_TESTFW_SKIP_IF( !CpuHasFma(), "the CPU has no FMA" );
    {
        volatile Float64 a64 = 1.0 + ldexp( 1.0, -27 );
        volatile Float64 c64 = 1.0 + ldexp( 1.0, -26 );
        GUCEF_TESTFW_SKIP_IF( ldexp( 1.0, -54 ) != ContractedMultiplyAdd( a64, a64, -c64 ), "this build does not contract a * b + c (no optimisation), the test cannot detect fusion" );
    }
    try
    {
        ASSERT_TRUE( ContractionInvariance( 8107 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    #endif /* GUCEF_TEST_CONTRACTION_SUPPORTED ? */
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "Delaunay: 10^4 seeded points with 100 constraints (timing reported)" )
    try
    {
        ASSERT_TRUE( ( RandomSets< Float64 >( "Float64", 8106, 1, ScaledCount( 10000 ), 100, MakeFloat64, 1e7, 1e-6, false ) ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
