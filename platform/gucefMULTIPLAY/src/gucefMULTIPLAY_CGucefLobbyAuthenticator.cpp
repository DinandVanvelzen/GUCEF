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

#ifndef GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H
#include "gucefDRN_CDRNSharedSecretAuthenticator.h"
#define GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNSHAREDSECRETAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#include "gucefDRN_CDRNTokenAuthenticator.h"
#define GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H
#endif /* GUCEF_DRN_CDRNTOKENAUTHENTICATOR_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#ifndef GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#include "gucefMULTIPLAY_CGucefLobbyProtocol.h"
#define GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H
#endif /* GUCEF_MULTIPLAY_CGUCEFLOBBYPROTOCOL_H ? */

#include "gucefMULTIPLAY_CGucefLobbyAuthenticator.h"

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

CGucefLobbyAuthenticator::CGucefLobbyAuthenticator( void )
    : DRN::CIDRNAuthenticator()
    , m_password()
    , m_token()
    , m_identityKey()
    , m_issuerPublicKey()
    , m_requireIdentity( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGucefLobbyAuthenticator::~CGucefLobbyAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CGucefLobbyAuthenticator::GetMethodName( void ) const
{GUCEF_TRACE;

    return CGucefLobbyProtocol::AuthenticatorType;
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyAuthenticator::MakeIdentityChallenge( const DRN::CDRNAuthContext& context, CORE::CDynamicBuffer& message )
{GUCEF_TRACE;

    static const char domain[] = "gucef-lobby-identity-v1";
    message.SetDataSize( 0 );
    message.Append( domain, sizeof( domain ), true );
    message.Append( context.nonce, sizeof( context.nonce ), true );
    message.Append( context.transcriptHash, sizeof( context.transcriptHash ), true );
    // Each text with its terminating zero, so no two sets of texts give the same bytes
    message.Append( context.serviceName.C_String(), context.serviceName.ByteSize(), true );
    message.Append( context.initiatorNodeId.C_String(), context.initiatorNodeId.ByteSize(), true );
    message.Append( context.acceptorNodeId.C_String(), context.acceptorNodeId.ByteSize(), true );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyAuthenticator::CreateProof( const DRN::CDRNAuthContext& context, CORE::CDynamicBuffer& proof )
{GUCEF_TRACE;

    proof.SetDataSize( 0 );
    if ( !m_token.IsNULLOrEmpty() )
    {
        UInt32 maxSize = m_token.ByteSize() + 1;
        proof.SetDataSize( maxSize );
        UInt8* bytes = static_cast< UInt8* >( proof.GetBufferPtr() );
        bytes[ 0 ] = (UInt8) PROOFKIND_TOKEN;
        UInt32 used = 0;
        if ( !CORE::Base64Decode( m_token, bytes + 1, maxSize - 1, used ) )
            return false;
        proof.SetDataSize( used + 1 );
        return true;
    }

    UInt8 mac[ CORE::CSha256::DigestSize ];
    bool hasPassword = !m_password.IsNULLOrEmpty();
    if ( hasPassword )
        DRN::CDRNSharedSecretAuthenticator::ComputeProof( m_password, context, mac );

    if ( !m_identityKey.IsNULLOrEmpty() )
    {
        CPlayerIdentity identity;
        if ( !identity.SetPrivateKey( m_identityKey ) )
            return false;
        UInt8 publicKey[ CPlayerIdentity::KeySize ];
        UInt8 signature[ CPlayerIdentity::SignatureSize ];
        CORE::CDynamicBuffer message;
        MakeIdentityChallenge( context, message );
        if ( !identity.GetPublicKey( publicKey ) || !identity.Sign( message.GetConstBufferPtr(), message.GetDataSize(), signature ) )
            return false;
        UInt8 header[ 2 ] = { (UInt8) PROOFKIND_IDENTITY, (UInt8)( hasPassword ? 1 : 0 ) };
        proof.Append( header, sizeof( header ), true );
        if ( hasPassword )
            proof.Append( mac, sizeof( mac ), true );
        proof.Append( publicKey, sizeof( publicKey ), true );
        proof.Append( signature, sizeof( signature ), true );
        return true;
    }

    if ( hasPassword )
    {
        UInt8 kind = (UInt8) PROOFKIND_PASSWORD;
        proof.Append( &kind, 1, true );
        proof.Append( mac, sizeof( mac ), true );
        return true;
    }
    UInt8 none = (UInt8) PROOFKIND_NONE;
    proof.Append( &none, 1, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyAuthenticator::VerifyPassword( const DRN::CDRNAuthContext& context, const UInt8* mac, DRN::CDRNAuthResult& result ) const
{GUCEF_TRACE;

    if ( m_password.IsNULLOrEmpty() )
        return true;
    if ( GUCEF_NULL == mac )
    {
        result.refusalText = "the server wants a password";
        return false;
    }
    UInt8 expected[ CORE::CSha256::DigestSize ];
    DRN::CDRNSharedSecretAuthenticator::ComputeProof( m_password, context, expected );
    if ( !CORE::CSha256::ConstantTimeEquals( expected, mac, CORE::CSha256::DigestSize ) )
    {
        result.refusalText = "wrong password";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyAuthenticator::VerifyProof( const DRN::CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, DRN::CDRNAuthResult& result )
{GUCEF_TRACE;

    if ( GUCEF_NULL == proof || 0 == proofSize )
    {
        result.refusalText = "malformed proof";
        return false;
    }

    switch ( proof[ 0 ] )
    {
        case PROOFKIND_TOKEN:
        {
            UInt8 issuerKey[ DRN::CDRNCrypto::KeySize ];
            CORE::CString session, userId, role;
            UInt64 expiry = 0;
            CORE::CDynamicBuffer userKey;
            CORE::CDynamicBuffer policyHash;
            if ( !DRN::CDRNCrypto::FromHex( m_issuerPublicKey, issuerKey, sizeof( issuerKey ) ) ||
                 !DRN::CDRNTokenAuthenticator::ParseSignedToken( issuerKey, proof + 1, proofSize - 1, session, userId, role, expiry, userKey, policyHash ) )
            {
                result.refusalText = "invalid token";
                return false;
            }
            if ( expiry <= DRN::CDRNTokenAuthenticator::GetUnixTimeNow() )
            {
                result.refusalText = "token expired";
                return false;
            }
            if ( 0 == policyHash.GetDataSize() ? context.requirePolicyHash :
                 ( sizeof( context.policyHash ) != policyHash.GetDataSize() ||
                   !DRN::CDRNCrypto::ConstantTimeEquals( policyHash.GetConstBufferPtr(), context.policyHash, sizeof( context.policyHash ) ) ) )
            {
                result.refusalText = "the token is for another security policy";
                return false;
            }
            result.userId = userId;
            result.role = role;
            result.sessionName = session;
            result.publicKey = userKey;
            return true;
        }
        case PROOFKIND_IDENTITY:
        {
            const UInt32 keySize = CPlayerIdentity::KeySize;
            const UInt32 signatureSize = CPlayerIdentity::SignatureSize;
            const UInt32 macSize = CORE::CSha256::DigestSize;
            if ( proofSize < 2 )
            {
                result.refusalText = "malformed proof";
                return false;
            }
            bool hasPassword = 0 != proof[ 1 ];
            UInt32 expectedSize = 2 + ( hasPassword ? macSize : 0 ) + keySize + signatureSize;
            if ( proofSize != expectedSize )
            {
                result.refusalText = "malformed proof";
                return false;
            }
            const UInt8* mac = hasPassword ? proof + 2 : GUCEF_NULL;
            const UInt8* publicKey = proof + 2 + ( hasPassword ? macSize : 0 );
            const UInt8* signature = publicKey + keySize;
            if ( !VerifyPassword( context, mac, result ) )
                return false;
            CORE::CDynamicBuffer message;
            MakeIdentityChallenge( context, message );
            if ( !DRN::CDRNCrypto::Verify( publicKey, message.GetConstBufferPtr(), message.GetDataSize(), signature ) )
            {
                result.refusalText = "the identity proof does not hold";
                return false;
            }
            result.userId = CPlayerIdentity::MakeUserId( publicKey );
            result.role = "user";
            result.publicKey.SetDataSize( 0 );
            result.publicKey.Append( publicKey, keySize, true );
            return true;
        }
        case PROOFKIND_PASSWORD:
        case PROOFKIND_NONE:
        {
            if ( m_requireIdentity )
            {
                result.refusalText = "the server wants proof of an identity key";
                return false;
            }
            if ( PROOFKIND_PASSWORD == proof[ 0 ] && proofSize != CORE::CSha256::DigestSize + 1 )
            {
                result.refusalText = "malformed proof";
                return false;
            }
            return VerifyPassword( context, PROOFKIND_PASSWORD == proof[ 0 ] ? proof + 1 : GUCEF_NULL, result );
        }
        default:
        {
            result.refusalText = "malformed proof";
            return false;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyAuthenticator::MakeDirectoryCredentials( const CORE::CString& identityKey, const CORE::CString& password, DRN::CDRNAuthSettings& settings )
{GUCEF_TRACE;

    settings = DRN::CDRNAuthSettings();
    settings.authenticatorType = CGucefLobbyProtocol::AuthenticatorType;
    settings.SetSetting( "password", CORE::CVariant( password ) );
    settings.SetSetting( "identityKey", CORE::CVariant( identityKey ) );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyAuthenticator::MakeTokenCredentials( const CORE::CString& token, DRN::CDRNAuthSettings& settings )
{GUCEF_TRACE;

    settings = DRN::CDRNAuthSettings();
    settings.authenticatorType = CGucefLobbyProtocol::AuthenticatorType;
    settings.SetSetting( "token", CORE::CVariant( token ) );
}

/*-------------------------------------------------------------------------*/

void
CGucefLobbyAuthenticator::MakeServerSettings( const CORE::CString& issuerPublicKey, const CORE::CString& password, bool requireIdentity, DRN::CDRNAuthSettings& settings )
{GUCEF_TRACE;

    settings = DRN::CDRNAuthSettings();
    settings.authenticatorType = CGucefLobbyProtocol::AuthenticatorType;
    settings.SetSetting( "issuerPublicKey", CORE::CVariant( issuerPublicKey ) );
    settings.SetSetting( "password", CORE::CVariant( password ) );
    settings.SetSetting( "requireIdentity", CORE::CVariant( requireIdentity ) );
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyAuthenticator::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "issuerPublicKey", m_issuerPublicKey );
    cfg.SetAttribute( "password", m_password );
    cfg.SetAttribute( "requireIdentity", m_requireIdentity );
    cfg.SetAttribute( "identityKey", m_identityKey );
    cfg.SetAttribute( "token", m_token );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGucefLobbyAuthenticator::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_issuerPublicKey = cfg.GetAttributeValueOrChildValueByName( "issuerPublicKey" ).AsString( m_issuerPublicKey, true );
    m_password = cfg.GetAttributeValueOrChildValueByName( "password" ).AsString( m_password, true );
    m_requireIdentity = cfg.GetAttributeValueOrChildValueByName( "requireIdentity" ).AsBool( m_requireIdentity, true );
    m_identityKey = cfg.GetAttributeValueOrChildValueByName( "identityKey" ).AsString( m_identityKey, true );
    m_token = cfg.GetAttributeValueOrChildValueByName( "token" ).AsString( m_token, true );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGucefLobbyAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGucefLobbyAuthenticator";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
