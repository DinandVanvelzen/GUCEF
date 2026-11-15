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

#ifndef GUCEF_GEOMETRY_CTQUADTREE_H
#define GUCEF_GEOMETRY_CTQUADTREE_H

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTLOOSETREE_H
#include "gucefGEOMETRY_CTLooseTree.h"
#define GUCEF_GEOMETRY_CTLOOSETREE_H
#endif /* GUCEF_GEOMETRY_CTLOOSETREE_H ? */

#ifndef GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H
#include "gucefGEOMETRY_SpatialQueryRegions.h"
#define GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H
#endif /* GUCEF_GEOMETRY_SPATIALQUERYREGIONS_H ? */

#ifndef GUCEF_GEOMETRY_CTRECTANGLE_H
#include "gucefGEOMETRY_CTRectangle.h"
#define GUCEF_GEOMETRY_CTRECTANGLE_H
#endif /* GUCEF_GEOMETRY_CTRECTANGLE_H ? */

#ifndef GUCEF_GEOMETRY_CTCIRCLE_H
#include "gucefGEOMETRY_CTCircle.h"
#define GUCEF_GEOMETRY_CTCIRCLE_H
#endif /* GUCEF_GEOMETRY_CTCIRCLE_H ? */

namespace GUCEF {
namespace GEOMETRY {

/**
 *  The loose quadtree of section 7.8.4 of the design: items with CTRectangle bounds in a CTLooseTree (see there for the placement, the
 *  loose factor, the node capacity, the visitor form 'bool Visit( UInt32 handle, const TItem& item )' and the exact
 *  agreement of the query results with a scan of all items). Insert, Remove and Move work by handle, the form that
 *  the bounds invalidation notifications of moved and removed geo objects need.
 *
 *  - Query by a box: the items whose bounds touch or overlap the box (closed); by a circle: the items whose bounds
 *    are at most the radius from the centre.
 *  - Kinds: Float32 and Float64. Items that are not finite or inverted are refused (InvalidHandle).
 *  - Threading: one owner thread; Query is const and allocation free.
 */
template < typename T, typename TItem >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTQuadtree
{
    public:

    typedef T TValue;
    typedef CTRectangle< T > TBox;
    typedef CTCircle< T > TBall;
    typedef CTLooseTree< T, TItem, 2 > TTree;

    enum { InvalidHandle = TTree::InvalidHandle };    /**< an enumerator: no definition is needed in C++98 */

    /** Sets the world, the deepest level (at most 24), the node capacity and the loose factor (above 1); removes every item. False for invalid arguments */
    bool Initialize( const TBox& world, UInt32 maxDepth = 8, UInt32 nodeCapacity = 8, const T& looseFactor = T( 2 ) )
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( world, lo, hi );
        return m_tree.Initialize( lo, hi, maxDepth, nodeCapacity, looseFactor );
    }

    void Clear( void ) { m_tree.Clear(); }

    /** Adds an item; its handle, or InvalidHandle for invalid bounds or an uninitialised tree */
    UInt32 Insert( const TItem& item, const TBox& bounds )
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( bounds, lo, hi );
        return m_tree.Insert( item, lo, hi );
    }

    bool Remove( UInt32 handle ) { return m_tree.Remove( handle ); }

    /** New bounds for an item, which keeps its handle */
    bool Move( UInt32 handle, const TBox& bounds )
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( bounds, lo, hi );
        return m_tree.Move( handle, lo, hi );
    }

    UInt32 GetCount( void ) const { return m_tree.GetCount(); }
    bool IsValid( UInt32 handle ) const { return m_tree.IsValid( handle ); }
    const TItem& GetItem( UInt32 handle ) const { return m_tree.GetItem( handle ); }
    UInt32 GetNodeCount( void ) const { return m_tree.GetNodeCount(); }
    UInt32 GetItemDepth( UInt32 handle ) const { return m_tree.GetItemDepth( handle ); }
    bool Validate( void ) const { return m_tree.Validate(); }

    /** The bounds of an item (the empty box for an invalid handle) */
    TBox GetBounds( UInt32 handle ) const
    {
        TBox result;
        if ( m_tree.IsValid( handle ) )
        {
            const T* lo = m_tree.GetLow( handle );
            const T* hi = m_tree.GetHigh( handle );
            result.min.x = lo[ 0 ]; result.min.y = lo[ 1 ];
            result.max.x = hi[ 0 ]; result.max.y = hi[ 1 ];
        }
        return result;
    }

    template < typename TVisitor >
    void Query( const TBox& box, TVisitor& visitor ) const
    {
        T lo[ 2 ], hi[ 2 ];
        ToArrays( box, lo, hi );
        m_tree.Query( CTBoxQueryRegion< T, 2 >( lo, hi ), visitor );
    }

    template < typename TVisitor >
    void Query( const TBall& ball, TVisitor& visitor ) const
    {
        const T centre[ 2 ] = { ball.centre.x, ball.centre.y };
        m_tree.Query( CTSphereQueryRegion< T, 2 >( centre, ball.radius ), visitor );
    }

    private:

    static void ToArrays( const TBox& box, T* lo, T* hi )
    {
        lo[ 0 ] = box.min.x; lo[ 1 ] = box.min.y;
        hi[ 0 ] = box.max.x; hi[ 1 ] = box.max.y;
    }

    TTree m_tree;
};

typedef CTQuadtree< Float32, UInt32 > TFloat32QuadtreeUInt32;
typedef CTQuadtree< Float64, UInt32 > TFloat64QuadtreeUInt32;

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

#endif /* GUCEF_GEOMETRY_CTQUADTREE_H ? */
