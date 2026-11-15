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

#ifndef GUCEF_GEOMETRY_CTLOOSETREE_H
#define GUCEF_GEOMETRY_CTLOOSETREE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H
#include "gucefGEOMETRY_CTSpatialItemStore.h"
#define GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H
#endif /* GUCEF_GEOMETRY_CTSPATIALITEMSTORE_H ? */

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
 *  The loose tree behind CTOctree (Dim 3) and CTQuadtree (Dim 2), section 7.8.4 of the design. The cells halve per
 *  level from a root cube that covers the world; each node also has a loose box, its cell widened by the loose factor k
 *  (2 by default: the loose box has k times the half size of the cell), and an item is stored in the deepest node on the
 *  path of its centre whose loose box contains the item's bounds, so an item never has to be split or stored twice:
 *  insert, remove and move by handle are O( depth ).
 *
 *  - Placement: the item goes down from the root while the child cell is not smaller than the item's half extent divided
 *    by ( k - 1 ), the depth is below the maximum, and the node is internal or already holds the node capacity of items
 *    (a leaf with room keeps the item, so a sparse tree stays shallow); the test that decides is made on the stored
 *    loose box of the node, so a rounding error cannot place an item outside it. An item whose centre is outside the
 *    root cell, or that is larger than the root loose box allows, lives in the root. Nodes are created when an item
 *    needs them and removed when they become empty.
 *  - Query: a region (SpatialQueryRegions.h: box, sphere, frustum) visits the nodes whose loose box it overlaps; the
 *    root is always visited. The items of the result are those the region accepts, exactly as a scan of all items
 *    would find them. The visitor is a template functor with 'bool Visit( UInt32 handle, const TItem& item )' that
 *    returns false to stop; the order is the depth first order of the nodes by child index, items in the order they
 *    were added to the node: deterministic.
 *  - Kinds: float kinds. Handles: see CTSpatialItemStore.
 *  - Threading: owned by one thread; queries are const and allocation free (a fixed stack).
 *  - Realtime: not realtime-safe (Insert allocates); Query allocates nothing (the visitor decides the rest), but is not annotated for RTSan.
 */
template < typename T, typename TItem, UInt32 Dim >
class CTLooseTree
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef CTSpatialItemStore< T, TItem, Dim > TStore;

    enum { InvalidHandle = TStore::InvalidHandle };    /**< an enumerator: no definition is needed in C++98 */
    enum { ChildCount = (UInt32) 1 << Dim };    /**< an enumerator: no definition is needed in C++98 */
    enum { MaxDepthLimit = 24 };    /**< an enumerator: no definition is needed in C++98 */

    CTLooseTree( void ) : m_store(), m_nodes(), m_itemNode(), m_itemPrev(), m_itemNext(), m_root( InvalidHandle ), m_firstFreeNode( InvalidHandle ),
                          m_maxDepth( 8 ), m_capacity( 8 ), m_loose( T( 2 ) ), m_liveNodes( 0 ), m_worldHalf( T( 0 ) ), m_initialized( false )
    {
        for ( UInt32 a=0; a<Dim; ++a )
            m_worldCentre[ a ] = T( 0 );
    }

    /**
     *  Sets the world (the cube around the box from 'worldLo' to 'worldHi'), the deepest level, the node capacity and the
     *  loose factor (above 1), and removes every item. False (unchanged) for a world that is empty or not finite, a loose
     *  factor not above 1 or a maximum depth above MaxDepthLimit.
     */
    bool Initialize( const T* worldLo, const T* worldHi, UInt32 maxDepth, UInt32 nodeCapacity, const T& looseFactor )
    {
        if ( !TStore::AreBoundsValid( worldLo, worldHi ) || !( looseFactor > TTraits::One() ) || !TTraits::IsFinite( looseFactor ) || maxDepth > MaxDepthLimit || 0 == nodeCapacity )
            return false;
        m_initialized = false;
        ClearItems();
        m_maxDepth = maxDepth;
        m_capacity = nodeCapacity;
        m_loose = looseFactor;
        T half = TTraits::Zero();
        for ( UInt32 a=0; a<Dim; ++a )
        {
            const T extent = worldHi[ a ] - worldLo[ a ];
            m_worldCentre[ a ] = worldLo[ a ] + extent * TTraits::Half();
            if ( extent * TTraits::Half() > half )
                half = extent * TTraits::Half();
        }
        if ( !( half > TTraits::Zero() ) )
            half = TTraits::One();
        m_worldHalf = half;
        m_root = CreateNode( m_worldCentre, m_worldHalf, 0, InvalidHandle );
        m_initialized = true;
        return true;
    }

    /** Removes every item and node (the world stays) */
    void Clear( void )
    {
        ClearItems();
        if ( m_initialized )
            m_root = CreateNode( m_worldCentre, m_worldHalf, 0, InvalidHandle );
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetCount( void ) const { return m_store.GetCount(); }
    GUCEF_GEOMETRY_FORCE_INLINE bool IsValid( UInt32 handle ) const { return m_store.IsValid( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE const TItem& GetItem( UInt32 handle ) const { return m_store.GetItem( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE const T* GetLow( UInt32 handle ) const { return m_store.GetLow( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE const T* GetHigh( UInt32 handle ) const { return m_store.GetHigh( handle ); }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetNodeCount( void ) const { return m_liveNodes; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetHandleLimit( void ) const { return m_store.GetHandleLimit(); }

    /** The depth of the node that holds the item (0 is the root); 0xFFFFFFFF for an invalid handle */
    UInt32 GetItemDepth( UInt32 handle ) const
    {
        return m_store.IsValid( handle ) ? m_nodes[ m_itemNode[ handle ] ].depth : 0xFFFFFFFFU;
    }

    /** Adds an item with the closed bounds 'lo' to 'hi'; the handle, or InvalidHandle for invalid bounds or an uninitialised tree */
    UInt32 Insert( const TItem& item, const T* lo, const T* hi )
    {
        if ( InvalidHandle == m_root )
            return InvalidHandle;
        const UInt32 handle = m_store.Allocate( item, lo, hi );
        if ( InvalidHandle == handle )
            return InvalidHandle;
        if ( handle >= m_itemNode.size() )
        {
            m_itemNode.resize( handle + 1, InvalidHandle );
            m_itemPrev.resize( handle + 1, InvalidHandle );
            m_itemNext.resize( handle + 1, InvalidHandle );
        }
        Link( handle );
        return handle;
    }

    /** Removes an item; false for an invalid handle */
    bool Remove( UInt32 handle )
    {
        if ( !m_store.IsValid( handle ) )
            return false;
        Unlink( handle );
        return m_store.Release( handle );
    }

    /** Gives an item new bounds (it keeps its handle); false (unchanged) for an invalid handle or invalid bounds */
    bool Move( UInt32 handle, const T* lo, const T* hi )
    {
        if ( !m_store.IsValid( handle ) || !TStore::AreBoundsValid( lo, hi ) )
            return false;
        Unlink( handle );
        m_store.SetBounds( handle, lo, hi );
        Link( handle );
        return true;
    }

    /** Visits the items that 'region' accepts; see the class description */
    template < typename TRegion, typename TVisitor >
    void Query( const TRegion& region, TVisitor& visitor ) const
    {
        if ( InvalidHandle == m_root )
            return;
        SStackEntry stack[ MaxDepthLimit * ChildCount + 2 ];
        UInt32 top = 0;
        stack[ top ].node = m_root;
        stack[ top ].mask = region.GetInitialMask();
        ++top;
        while ( top > 0 )
        {
            --top;
            const SNode& node = m_nodes[ stack[ top ].node ];
            UInt32 mask = stack[ top ].mask;
            if ( stack[ top ].node != m_root && !region.OverlapsNode( node.lo, node.hi, mask ) )
                continue;
            for ( UInt32 it=node.head; InvalidHandle!=it; it=m_itemNext[ it ] )
                if ( region.Accepts( m_store.GetLow( it ), m_store.GetHigh( it ) ) )
                    if ( !visitor.Visit( it, m_store.GetItem( it ) ) )
                        return;
            for ( UInt32 c=ChildCount; c-->0; )
                if ( InvalidHandle != node.children[ c ] )
                {
                    stack[ top ].node = node.children[ c ];
                    stack[ top ].mask = mask;
                    ++top;
                }
        }
    }

    /**
     *  Checks the invariants: every item is in a live node whose loose box contains its bounds, the lists and counts
     *  agree, a node's box contains its children's boxes. True when all hold.
     */
    bool Validate( void ) const
    {
        if ( InvalidHandle == m_root )
            return 0 == m_store.GetCount();
        UInt32 found = 0;
        for ( UInt32 n=0; n<m_nodes.size(); ++n )
        {
            if ( !m_nodes[ n ].used )
                continue;
            const SNode& node = m_nodes[ n ];
            UInt32 count = 0;
            UInt32 previous = InvalidHandle;
            for ( UInt32 it=node.head; InvalidHandle!=it; it=m_itemNext[ it ] )
            {
                if ( !m_store.IsValid( it ) || m_itemNode[ it ] != n || m_itemPrev[ it ] != previous )
                    return false;
                if ( n != m_root )
                    for ( UInt32 a=0; a<Dim; ++a )
                        if ( m_store.GetLow( it )[ a ] < node.lo[ a ] || m_store.GetHigh( it )[ a ] > node.hi[ a ] )
                            return false;
                previous = it;
                ++count;
            }
            if ( count != node.itemCount || previous != node.tail )
                return false;
            found += count;
            for ( UInt32 c=0; c<ChildCount; ++c )
                if ( InvalidHandle != node.children[ c ] )
                {
                    const SNode& child = m_nodes[ node.children[ c ] ];
                    if ( !child.used || child.parent != n )
                        return false;
                    for ( UInt32 a=0; a<Dim; ++a )
                        if ( child.lo[ a ] < node.lo[ a ] || child.hi[ a ] > node.hi[ a ] )
                            return false;
                }
        }
        return found == m_store.GetCount();
    }

    private:

    void ClearItems( void )
    {
        m_store.Clear();
        m_nodes.clear();
        m_itemNode.clear();
        m_itemPrev.clear();
        m_itemNext.clear();
        m_firstFreeNode = InvalidHandle;
        m_liveNodes = 0;
        m_root = InvalidHandle;
    }

    struct SNode
    {
        T centre[ Dim ];
        T half;
        T lo[ Dim ];                    /**< the loose box, grown to contain the loose boxes of the children */
        T hi[ Dim ];
        UInt32 children[ ChildCount ];
        UInt32 parent;
        UInt32 head;
        UInt32 tail;
        UInt32 itemCount;
        UInt32 childCount;
        UInt32 depth;
        bool used;
    };

    struct SStackEntry
    {
        UInt32 node;
        UInt32 mask;
    };

    UInt32 CreateNode( const T* centre, const T& half, UInt32 depth, UInt32 parent )
    {
        UInt32 index;
        if ( InvalidHandle != m_firstFreeNode )
        {
            index = m_firstFreeNode;
            m_firstFreeNode = m_nodes[ index ].parent;
        }
        else
        {
            index = (UInt32) m_nodes.size();
            m_nodes.push_back( SNode() );
        }
        SNode& node = m_nodes[ index ];
        node.half = half;
        const T looseHalf = half * m_loose;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            node.centre[ a ] = centre[ a ];
            node.lo[ a ] = centre[ a ] - looseHalf;
            node.hi[ a ] = centre[ a ] + looseHalf;
        }
        for ( UInt32 c=0; c<ChildCount; ++c )
            node.children[ c ] = InvalidHandle;
        node.parent = parent;
        node.head = node.tail = InvalidHandle;
        node.itemCount = node.childCount = 0;
        node.depth = depth;
        node.used = true;
        ++m_liveNodes;
        // the boxes of the ancestors contain the box of the child
        for ( UInt32 p=parent; InvalidHandle!=p; p=m_nodes[ p ].parent )
            for ( UInt32 a=0; a<Dim; ++a )
            {
                if ( m_nodes[ index ].lo[ a ] < m_nodes[ p ].lo[ a ] )
                    m_nodes[ p ].lo[ a ] = m_nodes[ index ].lo[ a ];
                if ( m_nodes[ index ].hi[ a ] > m_nodes[ p ].hi[ a ] )
                    m_nodes[ p ].hi[ a ] = m_nodes[ index ].hi[ a ];
            }
        return index;
    }

    bool NodeContains( UInt32 node, const T* lo, const T* hi ) const
    {
        for ( UInt32 a=0; a<Dim; ++a )
            if ( lo[ a ] < m_nodes[ node ].lo[ a ] || hi[ a ] > m_nodes[ node ].hi[ a ] )
                return false;
        return true;
    }

    /** Puts the item into the deepest suitable node (see the class description) */
    void Link( UInt32 handle )
    {
        const T* lo = m_store.GetLow( handle );
        const T* hi = m_store.GetHigh( handle );
        T centre[ Dim ];
        T halfExtent = TTraits::Zero();
        for ( UInt32 a=0; a<Dim; ++a )
        {
            const T extent = hi[ a ] - lo[ a ];
            centre[ a ] = lo[ a ] + extent * TTraits::Half();
            if ( extent * TTraits::Half() > halfExtent )
                halfExtent = extent * TTraits::Half();
        }
        const T requiredHalf = halfExtent / ( m_loose - TTraits::One() );

        UInt32 node = m_root;
        bool insideRoot = true;
        for ( UInt32 a=0; a<Dim; ++a )
            if ( TTraits::Abs( centre[ a ] - m_nodes[ m_root ].centre[ a ] ) > m_nodes[ m_root ].half )
                insideRoot = false;
        while ( insideRoot )
        {
            const SNode& current = m_nodes[ node ];
            if ( current.depth >= m_maxDepth )
                break;
            const T childHalf = current.half * TTraits::Half();
            if ( childHalf < requiredHalf )
                break;
            if ( 0 == current.childCount && current.itemCount < m_capacity )
                break;
            UInt32 index = 0;
            T childCentre[ Dim ];
            for ( UInt32 a=0; a<Dim; ++a )
            {
                const bool upper = !( centre[ a ] < current.centre[ a ] );
                if ( upper )
                    index |= (UInt32) 1 << a;
                childCentre[ a ] = upper ? current.centre[ a ] + childHalf : current.centre[ a ] - childHalf;
            }
            UInt32 child = current.children[ index ];
            if ( InvalidHandle == child )
            {
                child = CreateNode( childCentre, childHalf, current.depth + 1, node );
                m_nodes[ node ].children[ index ] = child;
                ++m_nodes[ node ].childCount;
            }
            // the stored box decides: an item that does not fit stays in this node
            if ( !NodeContains( child, lo, hi ) )
            {
                if ( 0 == m_nodes[ child ].itemCount && 0 == m_nodes[ child ].childCount )
                {
                    // the node was created for this item only: take it away again, but not its parent
                    m_nodes[ node ].children[ index ] = InvalidHandle;
                    --m_nodes[ node ].childCount;
                    m_nodes[ child ].used = false;
                    m_nodes[ child ].parent = m_firstFreeNode;
                    m_firstFreeNode = child;
                    --m_liveNodes;
                }
                break;
            }
            node = child;
        }
        // append to the node's list
        SNode& target = m_nodes[ node ];
        m_itemNode[ handle ] = node;
        m_itemNext[ handle ] = InvalidHandle;
        m_itemPrev[ handle ] = target.tail;
        if ( InvalidHandle != target.tail )
            m_itemNext[ target.tail ] = handle;
        else
            target.head = handle;
        target.tail = handle;
        ++target.itemCount;
    }

    void Unlink( UInt32 handle )
    {
        const UInt32 node = m_itemNode[ handle ];
        SNode& owner = m_nodes[ node ];
        const UInt32 prev = m_itemPrev[ handle ];
        const UInt32 next = m_itemNext[ handle ];
        if ( InvalidHandle != prev )
            m_itemNext[ prev ] = next;
        else
            owner.head = next;
        if ( InvalidHandle != next )
            m_itemPrev[ next ] = prev;
        else
            owner.tail = prev;
        --owner.itemCount;
        m_itemNode[ handle ] = InvalidHandle;
        RemoveNodeIfEmpty( node );
    }

    /** Frees a node without items and children and then its parent if that became empty; the root stays */
    void RemoveNodeIfEmpty( UInt32 node )
    {
        while ( InvalidHandle != node && node != m_root && 0 == m_nodes[ node ].itemCount && 0 == m_nodes[ node ].childCount )
        {
            const UInt32 parent = m_nodes[ node ].parent;
            SNode& owner = m_nodes[ parent ];
            for ( UInt32 c=0; c<ChildCount; ++c )
                if ( owner.children[ c ] == node )
                {
                    owner.children[ c ] = InvalidHandle;
                    --owner.childCount;
                    break;
                }
            m_nodes[ node ].used = false;
            m_nodes[ node ].parent = m_firstFreeNode;
            m_firstFreeNode = node;
            --m_liveNodes;
            node = parent;
        }
    }

    TStore m_store;
    GUCEF::vector< SNode > m_nodes;
    GUCEF::vector< UInt32 > m_itemNode;
    GUCEF::vector< UInt32 > m_itemPrev;
    GUCEF::vector< UInt32 > m_itemNext;
    UInt32 m_root;
    UInt32 m_firstFreeNode;
    UInt32 m_maxDepth;
    UInt32 m_capacity;
    T m_loose;
    UInt32 m_liveNodes;
    T m_worldCentre[ Dim ];
    T m_worldHalf;
    bool m_initialized;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTLOOSETREE_H ? */
