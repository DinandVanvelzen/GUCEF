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


#ifndef GUCEF_INPUT_WAYLAND_CKEYREPEAT_H
#define GUCEF_INPUT_WAYLAND_CKEYREPEAT_H

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
 *  The repeating of a held key. Wayland leaves it to the client: the compositor says how many characters a second and how
 *  long to wait for the first (wl_keyboard.repeat_info) and the client makes the repeats. One key repeats at a time, the
 *  last one pressed, as on every keyboard. Times are tick counts in milliseconds, which may wrap.
 */
class GUCEF_INPUT_PLUGIN_PUBLIC_CPP CKeyRepeat
{
    public:

    /** The most repeats given at once when the program was stalled for a long time; the rest is dropped */
    static const UInt32 MaxRepeatsPerPoll = 4;

    CKeyRepeat( void );

    /** Characters a second (0 turns repeating off) and the delay before the first repeat in milliseconds */
    void SetRate( Int32 charactersPerSecond, Int32 delayInMs );

    Int32 GetRate( void ) const;

    Int32 GetDelay( void ) const;

    void Start( UInt32 key, UInt32 now );

    /** The key was released: stops when it is the one repeating */
    void Stop( UInt32 key );

    void StopAll( void );

    bool IsRepeating( void ) const;

    UInt32 GetRepeatingKey( void ) const;

    /** The number of repeats of the held key that are due at 'now' (the key is in 'key'); 0 when none */
    UInt32 Poll( UInt32 now, UInt32& key );

    private:

    Int32 m_rate;
    Int32 m_delay;
    bool m_active;
    UInt32 m_key;
    UInt32 m_nextAt;
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

#endif /* GUCEF_INPUT_WAYLAND_CKEYREPEAT_H ? */
