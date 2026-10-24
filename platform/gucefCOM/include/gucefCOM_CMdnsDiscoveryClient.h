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

#ifndef GUCEF_COM_CMDNSDISCOVERYCLIENT_H
#define GUCEF_COM_CMDNSDISCOVERYCLIENT_H

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

#ifndef GUCEF_COM_CDNSMESSAGE_H
#include "gucefCOM_CDnsMessage.h"
#define GUCEF_COM_CDNSMESSAGE_H
#endif /* GUCEF_COM_CDNSMESSAGE_H ? */

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
 *  Discovery with mDNS and DNS-SD (RFC 6762, RFC 6763) on 224.0.0.251 port 5353, so it finds and is found by
 *  Avahi, Bonjour and the like. The socket shares the port with a responder of the system.
 *
 *  A browse for "_gucef-game._udp" asks for the PTR records of "_gucef-game._udp.local", resolves the instances
 *  they point to with their SRV, TXT and A records and reports an entry once the address is known. Queries repeat
 *  with a doubling interval and carry the known answers, entries are refreshed before their TTL runs out.
 *
 *  An advertised entry is announced twice with PTR, SRV, TXT and A records and answered for after that. The SRV
 *  port is the port of the first endpoint of the entry, the TXT strings are the attributes as key=value plus the
 *  application instance id under the key "gucef-id". A withdrawal sends the records with TTL 0.
 *
 *  Settings (LoadConfig): port, group, hostName (without .local, defaults to the name of the host plus -gucef),
 *  maxQueryIntervalInMs
 */
class GUCEF_COM_PUBLIC_CPP CMdnsDiscoveryClient : public COMCORE::CDiscoveryClientBase
{
    public:

    static const CORE::CString TypeName;            /**< "mdns" */
    static const CORE::CString DefaultGroup;        /**< 224.0.0.251 */
    static const UInt16 DefaultPort;                /**< 5353 */
    static const CORE::CString InstanceIdTextKey;   /**< "gucef-id": the TXT key of the application instance id */
    static const CORE::CString ServiceEnumerationName; /**< "_services._dns-sd._udp.local" */

    CMdnsDiscoveryClient( void );

    explicit CMdnsDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator );

    virtual ~CMdnsDiscoveryClient();

    void SetPort( UInt16 port );

    UInt16 GetPort( void ) const;

    /** The host name used in SRV and A records, without .local */
    void SetHostName( const CORE::CString& hostName );

    const CORE::CString& GetHostName( void ) const;

    void SetMaxQueryIntervalInMs( UInt32 maxQueryIntervalInMs );

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

    /** What is known of a service instance found on the network */
    struct SInstanceInfo
    {
        CORE::CString serviceType;          /**< without .local */
        CORE::CString instanceName;         /**< the first label, unescaped */
        CORE::CString fullName;             /**< as on the wire */
        UInt32 ttlInSecs;                   /**< of the PTR record */
        UInt64 expiresAtInMs;
        bool refreshQueried;
        bool hasSrv;
        CORE::CString host;
        UInt16 port;
        bool hasTxt;
        CORE::CString::StringVector texts;
        UInt64 lastResolveQueryInMs;
        bool reported;
        CORE::CString reportedInstanceId;
    };
    typedef struct SInstanceInfo TInstanceInfo;
    typedef GUCEF::map< CORE::CString, TInstanceInfo > TInstanceMap;

    struct SHostInfo
    {
        GUCEF::vector< UInt32 > addresses;  /**< network byte order */
        UInt64 expiresAtInMs;
    };
    typedef struct SHostInfo THostInfo;
    typedef GUCEF::map< CORE::CString, THostInfo > THostMap;

    struct SQuerySchedule
    {
        UInt64 nextQueryInMs;
        UInt32 intervalInMs;
    };
    typedef struct SQuerySchedule TQuerySchedule;
    typedef GUCEF::map< CORE::CString, TQuerySchedule > TQueryScheduleMap;

    struct SAnnouncement
    {
        UInt64 nextInMs;
        UInt32 remaining;
    };
    typedef struct SAnnouncement TAnnouncement;
    typedef GUCEF::map< CORE::CString, TAnnouncement > TAnnouncementMap;

    typedef GUCEF::vector< UInt32 > TAddressVector;

    CMdnsDiscoveryClient( const CMdnsDiscoveryClient& src );
    CMdnsDiscoveryClient& operator=( const CMdnsDiscoveryClient& src );

    bool EnsureSocketIsOpen( void );
    bool SendMessage( const CDnsMessage& message, const COMCORE::CIPv4Address& destination );
    bool SendToGroup( const CDnsMessage& message );
    bool SendBrowseQuery( const CORE::CString& serviceType, UInt64 nowInMs );
    bool SendResolveQuery( TInstanceInfo& instance, UInt64 nowInMs );

    void GetOwnAddresses( TAddressVector& addresses ) const;
    CORE::CString GetOwnHostFullName( void ) const;
    bool GetInstanceLabel( const COMCORE::CIDiscoveryEntry& entry, CORE::CString& label ) const;
    UInt16 GetServicePort( const COMCORE::CIDiscoveryEntry& entry ) const;
    bool AddEntryRecords( const COMCORE::CIDiscoveryEntry& entry, UInt32 ttlInSecs, bool asAnswers, CDnsMessage& message ) const;
    bool Announce( const COMCORE::CIDiscoveryEntry& entry, UInt32 ttlInSecs );

    bool OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet );
    bool OnQuery( const COMCORE::CIPv4Address& source, const CDnsMessage& query );
    bool OnResponse( const CDnsMessage& response );
    void OnRecord( const CDnsMessage::CResourceRecord& record, UInt64 nowInMs, CORE::CString::StringSet& touched );
    bool UpdateInstance( TInstanceInfo& instance, UInt64 nowInMs );
    bool ForgetInstance( const CORE::CString& key );

    bool IsKnownAnswer( const CDnsMessage& query, const CDnsMessage::CResourceRecord& record ) const;
    bool IsServiceTypeOfInterest( const CORE::CString& serviceType ) const;

    static CORE::CString StripLocal( const CORE::CString& name );

    private:

    COMCORE::CUDPSocket m_socket;
    COMCORE::CIPv4Address m_group;
    UInt16 m_port;
    CORE::CString m_hostName;
    UInt32 m_maxQueryIntervalInMs;
    CORE::CDynamicBuffer m_sendBuffer;
    TInstanceMap m_instances;
    THostMap m_hosts;
    TQueryScheduleMap m_querySchedules;
    TAnnouncementMap m_announcements;
};

/*-------------------------------------------------------------------------*/

typedef CORE::CTFactory< COMCORE::CIDiscoveryClient, CMdnsDiscoveryClient, MT::CMutex > TMdnsDiscoveryClientFactory;

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_COM_CMDNSDISCOVERYCLIENT_H ? */
