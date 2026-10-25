/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#define GUCEF_AUTH_CAUTHCRYPTO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_AUTH_MACROS_H
#include "gucefAUTH_macros.h"
#define GUCEF_AUTH_MACROS_H
#endif /* GUCEF_AUTH_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The few cryptographic operations the module needs, on top of the codec registry of gucefCORE: hashing, hex, random
 *  bytes, AEAD with the "xchacha20-poly1305" codec and the verification of "ed25519" signatures. The AEAD and the
 *  signature codec come from codecspluginMONOCYPHER (or another provider of the same types); without them the
 *  operations that need them report false.
 */
class GUCEF_AUTH_EXPORT_CPP CAuthCrypto
{
    public:

    static const UInt32 KeySize = 32;               /**< AEAD key, Ed25519 public key */
    static const UInt32 NonceSize = 24;             /**< XChaCha20-Poly1305 */
    static const UInt32 TagSize = 16;
    static const UInt32 DigestSize = 32;            /**< SHA-256 */
    static const UInt32 SignatureSize = 64;         /**< Ed25519 */

    static const CORE::CString AeadCipher;          /**< "xchacha20-poly1305" */
    static const CORE::CString SignatureType;       /**< "ed25519" */

    /** Random bytes from the operating system; false when it has none */
    static bool RandomBytes( void* buffer, UInt32 size );

    static void Sha256( const void* data, UInt32 size, UInt8* digest );

    /** SHA-256 as 64 lower case hex digits */
    static CORE::CString Sha256Hex( const void* data, UInt32 size );

    static CORE::CString ToHex( const void* data, UInt32 size );

    /** Decodes hex of upper or lower case digits, an even number of them; false for anything else */
    static bool FromHex( const CORE::CString& hex, CORE::CDynamicBuffer& output );

    /** Decodes exactly expectedSize bytes of hex */
    static bool FromHexFixed( const CORE::CString& hex, UInt8* output, UInt32 expectedSize );

    /** Compares in time that does not depend on where the data differs */
    static bool ConstantTimeEquals( const void* a, const void* b, UInt32 size );

    /** Overwrites memory in a way the compiler does not remove */
    static void Wipe( void* memory, UInt32 size );

    static bool IsAeadAvailable( void );

    /** Encrypts and authenticates: appends the ciphertext followed by the tag to output */
    static bool AeadSeal( const UInt8* key                  ,
                          const UInt8* nonce                ,
                          const void* aad                   ,
                          UInt32 aadSize                    ,
                          const void* plaintext             ,
                          UInt32 plaintextSize              ,
                          CORE::CDynamicBuffer& output      );

    /** Checks the tag and appends the plaintext to output; false when the data is not authentic, nothing is appended then */
    static bool AeadOpen( const UInt8* key                  ,
                          const UInt8* nonce                ,
                          const void* aad                   ,
                          UInt32 aadSize                    ,
                          const void* sealed                ,
                          UInt32 sealedSize                 ,
                          CORE::CDynamicBuffer& output      );

    static bool IsSignatureAvailable( void );

    /** Whether the RSA PKCS#1 v1.5 signature codecs are there (gucefCORE has them in software) */
    static bool IsRsaAvailable( void );

    /**
     *  Verifies an RSASSA-PKCS1-v1_5 signature as JWS uses it with the "rsa-pkcs1-sha256", "-sha384" or "-sha512" codec of
     *  the registry, picked by the JWS algorithm name RS256, RS384 or RS512. The public key is the big endian modulus and
     *  exponent. False for any other algorithm name, a key the codec refuses and a signature that does not match.
     */
    static bool VerifyRsaPkcs1( const CORE::CString& jwsAlgorithm  ,
                                const void* modulus                ,
                                UInt32 modulusSize                 ,
                                const void* exponent               ,
                                UInt32 exponentSize                ,
                                const void* message                ,
                                UInt32 messageSize                 ,
                                const void* signature              ,
                                UInt32 signatureSize               );

    /** Verifies an Ed25519 signature given as hex, with the public key as hex */
    static bool VerifySignature( const CORE::CString& publicKeyHex ,
                                 const void* message               ,
                                 UInt32 messageSize                ,
                                 const CORE::CString& signatureHex );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */
