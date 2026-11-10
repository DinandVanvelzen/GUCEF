/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_CTASK_H
#include "gucefCORE_CTask.h"
#define GUCEF_CORE_CTASK_H
#endif /* GUCEF_CORE_CTASK_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_AUTH_CAUTHCRYPTO_H
#include "gucefAUTH_CAuthCrypto.h"
#define GUCEF_AUTH_CAUTHCRYPTO_H
#endif /* GUCEF_AUTH_CAUTHCRYPTO_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAuthResult.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CAuthProof::KindOidcIdToken = "oidc-id-token";
const CORE::CString CAuthProof::KindOpenId2Assertion = "openid2-assertion";
const CORE::CString CAuthProof::KindOAuth2AccessToken = "oauth2-access-token";
const CORE::CString CAuthProof::KindPlatformTicket = "platform-ticket";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAuthProof::CAuthProof( void )
    : kind()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthProof::CAuthProof( const CORE::CString& proofKind, const CORE::CString& proofData )
    : kind( proofKind )
    , data( proofData )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAuthProof::IsEmpty( void ) const
{GUCEF_TRACE;

    return data.IsEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthProof::GetDigestHex( void ) const
{GUCEF_TRACE;

    if ( data.IsEmpty() )
        return CORE::CString::Empty;
    CORE::CString material = kind;
    material += ':';
    material += data.Reveal();
    CORE::CString digest = CAuthCrypto::Sha256Hex( material.C_String(), GetByteLength( material ) );
    CAuthCrypto::Wipe( material.C_String(), GetByteLength( material ) );
    return digest;
}

/*-------------------------------------------------------------------------*/

void
CAuthProof::Clear( void )
{GUCEF_TRACE;

    kind.Clear();
    data.Clear();
}

/*-------------------------------------------------------------------------*/

CAuthResult::CAuthResult( void )
    : CORE::CIDataNodeSerializable()
    , providerName()
    , accountRef()
    , displayName()
    , avatarUrl()
    , loginHint()
    , accessToken()
    , idToken()
    , refreshToken()
    , expiresAtMs( 0 )
    , scopes()
    , proof()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthResult::CAuthResult( const CAuthResult& src )
    : CORE::CIDataNodeSerializable( src )
    , providerName( src.providerName )
    , accountRef( src.accountRef )
    , displayName( src.displayName )
    , avatarUrl( src.avatarUrl )
    , loginHint( src.loginHint )
    , accessToken( src.accessToken )
    , idToken( src.idToken )
    , refreshToken( src.refreshToken )
    , expiresAtMs( src.expiresAtMs )
    , scopes( src.scopes )
    , proof( src.proof )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthResult::~CAuthResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAuthResult&
CAuthResult::operator=( const CAuthResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        providerName = src.providerName;
        accountRef = src.accountRef;
        displayName = src.displayName;
        avatarUrl = src.avatarUrl;
        loginHint = src.loginHint;
        accessToken = src.accessToken;
        idToken = src.idToken;
        refreshToken = src.refreshToken;
        expiresAtMs = src.expiresAtMs;
        scopes = src.scopes;
        proof = src.proof;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CAuthResult::Serialize( CORE::CDataNode& domRootNode                        ,
                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    // Tokens and proof are secrets: they are only stored through a credential store
    domRootNode.SetAttribute( "providerName", providerName );
    domRootNode.SetAttribute( "accountRef", accountRef.ToString() );
    domRootNode.SetAttribute( "displayName", displayName );
    domRootNode.SetAttribute( "avatarUrl", avatarUrl );
    domRootNode.SetAttribute( "expiresAtMs", CORE::CString( CORE::UInt64ToString( expiresAtMs ) ) );
    domRootNode.SetAttribute( "proofKind", proof.kind );
    domRootNode.SetValuesOfChildByName( "scopes", scopes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthResult::Deserialize( const CORE::CDataNode& domRootNode                  ,
                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    providerName = domRootNode.GetAttributeValueOrChildValueByName( "providerName" ).AsString( CORE::CString::Empty, true );
    CAccountRef::TryParse( domRootNode.GetAttributeValueOrChildValueByName( "accountRef" ).AsString( CORE::CString::Empty, true ), accountRef );
    displayName = domRootNode.GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
    avatarUrl = domRootNode.GetAttributeValueOrChildValueByName( "avatarUrl" ).AsString( CORE::CString::Empty, true );
    expiresAtMs = domRootNode.GetAttributeValueOrChildValueByName( "expiresAtMs" ).AsUInt64( 0, true );
    proof.kind = domRootNode.GetAttributeValueOrChildValueByName( "proofKind" ).AsString( CORE::CString::Empty, true );
    accessToken.Clear();
    idToken.Clear();
    refreshToken.Clear();
    proof.data.Clear();
    domRootNode.GetValuesOfChildByName( "scopes", scopes );
    return accountRef.IsValid();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CAuthResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CAuthResult( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAuthResult::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CAuthResult";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CAuthResult::IsValid( void ) const
{GUCEF_TRACE;

    return accountRef.IsValid();
}

/*-------------------------------------------------------------------------*/

bool
CAuthResult::IsExpired( UInt64 nowEpochMs, UInt32 skewMs ) const
{GUCEF_TRACE;

    return 0 != expiresAtMs && nowEpochMs + skewMs >= expiresAtMs;
}

/*-------------------------------------------------------------------------*/

void
CAuthResult::WipeSecrets( void )
{GUCEF_TRACE;

    accessToken.Clear();
    idToken.Clear();
    refreshToken.Clear();
    proof.data.Clear();
}

/*-------------------------------------------------------------------------*/

const char*
CAuthResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CAuthResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::AUTH::CAuthResult";
}

/*-------------------------------------------------------------------------*/

bool
CAuthResult::TryGetFromFuture( const CORE::CFutureResult& future, CAuthResult& result )
{GUCEF_TRACE;

    if ( future.HasNoFuture() || !future.IsReady() )
        return false;
    CORE::CTaskPtr task = future.GetResult();
    if ( task.IsNULL() || CORE::TTaskStatus::TASKSTATUS_FINISHED != task->GetTaskStatus() )
        return false;

    // the futures of this module that finish well carry a CAuthResult, anything else is refused
    const CAuthResult* data = CORE::CICloneable::CloneableAs< CAuthResult >( task->GetTaskData() );
    if ( GUCEF_NULL == data )
        return false;
    result = *data;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
