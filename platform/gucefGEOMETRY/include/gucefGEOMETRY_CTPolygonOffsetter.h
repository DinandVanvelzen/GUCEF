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

#ifndef GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H
#define GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#include "gucefGEOMETRY_CTPolygonClipper.h"
#define GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#endif /* GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H ? */

#ifndef GUCEF_GEOMETRY_CTPOLYLINE2D_H
#include "gucefGEOMETRY_CTPolyline2D.h"
#define GUCEF_GEOMETRY_CTPOLYLINE2D_H
#endif /* GUCEF_GEOMETRY_CTPOLYLINE2D_H ? */

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

/** How the offset edges meet at a vertex where they move apart */
enum EPolygonJoinType
{
    POLYGONJOIN_SQUARE  = 0 ,   /**< cut square at distance |delta| from the vertex, across the bisector */
    POLYGONJOIN_ROUND   = 1 ,   /**< an arc of radius |delta| (within the arc tolerance, its points on the arc) */
    POLYGONJOIN_MITER   = 2     /**< the edges extended to their intersection while it lies within miter limit x |delta| of the vertex, else cut straight (bevel) */
};
typedef enum EPolygonJoinType TPolygonJoinType;

/** How a stroked polyline ends */
enum EPolylineEndType
{
    POLYLINEEND_BUTT    = 0 ,   /**< square at the end point */
    POLYLINEEND_SQUARE  = 1 ,   /**< square, extended by |delta| beyond the end point */
    POLYLINEEND_ROUND   = 2     /**< a half circle of radius |delta| */
};
typedef enum EPolylineEndType TPolylineEndType;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Offsetting of closed polygons and stroking of polylines (section 7.9 of the design).
 *
 *  - Closed rings: every edge moves by delta along its right normal (outwards for a counter-clockwise ring, so a
 *    positive delta grows a polygon and shrinks its holes, a negative one shrinks it). Where the offset edges move
 *    apart the join closes the gap (square, round, miter); where they overlap the path runs through the original
 *    vertex, which makes a loop of negative winding. The union of all offset rings with the fill rule POSITIVE through
 *    CTPolygonClipper then removes every self-intersection and loop: for round joins the result is the set of points
 *    of the region farther than -delta from its boundary (delta < 0) or within delta of the region (delta > 0), up to
 *    the arc tolerance and the grid rounding. Features narrower than 2 |delta| vanish when shrinking.
 *  - Polylines: the outline at distance |delta| on both sides with the joins, closed by the end caps (butt, square,
 *    round), united with POSITIVE: a straight polyline of length L with round caps covers 2 |delta| L + pi delta^2.
 *    A single point gives a circle (round) or a square (square), nothing for butt ends.
 *  - Rings added by AddPolygon are made counter-clockwise; a CTPolygonWithHoles2D gets a counter-clockwise outer ring and
 *    clockwise holes. Rings and polylines with consecutive duplicate points are cleaned. A ring without area (fewer
 *    than three distinct points, or all its points on one line: exact test) is ignored for every delta, as the
 *    clipper treats it: it bounds no region. To stroke such a shape add it as a polyline.
 *  - Grid: the offset rings are built in Float64 on the integer grid of CTPolygonClipper (integers directly, fixed point
 *    by its raw value, floats scaled by a power of two chosen from the extent and delta (at most 2^1023), or
 *    SetGridScale) and rounded to it; the result rings follow the rules of CTPolygonClipper (outer counter-clockwise,
 *    holes clockwise). POLYGONCLIPPER_OUT_OF_RANGE (and no result) when the offset extent does not fit the grid or a
 *    result coordinate does not fit the kind (an Int32 result beyond the Int32 range, a Float32 result beyond its
 *    finite range).
 *  - Arc tolerance: the largest distance between a round join and its true arc (coordinate units); 0 (the default)
 *    means |delta| / 400, about 45 points per full circle. The miter limit (default 2, at least 1) is the largest ratio of
 *    the miter length to |delta|.
 *  - Kinds: Float32, Float64, Int32, Int64 and TFixed32_32 (the instantiations of the module).
 *  - Determinism: no C library function enters the geometry. The segment count of a round join or cap comes from the arc
 *    tolerance by a bisection on the correctly rounded MATH::SinCosOfTurnFraction, the arc points from a rotation
 *    recurrence with that step in a fixed operation order, and the end of every arc is the exact offset end normal; the
 *    rest is +, -, *, /, sqrt, floor and integer arithmetic. The result is therefore bit identical across compilers and
 *    platforms with IEEE 754 binary64 arithmetic and no floating point contraction (section 5.5 of the design).
 *  - Allocation: the object keeps its input, work arrays and clipper; a call allocates when the input or the result is
 *    larger than for an earlier call, for the output, and for the rings of the intermediate union result (the size
 *    of the output; see CTPolygonClipper). Not realtime-safe; one thread at a time; not copyable.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPolygonOffsetter
{
    public:

    typedef T TValue;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef GUCEF::vector< TPoint > TPath;
    typedef GUCEF::vector< TPath > TPaths;
    typedef CTPolygonWithHoles2D< T > TPolygon;
    typedef GUCEF::vector< TPolygon > TPolygons;

    CTPolygonOffsetter( void );

    ~CTPolygonOffsetter();

    /** The largest miter length as a multiple of |delta| (values below 1 count as 1) */
    void SetMiterLimit( Float64 limit );

    /** The largest distance of a round join from the true arc, in coordinate units; 0 for |delta| / 400 */
    void SetArcTolerance( Float64 tolerance );

    /** Float kinds: grid units per coordinate unit (a power of two); 0 (the default) chooses it per call */
    void SetGridScale( Float64 scale );

    /** The grid units per coordinate unit of the last Execute */
    Float64 GetGridScale( void ) const;

    /** Removes all polygons and polylines */
    void Clear( void );

    /** Adds a closed ring, made counter-clockwise */
    void AddPolygon( const TPoint* ring, UInt32 count );

    void AddPolygon( const TPath& ring );

    void AddPolygon( const CTPolygon2D< T >& polygon );

    /** Adds a polygon with holes: the outer ring counter-clockwise, the holes clockwise */
    void AddPolygon( const TPolygon& polygon );

    /** Adds an open polyline to stroke */
    void AddPolyline( const TPoint* points, UInt32 count );

    void AddPolyline( const TPath& points );

    void AddPolyline( const CTPolyline2D< T >& polyline );

    /** Offsets the polygons by 'delta' and strokes the polylines at distance |delta|; 'rings' receives the result */
    TPolygonClipperStatus Execute( const T& delta, TPolygonJoinType join, TPolylineEndType ends, TPaths& rings );

    /** As the ring form, as polygons with holes */
    TPolygonClipperStatus Execute( const T& delta, TPolygonJoinType join, TPolylineEndType ends, TPolygons& polygons );

    private:

    CTPolygonOffsetter( const CTPolygonOffsetter& src );                /**< not implemented */
    CTPolygonOffsetter& operator=( const CTPolygonOffsetter& src );     /**< not implemented */

    struct SInput
    {
        UInt32 first;
        UInt32 count;
        Int32 kind;         /**< +1 counter-clockwise ring, -1 clockwise ring, 0 polyline */
    };

    void AddInput( const TPoint* points, UInt32 count, Int32 kind );

    /** Builds the offset rings into the clipper; false when a value does not fit the grid */
    TPolygonClipperStatus Build( const T& delta, TPolygonJoinType join, TPolylineEndType ends );

    GUCEF::vector< TPoint > m_points;
    GUCEF::vector< SInput > m_inputs;
    GUCEF::vector< Float64 > m_work;
    GUCEF::vector< Float64 > m_ring;
    GUCEF::vector< Float64 > m_normals;
    GUCEF::vector< MATH::CTPoint2D< Int64 > > m_gridRing;
    GUCEF::vector< GUCEF::vector< MATH::CTPoint2D< Int64 > > > m_gridRings;
    GUCEF::vector< CTPolygonWithHoles2D< Int64 > > m_gridPolygons;
    CTPolygonClipper< Int64 >* m_clipper;
    Float64 m_miterLimit;
    Float64 m_arcTolerance;
    Float64 m_requestedScale;
    Float64 m_scale;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Offsets a polygon with holes by 'delta' (temporary offsetter; repeated calls should keep a CTPolygonOffsetter) */
template < typename T >
TPolygonClipperStatus
OffsetPolygon( const CTPolygonWithHoles2D< T >& polygon, const T& delta, TPolygonJoinType join, GUCEF::vector< CTPolygonWithHoles2D< T > >& result )
{
    CTPolygonOffsetter< T > offsetter;
    offsetter.AddPolygon( polygon );
    return offsetter.Execute( delta, join, POLYLINEEND_BUTT, result );
}

/*-------------------------------------------------------------------------*/

/** Strokes a polyline at distance |halfWidth| (temporary offsetter) */
template < typename T >
TPolygonClipperStatus
StrokePolyline( const CTPolyline2D< T >& polyline, const T& halfWidth, TPolygonJoinType join, TPolylineEndType ends, GUCEF::vector< CTPolygonWithHoles2D< T > >& result )
{
    CTPolygonOffsetter< T > offsetter;
    offsetter.AddPolyline( polyline );
    return offsetter.Execute( halfWidth, join, ends, result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTPolygonOffsetter< Float32 > TFloat32PolygonOffsetter;
typedef CTPolygonOffsetter< Float64 > TFloat64PolygonOffsetter;
typedef CTPolygonOffsetter< Int32 > TInt32PolygonOffsetter;
typedef CTPolygonOffsetter< Int64 > TInt64PolygonOffsetter;
typedef CTPolygonOffsetter< MATH::TFixed32_32 > TFixed32_32PolygonOffsetter;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonOffsetter< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonOffsetter< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonOffsetter< Int32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonOffsetter< Int64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonOffsetter< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPOLYGONOFFSETTER_H ? */
