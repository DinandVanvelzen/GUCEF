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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#include "gucefDRN_CDRNSharedSecretAuthenticator.h"

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

const CORE::CString CDRNSharedSecretAuthenticator::MethodName = "shared-secret";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
HmacUpdateString( CORE::CHmacSha256& hmac, const CORE::CString& str )
{GUCEF_TRACE;
    CORE::CUtf8String utf8 = CORE::ToUtf8String( str );
    UInt32 size = utf8.ByteSize() > 0 ? utf8.ByteSize() - 1 : 0;
    UInt8 sizeBytes[ 4 ] = { (UInt8) ( size >> 24 ), (UInt8) ( size >> 16 ), (UInt8) ( size >> 8 ), (UInt8) size };
    hmac.Update( sizeBytes, 4 );
    hmac.Update( utf8.C_String(), size );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDRNSharedSecretAuthenticator::CDRNSharedSecretAuthenticator( void )
    : CIDRNAuthenticator()
    , m_secret()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNSharedSecretAuthenticator::~CDRNSharedSecretAuthenticator()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNSharedSecretAuthenticator::SetSecret( const CORE::CString& secret )
{GUCEF_TRACE;

    m_secret = secret;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNSharedSecretAuthenticator::GetMethodName( void ) const
{GUCEF_TRACE;

    return MethodName;
}

/*-------------------------------------------------------------------------*/

void
CDRNSharedSecretAuthenticator::ComputeProof( const CORE::CString& secret, const CDRNAuthContext& context, UInt8* mac )
{GUCEF_TRACE;

    CORE::CUtf8String utf8Secret = CORE::ToUtf8String( secret );
    UInt32 secretSize = utf8Secret.ByteSize() > 0 ? utf8Secret.ByteSize() - 1 : 0;
    CORE::CHmacSha256 hmac( utf8Secret.C_String(), secretSize );
    hmac.Update( context.nonce, DRN_CHALLENGE_NONCE_SIZE );
    hmac.Update( context.transcriptHash, DRN_HANDSHAKE_HASH_SIZE );
    HmacUpdateString( hmac, context.serviceName );
    HmacUpdateString( hmac, context.initiatorNodeId );
    HmacUpdateString( hmac, context.acceptorNodeId );
    hmac.Final( mac );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSharedSecretAuthenticator::CreateProof( const CDRNAuthContext& context, CORE::CDynamicBuffer& proof )
{GUCEF_TRACE;

    UInt8 mac[ CORE::CSha256::DigestSize ];
    ComputeProof( m_secret, context, mac );
    proof.SetDataSize( 0 );
    proof.Append( mac, CORE::CSha256::DigestSize, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSharedSecretAuthenticator::VerifyProof( const CDRNAuthContext& context, const UInt8* proof, UInt32 proofSize, CDRNAuthResult& result )
{GUCEF_TRACE;

    if ( CORE::CSha256::DigestSize != proofSize || GUCEF_NULL == proof )
    {
        result.refusalText = "malformed proof";
        return false;
    }
    UInt8 mac[ CORE::CSha256::DigestSize ];
    ComputeProof( m_secret, context, mac );
    if ( !CORE::CSha256::ConstantTimeEquals( mac, proof, CORE::CSha256::DigestSize ) )
    {
        result.refusalText = "wrong secret";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNSharedSecretAuthenticator::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    return cfg.SetAttribute( "secret", m_secret );
}

/*-------------------------------------------------------------------------*/

bool
CDRNSharedSecretAuthenticator::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_secret = cfg.GetAttributeValueOrChildValueByName( "secret" ).AsString( m_secret, true );
    return !m_secret.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNSharedSecretAuthenticator::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNSharedSecretAuthenticator";
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
