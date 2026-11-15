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

#ifndef GUCEF_CORE_CSHA1_H
#define GUCEF_CORE_CSHA1_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

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
 *  SHA-1 (FIPS 180-4), portable and without heap allocation, with the API of CSha256.
 *
 *  SHA-1 is not collision resistant: use it to identify content where others already do (the digests online stores
 *  and version control systems publish), not to protect anything. CSha256 is the choice for new formats.
 *  Inputs of any length are hashed by calling Update() as often as needed.
 */
class GUCEF_CORE_PUBLIC_CPP CSha1
{
    public:

    static const UInt32 DigestSize = 20;
    static const UInt32 BlockSize = 64;
    static const UInt32 HexDigestLength = 40;

    CSha1( void );

    void Reset( void );

    void Update( const void* data, UInt32 size );

    /** Writes the 20 byte digest, after which the object must be Reset() before reuse */
    void Final( UInt8* digest );

    /** Final() as 40 lower case hexadecimal characters, after which the object must be Reset() before reuse */
    CString FinalHex( void );

    /** One shot hash */
    static void Hash( const void* data, UInt32 size, UInt8* digest );

    /** One shot hash as 40 lower case hexadecimal characters */
    static CString HashHex( const void* data, UInt32 size );

    /** The 20 byte digest as 40 lower case hexadecimal characters */
    static CString DigestToHex( const UInt8* digest );

    /**
     *  The 20 byte digest from 40 hexadecimal characters, upper or lower case; false (and the digest untouched) for
     *  text of another length or with other characters
     */
    static bool DigestFromHex( const CString& hex, UInt8* digest );

    private:

    void Transform( const UInt8* block );

    UInt32 m_state[ 5 ];
    UInt64 m_totalSize;
    UInt8 m_block[ BlockSize ];
    UInt32 m_blockFill;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CSHA1_H ? */
