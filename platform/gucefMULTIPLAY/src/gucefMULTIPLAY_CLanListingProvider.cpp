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

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefMULTIPLAY_CLanListingProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CLanListingProvider::ProviderName = "lan";
const CORE::CString CLanListingProvider::DefaultServiceType = "_gucef-game._udp";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The attributes that carry the fields of a listing, the rest are extra */
const char* ListingAttributes[] = { "kind", "game", "ver", "map", "mode", "pl", "max", "spec", "pw", "id", "host" };
const UInt32 ListingAttributeCount = sizeof( ListingAttributes ) / sizeof( ListingAttributes[ 0 ] );

bool
IsListingAttribute( const CORE::CString& key )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<ListingAttributeCount; ++i )
        if ( key == ListingAttributes[ i ] )
            return true;
    return false;
}

CORE::CString
GetAttribute( const CORE::CValueList& attributes, const char* key )
{GUCEF_TRACE;

    return attributes.GetValueAlways( key, CORE::CVariant::Empty ).AsString( CORE::CString::Empty, true );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CDiscoveryClientList::GetDefaultClientTypes( CORE::CString::StringVector& types )
{GUCEF_TRACE;

    types.push_back( "broadcast" );
    types.push_back( "multicast" );
    types.push_back( "mdns" );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientList::CreateFromTypes( const CORE::CString::StringVector& types, TDiscoveryClientVector& clients )
{GUCEF_TRACE;

    COMCORE::CDiscoveryClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryClientFactory();
    bool allMade = true;
    for ( size_t i=0; i<types.size(); ++i )
    {
        COMCORE::CIDiscoveryClientPtr client = factory.Create( types[ i ] );
        if ( client.IsNULL() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClientList: There is no discovery client of type \"" << types[ i ] << "\"" );
            allMade = false;
            continue;
        }
        clients.push_back( client );
    }
    return allMade;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientList::CreateFromConfig( const CORE::CDataNode& config, TDiscoveryClientVector& clients )
{GUCEF_TRACE;

    COMCORE::CDiscoveryClientFactory& factory = COMCORE::CComCoreGlobal::Instance()->GetDiscoveryClientFactory();
    bool allMade = true;
    bool anyChild = false;
    CORE::CDataNode::const_iterator i = config.ConstBegin();
    while ( i != config.ConstEnd() )
    {
        const CORE::CDataNode& child = *(*i);
        ++i;
        if ( "DiscoveryClient" != child.GetName() )
            continue;

        anyChild = true;
        CORE::CString type = child.GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true );
        COMCORE::CIDiscoveryClientPtr client = factory.Create( type );
        if ( client.IsNULL() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClientList: There is no discovery client of type \"" << type << "\"" );
            allMade = false;
            continue;
        }
        if ( !client->LoadConfig( child ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClientList: The config of the discovery client of type \"" << type << "\" has problems" );
            allMade = false;
        }
        clients.push_back( client );
    }
    if ( anyChild )
        return allMade;

    CORE::CString::StringVector types;
    CORE::CString typeList = config.GetAttributeValueOrChildValueByName( "discoveryClients" ).AsString( CORE::CString::Empty, true );
    if ( typeList.IsNULLOrEmpty() )
    {
        GetDefaultClientTypes( types );
    }
    else
    {
        CORE::CString::StringVector parts = typeList.ParseElements( ',', false );
        for ( size_t n=0; n<parts.size(); ++n )
        {
            CORE::CString type = parts[ n ].Trim( true ).Trim( false );
            if ( !type.IsNULLOrEmpty() )
                types.push_back( type );
        }
    }
    return CreateFromTypes( types, clients );
}

/*-------------------------------------------------------------------------*/

CLanListingProvider::CLanListingProvider( void )
    : CIGameListingProvider()
    , m_serviceType( DefaultServiceType )
    , m_clients()
    , m_clientConfig()
    , m_clientsCreated( false )
    , m_active( false )
    , m_sources()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CLanListingProvider::~CLanListingProvider()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    for ( size_t i=0; i<m_clients.size(); ++i )
        m_clients[ i ]->StopBrowse( m_serviceType );
    m_clients.clear();
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::EnsureClients( void )
{GUCEF_TRACE;

    if ( m_clientsCreated )
        return true;
    m_clientsCreated = true;

    // Clients added by hand replace the default ones, the configured ones are always made
    bool configured = false;
    CORE::CDataNode::const_iterator i = m_clientConfig.ConstBegin();
    while ( i != m_clientConfig.ConstEnd() && !configured )
    {
        configured = "DiscoveryClient" == (*i)->GetName();
        ++i;
    }
    configured = configured || !m_clientConfig.GetAttributeValueOrChildValueByName( "discoveryClients" ).AsString( CORE::CString::Empty, true ).IsNULLOrEmpty();
    if ( !configured && !m_clients.empty() )
        return true;

    return CDiscoveryClientList::CreateFromConfig( m_clientConfig, m_clients );
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::StartClient( COMCORE::CIDiscoveryClient& client )
{GUCEF_TRACE;

    SubscribeTo( &client );
    if ( !client.Browse( m_serviceType ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "LanListingProvider: The discovery client " << client.GetDiscoveryClientName() << " cannot browse " << m_serviceType );
        return false;
    }

    // What the client knows already
    COMCORE::CIDiscovery::TIDiscoveryEntrySet entries;
    client.GetDiscoveredEntries( m_serviceType, entries );
    COMCORE::CIDiscovery::TIDiscoveryEntrySet::iterator i = entries.begin();
    while ( i != entries.end() )
    {
        if ( !(*i).IsNULL() )
            OnEntryFound( &client, *(*i) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::Start( const CGameListingFilter& filter )
{GUCEF_TRACE;

    if ( m_active )
        Stop();

    SetFilter( filter );
    EnsureClients();

    bool anyStarted = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        anyStarted = StartClient( *m_clients[ i ] ) || anyStarted;

    m_active = anyStarted;
    if ( !anyStarted )
        ReportProviderError( "No discovery client could browse the LAN" );
    return anyStarted;
}

/*-------------------------------------------------------------------------*/

void
CLanListingProvider::Stop( void )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_clients.size(); ++i )
    {
        UnsubscribeFrom( m_clients[ i ].GetPointerAlways() );
        m_clients[ i ]->StopBrowse( m_serviceType );
    }
    m_active = false;
    m_sources.clear();
    RemoveAllListings();
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::Refresh( void )
{GUCEF_TRACE;

    if ( !m_active )
        return false;

    // Browsing again asks the LAN again
    bool anyStarted = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
    {
        m_clients[ i ]->StopBrowse( m_serviceType );
        anyStarted = m_clients[ i ]->Browse( m_serviceType ) || anyStarted;
    }
    return anyStarted;
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLanListingProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::AddDiscoveryClient( COMCORE::CIDiscoveryClientPtr client )
{GUCEF_TRACE;

    if ( client.IsNULL() )
        return false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        if ( m_clients[ i ] == client )
            return true;
    m_clients.push_back( client );
    if ( m_active )
        return StartClient( *client );
    return true;
}

/*-------------------------------------------------------------------------*/

const CLanListingProvider::TDiscoveryClientVector&
CLanListingProvider::GetDiscoveryClients( void ) const
{GUCEF_TRACE;

    return m_clients;
}

/*-------------------------------------------------------------------------*/

void
CLanListingProvider::SetServiceType( const CORE::CString& serviceType )
{GUCEF_TRACE;

    m_serviceType = serviceType;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLanListingProvider::GetServiceType( void ) const
{GUCEF_TRACE;

    return m_serviceType;
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::EntryToListing( const COMCORE::CIDiscoveryEntry& entry, CGameListingEntry& listing )
{GUCEF_TRACE;

    const CORE::CValueList& attributes = entry.GetAttributes();

    listing.listingId = GetAttribute( attributes, "id" );
    if ( listing.listingId.IsNULLOrEmpty() )
        listing.listingId = entry.GetEntryKey();
    listing.name = entry.GetInstanceName();
    if ( listing.listingId.IsNULLOrEmpty() && listing.name.IsNULLOrEmpty() )
        return false;

    listing.kind = CGameListingEntry::StringToKind( GetAttribute( attributes, "kind" ) );
    listing.gameId = GetAttribute( attributes, "game" );
    listing.gameVersion = GetAttribute( attributes, "ver" );
    listing.mapName = GetAttribute( attributes, "map" );
    listing.modeName = GetAttribute( attributes, "mode" );
    listing.players = CORE::StringToUInt32( GetAttribute( attributes, "pl" ), 0 );
    listing.maxPlayers = CORE::StringToUInt32( GetAttribute( attributes, "max" ), 0 );
    listing.spectators = CORE::StringToUInt32( GetAttribute( attributes, "spec" ), 0 );
    listing.passworded = CORE::StringToBool( GetAttribute( attributes, "pw" ), false );
    listing.hostName = GetAttribute( attributes, "host" );
    listing.lastSeen = entry.GetLastSeen();

    listing.endpoints = entry.GetEndpoints();
    if ( listing.endpoints.empty() )
    {
        COMCORE::CHostAddress host;
        if ( entry.GetHost( host ) && 0 != host.GetPortInHostByteOrder() )
            listing.endpoints.push_back( CGameListingEntry::MakeEndpoint( "udp", host.GetFirstAddressAsString(), host.GetPortInHostByteOrder() ) );
    }
    if ( listing.hostName.IsNULLOrEmpty() )
    {
        COMCORE::CHostAddress host;
        if ( entry.GetHost( host ) )
            listing.hostName = host.GetHostname().IsNULLOrEmpty() ? host.GetFirstAddressAsString() : host.GetHostname();
    }

    listing.extra.DeleteAll();
    listing.extra.SetAllowMultipleValues( false );
    UInt32 keyCount = attributes.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        CORE::CString key = attributes.GetKey( i ).AsString();
        if ( !IsListingAttribute( key ) )
            listing.extra.Set( key, attributes.GetValue( i ).AsString() );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CLanListingProvider::ListingToEntry( const CGameListingEntry& listing, const CORE::CString& serviceType, COMCORE::CDiscoveryEntry& entry )
{GUCEF_TRACE;

    entry.SetServiceType( serviceType );
    entry.SetInstanceName( listing.name );
    if ( !listing.listingId.IsNULLOrEmpty() )
        entry.SetApplicationInstanceId( listing.listingId );
    entry.SetEndpoints( listing.endpoints );

    CORE::CValueList& attributes = entry.GetAttributes();
    attributes.DeleteAll();
    attributes.SetAllowMultipleValues( false );

    // The extra values first, so the listing fields win
    UInt32 keyCount = listing.extra.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
        attributes.Set( listing.extra.GetKey( i ).AsString(), listing.extra.GetValue( i ).AsString() );

    attributes.Set( "id", listing.listingId );
    attributes.Set( "kind", CORE::CString( CGameListingEntry::KindToString( listing.kind ) ) );
    attributes.Set( "game", listing.gameId );
    attributes.Set( "ver", listing.gameVersion );
    attributes.Set( "map", listing.mapName );
    attributes.Set( "mode", listing.modeName );
    attributes.Set( "pl", CORE::ToString( listing.players ) );
    attributes.Set( "max", CORE::ToString( listing.maxPlayers ) );
    attributes.Set( "spec", CORE::ToString( listing.spectators ) );
    attributes.Set( "pw", CORE::CString( listing.passworded ? "1" : "0" ) );
    if ( !listing.hostName.IsNULLOrEmpty() )
        attributes.Set( "host", listing.hostName );
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::OnEntryFound( COMCORE::CIDiscoveryClient* client, const COMCORE::CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    if ( entry.GetServiceType() != m_serviceType )
        return true;

    CGameListingEntry listing;
    if ( !EntryToListing( entry, listing ) )
        return true;
    listing.backendName = ProviderName;

    m_sources[ listing.listingId ].insert( client );
    return UpsertListing( listing );
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::OnEntryLost( COMCORE::CIDiscoveryClient* client, const COMCORE::CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    if ( entry.GetServiceType() != m_serviceType )
        return true;

    CGameListingEntry listing;
    if ( !EntryToListing( entry, listing ) )
        return true;

    TListingSourceMap::iterator i = m_sources.find( listing.listingId );
    if ( i != m_sources.end() )
    {
        (*i).second.erase( client );
        if ( !(*i).second.empty() )
            return true;
        m_sources.erase( i );
    }
    return RemoveListing( listing.listingId );
}

/*-------------------------------------------------------------------------*/

void
CLanListingProvider::OnNotify( CORE::CNotifier* notifier                 ,
                               const CORE::CEvent& eventId               ,
                               CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    COMCORE::CIDiscoveryClient* client = GUCEF_NULL;
    for ( size_t i=0; i<m_clients.size() && GUCEF_NULL == client; ++i )
        if ( static_cast< CORE::CNotifier* >( m_clients[ i ].GetPointerAlways() ) == notifier )
            client = m_clients[ i ].GetPointerAlways();
    if ( GUCEF_NULL == client )
        return;

    if ( COMCORE::CIDiscoveryClient::EntryDiscoveredEvent == eventId || COMCORE::CIDiscoveryClient::EntryUpdatedEvent == eventId )
    {
        COMCORE::CDiscoveryEntryEventData* entry = static_cast< COMCORE::CDiscoveryEntryEventData* >( eventData );
        if ( GUCEF_NULL != entry && !entry->IsNULL() )
            OnEntryFound( client, *(*entry) );
    }
    else
    if ( COMCORE::CIDiscoveryClient::EntryLostEvent == eventId )
    {
        COMCORE::CDiscoveryEntryEventData* entry = static_cast< COMCORE::CDiscoveryEntryEventData* >( eventData );
        if ( GUCEF_NULL != entry && !entry->IsNULL() )
            OnEntryLost( client, *(*entry) );
    }
    else
    if ( COMCORE::CIDiscoveryClient::DiscoveryErrorEvent == eventId )
    {
        const COMCORE::CIDiscoveryClient::TDiscoveryErrorEventData* error = static_cast< const COMCORE::CIDiscoveryClient::TDiscoveryErrorEventData* >( eventData );
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "LanListingProvider: Discovery client " << client->GetDiscoveryClientName() << " reports: "
            << ( GUCEF_NULL != error ? error->GetData() : CORE::CString::Empty ) );
    }
}

/*-------------------------------------------------------------------------*/

bool
CLanListingProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "serviceType", m_serviceType );
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
CLanListingProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_serviceType = config.GetAttributeValueOrChildValueByName( "serviceType" ).AsString( m_serviceType, true );
    m_clientConfig = config;
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CLanListingProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CLanListingProvider";
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
