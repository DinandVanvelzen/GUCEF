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

#ifndef GUCEF_MATH_CPCG32_H
#define GUCEF_MATH_CPCG32_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

#ifndef GUCEF_MATH_BITFUNCTIONS_H
#include "gucefMATH_BitFunctions.h"
#define GUCEF_MATH_BITFUNCTIONS_H
#endif /* GUCEF_MATH_BITFUNCTIONS_H ? */

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
 *  PCG-XSH-RR 64/32 (M. E. O'Neill, pcg-random.org). 64 bit state, 32 bit output, selectable stream: generators
 *  with different streams produce unrelated sequences from the same seed.
 *
 *  A value type: no global state, copyable, cheap. The integer output is bit identical on all platforms and
 *  equals the reference implementation for the same seed and stream. Not thread safe: one thread per object.
 *  Realtime safe.
 */
class CPcg32
{
    public:

    /** Number of 64 bit words of the state, as used by GetState() and SetState(): the state and the increment */
    enum { STATE_WORD_COUNT = 2 };

    /** Default seed and stream of the reference implementation */
    CPcg32( void ) GUCEF_MATH_REALTIME_CONTEXT
        : m_state( 0x853C49E6748FEA9BULL ) ,
          m_increment( 0xDA3E39CB94B95BDBULL )
    {
    }

    CPcg32( UInt64 seed, UInt64 stream ) GUCEF_MATH_REALTIME_CONTEXT
        : m_state( 0 ) ,
          m_increment( 0 )
    {
        Seed( seed, stream );
    }

    /** Restarts the sequence (pcg32_srandom_r of the reference implementation) */
    void Seed( UInt64 seed, UInt64 stream ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_state = 0;
        m_increment = ( stream << 1 ) | 1ULL;
        NextUInt32();
        m_state += seed;
        NextUInt32();
    }

    /** Next 32 random bits */
    UInt32 NextUInt32( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 oldState = m_state;
        m_state = oldState * 6364136223846793005ULL + m_increment;
        UInt32 xorShifted = (UInt32) ( ( ( oldState >> 18 ) ^ oldState ) >> 27 );
        UInt32 rot = (UInt32) ( oldState >> 59 );
        return RotateRight( xorShifted, rot );
    }

    /** Next 64 random bits: two outputs, the first one is the upper half */
    UInt64 NextUInt64( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 high = NextUInt32();
        return ( high << 32 ) | NextUInt32();
    }

    /** Moves the sequence forward by the given number of outputs in O( log delta ) time. Wraps around the period */
    void Advance( UInt64 delta ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 accMult = 1;
        UInt64 accPlus = 0;
        UInt64 curMult = 6364136223846793005ULL;
        UInt64 curPlus = m_increment;
        while ( delta > 0 )
        {
            if ( delta & 1 )
            {
                accMult *= curMult;
                accPlus = accPlus * curMult + curPlus;
            }
            curPlus = ( curMult + 1 ) * curPlus;
            curMult *= curMult;
            delta >>= 1;
        }
        m_state = accMult * m_state + accPlus;
    }

    /** Copies the state out, for saving and replay */
    void GetState( UInt64* words ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        words[ 0 ] = m_state;
        words[ 1 ] = m_increment;
    }

    /** Restores a state obtained from GetState(). The increment is forced odd, as the generator requires */
    void SetState( const UInt64* words ) GUCEF_MATH_REALTIME_CONTEXT
    {
        m_state = words[ 0 ];
        m_increment = words[ 1 ] | 1ULL;
    }

    bool operator==( const CPcg32& other ) const
    {
        return m_state == other.m_state && m_increment == other.m_increment;
    }

    bool operator!=( const CPcg32& other ) const
    {
        return !( *this == other );
    }

    private:

    UInt64 m_state;
    UInt64 m_increment;
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

#endif /* GUCEF_MATH_CPCG32_H ? */
