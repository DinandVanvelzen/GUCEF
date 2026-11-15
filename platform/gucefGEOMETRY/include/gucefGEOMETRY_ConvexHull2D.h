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

#ifndef GUCEF_GEOMETRY_CONVEXHULL2D_H
#define GUCEF_GEOMETRY_CONVEXHULL2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Orders the indices of a point array by x, then y, then index: a total order, so the sort is deterministic */
template < typename T >
class CTConvexHull2DLess
{
    public:

    explicit CTConvexHull2DLess( const MATH::CTPoint2D< T >* points ) : m_points( points ) {}

    bool operator()( UInt32 a, UInt32 b ) const
    {
        const MATH::CTPoint2D< T >& pa = m_points[ a ];
        const MATH::CTPoint2D< T >& pb = m_points[ b ];
        if ( pa.x < pb.x ) return true;
        if ( pb.x < pa.x ) return false;
        if ( pa.y < pb.y ) return true;
        if ( pb.y < pa.y ) return false;
        return a < b;
    }

    private:

    const MATH::CTPoint2D< T >* m_points;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The convex hull of a point set by Andrew's monotone chain with the exact orientation predicate (section 7.9 of the
 *  design). 'hull' receives the indices of the hull points counter-clockwise, starting at the lowest point by x and
 *  then y. Duplicate points count once (the one with the lowest index). Collinear points on the edges are left out, or
 *  kept in the order along the edge with 'keepCollinear'. A set of one distinct point gives one index, a set whose points
 *  are all collinear gives its two end points (all points in order with 'keepCollinear'). Points that are not finite are
 *  skipped. Returns the number of hull points. Not realtime-safe (allocates the work arrays).
 */
template < typename T >
UInt32
ConvexHull2DIndices( const MATH::CTPoint2D< T >* points, UInt32 count, GUCEF::vector< UInt32 >& hull, bool keepCollinear = false )
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef CTPredicates< T > TPredicates;
    hull.clear();
    GUCEF::vector< UInt32 > order;
    order.reserve( count );
    for ( UInt32 i=0; i<count; ++i )
        if ( TTraits::IsFinite( points[ i ].x ) && TTraits::IsFinite( points[ i ].y ) )
            order.push_back( i );
    std::sort( order.begin(), order.end(), CTConvexHull2DLess< T >( points ) );

    // remove duplicates: the first of each run of equal points stays
    GUCEF::vector< UInt32 > sorted;
    sorted.reserve( order.size() );
    for ( size_t i=0; i<order.size(); ++i )
        if ( sorted.empty() || !( points[ sorted.back() ].x == points[ order[ i ] ].x && points[ sorted.back() ].y == points[ order[ i ] ].y ) )
            sorted.push_back( order[ i ] );
    const size_t n = sorted.size();
    if ( n <= 2 )
    {
        hull = sorted;
        return (UInt32) hull.size();
    }

    // all collinear: the monotone chain would walk the same line twice
    bool collinear = true;
    for ( size_t i=1; i+1<n && collinear; ++i )
        collinear = 0 == TPredicates::Orient2D( points[ sorted[ 0 ] ], points[ sorted[ n - 1 ] ], points[ sorted[ i ] ] );
    if ( collinear )
    {
        if ( keepCollinear )
            hull = sorted;
        else
        {
            hull.push_back( sorted[ 0 ] );
            hull.push_back( sorted[ n - 1 ] );
        }
        return (UInt32) hull.size();
    }

    // the strict chain pops on 'not a left turn', the collinear keeping chain only on a right turn
    GUCEF::vector< UInt32 > chain;
    chain.reserve( n + 1 );
    for ( size_t pass=0; pass<2; ++pass )
    {
        const size_t floor = chain.size();
        for ( size_t k=0; k<n; ++k )
        {
            const UInt32 index = 0 == pass ? sorted[ k ] : sorted[ n - 1 - k ];
            while ( chain.size() >= floor + 2 )
            {
                const Int32 turn = TPredicates::Orient2D( points[ chain[ chain.size() - 2 ] ], points[ chain[ chain.size() - 1 ] ], points[ index ] );
                if ( turn < 0 || ( !keepCollinear && 0 == turn ) )
                    chain.pop_back();
                else
                    break;
            }
            chain.push_back( index );
        }
        // the last point of each chain is the first of the next
        chain.pop_back();
    }
    hull = chain;
    return (UInt32) hull.size();
}

/*-------------------------------------------------------------------------*/

/** As ConvexHull2DIndices, with the hull points themselves */
template < typename T >
UInt32
ConvexHull2D( const MATH::CTPoint2D< T >* points, UInt32 count, GUCEF::vector< MATH::CTPoint2D< T > >& hull, bool keepCollinear = false )
{
    GUCEF::vector< UInt32 > indices;
    ConvexHull2DIndices( points, count, indices, keepCollinear );
    hull.clear();
    hull.reserve( indices.size() );
    for ( size_t i=0; i<indices.size(); ++i )
        hull.push_back( points[ indices[ i ] ] );
    return (UInt32) hull.size();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CONVEXHULL2D_H ? */
