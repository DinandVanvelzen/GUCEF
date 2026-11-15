/*
 *  gucefASSETSTORE: GUCEF module for online asset stores and the primary asset store
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

#ifndef GUCEF_CORE_CIOACCESS_H
#include "CIOAccess.h"
#define GUCEF_CORE_CIOACCESS_H
#endif /* GUCEF_CORE_CIOACCESS_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */
#ifndef GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#include "CDynamicBufferAccess.h"
#define GUCEF_CORE_CDYNAMICBUFFERACCESS_H
#endif /* GUCEF_CORE_CDYNAMICBUFFERACCESS_H ? */
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
#ifndef GUCEF_IMAGE_CPIXELMAP_H
#include "gucefIMAGE_CPixelMap.h"
#define GUCEF_IMAGE_CPIXELMAP_H
#endif /* GUCEF_IMAGE_CPIXELMAP_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H
#include "gucefASSETSTORE_CAssetImageHeaderProbe.h"
#define GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H
#endif /* GUCEF_ASSETSTORE_CASSETIMAGEHEADERPROBE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#include "gucefASSETSTORE_CAssetNameHints.h"
#define GUCEF_ASSETSTORE_CASSETNAMEHINTS_H
#endif /* GUCEF_ASSETSTORE_CASSETNAMEHINTS_H ? */
#include "gucefASSETSTORE_CAssetTextureAnalyzer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether the average colour of the first pixels is that of a tangent space normal map */
bool
LooksLikeNormalMap( const IMAGE::CPixelMap& pixels )
{
    if ( 1 != pixels.GetPixelChannelSize() )
        return false;
    const IMAGE::TPixelStorageFormat format = pixels.GetPixelStorageFormat();
    const bool bgr = IMAGE::PSF_BGR == format || IMAGE::PSF_BGRA == format;
    if ( !bgr && IMAGE::PSF_RGB != format && IMAGE::PSF_RGBA != format )
        return false;
    const UInt32 pixelSize = pixels.GetSizeOfPixelInBytes();
    const UInt32 count = pixels.GetPixelCount() > 4096 ? 4096 : pixels.GetPixelCount();
    if ( 0 == count || pixelSize < 3 )
        return false;
    const UInt8* data = static_cast< const UInt8* >( pixels.GetDataPtr() );
    UInt64 sum[ 3 ] = { 0, 0, 0 };
    for ( UInt32 p=0; p<count; ++p )
        for ( UInt32 c=0; c<3; ++c )
            sum[ c ] += data[ p * pixelSize + c ];
    const UInt64 red = ( bgr ? sum[ 2 ] : sum[ 0 ] ) / count;
    const UInt64 green = sum[ 1 ] / count;
    const UInt64 blue = ( bgr ? sum[ 0 ] : sum[ 2 ] ) / count;
    return blue >= 200 && red >= 100 && red <= 160 && green >= 100 && green <= 160;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetTextureAnalyzer::CAssetTextureAnalyzer( void )
    : CIAssetContentAnalyzer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAssetTextureAnalyzer::~CAssetTextureAnalyzer()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTextureAnalyzer::GetName( void ) const
{GUCEF_TRACE;

    return "texture";
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetTextureAnalyzer::GetVersion( void ) const
{GUCEF_TRACE;

    return "1";
}

/*-------------------------------------------------------------------------*/

bool
CAssetTextureAnalyzer::CanAnalyze( const CAssetRecord& asset ) const
{GUCEF_TRACE;

    return asset.kind == "texture" || asset.kind == "decal" || asset.kind == "hdri" || asset.kind == "icon" || asset.kind == "sprite";
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetTextureAnalyzer::Analyze( const CORE::CString& vfsPath, CAssetRecord& asset )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::TBasicVfsResourcePtr file = vfs.GetFile( vfsPath, "rb", false );
    if ( file.IsNULL() || file->GetAccess().IsNULL() )
        return ASSETSTORE_STATUS_IO_ERROR;
    CORE::IOAccessPtr access = file->GetAccess();

    CAssetTextureProps& props = asset.texture;
    CORE::CString role = CAssetNameHints::GuessTextureRole( asset.name );

    // the header where the format allows, without decoding
    CORE::CDynamicBuffer header;
    header.SetBufferSize( CAssetImageHeaderProbe::HeaderBytes );
    const UInt32 read = access->Read( header.GetBufferPtr(), 1, CAssetImageHeaderProbe::HeaderBytes );
    header.SetDataSize( read );
    CAssetImageHeaderProbe::SImageInfo info;
    if ( CAssetImageHeaderProbe::Probe( header.AsConstTypePtr< UInt8 >(), read, info ) )
    {
        props.width = info.width;
        props.height = info.height;
        props.channels = info.channels;
        props.bitDepth = info.bitDepth;
        props.compression = info.compression;
    }
    else
    {
        // a format the probe does not know: decoded by a codec of gucefIMAGE
        access->Setpos( 0 );
        IMAGE::CImage image;
        if ( !image.Load( *access, CAssetNameHints::GetExtension( vfsPath ), true, false ) || 0 == image.GetFrameCount() )
            return ASSETSTORE_STATUS_UNSUPPORTED;
        IMAGE::TPixelMapPtr pixels = image.GetPixelMap( 0, 0 );
        if ( pixels.IsNULL() )
            return ASSETSTORE_STATUS_UNSUPPORTED;
        props.width = pixels->GetWidthInPixels();
        props.height = pixels->GetHeightInPixels();
        props.channels = pixels->GetNumberOfChannelsPerPixel();
        props.bitDepth = pixels->GetPixelChannelSize() * 8;
        props.compression = "none";
        if ( role.IsNULLOrEmpty() && LooksLikeNormalMap( *pixels ) )
            role = "normal";
    }
    props.role = role;
    props.colourSpace = ( role.IsNULLOrEmpty() || role == "albedo" || role == "emissive" ) ? "srgb" : "linear";
    if ( asset.kind == "hdri" )
        props.colourSpace = "linear";
    props.isSet = true;
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
