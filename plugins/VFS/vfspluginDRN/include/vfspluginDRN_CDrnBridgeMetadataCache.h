/*
 *  vfspluginDRN: GUCEF VFS plugin presenting a share of another DRN node as a VFS archive
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

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CSTRING_H
#include "gucefCORE_CString.h"
#define GUCEF_CORE_CSTRING_H
#endif /* GUCEF_CORE_CSTRING_H ? */

#ifndef GUCEF_CORE_CTSHAREDPTR_H
#include "CTSharedPtr.h"
#define GUCEF_CORE_CTSHAREDPTR_H
#endif /* GUCEF_CORE_CTSHAREDPTR_H ? */

#ifndef GUCEF_DRN_CDRNNAMESPACEENTRY_H
#include "gucefDRN_CDRNNamespaceEntry.h"
#define GUCEF_DRN_CDRNNAMESPACEENTRY_H
#endif /* GUCEF_DRN_CDRNNAMESPACEENTRY_H ? */

#ifndef GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#include "vfspluginDRN_CDrnBridgeSettings.h"
#define GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H
#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGESETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFSPLUGIN {
namespace DRNBRIDGE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What the archive has learned about the share and still takes as current: the listings of directories and the
 *  descriptions of single paths, each for a time (CDrnBridgeSettings::metadataTtlMs), "there is no such entry" for a
 *  shorter one (negativeTtlMs), and no more entries than maxCachedEntries in all (the least recently used go first).
 *  A time of 0 turns the cache off: nothing is remembered.
 *
 *  Two kinds of records, by path relative to the share (no leading or trailing separator, '/' inside, "" is the root):
 *    - a listing: the entries of a directory, sorted by name, with the revision of the directory. It tells which names
 *      the directory holds. When it is old, the archive asks the serving side whether the directory still has that
 *      revision (one small call that answers "unchanged") and, when it does, renews the listing (TouchListing()).
 *      The directory's revision moves when entries are added or removed; it does not move when a file in it changes, so
 *      the attributes of the listed entries age on their own: an entry taken from an old listing is "stale", to be
 *      checked with a stat that carries the revision the entry has (the answer is "unchanged" when it still is).
 *    - an entry: what a stat said about one path, or that the path is not there.
 *  A look-up looks at the entry records first and then at the listing of the parent directory. An absence is never
 *  concluded from a listing: the serving side may leave out names a stat still finds, so a name that is not listed is
 *  asked for.
 *
 *  A listing that is handed out is a snapshot (a shared, never changed vector): the holder reads it without the cache's
 *  lock and a later update replaces the record, not the snapshot. All calls are thread safe.
 */
class GUCEF_VFSPLUGIN_DRN_EXPORT_CPP CDrnBridgeMetadataCache
{
    public:

    typedef GUCEF::vector< DRN::CDRNNamespaceEntry > TEntryVector;
    typedef CORE::CTSharedPtr< TEntryVector, MT::CMutex > TEntryVectorPtr;

    enum TState
    {
        STATE_MISS      ,   /**< nothing is known (or the cache is off) */
        STATE_FRESH     ,   /**< known and still taken as current */
        STATE_STALE         /**< known, but older than the time: to be checked (an offline mount may still serve it) */
    };

    explicit CDrnBridgeMetadataCache( const CDrnBridgeSettings& settings );

    ~CDrnBridgeMetadataCache();

    bool IsEnabled( void ) const;

    /** Milliseconds on a clock that only goes forward, the cache's clock */
    static UInt64 NowMs( void );

    /**
     *  What is known about a path. 'exists' is false for a remembered absence (the entry is then empty). For a stale answer the entry
     *  is what was known, so that its revision can be sent along with the check.
     */
    TState FindEntry( const CORE::CString& path, DRN::CDRNNamespaceEntry& entry, bool& exists ) const;

    /** Remembers what a stat said about a path; the entry's name is set to the path */
    void PutEntry( const CORE::CString& path, const DRN::CDRNNamespaceEntry& entry );

    /** Remembers that there is nothing at the path (for the negative time) */
    void PutAbsent( const CORE::CString& path );

    /** A check found the entry unchanged: it counts as current again */
    void TouchEntry( const CORE::CString& path );

    /** The listing of a directory; for a stale one the entries and revision are what to check with ifRevisionNot */
    TState FindListing( const CORE::CString& dir, TEntryVectorPtr& entries, CORE::CString& dirRevision ) const;

    /**
     *  Remembers a listing: sorts it by name (the names stay the names within the directory) and puts it
     *  in the cache when it fits. Gives the snapshot that was made, also when the cache is off or the listing is too large to keep.
     */
    TEntryVectorPtr PutListing( const CORE::CString& dir, const TEntryVector& entries, const CORE::CString& dirRevision );

    /** The directory is still at the revision the listing has: its names count as current again */
    void TouchListing( const CORE::CString& dir );

    /** Whether what the cache holds of the entries of a directory (sizes, revisions, digests) counts as current: false when the listing was renewed by a check of the directory alone and the entries were not asked again */
    bool AreAttributesCurrent( const CORE::CString& dir ) const;

    /** A path changed or is not what the cache says: its record goes and the listing of its directory no longer vouches for its attributes */
    void InvalidatePath( const CORE::CString& path );

    /** The names of a directory are not what the listing says: the listing goes */
    void InvalidateDirectory( const CORE::CString& dir );

    void InvalidateAll( void );

    /** Records held (entries in listings plus entry records) */
    UInt32 GetEntryCount( void ) const;

    static CORE::CString ParentOf( const CORE::CString& path );
    static CORE::CString LeafOf( const CORE::CString& path );

    private:

    CDrnBridgeMetadataCache( const CDrnBridgeMetadataCache& src );                  /**< not implemented */
    CDrnBridgeMetadataCache& operator=( const CDrnBridgeMetadataCache& src );       /**< not implemented */

    class CEntryRecord
    {
        public:
        DRN::CDRNNamespaceEntry entry;
        bool exists;
        UInt64 stamp;
        UInt64 lastUsed;
        CEntryRecord( void ) : entry(), exists( false ), stamp( 0 ), lastUsed( 0 ) {}
    };

    class CListingRecord
    {
        public:
        TEntryVectorPtr entries;
        CORE::CString dirRevision;
        UInt64 stamp;               /**< the names are current as of this moment */
        UInt64 attributeStamp;      /**< the attributes of the entries are current as of this moment */
        bool attributesStale;       /**< something changed in the directory that its listing cannot vouch for */
        UInt64 lastUsed;
        CListingRecord( void ) : entries(), dirRevision(), stamp( 0 ), attributeStamp( 0 ), attributesStale( false ), lastUsed( 0 ) {}
    };

    typedef GUCEF::map< CORE::CString, CEntryRecord > TEntryMap;
    typedef GUCEF::map< CORE::CString, CListingRecord > TListingMap;

    bool IsFresh( UInt64 stamp, UInt64 now, bool absence ) const;
    void TrimIfNeeded( void );
    void RemoveListing( TListingMap::iterator i );

    CDrnBridgeSettings m_settings;
    mutable MT::CMutex m_lock;
    mutable TEntryMap m_entries;
    mutable TListingMap m_listings;
    UInt32 m_entryCount;
};

typedef CORE::CTSharedPtr< CDrnBridgeMetadataCache, MT::CMutex > CDrnBridgeMetadataCachePtr;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFSPLUGIN_DRN_CDRNBRIDGEMETADATACACHE_H ? */
