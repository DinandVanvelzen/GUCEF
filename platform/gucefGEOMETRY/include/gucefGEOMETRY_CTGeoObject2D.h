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

#ifndef GUCEF_GEOMETRY_CTGEOOBJECT2D_H
#define GUCEF_GEOMETRY_CTGEOOBJECT2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#include "gucefGEOMETRY_CTGeoObjectBase.h"
#define GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECTBASE_H ? */

#ifndef GUCEF_GEOMETRY_CTSHAPETRAITS_H
#include "gucefGEOMETRY_CTShapeTraits.h"
#define GUCEF_GEOMETRY_CTSHAPETRAITS_H
#endif /* GUCEF_GEOMETRY_CTSHAPETRAITS_H ? */

#ifndef GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#include "gucefGEOMETRY_PolygonTriangulation.h"
#define GUCEF_GEOMETRY_POLYGONTRIANGULATION_H
#endif /* GUCEF_GEOMETRY_POLYGONTRIANGULATION_H ? */

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
 *  The triangulation of the 2D shapes that are regions and can be non-convex (polygon, polygon with holes): the
 *  triangles of the hosted CTGeoObject2D cache. Other shapes have no pieces (Has = 0); a polyline is not a region.
 */
template < typename TShape2D >
class CTGeoPolygonPieces
{
    public:

    enum { Has = 0 };

    static bool Build( const TShape2D& /*shape*/, GUCEF::vector< CTTriangle2D< typename TShape2D::TValue > >& /*triangles*/ ) { return false; }
};

template < typename T >
class CTGeoPolygonPieces< CTPolygonWithHoles2D< T > >
{
    public:

    enum { Has = 1 };

    /** The ear clipping triangulation (best effort for a polygon that is not simple); false without triangles */
    static bool Build( const CTPolygonWithHoles2D< T >& shape, GUCEF::vector< CTTriangle2D< T > >& triangles )
    {
        CTPolygonTriangulator< T > triangulator;
        GUCEF::vector< UInt32 > indices;
        TriangulatePolygon( shape, triangles, triangulator, indices );
        return !triangles.empty();
    }
};

template < typename T >
class CTGeoPolygonPieces< CTPolygon2D< T > >
{
    public:

    enum { Has = 1 };

    static bool Build( const CTPolygon2D< T >& shape, GUCEF::vector< CTTriangle2D< T > >& triangles )
    {
        return CTGeoPolygonPieces< CTPolygonWithHoles2D< T > >::Build( CTPolygonWithHoles2D< T >( shape ), triangles );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  One 2D value shape (circle, rectangle, oriented rectangle, triangle, polygon, polygon with holes, polyline) linked
 *  into a CTGeoSpace tree (section 7.4.5 of the design). The shape lies in the local XY plane of the node; parented to a
 *  CTGeoPlane (or any node), it lies in that node's XY plane and maps to 3D through the transforms of the chain.
 *
 *  - Placement in the plane: SetLocalTransform2D( CTTransform2D ) sets translation ( x, y, 0 ), the rotation about the
 *    local +Z axis and the scale ( sx, sy, 1 ); the full 3D local transform of CTGeoSpace remains available.
 *  - 3D answers (7.4.5): a ray meets the plane of the shape and the 2D hit point is tested; the distance from a 3D point
 *    is sqrt( h^2 + d2D^2 ) (closest points lift the 2D closest point); convex shapes have a flat support mapping, so
 *    GJK pairs them with 3D convex shapes. Non-convex polygons (and polygons with holes) answer the pair queries through
 *    a lazily built cache of the triangles of their triangulation (cleared when the shape is set): each triangle is a
 *    flat convex piece (GetConvexPieceCount), and the pair queries take the minimum, any or deepest over the pieces; their
 *    point and ray queries are exact and do not use the cache. A polyline has no area and no pieces.
 *  - Kinds: Float32 and Float64 instantiations of the listed shapes are exported.
 */
template < typename TShape2D >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoObject2D : public CTGeoObjectBase< typename TShape2D::TValue >
{
    public:

    typedef typename TShape2D::TValue TKind;
    typedef CTGeoObjectBase< TKind > TBase;
    typedef MATH::CTNumericTraits< TKind > TTraits;
    typedef TShape2D TShapeType;
    typedef CTShapeTraits< TShape2D > TShapeTraits;
    typedef MATH::CTTransform3D< TKind > TTransform;
    typedef MATH::CTTransform2D< TKind > TTransform2D;
    typedef MATH::CTPoint3D< TKind > TPoint;
    typedef MATH::CTVector3D< TKind > TVector;
    typedef MATH::CTPoint2D< TKind > TPoint2D;
    typedef MATH::CTVector2D< TKind > TVector2D;
    typedef CTAxisAlignedBox3D< TKind > TBox;
    typedef CTRay3D< TKind > TRay;
    typedef SRayCastResult< TKind > TRayHit;

    GUCEF_GEOMETRY_STATIC_ASSERT( 2 == TShapeTraits::Dimensions, CTGeoObject2D_requires_a_2D_shape_use_CTGeoObject_for_3D_shapes );

    CTGeoObject2D( void ) : TBase(), m_shape(), m_triangles(), m_convex( false ), m_empty( false ), m_trianglesBuilt( false ) { ShapeSet(); }

    explicit CTGeoObject2D( const TShape2D& shape ) : TBase(), m_shape( shape ), m_triangles(), m_convex( false ), m_empty( false ), m_trianglesBuilt( false ) { ShapeSet(); }

    CTGeoObject2D( const TShape2D& shape, const TTransform2D& localTransform2D )
        : TBase( ToTransform3D( localTransform2D ) ), m_shape( shape ), m_triangles(), m_convex( false ), m_empty( false ), m_trianglesBuilt( false )
    {
        ShapeSet();
    }

    virtual ~CTGeoObject2D() {}

    GUCEF_GEOMETRY_FORCE_INLINE const TShape2D& GetShape( void ) const { return m_shape; }

    /** Replaces the shape (plane coordinates): the triangle cache cleared, world bounds invalidated, structures told */
    void SetShape( const TShape2D& shape )
    {
        m_shape = shape;
        ShapeSet();
        this->OnShapeChanged();
    }

    /** Places the shape in the XY plane of the parent: translation ( x, y, 0 ), rotation about +Z, scale ( sx, sy, 1 ) */
    void SetLocalTransform2D( const TTransform2D& localTransform2D ) { this->SetLocalTransform( ToTransform3D( localTransform2D ) ); }

    /** The 2D part of the local transform: x, y of the position, the angle about +Z, x, y of the scale */
    TTransform2D GetLocalTransform2D( void ) const
    {
        const TTransform& local = this->GetLocalTransform();
        const TKind angle = TTraits::Two() * TTraits::Atan2( local.rotation.z, local.rotation.w );
        return TTransform2D( TVector2D( local.position.x, local.position.y ), MATH::CTRadian< TKind >( angle ), TVector2D( local.scale.x, local.scale.y ) );
    }

    /** The 3D local transform of a 2D one */
    static TTransform ToTransform3D( const TTransform2D& t )
    {
        return TTransform( TVector( t.position.x, t.position.y, TTraits::Zero() ),
                           MATH::CTQuaternion< TKind >::FromAxisAngle( MATH::CTDirectionVector3D< TKind >::UnitZ(), t.rotation ),
                           TVector( t.scale.x, t.scale.y, TTraits::One() ) );
    }

    virtual TGeoShapeType GetShapeType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::GetShapeType(); }

    virtual void GetLocalBounds( TBox& bounds ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const CTRectangle< TKind > rectangle = TShapeTraits::GetBounds( m_shape );
        if ( rectangle.IsEmpty() )
        {
            bounds = TBox();
            return;
        }
        bounds = TBox( TPoint( rectangle.min.x, rectangle.min.y, TTraits::Zero() ), TPoint( rectangle.max.x, rectangle.max.y, TTraits::Zero() ) );
    }

    /** The flat support: the 2D support along ( x, y ) of the direction at z = 0 (any point of the shape when that is zero) */
    virtual TPoint GetLocalSupport( const TVector& localDirection ) const GUCEF_VIRTUAL_OVERRIDE
    {
        TVector2D direction( localDirection.x, localDirection.y );
        if ( TTraits::Zero() == direction.x && TTraits::Zero() == direction.y )
            direction = TVector2D( TTraits::One(), TTraits::Zero() );
        const TPoint2D p = TShapeTraits::GetSupport( m_shape, direction );
        return TPoint( p.x, p.y, TTraits::Zero() );
    }

    /** The triangles of the triangulation of a non-convex polygon (built on first use); 0 for every other shape and for a degenerate polygon */
    virtual UInt32 GetConvexPieceCount( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( 0 == (int) CTGeoPolygonPieces< TShape2D >::Has || m_convex || m_empty )
            return 0;
        EnsureTriangles();
        return (UInt32) m_triangles.size();
    }

    /** The flat support of triangle 'piece': its corner furthest along ( x, y ) of the direction (the first on ties) */
    virtual TPoint GetLocalPieceSupport( UInt32 piece, const TVector& localDirection ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( piece >= (UInt32) m_triangles.size() )
            return GetLocalSupport( localDirection );
        TVector2D direction( localDirection.x, localDirection.y );
        if ( TTraits::Zero() == direction.x && TTraits::Zero() == direction.y )
            direction = TVector2D( TTraits::One(), TTraits::Zero() );
        const TPoint2D p = m_triangles[ piece ].GetSupport( direction );
        return TPoint( p.x, p.y, TTraits::Zero() );
    }

    virtual bool LocalRayCast( const TRay& localRay, const TKind& maxDistance, TRayHit& hit ) const GUCEF_VIRTUAL_OVERRIDE
    {
        TKind distance = TTraits::Zero();
        TPoint point;
        if ( !TBase::IntersectLocalXYPlane( localRay, maxDistance, distance, point ) || !TShapeTraits::Contains( m_shape, TPoint2D( point.x, point.y ) ) )
            return false;
        TBase::SetFlatHit( localRay, distance, point, hit );
        return true;
    }

    virtual TPoint GetLocalClosestPoint( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE
    {
        const TPoint2D p = TShapeTraits::ClosestPoint( m_shape, TPoint2D( localPoint.x, localPoint.y ) );
        return TPoint( p.x, p.y, TTraits::Zero() );
    }

    virtual bool LocalContains( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return TBase::IsOnLocalXYPlane( localPoint ) && TShapeTraits::Contains( m_shape, TPoint2D( localPoint.x, localPoint.y ) );
    }

    /** Cached when the shape is set (convexity of a polygon is O(n)) */
    virtual bool IsConvex( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_convex; }
    virtual bool IsEmpty( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_empty; }
    virtual bool IsFlat( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    private:

    /** Builds the triangle cache once per shape (owner-thread only, like every query of a linked object) */
    void EnsureTriangles( void ) const
    {
        if ( m_trianglesBuilt )
            return;
        m_trianglesBuilt = true;
        if ( !CTGeoPolygonPieces< TShape2D >::Build( m_shape, m_triangles ) )
            m_triangles.clear();
    }

    void ShapeSet( void )
    {
        m_triangles.clear();
        m_trianglesBuilt = false;
        m_empty = TShapeTraits::IsEmpty( m_shape );
        m_convex = !m_empty && TShapeTraits::IsConvex( m_shape );
        this->SetLibraryShapeType( TShapeTraits::GetShapeType() );
    }

    TShape2D m_shape;
    mutable GUCEF::vector< CTTriangle2D< TKind > > m_triangles;     /**< the lazily built triangle cache of a non-convex polygon */
    bool m_convex;
    bool m_empty;
    mutable bool m_trianglesBuilt;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_GEOMETRY_CTGEOOBJECT2D_KIND( macro, kind )               \
    macro( CTGeoObject2D< CTCircle< kind > > )                          \
    macro( CTGeoObject2D< CTRectangle< kind > > )                       \
    macro( CTGeoObject2D< CTOrientedRectangle< kind > > )               \
    macro( CTGeoObject2D< CTTriangle2D< kind > > )                      \
    macro( CTGeoObject2D< CTPolygon2D< kind > > )                       \
    macro( CTGeoObject2D< CTPolygonWithHoles2D< kind > > )              \
    macro( CTGeoObject2D< CTPolyline2D< kind > > )

GUCEF_GEOMETRY_CTGEOOBJECT2D_KIND( GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS, Float32 )
GUCEF_GEOMETRY_CTGEOOBJECT2D_KIND( GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS, Float64 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOOBJECT2D_H ? */
