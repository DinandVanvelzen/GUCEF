/*
 *  GucefArchiver: Tool to manipulate archives
 *  Copyright (C) 2002 - 2015.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <stdio.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#include "gucefCORE_CGlobalConfigValueList.h"
#define GUCEF_CORE_CGLOBALCONFIGVALUELIST_H
#endif /* GUCEF_CORE_CGLOBALCONFIGVALUELIST_H ? */

#ifndef GUCEF_CORE_CPLUGINCONTROL_H
#include "CPluginControl.h"
#define GUCEF_CORE_CPLUGINCONTROL_H
#endif /* GUCEF_CORE_CPLUGINCONTROL_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_DVCPPOSWRAP_H
#include "DVCPPOSWRAP.h"
#define GUCEF_CORE_DVCPPOSWRAP_H
#endif /* GUCEF_CORE_DVCPPOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_CONFIGSTORE_H
#include "CConfigStore.h"
#define GUCEF_CORE_CONFIGSTORE_H
#endif /* GUCEF_CORE_CONFIGSTORE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_IMAGE_H
#include "gucefIMAGE.h"
#define GUCEF_IMAGE_H
#endif /* GUCEF_IMAGE_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

struct SPcmWaveHeader
{
    // The RIFF chunk descriptor
    char ChunkID[ 4 ];              // should be "RIFF"
    CORE::Int32 ChunkSize;          // 36 + SubChunk2Size, or more precisely: 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size). This is the size of the rest of the chunk following this number.  This is the size of the entire file in bytes minus 8 bytes for the two fields not included in this count: ChunkID and ChunkSize.
    char Format[ 4 ];               // should be "WAVE"

    // The format sub chunk
    char Subchunk1ID[ 4 ];          // should be "fmt "
    CORE::Int32 Subchunk1Size;      // 16 for PCM. This is the size of the rest of the Subchunk which follows this number.
    CORE::Int16 AudioFormat;        // PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some form of compression.
    CORE::Int16 NumChannels;        // Mono = 1, Stereo = 2, etc.
    CORE::Int32 SampleRate;         // 8000, 44100, etc.
    CORE::Int32 ByteRate;           // SampleRate * NumChannels * BitsPerSample/8
    CORE::Int16 BlockAlign;         // NumChannels * BitsPerSample/8  The number of bytes for one sample including all channels. I wonder what happens when this number isn't an integer?
    CORE::Int16 BitsPerSample;      // 8 bits = 8, 16 bits = 16, etc.

    // The data sub chunk
    char Subchunk2ID[ 4 ];          // should be "data"
    CORE::Int32 Subchunk2Size;      // == NumSamples * NumChannels * BitsPerSample/8   This is the number of bytes in the data. You can also think of this as the size of the read of the subchunk following this number.
    // ? Data                       // The actual data block with all the raw data
};
typedef struct SPcmWaveHeader PcmWaveHeader;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
ConfigureITVPcmHeader( PcmWaveHeader& header, CORE::Int32 dataSize, CORE::Int32 sampleRate )
{
    memcpy( header.ChunkID, "RIFF", 4 );
    header.ChunkSize = 36 + dataSize;
    memcpy( header.Format, "WAVE", 4 );
    memcpy( header.Subchunk1ID, "fmt ", 4 );
    header.Subchunk1Size = 16;
    header.AudioFormat = 1; // ITV Audio is always PCM
    header.NumChannels = 1; // ITV PCM is always mono
    header.SampleRate = sampleRate; // ITV PCM is 8 bit unsigned mono at 22000 Hz
    header.BitsPerSample = 8; // ITV PCM is always 8 bit sampling depth
    header.ByteRate = header.SampleRate * header.NumChannels * (header.BitsPerSample/8);
    header.BlockAlign = header.NumChannels * (header.BitsPerSample/8);
    memcpy( header.Subchunk2ID, "data", 4 );
    header.Subchunk2Size = dataSize;
}

/*-------------------------------------------------------------------------*/

void
WritePcmWaveHeader( const PcmWaveHeader& header ,
                    CORE::CIOAccess& outStream  )
{
    outStream.Write( header.ChunkID, 4, 1 );
    outStream.Write( &header.ChunkSize, 4, 1 );
    outStream.Write( header.Format, 4, 1 );
    outStream.Write( &header.Subchunk1ID, 4, 1 );
    outStream.Write( &header.Subchunk1Size, 4, 1 );
    outStream.Write( &header.AudioFormat, 2, 1 );
    outStream.Write( &header.NumChannels, 2, 1 );
    outStream.Write( &header.SampleRate, 4, 1 );
    outStream.Write( &header.ByteRate, 4, 1 );
    outStream.Write( &header.BlockAlign, 2, 1 );
    outStream.Write( &header.BitsPerSample, 2, 1 );
    outStream.Write( header.Subchunk2ID, 4, 1 );
    outStream.Write( &header.Subchunk2Size, 4, 1 );
}

/*-------------------------------------------------------------------------*/

/** Setpos() follows fsetpos(): 0 is success */
bool
SeekTo( CORE::CIOAccess& access, CORE::UInt64 position )
{
    return 0 == access.Setpos( position );
}

/*-------------------------------------------------------------------------*/

/**
 *  An ITV sound resource is the 55 byte resource header, followed by a u32 with the nr of bytes
 *  of sample data, followed by the samples (unsigned 8 bit, mono)
 */
bool
WriteITVPcm( CORE::CIOAccess& rawStream ,
             CORE::CIOAccess& outStream ,
             CORE::Int32 sampleRate     )
{GUCEF_TRACE;

    const CORE::UInt64 resourceHeaderSize = 55;
    CORE::UInt64 resourceSize = rawStream.GetSize();
    CORE::UInt32 sampleDataSize = 0;
    if ( resourceSize < resourceHeaderSize + 4 ||
         !SeekTo( rawStream, resourceHeaderSize ) ||
         1 != rawStream.Read( &sampleDataSize, 4, 1 ) )
    {
        return false;
    }

    // Sanity check the nr of samples against what is actually available
    CORE::UInt64 available = resourceSize - resourceHeaderSize - 4;
    if ( sampleDataSize != available )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ITV sound declares " << sampleDataSize << " bytes of samples but " <<
                           available << " bytes are available, using the smallest" );
        if ( available < sampleDataSize )
            sampleDataSize = (CORE::UInt32) available;
    }

    PcmWaveHeader header;
    ConfigureITVPcmHeader( header, (CORE::Int32) sampleDataSize, sampleRate );
    WritePcmWaveHeader( header, outStream );

    CORE::UInt8 buffer[ 16384 ];
    CORE::UInt32 remaining = sampleDataSize;
    while ( remaining > 0 )
    {
        CORE::UInt32 chunk = remaining < sizeof( buffer ) ? remaining : (CORE::UInt32) sizeof( buffer );
        if ( 1 != rawStream.Read( buffer, chunk, 1 ) )
            return false;
        outStream.Write( buffer, chunk, 1 );
        remaining -= chunk;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  ITV palettes are stored as VGA palettes which have 6 bits per color component (0..63).
 *  This scales the components to the full 8 bit range (0..255) in place.
 */
void
ExpandVgaPalette( IMAGE::CPixelMap& palette )
{GUCEF_TRACE;

    CORE::UInt8* components = static_cast< CORE::UInt8* >( palette.GetDataPtr() );
    CORE::UInt32 componentCount = palette.GetTotalSizeInBytes();
    for ( CORE::UInt32 i=0; i<componentCount; ++i )
    {
        CORE::UInt32 value = components[ i ];
        if ( value > 63 )
            value = 63;
        components[ i ] = (CORE::UInt8) ( ( value * 255 + 31 ) / 63 );
    }
}

/*-------------------------------------------------------------------------*/

typedef GUCEF::vector< CORE::UInt8 > TByteVector;

/**
 *  Reads the payload of a resource, which is everything after the 55 byte resource header
 */
bool
ReadResourcePayload( VFS::CVFSHandle& resource ,
                     TByteVector& payload      )
{GUCEF_TRACE;

    const CORE::UInt64 resourceHeaderSize = 55;
    CORE::IOAccessPtr access = resource.GetAccess();
    CORE::UInt64 size = access->GetSize();
    if ( size < resourceHeaderSize || !SeekTo( *access, resourceHeaderSize ) )
        return false;

    payload.resize( (size_t) ( size - resourceHeaderSize ) );
    return payload.empty() || 1 == access->Read( &payload[ 0 ], (CORE::UInt32) payload.size(), 1 );
}

/*-------------------------------------------------------------------------*/

CORE::UInt32
ReadLE32( const TByteVector& data, size_t offset )
{
    return (CORE::UInt32) data[ offset ]            | ( (CORE::UInt32) data[ offset+1 ] << 8 ) |
           ( (CORE::UInt32) data[ offset+2 ] << 16 ) | ( (CORE::UInt32) data[ offset+3 ] << 24 );
}

/*-------------------------------------------------------------------------*/

CORE::UInt16
ReadLE16( const TByteVector& data, size_t offset )
{
    return (CORE::UInt16) ( data[ offset ] | ( data[ offset+1 ] << 8 ) );
}

/*-------------------------------------------------------------------------*/

CORE::CString
ToHex( const CORE::UInt8* data, size_t size )
{
    static const char digits[] = "0123456789abcdef";
    CORE::CString hex;
    for ( size_t i=0; i<size; ++i )
    {
        hex += digits[ data[ i ] >> 4 ];
        hex += digits[ data[ i ] & 0xF ];
    }
    return hex;
}

/*-------------------------------------------------------------------------*/

/**
 *  The text in ITV is 8 bit text (assumed to be ASCII/Latin-1), bytes above 127 are
 *  emitted as their Latin-1 code point so no information is lost.
 */
CORE::CString
JsonEscape( const CORE::UInt8* text, size_t size )
{
    CORE::CString out;
    for ( size_t i=0; i<size; ++i )
    {
        CORE::UInt8 c = text[ i ];
        switch ( c )
        {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
            {
                if ( c < 0x20 || c > 0x7E )
                {
                    char buffer[ 8 ];
                    sprintf( buffer, "\\u%04x", (unsigned int) c );
                    out += buffer;
                }
                else
                {
                    out += (char) c;
                }
            }
        }
    }
    return out;
}

/*-------------------------------------------------------------------------*/

/**
 *  String resource: u16 length followed by that many bytes of text. The text is terminated
 *  with CR LF and a NUL which are included in the length.
 *  Appends a json object for the string to jsonEntries and a plain text line to textLines.
 */
bool
ExtractTextResource( VFS::CVFSHandle& resource             ,
                     const CORE::CString& resourceFilename ,
                     CORE::CString& jsonEntries            ,
                     CORE::CString& textLines              )
{GUCEF_TRACE;

    TByteVector payload;
    if ( !ReadResourcePayload( resource, payload ) || payload.size() < 2 )
        return false;

    size_t length = ReadLE16( payload, 0 );
    if ( length > payload.size() - 2 )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Text resource " << resourceFilename << " declares more text than available, truncating" );
        length = payload.size() - 2;
    }

    // Trim the trailing NUL and CR LF terminators
    const CORE::UInt8* text = &payload[ 2 ];
    while ( length > 0 && ( text[ length-1 ] == 0 || text[ length-1 ] == '\r' || text[ length-1 ] == '\n' ) )
        --length;

    CORE::CString resourceId = resourceFilename.SubstrToChar( '.', true );
    if ( !jsonEntries.IsNULLOrEmpty() )
        jsonEntries += ",\n";
    jsonEntries += "  { \"id\": " + resourceId + ", \"text\": \"" + JsonEscape( text, length ) + "\" }";
    textLines += resourceId + "\t" + CORE::CString( (const char*) text, (CORE::UInt32) length ) + "\n";
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Names for the fields of the definition tables.
 *  Fields whose use is not known yet keep a neutral name (unknown...).
 *
 *  All definition resources share a 14 byte header (u8 flag, u8 format, u32 pictureId, u32 researchCost, u32 level)
 *  followed by the name and description, followed by a table specific tail:
 *    building (17):  u32 maxWorkers, u32 maintenanceCost, u32 buildingCost, f32 productionFactor, u8 buildingType
 *    part (11..16):  u32 power, u32 spaces, u32 cost, u32 n, u32 materialIds[n], u32 materialAmounts[n], u32 extras[]
 *    habitat (6..9): s8 attributes[15][2] as (max,min) pairs, then planet: u8 orbitMax, u8 orbitMin, star: u8 maxPlanets,
 *                    material: u8 rarity, race: u32 unused, f32 agroPerCitizen, u8 traits[7]
 */
struct SPartKind
{
    const char* kind;
    const char* powerName;
    size_t extraCount;
    const char* extraNames[ 3 ];
};

static const SPartKind* GetPartKind( const CORE::CString& resourceType )
{
    static const SPartKind hull =        { "hull",        "powerRequired", 1, { "baseDefense", GUCEF_NULL, GUCEF_NULL } };
    static const SPartKind powerPlant =  { "powerPlant",  "powerOutput",   0, { GUCEF_NULL, GUCEF_NULL, GUCEF_NULL } };
    static const SPartKind weapon =      { "weapon",      "powerRequired", 1, { "attack", GUCEF_NULL, GUCEF_NULL } };
    static const SPartKind defense =     { "defense",     "powerRequired", 3, { "defense", "evasion", "effectivenessPercent" } };
    static const SPartKind ftlDrive =    { "ftlDrive",    "powerRequired", 2, { "movementPerTurn", "range", GUCEF_NULL } };
    static const SPartKind sensor =      { "sensor",      "powerRequired", 2, { "detectionPercentage", "detectionRange", GUCEF_NULL } };
    if ( "itv11" == resourceType ) return &hull;
    if ( "itv12" == resourceType ) return &powerPlant;
    if ( "itv13" == resourceType ) return &weapon;
    if ( "itv14" == resourceType ) return &defense;
    if ( "itv15" == resourceType ) return &ftlDrive;
    if ( "itv16" == resourceType ) return &sensor;
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

static void
AppendJsonU32( CORE::CString& json, const char* name, CORE::UInt32 value )
{
    char buffer[ 64 ];
    sprintf( buffer, ",\n      \"%s\": %u", name, (unsigned int) value );
    json += buffer;
}

static void
AppendJsonFloat( CORE::CString& json, const char* name, CORE::UInt32 rawValue )
{
    float value;
    memcpy( &value, &rawValue, sizeof( value ) );
    char buffer[ 64 ];
    sprintf( buffer, ",\n      \"%s\": %.6g", name, (double) value );
    json += buffer;
}

static void
AppendJsonSignedPairs( CORE::CString& json, const char* name, const CORE::UInt8* data, size_t pairCount )
{
    // The game stores a range as ( max, min ) with signed bytes, a fixed value as ( v, v )
    json += ",\n      \"";
    json += name;
    json += "\": [";
    for ( size_t i=0; i<pairCount; ++i )
    {
        char buffer[ 32 ];
        sprintf( buffer, i > 0 ? ", [%d, %d]" : "[%d, %d]", (int) (CORE::Int8) data[ 2 * i ], (int) (CORE::Int8) data[ 2 * i + 1 ] );
        json += buffer;
    }
    json += "]";
}

/*-------------------------------------------------------------------------*/

/**
 *  Decodes the header and tail of a definition resource into named json fields.
 *  Returns an empty string if the tail does not have the size the table layout requires.
 */
static CORE::CString
DecodeDefinitionFields( const CORE::CString& resourceType ,
                        const TByteVector& payload        ,
                        size_t tailOffset                 )
{
    CORE::CString json = "{\n      \"flag\": ";
    char buffer[ 64 ];
    sprintf( buffer, "%u", (unsigned int) payload[ 0 ] );
    json += buffer;
    sprintf( buffer, ",\n      \"format\": %u", (unsigned int) payload[ 1 ] );
    json += buffer;
    AppendJsonU32( json, "pictureId", ReadLE32( payload, 2 ) );
    AppendJsonU32( json, "researchCost", ReadLE32( payload, 6 ) );
    AppendJsonU32( json, "level", ReadLE32( payload, 10 ) );

    const CORE::UInt8* tail = payload.size() > tailOffset ? &payload[ tailOffset ] : GUCEF_NULL;
    size_t tailSize = payload.size() - tailOffset;
    TByteVector tailBytes( tail, tail + tailSize );

    const SPartKind* part = GetPartKind( resourceType );
    if ( GUCEF_NULL != part )
    {
        if ( tailSize < 16 )
            return CORE::CString();
        CORE::UInt32 materialCount = ReadLE32( tailBytes, 12 );
        if ( materialCount > 1000 || tailSize != 16 + 8 * (size_t) materialCount + 4 * part->extraCount )
            return CORE::CString();

        json += CORE::CString( ",\n      \"kind\": \"" ) + part->kind + "\"";
        AppendJsonU32( json, part->powerName, ReadLE32( tailBytes, 0 ) );
        AppendJsonU32( json, "spaces", ReadLE32( tailBytes, 4 ) );
        AppendJsonU32( json, "cost", ReadLE32( tailBytes, 8 ) );
        json += ",\n      \"materials\": [";
        for ( CORE::UInt32 i=0; i<materialCount; ++i )
        {
            sprintf( buffer, "%s{ \"id\": %u, \"amount\": %u }", i > 0 ? ", " : "",
                     (unsigned int) ReadLE32( tailBytes, 16 + 4 * i ), (unsigned int) ReadLE32( tailBytes, 16 + 4 * ( materialCount + i ) ) );
            json += buffer;
        }
        json += "]";
        for ( size_t e=0; e<part->extraCount; ++e )
            AppendJsonU32( json, part->extraNames[ e ], ReadLE32( tailBytes, 16 + 8 * (size_t) materialCount + 4 * e ) );
    }
    else
    if ( "itv17" == resourceType )
    {
        if ( tailSize != 17 )
            return CORE::CString();
        json += ",\n      \"kind\": \"building\"";
        AppendJsonU32( json, "maxWorkers", ReadLE32( tailBytes, 0 ) );
        AppendJsonU32( json, "maintenanceCost", ReadLE32( tailBytes, 4 ) );
        AppendJsonU32( json, "buildingCost", ReadLE32( tailBytes, 8 ) );
        AppendJsonFloat( json, "productionFactor", ReadLE32( tailBytes, 12 ) );
        AppendJsonU32( json, "buildingType", tailBytes[ 16 ] );
    }
    else
    if ( "itv6" == resourceType || "itv7" == resourceType || "itv8" == resourceType || "itv9" == resourceType )
    {
        // All 4 tables derive from the same class which starts with a block of 30 bytes
        size_t extraSize = "itv6" == resourceType ? 2 : ( "itv8" == resourceType ? 15 : 1 );
        if ( tailSize != 30 + extraSize )
            return CORE::CString();
        const char* kind = "itv6" == resourceType ? "planet" : ( "itv7" == resourceType ? "star" : ( "itv8" == resourceType ? "race" : "material" ) );
        json += CORE::CString( ",\n      \"kind\": \"" ) + kind + "\"";
        AppendJsonSignedPairs( json, "attributes", &tailBytes[ 0 ], 15 );
        if ( "itv8" == resourceType )
        {
            AppendJsonU32( json, "unused", ReadLE32( tailBytes, 30 ) );
            AppendJsonFloat( json, "agroPerCitizen", ReadLE32( tailBytes, 34 ) );
            // Order as shown on the new game screen of the game
            static const char* traits[ 7 ] = { "aggression", "diplomacy", "production", "exploration", "espionage", "research", "colonization" };
            json += ",\n      \"traits\": {";
            for ( size_t t=0; t<7; ++t )
            {
                sprintf( buffer, "%s \"%s\": %u", t > 0 ? "," : "", traits[ t ], (unsigned int) tailBytes[ 38 + t ] );
                json += buffer;
            }
            json += " }";
        }
        else
        if ( "itv6" == resourceType )
        {
            AppendJsonU32( json, "orbitMax", tailBytes[ 30 ] );
            AppendJsonU32( json, "orbitMin", tailBytes[ 31 ] );
        }
        else
        if ( "itv7" == resourceType )
        {
            AppendJsonU32( json, "maxPlanets", tailBytes[ 30 ] );
        }
        else
        {
            AppendJsonU32( json, "rarity", tailBytes[ 30 ] );
        }
    }
    else
    {
        return CORE::CString();
    }
    json += "\n    }";
    return json;
}

/*-------------------------------------------------------------------------*/

/**
 *  Definition tables (hulls, weapons, buildings, etc) share a layout:
 *      14 bytes  header (table specific, kept as hex)
 *      u32 nameLength, name (NUL terminated)
 *      u32 descriptionLength, description (NUL terminated, starts with the name, CR LF CR LF and the text)
 *      remainder: table specific numeric data, kept as hex plus as list of u32 values when 4 byte aligned
 *  The table specific data is exported as is, its fields are not named here
 */
bool
ExtractDefinitionResource( VFS::CVFSHandle& resource             ,
                           const CORE::CString& resourceFilename ,
                           const CORE::CString& resourceType     ,
                           CORE::CString& jsonEntries            )
{GUCEF_TRACE;

    TByteVector payload;
    if ( !ReadResourcePayload( resource, payload ) )
        return false;

    const size_t prefixSize = 14;
    if ( payload.size() < prefixSize + 4 )
        return false;

    size_t pos = prefixSize;
    CORE::UInt32 nameLength = ReadLE32( payload, pos ); pos += 4;
    if ( nameLength > payload.size() - pos - 4 )
        return false;
    const CORE::UInt8* name = &payload[ pos ]; pos += nameLength;
    CORE::UInt32 descLength = ReadLE32( payload, pos ); pos += 4;
    if ( descLength > payload.size() - pos )
        return false;
    const CORE::UInt8* desc = &payload[ pos ]; pos += descLength;

    size_t nameChars = nameLength > 0 && name[ nameLength-1 ] == 0 ? nameLength-1 : nameLength;
    size_t descChars = descLength > 0 && desc[ descLength-1 ] == 0 ? descLength-1 : descLength;

    CORE::CString resourceId = resourceFilename.SubstrToChar( '.', true );
    CORE::CString entry = "  {\n";
    entry += "    \"id\": " + resourceId + ",\n";
    entry += "    \"header\": \"" + ToHex( &payload[ 0 ], prefixSize ) + "\",\n";
    entry += "    \"name\": \"" + JsonEscape( name, nameChars ) + "\",\n";
    entry += "    \"description\": \"" + JsonEscape( desc, descChars ) + "\",\n";

    CORE::CString fields = DecodeDefinitionFields( resourceType, payload, pos );
    if ( !fields.IsNULLOrEmpty() )
    {
        entry += "    \"fields\": " + fields + ",\n";
    }
    else
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Definition resource " << resourceFilename << " does not match the known layout of its table, only raw data is exported" );
    }

    size_t tailSize = payload.size() - pos;
    entry += "    \"dataSize\": " + CORE::UInt32ToString( (CORE::UInt32) tailSize ) + ",\n";
    if ( tailSize % 4 == 0 )
    {
        entry += "    \"dataU32\": [";
        for ( size_t i=0; i<tailSize; i+=4 )
        {
            if ( i > 0 )
                entry += ",";
            entry += CORE::UInt32ToString( ReadLE32( payload, pos + i ) );
        }
        entry += "],\n";
    }
    entry += "    \"dataHex\": \"" + ToHex( tailSize > 0 ? &payload[ pos ] : GUCEF_NULL, tailSize ) + "\"\n";
    entry += "  }";

    if ( !jsonEntries.IsNULLOrEmpty() )
        jsonEntries += ",\n";
    jsonEntries += entry;
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Loads the palette which all ITV images, fonts and cursors use if it has not been loaded yet
 */
bool
EnsureItvPalette( VFS::CVFS& vfs                   ,
                  const CORE::CString& archiveType ,
                  IMAGE::CImage& palette           ,
                  IMAGE::TPixelMapPtr& palettePixels )
{GUCEF_TRACE;

    if ( palette.TryGetPixelMap( 0, 0, palettePixels ) )
        return true;

    CORE::CString paletteArchive = "IV_PAL.RES";
    if ( !vfs.FileExists( paletteArchive ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to find the ITV palette archive " << paletteArchive );
        return false;
    }

    CORE::CString mount = '[' + paletteArchive + ']';
    if ( !vfs.MountArchive( mount, paletteArchive, archiveType, mount, false, false ) )
        return false;

    bool success = false;
    CORE::CString subPath = mount + '\\' + "100.itv5";
    VFS::TBasicVfsResourcePtr paletteFile = vfs.GetFile( subPath, "rb", false );
    if ( paletteFile && palette.Load( *paletteFile->GetAccess(), "itv5" ) )
    {
        palettePixels = palette.GetPixelMap();
        ExpandVgaPalette( *palettePixels );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully loaded palette for ITV images from " << subPath );
        success = true;
    }
    paletteFile.Unlink();
    vfs.UnmountArchiveByName( mount );
    return success;
}

/*-------------------------------------------------------------------------*/

/**
 *  Converts 8 bit palette indices to an RGBA pixel map. Pixels which have the transparent index get an alpha of 0.
 *  Use a negative transparentIndex for fully opaque output.
 */
IMAGE::TPixelMapPtr
CreateRgbaPixelMap( const CORE::UInt8* indices        ,
                    CORE::UInt32 width                ,
                    CORE::UInt32 height               ,
                    const IMAGE::CPixelMap& palette   ,
                    CORE::Int32 transparentIndex      )
{GUCEF_TRACE;

    const CORE::UInt8* colors = static_cast< const CORE::UInt8* >( palette.GetDataPtr() );
    CORE::UInt32 colorCount = palette.GetWidthInPixels() * palette.GetHeightInPixels();

    GUCEF::vector< CORE::UInt8 > rgba( (size_t) width * height * 4 );
    for ( size_t i=0; i<(size_t) width * height; ++i )
    {
        CORE::UInt8 index = indices[ i ];
        if ( index < colorCount )
        {
            rgba[ i*4     ] = colors[ index*3     ];
            rgba[ i*4 + 1 ] = colors[ index*3 + 1 ];
            rgba[ i*4 + 2 ] = colors[ index*3 + 2 ];
        }
        else
        {
            rgba[ i*4 ] = rgba[ i*4 + 1 ] = rgba[ i*4 + 2 ] = 0;
        }
        rgba[ i*4 + 3 ] = (CORE::Int32) index == transparentIndex ? 0 : 255;
    }
    return IMAGE::TPixelMapPtr( new IMAGE::CPixelMap( &rgba[ 0 ], width, height, IMAGE::PSF_RGBA, (IMAGE::TBuildinDataType) GUCEF_DATATYPE_UINT8 ) );
}

/*-------------------------------------------------------------------------*/

bool
SavePixelMapAsPng( const IMAGE::TPixelMapPtr& pixelMap ,
                   const CORE::CString& filePath       )
{GUCEF_TRACE;

    IMAGE::CImage image( pixelMap );
    CORE::CFileAccess file;
    if ( !file.Open( filePath, "wb" ) || !image.Save( file, "png" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save PNG to " << filePath );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Mouse cursor resource: u16 width, u16 height, width*height palette indices (0 is transparent),
 *  u16 hotspot x, u16 hotspot y. Exported as RGBA PNG, the hotspot is added to the json entry list.
 */
bool
ExtractCursorResource( VFS::CVFSHandle& resource             ,
                       const CORE::CString& resourceFilename ,
                       const CORE::CString& archiveOutputDir ,
                       const IMAGE::CPixelMap& palette       ,
                       CORE::CString& jsonEntries            )
{GUCEF_TRACE;

    TByteVector payload;
    if ( !ReadResourcePayload( resource, payload ) || payload.size() < 4 )
        return false;

    CORE::UInt32 width = ReadLE16( payload, 0 );
    CORE::UInt32 height = ReadLE16( payload, 2 );
    if ( width == 0 || height == 0 || payload.size() != 4 + (size_t) width * height + 4 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Cursor " << resourceFilename << " has an unexpected size of " << payload.size() << " bytes for " << width << "x" << height << " pixels" );
        return false;
    }

    CORE::UInt32 hotspotX = ReadLE16( payload, 4 + (size_t) width * height );
    CORE::UInt32 hotspotY = ReadLE16( payload, 4 + (size_t) width * height + 2 );

    IMAGE::TPixelMapPtr pixelMap = CreateRgbaPixelMap( &payload[ 4 ], width, height, palette, 0 );
    CORE::CString pngFilename = resourceFilename + ".png";
    if ( !SavePixelMapAsPng( pixelMap, CORE::CombinePath( archiveOutputDir, pngFilename ) ) )
        return false;

    if ( !jsonEntries.IsNULLOrEmpty() )
        jsonEntries += ",\n";
    jsonEntries += "  { \"id\": " + resourceFilename.SubstrToChar( '.', true ) + ", \"file\": \"" + pngFilename + "\", \"width\": " +
                   CORE::UInt32ToString( width ) + ", \"height\": " + CORE::UInt32ToString( height ) + ", \"hotspotX\": " +
                   CORE::UInt32ToString( hotspotX ) + ", \"hotspotY\": " + CORE::UInt32ToString( hotspotY ) + " }";
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Bitmap font resource: u16 glyph count, u16 glyph width, u16 glyph height, then per glyph
 *  u8 character code followed by width*height palette indices (0 is transparent).
 *  Exported as an RGBA PNG atlas (glyphs in a grid of 16 columns) plus a json entry that maps character codes to atlas cells.
 */
bool
ExtractFontResource( VFS::CVFSHandle& resource             ,
                     const CORE::CString& resourceFilename ,
                     const CORE::CString& archiveOutputDir ,
                     const IMAGE::CPixelMap& palette       ,
                     CORE::CString& jsonEntries            )
{GUCEF_TRACE;

    TByteVector payload;
    if ( !ReadResourcePayload( resource, payload ) || payload.size() < 6 )
        return false;

    CORE::UInt32 glyphCount = ReadLE16( payload, 0 );
    CORE::UInt32 glyphWidth = ReadLE16( payload, 2 );
    CORE::UInt32 glyphHeight = ReadLE16( payload, 4 );
    size_t glyphSize = 1 + (size_t) glyphWidth * glyphHeight;
    if ( glyphCount == 0 || glyphWidth == 0 || glyphHeight == 0 || payload.size() != 6 + glyphCount * glyphSize )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Font " << resourceFilename << " has an unexpected size of " << payload.size() << " bytes for " << glyphCount << " glyphs of " << glyphWidth << "x" << glyphHeight );
        return false;
    }

    const CORE::UInt32 columns = 16;
    CORE::UInt32 rows = ( glyphCount + columns - 1 ) / columns;
    CORE::UInt32 atlasWidth = columns * glyphWidth;
    CORE::UInt32 atlasHeight = rows * glyphHeight;
    GUCEF::vector< CORE::UInt8 > atlas( (size_t) atlasWidth * atlasHeight, 0 );

    CORE::CString glyphs;
    for ( CORE::UInt32 g=0; g<glyphCount; ++g )
    {
        const CORE::UInt8* glyph = &payload[ 6 + g * glyphSize ];
        CORE::UInt32 cellX = ( g % columns ) * glyphWidth;
        CORE::UInt32 cellY = ( g / columns ) * glyphHeight;
        for ( CORE::UInt32 y=0; y<glyphHeight; ++y )
            memcpy( &atlas[ (size_t) ( cellY + y ) * atlasWidth + cellX ], glyph + 1 + (size_t) y * glyphWidth, glyphWidth );

        if ( g > 0 )
            glyphs += ", ";
        glyphs += "{ \"code\": " + CORE::UInt32ToString( glyph[ 0 ] ) + ", \"x\": " + CORE::UInt32ToString( cellX ) + ", \"y\": " + CORE::UInt32ToString( cellY ) + " }";
    }

    IMAGE::TPixelMapPtr pixelMap = CreateRgbaPixelMap( &atlas[ 0 ], atlasWidth, atlasHeight, palette, 0 );
    CORE::CString pngFilename = resourceFilename + ".png";
    if ( !SavePixelMapAsPng( pixelMap, CORE::CombinePath( archiveOutputDir, pngFilename ) ) )
        return false;

    if ( !jsonEntries.IsNULLOrEmpty() )
        jsonEntries += ",\n";
    jsonEntries += "  {\n    \"id\": " + resourceFilename.SubstrToChar( '.', true ) + ",\n    \"atlas\": \"" + pngFilename + "\",\n    \"glyphWidth\": " +
                   CORE::UInt32ToString( glyphWidth ) + ",\n    \"glyphHeight\": " + CORE::UInt32ToString( glyphHeight ) + ",\n    \"glyphs\": [" + glyphs + "]\n  }";
    return true;
}

/*-------------------------------------------------------------------------*/

bool
IsDefinitionResourceType( const CORE::CString& dataType )
{
    // Types 6 to 9 and 11 to 17, see the IDX type table in CResArchive
    static const char* types[] = { "itv6", "itv7", "itv8", "itv9", "itv11", "itv12", "itv13", "itv14", "itv15", "itv16", "itv17" };
    for ( size_t i=0; i<sizeof(types)/sizeof(types[0]); ++i )
    {
        if ( dataType == types[ i ] )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/**
 *  Path of the file which indicates that a resource has already been extracted.
 *  Text and definition resources are collected into a single file per archive, those are always extracted.
 */
CORE::CString
GetExtractedResourcePath( const CORE::CString& resourceType   ,
                          const CORE::CString& archiveOutputDir ,
                          const CORE::CString& resourceFilename )
{GUCEF_TRACE;

    if ( "itv1" == resourceType || "itv2" == resourceType || "itv4" == resourceType )
        return CORE::CombinePath( archiveOutputDir, resourceFilename + ".png" );
    if ( "itv10" == resourceType )
        return CORE::CombinePath( archiveOutputDir, resourceFilename + ".wav" );
    if ( "itv19" == resourceType )
        return CORE::CombinePath( CORE::CombinePath( archiveOutputDir, resourceFilename.SubstrToChar( '.', true ) ), "manifest.json" );
    if ( "itv3" == resourceType || IsDefinitionResourceType( resourceType ) )
        return CORE::CString::Empty;
    return CORE::CombinePath( archiveOutputDir, resourceFilename );
}

/*-------------------------------------------------------------------------*/

bool
WriteStringToFile( const CORE::CString& filePath ,
                   const CORE::CString& content  )
{GUCEF_TRACE;

    CORE::CFileAccess file;
    if ( !file.Open( filePath, "wb" ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to open target file for writing: " << filePath );
        return false;
    }
    file.Write( content.C_String(), 1, content.Length() );
    file.Close();
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Exports an ITV FLIC animation resource as a series of lossless PNG frames
 *  plus a manifest.json which describes the animation. Layout:
 *      <archiveOutputDir>/<resourceId>/frame_000.png ... frame_NNN.png
 *      <archiveOutputDir>/<resourceId>/manifest.json
 */
bool
ExtractAnimation( VFS::CVFSHandle& resource             ,
                  const CORE::CString& resourceFilename ,
                  const CORE::CString& archiveFilename  ,
                  const CORE::CString& archiveOutputDir )
{GUCEF_TRACE;

    // The ITV codec skips the ITV resource header and decodes the FLIC data in place
    IMAGE::CImage animation;
    if ( !animation.Load( *resource.GetAccess(), "itv19", true, false ) || !animation.HasFrames() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to decode FLIC animation " << resourceFilename );
        return false;
    }

    CORE::CString resourceId = resourceFilename.SubstrToChar( '.', true );
    CORE::CString animationDir = CORE::CombinePath( archiveOutputDir, resourceId );
    if ( !CORE::CreateDirs( animationDir ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to create output directory " << animationDir );
        return false;
    }

    IMAGE::TPixelMapPtr firstFrame = animation.GetPixelMap( 0, 0 );
    CORE::CString delays;
    for ( CORE::UInt32 f=0; f<animation.GetFrameCount(); ++f )
    {
        char frameName[ 32 ];
        sprintf( frameName, "frame_%03u.png", (unsigned int) f );
        CORE::CString framePath = CORE::CombinePath( animationDir, frameName );

        IMAGE::CImage frameImage( animation.GetPixelMap( f, 0 ) );
        CORE::CFileAccess frameFile;
        if ( !frameFile.Open( framePath, "wb" ) || !frameImage.Save( frameFile, "png" ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to save animation frame to " << framePath );
            return false;
        }

        if ( f > 0 )
            delays += ",";
        delays += CORE::UInt32ToString( animation.GetFrameDurationInMs( f ) );
    }

    CORE::CString manifest = "{\n";
    manifest += "  \"sourceArchive\": \"" + archiveFilename + "\",\n";
    manifest += "  \"resource\": \"" + resourceFilename + "\",\n";
    manifest += "  \"id\": " + resourceId + ",\n";
    manifest += "  \"width\": " + CORE::UInt32ToString( firstFrame->GetWidthInPixels() ) + ",\n";
    manifest += "  \"height\": " + CORE::UInt32ToString( firstFrame->GetHeightInPixels() ) + ",\n";
    manifest += "  \"frameCount\": " + CORE::UInt32ToString( animation.GetFrameCount() ) + ",\n";
    manifest += "  \"loopCount\": " + CORE::UInt32ToString( animation.GetLoopCount() ) + ",\n";
    manifest += "  \"totalDurationMs\": " + CORE::UInt32ToString( animation.GetTotalDurationInMs() ) + ",\n";
    manifest += "  \"frameDurationsMs\": [" + delays + "],\n";
    manifest += "  \"framePattern\": \"frame_%03d.png\"\n";
    manifest += "}\n";

    if ( !WriteStringToFile( CORE::CombinePath( animationDir, "manifest.json" ), manifest ) )
        return false;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Exported " << animation.GetFrameCount() << " frames of animation " << resourceFilename << " to " << animationDir );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
LookForConfigFile( const CORE::CString& configFile )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFile );
    if ( !CORE::FileExists( configFile ) )
    {
        CORE::CString configFilePath = CORE::CombinePath( "$CURWORKDIR$", configFile );
        configFilePath = CORE::RelativePath( configFilePath );

        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
        if ( !CORE::FileExists( configFilePath ) )
        {
            configFilePath = CORE::CombinePath( "$MODULEDIR$", configFile );
            configFilePath = CORE::RelativePath( configFilePath );

            GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
            if ( !FileExists( configFilePath ) )
            {
                configFilePath = CORE::CombinePath( "$TEMPDIR$", configFile );
                configFilePath = CORE::RelativePath( configFilePath );

                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "Checking for config file @ " + configFilePath );
                if ( !FileExists( configFilePath ) )
                {
                    return CORE::CString::Empty;
                }
            }
        }

        return configFilePath;
    }

    return configFile;
}

/*-------------------------------------------------------------------------*/

bool
LoadConfig( const CORE::CString& bootstrapConfigPath   ,
            const CORE::CString& configPath            ,
            CORE::CValueList& keyValueList             ,
            CORE::CDataNode* loadedConfig = GUCEF_NULL )
{GUCEF_TRACE;

    #ifdef GUCEF_DEBUG_MODE
    const CORE::CString bootstrapConfigFile = "itvExporter_bootstrap_d.ini";
    const CORE::CString configFile = "itvExporter_d.ini";
    #else
    const CORE::CString bootstrapConfigFile = "itvExporter_bootstrap.ini";
    const CORE::CString configFile = "itvExporter.ini";
    #endif

    CORE::CConfigStore& configStore = CORE::CCoreGlobal::Instance()->GetConfigStore();

    CORE::CString bootstrapConfigFilePath = LookForConfigFile( bootstrapConfigFile );
    CORE::CString configFilePath = LookForConfigFile( configFile );

    if ( !bootstrapConfigFilePath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located bootstrap config file @ " + bootstrapConfigFilePath );
        configStore.SetBootstrapConfigFile( bootstrapConfigFilePath );
    }
    if ( !configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Located config file @ " + configFilePath );
        configStore.SetConfigFile( configFilePath );
    }
    if ( bootstrapConfigFilePath.IsNULLOrEmpty() && configFilePath.IsNULLOrEmpty() )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "Unable to locate any config file, will rely on params only" );
    }

    CORE::CGlobalConfigValueList globalCfg;
    globalCfg.SetConfigNamespace( "Main/AppArgs" );
    globalCfg.SetAllowDuplicates( false );
    globalCfg.SetAllowMultipleValues( true );

    bool loadSuccess = configStore.LoadConfig( loadedConfig );

    keyValueList = globalCfg;
    return loadSuccess;
}

/*-------------------------------------------------------------------------*/

void
ParseParams( const int argc                 ,
             char* argv[]                   ,
             CORE::CValueList& keyValueList )
{GUCEF_TRACE;

    GUCEF::CORE::CString argString;
    if ( argc > 1 )
    {
        // argv[0] is the path of the application itself, it is not a parameter and would corrupt the first real parameter
        argString = argv[ 1 ];

        // Combine the argument strings back into a single string because we don't want to use
        // a space as the deliminator
        for ( int i=2; i<argc; ++i )
        {
            argString += ' ' + CORE::CString( argv[ i ] );
        }

        // Parse the param list based on the ' symbol
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Application parameters: " + argString );
        keyValueList.SetMultiple( argString, '\'' );
    }
}

/*---------------------------------------------------------------------------*/

/*
 *      Application entry point
 */
GUCEF_OSMAIN_BEGIN
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "This tool was compiled on: " __DATE__ " @ " __TIME__ );

    CORE::CCoreGlobal::Instance();
    VFS::CVfsGlobal::Instance();
    IMAGE::CImageGlobal::Instance();

    CORE::CPlatformNativeConsoleLogger console;
    // There is no console logger when no console is available, for example when GUCEF_CONSOLE_LOGGER=none
    CORE::CIConsoleLogger* consoleLogger = console.GetLogger();
    if ( GUCEF_NULL != consoleLogger )
        CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( consoleLogger );

    // Check for config param first
    CORE::CValueList keyValueList;
    ParseParams( argc, argv, keyValueList );
    CORE::CString bootstrapConfigPathParam = keyValueList.GetValueAlways( "BootstrapConfigPath" );
    CORE::CString configPathParam = keyValueList.GetValueAlways( "configPath" );
    keyValueList.Clear();

    // Load settings from a config file (if any) and then override with params (if any)
    LoadConfig( bootstrapConfigPathParam, configPathParam, keyValueList );
    ParseParams( argc, argv, keyValueList );

    CORE::Int32 minLogLevel = CORE::LOGLEVEL_BELOW_NORMAL;
    CORE::CString valueStr = keyValueList.GetValueAlways( "minimalLogLevel" );
    if ( !valueStr.IsNULLOrEmpty() )
    {
        minLogLevel = CORE::StringToInt32( valueStr );
        CORE::CCoreGlobal::Instance()->GetLogManager().SetMinLogLevel( minLogLevel );
    }

    CORE::CString outputDir = CORE::RelativePath( keyValueList.GetValueAlways( "outputDir" ) );
    if ( outputDir.IsNULLOrEmpty() )
    {
        outputDir = CORE::RelativePath( "$CURWORKDIR$" );
    }
    CORE::CreateDirs( outputDir );

    CORE::CString logFilename = CORE::CombinePath( outputDir, "itvExporter_Log.txt" );

    keyValueList.Set( "logfile", logFilename );

    CORE::CFileAccess logFileAccess( logFilename, "w" );
    CORE::CStdLogger logger( logFileAccess );
    CORE::CCoreGlobal::Instance()->GetLogManager().AddLogger( &logger );

    CORE::CCoreGlobal::Instance()->GetLogManager().FlushBootstrapLogEntriesToLogs();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Flushed to log @ " + logFilename );

    //if ( argc > 0 )
    //{
        CORE::CString archivePath = keyValueList.GetValueAlways( "archive" );
        archivePath = archivePath.Trim( true ).Trim( false );
        if ( !archivePath.IsNULLOrEmpty() )
        {
            CORE::CString archiveType = keyValueList.GetValueAlways( "archiveType" );
            if ( archiveType.IsNULLOrEmpty() )
            {
                archiveType = CORE::ExtractFileExtention( archivePath ).Lowercase();
            }
            archiveType = archiveType.Trim( true ).Trim( false );

            bool listArchiveContents = CORE::StringToBool( keyValueList.GetValueAlways( "list", "true" ) );
            bool extractArchiveContents = CORE::StringToBool( keyValueList.GetValueAlways( "extract", "false" ) );
            bool overwrite = CORE::StringToBool( keyValueList.GetValueAlways( "overwrite", "false" ) );
            bool extractImages = CORE::StringToBool( keyValueList.GetValueAlways( "extractImages", "true" ) );
            bool extractAudio = CORE::StringToBool( keyValueList.GetValueAlways( "extractAudio", "true" ) );
            CORE::Int32 audioSampleRate = CORE::StringToInt32( keyValueList.GetValueAlways( "audioSampleRate", "22000" ) );
            bool extractText = CORE::StringToBool( keyValueList.GetValueAlways( "extractText", "true" ) );
            bool extractDefs = CORE::StringToBool( keyValueList.GetValueAlways( "extractDefs", "true" ) );
            bool extractFonts = CORE::StringToBool( keyValueList.GetValueAlways( "extractFonts", "true" ) );
            bool extractCursors = CORE::StringToBool( keyValueList.GetValueAlways( "extractCursors", "true" ) );

            // Palette index which should become transparent in exported images, a negative value (default) keeps images opaque.
            // The data does not tell whether index 0 is a color key for a given image so this is left up to the user,
            // for example set it to 0 and export the sprite archives separately.
            CORE::Int32 transparentIndex = CORE::StringToInt32( keyValueList.GetValueAlways( "transparentIndex", "-1" ) );
            bool extractAnimations = CORE::StringToBool( keyValueList.GetValueAlways( "extractAnimations", "true" ) );
            bool extractOther = CORE::StringToBool( keyValueList.GetValueAlways( "extractOther", "true" ) );

            CORE::CString archiveFilename = CORE::ExtractFilename( archivePath );
            CORE::CString archiveDir = CORE::StripFilename( archivePath );

            VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();

            if ( archiveDir.IsNULLOrEmpty() )
            {
                archiveDir = CORE::RelativePath( "$CURWORKDIR$" );
            }

            // Add the archive dir as a VFS root named root
            vfs.AddRoot( archiveDir, "root", false, false );

            VFS::CVFS::TStringVector archiveFileNames;
            if ( -1 < archiveFilename.HasChar( '*' ) )
            {
                // Use the filename as a filter
                vfs.GetFileList( archiveFileNames, CORE::CString::Empty, false, true, archiveFilename );
            }
            else
            {
                // directly use the filename
                archiveFileNames.push_back( archiveFilename );
            }

            GUCEF::IMAGE::CImage palette;

            // Batch extract each of the archives
            VFS::CVFS::TStringVector::iterator n = archiveFileNames.begin();
            while ( n != archiveFileNames.end() )
            {
                archiveFilename = (*n);

                if ( vfs.FileExists( archiveFilename ) )
                {
                    CORE::CString mount = '[' + archiveFilename + ']';

                    // Mount the archive
                    if ( vfs.MountArchive( mount, archiveFilename, archiveType, mount, false, false ) )
                    {
                        if ( listArchiveContents )
                        {
                            // Get the contents of the archive
                            VFS::CVFS::TStringVector archiveContent;
                            vfs.GetFileList( archiveContent, mount, true, true, CORE::CString::Empty );

                            // Write the contents to the log which includes the console
                            if ( GUCEF_NULL != consoleLogger )
                                consoleLogger->SetFormatAsConsoleUI( true );
                            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_NORMAL, "***** BEGIN ARCHIVE CONTENTS *****" );
                            CORE::UInt32 n=1;
                            VFS::CVFS::TStringVector::iterator i = archiveContent.begin();
                            while ( i != archiveContent.end() )
                            {
                                GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_NORMAL, CORE::UInt32ToString( n ) + ": " + (*i) );
                                ++i; ++n;
                            }
                            GUCEF_CONSOLE_LOG( CORE::LOGLEVEL_NORMAL, "***** END ARCHIVE CONTENTS *****" );
                            if ( GUCEF_NULL != consoleLogger )
                                consoleLogger->SetFormatAsConsoleUI( false );
                        }

                        if ( extractArchiveContents )
                        {
                            // Get the contents of the archive
                            VFS::CVFS::TStringVector archiveContent;
                            vfs.GetFileList( archiveContent, mount, true, true, CORE::CString::Empty );

                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "archive contains " + CORE::ToString( archiveContent.size() ) + " entries" );

                            // prepare the output folder
                            CORE::CString archiveOutputDir = archiveFilename.ReplaceChar( '.', '_' );
                            archiveOutputDir = CORE::CombinePath( outputDir, archiveOutputDir );
                            if ( CORE::CreateDirs( archiveOutputDir ) )
                            {
                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Output path " + archiveOutputDir + " is available for extraction" );

                                CORE::CString textJsonEntries, textLines, defJsonEntries, fontJsonEntries, cursorJsonEntries;

                                // Obtain access to each of the resources in the archive and extract it
                                VFS::CVFS::TStringVector::iterator i = archiveContent.begin();
                                while ( i != archiveContent.end() )
                                {
                                    CORE::CString filename = CORE::ExtractFilename( (*i) );
                                    CORE::CString vfsFilePath = CORE::CombinePath( mount, filename );

                                    CORE::CString extractedFilePath = filename;
                                    extractedFilePath = CORE::CombinePath( archiveOutputDir, extractedFilePath );

                                    if ( !overwrite )
                                    {
                                        // Check if the destination already exists, which depends on what the resource gets converted to
                                        CORE::CString resourceType = CORE::ExtractFileExtention( filename );
                                        CORE::CString destinationPath = GetExtractedResourcePath( resourceType, archiveOutputDir, filename );
                                        if ( !destinationPath.IsNULLOrEmpty() && CORE::FileExists( destinationPath ) )
                                        {
                                            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Skipping extraction of file since the destination path already exists and 'overwrite=false'. Path: " << destinationPath );
                                            ++i;
                                            continue;
                                        }
                                    }

                                    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsFilePath, "rb", false );
                                    if ( !file.IsNULL() )
                                    {
                                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Obtained access to resource: " + (*i) );

                                        CORE::CString dataType = CORE::ExtractFileExtention( (*i) );
                                        if ( "itv10" == dataType )
                                        {
                                            if ( extractAudio )
                                            {
                                                extractedFilePath = extractedFilePath + ".wav";
                                                CORE::CFileAccess extractedFile;
                                                if ( extractedFile.Open( extractedFilePath, "wb" ) )
                                                {
                                                    bool wroteWave = WriteITVPcm( *file->GetAccess(), extractedFile, audioSampleRate );
                                                    extractedFile.Close();
                                                    if ( wroteWave )
                                                    {
                                                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully wrote PCM audio as WAVE file to: " << extractedFilePath );
                                                    }
                                                    else
                                                    {
                                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to write PCM audio as WAVE file to: " << extractedFilePath );
                                                    }
                                                }
                                                else
                                                {
                                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to open target file for writing: " + extractedFilePath );
                                                }
                                            }
                                        }
                                        else
                                        if ( "itv4" == dataType )
                                        {
                                            IMAGE::TPixelMapPtr palettePixels;
                                            if ( extractFonts && EnsureItvPalette( vfs, archiveType, palette, palettePixels ) )
                                                ExtractFontResource( *file, filename, archiveOutputDir, *palettePixels, fontJsonEntries );
                                        }
                                        else
                                        if ( "itv2" == dataType )
                                        {
                                            IMAGE::TPixelMapPtr palettePixels;
                                            if ( extractCursors && EnsureItvPalette( vfs, archiveType, palette, palettePixels ) )
                                                ExtractCursorResource( *file, filename, archiveOutputDir, *palettePixels, cursorJsonEntries );
                                        }
                                        else
                                        if ( "itv3" == dataType )
                                        {
                                            if ( extractText )
                                                ExtractTextResource( *file, filename, textJsonEntries, textLines );
                                        }
                                        else
                                        if ( IsDefinitionResourceType( dataType ) )
                                        {
                                            if ( extractDefs )
                                            {
                                                if ( !ExtractDefinitionResource( *file, filename, dataType, defJsonEntries ) )
                                                {
                                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to decode definition resource " << filename );
                                                }
                                            }
                                        }
                                        else
                                        if ( "itv19" == dataType )
                                        {
                                            if ( extractAnimations )
                                            {
                                                ExtractAnimation( *file                                    ,
                                                                  CORE::ExtractFilename( (*i) )            ,
                                                                  archiveFilename                          ,
                                                                  archiveOutputDir                         );
                                            }
                                        }
                                        else
                                        if ( "itv1" == dataType )
                                        {
                                            if ( extractImages )
                                            {
                                                // Images are extracted as a common format instead
                                                IMAGE::CImage image;
                                                if ( image.Load( *file->GetAccess(), dataType ) )
                                                {
                                                    // lazy load pallette if neded
                                                    IMAGE::TPixelMapPtr palettePixels;
                                                    if ( EnsureItvPalette( vfs, archiveType, palette, palettePixels ) )
                                                    {
                                                        // Note: ITV image data is stored top-down, no flipping is needed
                                                        extractedFilePath = extractedFilePath + ".png";
                                                        IMAGE::TPixelMapPtr indices = image.GetPixelMap();
                                                        if ( transparentIndex >= 0 )
                                                        {
                                                            // Apply the palette ourselves so that we can control the alpha channel
                                                            IMAGE::TPixelMapPtr rgba = CreateRgbaPixelMap( static_cast< const CORE::UInt8* >( indices->GetDataPtr() ) ,
                                                                                                          indices->GetWidthInPixels()                                  ,
                                                                                                          indices->GetHeightInPixels()                                 ,
                                                                                                          *palettePixels                                               ,
                                                                                                          transparentIndex                                             );
                                                            if ( SavePixelMapAsPng( rgba, extractedFilePath ) )
                                                            {
                                                                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully coverted image to PNG saved to " << extractedFilePath );
                                                            }
                                                        }
                                                        else
                                                        // Apply the palette so that we can save in another format
                                                        if ( indices->ApplyPalette( palettePixels ) )
                                                        {
                                                            CORE::CFileAccess extractedFile;
                                                            if ( extractedFile.Open( extractedFilePath, "wb" ) )
                                                            {
                                                                if ( image.Save( extractedFile, "png" ) )
                                                                {
                                                                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Successfully coverted image to PNG saved to " << extractedFilePath );
                                                                }
                                                            }
                                                        }
                                                        else
                                                        {
                                                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to apply palette to image " << file->GetFilename() );
                                                        }
                                                    }
                                                    else
                                                    {
                                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Failed to obtain palette for image " << file->GetFilename() );
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if ( extractOther )
                                            {
                                                CORE::CFileAccess extractedFile;
                                                if ( extractedFile.Open( extractedFilePath, "wb" ) )
                                                {
                                                    CORE::IOAccessPtr fileAccess = file->GetAccess();
                                                    CORE::UInt64 fileSize = fileAccess->GetSize();
                                                    CORE::UInt64 bytesExtracted = extractedFile.Write( *fileAccess );
                                                    extractedFile.Close();

                                                    if ( fileSize == bytesExtracted )
                                                    {
                                                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Extracted " + CORE::ToString( bytesExtracted ) + " bytes into file: " + extractedFilePath );
                                                    }
                                                    else
                                                    {
                                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Extracted " + CORE::ToString( bytesExtracted ) + " bytes instead of " +
                                                                 CORE::ToString( fileSize ) + " expected bytes into file: " + extractedFilePath + ". The file is probably corrupt" );
                                                    }
                                                }
                                                else
                                                {
                                                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to open target file for writing: " + extractedFilePath );
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to obtain access to resource: " + filename );
                                    }
                                    ++i;
                                }

                                if ( !textJsonEntries.IsNULLOrEmpty() )
                                {
                                    WriteStringToFile( CORE::CombinePath( archiveOutputDir, "text.json" ), "[\n" + textJsonEntries + "\n]\n" );
                                    WriteStringToFile( CORE::CombinePath( archiveOutputDir, "text.txt" ), textLines );
                                }
                                if ( !fontJsonEntries.IsNULLOrEmpty() )
                                {
                                    WriteStringToFile( CORE::CombinePath( archiveOutputDir, "fonts.json" ), "[\n" + fontJsonEntries + "\n]\n" );
                                }
                                if ( !cursorJsonEntries.IsNULLOrEmpty() )
                                {
                                    WriteStringToFile( CORE::CombinePath( archiveOutputDir, "cursors.json" ), "[\n" + cursorJsonEntries + "\n]\n" );
                                }
                                if ( !defJsonEntries.IsNULLOrEmpty() )
                                {
                                    WriteStringToFile( CORE::CombinePath( archiveOutputDir, "definitions.json" ), "[\n" + defJsonEntries + "\n]\n" );
                                }
                            }
                        }

                        if ( !vfs.UnmountArchiveByName( mount ) )
                        {
                            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Unable to unmount archive" );
                            break;
                        }
                    }
                    else
                    {
                        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Failed to mount archive" );
                    }
                }
                else
                {
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Archive file does not exist, cannot proceed" );
                }

                ++n;
            }
        }
        else
        {
            GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Missing minimal param \"archive\" thus no idea which archive to use" );
        }
    //}

	GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "Wrote log file to: " + logFilename );

    CORE::CCoreGlobal::Instance()->GetPluginControl().UnloadAll();
    CORE::CCoreGlobal::Instance()->GetLogManager().ClearLoggers();    

    return 0;
}
GUCEF_OSMAIN_END

/*-------------------------------------------------------------------------//
//                                                                         //
//      Info & Changes                                                     //
//                                                                         //
//-------------------------------------------------------------------------//

- 25-08-2015 :
        - Dinand: Initial implementation

---------------------------------------------------------------------------*/
