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

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

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
 *  "token": a token { session, user id, role, expiry } issued by a lobby service.
 *
 *  Signed tokens (version 2): u8 version (2), string session, string userId, string role, varint expiry (UTC seconds
 *  since the Unix epoch), bytes userPublicKey (0 or 32), bytes policyHash (0 or 32), then the 64 byte Ed25519
 *  signature of the issuer over everything before it. Checked with the issuer's public key only, so whoever checks
 *  cannot make tokens.
 *  HMAC tokens (version 3): u8 version (3), string session, string userId, string role, varint expiry, bytes
 *  policyHash (0 or 32), then 32 bytes HMAC-SHA256 with a key that issuer and checker share. Version 1, without the
 *  policy hash, is still read. The text form of all is base64.
 *  policyHash is the CDRNSecurityPolicy::GetPolicyHash() of the game the token is for: a token with another hash
 *  is refused, a token without one only when the policy does not require it (requireTokenPolicyHash).
 *  A token travels in AUTH, which is protected when the handshake agreed on a cipher: it is bound to the connection
 *  and cannot be read or used on another one.
 *
 *  Settings: "token" (base64, connecting side); accepting side "issuerPublicKey" (hex: only signed tokens are taken)
 *  or "key" (HMAC tokens); "expectedSession" (optional: the session a token must be for).
 */
class GUCEF_DRN_EXPORT_CPP CDRNTokenAuthenticator : public CIDRNAuthenticator
{
    public:

    static const CORE::CString MethodName;

    CDRNTokenAuthenticator( void );

    virtual ~CDRNTokenAuthenticator();

    virtual CORE::CString GetMethodName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateProof( const CDRNAuthContext& context, CORE::CDynamicBuffer& proof ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool VerifyProof( const CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, CDRNAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Issues an HMAC token, as a lobby service would. Returns the base64 text form
     *  @param policyHash the policy hash (32 bytes) of the game the token is for, NULL for none
     */
    static CORE::CString IssueToken( const CORE::CString& key        ,
                                     const CORE::CString& session    ,
                                     const CORE::CString& userId     ,
                                     const CORE::CString& role       ,
                                     UInt64 expiryUnixSeconds        ,
                                     const UInt8* policyHash = GUCEF_NULL );

    /**
     *  Checks the MAC of a binary HMAC token and reads its fields
     *  @param policyHash receives the policy hash of the token (0 or 32 bytes) when given
     */
    static bool ParseToken( const CORE::CString& key ,
                            const UInt8* token       ,
                            UInt32 tokenSize         ,
                            CORE::CString& session   ,
                            CORE::CString& userId    ,
                            CORE::CString& role      ,
                            UInt64& expiryUnixSeconds,
                            CORE::CDynamicBuffer* policyHash = GUCEF_NULL );

    /**
     *  Issues a signed token with the issuer's Ed25519 key (the 32 byte seed), base64 text, empty when there is no
     *  signature codec. userPublicKey (32 bytes) may be NULL.
     */
    static CORE::CString IssueSignedToken( const UInt8* issuerPrivateKey   ,
                                           const CORE::CString& session    ,
                                           const CORE::CString& userId     ,
                                           const CORE::CString& role       ,
                                           UInt64 expiryUnixSeconds        ,
                                           const UInt8* userPublicKey = GUCEF_NULL ,
                                           const UInt8* policyHash = GUCEF_NULL    );

    /** Checks the signature of a binary signed token with the issuer's public key and reads its fields */
    static bool ParseSignedToken( const UInt8* issuerPublicKey   ,
                                  const UInt8* token             ,
                                  UInt32 tokenSize               ,
                                  CORE::CString& session         ,
                                  CORE::CString& userId          ,
                                  CORE::CString& role            ,
                                  UInt64& expiryUnixSeconds      ,
                                  CORE::CDynamicBuffer& userPublicKey ,
                                  CORE::CDynamicBuffer& policyHash    );

    /** UTC seconds since the Unix epoch */
    static UInt64 GetUnixTimeNow( void );

    /** The accepting side takes signed tokens only */
    bool IsVerifyingSignedTokens( void ) const;

    private:

    bool m_haveIssuerKey;
    UInt8 m_issuerPublicKey[ 32 ];
    CORE::CString m_key;
    CORE::CString m_token;
    CORE::CString m_expectedSession;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */
