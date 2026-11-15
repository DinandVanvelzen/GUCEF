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

#ifndef GUCEF_GEOMETRY_POINTINPOLYGON_H
#define GUCEF_GEOMETRY_POINTINPOLYGON_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTPREDICATES_H
#include "gucefGEOMETRY_CTPredicates.h"
#define GUCEF_GEOMETRY_CTPREDICATES_H
#endif /* GUCEF_GEOMETRY_CTPREDICATES_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGON2D_H
#include "gucefGEOMETRY_CTPolygon2D.h"
#define GUCEF_GEOMETRY_CTPOLYGON2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGON2D_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#include "gucefGEOMETRY_CTPolygonWithHoles2D.h"
#define GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H ? */

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

/** The position of a point relative to a polygon */
enum EPointInPolygon
{
    POINTINPOLYGON_OUTSIDE      = 0 ,
    POINTINPOLYGON_INSIDE       = 1 ,       /**< strictly inside: not on an edge or a vertex */
    POINTINPOLYGON_ON_BOUNDARY  = 2         /**< on an edge or a vertex of the outer ring or of a hole */
};
typedef enum EPointInPolygon TPointInPolygon;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  Point in polygon (section 7.9 of the design): the winding number of Sunday with the exact orientation predicate of
 *  CTPredicates, so the answer is exact for every kind: a point on an edge or a vertex is ON_BOUNDARY whatever the
 *  rounding of the kind, a point left of an upward edge or right of a downward edge that crosses its height changes the
 *  winding number. The rings may be clockwise or counter-clockwise, concave, or self-intersecting (a non zero winding
 *  number is inside, as in CTPolygon2D::Contains). A point or ring point that is not finite is OUTSIDE. Linear in the
 *  number of ring points. Not annotated as realtime-safe: the integer and fixed-point kinds allocate nothing, but the exact stage
 *  of the Float32 and Float64 predicates (near-degenerate input) allocates (see CTPredicates).
 */

/** The position of 'point' relative to the closed ring of 'count' points (a ring of fewer than three points has no inside: a point is ON_BOUNDARY only when it lies on it) */
template < typename T >
TPointInPolygon
ClassifyPointInRing( const MATH::CTPoint2D< T >* ring, UInt32 count, const MATH::CTPoint2D< T >& point ) 
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef CTPredicates< T > TPredicates;
    if ( 0 == count || !TTraits::IsFinite( point.x ) || !TTraits::IsFinite( point.y ) )
        return POINTINPOLYGON_OUTSIDE;
    Int32 winding = 0;
    UInt32 previous = count - 1;
    for ( UInt32 i=0; i<count; previous=i++ )
    {
        const MATH::CTPoint2D< T >& a = ring[ previous ];
        const MATH::CTPoint2D< T >& b = ring[ i ];
        if ( !TTraits::IsFinite( a.x ) || !TTraits::IsFinite( a.y ) || !TTraits::IsFinite( b.x ) || !TTraits::IsFinite( b.y ) )
            return POINTINPOLYGON_OUTSIDE;
        // the edge test also covers the degenerate edge a == b: it contains the point only when the point is that vertex
        const bool inBoxX = a.x < b.x ? ( !( point.x < a.x ) && !( b.x < point.x ) ) : ( !( point.x < b.x ) && !( a.x < point.x ) );
        const bool inBoxY = a.y < b.y ? ( !( point.y < a.y ) && !( b.y < point.y ) ) : ( !( point.y < b.y ) && !( a.y < point.y ) );
        const Int32 side = TPredicates::Orient2D( a, b, point );
        if ( 0 == side && inBoxX && inBoxY )
            return POINTINPOLYGON_ON_BOUNDARY;
        if ( !( point.y < a.y ) )
        {
            // an upward crossing with the point to the left of the edge
            if ( b.y > point.y && side > 0 )
                ++winding;
        }
        else if ( !( b.y > point.y ) && side < 0 )
        {
            // a downward crossing with the point to the right of the edge
            --winding;
        }
    }
    return 0 != winding ? POINTINPOLYGON_INSIDE : POINTINPOLYGON_OUTSIDE;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE TPointInPolygon
ClassifyPointInPolygon( const CTPolygon2D< T >& polygon, const MATH::CTPoint2D< T >& point ) 
{
    return ClassifyPointInRing( polygon.points.empty() ? static_cast< const MATH::CTPoint2D< T >* >( GUCEF_NULL ) : &polygon.points[ 0 ], (UInt32) polygon.points.size(), point );
}

/*-------------------------------------------------------------------------*/

/** The outer ring minus the holes: a point on the boundary of the outer ring or of a hole is ON_BOUNDARY, one strictly inside a hole is OUTSIDE */
template < typename T >
TPointInPolygon
ClassifyPointInPolygon( const CTPolygonWithHoles2D< T >& polygon, const MATH::CTPoint2D< T >& point ) 
{
    const TPointInPolygon outer = ClassifyPointInPolygon( polygon.outer, point );
    if ( POINTINPOLYGON_INSIDE != outer )
        return outer;
    for ( size_t i=0; i<polygon.holes.size(); ++i )
    {
        const TPointInPolygon hole = ClassifyPointInPolygon( polygon.holes[ i ], point );
        if ( POINTINPOLYGON_ON_BOUNDARY == hole )
            return POINTINPOLYGON_ON_BOUNDARY;
        if ( POINTINPOLYGON_INSIDE == hole )
            return POINTINPOLYGON_OUTSIDE;
    }
    return POINTINPOLYGON_INSIDE;
}

/*-------------------------------------------------------------------------*/

/** True when the point is inside or on the boundary */
template < typename T, typename TPolygon >
GUCEF_GEOMETRY_FORCE_INLINE bool
IsPointInPolygon( const TPolygon& polygon, const MATH::CTPoint2D< T >& point ) 
{
    return POINTINPOLYGON_OUTSIDE != ClassifyPointInPolygon( polygon, point );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_POINTINPOLYGON_H ? */
