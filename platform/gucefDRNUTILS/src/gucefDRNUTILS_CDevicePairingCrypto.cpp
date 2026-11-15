/*
 *  gucefDRNUTILS: GUCEF module providing remote view and control of job schedulers and the task manager
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

#include "gucefDRNUTILS_CDevicePairingCrypto.h"

#include <string.h>

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_DRN_CDRNCRYPTO_H
#include "gucefDRN_CDRNCrypto.h"
#define GUCEF_DRN_CDRNCRYPTO_H
#endif /* GUCEF_DRN_CDRNCRYPTO_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRNUTILS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char CrockfordAlphabet[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
const char KeySaltPrefix[] = "gucef.drnutils.pairing.v1:";
const char AuthInfo[] = "auth";
const char ServerProofInfo[] = "server-proof";
const char ServerProofLabel[] = "gucef.drnutils.pairing.server.v1";
const char DeviceProofLabel[] = "gucef.drnutils.pairing.device.v1";

/** The value of a Crockford base32 character as typed; -1 for a character that is not one */
Int32
CrockfordValue( char c )
{GUCEF_TRACE;

    if ( c >= 'a' && c <= 'z' )
        c = (char) ( c - 'a' + 'A' );
    if ( 'O' == c )
        c = '0';
    else if ( 'I' == c || 'L' == c )
        c = '1';
    for ( Int32 i=0; i<32; ++i )
    {
        if ( CrockfordAlphabet[ i ] == c )
            return i;
    }
    return -1;
}

/*-------------------------------------------------------------------------*/

CORE::CString
GroupsOfFour( const CORE::CString& text )
{GUCEF_TRACE;

    CORE::CString result;
    const char* c = text.C_String();
    UInt32 length = text.Length();
    for ( UInt32 i=0; i<length; ++i )
    {
        if ( i > 0 && 0 == ( i % 4 ) )
            result += '-';
        result += c[ i ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
RandomCharacters( UInt32 bytes, UInt32 characters, CORE::CString& result )
{GUCEF_TRACE;

    UInt8 buffer[ 32 ];
    if ( bytes > sizeof( buffer ) || !CDevicePairingCrypto::RandomBytes( buffer, bytes ) )
        return false;
    result = CDevicePairingCrypto::ToCrockfordBase32( buffer, bytes, characters );
    memset( buffer, 0, sizeof( buffer ) );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
TextBytes( const CORE::CString& text )
{GUCEF_TRACE;

    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::RandomBytes( UInt8* buffer, UInt32 size )
{GUCEF_TRACE;

    return CORE::CRandom::GetRandomBytes( buffer, size );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDevicePairingCrypto::ToCrockfordBase32( const UInt8* data, UInt32 size, UInt32 characters )
{GUCEF_TRACE;

    CORE::CString result;
    UInt32 totalBits = size * 8;
    for ( UInt32 c=0; c<characters; ++c )
    {
        UInt32 value = 0;
        for ( UInt32 b=0; b<5; ++b )
        {
            UInt32 bit = c * 5 + b;
            value <<= 1;
            if ( bit < totalBits && 0 != ( data[ bit / 8 ] & ( 0x80 >> ( bit % 8 ) ) ) )
                value |= 1;
        }
        result += CrockfordAlphabet[ value ];
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::GenerateCode( CORE::CString& code )
{GUCEF_TRACE;

    return RandomCharacters( 10, CodeLength, code );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDevicePairingCrypto::FormatCode( const CORE::CString& normalisedCode )
{GUCEF_TRACE;

    return GroupsOfFour( normalisedCode );
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::NormaliseCode( const CORE::CString& typed, CORE::CString& normalisedCode )
{GUCEF_TRACE;

    CORE::CString result;
    const char* c = typed.C_String();
    UInt32 length = TextBytes( typed );
    for ( UInt32 i=0; i<length; ++i )
    {
        char ch = c[ i ];
        if ( '-' == ch || ' ' == ch || '\t' == ch )
            continue;
        Int32 value = CrockfordValue( ch );
        if ( value < 0 )
            return false;
        result += CrockfordAlphabet[ value ];
    }
    if ( CodeLength != result.Length() )
        return false;
    normalisedCode = result;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::DeriveKeys( const CORE::CString& normalisedCode, const CORE::CString& serviceName,
                                  CORE::CString& authSecretHex, UInt8* serverProofKey )
{GUCEF_TRACE;

    if ( CodeLength != normalisedCode.Length() || GUCEF_NULL == serverProofKey )
        return false;
    CORE::CString salt = CORE::CString( KeySaltPrefix ) + serviceName;
    UInt8 authKey[ KeySize ];
    if ( !CORE::CHkdfSha256::Derive( salt.C_String(), TextBytes( salt ), normalisedCode.C_String(), TextBytes( normalisedCode ),
                                     AuthInfo, (UInt32) sizeof( AuthInfo ) - 1, authKey, KeySize ) )
        return false;
    if ( !CORE::CHkdfSha256::Derive( salt.C_String(), TextBytes( salt ), normalisedCode.C_String(), TextBytes( normalisedCode ),
                                     ServerProofInfo, (UInt32) sizeof( ServerProofInfo ) - 1, serverProofKey, KeySize ) )
    {
        memset( authKey, 0, sizeof( authKey ) );
        return false;
    }
    authSecretHex = DRN::CDRNCrypto::ToHex( authKey, KeySize ).Lowercase();
    memset( authKey, 0, sizeof( authKey ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::ComputeServerProof( const UInt8* serverProofKey, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, UInt8* proof )
{GUCEF_TRACE;

    if ( GUCEF_NULL == serverProofKey || GUCEF_NULL == staticKey || GUCEF_NULL == proof || ( GUCEF_NULL == nonce && nonceSize > 0 ) )
        return false;
    CORE::CHmacSha256 mac( serverProofKey, KeySize );
    mac.Update( ServerProofLabel, (UInt32) sizeof( ServerProofLabel ) - 1 );
    mac.Update( staticKey, KeySize );
    if ( nonceSize > 0 )
        mac.Update( nonce, nonceSize );
    mac.Final( proof );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::VerifyServerProof( const UInt8* serverProofKey, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, const UInt8* proof, UInt32 proofSize )
{GUCEF_TRACE;

    if ( CORE::CHmacSha256::MacSize != proofSize || GUCEF_NULL == proof )
        return false;
    UInt8 expected[ CORE::CHmacSha256::MacSize ];
    if ( !ComputeServerProof( serverProofKey, staticKey, nonce, nonceSize, expected ) )
        return false;
    return Equals( expected, proof, CORE::CHmacSha256::MacSize );
}

/*-------------------------------------------------------------------------*/

void
CDevicePairingCrypto::GetDeviceProofMessage( const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, CORE::CDynamicBuffer& message )
{GUCEF_TRACE;

    message.Clear();
    message.Append( DeviceProofLabel, (UInt32) sizeof( DeviceProofLabel ) - 1 );
    if ( GUCEF_NULL != staticKey )
        message.Append( staticKey, KeySize );
    if ( GUCEF_NULL != nonce && nonceSize > 0 )
        message.Append( nonce, nonceSize );
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::SignDeviceProof( const UInt8* deviceSeed, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, UInt8* signature )
{GUCEF_TRACE;

    if ( GUCEF_NULL == deviceSeed || GUCEF_NULL == staticKey || GUCEF_NULL == signature )
        return false;
    CORE::CDynamicBuffer message;
    GetDeviceProofMessage( staticKey, nonce, nonceSize, message );
    return DRN::CDRNCrypto::Sign( deviceSeed, message.GetConstBufferPtr(), message.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::VerifyDeviceProof( const UInt8* devicePublicKey, const UInt8* staticKey, const UInt8* nonce, UInt32 nonceSize, const UInt8* signature, UInt32 signatureSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == devicePublicKey || GUCEF_NULL == staticKey || GUCEF_NULL == signature || SignatureSize != signatureSize )
        return false;
    CORE::CDynamicBuffer message;
    GetDeviceProofMessage( staticKey, nonce, nonceSize, message );
    return DRN::CDRNCrypto::Verify( devicePublicKey, message.GetConstBufferPtr(), message.GetDataSize(), signature );
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::GenerateDeviceKey( UInt8* seed, UInt8* publicKey )
{GUCEF_TRACE;

    if ( GUCEF_NULL == seed || GUCEF_NULL == publicKey )
        return false;
    return DRN::CDRNCrypto::GenerateSignatureKeyPair( seed, publicKey );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDevicePairingCrypto::Fingerprint( const UInt8* publicKey )
{GUCEF_TRACE;

    if ( GUCEF_NULL == publicKey )
        return CORE::CString::Empty;
    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( publicKey, KeySize, digest );
    return GroupsOfFour( ToCrockfordBase32( digest, CORE::CSha256::DigestSize, 20 ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDevicePairingCrypto::FingerprintOfHex( const CORE::CString& publicKeyHex )
{GUCEF_TRACE;

    UInt8 key[ KeySize ];
    if ( !DRN::CDRNCrypto::FromHex( publicKeyHex, key, KeySize ) )
        return CORE::CString::Empty;
    return Fingerprint( key );
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::GenerateDeviceId( CORE::CString& deviceId )
{GUCEF_TRACE;

    CORE::CString id;
    if ( !RandomCharacters( 16, 26, id ) )
        return false;
    deviceId = "d-" + id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::GenerateSessionId( CORE::CString& sessionId )
{GUCEF_TRACE;

    return RandomCharacters( 16, 26, sessionId );
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::GenerateWindowId( CORE::CString& windowId )
{GUCEF_TRACE;

    CORE::CString id;
    if ( !RandomCharacters( 10, 16, id ) )
        return false;
    windowId = "w-" + id;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDevicePairingCrypto::Equals( const void* a, const void* b, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == a || GUCEF_NULL == b )
        return false;
    return CORE::CSha256::ConstantTimeEquals( a, b, size );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNUTILS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
