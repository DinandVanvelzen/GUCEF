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

#ifndef GUCEF_AUTH_CJWKSCACHE_H
#define GUCEF_AUTH_CJWKSCACHE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MT_CMUTEX_H
#include "gucefMT_CMutex.h"
#define GUCEF_MT_CMUTEX_H
#endif /* GUCEF_MT_CMUTEX_H ? */

#ifndef GUCEF_AUTH_CJWK_H
#include "gucefAUTH_CJwk.h"
#define GUCEF_AUTH_CJWK_H
#endif /* GUCEF_AUTH_CJWK_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The key sets of providers by the address they were fetched from. A set lives as long as the provider says
 *  (Cache-Control max-age) within bounds. A key id that is not in a fresh set allows one refresh of that set, and
 *  another one only after a minimum interval, so an attacker who presents tokens with made up key ids cannot make the
 *  application fetch in a loop. The cache does no network traffic itself: Lookup() says what the caller has to do.
 *  All members are thread safe.
 */
class GUCEF_AUTH_EXPORT_CPP CJwksCache
{
    public:

    static const UInt32 DefaultMinTtlInSeconds = 60;
    static const UInt32 DefaultMaxTtlInSeconds = 86400;
    static const UInt32 DefaultTtlInSeconds = 3600;
    static const UInt32 DefaultMinRefreshIntervalInMs = 10000;
    static const UInt32 MaxEntries = 32;

    enum ELookup
    {
        LOOKUP_FOUND = 0,           /**< the key is in the output */
        LOOKUP_FETCH_NEEDED,        /**< there is no set or it is stale: fetch it and Store() it */
        LOOKUP_REFRESH_NEEDED,      /**< the set is fresh but has no such key and a refresh is allowed now: fetch it and Store() it, then look again */
        LOOKUP_UNKNOWN_KEY          /**< the set is fresh, has no such key, and refreshing is not allowed now (too soon after the last fetch) */
    };

    CJwksCache( void );
    ~CJwksCache();

    void SetPolicy( UInt32 minTtlInSeconds, UInt32 maxTtlInSeconds, UInt32 defaultTtlInSeconds, UInt32 minRefreshIntervalInMs );

    ELookup Lookup( const CORE::CString& address, const CORE::CString& keyId, UInt64 nowMs, CJwk& key );

    /** Keeps the set; the life of it follows from the max-age the provider gave (0 for none) */
    void Store( const CORE::CString& address, const CJwks& jwks, UInt32 maxAgeInSeconds, UInt64 nowMs );

    void Remove( const CORE::CString& address );

    void Clear( void );

    /** How often a set was stored: for the tests and for statistics */
    UInt32 GetStoreCount( void ) const;

    /** The max-age of a Cache-Control header value in seconds; 0 when there is none; 1 when the header says not to cache, which the minimum life of the cache makes the shortest there is */
    static UInt32 ParseMaxAge( const CORE::CString& cacheControl );

    private:

    struct SEntry
    {
        CJwks jwks;
        UInt64 expiresAtMs;
        UInt64 storedAtMs;
        UInt64 lastKeyRefreshAtMs;

        SEntry( void ) : jwks(), expiresAtMs( 0 ), storedAtMs( 0 ), lastKeyRefreshAtMs( 0 ) {}
    };

    typedef GUCEF::map< CORE::CString, SEntry > TEntryMap;

    CJwksCache( const CJwksCache& src );                /**< not implemented */
    CJwksCache& operator=( const CJwksCache& src );     /**< not implemented */

    private:

    mutable MT::CMutex m_lock;
    TEntryMap m_entries;
    UInt32 m_minTtl;
    UInt32 m_maxTtl;
    UInt32 m_defaultTtl;
    UInt32 m_minRefreshIntervalInMs;
    UInt32 m_storeCount;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace AUTH */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_AUTH_CJWKSCACHE_H ? */
