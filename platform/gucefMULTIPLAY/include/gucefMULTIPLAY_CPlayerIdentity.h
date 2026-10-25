/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CIDATANODESERIALIZABLE_H
#include "gucefCORE_CIDataNodeSerializable.h"
#define GUCEF_CORE_CIDATANODESERIALIZABLE_H
#endif /* GUCEF_CORE_CIDATANODESERIALIZABLE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_MULTIPLAY_CLINKPROOF_H
#include "gucefMULTIPLAY_CLinkProof.h"
#define GUCEF_MULTIPLAY_CLINKPROOF_H
#endif /* GUCEF_MULTIPLAY_CLINKPROOF_H ? */

#ifndef GUCEF_MULTIPLAY_MACROS_H
#include "gucefMULTIPLAY_macros.h"
#define GUCEF_MULTIPLAY_MACROS_H
#endif /* GUCEF_MULTIPLAY_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Who the local player is: a stable id per profile, the name others see and the accounts at services.
 *
 *  An identity with a key pair (Ed25519) is bound to its key: the user id follows from the public key
 *  (MakeUserId()), so a server that checked a signature of the key knows the user id is the player's own.
 *  The private key never leaves the process through Serialize(): CLocalIdentityProvider keeps it in the profile
 *  itself, or in the credential store of gucefAUTH. The key pair needs a SignatureCodec "ed25519"
 *  (codecspluginMONOCYPHER or codecspluginOPENSSL).
 *
 *  accounts: the backend name is the key and the account id its value. Accounts at identity providers that are linked
 *  to the profile (CAuthIdentityProvider) are in it with the provider namespace as the backend name and the subject
 *  as the account id, so "epic" and "0123456789abcdef0123456789abcdef" is the account epic:0123456789abcdef0123456789abcdef; two accounts at one
 *  provider are two values of one key (the list then allows multiple values, Serialize() keeps the first of them).
 *
 *  linkProofs: the proofs of the logins behind those accounts, for the services that show them to a server together
 *  with the proof of the key. Like the private key they are not serialized, and a public identity has none.
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CPlayerIdentity : public CORE::CIDataNodeSerializable
{
    public:

    static const UInt32 KeySize = 32;           /**< of the public key and of the private key (the RFC 8032 seed) */
    static const UInt32 SignatureSize = 64;

    CORE::CString userId;               /**< MakeUserId() of the public key; a GUID for an identity without a key */
    CORE::CString displayName;
    CORE::CValueList accounts;          /**< backend name -> account id */
    CORE::CString publicKey;            /**< the Ed25519 public key in hex, empty when the identity has no key pair */
    CORE::CString privateKey;           /**< the Ed25519 seed in hex, not serialized */
    CORE::CString previousUserId;       /**< the GUID of a profile from before it had a key pair */
    TLinkProofVector linkProofs;        /**< proofs of the logins of the linked accounts, not serialized */

    CPlayerIdentity( void );

    CPlayerIdentity( const CPlayerIdentity& src );

    virtual ~CPlayerIdentity();

    CPlayerIdentity& operator=( const CPlayerIdentity& src );

    virtual bool Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings ) GUCEF_VIRTUAL_OVERRIDE;

    virtual CORE::CICloneable* Clone( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const char* GetCloneableTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** The name GetCloneableTypeName() says, see CORE::CICloneable::CloneableAs() */
    static const char* CloneableTypeName( void );

    /** A new random version 4 GUID as text, for example 3f2504e0-4f89-41d3-9a0c-0305e82c3301 */
    static CORE::CString MakeGuid( void );

    /**
     *  The user id of a public key: the first 16 bytes of its SHA-256 in the text form of a GUID
     *  @param publicKey KeySize bytes
     */
    static CORE::CString MakeUserId( const UInt8* publicKey );

    /** MakeUserId() of a public key in hex, empty when the text is not a key */
    static CORE::CString MakeUserIdFromHex( const CORE::CString& publicKeyHex );

    /** Whether the signature codec the key pair needs is there */
    static bool IsKeyPairAvailable( void );

    /** Makes a new key pair and the user id that goes with it; false without randomness or a signature codec */
    bool GenerateKeyPair( void );

    /** Sets the key pair from the private key in hex, with the public key and the user id that go with it */
    bool SetPrivateKey( const CORE::CString& privateKeyHex );

    /** Whether the identity has a key pair: a private key and the public key of it */
    bool HasKeyPair( void ) const;

    /** Whether the user id is the one of the public key */
    bool IsUserIdOfKey( void ) const;

    /** Signs the data with the private key, signature receives SignatureSize bytes */
    bool Sign( const void* data, UInt32 dataSize, UInt8* signature ) const;

    /** The public key as bytes, publicKeyBytes receives KeySize bytes */
    bool GetPublicKey( UInt8* publicKeyBytes ) const;

    /** A copy without the private key and the link proofs, for passing the identity on */
    CPlayerIdentity GetPublicIdentity( void ) const;

    private:

    /** 16 bytes as the text form of a GUID */
    static CORE::CString GuidText( const UInt8* bytes );
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */
