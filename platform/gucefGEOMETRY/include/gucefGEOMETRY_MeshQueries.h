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

#ifndef GUCEF_GEOMETRY_MESHQUERIES_H
#define GUCEF_GEOMETRY_MESHQUERIES_H

/*
 *  Meshes against convex value shapes (sections 7.5.3 and 7.5.7 of the design), on a mesh through its CTMeshBvh:
 *
 *  - MeshIntersects( mesh, shape ): a triangle shares a point with the bounded convex shape (CTMeshBvh::Overlaps).
 *  - MeshClosestPoints( mesh, shape, onMesh, onShape, triangle ): the nearest points of the mesh surface and the shape,
 *    by branch and bound over the BVH (the distance of a node box to the bounds of the shape is the lower bound) and
 *    ClosestPoints of a triangle and the shape; false for an empty mesh or a shape without points.
 *  - MeshComputeContact( mesh, shape, deepest, all ): ComputeContact of every triangle whose box touches the shape's
 *    bounds with the shape (the triangle is A, the shape B); 'deepest' receives the touching contact of greatest depth,
 *    'all' (optional) every touching triangle contact. True when any triangle touches.
 *  - MeshSphereCast( ray, radius, mesh, maxDistance, result ) and MeshShapeCast( shape, direction, maxDistance, mesh,
 *    result ): the nearest sweep hit over the triangles whose boxes touch the swept bounds (the analytic sphere cast of
 *    a triangle, and the GJK cast of ShapeCast); featureIndex is the triangle.
 *  - The ray cast of a mesh is CTMeshBvh::RayCast (also through the facade RayCast( ray, bvh, maxDistance, result )).
 *
 *  The functions carry a Mesh prefix: a generic Intersects( mesh, shape ) would be ambiguous with the generic ray
 *  facade overloads of gucefGEOMETRY_RayQueries.h. Float32 and Float64 (the kinds of the BVH). Allocation-free except
 *  for the optional 'all' list and for BVHs deeper than 64 levels; not realtime-safe (section 2.4 rule 5).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_SWEEPQUERIES_H
#include "gucefGEOMETRY_SweepQueries.h"
#define GUCEF_GEOMETRY_SWEEPQUERIES_H
#endif /* GUCEF_GEOMETRY_SWEEPQUERIES_H ? */

#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#include "gucefGEOMETRY_CTMeshBvh.h"
#define GUCEF_GEOMETRY_CTMESHBVH_H
#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The contact of one mesh triangle with a shape (MeshComputeContact) */
template < typename T >
struct SMeshTriangleContact
{
    UInt32 triangle;
    CTContactManifold< T > manifold;

    SMeshTriangleContact( void ) : triangle( 0 ), manifold() {}
};

/** An entry of the explicit traversal stack of the mesh queries */
template < typename T >
struct SMeshBvhStackEntry
{
    UInt32 node;
    T distance;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The squared distance between two boxes (0 when they touch), the largest number of the kind when one is empty */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
MeshBoxSquaredDistance( const CTAxisAlignedBox3D< T >& a, const CTAxisAlignedBox3D< T >& b )
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( a.IsEmpty() || b.IsEmpty() )
        return TTraits::Max();
    T sum = TTraits::Zero();
    for ( UInt32 axis=0; axis<3; ++axis )
    {
        T gap = TTraits::Zero();
        if ( a.max.GetComponent( axis ) < b.min.GetComponent( axis ) )
            gap = b.min.GetComponent( axis ) - a.max.GetComponent( axis );
        else if ( b.max.GetComponent( axis ) < a.min.GetComponent( axis ) )
            gap = a.min.GetComponent( axis ) - b.max.GetComponent( axis );
        sum = sum + gap * gap;
    }
    return sum;
}

/*-------------------------------------------------------------------------*/

/** True when a triangle of the mesh shares a point with the bounded convex shape */
template < typename T, typename TShape >
GUCEF_GEOMETRY_FORCE_INLINE bool
MeshIntersects( const CTMeshBvh< T >& mesh, const TShape& shape )
{
    return mesh.Overlaps( shape );
}

/*-------------------------------------------------------------------------*/

/** The nearest points of the mesh surface and the bounded convex shape; 'triangle' (optional) receives the triangle */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
MeshClosestPoints( const CTMeshBvh< T >& mesh, const TShape& shape, MATH::CTPoint3D< T >& onMesh, MATH::CTPoint3D< T >& onShape, UInt32* triangle )
{
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef typename CTMeshBvh< T >::SNode TNode;
    if ( 0 == mesh.GetNodeCount() || !IsQueryableConvex( shape ) )
        return false;
    const CTTriangleMesh< T >& triangles = *mesh.GetMesh();
    const CTAxisAlignedBox3D< T > shapeBounds = shape.GetBounds();

    typedef SMeshBvhStackEntry< T > SEntry;
    SEntry local[ 64 ];
    GUCEF::vector< SEntry > heap;
    SEntry* stack = local;
    if ( mesh.GetDepth() + 2 > 64 )
    {
        heap.resize( mesh.GetDepth() + 2 );
        stack = &heap[ 0 ];
    }
    UInt32 count = 0;
    stack[ count ].node = 0;
    stack[ count ].distance = MeshBoxSquaredDistance( mesh.GetNode( 0 ).bounds, shapeBounds );
    ++count;

    bool found = false;
    T best = TTraits::Max();
    const T slack = TTraits::One() + TTraits::Epsilon() * TTraits::FromInt32( 16 );
    while ( 0 < count )
    {
        const SEntry current = stack[ --count ];
        if ( found && best * slack < current.distance )
            continue;
        const TNode& node = mesh.GetNode( current.node );
        if ( 0 < node.count )
        {
            for ( UInt32 i=0; i<node.count; ++i )
            {
                const UInt32 t = mesh.GetPrimitive( node.offset + i );
                MATH::CTPoint3D< T > a;
                MATH::CTPoint3D< T > b;
                if ( !ClosestPoints( triangles.GetTriangle( t ), shape, a, b ) )
                    continue;
                const T squared = ( b - a ).GetSquaredLength();
                if ( !found || squared < best )
                {
                    found = true;
                    best = squared;
                    onMesh = a;
                    onShape = b;
                    if ( GUCEF_NULL != triangle )
                        *triangle = t;
                }
            }
            continue;
        }
        const UInt32 first = current.node + 1;
        const UInt32 second = node.offset;
        const T firstDistance = MeshBoxSquaredDistance( mesh.GetNode( first ).bounds, shapeBounds );
        const T secondDistance = MeshBoxSquaredDistance( mesh.GetNode( second ).bounds, shapeBounds );
        /* the nearer child is popped first */
        if ( secondDistance < firstDistance )
        {
            stack[ count ].node = first;
            stack[ count++ ].distance = firstDistance;
            stack[ count ].node = second;
            stack[ count++ ].distance = secondDistance;
        }
        else
        {
            stack[ count ].node = second;
            stack[ count++ ].distance = secondDistance;
            stack[ count ].node = first;
            stack[ count++ ].distance = firstDistance;
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

/** The visitor of MeshComputeContact */
template < typename T, typename TShape >
struct SMeshContactVisitor
{
    const CTTriangleMesh< T >* triangles;
    const TShape* shape;
    CTContactManifold< T >* deepest;
    GUCEF::vector< SMeshTriangleContact< T > >* all;
    bool found;
    T depth;

    bool Visit( UInt32 triangle )
    {
        SMeshTriangleContact< T > contact;
        contact.triangle = triangle;
        if ( !ComputeContact( triangles->GetTriangle( triangle ), *shape, contact.manifold ) || !contact.manifold.IsTouching() )
            return true;
        const T candidate = contact.manifold.GetMaxDepth();
        if ( !found || depth < candidate )
        {
            found = true;
            depth = candidate;
            *deepest = contact.manifold;
        }
        if ( GUCEF_NULL != all )
            all->push_back( contact );
        return true;
    }
};

/** The contacts of the mesh triangles with the bounded convex shape (see the header comment) */
template < typename T, typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
MeshComputeContact( const CTMeshBvh< T >& mesh, const TShape& shape, CTContactManifold< T >& deepest, GUCEF::vector< SMeshTriangleContact< T > >* all )
{
    deepest.Clear();
    if ( 0 == mesh.GetNodeCount() || !IsQueryableConvex( shape ) )
        return false;
    SMeshContactVisitor< T, TShape > visitor;
    visitor.triangles = mesh.GetMesh();
    visitor.shape = &shape;
    visitor.deepest = &deepest;
    visitor.all = all;
    visitor.found = false;
    visitor.depth = MATH::CTNumericTraits< T >::Zero();
    mesh.QueryBox( shape.GetBounds(), visitor );
    return visitor.found;
}

/*-------------------------------------------------------------------------*/

/** The visitor of the mesh sweeps: the nearest hit over the candidate triangles */
template < typename T, typename TCaster >
struct SMeshSweepVisitor
{
    const CTTriangleMesh< T >* triangles;
    const TCaster* caster;
    T maxDistance;
    bool found;
    SRayCastResult< T > best;

    bool Visit( UInt32 triangle )
    {
        SRayCastResult< T > hit;
        if ( !caster->Cast( triangles->GetTriangle( triangle ), maxDistance, hit ) )
            return true;
        if ( !found || hit.distance < best.distance || ( !( best.distance < hit.distance ) && (Int32) triangle < best.featureIndex ) )
        {
            hit.featureIndex = (Int32) triangle;
            best = hit;
            found = true;
        }
        return true;
    }
};

/** Sphere cast of one triangle (the caster of MeshSphereCast) */
template < typename T >
struct SMeshSphereCaster
{
    CTRay3D< T > ray;
    T radius;

    bool Cast( const CTTriangle3D< T >& triangle, const T& maxDistance, SRayCastResult< T >& hit ) const { return SphereCast( ray, radius, triangle, maxDistance, hit ); }
};

/** Shape cast of one triangle (the caster of MeshShapeCast) */
template < typename TShape >
struct SMeshShapeCaster
{
    typedef typename TShape::TValue T;

    const TShape* shape;
    MATH::CTDirectionVector3D< T > direction;

    bool Cast( const CTTriangle3D< T >& triangle, const T& maxDistance, SRayCastResult< T >& hit ) const { return ShapeCast( *shape, direction, maxDistance, triangle, hit ); }
};

/*-------------------------------------------------------------------------*/

/** A sphere of 'radius' moved along the ray by at most maxDistance against the mesh: the nearest hit */
template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
MeshSphereCast( const CTRay3D< T >& ray, const T& radius, const CTMeshBvh< T >& mesh, const T& maxDistance, SRayCastResult< T >& result )
{
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( 0 == mesh.GetNodeCount() || TTraits::IsNegative( radius ) || TTraits::IsNegative( maxDistance ) )
        return false;
    /* the swept bounds: the sphere at the start and at the end of the cast, clipped to the mesh bounds for long casts */
    const CTAxisAlignedBox3D< T > meshBounds = mesh.GetBounds().Expanded( radius );
    T length = maxDistance;
    const T reach = TTraits::Sqrt( MeshBoxSquaredDistance( CTAxisAlignedBox3D< T >( ray.origin, ray.origin ), meshBounds ) ) + ( meshBounds.max - meshBounds.min ).GetLength();
    if ( reach < length )
        length = reach;
    const MATH::CTPoint3D< T > end = ray.origin + ray.direction.AsVector() * length;
    const CTAxisAlignedBox3D< T > swept = CTAxisAlignedBox3D< T >( ray.origin.Min( end ), ray.origin.Max( end ) ).Expanded( radius );

    SMeshSphereCaster< T > caster;
    caster.ray = ray;
    caster.radius = radius;
    SMeshSweepVisitor< T, SMeshSphereCaster< T > > visitor;
    visitor.triangles = mesh.GetMesh();
    visitor.caster = &caster;
    visitor.maxDistance = maxDistance;
    visitor.found = false;
    mesh.QueryBox( swept, visitor );
    if ( visitor.found )
        result = visitor.best;
    return visitor.found;
}

/*-------------------------------------------------------------------------*/

/** A bounded convex shape moved along the unit direction by at most maxDistance against the mesh: the nearest hit */
template < typename TShape >
GUCEF_GEOMETRY_PRECISE_INLINE bool
MeshShapeCast( const TShape& shape, const MATH::CTDirectionVector3D< typename TShape::TValue >& direction, const typename TShape::TValue& maxDistance,
               const CTMeshBvh< typename TShape::TValue >& mesh, SRayCastResult< typename TShape::TValue >& result )
{
    typedef typename TShape::TValue T;
    typedef MATH::CTNumericTraits< T > TTraits;
    if ( 0 == mesh.GetNodeCount() || !IsQueryableConvex( shape ) || TTraits::IsNegative( maxDistance ) )
        return false;
    const CTAxisAlignedBox3D< T > start = shape.GetBounds();
    const CTAxisAlignedBox3D< T > meshBounds = mesh.GetBounds();
    T length = maxDistance;
    const T reach = TTraits::Sqrt( MeshBoxSquaredDistance( start, meshBounds ) ) + ( meshBounds.max - meshBounds.min ).GetLength() + ( start.max - start.min ).GetLength();
    if ( reach < length )
        length = reach;
    const MATH::CTVector3D< T > move = direction.AsVector() * length;
    const CTAxisAlignedBox3D< T > swept = start.Merged( CTAxisAlignedBox3D< T >( start.min + move, start.max + move ) );

    SMeshShapeCaster< TShape > caster;
    caster.shape = &shape;
    caster.direction = direction;
    SMeshSweepVisitor< T, SMeshShapeCaster< TShape > > visitor;
    visitor.triangles = mesh.GetMesh();
    visitor.caster = &caster;
    visitor.maxDistance = maxDistance;
    visitor.found = false;
    mesh.QueryBox( swept, visitor );
    if ( visitor.found )
        result = visitor.best;
    return visitor.found;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_MESHQUERIES_H ? */
