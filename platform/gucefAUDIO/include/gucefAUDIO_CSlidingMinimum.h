/*
 *  gucefAUDIO: GUCEF module providing audio playback, mixing and decoding
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

#ifndef GUCEF_AUDIO_CSLIDINGMINIMUM_H
#define GUCEF_AUDIO_CSLIDINGMINIMUM_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_AUDIO_MACROS_H
#include "gucefAUDIO_macros.h"
#define GUCEF_AUDIO_MACROS_H
#endif /* GUCEF_AUDIO_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUDIO {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The smallest value of the last N values of a stream, found with an amortised constant amount of work per value
 *  however large N is: a monotonic deque kept in a ring that was allocated for N values (D. Lemire, "Streaming
 *  Maximum-Minimum Filter Using No More than Three Comparisons per Element", 2006). Used by the lookahead limiter
 *  to find the strongest gain reduction ahead.
 *
 *  Push() adds a value and returns the minimum of the window ending with it. Until N values have been pushed the
 *  window holds the values that there are. To get a maximum, push negated values.
 *
 *  The ring is allocated in Prepare(), nothing allocates or locks after that. Not thread safe: one thread at a time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CSlidingMinimum
{
    public:

    CSlidingMinimum( void );

    ~CSlidingMinimum();

    /** Sets the window size (at least 1) and forgets all values. Allocates: not for the real time thread */
    bool Prepare( UInt32 windowSize );

    /**
     *  Sets another window size within the room the last Prepare() reserved (1 .. GetCapacity()) and forgets all values.
     *  Allocates nothing, so it is fit for the real time thread. False, and nothing changed, for a size that does not fit
     */
    bool SetWindowSize( UInt32 windowSize );

    UInt32 GetWindowSize( void ) const;

    /** The largest window size the last Prepare() made room for */
    UInt32 GetCapacity( void ) const;

    /** Forgets all values */
    void Reset( void );

    /** Adds a value, returns the smallest of the last GetWindowSize() values including this one */
    inline Float32 Push( Float32 value )
    {
        // values that left the window are at the front
        if ( m_count > 0 && m_position - m_indices[ m_head ] >= m_windowSize )
        {
            m_head = ( m_head + 1 == m_windowSize ) ? 0 : m_head + 1;
            --m_count;
        }

        // values at the back that are not smaller can never be the minimum again
        while ( m_count > 0 )
        {
            UInt32 back = m_head + m_count - 1;
            if ( back >= m_windowSize ) back -= m_windowSize;
            if ( m_values[ back ] < value )
                break;
            --m_count;
        }

        UInt32 slot = m_head + m_count;
        if ( slot >= m_windowSize ) slot -= m_windowSize;
        m_values[ slot ] = value;
        m_indices[ slot ] = m_position;
        ++m_count;
        ++m_position;

        return m_values[ m_head ];
    }

    /** The minimum of the window as of the last Push(), 0 when nothing was pushed */
    inline Float32 GetMinimum( void ) const
    {
        return m_count > 0 ? m_values[ m_head ] : 0.0f;
    }

    private:

    CSlidingMinimum( const CSlidingMinimum& src );              /**< not implemented */
    CSlidingMinimum& operator=( const CSlidingMinimum& src );   /**< not implemented */

    private:

    GUCEF::vector< Float32 > m_values;      /**< the ring: values that can still become the minimum, ascending */
    GUCEF::vector< UInt64 > m_indices;      /**< the ring: running number of each of those values */
    UInt32 m_windowSize;
    UInt32 m_head;                          /**< ring slot of the oldest value */
    UInt32 m_count;                         /**< values in the ring */
    UInt64 m_position;                      /**< running number of the next value */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CSLIDINGMINIMUM_H ? */
