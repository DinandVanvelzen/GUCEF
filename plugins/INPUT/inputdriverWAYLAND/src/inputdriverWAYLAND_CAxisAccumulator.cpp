/*
 *  inputdriverWAYLAND: GUCEF input driver for Wayland: the pointer and keyboard of the seat for the windows of guidriverWAYLAND
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

#include "inputdriverWAYLAND_CAxisAccumulator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace INPUT {
namespace WAYLAND {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAxisAccumulator::CAxisAccumulator( void )
    : m_unitsPerNotch( 15.0 )
{GUCEF_TRACE;

    Reset();
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::SetUnitsPerNotch( Float64 units )
{GUCEF_TRACE;

    if ( units > 0.0 )
        m_unitsPerNotch = units;
}

/*-------------------------------------------------------------------------*/

Float64
CAxisAccumulator::GetUnitsPerNotch( void ) const
{GUCEF_TRACE;

    return m_unitsPerNotch;
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::Reset( void )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<AXIS_COUNT; ++i )
    {
        m_remainder[ i ] = 0.0;
        m_axisValue[ i ] = 0.0;
        m_discrete[ i ] = 0;
        m_value120[ i ] = 0;
        m_direction[ i ] = 0;
        m_hasAxisValue[ i ] = false;
        m_hasDiscrete[ i ] = false;
        m_hasValue120[ i ] = false;
        m_stopped[ i ] = false;
    }
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::OnAxis( UInt32 axis, Float64 value )
{GUCEF_TRACE;

    if ( axis >= AXIS_COUNT )
        return;
    // a value that is not a number (it cannot come from the fixed point numbers of the protocol) must not poison the running sum
    if ( value != value )
        return;
    m_axisValue[ axis ] += value;
    m_hasAxisValue[ axis ] = true;
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::OnAxisSource( UInt32 source )
{GUCEF_TRACE;

    // what the source is does not change the arithmetic: the best value that came decides
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::OnAxisStop( UInt32 axis )
{GUCEF_TRACE;

    if ( axis < AXIS_COUNT )
        m_stopped[ axis ] = true;
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::OnAxisRelativeDirection( UInt32 axis, UInt32 direction )
{GUCEF_TRACE;

    if ( axis < AXIS_COUNT )
        m_direction[ axis ] = direction;
}

/*-------------------------------------------------------------------------*/

UInt32
CAxisAccumulator::GetRelativeDirection( UInt32 axis ) const
{GUCEF_TRACE;

    return axis < AXIS_COUNT ? m_direction[ axis ] : 0;
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::OnAxisDiscrete( UInt32 axis, Int32 discrete )
{GUCEF_TRACE;

    if ( axis >= AXIS_COUNT )
        return;
    m_discrete[ axis ] += (Int64) discrete;
    m_hasDiscrete[ axis ] = true;
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::OnAxisValue120( UInt32 axis, Int32 value120 )
{GUCEF_TRACE;

    if ( axis >= AXIS_COUNT )
        return;
    m_value120[ axis ] += (Int64) value120;
    m_hasValue120[ axis ] = true;
}

/*-------------------------------------------------------------------------*/

void
CAxisAccumulator::Accumulate( UInt32 axis, Float64 notches )
{GUCEF_TRACE;

    // a scroll that turns around leaves the part of a notch of the other direction behind
    if ( ( notches > 0.0 && m_remainder[ axis ] < 0.0 ) || ( notches < 0.0 && m_remainder[ axis ] > 0.0 ) )
        m_remainder[ axis ] = 0.0;
    m_remainder[ axis ] += notches;
}

/*-------------------------------------------------------------------------*/

bool
CAxisAccumulator::Flush( Int32& vertical, Int32& horizontal )
{GUCEF_TRACE;

    Int32 whole[ AXIS_COUNT ] = { 0, 0 };
    for ( UInt32 axis=0; axis<AXIS_COUNT; ++axis )
    {
        // the best of what the group has; the others describe the same movement
        if ( m_hasValue120[ axis ] )
            Accumulate( axis, m_value120[ axis ] / 120.0 );
        else
        if ( m_hasDiscrete[ axis ] )
            Accumulate( axis, (Float64) m_discrete[ axis ] );
        else
        if ( m_hasAxisValue[ axis ] )
            Accumulate( axis, m_axisValue[ axis ] / m_unitsPerNotch );

        // the part of a notch that is due; what is left stays, except after the end of a scroll
        // (truncates towards zero; the small bias keeps three thirds of a notch from adding up to just under one)
        Float64 wholeNotches = m_remainder[ axis ] + ( m_remainder[ axis ] >= 0.0 ? 1e-6 : -1e-6 );
        if ( wholeNotches > (Float64) MaxNotchesPerGroup )
            wholeNotches = (Float64) MaxNotchesPerGroup;
        else
        if ( wholeNotches < -(Float64) MaxNotchesPerGroup )
            wholeNotches = -(Float64) MaxNotchesPerGroup;
        const Int32 due = (Int32) wholeNotches;
        m_remainder[ axis ] -= due;
        // what was cut off is not owed to the next group
        if ( m_remainder[ axis ] > (Float64) MaxNotchesPerGroup || m_remainder[ axis ] < -(Float64) MaxNotchesPerGroup )
            m_remainder[ axis ] = 0.0;
        if ( m_stopped[ axis ] )
            m_remainder[ axis ] = 0.0;
        whole[ axis ] = due;

        m_axisValue[ axis ] = 0.0;
        m_discrete[ axis ] = 0;
        m_value120[ axis ] = 0;
        m_hasAxisValue[ axis ] = false;
        m_hasDiscrete[ axis ] = false;
        m_hasValue120[ axis ] = false;
        m_stopped[ axis ] = false;
    }

    // Wayland counts the vertical axis down, gucefINPUT counts a wheel turned away from the user (up) as positive
    vertical = -whole[ AXIS_VERTICAL ];
    horizontal = whole[ AXIS_HORIZONTAL ];
    return 0 != vertical || 0 != horizontal;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
