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

#ifndef GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H
#define GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_GEOMETRY_MACROS_H
#include "gucefGEOMETRY_macros.h"
#define GUCEF_GEOMETRY_MACROS_H
#endif /* GUCEF_GEOMETRY_MACROS_H ? */

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
 *  The integer line iterator of Bresenham for all eight octants (section 7.8.6 of the design): the cells from 'from' to
 *  'to', both included, one per step of the major axis (the longer of the two coordinate differences). The error term
 *  is the symmetric integer form: with dx = |x1 - x0| and dy = -|y1 - y0|, err = dx + dy and for each cell e2 = 2 err; the
 *  x step is taken when e2 >= dy, the y step when e2 <= dx (both on a diagonal; a tie of the ideal line between two cells
 *  goes to the cell that takes the minor step). The line from a to b visits the same cells as the line from b to a
 *  except at such ties.
 *
 *  - Coordinates: Int32; the arithmetic is Int64, so the whole Int32 range works.
 *  - Realtime: every member is realtime-safe; no allocation.
 */
class CTBresenhamLine2D
{
    public:

    typedef MATH::CTPoint2D< Int32 > TCell;

    CTBresenhamLine2D( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT
        : m_x( 0 ), m_y( 0 ), m_x1( 0 ), m_y1( 0 ), m_dx( 0 ), m_dy( 0 ), m_sx( 1 ), m_sy( 1 ), m_err( 0 ), m_done( true )
    {
    }

    CTBresenhamLine2D( const TCell& from, const TCell& to ) GUCEF_GEOMETRY_REALTIME_CONTEXT
        : m_x( 0 ), m_y( 0 ), m_x1( 0 ), m_y1( 0 ), m_dx( 0 ), m_dy( 0 ), m_sx( 1 ), m_sy( 1 ), m_err( 0 ), m_done( true )
    {
        Start( from, to );
    }

    /** Restarts the iterator on the line from 'from' to 'to' */
    void Start( const TCell& from, const TCell& to ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        m_x = from.x;
        m_y = from.y;
        m_x1 = to.x;
        m_y1 = to.y;
        const Int64 ax = (Int64) to.x - (Int64) from.x;
        const Int64 ay = (Int64) to.y - (Int64) from.y;
        m_dx = ax < 0 ? -ax : ax;
        m_dy = ay < 0 ? ay : -ay;
        m_sx = ax < 0 ? -1 : 1;
        m_sy = ay < 0 ? -1 : 1;
        m_err = m_dx + m_dy;
        m_done = false;
    }

    /** The number of cells of the line, both ends included */
    GUCEF_GEOMETRY_FORCE_INLINE UInt64 GetCellCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const Int64 major = m_dx > -m_dy ? m_dx : -m_dy;
        return (UInt64) major + 1;
    }

    /** The next cell; false after the last one */
    bool Next( TCell& cell ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( m_done )
            return false;
        cell.x = (Int32) m_x;
        cell.y = (Int32) m_y;
        if ( m_x == m_x1 && m_y == m_y1 )
        {
            m_done = true;
            return true;
        }
        const Int64 e2 = 2 * m_err;
        if ( e2 >= m_dy )
        {
            m_err += m_dy;
            m_x += m_sx;
        }
        if ( e2 <= m_dx )
        {
            m_err += m_dx;
            m_y += m_sy;
        }
        return true;
    }

    private:

    Int64 m_x;
    Int64 m_y;
    Int64 m_x1;
    Int64 m_y1;
    Int64 m_dx;
    Int64 m_dy;
    Int64 m_sx;
    Int64 m_sy;
    Int64 m_err;
    bool m_done;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace GEOMETRY */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_GEOMETRY_CTBRESENHAMLINE2D_H ? */
