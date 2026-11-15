/*
 *  AssetViewerLib: thumbnails of the content browser: rendered through the capture path and cached by the hash of the content
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#ifndef ASSETVIEWERLIB_CTEXTUREVIEW_H
#include "AssetViewerLib_CTextureView.h"
#define ASSETVIEWERLIB_CTEXTUREVIEW_H
#endif /* ASSETVIEWERLIB_CTEXTUREVIEW_H ? */

#include "AssetViewerLib_CThumbnailProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETVIEWER {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Raise it whenever the look of the thumbnails changes: the cache files of the version before are then not used */
static const UInt32 ViewerVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::GetViewerVersion( void )
{GUCEF_TRACE;

    return ViewerVersion;
}

/*-------------------------------------------------------------------------*/

CThumbnailProvider::CThumbnailProvider( CAssetMounter& mounter, CAssetCapture& capture )
    : GUI::CIThumbnailProvider()
    , m_mounter( mounter )
    , m_capture( capture )
    , m_cacheFolder()
    , m_queue()
    , m_cacheHits( 0 )
    , m_renders( 0 )
    , m_failures( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CThumbnailProvider::~CThumbnailProvider()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CThumbnailProvider::SetCacheFolder( const CORE::CString& folder )
{GUCEF_TRACE;

    m_cacheFolder = folder.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::RelativePath( folder );
    if ( !m_cacheFolder.IsNULLOrEmpty() && !CORE::DirExists( m_cacheFolder ) )
        CORE::CreateDirs( m_cacheFolder );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CThumbnailProvider::GetCacheFolder( void ) const
{GUCEF_TRACE;

    return m_cacheFolder;
}

/*-------------------------------------------------------------------------*/

bool
CThumbnailProvider::IsRenderable( const CORE::CString& path ) const
{GUCEF_TRACE;

    const EAssetKind kind = CAssetTypeResolver::GetKind( path );
    return ASSETKIND_MODEL == kind || ASSETKIND_TEXTURE == kind || ASSETKIND_MATERIAL == kind || ASSETKIND_SCENE == kind;
}

/*-------------------------------------------------------------------------*/

void
CThumbnailProvider::RequestThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_queue.size(); ++i )
        if ( m_queue[ i ].path == path && m_queue[ i ].size == size )
            return;
    SRequest request;
    request.path = path;
    request.size = size;
    m_queue.push_back( request );
}

/*-------------------------------------------------------------------------*/

void
CThumbnailProvider::CancelThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_queue.size(); ++i )
    {
        if ( m_queue[ i ].path == path && m_queue[ i ].size == size )
        {
            m_queue.erase( m_queue.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CThumbnailProvider::HashContent( const CORE::CString& vfsPath )
{GUCEF_TRACE;

    VFS::TBasicVfsResourcePtr resource = VFS::CVfsGlobal::Instance()->GetVfs().GetFile( CAssetMounter::NormalizePath( vfsPath ), "rb", false );
    if ( resource.IsNULL() )
        return CORE::CString::Empty;
    CORE::IOAccessPtr access = resource->GetAccess();
    if ( access.IsNULL() )
        return CORE::CString::Empty;

    // in pieces, so that a large asset is not held in memory for its hash
    CORE::CSha256 sha;
    UInt8 piece[ 65536 ];
    UInt32 total = 0;
    while ( true )
    {
        const UInt32 read = access->Read( piece, 1, sizeof( piece ) );
        if ( 0 == read )
            break;
        sha.Update( piece, read );
        total += read;
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    CORE::CString hex = CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false );
    return hex.Lowercase();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CThumbnailProvider::MakeCacheFileName( const CORE::CString& contentHash, UInt32 size )
{GUCEF_TRACE;

    return contentHash + "_v" + CORE::UInt32ToString( ViewerVersion ) + "_" + CORE::UInt32ToString( size ) + ".png";
}

/*-------------------------------------------------------------------------*/

bool
CThumbnailProvider::ReadCache( const CORE::CString& file, IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !CORE::FileExists( file ) )
        return false;
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
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CThumbnailProvider: The cache file " << file << " cannot be read: " << e.getDescription() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

void
CThumbnailProvider::Answer( const SRequest& request, const IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    TThumbnail thumbnail;
    thumbnail.path = request.path;
    thumbnail.size = request.size;
    thumbnail.pixels = pixels;
    TThumbnailReadyEventData eventData( thumbnail );
    NotifyObservers( ThumbnailReadyEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::Update( UInt32 budgetInMs )
{GUCEF_TRACE;

    const UInt64 start = MT::PrecisionTickCount();
    const UInt64 frequency = MT::PrecisionTimerResolution();
    UInt32 answered = 0;
    while ( !m_queue.empty() )
    {
        if ( answered > 0 && ( ( MT::PrecisionTickCount() - start ) * 1000 ) / frequency >= budgetInMs )
            break;
        const SRequest request = m_queue.front();
        m_queue.erase( m_queue.begin() );

        // what is no asset has no picture
        if ( !IsRenderable( request.path ) )
        {
            Answer( request, IMAGE::TPixelMapPtr() );
            ++answered;
            continue;
        }

        const CORE::CString hash = HashContent( request.path );
        IMAGE::TPixelMapPtr pixels;
        CORE::CString cacheFile;
        if ( !hash.IsNULLOrEmpty() && !m_cacheFolder.IsNULLOrEmpty() )
        {
            cacheFile = CORE::CombinePath( m_cacheFolder, MakeCacheFileName( hash, request.size ) );
            if ( ReadCache( cacheFile, pixels ) )
            {
                ++m_cacheHits;
                Answer( request, pixels );
                ++answered;
                continue;
            }
        }

        // a render: at most one per call, so that a frame is not held up by a screen of thumbnails
        if ( m_capture.IsInitialized() && m_capture.CaptureThumbnail( m_mounter, request.path, request.size, pixels ) )
        {
            ++m_renders;
            if ( !cacheFile.IsNULLOrEmpty() )
                CAssetCapture::WritePng( pixels, cacheFile );
        }
        else
        {
            ++m_failures;
            pixels.Unlink();
        }
        Answer( request, pixels );
        ++answered;
        break;
    }
    return answered;
}

/*-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::GetQueuedCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_queue.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::GetCacheHitCount( void ) const
{GUCEF_TRACE;

    return m_cacheHits;
}

/*-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::GetRenderCount( void ) const
{GUCEF_TRACE;

    return m_renders;
}

/*-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::GetFailureCount( void ) const
{GUCEF_TRACE;

    return m_failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETVIEWER */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
