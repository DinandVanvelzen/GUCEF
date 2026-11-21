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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_VFS_CVFSREMOTEPATH_H
#include "gucefVFS_CVfsRemotePath.h"
#define GUCEF_VFS_CVFSREMOTEPATH_H
#endif /* GUCEF_VFS_CVFSREMOTEPATH_H ? */

#include "gucefVFS_CVfsRemoteMetadataCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace VFS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CVfsRemoteMetadataCache::CVfsRemoteMetadataCache( UInt32 timeToLiveMs, UInt32 maxEntries )
    : m_lock()
    , m_timeToLiveMs( timeToLiveMs )
    , m_maxEntries( maxEntries )
    , m_records()
    , m_listings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CVfsRemoteMetadataCache::~CVfsRemoteMetadataCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteMetadataCache::Configure( UInt32 timeToLiveMs, UInt32 maxEntries )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_timeToLiveMs = timeToLiveMs;
    m_maxEntries = maxEntries;
    EnforceBoundLocked();
}

/*-------------------------------------------------------------------------*/

UInt64
CVfsRemoteMetadataCache::NowMs( void )
{GUCEF_TRACE;

    return MT::PrecisionTickCount() * 1000 / MT::PrecisionTimerResolution();
}

/*-------------------------------------------------------------------------*/

CVfsRemoteMetadataCache::EState
CVfsRemoteMetadataCache::StateOfAge( UInt64 fetchedMs ) const
{GUCEF_TRACE;

    if ( 0 == m_timeToLiveMs )
        return STATE_STALE;
    return ( NowMs() - fetchedMs ) <= m_timeToLiveMs ? STATE_FRESH : STATE_STALE;
}

/*-------------------------------------------------------------------------*/

CVfsRemoteMetadataCache::EState
CVfsRemoteMetadataCache::FindEntry( const CORE::CString& path, CVfsRemoteEntry& entry ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    bool haveRecord = false;
    UInt64 recordMs = 0;
    TRecordMap::const_iterator r = m_records.find( path );
    if ( r != m_records.end() )
    {
        haveRecord = true;
        recordMs = r->second.fetchedMs;
    }

    bool haveListed = false;
    UInt64 listedMs = 0;
    const CVfsRemoteEntry* listedEntry = GUCEF_NULL;
    if ( !path.IsNULLOrEmpty() )
    {
        TListingMap::const_iterator l = m_listings.find( CVfsRemotePath::ParentOf( path ) );
        if ( l != m_listings.end() )
        {
            const CORE::CString leaf = CVfsRemotePath::LeafOf( path );
            const CVfsRemoteEntry::TVector& entries = l->second.entries;
            for ( size_t i=0; i<entries.size(); ++i )
            {
                if ( entries[ i ].name == leaf )
                {
                    haveListed = true;
                    listedMs = l->second.fetchedMs;
                    listedEntry = &entries[ i ];
                    break;
                }
            }
            if ( !haveListed && l->second.dirExists && !haveRecord )
            {
                // the listing of the parent is complete: a name that is not in it is not there
                entry = CVfsRemoteEntry();
                entry.name = leaf;
                entry.exists = false;
                return StateOfAge( l->second.fetchedMs );
            }
        }
    }

    if ( haveRecord && ( !haveListed || recordMs >= listedMs ) )
    {
        entry = r->second.entry;
        return StateOfAge( recordMs );
    }
    if ( haveListed )
    {
        entry = *listedEntry;

        // a listing that was renewed without its entries being asked again vouches for the names and not for what is known of them
        if ( !listedEntry->attributesKnown )
            return STATE_STALE;
        return StateOfAge( listedMs );
    }

    // a directory that has a listing is there
    TListingMap::const_iterator own = m_listings.find( path );
    if ( own != m_listings.end() )
    {
        entry = CVfsRemoteEntry();
        entry.name = CVfsRemotePath::LeafOf( path );
        entry.exists = own->second.dirExists;
        entry.isDirectory = own->second.dirExists;
        return StateOfAge( own->second.fetchedMs );
    }

    return STATE_UNKNOWN;
}

/*-------------------------------------------------------------------------*/

CVfsRemoteMetadataCache::EState
CVfsRemoteMetadataCache::FindListing( const CORE::CString& dirPath, CVfsRemoteEntry::TVector& entries, bool& dirExists ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TListingMap::const_iterator l = m_listings.find( dirPath );
    if ( l == m_listings.end() )
        return STATE_UNKNOWN;

    entries = l->second.entries;
    dirExists = l->second.dirExists;
    return StateOfAge( l->second.fetchedMs );
}

/*-------------------------------------------------------------------------*/

CVfsRemoteMetadataCache::EState
CVfsRemoteMetadataCache::GetState( const CORE::CString& path ) const
{GUCEF_TRACE;

    CVfsRemoteEntry entry;
    return FindEntry( path, entry );
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteMetadataCache::StoreEntry( const CORE::CString& path, const CVfsRemoteEntry& entry, CVfsRemoteEntry* previous, bool* hadPrevious )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    bool changed = true;
    bool had = false;
    TRecordMap::iterator r = m_records.find( path );
    if ( r != m_records.end() )
    {
        had = true;
        if ( GUCEF_NULL != previous )
            *previous = r->second.entry;
        changed = !r->second.entry.IsSameAs( entry );
    }
    else
    {
        // what the listing of the parent says of the path is the previous word
        CVfsRemoteEntry listed;
        if ( !path.IsNULLOrEmpty() )
        {
            TListingMap::const_iterator l = m_listings.find( CVfsRemotePath::ParentOf( path ) );
            if ( l != m_listings.end() )
            {
                const CORE::CString leaf = CVfsRemotePath::LeafOf( path );
                for ( size_t i=0; i<l->second.entries.size(); ++i )
                {
                    if ( l->second.entries[ i ].name == leaf )
                    {
                        had = true;
                        listed = l->second.entries[ i ];
                        break;
                    }
                }
            }
        }
        if ( had )
        {
            if ( GUCEF_NULL != previous )
                *previous = listed;
            CVfsRemoteEntry compare( entry );
            compare.name = listed.name;
            changed = !listed.IsSameAs( compare );
        }
    }
    if ( GUCEF_NULL != hadPrevious )
        *hadPrevious = had;

    SRecord& record = m_records[ path ];
    record.entry = entry;
    record.entry.name = CVfsRemotePath::LeafOf( path );
    record.fetchedMs = NowMs();
    EnforceBoundLocked();
    return changed;
}

/*-------------------------------------------------------------------------*/

bool
CVfsRemoteMetadataCache::StoreListing( const CORE::CString& dirPath, const CVfsRemoteEntry::TVector& entries, bool dirExists )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    bool changed = true;
    TListingMap::iterator l = m_listings.find( dirPath );
    if ( l != m_listings.end() && l->second.dirExists == dirExists && l->second.entries.size() == entries.size() )
    {
        changed = false;
        for ( size_t i=0; i<entries.size(); ++i )
        {
            if ( !entries[ i ].IsSameAs( l->second.entries[ i ] ) )
            {
                changed = true;
                break;
            }
        }
    }

    // the records of the entries in it go: the listing is the newer word on them
    const CORE::CString prefix = dirPath.IsNULLOrEmpty() ? CORE::CString::Empty : dirPath + '/';
    for ( size_t i=0; i<entries.size(); ++i )
        m_records.erase( prefix + entries[ i ].name );

    SListing& listing = m_listings[ dirPath ];
    listing.entries = entries;
    listing.dirExists = dirExists;
    listing.fetchedMs = NowMs();
    EnforceBoundLocked();
    return changed;
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteMetadataCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_records.clear();
    m_listings.clear();
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteMetadataCache::Invalidate( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_records.erase( path );
    m_listings.erase( path );
    if ( !path.IsNULLOrEmpty() )
        m_listings.erase( CVfsRemotePath::ParentOf( path ) );
}

/*-------------------------------------------------------------------------*/

UInt32
CVfsRemoteMetadataCache::GetEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) ( m_records.size() + m_listings.size() );
}

/*-------------------------------------------------------------------------*/

void
CVfsRemoteMetadataCache::EnforceBoundLocked( void )
{GUCEF_TRACE;

    // the lock is held by the caller: the oldest go
    while ( m_maxEntries > 0 && m_records.size() + m_listings.size() > m_maxEntries )
    {
        bool oldestIsRecord = true;
        UInt64 oldestMs = (UInt64) -1;
        TRecordMap::iterator oldestRecord = m_records.end();
        TListingMap::iterator oldestListing = m_listings.end();
        for ( TRecordMap::iterator r = m_records.begin(); r != m_records.end(); ++r )
        {
            if ( r->second.fetchedMs <= oldestMs )
            {
                oldestMs = r->second.fetchedMs;
                oldestRecord = r;
            }
        }
        for ( TListingMap::iterator l = m_listings.begin(); l != m_listings.end(); ++l )
        {
            if ( l->second.fetchedMs < oldestMs )
            {
                oldestMs = l->second.fetchedMs;
                oldestListing = l;
                oldestIsRecord = false;
            }
        }
        if ( oldestIsRecord && oldestRecord != m_records.end() )
            m_records.erase( oldestRecord );
        else
        if ( oldestListing != m_listings.end() )
            m_listings.erase( oldestListing );
        else
            break;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace VFS */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
