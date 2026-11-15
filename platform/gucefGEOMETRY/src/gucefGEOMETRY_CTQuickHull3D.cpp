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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <float.h>

#include "gucefGEOMETRY_CQuickHull3DCore.h"

#ifndef GUCEF_GEOMETRY_CONVEXHULL2D_H
#include "gucefGEOMETRY_ConvexHull2D.h"
#define GUCEF_GEOMETRY_CONVEXHULL2D_H
#endif /* GUCEF_GEOMETRY_CONVEXHULL2D_H ? */

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#include "gucefGEOMETRY_CTQuickHull3D.h"
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H
#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/*
 *  The structure follows the quickhull of John E. Lloyd (QuickHull3D, a Java implementation of Barber, Dobkin and
 *  Huhdanpaa 1996): faces are polygons held as rings of half edges, a new point replaces the faces it sees by a cone of
 *  triangles over the horizon, and the cone is merged back into polygons where the neighbours are not strictly convex
 *  (the centroid of one is not below the plane of the other by more than the tolerance). Everything is addressed by
 *  index into arrays that live in the engine; -1 is the null index.
 */
struct CQuickHull3DCore::SImpl
{
    enum EMark { MARK_VISIBLE = 0, MARK_NON_CONVEX = 1, MARK_DELETED = 2 };

    struct SVertex
    {
        Float64 p[ 3 ];
        Int32 prev;
        Int32 next;
        Int32 face;         /**< the face whose outside set holds the vertex */
    };

    struct SHalfEdge
    {
        Int32 vertex;       /**< the head */
        Int32 face;
        Int32 next;
        Int32 prev;
        Int32 opposite;
    };

    struct SFace
    {
        Int32 he0;
        Float64 normal[ 3 ];
        Float64 centroid[ 3 ];
        Float64 area;
        Float64 offset;
        UInt32 numVerts;
        Int32 mark;
        Int32 outside;      /**< the first vertex of the outside set; the set continues while vertex.face is this face */
    };

    GUCEF::vector< SVertex > verts;
    GUCEF::vector< SHalfEdge > edges;
    GUCEF::vector< SFace > faces;
    Float64 tolerance;
    Int32 claimedHead, claimedTail, unclaimedHead, unclaimedTail;
    GUCEF::vector< Int32 > horizon;
    GUCEF::vector< Int32 > newFaces;
    GUCEF::vector< Int32 > discarded;
    GUCEF::vector< UInt32 > vertexIndices;
    GUCEF::vector< UInt32 > faceIndices;
    GUCEF::vector< UInt32 > faceSizes;

    SImpl( void ) : tolerance( 0.0 ), claimedHead( -1 ), claimedTail( -1 ), unclaimedHead( -1 ), unclaimedTail( -1 ) {}

    /*---------------------------------------------------------------------*/

    void Clear( void )
    {
        verts.clear();
        edges.clear();
        faces.clear();
        horizon.clear();
        newFaces.clear();
        discarded.clear();
        vertexIndices.clear();
        faceIndices.clear();
        faceSizes.clear();
        claimedHead = claimedTail = unclaimedHead = unclaimedTail = -1;
    }

    /*---------------------------------------------------------------------*/

    // vertex lists

    void ListAdd( Int32& head, Int32& tail, Int32 v )
    {
        verts[ v ].prev = tail;
        verts[ v ].next = -1;
        if ( tail >= 0 )
            verts[ tail ].next = v;
        else
            head = v;
        tail = v;
    }

    void ListInsertBefore( Int32& head, Int32 v, Int32 before )
    {
        verts[ v ].prev = verts[ before ].prev;
        if ( verts[ before ].prev < 0 )
            head = v;
        else
            verts[ verts[ before ].prev ].next = v;
        verts[ v ].next = before;
        verts[ before ].prev = v;
    }

    /** Unlinks the chain from 'first' to 'last' (the same vertex for one) */
    void ListDelete( Int32& head, Int32& tail, Int32 first, Int32 last )
    {
        const Int32 before = verts[ first ].prev;
        const Int32 after = verts[ last ].next;
        if ( before < 0 )
            head = after;
        else
            verts[ before ].next = after;
        if ( after < 0 )
            tail = before;
        else
            verts[ after ].prev = before;
    }

    /*---------------------------------------------------------------------*/

    // half edges and faces

    Int32 Head( Int32 e ) const { return edges[ e ].vertex; }
    Int32 Tail( Int32 e ) const { return edges[ edges[ e ].prev ].vertex; }
    Int32 OppositeFace( Int32 e ) const { return edges[ edges[ e ].opposite ].face; }

    Float64 Distance( Int32 f, const Float64* p ) const
    {
        const SFace& face = faces[ f ];
        return face.normal[ 0 ] * p[ 0 ] + face.normal[ 1 ] * p[ 1 ] + face.normal[ 2 ] * p[ 2 ] - face.offset;
    }

    /** The edge 'index' steps from he0 of the face (negative: backwards) */
    Int32 GetEdge( Int32 f, Int32 index ) const
    {
        Int32 e = faces[ f ].he0;
        while ( index > 0 )
        {
            e = edges[ e ].next;
            --index;
        }
        while ( index < 0 )
        {
            e = edges[ e ].prev;
            ++index;
        }
        return e;
    }

    void ComputeNormalAndCentroid( Int32 f )
    {
        SFace& face = faces[ f ];
        const Int32 e0 = face.he0;
        Int32 e1 = edges[ e0 ].next;
        Int32 e2 = edges[ e1 ].next;
        const Float64* p0 = verts[ edges[ e0 ].vertex ].p;
        const Float64* p1 = verts[ edges[ e1 ].vertex ].p;
        Float64 d2[ 3 ] = { p1[ 0 ] - p0[ 0 ], p1[ 1 ] - p0[ 1 ], p1[ 2 ] - p0[ 2 ] };
        Float64 n[ 3 ] = { 0.0, 0.0, 0.0 };
        UInt32 count = 2;
        while ( e2 != e0 )
        {
            const Float64 d1[ 3 ] = { d2[ 0 ], d2[ 1 ], d2[ 2 ] };
            const Float64* p = verts[ edges[ e2 ].vertex ].p;
            d2[ 0 ] = p[ 0 ] - p0[ 0 ]; d2[ 1 ] = p[ 1 ] - p0[ 1 ]; d2[ 2 ] = p[ 2 ] - p0[ 2 ];
            n[ 0 ] += d1[ 1 ] * d2[ 2 ] - d1[ 2 ] * d2[ 1 ];
            n[ 1 ] += d1[ 2 ] * d2[ 0 ] - d1[ 0 ] * d2[ 2 ];
            n[ 2 ] += d1[ 0 ] * d2[ 1 ] - d1[ 1 ] * d2[ 0 ];
            e1 = e2;
            e2 = edges[ e2 ].next;
            ++count;
        }
        face.numVerts = count;
        face.area = sqrt( n[ 0 ] * n[ 0 ] + n[ 1 ] * n[ 1 ] + n[ 2 ] * n[ 2 ] );

        // the longest edge, for the robust normal of needle faces and for the scale
        Int32 longest = e0;
        Float64 longestSquared = 0.0;
        Int32 e = e0;
        do
        {
            const Float64* a = verts[ Tail( e ) ].p;
            const Float64* b = verts[ edges[ e ].vertex ].p;
            const Float64 dx = b[ 0 ] - a[ 0 ], dy = b[ 1 ] - a[ 1 ], dz = b[ 2 ] - a[ 2 ];
            const Float64 lengthSquared = dx * dx + dy * dy + dz * dz;
            if ( lengthSquared > longestSquared )
            {
                longestSquared = lengthSquared;
                longest = e;
            }
            e = edges[ e ].next;
        }
        while ( e != e0 );

        if ( face.area > 0.0 )
        {
            face.normal[ 0 ] = n[ 0 ] / face.area;
            face.normal[ 1 ] = n[ 1 ] / face.area;
            face.normal[ 2 ] = n[ 2 ] / face.area;
            // a needle face: the component of the normal along the longest edge is noise
            if ( face.area < 100.0 * tolerance * sqrt( longestSquared ) && longestSquared > 0.0 )
            {
                const Float64* a = verts[ Tail( longest ) ].p;
                const Float64* b = verts[ edges[ longest ].vertex ].p;
                const Float64 length = sqrt( longestSquared );
                const Float64 u[ 3 ] = { ( b[ 0 ] - a[ 0 ] ) / length, ( b[ 1 ] - a[ 1 ] ) / length, ( b[ 2 ] - a[ 2 ] ) / length };
                const Float64 dot = face.normal[ 0 ] * u[ 0 ] + face.normal[ 1 ] * u[ 1 ] + face.normal[ 2 ] * u[ 2 ];
                Float64 r[ 3 ] = { face.normal[ 0 ] - dot * u[ 0 ], face.normal[ 1 ] - dot * u[ 1 ], face.normal[ 2 ] - dot * u[ 2 ] };
                const Float64 rl = sqrt( r[ 0 ] * r[ 0 ] + r[ 1 ] * r[ 1 ] + r[ 2 ] * r[ 2 ] );
                if ( rl > 0.0 )
                {
                    face.normal[ 0 ] = r[ 0 ] / rl; face.normal[ 1 ] = r[ 1 ] / rl; face.normal[ 2 ] = r[ 2 ] / rl;
                }
            }
        }
        else
        {
            // no area: the face is flat in every direction that matters and merges with its neighbours
            face.normal[ 0 ] = face.normal[ 1 ] = face.normal[ 2 ] = 0.0;
        }

        Float64 c[ 3 ] = { 0.0, 0.0, 0.0 };
        e = e0;
        do
        {
            const Float64* p = verts[ edges[ e ].vertex ].p;
            c[ 0 ] += p[ 0 ]; c[ 1 ] += p[ 1 ]; c[ 2 ] += p[ 2 ];
            e = edges[ e ].next;
        }
        while ( e != e0 );
        const Float64 inverse = 1.0 / (Float64) count;
        face.centroid[ 0 ] = c[ 0 ] * inverse; face.centroid[ 1 ] = c[ 1 ] * inverse; face.centroid[ 2 ] = c[ 2 ] * inverse;
        face.offset = face.normal[ 0 ] * face.centroid[ 0 ] + face.normal[ 1 ] * face.centroid[ 1 ] + face.normal[ 2 ] * face.centroid[ 2 ];
    }

    /** A triangle of the vertices v0, v1, v2; the half edge k has the head vk */
    Int32 CreateTriangle( Int32 v0, Int32 v1, Int32 v2 )
    {
        const Int32 f = (Int32) faces.size();
        const Int32 e = (Int32) edges.size();
        SFace face;
        face.he0 = e;
        face.area = 0.0;
        face.offset = 0.0;
        face.numVerts = 3;
        face.mark = MARK_VISIBLE;
        face.outside = -1;
        face.normal[ 0 ] = face.normal[ 1 ] = face.normal[ 2 ] = 0.0;
        face.centroid[ 0 ] = face.centroid[ 1 ] = face.centroid[ 2 ] = 0.0;
        faces.push_back( face );
        const Int32 heads[ 3 ] = { v0, v1, v2 };
        for ( Int32 k=0; k<3; ++k )
        {
            SHalfEdge edge;
            edge.vertex = heads[ k ];
            edge.face = f;
            edge.next = e + ( k + 1 ) % 3;
            edge.prev = e + ( k + 2 ) % 3;
            edge.opposite = -1;
            edges.push_back( edge );
        }
        ComputeNormalAndCentroid( f );
        return f;
    }

    void SetOpposite( Int32 a, Int32 b )
    {
        edges[ a ].opposite = b;
        edges[ b ].opposite = a;
    }

    /*---------------------------------------------------------------------*/

    // outside sets

    void AddPointToFace( Int32 v, Int32 f )
    {
        verts[ v ].face = f;
        if ( faces[ f ].outside < 0 )
            ListAdd( claimedHead, claimedTail, v );
        else
            ListInsertBefore( claimedHead, v, faces[ f ].outside );
        faces[ f ].outside = v;
    }

    void RemovePointFromFace( Int32 v, Int32 f )
    {
        if ( v == faces[ f ].outside )
        {
            const Int32 next = verts[ v ].next;
            faces[ f ].outside = ( next >= 0 && verts[ next ].face == f ) ? next : -1;
        }
        ListDelete( claimedHead, claimedTail, v, v );
    }

    /** Takes the whole outside set of a face off the claimed list and returns its first vertex (-1 when empty), the chain ends at -1 */
    Int32 RemoveAllPointsFromFace( Int32 f )
    {
        const Int32 first = faces[ f ].outside;
        if ( first < 0 )
            return -1;
        Int32 last = first;
        while ( verts[ last ].next >= 0 && verts[ verts[ last ].next ].face == f )
            last = verts[ last ].next;
        ListDelete( claimedHead, claimedTail, first, last );
        verts[ last ].next = -1;
        verts[ first ].prev = -1;
        faces[ f ].outside = -1;
        return first;
    }

    /** Gives the outside points of a deleted face to 'absorbing' where they are above it, otherwise to the unclaimed list */
    void DeleteFacePoints( Int32 f, Int32 absorbing )
    {
        Int32 v = RemoveAllPointsFromFace( f );
        while ( v >= 0 )
        {
            const Int32 next = verts[ v ].next;
            if ( absorbing >= 0 && Distance( absorbing, verts[ v ].p ) > tolerance )
                AddPointToFace( v, absorbing );
            else
                ListAdd( unclaimedHead, unclaimedTail, v );
            v = next;
        }
    }

    /*---------------------------------------------------------------------*/

    // merging

    Float64 OppositeFaceDistance( Int32 e ) const
    {
        return Distance( edges[ e ].face, faces[ OppositeFace( e ) ].centroid );
    }

    /** Joins 'hedgePrev' and 'hedge' after the faces around them changed; returns the face that became redundant (-1 for none) */
    Int32 ConnectHalfEdges( Int32 hedgePrev, Int32 hedge )
    {
        Int32 discardedFace = -1;
        if ( OppositeFace( hedgePrev ) == OppositeFace( hedge ) )
        {
            // a redundant edge
            const Int32 oppFace = OppositeFace( hedge );
            Int32 hedgeOpp;
            const Int32 face = edges[ hedge ].face;
            if ( hedgePrev == faces[ face ].he0 )
                faces[ face ].he0 = hedge;
            if ( 3 == faces[ oppFace ].numVerts )
            {
                hedgeOpp = edges[ edges[ edges[ hedge ].opposite ].prev ].opposite;
                faces[ oppFace ].mark = MARK_DELETED;
                discardedFace = oppFace;
            }
            else
            {
                hedgeOpp = edges[ edges[ hedge ].opposite ].next;
                if ( faces[ oppFace ].he0 == edges[ hedgeOpp ].prev )
                    faces[ oppFace ].he0 = hedgeOpp;
                edges[ hedgeOpp ].prev = edges[ edges[ hedgeOpp ].prev ].prev;
                edges[ edges[ hedgeOpp ].prev ].next = hedgeOpp;
            }
            edges[ hedge ].prev = edges[ hedgePrev ].prev;
            edges[ edges[ hedge ].prev ].next = hedge;
            edges[ hedge ].opposite = hedgeOpp;
            edges[ hedgeOpp ].opposite = hedge;
            ComputeNormalAndCentroid( oppFace );
        }
        else
        {
            edges[ hedgePrev ].next = hedge;
            edges[ hedge ].prev = hedgePrev;
        }
        return discardedFace;
    }

    /** Merges the face on the other side of 'hedgeAdj' into the face of 'hedgeAdj'; the faces that disappear are appended to 'discarded' */
    void MergeAdjacentFace( Int32 hedgeAdj, GUCEF::vector< Int32 >& removed )
    {
        const Int32 face = edges[ hedgeAdj ].face;
        const Int32 oppFace = OppositeFace( hedgeAdj );
        removed.push_back( oppFace );
        faces[ oppFace ].mark = MARK_DELETED;
        const Int32 hedgeOpp = edges[ hedgeAdj ].opposite;
        Int32 hedgeAdjPrev = edges[ hedgeAdj ].prev;
        Int32 hedgeAdjNext = edges[ hedgeAdj ].next;
        Int32 hedgeOppPrev = edges[ hedgeOpp ].prev;
        Int32 hedgeOppNext = edges[ hedgeOpp ].next;
        while ( OppositeFace( hedgeAdjPrev ) == oppFace )
        {
            hedgeAdjPrev = edges[ hedgeAdjPrev ].prev;
            hedgeOppNext = edges[ hedgeOppNext ].next;
        }
        while ( OppositeFace( hedgeAdjNext ) == oppFace )
        {
            hedgeOppPrev = edges[ hedgeOppPrev ].prev;
            hedgeAdjNext = edges[ hedgeAdjNext ].next;
        }
        const Int32 stop = edges[ hedgeOppPrev ].next;
        for ( Int32 h=hedgeOppNext; h!=stop; h=edges[ h ].next )
            edges[ h ].face = face;
        if ( hedgeAdj == faces[ face ].he0 )
            faces[ face ].he0 = hedgeAdjNext;
        Int32 discardedFace = ConnectHalfEdges( hedgeOppPrev, hedgeAdjNext );
        if ( discardedFace >= 0 )
            removed.push_back( discardedFace );
        discardedFace = ConnectHalfEdges( hedgeAdjPrev, hedgeOppNext );
        if ( discardedFace >= 0 )
            removed.push_back( discardedFace );
        ComputeNormalAndCentroid( face );
    }

    /** One merge of 'face' with a neighbour that is not convex with it; false when none was merged. Type 0: not convex with respect to the larger face, 1: with respect to either */
    bool DoAdjacentMerge( Int32 face, Int32 mergeType )
    {
        Int32 hedge = faces[ face ].he0;
        bool convex = true;
        do
        {
            const Int32 oppFace = OppositeFace( hedge );
            bool merge = false;
            if ( 1 == mergeType )
            {
                if ( OppositeFaceDistance( hedge ) > -tolerance || OppositeFaceDistance( edges[ hedge ].opposite ) > -tolerance )
                    merge = true;
            }
            else
            {
                if ( faces[ face ].area > faces[ oppFace ].area )
                {
                    if ( OppositeFaceDistance( hedge ) > -tolerance )
                        merge = true;
                    else if ( OppositeFaceDistance( edges[ hedge ].opposite ) > -tolerance )
                        convex = false;
                }
                else
                {
                    if ( OppositeFaceDistance( edges[ hedge ].opposite ) > -tolerance )
                        merge = true;
                    else if ( OppositeFaceDistance( hedge ) > -tolerance )
                        convex = false;
                }
            }
            if ( merge )
            {
                discarded.clear();
                MergeAdjacentFace( hedge, discarded );
                for ( size_t i=0; i<discarded.size(); ++i )
                    DeleteFacePoints( discarded[ i ], face );
                return true;
            }
            hedge = edges[ hedge ].next;
        }
        while ( hedge != faces[ face ].he0 );
        if ( !convex )
            faces[ face ].mark = MARK_NON_CONVEX;
        return false;
    }

    /*---------------------------------------------------------------------*/

    // adding a point

    void CalculateHorizon( const Float64* eye, Int32 edge0, Int32 face )
    {
        DeleteFacePoints( face, -1 );
        faces[ face ].mark = MARK_DELETED;
        Int32 edge;
        if ( edge0 < 0 )
        {
            edge0 = faces[ face ].he0;
            edge = edge0;
        }
        else
            edge = edges[ edge0 ].next;
        do
        {
            const Int32 oppFace = OppositeFace( edge );
            if ( MARK_VISIBLE == faces[ oppFace ].mark )
            {
                if ( Distance( oppFace, eye ) > tolerance )
                    CalculateHorizon( eye, edges[ edge ].opposite, oppFace );
                else
                    horizon.push_back( edge );
            }
            edge = edges[ edge ].next;
        }
        while ( edge != edge0 );
    }

    Int32 AddAdjoiningFace( Int32 eye, Int32 he )
    {
        const Int32 face = CreateTriangle( eye, Tail( he ), Head( he ) );
        SetOpposite( GetEdge( face, -1 ), edges[ he ].opposite );
        return GetEdge( face, 0 );
    }

    void AddNewFaces( Int32 eye )
    {
        newFaces.clear();
        Int32 sidePrev = -1;
        Int32 sideBegin = -1;
        for ( size_t i=0; i<horizon.size(); ++i )
        {
            const Int32 side = AddAdjoiningFace( eye, horizon[ i ] );
            if ( sidePrev >= 0 )
                SetOpposite( edges[ side ].next, sidePrev );
            else
                sideBegin = side;
            newFaces.push_back( edges[ side ].face );
            sidePrev = side;
        }
        SetOpposite( edges[ sideBegin ].next, sidePrev );
    }

    void ResolveUnclaimedPoints( void )
    {
        Int32 next = unclaimedHead;
        for ( Int32 v=next; v>=0; v=next )
        {
            next = verts[ v ].next;
            Float64 maxDistance = tolerance;
            Int32 maxFace = -1;
            for ( size_t i=0; i<newFaces.size(); ++i )
            {
                const Int32 f = newFaces[ i ];
                if ( MARK_VISIBLE == faces[ f ].mark )
                {
                    const Float64 d = Distance( f, verts[ v ].p );
                    if ( d > maxDistance )
                    {
                        maxDistance = d;
                        maxFace = f;
                    }
                    if ( maxDistance > 1000.0 * tolerance )
                        break;
                }
            }
            if ( maxFace >= 0 )
                AddPointToFace( v, maxFace );
        }
    }

    void AddPointToHull( Int32 eye )
    {
        horizon.clear();
        unclaimedHead = unclaimedTail = -1;
        RemovePointFromFace( eye, verts[ eye ].face );
        CalculateHorizon( verts[ eye ].p, -1, verts[ eye ].face );
        AddNewFaces( eye );
        // first pass: merge faces that are not convex with respect to the larger face
        for ( size_t i=0; i<newFaces.size(); ++i )
            if ( MARK_VISIBLE == faces[ newFaces[ i ] ].mark )
                while ( DoAdjacentMerge( newFaces[ i ], 0 ) )
                    ;
        // second pass: merge faces that are not convex with respect to either
        for ( size_t i=0; i<newFaces.size(); ++i )
            if ( MARK_NON_CONVEX == faces[ newFaces[ i ] ].mark )
            {
                faces[ newFaces[ i ] ].mark = MARK_VISIBLE;
                while ( DoAdjacentMerge( newFaces[ i ], 1 ) )
                    ;
            }
        ResolveUnclaimedPoints();
    }

    /** The vertex of the face with the largest distance in its outside set */
    Int32 NextPointToAdd( void ) const
    {
        if ( claimedHead < 0 )
            return -1;
        const Int32 eyeFace = verts[ claimedHead ].face;
        Int32 eye = -1;
        Float64 maxDistance = 0.0;
        for ( Int32 v=faces[ eyeFace ].outside; v>=0 && verts[ v ].face == eyeFace; v=verts[ v ].next )
        {
            const Float64 d = Distance( eyeFace, verts[ v ].p );
            if ( d > maxDistance )
            {
                maxDistance = d;
                eye = v;
            }
        }
        return eye;
    }

    /*---------------------------------------------------------------------*/

    TQuickHullStatus Build( const Float64* xyz, UInt32 count, Float64 toleranceFactor );
    bool CreateInitialSimplex( TQuickHullStatus& status );
    void ExtractResult( void );
};

/*-------------------------------------------------------------------------*/

namespace {

GUCEF_GEOMETRY_FORCE_INLINE Float64 AbsF( Float64 x ) { return x < 0.0 ? -x : x; }

}

/*-------------------------------------------------------------------------*/

bool
CQuickHull3DCore::SImpl::CreateInitialSimplex( TQuickHullStatus& status )
{
    const UInt32 n = (UInt32) verts.size();
    // the extreme points per axis (the first one on ties)
    Int32 minV[ 3 ] = { 0, 0, 0 };
    Int32 maxV[ 3 ] = { 0, 0, 0 };
    for ( UInt32 i=1; i<n; ++i )
        for ( UInt32 a=0; a<3; ++a )
        {
            if ( verts[ i ].p[ a ] > verts[ maxV[ a ] ].p[ a ] )
                maxV[ a ] = (Int32) i;
            if ( verts[ i ].p[ a ] < verts[ minV[ a ] ].p[ a ] )
                minV[ a ] = (Int32) i;
        }

    // the two points with the largest separation along one axis
    UInt32 axis = 0;
    Float64 spread = verts[ maxV[ 0 ] ].p[ 0 ] - verts[ minV[ 0 ] ].p[ 0 ];
    for ( UInt32 a=1; a<3; ++a )
    {
        const Float64 s = verts[ maxV[ a ] ].p[ a ] - verts[ minV[ a ] ].p[ a ];
        if ( s > spread )
        {
            spread = s;
            axis = a;
        }
    }
    if ( spread <= tolerance )
    {
        status = QUICKHULL_POINT;
        vertexIndices.push_back( 0 );
        return false;
    }
    Int32 v[ 4 ];
    v[ 0 ] = maxV[ axis ];
    v[ 1 ] = minV[ axis ];

    // the point furthest from the line through them
    Float64 u01[ 3 ];
    Float64 length = 0.0;
    for ( UInt32 a=0; a<3; ++a )
    {
        u01[ a ] = verts[ v[ 1 ] ].p[ a ] - verts[ v[ 0 ] ].p[ a ];
        length += u01[ a ] * u01[ a ];
    }
    length = sqrt( length );
    for ( UInt32 a=0; a<3; ++a )
        u01[ a ] /= length;
    Float64 normal[ 3 ] = { 0.0, 0.0, 0.0 };
    Float64 maxSquared = 0.0;
    v[ 2 ] = -1;
    for ( UInt32 i=0; i<n; ++i )
    {
        if ( (Int32) i == v[ 0 ] || (Int32) i == v[ 1 ] )
            continue;
        const Float64 d[ 3 ] = { verts[ i ].p[ 0 ] - verts[ v[ 0 ] ].p[ 0 ], verts[ i ].p[ 1 ] - verts[ v[ 0 ] ].p[ 1 ], verts[ i ].p[ 2 ] - verts[ v[ 0 ] ].p[ 2 ] };
        const Float64 c[ 3 ] = { u01[ 1 ] * d[ 2 ] - u01[ 2 ] * d[ 1 ], u01[ 2 ] * d[ 0 ] - u01[ 0 ] * d[ 2 ], u01[ 0 ] * d[ 1 ] - u01[ 1 ] * d[ 0 ] };
        const Float64 squared = c[ 0 ] * c[ 0 ] + c[ 1 ] * c[ 1 ] + c[ 2 ] * c[ 2 ];
        if ( squared > maxSquared )
        {
            maxSquared = squared;
            v[ 2 ] = (Int32) i;
            normal[ 0 ] = c[ 0 ]; normal[ 1 ] = c[ 1 ]; normal[ 2 ] = c[ 2 ];
        }
    }
    if ( v[ 2 ] < 0 || sqrt( maxSquared ) <= 100.0 * tolerance )
    {
        status = QUICKHULL_LINE;
        vertexIndices.push_back( (UInt32) v[ 1 ] );
        vertexIndices.push_back( (UInt32) v[ 0 ] );
        return false;
    }
    const Float64 normalLength = sqrt( normal[ 0 ] * normal[ 0 ] + normal[ 1 ] * normal[ 1 ] + normal[ 2 ] * normal[ 2 ] );
    for ( UInt32 a=0; a<3; ++a )
        normal[ a ] /= normalLength;

    // the point furthest from the plane
    const Float64 d0 = normal[ 0 ] * verts[ v[ 2 ] ].p[ 0 ] + normal[ 1 ] * verts[ v[ 2 ] ].p[ 1 ] + normal[ 2 ] * verts[ v[ 2 ] ].p[ 2 ];
    Float64 maxDistance = 0.0;
    v[ 3 ] = -1;
    for ( UInt32 i=0; i<n; ++i )
    {
        if ( (Int32) i == v[ 0 ] || (Int32) i == v[ 1 ] || (Int32) i == v[ 2 ] )
            continue;
        const Float64 distance = AbsF( normal[ 0 ] * verts[ i ].p[ 0 ] + normal[ 1 ] * verts[ i ].p[ 1 ] + normal[ 2 ] * verts[ i ].p[ 2 ] - d0 );
        if ( distance > maxDistance )
        {
            maxDistance = distance;
            v[ 3 ] = (Int32) i;
        }
    }
    if ( v[ 3 ] < 0 || maxDistance <= 100.0 * tolerance )
    {
        // all points are in one plane: the 2D hull of the projection along the largest normal component
        status = QUICKHULL_PLANE;
        UInt32 drop = 0;
        for ( UInt32 a=1; a<3; ++a )
            if ( AbsF( normal[ a ] ) > AbsF( normal[ drop ] ) )
                drop = a;
        const UInt32 ua = ( drop + 1 ) % 3;
        const UInt32 ub = ( drop + 2 ) % 3;
        GUCEF::vector< MATH::CTPoint2D< Float64 > > projected;
        projected.reserve( n );
        for ( UInt32 i=0; i<n; ++i )
            projected.push_back( MATH::CTPoint2D< Float64 >( verts[ i ].p[ ua ], verts[ i ].p[ ub ] ) );
        ConvexHull2DIndices( &projected[ 0 ], n, vertexIndices, false );
        return false;
    }

    // the tetrahedron: every face is oriented away from the fourth point
    const Int32 triples[ 4 ][ 3 ] = { { v[ 0 ], v[ 1 ], v[ 2 ] }, { v[ 0 ], v[ 1 ], v[ 3 ] }, { v[ 0 ], v[ 2 ], v[ 3 ] }, { v[ 1 ], v[ 2 ], v[ 3 ] } };
    const Int32 opposite[ 4 ] = { v[ 3 ], v[ 2 ], v[ 1 ], v[ 0 ] };
    Int32 tris[ 4 ];
    for ( UInt32 k=0; k<4; ++k )
    {
        const Float64* a = verts[ triples[ k ][ 0 ] ].p;
        const Float64* b = verts[ triples[ k ][ 1 ] ].p;
        const Float64* c = verts[ triples[ k ][ 2 ] ].p;
        const Float64* o = verts[ opposite[ k ] ].p;
        const Float64 ab[ 3 ] = { b[ 0 ] - a[ 0 ], b[ 1 ] - a[ 1 ], b[ 2 ] - a[ 2 ] };
        const Float64 ac[ 3 ] = { c[ 0 ] - a[ 0 ], c[ 1 ] - a[ 1 ], c[ 2 ] - a[ 2 ] };
        const Float64 cross[ 3 ] = { ab[ 1 ] * ac[ 2 ] - ab[ 2 ] * ac[ 1 ], ab[ 2 ] * ac[ 0 ] - ab[ 0 ] * ac[ 2 ], ab[ 0 ] * ac[ 1 ] - ab[ 1 ] * ac[ 0 ] };
        const Float64 side = cross[ 0 ] * ( o[ 0 ] - a[ 0 ] ) + cross[ 1 ] * ( o[ 1 ] - a[ 1 ] ) + cross[ 2 ] * ( o[ 2 ] - a[ 2 ] );
        if ( side > 0.0 )
            tris[ k ] = CreateTriangle( triples[ k ][ 0 ], triples[ k ][ 2 ], triples[ k ][ 1 ] );
        else
            tris[ k ] = CreateTriangle( triples[ k ][ 0 ], triples[ k ][ 1 ], triples[ k ][ 2 ] );
    }
    // pair the half edges: the edge tail -> head of one face with head -> tail of another
    for ( UInt32 i=0; i<4; ++i )
        for ( Int32 ei=0; ei<3; ++ei )
        {
            const Int32 a = faces[ tris[ i ] ].he0 + ei;
            for ( UInt32 j=i+1; j<4; ++j )
                for ( Int32 ej=0; ej<3; ++ej )
                {
                    const Int32 b = faces[ tris[ j ] ].he0 + ej;
                    if ( Tail( a ) == Head( b ) && Head( a ) == Tail( b ) )
                        SetOpposite( a, b );
                }
        }

    // every other point goes to the face it is furthest above
    for ( UInt32 i=0; i<n; ++i )
    {
        if ( (Int32) i == v[ 0 ] || (Int32) i == v[ 1 ] || (Int32) i == v[ 2 ] || (Int32) i == v[ 3 ] )
            continue;
        Float64 best = tolerance;
        Int32 bestFace = -1;
        for ( UInt32 k=0; k<4; ++k )
        {
            const Float64 d = Distance( tris[ k ], verts[ i ].p );
            if ( d > best )
            {
                best = d;
                bestFace = tris[ k ];
            }
        }
        if ( bestFace >= 0 )
            AddPointToFace( (Int32) i, bestFace );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CQuickHull3DCore::SImpl::ExtractResult( void )
{
    const UInt32 n = (UInt32) verts.size();
    // the live faces and how many of them each vertex is in
    GUCEF::vector< UInt32 > incidence( n, 0 );
    GUCEF::vector< Int32 > live;
    for ( size_t f=0; f<faces.size(); ++f )
        if ( MARK_DELETED != faces[ f ].mark )
        {
            live.push_back( (Int32) f );
            Int32 e = faces[ f ].he0;
            do
            {
                ++incidence[ edges[ e ].vertex ];
                e = edges[ e ].next;
            }
            while ( e != faces[ f ].he0 );
        }
    // a vertex in only two faces lies in the middle of an edge that they share: not a vertex of the polyhedron
    GUCEF::vector< Int32 > outIndex( n, -1 );
    for ( UInt32 i=0; i<n; ++i )
        if ( incidence[ i ] >= 3 )
        {
            outIndex[ i ] = (Int32) vertexIndices.size();
            vertexIndices.push_back( i );
        }
    for ( size_t k=0; k<live.size(); ++k )
    {
        const Int32 f = live[ k ];
        UInt32 size = 0;
        Int32 e = faces[ f ].he0;
        do
        {
            const Int32 v = edges[ e ].vertex;
            if ( outIndex[ v ] >= 0 )
            {
                faceIndices.push_back( (UInt32) outIndex[ v ] );
                ++size;
            }
            e = edges[ e ].next;
        }
        while ( e != faces[ f ].he0 );
        faceSizes.push_back( size );
    }
}

/*-------------------------------------------------------------------------*/

TQuickHullStatus
CQuickHull3DCore::SImpl::Build( const Float64* xyz, UInt32 count, Float64 toleranceFactor )
{
    Clear();
    tolerance = 0.0;
    if ( 0 == count )
        return QUICKHULL_EMPTY;
    for ( UInt32 i=0; i<count * 3; ++i )
        if ( !( xyz[ i ] == xyz[ i ] && xyz[ i ] - xyz[ i ] == 0.0 ) )
            return QUICKHULL_NOT_FINITE;
    verts.resize( count );
    Float64 extent[ 3 ] = { 0.0, 0.0, 0.0 };
    for ( UInt32 i=0; i<count; ++i )
    {
        for ( UInt32 a=0; a<3; ++a )
        {
            verts[ i ].p[ a ] = xyz[ (size_t) i * 3 + a ];
            if ( AbsF( verts[ i ].p[ a ] ) > extent[ a ] )
                extent[ a ] = AbsF( verts[ i ].p[ a ] );
        }
        verts[ i ].prev = verts[ i ].next = verts[ i ].face = -1;
    }
    tolerance = toleranceFactor * DBL_EPSILON * ( extent[ 0 ] + extent[ 1 ] + extent[ 2 ] );
    if ( 1 == count )
    {
        vertexIndices.push_back( 0 );
        return QUICKHULL_POINT;
    }

    TQuickHullStatus status = QUICKHULL_OK;
    if ( !CreateInitialSimplex( status ) )
        return status;

    // the main loop: the point furthest above its face joins the hull; every point joins at most once
    UInt32 iterations = 0;
    for ( Int32 eye=NextPointToAdd(); eye>=0; eye=NextPointToAdd() )
    {
        if ( ++iterations > count + 8 )
            return QUICKHULL_FAILED;
        AddPointToHull( eye );
    }
    ExtractResult();
    if ( faceSizes.size() < 4 )
        return QUICKHULL_FAILED;
    return QUICKHULL_OK;
}

/*-------------------------------------------------------------------------*/

CQuickHull3DCore::CQuickHull3DCore( void )
    : m_impl( GUCEF_NEW SImpl() )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CQuickHull3DCore::~CQuickHull3DCore()
{GUCEF_TRACE;

    GUCEF_DELETE m_impl;
    m_impl = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

TQuickHullStatus
CQuickHull3DCore::Build( const Float64* xyz, UInt32 count, Float64 toleranceFactor )
{GUCEF_TRACE;

    return m_impl->Build( xyz, count, toleranceFactor );
}

/*-------------------------------------------------------------------------*/

Float64
CQuickHull3DCore::GetTolerance( void ) const
{
    return m_impl->tolerance;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< UInt32 >&
CQuickHull3DCore::GetVertexIndices( void ) const
{
    return m_impl->vertexIndices;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< UInt32 >&
CQuickHull3DCore::GetFaceIndices( void ) const
{
    return m_impl->faceIndices;
}

/*-------------------------------------------------------------------------*/

const GUCEF::vector< UInt32 >&
CQuickHull3DCore::GetFaceSizes( void ) const
{
    return m_impl->faceSizes;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      INSTANTIATIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTQuickHull3D< Float32 > )
GUCEF_GEOMETRY_INSTANTIATE_TEMPLATE_CLASS( CTQuickHull3D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
