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

#ifndef GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H
#define GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H

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

#ifndef GUCEF_MATH_FIXEDPOINT_H
#include "gucefMATH_fixedpoint.h"
#define GUCEF_MATH_FIXEDPOINT_H
#endif /* GUCEF_MATH_FIXEDPOINT_H ? */

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

/** The boolean operations of CTPolygonClipper: on the subject region S and the clip region C */
enum EPolygonBooleanOperation
{
    POLYGONBOOLEAN_UNION        = 0 ,   /**< S or C */
    POLYGONBOOLEAN_INTERSECTION = 1 ,   /**< S and C */
    POLYGONBOOLEAN_DIFFERENCE   = 2 ,   /**< S and not C */
    POLYGONBOOLEAN_XOR          = 3     /**< S or C but not both */
};
typedef enum EPolygonBooleanOperation TPolygonBooleanOperation;

/** Which points a set of rings covers, by their winding number w */
enum EPolygonFillRule
{
    POLYGONFILL_EVEN_ODD        = 0 ,   /**< w odd */
    POLYGONFILL_NON_ZERO        = 1 ,   /**< w != 0 */
    POLYGONFILL_POSITIVE        = 2 ,   /**< w > 0: counter-clockwise rings add, clockwise rings (holes) remove */
    POLYGONFILL_NEGATIVE        = 3     /**< w < 0 */
};
typedef enum EPolygonFillRule TPolygonFillRule;

/** The outcome of a polygon clipper or offsetter call */
enum EPolygonClipperStatus
{
    POLYGONCLIPPER_OK               = 0 ,
    POLYGONCLIPPER_INVALID_INPUT    = 1 ,   /**< a coordinate is not finite, or a parameter is invalid: no result */
    POLYGONCLIPPER_OUT_OF_RANGE     = 2     /**< a coordinate does not fit the integer grid (2^52 units): no result */
};
typedef enum EPolygonClipperStatus TPolygonClipperStatus;

/** The integer engine of the clipper (gucefGEOMETRY_CTPolygonClipper.cpp) */
struct SPolygonClipperEngine;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Boolean operations on polygons with holes (section 7.9 of the design): union, intersection, difference and xor of a
 *  subject and a clip region, each given as rings with a fill rule, robust for every input (touching and overlapping
 *  edges, shared vertices, holes, self-intersecting and self-touching rings).
 *
 *  - Integer grid: coordinates are mapped to Int64 grid points (|x|, |y| <= 2^52). Integer kinds are used directly,
 *    fixed point by its raw value; floats are scaled by a power of two (SetGridScale, or automatically so that the
 *    largest coordinate maps below 2^40, a relative precision of 2^-40, with the scale at most 2^1023 so that
 *    coordinates below about 2^-983 lose precision) and rounded to the nearest grid point; grid points map back
 *    exactly.
 *  - Algorithm (instead of the Vatti scan beam named in the design): snap rounding (Hobby; Guibas and Marimont) of the
 *    arrangement of all input edges: every input vertex and every crossing of two edges, rounded to its grid point,
 *    is a hot pixel (a half open unit square around a grid point); every edge is replaced by the chain of the centres of
 *    the hot pixels it passes through. Hot pixel tests, orientations and crossing roundings are exact (CInt128, and
 *    CBigInteger above 2^40). The snapped fragments meet only at their end points; coinciding fragments merge with
 *    their winding changes summed (so shared and overlapping edges need no special case). The winding numbers of both
 *    operands are propagated over the faces of the planar graph (a ray from the smallest vertex of each component fixes
 *    its outer face; one sweep over y answers all rays), the fill rules and the operation classify each face, and the
 *    output rings are the edges between faces of the result and faces outside it, linked with the region on their
 *    left.
 *  - Result: closed rings without repeated closing point: outer rings counter-clockwise, holes clockwise, never
 *    crossing; rings meet at most at vertices. At a vertex where the region touches itself the ring walk continues on
 *    the next boundary edge clockwise, so the touching parts come out as separate rings or as one ring that passes the
 *    vertex twice, depending on the configuration: two squares that touch at a corner give two rings, a square minus
 *    a notch that touches its boundary at a vertex gives one ring through that vertex twice. A ring may therefore
 *    repeat a vertex (never an edge).
 *    Straight-on vertices are removed (except where another ring touches) unless SetKeepCollinear( true ). All vertices
 *    are grid points, so on the grid the results of the four operations on the same operands are exactly consistent:
 *    area( S | C ) = area( S - C ) + area( C - S ) + area( S & C ) and area( S ^ C ) = area( S | C ) - area( S & C )
 *    hold exactly; they equal the input areas exactly when every crossing of the inputs is a grid point (otherwise the
 *    rounding moves each edge by at most half a grid unit at a crossing).
 *  - Kinds: Float32, Float64, Int32, Int64 and TFixed32_32 (the instantiations of the module; the members are defined
 *    in gucefGEOMETRY_CTPolygonClipper.cpp).
 *  - Cost: O( ( n + k ) log( n + k ) ) for n edges and k crossings in typical inputs: the edge pairs are found by a
 *    sweep over x with the active edges kept per band of y, the hot pixels of an edge by a search per column of
 *    pixels, the windings of the components by one sweep over y with a balanced tree, the holes of the polygon form
 *    by the faces of the result. Inputs whose edges are long compared with the spacing of the others (many edges
 *    spanning most bands) approach O( n^2 ) in the pair search, as every sweep without a y order does.
 *  - Allocation: the object keeps its input and work arrays and reuses them; Execute allocates when the input or the
 *    result is larger than for an earlier call, and for the output (each ring, and each polygon of the polygon form).
 *    Not realtime-safe; one thread at a time; not copyable.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTPolygonClipper
{
    public:

    typedef T TValue;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef GUCEF::vector< TPoint > TPath;
    typedef GUCEF::vector< TPath > TPaths;
    typedef CTPolygonWithHoles2D< T > TPolygon;
    typedef GUCEF::vector< TPolygon > TPolygons;

    CTPolygonClipper( void );

    ~CTPolygonClipper();

    /** Float kinds: grid units per coordinate unit (rounded down to a power of two); 0 (the default) chooses it per call */
    void SetGridScale( Float64 scale );

    /** The grid units per coordinate unit of the last Execute (1 for integers, 2^32 for TFixed32_32) */
    Float64 GetGridScale( void ) const;

    /** Keeps straight-on vertices in the output (false by default) */
    void SetKeepCollinear( bool keep );

    /** Removes all subject and clip rings */
    void Clear( void );

    /** Adds a subject ring as given (its orientation counts for the fill rule) */
    void AddSubject( const TPoint* ring, UInt32 count );

    void AddSubject( const TPath& ring );

    /** Adds the ring of a polygon as given */
    void AddSubject( const CTPolygon2D< T >& polygon );

    /** Adds a polygon with holes: the outer ring counter-clockwise and the holes clockwise, whatever their input orientation (use POLYGONFILL_POSITIVE or NON_ZERO) */
    void AddSubject( const TPolygon& polygon );

    void AddClip( const TPoint* ring, UInt32 count );

    void AddClip( const TPath& ring );

    void AddClip( const CTPolygon2D< T >& polygon );

    void AddClip( const TPolygon& polygon );

    /** Computes the operation; 'rings' receives the result rings (outer counter-clockwise, holes clockwise) */
    TPolygonClipperStatus Execute( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill, TPaths& rings );

    /** As the ring form, with every hole assigned to the smallest outer ring around it */
    TPolygonClipperStatus Execute( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill, TPolygons& polygons );

    private:

    CTPolygonClipper( const CTPolygonClipper& src );                /**< not implemented */
    CTPolygonClipper& operator=( const CTPolygonClipper& src );     /**< not implemented */

    /** 'orientation' 0 keeps the ring as given, +1 makes it counter-clockwise, -1 clockwise */
    void AddRing( const TPoint* ring, UInt32 count, UInt32 operand, Int32 orientation );

    TPolygonClipperStatus Run( TPolygonBooleanOperation operation, TPolygonFillRule subjectFill, TPolygonFillRule clipFill );

    struct SRing
    {
        UInt32 first;
        UInt32 count;
        UInt32 operand;
        Int32 orientation;
    };

    GUCEF::vector< TPoint > m_points;
    GUCEF::vector< SRing > m_rings;
    SPolygonClipperEngine* m_engine;
    Float64 m_requestedScale;
    Float64 m_scale;
    bool m_keepCollinear;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One boolean operation on two polygons with holes (fill rule POSITIVE on the normalised rings); the result as
 *  polygons with holes. Uses a temporary clipper; repeated calls should keep a CTPolygonClipper.
 */
template < typename T >
TPolygonClipperStatus
PolygonBoolean( const CTPolygonWithHoles2D< T >& subject, const CTPolygonWithHoles2D< T >& clip, TPolygonBooleanOperation operation, GUCEF::vector< CTPolygonWithHoles2D< T > >& result )
{
    CTPolygonClipper< T > clipper;
    clipper.AddSubject( subject );
    clipper.AddClip( clip );
    return clipper.Execute( operation, POLYGONFILL_POSITIVE, POLYGONFILL_POSITIVE, result );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTPolygonClipper< Float32 > TFloat32PolygonClipper;
typedef CTPolygonClipper< Float64 > TFloat64PolygonClipper;
typedef CTPolygonClipper< Int32 > TInt32PolygonClipper;
typedef CTPolygonClipper< Int64 > TInt64PolygonClipper;
typedef CTPolygonClipper< MATH::TFixed32_32 > TFixed32_32PolygonClipper;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonClipper< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonClipper< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonClipper< Int32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonClipper< Int64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTPolygonClipper< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTPOLYGONCLIPPER_H ? */
