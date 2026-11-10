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

#ifndef GUCEF_CORE_CSHA512_H
#define GUCEF_CORE_CSHA512_H

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
 *  SHA-512 and SHA-384 (FIPS 180-4), portable and without heap allocation.
 *  SHA-384 is SHA-512 with other start values and a digest cut to 48 bytes.
 *  Used by the software signature codecs; also usable directly where a codec lookup is not wanted.
 */
class GUCEF_CORE_PUBLIC_CPP CSha512
{
    public:

    static const UInt32 DigestSize = 64;
    static const UInt32 Digest384Size = 48;
    static const UInt32 BlockSize = 128;

    /** 'is384' selects SHA-384: the digest is Digest384Size bytes */
    explicit CSha512( bool is384 = false );

    void Reset( void );

    void Update( const void* data, UInt32 size );

    /** Writes the digest (DigestSize bytes, Digest384Size for SHA-384), after which the object must be Reset() before reuse */
    void Final( UInt8* digest );

    UInt32 GetDigestSize( void ) const;

    /** One shot SHA-512 */
    static void Hash( const void* data, UInt32 size, UInt8* digest );

    /** One shot SHA-384 */
    static void Hash384( const void* data, UInt32 size, UInt8* digest );

    private:

    void Transform( const UInt8* block );

    UInt64 m_state[ 8 ];
    UInt64 m_totalSize;
    UInt8 m_block[ BlockSize ];
    UInt32 m_blockFill;
    bool m_is384;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSHA512_H ? */
