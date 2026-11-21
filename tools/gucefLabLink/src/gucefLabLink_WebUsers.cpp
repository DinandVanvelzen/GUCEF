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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefLabLink_Protocol.h"
#include "gucefLabLink_WebUsers.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

using namespace GUCEF;

namespace GUCEFLabLink {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const char* WebRoleAdmin = "admin";
const char* WebRoleViewer = "viewer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static CORE::CString
DigestOf( const CORE::CString& text )
{GUCEF_TRACE;

    UInt8 digest[ CORE::CSha256::DigestSize ];
    CORE::CSha256::Hash( text.C_String(), text.Length(), digest );
    return CORE::Base16Encode( digest, sizeof( digest ) );
}

/*-------------------------------------------------------------------------*/

/** A field of a line of the users file: no tabs or line ends in it */
static CORE::CString
Field( const CORE::CString& value )
{GUCEF_TRACE;

    return value.ReplaceChar( '\t', ' ' ).ReplaceChar( '\n', ' ' ).ReplaceChar( '\r', ' ' );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebUserStore::CWebUserStore( CStore& store )
    : m_store( store )
    , m_users()
    , m_digest()
    , m_pendingChanged()
    , m_lastReadInMs( 0 )
    , m_read( false )
    , m_lock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::IsValidName( const CORE::CString& name )
{GUCEF_TRACE;

    const UInt32 length = name.Length();
    if ( 0 == length || length > 64 )
        return false;
    for ( UInt32 i=0; i<length; ++i )
    {
        const char c = name[ i ];
        const bool valid = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '_' == c || '-' == c || '@' == c;
        if ( !valid )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::IsValidRole( const CORE::CString& role )
{GUCEF_TRACE;

    return role == WebRoleAdmin || role == WebRoleViewer;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CWebUserStore::Serialize( const GUCEF::vector< SWebUser >& users )
{GUCEF_TRACE;

    CORE::CString content;
    for ( size_t i=0; i<users.size(); ++i )
    {
        const SWebUser& u = users[ i ];
        content += Field( u.name ) + "\t" + Field( u.role ) + "\t" + ( u.disabled ? "disabled" : "enabled" ) + "\t" +
                   CORE::ToString( u.createdInUnixSecs ) + "\t" + Field( u.record ) + "\n";
    }
    return content;
}

/*-------------------------------------------------------------------------*/

void
CWebUserStore::Parse( const CORE::CString& content, GUCEF::vector< SWebUser >& users )
{GUCEF_TRACE;

    users.clear();
    CORE::CString::StringVector lines = content.ParseElements( '\n', false );
    for ( size_t i=0; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ];
        while ( line.EndsWith( "\r" ) )
            line = line.CutChars( 1, false );
        if ( line.IsNULLOrEmpty() || '#' == line[ 0 ] )
            continue;
        CORE::CString::StringVector fields = line.ParseElements( '\t', true );
        if ( fields.size() < 5 || !IsValidName( fields[ 0 ] ) || !IsValidRole( fields[ 1 ] ) )
            continue;
        SWebUser user;
        user.name = fields[ 0 ];
        user.role = fields[ 1 ];
        user.disabled = fields[ 2 ] != "enabled";
        user.createdInUnixSecs = CORE::StringToUInt64( fields[ 3 ], 0 );
        user.record = fields[ 4 ];
        users.push_back( user );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::ReadLocked( TUserMap& users, CORE::CString& content, CORE::CString& problem ) const
{GUCEF_TRACE;

    users.clear();
    content.Clear();
    bool missing = false;
    if ( !m_store.ReadUsers( content, missing, problem ) )
    {
        if ( !missing )
            return false;
        content.Clear();
        problem.Clear();
    }
    GUCEF::vector< SWebUser > list;
    Parse( content, list );
    for ( size_t i=0; i<list.size(); ++i )
        users[ list[ i ].name ] = list[ i ];
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::WriteLocked( const TUserMap& users, CORE::CString& problem )
{GUCEF_TRACE;

    GUCEF::vector< SWebUser > list;
    for ( TUserMap::const_iterator i = users.begin(); i != users.end(); ++i )
        list.push_back( (*i).second );
    const CORE::CString content = Serialize( list );
    if ( !m_store.WriteUsers( content, problem ) )
        return false;
    // what the file held beyond what this store knew is a change as well: a command line that changed it meanwhile
    Adopt( users, content, &m_pendingChanged );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebUserStore::Adopt( const TUserMap& users, const CORE::CString& content, CORE::CString::StringVector* changed )
{GUCEF_TRACE;

    if ( GUCEF_NULL != changed )
    {
        for ( TUserMap::const_iterator i = m_users.begin(); i != m_users.end(); ++i )
        {
            const SWebUser& before = (*i).second;
            TUserMap::const_iterator now = users.find( (*i).first );
            if ( now == users.end() || (*now).second.record != before.record || (*now).second.disabled != before.disabled ||
                 (*now).second.role != before.role )
                changed->push_back( before.name );
        }
    }
    m_users = users;
    m_digest = DigestOf( content );
    m_read = true;
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::Load( CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUserMap users;
    CORE::CString content;
    if ( !ReadLocked( users, content, problem ) )
        return false;
    Adopt( users, content, GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::Refresh( UInt64 nowInMs, UInt32 intervalInMs, CORE::CString::StringVector& changed )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_pendingChanged.size(); ++i )
        changed.push_back( m_pendingChanged[ i ] );
    m_pendingChanged.clear();
    if ( m_read && nowInMs < m_lastReadInMs + intervalInMs && nowInMs >= m_lastReadInMs )
        return true;
    m_lastReadInMs = nowInMs;
    TUserMap users;
    CORE::CString content;
    CORE::CString problem;
    if ( !ReadLocked( users, content, problem ) )
        return false;
    if ( m_read && DigestOf( content ) == m_digest )
        return true;
    Adopt( users, content, m_read ? &changed : GUCEF_NULL );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::Find( const CORE::CString& name, SWebUser& user ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUserMap::const_iterator i = m_users.find( name );
    if ( i == m_users.end() )
        return false;
    user = (*i).second;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebUserStore::GetAll( GUCEF::vector< SWebUser >& users ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    users.clear();
    for ( TUserMap::const_iterator i = m_users.begin(); i != m_users.end(); ++i )
        users.push_back( (*i).second );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebUserStore::GetCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_users.size();
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::Add( const SWebUser& user, CORE::CString& problem )
{GUCEF_TRACE;

    if ( !IsValidName( user.name ) || !IsValidRole( user.role ) || user.record.IsNULLOrEmpty() )
    {
        problem = "a user needs a name of letters, digits, '.', '_', '-' or '@' (at most 64), the role admin or viewer, and a password";
        return false;
    }
    MT::CScopeMutex lock( m_lock );
    TUserMap users;
    CORE::CString content;
    if ( !ReadLocked( users, content, problem ) )
        return false;
    if ( users.find( user.name ) != users.end() )
    {
        problem = "there is a user " + user.name + " already";
        return false;
    }
    users[ user.name ] = user;
    return WriteLocked( users, problem );
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::SetRecord( const CORE::CString& name, const CORE::CString& record, CORE::CString& problem, const CORE::CString* upgradeOf )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUserMap users;
    CORE::CString content;
    if ( !ReadLocked( users, content, problem ) )
        return false;
    TUserMap::iterator i = users.find( name );
    if ( i == users.end() )
    {
        problem = "there is no user " + name;
        return false;
    }
    if ( GUCEF_NULL != upgradeOf && (*i).second.record != *upgradeOf )
    {
        problem = "the password of " + name + " was changed meanwhile";
        return false;
    }
    (*i).second.record = record;
    if ( !WriteLocked( users, problem ) )
        return false;
    if ( GUCEF_NULL != upgradeOf )
    {
        // the same password: its sessions stay
        CORE::CString::StringVector kept;
        for ( size_t n=0; n<m_pendingChanged.size(); ++n )
            if ( m_pendingChanged[ n ] != name )
                kept.push_back( m_pendingChanged[ n ] );
        m_pendingChanged = kept;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::SetDisabled( const CORE::CString& name, bool disabled, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUserMap users;
    CORE::CString content;
    if ( !ReadLocked( users, content, problem ) )
        return false;
    TUserMap::iterator i = users.find( name );
    if ( i == users.end() )
    {
        problem = "there is no user " + name;
        return false;
    }
    (*i).second.disabled = disabled;
    return WriteLocked( users, problem );
}

/*-------------------------------------------------------------------------*/

bool
CWebUserStore::Remove( const CORE::CString& name, CORE::CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    TUserMap users;
    CORE::CString content;
    if ( !ReadLocked( users, content, problem ) )
        return false;
    TUserMap::iterator i = users.find( name );
    if ( i == users.end() )
    {
        problem = "there is no user " + name;
        return false;
    }
    users.erase( i );
    return WriteLocked( users, problem );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUCEFLabLink */

/*-------------------------------------------------------------------------*/
