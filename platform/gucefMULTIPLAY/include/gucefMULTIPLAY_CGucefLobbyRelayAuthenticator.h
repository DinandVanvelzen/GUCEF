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

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H
#define GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H

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
 *  The DRN authenticator "gucef-lobby-relay" of the relay which the built-in lobby server offers the game sessions
 *  it launches. The nodes of a game session authenticate with the "token" method and the token of their launch
 *  description, which the server signed with its Ed25519 key. The relay checks the signature with the server's public
 *  key only, for every game session of the server. The token carries the policy of its game, which may differ from
 *  the relay's: the link with the relay runs under the relay's policy, the game's links over it under the game's,
 *  where the game checks the policy of the token. A token without a policy is refused when the relay's policy
 *  requires one. It only checks proofs: nodes reach the relay, the relay reaches no one.
 *
 *  Settings: issuerPublicKey (the server's public key in hex)
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGucefLobbyRelayAuthenticator : public DRN::CIDRNAuthenticator
{
    public:

    CGucefLobbyRelayAuthenticator( void );

    virtual ~CGucefLobbyRelayAuthenticator();

    /** The "token" method of the nodes of the game sessions */
    virtual CORE::CString GetMethodName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Always false: the relay does not authenticate itself to others */
    virtual bool CreateProof( const DRN::CDRNAuthContext& context, CORE::CDynamicBuffer& proof ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool VerifyProof( const DRN::CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, DRN::CDRNAuthResult& result ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& cfg ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& cfg ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Settings for the relay of the server: the public key of the server in hex */
    static void MakeRelaySettings( const CORE::CString& issuerPublicKey, DRN::CDRNAuthSettings& settings );

    private:

    CORE::CString m_issuerPublicKey;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< DRN::CIDRNAuthenticator, CGucefLobbyRelayAuthenticator, MT::CMutex > TGucefLobbyRelayAuthenticatorFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYRELAYAUTHENTICATOR_H ? */
