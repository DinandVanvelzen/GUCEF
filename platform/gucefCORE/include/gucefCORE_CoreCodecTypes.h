/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions 
 *  for standardized protocols
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_CORE_CORECODECTYPES_H
#define GUCEF_CORE_CORECODECTYPES_H

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
 *  The platform does not restrict in any way which types of codecs can be 
 *  registered and used but some codec types are fairly common for common 
 *  use-cases. In order to aid in having consistent keys under which such codecs
 *  are registered the below constants are provided intended as a helpful guide.
 */
class GUCEF_CORE_PUBLIC_CPP CoreCodecTypes
{    
    public:
    
    static const CString HashingCodec;
    static const CString CryptographicHashCodec;       /**< "sha256", "sha512", ... parameter "output" = "hex" (default) or "raw" */
    static const CString ChecksumCodec;
    static const CString CompressionCodec;             /**< "zlib", "deflate", "gzip", ... parameter "level" */

    /**
     *  Authenticated encryption (AEAD): "aes-256-gcm", "chacha20-poly1305", "xchacha20-poly1305".
     *  Parameter "key"; per call (or as parameters for one shot use) "nonce" and "aad".
     *  Encode gives the ciphertext followed by the tag, Decode checks the tag and gives the plaintext.
     */
    static const CString EncryptionCodec;

    /** Message authentication: "hmac-sha256". Parameter "key", Encode gives the MAC */
    static const CString MacCodec;

    /** Key derivation: "hkdf-sha256". Parameters "salt", "info", "length"; Encode turns the input secret into the derived key */
    static const CString KeyDerivationCodec;

    /**
     *  Key agreement: "x25519". Encode: own private key -> own public key.
     *  Decode with parameter "privateKey": the peer's public key -> the shared secret
     */
    static const CString KeyAgreementCodec;

    /**
     *  Signatures: "ed25519". Encode with parameter "privateKey": message -> signature.
     *  Decode with parameters "publicKey" and "signature": verifies the message, no output.
     *  "rsa-pkcs1-sha256", "rsa-pkcs1-sha384", "rsa-pkcs1-sha512": RSASSA-PKCS1-v1_5 verification only.
     *  Decode with parameters "modulus", "exponent" and "signature" (big endian bytes), see CRsaPkcs1VerifyCodec
     */
    static const CString SignatureCodec;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace CORE */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CORECODECTYPES_H ? */
   