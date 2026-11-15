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

#ifndef GUCEF_GEOMETRY_CTHEIGHTFIELD_H
#define GUCEF_GEOMETRY_CTHEIGHTFIELD_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_SRAYCASTRESULT_H
#include "gucefGEOMETRY_SRayCastResult.h"
#define GUCEF_GEOMETRY_SRAYCASTRESULT_H
#endif /* GUCEF_GEOMETRY_SRAYCASTRESULT_H ? */

#ifndef GUCEF_GEOMETRY_CTRAY3D_H
#include "gucefGEOMETRY_CTRay3D.h"
#define GUCEF_GEOMETRY_CTRAY3D_H
#endif /* GUCEF_GEOMETRY_CTRAY3D_H ? */

#ifndef GUCEF_GEOMETRY_CTTRIANGLE3D_H
#include "gucefGEOMETRY_CTTriangle3D.h"
#define GUCEF_GEOMETRY_CTTRIANGLE3D_H
#endif /* GUCEF_GEOMETRY_CTTRIANGLE3D_H ? */

#ifndef GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#include "gucefGEOMETRY_CTAxisAlignedBox3D.h"
#define GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H
#endif /* GUCEF_GEOMETRY_CTAXISALIGNEDBOX3D_H ? */

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#include "gucefGEOMETRY_CTGridTraversal2D.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H ? */

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
 *  A heightfield (section 7.2 of the design): a regular grid of heights over the XZ plane, the height along +Y (the up
 *  axis of the default convention). 'width' columns (x) and 'depth' rows (z) of vertices, row major: the height of
 *  vertex ( i, j ) is heights[ j * width + i ] and its position origin + ( i * cellSizeX, height, j * cellSizeZ ). Cell
 *  ( i, j ) between four vertices has two triangles split along the diagonal from vertex ( i, j ) to ( i + 1, j + 1 ):
 *      triangle 0: ( i, j ), ( i, j + 1 ), ( i + 1, j + 1 )      triangle 1: ( i, j ), ( i + 1, j + 1 ), ( i + 1, j )
 *  both facing +Y; triangle index = 2 * ( j * ( width - 1 ) + i ) + 0 or 1, the feature index of ray hits.
 *
 *  - The surface: GetHeight interpolates within the triangle with the barycentric weights of its vertices, so the
 *    height at a vertex is exactly its height. Outside the footprint there is no height (TryGetHeight false).
 *  - RayCast: the ray is clipped to the bounds, the cells it crosses (supercover traversal of CTGridTraversal2D) are
 *    visited in order and skipped by the minimum and maximum height of 8 by 8 blocks of cells against the height range
 *    of the rest of the ray; the closest hit of the triangle ray cast of CTTriangle3D wins. The result equals a ray cast
 *    against all triangles of the mesh built from the same heights.
 *  - ClosestPoint: the nearest point of the surface, by rings of cells around the point's cell with a lower bound per
 *    ring and per cell; equals the closest point over all triangles.
 *  - Heights: copied into the object (Set) or read from the caller's array (SetExternal: it must stay alive and unchanged
 *    while the heightfield is used; Refit after changing heights).
 *  - Kinds: float kinds. Fewer than two columns or rows is an empty heightfield that is never hit.
 *  - Realtime: the const queries are realtime-safe; Set, SetExternal and Refit allocate.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTHeightField
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;
    typedef MATH::CTPoint3D< T > TPoint;
    typedef MATH::CTVector3D< T > TVector;
    typedef MATH::CTDirectionVector3D< T > TDirection;
    typedef CTTriangle3D< T > TTriangle;

    enum { BlockSize = 8 };    /**< an enumerator: no definition is needed in C++98 */

    CTHeightField( void ) : m_owned(), m_heights( GUCEF_NULL ), m_width( 0 ), m_depth( 0 ), m_origin(), m_cellX( TTraits::One() ), m_cellZ( TTraits::One() ), m_blockMin(), m_blockMax(), m_blocksX( 0 ), m_blocksZ( 0 ), m_minHeight( TTraits::Zero() ), m_maxHeight( TTraits::Zero() ) {}

    /** Copies the heights; false (unchanged) for fewer than 2 columns or rows, a cell size that is not positive and finite, or an origin that is not finite */
    bool Set( const T* heights, UInt32 width, UInt32 depth, const TPoint& origin, const T& cellSizeX, const T& cellSizeZ )
    {
        if ( !Validate( heights, width, depth, origin, cellSizeX, cellSizeZ ) )
            return false;
        m_owned.assign( heights, heights + (size_t) width * depth );
        Assign( &m_owned[ 0 ], width, depth, origin, cellSizeX, cellSizeZ );
        return true;
    }

    /** As Set, reading the heights from the caller's array */
    bool SetExternal( const T* heights, UInt32 width, UInt32 depth, const TPoint& origin, const T& cellSizeX, const T& cellSizeZ )
    {
        if ( !Validate( heights, width, depth, origin, cellSizeX, cellSizeZ ) )
            return false;
        m_owned.clear();
        Assign( heights, width, depth, origin, cellSizeX, cellSizeZ );
        return true;
    }

    /** Recomputes the block minima and maxima after the heights were changed (for an external array) */
    void Refit( void )
    {
        if ( !IsEmpty() )
            BuildBlocks();
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool IsEmpty( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_width < 2 || m_depth < 2; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetWidth( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_width; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetDepth( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_depth; }
    GUCEF_GEOMETRY_FORCE_INLINE const TPoint& GetOrigin( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_origin; }
    GUCEF_GEOMETRY_FORCE_INLINE const T& GetCellSizeX( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_cellX; }
    GUCEF_GEOMETRY_FORCE_INLINE const T& GetCellSizeZ( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_cellZ; }
    GUCEF_GEOMETRY_FORCE_INLINE UInt32 GetTriangleCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return IsEmpty() ? 0 : 2 * ( m_width - 1 ) * ( m_depth - 1 ); }

    /** The height of vertex ( i, j ) (both in range) */
    GUCEF_GEOMETRY_FORCE_INLINE const T& GetVertexHeight( UInt32 i, UInt32 j ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_heights[ (size_t) j * m_width + i ]; }

    /** The position of vertex ( i, j ) */
    GUCEF_GEOMETRY_FORCE_INLINE TPoint GetVertex( UInt32 i, UInt32 j ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return TPoint( VertexX( i ), m_origin.y + GetVertexHeight( i, j ), VertexZ( j ) );
    }

    /** The two triangles of cell ( i, j ), see the class description; false for a cell out of range */
    bool GetCellTriangles( UInt32 i, UInt32 j, TTriangle& triangle0, TTriangle& triangle1 ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() || i + 1 >= m_width || j + 1 >= m_depth )
            return false;
        const TPoint v00 = GetVertex( i, j );
        const TPoint v01 = GetVertex( i, j + 1 );
        const TPoint v10 = GetVertex( i + 1, j );
        const TPoint v11 = GetVertex( i + 1, j + 1 );
        triangle0 = TTriangle( v00, v01, v11 );
        triangle1 = TTriangle( v00, v11, v10 );
        return true;
    }

    /** The lowest and the highest vertex height; both 0 when empty */
    GUCEF_GEOMETRY_FORCE_INLINE const T& GetMinHeight( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_minHeight; }
    GUCEF_GEOMETRY_FORCE_INLINE const T& GetMaxHeight( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_maxHeight; }

    /** The box around the surface; the empty box for an empty heightfield */
    CTAxisAlignedBox3D< T > GetBounds( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() )
            return CTAxisAlignedBox3D< T >();
        return CTAxisAlignedBox3D< T >( TPoint( m_origin.x, m_origin.y + m_minHeight, m_origin.z ), TPoint( VertexX( m_width - 1 ), m_origin.y + m_maxHeight, VertexZ( m_depth - 1 ) ) );
    }

    /** The height of the surface above ( x, z ) (the Y coordinate: the origin height included); false outside the footprint or when empty */
    bool TryGetHeight( const T& x, const T& z, T& height ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        UInt32 i, j;
        T u, w;
        if ( !Locate( x, z, i, j, u, w ) )
            return false;
        const T h00 = GetVertexHeight( i, j );
        const T h11 = GetVertexHeight( i + 1, j + 1 );
        if ( w > u )
            height = m_origin.y + ( ( h00 * ( TTraits::One() - w ) + GetVertexHeight( i, j + 1 ) * ( w - u ) ) + h11 * u );
        else
            height = m_origin.y + ( ( h00 * ( TTraits::One() - u ) + GetVertexHeight( i + 1, j ) * ( u - w ) ) + h11 * w );
        return true;
    }

    /** The normal of the triangle above ( x, z ); false outside the footprint or for a degenerate triangle */
    bool TryGetNormal( const T& x, const T& z, TDirection& normal ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        UInt32 i, j;
        T u, w;
        if ( !Locate( x, z, i, j, u, w ) )
            return false;
        TTriangle t0, t1;
        GetCellTriangles( i, j, t0, t1 );
        return w > u ? t0.TryGetNormal( normal ) : t1.TryGetNormal( normal );
    }

    /**
     *  The closest hit of the ray (within 'maxDistance') with the triangles of the surface, two sided; the feature index
     *  of the result is the triangle index of the class description.
     */
    bool RayCast( const CTRay3D< T >& ray, const T& maxDistance, SRayCastResult< T >& result ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() || !ray.origin.IsFinite() || !ray.direction.AsVector().IsFinite() || !( maxDistance >= TTraits::Zero() ) )
            return false;
        const T o[ 3 ] = { ray.origin.x, ray.origin.y, ray.origin.z };
        const T d[ 3 ] = { ray.direction.AsVector().x, ray.direction.AsVector().y, ray.direction.AsVector().z };
        const CTAxisAlignedBox3D< T > bounds = GetBounds();
        const T lo[ 3 ] = { bounds.min.x, bounds.min.y, bounds.min.z };
        const T hi[ 3 ] = { bounds.max.x, bounds.max.y, bounds.max.z };
        // the part of the ray inside the bounds
        T tEnter = TTraits::Zero();
        T tLeave = maxDistance;
        for ( UInt32 a=0; a<3; ++a )
        {
            if ( d[ a ] == TTraits::Zero() )
            {
                if ( o[ a ] < lo[ a ] || o[ a ] > hi[ a ] )
                    return false;
                continue;
            }
            T t1 = ( lo[ a ] - o[ a ] ) / d[ a ];
            T t2 = ( hi[ a ] - o[ a ] ) / d[ a ];
            if ( t2 < t1 )
            {
                const T swap = t1; t1 = t2; t2 = swap;
            }
            if ( t1 > tEnter ) tEnter = t1;
            if ( t2 < tLeave ) tLeave = t2;
        }
        if ( tEnter > tLeave )
            return false;

        // the traversal of the cells of the footprint
        const T x0 = m_origin.x, z0 = m_origin.z;
        const T x1 = VertexX( m_width - 1 ), z1 = VertexZ( m_depth - 1 );
        T sx = o[ 0 ] + d[ 0 ] * tEnter;
        T sz = o[ 2 ] + d[ 2 ] * tEnter;
        if ( sx < x0 ) sx = x0;
        if ( sx > x1 ) sx = x1;
        if ( sz < z0 ) sz = z0;
        if ( sz > z1 ) sz = z1;
        CTGridTraversal2D< T > traversal;
        traversal.SetCellBounds( MATH::CTPoint2D< Int32 >( 0, 0 ), MATH::CTPoint2D< Int32 >( (Int32) m_width - 2, (Int32) m_depth - 2 ) );
        if ( !traversal.Start( MATH::CTPoint2D< T >( x0, z0 ), MATH::CTVector2D< T >( m_cellX, m_cellZ ), MATH::CTPoint2D< T >( sx, sz ),
                               MATH::CTVector2D< T >( d[ 0 ], d[ 2 ] ), tLeave - tEnter, GRIDTRAVERSAL_SUPERCOVER ) )
            return false;

        const T yEnd = o[ 1 ] + d[ 1 ] * tLeave;
        bool found = false;
        SRayCastResult< T > best;
        MATH::CTPoint2D< Int32 > cell;
        T enter;
        while ( traversal.Next( cell, enter ) )
        {
            const T tCell = tEnter + enter;
            if ( found && tCell > best.distance )
                break;
            // the height range of the rest of the ray against the heights of the block of the cell
            const T yHere = o[ 1 ] + d[ 1 ] * tCell;
            const T yLow = yHere < yEnd ? yHere : yEnd;
            const T yHigh = yHere < yEnd ? yEnd : yHere;
            const UInt32 block = ( (UInt32) cell.y / BlockSize ) * m_blocksX + (UInt32) cell.x / BlockSize;
            const T slack = ( TTraits::Abs( yLow ) + TTraits::Abs( yHigh ) + TTraits::Abs( m_origin.y ) + TTraits::Abs( m_minHeight ) + TTraits::Abs( m_maxHeight ) ) * TTraits::Epsilon() * T( 16 );
            if ( yHigh < m_origin.y + m_blockMin[ block ] - slack || yLow > m_origin.y + m_blockMax[ block ] + slack )
                continue;
            TTriangle triangles[ 2 ];
            GetCellTriangles( (UInt32) cell.x, (UInt32) cell.y, triangles[ 0 ], triangles[ 1 ] );
            for ( UInt32 k=0; k<2; ++k )
            {
                SRayCastResult< T > hit;
                if ( triangles[ k ].RayCast( ray, maxDistance, hit ) && ( !found || hit.distance < best.distance ) )
                {
                    best = hit;
                    best.featureIndex = (Int32) ( 2 * ( (UInt32) cell.y * ( m_width - 1 ) + (UInt32) cell.x ) + k );
                    found = true;
                }
            }
        }
        if ( found )
            result = best;
        return found;
    }

    /** The nearest point of the surface to 'point' (the point itself when it lies on it); 'point' for an empty heightfield */
    TPoint ClosestPoint( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() || !point.IsFinite() )
            return point;
        const Int32 cellsX = (Int32) m_width - 1;
        const Int32 cellsZ = (Int32) m_depth - 1;
        const Int32 cx = CellIndex( point.x, m_origin.x, m_cellX, cellsX );
        const Int32 cz = CellIndex( point.z, m_origin.z, m_cellZ, cellsZ );
        const T minCell = m_cellX < m_cellZ ? m_cellX : m_cellZ;
        bool found = false;
        T bestSquared = TTraits::Zero();
        TPoint bestPoint = point;
        const Int32 maxRing = cellsX > cellsZ ? cellsX : cellsZ;
        for ( Int32 r=0; r<=maxRing; ++r )
        {
            if ( found && r > 1 )
            {
                // every cell of ring r is at least ( r - 1 ) cells away from the point in x or z
                const T gap = (T) ( r - 1 ) * minCell;
                if ( gap * gap > bestSquared )
                    break;
            }
            for ( Int32 j=cz-r; j<=cz+r; ++j )
            {
                if ( j < 0 || j >= cellsZ )
                    continue;
                const bool edgeRow = j == cz - r || j == cz + r;
                for ( Int32 i=cx-r; i<=cx+r; i += ( edgeRow || 0 == r ) ? 1 : 2 * r )
                {
                    if ( i < 0 || i >= cellsX )
                        continue;
                    if ( found && !CellMayBeCloser( (UInt32) i, (UInt32) j, point, bestSquared ) )
                        continue;
                    TTriangle t[ 2 ];
                    GetCellTriangles( (UInt32) i, (UInt32) j, t[ 0 ], t[ 1 ] );
                    for ( UInt32 k=0; k<2; ++k )
                    {
                        const TPoint c = t[ k ].ClosestPoint( point );
                        const T squared = ( point - c ).GetSquaredLength();
                        if ( !found || squared < bestSquared )
                        {
                            found = true;
                            bestSquared = squared;
                            bestPoint = c;
                        }
                    }
                }
            }
        }
        return bestPoint;
    }

    T GetSquaredDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetSquaredLength(); }
    T GetDistance( const TPoint& point ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return ( point - ClosestPoint( point ) ).GetLength(); }

    private:

    static bool Validate( const T* heights, UInt32 width, UInt32 depth, const TPoint& origin, const T& cellSizeX, const T& cellSizeZ )
    {
        if ( GUCEF_NULL == heights || width < 2 || depth < 2 || !origin.IsFinite() )
            return false;
        if ( !( cellSizeX > TTraits::Zero() ) || !( cellSizeZ > TTraits::Zero() ) || !TTraits::IsFinite( cellSizeX ) || !TTraits::IsFinite( cellSizeZ ) )
            return false;
        for ( size_t i=0; i<(size_t) width * depth; ++i )
            if ( !TTraits::IsFinite( heights[ i ] ) )
                return false;
        return true;
    }

    void Assign( const T* heights, UInt32 width, UInt32 depth, const TPoint& origin, const T& cellSizeX, const T& cellSizeZ )
    {
        m_heights = heights;
        m_width = width;
        m_depth = depth;
        m_origin = origin;
        m_cellX = cellSizeX;
        m_cellZ = cellSizeZ;
        BuildBlocks();
    }

    void BuildBlocks( void )
    {
        const UInt32 cellsX = m_width - 1;
        const UInt32 cellsZ = m_depth - 1;
        m_blocksX = ( cellsX + BlockSize - 1 ) / BlockSize;
        m_blocksZ = ( cellsZ + BlockSize - 1 ) / BlockSize;
        m_blockMin.assign( (size_t) m_blocksX * m_blocksZ, TTraits::Max() );
        m_blockMax.assign( (size_t) m_blocksX * m_blocksZ, TTraits::Lowest() );
        m_minHeight = m_maxHeight = m_heights[ 0 ];
        for ( UInt32 j=0; j<m_depth; ++j )
            for ( UInt32 i=0; i<m_width; ++i )
            {
                const T h = m_heights[ (size_t) j * m_width + i ];
                if ( h < m_minHeight ) m_minHeight = h;
                if ( h > m_maxHeight ) m_maxHeight = h;
                // the vertex belongs to the blocks of the up to four cells around it
                for ( UInt32 dj=0; dj<2; ++dj )
                    for ( UInt32 di=0; di<2; ++di )
                    {
                        if ( ( 1 == di && 0 == i ) || ( 1 == dj && 0 == j ) )
                            continue;
                        const UInt32 ci = i - di;
                        const UInt32 cj = j - dj;
                        if ( ci >= cellsX || cj >= cellsZ )
                            continue;
                        const size_t b = (size_t) ( cj / BlockSize ) * m_blocksX + ci / BlockSize;
                        if ( h < m_blockMin[ b ] ) m_blockMin[ b ] = h;
                        if ( h > m_blockMax[ b ] ) m_blockMax[ b ] = h;
                    }
            }
    }

    GUCEF_GEOMETRY_FORCE_INLINE T VertexX( UInt32 i ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_origin.x + (T) i * m_cellX; }
    GUCEF_GEOMETRY_FORCE_INLINE T VertexZ( UInt32 j ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return m_origin.z + (T) j * m_cellZ; }

    /** The cell of a coordinate along one axis as the vertex positions define it, clamped to 0 .. cells - 1 */
    Int32 CellIndex( const T& value, const T& origin, const T& size, Int32 cells ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const T f = TTraits::Floor( ( value - origin ) / size );
        if ( !( f > (T) 0 ) )
            return 0;
        if ( !( f < (T) ( cells - 1 ) ) )
            return cells - 1;
        return (Int32) f;
    }

    /** The cell and the position in it, 0 to 1 per axis; the far edge of the footprint belongs to the last cell */
    bool Locate( const T& x, const T& z, UInt32& i, UInt32& j, T& u, T& w ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( IsEmpty() || !( x >= m_origin.x ) || !( z >= m_origin.z ) || x > VertexX( m_width - 1 ) || z > VertexZ( m_depth - 1 ) )
            return false;
        Int32 ci = CellIndex( x, m_origin.x, m_cellX, (Int32) m_width - 1 );
        Int32 cj = CellIndex( z, m_origin.z, m_cellZ, (Int32) m_depth - 1 );
        // the vertex positions decide, not the quotient
        while ( ci > 0 && x < VertexX( (UInt32) ci ) )
            --ci;
        while ( ci + 1 < (Int32) m_width - 1 && !( x < VertexX( (UInt32) ci + 1 ) ) )
            ++ci;
        while ( cj > 0 && z < VertexZ( (UInt32) cj ) )
            --cj;
        while ( cj + 1 < (Int32) m_depth - 1 && !( z < VertexZ( (UInt32) cj + 1 ) ) )
            ++cj;
        i = (UInt32) ci;
        j = (UInt32) cj;
        const T xEnd = VertexX( i + 1 );
        const T zEnd = VertexZ( j + 1 );
        u = x >= xEnd ? TTraits::One() : ( x - VertexX( i ) ) / m_cellX;
        w = z >= zEnd ? TTraits::One() : ( z - VertexZ( j ) ) / m_cellZ;
        if ( u > TTraits::One() ) u = TTraits::One();
        if ( w > TTraits::One() ) w = TTraits::One();
        return true;
    }

    /** False when the box of the cell (its four heights) is further from the point than bestSquared */
    bool CellMayBeCloser( UInt32 i, UInt32 j, const TPoint& point, const T& bestSquared ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T low = GetVertexHeight( i, j );
        T high = low;
        const T others[ 3 ] = { GetVertexHeight( i + 1, j ), GetVertexHeight( i, j + 1 ), GetVertexHeight( i + 1, j + 1 ) };
        for ( UInt32 k=0; k<3; ++k )
        {
            if ( others[ k ] < low ) low = others[ k ];
            if ( others[ k ] > high ) high = others[ k ];
        }
        const T boxLo[ 3 ] = { VertexX( i ), m_origin.y + low, VertexZ( j ) };
        const T boxHi[ 3 ] = { VertexX( i + 1 ), m_origin.y + high, VertexZ( j + 1 ) };
        const T p[ 3 ] = { point.x, point.y, point.z };
        T sum = TTraits::Zero();
        for ( UInt32 a=0; a<3; ++a )
        {
            T excess = TTraits::Zero();
            if ( p[ a ] < boxLo[ a ] )
                excess = boxLo[ a ] - p[ a ];
            else if ( boxHi[ a ] < p[ a ] )
                excess = p[ a ] - boxHi[ a ];
            sum = sum + excess * excess;
        }
        // a little slack: the box and the triangles are computed separately
        return !( sum > bestSquared * ( TTraits::One() + TTraits::Epsilon() * T( 64 ) ) + TTraits::Epsilon() * T( 64 ) * ( TTraits::Abs( point.x ) + TTraits::Abs( point.z ) + TTraits::Abs( point.y ) ) * ( TTraits::Abs( point.x ) + TTraits::Abs( point.z ) + TTraits::Abs( point.y ) ) );
    }

    GUCEF::vector< T > m_owned;
    const T* m_heights;
    UInt32 m_width;
    UInt32 m_depth;
    TPoint m_origin;
    T m_cellX;
    T m_cellZ;
    GUCEF::vector< T > m_blockMin;
    GUCEF::vector< T > m_blockMax;
    UInt32 m_blocksX;
    UInt32 m_blocksZ;
    T m_minHeight;
    T m_maxHeight;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTHeightField< Float32 > TFloat32HeightField;
typedef CTHeightField< Float64 > TFloat64HeightField;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTHEIGHTFIELD_H ? */
