/*
 *  AssetConverterLib: bakes the operation chains of textures, once per unique chain
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CCODECREGISTRY_H
#include "CCodecRegistry.h"
#define GUCEF_CORE_CCODECREGISTRY_H
#endif /* GUCEF_CORE_CCODECREGISTRY_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef GUCEF_IMAGE_CIMAGE_H
#include "gucefIMAGE_CImage.h"
#define GUCEF_IMAGE_CIMAGE_H
#endif /* GUCEF_IMAGE_CIMAGE_H ? */

#ifndef GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#include "gucefIMAGE_CImageCodecRegistry.h"
#define GUCEF_IMAGE_CIMAGECODECREGISTRY_H
#endif /* GUCEF_IMAGE_CIMAGECODECREGISTRY_H ? */

#ifndef GUCEF_IMAGE_CIMAGEGLOBAL_H
#include "gucefIMAGE_CImageGlobal.h"
#define GUCEF_IMAGE_CIMAGEGLOBAL_H
#endif /* GUCEF_IMAGE_CIMAGEGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CCHANNELOPERATIONS_H
#include "gucefIMAGE_CChannelOperations.h"
#define GUCEF_IMAGE_CCHANNELOPERATIONS_H
#endif /* GUCEF_IMAGE_CCHANNELOPERATIONS_H ? */

#ifndef GUCEF_IMAGE_CPNGIMAGECODEC_H
#include "gucefIMAGE_CPngImageCodec.h"
#define GUCEF_IMAGE_CPNGIMAGECODEC_H
#endif /* GUCEF_IMAGE_CPNGIMAGECODEC_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */

#ifndef ASSETCONVERTERLIB_CCONTENTHASH_H
#include "AssetConverterLib_CContentHash.h"
#define ASSETCONVERTERLIB_CCONTENTHASH_H
#endif /* ASSETCONVERTERLIB_CCONTENTHASH_H ? */

#ifndef ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#include "AssetConverterLib_CConversionReport.h"
#define ASSETCONVERTERLIB_CCONVERSIONREPORT_H
#endif /* ASSETCONVERTERLIB_CCONVERSIONREPORT_H ? */

#ifndef ASSETCONVERTERLIB_CVFSPATHS_H
#include "AssetConverterLib_CVfsPaths.h"
#define ASSETCONVERTERLIB_CVFSPATHS_H
#endif /* ASSETCONVERTERLIB_CVFSPATHS_H ? */

#include "AssetConverterLib_CTextureBaker.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETCONVERTER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

const char* const LogicalChannels = "RGBA";

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CTextureBaker::CTextureBaker( const CORE::CString& outputFolder )
    : m_folder( outputFolder )
    , m_bakes()
    , m_lock()
    , m_written( 0 )
    , m_reused( 0 )
    , m_writesFiles( true )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CTextureBaker::SetWritesFiles( bool writesFiles )
{GUCEF_TRACE;

    m_writesFiles = writesFiles;
}

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::GetWritesFiles( void ) const
{GUCEF_TRACE;

    return m_writesFiles;
}

/*-------------------------------------------------------------------------*/

CTextureBaker::~CTextureBaker()
{GUCEF_TRACE;

    TBakeMap::iterator i = m_bakes.begin();
    while ( i != m_bakes.end() )
    {
        GUCEF_DELETE (*i).second;
        ++i;
    }
    m_bakes.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CTextureBaker::GetOutputFolder( void ) const { return m_folder; }
UInt32 CTextureBaker::GetWrittenCount( void ) const { return m_written; }
UInt32 CTextureBaker::GetReusedCount( void ) const { return m_reused; }

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::LoadPicture( const CORE::CString& vfsPath, IMAGE::TPixelMapPtr& picture )
{GUCEF_TRACE;

    CORE::CString problem;
    return LoadPicture( vfsPath, picture, problem );
}

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::LoadPicture( const CORE::CString& vfsPath, IMAGE::TPixelMapPtr& picture, CORE::CString& problem )
{GUCEF_TRACE;

    problem.Clear();
    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( vfsPath, "rb", false );
    if ( resource.IsNULL() || resource->GetAccess().IsNULL() )
    {
        problem = "the file cannot be opened";
        return false;
    }
    const CORE::CString extension = CVfsPaths::ExtensionOf( vfsPath ).Lowercase();
    IMAGE::CImage image;
    if ( !image.Load( *resource->GetAccess(), extension, true, false ) || !image.HasFrames() )
    {
        // CImage::Load uses a codec of the image codec registry, else one of the image codec family of the codec registry
        // (a plugin loaded on the basic codec interface): with a codec in either, the file is one the codec cannot decode
        IMAGE::CImageCodecRegistry::TImageCodecPtr codec;
        bool haveCodec = IMAGE::CImageGlobal::Instance()->GetImageCodecRegistry().TryLookup( extension, codec, false );
        if ( !haveCodec )
        {
            CORE::CCodecRegistry::TCodecFamilyRegistryPtr family;
            CORE::CCodecRegistry::TICodecPtr basicCodec;
            haveCodec = CORE::CCoreGlobal::Instance()->GetCodecRegistry().TryLookup( IMAGE::CPngImageCodec::CodecFamilyName, family, false ) &&
                        !family.IsNULL() && family->TryLookup( extension, basicCodec, false );
        }
        if ( !haveCodec )
            problem = "no image codec for ." + extension + " is loaded";
        else
            problem = "the image codec for ." + extension + " cannot decode the file (damaged, or a variant it does not support)";
        return false;
    }
    IMAGE::TPixelMapPtr pixels = image.GetPixelMap( 0, 0 );
    if ( pixels.IsNULL() )
    {
        problem = "the decoded image has no pixels";
        return false;
    }

    // floating point channels (hdr, exr): clamped to 0..1 and kept as 8 bits, as a texture of a model is read by the renderers
    const IMAGE::TPixelStorageFormat storage = pixels->GetPixelStorageFormat();
    if ( GUCEF::DATATYPE_LE_FLOAT32 == pixels->GetPixelComponentDataType() &&
         ( IMAGE::PSF_RGB == storage || IMAGE::PSF_RGBA == storage || IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE == storage ) )
    {
        const UInt32 values = pixels->GetPixelCount() * pixels->GetNumberOfChannelsPerPixel();
        const Float32* source = static_cast< const Float32* >( pixels->GetDataPtr() );
        GUCEF::vector< UInt8 > bytes( values );
        for ( UInt32 i=0; i<values; ++i )
        {
            const Float32 v = source[ i ] < 0.0f ? 0.0f : ( source[ i ] > 1.0f ? 1.0f : source[ i ] );
            bytes[ i ] = (UInt8) ( v * 255.0f + 0.5f );
        }
        picture = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( &bytes[ 0 ], pixels->GetWidthInPixels(), pixels->GetHeightInPixels(), storage, GUCEF::DATATYPE_UINT8 ) );
        return true;
    }

    // red first, whatever order the codec gives
    switch ( storage )
    {
        case IMAGE::PSF_BGR:
        {
            if ( IMAGE::CChannelOperations::Swizzle( *pixels, "RGB", picture ) )
                return true;
            break;
        }
        case IMAGE::PSF_BGRA:
        {
            if ( IMAGE::CChannelOperations::Swizzle( *pixels, "RGBA", picture ) )
                return true;
            break;
        }
        case IMAGE::PSF_RGB:
        case IMAGE::PSF_RGBA:
        case IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE:
        {
            picture = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( *pixels ) );
            return true;
        }
        case IMAGE::PSF_SINGLE_CHANNEL_STD_LUMINANCE:
        case IMAGE::PSF_SINGLE_CHANNEL_P1_LUMINANCE:
        case IMAGE::PSF_SINGLE_CHANNEL_P2_LUMINANCE:
        {
            // one grey channel of 8 or 16 bits (a 16 bit greyscale PNG): grey of 8 bits, 16 bit values rounded to the nearest
            const UInt32 count = pixels->GetPixelCount();
            if ( 0 == count || ( GUCEF::DATATYPE_UINT8 != pixels->GetPixelComponentDataType() && GUCEF_DATATYPE_UINT16 != pixels->GetPixelComponentDataType() ) )
                break;
            GUCEF::vector< UInt8 > grey( count );
            if ( GUCEF::DATATYPE_UINT8 == pixels->GetPixelComponentDataType() )
                memcpy( &grey[ 0 ], pixels->GetDataPtr(), count );
            else
            {
                const UInt16* values = static_cast< const UInt16* >( pixels->GetDataPtr() );
                for ( UInt32 i=0; i<count; ++i )
                    grey[ i ] = (UInt8) ( ( (UInt32) values[ i ] + 128 ) / 257 );
            }
            picture = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( &grey[ 0 ], pixels->GetWidthInPixels(), pixels->GetHeightInPixels(), IMAGE::PSF_SINGLE_CHANNEL_GRAYSCALE, GUCEF::DATATYPE_UINT8 ) );
            return true;
        }
        default:
        {
            IMAGE::TPixelMapPtr converted;
            const IMAGE::TPixelStorageFormat target = pixels->GetHasAlpha() ? IMAGE::PSF_RGBA : IMAGE::PSF_RGB;
            if ( pixels->ConvertFormatTo( target, pixels->GetPixelComponentDataType(), converted ) && !converted.IsNULL() )
            {
                picture = converted;
                return true;
            }
            break;
        }
    }
    problem = "the pixel format of the decoded image (storage format " + CORE::UInt32ToString( (UInt32) storage ) + ", component type " +
              CORE::UInt32ToString( (UInt32) pixels->GetPixelComponentDataType() ) + ") cannot be converted to RGB";
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::SavePng( const CORE::CString& vfsPath, const IMAGE::CPixelMap& picture )
{GUCEF_TRACE;

    IMAGE::TPixelMapPtr bytes( GUCEF_NEW IMAGE::CPixelMap( picture ) );
    if ( GUCEF::DATATYPE_UINT8 != bytes->GetPixelComponentDataType() && !bytes->ConvertFormatTo( GUCEF::DATATYPE_UINT8 ) )
        return false;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    const CORE::CString folder = CVfsPaths::FolderOf( vfsPath );
    if ( !folder.IsNULLOrEmpty() )
        vfs.CreateDir( folder );
    CORE::CDynamicBuffer png;
    CORE::CString problem;
    if ( !IMAGE::CPngImageCodec::EncodePixelMap( *bytes, png, problem ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "TextureBaker: " << vfsPath << " cannot be encoded as PNG: " << problem );
        return false;
    }
    return vfs.StoreAsFile( vfsPath, png, 0, true );
}

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::ApplyOperations( const CAssetTexture& texture, const TPixelMapVector& sources, IMAGE::TPixelMapPtr& result, CORE::CString& problem )
{GUCEF_TRACE;

    if ( sources.empty() || sources[ 0 ].IsNULL() )
    {
        problem = "the texture has no source";
        return false;
    }

    IMAGE::TPixelMapPtr current( GUCEF_NEW IMAGE::CPixelMap( *sources[ 0 ] ) );
    for ( size_t i=0; i<texture.operations.size(); ++i )
    {
        const CTextureOperation& op = texture.operations[ i ];
        bool ok = true;
        switch ( op.type )
        {
            case TEXTUREOPERATION_EXTRACTCHANNEL:
            {
                if ( op.channel >= 4 )
                {
                    ok = false;
                    break;
                }
                IMAGE::TPixelMapPtr extracted;
                ok = IMAGE::CChannelOperations::Swizzle( *current, CORE::CString( LogicalChannels[ op.channel ] ), extracted );
                if ( ok )
                    current = extracted;
                break;
            }
            case TEXTUREOPERATION_SWIZZLE:
            {
                IMAGE::TPixelMapPtr swizzled;
                ok = IMAGE::CChannelOperations::Swizzle( *current, op.mapping, swizzled );
                if ( ok )
                    current = swizzled;
                break;
            }
            case TEXTUREOPERATION_INVERT:
                ok = IMAGE::CChannelOperations::Invert( *current, op.channelMask );
                break;
            case TEXTUREOPERATION_SCALEBIAS:
                ok = IMAGE::CChannelOperations::ScaleBias( *current, op.channelMask, op.scale, op.bias );
                break;
            case TEXTUREOPERATION_GREENFLIP:
                ok = current->GetNumberOfChannelsPerPixel() >= 2 && IMAGE::CChannelOperations::Invert( *current, 0x2 );
                break;
            case TEXTUREOPERATION_PREMULTIPLYALPHA:
                ok = IMAGE::CChannelOperations::PremultiplyAlpha( *current, op.colourSpace );
                break;
            case TEXTUREOPERATION_UNPREMULTIPLYALPHA:
                ok = IMAGE::CChannelOperations::UnpremultiplyAlpha( *current, op.colourSpace );
                break;
            case TEXTUREOPERATION_PACK:
            {
                // the size of the first source used; the others are resampled to it
                IMAGE::TPixelMapPtr packSources[ 4 ];
                UInt32 width = 0, height = 0;
                for ( UInt32 c=0; c<4 && ok; ++c )
                {
                    const Int32 index = op.packSources[ c ];
                    if ( index < 0 )
                        continue;
                    if ( (size_t) index >= sources.size() || sources[ index ].IsNULL() )
                    {
                        ok = false;
                        break;
                    }
                    if ( 0 == width )
                    {
                        width = sources[ index ]->GetWidthInPixels();
                        height = sources[ index ]->GetHeightInPixels();
                    }
                    packSources[ c ] = sources[ index ];
                    if ( packSources[ c ]->GetWidthInPixels() != width || packSources[ c ]->GetHeightInPixels() != height )
                    {
                        IMAGE::TPixelMapPtr resampled;
                        ok = IMAGE::CPixelMapResampler::Resample( *packSources[ c ], width, height, IMAGE::RESAMPLEFILTER_TRIANGLE,
                                                                  IMAGE::COLOURSPACE_LINEAR, IMAGE::WRAPMODE_REPEAT, resampled );
                        packSources[ c ] = resampled;
                    }
                }
                IMAGE::TPixelMapPtr packed;
                ok = ok && IMAGE::CChannelOperations::Pack( packSources, op.packChannels, packed );
                if ( ok )
                    current = packed;
                break;
            }
            case TEXTUREOPERATION_COLOURSPACETAG:
            default:
                break;
        }
        if ( !ok || current.IsNULL() )
        {
            problem = "the step " + op.ToKeyText() + " of the operation chain failed";
            return false;
        }
    }
    result = current;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::DoBake( const CAssetTexture& texture, const CORE::CString& path, bool copySource, SBake& bake )
{GUCEF_TRACE;

    TPixelMapVector sources;
    for ( size_t i=0; i<texture.sources.size(); ++i )
    {
        IMAGE::TPixelMapPtr picture;
        CORE::CString reason;
        if ( !LoadPicture( texture.sources[ i ], picture, reason ) )
        {
            bake.problem = "the source " + texture.sources[ i ] + " cannot be read as a picture: " + reason;
            return false;
        }
        sources.push_back( picture );
    }
    IMAGE::TPixelMapPtr result;
    if ( !ApplyOperations( texture, sources, result, bake.problem ) )
        return false;
    bake.width = result->GetWidthInPixels();
    bake.height = result->GetHeightInPixels();
    bake.path = path;
    if ( !m_writesFiles )
        return true;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    if ( vfs.FileExists( path ) )
    {
        MT::CScopeMutex lock( m_lock );
        ++m_reused;
        return true;
    }
    if ( copySource )
    {
        const CORE::CString folder = CVfsPaths::FolderOf( path );
        if ( !folder.IsNULLOrEmpty() )
            vfs.CreateDir( folder );
        if ( !vfs.CopyFileContent( texture.sources[ 0 ], path, true ) )
        {
            bake.problem = "the source " + texture.sources[ 0 ] + " cannot be copied to " + path;
            return false;
        }
    }
    else
    if ( !SavePng( path, *result ) )
    {
        bake.problem = "the baked texture cannot be written to " + path;
        return false;
    }
    MT::CScopeMutex lock( m_lock );
    ++m_written;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CTextureBaker::Bake( CAssetTexture& texture, CConversionReport& report )
{GUCEF_TRACE;

    static const CORE::CString source = "TextureBaker";
    if ( texture.sources.empty() )
    {
        texture.missing = true;
        report.AddError( source, texture.name, "the texture has no source" );
        return false;
    }

    // the name: the first source and the hash of the contents and of the chain
    CORE::CString keyText;
    for ( size_t i=0; i<texture.sources.size(); ++i )
    {
        const CORE::CString hash = CContentHash::HashFile( texture.sources[ i ] );
        if ( hash.IsNULLOrEmpty() )
        {
            texture.missing = true;
            report.AddError( source, texture.name, "the source " + texture.sources[ i ] + " cannot be read" );
            return false;
        }
        keyText += hash + ";";
    }
    const CORE::CString extension = CVfsPaths::ExtensionOf( texture.sources[ 0 ] );
    const bool copySource = !texture.HasTexelOperations() && ( "png" == extension || "jpg" == extension || "jpeg" == extension );
    const CORE::CString outputExtension = copySource ? extension : CORE::CString( "png" );
    keyText += texture.GetChainKeyText() + "|" + outputExtension;
    const CORE::CString key = CContentHash::HashText( keyText );
    const CORE::CString path = CVfsPaths::Join( m_folder, CVfsPaths::StemOf( texture.sources[ 0 ] ) + "." + key.SubstrFromRange( 0, 16 ) + "." + outputExtension );

    // one bake per chain: the first task bakes, the others wait for its result
    SBake* bake = GUCEF_NULL;
    bool mine = false;
    {
        MT::CScopeMutex lock( m_lock );
        TBakeMap::iterator i = m_bakes.find( key );
        if ( i == m_bakes.end() )
        {
            bake = GUCEF_NEW SBake();
            bake->ok = false;
            bake->width = 0;
            bake->height = 0;
            bake->lock.Lock( GUCEF_MT_INFINITE_LOCK_TIMEOUT );
            m_bakes[ key ] = bake;
            mine = true;
        }
        else
        {
            bake = (*i).second;
        }
    }
    if ( mine )
    {
        bake->ok = DoBake( texture, path, copySource, *bake );
        bake->lock.Unlock();
    }
    else
    {
        // a bake can take long: waited for without a time limit
        MT::CScopeMutex wait( bake->lock, GUCEF_MT_INFINITE_LOCK_TIMEOUT );
        MT::CScopeMutex lock( m_lock );
        ++m_reused;
    }

    if ( !bake->ok )
    {
        texture.missing = true;
        report.AddError( source, texture.name, bake->problem );
        return false;
    }
    texture.bakedPath = bake->path;
    texture.width = bake->width;
    texture.height = bake->height;
    texture.missing = false;
    if ( m_writesFiles )
        report.AddOutputFile( bake->path, CORE::CString::StringVector() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETCONVERTER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
