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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <string.h>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_DRN_CDRNWIREWRITER_H
#include "gucefDRN_CDRNWireWriter.h"
#define GUCEF_DRN_CDRNWIREWRITER_H
#endif /* GUCEF_DRN_CDRNWIREWRITER_H ? */

#ifndef GUCEF_DRN_CDRNWIREREADER_H
#include "gucefDRN_CDRNWireReader.h"
#define GUCEF_DRN_CDRNWIREREADER_H
#endif /* GUCEF_DRN_CDRNWIREREADER_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#include "gucefDRN_CDRNTokenAuthenticator.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNTokenAuthenticator::MethodName = "token";

static const UInt8 TokenVersion = 1;
static const UInt8 SignedTokenVersion = 2;
static const UInt8 PolicyTokenVersion = 3;
static const UInt32 TokenPolicyHashSize = 32;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
ComputeTokenMac( const CORE::CString& key, const void* data, UInt32 size, UInt8* mac )
{GUCEF_TRACE;
    CORE::CUtf8String utf8Key = CORE::ToUtf8String( key );
    UInt32 keySize = utf8Key.ByteSize() > 0 ? utf8Key.ByteSize() - 1 : 0;
    CORE::CHmacSha256::Mac( utf8Key.C_String(), keySize, data, size, mac );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNTokenAuthenticator::CDRNTokenAuthenticator( void )
    : CIDRNAuthenticator()
    , m_haveIssuerKey( false )
    , m_key()
    , m_token()
    , m_expectedSession()
{GUCEF_TRACE;

    memset( m_issuerPublicKey, 0, sizeof( m_issuerPublicKey ) );
}

/*-------------------------------------------------------------------------*/

CDRNTokenAuthenticator::~CDRNTokenAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNTokenAuthenticator::GetMethodName( void ) const
{GUCEF_TRACE;

    return MethodName;
}

/*-------------------------------------------------------------------------*/

UInt64
CDRNTokenAuthenticator::GetUnixTimeNow( void )
{GUCEF_TRACE;

    return CORE::CDateTime::NowUTCDateTime().ToUnixEpochBasedTicksInMillisecs() / 1000;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNTokenAuthenticator::IssueToken( const CORE::CString& key        ,
                                    const CORE::CString& session    ,
                                    const CORE::CString& userId     ,
                                    const CORE::CString& role       ,
                                    UInt64 expiryUnixSeconds        ,
                                    const UInt8* policyHash         )
{GUCEF_TRACE;

    CORE::CDynamicBuffer token;
    CDRNWireWriter writer( token );
    writer.WriteUInt8( PolicyTokenVersion );
    writer.WriteString( session );
    writer.WriteString( userId );
    writer.WriteString( role );
    writer.WriteVarUInt64( expiryUnixSeconds );
    writer.WriteSizedBytes( policyHash, GUCEF_NULL != policyHash ? TokenPolicyHashSize : 0 );

    UInt8 mac[ CORE::CSha256::DigestSize ];
    ComputeTokenMac( key, token.GetConstBufferPtr(), writer.GetOffset(), mac );
    writer.WriteBytes( mac, CORE::CSha256::DigestSize );

    return CORE::Base64Encode( token.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNTokenAuthenticator::IssueSignedToken( const UInt8* issuerPrivateKey   ,
                                          const CORE::CString& session    ,
                                          const CORE::CString& userId     ,
                                          const CORE::CString& role       ,
                                          UInt64 expiryUnixSeconds        ,
                                          const UInt8* userPublicKey      ,
                                          const UInt8* policyHash         )
{GUCEF_TRACE;

    CORE::CDynamicBuffer token;
    CDRNWireWriter writer( token );
    writer.WriteUInt8( SignedTokenVersion );
    writer.WriteString( session );
    writer.WriteString( userId );
    writer.WriteString( role );
    writer.WriteVarUInt64( expiryUnixSeconds );
    writer.WriteSizedBytes( userPublicKey, GUCEF_NULL != userPublicKey ? CDRNCrypto::KeySize : 0 );
    writer.WriteSizedBytes( policyHash, GUCEF_NULL != policyHash ? TokenPolicyHashSize : 0 );

    UInt8 signature[ CDRNCrypto::SignatureSize ];
    if ( GUCEF_NULL == issuerPrivateKey || !CDRNCrypto::Sign( issuerPrivateKey, token.GetConstBufferPtr(), writer.GetOffset(), signature ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTokenAuthenticator: Cannot sign a token, is there an \"" << CDRNCrypto::SignatureType << "\" codec?" );
        return CORE::CString::Empty;
    }
    writer.WriteBytes( signature, CDRNCrypto::SignatureSize );
    return CORE::Base64Encode( token.GetConstBufferPtr(), writer.GetOffset() );
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::ParseSignedToken( const UInt8* issuerPublicKey   ,
                                          const UInt8* token             ,
                                          UInt32 tokenSize               ,
                                          CORE::CString& session         ,
                                          CORE::CString& userId          ,
                                          CORE::CString& role            ,
                                          UInt64& expiryUnixSeconds      ,
                                          CORE::CDynamicBuffer& userPublicKey ,
                                          CORE::CDynamicBuffer& policyHash    )
{GUCEF_TRACE;

    if ( GUCEF_NULL == token || GUCEF_NULL == issuerPublicKey || tokenSize <= CDRNCrypto::SignatureSize + 1 )
        return false;
    UInt32 signedSize = tokenSize - CDRNCrypto::SignatureSize;
    if ( !CDRNCrypto::Verify( issuerPublicKey, token, signedSize, token + signedSize ) )
        return false;

    CDRNWireReader reader( token, signedSize );
    UInt8 version = 0;
    const UInt8* userKey = GUCEF_NULL;
    UInt32 userKeySize = 0;
    const UInt8* hash = GUCEF_NULL;
    UInt32 hashSize = 0;
    if ( !reader.ReadUInt8( version ) || SignedTokenVersion != version ||
         !reader.ReadString( session ) || !reader.ReadString( userId ) || !reader.ReadString( role ) ||
         !reader.ReadVarUInt64( expiryUnixSeconds ) || !reader.ReadSizedBytes( userKey, userKeySize ) ||
         !reader.ReadSizedBytes( hash, hashSize ) || 0 != reader.GetRemaining() ||
         ( 0 != userKeySize && CDRNCrypto::KeySize != userKeySize ) || ( 0 != hashSize && TokenPolicyHashSize != hashSize ) )
        return false;
    userPublicKey.SetDataSize( 0 );
    if ( userKeySize > 0 )
        userPublicKey.CopyFrom( 0, userKeySize, userKey );
    policyHash.SetDataSize( 0 );
    if ( hashSize > 0 )
        policyHash.CopyFrom( 0, hashSize, hash );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::IsVerifyingSignedTokens( void ) const
{GUCEF_TRACE;

    return m_haveIssuerKey;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::ParseToken( const CORE::CString& key ,
                                    const UInt8* token       ,
                                    UInt32 tokenSize         ,
                                    CORE::CString& session   ,
                                    CORE::CString& userId    ,
                                    CORE::CString& role      ,
                                    UInt64& expiryUnixSeconds,
                                    CORE::CDynamicBuffer* policyHash )
{GUCEF_TRACE;

    if ( GUCEF_NULL == token || tokenSize <= CORE::CSha256::DigestSize + 1 )
        return false;

    UInt32 signedSize = tokenSize - CORE::CSha256::DigestSize;
    UInt8 mac[ CORE::CSha256::DigestSize ];
    ComputeTokenMac( key, token, signedSize, mac );
    if ( !CORE::CSha256::ConstantTimeEquals( mac, token + signedSize, CORE::CSha256::DigestSize ) )
        return false;

    CDRNWireReader reader( token, signedSize );
    UInt8 version = 0;
    if ( !reader.ReadUInt8( version ) || ( TokenVersion != version && PolicyTokenVersion != version ) )
        return false;
    if ( !reader.ReadString( session ) || !reader.ReadString( userId ) || !reader.ReadString( role ) || !reader.ReadVarUInt64( expiryUnixSeconds ) )
        return false;
    const UInt8* hash = GUCEF_NULL;
    UInt32 hashSize = 0;
    if ( PolicyTokenVersion == version && ( !reader.ReadSizedBytes( hash, hashSize ) || ( 0 != hashSize && TokenPolicyHashSize != hashSize ) ) )
        return false;
    if ( 0 != reader.GetRemaining() )
        return false;
    if ( GUCEF_NULL != policyHash )
    {
        policyHash->SetDataSize( 0 );
        if ( hashSize > 0 )
            policyHash->CopyFrom( 0, hashSize, hash );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::CreateProof( const CDRNAuthContext& /* context */, CORE::CDynamicBuffer& proof )
{GUCEF_TRACE;

    proof.SetDataSize( 0 );
    if ( m_token.IsNULLOrEmpty() )
        return false;

    UInt32 maxSize = m_token.ByteSize();
    proof.SetBufferSize( maxSize, false );
    UInt32 used = 0;
    if ( !CORE::Base64Decode( m_token, proof.GetBufferPtr(), maxSize, used ) )
        return false;
    proof.SetDataSize( used );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::VerifyProof( const CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, CDRNAuthResult& result )
{GUCEF_TRACE;

    CORE::CString session;
    CORE::CString userId;
    CORE::CString role;
    UInt64 expiry = 0;
    CORE::CDynamicBuffer userPublicKey;
    CORE::CDynamicBuffer policyHash;
    if ( m_haveIssuerKey )
    {
        // signed tokens only: checked with the issuer's public key
        if ( !CDRNCrypto::IsSignatureAvailable() )
        {
            result.refusalText = "no signature codec to check signed tokens";
            return false;
        }
        if ( GUCEF_NULL == proof || 0 == proofSize || SignedTokenVersion != proof[ 0 ] )
        {
            result.refusalText = "a signed token is required";
            return false;
        }
        if ( !ParseSignedToken( m_issuerPublicKey, proof, proofSize, session, userId, role, expiry, userPublicKey, policyHash ) )
        {
            result.refusalText = "invalid token";
            return false;
        }
    }
    else
    if ( m_key.IsNULLOrEmpty() || !ParseToken( m_key, proof, proofSize, session, userId, role, expiry, &policyHash ) )
    {
        result.refusalText = "invalid token";
        return false;
    }
    if ( expiry <= GetUnixTimeNow() )
    {
        result.refusalText = "token expired";
        return false;
    }
    if ( 0 == policyHash.GetDataSize() )
    {
        if ( context.requirePolicyHash )
        {
            result.refusalText = "the token has no security policy hash";
            return false;
        }
    }
    else
    if ( !CDRNCrypto::ConstantTimeEquals( policyHash.GetConstBufferPtr(), context.policyHash, TokenPolicyHashSize ) )
    {
        result.refusalText = "the token is for another security policy";
        return false;
    }
    if ( !m_expectedSession.IsNULLOrEmpty() && session != m_expectedSession )
    {
        result.refusalText = "token is for another session";
        return false;
    }
    result.userId = userId;
    result.role = role;
    result.sessionName = session;
    result.publicKey = userPublicKey;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    if ( !m_key.IsNULLOrEmpty() )
        totalSuccess = cfg.SetAttribute( "key", m_key ) && totalSuccess;
    if ( m_haveIssuerKey )
        totalSuccess = cfg.SetAttribute( "issuerPublicKey", CDRNCrypto::ToHex( m_issuerPublicKey, sizeof( m_issuerPublicKey ) ) ) && totalSuccess;
    if ( !m_token.IsNULLOrEmpty() )
        totalSuccess = cfg.SetAttribute( "token", m_token ) && totalSuccess;
    if ( !m_expectedSession.IsNULLOrEmpty() )
        totalSuccess = cfg.SetAttribute( "expectedSession", m_expectedSession ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNTokenAuthenticator::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_key = cfg.GetAttributeValueOrChildValueByName( "key" ).AsString( m_key, true );
    m_token = cfg.GetAttributeValueOrChildValueByName( "token" ).AsString( m_token, true );
    m_expectedSession = cfg.GetAttributeValueOrChildValueByName( "expectedSession" ).AsString( m_expectedSession, true );
    CORE::CString issuerKey = cfg.GetAttributeValueOrChildValueByName( "issuerPublicKey" ).AsString( CORE::CString::Empty, true );
    if ( !issuerKey.IsNULLOrEmpty() )
    {
        m_haveIssuerKey = CDRNCrypto::FromHex( issuerKey, m_issuerPublicKey, sizeof( m_issuerPublicKey ) );
        if ( !m_haveIssuerKey )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "DRNTokenAuthenticator: \"issuerPublicKey\" is not a 32 byte key in hex" );
            return false;
        }
    }
    return m_haveIssuerKey || !m_key.IsNULLOrEmpty() || !m_token.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNTokenAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNTokenAuthenticator";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
