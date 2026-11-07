/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMAILADDRESS_H
#include "gucefCOM_CMailAddress.h"
#define GUCEF_COM_CMAILADDRESS_H
#endif /* GUCEF_COM_CMAILADDRESS_H ? */

#include "matchprovidermail_CMailFiles.h"
#include "matchprovidermail_CMailAddressBook.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailAddressEntry::CMailAddressEntry( void )
    : address()
    , publicKey()
    , displayName()
    , firstSeen( 0 )
    , lastSeen( 0 )
    , blocked( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailAddressBook::CMailAddressBook( void )
    : CORE::CIDataNodeSerializable()
    , m_entries()
{GUCEF_TRACE;

}

CMailAddressBook::CMailAddressBook( const CMailAddressBook& src )
    : CORE::CIDataNodeSerializable( src )
    , m_entries( src.m_entries )
{GUCEF_TRACE;

}

CMailAddressBook::~CMailAddressBook()
{GUCEF_TRACE;

}

CMailAddressBook&
CMailAddressBook::operator=( const CMailAddressBook& src )
{GUCEF_TRACE;

    if ( &src != this )
        m_entries = src.m_entries;
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailAddressBook::Normalize( const CORE::CString& address )
{GUCEF_TRACE;

    return address.Lowercase();
}

/*-------------------------------------------------------------------------*/

static bool
IsUsableAddress( const CORE::CString& address )
{GUCEF_TRACE;

    if ( address.IsNULLOrEmpty() || address.Length() > 254 )
        return false;
    return COM::CMailAddress( address ).IsValid();
}

static bool
IsUsableKey( const CORE::CString& key )
{GUCEF_TRACE;

    return MULTIPLAY::CMatchSigning::IsHexOfLength( key, MULTIPLAY::CMatchSigning::PublicKeyHexSize );
}

/*-------------------------------------------------------------------------*/

CMailAddressEntry*
CMailAddressBook::FindEntry( const CORE::CString& address )
{GUCEF_TRACE;

    const CORE::CString wanted = Normalize( address );
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].address == wanted )
            return &m_entries[ i ];
    return GUCEF_NULL;
}

const CMailAddressEntry*
CMailAddressBook::FindEntry( const CORE::CString& address ) const
{GUCEF_TRACE;

    const CORE::CString wanted = Normalize( address );
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].address == wanted )
            return &m_entries[ i ];
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

CMailAddressBook::ECheck
CMailAddressBook::Check( const CORE::CString& address, const CORE::CString& publicKeyHex ) const
{GUCEF_TRACE;

    const CMailAddressEntry* entry = FindEntry( address );
    if ( GUCEF_NULL == entry || entry->publicKey.IsNULLOrEmpty() )
        return CHECK_UNKNOWN;
    return entry->publicKey == publicKeyHex.Lowercase() ? CHECK_SAME : CHECK_DIFFERENT;
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::Find( const CORE::CString& address, CMailAddressEntry& entry ) const
{GUCEF_TRACE;

    const CMailAddressEntry* found = FindEntry( address );
    if ( GUCEF_NULL == found )
        return false;
    entry = *found;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailAddressBook::MakeRoom( void )
{GUCEF_TRACE;

    while ( m_entries.size() >= MaxEntries )
    {
        size_t oldest = m_entries.size();
        for ( size_t i=0; i<m_entries.size(); ++i )
            if ( !m_entries[ i ].blocked && ( oldest == m_entries.size() || m_entries[ i ].lastSeen < m_entries[ oldest ].lastSeen ) )
                oldest = i;
        if ( oldest == m_entries.size() )
            return;
        m_entries.erase( m_entries.begin() + oldest );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::Remember( const CORE::CString& address, const CORE::CString& publicKeyHex, const CORE::CString& displayName, Int64 now )
{GUCEF_TRACE;

    if ( !IsUsableAddress( address ) || !IsUsableKey( publicKeyHex ) )
        return false;
    CMailAddressEntry* entry = FindEntry( address );
    if ( GUCEF_NULL == entry )
    {
        MakeRoom();
        if ( m_entries.size() >= MaxEntries )
            return false;
        CMailAddressEntry created;
        created.address = Normalize( address );
        created.firstSeen = now;
        m_entries.push_back( created );
        entry = &m_entries.back();
    }
    entry->publicKey = publicKeyHex.Lowercase();
    if ( !displayName.IsNULLOrEmpty() )
        entry->displayName = displayName.Length() > 64 ? CORE::CString::Empty : displayName;
    entry->lastSeen = now;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailAddressBook::SetBlocked( const CORE::CString& address, bool blocked, Int64 now )
{GUCEF_TRACE;

    CMailAddressEntry* entry = FindEntry( address );
    if ( GUCEF_NULL == entry )
    {
        if ( !blocked || !IsUsableAddress( address ) )
            return;
        MakeRoom();
        if ( m_entries.size() >= MaxEntries )
            return;
        CMailAddressEntry created;
        created.address = Normalize( address );
        created.firstSeen = now;
        created.lastSeen = now;
        created.blocked = true;
        m_entries.push_back( created );
        return;
    }
    entry->blocked = blocked;
    if ( !blocked && entry->publicKey.IsNULLOrEmpty() )
    {
        const CORE::CString wanted = Normalize( address );
        for ( size_t i=0; i<m_entries.size(); ++i )
            if ( m_entries[ i ].address == wanted )
            {
                m_entries.erase( m_entries.begin() + i );
                break;
            }
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::IsBlocked( const CORE::CString& address ) const
{GUCEF_TRACE;

    const CMailAddressEntry* entry = FindEntry( address );
    return GUCEF_NULL != entry && entry->blocked;
}

/*-------------------------------------------------------------------------*/

void
CMailAddressBook::Forget( const CORE::CString& address )
{GUCEF_TRACE;

    const CORE::CString wanted = Normalize( address );
    for ( size_t i=0; i<m_entries.size(); ++i )
        if ( m_entries[ i ].address == wanted )
        {
            if ( m_entries[ i ].blocked )
            {
                m_entries[ i ].publicKey.Clear();
                m_entries[ i ].displayName.Clear();
            }
            else
                m_entries.erase( m_entries.begin() + i );
            return;
        }
}

/*-------------------------------------------------------------------------*/

UInt32
CMailAddressBook::GetCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_entries.size();
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::SaveToFile( const CORE::CString& path ) const
{GUCEF_TRACE;

    return CMailFiles::WriteRecord( path, *this );
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::LoadFromFile( const CORE::CString& path )
{GUCEF_TRACE;

    CMailAddressBook loaded;
    if ( !CMailFiles::ReadRecord( path, loaded, MaxFileBytes ) )
        return false;
    m_entries = loaded.m_entries;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::Serialize( CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings ) const
{GUCEF_TRACE;

    CORE::CDataNode* entries = domRootNode.AddChild( "entries", GUCEF_DATATYPE_ARRAY );
    for ( size_t i=0; i<m_entries.size(); ++i )
    {
        CORE::CDataNode* node = entries->AddChild( "entry", GUCEF_DATATYPE_OBJECT );
        node->SetAttribute( "address", m_entries[ i ].address );
        node->SetAttribute( "publicKey", m_entries[ i ].publicKey );
        node->SetAttribute( "displayName", m_entries[ i ].displayName );
        node->SetAttribute( "firstSeen", m_entries[ i ].firstSeen );
        node->SetAttribute( "lastSeen", m_entries[ i ].lastSeen );
        node->SetAttribute( "blocked", m_entries[ i ].blocked );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailAddressBook::Deserialize( const CORE::CDataNode& domRootNode, const CORE::CDataNodeSerializableSettings& settings )
{GUCEF_TRACE;

    const CORE::CDataNode* entries = domRootNode.FindChild( "entries" );
    if ( GUCEF_NULL == entries )
        return false;
    m_entries.clear();
    CORE::CDataNode::const_iterator i = entries->ConstBegin();
    while ( i != entries->ConstEnd() )
    {
        if ( m_entries.size() >= MaxEntries )
            return false;
        CMailAddressEntry entry;
        entry.address = (*i)->GetAttributeValueOrChildValueByName( "address" ).AsString( CORE::CString::Empty, true );
        entry.publicKey = (*i)->GetAttributeValueOrChildValueByName( "publicKey" ).AsString( CORE::CString::Empty, true );
        entry.displayName = (*i)->GetAttributeValueOrChildValueByName( "displayName" ).AsString( CORE::CString::Empty, true );
        entry.firstSeen = (*i)->GetAttributeValueOrChildValueByName( "firstSeen" ).AsInt64( 0, true );
        entry.lastSeen = (*i)->GetAttributeValueOrChildValueByName( "lastSeen" ).AsInt64( 0, true );
        entry.blocked = (*i)->GetAttributeValueOrChildValueByName( "blocked" ).AsBool( false, true );
        // what is stored is not trusted either
        if ( !IsUsableAddress( entry.address ) || ( !entry.publicKey.IsNULLOrEmpty() && !IsUsableKey( entry.publicKey ) ) || entry.displayName.Length() > 64 )
            return false;
        entry.address = Normalize( entry.address );
        entry.publicKey = entry.publicKey.Lowercase();
        m_entries.push_back( entry );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailAddressBook::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailAddressBook( *this );
}

const CORE::CString&
CMailAddressBook::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailAddressBook";
    return classTypeName;
}

const char*
CMailAddressBook::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

const char*
CMailAddressBook::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailAddressBook";
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
