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
    , m_discovery()
    , m_services()
    , m_advertising( false )
    , m_listing()
{GUCEF_TRACE;

    COMCORE::CServiceAdvertiserSettings settings;
    settings.serviceType = CLanListingProvider::DefaultServiceType;
    settings.ttlInSecs = 30;
    settings.defaultClientTypes.clear();
    CDiscoveryClientList::GetDefaultClientTypes( settings.defaultClientTypes );
    m_discovery.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

CGameAdvertiser::~CGameAdvertiser()
{GUCEF_TRACE;

    Withdraw();
}

/*-------------------------------------------------------------------------*/

void
CGameAdvertiser::MakeAnnouncement( COMCORE::CServiceAnnouncement& announcement ) const
{GUCEF_TRACE;

    COMCORE::CDiscoveryEntry entry;
    CLanListingProvider::ListingToEntry( m_listing, m_discovery.GetSettings().serviceType, entry );
    announcement.FromEntry( entry );
    announcement.ttlInSecs = m_discovery.GetSettings().ttlInSecs;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::Publish( bool first )
{GUCEF_TRACE;

    bool anyPublished = false;
    COMCORE::CServiceAnnouncement announcement;
    MakeAnnouncement( announcement );
    if ( first )
    {
        // Clients or services given by hand replace the default clients, the configured clients are always made
        m_discovery.SetCreateDefaultClients( m_services.empty() );
        bool useDiscovery = m_services.empty() || !m_discovery.GetDiscoveryClients().empty() ||
                            COMCORE::CDiscoveryClientList::NamesClients( m_discovery.GetSettings().clientConfig );
        if ( useDiscovery && m_discovery.Advertise( announcement ) )
            anyPublished = true;
    }
    else
    if ( m_discovery.IsAdvertising() && m_discovery.Update( announcement ) )
    {
        anyPublished = true;
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

    m_listing = listing;
    if ( m_listing.listingId.IsNULLOrEmpty() )
        m_listing.listingId = CPlayerIdentity::MakeGuid();

    m_advertising = Publish( true );
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
    return Publish( false );
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
    return Publish( false );
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::Withdraw( void )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;
    m_advertising = false;
    m_discovery.Withdraw();
    for ( size_t i=0; i<m_services.size(); ++i )
        m_services[ i ]->WithdrawListing( m_listing.listingId );
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

    return m_discovery.AddDiscoveryClient( client );
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

    return m_discovery.GetDiscoveryClients();
}

/*-------------------------------------------------------------------------*/

void
CGameAdvertiser::SetServiceType( const CORE::CString& serviceType )
{GUCEF_TRACE;

    COMCORE::CServiceAdvertiserSettings settings( m_discovery.GetSettings() );
    settings.serviceType = serviceType;
    m_discovery.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

void
CGameAdvertiser::SetTtlInSecs( UInt32 ttlInSecs )
{GUCEF_TRACE;

    COMCORE::CServiceAdvertiserSettings settings( m_discovery.GetSettings() );
    settings.ttlInSecs = ttlInSecs;
    m_discovery.SetSettings( settings );
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    const COMCORE::CServiceAdvertiserSettings& settings = m_discovery.GetSettings();
    config.SetAttribute( "serviceType", settings.serviceType );
    config.SetAttribute( "ttlInSecs", settings.ttlInSecs );
    COMCORE::CDiscoveryClientList::CopyClientConfig( settings.clientConfig, config );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CGameAdvertiser::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    COMCORE::CServiceAdvertiserSettings settings( m_discovery.GetSettings() );
    settings.serviceType = config.GetAttributeValueOrChildValueByName( "serviceType" ).AsString( settings.serviceType, true );
    settings.ttlInSecs = config.GetAttributeValueOrChildValueByName( "ttlInSecs" ).AsUInt32( settings.ttlInSecs, true );
    settings.clientConfig.Clear();
    COMCORE::CDiscoveryClientList::CopyClientConfig( config, settings.clientConfig );
    m_discovery.SetSettings( settings );
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
