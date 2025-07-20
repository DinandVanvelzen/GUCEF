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

#include <inttypes.h>
#include <cmath>

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_COMCORE_CDNSCACHE_H
#include "gucefCOMCORE_CDnsCache.h"
#define GUCEF_COMCORE_CDNSCACHE_H
#endif /* GUCEF_COMCORE_CDNSCACHE_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefCOM_CStatsDClient.h"

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

const CORE::CString CStatsDClient::Type = "StatsDClient";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CStatsDClient::CStatsDClient( void )
    : CORE::CTSGNotifier()
    , CORE::CGloballyConfigurable()
    , CORE::CIMetricsSystemClient()
    , m_udpSender( false )
    , m_statsDestination()
    , m_statsInterface()
    , m_statNamePrefix()
    , m_transmit( false )
    , m_logStats( false )
{GUCEF_TRACE;

    m_statsDestination.SetPortInHostByteOrder( 8125 );
}

/*-------------------------------------------------------------------------*/

CStatsDClient::CStatsDClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator )
    , CGloballyConfigurable()
    , CORE::CIMetricsSystemClient()
    , m_udpSender( pulseGenerator, false )
    , m_statsDestination()
    , m_statsInterface()
    , m_statNamePrefix()
    , m_transmit( false )
    , m_logStats( false )
{GUCEF_TRACE;

    m_statsDestination.SetPortInHostByteOrder( 8125 );
}

/*-------------------------------------------------------------------------*/

CStatsDClient::~CStatsDClient()
{GUCEF_TRACE;

    m_udpSender.Close();
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Increment( const CString& key, const Float32 frequency ) const
{GUCEF_TRACE;

    Int32 countValue( 1 );
    Count( key, countValue, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Decrement( const CString& key, const Float32 frequency ) const
{GUCEF_TRACE;

    Int32 countValue( -1 );
    Count( key, countValue, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const Int32 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const Int64 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const UInt32 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const UInt64 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const CORE::CVariant& delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Int32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const UInt32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Int64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const UInt64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Float32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Float64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const UInt32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const UInt64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const Float32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const Float64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency );
}

/*-------------------------------------------------------------------------*/

template < typename valueType >
void
CStatsDClient::Transmit( const CString& key      ,
                         const valueType value   ,
                         const CString& type     ,
                         const Float32 frequency ) const
{GUCEF_TRACE;

    if ( !m_transmit && !m_logStats )
        return;

    if ( !m_udpSender.IsActive() )
    {
        CStatsDClient* thisObj = const_cast< CStatsDClient* >( this );
        if ( !thisObj->Open() )
            return;
    }

    const auto isFrequencyOne = []( const Float32 frequency ) noexcept
    {
        constexpr Float32 epsilon{ 0.0001f };
        return std::fabs( frequency - 1.0f ) < epsilon;
    };

    // Test if one should send or not, according to the frequency rate
    if ( !isFrequencyOne( frequency ) )
    {
        if ( frequency < static_cast<float>( std::rand() ) / RAND_MAX )
        {
            return;
        }
    }

    // Prepare the buffer, with a sampling rate if specified different from 1.0f
    char buffer[256];
    Int32 msgSize=0;
    if ( isFrequencyOne( frequency ) )
    {
        // Sampling rate is 1.0f, no need to specify it
        CORE::CString valueAsStr = CORE::ToString( value );
        msgSize = (Int32) std::snprintf( buffer, sizeof(buffer), "%s%s:%s|%s", m_statNamePrefix.C_String(), key.C_String(), valueAsStr.C_String(), type.C_String() );
    }
    else
    {
        // Sampling rate is different from 1.0f, hence specify it
        CORE::CString valueAsStr = CORE::ToString( value );
        msgSize = (Int32) std::snprintf( buffer, sizeof(buffer), "%s%s:%s|%s|@%.2f", m_statNamePrefix.C_String(), key.C_String(), valueAsStr.C_String(), type.C_String(), frequency );
    }

    // Send the message via the UDP sender
    if ( msgSize > 0 )
    {
        #ifdef GUCEF_DEBUG_MODE

        if ( m_logStats )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "StatsDClient:Transmit: " ) + buffer );
        }
        else
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, CORE::CString( "StatsDClient:Transmit: " ) + buffer );
        }

        #else

        if ( m_logStats )
        {
            GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "StatsDClient:Transmit: " ) + buffer );
        }

        #endif

        if ( m_transmit )
        {
            if ( msgSize != m_udpSender.SendPacketTo( m_statsDestination.GetFirstIPv4Address(), buffer, (UInt16) msgSize ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:Transmit: Failed to send stat via UDP of " + CORE::ToString( msgSize ) + " bytes: " + CORE::CString( buffer ) );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CStatsDClient::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* node = tree.Find( "StatsDClient" );
    if ( GUCEF_NULL == node )
    {
        node = tree.AddChild( "StatsDClient" );
        if ( GUCEF_NULL == node )
            return false;
    }

    node->SetAttribute( "statsDestination", m_statsDestination.HostnameAndPortAsString() );
    node->SetAttribute( "statsNamePrefix", m_statNamePrefix );
    node->SetAttribute( "statsInterface", m_statsInterface.AddressAndPortAsString() );
    node->SetAttribute( "transmit", m_transmit );
    node->SetAttribute( "logStats", m_logStats );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CStatsDClient::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* node = treeroot.Find( "StatsDClient" );
    if ( GUCEF_NULL != node )
    {
        // First load the mandatory settings
        CORE::CVariant value;
        value = node->GetAttributeValueOrChildValueByName( "statsDestination" );
        if ( !value.IsNULLOrEmpty() )
        {
            COMCORE::CHostAddress dest( value.AsString() );
            if ( !SetStatsDestination( dest ) )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:LoadConfig(): Failed set host and port to: " + value.AsString( CString::Empty, true ) );
                return false;
            }
        }
        else
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:LoadConfig(): Could not find mandatory host and port info in StatsDClient config, StatsD functionality will not be available" );
            return false;
        }

        // Load extra settings for which the default can also be good enough
        m_statNamePrefix = node->GetAttributeValueOrChildValueByName( "statsNamePrefix" ).AsString( m_statNamePrefix, true );
        m_statsInterface.SetAddressAndPort( node->GetAttributeValueOrChildValueByName( "statsInterface" ).AsString( m_statsInterface.AddressAndPortAsString(), true ) );
        m_transmit = node->GetAttributeValueOrChildValueByName( "transmit" ).AsBool( m_transmit, true );
        m_logStats = node->GetAttributeValueOrChildValueByName( "logStats" ).AsBool( m_logStats, true );
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:LoadConfig(): Could not find StatsDClient config, StatsD functionality will not be available" );
        return false;
    }

    // (Re?)Open to make settings effective
    Open();

    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStatsDClient::GetName( void ) const
{GUCEF_TRACE;

    return Type;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CStatsDClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::COM::CStatsDClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CStatsDClient::Open( void )
{GUCEF_TRACE;

    if ( m_udpSender.IsActive() )
        m_udpSender.Close();

    if ( m_udpSender.Open( m_statsInterface ) )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:Open(): Successfully opened socket at " + m_statsInterface.AddressAsString() + ':' + CORE::ToString( m_udpSender.GetPort() ) );
        return true;
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:Open(): Failed to open socket at " + m_statsInterface.AddressAndPortAsString() );
        return false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CStatsDClient::Close( void )
{GUCEF_TRACE;

    m_udpSender.Close();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::OnStatsDTargetDnsChange( CORE::CNotifier* notifier    ,
                                        const CORE::CEvent& eventId  ,
                                        CORE::CICloneable* eventdata )
{GUCEF_TRACE;

    COMCORE::CHostAddress oldStatsDestination( m_statsDestination );
    COMCORE::CDnsCacheEntry* dnsCacheEntry = static_cast< COMCORE::CDnsCacheEntry* >( notifier );
    if ( dnsCacheEntry->GetPrivateCopy( m_statsDestination, false ) )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:OnStatsDTargetDnsChange: Updated underlying for DNS target " + m_statsDestination.GetHostname() +
                                                     " from IPv4 \"" + oldStatsDestination.GetIPv4AddressesAsString() + 
                                                     "\" to IPv4 \"" + m_statsDestination.GetIPv4AddressesAsString() + "\"" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:OnStatsDTargetDnsChange: Failed to update DNS target for " + m_statsDestination.GetHostname() +
                                                     " from IPv4 \"" + oldStatsDestination.GetIPv4AddressesAsString() + 
                                                     "\" to IPv4 \"" + m_statsDestination.GetIPv4AddressesAsString() + "\"" );
    }
}

/*-------------------------------------------------------------------------*/

bool
CStatsDClient::SetStatsDestination( const COMCORE::CHostAddress& dest )
{GUCEF_TRACE;

    m_statsDestination = dest;
    
    if ( m_statsDestination.HasDnsBasedHostname() )
    {    
        COMCORE::CDnsCacheEntryPtr dnsEntry = COMCORE::CComCoreGlobal::Instance()->GetDnsCache().GetOrAddCacheEntryForDns( dest.GetHostname() );
        if ( !dnsEntry.IsNULL() )
        {
            TEventCallback callback( this, &CStatsDClient::OnStatsDTargetDnsChange );
            if ( SubscribeTo( dnsEntry.GetPointerAlways()                  , 
                              COMCORE::CDnsCacheEntry::DnsInfoChangedEvent ,
                              callback                                     ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient: Subscribed to dns cache entry for " + dest.GetHostname() );
                return true;
            }
            else
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient: Failed to subscribe to dns cache entry for " + dest.GetHostname() );
            }
        }
        else
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient: Failed to obtain dns cache entry for " + dest.GetHostname() );
        }

        return false;
    }
    else
    {
        return true;
    }
}

/*-------------------------------------------------------------------------*/

const COMCORE::CHostAddress&
CStatsDClient::GetStatsDestination( void ) const
{GUCEF_TRACE;

    return m_statsDestination;
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SetStatsInterface( const COMCORE::CHostAddress& interface )
{GUCEF_TRACE;

    m_statsInterface = interface.GetFirstIPv4Address();
}

/*-------------------------------------------------------------------------*/

const COMCORE::CIPv4Address&
CStatsDClient::GetStatsInterface( void ) const
{GUCEF_TRACE;

    return m_statsInterface;
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SetStatNamePrefix( const CString& prefix )
{GUCEF_TRACE;

    m_statNamePrefix = prefix;
}

/*-------------------------------------------------------------------------*/

const CString&
CStatsDClient::GetStatNamePrefix( void ) const
{GUCEF_TRACE;

    return m_statNamePrefix;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace COM */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
