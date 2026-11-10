/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CMCPREQUESTSTATESEALER_H
#define GUCEF_COM_CMCPREQUESTSTATESEALER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_COM_MCPTYPES_H
#include "gucefCOM_McpTypes.h"
#define GUCEF_COM_MCPTYPES_H
#endif /* GUCEF_COM_MCPTYPES_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Seals the state a handler gives with an input_required result into the opaque requestState text the client
 *  echoes on its retry, and verifies and unseals it then. The spec treats the echoed text as attacker controlled:
 *  a seal that fails verification, belongs to another principal or request, or has expired is refused.
 *
 *  Two protections, chosen with a setting:
 *  - HMAC-SHA256 (MacCodec "hmac-sha256" of the codec registry): integrity only, the client can read the state.
 *        base64url( payload ) "." base64url( HMAC( payload ) )
 *        payload: 0x01, flags, expiry (UInt64 Unix ms, big endian), binding (32 bytes), [nonce (16 bytes)], state JSON
 *  - AEAD (an EncryptionCodec of the codec registry): integrity and confidentiality.
 *        base64url( 0x02, algorithm, nonce, ciphertext and tag )
 *        plaintext: flags, expiry, [nonce], state JSON; associated data: 0x02, algorithm, binding
 *  The binding is SHA-256 over the principal subject, the method, the target and the digest of the arguments, each
 *  followed by a 0x00 byte. The MAC and the AEAD key are derived from one 32 byte key with HKDF-SHA256.
 *  A handler must not put secrets in the state, whichever protection is used: the protection may be HMAC.
 *
 *  Seal() and Unseal() are thread safe. Seals do not stop a replay within their lifetime.
 */
class GUCEF_COM_PUBLIC_CPP CMcpRequestStateSealer
{
    public:

    enum EProtection
    {
        PROTECTION_AUTO         ,   /**< AEAD when an AEAD codec is registered, else HMAC-SHA256 */
        PROTECTION_HMAC_SHA256  ,
        PROTECTION_AEAD             /**< refused when no AEAD codec is registered */
    };

    enum EUnsealResult
    {
        UNSEAL_OK               ,
        UNSEAL_MALFORMED        ,   /**< not something this sealer made */
        UNSEAL_TAMPERED         ,   /**< the MAC or the tag does not match; with AEAD also another binding */
        UNSEAL_EXPIRED          ,
        UNSEAL_WRONG_BINDING    ,   /**< another principal, method, target or arguments */
        UNSEAL_NOT_READY            /**< Initialize() did not succeed */
    };

    /** What a state is bound to */
    struct GUCEF_COM_PUBLIC_CPP SBinding
    {
        CORE::CString subject;
        CORE::CString method;
        CORE::CString target;
        UInt8 argumentsDigest[ 32 ];

        SBinding( void );
    };

    static const UInt32 KeySize = 32;
    static const UInt32 NonceSize = 16;
    static const UInt32 MaxSealedBytes = 16384;     /**< a sealed state may be at most 16 KiB of text */

    /** The AEAD codec types tried in order: "xchacha20-poly1305,chacha20-poly1305,aes-256-gcm" */
    static const CORE::CString DefaultAeadCodecTypes;

    CMcpRequestStateSealer( void );

    ~CMcpRequestStateSealer();

    /**
     *  Prepares the codec instances
     *  @param aeadCodecTypes   the EncryptionCodec types to try, in order, for AEAD
     *  @param key              32 bytes; GUCEF_NULL: random, from the secure random source of the platform
     */
    bool Initialize( EProtection protection                         ,
                     const CORE::CString::StringVector& aeadCodecTypes ,
                     const CORE::CDynamicBuffer* key                ,
                     CORE::CString& error                           );

    bool IsInitialized( void ) const;

    /** What is used: never PROTECTION_AUTO once initialized */
    EProtection GetProtection( void ) const;

    /** The EncryptionCodec type in use with AEAD */
    const CORE::CString& GetAeadCodecType( void ) const;

    bool Seal( const SBinding& binding              ,
               const CORE::CDataNode& state         ,
               bool withNonce                       ,
               UInt64 expiryUnixMs                  ,
               CORE::CString& sealed                ,
               CORE::CString& error                 ) const;

    /** @param nonceHex the nonce of the state as hexadecimal text, empty when it has none */
    EUnsealResult Unseal( const CORE::CString& sealed       ,
                          const SBinding& binding           ,
                          UInt64 nowUnixMs                  ,
                          CORE::CDataNode& state            ,
                          CORE::CString& nonceHex           ) const;

    /** SHA-256 of the JSON text CJsonRpcEndpoint::ToJsonText() writes for the arguments; GUCEF_NULL: of "{}" */
    static void DigestArguments( const CORE::CDataNode* arguments, UInt8* digest );

    /** A key file holds the 32 bytes raw, as base64url text or as 64 hexadecimal digits */
    static bool LoadKeyFile( const CORE::CString& path, CORE::CDynamicBuffer& key, CORE::CString& error );

    /** "auto", "hmac-sha256", "aead" */
    static bool ParseProtection( const CORE::CString& text, EProtection& protection );

    static const char* ProtectionToString( EProtection protection );

    private:

    CMcpRequestStateSealer( const CMcpRequestStateSealer& src );
    CMcpRequestStateSealer& operator=( const CMcpRequestStateSealer& src );

    static void ComputeBinding( const SBinding& binding, UInt8* bindingDigest );

    bool m_initialized;
    EProtection m_protection;
    CORE::CString m_aeadCodecType;
    UInt8 m_aeadAlgorithmId;
    UInt32 m_aeadNonceSize;
    mutable MT::CMutex m_lock;
    mutable CORE::CICodecInstancePtr m_mac;      /**< used under m_lock: an instance is used by one thread at a time */
    mutable CORE::CICodecInstancePtr m_aead;
    mutable CORE::CDynamicBuffer m_work;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMCPREQUESTSTATESEALER_H ? */
