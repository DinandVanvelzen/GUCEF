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

#ifndef GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H
#define GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H

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

#ifndef GUCEF_GEOMETRY_CTPOLYLINE2D_H
#include "gucefGEOMETRY_CTPolyline2D.h"
#define GUCEF_GEOMETRY_CTPOLYLINE2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYLINE2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYLINE3D_H
#include "gucefGEOMETRY_CTPolyline3D.h"
#define GUCEF_GEOMETRY_CTPOLYLINE3D_H
#endif /* GUCEF_GEOMETRY_CTPOLYLINE3D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGON2D_H
#include "gucefGEOMETRY_CTPolygon2D.h"
#define GUCEF_GEOMETRY_CTPOLYGON2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGON2D_H ? */

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

/** The squared distance from 'point' to the segment 'a' to 'b' (a degenerate segment is the point 'a'); points or vectors of 2 or 3 components */
template < typename TPoint >
typename TPoint::TValue
PolylineSimplificationSquaredDistance( const TPoint& point, const TPoint& a, const TPoint& b )
{
    typedef typename TPoint::TValue T;
    const T zero = MATH::CTNumericTraits< T >::Zero();
    const typename TPoint::TVector ab = b - a;
    const typename TPoint::TVector ap = point - a;
    const T projected = ap.Dot( ab );
    if ( !( projected > zero ) )
        return ap.GetSquaredLength();
    const T lengthSquared = ab.GetSquaredLength();
    if ( !( projected < lengthSquared ) )
        return ( point - b ).GetSquaredLength();
    return ( ap - ab * ( projected / lengthSquared ) ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

/**
 *  Douglas-Peucker on 'count' points, iterative with an explicit stack (section 7.9 of the design): the first and the
 *  last point are kept; the point furthest from the segment between two kept neighbours (the lowest index on ties)
 *  is kept when its distance is more than 'epsilon', and the two parts are simplified in turn. 'kept' receives the
 *  indices of the kept points in increasing order. Every removed point is within 'epsilon' of the simplified polyline
 *  (of the segment that replaced it); with an epsilon of 0 every point that is not on the segment of its neighbours is
 *  kept (collinear points may be kept too, the distance is computed in the kind). An epsilon that is negative or not a
 *  number keeps every point. Not realtime-safe (allocates the stack and the flags).
 */
template < typename TPoint >
UInt32
SimplifyPolylineIndices( const TPoint* points, UInt32 count, const typename TPoint::TValue& epsilon, GUCEF::vector< UInt32 >& kept )
{
    typedef typename TPoint::TValue T;
    kept.clear();
    if ( 0 == count )
        return 0;
    GUCEF::vector< UInt8 > keep( count, 0 );
    keep[ 0 ] = 1;
    keep[ count - 1 ] = 1;
    if ( count > 2 )
    {
        if ( !( epsilon >= MATH::CTNumericTraits< T >::Zero() ) )
        {
            for ( UInt32 i=0; i<count; ++i )
                keep[ i ] = 1;
        }
        else
        {
            const T epsilonSquared = epsilon * epsilon;
            GUCEF::vector< UInt32 > stack;
            stack.push_back( 0 );
            stack.push_back( count - 1 );
            while ( !stack.empty() )
            {
                const UInt32 last = stack.back();
                stack.pop_back();
                const UInt32 first = stack.back();
                stack.pop_back();
                if ( last <= first + 1 )
                    continue;
                UInt32 furthest = first;
                T furthestSquared = MATH::CTNumericTraits< T >::Zero();
                for ( UInt32 i=first+1; i<last; ++i )
                {
                    const T d = PolylineSimplificationSquaredDistance( points[ i ], points[ first ], points[ last ] );
                    if ( furthest == first || d > furthestSquared )
                    {
                        furthest = i;
                        furthestSquared = d;
                    }
                }
                // a distance that is not a number (non finite input) keeps the point
                if ( !( furthestSquared <= epsilonSquared ) )
                {
                    keep[ furthest ] = 1;
                    stack.push_back( furthest );
                    stack.push_back( last );
                    stack.push_back( first );
                    stack.push_back( furthest );
                }
            }
        }
    }
    for ( UInt32 i=0; i<count; ++i )
        if ( 0 != keep[ i ] )
            kept.push_back( i );
    return (UInt32) kept.size();
}

/*-------------------------------------------------------------------------*/

/** The simplified polyline (see SimplifyPolylineIndices); 'out' may not be 'in'. Returns the number of points */
template < typename T >
UInt32
SimplifyPolyline( const CTPolyline2D< T >& in, const T& epsilon, CTPolyline2D< T >& out )
{
    GUCEF::vector< UInt32 > kept;
    SimplifyPolylineIndices( in.points.empty() ? static_cast< const MATH::CTPoint2D< T >* >( GUCEF_NULL ) : &in.points[ 0 ], (UInt32) in.points.size(), epsilon, kept );
    out.points.clear();
    for ( size_t i=0; i<kept.size(); ++i )
        out.points.push_back( in.points[ kept[ i ] ] );
    return (UInt32) out.points.size();
}

template < typename T >
UInt32
SimplifyPolyline( const CTPolyline3D< T >& in, const T& epsilon, CTPolyline3D< T >& out )
{
    GUCEF::vector< UInt32 > kept;
    SimplifyPolylineIndices( in.points.empty() ? static_cast< const MATH::CTPoint3D< T >* >( GUCEF_NULL ) : &in.points[ 0 ], (UInt32) in.points.size(), epsilon, kept );
    out.points.clear();
    for ( size_t i=0; i<kept.size(); ++i )
        out.points.push_back( in.points[ kept[ i ] ] );
    return (UInt32) out.points.size();
}

/*-------------------------------------------------------------------------*/

/**
 *  Douglas-Peucker on a closed ring: the ring is split at its first point and at the point furthest from it (the lowest
 *  index on ties), and both halves are simplified as polylines, so every removed point is within 'epsilon' of the
 *  simplified ring. A ring that is nearly a line can shrink to two points. 'kept' receives the indices in increasing
 *  order. Rings of fewer than four points are kept whole.
 */
template < typename T >
UInt32
SimplifyPolygonIndices( const MATH::CTPoint2D< T >* points, UInt32 count, const T& epsilon, GUCEF::vector< UInt32 >& kept )
{
    kept.clear();
    if ( count < 4 )
    {
        for ( UInt32 i=0; i<count; ++i )
            kept.push_back( i );
        return count;
    }
    UInt32 far = 1;
    T farSquared = ( points[ 1 ] - points[ 0 ] ).GetSquaredLength();
    for ( UInt32 i=2; i<count; ++i )
    {
        const T d = ( points[ i ] - points[ 0 ] ).GetSquaredLength();
        if ( d > farSquared )
        {
            far = i;
            farSquared = d;
        }
    }
    // half one: points 0 .. far, half two: points far .. count - 1 and back to 0
    GUCEF::vector< MATH::CTPoint2D< T > > half;
    GUCEF::vector< UInt32 > halfKept;
    for ( UInt32 i=0; i<=far; ++i )
        half.push_back( points[ i ] );
    SimplifyPolylineIndices( &half[ 0 ], (UInt32) half.size(), epsilon, halfKept );
    for ( size_t i=0; i<halfKept.size(); ++i )
        kept.push_back( halfKept[ i ] );
    half.clear();
    for ( UInt32 i=far; i<count; ++i )
        half.push_back( points[ i ] );
    half.push_back( points[ 0 ] );
    SimplifyPolylineIndices( &half[ 0 ], (UInt32) half.size(), epsilon, halfKept );
    for ( size_t i=1; i<halfKept.size(); ++i )
        if ( far + halfKept[ i ] < count )
            kept.push_back( far + halfKept[ i ] );
    return (UInt32) kept.size();
}

/** The simplified ring (see SimplifyPolygonIndices); 'out' may not be 'in' */
template < typename T >
UInt32
SimplifyPolygon( const CTPolygon2D< T >& in, const T& epsilon, CTPolygon2D< T >& out )
{
    GUCEF::vector< UInt32 > kept;
    SimplifyPolygonIndices( in.points.empty() ? static_cast< const MATH::CTPoint2D< T >* >( GUCEF_NULL ) : &in.points[ 0 ], (UInt32) in.points.size(), epsilon, kept );
    out.points.clear();
    for ( size_t i=0; i<kept.size(); ++i )
        out.points.push_back( in.points[ kept[ i ] ] );
    return (UInt32) out.points.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_POLYLINESIMPLIFICATION_H ? */
