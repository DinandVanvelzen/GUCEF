/*
 *  gucefGEOMETRY: GUCEF module providing geometry: shapes, spaces and spatial queries
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

#ifndef GUCEF_GEOMETRY_CTGJK_H
#define GUCEF_GEOMETRY_CTGJK_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#include "gucefGEOMETRY_CTGeometryTolerance.h"
#define GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H
#endif /* GUCEF_GEOMETRY_CTGEOMETRYTOLERANCE_H ? */

#ifndef GUCEF_GEOMETRY_POINTQUERIES_H
#include "gucefGEOMETRY_PointQueries.h"
#define GUCEF_GEOMETRY_POINTQUERIES_H
#endif /* GUCEF_GEOMETRY_POINTQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SUPPORTMAPPINGS_H
#include "gucefGEOMETRY_SupportMappings.h"
#define GUCEF_GEOMETRY_SUPPORTMAPPINGS_H
#endif /* GUCEF_GEOMETRY_SUPPORTMAPPINGS_H ? */

#ifndef GUCEF_MATH_MATHSTATUS_H
#include "gucefMATH_MathStatus.h"
#define GUCEF_MATH_MATHSTATUS_H
#endif /* GUCEF_MATH_MATHSTATUS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A simplex of GJK: up to four vertices w = a - b of the Minkowski difference (as vectors from the origin), the
 *  support points a on A and b on B that formed them, the search direction that found them, and the barycentric
 *  weights of the point of the simplex nearest to the origin. The vertices are ordered oldest first.
 */
template < typename T >
struct SGjkSimplex
{
    UInt32 count;
    MATH::CTVector3D< T > w[ 4 ];
    MATH::CTPoint3D< T > a[ 4 ];
    MATH::CTPoint3D< T > b[ 4 ];
    MATH::CTVector3D< T > direction[ 4 ];
    T lambda[ 4 ];

    SGjkSimplex( void ) : count( 0 )
    {
        for ( UInt32 i=0; i<4; ++i )
            lambda[ i ] = MATH::CTNumericTraits< T >::Zero();
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The warm start of GJK for coherent queries (the same pair queried again after a small motion): the search
 *  directions of the final simplex of the previous query. The next query evaluates the supports along them first, so
 *  it starts near the answer. A default (empty) cache is a cold start. One cache per pair; not shared between threads.
 */
template < typename T >
struct SGjkCache
{
    UInt32 count;
    MATH::CTVector3D< T > directions[ 4 ];

    SGjkCache( void ) : count( 0 ) {}

    /** Forgets the previous query */
    void Reset( void ) { count = 0; }
};

/*-------------------------------------------------------------------------*/

/**
 *  The result of a GJK distance query between two support mappings A and B (cores plus margins).
 *
 *  - status: MATH::MATHSTATUS_OK, or MATH::MATHSTATUS_NOT_CONVERGED when the iteration cap was reached (the values are then the
 *    best estimate, an upper bound of the distance).
 *  - overlapping: the whole shapes touch or overlap (distance within the length tolerance of the query);
 *    coresOverlapping: the cores touch or overlap (the final simplex then encloses or touches the origin: EPA input).
 *  - distance: between the whole shapes, 0 when overlapping; coreDistance between the cores (0 when they overlap).
 *    distanceLowerBound: a proven lower bound of the distance of the whole shapes (the best separating plane found);
 *    equal to the distance within the tolerance when converged, the value to rely on after NOT_CONVERGED.
 *  - pointOnA, pointOnB: the nearest points of the whole shapes; when they overlap both are the same point, which
 *    lies in both shapes.
 *  - normal: unit vector from A towards B along the shortest connection of the cores; the zero vector when the cores
 *    overlap (EPA then gives the direction).
 *  - corePointOnA, corePointOnB: the nearest points of the cores.
 *  - simplex: the final simplex (input of EPA); iterations: support evaluations of this query.
 */
template < typename T >
struct SGjkResult
{
    MATH::TMathStatus status;
    bool overlapping;
    bool coresOverlapping;
    T distance;
    T coreDistance;
    T distanceLowerBound;
    MATH::CTPoint3D< T > pointOnA;
    MATH::CTPoint3D< T > pointOnB;
    MATH::CTPoint3D< T > corePointOnA;
    MATH::CTPoint3D< T > corePointOnB;
    MATH::CTVector3D< T > normal;
    UInt32 iterations;
    SGjkSimplex< T > simplex;

    SGjkResult( void )
        : status( MATH::MATHSTATUS_OK ), overlapping( false ), coresOverlapping( false ), distance( MATH::CTNumericTraits< T >::Zero() ), coreDistance( MATH::CTNumericTraits< T >::Zero() )
        , distanceLowerBound( MATH::CTNumericTraits< T >::Zero() ), pointOnA(), pointOnB(), corePointOnA(), corePointOnB(), normal(), iterations( 0 ), simplex()
    {
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  The result of a GJK shape cast (a convex shape A moved along a translation against a convex shape B).
 *
 *  - hit: the shapes meet for a fraction in [0, maxFraction] of the translation; fraction is the first such fraction
 *    (0 when they overlap at the start).
 *  - normal: unit surface normal of B at the contact, facing A (against the motion); the zero vector for a hit at 0
 *    with overlapping shapes.
 *  - pointOnA, pointOnB: the contact points of A (at its position at the hit) and of B.
 */
template < typename T >
struct SGjkCastResult
{
    MATH::TMathStatus status;
    bool hit;
    T fraction;
    MATH::CTVector3D< T > normal;
    MATH::CTPoint3D< T > pointOnA;
    MATH::CTPoint3D< T > pointOnB;
    UInt32 iterations;

    SGjkCastResult( void )
        : status( MATH::MATHSTATUS_OK ), hit( false ), fraction( MATH::CTNumericTraits< T >::Zero() ), normal(), pointOnA(), pointOnB(), iterations( 0 )
    {
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The Gilbert-Johnson-Keerthi algorithm on support mappings (section 7.6.2 of the design): distance, nearest points
 *  and overlap of two convex shapes (the final simplex is the input of EPA when the cores overlap), and the GJK ray
 *  cast (van den Bergen) for shape casts.
 *
 *  - Sub-algorithm: signed volumes (Montanari, Petrinic, Barbieri 2017): the nearest point of a simplex is found from
 *    the signs of sub-determinants instead of Johnson's recursive determinants, which stays correct on nearly
 *    degenerate simplices. Every determinant is evaluated on the simplex divided by its largest coordinate, so no
 *    product overflows or underflows (floats) and fixed point keeps its resolution for small simplices.
 *  - Termination: when the lower bound (unit search direction . support) is within max( absolute, relative * |v| ) of
 *    the upper bound |v| (CTGeometryTolerance), when a support repeats, when |v| stops decreasing, when the simplex
 *    encloses the origin, or at the iteration cap MaxIterations = 64 (MATH::MATHSTATUS_NOT_CONVERGED, never exceeded).
 *    Fixed point: a repeated support or a stalled |v| within 4096 units of the coordinate scale of the origin proves
 *    no separation: the shapes count as overlapping unless the best lower bound exceeds the length tolerance (a
 *    deep overlap whose final triangle passes that close to the origin would otherwise come out as a separation
 *    of about the resolution).
 *  - Margins: the distance runs on the cores and subtracts the margins (section 7.6.1).
 *  - Kind: floats and fixed point (Float32, Float64 and TFixed32_32 are exported). The fixed-point form uses no
 *    floating point and is bit-exact on every platform. The determinants are formed on the simplex scaled to its
 *    largest coordinate and the search directions are scaled to a largest component in [1/2, 1)
 *    (CTSearchDirectionScaler), so the only limit is the Q32.32 range of the coordinates, their differences and their
 *    dot products with such a direction: coordinates within about +-7e8 of the origin of the query frame. Precision
 *    is absolute (2^-32), so a frame near the shapes keeps the most of it (gucefPHYSICS queries relative to collider
 *    A).
 *  - Order: every result depends only on the inputs (no addresses, no hashing): ties keep the first candidate.
 *  - Realtime: every member is realtime-safe and allocation-free.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGjk
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTGeometryTolerance< T > TTolerance;
    typedef SGjkSimplex< T > TSimplex;

    enum { MaxIterations = 64 };

    /**
     *  Distance and nearest points of the support mappings A and B. 'cache' (may be GUCEF_NULL) warm starts the query
     *  and receives the final simplex. Returns result.status.
     */
    template < typename TA, typename TB >
    static MATH::TMathStatus ComputeDistance( const TA& a, const TB& b, const TTolerance& tolerance, SGjkCache< T >* cache, SGjkResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the whole shapes (cores plus margins) touch or overlap */
    template < typename TA, typename TB >
    static bool Intersects( const TA& a, const TB& b, const TTolerance& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The GJK ray cast (van den Bergen 2004) of A moved by fraction * translation against B, for fractions in
     *  [0, maxFraction]: the first fraction at which the whole shapes meet. Returns result.status. It stops on a hit
     *  when |v| is within the length tolerance, when the simplex encloses the origin, or when an iteration that did
     *  not advance makes no progress (rounding); an iteration that advanced is never a stop, also for shapes without
     *  margin (boxes, hulls, triangles), whose hits are exact up to the tolerance.
     */
    template < typename TA, typename TB >
    static MATH::TMathStatus Cast( const TA& a, const TVector& translation, const T& maxFraction, const TB& b, const TTolerance& tolerance, SGjkCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  The signed-volumes sub-algorithm: reduces the simplex to the smallest sub-simplex that holds the point nearest
     *  to the origin, sets the barycentric weights of that point and returns it in 'v' (the zero vector when a
     *  tetrahedron encloses the origin). An empty simplex gives the zero vector.
     */
    static void SolveSimplex( TSimplex& simplex, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The points of A and B for the barycentric weights of the simplex */
    static void GetClosestPoints( const TSimplex& simplex, TPoint& onA, TPoint& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** A monotone measure of the length of v for comparisons: the squared length for floats, the length for fixed point */
    static T GetMagnitude( const TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    private:

    /** The supports of the cores (GetSupport) */
    class CCorePolicy
    {
        public:
        template < typename TS >
        static GUCEF_GEOMETRY_FORCE_INLINE TPoint Support( const TS& s, const TVector& d ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return s.GetSupport( d ); }
    };

    /** The GJK iteration with the supports of TPolicy; 'overlapping' when the simplex encloses or touches the origin */
    template < typename TPolicy, typename TA, typename TB >
    static MATH::TMathStatus Run( const TA& a, const TB& b, const TTolerance& tolerance, SGjkCache< T >* cache, TSimplex& simplex, TVector& v, bool& overlapping, UInt32& iterations, T& lowerBound ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    static void SetSingle( const TSimplex& in, UInt32 i, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    static void SolveSegment( const TSimplex& in, UInt32 i, UInt32 j, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    static void SolveTriangle( const TSimplex& in, UInt32 i, UInt32 j, UInt32 k, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    static void SolveTetrahedron( const TSimplex& in, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    static void CopyVertex( const TSimplex& in, UInt32 i, TSimplex& out, UInt32 o ) GUCEF_GEOMETRY_REALTIME_CONTEXT;
    static bool IsPositive( const T& x ) GUCEF_GEOMETRY_REALTIME_CONTEXT { return TTraits::Zero() < x; }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTGjk< T >::GetMagnitude( const TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( TTraits::IsFloatingPoint )
        return v.GetSquaredLength();
    return v.GetLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::CopyVertex( const TSimplex& in, UInt32 i, TSimplex& out, UInt32 o ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    out.w[ o ] = in.w[ i ];
    out.a[ o ] = in.a[ i ];
    out.b[ o ] = in.b[ i ];
    out.direction[ o ] = in.direction[ i ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::SetSingle( const TSimplex& in, UInt32 i, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    CopyVertex( in, i, out, 0 );
    out.lambda[ 0 ] = TTraits::One();
    out.count = 1;
    v = in.w[ i ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::SolveSegment( const TSimplex& in, UInt32 i, UInt32 j, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector p = in.w[ i ];
    const TVector q = in.w[ j ];
    const TVector t = q - p;
    TVector unit;
    if ( !t.ScaleToUnitChebyshev( unit ) )
    {
        /* both ends coincide: the newer one */
        SetSingle( in, j, out, v );
        return;
    }
    /* weights of the projection of the origin: q with -p.t, p with q.t (t scaled to its largest component) */
    const T cq = -p.Dot( unit );
    const T cp = q.Dot( unit );
    if ( !IsPositive( cq ) )
    {
        SetSingle( in, i, out, v );
        return;
    }
    if ( !IsPositive( cp ) )
    {
        SetSingle( in, j, out, v );
        return;
    }
    const T lq = cq / ( cp + cq );
    CopyVertex( in, i, out, 0 );
    CopyVertex( in, j, out, 1 );
    out.lambda[ 0 ] = TTraits::One() - lq;
    out.lambda[ 1 ] = lq;
    out.count = 2;
    v = p + t * lq;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::SolveTriangle( const TSimplex& in, UInt32 i, UInt32 j, UInt32 k, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector p = in.w[ i ];
    const TVector e1 = in.w[ j ] - p;
    const TVector e2 = in.w[ k ] - p;
    const T scale = TTraits::Max( TTraits::Max( e1.GetChebyshevLength(), e2.GetChebyshevLength() ), p.GetChebyshevLength() );

    T cj = TTraits::Zero();
    T ck = TTraits::Zero();
    T mu = TTraits::Zero();
    bool planar = false;
    if ( IsPositive( scale ) )
    {
        const TVector sp = p / scale;
        const TVector s1 = e1 / scale;
        const TVector s2 = e2 / scale;
        const TVector n = s1.Cross( s2 );
        TVector nUnit;
        if ( n.ScaleToUnitChebyshev( nUnit ) )
        {
            planar = true;
            mu = nUnit.Dot( n );
            cj = nUnit.Dot( ( -sp ).Cross( s2 ) );
            ck = nUnit.Dot( s1.Cross( -sp ) );
        }
    }

    if ( planar && IsPositive( mu ) )
    {
        const T ci = mu - cj - ck;
        if ( IsPositive( ci ) && IsPositive( cj ) && IsPositive( ck ) )
        {
            const T lj = cj / mu;
            const T lk = ck / mu;
            CopyVertex( in, i, out, 0 );
            CopyVertex( in, j, out, 1 );
            CopyVertex( in, k, out, 2 );
            out.lambda[ 0 ] = TTraits::One() - lj - lk;
            out.lambda[ 1 ] = lj;
            out.lambda[ 2 ] = lk;
            out.count = 3;
            v = p + e1 * lj + e2 * lk;
            return;
        }
        /* the nearest point lies on an edge opposite a vertex with a non-positive weight */
        const bool candidate[ 3 ] = { !IsPositive( ci ), !IsPositive( cj ), !IsPositive( ck ) };
        const UInt32 edges[ 3 ][ 2 ] = { { j, k }, { i, k }, { i, j } };
        bool found = false;
        T best = TTraits::Zero();
        for ( UInt32 e=0; e<3; ++e )
        {
            if ( !candidate[ e ] )
                continue;
            TSimplex sub;
            TVector subV;
            SolveSegment( in, edges[ e ][ 0 ], edges[ e ][ 1 ], sub, subV );
            const T m = GetMagnitude( subV );
            if ( !found || m < best )
            {
                found = true;
                best = m;
                out = sub;
                v = subV;
            }
        }
        return;
    }

    /* degenerate (collinear or coincident) triangle: the best of the three edges */
    const UInt32 edges[ 3 ][ 2 ] = { { i, j }, { j, k }, { i, k } };
    T best = TTraits::Zero();
    for ( UInt32 e=0; e<3; ++e )
    {
        TSimplex sub;
        TVector subV;
        SolveSegment( in, edges[ e ][ 0 ], edges[ e ][ 1 ], sub, subV );
        const T m = GetMagnitude( subV );
        if ( 0 == e || m < best )
        {
            best = m;
            out = sub;
            v = subV;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::SolveTetrahedron( const TSimplex& in, TSimplex& out, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector p = in.w[ 0 ];
    const TVector e1 = in.w[ 1 ] - p;
    const TVector e2 = in.w[ 2 ] - p;
    const TVector e3 = in.w[ 3 ] - p;
    const T scale = TTraits::Max( TTraits::Max( e1.GetChebyshevLength(), e2.GetChebyshevLength() ), TTraits::Max( e3.GetChebyshevLength(), p.GetChebyshevLength() ) );

    T c[ 4 ] = { TTraits::Zero(), TTraits::Zero(), TTraits::Zero(), TTraits::Zero() };
    T mu = TTraits::Zero();
    if ( IsPositive( scale ) )
    {
        const TVector sp = -( p / scale );
        const TVector s1 = e1 / scale;
        const TVector s2 = e2 / scale;
        const TVector s3 = e3 / scale;
        const TVector n23 = s2.Cross( s3 );
        mu = s1.Dot( n23 );
        c[ 1 ] = sp.Dot( n23 );
        c[ 2 ] = s1.Dot( sp.Cross( s3 ) );
        c[ 3 ] = s1.Dot( s2.Cross( sp ) );
        c[ 0 ] = mu - c[ 1 ] - c[ 2 ] - c[ 3 ];
    }

    const bool positive = IsPositive( mu );
    const bool negative = TTraits::IsNegative( mu );
    bool sameSign[ 4 ];
    bool inside = positive || negative;
    for ( UInt32 i=0; i<4; ++i )
    {
        sameSign[ i ] = positive ? IsPositive( c[ i ] ) : ( negative && TTraits::IsNegative( c[ i ] ) );
        inside = inside && sameSign[ i ];
    }

    if ( inside )
    {
        out = in;
        out.count = 4;
        T sum = TTraits::Zero();
        for ( UInt32 i=1; i<4; ++i )
        {
            out.lambda[ i ] = c[ i ] / mu;
            sum = sum + out.lambda[ i ];
        }
        out.lambda[ 0 ] = TTraits::One() - sum;
        v = TVector();
        return;
    }

    /* the nearest point lies on a face opposite a vertex whose weight has the wrong sign (every face when flat) */
    const UInt32 faces[ 4 ][ 3 ] = { { 1, 2, 3 }, { 0, 2, 3 }, { 0, 1, 3 }, { 0, 1, 2 } };
    bool found = false;
    T best = TTraits::Zero();
    for ( UInt32 f=0; f<4; ++f )
    {
        if ( sameSign[ f ] )
            continue;
        TSimplex sub;
        TVector subV;
        SolveTriangle( in, faces[ f ][ 0 ], faces[ f ][ 1 ], faces[ f ][ 2 ], sub, subV );
        const T m = GetMagnitude( subV );
        if ( !found || m < best )
        {
            found = true;
            best = m;
            out = sub;
            v = subV;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::SolveSimplex( TSimplex& simplex, TVector& v ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TSimplex out;
    switch ( simplex.count )
    {
        case 1:
            SetSingle( simplex, 0, out, v );
            break;
        case 2:
            SolveSegment( simplex, 0, 1, out, v );
            break;
        case 3:
            SolveTriangle( simplex, 0, 1, 2, out, v );
            break;
        case 4:
            SolveTetrahedron( simplex, out, v );
            break;
        default:
            simplex.count = 0;
            v = TVector();
            return;
    }
    simplex = out;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGjk< T >::GetClosestPoints( const TSimplex& simplex, TPoint& onA, TPoint& onB ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( 0 == simplex.count )
    {
        onA = TPoint();
        onB = TPoint();
        return;
    }
    /* relative to the first vertex, so that coordinates far from the origin cost no precision */
    TVector da;
    TVector db;
    for ( UInt32 i=1; i<simplex.count; ++i )
    {
        da = da + ( simplex.a[ i ] - simplex.a[ 0 ] ) * simplex.lambda[ i ];
        db = db + ( simplex.b[ i ] - simplex.b[ 0 ] ) * simplex.lambda[ i ];
    }
    onA = simplex.a[ 0 ] + da;
    onB = simplex.b[ 0 ] + db;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TPolicy, typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
CTGjk< T >::Run( const TA& a, const TB& b, const TTolerance& tolerance, SGjkCache< T >* cache, TSimplex& simplex, TVector& v, bool& overlapping, UInt32& iterations, T& lowerBound ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    simplex = TSimplex();
    overlapping = false;
    iterations = 0;
    lowerBound = TTraits::Zero();

    /* warm start: the supports along the cached directions */
    if ( GUCEF_NULL != cache && cache->count > 0 && cache->count <= 4 )
    {
        for ( UInt32 i=0; i<cache->count; ++i )
        {
            const TVector d = CTSearchDirectionScaler< T >::Scale( cache->directions[ i ] );
            const TPoint pa = TPolicy::Support( a, d );
            const TPoint pb = TPolicy::Support( b, -d );
            const TVector w = pa - pb;
            bool duplicate = false;
            for ( UInt32 k=0; k<simplex.count; ++k )
                duplicate = duplicate || simplex.w[ k ] == w;
            if ( duplicate )
                continue;
            const UInt32 n = simplex.count;
            simplex.w[ n ] = w;
            simplex.a[ n ] = pa;
            simplex.b[ n ] = pb;
            simplex.direction[ n ] = d;
            simplex.count = n + 1;
        }
        ++iterations;
        SolveSimplex( simplex, v );
    }
    if ( 0 == simplex.count )
    {
        v = a.GetAnyPoint() - b.GetAnyPoint();
        if ( v.IsZero() )
            v = TVector::UnitX();
    }

    MATH::TMathStatus status = MATH::MATHSTATUS_OK;
    bool converged = false;
    T length = v.GetLength();
    while ( !converged )
    {
        /*
         *  the tolerance grows with the coordinates of the nearest points, whose rounding it has to absorb (not with
         *  the support points: a large face supports with far corners that say nothing about the precision here)
         */
        T scale = TTraits::Zero();
        if ( simplex.count > 0 )
        {
            TPoint nearA;
            TPoint nearB;
            GetClosestPoints( simplex, nearA, nearB );
            scale = TTraits::Max( GetCoordinateScale( nearA ), GetCoordinateScale( nearB ) );
        }
        /*
         *  floats: a vector within the rounding noise of the simplex (64 ulps of its largest coordinate) is the origin,
         *  also for a tolerance of 0 (coplanar shapes give a triangle around the origin whose v is noise in its plane)
         */
        T noiseScale = scale;
        for ( UInt32 k=0; k<simplex.count; ++k )
            noiseScale = TTraits::Max( noiseScale, simplex.w[ k ].GetChebyshevLength() );
        const T noise = TTraits::IsFloatingPoint ? TTraits::Epsilon() * TTraits::FromInt32( 64 ) * noiseScale : TTraits::Zero();
        const T lengthTolerance = TTraits::Max( tolerance.GetLengthTolerance( scale ), noise );
        /* fixed point: a simplex within 4096 units of the coordinates of the origin is not resolved (see the stops below) */
        const T unresolved = TTraits::IsFloatingPoint ? TTraits::Zero() : TTraits::Epsilon() * TTraits::FromInt32( 4096 ) * TTraits::Max( TTraits::One(), noiseScale );
        if ( 4 == simplex.count || ( simplex.count > 0 && !( lengthTolerance < length ) ) )
        {
            overlapping = true;
            break;
        }
        if ( iterations >= (UInt32) MaxIterations )
        {
            status = MATH::MATHSTATUS_NOT_CONVERGED;
            break;
        }
        ++iterations;

        const TVector d = CTSearchDirectionScaler< T >::Scale( -v );
        const TPoint pa = TPolicy::Support( a, d );
        const TPoint pb = TPolicy::Support( b, -d );
        const TVector w = pa - pb;

        if ( simplex.count > 0 )
        {
            /* lower bound of the distance along the unit search direction against the upper bound |v| */
            const TVector unit = v.Normalized();
            const T lower = unit.Dot( w );
            if ( lowerBound < lower )
                lowerBound = lower;
            const T gap = length - lower;
            const T allowed = TTraits::Max( tolerance.absolute, tolerance.relative * length );
            if ( !( allowed < gap ) )
                break;
            bool duplicate = false;
            for ( UInt32 k=0; k<simplex.count; ++k )
                duplicate = duplicate || simplex.w[ k ] == w;
            if ( duplicate )
            {
                /* no new support: see the no-progress case below */
                overlapping = !( unresolved < length ) && !( lengthTolerance < lowerBound );
                break;
            }
        }

        const TSimplex previous = simplex;
        const TVector previousV = v;
        const UInt32 n = simplex.count;
        simplex.w[ n ] = w;
        simplex.a[ n ] = pa;
        simplex.b[ n ] = pb;
        simplex.direction[ n ] = d;
        simplex.count = n + 1;
        SolveSimplex( simplex, v );

        const T newLength = v.GetLength();
        if ( n > 0 && 4 != simplex.count && !( newLength < length ) )
        {
            /*
             *  no progress (rounding): keep the previous simplex, it is the better answer. In fixed point a
             *  tetrahedron within the resolution of the origin is not resolved, so a stop with |v| within that
             *  resolution proves no separation: without a lower bound beyond the tolerance it is an overlap (a deep
             *  overlap would otherwise come out as a separation of about the resolution).
             */
            simplex = previous;
            v = previousV;
            overlapping = !( unresolved < length ) && !( lengthTolerance < lowerBound );
            break;
        }
        length = newLength;
    }

    if ( GUCEF_NULL != cache )
    {
        cache->count = simplex.count;
        for ( UInt32 i=0; i<simplex.count; ++i )
            cache->directions[ i ] = simplex.direction[ i ];
    }
    return status;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
CTGjk< T >::ComputeDistance( const TA& a, const TB& b, const TTolerance& tolerance, SGjkCache< T >* cache, SGjkResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TVector v;
    bool coresOverlap = false;
    T coreLowerBound = TTraits::Zero();
    result.status = Run< CCorePolicy >( a, b, tolerance, cache, result.simplex, v, coresOverlap, result.iterations, coreLowerBound );
    GetClosestPoints( result.simplex, result.corePointOnA, result.corePointOnB );

    const T marginA = a.GetMargin();
    const T marginB = b.GetMargin();
    const T margin = marginA + marginB;
    const T scale = TTraits::Max( GetCoordinateScale( result.corePointOnA ), GetCoordinateScale( result.corePointOnB ) );
    const T lengthTolerance = tolerance.GetLengthTolerance( scale );

    result.coresOverlapping = coresOverlap;
    if ( coresOverlap )
    {
        result.coreDistance = TTraits::Zero();
        result.normal = TVector();
        result.distance = TTraits::Zero();
        result.overlapping = true;
        result.pointOnA = result.corePointOnA;
        result.pointOnB = result.corePointOnA;
        return result.status;
    }

    const T coreDistance = v.GetLength();
    result.coreDistance = coreDistance;
    result.distanceLowerBound = margin < coreLowerBound ? coreLowerBound - margin : TTraits::Zero();
    if ( result.coreDistance < coreLowerBound )
        result.distanceLowerBound = margin < coreDistance ? coreDistance - margin : TTraits::Zero();
    TVector n;
    if ( !( -v ).TryNormalize( n, TTraits::Zero() ) )
        n = TVector();
    result.normal = n;

    if ( margin < coreDistance )
    {
        const T distance = coreDistance - margin;
        result.distance = distance;
        result.pointOnA = result.corePointOnA + n * marginA;
        result.pointOnB = result.corePointOnB - n * marginB;
        result.overlapping = !( lengthTolerance < distance );
        if ( result.overlapping )
        {
            result.distance = TTraits::Zero();
            result.pointOnB = result.pointOnA;
        }
        return result.status;
    }

    /* the margins overlap: a common point on the connection of the cores */
    result.distance = TTraits::Zero();
    result.overlapping = true;
    const T along = IsPositive( margin ) ? coreDistance * marginA / margin : TTraits::Zero();
    result.pointOnA = result.corePointOnA + n * along;
    result.pointOnB = result.pointOnA;
    return result.status;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGjk< T >::Intersects( const TA& a, const TB& b, const TTolerance& tolerance ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    SGjkResult< T > result;
    ComputeDistance( a, b, tolerance, GUCEF_NULL, result );
    return result.overlapping;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TA, typename TB >
GUCEF_GEOMETRY_PRECISE_INLINE MATH::TMathStatus
CTGjk< T >::Cast( const TA& a, const TVector& translation, const T& maxFraction, const TB& b, const TTolerance& tolerance, SGjkCastResult< T >& result ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    /*
     *  A moved by lambda * r meets B exactly when lambda * r lies in C = B - A: a ray from the origin along r against C
     *  (van den Bergen). The simplex holds the points p = b - a of C; its vertices for the sub-algorithm are x - p.
     */
    result = SGjkCastResult< T >();
    const TVector r = translation;
    const T rLength = r.GetLength();
    T lambda = TTraits::Zero();
    TVector x;
    TVector normal;
    TSimplex simplex;
    TVector v = x - ( b.GetAnyPoint() - a.GetAnyPoint() );
    MATH::TMathStatus status = MATH::MATHSTATUS_OK;
    UInt32 iterations = 0;

    for ( ;; )
    {
        T scale = TTraits::Zero();
        if ( simplex.count > 0 )
        {
            TPoint nearA;
            TPoint nearB;
            GetClosestPoints( simplex, nearA, nearB );
            scale = TTraits::Max( GetCoordinateScale( nearA ), GetCoordinateScale( nearB ) );
        }
        const T lengthTolerance = tolerance.GetLengthTolerance( scale );
        const T vLength = v.GetLength();
        if ( 4 == simplex.count || !( lengthTolerance < vLength ) )
            break;
        if ( iterations >= (UInt32) MaxIterations )
        {
            status = MATH::MATHSTATUS_NOT_CONVERGED;
            break;
        }
        ++iterations;

        const TVector searchDirection = CTSearchDirectionScaler< T >::Scale( v );
        const TPoint pb = GetSupportWithMargin( b, searchDirection );
        const TPoint pa = GetSupportWithMargin( a, -searchDirection );
        const TVector p = pb - pa;
        const TVector w = x - p;
        /*
         *  The products with v are formed with v scaled to its largest component, so they keep their precision when v
         *  is tiny near the contact. Advance only when x is apart from C by more than the length tolerance along v
         *  (the scaled product over the scaled length is that distance, bounded by |v|), so that rounding near the
         *  contact cannot push x through C when v is nearly perpendicular to r.
         */
        TVector u;
        if ( !v.ScaleToUnitChebyshev( u ) )
            break;
        const T uLength = u.GetLength();
        T uw = u.Dot( w );
        if ( vLength * uLength < uw )
            uw = vLength * uLength;
        bool advanced = false;
        if ( lengthTolerance * uLength < uw )
        {
            const T ur = u.Dot( r );
            if ( !TTraits::IsNegative( ur ) )
            {
                result.status = MATH::MATHSTATUS_OK;
                result.hit = false;
                result.iterations = iterations;
                return result.status;
            }
            const T step = MATH::ComponentNegate( uw / ur );
            lambda = lambda + step;
            /* a step within the length tolerance counts as no advance for the no-progress stop below */
            advanced = lengthTolerance < step * rLength;
            if ( maxFraction < lambda )
            {
                result.status = MATH::MATHSTATUS_OK;
                result.hit = false;
                result.iterations = iterations;
                return result.status;
            }
            x = r * lambda;
            normal = v;
        }

        /* rebuild the vertices x - p for the current x and add the new support */
        const UInt32 previousCount = simplex.count;
        bool duplicate = false;
        for ( UInt32 k=0; k<simplex.count; ++k )
            duplicate = duplicate || ( simplex.b[ k ] == pb && simplex.a[ k ] == pa );
        if ( !duplicate )
        {
            const UInt32 n = simplex.count;
            simplex.a[ n ] = pa;
            simplex.b[ n ] = pb;
            simplex.direction[ n ] = v;
            simplex.count = n + 1;
        }
        for ( UInt32 k=0; k<simplex.count; ++k )
            simplex.w[ k ] = x - ( simplex.b[ k ] - simplex.a[ k ] );
        const TVector previousV = v;
        SolveSimplex( simplex, v );
        /*
         *  No progress ends the search (rounding: the next support would be the same), but only when x stayed
         *  (moved by at most the length tolerance): after a real advance the vertices x - p moved, so the new v is
         *  not comparable with the previous one (stopping there on a repeated support reported hits at fractions
         *  where the shapes were still apart, for polytopes without margin). A support that does not repeat but is
         *  dropped again by the sub-algorithm is no progress either (it would cycle up to the iteration cap). The
         *  first v (from arbitrary points of the shapes) is no measure of progress.
         */
        if ( previousCount > 0 && !advanced && !( v.GetLength() < previousV.GetLength() ) )
            break;
    }

    result.status = status;
    result.hit = true;
    result.fraction = lambda;
    result.iterations = iterations;
    TVector unitNormal;
    if ( normal.TryNormalize( unitNormal, TTraits::Zero() ) )
        result.normal = unitNormal;
    TPoint onA;
    TPoint onB;
    GetClosestPoints( simplex, onA, onB );
    result.pointOnB = onB;
    result.pointOnA = onA + x;

    /*
     *  Shapes with margins touch with their cores apart: the nearest points of the cores at the hit give the exact
     *  time (two advancement steps on the gap of the cores), normal and contact points (the cast itself converges
     *  slowly on curved surfaces and its last advancing direction is only close to the normal)
     */
    const T margin = a.GetMargin() + b.GetMargin();
    if ( TTraits::Zero() < lambda && TTraits::Zero() < margin )
    {
        SGjkResult< T > atHit;
        for ( UInt32 refinement=0; refinement<3; ++refinement )
        {
            /* the cast stops within the length tolerance of the surface: two advancement steps on the exact gap of the cores */
            const CTTransformedSupport< TA > moved( a, MATH::CTMatrix3D< T >::Identity(), x );
            result.status |= ComputeDistance( moved, b, tolerance, GUCEF_NULL, atHit );
            if ( 2 == refinement || atHit.coresOverlapping || atHit.normal.IsZero() )
                break;
            const T approach = r.Dot( atHit.normal );
            if ( !( TTraits::Zero() < approach ) )
                break;
            T next = lambda + ( atHit.coreDistance - margin ) / approach;
            next = TTraits::IsNegative( next ) ? TTraits::Zero() : ( maxFraction < next ? maxFraction : next );
            if ( next == lambda )
                break;
            lambda = next;
            x = r * lambda;
        }
        result.fraction = lambda;
        if ( !atHit.coresOverlapping && !atHit.normal.IsZero() )
        {
            result.normal = -atHit.normal;
            const T along = atHit.coreDistance < margin ? atHit.coreDistance * a.GetMargin() / margin : a.GetMargin();
            result.pointOnA = atHit.corePointOnA + atHit.normal * along;
            result.pointOnB = atHit.corePointOnB - atHit.normal * ( atHit.coreDistance < margin ? atHit.coreDistance - along : b.GetMargin() );
        }
    }
    return result.status;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTGjk< Float32 > TFloat32Gjk;
typedef CTGjk< Float64 > TFloat64Gjk;
typedef CTGjk< MATH::TFixed32_32 > TFixed32_32Gjk;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The sub-algorithm is exported for Float32, Float64 and TFixed32_32 (section 5.10); the member templates over the
 *  support mappings are instantiated through the query functions of gucefGEOMETRY_ConvexQueries.h.
 */

#define GUCEF_GEOMETRY_CTGJK_KIND( prefix, kind ) \
    prefix void CTGjk< kind >::SolveSimplex( SGjkSimplex< kind >&, MATH::CTVector3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix void CTGjk< kind >::GetClosestPoints( const SGjkSimplex< kind >&, MATH::CTPoint3D< kind >&, MATH::CTPoint3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix kind CTGjk< kind >::GetMagnitude( const MATH::CTVector3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_CTGJK_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_CTGJK_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_CTGJK_KIND( prefix, Float64 ) \
    GUCEF_GEOMETRY_CTGJK_KIND( prefix, MATH::TFixed32_32 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_CTGJK_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGJK_H ? */
