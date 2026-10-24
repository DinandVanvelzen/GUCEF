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

#ifndef GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#define GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#include "gucefCOMCORE_CDiscoveryClientFactory.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#include "gucefCOMCORE_CDiscoveryEntry.h"
#define GUCEF_COMCORE_CDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */

#ifndef GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#include "gucefMULTIPLAY_CIGameListingProvider.h"
#define GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CIGAMELISTINGPROVIDER_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The discovery clients of a LAN listing provider or a game advertiser, as config gives them */
class GUCEF_MULTIPLAY_EXPORT_CPP CDiscoveryClientList
{
    public:

    typedef GUCEF::vector< COMCORE::CIDiscoveryClientPtr > TDiscoveryClientVector;

    /** The client types used when the config names none: broadcast, multicast and mdns */
    static void GetDefaultClientTypes( CORE::CString::StringVector& types );

    /**
     *  Makes the clients the config asks for: one per "DiscoveryClient" child (attribute "type", the rest is the
     *  config of the client), else one per type in the comma separated attribute "discoveryClients", else the
     *  default types. Types the factory does not know are skipped with a warning.
     */
    static bool CreateFromConfig( const CORE::CDataNode& config, TDiscoveryClientVector& clients );

    static bool CreateFromTypes( const CORE::CString::StringVector& types, TDiscoveryClientVector& clients );
};

/*-------------------------------------------------------------------------*/

/**
 *  Lists the games and lobbies on the LAN: browses a service type with COMCORE discovery clients.
 *
 *  The attributes of a discovery entry carry the listing: kind ("lobby" / "game"), game, ver, map, mode, pl, max,
 *  spec, pw ("1" / "0"), id and host (the host name); the other attributes become the extra values of the listing.
 *  The listing id is the attribute "id", else the key of the entry. A listing found by several clients is one
 *  listing, removed when the last client loses it.
 *
 *  Config: serviceType (default "_gucef-game._udp"), discoveryClients (default "broadcast,multicast,mdns") or
 *  "DiscoveryClient" children, see CDiscoveryClientList
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CLanListingProvider : public CIGameListingProvider
{
    public:

    static const CORE::CString ProviderName;            /**< "lan" */
    static const CORE::CString DefaultServiceType;      /**< "_gucef-game._udp" */

    typedef CDiscoveryClientList::TDiscoveryClientVector TDiscoveryClientVector;

    CLanListingProvider( void );

    virtual ~CLanListingProvider();

    virtual bool Start( const CGameListingFilter& filter ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void Stop( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool Refresh( void ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool IsActive( void ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetProviderName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** Uses the client in addition to the configured ones, also when the provider runs already */
    bool AddDiscoveryClient( COMCORE::CIDiscoveryClientPtr client );

    const TDiscoveryClientVector& GetDiscoveryClients( void ) const;

    void SetServiceType( const CORE::CString& serviceType );

    const CORE::CString& GetServiceType( void ) const;

    /** Reads a listing from the entry, false when the entry has no name and no id */
    static bool EntryToListing( const COMCORE::CIDiscoveryEntry& entry, CGameListingEntry& listing );

    /** Writes the listing into the entry */
    static void ListingToEntry( const CGameListingEntry& listing, const CORE::CString& serviceType, COMCORE::CDiscoveryEntry& entry );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    typedef GUCEF::set< COMCORE::CIDiscoveryClient* > TClientSet;
    typedef GUCEF::map< CORE::CString, TClientSet > TListingSourceMap;

    CLanListingProvider( const CLanListingProvider& src );
    CLanListingProvider& operator=( const CLanListingProvider& src );

    bool StartClient( COMCORE::CIDiscoveryClient& client );
    bool OnEntryFound( COMCORE::CIDiscoveryClient* client, const COMCORE::CIDiscoveryEntry& entry );
    bool OnEntryLost( COMCORE::CIDiscoveryClient* client, const COMCORE::CIDiscoveryEntry& entry );
    bool EnsureClients( void );

    private:

    CORE::CString m_serviceType;
    TDiscoveryClientVector m_clients;
    CORE::CDataNode m_clientConfig;
    bool m_clientsCreated;
    bool m_active;
    TListingSourceMap m_sources;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< CIGameListingProvider, CLanListingProvider, MT::CMutex > TLanListingProviderFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H ? */
