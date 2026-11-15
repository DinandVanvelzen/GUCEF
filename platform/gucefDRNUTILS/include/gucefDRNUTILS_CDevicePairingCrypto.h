/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#ifndef GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H
#define GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H

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

#ifndef GUCEF_DRNUTILS_MACROS_H
#include "gucefDRNUTILS_macros.h"
#define GUCEF_DRNUTILS_MACROS_H
#endif /* GUCEF_DRNUTILS_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The values and proofs of device pairing (gucefASSETSTORE_RemoteNetworking_Design.md 3.4). All keys are 32 bytes:
 *  x25519 for the service's static key, Ed25519 (seed and public key) for device keys.
 *  - The pairing code: 80 random bits as 16 Crockford base32 characters, shown in groups of four ("K7QM-3XRD-9HTV-W2PA");
 *    entry ignores case, separators and spaces, and reads I and L as 1 and O as 0.
 *  - Keys from the code: HKDF-SHA256( ikm = the normalised code, salt = "gucef.drnutils.pairing.v1:" + service name ),
 *    info "auth" gives the secret of the shared-secret authenticator (as 64 hexadecimal characters, the authenticator
 *    takes text), info "server-proof" the key of the server proof.
 *  - Server proof: HMAC-SHA256( server proof key, "gucef.drnutils.pairing.server.v1" | static key | pairing nonce ).
 *  - Device key proof: an Ed25519 signature by the device key over "gucef.drnutils.pairing.device.v1" | static key | nonce.
 *  - Fingerprint of a key: the first 20 Crockford base32 characters of its SHA-256, in groups of four.
 *  Every random value comes from CORE::CRandom and fails closed (false) without randomness. Comparisons are constant time.
 */
class GUCEF_DRNUTILS_PUBLIC_CPP CDevicePairingCrypto
{
    public:

    static const UInt32 KeySize = 32;
    static const UInt32 SignatureSize = 64;
    static const UInt32 NonceSize = 32;
    static const UInt32 CodeLength = 16;

    /** A new pairing code, normalised (16 characters, no separators) */
    static bool GenerateCode( CORE::CString& code );

    /** "K7QM3XRD9HTVW2PA" as "K7QM-3XRD-9HTV-W2PA" */
    static CORE::CString FormatCode( const CORE::CString& normalisedCode );

    /** Normalises what a person typed; false when it is not 16 Crockford base32 characters */
    static bool NormaliseCode( const CORE::CString& typed, CORE::CString& normalisedCode );

    /** The shared-secret authenticator's secret (64 hexadecimal characters) and the server proof key */
    static bool DeriveKeys( const CORE::CString& normalisedCode, const CORE::CString& serviceName,
                            CORE::CString& authSecretHex, UInt8* serverProofKey );

    static bool ComputeServerProof( const UInt8* serverProofKey, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, UInt8* proof );
    static bool VerifyServerProof( const UInt8* serverProofKey, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, const UInt8* proof, UInt32 proofSize );

    /** The message the device key signs at pairing */
    static void GetDeviceProofMessage( const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, CORE::CDynamicBuffer& message );
    static bool SignDeviceProof( const UInt8* deviceSeed, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, UInt8* signature );
    static bool VerifyDeviceProof( const UInt8* devicePublicKey, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, const UInt8* signature, UInt32 signatureSize );

    /** A new Ed25519 device key pair (seed and public key) */
    static bool GenerateDeviceKey( UInt8* seed, UInt8* publicKey );

    static CORE::CString Fingerprint( const UInt8* publicKey );

    /** Fingerprint() of a key in hexadecimal; empty for a malformed key */
    static CORE::CString FingerprintOfHex( const CORE::CString& publicKeyHex );

    /** Ids: "d-" + 26 characters (128 bits) for devices; 26 characters for client sessions; "w-" + 16 for pairing windows */
    static bool GenerateDeviceId( CORE::CString& deviceId );
    static bool GenerateSessionId( CORE::CString& sessionId );
    static bool GenerateWindowId( CORE::CString& windowId );

    /** Random bytes; false without randomness */
    static bool RandomBytes( UInt8* buffer, UInt32 size );

    /** Crockford base32 of bytes, as many characters as asked for (5 bits each, most significant first) */
    static CORE::CString ToCrockfordBase32( const UInt8* data, UInt32 size, UInt32 characters );

    /** Constant time equality */
    static bool Equals( const void* a, const void* b, UInt32 size );

    private:

    CDevicePairingCrypto( void );       /**< not implemented: static members only */
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRNUTILS_CDEVICEPAIRINGCRYPTO_H ? */
