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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <algorithm>

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "vfspluginDRN_CDrnBridgeMetadataCache.h"

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
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** A record that may have to go when the cache is full */
class CTrimCandidate
{
    public:
    UInt64 lastUsed;
    bool isListing;
    CORE::CString key;
    UInt32 count;
    CTrimCandidate( void ) : lastUsed( 0 ), isListing( false ), key(), count( 0 ) {}
};

bool
OlderFirst( const CTrimCandidate& a, const CTrimCandidate& b )
{GUCEF_TRACE;

    return a.lastUsed < b.lastUsed;
}

bool
EntryNameLess( const DRN::CDRNNamespaceEntry& a, const DRN::CDRNNamespaceEntry& b )
{GUCEF_TRACE;

    return a.name < b.name;
}

}; /* anonymous namespace */

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDrnBridgeMetadataCache::CDrnBridgeMetadataCache( const CDrnBridgeSettings& settings )
    : m_settings( settings )
    , m_lock()
    , m_entries()
    , m_listings()
    , m_entryCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDrnBridgeMetadataCache::~CDrnBridgeMetadataCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMetadataCache::IsEnabled( void ) const
{GUCEF_TRACE;

    return m_settings.metadataTtlMs > 0 && m_settings.maxCachedEntries > 0;
}

/*-------------------------------------------------------------------------*/

UInt64
CDrnBridgeMetadataCache::NowMs( void )
{GUCEF_TRACE;

    return (UInt64) MT::PrecisionTimerTicksToMs( MT::PrecisionTickCount() );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeMetadataCache::ParentOf( const CORE::CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? CORE::CString::Empty : path.SubstrToIndex( (UInt32) slash, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CDrnBridgeMetadataCache::LeafOf( const CORE::CString& path )
{GUCEF_TRACE;

    const Int32 slash = path.HasChar( '/', false );
    return slash < 0 ? path : path.CutChars( (UInt32) slash + 1, true, 0 );
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMetadataCache::IsFresh( UInt64 stamp, UInt64 now, bool absence ) const
{GUCEF_TRACE;

    const UInt32 ttl = absence ? m_settings.negativeTtlMs : m_settings.metadataTtlMs;
    return now >= stamp ? ( now - stamp ) < ttl : true;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMetadataCache::TState
CDrnBridgeMetadataCache::FindEntry( const CORE::CString& path, DRN::CDRNNamespaceEntry& entry, bool& exists ) const
{GUCEF_TRACE;

    entry.Clear();
    exists = false;
    if ( !IsEnabled() )
        return STATE_MISS;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TEntryMap::iterator e = m_entries.find( path );
    if ( e != m_entries.end() )
    {
        CEntryRecord& record = (*e).second;
        record.lastUsed = now;
        exists = record.exists;
        entry = record.entry;
        return IsFresh( record.stamp, now, !record.exists ) ? STATE_FRESH : STATE_STALE;
    }

    if ( path.IsNULLOrEmpty() )
        return STATE_MISS;

    TListingMap::iterator l = m_listings.find( ParentOf( path ) );
    if ( l == m_listings.end() )
        return STATE_MISS;

    CListingRecord& listing = (*l).second;
    const TEntryVector& entries = *listing.entries;
    DRN::CDRNNamespaceEntry probe;
    probe.name = LeafOf( path );
    TEntryVector::const_iterator found = std::lower_bound( entries.begin(), entries.end(), probe, EntryNameLess );
    if ( found == entries.end() || (*found).name != probe.name )
        return STATE_MISS;

    listing.lastUsed = now;
    exists = true;
    entry = (*found);
    entry.name = path;
    return !listing.attributesStale && IsFresh( listing.attributeStamp, now, false ) ? STATE_FRESH : STATE_STALE;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::PutEntry( const CORE::CString& path, const DRN::CDRNNamespaceEntry& entry )
{GUCEF_TRACE;

    if ( !IsEnabled() )
        return;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TEntryMap::iterator e = m_entries.find( path );
    if ( e == m_entries.end() )
    {
        e = m_entries.insert( TEntryMap::value_type( path, CEntryRecord() ) ).first;
        ++m_entryCount;
    }
    CEntryRecord& record = (*e).second;
    record.entry = entry;
    record.entry.name = path;
    record.exists = true;
    record.stamp = now;
    record.lastUsed = now;
    TrimIfNeeded();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::PutAbsent( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !IsEnabled() || 0 == m_settings.negativeTtlMs )
        return;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TEntryMap::iterator e = m_entries.find( path );
    if ( e == m_entries.end() )
    {
        e = m_entries.insert( TEntryMap::value_type( path, CEntryRecord() ) ).first;
        ++m_entryCount;
    }
    CEntryRecord& record = (*e).second;
    record.entry.Clear();
    record.exists = false;
    record.stamp = now;
    record.lastUsed = now;
    TrimIfNeeded();
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::TouchEntry( const CORE::CString& path )
{GUCEF_TRACE;

    if ( !IsEnabled() )
        return;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TEntryMap::iterator e = m_entries.find( path );
    if ( e != m_entries.end() )
    {
        (*e).second.stamp = now;
        (*e).second.lastUsed = now;
        return;
    }

    // the entry came from the listing of its directory: that listing now vouches for the attributes again, for this one entry,
    // which is kept as an entry record of its own
    TListingMap::iterator l = m_listings.find( ParentOf( path ) );
    if ( l == m_listings.end() )
        return;
    const TEntryVector& entries = *(*l).second.entries;
    DRN::CDRNNamespaceEntry probe;
    probe.name = LeafOf( path );
    TEntryVector::const_iterator found = std::lower_bound( entries.begin(), entries.end(), probe, EntryNameLess );
    if ( found == entries.end() || (*found).name != probe.name )
        return;

    CEntryRecord record;
    record.entry = (*found);
    record.entry.name = path;
    record.exists = true;
    record.stamp = now;
    record.lastUsed = now;
    m_entries[ path ] = record;
    ++m_entryCount;
    TrimIfNeeded();
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMetadataCache::TState
CDrnBridgeMetadataCache::FindListing( const CORE::CString& dir, TEntryVectorPtr& entries, CORE::CString& dirRevision ) const
{GUCEF_TRACE;

    entries.Unlink();
    dirRevision.Clear();
    if ( !IsEnabled() )
        return STATE_MISS;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TListingMap::iterator l = m_listings.find( dir );
    if ( l == m_listings.end() )
        return STATE_MISS;

    CListingRecord& listing = (*l).second;
    listing.lastUsed = now;
    entries = listing.entries;
    dirRevision = listing.dirRevision;
    return IsFresh( listing.stamp, now, false ) ? STATE_FRESH : STATE_STALE;
}

/*-------------------------------------------------------------------------*/

CDrnBridgeMetadataCache::TEntryVectorPtr
CDrnBridgeMetadataCache::PutListing( const CORE::CString& dir, const TEntryVector& entries, const CORE::CString& dirRevision )
{GUCEF_TRACE;

    TEntryVector* sorted = GUCEF_NEW TEntryVector( entries );
    std::sort( sorted->begin(), sorted->end(), EntryNameLess );
    TEntryVectorPtr snapshot( sorted );

    if ( !IsEnabled() || sorted->size() > m_settings.maxCachedEntries )
        return snapshot;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TListingMap::iterator l = m_listings.find( dir );
    if ( l == m_listings.end() )
        l = m_listings.insert( TListingMap::value_type( dir, CListingRecord() ) ).first;
    else
        m_entryCount -= (UInt32) (*l).second.entries->size();

    CListingRecord& listing = (*l).second;
    listing.entries = snapshot;
    listing.dirRevision = dirRevision;
    listing.stamp = now;
    listing.attributeStamp = now;
    listing.attributesStale = false;
    listing.lastUsed = now;
    m_entryCount += (UInt32) sorted->size();
    TrimIfNeeded();
    return snapshot;
}

/*-------------------------------------------------------------------------*/

bool
CDrnBridgeMetadataCache::AreAttributesCurrent( const CORE::CString& dir ) const
{GUCEF_TRACE;

    if ( !IsEnabled() )
        return false;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );
    TListingMap::const_iterator l = m_listings.find( dir );
    if ( l == m_listings.end() )
        return false;
    return !(*l).second.attributesStale && IsFresh( (*l).second.attributeStamp, now, false );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::TouchListing( const CORE::CString& dir )
{GUCEF_TRACE;

    if ( !IsEnabled() )
        return;

    const UInt64 now = NowMs();
    MT::CScopeMutex lock( m_lock );

    TListingMap::iterator l = m_listings.find( dir );
    if ( l != m_listings.end() )
    {
        (*l).second.stamp = now;
        (*l).second.lastUsed = now;
    }
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::InvalidatePath( const CORE::CString& path )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    TEntryMap::iterator e = m_entries.find( path );
    if ( e != m_entries.end() )
    {
        m_entries.erase( e );
        --m_entryCount;
    }

    // the listing of the directory the path is in, and the one of the path itself when it is a directory, cannot vouch for it any more
    TListingMap::iterator l = m_listings.find( ParentOf( path ) );
    if ( l != m_listings.end() )
        (*l).second.attributesStale = true;
    l = m_listings.find( path );
    if ( l != m_listings.end() )
        RemoveListing( l );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::RemoveListing( TListingMap::iterator i )
{GUCEF_TRACE;

    m_entryCount -= (UInt32) (*i).second.entries->size();
    m_listings.erase( i );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::InvalidateDirectory( const CORE::CString& dir )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TListingMap::iterator l = m_listings.find( dir );
    if ( l != m_listings.end() )
        RemoveListing( l );
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::InvalidateAll( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
    m_listings.clear();
    m_entryCount = 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CDrnBridgeMetadataCache::GetEntryCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_entryCount;
}

/*-------------------------------------------------------------------------*/

void
CDrnBridgeMetadataCache::TrimIfNeeded( void )
{GUCEF_TRACE;

    // the lock is held by the caller
    if ( m_entryCount <= m_settings.maxCachedEntries )
        return;

    GUCEF::vector< CTrimCandidate > candidates;
    candidates.reserve( m_entries.size() + m_listings.size() );
    for ( TEntryMap::const_iterator e = m_entries.begin(); e != m_entries.end(); ++e )
    {
        CTrimCandidate candidate;
        candidate.lastUsed = (*e).second.lastUsed;
        candidate.isListing = false;
        candidate.key = (*e).first;
        candidate.count = 1;
        candidates.push_back( candidate );
    }
    for ( TListingMap::const_iterator l = m_listings.begin(); l != m_listings.end(); ++l )
    {
        CTrimCandidate candidate;
        candidate.lastUsed = (*l).second.lastUsed;
        candidate.isListing = true;
        candidate.key = (*l).first;
        candidate.count = (UInt32) (*l).second.entries->size();
        candidates.push_back( candidate );
    }
    std::sort( candidates.begin(), candidates.end(), OlderFirst );

    // down to nine tenths, so that the next insert does not trim again
    const UInt32 target = m_settings.maxCachedEntries - m_settings.maxCachedEntries / 10;
    for ( GUCEF::vector< CTrimCandidate >::const_iterator c = candidates.begin(); c != candidates.end() && m_entryCount > target; ++c )
    {
        if ( (*c).isListing )
        {
            TListingMap::iterator l = m_listings.find( (*c).key );
            if ( l != m_listings.end() )
                RemoveListing( l );
        }
        else
        {
            TEntryMap::iterator e = m_entries.find( (*c).key );
            if ( e != m_entries.end() )
            {
                m_entries.erase( e );
                --m_entryCount;
            }
        }
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRNBRIDGE */
}; /* namespace VFSPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
