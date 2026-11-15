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

#ifndef GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H
#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H

/*
 *  Bounding-volume fitting (section 7.6.6 of the design):
 *
 *  - FitMinimumSphereSquared / FitMinimumSphere: the smallest enclosing sphere of a point set by Welzl's algorithm in
 *    the move-to-front form (recursion depth at most 4, no deep recursion). The points are first put in a canonical
 *    order (sorted by x, then y, then z) and then shuffled with the seed, so the result depends only on the set of
 *    points and the seed, not on the order of the input (expected linear time after the O(n log n) sort). The squared
 *    form needs no square root and is exact for the fraction kinds; floats compare with the relative tolerance of the
 *    kind so that rounding cannot make a point fall out of the sphere through its own support set, and treat support
 *    sets that are collinear or coplanar within that tolerance as degenerate (the circumsphere of nearly coplanar
 *    points is huge and meaningless). Float and fraction kinds only: fixed point and integers are rejected at compile
 *    time (the squared lengths and determinants overflow Q32.32 for coordinates above about 200).
 *  - FitRitterSphere: Ritter's approximation (two passes, at most about 5 % larger), a fast alternative.
 *  - FitOrientedBox: the principal component box: the eigenvectors of the covariance of the points
 *    (MATH::CTSymmetricEigen3) as axes, the extents from the projections. Float kinds.
 *  - FitOrientedBoxFromHull: the smallest of the principal component box of the vertices and the boxes with one axis
 *    along a face normal and one along an edge of that face (O( E * V ) for E face edges and V vertices).
 *
 *  The axis aligned box of points is CTAxisAlignedBox3D::FromPoints. Every function returns false (outputs unchanged)
 *  for an empty point set. The sphere functions take a caller-provided scratch array of 'count' points (the shuffled
 *  copy), so they do not allocate; the overloads with a GUCEF::vector grow it when needed. Not realtime-safe in general
 *  (time grows with the number of points), allocation-free with a caller-provided scratch array.
 */

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

#ifndef GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#include "gucefGEOMETRY_CTConvexHull3D.h"
#define GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#endif /* GUCEF_GEOMETRY_CTCONVEXHULL3D_H ? */

#ifndef GUCEF_GEOMETRY_CTSPHERE_H
#include "gucefGEOMETRY_CTSphere.h"
#define GUCEF_GEOMETRY_CTSPHERE_H
#endif /* GUCEF_GEOMETRY_CTSPHERE_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

#ifndef GUCEF_MATH_CSPLITMIX64_H
#include "gucefMATH_CSplitMix64.h"
#define GUCEF_MATH_CSPLITMIX64_H
#endif /* GUCEF_MATH_CSPLITMIX64_H ? */

#ifndef GUCEF_MATH_CTSYMMETRICEIGEN3_H
#include "gucefMATH_CTSymmetricEigen3.h"
#define GUCEF_MATH_CTSYMMETRICEIGEN3_H
#endif /* GUCEF_MATH_CTSYMMETRICEIGEN3_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** True when the point lies in the sphere (centre, squared radius), with the relative slack of the kind (exact kinds: none) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
SphereContainsSquared( const MATH::CTPoint3D< T >& centre, const T& squaredRadius, const MATH::CTPoint3D< T >& point ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const CTGeometryTolerance< T > tolerance;
    const T slack = squaredRadius * tolerance.relative * MATH::CTNumericTraits< T >::FromInt32( 4 ) + tolerance.absolute * tolerance.absolute;
    return ( point - centre ).GetSquaredLength() <= squaredRadius + slack;
}

/*-------------------------------------------------------------------------*/

/**
 *  The smallest sphere with the given points (0 to 4) on its surface, as centre and squared radius; false for no
 *  points. Affinely dependent points (three collinear, four coplanar) give the smallest sphere of a subset that
 *  contains them all: the result then still encloses the points. Floats count points as collinear when the squared
 *  sine of their angle is within the relative tolerance of the kind, and as coplanar when the squared normalised
 *  volume is (exact kinds: exactly). Exact for the fraction kinds; float and fraction kinds only.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
SphereThroughPoints( const MATH::CTPoint3D< T >* points, UInt32 count, MATH::CTPoint3D< T >& centre, T& squaredRadius ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    GUCEF_GEOMETRY_STATIC_ASSERT( !TTraits::IsFixedPoint && !TTraits::IsInteger, SphereThroughPoints_requires_a_floating_point_or_fraction_kind );
    const T two = TTraits::FromInt32( 2 );
    const T relative = CTGeometryTolerance< T >().relative;
    if ( 0 == count )
        return false;
    if ( 1 == count )
    {
        centre = points[ 0 ];
        squaredRadius = TTraits::Zero();
        return true;
    }
    if ( 2 == count )
    {
        const TVector d = points[ 1 ] - points[ 0 ];
        centre = points[ 0 ] + d / two;
        squaredRadius = ( centre - points[ 0 ] ).GetSquaredLength();
        return true;
    }
    if ( 3 == count )
    {
        const TVector u = points[ 1 ] - points[ 0 ];
        const TVector v = points[ 2 ] - points[ 0 ];
        const TVector w = u.Cross( v );
        const T ww = w.Dot( w );
        /* |u x v|^2 = |u|^2 |v|^2 sin^2: collinear within the tolerance (exactly 0 for the exact kinds) */
        if ( !( relative * u.Dot( u ) * v.Dot( v ) < ww ) )
        {
            /* collinear: the sphere over the two points that are furthest apart */
            const UInt32 pairs[ 3 ][ 2 ] = { { 0, 1 }, { 0, 2 }, { 1, 2 } };
            UInt32 best = 0;
            T bestLength = TTraits::Zero();
            for ( UInt32 p=0; p<3; ++p )
            {
                const T length = ( points[ pairs[ p ][ 1 ] ] - points[ pairs[ p ][ 0 ] ] ).GetSquaredLength();
                if ( 0 == p || bestLength < length )
                {
                    bestLength = length;
                    best = p;
                }
            }
            const TPoint pair[ 2 ] = { points[ pairs[ best ][ 0 ] ], points[ pairs[ best ][ 1 ] ] };
            return SphereThroughPoints( pair, 2, centre, squaredRadius );
        }
        const TVector offset = ( v.Cross( w ) * u.Dot( u ) + w.Cross( u ) * v.Dot( v ) ) / ( two * ww );
        centre = points[ 0 ] + offset;
        squaredRadius = offset.GetSquaredLength();
        return true;
    }
    const TVector u = points[ 1 ] - points[ 0 ];
    const TVector v = points[ 2 ] - points[ 0 ];
    const TVector w = points[ 3 ] - points[ 0 ];
    const T determinant = u.Dot( v.Cross( w ) );
    /* determinant^2 against |u|^2 |v|^2 |w|^2: coplanar within the tolerance (exactly 0 for the exact kinds) */
    if ( !( relative * u.Dot( u ) * v.Dot( v ) * w.Dot( w ) < determinant * determinant ) )
    {
        /* coplanar: the smallest sphere through three of them that encloses the fourth */
        bool found = false;
        for ( UInt32 skip=0; skip<4; ++skip )
        {
            TPoint subset[ 3 ];
            UInt32 n = 0;
            for ( UInt32 i=0; i<4; ++i )
                if ( i != skip )
                    subset[ n++ ] = points[ i ];
            TPoint c;
            T r2 = TTraits::Zero();
            SphereThroughPoints( subset, 3, c, r2 );
            if ( SphereContainsSquared( c, r2, points[ skip ] ) && ( !found || r2 < squaredRadius ) )
            {
                found = true;
                centre = c;
                squaredRadius = r2;
            }
        }
        if ( found )
            return true;
        if ( TTraits::Zero() == determinant )
            return SphereThroughPoints( points, 3, centre, squaredRadius );
    }
    const TVector offset = ( v.Cross( w ) * u.Dot( u ) + w.Cross( u ) * v.Dot( v ) + u.Cross( v ) * w.Dot( w ) ) / ( two * determinant );
    centre = points[ 0 ] + offset;
    squaredRadius = offset.GetSquaredLength();
    return true;
}

/*-------------------------------------------------------------------------*/

/** One level of the move-to-front Welzl recursion: the smallest sphere of points[ 0 .. end ) with 'boundary' on its surface */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
WelzlMoveToFront( MATH::CTPoint3D< T >* points, UInt32 end, MATH::CTPoint3D< T > ( &boundary )[ 4 ], UInt32 boundaryCount,
                  MATH::CTPoint3D< T >& centre, T& squaredRadius, bool& valid ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    valid = SphereThroughPoints( boundary, boundaryCount, centre, squaredRadius );
    if ( 4 == boundaryCount )
        return;
    for ( UInt32 i=0; i<end; ++i )
    {
        if ( valid && SphereContainsSquared( centre, squaredRadius, points[ i ] ) )
            continue;
        boundary[ boundaryCount ] = points[ i ];
        WelzlMoveToFront( points, i, boundary, boundaryCount + 1, centre, squaredRadius, valid );
        const MATH::CTPoint3D< T > moved = points[ i ];
        for ( UInt32 j=i; j>0; --j )
            points[ j ] = points[ j - 1 ];
        points[ 0 ] = moved;
    }
}

/*-------------------------------------------------------------------------*/

/** Lexicographic order of points (x, then y, then z): the canonical order of the Welzl input */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
PointLexicographicallyLess( const MATH::CTPoint3D< T >& a, const MATH::CTPoint3D< T >& b ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( a.x < b.x )
        return true;
    if ( b.x < a.x )
        return false;
    if ( a.y < b.y )
        return true;
    if ( b.y < a.y )
        return false;
    return a.z < b.z;
}

/*-------------------------------------------------------------------------*/

/** Sorts points lexicographically in place (heap sort: O( n log n ), no allocation, no recursion) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
SortPointsLexicographically( MATH::CTPoint3D< T >* points, UInt32 count ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( count < 2 )
        return;
    /* build a max-heap, then move the largest to the end repeatedly */
    for ( UInt32 start=count/2; start>0; --start )
    {
        UInt32 root = start - 1;
        for ( ;; )
        {
            UInt32 child = 2 * root + 1;
            if ( child >= count )
                break;
            if ( child + 1 < count && PointLexicographicallyLess( points[ child ], points[ child + 1 ] ) )
                ++child;
            if ( !PointLexicographicallyLess( points[ root ], points[ child ] ) )
                break;
            const MATH::CTPoint3D< T > swap = points[ root ];
            points[ root ] = points[ child ];
            points[ child ] = swap;
            root = child;
        }
    }
    for ( UInt32 end=count-1; end>0; --end )
    {
        const MATH::CTPoint3D< T > largest = points[ 0 ];
        points[ 0 ] = points[ end ];
        points[ end ] = largest;
        UInt32 root = 0;
        for ( ;; )
        {
            UInt32 child = 2 * root + 1;
            if ( child >= end )
                break;
            if ( child + 1 < end && PointLexicographicallyLess( points[ child ], points[ child + 1 ] ) )
                ++child;
            if ( !PointLexicographicallyLess( points[ root ], points[ child ] ) )
                break;
            const MATH::CTPoint3D< T > swap = points[ root ];
            points[ root ] = points[ child ];
            points[ child ] = swap;
            root = child;
        }
    }
}

/*-------------------------------------------------------------------------*/

/**
 *  The smallest enclosing sphere of 'count' points as centre and squared radius (Welzl, move-to-front, in canonical
 *  order shuffled with 'seed': the result does not depend on the order of the input); 'scratch' must hold 'count'
 *  points (it receives the shuffled points). False for no points.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
FitMinimumSphereSquared( const MATH::CTPoint3D< T >* points, UInt32 count, MATH::CTPoint3D< T >* scratch, UInt64 seed, MATH::CTPoint3D< T >& centre, T& squaredRadius ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( 0 == count || GUCEF_NULL == points || GUCEF_NULL == scratch )
        return false;
    for ( UInt32 i=0; i<count; ++i )
        scratch[ i ] = points[ i ];
    SortPointsLexicographically( scratch, count );
    MATH::CSplitMix64 random( seed );
    for ( UInt32 i=count-1; i>0; --i )
    {
        const UInt32 j = (UInt32) ( random.NextUInt64() % (UInt64) ( i + 1 ) );
        const MATH::CTPoint3D< T > swap = scratch[ i ];
        scratch[ i ] = scratch[ j ];
        scratch[ j ] = swap;
    }
    MATH::CTPoint3D< T > boundary[ 4 ];
    MATH::CTPoint3D< T > c;
    T r2 = MATH::CTNumericTraits< T >::Zero();
    bool valid = false;
    WelzlMoveToFront( scratch, count, boundary, 0, c, r2, valid );
    if ( !valid )
        return false;
    centre = c;
    squaredRadius = r2;
    return true;
}

/*-------------------------------------------------------------------------*/

/** The smallest enclosing sphere of 'count' points (Welzl, see FitMinimumSphereSquared); kinds with a square root */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
FitMinimumSphere( const MATH::CTPoint3D< T >* points, UInt32 count, MATH::CTPoint3D< T >* scratch, UInt64 seed, CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    MATH::CTPoint3D< T > centre;
    T squaredRadius = MATH::CTNumericTraits< T >::Zero();
    if ( !FitMinimumSphereSquared( points, count, scratch, seed, centre, squaredRadius ) )
        return false;
    sphere = CTSphere< T >( centre, MATH::CTNumericTraits< T >::Sqrt( squaredRadius ) );
    return true;
}

/*-------------------------------------------------------------------------*/

/** FitMinimumSphere with a scratch vector that grows when needed (allocates only then) */
template < typename T >
bool
FitMinimumSphere( const MATH::CTPoint3D< T >* points, UInt32 count, GUCEF::vector< MATH::CTPoint3D< T > >& scratch, UInt64 seed, CTSphere< T >& sphere )
{
    if ( 0 == count )
        return false;
    if ( scratch.size() < count )
        scratch.resize( count );
    return FitMinimumSphere( points, count, &scratch[ 0 ], seed, sphere );
}

/*-------------------------------------------------------------------------*/

/** Ritter's sphere: a fast enclosing sphere, usually a few percent larger than the smallest one */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
FitRitterSphere( const MATH::CTPoint3D< T >* points, UInt32 count, CTSphere< T >& sphere ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    if ( 0 == count || GUCEF_NULL == points )
        return false;
    UInt32 y = 0;
    T farthest = TTraits::Zero();
    for ( UInt32 i=1; i<count; ++i )
    {
        const T d = ( points[ i ] - points[ 0 ] ).GetSquaredLength();
        if ( farthest < d )
        {
            farthest = d;
            y = i;
        }
    }
    UInt32 z = y;
    farthest = TTraits::Zero();
    for ( UInt32 i=0; i<count; ++i )
    {
        const T d = ( points[ i ] - points[ y ] ).GetSquaredLength();
        if ( farthest < d )
        {
            farthest = d;
            z = i;
        }
    }
    TPoint centre = points[ y ] + ( points[ z ] - points[ y ] ) * TTraits::Half();
    T radius = ( points[ z ] - points[ y ] ).GetLength() * TTraits::Half();
    for ( UInt32 i=0; i<count; ++i )
    {
        const T d = ( points[ i ] - centre ).GetLength();
        if ( radius < d )
        {
            const T grown = ( radius + d ) * TTraits::Half();
            centre = centre + ( points[ i ] - centre ) * ( ( grown - radius ) / d );
            radius = grown;
        }
    }
    sphere = CTSphere< T >( centre, radius );
    return true;
}

/*-------------------------------------------------------------------------*/

/** The box with the given unit axes (a rotation, as columns) that encloses the points: the extents from the projections */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTOrientedBox3D< T >
FitOrientedBoxWithAxes( const MATH::CTPoint3D< T >* points, UInt32 count, const MATH::CTMatrix3D< T >& axes ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    const TVector a[ 3 ] = { axes.GetColumn( 0 ), axes.GetColumn( 1 ), axes.GetColumn( 2 ) };
    T low[ 3 ];
    T high[ 3 ];
    for ( UInt32 k=0; k<3; ++k )
    {
        low[ k ] = TTraits::Zero();
        high[ k ] = TTraits::Zero();
    }
    /* projections relative to the first point, so that coordinates farthest from the origin cost no precision */
    for ( UInt32 i=1; i<count; ++i )
    {
        const TVector d = points[ i ] - points[ 0 ];
        for ( UInt32 k=0; k<3; ++k )
        {
            const T p = a[ k ].Dot( d );
            low[ k ] = TTraits::Min( low[ k ], p );
            high[ k ] = TTraits::Max( high[ k ], p );
        }
    }
    TVector centreOffset;
    for ( UInt32 k=0; k<3; ++k )
        centreOffset = centreOffset + a[ k ] * ( ( low[ k ] + high[ k ] ) * TTraits::Half() );
    const TVector half( ( high[ 0 ] - low[ 0 ] ) * TTraits::Half(), ( high[ 1 ] - low[ 1 ] ) * TTraits::Half(), ( high[ 2 ] - low[ 2 ] ) * TTraits::Half() );
    return CTOrientedBox3D< T >( points[ 0 ] + centreOffset, MATH::CTQuaternion< T >::FromRotationMatrix( axes ), half );
}

/*-------------------------------------------------------------------------*/

/** The principal component box of the points (float kinds) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
FitOrientedBox( const MATH::CTPoint3D< T >* points, UInt32 count, CTOrientedBox3D< T >& box ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTVector3D< T > TVector;
    GUCEF_GEOMETRY_STATIC_ASSERT( TTraits::IsFloatingPoint, FitOrientedBox_requires_a_floating_point_kind );
    if ( 0 == count || GUCEF_NULL == points )
        return false;
    /* mean and covariance relative to the first point */
    TVector mean;
    for ( UInt32 i=1; i<count; ++i )
        mean = mean + ( points[ i ] - points[ 0 ] );
    mean = mean / TTraits::FromInt32( (Int32) count );
    T c[ 3 ][ 3 ];
    for ( UInt32 r=0; r<3; ++r )
        for ( UInt32 k=0; k<3; ++k )
            c[ r ][ k ] = TTraits::Zero();
    for ( UInt32 i=0; i<count; ++i )
    {
        const TVector d = ( points[ i ] - points[ 0 ] ) - mean;
        const T v[ 3 ] = { d.x, d.y, d.z };
        for ( UInt32 r=0; r<3; ++r )
            for ( UInt32 k=r; k<3; ++k )
                c[ r ][ k ] = c[ r ][ k ] + v[ r ] * v[ k ];
    }
    const MATH::CTMatrix3D< T > covariance = MATH::CTMatrix3D< T >::FromColumns( TVector( c[ 0 ][ 0 ], c[ 0 ][ 1 ], c[ 0 ][ 2 ] ),
                                                                              TVector( c[ 0 ][ 1 ], c[ 1 ][ 1 ], c[ 1 ][ 2 ] ),
                                                                              TVector( c[ 0 ][ 2 ], c[ 1 ][ 2 ], c[ 2 ][ 2 ] ) );
    MATH::CTSymmetricEigen3< T > eigen;
    eigen.Decompose( covariance );
    box = FitOrientedBoxWithAxes( points, count, eigen.GetEigenvectors() );
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  The smallest of the principal component box of the hull vertices and the boxes with an axis along a face normal and
 *  a second axis along an edge of that face (float kinds). Without faces: the principal component box.
 */
template < typename T >
bool
FitOrientedBoxFromHull( const CTConvexHull3D< T >& hull, CTOrientedBox3D< T >& box )
{
    typedef MATH::CTVector3D< T > TVector;
    if ( hull.IsEmpty() )
        return false;
    const MATH::CTPoint3D< T >* points = &hull.GetVertices()[ 0 ];
    const UInt32 count = hull.GetVertexCount();
    CTOrientedBox3D< T > best;
    FitOrientedBox( points, count, best );
    T bestVolume = best.GetVolume();
    for ( UInt32 f=0; f<hull.GetFaceCount(); ++f )
    {
        /* the face normal with each edge of the same face (an edge of a face lies in its plane) */
        const TVector n = hull.GetFacePlane( f ).normal.AsVector();
        const UInt32 size = hull.GetFaceVertexCount( f );
        for ( UInt32 k=0; k<size; ++k )
        {
            const TVector edge = hull.GetVertex( hull.GetFaceVertexIndex( f, ( k + 1 ) % size ) ) - hull.GetVertex( hull.GetFaceVertexIndex( f, k ) );
            TVector second;
            if ( !( edge - n * edge.Dot( n ) ).TryNormalize( second, CTGeometryTolerance< T >().angular ) )
                continue;
            const TVector third = n.Cross( second );
            const CTOrientedBox3D< T > candidate = FitOrientedBoxWithAxes( points, count, MATH::CTMatrix3D< T >::FromColumns( second, third, n ) );
            const T volume = candidate.GetVolume();
            if ( volume < bestVolume )
            {
                bestVolume = volume;
                best = candidate;
            }
        }
    }
    box = best;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* For Float32 and Float64 the module's instantiations (gucefGEOMETRY_ConvexQueryInstantiations.cpp) are used */

#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_KIND( prefix, kind ) \
    prefix bool SphereThroughPoints< kind >( const MATH::CTPoint3D< kind >*, UInt32, MATH::CTPoint3D< kind >&, kind& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool FitMinimumSphereSquared< kind >( const MATH::CTPoint3D< kind >*, UInt32, MATH::CTPoint3D< kind >*, UInt64, MATH::CTPoint3D< kind >&, kind& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool FitMinimumSphere< kind >( const MATH::CTPoint3D< kind >*, UInt32, MATH::CTPoint3D< kind >*, UInt64, CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool FitRitterSphere< kind >( const MATH::CTPoint3D< kind >*, UInt32, CTSphere< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix CTOrientedBox3D< kind > FitOrientedBoxWithAxes< kind >( const MATH::CTPoint3D< kind >*, UInt32, const MATH::CTMatrix3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT; \
    prefix bool FitOrientedBox< kind >( const MATH::CTPoint3D< kind >*, UInt32, CTOrientedBox3D< kind >& ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

#define GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_PREDEFINED_KINDS( prefix ) \
    GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_KIND( prefix, Float32 ) \
    GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_KIND( prefix, Float64 )

#ifdef GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED
GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_PREDEFINED_KINDS( GUCEF_GEOMETRY_EXTERN_TEMPLATE )
#endif /* GUCEF_GEOMETRY_EXTERN_TEMPLATES_ENABLED ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_BOUNDINGVOLUMEFITTING_H ? */
