/*
 *  codecspluginSTBRUMMEHASH: Codec plugin for RC32/MD5/SHA1/SHA3/SHA256/Keccak
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

#include <string.h>
#include <stdlib.h>

#include "codecspluginSTBRUMMEHASH.h"     // function prototypes for this plugin

#include "crc32.h"
#include "sha1.h"
#include "sha3.h"
#include "sha256.h"
#include "keccak.h"
#include "md5.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CODECSPLUGIN {
namespace STBRUMMEHASH {

using namespace GUCEF::CORE;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define CODECSPLUGIN_STBRUMMEHASH_MAYOR_VERSION       0
#define CODECSPLUGIN_STBRUMMEHASH_MINOR_VERSION       2
#define CODECSPLUGIN_STBRUMMEHASH_PATCH_VERSION       0
#define CODECSPLUGIN_STBRUMMEHASH_RELEASE_VERSION     0

#define TEMP_READ_HASH_BUFFER_SIZE                (1024)

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A hash of the library in progress, the library classes have no common base */
class CIHasher
{
    public:

    virtual ~CIHasher() {}

    virtual void Reset( void ) = 0;

    virtual void Add( const void* data, size_t size ) = 0;

    /** The digest of what was added so far as lower case hexadecimal text */
    virtual std::string GetHexHash( void ) = 0;
};

/*-------------------------------------------------------------------------*/

template < class THashAlgo >
class CTHasher : public CIHasher
{
    public:

    virtual void Reset( void ) { m_hash.reset(); }

    virtual void Add( const void* data, size_t size ) { m_hash.add( data, size ); }

    virtual std::string GetHexHash( void ) { return m_hash.getHash(); }

    private:

    THashAlgo m_hash;
};

/*-------------------------------------------------------------------------*/

typedef CIHasher* ( *TCreateHasher )( void );

template < class THashAlgo >
CIHasher* CreateHasher( void )
{
    return new CTHasher< THashAlgo >();
}

/*-------------------------------------------------------------------------*/

/** What a codec of this plugin is, the codecData of its link */
struct SStbHashCodec
{
    TCreateHasher createHasher;
    UInt32 digestSize;          /**< in bytes */
    bool rawByDefault;          /**< the "-raw" names */
};

/*-------------------------------------------------------------------------*/

/** A hash in progress for an instance */
struct SStbHashInstance
{
    const SStbHashCodec* codec;
    CIHasher* hasher;
    bool rawOutput;
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

/** The "output" parameter: raw digest bytes or hexadecimal text. INVALID_PARAMETER for anything but "raw" and "hex" */
TCodecStatus
GetRawOutputParam( const TVariantMapApi* params, bool defaultRaw, bool& rawOutput, TCodecErrorInfo* error )
{
    rawOutput = defaultRaw;
    if ( GUCEF_NULL == params || GUCEF_NULL == params->at_key )
        return GUCEF_CODEC_STATUS_OK;

    TVariantData key;
    memset( &key, 0, sizeof( key ) );
    key.containedType = GUCEF_DATATYPE_ASCII_STRING;
    key.union_data.heap_data.union_data.const_char_heap_data = "output";
    key.union_data.heap_data.heap_data_size = 7;
    key.union_data.heap_data.heap_data_is_linked = 1;

    TVariantData value;
    memset( &value, 0, sizeof( value ) );
    params->at_key( params->privateData, &key, &value );
    if ( GUCEF_DATATYPE_UNKNOWN == value.containedType )
        return GUCEF_CODEC_STATUS_OK;

    if ( GUCEF_DATATYPE_ASCII_STRING == value.containedType || GUCEF_DATATYPE_UTF8_STRING == value.containedType )
    {
        const char* text = value.union_data.heap_data.union_data.const_char_heap_data;
        UInt32 size = value.union_data.heap_data.heap_data_size;
        if ( GUCEF_NULL != text && size > 0 && '\0' == text[ size-1 ] )
            --size;
        if ( GUCEF_NULL != text && 3 == size && 0 == strncmp( text, "raw", 3 ) )
        {
            rawOutput = true;
            return GUCEF_CODEC_STATUS_OK;
        }
        if ( GUCEF_NULL != text && 3 == size && 0 == strncmp( text, "hex", 3 ) )
        {
            rawOutput = false;
            return GUCEF_CODEC_STATUS_OK;
        }
    }
    return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"output\" must be \"hex\" or \"raw\"" );
}

/*-------------------------------------------------------------------------*/

/**
 *  Converts a hex string as produced by the hash library into the binary bytes it represents
 */
bool
HexStringToBytes( const std::string& hexStr, UInt8* bytes, UInt32 byteCapacity )
{
    if ( 0 != ( hexStr.size() % 2 ) || hexStr.size() / 2 > byteCapacity )
        return false;

    for ( size_t i=0; i<hexStr.size(); i+=2 )
    {
        int byteValue = 0;
        for ( size_t n=i; n<i+2; ++n )
        {
            char c = hexStr[ n ];
            int nibble = 0;
            if ( c >= '0' && c <= '9' )
                nibble = c - '0';
            else
            if ( c >= 'a' && c <= 'f' )
                nibble = c - 'a' + 10;
            else
            if ( c >= 'A' && c <= 'F' )
                nibble = c - 'A' + 10;
            else
                return false;
            byteValue = ( byteValue << 4 ) | nibble;
        }
        bytes[ i / 2 ] = (UInt8) byteValue;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/** Writes the digest of the hasher, as text or as bytes, into memory with room for it */
bool
WriteDigest( CIHasher& hasher, bool rawOutput, UInt8* output, UInt32 outputCapacity, UInt32& outputSize )
{
    std::string hexHash = hasher.GetHexHash();
    if ( rawOutput )
    {
        if ( !HexStringToBytes( hexHash, output, outputCapacity ) )
            return false;
        outputSize = (UInt32) hexHash.size() / 2;
        return true;
    }
    if ( hexHash.size() > outputCapacity )
        return false;
    memcpy( output, hexHash.c_str(), hexHash.size() );
    outputSize = (UInt32) hexHash.size();
    return true;
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
HashEncode( void* pluginData             ,
            void* codecData              ,
            const TVariantMapApi* params ,
            TIOAccess* input             ,
            TIOAccess* output            ,
            TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SStbHashCodec* codec = static_cast< const SStbHashCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );

    bool rawOutput = codec->rawByDefault;
    TCodecStatus status = GetRawOutputParam( params, codec->rawByDefault, rawOutput, error );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;

    CIHasher* hasher = codec->createHasher();
    if ( GUCEF_NULL == hasher )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );

    // hash the input in chunks
    UInt8 tempInputBuffer[ TEMP_READ_HASH_BUFFER_SIZE ];
    while ( 0 == input->eof( input ) )
    {
        UInt32 inputBytesRead = input->read( input, tempInputBuffer, 1, TEMP_READ_HASH_BUFFER_SIZE );
        if ( 0 == inputBytesRead )
            break;
        hasher->Add( tempInputBuffer, inputBytesRead );
    }

    UInt8 digest[ 256 ];
    UInt32 digestSize = 0;
    bool written = WriteDigest( *hasher, rawOutput, digest, sizeof( digest ), digestSize );
    delete hasher;
    if ( !written )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to make the digest" );

    if ( 1 == output->write( output, digest, digestSize, 1 ) )
        return GUCEF_CODEC_STATUS_OK;
    return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
HashDecode( void* pluginData             ,
            void* codecData              ,
            const TVariantMapApi* params ,
            TIOAccess* input             ,
            TIOAccess* output            ,
            TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    // A crypto hash is a one-way hash, no such thing as a decode (we hope)
    // Note that MD5 has known collisions
    return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "A hash cannot be decoded" );
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
HashGetParameterInfo( void* pluginData          ,
                      void* codecData           ,
                      UInt32 index              ,
                      TCodecParameterInfo* info ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SStbHashCodec* codec = static_cast< const SStbHashCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == info || 0 != index )
        return GUCEF_CODEC_STATUS_UNSUPPORTED;

    info->name = "output";
    info->typeId = GUCEF_DATATYPE_STRING;
    info->required = 0;
    info->defaultValue = codec->rawByDefault ? "raw" : "hex";
    info->description = "\"hex\" for the digest as lower case hexadecimal text, \"raw\" for the digest bytes";
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
HashCreateInstance( void* pluginData             ,
                    void* codecData              ,
                    const TVariantMapApi* params ,
                    void** instanceData          ,
                    TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SStbHashCodec* codec = static_cast< const SStbHashCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == instanceData )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec" );

    bool rawOutput = codec->rawByDefault;
    TCodecStatus status = GetRawOutputParam( params, codec->rawByDefault, rawOutput, error );
    if ( GUCEF_CODEC_STATUS_OK != status )
        return status;

    SStbHashInstance* instance = new SStbHashInstance();
    instance->codec = codec;
    instance->hasher = codec->createHasher();
    instance->rawOutput = rawOutput;
    *instanceData = instance;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
HashInstanceProcess( void* instanceData         ,
                     UInt32 flags               ,
                     const void* input          ,
                     UInt32 inputSize           ,
                     const TCodecCallArgs* args ,
                     void* output               ,
                     UInt32 outputCapacity      ,
                     UInt32* outputSize         ,
                     TCodecErrorInfo* error     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SStbHashInstance* instance = static_cast< SStbHashInstance* >( instanceData );
    if ( GUCEF_NULL == instance || GUCEF_NULL == instance->hasher || GUCEF_NULL == outputSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No instance" );
    *outputSize = 0;
    if ( 0 != ( flags & GUCEF_CODEC_PROCESS_DECODE ) || 0 == ( flags & GUCEF_CODEC_PROCESS_ENCODE ) )
        return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "A hash cannot be decoded" );

    const UInt32 digestOutputSize = instance->rawOutput ? instance->codec->digestSize : 2 * instance->codec->digestSize;
    if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) && outputCapacity < digestOutputSize )
        return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output has no room for the digest" );

    if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
        instance->hasher->Reset();
    if ( GUCEF_NULL != input && inputSize > 0 )
        instance->hasher->Add( input, inputSize );
    if ( 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) )
    {
        bool written = WriteDigest( *instance->hasher, instance->rawOutput, static_cast< UInt8* >( output ), outputCapacity, *outputSize );
        instance->hasher->Reset();
        if ( !written )
            return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to make the digest" );
    }
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
HashDestroyInstance( void* instanceData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    SStbHashInstance* instance = static_cast< SStbHashInstance* >( instanceData );
    if ( GUCEF_NULL != instance )
    {
        delete instance->hasher;
        delete instance;
    }
}

/*-------------------------------------------------------------------------*/

TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
HashGetOutputSizeBound( void* pluginData   ,
                        void* codecData    ,
                        void* instanceData ,
                        UInt32 flags       ,
                        UInt32 inputSize   ,
                        UInt32* bound      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const SStbHashCodec* codec = static_cast< const SStbHashCodec* >( codecData );
    if ( GUCEF_NULL == codec || GUCEF_NULL == bound )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;

    // without an instance the output form is not known: room for the text, the larger of the two
    const SStbHashInstance* instance = static_cast< const SStbHashInstance* >( instanceData );
    UInt32 digestOutputSize = ( GUCEF_NULL != instance && instance->rawOutput ) ? codec->digestSize : 2 * codec->digestSize;
    *bound = 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) ? digestOutputSize : 0;
    return GUCEF_CODEC_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

const SStbHashCodec md5Codec = { CreateHasher< MD5 >, 16, false };
const SStbHashCodec sha1Codec = { CreateHasher< SHA1 >, 20, false };
const SStbHashCodec sha3Codec = { CreateHasher< SHA3 >, 32, false };
const SStbHashCodec sha256Codec = { CreateHasher< SHA256 >, 32, false };
const SStbHashCodec keccakCodec = { CreateHasher< Keccak >, 32, false };
const SStbHashCodec md5RawCodec = { CreateHasher< MD5 >, 16, true };
const SStbHashCodec sha1RawCodec = { CreateHasher< SHA1 >, 20, true };
const SStbHashCodec sha3RawCodec = { CreateHasher< SHA3 >, 32, true };
const SStbHashCodec sha256RawCodec = { CreateHasher< SHA256 >, 32, true };
const SStbHashCodec keccakRawCodec = { CreateHasher< Keccak >, 32, true };
const SStbHashCodec crc32Codec = { CreateHasher< CRC32 >, 4, false };

#define STB_HASH_CAPABILITIES   ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )

#define STB_HASH_LINK( family, type, codec )                                                                           \
    { sizeof( TCodecPluginLink ), family, type, const_cast< SStbHashCodec* >( &codec ), STB_HASH_CAPABILITIES, 0, 0,    \
      HashEncode, HashDecode, HashGetParameterInfo, HashCreateInstance, HashInstanceProcess, HashDestroyInstance,      \
      HashGetOutputSizeBound }

// The "-raw" names give the binary digest by default, as "output=raw" does for the others
const TCodecPluginLink codecLinks[] =
{
    STB_HASH_LINK( "CryptographicHashCodec", "md5", md5Codec ),
    STB_HASH_LINK( "CryptographicHashCodec", "sha1", sha1Codec ),
    STB_HASH_LINK( "CryptographicHashCodec", "sha3", sha3Codec ),
    STB_HASH_LINK( "CryptographicHashCodec", "sha256", sha256Codec ),
    STB_HASH_LINK( "CryptographicHashCodec", "keccak", keccakCodec ),
    STB_HASH_LINK( "CryptographicHashCodec", "md5-raw", md5RawCodec ),
    STB_HASH_LINK( "CryptographicHashCodec", "sha1-raw", sha1RawCodec ),
    STB_HASH_LINK( "CryptographicHashCodec", "sha3-raw", sha3RawCodec ),
    STB_HASH_LINK( "CryptographicHashCodec", "sha256-raw", sha256RawCodec ),
    STB_HASH_LINK( "CryptographicHashCodec", "keccak-raw", keccakRawCodec ),
    STB_HASH_LINK( "ChecksumCodec", "crc32", crc32Codec )
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
    return "codecspluginSTBRUMMEHASH: Codec plugin for CRC32/MD5/SHA1/SHA3/SHA256/Keccak";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Plugin Copyright (C) Dinand Vanvelzen. 1998 - 2026.  Licensed under the Apache License, Version 2.0. STBRUMME's Hash Library dependency is licensed under ZLIB\0";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;

    version.major = CODECSPLUGIN_STBRUMMEHASH_MAYOR_VERSION;
    version.minor = CODECSPLUGIN_STBRUMMEHASH_MINOR_VERSION;
    version.patch = CODECSPLUGIN_STBRUMMEHASH_PATCH_VERSION;
    version.release = CODECSPLUGIN_STBRUMMEHASH_RELEASE_VERSION;

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

}; /* namespace STBRUMMEHASH */
}; /* namespace CODECSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
