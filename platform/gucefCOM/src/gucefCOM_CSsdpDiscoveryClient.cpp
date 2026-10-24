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

#ifndef GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#include "gucefCOM_CUdpDiscoveryClientBase.h"
#define GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H
#endif /* GUCEF_COM_CUDPDISCOVERYCLIENTBASE_H ? */

#include "gucefCOM_CSsdpDiscoveryClient.h"

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

const CORE::CString CSsdpDiscoveryClient::TypeName = "ssdp";
const CORE::CString CSsdpDiscoveryClient::DefaultGroup = "239.255.255.250";
const UInt16 CSsdpDiscoveryClient::DefaultPort = 1900;

static const char* SearchAll = "ssdp:all";
static const char* ServerHeaderValue = "GUCEF/1.0 UPnP/1.1 GUCEF/1.0";
static const char* AttributeHeaderPrefix = "X-GUCEF-ATTR-";
static const UInt32 FirstSearchIntervalInMs = 1000;
static const UInt32 MaxDatagramSize = 1400;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

bool
EqualsIgnoringCase( const char* a, const char* b, UInt32 size )
{GUCEF_TRACE;

    for ( UInt32 i=0; i<size; ++i )
    {
        char ca = a[ i ];
        char cb = b[ i ];
        if ( ca >= 'A' && ca <= 'Z' ) ca = (char)( ca - 'A' + 'a' );
        if ( cb >= 'A' && cb <= 'Z' ) cb = (char)( cb - 'A' + 'a' );
        if ( ca != cb )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
AppendHeader( CORE::CString& message, const char* name, const CORE::CString& value )
{GUCEF_TRACE;

    message += name;
    message += ": ";
    message += value;
    message += "\r\n";
}

/*-------------------------------------------------------------------------*/

/** Puts in the address the message came from for endpoint hosts the advertiser did not know */
void
ResolveEndpointHost( CORE::CString& endpoint, const CORE::CString& sourceHost )
{GUCEF_TRACE;

    Int32 schemeEnd = endpoint.HasSubstr( "://", true );
    UInt32 hostStart = schemeEnd >= 0 ? (UInt32) schemeEnd + 3 : 0;
    CORE::CString rest = endpoint.CutChars( hostStart, true, 0 );
    if ( rest.StartsWith( "0.0.0.0" ) )
        endpoint = endpoint.SubstrToIndex( hostStart, true ) + sourceHost + rest.CutChars( 7, true, 0 );
    else
    if ( rest.StartsWith( ":" ) )
        endpoint = endpoint.SubstrToIndex( hostStart, true ) + sourceHost + rest;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSsdpDiscoveryClient::CSsdpDiscoveryClient( void )
    : COMCORE::CDiscoveryClientBase( TypeName, CORE::PulseGeneratorPtr() )
    , m_groupSocket( GetPulseGenerator(), false )
    , m_searchSocket( GetPulseGenerator(), false )
    , m_group( DefaultGroup, DefaultPort )
    , m_port( DefaultPort )
    , m_maxSearchIntervalInMs( 60000 )
    , m_searchSchedules()
    , m_nextAnnounceTimes()
{GUCEF_TRACE;

    m_groupSocket.SetNrOfReceiveBuffers( 4, 2048 );
    m_searchSocket.SetNrOfReceiveBuffers( 4, 2048 );
    SubscribeTo( &m_groupSocket );
    SubscribeTo( &m_searchSocket );
}

/*-------------------------------------------------------------------------*/

CSsdpDiscoveryClient::CSsdpDiscoveryClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : COMCORE::CDiscoveryClientBase( TypeName, pulseGenerator )
    , m_groupSocket( GetPulseGenerator(), false )
    , m_searchSocket( GetPulseGenerator(), false )
    , m_group( DefaultGroup, DefaultPort )
    , m_port( DefaultPort )
    , m_maxSearchIntervalInMs( 60000 )
    , m_searchSchedules()
    , m_nextAnnounceTimes()
{GUCEF_TRACE;

    m_groupSocket.SetNrOfReceiveBuffers( 4, 2048 );
    m_searchSocket.SetNrOfReceiveBuffers( 4, 2048 );
    SubscribeTo( &m_groupSocket );
    SubscribeTo( &m_searchSocket );
}

/*-------------------------------------------------------------------------*/

CSsdpDiscoveryClient::~CSsdpDiscoveryClient()
{GUCEF_TRACE;

    StopAll();
    UnsubscribeFrom( &m_groupSocket );
    UnsubscribeFrom( &m_searchSocket );
    m_groupSocket.Close();
    m_searchSocket.Close();
}

/*-------------------------------------------------------------------------*/

void
CSsdpDiscoveryClient::SetPort( UInt16 port )
{GUCEF_TRACE;

    if ( port != m_port )
    {
        m_port = port;
        m_group.SetPortInHostByteOrder( port );
        if ( m_groupSocket.IsActive() )
        {
            m_groupSocket.Close();
            m_searchSocket.Close();
            EnsureSocketsAreOpen();
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt16
CSsdpDiscoveryClient::GetPort( void ) const
{GUCEF_TRACE;

    return m_port;
}

/*-------------------------------------------------------------------------*/

void
CSsdpDiscoveryClient::SetMaxSearchIntervalInMs( UInt32 maxSearchIntervalInMs )
{GUCEF_TRACE;

    m_maxSearchIntervalInMs = maxSearchIntervalInMs > FirstSearchIntervalInMs ? maxSearchIntervalInMs : FirstSearchIntervalInMs;
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::IsActive( void ) const
{GUCEF_TRACE;

    return m_groupSocket.IsActive();
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::EnsureSocketsAreOpen( void )
{GUCEF_TRACE;

    if ( !m_groupSocket.IsActive() )
    {
        m_groupSocket.SetMulticastTTL( 2 );
        m_groupSocket.SetAllowMulticastLoopback( true );
        if ( !m_groupSocket.Open( m_port ) )
        {
            ReportDiscoveryError( "Unable to open the SSDP socket on port " + CORE::UInt16ToString( m_port ) );
            return false;
        }
        if ( 0 == CUdpDiscoveryClientBase::JoinOnAllInterfaces( m_groupSocket, m_group ) )
        {
            ReportDiscoveryError( "Unable to join the SSDP group " + m_group.AddressAsString() + " on any network interface" );
            m_groupSocket.Close();
            return false;
        }
    }
    if ( !m_searchSocket.IsActive() )
    {
        m_searchSocket.SetMulticastTTL( 2 );
        m_searchSocket.SetAllowMulticastLoopback( true );
        if ( !m_searchSocket.Open( (UInt16) 0 ) )
        {
            ReportDiscoveryError( "Unable to open the SSDP search socket" );
            return false;
        }
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::Send( COMCORE::CUDPSocket& socket, const CORE::CString& message, const COMCORE::CIPv4Address& destination )
{GUCEF_TRACE;

    UInt32 size = message.ByteSize() > 0 ? message.ByteSize() - 1 : 0;
    if ( size > MaxDatagramSize )
    {
        ReportDiscoveryError( "An SSDP message of " + CORE::UInt32ToString( size ) + " bytes is too large to send" );
        return false;
    }
    Int32 sent = socket.SendPacketTo( destination, message.C_String(), (UInt16) size );
    if ( sent != (Int32) size )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "SsdpDiscoveryClient(" << GetDiscoveryClientName() << "): Failed to send to " << destination.AddressAndPortAsString() );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSsdpDiscoveryClient::MakeUsn( const COMCORE::CIDiscoveryEntry& entry )
{GUCEF_TRACE;

    CORE::CString instanceId;
    entry.GetApplicationInstanceId( instanceId );
    if ( instanceId.IsNULLOrEmpty() )
        instanceId = entry.GetInstanceName();
    return "uuid:" + instanceId + "::" + entry.GetServiceType();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSsdpDiscoveryClient::GetInstanceIdFromUsn( const CORE::CString& usn )
{GUCEF_TRACE;

    CORE::CString id = usn;
    if ( id.Lowercase().StartsWith( "uuid:" ) )
        id = id.CutChars( 5, true, 0 );
    Int32 separator = id.HasSubstr( "::", true );
    if ( separator >= 0 )
        id = id.SubstrToIndex( (UInt32) separator, true );
    return id;
}

/*-------------------------------------------------------------------------*/

void
CSsdpDiscoveryClient::AppendEntryHeaders( const COMCORE::CIDiscoveryEntry& entry, CORE::CString& message ) const
{GUCEF_TRACE;

    const CORE::CString::StringVector& endpoints = entry.GetEndpoints();
    if ( !endpoints.empty() )
        AppendHeader( message, "LOCATION", endpoints[ 0 ] );
    AppendHeader( message, "SERVER", ServerHeaderValue );
    AppendHeader( message, "USN", MakeUsn( entry ) );
    if ( !entry.GetInstanceName().IsNULLOrEmpty() )
        AppendHeader( message, "X-GUCEF-NAME", entry.GetInstanceName() );
    for ( size_t i=0; i<endpoints.size(); ++i )
        AppendHeader( message, "X-GUCEF-ENDPOINT", endpoints[ i ] );

    const CORE::CValueList& attributes = entry.GetAttributes();
    UInt32 keyCount = attributes.GetKeyCount();
    for ( UInt32 k=0; k<keyCount; ++k )
    {
        CORE::CString headerName = AttributeHeaderPrefix + attributes.GetKey( k ).AsString();
        const CORE::CValueList::TVariantVector& values = attributes.GetValueVector( k );
        for ( size_t v=0; v<values.size(); ++v )
            AppendHeader( message, headerName.C_String(), values[ v ].AsString() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::SendSearch( const CORE::CString& serviceType )
{GUCEF_TRACE;

    if ( !EnsureSocketsAreOpen() )
        return false;

    CORE::CString message = "M-SEARCH * HTTP/1.1\r\n";
    AppendHeader( message, "HOST", m_group.AddressAndPortAsString() );
    AppendHeader( message, "MAN", "\"ssdp:discover\"" );
    AppendHeader( message, "MX", "1" );
    AppendHeader( message, "ST", serviceType == COMCORE::CIDiscoveryClient::AnyServiceType ? CORE::CString( SearchAll ) : serviceType );
    message += "\r\n";
    return Send( m_searchSocket, message, m_group );
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::SendNotify( const COMCORE::CIDiscoveryEntry& entry, bool alive )
{GUCEF_TRACE;

    if ( !EnsureSocketsAreOpen() )
        return false;

    CORE::CString message = "NOTIFY * HTTP/1.1\r\n";
    AppendHeader( message, "HOST", m_group.AddressAndPortAsString() );
    AppendHeader( message, "NT", entry.GetServiceType() );
    AppendHeader( message, "NTS", alive ? "ssdp:alive" : "ssdp:byebye" );
    if ( alive )
    {
        AppendHeader( message, "CACHE-CONTROL", "max-age=" + CORE::UInt32ToString( entry.GetTtlInSecs() ) );
        AppendEntryHeaders( entry, message );
    }
    else
    {
        AppendHeader( message, "USN", MakeUsn( entry ) );
    }
    message += "\r\n";
    return Send( m_groupSocket, message, m_group );
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::SendSearchResponse( const COMCORE::CIDiscoveryEntry& entry, const COMCORE::CIPv4Address& destination )
{GUCEF_TRACE;

    CORE::CString message = "HTTP/1.1 200 OK\r\n";
    AppendHeader( message, "CACHE-CONTROL", "max-age=" + CORE::UInt32ToString( entry.GetTtlInSecs() ) );
    AppendHeader( message, "EXT", CORE::CString::Empty );
    AppendHeader( message, "ST", entry.GetServiceType() );
    AppendEntryHeaders( entry, message );
    message += "\r\n";
    return Send( m_searchSocket, message, destination );
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::OnBrowseStarted( const CORE::CString& serviceType )
{GUCEF_TRACE;

    if ( !EnsureSocketsAreOpen() )
        return false;

    UInt64 nowInMs = GetTimeInMs();
    TSearchSchedule& schedule = m_searchSchedules[ serviceType ];
    schedule.intervalInMs = FirstSearchIntervalInMs;
    schedule.nextSearchInMs = nowInMs + FirstSearchIntervalInMs;
    SendSearch( serviceType );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSsdpDiscoveryClient::OnBrowseStopped( const CORE::CString& serviceType )
{GUCEF_TRACE;

    m_searchSchedules.erase( serviceType );
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::OnAdvertised( const COMCORE::CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    if ( !EnsureSocketsAreOpen() )
        return false;

    UInt32 ttl = entry->GetTtlInSecs();
    m_nextAnnounceTimes[ entry->GetEntryKey() ] = GetTimeInMs() + ( ttl > 0 ? ( ttl * (UInt64) 1000 ) / 3 : (UInt64) 1000 );
    SendNotify( *entry, true );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::OnWithdrawn( const COMCORE::CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    m_nextAnnounceTimes.erase( entry->GetEntryKey() );
    if ( !m_groupSocket.IsActive() )
        return true;
    return SendNotify( *entry, false );
}

/*-------------------------------------------------------------------------*/

void
CSsdpDiscoveryClient::OnHousekeeping( UInt64 nowInMs )
{GUCEF_TRACE;

    TIDiscoveryEntryPtrVector advertised;
    GetAdvertisedEntries( advertised );
    for ( size_t i=0; i<advertised.size(); ++i )
    {
        TAnnounceTimeMap::iterator n = m_nextAnnounceTimes.find( advertised[ i ]->GetEntryKey() );
        if ( n == m_nextAnnounceTimes.end() || (*n).second <= nowInMs )
        {
            UInt32 ttl = advertised[ i ]->GetTtlInSecs();
            m_nextAnnounceTimes[ advertised[ i ]->GetEntryKey() ] = nowInMs + ( ttl > 0 ? ( ttl * (UInt64) 1000 ) / 3 : (UInt64) 1000 );
            SendNotify( *advertised[ i ], true );
        }
    }

    TSearchScheduleMap::iterator s = m_searchSchedules.begin();
    while ( s != m_searchSchedules.end() )
    {
        TSearchSchedule& schedule = (*s).second;
        if ( schedule.nextSearchInMs <= nowInMs )
        {
            schedule.intervalInMs = schedule.intervalInMs * 2 > m_maxSearchIntervalInMs ? m_maxSearchIntervalInMs : schedule.intervalInMs * 2;
            schedule.nextSearchInMs = nowInMs + schedule.intervalInMs;
            SendSearch( (*s).first );
        }
        ++s;
    }
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::ParseMessage( const CORE::CDynamicBuffer& packet, CORE::CString& startLine, THeaderVector& headers )
{GUCEF_TRACE;

    const char* data = static_cast< const char* >( packet.GetConstBufferPtr() );
    UInt32 size = packet.GetDataSize();
    UInt32 lineStart = 0;
    bool first = true;
    for ( UInt32 i=0; i<=size; ++i )
    {
        if ( i < size && '\n' != data[ i ] )
            continue;

        UInt32 lineEnd = i;
        if ( lineEnd > lineStart && '\r' == data[ lineEnd-1 ] )
            --lineEnd;
        UInt32 lineSize = lineEnd - lineStart;
        const char* line = data + lineStart;
        lineStart = i + 1;

        if ( first )
        {
            if ( 0 == lineSize )
                return false;
            startLine.Set( line, lineSize );
            first = false;
            continue;
        }
        if ( 0 == lineSize )
            break;

        UInt32 colon = 0;
        while ( colon < lineSize && ':' != line[ colon ] )
            ++colon;
        if ( colon == lineSize )
            continue;

        UInt32 valueStart = colon + 1;
        while ( valueStart < lineSize && ( ' ' == line[ valueStart ] || '\t' == line[ valueStart ] ) )
            ++valueStart;
        UInt32 valueEnd = lineSize;
        while ( valueEnd > valueStart && ( ' ' == line[ valueEnd-1 ] || '\t' == line[ valueEnd-1 ] ) )
            --valueEnd;

        headers.push_back( THeader() );
        headers.back().name.Set( line, colon );
        if ( valueEnd > valueStart )
            headers.back().value.Set( line + valueStart, valueEnd - valueStart );
    }
    return !first;
}

/*-------------------------------------------------------------------------*/

const CORE::CString*
CSsdpDiscoveryClient::FindHeader( const THeaderVector& headers, const char* name )
{GUCEF_TRACE;

    UInt32 nameSize = (UInt32) strlen( name );
    for ( size_t i=0; i<headers.size(); ++i )
    {
        const CORE::CString& headerName = headers[ i ].name;
        if ( headerName.ByteSize() == nameSize + 1 && EqualsIgnoringCase( headerName.C_String(), name, nameSize ) )
            return &headers[ i ].value;
    }
    return GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::OnSearch( const COMCORE::CIPv4Address& source, const THeaderVector& headers )
{GUCEF_TRACE;

    const CORE::CString* st = FindHeader( headers, "ST" );
    if ( GUCEF_NULL == st )
        return true;

    bool all = (*st) == SearchAll;
    TIDiscoveryEntryPtrVector advertised;
    GetAdvertisedEntries( advertised );
    for ( size_t i=0; i<advertised.size(); ++i )
    {
        if ( all || advertised[ i ]->GetServiceType() == (*st) )
            SendSearchResponse( *advertised[ i ], source );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::OnAnnouncement( const COMCORE::CIPv4Address& source ,
                                      const THeaderVector& headers         ,
                                      const CORE::CString& serviceType     ,
                                      bool alive                           )
{GUCEF_TRACE;

    if ( serviceType.IsNULLOrEmpty() || !IsServiceTypeBrowsed( serviceType ) )
        return true;

    const CORE::CString* usn = FindHeader( headers, "USN" );
    CORE::CString instanceId = GUCEF_NULL != usn ? GetInstanceIdFromUsn( *usn ) : CORE::CString::Empty;

    if ( !alive )
        return ProcessGoodbye( serviceType, instanceId );

    COMCORE::CDiscoveryEntry entry;
    entry.SetServiceType( serviceType );
    entry.SetApplicationInstanceId( instanceId );

    const CORE::CString* name = FindHeader( headers, "X-GUCEF-NAME" );
    entry.SetInstanceName( GUCEF_NULL != name ? *name : ( GUCEF_NULL != usn ? *usn : instanceId ) );

    UInt32 ttl = 1800;
    const CORE::CString* cacheControl = FindHeader( headers, "CACHE-CONTROL" );
    if ( GUCEF_NULL != cacheControl )
    {
        Int32 maxAge = cacheControl->Lowercase().HasSubstr( "max-age", true );
        if ( maxAge >= 0 )
        {
            CORE::CString value = cacheControl->CutChars( (UInt32) maxAge + 7, true, 0 ).Trim( true );
            if ( value.StartsWith( "=" ) )
                ttl = CORE::StringToUInt32( value.CutChars( 1, true, 0 ).Trim( true ).SubstrToChar( ',', true ), ttl );
        }
    }
    entry.SetTtlInSecs( ttl );

    CORE::CString sourceHost = source.AddressAsString();
    entry.SetHost( COMCORE::CHostAddress( source, sourceHost ) );

    CORE::CString::StringVector& endpoints = entry.GetEndpoints();
    CORE::CValueList& attributes = entry.GetAttributes();
    attributes.SetAllowMultipleValues( true );
    UInt32 prefixSize = (UInt32) strlen( AttributeHeaderPrefix );
    for ( size_t i=0; i<headers.size(); ++i )
    {
        const CORE::CString& headerName = headers[ i ].name;
        if ( headerName.ByteSize() == 17 && EqualsIgnoringCase( headerName.C_String(), "X-GUCEF-ENDPOINT", 16 ) )
        {
            CORE::CString endpoint = headers[ i ].value;
            ResolveEndpointHost( endpoint, sourceHost );
            endpoints.push_back( endpoint );
        }
        else
        if ( headerName.ByteSize() > prefixSize + 1 && EqualsIgnoringCase( headerName.C_String(), AttributeHeaderPrefix, prefixSize ) )
        {
            attributes.Set( headerName.CutChars( prefixSize, true, 0 ), headers[ i ].value );
        }
    }

    // Others than GUCEF only have the location
    if ( endpoints.empty() )
    {
        const CORE::CString* location = FindHeader( headers, "LOCATION" );
        if ( GUCEF_NULL != location )
        {
            CORE::CString endpoint = *location;
            ResolveEndpointHost( endpoint, sourceHost );
            endpoints.push_back( endpoint );
        }
        const CORE::CString* server = FindHeader( headers, "SERVER" );
        if ( GUCEF_NULL != server )
            attributes.Set( "server", *server );
    }

    return ProcessHeardEntry( entry );
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::OnPacket( const COMCORE::CIPv4Address& source, const CORE::CDynamicBuffer& packet, bool fromSearchSocket )
{GUCEF_TRACE;

    CORE::CString startLine;
    THeaderVector headers;
    if ( !ParseMessage( packet, startLine, headers ) )
        return true;

    CORE::CString method = startLine.SubstrToChar( ' ', true ).Uppercase();
    if ( method == "M-SEARCH" )
    {
        return OnSearch( source, headers );
    }
    if ( method == "NOTIFY" )
    {
        const CORE::CString* nt = FindHeader( headers, "NT" );
        const CORE::CString* nts = FindHeader( headers, "NTS" );
        if ( GUCEF_NULL == nt || GUCEF_NULL == nts )
            return true;
        CORE::CString subType = nts->Lowercase();
        if ( subType == "ssdp:alive" )
            return OnAnnouncement( source, headers, *nt, true );
        if ( subType == "ssdp:byebye" )
            return OnAnnouncement( source, headers, *nt, false );
        return true;
    }
    if ( method.StartsWith( "HTTP/" ) )
    {
        const CORE::CString* st = FindHeader( headers, "ST" );
        if ( GUCEF_NULL != st )
            return OnAnnouncement( source, headers, *st, true );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSsdpDiscoveryClient::OnNotify( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_groupSocket || notifier == &m_searchSocket )
    {
        if ( COMCORE::CUDPSocket::UDPPacketsRecievedEvent == eventId && GUCEF_NULL != eventData )
        {
            bool fromSearchSocket = notifier == &m_searchSocket;
            const COMCORE::CUDPSocket::TUdpPacketsRecievedEventData& data = static_cast< COMCORE::CUDPSocket::UDPPacketsRecievedEventData* >( eventData )->GetData();
            for ( UInt32 i=0; i<data.packetsReceived; ++i )
            {
                const COMCORE::CUDPSocket::TPacketEntry& packet = data.packets[ i ];
                if ( !OnPacket( packet.sourceAddress, packet.dataBuffer.GetData(), fromSearchSocket ) )
                    return;
            }
        }
        return;
    }

    COMCORE::CDiscoveryClientBase::OnNotify( notifier, eventId, eventData );
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::LoadConfig( const CORE::CDataNode& config )
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
    SetMaxSearchIntervalInMs( config.GetAttributeValueOrChildValueByName( "maxSearchIntervalInMs" ).AsUInt32( m_maxSearchIntervalInMs, true ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CSsdpDiscoveryClient::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    if ( !COMCORE::CDiscoveryClientBase::SaveConfig( config ) )
        return false;

    config.SetAttribute( "group", m_group.AddressAsString() );
    config.SetAttribute( "port", m_port );
    config.SetAttribute( "maxSearchIntervalInMs", m_maxSearchIntervalInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSsdpDiscoveryClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CSsdpDiscoveryClient";
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
