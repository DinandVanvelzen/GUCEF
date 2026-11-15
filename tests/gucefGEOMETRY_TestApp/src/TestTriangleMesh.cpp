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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <math.h>
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_TEST_FRAMEWORK_H
#include "gucef_test_framework.h"
#define GUCEF_TEST_FRAMEWORK_H
#endif /* GUCEF_TEST_FRAMEWORK_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#include "gucefGEOMETRY_CTTriangleMesh.h"
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */

#include "TestMeshReference.h"
#include "TestTriangleMesh.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      MACROS                                                             //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ERRORHERE       GUCEF_TESTFW_ERRORHERE
#define ASSERT_TRUE(t)  GUCEF_TESTFW_ASSERT_TRUE(t)
#define ASSERT_FALSE(t) GUCEF_TESTFW_ASSERT_FALSE(t)

using namespace GUCEF;
using namespace GUCEF::MATH;
using namespace GUCEF::GEOMETRY;
using namespace MeshReference;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
static bool
Near( T a, T b, Float64 tolerance )
{
    return fabs( (Float64) a - (Float64) b ) <= tolerance;
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
NearPoint( const CTPoint3D< T >& a, Float64 x, Float64 y, Float64 z, Float64 tolerance )
{
    return Near( a.x, (T) x, tolerance ) && Near( a.y, (T) y, tolerance ) && Near( a.z, (T) z, tolerance );
}

/*-------------------------------------------------------------------------*/

/** Area, volume, centroid, closedness and corner normals of the tessellated unit cube [ 0, 1 ]^3 */
template < typename T >
static bool
CubeProperties( const char* name, UInt32 n, Float64 tolerance )
{
    CTTriangleMesh< T > cube;
    MakeBox( cube, CTPoint3D< T >( 0, 0, 0 ), CTPoint3D< T >( 1, 1, 1 ), n );
    bool ok = true;
    const UInt32 expectedVertices = 6 * ( n + 1 ) * ( n + 1 ) - 12 * ( n + 1 ) + 8;
    if ( cube.GetVertexCount() != expectedVertices || cube.GetTriangleCount() != 12 * n * n || !cube.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": " << cube.GetVertexCount() << " vertices (expected " << expectedVertices << "), " << cube.GetTriangleCount() << " triangles" );
        ok = false;
    }
    if ( !Near( cube.GetSurfaceArea(), (T) 6, tolerance ) || !Near( cube.GetVolume(), (T) 1, tolerance ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": area " << cube.GetSurfaceArea() << " volume " << cube.GetVolume() );
        ok = false;
    }
    if ( !NearPoint( cube.GetCentroid(), 0.5, 0.5, 0.5, tolerance ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": centroid " << cube.GetCentroid().ToString() );
        ok = false;
    }
    if ( !cube.IsClosed() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": the cube is not closed" );
        ok = false;
    }

    /* corner normals: the normalised diagonals; face normals: the axes */
    GUCEF::vector< CTVector3D< T > > normals;
    cube.ComputeVertexNormals( normals );
    const Float64 r = 1.0 / sqrt( 3.0 );
    UInt32 corners = 0;
    for ( UInt32 v=0; v<cube.GetVertexCount(); ++v )
    {
        const CTPoint3D< T >& p = cube.vertices[ v ];
        const bool corner = ( 0 == p.x || 1 == p.x ) && ( 0 == p.y || 1 == p.y ) && ( 0 == p.z || 1 == p.z );
        if ( !corner )
            continue;
        ++corners;
        const Float64 sx = 0 == p.x ? -r : r;
        const Float64 sy = 0 == p.y ? -r : r;
        const Float64 sz = 0 == p.z ? -r : r;
        if ( !Near( normals[ v ].x, (T) sx, tolerance ) || !Near( normals[ v ].y, (T) sy, tolerance ) || !Near( normals[ v ].z, (T) sz, tolerance ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": corner normal " << normals[ v ].ToString() << " at " << p.ToString() );
            ok = false;
        }
    }
    if ( 8 != corners )
        ok = false;
    GUCEF::vector< CTVector3D< T > > faceNormals;
    cube.ComputeFaceNormals( faceNormals );
    for ( UInt32 t=0; t<cube.GetTriangleCount(); ++t )
    {
        /* outward: the normal points from the centre to the triangle */
        const CTVector3D< T > out = cube.GetTriangle( t ).GetCentroid() - CTPoint3D< T >( (T) 0.5, (T) 0.5, (T) 0.5 );
        if ( !Near( faceNormals[ t ].GetLength(), (T) 1, tolerance ) || !( (T) 0 < faceNormals[ t ].Dot( out ) ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": face normal " << faceNormals[ t ].ToString() << " of triangle " << t );
            ok = false;
            break;
        }
    }

    /* removing a triangle opens the cube; reversing one triangle breaks the orientation */
    CTTriangleMesh< T > open = cube;
    open.indices.resize( open.indices.size() - 3 );
    CTTriangleMesh< T > flipped = cube;
    const UInt32 swap = flipped.indices[ 1 ];
    flipped.indices[ 1 ] = flipped.indices[ 2 ];
    flipped.indices[ 2 ] = swap;
    if ( open.IsClosed() || flipped.IsClosed() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": an open or inconsistently oriented cube reports closed" );
        ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** A sphere of radius 2 at ( 1, -2, 3 ): the area and volume converge to the sphere, the centroid is the centre */
template < typename T >
static bool
SphereProperties( const char* name, Float64 tolerance )
{
    CTTriangleMesh< T > sphere;
    MakeSphere( sphere, CTPoint3D< T >( 1, -2, 3 ), 2.0, 64, 96 );
    bool ok = sphere.IsClosed() && sphere.IsValid();
    const Float64 pi = 3.141592653589793;
    /* an inscribed polyhedron: a little below the sphere, within 0.5 percent at this resolution */
    const Float64 area = (Float64) sphere.GetSurfaceArea();
    const Float64 volume = (Float64) sphere.GetVolume();
    if ( !( area < 16.0 * pi && area > 16.0 * pi * 0.995 ) || !( volume < 32.0 / 3.0 * pi && volume > 32.0 / 3.0 * pi * 0.995 ) )
        ok = false;
    if ( !NearPoint( sphere.GetCentroid(), 1, -2, 3, tolerance ) )
        ok = false;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": sphere area " << area << " volume " << volume << " centroid " << sphere.GetCentroid().ToString() << " closed " << sphere.IsClosed() );
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Welding: a triangle soup copy of the cube welds back to the indexed cube; collapsed triangles are dropped */
template < typename T >
static bool
WeldCases( const char* name )
{
    bool ok = true;
    CTTriangleMesh< T > cube;
    MakeBox( cube, CTPoint3D< T >( -1, -1, -1 ), CTPoint3D< T >( 1, 1, 1 ), 3 );

    CTTriangleMesh< T > soup;
    for ( UInt32 t=0; t<cube.GetTriangleCount(); ++t )
    {
        const CTTriangle3D< T > triangle = cube.GetTriangle( t );
        const UInt32 a = soup.AddVertex( triangle.a );
        const UInt32 b = soup.AddVertex( triangle.b );
        const UInt32 c = soup.AddVertex( triangle.c );
        soup.AddTriangle( a, b, c );
    }
    if ( soup.IsClosed() )
        ok = false;
    const UInt32 before = soup.GetVertexCount();
    const UInt32 removed = soup.Weld( (T) 1e-4 );
    if ( removed != before - cube.GetVertexCount() || soup.GetVertexCount() != cube.GetVertexCount() || !soup.IsClosed() || soup.GetTriangleCount() != cube.GetTriangleCount() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": weld removed " << removed << " of " << before << ", " << soup.GetVertexCount() << " left, closed " << soup.IsClosed() );
        ok = false;
    }

    /* jittered duplicates within the tolerance merge, a far vertex stays; a sliver collapses and is removed */
    CTTriangleMesh< T > mesh;
    mesh.AddVertex( CTPoint3D< T >( 0, 0, 0 ) );
    mesh.AddVertex( CTPoint3D< T >( 1, 0, 0 ) );
    mesh.AddVertex( CTPoint3D< T >( 0, 1, 0 ) );
    mesh.AddVertex( CTPoint3D< T >( (T) 1.00001, (T) 0.00001, 0 ) );
    mesh.AddVertex( CTPoint3D< T >( 5, 5, 5 ) );
    mesh.AddTriangle( 0, 1, 2 );
    mesh.AddTriangle( 0, 3, 4 );
    mesh.AddTriangle( 1, 3, 4 );
    const UInt32 merged = mesh.Weld( (T) 1e-3 );
    if ( 1 != merged || 4 != mesh.GetVertexCount() || 2 != mesh.GetTriangleCount() || !mesh.IsValid() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": jitter weld merged " << merged << ", " << mesh.GetVertexCount() << " vertices, " << mesh.GetTriangleCount() << " triangles" );
        ok = false;
    }
    /* the kept vertex is the first one, the order of the kept vertices is preserved */
    if ( !( mesh.vertices[ 1 ] == CTPoint3D< T >( 1, 0, 0 ) ) || !( mesh.vertices[ 3 ] == CTPoint3D< T >( 5, 5, 5 ) ) )
        ok = false;

    CTTriangleMesh< T > empty;
    if ( 0 != empty.Weld( (T) 1 ) || !empty.IsEmpty() || empty.IsClosed() )
        ok = false;
    return ok;
}

/*-------------------------------------------------------------------------*/

/**
 *  The reference of Weld, independent of its sweep: every pair of vertices within the tolerance linked (all pairs),
 *  the groups found by a flood fill, each group kept at its lowest index; 'expected' receives the welded mesh
 */
template < typename T >
static void
ReferenceWeld( const CTTriangleMesh< T >& mesh, Float64 tolerance, CTTriangleMesh< T >& expected )
{
    const UInt32 count = mesh.GetVertexCount();
    GUCEF::vector< UInt32 > group( count, 0xFFFFFFFF );
    GUCEF::vector< UInt32 > pending;
    for ( UInt32 seed=0; seed<count; ++seed )
    {
        if ( 0xFFFFFFFF != group[ seed ] )
            continue;
        /* seeds are visited in index order, so the seed is the lowest index of its group */
        group[ seed ] = seed;
        pending.push_back( seed );
        while ( !pending.empty() )
        {
            const UInt32 v = pending.back();
            pending.pop_back();
            for ( UInt32 u=0; u<count; ++u )
            {
                if ( 0xFFFFFFFF != group[ u ] )
                    continue;
                const Float64 dx = (Float64) mesh.vertices[ u ].x - (Float64) mesh.vertices[ v ].x;
                const Float64 dy = (Float64) mesh.vertices[ u ].y - (Float64) mesh.vertices[ v ].y;
                const Float64 dz = (Float64) mesh.vertices[ u ].z - (Float64) mesh.vertices[ v ].z;
                if ( dx * dx + dy * dy + dz * dz <= tolerance * tolerance )
                {
                    group[ u ] = seed;
                    pending.push_back( u );
                }
            }
        }
    }
    GUCEF::vector< UInt32 > newIndex( count, 0xFFFFFFFF );
    expected = CTTriangleMesh< T >();
    for ( UInt32 v=0; v<count; ++v )
        if ( group[ v ] == v )
            newIndex[ v ] = expected.AddVertex( mesh.vertices[ v ] );
    for ( UInt32 t=0; t<mesh.GetTriangleCount(); ++t )
    {
        const UInt32 a = newIndex[ group[ mesh.indices[ 3 * t ] ] ];
        const UInt32 b = newIndex[ group[ mesh.indices[ 3 * t + 1 ] ] ];
        const UInt32 c = newIndex[ group[ mesh.indices[ 3 * t + 2 ] ] ];
        if ( a != b && b != c && c != a )
            expected.AddTriangle( a, b, c );
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
static bool
SameMesh( const CTTriangleMesh< T >& a, const CTTriangleMesh< T >& b )
{
    if ( a.vertices.size() != b.vertices.size() || a.indices != b.indices )
        return false;
    for ( size_t v=0; v<a.vertices.size(); ++v )
        if ( !( a.vertices[ v ] == b.vertices[ v ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Weld groups do not depend on the vertex order: a chain of three vertices 0.9 apart (tolerance 1) welds into one
 *  vertex in both orders along x; seeded clouds on a 1/4 grid (tolerance 0.55: no pair at exactly the tolerance) weld
 *  as the all-pairs reference, also after a random permutation of the vertices and after mirroring x
 */
template < typename T >
static bool
WeldGroups( const char* name, UInt64 seed, UInt32 meshes )
{
    bool ok = true;
    for ( UInt32 mirrored=0; mirrored<2; ++mirrored )
    {
        const T sign = mirrored ? (T) -1 : (T) 1;
        CTTriangleMesh< T > chain;
        chain.AddVertex( CTPoint3D< T >( 0, 0, 0 ) );
        chain.AddVertex( CTPoint3D< T >( sign * (T) 0.9, 0, 0 ) );
        chain.AddVertex( CTPoint3D< T >( sign * (T) 1.8, 0, 0 ) );
        chain.AddVertex( CTPoint3D< T >( 0, 5, 0 ) );
        chain.AddVertex( CTPoint3D< T >( 0, 0, 5 ) );
        chain.AddTriangle( 2, 3, 4 );
        const UInt32 removed = chain.Weld( (T) 1 );
        if ( 2 != removed || 3 != chain.GetVertexCount() || 1 != chain.GetTriangleCount() || 0 != chain.indices[ 0 ] )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": chain (mirrored " << mirrored << ") removed " << removed << ", " << chain.GetVertexCount() << " vertices left" );
            ok = false;
        }
    }

    QueryReference::CRandom random( seed );
    for ( UInt32 m=0; m<meshes && ok; ++m )
    {
        const UInt32 count = 20 + (UInt32) random.Index( 200 );
        CTTriangleMesh< T > cloud;
        for ( UInt32 v=0; v<count; ++v )
        {
            const Int32 x = random.Index( 41 ) - 20;
            const Int32 y = random.Index( 41 ) - 20;
            const Int32 z = random.Index( 9 ) - 4;
            cloud.AddVertex( CTPoint3D< T >( (T) x / (T) 4, (T) y / (T) 4, (T) z / (T) 4 ) );
        }
        for ( UInt32 t=0; t<count; ++t )
        {
            const UInt32 a = (UInt32) random.Index( (Int32) count );
            const UInt32 b = (UInt32) random.Index( (Int32) count );
            const UInt32 c = (UInt32) random.Index( (Int32) count );
            if ( a != b && b != c && c != a )
                cloud.AddTriangle( a, b, c );
        }

        /* the same cloud permuted, and mirrored in x */
        GUCEF::vector< UInt32 > permutation( count );
        for ( UInt32 v=0; v<count; ++v )
            permutation[ v ] = v;
        for ( UInt32 v=count; v>1; --v )
        {
            const UInt32 other = (UInt32) random.Index( (Int32) v );
            std::swap( permutation[ v - 1 ], permutation[ other ] );
        }
        CTTriangleMesh< T > permuted;
        CTTriangleMesh< T > mirror;
        GUCEF::vector< UInt32 > position( count );
        for ( UInt32 v=0; v<count; ++v )
        {
            position[ permutation[ v ] ] = v;
            permuted.AddVertex( cloud.vertices[ permutation[ v ] ] );
        }
        for ( UInt32 v=0; v<count; ++v )
        {
            const CTPoint3D< T >& p = cloud.vertices[ v ];
            mirror.AddVertex( CTPoint3D< T >( -p.x, p.y, p.z ) );
        }
        for ( UInt32 t=0; t<cloud.GetTriangleCount(); ++t )
        {
            permuted.AddTriangle( position[ cloud.indices[ 3 * t ] ], position[ cloud.indices[ 3 * t + 1 ] ], position[ cloud.indices[ 3 * t + 2 ] ] );
            mirror.AddTriangle( cloud.indices[ 3 * t ], cloud.indices[ 3 * t + 1 ], cloud.indices[ 3 * t + 2 ] );
        }

        CTTriangleMesh< T >* inputs[ 3 ] = { &cloud, &permuted, &mirror };
        UInt32 removedCounts[ 3 ] = { 0, 0, 0 };
        for ( UInt32 i=0; i<3; ++i )
        {
            CTTriangleMesh< T > expected;
            ReferenceWeld( *inputs[ i ], 0.55, expected );
            removedCounts[ i ] = inputs[ i ]->Weld( (T) 0.55 );
            if ( !SameMesh( *inputs[ i ], expected ) || removedCounts[ i ] != count - expected.GetVertexCount() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": cloud " << m << " input " << i << " welded to " << inputs[ i ]->GetVertexCount() << " vertices, the reference to " << expected.GetVertexCount() );
                ok = false;
            }
        }
        /* the groups are the same whatever the order: the same number of vertices is removed */
        if ( removedCounts[ 0 ] != removedCounts[ 1 ] || removedCounts[ 0 ] != removedCounts[ 2 ] )
            ok = false;
    }
    return ok;
}

/*-------------------------------------------------------------------------*/

/** Construction, validation, bounds, corner angles, transforms (a mirror reverses the winding) */
template < typename T >
static bool
BasicMembers( const char* name, Float64 tolerance )
{
    bool ok = true;
    const CTPoint3D< T > points[ 3 ] = { CTPoint3D< T >( 0, 0, 0 ), CTPoint3D< T >( 2, 0, 0 ), CTPoint3D< T >( 0, 3, 1 ) };
    const UInt32 indices[ 3 ] = { 0, 1, 2 };
    CTTriangleMesh< T > mesh( points, 3, indices, 3 );
    if ( 3 != mesh.GetVertexCount() || 1 != mesh.GetTriangleCount() || !mesh.IsValid() || mesh.IsEmpty() )
        ok = false;
    const CTAxisAlignedBox3D< T > bounds = mesh.GetBounds();
    if ( !( bounds.min == CTPoint3D< T >( 0, 0, 0 ) ) || !( bounds.max == CTPoint3D< T >( 2, 3, 1 ) ) )
        ok = false;
    mesh.indices.push_back( 7 );
    if ( mesh.IsValid() )
        ok = false;
    mesh.indices.push_back( 0 );
    mesh.indices.push_back( 1 );
    if ( mesh.IsValid() )
        ok = false;
    mesh.Clear();
    if ( !mesh.IsEmpty() || 0 != mesh.GetVertexCount() || !mesh.GetBounds().IsEmpty() || !mesh.IsValid() )
        ok = false;
    mesh.Reserve( 10, 10 );
    if ( mesh.vertices.capacity() < 10 || mesh.indices.capacity() < 30 )
        ok = false;

    /* corner angles of a right triangle, and zero for a degenerate corner */
    const CTTriangle3D< T > right( CTPoint3D< T >( 0, 0, 0 ), CTPoint3D< T >( 1, 0, 0 ), CTPoint3D< T >( 0, 1, 0 ) );
    if ( !Near( CTTriangleMesh< T >::GetCornerAngle( right, 0 ), (T) 1.5707963267948966, tolerance ) || !Near( CTTriangleMesh< T >::GetCornerAngle( right, 1 ), (T) 0.7853981633974483, tolerance ) )
        ok = false;
    const CTTriangle3D< T > point( CTPoint3D< T >( 1, 1, 1 ), CTPoint3D< T >( 1, 1, 1 ), CTPoint3D< T >( 1, 1, 1 ) );
    if ( (T) 0 != CTTriangleMesh< T >::GetCornerAngle( point, 0 ) )
        ok = false;

    /* a degenerate triangle has a zero face normal; an isolated vertex a zero vertex normal */
    CTTriangleMesh< T > degenerate;
    degenerate.AddVertex( CTPoint3D< T >( 0, 0, 0 ) );
    degenerate.AddVertex( CTPoint3D< T >( 1, 0, 0 ) );
    degenerate.AddVertex( CTPoint3D< T >( 2, 0, 0 ) );
    degenerate.AddVertex( CTPoint3D< T >( 9, 9, 9 ) );
    degenerate.AddTriangle( 0, 1, 2 );
    GUCEF::vector< CTVector3D< T > > normals;
    degenerate.ComputeFaceNormals( normals );
    if ( 1 != normals.size() || !normals[ 0 ].IsZero() )
        ok = false;
    degenerate.ComputeVertexNormals( normals );
    if ( 4 != normals.size() || !normals[ 3 ].IsZero() || !normals[ 0 ].IsZero() )
        ok = false;
    if ( (T) 0 != degenerate.GetSurfaceArea() || (T) 0 != degenerate.GetVolume() )
        ok = false;
    /* no area: the centroid is the mean of the vertices */
    if ( !NearPoint( degenerate.GetCentroid(), 3, 2.25, 2.25, tolerance ) )
        ok = false;

    /* transforms: a scale by 2 scales the volume by 8; a mirror keeps the volume positive and the mesh closed */
    CTTriangleMesh< T > cube;
    MakeBox( cube, CTPoint3D< T >( 0, 0, 0 ), CTPoint3D< T >( 1, 1, 1 ), 2 );
    const CTMatrix4D< T > scale = CTMatrix4D< T >::Scale( CTVector3D< T >( 2, 2, 2 ) );
    const CTMatrix4D< T > mirror = CTMatrix4D< T >::Scale( CTVector3D< T >( -1, 1, 1 ) );
    const CTTriangleMesh< T > scaled = cube.Transformed( scale );
    const CTTriangleMesh< T > mirrored = cube.Transformed( mirror );
    if ( !Near( scaled.GetVolume(), (T) 8, tolerance * 8 ) || !Near( mirrored.GetVolume(), (T) 1, tolerance ) || !mirrored.IsClosed() || !NearPoint( mirrored.GetCentroid(), -0.5, 0.5, 0.5, tolerance ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": transformed volumes " << scaled.GetVolume() << " and " << mirrored.GetVolume() );
        ok = false;
    }

    /* BuildBvh builds over the mesh; an invalid mesh is refused */
    CTMeshBvh< T > bvh;
    if ( !cube.BuildBvh( bvh ) || bvh.GetMesh() != &cube || bvh.GetPrimitiveCount() != cube.GetTriangleCount() )
        ok = false;
    CTTriangleMesh< T > invalid = cube;
    invalid.indices[ 0 ] = 100000;
    if ( invalid.BuildBvh( bvh ) || bvh.IsBuilt() )
        ok = false;
    if ( !ok )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, name << ": basic members failed" );
    return ok;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
PerformTriangleMeshTests( void )
{GUCEF_TRACE;

    GUCEF_TESTFW_SUITE_SCOPE( "TriangleMesh" );

    GUCEF_TESTFW_TESTCASE( "TriangleMesh: tessellated unit cube: area 6 and volume 1 (1e-6), centroid, closed, corner normals are the normalised diagonals" )
    try
    {
        ASSERT_TRUE( CubeProperties< Float64 >( "cube Float64 n=1", 1, 1e-12 ) );
        ASSERT_TRUE( CubeProperties< Float64 >( "cube Float64 n=8", 8, 1e-12 ) );
        ASSERT_TRUE( CubeProperties< Float32 >( "cube Float32 n=8", 8, 1e-6 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TriangleMesh: UV sphere: closed, area and volume of the inscribed polyhedron, centroid at the centre (1e-9 / 1e-5)" )
    try
    {
        ASSERT_TRUE( SphereProperties< Float64 >( "sphere Float64", 1e-9 ) );
        ASSERT_TRUE( SphereProperties< Float32 >( "sphere Float32", 1e-5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TriangleMesh: Weld merges duplicates into the first vertex, keeps the order, drops collapsed triangles; weld groups are order independent (all-pairs reference, permuted, mirrored)" )
    try
    {
        ASSERT_TRUE( WeldCases< Float64 >( "weld Float64" ) );
        ASSERT_TRUE( WeldCases< Float32 >( "weld Float32" ) );
        ASSERT_TRUE( WeldGroups< Float64 >( "weld groups Float64", 30711, 200 ) );
        ASSERT_TRUE( WeldGroups< Float32 >( "weld groups Float32", 30712, 200 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END

    GUCEF_TESTFW_TESTCASE( "TriangleMesh: construction, validity, bounds, corner angles, degenerate normals, transforms and BuildBvh" )
    try
    {
        ASSERT_TRUE( BasicMembers< Float64 >( "basic Float64", 1e-12 ) );
        ASSERT_TRUE( BasicMembers< Float32 >( "basic Float32", 1e-5 ) );
    }
    catch( ... )
    {
        ERRORHERE;
    }
    GUCEF_TESTFW_TESTCASE_END
}

/*-------------------------------------------------------------------------*/
