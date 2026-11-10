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


#ifndef GUCEF_INPUT_WAYLAND_CAXISACCUMULATOR_H
#define GUCEF_INPUT_WAYLAND_CAXISACCUMULATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_INPUT_MACROS_H
#include "gucefINPUT_macros.h"
#define GUCEF_INPUT_MACROS_H
#endif /* GUCEF_INPUT_MACROS_H ? */

#ifndef GUCEF_CORE_ETYPES_H
#include "gucefCORE_ETypes.h"
#define GUCEF_CORE_ETYPES_H
#endif /* GUCEF_CORE_ETYPES_H ? */

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
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Turns the axis events of wl_pointer into notches of a wheel. The events of one scroll arrive as a group that ends with
 *  the frame event (wl_pointer version 5; older compositors send no frame and each event is a group of its own). A group
 *  has, per axis, what the compositor knows best of it:
 *
 *  - value120 (version 8): the amount in 120ths of a notch, high resolution wheels and smooth scrolling alike
 *  - discrete (versions 5 to 7): whole notches of a wheel
 *  - the axis value: a distance in surface units, of a touch pad (finger source) or of a wheel of a compositor that says
 *    nothing else; a notch is 'unitsPerNotch' of it (15 by default)
 *
 *  Only the best of those is used, never two for the same axis in one group. The part of a notch that is left over is kept
 *  for the next group, except when the scroll ends (axis_stop) or goes the other way. The result is in notches with up and
 *  to the right as positive, which is the other way round vertically from the axis of Wayland (positive is down).
 */
class GUCEF_INPUT_PLUGIN_PUBLIC_CPP CAxisAccumulator
{
    public:

    enum EAxis
    {
        AXIS_VERTICAL   = 0 ,
        AXIS_HORIZONTAL = 1 ,

        AXIS_COUNT
    };

    CAxisAccumulator( void );

    /** How much of the axis value is a notch; the default is 15 */
    void SetUnitsPerNotch( Float64 units );

    Float64 GetUnitsPerNotch( void ) const;

    void OnAxis( UInt32 axis, Float64 value );

    void OnAxisSource( UInt32 source );

    void OnAxisStop( UInt32 axis );

    void OnAxisDiscrete( UInt32 axis, Int32 discrete );

    void OnAxisValue120( UInt32 axis, Int32 value120 );

    /**
     *  Whether the compositor turned the values of the axis around relative to the movement of the device (natural scrolling,
     *  wl_pointer version 9): GUCEF_WL_WL_POINTER_AXIS_RELATIVE_DIRECTION_IDENTICAL (0) or _INVERTED (1). The values are already
     *  what the user wants to see happen, so the notches are not changed by it; it is kept for who wants to know.
     */
    void OnAxisRelativeDirection( UInt32 axis, UInt32 direction );

    /** What the compositor last said about the axis, 0 (identical) when it said nothing or the axis is not one of the two */
    UInt32 GetRelativeDirection( UInt32 axis ) const;

    /** The most notches a group can give for an axis, whatever the compositor sends */
    static const Int32 MaxNotchesPerGroup = 4096;

    /** Ends a group: gives the whole notches that are due (vertical up and horizontal right positive). False when there are none */
    bool Flush( Int32& vertical, Int32& horizontal );

    /** Forgets everything, also the part of a notch that was left */
    void Reset( void );

    private:

    void Accumulate( UInt32 axis, Float64 notches );

    private:

    Float64 m_unitsPerNotch;
    Float64 m_remainder[ AXIS_COUNT ];          /**< notches not yet given, kept across groups */
    Float64 m_axisValue[ AXIS_COUNT ];          /**< of the group */
    Int64 m_discrete[ AXIS_COUNT ];             /**< a sum over a group: wide so that a flood of events cannot overflow it */
    Int64 m_value120[ AXIS_COUNT ];
    UInt32 m_direction[ AXIS_COUNT ];
    bool m_hasAxisValue[ AXIS_COUNT ];
    bool m_hasDiscrete[ AXIS_COUNT ];
    bool m_hasValue120[ AXIS_COUNT ];
    bool m_stopped[ AXIS_COUNT ];
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WAYLAND */
}; /* namespace INPUT */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_INPUT_WAYLAND_CAXISACCUMULATOR_H ? */
