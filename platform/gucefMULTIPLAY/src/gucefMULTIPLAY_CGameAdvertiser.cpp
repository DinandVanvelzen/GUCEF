/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
 *
 *  Copyright (C) 1998 - 2020.  Dinand Vanvelzen
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

#ifndef GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#include "gucefMULTIPLAY_CPlayerIdentity.h"
#define GUCEF_MULTIPLAY_CPLAYERIDENTITY_H
#endif /* GUCEF_MULTIPLAY_CPLAYERIDENTITY_H ? */

#include "gucefMULTIPLAY_CGameAdvertiser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CGameAdvertiser::CGameAdvertiser( void )
    : CORE::CIConfigurable()
    , m_serviceType( CLanListingProvider::DefaultServiceType )
    , m_ttlInSecs( 30 )
    , m_clients()
    , m_services()
    , m_clientConfig()
    , m_clientsCreated( false )
    , m_advertising( false )
    , m_listing()
    , m_entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CGameAdvertiser::~CGameAdvertiser()
{GUCEF_TRACE;

    Withdraw();
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::EnsureClients( void )
{GUCEF_TRACE;

    if ( m_clientsCreated )
        return true;
    m_clientsCreated = true;

    bool configured = !m_clientConfig.GetAttributeValueOrChildValueByName( "discoveryClients" ).AsString( CORE::CString::Empty, true ).IsNULLOrEmpty();
    CORE::CDataNode::const_iterator i = m_clientConfig.ConstBegin();
    while ( i != m_clientConfig.ConstEnd() && !configured )
    {
        configured = "DiscoveryClient" == (*i)->GetName();
        ++i;
    }

    // Clients or services given by hand replace the default clients
    if ( !configured && ( !m_clients.empty() || !m_services.empty() ) )
        return true;
    return CDiscoveryClientList::CreateFromConfig( m_clientConfig, m_clients );
}

/*-------------------------------------------------------------------------*/

COMCORE::CIDiscoveryEntryPtr
CGameAdvertiser::MakeEntry( void ) const
{GUCEF_TRACE;

    COMCORE::CDiscoveryEntry* entry = GUCEF_NEW COMCORE::CDiscoveryEntry();
    CLanListingProvider::ListingToEntry( m_listing, m_serviceType, *entry );
    entry->SetTtlInSecs( m_ttlInSecs );
    return COMCORE::CIDiscoveryEntryPtr( entry );
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::Publish( void )
{GUCEF_TRACE;

    // A new entry with the same key replaces the advertised one, the clients announce the change
    m_entry = MakeEntry();

    bool anyPublished = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
    {
        if ( m_clients[ i ]->Advertise( m_entry ) )
            anyPublished = true;
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "GameAdvertiser: The discovery client " << m_clients[ i ]->GetDiscoveryClientName() << " cannot advertise the game" );
    }
    for ( size_t i=0; i<m_services.size(); ++i )
    {
        if ( m_services[ i ]->PublishListing( m_listing ) )
            anyPublished = true;
    }
    return anyPublished;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::Advertise( const CGameListingEntry& listing )
{GUCEF_TRACE;

    if ( m_advertising )
        Withdraw();

    EnsureClients();
    m_listing = listing;
    if ( m_listing.listingId.IsNULLOrEmpty() )
        m_listing.listingId = CPlayerIdentity::MakeGuid();

    m_advertising = Publish();
    if ( !m_advertising )
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "GameAdvertiser: Nothing could publish the game " << m_listing.name );
    return m_advertising;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::UpdateListing( const CGameListingEntry& listing )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;

    // The id stays the one it was advertised with
    CGameListingEntry updated( listing );
    updated.listingId = m_listing.listingId;
    if ( updated.HasSameContent( m_listing ) )
        return true;
    m_listing = updated;
    return Publish();
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::UpdatePlayers( UInt32 players, UInt32 spectators )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;
    if ( m_listing.players == players && m_listing.spectators == spectators )
        return true;
    m_listing.players = players;
    m_listing.spectators = spectators;
    return Publish();
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::Withdraw( void )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;
    m_advertising = false;

    for ( size_t i=0; i<m_clients.size(); ++i )
        m_clients[ i ]->Withdraw( m_entry );
    for ( size_t i=0; i<m_services.size(); ++i )
        m_services[ i ]->WithdrawListing( m_listing.listingId );
    m_entry.Unlink();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::IsAdvertising( void ) const
{GUCEF_TRACE;

    return m_advertising;
}

/*-------------------------------------------------------------------------*/

const CGameListingEntry&
CGameAdvertiser::GetListing( void ) const
{GUCEF_TRACE;

    return m_listing;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::AddDiscoveryClient( COMCORE::CIDiscoveryClientPtr client )
{GUCEF_TRACE;

    if ( client.IsNULL() )
        return false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        if ( m_clients[ i ] == client )
            return true;
    m_clients.push_back( client );
    if ( m_advertising && !m_entry.IsNULL() )
        return client->Advertise( m_entry );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::AddLobbyService( CILobbyServicePtr service )
{GUCEF_TRACE;

    if ( service.IsNULL() )
        return false;
    for ( size_t i=0; i<m_services.size(); ++i )
        if ( m_services[ i ] == service )
            return true;
    m_services.push_back( service );
    if ( m_advertising )
        return service->PublishListing( m_listing );
    return true;
}

/*-------------------------------------------------------------------------*/

const CGameAdvertiser::TDiscoveryClientVector&
CGameAdvertiser::GetDiscoveryClients( void ) const
{GUCEF_TRACE;

    return m_clients;
}

/*-------------------------------------------------------------------------*/

void
CGameAdvertiser::SetServiceType( const CORE::CString& serviceType )
{GUCEF_TRACE;

    m_serviceType = serviceType;
}

/*-------------------------------------------------------------------------*/

void
CGameAdvertiser::SetTtlInSecs( UInt32 ttlInSecs )
{GUCEF_TRACE;

    m_ttlInSecs = ttlInSecs;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serviceType", m_serviceType );
    config.SetAttribute( "ttlInSecs", m_ttlInSecs );
    CORE::CDataNode::const_iterator i = m_clientConfig.ConstBegin();
    while ( i != m_clientConfig.ConstEnd() )
    {
        config.AddChild( *(*i) );
        ++i;
    }
    CORE::CString typeList = m_clientConfig.GetAttributeValueOrChildValueByName( "discoveryClients" ).AsString( CORE::CString::Empty, true );
    if ( !typeList.IsNULLOrEmpty() )
        config.SetAttribute( "discoveryClients", typeList );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_serviceType = config.GetAttributeValueOrChildValueByName( "serviceType" ).AsString( m_serviceType, true );
    m_ttlInSecs = config.GetAttributeValueOrChildValueByName( "ttlInSecs" ).AsUInt32( m_ttlInSecs, true );
    m_clientConfig = config;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CGameAdvertiser::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CGameAdvertiser";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
