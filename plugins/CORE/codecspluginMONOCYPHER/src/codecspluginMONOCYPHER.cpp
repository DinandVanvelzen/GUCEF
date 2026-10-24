/*
 *  codecspluginMONOCYPHER: Codec plugin for authenticated encryption, key agreement, signatures, hashing and key derivation using Monocypher
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
#include <stdlib.h>

#include "codecspluginMONOCYPHER.h"     // function prototypes for this plugin

#include "monocypher.h"
#include "monocypher-ed25519.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CODECSPLUGIN {
namespace MONOCYPHER {

using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define CODECSPLUGIN_MONOCYPHER_MAYOR_VERSION       0
#define CODECSPLUGIN_MONOCYPHER_MINOR_VERSION       1
#define CODECSPLUGIN_MONOCYPHER_PATCH_VERSION       0
#define CODECSPLUGIN_MONOCYPHER_RELEASE_VERSION     0

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const UInt32 KeySize = 32;
static const UInt32 TagSize = 16;
static const UInt32 SignatureSize = 64;
static const UInt32 MaxBlake2bSize = 64;
static const UInt32 MaxHkdfSha512Size = 255 * 64;
static const UInt32 ReadChunkSize = 4096;

/** What a codec of this plugin does, the codecData of its link */
enum EMonoCodecKind
{
    MONOCODEC_XCHACHA20_POLY1305 = 0,
    MONOCODEC_CHACHA20_POLY1305,
    MONOCODEC_X25519,
    MONOCODEC_ED25519,
    MONOCODEC_BLAKE2B,
    MONOCODEC_SHA512,
    MONOCODEC_HKDF_SHA512,
    MONOCODEC_ARGON2I
};

struct SMonoCodec
{
    EMonoCodecKind kind;
};

const SMonoCodec xchachaCodec = { MONOCODEC_XCHACHA20_POLY1305 };
const SMonoCodec chachaCodec = { MONOCODEC_CHACHA20_POLY1305 };
const SMonoCodec x25519Codec = { MONOCODEC_X25519 };
const SMonoCodec ed25519Codec = { MONOCODEC_ED25519 };
const SMonoCodec blake2bCodec = { MONOCODEC_BLAKE2B };
const SMonoCodec sha512Codec = { MONOCODEC_SHA512 };
const SMonoCodec hkdfSha512Codec = { MONOCODEC_HKDF_SHA512 };
const SMonoCodec argon2iCodec = { MONOCODEC_ARGON2I };

/*-------------------------------------------------------------------------*/

/** Memory of the plugin that is wiped before it is freed, for secrets and the input of a one shot call */
class CSecureBuffer
{
    public:

    CSecureBuffer( void ) : m_data( GUCEF_NULL ), m_size( 0 ), m_capacity( 0 ) {}

    ~CSecureBuffer() { Free(); }

    bool Reserve( UInt32 capacity )
    {
        if ( capacity <= m_capacity )
            return true;
        UInt8* data = static_cast< UInt8* >( malloc( capacity ) );
        if ( GUCEF_NULL == data )
            return false;
        if ( m_size > 0 )
            memcpy( data, m_data, m_size );
        Free( false );
        m_data = data;
        m_capacity = capacity;
        return true;
    }

    bool Append( const void* data, UInt32 size )
    {
        if ( size > GUCEF_UINT32MAX - m_size )
            return false;
        if ( m_size + size > m_capacity && !Reserve( m_size + size > 2 * m_capacity ? m_size + size : 2 * m_capacity ) )
            return false;
        if ( size > 0 )
            memcpy( m_data + m_size, data, size );
        m_size += size;
        return true;
    }

    UInt8* Data( void ) { return m_data; }
    UInt32 Size( void ) const { return m_size; }
    void SetSize( UInt32 size ) { m_size = size; }

    private:

    void Free( bool resetSize = true )
    {
        if ( GUCEF_NULL != m_data )
        {
            crypto_wipe( m_data, m_capacity );
            free( m_data );
        }
        m_data = GUCEF_NULL;
        m_capacity = 0;
        if ( resetSize )
            m_size = 0;
    }

    CSecureBuffer( const CSecureBuffer& );              /**< not implemented */
    CSecureBuffer& operator=( const CSecureBuffer& );   /**< not implemented */

    UInt8* m_data;
    UInt32 m_size;
    UInt32 m_capacity;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

TCodecStatus
SetError( TCodecErrorInfo* error, TCodecStatus status, const char* message )
{
    if ( GUCEF_NULL != error && GUCEF_NULL != message )
    {
        strncpy( error->message, message, GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        error->message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    }
    return status;
}

/*-------------------------------------------------------------------------*/

/** Looks up a parameter, false when it is absent */
bool
GetParam( const TVariantMapApi* params, const char* name, TVariantData& value )
{
    if ( GUCEF_NULL == params || GUCEF_NULL == params->at_key )
        return false;

    TVariantData key;
    memset( &key, 0, sizeof( key ) );
    key.containedType = GUCEF_DATATYPE_ASCII_STRING;
    key.union_data.heap_data.union_data.const_char_heap_data = name;
    key.union_data.heap_data.heap_data_size = (UInt32) strlen( name ) + 1;
    key.union_data.heap_data.heap_data_is_linked = 1;

    memset( &value, 0, sizeof( value ) );
    params->at_key( params->privateData, &key, &value );
    return GUCEF_DATATYPE_UNKNOWN != value.containedType;
}

/*-------------------------------------------------------------------------*/

bool
IsTextType( UInt8 typeId )
{
    return GUCEF_DATATYPE_ASCII_STRING == typeId || GUCEF_DATATYPE_UTF8_STRING == typeId;
}

/*-------------------------------------------------------------------------*/

/** The bytes of a parameter: a binary value or the bytes of a text (without its terminating zero). False when absent */
bool
GetBinaryParam( const TVariantMapApi* params, const char* name, const UInt8*& data, UInt32& size )
{
    TVariantData value;
    if ( !GetParam( params, name, value ) )
        return false;

    if ( GUCEF_DATATYPE_BINARY_BSOB == value.containedType )
    {
        data = GUCEF_NULL;  // a small object lives in the variant data that is handed over, it does not outlive this function
        size = 0;
        return false;
    }
    if ( GUCEF_DATATYPE_BINARY_BLOB != value.containedType && !IsTextType( value.containedType ) )
        return false;

    data = static_cast< const UInt8* >( value.union_data.heap_data.union_data.void_heap_data );
    size = value.union_data.heap_data.heap_data_size;
    if ( IsTextType( value.containedType ) && size > 0 && GUCEF_NULL != data && '\0' == data[ size-1 ] )
        --size;
    return GUCEF_NULL != data || 0 == size;
}

/*-------------------------------------------------------------------------*/

/** A whole number parameter: native integers or decimal text. 0: absent, 1: read, -1: not a whole number */
int
GetNumberParam( const TVariantMapApi* params, const char* name, UInt64& result )
{
    TVariantData value;
    if ( !GetParam( params, name, value ) )
        return 0;

    switch ( value.containedType )
    {
        case GUCEF_DATATYPE_UINT8 : { result = value.union_data.uint8_data; return 1; }
        case GUCEF_DATATYPE_UINT16 : { result = value.union_data.uint16_data; return 1; }
        case GUCEF_DATATYPE_UINT32 : { result = value.union_data.uint32_data; return 1; }
        case GUCEF_DATATYPE_UINT64 : { result = value.union_data.uint64_data; return 1; }
        case GUCEF_DATATYPE_INT8 : { if ( value.union_data.int8_data < 0 ) return -1; result = (UInt64) value.union_data.int8_data; return 1; }
        case GUCEF_DATATYPE_INT16 : { if ( value.union_data.int16_data < 0 ) return -1; result = (UInt64) value.union_data.int16_data; return 1; }
        case GUCEF_DATATYPE_INT32 : { if ( value.union_data.int32_data < 0 ) return -1; result = (UInt64) value.union_data.int32_data; return 1; }
        case GUCEF_DATATYPE_INT64 : { if ( value.union_data.int64_data < 0 ) return -1; result = (UInt64) value.union_data.int64_data; return 1; }
        case GUCEF_DATATYPE_ASCII_STRING :
        case GUCEF_DATATYPE_UTF8_STRING :
        {
            const char* text = value.union_data.heap_data.union_data.const_char_heap_data;
            UInt32 size = value.union_data.heap_data.heap_data_size;
            UInt64 number = 0;
            UInt32 digits = 0;
            for ( UInt32 i=0; GUCEF_NULL != text && i<size && '\0' != text[ i ]; ++i )
            {
                if ( text[ i ] < '0' || text[ i ] > '9' || digits >= 19 )
                    return -1;
                number = number * 10 + (UInt64) ( text[ i ] - '0' );
                ++digits;
            }
            if ( 0 == digits )
                return -1;
            result = number;
            return 1;
        }
        default :
            return -1;
    }
}

/*-------------------------------------------------------------------------*/

/** A number parameter with a default and a range */
TCodecStatus
GetRangedParam( const TVariantMapApi* params, const char* name, UInt64 defaultValue, UInt64 minimum, UInt64 maximum, UInt64& result, TCodecErrorInfo* error, const char* rangeMessage )
{
    result = defaultValue;
    int found = GetNumberParam( params, name, result );
    if ( found < 0 || result < minimum || result > maximum )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, rangeMessage );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** A text parameter compared against a word, absent counts as 'whenAbsent' */
int
TextParamIs( const TVariantMapApi* params, const char* name, const char* word, int whenAbsent )
{
    TVariantData value;
    if ( !GetParam( params, name, value ) )
        return whenAbsent;
    if ( !IsTextType( value.containedType ) )
        return 0;
    const char* text = value.union_data.heap_data.union_data.const_char_heap_data;
    UInt32 size = value.union_data.heap_data.heap_data_size;
    if ( GUCEF_NULL == text )
        return 0;
    if ( size > 0 && '\0' == text[ size-1 ] )
        --size;
    return ( strlen( word ) == size && 0 == strncmp( text, word, size ) ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

/** The "output" parameter of the hashes: 1 for raw, 0 for hex, -1 for anything else */
int
GetRawOutput( const TVariantMapApi* params )
{
    if ( TextParamIs( params, "output", "raw", 0 ) )
        return 1;
    if ( TextParamIs( params, "output", "hex", 1 ) )
        return 0;
    return -1;
}

/*-------------------------------------------------------------------------*/

bool
ReadAll( TIOAccess* input, CSecureBuffer& buffer )
{
    UInt8 chunk[ ReadChunkSize ];
    for ( ;; )
    {
        UInt32 bytesRead = input->read( input, chunk, 1, ReadChunkSize );
        if ( 0 == bytesRead )
            break;
        if ( !buffer.Append( chunk, bytesRead ) )
        {
            crypto_wipe( chunk, sizeof( chunk ) );
            return false;
        }
    }
    crypto_wipe( chunk, sizeof( chunk ) );
    return true;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
WriteAll( TIOAccess* output, const UInt8* data, UInt32 size, TCodecErrorInfo* error )
{
    while ( size > 0 )
    {
        UInt32 written = output->write( output, data, 1, size );
        if ( 0 == written || written > size )
            return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
        data += written;
        size -= written;
    }
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void
ToHex( const UInt8* bytes, UInt32 size, UInt8* hex )
{
    static const char* digits = "0123456789abcdef";
    for ( UInt32 i=0; i<size; ++i )
    {
        hex[ 2*i ] = (UInt8) digits[ bytes[ i ] >> 4 ];
        hex[ 2*i+1 ] = (UInt8) digits[ bytes[ i ] & 0xF ];
    }
}

/*-------------------------------------------------------------------------*/

UInt32
GetNonceSize( EMonoCodecKind kind )
{
    return MONOCODEC_XCHACHA20_POLY1305 == kind ? 24 : 12;
}

/*-------------------------------------------------------------------------*/

/** Seals or opens one AEAD message into memory with room for it. Nothing is written when the tag does not match */
TCodecStatus
ProcessAeadMessage( EMonoCodecKind kind, bool encode, const UInt8* key, const UInt8* nonce, UInt32 nonceSize,
                    const UInt8* aad, UInt32 aadSize, const UInt8* input, UInt32 inputSize, UInt8* output, TCodecErrorInfo* error )
{
    if ( GUCEF_NULL == nonce )
        return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "A nonce is required" );
    if ( GetNonceSize( kind ) != nonceSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, MONOCODEC_XCHACHA20_POLY1305 == kind ? "The nonce must be 24 bytes" : "The nonce must be 12 bytes" );
    if ( !encode && inputSize < TagSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is shorter than the tag" );

    // a context per message: the stream interface of Monocypher changes its key after each message
    crypto_aead_ctx context;
    if ( MONOCODEC_XCHACHA20_POLY1305 == kind )
        crypto_aead_init_x( &context, key, nonce );
    else
        crypto_aead_init_ietf( &context, key, nonce );

    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    if ( encode )
    {
        crypto_aead_write( &context, output, output + inputSize, aad, aadSize, input, inputSize );
    }
    else
    {
        const UInt32 textSize = inputSize - TagSize;
        if ( 0 != crypto_aead_read( &context, output, input + textSize, aad, aadSize, input, textSize ) )
            status = SetError( error, GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The tag does not match: the message is not authentic" );
    }
    crypto_wipe( &context, sizeof( context ) );
    return status;
}

/*-------------------------------------------------------------------------*/

bool
IsAllZero( const UInt8* data, UInt32 size )
{
    UInt8 bits = 0;
    for ( UInt32 i=0; i<size; ++i )
        bits |= data[ i ];
    return 0 == bits;
}

/*-------------------------------------------------------------------------*/

/** The derived key of the KDFs into memory with room for 'length' bytes */
TCodecStatus
DeriveKey( EMonoCodecKind kind, const TVariantMapApi* params, const UInt8* secret, UInt32 secretSize, CSecureBuffer& output, TCodecErrorInfo* error )
{
    const UInt8* salt = GUCEF_NULL;
    UInt32 saltSize = 0;
    GetBinaryParam( params, "salt", salt, saltSize );

    if ( MONOCODEC_HKDF_SHA512 == kind )
    {
        UInt64 length = 0;
        TCodecStatus status = GetRangedParam( params, "length", 64, 1, MaxHkdfSha512Size, length, error, "The parameter \"length\" must be 1 to 16320" );
        if ( GUCEF_CODEC_STATUS_OK != status )
            return status;
        const UInt8* info = GUCEF_NULL;
        UInt32 infoSize = 0;
        GetBinaryParam( params, "info", info, infoSize );
        if ( !output.Reserve( (UInt32) length ) )
            return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
        crypto_sha512_hkdf( output.Data(), (size_t) length, secret, secretSize, salt, saltSize, info, infoSize );
        output.SetSize( (UInt32) length );
        return GUCEF_CODEC_STATUS_OK;
    }

    // Argon2i
    if ( GUCEF_NULL == salt )
        return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"salt\" is required, 16 random bytes are recommended" );
    if ( saltSize < 8 )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"salt\" must be at least 8 bytes" );

    UInt64 length = 0, iterations = 0, lanes = 0, memory = 0;
    TCodecStatus status = GetRangedParam( params, "length", 32, 4, 1024 * 1024, length, error, "The parameter \"length\" must be 4 to 1048576" );
    if ( GUCEF_CODEC_STATUS_OK == status )
        status = GetRangedParam( params, "iterations", 3, 1, 1024 * 1024, iterations, error, "The parameter \"iterations\" must be 1 or more" );
    if ( GUCEF_CODEC_STATUS_OK == status )
        status = GetRangedParam( params, "lanes", 1, 1, 255, lanes, error, "The parameter \"lanes\" must be 1 to 255" );
    if ( GUCEF_CODEC_STATUS_OK == status )
        status = GetRangedParam( params, "memory", 65536, 8 * lanes, 4 * 1024 * 1024, memory, error, "The parameter \"memory\" (KiB) must be at least 8 per lane and at most 4194304" );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;

    crypto_argon2_config config;
    config.algorithm = CRYPTO_ARGON2_I;
    config.nb_blocks = (uint32_t) memory;
    config.nb_passes = (uint32_t) iterations;
    config.nb_lanes = (uint32_t) lanes;

    crypto_argon2_inputs inputs;
    inputs.pass = secret;
    inputs.pass_size = secretSize;
    inputs.salt = salt;
    inputs.salt_size = saltSize;

    crypto_argon2_extras extras = crypto_argon2_no_extras;
    const UInt8* key = GUCEF_NULL;
    UInt32 keySize = 0;
    if ( GetBinaryParam( params, "key", key, keySize ) )
    {
        extras.key = key;
        extras.key_size = keySize;
    }
    const UInt8* ad = GUCEF_NULL;
    UInt32 adSize = 0;
    if ( GetBinaryParam( params, "ad", ad, adSize ) )
    {
        extras.ad = ad;
        extras.ad_size = adSize;
    }

    void* workArea = malloc( (size_t) memory * 1024 );
    if ( GUCEF_NULL == workArea )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory for the work area" );
    if ( !output.Reserve( (UInt32) length ) )
    {
        free( workArea );
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    }
    crypto_argon2( output.Data(), (uint32_t) length, workArea, config, inputs, extras );
    crypto_wipe( workArea, (size_t) memory * 1024 );
    free( workArea );
    output.SetSize( (UInt32) length );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      LINK FUNCTIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoEncode( void* pluginData             ,
            void* codecData              ,
            const TVariantMapApi* params ,
            TIOAccess* input             ,
            TIOAccess* output            ,
            TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SMonoCodec* codec = static_cast< const SMonoCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );

    CSecureBuffer message;
    if ( !ReadAll( input, message ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to read the input" );

    switch ( codec->kind )
    {
        case MONOCODEC_XCHACHA20_POLY1305 :
        case MONOCODEC_CHACHA20_POLY1305 :
        {
            const UInt8* key = GUCEF_NULL;
            UInt32 keySize = 0;
            if ( !GetBinaryParam( params, "key", key, keySize ) )
                return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"key\" is required" );
            if ( KeySize != keySize )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"key\" must be 32 bytes" );
            const UInt8* nonce = GUCEF_NULL;
            UInt32 nonceSize = 0;
            GetBinaryParam( params, "nonce", nonce, nonceSize );
            const UInt8* aad = GUCEF_NULL;
            UInt32 aadSize = 0;
            GetBinaryParam( params, "aad", aad, aadSize );

            if ( message.Size() > GUCEF_UINT32MAX - TagSize )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is too large" );
            CSecureBuffer sealed;
            if ( !sealed.Reserve( message.Size() + TagSize ) )
                return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
            TCodecStatus status = ProcessAeadMessage( codec->kind, true, key, nonce, nonceSize, aad, aadSize, message.Data(), message.Size(), sealed.Data(), error );
            if ( GUCEF_CODEC_STATUS_OK != status )
                return status;
            return WriteAll( output, sealed.Data(), message.Size() + TagSize, error );
        }
        case MONOCODEC_X25519 :
        {
            // own private key -> own public key
            if ( KeySize != message.Size() )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input must be a private key of 32 bytes" );
            UInt8 publicKey[ KeySize ];
            crypto_x25519_public_key( publicKey, message.Data() );
            return WriteAll( output, publicKey, KeySize, error );
        }
        case MONOCODEC_ED25519 :
        {
            const UInt8* seed = GUCEF_NULL;
            UInt32 seedSize = 0;
            if ( !GetBinaryParam( params, "privateKey", seed, seedSize ) )
                return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"privateKey\" is required" );
            if ( KeySize != seedSize )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"privateKey\" must be the 32 byte seed of RFC 8032" );
            bool wantsPublicKey = 0 != TextParamIs( params, "output", "publicKey", 0 );
            if ( !wantsPublicKey && 0 == TextParamIs( params, "output", "signature", 1 ) )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"output\" must be \"signature\" or \"publicKey\"" );

            UInt8 seedCopy[ KeySize ];
            UInt8 secretKey[ 64 ];
            UInt8 publicKey[ KeySize ];
            memcpy( seedCopy, seed, KeySize );
            crypto_ed25519_key_pair( secretKey, publicKey, seedCopy );   // wipes the copy of the seed
            TCodecStatus status = GUCEF_CODEC_STATUS_OK;
            if ( wantsPublicKey )
            {
                status = WriteAll( output, publicKey, KeySize, error );
            }
            else
            {
                UInt8 signature[ SignatureSize ];
                crypto_ed25519_sign( signature, secretKey, message.Data(), message.Size() );
                status = WriteAll( output, signature, SignatureSize, error );
            }
            crypto_wipe( secretKey, sizeof( secretKey ) );
            return status;
        }
        case MONOCODEC_BLAKE2B :
        {
            int raw = GetRawOutput( params );
            if ( raw < 0 )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"output\" must be \"hex\" or \"raw\"" );
            UInt64 length = 0;
            TCodecStatus status = GetRangedParam( params, "length", MaxBlake2bSize, 1, MaxBlake2bSize, length, error, "The parameter \"length\" must be 1 to 64" );
            if ( GUCEF_CODEC_STATUS_OK != status )
                return status;
            const UInt8* key = GUCEF_NULL;
            UInt32 keySize = 0;
            GetBinaryParam( params, "key", key, keySize );
            if ( keySize > MaxBlake2bSize )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"key\" must be at most 64 bytes" );

            UInt8 digest[ MaxBlake2bSize ];
            UInt8 hex[ 2 * MaxBlake2bSize ];
            crypto_blake2b_keyed( digest, (size_t) length, key, keySize, message.Data(), message.Size() );
            if ( raw )
                return WriteAll( output, digest, (UInt32) length, error );
            ToHex( digest, (UInt32) length, hex );
            return WriteAll( output, hex, 2 * (UInt32) length, error );
        }
        case MONOCODEC_SHA512 :
        {
            int raw = GetRawOutput( params );
            if ( raw < 0 )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"output\" must be \"hex\" or \"raw\"" );
            UInt8 digest[ 64 ];
            UInt8 hex[ 128 ];
            crypto_sha512( digest, message.Data(), message.Size() );
            if ( raw )
                return WriteAll( output, digest, 64, error );
            ToHex( digest, 64, hex );
            return WriteAll( output, hex, 128, error );
        }
        case MONOCODEC_HKDF_SHA512 :
        case MONOCODEC_ARGON2I :
        {
            CSecureBuffer derived;
            TCodecStatus status = DeriveKey( codec->kind, params, message.Data(), message.Size(), derived, error );
            if ( GUCEF_CODEC_STATUS_OK != status )
                return status;
            return WriteAll( output, derived.Data(), derived.Size(), error );
        }
    }
    return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "Unknown codec" );
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoDecode( void* pluginData             ,
            void* codecData              ,
            const TVariantMapApi* params ,
            TIOAccess* input             ,
            TIOAccess* output            ,
            TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SMonoCodec* codec = static_cast< const SMonoCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );

    switch ( codec->kind )
    {
        case MONOCODEC_XCHACHA20_POLY1305 :
        case MONOCODEC_CHACHA20_POLY1305 :
        case MONOCODEC_X25519 :
        case MONOCODEC_ED25519 :
            break;
        default :
            return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "The codec does not decode" );
    }

    CSecureBuffer message;
    if ( !ReadAll( input, message ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to read the input" );

    if ( MONOCODEC_X25519 == codec->kind )
    {
        // peer public key -> shared secret
        const UInt8* privateKey = GUCEF_NULL;
        UInt32 privateKeySize = 0;
        if ( !GetBinaryParam( params, "privateKey", privateKey, privateKeySize ) )
            return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"privateKey\" is required" );
        if ( KeySize != privateKeySize )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"privateKey\" must be 32 bytes" );
        if ( KeySize != message.Size() )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input must be a public key of 32 bytes" );
        UInt8 sharedSecret[ KeySize ];
        crypto_x25519( sharedSecret, privateKey, message.Data() );
        if ( IsAllZero( sharedSecret, KeySize ) )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The public key is a point of low order: no shared secret" );
        TCodecStatus status = WriteAll( output, sharedSecret, KeySize, error );
        crypto_wipe( sharedSecret, sizeof( sharedSecret ) );
        return status;
    }

    if ( MONOCODEC_ED25519 == codec->kind )
    {
        // verifies the message, no output
        const UInt8* publicKey = GUCEF_NULL;
        UInt32 publicKeySize = 0;
        const UInt8* signature = GUCEF_NULL;
        UInt32 signatureSize = 0;
        if ( !GetBinaryParam( params, "publicKey", publicKey, publicKeySize ) )
            return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"publicKey\" is required" );
        if ( !GetBinaryParam( params, "signature", signature, signatureSize ) )
            return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"signature\" is required" );
        if ( KeySize != publicKeySize || SignatureSize != signatureSize )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The public key must be 32 bytes and the signature 64" );
        if ( 0 != crypto_ed25519_check( signature, publicKey, message.Data(), message.Size() ) )
            return SetError( error, GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The signature does not match the message and the public key" );
        return GUCEF_CODEC_STATUS_OK;
    }

    // AEAD
    const UInt8* key = GUCEF_NULL;
    UInt32 keySize = 0;
    if ( !GetBinaryParam( params, "key", key, keySize ) )
        return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"key\" is required" );
    if ( KeySize != keySize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"key\" must be 32 bytes" );
    const UInt8* nonce = GUCEF_NULL;
    UInt32 nonceSize = 0;
    GetBinaryParam( params, "nonce", nonce, nonceSize );
    const UInt8* aad = GUCEF_NULL;
    UInt32 aadSize = 0;
    GetBinaryParam( params, "aad", aad, aadSize );

    CSecureBuffer opened;
    if ( !opened.Reserve( message.Size() > TagSize ? message.Size() - TagSize : 1 ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    TCodecStatus status = ProcessAeadMessage( codec->kind, false, key, nonce, nonceSize, aad, aadSize, message.Data(), message.Size(), opened.Data(), error );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;
    return WriteAll( output, opened.Data(), message.Size() - TagSize, error );
}

/*-------------------------------------------------------------------------*/

struct SParamInfo
{
    const char* name;
    UInt8 typeId;
    UInt8 required;
    const char* defaultValue;
    const char* description;
};

const SParamInfo aeadParams[] =
{
    { "key", GUCEF_DATATYPE_BINARY_BLOB, 1, GUCEF_NULL, "The secret key, 32 bytes" },
    { "nonce", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Never twice with the same key: 12 bytes, 24 for xchacha20-poly1305; required for one shot use, per call for instances" },
    { "aad", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Associated data: authenticated, not encrypted" }
};
const SParamInfo x25519Params[] =
{
    { "privateKey", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Decode: own private key of 32 bytes. Encode takes the private key as input and gives the public key" }
};
const SParamInfo ed25519Params[] =
{
    { "privateKey", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Encode: the 32 byte seed of RFC 8032" },
    { "output", GUCEF_DATATYPE_STRING, 0, "signature", "Encode: \"signature\" signs the input, \"publicKey\" gives the public key of the private key" },
    { "publicKey", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Decode: the public key of 32 bytes" },
    { "signature", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Decode: the signature of 64 bytes to verify" }
};
const SParamInfo blake2bParams[] =
{
    { "output", GUCEF_DATATYPE_STRING, 0, "hex", "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes" },
    { "length", GUCEF_DATATYPE_UINT32, 0, "64", "The size of the digest in bytes, 1 to 64" },
    { "key", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "A key of at most 64 bytes for a keyed hash (a MAC)" }
};
const SParamInfo sha512Params[] =
{
    { "output", GUCEF_DATATYPE_STRING, 0, "hex", "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes" }
};
const SParamInfo hkdfParams[] =
{
    { "salt", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "A non secret random value" },
    { "info", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "What the key is for, binds the key to its use" },
    { "length", GUCEF_DATATYPE_UINT32, 0, "64", "The size of the derived key in bytes, at most 16320" }
};
const SParamInfo argon2Params[] =
{
    { "salt", GUCEF_DATATYPE_BINARY_BLOB, 1, GUCEF_NULL, "A random value of at least 8 bytes, 16 recommended, stored with the hash" },
    { "iterations", GUCEF_DATATYPE_UINT32, 0, "3", "Passes over the memory, the time cost" },
    { "memory", GUCEF_DATATYPE_UINT32, 0, "65536", "Memory in KiB, the memory cost" },
    { "lanes", GUCEF_DATATYPE_UINT32, 0, "1", "Parallelism (computed on one thread)" },
    { "length", GUCEF_DATATYPE_UINT32, 0, "32", "The size of the hash in bytes" },
    { "key", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "An optional secret (a pepper)" },
    { "ad", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "Optional associated data" }
};

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoGetParameterInfo( void* pluginData          ,
                      void* codecData           ,
                      UInt32 index              ,
                      TCodecParameterInfo* info ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SMonoCodec* codec = static_cast< const SMonoCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == info )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;

    const SParamInfo* list = GUCEF_NULL;
    UInt32 count = 0;
    switch ( codec->kind )
    {
        case MONOCODEC_XCHACHA20_POLY1305 :
        case MONOCODEC_CHACHA20_POLY1305 : { list = aeadParams; count = sizeof( aeadParams ) / sizeof( aeadParams[ 0 ] ); break; }
        case MONOCODEC_X25519 : { list = x25519Params; count = sizeof( x25519Params ) / sizeof( x25519Params[ 0 ] ); break; }
        case MONOCODEC_ED25519 : { list = ed25519Params; count = sizeof( ed25519Params ) / sizeof( ed25519Params[ 0 ] ); break; }
        case MONOCODEC_BLAKE2B : { list = blake2bParams; count = sizeof( blake2bParams ) / sizeof( blake2bParams[ 0 ] ); break; }
        case MONOCODEC_SHA512 : { list = sha512Params; count = sizeof( sha512Params ) / sizeof( sha512Params[ 0 ] ); break; }
        case MONOCODEC_HKDF_SHA512 : { list = hkdfParams; count = sizeof( hkdfParams ) / sizeof( hkdfParams[ 0 ] ); break; }
        case MONOCODEC_ARGON2I : { list = argon2Params; count = sizeof( argon2Params ) / sizeof( argon2Params[ 0 ] ); break; }
    }
    if ( index >= count )
        return GUCEF_CODEC_STATUS_UNSUPPORTED;

    info->name = list[ index ].name;
    info->typeId = list[ index ].typeId;
    info->required = list[ index ].required;
    info->defaultValue = list[ index ].defaultValue;
    info->description = list[ index ].description;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** An instance: an AEAD cipher with its key, or a hash in progress */
struct SMonoInstance
{
    EMonoCodecKind kind;
    UInt8 key[ KeySize ];
    crypto_blake2b_ctx blake2b;
    crypto_sha512_ctx sha512;
    UInt32 digestSize;
    UInt8 hashKey[ MaxBlake2bSize ];
    UInt32 hashKeySize;
    bool rawOutput;
};

/*-------------------------------------------------------------------------*/

void
StartHash( SMonoInstance* instance )
{
    if ( MONOCODEC_BLAKE2B == instance->kind )
        crypto_blake2b_keyed_init( &instance->blake2b, instance->digestSize, instance->hashKey, instance->hashKeySize );
    else
        crypto_sha512_init( &instance->sha512 );
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoCreateInstance( void* pluginData             ,
                    void* codecData              ,
                    const TVariantMapApi* params ,
                    void** instanceData          ,
                    TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SMonoCodec* codec = static_cast< const SMonoCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == instanceData )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec" );

    SMonoInstance* instance = static_cast< SMonoInstance* >( calloc( 1, sizeof( SMonoInstance ) ) );
    if ( GUCEF_NULL == instance )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    instance->kind = codec->kind;

    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    switch ( codec->kind )
    {
        case MONOCODEC_XCHACHA20_POLY1305 :
        case MONOCODEC_CHACHA20_POLY1305 :
        {
            const UInt8* key = GUCEF_NULL;
            UInt32 keySize = 0;
            if ( !GetBinaryParam( params, "key", key, keySize ) )
                status = SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"key\" is required" );
            else
            if ( KeySize != keySize )
                status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"key\" must be 32 bytes" );
            else
                memcpy( instance->key, key, KeySize );
            break;
        }
        case MONOCODEC_BLAKE2B :
        case MONOCODEC_SHA512 :
        {
            int raw = GetRawOutput( params );
            if ( raw < 0 )
            {
                status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"output\" must be \"hex\" or \"raw\"" );
                break;
            }
            instance->rawOutput = 0 != raw;
            instance->digestSize = 64;
            if ( MONOCODEC_BLAKE2B == codec->kind )
            {
                UInt64 length = 0;
                status = GetRangedParam( params, "length", MaxBlake2bSize, 1, MaxBlake2bSize, length, error, "The parameter \"length\" must be 1 to 64" );
                instance->digestSize = (UInt32) length;
                const UInt8* key = GUCEF_NULL;
                UInt32 keySize = 0;
                GetBinaryParam( params, "key", key, keySize );
                if ( keySize > MaxBlake2bSize )
                    status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"key\" must be at most 64 bytes" );
                else
                if ( keySize > 0 )
                {
                    memcpy( instance->hashKey, key, keySize );
                    instance->hashKeySize = keySize;
                }
            }
            if ( GUCEF_CODEC_STATUS_OK == status )
                StartHash( instance );
            break;
        }
        default :
            status = SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "The codec has no instances" );
    }

    if ( GUCEF_CODEC_STATUS_OK != status )
    {
        crypto_wipe( instance, sizeof( SMonoInstance ) );
        free( instance );
        return status;
    }
    *instanceData = instance;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoInstanceProcess( void* instanceData         ,
                     UInt32 flags               ,
                     const void* input          ,
                     UInt32 inputSize           ,
                     const TCodecCallArgs* args ,
                     void* output               ,
                     UInt32 outputCapacity      ,
                     UInt32* outputSize         ,
                     TCodecErrorInfo* error     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SMonoInstance* instance = static_cast< SMonoInstance* >( instanceData );
    if ( GUCEF_NULL == instance || GUCEF_NULL == outputSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No instance" );
    *outputSize = 0;

    const bool encode = 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE );
    if ( encode == ( 0 != ( flags & GUCEF_CODEC_PROCESS_DECODE ) ) )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "Exactly one of the encode and decode flags must be set" );
    if ( GUCEF_NULL == input && inputSize > 0 )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No input data" );
    const UInt8* in = static_cast< const UInt8* >( input );
    UInt8* out = static_cast< UInt8* >( output );

    if ( MONOCODEC_BLAKE2B == instance->kind || MONOCODEC_SHA512 == instance->kind )
    {
        if ( !encode )
            return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "A hash cannot be decoded" );
        const UInt32 resultSize = instance->rawOutput ? instance->digestSize : 2 * instance->digestSize;
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) && outputCapacity < resultSize )
            return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output has no room for the digest" );

        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            StartHash( instance );
        if ( inputSize > 0 )
        {
            if ( MONOCODEC_BLAKE2B == instance->kind )
                crypto_blake2b_update( &instance->blake2b, in, inputSize );
            else
                crypto_sha512_update( &instance->sha512, in, inputSize );
        }
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8 digest[ 64 ];
            if ( MONOCODEC_BLAKE2B == instance->kind )
                crypto_blake2b_final( &instance->blake2b, digest );
            else
                crypto_sha512_final( &instance->sha512, digest );
            if ( instance->rawOutput )
                memcpy( out, digest, instance->digestSize );
            else
                ToHex( digest, instance->digestSize, out );
            *outputSize = resultSize;
            StartHash( instance );
        }
        return GUCEF_CODEC_STATUS_OK;
    }

    // AEAD: a whole message per call, the room is checked before anything happens
    if ( ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) != ( flags & ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) ) )
        return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "An AEAD cipher takes a whole message per call: BEGIN and FINAL" );
    if ( GUCEF_NULL == args || args->structSize < sizeof( TCodecCallArgs ) )
        return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "A nonce is required" );
    if ( !encode && inputSize < TagSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is shorter than the tag" );
    if ( encode && inputSize > GUCEF_UINT32MAX - TagSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is too large" );
    const UInt32 resultSize = encode ? inputSize + TagSize : inputSize - TagSize;
    if ( outputCapacity < resultSize )
        return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output has no room for the message" );

    const UInt8* aad = static_cast< const UInt8* >( args->aad );
    TCodecStatus status = ProcessAeadMessage( instance->kind, encode, instance->key, static_cast< const UInt8* >( args->nonce ), args->nonceSize,
                                              aad, GUCEF_NULL != aad ? args->aadSize : 0, in, inputSize, out, error );
    if ( GUCEF_CODEC_STATUS_OK == status )
        *outputSize = resultSize;
    return status;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoDestroyInstance( void* instanceData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL != instanceData )
    {
        crypto_wipe( instanceData, sizeof( SMonoInstance ) );
        free( instanceData );
    }
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
MonoGetOutputSizeBound( void* pluginData   ,
                        void* codecData    ,
                        void* instanceData ,
                        UInt32 flags       ,
                        UInt32 inputSize   ,
                        UInt32* bound      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SMonoCodec* codec = static_cast< const SMonoCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == bound )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;

    const bool final = 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL );
    switch ( codec->kind )
    {
        case MONOCODEC_XCHACHA20_POLY1305 :
        case MONOCODEC_CHACHA20_POLY1305 :
        {
            if ( 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE ) )
                *bound = inputSize <= GUCEF_UINT32MAX - TagSize ? inputSize + TagSize : GUCEF_UINT32MAX;
            else
                *bound = inputSize > TagSize ? inputSize - TagSize : 0;
            return GUCEF_CODEC_STATUS_OK;
        }
        case MONOCODEC_BLAKE2B :
        case MONOCODEC_SHA512 :
        {
            const SMonoInstance* instance = static_cast< const SMonoInstance* >( instanceData );
            UInt32 size = 128;
            if ( GUCEF_NULL != instance )
                size = instance->rawOutput ? instance->digestSize : 2 * instance->digestSize;
            *bound = final ? size : 0;
            return GUCEF_CODEC_STATUS_OK;
        }
        case MONOCODEC_X25519 : { *bound = KeySize; return GUCEF_CODEC_STATUS_OK; }
        case MONOCODEC_ED25519 : { *bound = SignatureSize; return GUCEF_CODEC_STATUS_OK; }
        default :
            return GUCEF_CODEC_STATUS_UNSUPPORTED;
    }
}

/*-------------------------------------------------------------------------*/

#define MONO_LINK( family, type, codec, capabilities, overhead )                                                   \
    { sizeof( TCodecPluginLink ), family, type, const_cast< SMonoCodec* >( &codec ), capabilities, 0, overhead,  \
      MonoEncode, MonoDecode, MonoGetParameterInfo, MonoCreateInstance, MonoInstanceProcess, MonoDestroyInstance, \
      MonoGetOutputSizeBound }

#define MONO_AEAD_CAPABILITIES      ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC | GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY )
#define MONO_HASH_CAPABILITIES      ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
#define MONO_KEYS_CAPABILITIES      ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
#define MONO_KDF_CAPABILITIES       ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )

const TCodecPluginLink codecLinks[] =
{
    MONO_LINK( "EncryptionCodec", "xchacha20-poly1305", xchachaCodec, MONO_AEAD_CAPABILITIES, TagSize ),
    MONO_LINK( "EncryptionCodec", "chacha20-poly1305", chachaCodec, MONO_AEAD_CAPABILITIES, TagSize ),
    MONO_LINK( "KeyAgreementCodec", "x25519", x25519Codec, MONO_KEYS_CAPABILITIES, 0 ),
    MONO_LINK( "SignatureCodec", "ed25519", ed25519Codec, MONO_KEYS_CAPABILITIES, 0 ),
    MONO_LINK( "CryptographicHashCodec", "blake2b", blake2bCodec, MONO_HASH_CAPABILITIES, 0 ),
    MONO_LINK( "CryptographicHashCodec", "sha512", sha512Codec, MONO_HASH_CAPABILITIES, 0 ),
    MONO_LINK( "KeyDerivationCodec", "hkdf-sha512", hkdfSha512Codec, MONO_KDF_CAPABILITIES, 0 ),
    MONO_LINK( "KeyDerivationCodec", "argon2i", argon2iCodec, MONO_KDF_CAPABILITIES, 0 )
};
const UInt32 codecLinkCount = (UInt32) ( sizeof( codecLinks ) / sizeof( codecLinks[ 0 ] ) );

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetApiVersion( void ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return GUCEF_CODECPLUGIN_API_VERSION;
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Init( void** plugdata                    ,
                  TGucefCoreCApi* coreApi            ,
                  const TVariantMapApi* pluginParams ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *plugdata = GUCEF_NULL;
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Shutdown( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Description( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "codecspluginMONOCYPHER: XChaCha20-Poly1305, ChaCha20-Poly1305, X25519, Ed25519, BLAKE2b, SHA-512, HKDF-SHA512 and Argon2i codecs using Monocypher";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Plugin Copyright (C) Dinand Vanvelzen. 1998 - 2026.  Licensed under the Apache License, Version 2.0. Monocypher is licensed under CC0 or the 2-clause BSD license";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;

    version.major = CODECSPLUGIN_MONOCYPHER_MAYOR_VERSION;
    version.minor = CODECSPLUGIN_MONOCYPHER_MINOR_VERSION;
    version.patch = CODECSPLUGIN_MONOCYPHER_PATCH_VERSION;
    version.release = CODECSPLUGIN_MONOCYPHER_RELEASE_VERSION;

    return version;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecCount( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return codecLinkCount;
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata                     ,
                          UInt32 index                       ,
                          const TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL == codecLink )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    if ( index >= codecLinkCount )
    {
        *codecLink = GUCEF_NULL;
        return GUCEF_CODEC_STATUS_UNSUPPORTED;
    }
    *codecLink = &codecLinks[ index ];
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MONOCYPHER */
}; /* namespace CODECSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
