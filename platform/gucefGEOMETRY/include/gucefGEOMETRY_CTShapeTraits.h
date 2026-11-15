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

#ifndef GUCEF_GEOMETRY_CTSHAPETRAITS_H
#define GUCEF_GEOMETRY_CTSHAPETRAITS_H

/*
 *  CTShapeTraits< TShape >: what a linked object needs to know about a value shape (section 7.4.2 of the design), so
 *  that one template (CTGeoObject< TShape >, CTGeoObject2D< TShape2D >) serves every shape without per-shape classes.
 *
 *  3D shapes (Dimensions 3): the shape type id, convexity and boundedness, bounds, support, ray cast, closest point and
 *  containment, an optional per-object cache (TCache, BuildCache: the convex body of a plane-bounded volume), and the
 *  image of the shape under an affine map (HasImage, TImage, TryImage: the shape family it maps into, for the analytic
 *  pair queries; false when the map leaves the family, for example a sphere under a non-uniform scale).
 *  2D shapes (Dimensions 2): the shape type id, convexity, bounds, support, closest point and containment in the plane.
 *
 *  Shapes without a specialisation cannot be linked (compile error at CTGeoObject< TShape >). All members are static,
 *  inline and allocation-free except BuildCache.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_GEOMETRY_VOLUMESHAPES_H
#include "gucefGEOMETRY_volumeshapes.h"
#define GUCEF_GEOMETRY_VOLUMESHAPES_H
#endif /* GUCEF_GEOMETRY_VOLUMESHAPES_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#include "gucefGEOMETRY_CTConvexHull3D.h"
#define GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#endif /* GUCEF_GEOMETRY_CTCONVEXHULL3D_H ? */

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

/** The cache of shapes that need none */
struct SGeoShapeNoCache
{
};

/*-------------------------------------------------------------------------*/

/** Declared only: a shape without a specialisation cannot be linked */
template < typename TShape >
class CTShapeTraits;

/*-------------------------------------------------------------------------*/

/** The members shared by the bounded convex 3D shapes: forwards to the uniform members of the shape (section 2.1) */
template < typename TShape, int ShapeTypeId, typename TCacheType = SGeoShapeNoCache >
class CTConvexShapeTraits3D
{
    public:

    typedef typename TShape::TValue T;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTMatrix4D< T > TMatrix;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef TCacheType TCache;

    enum { Dimensions = 3 };

    static GUCEF_GEOMETRY_FORCE_INLINE TGeoShapeType GetShapeType( void ) { return (TGeoShapeType) ShapeTypeId; }
    static GUCEF_GEOMETRY_FORCE_INLINE void BuildCache( const TShape& /*shape*/, TCache& /*cache*/ ) {}
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsConvex( const TShape& /*shape*/, const TCache& /*cache*/ ) { return true; }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsBounded( const TShape& /*shape*/, const TCache& /*cache*/ ) { return true; }

    /** True for a shape without points (IsQueryableConvex of the query facade: an empty box, a negative radius, ...) */
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const TShape& shape, const TCache& /*cache*/ ) { return !IsQueryableConvex( shape ); }

    static GUCEF_GEOMETRY_FORCE_INLINE void GetBounds( const TShape& shape, const TCache& /*cache*/, TBox& bounds ) { bounds = shape.GetBounds(); }
    static GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TShape& shape, const TCache& /*cache*/, const TVector& direction ) { return shape.GetSupport( direction ); }
    static GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TShape& shape, const TCache& /*cache*/, const TPoint& point ) { return shape.ClosestPoint( point ); }
    static GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TShape& shape, const TCache& /*cache*/, const TPoint& point ) { return shape.Contains( point ); }

    static GUCEF_GEOMETRY_FORCE_INLINE bool RayCast( const TShape& shape, const TCache& /*cache*/, const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& hit )
    {
        return shape.RayCast( ray, maxDistance, hit );
    }
};

/*-------------------------------------------------------------------------*/

/** The image of a shape under an affine map through its TryTransformed member (closed under similarities) */
template < typename TShape, int ShapeTypeId, typename TCacheType = SGeoShapeNoCache >
class CTTransformableShapeTraits3D : public CTConvexShapeTraits3D< TShape, ShapeTypeId, TCacheType >
{
    public:

    typedef typename TShape::TValue T;
    typedef TShape TImage;

    enum { HasImage = 1 };

    static GUCEF_GEOMETRY_FORCE_INLINE bool TryImage( const TShape& shape, const MATH::CTMatrix4D< T >& matrix, TImage& image ) { return shape.TryTransformed( matrix, image ); }
};

/*-------------------------------------------------------------------------*/

template < typename T >
class CTShapeTraits< CTSphere< T > > : public CTTransformableShapeTraits3D< CTSphere< T >, GEOSHAPE_SPHERE >
{
};

template < typename T >
class CTShapeTraits< CTCapsule< T > > : public CTTransformableShapeTraits3D< CTCapsule< T >, GEOSHAPE_CAPSULE >
{
};

template < typename T >
class CTShapeTraits< CTOrientedBox3D< T > > : public CTTransformableShapeTraits3D< CTOrientedBox3D< T >, GEOSHAPE_ORIENTED_BOX >
{
};

/** The cache of a cylinder: its support adaptor of section 7.6.1 (axis normalised once, stable for directions along the axis) */
template < typename T >
struct SCylinderShapeCache
{
    CTCylinderSupport< T > support;

    SCylinderShapeCache( void ) : support( MATH::CTPoint3D< T >(), MATH::CTPoint3D< T >(), MATH::CTNumericTraits< T >::Zero() ) {}
};

/** A cylinder: support through its cached support adaptor */
template < typename T >
class CTShapeTraits< CTCylinder< T > > : public CTTransformableShapeTraits3D< CTCylinder< T >, GEOSHAPE_CYLINDER, SCylinderShapeCache< T > >
{
    public:

    typedef SCylinderShapeCache< T > TCache;

    static GUCEF_GEOMETRY_FORCE_INLINE void BuildCache( const CTCylinder< T >& shape, TCache& cache ) { cache.support = CTCylinderSupport< T >( shape.start, shape.end, shape.radius ); }

    static GUCEF_GEOMETRY_FORCE_INLINE MATH::CTPoint3D< T > GetSupport( const CTCylinder< T >& /*shape*/, const TCache& cache, const MATH::CTVector3D< T >& direction )
    {
        return cache.support.GetSupport( direction );
    }
};

/*-------------------------------------------------------------------------*/

/** The cache of a cone: its support adaptor of section 7.6.1 */
template < typename T >
struct SConeShapeCache
{
    CTConeSupport< T > support;

    SConeShapeCache( void ) : support( MATH::CTPoint3D< T >(), MATH::CTVector3D< T >(), MATH::CTNumericTraits< T >::Zero(), MATH::CTNumericTraits< T >::Zero() ) {}
};

/** A cone: support through its cached support adaptor */
template < typename T >
class CTShapeTraits< CTCone< T > > : public CTTransformableShapeTraits3D< CTCone< T >, GEOSHAPE_CONE, SConeShapeCache< T > >
{
    public:

    typedef SConeShapeCache< T > TCache;

    static GUCEF_GEOMETRY_FORCE_INLINE void BuildCache( const CTCone< T >& shape, TCache& cache ) { cache.support = CTSupportMapping< CTCone< T > >::Make( shape ); }

    static GUCEF_GEOMETRY_FORCE_INLINE MATH::CTPoint3D< T > GetSupport( const CTCone< T >& /*shape*/, const TCache& cache, const MATH::CTVector3D< T >& direction )
    {
        return cache.support.GetSupport( direction );
    }
};

/*-------------------------------------------------------------------------*/

/** An axis aligned box in its own space is an oriented box in any other: its image is a CTOrientedBox3D */
template < typename T >
class CTShapeTraits< CTAxisAlignedBox3D< T > > : public CTConvexShapeTraits3D< CTAxisAlignedBox3D< T >, GEOSHAPE_AXIS_ALIGNED_BOX >
{
    public:

    typedef CTOrientedBox3D< T > TImage;

    enum { HasImage = 1 };

    static bool TryImage( const CTAxisAlignedBox3D< T >& shape, const MATH::CTMatrix4D< T >& matrix, TImage& image )
    {
        if ( shape.IsEmpty() )
            return false;
        return TImage( shape.GetCentre(), MATH::CTQuaternion< T >(), shape.GetHalfExtents() ).TryTransformed( matrix, image );
    }
};

/*-------------------------------------------------------------------------*/

/** A triangle maps to a triangle under every affine map */
template < typename T >
class CTShapeTraits< CTTriangle3D< T > > : public CTConvexShapeTraits3D< CTTriangle3D< T >, GEOSHAPE_TRIANGLE >
{
    public:

    typedef CTTriangle3D< T > TImage;

    enum { HasImage = 1 };

    static GUCEF_GEOMETRY_FORCE_INLINE bool TryImage( const CTTriangle3D< T >& shape, const MATH::CTMatrix4D< T >& matrix, TImage& image )
    {
        image = shape.Transformed( matrix );
        return true;
    }
};

/*-------------------------------------------------------------------------*/

/** A hull is answered through its support mapping (its transformed copy would allocate) */
template < typename T >
class CTShapeTraits< CTConvexHull3D< T > > : public CTConvexShapeTraits3D< CTConvexHull3D< T >, GEOSHAPE_CONVEX_HULL >
{
    public:

    typedef CTConvexHull3D< T > TImage;

    enum { HasImage = 0 };

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsBounded( const CTConvexHull3D< T >& shape, const SGeoShapeNoCache& /*cache*/ ) { return !shape.IsEmpty(); }
    static GUCEF_GEOMETRY_FORCE_INLINE bool TryImage( const CTConvexHull3D< T >& /*shape*/, const MATH::CTMatrix4D< T >& /*matrix*/, TImage& /*image*/ ) { return false; }
};

/*-------------------------------------------------------------------------*/

/**
 *  A plane-bounded volume: convex, bounded when its planes close a body. The cache is the convex hull of that body
 *  (built when the shape is set; it allocates), which gives the support mapping and the closest points; an unbounded
 *  volume has no support and the closest point of an outside point is not offered (the point itself is returned).
 */
template < typename T >
class CTShapeTraits< CTPlaneBoundedVolume< T > >
{
    public:

    typedef CTPlaneBoundedVolume< T > TShape;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTConvexHull3D< T > TCache;
    typedef TShape TImage;

    enum { Dimensions = 3, HasImage = 0 };

    static GUCEF_GEOMETRY_FORCE_INLINE TGeoShapeType GetShapeType( void ) { return GEOSHAPE_PLANE_BOUNDED_VOLUME; }

    static void BuildCache( const TShape& shape, TCache& cache )
    {
        GUCEF::vector< TPoint > vertices;
        GUCEF::vector< SConvexBodyFace > faces;
        if ( shape.BuildConvexBody( vertices, faces ) && !vertices.empty() )
            cache = TCache( &vertices[ 0 ], (UInt32) vertices.size() );
        else
            cache = TCache();
    }

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsConvex( const TShape& /*shape*/, const TCache& /*cache*/ ) { return true; }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsBounded( const TShape& /*shape*/, const TCache& cache ) { return !cache.IsEmpty(); }

    /** A volume without a closed body is treated as unbounded (BuildConvexBody does not tell an empty region from an unbounded one) */
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const TShape& /*shape*/, const TCache& /*cache*/ ) { return false; }

    static void GetBounds( const TShape& shape, const TCache& /*cache*/, TBox& bounds )
    {
        if ( !shape.TryGetBounds( bounds ) )
        {
            const T big = MATH::CTNumericTraits< T >::Max();
            bounds = TBox( TPoint( -big, -big, -big ), TPoint( big, big, big ) );
        }
    }

    static GUCEF_GEOMETRY_FORCE_INLINE TPoint GetSupport( const TShape& /*shape*/, const TCache& cache, const TVector& direction )
    {
        return cache.IsEmpty() ? TPoint() : cache.GetSupport( direction );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE TPoint ClosestPoint( const TShape& shape, const TCache& cache, const TPoint& point )
    {
        return shape.Contains( point ) || cache.IsEmpty() ? point : cache.ClosestPoint( point );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TShape& shape, const TCache& /*cache*/, const TPoint& point ) { return shape.Contains( point ); }

    static GUCEF_GEOMETRY_FORCE_INLINE bool RayCast( const TShape& shape, const TCache& /*cache*/, const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& hit )
    {
        return shape.RayCast( ray, maxDistance, hit );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE bool TryImage( const TShape& /*shape*/, const MATH::CTMatrix4D< T >& /*matrix*/, TImage& /*image*/ ) { return false; }
};

/*-------------------------------------------------------------------------*/

/** The members shared by the 2D shapes: forwards to the uniform members of the shape in the plane */
template < typename TShape2D, int ShapeTypeId >
class CTShapeTraits2D
{
    public:

    typedef typename TShape2D::TValue T;
    typedef MATH::CTPoint2D< T > TPoint2D;
    typedef MATH::CTVector2D< T > TVector2D;
    typedef CTRectangle< T > TRectangle;

    enum { Dimensions = 2, HasImage = 0 };

    static GUCEF_GEOMETRY_FORCE_INLINE TGeoShapeType GetShapeType( void ) { return (TGeoShapeType) ShapeTypeId; }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsConvex( const TShape2D& /*shape*/ ) { return true; }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const TShape2D& /*shape*/ ) { return false; }
    static GUCEF_GEOMETRY_FORCE_INLINE TRectangle GetBounds( const TShape2D& shape ) { return shape.GetBounds(); }
    static GUCEF_GEOMETRY_FORCE_INLINE TPoint2D GetSupport( const TShape2D& shape, const TVector2D& direction ) { return shape.GetSupport( direction ); }
    static GUCEF_GEOMETRY_FORCE_INLINE TPoint2D ClosestPoint( const TShape2D& shape, const TPoint2D& point ) { return shape.ClosestPoint( point ); }
    static GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TShape2D& shape, const TPoint2D& point ) { return shape.Contains( point ); }
};

/*-------------------------------------------------------------------------*/

template < typename T >
class CTShapeTraits< CTCircle< T > > : public CTShapeTraits2D< CTCircle< T >, GEOSHAPE_CIRCLE >
{
    public:

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const CTCircle< T >& shape ) { return MATH::CTNumericTraits< T >::IsNegative( shape.radius ); }
};

template < typename T >
class CTShapeTraits< CTRectangle< T > > : public CTShapeTraits2D< CTRectangle< T >, GEOSHAPE_RECTANGLE >
{
    public:

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const CTRectangle< T >& shape ) { return shape.IsEmpty(); }
};

template < typename T >
class CTShapeTraits< CTOrientedRectangle< T > > : public CTShapeTraits2D< CTOrientedRectangle< T >, GEOSHAPE_ORIENTED_RECTANGLE >
{
    public:

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const CTOrientedRectangle< T >& shape )
    {
        return MATH::CTNumericTraits< T >::IsNegative( shape.halfExtents.x ) || MATH::CTNumericTraits< T >::IsNegative( shape.halfExtents.y );
    }
};

template < typename T >
class CTShapeTraits< CTTriangle2D< T > > : public CTShapeTraits2D< CTTriangle2D< T >, GEOSHAPE_TRIANGLE_2D >
{
};

/** A polygon is convex when its ring is (checked when asked; the support is that of its convex hull) */
template < typename T >
class CTShapeTraits< CTPolygon2D< T > > : public CTShapeTraits2D< CTPolygon2D< T >, GEOSHAPE_POLYGON >
{
    public:

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsConvex( const CTPolygon2D< T >& shape ) { return shape.GetPointCount() >= 3 && shape.IsConvex(); }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const CTPolygon2D< T >& shape ) { return shape.GetPointCount() < 3; }
};

/** A polygon with holes is convex only without holes and with a convex outer ring */
template < typename T >
class CTShapeTraits< CTPolygonWithHoles2D< T > > : public CTShapeTraits2D< CTPolygonWithHoles2D< T >, GEOSHAPE_POLYGON_WITH_HOLES >
{
    public:

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsConvex( const CTPolygonWithHoles2D< T >& shape ) { return 0 == shape.GetHoleCount() && shape.outer.GetPointCount() >= 3 && shape.outer.IsConvex(); }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const CTPolygonWithHoles2D< T >& shape ) { return shape.outer.GetPointCount() < 3; }
};

/** A polyline is never treated as convex; it contains the points on it within the default tolerance */
template < typename T >
class CTShapeTraits< CTPolyline2D< T > > : public CTShapeTraits2D< CTPolyline2D< T >, GEOSHAPE_POLYLINE_2D >
{
    public:

    static GUCEF_GEOMETRY_FORCE_INLINE bool IsConvex( const CTPolyline2D< T >& /*shape*/ ) { return false; }
    static GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( const CTPolyline2D< T >& shape ) { return 0 == shape.GetPointCount(); }
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTSHAPETRAITS_H ? */
