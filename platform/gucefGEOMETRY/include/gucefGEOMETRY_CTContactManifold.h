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

#ifndef GUCEF_GEOMETRY_CTCONTACTMANIFOLD_H
#define GUCEF_GEOMETRY_CTCONTACTMANIFOLD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

#ifndef GUCEF_MATH_MATHSTATUS_H
#include "gucefMATH_MathStatus.h"
#define GUCEF_MATH_MATHSTATUS_H
#endif /* GUCEF_MATH_MATHSTATUS_H ? */

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
 *  One point of a contact manifold: the point on the surface of each shape and the depth along the manifold normal
 *  (positive when the shapes overlap there, negative for the separation of a speculative or separated pair). The
 *  feature id names the features that produced the point (see the producer); 0 when not specified.
 */
template < typename T >
struct SContactPoint
{
    MATH::CTPoint3D< T > pointOnA;
    MATH::CTPoint3D< T > pointOnB;
    T depth;
    UInt32 featureId;

    SContactPoint( void ) : pointOnA(), pointOnB(), depth( MATH::CTNumericTraits< T >::Zero() ), featureId( 0 ) {}
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A single-shot contact manifold between two convex shapes A and B (sections 7.5 and 7.6.4 of the design): the
 *  contact normal and up to MaxPoints contact points. Persistent manifolds (matching over time, cached impulses) are
 *  not part of gucefGEOMETRY.
 *
 *  - normal: unit vector from A towards B: moving B along the normal by the depth of a point separates the shapes at
 *    that point. The zero vector when no direction is defined (no points, or two identical shapes without a
 *    preferred direction).
 *  - points: pointOnA lies on (or in) A, pointOnB on (or in) B; for an overlapping point pointOnA - pointOnB is
 *    normal * depth up to rounding, for a separated point pointOnB - pointOnA is normal * -depth.
 *  - status: the MATH::TMathStatus of the query that filled the manifold (MATHSTATUS_OK after Clear); for example
 *    MATHSTATUS_NOT_CONVERGED when GJK or EPA stopped at an iteration cap and the points are an estimate.
 *  - Capacity MaxPoints = 8 (a box face clipped against another box face gives up to 8 points); Reduce keeps the
 *    deepest point and maximises the contact area, so physics can keep 4.
 *  - Kind: floats and fixed point (Float32, Float64 and TFixed32_32 are exported). Every member uses only + - * and
 *    comparisons, so the fixed-point form is bit-exact.
 *  - Layout: a fixed array, no heap use; copying copies the points.
 *  - Realtime: every member except ToString is realtime-safe.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTContactManifold
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef SContactPoint< T > TContactPoint;

    enum { MaxPoints = 8 };

    TVector normal;
    UInt32 pointCount;
    TContactPoint points[ MaxPoints ];
    MATH::TMathStatus status;

    /** No points, zero normal, status MATHSTATUS_OK */
    CTContactManifold( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT : normal(), pointCount( 0 ), status( MATH::MATHSTATUS_OK ) {}

    /** Removes every point, zeroes the normal and sets the status to MATHSTATUS_OK */
    GUCEF_GEOMETRY_FORCE_INLINE void Clear( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        normal = TVector();
        pointCount = 0;
        status = MATH::MATHSTATUS_OK;
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPointCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return pointCount; }

    /** The point at 'index' (below GetPointCount; asserts in debug builds and gives the first slot otherwise) */
    GUCEF_GEOMETRY_FORCE_INLINE const TContactPoint& GetPoint( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( index < pointCount );
        return points[ index < MaxPoints ? index : 0 ];
    }

    /** Appends a point; false (nothing changes) when the manifold is full */
    GUCEF_GEOMETRY_FORCE_INLINE bool AddPoint( const TPoint& onA, const TPoint& onB, const T& depth, UInt32 featureId ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( pointCount >= (UInt32) MaxPoints )
            return false;
        TContactPoint& p = points[ pointCount ];
        p.pointOnA = onA;
        p.pointOnB = onB;
        p.depth = depth;
        p.featureId = featureId;
        ++pointCount;
        return true;
    }

    /** True when the manifold has a point with a depth of zero or more (the shapes touch or overlap) */
    bool IsTouching( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The index of the deepest point (the first on ties); 0 without points */
    UInt32 GetDeepestPointIndex( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The largest depth; the lowest number of the kind without points */
    T GetMaxDepth( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The manifold with the roles of A and B exchanged: the normal negated and the two points of each contact swapped */
    void Flip( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Keeps min( maxPoints, GetPointCount() ) points (maxPoints at least 1): the deepest point, then the point furthest
     *  from it in the contact plane, then the point that spans the largest triangle with those, then repeatedly the point
     *  outside the polygon kept so far that adds the largest area; when no point lies outside any more, the deepest of the
     *  remaining points. The points are kept in that order. Ties keep the lower index, so the result depends only on the
     *  input. Distances and areas are measured on pointOnA, perpendicular to the normal.
     */
    void Reduce( UInt32 maxPoints ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** "ContactManifold(normal (x, y, z), points [(onA (x, y, z), onB (x, y, z), depth d, feature f), ...])" (allocates) */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "ContactManifold(normal " );
        text += normal.ToString();
        text += ", points [";
        for ( UInt32 i=0; i<pointCount && i<(UInt32) MaxPoints; ++i )
        {
            if ( i > 0 )
                text += ", ";
            text += "(onA " + points[ i ].pointOnA.ToString() + ", onB " + points[ i ].pointOnB.ToString() + ", depth " + TTraits::ToString( points[ i ].depth ) +
                    ", feature " + CORE::ToString( points[ i ].featureId ) + ")";
        }
        text += "])";
        return text;
    }

    private:

    /** True when 'index' is one of the first 'count' entries of 'chosen' */
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsChosen( const UInt32* chosen, UInt32 count, UInt32 index ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        for ( UInt32 k=0; k<count; ++k )
            if ( chosen[ k ] == index )
                return true;
        return false;
    }

    /** ( b - a ) without its component along the normal: the in-plane displacement used by Reduce */
    TVector InPlane( const TPoint& a, const TPoint& b ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTContactManifold< T >::IsTouching( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    for ( UInt32 i=0; i<pointCount; ++i )
        if ( !TTraits::IsNegative( points[ i ].depth ) )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE UInt32
CTContactManifold< T >::GetDeepestPointIndex( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    UInt32 best = 0;
    for ( UInt32 i=1; i<pointCount; ++i )
        if ( points[ best ].depth < points[ i ].depth )
            best = i;
    return best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTContactManifold< T >::GetMaxDepth( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( 0 == pointCount )
        return TTraits::Lowest();
    return points[ GetDeepestPointIndex() ].depth;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTContactManifold< T >::Flip( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    normal = -normal;
    for ( UInt32 i=0; i<pointCount; ++i )
    {
        const TPoint swap = points[ i ].pointOnA;
        points[ i ].pointOnA = points[ i ].pointOnB;
        points[ i ].pointOnB = swap;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTContactManifold< T >::TVector
CTContactManifold< T >::InPlane( const TPoint& a, const TPoint& b ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const TVector d = b - a;
    return d - normal * d.Dot( normal );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTContactManifold< T >::Reduce( UInt32 maxPoints ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( maxPoints < 1 )
        maxPoints = 1;
    if ( pointCount <= maxPoints )
        return;

    UInt32 chosen[ MaxPoints ];
    UInt32 chosenCount = 0;
    chosen[ chosenCount++ ] = GetDeepestPointIndex();

    if ( maxPoints >= 2 )
    {
        UInt32 farthest = chosen[ 0 ];
        T farthestDistance = TTraits::Zero();
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            const T d = InPlane( points[ chosen[ 0 ] ].pointOnA, points[ i ].pointOnA ).GetSquaredLength();
            if ( farthestDistance < d )
            {
                farthestDistance = d;
                farthest = i;
            }
        }
        if ( farthest != chosen[ 0 ] )
            chosen[ chosenCount++ ] = farthest;
    }

    T triangleSign = TTraits::Zero();
    if ( maxPoints >= 3 && 2 == chosenCount )
    {
        const TPoint& p0 = points[ chosen[ 0 ] ].pointOnA;
        const TVector e = InPlane( p0, points[ chosen[ 1 ] ].pointOnA );
        UInt32 best = pointCount;
        T bestArea = TTraits::Zero();
        for ( UInt32 i=0; i<pointCount; ++i )
        {
            const T signedArea = e.Cross( InPlane( p0, points[ i ].pointOnA ) ).Dot( normal );
            const T area = TTraits::Abs( signedArea );
            if ( bestArea < area )
            {
                bestArea = area;
                best = i;
                triangleSign = signedArea;
            }
        }
        if ( best < pointCount )
            chosen[ chosenCount++ ] = best;
    }

    if ( 3 == chosenCount && maxPoints > 3 )
    {
        /*
         *  Grow the polygon: each round adds the point furthest outside one of its edges, measured as the area it adds,
         *  and inserts it into the ring between the two ends of that edge (the ring keeps the orientation of the triangle)
         */
        const bool counterClockwise = !TTraits::IsNegative( triangleSign );
        UInt32 ring[ MaxPoints ];
        UInt32 ringCount = 3;
        ring[ 0 ] = chosen[ 0 ];
        ring[ 1 ] = chosen[ 1 ];
        ring[ 2 ] = chosen[ 2 ];
        while ( chosenCount < maxPoints )
        {
            UInt32 best = pointCount;
            UInt32 bestEdge = 0;
            T bestArea = TTraits::Zero();
            for ( UInt32 i=0; i<pointCount; ++i )
            {
                if ( IsChosen( chosen, chosenCount, i ) )
                    continue;
                for ( UInt32 e=0; e<ringCount; ++e )
                {
                    const TPoint& a = points[ ring[ e ] ].pointOnA;
                    const TPoint& b = points[ ring[ ( e + 1 ) % ringCount ] ].pointOnA;
                    T outside = InPlane( a, b ).Cross( InPlane( a, points[ i ].pointOnA ) ).Dot( normal );
                    if ( counterClockwise )
                        outside = -outside;
                    if ( bestArea < outside )
                    {
                        bestArea = outside;
                        best = i;
                        bestEdge = e;
                    }
                }
            }
            if ( best >= pointCount )
                break;
            for ( UInt32 k=ringCount; k>bestEdge+1; --k )
                ring[ k ] = ring[ k-1 ];
            ring[ bestEdge+1 ] = best;
            ++ringCount;
            chosen[ chosenCount++ ] = best;
        }
    }

    /* every remaining point lies inside the area kept (or on its line): fill the request with the deepest of them */
    while ( chosenCount < maxPoints )
    {
        UInt32 best = pointCount;
        for ( UInt32 i=0; i<pointCount; ++i )
            if ( !IsChosen( chosen, chosenCount, i ) && ( best >= pointCount || points[ best ].depth < points[ i ].depth ) )
                best = i;
        if ( best >= pointCount )
            break;
        chosen[ chosenCount++ ] = best;
    }

    TContactPoint kept[ MaxPoints ];
    for ( UInt32 i=0; i<chosenCount; ++i )
        kept[ i ] = points[ chosen[ i ] ];
    for ( UInt32 i=0; i<chosenCount; ++i )
        points[ i ] = kept[ i ];
    pointCount = chosenCount;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTContactManifold< Float32 > TFloat32ContactManifold;
typedef CTContactManifold< Float64 > TFloat64ContactManifold;
typedef CTContactManifold< MATH::TFixed32_32 > TFixed32_32ContactManifold;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTContactManifold< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTContactManifold< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTContactManifold< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCONTACTMANIFOLD_H ? */
