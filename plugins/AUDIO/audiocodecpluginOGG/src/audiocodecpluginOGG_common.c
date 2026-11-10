/*
 *  audiocodecpluginOGG: gucefAUDIO codec plugin for Ogg Vorbis and Ogg Opus audio
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
 *  What the Vorbis and the Opus part have in common: the input, options and tags, and the channel orders.
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "audiocodecpluginOGG_internal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define OGG_MAX_TAG_VALUE_BYTES     ( 64U * 1024U )     /* a longer tag value is cut off */
#define OGG_MAX_TAG_NAME_BYTES      256U
#define OGG_MAX_TAGS                4096                /* tags beyond this many are not read */

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

size_t
OggInputRead( TOggInput* io, void* destination, size_t bytes )
{
    size_t done = 0;
    while ( done < bytes )
    {
        size_t left = bytes - done;
        UInt32 chunk = left > 0x40000000UL ? 0x40000000UL : (UInt32) left;
        UInt32 got = io->input->read( io->input, ( (UInt8*) destination ) + done, 1, chunk );
        done += got;
        if ( got < chunk )
            break;
    }
    return done;
}

/*-------------------------------------------------------------------------*/

int
OggInputSeek( TOggInput* io, Int64 offset, int whence )
{
    Int64 target;
    switch ( whence )
    {
        case 0: target = offset; break;
        case 1: target = (Int64) ( io->input->tell( io->input ) - io->start ) + offset; break;
        case 2:
        {
            if ( 0 == io->size )
                return -1;
            target = (Int64) io->size + offset;
            break;
        }
        default: return -1;
    }
    if ( target < 0 || ( 0 != io->size && (UInt64) target > io->size ) )
        return -1;
    return 0 == io->input->setpos( io->input, io->start + (UInt64) target ) ? 0 : -1;
}

/*-------------------------------------------------------------------------*/

Int64
OggInputTell( TOggInput* io )
{
    UInt64 position = io->input->tell( io->input );
    if ( position < io->start )
        return -1;
    return (Int64) ( position - io->start );
}

/*-------------------------------------------------------------------------*/

const char*
OggGetTextOption( const TVariantMapApi* map, const char* key )
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

double
OggGetNumberOption( const TVariantMapApi* map, const char* key, double defaultValue )
{
    const char* text = OggGetTextOption( map, key );
    char* end;
    double value;
    if ( NULL == text )
        return defaultValue;
    value = strtod( text, &end );
    return end == text ? defaultValue : value;
}

/*-------------------------------------------------------------------------*/

void
OggSetText( TVariantMapApi* map, const char* key, const char* value, UInt32 valueSize )
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
    valueData.union_data.heap_data.heap_data_size = valueSize;
    valueData.union_data.heap_data.heap_data_is_linked = 1;

    map->insert( map->privateData, &keyData, &valueData, 0 );
}

/*-------------------------------------------------------------------------*/

void
OggAddComments( TVariantMapApi* map, char** comments, const int* lengths, int count )
{
    char name[ OGG_MAX_TAG_NAME_BYTES + 1 ];
    char* value;
    int i;

    if ( NULL == comments || NULL == lengths || count <= 0 )
        return;
    value = (char*) malloc( OGG_MAX_TAG_VALUE_BYTES + 1 );
    if ( NULL == value )
        return;
    if ( count > OGG_MAX_TAGS )
        count = OGG_MAX_TAGS;

    for ( i=0; i<count; ++i )
    {
        const char* comment = comments[ i ];
        UInt32 length, nameLength, valueLength, n;

        if ( NULL == comment || lengths[ i ] <= 0 )
            continue;
        length = (UInt32) lengths[ i ];

        /* the name is up to the first '=' and made of printable ASCII */
        nameLength = 0;
        while ( nameLength < length && '=' != comment[ nameLength ] )
            ++nameLength;
        if ( nameLength == length || 0 == nameLength || nameLength > OGG_MAX_TAG_NAME_BYTES )
            continue;
        for ( n=0; n<nameLength; ++n )
        {
            char c = comment[ n ];
            if ( c < 0x20 || c > 0x7D )
                break;
            name[ n ] = ( c >= 'A' && c <= 'Z' ) ? (char) ( c + ( 'a' - 'A' ) ) : c;
        }
        if ( n != nameLength )
            continue;
        name[ nameLength ] = '\0';

        /* pictures are far too big to be a tag of a game, the rest is text */
        if ( 0 == strcmp( name, "metadata_block_picture" ) )
            continue;

        valueLength = length - nameLength - 1;
        if ( valueLength > OGG_MAX_TAG_VALUE_BYTES )
            valueLength = OGG_MAX_TAG_VALUE_BYTES;
        memcpy( value, comment + nameLength + 1, valueLength );
        value[ valueLength ] = '\0';
        OggSetText( map, name, value, (UInt32) strlen( value ) );
    }
    free( value );
}

/*-------------------------------------------------------------------------*/

const char*
OggNextTagLine( const char* lines, TOggTagLine* line )
{
    while ( NULL != lines && '\0' != *lines )
    {
        const char* end = strchr( lines, '\n' );
        size_t length = NULL != end ? (size_t) ( end - lines ) : strlen( lines );
        const char* next = NULL != end ? end + 1 : lines + length;
        const char* equals;

        if ( length > 0 && '\r' == lines[ length - 1 ] )
            --length;
        equals = length > 0 ? (const char*) memchr( lines, '=', length ) : NULL;
        if ( NULL != equals && equals != lines )
        {
            line->name = lines;
            line->nameLength = (size_t) ( equals - lines );
            line->value = equals + 1;
            line->valueLength = length - line->nameLength - 1;
            return next;
        }
        lines = next;
    }
    return NULL;
}

/*-------------------------------------------------------------------------*/

/*
 *  Vorbis (and Opus family 1) order the channels of a surround layout differently than WAVE:
 *    3  Vorbis L C R                      WAVE L R C
 *    5  Vorbis L C R BL BR                WAVE L R C BL BR
 *    6  Vorbis L C R BL BR LFE            WAVE L R C LFE BL BR
 *    7  Vorbis L C R SL SR BC LFE         WAVE L R C LFE BC SL SR
 *    8  Vorbis L C R SL SR BL BR LFE      WAVE L R C LFE BL BR SL SR
 *  The table says for every channel in WAVE order the channel of Vorbis order that is the same speaker.
 */
const UInt8*
OggWaveToVorbisOrder( UInt32 channels )
{
    static const UInt8 order[ 9 ][ 8 ] = {
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 1, 0, 0, 0, 0, 0, 0 },
        { 0, 2, 1, 0, 0, 0, 0, 0 },
        { 0, 1, 2, 3, 0, 0, 0, 0 },
        { 0, 2, 1, 3, 4, 0, 0, 0 },
        { 0, 2, 1, 5, 3, 4, 0, 0 },
        { 0, 2, 1, 6, 5, 3, 4, 0 },
        { 0, 2, 1, 7, 5, 6, 3, 4 } };
    return channels >= 1 && channels <= 8 ? order[ channels ] : order[ 0 ];
}

/*-------------------------------------------------------------------------*/

/** The speaker mask of the WAVE order channels: mono and stereo have no explicit one (0) */
UInt32
OggWaveLayout( UInt32 channels )
{
    switch ( channels )
    {
        case 3: return 0x007;
        case 4: return 0x033;
        case 5: return 0x037;
        case 6: return 0x03F;
        case 7: return 0x70F;
        case 8: return 0x63F;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
OggBytesPerSample( UInt32 sampleType )
{
    switch ( sampleType )
    {
        case GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8: return 1;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT16: return 2;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT24: return 3;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT32: return 4;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT32: return 4;
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64: return 8;
        default: return 0;
    }
}

/*-------------------------------------------------------------------------*/

float
OggGetSampleAsFloat( const UInt8* data, UInt32 sampleType, UInt64 index )
{
    switch ( sampleType )
    {
        case GUCEF_AUDIOCODEC_SAMPLETYPE_UINT8:
        {
            return ( (float) data[ index ] - 128.0f ) / 128.0f;
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT16:
        {
            Int16 value;
            memcpy( &value, data + index * 2, 2 );
            return (float) value / 32768.0f;
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT24:
        {
            const UInt8* p = data + index * 3;
            Int32 value = IsHostBigEndian() ? ( ( (Int32) p[ 0 ] << 16 ) | ( (Int32) p[ 1 ] << 8 ) | (Int32) p[ 2 ] )
                                            : ( ( (Int32) p[ 2 ] << 16 ) | ( (Int32) p[ 1 ] << 8 ) | (Int32) p[ 0 ] );
            if ( value & 0x800000 )
                value -= 0x1000000;
            return (float) value / 8388608.0f;
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_INT32:
        {
            Int32 value;
            memcpy( &value, data + index * 4, 4 );
            return (float) ( (double) value / 2147483648.0 );
        }
        case GUCEF_AUDIOCODEC_SAMPLETYPE_FLOAT64:
        {
            double value;
            memcpy( &value, data + index * 8, 8 );
            return (float) value;
        }
        default:
        {
            float value;
            memcpy( &value, data + index * 4, 4 );
            return value;
        }
    }
}

/*-------------------------------------------------------------------------*/

int
OggWriteBytes( TIOAccess* output, const void* data, size_t size )
{
    const UInt8* p = (const UInt8*) data;
    while ( size > 0 )
    {
        UInt32 chunk = size > 0x40000000UL ? 0x40000000UL : (UInt32) size;
        if ( chunk != output->write( output, p, 1, chunk ) )
            return 0;
        p += chunk;
        size -= chunk;
    }
    return 1;
}

/*-------------------------------------------------------------------------*/
