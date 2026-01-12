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
      m_pingers()          ,
      m_toBePingedList()
{GUCEF_TRACE;

    SetParallelPingMax( DEFAULT_MAX_PARALLEL_PINGS );
}

/*-------------------------------------------------------------------------*/
    
CServerList::CServerList( const CServerList& src )
    : CObservingNotifier( src )        ,
      m_serverList( src.m_serverList ) ,
      m_pingers()                      ,
      m_toBePingedList()
{GUCEF_TRACE;

    SetParallelPingMax( (UInt32) src.m_pingers.size() );
}

/*-------------------------------------------------------------------------*/
 
CServerList::~CServerList()
{GUCEF_TRACE;

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
        // First we handle the response
        COMCORE::CPing& ping = *static_cast< COMCORE::CPing* >( notifier );
        UInt32 pingTime = static_cast< COMCORE::CPing::CPingEventData* >( eventdata )->GetRoundTripTime();
        
        TServerListEntryPtr entry = *static_cast< TServerListEntryPtr* >( ping.GetUserData() );
        delete static_cast< TServerListEntryPtr* >( ping.GetUserData() );
        
        entry->ping = pingTime;
        
        // Now we set the ping object back to work
        TServerInfoList::iterator n = m_toBePingedList.begin();
        if ( n != m_toBePingedList.end() )
        {
            entry = (*n);
            m_toBePingedList.erase( n );
            
            ping.SetUserData( new TServerListEntryPtr( entry ) );
            ping.Start( entry->serverInfo.hostAddress.GetFirstAddressAsString() ,
                        1                                               );
        }
        
        NotifyObservers( ServerListChangedEvent );
    }
    else
    if ( ( eventid == COMCORE::CPing::PingTimeoutEvent ) ||
         ( eventid == COMCORE::CPing::PingFailedEvent )   )
    {
        COMCORE::CPing& ping = *static_cast< COMCORE::CPing* >( notifier );
        TServerListEntryPtr entry = *static_cast< TServerListEntryPtr* >( ping.GetUserData() );
        delete static_cast< TServerListEntryPtr* >( ping.GetUserData() );
        
        entry->ping = -1;
        
        // Now we set the ping object back to work
        TServerInfoList::iterator n = m_toBePingedList.begin();
        if ( n != m_toBePingedList.end() )
        {
            entry = (*n);
            m_toBePingedList.erase( n );
            
            ping.SetUserData( new TServerListEntryPtr( entry ) );
            ping.Start( entry->serverInfo.hostAddress.GetFirstAddressAsString() ,
                        1                                               );
        }
                
        NotifyObservers( ServerListChangedEvent );
    }
}

/*-------------------------------------------------------------------------*/

void
CServerList::RefreshPings( void )
{GUCEF_TRACE;
    
    if ( !IsBusyWithPing() )
    {
        // Shove the entire list into our ping 'todo' list
        m_toBePingedList = m_serverList;
        
        if ( m_toBePingedList.size() > 0 )
        {
            // Set things in motion
            TServerListEntryPtr entry;
            TServerInfoList::iterator n = m_toBePingedList.begin();
            TPingers::iterator i = m_pingers.begin();
            while ( i != m_pingers.end() )
            {
                entry = (*n);
                n = m_toBePingedList.erase( n );
                (*i)->SetUserData( new TServerListEntryPtr( entry ) );
                (*i)->Start( entry->serverInfo.hostAddress.GetFirstAddressAsString() ,
                             1                                               );
                
                if ( n == m_toBePingedList.end() )
                {
                    break;
                }
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

    m_toBePingedList.clear();
}

/*-------------------------------------------------------------------------*/

bool
CServerList::IsBusyWithPing( void ) const
{GUCEF_TRACE;

    return m_toBePingedList.size() > 0;
}

/*-------------------------------------------------------------------------*/

bool
CServerList::SetParallelPingMax( const UInt32 max )
{GUCEF_TRACE;

    if ( !IsBusyWithPing() )
    {
        UInt32 oldMax = (UInt32) m_pingers.size();
        if ( oldMax > max )
        {
            for ( UInt32 i=max; i<oldMax; ++i )
            {
                delete m_pingers[ i ];
            }
        }
        
        m_pingers.resize( max );
        
        if ( oldMax < max )
        {
            for ( UInt32 i=oldMax; i<max; ++i )
            {
                m_pingers[ i ] = new COMCORE::CPing();
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
