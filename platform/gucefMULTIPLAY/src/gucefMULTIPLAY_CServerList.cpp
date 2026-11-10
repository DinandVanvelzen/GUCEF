/*
 *  gucefMULTIPLAY: GUCE module providing multiplayer RAD functionality
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include "gucefMULTIPLAY_CServerList.h"

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

const CORE::CEvent CServerList::ServerListChangedEvent = "GUCEF::MULTIPLAY::CServerList::ServerListChangedEvent";

#define DEFAULT_MAX_PARALLEL_PINGS  4

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CServerList::RegisterEvents( void )
{GUCEF_TRACE;
    
    ServerListChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CServerList::CServerList( void )
    : CObservingNotifier() ,
      m_serverList()       ,
      m_toBePingedList()   ,
      m_pingers()          ,
      m_activePings( 0 )
{GUCEF_TRACE;

    SetParallelPingMax( DEFAULT_MAX_PARALLEL_PINGS );
}

/*-------------------------------------------------------------------------*/
    
CServerList::CServerList( const CServerList& src )
    : CObservingNotifier( src )        ,
      m_serverList( src.m_serverList ) ,
      m_toBePingedList()               ,
      m_pingers()                      ,
      m_activePings( 0 )
{GUCEF_TRACE;

    SetParallelPingMax( (UInt32) src.m_pingers.size() );
}

/*-------------------------------------------------------------------------*/
 
CServerList::~CServerList()
{GUCEF_TRACE;

    m_toBePingedList.clear();
    DestroyPingers( 0 );
}

/*-------------------------------------------------------------------------*/

void
CServerList::DestroyPingers( UInt32 firstIndex )
{GUCEF_TRACE;

    for ( UInt32 i=firstIndex; i<(UInt32) m_pingers.size(); ++i )
    {
        COMCORE::CPing* ping = m_pingers[ i ];
        if ( GUCEF_NULL != ping )
        {
            UnsubscribeFrom( ping );
            ping->Stop();
            ReleasePingUserData( *ping );
            delete ping;
        }
    }
    if ( firstIndex < (UInt32) m_pingers.size() )
        m_pingers.resize( firstIndex );
}

/*-------------------------------------------------------------------------*/

void
CServerList::ReleasePingUserData( COMCORE::CPing& ping )
{GUCEF_TRACE;

    TServerListEntryPtr* entry = static_cast< TServerListEntryPtr* >( ping.GetUserData() );
    delete entry;
    ping.SetUserData( GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CServerList::StartNextPing( COMCORE::CPing& ping )
{GUCEF_TRACE;

    while ( !m_toBePingedList.empty() )
    {
        TServerListEntryPtr entry = m_toBePingedList.front();
        m_toBePingedList.erase( m_toBePingedList.begin() );

        ping.SetUserData( new TServerListEntryPtr( entry ) );
        if ( ping.Start( entry->serverInfo.hostAddress.GetFirstAddressAsString(), 1 ) )
            return true;

        // this one cannot be pinged at all
        entry->ping = -1;
        ReleasePingUserData( ping );
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
CServerList&
CServerList::operator=( const CServerList& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CObservingNotifier::operator=( src );
        
        m_serverList = src.m_serverList;
        SetParallelPingMax( src.GetParallelPingMax() );
        
        NotifyObservers( ServerListChangedEvent );        
    }    
    return *this;
}

/*-------------------------------------------------------------------------*/
    
void
CServerList::SetList( const TServerInfoList& newList )
{GUCEF_TRACE;

    m_serverList = newList;
    NotifyObservers( ServerListChangedEvent );
}

/*-------------------------------------------------------------------------*/

const CServerList::TServerInfoList&
CServerList::GetList( void ) const
{GUCEF_TRACE;

    return m_serverList;
}

/*-------------------------------------------------------------------------*/
    
void
CServerList::Clear( void )
{GUCEF_TRACE;

    m_serverList.clear();
    NotifyObservers( ServerListChangedEvent );
}

/*-------------------------------------------------------------------------*/

void
CServerList::OnNotify( GUCEF::CORE::CNotifier* notifier                 ,
                       const GUCEF::CORE::CEvent& eventid               ,
                       GUCEF::CORE::CICloneable* eventdata /* = NULL */ )
{GUCEF_TRACE;

    // mandatory, call base
    CObservingNotifier::OnNotify( notifier  ,
                                  eventid   ,
                                  eventdata );
                                  
    // check if we received server info
    if ( eventid == CServerListProvider::ServerInfoEvent )
    {
        // Add the info to our list
        const CServerListProvider::TServerInfo& serverInfo = static_cast< CServerListProvider::ServerInfoEventData* >( eventdata )->GetData();
        TServerListEntry* entry = new TServerListEntry;
        entry->serverInfo = serverInfo;
        entry->ping = -1;
        m_serverList.push_back( TServerListEntryPtr( entry ) );
        
        NotifyObservers( ServerListChangedEvent );
    }
    else
    if ( eventid == COMCORE::CPing::PingReponseEvent )
    {
        // the answer of the server the pinger works on
        COMCORE::CPing& ping = *static_cast< COMCORE::CPing* >( notifier );
        TServerListEntryPtr* entry = static_cast< TServerListEntryPtr* >( ping.GetUserData() );
        if ( GUCEF_NULL != entry )
            (*entry)->ping = (Int32) static_cast< COMCORE::CPing::CPingEventData* >( eventdata )->GetRoundTripTime();
    }
    else
    if ( ( eventid == COMCORE::CPing::PingTimeoutEvent ) ||
         ( eventid == COMCORE::CPing::PingFailedEvent )   )
    {
        COMCORE::CPing& ping = *static_cast< COMCORE::CPing* >( notifier );
        TServerListEntryPtr* entry = static_cast< TServerListEntryPtr* >( ping.GetUserData() );
        if ( GUCEF_NULL != entry )
            (*entry)->ping = -1;
    }
    else
    if ( eventid == COMCORE::CPing::PingStoppedEvent )
    {
        // the pinger is free again: on to the next server, if any
        COMCORE::CPing& ping = *static_cast< COMCORE::CPing* >( notifier );
        if ( GUCEF_NULL == ping.GetUserData() )
            return;
        ReleasePingUserData( ping );
        if ( !StartNextPing( ping ) && m_activePings > 0 )
            --m_activePings;

        NotifyObservers( ServerListChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerList::RefreshPings( void )
{GUCEF_TRACE;

    if ( !IsBusyWithPing() )
    {
        // Shove the entire list into our ping 'todo' list and give every pinger a server
        m_toBePingedList = m_serverList;
        if ( !m_toBePingedList.empty() )
        {
            TPingers::iterator i = m_pingers.begin();
            while ( i != m_pingers.end() && !m_toBePingedList.empty() )
            {
                if ( StartNextPing( *(*i) ) )
                    ++m_activePings;
                ++i;
            }
            NotifyObservers( ServerListChangedEvent );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CServerList::StopRefreshingPings( void )
{GUCEF_TRACE;

    // the pings that run finish, no new ones start
    m_toBePingedList.clear();
}

/*-------------------------------------------------------------------------*/

bool
CServerList::IsBusyWithPing( void ) const
{GUCEF_TRACE;

    return m_activePings > 0 || !m_toBePingedList.empty();
}

/*-------------------------------------------------------------------------*/

bool
CServerList::SetParallelPingMax( const UInt32 max )
{GUCEF_TRACE;

    if ( !IsBusyWithPing() )
    {
        UInt32 oldMax = (UInt32) m_pingers.size();
        if ( oldMax > max )
            DestroyPingers( max );

        if ( oldMax < max )
        {
            m_pingers.resize( max );
            for ( UInt32 i=oldMax; i<max; ++i )
            {
                m_pingers[ i ] = new COMCORE::CPing();
                SubscribeTo( m_pingers[ i ] );
            }
        }
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/
    
UInt32
CServerList::GetParallelPingMax( void ) const
{GUCEF_TRACE;

    return (UInt32) m_pingers.size();
}

/*-------------------------------------------------------------------------*/

void
CServerList::GetServerHostnameList( TStringList& list ) const
{GUCEF_TRACE;
    
    TServerInfoList::const_iterator i = m_serverList.begin();
    while ( i != m_serverList.end() )
    {
        list.push_back( (*i)->serverInfo.hostAddress.GetHostname() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CServerList::GetServerAddressList( TStringList& list ) const
{GUCEF_TRACE;
    
    TServerInfoList::const_iterator i = m_serverList.begin();
    while ( i != m_serverList.end() )
    {
        list.push_back( (*i)->serverInfo.hostAddress.GetFirstAddressAsString() );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CServerList::GetServerNameList( TStringList& list ) const
{GUCEF_TRACE;
    
    TServerInfoList::const_iterator i = m_serverList.begin();
    while ( i != m_serverList.end() )
    {
        list.push_back( (*i)->serverInfo.name );
        ++i;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MULTIPLAY */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
