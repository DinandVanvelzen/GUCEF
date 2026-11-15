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

#ifndef GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H
#define GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#include "gucefGEOMETRY_CTGeoObjectBase.h"
#define GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECTBASE_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#include "gucefGEOMETRY_CTTriangleMesh.h"
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */

#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#include "gucefGEOMETRY_CTMeshBvh.h"
#define GUCEF_GEOMETRY_CTMESHBVH_H
#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */

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
 *  A triangle mesh linked into a CTGeoSpace tree (sections 7.4.2 and 7.7 of the design): the mesh and its hierarchy
 *  are shared through CORE::CTSharedPtr, so many placed instances use one mesh and one BVH. The mesh is given in the
 *  node's local coordinates.
 *
 *  - Ray casts go through the BVH in local coordinates (exact under any affine placement, see CTGeoObjectBase); the hit
 *    carries the triangle index as featureIndex, its barycentrics and RAYHIT_BACKFACE for a hit from behind (as seen in
 *    local coordinates).
 *  - Surface or solid: by default the mesh is a surface (a room seen from inside, terrain, a picking mesh): Contains is
 *    true on the surface only, ClosestPoint is on the surface, a ray never starts inside. SetSolid( true ) treats a
 *    closed, consistently oriented mesh as the solid it bounds (Contains by the sign of the BVH signed distance, a ray
 *    from inside reports RAYHIT_STARTED_INSIDE, ClosestPoint returns an inside point itself).
 *  - Local support is the vertex furthest along the direction (the support of the convex hull); the shape is not convex,
 *    so the pair queries of section 7.4.4 do not offer it (it is dispatched as a user shape, GEOSHAPE_UNKNOWN).
 *    AsTriangleMesh returns the object, so CTRayCaster sweeps and overlaps it triangle by triangle however it was
 *    registered.
 *  - After the shared mesh changes: Refit (or Build) the shared BVH, then call OnMeshChanged on every object that uses
 *    it, which invalidates the world bounds and tells the structures (ray casters refit it in their Update).
 *  - Kinds: Float32 and Float64 are exported. Owner-thread only like every linked object; const ray casts may run
 *    concurrently once the world caches are valid (see CTRayCaster).
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoTriangleMesh : public CTGeoObjectBase< T >
{
    public:

    typedef CTGeoObjectBase< T > TBase;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTTransform3D< T > TTransform;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTRay3D< T > TRay;
    typedef SRayCastResult< T > TRayHit;
    typedef CTTriangleMesh< T > TMesh;
    typedef CTMeshBvh< T > TBvh;
    typedef CORE::CTSharedPtr< TMesh, MT::CMutex > TMeshPtr;
    typedef CORE::CTSharedPtr< TBvh, MT::CMutex > TBvhPtr;

    /** No mesh (an empty shape) */
    CTGeoTriangleMesh( void );

    /**
     *  Shares 'mesh' and its hierarchy 'bvh' when 'bvh' is built over that mesh. Otherwise (not built, built over
     *  another mesh, or null) 'bvh' is left untouched and the object builds a private hierarchy over 'mesh' (GetBvh
     *  returns it): a shared hierarchy is never rebuilt in place under the objects that use it.
     */
    CTGeoTriangleMesh( const TMeshPtr& mesh, const TBvhPtr& bvh );

    CTGeoTriangleMesh( const TMeshPtr& mesh, const TBvhPtr& bvh, const TTransform& localTransform );

    /** A private copy of 'mesh' with its own hierarchy, built here */
    explicit CTGeoTriangleMesh( const TMesh& mesh );

    virtual ~CTGeoTriangleMesh();

    /** Replaces the shared mesh and hierarchy (as the constructor: a private hierarchy when 'bvh' is not built over 'mesh'); bounds invalidated, structures told */
    void SetMesh( const TMeshPtr& mesh, const TBvhPtr& bvh );

    GUCEF_GEOMETRY_FORCE_INLINE const TMeshPtr& GetMesh( void ) const { return m_mesh; }
    GUCEF_GEOMETRY_FORCE_INLINE const TBvhPtr& GetBvh( void ) const { return m_bvh; }

    /** The shared mesh or its hierarchy changed (vertices moved and the BVH refitted): bounds invalidated, structures told */
    void OnMeshChanged( void );

    /** Surface (false, the default) or solid (see the class comment) */
    void SetSolid( bool solid );
    GUCEF_GEOMETRY_FORCE_INLINE bool IsSolid( void ) const { return m_solid; }

    virtual TGeoShapeType GetShapeType( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual void GetLocalBounds( TBox& bounds ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TPoint GetLocalSupport( const TVector& localDirection ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LocalRayCast( const TRay& localRay, const T& maxDistance, TRayHit& hit ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual TPoint GetLocalClosestPoint( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LocalContains( const TPoint& localPoint ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsConvex( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsEmpty( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** This object: the ray caster sweeps and overlaps it triangle by triangle however it was registered */
    virtual const CTGeoTriangleMesh* AsTriangleMesh( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CTGeoTriangleMesh( const CTGeoTriangleMesh& );
    CTGeoTriangleMesh& operator=( const CTGeoTriangleMesh& );

    void Attach( const TMeshPtr& mesh, const TBvhPtr& bvh );

    TMeshPtr m_mesh;
    TBvhPtr m_bvh;
    const TMesh* m_meshRaw;         /**< cached pointers: queries do not touch the shared pointers' locks */
    const TBvh* m_bvhRaw;
    bool m_solid;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTGeoTriangleMesh< T >::CTGeoTriangleMesh( void )
    : TBase(), m_mesh(), m_bvh(), m_meshRaw( GUCEF_NULL ), m_bvhRaw( GUCEF_NULL ), m_solid( false )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoTriangleMesh< T >::CTGeoTriangleMesh( const TMeshPtr& mesh, const TBvhPtr& bvh )
    : TBase(), m_mesh(), m_bvh(), m_meshRaw( GUCEF_NULL ), m_bvhRaw( GUCEF_NULL ), m_solid( false )
{
    Attach( mesh, bvh );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoTriangleMesh< T >::CTGeoTriangleMesh( const TMeshPtr& mesh, const TBvhPtr& bvh, const TTransform& localTransform )
    : TBase( localTransform ), m_mesh(), m_bvh(), m_meshRaw( GUCEF_NULL ), m_bvhRaw( GUCEF_NULL ), m_solid( false )
{
    Attach( mesh, bvh );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoTriangleMesh< T >::CTGeoTriangleMesh( const TMesh& mesh )
    : TBase(), m_mesh(), m_bvh(), m_meshRaw( GUCEF_NULL ), m_bvhRaw( GUCEF_NULL ), m_solid( false )
{
    Attach( TMeshPtr( GUCEF_NEW TMesh( mesh ) ), TBvhPtr( GUCEF_NEW TBvh() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoTriangleMesh< T >::~CTGeoTriangleMesh()
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoTriangleMesh< T >::Attach( const TMeshPtr& mesh, const TBvhPtr& bvh )
{
    m_mesh = mesh;
    m_bvh = bvh;
    if ( m_mesh.IsNULL() )
        m_bvh = TBvhPtr();
    m_meshRaw = m_mesh.GetPointerAlways();
    if ( GUCEF_NULL != m_meshRaw && ( m_bvh.IsNULL() || m_bvh->GetMesh() != m_meshRaw ) )
    {
        /*
         *  Not built over this mesh (not built, or built over another mesh): the given hierarchy may be shared with
         *  objects that use it for their own mesh, so it is never rebuilt in place; this object builds a private one
         */
        m_bvh = TBvhPtr( GUCEF_NEW TBvh() );
        m_bvh->Build( *m_meshRaw );
    }
    m_bvhRaw = m_bvh.GetPointerAlways();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoTriangleMesh< T >::SetMesh( const TMeshPtr& mesh, const TBvhPtr& bvh )
{
    Attach( mesh, bvh );
    this->OnShapeChanged();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoTriangleMesh< T >::OnMeshChanged( void )
{
    this->OnShapeChanged();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoTriangleMesh< T >::SetSolid( bool solid )
{
    m_solid = solid;
    this->OnShapeChanged();
}

/*-------------------------------------------------------------------------*/

template < typename T >
TGeoShapeType
CTGeoTriangleMesh< T >::GetShapeType( void ) const
{
    return GEOSHAPE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoTriangleMesh< T >::IsConvex( void ) const
{
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
const CTGeoTriangleMesh< T >*
CTGeoTriangleMesh< T >::AsTriangleMesh( void ) const
{
    return this;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoTriangleMesh< T >::IsEmpty( void ) const
{
    return GUCEF_NULL == m_meshRaw || m_meshRaw->IsEmpty() || 0 == m_bvhRaw->GetNodeCount();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoTriangleMesh< T >::GetLocalBounds( TBox& bounds ) const
{
    bounds = GUCEF_NULL == m_bvhRaw ? TBox() : m_bvhRaw->GetBounds();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoTriangleMesh< T >::TPoint
CTGeoTriangleMesh< T >::GetLocalSupport( const TVector& localDirection ) const
{
    if ( GUCEF_NULL == m_meshRaw || m_meshRaw->vertices.empty() )
        return TPoint();
    const GUCEF::vector< TPoint >& vertices = m_meshRaw->vertices;
    UInt32 best = 0;
    T bestDot = vertices[ 0 ].ToPositionVector().Dot( localDirection );
    for ( UInt32 v=1; v<(UInt32) vertices.size(); ++v )
    {
        const T dot = vertices[ v ].ToPositionVector().Dot( localDirection );
        if ( bestDot < dot )
        {
            bestDot = dot;
            best = v;
        }
    }
    return vertices[ best ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoTriangleMesh< T >::LocalRayCast( const TRay& localRay, const T& maxDistance, TRayHit& hit ) const
{
    if ( IsEmpty() )
        return false;
    if ( m_solid && LocalContains( localRay.origin ) )
    {
        TRayHit inside;
        inside.distance = TTraits::Zero();
        inside.point = localRay.origin;
        inside.normal = -localRay.direction.AsVector();
        inside.flags = RAYHIT_STARTED_INSIDE;
        hit = inside;
        return true;
    }
    return m_bvhRaw->RayCast( localRay, maxDistance, hit );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoTriangleMesh< T >::TPoint
CTGeoTriangleMesh< T >::GetLocalClosestPoint( const TPoint& localPoint ) const
{
    if ( IsEmpty() )
        return localPoint;
    SMeshClosestPoint< T > closest;
    if ( m_solid )
    {
        T signedDistance = TTraits::Zero();
        if ( !m_bvhRaw->GetSignedDistance( localPoint, TTraits::Max(), signedDistance, &closest ) )
            return localPoint;
        return TTraits::IsNegative( signedDistance ) ? localPoint : closest.point;
    }
    if ( !m_bvhRaw->GetClosestPoint( localPoint, TTraits::Max(), closest ) )
        return localPoint;
    return closest.point;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoTriangleMesh< T >::LocalContains( const TPoint& localPoint ) const
{
    if ( IsEmpty() )
        return false;
    SMeshClosestPoint< T > closest;
    if ( m_solid )
    {
        T signedDistance = TTraits::Zero();
        return m_bvhRaw->GetSignedDistance( localPoint, TTraits::Max(), signedDistance, &closest ) && !( TTraits::Zero() < signedDistance );
    }
    return m_bvhRaw->GetClosestPoint( localPoint, TTraits::Zero(), closest );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTGeoTriangleMesh< Float32 > CGeoTriangleMesh;
typedef CTGeoTriangleMesh< Float64 > CGeoTriangleMesh64;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoTriangleMesh< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoTriangleMesh< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H ? */
