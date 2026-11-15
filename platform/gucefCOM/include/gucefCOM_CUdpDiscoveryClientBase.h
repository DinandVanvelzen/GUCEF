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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

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
 *  Advertisers announce on advertising and every TTL/3 after that to the broadcast or multicast destinations, and say
 *  goodbye when an entry is withdrawn.
 *
 *  A browse sends a query from a socket of its own on a port the system picks, padded to queryPaddingBytes. An
 *  advertiser answers a query with its matching announcements by unicast to the address and port the query came from,
 *  so exactly the asker hears the answer (also with several clients on one host), and only:
 *  - for service types it advertises (a query for every type, "*", only when answerWildcardQueries is on),
 *  - from a source that can be a real asker and that still has answers left (CDiscoveryAnswerLimiter: per source
 *    and per process, settings of COMCORE::CDiscoveryClientBase),
 *  - with answers that together are no larger than the query times amplificationFactor (default 1), unless the
 *    source is one of the extraTargets, which the configuration names and so are verified.
 *  A forged query thus cannot make the advertiser send more bytes to a victim than the forger sent, and not more often
 *  than the limits allow. Answers that do not fit are left out; the asker hears them with the next announcement.
 *
 *  Settings (LoadConfig): port, extraTargets (a comma separated list of host:port to send to as well, for peers
 *  the broadcast or multicast does not reach), queryPaddingBytes (default and maximum CUdpDiscoveryCodec::MaxBeaconSizeInBytes),
 *  answerWildcardQueries (default true), the answer limits of COMCORE::CDiscoveryClientBase, plus those of the
 *  derived class.
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

    /** Whether the socket the queries are sent from, and their answers arrive on, is open: only while browsing */
    bool IsQuerySocketActive( void ) const;

    /** The port of the query socket, 0 while it is closed */
    UInt16 GetQuerySocketPort( void ) const;

    /** The size queries are padded to, which is what lets an advertiser answer within the amplification bound */
    void SetQueryPaddingBytes( UInt32 queryPaddingBytes );
    UInt32 GetQueryPaddingBytes( void ) const;

    /** Whether a query for every service type ("*") is answered, within the same bounds; default true */
    void SetAnswerWildcardQueries( bool answerWildcardQueries );
    bool GetAnswerWildcardQueries( void ) const;

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

    /** Prepares and opens the query socket on a port the system picks; by default with broadcasts allowed */
    virtual bool OpenQuerySocket( COMCORE::CUDPSocket& socket );

    /** The addresses everything is sent to, next to the extra targets. Asked for each time the socket opens */
    virtual void GetDestinations( TIPv4AddressVector& destinations ) const = 0;

    virtual bool OnBrowseStarted( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;
    virtual void OnBrowseStopped( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnAdvertised( const COMCORE::CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnWithdrawn( const COMCORE::CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHousekeeping( UInt64 nowInMs ) GUCEF_VIRTUAL_OVERRIDE;

    /** Nothing is browsed or advertised: the socket is closed, which also leaves the groups it joined */
    virtual void OnIdle( void ) GUCEF_VIRTUAL_OVERRIDE;

    /** Moves both sockets to the new pulse generator */
    virtual void OnPulseGeneratorChanged( const CORE::PulseGeneratorPtr& pulseGenerator ) GUCEF_VIRTUAL_OVERRIDE;

    /** Closes the socket after withdrawing everything. Derived classes call it from their destructor */
    void Shutdown( void );

    private:

    typedef GUCEF::map< CORE::CString, UInt64 > TAnnounceTimeMap;
    typedef CORE::CTEventHandlerFunctor< CUdpDiscoveryClientBase > TEventCallback;

    CUdpDiscoveryClientBase( const CUdpDiscoveryClientBase& src );
    CUdpDiscoveryClientBase& operator=( const CUdpDiscoveryClientBase& src );

    bool EnsureSocketIsOpen( void );
    bool EnsureQuerySocketIsOpen( void );
    bool SendBeacon( const CORE::CDynamicBuffer& beacon );
    bool SendBeaconFrom( COMCORE::CUDPSocket& socket, const CORE::CDynamicBuffer& beacon );
    bool Announce( const COMCORE::CIDiscoveryEntry& entry, UInt64 nowInMs );
    bool IsVerifiedSource( const COMCORE::CIPv4Address& source ) const;
    bool IsOwnDestination( const COMCORE::CIPv4Address& source ) const;
    bool OnQuery( const COMCORE::CIPv4Address& source, UInt32 querySizeInBytes, const CORE::CString& serviceType );
    bool OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet, bool fromQuerySocket );

    void RegisterSocketEventHandlers( void );

    void OnSocketPackets( CORE::CNotifier* notifier    ,
                          const CORE::CEvent& eventId  ,
                          CORE::CICloneable* eventData );

    void OnQuerySocketPackets( CORE::CNotifier* notifier    ,
                               const CORE::CEvent& eventId  ,
                               CORE::CICloneable* eventData );

    void ProcessPackets( CORE::CICloneable* eventData, bool fromQuerySocket );

    private:

    COMCORE::CUDPSocket m_socket;
    COMCORE::CUDPSocket m_querySocket;
    UInt32 m_queryPaddingBytes;
    bool m_answerWildcardQueries;
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
