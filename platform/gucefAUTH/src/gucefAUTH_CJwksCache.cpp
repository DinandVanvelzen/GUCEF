/*
 *  gucefAUTH: GUCEF module providing player and service authentication through external identity providers
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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefAUTH_CJwksCache.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CJwksCache::DefaultMinTtlInSeconds;
const UInt32 CJwksCache::DefaultMaxTtlInSeconds;
const UInt32 CJwksCache::DefaultTtlInSeconds;
const UInt32 CJwksCache::DefaultMinRefreshIntervalInMs;
const UInt32 CJwksCache::MaxEntries;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CJwksCache::CJwksCache( void )
    : m_lock()
    , m_entries()
    , m_minTtl( DefaultMinTtlInSeconds )
    , m_maxTtl( DefaultMaxTtlInSeconds )
    , m_defaultTtl( DefaultTtlInSeconds )
    , m_minRefreshIntervalInMs( DefaultMinRefreshIntervalInMs )
    , m_storeCount( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CJwksCache::~CJwksCache()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CJwksCache::SetPolicy( UInt32 minTtlInSeconds, UInt32 maxTtlInSeconds, UInt32 defaultTtlInSeconds, UInt32 minRefreshIntervalInMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_minTtl = minTtlInSeconds;
    m_maxTtl = maxTtlInSeconds < minTtlInSeconds ? minTtlInSeconds : maxTtlInSeconds;
    m_defaultTtl = defaultTtlInSeconds < m_minTtl ? m_minTtl : ( defaultTtlInSeconds > m_maxTtl ? m_maxTtl : defaultTtlInSeconds );
    m_minRefreshIntervalInMs = minRefreshIntervalInMs;
}

/*-------------------------------------------------------------------------*/

CJwksCache::ELookup
CJwksCache::Lookup( const CORE::CString& address, const CORE::CString& keyId, UInt64 nowMs, CJwk& key )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TEntryMap::iterator i = m_entries.find( address );
    if ( i == m_entries.end() || nowMs >= (*i).second.expiresAtMs )
        return LOOKUP_FETCH_NEEDED;

    SEntry& entry = (*i).second;
    const CJwk* found = entry.jwks.Find( keyId );
    if ( GUCEF_NULL != found )
    {
        key = *found;
        return LOOKUP_FOUND;
    }

    // A key id that this set does not know: a refresh, at most one per interval, and not right after the set was fetched.
    // The refresh is claimed now so that callers which look at the same time do not all fetch
    UInt64 last = entry.storedAtMs > entry.lastKeyRefreshAtMs ? entry.storedAtMs : entry.lastKeyRefreshAtMs;
    if ( nowMs >= last + m_minRefreshIntervalInMs )
    {
        entry.lastKeyRefreshAtMs = nowMs;
        return LOOKUP_REFRESH_NEEDED;
    }
    return LOOKUP_UNKNOWN_KEY;
}

/*-------------------------------------------------------------------------*/

void
CJwksCache::Store( const CORE::CString& address, const CJwks& jwks, UInt32 maxAgeInSeconds, UInt64 nowMs )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );

    UInt32 ttl = 0 == maxAgeInSeconds ? m_defaultTtl : maxAgeInSeconds;
    if ( ttl < m_minTtl )
        ttl = m_minTtl;
    if ( ttl > m_maxTtl )
        ttl = m_maxTtl;

    if ( m_entries.find( address ) == m_entries.end() && m_entries.size() >= MaxEntries )
    {
        // The oldest goes
        TEntryMap::iterator oldest = m_entries.begin();
        for ( TEntryMap::iterator i = m_entries.begin(); i != m_entries.end(); ++i )
            if ( (*i).second.storedAtMs < (*oldest).second.storedAtMs )
                oldest = i;
        m_entries.erase( oldest );
    }

    SEntry& entry = m_entries[ address ];
    UInt64 lastRefresh = entry.lastKeyRefreshAtMs;
    entry.jwks = jwks;
    entry.storedAtMs = nowMs;
    entry.expiresAtMs = nowMs + (UInt64) ttl * 1000;
    // The limit on refreshes for unknown key ids outlives the set
    entry.lastKeyRefreshAtMs = lastRefresh;
    ++m_storeCount;
}

/*-------------------------------------------------------------------------*/

void
CJwksCache::Remove( const CORE::CString& address )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.erase( address );
}

/*-------------------------------------------------------------------------*/

void
CJwksCache::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CJwksCache::GetStoreCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_storeCount;
}

/*-------------------------------------------------------------------------*/

UInt32
CJwksCache::ParseMaxAge( const CORE::CString& cacheControl )
{GUCEF_TRACE;

    if ( cacheControl.IsNULLOrEmpty() )
        return 0;
    CORE::CString lower = cacheControl.Lowercase();
    CORE::CString::StringVector directives = lower.ParseElements( ',', false );
    UInt32 maxAge = 0;
    for ( size_t i=0; i<directives.size(); ++i )
    {
        CORE::CString directive = directives[ i ].RemoveChar( ' ' );
        // A provider that says the set is not to be kept gets the shortest life there is
        if ( directive == "no-store" || directive == "no-cache" )
            return 1;
        if ( directive.StartsWith( "max-age=" ) )
        {
            CORE::CString number = directive.CutChars( 8, true );
            UInt32 digits = 0;
            UInt64 value = 0;
            const char* p = number.C_String();
            while ( '\0' != p[ digits ] && p[ digits ] >= '0' && p[ digits ] <= '9' && digits < 10 )
            {
                value = value * 10 + (UInt64) ( p[ digits ] - '0' );
                ++digits;
            }
            if ( 0 == digits )
                continue;
            maxAge = value > GUCEF_UINT32MAX ? GUCEF_UINT32MAX : (UInt32) value;
        }
    }
    return maxAge;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
