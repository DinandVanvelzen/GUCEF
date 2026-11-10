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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_CAUTHGLOBAL_H
#include "gucefAUTH_CAuthGlobal.h"
#define GUCEF_AUTH_CAUTHGLOBAL_H
#endif /* GUCEF_AUTH_CAUTHGLOBAL_H ? */

#ifndef GUCEF_AUTH_CAUTHMANAGER_H
#include "gucefAUTH_CAuthManager.h"
#define GUCEF_AUTH_CAUTHMANAGER_H
#endif /* GUCEF_AUTH_CAUTHMANAGER_H ? */

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
    , m_linkProofs()
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

void
CGucefLobbyAuthenticator::MakeLinkChallenge( const DRN::CDRNAuthContext& context ,
                                             const CORE::CString& providerName   ,
                                             const CORE::CString& proofKind      ,
                                             const CORE::CString& proofData      ,
                                             CORE::CDynamicBuffer& message       )
{GUCEF_TRACE;

    static const char domain[] = "gucef-lobby-link-v1";
    message.SetDataSize( 0 );
    message.Append( domain, sizeof( domain ), true );
    // Each text with its terminating zero, so the provider and the kind cannot be moved into one another
    message.Append( providerName.C_String(), providerName.ByteSize(), true );
    message.Append( proofKind.C_String(), proofKind.ByteSize(), true );
    UInt8 digest[ DRN::CDRNCrypto::HashSize ];
    DRN::CDRNCrypto::Sha256( proofData.C_String(), proofData.ByteSize() - 1, digest );
    message.Append( digest, sizeof( digest ), true );
    CORE::CDynamicBuffer challenge;
    MakeIdentityChallenge( context, challenge );
    message.Append( challenge.GetConstBufferPtr(), challenge.GetDataSize(), true );
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
        // Link proofs the client shows; the ones that cannot be sent are left out
        TLinkProofVector sendable;
        for ( size_t p=0; p<m_linkProofs.size() && sendable.size() < CLinkProof::MaxCount; ++p )
        {
            if ( m_linkProofs[ p ].IsSendable() )
                sendable.push_back( m_linkProofs[ p ] );
        }

        UInt8 header[ 2 ] = { (UInt8)( sendable.empty() ? PROOFKIND_IDENTITY : PROOFKIND_IDENTITY_LINKED ), (UInt8)( hasPassword ? 1 : 0 ) };
        proof.Append( header, sizeof( header ), true );
        if ( hasPassword )
            proof.Append( mac, sizeof( mac ), true );
        proof.Append( publicKey, sizeof( publicKey ), true );
        proof.Append( signature, sizeof( signature ), true );
        if ( sendable.empty() )
            return true;

        UInt8 count = (UInt8) sendable.size();
        proof.Append( &count, 1, true );
        for ( size_t p=0; p<sendable.size(); ++p )
        {
            const CORE::CString& providerName = sendable[ p ].providerName;
            const CORE::CString& kind = sendable[ p ].proof.kind;
            const CORE::CString& data = sendable[ p ].proof.data.Reveal();
            UInt8 providerSize = (UInt8)( providerName.ByteSize() - 1 );
            UInt8 kindSize = (UInt8)( kind.ByteSize() - 1 );
            UInt32 dataSize = data.ByteSize() - 1;
            UInt8 dataSizeBytes[ 2 ] = { (UInt8)( ( dataSize >> 8 ) & 0xFF ), (UInt8)( dataSize & 0xFF ) };

            CORE::CDynamicBuffer linkMessage;
            UInt8 linkSignature[ CPlayerIdentity::SignatureSize ];
            MakeLinkChallenge( context, providerName, kind, data, linkMessage );
            if ( !identity.Sign( linkMessage.GetConstBufferPtr(), linkMessage.GetDataSize(), linkSignature ) )
                return false;

            proof.Append( &providerSize, 1, true );
            proof.Append( providerName.C_String(), providerSize, true );
            proof.Append( &kindSize, 1, true );
            proof.Append( kind.C_String(), kindSize, true );
            proof.Append( dataSizeBytes, sizeof( dataSizeBytes ), true );
            proof.Append( data.C_String(), dataSize, true );
            proof.Append( linkSignature, sizeof( linkSignature ), true );
        }
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

void
CGucefLobbyAuthenticator::VerifyLinkProofs( const DRN::CDRNAuthContext& context ,
                                            const UInt8* publicKey              ,
                                            const UInt8* proofs                 ,
                                            UInt32 proofsSize                   ,
                                            DRN::CDRNAuthResult& result         ) const
{GUCEF_TRACE;

    // A proof is length prefixed, so one that is over a limit is stepped over; what cannot be stepped over ends it
    UInt32 offset = 0;
    UInt32 count = proofs[ offset++ ];
    if ( count > CLinkProof::MaxCount )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyAuthenticator: " << count << " link proofs shown, only the first " << CLinkProof::MaxCount << " are looked at" );
        count = CLinkProof::MaxCount;
    }

    AUTH::CAuthManager& manager = AUTH::CAuthGlobal::Instance()->GetManager();
    for ( UInt32 p=0; p<count; ++p )
    {
        UInt32 providerSize = 0, kindSize = 0, dataSize = 0;
        const UInt8* providerText = GUCEF_NULL;
        const UInt8* kindText = GUCEF_NULL;
        const UInt8* dataText = GUCEF_NULL;
        const UInt8* signature = GUCEF_NULL;

        if ( offset + 1 > proofsSize )
            break;
        providerSize = proofs[ offset++ ];
        if ( offset + providerSize + 1 > proofsSize ) break;
        providerText = proofs + offset;
        offset += providerSize;
        kindSize = proofs[ offset++ ];
        if ( offset + kindSize + 2 > proofsSize ) break;
        kindText = proofs + offset;
        offset += kindSize;
        dataSize = ( (UInt32) proofs[ offset ] << 8 ) | proofs[ offset + 1 ];
        offset += 2;
        if ( offset + dataSize + CPlayerIdentity::SignatureSize > proofsSize ) break;
        dataText = proofs + offset;
        offset += dataSize;
        signature = proofs + offset;
        offset += CPlayerIdentity::SignatureSize;

        if ( 0 == providerSize || providerSize > CLinkProof::MaxProviderNameLength ||
             0 == kindSize || kindSize > CLinkProof::MaxKindLength ||
             0 == dataSize || dataSize > CLinkProof::MaxDataSize )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyAuthenticator: A link proof is over the limits, it is ignored" );
            continue;
        }

        CORE::CString providerName; providerName.Set( reinterpret_cast< const char* >( providerText ), providerSize );
        CORE::CString kind; kind.Set( reinterpret_cast< const char* >( kindText ), kindSize );
        CORE::CString data; data.Set( reinterpret_cast< const char* >( dataText ), dataSize );

        // The key of the player vouches for this proof on this connection, before anything is asked of the provider
        CORE::CDynamicBuffer message;
        MakeLinkChallenge( context, providerName, kind, data, message );
        if ( !DRN::CDRNCrypto::Verify( publicKey, message.GetConstBufferPtr(), message.GetDataSize(), signature ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyAuthenticator: The signature of the link proof for \"" << providerName << "\" does not hold, it is ignored" );
            continue;
        }

        AUTH::CIAuthProvider* provider = manager.GetProvider( providerName );
        if ( GUCEF_NULL == provider || 0 == ( provider->GetCapabilities() & AUTH::AUTHCAP_SERVER_VERIFIABLE ) )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyAuthenticator: There is no provider \"" << providerName << "\" that can verify a link proof, it is ignored" );
            continue;
        }

        AUTH::CAuthResult verified;
        if ( !provider->Verify( AUTH::CAuthProof( kind, data ), verified ) || !verified.accountRef.IsValid() )
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "GucefLobbyAuthenticator: The provider \"" << providerName << "\" does not accept the link proof, it is ignored" );
            continue;
        }

        CORE::CString account = verified.accountRef.ToString();
        bool known = false;
        for ( size_t a=0; a<result.accounts.size(); ++a )
            known = known || result.accounts[ a ] == account;
        if ( !known )
            result.accounts.push_back( account );
    }
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
        case PROOFKIND_IDENTITY_LINKED:
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
            // The identity proof is of exactly this size, the one with link proofs has them after it
            if ( PROOFKIND_IDENTITY == proof[ 0 ] ? proofSize != expectedSize : proofSize <= expectedSize )
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
            if ( PROOFKIND_IDENTITY_LINKED == proof[ 0 ] )
            {
                // What does not verify is not an account; it does not refuse the connection
                result.accounts.clear();
                VerifyLinkProofs( context, publicKey, proof + expectedSize, proofSize - expectedSize, result );
            }
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
CGucefLobbyAuthenticator::MakeDirectoryCredentials( const CORE::CString& identityKey, const CORE::CString& password, const TLinkProofVector& linkProofs, DRN::CDRNAuthSettings& settings )
{GUCEF_TRACE;

    MakeDirectoryCredentials( identityKey, password, settings );
    if ( identityKey.IsNULLOrEmpty() )
        return;
    UInt32 added = 0;
    for ( size_t i=0; i<linkProofs.size() && added < CLinkProof::MaxCount; ++i )
    {
        if ( !linkProofs[ i ].IsSendable() )
            continue;
        CORE::CDataNode* child = settings.authenticatorConfig.AddChild( "LinkProof", GUCEF_DATATYPE_OBJECT );
        if ( GUCEF_NULL == child )
            continue;
        child->SetAttribute( "provider", linkProofs[ i ].providerName );
        child->SetAttribute( "kind", linkProofs[ i ].proof.kind );
        child->SetAttribute( "data", linkProofs[ i ].proof.data.Reveal() );
        ++added;
    }
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

    // The link proofs of a client, children "LinkProof" with provider, kind and data
    m_linkProofs.clear();
    CORE::CDataNode::const_iterator i = cfg.ConstBegin();
    while ( i != cfg.ConstEnd() && m_linkProofs.size() < CLinkProof::MaxCount )
    {
        const CORE::CDataNode* child = (*i);
        if ( GUCEF_NULL != child && child->GetName() == "LinkProof" )
        {
            CLinkProof linkProof( child->GetAttributeValueOrChildValueByName( "provider" ).AsString( CORE::CString::Empty, true ),
                                  AUTH::CAuthProof( child->GetAttributeValueOrChildValueByName( "kind" ).AsString( CORE::CString::Empty, true ),
                                                    child->GetAttributeValueOrChildValueByName( "data" ).AsString( CORE::CString::Empty, true ) ) );
            if ( linkProof.IsSendable() )
                m_linkProofs.push_back( linkProof );
        }
        ++i;
    }
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
