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

#ifndef GUCEF_GEOMETRY_CTGEOSPACE_H
#define GUCEF_GEOMETRY_CTGEOSPACE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOSPACESNAPSHOT_H
#include "gucefGEOMETRY_CTGeoSpaceSnapshot.h"
#define GUCEF_GEOMETRY_CTGEOSPACESNAPSHOT_H
#endif /* GUCEF_GEOMETRY_CTGEOSPACESNAPSHOT_H ? */

#ifndef GUCEF_MATH_TRANSFORMS_H
#include "gucefMATH_transforms.h"
#define GUCEF_MATH_TRANSFORMS_H
#endif /* GUCEF_MATH_TRANSFORMS_H ? */

#ifndef GUCEF_MATH_CCOORDINATESYSTEM_H
#include "gucefMATH_CCoordinateSystem.h"
#define GUCEF_MATH_CCOORDINATESYSTEM_H
#endif /* GUCEF_MATH_CCOORDINATESYSTEM_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

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
 *  A node of a relative transform tree (section 7.3 of the design): a coordinate space placed relative to the space of
 *  its parent, like a scene graph node. Children follow their parent automatically: world( node ) = world( parent ) *
 *  local( node ), with local = Translation * Rotation * Scale, composed as 4x4 affine matrices so that the shear of a
 *  non-uniform scale under a rotated parent is represented exactly.
 *
 *  Ownership and lifetime (7.3.2)
 *  - The tree does not own its nodes: SetParent links, it never transfers ownership. A node is not copyable.
 *  - Destroying a node detaches it from its parent and makes each child a root that keeps its world placement (its
 *    local transform becomes its former world transform; exact unless that world matrix has shear, see
 *    "Placements that need shear" below).
 *  - Children are held in attach order in an intrusive sibling list: attaching and detaching are O(1) and never
 *    allocate; GetChild( index ) walks the list (use GetFirstChild / GetNextSibling to iterate). Re-parenting a child
 *    unlinks it, so read GetNextSibling before calling SetParent while iterating, or use MoveChildrenTo.
 *  - The cycle check of SetParent walks the ancestors of the new parent (O(depth)); a node without children is
 *    attached in O(1).
 *
 *  Caching (7.3.3)
 *  - World and inverse world matrices are computed lazily and cached, top-down along the chain of invalid ancestors
 *    without recursion (any depth, any stack size). Reading a valid world matrix is a flag test.
 *  - The inverse world matrix is composed from the exact inverses of the local TRS transforms (S^-1 R^T T^-1) along
 *    the chain, never by a general matrix inversion: it is as precise as the world matrix, also for fixed point.
 *  - A change invalidates only the part of the subtree that holds valid caches or armed notifications: every node
 *    keeps an intrusive list of its "live" children (children with valid caches, an armed notification, or live
 *    descendants), so a root with 10^5 children of which one was read costs one visit per change.
 *  - The similarity flag is derived from the columns of the world matrix (equal lengths, orthogonal, within the
 *    relative tolerance of the kind), so it cannot drift over long chains. Fixed point has relative tolerance 0 and
 *    rounds absolutely: its shear and similarity checks (the flag, GetWorldTransform, SetParent( keepWorld ),
 *    Rotate( WORLD ), SetWorldTransform, the destructor) allow 256 raw units times max( 1, magnitude ), so products
 *    of rotations and uniform scales count as similarities, deterministically on every platform.
 *  - GetWorldRecomputeCount counts the recomputations of this node's world matrix (diagnostics, always present).
 *
 *  Notification (the hook of linked objects, structures and gucefPHYSICS)
 *  - OnWorldPlacementInvalidated() runs on the owner thread when the node's world placement changes: always for the
 *    node whose local transform or parent was changed directly, and for each descendant that is armed. A node is
 *    armed when it is created, after any read of its placement (world or inverse world matrix, GetTransformToPrecise
 *    and ToSpacePrecise on either end) and after RearmPlacementNotification(); the notification for a change of an
 *    ancestor disarms it (a direct change leaves the armed state as it was). So a consumer is told of the first
 *    ancestor change after each of its reads and never misses one, and a node that was never read is told of the
 *    first one: gucefPHYSICS sync-in, which reads kinematic bodies with GetTransformToPrecise, re-arms them by that
 *    read; a consumer that does not read re-arms with RearmPlacementNotification after handling a notification.
 *  - The hooks of one change run after the whole subtree has been invalidated, so a hook may read placements (also
 *    of other nodes); it must not change the tree. Overrides should call the base class.
 *
 *  Placements that need shear
 *  - SetParent( p, true ), SetWorldTransform and the re-rooting of children by the destructor compute a new local
 *    TRS from a matrix. When that matrix has shear (non-uniform scale under a rotated parent) no TRS reproduces it:
 *    the result keeps the position and the three axis lengths exactly, the first axis direction exactly and the other
 *    two axes the nearest orthogonal directions (Gram-Schmidt); the dropped part is the shear. The members report it
 *    (false from SetWorldTransform and GetWorldTransform, the 'approximated' output of SetParent).
 *
 *  Precision
 *  - ToSpace / GetTransformTo go through the cached world matrices: O(1) when valid.
 *  - ToSpacePrecise / GetTransformToPrecise compose local transforms along the paths to the lowest common ancestor and
 *    never form world coordinates; below the ancestor the translations of the two branches are subtracted first, so two
 *    siblings far from the origin are related with the precision of their offset. Different trees use world space.
 *  - Fixed point (TFixed32_32, amendment A-1 of section 11.2): every cache and conversion is bit-exact on every
 *    platform. RebaseOrigin then shifts world positions by exactly -offset and keeps GetTransformToPrecise between any
 *    two non-root nodes bit-identical.
 *
 *  Threading (7.3.7): the tree is mutated and read on one owner thread; the const getters write the caches, so they
 *  are owner-thread only as well. Other threads read snapshots (CaptureSnapshot, CTGeoSpaceSnapshotSet) published
 *  through MT::CTSlotDataExchange, never the nodes. Debug builds record the thread of the first use of each node and
 *  assert on use from another thread (ClearOwnerThread hands a node over after a synchronising join).
 *
 *  - Kinds: needs a square root and transcendentals; Float32, Float64 and TFixed32_32 are exported instantiations.
 *  - No nested TValue: the value-shape facade templates (Distance( a, b ) and others) are kept out of overload
 *    resolution for nodes and linked objects. The kind is TKind.
 *  - Not realtime-safe (section 2.4 rule 5): no member allocates or locks, but tree operations run on the owner thread.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGeoSpace
{
    public:

    typedef T TKind;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTTransform3D< T > TTransform;
    typedef MATH::CTMatrix4D< T > TMatrix;
    typedef MATH::CTMatrix3D< T > TMatrix3;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTPoint2D< T > TPoint2D;
    typedef MATH::CTQuaternion< T > TQuaternion;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTGeoSpaceSnapshot< T > TSnapshot;

    /** A root at the identity placement */
    CTGeoSpace( void );

    /** A root at 'localTransform' */
    explicit CTGeoSpace( const TTransform& localTransform );

    /** Detaches from the parent; children become roots that keep their world placement (see the class comment) */
    virtual ~CTGeoSpace();

    /*---------------------------------------------------------------------*/
    /*  Hierarchy                                                           */
    /*---------------------------------------------------------------------*/

    /**
     *  Links this node under 'parent' (NULL makes it a root). keepWorldPlacement: the local transform is recomputed so
     *  that the world placement stays ('approximated', when given, tells whether that needed the shear-free
     *  approximation of the class comment); false: the local transform is kept and the node moves with its new parent.
     *  False, nothing changed, when 'parent' is this node or one of its descendants (a cycle). Re-linking to the current
     *  parent is a no-op returning true.
     */
    bool SetParent( CTGeoSpace* parent, bool keepWorldPlacement, bool* approximated = GUCEF_NULL );

    /**
     *  Re-parents every child of this node to 'newParent' (NULL: they become roots) in attach order; children for which
     *  that would create a cycle stay. Returns the number of children moved.
     */
    UInt32 MoveChildrenTo( CTGeoSpace* newParent, bool keepWorldPlacement );

    GUCEF_GEOMETRY_FORCE_INLINE CTGeoSpace* GetParent( void ) const { return m_parent; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetChildCount( void ) const { return m_childCount; }

    /** The child at 'index' in attach order (O(index)); NULL when out of range */
    CTGeoSpace* GetChild( UInt32 index ) const;

    /** The first child in attach order, NULL when there is none */
    GUCEF_GEOMETRY_FORCE_INLINE CTGeoSpace* GetFirstChild( void ) const { return m_firstChild; }

    /** The next child of the same parent in attach order, NULL for the last one */
    GUCEF_GEOMETRY_FORCE_INLINE CTGeoSpace* GetNextSibling( void ) const { return m_nextSibling; }

    /** The root of the tree of this node (this node for a root) */
    CTGeoSpace* GetRoot( void ) const;

    /** True when this node is a proper ancestor of 'other' (false for 'other' itself) */
    bool IsAncestorOf( const CTGeoSpace& other ) const;

    /** The deepest node that is an ancestor of both or one of them; this node for itself; NULL for different trees */
    const CTGeoSpace* GetLowestCommonAncestor( const CTGeoSpace& other ) const;

    /** The number of ancestors (0 for a root) */
    UInt32 GetDepth( void ) const;

    /*---------------------------------------------------------------------*/
    /*  Local placement, relative to the parent's coordinate space          */
    /*---------------------------------------------------------------------*/

    /** Sets the TRS relative to the parent (the rotation must be a unit quaternion) */
    void SetLocalTransform( const TTransform& transform );

    GUCEF_GEOMETRY_FORCE_INLINE const TTransform& GetLocalTransform( void ) const { return m_local; }

    void SetLocalPosition( const TVector& position );
    void SetLocalRotation( const TQuaternion& rotation );
    void SetLocalScale( const TVector& scale );

    /**
     *  Moves the node by 'delta': LOCAL along the node's own rotated axes (in parent units, the scale is not applied),
     *  PARENT in the parent's axes, WORLD in world axes (mapped into the parent's space; exact under any parent)
     */
    void Translate( const TVector& delta, TGeoSpaceRelativeTo relativeTo );

    /**
     *  Rotates the node about its own origin: LOCAL rotation * q, PARENT q * rotation (both always), WORLD so that the
     *  new world matrix is q applied to the old one about the node's world position. Under a parent whose world
     *  matrix is a similarity without mirror this is the quaternion product; under any other parent the new local
     *  transform is solved from the matrices, exact also for mirrors and for non-uniform scale that the rotation keeps
     *  shear-free. False, nothing changed, when no TRS can represent the result (it would need shear).
     */
    bool Rotate( const TQuaternion& rotation, TGeoSpaceRelativeTo relativeTo );

    /**
     *  Turns the node so that the world image of the forward axis of 'system' points exactly at 'worldTarget' and the
     *  image of its up axis has the largest component along 'worldUp' that a rotation allows, under any parent: the
     *  directions are solved in the parent's space (mirrors, non-uniform scale and shear included). False, nothing changed, when the target
     *  is at the node's world position or the parent's world matrix is singular.
     */
    bool LookAt( const TPoint& worldTarget, const TVector& worldUp, const MATH::CCoordinateSystem& system );

    /** The local TRS as a matrix */
    GUCEF_GEOMETRY_FORCE_INLINE TMatrix GetLocalMatrix( void ) const { return m_local.ToMatrix(); }

    /*---------------------------------------------------------------------*/
    /*  World placement (derived, cached)                                   */
    /*---------------------------------------------------------------------*/

    /** The world matrix: a flag test when valid, else composed top-down from the first valid ancestor */
    GUCEF_GEOMETRY_FORCE_INLINE const TMatrix& GetWorldMatrix( void ) const
    {
        if ( 0 != ( m_cacheFlags & GEOSPACECACHE_WORLD_DIRTY ) )
            UpdateWorldMatrix();
        return m_world;
    }

    /** The inverse world matrix, composed from the exact inverses of the local transforms (see the class comment) */
    GUCEF_GEOMETRY_FORCE_INLINE const TMatrix& GetInverseWorldMatrix( void ) const
    {
        if ( 0 != ( m_cacheFlags & GEOSPACECACHE_INVERSE_DIRTY ) )
            UpdateInverseWorldMatrix();
        return m_inverseWorld;
    }

    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetWorldPosition( void ) const { return TPoint( GetWorldMatrix().GetTranslation() ); }

    /** The rotation of the world matrix: exact without shear (a mirror is placed on x), the orthonormalised linear part otherwise */
    TQuaternion GetWorldRotation( void ) const;

    /** The scale of the world matrix (a mirror is placed on x); the column lengths when the chain produces shear */
    TVector GetWorldScaleApproximation( void ) const;

    /** True when the world matrix is a rotation, a uniform scale (mirrors allowed) and a translation (from its columns, cached with it) */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsWorldTransformSimilarity( void ) const
    {
        GetWorldMatrix();
        return 0 != ( m_cacheFlags & GEOSPACECACHE_SIMILARITY );
    }

    /** Sets the local transform so that the world placement becomes 'worldTransform'; false when that needed the shear-free approximation */
    bool SetWorldTransform( const TTransform& worldTransform );

    /** Sets the local position so that the world position becomes 'worldPosition' (rotation and scale are kept; exact) */
    void SetWorldPosition( const TPoint& worldPosition );

    /** The world placement as a TRS; false when the world matrix has shear and the result is the approximation of the class comment */
    bool GetWorldTransform( TTransform& worldTransform ) const;

    /** True while the cached world placement is invalid (diagnostics and owners of notification guards) */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsWorldPlacementDirty( void ) const { return 0 != ( m_cacheFlags & GEOSPACECACHE_WORLD_DIRTY ); }

    /** Arms the notification: the next change of this node's world placement calls OnWorldPlacementInvalidated */
    void RearmPlacementNotification( void ) const;

    /** True while the notification is armed (see the class comment) */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsPlacementNotificationArmed( void ) const
    {
        return 0 != ( m_cacheFlags & GEOSPACECACHE_ARMED ) || GEOSPACECACHE_FULLY_DIRTY != ( m_cacheFlags & GEOSPACECACHE_FULLY_DIRTY );
    }

    /** The number of times this node's world matrix was recomputed */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetWorldRecomputeCount( void ) const { return m_worldRecomputeCount; }

    /** Debug builds: forgets the owner thread, so that another thread may take the node over after a synchronising join */
    GUCEF_GEOMETRY_FORCE_INLINE void ClearOwnerThread( void ) const { m_ownerThread = 0; }

    /*---------------------------------------------------------------------*/
    /*  Conversions                                                         */
    /*---------------------------------------------------------------------*/

    GUCEF_GEOMETRY_FORCE_INLINE TPoint ToWorld( const TPoint& localPoint ) const { return GetWorldMatrix().TransformPoint( localPoint ); }
    GUCEF_GEOMETRY_FORCE_INLINE TVector ToWorld( const TVector& localVector ) const { return GetWorldMatrix().TransformVector( localVector ); }

    /** The direction of the world image of the local direction (renormalised; the input when the image is zero) */
    GUCEF_GEOMETRY_FORCE_INLINE TDirection ToWorld( const TDirection& localDirection ) const { return GetWorldMatrix().TransformDirection( localDirection ); }

    /** The world normal of a local surface normal: the inverse transpose, renormalised (the input when degenerate) */
    TDirection NormalToWorld( const TDirection& localNormal ) const;

    /** The point ( x, y, 0 ) of this space in world coordinates: 2D geometry lies in the XY plane of its space */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint ToWorld( const TPoint2D& planePoint ) const { return GetWorldMatrix().TransformPoint( TPoint( planePoint.x, planePoint.y, TTraits::Zero() ) ); }

    GUCEF_GEOMETRY_FORCE_INLINE TPoint ToLocal( const TPoint& worldPoint ) const { return GetInverseWorldMatrix().TransformPoint( worldPoint ); }
    GUCEF_GEOMETRY_FORCE_INLINE TVector ToLocal( const TVector& worldVector ) const { return GetInverseWorldMatrix().TransformVector( worldVector ); }
    GUCEF_GEOMETRY_FORCE_INLINE TDirection ToLocal( const TDirection& worldDirection ) const { return GetInverseWorldMatrix().TransformDirection( worldDirection ); }

    /** The local normal of a world surface normal: the transposed world linear part, renormalised (the input when degenerate) */
    TDirection NormalToLocal( const TDirection& worldNormal ) const;

    /** 'point' of this space in the coordinates of 'target': inverseWorld( target ) * world( this ) * point, from the caches */
    TPoint ToSpace( const CTGeoSpace& target, const TPoint& point ) const;
    TVector ToSpace( const CTGeoSpace& target, const TVector& vector ) const;
    TDirection ToSpace( const CTGeoSpace& target, const TDirection& direction ) const;

    /** As ToSpace through GetTransformToPrecise: never forms world coordinates within one tree */
    TPoint ToSpacePrecise( const CTGeoSpace& target, const TPoint& point ) const;
    TVector ToSpacePrecise( const CTGeoSpace& target, const TVector& vector ) const;
    TDirection ToSpacePrecise( const CTGeoSpace& target, const TDirection& direction ) const;

    /** The matrix that maps this space's coordinates to those of 'target', from the cached world matrices */
    TMatrix GetTransformTo( const CTGeoSpace& target ) const;

    /**
     *  As GetTransformTo along the paths to the lowest common ancestor (see the class comment); the identity for itself.
     *  Arms the notification of both ends (a precise read is a read).
     */
    TMatrix GetTransformToPrecise( const CTGeoSpace& target ) const;

    /*---------------------------------------------------------------------*/
    /*  Snapshots for other threads                                         */
    /*---------------------------------------------------------------------*/

    /**
     *  Captures the world placement with the axes of 'system'; the velocity is zero. The frame is that of the world
     *  matrix: forward is the direction of the image of the system's forward axis, up the image of its up axis made
     *  orthogonal to it, right completes the rotation (worldRotation): consistent with ToWorld under any placement; under
     *  a mirrored world matrix right is the proper-rotation right, opposite to the image of the right axis.
     */
    void CaptureSnapshot( TSnapshot& out, const MATH::CCoordinateSystem& system ) const;

    /** Captures the world placement with the axes of 'system' and the velocity tracked by the owner */
    void CaptureSnapshot( TSnapshot& out, const MATH::CCoordinateSystem& system, const TVector& velocity ) const;

    /**
     *  Captures the placement relative to the world point 'origin' in the kind U: the translation is formed in T and only
     *  then converted, so a Float32 snapshot of a Float64 tree is exact to Float32 precision (camera-relative rendering
     *  and audio). The rotation is converted and renormalised in U.
     */
    template < typename U >
    void CaptureSnapshotRelativeTo( CTGeoSpaceSnapshot< U >& out, const TPoint& origin, const MATH::CCoordinateSystem& system, const TVector& velocity ) const
    {
        TSnapshot local;
        CaptureSnapshot( local, system, velocity );
        TMatrix relative = local.worldMatrix;
        const TVector offset = local.worldPosition - origin;
        relative.SetTranslation( offset );
        out.worldMatrix = MATH::CTMatrix4D< U >( relative );
        out.worldPosition = MATH::CTPoint3D< U >( MATH::CTVector3D< U >( offset ) );
        out.worldRotation = MATH::CTQuaternion< U >( local.worldRotation ).Normalized();
        out.forward = MATH::CTDirectionVector3D< U >::FromNormalized( MATH::CTVector3D< U >( local.forward.AsVector() ) );
        out.up = MATH::CTDirectionVector3D< U >::FromNormalized( MATH::CTVector3D< U >( local.up.AsVector() ) );
        out.right = MATH::CTDirectionVector3D< U >::FromNormalized( MATH::CTVector3D< U >( local.right.AsVector() ) );
        out.velocity = MATH::CTVector3D< U >( velocity );
    }

    /** As above with a zero velocity */
    template < typename U >
    void CaptureSnapshotRelativeTo( CTGeoSpaceSnapshot< U >& out, const TPoint& origin, const MATH::CCoordinateSystem& system ) const
    {
        CaptureSnapshotRelativeTo( out, origin, system, TVector() );
    }

    /*---------------------------------------------------------------------*/
    /*  Whole-tree helpers (owner thread)                                   */
    /*---------------------------------------------------------------------*/

    /** Computes every invalid world matrix of this subtree top-down in one pass (the same values as lazy reads) */
    void UpdateWorldMatrices( void );

    /**
     *  Subtracts 'offset' (in the root's coordinates) from the local positions of the root's direct children only;
     *  every deeper node follows (section 7.3.5). With an identity root every world position moves by -offset. The
     *  caches of the subtrees are invalidated (bounded by the number of live nodes).
     */
    static void RebaseOrigin( CTGeoSpace& root, const TVector& offset );

    /**
     *  The TRS of an affine matrix: true and exact when it has no shear (CTTransform3D::FromMatrix), else false and the
     *  shear-free approximation of the class comment
     */
    static bool TransformFromMatrix( const TMatrix& matrix, TTransform& out );

    protected:

    /**
     *  Called on the owner thread when this node's world placement changes (see the class comment); the default does
     *  nothing. Must not change the tree.
     */
    virtual void OnWorldPlacementInvalidated( void );

    /** True while a cache that a derived class keeps with the world matrix (bounds) is valid; cleared with the world cache */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsWorldDerivedCacheValid( void ) const { return 0 != ( m_cacheFlags & GEOSPACECACHE_DERIVED_VALID ); }

    /** Marks the derived cache valid; call only after reading the world matrix */
    GUCEF_GEOMETRY_FORCE_INLINE void SetWorldDerivedCacheValid( bool valid ) const
    {
        m_cacheFlags = valid ? (UInt8) ( m_cacheFlags | GEOSPACECACHE_DERIVED_VALID ) : (UInt8) ( m_cacheFlags & ~GEOSPACECACHE_DERIVED_VALID );
    }

    /** Debug builds: asserts that the calling thread is the node's owner (the first thread that used it) */
    void CheckOwnerThread( void ) const;

    private:

    enum
    {
        GEOSPACECACHE_WORLD_DIRTY   = 1,
        GEOSPACECACHE_INVERSE_DIRTY = 2,
        GEOSPACECACHE_SIMILARITY    = 4,
        GEOSPACECACHE_DERIVED_VALID = 8,
        GEOSPACECACHE_ARMED         = 16,     /**< notify the next change even when the caches are invalid */
        GEOSPACECACHE_LIVE          = 32,     /**< linked in the parent's list of live children */
        GEOSPACECACHE_FULLY_DIRTY   = GEOSPACECACHE_WORLD_DIRTY | GEOSPACECACHE_INVERSE_DIRTY
    };

    /** Raw units of the fixed point tolerance of SimilarityTolerance */
    enum { GEOSPACE_FIXED_TOLERANCE_ULPS = 256 };

    CTGeoSpace( const CTGeoSpace& );                    /**< not copyable: identity matters */
    CTGeoSpace& operator=( const CTGeoSpace& );         /**< not copyable: identity matters */

    void UpdateWorldMatrix( void ) const;
    void UpdateInverseWorldMatrix( void ) const;
    void InvalidateSubtree( void );
    void LinkChild( CTGeoSpace* child );
    void UnlinkChild( CTGeoSpace* child );
    bool SetLocalFromWorld( const TMatrix& world, const CTGeoSpace* parent );
    void MakeLive( void ) const;
    void UnlinkLive( void ) const;

    /** The exact inverse of a TRS: S^-1 R^T T^-1 (a zero scale component gives 0) */
    static TMatrix InverseOfTransform( const TTransform& transform );

    /**
     *  The allowed deviation of a value of the given magnitude in the shear and similarity checks: the relative
     *  tolerance of the kind times the magnitude for floating point; for fixed point (relative tolerance 0, absolute
     *  rounding) GEOSPACE_FIXED_TOLERANCE_ULPS raw units times max( 1, magnitude )
     */
    static T SimilarityTolerance( const T& magnitude );

    /** Equal column lengths and orthogonal columns within SimilarityTolerance (squared lengths, no root) */
    static bool HasSimilarityColumns( const TMatrix& matrix );

    /** Every element of 'a' within SimilarityTolerance of the largest element magnitude of 'b' (and 1) */
    static bool IsCloseMatrix( const TMatrix& a, const TMatrix& b );

    TTransform m_local;
    CTGeoSpace* m_parent;
    CTGeoSpace* m_firstChild;
    CTGeoSpace* m_lastChild;
    CTGeoSpace* m_previousSibling;
    CTGeoSpace* m_nextSibling;
    mutable CTGeoSpace* m_firstLive;               /**< the live children */
    mutable CTGeoSpace* m_previousLive;            /**< the neighbours in the parent's list of live children */
    mutable CTGeoSpace* m_nextLive;
    mutable const CTGeoSpace* m_updateNext;        /**< scratch link of the top-down lazy update */
    mutable CTGeoSpace* m_notifyNext;              /**< scratch link of the notifications of one change */
    mutable GUCEF::TThreadId m_ownerThread;           /**< debug builds: the owner thread (0 until first use) */
    UInt32 m_childCount;
    mutable UInt32 m_worldRecomputeCount;
    mutable UInt8 m_cacheFlags;
    mutable TMatrix m_world;
    mutable TMatrix m_inverseWorld;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTGeoSpace< T >::CTGeoSpace( void )
    : m_local(), m_parent( GUCEF_NULL ), m_firstChild( GUCEF_NULL ), m_lastChild( GUCEF_NULL ), m_previousSibling( GUCEF_NULL ),
      m_nextSibling( GUCEF_NULL ), m_firstLive( GUCEF_NULL ), m_previousLive( GUCEF_NULL ), m_nextLive( GUCEF_NULL ), m_updateNext( GUCEF_NULL ),
      m_notifyNext( GUCEF_NULL ), m_ownerThread( 0 ), m_childCount( 0 ), m_worldRecomputeCount( 0 ),
      m_cacheFlags( GEOSPACECACHE_FULLY_DIRTY | GEOSPACECACHE_ARMED | GEOSPACECACHE_LIVE ), m_world(), m_inverseWorld()
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoSpace< T >::CTGeoSpace( const TTransform& localTransform )
    : m_local( localTransform ), m_parent( GUCEF_NULL ), m_firstChild( GUCEF_NULL ), m_lastChild( GUCEF_NULL ), m_previousSibling( GUCEF_NULL ),
      m_nextSibling( GUCEF_NULL ), m_firstLive( GUCEF_NULL ), m_previousLive( GUCEF_NULL ), m_nextLive( GUCEF_NULL ), m_updateNext( GUCEF_NULL ),
      m_notifyNext( GUCEF_NULL ), m_ownerThread( 0 ), m_childCount( 0 ), m_worldRecomputeCount( 0 ),
      m_cacheFlags( GEOSPACECACHE_FULLY_DIRTY | GEOSPACECACHE_ARMED | GEOSPACECACHE_LIVE ), m_world(), m_inverseWorld()
{
}

/*-------------------------------------------------------------------------*/

/*
 *  The children become roots at their former world placement. Under a world matrix that is a similarity (the common
 *  case) every child's new local transform is the exact TRS product of this node's world TRS and its local transform,
 *  without forming the child's world matrix; otherwise each child's world matrix is decomposed (approximated when it
 *  has shear, see the class comment).
 */
template < typename T >
CTGeoSpace< T >::~CTGeoSpace()
{
    CheckOwnerThread();
    if ( GUCEF_NULL != m_firstChild )
    {
        TTransform worldTrs;
        const bool exact = GetWorldTransform( worldTrs ) && IsWorldTransformSimilarity();
        while ( GUCEF_NULL != m_firstChild )
        {
            CTGeoSpace* child = m_firstChild;
            TTransform childWorld;
            if ( exact )
                childWorld = TTransform::Combine( worldTrs, child->m_local );
            else
                TransformFromMatrix( child->GetWorldMatrix(), childWorld );
            UnlinkChild( child );
            child->m_local = childWorld;
            child->InvalidateSubtree();
        }
    }
    if ( GUCEF_NULL != m_parent )
        m_parent->UnlinkChild( this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::OnWorldPlacementInvalidated( void )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::CheckOwnerThread( void ) const
{
    #ifdef GUCEF_GEOMETRY_DEBUG_MODE
    const GUCEF::TThreadId current = MT::GetCurrentThreadID();
    if ( 0 == m_ownerThread )
        m_ownerThread = current;
    /* a node is used by one owner thread (section 7.3.7): other threads read published snapshots */
    GUCEF_GEOMETRY_ASSERT( m_ownerThread == current );
    #endif
}

/*-------------------------------------------------------------------------*/

/* links this node, and every ancestor that is not linked yet, into the parent's list of live children */
template < typename T >
void
CTGeoSpace< T >::MakeLive( void ) const
{
    CTGeoSpace* node = const_cast< CTGeoSpace* >( this );
    while ( GUCEF_NULL != node && 0 == ( node->m_cacheFlags & GEOSPACECACHE_LIVE ) )
    {
        node->m_cacheFlags = (UInt8) ( node->m_cacheFlags | GEOSPACECACHE_LIVE );
        CTGeoSpace* parent = node->m_parent;
        if ( GUCEF_NULL == parent )
            return;
        node->m_previousLive = GUCEF_NULL;
        node->m_nextLive = parent->m_firstLive;
        if ( GUCEF_NULL != parent->m_firstLive )
            parent->m_firstLive->m_previousLive = node;
        parent->m_firstLive = node;
        node = parent;
    }
}

/*-------------------------------------------------------------------------*/

/* takes this node out of its parent's list of live children; the flag is kept (re-linked by LinkChild) */
template < typename T >
void
CTGeoSpace< T >::UnlinkLive( void ) const
{
    if ( 0 == ( m_cacheFlags & GEOSPACECACHE_LIVE ) || GUCEF_NULL == m_parent )
        return;
    if ( GUCEF_NULL != m_previousLive )
        m_previousLive->m_nextLive = m_nextLive;
    else
        m_parent->m_firstLive = m_nextLive;
    if ( GUCEF_NULL != m_nextLive )
        m_nextLive->m_previousLive = m_previousLive;
    m_previousLive = GUCEF_NULL;
    m_nextLive = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::LinkChild( CTGeoSpace* child )
{
    child->m_parent = this;
    child->m_previousSibling = m_lastChild;
    child->m_nextSibling = GUCEF_NULL;
    if ( GUCEF_NULL != m_lastChild )
        m_lastChild->m_nextSibling = child;
    else
        m_firstChild = child;
    m_lastChild = child;
    ++m_childCount;
    if ( 0 != ( child->m_cacheFlags & GEOSPACECACHE_LIVE ) )
    {
        child->m_cacheFlags = (UInt8) ( child->m_cacheFlags & ~GEOSPACECACHE_LIVE );
        child->MakeLive();
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::UnlinkChild( CTGeoSpace* child )
{
    child->UnlinkLive();
    if ( GUCEF_NULL != child->m_previousSibling )
        child->m_previousSibling->m_nextSibling = child->m_nextSibling;
    else
        m_firstChild = child->m_nextSibling;
    if ( GUCEF_NULL != child->m_nextSibling )
        child->m_nextSibling->m_previousSibling = child->m_previousSibling;
    else
        m_lastChild = child->m_previousSibling;
    child->m_parent = GUCEF_NULL;
    child->m_previousSibling = GUCEF_NULL;
    child->m_nextSibling = GUCEF_NULL;
    --m_childCount;
}

/*-------------------------------------------------------------------------*/

/*
 *  This node changed: it is notified, and so is every live descendant that has a valid cache or an armed
 *  notification. The live children lists are emptied on the way (pre-order, no stack: each popped child is entered,
 *  and its parent resumed when its own list is empty); the hooks run afterwards, in visiting order, so that a hook may
 *  read placements without disturbing the walk.
 */
template < typename T >
void
CTGeoSpace< T >::InvalidateSubtree( void )
{
    CheckOwnerThread();
    /* the changed node is told whatever its state; its armed state stays for the next change of an ancestor */
    m_cacheFlags = (UInt8) ( ( m_cacheFlags | GEOSPACECACHE_FULLY_DIRTY ) & ~GEOSPACECACHE_DERIVED_VALID );
    CTGeoSpace* notifyTail = this;
    m_notifyNext = GUCEF_NULL;
    CTGeoSpace* node = this;
    for ( ;; )
    {
        CTGeoSpace* child = node->m_firstLive;
        if ( GUCEF_NULL != child )
        {
            node->m_firstLive = child->m_nextLive;
            if ( GUCEF_NULL != child->m_nextLive )
                child->m_nextLive->m_previousLive = GUCEF_NULL;
            child->m_nextLive = GUCEF_NULL;
            child->m_previousLive = GUCEF_NULL;
            const UInt8 flags = child->m_cacheFlags;
            if ( GEOSPACECACHE_FULLY_DIRTY != ( flags & GEOSPACECACHE_FULLY_DIRTY ) || 0 != ( flags & GEOSPACECACHE_ARMED ) )
            {
                notifyTail->m_notifyNext = child;
                notifyTail = child;
                child->m_notifyNext = GUCEF_NULL;
            }
            child->m_cacheFlags = (UInt8) ( ( flags | GEOSPACECACHE_FULLY_DIRTY ) & ~( GEOSPACECACHE_DERIVED_VALID | GEOSPACECACHE_ARMED | GEOSPACECACHE_LIVE ) );
            node = child;
            continue;
        }
        if ( node == this )
            break;
        node = node->m_parent;
    }
    CTGeoSpace* notify = this;
    while ( GUCEF_NULL != notify )
    {
        CTGeoSpace* next = notify->m_notifyNext;
        notify->m_notifyNext = GUCEF_NULL;
        notify->OnWorldPlacementInvalidated();
        notify = next;
    }
}

/*-------------------------------------------------------------------------*/

/*
 *  Fixed point rounds absolutely: a product chain of rotations and uniform scales leaves the world matrix a few raw
 *  units off a similarity per level (measured: up to about 25 for 10 levels of scale 1 .. 10, about 230 for a scale
 *  100 under a scale 0.01), scaled by the magnitude above 1. 256 raw units is 6 * 10^-8 for Q32.32, still finer
 *  than the 10^-5 of Float32, and is integer arithmetic, so the checks give the same answer on every platform.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTGeoSpace< T >::SimilarityTolerance( const T& magnitude )
{
    if ( TTraits::IsFixedPoint )
        return T( (Int32) GEOSPACE_FIXED_TOLERANCE_ULPS ) * TTraits::Epsilon() * TTraits::Max( TTraits::One(), magnitude );
    return TTraits::DefaultRelativeTolerance() * magnitude;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::HasSimilarityColumns( const TMatrix& m )
{
    if ( !m.IsAffine() )
        return false;
    const TMatrix3 linear = m.GetLinear();
    const TVector c0 = linear.GetColumn( 0 );
    const TVector c1 = linear.GetColumn( 1 );
    const TVector c2 = linear.GetColumn( 2 );
    const T l0 = c0.Dot( c0 );
    const T l1 = c1.Dot( c1 );
    const T l2 = c2.Dot( c2 );
    const T largest = TTraits::Max( TTraits::Max( l0, l1 ), l2 );
    if ( !( TTraits::Zero() < largest ) )
        return false;
    /* squared lengths: twice the relative tolerance of the lengths */
    const T allowed = SimilarityTolerance( largest ) + SimilarityTolerance( largest );
    return !( allowed < largest - TTraits::Min( TTraits::Min( l0, l1 ), l2 ) ) && !( allowed < TTraits::Abs( c0.Dot( c1 ) ) ) &&
           !( allowed < TTraits::Abs( c0.Dot( c2 ) ) ) && !( allowed < TTraits::Abs( c1.Dot( c2 ) ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::IsCloseMatrix( const TMatrix& a, const TMatrix& b )
{
    T largest = TTraits::One();
    for ( UInt32 i=0; i<16; ++i )
        largest = TTraits::Max( largest, TTraits::Abs( b.GetData()[ i ] ) );
    const T allowed = SimilarityTolerance( largest );
    for ( UInt32 i=0; i<16; ++i )
        if ( allowed < TTraits::Abs( a.GetData()[ i ] - b.GetData()[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/* top-down along the chain of ancestors with an invalid world matrix, linked through m_updateNext (no recursion) */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoSpace< T >::UpdateWorldMatrix( void ) const
{
    CheckOwnerThread();
    const CTGeoSpace* top = this;
    m_updateNext = GUCEF_NULL;
    while ( GUCEF_NULL != top->m_parent && 0 != ( top->m_parent->m_cacheFlags & GEOSPACECACHE_WORLD_DIRTY ) )
    {
        top->m_parent->m_updateNext = top;
        top = top->m_parent;
    }
    for ( const CTGeoSpace* node = top; GUCEF_NULL != node; )
    {
        const TMatrix local = node->m_local.ToMatrix();
        if ( GUCEF_NULL != node->m_parent )
            node->m_world = node->m_parent->m_world * local;
        else
            node->m_world = local;
        const bool similar = HasSimilarityColumns( node->m_world );
        node->m_cacheFlags = (UInt8) ( ( node->m_cacheFlags & ~( GEOSPACECACHE_WORLD_DIRTY | GEOSPACECACHE_SIMILARITY ) ) | ( similar ? GEOSPACECACHE_SIMILARITY : 0 ) );
        ++node->m_worldRecomputeCount;
        node->MakeLive();
        const CTGeoSpace* next = node->m_updateNext;
        node->m_updateNext = GUCEF_NULL;
        node = next;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TMatrix
CTGeoSpace< T >::InverseOfTransform( const TTransform& transform )
{
    const T zero = TTraits::Zero();
    const T one = TTraits::One();
    const TVector& s = transform.scale;
    const TVector inverseScale( zero == s.x ? zero : one / s.x, zero == s.y ? zero : one / s.y, zero == s.z ? zero : one / s.z );
    /* rows of S^-1 R^T are the columns of R scaled by 1 / s */
    const TMatrix3 rotation = TMatrix3::FromQuaternion( transform.rotation );
    const TMatrix3 linear = TMatrix3::FromColumns( inverseScale.ComponentMultiply( rotation.GetRow( 0 ) ), inverseScale.ComponentMultiply( rotation.GetRow( 1 ) ),
                                                   inverseScale.ComponentMultiply( rotation.GetRow( 2 ) ) );
    TMatrix inverse( linear );
    inverse.SetTranslation( -( linear * transform.position ) );
    return inverse;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoSpace< T >::UpdateInverseWorldMatrix( void ) const
{
    CheckOwnerThread();
    const CTGeoSpace* top = this;
    m_updateNext = GUCEF_NULL;
    while ( GUCEF_NULL != top->m_parent && 0 != ( top->m_parent->m_cacheFlags & GEOSPACECACHE_INVERSE_DIRTY ) )
    {
        top->m_parent->m_updateNext = top;
        top = top->m_parent;
    }
    for ( const CTGeoSpace* node = top; GUCEF_NULL != node; )
    {
        const TMatrix inverseLocal = InverseOfTransform( node->m_local );
        if ( GUCEF_NULL != node->m_parent )
            node->m_inverseWorld = inverseLocal * node->m_parent->m_inverseWorld;
        else
            node->m_inverseWorld = inverseLocal;
        node->m_cacheFlags = (UInt8) ( node->m_cacheFlags & ~GEOSPACECACHE_INVERSE_DIRTY );
        node->MakeLive();
        const CTGeoSpace* next = node->m_updateNext;
        node->m_updateNext = GUCEF_NULL;
        node = next;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::RearmPlacementNotification( void ) const
{
    CheckOwnerThread();
    m_cacheFlags = (UInt8) ( m_cacheFlags | GEOSPACECACHE_ARMED );
    MakeLive();
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoSpace< T >::SetParent( CTGeoSpace* parent, bool keepWorldPlacement, bool* approximated )
{
    if ( GUCEF_NULL != approximated )
        *approximated = false;
    if ( parent == m_parent )
        return true;
    /* a node without children cannot be an ancestor of the new parent: O(1) */
    if ( parent == this || ( GUCEF_NULL != parent && GUCEF_NULL != m_firstChild && IsAncestorOf( *parent ) ) )
        return false;
    if ( keepWorldPlacement )
    {
        const bool exact = SetLocalFromWorld( GetWorldMatrix(), parent );
        if ( GUCEF_NULL != approximated )
            *approximated = !exact;
    }
    if ( GUCEF_NULL != m_parent )
        m_parent->UnlinkChild( this );
    if ( GUCEF_NULL != parent )
        parent->LinkChild( this );
    InvalidateSubtree();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTGeoSpace< T >::MoveChildrenTo( CTGeoSpace* newParent, bool keepWorldPlacement )
{
    if ( newParent == this )
        return 0;
    UInt32 moved = 0;
    CTGeoSpace* child = m_firstChild;
    while ( GUCEF_NULL != child )
    {
        CTGeoSpace* next = child->m_nextSibling;
        if ( child->SetParent( newParent, keepWorldPlacement ) )
            ++moved;
        child = next;
    }
    return moved;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::TransformFromMatrix( const TMatrix& matrix, TTransform& out )
{
    /*
     *  FromMatrix accepts a shear up to 4 sqrt( epsilon ); the TRS counts as exact only when it reproduces the matrix to
     *  the relative tolerance of the kind
     */
    if ( TTransform::FromMatrix( matrix, out ) && IsCloseMatrix( out.ToMatrix(), matrix ) )
        return true;
    /* shear: keep the position, the axis lengths and the first axis; Gram-Schmidt for the others */
    const TMatrix3 linear = matrix.GetLinear();
    const TVector c0 = linear.GetColumn( 0 );
    const TVector c1 = linear.GetColumn( 1 );
    const TVector c2 = linear.GetColumn( 2 );
    const T l0 = c0.GetLength();
    const T l1 = c1.GetLength();
    const T l2 = c2.GetLength();
    const bool mirror = TTraits::IsNegative( linear.GetDeterminant() );
    TVector u0;
    if ( !c0.TryNormalize( u0, TTraits::Zero() ) )
        u0 = TVector::UnitX();
    if ( mirror )
        u0 = -u0;
    TVector u1;
    if ( !( c1 - u0 * c1.Dot( u0 ) ).TryNormalize( u1, TTraits::Zero() ) )
    {
        const TVector helper = TTraits::Abs( u0.x ) < TTraits::Half() ? TVector::UnitX() : TVector::UnitY();
        ( helper - u0 * helper.Dot( u0 ) ).TryNormalize( u1, TTraits::Zero() );
    }
    const TVector u2 = u0.Cross( u1 );
    const TQuaternion rotation = TMatrix3::FromColumns( u0, u1, u2 ).ToQuaternion().Normalized();
    out = TTransform( matrix.GetTranslation(), rotation, TVector( mirror ? -l0 : l0, l1, TTraits::IsNegative( c2.Dot( u2 ) ) ? -l2 : l2 ) );
    return false;
}

/*-------------------------------------------------------------------------*/

/* m_local := the TRS of inverseWorld( parent ) * world; true when exact */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::SetLocalFromWorld( const TMatrix& world, const CTGeoSpace* parent )
{
    TTransform local;
    const bool exact = TransformFromMatrix( GUCEF_NULL == parent ? world : parent->GetInverseWorldMatrix() * world, local );
    m_local = local;
    return exact;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::GetWorldTransform( TTransform& worldTransform ) const
{
    return TransformFromMatrix( GetWorldMatrix(), worldTransform );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoSpace< T >*
CTGeoSpace< T >::GetChild( UInt32 index ) const
{
    CTGeoSpace* child = m_firstChild;
    for ( UInt32 i=0; i<index && GUCEF_NULL != child; ++i )
        child = child->m_nextSibling;
    return child;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTGeoSpace< T >*
CTGeoSpace< T >::GetRoot( void ) const
{
    const CTGeoSpace* node = this;
    while ( GUCEF_NULL != node->m_parent )
        node = node->m_parent;
    return const_cast< CTGeoSpace* >( node );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoSpace< T >::IsAncestorOf( const CTGeoSpace& other ) const
{
    for ( const CTGeoSpace* node = other.m_parent; GUCEF_NULL != node; node = node->m_parent )
        if ( node == this )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTGeoSpace< T >::GetDepth( void ) const
{
    UInt32 depth = 0;
    for ( const CTGeoSpace* node = m_parent; GUCEF_NULL != node; node = node->m_parent )
        ++depth;
    return depth;
}

/*-------------------------------------------------------------------------*/

template < typename T >
const CTGeoSpace< T >*
CTGeoSpace< T >::GetLowestCommonAncestor( const CTGeoSpace& other ) const
{
    const CTGeoSpace* a = this;
    const CTGeoSpace* b = &other;
    UInt32 depthA = GetDepth();
    UInt32 depthB = other.GetDepth();
    while ( depthA > depthB )
    {
        a = a->m_parent;
        --depthA;
    }
    while ( depthB > depthA )
    {
        b = b->m_parent;
        --depthB;
    }
    while ( a != b )
    {
        a = a->m_parent;
        b = b->m_parent;
    }
    return a;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::SetLocalTransform( const TTransform& transform )
{
    m_local = transform;
    InvalidateSubtree();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::SetLocalPosition( const TVector& position )
{
    m_local.position = position;
    InvalidateSubtree();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::SetLocalRotation( const TQuaternion& rotation )
{
    m_local.rotation = rotation;
    InvalidateSubtree();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::SetLocalScale( const TVector& scale )
{
    m_local.scale = scale;
    InvalidateSubtree();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoSpace< T >::Translate( const TVector& delta, TGeoSpaceRelativeTo relativeTo )
{
    switch ( relativeTo )
    {
        case GEOSPACE_RELATIVETO_LOCAL:
            m_local.position = m_local.position + m_local.rotation.Rotate( delta );
            break;
        case GEOSPACE_RELATIVETO_WORLD:
            m_local.position = m_local.position + ( GUCEF_NULL != m_parent ? m_parent->GetInverseWorldMatrix().TransformVector( delta ) : delta );
            break;
        default:
            m_local.position = m_local.position + delta;
            break;
    }
    InvalidateSubtree();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::Rotate( const TQuaternion& rotation, TGeoSpaceRelativeTo relativeTo )
{
    if ( GEOSPACE_RELATIVETO_LOCAL == relativeTo )
    {
        m_local.rotation = ( m_local.rotation * rotation ).Normalized();
    }
    else
    if ( GEOSPACE_RELATIVETO_WORLD != relativeTo || GUCEF_NULL == m_parent )
    {
        m_local.rotation = ( rotation * m_local.rotation ).Normalized();
    }
    else
    if ( m_parent->IsWorldTransformSimilarity() && !TTraits::IsNegative( m_parent->GetWorldMatrix().GetLinear().GetDeterminant() ) )
    {
        /* under a rotation times a positive uniform scale: q expressed in the parent */
        const TQuaternion parentRotation = m_parent->GetWorldRotation();
        m_local.rotation = ( parentRotation.Conjugate() * rotation * parentRotation * m_local.rotation ).Normalized();
    }
    else
    {
        /* solve local = inverseWorld( parent ) * T( p ) R( q ) T( -p ) * world exactly; refuse when that needs shear */
        TMatrix world = GetWorldMatrix();
        const TVector position = world.GetTranslation();
        world = TMatrix( rotation.ToRotationMatrix3D() * world.GetLinear() );
        world.SetTranslation( position );
        TTransform local;
        if ( !TransformFromMatrix( m_parent->GetInverseWorldMatrix() * world, local ) )
            return false;
        m_local = local;
    }
    InvalidateSubtree();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTGeoSpace< T >::LookAt( const TPoint& worldTarget, const TVector& worldUp, const MATH::CCoordinateSystem& system )
{
    /*
     *  forward: the parent's inverse maps the world direction into the parent's space (its image then points exactly
     *  at the target); up: the transposed linear part of the parent's world matrix gives the local direction,
     *  perpendicular to forward, whose image has the largest component along worldUp
     */
    TVector direction = worldTarget - GetWorldPosition();
    TVector up = worldUp;
    if ( GUCEF_NULL != m_parent )
    {
        direction = m_parent->GetInverseWorldMatrix().TransformVector( direction );
        up = m_parent->GetWorldMatrix().GetLinear().Transposed() * up;
    }
    /* the node's own scale may mirror its forward or up axis: the axis image then points the other way */
    const TVector forwardAxis = system.GetForward< T >();
    const TVector upAxis = system.GetUp< T >();
    if ( TTraits::IsNegative( m_local.scale.Dot( forwardAxis.Abs() ) ) )
        direction = -direction;
    if ( TTraits::IsNegative( m_local.scale.Dot( upAxis.Abs() ) ) )
        up = -up;
    TDirection forward;
    if ( !TDirection::TryCreate( direction, forward ) )
        return false;
    m_local.rotation = TQuaternion::LookRotation( forward, up, system ).Normalized();
    InvalidateSubtree();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TQuaternion
CTGeoSpace< T >::GetWorldRotation( void ) const
{
    const TMatrix& world = GetWorldMatrix();
    TVector translation;
    TQuaternion rotation;
    TVector scale;
    if ( world.Decompose( translation, rotation, scale ) )
        return rotation;
    /* shear: the rotation of the orthonormalised linear part, a mirror moved onto x as Decompose does */
    TMatrix3 linear = world.GetLinear();
    if ( TTraits::IsNegative( linear.GetDeterminant() ) )
        linear.SetColumn( 0, -linear.GetColumn( 0 ) );
    return linear.Orthonormalized().ToQuaternion().Normalized();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TVector
CTGeoSpace< T >::GetWorldScaleApproximation( void ) const
{
    const TMatrix& world = GetWorldMatrix();
    TVector translation;
    TQuaternion rotation;
    TVector scale;
    if ( world.Decompose( translation, rotation, scale ) )
        return scale;
    const TMatrix3 linear = world.GetLinear();
    const T x = linear.GetColumn( 0 ).GetLength();
    return TVector( TTraits::IsNegative( linear.GetDeterminant() ) ? -x : x, linear.GetColumn( 1 ).GetLength(), linear.GetColumn( 2 ).GetLength() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTGeoSpace< T >::SetWorldTransform( const TTransform& worldTransform )
{
    bool exact = true;
    if ( GUCEF_NULL == m_parent )
        m_local = worldTransform;
    else
        exact = SetLocalFromWorld( worldTransform.ToMatrix(), m_parent );
    InvalidateSubtree();
    return exact;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::SetWorldPosition( const TPoint& worldPosition )
{
    m_local.position = ( GUCEF_NULL != m_parent ? m_parent->GetInverseWorldMatrix().TransformPoint( worldPosition ) : worldPosition ).ToPositionVector();
    InvalidateSubtree();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TDirection
CTGeoSpace< T >::NormalToWorld( const TDirection& localNormal ) const
{
    TDirection result( localNormal );
    if ( !TDirection::TryCreate( GetInverseWorldMatrix().GetLinear().Transposed() * localNormal.AsVector(), result ) )
        return localNormal;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TDirection
CTGeoSpace< T >::NormalToLocal( const TDirection& worldNormal ) const
{
    TDirection result( worldNormal );
    if ( !TDirection::TryCreate( GetWorldMatrix().GetLinear().Transposed() * worldNormal.AsVector(), result ) )
        return worldNormal;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TPoint
CTGeoSpace< T >::ToSpace( const CTGeoSpace& target, const TPoint& point ) const
{
    return target.GetInverseWorldMatrix().TransformPoint( GetWorldMatrix().TransformPoint( point ) );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TVector
CTGeoSpace< T >::ToSpace( const CTGeoSpace& target, const TVector& vector ) const
{
    return target.GetInverseWorldMatrix().TransformVector( GetWorldMatrix().TransformVector( vector ) );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TDirection
CTGeoSpace< T >::ToSpace( const CTGeoSpace& target, const TDirection& direction ) const
{
    TDirection result( direction );
    if ( !TDirection::TryCreate( ToSpace( target, direction.AsVector() ), result ) )
        return direction;
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TPoint
CTGeoSpace< T >::ToSpacePrecise( const CTGeoSpace& target, const TPoint& point ) const
{
    return GetTransformToPrecise( target ).TransformPoint( point );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TVector
CTGeoSpace< T >::ToSpacePrecise( const CTGeoSpace& target, const TVector& vector ) const
{
    return GetTransformToPrecise( target ).TransformVector( vector );
}

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TDirection
CTGeoSpace< T >::ToSpacePrecise( const CTGeoSpace& target, const TDirection& direction ) const
{
    return GetTransformToPrecise( target ).TransformDirection( direction );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TMatrix
CTGeoSpace< T >::GetTransformTo( const CTGeoSpace& target ) const
{
    return target.GetInverseWorldMatrix() * GetWorldMatrix();
}

/*-------------------------------------------------------------------------*/

/*
 *  With a1 / b1 the children of the common ancestor on the paths to this node and to the target:
 *      ( L_b1 ... L_target )^-1 * ( L_a1 ... L_this )
 *    = ( L_target^-1 ... L_b2^-1 ) * ( S_b1^-1 R_b1^T T( t_a1 - t_b1 ) R_a1 S_a1 ) * ( L_a2 ... L_this )
 *  so the translations of the two branches meet as one difference.
 */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTGeoSpace< T >::TMatrix
CTGeoSpace< T >::GetTransformToPrecise( const CTGeoSpace& target ) const
{
    RearmPlacementNotification();
    if ( &target == this )
        return TMatrix();
    target.RearmPlacementNotification();
    const CTGeoSpace* ancestor = GetLowestCommonAncestor( target );
    if ( GUCEF_NULL == ancestor )
        return GetTransformTo( target );

    TMatrix pathA;
    const CTGeoSpace* a1 = GUCEF_NULL;
    for ( const CTGeoSpace* node = this; node != ancestor; node = node->m_parent )
    {
        if ( node->m_parent == ancestor )
        {
            a1 = node;
            break;
        }
        pathA = node->m_local.ToMatrix() * pathA;
    }
    TMatrix inversePathB;
    const CTGeoSpace* b1 = GUCEF_NULL;
    for ( const CTGeoSpace* node = &target; node != ancestor; node = node->m_parent )
    {
        if ( node->m_parent == ancestor )
        {
            b1 = node;
            break;
        }
        inversePathB = inversePathB * InverseOfTransform( node->m_local );
    }

    TMatrix middle;
    if ( GUCEF_NULL != a1 && GUCEF_NULL != b1 )
    {
        const TTransform& la = a1->m_local;
        const TTransform& lb = b1->m_local;
        middle = InverseOfTransform( TTransform( TVector(), lb.rotation, lb.scale ) ) * TMatrix::FromTRS( la.position - lb.position, la.rotation, la.scale );
    }
    else
    if ( GUCEF_NULL != a1 )
    {
        middle = a1->m_local.ToMatrix();
    }
    else
    if ( GUCEF_NULL != b1 )
    {
        middle = InverseOfTransform( b1->m_local );
    }
    return inversePathB * middle * pathA;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::CaptureSnapshot( TSnapshot& out, const MATH::CCoordinateSystem& system ) const
{
    CaptureSnapshot( out, system, TVector() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTGeoSpace< T >::CaptureSnapshot( TSnapshot& out, const MATH::CCoordinateSystem& system, const TVector& velocity ) const
{
    out.worldMatrix = GetWorldMatrix();
    out.worldPosition = TPoint( out.worldMatrix.GetTranslation() );
    /* the frame of the matrix: the image of the forward axis exactly, the image of the up axis made orthogonal */
    TDirection forward;
    if ( TDirection::TryCreate( out.worldMatrix.TransformVector( system.GetForward< T >() ), forward ) )
        out.worldRotation = TQuaternion::LookRotation( forward, out.worldMatrix.TransformVector( system.GetUp< T >() ), system ).Normalized();
    else
        out.worldRotation = GetWorldRotation();
    out.forward = out.worldRotation.GetForward( system );
    out.up = out.worldRotation.GetUp( system );
    out.right = out.worldRotation.GetRight( system );
    out.velocity = velocity;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::UpdateWorldMatrices( void )
{
    CTGeoSpace* node = this;
    for ( ;; )
    {
        node->GetWorldMatrix();
        if ( GUCEF_NULL != node->m_firstChild )
        {
            node = node->m_firstChild;
            continue;
        }
        while ( node != this && GUCEF_NULL == node->m_nextSibling )
            node = node->m_parent;
        if ( node == this )
            return;
        node = node->m_nextSibling;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTGeoSpace< T >::RebaseOrigin( CTGeoSpace& root, const TVector& offset )
{
    for ( CTGeoSpace* child = root.m_firstChild; GUCEF_NULL != child; child = child->m_nextSibling )
    {
        child->m_local.position = child->m_local.position - offset;
        child->InvalidateSubtree();
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpace< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpace< Float64 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTGeoSpace< MATH::TFixed32_32 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGEOSPACE_H ? */
