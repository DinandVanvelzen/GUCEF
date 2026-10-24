/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#ifndef GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#define GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDYNAMICBUFFER_H
#include "CDynamicBuffer.h"
#define GUCEF_CORE_CDYNAMICBUFFER_H
#endif /* GUCEF_CORE_CDYNAMICBUFFER_H ? */

#ifndef GUCEF_COMCORE_CUDPSOCKET_H
#include "CUDPSocket.h"
#define GUCEF_COMCORE_CUDPSOCKET_H
#endif /* GUCEF_COMCORE_CUDPSOCKET_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#include "gucefCOMCORE_CDiscoveryClientBase.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTBASE_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#include "gucefCOMCORE_CDiscoveryClientFactory.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H ? */

#ifndef GUCEF_COM_MACROS_H
#include "gucefCOM_macros.h"      /* often used gucefCOM macros */
#define GUCEF_COM_MACROS_H
#endif /* GUCEF_COM_MACROS_H ? */

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CLASSES                                                            //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  The discovery protocol of the broadcast and multicast clients, see CUdpDiscoveryCodec for the beacon.
 *  A browse sends a query, which every advertiser of a matching entry answers with an announcement. Advertisers
 *  announce on advertising and every TTL/3 after that, and say goodbye when an entry is withdrawn. Answers go to
 *  the same destinations as everything else, so every browser on the network hears them.
 *
 *  Settings (LoadConfig): port, extraTargets (a comma separated list of host:port to send to as well, for peers
 *  the broadcast or multicast does not reach) plus those of the derived class.
 */
class GUCEF_COM_PUBLIC_CPP CUdpDiscoveryClientBase : public COMCORE::CDiscoveryClientBase
{
    public:

    static const UInt16 DefaultPort;

    virtual ~CUdpDiscoveryClientBase();

    void SetPort( UInt16 port );

    UInt16 GetPort( void ) const;

    /** Adds a host:port to send everything to as well, next to the broadcast or multicast destinations */
    void AddExtraTarget( const COMCORE::CIPv4Address& target );

    void ClearExtraTargets( void );

    /** Whether the socket is open */
    bool IsActive( void ) const;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    /**
     *  Joins the multicast group on every network interface that has an address, one by one, so one interface
     *  that refuses does not keep the group from the others. Falls back to letting the system pick the interface.
     *
     *  @return the number of successful joins
     */
    static UInt32 JoinOnAllInterfaces( COMCORE::CUDPSocket& socket, const COMCORE::CIPv4Address& group );

    protected:

    typedef COMCORE::CIPv4Address::TIPv4AddressVector TIPv4AddressVector;

    CUdpDiscoveryClientBase( const CORE::CString& defaultClientName      ,
                             const CORE::PulseGeneratorPtr& pulseGenerator );

    /** Prepares and opens the socket, joining whatever has to be joined */
    virtual bool OpenSocket( COMCORE::CUDPSocket& socket, UInt16 port ) = 0;

    /** The addresses everything is sent to, next to the extra targets. Asked for each time the socket opens */
    virtual void GetDestinations( TIPv4AddressVector& destinations ) const = 0;

    virtual bool OnBrowseStarted( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnAdvertised( const COMCORE::CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnWithdrawn( const COMCORE::CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHousekeeping( UInt64 nowInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    /** Closes the socket after withdrawing everything. Derived classes call it from their destructor */
    void Shutdown( void );

    private:

    typedef GUCEF::map< CORE::CString, UInt64 > TAnnounceTimeMap;

    CUdpDiscoveryClientBase( const CUdpDiscoveryClientBase& src );
    CUdpDiscoveryClientBase& operator=( const CUdpDiscoveryClientBase& src );

    bool EnsureSocketIsOpen( void );
    bool SendBeacon( const CORE::CDynamicBuffer& beacon );
    bool Announce( const COMCORE::CIDiscoveryEntry& entry, UInt64 nowInMs );
    bool OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet );

    private:

    COMCORE::CUDPSocket m_socket;
    UInt16 m_port;
    TIPv4AddressVector m_extraTargets;
    TIPv4AddressVector m_destinations;
    CORE::CDynamicBuffer m_sendBuffer;
    TAnnounceTimeMap m_nextAnnounceTimes;
    bool m_sendErrorReported;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H ? */
