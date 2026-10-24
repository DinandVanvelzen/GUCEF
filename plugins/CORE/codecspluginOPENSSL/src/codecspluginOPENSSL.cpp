/*
 *  codecspluginOPENSSL: Codec plugin for authenticated encryption, key agreement, signatures, hashing and key derivation using the OpenSSL libcrypto of the system
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

#include "codecspluginOPENSSL.h"     // function prototypes for this plugin

/*
 *  Without the OpenSSL 3 development headers the plugin builds without codecs
 */
#if defined( __has_include )
  #if __has_include( <openssl/evp.h> )
    #include <openssl/opensslv.h>
    #if OPENSSL_VERSION_NUMBER >= 0x30000000L
      #include <openssl/evp.h>
      #include <openssl/kdf.h>
      #include <openssl/core_names.h>
      #include <openssl/crypto.h>
      #define GUCEF_CODECSPLUGIN_OPENSSL_AVAILABLE
    #endif
  #endif
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CODECSPLUGIN {
namespace OPENSSL {

using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define CODECSPLUGIN_OPENSSL_MAYOR_VERSION       0
#define CODECSPLUGIN_OPENSSL_MINOR_VERSION       1
#define CODECSPLUGIN_OPENSSL_PATCH_VERSION       0
#define CODECSPLUGIN_OPENSSL_RELEASE_VERSION     0

#ifdef GUCEF_CODECSPLUGIN_OPENSSL_AVAILABLE

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

static const UInt32 KeySize = 32;
static const UInt32 NonceSize = 12;
static const UInt32 TagSize = 16;
static const UInt32 SignatureSize = 64;
static const UInt32 ReadChunkSize = 4096;
static const UInt32 MaxHkdfSha256Size = 255 * 32;

/** What a codec of this plugin does, the codecData of its link */
enum ESslCodecKind
{
    SSLCODEC_AES256GCM = 0,
    SSLCODEC_CHACHA20_POLY1305,
    SSLCODEC_X25519,
    SSLCODEC_ED25519,
    SSLCODEC_HKDF_SHA256,
    SSLCODEC_HMAC_SHA256,
    SSLCODEC_SHA256,
    SSLCODEC_SHA512
};

struct SSslCodec
{
    ESslCodecKind kind;
    const char* algorithmName;      /**< as OpenSSL fetches it, to see whether the library offers it */
};

const SSslCodec aesGcmCodec = { SSLCODEC_AES256GCM, "AES-256-GCM" };
const SSslCodec chachaCodec = { SSLCODEC_CHACHA20_POLY1305, "ChaCha20-Poly1305" };
const SSslCodec x25519Codec = { SSLCODEC_X25519, "X25519" };
const SSslCodec ed25519Codec = { SSLCODEC_ED25519, "ED25519" };
const SSslCodec hkdfCodec = { SSLCODEC_HKDF_SHA256, "HKDF" };
const SSslCodec hmacCodec = { SSLCODEC_HMAC_SHA256, "HMAC" };
const SSslCodec sha256Codec = { SSLCODEC_SHA256, "SHA256" };
const SSslCodec sha512Codec = { SSLCODEC_SHA512, "SHA512" };

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
            OPENSSL_cleanse( m_data, m_capacity );
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

/*-------------------------------------------------------------------------*/

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
            OPENSSL_cleanse( chunk, sizeof( chunk ) );
            return false;
        }
    }
    OPENSSL_cleanse( chunk, sizeof( chunk ) );
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

bool
IsAllZero( const UInt8* data, UInt32 size )
{
    UInt8 bits = 0;
    for ( UInt32 i=0; i<size; ++i )
        bits |= data[ i ];
    return 0 == bits;
}

/*-------------------------------------------------------------------------*/

bool
IsAead( ESslCodecKind kind )
{
    return SSLCODEC_AES256GCM == kind || SSLCODEC_CHACHA20_POLY1305 == kind;
}

/*-------------------------------------------------------------------------*/

bool
IsHash( ESslCodecKind kind )
{
    return SSLCODEC_SHA256 == kind || SSLCODEC_SHA512 == kind || SSLCODEC_HMAC_SHA256 == kind;
}

/*-------------------------------------------------------------------------*/

const EVP_CIPHER*
GetCipher( ESslCodecKind kind )
{
    return SSLCODEC_AES256GCM == kind ? EVP_aes_256_gcm() : EVP_chacha20_poly1305();
}

/*-------------------------------------------------------------------------*/

const EVP_MD*
GetDigest( ESslCodecKind kind )
{
    return SSLCODEC_SHA512 == kind ? EVP_sha512() : EVP_sha256();
}

/*-------------------------------------------------------------------------*/

/** An instance: an AEAD cipher with its key in a context per direction, or a hash or HMAC in progress */
struct SSslInstance
{
    ESslCodecKind kind;
    EVP_CIPHER_CTX* encryptContext;
    EVP_CIPHER_CTX* decryptContext;
    EVP_MD_CTX* digestContext;
    EVP_MAC_CTX* macContext;
    bool rawOutput;
    UInt32 digestSize;
};

/*-------------------------------------------------------------------------*/

void
DestroyInstance( SSslInstance* instance )
{
    if ( GUCEF_NULL == instance )
        return;
    EVP_CIPHER_CTX_free( instance->encryptContext );
    EVP_CIPHER_CTX_free( instance->decryptContext );
    EVP_MD_CTX_free( instance->digestContext );
    EVP_MAC_CTX_free( instance->macContext );
    free( instance );
}

/*-------------------------------------------------------------------------*/

/** A cipher context with the key, ready for a nonce per message */
EVP_CIPHER_CTX*
CreateCipherContext( ESslCodecKind kind, bool encrypt, const UInt8* key )
{
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if ( GUCEF_NULL == context )
        return GUCEF_NULL;
    if ( 1 != EVP_CipherInit_ex( context, GetCipher( kind ), GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, encrypt ? 1 : 0 ) ||
         1 != EVP_CIPHER_CTX_ctrl( context, EVP_CTRL_AEAD_SET_IVLEN, (int) NonceSize, GUCEF_NULL ) ||
         1 != EVP_CipherInit_ex( context, GUCEF_NULL, GUCEF_NULL, key, GUCEF_NULL, encrypt ? 1 : 0 ) )
    {
        EVP_CIPHER_CTX_free( context );
        return GUCEF_NULL;
    }
    return context;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
StartHash( SSslInstance* instance, TCodecErrorInfo* error )
{
    if ( SSLCODEC_HMAC_SHA256 == instance->kind )
    {
        // the key that was set stays
        if ( 1 != EVP_MAC_init( instance->macContext, GUCEF_NULL, 0, GUCEF_NULL ) )
            return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to start the HMAC" );
        return GUCEF_CODEC_STATUS_OK;
    }
    if ( 1 != EVP_DigestInit_ex( instance->digestContext, GetDigest( instance->kind ), GUCEF_NULL ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to start the hash" );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
CreateSslInstance( ESslCodecKind kind, const TVariantMapApi* params, SSslInstance*& result, TCodecErrorInfo* error )
{
    SSslInstance* instance = static_cast< SSslInstance* >( calloc( 1, sizeof( SSslInstance ) ) );
    if ( GUCEF_NULL == instance )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    instance->kind = kind;

    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    if ( IsAead( kind ) )
    {
        const UInt8* key = GUCEF_NULL;
        UInt32 keySize = 0;
        if ( !GetBinaryParam( params, "key", key, keySize ) )
            status = SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"key\" is required" );
        else
        if ( KeySize != keySize )
            status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"key\" must be 32 bytes" );
        else
        {
            instance->encryptContext = CreateCipherContext( kind, true, key );
            instance->decryptContext = CreateCipherContext( kind, false, key );
            if ( GUCEF_NULL == instance->encryptContext || GUCEF_NULL == instance->decryptContext )
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to set up the cipher" );
        }
    }
    else
    if ( SSLCODEC_HMAC_SHA256 == kind )
    {
        const UInt8* key = GUCEF_NULL;
        UInt32 keySize = 0;
        instance->rawOutput = true;
        instance->digestSize = 32;
        if ( !GetBinaryParam( params, "key", key, keySize ) )
        {
            status = SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"key\" is required" );
        }
        else
        {
            EVP_MAC* mac = EVP_MAC_fetch( GUCEF_NULL, "HMAC", GUCEF_NULL );
            instance->macContext = GUCEF_NULL != mac ? EVP_MAC_CTX_new( mac ) : GUCEF_NULL;
            EVP_MAC_free( mac );
            char digestName[] = "SHA256";
            OSSL_PARAM macParams[ 2 ];
            macParams[ 0 ] = OSSL_PARAM_construct_utf8_string( OSSL_MAC_PARAM_DIGEST, digestName, 0 );
            macParams[ 1 ] = OSSL_PARAM_construct_end();
            static const UInt8 emptyKey = 0;
            if ( GUCEF_NULL == instance->macContext ||
                 1 != EVP_MAC_init( instance->macContext, keySize > 0 ? key : &emptyKey, keySize, macParams ) )
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to set up the HMAC" );
        }
    }
    else
    if ( IsHash( kind ) )
    {
        int raw = GetRawOutput( params );
        instance->digestSize = SSLCODEC_SHA512 == kind ? 64 : 32;
        if ( raw < 0 )
        {
            status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"output\" must be \"hex\" or \"raw\"" );
        }
        else
        {
            instance->rawOutput = 0 != raw;
            instance->digestContext = EVP_MD_CTX_new();
            if ( GUCEF_NULL == instance->digestContext )
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
            else
                status = StartHash( instance, error );
        }
    }
    else
    {
        status = SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "The codec has no instances" );
    }

    if ( GUCEF_CODEC_STATUS_OK != status )
    {
        DestroyInstance( instance );
        return status;
    }
    result = instance;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** One step of an instance into memory with 'outputCapacity' bytes of room */
TCodecStatus
ProcessSslInstance( SSslInstance* instance, UInt32 flags, const UInt8* in, UInt32 inputSize, const UInt8* nonce, UInt32 nonceSize,
                    const UInt8* aad, UInt32 aadSize, UInt8* out, UInt32 outputCapacity, UInt32& outputSize, TCodecErrorInfo* error )
{
    outputSize = 0;
    const bool encode = 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE );
    if ( encode == ( 0 != ( flags & GUCEF_CODEC_PROCESS_DECODE ) ) )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "Exactly one of the encode and decode flags must be set" );
    if ( GUCEF_NULL == in && inputSize > 0 )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No input data" );

    if ( IsHash( instance->kind ) )
    {
        if ( !encode )
            return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "A hash cannot be decoded" );
        const UInt32 resultSize = instance->rawOutput ? instance->digestSize : 2 * instance->digestSize;
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) && outputCapacity < resultSize )
            return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output has no room for the digest" );

        TCodecStatus status = GUCEF_CODEC_STATUS_OK;
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            status = StartHash( instance, error );
        if ( GUCEF_CODEC_STATUS_OK == status && inputSize > 0 )
        {
            int result = SSLCODEC_HMAC_SHA256 == instance->kind ? EVP_MAC_update( instance->macContext, in, inputSize )
                                                                : EVP_DigestUpdate( instance->digestContext, in, inputSize );
            if ( 1 != result )
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to hash the input" );
        }
        if ( GUCEF_CODEC_STATUS_OK == status && 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
        {
            UInt8 digest[ 64 ];
            int result = 0;
            if ( SSLCODEC_HMAC_SHA256 == instance->kind )
            {
                size_t macSize = 0;
                result = EVP_MAC_final( instance->macContext, digest, &macSize, sizeof( digest ) );
            }
            else
            {
                unsigned int digestSize = 0;
                result = EVP_DigestFinal_ex( instance->digestContext, digest, &digestSize );
            }
            if ( 1 != result )
                return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to finish the hash" );
            if ( instance->rawOutput )
                memcpy( out, digest, instance->digestSize );
            else
                ToHex( digest, instance->digestSize, out );
            outputSize = resultSize;
            status = StartHash( instance, error );
        }
        return status;
    }

    // AEAD: a whole message per call, the room is checked before anything happens
    if ( ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) != ( flags & ( GUCEF_CODEC_PROCESS_BEGIN | GUCEF_CODEC_PROCESS_FINAL ) ) )
        return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "An AEAD cipher takes a whole message per call: BEGIN and FINAL" );
    if ( GUCEF_NULL == nonce )
        return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "A nonce is required" );
    if ( NonceSize != nonceSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The nonce must be 12 bytes" );
    if ( !encode && inputSize < TagSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is shorter than the tag" );
    if ( encode && inputSize > 0x7FFFFFF0 )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is too large" );
    const UInt32 textSize = encode ? inputSize : inputSize - TagSize;
    const UInt32 resultSize = encode ? inputSize + TagSize : textSize;
    if ( outputCapacity < resultSize )
        return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output has no room for the message" );

    EVP_CIPHER_CTX* context = encode ? instance->encryptContext : instance->decryptContext;
    int length = 0;
    if ( 1 != EVP_CipherInit_ex( context, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, nonce, encode ? 1 : 0 ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to set the nonce" );
    if ( aadSize > 0 && 1 != EVP_CipherUpdate( context, GUCEF_NULL, &length, aad, (int) aadSize ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to take the associated data" );
    int written = 0;
    if ( textSize > 0 && 1 != EVP_CipherUpdate( context, out, &written, in, (int) textSize ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to process the message" );

    if ( encode )
    {
        int finalLength = 0;
        if ( 1 != EVP_CipherFinal_ex( context, out + written, &finalLength ) ||
             1 != EVP_CIPHER_CTX_ctrl( context, EVP_CTRL_AEAD_GET_TAG, (int) TagSize, out + textSize ) )
            return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to finish the message" );
    }
    else
    {
        int finalLength = 0;
        if ( 1 != EVP_CIPHER_CTX_ctrl( context, EVP_CTRL_AEAD_SET_TAG, (int) TagSize, const_cast< UInt8* >( in + textSize ) ) ||
             1 != EVP_CipherFinal_ex( context, out + written, &finalLength ) )
        {
            // what was decrypted is not authentic, it does not stay behind
            if ( textSize > 0 )
                OPENSSL_cleanse( out, textSize );
            return SetError( error, GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The tag does not match: the message is not authentic" );
        }
    }
    outputSize = resultSize;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

/** A raw key of 32 bytes as an OpenSSL key of the algorithm */
EVP_PKEY*
RawKey( int type, bool isPrivate, const UInt8* key )
{
    return isPrivate ? EVP_PKEY_new_raw_private_key( type, GUCEF_NULL, key, KeySize ) : EVP_PKEY_new_raw_public_key( type, GUCEF_NULL, key, KeySize );
}

/*-------------------------------------------------------------------------*/

TCodecStatus
PublicKeyOf( int type, const UInt8* privateKey, UInt8* publicKey, TCodecErrorInfo* error )
{
    EVP_PKEY* key = RawKey( type, true, privateKey );
    size_t size = KeySize;
    bool success = GUCEF_NULL != key && 1 == EVP_PKEY_get_raw_public_key( key, publicKey, &size ) && KeySize == size;
    EVP_PKEY_free( key );
    if ( !success )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "OpenSSL: the private key was refused" );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
X25519SharedSecret( const UInt8* privateKey, const UInt8* peerPublicKey, UInt8* secret, TCodecErrorInfo* error )
{
    EVP_PKEY* own = RawKey( EVP_PKEY_X25519, true, privateKey );
    EVP_PKEY* peer = RawKey( EVP_PKEY_X25519, false, peerPublicKey );
    EVP_PKEY_CTX* context = GUCEF_NULL != own ? EVP_PKEY_CTX_new( own, GUCEF_NULL ) : GUCEF_NULL;
    size_t size = KeySize;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    if ( GUCEF_NULL == own || GUCEF_NULL == peer || GUCEF_NULL == context || 1 != EVP_PKEY_derive_init( context ) )
        status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "OpenSSL: the keys were refused" );
    else
    if ( 1 != EVP_PKEY_derive_set_peer( context, peer ) || 1 != EVP_PKEY_derive( context, secret, &size ) || KeySize != size || IsAllZero( secret, KeySize ) )
        status = SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The public key is a point of low order: no shared secret" );
    EVP_PKEY_CTX_free( context );
    EVP_PKEY_free( peer );
    EVP_PKEY_free( own );
    return status;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
Ed25519Sign( const UInt8* seed, const UInt8* message, UInt32 messageSize, UInt8* signature, TCodecErrorInfo* error )
{
    EVP_PKEY* key = RawKey( EVP_PKEY_ED25519, true, seed );
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    size_t size = SignatureSize;
    static const UInt8 nothing = 0;
    bool success = GUCEF_NULL != key && GUCEF_NULL != context &&
                   1 == EVP_DigestSignInit( context, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, key ) &&
                   1 == EVP_DigestSign( context, signature, &size, messageSize > 0 ? message : &nothing, messageSize ) && SignatureSize == size;
    EVP_MD_CTX_free( context );
    EVP_PKEY_free( key );
    if ( !success )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to sign" );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
Ed25519Verify( const UInt8* publicKey, const UInt8* signature, const UInt8* message, UInt32 messageSize, TCodecErrorInfo* error )
{
    EVP_PKEY* key = RawKey( EVP_PKEY_ED25519, false, publicKey );
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    static const UInt8 nothing = 0;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    if ( GUCEF_NULL == key || GUCEF_NULL == context || 1 != EVP_DigestVerifyInit( context, GUCEF_NULL, GUCEF_NULL, GUCEF_NULL, key ) )
        status = SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "OpenSSL: the public key was refused" );
    else
    if ( 1 != EVP_DigestVerify( context, signature, SignatureSize, messageSize > 0 ? message : &nothing, messageSize ) )
        status = SetError( error, GUCEF_CODEC_STATUS_AUTHENTICATION_FAILED, "The signature does not match the message and the public key" );
    EVP_MD_CTX_free( context );
    EVP_PKEY_free( key );
    return status;
}

/*-------------------------------------------------------------------------*/

TCodecStatus
HkdfSha256( const TVariantMapApi* params, const UInt8* secret, UInt32 secretSize, CSecureBuffer& output, TCodecErrorInfo* error )
{
    UInt64 length = 0;
    TCodecStatus status = GetRangedParam( params, "length", 32, 1, MaxHkdfSha256Size, length, error, "The parameter \"length\" must be 1 to 8160" );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;
    const UInt8* salt = GUCEF_NULL;
    UInt32 saltSize = 0;
    GetBinaryParam( params, "salt", salt, saltSize );
    const UInt8* info = GUCEF_NULL;
    UInt32 infoSize = 0;
    GetBinaryParam( params, "info", info, infoSize );
    if ( !output.Reserve( (UInt32) length ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );

    // an absent salt is HashLen zeros, as RFC 5869 says
    static const UInt8 zeroSalt[ 32 ] = { 0 };
    static const UInt8 nothing = 0;
    EVP_KDF* kdf = EVP_KDF_fetch( GUCEF_NULL, "HKDF", GUCEF_NULL );
    EVP_KDF_CTX* context = GUCEF_NULL != kdf ? EVP_KDF_CTX_new( kdf ) : GUCEF_NULL;
    EVP_KDF_free( kdf );
    char digestName[] = "SHA256";
    OSSL_PARAM kdfParams[ 5 ];
    kdfParams[ 0 ] = OSSL_PARAM_construct_utf8_string( OSSL_KDF_PARAM_DIGEST, digestName, 0 );
    kdfParams[ 1 ] = OSSL_PARAM_construct_octet_string( OSSL_KDF_PARAM_KEY, const_cast< UInt8* >( secretSize > 0 ? secret : &nothing ), secretSize );
    kdfParams[ 2 ] = OSSL_PARAM_construct_octet_string( OSSL_KDF_PARAM_SALT, const_cast< UInt8* >( saltSize > 0 ? salt : zeroSalt ), saltSize > 0 ? saltSize : sizeof( zeroSalt ) );
    kdfParams[ 3 ] = OSSL_PARAM_construct_octet_string( OSSL_KDF_PARAM_INFO, const_cast< UInt8* >( infoSize > 0 ? info : &nothing ), infoSize );
    kdfParams[ 4 ] = OSSL_PARAM_construct_end();
    bool success = GUCEF_NULL != context && 1 == EVP_KDF_derive( context, output.Data(), (size_t) length, kdfParams );
    EVP_KDF_CTX_free( context );
    if ( !success )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "OpenSSL: unable to derive the key" );
    output.SetSize( (UInt32) length );
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      LINK FUNCTIONS                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
SslEncode( void* pluginData             ,
           void* codecData              ,
           const TVariantMapApi* params ,
           TIOAccess* input             ,
           TIOAccess* output            ,
           TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SSslCodec* codec = static_cast< const SSslCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );

    CSecureBuffer message;
    if ( !ReadAll( input, message ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to read the input" );

    switch ( codec->kind )
    {
        case SSLCODEC_X25519 :
        {
            if ( KeySize != message.Size() )
                return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input must be a private key of 32 bytes" );
            UInt8 publicKey[ KeySize ];
            TCodecStatus status = PublicKeyOf( EVP_PKEY_X25519, message.Data(), publicKey, error );
            return GUCEF_CODEC_STATUS_OK == status ? WriteAll( output, publicKey, KeySize, error ) : status;
        }
        case SSLCODEC_ED25519 :
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
            if ( wantsPublicKey )
            {
                UInt8 publicKey[ KeySize ];
                TCodecStatus status = PublicKeyOf( EVP_PKEY_ED25519, seed, publicKey, error );
                return GUCEF_CODEC_STATUS_OK == status ? WriteAll( output, publicKey, KeySize, error ) : status;
            }
            UInt8 signature[ SignatureSize ];
            TCodecStatus status = Ed25519Sign( seed, message.Data(), message.Size(), signature, error );
            return GUCEF_CODEC_STATUS_OK == status ? WriteAll( output, signature, SignatureSize, error ) : status;
        }
        case SSLCODEC_HKDF_SHA256 :
        {
            CSecureBuffer derived;
            TCodecStatus status = HkdfSha256( params, message.Data(), message.Size(), derived, error );
            return GUCEF_CODEC_STATUS_OK == status ? WriteAll( output, derived.Data(), derived.Size(), error ) : status;
        }
        default :
            break;
    }

    // the codecs with instances run one shot through one
    SSslInstance* instance = GUCEF_NULL;
    TCodecStatus status = CreateSslInstance( codec->kind, params, instance, error );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;

    const UInt8* nonce = GUCEF_NULL;
    UInt32 nonceSize = 0;
    GetBinaryParam( params, "nonce", nonce, nonceSize );
    const UInt8* aad = GUCEF_NULL;
    UInt32 aadSize = 0;
    GetBinaryParam( params, "aad", aad, aadSize );

    CSecureBuffer result;
    UInt32 room = IsAead( codec->kind ) ? message.Size() + TagSize : 128;
    UInt32 written = 0;
    if ( IsAead( codec->kind ) && message.Size() > 0x7FFFFFF0 )
        status = SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is too large" );
    else
    if ( !result.Reserve( room ) )
        status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    else
        status = ProcessSslInstance( instance, GUCEF_CODEC_PROCESS_ENCODE_MESSAGE, message.Data(), message.Size(), nonce, nonceSize, aad, aadSize, result.Data(), room, written, error );
    DestroyInstance( instance );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;
    return WriteAll( output, result.Data(), written, error );
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
SslDecode( void* pluginData             ,
           void* codecData              ,
           const TVariantMapApi* params ,
           TIOAccess* input             ,
           TIOAccess* output            ,
           TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SSslCodec* codec = static_cast< const SSslCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );
    if ( !IsAead( codec->kind ) && SSLCODEC_X25519 != codec->kind && SSLCODEC_ED25519 != codec->kind )
        return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "The codec does not decode" );

    CSecureBuffer message;
    if ( !ReadAll( input, message ) )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to read the input" );

    if ( SSLCODEC_X25519 == codec->kind )
    {
        const UInt8* privateKey = GUCEF_NULL;
        UInt32 privateKeySize = 0;
        if ( !GetBinaryParam( params, "privateKey", privateKey, privateKeySize ) )
            return SetError( error, GUCEF_CODEC_STATUS_MISSING_PARAMETER, "The parameter \"privateKey\" is required" );
        if ( KeySize != privateKeySize )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"privateKey\" must be 32 bytes" );
        if ( KeySize != message.Size() )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input must be a public key of 32 bytes" );
        UInt8 secret[ KeySize ];
        TCodecStatus status = X25519SharedSecret( privateKey, message.Data(), secret, error );
        if ( GUCEF_CODEC_STATUS_OK == status )
            status = WriteAll( output, secret, KeySize, error );
        OPENSSL_cleanse( secret, sizeof( secret ) );
        return status;
    }

    if ( SSLCODEC_ED25519 == codec->kind )
    {
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
        return Ed25519Verify( publicKey, signature, message.Data(), message.Size(), error );
    }

    SSslInstance* instance = GUCEF_NULL;
    TCodecStatus status = CreateSslInstance( codec->kind, params, instance, error );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;
    const UInt8* nonce = GUCEF_NULL;
    UInt32 nonceSize = 0;
    GetBinaryParam( params, "nonce", nonce, nonceSize );
    const UInt8* aad = GUCEF_NULL;
    UInt32 aadSize = 0;
    GetBinaryParam( params, "aad", aad, aadSize );

    CSecureBuffer result;
    UInt32 room = message.Size() > TagSize ? message.Size() - TagSize : 1;
    UInt32 written = 0;
    if ( !result.Reserve( room ) )
        status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    else
        status = ProcessSslInstance( instance, GUCEF_CODEC_PROCESS_DECODE_MESSAGE, message.Data(), message.Size(), nonce, nonceSize, aad, aadSize, result.Data(), room, written, error );
    DestroyInstance( instance );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;
    return WriteAll( output, result.Data(), written, error );
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
    { "nonce", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "12 bytes, never twice with the same key; required for one shot use, per call for instances" },
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
const SParamInfo hkdfParams[] =
{
    { "salt", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "A non secret random value, none is the same as 32 zero bytes" },
    { "info", GUCEF_DATATYPE_BINARY_BLOB, 0, GUCEF_NULL, "What the key is for, binds the key to its use" },
    { "length", GUCEF_DATATYPE_UINT32, 0, "32", "The size of the derived key in bytes, at most 8160" }
};
const SParamInfo hmacParams[] =
{
    { "key", GUCEF_DATATYPE_BINARY_BLOB, 1, GUCEF_NULL, "The secret key, of any length" }
};
const SParamInfo hashParams[] =
{
    { "output", GUCEF_DATATYPE_STRING, 0, "hex", "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes" }
};

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
SslGetParameterInfo( void* pluginData          ,
                     void* codecData           ,
                     UInt32 index              ,
                     TCodecParameterInfo* info ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SSslCodec* codec = static_cast< const SSslCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == info )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;

    const SParamInfo* list = GUCEF_NULL;
    UInt32 count = 0;
    switch ( codec->kind )
    {
        case SSLCODEC_AES256GCM :
        case SSLCODEC_CHACHA20_POLY1305 : { list = aeadParams; count = sizeof( aeadParams ) / sizeof( aeadParams[ 0 ] ); break; }
        case SSLCODEC_X25519 : { list = x25519Params; count = sizeof( x25519Params ) / sizeof( x25519Params[ 0 ] ); break; }
        case SSLCODEC_ED25519 : { list = ed25519Params; count = sizeof( ed25519Params ) / sizeof( ed25519Params[ 0 ] ); break; }
        case SSLCODEC_HKDF_SHA256 : { list = hkdfParams; count = sizeof( hkdfParams ) / sizeof( hkdfParams[ 0 ] ); break; }
        case SSLCODEC_HMAC_SHA256 : { list = hmacParams; count = sizeof( hmacParams ) / sizeof( hmacParams[ 0 ] ); break; }
        case SSLCODEC_SHA256 :
        case SSLCODEC_SHA512 : { list = hashParams; count = sizeof( hashParams ) / sizeof( hashParams[ 0 ] ); break; }
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

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
SslCreateInstance( void* pluginData             ,
                   void* codecData              ,
                   const TVariantMapApi* params ,
                   void** instanceData          ,
                   TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SSslCodec* codec = static_cast< const SSslCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == instanceData )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec" );

    SSslInstance* instance = GUCEF_NULL;
    TCodecStatus status = CreateSslInstance( codec->kind, params, instance, error );
    if ( GUCEF_CODEC_STATUS_OK == status )
        *instanceData = instance;
    return status;
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
SslInstanceProcess( void* instanceData         ,
                    UInt32 flags               ,
                    const void* input          ,
                    UInt32 inputSize           ,
                    const TCodecCallArgs* args ,
                    void* output               ,
                    UInt32 outputCapacity      ,
                    UInt32* outputSize         ,
                    TCodecErrorInfo* error     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SSslInstance* instance = static_cast< SSslInstance* >( instanceData );
    if ( GUCEF_NULL == instance || GUCEF_NULL == outputSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No instance" );

    const UInt8* nonce = GUCEF_NULL;
    UInt32 nonceSize = 0;
    const UInt8* aad = GUCEF_NULL;
    UInt32 aadSize = 0;
    if ( GUCEF_NULL != args && args->structSize >= sizeof( TCodecCallArgs ) )
    {
        nonce = static_cast< const UInt8* >( args->nonce );
        nonceSize = args->nonceSize;
        aad = static_cast< const UInt8* >( args->aad );
        aadSize = GUCEF_NULL != aad ? args->aadSize : 0;
    }
    return ProcessSslInstance( instance, flags, static_cast< const UInt8* >( input ), inputSize, nonce, nonceSize, aad, aadSize,
                               static_cast< UInt8* >( output ), outputCapacity, *outputSize, error );
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
SslDestroyInstance( void* instanceData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    DestroyInstance( static_cast< SSslInstance* >( instanceData ) );
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
SslGetOutputSizeBound( void* pluginData   ,
                       void* codecData    ,
                       void* instanceData ,
                       UInt32 flags       ,
                       UInt32 inputSize   ,
                       UInt32* bound      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SSslCodec* codec = static_cast< const SSslCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == bound )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;

    const bool final = 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL );
    if ( IsAead( codec->kind ) )
    {
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE ) )
            *bound = inputSize <= GUCEF_UINT32MAX - TagSize ? inputSize + TagSize : GUCEF_UINT32MAX;
        else
            *bound = inputSize > TagSize ? inputSize - TagSize : 0;
        return GUCEF_CODEC_STATUS_OK;
    }
    if ( IsHash( codec->kind ) )
    {
        const SSslInstance* instance = static_cast< const SSslInstance* >( instanceData );
        UInt32 size = 128;
        if ( GUCEF_NULL != instance )
            size = instance->rawOutput ? instance->digestSize : 2 * instance->digestSize;
        *bound = final ? size : 0;
        return GUCEF_CODEC_STATUS_OK;
    }
    if ( SSLCODEC_X25519 == codec->kind ) { *bound = KeySize; return GUCEF_CODEC_STATUS_OK; }
    if ( SSLCODEC_ED25519 == codec->kind ) { *bound = SignatureSize; return GUCEF_CODEC_STATUS_OK; }
    return GUCEF_CODEC_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------*/

#define SSL_LINK( family, type, codec, capabilities, overhead )                                                    \
    { sizeof( TCodecPluginLink ), family, type, const_cast< SSslCodec* >( &codec ), capabilities, 0, overhead,    \
      SslEncode, SslDecode, SslGetParameterInfo, SslCreateInstance, SslInstanceProcess, SslDestroyInstance,       \
      SslGetOutputSizeBound }

#define SSL_AEAD_CAPABILITIES       ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC | GUCEF_CODEC_CAPABILITY_MESSAGE_ONLY )
#define SSL_HASH_CAPABILITIES       ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
#define SSL_KEYS_CAPABILITIES       ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
#define SSL_KDF_CAPABILITIES        ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )

const TCodecPluginLink allCodecLinks[] =
{
    SSL_LINK( "EncryptionCodec", "aes-256-gcm", aesGcmCodec, SSL_AEAD_CAPABILITIES, TagSize ),
    SSL_LINK( "EncryptionCodec", "chacha20-poly1305", chachaCodec, SSL_AEAD_CAPABILITIES, TagSize ),
    SSL_LINK( "KeyAgreementCodec", "x25519", x25519Codec, SSL_KEYS_CAPABILITIES, 0 ),
    SSL_LINK( "SignatureCodec", "ed25519", ed25519Codec, SSL_KEYS_CAPABILITIES, 0 ),
    SSL_LINK( "KeyDerivationCodec", "hkdf-sha256", hkdfCodec, SSL_KDF_CAPABILITIES, 0 ),
    SSL_LINK( "MacCodec", "hmac-sha256", hmacCodec, SSL_HASH_CAPABILITIES, 0 ),
    SSL_LINK( "CryptographicHashCodec", "sha256", sha256Codec, SSL_HASH_CAPABILITIES, 0 ),
    SSL_LINK( "CryptographicHashCodec", "sha512", sha512Codec, SSL_HASH_CAPABILITIES, 0 )
};
const UInt32 allCodecLinkCount = (UInt32) ( sizeof( allCodecLinks ) / sizeof( allCodecLinks[ 0 ] ) );

/** The links of what the library offers (a FIPS build may leave algorithms out), set at Init */
const TCodecPluginLink* availableCodecLinks[ sizeof( allCodecLinks ) / sizeof( allCodecLinks[ 0 ] ) ];
UInt32 availableCodecLinkCount = 0;

/*-------------------------------------------------------------------------*/

bool
IsAvailable( const SSslCodec& codec )
{
    bool available = false;
    switch ( codec.kind )
    {
        case SSLCODEC_AES256GCM :
        case SSLCODEC_CHACHA20_POLY1305 :
        {
            EVP_CIPHER* cipher = EVP_CIPHER_fetch( GUCEF_NULL, codec.algorithmName, GUCEF_NULL );
            available = GUCEF_NULL != cipher;
            EVP_CIPHER_free( cipher );
            break;
        }
        case SSLCODEC_X25519 :
        case SSLCODEC_ED25519 :
        {
            EVP_PKEY_CTX* context = EVP_PKEY_CTX_new_from_name( GUCEF_NULL, codec.algorithmName, GUCEF_NULL );
            available = GUCEF_NULL != context;
            EVP_PKEY_CTX_free( context );
            break;
        }
        case SSLCODEC_HKDF_SHA256 :
        {
            EVP_KDF* kdf = EVP_KDF_fetch( GUCEF_NULL, codec.algorithmName, GUCEF_NULL );
            available = GUCEF_NULL != kdf;
            EVP_KDF_free( kdf );
            break;
        }
        case SSLCODEC_HMAC_SHA256 :
        {
            EVP_MAC* mac = EVP_MAC_fetch( GUCEF_NULL, codec.algorithmName, GUCEF_NULL );
            available = GUCEF_NULL != mac;
            EVP_MAC_free( mac );
            break;
        }
        case SSLCODEC_SHA256 :
        case SSLCODEC_SHA512 :
        {
            EVP_MD* md = EVP_MD_fetch( GUCEF_NULL, codec.algorithmName, GUCEF_NULL );
            available = GUCEF_NULL != md;
            EVP_MD_free( md );
            break;
        }
    }
    return available;
}

/*-------------------------------------------------------------------------*/

}; /* anonymous namespace */

#endif /* GUCEF_CODECSPLUGIN_OPENSSL_AVAILABLE ? */

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

    #ifdef GUCEF_CODECSPLUGIN_OPENSSL_AVAILABLE
    availableCodecLinkCount = 0;
    for ( UInt32 i=0; i<allCodecLinkCount; ++i )
    {
        if ( IsAvailable( *static_cast< const SSslCodec* >( allCodecLinks[ i ].codecData ) ) )
            availableCodecLinks[ availableCodecLinkCount++ ] = &allCodecLinks[ i ];
        else
        if ( GUCEF_NULL != coreApi && GUCEF_NULL != coreApi->Log )
            coreApi->Log( GUCEF_LOGMSGTYPE_STANDARD, GUCEF_LOGLEVEL_NORMAL, "codecspluginOPENSSL: the OpenSSL library does not offer one of the algorithms, its codec is left out" );
    }
    #else
    if ( GUCEF_NULL != coreApi && GUCEF_NULL != coreApi->Log )
        coreApi->Log( GUCEF_LOGMSGTYPE_STANDARD, GUCEF_LOGLEVEL_NORMAL, "codecspluginOPENSSL: built without the OpenSSL 3 development headers, it offers no codecs" );
    #endif
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
    return "codecspluginOPENSSL: AES-256-GCM, ChaCha20-Poly1305, X25519, Ed25519, HKDF-SHA256, HMAC-SHA256, SHA-256 and SHA-512 codecs using the OpenSSL libcrypto of the system";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Plugin Copyright (C) Dinand Vanvelzen. 1998 - 2026.  Licensed under the Apache License, Version 2.0. OpenSSL is licensed under the Apache License, Version 2.0";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;

    version.major = CODECSPLUGIN_OPENSSL_MAYOR_VERSION;
    version.minor = CODECSPLUGIN_OPENSSL_MINOR_VERSION;
    version.patch = CODECSPLUGIN_OPENSSL_PATCH_VERSION;
    version.release = CODECSPLUGIN_OPENSSL_RELEASE_VERSION;

    return version;
}

/*---------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecCount( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    #ifdef GUCEF_CODECSPLUGIN_OPENSSL_AVAILABLE
    return availableCodecLinkCount;
    #else
    return 0;
    #endif
}

/*---------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_GetCodecLink( void* plugdata                     ,
                          UInt32 index                       ,
                          const TCodecPluginLink** codecLink ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( GUCEF_NULL == codecLink )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;
    *codecLink = GUCEF_NULL;
    #ifdef GUCEF_CODECSPLUGIN_OPENSSL_AVAILABLE
    if ( index < availableCodecLinkCount )
    {
        *codecLink = availableCodecLinks[ index ];
        return GUCEF_CODEC_STATUS_OK;
    }
    #endif
    return GUCEF_CODEC_STATUS_UNSUPPORTED;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace OPENSSL */
}; /* namespace CODECSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
