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
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

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

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CCODECPARAMETERINFO_H
#include "gucefCORE_CCodecParameterInfo.h"
#define GUCEF_CORE_CCODECPARAMETERINFO_H
#endif /* GUCEF_CORE_CCODECPARAMETERINFO_H ? */

#include "gucefDRN_CDRNCrypto.h"

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

const UInt32 CDRNCrypto::KeySize;
const UInt32 CDRNCrypto::SignatureSize;
const UInt32 CDRNCrypto::HashSize;
const UInt32 CDRNCrypto::CipherTagSize;
const UInt32 CDRNCrypto::CipherMinNonceSize;
const UInt32 CDRNCrypto::CipherMaxNonceSize;
const UInt32 CDRNCrypto::CipherCapabilities = GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_AUTHENTICATED;
const CORE::CString CDRNCrypto::KeyAgreementType = "x25519";
const CORE::CString CDRNCrypto::SignatureType = "ed25519";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

bool
CDRNCrypto::ConstantTimeEquals( const void* a, const void* b, UInt32 size )
{GUCEF_TRACE;

    return CORE::CSha256::ConstantTimeEquals( a, b, size );
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::ConstantTimeEqualsHashed( const void* a, UInt32 aSize, const void* b, UInt32 bSize )
{GUCEF_TRACE;

    UInt8 digestA[ HashSize ];
    UInt8 digestB[ HashSize ];
    CORE::CSha256::Hash( a, aSize, digestA );
    CORE::CSha256::Hash( b, bSize, digestB );
    bool sameSize = aSize == bSize;
    bool sameDigest = CORE::CSha256::ConstantTimeEquals( digestA, digestB, HashSize );
    return sameDigest && sameSize;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::RandomBytes( void* buffer, UInt32 size )
{GUCEF_TRACE;

    if ( !CORE::CRandom::GetRandomBytes( buffer, size ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "DRNCrypto: The O/S gave no random bytes" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CCodecRegistry::TICodecPtr
CDRNCrypto::GetCodec( const CORE::CString& family, const CORE::CString& type )
{GUCEF_TRACE;

    // The algorithm is chosen by the caller (the ciphers of the security policy, in their order); this picks the
    // implementation of it: the provider the registry ranks first (operating system, plugin, built in, or as their
    // configuration sets it). An algorithm without a provider is not available, which the caller handles: no error
    CORE::CCodecRegistry::TICodecPtrVector codecs;
    if ( CORE::CCoreGlobal::Instance()->GetCodecRegistry().GetCodecs( family, type, codecs ) && !codecs.empty() )
        return codecs.front();
    return CORE::CCodecRegistry::TICodecPtr();
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::IsKeyAgreementAvailable( void )
{GUCEF_TRACE;

    return !GetCodec( CORE::CoreCodecTypes::KeyAgreementCodec, KeyAgreementType ).IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::IsSignatureAvailable( void )
{GUCEF_TRACE;

    return !GetCodec( CORE::CoreCodecTypes::SignatureCodec, SignatureType ).IsNULL();
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::IsCipherAvailable( const CORE::CString& cipher )
{GUCEF_TRACE;

    return !cipher.IsNULLOrEmpty() && !GetCipherCodec( cipher ).IsNULL();
}

/*-------------------------------------------------------------------------*/

/**
 *  Whether packet protection can use the cipher codec (see CDRNCrypto::CipherCapabilities), and the nonce size it
 *  would use with it
 */
static bool
IsUsableCipherCodec( const CORE::CICodec& codec, UInt32& nonceSize )
{GUCEF_TRACE;

    if ( ( codec.GetCapabilities() & CDRNCrypto::CipherCapabilities ) != CDRNCrypto::CipherCapabilities )
        return false;
    if ( CDRNCrypto::CipherTagSize != codec.GetOverhead() )
        return false;

    bool keyFits = false;
    nonceSize = 0;
    CORE::CCodecParameterInfoList params;
    codec.GetParameterInfo( params );
    for ( UInt32 i=0; i<params.size(); ++i )
    {
        const CORE::CCodecParameterInfo& param = params[ i ];
        if ( !param.HasSizeRange() )
            continue;
        if ( param.GetName() == "key" )
            keyFits = param.AcceptsSize( CDRNCrypto::KeySize );
        else
        if ( param.GetName() == "nonce" )
        {
            for ( UInt32 size=CDRNCrypto::CipherMinNonceSize; size<=CDRNCrypto::CipherMaxNonceSize && 0 == nonceSize; ++size )
                if ( param.AcceptsSize( size ) )
                    nonceSize = size;
        }
    }
    return keyFits && 0 != nonceSize;
}

/*-------------------------------------------------------------------------*/

CORE::CCodecRegistry::TICodecPtr
CDRNCrypto::GetCipherCodec( const CORE::CString& cipher, UInt32* nonceSize )
{GUCEF_TRACE;

    CORE::CCodecRegistry::TICodecPtrVector codecs;
    if ( cipher.IsNULLOrEmpty() ||
         !CORE::CCoreGlobal::Instance()->GetCodecRegistry().GetCodecs( CORE::CoreCodecTypes::EncryptionCodec, cipher.Lowercase(), codecs ) )
        return CORE::CCodecRegistry::TICodecPtr();

    // highest priority first
    for ( UInt32 i=0; i<codecs.size(); ++i )
    {
        UInt32 size = 0;
        if ( !codecs[ i ].IsNULL() && IsUsableCipherCodec( *codecs[ i ], size ) )
        {
            if ( GUCEF_NULL != nonceSize )
                *nonceSize = size;
            return codecs[ i ];
        }
    }
    return CORE::CCodecRegistry::TICodecPtr();
}

/*-------------------------------------------------------------------------*/

const CORE::CString::StringVector&
CDRNCrypto::GetPreferredCiphers( void )
{GUCEF_TRACE;

    static CORE::CString::StringVector preferred;
    if ( preferred.empty() )
    {
        preferred.push_back( "xchacha20-poly1305" );
        preferred.push_back( "chacha20-poly1305" );
        preferred.push_back( "aes-256-gcm" );
    }
    return preferred;
}

/*-------------------------------------------------------------------------*/

/** A usable cipher with the priority of its best provider and its place in the preferred ciphers */
struct SUsableCipher
{
    CORE::CString name;
    Int32 priority;
    UInt32 preference;
};

static bool
IsMoreWanted( const SUsableCipher& a, const SUsableCipher& b )
{
    if ( a.priority != b.priority )
        return a.priority > b.priority;
    return a.preference < b.preference;
}

/*-------------------------------------------------------------------------*/

CORE::CString::StringVector
CDRNCrypto::GetUsableCiphers( void )
{GUCEF_TRACE;

    // highest priority first, equal ones by name
    CORE::CCodecRegistry::TCodecProviderVector providers;
    CORE::CCoreGlobal::Instance()->GetCodecRegistry().FindCodecs( CORE::CoreCodecTypes::EncryptionCodec, CipherCapabilities, providers );

    const CORE::CString::StringVector& preferred = GetPreferredCiphers();
    GUCEF::vector< SUsableCipher > usable;
    CORE::CString::StringSet listed;
    for ( UInt32 i=0; i<providers.size(); ++i )
    {
        const CORE::CString name = providers[ i ].codecType.Lowercase();
        UInt32 nonceSize = 0;
        if ( listed.find( name ) != listed.end() || providers[ i ].codec.IsNULL() || !IsUsableCipherCodec( *providers[ i ].codec, nonceSize ) )
            continue;
        listed.insert( name );
        SUsableCipher cipher;
        cipher.name = name;
        cipher.priority = providers[ i ].priority;
        cipher.preference = (UInt32) preferred.size();
        for ( UInt32 p=0; p<preferred.size(); ++p )
            if ( preferred[ p ] == name )
                cipher.preference = p;
        usable.push_back( cipher );
    }
    std::stable_sort( usable.begin(), usable.end(), IsMoreWanted );

    CORE::CString::StringVector ciphers;
    for ( UInt32 i=0; i<usable.size(); ++i )
        ciphers.push_back( usable[ i ].name );
    return ciphers;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::RunCodec( const CORE::CString& family          ,
                      const CORE::CString& type            ,
                      bool encode                          ,
                      const CORE::CValueList& params       ,
                      const void* input                    ,
                      UInt32 inputSize                     ,
                      CORE::CDynamicBuffer& output         ,
                      CORE::TCodecStatus* status                 )
{GUCEF_TRACE;

    if ( GUCEF_NULL != status )
        *status = CORE::GUCEF_CODEC_STATUS_UNSUPPORTED;

    CORE::CCodecRegistry::TICodecPtr codec = GetCodec( family, type );
    if ( codec.IsNULL() )
        return false;

    CORE::CICodecInstancePtr instance;
    if ( !codec->CreateInstance( params, instance ) || instance.IsNULL() )
    {
        if ( GUCEF_NULL != status )
            *status = CORE::GUCEF_CODEC_STATUS_FAILED;
        return false;
    }
    output.SetDataSize( 0 );
    bool ok = instance->Process( encode ? GUCEF_CODEC_PROCESS_ENCODE_MESSAGE : GUCEF_CODEC_PROCESS_DECODE_MESSAGE, input, inputSize, GUCEF_NULL, output );
    if ( GUCEF_NULL != status )
        *status = ok ? CORE::GUCEF_CODEC_STATUS_OK : instance->GetLastStatus();
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::GetKeyAgreementPublicKey( const UInt8* privateKey, UInt8* publicKey )
{GUCEF_TRACE;

    CORE::CValueList params;
    CORE::CDynamicBuffer output;
    if ( !RunCodec( CORE::CoreCodecTypes::KeyAgreementCodec, KeyAgreementType, true, params, privateKey, KeySize, output, GUCEF_NULL ) ||
         KeySize != output.GetDataSize() )
        return false;
    memcpy( publicKey, output.GetConstBufferPtr(), KeySize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::GenerateKeyAgreementKeyPair( UInt8* privateKey, UInt8* publicKey )
{GUCEF_TRACE;

    if ( !RandomBytes( privateKey, KeySize ) )
        return false;
    if ( !GetKeyAgreementPublicKey( privateKey, publicKey ) )
    {
        memset( privateKey, 0, KeySize );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::ComputeSharedSecret( const UInt8* privateKey, const UInt8* peerPublicKey, UInt8* sharedSecret )
{GUCEF_TRACE;

    CORE::CValueList params;
    params.Set( "privateKey", CORE::CVariant( privateKey, KeySize ) );
    CORE::CDynamicBuffer output;
    bool ok = RunCodec( CORE::CoreCodecTypes::KeyAgreementCodec, KeyAgreementType, false, params, peerPublicKey, KeySize, output, GUCEF_NULL ) &&
              KeySize == output.GetDataSize();
    if ( ok )
    {
        // an all zero secret comes from a low order point of the peer: refused (RFC 7748, section 6.1)
        UInt8 zero[ KeySize ];
        memset( zero, 0, sizeof( zero ) );
        ok = !CORE::CSha256::ConstantTimeEquals( output.GetConstBufferPtr(), zero, KeySize );
        if ( ok )
            memcpy( sharedSecret, output.GetConstBufferPtr(), KeySize );
    }
    memset( output.GetBufferPtr(), 0, output.GetDataSize() );
    return ok;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::GetSignaturePublicKey( const UInt8* seed, UInt8* publicKey )
{GUCEF_TRACE;

    CORE::CValueList params;
    params.Set( "privateKey", CORE::CVariant( seed, KeySize ) );
    params.Set( "output", CORE::CVariant( "publicKey" ) );
    CORE::CDynamicBuffer output;
    if ( !RunCodec( CORE::CoreCodecTypes::SignatureCodec, SignatureType, true, params, GUCEF_NULL, 0, output, GUCEF_NULL ) ||
         KeySize != output.GetDataSize() )
        return false;
    memcpy( publicKey, output.GetConstBufferPtr(), KeySize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::GenerateSignatureKeyPair( UInt8* seed, UInt8* publicKey )
{GUCEF_TRACE;

    if ( !RandomBytes( seed, KeySize ) )
        return false;
    if ( !GetSignaturePublicKey( seed, publicKey ) )
    {
        memset( seed, 0, KeySize );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::Sign( const UInt8* seed, const void* message, UInt32 messageSize, UInt8* signature )
{GUCEF_TRACE;

    CORE::CValueList params;
    params.Set( "privateKey", CORE::CVariant( seed, KeySize ) );
    CORE::CDynamicBuffer output;
    if ( !RunCodec( CORE::CoreCodecTypes::SignatureCodec, SignatureType, true, params, message, messageSize, output, GUCEF_NULL ) ||
         SignatureSize != output.GetDataSize() )
        return false;
    memcpy( signature, output.GetConstBufferPtr(), SignatureSize );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::Verify( const UInt8* publicKey, const void* message, UInt32 messageSize, const UInt8* signature )
{GUCEF_TRACE;

    CORE::CValueList params;
    params.Set( "publicKey", CORE::CVariant( publicKey, KeySize ) );
    params.Set( "signature", CORE::CVariant( signature, SignatureSize ) );
    CORE::CDynamicBuffer output;
    CORE::TCodecStatus status = CORE::GUCEF_CODEC_STATUS_FAILED;
    return RunCodec( CORE::CoreCodecTypes::SignatureCodec, SignatureType, false, params, message, messageSize, output, &status ) &&
           CORE::GUCEF_CODEC_STATUS_OK == status;
}

/*-------------------------------------------------------------------------*/

bool
CDRNCrypto::Hkdf( const void* salt, UInt32 saltSize, const void* ikm, UInt32 ikmSize, const void* info, UInt32 infoSize, UInt8* okm, UInt32 okmSize )
{GUCEF_TRACE;

    return CORE::CHkdfSha256::Derive( salt, saltSize, ikm, ikmSize, info, infoSize, okm, okmSize );
}

/*-------------------------------------------------------------------------*/

void
CDRNCrypto::Sha256( const void* data, UInt32 size, UInt8* digest )
{GUCEF_TRACE;

    CORE::CSha256::Hash( data, size, digest );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDRNCrypto::ToHex( const void* data, UInt32 size )
{GUCEF_TRACE;

    static const char hexDigits[] = "0123456789abcdef";
    const UInt8* bytes = static_cast< const UInt8* >( data );
    CORE::CString result;
    if ( GUCEF_NULL == bytes || 0 == size )
        return result;
    char pair[ 3 ] = { 0, 0, 0 };
    for ( UInt32 i=0; i<size; ++i )
    {
        pair[ 0 ] = hexDigits[ bytes[ i ] >> 4 ];
        pair[ 1 ] = hexDigits[ bytes[ i ] & 0x0F ];
        result += pair;
    }
    return result;
}

/*-------------------------------------------------------------------------*/

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
CDRNCrypto::FromHex( const CORE::CString& hex, UInt8* output, UInt32 expectedSize )
{GUCEF_TRACE;

    const char* text = hex.C_String();
    UInt32 size = hex.ByteSize() > 0 ? hex.ByteSize() - 1 : 0;
    if ( GUCEF_NULL == text || size != expectedSize * 2 )
        return false;
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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
