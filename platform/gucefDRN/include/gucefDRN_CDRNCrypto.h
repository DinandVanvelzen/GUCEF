/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#define GUCEF_DRN_CDRNCRYPTO_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_DRN_MACROS_H
#include "gucefDRN_macros.h"
#define GUCEF_DRN_MACROS_H
#endif /* GUCEF_DRN_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The cryptography DRN uses, from the codecs of gucefCORE (CCodecRegistry: the provider with the highest priority):
 *  key agreement "x25519" (KeyAgreementCodec), signatures "ed25519" (SignatureCodec), the AEAD ciphers
 *  (EncryptionCodec), and SHA-256 / HMAC-SHA256 / HKDF-SHA256 of gucefCORE itself.
 *  Every call that needs randomness checks it and fails without it.
 */
class GUCEF_DRN_EXPORT_CPP CDRNCrypto
{
    public:

    static const UInt32 KeySize = 32;           /**< x25519 keys, AEAD keys, ed25519 seeds and public keys */
    static const UInt32 SignatureSize = 64;     /**< ed25519 */
    static const UInt32 HashSize = 32;          /**< SHA-256 */

    static const CORE::CString KeyAgreementType;    /**< "x25519" */
    static const CORE::CString SignatureType;       /**< "ed25519" */

    /** Compares two secrets of any size in time independent of where they differ and of their sizes */
    static bool ConstantTimeEqualsHashed( const void* a, UInt32 aSize, const void* b, UInt32 bSize );

    static bool ConstantTimeEquals( const void* a, const void* b, UInt32 size );

    /** Random bytes from the O/S, false when there are none */
    static bool RandomBytes( void* buffer, UInt32 size );

    static bool IsKeyAgreementAvailable( void );

    static bool IsSignatureAvailable( void );

    /** Is there a codec for the AEAD cipher */
    static bool IsCipherAvailable( const CORE::CString& cipher );

    /** A new x25519 key pair */
    static bool GenerateKeyAgreementKeyPair( UInt8* privateKey, UInt8* publicKey );

    static bool GetKeyAgreementPublicKey( const UInt8* privateKey, UInt8* publicKey );

    /** The x25519 shared secret of our private key and the peer's public key */
    static bool ComputeSharedSecret( const UInt8* privateKey, const UInt8* peerPublicKey, UInt8* sharedSecret );

    /** A new ed25519 key pair: the 32 byte seed (RFC 8032) and the public key */
    static bool GenerateSignatureKeyPair( UInt8* seed, UInt8* publicKey );

    static bool GetSignaturePublicKey( const UInt8* seed, UInt8* publicKey );

    static bool Sign( const UInt8* seed, const void* message, UInt32 messageSize, UInt8* signature );

    static bool Verify( const UInt8* publicKey, const void* message, UInt32 messageSize, const UInt8* signature );

    /** HKDF-SHA256 (RFC 5869) */
    static bool Hkdf( const void* salt, UInt32 saltSize, const void* ikm, UInt32 ikmSize, const void* info, UInt32 infoSize, UInt8* okm, UInt32 okmSize );

    static void Sha256( const void* data, UInt32 size, UInt8* digest );

    /** Lowercase hex */
    static CORE::CString ToHex( const void* data, UInt32 size );

    /** Hex of exactly expectedSize bytes, either case */
    static bool FromHex( const CORE::CString& hex, UInt8* output, UInt32 expectedSize );

    /** The codec with the highest priority, NULL if there is none */
    static CORE::CCodecRegistry::TICodecPtr GetCodec( const CORE::CString& family, const CORE::CString& type );

    private:

    static bool RunCodec( const CORE::CString& family, const CORE::CString& type, bool encode, const CORE::CValueList& params,
                          const void* input, UInt32 inputSize, CORE::CDynamicBuffer& output, CORE::TCodecStatus* status );

    CDRNCrypto( void );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */
