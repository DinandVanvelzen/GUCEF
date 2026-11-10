/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CBIGUNSIGNED_H
#define GUCEF_CORE_CBIGUNSIGNED_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  An unsigned integer of any size, held as 32 bit limbs (least significant first, no leading zero limbs).
 *  Made for the public key operations of signature verification: it converts from and to big endian bytes,
 *  compares and tests bits. The arithmetic is in CMontgomeryModulus. Not meant for secrets: nothing here is
 *  constant time and nothing is wiped.
 */
class GUCEF_CORE_PUBLIC_CPP CBigUnsigned
{
    public:

    CBigUnsigned( void );

    /** Sets the value from big endian bytes, leading zero bytes are fine. False, value 0, for NULL bytes with a size */
    bool SetFromBigEndian( const void* bytes, UInt32 size );

    void SetZero( void );

    /** Writes the value as 'size' big endian bytes, padded with zeros in front. False, nothing written, when it needs more bytes */
    bool ToBigEndian( void* bytes, UInt32 size ) const;

    /** The number of bits up to and including the highest 1 bit, 0 for the value 0 */
    UInt32 GetBitLength( void ) const;

    /** The number of bytes the value needs, 0 for the value 0 */
    UInt32 GetByteLength( void ) const;

    bool IsZero( void ) const;

    bool IsOdd( void ) const;

    /** Bit 0 is the least significant, bits beyond the value are 0 */
    bool GetBit( UInt32 bitIndex ) const;

    /** The value as a 64 bit number, false when it does not fit */
    bool TryGetUInt64( UInt64& value ) const;

    /** -1, 0 or 1 for smaller, equal or larger than 'other' */
    Int32 Compare( const CBigUnsigned& other ) const;

    UInt32 GetLimbCount( void ) const;

    /** The limbs, least significant first. NULL for the value 0 */
    const UInt32* GetLimbs( void ) const;

    /** Makes the value 'limbCount' limbs from 'limbs' (least significant first, leading zero limbs are dropped) */
    void SetFromLimbs( const UInt32* limbs, UInt32 limbCount );

    private:

    GUCEF::vector< UInt32 > m_limbs;
};

/*-------------------------------------------------------------------------*/

/**
 *  Modular arithmetic for one odd modulus with Montgomery multiplication, for public exponent operations such as
 *  RSA verification: the context (Montgomery constants) is made once with Initialize() and then reused for many
 *  ModExp() calls, which make no heap allocation once the result has grown to the size of the modulus.
 *  A 2048 bit modulus with the exponent 65537 takes well under a millisecond.
 *
 *  The exponent is public: the time depends on it. One context is used by one thread at a time.
 */
class GUCEF_CORE_PUBLIC_CPP CMontgomeryModulus
{
    public:

    CMontgomeryModulus( void );

    /** Takes in the modulus, which must be odd and larger than 1. False otherwise */
    bool Initialize( const CBigUnsigned& modulus );

    bool IsInitialized( void ) const;

    const CBigUnsigned& GetModulus( void ) const;

    /** result = base ^ exponent mod modulus. False when not initialized, 'base' is not smaller than the modulus or the exponent is 0 */
    bool ModExp( const CBigUnsigned& base      ,
                 const CBigUnsigned& exponent  ,
                 CBigUnsigned& result          );

    private:

    CMontgomeryModulus( const CMontgomeryModulus& src );                /**< not implemented */
    CMontgomeryModulus& operator=( const CMontgomeryModulus& src );     /**< not implemented */

    void MontMul( const UInt32* a, const UInt32* b, UInt32* out );

    void DoubleMod( UInt32* value ) const;

    private:

    CBigUnsigned m_modulus;
    UInt32 m_limbCount;
    UInt32 m_n0Inverse;                         /**< -1/n mod 2^32 */
    GUCEF::vector< UInt32 > m_rr;               /**< R^2 mod n, R = 2^(32 * limbCount) */
    GUCEF::vector< UInt32 > m_scratch;          /**< limbCount + 2 limbs for MontMul */
    GUCEF::vector< UInt32 > m_baseMontgomery;
    GUCEF::vector< UInt32 > m_accumulator;
    GUCEF::vector< UInt32 > m_one;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CBIGUNSIGNED_H ? */
