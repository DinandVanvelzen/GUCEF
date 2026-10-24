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
 *      PROOFKIND_TOKEN     the signed token
 *      PROOFKIND_PASSWORD  32 bytes HMAC (no identity, refused when the server requires one)
 *      PROOFKIND_NONE      nothing (likewise)
 *  An identity proof gives the user id of the key (CPlayerIdentity::MakeUserId()) and the key, which the ban list
 *  is checked against; a token proof gives the user id, role, key and session of the token, and the server's access
 *  policy lets such a link join only that session.
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
        PROOFKIND_IDENTITY  = 3
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

    /** Settings for a client of a lobby */
    static void MakeTokenCredentials( const CORE::CString& token, DRN::CDRNAuthSettings& settings );

    /** Settings for the server: the public key its tokens are checked with (hex), its password and whether it requires identities */
    static void MakeServerSettings( const CORE::CString& issuerPublicKey, const CORE::CString& password, bool requireIdentity, DRN::CDRNAuthSettings& settings );

    /** What the identity proof signs: a domain text and the challenge of the connection */
    static void MakeIdentityChallenge( const DRN::CDRNAuthContext& context, CORE::CDynamicBuffer& message );

    private:

    bool VerifyPassword( const DRN::CDRNAuthContext& context, const UInt8* mac, DRN::CDRNAuthResult& result ) const;

    private:

    CORE::CString m_password;
    CORE::CString m_token;
    CORE::CString m_identityKey;
    CORE::CString m_issuerPublicKey;
    bool m_requireIdentity;
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
