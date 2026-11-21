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

#include <string.h>

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefLabLink_WebSessions.h"

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

CWebSessionTable::CWebSessionTable( UInt32 maxSessions, UInt32 maxPerUser )
    : m_sessions()
    , m_maxSessions( maxSessions < 1 ? 1 : maxSessions )
    , m_maxPerUser( maxPerUser < 1 ? 1 : maxPerUser )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebSessionTable::KeyOf( const CORE::CString& id )
{GUCEF_TRACE;

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( id.C_String(), id.Length(), digest );
    return CORE::Base16Encode( digest, sizeof( digest ) );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionTable::HasEnded( const SWebSession& session, UInt64 nowInMs )
{GUCEF_TRACE;

    return nowInMs >= session.endsInMs || nowInMs >= session.lastUsedInMs + session.idleTimeoutInMs;
}

/*-------------------------------------------------------------------------*/

void
CWebSessionTable::RemoveLeastRecentlyUsed( const CORE::CString* user )
{GUCEF_TRACE;

    TSessionMap::iterator oldest = m_sessions.end();
    for ( TSessionMap::iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
    {
        if ( GUCEF_NULL != user && (*i).second.user != *user )
            continue;
        if ( oldest == m_sessions.end() || (*i).second.lastUsedInMs < (*oldest).second.lastUsedInMs )
            oldest = i;
    }
    if ( oldest != m_sessions.end() )
        m_sessions.erase( oldest );
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionTable::Create( const CORE::CString& user, bool viaToken, bool persistent, UInt64 nowInMs, UInt32 idleTimeoutInMs,
                          UInt64 lifetimeInMs, CORE::CString& id )
{GUCEF_TRACE;

    id.Clear();
    UInt8 random[ 32 ];
    if ( !CORE::CRandom::GetRandomBytes( random, sizeof( random ) ) )
        return false;
    id = CORE::Base16Encode( random, sizeof( random ) ).Lowercase();
    ::memset( random, 0, sizeof( random ) );

    // within the bounds: the ended ones go first, then the one used longest ago, of this user first
    while ( GetCountOfUser( user ) >= m_maxPerUser )
        RemoveLeastRecentlyUsed( &user );
    if ( m_sessions.size() >= m_maxSessions )
        Sweep( nowInMs );
    while ( m_sessions.size() >= m_maxSessions )
        RemoveLeastRecentlyUsed( GUCEF_NULL );

    SWebSession session;
    session.user = user;
    session.viaToken = viaToken;
    session.persistent = persistent;
    session.createdInMs = nowInMs;
    session.lastUsedInMs = nowInMs;
    session.endsInMs = nowInMs + lifetimeInMs;
    session.idleTimeoutInMs = idleTimeoutInMs;
    m_sessions[ KeyOf( id ) ] = session;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionTable::Use( const CORE::CString& id, UInt64 nowInMs, SWebSession& session )
{GUCEF_TRACE;

    if ( id.Length() != 64 )
        return false;
    TSessionMap::iterator i = m_sessions.find( KeyOf( id ) );
    if ( i == m_sessions.end() )
        return false;
    if ( HasEnded( (*i).second, nowInMs ) )
    {
        m_sessions.erase( i );
        return false;
    }
    (*i).second.lastUsedInMs = nowInMs;
    session = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebSessionTable::Remove( const CORE::CString& id )
{GUCEF_TRACE;

    TSessionMap::iterator i = m_sessions.find( KeyOf( id ) );
    if ( i == m_sessions.end() )
        return false;
    m_sessions.erase( i );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSessionTable::RemoveUser( const CORE::CString& user )
{GUCEF_TRACE;

    UInt32 removed = 0;
    TSessionMap::iterator i = m_sessions.begin();
    while ( i != m_sessions.end() )
    {
        if ( !(*i).second.viaToken && (*i).second.user == user )
        {
            m_sessions.erase( i++ );
            ++removed;
        }
        else
            ++i;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSessionTable::Sweep( UInt64 nowInMs )
{GUCEF_TRACE;

    UInt32 removed = 0;
    TSessionMap::iterator i = m_sessions.begin();
    while ( i != m_sessions.end() )
    {
        if ( HasEnded( (*i).second, nowInMs ) )
        {
            m_sessions.erase( i++ );
            ++removed;
        }
        else
            ++i;
    }
    return removed;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSessionTable::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_sessions.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebSessionTable::GetCountOfUser( const CORE::CString& user ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TSessionMap::const_iterator i = m_sessions.begin(); i != m_sessions.end(); ++i )
        if ( (*i).second.user == user )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
