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

#include "inputdriverWAYLAND_CKeyRepeat.h"

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

CKeyRepeat::CKeyRepeat( void )
    : m_rate( 25 )
    , m_delay( 600 )
    , m_active( false )
    , m_key( 0 )
    , m_nextAt( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CKeyRepeat::SetRate( Int32 charactersPerSecond, Int32 delayInMs )
{GUCEF_TRACE;

    m_rate = charactersPerSecond > 0 ? charactersPerSecond : 0;
    m_delay = delayInMs > 0 ? delayInMs : 0;
    if ( 0 == m_rate )
        m_active = false;
}

/*-------------------------------------------------------------------------*/

Int32 CKeyRepeat::GetRate( void ) const { return m_rate; }
Int32 CKeyRepeat::GetDelay( void ) const { return m_delay; }

/*-------------------------------------------------------------------------*/

void
CKeyRepeat::Start( UInt32 key, UInt32 now )
{GUCEF_TRACE;

    if ( 0 == m_rate )
    {
        m_active = false;
        return;
    }
    m_active = true;
    m_key = key;
    m_nextAt = now + (UInt32) m_delay;
}

/*-------------------------------------------------------------------------*/

void
CKeyRepeat::Stop( UInt32 key )
{GUCEF_TRACE;

    if ( m_active && m_key == key )
        m_active = false;
}

/*-------------------------------------------------------------------------*/

void
CKeyRepeat::StopAll( void )
{GUCEF_TRACE;

    m_active = false;
}

/*-------------------------------------------------------------------------*/

bool
CKeyRepeat::IsRepeating( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyRepeat::GetRepeatingKey( void ) const
{GUCEF_TRACE;

    return m_key;
}

/*-------------------------------------------------------------------------*/

UInt32
CKeyRepeat::Poll( UInt32 now, UInt32& key )
{GUCEF_TRACE;

    if ( !m_active || 0 == m_rate )
        return 0;

    // the signed difference is right also when the tick count wrapped
    if ( (Int32) ( now - m_nextAt ) < 0 )
        return 0;

    UInt32 period = 1000 / (UInt32) m_rate;
    if ( 0 == period )
        period = 1;
    UInt32 due = 1 + ( now - m_nextAt ) / period;
    if ( due > MaxRepeatsPerPoll )
    {
        due = MaxRepeatsPerPoll;
        m_nextAt = now + period;
    }
    else
        m_nextAt += due * period;
    key = m_key;
    return due;
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
