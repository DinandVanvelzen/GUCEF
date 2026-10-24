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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CCOM_H
#include "CCom.h"
#define GUCEF_COMCORE_CCOM_H
#endif /* GUCEF_COMCORE_CCOM_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#include "gucefCOM_CUdpDiscoveryClientBase.h"
#define GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H ? */

#include "gucefCOM_CMdnsDiscoveryClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMdnsDiscoveryClient::TypeName = "mdns";
const CORE::CString CMdnsDiscoveryClient::DefaultGroup = "224.0.0.251";
const UInt16 CMdnsDiscoveryClient::DefaultPort = 5353;
const CORE::CString CMdnsDiscoveryClient::InstanceIdTextKey = "gucef-id";
const CORE::CString CMdnsDiscoveryClient::ServiceEnumerationName = "_services._dns-sd._udp.local";

static const CORE::CString LocalDomain = "local";
static const UInt32 HostRecordTtlInSecs = 120;
static const UInt32 FirstQueryIntervalInMs = 1000;
static const UInt32 ResolveQueryIntervalInMs = 1000;
static const UInt32 AnnouncementCount = 2;
static const UInt32 AnnouncementIntervalInMs = 1000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** Whether the name looks like a DNS-SD service type: _name._udp or _name._tcp */
bool
LooksLikeServiceType( const CORE::CString& serviceType )
{GUCEF_TRACE;

    CORE::CString lower = serviceType.Lowercase();
    return lower.StartsWith( "_" ) && ( lower.EndsWith( "._udp" ) || lower.EndsWith( "._tcp" ) );
}

/*-------------------------------------------------------------------------*/

/** The scheme of the endpoints of a service type: udp for _x._udp, tcp for _x._tcp */
CORE::CString
GetEndpointScheme( const CORE::CString& serviceType )
{GUCEF_TRACE;

    return serviceType.Lowercase().EndsWith( "._tcp" ) ? CORE::CString( "tcp" ) : CORE::CString( "udp" );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMdnsDiscoveryClient::CMdnsDiscoveryClient( void )
    : COMCORE::CDiscoveryClientBase( TypeName, CORE::PulseGeneratorPtr() )
    , m_socket( GetPulseGenerator(), false )
    , m_group( DefaultGroup, DefaultPort )
    , m_port( DefaultPort )
    , m_hostName( CORE::GetHostname() + "-gucef" )
    , m_maxQueryIntervalInMs( 60000 )
    , m_sendBuffer( 1500, true )
    , m_instances()
    , m_hosts()
    , m_querySchedules()
    , m_announcements()
{GUCEF_TRACE;

    SubscribeTo( &m_socket );
}

/*-------------------------------------------------------------------------*/

CMdnsDiscoveryClient::CMdnsDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CDiscoveryClientBase( TypeName, pulseGenerator )
    , m_socket( GetPulseGenerator(), false )
    , m_group( DefaultGroup, DefaultPort )
    , m_port( DefaultPort )
    , m_hostName( CORE::GetHostname() + "-gucef" )
    , m_maxQueryIntervalInMs( 60000 )
    , m_sendBuffer( 1500, true )
    , m_instances()
    , m_hosts()
    , m_querySchedules()
    , m_announcements()
{GUCEF_TRACE;

    SubscribeTo( &m_socket );
}

/*-------------------------------------------------------------------------*/

CMdnsDiscoveryClient::~CMdnsDiscoveryClient()
{GUCEF_TRACE;

    StopAll();
    UnsubscribeFrom( &m_socket );
    m_socket.Close();
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::SetPort( UInt16 port )
{GUCEF_TRACE;

    if ( port != m_port )
    {
        m_port = port;
        m_group.SetPortInHostByteOrder( port );
        if ( m_socket.IsActive() )
        {
            m_socket.Close();
            EnsureSocketIsOpen();
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt16
CMdnsDiscoveryClient::GetPort( void ) const
{GUCEF_TRACE;

    return m_port;
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::SetHostName( const CORE::CString& hostName )
{GUCEF_TRACE;

    m_hostName = StripLocal( hostName );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMdnsDiscoveryClient::GetHostName( void ) const
{GUCEF_TRACE;

    return m_hostName;
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::SetMaxQueryIntervalInMs( UInt32 maxQueryIntervalInMs )
{GUCEF_TRACE;

    m_maxQueryIntervalInMs = maxQueryIntervalInMs > FirstQueryIntervalInMs ? maxQueryIntervalInMs : FirstQueryIntervalInMs;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::IsActive( void ) const
{GUCEF_TRACE;

    return m_socket.IsActive();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMdnsDiscoveryClient::StripLocal( const CORE::CString& name )
{GUCEF_TRACE;

    CORE::CString lower = name.Lowercase();
    if ( lower.EndsWith( ".local" ) )
        return name.CutChars( 6, false, 0 );
    if ( lower.EndsWith( ".local." ) )
        return name.CutChars( 7, false, 0 );
    return name;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMdnsDiscoveryClient::GetOwnHostFullName( void ) const
{GUCEF_TRACE;

    return CDnsMessage::MakeName( m_hostName, LocalDomain );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::EnsureSocketIsOpen( void )
{GUCEF_TRACE;

    if ( m_socket.IsActive() )
        return true;

    // RFC 6762: multicast DNS messages go out with an IP TTL of 255
    m_socket.SetMulticastTTL( 255 );
    m_socket.SetAllowMulticastLoopback( true );
    if ( !m_socket.Open( m_port ) )
    {
        ReportDiscoveryError( "Unable to open the mDNS socket on port " + CORE::UInt16ToString( m_port ) );
        return false;
    }
    if ( 0 == CUdpDiscoveryClientBase::JoinOnAllInterfaces( m_socket, m_group ) )
    {
        ReportDiscoveryError( "Unable to join the mDNS group " + m_group.AddressAsString() + " on any network interface" );
        m_socket.Close();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::SendMessage( const CDnsMessage& message, const COMCORE::CIPv4Address& destination )
{GUCEF_TRACE;

    if ( !EnsureSocketIsOpen() )
        return false;
    if ( !message.Serialize( m_sendBuffer ) || m_sendBuffer.GetDataSize() > 0xFFFF )
        return false;

    Int32 sent = m_socket.SendPacketTo( destination, m_sendBuffer.GetConstBufferPtr(), (UInt16) m_sendBuffer.GetDataSize() );
    if ( sent != (Int32) m_sendBuffer.GetDataSize() )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MdnsDiscoveryClient(" << GetDiscoveryClientName() << "): Failed to send a message to " << destination.AddressAndPortAsString() );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::SendToGroup( const CDnsMessage& message )
{GUCEF_TRACE;

    return SendMessage( message, m_group );
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::GetOwnAddresses( TAddressVector& addresses ) const
{GUCEF_TRACE;

    COMCORE::CINetworkInterface::TIPv4InfoVector ipInfo;
    if ( COMCORE::CComCoreGlobal::Instance()->GetCom().GetAllNetworkInterfaceIPInfo( ipInfo ) )
    {
        for ( size_t i=0; i<ipInfo.size(); ++i )
        {
            UInt32 address = ipInfo[ i ].ip.GetAddress();
            if ( 0 == address || 0x7F == ( ipInfo[ i ].ip.GetAddressInHostByteOrder() >> 24 ) )
                continue;
            bool known = false;
            for ( size_t n=0; n<addresses.size(); ++n )
                if ( addresses[ n ] == address ) { known = true; break; }
            if ( !known )
                addresses.push_back( address );
        }
    }
    if ( addresses.empty() )
        addresses.push_back( COMCORE::CIPv4Address::Loopback.GetAddress() );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::GetInstanceLabel( const COMCORE::CIDiscoveryEntry& entry, CORE::CString& label ) const
{GUCEF_TRACE;

    label = entry.GetInstanceName();
    if ( label.IsNULLOrEmpty() )
        entry.GetApplicationInstanceId( label );
    return !label.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

UInt16
CMdnsDiscoveryClient::GetServicePort( const COMCORE::CIDiscoveryEntry& entry ) const
{GUCEF_TRACE;

    const CORE::CString::StringVector& endpoints = entry.GetEndpoints();
    for ( size_t i=0; i<endpoints.size(); ++i )
    {
        Int32 portStart = endpoints[ i ].HasChar( ':', false );
        if ( portStart > 0 )
        {
            CORE::CString portStr = endpoints[ i ].CutChars( (UInt32) portStart + 1, true, 0 );
            UInt16 port = CORE::StringToUInt16( portStr.SubstrToChar( '/', true ), 0 );
            if ( port > 0 )
                return port;
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::AddEntryRecords( const COMCORE::CIDiscoveryEntry& entry ,
                                       UInt32 ttlInSecs                       ,
                                       bool asAnswers                         ,
                                       CDnsMessage& message                   ) const
{GUCEF_TRACE;

    CORE::CString label;
    if ( !GetInstanceLabel( entry, label ) )
        return false;

    CORE::CString serviceName = StripLocal( entry.GetServiceType() ) + ".local";
    CORE::CString instanceName = CDnsMessage::MakeName( label, serviceName );
    CORE::CString hostFullName = GetOwnHostFullName();

    CORE::CString::StringVector texts;
    CDnsMessage::AttributesToTexts( entry.GetAttributes(), texts );
    CORE::CString instanceId;
    entry.GetApplicationInstanceId( instanceId );
    if ( !instanceId.IsNULLOrEmpty() )
        texts.push_back( InstanceIdTextKey + "=" + instanceId );

    message.answers.push_back( CDnsMessage::CResourceRecord::MakePtr( serviceName, instanceName, ttlInSecs ) );

    CDnsMessage::TResourceRecordVector& rest = asAnswers ? message.answers : message.additionals;
    rest.push_back( CDnsMessage::CResourceRecord::MakeSrv( instanceName, hostFullName, GetServicePort( entry ), ttlInSecs ) );
    rest.push_back( CDnsMessage::CResourceRecord::MakeTxt( instanceName, texts, ttlInSecs ) );

    // A goodbye is about the service, the host stays
    if ( ttlInSecs > 0 )
    {
        TAddressVector addresses;
        GetOwnAddresses( addresses );
        for ( size_t i=0; i<addresses.size(); ++i )
            rest.push_back( CDnsMessage::CResourceRecord::MakeA( hostFullName, addresses[ i ], HostRecordTtlInSecs ) );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::Announce( const COMCORE::CIDiscoveryEntry& entry, UInt32 ttlInSecs )
{GUCEF_TRACE;

    CDnsMessage message;
    message.flags = CDnsMessage::FlagResponse | CDnsMessage::FlagAuthoritative;
    if ( !AddEntryRecords( entry, ttlInSecs, true, message ) )
    {
        ReportDiscoveryError( "The entry " + entry.GetEntryKey() + " has no instance name to advertise with" );
        return false;
    }
    return SendToGroup( message );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::IsServiceTypeOfInterest( const CORE::CString& serviceType ) const
{GUCEF_TRACE;

    if ( IsServiceTypeBrowsed( serviceType ) )
        return true;

    // A browse may have been started with .local in the service type or with another case of letters
    CORE::CString::StringSet browsed;
    GetBrowsedServiceTypes( browsed );
    CORE::CString::StringSet::iterator i = browsed.begin();
    while ( i != browsed.end() )
    {
        if ( CDnsMessage::NamesAreEqual( StripLocal( (*i) ), serviceType ) )
            return true;
        ++i;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::SendBrowseQuery( const CORE::CString& serviceType, UInt64 nowInMs )
{GUCEF_TRACE;

    CDnsMessage query;
    if ( serviceType == COMCORE::CIDiscoveryClient::AnyServiceType )
    {
        query.questions.push_back( CDnsMessage::CQuestion( ServiceEnumerationName, CDnsMessage::RECORDTYPE_PTR ) );
        return SendToGroup( query );
    }

    CORE::CString serviceName = StripLocal( serviceType ) + ".local";
    query.questions.push_back( CDnsMessage::CQuestion( serviceName, CDnsMessage::RECORDTYPE_PTR ) );

    // Known answer suppression: what we know with more than half of its TTL left does not need to be told again
    TInstanceMap::iterator i = m_instances.begin();
    while ( i != m_instances.end() )
    {
        const TInstanceInfo& instance = (*i).second;
        if ( CDnsMessage::NamesAreEqual( instance.serviceType, StripLocal( serviceType ) ) && instance.expiresAtInMs > nowInMs )
        {
            UInt64 remainingInMs = instance.expiresAtInMs - nowInMs;
            if ( remainingInMs * 2 > instance.ttlInSecs * (UInt64) 1000 )
                query.answers.push_back( CDnsMessage::CResourceRecord::MakePtr( serviceName, instance.fullName, (UInt32)( remainingInMs / 1000 ) ) );
        }
        ++i;
    }
    return SendToGroup( query );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::SendResolveQuery( TInstanceInfo& instance, UInt64 nowInMs )
{GUCEF_TRACE;

    if ( 0 != instance.lastResolveQueryInMs && nowInMs - instance.lastResolveQueryInMs < ResolveQueryIntervalInMs )
        return true;
    instance.lastResolveQueryInMs = nowInMs;

    CDnsMessage query;
    if ( !instance.hasSrv )
        query.questions.push_back( CDnsMessage::CQuestion( instance.fullName, CDnsMessage::RECORDTYPE_SRV ) );
    if ( !instance.hasTxt )
        query.questions.push_back( CDnsMessage::CQuestion( instance.fullName, CDnsMessage::RECORDTYPE_TXT ) );
    if ( instance.hasSrv )
        query.questions.push_back( CDnsMessage::CQuestion( instance.host, CDnsMessage::RECORDTYPE_A ) );
    if ( query.questions.empty() )
        return true;
    return SendToGroup( query );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::OnBrowseStarted( const CORE::CString& serviceType )
{GUCEF_TRACE;

    if ( serviceType != COMCORE::CIDiscoveryClient::AnyServiceType && !LooksLikeServiceType( StripLocal( serviceType ) ) )
    {
        ReportDiscoveryError( "\"" + serviceType + "\" is not a DNS-SD service type such as _name._udp" );
        return false;
    }
    if ( !EnsureSocketIsOpen() )
        return false;

    UInt64 nowInMs = GetTimeInMs();
    TQuerySchedule& schedule = m_querySchedules[ serviceType ];
    schedule.intervalInMs = FirstQueryIntervalInMs;
    schedule.nextQueryInMs = nowInMs + FirstQueryIntervalInMs;

    // The browse goes on even when this first query does not get out, the next ones may
    SendBrowseQuery( serviceType, nowInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::OnBrowseStopped( const CORE::CString& serviceType )
{GUCEF_TRACE;

    m_querySchedules.erase( serviceType );

    // Forget the instances that are no longer of interest
    TInstanceMap::iterator i = m_instances.begin();
    while ( i != m_instances.end() )
    {
        if ( !IsServiceTypeOfInterest( (*i).second.serviceType ) )
            m_instances.erase( i++ );
        else
            ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::OnAdvertised( const COMCORE::CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    if ( !LooksLikeServiceType( StripLocal( entry->GetServiceType() ) ) )
    {
        ReportDiscoveryError( "\"" + entry->GetServiceType() + "\" is not a DNS-SD service type such as _name._udp" );
        return false;
    }
    CORE::CString label;
    if ( !GetInstanceLabel( *entry, label ) )
    {
        ReportDiscoveryError( "The entry " + entry->GetEntryKey() + " has no instance name to advertise with" );
        return false;
    }
    if ( !EnsureSocketIsOpen() )
        return false;

    Announce( *entry, entry->GetTtlInSecs() );

    TAnnouncement& announcement = m_announcements[ entry->GetEntryKey() ];
    announcement.remaining = AnnouncementCount - 1;
    announcement.nextInMs = GetTimeInMs() + AnnouncementIntervalInMs;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::OnWithdrawn( const COMCORE::CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    m_announcements.erase( entry->GetEntryKey() );
    if ( !m_socket.IsActive() )
        return true;
    return Announce( *entry, 0 );
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::OnHousekeeping( UInt64 nowInMs )
{GUCEF_TRACE;

    // The second announcement of new advertisements
    if ( !m_announcements.empty() )
    {
        TIDiscoveryEntryPtrVector advertised;
        GetAdvertisedEntries( advertised );
        for ( size_t i=0; i<advertised.size(); ++i )
        {
            TAnnouncementMap::iterator n = m_announcements.find( advertised[ i ]->GetEntryKey() );
            if ( n != m_announcements.end() && (*n).second.nextInMs <= nowInMs )
            {
                Announce( *advertised[ i ], advertised[ i ]->GetTtlInSecs() );
                if ( --(*n).second.remaining == 0 )
                    m_announcements.erase( n );
                else
                    (*n).second.nextInMs = nowInMs + AnnouncementIntervalInMs;
            }
        }
    }

    // The repeating browse queries with a doubling interval
    CORE::CString::StringSet refreshTypes;
    TQueryScheduleMap::iterator q = m_querySchedules.begin();
    while ( q != m_querySchedules.end() )
    {
        TQuerySchedule& schedule = (*q).second;
        if ( schedule.nextQueryInMs <= nowInMs )
        {
            schedule.intervalInMs = schedule.intervalInMs * 2 > m_maxQueryIntervalInMs ? m_maxQueryIntervalInMs : schedule.intervalInMs * 2;
            schedule.nextQueryInMs = nowInMs + schedule.intervalInMs;
            refreshTypes.insert( (*q).first );
        }
        ++q;
    }

    // Instances near the end of their TTL are asked for again, those past it are forgotten
    TInstanceMap::iterator i = m_instances.begin();
    while ( i != m_instances.end() )
    {
        TInstanceInfo& instance = (*i).second;
        if ( instance.expiresAtInMs <= nowInMs )
        {
            m_instances.erase( i++ );
            continue;
        }
        UInt64 ttlInMs = instance.ttlInSecs * (UInt64) 1000;
        if ( !instance.refreshQueried && instance.expiresAtInMs - nowInMs < ttlInMs / 5 )
        {
            instance.refreshQueried = true;
            CORE::CString::StringSet browsed;
            GetBrowsedServiceTypes( browsed );
            CORE::CString::StringSet::iterator b = browsed.begin();
            while ( b != browsed.end() )
            {
                if ( CDnsMessage::NamesAreEqual( StripLocal( (*b) ), instance.serviceType ) )
                    refreshTypes.insert( (*b) );
                ++b;
            }
        }
        else
        if ( !instance.reported )
        {
            SendResolveQuery( instance, nowInMs );
        }
        ++i;
    }

    THostMap::iterator h = m_hosts.begin();
    while ( h != m_hosts.end() )
    {
        if ( (*h).second.expiresAtInMs <= nowInMs )
            m_hosts.erase( h++ );
        else
            ++h;
    }

    CORE::CString::StringSet::iterator r = refreshTypes.begin();
    while ( r != refreshTypes.end() )
    {
        SendBrowseQuery( (*r), nowInMs );
        ++r;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::IsKnownAnswer( const CDnsMessage& query, const CDnsMessage::CResourceRecord& record ) const
{GUCEF_TRACE;

    for ( size_t i=0; i<query.answers.size(); ++i )
    {
        const CDnsMessage::CResourceRecord& known = query.answers[ i ];
        if ( known.type == record.type &&
             CDnsMessage::NamesAreEqual( known.name, record.name ) &&
             CDnsMessage::NamesAreEqual( known.target, record.target ) &&
             known.ttlInSecs * 2 >= record.ttlInSecs )
            return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::OnQuery( const COMCORE::CIPv4Address& source, const CDnsMessage& query )
{GUCEF_TRACE;

    TIDiscoveryEntryPtrVector advertised;
    GetAdvertisedEntries( advertised );
    if ( advertised.empty() )
        return true;

    CDnsMessage response;
    response.flags = CDnsMessage::FlagResponse | CDnsMessage::FlagAuthoritative;
    CORE::CString hostFullName = GetOwnHostFullName();
    bool answerHost = false;

    for ( size_t q=0; q<query.questions.size(); ++q )
    {
        const CDnsMessage::CQuestion& question = query.questions[ q ];
        bool anyType = CDnsMessage::RECORDTYPE_ANY == question.type;

        if ( ( anyType || CDnsMessage::RECORDTYPE_PTR == question.type ) && CDnsMessage::NamesAreEqual( question.name, ServiceEnumerationName ) )
        {
            CORE::CString::StringSet served;
            for ( size_t a=0; a<advertised.size(); ++a )
            {
                CORE::CString serviceName = StripLocal( advertised[ a ]->GetServiceType() ) + ".local";
                if ( served.insert( serviceName.Lowercase() ).second )
                    response.answers.push_back( CDnsMessage::CResourceRecord::MakePtr( ServiceEnumerationName, serviceName, advertised[ a ]->GetTtlInSecs() ) );
            }
            continue;
        }

        for ( size_t a=0; a<advertised.size(); ++a )
        {
            const COMCORE::CIDiscoveryEntry& entry = *advertised[ a ];
            CORE::CString serviceName = StripLocal( entry.GetServiceType() ) + ".local";
            CORE::CString label;
            if ( !GetInstanceLabel( entry, label ) )
                continue;
            CORE::CString instanceName = CDnsMessage::MakeName( label, serviceName );

            if ( ( anyType || CDnsMessage::RECORDTYPE_PTR == question.type ) && CDnsMessage::NamesAreEqual( question.name, serviceName ) )
            {
                CDnsMessage::CResourceRecord ptr = CDnsMessage::CResourceRecord::MakePtr( serviceName, instanceName, entry.GetTtlInSecs() );
                if ( !IsKnownAnswer( query, ptr ) )
                    AddEntryRecords( entry, entry.GetTtlInSecs(), false, response );
            }
            else
            if ( CDnsMessage::NamesAreEqual( question.name, instanceName ) &&
                 ( anyType || CDnsMessage::RECORDTYPE_SRV == question.type || CDnsMessage::RECORDTYPE_TXT == question.type ) )
            {
                CDnsMessage records;
                AddEntryRecords( entry, entry.GetTtlInSecs(), true, records );
                for ( size_t r=0; r<records.answers.size(); ++r )
                {
                    const CDnsMessage::CResourceRecord& record = records.answers[ r ];
                    if ( ( anyType || record.type == question.type ) && CDnsMessage::RECORDTYPE_PTR != record.type )
                        response.answers.push_back( record );
                    else
                    if ( CDnsMessage::RECORDTYPE_A == record.type )
                        response.additionals.push_back( record );
                }
            }
        }

        if ( ( anyType || CDnsMessage::RECORDTYPE_A == question.type ) && CDnsMessage::NamesAreEqual( question.name, hostFullName ) )
            answerHost = true;
    }

    if ( answerHost )
    {
        TAddressVector addresses;
        GetOwnAddresses( addresses );
        for ( size_t i=0; i<addresses.size(); ++i )
            response.answers.push_back( CDnsMessage::CResourceRecord::MakeA( hostFullName, addresses[ i ], HostRecordTtlInSecs ) );
    }

    if ( response.answers.empty() )
        return true;

    // A query from another port than the mDNS port is a simple resolver: it gets a unicast answer the classic way
    if ( source.GetPortInHostByteOrder() != m_port )
    {
        response.id = query.id;
        response.questions = query.questions;
        return SendMessage( response, source );
    }
    return SendToGroup( response );
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::OnRecord( const CDnsMessage::CResourceRecord& record ,
                                UInt64 nowInMs                             ,
                                CORE::CString::StringSet& touched          )
{GUCEF_TRACE;

    switch ( record.type )
    {
        case CDnsMessage::RECORDTYPE_A:
        {
            CORE::CString key = record.name.Lowercase();
            if ( 0 == record.ttlInSecs )
            {
                m_hosts.erase( key );
                return;
            }
            THostInfo& host = m_hosts[ key ];
            if ( record.cacheFlush && host.expiresAtInMs <= nowInMs )
                host.addresses.clear();
            bool known = false;
            for ( size_t i=0; i<host.addresses.size(); ++i )
                if ( host.addresses[ i ] == record.ipv4AddressInNetworkByteOrder ) { known = true; break; }
            if ( !known )
                host.addresses.push_back( record.ipv4AddressInNetworkByteOrder );
            host.expiresAtInMs = nowInMs + record.ttlInSecs * (UInt64) 1000;

            // The instances on this host may now be complete
            TInstanceMap::iterator i = m_instances.begin();
            while ( i != m_instances.end() )
            {
                if ( (*i).second.hasSrv && CDnsMessage::NamesAreEqual( (*i).second.host, record.name ) )
                    touched.insert( (*i).first );
                ++i;
            }
            return;
        }
        case CDnsMessage::RECORDTYPE_PTR:
        {
            if ( CDnsMessage::NamesAreEqual( record.name, ServiceEnumerationName ) )
            {
                // A service type someone offers: when browsing for anything, ask for its instances
                CORE::CString serviceType = StripLocal( record.target );
                if ( record.ttlInSecs > 0 && IsServiceTypeBrowsed( COMCORE::CIDiscoveryClient::AnyServiceType ) && LooksLikeServiceType( serviceType ) )
                {
                    CDnsMessage query;
                    query.questions.push_back( CDnsMessage::CQuestion( record.target, CDnsMessage::RECORDTYPE_PTR ) );
                    SendToGroup( query );
                }
                return;
            }

            CORE::CString serviceType = StripLocal( record.name );
            if ( !LooksLikeServiceType( serviceType ) || !IsServiceTypeOfInterest( serviceType ) )
                return;

            CORE::CString key = record.target.Lowercase();
            if ( 0 == record.ttlInSecs )
            {
                ForgetInstance( key );
                return;
            }

            TInstanceMap::iterator i = m_instances.find( key );
            if ( i == m_instances.end() )
            {
                TInstanceInfo& instance = m_instances[ key ];
                CORE::CString rest;
                CDnsMessage::SplitFirstLabel( record.target, instance.instanceName, rest );
                instance.serviceType = serviceType;
                instance.fullName = record.target;
                instance.hasSrv = false;
                instance.port = 0;
                instance.hasTxt = false;
                instance.lastResolveQueryInMs = 0;
                instance.reported = false;
                i = m_instances.find( key );
            }
            TInstanceInfo& instance = (*i).second;
            instance.ttlInSecs = record.ttlInSecs;
            instance.expiresAtInMs = nowInMs + record.ttlInSecs * (UInt64) 1000;
            instance.refreshQueried = false;
            touched.insert( key );
            return;
        }
        case CDnsMessage::RECORDTYPE_SRV:
        case CDnsMessage::RECORDTYPE_TXT:
        {
            CORE::CString key = record.name.Lowercase();
            TInstanceMap::iterator i = m_instances.find( key );
            if ( i == m_instances.end() )
                return;
            if ( 0 == record.ttlInSecs )
            {
                ForgetInstance( key );
                return;
            }
            TInstanceInfo& instance = (*i).second;
            if ( CDnsMessage::RECORDTYPE_SRV == record.type )
            {
                instance.hasSrv = true;
                instance.host = record.target;
                instance.port = record.port;
            }
            else
            {
                instance.hasTxt = true;
                instance.texts = record.texts;
            }
            touched.insert( key );
            return;
        }
        default:
            return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::ForgetInstance( const CORE::CString& key )
{GUCEF_TRACE;

    TInstanceMap::iterator i = m_instances.find( key );
    if ( i == m_instances.end() )
        return true;

    bool reported = (*i).second.reported;
    CORE::CString serviceType = (*i).second.serviceType;
    CORE::CString instanceId = (*i).second.reportedInstanceId;
    m_instances.erase( i );

    if ( !reported )
        return true;

    // Goodbyes are reported for the service type as it was browsed
    CORE::CString::StringSet browsed;
    GetBrowsedServiceTypes( browsed );
    CORE::CString::StringSet::iterator b = browsed.begin();
    while ( b != browsed.end() )
    {
        if ( CDnsMessage::NamesAreEqual( StripLocal( (*b) ), serviceType ) )
            return ProcessGoodbye( (*b), instanceId );
        ++b;
    }
    return ProcessGoodbye( serviceType, instanceId );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::UpdateInstance( TInstanceInfo& instance, UInt64 nowInMs )
{GUCEF_TRACE;

    if ( !instance.hasSrv || !instance.hasTxt )
    {
        SendResolveQuery( instance, nowInMs );
        return true;
    }

    THostMap::iterator h = m_hosts.find( instance.host.Lowercase() );
    if ( h == m_hosts.end() || (*h).second.addresses.empty() )
    {
        SendResolveQuery( instance, nowInMs );
        return true;
    }

    // Report it with the service type the way it was browsed for
    CORE::CString serviceType = instance.serviceType;
    CORE::CString::StringSet browsed;
    GetBrowsedServiceTypes( browsed );
    CORE::CString::StringSet::iterator b = browsed.begin();
    while ( b != browsed.end() )
    {
        if ( CDnsMessage::NamesAreEqual( StripLocal( (*b) ), instance.serviceType ) )
        {
            serviceType = (*b);
            break;
        }
        ++b;
    }

    COMCORE::CDiscoveryEntry entry;
    entry.SetServiceType( serviceType );
    entry.SetInstanceName( instance.instanceName );
    entry.SetTtlInSecs( instance.ttlInSecs );

    CORE::CValueList& attributes = entry.GetAttributes();
    CDnsMessage::TextsToAttributes( instance.texts, attributes );
    CORE::CString instanceId = attributes.GetValueAlways( InstanceIdTextKey ).AsString();
    attributes.Delete( InstanceIdTextKey );
    entry.SetApplicationInstanceId( instanceId );

    const GUCEF::vector< UInt32 >& addresses = (*h).second.addresses;
    COMCORE::CIPv4Address address( addresses[ 0 ], COMCORE::CIPv4Address::HostByteOrderToNetworkByteOrder( instance.port ) );
    entry.SetHost( COMCORE::CHostAddress( address, StripLocal( instance.host ) ) );

    CORE::CString scheme = GetEndpointScheme( instance.serviceType );
    CORE::CString::StringVector& endpoints = entry.GetEndpoints();
    for ( size_t i=0; i<addresses.size(); ++i )
    {
        COMCORE::CIPv4Address endpointAddress( addresses[ i ], 0 );
        endpoints.push_back( scheme + "://" + endpointAddress.AddressAsString() + ":" + CORE::UInt16ToString( instance.port ) );
    }

    instance.reported = true;
    instance.reportedInstanceId = instanceId;
    return ProcessHeardEntry( entry );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::OnResponse( const CDnsMessage& response )
{GUCEF_TRACE;

    UInt64 nowInMs = GetTimeInMs();
    CORE::CString::StringSet touched;

    // Host records first, so the instances that need them find them in the same message
    const CDnsMessage::TResourceRecordVector* sections[ 3 ] = { &response.answers, &response.additionals, &response.authorities };
    for ( int pass=0; pass<2; ++pass )
    {
        for ( int s=0; s<3; ++s )
        {
            const CDnsMessage::TResourceRecordVector& records = *sections[ s ];
            for ( size_t i=0; i<records.size(); ++i )
            {
                bool isHostRecord = CDnsMessage::RECORDTYPE_A == records[ i ].type;
                if ( ( 0 == pass ) == isHostRecord )
                    OnRecord( records[ i ], nowInMs, touched );
            }
        }
    }

    CORE::CString::StringSet::iterator t = touched.begin();
    while ( t != touched.end() )
    {
        TInstanceMap::iterator i = m_instances.find( (*t) );
        if ( i != m_instances.end() )
        {
            if ( !UpdateInstance( (*i).second, nowInMs ) )
                return false;
        }
        ++t;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet )
{GUCEF_TRACE;

    CDnsMessage message;
    if ( !message.Deserialize( packet.GetConstBufferPtr(), packet.GetDataSize() ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "MdnsDiscoveryClient(" << GetDiscoveryClientName() << "): Ignoring a malformed message from " << source.AddressAndPortAsString() );
        return true;
    }

    if ( message.IsResponse() )
        return OnResponse( message );
    return OnQuery( source, message );
}

/*-------------------------------------------------------------------------*/

void
CMdnsDiscoveryClient::OnNotify( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_socket )
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent == eventId && GUCEF_NULL != eventData )
        {
            const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
            for ( UInt32 i=0; i<data.packetsReceived; ++i )
            {
                const COMCORE::CUDPSocket::TPacketEntry& packet = data.packets[ i ];
                if ( !OnPacket( packet.sourceAddress, packet.dataBuffer.GetData() ) )
                    return;
            }
        }
        return;
    }

    COMCORE::CDiscoveryClientBase::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    if ( !COMCORE::CDiscoveryClientBase::LoadConfig( config ) )
        return false;

    CORE::CString group = config.GetAttributeValueOrChildValueByName( "group" ).AsString( CORE::CString::Empty, true );
    if ( !group.IsNULLOrEmpty() )
    {
        COMCORE::CIPv4Address groupAddress;
        if ( !groupAddress.SetAddress( group ) || !groupAddress.IsMulticast() )
            return false;
        groupAddress.SetPortInHostByteOrder( m_port );
        m_group = groupAddress;
    }
    SetPort( config.GetAttributeValueOrChildValueByName( "port" ).AsUInt16( m_port, true ) );

    CORE::CString hostName = config.GetAttributeValueOrChildValueByName( "hostName" ).AsString( CORE::CString::Empty, true );
    if ( !hostName.IsNULLOrEmpty() )
        SetHostName( hostName );

    SetMaxQueryIntervalInMs( config.GetAttributeValueOrChildValueByName( "maxQueryIntervalInMs" ).AsUInt32( m_maxQueryIntervalInMs, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMdnsDiscoveryClient::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !COMCORE::CDiscoveryClientBase::SaveConfig( config ) )
        return false;

    config.SetAttribute( "group", m_group.AddressAsString() );
    config.SetAttribute( "port", m_port );
    config.SetAttribute( "hostName", m_hostName );
    config.SetAttribute( "maxQueryIntervalInMs", m_maxQueryIntervalInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMdnsDiscoveryClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMdnsDiscoveryClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
