/*
 *  AssetWorkbenchLib: automatic thumbnails
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

#include <math.h>
#include <string.h>
#include <algorithm>

#include <Ogre.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#include "gucefIMAGE_CPixelMapResampler.h"
#define GUCEF_IMAGE_CPIXELMAPRESAMPLER_H
#endif /* GUCEF_IMAGE_CPIXELMAPRESAMPLER_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETCAPTURE_H
#include "AssetWorkbenchLib_CAssetCapture.h"
#define ASSETWORKBENCHLIB_CASSETCAPTURE_H
#endif /* ASSETWORKBENCHLIB_CASSETCAPTURE_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETMOUNTER_H
#include "AssetWorkbenchLib_CAssetMounter.h"
#define ASSETWORKBENCHLIB_CASSETMOUNTER_H
#endif /* ASSETWORKBENCHLIB_CASSETMOUNTER_H ? */

#ifndef ASSETWORKBENCHLIB_CTEXTUREVIEW_H
#include "AssetWorkbenchLib_CTextureView.h"
#define ASSETWORKBENCHLIB_CTEXTUREVIEW_H
#endif /* ASSETWORKBENCHLIB_CTEXTUREVIEW_H ? */

#ifndef ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#include "AssetWorkbenchLib_CThumbnailProvider.h"
#define ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H
#endif /* ASSETWORKBENCHLIB_CTHUMBNAILPROVIDER_H ? */

#include "AssetWorkbenchLib_CAutoThumbnailStore.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAutoThumbnailStore::SQuality::SQuality( void )
    : deviation( 0.0f ), coverage( 0.0f ), maximum( 0.0f ), worthwhile( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAutoThumbnailStore::CAutoThumbnailStore( void )
    : m_folder(), m_enabled( true ), m_stored( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAutoThumbnailStore::SetFolder( const CORE::CString& folder )
{GUCEF_TRACE;

    m_folder = folder.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::RelativePath( folder );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CAutoThumbnailStore::GetFolder( void ) const { return m_folder; }
void CAutoThumbnailStore::SetEnabled( bool enabled ) { m_enabled = enabled; }
bool CAutoThumbnailStore::IsEnabled( void ) const { return m_enabled; }
UInt32 CAutoThumbnailStore::GetStoredCount( void ) const { return m_stored; }

/*-------------------------------------------------------------------------*/

CORE::CString
CAutoThumbnailStore::MakeFileName( const CORE::CString& contentHash, UInt32 size )
{GUCEF_TRACE;

    const CORE::CString real = CThumbnailProvider::MakeCacheFileName( contentHash, size );
    return real.SubstrFromRange( 0, real.Length() - 4 ) + "_auto.png";
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::IsAutoFileName( const CORE::CString& name )
{GUCEF_TRACE;

    return name.Length() > 9 && name.SubstrFromRange( name.Length() - 9, name.Length() ) == "_auto.png";
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::IsWorthwhile( const IMAGE::TPixelMapPtr& pixels, SQuality* quality )
{GUCEF_TRACE;

    SQuality local;
    SQuality& result = GUCEF_NULL != quality ? *quality : local;
    result = SQuality();
    if ( pixels.IsNULL() || pixels->GetWidthInPixels() < 2 || pixels->GetHeightInPixels() < 2 )
        return false;
    const UInt32 width = pixels->GetWidthInPixels();
    const UInt32 height = pixels->GetHeightInPixels();
    const UInt8* data = static_cast< const UInt8* >( pixels->GetDataPtr() );
    const size_t count = (size_t) width * height;

    // the background is the colour of the four corners
    const UInt8* corners[ 4 ] = { data, data + ( width - 1 ) * 4, data + (size_t) ( height - 1 ) * width * 4, data + ( (size_t) height * width - 1 ) * 4 };
    Float32 background[ 3 ] = { 0.0f, 0.0f, 0.0f };
    for ( int c=0; c<4; ++c )
        for ( int k=0; k<3; ++k )
            background[ k ] += (Float32) corners[ c ][ k ] * 0.25f;

    Float64 sum = 0.0, sumSquares = 0.0;
    size_t different = 0;
    UInt8 maximum = 0;
    for ( size_t i=0; i<count; ++i )
    {
        const UInt8* p = data + i * 4;
        const Float64 brightness = 0.299 * p[ 0 ] + 0.587 * p[ 1 ] + 0.114 * p[ 2 ];
        sum += brightness;
        sumSquares += brightness * brightness;
        const Float32 dr = (Float32) p[ 0 ] - background[ 0 ], dg = (Float32) p[ 1 ] - background[ 1 ], db = (Float32) p[ 2 ] - background[ 2 ];
        if ( fabsf( dr ) + fabsf( dg ) + fabsf( db ) > 36.0f )
            ++different;
        for ( int k=0; k<3; ++k )
            if ( p[ k ] > maximum )
                maximum = p[ k ];
    }
    const Float64 mean = sum / (Float64) count;
    const Float64 variance = sumSquares / (Float64) count - mean * mean;
    result.deviation = (Float32) sqrt( variance > 0.0 ? variance : 0.0 );
    result.coverage = (Float32) different / (Float32) count;
    result.maximum = (Float32) maximum;
    // a flat frame (a black one, an empty one) has no deviation; a picture of a model has some, and the model covers a part of it
    result.worthwhile = result.deviation >= 4.0f && result.coverage >= 0.02f && maximum >= 24;
    return result.worthwhile;
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::FitToSquare( const IMAGE::TPixelMapPtr& pixels, UInt32 size, IMAGE::TPixelMapPtr& fitted )
{GUCEF_TRACE;

    if ( pixels.IsNULL() || 0 == size )
        return false;
    const UInt32 width = pixels->GetWidthInPixels();
    const UInt32 height = pixels->GetHeightInPixels();
    const Float32 scale = (Float32) size / (Float32) ( width > height ? width : height );
    UInt32 targetWidth = (UInt32) ( (Float32) width * scale + 0.5f );
    UInt32 targetHeight = (UInt32) ( (Float32) height * scale + 0.5f );
    if ( targetWidth < 1 ) targetWidth = 1;
    if ( targetHeight < 1 ) targetHeight = 1;
    IMAGE::TPixelMapPtr scaled = pixels;
    if ( targetWidth != width || targetHeight != height )
        if ( !IMAGE::CPixelMapResampler::Resample( *pixels, targetWidth, targetHeight, IMAGE::RESAMPLEFILTER_TRIANGLE, IMAGE::COLOURSPACE_SRGB, IMAGE::WRAPMODE_CLAMP, scaled ) )
            return false;
    if ( scaled->GetPixelStorageFormat() != IMAGE::PSF_RGBA )
        return false;
    fitted = IMAGE::TPixelMapPtr( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, size, size, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
    UInt8 red = 0, green = 0, blue = 0;
    CAssetCapture::GetSheetColour( red, green, blue );
    UInt8* target = static_cast< UInt8* >( fitted->GetDataPtr() );
    for ( size_t i=0; i<(size_t) size*size; ++i )
    {
        target[ i*4 ] = red;
        target[ i*4+1 ] = green;
        target[ i*4+2 ] = blue;
        target[ i*4+3 ] = 255;
    }
    const UInt32 left = ( size - targetWidth ) / 2, top = ( size - targetHeight ) / 2;
    for ( UInt32 row=0; row<targetHeight; ++row )
        memcpy( target + ( (size_t) ( top + row ) * size + left ) * 4, static_cast< const UInt8* >( scaled->GetDataPtr() ) + (size_t) row * targetWidth * 4, (size_t) targetWidth * 4 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::HasRealThumbnail( const CORE::CString& contentHash, UInt32 size ) const
{GUCEF_TRACE;

    return !m_folder.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( m_folder, CThumbnailProvider::MakeCacheFileName( contentHash, size ) ) );
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::Has( const CORE::CString& contentHash, UInt32 size ) const
{GUCEF_TRACE;

    return !m_folder.IsNULLOrEmpty() && CORE::FileExists( CORE::CombinePath( m_folder, MakeFileName( contentHash, size ) ) );
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::Store( const CORE::CString& contentHash, const IMAGE::TPixelMapPtr& pixels, bool loadHadErrors, CORE::CString& reason, UInt32 size )
{GUCEF_TRACE;

    reason.Clear();
    if ( !m_enabled )
        reason = "automatic thumbnails are switched off";
    else
    if ( m_folder.IsNULLOrEmpty() || contentHash.IsNULLOrEmpty() )
        reason = "there is no folder or content hash";
    else
    if ( loadHadErrors )
        reason = "the load had errors";
    else
    if ( HasRealThumbnail( contentHash, size ) )
        reason = "the content has a thumbnail of its own";
    else
    if ( Has( contentHash, size ) )
        reason = "an automatic thumbnail is there already";
    if ( !reason.IsNULLOrEmpty() )
        return false;
    SQuality quality;
    if ( !IsWorthwhile( pixels, &quality ) )
    {
        reason = "the picture is not worthwhile (deviation " + CORE::FloatToString( quality.deviation ) + ", coverage " + CORE::FloatToString( quality.coverage ) + ")";
        return false;
    }
    IMAGE::TPixelMapPtr fitted;
    if ( !FitToSquare( pixels, size, fitted ) )
    {
        reason = "the picture cannot be scaled";
        return false;
    }
    if ( !CORE::DirExists( m_folder ) )
        CORE::CreateDirs( m_folder );
    if ( !CAssetCapture::WritePng( fitted, CORE::CombinePath( m_folder, MakeFileName( contentHash, size ) ) ) )
    {
        reason = "the file cannot be written";
        return false;
    }
    ++m_stored;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAutoThumbnailStore::Read( const CORE::CString& contentHash, UInt32 size, IMAGE::TPixelMapPtr& pixels ) const
{GUCEF_TRACE;

    if ( !Has( contentHash, size ) )
        return false;
    const CORE::CString file = CORE::CombinePath( m_folder, MakeFileName( contentHash, size ) );
    CORE::CDynamicBuffer data;
    if ( !data.LoadContentFromFile( file ) || 0 == data.GetDataSize() )
        return false;
    try
    {
        Ogre::DataStreamPtr stream( OGRE_NEW Ogre::MemoryDataStream( data.GetBufferPtr(), data.GetDataSize(), false, true ) );
        Ogre::Image image;
        image.load( stream, "png" );
        CTextureView view;
        if ( !view.SetImage( image, file ) )
            return false;
        pixels = view.GetFirstPixelMap();
        return !pixels.IsNULL();
    }
    catch ( const Ogre::Exception& e )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CAutoThumbnailStore: The file " << file << " cannot be read: " << e.getDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CAutoThumbnailStore::Clear( void )
{GUCEF_TRACE;

    UInt32 deleted = 0;
    if ( m_folder.IsNULLOrEmpty() || !CORE::DirExists( m_folder ) )
        return 0;
    GUCEF::vector< CORE::CString > names;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( m_folder ) )
    {
        do
        {
            if ( iterator.IsAFile() && IsAutoFileName( iterator.GetResourceName() ) )
                names.push_back( iterator.GetResourceName() );
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    for ( size_t i=0; i<names.size(); ++i )
        if ( CORE::DeleteFile( CORE::CombinePath( m_folder, names[ i ] ) ) )
            ++deleted;
    return deleted;
}

/*-------------------------------------------------------------------------*/

UInt32
CAutoThumbnailStore::GetCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    if ( m_folder.IsNULLOrEmpty() || !CORE::DirExists( m_folder ) )
        return 0;
    CORE::CFileSystemIterator iterator;
    if ( iterator.FindFirst( m_folder ) )
    {
        do
        {
            if ( iterator.IsAFile() && IsAutoFileName( iterator.GetResourceName() ) )
                ++count;
        }
        while ( iterator.FindNext() );
        iterator.FindClose();
    }
    return count;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAutoThumbnailStore::HashPack( const CORE::CString& vfsFolder )
{GUCEF_TRACE;

    VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
    VFS::CVFS::TStringVector listed;
    const CORE::CString folder = CAssetMounter::NormalizePath( vfsFolder );
    if ( !vfs.GetFileList( listed, folder, true, true, CORE::CString::Empty, 200000 ) || listed.empty() )
        return CORE::CString::Empty;
    GUCEF::vector< CORE::CString > paths;
    for ( size_t i=0; i<listed.size(); ++i )
        paths.push_back( CAssetMounter::NormalizePath( listed[ i ] ) );
    std::sort( paths.begin(), paths.end() );
    CORE::CSha256 sha;
    for ( size_t i=0; i<paths.size(); ++i )
    {
        const CORE::CString line = paths[ i ] + "\t" + CORE::UInt64ToString( vfs.GetFileSize( paths[ i ] ) ) + "\n";
        sha.Update( line.C_String(), line.Length() );
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    return CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false ).Lowercase();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
