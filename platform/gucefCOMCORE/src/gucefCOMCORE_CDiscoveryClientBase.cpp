/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "gucefCOMCORE_CDiscoveryClientBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      CONSTANTS                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

static const UInt32 DefaultHousekeepingIntervalInMs = 250;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CDiscoveryClientBase::CDiscoveryClientBase( const CORE::CString& defaultClientName )
    : CIDiscoveryClient()
    , m_clientName( defaultClientName )
    , m_pulseGenerator( CORE::CCoreGlobal::Instance()->GetPulseGenerator() )
    , m_housekeepingTimer( m_pulseGenerator, DefaultHousekeepingIntervalInMs )
    , m_browsedServiceTypes()
    , m_advertised()
    , m_discovered()
    , m_dataLock()
{GUCEF_TRACE;

    SubscribeTo( &m_housekeepingTimer );
}

/*-------------------------------------------------------------------------*/

CDiscoveryClientBase::CDiscoveryClientBase( const CORE::CString& defaultClientName      ,
                                            const CORE::PulseGeneratorPtr& pulseGenerator )
    : CIDiscoveryClient()
    , m_clientName( defaultClientName )
    , m_pulseGenerator( pulseGenerator.IsNULL() ? CORE::CCoreGlobal::Instance()->GetPulseGenerator() : pulseGenerator )
    , m_housekeepingTimer( m_pulseGenerator, DefaultHousekeepingIntervalInMs )
    , m_browsedServiceTypes()
    , m_advertised()
    , m_discovered()
    , m_dataLock()
{GUCEF_TRACE;

    SubscribeTo( &m_housekeepingTimer );
}

/*-------------------------------------------------------------------------*/

CDiscoveryClientBase::~CDiscoveryClientBase()
{GUCEF_TRACE;

    m_housekeepingTimer.SetEnabled( false );
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

UInt64
CDiscoveryClientBase::GetTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------*/

const CORE::PulseGeneratorPtr&
CDiscoveryClientBase::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDiscoveryClientBase::GetDiscoveryClientName( void ) const
{GUCEF_TRACE;

    return m_clientName;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::SetDiscoveryClientName( const CORE::CString& clientName )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    m_clientName = clientName;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::UpdateHousekeepingTimer( void )
{GUCEF_TRACE;

    bool needed = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        needed = !m_browsedServiceTypes.empty() || !m_advertised.empty() || !m_discovered.empty();
    }
    if ( needed != m_housekeepingTimer.GetEnabled() )
        m_housekeepingTimer.SetEnabled( needed );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::Browse( const CORE::CString& serviceType )
{GUCEF_TRACE;

    if ( serviceType.IsNULLOrEmpty() )
        return false;

    bool isNew = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        isNew = m_browsedServiceTypes.insert( serviceType ).second;
    }
    UpdateHousekeepingTimer();

    if ( !OnBrowseStarted( serviceType ) )
    {
        MT::CScopeMutex lock( m_dataLock );
        if ( isNew )
            m_browsedServiceTypes.erase( serviceType );
        return false;
    }

    if ( isNew )
    {
        TBrowseEventData eData( serviceType );
        if ( !NotifyObservers( BrowseStartedEvent, &eData ) ) return true;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::StopBrowse( const CORE::CString& serviceType )
{GUCEF_TRACE;

    TIDiscoveryEntryPtrVector lostEntries;
    {
        MT::CScopeMutex lock( m_dataLock );
        if ( 0 == m_browsedServiceTypes.erase( serviceType ) )
            return false;

        // What was found for a service type that is no longer browsed is forgotten
        TDiscoveredEntryMap::iterator i = m_discovered.begin();
        while ( i != m_discovered.end() )
        {
            if ( !IsServiceTypeBrowsed( (*i).second.entry->GetServiceType() ) )
            {
                lostEntries.push_back( CIDiscoveryEntryPtr( (*i).second.entry ) );
                m_discovered.erase( i++ );
            }
            else
                ++i;
        }
    }

    OnBrowseStopped( serviceType );
    UpdateHousekeepingTimer();

    for ( size_t n=0; n<lostEntries.size(); ++n )
    {
        CDiscoveryEntryEventData eData( lostEntries[ n ] );
        if ( !NotifyObservers( EntryLostEvent, &eData ) ) return true;
    }

    TBrowseEventData eData( serviceType );
    NotifyObservers( BrowseStoppedEvent, &eData );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::IsBrowsing( const CORE::CString& serviceType ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    if ( serviceType.IsNULLOrEmpty() )
        return !m_browsedServiceTypes.empty();
    return m_browsedServiceTypes.find( serviceType ) != m_browsedServiceTypes.end();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::IsServiceTypeBrowsed( const CORE::CString& serviceType ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    if ( m_browsedServiceTypes.find( serviceType ) != m_browsedServiceTypes.end() )
        return true;
    return m_browsedServiceTypes.find( AnyServiceType ) != m_browsedServiceTypes.end();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::IsOwnEntry( const CORE::CString& entryKey ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    return m_advertised.find( entryKey ) != m_advertised.end();
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::GetAdvertisedEntries( TIDiscoveryEntryPtrVector& entries ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    entries.reserve( entries.size() + m_advertised.size() );
    TAdvertisedEntryMap::const_iterator i = m_advertised.begin();
    while ( i != m_advertised.end() )
    {
        entries.push_back( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::GetBrowsedServiceTypes( CORE::CString::StringSet& serviceTypes ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    serviceTypes.insert( m_browsedServiceTypes.begin(), m_browsedServiceTypes.end() );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::Advertise( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    if ( entry.IsNULL() || entry->GetServiceType().IsNULLOrEmpty() )
        return false;

    CORE::CString key = entry->GetEntryKey();
    {
        MT::CScopeMutex lock( m_dataLock );
        m_advertised[ key ] = entry;
    }
    UpdateHousekeepingTimer();

    if ( !OnAdvertised( entry ) )
    {
        MT::CScopeMutex lock( m_dataLock );
        m_advertised.erase( key );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::Withdraw( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    if ( entry.IsNULL() )
        return false;

    CORE::CString key = entry->GetEntryKey();
    CIDiscoveryEntryPtr advertised;
    {
        MT::CScopeMutex lock( m_dataLock );
        TAdvertisedEntryMap::iterator i = m_advertised.find( key );
        if ( i == m_advertised.end() )
            return false;
        advertised = (*i).second;
        m_advertised.erase( i );
    }

    bool success = OnWithdrawn( advertised );
    UpdateHousekeepingTimer();
    return success;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::StopAll( void )
{GUCEF_TRACE;

    TIDiscoveryEntryPtrVector advertised;
    GetAdvertisedEntries( advertised );
    for ( size_t i=0; i<advertised.size(); ++i )
        Withdraw( advertised[ i ] );

    CORE::CString::StringSet browsed;
    GetBrowsedServiceTypes( browsed );
    CORE::CString::StringSet::iterator n = browsed.begin();
    while ( n != browsed.end() )
    {
        StopBrowse( (*n) );
        ++n;
    }
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::GetDiscoveredEntries( const CORE::CString& serviceType ,
                                            TIDiscoveryEntrySet& entries     ) const
{GUCEF_TRACE;

    bool anyServiceType = serviceType.IsNULLOrEmpty() || serviceType == AnyServiceType;

    MT::CScopeMutex lock( m_dataLock );
    TDiscoveredEntryMap::const_iterator i = m_discovered.begin();
    while ( i != m_discovered.end() )
    {
        const CDiscoveryEntryPtr& entry = (*i).second.entry;
        if ( anyServiceType || entry->GetServiceType() == serviceType )
            entries.insert( CIDiscoveryEntryPtr( entry ) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::FindCapability( const CORE::CString& capabilityId ,
                                      TIDiscoveryEntrySet& entries      ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    CORE::CString capabilities;
    TDiscoveredEntryMap::const_iterator i = m_discovered.begin();
    while ( i != m_discovered.end() )
    {
        const CDiscoveryEntryPtr& entry = (*i).second.entry;
        if ( entry->GetCapabilities( capabilities ) )
        {
            CORE::CString::StringVector capabilityList = capabilities.ParseElements( ',', false );
            for ( size_t n=0; n<capabilityList.size(); ++n )
            {
                if ( capabilityList[ n ].Trim( true ).Trim( false ) == capabilityId )
                {
                    entries.insert( CIDiscoveryEntryPtr( entry ) );
                    break;
                }
            }
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::FindApplication( const CORE::CString& applicationId ,
                                       TIDiscoveryEntrySet& entries       ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    CORE::CString entryAppId;
    TDiscoveredEntryMap::const_iterator i = m_discovered.begin();
    while ( i != m_discovered.end() )
    {
        const CDiscoveryEntryPtr& entry = (*i).second.entry;
        if ( entry->GetApplicationId( entryAppId ) && entryAppId == applicationId )
            entries.insert( CIDiscoveryEntryPtr( entry ) );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::FindHost( const COMCORE::CHostAddress& host ,
                                TIDiscoveryEntrySet& entries      ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    CHostAddress entryHost;
    TDiscoveredEntryMap::const_iterator i = m_discovered.begin();
    while ( i != m_discovered.end() )
    {
        const CDiscoveryEntryPtr& entry = (*i).second.entry;
        if ( entry->GetHost( entryHost ) )
        {
            bool match = false;
            if ( !host.GetHostname().IsNULLOrEmpty() && host.GetHostname() == entryHost.GetHostname() )
                match = true;
            else
            if ( !host.GetIPv4Addresses().empty() && !entryHost.GetIPv4Addresses().empty() &&
                 host.GetFirstIPv4Address().GetAddress() == entryHost.GetFirstIPv4Address().GetAddress() )
                match = true;
            if ( match )
                entries.insert( CIDiscoveryEntryPtr( entry ) );
        }
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::ProcessHeardEntry( const CIDiscoveryEntry& heard )
{GUCEF_TRACE;

    const CORE::CString& serviceType = heard.GetServiceType();
    if ( serviceType.IsNULLOrEmpty() || !IsServiceTypeBrowsed( serviceType ) )
        return true;

    CORE::CString key = heard.GetEntryKey();
    if ( IsOwnEntry( key ) )
        return true;

    if ( 0 == heard.GetTtlInSecs() )
    {
        CORE::CString instanceId;
        heard.GetApplicationInstanceId( instanceId );
        if ( instanceId.IsNULLOrEmpty() )
            instanceId = heard.GetInstanceName();
        return ProcessGoodbye( serviceType, instanceId );
    }

    UInt64 nowInMs = GetTimeInMs();
    CORE::CDateTime now = CORE::CDateTime::NowUTCDateTime();
    const CORE::CEvent* eventToSend = GUCEF_NULL;
    CIDiscoveryEntryPtr eventEntry;
    {
        MT::CScopeMutex lock( m_dataLock );
        TDiscoveredEntryMap::iterator i = m_discovered.find( key );
        if ( i == m_discovered.end() )
        {
            TDiscoveredEntry& record = m_discovered[ key ];
            record.entry = CDiscoveryEntryPtr( GUCEF_NEW CDiscoveryEntry( heard ) );
            record.entry->SetLastSeen( now );
            record.entry->SetSourceClientName( m_clientName );
            record.expiresAtInMs = nowInMs + ( heard.GetTtlInSecs() * (UInt64) 1000 );
            eventToSend = &EntryDiscoveredEvent;
            eventEntry = record.entry;
        }
        else
        {
            TDiscoveredEntry& record = (*i).second;
            record.expiresAtInMs = nowInMs + ( heard.GetTtlInSecs() * (UInt64) 1000 );
            if ( record.entry->HasSameContent( heard ) )
            {
                record.entry->SetLastSeen( now );
            }
            else
            {
                // A new entry object: whoever holds the old one keeps a consistent picture
                record.entry = CDiscoveryEntryPtr( GUCEF_NEW CDiscoveryEntry( heard ) );
                record.entry->SetLastSeen( now );
                record.entry->SetSourceClientName( m_clientName );
                eventToSend = &EntryUpdatedEvent;
                eventEntry = record.entry;
            }
        }
    }
    UpdateHousekeepingTimer();

    if ( GUCEF_NULL != eventToSend )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DiscoveryClient(" << m_clientName << "): " <<
            ( eventToSend == &EntryDiscoveredEvent ? "Discovered " : "Updated " ) << key );

        CDiscoveryEntryEventData eData( eventEntry );
        return NotifyObservers( *eventToSend, &eData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::ProcessGoodbye( const CORE::CString& serviceType, const CORE::CString& instanceId )
{GUCEF_TRACE;

    CORE::CString key = CIDiscoveryEntry::MakeEntryKey( serviceType, instanceId );
    CIDiscoveryEntryPtr lostEntry;
    {
        MT::CScopeMutex lock( m_dataLock );
        TDiscoveredEntryMap::iterator i = m_discovered.find( key );
        if ( i == m_discovered.end() )
            return true;
        lostEntry = (*i).second.entry;
        m_discovered.erase( i );
    }
    UpdateHousekeepingTimer();

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DiscoveryClient(" << m_clientName << "): Lost " << key << " because it was withdrawn" );

    CDiscoveryEntryEventData eData( lostEntry );
    return NotifyObservers( EntryLostEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::ReportDiscoveryError( const CORE::CString& description )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryClient(" << m_clientName << "): " << description );

    TDiscoveryErrorEventData eData( description );
    return NotifyObservers( DiscoveryErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::ExpireEntries( UInt64 nowInMs )
{GUCEF_TRACE;

    TIDiscoveryEntryPtrVector lostEntries;
    {
        MT::CScopeMutex lock( m_dataLock );
        TDiscoveredEntryMap::iterator i = m_discovered.begin();
        while ( i != m_discovered.end() )
        {
            if ( (*i).second.expiresAtInMs <= nowInMs )
            {
                lostEntries.push_back( CIDiscoveryEntryPtr( (*i).second.entry ) );
                m_discovered.erase( i++ );
            }
            else
                ++i;
        }
    }

    for ( size_t n=0; n<lostEntries.size(); ++n )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DiscoveryClient(" << m_clientName << "): Lost " << lostEntries[ n ]->GetEntryKey() << " because its TTL ran out" );

        CDiscoveryEntryEventData eData( lostEntries[ n ] );
        if ( !NotifyObservers( EntryLostEvent, &eData ) ) return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::OnBrowseStopped( const CORE::CString& serviceType )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::OnHousekeeping( UInt64 nowInMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDiscoveryClientBase::OnNotify( CORE::CNotifier* notifier    ,
                                const CORE::CEvent& eventId  ,
                                CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( notifier == &m_housekeepingTimer && CORE::CTimer::TimerUpdateEvent == eventId )
    {
        UInt64 nowInMs = GetTimeInMs();
        if ( !ExpireEntries( nowInMs ) )
            return;
        OnHousekeeping( nowInMs );
        UpdateHousekeepingTimer();
    }
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    CORE::CString name = config.GetAttributeValueOrChildValueByName( "name" ).AsString( m_clientName, true );
    if ( !name.IsNULLOrEmpty() )
        SetDiscoveryClientName( name );

    UInt32 interval = config.GetAttributeValueOrChildValueByName( "housekeepingIntervalInMs" ).AsUInt32( m_housekeepingTimer.GetInterval(), true );
    if ( interval > 0 )
        m_housekeepingTimer.SetInterval( interval );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryClientBase::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "name", m_clientName );
    config.SetAttribute( "housekeepingIntervalInMs", m_housekeepingTimer.GetInterval() );
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
