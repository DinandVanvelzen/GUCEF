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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefMULTIPLAY_CGucefLobbyRelayAuthenticator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGucefLobbyRelayAuthenticator::CGucefLobbyRelayAuthenticator( void )
    : DRN::CIDRNAuthenticator()
    , m_issuerPublicKey()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefLobbyRelayAuthenticator::~CGucefLobbyRelayAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyRelayAuthenticator::GetMethodName( void ) const
{GUCEF_TRACE;

    return DRN::CDRNTokenAuthenticator::MethodName;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyRelayAuthenticator::CreateProof( const DRN::CDRNAuthContext& /* context */, CORE::CDynamicBuffer& proof )
{GUCEF_TRACE;

    proof.SetDataSize( 0 );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyRelayAuthenticator::VerifyProof( const DRN::CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, DRN::CDRNAuthResult& result )
{GUCEF_TRACE;

    // The proof of the "token" method is the token: its version and the name of its session come before what is
    // signed is checked, the name tells which key to check it with
    CORE::CString session;
    {
        DRN::CDRNWireReader reader( proof, proofSize );
        UInt8 version = 0;
        if ( GUCEF_NULL == proof || !reader.ReadUInt8( version ) || !reader.ReadString( session ) ||
             !session.StartsWith( CGucefLobbyProtocol::GameSessionPrefix ) )
        {
            result.refusalText = "not a token of a game session";
            return false;
        }
    }

    UInt8 issuerKey[ DRN::CDRNCrypto::KeySize ];
    CORE::CString tokenSession;
    CORE::CString userId;
    CORE::CString role;
    UInt64 expiry = 0;
    CORE::CDynamicBuffer userKey;
    CORE::CDynamicBuffer policyHash;
    if ( !DRN::CDRNCrypto::FromHex( m_issuerPublicKey, issuerKey, sizeof( issuerKey ) ) ||
         !DRN::CDRNTokenAuthenticator::ParseSignedToken( issuerKey, proof, proofSize, tokenSession, userId, role, expiry, userKey, policyHash ) ||
         tokenSession != session )
    {
        result.refusalText = "invalid token";
        return false;
    }
    if ( expiry <= DRN::CDRNTokenAuthenticator::GetUnixTimeNow() )
    {
        result.refusalText = "token expired";
        return false;
    }
    // The token holds the policy of its game, which may be stricter than the relay's: the server signs tokens for the
    // policies of the games it launches only, none weaker than its minimum. The links of the game over the relay run
    // under the game's policy and the game checks the token against it there
    if ( 0 == policyHash.GetDataSize() ? context.requirePolicyHash : sizeof( context.policyHash ) != policyHash.GetDataSize() )
    {
        result.refusalText = "the token has no valid security policy hash";
        return false;
    }
    result.publicKey = userKey;
    result.userId = userId;
    result.role = role;
    result.sessionName = session;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyRelayAuthenticator::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    return m_issuerPublicKey.IsNULLOrEmpty() || cfg.SetAttribute( "issuerPublicKey", m_issuerPublicKey );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyRelayAuthenticator::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_issuerPublicKey = cfg.GetAttributeValueOrChildValueByName( "issuerPublicKey" ).AsString( m_issuerPublicKey, true );
    return !m_issuerPublicKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyRelayAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobbyRelayAuthenticator";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyRelayAuthenticator::MakeRelaySettings( const CORE::CString& issuerPublicKey, DRN::CDRNAuthSettings& settings )
{GUCEF_TRACE;

    settings = DRN::CDRNAuthSettings();
    settings.authenticatorType = CGucefLobbyProtocol::RelayAuthenticatorType;
    settings.SetSetting( "issuerPublicKey", CORE::CVariant( issuerPublicKey ) );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
