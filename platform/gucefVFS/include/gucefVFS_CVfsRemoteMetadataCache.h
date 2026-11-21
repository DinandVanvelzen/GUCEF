/*
 *  gucefVFS: GUCEF module implementing a Virtual File System
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

#ifndef GUCEF_VFS_CVFSREMOTEMETADATACACHE_H
#define GUCEF_VFS_CVFSREMOTEMETADATACACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEENTRY_H
#include "gucefVFS_CVfsRemoteEntry.h"
#define GUCEF_VFS_CVFSREMOTEENTRY_H
#endif /* GUCEF_VFS_CVFSREMOTEENTRY_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  What a remote archive knows in memory of the paths of a remote place: the metadata of paths (also the knowledge that
 *  nothing is at a path) and the listings of directories, each with the time it was fetched. What is not older than the time
 *  to live is fresh, what is older is stale: a stale answer may still be given (to a thread that must not wait, or while the
 *  link is down) while it is checked again.
 *
 *  The metadata of a path comes from its own record or from the listing of its parent, whichever was fetched last: a
 *  listing of a directory answers the stat of every file in it. The number of records and listings is bounded: past the bound
 *  the oldest go. Thread safe.
 */
class GUCEF_VFS_PUBLIC_CPP CVfsRemoteMetadataCache
{
    public:

    enum EState
    {
        STATE_UNKNOWN = 0,
        STATE_FRESH,
        STATE_STALE
    };

    CVfsRemoteMetadataCache( UInt32 timeToLiveMs, UInt32 maxEntries );

    ~CVfsRemoteMetadataCache();

    void Configure( UInt32 timeToLiveMs, UInt32 maxEntries );

    /** The metadata of a path (a file or a directory) from its record or the listing of its parent; the state says how old it is */
    EState FindEntry( const CORE::CString& path, CVfsRemoteEntry& entry ) const;

    /** The listing of a directory: the entries and whether the directory exists */
    EState FindListing( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists ) const;

    /** What is known of a path as a file or a directory: its record, its listing or the listing of its parent */
    EState GetState( const CORE::CString& path ) const;

    /**
     *  Keeps the record of a path. True when it is new or differs from what was held (an unchanged revalidation says false);
     *  'previous' gets the record that was held when there was one.
     */
    bool StoreEntry( const CORE::CString& path, const CVfsRemoteEntry& entry, CVfsRemoteEntry* previous = GUCEF_NULL, bool* hadPrevious = GUCEF_NULL );

    /** Keeps the listing of a directory; true when it is new or differs from the one held. The records of the entries in it go: the listing is the newer word */
    bool StoreListing( const CORE::CString& dirPath, const CVfsRemoteEntry::TVector& entries, bool dirExists );

    /** Forgets everything */
    void Clear( void );

    /** Forgets the record of a path, its listing and the listing of its parent */
    void Invalidate( const CORE::CString& path );

    UInt32 GetEntryCount( void ) const;

    private:

    CVfsRemoteMetadataCache( const CVfsRemoteMetadataCache& src );                  /**< not implemented */
    CVfsRemoteMetadataCache& operator=( const CVfsRemoteMetadataCache& src );       /**< not implemented */

    struct SRecord
    {
        CVfsRemoteEntry entry;
        UInt64 fetchedMs;
        SRecord( void ) : entry(), fetchedMs( 0 ) {}
    };

    struct SListing
    {
        CVfsRemoteEntry::TVector entries;
        bool dirExists;
        UInt64 fetchedMs;
        SListing( void ) : entries(), dirExists( false ), fetchedMs( 0 ) {}
    };

    typedef GUCEF::map< CORE::CString, SRecord > TRecordMap;
    typedef GUCEF::map< CORE::CString, SListing > TListingMap;

    static UInt64 NowMs( void );
    EState StateOfAge( UInt64 fetchedMs ) const;
    void EnforceBoundLocked( void );

    private:

    mutable MT::CMutex m_lock;
    UInt32 m_timeToLiveMs;
    UInt32 m_maxEntries;
    TRecordMap m_records;
    TListingMap m_listings;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_VFS_CVFSREMOTEMETADATACACHE_H ? */
