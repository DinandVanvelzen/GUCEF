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

#ifndef GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H
#define GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H

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
 *  The integer line iterator of Bresenham in 3D (section 7.8.6 of the design): the cells from 'from' to 'to', both
 *  included, one per step of the major axis (the longest of the three coordinate differences dm). Each axis has an
 *  error term that starts at dm / 2; per cell it is reduced by the axis difference and, when it goes below zero, dm is
 *  added and the axis takes its step. In two dimensions (one axis difference 0) this is the line of CTBresenhamLine2D
 *  up to ties of the ideal line between two cells.
 *
 *  - Coordinates: Int32; the arithmetic is Int64, so the whole Int32 range works.
 *  - Realtime: every member is realtime-safe; no allocation.
 */
class CTBresenhamLine3D
{
    public:

    typedef MATH::CTPoint3D< Int32 > TCell;

    CTBresenhamLine3D( void ) GUCEF_GEOMETRY_REALTIME_CONTEXT : m_remaining( 0 ), m_dm( 0 ), m_done( true )
    {
        for ( UInt32 a=0; a<3; ++a )
            m_position[ a ] = m_difference[ a ] = m_step[ a ] = m_error[ a ] = 0;
    }

    CTBresenhamLine3D( const TCell& from, const TCell& to ) GUCEF_GEOMETRY_REALTIME_CONTEXT : m_remaining( 0 ), m_dm( 0 ), m_done( true )
    {
        for ( UInt32 a=0; a<3; ++a )
            m_position[ a ] = m_difference[ a ] = m_step[ a ] = m_error[ a ] = 0;
        Start( from, to );
    }

    /** Restarts the iterator on the line from 'from' to 'to' */
    void Start( const TCell& from, const TCell& to ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        const Int64 a[ 3 ] = { (Int64) from.x, (Int64) from.y, (Int64) from.z };
        const Int64 b[ 3 ] = { (Int64) to.x, (Int64) to.y, (Int64) to.z };
        m_dm = 0;
        for ( UInt32 i=0; i<3; ++i )
        {
            const Int64 d = b[ i ] - a[ i ];
            m_position[ i ] = a[ i ];
            m_difference[ i ] = d < 0 ? -d : d;
            m_step[ i ] = d < 0 ? -1 : 1;
            if ( m_difference[ i ] > m_dm )
                m_dm = m_difference[ i ];
        }
        for ( UInt32 i=0; i<3; ++i )
            m_error[ i ] = m_dm / 2;
        m_remaining = m_dm;
        m_done = false;
    }

    /** The number of cells of the line, both ends included */
    GUCEF_GEOMETRY_FORCE_INLINE UInt64 GetCellCount( void ) const GUCEF_GEOMETRY_REALTIME_CONTEXT { return (UInt64) m_dm + 1; }

    /** The next cell; false after the last one */
    bool Next( TCell& cell ) GUCEF_GEOMETRY_REALTIME_CONTEXT
    {
        if ( m_done )
            return false;
        cell.x = (Int32) m_position[ 0 ];
        cell.y = (Int32) m_position[ 1 ];
        cell.z = (Int32) m_position[ 2 ];
        if ( 0 == m_remaining )
        {
            m_done = true;
            return true;
        }
        --m_remaining;
        for ( UInt32 i=0; i<3; ++i )
        {
            m_error[ i ] -= m_difference[ i ];
            if ( m_error[ i ] < 0 )
            {
                m_error[ i ] += m_dm;
                m_position[ i ] += m_step[ i ];
            }
        }
        return true;
    }

    private:

    Int64 m_position[ 3 ];
    Int64 m_difference[ 3 ];
    Int64 m_step[ 3 ];
    Int64 m_error[ 3 ];
    Int64 m_remaining;
    Int64 m_dm;
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

#endif /* GUCEF_GEOMETRY_CTBRESENHAMLINE3D_H ? */
