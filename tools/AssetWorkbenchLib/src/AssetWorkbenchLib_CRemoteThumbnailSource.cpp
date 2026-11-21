/*
 *  AssetWorkbenchLib: the thumbnails of a remote asset store service
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETCAPTURE_H
#include "AssetWorkbenchLib_CAssetCapture.h"
#define ASSETWORKBENCHLIB_CASSETCAPTURE_H
#endif /* ASSETWORKBENCHLIB_CASSETCAPTURE_H ? */

#ifndef ASSETWORKBENCHLIB_CASSETMOUNTER_H
#include "AssetWorkbenchLib_CAssetMounter.h"
#define ASSETWORKBENCHLIB_CASSETMOUNTER_H
#endif /* ASSETWORKBENCHLIB_CASSETMOUNTER_H ? */

#ifndef GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H
#include "gucefASSETSTORE_CAssetThumbnailFiles.h"
#define GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H
#endif /* GUCEF_ASSETSTORE_CASSETTHUMBNAILFILES_H ? */

#ifndef GUCEF_VFS_CVFSGLOBAL_H
#include "gucefVFS_CVfsGlobal.h"
#define GUCEF_VFS_CVFSGLOBAL_H
#endif /* GUCEF_VFS_CVFSGLOBAL_H ? */

#ifndef GUCEF_VFS_CVFS_H
#include "gucefVFS_CVFS.h"
#define GUCEF_VFS_CVFS_H
#endif /* GUCEF_VFS_CVFS_H ? */

#include "AssetWorkbenchLib_CRemoteThumbnailSource.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETWORKBENCH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CRemoteThumbnailSource::ThumbnailReadyEvent = "GUCEF::ASSETWORKBENCH::CRemoteThumbnailSource::ThumbnailReadyEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteThumbnailSource::SReady::SReady( void ) : path(), size( 0 ), found( false ) {}
CRemoteThumbnailSource::SWanted::SWanted( void ) : assetId(), size( 0 ), paths(), queuedMs( 0 ), sent( false ) {}
CRemoteThumbnailSource::SBatch::SBatch( void ) : future( CORE::TASKSTATUS_UNDEFINED ), size( 0 ), ids() {}
CRemoteThumbnailSource::SPicture::SPicture( void ) : future( CORE::TASKSTATUS_UNDEFINED ), result(), assetId(), size( 0 ) {}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::RegisterEvents( void )
{GUCEF_TRACE;

    ThumbnailReadyEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CRemoteThumbnailSource::CRemoteThumbnailSource( void )
    : CORE::CObservingNotifier()
    , m_service( GUCEF_NULL )
    , m_system( GUCEF_NULL )
    , m_folder()
    , m_assetOfPath()
    , m_missing()
    , m_wanted()
    , m_batches()
    , m_pictures()
    , m_uploads()
    , m_requested( 0 )
    , m_fetched( 0 )
    , m_missingCount( 0 )
    , m_batchCount( 0 )
    , m_uploadCount( 0 )
    , m_uploadNumber( 0 )
    , m_mayUpload( true )
    , m_renderedAssets()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CRemoteThumbnailSource::~CRemoteThumbnailSource()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRemoteThumbnailSource::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETWORKBENCH::CRemoteThumbnailSource";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

UInt64
CRemoteThumbnailSource::NowMs( void )
{
    const UInt64 resolution = MT::PrecisionTimerResolution();
    return resolution > 0 ? ( MT::PrecisionTickCount() * 1000ULL ) / resolution : 0;
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::SetService( ASSETSTORE::CIAssetStoreService* service, ASSETSTORE::CRemoteAssetShares* system )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service )
        UnsubscribeFrom( m_service );
    m_service = service;
    m_system = system;
    m_wanted.clear();
    m_batches.clear();
    m_pictures.clear();
    m_missing.clear();
    m_assetOfPath.clear();
    m_uploads.clear();
    if ( GUCEF_NULL != m_service )
    {
        TEventCallback callback( this, &CRemoteThumbnailSource::OnThumbnailUpdated );
        SubscribeTo( m_service, ASSETSTORE::CIAssetStoreService::ThumbnailUpdatedEvent, callback );
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::SetCacheFolder( const CORE::CString& folder )
{GUCEF_TRACE;

    m_folder = folder.IsNULLOrEmpty() ? CORE::CString::Empty : CORE::RelativePath( folder );
    if ( !m_folder.IsNULLOrEmpty() && !CORE::DirExists( m_folder ) )
        CORE::CreateDirs( m_folder );
}

/*-------------------------------------------------------------------------*/

const CORE::CString& CRemoteThumbnailSource::GetCacheFolder( void ) const { return m_folder; }

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteThumbnailSource::ToClientPath( const CORE::CString& servicePath ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_system ? m_system->ToClientPath( servicePath ) : servicePath;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteThumbnailSource::IsRemotePath( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CString servicePath;
    return GUCEF_NULL != m_system && m_system->ToServicePath( vfsPath, servicePath );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteThumbnailSource::IsFileAvailable( const CORE::CString& vfsPath ) const
{GUCEF_TRACE;

    CORE::CString servicePath;
    if ( GUCEF_NULL == m_system || !m_system->ToServicePath( vfsPath, servicePath ) )
        return false;
    ASSETSTORE::CIRemoteFileSource::SFileInfo info;
    return m_system->GetFileInfo( servicePath, info ) && !info.isDirectory && m_system->IsFileCached( servicePath, info.size );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteThumbnailSource::GetPackFolder( const CORE::CString& vfsPath, CORE::CString& packFolder ) const
{GUCEF_TRACE;

    CORE::CString servicePath;
    if ( GUCEF_NULL == m_system || !m_system->ToServicePath( vfsPath, servicePath ) )
        return false;
    const CORE::CString::StringVector segments = servicePath.ParseElements( '/', false );
    for ( size_t i=0; i+2<segments.size(); ++i )
    {
        if ( !ASSETSTORE::CPackStorageLayout::IsAreaName( segments[ i ] ) )
            continue;
        CORE::CString folder;
        for ( size_t s=0; s<=i+1; ++s )
            folder += ( 0 == s ? CORE::CString::Empty : CORE::CString( "/" ) ) + segments[ s ];
        packFolder = CAssetMounter::NormalizePath( m_system->ToClientPath( folder ) );
        return !packFolder.IsNULLOrEmpty();
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::NoteAsset( const CORE::CString& vfsPath, const CORE::CString& assetId )
{GUCEF_TRACE;

    if ( vfsPath.IsNULLOrEmpty() || assetId.IsNULLOrEmpty() )
        return;
    if ( m_assetOfPath.size() > 20000 )
        m_assetOfPath.clear();
    m_assetOfPath[ ASSETSTORE::CIRemoteFileSource::Normalize( vfsPath ) ] = assetId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteThumbnailSource::MakeCacheFile( const CORE::CString& assetId, UInt32 size ) const
{GUCEF_TRACE;

    ASSETSTORE::CAssetThumbnailKey key;
    key.digest = 0 == assetId.HasSubstr( "pack:" ) ? ASSETSTORE::CAssetThumbnailFiles::MakePackDigest( assetId.CutChars( 5, true ) ) : assetId;
    key.rendererId = "remote";
    key.rendererVersion = "1";
    key.size = size;
    return CORE::CombinePath( m_folder, ASSETSTORE::CAssetThumbnailFiles::MakeFileName( key ) );
}

/*-------------------------------------------------------------------------*/

CRemoteThumbnailSource::ELookup
CRemoteThumbnailSource::Lookup( const CORE::CString& vfsPath, UInt32 size, CORE::CString& cacheFile )
{GUCEF_TRACE;

    cacheFile.Clear();
    if ( GUCEF_NULL == m_service || m_folder.IsNULLOrEmpty() )
        return LOOKUP_NONE;
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator known = m_assetOfPath.find( ASSETSTORE::CIRemoteFileSource::Normalize( vfsPath ) );
    if ( known == m_assetOfPath.end() )
        return LOOKUP_NONE;
    const CORE::CString& assetId = known->second;
    const CORE::CString file = MakeCacheFile( assetId, size );
    if ( CORE::FileExists( file ) )
    {
        cacheFile = file;
        return LOOKUP_READY;
    }
    GUCEF::map< CORE::CString, UInt64 >::const_iterator missing = m_missing.find( assetId + "\n" + CORE::ToString( size ) );
    if ( missing != m_missing.end() && NowMs() - missing->second < (UInt64) NegativeMs )
        return LOOKUP_NONE;
    for ( size_t i=0; i<m_wanted.size(); ++i )
    {
        if ( m_wanted[ i ].assetId == assetId && m_wanted[ i ].size == size )
        {
            for ( size_t p=0; p<m_wanted[ i ].paths.size(); ++p )
                if ( m_wanted[ i ].paths[ p ] == vfsPath )
                    return LOOKUP_PENDING;
            m_wanted[ i ].paths.push_back( vfsPath );
            return LOOKUP_PENDING;
        }
    }
    SWanted wanted;
    wanted.assetId = assetId;
    wanted.size = size;
    wanted.paths.push_back( vfsPath );
    wanted.queuedMs = NowMs();
    m_wanted.push_back( wanted );
    ++m_requested;
    return LOOKUP_PENDING;
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::Cancel( const CORE::CString& vfsPath, UInt32 size )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_wanted.size(); ++i )
    {
        if ( m_wanted[ i ].size != size || m_wanted[ i ].sent )
            continue;
        for ( size_t p=0; p<m_wanted[ i ].paths.size(); ++p )
        {
            if ( m_wanted[ i ].paths[ p ] == vfsPath )
            {
                m_wanted[ i ].paths.erase( m_wanted[ i ].paths.begin() + p );
                break;
            }
        }
        if ( m_wanted[ i ].paths.empty() )
        {
            m_wanted.erase( m_wanted.begin() + i );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::Finish( const CORE::CString& assetId, UInt32 size, bool found )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_wanted.size(); ++i )
    {
        if ( m_wanted[ i ].assetId != assetId || m_wanted[ i ].size != size )
            continue;
        if ( !found )
            m_missing[ assetId + "\n" + CORE::ToString( size ) ] = NowMs();
        const GUCEF::vector< CORE::CString > paths = m_wanted[ i ].paths;
        m_wanted.erase( m_wanted.begin() + i );
        for ( size_t p=0; p<paths.size(); ++p )
        {
            SReady ready;
            ready.path = paths[ p ];
            ready.size = size;
            ready.found = found;
            TReadyEventData data( ready );
            NotifyObservers( ThumbnailReadyEvent, &data );
        }
        return;
    }
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::Update( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_service )
        return;

    // the answers of the batches: a picture to read for each asset that has one, the others are over
    for ( size_t b=0; b<m_batches.size(); )
    {
        SBatch& batch = m_batches[ b ];
        if ( batch.future.HasAFuture() && !batch.future.IsReady() )
        {
            ++b;
            continue;
        }
        ASSETSTORE::CAssetThumbnailPage page;
        const bool got = ASSETSTORE::ASSETSTORE_STATUS_OK == ASSETSTORE::CIAssetStoreService::GetStatusOfFuture( batch.future ) &&
                         ASSETSTORE::CIAssetStoreService::TryGetFutureResult( batch.future, page );
        const UInt32 size = batch.size;
        const CORE::CString::StringVector ids = batch.ids;
        m_batches.erase( m_batches.begin() + b );
        for ( size_t i=0; i<ids.size(); ++i )
        {
            const ASSETSTORE::CAssetThumbnail* entry = GUCEF_NULL;
            if ( got )
                for ( size_t e=0; e<page.entries.size(); ++e )
                    if ( page.entries[ e ].assetId == ids[ i ] )
                        entry = &page.entries[ e ];
            if ( GUCEF_NULL == entry || ASSETSTORE::ASSETSTORE_STATUS_OK != entry->status || entry->vfsPath.IsNULLOrEmpty() )
            {
                ++m_missingCount;
                Finish( ids[ i ], size, false );
                continue;
            }
            SPicture picture;
            picture.assetId = ids[ i ];
            picture.size = size;
            // the picture file through the VFS: one more byte than a picture may have tells a file that is too large
            if ( GUCEF_NULL != m_system )
                picture.future = VFS::CVfsGlobal::Instance()->GetVfs().ReadFileRangeAsync( m_system->ToClientPath( entry->vfsPath ), 0, (UInt32) MaxPictureBytes + 1, picture.result );
            m_pictures.push_back( picture );
        }
    }

    // the pictures that arrived are kept
    for ( size_t p=0; p<m_pictures.size(); )
    {
        SPicture& picture = m_pictures[ p ];
        if ( picture.future.HasAFuture() && !picture.future.IsReady() )
        {
            ++p;
            continue;
        }
        bool found = false;
        if ( !picture.result.IsNULL() && picture.result->result.IsSuccess() && picture.result->data.GetDataSize() > 0 && picture.result->data.GetDataSize() <= (UInt32) MaxPictureBytes )
        {
            const CORE::CDynamicBuffer& bytes = picture.result->data;
            CORE::CFileAccess file( MakeCacheFile( picture.assetId, picture.size ), "wb" );
            found = file.Opened() && bytes.GetDataSize() == file.Write( bytes.GetConstBufferPtr(), 1, bytes.GetDataSize() );
        }
        const CORE::CString assetId = picture.assetId;
        const UInt32 size = picture.size;
        m_pictures.erase( m_pictures.begin() + p );
        if ( found )
            ++m_fetched;
        else
            ++m_missingCount;
        Finish( assetId, size, found );
    }

    // the requests that are due: one call per size, at most MaxBatch assets, after the time to coalesce
    if ( m_batches.size() >= (size_t) MaxInFlight || m_wanted.empty() )
        return;
    const UInt64 now = NowMs();
    UInt32 size = 0;
    UInt32 count = 0;
    bool due = false;
    for ( size_t i=0; i<m_wanted.size(); ++i )
    {
        if ( m_wanted[ i ].sent )
            continue;
        if ( 0 == size )
            size = m_wanted[ i ].size;
        if ( m_wanted[ i ].size != size )
            continue;
        ++count;
        if ( now - m_wanted[ i ].queuedMs >= (UInt64) CoalesceMs )
            due = true;
    }
    if ( 0 == count || ( !due && count < (UInt32) MaxBatch ) )
        return;
    SBatch batch;
    batch.size = size;
    for ( size_t i=0; i<m_wanted.size() && batch.ids.size() < (size_t) MaxBatch; ++i )
    {
        if ( m_wanted[ i ].sent || m_wanted[ i ].size != size )
            continue;
        m_wanted[ i ].sent = true;
        batch.ids.push_back( m_wanted[ i ].assetId );
    }
    batch.future = m_service->GetThumbnailsAsync( batch.ids, size );
    ++m_batchCount;
    m_batches.push_back( batch );
}

/*-------------------------------------------------------------------------*/

void CRemoteThumbnailSource::SetMayUpload( bool mayUpload ) { m_mayUpload = mayUpload; }
bool CRemoteThumbnailSource::GetMayUpload( void ) const { return m_mayUpload; }

/*-------------------------------------------------------------------------*/

bool
CRemoteThumbnailSource::Upload( const CORE::CString& vfsPath, UInt32 size, const CORE::CDynamicBuffer& png )
{GUCEF_TRACE;

    // the service takes pictures from the owner devices only: no call, no refusal to log
    if ( !m_mayUpload )
        return false;
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator known = m_assetOfPath.find( ASSETSTORE::CIRemoteFileSource::Normalize( vfsPath ) );
    if ( GUCEF_NULL == m_service || known == m_assetOfPath.end() || 0 == png.GetDataSize() || png.GetDataSize() > (UInt32) MaxPictureBytes )
        return false;
    ASSETSTORE::CAssetThumbnail thumbnail;
    thumbnail.assetId = known->second;
    thumbnail.kind = 0 == known->second.HasSubstr( "pack:" ) ? "auto" : "thumbnail";
    thumbnail.size = size;
    thumbnail.key.size = size;
    thumbnail.key.rendererId = "workbench";
    thumbnail.key.rendererVersion = "1";
    thumbnail.encoded = png;
    // the picture is on the disk already: what the next Lookup() finds
    if ( !m_folder.IsNULLOrEmpty() )
    {
        CORE::CFileAccess file( MakeCacheFile( known->second, size ), "wb" );
        if ( file.Opened() )
            file.Write( png.GetConstBufferPtr(), 1, png.GetDataSize() );
    }
    // the ones that were sent are over: the list does not grow
    for ( size_t i=0; i<m_uploads.size(); )
    {
        if ( m_uploads[ i ].IsReady() || !m_uploads[ i ].HasAFuture() )
            m_uploads.erase( m_uploads.begin() + i );
        else
            ++i;
    }
    if ( m_uploads.size() >= 16 )
        return false;
    m_uploads.push_back( m_service->PutThumbnailAsync( "wb-thumb-" + CORE::ToString( ++m_uploadNumber ) + "-" + known->second + "-" + CORE::ToString( size ), thumbnail ) );
    ++m_uploadCount;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteThumbnailSource::UploadRender( const CORE::CString& vfsPath, const IMAGE::TPixelMapPtr& pixels )
{GUCEF_TRACE;

    if ( !m_mayUpload )
        return false;
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator known = m_assetOfPath.find( ASSETSTORE::CIRemoteFileSource::Normalize( vfsPath ) );
    if ( GUCEF_NULL == m_service || known == m_assetOfPath.end() || pixels.IsNULL() || m_folder.IsNULLOrEmpty() )
        return false;
    if ( !m_renderedAssets.insert( known->second ).second )
        return false;
    const UInt32 size = pixels->GetWidthInPixels();
    const CORE::CString file = MakeCacheFile( known->second, size );
    CORE::CDynamicBuffer png;
    if ( !CAssetCapture::WritePng( pixels, file ) || !png.LoadContentFromFile( file ) )
        return false;
    return Upload( vfsPath, size, png );
}

/*-------------------------------------------------------------------------*/

void
CRemoteThumbnailSource::OnThumbnailUpdated( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a picture was put or replaced on the service: the pictures that are kept for the asset are asked for again
    const ASSETSTORE::CAssetThumbnail* thumbnail = CORE::CICloneable::CloneableAs< ASSETSTORE::CAssetThumbnail >( eventData );
    if ( GUCEF_NULL == thumbnail )
        return;
    GUCEF::map< CORE::CString, UInt64 >::iterator i = m_missing.begin();
    while ( i != m_missing.end() )
    {
        if ( 0 == i->first.HasSubstr( thumbnail->assetId + "\n" ) )
            m_missing.erase( i++ );
        else
            ++i;
    }
}

UInt32 CRemoteThumbnailSource::GetRequestedCount( void ) const { return m_requested; }
UInt32 CRemoteThumbnailSource::GetFetchedCount( void ) const { return m_fetched; }
UInt32 CRemoteThumbnailSource::GetMissingCount( void ) const { return m_missingCount; }
UInt32 CRemoteThumbnailSource::GetBatchCount( void ) const { return m_batchCount; }
UInt32 CRemoteThumbnailSource::GetUploadCount( void ) const { return m_uploadCount; }
UInt32 CRemoteThumbnailSource::GetPendingCount( void ) const { return (UInt32) m_wanted.size(); }

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETWORKBENCH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
