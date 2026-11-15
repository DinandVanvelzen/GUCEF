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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTCONVEXREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTCONVEXREFERENCE_H

/*
 *  Test helpers of the GJK, EPA, SAT, sweep and time of impact tests of the gucefGEOMETRY test application, sharing no
 *  code with the functions under test: polytopes in Float64 (a brute-force hull of a point set: every triangle of
 *  vertices with all other points on one side), the brute-force polytope distance (minimum over the vertex-triangle
 *  and edge-edge pairs, with their own closest point routines), the separating axis overlap of polytopes, an FNV-1a
 *  hash of fixed-point raw values for the pinned bit-exact runs, and a seeded source of fixed-point values that uses
 *  integer arithmetic only. Specific to gucefGEOMETRY, so it lives with its test application.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#include "TestQueryReference.h"

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

#ifndef GUCEF_MATH_CSPLITMIX64_H
#include "gucefMATH_CSplitMix64.h"
#define GUCEF_MATH_CSPLITMIX64_H
#endif /* GUCEF_MATH_CSPLITMIX64_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace ConvexReference {

typedef QueryReference::P3 P3;
typedef QueryReference::V3 V3;

/*-------------------------------------------------------------------------*/

/** A convex polytope in Float64: vertices, outward triangles (3 indices each) and edges (2 indices each, each once) */
struct SPolytope
{
    GUCEF::vector< P3 > vertices;
    GUCEF::vector< GUCEF::UInt32 > triangles;
    GUCEF::vector< GUCEF::UInt32 > edges;
};

/*-------------------------------------------------------------------------*/

inline void
AddEdge( SPolytope& p, GUCEF::UInt32 a, GUCEF::UInt32 b )
{
    const GUCEF::UInt32 lo = a < b ? a : b;
    const GUCEF::UInt32 hi = a < b ? b : a;
    for ( size_t i=0; i<p.edges.size(); i+=2 )
        if ( p.edges[ i ] == lo && p.edges[ i + 1 ] == hi )
            return;
    p.edges.push_back( lo );
    p.edges.push_back( hi );
}

/*-------------------------------------------------------------------------*/

/** The hull of the points by brute force: every triangle of points with no other point strictly in front of it */
inline SPolytope
BruteForceHull( const GUCEF::vector< P3 >& points )
{
    SPolytope p;
    p.vertices = points;
    const GUCEF::UInt32 n = (GUCEF::UInt32) points.size();
    GUCEF::Float64 scale = 1.0;
    for ( GUCEF::UInt32 i=0; i<n; ++i )
        scale = GUCEF::MATH::CTNumericTraits< GUCEF::Float64 >::Max( scale, QueryReference::V3( points[ i ].x, points[ i ].y, points[ i ].z ).GetChebyshevLength() );
    const GUCEF::Float64 epsilon = 1.0e-12 * scale * scale * scale;
    for ( GUCEF::UInt32 i=0; i<n; ++i )
        for ( GUCEF::UInt32 j=i+1; j<n; ++j )
            for ( GUCEF::UInt32 k=j+1; k<n; ++k )
            {
                const V3 normal = ( points[ j ] - points[ i ] ).Cross( points[ k ] - points[ i ] );
                if ( normal.GetLength() < 1.0e-12 * scale * scale )
                    continue;
                bool front = false;
                bool behind = false;
                for ( GUCEF::UInt32 m=0; m<n; ++m )
                {
                    const GUCEF::Float64 side = normal.Dot( points[ m ] - points[ i ] );
                    front = front || side > epsilon;
                    behind = behind || side < -epsilon;
                }
                if ( front && behind )
                    continue;
                /* outward: every point behind */
                if ( front )
                {
                    p.triangles.push_back( i );
                    p.triangles.push_back( k );
                    p.triangles.push_back( j );
                }
                else
                {
                    p.triangles.push_back( i );
                    p.triangles.push_back( j );
                    p.triangles.push_back( k );
                }
                AddEdge( p, i, j );
                AddEdge( p, j, k );
                AddEdge( p, i, k );
            }
    return p;
}

/*-------------------------------------------------------------------------*/

/** The polytope of an oriented box in Float64 */
inline SPolytope
BoxPolytope( const GUCEF::GEOMETRY::CTOrientedBox3D< GUCEF::Float64 >& box )
{
    P3 corners[ 8 ];
    box.GetCorners( corners );
    GUCEF::vector< P3 > points( corners, corners + 8 );
    return BruteForceHull( points );
}

/*-------------------------------------------------------------------------*/

/** The closest point of the triangle a, b, c to p, by the Voronoi regions (Ericson 5.1.5), written for the test */
inline P3
ReferenceClosestOnTriangle( const P3& p, const P3& a, const P3& b, const P3& c )
{
    const V3 ab = b - a;
    const V3 ac = c - a;
    const V3 ap = p - a;
    const GUCEF::Float64 d1 = ab.Dot( ap );
    const GUCEF::Float64 d2 = ac.Dot( ap );
    if ( d1 <= 0.0 && d2 <= 0.0 )
        return a;
    const V3 bp = p - b;
    const GUCEF::Float64 d3 = ab.Dot( bp );
    const GUCEF::Float64 d4 = ac.Dot( bp );
    if ( d3 >= 0.0 && d4 <= d3 )
        return b;
    const GUCEF::Float64 vc = d1 * d4 - d3 * d2;
    if ( vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0 )
        return a + ab * ( d1 / ( d1 - d3 ) );
    const V3 cp = p - c;
    const GUCEF::Float64 d5 = ab.Dot( cp );
    const GUCEF::Float64 d6 = ac.Dot( cp );
    if ( d6 >= 0.0 && d5 <= d6 )
        return c;
    const GUCEF::Float64 vb = d5 * d2 - d1 * d6;
    if ( vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0 )
        return a + ac * ( d2 / ( d2 - d6 ) );
    const GUCEF::Float64 va = d3 * d6 - d5 * d4;
    if ( va <= 0.0 && ( d4 - d3 ) >= 0.0 && ( d5 - d6 ) >= 0.0 )
        return b + ( c - b ) * ( ( d4 - d3 ) / ( ( d4 - d3 ) + ( d5 - d6 ) ) );
    const GUCEF::Float64 denominator = 1.0 / ( va + vb + vc );
    return a + ab * ( vb * denominator ) + ac * ( vc * denominator );
}

/*-------------------------------------------------------------------------*/

/** The distance of two segments by a fine ternary search over the parameter of the first (convex in it), written for the test */
inline GUCEF::Float64
ReferenceSegmentDistance( const P3& a0, const P3& a1, const P3& b0, const P3& b1 )
{
    struct SPointSegment
    {
        static GUCEF::Float64 Distance( const P3& p, const P3& s0, const P3& s1 )
        {
            const V3 d = s1 - s0;
            const GUCEF::Float64 dd = d.Dot( d );
            GUCEF::Float64 t = dd > 0.0 ? ( p - s0 ).Dot( d ) / dd : 0.0;
            t = t < 0.0 ? 0.0 : ( t > 1.0 ? 1.0 : t );
            return ( p - ( s0 + d * t ) ).GetLength();
        }
    };
    GUCEF::Float64 lo = 0.0;
    GUCEF::Float64 hi = 1.0;
    for ( GUCEF::UInt32 i=0; i<200; ++i )
    {
        const GUCEF::Float64 m1 = lo + ( hi - lo ) / 3.0;
        const GUCEF::Float64 m2 = hi - ( hi - lo ) / 3.0;
        const GUCEF::Float64 f1 = SPointSegment::Distance( a0 + ( a1 - a0 ) * m1, b0, b1 );
        const GUCEF::Float64 f2 = SPointSegment::Distance( a0 + ( a1 - a0 ) * m2, b0, b1 );
        if ( f1 < f2 )
            hi = m2;
        else
            lo = m1;
    }
    return SPointSegment::Distance( a0 + ( a1 - a0 ) * ( ( lo + hi ) * 0.5 ), b0, b1 );
}

/*-------------------------------------------------------------------------*/

/** The distance of two separated convex polytopes: the minimum over vertex-triangle pairs (both ways) and edge-edge pairs */
inline GUCEF::Float64
PolytopeDistance( const SPolytope& a, const SPolytope& b )
{
    GUCEF::Float64 best = 1.0e300;
    for ( int pass=0; pass<2; ++pass )
    {
        const SPolytope& p = 0 == pass ? a : b;
        const SPolytope& q = 0 == pass ? b : a;
        for ( size_t v=0; v<p.vertices.size(); ++v )
            for ( size_t t=0; t<q.triangles.size(); t+=3 )
            {
                const P3 c = ReferenceClosestOnTriangle( p.vertices[ v ], q.vertices[ q.triangles[ t ] ], q.vertices[ q.triangles[ t + 1 ] ], q.vertices[ q.triangles[ t + 2 ] ] );
                const GUCEF::Float64 d = ( p.vertices[ v ] - c ).GetLength();
                best = d < best ? d : best;
            }
    }
    for ( size_t i=0; i<a.edges.size(); i+=2 )
        for ( size_t j=0; j<b.edges.size(); j+=2 )
        {
            const GUCEF::Float64 d = ReferenceSegmentDistance( a.vertices[ a.edges[ i ] ], a.vertices[ a.edges[ i + 1 ] ], b.vertices[ b.edges[ j ] ], b.vertices[ b.edges[ j + 1 ] ] );
            best = d < best ? d : best;
        }
    return best;
}

/*-------------------------------------------------------------------------*/

/** The largest separation of the projections of two convex polytopes over the triangle normals and edge pairs (> 0: apart) */
inline GUCEF::Float64
PolytopeSeparation( const SPolytope& a, const SPolytope& b )
{
    GUCEF::vector< V3 > axes;
    for ( int pass=0; pass<2; ++pass )
    {
        const SPolytope& p = 0 == pass ? a : b;
        for ( size_t t=0; t<p.triangles.size(); t+=3 )
            axes.push_back( ( p.vertices[ p.triangles[ t + 1 ] ] - p.vertices[ p.triangles[ t ] ] ).Cross( p.vertices[ p.triangles[ t + 2 ] ] - p.vertices[ p.triangles[ t ] ] ) );
    }
    for ( size_t i=0; i<a.edges.size(); i+=2 )
        for ( size_t j=0; j<b.edges.size(); j+=2 )
            axes.push_back( ( a.vertices[ a.edges[ i + 1 ] ] - a.vertices[ a.edges[ i ] ] ).Cross( b.vertices[ b.edges[ j + 1 ] ] - b.vertices[ b.edges[ j ] ] ) );
    GUCEF::Float64 best = -1.0e300;
    for ( size_t k=0; k<axes.size(); ++k )
    {
        const GUCEF::Float64 length = axes[ k ].GetLength();
        if ( length < 1.0e-9 )
            continue;
        const V3 axis = axes[ k ] * ( 1.0 / length );
        GUCEF::Float64 aLow = 1.0e300, aHigh = -1.0e300, bLow = 1.0e300, bHigh = -1.0e300;
        for ( size_t v=0; v<a.vertices.size(); ++v )
        {
            const GUCEF::Float64 d = axis.Dot( a.vertices[ v ].ToPositionVector() );
            aLow = d < aLow ? d : aLow;
            aHigh = d > aHigh ? d : aHigh;
        }
        for ( size_t v=0; v<b.vertices.size(); ++v )
        {
            const GUCEF::Float64 d = axis.Dot( b.vertices[ v ].ToPositionVector() );
            bLow = d < bLow ? d : bLow;
            bHigh = d > bHigh ? d : bHigh;
        }
        const GUCEF::Float64 separation = bLow - aHigh > aLow - bHigh ? bLow - aHigh : aLow - bHigh;
        best = separation > best ? separation : best;
    }
    return best;
}

/*-------------------------------------------------------------------------*/

/** 'count' random points in a ball of 'radius' around 'centre' (values drawn into locals) */
inline GUCEF::vector< P3 >
RandomPointCloud( QueryReference::CRandom& r, const P3& centre, GUCEF::Float64 radius, GUCEF::UInt32 count )
{
    GUCEF::vector< P3 > points;
    while ( points.size() < count )
    {
        const V3 offset = r.Vector( radius );
        if ( offset.GetLength() <= radius )
            points.push_back( centre + offset );
    }
    return points;
}

/*-------------------------------------------------------------------------*/

/** A hull of the kind T of the polytope's vertices, with its triangles as faces when 'withFaces' */
template < typename T >
GUCEF::GEOMETRY::CTConvexHull3D< T >
ToHull( const SPolytope& polytope, bool withFaces )
{
    GUCEF::vector< GUCEF::MATH::CTPoint3D< T > > points;
    for ( size_t i=0; i<polytope.vertices.size(); ++i )
        points.push_back( QueryReference::ToP< T >( polytope.vertices[ i ] ) );
    GUCEF::GEOMETRY::CTConvexHull3D< T > hull( &points[ 0 ], (GUCEF::UInt32) points.size() );
    if ( withFaces && !polytope.triangles.empty() )
    {
        GUCEF::vector< GUCEF::UInt32 > sizes( polytope.triangles.size() / 3, 3 );
        hull.SetFaces( &polytope.triangles[ 0 ], &sizes[ 0 ], (GUCEF::UInt32) sizes.size() );
    }
    return hull;
}

/*-------------------------------------------------------------------------*/

/**
 *  The size of a seeded floating-point sweep: the full count in optimised builds, a tenth (at least 100) in debug and
 *  sanitizer builds, which run the same checks on fewer cases. The pinned fixed-point runs never scale.
 */
inline GUCEF::UInt32
ScaledCount( GUCEF::UInt32 count )
{
    #ifdef GUCEF_GEOMETRY_DEBUG_MODE
    return count / 10 > 100 ? count / 10 : ( count < 100 ? count : 100 );
    #else
    return count;
    #endif
}

/*-------------------------------------------------------------------------*/

/** FNV-1a over 64 bit raw values: the pinned hashes of the fixed-point runs */
class CRawHash
{
    public:

    CRawHash( void ) : m_hash( 14695981039346656037ULL ), m_count( 0 ) {}

    void AddRaw( GUCEF::Int64 raw )
    {
        GUCEF::UInt64 bits = (GUCEF::UInt64) raw;
        for ( GUCEF::UInt32 i=0; i<8; ++i )
        {
            m_hash ^= bits & 0xFF;
            m_hash *= 1099511628211ULL;
            bits >>= 8;
        }
        ++m_count;
    }

    void Add( const GUCEF::MATH::TFixed32_32& value ) { AddRaw( value.GetRaw() ); }
    void Add( GUCEF::UInt32 value ) { AddRaw( (GUCEF::Int64) value ); }
    void Add( bool value ) { AddRaw( value ? 1 : 0 ); }
    template < typename V > void Add3( const V& v ) { Add( v.x ); Add( v.y ); Add( v.z ); }

    GUCEF::UInt64 GetHash( void ) const { return m_hash; }
    GUCEF::UInt32 GetCount( void ) const { return m_count; }

    private:

    GUCEF::UInt64 m_hash;
    GUCEF::UInt32 m_count;
};

/*-------------------------------------------------------------------------*/

/** Seeded fixed-point values from integer arithmetic only, so that the inputs of the pinned runs need no floating point */
class CFixedRandom
{
    public:

    typedef GUCEF::MATH::TFixed32_32 TFixed;
    typedef GUCEF::MATH::CTPoint3D< TFixed > TPoint;
    typedef GUCEF::MATH::CTVector3D< TFixed > TVector;

    explicit CFixedRandom( GUCEF::UInt64 seed ) : m_random( seed ) {}

    /** Uniform raw value in [ low, high ] given in 1/65536 units of the value (so that the range is exact) */
    TFixed Uniform( GUCEF::Int32 low65536, GUCEF::Int32 high65536 )
    {
        const GUCEF::UInt64 span = (GUCEF::UInt64) ( (GUCEF::Int64) high65536 - (GUCEF::Int64) low65536 ) << 16;
        const GUCEF::UInt64 draw = m_random.NextUInt64() % ( span + 1 );
        const GUCEF::Int64 raw = ( (GUCEF::Int64) low65536 * 65536 ) + (GUCEF::Int64) draw;
        return TFixed::FromRaw( raw );
    }

    TPoint Point( GUCEF::Int32 range65536 )
    {
        const TFixed x = Uniform( -range65536, range65536 );
        const TFixed y = Uniform( -range65536, range65536 );
        const TFixed z = Uniform( -range65536, range65536 );
        return TPoint( x, y, z );
    }

    TVector Vector( GUCEF::Int32 range65536 )
    {
        const TFixed x = Uniform( -range65536, range65536 );
        const TFixed y = Uniform( -range65536, range65536 );
        const TFixed z = Uniform( -range65536, range65536 );
        return TVector( x, y, z );
    }

    /** A unit quaternion from four integer draws, normalised in fixed point */
    GUCEF::MATH::CTQuaternion< TFixed > Rotation( void )
    {
        for ( ;; )
        {
            const TFixed x = Uniform( -65536, 65536 );
            const TFixed y = Uniform( -65536, 65536 );
            const TFixed z = Uniform( -65536, 65536 );
            const TFixed w = Uniform( -65536, 65536 );
            const GUCEF::MATH::CTQuaternion< TFixed > q = GUCEF::MATH::CTQuaternion< TFixed >::FromXYZW( x, y, z, w );
            if ( TFixed( 1 ) / TFixed( 4 ) < q.Dot( q ) )
                return q.Normalized();
        }
    }

    GUCEF::UInt32 Index( GUCEF::UInt32 count ) { return (GUCEF::UInt32) ( m_random.NextUInt64() % count ); }

    private:

    GUCEF::MATH::CSplitMix64 m_random;
};

/*-------------------------------------------------------------------------*/

/** The value of a fixed-point number in Float64 from its raw value (exact for |value| < 2^21) */
inline GUCEF::Float64
FixedToFloat64( const GUCEF::MATH::TFixed32_32& value )
{
    return ldexp( (GUCEF::Float64) value.GetRaw(), -32 );
}

/** A fixed-point point as Float64 */
inline P3
FixedToP3( const GUCEF::MATH::CTPoint3D< GUCEF::MATH::TFixed32_32 >& p )
{
    return P3( FixedToFloat64( p.x ), FixedToFloat64( p.y ), FixedToFloat64( p.z ) );
}

}  /* namespace ConvexReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTCONVEXREFERENCE_H ? */
