/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
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

#ifndef GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H
#define GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H

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

#ifndef GUCEF_CORE_MACROS_H
#include "gucefCORE_macros.h"
#define GUCEF_CORE_MACROS_H
#endif /* GUCEF_CORE_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

/** What CTCommandIdempotencyCache::Find() found */
enum ECommandIdempotencyLookup
{
    COMMANDIDEMPOTENCY_MISS     = 0 ,   /**< not applied before, or its record expired or was pushed out: apply it */
    COMMANDIDEMPOTENCY_HIT      = 1 ,   /**< applied before with the same arguments: answer with the stored outcome */
    COMMANDIDEMPOTENCY_CONFLICT = 2     /**< the command id was used before with other arguments: refuse it */
};
typedef enum ECommandIdempotencyLookup TCommandIdempotencyLookup;

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The outcomes of commands by ( subject, command id ), so that a repeated command returns the stored outcome instead
 *  of being applied twice: the idempotency key of a remote command, the request key of a tool call that starts work.
 *
 *  - An outcome is any copyable value (TOutcome): a CDataNode (CCommandIdempotencyCache), a struct of the caller.
 *  - An optional digest of the arguments is kept with the outcome: the same command id with another digest is a
 *    conflict, so that a reused key cannot silently answer for a different command. An empty digest is not compared.
 *  - In memory only; bounded by a time to live, a maximum per subject and a maximum in total, oldest out first
 *    (0 = no bound for that dimension). Times are monotonic milliseconds given by the caller, so tests control them.
 *  - Thread safe. The lock is a leaf: nothing is called under it except the copy of the outcome.
 */
template< typename TOutcome >
class CTCommandIdempotencyCache
{
    public:

    static const UInt64 DefaultTtlMs = 86400000;
    static const UInt32 DefaultMaxPerSubject = 256;
    static const UInt32 DefaultMaxTotal = 4096;

    CTCommandIdempotencyCache( void );

    /** 0 = no bound for that dimension */
    void SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal );

    UInt64 GetTtlMs( void ) const;
    UInt32 GetMaxPerSubject( void ) const;
    UInt32 GetMaxTotal( void ) const;

    /**
     *  HIT with the stored outcome when the command was stored before with the same digest (or either digest is empty)
     *  and its record did not expire; CONFLICT when it was stored with another digest; MISS otherwise
     */
    TCommandIdempotencyLookup Find( const CString& subject         ,
                                    const CString& commandId       ,
                                    const CString& argumentsDigest ,
                                    UInt64 nowMs                   ,
                                    TOutcome& outcome              );

    /** Find() without a digest: HIT or MISS */
    bool Find( const CString& subject, const CString& commandId, UInt64 nowMs, TOutcome& outcome );

    /** Stores (or replaces) the outcome of the command; the oldest records go first when a bound is reached */
    void Store( const CString& subject         ,
                const CString& commandId       ,
                const CString& argumentsDigest ,
                UInt64 nowMs                   ,
                const TOutcome& outcome        );

    /** Store() without a digest */
    void Store( const CString& subject, const CString& commandId, UInt64 nowMs, const TOutcome& outcome );

    /** Forgets one record; false when there was none */
    bool Remove( const CString& subject, const CString& commandId );

    UInt32 GetCount( void ) const;

    UInt32 GetCountOfSubject( const CString& subject ) const;

    void Clear( void );

    private:

    struct SEntry
    {
        UInt64 storedAtMs;
        UInt64 sequence;
        CString subject;
        CString argumentsDigest;
        TOutcome outcome;
    };
    typedef GUCEF::map< CString, SEntry > TEntryMap;            /**< key: subject, line feed, command id */
    typedef GUCEF::map< UInt64, CString > TOrderMap;            /**< sequence to key, oldest first */
    typedef GUCEF::map< CString, UInt32 > TSubjectCountMap;

    static CString MakeKey( const CString& subject, const CString& commandId );
    void Expire( UInt64 nowMs );
    void RemoveKey( const CString& key );
    void RemoveOldestOfSubject( const CString& subject );

    CTCommandIdempotencyCache( const CTCommandIdempotencyCache& src );              /**< not implemented */
    CTCommandIdempotencyCache& operator=( const CTCommandIdempotencyCache& src );   /**< not implemented */

    mutable MT::CMutex m_lock;
    UInt64 m_ttlMs;
    UInt32 m_maxPerSubject;
    UInt32 m_maxTotal;
    UInt64 m_nextSequence;
    TEntryMap m_entries;
    TOrderMap m_order;
    TSubjectCountMap m_subjectCounts;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

template< typename TOutcome >
CTCommandIdempotencyCache< TOutcome >::CTCommandIdempotencyCache( void )
    : m_lock()
    , m_ttlMs( DefaultTtlMs )
    , m_maxPerSubject( DefaultMaxPerSubject )
    , m_maxTotal( DefaultMaxTotal )
    , m_nextSequence( 1 )
    , m_entries()
    , m_order()
    , m_subjectCounts()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
CString
CTCommandIdempotencyCache< TOutcome >::MakeKey( const CString& subject, const CString& commandId )
{GUCEF_TRACE;

    return subject + "\n" + commandId;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::SetBounds( UInt64 ttlMs, UInt32 maxPerSubject, UInt32 maxTotal )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_ttlMs = ttlMs;
    m_maxPerSubject = maxPerSubject;
    m_maxTotal = maxTotal;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
UInt64
CTCommandIdempotencyCache< TOutcome >::GetTtlMs( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_ttlMs;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
UInt32
CTCommandIdempotencyCache< TOutcome >::GetMaxPerSubject( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxPerSubject;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
UInt32
CTCommandIdempotencyCache< TOutcome >::GetMaxTotal( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_maxTotal;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::RemoveKey( const CString& key )
{GUCEF_TRACE;

    typename TEntryMap::iterator i = m_entries.find( key );
    if ( i == m_entries.end() )
        return;
    m_order.erase( (*i).second.sequence );
    TSubjectCountMap::iterator s = m_subjectCounts.find( (*i).second.subject );
    if ( s != m_subjectCounts.end() )
    {
        if ( (*s).second <= 1 )
            m_subjectCounts.erase( s );
        else
            --(*s).second;
    }
    m_entries.erase( i );
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::Expire( UInt64 nowMs )
{GUCEF_TRACE;

    if ( 0 == m_ttlMs )
        return;

    // records are stored in time order: the oldest are in front
    while ( !m_order.empty() )
    {
        CString key = (*m_order.begin()).second;
        typename TEntryMap::iterator i = m_entries.find( key );
        if ( i == m_entries.end() )
        {
            m_order.erase( m_order.begin() );
            continue;
        }
        if ( (*i).second.storedAtMs + m_ttlMs > nowMs )
            break;
        RemoveKey( key );
    }
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::RemoveOldestOfSubject( const CString& subject )
{GUCEF_TRACE;

    TOrderMap::iterator o = m_order.begin();
    while ( o != m_order.end() )
    {
        typename TEntryMap::iterator i = m_entries.find( (*o).second );
        if ( i != m_entries.end() && (*i).second.subject == subject )
        {
            CString key = (*o).second;
            RemoveKey( key );
            return;
        }
        ++o;
    }
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
TCommandIdempotencyLookup
CTCommandIdempotencyCache< TOutcome >::Find( const CString& subject         ,
                                             const CString& commandId       ,
                                             const CString& argumentsDigest ,
                                             UInt64 nowMs                   ,
                                             TOutcome& outcome              )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Expire( nowMs );
    typename TEntryMap::iterator i = m_entries.find( MakeKey( subject, commandId ) );
    if ( i == m_entries.end() )
        return COMMANDIDEMPOTENCY_MISS;
    const SEntry& entry = (*i).second;
    if ( !argumentsDigest.IsNULLOrEmpty() && !entry.argumentsDigest.IsNULLOrEmpty() && argumentsDigest != entry.argumentsDigest )
        return COMMANDIDEMPOTENCY_CONFLICT;
    outcome = entry.outcome;
    return COMMANDIDEMPOTENCY_HIT;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
bool
CTCommandIdempotencyCache< TOutcome >::Find( const CString& subject, const CString& commandId, UInt64 nowMs, TOutcome& outcome )
{GUCEF_TRACE;

    return COMMANDIDEMPOTENCY_HIT == Find( subject, commandId, CString::Empty, nowMs, outcome );
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::Store( const CString& subject         ,
                                              const CString& commandId       ,
                                              const CString& argumentsDigest ,
                                              UInt64 nowMs                   ,
                                              const TOutcome& outcome        )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    Expire( nowMs );
    CString key = MakeKey( subject, commandId );
    RemoveKey( key );
    if ( m_maxPerSubject > 0 )
    {
        TSubjectCountMap::iterator s = m_subjectCounts.find( subject );
        while ( s != m_subjectCounts.end() && (*s).second >= m_maxPerSubject )
        {
            RemoveOldestOfSubject( subject );
            s = m_subjectCounts.find( subject );
        }
    }
    if ( m_maxTotal > 0 )
    {
        while ( m_entries.size() >= m_maxTotal && !m_order.empty() )
        {
            CString oldest = (*m_order.begin()).second;
            RemoveKey( oldest );
        }
    }
    SEntry& entry = m_entries[ key ];
    entry.storedAtMs = nowMs;
    entry.sequence = m_nextSequence++;
    entry.subject = subject;
    entry.argumentsDigest = argumentsDigest;
    entry.outcome = outcome;
    m_order[ entry.sequence ] = key;
    ++m_subjectCounts[ subject ];
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::Store( const CString& subject, const CString& commandId, UInt64 nowMs, const TOutcome& outcome )
{GUCEF_TRACE;

    Store( subject, commandId, CString::Empty, nowMs, outcome );
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
bool
CTCommandIdempotencyCache< TOutcome >::Remove( const CString& subject, const CString& commandId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CString key = MakeKey( subject, commandId );
    if ( m_entries.end() == m_entries.find( key ) )
        return false;
    RemoveKey( key );
    return true;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
UInt32
CTCommandIdempotencyCache< TOutcome >::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
UInt32
CTCommandIdempotencyCache< TOutcome >::GetCountOfSubject( const CString& subject ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TSubjectCountMap::const_iterator s = m_subjectCounts.find( subject );
    return s == m_subjectCounts.end() ? 0 : (*s).second;
}

/*-------------------------------------------------------------------------*/

template< typename TOutcome >
void
CTCommandIdempotencyCache< TOutcome >::Clear( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_entries.clear();
    m_order.clear();
    m_subjectCounts.clear();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_CORE_CTCOMMANDIDEMPOTENCYCACHE_H ? */
