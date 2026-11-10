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

#ifndef GUCEF_MULTIPLAY_CGAMEADVERTISER_H
#define GUCEF_MULTIPLAY_CGAMEADVERTISER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#include "gucefMULTIPLAY_CLanListingProvider.h"
#define GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H
#endif /* GUCEF_MULTIPLAY_CLANLISTINGPROVIDER_H ? */

#ifndef GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#include "gucefMULTIPLAY_CILobbyService.h"
#define GUCEF_MULTIPLAY_CILOBBYSERVICE_H
#endif /* GUCEF_MULTIPLAY_CILOBBYSERVICE_H ? */

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

/**
 *  Publishes the local lobby or game: on the LAN through COMCORE discovery clients (the entry the `lan` listing
 *  provider reads) and through lobby services that list games hosted elsewhere (CILobbyService::PublishListing).
 *  Changes, such as the player count, are published again; Withdraw() takes the listing away everywhere.
 *
 *  Config: serviceType (default "_gucef-game._udp"), ttlInSecs (default 30), discoveryClients or
 *  "DiscoveryClient" children as for CDiscoveryClientList
 */
class GUCEF_MULTIPLAY_EXPORT_CPP CGameAdvertiser : public CORE::CIConfigurable
{
    public:

    typedef CDiscoveryClientList::TDiscoveryClientVector     TDiscoveryClientVector;
    typedef GUCEF::vector< CILobbyServicePtr >              TLobbyServiceVector;

    CGameAdvertiser( void );

    virtual ~CGameAdvertiser();

    /**
     *  Starts publishing the listing. A listing without an id gets a new one.
     *
     *  @return whether at least one place publishes it
     */
    bool Advertise( const CGameListingEntry& listing );

    /** Publishes the changed listing, its id stays the same */
    bool UpdateListing( const CGameListingEntry& listing );

    bool UpdatePlayers( UInt32 players, UInt32 spectators );

    bool Withdraw( void );

    bool IsAdvertising( void ) const;

    const CGameListingEntry& GetListing( void ) const;

    /** Publishes through the client too, also when it advertises already */
    bool AddDiscoveryClient( COMCORE::CIDiscoveryClientPtr client );

    bool AddLobbyService( CILobbyServicePtr service );

    const TDiscoveryClientVector& GetDiscoveryClients( void ) const;

    void SetServiceType( const CORE::CString& serviceType );

    void SetTtlInSecs( UInt32 ttlInSecs );

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    private:

    CGameAdvertiser( const CGameAdvertiser& src );
    CGameAdvertiser& operator=( const CGameAdvertiser& src );

    bool EnsureClients( void );
    COMCORE::CIDiscoveryEntryPtr MakeEntry( void ) const;
    bool Publish( void );

    private:

    CORE::CString m_serviceType;
    UInt32 m_ttlInSecs;
    TDiscoveryClientVector m_clients;
    TLobbyServiceVector m_services;
    CORE::CDataNode m_clientConfig;
    bool m_clientsCreated;
    bool m_advertising;
    CGameListingEntry m_listing;
    COMCORE::CIDiscoveryEntryPtr m_entry;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MULTIPLAY_CGAMEADVERTISER_H ? */
