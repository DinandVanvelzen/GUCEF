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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

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
    , m_dataAccessMutex()
    , m_transmitBuffer( 512, true )
    , m_udpSender( false )
    , m_statsDestination()
    , m_statsInterface()
    , m_statNamePrefix()
    , m_globalTags()
    , m_transmit( false )
    , m_logStats( false )
    , m_dogStatsDEnabled( false )
{GUCEF_TRACE;

    m_statsDestination.SetPortInHostByteOrder( 8125 );
}

/*-------------------------------------------------------------------------*/

CStatsDClient::CStatsDClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CTSGNotifier( pulseGenerator )
    , CGloballyConfigurable()
    , CORE::CIMetricsSystemClient()
    , m_dataAccessMutex()
    , m_transmitBuffer( 512, true )
    , m_udpSender( pulseGenerator, false )
    , m_statsDestination()
    , m_statsInterface()
    , m_statNamePrefix()
    , m_globalTags()
    , m_transmit( false )
    , m_logStats( false )
    , m_dogStatsDEnabled( false )
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
    Transmit( key, delta, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const Int64 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const UInt32 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const UInt64 delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Count( const CString& key, const CORE::CVariant& delta, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "c";
    Transmit( key, delta, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Int32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const UInt32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Int64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const UInt64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Float32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Gauge( const CString& key, const Float64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "g";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const UInt32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const UInt64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const Float32 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Timing( const CString& key, const Float64 value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "ms";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

CString
CStatsDClient::MergeTags( const CString& callTags ) const
{GUCEF_TRACE;

    if ( m_globalTags.IsNULLOrEmpty() )
        return callTags;
    if ( callTags.IsNULLOrEmpty() )
        return m_globalTags;
    return m_globalTags + ',' + callTags;
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SendTransmitBuffer( void ) const
{GUCEF_TRACE;

    // Caller must hold m_dataAccessMutex and have populated m_transmitBuffer
    UInt32 msgSize = m_transmitBuffer.GetDataSize();
    if ( msgSize == 0 )
        return;

    const char* buffer = static_cast< const char* >( m_transmitBuffer.GetConstBufferPtr() );

    #ifdef GUCEF_DEBUG_MODE

    if ( m_logStats )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "StatsDClient:Transmit: " ) + CORE::CString( buffer, (CORE::UInt32) msgSize ) );
    }
    else
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, CORE::CString( "StatsDClient:Transmit: " ) + CORE::CString( buffer, (CORE::UInt32) msgSize ) );
    }

    #else

    if ( m_logStats )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, CORE::CString( "StatsDClient:Transmit: " ) + CORE::CString( buffer, (CORE::UInt32) msgSize ) );
    }

    #endif

    if ( m_transmit )
    {
        if ( (Int32) msgSize != m_udpSender.SendPacketTo( m_statsDestination.GetFirstIPv4Address(), buffer, (UInt16) msgSize ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "StatsDClient:Transmit: Failed to send stat via UDP of " + CORE::ToString( msgSize ) + " bytes" );
        }
    }
}

/*-------------------------------------------------------------------------*/

template < typename valueType >
void
CStatsDClient::Transmit( const CString& key      ,
                         const valueType value   ,
                         const CString& type     ,
                         const Float32 frequency ,
                         const CString& tags     ) const
{GUCEF_TRACE;

    if ( !m_transmit && !m_logStats )
        return;

    if ( !m_udpSender.IsActive() )
    {
        CStatsDClient* thisObj = const_cast< CStatsDClient* >( this );
        if ( !thisObj->Open() )
            return;
    }

    const Float32 epsilon = 0.0001f;
    bool isFrequencyOne = std::fabs( frequency - 1.0f ) < epsilon;

    // Test if one should send or not, according to the frequency rate
    if ( !isFrequencyOne )
    {
        if ( frequency < static_cast<float>( std::rand() ) / RAND_MAX )
        {
            return;
        }
    }

    MT::CScopeMutex lock( m_dataAccessMutex );

    m_transmitBuffer.Clear( true );

    CORE::CString valueAsStr = CORE::ToString( value );
    if ( isFrequencyOne )
    {
        m_transmitBuffer.AppendPrintf( "%s%s:%s|%s", m_statNamePrefix.C_String(), key.C_String(), valueAsStr.C_String(), type.C_String() );
    }
    else
    {
        m_transmitBuffer.AppendPrintf( "%s%s:%s|%s|@%.2f", m_statNamePrefix.C_String(), key.C_String(), valueAsStr.C_String(), type.C_String(), frequency );
    }

    if ( m_dogStatsDEnabled )
    {
        CString mergedTags = MergeTags( tags );
        if ( !mergedTags.IsNULLOrEmpty() )
        {
            m_transmitBuffer.AppendPrintf( "|#%s", mergedTags.C_String() );
        }
    }

    SendTransmitBuffer();
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::Set( const CString& key, const CString& value, const Float32 frequency ) const
{GUCEF_TRACE;

    static const CString statTypeName = "s";
    Transmit( key, value, statTypeName, frequency, CString::Empty );
}

/*-------------------------------------------------------------------------*/

void CStatsDClient::Histogram( const CString& key, const Int32   value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "h";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Histogram( const CString& key, const UInt32  value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "h";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Histogram( const CString& key, const Int64   value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "h";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Histogram( const CString& key, const UInt64  value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "h";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Histogram( const CString& key, const Float32 value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "h";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Histogram( const CString& key, const Float64 value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "h";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

/*-------------------------------------------------------------------------*/

void CStatsDClient::Distribution( const CString& key, const Int32   value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "d";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Distribution( const CString& key, const UInt32  value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "d";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Distribution( const CString& key, const Int64   value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "d";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Distribution( const CString& key, const UInt64  value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "d";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Distribution( const CString& key, const Float32 value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "d";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

void CStatsDClient::Distribution( const CString& key, const Float64 value, const Float32 frequency, const CString& tags ) const
{GUCEF_TRACE;
    static const CString ddTypeName = "d";
    static const CString fallbackTypeName = "ms";
    Transmit( key, value, m_dogStatsDEnabled ? ddTypeName : fallbackTypeName, frequency, tags );
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SendDogStatsDEvent( const CString& title        ,
                                   const CString& text         ,
                                   const CString& tags         ,
                                   const CString& hostname     ,
                                   const CString& alertType    ,
                                   const CString& priority     ,
                                   const CString& aggregationKey ,
                                   Int64          timestamp    ) const
{GUCEF_TRACE;

    if ( !m_dogStatsDEnabled )
        return;
    if ( !m_transmit && !m_logStats )
        return;
    if ( !m_udpSender.IsActive() )
    {
        CStatsDClient* thisObj = const_cast< CStatsDClient* >( this );
        if ( !thisObj->Open() )
            return;
    }

    MT::CScopeMutex lock( m_dataAccessMutex );

    m_transmitBuffer.Clear( true );

    // Wire format: _e{<title.len>,<text.len>}:<title>|<text>
    m_transmitBuffer.AppendPrintf( "_e{%d,%d}:%s|%s",
        (int) title.Length(), (int) text.Length(),
        title.C_String(), text.C_String() );

    if ( timestamp != 0 )
        m_transmitBuffer.AppendPrintf( "|d:%lld", (long long) timestamp );
    if ( !hostname.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|h:%s", hostname.C_String() );
    if ( !aggregationKey.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|k:%s", aggregationKey.C_String() );
    if ( !priority.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|p:%s", priority.C_String() );
    if ( !alertType.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|t:%s", alertType.C_String() );

    CString mergedTags = MergeTags( tags );
    if ( !mergedTags.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|#%s", mergedTags.C_String() );

    SendTransmitBuffer();
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SendDogStatsDServiceCheck( const CString& name     ,
                                          EDataDogServiceCheckStatus status ,
                                          const CString& tags     ,
                                          const CString& hostname ,
                                          const CString& message  ,
                                          Int64          timestamp ) const
{GUCEF_TRACE;

    if ( !m_dogStatsDEnabled )
        return;
    if ( !m_transmit && !m_logStats )
        return;
    if ( !m_udpSender.IsActive() )
    {
        CStatsDClient* thisObj = const_cast< CStatsDClient* >( this );
        if ( !thisObj->Open() )
            return;
    }

    MT::CScopeMutex lock( m_dataAccessMutex );

    m_transmitBuffer.Clear( true );

    // Wire format: _sc|<name>|<status>
    m_transmitBuffer.AppendPrintf( "_sc|%s|%d", name.C_String(), (int) status );

    if ( timestamp != 0 )
        m_transmitBuffer.AppendPrintf( "|d:%lld", (long long) timestamp );
    if ( !hostname.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|h:%s", hostname.C_String() );

    CString mergedTags = MergeTags( tags );
    if ( !mergedTags.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|#%s", mergedTags.C_String() );

    // Message must come last per the DogStatsD spec
    if ( !message.IsNULLOrEmpty() )
        m_transmitBuffer.AppendPrintf( "|m:%s", message.C_String() );

    SendTransmitBuffer();
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SetDogStatsDEnabled( bool enabled )
{GUCEF_TRACE;

    m_dogStatsDEnabled = enabled;
}

/*-------------------------------------------------------------------------*/

bool
CStatsDClient::GetDogStatsDEnabled( void ) const
{GUCEF_TRACE;

    return m_dogStatsDEnabled;
}

/*-------------------------------------------------------------------------*/

void
CStatsDClient::SetGlobalTags( const CString& tags )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataAccessMutex );
    m_globalTags = tags;
}

/*-------------------------------------------------------------------------*/

const CString&
CStatsDClient::GetGlobalTags( void ) const
{GUCEF_TRACE;

    return m_globalTags;
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
    node->SetAttribute( "dogStatsDExtensionsEnabled", m_dogStatsDEnabled );
    node->SetAttribute( "dogStatsDGlobalTags", m_globalTags );
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
        m_dogStatsDEnabled = node->GetAttributeValueOrChildValueByName( "dogStatsDExtensionsEnabled" ).AsBool( m_dogStatsDEnabled, true );
        m_globalTags = node->GetAttributeValueOrChildValueByName( "dogStatsDGlobalTags" ).AsString( m_globalTags, true );
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
