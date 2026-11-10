/*
 *  audiocodecpluginMP3: gucefAUDIO codec plugin for MP3 audio
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
 *  The text of ID3 tags. dr_mp3 hands out the raw tag, this turns the text frames into entries of the map of the
 *  host. Only what a game needs: title, artist, album, track, date, genre, a few more common ones and the user
 *  defined text frames (where loop points end up for MP3 files).
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>

#include "audiocodecpluginMP3_internal.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

#define ID3_MAX_TEXT_BYTES      ( 64U * 1024U )     /* the longest frame text that is used, longer is cut off */
#define ID3_MAX_NAME_BYTES      128U

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static void
InsertText( TVariantMapApi* map, const char* key, const char* value )
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

    map->insert( map->privateData, &keyData, &valueData, 0 );
}

/*-------------------------------------------------------------------------*/

static UInt32
SyncSafe( const UInt8* bytes )
{
    return ( (UInt32) ( bytes[ 0 ] & 0x7F ) << 21 ) | ( (UInt32) ( bytes[ 1 ] & 0x7F ) << 14 ) | ( (UInt32) ( bytes[ 2 ] & 0x7F ) << 7 ) | (UInt32) ( bytes[ 3 ] & 0x7F );
}

/*-------------------------------------------------------------------------*/

/** Removes the zero byte that unsynchronisation puts after every 0xFF. Returns the new size, works in place */
static UInt32
RemoveUnsynchronisation( UInt8* data, UInt32 size )
{
    UInt32 in, out = 0;
    for ( in=0; in<size; ++in )
    {
        data[ out++ ] = data[ in ];
        if ( 0xFF == data[ in ] && in + 1 < size && 0x00 == data[ in + 1 ] )
            ++in;
    }
    return out;
}

/*-------------------------------------------------------------------------*/

/** Appends a code point as UTF-8, returns the new length. 'out' has room for 4 bytes */
static UInt32
AppendUtf8( char* out, UInt32 length, UInt32 codePoint )
{
    if ( codePoint < 0x80 )
        out[ length++ ] = (char) codePoint;
    else if ( codePoint < 0x800 )
    {
        out[ length++ ] = (char) ( 0xC0 | ( codePoint >> 6 ) );
        out[ length++ ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    else if ( codePoint < 0x10000UL )
    {
        out[ length++ ] = (char) ( 0xE0 | ( codePoint >> 12 ) );
        out[ length++ ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        out[ length++ ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    else
    {
        out[ length++ ] = (char) ( 0xF0 | ( codePoint >> 18 ) );
        out[ length++ ] = (char) ( 0x80 | ( ( codePoint >> 12 ) & 0x3F ) );
        out[ length++ ] = (char) ( 0x80 | ( ( codePoint >> 6 ) & 0x3F ) );
        out[ length++ ] = (char) ( 0x80 | ( codePoint & 0x3F ) );
    }
    return length;
}

/*-------------------------------------------------------------------------*/

/**
 *  Converts the text that starts at 'data' to a UTF-8 string in 'out' (NUL terminated, room for 2 * size + 5 bytes
 *  and at most ID3_MAX_TEXT_BYTES + 4 used). The text ends at its terminator or at the end of the data. Returns the
 *  number of bytes of 'data' the text and its terminator took.
 */
static UInt32
DecodeText( UInt32 encoding, const UInt8* data, UInt32 size, char* out )
{
    UInt32 length = 0;
    UInt32 i = 0;

    if ( 1 == encoding || 2 == encoding )
    {
        int isBigEndian = 2 == encoding;
        if ( 1 == encoding && size >= 2 )
        {
            if ( 0xFF == data[ 0 ] && 0xFE == data[ 1 ] )
                i = 2;
            else if ( 0xFE == data[ 0 ] && 0xFF == data[ 1 ] )
            {
                isBigEndian = 1;
                i = 2;
            }
        }
        while ( i + 1 < size && length < ID3_MAX_TEXT_BYTES )
        {
            UInt32 unit = isBigEndian ? ( ( (UInt32) data[ i ] << 8 ) | data[ i + 1 ] ) : ( ( (UInt32) data[ i + 1 ] << 8 ) | data[ i ] );
            i += 2;
            if ( 0 == unit )
                break;
            if ( unit >= 0xD800 && unit < 0xDC00 && i + 1 < size )
            {
                UInt32 low = isBigEndian ? ( ( (UInt32) data[ i ] << 8 ) | data[ i + 1 ] ) : ( ( (UInt32) data[ i + 1 ] << 8 ) | data[ i ] );
                if ( low >= 0xDC00 && low < 0xE000 )
                {
                    unit = 0x10000UL + ( ( unit - 0xD800 ) << 10 ) + ( low - 0xDC00 );
                    i += 2;
                }
                else
                    unit = 0xFFFD;
            }
            else if ( unit >= 0xD800 && unit < 0xE000 )
                unit = 0xFFFD;
            length = AppendUtf8( out, length, unit );
        }
        /* an odd trailing byte belongs to nothing */
    }
    else if ( 0 == encoding )
    {
        /* ISO 8859-1: the first 256 code points */
        while ( i < size && length < ID3_MAX_TEXT_BYTES )
        {
            UInt8 c = data[ i++ ];
            if ( 0 == c )
                break;
            length = AppendUtf8( out, length, c );
        }
    }
    else
    {
        /* UTF-8, kept as it is */
        while ( i < size && length < ID3_MAX_TEXT_BYTES )
        {
            UInt8 c = data[ i++ ];
            if ( 0 == c )
                break;
            out[ length++ ] = (char) c;
        }
    }
    out[ length ] = '\0';
    return i;
}

/*-------------------------------------------------------------------------*/

/** The name a text frame has in the map, NULL for frames that are not used */
static const char*
NameOfTextFrame( const char* id, UInt32 version )
{
    if ( version == 2 )
    {
        if ( 0 == memcmp( id, "TT2", 3 ) ) return "title";
        if ( 0 == memcmp( id, "TP1", 3 ) ) return "artist";
        if ( 0 == memcmp( id, "TAL", 3 ) ) return "album";
        if ( 0 == memcmp( id, "TP2", 3 ) ) return "albumartist";
        if ( 0 == memcmp( id, "TCM", 3 ) ) return "composer";
        if ( 0 == memcmp( id, "TRK", 3 ) ) return "track";
        if ( 0 == memcmp( id, "TPA", 3 ) ) return "disc";
        if ( 0 == memcmp( id, "TYE", 3 ) ) return "date";
        if ( 0 == memcmp( id, "TCO", 3 ) ) return "genre";
        return NULL;
    }
    if ( 0 == memcmp( id, "TIT2", 4 ) ) return "title";
    if ( 0 == memcmp( id, "TPE1", 4 ) ) return "artist";
    if ( 0 == memcmp( id, "TALB", 4 ) ) return "album";
    if ( 0 == memcmp( id, "TPE2", 4 ) ) return "albumartist";
    if ( 0 == memcmp( id, "TCOM", 4 ) ) return "composer";
    if ( 0 == memcmp( id, "TRCK", 4 ) ) return "track";
    if ( 0 == memcmp( id, "TPOS", 4 ) ) return "disc";
    if ( 0 == memcmp( id, "TDRC", 4 ) ) return "date";
    if ( 0 == memcmp( id, "TYER", 4 ) ) return "date";
    if ( 0 == memcmp( id, "TCON", 4 ) ) return "genre";
    return NULL;
}

/*-------------------------------------------------------------------------*/

/** Index of the end of the terminator of the string in 'data' that starts at 'offset' with the given encoding */
static UInt32
SkipTerminatedString( UInt32 encoding, const UInt8* data, UInt32 size, UInt32 offset )
{
    if ( 1 == encoding || 2 == encoding )
    {
        while ( offset + 1 < size )
        {
            if ( 0 == data[ offset ] && 0 == data[ offset + 1 ] )
                return offset + 2;
            offset += 2;
        }
        return size;
    }
    while ( offset < size )
    {
        if ( 0 == data[ offset++ ] )
            return offset;
    }
    return size;
}

/*-------------------------------------------------------------------------*/

static char
LowerCase( char c )
{
    return ( c >= 'A' && c <= 'Z' ) ? (char) ( c + ( 'a' - 'A' ) ) : c;
}

/*-------------------------------------------------------------------------*/

/** Handles the content of one frame. 'frame' is a copy that may be changed */
static UInt32
AddFrame( TVariantMapApi* map, const char* id, UInt32 version, UInt8* frame, UInt32 size, char* text )
{
    UInt32 encoding;

    if ( 'T' != id[ 0 ] || size < 2 )
        return 0;
    encoding = frame[ 0 ];
    if ( encoding > 3 )
        return 0;

    if ( 0 == memcmp( id, "TXXX", 4 ) || 0 == memcmp( id, "TXX", 3 ) )
    {
        /* a description and then a value: the description is the name */
        char name[ ID3_MAX_NAME_BYTES + 1 ];
        UInt32 end = SkipTerminatedString( encoding, frame, size, 1 );
        UInt32 i;
        UInt32 nameLength;
        DecodeText( encoding, frame + 1, end - 1, text );
        nameLength = (UInt32) strlen( text );
        if ( 0 == nameLength || nameLength > ID3_MAX_NAME_BYTES )
            return 0;
        for ( i=0; i<nameLength; ++i )
        {
            char c = LowerCase( text[ i ] );
            if ( c < 0x21 || c > 0x7D || '=' == c )
                return 0;
            name[ i ] = c;
        }
        name[ nameLength ] = '\0';
        if ( end >= size )
            return 0;
        DecodeText( encoding, frame + end, size - end, text );
        InsertText( map, name, text );
        return 1;
    }
    else
    {
        const char* name = NameOfTextFrame( id, version );
        if ( NULL == name )
            return 0;
        DecodeText( encoding, frame + 1, size - 1, text );
        if ( '\0' == text[ 0 ] )
            return 0;
        InsertText( map, name, text );
        return 1;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      ID3V2                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
Mp3AddId3v2Tags( TVariantMapApi* map, const UInt8* data, UInt32 size )
{
    UInt32 version, flags, bodySize, offset = 0, added = 0, frameHeaderSize;
    UInt8* body;
    char* text;

    if ( NULL == data || size < 10 || 0 != memcmp( data, "ID3", 3 ) )
        return 0;
    version = data[ 3 ];
    flags = data[ 5 ];
    if ( version < 2 || version > 4 || 0xFF == data[ 4 ] )
        return 0;

    bodySize = SyncSafe( data + 6 );
    if ( bodySize > size - 10 )
        bodySize = size - 10;
    if ( 0 == bodySize )
        return 0;

    /* a copy, as unsynchronisation is removed in place */
    body = (UInt8*) malloc( bodySize );
    text = (char*) malloc( 2U * ( ID3_MAX_TEXT_BYTES + 8U ) );
    if ( NULL == body || NULL == text )
    {
        free( body );
        free( text );
        return 0;
    }
    memcpy( body, data + 10, bodySize );
    if ( flags & 0x80 )
        bodySize = RemoveUnsynchronisation( body, bodySize );

    /* extended header */
    if ( version >= 3 && ( flags & 0x40 ) )
    {
        if ( bodySize < 4 )
            bodySize = 0;
        else
        {
            UInt32 extSize = 3 == version ? ( ( (UInt32) body[ 0 ] << 24 ) | ( (UInt32) body[ 1 ] << 16 ) | ( (UInt32) body[ 2 ] << 8 ) | body[ 3 ] ) + 4U : SyncSafe( body );
            if ( extSize > bodySize )
                bodySize = 0;
            else
                offset = extSize;
        }
    }

    frameHeaderSize = 2 == version ? 6 : 10;
    while ( offset + frameHeaderSize <= bodySize )
    {
        const UInt8* header = body + offset;
        char id[ 5 ];
        UInt32 frameSize, frameFlags = 0, skip = 0;
        UInt32 contentOffset;

        if ( 0 == header[ 0 ] )
            break;      /* padding */
        memset( id, 0, sizeof( id ) );
        memcpy( id, header, 2 == version ? 3 : 4 );

        if ( 2 == version )
            frameSize = ( (UInt32) header[ 3 ] << 16 ) | ( (UInt32) header[ 4 ] << 8 ) | header[ 5 ];
        else if ( 3 == version )
        {
            frameSize = ( (UInt32) header[ 4 ] << 24 ) | ( (UInt32) header[ 5 ] << 16 ) | ( (UInt32) header[ 6 ] << 8 ) | header[ 7 ];
            frameFlags = ( (UInt32) header[ 8 ] << 8 ) | header[ 9 ];
        }
        else
        {
            frameSize = SyncSafe( header + 4 );
            frameFlags = ( (UInt32) header[ 8 ] << 8 ) | header[ 9 ];
        }

        contentOffset = offset + frameHeaderSize;
        if ( frameSize > bodySize - contentOffset )
            break;

        /* compressed or encrypted frames can not be read; the others may carry a length indicator or be unsynchronised */
        if ( 3 == version && ( frameFlags & 0x00C0 ) )
            skip = 1;
        if ( 4 == version && ( frameFlags & 0x000C ) )
            skip = 1;

        if ( !skip && frameSize > 0 && 'T' == id[ 0 ] )
        {
            UInt8* content = body + contentOffset;
            UInt32 contentSize = frameSize;
            if ( 4 == version && ( frameFlags & 0x0001 ) )
            {
                if ( contentSize < 4 )
                    contentSize = 0;
                else
                {
                    content += 4;
                    contentSize -= 4;
                }
            }
            if ( 4 == version && ( frameFlags & 0x0002 ) )
                contentSize = RemoveUnsynchronisation( content, contentSize );
            if ( contentSize > 0 )
                added += AddFrame( map, id, version, content, contentSize, text );
        }
        offset = contentOffset + frameSize;
    }

    free( body );
    free( text );
    return added;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      ID3V1                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** A fixed width Latin-1 field without the padding of spaces and NULs */
static void
AddFixedField( TVariantMapApi* map, const char* name, const UInt8* field, UInt32 size, UInt32* added )
{
    char text[ 64 ];
    UInt32 length = 0;
    UInt32 end = 0;
    UInt32 i;

    for ( i=0; i<size; ++i )
    {
        if ( 0 == field[ i ] )
            break;
        if ( ' ' != field[ i ] )
            end = i + 1;
    }
    for ( i=0; i<end; ++i )
        length = AppendUtf8( text, length, field[ i ] );
    text[ length ] = '\0';
    if ( length > 0 )
    {
        InsertText( map, name, text );
        ++*added;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
Mp3AddId3v1Tags( TVariantMapApi* map, const UInt8* data )
{
    UInt32 added = 0;
    if ( NULL == data || 0 != memcmp( data, "TAG", 3 ) )
        return 0;

    AddFixedField( map, "title", data + 3, 30, &added );
    AddFixedField( map, "artist", data + 33, 30, &added );
    AddFixedField( map, "album", data + 63, 30, &added );
    AddFixedField( map, "date", data + 93, 4, &added );

    /* version 1.1: a zero byte and then the track number take the place of the end of the comment */
    if ( 0 == data[ 125 ] && 0 != data[ 126 ] )
    {
        char track[ 8 ];
        UInt32 value = data[ 126 ];
        UInt32 length = 0;
        char digits[ 4 ];
        UInt32 n = 0;
        while ( value > 0 )
        {
            digits[ n++ ] = (char) ( '0' + value % 10 );
            value /= 10;
        }
        while ( n > 0 )
            track[ length++ ] = digits[ --n ];
        track[ length ] = '\0';
        InsertText( map, "track", track );
        ++added;
        AddFixedField( map, "comment", data + 97, 28, &added );
    }
    else
        AddFixedField( map, "comment", data + 97, 30, &added );
    return added;
}

/*-------------------------------------------------------------------------*/
