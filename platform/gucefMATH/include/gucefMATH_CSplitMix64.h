/*
 *  gucefMATH: GUCEF module providing math: scalar, vector, matrix and numeric classes
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

#ifndef GUCEF_MATH_CSPLITMIX64_H
#define GUCEF_MATH_CSPLITMIX64_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MATH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  SplitMix64 (G. L. Steele, D. Lea, C. H. Flood). A 64 bit state generator that passes the common statistical
 *  test batteries. Its main use is expanding one seed into the state of a larger generator (see
 *  CXoshiro256StarStar::Seed), it is also a fine generator by itself.
 *
 *  A value type: no global state, copyable, cheap. The integer output is bit identical on all platforms.
 *  Not thread safe: one thread per object. Realtime safe.
 */
class CSplitMix64
{
    public:

    /** Number of 64 bit words of the state, as used by GetState() and SetState() */
    enum { STATE_WORD_COUNT = 1 };

    CSplitMix64( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_state( 0 )
    {
    }

    explicit CSplitMix64( UInt64 seed ) GUCEF_MATH_REALTIME_CONTEXT
        : m_state( seed )
    {
    }

    /** Restarts the sequence from the given seed. Every seed is valid */
    void Seed( UInt64 seed ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_state = seed;
    }

    /** Next 64 random bits */
    UInt64 NextUInt64( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_state += 0x9E3779B97F4A7C15ULL;
        UInt64 z = m_state;
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
        return z ^ ( z >> 31 );
    }

    /** Next 32 random bits (the upper half of the 64 bit output) */
    UInt32 NextUInt32( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return (UInt32) ( NextUInt64() >> 32 );
    }

    /** Copies the state out, for saving and replay */
    void GetState( UInt64* words ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        words[ 0 ] = m_state;
    }

    /** Restores a state obtained from GetState() */
    void SetState( const UInt64* words ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_state = words[ 0 ];
    }

    bool operator==( const CSplitMix64& other ) const
    {
        return m_state == other.m_state;
    }

    bool operator!=( const CSplitMix64& other ) const
    {
        return m_state != other.m_state;
    }

    private:

    UInt64 m_state;
};

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}  /* namespace MATH */
}  /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_CSPLITMIX64_H ? */
