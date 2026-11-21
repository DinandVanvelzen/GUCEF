/*
 *  gucefLabLink: links the machines of a home network to its k3s cluster
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

#include "gucefLabLink_LoginThrottle.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CLoginThrottle::CLoginThrottle( UInt32 maxEntries, UInt32 freeFailures, UInt32 baseDelayInMs, UInt32 maxDelayInMs, UInt32 forgetAfterInMs )
    : m_entries()
    , m_maxEntries( maxEntries < 1 ? 1 : maxEntries )
    , m_freeFailures( freeFailures )
    , m_baseDelayInMs( baseDelayInMs < 1 ? 1 : baseDelayInMs )
    , m_maxDelayInMs( maxDelayInMs < baseDelayInMs ? baseDelayInMs : maxDelayInMs )
    , m_forgetAfterInMs( forgetAfterInMs < maxDelayInMs ? maxDelayInMs : forgetAfterInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

UInt32
CLoginThrottle::GetWaitInMs( const CORE::CString& key, UInt64 nowInMs ) const
{GUCEF_TRACE;

    TEntryMap::const_iterator i = m_entries.find( key );
    if ( i == m_entries.end() || nowInMs >= (*i).second.nextAttemptInMs )
        return 0;
    return (UInt32) ( (*i).second.nextAttemptInMs - nowInMs );
}

/*-------------------------------------------------------------------------*/

void
CLoginThrottle::OnFailure( const CORE::CString& key, UInt64 nowInMs )
{GUCEF_TRACE;

    TEntryMap::iterator i = m_entries.find( key );
    if ( i == m_entries.end() )
    {
        if ( m_entries.size() >= m_maxEntries )
            Sweep( nowInMs );
        while ( m_entries.size() >= m_maxEntries )
        {
            TEntryMap::iterator oldest = m_entries.begin();
            for ( TEntryMap::iterator e = m_entries.begin(); e != m_entries.end(); ++e )
                if ( (*e).second.lastFailureInMs < (*oldest).second.lastFailureInMs )
                    oldest = e;
            m_entries.erase( oldest );
        }
        i = m_entries.insert( TEntryMap::value_type( key, SEntry() ) ).first;
    }
    SEntry& entry = (*i).second;
    if ( entry.failures < 1000 )
        ++entry.failures;
    entry.lastFailureInMs = nowInMs;
    if ( entry.failures <= m_freeFailures )
    {
        entry.nextAttemptInMs = nowInMs;
        return;
    }
    // base, twice that, four times, ... up to the bound
    UInt64 delay = m_baseDelayInMs;
    for ( UInt32 n = m_freeFailures + 1; n < entry.failures && delay < m_maxDelayInMs; ++n )
        delay *= 2;
    if ( delay > m_maxDelayInMs )
        delay = m_maxDelayInMs;
    entry.nextAttemptInMs = nowInMs + delay;
}

/*-------------------------------------------------------------------------*/

void
CLoginThrottle::OnSuccess( const CORE::CString& key )
{GUCEF_TRACE;

    m_entries.erase( key );
}

/*-------------------------------------------------------------------------*/

void
CLoginThrottle::Sweep( UInt64 nowInMs )
{GUCEF_TRACE;

    TEntryMap::iterator i = m_entries.begin();
    while ( i != m_entries.end() )
    {
        if ( nowInMs >= (*i).second.lastFailureInMs + m_forgetAfterInMs && nowInMs >= (*i).second.nextAttemptInMs )
            m_entries.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CLoginThrottle::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CLoginThrottle::GetFailures( const CORE::CString& key ) const
{GUCEF_TRACE;

    TEntryMap::const_iterator i = m_entries.find( key );
    return i == m_entries.end() ? 0 : (*i).second.failures;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
