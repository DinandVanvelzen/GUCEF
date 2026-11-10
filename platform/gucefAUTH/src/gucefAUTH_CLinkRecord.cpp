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

#include "gucefAUTH_CLinkRecord.h"

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

const UInt32 CLinkRecord::CurrentVersion;

static const UInt32 MaxIdLength = 128;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Lower case hex digits only, exactly the given number of them (or none when allowEmpty) */
static bool
IsLowerHex( const CORE::CString& text, UInt32 digits, bool allowEmpty )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return allowEmpty;
    if ( GetByteLength( text ) != digits )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<digits; ++i )
        if ( !( ( chars[ i ] >= '0' && chars[ i ] <= '9' ) || ( chars[ i ] >= 'a' && chars[ i ] <= 'f' ) ) )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Visible ASCII without spaces: nothing that can break the line oriented signed text */
static bool
IsPlainId( const CORE::CString& text, bool allowEmpty )
{GUCEF_TRACE;

    if ( text.IsNULLOrEmpty() )
        return allowEmpty;
    if ( GetByteLength( text ) > MaxIdLength )
        return false;
    const char* chars = text.C_String();
    for ( UInt32 i=0; i<GetByteLength( text ); ++i )
        if ( chars[ i ] < 0x21 || chars[ i ] > 0x7E )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

CLinkRecord::CLinkRecord( void )
    : CORE::CIDataNodeSerializable()
    , version( CurrentVersion )
    , accountRef()
    , profileKeyId()
    , linkedAtMs( 0 )
    , proofDigest()
    , publicKey()
    , signature()
    , mergedFrom()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkRecord::CLinkRecord( const CLinkRecord& src )
    : CORE::CIDataNodeSerializable( src )
    , version( src.version )
    , accountRef( src.accountRef )
    , profileKeyId( src.profileKeyId )
    , linkedAtMs( src.linkedAtMs )
    , proofDigest( src.proofDigest )
    , publicKey( src.publicKey )
    , signature( src.signature )
    , mergedFrom( src.mergedFrom )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkRecord::~CLinkRecord()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLinkRecord&
CLinkRecord::operator=( const CLinkRecord& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        version = src.version;
        accountRef = src.accountRef;
        profileKeyId = src.profileKeyId;
        linkedAtMs = src.linkedAtMs;
        proofDigest = src.proofDigest;
        publicKey = src.publicKey;
        signature = src.signature;
        mergedFrom = src.mergedFrom;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::Create( ILinkSigner& signer                ,
                     const CAccountRef& accountRef      ,
                     const CORE::CString& proofDigest   ,
                     UInt64 linkedAtMs                  ,
                     CLinkRecord& record                ,
                     const CORE::CString& mergedFrom    )
{GUCEF_TRACE;

    CLinkRecord created;
    created.accountRef = accountRef;
    created.proofDigest = proofDigest;
    created.linkedAtMs = linkedAtMs;
    created.mergedFrom = mergedFrom;
    if ( !created.Sign( signer ) )
        return false;
    record = created;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::IsKnownVersion( void ) const
{GUCEF_TRACE;

    return version >= 1 && version <= CurrentVersion;
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::IsWellFormed( void ) const
{GUCEF_TRACE;

    return accountRef.IsValid()                                         &&
           IsPlainId( profileKeyId, false )                             &&
           IsLowerHex( proofDigest, CAuthCrypto::DigestSize * 2, true ) &&
           IsLowerHex( publicKey, CAuthCrypto::KeySize * 2, false )     &&
           IsPlainId( mergedFrom, true );
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::GetSignedBytes( CORE::CDynamicBuffer& signedBytes ) const
{GUCEF_TRACE;

    if ( !IsWellFormed() )
        return false;
    CORE::CString text = "gucefAUTH-link-v1\n";
    text += accountRef.ToString();
    text += '\n';
    text += profileKeyId;
    text += '\n';
    text += CORE::UInt64ToString( linkedAtMs );
    text += '\n';
    text += proofDigest;
    text += '\n';
    text += publicKey;
    text += '\n';
    text += mergedFrom;
    signedBytes.SetDataSize( 0 );
    signedBytes.Append( text.C_String(), GetByteLength( text ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::Sign( ILinkSigner& signer )
{GUCEF_TRACE;

    CLinkRecord candidate( *this );
    candidate.version = CurrentVersion;
    candidate.profileKeyId = signer.GetKeyId();
    candidate.publicKey = signer.GetPublicKeyHex();
    candidate.signature.Clear();
    CORE::CDynamicBuffer signedBytes;
    if ( !candidate.GetSignedBytes( signedBytes ) )
        return false;

    CORE::CDynamicBuffer signatureBytes;
    if ( !signer.Sign( signedBytes.GetConstBufferPtr(), signedBytes.GetDataSize(), signatureBytes ) ||
         CAuthCrypto::SignatureSize != signatureBytes.GetDataSize() )
        return false;
    candidate.signature = CAuthCrypto::ToHex( signatureBytes.GetConstBufferPtr(), signatureBytes.GetDataSize() );
    *this = candidate;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::VerifySignature( void ) const
{GUCEF_TRACE;

    if ( !IsKnownVersion() )
        return false;
    CORE::CDynamicBuffer signedBytes;
    if ( !GetSignedBytes( signedBytes ) )
        return false;
    return CAuthCrypto::VerifySignature( publicKey, signedBytes.GetConstBufferPtr(), signedBytes.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::Serialize( CORE::CDataNode& domRootNode                        ,
                        const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "version", version );
    domRootNode.SetAttribute( "accountRef", accountRef.ToString() );
    domRootNode.SetAttribute( "profileKeyId", profileKeyId );
    domRootNode.SetAttribute( "linkedAtMs", CORE::CString( CORE::UInt64ToString( linkedAtMs ) ) );
    domRootNode.SetAttribute( "proofDigest", proofDigest );
    domRootNode.SetAttribute( "publicKey", publicKey );
    domRootNode.SetAttribute( "signature", signature );
    domRootNode.SetAttribute( "mergedFrom", mergedFrom );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLinkRecord::Deserialize( const CORE::CDataNode& domRootNode                  ,
                          const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    version = domRootNode.GetAttributeValueOrChildValueByName( "version" ).AsUInt32( 0, true );
    if ( 0 == version )
        return false;
    accountRef = CAccountRef();
    CAccountRef::TryParse( domRootNode.GetAttributeValueOrChildValueByName( "accountRef" ).AsString( CORE::CString::Empty, true ), accountRef );
    profileKeyId = domRootNode.GetAttributeValueOrChildValueByName( "profileKeyId" ).AsString( CORE::CString::Empty, true );
    linkedAtMs = domRootNode.GetAttributeValueOrChildValueByName( "linkedAtMs" ).AsUInt64( 0, true );
    proofDigest = domRootNode.GetAttributeValueOrChildValueByName( "proofDigest" ).AsString( CORE::CString::Empty, true );
    publicKey = domRootNode.GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
    signature = domRootNode.GetAttributeValueOrChildValueByName( "signature" ).AsString( CORE::CString::Empty, true );
    mergedFrom = domRootNode.GetAttributeValueOrChildValueByName( "mergedFrom" ).AsString( CORE::CString::Empty, true );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CLinkRecord::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CLinkRecord( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLinkRecord::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTH::CLinkRecord";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
