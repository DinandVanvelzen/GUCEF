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

#ifndef GUCEF_GEOMETRY_CTKDTREE_H
#define GUCEF_GEOMETRY_CTKDTREE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

#ifndef GUCEF_MATH_VECTORS_H
#include "gucefMATH_vectors.h"
#define GUCEF_MATH_VECTORS_H
#endif /* GUCEF_MATH_VECTORS_H ? */

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

/** The point type of a k-d tree by the number of dimensions */
template < typename T, UInt32 Dimensions > class CTKdTreePoint;
template < typename T > class CTKdTreePoint< T, 2 > { public: typedef MATH::CTPoint2D< T > TPoint; };
template < typename T > class CTKdTreePoint< T, 3 > { public: typedef MATH::CTPoint3D< T > TPoint; };

/**
 *  A static k-d tree over points in 2D or 3D (section 7.8.4 of the design). Build splits the index array at the median of
 *  the axis with the largest extent (an nth_element selection ordered by coordinate and then index, so the tree does not
 *  depend on the platform's sort) until a node holds at most 'leafSize' points; the points themselves are not copied: the
 *  caller keeps the array alive and unchanged while the tree is used. A point is addressed by its index in that array.
 *
 *  - FindNearest: the closest point; on equal squared distances the lowest index.
 *  - FindKNearest( point, k, indices, squaredDistances ): the k closest in increasing order of ( squared distance,
 *    index ) through a bounded max-heap in the caller's arrays of size k; returns how many were found (fewer than k when
 *    the tree has fewer points).
 *  - FindWithinRadius: every point whose squared distance is at most radius squared, in the order of the tree; the
 *    visitor form 'bool Visit( UInt32 index, const T& squaredDistance )' stops by returning false.
 *  - The squared distance is the one of ( point - other ).GetSquaredLength() of the vector class: identical for the
 *    tree and for a scan of the points. Duplicate points are separate entries. Points that are not finite are skipped by
 *    Build (their index is never returned).
 *  - Kinds: float kinds. Build allocates (the index array and the nodes); the queries do not (a fixed stack), and are
 *    allocation free (they are not annotated for RTSan; the visitor decides the rest).
 */
template < typename T, UInt32 Dimensions >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTKdTree
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef typename CTKdTreePoint< T, Dimensions >::TPoint TPoint;

    enum { MaxDepth = 64 };    /**< an enumerator: no definition is needed in C++98 */

    CTKdTree( void ) : m_points( GUCEF_NULL ), m_count( 0 ), m_leafSize( 8 ), m_indices(), m_nodes() {}

    /** Builds the tree over 'count' points (kept by pointer); 'leafSize' of at least 1 points per leaf */
    void Build( const TPoint* points, UInt32 count, UInt32 leafSize = 8 )
    {
        m_points = points;
        m_count = 0;
        m_indices.clear();
        m_nodes.clear();
        if ( 0 == leafSize )
            leafSize = 1;
        for ( UInt32 i=0; i<count; ++i )
            if ( IsPointFinite( points[ i ] ) )
                m_indices.push_back( i );
        m_count = (UInt32) m_indices.size();
        if ( 0 == m_count )
            return;
        m_leafSize = leafSize;
        m_nodes.reserve( 2 * ( m_count / leafSize + 1 ) );
        BuildNode( 0, m_count, 0 );
    }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPointCount( void ) const { return m_count; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetNodeCount( void ) const { return (UInt32) m_nodes.size(); }

    /** The index of the closest point and its squared distance; false for an empty tree */
    bool FindNearest( const TPoint& point, UInt32& index, T& squaredDistance ) const
    {
        return 1 == FindKNearest( point, 1, &index, &squaredDistance );
    }

    /**
     *  The 'k' closest points, nearest first, into 'indices' and 'squaredDistances' (arrays of at least 'k' values).
     *  Returns the number written.
     */
    UInt32 FindKNearest( const TPoint& point, UInt32 k, UInt32* indices, T* squaredDistances ) const
    {
        if ( 0 == k || 0 == m_count || !IsPointFinite( point ) )
            return 0;
        UInt32 size = 0;
        SStackEntry stack[ MaxDepth * 2 + 2 ];
        UInt32 top = 0;
        stack[ top ].node = 0;
        stack[ top ].bound = TTraits::Zero();
        ++top;
        while ( top > 0 )
        {
            --top;
            const SNode& node = m_nodes[ stack[ top ].node ];
            // the heap root is the worst of the k best so far
            if ( size == k && stack[ top ].bound > squaredDistances[ 0 ] )
                continue;
            if ( node.left < 0 )
            {
                for ( UInt32 i=node.begin; i<node.end; ++i )
                {
                    const UInt32 index = m_indices[ i ];
                    const T d = SquaredDistance( point, m_points[ index ] );
                    if ( size < k )
                    {
                        HeapPush( indices, squaredDistances, size, index, d );
                        ++size;
                    }
                    else if ( Less( d, index, squaredDistances[ 0 ], indices[ 0 ] ) )
                        HeapReplaceRoot( indices, squaredDistances, size, index, d );
                }
                continue;
            }
            const T difference = Coordinate( point, node.axis ) - node.split;
            const T planeSquared = difference * difference;
            const UInt32 nearChild = difference < TTraits::Zero() ? (UInt32) node.left : (UInt32) node.right;
            const UInt32 farChild = difference < TTraits::Zero() ? (UInt32) node.right : (UInt32) node.left;
            // the far side after the near side: pushed first, popped last
            const T parentBound = stack[ top ].bound;
            stack[ top ].node = farChild;
            stack[ top ].bound = planeSquared > parentBound ? planeSquared : parentBound;
            ++top;
            stack[ top ].node = nearChild;
            stack[ top ].bound = parentBound;
            ++top;
        }
        // heap sort in place: the largest to the end, so the arrays end up ascending
        for ( UInt32 n=size; n>1; --n )
        {
            Swap( indices, squaredDistances, 0, n - 1 );
            SiftDown( indices, squaredDistances, n - 1, 0 );
        }
        return size;
    }

    /** Visits every point at most 'radius' away (squared distance at most radius squared) */
    template < typename TVisitor >
    void FindWithinRadius( const TPoint& point, const T& radius, TVisitor& visitor ) const
    {
        if ( 0 == m_count || !IsPointFinite( point ) || !( radius >= TTraits::Zero() ) )
            return;
        const T radiusSquared = radius * radius;
        UInt32 stack[ MaxDepth * 2 + 2 ];
        UInt32 top = 0;
        stack[ top++ ] = 0;
        while ( top > 0 )
        {
            const SNode& node = m_nodes[ stack[ --top ] ];
            if ( node.left < 0 )
            {
                for ( UInt32 i=node.begin; i<node.end; ++i )
                {
                    const UInt32 index = m_indices[ i ];
                    const T d = SquaredDistance( point, m_points[ index ] );
                    if ( d <= radiusSquared && !visitor.Visit( index, d ) )
                        return;
                }
                continue;
            }
            const T difference = Coordinate( point, node.axis ) - node.split;
            const bool nearIsLeft = difference < TTraits::Zero();
            // the far side holds only points at least |difference| away along the axis
            if ( difference * difference <= radiusSquared )
                stack[ top++ ] = nearIsLeft ? (UInt32) node.right : (UInt32) node.left;
            stack[ top++ ] = nearIsLeft ? (UInt32) node.left : (UInt32) node.right;
        }
    }

    private:

    struct SNode
    {
        UInt32 begin;
        UInt32 end;
        Int32 left;         /**< -1 for a leaf */
        Int32 right;
        UInt32 axis;
        T split;            /**< the coordinate of the median point: the left holds points before it in the order of the axis, the right the others */
    };

    struct SStackEntry
    {
        UInt32 node;
        T bound;            /**< a lower bound of the squared distance to everything in the node */
    };

    class CLess
    {
        public:

        CLess( const TPoint* points, UInt32 axis ) : m_points( points ), m_axis( axis ) {}

        bool operator()( UInt32 a, UInt32 b ) const
        {
            const T ca = Coordinate( m_points[ a ], m_axis );
            const T cb = Coordinate( m_points[ b ], m_axis );
            if ( ca < cb ) return true;
            if ( cb < ca ) return false;
            return a < b;
        }

        private:

        const TPoint* m_points;
        UInt32 m_axis;
    };

    static GUCEF_GEOMETRY_FORCE_INLINE T Coordinate( const TPoint& p, UInt32 axis )
    {
        return 0 == axis ? p.x : ( 1 == axis ? p.y : CoordinateZ( p ) );
    }

    static GUCEF_GEOMETRY_FORCE_INLINE T CoordinateZ( const MATH::CTPoint2D< T >& /* p */ ) { return TTraits::Zero(); }
    static GUCEF_GEOMETRY_FORCE_INLINE T CoordinateZ( const MATH::CTPoint3D< T >& p ) { return p.z; }

    static bool IsPointFinite( const TPoint& p )
    {
        for ( UInt32 a=0; a<Dimensions; ++a )
            if ( !TTraits::IsFinite( Coordinate( p, a ) ) )
                return false;
        return true;
    }

    static GUCEF_GEOMETRY_FORCE_INLINE T SquaredDistance( const TPoint& a, const TPoint& b )
    {
        return ( a - b ).GetSquaredLength();
    }

    /** Orders by squared distance, then by index */
    static GUCEF_GEOMETRY_FORCE_INLINE bool Less( const T& da, UInt32 ia, const T& db, UInt32 ib )
    {
        return da < db || ( !( db < da ) && ia < ib );
    }

    UInt32 BuildNode( UInt32 begin, UInt32 end, UInt32 depth )
    {
        const UInt32 index = (UInt32) m_nodes.size();
        m_nodes.push_back( SNode() );
        m_nodes[ index ].begin = begin;
        m_nodes[ index ].end = end;
        m_nodes[ index ].left = m_nodes[ index ].right = -1;
        m_nodes[ index ].axis = 0;
        m_nodes[ index ].split = TTraits::Zero();
        if ( end - begin <= m_leafSize || depth + 1 >= MaxDepth )
            return index;
        // the axis of the largest extent
        UInt32 axis = 0;
        T bestExtent = TTraits::Zero();
        for ( UInt32 a=0; a<Dimensions; ++a )
        {
            T low = Coordinate( m_points[ m_indices[ begin ] ], a );
            T high = low;
            for ( UInt32 i=begin+1; i<end; ++i )
            {
                const T c = Coordinate( m_points[ m_indices[ i ] ], a );
                if ( c < low ) low = c;
                if ( c > high ) high = c;
            }
            if ( high - low > bestExtent )
            {
                bestExtent = high - low;
                axis = a;
            }
        }
        const UInt32 middle = begin + ( end - begin ) / 2;
        std::nth_element( m_indices.begin() + begin, m_indices.begin() + middle, m_indices.begin() + end, CLess( m_points, axis ) );
        const T split = Coordinate( m_points[ m_indices[ middle ] ], axis );
        m_nodes[ index ].axis = axis;
        m_nodes[ index ].split = split;
        const UInt32 left = BuildNode( begin, middle, depth + 1 );
        const UInt32 right = BuildNode( middle, end, depth + 1 );
        m_nodes[ index ].left = (Int32) left;
        m_nodes[ index ].right = (Int32) right;
        return index;
    }

    /* the bounded max-heap on ( squared distance, index ), the worst at the root */

    static void Swap( UInt32* indices, T* distances, UInt32 a, UInt32 b )
    {
        const UInt32 ti = indices[ a ]; indices[ a ] = indices[ b ]; indices[ b ] = ti;
        const T td = distances[ a ]; distances[ a ] = distances[ b ]; distances[ b ] = td;
    }

    static void SiftDown( UInt32* indices, T* distances, UInt32 size, UInt32 position )
    {
        for ( ;; )
        {
            UInt32 worst = position;
            const UInt32 l = 2 * position + 1;
            const UInt32 r = l + 1;
            if ( l < size && Less( distances[ worst ], indices[ worst ], distances[ l ], indices[ l ] ) )
                worst = l;
            if ( r < size && Less( distances[ worst ], indices[ worst ], distances[ r ], indices[ r ] ) )
                worst = r;
            if ( worst == position )
                return;
            Swap( indices, distances, position, worst );
            position = worst;
        }
    }

    static void HeapPush( UInt32* indices, T* distances, UInt32 size, UInt32 index, const T& d )
    {
        UInt32 position = size;
        indices[ position ] = index;
        distances[ position ] = d;
        while ( position > 0 )
        {
            const UInt32 parent = ( position - 1 ) / 2;
            if ( !Less( distances[ parent ], indices[ parent ], distances[ position ], indices[ position ] ) )
                break;
            Swap( indices, distances, parent, position );
            position = parent;
        }
    }

    static void HeapReplaceRoot( UInt32* indices, T* distances, UInt32 size, UInt32 index, const T& d )
    {
        indices[ 0 ] = index;
        distances[ 0 ] = d;
        SiftDown( indices, distances, size, 0 );
    }

    const TPoint* m_points;
    UInt32 m_count;
    UInt32 m_leafSize;
    GUCEF::vector< UInt32 > m_indices;
    GUCEF::vector< SNode > m_nodes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTKdTree< Float32, 2 > TFloat32KdTree2D;
typedef CTKdTree< Float32, 3 > TFloat32KdTree3D;
typedef CTKdTree< Float64, 2 > TFloat64KdTree2D;
typedef CTKdTree< Float64, 3 > TFloat64KdTree3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTKDTREE_H ? */
