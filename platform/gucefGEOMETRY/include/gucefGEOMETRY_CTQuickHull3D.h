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

#ifndef GUCEF_GEOMETRY_CTQUICKHULL3D_H
#define GUCEF_GEOMETRY_CTQUICKHULL3D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CQUICKHULL3DCORE_H
#include "gucefGEOMETRY_CQuickHull3DCore.h"
#define GUCEF_GEOMETRY_CQUICKHULL3DCORE_H
#endif /* GUCEF_GEOMETRY_CQUICKHULL3DCORE_H ? */

#ifndef GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#include "gucefGEOMETRY_CTConvexHull3D.h"
#define GUCEF_GEOMETRY_CTCONVEXHULL3D_H
#endif /* GUCEF_GEOMETRY_CTCONVEXHULL3D_H ? */

#ifndef GUCEF_MATH_POINTS_H
#include "gucefMATH_points.h"
#define GUCEF_MATH_POINTS_H
#endif /* GUCEF_MATH_POINTS_H ? */

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
 *  The convex hull of a 3D point set by quickhull (section 7.9 of the design), producing a CTConvexHull3D with faces.
 *  The arithmetic is Float64 for both kinds (a Float32 input is widened exactly), with a relative merge tolerance and
 *  merging of coplanar faces into polygons (see CQuickHull3DCore): the hull is a robust approximation, not an exact
 *  predicate; points within the tolerance of a face are inside, and CTConvexHull3D::Contains of the result is not an
 *  exact predicate either (it works on the stored planes).
 *
 *  - Kinds: Float32 and Float64 (a kind without floating point is not supported).
 *  - Result: Build keeps the hull in this object. A hull of at least four points that are not coplanar has status OK,
 *    faces counter-clockwise from outside, and only extreme points as vertices (points on an edge or a face are not
 *    vertices). Degenerate sets give a lower dimensional result without faces (see TQuickHullStatus).
 *  - Layout: the object owns its work arrays and its result; copying is not supported.
 *  - Realtime: not realtime-safe (allocation); one thread at a time.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTQuickHull3D
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;

    /** An engine with the default tolerance factor 3 */
    CTQuickHull3D( void ) : m_core(), m_toleranceFactor( 3.0 ), m_status( QUICKHULL_EMPTY ), m_hull(), m_vertexIndices() {}

    /** The tolerance is 'factor' * DBL_EPSILON times the sum of the largest absolute coordinates per axis (3 by default; larger merges more) */
    void SetToleranceFactor( Float64 factor ) { m_toleranceFactor = factor; }

    GUCEF_GEOMETRY_FORCE_INLINE Float64 GetToleranceFactor( void ) const { return m_toleranceFactor; }

    /** The tolerance used by the last Build, in the units of the coordinates */
    GUCEF_GEOMETRY_FORCE_INLINE Float64 GetTolerance( void ) const { return m_core.GetTolerance(); }

    /** Builds the hull of 'count' points and returns the status; the hull is then available through GetHull */
    TQuickHullStatus Build( const TPoint* points, UInt32 count );

    GUCEF_GEOMETRY_FORCE_INLINE TQuickHullStatus GetStatus( void ) const { return m_status; }

    /** The hull of the last Build: vertices (a subset of the input points) and, for status OK, faces */
    GUCEF_GEOMETRY_FORCE_INLINE const CTConvexHull3D< T >& GetHull( void ) const { return m_hull; }

    /** For each vertex of the hull the index of the input point it is */
    GUCEF_GEOMETRY_FORCE_INLINE const GUCEF::vector< UInt32 >& GetVertexIndices( void ) const { return m_vertexIndices; }

    /** The number of edges of the polyhedron of status OK (the sum of the face sizes halved); 0 otherwise */
    UInt32 GetEdgeCount( void ) const;

    private:

    CTQuickHull3D( const CTQuickHull3D& src );              /**< not implemented */
    CTQuickHull3D& operator=( const CTQuickHull3D& src );   /**< not implemented */

    CQuickHull3DCore m_core;
    Float64 m_toleranceFactor;
    TQuickHullStatus m_status;
    CTConvexHull3D< T > m_hull;
    GUCEF::vector< UInt32 > m_vertexIndices;
};

/*-------------------------------------------------------------------------*/

template < typename T >
TQuickHullStatus
CTQuickHull3D< T >::Build( const TPoint* points, UInt32 count )
{
    GUCEF::vector< Float64 > xyz;
    xyz.reserve( (size_t) count * 3 );
    for ( UInt32 i=0; i<count; ++i )
    {
        xyz.push_back( TTraits::ToFloat64( points[ i ].x ) );
        xyz.push_back( TTraits::ToFloat64( points[ i ].y ) );
        xyz.push_back( TTraits::ToFloat64( points[ i ].z ) );
    }
    m_status = m_core.Build( xyz.empty() ? GUCEF_NULL : &xyz[ 0 ], count, m_toleranceFactor );
    m_vertexIndices = m_core.GetVertexIndices();
    m_hull = CTConvexHull3D< T >();
    if ( QUICKHULL_OK != m_status && QUICKHULL_POINT != m_status && QUICKHULL_LINE != m_status && QUICKHULL_PLANE != m_status )
    {
        m_vertexIndices.clear();
        return m_status;
    }
    GUCEF::vector< TPoint > vertices;
    vertices.reserve( m_vertexIndices.size() );
    for ( size_t i=0; i<m_vertexIndices.size(); ++i )
        vertices.push_back( points[ m_vertexIndices[ i ] ] );
    m_hull.SetVertices( vertices.empty() ? GUCEF_NULL : &vertices[ 0 ], (UInt32) vertices.size() );
    if ( QUICKHULL_OK == m_status )
    {
        const GUCEF::vector< UInt32 >& faceIndices = m_core.GetFaceIndices();
        const GUCEF::vector< UInt32 >& faceSizes = m_core.GetFaceSizes();
        if ( !m_hull.SetFaces( &faceIndices[ 0 ], &faceSizes[ 0 ], (UInt32) faceSizes.size() ) )
        {
            m_status = QUICKHULL_FAILED;
            m_hull = CTConvexHull3D< T >();
            m_vertexIndices.clear();
        }
    }
    return m_status;
}

/*-------------------------------------------------------------------------*/

template < typename T >
UInt32
CTQuickHull3D< T >::GetEdgeCount( void ) const
{
    if ( QUICKHULL_OK != m_status )
        return 0;
    UInt32 total = 0;
    const GUCEF::vector< UInt32 >& sizes = m_core.GetFaceSizes();
    for ( size_t i=0; i<sizes.size(); ++i )
        total += sizes[ i ];
    return total / 2;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTQuickHull3D< Float32 > TFloat32QuickHull3D;
typedef CTQuickHull3D< Float64 > TFloat64QuickHull3D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      EXPORTED INSTANTIATIONS                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTQuickHull3D< Float32 > )
GUCEF_GEOMETRY_EXTERN_TEMPLATE_CLASS( CTQuickHull3D< Float64 > )

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTQUICKHULL3D_H ? */
