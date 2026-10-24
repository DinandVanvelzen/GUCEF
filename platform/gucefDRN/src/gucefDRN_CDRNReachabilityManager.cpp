/*
 *  gucefDRN: GUCEF module providing RAD networking trough data replication
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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_COMCORE_CCOMCOREGLOBAL_H
#include "gucefCOMCORE_CComCoreGlobal.h"
#define GUCEF_COMCORE_CCOMCOREGLOBAL_H
#endif /* GUCEF_COMCORE_CCOMCOREGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CPORTMAPPINGRESULT_H
#include "gucefCOMCORE_CPortMappingResult.h"
#define GUCEF_COMCORE_CPORTMAPPINGRESULT_H
#endif /* GUCEF_COMCORE_CPORTMAPPINGRESULT_H ? */

#ifndef GUCEF_COM_CSTUNCLIENT_H
#include "gucefCOM_CStunClient.h"
#define GUCEF_COM_CSTUNCLIENT_H
#endif /* GUCEF_COM_CSTUNCLIENT_H ? */

#ifndef GUCEF_DRN_CDRNCLOCK_H
#include "gucefDRN_CDRNClock.h"
#define GUCEF_DRN_CDRNCLOCK_H
#endif /* GUCEF_DRN_CDRNCLOCK_H ? */

#ifndef GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#include "gucefDRN_CDRNUdpChannelListener.h"
#define GUCEF_DRN_CDRNUDPCHANNELLISTENER_H
#endif /* GUCEF_DRN_CDRNUDPCHANNELLISTENER_H ? */

#ifndef GUCEF_DRN_CDRNNODE_H
#include "gucefDRN_CDRNNode.h"
#define GUCEF_DRN_CDRNNODE_H
#endif /* GUCEF_DRN_CDRNNODE_H ? */

#include "gucefDRN_CDRNReachabilityManager.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace DRN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/** Adds an endpoint to a list unless the list has it already */
static void
AddOnce( CDRNEndpointList& list, const CDRNEndpoint& endpoint )
{GUCEF_TRACE;

    CORE::CString text = endpoint.ToString();
    for ( UInt32 i=0; i<list.GetCount(); ++i )
        if ( list.Get( i ).ToString() == text )
            return;
    list.Add( endpoint );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CDRNReachabilitySettings::ClassTypeName = "GUCEF::DRN::CDRNReachabilitySettings";
const CORE::CEvent CDRNReachabilityManager::ReachabilityChangedEvent = "GUCEF::DRN::CDRNReachabilityManager::ReachabilityChangedEvent";
const CORE::CEvent CDRNReachabilityManager::ReachabilityCompleteEvent = "GUCEF::DRN::CDRNReachabilityManager::ReachabilityCompleteEvent";

/*-------------------------------------------------------------------------*/

CDRNReachabilitySettings::CDRNReachabilitySettings( void )
    : CORE::CIConfigurable()
    , portMappers()
    , stunServers()
    , relays()
    , relayRealm()
    , mappingLeaseInSecs( 3600 )
    , timeoutMs( 5000 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNReachabilitySettings::CDRNReachabilitySettings( const CDRNReachabilitySettings& src )
    : CORE::CIConfigurable( src )
    , portMappers( src.portMappers )
    , stunServers( src.stunServers )
    , relays( src.relays )
    , relayRealm( src.relayRealm )
    , mappingLeaseInSecs( src.mappingLeaseInSecs )
    , timeoutMs( src.timeoutMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNReachabilitySettings::~CDRNReachabilitySettings()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CDRNReachabilitySettings&
CDRNReachabilitySettings::operator=( const CDRNReachabilitySettings& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        CORE::CIConfigurable::operator=( src );
        portMappers = src.portMappers;
        stunServers = src.stunServers;
        relays = src.relays;
        relayRealm = src.relayRealm;
        mappingLeaseInSecs = src.mappingLeaseInSecs;
        timeoutMs = src.timeoutMs;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

void
CDRNReachabilitySettings::AddPortMapper( const CORE::CString& name, const CORE::CDataNode& config )
{GUCEF_TRACE;

    CPortMapperConfig mapper;
    mapper.name = name;
    mapper.config = config;
    portMappers.push_back( mapper );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNReachabilitySettings::GetClassTypeName( void ) const
{GUCEF_TRACE;

    return ClassTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReachabilitySettings::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    bool totalSuccess = true;
    totalSuccess = cfg.SetAttribute( "relayRealm", relayRealm ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "mappingLeaseInSecs", mappingLeaseInSecs ) && totalSuccess;
    totalSuccess = cfg.SetAttribute( "timeoutMs", timeoutMs ) && totalSuccess;
    for ( UInt32 i=0; i<portMappers.size(); ++i )
    {
        CORE::CDataNode* mapperNode = cfg.AddChild( "PortMapper" );
        mapperNode->SetAttribute( "name", portMappers[ i ].name );
        CORE::CDataNode* configNode = mapperNode->AddChild( portMappers[ i ].config );
        configNode->SetName( "Config" );
    }
    for ( UInt32 i=0; i<stunServers.size(); ++i )
        cfg.AddChildWithValue( "StunServer", CORE::CVariant( stunServers[ i ] ) );
    CORE::CDataNode* relaysNode = cfg.FindOrAddChild( "Relays" );
    totalSuccess = GUCEF_NULL != relaysNode && relays.SaveConfig( *relaysNode ) && totalSuccess;
    return totalSuccess;
}

/*-------------------------------------------------------------------------*/

bool
CDRNReachabilitySettings::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    relayRealm = cfg.GetAttributeValueOrChildValueByName( "relayRealm" ).AsString( relayRealm, true );
    mappingLeaseInSecs = cfg.GetAttributeValueOrChildValueByName( "mappingLeaseInSecs" ).AsUInt32( mappingLeaseInSecs, true );
    timeoutMs = cfg.GetAttributeValueOrChildValueByName( "timeoutMs" ).AsUInt32( timeoutMs, true );

    portMappers.clear();
    stunServers.clear();
    CORE::CDataNode::const_iterator i = cfg.ConstBegin();
    while ( i != cfg.ConstEnd() )
    {
        const CORE::CDataNode* child = (*i);
        ++i;
        if ( child->GetName() == "PortMapper" )
        {
            CPortMapperConfig mapper;
            mapper.name = child->GetAttributeValue( "name" ).AsString( CORE::CString::Empty, true );
            const CORE::CDataNode* configNode = child->FindChild( "Config" );
            if ( GUCEF_NULL != configNode )
                mapper.config = *configNode;
            if ( !mapper.name.IsNULLOrEmpty() )
                portMappers.push_back( mapper );
        }
        else
        if ( child->GetName() == "StunServer" )
        {
            CORE::CString server = child->GetValue().AsString( CORE::CString::Empty, true );
            if ( !server.IsNULLOrEmpty() )
                stunServers.push_back( server );
        }
    }
    const CORE::CDataNode* relaysNode = cfg.FindChild( "Relays" );
    if ( GUCEF_NULL != relaysNode )
        relays.LoadConfig( *relaysNode );
    return true;
}

/*-------------------------------------------------------------------------*/

CDRNReachabilityManager::CQuestion::CQuestion( void )
    : future( CORE::TTaskStatus::TASKSTATUS_UNDEFINED )
    , isStun( false )
    , channelType()
    , path()
    , answered( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CDRNReachabilityManager::CInternalObserver::OnNotify( CORE::CNotifier* /* notifier */, const CORE::CEvent& eventid, CORE::CICloneable* /* eventdata */ )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_manager && CORE::CPulseGenerator::PulseEvent == eventid )
        m_manager->OnPulse();
}

/*-------------------------------------------------------------------------*/

void
CDRNReachabilityManager::RegisterEvents( void )
{GUCEF_TRACE;

    ReachabilityChangedEvent.Initialize();
    ReachabilityCompleteEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CDRNReachabilityManager::CDRNReachabilityManager( CDRNNode& node )
    : CORE::CTSGNotifier( node.GetAppPulseGenerator(), true, true )
    , m_node( &node )
    , m_observer()
    , m_settings()
    , m_mappers()
    , m_stunClients()
    , m_stunListeners()
    , m_questions()
    , m_mapped()
    , m_reflexive()
    , m_deadline( 0 )
    , m_started( false )
    , m_complete( false )
    , m_lastList()
{GUCEF_TRACE;

    RegisterEvents();
    m_observer.m_manager = this;
}

/*-------------------------------------------------------------------------*/

CDRNReachabilityManager::~CDRNReachabilityManager()
{GUCEF_TRACE;

    Stop();
    m_observer.m_manager = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CDRNReachabilityManager::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::DRN::CDRNReachabilityManager";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CDRNReachabilityManager::Emit( const CORE::CEvent& eventid )
{GUCEF_TRACE;

    if ( m_node->IsRunningOwnThread() )
        NotifyObserversFromThread( eventid );
    else
        NotifyObservers( eventid );
}

/*-------------------------------------------------------------------------*/

bool
CDRNReachabilityManager::Start( const CDRNReachabilitySettings& settings )
{GUCEF_TRACE;

    Stop();

    MT::CScopeMutex lock( m_node->GetLock() );
    m_settings = settings;
    m_deadline = CDRNClock::NowInMicroseconds() + (UInt64) settings.timeoutMs * 1000;
    m_complete = false;
    m_lastList.Clear();

    // the port mappers, for every UDP and TCP listener
    for ( UInt32 m=0; m<settings.portMappers.size(); ++m )
    {
        const CDRNReachabilitySettings::CPortMapperConfig& mapperConfig = settings.portMappers[ m ];
        COMCORE::CIPortMapperPtr mapper = COMCORE::CComCoreGlobal::Instance()->GetPortMapperFactory().Create( mapperConfig.name );
        if ( mapper.IsNULL() )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "DRNReachabilityManager: There is no port mapper \"" << mapperConfig.name << "\"" );
            continue;
        }
        if ( mapperConfig.config.GetAttCount() > 0 || mapperConfig.config.GetNrOfChildNodes() > 0 )
            mapper->LoadConfig( mapperConfig.config );
        m_mappers.push_back( mapper );

        for ( UInt32 i=0; i<m_node->m_listeners.size(); ++i )
        {
            const CIDRNChannelListenerPtr& listener = m_node->m_listeners[ i ];
            if ( !listener->IsListening() )
                continue;
            CDRNEndpoint endpoint = listener->GetListenEndpoint();
            CORE::CString type = endpoint.GetChannelType();
            if ( type != CDRNEndpoint::ChannelTypeUdp && type != CDRNEndpoint::ChannelTypeTcp && type != CDRNEndpoint::ChannelTypeWebSocket )
                continue;

            CQuestion question;
            question.channelType = type;
            question.path = endpoint.GetPath();
            COMCORE::CPortMappingResult::TProtocol protocol = type == CDRNEndpoint::ChannelTypeUdp ? COMCORE::CPortMappingResult::PROTOCOL_UDP : COMCORE::CPortMappingResult::PROTOCOL_TCP;
            if ( mapper->RequestMapping( protocol, endpoint.GetPort(), endpoint.GetPort(), settings.mappingLeaseInSecs, question.future ) )
                m_questions.push_back( question );
        }
    }

    // STUN, for every UDP listener: the reflexive address of its socket
    if ( !settings.stunServers.empty() )
    {
        for ( UInt32 i=0; i<m_node->m_listeners.size(); ++i )
        {
            const CIDRNChannelListenerPtr& listener = m_node->m_listeners[ i ];
            if ( !listener->IsListening() || listener->GetChannelType() != CDRNEndpoint::ChannelTypeUdp )
                continue;
            COMCORE::CUDPSocket* socket = static_cast< const CDRNUdpChannelListener* >( listener.GetPointerAlways() )->GetSocket();
            if ( GUCEF_NULL == socket )
                continue;
            COM::CStunClient* stun = GUCEF_NEW COM::CStunClient( *socket, m_node->GetPulseGenerator() );
            m_stunClients.push_back( stun );
            m_stunListeners.push_back( listener );
            for ( UInt32 s=0; s<settings.stunServers.size(); ++s )
            {
                CQuestion question;
                question.isStun = true;
                question.channelType = CDRNEndpoint::ChannelTypeUdp;
                if ( stun->QueryReflexiveAddress( COMCORE::CHostAddress( settings.stunServers[ s ] ), question.future ) )
                    m_questions.push_back( question );
            }
        }
    }

    // the relays, the last resort
    for ( UInt32 i=0; i<settings.relays.GetCount(); ++i )
        m_node->BindAtRelay( settings.relays.Get( i ), settings.relayRealm );

    m_observer.SubscribeTo( m_node->GetPulseGenerator().GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
    m_started = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CDRNReachabilityManager::Stop( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    m_observer.UnsubscribeAllFromObserver();
    m_questions.clear();
    for ( UInt32 i=0; i<m_stunClients.size(); ++i )
        GUCEF_DELETE m_stunClients[ i ];
    m_stunClients.clear();
    m_stunListeners.clear();
    m_mappers.clear();
    m_mapped.Clear();
    m_reflexive.Clear();
    if ( m_started )
        m_node->SetAdvertisedEndpoints( CDRNEndpointList() );
    m_started = false;
}

/*-------------------------------------------------------------------------*/

void
CDRNReachabilityManager::OnPulse( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    if ( !m_started )
        return;

    bool allAnswered = true;
    for ( UInt32 i=0; i<m_questions.size(); ++i )
    {
        CQuestion& question = m_questions[ i ];
        if ( question.answered )
            continue;
        if ( !question.future.IsReady() && !question.future.HasNoFuture() )
        {
            allAnswered = false;
            continue;
        }
        question.answered = true;
        if ( question.isStun )
        {
            COM::CStunClient::CStunResult result;
            if ( COM::CStunClient::TryGetResultFromFuture( question.future, result ) )
                AddOnce( m_reflexive, CDRNEndpoint( CDRNEndpoint::ChannelTypeUdp, result.reflexiveAddress.AddressAndPortAsString() ) );
        }
        else
        {
            COMCORE::CPortMappingResult result;
            if ( COMCORE::CPortMappingResult::TryGetFromFuture( question.future, result ) && 0 != result.externalPort )
                AddOnce( m_mapped, CDRNEndpoint( question.channelType, result.externalAddress.AddressAsString() + ":" + CORE::UInt32ToString( result.externalPort ) + question.path ) );
        }
    }
    bool relaysBound = true;
    for ( UInt32 i=0; i<m_settings.relays.GetCount(); ++i )
        relaysBound = relaysBound && m_node->IsBoundAtRelay( m_settings.relays.Get( i ), m_settings.relayRealm );

    CDRNEndpointList endpoints = GetEndpoints();
    CORE::CString list = endpoints.ToString();
    bool changed = list != m_lastList;
    m_lastList = list;
    if ( changed )
        m_node->SetAdvertisedEndpoints( endpoints );
    bool completeNow = !m_complete && ( ( allAnswered && relaysBound ) || CDRNClock::NowInMicroseconds() >= m_deadline );
    if ( completeNow )
        m_complete = true;

    if ( changed )
        Emit( ReachabilityChangedEvent );
    if ( completeNow )
    {
        GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "DRNReachabilityManager: The node can be reached at " << list );
        Emit( ReachabilityCompleteEvent );
    }
}

/*-------------------------------------------------------------------------*/

bool
CDRNReachabilityManager::IsComplete( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    return m_complete;
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNReachabilityManager::GetEndpoints( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    CDRNEndpointList endpoints;
    CDRNEndpointList listening = m_node->GetListenEndpoints();
    for ( UInt32 i=0; i<listening.GetCount(); ++i )
        if ( listening.Get( i ).GetChannelType() != CDRNEndpoint::ChannelTypeRelay )
            AddOnce( endpoints, listening.Get( i ) );
    for ( UInt32 i=0; i<m_mapped.GetCount(); ++i )
        AddOnce( endpoints, m_mapped.Get( i ) );
    for ( UInt32 i=0; i<m_reflexive.GetCount(); ++i )
        AddOnce( endpoints, m_reflexive.Get( i ) );
    CDRNEndpointList relayed = m_node->GetRelayEndpoints();
    for ( UInt32 i=0; i<relayed.GetCount(); ++i )
        AddOnce( endpoints, relayed.Get( i ) );
    return endpoints;
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNReachabilityManager::GetMappedEndpoints( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    return m_mapped;
}

/*-------------------------------------------------------------------------*/

CDRNEndpointList
CDRNReachabilityManager::GetReflexiveEndpoints( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_node->GetLock() );
    return m_reflexive;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace DRN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
