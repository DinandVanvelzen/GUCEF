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

#ifndef GUCEF_AUDIO_CSLIDINGQUANTILE_H
#define GUCEF_AUDIO_CSLIDINGQUANTILE_H

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
 *  Order statistics of the last N values of a stream: the smallest, the k-th smallest or a quantile of the window, and
 *  how many values of the window lie below a limit. The window is kept twice: in arrival order (a ring, to know which
 *  value leaves) and sorted (to answer). A Push() costs a binary search and a move of at most N values in each
 *  direction, which for the window sizes it is meant for (a few thousand values per second of data at most) is a few
 *  microseconds; a query is constant time (rank, quantile) or a binary search (CountBelow()).
 *
 *  Values are Float64, so a stream that is corrected by a growing offset keeps its precision.
 *
 *  Prepare() allocates; nothing allocates or locks after it. Not thread safe: one thread at a time.
 */
class GUCEF_AUDIO_PUBLIC_CPP CSlidingQuantile
{
    public:

    CSlidingQuantile( void );

    ~CSlidingQuantile();

    /** Sets the window size (at least 1) and forgets all values. Allocates: not for the real time thread */
    bool Prepare( UInt32 windowSize );

    UInt32 GetWindowSize( void ) const;

    /** Values in the window: up to the window size */
    UInt32 GetCount( void ) const;

    /** Forgets all values */
    void Reset( void );

    /** Adds a value; the oldest leaves when the window is full */
    void Push( Float64 value );

    /** The smallest value of the window, 0 when it is empty */
    Float64 GetMinimum( void ) const;

    /** The value of rank 'rank' (0: the smallest), limited to the largest; 0 when the window is empty */
    Float64 GetRanked( UInt32 rank ) const;

    /**
     *  The value below which a fraction 'fraction' (0 to 1) of the window lies: the value of rank
     *  floor( fraction * count ), so 0 gives the minimum and a fraction smaller than 1 / count does too. 0 when empty
     */
    Float64 GetQuantile( Float64 fraction ) const;

    /** The rank GetQuantile() uses for 'fraction' with the values there are now */
    UInt32 GetQuantileRank( Float64 fraction ) const;

    /** The number of values in the window that are smaller than 'limit' */
    UInt32 CountBelow( Float64 limit ) const;

    private:

    CSlidingQuantile( const CSlidingQuantile& src );              /**< not implemented */
    CSlidingQuantile& operator=( const CSlidingQuantile& src );   /**< not implemented */

    /** The first position in the sorted window whose value is not smaller than 'value' */
    UInt32 LowerBound( Float64 value ) const;

    private:

    GUCEF::vector< Float64 > m_ring;        /**< the window in arrival order */
    GUCEF::vector< Float64 > m_sorted;      /**< the same values, ascending, in the first m_count places */
    UInt32 m_windowSize;
    UInt32 m_head;                          /**< ring slot of the oldest value */
    UInt32 m_count;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUDIO */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUDIO_CSLIDINGQUANTILE_H ? */
