/*
 *  gucefIMAGE: GUCEF module providing image utilities
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

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_CORE_COREcodectypes_H
#include "gucefCORE_CoreCodecTypes.h"
#define GUCEF_CORE_COREcodectypes_H
#endif /* GUCEF_CORE_COREcodectypes_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_IMAGE_CPNGIMAGECODEC_H
#include "gucefIMAGE_CPngImageCodec.h"
#define GUCEF_IMAGE_CPNGIMAGECODEC_H
#endif /* GUCEF_IMAGE_CPNGIMAGECODEC_H ? */

#include "gucefIMAGE_CGupImgImageCodec.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace IMAGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CString CGupImgImageCodec::CodecFamilyName = "ImageCodec";
const CString CGupImgImageCodec::CodecTypeName = "gupimg";
const CString CGupImgImageCodec::DefaultCompressionCodecName = "zlib";
const CString CGupImgImageCodec::CompressionParamName = "compression";
const CString CGupImgImageCodec::ChecksumsParamName = "checksums";
const CString CGupImgImageCodec::NoCompressionName = "none";
const UInt16 CGupImgImageCodec::FormatVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const UInt8 FileSignature[ 8 ] = { 0x47, 0x55, 0x50, 0x49, 0x4D, 0x47, 0x1A, 0x0A };
const UInt32 HeaderSize = 32;

const UInt32 FileFlagChecksums = 0x01;
const UInt32 FileFlagRowsBottomToTop = 0x02;
const UInt32 KnownFileFlags = FileFlagChecksums | FileFlagRowsBottomToTop;

const UInt32 LevelFlagNormalized = 0x01;
const UInt32 KnownLevelFlags = LevelFlagNormalized;

/** What a file may claim before it is refused without looking further: a corrupt size must not become an allocation */
const UInt32 MaxFrames = 1048576;
const UInt32 MaxLevelsPerFrame = 64;
const UInt32 MaxSectionSize = 268435456;
const UInt32 MaxLevelSize = 1073741824;
const UInt32 MaxCodecNameLength = 256;

/*-------------------------------------------------------------------------*/

void
AppendU16( CORE::CDynamicBuffer& buffer, UInt16 value )
{GUCEF_TRACE;

    UInt8 bytes[ 2 ];
    bytes[ 0 ] = (UInt8) value;
    bytes[ 1 ] = (UInt8) ( value >> 8 );
    buffer.Append( bytes, 2 );
}

/*-------------------------------------------------------------------------*/

void
AppendU32( CORE::CDynamicBuffer& buffer, UInt32 value )
{GUCEF_TRACE;

    UInt8 bytes[ 4 ];
    bytes[ 0 ] = (UInt8) value;
    bytes[ 1 ] = (UInt8) ( value >> 8 );
    bytes[ 2 ] = (UInt8) ( value >> 16 );
    bytes[ 3 ] = (UInt8) ( value >> 24 );
    buffer.Append( bytes, 4 );
}

/*-------------------------------------------------------------------------*/

void
AppendU64( CORE::CDynamicBuffer& buffer, UInt64 value )
{GUCEF_TRACE;

    AppendU32( buffer, (UInt32) value );
    AppendU32( buffer, (UInt32) ( value >> 32 ) );
}

/*-------------------------------------------------------------------------*/

void
AppendText( CORE::CDynamicBuffer& buffer, const CString& text )
{GUCEF_TRACE;

    AppendU32( buffer, text.Length() );
    if ( text.Length() > 0 )
        buffer.Append( text.C_String(), text.Length() );
}

/*-------------------------------------------------------------------------*/

/** Reads from a block of memory, every read checks that the bytes are there */
class CMemoryReader
{
    public:

    CMemoryReader( const UInt8* data, UInt32 size ) : m_data( data ), m_size( size ), m_position( 0 ) {}

    bool ReadU16( UInt16& value )
    {
        if ( m_size - m_position < 2 )
            return false;
        value = (UInt16) ( m_data[ m_position ] | ( m_data[ m_position + 1 ] << 8 ) );
        m_position += 2;
        return true;
    }

    bool ReadU32( UInt32& value )
    {
        if ( m_size - m_position < 4 )
            return false;
        const UInt8* p = m_data + m_position;
        value = (UInt32) p[ 0 ] | ( (UInt32) p[ 1 ] << 8 ) | ( (UInt32) p[ 2 ] << 16 ) | ( (UInt32) p[ 3 ] << 24 );
        m_position += 4;
        return true;
    }

    bool ReadI32( Int32& value )
    {
        UInt32 raw = 0;
        if ( !ReadU32( raw ) )
            return false;
        value = (Int32) raw;
        return true;
    }

    bool ReadU64( UInt64& value )
    {
        UInt32 low = 0, high = 0;
        if ( !ReadU32( low ) || !ReadU32( high ) )
            return false;
        value = (UInt64) low | ( (UInt64) high << 32 );
        return true;
    }

    bool ReadText( CString& text, UInt32 length )
    {
        if ( m_size - m_position < length )
            return false;
        text.Set( reinterpret_cast< const char* >( m_data + m_position ), length );
        m_position += length;
        return true;
    }

    bool IsAtEnd( void ) const { return m_position == m_size; }

    private:

    const UInt8* m_data;
    UInt32 m_size;
    UInt32 m_position;
};

/*-------------------------------------------------------------------------*/

struct SFrameEntry
{
    UInt32 durationInMs;
    UInt32 levelCount;
};

struct SLevelEntry
{
    UInt32 width;
    UInt32 height;
    Int32 pixelStorageFormat;
    Int32 componentDataType;
    UInt32 flags;
    UInt32 rawSize;
    UInt32 storedSize;
    UInt64 offset;
    UInt32 crc;
    CString compressionCodec;
};

typedef GUCEF::vector< SFrameEntry > TFrameEntryVector;
typedef GUCEF::vector< SLevelEntry > TLevelEntryVector;

/** Everything of the file before the data section */
struct SFileInfo
{
    UInt32 flags;
    UInt32 loopCount;
    TFrameEntryVector frames;
    TLevelEntryVector levels;       /**< all levels, frame by frame */

    SFileInfo( void ) : flags( 0 ), loopCount( 0 ), frames(), levels() {}
};

/*-------------------------------------------------------------------------*/

bool
ReadExactly( CORE::CIOAccess& input, CORE::CDynamicBuffer& buffer, UInt32 size )
{GUCEF_TRACE;

    buffer.SetDataSize( size );
    if ( 0 == size )
        return true;
    return 1 == input.Read( buffer.GetBufferPtr(), size, 1 );
}

/*-------------------------------------------------------------------------*/

/** The size in bytes of the pixels of a level as its entry describes them, 0 when the description is not valid */
UInt64
GetPixelBytesOfLevel( const SLevelEntry& level )
{GUCEF_TRACE;

    if ( level.pixelStorageFormat <= PSF_UNKNOWN || level.pixelStorageFormat >= PSF_COUNT )
        return 0;
    const UInt32 channels = CPixelMap::GetChannelCountForFormat( (TPixelStorageFormat) level.pixelStorageFormat );
    const UInt32 channelSize = CPixelMap::GetPixelChannelSize( (TBuildinDataType) level.componentDataType );
    if ( 0 == channels || 0 == channelSize || 0 == level.width || 0 == level.height )
        return 0;
    return (UInt64) level.width * level.height * channels * channelSize;
}

/*-------------------------------------------------------------------------*/

bool
ReadMetaDataSection( const CORE::CDynamicBuffer& section, CORE::CValueList* metaData, CString& problem )
{GUCEF_TRACE;

    CMemoryReader reader( static_cast< const UInt8* >( section.GetConstBufferPtr() ), section.GetDataSize() );
    UInt32 keyCount = 0;
    if ( !reader.ReadU32( keyCount ) )
    {
        problem = "the meta-data section is incomplete";
        return false;
    }
    for ( UInt32 k=0; k<keyCount; ++k )
    {
        UInt32 length = 0;
        CString key;
        UInt32 valueCount = 0;
        if ( !reader.ReadU32( length ) || !reader.ReadText( key, length ) || !reader.ReadU32( valueCount ) )
        {
            problem = "the meta-data section is incomplete";
            return false;
        }
        for ( UInt32 v=0; v<valueCount; ++v )
        {
            CString value;
            if ( !reader.ReadU32( length ) || !reader.ReadText( value, length ) )
            {
                problem = "the meta-data section is incomplete";
                return false;
            }
            if ( GUCEF_NULL != metaData )
                metaData->Set( key, CORE::CVariant( value ) );
        }
    }
    if ( !reader.IsAtEnd() )
    {
        problem = "the meta-data section has bytes after its last entry";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

/**
 *  Reads the header, the table section and the meta-data section, which brings the input to the start of the data section.
 *  'metaData' may be NULL when it is not wanted. On failure 'problem' says why.
 */
bool
ReadFileInfo( CORE::CIOAccess& input, SFileInfo& info, CORE::CValueList* metaData, CString& problem )
{GUCEF_TRACE;

    CORE::CDynamicBuffer headerBytes;
    if ( !ReadExactly( input, headerBytes, HeaderSize ) )
    {
        problem = "the input is too short to be a gupimg file";
        return false;
    }
    const UInt8* header = static_cast< const UInt8* >( headerBytes.GetConstBufferPtr() );
    if ( 0 != memcmp( header, FileSignature, sizeof( FileSignature ) ) )
    {
        problem = "the input is not a gupimg file: the signature does not match";
        return false;
    }

    CMemoryReader reader( header + 8, HeaderSize - 8 );
    UInt16 version = 0, headerSize = 0;
    UInt32 frameCount = 0, tableSize = 0, metaDataSize = 0;
    reader.ReadU16( version );
    reader.ReadU16( headerSize );
    reader.ReadU32( info.flags );
    reader.ReadU32( frameCount );
    reader.ReadU32( info.loopCount );
    reader.ReadU32( tableSize );
    reader.ReadU32( metaDataSize );

    if ( version < 1 || version > CGupImgImageCodec::FormatVersion )
    {
        problem = "the gupimg file has format version " + CORE::UInt32ToString( version ) + ", this reader knows up to version " +
                  CORE::UInt32ToString( CGupImgImageCodec::FormatVersion );
        return false;
    }
    if ( headerSize < HeaderSize )
    {
        problem = "the gupimg file states a header size that is smaller than the header";
        return false;
    }
    if ( 0 != ( info.flags & ~KnownFileFlags ) )
    {
        problem = "the gupimg file uses flags that this reader does not know";
        return false;
    }
    if ( 0 == frameCount || frameCount > MaxFrames )
    {
        problem = "the gupimg file has " + CORE::UInt32ToString( frameCount ) + " frames, which is not a valid number";
        return false;
    }
    if ( tableSize > MaxSectionSize || metaDataSize > MaxSectionSize )
    {
        problem = "the gupimg file has a table or meta-data section that is too large";
        return false;
    }
    if ( headerSize > HeaderSize )
    {
        CORE::CDynamicBuffer skipped;
        if ( !ReadExactly( input, skipped, (UInt32) ( headerSize - HeaderSize ) ) )
        {
            problem = "the header of the gupimg file is incomplete";
            return false;
        }
    }

    CORE::CDynamicBuffer tables;
    if ( !ReadExactly( input, tables, tableSize ) )
    {
        problem = "the table section of the gupimg file is incomplete";
        return false;
    }
    CMemoryReader tableReader( static_cast< const UInt8* >( tables.GetConstBufferPtr() ), tables.GetDataSize() );

    info.frames.clear();
    info.levels.clear();
    UInt64 totalLevels = 0;
    for ( UInt32 f=0; f<frameCount; ++f )
    {
        SFrameEntry frame;
        if ( !tableReader.ReadU32( frame.durationInMs ) || !tableReader.ReadU32( frame.levelCount ) )
        {
            problem = "the frame table of the gupimg file is incomplete";
            return false;
        }
        if ( 0 == frame.levelCount || frame.levelCount > MaxLevelsPerFrame )
        {
            problem = "a frame of the gupimg file has " + CORE::UInt32ToString( frame.levelCount ) + " levels, which is not a valid number";
            return false;
        }
        totalLevels += frame.levelCount;
        info.frames.push_back( frame );
    }

    UInt64 expectedOffset = 0;
    for ( UInt32 f=0; f<frameCount; ++f )
    {
        for ( UInt32 l=0; l<info.frames[ f ].levelCount; ++l )
        {
            SLevelEntry level;
            UInt16 nameLength = 0;
            if ( !tableReader.ReadU32( level.width ) || !tableReader.ReadU32( level.height )                  ||
                 !tableReader.ReadI32( level.pixelStorageFormat ) || !tableReader.ReadI32( level.componentDataType ) ||
                 !tableReader.ReadU32( level.flags ) || !tableReader.ReadU32( level.rawSize )                   ||
                 !tableReader.ReadU32( level.storedSize ) || !tableReader.ReadU64( level.offset )               ||
                 !tableReader.ReadU32( level.crc ) || !tableReader.ReadU16( nameLength )                        ||
                 !tableReader.ReadText( level.compressionCodec, nameLength )                                    )
            {
                problem = "the level table of the gupimg file is incomplete";
                return false;
            }
            if ( nameLength > MaxCodecNameLength || 0 != ( level.flags & ~KnownLevelFlags ) )
            {
                problem = "a level of the gupimg file has a name or flags that are not valid";
                return false;
            }
            const UInt64 pixelBytes = GetPixelBytesOfLevel( level );
            if ( 0 == pixelBytes || pixelBytes > MaxLevelSize || pixelBytes != level.rawSize )
            {
                problem = "a level of the gupimg file describes pixels that do not add up: " + CORE::UInt32ToString( level.width ) + " x " +
                          CORE::UInt32ToString( level.height ) + " with " + CORE::UInt32ToString( level.rawSize ) + " bytes";
                return false;
            }
            if ( level.storedSize > MaxLevelSize || ( level.compressionCodec.IsNULLOrEmpty() && level.storedSize != level.rawSize ) )
            {
                problem = "a level of the gupimg file has a stored size that is not valid";
                return false;
            }
            if ( level.offset != expectedOffset )
            {
                problem = "the levels of the gupimg file are not stored back to back in the order of the table";
                return false;
            }
            expectedOffset += level.storedSize;
            info.levels.push_back( level );
        }
    }
    if ( !tableReader.IsAtEnd() )
    {
        problem = "the table section of the gupimg file has bytes after its last entry";
        return false;
    }

    CORE::CDynamicBuffer metaDataBytes;
    if ( !ReadExactly( input, metaDataBytes, metaDataSize ) )
    {
        problem = "the meta-data section of the gupimg file is incomplete";
        return false;
    }
    if ( metaDataSize > 0 )
        return ReadMetaDataSection( metaDataBytes, metaData, problem );
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CCodecRegistry::TICodecPtr
GetCompressionCodec( const CString& name )
{GUCEF_TRACE;

    return CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryGetCodec( CORE::CoreCodecTypes::CompressionCodec, name );
}

/*-------------------------------------------------------------------------*/

struct SStoredLevel
{
    CORE::CDynamicBuffer bytes;
    CString compressionCodec;
    UInt32 rawSize;
    UInt32 crc;
};

typedef GUCEF::vector< SStoredLevel > TStoredLevelVector;

} /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGupImgImageCodec::CGupImgImageCodec( void )
    : CIImageCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGupImgImageCodec::CGupImgImageCodec( const CGupImgImageCodec& src )
    : CIImageCodec( src )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGupImgImageCodec::~CGupImgImageCodec()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGupImgImageCodec&
CGupImgImageCodec::operator=( const CGupImgImageCodec& src )
{GUCEF_TRACE;

    if ( &src != this )
        CIImageCodec::operator=( src );
    return *this;
}

/*-------------------------------------------------------------------------*/

bool
CGupImgImageCodec::Encode( const CImage& inputImage       ,
                           CORE::CIOAccess& encodedOutput )
{GUCEF_TRACE;

    return Encode( inputImage, encodedOutput, CORE::CValueList() );
}

/*-------------------------------------------------------------------------*/

bool
CGupImgImageCodec::Encode( const CImage& inputImage       ,
                           CORE::CIOAccess& encodedOutput ,
                           const CORE::CValueList& params )
{GUCEF_TRACE;

    const UInt32 frameCount = inputImage.GetFrameCount();
    if ( !inputImage.HasFrames() || 0 == frameCount )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The image has no frames, there is nothing to write" );
        return false;
    }

    // What to compress with: a name that is given has to be there, the default is used when it is there
    CString compressionName = DefaultCompressionCodecName;
    bool compressionGiven = false;
    CORE::CVariant compressionParam;
    if ( params.TryGetValue( CompressionParamName, compressionParam ) )
    {
        compressionName = compressionParam.AsString( DefaultCompressionCodecName );
        compressionGiven = true;
    }
    if ( compressionName.IsNULLOrEmpty() || compressionName.Equals( NoCompressionName, false ) )
        compressionName.Clear();
    const bool checksums = params.GetValueAlways( ChecksumsParamName, true ).AsBool( true );

    CORE::CCodecRegistry::TICodecPtr compressor;
    if ( !compressionName.IsNULLOrEmpty() )
    {
        compressor = GetCompressionCodec( compressionName );
        if ( compressor.IsNULL() )
        {
            if ( compressionGiven )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The compression codec \"" << compressionName << "\" is not available" );
                return false;
            }
            compressionName.Clear();
        }
    }

    // Compress every level on its own and collect what the tables need
    CORE::CDynamicBuffer frameTable;
    CORE::CDynamicBuffer levelTable;
    TStoredLevelVector stored;
    UInt64 dataOffset = 0;
    for ( UInt32 f=0; f<frameCount; ++f )
    {
        const CImage::TMipMapList& levels = inputImage.GetFrame( f );
        if ( levels.empty() || levels.size() > MaxLevelsPerFrame )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: Frame " << f << " has " << (UInt32) levels.size() << " mipmap levels, which cannot be written" );
            return false;
        }
        AppendU32( frameTable, inputImage.GetFrameDurationInMs( f ) );
        AppendU32( frameTable, (UInt32) levels.size() );

        for ( size_t l=0; l<levels.size(); ++l )
        {
            const TPixelMapPtr& pixels = levels[ l ];
            if ( pixels.IsNULL() )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: Frame " << f << " level " << (UInt32) l << " has no pixels" );
                return false;
            }

            SLevelEntry entry;
            entry.width = pixels->GetWidthInPixels();
            entry.height = pixels->GetHeightInPixels();
            entry.pixelStorageFormat = (Int32) pixels->GetPixelStorageFormat();
            entry.componentDataType = (Int32) pixels->GetPixelComponentDataType();
            entry.flags = pixels->IsNormalizedAsPercentage() ? LevelFlagNormalized : 0;
            entry.rawSize = pixels->GetTotalSizeInBytes();
            const UInt64 expected = GetPixelBytesOfLevel( entry );
            if ( 0 == expected || expected != entry.rawSize || expected > MaxLevelSize )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: Frame " << f << " level " << (UInt32) l << " has pixels that cannot be written: " <<
                                 entry.width << " x " << entry.height << ", format " << entry.pixelStorageFormat << ", type " << entry.componentDataType );
                return false;
            }

            stored.push_back( SStoredLevel() );
            SStoredLevel& level = stored.back();
            level.rawSize = entry.rawSize;

            CORE::CDynamicBuffer raw;
            raw.LinkTo( pixels->GetDataPtr(), entry.rawSize );
            if ( !compressor.IsNULL() )
            {
                CORE::CDynamicBufferAccess in( raw );
                CORE::CDynamicBufferAccess out( &level.bytes, false );
                if ( compressor->Encode( in, out ) && level.bytes.GetDataSize() > 0 && level.bytes.GetDataSize() < entry.rawSize )
                    level.compressionCodec = compressionName;
                else
                    level.bytes.Clear();
            }
            if ( level.compressionCodec.IsNULLOrEmpty() )
                level.bytes.CopyFrom( entry.rawSize, raw.GetConstBufferPtr() );
            level.crc = checksums ? CPngImageCodec::Crc32( static_cast< const UInt8* >( level.bytes.GetConstBufferPtr() ), level.bytes.GetDataSize() ) : 0;

            AppendU32( levelTable, entry.width );
            AppendU32( levelTable, entry.height );
            AppendU32( levelTable, (UInt32) entry.pixelStorageFormat );
            AppendU32( levelTable, (UInt32) entry.componentDataType );
            AppendU32( levelTable, entry.flags );
            AppendU32( levelTable, entry.rawSize );
            AppendU32( levelTable, level.bytes.GetDataSize() );
            AppendU64( levelTable, dataOffset );
            AppendU32( levelTable, level.crc );
            AppendU16( levelTable, (UInt16) level.compressionCodec.Length() );
            if ( level.compressionCodec.Length() > 0 )
                levelTable.Append( level.compressionCodec.C_String(), level.compressionCodec.Length() );
            dataOffset += level.bytes.GetDataSize();
        }
    }

    // The meta-data of the image, as text
    CORE::CDynamicBuffer metaDataSection;
    const CORE::CValueList& metaData = inputImage.GetMetaData();
    const UInt32 keyCount = metaData.GetKeyCount();
    if ( keyCount > 0 )
    {
        AppendU32( metaDataSection, keyCount );
        for ( UInt32 k=0; k<keyCount; ++k )
        {
            AppendText( metaDataSection, metaData.GetKey( k ).AsString() );
            const CORE::CValueList::TVariantVector& values = metaData.GetValueVector( k );
            AppendU32( metaDataSection, (UInt32) values.size() );
            for ( size_t v=0; v<values.size(); ++v )
                AppendText( metaDataSection, values[ v ].AsString() );
        }
    }

    const UInt32 tableSize = frameTable.GetDataSize() + levelTable.GetDataSize();
    if ( tableSize > MaxSectionSize || metaDataSection.GetDataSize() > MaxSectionSize )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The tables or the meta-data of the image are too large to write" );
        return false;
    }

    CORE::CDynamicBuffer header;
    header.Append( FileSignature, sizeof( FileSignature ) );
    AppendU16( header, FormatVersion );
    AppendU16( header, (UInt16) HeaderSize );
    AppendU32( header, checksums ? FileFlagChecksums : 0 );
    AppendU32( header, frameCount );
    AppendU32( header, inputImage.GetLoopCount() );
    AppendU32( header, tableSize );
    AppendU32( header, metaDataSection.GetDataSize() );

    // One write per section, so a failing output shows where it stopped
    bool success = 1 == encodedOutput.Write( header.GetConstBufferPtr(), header.GetDataSize(), 1 ) &&
                   1 == encodedOutput.Write( frameTable.GetConstBufferPtr(), frameTable.GetDataSize(), 1 ) &&
                   1 == encodedOutput.Write( levelTable.GetConstBufferPtr(), levelTable.GetDataSize(), 1 );
    if ( success && metaDataSection.GetDataSize() > 0 )
        success = 1 == encodedOutput.Write( metaDataSection.GetConstBufferPtr(), metaDataSection.GetDataSize(), 1 );
    for ( size_t i=0; success && i<stored.size(); ++i )
        success = 1 == encodedOutput.Write( stored[ i ].bytes.GetConstBufferPtr(), stored[ i ].bytes.GetDataSize(), 1 );
    if ( !success )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The output refused the bytes of the image" );
    return success;
}

/*-------------------------------------------------------------------------*/

bool
CGupImgImageCodec::Decode( CORE::CIOAccess& encodedInput ,
                           CImage& outputImage           )
{GUCEF_TRACE;

    SFileInfo info;
    CString problem;
    if ( !ReadFileInfo( encodedInput, info, GUCEF_NULL, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: " << problem );
        return false;
    }

    // Read, verify and decompress every level before the image is touched
    CImage::TFrameList frames;
    size_t levelIndex = 0;
    for ( size_t f=0; f<info.frames.size(); ++f )
    {
        CImage::TMipMapList mipLevels;
        for ( UInt32 l=0; l<info.frames[ f ].levelCount; ++l, ++levelIndex )
        {
            const SLevelEntry& level = info.levels[ levelIndex ];

            CORE::CDynamicBuffer storedBytes;
            if ( !ReadExactly( encodedInput, storedBytes, level.storedSize ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The data of frame " << (UInt32) f << " level " << l << " is incomplete" );
                return false;
            }
            if ( 0 != ( info.flags & FileFlagChecksums ) &&
                 level.crc != CPngImageCodec::Crc32( static_cast< const UInt8* >( storedBytes.GetConstBufferPtr() ), storedBytes.GetDataSize() ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The checksum of frame " << (UInt32) f << " level " << l << " does not match, the file is damaged" );
                return false;
            }

            CORE::CDynamicBuffer pixelBytes;
            if ( level.compressionCodec.IsNULLOrEmpty() )
            {
                pixelBytes.LinkTo( storedBytes );
            }
            else
            {
                CORE::CCodecRegistry::TICodecPtr decompressor = GetCompressionCodec( level.compressionCodec );
                if ( decompressor.IsNULL() )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The file needs the compression codec \"" << level.compressionCodec << "\", which is not available" );
                    return false;
                }
                CORE::CDynamicBufferAccess in( storedBytes );
                CORE::CDynamicBufferAccess out( &pixelBytes, false );
                if ( !decompressor->Decode( in, out ) )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The compression codec \"" << level.compressionCodec << "\" could not decompress frame " <<
                                     (UInt32) f << " level " << l );
                    return false;
                }
            }
            if ( pixelBytes.GetDataSize() != level.rawSize )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: Frame " << (UInt32) f << " level " << l << " has " << pixelBytes.GetDataSize() <<
                                 " bytes of pixels after decompression, the table says " << level.rawSize );
                return false;
            }

            TPixelMapPtr pixels( GUCEF_NEW CPixelMap( pixelBytes.GetConstBufferPtr()                     ,
                                                      level.width                                       ,
                                                      level.height                                      ,
                                                      (TPixelStorageFormat) level.pixelStorageFormat    ,
                                                      (TBuildinDataType) level.componentDataType        ) );
            if ( 0 != ( level.flags & LevelFlagNormalized ) )
                pixels->SetIsNormalizedAsPercentage( true );
            if ( 0 != ( info.flags & FileFlagRowsBottomToTop ) )
                pixels->FlipVertical();
            mipLevels.push_back( pixels );
        }
        frames.push_back( mipLevels );
    }

    outputImage.Clear();
    for ( size_t f=0; f<frames.size(); ++f )
        outputImage.AddFrame( frames[ f ], info.frames[ f ].durationInMs );
    outputImage.SetLoopCount( info.loopCount );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGupImgImageCodec::DecodeMetaData( CORE::CIOAccess& encodedInput ,
                                   CORE::CValueList& metaData    )
{GUCEF_TRACE;

    SFileInfo info;
    CString problem;
    metaData.Clear();
    if ( !ReadFileInfo( encodedInput, info, &metaData, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: " << problem );
        metaData.Clear();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGupImgImageCodec::Encode( CORE::CIOAccess& source ,
                           CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The codec works on images, use the image interface" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CGupImgImageCodec::Decode( CORE::CIOAccess& source ,
                           CORE::CIOAccess& dest   )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GupImgImageCodec: The codec works on images, use the image interface" );
    return false;
}

/*-------------------------------------------------------------------------*/

UInt32
CGupImgImageCodec::GetCapabilities( void ) const
{GUCEF_TRACE;

    return GUCEF_CODEC_CAPABILITY_ENCODE | GUCEF_CODEC_CAPABILITY_DECODE;
}

/*-------------------------------------------------------------------------*/

UInt32
CGupImgImageCodec::GetImageCapabilities( void ) const
{GUCEF_TRACE;

    return IMAGECAPABILITY_MULTIPLE_FRAMES | IMAGECAPABILITY_MIP_LEVELS;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGupImgImageCodec::GetType( void ) const
{GUCEF_TRACE;

    return CodecTypeName;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CGupImgImageCodec::GetFamilyName( void ) const
{GUCEF_TRACE;

    return CodecFamilyName;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CGupImgImageCodec::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CGupImgImageCodec( *this );
}

/*-------------------------------------------------------------------------*/

const CString&
CGupImgImageCodec::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::IMAGE::CGupImgImageCodec";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace IMAGE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
