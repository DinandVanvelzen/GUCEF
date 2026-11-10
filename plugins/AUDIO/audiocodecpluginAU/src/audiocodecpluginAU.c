/*
 *  audiocodecpluginAU: gucefAUDIO codec plugin for Sun/NeXT .au audio
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

/*
 *  The .au format: a big endian header of six 32 bit words
 *
 *      magic ".snd", offset of the audio data, size of the audio data (0xFFFFFFFF = unknown),
 *      encoding, sample rate, channel count
 *
 *  followed by free text (the annotation) up to the data, then interleaved big endian samples.
 *  This plugin reads and writes the plain encodings and the two G.711 companded ones; it decodes mu-law and
 *  A-law to 16 bit linear. It is also the reference for writing a gucefAUDIO codec plugin in C: it uses only
 *  the plugin interface headers and links nothing from GUCEF.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#ifndef GUCEF_CORE_C_LOGGING_H
#include "gucefCORE_c_logging.h"
#define GUCEF_CORE_C_LOGGING_H
#endif /* GUCEF_CORE_C_LOGGING_H ? */

#include "audiocodecpluginAU.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define AU_MAGIC                0x2E736E64UL    /* ".snd" */
#define AU_HEADER_SIZE          24
#define AU_WRITE_DATA_OFFSET    28              /* the header plus the minimal 4 byte annotation */
#define AU_UNKNOWN_SIZE         0xFFFFFFFFUL
#define AU_MAX_ANNOTATION       4096            /* longer annotations are skipped, not read */

#define AU_ENCODING_MULAW       1
#define AU_ENCODING_LINEAR8     2
#define AU_ENCODING_LINEAR16    3
#define AU_ENCODING_LINEAR24    4
#define AU_ENCODING_LINEAR32    5
#define AU_ENCODING_FLOAT       6
#define AU_ENCODING_DOUBLE      7
#define AU_ENCODING_ALAW        27

#define AU_COPY_CHUNK_BYTES     16384

/* The two names of the format. A build of this source for another purpose than the plugin itself may give them other names */
#ifndef AU_CODEC_TYPE_PRIMARY
  #define AU_CODEC_TYPE_PRIMARY     "au"
#endif
#ifndef AU_CODEC_TYPE_SECONDARY
  #define AU_CODEC_TYPE_SECONDARY   "snd"
#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

typedef struct SAuPlugin
{
    TGucefCoreCApi libApi;
} TAuPlugin;

typedef struct SAuSource
{
    TIOAccess* input;
    UInt32 encoding;
    UInt32 channels;
    UInt32 bytesPerSampleIn;        /* in the file */
    UInt32 bytesPerSampleOut;       /* in the frames handed out */
    UInt64 dataOffset;
    UInt64 frameCount;              /* GUCEF_AUDIOCODEC_LENGTH_UNKNOWN when the file does not tell */
    UInt64 position;
} TAuSource;

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* g_codecTypes[] = { AU_CODEC_TYPE_PRIMARY, AU_CODEC_TYPE_SECONDARY };

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static int
IsHostBigEndian( void )
{
    const UInt16 probe = 0x0102;
    return 0x01 == *( (const UInt8*) &probe );
}

/*-------------------------------------------------------------------------*/

static UInt32
ReadBE32( const UInt8* bytes )
{
    return ( (UInt32) bytes[ 0 ] << 24 ) | ( (UInt32) bytes[ 1 ] << 16 ) | ( (UInt32) bytes[ 2 ] << 8 ) | (UInt32) bytes[ 3 ];
}

/*-------------------------------------------------------------------------*/

static void
WriteBE32( UInt8* bytes, UInt32 value )
{
    bytes[ 0 ] = (UInt8) ( value >> 24 );
    bytes[ 1 ] = (UInt8) ( value >> 16 );
    bytes[ 2 ] = (UInt8) ( value >> 8 );
    bytes[ 3 ] = (UInt8) value;
}

/*-------------------------------------------------------------------------*/

/* Reverses the byte order of each sample, which turns big endian into little endian and back */
static void
SwapSamples( UInt8* data, UInt32 sampleCount, UInt32 bytesPerSample )
{
    UInt32 i, n;
    for ( i=0; i<sampleCount; ++i )
    {
        UInt8* sample = data + (size_t) i * bytesPerSample;
        for ( n=0; n<bytesPerSample/2; ++n )
        {
            UInt8 b = sample[ n ];
            sample[ n ] = sample[ bytesPerSample-1-n ];
            sample[ bytesPerSample-1-n ] = b;
        }
    }
}

/*-------------------------------------------------------------------------*/

/* G.711 mu-law to 16 bit linear */
static Int16
MuLawToLinear( UInt8 code )
{
    Int32 magnitude;
    code = (UInt8) ~code;
    magnitude = ( ( (Int32) ( code & 0x0F ) << 3 ) + 0x84 ) << ( ( code & 0x70 ) >> 4 );
    return (Int16) ( ( code & 0x80 ) ? ( 0x84 - magnitude ) : ( magnitude - 0x84 ) );
}

/*-------------------------------------------------------------------------*/

/* G.711 A-law to 16 bit linear */
static Int16
ALawToLinear( UInt8 code )
{
    Int32 magnitude;
    Int32 segment;
    code ^= 0x55;
    magnitude = (Int32) ( code & 0x0F ) << 4;
    segment = ( code & 0x70 ) >> 4;
    if ( 0 == segment )
        magnitude += 8;
    else
    {
        magnitude += 0x108;
        if ( segment > 1 )
            magnitude <<= segment - 1;
    }
    return (Int16) ( ( code & 0x80 ) ? magnitude : -magnitude );
}

/*-------------------------------------------------------------------------*/

/* 16 bit linear to G.711 mu-law */
static UInt8
LinearToMuLaw( Int16 sample )
{
    Int32 value = sample >> 2;
    UInt8 mask = 0xFF;
    Int32 segment = 0;
    if ( value < 0 )
    {
        value = -value;
        mask = 0x7F;
    }
    if ( value > 8159 )
        value = 8159;
    value += 0x21;
    while ( segment < 8 && value > ( ( 0x40 << segment ) - 1 ) )
        ++segment;
    if ( segment >= 8 )
        return (UInt8) ( 0x7F ^ mask );
    return (UInt8) ( ( ( segment << 4 ) | ( ( value >> ( segment + 1 ) ) & 0x0F ) ) ^ mask );
}

/*-------------------------------------------------------------------------*/

/* 16 bit linear to G.711 A-law */
static UInt8
LinearToALaw( Int16 sample )
{
    Int32 value = sample >> 3;
    UInt8 mask = 0xD5;
    Int32 segment = 0;
    UInt8 code;
    if ( value < 0 )
    {
        value = -value - 1;
        mask = 0x55;
    }
    while ( segment < 8 && value > ( ( 0x20 << segment ) - 1 ) )
        ++segment;
    if ( segment >= 8 )
        return (UInt8) ( 0x7F ^ mask );
    code = (UInt8) ( segment << 4 );
    if ( segment < 2 )
        code |= (UInt8) ( ( value >> 1 ) & 0x0F );
    else
        code |= (UInt8) ( ( value >> segment ) & 0x0F );
    return (UInt8) ( code ^ mask );
}

/*-------------------------------------------------------------------------*/

static UInt32
BytesPerSampleOfEncoding( UInt32 encoding )
{
    switch ( encoding )
    {
        case AU_ENCODING_MULAW:
        case AU_ENCODING_ALAW:
        case AU_ENCODING_LINEAR8: return 1;
        case AU_ENCODING_LINEAR16: return 2;
        case AU_ENCODING_LINEAR24: return 3;
        case AU_ENCODING_LINEAR32:
        case AU_ENCODING_FLOAT: return 4;
        case AU_ENCODING_DOUBLE: return 8;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

static UInt32
SampleTypeOfEncoding( UInt32 encoding )
{
    switch ( encoding )
    {
        case AU_ENCODING_MULAW:
        case AU_ENCODING_ALAW:
        case AU_ENCODING_LINEAR16: return GUCEF_AUDIOCODEC_SAMPLETYPE_INT16;
        case AU_ENCODING_LINEAR8: return GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8;
        case AU_ENCODING_LINEAR24: return GUCEF_AUDIOCODEC_SAMPLETYPE_INT24;
        case AU_ENCODING_LINEAR32: return GUCEF_AUDIOCODEC_SAMPLETYPE_INT32;
        case AU_ENCODING_FLOAT: return GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32;
        case AU_ENCODING_DOUBLE: return GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64;
        default: return GUCEF_AUDIOCODEC_SAMPLETYPE_UNKNOWN;
    }
}

/*-------------------------------------------------------------------------*/

/* The plain encoding that stores a sample type as is */
static UInt32
EncodingOfSampleType( UInt32 sampleType )
{
    switch ( sampleType )
    {
        case GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8: return AU_ENCODING_LINEAR8;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT16: return AU_ENCODING_LINEAR16;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT24: return AU_ENCODING_LINEAR24;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT32: return AU_ENCODING_LINEAR32;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32: return AU_ENCODING_FLOAT;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64: return AU_ENCODING_DOUBLE;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

/* A string from a variant map, NULL when the key is absent or not text */
static const char*
GetTextOption( const TVariantMapApi* map, const char* key )
{
    TVariantData keyData;
    TVariantData valueData;

    if ( NULL == map || NULL == map->at_key )
        return NULL;

    memset( &keyData, 0, sizeof( keyData ) );
    keyData.containedType = GUCEF_DATATYPE_ASCII_STRING;
    keyData.union_data.heap_data.union_data.const_char_heap_data = key;
    keyData.union_data.heap_data.heap_data_size = (UInt32) strlen( key );
    keyData.union_data.heap_data.heap_data_is_linked = 1;

    memset( &valueData, 0, sizeof( valueData ) );
    map->at_key( map->privateData, &keyData, &valueData );
    if ( GUCEF_DATATYPE_ASCII_STRING != valueData.containedType && GUCEF_DATATYPE_UTF8_STRING != valueData.containedType )
        return NULL;
    return valueData.union_data.heap_data.union_data.const_char_heap_data;
}

/*-------------------------------------------------------------------------*/

static void
SetTextValue( TVariantMapApi* map, const char* key, const char* value )
{
    TVariantData keyData;
    TVariantData valueData;

    if ( NULL == map || NULL == map->insert )
        return;

    memset( &keyData, 0, sizeof( keyData ) );
    keyData.containedType = GUCEF_DATATYPE_ASCII_STRING;
    keyData.union_data.heap_data.union_data.const_char_heap_data = key;
    keyData.union_data.heap_data.heap_data_size = (UInt32) strlen( key );
    keyData.union_data.heap_data.heap_data_is_linked = 1;

    memset( &valueData, 0, sizeof( valueData ) );
    valueData.containedType = GUCEF_DATATYPE_UTF8_STRING;
    valueData.union_data.heap_data.union_data.const_char_heap_data = value;
    valueData.union_data.heap_data.heap_data_size = (UInt32) strlen( value );
    valueData.union_data.heap_data.heap_data_is_linked = 1;

    /* the host copies */
    map->insert( map->privateData, &keyData, &valueData, 0 );
}

/*-------------------------------------------------------------------------*/

/* Reads and checks the header, the input is left at the start of the annotation */
static int
ReadHeader( TIOAccess* input, UInt32* dataOffset, UInt32* dataSize, UInt32* encoding, UInt32* sampleRate, UInt32* channels )
{
    UInt8 header[ AU_HEADER_SIZE ];
    if ( NULL == input || NULL == input->read )
        return 0;
    if ( 1 != input->read( input, header, AU_HEADER_SIZE, 1 ) )
        return 0;
    if ( AU_MAGIC != ReadBE32( header ) )
        return 0;

    *dataOffset = ReadBE32( header + 4 );
    *dataSize = ReadBE32( header + 8 );
    *encoding = ReadBE32( header + 12 );
    *sampleRate = ReadBE32( header + 16 );
    *channels = ReadBE32( header + 20 );

    return *dataOffset >= AU_HEADER_SIZE && 0 != *sampleRate && 0 != *channels && *channels <= 64 &&
           0 != BytesPerSampleOfEncoding( *encoding );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      PLUGIN API                                                         //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Init )( void** plugdata, TGucefCoreCApi* libApi ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAuPlugin* plugin = (TAuPlugin*) calloc( 1, sizeof( TAuPlugin ) );
    if ( NULL == plugin )
        return 0;
    if ( NULL != libApi )
        plugin->libApi = *libApi;
    *plugdata = plugin;

    GUCEF_C_LOG( plugin->libApi, GUCEF_LOGLEVEL_BELOW_NORMAL, "audiocodecpluginAU: initialized" );
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Shutdown )( void** plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    free( *plugdata );
    *plugdata = NULL;
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Name )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "gucefAUDIO codec plugin for Sun/NeXT .au audio";
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Copyright )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return "Copyright (C) Dinand Vanvelzen. Licensed under the Apache License, Version 2.0";
}

/*-------------------------------------------------------------------------*/

const TVersion* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Version )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    static TVersion version;
    version.major = 0;
    version.minor = 1;
    version.patch = 0;
    version.release = 0;
    return &version;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type_Count )( const void* plugdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    return (UInt32) ( sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) );
}

/*-------------------------------------------------------------------------*/

const char* GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Type )( const void* plugdata, UInt32 index ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    if ( index >= sizeof( g_codecTypes ) / sizeof( g_codecTypes[ 0 ] ) )
        return NULL;
    return g_codecTypes[ index ];
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Create_Codec )( void** plugdata, const char* codecType, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    /* Both types are the same format, there is no per codec state */
    *codecdata = NULL;
    return NULL != codecType && ( 0 == strcmp( codecType, AU_CODEC_TYPE_PRIMARY ) || 0 == strcmp( codecType, AU_CODEC_TYPE_SECONDARY ) ) ? 1 : 0;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Destroy_Codec )( void** plugdata, void** codecdata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    *codecdata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Open )( void** plugdata, void** codecdata, void** sourcedata, TIOAccess* input, const TVariantMapApi* options, TAudioFormatInfo* format, UInt64* lengthInFrames ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAuPlugin* plugin = (TAuPlugin*) *plugdata;
    TAuSource* source;
    UInt64 start;
    UInt32 dataOffset, dataSize, encoding, sampleRate, channels;

    if ( NULL == input || NULL == input->tell || NULL == input->setpos )
        return 0;

    start = input->tell( input );
    if ( !ReadHeader( input, &dataOffset, &dataSize, &encoding, &sampleRate, &channels ) )
        return 0;
    if ( 0 != input->setpos( input, start + dataOffset ) )
    {
        GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginAU: cannot move to the audio data" );
        return 0;
    }

    source = (TAuSource*) calloc( 1, sizeof( TAuSource ) );
    if ( NULL == source )
        return 0;

    source->input = input;
    source->encoding = encoding;
    source->channels = channels;
    source->bytesPerSampleIn = BytesPerSampleOfEncoding( encoding );
    source->bytesPerSampleOut = ( AU_ENCODING_MULAW == encoding || AU_ENCODING_ALAW == encoding ) ? 2 : source->bytesPerSampleIn;
    source->dataOffset = start + dataOffset;
    source->frameCount = AU_UNKNOWN_SIZE == dataSize ? GUCEF_AUDIOCODEC_LENGTH_UNKNOWN : dataSize / ( source->bytesPerSampleIn * channels );
    source->position = 0;

    format->sampleRate = sampleRate;
    format->channelCount = channels;
    format->sampleType = SampleTypeOfEncoding( encoding );
    format->channelLayout = 0;
    *lengthInFrames = source->frameCount;
    *sourcedata = source;
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Read_Frames )( void** plugdata, void** codecdata, void** sourcedata, void* destination, UInt32 frameCapacity, UInt32* framesRead ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAuSource* source = (TAuSource*) *sourcedata;
    UInt8* out = (UInt8*) destination;
    UInt32 frameBytesIn, sampleCount, bytesRead, i;
    UInt8* in;

    *framesRead = 0;
    if ( NULL == source )
        return 0;

    if ( GUCEF_AUDIOCODEC_LENGTH_UNKNOWN != source->frameCount )
    {
        UInt64 left = source->frameCount > source->position ? source->frameCount - source->position : 0;
        if ( left < frameCapacity )
            frameCapacity = (UInt32) left;
    }
    if ( 0 == frameCapacity )
        return 1;

    /*
     *  The file samples go to the end of the destination; expanding 1 byte companded samples to 2 bytes from the
     *  front then never overwrites a sample before it is read
     */
    frameBytesIn = source->bytesPerSampleIn * source->channels;
    in = out + (size_t) frameCapacity * source->channels * ( source->bytesPerSampleOut - source->bytesPerSampleIn );
    bytesRead = source->input->read( source->input, in, 1, frameCapacity * frameBytesIn );
    *framesRead = bytesRead / frameBytesIn;
    sampleCount = *framesRead * source->channels;

    switch ( source->encoding )
    {
        case AU_ENCODING_MULAW:
        {
            Int16* samples = (Int16*) out;
            for ( i=0; i<sampleCount; ++i )
                samples[ i ] = MuLawToLinear( in[ i ] );
            break;
        }
        case AU_ENCODING_ALAW:
        {
            Int16* samples = (Int16*) out;
            for ( i=0; i<sampleCount; ++i )
                samples[ i ] = ALawToLinear( in[ i ] );
            break;
        }
        case AU_ENCODING_LINEAR8:
        {
            /* signed in the file, unsigned in gucefAUDIO */
            for ( i=0; i<sampleCount; ++i )
                out[ i ] = (UInt8) ( in[ i ] ^ 0x80 );
            break;
        }
        default:
        {
            if ( !IsHostBigEndian() )
                SwapSamples( out, sampleCount, source->bytesPerSampleIn );
            break;
        }
    }

    source->position += *framesRead;
    if ( bytesRead % frameBytesIn != 0 )
    {
        /* a partial frame at the end: the file was cut short, end here */
        source->frameCount = source->position;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Seek )( void** plugdata, void** codecdata, void** sourcedata, UInt64 frame ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAuSource* source = (TAuSource*) *sourcedata;
    if ( NULL == source )
        return 0;
    if ( GUCEF_AUDIOCODEC_LENGTH_UNKNOWN != source->frameCount && frame > source->frameCount )
        return 0;
    if ( 0 != source->input->setpos( source->input, source->dataOffset + frame * source->bytesPerSampleIn * source->channels ) )
        return 0;
    source->position = frame;
    return 1;
}

/*-------------------------------------------------------------------------*/

void GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Src_Close )( void** plugdata, void** codecdata, void** sourcedata ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    free( *sourcedata );
    *sourcedata = NULL;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Decode_Meta_Data )( void** plugdata, void** codecdata, TIOAccess* input, TVariantMapApi* metaData ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    UInt32 dataOffset, dataSize, encoding, sampleRate, channels, length;
    char* annotation;
    const char* encodingName;

    if ( !ReadHeader( input, &dataOffset, &dataSize, &encoding, &sampleRate, &channels ) )
        return 0;

    switch ( encoding )
    {
        case AU_ENCODING_MULAW: encodingName = "mu-law"; break;
        case AU_ENCODING_ALAW: encodingName = "A-law"; break;
        case AU_ENCODING_FLOAT: encodingName = "float"; break;
        case AU_ENCODING_DOUBLE: encodingName = "double"; break;
        default: encodingName = "linear"; break;
    }
    SetTextValue( metaData, "encoding", encodingName );

    /* The annotation is free text, usually a title or a description */
    length = dataOffset - AU_HEADER_SIZE;
    if ( 0 == length || length > AU_MAX_ANNOTATION )
        return 1;
    annotation = (char*) calloc( 1, length + 1 );
    if ( NULL == annotation )
        return 1;
    if ( 1 == input->read( input, annotation, length, 1 ) && '\0' != annotation[ 0 ] )
        SetTextValue( metaData, "comment", annotation );
    free( annotation );
    return 1;
}

/*-------------------------------------------------------------------------*/

UInt32 GUCEF_PLUGIN_CALLSPEC_PREFIX
MODULE_API_FUNCTION( AUDIOCODECPLUG_Encode )( void** plugdata, void** codecdata, const TAudioFormatInfo* format, const void* frames, UInt64 frameCount, TIOAccess* output, const TVariantMapApi* options ) GUCEF_PLUGIN_CALLSPEC_SUFFIX
{
    TAuPlugin* plugin = (TAuPlugin*) *plugdata;
    const char* requested = GetTextOption( options, "encoding" );
    UInt32 encoding = EncodingOfSampleType( format->sampleType );
    UInt32 bytesPerSampleIn, bytesPerSampleOut;
    UInt64 sampleCount, dataSize, done;
    UInt8 header[ AU_WRITE_DATA_OFFSET ];
    UInt8 chunk[ AU_COPY_CHUNK_BYTES ];
    const UInt8* in = (const UInt8*) frames;

    if ( NULL == output || NULL == output->write || 0 == encoding || 0 == format->channelCount || 0 == format->sampleRate )
        return 0;

    if ( NULL != requested && ( 0 == strcmp( requested, "mulaw" ) || 0 == strcmp( requested, "alaw" ) ) )
    {
        if ( GUCEF_AUDIOCODEC_SAMPLETYPE_INT16 != format->sampleType )
        {
            GUCEF_C_ERROR_LOG( plugin->libApi, GUCEF_LOGLEVEL_NORMAL, "audiocodecpluginAU: mu-law and A-law are written from 16 bit samples only" );
            return 0;
        }
        encoding = 0 == strcmp( requested, "mulaw" ) ? AU_ENCODING_MULAW : AU_ENCODING_ALAW;
    }

    bytesPerSampleIn = BytesPerSampleOfEncoding( EncodingOfSampleType( format->sampleType ) );
    bytesPerSampleOut = BytesPerSampleOfEncoding( encoding );
    sampleCount = frameCount * format->channelCount;
    dataSize = sampleCount * bytesPerSampleOut;

    memset( header, 0, sizeof( header ) );
    WriteBE32( header, (UInt32) AU_MAGIC );
    WriteBE32( header + 4, AU_WRITE_DATA_OFFSET );
    WriteBE32( header + 8, dataSize < AU_UNKNOWN_SIZE ? (UInt32) dataSize : (UInt32) AU_UNKNOWN_SIZE );
    WriteBE32( header + 12, encoding );
    WriteBE32( header + 16, format->sampleRate );
    WriteBE32( header + 20, format->channelCount );
    if ( 1 != output->write( output, header, sizeof( header ), 1 ) )
        return 0;

    /* In chunks, converted to the order and form of the file */
    done = 0;
    while ( done < sampleCount )
    {
        UInt64 samples = ( sizeof( chunk ) / 8 );
        UInt64 i;
        if ( samples > sampleCount - done )
            samples = sampleCount - done;

        switch ( encoding )
        {
            case AU_ENCODING_MULAW:
            {
                const Int16* source = (const Int16*) in;
                for ( i=0; i<samples; ++i )
                    chunk[ i ] = LinearToMuLaw( source[ done+i ] );
                break;
            }
            case AU_ENCODING_ALAW:
            {
                const Int16* source = (const Int16*) in;
                for ( i=0; i<samples; ++i )
                    chunk[ i ] = LinearToALaw( source[ done+i ] );
                break;
            }
            case AU_ENCODING_LINEAR8:
            {
                for ( i=0; i<samples; ++i )
                    chunk[ i ] = (UInt8) ( in[ done+i ] ^ 0x80 );
                break;
            }
            default:
            {
                memcpy( chunk, in + done * bytesPerSampleIn, (size_t) ( samples * bytesPerSampleIn ) );
                if ( !IsHostBigEndian() )
                    SwapSamples( chunk, (UInt32) samples, bytesPerSampleIn );
                break;
            }
        }

        if ( 1 != output->write( output, chunk, (UInt32) ( samples * bytesPerSampleOut ), 1 ) )
            return 0;
        done += samples;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/
