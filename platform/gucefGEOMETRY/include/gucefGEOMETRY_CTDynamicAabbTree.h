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

#ifndef GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#define GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

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
 *  The insertion cost of CTDynamicAabbTree for a kind: half the surface area for floating-point kinds (TCost is the
 *  kind), the sum of the extents (the perimeter heuristic) for the other kinds. Fixed-point kinds sum the extents as
 *  raw values in UInt64: exact, and the costs the tree compares (at most nine extents) fit for extents up to 2^62 raw
 *  units, 2^30 for TFixed32_32, so the cost cannot wrap anywhere in the documented coordinate range.
 */
template < typename T, int FloatingPoint = (int) MATH::CTNumericTraits< T >::IsFloatingPoint >
struct CTDynamicAabbTreeCost
{
    typedef T TCost;

    static GUCEF_GEOMETRY_FORCE_INLINE TCost Zero( void ) { return MATH::CTNumericTraits< T >::Zero(); }

    static TCost Of( const CTAxisAlignedBox3D< T >& box )
    {
        if ( box.IsEmpty() )
            return Zero();
        const MATH::CTVector3D< T > e = box.max - box.min;
        return ( e.x * e.y + e.y * e.z ) + e.z * e.x;
    }
};

/** Kinds that are not floating point and not fixed point (integers): the perimeter in the kind */
template < typename T >
struct CTDynamicAabbTreeCost< T, 0 >
{
    typedef T TCost;

    static GUCEF_GEOMETRY_FORCE_INLINE TCost Zero( void ) { return MATH::CTNumericTraits< T >::Zero(); }

    static TCost Of( const CTAxisAlignedBox3D< T >& box )
    {
        if ( box.IsEmpty() )
            return Zero();
        const MATH::CTVector3D< T > e = box.max - box.min;
        return ( e.x + e.y ) + e.z;
    }
};

/** Fixed-point kinds: the perimeter as a sum of raw extents in UInt64 (exact, no wrap in range) */
template < typename TStorage, UInt32 FractionBits >
struct CTDynamicAabbTreeCost< MATH::CTFixedPoint< TStorage, FractionBits >, 0 >
{
    typedef UInt64 TCost;

    static GUCEF_GEOMETRY_FORCE_INLINE TCost Zero( void ) { return 0; }

    static GUCEF_GEOMETRY_FORCE_INLINE UInt64 Extent( const MATH::CTFixedPoint< TStorage, FractionBits >& low, const MATH::CTFixedPoint< TStorage, FractionBits >& high )
    {
        /* modulo 2^64: exact for any extent below 2^64 raw units, even when high - low does not fit the kind */
        return (UInt64) (Int64) high.GetRaw() - (UInt64) (Int64) low.GetRaw();
    }

    static TCost Of( const CTAxisAlignedBox3D< MATH::CTFixedPoint< TStorage, FractionBits > >& box )
    {
        if ( box.IsEmpty() )
            return 0;
        return ( Extent( box.min.x, box.max.x ) + Extent( box.min.y, box.max.y ) ) + Extent( box.min.z, box.max.z );
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  Broadphase tree of proxies in the style of the Box2D dynamic tree (section 7.8.3 of the design): each proxy is a
 *  leaf holding a fattened axis aligned box and a user value; interior nodes hold the union of their children. The
 *  tree knows nothing about what the proxies stand for: the ray caster keeps linked objects in it, gucefPHYSICS its
 *  colliders.
 *
 *  - Proxies: CreateProxy( bounds, userData ) returns the proxy id (the index of its leaf, stable for the life of the
 *    proxy); DestroyProxy frees it, the id may be reused by a later CreateProxy. The stored box is the given bounds
 *    grown by the margin on every side.
 *  - MoveProxy( id, bounds, displacement ): when the fat box still contains the new bounds and is not too large
 *    (within the new fat box grown by four margins) nothing changes and the result is false. Otherwise the leaf is
 *    reinserted with the new bounds grown by the margin and stretched along the displacement times the displacement
 *    multiplier (prediction of the next move), and the result is true.
 *  - Insertion descends by the cost heuristic of Box2D (the cost of a new parent plus the inherited enlargement of the
 *    ancestors). The cost is the surface area for floating-point kinds and the sum of the extents (the perimeter
 *    heuristic) for the other kinds (CTDynamicAabbTreeCost); fixed point sums raw extents in UInt64, exact and free of
 *    overflow over the documented coordinate range. Removal and insertion walk to
 *    the root and rotate any node whose children differ by more than one in height (one rotation per node on the path,
 *    as Box2D: the tree is not strictly AVL balanced, but the height stays logarithmic in the proxy count in practice).
 *  - Nodes live in one GUCEF::vector with a free list: no allocation per node once the pool has grown, proxy ids are
 *    node indices.
 *  - Query( bounds, visitor ): calls visitor.Visit( proxyId ) for every proxy whose fat box shares a point with the
 *    bounds (touching counts); a false return stops the query.
 *  - RayCast( ray, maxDistance, visitor ): calls visitor.Visit( proxyId, maxDistance ) for every proxy whose fat box
 *    the segment from the ray origin to origin + direction * length touches (RayOverlapsBox, a separating axis test
 *    on the three box axes and the three cross products of the direction with them; products only, no division).
 *    The visitor may lower maxDistance (the segment is clipped for the rest of the cast, a nearest-hit search) and
 *    returns false to stop. The length starts at ClampRayLength( ray, maxDistance ): maxDistance, or less when that
 *    reaches beyond every box of the tree (a saturating sum, so it cannot wrap in fixed point). The box of the segment
 *    used for the overlap tests is intersected with the root box, so its end point is never formed outside the coordinate range.
 *  - Validate checks the structure (links, heights, boxes equal to the union of the children, the free list and the
 *    proxy count; not the balance); GetHeight is the height of the root (0 for one proxy or none).
 *  - ShiftOrigin moves every box by -offset (origin rebasing of large worlds).
 *
 *  Determinism: the structure depends only on the sequence of calls and their values; for fixed point every
 *  operation is integer arithmetic (the boxes and the insertion costs exactly, the ray test with the rounded products
 *  of the kind, deterministic but not exact), so a sequence of calls gives a bit-identical tree and bit-identical
 *  query results on every platform.
 *  Fixed point: keep every fat box (the bounds grown by the margin and stretched by the predicted displacement) and
 *  every ray origin strictly inside +-2^29, so that sums and differences of two coordinates fit the kind.
 *
 *  - TUserData: a copyable, default constructible value (a pointer or an index).
 *  - Kinds: Float32, Float64 and TFixed32_32 are exported with UInt32 user data (amendment A-3 of section 11.2); the
 *    ray caster instantiates its own pointer user data.
 *  - Not realtime-safe (section 2.4): queries do not allocate unless the tree is deeper than 254 levels, but structure
 *    operations allocate when the pool grows. Owner thread for mutation; const queries may run concurrently while
 *    nothing mutates the tree.
 */
template < typename T, typename TUserData >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTDynamicAabbTree
{
    public:

    typedef T TValue;
    typedef TUserData TUserDataType;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTRay3D< T > TRay;

    /** The id of no proxy (DestroyProxy and the queries ignore it) */
    enum { NullProxy = -1 };

    /** An empty tree with the given margin (not negative) and displacement multiplier (not negative) */
    explicit CTDynamicAabbTree( const T& margin = DefaultMargin(), const T& displacementMultiplier = TTraits::Two() );

    /** The default margin: 1/10 of a unit */
    static T DefaultMargin( void );

    /** Removes every proxy; keeps the pool memory */
    void Clear( void );

    /** Grows the node pool for 'proxyCount' proxies (2 * proxyCount - 1 nodes) */
    void Reserve( UInt32 proxyCount );

    /** Creates a proxy for 'bounds' (stored grown by the margin) and returns its id */
    Int32 CreateProxy( const TBox& bounds, const TUserData& userData );

    /** Destroys a proxy; ignored for NullProxy and ids that are not proxies (debug builds assert) */
    void DestroyProxy( Int32 proxyId );

    /** Moves a proxy to new bounds (see the class comment); true when the proxy was reinserted */
    bool MoveProxy( Int32 proxyId, const TBox& bounds, const TVector& displacement );

    /** True when the id names a live proxy */
    bool IsProxy( Int32 proxyId ) const;

    /** The fat box of a proxy */
    GUCEF_GEOMETRY_FORCE_INLINE const TBox& GetFatBounds( Int32 proxyId ) const { return m_nodes[ proxyId ].bounds; }

    /** The user data of a proxy */
    GUCEF_GEOMETRY_FORCE_INLINE const TUserData& GetUserData( Int32 proxyId ) const { return m_nodes[ proxyId ].userData; }

    GUCEF_GEOMETRY_FORCE_INLINE void SetUserData( Int32 proxyId, const TUserData& userData ) { m_nodes[ proxyId ].userData = userData; }

    /** The number of live proxies */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetProxyCount( void ) const { return m_proxyCount; }

    /** The number of nodes in the pool (live and free) */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetNodeCapacity( void ) const { return (UInt32) m_nodes.size(); }

    /** The height of the tree: 0 for no proxy or one, otherwise the longest path from the root to a leaf */
    UInt32 GetHeight( void ) const;

    /** The box of the root (the union of all fat boxes; empty without proxies) */
    TBox GetRootBounds( void ) const;

    GUCEF_GEOMETRY_FORCE_INLINE const T& GetMargin( void ) const { return m_margin; }
    GUCEF_GEOMETRY_FORCE_INLINE const T& GetDisplacementMultiplier( void ) const { return m_displacementMultiplier; }

    /** Margin for proxies created or reinserted from now on (not negative) */
    void SetMargin( const T& margin );

    void SetDisplacementMultiplier( const T& multiplier );

    /** Moves every box by -offset (origin rebasing); the structure is kept */
    void ShiftOrigin( const TVector& offset );

    /** Checks the structure (see the class comment); true when it is consistent */
    bool Validate( void ) const;

    /** Calls visitor.Visit( Int32 proxyId ) -> bool for every proxy whose fat box touches 'bounds'; false stops */
    template < typename TVisitor >
    void Query( const TBox& bounds, TVisitor& visitor ) const;

    /**
     *  Calls visitor.Visit( Int32 proxyId, T& maxDistance ) -> bool for every proxy whose fat box the clipped segment of
     *  the ray touches; the visitor may lower maxDistance, false stops. A ray with a value that is not finite, or a
     *  negative maxDistance, visits nothing.
     */
    template < typename TVisitor >
    void RayCast( const TRay& ray, const T& maxDistance, TVisitor& visitor ) const;

    /** The segment length a ray cast starts with: maxDistance, lowered to a bound on the distance to the farthest box corner of the tree */
    T ClampRayLength( const TRay& ray, const T& maxDistance ) const;

    /** The test of RayCast: true when the segment from the ray origin along its direction over 'length' touches the box */
    static bool RayOverlapsBox( const TRay& ray, const T& length, const TBox& box );

    private:

    struct SNode
    {
        TBox bounds;
        TUserData userData;
        Int32 parent;           /**< the parent, or the next free node while free */
        Int32 child1;           /**< NullProxy for a leaf */
        Int32 child2;
        Int32 height;           /**< 0 for a leaf, -1 while free */

        GUCEF_GEOMETRY_FORCE_INLINE bool IsLeaf( void ) const { return NullProxy == child1; }
    };

    typedef CTDynamicAabbTreeCost< T > TCostPolicy;
    typedef typename TCostPolicy::TCost TCost;

    /** The insertion cost of a box (CTDynamicAabbTreeCost); 0 for an empty box */
    static GUCEF_GEOMETRY_FORCE_INLINE TCost Cost( const TBox& box ) { return TCostPolicy::Of( box ); }

    static bool SegmentAxesSeparate( const TRay& ray, const TBox& box );

    /** The box of the segment over 'length' intersected with 'root' (the end point is never formed outside it); false when they do not intersect */
    static bool ClippedSegmentBox( const TRay& ray, const T& length, const TBox& root, TBox& segment );

    Int32 AllocateNode( void );
    void FreeNode( Int32 node );
    void InsertLeaf( Int32 leaf );
    void RemoveLeaf( Int32 leaf );
    Int32 Balance( Int32 node );
    void RefitUpwards( Int32 node );
    UInt32 ValidateNode( Int32 node, bool& valid ) const;

    GUCEF::vector< SNode > m_nodes;
    Int32 m_root;
    Int32 m_freeList;
    UInt32 m_proxyCount;
    T m_margin;
    T m_displacementMultiplier;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
CTDynamicAabbTree< T, TUserData >::CTDynamicAabbTree( const T& margin, const T& displacementMultiplier )
    : m_nodes(), m_root( NullProxy ), m_freeList( NullProxy ), m_proxyCount( 0 ), m_margin( margin ), m_displacementMultiplier( displacementMultiplier )
{
    GUCEF_GEOMETRY_ASSERT( !TTraits::IsNegative( margin ) && !TTraits::IsNegative( displacementMultiplier ) );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
T
CTDynamicAabbTree< T, TUserData >::DefaultMargin( void )
{
    return TTraits::One() / TTraits::FromInt32( 10 );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::Clear( void )
{
    m_nodes.clear();
    m_root = NullProxy;
    m_freeList = NullProxy;
    m_proxyCount = 0;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::Reserve( UInt32 proxyCount )
{
    if ( 0 < proxyCount )
        m_nodes.reserve( 2 * (size_t) proxyCount - 1 );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::SetMargin( const T& margin )
{
    GUCEF_GEOMETRY_ASSERT( !TTraits::IsNegative( margin ) );
    m_margin = margin;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::SetDisplacementMultiplier( const T& multiplier )
{
    GUCEF_GEOMETRY_ASSERT( !TTraits::IsNegative( multiplier ) );
    m_displacementMultiplier = multiplier;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
Int32
CTDynamicAabbTree< T, TUserData >::AllocateNode( void )
{
    Int32 node = m_freeList;
    if ( NullProxy == node )
    {
        node = (Int32) m_nodes.size();
        m_nodes.push_back( SNode() );
    }
    else
    {
        m_freeList = m_nodes[ node ].parent;
    }
    SNode& n = m_nodes[ node ];
    n.bounds = TBox();
    n.userData = TUserData();
    n.parent = NullProxy;
    n.child1 = NullProxy;
    n.child2 = NullProxy;
    n.height = 0;
    return node;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::FreeNode( Int32 node )
{
    SNode& n = m_nodes[ node ];
    n.parent = m_freeList;
    n.child1 = NullProxy;
    n.child2 = NullProxy;
    n.height = -1;
    n.userData = TUserData();
    m_freeList = node;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
bool
CTDynamicAabbTree< T, TUserData >::IsProxy( Int32 proxyId ) const
{
    return 0 <= proxyId && proxyId < (Int32) m_nodes.size() && 0 == m_nodes[ proxyId ].height && m_nodes[ proxyId ].IsLeaf();
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
Int32
CTDynamicAabbTree< T, TUserData >::CreateProxy( const TBox& bounds, const TUserData& userData )
{
    const Int32 proxy = AllocateNode();
    m_nodes[ proxy ].bounds = bounds.Expanded( m_margin );
    m_nodes[ proxy ].userData = userData;
    InsertLeaf( proxy );
    ++m_proxyCount;
    return proxy;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::DestroyProxy( Int32 proxyId )
{
    if ( NullProxy == proxyId )
        return;
    GUCEF_GEOMETRY_ASSERT( IsProxy( proxyId ) );
    if ( !IsProxy( proxyId ) )
        return;
    RemoveLeaf( proxyId );
    FreeNode( proxyId );
    --m_proxyCount;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
bool
CTDynamicAabbTree< T, TUserData >::MoveProxy( Int32 proxyId, const TBox& bounds, const TVector& displacement )
{
    GUCEF_GEOMETRY_ASSERT( IsProxy( proxyId ) );
    if ( !IsProxy( proxyId ) )
        return false;

    TBox fat = bounds.Expanded( m_margin );
    const TVector d = displacement * m_displacementMultiplier;
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T component = d.GetComponent( axis );
        if ( TTraits::IsNegative( component ) )
            fat.min.SetComponent( axis, fat.min.GetComponent( axis ) + component );
        else
            fat.max.SetComponent( axis, fat.max.GetComponent( axis ) + component );
    }

    const TBox& stored = m_nodes[ proxyId ].bounds;
    if ( stored.Contains( bounds ) )
    {
        /* still enclosing: keep it unless it became much larger than needed */
        const TBox huge = fat.Expanded( m_margin * TTraits::FromInt32( 4 ) );
        if ( huge.Contains( stored ) )
            return false;
    }

    RemoveLeaf( proxyId );
    m_nodes[ proxyId ].bounds = fat;
    InsertLeaf( proxyId );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::InsertLeaf( Int32 leaf )
{
    if ( NullProxy == m_root )
    {
        m_root = leaf;
        m_nodes[ leaf ].parent = NullProxy;
        return;
    }

    /* the best sibling by the cost of a new parent plus the enlargement inherited by the ancestors */
    const TBox leafBounds = m_nodes[ leaf ].bounds;
    Int32 index = m_root;
    while ( !m_nodes[ index ].IsLeaf() )
    {
        const SNode& node = m_nodes[ index ];
        const Int32 child1 = node.child1;
        const Int32 child2 = node.child2;

        const TCost area = Cost( node.bounds );
        const TCost combinedArea = Cost( node.bounds.Merged( leafBounds ) );
        const TCost cost = combinedArea + combinedArea;
        const TCost inheritance = ( combinedArea - area ) + ( combinedArea - area );

        TCost cost1 = Cost( leafBounds.Merged( m_nodes[ child1 ].bounds ) ) + inheritance;
        if ( !m_nodes[ child1 ].IsLeaf() )
            cost1 = cost1 - Cost( m_nodes[ child1 ].bounds );
        TCost cost2 = Cost( leafBounds.Merged( m_nodes[ child2 ].bounds ) ) + inheritance;
        if ( !m_nodes[ child2 ].IsLeaf() )
            cost2 = cost2 - Cost( m_nodes[ child2 ].bounds );

        if ( cost < cost1 && cost < cost2 )
            break;
        index = cost1 < cost2 ? child1 : child2;
    }

    /* a new parent for the sibling and the leaf */
    const Int32 sibling = index;
    const Int32 oldParent = m_nodes[ sibling ].parent;
    const Int32 newParent = AllocateNode();
    SNode& parent = m_nodes[ newParent ];
    parent.parent = oldParent;
    parent.bounds = leafBounds.Merged( m_nodes[ sibling ].bounds );
    parent.height = m_nodes[ sibling ].height + 1;
    parent.child1 = sibling;
    parent.child2 = leaf;
    m_nodes[ sibling ].parent = newParent;
    m_nodes[ leaf ].parent = newParent;
    if ( NullProxy != oldParent )
    {
        if ( m_nodes[ oldParent ].child1 == sibling )
            m_nodes[ oldParent ].child1 = newParent;
        else
            m_nodes[ oldParent ].child2 = newParent;
    }
    else
    {
        m_root = newParent;
    }

    RefitUpwards( m_nodes[ leaf ].parent );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::RemoveLeaf( Int32 leaf )
{
    if ( leaf == m_root )
    {
        m_root = NullProxy;
        return;
    }
    const Int32 parent = m_nodes[ leaf ].parent;
    const Int32 grandParent = m_nodes[ parent ].parent;
    const Int32 sibling = m_nodes[ parent ].child1 == leaf ? m_nodes[ parent ].child2 : m_nodes[ parent ].child1;

    if ( NullProxy != grandParent )
    {
        if ( m_nodes[ grandParent ].child1 == parent )
            m_nodes[ grandParent ].child1 = sibling;
        else
            m_nodes[ grandParent ].child2 = sibling;
        m_nodes[ sibling ].parent = grandParent;
        FreeNode( parent );
        RefitUpwards( grandParent );
    }
    else
    {
        m_root = sibling;
        m_nodes[ sibling ].parent = NullProxy;
        FreeNode( parent );
    }
    m_nodes[ leaf ].parent = NullProxy;
}

/*-------------------------------------------------------------------------*/

/* walks to the root: balances each node, then refits its height and box from its children */
template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::RefitUpwards( Int32 node )
{
    Int32 index = node;
    while ( NullProxy != index )
    {
        index = Balance( index );
        SNode& n = m_nodes[ index ];
        const SNode& c1 = m_nodes[ n.child1 ];
        const SNode& c2 = m_nodes[ n.child2 ];
        n.height = 1 + ( c1.height < c2.height ? c2.height : c1.height );
        n.bounds = c1.bounds.Merged( c2.bounds );
        index = n.parent;
    }
}

/*-------------------------------------------------------------------------*/

/*
 *  When the children of node A differ by more than one in height, the higher child (C, or B for the other side) is
 *  rotated up: it takes A's place, A becomes its first child and receives the lower grandchild, C keeps the higher.
 *  Returns the node now in A's place.
 */
template < typename T, typename TUserData >
Int32
CTDynamicAabbTree< T, TUserData >::Balance( Int32 iA )
{
    SNode* nodes = &m_nodes[ 0 ];
    SNode& A = nodes[ iA ];
    if ( A.IsLeaf() || A.height < 2 )
        return iA;

    const Int32 iB = A.child1;
    const Int32 iC = A.child2;
    SNode& B = nodes[ iB ];
    SNode& C = nodes[ iC ];
    const Int32 balance = C.height - B.height;

    if ( balance > 1 )
    {
        /* rotate C up */
        const Int32 iF = C.child1;
        const Int32 iG = C.child2;
        SNode& F = nodes[ iF ];
        SNode& G = nodes[ iG ];

        C.child1 = iA;
        C.parent = A.parent;
        A.parent = iC;
        if ( NullProxy != C.parent )
        {
            if ( nodes[ C.parent ].child1 == iA )
                nodes[ C.parent ].child1 = iC;
            else
                nodes[ C.parent ].child2 = iC;
        }
        else
        {
            m_root = iC;
        }

        if ( F.height > G.height )
        {
            C.child2 = iF;
            A.child2 = iG;
            G.parent = iA;
            A.bounds = B.bounds.Merged( G.bounds );
            C.bounds = A.bounds.Merged( F.bounds );
            A.height = 1 + ( B.height < G.height ? G.height : B.height );
            C.height = 1 + ( A.height < F.height ? F.height : A.height );
        }
        else
        {
            C.child2 = iG;
            A.child2 = iF;
            F.parent = iA;
            A.bounds = B.bounds.Merged( F.bounds );
            C.bounds = A.bounds.Merged( G.bounds );
            A.height = 1 + ( B.height < F.height ? F.height : B.height );
            C.height = 1 + ( A.height < G.height ? G.height : A.height );
        }
        return iC;
    }

    if ( balance < -1 )
    {
        /* rotate B up */
        const Int32 iD = B.child1;
        const Int32 iE = B.child2;
        SNode& D = nodes[ iD ];
        SNode& E = nodes[ iE ];

        B.child1 = iA;
        B.parent = A.parent;
        A.parent = iB;
        if ( NullProxy != B.parent )
        {
            if ( nodes[ B.parent ].child1 == iA )
                nodes[ B.parent ].child1 = iB;
            else
                nodes[ B.parent ].child2 = iB;
        }
        else
        {
            m_root = iB;
        }

        if ( D.height > E.height )
        {
            B.child2 = iD;
            A.child1 = iE;
            E.parent = iA;
            A.bounds = C.bounds.Merged( E.bounds );
            B.bounds = A.bounds.Merged( D.bounds );
            A.height = 1 + ( C.height < E.height ? E.height : C.height );
            B.height = 1 + ( A.height < D.height ? D.height : A.height );
        }
        else
        {
            B.child2 = iE;
            A.child1 = iD;
            D.parent = iA;
            A.bounds = C.bounds.Merged( D.bounds );
            B.bounds = A.bounds.Merged( E.bounds );
            A.height = 1 + ( C.height < D.height ? D.height : C.height );
            B.height = 1 + ( A.height < E.height ? E.height : A.height );
        }
        return iB;
    }

    return iA;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
UInt32
CTDynamicAabbTree< T, TUserData >::GetHeight( void ) const
{
    return NullProxy == m_root ? 0 : (UInt32) m_nodes[ m_root ].height;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
typename CTDynamicAabbTree< T, TUserData >::TBox
CTDynamicAabbTree< T, TUserData >::GetRootBounds( void ) const
{
    return NullProxy == m_root ? TBox() : m_nodes[ m_root ].bounds;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
void
CTDynamicAabbTree< T, TUserData >::ShiftOrigin( const TVector& offset )
{
    for ( UInt32 i=0; i<(UInt32) m_nodes.size(); ++i )
    {
        SNode& n = m_nodes[ i ];
        if ( 0 <= n.height )
        {
            n.bounds.min = n.bounds.min - offset;
            n.bounds.max = n.bounds.max - offset;
        }
    }
}

/*-------------------------------------------------------------------------*/

/* returns the height of the subtree and clears 'valid' on any inconsistency */
template < typename T, typename TUserData >
UInt32
CTDynamicAabbTree< T, TUserData >::ValidateNode( Int32 node, bool& valid ) const
{
    /* iterative: the explicit stack holds nodes whose children are checked; heights are verified per node */
    GUCEF::vector< Int32 > stack;
    stack.push_back( node );
    UInt32 visited = 0;
    while ( !stack.empty() && valid )
    {
        const Int32 index = stack.back();
        stack.pop_back();
        ++visited;
        if ( index < 0 || index >= (Int32) m_nodes.size() || visited > (UInt32) m_nodes.size() )
        {
            valid = false;
            break;
        }
        const SNode& n = m_nodes[ index ];
        if ( n.IsLeaf() )
        {
            if ( 0 != n.height || NullProxy != n.child2 )
                valid = false;
            continue;
        }
        const Int32 c1 = n.child1;
        const Int32 c2 = n.child2;
        if ( c1 < 0 || c2 < 0 || c1 >= (Int32) m_nodes.size() || c2 >= (Int32) m_nodes.size() || c1 == c2 )
        {
            valid = false;
            break;
        }
        const SNode& n1 = m_nodes[ c1 ];
        const SNode& n2 = m_nodes[ c2 ];
        if ( n1.parent != index || n2.parent != index || n1.height < 0 || n2.height < 0 )
        {
            valid = false;
            break;
        }
        const Int32 height = 1 + ( n1.height < n2.height ? n2.height : n1.height );
        if ( n.height != height )
            valid = false;
        if ( !( n.bounds == n1.bounds.Merged( n2.bounds ) ) )
            valid = false;
        stack.push_back( c1 );
        stack.push_back( c2 );
    }
    return visited;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
bool
CTDynamicAabbTree< T, TUserData >::Validate( void ) const
{
    bool valid = true;
    UInt32 treeNodes = 0;
    if ( NullProxy != m_root )
    {
        if ( m_root < 0 || m_root >= (Int32) m_nodes.size() || NullProxy != m_nodes[ m_root ].parent )
            return false;
        treeNodes = ValidateNode( m_root, valid );
    }
    if ( !valid )
        return false;

    /* leaves in the tree are the proxies; the free list holds the rest of the pool */
    UInt32 freeNodes = 0;
    Int32 index = m_freeList;
    while ( NullProxy != index )
    {
        if ( index < 0 || index >= (Int32) m_nodes.size() || -1 != m_nodes[ index ].height || freeNodes > (UInt32) m_nodes.size() )
            return false;
        ++freeNodes;
        index = m_nodes[ index ].parent;
    }
    if ( treeNodes + freeNodes != (UInt32) m_nodes.size() )
        return false;
    const UInt32 expectedTreeNodes = 0 == m_proxyCount ? 0 : 2 * m_proxyCount - 1;
    return treeNodes == expectedTreeNodes;
}

/*-------------------------------------------------------------------------*/

/* the cross products of the direction with the box axes: true when one of them separates the line from the box */
template < typename T, typename TUserData >
bool
CTDynamicAabbTree< T, TUserData >::SegmentAxesSeparate( const TRay& ray, const TBox& box )
{
    const TVector d = ray.direction.AsVector();
    const TVector o = ray.origin.ToPositionVector();
    /*
     *  v = 2 ( centre - origin ), e = 2 half extents: no halving and no division, so nothing here can overflow in
     *  fixed point for boxes and origins in range. The products with the direction round in fixed point (to nearest,
     *  ties up): deterministic, but a segment exactly touching an edge or corner of the box may be reported separated
     *  or the other way round within the rounding of the kind (2^-32 for TFixed32_32).
     */
    const TVector v = ( box.min.ToPositionVector() + box.max.ToPositionVector() ) - ( o + o );
    const TVector e = box.max - box.min;
    const T ax = TTraits::Abs( d.x );
    const T ay = TTraits::Abs( d.y );
    const T az = TTraits::Abs( d.z );
    if ( ay * e.z + az * e.y < TTraits::Abs( d.z * v.y - d.y * v.z ) )
        return true;
    if ( ax * e.z + az * e.x < TTraits::Abs( d.x * v.z - d.z * v.x ) )
        return true;
    if ( ax * e.y + ay * e.x < TTraits::Abs( d.y * v.x - d.x * v.y ) )
        return true;
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
bool
CTDynamicAabbTree< T, TUserData >::RayOverlapsBox( const TRay& ray, const T& length, const TBox& box )
{
    if ( box.IsEmpty() || TTraits::IsNegative( length ) )
        return false;
    /* the box of the segment intersected with the box is empty exactly when the box of the segment misses it */
    TBox segment;
    if ( !ClippedSegmentBox( ray, length, box, segment ) )
        return false;
    return !SegmentAxesSeparate( ray, box );
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
T
CTDynamicAabbTree< T, TUserData >::ClampRayLength( const TRay& ray, const T& maxDistance ) const
{
    if ( NullProxy == m_root )
        return maxDistance;
    /*
     *  The L1 distance to the farthest corner bounds the Euclidean distance to every point of the root box. The sum
     *  saturates at maxDistance: each axis term fits the kind in range, the sum of three may not (fixed point).
     */
    const TBox& root = m_nodes[ m_root ].bounds;
    T bound = TTraits::Zero();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T o = ray.origin.GetComponent( axis );
        const T term = TTraits::Max( TTraits::Abs( o - root.min.GetComponent( axis ) ), TTraits::Abs( root.max.GetComponent( axis ) - o ) );
        if ( !( term < maxDistance - bound ) )
            return maxDistance;
        bound = bound + term;
    }
    return bound;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
bool
CTDynamicAabbTree< T, TUserData >::ClippedSegmentBox( const TRay& ray, const T& length, const TBox& root, TBox& segment )
{
    const TVector d = ray.direction.AsVector();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        /* the extent of the segment on the axis intersected with the root's; the far end is only formed inside it */
        const T o = ray.origin.GetComponent( axis );
        const T step = d.GetComponent( axis ) * length;
        const T rootLow = root.min.GetComponent( axis );
        const T rootHigh = root.max.GetComponent( axis );
        T low = o;
        T high = o;
        if ( TTraits::IsNegative( step ) )
            low = step < rootLow - o ? rootLow : o + step;
        else
            high = rootHigh - o < step ? rootHigh : o + step;
        if ( low < rootLow )
            low = rootLow;
        if ( rootHigh < high )
            high = rootHigh;
        if ( high < low )
            return false;
        segment.min.SetComponent( axis, low );
        segment.max.SetComponent( axis, high );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
template < typename TVisitor >
void
CTDynamicAabbTree< T, TUserData >::Query( const TBox& bounds, TVisitor& visitor ) const
{
    if ( NullProxy == m_root )
        return;
    Int32 localStack[ 256 ];
    GUCEF::vector< Int32 > heapStack;
    Int32* stack = localStack;
    const UInt32 needed = (UInt32) m_nodes[ m_root ].height + 2;
    if ( needed > 256 )
    {
        heapStack.resize( needed );
        stack = &heapStack[ 0 ];
    }
    UInt32 count = 0;
    stack[ count++ ] = m_root;
    while ( 0 < count )
    {
        const Int32 index = stack[ --count ];
        const SNode& node = m_nodes[ index ];
        if ( !node.bounds.Intersects( bounds ) )
            continue;
        if ( node.IsLeaf() )
        {
            if ( !visitor.Visit( index ) )
                return;
        }
        else
        {
            stack[ count++ ] = node.child2;
            stack[ count++ ] = node.child1;
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T, typename TUserData >
template < typename TVisitor >
void
CTDynamicAabbTree< T, TUserData >::RayCast( const TRay& ray, const T& maxDistance, TVisitor& visitor ) const
{
    if ( NullProxy == m_root || TTraits::IsNegative( maxDistance ) )
        return;
    if ( !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() || !TTraits::IsFinite( maxDistance ) )
        return;

    T length = ClampRayLength( ray, maxDistance );
    const TBox root = m_nodes[ m_root ].bounds;
    TBox segment;
    if ( !ClippedSegmentBox( ray, length, root, segment ) )
        return;

    Int32 localStack[ 256 ];
    GUCEF::vector< Int32 > heapStack;
    Int32* stack = localStack;
    const UInt32 needed = (UInt32) m_nodes[ m_root ].height + 2;
    if ( needed > 256 )
    {
        heapStack.resize( needed );
        stack = &heapStack[ 0 ];
    }
    UInt32 count = 0;
    stack[ count++ ] = m_root;
    while ( 0 < count )
    {
        const Int32 index = stack[ --count ];
        const SNode& node = m_nodes[ index ];
        if ( !node.bounds.Intersects( segment ) || SegmentAxesSeparate( ray, node.bounds ) )
            continue;
        if ( node.IsLeaf() )
        {
            T clipped = length;
            if ( !visitor.Visit( index, clipped ) )
                return;
            if ( clipped < length )
            {
                if ( TTraits::IsNegative( clipped ) )
                    return;
                length = clipped;
                if ( !ClippedSegmentBox( ray, length, root, segment ) )
                    return;
            }
        }
        else
        {
            stack[ count++ ] = node.child2;
            stack[ count++ ] = node.child1;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTDynamicAabbTree< Float32, UInt32 > TFloat32DynamicAabbTree;
typedef CTDynamicAabbTree< Float64, UInt32 > TFloat64DynamicAabbTree;
typedef CTDynamicAabbTree< MATH::TFixed32_32, UInt32 > TFixed32_32DynamicAabbTree;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* the template arguments hold a comma: a macro stands in for it (a typedef name may not follow 'class') */
#define GUCEF_GEOMETRY_DYNAMICAABBTREE_COMMA ,
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTDynamicAabbTree< Float32 GUCEF_GEOMETRY_DYNAMICAABBTREE_COMMA UInt32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTDynamicAabbTree< Float64 GUCEF_GEOMETRY_DYNAMICAABBTREE_COMMA UInt32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTDynamicAabbTree< MATH::TFixed32_32 GUCEF_GEOMETRY_DYNAMICAABBTREE_COMMA UInt32 > )
#undef GUCEF_GEOMETRY_DYNAMICAABBTREE_COMMA

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H ? */
