/*
 *  gucefMULTIPLAY: GUCEF module providing multiplayer RAD functionality
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "gucefMULTIPLAY_CHttpListListingProvider.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAY {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CHttpListListingProvider::ProviderName = "http-list";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The value of the extra by the key, compared without case */
bool
FindExtra( const CORE::CValueList& extra, const char* key, CORE::CString& value )
{GUCEF_TRACE;

    UInt32 keyCount = extra.GetKeyCount();
    for ( UInt32 i=0; i<keyCount; ++i )
    {
        if ( extra.GetKey( i ).AsString().Equals( key, false ) )
        {
            value = extra.GetValue( i ).AsString();
            return true;
        }
    }
    return false;
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CHttpListListingProvider::CHttpListListingProvider( void )
    : CIGameListingProvider()
    , m_list()
    , m_refreshTimer()
    , m_url()
    , m_codec( "xml" )
    , m_endpointScheme( "udp" )
    , m_refreshIntervalInSecs( 0 )
    , m_active( false )
    , m_seenThisRound()
{GUCEF_TRACE;

    SubscribeTo( &m_list );
    SubscribeTo( &m_refreshTimer );
}

/*-------------------------------------------------------------------------*/

CHttpListListingProvider::~CHttpListListingProvider()
{GUCEF_TRACE;

    m_refreshTimer.SetEnabled( false );
    UnsubscribeAllFromObserver();
    m_list.Stop();
}

/*-------------------------------------------------------------------------*/

bool
CHttpListListingProvider::StartRetrieval( void )
{GUCEF_TRACE;

    if ( m_list.IsBusy() )
        return true;

    m_seenThisRound.clear();
    m_list.SetURL( m_url );
    m_list.SetListCodecType( m_codec );
    if ( !m_list.Start( CORE::CValueList() ) )
    {
        ReportProviderError( "Cannot retrieve the list at " + m_url );
        ScheduleRefresh();
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::ScheduleRefresh( void )
{GUCEF_TRACE;

    if ( m_active && m_refreshIntervalInSecs > 0 )
    {
        m_refreshTimer.SetInterval( m_refreshIntervalInSecs * 1000 );
        m_refreshTimer.SetEnabled( true );
    }
}

/*-------------------------------------------------------------------------*/

bool
CHttpListListingProvider::Start( const CGameListingFilter& filter )
{GUCEF_TRACE;

    if ( m_url.IsNULLOrEmpty() )
    {
        ReportProviderError( "No url is configured" );
        return false;
    }
    if ( m_active )
        Stop();

    SetFilter( filter );
    m_active = true;
    return StartRetrieval();
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::Stop( void )
{GUCEF_TRACE;

    m_active = false;
    m_refreshTimer.SetEnabled( false );
    m_list.Stop();
    m_seenThisRound.clear();
    RemoveAllListings();
}

/*-------------------------------------------------------------------------*/

bool
CHttpListListingProvider::Refresh( void )
{GUCEF_TRACE;

    if ( !m_active )
        return false;
    m_refreshTimer.SetEnabled( false );
    return StartRetrieval();
}

/*-------------------------------------------------------------------------*/

bool
CHttpListListingProvider::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CHttpListListingProvider::GetProviderName( void ) const
{GUCEF_TRACE;

    return ProviderName;
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::SetUrl( const CORE::CString& url )
{GUCEF_TRACE;

    m_url = url;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CHttpListListingProvider::GetUrl( void ) const
{GUCEF_TRACE;

    return m_url;
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::SetCodec( const CORE::CString& codec )
{GUCEF_TRACE;

    m_codec = codec;
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::SetEndpointScheme( const CORE::CString& scheme )
{GUCEF_TRACE;

    m_endpointScheme = scheme;
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::SetRefreshIntervalInSecs( UInt32 seconds )
{GUCEF_TRACE;

    m_refreshIntervalInSecs = seconds;
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::ServerInfoToListing( const CServerListProvider::TServerInfo& info ,
                                               const CORE::CString& endpointScheme         ,
                                               CGameListingEntry& listing                  )
{GUCEF_TRACE;

    CORE::CString host = info.hostAddress.GetHostname();
    UInt16 port = info.hostAddress.GetPortInHostByteOrder();

    listing.listingId = "http:" + host + ":" + CORE::ToString( port );
    listing.name = info.name;
    listing.hostName = host;
    listing.endpoints.clear();
    listing.endpoints.push_back( CGameListingEntry::MakeEndpoint( endpointScheme, host, port ) );
    listing.extra = info.extra;
    listing.lastSeen = CORE::CDateTime::NowUTCDateTime();

    CORE::CString value;
    if ( FindExtra( info.extra, "kind", value ) ) listing.kind = CGameListingEntry::StringToKind( value );
    if ( FindExtra( info.extra, "game", value ) ) listing.gameId = value;
    if ( FindExtra( info.extra, "ver", value ) ) listing.gameVersion = value;
    if ( FindExtra( info.extra, "map", value ) ) listing.mapName = value;
    if ( FindExtra( info.extra, "mode", value ) ) listing.modeName = value;
    if ( FindExtra( info.extra, "pl", value ) ) listing.players = CORE::StringToUInt32( value, 0 );
    if ( FindExtra( info.extra, "max", value ) ) listing.maxPlayers = CORE::StringToUInt32( value, 0 );
    if ( FindExtra( info.extra, "spec", value ) ) listing.spectators = CORE::StringToUInt32( value, 0 );
    if ( FindExtra( info.extra, "pw", value ) ) listing.passworded = CORE::StringToBool( value, false );
}

/*-------------------------------------------------------------------------*/

void
CHttpListListingProvider::OnNotify( CORE::CNotifier* notifier                 ,
                                    const CORE::CEvent& eventId               ,
                                    CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_refreshTimer )
    {
        if ( CORE::CTimer::TimerUpdateEvent == eventId )
        {
            m_refreshTimer.SetEnabled( false );
            if ( m_active )
                StartRetrieval();
        }
        return;
    }
    if ( notifier != &m_list || !m_active )
        return;

    if ( CServerListProvider::ServerInfoEvent == eventId )
    {
        const CServerListProvider::ServerInfoEventData* info = static_cast< const CServerListProvider::ServerInfoEventData* >( eventData );
        if ( GUCEF_NULL == info )
            return;
        CGameListingEntry listing;
        ServerInfoToListing( info->GetData(), m_endpointScheme, listing );
        listing.backendName = ProviderName;
        m_seenThisRound.insert( listing.listingId );
        UpsertListing( listing );
    }
    else
    if ( CServerListProvider::ServerListProviderFinisedEvent == eventId )
    {
        CORE::CString::StringSet seen = m_seenThisRound;
        m_seenThisRound.clear();
        if ( !RemoveListingsExcept( seen ) )
            return;
        ScheduleRefresh();
    }
    else
    if ( CServerListProvider::ServerListProviderErrorEvent == eventId )
    {
        m_seenThisRound.clear();
        if ( !ReportProviderError( "Cannot retrieve the list at " + m_url ) )
            return;
        ScheduleRefresh();
    }
}

/*-------------------------------------------------------------------------*/

bool
CHttpListListingProvider::SaveConfig( CORE::CDataNode& config ) const
{GUCEF_TRACE;

    config.SetAttribute( "url", m_url );
    config.SetAttribute( "codec", m_codec );
    config.SetAttribute( "endpointScheme", m_endpointScheme );
    config.SetAttribute( "refreshIntervalInSecs", m_refreshIntervalInSecs );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CHttpListListingProvider::LoadConfig( const CORE::CDataNode& config )
{GUCEF_TRACE;

    m_url = config.GetAttributeValueOrChildValueByName( "url" ).AsString( m_url, true );
    m_codec = config.GetAttributeValueOrChildValueByName( "codec" ).AsString( m_codec, true );
    m_endpointScheme = config.GetAttributeValueOrChildValueByName( "endpointScheme" ).AsString( m_endpointScheme, true );
    m_refreshIntervalInSecs = config.GetAttributeValueOrChildValueByName( "refreshIntervalInSecs" ).AsUInt32( m_refreshIntervalInSecs, true );
    return !m_url.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CHttpListListingProvider::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::MULTIPLAY::CHttpListListingProvider";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
