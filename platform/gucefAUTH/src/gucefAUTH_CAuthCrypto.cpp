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

#include <string.h>

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CORECODECTYPES_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_CORECODECTYPES_H
#endif /* GUCEF_CORE_CORECODECTYPES_H ? */

#ifndef GUCEF_CORE_CICODECINSTANCE_H
#include "gucefCORE_CICodecInstance.h"
#define GUCEF_CORE_CICODECINSTANCE_H
#endif /* GUCEF_CORE_CICODECINSTANCE_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_AUTH_STRINGUTILS_H
#include "gucefAUTH_StringUtils.h"
#define GUCEF_AUTH_STRINGUTILS_H
#endif /* GUCEF_AUTH_STRINGUTILS_H ? */

#include "gucefAUTH_CAuthCrypto.h"

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

const UInt32 CAuthCrypto::KeySize;
const UInt32 CAuthCrypto::NonceSize;
const UInt32 CAuthCrypto::TagSize;
const UInt32 CAuthCrypto::DigestSize;
const UInt32 CAuthCrypto::SignatureSize;
const CORE::CString CAuthCrypto::AeadCipher = "xchacha20-poly1305";
const CORE::CString CAuthCrypto::SignatureType = "ed25519";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static Int32
HexValue( char c )
{GUCEF_TRACE;

    if ( c >= '0' && c <= '9' )
        return c - '0';
    if ( c >= 'a' && c <= 'f' )
        return c - 'a' + 10;
    if ( c >= 'A' && c <= 'F' )
        return c - 'A' + 10;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::RandomBytes( void* buffer, UInt32 size )
{GUCEF_TRACE;

    if ( !CORE::CRandom::GetRandomBytes( buffer, size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "AuthCrypto: The O/S gave no random bytes" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAuthCrypto::Sha256( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    CORE::CSha256::Hash( data, size, digest );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthCrypto::Sha256Hex( const void* data, UInt32 size )
{GUCEF_TRACE;

    UInt8 digest[ DigestSize ];
    CORE::CSha256::Hash( data, size, digest );
    return ToHex( digest, DigestSize );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAuthCrypto::ToHex( const void* data, UInt32 size )
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789abcdef";
    CORE::CString result;
    const UInt8* bytes = static_cast< const UInt8* >( data );
    if ( GUCEF_NULL == bytes || 0 == size )
        return result;
    // in blocks, a string appends faster than it grows by single characters
    char block[ 129 ];
    UInt32 done = 0;
    while ( done < size )
    {
        UInt32 count = size - done > 64 ? 64 : size - done;
        for ( UInt32 i=0; i<count; ++i )
        {
            block[ i*2 ] = hexDigits[ bytes[ done + i ] >> 4 ];
            block[ i*2 + 1 ] = hexDigits[ bytes[ done + i ] & 0x0F ];
        }
        block[ count * 2 ] = 0;
        result.Append( block, count * 2 );
        done += count;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::FromHex( const CORE::CString& hex, CORE::CDynamicBuffer& output )
{GUCEF_TRACE;

    output.SetDataSize( 0 );
    UInt32 length = GetByteLength( hex );
    if ( 0 != ( length % 2 ) )
        return false;
    const char* text = hex.C_String();
    if ( 0 == length )
        return true;
    output.SetBufferSize( length / 2 );
    UInt8* bytes = static_cast< UInt8* >( output.GetBufferPtr() );
    for ( UInt32 i=0; i<length/2; ++i )
    {
        Int32 high = HexValue( text[ i*2 ] );
        Int32 low = HexValue( text[ i*2 + 1 ] );
        if ( high < 0 || low < 0 )
        {
            output.SetDataSize( 0 );
            return false;
        }
        bytes[ i ] = (UInt8) ( ( high << 4 ) | low );
    }
    output.SetDataSize( length / 2 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::FromHexFixed( const CORE::CString& hex, UInt8* output, UInt32 expectedSize )
{GUCEF_TRACE;

    if ( GUCEF_NULL == output || GetByteLength( hex ) != expectedSize * 2 )
        return false;
    const char* text = hex.C_String();
    for ( UInt32 i=0; i<expectedSize; ++i )
    {
        Int32 high = HexValue( text[ i*2 ] );
        Int32 low = HexValue( text[ i*2 + 1 ] );
        if ( high < 0 || low < 0 )
            return false;
        output[ i ] = (UInt8) ( ( high << 4 ) | low );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::ConstantTimeEquals( const void* a, const void* b, UInt32 size )
{GUCEF_TRACE;

    return CORE::CSha256::ConstantTimeEquals( a, b, size );
}

/*-------------------------------------------------------------------------*/

void
CAuthCrypto::Wipe( void* memory, UInt32 size )
{GUCEF_TRACE;

    volatile UInt8* bytes = static_cast< volatile UInt8* >( memory );
    for ( UInt32 i=0; i<size; ++i )
        bytes[ i ] = 0;
}

/*-------------------------------------------------------------------------*/

static CORE::CCodecRegistry::TICodecPtr
GetCodec( const CORE::CString& family, const CORE::CString& type )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( family, type );
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::IsAeadAvailable( void )
{GUCEF_TRACE;

    return !GetCodec( CORE::CoreCodecTypes::EncryptionCodec, AeadCipher ).IsNULL();
}

/*-------------------------------------------------------------------------*/

static bool
RunAead( bool encode                        ,
         const UInt8* key                   ,
         const UInt8* nonce                 ,
         const void* aad                    ,
         UInt32 aadSize                     ,
         const void* input                  ,
         UInt32 inputSize                   ,
         CORE::CDynamicBuffer& output       )
{GUCEF_TRACE;

    if ( GUCEF_NULL == key || GUCEF_NULL == nonce )
        return false;
    CORE::CCodecRegistry::TICodecPtr codec = GetCodec( CORE::CoreCodecTypes::EncryptionCodec, CAuthCrypto::AeadCipher );
    if ( codec.IsNULL() )
        return false;
    CORE::CValueList params;
    params.Set( "key", CORE::CVariant( key, CAuthCrypto::KeySize ) );
    CORE::CICodecInstancePtr instance;
    if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() )
        return false;

    CORE::SCodecCallArgs args;
    memset( &args, 0, sizeof( args ) );
    args.structSize = sizeof( args );
    args.nonce = nonce;
    args.nonceSize = CAuthCrypto::NonceSize;
    args.aad = aad;
    args.aadSize = aadSize;

    // The codec appends: on failure the output is put back as it was
    UInt32 sizeBefore = output.GetDataSize();
    bool ok = instance->Process( encode ? GUCEF_CODEC_PROCESS_ENCODE_MESSAGE : GUCEF_CODEC_PROCESS_DECODE_MESSAGE, input, inputSize, &args, output );
    if ( !ok )
        output.SetDataSize( sizeBefore );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::AeadSeal( const UInt8* key                 ,
                       const UInt8* nonce               ,
                       const void* aad                  ,
                       UInt32 aadSize                   ,
                       const void* plaintext            ,
                       UInt32 plaintextSize             ,
                       CORE::CDynamicBuffer& output     )
{GUCEF_TRACE;

    return RunAead( true, key, nonce, aad, aadSize, plaintext, plaintextSize, output );
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::AeadOpen( const UInt8* key                 ,
                       const UInt8* nonce               ,
                       const void* aad                  ,
                       UInt32 aadSize                   ,
                       const void* sealed               ,
                       UInt32 sealedSize                ,
                       CORE::CDynamicBuffer& output     )
{GUCEF_TRACE;

    if ( sealedSize < TagSize )
        return false;
    return RunAead( false, key, nonce, aad, aadSize, sealed, sealedSize, output );
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::IsSignatureAvailable( void )
{GUCEF_TRACE;

    return !GetCodec( CORE::CoreCodecTypes::SignatureCodec, SignatureType ).IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::VerifySignature( const CORE::CString& publicKeyHex ,
                              const void* message               ,
                              UInt32 messageSize                ,
                              const CORE::CString& signatureHex )
{GUCEF_TRACE;

    UInt8 publicKey[ KeySize ];
    UInt8 signature[ SignatureSize ];
    if ( !FromHexFixed( publicKeyHex, publicKey, KeySize ) || !FromHexFixed( signatureHex, signature, SignatureSize ) )
        return false;
    CORE::CCodecRegistry::TICodecPtr codec = GetCodec( CORE::CoreCodecTypes::SignatureCodec, SignatureType );
    if ( codec.IsNULL() )
        return false;

    CORE::CValueList params;
    params.Set( "publicKey", CORE::CVariant( publicKey, KeySize ) );
    params.Set( "signature", CORE::CVariant( signature, SignatureSize ) );
    CORE::CICodecInstancePtr instance;
    if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() )
        return false;
    CORE::CDynamicBuffer output;
    bool ok = instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, message, messageSize, GUCEF_NULL, output );
    return ok && CORE::GUCEF_CODEC_STATUS_OK == instance->GetLastStatus();
}

/*-------------------------------------------------------------------------*/

static const char*
RsaCodecNameForAlgorithm( const CORE::CString& jwsAlgorithm )
{GUCEF_TRACE;

    if ( jwsAlgorithm == "RS256" )
        return "rsa-pkcs1-sha256";
    if ( jwsAlgorithm == "RS384" )
        return "rsa-pkcs1-sha384";
    if ( jwsAlgorithm == "RS512" )
        return "rsa-pkcs1-sha512";
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::IsRsaAvailable( void )
{GUCEF_TRACE;

    return !GetCodec( CORE::CoreCodecTypes::SignatureCodec, "rsa-pkcs1-sha256" ).IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CAuthCrypto::VerifyRsaPkcs1( const CORE::CString& jwsAlgorithm ,
                             const void* modulus               ,
                             UInt32 modulusSize                ,
                             const void* exponent              ,
                             UInt32 exponentSize               ,
                             const void* message               ,
                             UInt32 messageSize                ,
                             const void* signature             ,
                             UInt32 signatureSize              )
{GUCEF_TRACE;

    const char* codecName = RsaCodecNameForAlgorithm( jwsAlgorithm );
    if ( GUCEF_NULL == codecName || GUCEF_NULL == modulus || 0 == modulusSize || GUCEF_NULL == exponent || 0 == exponentSize ||
         GUCEF_NULL == message || GUCEF_NULL == signature || 0 == signatureSize )
        return false;

    CORE::CCodecRegistry::TICodecPtr codec = GetCodec( CORE::CoreCodecTypes::SignatureCodec, codecName );
    if ( codec.IsNULL() )
        return false;

    CORE::CValueList params;
    params.Set( "modulus", CORE::CVariant( modulus, modulusSize ) );
    params.Set( "exponent", CORE::CVariant( exponent, exponentSize ) );
    CORE::CICodecInstancePtr instance;
    if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() )
        return false;

    CORE::SCodecCallArgs args;
    memset( &args, 0, sizeof( args ) );
    args.structSize = sizeof( args );
    args.aad = signature;
    args.aadSize = signatureSize;

    CORE::CDynamicBuffer output;
    bool ok = instance->Process( GUCEF_CODEC_PROCESS_DECODE_MESSAGE, message, messageSize, &args, output );
    return ok && CORE::GUCEF_CODEC_STATUS_OK == instance->GetLastStatus();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
