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

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#include "gucefCOMCORE_CServiceAdvertiser.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CServiceAdvertiser::CServiceAdvertiser( void )
    : CORE::CIConfigurable()
    , m_pulseGenerator()
    , m_settings()
    , m_clients()
    , m_clientsCreated( false )
    , m_createDefaultClients( true )
    , m_advertising( false )
    , m_instanceId()
    , m_announcement()
    , m_entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAdvertiser::CServiceAdvertiser( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CORE::CIConfigurable()
    , m_pulseGenerator( pulseGenerator )
    , m_settings()
    , m_clients()
    , m_clientsCreated( false )
    , m_createDefaultClients( true )
    , m_advertising( false )
    , m_instanceId()
    , m_announcement()
    , m_entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAdvertiser::CServiceAdvertiser( const CServiceAdvertiserSettings& settings )
    : CORE::CIConfigurable()
    , m_pulseGenerator()
    , m_settings( settings )
    , m_clients()
    , m_clientsCreated( false )
    , m_createDefaultClients( true )
    , m_advertising( false )
    , m_instanceId()
    , m_announcement()
    , m_entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAdvertiser::CServiceAdvertiser( const CORE::PulseGeneratorPtr& pulseGenerator, const CServiceAdvertiserSettings& settings )
    : CORE::CIConfigurable()
    , m_pulseGenerator( pulseGenerator )
    , m_settings( settings )
    , m_clients()
    , m_clientsCreated( false )
    , m_createDefaultClients( true )
    , m_advertising( false )
    , m_instanceId()
    , m_announcement()
    , m_entry()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CServiceAdvertiser::~CServiceAdvertiser()
{GUCEF_TRACE;

    Withdraw();
    m_clients.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CServiceAdvertiser::MakeInstanceId( void )
{GUCEF_TRACE;

    UInt8 bytes[ 16 ];
    if ( !CORE::CRandom::GetRandomBytes( bytes, sizeof( bytes ) ) )
    {
        for ( int i=0; i<16; ++i )
            bytes[ i ] = (UInt8) CORE::CRandom::GetRandomUInt32( 0, 255 );
    }
    static const char hexDigits[] = "0123456789abcdef";
    char text[ 33 ];
    for ( int i=0; i<16; ++i )
    {
        text[ i*2 ] = hexDigits[ ( bytes[ i ] >> 4 ) & 0x0F ];
        text[ i*2+1 ] = hexDigits[ bytes[ i ] & 0x0F ];
    }
    text[ 32 ] = '\0';
    return CORE::CString( text );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::PrepareClient( CIDiscoveryClient& client ) const
{GUCEF_TRACE;

    if ( m_pulseGenerator.IsNULL() )
        return true;
    if ( client.SetPulseGenerator( m_pulseGenerator ) )
        return true;
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: The discovery client " << client.GetDiscoveryClientName() << " cannot run on the pulse generator of the advertiser, it is not used" );
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::EnsureClients( void )
{GUCEF_TRACE;

    if ( m_clientsCreated )
        return true;
    m_clientsCreated = true;

    // Clients the config names are always made; clients given by hand replace the default types
    bool configured = CDiscoveryClientList::NamesClients( m_settings.clientConfig );
    if ( !configured && ( !m_clients.empty() || !m_createDefaultClients ) )
        return true;

    TDiscoveryClientVector made;
    bool allMade = CDiscoveryClientList::CreateFromConfig( m_settings.clientConfig, m_settings.defaultClientTypes, made );
    for ( size_t i=0; i<made.size(); ++i )
    {
        if ( PrepareClient( *made[ i ] ) )
            m_clients.push_back( made[ i ] );
        else
            allMade = false;
    }
    return allMade;
}

/*-------------------------------------------------------------------------*/

void
CServiceAdvertiser::Complete( CServiceAnnouncement& announcement ) const
{GUCEF_TRACE;

    if ( announcement.serviceType.IsNULLOrEmpty() )
        announcement.serviceType = m_settings.serviceType;
    if ( announcement.instanceId.IsNULLOrEmpty() )
        announcement.instanceId = m_settings.instanceId.IsNULLOrEmpty() ? m_instanceId : m_settings.instanceId;
    if ( announcement.displayName.IsNULLOrEmpty() )
        announcement.displayName = m_settings.displayName;
    if ( m_settings.ttlInSecs > 0 && CServiceAnnouncement::DefaultTtlInSecs == announcement.ttlInSecs )
        announcement.ttlInSecs = m_settings.ttlInSecs;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::Publish( void )
{GUCEF_TRACE;

    // A new entry with the same key replaces the advertised one, the clients announce the change
    CDiscoveryEntry* entry = GUCEF_NEW CDiscoveryEntry();
    m_announcement.ToEntry( *entry );
    m_entry = CIDiscoveryEntryPtr( entry );

    bool anyPublished = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
    {
        if ( m_clients[ i ]->Advertise( m_entry ) )
            anyPublished = true;
        else
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: The discovery client " << m_clients[ i ]->GetDiscoveryClientName() << " cannot advertise " << m_announcement.GetKey() );
    }
    return anyPublished;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::Advertise( const CServiceAnnouncement& announcement )
{GUCEF_TRACE;

    if ( m_advertising )
        Withdraw();

    CServiceAnnouncement completed( announcement );
    if ( completed.instanceId.IsNULLOrEmpty() && m_settings.instanceId.IsNULLOrEmpty() && m_instanceId.IsNULLOrEmpty() )
        m_instanceId = MakeInstanceId();
    Complete( completed );

    CORE::CString reason;
    if ( !completed.IsValid( reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: The announcement cannot be advertised: " << reason );
        return false;
    }

    EnsureClients();
    m_announcement = completed;
    m_advertising = Publish();
    if ( m_advertising )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: Advertising " << m_announcement.serviceType << " instance " << m_announcement.instanceId << " through " << m_clients.size() << " discovery clients" );
    }
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: No discovery client could advertise " << m_announcement.GetKey() );
    }

    return m_advertising;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::Update( const CServiceAnnouncement& announcement )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;

    // The identity stays the one it was advertised with
    CServiceAnnouncement updated( announcement );
    updated.serviceType = m_announcement.serviceType;
    updated.instanceId = m_announcement.instanceId;
    Complete( updated );
    if ( updated.HasSameContent( m_announcement ) && updated.ttlInSecs == m_announcement.ttlInSecs )
        return true;

    CORE::CString reason;
    if ( !updated.IsValid( reason ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: The changed announcement cannot be advertised: " << reason );
        return false;
    }
    m_announcement = updated;
    return Publish();
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::Refresh( void )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;
    return Publish();
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::Withdraw( void )
{GUCEF_TRACE;

    if ( !m_advertising )
        return false;
    m_advertising = false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        m_clients[ i ]->Withdraw( m_entry );
    m_entry.Unlink();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "ServiceAdvertiser: Withdrew " << m_announcement.GetKey() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::IsAdvertising( void ) const
{GUCEF_TRACE;

    return m_advertising;
}

/*-------------------------------------------------------------------------*/

const CServiceAnnouncement&
CServiceAdvertiser::GetAnnouncement( void ) const
{GUCEF_TRACE;

    return m_announcement;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::AddDiscoveryClient( CIDiscoveryClientPtr client )
{GUCEF_TRACE;

    if ( client.IsNULL() )
        return false;
    for ( size_t i=0; i<m_clients.size(); ++i )
        if ( m_clients[ i ] == client )
            return true;
    m_clients.push_back( client );
    if ( m_advertising && !m_entry.IsNULL() )
        return client->Advertise( m_entry );
    return true;
}

/*-------------------------------------------------------------------------*/

const CServiceAdvertiser::TDiscoveryClientVector&
CServiceAdvertiser::GetDiscoveryClients( void ) const
{GUCEF_TRACE;

    return m_clients;
}

/*-------------------------------------------------------------------------*/

void
CServiceAdvertiser::SetCreateDefaultClients( bool createDefaultClients )
{GUCEF_TRACE;

    m_createDefaultClients = createDefaultClients;
}

/*-------------------------------------------------------------------------*/

void
CServiceAdvertiser::SetSettings( const CServiceAdvertiserSettings& settings )
{GUCEF_TRACE;

    m_settings = settings;
}

/*-------------------------------------------------------------------------*/

const CServiceAdvertiserSettings&
CServiceAdvertiser::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    return m_settings.SaveConfig( config );
}

/*-------------------------------------------------------------------------*/

bool
CServiceAdvertiser::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    return m_settings.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CServiceAdvertiser::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CServiceAdvertiser";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
