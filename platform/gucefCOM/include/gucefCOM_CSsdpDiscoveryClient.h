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

#ifndef GUCEF_COM_CSSDPDISCOVERYCLIENT_H
#define GUCEF_COM_CSSDPDISCOVERYCLIENT_H

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
 *  Discovery with SSDP, the way UPnP finds devices, on 239.255.255.250 port 1900.
 *
 *  A browse sends M-SEARCH queries with the service type as ST ("ssdp:all" when browsing for any service type)
 *  from a socket of its own, which receives the unicast answers, and listens for NOTIFY messages on the group.
 *  UPnP devices answer too: their entries have the ST as service type, the USN as instance id and the LOCATION
 *  as endpoint.
 *
 *  An advertised entry is announced with NOTIFY ssdp:alive on advertising and every TTL/3, answered for on
 *  M-SEARCH and withdrawn with NOTIFY ssdp:byebye. The USN is uuid:<application instance id>::<service type>,
 *  the LOCATION the first endpoint, and the rest travels in headers: X-GUCEF-NAME (instance name),
 *  X-GUCEF-ENDPOINT (each endpoint) and X-GUCEF-ATTR-<key> (each attribute).
 *
 *  Settings (LoadConfig): port, group, maxSearchIntervalInMs
 */
class GUCEF_COM_PUBLIC_CPP CSsdpDiscoveryClient : public COMCORE::CDiscoveryClientBase
{
    public:

    static const CORE::CString TypeName;            /**< "ssdp" */
    static const CORE::CString DefaultGroup;        /**< 239.255.255.250 */
    static const UInt16 DefaultPort;                /**< 1900 */

    CSsdpDiscoveryClient( void );

    explicit CSsdpDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CSsdpDiscoveryClient();

    void SetPort( UInt16 port );

    UInt16 GetPort( void ) const;

    void SetMaxSearchIntervalInMs( UInt32 maxSearchIntervalInMs );

    bool IsActive( void ) const;

    virtual bool LoadConfig( const CORE::CDataNode& config ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool SaveConfig( CORE::CDataNode& config ) const GUCEF_VIRTUAL_OVERRIDE;

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE;

    protected:

    virtual bool OnBrowseStarted( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnBrowseStopped( const CORE::CString& serviceType ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnAdvertised( const COMCORE::CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual bool OnWithdrawn( const COMCORE::CIDiscoveryEntryPtr& entry ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnHousekeeping( UInt64 nowInMs ) GUCEF_VIRTUAL_OVERRIDE;

    virtual void OnNotify( CORE::CNotifier* notifier                 ,
                           const CORE::CEvent& eventId               ,
                           CORE::CICloneable* eventData = GUCEF_NULL ) GUCEF_VIRTUAL_OVERRIDE;

    private:

    struct SHeader
    {
        CORE::CString name;
        CORE::CString value;
    };
    typedef struct SHeader                              THeader;
    typedef GUCEF::vector< THeader >                    THeaderVector;

    struct SSearchSchedule
    {
        UInt64 nextSearchInMs;
        UInt32 intervalInMs;
    };
    typedef struct SSearchSchedule TSearchSchedule;
    typedef GUCEF::map< CORE::CString, TSearchSchedule > TSearchScheduleMap;
    typedef GUCEF::map< CORE::CString, UInt64 > TAnnounceTimeMap;

    CSsdpDiscoveryClient( const CSsdpDiscoveryClient& src );
    CSsdpDiscoveryClient& operator=( const CSsdpDiscoveryClient& src );

    bool EnsureSocketsAreOpen( void );
    bool Send( COMCORE::CUDPSocket& socket, const CORE::CString& message, const COMCORE::CIPv4Address& destination );
    bool SendSearch( const CORE::CString& serviceType );
    bool SendNotify( const COMCORE::CIDiscoveryEntry& entry, bool alive );
    bool SendSearchResponse( const COMCORE::CIDiscoveryEntry& entry, const COMCORE::CIPv4Address& destination );
    void AppendEntryHeaders( const COMCORE::CIDiscoveryEntry& entry, CORE::CString& message ) const;

    bool OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet, bool fromSearchSocket );
    bool OnSearch( const COMCORE::CIPv4Address& source, const THeaderVector& headers );
    bool OnAnnouncement( const COMCORE::CIPv4Address& source, const THeaderVector& headers, const CORE::CString& serviceType, bool alive );

    static bool ParseMessage( const CORE::CDynamicBuffer& packet, CORE::CString& startLine, THeaderVector& headers );
    static const CORE::CString* FindHeader( const THeaderVector& headers, const char* name );
    static CORE::CString MakeUsn( const COMCORE::CIDiscoveryEntry& entry );
    static CORE::CString GetInstanceIdFromUsn( const CORE::CString& usn );

    private:

    COMCORE::CUDPSocket m_groupSocket;
    COMCORE::CUDPSocket m_searchSocket;
    COMCORE::CIPv4Address m_group;
    UInt16 m_port;
    UInt32 m_maxSearchIntervalInMs;
    TSearchScheduleMap m_searchSchedules;
    TAnnounceTimeMap m_nextAnnounceTimes;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDiscoveryClient, CSsdpDiscoveryClient, MT::CMutex > TSsdpDiscoveryClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CSSDPDISCOVERYCLIENT_H ? */
