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

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H
#define GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

#ifndef GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H
#include "gucefGEOMETRY_CTGridTraversalCore.h"
#define GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H
#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSALCORE_H ? */

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
namespace GEOMETRY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The cells of a regular 2D grid that a segment or a ray passes through, in the order along the segment (the
 *  Amanatides-Woo DDA, section 7.8.6 of the design; the rules, the standard and the supercover mode are those of
 *  CTGridTraversalCore). Cell ( i, ... ) covers [ origin + i * cellSize, origin + ( i + 1 ) * cellSize ) per axis.
 *  Used by the uniform grid, the spatial hash and the heightfield, and usable for tile maps.
 *
 *  - Kinds: Float32 and Float64. Layout: a fixed size state, no heap.
 *  - Use: Start...( ... ), then Next( cell ) until it returns false. A traversal without cell bounds of a ray without
 *    a maximum distance does not end: set cell bounds or stop in the caller.
 *  - Realtime: every member is realtime-safe.
 */
template < typename T >
class GUCEF_GEOMETRY_TEMPLATE_PUBLIC CTGridTraversal2D
{
    public:

    typedef T TValue;
    typedef MATH::CTPoint2D< T > TPoint;
    typedef MATH::CTVector2D< T > TVector;
    typedef MATH::CTPoint2D< Int32 > TCell;

    /** Limits the traversal to the cells from 'cellMin' to 'cellMax' inclusive; call before Start */
    GUCEF_GEOMETRY_FORCE_INLINE void SetCellBounds( const TCell& cellMin, const TCell& cellMax ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        Int32 low[ 2 ];
        Int32 high[ 2 ];
        ToArray( cellMin, low );
        ToArray( cellMax, high );
        m_core.SetCellBounds( low, high );
    }

    GUCEF_GEOMETRY_FORCE_INLINE void ClearCellBounds( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT { m_core.ClearCellBounds(); }

    /** Starts at 'start' with the displacement 'delta' per unit of the parameter, up to the parameter 'tEnd' (1 for the whole segment); false when there is nothing to visit (see CTGridTraversalCore::Start) */
    bool Start( const TPoint& gridOrigin, const TVector& cellSize, const TPoint& start, const TVector& delta, const T& tEnd, TGridTraversalMode mode = GRIDTRAVERSAL_STANDARD ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T o[ 2 ], c[ 2 ], s[ 2 ], d[ 2 ];
        ToArray( gridOrigin, o );
        ToArray( cellSize, c );
        ToArray( start, s );
        ToArray( delta, d );
        return m_core.Start( o, c, s, d, tEnd, mode );
    }

    /** The cells of the segment from 'from' to 'to' */
    GUCEF_GEOMETRY_FORCE_INLINE bool StartSegment( const TPoint& gridOrigin, const TVector& cellSize, const TPoint& from, const TPoint& to, TGridTraversalMode mode = GRIDTRAVERSAL_STANDARD ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Start( gridOrigin, cellSize, from, to - from, MATH::CTNumericTraits< T >::One(), mode );
    }

    /** The cells of the ray from 'origin' along 'direction' (a direction of any length: the parameter is then the distance in units of its length) up to the parameter 'maxDistance' */
    GUCEF_GEOMETRY_FORCE_INLINE bool StartRay( const TPoint& gridOrigin, const TVector& cellSize, const TPoint& origin, const TVector& direction, const T& maxDistance, TGridTraversalMode mode = GRIDTRAVERSAL_STANDARD ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        return Start( gridOrigin, cellSize, origin, direction, maxDistance, mode );
    }

    /** The next cell; false at the end */
    GUCEF_GEOMETRY_FORCE_INLINE bool Next( TCell& cell ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        T ignored;
        return Next( cell, ignored );
    }

    /** The next cell and the parameter where the segment enters it (0 for the first cell) */
    bool Next( TCell& cell, T& enterParameter ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        Int32 c[ 2 ];
        if ( !m_core.Next( c, &enterParameter ) )
            return false;
        FromArray( c, cell );
        return true;
    }

    private:

    template < typename TObject >
    static GUCEF_GEOMETRY_FORCE_INLINE void ToArray( const TObject& value, typename TObject::TValue* out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        out[ 0 ] = value.x;
        out[ 1 ] = value.y;
    }

    static GUCEF_GEOMETRY_FORCE_INLINE void FromArray( const Int32* in, TCell& out ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        out.x = in[ 0 ];
        out.y = in[ 1 ];
    }

    CTGridTraversalCore< T, 2 > m_core;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef CTGridTraversal2D< Float32 > TFloat32GridTraversal2D;
typedef CTGridTraversal2D< Float64 > TFloat64GridTraversal2D;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTGRIDTRAVERSAL2D_H ? */
