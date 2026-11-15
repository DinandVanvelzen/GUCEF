/*
 *  gucefGEOMETRY_TestApp: GUCEF test application for the Geometry module
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

#ifndef GUCEF_GEOMETRY_TESTAPP_TESTMESHREFERENCE_H
#define GUCEF_GEOMETRY_TESTAPP_TESTMESHREFERENCE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#include "gucefGEOMETRY_CTTriangleMesh.h"
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */

#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#include "gucefGEOMETRY_CTMeshBvh.h"
#define GUCEF_GEOMETRY_CTMESHBVH_H
#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */

#include "TestQueryReference.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Meshes and brute-force references of the mesh, BVH and ray caster tests: a tessellated box, a UV sphere and a random
 *  triangle soup, and the scans over all triangles that the hierarchy must reproduce.
 */
namespace MeshReference {

/** The Float64 value of a point of the kind T */
template < typename T >
QueryReference::P3
ToP64( const GUCEF::MATH::CTPoint3D< T >& p )
{
    return QueryReference::P3( (GUCEF::Float64) p.x, (GUCEF::Float64) p.y, (GUCEF::Float64) p.z );
}

/*-------------------------------------------------------------------------*/

/** A BVH box query visitor that collects the triangles, stopping after 'stopAfter' of them */
struct SCollectTriangles
{
    GUCEF::vector< GUCEF::UInt32 >* triangles;
    GUCEF::UInt32 stopAfter;

    bool Visit( GUCEF::UInt32 triangle )
    {
        triangles->push_back( triangle );
        return (GUCEF::UInt32) triangles->size() < stopAfter;
    }
};

/*-------------------------------------------------------------------------*/

/**
 *  A closed box from 'lo' to 'hi' with every face split into n x n quads of two triangles, counter-clockwise from
 *  outside; the faces are built separately and welded with tolerance 0 (the shared edge points are computed by the same
 *  expressions, so they are equal)
 */
template < typename T >
void
MakeBox( GUCEF::GEOMETRY::CTTriangleMesh< T >& mesh, const GUCEF::MATH::CTPoint3D< T >& lo, const GUCEF::MATH::CTPoint3D< T >& hi, GUCEF::UInt32 n )
{
    typedef GUCEF::MATH::CTPoint3D< T > TPoint;
    mesh.Clear();
    /* each face: the fixed axis, its side, and the two running axes in the order that makes u x v the outward normal */
    const GUCEF::UInt32 faces[ 6 ][ 4 ] = { { 0, 1, 1, 2 }, { 0, 0, 2, 1 }, { 1, 1, 2, 0 }, { 1, 0, 0, 2 }, { 2, 1, 0, 1 }, { 2, 0, 1, 0 } };
    for ( GUCEF::UInt32 f=0; f<6; ++f )
    {
        const GUCEF::UInt32 fixed = faces[ f ][ 0 ];
        const bool high = 1 == faces[ f ][ 1 ];
        const GUCEF::UInt32 ua = faces[ f ][ 2 ];
        const GUCEF::UInt32 va = faces[ f ][ 3 ];
        const GUCEF::UInt32 base = mesh.GetVertexCount();
        for ( GUCEF::UInt32 j=0; j<=n; ++j )
        {
            for ( GUCEF::UInt32 i=0; i<=n; ++i )
            {
                TPoint p;
                p.SetComponent( fixed, high ? hi.GetComponent( fixed ) : lo.GetComponent( fixed ) );
                p.SetComponent( ua, lo.GetComponent( ua ) + ( hi.GetComponent( ua ) - lo.GetComponent( ua ) ) * (T) i / (T) n );
                p.SetComponent( va, lo.GetComponent( va ) + ( hi.GetComponent( va ) - lo.GetComponent( va ) ) * (T) j / (T) n );
                mesh.AddVertex( p );
            }
        }
        for ( GUCEF::UInt32 j=0; j<n; ++j )
        {
            for ( GUCEF::UInt32 i=0; i<n; ++i )
            {
                const GUCEF::UInt32 a = base + j * ( n + 1 ) + i;
                const GUCEF::UInt32 b = a + 1;
                const GUCEF::UInt32 c = a + n + 1;
                const GUCEF::UInt32 d = c + 1;
                mesh.AddTriangle( a, b, d );
                mesh.AddTriangle( a, d, c );
            }
        }
    }
    mesh.Weld( (T) 0 );
}

/*-------------------------------------------------------------------------*/

/** A closed UV sphere: two poles, rings - 1 rings of 'segments' vertices, counter-clockwise from outside */
template < typename T >
void
MakeSphere( GUCEF::GEOMETRY::CTTriangleMesh< T >& mesh, const GUCEF::MATH::CTPoint3D< T >& centre, GUCEF::Float64 radius, GUCEF::UInt32 rings, GUCEF::UInt32 segments )
{
    typedef GUCEF::MATH::CTPoint3D< T > TPoint;
    const GUCEF::Float64 pi = 3.141592653589793;
    mesh.Clear();
    mesh.AddVertex( TPoint( centre.x, centre.y, (T) ( (GUCEF::Float64) centre.z + radius ) ) );
    for ( GUCEF::UInt32 r=1; r<rings; ++r )
    {
        const GUCEF::Float64 polar = pi * (GUCEF::Float64) r / (GUCEF::Float64) rings;
        for ( GUCEF::UInt32 s=0; s<segments; ++s )
        {
            const GUCEF::Float64 azimuth = 2.0 * pi * (GUCEF::Float64) s / (GUCEF::Float64) segments;
            const GUCEF::Float64 x = radius * sin( polar ) * cos( azimuth );
            const GUCEF::Float64 y = radius * sin( polar ) * sin( azimuth );
            const GUCEF::Float64 z = radius * cos( polar );
            mesh.AddVertex( TPoint( (T) ( centre.x + x ), (T) ( centre.y + y ), (T) ( centre.z + z ) ) );
        }
    }
    const GUCEF::UInt32 south = mesh.AddVertex( TPoint( centre.x, centre.y, (T) ( (GUCEF::Float64) centre.z - radius ) ) );
    for ( GUCEF::UInt32 s=0; s<segments; ++s )
    {
        const GUCEF::UInt32 next = ( s + 1 ) % segments;
        mesh.AddTriangle( 0, 1 + s, 1 + next );
        for ( GUCEF::UInt32 r=1; r+1<rings; ++r )
        {
            const GUCEF::UInt32 a = 1 + ( r - 1 ) * segments + s;
            const GUCEF::UInt32 b = 1 + ( r - 1 ) * segments + next;
            const GUCEF::UInt32 c = 1 + r * segments + s;
            const GUCEF::UInt32 d = 1 + r * segments + next;
            mesh.AddTriangle( a, c, d );
            mesh.AddTriangle( a, d, b );
        }
        mesh.AddTriangle( south, 1 + ( rings - 2 ) * segments + next, 1 + ( rings - 2 ) * segments + s );
    }
}

/*-------------------------------------------------------------------------*/

/** 'count' random triangles in the cube of half size 'extent', edges up to 'size' */
template < typename T >
void
MakeSoup( GUCEF::GEOMETRY::CTTriangleMesh< T >& mesh, QueryReference::CRandom& random, GUCEF::UInt32 count, GUCEF::Float64 extent, GUCEF::Float64 size )
{
    mesh.Clear();
    for ( GUCEF::UInt32 t=0; t<count; ++t )
    {
        const QueryReference::P3 centre = random.Point( extent );
        GUCEF::UInt32 first = 0;
        for ( GUCEF::UInt32 k=0; k<3; ++k )
        {
            const QueryReference::V3 offset = random.Vector( size );
            const GUCEF::UInt32 index = mesh.AddVertex( QueryReference::ToP< T >( centre + offset ) );
            if ( 0 == k )
                first = index;
        }
        mesh.AddTriangle( first, first + 1, first + 2 );
    }
}

/*-------------------------------------------------------------------------*/

/** The nearest hit over all triangles in index order (a later triangle wins only when strictly nearer) */
template < typename T >
bool
BruteRayCast( const GUCEF::GEOMETRY::CTTriangleMesh< T >& mesh, const GUCEF::GEOMETRY::CTRay3D< T >& ray, const T& maxDistance, GUCEF::GEOMETRY::SRayCastResult< T >& best )
{
    bool found = false;
    const GUCEF::UInt32 count = mesh.GetTriangleCount();
    for ( GUCEF::UInt32 t=0; t<count; ++t )
    {
        GUCEF::GEOMETRY::SRayCastResult< T > hit;
        if ( GUCEF::GEOMETRY::RayCastTriangleWatertight( ray, mesh.GetTriangle( t ), maxDistance, hit ) && ( !found || hit.distance < best.distance ) )
        {
            hit.featureIndex = (GUCEF::Int32) t;
            best = hit;
            found = true;
        }
    }
    return found;
}

/*-------------------------------------------------------------------------*/

/** Every hit over all triangles: ( triangle, distance ) pairs in triangle order */
template < typename T >
void
BruteRayCastAll( const GUCEF::GEOMETRY::CTTriangleMesh< T >& mesh, const GUCEF::GEOMETRY::CTRay3D< T >& ray, const T& maxDistance, GUCEF::vector< GUCEF::GEOMETRY::SRayCastResult< T > >& hits )
{
    hits.clear();
    const GUCEF::UInt32 count = mesh.GetTriangleCount();
    for ( GUCEF::UInt32 t=0; t<count; ++t )
    {
        GUCEF::GEOMETRY::SRayCastResult< T > hit;
        if ( GUCEF::GEOMETRY::RayCastTriangleWatertight( ray, mesh.GetTriangle( t ), maxDistance, hit ) )
        {
            hit.featureIndex = (GUCEF::Int32) t;
            hits.push_back( hit );
        }
    }
}

/*-------------------------------------------------------------------------*/

/** The smallest squared distance to a triangle, and the first triangle that has it */
template < typename T >
T
BruteClosest( const GUCEF::GEOMETRY::CTTriangleMesh< T >& mesh, const GUCEF::MATH::CTPoint3D< T >& point, GUCEF::UInt32& triangle )
{
    T best = GUCEF::MATH::CTNumericTraits< T >::Max();
    triangle = 0;
    const GUCEF::UInt32 count = mesh.GetTriangleCount();
    for ( GUCEF::UInt32 t=0; t<count; ++t )
    {
        const T squared = ( mesh.GetTriangle( t ).ClosestPoint( point ) - point ).GetSquaredLength();
        if ( squared < best )
        {
            best = squared;
            triangle = t;
        }
    }
    return best;
}

}  /* namespace MeshReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_TESTAPP_TESTMESHREFERENCE_H ? */
