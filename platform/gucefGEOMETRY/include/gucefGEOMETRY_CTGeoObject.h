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

#ifndef GUCEF_GEOMETRY_CTGEOOBJECT_H
#define GUCEF_GEOMETRY_CTGEOOBJECT_H

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
 *  One 3D value shape linked into a CTGeoSpace tree (section 7.4.2 of the design): the shape is held in the node's
 *  local coordinates; the local primitives of CTGeoObjectBase come from CTShapeTraits< TShape >, so one template serves
 *  every shape (sphere, axis aligned box, oriented box, capsule, cylinder, cone, triangle, convex hull, plane-bounded
 *  volume). An axis aligned box is axis aligned in its own space and oriented in the world.
 *
 *  - SetShape replaces the shape, invalidates the world bounds and reports the change to the registered structures.
 *  - Kinds: Float32 and Float64 instantiations of the listed shapes are exported (typedefs in gucefGEOMETRY_geospaces.h).
 */
template < typename TShape >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoObject : public CTGeoObjectBase< typename TShape::TValue >
{
    public:

    typedef typename TShape::TValue TKind;
    typedef CTGeoObjectBase< TKind > TBase;
    typedef TShape TShapeType;
    typedef CTShapeTraits< TShape > TShapeTraits;
    typedef typename TShapeTraits::TCache TShapeCache;
    typedef MATH::CTTransform3D< TKind > TTransform;
    typedef MATH::CTPoint3D< TKind > TPoint;
    typedef MATH::CTVector3D< TKind > TVector;
    typedef CTAxisAlignedBox3D< TKind > TBox;
    typedef CTRay3D< TKind > TRay;
    typedef SRayCastResult< TKind > TRayHit;

    GUCEF_GEOMETRY_STATIC_ASSERT( 3 == TShapeTraits::Dimensions, CTGeoObject_requires_a_3D_shape_use_CTGeoObject2D_for_2D_shapes );

    /** The default shape at the identity placement */
    CTGeoObject( void )
        : TBase(), m_shape(), m_cache()
    {
        TShapeTraits::BuildCache( m_shape, m_cache );
        this->SetLibraryShapeType( TShapeTraits::GetShapeType() );
    }

    explicit CTGeoObject( const TShape& shape )
        : TBase(), m_shape( shape ), m_cache()
    {
        TShapeTraits::BuildCache( m_shape, m_cache );
        this->SetLibraryShapeType( TShapeTraits::GetShapeType() );
    }

    CTGeoObject( const TShape& shape, const TTransform& localTransform )
        : TBase( localTransform ), m_shape( shape ), m_cache()
    {
        TShapeTraits::BuildCache( m_shape, m_cache );
        this->SetLibraryShapeType( TShapeTraits::GetShapeType() );
    }

    virtual ~CTGeoObject() {}

    /** The shape in local coordinates */
    GUCEF_GEOMETRY_FORCE_INLINE const TShape& GetShape( void ) const { return m_shape; }

    /** Replaces the shape (local coordinates): world bounds invalidated, structures told */
    void SetShape( const TShape& shape )
    {
        m_shape = shape;
        TShapeTraits::BuildCache( m_shape, m_cache );
        this->OnShapeChanged();
    }

    /** The per-object cache of the shape traits (the convex body of a plane-bounded volume) */
    GUCEF_GEOMETRY_FORCE_INLINE const TShapeCache& GetShapeCache( void ) const { return m_cache; }

    virtual TGeoShapeType GetShapeType( void ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::GetShapeType(); }
    virtual void GetLocalBounds( TBox& bounds ) const GUCEF_VIRTUAL_OVERRIDE { TShapeTraits::GetBounds( m_shape, m_cache, bounds ); }
    virtual TPoint GetLocalSupport( const TVector& localDirection ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::GetSupport( m_shape, m_cache, localDirection ); }
    virtual TPoint GetLocalClosestPoint( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::ClosestPoint( m_shape, m_cache, localPoint ); }
    virtual bool LocalContains( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::Contains( m_shape, m_cache, localPoint ); }
    virtual bool IsConvex( void ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::IsConvex( m_shape, m_cache ); }
    virtual bool IsBounded( void ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::IsBounded( m_shape, m_cache ); }
    virtual bool IsEmpty( void ) const GUCEF_VIRTUAL_OVERRIDE { return TShapeTraits::IsEmpty( m_shape, m_cache ); }

    virtual bool LocalRayCast( const TRay& localRay, const TKind& maxDistance, TRayHit& hit ) const GUCEF_VIRTUAL_OVERRIDE
    {
        return TShapeTraits::RayCast( m_shape, m_cache, localRay, maxDistance, hit );
    }

    private:

    TShape m_shape;
    TShapeCache m_cache;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

#define GUCEF_GEOMETRY_CTGEOOBJECT_KIND( macro, kind )                 \
    macro( CTGeoObject< CTSphere< kind > > )                            \
    macro( CTGeoObject< CTAxisAlignedBox3D< kind > > )                  \
    macro( CTGeoObject< CTOrientedBox3D< kind > > )                     \
    macro( CTGeoObject< CTCapsule< kind > > )                           \
    macro( CTGeoObject< CTCylinder< kind > > )                          \
    macro( CTGeoObject< CTCone< kind > > )                              \
    macro( CTGeoObject< CTTriangle3D< kind > > )                        \
    macro( CTGeoObject< CTConvexHull3D< kind > > )                      \
    macro( CTGeoObject< CTPlaneBoundedVolume< kind > > )

GUCEF_GEOMETRY_CTGEOOBJECT_KIND( GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS, Float32 )
GUCEF_GEOMETRY_CTGEOOBJECT_KIND( GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS, Float64 )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOOBJECT_H ? */
