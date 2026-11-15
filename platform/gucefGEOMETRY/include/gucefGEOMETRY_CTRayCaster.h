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

#ifndef GUCEF_GEOMETRY_CTRAYCASTER_H
#define GUCEF_GEOMETRY_CTRAYCASTER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#include "gucefGEOMETRY_CTDynamicAabbTree.h"
#define GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H
#endif /* GUCEF_GEOMETRY_CTDYNAMICAABBTREE_H ? */

#ifndef GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H
#include "gucefGEOMETRY_CTGeoTriangleMesh.h"
#define GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTGEOTRIANGLEMESH_H ? */

#ifndef GUCEF_GEOMETRY_CTGEORAYFILTER_H
#include "gucefGEOMETRY_CTGeoRayFilter.h"
#define GUCEF_GEOMETRY_CTGEORAYFILTER_H
#endif /* GUCEF_GEOMETRY_CTGEORAYFILTER_H ? */

#ifndef GUCEF_GEOMETRY_SGEORAYHIT_H
#include "gucefGEOMETRY_SGeoRayHit.h"
#define GUCEF_GEOMETRY_SGEORAYHIT_H
#endif /* GUCEF_GEOMETRY_SGEORAYHIT_H ? */

#ifndef GUCEF_GEOMETRY_SWEEPQUERIES_H
#include "gucefGEOMETRY_SweepQueries.h"
#define GUCEF_GEOMETRY_SWEEPQUERIES_H
#endif /* GUCEF_GEOMETRY_SWEEPQUERIES_H ? */

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
 *  Scene queries over linked objects (section 7.7 of the design): ray casts, sweeps and overlaps against the objects
 *  registered with AddObject, whatever spaces they live in.
 *
 *  Structure
 *  - Broadphase: a CTDynamicAabbTree over the world bounds of the bounded objects (fattened by the margin given to the
 *    constructor), with the object pointer as proxy user data; unbounded objects (an unbounded plane or plane-bounded
 *    volume) are kept in a list that every query tests. The ray caster is a CTGeoObjectStructure: a registered object
 *    reports its moves (its own, an ancestor's, or a shape change) once, and Update refits only the moved objects,
 *    O(moved). A destroyed object unregisters itself, a destroyed ray caster unregisters from its objects.
 *  - Update also brings the world matrices, inverse world matrices and world bounds of the moved objects up to date,
 *    so that the const queries only read the caches. Queries before Update see the previous placements of moved
 *    objects in the broadphase.
 *
 *  Narrowphase
 *  - Ray casts: CTGeoObjectBase::RayCast of each candidate (the ray mapped into the object's space, exact under any
 *    affine placement), nearest first by clipping the broadphase segment; a hit at exactly the distance of an earlier
 *    one keeps the earlier one.
 *  - Sphere and shape casts: bounded convex objects by the GJK ray cast against their placed support mapping;
 *    triangle meshes (objects whose AsTriangleMesh is not null, CTGeoTriangleMesh) triangle by triangle in world space
 *    (the candidate triangles from the BVH of the mesh, each placed by the world matrix), exact under any affine
 *    placement. Other objects (non-convex flat shapes, unbounded shapes) are not swept.
 *  - OverlapSphere and CheckSphere: the distance of the centre to each candidate (CTGeoObjectBase::GetDistance) against
 *    the radius; meshes triangle by triangle in world space. OverlapBox: GJK on the placed support mappings for bounded
 *    convex objects, meshes triangle by triangle; other objects are not tested.
 *  - Filtering: an object takes part when ( object.GetLayerMask() & layerMask ) != 0 and the optional filter accepts it.
 *  - HasLineOfSight: no hit (any object, including one the segment starts inside) on the segment from 'from' to 'to'.
 *  - RaycastBatch: Raycast per ray on the calling thread.
 *
 *  Threading: owner thread for AddObject, RemoveObject, Update and every change of the scene; the const queries may run
 *  concurrently on several threads while nothing of that runs (and while no registered object or its spaces change).
 *  Update must follow the last change before concurrent queries start: a moved object (or an object under a moved
 *  space) recomputes its world caches lazily on its first query, which is a write and races with other query
 *  threads. Update (like AddObject) brings every cache the queries read up to date (world matrix, inverse world
 *  matrix, world bounds), so queries after it only read. Queries between a change and Update are fine on the owner
 *  thread alone. Queries are not realtime-safe (time depends on the scene). Kinds: Float32 and Float64 are exported.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTRayCaster : public CTGeoObjectStructure< T >
{
    public:

    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTMatrix4D< T > TMatrix;
    typedef MATH::CTTransform3D< T > TTransform;
    typedef CTAxisAlignedBox3D< T > TBox;
    typedef CTRay3D< T > TRay;
    typedef CTGeoObjectBase< T > TObject;
    typedef CTGeoTriangleMesh< T > TMeshObject;
    typedef CTGeoRayFilter< T > TFilter;
    typedef SGeoRayHit< T > THit;
    typedef SRayCastResult< T > TShapeHit;
    typedef CTDynamicAabbTree< T, TObject* > TTree;

    /** An empty ray caster with the given broadphase margin (world units, not negative) */
    explicit CTRayCaster( const T& margin = TTree::DefaultMargin() );

    /** Unregisters from every object */
    virtual ~CTRayCaster();

    /** Registers an object (non-owning); false when it is registered already. A mesh (AsTriangleMesh) also takes part in sweeps and overlaps */
    bool AddObject( TObject& object );

    /** Unregisters an object; false when it was not registered */
    bool RemoveObject( TObject& object );

    /** True when the object is registered */
    bool IsRegistered( const TObject& object ) const;

    /** Refits the moved objects (section 7.4.3); owner thread */
    void Update( void );

    /** The number of objects moved since the last Update */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetMovedCount( void ) const { return (UInt32) m_moved.size(); }

    /** The number of registered unbounded objects (tested by every query) */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetUnboundedCount( void ) const { return (UInt32) m_unbounded.size(); }

    /** The broadphase tree */
    GUCEF_GEOMETRY_FORCE_INLINE const TTree& GetTree( void ) const { return m_tree; }

    /** The nearest hit within maxDistance */
    bool Raycast( const TRay& ray, const T& maxDistance, UInt32 layerMask, THit& hit, const TFilter* filter = GUCEF_NULL ) const;

    /** Every hit within maxDistance, at most one per object (its nearest); replaces the content of 'hits'; returns the count */
    UInt32 RaycastAll( const TRay& ray, const T& maxDistance, UInt32 layerMask, GUCEF::vector< THit >& hits, bool sortByDistance, const TFilter* filter = GUCEF_NULL ) const;

    /** The nearest hit of a sphere of 'radius' moved along the ray by at most maxDistance */
    bool SphereCast( const TRay& ray, const T& radius, const T& maxDistance, UInt32 layerMask, THit& hit, const TFilter* filter = GUCEF_NULL ) const;

    /** The nearest hit of a bounded convex shape placed by 'start' and moved by 'translation' */
    template < typename TConvex >
    bool ShapeCast( const TConvex& shape, const TTransform& start, const TVector& translation, UInt32 layerMask, THit& hit, const TFilter* filter = GUCEF_NULL ) const;

    /** The objects that share a point with the sphere (appended to 'results'); returns how many were added */
    UInt32 OverlapSphere( const CTSphere< T >& sphere, UInt32 layerMask, GUCEF::vector< TObject* >& results, const TFilter* filter = GUCEF_NULL ) const;

    /** The objects that share a point with the oriented box (appended to 'results'); returns how many were added */
    UInt32 OverlapBox( const CTOrientedBox3D< T >& box, UInt32 layerMask, GUCEF::vector< TObject* >& results, const TFilter* filter = GUCEF_NULL ) const;

    /** True when any object shares a point with the sphere */
    bool CheckSphere( const CTSphere< T >& sphere, UInt32 layerMask, const TFilter* filter = GUCEF_NULL ) const;

    /** True when no object is hit on the segment from 'from' to 'to' */
    bool HasLineOfSight( const TPoint& from, const TPoint& to, UInt32 layerMask, const TFilter* filter = GUCEF_NULL ) const;

    /** Raycast for each of 'count' rays: hits[ i ] and hitFlags[ i ] receive the result of ray i */
    void RaycastBatch( const TRay* rays, UInt32 count, const T& maxDistance, UInt32 layerMask, THit* hits, bool* hitFlags, const TFilter* filter = GUCEF_NULL ) const;

    protected:

    virtual void OnGeoObjectMoved( TObject& object ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnGeoObjectDestroyed( TObject& object ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SEntry
    {
        Int32 proxy;                /**< TTree::NullProxy for an unbounded object */
        TBox bounds;                /**< the world bounds of the last refit */
        const TMeshObject* mesh;    /**< the object's AsTriangleMesh at registration */
    };

    typedef GUCEF::map< const TObject*, SEntry > TEntryMap;

    /** Common parts of the visitors: filtering */
    struct SFilterContext
    {
        UInt32 layerMask;
        const TFilter* filter;

        GUCEF_GEOMETRY_FORCE_INLINE bool Accepts( const TObject& object ) const
        {
            return 0 != ( object.GetLayerMask() & layerMask ) && ( GUCEF_NULL == filter || filter->Accept( object ) );
        }
    };

    /** The nearest ray hit (Raycast), or any hit (anyHit, HasLineOfSight) */
    struct SNearestRayVisitor
    {
        const TTree* tree;
        const TRay* ray;
        SFilterContext context;
        bool anyHit;
        bool found;
        THit best;

        bool Visit( Int32 proxyId, T& maxDistance );
        bool Test( TObject& object, T& maxDistance );
    };

    /** Every ray hit (RaycastAll) */
    struct SAllRayVisitor
    {
        const TTree* tree;
        const TRay* ray;
        T maxDistance;
        SFilterContext context;
        GUCEF::vector< THit >* hits;

        bool Visit( Int32 proxyId, T& maxDistance );
        void Test( TObject& object );
    };

    /** An overlap query: TTest::Overlaps( caster, object ) decides each candidate that passes the filter */
    template < typename TTest >
    struct STOverlapVisitor
    {
        const CTRayCaster* caster;
        const TTest* test;
        SFilterContext context;
        GUCEF::vector< TObject* >* results;     /**< GUCEF_NULL: stop at the first overlap */
        UInt32 added;

        bool Visit( Int32 proxyId ) { return Check( *caster->m_tree.GetUserData( proxyId ) ); }

        bool Check( TObject& object )
        {
            if ( !context.Accepts( object ) || object.IsEmpty() || !test->Overlaps( *caster, object ) )
                return true;
            ++added;
            if ( GUCEF_NULL == results )
                return false;
            results->push_back( &object );
            return true;
        }
    };

    /** The sphere test of OverlapSphere */
    struct SSphereOverlapTest
    {
        CTSphere< T > sphere;
        TBox bounds;

        bool Overlaps( const CTRayCaster& caster, const TObject& object ) const;
    };

    /** The oriented box test of OverlapBox */
    struct SBoxOverlapTest
    {
        CTOrientedBox3D< T > box;
        CTOrientedBox3D< T > relative;      /**< the box moved to the origin */
        TVector origin;
        TBox bounds;

        bool Overlaps( const CTRayCaster& caster, const TObject& object ) const;
    };

    /** A sweep of a moving convex support mapping (world, relative to 'origin') against candidate objects */
    template < typename TSupport >
    struct STSweepVisitor
    {
        const CTRayCaster* caster;
        const TSupport* support;
        TVector origin;
        TVector translation;    /**< the full motion */
        T length;               /**< its length */
        SFilterContext context;
        bool found;
        THit best;

        bool Visit( Int32 proxyId );
        void Test( TObject& object );
    };

    /** The triangles of a mesh object as a candidate set for the world-space triangle tests */
    template < typename TTriangleTest >
    struct STMeshTriangleVisitor
    {
        const TMatrix* world;
        const CTTriangleMesh< T >* triangles;
        TTriangleTest* test;

        bool Visit( UInt32 triangle )
        {
            const CTTriangle3D< T > local = triangles->GetTriangle( triangle );
            const CTTriangle3D< T > placed( world->TransformPoint( local.a ), world->TransformPoint( local.b ), world->TransformPoint( local.c ) );
            return test->Test( placed, triangle );
        }
    };

    /** World-space triangle test of a sweep */
    template < typename TSupport >
    struct STTriangleSweepTest
    {
        const TSupport* support;
        TVector origin;
        TVector translation;
        T length;
        bool found;
        TShapeHit best;

        bool Test( const CTTriangle3D< T >& triangle, UInt32 index );
    };

    /** World-space triangle test of a sphere or box overlap */
    template < typename TShape >
    struct STTriangleOverlapTest
    {
        const TShape* shape;
        bool found;

        bool Test( const CTTriangle3D< T >& triangle, UInt32 ) { found = Intersects( triangle, *shape ); return !found; }
    };

    CTRayCaster( const CTRayCaster& );
    CTRayCaster& operator=( const CTRayCaster& );

    void Forget( const TObject& object );
    void Refit( TObject& object, SEntry& entry );
    const SEntry* FindEntry( const TObject& object ) const;

    template < typename TTest >
    UInt32 Overlap( const TTest& test, const TBox& bounds, bool testUnbounded, UInt32 layerMask, GUCEF::vector< TObject* >* results, const TFilter* filter ) const;

    template < typename TSupport >
    bool Sweep( const TSupport& support, const TVector& origin, const TBox& startBounds, const TVector& translation, const SFilterContext& context, THit& hit ) const;

    template < typename TShape >
    bool MeshOverlaps( const TMeshObject& mesh, const TShape& worldShape, const TBox& worldBounds ) const;

    static TBox ToLocalBox( const TObject& object, const TBox& worldBox );

    TTree m_tree;
    TEntryMap m_entries;
    GUCEF::vector< TObject* > m_unbounded;
    GUCEF::vector< TObject* > m_moved;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTRayCaster< T >::CTRayCaster( const T& margin )
    : CTGeoObjectStructure< T >(), m_tree( margin ), m_entries(), m_unbounded(), m_moved()
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTRayCaster< T >::~CTRayCaster()
{
    this->UnregisterAll();
}

/*-------------------------------------------------------------------------*/

template < typename T >
const typename CTRayCaster< T >::SEntry*
CTRayCaster< T >::FindEntry( const TObject& object ) const
{
    typename TEntryMap::const_iterator i = m_entries.find( &object );
    return i == m_entries.end() ? GUCEF_NULL : &( *i ).second;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::IsRegistered( const TObject& object ) const
{
    return GUCEF_NULL != FindEntry( object );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::AddObject( TObject& object )
{
    if ( IsRegistered( object ) || !object.RegisterStructure( *this ) )
        return false;
    SEntry entry;
    entry.proxy = TTree::NullProxy;
    entry.bounds = TBox();
    entry.mesh = object.AsTriangleMesh();
    /* the caches are read here so that the const queries only read them */
    const TBox& bounds = object.GetWorldBounds();
    object.GetInverseWorldMatrix();
    entry.bounds = bounds;
    if ( object.IsBounded() )
        entry.proxy = m_tree.CreateProxy( bounds, &object );
    else
        m_unbounded.push_back( &object );
    m_entries[ &object ] = entry;
    return true;
}

/*-------------------------------------------------------------------------*/

/* removes every reference to the object: its proxy or unbounded slot, its moved entry and its entry */
template < typename T >
void
CTRayCaster< T >::Forget( const TObject& object )
{
    typename TEntryMap::iterator i = m_entries.find( &object );
    if ( i == m_entries.end() )
        return;
    if ( TTree::NullProxy != ( *i ).second.proxy )
    {
        m_tree.DestroyProxy( ( *i ).second.proxy );
    }
    else
    {
        for ( size_t u=0; u<m_unbounded.size(); ++u )
        {
            if ( m_unbounded[ u ] == &object )
            {
                m_unbounded.erase( m_unbounded.begin() + u );
                break;
            }
        }
    }
    for ( size_t m=0; m<m_moved.size(); ++m )
    {
        if ( m_moved[ m ] == &object )
        {
            m_moved.erase( m_moved.begin() + m );
            break;
        }
    }
    m_entries.erase( i );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::RemoveObject( TObject& object )
{
    if ( !IsRegistered( object ) )
        return false;
    object.UnregisterStructure( *this );
    Forget( object );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTRayCaster< T >::OnGeoObjectMoved( TObject& object )
{
    /* told at most once until acknowledged: no duplicates */
    m_moved.push_back( &object );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTRayCaster< T >::OnGeoObjectDestroyed( TObject& object )
{
    Forget( object );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTRayCaster< T >::Refit( TObject& object, SEntry& entry )
{
    const TBox& bounds = object.AcknowledgeMove( *this );
    object.GetInverseWorldMatrix();
    const bool bounded = object.IsBounded();
    if ( bounded && TTree::NullProxy != entry.proxy )
    {
        const TVector displacement = bounds.IsEmpty() || entry.bounds.IsEmpty() ? TVector() : bounds.GetCentre() - entry.bounds.GetCentre();
        m_tree.MoveProxy( entry.proxy, bounds, displacement );
    }
    else if ( bounded )
    {
        /* became bounded: from the unbounded list into the tree */
        for ( size_t u=0; u<m_unbounded.size(); ++u )
        {
            if ( m_unbounded[ u ] == &object )
            {
                m_unbounded.erase( m_unbounded.begin() + u );
                break;
            }
        }
        entry.proxy = m_tree.CreateProxy( bounds, &object );
    }
    else if ( TTree::NullProxy != entry.proxy )
    {
        /* became unbounded */
        m_tree.DestroyProxy( entry.proxy );
        entry.proxy = TTree::NullProxy;
        m_unbounded.push_back( &object );
    }
    entry.bounds = bounds;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTRayCaster< T >::Update( void )
{
    /* the list is taken first: reading placements re-arms notifications, which must not append to the list being processed */
    GUCEF::vector< TObject* > moved;
    moved.swap( m_moved );
    for ( size_t m=0; m<moved.size(); ++m )
    {
        typename TEntryMap::iterator i = m_entries.find( moved[ m ] );
        if ( i != m_entries.end() )
            Refit( *moved[ m ], ( *i ).second );
    }
    /* keep the capacity for the next frame */
    moved.clear();
    if ( m_moved.empty() )
        m_moved.swap( moved );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::SNearestRayVisitor::Test( TObject& object, T& maxDistance )
{
    if ( !context.Accepts( object ) )
        return true;
    TShapeHit shapeHit;
    if ( !object.RayCast( *ray, maxDistance, shapeHit ) )
        return true;
    if ( !found || shapeHit.distance < best.distance )
    {
        best = THit( shapeHit, &object );
        found = true;
        maxDistance = shapeHit.distance;
    }
    return !anyHit;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::SNearestRayVisitor::Visit( Int32 proxyId, T& maxDistance )
{
    return Test( *tree->GetUserData( proxyId ), maxDistance );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::Raycast( const TRay& ray, const T& maxDistance, UInt32 layerMask, THit& hit, const TFilter* filter ) const
{
    if ( TTraits::IsNegative( maxDistance ) )
        return false;
    SNearestRayVisitor visitor;
    visitor.tree = &m_tree;
    visitor.ray = &ray;
    visitor.context.layerMask = layerMask;
    visitor.context.filter = filter;
    visitor.anyHit = false;
    visitor.found = false;
    T limit = maxDistance;
    for ( size_t u=0; u<m_unbounded.size(); ++u )
        visitor.Test( *m_unbounded[ u ], limit );
    m_tree.RayCast( ray, limit, visitor );
    if ( visitor.found )
        hit = visitor.best;
    return visitor.found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTRayCaster< T >::SAllRayVisitor::Test( TObject& object )
{
    if ( !context.Accepts( object ) )
        return;
    TShapeHit shapeHit;
    if ( object.RayCast( *ray, maxDistance, shapeHit ) )
        hits->push_back( THit( shapeHit, &object ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::SAllRayVisitor::Visit( Int32 proxyId, T& )
{
    Test( *tree->GetUserData( proxyId ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTRayCaster< T >::RaycastAll( const TRay& ray, const T& maxDistance, UInt32 layerMask, GUCEF::vector< THit >& hits, bool sortByDistance, const TFilter* filter ) const
{
    hits.clear();
    if ( TTraits::IsNegative( maxDistance ) )
        return 0;
    SAllRayVisitor visitor;
    visitor.tree = &m_tree;
    visitor.ray = &ray;
    visitor.maxDistance = maxDistance;
    visitor.context.layerMask = layerMask;
    visitor.context.filter = filter;
    visitor.hits = &hits;
    for ( size_t u=0; u<m_unbounded.size(); ++u )
        visitor.Test( *m_unbounded[ u ] );
    m_tree.RayCast( ray, maxDistance, visitor );
    if ( sortByDistance )
    {
        /* insertion sort by distance; equal distances keep the order of the traversal */
        for ( size_t i=1; i<hits.size(); ++i )
        {
            const THit moving = hits[ i ];
            size_t j = i;
            while ( j > 0 && moving.distance < hits[ j - 1 ].distance )
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
bool
CTRayCaster< T >::HasLineOfSight( const TPoint& from, const TPoint& to, UInt32 layerMask, const TFilter* filter ) const
{
    const TVector delta = to - from;
    const T length = delta.GetLength();
    MATH::CTDirectionVector3D< T > direction;
    if ( !( TTraits::Zero() < length ) || !MATH::CTDirectionVector3D< T >::TryCreate( delta, direction ) )
        return true;
    const TRay ray( from, direction );
    SNearestRayVisitor visitor;
    visitor.tree = &m_tree;
    visitor.ray = &ray;
    visitor.context.layerMask = layerMask;
    visitor.context.filter = filter;
    visitor.anyHit = true;
    visitor.found = false;
    T limit = length;
    for ( size_t u=0; u<m_unbounded.size() && !visitor.found; ++u )
        visitor.Test( *m_unbounded[ u ], limit );
    if ( !visitor.found )
    {
        limit = length;
        m_tree.RayCast( ray, limit, visitor );
    }
    return !visitor.found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTRayCaster< T >::RaycastBatch( const TRay* rays, UInt32 count, const T& maxDistance, UInt32 layerMask, THit* hits, bool* hitFlags, const TFilter* filter ) const
{
    for ( UInt32 i=0; i<count; ++i )
    {
        THit hit;
        const bool found = Raycast( rays[ i ], maxDistance, layerMask, hit, filter );
        hits[ i ] = found ? hit : THit();
        hitFlags[ i ] = found;
    }
}

/*-------------------------------------------------------------------------*/

/* a world box mapped into the object's local space (the bounds of its transformed corners) */
template < typename T >
typename CTRayCaster< T >::TBox
CTRayCaster< T >::ToLocalBox( const TObject& object, const TBox& worldBox )
{
    return worldBox.Transformed( object.GetInverseWorldMatrix() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TSupport >
bool
CTRayCaster< T >::STTriangleSweepTest< TSupport >::Test( const CTTriangle3D< T >& triangle, UInt32 index )
{
    const CTTriangle3D< T > relative( triangle.a - origin, triangle.b - origin, triangle.c - origin );
    SGjkCastResult< T > cast;
    CTGjk< T >::Cast( *support, translation, TTraits::One(), CTSupportMapping< CTTriangle3D< T > >::Make( relative ), CTGeometryTolerance< T >(), cast );
    if ( !cast.hit )
        return true;
    TShapeHit hit;
    hit.distance = cast.fraction * length;
    hit.point = cast.pointOnB + origin;
    hit.featureIndex = (Int32) index;
    if ( TTraits::Zero() == cast.fraction || cast.normal.IsZero() )
    {
        hit.distance = TTraits::Zero();
        MATH::CTDirectionVector3D< T > back;
        hit.normal = MATH::CTDirectionVector3D< T >::TryCreate( -translation, back ) ? back.AsVector() : TVector();
        hit.flags = RAYHIT_STARTED_INSIDE;
    }
    else
    {
        hit.normal = cast.normal;
    }
    if ( !found || hit.distance < best.distance || ( !( best.distance < hit.distance ) && hit.featureIndex < best.featureIndex ) )
    {
        best = hit;
        found = true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TSupport >
void
CTRayCaster< T >::STSweepVisitor< TSupport >::Test( TObject& object )
{
    if ( !context.Accepts( object ) || object.IsEmpty() )
        return;
    TShapeHit hit;
    bool hitFound = false;
    const SEntry* entry = caster->FindEntry( object );
    if ( GUCEF_NULL != entry && GUCEF_NULL != entry->mesh )
    {
        /* candidate triangles from the BVH by the swept box mapped into the mesh's space, each tested placed in world */
        const TMeshObject& mesh = *entry->mesh;
        const TBox start = GetSupportBounds( *support );
        const TBox swept = start.Merged( TBox( start.min + translation, start.max + translation ) );
        const TBox worldSwept( swept.min + origin, swept.max + origin );
        STTriangleSweepTest< TSupport > test;
        test.support = support;
        test.origin = origin;
        test.translation = translation;
        test.length = length;
        test.found = false;
        STMeshTriangleVisitor< STTriangleSweepTest< TSupport > > visitor;
        visitor.world = &mesh.GetWorldMatrix();
        visitor.triangles = mesh.GetMesh().GetPointerAlways();
        visitor.test = &test;
        mesh.GetBvh()->QueryBox( ToLocalBox( mesh, worldSwept ), visitor );
        hitFound = test.found;
        hit = test.best;
    }
    else if ( object.IsConvex() && object.IsBounded() )
    {
        SGjkCastResult< T > cast;
        CTGjk< T >::Cast( *support, translation, TTraits::One(), object.GetPlacedSupport( origin ), CTGeometryTolerance< T >(), cast );
        if ( cast.hit )
        {
            hitFound = true;
            hit.distance = cast.fraction * length;
            hit.point = cast.pointOnB + origin;
            if ( TTraits::Zero() == cast.fraction || cast.normal.IsZero() )
            {
                hit.distance = TTraits::Zero();
                MATH::CTDirectionVector3D< T > back;
                hit.normal = MATH::CTDirectionVector3D< T >::TryCreate( -translation, back ) ? back.AsVector() : TVector();
                hit.flags = RAYHIT_STARTED_INSIDE;
            }
            else
            {
                hit.normal = cast.normal;
            }
        }
    }
    if ( hitFound && ( !found || hit.distance < best.distance ) )
    {
        best = THit( hit, &object );
        found = true;
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TSupport >
bool
CTRayCaster< T >::STSweepVisitor< TSupport >::Visit( Int32 proxyId )
{
    Test( *caster->m_tree.GetUserData( proxyId ) );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TSupport >
bool
CTRayCaster< T >::Sweep( const TSupport& support, const TVector& origin, const TBox& startBounds, const TVector& translation, const SFilterContext& context, THit& hit ) const
{
    STSweepVisitor< TSupport > visitor;
    visitor.caster = this;
    visitor.support = &support;
    visitor.origin = origin;
    visitor.translation = translation;
    visitor.length = translation.GetLength();
    visitor.context = context;
    visitor.found = false;
    const TBox swept = startBounds.Merged( TBox( startBounds.min + translation, startBounds.max + translation ) );
    m_tree.Query( TBox( swept.min + origin, swept.max + origin ), visitor );
    if ( visitor.found )
        hit = visitor.best;
    return visitor.found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::SphereCast( const TRay& ray, const T& radius, const T& maxDistance, UInt32 layerMask, THit& hit, const TFilter* filter ) const
{
    if ( TTraits::IsNegative( radius ) || TTraits::IsNegative( maxDistance ) )
        return false;
    /* the sphere relative to the ray origin: a point core with the radius as margin */
    const CTSphere< T > sphere( TPoint(), radius );
    const typename CTSupportMapping< CTSphere< T > >::TSupport support = CTSupportMapping< CTSphere< T > >::Make( sphere );
    SFilterContext context;
    context.layerMask = layerMask;
    context.filter = filter;
    const TVector half( radius, radius, radius );
    return Sweep( support, ray.origin.ToPositionVector(), TBox( TPoint( -half ), TPoint( half ) ), ray.direction.AsVector() * maxDistance, context, hit );
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TConvex >
bool
CTRayCaster< T >::ShapeCast( const TConvex& shape, const TTransform& start, const TVector& translation, UInt32 layerMask, THit& hit, const TFilter* filter ) const
{
    if ( !IsQueryableConvex( shape ) )
        return false;
    typedef typename CTSupportMapping< TConvex >::TSupport TShapeSupport;
    typedef CTTransformedSupport< TShapeSupport > TPlaced;
    const TVector origin = start.position;
    const TMatrix placement = start.ToMatrixRelativeTo( origin );
    const TPlaced support( CTSupportMapping< TConvex >::Make( shape ), placement.GetLinear(), placement.GetTranslation() );
    SFilterContext context;
    context.layerMask = layerMask;
    context.filter = filter;
    return Sweep( support, origin, GetSupportBounds( support ), translation, context, hit );
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TShape >
bool
CTRayCaster< T >::MeshOverlaps( const TMeshObject& mesh, const TShape& worldShape, const TBox& worldBounds ) const
{
    STTriangleOverlapTest< TShape > test;
    test.shape = &worldShape;
    test.found = false;
    STMeshTriangleVisitor< STTriangleOverlapTest< TShape > > visitor;
    visitor.world = &mesh.GetWorldMatrix();
    visitor.triangles = mesh.GetMesh().GetPointerAlways();
    visitor.test = &test;
    mesh.GetBvh()->QueryBox( ToLocalBox( mesh, worldBounds ), visitor );
    return test.found;
}

/*-------------------------------------------------------------------------*/

template < typename T >
template < typename TTest >
UInt32
CTRayCaster< T >::Overlap( const TTest& test, const TBox& bounds, bool testUnbounded, UInt32 layerMask, GUCEF::vector< TObject* >* results, const TFilter* filter ) const
{
    STOverlapVisitor< TTest > visitor;
    visitor.caster = this;
    visitor.test = &test;
    visitor.context.layerMask = layerMask;
    visitor.context.filter = filter;
    visitor.results = results;
    visitor.added = 0;
    if ( testUnbounded )
    {
        for ( size_t u=0; u<m_unbounded.size(); ++u )
            if ( !visitor.Check( *m_unbounded[ u ] ) )
                return visitor.added;
    }
    m_tree.Query( bounds, visitor );
    return visitor.added;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::SSphereOverlapTest::Overlaps( const CTRayCaster& caster, const TObject& object ) const
{
    const SEntry* entry = caster.FindEntry( object );
    if ( GUCEF_NULL != entry && GUCEF_NULL != entry->mesh )
        return caster.MeshOverlaps( *entry->mesh, sphere, bounds );
    return !( sphere.radius < object.GetDistance( sphere.centre ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::SBoxOverlapTest::Overlaps( const CTRayCaster& caster, const TObject& object ) const
{
    const SEntry* entry = caster.FindEntry( object );
    if ( GUCEF_NULL != entry && GUCEF_NULL != entry->mesh )
        return caster.MeshOverlaps( *entry->mesh, box, bounds );
    if ( !object.IsConvex() || !object.IsBounded() )
        return false;
    return CTGjk< T >::Intersects( CTSupportMapping< CTOrientedBox3D< T > >::Make( relative ), object.GetPlacedSupport( origin ), CTGeometryTolerance< T >() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTRayCaster< T >::OverlapSphere( const CTSphere< T >& sphere, UInt32 layerMask, GUCEF::vector< TObject* >& results, const TFilter* filter ) const
{
    if ( TTraits::IsNegative( sphere.radius ) )
        return 0;
    SSphereOverlapTest test;
    test.sphere = sphere;
    test.bounds = sphere.GetBounds();
    return Overlap( test, test.bounds, true, layerMask, &results, filter );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTRayCaster< T >::CheckSphere( const CTSphere< T >& sphere, UInt32 layerMask, const TFilter* filter ) const
{
    if ( TTraits::IsNegative( sphere.radius ) )
        return false;
    SSphereOverlapTest test;
    test.sphere = sphere;
    test.bounds = sphere.GetBounds();
    return 0 < Overlap( test, test.bounds, true, layerMask, GUCEF_NULL, filter );
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTRayCaster< T >::OverlapBox( const CTOrientedBox3D< T >& box, UInt32 layerMask, GUCEF::vector< TObject* >& results, const TFilter* filter ) const
{
    if ( !box.IsValid() )
        return 0;
    SBoxOverlapTest test;
    test.box = box;
    test.relative = box;
    test.relative.centre = TPoint();
    test.origin = box.centre.ToPositionVector();
    test.bounds = box.GetBounds();
    return Overlap( test, test.bounds, false, layerMask, &results, filter );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTRayCaster< Float32 > CRayCaster;
typedef CTRayCaster< Float64 > CRayCaster64;
typedef CTDynamicAabbTree< Float32, CTGeoObjectBase< Float32 >* > TFloat32GeoObjectAabbTree;
typedef CTDynamicAabbTree< Float64, CTGeoObjectBase< Float64 >* > TFloat64GeoObjectAabbTree;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/* the template arguments hold a comma: a macro stands in for it (a typedef name may not follow 'class') */
#define GUCEF_GEOMETRY_RAYCASTER_COMMA ,
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTDynamicAabbTree< Float32 GUCEF_GEOMETRY_RAYCASTER_COMMA CTGeoObjectBase< Float32 >* > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTDynamicAabbTree< Float64 GUCEF_GEOMETRY_RAYCASTER_COMMA CTGeoObjectBase< Float64 >* > )
#undef GUCEF_GEOMETRY_RAYCASTER_COMMA
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTRayCaster< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTRayCaster< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTRAYCASTER_H ? */
