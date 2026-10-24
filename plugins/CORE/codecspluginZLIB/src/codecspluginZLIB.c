/*
 *  codecspluginZLIB: Codec plugin for zlib/gzip/deflate/crc32/adler32
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

#include <stdlib.h>               /* for memory utils */
#include <string.h>               /* for memset */
#include <stdio.h>                /* for snprintf */

#include "zlib.h"

#define GUCEF_DONT_TYPEDEF_BYTE
#include "codecspluginZLIB.h"     /* function prototypes for this plugin */

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

#define CODECSPLUGIN_ZLIB_MAYOR_VERSION       0
#define CODECSPLUGIN_ZLIB_MINOR_VERSION       2
#define CODECSPLUGIN_ZLIB_PATCH_VERSION       0
#define CODECSPLUGIN_ZLIB_RELEASE_VERSION     0

#define TEMP_READ_CHECKSUM_BUFFER_SIZE                (1024)
#define TEMP_READ_BUFFER_SIZE                         (128 * 1024)
#define TEMP_INPROCESS_BUFFER_SIZE                    (128 * 1024)

/* The compression level when none is given: the smallest output, as this plugin always did */
#define DEFAULT_COMPRESSION_LEVEL                     Z_BEST_COMPRESSION

/* zlib window bits: the size of the window plus what wraps the deflate data */
#define WINDOW_BITS_RAW_DEFLATE                       (-15)
#define WINDOW_BITS_ZLIB_WRAPPER                      (15)
#define WINDOW_BITS_GZIP_WRAPPER                      (31)
#define WINDOW_BITS_GZIP_OR_ZLIB_AUTO                 (47)     /* decoding: gzip or zlib, whichever it is */

/*---------------------------------------------------------------------------*/

/* What a codec of this plugin does, the codecData of its link */
typedef enum EZlibCodecKind
{
    ZLIBCODEC_COMPRESSION = 0,
    ZLIBCODEC_CRC32,
    ZLIBCODEC_ADLER32
} TZlibCodecKind;

typedef struct SZlibCodec
{
    TZlibCodecKind kind;
    int encodeWindowBits;       /* for compression */
    int decodeWindowBits;       /* for decompression */
} TZlibCodec;

static TZlibCodec zlibCodec      = { ZLIBCODEC_COMPRESSION, WINDOW_BITS_ZLIB_WRAPPER, WINDOW_BITS_GZIP_OR_ZLIB_AUTO };
static TZlibCodec deflateCodec   = { ZLIBCODEC_COMPRESSION, WINDOW_BITS_RAW_DEFLATE, WINDOW_BITS_RAW_DEFLATE };
static TZlibCodec gzipCodec      = { ZLIBCODEC_COMPRESSION, WINDOW_BITS_GZIP_WRAPPER, WINDOW_BITS_GZIP_OR_ZLIB_AUTO };
static TZlibCodec crc32Codec     = { ZLIBCODEC_CRC32, 0, 0 };
static TZlibCodec adler32Codec   = { ZLIBCODEC_ADLER32, 0, 0 };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static TCodecStatus
SetError( TCodecErrorInfo* error, TCodecStatus status, const char* message )
{
    if ( GUCEF_NULL != error && GUCEF_NULL != message )
    {
        strncpy( error->message, message, GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 );
        error->message[ GUCEF_CODEC_ERRORINFO_MESSAGE_SIZE-1 ] = '\0';
    }
    return status;
}

/*---------------------------------------------------------------------------*/

/* Looks up a parameter, returns 1 when it is there */
static int
GetParam( const TVariantMapApi* params, const char* name, TVariantData* value )
{
    TVariantData key;

    if ( GUCEF_NULL == params || GUCEF_NULL == params->at_key )
        return 0;

    memset( &key, 0, sizeof( key ) );
    key.containedType = GUCEF_DATATYPE_ASCII_STRING;
    key.union_data.heap_data.union_data.const_char_heap_data = name;
    key.union_data.heap_data.heap_data_size = (UInt32) strlen( name ) + 1;
    key.union_data.heap_data.heap_data_is_linked = 1;

    memset( value, 0, sizeof( *value ) );
    params->at_key( params->privateData, &key, value );
    return GUCEF_DATATYPE_UNKNOWN != value->containedType ? 1 : 0;
}

/*---------------------------------------------------------------------------*/

/* A parameter as a whole number: native integers, or text that is a decimal number. Returns 1 when it is one */
static int
GetIntParam( const TVariantMapApi* params, const char* name, Int64* result, int* isPresent )
{
    TVariantData value;
    *isPresent = GetParam( params, name, &value );
    if ( 0 == *isPresent )
        return 0;

    switch ( value.containedType )
    {
        case GUCEF_DATATYPE_INT8 : { *result = value.union_data.int8_data; return 1; }
        case GUCEF_DATATYPE_UINT8 : { *result = value.union_data.uint8_data; return 1; }
        case GUCEF_DATATYPE_INT16 : { *result = value.union_data.int16_data; return 1; }
        case GUCEF_DATATYPE_UINT16 : { *result = value.union_data.uint16_data; return 1; }
        case GUCEF_DATATYPE_INT32 : { *result = value.union_data.int32_data; return 1; }
        case GUCEF_DATATYPE_UINT32 : { *result = value.union_data.uint32_data; return 1; }
        case GUCEF_DATATYPE_INT64 : { *result = value.union_data.int64_data; return 1; }
        case GUCEF_DATATYPE_UINT64 : { *result = (Int64) value.union_data.uint64_data; return 1; }
        case GUCEF_DATATYPE_ASCII_STRING :
        case GUCEF_DATATYPE_UTF8_STRING :
        {
            const char* text = value.union_data.heap_data.union_data.const_char_heap_data;
            UInt32 size = value.union_data.heap_data.heap_data_size;
            UInt32 i = 0;
            Int64 number = 0;
            int negative = 0;
            int digits = 0;

            if ( GUCEF_NULL == text )
                return 0;
            if ( i < size && '-' == text[ i ] ) { negative = 1; ++i; }
            for ( ; i < size && '\0' != text[ i ]; ++i )
            {
                if ( text[ i ] < '0' || text[ i ] > '9' || digits > 9 )
                    return 0;
                number = number * 10 + ( text[ i ] - '0' );
                ++digits;
            }
            if ( 0 == digits )
                return 0;
            *result = negative ? -number : number;
            return 1;
        }
        default :
            return 0;
    }
}

/*---------------------------------------------------------------------------*/

/* The compression level from the parameters: "level", -1 (the zlib default) or 0 to 9 */
static TCodecStatus
GetCompressionLevel( const TVariantMapApi* params, int* level, TCodecErrorInfo* error )
{
    Int64 value = 0;
    int isPresent = 0;

    *level = DEFAULT_COMPRESSION_LEVEL;
    if ( 0 == GetIntParam( params, "level", &value, &isPresent ) )
    {
        if ( 0 != isPresent )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"level\" must be a whole number" );
        return GUCEF_CODEC_STATUS_OK;
    }
    if ( value < Z_DEFAULT_COMPRESSION || value > Z_BEST_COMPRESSION )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "The parameter \"level\" must be -1 (the zlib default) or 0 (stored) to 9 (smallest)" );
    *level = (int) value;
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

/**
 *  Writes all of the given bytes to the output, also when the output only accepts part of the data per write call
 *  Returns 1 on success and 0 if the output stops accepting data
 */
static int
WriteAllToOutput( TIOAccess* output    ,
                  const UInt8* data    ,
                  UInt32 dataSize      )
{
    while ( dataSize > 0 )
    {
        UInt32 bytesWritten = output->write( output, data, 1, dataSize );
        if ( 0 == bytesWritten || bytesWritten > dataSize )
            return 0;
        data += bytesWritten;
        dataSize -= bytesWritten;
    }
    return 1;
}

/*---------------------------------------------------------------------------*/

static TCodecStatus
ZLibCompress( TIOAccess* input        ,
              TIOAccess* output       ,
              int windowBits          ,
              int level               ,
              TCodecErrorInfo* error  )
{
    int zlibResult = Z_OK;
    int flushMode = Z_NO_FLUSH;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    UInt8* tempInputBuffer = (UInt8*) malloc( TEMP_READ_BUFFER_SIZE );
    UInt8* tempOutputBuffer = (UInt8*) malloc( TEMP_INPROCESS_BUFFER_SIZE );
    z_stream strm;

    if ( GUCEF_NULL == tempInputBuffer || GUCEF_NULL == tempOutputBuffer )
    {
        free( tempInputBuffer );
        free( tempOutputBuffer );
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    }

    memset( &strm, 0, sizeof( strm ) );
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    zlibResult = deflateInit2( &strm, level, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY );
    if ( Z_OK != zlibResult )
    {
        free( tempInputBuffer );
        free( tempOutputBuffer );
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "zlib refused to start compressing" );
    }

    /* read blocks of data from the input, encode them and write them to the output */
    /* once the input is exhausted we finish the stream which also writes any trailer info */
    do
    {
        strm.avail_in = input->read( input, tempInputBuffer, 1, TEMP_READ_BUFFER_SIZE );
        strm.next_in = tempInputBuffer;
        flushMode = ( 0 == strm.avail_in ) ? Z_FINISH : Z_NO_FLUSH;

        /* keep deflating until deflate no longer fills up the entire output buffer */
        do
        {
            strm.next_out = tempOutputBuffer;
            strm.avail_out = TEMP_INPROCESS_BUFFER_SIZE;

            zlibResult = deflate( &strm, flushMode );
            if ( Z_STREAM_ERROR == zlibResult )
            {
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "zlib failed to compress" );
                break;
            }

            if ( 0 == WriteAllToOutput( output, tempOutputBuffer, TEMP_INPROCESS_BUFFER_SIZE - strm.avail_out ) )
            {
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
                break;
            }
        }
        while ( 0 == strm.avail_out );
    }
    while ( GUCEF_CODEC_STATUS_OK == status && Z_FINISH != flushMode );

    if ( GUCEF_CODEC_STATUS_OK == status && Z_STREAM_END != zlibResult )
        status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "zlib did not finish the stream" );

    deflateEnd( &strm );
    free( tempInputBuffer );
    free( tempOutputBuffer );

    return status;
}

/*---------------------------------------------------------------------------*/

static TCodecStatus
ZLibDecompress( TIOAccess* input       ,
                TIOAccess* output      ,
                int windowBits         ,
                TCodecErrorInfo* error )
{
    int zlibResult = Z_OK;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;
    UInt8* tempInputBuffer = (UInt8*) malloc( TEMP_READ_BUFFER_SIZE );
    UInt8* tempOutputBuffer = (UInt8*) malloc( TEMP_INPROCESS_BUFFER_SIZE );
    z_stream strm;

    if ( GUCEF_NULL == tempInputBuffer || GUCEF_NULL == tempOutputBuffer )
    {
        free( tempInputBuffer );
        free( tempOutputBuffer );
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    }

    memset( &strm, 0, sizeof( strm ) );
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.next_in = Z_NULL;
    strm.avail_in = 0;

    zlibResult = inflateInit2( &strm, windowBits );
    if ( Z_OK != zlibResult )
    {
        free( tempInputBuffer );
        free( tempOutputBuffer );
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "zlib refused to start decompressing" );
    }

    /* read blocks of data from the input, decode them and write them to the output until the end of the stream */
    do
    {
        strm.avail_in = input->read( input, tempInputBuffer, 1, TEMP_READ_BUFFER_SIZE );
        strm.next_in = tempInputBuffer;
        if ( 0 == strm.avail_in )
        {
            /* the input ended before the end of the compressed stream */
            break;
        }

        /* keep inflating until inflate no longer fills up the entire output buffer */
        do
        {
            strm.next_out = tempOutputBuffer;
            strm.avail_out = TEMP_INPROCESS_BUFFER_SIZE;

            zlibResult = inflate( &strm, Z_NO_FLUSH );
            if ( Z_STREAM_ERROR == zlibResult || Z_NEED_DICT == zlibResult ||
                 Z_DATA_ERROR == zlibResult || Z_MEM_ERROR == zlibResult )
            {
                status = SetError( error, Z_MEM_ERROR == zlibResult ? GUCEF_CODEC_STATUS_FAILED : GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is not valid compressed data" );
                break;
            }

            if ( 0 == WriteAllToOutput( output, tempOutputBuffer, TEMP_INPROCESS_BUFFER_SIZE - strm.avail_out ) )
            {
                status = SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
                break;
            }
        }
        while ( 0 == strm.avail_out );
    }
    while ( GUCEF_CODEC_STATUS_OK == status && Z_STREAM_END != zlibResult );

    if ( GUCEF_CODEC_STATUS_OK == status && Z_STREAM_END != zlibResult )
        status = SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input ended before the end of the compressed data" );

    inflateEnd( &strm );
    free( tempInputBuffer );
    free( tempOutputBuffer );

    return status;
}

/*---------------------------------------------------------------------------*/

static TCodecStatus
GenerateChecksum( TIOAccess* input       ,
                  TIOAccess* output      ,
                  TZlibCodecKind kind    ,
                  TCodecErrorInfo* error )
{
    UInt32 inputBytesRead = 0;
    UInt8 tempInputBuffer[ TEMP_READ_CHECKSUM_BUFFER_SIZE ];
    UInt32 finalChecksum = 0;
    uLong checksum = ( ZLIBCODEC_CRC32 == kind ) ? crc32( 0L, Z_NULL, 0 ) : adler32( 0L, Z_NULL, 0 );

    /* checksum the input in chunks */
    while ( 0 == input->eof( input ) )
    {
        inputBytesRead = input->read( input, tempInputBuffer, 1, TEMP_READ_CHECKSUM_BUFFER_SIZE );
        if ( 0 == inputBytesRead )
            break;
        checksum = ( ZLIBCODEC_CRC32 == kind ) ? crc32( checksum, tempInputBuffer, inputBytesRead ) : adler32( checksum, tempInputBuffer, inputBytesRead );
    }

    /* produce the result checksum */
    finalChecksum = (UInt32) checksum;
    if ( 1 == output->write( output, &finalChecksum, sizeof(finalChecksum), 1 ) )
        return GUCEF_CODEC_STATUS_OK;
    return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Unable to write the output" );
}

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibEncode( void* pluginData             ,
            void* codecData              ,
            const TVariantMapApi* params ,
            TIOAccess* input             ,
            TIOAccess* output            ,
            TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TZlibCodec* codec = (const TZlibCodec*) codecData;
    int level = DEFAULT_COMPRESSION_LEVEL;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;

    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );

    if ( ZLIBCODEC_COMPRESSION == codec->kind )
    {
        status = GetCompressionLevel( params, &level, error );
        if ( GUCEF_CODEC_STATUS_OK != status )
            return status;
        return ZLibCompress( input, output, codec->encodeWindowBits, level, error );
    }
    return GenerateChecksum( input, output, codec->kind, error );
}

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibDecode( void* pluginData             ,
            void* codecData              ,
            const TVariantMapApi* params ,
            TIOAccess* input             ,
            TIOAccess* output            ,
            TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TZlibCodec* codec = (const TZlibCodec*) codecData;

    if ( GUCEF_NULL == codec || GUCEF_NULL == input || GUCEF_NULL == output )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec, input or output" );
    if ( ZLIBCODEC_COMPRESSION != codec->kind )
        return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "A checksum cannot be decoded" );
    return ZLibDecompress( input, output, codec->decodeWindowBits, error );
}

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibGetParameterInfo( void* pluginData          ,
                      void* codecData           ,
                      UInt32 index              ,
                      TCodecParameterInfo* info ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TZlibCodec* codec = (const TZlibCodec*) codecData;

    if ( GUCEF_NULL == codec || GUCEF_NULL == info || 0 != index || ZLIBCODEC_COMPRESSION != codec->kind )
        return GUCEF_CODEC_STATUS_UNSUPPORTED;

    info->name = "level";
    info->typeId = GUCEF_DATATYPE_INT32;
    info->required = 0;
    info->defaultValue = "9";
    info->description = "Compression level: 0 (stored) to 9 (smallest), -1 for the zlib default. Decompression ignores it";
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

/* A stream in progress: compression, decompression or a checksum */
typedef struct SZlibInstance
{
    const TZlibCodec* codec;
    int level;
    int streamMode;             /* 0: none yet, 1: deflate, 2: inflate */
    int streamEnded;            /* decompression: the end of the compressed data was seen */
    z_stream strm;
    uLong checksum;
    UInt8* pendingInput;        /* input that was taken in but not processed because the output was full */
    UInt32 pendingInputSize;
    UInt32 pendingInputOffset;
} TZlibInstance;

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibCreateInstance( void* pluginData             ,
                    void* codecData              ,
                    const TVariantMapApi* params ,
                    void** instanceData          ,
                    TCodecErrorInfo* error       ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TZlibCodec* codec = (const TZlibCodec*) codecData;
    TZlibInstance* instance = GUCEF_NULL;
    int level = DEFAULT_COMPRESSION_LEVEL;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;

    if ( GUCEF_NULL == codec || GUCEF_NULL == instanceData )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No codec" );

    if ( ZLIBCODEC_COMPRESSION == codec->kind )
    {
        status = GetCompressionLevel( params, &level, error );
        if ( GUCEF_CODEC_STATUS_OK != status )
            return status;
    }

    instance = (TZlibInstance*) malloc( sizeof( TZlibInstance ) );
    if ( GUCEF_NULL == instance )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    memset( instance, 0, sizeof( TZlibInstance ) );
    instance->codec = codec;
    instance->level = level;
    instance->checksum = ( ZLIBCODEC_CRC32 == codec->kind ) ? crc32( 0L, Z_NULL, 0 ) : adler32( 0L, Z_NULL, 0 );
    *instanceData = instance;
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

static void
ZlibEndStream( TZlibInstance* instance )
{
    if ( 1 == instance->streamMode )
        deflateEnd( &instance->strm );
    else
    if ( 2 == instance->streamMode )
        inflateEnd( &instance->strm );
    instance->streamMode = 0;
    instance->streamEnded = 0;
    instance->pendingInputSize = 0;
    instance->pendingInputOffset = 0;
}

/*---------------------------------------------------------------------------*/

static void GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibDestroyInstance( void* instanceData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TZlibInstance* instance = (TZlibInstance*) instanceData;
    if ( GUCEF_NULL == instance )
        return;
    ZlibEndStream( instance );
    free( instance->pendingInput );
    free( instance );
}

/*---------------------------------------------------------------------------*/

/* Keeps the input zlib did not take, the caller's buffer is only valid during the call */
static TCodecStatus
ZlibKeepPendingInput( TZlibInstance* instance, TCodecErrorInfo* error )
{
    UInt8* kept = GUCEF_NULL;
    UInt32 size = (UInt32) instance->strm.avail_in;

    if ( 0 == size )
        return GUCEF_CODEC_STATUS_OK;
    if ( instance->strm.next_in >= instance->pendingInput && instance->strm.next_in < instance->pendingInput + instance->pendingInputSize )
    {
        /* already ours */
        instance->pendingInputOffset = (UInt32) ( instance->strm.next_in - instance->pendingInput );
        return GUCEF_CODEC_STATUS_OK;
    }

    kept = (UInt8*) malloc( size );
    if ( GUCEF_NULL == kept )
        return SetError( error, GUCEF_CODEC_STATUS_FAILED, "Out of memory" );
    memcpy( kept, instance->strm.next_in, size );
    free( instance->pendingInput );
    instance->pendingInput = kept;
    instance->pendingInputSize = size;
    instance->pendingInputOffset = 0;
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibInstanceProcess( void* instanceData         ,
                     UInt32 flags               ,
                     const void* input          ,
                     UInt32 inputSize           ,
                     const TCodecCallArgs* args ,
                     void* output               ,
                     UInt32 outputCapacity      ,
                     UInt32* outputSize         ,
                     TCodecErrorInfo* error     ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TZlibInstance* instance = (TZlibInstance*) instanceData;
    int encode = 0 != ( flags & GUCEF_CODEC_PROCESS_ENCODE ) ? 1 : 0;
    int isFinal = 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) ? 1 : 0;
    int zlibResult = Z_OK;
    TCodecStatus status = GUCEF_CODEC_STATUS_OK;

    if ( GUCEF_NULL == instance || GUCEF_NULL == outputSize )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "No instance" );
    *outputSize = 0;
    if ( encode == ( 0 != ( flags & GUCEF_CODEC_PROCESS_DECODE ) ? 1 : 0 ) )
        return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "Exactly one of the encode and decode flags must be set" );

    /* checksums */
    if ( ZLIBCODEC_COMPRESSION != instance->codec->kind )
    {
        UInt32 finalChecksum = 0;
        if ( !encode )
            return SetError( error, GUCEF_CODEC_STATUS_UNSUPPORTED, "A checksum cannot be decoded" );
        if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) )
            instance->checksum = ( ZLIBCODEC_CRC32 == instance->codec->kind ) ? crc32( 0L, Z_NULL, 0 ) : adler32( 0L, Z_NULL, 0 );
        if ( isFinal && outputCapacity < sizeof( finalChecksum ) )
            return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "The output has no room for the checksum" );
        if ( GUCEF_NULL != input && inputSize > 0 )
            instance->checksum = ( ZLIBCODEC_CRC32 == instance->codec->kind ) ? crc32( instance->checksum, (const Bytef*) input, inputSize ) : adler32( instance->checksum, (const Bytef*) input, inputSize );
        if ( isFinal )
        {
            finalChecksum = (UInt32) instance->checksum;
            memcpy( output, &finalChecksum, sizeof( finalChecksum ) );
            *outputSize = sizeof( finalChecksum );
            instance->checksum = ( ZLIBCODEC_CRC32 == instance->codec->kind ) ? crc32( 0L, Z_NULL, 0 ) : adler32( 0L, Z_NULL, 0 );
        }
        return GUCEF_CODEC_STATUS_OK;
    }

    /* a new stream, or a stream of the other direction */
    if ( 0 != ( flags & GUCEF_CODEC_PROCESS_BEGIN ) || 0 == instance->streamMode || ( encode ? 1 : 2 ) != instance->streamMode )
    {
        ZlibEndStream( instance );
        memset( &instance->strm, 0, sizeof( instance->strm ) );
        if ( encode )
            zlibResult = deflateInit2( &instance->strm, instance->level, Z_DEFLATED, instance->codec->encodeWindowBits, 8, Z_DEFAULT_STRATEGY );
        else
            zlibResult = inflateInit2( &instance->strm, instance->codec->decodeWindowBits );
        if ( Z_OK != zlibResult )
            return SetError( error, GUCEF_CODEC_STATUS_FAILED, "zlib refused to start a stream" );
        instance->streamMode = encode ? 1 : 2;
    }

    /* input kept from a call that ran out of room comes first, then that of this call */
    if ( instance->pendingInputOffset < instance->pendingInputSize )
    {
        instance->strm.next_in = instance->pendingInput + instance->pendingInputOffset;
        instance->strm.avail_in = instance->pendingInputSize - instance->pendingInputOffset;
        if ( GUCEF_NULL != input && inputSize > 0 )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_PARAMETER, "Kept output must be collected (a call without input) before new input" );
    }
    else
    {
        instance->strm.next_in = (Bytef*) input;
        instance->strm.avail_in = GUCEF_NULL != input ? inputSize : 0;
    }
    instance->strm.next_out = (Bytef*) output;
    instance->strm.avail_out = outputCapacity;

    if ( encode )
    {
        int flushMode = isFinal ? Z_FINISH : Z_NO_FLUSH;
        do
        {
            zlibResult = deflate( &instance->strm, flushMode );
            if ( Z_STREAM_ERROR == zlibResult )
                return SetError( error, GUCEF_CODEC_STATUS_FAILED, "zlib failed to compress" );
        }
        while ( 0 != instance->strm.avail_out && ( 0 != instance->strm.avail_in || ( isFinal && Z_STREAM_END != zlibResult ) ) && Z_BUF_ERROR != zlibResult );

        *outputSize = outputCapacity - (UInt32) instance->strm.avail_out;
        if ( 0 != instance->strm.avail_in || ( isFinal && Z_STREAM_END != zlibResult ) )
        {
            status = ZlibKeepPendingInput( instance, error );
            if ( GUCEF_CODEC_STATUS_OK != status )
                return status;
            return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "More output follows" );
        }
        instance->pendingInputSize = 0;
        instance->pendingInputOffset = 0;
        if ( isFinal )
            ZlibEndStream( instance );
        return GUCEF_CODEC_STATUS_OK;
    }

    /* decompression */
    while ( !instance->streamEnded && 0 != instance->strm.avail_out )
    {
        zlibResult = inflate( &instance->strm, Z_NO_FLUSH );
        if ( Z_STREAM_END == zlibResult )
        {
            instance->streamEnded = 1;
            break;
        }
        if ( Z_STREAM_ERROR == zlibResult || Z_NEED_DICT == zlibResult || Z_DATA_ERROR == zlibResult || Z_MEM_ERROR == zlibResult )
        {
            ZlibEndStream( instance );
            return SetError( error, Z_MEM_ERROR == zlibResult ? GUCEF_CODEC_STATUS_FAILED : GUCEF_CODEC_STATUS_INVALID_INPUT, "The input is not valid compressed data" );
        }
        if ( Z_BUF_ERROR == zlibResult )
            break; /* no progress possible: all input used, nothing pending */
    }

    *outputSize = outputCapacity - (UInt32) instance->strm.avail_out;
    if ( 0 == instance->strm.avail_out && !instance->streamEnded )
    {
        /* the output is full: there may be more, from kept input or from what zlib holds */
        status = ZlibKeepPendingInput( instance, error );
        if ( GUCEF_CODEC_STATUS_OK != status )
            return status;
        return SetError( error, GUCEF_CODEC_STATUS_OUTPUT_TOO_SMALL, "More output follows" );
    }
    instance->pendingInputSize = 0;
    instance->pendingInputOffset = 0;
    if ( isFinal )
    {
        int ended = instance->streamEnded;
        ZlibEndStream( instance );
        if ( !ended )
            return SetError( error, GUCEF_CODEC_STATUS_INVALID_INPUT, "The input ended before the end of the compressed data" );
    }
    return GUCEF_CODEC_STATUS_OK;
}

/*---------------------------------------------------------------------------*/

static TCodecStatus GUCEF_PLUGIN_CALLSPEC_PREFIX
ZlibGetOutputSizeBound( void* pluginData   ,
                        void* codecData    ,
                        void* instanceData ,
                        UInt32 flags       ,
                        UInt32 inputSize   ,
                        UInt32* bound      ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    const TZlibCodec* codec = (const TZlibCodec*) codecData;
    if ( GUCEF_NULL == codec || GUCEF_NULL == bound )
        return GUCEF_CODEC_STATUS_INVALID_PARAMETER;

    if ( ZLIBCODEC_COMPRESSION != codec->kind )
    {
        *bound = 0 != ( flags & GUCEF_CODEC_PROCESS_FINAL ) ? sizeof( UInt32 ) : 0;
        return GUCEF_CODEC_STATUS_OK;
    }

    /* how much (de)compression gives depends on what came before, the host grows the room when asked */
    return GUCEF_CODEC_STATUS_UNSUPPORTED;
}

/*---------------------------------------------------------------------------*/

#define ZLIB_CODEC_LINK( family, type, codec, capabilities )                                           \
    { sizeof( TCodecPluginLink ), family, type, codec, capabilities, 0, 0,                          \
      ZlibEncode, ZlibDecode, ZlibGetParameterInfo, ZlibCreateInstance, ZlibInstanceProcess,         \
      ZlibDestroyInstance, ZlibGetOutputSizeBound }

#define ZLIB_COMPRESSION_CAPABILITIES   ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )
#define ZLIB_CHECKSUM_CAPABILITIES      ( GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_INSTANCES | GUCEF_CODEC_CAPABILITY_DETERMINISTIC )

static TCodecPluginLink codecLinks[] =
{
    ZLIB_CODEC_LINK( "CompressionCodec", "zlib", &zlibCodec, ZLIB_COMPRESSION_CAPABILITIES ),
    ZLIB_CODEC_LINK( "CompressionCodec", "deflate", &deflateCodec, ZLIB_COMPRESSION_CAPABILITIES ),
    ZLIB_CODEC_LINK( "CompressionCodec", "inflate", &deflateCodec, ZLIB_COMPRESSION_CAPABILITIES ),     /* the inverse name of the same raw deflate codec */
    ZLIB_CODEC_LINK( "CompressionCodec", "gzip", &gzipCodec, ZLIB_COMPRESSION_CAPABILITIES ),
    ZLIB_CODEC_LINK( "ChecksumCodec", "crc-32", &crc32Codec, ZLIB_CHECKSUM_CAPABILITIES ),
    ZLIB_CODEC_LINK( "ChecksumCodec", "adler-32", &adler32Codec, ZLIB_CHECKSUM_CAPABILITIES )
};
static const UInt32 codecLinkCount = (UInt32) ( sizeof( codecLinks ) / sizeof( codecLinks[ 0 ] ) );

/*---------------------------------------------------------------------------*/

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
    return "compressioncodecZLIB: zlib/deflate/gzip (de)compression and crc32/adler32 checksum codecs support using zlib\0";
}

/*---------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Copyright( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Plugin Copyright (C) Dinand Vanvelzen. 1998 - 2026.  Licensed under the Apache License, Version 2.0\0";
}

/*---------------------------------------------------------------------------*/

TVersion GUCEF_PLUGIN_CALLSPEC_PREFIX
CODECPLUGIN_Version( void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TVersion version;

    version.major = CODECSPLUGIN_ZLIB_MAYOR_VERSION;
    version.minor = CODECSPLUGIN_ZLIB_MINOR_VERSION;
    version.patch = CODECSPLUGIN_ZLIB_PATCH_VERSION;
    version.release = CODECSPLUGIN_ZLIB_RELEASE_VERSION;

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

/*-------------------------------------------------------------------------*/
