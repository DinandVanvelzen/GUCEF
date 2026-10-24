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

#ifndef GUCEF_CORE_CDATANODE_H
#include "CDataNode.h"
#define GUCEF_CORE_CDATANODE_H
#endif /* GUCEF_CORE_CDATANODE_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYENTRY_H
#include "gucefCOMCORE_CDiscoveryEntry.h"
#define GUCEF_COMCORE_CDISCOVERYENTRY_H
#endif /* GUCEF_COMCORE_CDISCOVERYENTRY_H ? */

#ifndef GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#include "gucefCOMCORE_CDiscoveryClientFactory.h"
#define GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H
#endif /* GUCEF_COMCORE_CDISCOVERYCLIENTFACTORY_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#include "gucefCOMCORE_CDiscoveryManager.h"

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

CDiscoveryManager::CDiscoveryManager( void )
    : CORE::CObservingNotifier()
    , CORE::CGloballyConfigurable( false )
    , CIDiscovery()
    , m_clients()
    , m_clientTypes()
    , m_browsedServiceTypes()
    , m_advertised()
    , m_merged()
    , m_dataLock()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDiscoveryManager::~CDiscoveryManager()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();

    MT::CScopeMutex lock( m_dataLock );
    m_merged.clear();
    m_advertised.clear();
    m_clients.clear();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::RegisterDiscoveryClient( CIDiscoveryClientPtr discoveryClient )
{GUCEF_TRACE;

    if ( discoveryClient.IsNULL() )
        return false;

    TStringSet browsed;
    TEntryVector advertised;
    {
        MT::CScopeMutex lock( m_dataLock );
        if ( !m_clients.insert( discoveryClient ).second )
            return true;
        browsed = m_browsedServiceTypes;
        TAdvertisedEntryMap::iterator i = m_advertised.begin();
        while ( i != m_advertised.end() )
        {
            advertised.push_back( (*i).second );
            ++i;
        }
    }

    SubscribeTo( discoveryClient.GetPointerAlways() );

    // The newcomer joins in with what is already going on
    bool totalSuccess = true;
    TStringSet::iterator n = browsed.begin();
    while ( n != browsed.end() )
    {
        totalSuccess = discoveryClient->Browse( (*n) ) && totalSuccess;
        ++n;
    }
    for ( size_t a=0; a<advertised.size(); ++a )
        totalSuccess = discoveryClient->Advertise( advertised[ a ] ) && totalSuccess;

    // Pick up what the client found before it was registered
    CIDiscovery::TIDiscoveryEntrySet known;
    discoveryClient->GetDiscoveredEntries( CORE::CString::Empty, known );
    CORE::CString clientName = discoveryClient->GetDiscoveryClientName();
    CIDiscovery::TIDiscoveryEntrySet::iterator k = known.begin();
    while ( k != known.end() )
    {
        if ( !OnClientEntryFound( clientName, (*k) ) ) return totalSuccess;
        ++k;
    }

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryManager: Registered discovery client \"" << clientName << "\"" );
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::UnregisterDiscoveryClient( const CORE::CString& discoveryClientName )
{GUCEF_TRACE;

    CIDiscoveryClientPtr client;
    {
        MT::CScopeMutex lock( m_dataLock );
        TDiscoveryClientSet::iterator i = m_clients.begin();
        while ( i != m_clients.end() )
        {
            if ( discoveryClientName == (*i)->GetDiscoveryClientName() )
            {
                client = (*i);
                m_clients.erase( i );
                m_clientTypes.erase( discoveryClientName );
                break;
            }
            ++i;
        }
    }

    if ( client.IsNULL() )
        return false;

    UnsubscribeFrom( client.GetPointerAlways() );
    ForgetClientEntries( discoveryClientName );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryManager: Unregistered discovery client \"" << discoveryClientName << "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CDiscoveryManager::UnregisterDiscoveryClientsOfType( const CORE::CString& clientType )
{GUCEF_TRACE;

    CORE::CString::StringVector names;
    {
        MT::CScopeMutex lock( m_dataLock );
        TStringMap::iterator i = m_clientTypes.begin();
        while ( i != m_clientTypes.end() )
        {
            if ( (*i).second == clientType )
                names.push_back( (*i).first );
            ++i;
        }
    }

    UInt32 count = 0;
    for ( size_t n=0; n<names.size(); ++n )
    {
        if ( UnregisterDiscoveryClient( names[ n ] ) )
            ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClientPtr
CDiscoveryManager::GetDiscoveryClient( const CORE::CString& discoveryClientName ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    TDiscoveryClientSet::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        if ( discoveryClientName == (*i)->GetDiscoveryClientName() )
            return (*i);
        ++i;
    }
    return CIDiscoveryClientPtr();
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryManager::GetDiscoveryClients( TDiscoveryClientVector& clients ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    clients.reserve( clients.size() + m_clients.size() );
    TDiscoveryClientSet::const_iterator i = m_clients.begin();
    while ( i != m_clients.end() )
    {
        clients.push_back( (*i) );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

CIDiscoveryClientPtr
CDiscoveryManager::CreateDiscoveryClient( const CORE::CString& clientType ,
                                          const CORE::CDataNode* config   )
{GUCEF_TRACE;

    CDiscoveryClientFactory& factory = CComCoreGlobal::Instance()->GetDiscoveryClientFactory();
    CIDiscoveryClientPtr client = factory.Create( clientType );
    if ( client.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DiscoveryManager: Unable to create a discovery client of type \"" << clientType << "\"" );
        return CIDiscoveryClientPtr();
    }

    if ( GUCEF_NULL != config )
    {
        if ( !client->LoadConfig( *config ) )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DiscoveryManager: The config of the discovery client of type \"" << clientType << "\" could not be loaded" );
            return CIDiscoveryClientPtr();
        }
    }

    {
        MT::CScopeMutex lock( m_dataLock );
        m_clientTypes[ client->GetDiscoveryClientName() ] = clientType;
    }
    if ( !RegisterDiscoveryClient( client ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DiscoveryManager: The new discovery client \"" << client->GetDiscoveryClientName() << "\" could not join all active browses and advertisements" );
    }
    return client;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::Browse( const CORE::CString& serviceType )
{GUCEF_TRACE;

    if ( serviceType.IsNULLOrEmpty() )
        return false;

    TDiscoveryClientVector clients;
    bool isNew = false;
    {
        MT::CScopeMutex lock( m_dataLock );
        isNew = m_browsedServiceTypes.insert( serviceType ).second;
    }
    GetDiscoveryClients( clients );

    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->Browse( serviceType ) && totalSuccess;

    if ( isNew )
    {
        CIDiscoveryClient::TBrowseEventData eData( serviceType );
        if ( !NotifyObservers( CIDiscoveryClient::BrowseStartedEvent, &eData ) ) return totalSuccess;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::StopBrowse( const CORE::CString& serviceType )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_dataLock );
        if ( 0 == m_browsedServiceTypes.erase( serviceType ) )
            return false;
    }

    // The clients report the entries they forget as lost, which takes them out of the merged view
    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );
    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->StopBrowse( serviceType ) && totalSuccess;

    CIDiscoveryClient::TBrowseEventData eData( serviceType );
    NotifyObservers( CIDiscoveryClient::BrowseStoppedEvent, &eData );
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::IsBrowsing( const CORE::CString& serviceType ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_dataLock );
    if ( serviceType.IsNULLOrEmpty() )
        return !m_browsedServiceTypes.empty();
    return m_browsedServiceTypes.find( serviceType ) != m_browsedServiceTypes.end();
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::Advertise( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    if ( entry.IsNULL() )
        return false;

    {
        MT::CScopeMutex lock( m_dataLock );
        m_advertised[ entry->GetEntryKey() ] = entry;
    }

    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );
    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->Advertise( entry ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::Withdraw( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    if ( entry.IsNULL() )
        return false;

    {
        MT::CScopeMutex lock( m_dataLock );
        m_advertised.erase( entry->GetEntryKey() );
    }

    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );
    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->Withdraw( entry ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::Register( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    return Advertise( entry );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::Unregister( CIDiscoveryEntryPtr entry )
{GUCEF_TRACE;

    return Withdraw( entry );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::GetDiscoveredEntries( const CORE::CString& serviceType ,
                                         TIDiscoveryEntrySet& entries     ) const
{GUCEF_TRACE;

    bool anyServiceType = serviceType.IsNULLOrEmpty() || serviceType == CIDiscoveryClient::AnyServiceType;

    MT::CScopeMutex lock( m_dataLock );
    TMergedEntryMap::const_iterator i = m_merged.begin();
    while ( i != m_merged.end() )
    {
        const CIDiscoveryEntryPtr& entry = (*i).second.entry;
        if ( anyServiceType || entry->GetServiceType() == serviceType )
            entries.insert( entry );
        ++i;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::FindCapability( const CORE::CString& capabilityId ,
                                   TIDiscoveryEntrySet& entries      ) const
{GUCEF_TRACE;

    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );
    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->FindCapability( capabilityId, entries ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::FindApplication( const CORE::CString& applicationId ,
                                    TIDiscoveryEntrySet& entries        ) const
{GUCEF_TRACE;

    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );
    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->FindApplication( applicationId, entries ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::FindHost( const COMCORE::CHostAddress& host ,
                             TIDiscoveryEntrySet& entries      ) const
{GUCEF_TRACE;

    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );
    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
        totalSuccess = clients[ i ]->FindHost( host, entries ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::OnClientEntryFound( const CORE::CString& clientName, const CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    if ( entry.IsNULL() )
        return true;

    CORE::CString key = entry->GetEntryKey();
    const CORE::CEvent* eventToSend = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_dataLock );
        TMergedEntryMap::iterator i = m_merged.find( key );
        if ( i == m_merged.end() )
        {
            TMergedEntry& merged = m_merged[ key ];
            merged.entry = entry;
            merged.clientNames.insert( clientName );
            eventToSend = &CIDiscoveryClient::EntryDiscoveredEvent;
        }
        else
        {
            TMergedEntry& merged = (*i).second;
            merged.clientNames.insert( clientName );
            if ( merged.entry != entry )
            {
                CDiscoveryEntry known( *merged.entry );
                merged.entry = entry;
                if ( !known.HasSameContent( *entry ) )
                    eventToSend = &CIDiscoveryClient::EntryUpdatedEvent;
            }
        }
    }

    if ( GUCEF_NULL != eventToSend )
    {
        CDiscoveryEntryEventData eData( entry );
        return NotifyObservers( *eventToSend, &eData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::OnClientEntryLost( const CORE::CString& clientName, const CIDiscoveryEntryPtr& entry )
{GUCEF_TRACE;

    if ( entry.IsNULL() )
        return true;

    CIDiscoveryEntryPtr lostEntry;
    {
        MT::CScopeMutex lock( m_dataLock );
        TMergedEntryMap::iterator i = m_merged.find( entry->GetEntryKey() );
        if ( i == m_merged.end() )
            return true;
        TMergedEntry& merged = (*i).second;
        merged.clientNames.erase( clientName );
        if ( !merged.clientNames.empty() )
            return true;
        lostEntry = merged.entry;
        m_merged.erase( i );
    }

    CDiscoveryEntryEventData eData( lostEntry );
    return NotifyObservers( CIDiscoveryClient::EntryLostEvent, &eData );
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::ForgetClientEntries( const CORE::CString& clientName )
{GUCEF_TRACE;

    TEntryVector lostEntries;
    {
        MT::CScopeMutex lock( m_dataLock );
        TMergedEntryMap::iterator i = m_merged.begin();
        while ( i != m_merged.end() )
        {
            TMergedEntry& merged = (*i).second;
            merged.clientNames.erase( clientName );
            if ( merged.clientNames.empty() )
            {
                lostEntries.push_back( merged.entry );
                m_merged.erase( i++ );
            }
            else
                ++i;
        }
    }

    for ( size_t n=0; n<lostEntries.size(); ++n )
    {
        CDiscoveryEntryEventData eData( lostEntries[ n ] );
        if ( !NotifyObservers( CIDiscoveryClient::EntryLostEvent, &eData ) ) return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDiscoveryManager::OnNotify( CORE::CNotifier* notifier    ,
                             const CORE::CEvent& eventId  ,
                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == notifier )
        return;

    // All the notifiers we observe are our clients
    CIDiscoveryClient* client = static_cast< CIDiscoveryClient* >( notifier );
    if ( CIDiscoveryClient::EntryDiscoveredEvent == eventId || CIDiscoveryClient::EntryUpdatedEvent == eventId )
    {
        CDiscoveryEntryEventData* entryData = static_cast< CDiscoveryEntryEventData* >( eventData );
        if ( GUCEF_NULL != entryData )
            OnClientEntryFound( client->GetDiscoveryClientName(), *entryData );
    }
    else
    if ( CIDiscoveryClient::EntryLostEvent == eventId )
    {
        CDiscoveryEntryEventData* entryData = static_cast< CDiscoveryEntryEventData* >( eventData );
        if ( GUCEF_NULL != entryData )
            OnClientEntryLost( client->GetDiscoveryClientName(), *entryData );
    }
    else
    if ( CIDiscoveryClient::DiscoveryErrorEvent == eventId )
    {
        CIDiscoveryClient::TDiscoveryErrorEventData* errorData = static_cast< CIDiscoveryClient::TDiscoveryErrorEventData* >( eventData );
        CORE::CString description = client->GetDiscoveryClientName() + ": ";
        if ( GUCEF_NULL != errorData )
            description += errorData->GetData();
        CIDiscoveryClient::TDiscoveryErrorEventData eData( description );
        NotifyObservers( CIDiscoveryClient::DiscoveryErrorEvent, &eData );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::SaveConfig( CORE::CDataNode& tree ) const
{GUCEF_TRACE;

    CORE::CDataNode* managerNode = tree.FindOrAddChild( "DiscoveryManager" );
    if ( GUCEF_NULL == managerNode )
        return false;

    TDiscoveryClientVector clients;
    GetDiscoveryClients( clients );

    bool totalSuccess = true;
    for ( size_t i=0; i<clients.size(); ++i )
    {
        CORE::CDataNode* clientNode = managerNode->AddChild( "DiscoveryClient" );
        if ( GUCEF_NULL == clientNode )
            return false;

        const CORE::CString& clientName = clients[ i ]->GetDiscoveryClientName();
        {
            MT::CScopeMutex lock( m_dataLock );
            TStringMap::const_iterator t = m_clientTypes.find( clientName );
            if ( t != m_clientTypes.end() )
                clientNode->SetAttribute( "type", (*t).second );
        }
        clientNode->SetAttribute( "name", clientName );
        totalSuccess = clients[ i ]->SaveConfig( *clientNode ) && totalSuccess;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDiscoveryManager::LoadConfig( const CORE::CDataNode& treeroot )
{GUCEF_TRACE;

    const CORE::CDataNode* managerNode = GUCEF_NULL;
    if ( treeroot.GetName() == "DiscoveryManager" )
        managerNode = &treeroot;
    else
        managerNode = treeroot.FindChild( "DiscoveryManager" );

    // Without a section of our own there is nothing for us in this config
    if ( GUCEF_NULL == managerNode )
        return true;

    bool totalSuccess = true;
    CORE::CDataNode::TConstDataNodeSet clientNodes = managerNode->FindChildrenOfType( "DiscoveryClient" );
    CORE::CDataNode::TConstDataNodeSet::iterator i = clientNodes.begin();
    while ( i != clientNodes.end() )
    {
        const CORE::CDataNode* clientNode = (*i);
        CORE::CString clientType = clientNode->GetAttributeValueOrChildValueByName( "type" ).AsString( CORE::CString::Empty, true );
        CORE::CString clientName = clientNode->GetAttributeValueOrChildValueByName( "name" ).AsString( clientType, true );

        CIDiscoveryClientPtr existing = GetDiscoveryClient( clientName );
        if ( !existing.IsNULL() )
        {
            totalSuccess = existing->LoadConfig( *clientNode ) && totalSuccess;
        }
        else
        if ( clientType.IsNULLOrEmpty() )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "DiscoveryManager: A DiscoveryClient in the config has no type" );
            totalSuccess = false;
        }
        else
        {
            totalSuccess = !CreateDiscoveryClient( clientType, clientNode ).IsNULL() && totalSuccess;
        }
        ++i;
    }
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDiscoveryManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::CORE::CDiscoveryManager";
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
