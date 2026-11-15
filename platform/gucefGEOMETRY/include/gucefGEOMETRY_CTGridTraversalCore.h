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

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H
#define GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_MATH_CTNUMERICTRAITS_H
#include "gucefMATH_CTNumericTraits.h"
#define GUCEF_MATH_CTNUMERICTRAITS_H
#endif /* GUCEF_MATH_CTNUMERICTRAITS_H ? */

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

/** Which cells a grid traversal reports at a cell corner, an edge and the ends of the segment */
enum EGridTraversalMode
{
    GRIDTRAVERSAL_STANDARD   = 0 ,      /**< the cells the segment passes through with a length above zero; at an exact corner crossing the diagonal cell only */
    GRIDTRAVERSAL_SUPERCOVER = 1        /**< every cell whose closed square touches the segment: at an exact corner crossing the neighbours too */
};
typedef enum EGridTraversalMode TGridTraversalMode;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The shared implementation of CTGridTraversal2D and CTGridTraversal3D (section 7.8.6 of the design): the Amanatides-Woo
 *  DDA over the cells of a regular grid, for Dim axes (2 or 3). Cell i of an axis covers [ origin + i * size, origin +
 *  ( i + 1 ) * size ). The segment is start + t * delta for t from 0 to tEnd (a ray: delta the direction, tEnd the
 *  maximum distance).
 *
 *  - Modes: see EGridTraversalMode. The standard mode is the set of cells the segment has a positive length inside
 *    (the cell of a segment of zero length is the one that contains the point); a segment that lies on a grid line
 *    belongs to the cells on the positive side of the line. The supercover mode adds every cell whose closed square
 *    touches the segment, including the ones behind a start point on a grid line and those meeting an exact corner
 *    crossing (reported in the order of the bit mask of the axes they step: x, y, then both).
 *  - Exactness: the parameter of each boundary crossing is computed directly from the boundary ( boundary - start ) /
 *    delta, never accumulated, so equal crossing parameters (a ray through a cell corner) compare equal whenever the
 *    inputs are exactly representable and the quotients are exact or correctly rounded to the same value; distinct
 *    crossings that the kind cannot tell apart are reported as a corner.
 *  - Kinds: Float32 and Float64.
 *  - Realtime: Start and Next are realtime-safe; no allocation, the state is a fixed size array.
 */
template < typename T, UInt32 Dim >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGridTraversalCore
{
    public:

    typedef T TValue;
    typedef MATH::CTNumericTraits< T > TTraits;

    CTGridTraversalCore( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
        : m_tEnd( TTraits::Zero() ), m_supercover( false ), m_done( true ), m_firstPending( false ), m_pendingCount( 0 ), m_pendingNext( 0 ), m_hasBounds( false ), m_lineAxisCount( 0 ), m_variantCount( 0 ), m_variantNext( 0 ), m_variantParameter( TTraits::Zero() )
    {
        for ( UInt32 a=0; a<Dim; ++a )
        {
            m_origin[ a ] = m_size[ a ] = m_start[ a ] = m_delta[ a ] = m_tMax[ a ] = TTraits::Zero();
            m_cell[ a ] = m_step[ a ] = m_boundsMin[ a ] = m_boundsMax[ a ] = 0;
        }
    }

    /** Limits the traversal to the cells from 'cellMin' to 'cellMax' inclusive (one Int32 per axis); it ends when a step leaves them. Call before Start. */
    void SetCellBounds( const Int32* cellMin, const Int32* cellMax ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        for ( UInt32 a=0; a<Dim; ++a )
        {
            m_boundsMin[ a ] = cellMin[ a ];
            m_boundsMax[ a ] = cellMax[ a ];
        }
        m_hasBounds = true;
    }

    void ClearCellBounds( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT { m_hasBounds = false; }

    /**
     *  Starts a traversal. False (nothing to visit) when a number is not finite, a cell size is not positive, tEnd is
     *  negative, not a number or infinite without cell bounds, the start cell is outside the bounds or a cell index does
     *  not fit 30 bits.
     */
    bool Start( const T* origin, const T* cellSize, const T* start, const T* delta, const T& tEnd, TGridTraversalMode mode ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    /** The next cell into 'cell' (Dim values); false at the end. 'enterParameter' (optional) receives t where the segment enters the cell */
    bool Next( Int32* cell, T* enterParameter ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    private:

    /** The cells of the DDA itself, without the cells beside a segment that runs along a grid line */
    bool NextBase( Int32* cell, T* enterParameter ) GUCEF_GEOMETRY_REALTIME_CONTEXT;

    public:

    private:

    enum { MaxPending = 8 };    /**< an enumerator: no definition is needed in C++98 */

    GUCEF_GEOMETRY_FORCE_INLINE T Boundary( UInt32 axis, Int32 index ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return m_origin[ axis ] + (T) index * m_size[ axis ];
    }

    /** The parameter where the segment reaches the next boundary of 'axis' after the cell m_cell */
    GUCEF_GEOMETRY_FORCE_INLINE T NextCrossing( UInt32 axis ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const T boundary = Boundary( axis, m_step[ axis ] > 0 ? m_cell[ axis ] + 1 : m_cell[ axis ] );
        return ( boundary - m_start[ axis ] ) / m_delta[ axis ];
    }

    GUCEF_GEOMETRY_FORCE_INLINE bool InBounds( const Int32* cell ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( !m_hasBounds )
            return true;
        for ( UInt32 a=0; a<Dim; ++a )
            if ( cell[ a ] < m_boundsMin[ a ] || cell[ a ] > m_boundsMax[ a ] )
                return false;
        return true;
    }

    void PushPending( const Int32* cell, const T& parameter ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( m_pendingCount < MaxPending && InBounds( cell ) )
        {
            for ( UInt32 a=0; a<Dim; ++a )
                m_pending[ m_pendingCount ][ a ] = cell[ a ];
            m_pendingParameter[ m_pendingCount ] = parameter;
            ++m_pendingCount;
        }
    }

    T m_origin[ Dim ];
    T m_size[ Dim ];
    T m_start[ Dim ];
    T m_delta[ Dim ];
    T m_tMax[ Dim ];                    /**< the parameter of the next boundary crossing per axis, valid where the step is not 0 */
    T m_tEnd;
    Int32 m_cell[ Dim ];
    Int32 m_step[ Dim ];
    Int32 m_boundsMin[ Dim ];
    Int32 m_boundsMax[ Dim ];
    Int32 m_pending[ MaxPending ][ Dim ];
    T m_pendingParameter[ MaxPending ];
    bool m_supercover;
    bool m_done;
    bool m_firstPending;                /**< the start cell was not returned yet */
    UInt32 m_pendingCount;
    UInt32 m_pendingNext;
    bool m_hasBounds;
    UInt32 m_lineAxes[ Dim ];           /**< supercover: the axes along which the segment does not move and lies on a grid line: the cells on both sides belong to it */
    UInt32 m_lineAxisCount;
    Int32 m_variant[ MaxPending ][ Dim ];
    UInt32 m_variantCount;
    UInt32 m_variantNext;
    T m_variantParameter;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

template < typename T, UInt32 Dim >
bool
CTGridTraversalCore< T, Dim >::Start( const T* origin, const T* cellSize, const T* start, const T* delta, const T& tEnd, TGridTraversalMode mode ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    m_done = true;
    m_pendingCount = 0;
    m_pendingNext = 0;
    m_firstPending = false;
    m_lineAxisCount = 0;
    m_variantCount = 0;
    m_variantNext = 0;
    m_supercover = GRIDTRAVERSAL_SUPERCOVER == mode;
    if ( !( tEnd >= TTraits::Zero() ) || ( !TTraits::IsFinite( tEnd ) && !m_hasBounds ) )
        return false;
    m_tEnd = tEnd;
    const T zero = TTraits::Zero();
    bool onBoundary[ Dim ];
    for ( UInt32 a=0; a<Dim; ++a )
    {
        if ( !TTraits::IsFinite( origin[ a ] ) || !TTraits::IsFinite( start[ a ] ) || !TTraits::IsFinite( delta[ a ] ) || !( cellSize[ a ] > zero ) || !TTraits::IsFinite( cellSize[ a ] ) )
            return false;
        m_origin[ a ] = origin[ a ];
        m_size[ a ] = cellSize[ a ];
        m_start[ a ] = start[ a ];
        m_delta[ a ] = delta[ a ];
        const T f = TTraits::Floor( ( start[ a ] - origin[ a ] ) / cellSize[ a ] );
        if ( !( f > (T) -1.0e9 && f < (T) 1.0e9 ) )
            return false;
        Int32 c = (Int32) f;
        // the cell as the boundaries compute it: the quotient above can be rounded across a boundary
        while ( m_start[ a ] < Boundary( a, c ) )
            --c;
        while ( !( m_start[ a ] < Boundary( a, c + 1 ) ) )
            ++c;
        onBoundary[ a ] = m_start[ a ] == Boundary( a, c );
        if ( delta[ a ] < zero && onBoundary[ a ] )
            --c;
        m_cell[ a ] = c;
        m_step[ a ] = delta[ a ] > zero ? 1 : ( delta[ a ] < zero ? -1 : 0 );
    }
    if ( !InBounds( m_cell ) )
        return false;
    for ( UInt32 a=0; a<Dim; ++a )
        m_tMax[ a ] = 0 != m_step[ a ] ? NextCrossing( a ) : zero;

    m_done = false;
    m_firstPending = true;
    if ( m_supercover )
    {
        // the cells behind the start point when it lies on grid lines: every non empty subset of those axes, flipped
        UInt32 axes[ Dim ];
        UInt32 axisCount = 0;
        for ( UInt32 a=0; a<Dim; ++a )
        {
            if ( onBoundary[ a ] && 0 == m_step[ a ] )
                m_lineAxes[ m_lineAxisCount++ ] = a;
            else if ( onBoundary[ a ] )
                axes[ axisCount++ ] = a;
        }
        for ( UInt32 mask=1; mask<( (UInt32) 1 << axisCount ); ++mask )
        {
            Int32 cell[ Dim ];
            for ( UInt32 a=0; a<Dim; ++a )
                cell[ a ] = m_cell[ a ];
            for ( UInt32 k=0; k<axisCount; ++k )
                if ( 0 != ( mask & ( (UInt32) 1 << k ) ) )
                {
                    const UInt32 a = axes[ k ];
                    // the cell is the positive side of the line unless the segment moves down from it
                    cell[ a ] += m_delta[ a ] < zero ? 1 : -1;
                }
            PushPending( cell, zero );
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Dim >
bool
CTGridTraversalCore< T, Dim >::Next( Int32* cell, T* enterParameter ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_variantNext < m_variantCount )
    {
        for ( UInt32 a=0; a<Dim; ++a )
            cell[ a ] = m_variant[ m_variantNext ][ a ];
        if ( GUCEF_NULL != enterParameter )
            *enterParameter = m_variantParameter;
        ++m_variantNext;
        return true;
    }
    if ( !NextBase( cell, enterParameter ) )
        return false;
    // a segment that runs along a grid line also touches the cells on the other side of it, along its whole length
    m_variantCount = 0;
    m_variantNext = 0;
    if ( m_lineAxisCount > 0 )
    {
        m_variantParameter = GUCEF_NULL != enterParameter ? *enterParameter : TTraits::Zero();
        for ( UInt32 mask=1; mask<( (UInt32) 1 << m_lineAxisCount ); ++mask )
        {
            Int32 variant[ Dim ];
            for ( UInt32 a=0; a<Dim; ++a )
                variant[ a ] = cell[ a ];
            for ( UInt32 k=0; k<m_lineAxisCount; ++k )
                if ( 0 != ( mask & ( (UInt32) 1 << k ) ) )
                    --variant[ m_lineAxes[ k ] ];
            if ( InBounds( variant ) )
            {
                for ( UInt32 a=0; a<Dim; ++a )
                    m_variant[ m_variantCount ][ a ] = variant[ a ];
                ++m_variantCount;
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

template < typename T, UInt32 Dim >
bool
CTGridTraversalCore< T, Dim >::NextBase( Int32* cell, T* enterParameter ) GUCEF_GEOMETRY_REALTIME_CONTEXT
{
    if ( m_firstPending )
    {
        m_firstPending = false;
        for ( UInt32 a=0; a<Dim; ++a )
            cell[ a ] = m_cell[ a ];
        if ( GUCEF_NULL != enterParameter )
            *enterParameter = TTraits::Zero();
        return true;
    }
    for ( ;; )
    {
        if ( m_pendingNext < m_pendingCount )
        {
            for ( UInt32 a=0; a<Dim; ++a )
                cell[ a ] = m_pending[ m_pendingNext ][ a ];
            if ( GUCEF_NULL != enterParameter )
                *enterParameter = m_pendingParameter[ m_pendingNext ];
            ++m_pendingNext;
            return true;
        }
        m_pendingCount = 0;
        m_pendingNext = 0;
        if ( m_done )
            return false;

        // the nearest boundary crossing
        T tMin = TTraits::Zero();
        bool any = false;
        for ( UInt32 a=0; a<Dim; ++a )
            if ( 0 != m_step[ a ] && ( !any || m_tMax[ a ] < tMin ) )
            {
                tMin = m_tMax[ a ];
                any = true;
            }
        if ( !any || !( m_supercover ? tMin <= m_tEnd : tMin < m_tEnd ) )
        {
            m_done = true;
            return false;
        }
        UInt32 tied[ Dim ];
        UInt32 tiedCount = 0;
        for ( UInt32 a=0; a<Dim; ++a )
            if ( 0 != m_step[ a ] && m_tMax[ a ] == tMin )
                tied[ tiedCount++ ] = a;

        if ( m_supercover )
        {
            // every non empty subset of the tied axes, the full one last; it becomes the current cell
            for ( UInt32 mask=1; mask<( (UInt32) 1 << tiedCount ); ++mask )
            {
                Int32 next[ Dim ];
                for ( UInt32 a=0; a<Dim; ++a )
                    next[ a ] = m_cell[ a ];
                for ( UInt32 k=0; k<tiedCount; ++k )
                    if ( 0 != ( mask & ( (UInt32) 1 << k ) ) )
                        next[ tied[ k ] ] += m_step[ tied[ k ] ];
                PushPending( next, tMin );
            }
        }
        for ( UInt32 k=0; k<tiedCount; ++k )
            m_cell[ tied[ k ] ] += m_step[ tied[ k ] ];
        for ( UInt32 k=0; k<tiedCount; ++k )
            m_tMax[ tied[ k ] ] = NextCrossing( tied[ k ] );
        if ( !InBounds( m_cell ) )
            m_done = true;
        else if ( !m_supercover )
        {
            for ( UInt32 a=0; a<Dim; ++a )
                cell[ a ] = m_cell[ a ];
            if ( GUCEF_NULL != enterParameter )
                *enterParameter = tMin;
            return true;
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H ? */
