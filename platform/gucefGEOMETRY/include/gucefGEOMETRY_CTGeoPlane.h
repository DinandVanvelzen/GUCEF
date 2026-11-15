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

#ifndef GUCEF_GEOMETRY_CTGEOPLANE_H
#define GUCEF_GEOMETRY_CTGEOPLANE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#include "gucefGEOMETRY_CTGeoObjectBase.h"
#define GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECTBASE_H ? */

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
 *  A plane that hosts 2D geometry (section 7.4.5 of the design): the local XY plane of the node is the plane, local +Z
 *  its normal (front). Children placed with 2D transforms (CTGeoObject2D) lie in it, and every 2D point of the plane
 *  maps to 3D through the node's world matrix.
 *
 *  - Extents: unbounded by default (an infinite two-sided sheet); SetExtents bounds it to a rectangle of the plane,
 *    which makes it a flat convex shape (support mapping, GJK pairs); an empty rectangle bounds it to nothing (an
 *    empty shape: no hits, empty bounds, no pair queries).
 *  - ProjectToPlane projects along the world normal, so the plane coordinates and the signed height are exact for any
 *    affine placement (non-uniform scale and shear included); FromPlane is its inverse for points in the plane.
 *  - Kinds: Float32 and Float64 are exported.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoPlane : public CTGeoObjectBase< T >
{
    public:

    typedef CTGeoObjectBase< T > TBase;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTTransform3D< T > TTransform;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTPoint2D< T > TPoint2D;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTRay3D< T > TRay;
    typedef SRayCastResult< T > TRayHit;
    typedef CTRectangle< T > TRectangle;

    /** An unbounded plane at the identity placement: the XY plane, normal +Z */
    CTGeoPlane( void ) : TBase(), m_extents(), m_bounded( false ) { this->SetLibraryShapeType( GEOSHAPE_PLANE ); }

    /** An unbounded plane at 'localTransform' */
    explicit CTGeoPlane( const TTransform& localTransform ) : TBase( localTransform ), m_extents(), m_bounded( false ) { this->SetLibraryShapeType( GEOSHAPE_PLANE ); }

    virtual ~CTGeoPlane() {}

    /** Bounds the plane to a rectangle in plane coordinates (an empty rectangle bounds it to nothing) */
    void SetExtents( const TRectangle& extents )
    {
        m_extents = extents;
        m_bounded = true;
        this->OnShapeChanged();
    }

    /** Makes the plane unbounded again */
    void ClearExtents( void )
    {
        m_bounded = false;
        this->OnShapeChanged();
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool HasExtents( void ) const { return m_bounded; }
    GUCEF_GEOMETRY_FORCE_INLINE const TRectangle& GetExtents( void ) const { return m_extents; }

    /**
     *  The plane coordinates of the foot of the perpendicular from a world point and the signed height of the point above
     *  the plane in world units (positive on the +Z side); exact for any affine placement
     */
    void ProjectToPlane( const TPoint& worldPoint, TPoint2D& planePoint, T& signedHeight ) const
    {
        const CTPlane< T > plane = GetPlane();
        signedHeight = plane.GetSignedDistance( worldPoint );
        const TPoint local = this->ToLocal( plane.Project( worldPoint ) );
        planePoint = TPoint2D( local.x, local.y );
    }

    /** The world point of a point of the plane */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint FromPlane( const TPoint2D& planePoint ) const { return this->ToWorld( planePoint ); }

    /**
     *  Where a world ray meets the plane (within the extents when bounded), in plane coordinates, and the world distance
     *  along the ray; false when it does not
     */
    bool RayCastToPlane( const TRay& worldRay, TPoint2D& hit2D, T& distance ) const
    {
        TRayHit hit;
        if ( !this->RayCast( worldRay, TTraits::Max(), hit ) )
            return false;
        const TPoint local = this->ToLocal( hit.point );
        hit2D = TPoint2D( local.x, local.y );
        distance = hit.distance;
        return true;
    }

    /** The plane in world coordinates: through the world origin of the node, normal the world image of +Z (inverse transpose) */
    CTPlane< T > GetPlane( void ) const
    {
        return CTPlane< T >::FromPointNormal( this->GetWorldPosition(), this->NormalToWorld( TDirection::UnitZ() ) );
    }

    virtual TGeoShapeType GetShapeType( void ) const GUCEF_VIRTUAL_OVERRIDE { return GEOSHAPE_PLANE; }

    virtual void GetLocalBounds( TBox& bounds ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_bounded )
            bounds = TBase::GetUnboundedBox();
        else
        if ( m_extents.IsEmpty() )
            bounds = TBox();
        else
            bounds = TBox( TPoint( m_extents.min.x, m_extents.min.y, TTraits::Zero() ), TPoint( m_extents.max.x, m_extents.max.y, TTraits::Zero() ) );
    }

    /** The support of the extents (the local origin for an unbounded plane, which has no support) */
    virtual TPoint GetLocalSupport( const TVector& localDirection ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( !m_bounded || m_extents.IsEmpty() )
            return TPoint();
        const TPoint2D p = m_extents.GetSupport( MATH::CTVector2D< T >( localDirection.x, localDirection.y ) );
        return TPoint( p.x, p.y, TTraits::Zero() );
    }

    virtual bool LocalRayCast( const TRay& localRay, const T& maxDistance, TRayHit& hit ) const GUCEF_VIRTUAL_OVERRIDE
    {
        T distance = TTraits::Zero();
        TPoint point;
        if ( !TBase::IntersectLocalXYPlane( localRay, maxDistance, distance, point ) || ( m_bounded && !m_extents.Contains( TPoint2D( point.x, point.y ) ) ) )
            return false;
        TBase::SetFlatHit( localRay, distance, point, hit );
        return true;
    }

    virtual TPoint GetLocalClosestPoint( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE
    {
        TPoint2D p( localPoint.x, localPoint.y );
        if ( m_bounded && !m_extents.IsEmpty() )
            p = m_extents.ClosestPoint( p );
        return TPoint( p.x, p.y, TTraits::Zero() );
    }

    virtual bool LocalContains( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return TBase::IsOnLocalXYPlane( localPoint ) && ( !m_bounded || m_extents.Contains( TPoint2D( localPoint.x, localPoint.y ) ) );
    }

    virtual bool IsConvex( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    /** Bounded once extents are set (empty extents: bounded to nothing, see IsEmpty) */
    virtual bool IsBounded( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_bounded; }

    /** Empty when bounded to an empty rectangle: no points */
    virtual bool IsEmpty( void ) const GUCEF_VIRTUAL_OVERRIDE { return m_bounded && m_extents.IsEmpty(); }

    virtual bool IsFlat( void ) const GUCEF_VIRTUAL_OVERRIDE { return true; }

    protected:

    /** The unbounded plane: the orthogonal projection on the world plane */
    virtual TPoint ComputeClosestPointUnderAffine( const TPoint& worldPoint ) const GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_bounded )
            return TBase::ComputeClosestPointUnderAffine( worldPoint );
        return GetPlane().Project( worldPoint );
    }

    private:

    TRectangle m_extents;
    bool m_bounded;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoPlane< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoPlane< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOPLANE_H ? */
