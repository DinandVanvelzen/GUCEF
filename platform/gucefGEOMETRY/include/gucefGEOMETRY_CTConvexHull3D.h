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

#ifndef GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#define GUCEF_GEOMETRY_CTCONVEXHULL3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#include "gucefGEOMETRY_SShapeTextReader.h"
#define GUCEF_GEOMETRY_SSHAPETEXTREADER_H
#endif /* GUCEF_GEOMETRY_SSHAPETEXTREADER_H ? */

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

#ifndef GUCEF_GEOMETRY_CTPLANE_H
#include "gucefGEOMETRY_CTPlane.h"
#define GUCEF_GEOMETRY_CTPLANE_H
#endif /* GUCEF_GEOMETRY_CTPLANE_H ? */

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
namespace MATH {
template < typename T > class CTMatrix4D;
}
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T > class CTRay3D;
template < typename T > class CTSegment3D;
template < typename T > class CTAxisAlignedBox3D;
template < typename T > class CTOrientedBox3D;

/*-------------------------------------------------------------------------*/

/**
 *  A convex polyhedron given by its vertices, with optional faces (section 7.2): the shape of convex hulls of point
 *  sets (built by quickhull, section 7.9, or from given points) and of convex physics shapes.
 *
 *  - Without faces the shape is the convex hull of the vertices: the support is a scan of the vertices, containment,
 *    closest points and ray casts use GJK. With faces (convex polygons, counter-clockwise seen from outside, every edge
 *    shared by two faces) the face planes are kept: containment and ray casts are plane tests (Cyrus-Beck), the
 *    support climbs the vertex adjacency of the faces from the first vertex of the first face (vertices that no face
 *    uses are then ignored), the volume and surface area are defined.
 *  - Kind: floats and fixed point (Float32 and Float64 are exported; TFixed32_32 is used by the fixed-point GJK and
 *    EPA); needs a square root.
 *  - Layout: owns its vertices, faces, planes and adjacency in GUCEF::vector members (copying copies them); the
 *    constructors and the Set members allocate, the const queries do not.
 *  - Degenerate: no vertices is an empty hull that contains nothing (support: the origin); one vertex is a point, two
 *    a segment. SetFaces rejects indices out of range and faces with fewer than three vertices.
 *  - Realtime: the const queries are realtime-safe; construction, the Set members, Transformed, the wireframe, text and
 *    DataNode members are not.
 *  - Text: "ConvexHull3D(vertices ((x, y, z), ...), faces ((0, 1, 2), ...))"; DataNode: child node vertices with one
 *    child node point per vertex, child node faces with one child node face per face whose attribute indices lists
 *    the vertex indices ("0, 1, 2").
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTConvexHull3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef GUCEF::vector< TPoint > TPointVector;
    typedef GUCEF::vector< UInt32 > TIndexVector;
    typedef GUCEF::vector< CTPlane< T > > TPlaneVector;

    /** The empty hull */
    CTConvexHull3D( void );

    /** The hull of 'count' points, without faces */
    CTConvexHull3D( const TPoint* points, UInt32 count );

    /** The eight corners and six faces of an axis aligned box (empty for the empty box) */
    static CTConvexHull3D FromAxisAlignedBox( const CTAxisAlignedBox3D< T >& box );

    /** The eight corners and six faces of an oriented box (empty for an invalid box) */
    static CTConvexHull3D FromOrientedBox( const CTOrientedBox3D< T >& box );

    /** Replaces the vertices; the faces are removed */
    void SetVertices( const TPoint* points, UInt32 count );

    /**
     *  Sets the faces: 'faceSizes' holds the vertex count of each of the 'faceCount' faces, 'indices' their vertex
     *  indices one face after the other, counter-clockwise seen from outside. False (faces unchanged) for an index out
     *  of range or a face with fewer than three vertices. The face planes come from Newell's normal of each polygon.
     */
    bool SetFaces( const UInt32* indices, const UInt32* faceSizes, UInt32 faceCount );

    /** Removes the faces, keeping the vertices */
    void ClearFaces( void );

    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_vertices.empty(); }
    GUCEF_GEOMETRY_FORCE_INLINE bool HasFaces( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return !m_planes.empty(); }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetVertexCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return (UInt32) m_vertices.size(); }

    /** The vertex at 'index' (below GetVertexCount) */
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetVertex( UInt32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( index < m_vertices.size() );
        return m_vertices[ index ];
    }

    GUCEF_GEOMETRY_FORCE_INLINE const TPointVector& GetVertices( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_vertices; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetFaceCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return (UInt32) m_planes.size(); }

    /** The number of vertices of face 'face' (below GetFaceCount) */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetFaceVertexCount( UInt32 face ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( face < m_planes.size() );
        return m_faceStarts[ face + 1 ] - m_faceStarts[ face ];
    }

    /** The vertex index of corner 'corner' of face 'face' */
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetFaceVertexIndex( UInt32 face, UInt32 corner ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( face < m_planes.size() && corner < GetFaceVertexCount( face ) );
        return m_faceIndices[ m_faceStarts[ face ] + corner ];
    }

    /** The outward plane of face 'face' */
    GUCEF_GEOMETRY_FORCE_INLINE const CTPlane< T >& GetFacePlane( UInt32 face ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        GUCEF_GEOMETRY_ASSERT( face < m_planes.size() );
        return m_planes[ face ];
    }

    /**
     *  True when the point is inside or on the surface: with faces, the signed distance to every stored face plane is
     *  not positive (the planes are stored rounded, so floats can misclassify points within rounding of a face; the
     *  test is not an exact predicate); without faces, GJK within the default length tolerance of the kind. False for
     *  the empty hull.
     */
    bool Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The nearest point of the solid hull (GJK): the point itself when inside; the point for the empty hull */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Distance from the solid to the point, 0 inside; the largest number of the kind for the empty hull */
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The vertex furthest along 'direction' (the first one found on ties); the origin for the empty hull */
    TPoint GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The axis aligned bounds of the vertices; empty for the empty hull */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /**
     *  Hit of the ray with the solid hull: Cyrus-Beck against the face planes when faces are present (the feature index
     *  is the entry face), the GJK ray cast otherwise. A ray starting inside hits at distance 0 with
     *  RAYHIT_STARTED_INSIDE and the normal opposite to the ray. The empty hull is missed.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The enclosed volume (divergence theorem over the faces); 0 without faces */
    T GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The area of the faces; 0 without faces */
    T GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** Appends every edge of the faces once; without faces, nothing. 'segmentsPerCircle' is not used */
    void GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 segmentsPerCircle ) const;

    /**
     *  The image under an affine matrix: an affine image of a convex polyhedron is convex, so the hull family is closed
     *  under every affine map. The faces are kept (reversed when the map mirrors), the planes recomputed (allocates).
     */
    CTConvexHull3D Transformed( const MATH::CTMatrix4D< T >& matrix ) const;

    friend bool operator==( const CTConvexHull3D& a, const CTConvexHull3D& b )
    {
        return a.m_vertices == b.m_vertices && a.m_faceIndices == b.m_faceIndices && a.m_faceStarts == b.m_faceStarts;
    }
    friend bool operator!=( const CTConvexHull3D& a, const CTConvexHull3D& b ) { return !( a == b ); }

    /** The same faces and vertices equal within the default tolerances of the kind */
    bool Equals( const CTConvexHull3D& other ) const;

    /** "ConvexHull3D(vertices ((x, y, z), ...), faces ((0, 1, 2), ...))" */
    CORE::CString ToString( void ) const;

    /** Parses the ToString form; false leaves this hull unchanged */
    bool FromString( const CORE::CString& text );

    /** Writes the child nodes vertices and faces into 'node' */
    bool Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const;

    /** Reads the child nodes vertices and faces (faces may be missing); a failure leaves this hull unchanged */
    bool Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings );

    private:

    /** "( i, j, k )" */
    static bool TryIndexList( SShapeTextReader& reader, TIndexVector& indices );
    static bool ParseIndexText( const CORE::CString& text, TIndexVector& indices );
    void RebuildDerived( void );

    TPointVector m_vertices;
    TIndexVector m_faceIndices;
    TIndexVector m_faceStarts;          /**< face f uses m_faceIndices[ m_faceStarts[ f ] .. m_faceStarts[ f + 1 ] ), size faceCount + 1 (or empty) */
    TPlaneVector m_planes;
    TIndexVector m_adjacencyStarts;     /**< neighbours of vertex v: m_adjacency[ m_adjacencyStarts[ v ] .. m_adjacencyStarts[ v + 1 ] ) */
    TIndexVector m_adjacency;
    UInt32 m_firstFaceVertex;           /**< the start of the hill climbing: the first vertex used by a face */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTConvexHull3D< Float32 > TFloat32ConvexHull3D;
typedef CTConvexHull3D< Float64 > TFloat64ConvexHull3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*
 *  The related classes used by the members; included after the class so that each header can be included first.
 */

#ifndef GUCEF_GEOMETRY_CTSEGMENT3D_H
#include "gucefGEOMETRY_CTSegment3D.h"
#define GUCEF_GEOMETRY_CTSEGMENT3D_H
#endif /* GUCEF_GEOMETRY_CTSEGMENT3D_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#include "gucefGEOMETRY_CTOrientedBox3D.h"
#define GUCEF_GEOMETRY_CTORIENTEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTORIENTEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTGJK_H
#include "gucefGEOMETRY_CTGjk.h"
#define GUCEF_GEOMETRY_CTGJK_H
#endif /* GUCEF_GEOMETRY_CTGJK_H ? */

#ifndef GUCEF_MATH_MATRICES_H
#include "gucefMATH_matrices.h"
#define GUCEF_MATH_MATRICES_H
#endif /* GUCEF_MATH_MATRICES_H ? */

namespace GUCEF {
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T >
CTConvexHull3D< T >::CTConvexHull3D( void )
    : m_vertices(), m_faceIndices(), m_faceStarts(), m_planes(), m_adjacencyStarts(), m_adjacency(), m_firstFaceVertex( 0 )
{
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTConvexHull3D< T >::CTConvexHull3D( const TPoint* points, UInt32 count )
    : m_vertices(), m_faceIndices(), m_faceStarts(), m_planes(), m_adjacencyStarts(), m_adjacency(), m_firstFaceVertex( 0 )
{
    SetVertices( points, count );
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConvexHull3D< T >::SetVertices( const TPoint* points, UInt32 count )
{
    GUCEF_GEOMETRY_ASSERT( 0 == count || GUCEF_NULL != points );
    m_vertices.clear();
    if ( GUCEF_NULL != points )
        m_vertices.assign( points, points + count );
    ClearFaces();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConvexHull3D< T >::ClearFaces( void )
{
    m_faceIndices.clear();
    m_faceStarts.clear();
    m_planes.clear();
    m_adjacencyStarts.clear();
    m_adjacency.clear();
    m_firstFaceVertex = 0;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::SetFaces( const UInt32* indices, const UInt32* faceSizes, UInt32 faceCount )
{
    if ( 0 == faceCount )
    {
        ClearFaces();
        return true;
    }
    if ( GUCEF_NULL == indices || GUCEF_NULL == faceSizes )
        return false;
    TIndexVector starts;
    starts.reserve( faceCount + 1 );
    UInt32 total = 0;
    starts.push_back( 0 );
    for ( UInt32 f=0; f<faceCount; ++f )
    {
        if ( faceSizes[ f ] < 3 )
            return false;
        total += faceSizes[ f ];
        starts.push_back( total );
    }
    for ( UInt32 i=0; i<total; ++i )
        if ( indices[ i ] >= m_vertices.size() )
            return false;
    m_faceStarts.swap( starts );
    m_faceIndices.assign( indices, indices + total );
    RebuildDerived();
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConvexHull3D< T >::RebuildDerived( void )
{
    const UInt32 faceCount = m_faceStarts.empty() ? 0 : (UInt32) m_faceStarts.size() - 1;
    m_planes.clear();
    m_planes.reserve( faceCount );
    for ( UInt32 f=0; f<faceCount; ++f )
    {
        /* Newell's normal of the polygon, relative to its first vertex; the plane through the mean of its vertices */
        const UInt32 first = m_faceStarts[ f ];
        const UInt32 size = m_faceStarts[ f + 1 ] - first;
        const TPoint& origin = m_vertices[ m_faceIndices[ first ] ];
        TVector normal;
        TVector sum;
        for ( UInt32 k=0; k<size; ++k )
        {
            const TVector p = m_vertices[ m_faceIndices[ first + k ] ] - origin;
            const TVector q = m_vertices[ m_faceIndices[ first + ( k + 1 ) % size ] ] - origin;
            normal = normal + p.Cross( q );
            sum = sum + p;
        }
        TDirection unit;
        if ( !TDirection::TryCreate( normal, unit ) )
            unit = TDirection::UnitX();
        const TPoint centre = origin + sum / TTraits::FromInt32( (Int32) size );
        m_planes.push_back( CTPlane< T >::FromPointNormal( centre, unit ) );
    }

    /* vertex adjacency (CSR) from the directed face edges; each undirected edge once per direction */
    const UInt32 vertexCount = (UInt32) m_vertices.size();
    m_adjacencyStarts.assign( vertexCount + 1, 0 );
    m_adjacency.clear();
    if ( 0 == faceCount )
    {
        m_adjacencyStarts.clear();
        return;
    }
    for ( UInt32 f=0; f<faceCount; ++f )
    {
        const UInt32 first = m_faceStarts[ f ];
        const UInt32 size = m_faceStarts[ f + 1 ] - first;
        for ( UInt32 k=0; k<size; ++k )
            ++m_adjacencyStarts[ m_faceIndices[ first + k ] + 1 ];
    }
    for ( UInt32 v=0; v<vertexCount; ++v )
        m_adjacencyStarts[ v + 1 ] += m_adjacencyStarts[ v ];
    m_adjacency.assign( m_adjacencyStarts[ vertexCount ], 0 );
    TIndexVector fill( m_adjacencyStarts.begin(), m_adjacencyStarts.end() - 1 );
    for ( UInt32 f=0; f<faceCount; ++f )
    {
        const UInt32 first = m_faceStarts[ f ];
        const UInt32 size = m_faceStarts[ f + 1 ] - first;
        for ( UInt32 k=0; k<size; ++k )
        {
            const UInt32 from = m_faceIndices[ first + k ];
            const UInt32 to = m_faceIndices[ first + ( k + 1 ) % size ];
            m_adjacency[ fill[ from ]++ ] = to;
        }
    }
    m_firstFaceVertex = m_faceIndices[ 0 ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTConvexHull3D< T >
CTConvexHull3D< T >::FromAxisAlignedBox( const CTAxisAlignedBox3D< T >& box )
{
    CTConvexHull3D hull;
    if ( box.IsEmpty() )
        return hull;
    TPoint corners[ 8 ];
    for ( UInt32 i=0; i<8; ++i )
        corners[ i ] = TPoint( 0 != ( i & 1 ) ? box.max.x : box.min.x, 0 != ( i & 2 ) ? box.max.y : box.min.y, 0 != ( i & 4 ) ? box.max.z : box.min.z );
    hull.SetVertices( corners, 8 );
    /* corner index bits: 1 = +X, 2 = +Y, 4 = +Z; faces counter-clockwise seen from outside */
    static const UInt32 indices[ 24 ] = { 0, 4, 6, 2,   1, 3, 7, 5,   0, 1, 5, 4,   2, 6, 7, 3,   0, 2, 3, 1,   4, 5, 7, 6 };
    static const UInt32 sizes[ 6 ] = { 4, 4, 4, 4, 4, 4 };
    hull.SetFaces( indices, sizes, 6 );
    return hull;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTConvexHull3D< T >
CTConvexHull3D< T >::FromOrientedBox( const CTOrientedBox3D< T >& box )
{
    CTConvexHull3D hull;
    if ( !box.IsValid() )
        return hull;
    TPoint corners[ 8 ];
    box.GetCorners( corners );
    hull.SetVertices( corners, 8 );
    /* the corner order of CTOrientedBox3D::GetCorner: bit 0 +X, bit 1 +Y, bit 2 +Z (a rotation keeps the orientation) */
    static const UInt32 indices[ 24 ] = { 0, 4, 6, 2,   1, 3, 7, 5,   0, 1, 5, 4,   2, 6, 7, 3,   0, 2, 3, 1,   4, 5, 7, 6 };
    static const UInt32 sizes[ 6 ] = { 4, 4, 4, 4, 4, 4 };
    hull.SetFaces( indices, sizes, 6 );
    return hull;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTConvexHull3D< T >::TPoint
CTConvexHull3D< T >::GetSupport( const TVector& direction ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    const UInt32 count = (UInt32) m_vertices.size();
    if ( 0 == count )
        return TPoint();
    if ( !m_adjacencyStarts.empty() )
    {
        UInt32 best = m_firstFaceVertex;
        T bestDot = m_vertices[ best ].ToPositionVector().Dot( direction );
        /* steepest ascent over the vertex graph: on a convex polyhedron every local maximum is a global one */
        bool improved = true;
        UInt32 steps = 0;
        while ( improved && steps <= count )
        {
            improved = false;
            ++steps;
            const UInt32 current = best;
            for ( UInt32 k=m_adjacencyStarts[ current ]; k<m_adjacencyStarts[ current + 1 ]; ++k )
            {
                const UInt32 neighbour = m_adjacency[ k ];
                const T d = m_vertices[ neighbour ].ToPositionVector().Dot( direction );
                if ( bestDot < d )
                {
                    bestDot = d;
                    best = neighbour;
                    improved = true;
                }
            }
        }
        return m_vertices[ best ];
    }
    UInt32 best = 0;
    T bestDot = m_vertices[ 0 ].ToPositionVector().Dot( direction );
    for ( UInt32 i=1; i<count; ++i )
    {
        const T d = m_vertices[ i ].ToPositionVector().Dot( direction );
        if ( bestDot < d )
        {
            bestDot = d;
            best = i;
        }
    }
    return m_vertices[ best ];
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTConvexHull3D< T >::Contains( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertices.empty() )
        return false;
    if ( !m_planes.empty() )
    {
        for ( UInt32 f=0; f<(UInt32) m_planes.size(); ++f )
            if ( TTraits::Zero() < m_planes[ f ].GetSignedDistance( point ) )
                return false;
        return true;
    }
    SGjkResult< T > result;
    CTGjk< T >::ComputeDistance( CTShapeSupport< CTConvexHull3D >( *this ), CTPointSupport< T >( point ), CTGeometryTolerance< T >(), GUCEF_NULL, result );
    return result.overlapping;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE typename CTConvexHull3D< T >::TPoint
CTConvexHull3D< T >::ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertices.empty() || ( !m_planes.empty() && Contains( point ) ) )
        return point;
    SGjkResult< T > result;
    CTGjk< T >::ComputeDistance( CTShapeSupport< CTConvexHull3D >( *this ), CTPointSupport< T >( point ), CTGeometryTolerance< T >(), GUCEF_NULL, result );
    if ( result.overlapping )
        return point;
    return result.pointOnA;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTConvexHull3D< T >::GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertices.empty() )
        return TTraits::Max();
    return ( point - ClosestPoint( point ) ).GetLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTConvexHull3D< T >::GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertices.empty() )
        return TTraits::Max();
    return ( point - ClosestPoint( point ) ).GetSquaredLength();
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTAxisAlignedBox3D< T >
CTConvexHull3D< T >::GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertices.empty() )
        return CTAxisAlignedBox3D< T >();
    return CTAxisAlignedBox3D< T >::FromPoints( &m_vertices[ 0 ], (UInt32) m_vertices.size() );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE bool
CTConvexHull3D< T >::RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_vertices.empty() || TTraits::IsNegative( maxDistance ) )
        return false;
    const TVector d = ray.direction.AsVector();
    if ( !m_planes.empty() )
    {
        T enter = TTraits::Zero();
        T exit = maxDistance;
        Int32 enterFace = -1;
        for ( UInt32 f=0; f<(UInt32) m_planes.size(); ++f )
        {
            const CTPlane< T >& plane = m_planes[ f ];
            const T denominator = plane.normal.Dot( d );
            const T numerator = TTraits::Zero() - plane.GetSignedDistance( ray.origin );
            if ( TTraits::Zero() == denominator )
            {
                if ( TTraits::IsNegative( numerator ) )
                    return false;
                continue;
            }
            const T t = numerator / denominator;
            if ( TTraits::IsNegative( denominator ) )
            {
                if ( enter < t )
                {
                    enter = t;
                    enterFace = (Int32) f;
                }
            }
            else if ( t < exit )
            {
                exit = t;
            }
            if ( exit < enter )
                return false;
        }
        SRayCastResult< T > hit;
        if ( enterFace < 0 )
        {
            hit.distance = TTraits::Zero();
            hit.point = ray.origin;
            hit.normal = -d;
            hit.flags = RAYHIT_STARTED_INSIDE;
        }
        else
        {
            hit.distance = enter;
            hit.point = ray.GetPoint( enter );
            hit.normal = m_planes[ enterFace ].normal.AsVector();
            hit.featureIndex = enterFace;
        }
        result = hit;
        return true;
    }

    SGjkCastResult< T > cast;
    CTGjk< T >::Cast( CTPointSupport< T >( ray.origin ), d * maxDistance, TTraits::One(), CTShapeSupport< CTConvexHull3D >( *this ), CTGeometryTolerance< T >(), cast );
    if ( !cast.hit )
        return false;
    SRayCastResult< T > hit;
    hit.distance = cast.fraction * maxDistance;
    hit.point = ray.GetPoint( hit.distance );
    if ( TTraits::Zero() == cast.fraction || cast.normal.IsZero() )
    {
        hit.distance = TTraits::Zero();
        hit.point = ray.origin;
        hit.normal = -d;
        hit.flags = RAYHIT_STARTED_INSIDE;
    }
    else
    {
        hit.normal = cast.normal;
    }
    result = hit;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTConvexHull3D< T >::GetVolume( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_planes.empty() )
        return TTraits::Zero();
    /* the signed volumes of the tetrahedra from vertex 0 to the triangle fans of the faces */
    const TPoint& reference = m_vertices[ 0 ];
    T sum = TTraits::Zero();
    for ( UInt32 f=0; f<(UInt32) m_planes.size(); ++f )
    {
        const UInt32 first = m_faceStarts[ f ];
        const UInt32 size = m_faceStarts[ f + 1 ] - first;
        const TVector p0 = m_vertices[ m_faceIndices[ first ] ] - reference;
        for ( UInt32 k=1; k+1<size; ++k )
        {
            const TVector p1 = m_vertices[ m_faceIndices[ first + k ] ] - reference;
            const TVector p2 = m_vertices[ m_faceIndices[ first + k + 1 ] ] - reference;
            sum = sum + p0.Dot( p1.Cross( p2 ) );
        }
    }
    return TTraits::Abs( sum ) / TTraits::FromInt32( 6 );
}

/*-------------------------------------------------------------------------*/

template < typename T >
GUCEF_GEOMETRY_PRECISE_INLINE T
CTConvexHull3D< T >::GetSurfaceArea( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    T sum = TTraits::Zero();
    for ( UInt32 f=0; f<(UInt32) m_planes.size(); ++f )
    {
        const UInt32 first = m_faceStarts[ f ];
        const UInt32 size = m_faceStarts[ f + 1 ] - first;
        const TPoint& origin = m_vertices[ m_faceIndices[ first ] ];
        TVector normal;
        for ( UInt32 k=1; k+1<size; ++k )
            normal = normal + ( m_vertices[ m_faceIndices[ first + k ] ] - origin ).Cross( m_vertices[ m_faceIndices[ first + k + 1 ] ] - origin );
        sum = sum + normal.GetLength();
    }
    return sum * TTraits::Half();
}

/*-------------------------------------------------------------------------*/

template < typename T >
void
CTConvexHull3D< T >::GetWireframeLines( GUCEF::vector< CTSegment3D< T > >& lines, UInt32 /*segmentsPerCircle*/ ) const
{
    for ( UInt32 f=0; f<(UInt32) m_planes.size(); ++f )
    {
        const UInt32 first = m_faceStarts[ f ];
        const UInt32 size = m_faceStarts[ f + 1 ] - first;
        for ( UInt32 k=0; k<size; ++k )
        {
            const UInt32 from = m_faceIndices[ first + k ];
            const UInt32 to = m_faceIndices[ first + ( k + 1 ) % size ];
            if ( from < to )
                lines.push_back( CTSegment3D< T >( m_vertices[ from ], m_vertices[ to ] ) );
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename T >
CTConvexHull3D< T >
CTConvexHull3D< T >::Transformed( const MATH::CTMatrix4D< T >& matrix ) const
{
    CTConvexHull3D result;
    result.m_vertices.reserve( m_vertices.size() );
    for ( UInt32 i=0; i<(UInt32) m_vertices.size(); ++i )
        result.m_vertices.push_back( matrix.TransformPoint( m_vertices[ i ] ) );
    result.m_faceStarts = m_faceStarts;
    result.m_faceIndices = m_faceIndices;
    if ( TTraits::IsNegative( matrix.GetDeterminant() ) )
    {
        /* a mirror reverses the orientation: reverse every face */
        for ( UInt32 f=0; f+1<(UInt32) result.m_faceStarts.size(); ++f )
        {
            UInt32 i = result.m_faceStarts[ f ];
            UInt32 j = result.m_faceStarts[ f + 1 ] - 1;
            while ( i < j )
            {
                const UInt32 swap = result.m_faceIndices[ i ];
                result.m_faceIndices[ i ] = result.m_faceIndices[ j ];
                result.m_faceIndices[ j ] = swap;
                ++i;
                --j;
            }
        }
    }
    result.RebuildDerived();
    return result;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::Equals( const CTConvexHull3D& other ) const
{
    if ( m_vertices.size() != other.m_vertices.size() || m_faceIndices != other.m_faceIndices || m_faceStarts != other.m_faceStarts )
        return false;
    for ( UInt32 i=0; i<(UInt32) m_vertices.size(); ++i )
        if ( !m_vertices[ i ].Equals( other.m_vertices[ i ] ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
CORE::CString
CTConvexHull3D< T >::ToString( void ) const
{
    CORE::CString text( "ConvexHull3D(vertices " );
    text += ShapePointsToString( "", m_vertices );
    text += ", faces (";
    for ( UInt32 f=0; f<GetFaceCount(); ++f )
    {
        if ( f > 0 )
            text += ", ";
        text += '(';
        for ( UInt32 k=m_faceStarts[ f ]; k<m_faceStarts[ f + 1 ]; ++k )
        {
            if ( k > m_faceStarts[ f ] )
                text += ", ";
            text += CORE::ToString( m_faceIndices[ k ] );
        }
        text += ')';
    }
    text += "))";
    return text;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::TryIndexList( SShapeTextReader& reader, TIndexVector& indices )
{
    const UInt32 start = reader.position;
    TIndexVector parsed;
    if ( reader.TryChar( '(' ) )
    {
        for ( ;; )
        {
            UInt32 index = 0;
            if ( !reader.TryScalar( index ) )
                break;
            parsed.push_back( index );
            if ( reader.TryChar( ',' ) )
                continue;
            if ( reader.TryChar( ')' ) )
            {
                indices.swap( parsed );
                return true;
            }
            break;
        }
    }
    reader.position = start;
    return false;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::FromString( const CORE::CString& text )
{
    SShapeTextReader reader( text );
    TPointVector vertices;
    if ( !reader.TryLiteral( "ConvexHull3D" ) || !reader.TryChar( '(' ) || !reader.TryLiteral( "vertices" ) || !reader.TryPointList( vertices ) ||
         !reader.TryChar( ',' ) || !reader.TryLiteral( "faces" ) || !reader.TryChar( '(' ) )
        return false;
    TIndexVector indices;
    TIndexVector sizes;
    if ( !reader.TryChar( ')' ) )
    {
        for ( ;; )
        {
            TIndexVector face;
            if ( !TryIndexList( reader, face ) )
                return false;
            sizes.push_back( (UInt32) face.size() );
            indices.insert( indices.end(), face.begin(), face.end() );
            if ( reader.TryChar( ',' ) )
                continue;
            if ( reader.TryChar( ')' ) )
                break;
            return false;
        }
    }
    if ( !reader.TryChar( ')' ) || !reader.AtEnd() )
        return false;
    CTConvexHull3D parsed( vertices.empty() ? GUCEF_NULL : &vertices[ 0 ], (UInt32) vertices.size() );
    if ( !sizes.empty() && !parsed.SetFaces( &indices[ 0 ], &sizes[ 0 ], (UInt32) sizes.size() ) )
        return false;
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::ParseIndexText( const CORE::CString& text, TIndexVector& indices )
{
    CORE::CString wrapped( "(" );
    wrapped += text;
    wrapped += ")";
    SShapeTextReader reader( wrapped );
    TIndexVector parsed;
    if ( !TryIndexList( reader, parsed ) || !reader.AtEnd() )
        return false;
    indices.swap( parsed );
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::Serialize( CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings ) const
{
    if ( !SerializeShapePoints( node, "vertices", m_vertices, settings ) )
        return false;
    CORE::CDataNode* faces = node.AddChild( CORE::CString( "faces" ), GUCEF_DATATYPE_ARRAY );
    if ( GUCEF_NULL == faces )
        return false;
    for ( UInt32 f=0; f<GetFaceCount(); ++f )
    {
        CORE::CString list;
        for ( UInt32 k=m_faceStarts[ f ]; k<m_faceStarts[ f + 1 ]; ++k )
        {
            if ( k > m_faceStarts[ f ] )
                list += ", ";
            list += CORE::ToString( m_faceIndices[ k ] );
        }
        CORE::CDataNode* face = faces->AddChild( CORE::CString( "face" ), GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == face || !face->SetAttribute( CORE::CString( "indices" ), list ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T >
bool
CTConvexHull3D< T >::Deserialize( const CORE::CDataNode& node, const CORE::CDataNodeSerializableSettings& settings )
{
    TPointVector vertices;
    if ( !DeserializeShapePoints( node, "vertices", vertices, settings ) )
        return false;
    TIndexVector indices;
    TIndexVector sizes;
    const CORE::CDataNode* faces = node.FindChild( CORE::CString( "faces" ) );
    if ( GUCEF_NULL != faces )
    {
        CORE::CDataNode::const_iterator i = faces->ConstBegin();
        while ( i != faces->ConstEnd() )
        {
            TIndexVector face;
            if ( !ParseIndexText( ( *i )->GetAttributeValue( CORE::CString( "indices" ) ).AsString(), face ) )
                return false;
            sizes.push_back( (UInt32) face.size() );
            indices.insert( indices.end(), face.begin(), face.end() );
            ++i;
        }
    }
    CTConvexHull3D parsed( vertices.empty() ? GUCEF_NULL : &vertices[ 0 ], (UInt32) vertices.size() );
    if ( !sizes.empty() && !parsed.SetFaces( &indices[ 0 ], &sizes[ 0 ], (UInt32) sizes.size() ) )
        return false;
    *this = parsed;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTConvexHull3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTConvexHull3D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTCONVEXHULL3D_H ? */
