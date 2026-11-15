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

#ifndef GUCEF_CORE_CAES_H
#define GUCEF_CORE_CAES_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

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
 *  The AES block cipher (FIPS-197) with 128, 192 and 256 bit keys, portable and without heap allocation, and the CBC mode
 *  of NIST SP 800-38A with optional PKCS#7 padding. For reading data that other software encrypted that way (a file format
 *  with a fixed key); it is not an authenticated mode: new data that has to stay confidential and unaltered uses an AEAD
 *  codec (CoreCodecTypes, "aes-256-gcm"). The key schedule is wiped by Clear() and the destructor.
 *
 *  Not thread safe per instance; separate instances may be used on separate threads.
 */
class GUCEF_CORE_PUBLIC_CPP CAes
{
    public:

    static const UInt32 BlockSize = 16;

    CAes( void );
    ~CAes();

    /** Sets the key: 16, 24 or 32 bytes; false for another size */
    bool SetKey( const UInt8* key, UInt32 keySize );

    bool IsKeySet( void ) const { return m_keySet; }

    /** Encrypts or decrypts one block of 16 bytes ('in' and 'out' may be the same); nothing is done without a key */
    void EncryptBlock( const UInt8* in, UInt8* out ) const;
    void DecryptBlock( const UInt8* in, UInt8* out ) const;

    /** Wipes the key schedule */
    void Clear( void );

    /**
     *  CBC decryption of 'inputSize' bytes (a multiple of 16) with the key and the 16 byte IV; with 'removePadding' the PKCS#7
     *  padding is checked and removed. False for a key size that is not 16, 24 or 32, an input that is not whole blocks or
     *  a padding that is not valid ('output' is then empty)
     */
    static bool CbcDecrypt( const UInt8* key, UInt32 keySize, const UInt8* iv, const UInt8* input, UInt32 inputSize, bool removePadding, CDynamicBuffer& output );

    /** CBC encryption; with 'addPadding' PKCS#7 padding is added, else the input has to be whole blocks */
    static bool CbcEncrypt( const UInt8* key, UInt32 keySize, const UInt8* iv, const UInt8* input, UInt32 inputSize, bool addPadding, CDynamicBuffer& output );

    private:

    CAes( const CAes& src );                /**< not implemented: a key schedule is not copied */
    CAes& operator=( const CAes& src );     /**< not implemented */

    UInt8 m_roundKeys[ 240 ];
    UInt32 m_rounds;
    bool m_keySet;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CAES_H ? */
