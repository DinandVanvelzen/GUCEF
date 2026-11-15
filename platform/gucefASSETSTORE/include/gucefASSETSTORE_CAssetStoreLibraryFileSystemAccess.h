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

#ifndef GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H
#define GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */
#ifndef GUCEF_CORE_CTSGNOTIFIER_H
#include "CTSGNotifier.h"
#define GUCEF_CORE_CTSGNOTIFIER_H
#endif /* GUCEF_CORE_CTSGNOTIFIER_H ? */
#ifndef GUCEF_CORE_CTIMER_H
#include "CTimer.h"
#define GUCEF_CORE_CTIMER_H
#endif /* GUCEF_CORE_CTIMER_H ? */
#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */
#ifndef GUCEF_CORE_CFUTURERESULT_H
#include "gucefCORE_CFutureResult.h"
#define GUCEF_CORE_CFUTURERESULT_H
#endif /* GUCEF_CORE_CFUTURERESULT_H ? */
#ifndef GUCEF_CORE_IFILESYSTEMACCESS_H
#include "gucefCORE_IFileSystemAccess.h"
#define GUCEF_CORE_IFILESYSTEMACCESS_H
#endif /* GUCEF_CORE_IFILESYSTEMACCESS_H ? */
#ifndef GUCEF_ASSETSTORE_ETYPES_H
#include "gucefASSETSTORE_ETypes.h"
#define GUCEF_ASSETSTORE_ETYPES_H
#endif /* GUCEF_ASSETSTORE_ETYPES_H ? */
#ifndef GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#include "gucefASSETSTORE_CAssetStoreItem.h"
#define GUCEF_ASSETSTORE_CASSETSTOREITEM_H
#endif /* GUCEF_ASSETSTORE_CASSETSTOREITEM_H ? */
#ifndef GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#include "gucefASSETSTORE_CIAssetStoreProvider.h"
#define GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H
#endif /* GUCEF_ASSETSTORE_CIASSETSTOREPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace ASSETSTORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The library of one store provider as a file system that nothing can be written to (section 10 of the design), so that
 *  a content browser shows it as it shows folders. The root "/" lists the listing types as folders ("other" for items
 *  without one), a listing type folder lists the sellers as folders ("unknown seller" for items without one), and a
 *  seller folder lists the items as files named "<safe title> [<item id>]". The id between the brackets is the id of the
 *  item, so a name gives the item back; the size of a file is the total size of the first format of the item when it
 *  is known, 0 otherwise. Paths use '/' as the separator and start with one; a trailing separator is accepted.
 *
 *  The provider is given as a shared pointer, which keeps it alive for the library queries that are under way (the
 *  application gets it from CAssetStoreRegistry::TryGetProvider()).
 *
 *  Listing never waits for the network: every question is answered from a cache of the items. Refresh() fills the
 *  cache: it queries the pages of the library as async operations of the provider (CIAssetStoreProvider::QueryLibraryAsync(),
 *  following the next cursor, at most GetMaxItems() items), and when all pages are in, the new cache replaces the old
 *  one in one step. A refresh that fails (not signed in, store unreachable) keeps the cache it had and leaves its
 *  status in GetLastStatus() and GetLastMessage(). Starting another refresh while one runs replaces it: the results
 *  of the earlier one are dropped.
 *
 *  Events are raised on the thread of the pulse generator the object was made with (the global one by default):
 *  LibraryChangedEvent (no data) once for every refresh that completed, RefreshFailedEvent (TCloneableString: the message)
 *  for every refresh that failed. The pages are picked up from the pulse generator thread, so it has to be pumped for a
 *  refresh to complete. The listing functions and the lookups can be called from any thread, also while a refresh
 *  completes. A content browser form takes the new listing after its own Refresh().
 *
 *  Made with GUCEF_NEW and used through CORE::TFileSystemAccessPtr ( ( GUCEF_NEW CAssetStoreLibraryFileSystemAccess( provider ) )->CreateSharedPtr() ).
 */
class GUCEF_ASSETSTORE_PUBLIC_CPP CAssetStoreLibraryFileSystemAccess : public CORE::IFileSystemAccess ,
                                                                       public CORE::CTSGNotifier
{
    public:

    static const CORE::CEvent LibraryChangedEvent;          /**< no data: a refresh completed and the cache was replaced */
    static const CORE::CEvent RefreshFailedEvent;           /**< TCloneableString: a refresh failed; the message */

    static const UInt32 DefaultMaxItems = 5000;
    static const UInt32 DefaultPageSize = 50;

    static const CORE::CString OtherListingTypeName;        /**< "other": the folder of the items that have no listing type */
    static const CORE::CString UnknownSellerName;           /**< "unknown seller": the folder of the items that have no seller */

    static void RegisterEvents( void );

    /** Events are raised on the thread of the global pulse generator */
    explicit CAssetStoreLibraryFileSystemAccess( const TAssetStoreProviderPtr& provider );

    /** Events are raised on the thread of the given pulse generator */
    CAssetStoreLibraryFileSystemAccess( const TAssetStoreProviderPtr& provider, CORE::PulseGeneratorPtr pulseGenerator );

    virtual ~CAssetStoreLibraryFileSystemAccess() GUCEF_VIRTUAL_OVERRIDE;

    /** The id of the store of the library */
    const CORE::CString& GetProviderId( void ) const;

    /** The most items a refresh collects; the rest of the library is left out (WasTruncated()). 0 gives the default */
    void SetMaxItems( UInt32 maxItems );
    UInt32 GetMaxItems( void ) const;

    /** The number of items asked of the store per page, for the next refresh. 0 gives the default */
    void SetPageSize( UInt32 pageSize );
    UInt32 GetPageSize( void ) const;

    /** Free text that the next refresh passes on to the query of the store; empty for all items */
    void SetQueryText( const CORE::CString& text );
    CORE::CString GetQueryText( void ) const;

    /** A listing type that the next refresh passes on to the query of the store, as the store names it; empty for all */
    void SetListingTypeFilter( const CORE::CString& listingType );
    CORE::CString GetListingTypeFilter( void ) const;

    /**
     *  Starts an asynchronous fetch of the library; the cache keeps what it has until the fetch is complete.
     *  False when the first page could not be queued (the failure is then reported as for any failed refresh)
     */
    bool Refresh( void );

    bool IsLoading( void ) const;

    /** The number of items in the cache */
    UInt32 GetItemCount( void ) const;

    /** Whether the last completed refresh left items out because of the bound of GetMaxItems() */
    bool WasTruncated( void ) const;

    /** The outcome of the last refresh that ended: OK, or why it failed; OK before the first refresh */
    TAssetStoreStatus GetLastStatus( void ) const;
    CORE::CString GetLastMessage( void ) const;

    /** The item of a file of the library, false for a folder or a path that is not there */
    bool TryGetItem( const CORE::CString& path, CAssetStoreItem& item ) const;

    /** The id of the item of a file; empty for a folder or a path that is not there */
    CORE::CString GetItemIdOfPath( const CORE::CString& path ) const;

    /** The path of the file of an item; empty when the cache has no such item */
    CORE::CString GetPathOfItem( const CORE::CString& itemId ) const;

    /** The thumbnail address of the item of a file (the first picture of the item when it has no thumbnail); empty when there is none */
    CORE::CString GetThumbnailUrlOfPath( const CORE::CString& path ) const;

    /** The name of the folder of a listing type, "other" for an empty one */
    static CORE::CString MakeListingFolderName( const CORE::CString& listingType );

    /** The name of the folder of a seller, "unknown seller" for an empty one */
    static CORE::CString MakeSellerFolderName( const CORE::CString& seller );

    /** The name of the file of an item: "<safe title> [<item id>]" */
    static CORE::CString MakeItemFileName( const CAssetStoreItem& item );

    virtual CORE::CString GetDisplayName( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::CString GetRootPath( void ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool IsWriteable( const CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryCreateDirectory( const CORE::CString& path, bool createParents ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryDeleteDirectory( const CORE::CString& path, bool deleteContents ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveDirectory( const CORE::CString& oldPath, const CORE::CString& newPath ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveFile( const CORE::CString& oldPath, const CORE::CString& newPath, bool overwrite ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryMoveFiles( const CORE::CString::StringVector& filePaths     ,
                                                        const CORE::CString& destinationDir             ,
                                                        bool overwrite                                  ,
                                                        TFileSystemAccessResultVector* perFileResults   ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryRenameFile( const CORE::CString& path, const CORE::CString& newName ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryDeleteFile( const CORE::CString& path ) GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetMetaData( const CORE::CString& path, CORE::CResourceMetaData& metaData ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetEntryType( const CORE::CString& path, CORE::TFileSystemEntryType& entryType ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryGetFileSystemIterator( const CORE::CString& dirPath, CORE::TFileSystemIteratorPtr& iterator ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult ListAvailableSpecialDirs( CORE::TSpecialDirSet& specialDirs ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual CORE::TFileSystemAccessResult TryResolveSpecialDir( CORE::TSpecialDirs specialDir, CORE::CString& path ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef CORE::CTEventHandlerFunctor< CAssetStoreLibraryFileSystemAccess > TEventCallback;

    /** A folder or a file of the cache */
    struct SEntry
    {
        CORE::CString name;
        CORE::CString path;
        bool isDirectory;
        UInt64 size;
        Int32 itemIndex;                    /**< the index in the items of the cache, -1 for a folder */
    };
    typedef GUCEF::vector< SEntry > TEntryVector;
    typedef GUCEF::map< CORE::CString, UInt32 > TIndexMap;
    typedef GUCEF::map< CORE::CString, TIndexMap > TChildMap;

    /** What the listing functions answer from: replaced as a whole when a refresh completes */
    struct SCache
    {
        TAssetStoreItemVector items;
        TEntryVector entries;               /**< the root is the first entry */
        TIndexMap entryByPath;
        TChildMap children;                 /**< by the path of the folder: the names of the entries in it, in order */
        TIndexMap entryByItemId;

        SCache( void );
        void Clear( void );
        void Swap( SCache& other );
        UInt32 AddEntry( const CORE::CString& name, const CORE::CString& path, bool isDirectory, UInt64 size, Int32 itemIndex );
        UInt32 EnsureFolder( const CORE::CString& name, const CORE::CString& path, const CORE::CString& parentPath );
    };

    void Initialize( void );
    void OnTimer( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData );
    bool QueueNextPage( const CORE::CString& cursor );
    void FailRefresh( UInt64 generation, TAssetStoreStatus status, const CORE::CString& message );
    void CompleteRefresh( UInt64 generation, const TAssetStoreItemVector& items, bool truncated );
    static void BuildCache( const TAssetStoreItemVector& items, SCache& cache );
    static CORE::CString Conform( const CORE::CString& path );
    const SEntry* FindEntryLocked( const CORE::CString& conformedPath ) const;
    static void FillMetaData( const SEntry& entry, const CORE::CString& rootName, CORE::CResourceMetaData& metaData );

    CAssetStoreLibraryFileSystemAccess( const CAssetStoreLibraryFileSystemAccess& src );              /**< not implemented */
    CAssetStoreLibraryFileSystemAccess& operator=( const CAssetStoreLibraryFileSystemAccess& src );   /**< not implemented */

    private:

    TAssetStoreProviderPtr m_provider;
    UInt32 m_maxItems;
    UInt32 m_pageSize;
    CORE::CString m_queryText;
    CORE::CString m_listingTypeFilter;
    SCache m_cache;
    bool m_truncated;
    bool m_loading;
    UInt64 m_generation;
    TAssetStoreStatus m_lastStatus;
    CORE::CString m_lastMessage;
    TAssetStoreItemVector m_collected;
    CORE::CString::StringSet m_collectedIds;
    CORE::CString m_lastCursor;
    CORE::CFutureResult m_pageFuture;
    bool m_pageQueued;
    CORE::CTimer m_timer;
    mutable MT::CMutex m_lock;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ASSETSTORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_ASSETSTORE_CASSETSTORELIBRARYFILESYSTEMACCESS_H ? */
