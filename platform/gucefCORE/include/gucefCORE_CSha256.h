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

#ifndef GUCEF_CORE_CSHA256_H
#define GUCEF_CORE_CSHA256_H

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
 *  SHA-256 (FIPS 180-4), portable and without heap allocation.
 *  The built in fallback of the "sha256" codec; also usable directly where a codec lookup is not wanted.
 */
class GUCEF_CORE_PUBLIC_CPP CSha256
{
    public:

    static const UInt32 DigestSize = 32;
    static const UInt32 BlockSize = 64;

    CSha256( void );

    void Reset( void );

    void Update( const void* data, UInt32 size );

    /** Writes the 32 byte digest, after which the object must be Reset() before reuse */
    void Final( UInt8* digest );

    /** One shot hash */
    static void Hash( const void* data, UInt32 size, UInt8* digest );

    /** Compares in time independent of where the first difference is */
    static bool ConstantTimeEquals( const void* a, const void* b, UInt32 size );

    private:

    void Transform( const UInt8* block );

    UInt32 m_state[ 8 ];
    UInt64 m_totalSize;
    UInt8 m_block[ BlockSize ];
    UInt32 m_blockFill;
};

/*-------------------------------------------------------------------------*/

/**
 *  Incremental HMAC-SHA256 (RFC 2104). The key is taken in once, Reset() starts a new message with the same key.
 */
class GUCEF_CORE_PUBLIC_CPP CHmacSha256
{
    public:

    static const UInt32 MacSize = CSha256::DigestSize;

    CHmacSha256( void );

    CHmacSha256( const void* key, UInt32 keySize );

    void SetKey( const void* key, UInt32 keySize );

    /** Starts a new message with the key that was set */
    void Reset( void );

    void Update( const void* data, UInt32 size );

    /** Writes the 32 byte MAC, after which Reset() starts the next message */
    void Final( UInt8* mac );

    /** One shot HMAC-SHA256 */
    static void Mac( const void* key, UInt32 keySize, const void* data, UInt32 dataSize, UInt8* mac );

    private:

    CSha256 m_inner;
    CSha256 m_innerStart;       /**< the inner hash after the key block, where every message starts */
    CSha256 m_outerStart;       /**< the outer hash after the key block */
};

/*-------------------------------------------------------------------------*/

/**
 *  HKDF with HMAC-SHA256 (RFC 5869)
 */
class GUCEF_CORE_PUBLIC_CPP CHkdfSha256
{
    public:

    /** The longest output HKDF-SHA256 can give: 255 blocks of 32 bytes */
    static const UInt32 MaxOutputSize = 255 * CSha256::DigestSize;

    /** Extract: the pseudo random key of 32 bytes from the input keying material, an empty salt counts as 32 zero bytes */
    static void Extract( const void* salt, UInt32 saltSize, const void* ikm, UInt32 ikmSize, UInt8* prk );

    /** Expand: 'okmSize' bytes of output keying material from a pseudo random key. False when 'okmSize' is larger than MaxOutputSize */
    static bool Expand( const UInt8* prk, const void* info, UInt32 infoSize, UInt8* okm, UInt32 okmSize );

    /** Extract and Expand */
    static bool Derive( const void* salt, UInt32 saltSize, const void* ikm, UInt32 ikmSize, const void* info, UInt32 infoSize, UInt8* okm, UInt32 okmSize );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSHA256_H ? */
