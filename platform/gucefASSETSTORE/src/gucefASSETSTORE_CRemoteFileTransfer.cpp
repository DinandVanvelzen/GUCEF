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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#include "gucefASSETSTORE_CPackStorageLayout.h"
#define GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H
#endif /* GUCEF_ASSETSTORE_CPACKSTORAGELAYOUT_H ? */

#include "gucefASSETSTORE_CRemoteFileTransfer.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CRemoteFileTransfer::SItem::SItem( void )
    : servicePath(), target(), size( 0 ), digest()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteFileTransfer::CRemoteFileTransfer( EMode mode, const CORE::CString& name )
    : m_mode( mode )
    , m_name( name )
    , m_destination()
    , m_items()
    , m_folders()
    , m_pendingDigests()
    , m_requests()
    , m_listings()
    , m_recursiveListings()
    , m_discoveries()
    , m_assetCount( 0 )
    , m_assetRelations( 0 )
    , m_assetFolder()
    , m_fetchLimit( 0 )
    , m_lock()
    , m_message()
    , m_current()
    , m_state( (UInt32) STATE_QUEUED )
    , m_status( (UInt32) ASSETSTORE_STATUS_OK )
    , m_cancel( false )
    , m_bytesDone( 0 )
    , m_fetching( 0 )
    , m_bytesTotal( 0 )
    , m_bytesReused( 0 )
    , m_filesDone( 0 )
    , m_fileCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CRemoteFileTransfer::~CRemoteFileTransfer()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_discoveries.size(); ++i )
        if ( m_discoveries[ i ].owned )
            GUCEF_DELETE m_discoveries[ i ].discovery;
}

/*-------------------------------------------------------------------------*/

CRemoteFileTransfer::EMode CRemoteFileTransfer::GetMode( void ) const { return m_mode; }
const CORE::CString& CRemoteFileTransfer::GetName( void ) const { return m_name; }
const CORE::CString& CRemoteFileTransfer::GetDestination( void ) const { return m_destination; }
void CRemoteFileTransfer::SetDestination( const CORE::CString& folder ) { m_destination = folder; }
void
CRemoteFileTransfer::SetDiscovery( CIRemoteFileDiscovery* discovery, bool takeOwnership )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_discoveries.size(); ++i )
        if ( m_discoveries[ i ].owned && m_discoveries[ i ].discovery != discovery )
            GUCEF_DELETE m_discoveries[ i ].discovery;
    m_discoveries.clear();
    AddDiscovery( discovery, takeOwnership );
}

void
CRemoteFileTransfer::AddDiscovery( CIRemoteFileDiscovery* discovery, bool takeOwnership )
{GUCEF_TRACE;

    if ( GUCEF_NULL == discovery )
        return;
    SDiscovery entry;
    entry.discovery = discovery;
    entry.owned = takeOwnership;
    m_discoveries.push_back( entry );
}

void
CRemoteFileTransfer::AddFallbackDiscovery( CIRemoteFileDiscovery* discovery, bool takeOwnership )
{GUCEF_TRACE;

    if ( GUCEF_NULL == discovery )
        return;
    SDiscovery entry;
    entry.discovery = discovery;
    entry.owned = takeOwnership;
    entry.fallback = true;
    m_discoveries.push_back( entry );
}

UInt32 CRemoteFileTransfer::GetAssetRelationCount( void ) const { return m_assetRelations.Load(); }

void CRemoteFileTransfer::SetFetchLimit( UInt64 bytes ) { m_fetchLimit = bytes; }
void CRemoteFileTransfer::Cancel( void ) { m_cancel.Store( true ); }
bool CRemoteFileTransfer::IsCancelRequested( void ) const { return m_cancel.Load(); }
CRemoteFileTransfer::EState CRemoteFileTransfer::GetState( void ) const { return (EState) m_state.Load(); }
bool CRemoteFileTransfer::IsEnded( void ) const { const EState state = GetState(); return STATE_DONE == state || STATE_FAILED == state || STATE_CANCELLED == state; }
TAssetStoreStatus CRemoteFileTransfer::GetStatus( void ) const { return (TAssetStoreStatus) m_status.Load(); }
UInt64
CRemoteFileTransfer::GetBytesDone( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_bytesDone.Load() + m_fetching.Load();
}
UInt64 CRemoteFileTransfer::GetBytesTotal( void ) const { return m_bytesTotal.Load(); }
UInt64 CRemoteFileTransfer::GetBytesReused( void ) const { return m_bytesReused.Load(); }
UInt32 CRemoteFileTransfer::GetFilesDone( void ) const { return m_filesDone.Load(); }
UInt32 CRemoteFileTransfer::GetFileCount( void ) const { return m_fileCount.Load(); }

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteFileTransfer::GetMessage( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_message;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CRemoteFileTransfer::GetCurrentFile( void ) const
{
    MT::CScopeMutex lock( m_lock );
    return m_current;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::SetCurrent( const CORE::CString& file )
{
    MT::CScopeMutex lock( m_lock );
    m_current = file;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::AddFile( const CORE::CString& servicePath, const CORE::CString& targetPath, UInt64 size, const CORE::CString& digest )
{GUCEF_TRACE;

    SItem item;
    item.servicePath = CIRemoteFileSource::Normalize( servicePath );
    item.target = CIRemoteFileSource::Normalize( targetPath );
    item.size = size;
    item.digest = digest.Lowercase();
    MT::CScopeMutex lock( m_lock );
    m_items.push_back( item );
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::AddFolder( const CORE::CString& serviceFolder, const CORE::CString& targetPrefix )
{GUCEF_TRACE;

    SFolder folder;
    folder.servicePath = CIRemoteFileSource::Normalize( serviceFolder );
    folder.target = CIRemoteFileSource::Normalize( targetPrefix );
    MT::CScopeMutex lock( m_lock );
    m_folders.push_back( folder );
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::AddListing( const CORE::CString& serviceFolder, bool recursive )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    const CORE::CString folder = CIRemoteFileSource::Normalize( serviceFolder );
    m_listings.push_back( folder );
    if ( recursive )
        m_recursiveListings.insert( folder );
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::RequestPack( CIAssetStoreService& service, const CORE::CString& packId )
{GUCEF_TRACE;

    SRequest request;
    request.service = &service;
    request.id = packId;
    request.pack = true;
    MT::CScopeMutex lock( m_lock );
    m_requests.push_back( request );
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::RequestAsset( CIAssetStoreService& service, const CORE::CString& assetId )
{GUCEF_TRACE;

    SRequest request;
    request.service = &service;
    request.id = assetId;
    request.pack = false;
    MT::CScopeMutex lock( m_lock );
    m_requests.push_back( request );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileTransfer::AddPack( CIAssetStoreService& service, const CORE::CString& packId, CORE::CString& message )
{GUCEF_TRACE;

    CAssetPackRecord pack;
    TAssetStoreStatus status = service.GetPack( packId, pack );
    if ( ASSETSTORE_STATUS_OK != status )
    {
        message = "The pack " + packId + " is not in the catalogue of the store service";
        return false;
    }
    CAssetLocationPage locations;
    if ( ASSETSTORE_STATUS_OK != service.GetLocations( locations ) )
    {
        message = "The storage locations of the store service cannot be read";
        return false;
    }
    CORE::CString locationRoot;
    for ( size_t i=0; i<locations.entries.size(); ++i )
        if ( locations.entries[ i ].location.locationId == pack.locationId )
            locationRoot = locations.entries[ i ].location.vfsRoot;
    if ( locationRoot.IsNULLOrEmpty() )
    {
        message = "The pack " + pack.title + " is in a storage location that the store service does not offer";
        return false;
    }
    const CPackStorageLayout layout( pack.rootPath );
    const CORE::CString folder = layout.GetBrowseFolder( locationRoot );
    CORE::CString name = CIRemoteFileSource::Normalize( layout.GetPackFolder() );
    if ( name.IsNULLOrEmpty() )
        name = pack.title.IsNULLOrEmpty() ? packId : pack.title;
    AddFolder( folder, name );

    // the digests of the files of the assets: the catalogue has them, so the service does not hash anything for this
    CORE::CString pageToken;
    do
    {
        CAssetRecordPage assets;
        status = service.GetAssets( packId, pageToken, 0, assets );
        if ( ASSETSTORE_STATUS_OK != status )
            break;
        for ( size_t a=0; a<assets.entries.size(); ++a )
        {
            const TAssetFileRecordVector& files = assets.entries[ a ].files;
            for ( size_t f=0; f<files.size(); ++f )
            {
                if ( files[ f ].digest.IsNULLOrEmpty() || files[ f ].sizeBytes < 0 )
                    continue;
                m_pendingDigests.push_back( TDigestVector::value_type( layout.Resolve( locationRoot, files[ f ].path ), files[ f ].digest ) );
            }
        }
        pageToken = assets.nextPageToken;
    }
    while ( !pageToken.IsNULLOrEmpty() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileTransfer::AddAsset( CIAssetStoreService& service, const CORE::CString& assetId, CORE::CString& message )
{GUCEF_TRACE;

    CAssetFileSet set;
    const TAssetStoreStatus status = service.GetAssetFileSet( assetId, set );
    if ( ASSETSTORE_STATUS_OK != status || set.files.empty() )
    {
        message = "The asset " + assetId + " has no files on the store service";
        return false;
    }
    // the folder that all the files are in
    CORE::CString common = CIRemoteFileSource::Normalize( set.files[ 0 ].path );
    {
        const Int32 slash = common.HasChar( '/', false );
        common = slash < 0 ? CORE::CString::Empty : common.SubstrFromRange( 0, (UInt32) slash );
    }
    for ( size_t i=1; i<set.files.size(); ++i )
    {
        const CORE::CString path = CIRemoteFileSource::Normalize( set.files[ i ].path );
        while ( !common.IsNULLOrEmpty() && !( 0 == path.HasSubstr( common + "/" ) ) )
        {
            const Int32 slash = common.HasChar( '/', false );
            common = slash < 0 ? CORE::CString::Empty : common.SubstrFromRange( 0, (UInt32) slash );
        }
    }
    for ( size_t i=0; i<set.files.size(); ++i )
    {
        const CORE::CString path = CIRemoteFileSource::Normalize( set.files[ i ].path );
        CORE::CString target = path;
        if ( !common.IsNULLOrEmpty() && 0 == path.HasSubstr( common + "/" ) )
            target = path.CutChars( common.Length() + 1, true );
        AddFile( path, target, set.files[ i ].sizeBytes > 0 ? (UInt64) set.files[ i ].sizeBytes : 0, set.files[ i ].digest );
    }
    // the relations the set followed decide whether the fallback discoveries are asked; they see the set
    {
        MT::CScopeMutex lock( m_lock );
        if ( 0 == m_assetCount )
            m_assetFolder = common;
        ++m_assetCount;
    }
    m_assetRelations.FetchAdd( set.relationCount );
    for ( size_t d=0; d<m_discoveries.size(); ++d )
        if ( m_discoveries[ d ].fallback )
            m_discoveries[ d ].discovery->OnAssetFileSet( set );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRemoteFileTransfer::Finish( EState state, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_message = message;
        m_current.Clear();
    }
    m_status.Store( (UInt32) status );
    m_state.Store( (UInt32) state );
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileTransfer::Expand( CIRemoteFileSource& system )
{GUCEF_TRACE;

    // the digests that the catalogue gave are known to the file system before the folders are listed
    TDigestVector digests;
    TFolderVector folders;
    {
        MT::CScopeMutex lock( m_lock );
        digests = m_pendingDigests;
        folders = m_folders;
        m_pendingDigests.clear();
        m_folders.clear();
    }
    for ( size_t i=0; i<digests.size(); ++i )
        system.NoteDigest( digests[ i ].first, digests[ i ].second );

    for ( size_t f=0; f<folders.size(); ++f )
    {
        CORE::CString::StringVector pending;
        pending.push_back( CORE::CString::Empty );
        while ( !pending.empty() )
        {
            if ( IsCancelRequested() )
                return false;
            const CORE::CString relative = pending.back();
            pending.pop_back();
            const CORE::CString folder = relative.IsNULLOrEmpty() ? folders[ f ].servicePath : folders[ f ].servicePath + "/" + relative;
            CORE::CString::StringVector dirs;
            CORE::CString::StringVector files;
            bool truncated = false;
            if ( !system.GetFolderEntries( folder, dirs, files, &truncated ) )
            {
                Finish( STATE_FAILED, system.IsOffline() ? ASSETSTORE_STATUS_UNAVAILABLE : ASSETSTORE_STATUS_NOT_FOUND,
                        system.IsOffline() ? "The folder " + folder + " cannot be listed: the store service cannot be reached" : "The folder " + folder + " is not on the store service" );
                return false;
            }
            for ( size_t d=0; d<dirs.size(); ++d )
                pending.push_back( relative.IsNULLOrEmpty() ? dirs[ d ] : relative + "/" + dirs[ d ] );
            for ( size_t n=0; n<files.size(); ++n )
            {
                const CORE::CString servicePath = folder + "/" + files[ n ];
                CIRemoteFileSource::SFileInfo info;
                if ( !system.GetFileInfo( servicePath, info ) )
                    continue;
                CORE::CString target = relative.IsNULLOrEmpty() ? files[ n ] : relative + "/" + files[ n ];
                if ( !folders[ f ].target.IsNULLOrEmpty() )
                    target = folders[ f ].target + "/" + target;
                SItem item;
                item.servicePath = servicePath;
                item.target = target;
                item.size = info.size;
                item.digest = info.digest;
                MT::CScopeMutex lock( m_lock );
                m_items.push_back( item );
            }
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileTransfer::HashFile( const CORE::CString& path, CORE::CString& hex )
{GUCEF_TRACE;

    CORE::CFileAccess file( path, "rb" );
    if ( !file.Opened() )
        return false;
    CORE::CSha256 sha;
    GUCEF::vector< UInt8 > piece;
    piece.resize( ChunkBytes );
    while ( true )
    {
        const UInt32 read = file.Read( &piece[ 0 ], 1, ChunkBytes );
        if ( 0 == read )
            break;
        sha.Update( &piece[ 0 ], read );
    }
    UInt8 digest[ CORE::CSha256::DigestSize ];
    sha.Final( digest );
    hex = CORE::ConvertBytesToHexString( digest, CORE::CSha256::DigestSize, false, false ).Lowercase();
    return true;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteFileTransfer::Run( CIRemoteFileSource& system )
{GUCEF_TRACE;

    m_state.Store( (UInt32) STATE_LISTING );
    TRequestVector requests;
    {
        MT::CScopeMutex lock( m_lock );
        requests = m_requests;
        m_requests.clear();
    }
    for ( size_t r=0; r<requests.size(); ++r )
    {
        CORE::CString message;
        const bool added = requests[ r ].pack ? AddPack( *requests[ r ].service, requests[ r ].id, message ) : AddAsset( *requests[ r ].service, requests[ r ].id, message );
        if ( !added )
        {
            Finish( STATE_FAILED, ASSETSTORE_STATUS_NOT_FOUND, message );
            return GetStatus();
        }
    }
    if ( MODE_DOWNLOAD == m_mode && m_destination.IsNULLOrEmpty() )
    {
        Finish( STATE_FAILED, ASSETSTORE_STATUS_INVALID_ARGUMENT, "No folder is given to download to" );
        return GetStatus();
    }
    if ( !Expand( system ) )
    {
        if ( IsCancelRequested() && !IsEnded() )
            Finish( STATE_CANCELLED, ASSETSTORE_STATUS_CANCELLED, "Cancelled" );
        return GetStatus();
    }
    TItemVector items;
    {
        MT::CScopeMutex lock( m_lock );
        items = m_items;
    }
    UInt64 total = 0;
    for ( size_t i=0; i<items.size(); ++i )
    {
        // a size that is not known is asked of the service
        if ( 0 == items[ i ].size )
        {
            CIRemoteFileSource::SFileInfo info;
            if ( system.GetFileInfo( items[ i ].servicePath, info ) && !info.isDirectory )
            {
                items[ i ].size = info.size;
                if ( items[ i ].digest.IsNULLOrEmpty() )
                    items[ i ].digest = info.digest;
            }
        }
        total += items[ i ].size;
    }
    m_bytesTotal.Store( total );
    m_fileCount.Store( (UInt32) items.size() );
    m_state.Store( (UInt32) STATE_RUNNING );

    // the folders of the files are listed, so the sizes and kinds that the views ask for are known without a wait
    if ( MODE_PREFETCH == m_mode || MODE_KEEP_OFFLINE == m_mode )
    {
        CORE::CString::StringSet warmed;
        CORE::CString::StringVector listings;
        CORE::CString::StringSet recursiveListings;
        {
            MT::CScopeMutex lock( m_lock );
            listings = m_listings;
            recursiveListings = m_recursiveListings;
        }

        // the folders below a folder that is listed recursively are read as well, a bound keeps a folder with very many folders from taking the transfer
        static const UInt32 maxListingDepth = 6;
        static const UInt32 maxListedFolders = 256;
        UInt32 listedFolders = 0;
        for ( size_t i=0; i<listings.size(); ++i )
        {
            if ( !warmed.insert( listings[ i ] ).second )
                continue;
            GUCEF::vector< std::pair< CORE::CString, UInt32 > > pending;
            pending.push_back( std::make_pair( listings[ i ], (UInt32) 0 ) );
            const bool recursive = recursiveListings.find( listings[ i ] ) != recursiveListings.end();
            while ( !pending.empty() && !IsCancelRequested() && listedFolders < maxListedFolders )
            {
                const CORE::CString folder = pending.back().first;
                const UInt32 depth = pending.back().second;
                pending.pop_back();
                CORE::CString::StringVector dirs;
                CORE::CString::StringVector names;
                system.GetFolderEntries( folder, dirs, names );
                ++listedFolders;
                if ( !recursive || depth >= maxListingDepth )
                    continue;
                for ( size_t d=0; d<dirs.size(); ++d )
                {
                    const CORE::CString child = folder.IsNULLOrEmpty() ? dirs[ d ] : folder + "/" + dirs[ d ];
                    if ( warmed.insert( child ).second )
                        pending.push_back( std::make_pair( child, depth + 1 ) );
                }
            }
        }
        for ( size_t i=0; i<items.size(); ++i )
        {
            const Int32 slash = items[ i ].servicePath.HasChar( '/', false );
            const CORE::CString parent = slash < 0 ? CORE::CString::Empty : items[ i ].servicePath.SubstrFromRange( 0, (UInt32) slash );
            if ( warmed.insert( parent ).second )
            {
                CORE::CString::StringVector dirs;
                CORE::CString::StringVector names;
                system.GetFolderEntries( parent, dirs, names );
            }
        }
    }

    for ( size_t i=0; i<items.size(); ++i )
    {
        if ( IsCancelRequested() )
        {
            Finish( STATE_CANCELLED, ASSETSTORE_STATUS_CANCELLED, "Cancelled after " + CORE::ToString( GetFilesDone() ) + " of " + CORE::ToString( (UInt32) items.size() ) + " files" );
            return GetStatus();
        }
        if ( m_fetchLimit > 0 && ( m_bytesDone.Load() - m_bytesReused.Load() ) >= m_fetchLimit )
            break;
        SetCurrent( items[ i ].target.IsNULLOrEmpty() ? items[ i ].servicePath : items[ i ].target );
        CORE::CString message;
        const TAssetStoreStatus status = RunItem( system, items[ i ], message );
        if ( ASSETSTORE_STATUS_OK != status )
        {
            Finish( ASSETSTORE_STATUS_CANCELLED == status ? STATE_CANCELLED : STATE_FAILED, status, message );
            return GetStatus();
        }
        m_filesDone.FetchAdd( 1 );
    }

    // what the application knows to be needed as well, now that the files are there to read: the discoveries of a prefetch, and the
    // fallback discoveries of assets that have no relations (their companion files are not in their file sets)
    if ( !m_discoveries.empty() && !IsCancelRequested() && MODE_RELEASE_OFFLINE != m_mode )
    {
        const bool useFallback = m_assetCount > 0 && 0 == m_assetRelations.Load();
        CORE::CString::StringVector more;
        for ( size_t d=0; d<m_discoveries.size(); ++d )
            if ( m_discoveries[ d ].fallback ? useFallback : MODE_PREFETCH == m_mode )
                m_discoveries[ d ].discovery->Discover( system, more );
        CORE::CString::StringSet known;
        for ( size_t i=0; i<items.size(); ++i )
            known.insert( items[ i ].servicePath );
        for ( size_t i=0; i<more.size() && !IsCancelRequested(); ++i )
        {
            SItem item;
            item.servicePath = CIRemoteFileSource::Normalize( more[ i ] );
            if ( !known.insert( item.servicePath ).second )
                continue;
            if ( !m_assetFolder.IsNULLOrEmpty() && 0 == item.servicePath.HasSubstr( m_assetFolder + "/" ) )
                item.target = item.servicePath.CutChars( m_assetFolder.Length() + 1, true );
            CIRemoteFileSource::SFileInfo info;
            if ( !system.GetFileInfo( item.servicePath, info ) || info.isDirectory )
                continue;
            item.size = info.size;
            item.digest = info.digest;
            m_bytesTotal.FetchAdd( item.size );
            m_fileCount.FetchAdd( 1 );
            SetCurrent( item.servicePath );
            CORE::CString message;
            const TAssetStoreStatus status = RunItem( system, item, message );
            if ( ASSETSTORE_STATUS_OK != status )
            {
                Finish( ASSETSTORE_STATUS_CANCELLED == status ? STATE_CANCELLED : STATE_FAILED, status, message );
                return GetStatus();
            }
            m_filesDone.FetchAdd( 1 );
        }
    }
    if ( IsCancelRequested() )
    {
        Finish( STATE_CANCELLED, ASSETSTORE_STATUS_CANCELLED, "Cancelled" );
        return GetStatus();
    }
    const CORE::CString verb = MODE_DOWNLOAD == m_mode ? "Downloaded " : MODE_KEEP_OFFLINE == m_mode ? "Kept offline " : MODE_RELEASE_OFFLINE == m_mode ? "Released " : "Fetched ";
    Finish( STATE_DONE, ASSETSTORE_STATUS_OK, verb + CORE::ToString( GetFilesDone() ) + " files" );
    return ASSETSTORE_STATUS_OK;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteFileTransfer::RunItem( CIRemoteFileSource& system, const SItem& item, CORE::CString& message )
{GUCEF_TRACE;

    if ( MODE_RELEASE_OFFLINE == m_mode )
    {
        system.UnpinFile( item.servicePath );
        m_bytesDone.FetchAdd( item.size );
        return ASSETSTORE_STATUS_OK;
    }
    if ( MODE_DOWNLOAD == m_mode )
        return DownloadItem( system, item, message );

    // prefetch and keep offline: through the cache, a link that went away is waited for
    const bool pin = MODE_KEEP_OFFLINE == m_mode;
    if ( system.IsFileCached( item.servicePath, item.size ) && !pin )
    {
        m_bytesDone.FetchAdd( item.size );
        m_bytesReused.FetchAdd( item.size );
        return ASSETSTORE_STATUS_OK;
    }
    UInt32 delayMs = 1000;
    for ( UInt32 attempt=1; ; ++attempt )
    {
        const UInt64 doneBefore = m_bytesDone.Load();
        const bool cachedBefore = system.IsFileCached( item.servicePath, item.size );
        // the bytes of the file come in while it is fetched: the progress shows them as they arrive
        m_fetching.Store( 0 );
        const TAssetStoreStatus status = system.FetchWholeFile( item.servicePath, pin, &m_cancel, &m_fetching, message );
        {
            MT::CScopeMutex lock( m_lock );
            m_bytesDone.FetchAdd( m_fetching.Load() );
            m_fetching.Store( 0 );
        }
        if ( cachedBefore || ASSETSTORE_STATUS_OK == status )
        {
            // what was in the cache already counts as done
            const UInt64 counted = m_bytesDone.Load() - doneBefore;
            if ( ASSETSTORE_STATUS_OK == status && counted < item.size )
            {
                m_bytesDone.FetchAdd( item.size - counted );
                m_bytesReused.FetchAdd( item.size - counted );
            }
        }
        if ( ASSETSTORE_STATUS_OK == status )
            return status;
        if ( ASSETSTORE_STATUS_CANCELLED == status || !IsRetryable( status ) || attempt >= MaxAttempts )
            return status;
        // the service cannot be reached: waited for, in pieces so a cancel is seen
        for ( UInt32 waited=0; waited<delayMs && !IsCancelRequested(); waited+=50 )
            MT::PrecisionDelay( 50 );
        if ( IsCancelRequested() )
            return ASSETSTORE_STATUS_CANCELLED;
        delayMs *= 2;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRemoteFileTransfer::IsRetryable( TAssetStoreStatus status )
{
    return ASSETSTORE_STATUS_UNAVAILABLE == status || ASSETSTORE_STATUS_NETWORK_ERROR == status || ASSETSTORE_STATUS_TIMED_OUT == status;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteFileTransfer::DownloadItem( CIRemoteFileSource& system, const SItem& item, CORE::CString& message )
{GUCEF_TRACE;

    const CORE::CString target = item.target.IsNULLOrEmpty() ? item.servicePath : item.target;
    const CORE::CString destination = CORE::CombinePath( m_destination, target );
    const CORE::CString part = destination + ".part";
    CORE::CString folder = CORE::StripFilename( destination );
    if ( !folder.IsNULLOrEmpty() && !CORE::DirExists( folder ) && !CORE::CreateDirs( folder ) )
    {
        message = "The folder " + folder + " cannot be made";
        return ASSETSTORE_STATUS_IO_ERROR;
    }

    // a file that is there already, whole and right, is left alone
    if ( CORE::FileExists( destination ) && CORE::FileSize( destination ) == item.size )
    {
        CORE::CString have;
        if ( item.digest.IsNULLOrEmpty() || ( HashFile( destination, have ) && have == item.digest ) )
        {
            m_bytesDone.FetchAdd( item.size );
            m_bytesReused.FetchAdd( item.size );
            return ASSETSTORE_STATUS_OK;
        }
    }

    bool fromStart = false;
    UInt32 delayMs = 1000;
    for ( UInt32 attempt=1; ; ++attempt )
    {
        bool partWasBad = false;
        const TAssetStoreStatus status = DownloadAttempt( system, item, part, fromStart, partWasBad, message );
        if ( ASSETSTORE_STATUS_OK == status )
        {
            if ( !CORE::MoveFileAtomically( part, destination, true ) )
            {
                message = "The file " + destination + " cannot be put in its place";
                return ASSETSTORE_STATUS_IO_ERROR;
            }
            return ASSETSTORE_STATUS_OK;
        }
        if ( ASSETSTORE_STATUS_DIGEST_MISMATCH == status && !fromStart )
        {
            // the part that was continued was not right: once more from the start
            fromStart = true;
            continue;
        }
        if ( ASSETSTORE_STATUS_CANCELLED == status || !IsRetryable( status ) || attempt >= MaxAttempts )
            return status;
        for ( UInt32 waited=0; waited<delayMs && !IsCancelRequested(); waited+=50 )
            MT::PrecisionDelay( 50 );
        if ( IsCancelRequested() )
            return ASSETSTORE_STATUS_CANCELLED;
        delayMs *= 2;
    }
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CRemoteFileTransfer::DownloadAttempt( CIRemoteFileSource& system, const SItem& item, const CORE::CString& part, bool fromStart, bool& partWasBad, CORE::CString& message )
{GUCEF_TRACE;

    // where the part of an earlier run ends: the last whole block, so a torn tail is never kept
    UInt64 start = 0;
    if ( !fromStart && CORE::FileExists( part ) )
    {
        const UInt64 partSize = CORE::FileSize( part );
        start = ( partSize / PartAlignBytes ) * PartAlignBytes;
        if ( start > item.size )
            start = 0;
    }
    if ( 0 == start )
        CORE::DeleteFile( part );
    else
    if ( CORE::FileSize( part ) != start )
        CORE::TruncateFile( part, start );

    CIRemoteFileSource::SFileInfo source;
    if ( !system.GetFileInfo( item.servicePath, source ) || source.isDirectory )
    {
        message = source.isDirectory ? "The path " + item.servicePath + " is a folder of the store service" :
                  system.IsOffline() ? "The file " + item.servicePath + " is not available offline: it is not in the cache of this computer and the store service cannot be reached" :
                                       "The file " + item.servicePath + " is not on the store service (any more)";
        return system.IsOffline() ? ASSETSTORE_STATUS_UNAVAILABLE : ASSETSTORE_STATUS_NOT_FOUND;
    }
    if ( item.size != 0 && source.size != item.size )
    {
        message = "The file " + item.servicePath + " is not the size the catalogue says (" + CORE::ToString( source.size ) + " bytes where " + CORE::ToString( item.size ) + " are expected)";
        return ASSETSTORE_STATUS_SIZE_MISMATCH;
    }
    CORE::CFileAccess out( part, start > 0 ? "r+b" : "wb" );
    if ( !out.Opened() )
    {
        message = "The file " + part + " cannot be written";
        return ASSETSTORE_STATUS_IO_ERROR;
    }
    out.Setpos( start );
    if ( start > 0 )
    {
        m_bytesDone.FetchAdd( start );
        m_bytesReused.FetchAdd( start );
    }
    CORE::CDynamicBuffer chunk;
    UInt64 offset = start;
    TAssetStoreStatus result = ASSETSTORE_STATUS_OK;
    while ( offset < source.size )
    {
        if ( IsCancelRequested() )
        {
            result = ASSETSTORE_STATUS_CANCELLED;
            message = "Cancelled: the part of " + item.target + " is kept to continue";
            break;
        }
        CORE::CString readMessage;
        const TAssetStoreStatus readStatus = system.ReadFileRange( item.servicePath, offset, ChunkBytes, chunk, readMessage );
        const UInt32 read = chunk.GetDataSize();
        if ( 0 == read )
        {
            result = ASSETSTORE_STATUS_OK == readStatus ? ASSETSTORE_STATUS_IO_ERROR : readStatus;
            message = readMessage;
            break;
        }
        if ( read != out.Write( chunk.GetConstBufferPtr(), 1, read ) )
        {
            result = ASSETSTORE_STATUS_IO_ERROR;
            message = "The file " + part + " cannot be written: the disk may be full";
            break;
        }
        offset += read;
        m_bytesDone.FetchAdd( read );
    }
    out.Flush();
    out.Close();
    if ( ASSETSTORE_STATUS_OK != result )
        return result;

    if ( !item.digest.IsNULLOrEmpty() )
    {
        CORE::CString have;
        if ( !HashFile( part, have ) || have != item.digest )
        {
            partWasBad = true;
            // what was counted for this attempt is taken back: the file is fetched again
            m_bytesDone.FetchAdd( (UInt64) 0 - offset );
            CORE::DeleteFile( part );
            message = "The file " + item.target + " does not have the SHA-256 that the catalogue gives" + CORE::CString( fromStart ? " (also after it was fetched again)" : "" );
            return ASSETSTORE_STATUS_DIGEST_MISMATCH;
        }
    }
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
