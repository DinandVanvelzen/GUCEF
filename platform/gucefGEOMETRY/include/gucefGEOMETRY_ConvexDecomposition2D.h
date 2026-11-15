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

#ifndef GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H
#define GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H

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
 *  The reusable work arrays of DecomposeIntoConvex: the triangulator and the half edges of the triangulation. One
 *  thread at a time; a call allocates only when the input is larger than any earlier one (plus the output growth).
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTConvexDecomposition2DWorkspace
{
    public:

    struct SHalfEdge
    {
        UInt32 origin;      /**< point index */
        UInt32 next;
        UInt32 prev;
        UInt32 twin;        /**< NoEdge on the boundary */
        bool alive;
    };

    /** An undirected edge key and the half edge it came from, for the twin search */
    struct SEdgeKey
    {
        UInt64 key;
        UInt32 halfEdge;

        bool operator<( const SEdgeKey& other ) const { return key < other.key || ( key == other.key && halfEdge < other.halfEdge ); }
    };

    enum { NoEdge = 0xFFFFFFFFU };

    CTPolygonTriangulator< T > triangulator;
    GUCEF::vector< UInt32 > triangles;
    GUCEF::vector< SHalfEdge > halfEdges;
    GUCEF::vector< SEdgeKey > keys;
    GUCEF::vector< MATH::CTPoint2D< T > > points;
    GUCEF::vector< UInt32 > holeCounts;
    GUCEF::vector< UInt8 > visited;
    GUCEF::vector< UInt32 > indices;
    GUCEF::vector< UInt32 > sizes;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Decomposition of a polygon with holes into convex pieces by Hertel-Mehlhorn (section 7.9 of the design): the ear
 *  clipping triangulation of CTPolygonTriangulator (hole bridges count as diagonals), then every diagonal is removed,
 *  in the order of the triangles, when both of its end points stay convex (a left turn or straight on, exact
 *  predicate) in the merged piece. Every piece is convex and counter-clockwise; the pieces tile the polygon. For a simple
 *  polygon with r reflex vertices the count lies between ceil( r / 2 ) + 1 (every convex decomposition) and
 *  min( n - 2, 2 r + 1 ) (at most four times the optimum).
 *
 *  'points' holds the outer ring ('outerCount' points) followed by the holes, as for CTPolygonTriangulator.
 *  'pieceIndices' receives the point indices of the pieces one after another, 'pieceSizes' the number of points of each
 *  piece. The status is that of the triangulation; without triangles there are no pieces. Every kind of CTPredicates.
 *  Allocation as described for the workspace; not realtime-safe.
 */
template < typename T >
TTriangulationStatus
DecomposeIntoConvex( const MATH::CTPoint2D< T >* points, UInt32 outerCount, const UInt32* holeCounts, UInt32 holeCount,
                     GUCEF::vector< UInt32 >& pieceIndices, GUCEF::vector< UInt32 >& pieceSizes, CTConvexDecomposition2DWorkspace< T >& workspace )
{
    typedef CTConvexDecomposition2DWorkspace< T > TWorkspace;
    typedef typename TWorkspace::SHalfEdge THalfEdge;
    typedef typename TWorkspace::SEdgeKey TEdgeKey;
    typedef CTPredicates< T > TPredicates;
    const UInt32 noEdge = (UInt32) TWorkspace::NoEdge;

    pieceIndices.clear();
    pieceSizes.clear();
    GUCEF::vector< UInt32 >& triangles = workspace.triangles;
    const TTriangulationStatus status = workspace.triangulator.Triangulate( points, outerCount, holeCounts, holeCount, triangles );
    if ( triangles.empty() )
        return status;

    // the half edges of the triangles: 3 t + k runs from corner k to corner k + 1
    const UInt32 edgeCount = (UInt32) triangles.size();
    GUCEF::vector< THalfEdge >& edges = workspace.halfEdges;
    edges.resize( edgeCount );
    GUCEF::vector< TEdgeKey >& keys = workspace.keys;
    keys.resize( edgeCount );
    for ( UInt32 h=0; h<edgeCount; ++h )
    {
        const UInt32 t = h - h % 3;
        THalfEdge& e = edges[ h ];
        e.origin = triangles[ h ];
        e.next = t + ( h - t + 1 ) % 3;
        e.prev = t + ( h - t + 2 ) % 3;
        e.twin = noEdge;
        e.alive = true;
        const UInt32 a = triangles[ h ];
        const UInt32 b = triangles[ e.next ];
        keys[ h ].key = ( (UInt64) ( a < b ? a : b ) << 32 ) | (UInt64) ( a < b ? b : a );
        keys[ h ].halfEdge = h;
    }
    std::sort( keys.begin(), keys.end() );
    for ( UInt32 k=0; k+1<edgeCount; ++k )
    {
        if ( keys[ k ].key != keys[ k + 1 ].key )
            continue;
        const UInt32 h = keys[ k ].halfEdge;
        const UInt32 g = keys[ k + 1 ].halfEdge;
        // twins run in opposite directions
        if ( noEdge == edges[ h ].twin && noEdge == edges[ g ].twin && edges[ h ].origin != edges[ g ].origin )
        {
            edges[ h ].twin = g;
            edges[ g ].twin = h;
            ++k;
        }
    }

    // remove the diagonals that are not essential
    for ( UInt32 h=0; h<edgeCount; ++h )
    {
        const UInt32 g = edges[ h ].twin;
        if ( noEdge == g || g < h || !edges[ h ].alive )
            continue;
        const MATH::CTPoint2D< T >& u = points[ edges[ h ].origin ];
        const MATH::CTPoint2D< T >& v = points[ edges[ g ].origin ];
        const MATH::CTPoint2D< T >& beforeU = points[ edges[ edges[ h ].prev ].origin ];
        const MATH::CTPoint2D< T >& afterU = points[ edges[ edges[ edges[ g ].next ].next ].origin ];
        const MATH::CTPoint2D< T >& beforeV = points[ edges[ edges[ g ].prev ].origin ];
        const MATH::CTPoint2D< T >& afterV = points[ edges[ edges[ edges[ h ].next ].next ].origin ];
        const Int32 turnU = TPredicates::Orient2D( beforeU, u, afterU );
        const Int32 turnV = TPredicates::Orient2D( beforeV, v, afterV );
        const bool convexU = turnU > 0 || ( 0 == turnU && !( u.x < beforeU.x && u.x < afterU.x ) && !( beforeU.x < u.x && afterU.x < u.x ) &&
                                            !( u.y < beforeU.y && u.y < afterU.y ) && !( beforeU.y < u.y && afterU.y < u.y ) && !( beforeU.x == afterU.x && beforeU.y == afterU.y ) );
        const bool convexV = turnV > 0 || ( 0 == turnV && !( v.x < beforeV.x && v.x < afterV.x ) && !( beforeV.x < v.x && afterV.x < v.x ) &&
                                            !( v.y < beforeV.y && v.y < afterV.y ) && !( beforeV.y < v.y && afterV.y < v.y ) && !( beforeV.x == afterV.x && beforeV.y == afterV.y ) );
        if ( !convexU || !convexV )
            continue;
        const UInt32 hp = edges[ h ].prev, hn = edges[ h ].next, gp = edges[ g ].prev, gn = edges[ g ].next;
        edges[ hp ].next = gn;
        edges[ gn ].prev = hp;
        edges[ gp ].next = hn;
        edges[ hn ].prev = gp;
        edges[ h ].alive = false;
        edges[ g ].alive = false;
    }

    // the pieces: the cycles of the live half edges, in the order of their first half edge
    GUCEF::vector< UInt8 >& visited = workspace.visited;
    visited.assign( edgeCount, 0 );
    for ( UInt32 h=0; h<edgeCount; ++h )
    {
        if ( !edges[ h ].alive || 0 != visited[ h ] )
            continue;
        UInt32 size = 0;
        UInt32 e = h;
        do
        {
            visited[ e ] = 1;
            pieceIndices.push_back( edges[ e ].origin );
            ++size;
            e = edges[ e ].next;
        }
        while ( e != h && size <= edgeCount );
        pieceSizes.push_back( size );
    }
    return status;
}

/*-------------------------------------------------------------------------*/

/** As the array form, without holes and with a temporary workspace */
template < typename T >
TTriangulationStatus
DecomposeIntoConvex( const MATH::CTPoint2D< T >* ring, UInt32 count, GUCEF::vector< UInt32 >& pieceIndices, GUCEF::vector< UInt32 >& pieceSizes )
{
    CTConvexDecomposition2DWorkspace< T > workspace;
    return DecomposeIntoConvex( ring, count, GUCEF_NULL, 0, pieceIndices, pieceSizes, workspace );
}

/*-------------------------------------------------------------------------*/

/** The pieces of a polygon with holes as polygons (counter-clockwise), with the given workspace */
template < typename T >
TTriangulationStatus
DecomposeIntoConvex( const CTPolygonWithHoles2D< T >& polygon, GUCEF::vector< CTPolygon2D< T > >& pieces, CTConvexDecomposition2DWorkspace< T >& workspace )
{
    workspace.points.clear();
    workspace.holeCounts.clear();
    workspace.points.insert( workspace.points.end(), polygon.outer.points.begin(), polygon.outer.points.end() );
    for ( size_t h=0; h<polygon.holes.size(); ++h )
    {
        workspace.points.insert( workspace.points.end(), polygon.holes[ h ].points.begin(), polygon.holes[ h ].points.end() );
        workspace.holeCounts.push_back( (UInt32) polygon.holes[ h ].points.size() );
    }
    GUCEF::vector< UInt32 >& indices = workspace.indices;
    GUCEF::vector< UInt32 >& sizes = workspace.sizes;
    const TTriangulationStatus status = DecomposeIntoConvex( workspace.points.empty() ? static_cast< const MATH::CTPoint2D< T >* >( GUCEF_NULL ) : &workspace.points[ 0 ],
                                                             (UInt32) polygon.outer.points.size(), workspace.holeCounts.empty() ? static_cast< const UInt32* >( GUCEF_NULL ) : &workspace.holeCounts[ 0 ],
                                                             (UInt32) workspace.holeCounts.size(), indices, sizes, workspace );
    pieces.clear();
    pieces.resize( sizes.size() );
    size_t offset = 0;
    for ( size_t p=0; p<sizes.size(); ++p )
    {
        pieces[ p ].points.reserve( sizes[ p ] );
        for ( UInt32 i=0; i<sizes[ p ]; ++i )
            pieces[ p ].points.push_back( workspace.points[ indices[ offset + i ] ] );
        offset += sizes[ p ];
    }
    return status;
}

/*-------------------------------------------------------------------------*/

/** The pieces of a polygon as polygons (counter-clockwise), with a temporary workspace */
template < typename T >
TTriangulationStatus
DecomposeIntoConvex( const CTPolygon2D< T >& polygon, GUCEF::vector< CTPolygon2D< T > >& pieces )
{
    CTConvexDecomposition2DWorkspace< T > workspace;
    return DecomposeIntoConvex( CTPolygonWithHoles2D< T >( polygon ), pieces, workspace );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CONVEXDECOMPOSITION2D_H ? */
