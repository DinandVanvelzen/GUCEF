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

#ifndef GUCEF_GEOMETRY_GEOOBJECTQUERIES_H
#define GUCEF_GEOMETRY_GEOOBJECTQUERIES_H

/*
 *  Queries between linked objects (section 7.4.4 of the design): the facade functions Distance, ClosestPoints (world
 *  coordinates), Intersects, ComputeContact and Contains on CTGeoObjectBase references, whatever spaces the objects
 *  live in. Point and ray queries need nothing here: the objects have the uniform world members (section 2.1), so
 *  Distance( point, object ), Intersects( ray, object ), RayCast( ray, object, ... ) work through the value facade.
 *
 *  How a pair is answered, in this order:
 *  1. Analytic: when one object's world matrix is a similarity and the other's shape has an image in that object's
 *     space under the relative transform (CTShapeTraits::TryImage: always for triangles, for similarities for spheres,
 *     capsules, cylinders and cones, while the axes stay orthogonal for boxes), the value query of section 7.5 runs in
 *     that space and the results are mapped to world. The pair dispatch table CTGeoPairDispatch, indexed by the two
 *     shape types, holds these per-pair functions: a constant-initialised array of function pointers (template
 *     instantiations; no RTTI, no dynamic initialisation).
 *  2. Support mappings: for any other pair of bounded convex objects (non-uniform scale or shear, hulls, plane-bounded
 *     volumes, convex 2D shapes, bounded planes, user shapes) GJK and EPA run on the placed support mappings
 *     support_world( d ) = M * support_local( M^T d ), exact for any affine placement, in world coordinates relative
 *     to the first object's origin.
 *  3. An unbounded plane against a bounded convex object: the support range along the world plane normal.
 *  Contact of two coplanar flat objects (2D shapes or bounded planes in one plane) is answered in their plane: both
 *  are extruded along the normal by much more than they can overlap, so EPA finds the in-plane penetration; the
 *  points are projected back into the plane. A pair that does not touch gives an empty manifold (ClosestPoints gives
 *  the separation).
 *  4. Non-convex objects that have convex pieces (CTGeoObjectBase::GetConvexPieceCount: a non-convex polygon or polygon
 *     with holes answers through the triangles of its cached triangulation): the pair is answered over every pair of
 *     pieces (a convex object is its own single piece), each piece a flat convex support mapping run through GJK / EPA
 *     as in item 2: ClosestPoints and Distance take the minimum (0 and the points of the first overlapping pair when any
 *     pair touches), Intersects is true when any pair touches, ComputeContact returns the contact of the deepest pair
 *     (an empty manifold when no pair touches). The path is GEOPAIRPATH_SUPPORT; an unbounded plane against such an
 *     object takes the minimum over its pieces (GEOPAIRPATH_PLANE). Contact with an unbounded plane is not offered.
 *     A convex pair is never answered over pieces. Contains( outer, inner ) with an outer box, oriented box or
 *     plane-bounded volume accepts such an inner object too (a convex outer contains a region exactly when it contains
 *     its convex hull, whose support the object gives); a non-convex object as the outer object is not offered.
 *  5. Anything else (empty shapes, polylines, a non-convex object without pieces, two unbounded planes, contact with an
 *     unbounded plane) is not offered: the documented response false (Distance: the largest finite number of the kind; ComputeContact: an empty
 *     manifold; the path GEOPAIRPATH_NOT_OFFERED), never an assertion. IsGeoPairQueryOffered tells beforehand.
 *  Contains( outer, inner ): an axis aligned box, oriented box or plane-bounded volume contains a bounded convex object
 *  when its planes contain the inner support mapping placed in its space (exact for any affine placement); a sphere
 *  contains a sphere, axis aligned box or oriented box when that inner shape has an image in the sphere's space (the
 *  same test decides IsGeoPairQueryOffered); other outers are not offered.
 *  The dispatch uses CTGeoObjectBase::GetDispatchShapeType (the library tag), never a user's GetShapeType.
 *
 *  Float32 and Float64 are exported instantiations. Owner-thread only (section 7.3.7); allocation-free except for the
 *  first query that builds the triangle cache of a non-convex polygon.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOOBJECT_H
#include "gucefGEOMETRY_CTGeoObject.h"
#define GUCEF_GEOMETRY_CTGEOOBJECT_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECT_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOOBJECT2D_H
#include "gucefGEOMETRY_CTGeoObject2D.h"
#define GUCEF_GEOMETRY_CTGEOOBJECT2D_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECT2D_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOPLANE_H
#include "gucefGEOMETRY_CTGeoPlane.h"
#define GUCEF_GEOMETRY_CTGEOPLANE_H
#endif /* GUCEF_GEOMETRY_CTGEOPLANE_H ? */

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

/** The kinds of linked pair queries (IsGeoPairQueryOffered) */
enum EGeoPairQuery
{
    GEOPAIRQUERY_CLOSESTPOINTS = 0 ,    /**< ClosestPoints and Distance */
    GEOPAIRQUERY_INTERSECTS        ,
    GEOPAIRQUERY_CONTACT           ,
    GEOPAIRQUERY_CONTAINS
};
typedef enum EGeoPairQuery TGeoPairQuery;

/** The way a linked pair query was answered (see the header comment) */
enum EGeoPairPath
{
    GEOPAIRPATH_NOT_OFFERED = 0 ,
    GEOPAIRPATH_ANALYTIC        ,       /**< the value query in the space of one of the objects */
    GEOPAIRPATH_SUPPORT         ,       /**< GJK / EPA on the placed support mappings */
    GEOPAIRPATH_PLANE           ,       /**< an unbounded plane against a convex object */
    GEOPAIRPATH_PLANE_SET               /**< containment by the planes of the outer object */
};
typedef enum EGeoPairPath TGeoPairPath;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The value shape class of a shape type id that has an analytic image path (Valid = 1), for the dispatch table */
template < typename T, int ShapeTypeId >
struct CTGeoImageShapeOf { enum { Valid = 0 }; typedef CTSphere< T > TShape; };

template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_SPHERE > { enum { Valid = 1 }; typedef CTSphere< T > TShape; };
template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_AXIS_ALIGNED_BOX > { enum { Valid = 1 }; typedef CTAxisAlignedBox3D< T > TShape; };
template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_TRIANGLE > { enum { Valid = 1 }; typedef CTTriangle3D< T > TShape; };
template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_CAPSULE > { enum { Valid = 1 }; typedef CTCapsule< T > TShape; };
template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_ORIENTED_BOX > { enum { Valid = 1 }; typedef CTOrientedBox3D< T > TShape; };
template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_CYLINDER > { enum { Valid = 1 }; typedef CTCylinder< T > TShape; };
template < typename T > struct CTGeoImageShapeOf< T, GEOSHAPE_CONE > { enum { Valid = 1 }; typedef CTCone< T > TShape; };

/*-------------------------------------------------------------------------*/

/**
 *  The analytic path of one pair of shape classes: shape B is mapped into the space of object A (whose world matrix
 *  must be a similarity), the value query runs there and the results are mapped to world. Every function returns -1
 *  when the path does not apply (then the caller tries the next one), else 0 or 1 as the answer.
 */
template < typename TShapeA, typename TShapeB >
class CTGeoObjectImagePair
{
    public:

    typedef typename TShapeA::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTMatrix4D< T > TMatrix;
    typedef CTGeoObjectBase< T > TObject;
    typedef CTGeoObject< TShapeA > TObjectA;
    typedef CTGeoObject< TShapeB > TObjectB;
    typedef CTShapeTraits< TShapeB > TTraitsB;
    typedef typename TTraitsB::TImage TImageB;

    static int ClosestPoints( const TObject& a, const TObject& b, TPoint& onA, TPoint& onB, T& distance )
    {
        TImageB image;
        T scale = TTraits::One();
        if ( !MakeImage( a, b, image, scale ) )
            return -1;
        TPoint pa;
        TPoint pb;
        if ( !GEOMETRY::ClosestPoints( static_cast< const TObjectA& >( a ).GetShape(), image, pa, pb ) )
            return 0;
        const TMatrix& world = a.GetWorldMatrix();
        onA = world.TransformPoint( pa );
        onB = world.TransformPoint( pb );
        distance = ( pb - pa ).GetLength() * scale;
        return 1;
    }

    static int Intersects( const TObject& a, const TObject& b )
    {
        TImageB image;
        T scale = TTraits::One();
        if ( !MakeImage( a, b, image, scale ) )
            return -1;
        return GEOMETRY::Intersects( static_cast< const TObjectA& >( a ).GetShape(), image ) ? 1 : 0;
    }

    static int ComputeContact( const TObject& a, const TObject& b, CTContactManifold< T >& manifold )
    {
        TImageB image;
        T scale = TTraits::One();
        if ( !MakeImage( a, b, image, scale ) )
            return -1;
        CTContactManifold< T > local;
        const bool touching = GEOMETRY::ComputeContact( static_cast< const TObjectA& >( a ).GetShape(), image, local );
        const TMatrix& world = a.GetWorldMatrix();
        manifold.Clear();
        MATH::CTDirectionVector3D< T > normal;
        if ( MATH::CTDirectionVector3D< T >::TryCreate( world.TransformVector( local.normal ), normal ) )
            manifold.normal = normal.AsVector();
        for ( UInt32 i=0; i<local.GetPointCount(); ++i )
        {
            const SContactPoint< T >& p = local.GetPoint( i );
            manifold.AddPoint( world.TransformPoint( p.pointOnA ), world.TransformPoint( p.pointOnB ), p.depth * scale, p.featureId );
        }
        return touching ? 1 : 0;
    }

    private:

    /** B's shape in A's space; false when A's world matrix is not a similarity or B leaves its family under the map */
    static bool MakeImage( const TObject& a, const TObject& b, TImageB& image, T& scale )
    {
        if ( !a.IsWorldTransformSimilarity() )
            return false;
        scale = a.GetWorldMatrix().GetLinear().GetColumn( 0 ).GetLength();
        return TTraitsB::TryImage( static_cast< const TObjectB& >( b ).GetShape(), b.GetTransformTo( a ), image );
    }
};

/*-------------------------------------------------------------------------*/

/** The functions of one cell of the pair dispatch table */
template < typename T >
struct SGeoPairCell
{
    typedef int ( *TClosestPoints )( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB, T& distance );
    typedef int ( *TIntersects )( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b );
    typedef int ( *TComputeContact )( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b, CTContactManifold< T >& manifold );

    TClosestPoints closestPoints;
    TIntersects intersects;
    TComputeContact computeContact;
};

/*-------------------------------------------------------------------------*/

/** The cell functions of pairs without an analytic path: they always return -1 (one shared set per kind) */
template < typename T >
class CTGeoPairNoImage
{
    public:

    static int ClosestPoints( const CTGeoObjectBase< T >&, const CTGeoObjectBase< T >&, MATH::CTPoint3D< T >&, MATH::CTPoint3D< T >&, T& ) { return -1; }
    static int Intersects( const CTGeoObjectBase< T >&, const CTGeoObjectBase< T >& ) { return -1; }
    static int ComputeContact( const CTGeoObjectBase< T >&, const CTGeoObjectBase< T >&, CTContactManifold< T >& ) { return -1; }
};

/** The cell functions of the shape type ids A and B: the analytic pair when both have an image path */
template < typename T, int A, int B, bool HasImages = ( 0 != CTGeoImageShapeOf< T, A >::Valid && 0 != CTGeoImageShapeOf< T, B >::Valid ) >
class CTGeoPairCellFunctions : public CTGeoPairNoImage< T >
{
};

template < typename T, int A, int B >
class CTGeoPairCellFunctions< T, A, B, true > : public CTGeoObjectImagePair< typename CTGeoImageShapeOf< T, A >::TShape, typename CTGeoImageShapeOf< T, B >::TShape >
{
};

/*-------------------------------------------------------------------------*/

/**
 *  The pair dispatch table of section 7.4.4: one cell per ordered pair of shape types (TGeoShapeType), constant
 *  initialised with the addresses of the cell functions. Cells of pairs without an analytic path share the functions
 *  of CTGeoPairNoImage.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoPairDispatch
{
    public:

    /** The cell of the ordered pair; out-of-range types give the cell of ( GEOSHAPE_UNKNOWN, GEOSHAPE_UNKNOWN ) */
    static const SGeoPairCell< T >& GetCell( TGeoShapeType a, TGeoShapeType b )
    {
        const UInt32 ia = (UInt32) a < (UInt32) GEOSHAPE_COUNT ? (UInt32) a : 0;
        const UInt32 ib = (UInt32) b < (UInt32) GEOSHAPE_COUNT ? (UInt32) b : 0;
        return s_cells[ ia ][ ib ];
    }

    private:

    GUCEF_GEOMETRY_STATIC_ASSERT( 23 == GEOSHAPE_COUNT, the_pair_dispatch_table_rows_must_list_every_shape_type );

    static const SGeoPairCell< T > s_cells[ GEOSHAPE_COUNT ][ GEOSHAPE_COUNT ];
};

#define GUCEF_GEOMETRY_GEOPAIR_CELL( A, B ) \
    { &CTGeoPairCellFunctions< T, A, B >::ClosestPoints, &CTGeoPairCellFunctions< T, A, B >::Intersects, &CTGeoPairCellFunctions< T, A, B >::ComputeContact }

#define GUCEF_GEOMETRY_GEOPAIR_ROW( A ) \
    { GUCEF_GEOMETRY_GEOPAIR_CELL( A, 0 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 1 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 2 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 3 ), \
      GUCEF_GEOMETRY_GEOPAIR_CELL( A, 4 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 5 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 6 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 7 ), \
      GUCEF_GEOMETRY_GEOPAIR_CELL( A, 8 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 9 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 10 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 11 ), \
      GUCEF_GEOMETRY_GEOPAIR_CELL( A, 12 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 13 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 14 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 15 ), \
      GUCEF_GEOMETRY_GEOPAIR_CELL( A, 16 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 17 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 18 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 19 ), \
      GUCEF_GEOMETRY_GEOPAIR_CELL( A, 20 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 21 ), GUCEF_GEOMETRY_GEOPAIR_CELL( A, 22 ) }

template < typename T >
const SGeoPairCell< T > CTGeoPairDispatch< T >::s_cells[ GEOSHAPE_COUNT ][ GEOSHAPE_COUNT ] =
{
    GUCEF_GEOMETRY_GEOPAIR_ROW( 0 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 1 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 2 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 3 ),
    GUCEF_GEOMETRY_GEOPAIR_ROW( 4 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 5 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 6 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 7 ),
    GUCEF_GEOMETRY_GEOPAIR_ROW( 8 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 9 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 10 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 11 ),
    GUCEF_GEOMETRY_GEOPAIR_ROW( 12 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 13 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 14 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 15 ),
    GUCEF_GEOMETRY_GEOPAIR_ROW( 16 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 17 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 18 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 19 ),
    GUCEF_GEOMETRY_GEOPAIR_ROW( 20 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 21 ), GUCEF_GEOMETRY_GEOPAIR_ROW( 22 )
};

#undef GUCEF_GEOMETRY_GEOPAIR_ROW
#undef GUCEF_GEOMETRY_GEOPAIR_CELL

/*-------------------------------------------------------------------------*/

/**
 *  A flat support mapping extruded along its plane normal by +-halfThickness: the in-plane contact of coplanar flat
 *  shapes through EPA (see the header comment)
 */
template < typename TSupport >
class CTGeoFlatPrismSupport
{
    public:

    typedef typename TSupport::TValue TValue;
    typedef MATH::CTPoint3D< TValue > TPoint;
    typedef MATH::CTVector3D< TValue > TVector;

    TSupport inner;
    TVector normal;
    TValue halfThickness;

    CTGeoFlatPrismSupport( const TSupport& support, const TVector& unitNormal, const TValue& half ) : inner( support ), normal( unitNormal ), halfThickness( half ) {}

    TPoint GetSupport( const TVector& direction ) const
    {
        const TPoint p = inner.GetSupport( direction );
        return MATH::CTNumericTraits< TValue >::Zero() < direction.Dot( normal ) ? p + normal * halfThickness : p - normal * halfThickness;
    }
    TValue GetMargin( void ) const { return inner.GetMargin(); }
    TPoint GetAnyPoint( void ) const { return inner.GetAnyPoint(); }
};

/*-------------------------------------------------------------------------*/

/**
 *  The linked pair queries (see the header comment); the free functions below forward to these. The forms with a
 *  TGeoPairPath report how the answer was obtained.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoObjectQueries
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTMatrix4D< T > TMatrix;
    typedef CTGeoObjectBase< T > TObject;
    typedef CTContactManifold< T > TManifold;

    /** True when the query is offered for the pair as the objects are now (convexity and extents are run-time properties) */
    static bool IsOffered( const TObject& a, const TObject& b, TGeoPairQuery query );

    /** The nearest points in world coordinates and their world distance; false when not offered or a shape has no points */
    static bool ClosestPoints( const TObject& a, const TObject& b, TPoint& onA, TPoint& onB, T& distance, TGeoPairPath& path );

    /** The world distance, 0 when touching or overlapping; the largest finite number of the kind when not offered */
    static T Distance( const TObject& a, const TObject& b );

    static bool Intersects( const TObject& a, const TObject& b, TGeoPairPath& path );

    /** The contact in world coordinates (normal from a towards b); true when touching or overlapping */
    static bool ComputeContact( const TObject& a, const TObject& b, TManifold& manifold, TGeoPairPath& path );

    /** True when 'inner' lies entirely in 'outer' */
    static bool Contains( const TObject& outer, const TObject& inner, TGeoPairPath& path );

    private:

    static bool IsSupportShape( const TObject& o ) { return !o.IsEmpty() && o.IsConvex() && o.IsBounded(); }

    /** A bounded non-convex object that is answered over its convex pieces (builds the piece cache on first use) */
    static bool IsPieceShape( const TObject& o ) { return !o.IsEmpty() && !o.IsConvex() && o.IsBounded() && o.GetConvexPieceCount() > 0; }

    /** A support shape or a piece shape: the objects of the piece-wise pair queries */
    static bool IsPairShape( const TObject& o ) { return IsSupportShape( o ) || IsPieceShape( o ); }

    /** The number of convex pieces: 1 for a support shape (its whole self), the piece count otherwise */
    static UInt32 PieceCount( const TObject& o ) { return IsSupportShape( o ) ? 1 : o.GetConvexPieceCount(); }

    /** The placed support of piece 'piece' */
    static typename TObject::TPlacedSupport PieceSupport( const TObject& o, UInt32 piece, const TVector& origin )
    {
        return IsSupportShape( o ) ? o.GetPlacedSupport( origin ) : o.GetPlacedSupport( origin, piece );
    }

    /** The queries over the pieces of a pair of which at least one is not convex (both satisfy IsPairShape) */
    static bool PieceClosestPoints( const TObject& a, const TObject& b, TPoint& onA, TPoint& onB, T& distance );
    static bool PieceIntersects( const TObject& a, const TObject& b );
    static bool PieceContact( const TObject& a, const TObject& b, bool coplanar, const TVector& planeNormal, TManifold& manifold );
    static bool IsUnboundedPlane( const TObject& o ) { return GEOSHAPE_PLANE == o.GetDispatchShapeType() && !o.IsBounded(); }
    static void NotOffered( TGeoPairPath& path );

    /** The sphere containment cell: false when the inner shape has no image in the sphere's space ('imaged' false) */
    static bool SphereContains( const TObject& outer, const TObject& inner, bool& imaged );

    /** True when both are flat and lie in one plane (within the tolerance of the kind) */
    static bool AreCoplanarFlat( const TObject& a, const TObject& b, TVector& normal );

    /** The in-plane contact of coplanar flat objects */
    static void CoplanarContact( const TObject& a, const TObject& b, const TVector& normal, TManifold& manifold );

    /** Back into the plane: the normal in the plane, the points on it (points relative to 'origin', which is added back) */
    static void ProjectCoplanarManifold( const TVector& normal, const TVector& origin, TManifold& manifold );
    static CTPlane< T > ShiftedPlane( const TObject& plane, const TVector& origin );
    static void PlaneClosestPoints( const TObject& plane, const TObject& other, TPoint& onPlane, TPoint& onOther, T& distance );
};

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectQueries< T >::IsOffered( const TObject& a, const TObject& b, TGeoPairQuery query )
{
    switch ( query )
    {
        case GEOPAIRQUERY_CONTACT:
            return IsPairShape( a ) && IsPairShape( b );
        case GEOPAIRQUERY_CONTAINS:
        {
            if ( a.IsEmpty() || b.IsEmpty() )
                return false;
            const TGeoShapeType outer = a.GetDispatchShapeType();
            if ( GEOSHAPE_AXIS_ALIGNED_BOX == outer || GEOSHAPE_ORIENTED_BOX == outer || GEOSHAPE_PLANE_BOUNDED_VOLUME == outer )
                return IsPairShape( b );
            if ( GEOSHAPE_SPHERE == outer )
            {
                bool imaged = false;
                SphereContains( a, b, imaged );
                return imaged;
            }
            return false;
        }
        default:
            return ( IsPairShape( a ) && IsPairShape( b ) ) || ( IsUnboundedPlane( a ) && IsPairShape( b ) ) || ( IsPairShape( a ) && IsUnboundedPlane( b ) );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoObjectQueries< T >::NotOffered( TGeoPairPath& path )
{
    /*
     *  a pair without an algorithm (section 7.4.4 item 4): a defined result, not an assertion (false, Distance the
     *  largest finite number of the kind, an empty manifold, the path GEOPAIRPATH_NOT_OFFERED), so that callers that
     *  iterate mixed object sets need no pre-check; IsGeoPairQueryOffered tells beforehand and always agrees
     */
    path = GEOPAIRPATH_NOT_OFFERED;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTPlane< T >
CTGeoObjectQueries< T >::ShiftedPlane( const TObject& plane, const TVector& origin )
{
    const CTPlane< T > world = static_cast< const CTGeoPlane< T >& >( plane ).GetPlane();
    return CTPlane< T >( world.normal, world.offset - world.normal.Dot( origin ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoObjectQueries< T >::PlaneClosestPoints( const TObject& plane, const TObject& other, TPoint& onPlane, TPoint& onOther, T& distance )
{
    const TVector origin = other.GetWorldMatrix().GetTranslation();
    const CTPlane< T > shifted = ShiftedPlane( plane, origin );
    /* a convex object is its own single piece; a non-convex one takes the minimum over its pieces */
    const UInt32 count = PieceCount( other );
    bool found = false;
    for ( UInt32 i=0; i<count; ++i )
    {
        TPoint p;
        TPoint q;
        PlaneConvexClosestPoints( shifted, PieceSupport( other, i, origin ), p, q );
        const T d = ( q - p ).GetLength();
        if ( !found || d < distance )
        {
            found = true;
            distance = d;
            onPlane = p + origin;
            onOther = q + origin;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::ClosestPoints( const TObject& a, const TObject& b, TPoint& onA, TPoint& onB, T& distance, TGeoPairPath& path )
{
    if ( IsSupportShape( a ) && IsSupportShape( b ) )
    {
        int result = CTGeoPairDispatch< T >::GetCell( a.GetDispatchShapeType(), b.GetDispatchShapeType() ).closestPoints( a, b, onA, onB, distance );
        if ( result < 0 )
            result = CTGeoPairDispatch< T >::GetCell( b.GetDispatchShapeType(), a.GetDispatchShapeType() ).closestPoints( b, a, onB, onA, distance );
        if ( result >= 0 )
        {
            path = GEOPAIRPATH_ANALYTIC;
            return 0 != result;
        }
        const TVector origin = a.GetWorldMatrix().GetTranslation();
        SGjkResult< T > gjk;
        CTGjk< T >::ComputeDistance( a.GetPlacedSupport( origin ), b.GetPlacedSupport( origin ), CTGeometryTolerance< T >(), GUCEF_NULL, gjk );
        onA = gjk.pointOnA + origin;
        onB = gjk.pointOnB + origin;
        distance = gjk.overlapping ? TTraits::Zero() : gjk.distance;
        path = GEOPAIRPATH_SUPPORT;
        return true;
    }
    if ( IsPairShape( a ) && IsPairShape( b ) )
    {
        if ( !PieceClosestPoints( a, b, onA, onB, distance ) )
        {
            NotOffered( path );
            return false;
        }
        path = GEOPAIRPATH_SUPPORT;
        return true;
    }
    if ( IsUnboundedPlane( a ) && IsPairShape( b ) )
    {
        PlaneClosestPoints( a, b, onA, onB, distance );
        path = GEOPAIRPATH_PLANE;
        return true;
    }
    if ( IsPairShape( a ) && IsUnboundedPlane( b ) )
    {
        PlaneClosestPoints( b, a, onB, onA, distance );
        path = GEOPAIRPATH_PLANE;
        return true;
    }
    NotOffered( path );
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::PieceClosestPoints( const TObject& a, const TObject& b, TPoint& onA, TPoint& onB, T& distance )
{
    const TVector origin = a.GetWorldMatrix().GetTranslation();
    const UInt32 countA = PieceCount( a );
    const UInt32 countB = PieceCount( b );
    bool found = false;
    for ( UInt32 i=0; i<countA; ++i )
    {
        const typename TObject::TPlacedSupport supportA = PieceSupport( a, i, origin );
        for ( UInt32 j=0; j<countB; ++j )
        {
            SGjkResult< T > gjk;
            CTGjk< T >::ComputeDistance( supportA, PieceSupport( b, j, origin ), CTGeometryTolerance< T >(), GUCEF_NULL, gjk );
            const T d = gjk.overlapping ? TTraits::Zero() : gjk.distance;
            if ( !found || d < distance )
            {
                found = true;
                distance = d;
                onA = gjk.pointOnA + origin;
                onB = gjk.pointOnB + origin;
                /* touching pieces: nothing is nearer */
                if ( TTraits::Zero() == d )
                    return true;
            }
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
T
CTGeoObjectQueries< T >::Distance( const TObject& a, const TObject& b )
{
    TPoint onA;
    TPoint onB;
    T distance = TTraits::Zero();
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    if ( !ClosestPoints( a, b, onA, onB, distance, path ) )
        return TTraits::Max();
    return distance;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::Intersects( const TObject& a, const TObject& b, TGeoPairPath& path )
{
    if ( IsSupportShape( a ) && IsSupportShape( b ) )
    {
        int result = CTGeoPairDispatch< T >::GetCell( a.GetDispatchShapeType(), b.GetDispatchShapeType() ).intersects( a, b );
        if ( result < 0 )
            result = CTGeoPairDispatch< T >::GetCell( b.GetDispatchShapeType(), a.GetDispatchShapeType() ).intersects( b, a );
        if ( result >= 0 )
        {
            path = GEOPAIRPATH_ANALYTIC;
            return 0 != result;
        }
        const TVector origin = a.GetWorldMatrix().GetTranslation();
        path = GEOPAIRPATH_SUPPORT;
        return CTGjk< T >::Intersects( a.GetPlacedSupport( origin ), b.GetPlacedSupport( origin ), CTGeometryTolerance< T >() );
    }
    if ( IsPairShape( a ) && IsPairShape( b ) )
    {
        path = GEOPAIRPATH_SUPPORT;
        return PieceIntersects( a, b );
    }
    const bool planeFirst = IsUnboundedPlane( a ) && IsPairShape( b );
    if ( planeFirst || ( IsPairShape( a ) && IsUnboundedPlane( b ) ) )
    {
        const TObject& plane = planeFirst ? a : b;
        const TObject& other = planeFirst ? b : a;
        const TVector origin = other.GetWorldMatrix().GetTranslation();
        const CTPlane< T > shifted = ShiftedPlane( plane, origin );
        path = GEOPAIRPATH_PLANE;
        const UInt32 count = PieceCount( other );
        for ( UInt32 i=0; i<count; ++i )
        {
            T lowest = TTraits::Zero();
            T highest = TTraits::Zero();
            PlaneConvexRange( shifted, PieceSupport( other, i, origin ), lowest, highest );
            if ( !( TTraits::Zero() < lowest ) && !( highest < TTraits::Zero() ) )
                return true;
        }
        return false;
    }
    NotOffered( path );
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::PieceIntersects( const TObject& a, const TObject& b )
{
    const TVector origin = a.GetWorldMatrix().GetTranslation();
    const UInt32 countA = PieceCount( a );
    const UInt32 countB = PieceCount( b );
    for ( UInt32 i=0; i<countA; ++i )
    {
        const typename TObject::TPlacedSupport supportA = PieceSupport( a, i, origin );
        for ( UInt32 j=0; j<countB; ++j )
        {
            if ( CTGjk< T >::Intersects( supportA, PieceSupport( b, j, origin ), CTGeometryTolerance< T >() ) )
                return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::ComputeContact( const TObject& a, const TObject& b, TManifold& manifold, TGeoPairPath& path )
{
    manifold.Clear();
    if ( !IsPairShape( a ) || !IsPairShape( b ) )
    {
        NotOffered( path );
        return false;
    }
    TVector planeNormal;
    if ( !IsSupportShape( a ) || !IsSupportShape( b ) )
    {
        /* a non-convex object: the deepest contact over the pairs of pieces */
        path = GEOPAIRPATH_SUPPORT;
        const bool coplanar = AreCoplanarFlat( a, b, planeNormal );
        if ( PieceContact( a, b, coplanar, planeNormal, manifold ) )
            return true;
        manifold.Clear();
        return false;
    }
    if ( AreCoplanarFlat( a, b, planeNormal ) )
    {
        CoplanarContact( a, b, planeNormal, manifold );
        path = GEOPAIRPATH_SUPPORT;
        if ( manifold.IsTouching() )
            return true;
        manifold.Clear();
        return false;
    }
    int result = CTGeoPairDispatch< T >::GetCell( a.GetDispatchShapeType(), b.GetDispatchShapeType() ).computeContact( a, b, manifold );
    if ( result < 0 )
    {
        TManifold swapped;
        result = CTGeoPairDispatch< T >::GetCell( b.GetDispatchShapeType(), a.GetDispatchShapeType() ).computeContact( b, a, swapped );
        if ( result >= 0 )
        {
            manifold.normal = -swapped.normal;
            for ( UInt32 i=0; i<swapped.GetPointCount(); ++i )
            {
                const SContactPoint< T >& p = swapped.GetPoint( i );
                manifold.AddPoint( p.pointOnB, p.pointOnA, p.depth, p.featureId );
            }
        }
    }
    if ( result >= 0 )
    {
        path = GEOPAIRPATH_ANALYTIC;
        if ( 0 == result )
            manifold.Clear();
        return 0 != result;
    }
    const TVector origin = a.GetWorldMatrix().GetTranslation();
    ComputeSupportContact( a.GetPlacedSupport( origin ), b.GetPlacedSupport( origin ), CTGeometryTolerance< T >(), manifold );
    for ( UInt32 i=0; i<manifold.pointCount; ++i )
    {
        manifold.points[ i ].pointOnA = manifold.points[ i ].pointOnA + origin;
        manifold.points[ i ].pointOnB = manifold.points[ i ].pointOnB + origin;
    }
    path = GEOPAIRPATH_SUPPORT;
    if ( manifold.IsTouching() )
        return true;
    manifold.Clear();
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::AreCoplanarFlat( const TObject& a, const TObject& b, TVector& normal )
{
    if ( !a.IsFlat() || !b.IsFlat() )
        return false;
    const TVector na = a.NormalToWorld( MATH::CTDirectionVector3D< T >::UnitZ() ).AsVector();
    const TVector nb = b.NormalToWorld( MATH::CTDirectionVector3D< T >::UnitZ() ).AsVector();
    const T tolerance = TTraits::DefaultRelativeTolerance() * T( 100 );
    /*
     *  the sine of the angle between the planes (the length of the cross product of the unit normals), linear in the
     *  angle: 1 - |cos| is quadratic and let Float32 plates 2.5 degrees apart count as coplanar
     */
    if ( tolerance * tolerance < na.Cross( nb ).GetSquaredLength() )
        return false;
    const CTAxisAlignedBox3D< T >& ba = a.GetWorldBounds();
    const CTAxisAlignedBox3D< T >& bb = b.GetWorldBounds();
    const T size = ba.Merged( bb ).GetExtents().GetLength();
    const TVector offset = b.GetWorldPosition() - a.GetWorldPosition();
    if ( TTraits::Abs( na.Dot( offset ) ) > tolerance * ( TTraits::One() + size ) )
        return false;
    normal = na;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoObjectQueries< T >::CoplanarContact( const TObject& a, const TObject& b, const TVector& normal, TManifold& manifold )
{
    const TVector origin = a.GetWorldMatrix().GetTranslation();
    const T size = a.GetWorldBounds().Merged( b.GetWorldBounds() ).GetExtents().GetLength();
    const T half = TTraits::One() + size;
    typedef CTGeoFlatPrismSupport< typename TObject::TPlacedSupport > TPrism;
    ComputeSupportContact( TPrism( a.GetPlacedSupport( origin ), normal, half ), TPrism( b.GetPlacedSupport( origin ), normal, half ), CTGeometryTolerance< T >(), manifold );
    ProjectCoplanarManifold( normal, origin, manifold );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoObjectQueries< T >::ProjectCoplanarManifold( const TVector& normal, const TVector& origin, TManifold& manifold )
{
    /* back into the plane: the normal in the plane, the points on it */
    MATH::CTDirectionVector3D< T > inPlane;
    if ( MATH::CTDirectionVector3D< T >::TryCreate( manifold.normal - normal * manifold.normal.Dot( normal ), inPlane ) )
        manifold.normal = inPlane.AsVector();
    for ( UInt32 i=0; i<manifold.pointCount; ++i )
    {
        const TVector pa = manifold.points[ i ].pointOnA.ToPositionVector();
        const TVector pb = manifold.points[ i ].pointOnB.ToPositionVector();
        manifold.points[ i ].pointOnA = TPoint( pa - normal * pa.Dot( normal ) + origin );
        manifold.points[ i ].pointOnB = TPoint( pb - normal * pb.Dot( normal ) + origin );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::PieceContact( const TObject& a, const TObject& b, bool coplanar, const TVector& planeNormal, TManifold& manifold )
{
    manifold.Clear();
    const TVector origin = a.GetWorldMatrix().GetTranslation();
    const T size = a.GetWorldBounds().Merged( b.GetWorldBounds() ).GetExtents().GetLength();
    const T half = TTraits::One() + size;
    typedef typename TObject::TPlacedSupport TSupport;
    typedef CTGeoFlatPrismSupport< TSupport > TPrism;
    const UInt32 countA = PieceCount( a );
    const UInt32 countB = PieceCount( b );
    bool found = false;
    T deepest = TTraits::Zero();
    for ( UInt32 i=0; i<countA; ++i )
    {
        const TSupport supportA = PieceSupport( a, i, origin );
        for ( UInt32 j=0; j<countB; ++j )
        {
            TManifold candidate;
            const TSupport supportB = PieceSupport( b, j, origin );
            /* coplanar flat pieces are extruded along the normal: EPA then finds the in-plane penetration */
            if ( coplanar )
                ComputeSupportContact( TPrism( supportA, planeNormal, half ), TPrism( supportB, planeNormal, half ), CTGeometryTolerance< T >(), candidate );
            else
                ComputeSupportContact( supportA, supportB, CTGeometryTolerance< T >(), candidate );
            if ( !candidate.IsTouching() )
                continue;
            const T depth = candidate.GetMaxDepth();
            if ( !found || deepest < depth )
            {
                found = true;
                deepest = depth;
                manifold = candidate;
            }
        }
    }
    if ( !found )
        return false;
    if ( coplanar )
    {
        ProjectCoplanarManifold( planeNormal, origin, manifold );
        return true;
    }
    for ( UInt32 i=0; i<manifold.pointCount; ++i )
    {
        manifold.points[ i ].pointOnA = manifold.points[ i ].pointOnA + origin;
        manifold.points[ i ].pointOnB = manifold.points[ i ].pointOnB + origin;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::SphereContains( const TObject& outer, const TObject& inner, bool& imaged )
{
    imaged = false;
    const TMatrix toOuter = inner.GetTransformTo( outer );
    const CTSphere< T >& sphere = static_cast< const CTGeoObject< CTSphere< T > >& >( outer ).GetShape();
    switch ( inner.GetDispatchShapeType() )
    {
        case GEOSHAPE_SPHERE:
        {
            CTSphere< T > image;
            imaged = CTShapeTraits< CTSphere< T > >::TryImage( static_cast< const CTGeoObject< CTSphere< T > >& >( inner ).GetShape(), toOuter, image );
            return imaged && GEOMETRY::Contains( sphere, image );
        }
        case GEOSHAPE_AXIS_ALIGNED_BOX:
        {
            CTOrientedBox3D< T > image;
            imaged = CTShapeTraits< CTAxisAlignedBox3D< T > >::TryImage( static_cast< const CTGeoObject< CTAxisAlignedBox3D< T > >& >( inner ).GetShape(), toOuter, image );
            return imaged && GEOMETRY::Contains( sphere, image );
        }
        case GEOSHAPE_ORIENTED_BOX:
        {
            CTOrientedBox3D< T > image;
            imaged = CTShapeTraits< CTOrientedBox3D< T > >::TryImage( static_cast< const CTGeoObject< CTOrientedBox3D< T > >& >( inner ).GetShape(), toOuter, image );
            return imaged && GEOMETRY::Contains( sphere, image );
        }
        default:
            return false;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectQueries< T >::Contains( const TObject& outer, const TObject& inner, TGeoPairPath& path )
{
    if ( !IsOffered( outer, inner, GEOPAIRQUERY_CONTAINS ) )
    {
        NotOffered( path );
        return false;
    }
    /* containment is affine invariant: everything happens in the outer object's space */
    const TGeoShapeType outerType = outer.GetDispatchShapeType();
    if ( GEOSHAPE_SPHERE == outerType )
    {
        bool imaged = false;
        const bool contained = SphereContains( outer, inner, imaged );
        if ( !imaged )
        {
            NotOffered( path );
            return false;
        }
        path = GEOPAIRPATH_ANALYTIC;
        return contained;
    }

    const TMatrix toOuter = inner.GetTransformTo( outer );
    CTPlaneBoundedVolume< T > volume;
    if ( GEOSHAPE_AXIS_ALIGNED_BOX == outerType )
        volume = CTPlaneBoundedVolume< T >::FromAxisAlignedBox( static_cast< const CTGeoObject< CTAxisAlignedBox3D< T > >& >( outer ).GetShape() );
    else
    if ( GEOSHAPE_ORIENTED_BOX == outerType )
        volume = CTPlaneBoundedVolume< T >::FromOrientedBox( static_cast< const CTGeoObject< CTOrientedBox3D< T > >& >( outer ).GetShape() );
    else
        volume = static_cast< const CTGeoObject< CTPlaneBoundedVolume< T > >& >( outer ).GetShape();
    const typename TObject::TPlacedSupport placed( typename TObject::CLocalSupport( inner ), toOuter.GetLinear(), toOuter.GetTranslation() );
    path = GEOPAIRPATH_PLANE_SET;
    return FRUSTUMCLASSIFICATION_INSIDE == ClassifyConvexToPlaneSet( volume.GetPlanes(), volume.GetPlaneCount(), placed );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** True when the pair query is offered for the two linked objects as they are now */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
IsGeoPairQueryOffered( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b, TGeoPairQuery query )
{
    return CTGeoObjectQueries< T >::IsOffered( a, b, query );
}

/** The nearest points of two linked objects in world coordinates (false when not offered) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
ClosestPoints( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b, MATH::CTPoint3D< T >& onA, MATH::CTPoint3D< T >& onB )
{
    T distance = MATH::CTNumericTraits< T >::Zero();
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    return CTGeoObjectQueries< T >::ClosestPoints( a, b, onA, onB, distance, path );
}

/** The world distance between two linked objects, 0 when they touch or overlap; the largest finite number of the kind when not offered */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE T
Distance( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b )
{
    return CTGeoObjectQueries< T >::Distance( a, b );
}

/** True when two linked objects touch or overlap (false when not offered) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Intersects( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b )
{
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    return CTGeoObjectQueries< T >::Intersects( a, b, path );
}

/** The contact of two linked objects in world coordinates; true when they touch or overlap (false and an empty manifold when not offered) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
ComputeContact( const CTGeoObjectBase< T >& a, const CTGeoObjectBase< T >& b, CTContactManifold< T >& manifold )
{
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    return CTGeoObjectQueries< T >::ComputeContact( a, b, manifold, path );
}

/** True when the linked object 'inner' lies entirely in 'outer' (false when not offered) */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE bool
Contains( const CTGeoObjectBase< T >& outer, const CTGeoObjectBase< T >& inner )
{
    TGeoPairPath path = GEOPAIRPATH_NOT_OFFERED;
    return CTGeoObjectQueries< T >::Contains( outer, inner, path );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoPairDispatch< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoPairDispatch< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoObjectQueries< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoObjectQueries< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_GEOOBJECTQUERIES_H ? */
