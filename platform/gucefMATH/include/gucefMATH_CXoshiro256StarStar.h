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

#ifndef GUCEF_MATH_CXOSHIRO256STARSTAR_H
#define GUCEF_MATH_CXOSHIRO256STARSTAR_H

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

#ifndef GUCEF_MATH_CSPLITMIX64_H
#include "gucefMATH_CSplitMix64.h"
#define GUCEF_MATH_CSPLITMIX64_H
#endif /* GUCEF_MATH_CSPLITMIX64_H ? */

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
 *  xoshiro256** 1.0 (D. Blackman, S. Vigna). 256 bit state, 64 bit output, period 2^256 - 1, with Jump()
 *  (2^128 steps) and LongJump() (2^192 steps) to carve independent sequences out of one seed.
 *
 *  A value type: no global state, copyable. The integer output is bit identical on all platforms and equals the
 *  reference implementation for the same state. The state must not be all zero: Seed() guarantees that, SetState()
 *  replaces an all zero state by a valid one. Not thread safe: one thread per object. Realtime safe.
 */
class CXoshiro256StarStar
{
    public:

    /** Number of 64 bit words of the state, as used by GetState() and SetState() */
    enum { STATE_WORD_COUNT = 4 };

    /** Seeded with 0 */
    CXoshiro256StarStar( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Seed( 0 );
    }

    explicit CXoshiro256StarStar( UInt64 seed ) GUCEF_MATH_REALTIME_CONTEXT
    {
        Seed( seed );
    }

    /** Expands the seed into the 256 bit state with SplitMix64, as recommended by the authors */
    void Seed( UInt64 seed ) GUCEF_MATH_REALTIME_CONTEXT
    {
        CSplitMix64 expander( seed );
        m_state[ 0 ] = expander.NextUInt64();
        m_state[ 1 ] = expander.NextUInt64();
        m_state[ 2 ] = expander.NextUInt64();
        m_state[ 3 ] = expander.NextUInt64();
    }

    /** Next 64 random bits */
    UInt64 NextUInt64( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        const UInt64 result = RotateLeft( (UInt64) ( m_state[ 1 ] * 5ULL ), 7 ) * 9ULL;
        const UInt64 t = m_state[ 1 ] << 17;
        m_state[ 2 ] ^= m_state[ 0 ];
        m_state[ 3 ] ^= m_state[ 1 ];
        m_state[ 1 ] ^= m_state[ 2 ];
        m_state[ 0 ] ^= m_state[ 3 ];
        m_state[ 2 ] ^= t;
        m_state[ 3 ] = RotateLeft( m_state[ 3 ], 45 );
        return result;
    }

    /** Next 32 random bits (the upper half of the 64 bit output) */
    UInt32 NextUInt32( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        return (UInt32) ( NextUInt64() >> 32 );
    }

    /** Equivalent to 2^128 calls of NextUInt64(): 2^128 non overlapping sequences of length 2^128 */
    void Jump( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const UInt64 jump[ 4 ] = { 0x180EC6D33CFD0ABAULL, 0xD5A61266F0C9392CULL, 0xA9582618E03FC9AAULL, 0x39ABDC4529B1661CULL };
        ApplyJumpPolynomial( jump );
    }

    /** Equivalent to 2^192 calls of NextUInt64(): 2^64 starting points for sequences that each use Jump() */
    void LongJump( void ) GUCEF_MATH_REALTIME_CONTEXT
    {
        static const UInt64 longJump[ 4 ] = { 0x76E15D3EFEFDCBBFULL, 0xC5004E441C522FB3ULL, 0x77710069854EE241ULL, 0x39109BB02ACBE635ULL };
        ApplyJumpPolynomial( longJump );
    }

    /**
     *  Advances the state by the number of steps that the given polynomial of the transition matrix stands for
     *  (the mechanism of the reference Jump() and LongJump()): bit i of the 256 bit polynomial selects that the
     *  state after i steps is added to the result. Public so that the mechanism can be checked with other
     *  polynomials than the published constants.
     */
    void ApplyJumpPolynomial( const UInt64* polynomial ) GUCEF_MATH_REALTIME_CONTEXT
    {
        UInt64 s0 = 0;
        UInt64 s1 = 0;
        UInt64 s2 = 0;
        UInt64 s3 = 0;
        for ( UInt32 i = 0; i < 4; ++i )
        {
            for ( UInt32 b = 0; b < 64; ++b )
            {
                if ( ( polynomial[ i ] >> b ) & 1ULL )
                {
                    s0 ^= m_state[ 0 ];
                    s1 ^= m_state[ 1 ];
                    s2 ^= m_state[ 2 ];
                    s3 ^= m_state[ 3 ];
                }
                NextUInt64();
            }
        }
        m_state[ 0 ] = s0;
        m_state[ 1 ] = s1;
        m_state[ 2 ] = s2;
        m_state[ 3 ] = s3;
    }

    /** Copies the state out, for saving and replay */
    void GetState( UInt64* words ) const GUCEF_MATH_REALTIME_CONTEXT
    {
        words[ 0 ] = m_state[ 0 ];
        words[ 1 ] = m_state[ 1 ];
        words[ 2 ] = m_state[ 2 ];
        words[ 3 ] = m_state[ 3 ];
    }

    /** Restores a state obtained from GetState(). An all zero state, which the generator cannot leave, is replaced by Seed( 0 ) */
    void SetState( const UInt64* words ) GUCEF_MATH_REALTIME_CONTEXT
    {
        if ( 0 == ( words[ 0 ] | words[ 1 ] | words[ 2 ] | words[ 3 ] ) )
        {
            Seed( 0 );
            return;
        }
        m_state[ 0 ] = words[ 0 ];
        m_state[ 1 ] = words[ 1 ];
        m_state[ 2 ] = words[ 2 ];
        m_state[ 3 ] = words[ 3 ];
    }

    bool operator==( const CXoshiro256StarStar& other ) const
    {
        return m_state[ 0 ] == other.m_state[ 0 ] && m_state[ 1 ] == other.m_state[ 1 ] &&
               m_state[ 2 ] == other.m_state[ 2 ] && m_state[ 3 ] == other.m_state[ 3 ];
    }

    bool operator!=( const CXoshiro256StarStar& other ) const
    {
        return !( *this == other );
    }

    private:

    UInt64 m_state[ 4 ];
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

#endif /* GUCEF_MATH_CXOSHIRO256STARSTAR_H ? */
