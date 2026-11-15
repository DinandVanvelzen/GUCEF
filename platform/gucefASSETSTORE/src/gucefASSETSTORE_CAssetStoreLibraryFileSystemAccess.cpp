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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */
#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */
#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */
#ifndef GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#include "gucefCORE_CFileSystemEntryListIterator.h"
#define GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H
#endif /* GUCEF_CORE_CFILESYSTEMENTRYLISTITERATOR_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#include "gucefASSETSTORE_CAssetStoreLibraryPage.h"
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYPAGE_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREQUERY_H
#include "gucefASSETSTORE_CAssetStoreQuery.h"
#define GUCEF_ASSETSTORE_CASSETSTOREQUERY_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREQUERY_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#include "gucefASSETSTORE_CAssetStoreSafeNames.h"
#define GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H
#endif /* GUCEF_ASSETSTORE_CASSETSTORESAFENAMES_H ? */
#include "gucefASSETSTORE_CAssetStoreLibraryFileSystemAccess.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CAssetStoreLibraryFileSystemAccess::LibraryChangedEvent = "GUCEF::ASSETSTORE::CAssetStoreLibraryFileSystemAccess::LibraryChangedEvent";
const CORE::CEvent CAssetStoreLibraryFileSystemAccess::RefreshFailedEvent = "GUCEF::ASSETSTORE::CAssetStoreLibraryFileSystemAccess::RefreshFailedEvent";

const CORE::CString CAssetStoreLibraryFileSystemAccess::OtherListingTypeName = "other";
const CORE::CString CAssetStoreLibraryFileSystemAccess::UnknownSellerName = "unknown seller";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The time between two looks at the page that is being fetched */
const UInt32 PollIntervalInMs = 10;

/** The bytes of the text without the terminating zero, for every string format of CString */
inline UInt32
ByteLengthOf( const CORE::CString& text )
{
    const UInt32 size = text.ByteSize();
    return size > 0 ? size - 1 : 0;
}

/** The text with the characters that cannot be in a name replaced */
CORE::CString
MakeNameSafe( const CORE::CString& text )
{GUCEF_TRACE;

    GUCEF::vector< char > bytes;
    const char* chars = text.C_String();
    const UInt32 length = ByteLengthOf( text );
    for ( UInt32 i=0; i<length; ++i )
    {
        unsigned char c = (unsigned char) chars[ i ];
        if ( c < 0x20 || 0x7F == c || '/' == c || '\\' == c )
            c = '_';
        bytes.push_back( (char) c );
    }
    return bytes.empty() ? CORE::CString() : CORE::CString( &bytes[ 0 ], (UInt32) bytes.size() );
}

CORE::CString
JoinPath( const CORE::CString& parent, const CORE::CString& name )
{GUCEF_TRACE;

    if ( parent.IsNULLOrEmpty() || "/" == parent )
        return "/" + name;
    return parent + "/" + name;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAssetStoreLibraryFileSystemAccess::SCache::SCache( void )
    : items()
    , entries()
    , entryByPath()
    , children()
    , entryByItemId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::SCache::Clear( void )
{GUCEF_TRACE;

    items.clear();
    entries.clear();
    entryByPath.clear();
    children.clear();
    entryByItemId.clear();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::SCache::Swap( SCache& other )
{GUCEF_TRACE;

    items.swap( other.items );
    entries.swap( other.entries );
    entryByPath.swap( other.entryByPath );
    children.swap( other.children );
    entryByItemId.swap( other.entryByItemId );
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreLibraryFileSystemAccess::SCache::AddEntry( const CORE::CString& name, const CORE::CString& path, bool isDirectory, UInt64 size, Int32 itemIndex )
{GUCEF_TRACE;

    SEntry entry;
    entry.name = name;
    entry.path = path;
    entry.isDirectory = isDirectory;
    entry.size = size;
    entry.itemIndex = itemIndex;
    entries.push_back( entry );
    const UInt32 index = (UInt32) ( entries.size() - 1 );
    entryByPath[ path ] = index;
    if ( isDirectory )
        children[ path ];
    return index;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreLibraryFileSystemAccess::SCache::EnsureFolder( const CORE::CString& name, const CORE::CString& path, const CORE::CString& parentPath )
{GUCEF_TRACE;

    TIndexMap::const_iterator i = entryByPath.find( path );
    if ( i != entryByPath.end() )
        return (*i).second;
    const UInt32 index = AddEntry( name, path, true, 0, -1 );
    children[ parentPath ][ name ] = index;
    return index;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::RegisterEvents( void )
{GUCEF_TRACE;

    LibraryChangedEvent.Initialize();
    RefreshFailedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreLibraryFileSystemAccess::CAssetStoreLibraryFileSystemAccess( const TAssetStoreProviderPtr& provider )
    : CORE::IFileSystemAccess()
    , CORE::CTSGNotifier()
    , m_provider( provider )
    , m_maxItems( DefaultMaxItems )
    , m_pageSize( DefaultPageSize )
    , m_queryText()
    , m_listingTypeFilter()
    , m_cache()
    , m_truncated( false )
    , m_loading( false )
    , m_generation( 0 )
    , m_lastStatus( ASSETSTORE_STATUS_OK )
    , m_lastMessage()
    , m_collected()
    , m_collectedIds()
    , m_lastCursor()
    , m_pageFuture( CORE::TASKSTATUS_QUEUEING_FAILED )
    , m_pageQueued( false )
    , m_timer( CORE::CCoreGlobal::Instance()->GetPulseGenerator(), PollIntervalInMs )
    , m_lock()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CAssetStoreLibraryFileSystemAccess::CAssetStoreLibraryFileSystemAccess( const TAssetStoreProviderPtr& provider, CORE::PulseGeneratorPtr pulseGenerator )
    : CORE::IFileSystemAccess()
    , CORE::CTSGNotifier( pulseGenerator )
    , m_provider( provider )
    , m_maxItems( DefaultMaxItems )
    , m_pageSize( DefaultPageSize )
    , m_queryText()
    , m_listingTypeFilter()
    , m_cache()
    , m_truncated( false )
    , m_loading( false )
    , m_generation( 0 )
    , m_lastStatus( ASSETSTORE_STATUS_OK )
    , m_lastMessage()
    , m_collected()
    , m_collectedIds()
    , m_lastCursor()
    , m_pageFuture( CORE::TASKSTATUS_QUEUEING_FAILED )
    , m_pageQueued( false )
    , m_timer( pulseGenerator, PollIntervalInMs )
    , m_lock()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();
    BuildCache( TAssetStoreItemVector(), m_cache );
    m_timer.SetEnabled( false );
    TEventCallback timerCallback( this, &CAssetStoreLibraryFileSystemAccess::OnTimer );
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent, timerCallback );
}

/*-------------------------------------------------------------------------*/

CAssetStoreLibraryFileSystemAccess::~CAssetStoreLibraryFileSystemAccess()
{GUCEF_TRACE;

    SignalUpcomingDestruction();
    m_timer.SetEnabled( false );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreLibraryFileSystemAccess::GetProviderId( void ) const
{GUCEF_TRACE;

    static const CORE::CString noProvider;
    return m_provider.IsNULL() ? noProvider : m_provider->GetProviderId();
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::SetMaxItems( UInt32 maxItems )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_maxItems = 0 == maxItems ? (UInt32) DefaultMaxItems : maxItems;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreLibraryFileSystemAccess::GetMaxItems( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxItems;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::SetPageSize( UInt32 pageSize )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_pageSize = 0 == pageSize ? (UInt32) DefaultPageSize : pageSize;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreLibraryFileSystemAccess::GetPageSize( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_pageSize;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::SetQueryText( const CORE::CString& text )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_queryText = text;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetQueryText( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_queryText;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::SetListingTypeFilter( const CORE::CString& listingType )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_listingTypeFilter = listingType;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetListingTypeFilter( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_listingTypeFilter;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryFileSystemAccess::Refresh( void )
{GUCEF_TRACE;

    UInt64 generation = 0;
    {
        MT::CScopeMutex lock( m_lock );
        generation = ++m_generation;
        m_loading = true;
        m_collected.clear();
        m_collectedIds.clear();
        m_lastCursor.Clear();
        m_pageQueued = false;
    }
    if ( m_provider.IsNULL() )
    {
        FailRefresh( generation, ASSETSTORE_STATUS_NOT_FOUND, "the library has no store" );
        return false;
    }
    if ( !QueueNextPage( CORE::CString::Empty ) )
    {
        FailRefresh( generation, ASSETSTORE_STATUS_FAILED, "the library query could not be queued" );
        return false;
    }
    m_timer.SetEnabled( true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryFileSystemAccess::QueueNextPage( const CORE::CString& cursor )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CAssetStoreQuery query;
    query.text = m_queryText;
    query.listingType = m_listingTypeFilter;
    query.pageSize = m_pageSize;
    query.cursor = cursor;
    m_lastCursor = cursor;

    CORE::CFutureResult future = m_provider->QueryLibraryAsync( query );
    if ( future.HasNoFuture() )
        return false;
    m_pageFuture = future;
    m_pageQueued = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    CORE::CFutureResult future( CORE::TASKSTATUS_QUEUEING_FAILED );
    UInt64 generation = 0;
    bool idle = false;
    bool ready = false;
    {
        MT::CScopeMutex lock( m_lock );
        idle = !m_loading;
        ready = m_loading && m_pageQueued && m_pageFuture.IsReady();
        if ( ready )
        {
            m_pageQueued = false;
            future = m_pageFuture;
            generation = m_generation;
        }
    }
    if ( idle )
    {
        // the timer stops when nothing is loading; a refresh that started meanwhile keeps it
        m_timer.SetEnabled( false );
        bool loadingAgain = false;
        {
            MT::CScopeMutex lock( m_lock );
            loadingAgain = m_loading;
        }
        if ( loadingAgain )
            m_timer.SetEnabled( true );
    }
    if ( !ready )
        return;

    CORE::CTaskPtr task = future.GetResult( 0 );
    const TAssetStoreStatus status = CIAssetStoreProvider::GetStatusOfFuture( future );
    const CAssetStoreLibraryPage* page = task.IsNULL() ? GUCEF_NULL : CORE::CICloneable::CloneableAs< CAssetStoreLibraryPage >( task->GetTaskData() );
    if ( ASSETSTORE_STATUS_OK != status || GUCEF_NULL == page )
    {
        CORE::CString message = task.IsNULL() ? CORE::CString::Empty : task->GetTaskStatusExtraInfo();
        if ( message.IsNULLOrEmpty() )
            message = AssetStoreStatusToString( ASSETSTORE_STATUS_OK == status ? ASSETSTORE_STATUS_FAILED : status );
        FailRefresh( generation, ASSETSTORE_STATUS_OK == status ? ASSETSTORE_STATUS_FAILED : status, message );
        return;
    }

    // the page joins what was collected, items seen before and items beyond the bound are left out
    CORE::CString nextCursor;
    TAssetStoreItemVector collected;
    bool truncated = false;
    bool more = false;
    {
        MT::CScopeMutex lock( m_lock );
        if ( generation != m_generation || !m_loading )
            return;
        UInt32 added = 0;
        for ( size_t i=0; i<page->items.size(); ++i )
        {
            const CAssetStoreItem& item = page->items[ i ];
            if ( item.itemId.IsNULLOrEmpty() || m_collectedIds.end() != m_collectedIds.find( item.itemId ) )
                continue;
            if ( m_collected.size() >= m_maxItems )
            {
                truncated = true;
                break;
            }
            m_collectedIds.insert( item.itemId );
            m_collected.push_back( item );
            ++added;
        }
        nextCursor = page->nextCursor;
        const bool bound = m_collected.size() >= m_maxItems;
        if ( bound && !nextCursor.IsNULLOrEmpty() )
            truncated = true;
        more = !bound && !nextCursor.IsNULLOrEmpty() && nextCursor != m_lastCursor && added > 0;
        if ( !more )
            collected = m_collected;
    }

    if ( more )
    {
        if ( !QueueNextPage( nextCursor ) )
            FailRefresh( generation, ASSETSTORE_STATUS_FAILED, "the library query could not be queued" );
        return;
    }
    CompleteRefresh( generation, collected, truncated );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::FailRefresh( UInt64 generation, TAssetStoreStatus status, const CORE::CString& message )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        if ( generation != m_generation )
            return;
        m_loading = false;
        m_lastStatus = status;
        m_lastMessage = message;
        m_collected.clear();
        m_collectedIds.clear();
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreLibraryFileSystemAccess: The refresh of the library of the store \"" << GetProviderId() << "\" failed: " << message );
    CORE::TCloneableString eventData( message );
    NotifyObserversFromThread( RefreshFailedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::CompleteRefresh( UInt64 generation, const TAssetStoreItemVector& items, bool truncated )
{GUCEF_TRACE;

    // the new cache is made outside the lock and replaces the old one in one step
    SCache cache;
    BuildCache( items, cache );
    {
        MT::CScopeMutex lock( m_lock );
        if ( generation != m_generation )
            return;
        m_cache.Swap( cache );
        m_truncated = truncated;
        m_loading = false;
        m_lastStatus = ASSETSTORE_STATUS_OK;
        m_lastMessage.Clear();
        m_collected.clear();
        m_collectedIds.clear();
    }
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "AssetStoreLibraryFileSystemAccess: The library of the store \"" << GetProviderId() << "\" has " << items.size() << " items" );
    NotifyObserversFromThread( LibraryChangedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryFileSystemAccess::IsLoading( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_loading;
}

/*-------------------------------------------------------------------------*/

UInt32
CAssetStoreLibraryFileSystemAccess::GetItemCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_cache.items.size();
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryFileSystemAccess::WasTruncated( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_truncated;
}

/*-------------------------------------------------------------------------*/

TAssetStoreStatus
CAssetStoreLibraryFileSystemAccess::GetLastStatus( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastStatus;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetLastMessage( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_lastMessage;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::MakeListingFolderName( const CORE::CString& listingType )
{GUCEF_TRACE;

    if ( listingType.Trim( true ).Trim( false ).IsNULLOrEmpty() )
        return OtherListingTypeName;
    return CAssetStoreSafeNames::MakeSafePart( listingType );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::MakeSellerFolderName( const CORE::CString& seller )
{GUCEF_TRACE;

    if ( seller.Trim( true ).Trim( false ).IsNULLOrEmpty() )
        return UnknownSellerName;
    return CAssetStoreSafeNames::MakeSafePart( seller );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::MakeItemFileName( const CAssetStoreItem& item )
{GUCEF_TRACE;

    CORE::CString title = item.title.Trim( true ).Trim( false ).IsNULLOrEmpty() ? CORE::CString( "untitled" ) : CAssetStoreSafeNames::MakeSafePart( item.title );
    return title + " [" + MakeNameSafe( item.itemId ) + "]";
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::BuildCache( const TAssetStoreItemVector& items, SCache& cache )
{GUCEF_TRACE;

    cache.Clear();
    cache.AddEntry( CORE::CString::Empty, "/", true, 0, -1 );
    for ( size_t i=0; i<items.size(); ++i )
    {
        const CAssetStoreItem& item = items[ i ];
        if ( item.itemId.IsNULLOrEmpty() || cache.entryByItemId.end() != cache.entryByItemId.find( item.itemId ) )
            continue;

        const CORE::CString listingName = MakeListingFolderName( item.listingType );
        const CORE::CString sellerName = MakeSellerFolderName( item.seller );
        const CORE::CString listingPath = JoinPath( "/", listingName );
        const CORE::CString sellerPath = JoinPath( listingPath, sellerName );
        cache.EnsureFolder( listingName, listingPath, "/" );
        cache.EnsureFolder( sellerName, sellerPath, listingPath );

        const CORE::CString fileName = MakeItemFileName( item );
        const CORE::CString filePath = JoinPath( sellerPath, fileName );
        if ( cache.entryByPath.end() != cache.entryByPath.find( filePath ) )
            continue;

        cache.items.push_back( item );
        const UInt64 size = item.formats.empty() ? 0 : item.formats[ 0 ].GetTotalSize();
        const UInt32 index = cache.AddEntry( fileName, filePath, false, size, (Int32) ( cache.items.size() - 1 ) );
        cache.children[ sellerPath ][ fileName ] = index;
        cache.entryByItemId[ item.itemId ] = index;
    }
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::Conform( const CORE::CString& path )
{GUCEF_TRACE;

    CORE::CString conformed = path.ReplaceChar( '\\', '/' );
    while ( conformed.Length() > 1 && '/' == conformed[ conformed.Length()-1 ] )
        conformed = conformed.CutChars( 1, false );
    if ( conformed.IsNULLOrEmpty() || '/' != conformed[ 0 ] )
        conformed = "/" + conformed;
    return conformed;
}

/*-------------------------------------------------------------------------*/

const CAssetStoreLibraryFileSystemAccess::SEntry*
CAssetStoreLibraryFileSystemAccess::FindEntryLocked( const CORE::CString& conformedPath ) const
{GUCEF_TRACE;

    TIndexMap::const_iterator i = m_cache.entryByPath.find( conformedPath );
    return i == m_cache.entryByPath.end() ? GUCEF_NULL : &m_cache.entries[ (*i).second ];
}

/*-------------------------------------------------------------------------*/

void
CAssetStoreLibraryFileSystemAccess::FillMetaData( const SEntry& entry, const CORE::CString& rootName, CORE::CResourceMetaData& metaData )
{GUCEF_TRACE;

    metaData.resourceExists = true;
    metaData.name = "/" == entry.path ? rootName : entry.name;
    metaData.hasName = true;
    metaData.hasIsReadOnly = true;
    metaData.isReadOnly = true;
    if ( !entry.isDirectory )
    {
        metaData.resourceSizeInBytes = entry.size;
        metaData.hasResourceSizeInBytes = true;
    }
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryFileSystemAccess::TryGetItem( const CORE::CString& path, CAssetStoreItem& item ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( path );
    MT::CScopeMutex lock( m_lock );
    const SEntry* entry = FindEntryLocked( conformed );
    if ( GUCEF_NULL == entry || entry->isDirectory || entry->itemIndex < 0 )
        return false;
    item = m_cache.items[ (size_t) entry->itemIndex ];
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetItemIdOfPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( path );
    MT::CScopeMutex lock( m_lock );
    const SEntry* entry = FindEntryLocked( conformed );
    if ( GUCEF_NULL == entry || entry->isDirectory || entry->itemIndex < 0 )
        return CORE::CString::Empty;
    return m_cache.items[ (size_t) entry->itemIndex ].itemId;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetPathOfItem( const CORE::CString& itemId ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TIndexMap::const_iterator i = m_cache.entryByItemId.find( itemId );
    return i == m_cache.entryByItemId.end() ? CORE::CString::Empty : m_cache.entries[ (*i).second ].path;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetThumbnailUrlOfPath( const CORE::CString& path ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( path );
    MT::CScopeMutex lock( m_lock );
    const SEntry* entry = FindEntryLocked( conformed );
    if ( GUCEF_NULL == entry || entry->isDirectory || entry->itemIndex < 0 )
        return CORE::CString::Empty;
    const CAssetStoreItem& item = m_cache.items[ (size_t) entry->itemIndex ];
    if ( !item.thumbnailUrl.IsNULLOrEmpty() )
        return item.thumbnailUrl;
    return item.imageUrls.empty() ? CORE::CString::Empty : item.imageUrls[ 0 ];
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetDisplayName( void ) const
{GUCEF_TRACE;

    return m_provider.IsNULL() ? CORE::CString( "Library" ) : m_provider->GetDisplayName();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CAssetStoreLibraryFileSystemAccess::GetRootPath( void ) const
{GUCEF_TRACE;

    return "/";
}

/*-------------------------------------------------------------------------*/

bool
CAssetStoreLibraryFileSystemAccess::IsWriteable( const CORE::CString& path ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryCreateDirectory( const CORE::CString& path, bool createParents )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryDeleteDirectory( const CORE::CString& path, bool deleteContents )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryMoveFiles( const CORE::CString::StringVector& filePaths     ,
                                                  const CORE::CString& destinationDir             ,
                                                  bool overwrite                                  ,
                                                  TFileSystemAccessResultVector* perFileResults   )
{GUCEF_TRACE;

    if ( GUCEF_NULL != perFileResults )
        perFileResults->assign( filePaths.size(), CORE::FILESYSTEMACCESS_READ_ONLY );
    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryRenameFile( const CORE::CString& path, const CORE::CString& newName )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryDeleteFile( const CORE::CString& path )
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_READ_ONLY;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( path );
    MT::CScopeMutex lock( m_lock );
    const SEntry* entry = FindEntryLocked( conformed );
    if ( GUCEF_NULL == entry )
        entryType = CORE::FILESYSTEMENTRYTYPE_NONE;
    else
        entryType = entry->isDirectory ? CORE::FILESYSTEMENTRYTYPE_DIRECTORY : CORE::FILESYSTEMENTRYTYPE_FILE;
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( path );
    const CORE::CString rootName = GetDisplayName();
    MT::CScopeMutex lock( m_lock );
    const SEntry* entry = FindEntryLocked( conformed );
    if ( GUCEF_NULL == entry )
        return CORE::FILESYSTEMACCESS_NOT_FOUND;
    FillMetaData( *entry, rootName, metaData );
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const
{GUCEF_TRACE;

    const CORE::CString conformed = Conform( dirPath );
    const CORE::CString rootName = GetDisplayName();
    CORE::CFileSystemEntryListIterator::TEntryVector listing;
    {
        MT::CScopeMutex lock( m_lock );
        const SEntry* dir = FindEntryLocked( conformed );
        if ( GUCEF_NULL == dir )
            return CORE::FILESYSTEMACCESS_NOT_FOUND;
        if ( !dir->isDirectory )
            return CORE::FILESYSTEMACCESS_NOT_A_DIRECTORY;

        TChildMap::const_iterator children = m_cache.children.find( conformed );
        if ( children != m_cache.children.end() )
        {
            TIndexMap::const_iterator c = (*children).second.begin();
            while ( c != (*children).second.end() )
            {
                const SEntry& entry = m_cache.entries[ (*c).second ];
                CORE::CFileSystemEntryListIterator::CEntry item;
                item.name = entry.name;
                item.path = entry.path;
                item.isDirectory = entry.isDirectory;
                item.isFile = !entry.isDirectory;
                item.hasMetaData = true;
                FillMetaData( entry, rootName, item.metaData );
                listing.push_back( item );
                ++c;
            }
        }
    }
    iterator = CORE::TFileSystemIteratorPtr( GUCEF_NEW CORE::CFileSystemEntryListIterator( conformed, listing ) );
    return CORE::FILESYSTEMACCESS_SUCCESS;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const
{GUCEF_TRACE;

    specialDirs.clear();
    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------*/

CORE::TFileSystemAccessResult
CAssetStoreLibraryFileSystemAccess::TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const
{GUCEF_TRACE;

    return CORE::FILESYSTEMACCESS_NOT_SUPPORTED;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAssetStoreLibraryFileSystemAccess::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::ASSETSTORE::CAssetStoreLibraryFileSystemAccess";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
