/*
 *  assetpluginUNREAL: Texture2D assets to image files and textures of the scene
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
#include <stdio.h>

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#include "assetpluginUNREAL_CUnrealCapabilities.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCAPABILITIES_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#include "assetpluginUNREAL_CUnrealCompressedBuffer.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALCOMPRESSEDBUFFER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#include "assetpluginUNREAL_CUnrealDeltaTextureDecoder.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALDELTATEXTUREDECODER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#include "assetpluginUNREAL_CUnrealLegacyBulkData.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALLEGACYBULKDATA_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#include "assetpluginUNREAL_CUnrealPngWriter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALPNGWRITER_H ? */

#ifndef GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#include "assetpluginUNREAL_CUnrealTextureConverter.h"
#define GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H
#endif /* GUCEF_ASSETPLUGIN_UNREAL_CUNREALTEXTURECONVERTER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETPLUGIN {
namespace UNREAL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

static const char* const SourceName = "unreal";

/*-------------------------------------------------------------------------*/

CUnrealTextureConverter::SResult::SResult( void )
    : file()
    , textureName()
    , sourceFormat()
    , width( 0 )
    , height( 0 )
    , channels( 0 )
    , bitsPerChannel( 0 )
    , fileChecksum( 0 )
    , fileSize( 0 )
    , reducedTo8Bit( false )
    , greyAsOneChannel( false )
    , unsupported( false )
    , providerMissing( false )
    , cancelled( false )
    , toolCode( 0 )
    , error()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

/** The first bytes of a block for the text of an error */
static CORE::CString
HexOfStart( const UInt8* bytes, UInt32 size )
{GUCEF_TRACE;

    CORE::CString text;
    for ( UInt32 i=0; i<8 && i<size; ++i )
    {
        char hex[ 8 ];
        sprintf( hex, "%02x ", (unsigned int) bytes[ i ] );
        text += hex;
    }
    return text.IsNULLOrEmpty() ? CORE::CString( "nothing" ) : text;
}

/*-------------------------------------------------------------------------*/

/** Raw pixels of the source to the channels and bits of a PNG file; false for the formats that are not handled */
static bool
ConvertRaw( const CUnrealTextureReader::STextureSource& source, const UInt8* data, GUCEF::vector< UInt8 >& pixels, UInt32& channels, UInt32& bits, bool& reduced )
{GUCEF_TRACE;

    const size_t count = (size_t) source.width * source.height;
    reduced = false;
    if ( "TSF_G8" == source.format )
    {
        channels = 1; bits = 8;
        pixels.assign( data, data + count );
        return true;
    }
    if ( "TSF_BGRA8" == source.format )
    {
        channels = 4; bits = 8;
        pixels.resize( count * 4 );
        for ( size_t i=0; i<count; ++i )
        {
            pixels[ i * 4 ] = data[ i * 4 + 2 ];
            pixels[ i * 4 + 1 ] = data[ i * 4 + 1 ];
            pixels[ i * 4 + 2 ] = data[ i * 4 ];
            pixels[ i * 4 + 3 ] = data[ i * 4 + 3 ];
        }
        return true;
    }
    if ( "TSF_RGBA8" == source.format )
    {
        channels = 4; bits = 8;
        pixels.assign( data, data + count * 4 );
        return true;
    }
    if ( "TSF_RGBA16" == source.format || "TSF_G16" == source.format )
    {
        channels = "TSF_G16" == source.format ? 1 : 4; bits = 8;
        reduced = true;
        const size_t values = count * channels;
        pixels.resize( values );
        for ( size_t i=0; i<values; ++i )
        {
            const UInt32 value = (UInt32) data[ i * 2 ] | ( (UInt32) data[ i * 2 + 1 ] << 8 );
            pixels[ i ] = (UInt8) ( ( value * 255 + 32767 ) / 65535 );
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

/** Four 8 bit channels whose colour channels are equal in every pixel and whose alpha is opaque become one grey channel, losslessly */
static bool
ReduceGreyToOneChannel( GUCEF::vector< UInt8 >& pixels, UInt32& channels, UInt32 bits )
{GUCEF_TRACE;

    if ( 4 != channels || 8 != bits || pixels.empty() )
        return false;
    const size_t count = pixels.size() / 4;
    for ( size_t i=0; i<count; ++i )
    {
        const UInt8* p = &pixels[ i * 4 ];
        if ( p[ 0 ] != p[ 1 ] || p[ 1 ] != p[ 2 ] || 255 != p[ 3 ] )
            return false;
    }
    for ( size_t i=0; i<count; ++i )
        pixels[ i ] = pixels[ i * 4 ];
    pixels.resize( count );
    channels = 1;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealTextureConverter::Convert( CUnrealPackage& package                       ,
                                  UInt32 exportIndex                            ,
                                  CIUnrealOodleProvider* provider               ,
                                  const ASSETCONVERTER::CConversionReport* cancellation ,
                                  const CORE::CString& folder                   ,
                                  SResult& result                               )
{GUCEF_TRACE;

    return ConvertSource( package, exportIndex, provider, cancellation, folder, result, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealTextureConverter::ConvertWithPixels( CUnrealPackage& package                       ,
                                            UInt32 exportIndex                            ,
                                            CIUnrealOodleProvider* provider               ,
                                            const ASSETCONVERTER::CConversionReport* cancellation ,
                                            const CORE::CString& folder                   ,
                                            SResult& result                               ,
                                            GUCEF::vector< UInt8 >& pixels                )
{GUCEF_TRACE;

    pixels.clear();
    return ConvertSource( package, exportIndex, provider, cancellation, folder, result, &pixels );
}

/*-------------------------------------------------------------------------*/

bool
CUnrealTextureConverter::ConvertSource( CUnrealPackage& package                       ,
                                        UInt32 exportIndex                            ,
                                        CIUnrealOodleProvider* provider               ,
                                        const ASSETCONVERTER::CConversionReport* cancellation ,
                                        const CORE::CString& folder                   ,
                                        SResult& result                               ,
                                        GUCEF::vector< UInt8 >* pixelsOut             )
{GUCEF_TRACE;

    result = SResult();
    CUnrealTextureReader reader;
    CUnrealTextureReader::STextureSource source;
    if ( !reader.Read( package, exportIndex, source ) )
    {
        result.error = reader.GetError();
        return false;
    }
    result.textureName = source.name;
    result.sourceFormat = source.compressionFormat + "/" + source.format;
    const bool delta = "TSCF_UEDELTA" == source.compressionFormat;
    if ( delta )
    {
        // the format and the sizes are checked before the payload is read: a source this decoder refuses costs no decompression
        UInt32 rows = 0;
        if ( !CUnrealDeltaTextureDecoder::StripRows( source.format, source.width, source.height, rows, result.error ) )
        {
            result.unsupported = 0 != source.width && 0 != source.height && source.width <= CUnrealDeltaTextureDecoder::MaxDimension && source.height <= CUnrealDeltaTextureDecoder::MaxDimension;
            return false;
        }
    }
    else
    if ( "TSCF_PNG" != source.compressionFormat && "TSCF_JPEG" != source.compressionFormat && "TSCF_None" != source.compressionFormat )
    {
        result.unsupported = true;
        result.error = "The source compression format " + source.compressionFormat + " is not known to this reader";
        return false;
    }
    if ( !source.hasPayload || CUnrealPayloadReference::KIND_EMPTY == source.payload.kind )
    {
        result.error = "The texture holds no source data";
        return false;
    }
    if ( !source.payload.IsReadable() )
    {
        result.error = "The source data is not in this package: " + source.payload.reason;
        return false;
    }

    // the bytes of the source
    CORE::CDynamicBuffer data;
    if ( source.isLegacyPayload && 0 != ( source.payload.flags & CUnrealLegacyBulkData::FLAG_COMPRESSED_ZLIB ) )
    {
        result.unsupported = true;
        result.error = "The source is legacy bulk data compressed with zlib in blocks (flag 0x2 of the bulk data, as the engines before UE 5.0 store it): this reader does not inflate it yet";
        return false;
    }
    if ( source.isLegacyPayload )
    {
        const UInt64 size = source.payload.storedSize;
        CUnrealArchiveReader payload;
        if ( size > package.GetLimits().maxPayloadRawBytes || size > 0xFFFFFFF0ULL || !package.OpenPayload( source.payload, payload ) )
        {
            result.error = "The source data cannot be opened";
            return false;
        }
        data.SetBufferSize( (UInt32) size, false );
        data.SetDataSize( (UInt32) size );
        if ( !payload.Read( data.GetBufferPtr(), size ) )
        {
            result.error = "The source data cannot be read: " + payload.GetError();
            return false;
        }
    }
    else
    {
        CUnrealCompressedBuffer buffer( package, provider );
        buffer.SetCancellationSource( cancellation );
        if ( !buffer.Open( source.payload ) || !buffer.ReadAll( data ) )
        {
            result.error = buffer.GetError();
            result.providerMissing = buffer.IsProviderMissing();
            result.cancelled = buffer.IsCancelled();
            result.toolCode = buffer.GetToolCode();
            return false;
        }
    }

    // the file
    const UInt8* bytes = static_cast< const UInt8* >( data.GetConstBufferPtr() );
    CORE::CDynamicBuffer image;
    CORE::CString extension = "png";
    if ( "TSCF_PNG" == source.compressionFormat )
    {
        if ( !CUnrealPngWriter::ReadPngInfo( bytes, data.GetDataSize(), result.width, result.height, result.channels, result.bitsPerChannel ) )
        {
            result.error = "The source is not a PNG file (it starts with " + HexOfStart( bytes, data.GetDataSize() ) + ")";
            return false;
        }
        image.CopyFrom( data.GetDataSize(), bytes );
    }
    else
    if ( "TSCF_JPEG" == source.compressionFormat )
    {
        extension = "jpg";
        result.bitsPerChannel = 8;
        if ( !CUnrealPngWriter::ReadJpegInfo( bytes, data.GetDataSize(), result.width, result.height, result.channels ) )
        {
            result.error = "The source is not a JPEG file (it starts with " + HexOfStart( bytes, data.GetDataSize() ) + ")";
            return false;
        }
        image.CopyFrom( data.GetDataSize(), bytes );
    }
    else
    {
        const UInt32 bpp = source.GetBytesPerPixel();
        if ( 0 == bpp )
        {
            result.unsupported = true;
            result.error = "The source pixel format " + source.format + " is not supported";
            return false;
        }
        const UInt64 first = (UInt64) source.width * source.height * bpp;
        if ( data.GetDataSize() < first )
        {
            result.error = "The raw source holds " + CORE::UInt32ToString( data.GetDataSize() ) + " bytes where " + CORE::UInt64ToString( first ) + " are needed for the first image";
            return false;
        }
        if ( delta && !CUnrealDeltaTextureDecoder::Decode( source.format, source.width, source.height, static_cast< UInt8* >( data.GetBufferPtr() ), data.GetDataSize(), result.error ) )
            return false;
        GUCEF::vector< UInt8 > pixels;
        if ( !ConvertRaw( source, bytes, pixels, result.channels, result.bitsPerChannel, result.reducedTo8Bit ) )
        {
            result.unsupported = true;
            result.error = "The source pixel format " + source.format + " is not supported";
            return false;
        }
        result.greyAsOneChannel = ReduceGreyToOneChannel( pixels, result.channels, result.bitsPerChannel );
        result.width = source.width;
        result.height = source.height;
        if ( !CUnrealPngWriter::Write( &pixels[ 0 ], result.width, result.height, result.channels, result.bitsPerChannel, image ) )
        {
            result.error = "The PNG file cannot be made of the raw source";
            return false;
        }
        if ( GUCEF_NULL != pixelsOut )
            pixelsOut->swap( pixels );
    }

    if ( GUCEF_NULL != pixelsOut && folder.IsNULLOrEmpty() )
        return true;
    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    vfs.CreateDir( folder );
    result.file = ASSETCONVERTER::CVfsPaths::Join( folder, source.name + "." + extension );
    if ( !vfs.StoreAsFile( result.file, image, 0, true ) )
    {
        result.error = "The image file cannot be written to " + result.file;
        result.file.Clear();
        return false;
    }
    result.fileSize = image.GetDataSize();
    result.fileChecksum = CUnrealPngWriter::Crc32( static_cast< const UInt8* >( image.GetConstBufferPtr() ), image.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

static ASSETCONVERTER::TTextureUsage
UsageOf( const CUnrealTextureReader::STextureSource& source )
{GUCEF_TRACE;

    if ( source.IsNormalMap() )
        return ASSETCONVERTER::TEXTUREUSAGE_NORMAL;
    if ( !source.srgb )
        return ASSETCONVERTER::TEXTUREUSAGE_MASK;
    return ASSETCONVERTER::TEXTUREUSAGE_COLOUR;
}

/*-------------------------------------------------------------------------*/

bool
CUnrealTextureConverter::AddToScene( CUnrealPackage& package                    ,
                                     Int32 exportIndex                          ,
                                     CIUnrealOodleProvider* provider            ,
                                     const CORE::CString& folder                ,
                                     ASSETCONVERTER::CAssetScene& scene         ,
                                     ASSETCONVERTER::CConversionReport& report  ,
                                     Int32& textureIndex                        )
{GUCEF_TRACE;

    textureIndex = -1;
    if ( exportIndex < 0 )
        for ( size_t i=0; i<package.GetExports().size(); ++i )
            if ( package.GetExports()[ i ].outerIndex.IsNull() && "Texture2D" == package.GetClassName( package.GetExports()[ i ] ) )
            {
                exportIndex = (Int32) i;
                break;
            }
    if ( exportIndex < 0 || (size_t) exportIndex >= package.GetExports().size() )
    {
        report.AddWarning( SourceName, package.GetSource().GetName(), "The package holds no Texture2D" );
        return false;
    }
    const CORE::CString name = package.GetExports()[ (size_t) exportIndex ].objectName;
    if ( folder.IsNULLOrEmpty() )
    {
        report.AddWarning( SourceName, name, "The texture is not extracted: the manifest has no intermediate folder to write the image to" );
        return false;
    }

    SResult result;
    if ( !Convert( package, (UInt32) exportIndex, provider, &report, folder, result ) )
    {
        if ( result.providerMissing )
            report.AddSkipped( SourceName, name, result.error );
        else
        if ( result.unsupported )
            report.AddSkipped( SourceName, name, "Unsupported texture source: " + result.error );
        else
            report.AddWarning( SourceName, name, "The texture cannot be read: " + result.error + ( 0 != result.toolCode ? " (exit code " + CORE::Int32ToString( result.toolCode ) + ")" : CORE::CString::Empty ) );
        return false;
    }
    CUnrealTextureReader reader;
    CUnrealTextureReader::STextureSource source;
    reader.Read( package, (UInt32) exportIndex, source );

    ASSETCONVERTER::CAssetTexture texture;
    texture.name = name;
    texture.sources.push_back( result.file );
    texture.usage = UsageOf( source );
    texture.wrapS = IMAGE::WRAPMODE_REPEAT;
    texture.wrapT = IMAGE::WRAPMODE_REPEAT;
    scene.textures.push_back( texture );
    textureIndex = (Int32) scene.textures.size() - 1;
    report.AddOutputFile( result.file, CORE::CString::StringVector() );
    if ( result.reducedTo8Bit )
        report.AddApproximation( SourceName, name, "the source has 16 bits per channel; the image file has 8" );
    report.AddInfo( SourceName, name, "texture " + CORE::UInt32ToString( result.width ) + " x " + CORE::UInt32ToString( result.height ) + ", " + CORE::UInt32ToString( result.channels ) + " channels, source " + result.sourceFormat +
                    ( result.greyAsOneChannel ? " (equal colour channels and opaque alpha: written as one grey channel)" : "" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CUnrealTextureConverter::AttachToMaterials( const CUnrealObjectPathResolver& resolver ,
                                            CIUnrealOodleProvider* provider           ,
                                            const CORE::CString& folder               ,
                                            ASSETCONVERTER::CAssetScene& scene        ,
                                            ASSETCONVERTER::CConversionReport& report )
{GUCEF_TRACE;

    typedef GUCEF::map< CORE::CString, Int32 > TObjectMap;
    TObjectMap done;                // object path of a texture to its index in the scene, -1 for one that cannot be read
    UInt32 filled = 0;
    CORE::CString::StringVector notInPack;
    for ( size_t m=0; m<scene.materials.size(); ++m )
    {
        ASSETCONVERTER::CAssetMaterial& material = scene.materials[ m ];
        for ( CORE::CDataNode::const_iterator t = material.source.textures.ConstBegin(); t != material.source.textures.ConstEnd(); ++t )
        {
            const CORE::CDataNode* parameter = *t;
            const CORE::CString object = parameter->GetAttributeValue( "object" ).AsString();
            const CORE::CString slot = parameter->GetAttributeValue( "slot" ).AsString();
            if ( object.IsNULLOrEmpty() || slot.IsNULLOrEmpty() )
                continue;
            Int32 index = -1;
            TObjectMap::const_iterator known = done.find( object );
            if ( known != done.end() )
                index = (*known).second;
            else
            {
                CORE::CString vfsPath;
                // a texture that is not in the pack (the shared textures of an engine are in its content, not in a package of a character) is
                // one line of the report for all of them, and no attempt to open a file that is not there
                if ( !resolver.ResolveReference( object, ASSETCONVERTER::CAssetReference::KindUsesTexture, vfsPath ) || !VFS::CVfsGlobal::Instance()->GetVfs().FileExists( vfsPath ) )
                    notInPack.push_back( object );
                else
                {
                    CUnrealPackage package;
                    if ( !package.Open( vfsPath ) )
                        report.AddWarning( SourceName, material.name, "The texture package of " + object + " cannot be read: " + package.GetError() );
                    else
                        AddToScene( package, -1, provider, folder, scene, report, index );
                }
                done[ object ] = index;
            }
            if ( index < 0 )
                continue;
            if ( "baseColorTexture" == slot )
                material.baseColorTexture = ASSETCONVERTER::CAssetTextureReference( index );
            else
            if ( "normalTexture" == slot )
            {
                const Float32 scale = material.normalTexture.scale;
                material.normalTexture = ASSETCONVERTER::CAssetTextureReference( index );
                material.normalTexture.scale = scale;
            }
            else
            if ( "emissiveTexture" == slot )
                material.emissiveTexture = ASSETCONVERTER::CAssetTextureReference( index );
            else
            if ( slot.HasSubstr( "metallicRoughnessTexture" ) >= 0 )
            {
                material.occlusionTexture = ASSETCONVERTER::CAssetTextureReference( index );
                material.metallicRoughnessTexture = ASSETCONVERTER::CAssetTextureReference( index );
                scene.textures[ (size_t) index ].usage = ASSETCONVERTER::TEXTUREUSAGE_MASK;
            }
            else
                continue;
            ++filled;
        }
    }
    if ( !notInPack.empty() )
    {
        CORE::CString list;
        for ( size_t i=0; i<notInPack.size() && i<3; ++i )
            list += ( 0 == i ? "" : ", " ) + ASSETCONVERTER::CVfsPaths::NameOf( notInPack[ i ] );
        report.AddWarning( SourceName, "textures", CORE::UInt32ToString( (UInt32) notInPack.size() ) + " textures of the materials are not in the pack, those materials have no picture for them (" + list +
                           ( notInPack.size() > 3 ? " and more" : "" ) + ")" );
    }
    return filled;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace UNREAL */
}; /* namespace ASSETPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
