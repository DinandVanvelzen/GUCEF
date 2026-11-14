/*
 *  gucefMATH_TestApp: GUCEF test application for the Math module
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

#ifndef GUCEF_MATH_TESTAPP_TESTMATHREFERENCE_H
#define GUCEF_MATH_TESTAPP_TESTMATHREFERENCE_H

/*
 *  Test helpers of the gucefMATH test application: a seeded generator for reproducible sweeps, FNV-1a 64 bit hashing
 *  of raw outputs for the determinism checks, and an environment switch for the long exhaustive sweeps.
 *  Specific to gucefMATH, so it lives with its test application.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_MATH_MACROS_H
#include "gucefMATH_macros.h"
#define GUCEF_MATH_MACROS_H
#endif /* GUCEF_MATH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace MathReference {

/**
 *  Seeded generator for test sweeps (SplitMix64, Steele, Lea and Flood 2014): small, fast and identical on every
 *  platform. Deliberately independent of the generators under test in gucefMATH.
 */
class CSeededSequence
{
    public:

    explicit CSeededSequence( GUCEF::UInt64 seed )
        : m_state( seed )
    {
    }

    GUCEF::UInt64 NextUInt64( void )
    {
        GUCEF::UInt64 z = ( m_state += 0x9E3779B97F4A7C15ULL );
        z = ( z ^ ( z >> 30 ) ) * 0xBF58476D1CE4E5B9ULL;
        z = ( z ^ ( z >> 27 ) ) * 0x94D049BB133111EBULL;
        return z ^ ( z >> 31 );
    }

    GUCEF::UInt32 NextUInt32( void )
    {
        return (GUCEF::UInt32) ( NextUInt64() >> 32 );
    }

    /** Signed value whose magnitude has a uniformly chosen bit length 1 .. bits - 1, so small and large magnitudes both occur */
    GUCEF::Int64 NextSignedWithRandomLength( GUCEF::UInt32 bits )
    {
        GUCEF::UInt64 r = NextUInt64();
        GUCEF::UInt32 length = 1 + (GUCEF::UInt32) ( NextUInt64() % ( bits - 1 ) );
        GUCEF::UInt64 magnitude = ( r & ( ( (GUCEF::UInt64) 1 << length ) - 1 ) ) | ( (GUCEF::UInt64) 1 << ( length - 1 ) );
        GUCEF::Int64 value = (GUCEF::Int64) magnitude;
        return 0 != ( NextUInt64() & 1 ) ? -value : value;
    }

    /** Uniform in [ low, high ) */
    GUCEF::Float64 NextFloat64( GUCEF::Float64 low, GUCEF::Float64 high )
    {
        GUCEF::Float64 unit = (GUCEF::Float64) ( NextUInt64() >> 11 ) * ( 1.0 / 9007199254740992.0 );
        return low + ( high - low ) * unit;
    }

    private:

    GUCEF::UInt64 m_state;
};

/*-------------------------------------------------------------------------*/

/** FNV-1a 64 bit over raw bytes, continued from 'hash' (start with Fnv1aOffsetBasis()) */
inline GUCEF::UInt64 Fnv1aOffsetBasis( void ) { return 0xCBF29CE484222325ULL; }

inline GUCEF::UInt64
Fnv1a( const void* data, GUCEF::UInt32 byteCount, GUCEF::UInt64 hash )
{
    const GUCEF::UInt8* bytes = static_cast< const GUCEF::UInt8* >( data );
    for ( GUCEF::UInt32 i=0; i<byteCount; ++i )
    {
        hash ^= bytes[ i ];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

/** Hashes a 64 bit value as 8 little endian bytes, so the hash does not depend on the byte order of the platform */
inline GUCEF::UInt64
Fnv1aInt64( GUCEF::Int64 value, GUCEF::UInt64 hash )
{
    GUCEF::UInt64 bits = (GUCEF::UInt64) value;
    GUCEF::UInt8 bytes[ 8 ];
    for ( GUCEF::UInt32 i=0; i<8; ++i )
        bytes[ i ] = (GUCEF::UInt8) ( bits >> ( 8 * i ) );
    return Fnv1a( bytes, 8, hash );
}

/*-------------------------------------------------------------------------*/

/** True when GUCEF_TEST_MATH_EXHAUSTIVE=1: run the long exhaustive sweeps as well */
inline bool
ExhaustiveTestsEnabled( void )
{
    const char* value = getenv( "GUCEF_TEST_MATH_EXHAUSTIVE" );
    return GUCEF_NULL != value && 0 == strcmp( value, "1" );
}

}; /* namespace MathReference */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MATH_TESTAPP_TESTMATHREFERENCE_H ? */
