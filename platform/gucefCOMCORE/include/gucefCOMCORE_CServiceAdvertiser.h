/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_COMCORE_CSERVICEADVERTISER_H
#define GUCEF_COMCORE_CSERVICEADVERTISER_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#include "gucefCOMCORE_CDiscoveryClientList.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTLIST_H ? */

#ifndef GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#include "gucefCOMCORE_CServiceAnnouncement.h"
#define GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H
#endif /* GUCEF_COMCORE_CSERVICEANNOUNCEMENT_H ? */

#ifndef GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H
#include "gucefCOMCORE_CServiceAdvertiserSettings.h"
#define GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H
#endif /* GUCEF_COMCORE_CSERVICEADVERTISERSETTINGS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  Tells the local network that a service is here, so clients can find it without knowing its address: answers the
 *  queries of finders (CServiceFinder) and announces itself every TTL/3 through the discovery clients its settings
 *  name. Discovery only adds candidates: the configured address of a service (a DNS name or an IP and a port) stays the
 *  way that always works, and what a finder hears is never trusted for that alone.
 *
 *  The clients are made from the settings at the first Advertise() (CDiscoveryClientList). Clients added by hand
 *  (AddDiscoveryClient()) replace the default types, but not clients the config names. With a pulse generator given
 *  at construction the clients the advertiser makes run on it (CIDiscoveryClient::SetPulseGenerator()), else on the
 *  global one. All calls on the thread of that pulse generator.
 */
class GUCEF_COMCORE_EXPORT_CPP CServiceAdvertiser : public CORE::CIConfigurable
{
    public:

    typedef CDiscoveryClientList::TDiscoveryClientVector TDiscoveryClientVector;

    CServiceAdvertiser( void );
    explicit CServiceAdvertiser( const CORE::PulseGeneratorPtr& pulseGenerator );
    explicit CServiceAdvertiser( const CServiceAdvertiserSettings& settings );
    CServiceAdvertiser( const CORE::PulseGeneratorPtr& pulseGenerator, const CServiceAdvertiserSettings& settings );
    virtual ~CServiceAdvertiser();

    /**
     *  Starts advertising. Empty fields of the announcement are filled from the settings (service type, instance id,
     *  display name, TTL); an announcement still without an instance id gets a new random one, kept for later calls.
     *
     *  @return false when the announcement is not valid (CServiceAnnouncement::IsValid(), logged) or no client could
     *          advertise it
     */
    bool Advertise( const CServiceAnnouncement& announcement );

    /** Publishes the changed announcement; the instance id stays the one it was advertised with. Nothing is sent when nothing changed */
    bool Update( const CServiceAnnouncement& announcement );

    /** Announces again now, for example after a network change */
    bool Refresh( void );

    /** Takes the announcement away: a goodbye where the protocol has one, else browsers lose it when its TTL runs out */
    bool Withdraw( void );

    bool IsAdvertising( void ) const;

    /** The announcement as it is advertised, with the filled in fields */
    const CServiceAnnouncement& GetAnnouncement( void ) const;

    /** Publishes through the client too, also while advertising already */
    bool AddDiscoveryClient( CIDiscoveryClientPtr client );
    const TDiscoveryClientVector& GetDiscoveryClients( void ) const;

    /** Whether the default client types are made when neither the config nor AddDiscoveryClient() gave clients; default true */
    void SetCreateDefaultClients( bool createDefaultClients );

    void SetSettings( const CServiceAdvertiserSettings& settings );
    const CServiceAdvertiserSettings& GetSettings( void ) const;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;
    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;
    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    /** A random instance id: 32 hexadecimal characters */
    static CORE::CString MakeInstanceId( void );

    private:

    CServiceAdvertiser( const CServiceAdvertiser& src );
    CServiceAdvertiser& operator=( const CServiceAdvertiser& src );

    bool EnsureClients( void );
    bool PrepareClient( CIDiscoveryClient& client ) const;
    void Complete( CServiceAnnouncement& announcement ) const;
    bool Publish( void );

    private:

    CORE::PulseGeneratorPtr m_pulseGenerator;
    CServiceAdvertiserSettings m_settings;
    TDiscoveryClientVector m_clients;
    bool m_clientsCreated;
    bool m_createDefaultClients;
    bool m_advertising;
    CORE::CString m_instanceId;
    CServiceAnnouncement m_announcement;
    CIDiscoveryEntryPtr m_entry;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COMCORE_CSERVICEADVERTISER_H ? */
