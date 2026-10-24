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

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

#ifndef GUCEF_MULTIPLAY_DATANODEUTILS_H
#include "gucefMULTIPLAY_DataNodeUtils.h"
#define GUCEF_MULTIPLAY_DATANODEUTILS_H
#endif /* GUCEF_MULTIPLAY_DATANODEUTILS_H ? */

#include "gucefMULTIPLAY_CPlayerIdentity.h"

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

CPlayerIdentity::CPlayerIdentity( void )
    : CORE::CIDataNodeSerializable()
    , userId()
    , displayName()
    , accounts()
    , publicKey()
    , privateKey()
    , previousUserId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerIdentity::CPlayerIdentity( const CPlayerIdentity& src )
    : CORE::CIDataNodeSerializable( src )
    , userId( src.userId )
    , displayName( src.displayName )
    , accounts( src.accounts )
    , publicKey( src.publicKey )
    , privateKey( src.privateKey )
    , previousUserId( src.previousUserId )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerIdentity::~CPlayerIdentity()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPlayerIdentity&
CPlayerIdentity::operator=( const CPlayerIdentity& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        userId = src.userId;
        displayName = src.displayName;
        accounts = src.accounts;
        publicKey = src.publicKey;
        privateKey = src.privateKey;
        previousUserId = src.previousUserId;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::Serialize( CORE::CDataNode& domRootNode                        ,
                            const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    domRootNode.SetAttribute( "userId", userId );
    domRootNode.SetAttribute( "displayName", displayName );
    if ( !publicKey.IsNULLOrEmpty() )
        domRootNode.SetAttribute( "publicKey", publicKey );
    if ( !previousUserId.IsNULLOrEmpty() )
        domRootNode.SetAttribute( "previousUserId", previousUserId );
    WriteValueListAsChild( domRootNode, "accounts", accounts );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::Deserialize( const CORE::CDataNode& domRootNode                  ,
                              const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    userId = domRootNode.GetAttributeValueOrChildValueByName( "userId" ).AsString( CORE::CString::Empty, true );
    displayName = domRootNode.GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
    publicKey = domRootNode.GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
    previousUserId = domRootNode.GetAttributeValueOrChildValueByName( "previousUserId" ).AsString( CORE::CString::Empty, true );
    privateKey.Clear();
    ReadValueListFromChild( domRootNode, "accounts", accounts );
    if ( userId.IsNULLOrEmpty() && !publicKey.IsNULLOrEmpty() )
        userId = MakeUserIdFromHex( publicKey );
    return !userId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPlayerIdentity::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPlayerIdentity( *this );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPlayerIdentity::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CPlayerIdentity";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerIdentity::MakeGuid( void )
{GUCEF_TRACE;

    UInt8 bytes[ 16 ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, sizeof( bytes ) ) )
    {
        for ( int i=0; i<16; ++i )
            bytes[ i ] = (UInt8) CORE::CRandom::GetRandomUInt32( 0, 255 );
    }

    // RFC 4122: version 4 (random), variant 10
    bytes[ 6 ] = (UInt8)( ( bytes[ 6 ] & 0x0F ) | 0x40 );
    bytes[ 8 ] = (UInt8)( ( bytes[ 8 ] & 0x3F ) | 0x80 );
    return GuidText( bytes );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerIdentity::MakeUserId( const UInt8* publicKeyBytes )
{GUCEF_TRACE;

    if ( GUCEF_NULL == publicKeyBytes )
        return CORE::CString::Empty;
    UInt8 digest[ DRN::CDRNCrypto::HashSize ];
    DRN::CDRNCrypto::Sha256( publicKeyBytes, KeySize, digest );
    return GuidText( digest );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerIdentity::MakeUserIdFromHex( const CORE::CString& publicKeyHex )
{GUCEF_TRACE;

    UInt8 key[ KeySize ];
    if ( !DRN::CDRNCrypto::FromHex( publicKeyHex, key, KeySize ) )
        return CORE::CString::Empty;
    return MakeUserId( key );
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::IsKeyPairAvailable( void )
{GUCEF_TRACE;

    return DRN::CDRNCrypto::IsSignatureAvailable();
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::GenerateKeyPair( void )
{GUCEF_TRACE;

    UInt8 seed[ KeySize ];
    UInt8 key[ KeySize ];
    if ( !DRN::CDRNCrypto::GenerateSignatureKeyPair( seed, key ) )
        return false;
    privateKey = DRN::CDRNCrypto::ToHex( seed, KeySize );
    publicKey = DRN::CDRNCrypto::ToHex( key, KeySize );
    userId = MakeUserId( key );
    memset( seed, 0, sizeof( seed ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::SetPrivateKey( const CORE::CString& privateKeyHex )
{GUCEF_TRACE;

    UInt8 seed[ KeySize ];
    UInt8 key[ KeySize ];
    if ( !DRN::CDRNCrypto::FromHex( privateKeyHex, seed, KeySize ) || !DRN::CDRNCrypto::GetSignaturePublicKey( seed, key ) )
    {
        memset( seed, 0, sizeof( seed ) );
        return false;
    }
    privateKey = DRN::CDRNCrypto::ToHex( seed, KeySize );
    publicKey = DRN::CDRNCrypto::ToHex( key, KeySize );
    userId = MakeUserId( key );
    memset( seed, 0, sizeof( seed ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::HasKeyPair( void ) const
{GUCEF_TRACE;

    return !privateKey.IsNULLOrEmpty() && !publicKey.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::IsUserIdOfKey( void ) const
{GUCEF_TRACE;

    return !publicKey.IsNULLOrEmpty() && userId == MakeUserIdFromHex( publicKey );
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::Sign( const void* data, UInt32 dataSize, UInt8* signature ) const
{GUCEF_TRACE;

    UInt8 seed[ KeySize ];
    if ( GUCEF_NULL == signature || !DRN::CDRNCrypto::FromHex( privateKey, seed, KeySize ) )
        return false;
    bool signedIt = DRN::CDRNCrypto::Sign( seed, data, dataSize, signature );
    memset( seed, 0, sizeof( seed ) );
    return signedIt;
}

/*-------------------------------------------------------------------------*/

bool
CPlayerIdentity::GetPublicKey( UInt8* publicKeyBytes ) const
{GUCEF_TRACE;

    return GUCEF_NULL != publicKeyBytes && DRN::CDRNCrypto::FromHex( publicKey, publicKeyBytes, KeySize );
}

/*-------------------------------------------------------------------------*/

CPlayerIdentity
CPlayerIdentity::GetPublicIdentity( void ) const
{GUCEF_TRACE;

    CPlayerIdentity copy( *this );
    copy.privateKey.Clear();
    return copy;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CPlayerIdentity::GuidText( const UInt8* bytes )
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789abcdef";
    char text[ 37 ];
    UInt32 pos = 0;
    for ( int i=0; i<16; ++i )
    {
        if ( 4 == i || 6 == i || 8 == i || 10 == i )
            text[ pos++ ] = '-';
        text[ pos++ ] = hexDigits[ bytes[ i ] >> 4 ];
        text[ pos++ ] = hexDigits[ bytes[ i ] & 0x0F ];
    }
    text[ pos ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
