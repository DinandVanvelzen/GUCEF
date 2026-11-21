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

#ifndef GUCEF_GEOMETRY_CTGEOOBJECTBASE_H
#define GUCEF_GEOMETRY_CTGEOOBJECTBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTGEOSPACE_H
#include "gucefGEOMETRY_CTGeoSpace.h"
#define GUCEF_GEOMETRY_CTGEOSPACE_H
#endif /* GUCEF_GEOMETRY_CTGEOSPACE_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOOBJECTSTRUCTURE_H
#include "gucefGEOMETRY_CTGeoObjectStructure.h"
#define GUCEF_GEOMETRY_CTGEOOBJECTSTRUCTURE_H
#endif /* GUCEF_GEOMETRY_CTGEOOBJECTSTRUCTURE_H ? */

#ifndef GUCEF_GEOMETRY_CTGJK_H
#include "gucefGEOMETRY_CTGjk.h"
#define GUCEF_GEOMETRY_CTGJK_H
#endif /* GUCEF_GEOMETRY_CTGJK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

template < typename T > class CTGeoTriangleMesh;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  A shape that lives in a CTGeoSpace node (section 7.4.1 of the design): the shape is given in the node's local
 *  coordinates and follows every move of the node and of its ancestors. Abstract; CTGeoObject< TShape >,
 *  CTGeoObject2D< TShape2D > and CTGeoPlane< T > are the implementations, and a user class may derive from it for its
 *  own shape (GetShapeType GEOSHAPE_UNKNOWN: pair queries then use its support mapping when it is convex).
 *
 *  - Local primitives (virtual, one call per object per query): bounds, support, ray cast, closest point, containment.
 *  - World members with the uniform names of section 2.1 (Contains, ClosestPoint, GetDistance, RayCast), so the point
 *    and ray facade (Distance( point, object ), Intersects( ray, object ), ...) answers for linked objects as well.
 *    Ray casts and containment are exact under any affine placement; closest points and distances are exact under a
 *    similarity placement and, for convex shapes, under any affine placement (GJK on the placed support mapping);
 *    a non-convex flat shape under a non-uniform in-plane scale gives the image of the local closest point.
 *  - World bounds are cached with the world matrix: exact (support along the six axes) for bounded convex shapes, the
 *    transformed local bounds otherwise, everything for unbounded shapes.
 *  - Structures (CTGeoObjectStructure) register with the object and are told once per change batch when it moved
 *    (section 7.4.3); a destroyed object removes itself from them.
 *  - Layer mask (default all layers), user data pointer and user id for the users of structures and ray casters.
 *  - Empty shapes (an empty box, a negative radius, empty extents) have no points: no ray hits, no containment,
 *    empty world bounds, the largest distance, and no pair queries (not offered).
 *  - The pair dispatch casts by the library tag that only CTGeoObject, CTGeoObject2D and CTGeoPlane set; a user class
 *    is dispatched as a user shape whatever GetShapeType returns (debug builds assert that it does not claim a library
 *    shape type).
 *  - Kinds: Float32 and Float64 only (exported). Linked objects are the scene-side view of geometry (rendering, audio,
 *    picking, gameplay queries): deterministic fixed-point simulation (gucefPHYSICS) keeps its colliders as proxies of
 *    its own broadphase rather than as nodes, so no fixed-point linked objects exist; the transform tree itself is
 *    instantiated for TFixed32_32. Owner-thread only (section 7.3.7), not realtime-safe.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoObjectBase : public CTGeoSpace< T >
{
    public:

    typedef CTGeoSpace< T > TGeoSpace;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTTransform3D< T > TTransform;
    typedef MATH::CTMatrix4D< T > TMatrix;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTRay3D< T > TRay;
    typedef SRayCastResult< T > TRayHit;
    typedef CTGeoObjectStructure< T > TStructure;

    /** The local support mapping of an object as a support mapping type (section 7.6.1): one virtual call per support */
    class CLocalSupport
    {
        public:

        typedef T TValue;
        typedef MATH::CTPoint3D< T > TPoint;
        typedef MATH::CTVector3D< T > TVector;

        explicit CLocalSupport( const CTGeoObjectBase& object ) : m_object( &object ), m_piece( WholeShape() ) {}

        /** The support of one convex piece of a non-convex object (see GetConvexPieceCount) */
        CLocalSupport( const CTGeoObjectBase& object, UInt32 piece ) : m_object( &object ), m_piece( piece ) {}

        /** The piece index that stands for the whole shape */
        static GUCEF_GEOMETRY_FORCE_INLINE UInt32 WholeShape( void ) { return 0xFFFFFFFFU; }

        TPoint GetSupport( const TVector& direction ) const
        {
            return WholeShape() == m_piece ? m_object->GetLocalSupport( direction ) : m_object->GetLocalPieceSupport( m_piece, direction );
        }
        T GetMargin( void ) const { return MATH::CTNumericTraits< T >::Zero(); }
        TPoint GetAnyPoint( void ) const { return GetSupport( TVector::UnitX() ); }

        private:

        const CTGeoObjectBase* m_object;
        UInt32 m_piece;
    };

    /** The support mapping of the object placed by a matrix (the world matrix, or one relative to an origin) */
    typedef CTTransformedSupport< CLocalSupport > TPlacedSupport;

    /** Removes the object from every structure it is registered in */
    virtual ~CTGeoObjectBase();

    /*---------------------------------------------------------------------*/
    /*  Local primitives                                                    */
    /*---------------------------------------------------------------------*/

    /** The shape type: indexes the pair dispatch table; GEOSHAPE_UNKNOWN for user shapes */
    virtual TGeoShapeType GetShapeType( void ) const = 0;

    /** The axis aligned bounds in local coordinates (everything for an unbounded shape) */
    virtual void GetLocalBounds( TBox& bounds ) const = 0;

    /** A point of the shape furthest along 'localDirection' (convex shapes; others: the support of their convex hull) */
    virtual TPoint GetLocalSupport( const TVector& localDirection ) const = 0;

    /**
     *  The number of convex pieces a non-convex object is answered through in the pair queries (default 0: none). A
     *  non-convex polygon returns the triangles of its cached triangulation; convex objects return 0 (they are their
     *  own piece). The pair queries then answer as the minimum, any or deepest over the pieces.
     */
    virtual UInt32 GetConvexPieceCount( void ) const;

    /** A point of convex piece 'piece' furthest along 'localDirection' (default: the support of the whole shape) */
    virtual TPoint GetLocalPieceSupport( UInt32 piece, const TVector& localDirection ) const;

    /** The first hit of a ray given in local coordinates within 'maxDistance' (local units) */
    virtual bool LocalRayCast( const TRay& localRay, const T& maxDistance, TRayHit& hit ) const = 0;

    /** The point of the shape nearest to a local point (in local units; the point itself inside a solid) */
    virtual TPoint GetLocalClosestPoint( const TPoint& localPoint ) const = 0;

    /** True when the shape contains the local point */
    virtual bool LocalContains( const TPoint& localPoint ) const = 0;

    /** True when the shape is convex (GJK and EPA apply) */
    virtual bool IsConvex( void ) const = 0;

    /** True when the shape is bounded (default); an unbounded plane or plane-bounded volume is not */
    virtual bool IsBounded( void ) const;

    /** True when the shape has no points (default false): see the class comment */
    virtual bool IsEmpty( void ) const;

    /** True for shapes in the local XY plane (2D objects and planes; default false) */
    virtual bool IsFlat( void ) const;

    /**
     *  The triangle mesh object this is, or GUCEF_NULL (default): CTGeoTriangleMesh returns itself. Lets the queries
     *  that handle meshes triangle by triangle (the sweeps and overlaps of CTRayCaster) find a mesh whatever type it was
     *  registered through; the convex pair queries of section 7.4.4 do not use it.
     */
    virtual const CTGeoTriangleMesh< T >* AsTriangleMesh( void ) const;

    /** The shape type the pair dispatch uses: the library tag of the implementation class, GEOSHAPE_UNKNOWN for user classes */
    TGeoShapeType GetDispatchShapeType( void ) const;

    /*---------------------------------------------------------------------*/
    /*  World members                                                       */
    /*---------------------------------------------------------------------*/

    /** The world bounds (cached with the world matrix; see the class comment) */
    const TBox& GetWorldBounds( void ) const;

    GUCEF_GEOMETRY_FORCE_INLINE void GetWorldBounds( TBox& bounds ) const { bounds = GetWorldBounds(); }

    /**
     *  The first hit of a world ray within 'maxDistance' (world units): the ray is mapped into local coordinates, so the
     *  hit is exact under any affine placement; the normal is the world normal (inverse transpose), a ray starting
     *  inside reports distance 0 and the normal opposite to the ray direction
     */
    bool RayCast( const TRay& worldRay, const T& maxDistance, TRayHit& hit ) const;

    /** The point of the shape nearest to a world point (the point itself when contained, and for an empty shape) */
    TPoint ClosestPoint( const TPoint& worldPoint ) const;

    /** The world distance from the point to the shape, 0 when contained, the largest number of the kind for an empty shape */
    T GetDistance( const TPoint& worldPoint ) const;

    T GetSquaredDistance( const TPoint& worldPoint ) const;

    /** True when the shape contains the world point (exact under any affine placement) */
    GUCEF_GEOMETRY_FORCE_INLINE bool Contains( const TPoint& worldPoint ) const { return !IsEmpty() && LocalContains( this->ToLocal( worldPoint ) ); }

    /** A world point of the shape furthest along the world direction: W * support( W^T d ) */
    TPoint GetWorldSupport( const TVector& worldDirection ) const;

    /** The support mapping of the shape placed by its world matrix moved by -origin (exact for any affine placement) */
    TPlacedSupport GetPlacedSupport( const TVector& origin ) const;

    /** The same for convex piece 'piece' of a non-convex object (see GetConvexPieceCount) */
    TPlacedSupport GetPlacedSupport( const TVector& origin, UInt32 piece ) const;

    /*---------------------------------------------------------------------*/
    /*  Filtering and user data                                             */
    /*---------------------------------------------------------------------*/

    GUCEF_GEOMETRY_FORCE_INLINE void SetLayerMask( UInt32 mask ) { m_layerMask = mask; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetLayerMask( void ) const { return m_layerMask; }
    GUCEF_GEOMETRY_FORCE_INLINE void SetUserData( void* userData ) { m_userData = userData; }
    GUCEF_GEOMETRY_FORCE_INLINE void* GetUserData( void ) const { return m_userData; }
    GUCEF_GEOMETRY_FORCE_INLINE void SetUserId( UInt64 id ) { m_userId = id; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt64 GetUserId( void ) const { return m_userId; }

    /*---------------------------------------------------------------------*/
    /*  Structures (section 7.4.3)                                          */
    /*---------------------------------------------------------------------*/

    /**
     *  Registers a structure that holds this object (called by the structure when it inserts the object; allocates when
     *  the link array grows). The world matrix is brought up to date so that the next move is reported. False when the
     *  structure is registered already.
     */
    bool RegisterStructure( TStructure& structure );

    /** Removes the link to a structure; false when it was not registered */
    bool UnregisterStructure( TStructure& structure );

    /** True when the structure is registered with this object */
    bool IsRegisteredWith( const TStructure& structure ) const;

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetStructureCount( void ) const { return (UInt32) m_structures.size(); }

    /**
     *  The structure has processed the move of this object: the next move is reported again. Returns the world bounds,
     *  which the structure typically refits with (reading them re-arms the notification of the world cache).
     */
    const TBox& AcknowledgeMove( const TStructure& structure );

    /** True while a move has been reported to the structure and not acknowledged */
    bool IsMoveQueued( const TStructure& structure ) const;

    protected:

    CTGeoObjectBase( void );
    explicit CTGeoObjectBase( const TTransform& localTransform );

    /** For the library implementations only: the tag that lets the pair dispatch cast to the implementation class */
    GUCEF_GEOMETRY_FORCE_INLINE void SetLibraryShapeType( TGeoShapeType shapeType ) { m_libraryShapeType = (UInt8) shapeType; }

    /** Reports the move to the registered structures; overrides must call this base */
    virtual void OnWorldPlacementInvalidated( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** For implementations: the shape changed; invalidates the world bounds and reports the change to the structures */
    void OnShapeChanged( void );

    /** The world bounds for 'world' (see the class comment); implementations may refine */
    virtual void ComputeWorldBounds( const TMatrix& world, TBox& bounds ) const;

    /**
     *  The closest point for a shape that is not bounded and convex under a placement that is not a similarity; the
     *  default maps the local closest point (exact for solids that contain the point, an approximation otherwise)
     */
    virtual TPoint ComputeClosestPointUnderAffine( const TPoint& worldPoint ) const;

    /**
     *  For flat shapes in the local XY plane: where a local ray meets z = 0 within maxDistance. A ray that starts in the
     *  plane meets it at 0 (also when it runs along it); a ray parallel to and off the plane, or pointing away, does not.
     */
    static bool IntersectLocalXYPlane( const TRay& localRay, const T& maxDistance, T& distance, TPoint& point );

    /** For flat shapes: fills a hit at 'distance' and 'point' on the XY plane, the normal +Z or -Z facing the ray */
    static void SetFlatHit( const TRay& localRay, const T& distance, const TPoint& point, TRayHit& hit );

    /** For flat shapes: true when the local z of a point is zero within the relative tolerance of the kind */
    static bool IsOnLocalXYPlane( const TPoint& localPoint );

    /** The box that contains everything (the bounds of unbounded shapes) */
    static TBox GetUnboundedBox( void );

    private:

    struct SStructureLink
    {
        TStructure* structure;
        UInt32 slot;                /**< the index of this object in the structure's object list */
        bool queued;
    };

    void NotifyStructures( void );
    void RemoveLink( UInt32 index );

    GUCEF::vector< SStructureLink > m_structures;
    mutable TBox m_worldBounds;
    void* m_userData;
    UInt64 m_userId;
    UInt32 m_layerMask;
    UInt8 m_libraryShapeType;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTGeoObjectBase< T >::CTGeoObjectBase( void )
    : TGeoSpace(), m_structures(), m_worldBounds(), m_userData( GUCEF_NULL ), m_userId( 0 ), m_layerMask( 0xFFFFFFFF ), m_libraryShapeType( GEOSHAPE_UNKNOWN )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoObjectBase< T >::CTGeoObjectBase( const TTransform& localTransform )
    : TGeoSpace( localTransform ), m_structures(), m_worldBounds(), m_userData( GUCEF_NULL ), m_userId( 0 ), m_layerMask( 0xFFFFFFFF ), m_libraryShapeType( GEOSHAPE_UNKNOWN )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoObjectBase< T >::~CTGeoObjectBase()
{
    /* the link is removed before the structure is told, so that it may also unregister while being told */
    while ( !m_structures.empty() )
    {
        TStructure* structure = m_structures.back().structure;
        RemoveLink( (UInt32) m_structures.size() - 1 );
        structure->OnGeoObjectDestroyed( *this );
    }
}

/*-------------------------------------------------------------------------*/

/* removes link 'index' and this object from the structure's list (swap with its last entry, whose link learns the slot) */
template < typename T >
void
CTGeoObjectBase< T >::RemoveLink( UInt32 index )
{
    TStructure* structure = m_structures[ index ].structure;
    const UInt32 slot = m_structures[ index ].slot;
    GUCEF::vector< CTGeoObjectBase* >& objects = structure->m_objects;
    CTGeoObjectBase* last = objects.back();
    objects[ slot ] = last;
    objects.pop_back();
    if ( last != this )
    {
        for ( UInt32 i=0; i<(UInt32) last->m_structures.size(); ++i )
            if ( last->m_structures[ i ].structure == structure )
                last->m_structures[ i ].slot = slot;
    }
    m_structures.erase( m_structures.begin() + index );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoObjectStructure< T >::UnregisterAll( void )
{
    while ( !m_objects.empty() )
        m_objects.back()->UnregisterStructure( *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTGeoObjectBase< T >::GetConvexPieceCount( void ) const
{
    return 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTGeoObjectBase< T >::TPoint
CTGeoObjectBase< T >::GetLocalPieceSupport( UInt32 /*piece*/, const TVector& localDirection ) const
{
    return GetLocalSupport( localDirection );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::IsBounded( void ) const
{
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::IsEmpty( void ) const
{
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
const CTGeoTriangleMesh< T >*
CTGeoObjectBase< T >::AsTriangleMesh( void ) const
{
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::IsFlat( void ) const
{
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
TGeoShapeType
CTGeoObjectBase< T >::GetDispatchShapeType( void ) const
{
    /* a user class must not claim a library shape type: the dispatch would cast it to the library class */
    GUCEF_GEOMETRY_ASSERT( GEOSHAPE_UNKNOWN != m_libraryShapeType || GEOSHAPE_UNKNOWN == GetShapeType() || (UInt32) GetShapeType() >= (UInt32) GEOSHAPE_COUNT );
    return (TGeoShapeType) m_libraryShapeType;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTGeoObjectBase< T >::TBox
CTGeoObjectBase< T >::GetUnboundedBox( void )
{
    const T big = TTraits::Max();
    return TBox( TPoint( -big, -big, -big ), TPoint( big, big, big ) );
}

/*-------------------------------------------------------------------------*/

/*
 *  A structure may unregister itself (or register another) while being told: after every call the scan restarts when
 *  the links changed. Links told already are queued, so nobody is told twice and nobody is skipped.
 */
template < typename T >
void
CTGeoObjectBase< T >::NotifyStructures( void )
{
    UInt32 i = 0;
    while ( i < (UInt32) m_structures.size() )
    {
        if ( m_structures[ i ].queued )
        {
            ++i;
            continue;
        }
        m_structures[ i ].queued = true;
        TStructure* structure = m_structures[ i ].structure;
        const UInt32 count = (UInt32) m_structures.size();
        structure->OnGeoObjectMoved( *this );
        if ( count != (UInt32) m_structures.size() || i >= (UInt32) m_structures.size() || m_structures[ i ].structure != structure )
            i = 0;
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoObjectBase< T >::OnWorldPlacementInvalidated( void )
{
    NotifyStructures();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoObjectBase< T >::OnShapeChanged( void )
{
    this->SetWorldDerivedCacheValid( false );
    NotifyStructures();
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::RegisterStructure( TStructure& structure )
{
    if ( IsRegisteredWith( structure ) )
        return false;
    SStructureLink link;
    link.structure = &structure;
    link.slot = (UInt32) structure.m_objects.size();
    link.queued = false;
    m_structures.push_back( link );
    structure.m_objects.push_back( this );
    GetWorldBounds();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::UnregisterStructure( TStructure& structure )
{
    for ( UInt32 i=0; i<(UInt32) m_structures.size(); ++i )
    {
        if ( m_structures[ i ].structure == &structure )
        {
            RemoveLink( i );
            return true;
        }
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::IsRegisteredWith( const TStructure& structure ) const
{
    for ( UInt32 i=0; i<(UInt32) m_structures.size(); ++i )
        if ( m_structures[ i ].structure == &structure )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::IsMoveQueued( const TStructure& structure ) const
{
    for ( UInt32 i=0; i<(UInt32) m_structures.size(); ++i )
        if ( m_structures[ i ].structure == &structure )
            return m_structures[ i ].queued;
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
const typename CTGeoObjectBase< T >::TBox&
CTGeoObjectBase< T >::AcknowledgeMove( const TStructure& structure )
{
    for ( UInt32 i=0; i<(UInt32) m_structures.size(); ++i )
        if ( m_structures[ i ].structure == &structure )
            m_structures[ i ].queued = false;
    return GetWorldBounds();
}

/*-------------------------------------------------------------------------*/

template < typename T >
const typename CTGeoObjectBase< T >::TBox&
CTGeoObjectBase< T >::GetWorldBounds( void ) const
{
    if ( !this->IsWorldDerivedCacheValid() )
    {
        ComputeWorldBounds( this->GetWorldMatrix(), m_worldBounds );
        this->SetWorldDerivedCacheValid( true );
    }
    return m_worldBounds;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoObjectBase< T >::ComputeWorldBounds( const TMatrix& world, TBox& bounds ) const
{
    if ( IsEmpty() )
    {
        bounds = TBox();
        return;
    }
    if ( !IsBounded() )
    {
        bounds = GetUnboundedBox();
        return;
    }
    if ( IsConvex() )
    {
        bounds = GetSupportBounds( TPlacedSupport( CLocalSupport( *this ), world.GetLinear(), world.GetTranslation() ) );
        return;
    }
    TBox local;
    GetLocalBounds( local );
    bounds = local.Transformed( world );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoObjectBase< T >::TPlacedSupport
CTGeoObjectBase< T >::GetPlacedSupport( const TVector& origin ) const
{
    const TMatrix& world = this->GetWorldMatrix();
    return TPlacedSupport( CLocalSupport( *this ), world.GetLinear(), world.GetTranslation() - origin );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoObjectBase< T >::TPlacedSupport
CTGeoObjectBase< T >::GetPlacedSupport( const TVector& origin, UInt32 piece ) const
{
    const TMatrix& world = this->GetWorldMatrix();
    return TPlacedSupport( CLocalSupport( *this, piece ), world.GetLinear(), world.GetTranslation() - origin );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoObjectBase< T >::TPoint
CTGeoObjectBase< T >::GetWorldSupport( const TVector& worldDirection ) const
{
    const TMatrix& world = this->GetWorldMatrix();
    return world.TransformPoint( GetLocalSupport( world.GetLinear().Transposed() * worldDirection ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectBase< T >::RayCast( const TRay& worldRay, const T& maxDistance, TRayHit& hit ) const
{
    if ( IsEmpty() )
        return false;
    const TMatrix& inverse = this->GetInverseWorldMatrix();
    const TVector localVector = inverse.TransformVector( worldRay.direction.AsVector() );
    const T length = localVector.GetLength();
    TDirection localDirection;
    if ( !( TTraits::Zero() < length ) || !TTraits::IsFinite( length ) || !TDirection::TryCreate( localVector, localDirection ) )
        return false;
    /* a ray parameter scales by the length of the mapped direction under any affine map */
    T localMax = TTraits::Max();
    if ( !( TTraits::One() < length ) || maxDistance < TTraits::Max() / length )
        localMax = maxDistance * length;
    TRayHit local;
    if ( !LocalRayCast( TRay( inverse.TransformPoint( worldRay.origin ), localDirection ), localMax, local ) )
        return false;
    hit = local;
    hit.distance = local.distance / length;
    hit.point = this->GetWorldMatrix().TransformPoint( local.point );
    if ( local.StartedInside() )
    {
        hit.normal = -worldRay.direction.AsVector();
    }
    else
    {
        TDirection normal;
        if ( TDirection::TryCreate( inverse.GetLinear().Transposed() * local.normal, normal ) )
            hit.normal = normal.AsVector();
        else
            hit.normal = -worldRay.direction.AsVector();
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoObjectBase< T >::TPoint
CTGeoObjectBase< T >::ClosestPoint( const TPoint& worldPoint ) const
{
    if ( IsEmpty() )
        return worldPoint;
    if ( this->IsWorldTransformSimilarity() )
        return this->ToWorld( GetLocalClosestPoint( this->ToLocal( worldPoint ) ) );
    if ( !IsConvex() || !IsBounded() )
        return ComputeClosestPointUnderAffine( worldPoint );
    if ( LocalContains( this->ToLocal( worldPoint ) ) )
        return worldPoint;
    /* a convex shape under a non-similarity: GJK of the point against the placed support, relative to the placement origin */
    const TVector origin = this->GetWorldMatrix().GetTranslation();
    SGjkResult< T > result;
    CTGjk< T >::ComputeDistance( CTPointSupport< T >( TPoint( worldPoint.ToPositionVector() - origin ) ), GetPlacedSupport( origin ), CTGeometryTolerance< T >(),
                                 GUCEF_NULL, result );
    return result.pointOnB + origin;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoObjectBase< T >::IntersectLocalXYPlane( const TRay& localRay, const T& maxDistance, T& distance, TPoint& point )
{
    const T zero = TTraits::Zero();
    const T dz = localRay.direction.GetZ();
    if ( zero == localRay.origin.z )
    {
        distance = zero;
        point = localRay.origin;
        return true;
    }
    if ( zero == dz || TTraits::IsNegative( localRay.origin.z ) == TTraits::IsNegative( dz ) )
        return false;
    const T t = MATH::ComponentNegate( localRay.origin.z ) / dz;
    if ( maxDistance < t || !TTraits::IsFinite( t ) )
        return false;
    distance = t;
    point = localRay.GetPoint( t );
    point.z = zero;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoObjectBase< T >::SetFlatHit( const TRay& localRay, const T& distance, const TPoint& point, TRayHit& hit )
{
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    hit = TRayHit();
    hit.distance = distance;
    hit.point = point;
    hit.normal = TVector( zero, zero, TTraits::IsNegative( localRay.direction.GetZ() ) ? one : MATH::ComponentNegate( one ) );
    if ( zero == localRay.direction.GetZ() )
    {
        hit.normal = -localRay.direction.AsVector();
        hit.flags = RAYHIT_STARTED_INSIDE;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoObjectBase< T >::IsOnLocalXYPlane( const TPoint& localPoint )
{
    const T scale = TTraits::One() + TTraits::Abs( localPoint.x ) + TTraits::Abs( localPoint.y );
    return !( TTraits::DefaultRelativeTolerance() * scale < TTraits::Abs( localPoint.z ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoObjectBase< T >::TPoint
CTGeoObjectBase< T >::ComputeClosestPointUnderAffine( const TPoint& worldPoint ) const
{
    return this->ToWorld( GetLocalClosestPoint( this->ToLocal( worldPoint ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTGeoObjectBase< T >::GetSquaredDistance( const TPoint& worldPoint ) const
{
    if ( IsEmpty() )
        return TTraits::Max();
    return ( ClosestPoint( worldPoint ) - worldPoint ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTGeoObjectBase< T >::GetDistance( const TPoint& worldPoint ) const
{
    if ( IsEmpty() )
        return TTraits::Max();
    return ( ClosestPoint( worldPoint ) - worldPoint ).GetLength();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoObjectBase< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoObjectBase< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoObjectStructure< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoObjectStructure< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOOBJECTBASE_H ? */
