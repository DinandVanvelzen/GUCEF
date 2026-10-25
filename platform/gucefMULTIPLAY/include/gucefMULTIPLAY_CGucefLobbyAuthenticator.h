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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CTFACTORY_H
#include "CTFactory.h"
#define GUCEF_CORE_CTFACTORY_H
#endif /* GUCEF_CORE_CTFACTORY_H ? */

#ifndef GUCEF_DRN_CIDRNAUTHENTICATOR_H
#include "gucefDRN_CIDRNAuthenticator.h"
#define GUCEF_DRN_CIDRNAUTHENTICATOR_H
#endif /* GUCEF_DRN_CIDRNAUTHENTICATOR_H ? */

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
 *  The DRN authenticator "gucef-lobby" of the built-in lobby server, which is reached two ways over one node:
 *      the directory, with proof of the player's identity: a signature of the player's Ed25519 key over the
 *      challenge of the connection (its nonce, transcript hash, service and node ids), and the server password
 *      (an HMAC of the challenge, as "shared-secret" does) when the server has one;
 *      a lobby, with the token the directory issued (a DRN token signed with the server's Ed25519 key).
 *  The proof is one byte telling which, then the proof itself:
 *      PROOFKIND_IDENTITY  u8 has password, [32 bytes HMAC], 32 bytes public key, 64 bytes signature
 *      PROOFKIND_IDENTITY_LINKED   that of PROOFKIND_IDENTITY, followed by u8 count and per link proof: u8 length and
 *                          the provider name, u8 length and the kind of the proof, u16 (big endian) length and the
 *                          proof data, 64 bytes signature of the key over MakeLinkChallenge()
 *      PROOFKIND_TOKEN     the signed token
 *      PROOFKIND_PASSWORD  32 bytes HMAC (no identity, refused when the server requires one)
 *      PROOFKIND_NONE      nothing (likewise)
 *  An identity proof gives the user id of the key (CPlayerIdentity::MakeUserId()) and the key, which the ban list
 *  is checked against; a token proof gives the user id, role, key and session of the token, and the server's access
 *  policy lets such a link join only that session.
 *
 *  Link proofs show the server accounts at external identity providers (gucefAUTH) that belong to the player. Each is
 *  the proof of a login (an identity token, a ticket) with the signature of the player's key over the proof data and the
 *  challenge of this connection (MakeLinkChallenge()), so a proof made for one connection or one key is no proof on
 *  another. The server checks the signature against the key of the identity proof, finds the provider of gucefAUTH of
 *  the name (CAuthGlobal's manager) and lets it verify the proof (CIAuthProvider::Verify(), which runs on the thread of
 *  the node and can wait for the network on a cache miss of the provider's keys). What verifies gives an account in
 *  CDRNAuthResult::accounts, which the node checks against the ban list; what does not (a signature that does not hold,
 *  a proof for another connection or key, a proof the provider refuses or has expired, an unknown or not verifying
 *  provider, a proof over the size limits, a proof past the maximum number) is ignored: the connection is that of a
 *  player without that account, a guest when none verifies. Whether that is enough is the policy of the server
 *  (CGucefLobbyServerSettings::requireLinkedProvider). A link proof is carried only in an identity proof; a client
 *  without link proofs sends the proof it always did. The handshake proof has to fit one packet before the link is
 *  operational (stream channels: maxPreAuthPacketSize of the policy, 16 KB by default), the limits of CLinkProof keep it
 *  within that.
 *
 *  Settings: password (the directory password), the server's issuerPublicKey (hex) and requireIdentity (default
 *  true), a client's identityKey (the Ed25519 seed in hex) or token
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyAuthenticator : public DRN::CIDRNAuthenticator
{
    public:

    enum EProofKind
    {
        PROOFKIND_NONE      = 0,
        PROOFKIND_PASSWORD  = 1,
        PROOFKIND_TOKEN     = 2,
        PROOFKIND_IDENTITY  = 3,
        PROOFKIND_IDENTITY_LINKED = 4
    };

    CGucefLobbyAuthenticator( void );

    virtual ~CGucefLobbyAuthenticator();

    virtual CORE::CString GetMethodName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool CreateProof( const DRN::CDRNAuthContext& context, CORE::CDynamicBuffer& proof ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool VerifyProof( const DRN::CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, DRN::CDRNAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Settings for a client of the directory: its identity key (the Ed25519 seed in hex, empty for none) and the password */
    static void MakeDirectoryCredentials( const CORE::CString& identityKey, const CORE::CString& password, DRN::CDRNAuthSettings& settings );

    /**
     *  The same with the link proofs the client shows. Those that cannot be sent (see CLinkProof::IsSendable()) and
     *  the ones past CLinkProof::MaxCount are left out; without an identity key there are none
     */
    static void MakeDirectoryCredentials( const CORE::CString& identityKey, const CORE::CString& password, const TLinkProofVector& linkProofs, DRN::CDRNAuthSettings& settings );

    /** Settings for a client of a lobby */
    static void MakeTokenCredentials( const CORE::CString& token, DRN::CDRNAuthSettings& settings );

    /** Settings for the server: the public key its tokens are checked with (hex), its password and whether it requires identities */
    static void MakeServerSettings( const CORE::CString& issuerPublicKey, const CORE::CString& password, bool requireIdentity, DRN::CDRNAuthSettings& settings );

    /** What the identity proof signs: a domain text and the challenge of the connection */
    static void MakeIdentityChallenge( const DRN::CDRNAuthContext& context, CORE::CDynamicBuffer& message );

    /**
     *  What the key signs for a link proof: a domain text, the provider name and the kind of the proof, the SHA-256 of
     *  the proof data, and the identity challenge of the connection
     */
    static void MakeLinkChallenge( const DRN::CDRNAuthContext& context ,
                                   const CORE::CString& providerName   ,
                                   const CORE::CString& proofKind      ,
                                   const CORE::CString& proofData      ,
                                   CORE::CDynamicBuffer& message       );

    private:

    bool VerifyPassword( const DRN::CDRNAuthContext& context, const UInt8* mac, DRN::CDRNAuthResult& result ) const;
    void VerifyLinkProofs( const DRN::CDRNAuthContext& context, const UInt8* publicKey, const UInt8* proofs, UInt32 proofsSize, DRN::CDRNAuthResult& result ) const;

    private:

    CORE::CString m_password;
    CORE::CString m_token;
    CORE::CString m_identityKey;
    CORE::CString m_issuerPublicKey;
    bool m_requireIdentity;
    TLinkProofVector m_linkProofs;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< DRN::CIDRNAuthenticator, CGucefLobbyAuthenticator, MT::CMutex > TGucefLobbyAuthenticatorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYAUTHENTICATOR_H ? */
