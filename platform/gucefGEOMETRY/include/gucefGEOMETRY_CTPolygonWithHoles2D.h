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

#ifndef GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H
#define GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

#ifndef GUCEF_MATH_DIRECTIONS_H
#include "gucefMATH_directions.h"
#define GUCEF_MATH_DIRECTIONS_H
#endif /* GUCEF_MATH_DIRECTIONS_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A polygon region with holes in 2D: an outer ring and any number of hole rings (section 7.2).
 *
 *  - Kind: floats (Float32, Float64 are exported); needs a square root.
 *  - Layout: the outer ring and a GUCEF::vector of hole rings, both public (no invariant is enforced: the holes are
 *    expected inside the outer ring and not to overlap; the orientation of each ring is free, areas use the magnitude).
 *  - The region contains the points of the outer ring (boundary included) except the points strictly inside a hole;
 *    hole boundaries belong to the region.
 *  - Realtime: the const queries are realtime-safe; building, the wireframe, text and DataNode members are not.
 *  - Text: "PolygonWithHoles2D(outer Polygon2D(...), hole Polygon2D(...), ...)"; DataNode: child node outer and a child
 *    node holes with one child node per hole ring.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPolygonWithHoles2D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef CTPolygon2D< T > TRing;
    typedef GUCEF::vector< TRing > TRingVector;

    TRing outer;
    TRingVector holes;

    CTPolygonWithHoles2D( void ) : outer(), holes() {}

    explicit CTPolygonWithHoles2D( const TRing& outerRing ) : outer( outerRing ), holes() {}

    void AddHole( const TRing& hole ) { holes.push_back( hole ); }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetHoleCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return (UInt32) holes.size(); }

    /** The area of the outer ring minus the areas of the holes */
    T GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The sum of the boundary lengths of all rings */
    T GetPerimeter( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The area centroid of the region; the centroid of the outer ring when the region has no area */
    TPoint GetCentroid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** True when the point is in the region: inside or on the outer ring and not strictly inside a hole */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the boundary of the outer ring and the holes */
    TPoint GetClosestBoundaryPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the region: the point itself when inside, otherwise on a boundary */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    GUCEF_GEOMETRY_FORCE_INLINE T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    GUCEF_GEOMETRY_FORCE_INLINE T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    /** The bounds of the outer ring */
    GUCEF_GEOMETRY_FORCE_INLINE CTRectangle< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return outer.GetBounds(); }

    /** The support point of the outer ring */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return outer.GetSupport( direction ); }

    /** Appends the edges of every ring; 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const;

    friend bool operator==( const CTPolygonWithHoles2D& a, const CTPolygonWithHoles2D& b ) { return a.outer == b.outer && a.holes == b.holes; }
    friend bool operator!=( const CTPolygonWithHoles2D& a, const CTPolygonWithHoles2D& b ) { return !( a == b ); }

    bool Equals( const CTPolygonWithHoles2D& other ) const
    {
        if ( !outer.Equals( other.outer ) || holes.size() != other.holes.size() )
            return false;
        for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
        {
            if ( !holes[ i ].Equals( other.holes[ i ] ) )
                return false;
        }
        return true;
    }

    /** "PolygonWithHoles2D(outer Polygon2D(...), hole Polygon2D(...), ...)" */
    CORE::CString ToString( void ) const
    {
        CORE::CString text( "PolygonWithHoles2D(outer " );
        text += outer.ToString();
        for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
        {
            text += ", hole ";
            text += holes[ i ].ToString();
        }
        text += ')';
        return text;
    }

    /** Parses the ToString form; false leaves this polygon unchanged */
    bool FromString( const CORE::CString& text )
    {
        SShapeTextReader reader( text );
        TRing parsedOuter;
        TRingVector parsedHoles;
        if ( !reader.TryLiteral( "PolygonWithHoles2D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "outer" ) || !reader.TryShape( parsedOuter ) )
            return false;
        while ( reader.TryChar( ',' ) )
        {
            TRing hole;
            if ( !reader.TryLiteral( "hole" ) || !reader.TryShape( hole ) )
                return false;
            parsedHoles.push_back( hole );
        }
        if ( !reader.TryChar( ')' ) || !reader.AtEnd() )
            return false;
        outer = parsedOuter;
        holes.swap( parsedHoles );
        return true;
    }

    /** Writes the child node outer and the child node holes (one child node per hole ring) into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
    {
        if ( !SerializeShapeChild( node, "outer", outer, settings ) )
            return false;
        CORE::CDataNode* list = node.AddChild( CORE::CString( "holes" ), GUCEF_DATATYPE_ARRAY );
        if ( GUCEF_NULL == list )
            return false;
        for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
        {
            CORE::CDataNode* child = list->AddChild( CORE::CString( "hole" ), GUCEF_DATATYPE_OBJECT );
            if ( GUCEF_NULL == child || !holes[ i ].Serialize( *child, settings ) )
                return false;
        }
        return true;
    }

    /** Reads the child nodes outer and holes; a missing or invalid one fails and leaves this polygon unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
    {
        TRing parsedOuter;
        if ( !DeserializeShapeChild( node, "outer", parsedOuter, settings ) )
            return false;
        const CORE::CDataNode* list = node.FindChild( CORE::CString( "holes" ) );
        if ( GUCEF_NULL == list )
            return false;
        TRingVector parsedHoles;
        CORE::CDataNode::const_iterator i = list->ConstBegin();
        while ( i != list->ConstEnd() )
        {
            TRing hole;
            if ( !hole.Deserialize( **i, settings ) )
                return false;
            parsedHoles.push_back( hole );
            ++i;
        }
        outer = parsedOuter;
        holes.swap( parsedHoles );
        return true;
    }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTPolygonWithHoles2D< T >::GetArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T area = outer.GetArea();
    for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
        area = area - holes[ i ].GetArea();
    return area;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTPolygonWithHoles2D< T >::GetPerimeter( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T length = outer.GetPerimeter();
    for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
        length = length + holes[ i ].GetPerimeter();
    return length;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolygonWithHoles2D< T >::TPoint
CTPolygonWithHoles2D< T >::GetCentroid( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    /* the moments about the centroid of the outer ring: M = A0 * 0 - sum Ai * ( Ci - C0 ) */
    const TPoint outerCentroid = outer.GetCentroid();
    T area = outer.GetArea();
    TVector moment;
    for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
    {
        const T holeArea = holes[ i ].GetArea();
        moment = moment - ( holes[ i ].GetCentroid() - outerCentroid ) * holeArea;
        area = area - holeArea;
    }
    if ( !( TTraits::Zero() < area ) )
        return outerCentroid;
    return outerCentroid + moment / area;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTPolygonWithHoles2D< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( !outer.Contains( point ) )
        return false;
    for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
    {
        if ( holes[ i ].Contains( point ) && !holes[ i ].IsOnBoundary( point ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolygonWithHoles2D< T >::TPoint
CTPolygonWithHoles2D< T >::GetClosestBoundaryPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    TPoint best = outer.GetClosestBoundaryPoint( point );
    T bestDistance = ( point - best ).GetSquaredLength();
    for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
    {
        if ( holes[ i ].GetPointCount() < 1 )
            continue;
        TPoint candidate = holes[ i ].GetClosestBoundaryPoint( point );
        T distance = ( point - candidate ).GetSquaredLength();
        if ( distance < bestDistance )
        {
            best = candidate;
            bestDistance = distance;
        }
    }
    return best;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTPolygonWithHoles2D< T >::TPoint
CTPolygonWithHoles2D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( Contains( point ) )
        return point;
    return GetClosestBoundaryPoint( point );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTPolygonWithHoles2D< T >::GetWireframeLines( GUCEF::vector< CTSegment2D< T > >& lines, UInt32 segmentsPerCircle ) const
{
    outer.GetWireframeLines( lines, segmentsPerCircle );
    for ( UInt32 i=0; i<(UInt32) holes.size(); ++i )
        holes[ i ].GetWireframeLines( lines, segmentsPerCircle );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonWithHoles2D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonWithHoles2D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPOLYGONWITHHOLES2D_H ? */
