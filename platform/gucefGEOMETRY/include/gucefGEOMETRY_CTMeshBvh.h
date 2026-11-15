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

#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#define GUCEF_GEOMETRY_CTMESHBVH_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */

#ifndef GUCEF_CORE_CTASKCONSUMER_H
#include "gucefCORE_CTaskConsumer.h"
#define GUCEF_CORE_CTASKCONSUMER_H
#endif /* GUCEF_CORE_CTASKCONSUMER_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_GEOMETRY_CONVEXQUERIES_H
#include "gucefGEOMETRY_ConvexQueries.h"
#define GUCEF_GEOMETRY_CONVEXQUERIES_H
#endif /* GUCEF_GEOMETRY_CONVEXQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_RAYQUERIES_H
#include "gucefGEOMETRY_RayQueries.h"
#define GUCEF_GEOMETRY_RAYQUERIES_H
#endif /* GUCEF_GEOMETRY_RAYQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_SGEORAYHIT_H
#include "gucefGEOMETRY_SGeoRayHit.h"
#define GUCEF_GEOMETRY_SGEORAYHIT_H
#endif /* GUCEF_GEOMETRY_SGEORAYHIT_H ? */

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

template < typename T > class CTTriangleMesh;
template < typename T > class CTMeshBvh;

/*-------------------------------------------------------------------------*/

/** The feature of a triangle that holds the closest point (SMeshClosestPoint::feature) */
enum EMeshFeature
{
    MESHFEATURE_VERTEX_A = 0 ,
    MESHFEATURE_VERTEX_B     ,
    MESHFEATURE_VERTEX_C     ,
    MESHFEATURE_EDGE_AB      ,   /**< edge 0: from the first to the second vertex */
    MESHFEATURE_EDGE_BC      ,   /**< edge 1 */
    MESHFEATURE_EDGE_CA      ,   /**< edge 2 */
    MESHFEATURE_FACE
};
typedef enum EMeshFeature TMeshFeature;

/*-------------------------------------------------------------------------*/

/** The closest point of a mesh to a query point */
template < typename T >
struct SMeshClosestPoint
{
    MATH::CTPoint3D< T > point;     /**< on the mesh */
    T squaredDistance;              /**< from the query point */
    UInt32 triangle;                /**< the triangle that holds the point (index in the mesh) */
    TMeshFeature feature;           /**< the feature of that triangle that holds it */

    SMeshClosestPoint( void ) : point(), squaredDistance( MATH::CTNumericTraits< T >::Zero() ), triangle( 0 ), feature( MESHFEATURE_FACE ) {}
};

/*-------------------------------------------------------------------------*/

/** The compiled builders of the hierarchy (src/gucefGEOMETRY_CTMeshBvh.cpp); used by CTMeshBvh::Build and BuildAsync */
GUCEF_GEOMETRY_PUBLIC_CPP bool BuildMeshBvh( const CTTriangleMesh< Float32 >& mesh, CTMeshBvh< Float32 >& bvh );
GUCEF_GEOMETRY_PUBLIC_CPP bool BuildMeshBvh( const CTTriangleMesh< Float64 >& mesh, CTMeshBvh< Float64 >& bvh );
GUCEF_GEOMETRY_PUBLIC_CPP CORE::CFutureResult BuildMeshBvhAsync( const CTTriangleMesh< Float32 >& mesh, CTMeshBvh< Float32 >& bvh, CORE::ThreadPoolPtr threadPool );
GUCEF_GEOMETRY_PUBLIC_CPP CORE::CFutureResult BuildMeshBvhAsync( const CTTriangleMesh< Float64 >& mesh, CTMeshBvh< Float64 >& bvh, CORE::ThreadPoolPtr threadPool );

/*-------------------------------------------------------------------------*/

/**
 *  Bounding volume hierarchy over the triangles of a CTTriangleMesh (section 7.8.2 of the design).
 *
 *  Build
 *  - Binned surface area heuristic: 16 bins along each axis of the bounds of the triangle centroids, leaves of at most 4
 *    triangles (ranges whose centroids coincide are split in the middle). Nodes are flattened depth first: the first
 *    child of an interior node follows it, the second is at 'offset'; a leaf holds 'count' triangles from 'offset' in
 *    the permuted triangle order (GetPrimitive), so the triangles of a leaf are adjacent in memory. A Float32 node is
 *    32 bytes. The build is deterministic: the same mesh gives the same tree on every thread.
 *  - Build runs on the calling thread. BuildAsync runs the same build as a task on a thread pool of the task manager
 *    (the default pool unless one is given) and returns its CORE::CFutureResult; the mesh and the hierarchy must stay
 *    alive and untouched until the future is ready. An invalid mesh fails at once as with Build: the hierarchy is
 *    cleared and the returned future is ready with no future (HasNoFuture). GetBuildThreadId tells which thread
 *    built it.
 *  - The hierarchy refers to the mesh it was built from (GetMesh): the mesh must outlive it, and after the vertices
 *    move (same triangles) Refit recomputes the boxes and the pseudo-normals; a change of the triangles needs a Build.
 *
 *  Queries (world units of the mesh; Float32 and Float64)
 *  - RayCast: the nearest triangle hit by the watertight ray triangle test (RayCastTriangleWatertight), front to back
 *    traversal with robust slab tests (the far distance widened by a few units in the last place, so boxes that a hit
 *    lies on are never culled). featureIndex is the triangle index, u and v its barycentrics, the normal faces the ray,
 *    RAYHIT_BACKFACE is set when the ray runs along the counter-clockwise normal. Among hits at exactly the same
 *    distance the first one found is kept. RayTest: any hit; RayCastAll: every hit within maxDistance.
 *  - GetClosestPoint( point, maxDistance ): branch and bound by the distance of the boxes; false when no triangle is
 *    within maxDistance. GetSignedDistance: the closest point and the sign of the angle-weighted pseudo-normal of the
 *    feature that holds it (Baerentzen and Aanaes): negative inside a closed, consistently oriented mesh.
 *  - QueryBox( box, visitor ): visitor.Visit( UInt32 triangle ) -> bool for every triangle whose box touches 'box'
 *    (false stops). Overlaps / Overlap( convex shape ): the triangles that share a point with a bounded convex value
 *    shape (Intersects of the triangle and the shape), pruned by the shape's bounds.
 *  - Queries do not allocate (an explicit stack of 64 entries on the stack, a heap stack only for deeper trees) and
 *    may run concurrently on several threads; Build, Refit and Clear must not run meanwhile.
 *  - Kinds: Float32 and Float64 are exported. Not realtime-safe (section 2.4 rule 5).
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTMeshBvh
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTRay3D< T > TRay;
    typedef CTTriangle3D< T > TTriangle;
    typedef CTTriangleMesh< T > TMesh;
    typedef SRayCastResult< T > TRayHit;
    typedef SMeshClosestPoint< T > TClosestPoint;

    enum { BinCount = 16, MaxLeafSize = 4 };

    /** A flattened node: a leaf when count > 0 (triangles offset .. offset + count - 1), else an interior node (children index + 1 and offset) */
    struct SNode
    {
        TBox bounds;
        UInt32 offset;
        UInt32 count;
    };

    CTMeshBvh( void );

    /** Builds over 'mesh' on the calling thread; false (and an empty hierarchy) for an invalid mesh */
    bool Build( const TMesh& mesh );

    /** Builds over 'mesh' as a task on the default thread pool of the task manager; an invalid mesh fails at once (see the class comment) */
    CORE::CFutureResult BuildAsync( const TMesh& mesh );

    /** Builds over 'mesh' as a task on 'threadPool' (the default pool when it is null) */
    CORE::CFutureResult BuildAsync( const TMesh& mesh, CORE::ThreadPoolPtr threadPool );

    /** Recomputes boxes and pseudo-normals after the vertices of the mesh moved; false when the triangle count changed */
    bool Refit( void );

    /** Back to the state of a default hierarchy (no mesh) */
    void Clear( void );

    GUCEF_GEOMETRY_FORCE_INLINE bool IsBuilt( void ) const { return GUCEF_NULL != m_mesh; }
    GUCEF_GEOMETRY_FORCE_INLINE const TMesh* GetMesh( void ) const { return m_mesh; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetNodeCount( void ) const { return (UInt32) m_nodes.size(); }
    GUCEF_GEOMETRY_FORCE_INLINE const SNode& GetNode( UInt32 index ) const { return m_nodes[ index ]; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPrimitiveCount( void ) const { return (UInt32) m_primitives.size(); }

    /** The mesh triangle at slot 'index' of the permuted order */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetPrimitive( UInt32 index ) const { return m_primitives[ index ]; }

    /** The number of node levels (0 without triangles, 1 for a single leaf) */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetDepth( void ) const { return m_depth; }

    /** The thread that ran the last Build (0 before) */
    GUCEF_GEOMETRY_FORCE_INLINE GUCEF::TThreadId GetBuildThreadId( void ) const { return m_buildThread; }

    /** The bounds of all triangles (empty without triangles) */
    TBox GetBounds( void ) const;

    /** True when both hierarchies have the same nodes and triangle order */
    bool HasSameTree( const CTMeshBvh& other ) const;

    /** The unit normal of a triangle (zero when degenerate) */
    GUCEF_GEOMETRY_FORCE_INLINE const TVector& GetFaceNormal( UInt32 triangle ) const { return m_faceNormals[ triangle ]; }

    /** The angle-weighted pseudo-normal of a vertex (unit, zero without a non-degenerate triangle) */
    GUCEF_GEOMETRY_FORCE_INLINE const TVector& GetVertexPseudoNormal( UInt32 vertex ) const { return m_vertexNormals[ vertex ]; }

    /** The pseudo-normal of edge 'edge' (0, 1, 2) of a triangle: the sum of the normals of the triangles sharing it */
    GUCEF_GEOMETRY_FORCE_INLINE const TVector& GetEdgePseudoNormal( UInt32 triangle, UInt32 edge ) const { return m_edgeNormals[ 3 * (size_t) triangle + edge ]; }

    /** The nearest hit within maxDistance (see the class comment) */
    bool RayCast( const TRay& ray, const T& maxDistance, TRayHit& hit ) const;

    /** True when any triangle is hit within maxDistance */
    bool RayTest( const TRay& ray, const T& maxDistance ) const;

    /** Every hit within maxDistance (replaces the content of 'hits'), by distance then triangle when sorted; returns the count */
    UInt32 RayCastAll( const TRay& ray, const T& maxDistance, GUCEF::vector< TRayHit >& hits, bool sortByDistance ) const;

    /** The closest point of the mesh within maxDistance of 'point'; false when there is none */
    bool GetClosestPoint( const TPoint& point, const T& maxDistance, TClosestPoint& result ) const;

    /** The signed distance within maxDistance (negative inside, see the class comment); false when no triangle is that close */
    bool GetSignedDistance( const TPoint& point, const T& maxDistance, T& signedDistance, TClosestPoint* closest ) const;

    /** Calls visitor.Visit( UInt32 triangle ) -> bool for the triangles whose box touches 'box'; false stops */
    template < typename TVisitor >
    void QueryBox( const TBox& box, TVisitor& visitor ) const;

    /** True when a triangle shares a point with the bounded convex shape */
    template < typename TShape >
    bool Overlaps( const TShape& shape ) const;

    /** The triangles that share a point with the bounded convex shape (appended to 'triangles'); returns how many were added */
    template < typename TShape >
    UInt32 Overlap( const TShape& shape, GUCEF::vector< UInt32 >& triangles ) const;

    /** The feature of the triangle that holds its closest point to 'point' (the regions of CTTriangle3D::ClosestPoint) */
    static TMeshFeature ClassifyClosestFeature( const TTriangle& triangle, const TPoint& point );

    private:

    friend bool BuildMeshBvh( const CTTriangleMesh< Float32 >& mesh, CTMeshBvh< Float32 >& bvh );
    friend bool BuildMeshBvh( const CTTriangleMesh< Float64 >& mesh, CTMeshBvh< Float64 >& bvh );

    /** A ray prepared for the slab tests */
    struct SRaySetup
    {
        TPoint origin;
        TVector inverse;
        bool parallel[ 3 ];
    };

    struct SEdge
    {
        UInt64 key;
        UInt32 slot;
        bool operator<( const SEdge& other ) const { return key < other.key || ( key == other.key && slot < other.slot ); }
    };

    template < typename TShape >
    struct SAnyOverlapVisitor
    {
        const CTMeshBvh* bvh;
        const TShape* shape;
        bool found;
        bool Visit( UInt32 triangle )
        {
            if ( Intersects( bvh->m_mesh->GetTriangle( triangle ), *shape ) )
            {
                found = true;
                return false;
            }
            return true;
        }
    };

    template < typename TShape >
    struct SAllOverlapVisitor
    {
        const CTMeshBvh* bvh;
        const TShape* shape;
        GUCEF::vector< UInt32 >* triangles;
        UInt32 added;
        bool Visit( UInt32 triangle )
        {
            if ( Intersects( bvh->m_mesh->GetTriangle( triangle ), *shape ) )
            {
                triangles->push_back( triangle );
                ++added;
            }
            return true;
        }
    };

    struct SStackEntry
    {
        UInt32 node;
        T distance;
    };

    /** Up to 64 entries on the stack of the query, a heap array for deeper trees */
    class CTraversalStack
    {
        public:

        explicit CTraversalStack( UInt32 depth ) : m_heap(), m_entries( m_local ), m_count( 0 )
        {
            if ( depth + 2 > LocalCapacity )
            {
                m_heap.resize( depth + 2 );
                m_entries = &m_heap[ 0 ];
            }
        }

        GUCEF_GEOMETRY_FORCE_INLINE void Push( UInt32 node, const T& distance ) { m_entries[ m_count ].node = node; m_entries[ m_count ].distance = distance; ++m_count; }
        GUCEF_GEOMETRY_FORCE_INLINE bool Pop( SStackEntry& entry ) { if ( 0 == m_count ) return false; entry = m_entries[ --m_count ]; return true; }

        private:

        enum { LocalCapacity = 64 };

        CTraversalStack( const CTraversalStack& );
        CTraversalStack& operator=( const CTraversalStack& );

        SStackEntry m_local[ LocalCapacity ];
        GUCEF::vector< SStackEntry > m_heap;
        SStackEntry* m_entries;
        UInt32 m_count;
    };

    static SRaySetup SetupRay( const TRay& ray );
    static bool SlabTest( const TBox& box, const SRaySetup& setup, const T& maxDistance, T& entry );
    static T Widened( const T& distance );
    static T SquaredDistanceBound( const T& maxDistance );

    void FinishBuild( const TMesh& mesh );
    void BuildPseudoNormals( void );
    void SetHit( UInt32 triangle, const TRay& ray, TRayHit& hit ) const;

    GUCEF::vector< SNode > m_nodes;
    GUCEF::vector< UInt32 > m_primitives;
    GUCEF::vector< TVector > m_faceNormals;
    GUCEF::vector< TVector > m_vertexNormals;
    GUCEF::vector< TVector > m_edgeNormals;
    const TMesh* m_mesh;
    UInt32 m_depth;
    GUCEF::TThreadId m_buildThread;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTMeshBvh< T >::CTMeshBvh( void )
    : m_nodes(), m_primitives(), m_faceNormals(), m_vertexNormals(), m_edgeNormals(), m_mesh( GUCEF_NULL ), m_depth( 0 ), m_buildThread( 0 )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTMeshBvh< T >::Build( const TMesh& mesh )
{
    return BuildMeshBvh( mesh, *this );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CORE::CFutureResult
CTMeshBvh< T >::BuildAsync( const TMesh& mesh )
{
    return BuildMeshBvhAsync( mesh, *this, CORE::ThreadPoolPtr() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
CORE::CFutureResult
CTMeshBvh< T >::BuildAsync( const TMesh& mesh, CORE::ThreadPoolPtr threadPool )
{
    return BuildMeshBvhAsync( mesh, *this, threadPool );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTMeshBvh< T >::Clear( void )
{
    m_nodes.clear();
    m_primitives.clear();
    m_faceNormals.clear();
    m_vertexNormals.clear();
    m_edgeNormals.clear();
    m_mesh = GUCEF_NULL;
    m_depth = 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTMeshBvh< T >::FinishBuild( const TMesh& mesh )
{
    m_mesh = &mesh;
    m_buildThread = MT::GetCurrentThreadID();
    BuildPseudoNormals();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTMeshBvh< T >::BuildPseudoNormals( void )
{
    const TMesh& mesh = *m_mesh;
    mesh.ComputeFaceNormals( m_faceNormals );
    mesh.ComputeVertexNormals( m_vertexNormals );

    /* edges: the undirected edges sorted, each group sums the normals of its triangles */
    const UInt32 count = mesh.GetTriangleCount();
    m_edgeNormals.assign( 3 * (size_t) count, TVector() );
    GUCEF::vector< SEdge > edges( 3 * (size_t) count );
    for ( UInt32 t=0; t<count; ++t )
    {
        for ( UInt32 e=0; e<3; ++e )
        {
            const UInt64 from = mesh.indices[ 3 * (size_t) t + e ];
            const UInt64 to = mesh.indices[ 3 * (size_t) t + ( e + 1 ) % 3 ];
            SEdge& edge = edges[ 3 * (size_t) t + e ];
            edge.key = from < to ? ( ( from << 32 ) | to ) : ( ( to << 32 ) | from );
            edge.slot = 3 * t + e;
        }
    }
    std::sort( edges.begin(), edges.end() );
    size_t begin = 0;
    while ( begin < edges.size() )
    {
        size_t end = begin + 1;
        while ( end < edges.size() && edges[ end ].key == edges[ begin ].key )
            ++end;
        TVector sum;
        for ( size_t i=begin; i<end; ++i )
            sum = sum + m_faceNormals[ edges[ i ].slot / 3 ];
        for ( size_t i=begin; i<end; ++i )
            m_edgeNormals[ edges[ i ].slot ] = sum;
        begin = end;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTMeshBvh< T >::Refit( void )
{
    if ( GUCEF_NULL == m_mesh || m_mesh->GetTriangleCount() != (UInt32) m_primitives.size() || !m_mesh->IsValid() )
        return false;
    const TMesh& mesh = *m_mesh;
    /* children always follow their parent in the depth-first order, so a reverse sweep sees children first */
    for ( size_t n=m_nodes.size(); n>0; --n )
    {
        SNode& node = m_nodes[ n - 1 ];
        if ( 0 < node.count )
        {
            TBox bounds;
            for ( UInt32 i=0; i<node.count; ++i )
                bounds = bounds.Merged( mesh.GetTriangle( m_primitives[ node.offset + i ] ).GetBounds() );
            node.bounds = bounds;
        }
        else
        {
            node.bounds = m_nodes[ n ].bounds.Merged( m_nodes[ node.offset ].bounds );
        }
    }
    BuildPseudoNormals();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTMeshBvh< T >::TBox
CTMeshBvh< T >::GetBounds( void ) const
{
    return m_nodes.empty() ? TBox() : m_nodes[ 0 ].bounds;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTMeshBvh< T >::HasSameTree( const CTMeshBvh& other ) const
{
    if ( m_nodes.size() != other.m_nodes.size() || m_primitives != other.m_primitives || m_depth != other.m_depth )
        return false;
    for ( size_t i=0; i<m_nodes.size(); ++i )
    {
        const SNode& a = m_nodes[ i ];
        const SNode& b = other.m_nodes[ i ];
        if ( a.offset != b.offset || a.count != b.count || !( a.bounds == b.bounds ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTMeshBvh< T >::SRaySetup
CTMeshBvh< T >::SetupRay( const TRay& ray )
{
    SRaySetup setup;
    setup.origin = ray.origin;
    const TVector d = ray.direction.AsVector();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T component = d.GetComponent( axis );
        T inverse = TTraits::Zero();
        bool parallel = TTraits::Zero() == component;
        if ( !parallel )
        {
            inverse = TTraits::One() / component;
            parallel = !TTraits::IsFinite( inverse );
        }
        setup.inverse.SetComponent( axis, parallel ? TTraits::Zero() : inverse );
        setup.parallel[ axis ] = parallel;
    }
    return setup;
}

/*-------------------------------------------------------------------------*/

/* a distance moved outwards by a few units in the last place, so that rounding never culls a box a hit lies on */
template < typename T >
GUCEF_GEOMETRY_FORCE_INLINE T
CTMeshBvh< T >::Widened( const T& distance )
{
    return distance + TTraits::Abs( distance ) * ( TTraits::Epsilon() * TTraits::FromInt32( 8 ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTMeshBvh< T >::SlabTest( const TBox& box, const SRaySetup& setup, const T& maxDistance, T& entry )
{
    T tNear = TTraits::Zero();
    T tFar = Widened( maxDistance );
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        const T o = setup.origin.GetComponent( axis );
        const T lo = box.min.GetComponent( axis );
        const T hi = box.max.GetComponent( axis );
        if ( setup.parallel[ axis ] )
        {
            if ( o < lo || hi < o )
                return false;
            continue;
        }
        const T inverse = setup.inverse.GetComponent( axis );
        T t1 = ( lo - o ) * inverse;
        T t2 = ( hi - o ) * inverse;
        if ( t2 < t1 )
        {
            const T swap = t1;
            t1 = t2;
            t2 = swap;
        }
        t1 = t1 - TTraits::Abs( t1 ) * ( TTraits::Epsilon() * TTraits::FromInt32( 8 ) );
        t2 = Widened( t2 );
        if ( tNear < t1 )
            tNear = t1;
        if ( t2 < tFar )
            tFar = t2;
        if ( tFar < tNear )
            return false;
    }
    entry = tNear;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTMeshBvh< T >::SetHit( UInt32 triangle, const TRay& ray, TRayHit& hit ) const
{
    hit.featureIndex = (Int32) triangle;
    if ( TTraits::Zero() < m_faceNormals[ triangle ].Dot( ray.direction.AsVector() ) )
        hit.flags |= RAYHIT_BACKFACE;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTMeshBvh< T >::RayCast( const TRay& ray, const T& maxDistance, TRayHit& hit ) const
{
    if ( m_nodes.empty() || TTraits::IsNegative( maxDistance ) )
        return false;
    const SRaySetup setup = SetupRay( ray );
    T entry = TTraits::Zero();
    if ( !SlabTest( m_nodes[ 0 ].bounds, setup, maxDistance, entry ) )
        return false;

    const TMesh& mesh = *m_mesh;
    T best = maxDistance;
    bool found = false;
    TRayHit bestHit;
    CTraversalStack stack( m_depth );
    stack.Push( 0, entry );
    SStackEntry current;
    while ( stack.Pop( current ) )
    {
        if ( Widened( best ) < current.distance )
            continue;
        const SNode& node = m_nodes[ current.node ];
        if ( 0 < node.count )
        {
            for ( UInt32 i=0; i<node.count; ++i )
            {
                const UInt32 triangle = m_primitives[ node.offset + i ];
                TRayHit candidate;
                if ( RayCastTriangleWatertight( ray, mesh.GetTriangle( triangle ), best, candidate ) && ( !found || candidate.distance < best ) )
                {
                    SetHit( triangle, ray, candidate );
                    bestHit = candidate;
                    best = candidate.distance;
                    found = true;
                }
            }
            continue;
        }
        const UInt32 first = current.node + 1;
        const UInt32 second = node.offset;
        T firstEntry = TTraits::Zero();
        T secondEntry = TTraits::Zero();
        const bool hitFirst = SlabTest( m_nodes[ first ].bounds, setup, best, firstEntry );
        const bool hitSecond = SlabTest( m_nodes[ second ].bounds, setup, best, secondEntry );
        /* the nearer child is popped first */
        if ( hitFirst && hitSecond )
        {
            if ( secondEntry < firstEntry )
            {
                stack.Push( first, firstEntry );
                stack.Push( second, secondEntry );
            }
            else
            {
                stack.Push( second, secondEntry );
                stack.Push( first, firstEntry );
            }
        }
        else if ( hitFirst )
        {
            stack.Push( first, firstEntry );
        }
        else if ( hitSecond )
        {
            stack.Push( second, secondEntry );
        }
    }
    if ( found )
        hit = bestHit;
    return found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTMeshBvh< T >::RayTest( const TRay& ray, const T& maxDistance ) const
{
    if ( m_nodes.empty() || TTraits::IsNegative( maxDistance ) )
        return false;
    const SRaySetup setup = SetupRay( ray );
    const TMesh& mesh = *m_mesh;
    CTraversalStack stack( m_depth );
    stack.Push( 0, TTraits::Zero() );
    SStackEntry current;
    while ( stack.Pop( current ) )
    {
        const SNode& node = m_nodes[ current.node ];
        T entry = TTraits::Zero();
        if ( !SlabTest( node.bounds, setup, maxDistance, entry ) )
            continue;
        if ( 0 < node.count )
        {
            for ( UInt32 i=0; i<node.count; ++i )
            {
                TRayHit candidate;
                if ( RayCastTriangleWatertight( ray, mesh.GetTriangle( m_primitives[ node.offset + i ] ), maxDistance, candidate ) )
                    return true;
            }
            continue;
        }
        stack.Push( node.offset, entry );
        stack.Push( current.node + 1, entry );
    }
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE UInt32
CTMeshBvh< T >::RayCastAll( const TRay& ray, const T& maxDistance, GUCEF::vector< TRayHit >& hits, bool sortByDistance ) const
{
    hits.clear();
    if ( m_nodes.empty() || TTraits::IsNegative( maxDistance ) )
        return 0;
    const SRaySetup setup = SetupRay( ray );
    const TMesh& mesh = *m_mesh;
    CTraversalStack stack( m_depth );
    stack.Push( 0, TTraits::Zero() );
    SStackEntry current;
    while ( stack.Pop( current ) )
    {
        const SNode& node = m_nodes[ current.node ];
        T entry = TTraits::Zero();
        if ( !SlabTest( node.bounds, setup, maxDistance, entry ) )
            continue;
        if ( 0 < node.count )
        {
            for ( UInt32 i=0; i<node.count; ++i )
            {
                const UInt32 triangle = m_primitives[ node.offset + i ];
                TRayHit candidate;
                if ( RayCastTriangleWatertight( ray, mesh.GetTriangle( triangle ), maxDistance, candidate ) )
                {
                    SetHit( triangle, ray, candidate );
                    hits.push_back( candidate );
                }
            }
            continue;
        }
        stack.Push( node.offset, entry );
        stack.Push( current.node + 1, entry );
    }
    if ( sortByDistance )
    {
        /* insertion sort by ( distance, triangle ): hit lists are short, and the order is deterministic */
        for ( size_t i=1; i<hits.size(); ++i )
        {
            const TRayHit moving = hits[ i ];
            size_t j = i;
            while ( j > 0 && ( moving.distance < hits[ j - 1 ].distance || ( !( hits[ j - 1 ].distance < moving.distance ) && moving.featureIndex < hits[ j - 1 ].featureIndex ) ) )
            {
                hits[ j ] = hits[ j - 1 ];
                --j;
            }
            hits[ j ] = moving;
        }
    }
    return (UInt32) hits.size();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTMeshBvh< T >::SquaredDistanceBound( const T& maxDistance )
{
    /* the square of maxDistance, or the largest number of the kind when that would overflow */
    if ( !( maxDistance < TTraits::Sqrt( TTraits::Max() ) ) )
        return TTraits::Max();
    return maxDistance * maxDistance;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTMeshBvh< T >::GetClosestPoint( const TPoint& point, const T& maxDistance, TClosestPoint& result ) const
{
    if ( m_nodes.empty() || TTraits::IsNegative( maxDistance ) || !point.IsFinite() )
        return false;
    const TMesh& mesh = *m_mesh;
    const T limit = SquaredDistanceBound( maxDistance );
    T best = limit;
    bool found = false;
    TClosestPoint bestPoint;

    CTraversalStack stack( m_depth );
    stack.Push( 0, m_nodes[ 0 ].bounds.GetSquaredDistance( point ) );
    SStackEntry current;
    while ( stack.Pop( current ) )
    {
        if ( Widened( best ) < current.distance )
            continue;
        const SNode& node = m_nodes[ current.node ];
        if ( 0 < node.count )
        {
            for ( UInt32 i=0; i<node.count; ++i )
            {
                const UInt32 triangle = m_primitives[ node.offset + i ];
                const TPoint closest = mesh.GetTriangle( triangle ).ClosestPoint( point );
                const T squared = ( closest - point ).GetSquaredLength();
                if ( found ? squared < best : !( limit < squared ) )
                {
                    best = squared;
                    bestPoint.point = closest;
                    bestPoint.squaredDistance = squared;
                    bestPoint.triangle = triangle;
                    found = true;
                }
            }
            continue;
        }
        const UInt32 first = current.node + 1;
        const UInt32 second = node.offset;
        const T firstDistance = m_nodes[ first ].bounds.GetSquaredDistance( point );
        const T secondDistance = m_nodes[ second ].bounds.GetSquaredDistance( point );
        if ( secondDistance < firstDistance )
        {
            stack.Push( first, firstDistance );
            stack.Push( second, secondDistance );
        }
        else
        {
            stack.Push( second, secondDistance );
            stack.Push( first, firstDistance );
        }
    }
    if ( !found )
        return false;
    bestPoint.feature = ClassifyClosestFeature( mesh.GetTriangle( bestPoint.triangle ), point );
    result = bestPoint;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE TMeshFeature
CTMeshBvh< T >::ClassifyClosestFeature( const TTriangle& triangle, const TPoint& point )
{
    /* the regions of CTTriangle3D::ClosestPoint (Ericson 5.1.5), in the same order */
    const T zero = TTraits::Zero();
    if ( triangle.IsDegenerate() )
    {
        /* the nearest of the three edges; its nearer end point when the closest point is an end */
        TMeshFeature feature = MESHFEATURE_EDGE_AB;
        T best = TTraits::Max();
        for ( UInt32 e=0; e<3; ++e )
        {
            const CTSegment3D< T > edge( triangle.GetVertex( e ), triangle.GetVertex( ( e + 1 ) % 3 ) );
            const T squared = ( edge.ClosestPoint( point ) - point ).GetSquaredLength();
            if ( squared < best )
            {
                best = squared;
                feature = (TMeshFeature) ( MESHFEATURE_EDGE_AB + e );
            }
        }
        return feature;
    }
    const TVector ab = triangle.b - triangle.a;
    const TVector ac = triangle.c - triangle.a;
    const TVector ap = point - triangle.a;
    const T d1 = ab.Dot( ap );
    const T d2 = ac.Dot( ap );
    if ( d1 <= zero && d2 <= zero )
        return MESHFEATURE_VERTEX_A;
    const TVector bp = point - triangle.b;
    const T d3 = ab.Dot( bp );
    const T d4 = ac.Dot( bp );
    if ( d3 >= zero && d4 <= d3 )
        return MESHFEATURE_VERTEX_B;
    const T vc = d1 * d4 - d3 * d2;
    if ( vc <= zero && d1 >= zero && d3 <= zero )
        return MESHFEATURE_EDGE_AB;
    const TVector cp = point - triangle.c;
    const T d5 = ab.Dot( cp );
    const T d6 = ac.Dot( cp );
    if ( d6 >= zero && d5 <= d6 )
        return MESHFEATURE_VERTEX_C;
    const T vb = d5 * d2 - d1 * d6;
    if ( vb <= zero && d2 >= zero && d6 <= zero )
        return MESHFEATURE_EDGE_CA;
    const T va = d3 * d6 - d5 * d4;
    if ( va <= zero && ( d4 - d3 ) >= zero && ( d5 - d6 ) >= zero )
        return MESHFEATURE_EDGE_BC;
    return MESHFEATURE_FACE;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTMeshBvh< T >::GetSignedDistance( const TPoint& point, const T& maxDistance, T& signedDistance, TClosestPoint* closest ) const
{
    TClosestPoint result;
    if ( !GetClosestPoint( point, maxDistance, result ) )
        return false;
    const UInt32 t = result.triangle;
    TVector normal;
    switch ( result.feature )
    {
        case MESHFEATURE_VERTEX_A:
        case MESHFEATURE_VERTEX_B:
        case MESHFEATURE_VERTEX_C:
            normal = m_vertexNormals[ m_mesh->indices[ 3 * (size_t) t + ( (UInt32) result.feature - MESHFEATURE_VERTEX_A ) ] ];
            break;
        case MESHFEATURE_EDGE_AB:
        case MESHFEATURE_EDGE_BC:
        case MESHFEATURE_EDGE_CA:
            normal = m_edgeNormals[ 3 * (size_t) t + ( (UInt32) result.feature - MESHFEATURE_EDGE_AB ) ];
            break;
        default:
            normal = m_faceNormals[ t ];
            break;
    }
    const T distance = TTraits::Sqrt( result.squaredDistance );
    signedDistance = TTraits::IsNegative( ( point - result.point ).Dot( normal ) ) ? MATH::ComponentNegate( distance ) : distance;
    if ( GUCEF_NULL != closest )
        *closest = result;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TVisitor >
void
CTMeshBvh< T >::QueryBox( const TBox& box, TVisitor& visitor ) const
{
    if ( m_nodes.empty() || box.IsEmpty() )
        return;
    CTraversalStack stack( m_depth );
    stack.Push( 0, TTraits::Zero() );
    SStackEntry current;
    while ( stack.Pop( current ) )
    {
        const SNode& node = m_nodes[ current.node ];
        if ( !node.bounds.Intersects( box ) )
            continue;
        if ( 0 < node.count )
        {
            for ( UInt32 i=0; i<node.count; ++i )
            {
                const UInt32 triangle = m_primitives[ node.offset + i ];
                if ( m_mesh->GetTriangle( triangle ).GetBounds().Intersects( box ) && !visitor.Visit( triangle ) )
                    return;
            }
            continue;
        }
        stack.Push( node.offset, TTraits::Zero() );
        stack.Push( current.node + 1, TTraits::Zero() );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TShape >
bool
CTMeshBvh< T >::Overlaps( const TShape& shape ) const
{
    SAnyOverlapVisitor< TShape > visitor;
    visitor.bvh = this;
    visitor.shape = &shape;
    visitor.found = false;
    QueryBox( shape.GetBounds(), visitor );
    return visitor.found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TShape >
UInt32
CTMeshBvh< T >::Overlap( const TShape& shape, GUCEF::vector< UInt32 >& triangles ) const
{
    SAllOverlapVisitor< TShape > visitor;
    visitor.bvh = this;
    visitor.shape = &shape;
    visitor.triangles = &triangles;
    visitor.added = 0;
    QueryBox( shape.GetBounds(), visitor );
    return visitor.added;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTMeshBvh< Float32 > TFloat32MeshBvh;
typedef CTMeshBvh< Float64 > TFloat64MeshBvh;
typedef CTMeshBvh< Float32 > CMeshBvh;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTMeshBvh< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTMeshBvh< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#include "gucefGEOMETRY_CTTriangleMesh.h"
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */
