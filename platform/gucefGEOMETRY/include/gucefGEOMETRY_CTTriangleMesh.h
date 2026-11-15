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

#ifndef GUCEF_GEOMETRY_CTTRIANGLEMESH_H
#define GUCEF_GEOMETRY_CTTRIANGLEMESH_H

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

#ifndef GUCEF_GEOMETRY_CTTRIANGLE3D_H
#include "gucefGEOMETRY_CTTriangle3D.h"
#define GUCEF_GEOMETRY_CTTRIANGLE3D_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLE3D_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

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

template < typename T > class CTMeshBvh;

/*-------------------------------------------------------------------------*/

/**
 *  An indexed triangle mesh (section 7.8.1 of the design): vertex positions and three vertex indices per triangle,
 *  counter-clockwise seen from the front (the face normal is ( b - a ) x ( c - a )).
 *
 *  - Public data: 'vertices' and 'indices'. The mesh is valid (IsValid) when the index count is a multiple of three
 *    and every index is below the vertex count; the members that read triangles require a valid mesh.
 *  - Face normals are unit vectors, the zero vector for a degenerate triangle. Vertex normals are angle weighted (the
 *    face normals of the triangles around the vertex weighted by the corner angle at it), unit length, zero for a vertex
 *    without a non-degenerate triangle.
 *  - GetVolume is the signed sum of the tetrahedra spanned by the origin and each triangle: the enclosed volume for a
 *    closed mesh with outward (counter-clockwise) triangles, negative for inward triangles, meaningless otherwise.
 *  - GetCentroid is the centroid of the enclosed solid when the volume is not zero, otherwise the area-weighted centroid
 *    of the surface, otherwise the mean of the vertices (the origin for an empty mesh).
 *  - IsClosed: every edge is shared by exactly two triangles that use it in opposite directions (a closed, consistently
 *    oriented two-manifold surface); false for an empty mesh.
 *  - Weld( tolerance ): two vertices within 'tolerance' of each other (Euclidean distance) are linked; every group of
 *    vertices connected by links (transitively: a chain of vertices each within the tolerance of the next is one
 *    group, however far its ends are apart) merges into the vertex of lowest index of the group. The groups depend on
 *    the positions only, not on the vertex order, so a permuted or mirrored mesh welds into the same groups. Then the
 *    vertex array is compacted keeping the order of the kept vertices, and triangles that collapse (two equal
 *    indices) are removed. Returns the number of vertices removed.
 *  - Transformed maps every vertex; a mirroring transform (negative determinant) also reverses the winding, so outward
 *    triangles stay outward.
 *  - Kinds: Float32 and Float64 are exported. Not realtime-safe (IsClosed and Weld allocate work arrays); value type,
 *    concurrent const use is safe.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTTriangleMesh
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef CTTriangle3D< T > TTriangle;
    typedef CTAxisAlignedBox3D< T > TBox;

    GUCEF::vector< TPoint > vertices;
    GUCEF::vector< UInt32 > indices;

    CTTriangleMesh( void ) : vertices(), indices() {}

    /** A mesh from 'vertexCount' points and 'indexCount' indices (three per triangle) */
    CTTriangleMesh( const TPoint* vertexArray, UInt32 vertexCount, const UInt32* indexArray, UInt32 indexCount );

    /** Appends a vertex and returns its index */
    UInt32 AddVertex( const TPoint& point );

    /** Appends a triangle of three vertex indices and returns its index */
    UInt32 AddTriangle( UInt32 a, UInt32 b, UInt32 c );

    void Clear( void );

    void Reserve( UInt32 vertexCount, UInt32 triangleCount );

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetVertexCount( void ) const { return (UInt32) vertices.size(); }

    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetTriangleCount( void ) const { return (UInt32) ( indices.size() / 3 ); }

    /** True without triangles */
    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const { return indices.size() < 3; }

    /** True when the index count is a multiple of three and every index names a vertex */
    bool IsValid( void ) const;

    /** Triangle 'index' (below GetTriangleCount) */
    GUCEF_GEOMETRY_FORCE_INLINE TTriangle GetTriangle( UInt32 index ) const
    {
        const UInt32* i = &indices[ 3 * (size_t) index ];
        return TTriangle( vertices[ i[ 0 ] ], vertices[ i[ 1 ] ], vertices[ i[ 2 ] ] );
    }

    /** The unit normal of every triangle (zero for a degenerate one), one per triangle */
    void ComputeFaceNormals( GUCEF::vector< TVector >& normals ) const;

    /** The angle-weighted unit normal of every vertex (zero for a vertex without a non-degenerate triangle) */
    void ComputeVertexNormals( GUCEF::vector< TVector >& normals ) const;

    /** The sum of the triangle areas */
    T GetSurfaceArea( void ) const;

    /** The signed enclosed volume (see the class comment) */
    T GetVolume( void ) const;

    /** The centroid (see the class comment) */
    TPoint GetCentroid( void ) const;

    /** True for a closed, consistently oriented surface (see the class comment) */
    bool IsClosed( void ) const;

    /** Merges vertices within 'tolerance' (see the class comment); returns the number of vertices removed */
    UInt32 Weld( const T& tolerance );

    /** The bounds of the vertices (empty for a mesh without vertices) */
    TBox GetBounds( void ) const;

    /** The mesh with every vertex transformed (see the class comment) */
    CTTriangleMesh Transformed( const MATH::CTMatrix4D< T >& matrix ) const;

    /** Builds a bounding volume hierarchy over the mesh (CTMeshBvh::Build); false for an invalid mesh */
    bool BuildBvh( CTMeshBvh< T >& bvh ) const;

    /** The angle of the triangle corner at vertex 'corner' (0, 1 or 2) in radians, 0 for a degenerate corner */
    static T GetCornerAngle( const TTriangle& triangle, UInt32 corner );

    private:

    /** The root of a vertex in the union-find forest of Weld (path halving); a root is the lowest index of its group */
    static UInt32 FindWeldRoot( GUCEF::vector< UInt32 >& parent, UInt32 vertex );

    /** Orders vertex indices by x, then by index (the sweep of Weld) */
    struct SVertexByX
    {
        const GUCEF::vector< TPoint >* points;
        bool operator()( UInt32 a, UInt32 b ) const
        {
            const T ax = ( *points )[ a ].x;
            const T bx = ( *points )[ b ].x;
            return ax < bx || ( !( bx < ax ) && a < b );
        }
    };
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTTriangleMesh< T >::CTTriangleMesh( const TPoint* vertexArray, UInt32 vertexCount, const UInt32* indexArray, UInt32 indexCount )
    : vertices(), indices()
{
    if ( GUCEF_NULL != vertexArray && 0 < vertexCount )
        vertices.assign( vertexArray, vertexArray + vertexCount );
    if ( GUCEF_NULL != indexArray && 0 < indexCount )
        indices.assign( indexArray, indexArray + indexCount );
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTTriangleMesh< T >::AddVertex( const TPoint& point )
{
    vertices.push_back( point );
    return (UInt32) vertices.size() - 1;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTTriangleMesh< T >::AddTriangle( UInt32 a, UInt32 b, UInt32 c )
{
    indices.push_back( a );
    indices.push_back( b );
    indices.push_back( c );
    return GetTriangleCount() - 1;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTTriangleMesh< T >::Clear( void )
{
    vertices.clear();
    indices.clear();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTTriangleMesh< T >::Reserve( UInt32 vertexCount, UInt32 triangleCount )
{
    vertices.reserve( vertexCount );
    indices.reserve( 3 * (size_t) triangleCount );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTTriangleMesh< T >::IsValid( void ) const
{
    if ( 0 != indices.size() % 3 )
        return false;
    const UInt32 count = (UInt32) vertices.size();
    for ( size_t i=0; i<indices.size(); ++i )
        if ( indices[ i ] >= count )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTTriangleMesh< T >::GetCornerAngle( const TTriangle& triangle, UInt32 corner )
{
    const TPoint& p = triangle.GetVertex( corner );
    const TVector e1 = triangle.GetVertex( ( corner + 1 ) % 3 ) - p;
    const TVector e2 = triangle.GetVertex( ( corner + 2 ) % 3 ) - p;
    const T sine = e1.Cross( e2 ).GetLength();
    const T cosine = e1.Dot( e2 );
    if ( TTraits::Zero() == sine && TTraits::Zero() == cosine )
        return TTraits::Zero();
    return TTraits::Atan2( sine, cosine );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTTriangleMesh< T >::ComputeFaceNormals( GUCEF::vector< TVector >& normals ) const
{
    const UInt32 count = GetTriangleCount();
    normals.assign( count, TVector() );
    for ( UInt32 t=0; t<count; ++t )
    {
        MATH::CTDirectionVector3D< T > normal;
        if ( GetTriangle( t ).TryGetNormal( normal ) )
            normals[ t ] = normal.AsVector();
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE void
CTTriangleMesh< T >::ComputeVertexNormals( GUCEF::vector< TVector >& normals ) const
{
    normals.assign( vertices.size(), TVector() );
    const UInt32 count = GetTriangleCount();
    for ( UInt32 t=0; t<count; ++t )
    {
        const TTriangle triangle = GetTriangle( t );
        MATH::CTDirectionVector3D< T > normal;
        if ( !triangle.TryGetNormal( normal ) )
            continue;
        for ( UInt32 corner=0; corner<3; ++corner )
        {
            TVector& n = normals[ indices[ 3 * (size_t) t + corner ] ];
            n = n + normal.AsVector() * GetCornerAngle( triangle, corner );
        }
    }
    for ( size_t v=0; v<normals.size(); ++v )
    {
        MATH::CTDirectionVector3D< T > unit;
        normals[ v ] = MATH::CTDirectionVector3D< T >::TryCreate( normals[ v ], unit ) ? unit.AsVector() : TVector();
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTTriangleMesh< T >::GetSurfaceArea( void ) const
{
    T area = TTraits::Zero();
    const UInt32 count = GetTriangleCount();
    for ( UInt32 t=0; t<count; ++t )
        area = area + GetTriangle( t ).GetArea();
    return area;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTTriangleMesh< T >::GetVolume( void ) const
{
    T sixTimes = TTraits::Zero();
    const UInt32 count = GetTriangleCount();
    for ( UInt32 t=0; t<count; ++t )
    {
        const TTriangle triangle = GetTriangle( t );
        sixTimes = sixTimes + triangle.a.ToPositionVector().Dot( triangle.b.ToPositionVector().Cross( triangle.c.ToPositionVector() ) );
    }
    return sixTimes / TTraits::FromInt32( 6 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTTriangleMesh< T >::TPoint
CTTriangleMesh< T >::GetCentroid( void ) const
{
    const UInt32 count = GetTriangleCount();

    /* the solid: each tetrahedron ( origin, a, b, c ) has its centroid at ( a + b + c ) / 4, weighted by its signed volume */
    T sixVolume = TTraits::Zero();
    TVector weighted;
    for ( UInt32 t=0; t<count; ++t )
    {
        const TTriangle triangle = GetTriangle( t );
        const TVector a = triangle.a.ToPositionVector();
        const TVector b = triangle.b.ToPositionVector();
        const TVector c = triangle.c.ToPositionVector();
        const T six = a.Dot( b.Cross( c ) );
        sixVolume = sixVolume + six;
        weighted = weighted + ( ( a + b ) + c ) * six;
    }
    if ( TTraits::Zero() != sixVolume )
        return TPoint( weighted / ( sixVolume * TTraits::FromInt32( 4 ) ) );

    /* the surface: triangle centroids weighted by area */
    T area = TTraits::Zero();
    TVector surface;
    for ( UInt32 t=0; t<count; ++t )
    {
        const TTriangle triangle = GetTriangle( t );
        const T a = triangle.GetArea();
        area = area + a;
        surface = surface + triangle.GetCentroid().ToPositionVector() * a;
    }
    if ( TTraits::Zero() < area )
        return TPoint( surface / area );

    if ( vertices.empty() )
        return TPoint();
    TVector sum;
    for ( size_t v=0; v<vertices.size(); ++v )
        sum = sum + vertices[ v ].ToPositionVector();
    return TPoint( sum / TTraits::FromInt32( (Int32) vertices.size() ) );
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTTriangleMesh< T >::IsClosed( void ) const
{
    if ( IsEmpty() || !IsValid() )
        return false;

    /* every directed edge as ( low, high, direction ): sorted, an edge must appear exactly twice, once in each direction */
    const UInt32 count = GetTriangleCount();
    GUCEF::vector< UInt64 > edges;
    edges.reserve( 3 * (size_t) count );
    for ( UInt32 t=0; t<count; ++t )
    {
        for ( UInt32 e=0; e<3; ++e )
        {
            const UInt32 from = indices[ 3 * (size_t) t + e ];
            const UInt32 to = indices[ 3 * (size_t) t + ( e + 1 ) % 3 ];
            if ( from == to )
                return false;
            const UInt64 low = from < to ? from : to;
            const UInt64 high = from < to ? to : from;
            /* 31 bits per index plus a direction bit: meshes up to 2^31 vertices */
            edges.push_back( ( low << 32 ) | ( high << 1 ) | ( from < to ? 0 : 1 ) );
        }
    }
    std::sort( edges.begin(), edges.end() );
    if ( 0 != edges.size() % 2 )
        return false;
    for ( size_t i=0; i<edges.size(); i+=2 )
    {
        /* the pair must be the same undirected edge in both directions, and the next entry must be another edge */
        if ( ( edges[ i ] | 1 ) != edges[ i + 1 ] || 0 != ( edges[ i ] & 1 ) )
            return false;
        if ( i + 2 < edges.size() && ( edges[ i + 2 ] >> 1 ) == ( edges[ i ] >> 1 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTTriangleMesh< T >::FindWeldRoot( GUCEF::vector< UInt32 >& parent, UInt32 vertex )
{
    while ( parent[ vertex ] != vertex )
    {
        parent[ vertex ] = parent[ parent[ vertex ] ];
        vertex = parent[ vertex ];
    }
    return vertex;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTTriangleMesh< T >::Weld( const T& tolerance )
{
    const UInt32 count = (UInt32) vertices.size();
    if ( 0 == count )
        return 0;

    /* sweep along x: every pair within the tolerance is linked once (the later vertex of the sweep looks back) */
    GUCEF::vector< UInt32 > order( count );
    for ( UInt32 i=0; i<count; ++i )
        order[ i ] = i;
    GUCEF::vector< UInt32 > representative( count );
    for ( UInt32 i=0; i<count; ++i )
        representative[ i ] = i;

    SVertexByX byX;
    byX.points = &vertices;
    std::sort( order.begin(), order.end(), byX );

    const T squaredTolerance = tolerance * tolerance;
    for ( UInt32 k=1; k<count; ++k )
    {
        const UInt32 v = order[ k ];
        const TPoint& p = vertices[ v ];
        for ( UInt32 j=k; j>0; --j )
        {
            const UInt32 u = order[ j - 1 ];
            if ( tolerance < p.x - vertices[ u ].x )
                break;
            if ( squaredTolerance < ( vertices[ u ] - p ).GetSquaredLength() )
                continue;
            /* union: the root of lower index stays the root, so every root is the lowest index of its group */
            const UInt32 ru = FindWeldRoot( representative, u );
            const UInt32 rv = FindWeldRoot( representative, v );
            if ( ru < rv )
                representative[ rv ] = ru;
            else if ( rv < ru )
                representative[ ru ] = rv;
        }
    }
    for ( UInt32 v=0; v<count; ++v )
        representative[ v ] = FindWeldRoot( representative, v );

    /* remap the triangles, drop collapsed ones, then compact the vertices in first-use order */
    GUCEF::vector< UInt32 > kept;
    kept.reserve( indices.size() );
    for ( size_t t=0; t+2<indices.size(); t+=3 )
    {
        const UInt32 a = representative[ indices[ t ] ];
        const UInt32 b = representative[ indices[ t + 1 ] ];
        const UInt32 c = representative[ indices[ t + 2 ] ];
        if ( a == b || b == c || c == a )
            continue;
        kept.push_back( a );
        kept.push_back( b );
        kept.push_back( c );
    }
    const UInt32 unused = 0xFFFFFFFF;
    GUCEF::vector< UInt32 > newIndex( count, unused );
    GUCEF::vector< TPoint > compacted;
    compacted.reserve( count );
    for ( UInt32 v=0; v<count; ++v )
    {
        if ( representative[ v ] == v )
        {
            newIndex[ v ] = (UInt32) compacted.size();
            compacted.push_back( vertices[ v ] );
        }
    }
    for ( size_t i=0; i<kept.size(); ++i )
        kept[ i ] = newIndex[ kept[ i ] ];
    const UInt32 removed = count - (UInt32) compacted.size();
    vertices.swap( compacted );
    indices.swap( kept );
    return removed;
}

/*-------------------------------------------------------------------------*/

template < typename T >
typename CTTriangleMesh< T >::TBox
CTTriangleMesh< T >::GetBounds( void ) const
{
    if ( vertices.empty() )
        return TBox();
    return TBox::FromPoints( &vertices[ 0 ], (UInt32) vertices.size() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE CTTriangleMesh< T >
CTTriangleMesh< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const
{
    CTTriangleMesh result( *this );
    for ( size_t v=0; v<result.vertices.size(); ++v )
        result.vertices[ v ] = matrix.TransformPoint( vertices[ v ] );
    if ( TTraits::IsNegative( matrix.GetLinear().GetDeterminant() ) )
    {
        for ( size_t t=0; t+2<result.indices.size(); t+=3 )
        {
            const UInt32 swap = result.indices[ t + 1 ];
            result.indices[ t + 1 ] = result.indices[ t + 2 ];
            result.indices[ t + 2 ] = swap;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTTriangleMesh< Float32 > TFloat32TriangleMesh;
typedef CTTriangleMesh< Float64 > TFloat64TriangleMesh;
typedef CTTriangleMesh< Float32 > CTriangleMesh;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

/* BuildBvh needs the hierarchy; included after the class so that either header can be included first */
#ifndef GUCEF_GEOMETRY_CTMESHBVH_H
#include "gucefGEOMETRY_CTMeshBvh.h"
#define GUCEF_GEOMETRY_CTMESHBVH_H
#endif /* GUCEF_GEOMETRY_CTMESHBVH_H ? */

namespace GUCEF {
namespace GEOMETRY {

template < typename T >
bool
CTTriangleMesh< T >::BuildBvh( CTMeshBvh< T >& bvh ) const
{
    return bvh.Build( *this );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTTriangleMesh< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTTriangleMesh< Float64 > )

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

#endif /* GUCEF_GEOMETRY_CTTRIANGLEMESH_H ? */
