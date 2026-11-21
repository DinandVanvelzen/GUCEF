/*
 *  AssetWorkbenchLib: thumbnails of the content browser: rendered through the capture path and cached by the hash of the content
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

#ifndef GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#include "gucefVFS_CVfsThreadPolicyScope.h"
#define GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H
#endif /* GUCEF_VFS_CVFSTHREADPOLICYSCOPE_H ? */

#ifndef GUCEF_VFS_CIREMOTEARCHIVE_H
#include "gucefVFS_CIRemoteArchive.h"
#define GUCEF_VFS_CIREMOTEARCHIVE_H
#endif /* GUCEF_VFS_CIREMOTEARCHIVE_H ? */

#ifndef ASSETWORKBENCHLIB_CTEXTUREVIEW_H
#include "AssetWorkbenchLib_CTextureView.h"
#define ASSETWORKBENCHLIB_CTEXTUREVIEW_H
#endif /* ASSETWORKBENCHLIB_CTEXTUREVIEW_H ? */

#include "AssetWorkbenchLib_CThumbnailProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Raise it whenever the look of the thumbnails changes: the cache files of the version before are then not used */
static const UInt32 WorkbenchVersion = 1;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

UInt32
CThumbnailProvider::GetWorkbenchVersion( void )
{GUCEF_TRACE;

    return WorkbenchVersion;
}

/*-------------------------------------------------------------------------*/

CThumbnailProvider::CThumbnailProvider( CAssetMounter& mounter, CAssetCapture& capture )
    : GUI::CIThumbnailProvider()
    , m_mounter( mounter )
    , m_capture( capture )
    , m_auto( GUCEF_NULL )
    , m_autoHits( 0 )
    , m_packHashes()
    , m_cacheFolder()
    , m_queue()
    , m_waitingRemote()
    , m_remote( GUCEF_NULL )
    , m_remoteHits( 0 )
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
    return CAssetTypeResolver::HasView( path ) && ( ASSETKIND_MODEL == kind || ASSETKIND_TEXTURE == kind || ASSETKIND_IMAGE == kind || ASSETKIND_MATERIAL == kind || ASSETKIND_SCENE == kind );
}

/*-------------------------------------------------------------------------*/

void
CThumbnailProvider::SetAutoThumbnails( CAutoThumbnailStore* store )
{GUCEF_TRACE;

    m_auto = store;
}

/*-------------------------------------------------------------------------*/

UInt32 CThumbnailProvider::GetAutoHitCount( void ) const { return m_autoHits; }
UInt32 CThumbnailProvider::GetRemoteHitCount( void ) const { return m_remoteHits; }

/*-------------------------------------------------------------------------*/

void
CThumbnailProvider::SetRemoteSource( CRemoteThumbnailSource* source )
{GUCEF_TRACE;

    m_remote = source;
    // what waited for the service is asked again
    for ( size_t i=0; i<m_waitingRemote.size(); ++i )
        m_queue.push_back( m_waitingRemote[ i ] );
    m_waitingRemote.clear();
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
    request.existingOnly = false;
    m_queue.push_back( request );
}

/*-------------------------------------------------------------------------*/
void
CThumbnailProvider::RequestExistingThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_queue.size(); ++i )
        if ( m_queue[ i ].path == path && m_queue[ i ].size == size )
            return;
    SRequest request;
    request.path = path;
    request.size = size;
    request.existingOnly = true;
    m_queue.push_back( request );
}

/*-------------------------------------------------------------------------*/


void
CThumbnailProvider::CancelThumbnail( const CORE::CString& path, UInt32 size )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_waitingRemote.size(); ++i )
    {
        if ( m_waitingRemote[ i ].path == path && m_waitingRemote[ i ].size == size )
        {
            m_waitingRemote.erase( m_waitingRemote.begin() + i );
            if ( GUCEF_NULL != m_remote )
                m_remote->Cancel( path, size );
            break;
        }
    }

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

    // a file of a remote mount of the VFS: the SHA-256 that the share or the catalogue gave is the hash of its content, no read is needed for it
    // (from what is held: a thread that may wait does not ask the service for it either)
    {
        VFS::CVFS& vfs = VFS::CVfsGlobal::Instance()->GetVfs();
        const CORE::CString path = CAssetMounter::NormalizePath( vfsPath );
        VFS::CVFS::TArchivePtr archive;
        CORE::CString remainder;
        if ( vfs.ResolveMount( path, archive, remainder ) && !archive.IsNULL() && GUCEF_NULL != archive->GetRemoteArchiveInterface() )
        {
            VFS::CVfsThreadPolicy policy;
            policy.missAction = VFS::MISSACTION_FAIL;
            VFS::CVfsThreadPolicyScope scope( policy );
            VFS::CVfsDigest digest;
            if ( vfs.GetFileDigest( path, digest ) && VFS::CVfsDigest::ALGORITHM_SHA256 == digest.algorithm && !digest.IsEmpty() )
                return digest.value.Lowercase();
        }
    }

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

    return contentHash + "_v" + CORE::UInt32ToString( WorkbenchVersion ) + "_" + CORE::UInt32ToString( size ) + ".png";
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

    // the requests that wait for the service: asked again when its answer is in (a picture, or none)
    for ( size_t w=0; w<m_waitingRemote.size(); )
    {
        CORE::CString waitingFile;
        if ( GUCEF_NULL == m_remote || CRemoteThumbnailSource::LOOKUP_PENDING != m_remote->Lookup( m_waitingRemote[ w ].path, m_waitingRemote[ w ].size, waitingFile ) )
        {
            m_queue.insert( m_queue.begin(), m_waitingRemote[ w ] );
            m_waitingRemote.erase( m_waitingRemote.begin() + w );
        }
        else
            ++w;
    }

    while ( !m_queue.empty() )
    {
        if ( answered > 0 && ( ( MT::PrecisionTickCount() - start ) * 1000 ) / frequency >= budgetInMs )
            break;
        const SRequest request = m_queue.front();
        m_queue.erase( m_queue.begin() );

        // a file of a remote store service: its picture is the one the service stores, else it is rendered here when its files are at hand
        if ( GUCEF_NULL != m_remote && m_remote->IsRemotePath( request.path ) )
        {
            CORE::CString remoteFile;
            const CRemoteThumbnailSource::ELookup found = m_remote->Lookup( request.path, request.size, remoteFile );
            if ( CRemoteThumbnailSource::LOOKUP_READY == found )
            {
                IMAGE::TPixelMapPtr remotePixels;
                if ( ReadCache( remoteFile, remotePixels ) )
                {
                    ++m_remoteHits;
                    Answer( request, remotePixels );
                    ++answered;
                    continue;
                }
            }
            else
            if ( CRemoteThumbnailSource::LOOKUP_PENDING == found )
            {
                m_waitingRemote.push_back( request );
                continue;
            }
            if ( !m_remote->IsFileAvailable( request.path ) )
            {
                Answer( request, IMAGE::TPixelMapPtr() );
                ++answered;
                continue;
            }
        }

        // the root of a pack has the automatic thumbnail of its presentation, when it has one
        if ( GUCEF_NULL != m_auto && !CAssetTypeResolver::HasView( request.path ) )
        {
            CAssetMounter::SMount packMount;
            CORE::CString packRelative;
            if ( m_mounter.FindMount( request.path, packMount, packRelative ) && packRelative.IsNULLOrEmpty() )
            {
                const CORE::CString normalized = CAssetMounter::NormalizePath( request.path );
                GUCEF::map< CORE::CString, CORE::CString >::iterator known = m_packHashes.find( normalized );
                if ( known == m_packHashes.end() )
                    known = m_packHashes.insert( std::make_pair( normalized, CAutoThumbnailStore::HashPack( normalized ) ) ).first;
                IMAGE::TPixelMapPtr autoPixels;
                if ( !(*known).second.IsNULLOrEmpty() && m_auto->Read( (*known).second, request.size, autoPixels ) )
                {
                    ++m_autoHits;
                    Answer( request, autoPixels );
                    ++answered;
                    continue;
                }
            }
        }

        // what is no asset has no picture
        if ( !IsRenderable( request.path ) )
        {
            Answer( request, IMAGE::TPixelMapPtr() );
            ++answered;
            continue;
        }

        // a file of the computer that is not in a mount (the content browser lists those too) is not read for a picture
        CAssetMounter::SMount mount;
        CORE::CString mountRelative;
        if ( !m_mounter.FindMount( request.path, mount, mountRelative ) )
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

        // the automatic thumbnail of a model that has no thumbnail of its own yet
        if ( !hash.IsNULLOrEmpty() && GUCEF_NULL != m_auto && m_auto->Read( hash, request.size, pixels ) )
        {
            ++m_autoHits;
            Answer( request, pixels );
            ++answered;
            continue;
        }

        // a request for what exists has no more to look at: no render for it
        if ( request.existingOnly )
        {
            Answer( request, IMAGE::TPixelMapPtr() );
            ++answered;
            continue;
        }

        // the render of a file of a remote store: Ogre reads the folders of a place when its resource group is made, so the pack of the file
        // gets its own group now that its files are at hand (the lists of its folders are known to the cache from the fetch)
        if ( GUCEF_NULL != m_remote && m_remote->IsRemotePath( request.path ) )
        {
            CORE::CString packFolder;
            if ( m_remote->GetPackFolder( request.path, packFolder ) )
                m_mounter.AdoptRemotePack( packFolder, CORE::CString::StringVector() );
        }

        // a render: at most one per call, so that a frame is not held up by a screen of thumbnails
        if ( m_capture.IsInitialized() && m_capture.CaptureThumbnail( m_mounter, request.path, request.size, pixels ) )
        {
            ++m_renders;
            if ( !cacheFile.IsNULLOrEmpty() )
            {
                CAssetCapture::WritePng( pixels, cacheFile );
                // the render of a file of a remote store goes to the service, so that the other devices have the picture
                if ( GUCEF_NULL != m_remote && m_remote->IsRemotePath( request.path ) )
                {
                    CORE::CDynamicBuffer png;
                    if ( png.LoadContentFromFile( cacheFile ) )
                        m_remote->Upload( request.path, request.size, png );
                }
            }
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

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
