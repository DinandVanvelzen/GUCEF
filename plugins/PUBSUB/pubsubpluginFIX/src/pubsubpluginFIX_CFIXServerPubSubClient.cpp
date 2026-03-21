/*
 *  pubsubpluginFIX: Generic GUCEF COMCORE plugin for providing pubsub via FIX protocol
 *
 *  Copyright (C) 1998 - 2024.  Dinand Vanvelzen
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

#include <string.h>

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#include "gucefPUBSUB_CPubSubGlobal.h"
#define GUCEF_PUBSUB_CPUBSUBGLOBAL_H
#endif /* GUCEF_PUBSUB_CPUBSUBGLOBAL_H ? */

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#include "CTCPConnection.h"
#define GUCEF_COMCORE_CTCPCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

#include "pubsubpluginFIX_CFIXServerPubSubClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PUBSUBPLUGIN {
namespace FIX {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CFIXServerPubSubClient::TypeName = "FIXServer";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFIXServerPubSubClient::CFIXServerPubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_fixConfig()
    , m_tcpServerSocket( config.pulseGenerator.IsNULL()
                             ? CORE::CCoreGlobal::Instance()->GetPulseGenerator()
                             : config.pulseGenerator
                         , false /* non-blocking */ )
    , m_topicMap()
    , m_pendingMap()
    , m_lock()
    , m_initialized( false )
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXServerPubSubClient: Failed to load config at construction" );
    }

    if ( m_fixConfig.pulseGenerator.IsNULL() )
        m_fixConfig.pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    RegisterEventHandlers();
    m_initialized = true;
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClient::~CFIXServerPubSubClient()
{GUCEF_TRACE;

    Disconnect();

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->Shutdown();
        (*i).second.Unlink();
        ++i;
    }
    m_topicMap.clear();

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    TEventCallback cbConnected( this, &CFIXServerPubSubClient::OnClientConnected );
    SubscribeTo( &m_tcpServerSocket                                ,
                 COMCORE::CTCPServerSocket::ClientConnectedEvent   ,
                 cbConnected                                       );

    TEventCallback cbDisconnected( this, &CFIXServerPubSubClient::OnClientDisconnected );
    SubscribeTo( &m_tcpServerSocket                                   ,
                 COMCORE::CTCPServerSocket::ClientDisconnectedEvent   ,
                 cbDisconnected                                        );

    TEventCallback cbOpened( this, &CFIXServerPubSubClient::OnServerSocketOpened );
    SubscribeTo( &m_tcpServerSocket                                     ,
                 COMCORE::CTCPServerSocket::ServerSocketOpenedEvent     ,
                 cbOpened                                               );

    TEventCallback cbClosed( this, &CFIXServerPubSubClient::OnServerSocketClosed );
    SubscribeTo( &m_tcpServerSocket                                     ,
                 COMCORE::CTCPServerSocket::ServerSocketClosedEvent     ,
                 cbClosed                                               );

    TEventCallback cbError( this, &CFIXServerPubSubClient::OnServerSocketError );
    SubscribeTo( &m_tcpServerSocket                                     ,
                 COMCORE::CTCPServerSocket::ServerSocketErrorEvent      ,
                 cbError                                                );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator ,
                                            bool includeTopics                        )
{GUCEF_TRACE;

    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    m_fixConfig.pulseGenerator = newPulseGenerator;

    if ( includeTopics )
    {
        m_fixConfig.topicPulseGenerator = m_fixConfig.pulseGenerator;
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            (*i).second->SetPulseGenerator( newPulseGenerator );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
{GUCEF_TRACE;

    features.supportsBinaryPayloads              = true;
    features.supportsPerMsgIds                   = true;
    features.supportsPrimaryPayloadPerMsg        = true;
    features.supportsAbsentPrimaryPayloadPerMsg  = false;
    features.supportsKeyValueSetPerMsg           = true;
    features.supportsDuplicateKeysPerMsg         = false;
    features.supportsMetaDataKeyValueSetPerMsg   = true;
    features.supportsMultiHostSharding           = false;
    features.supportsPublishing                  = m_fixConfig.allowPublishing;
    features.supportsSubscribing                 = true;
    features.supportsMetrics                     = false;
    features.supportsAutoReconnect               = false;   // server: clients reconnect to us
    features.supportsAckUsingBookmark            = false;
    features.supportsSubscriberMsgReceivedAck    = false;
    features.supportsAutoMsgReceivedAck          = false;
    features.supportsAbsentMsgReceivedAck        = false;
    features.supportsAckUsingLastMsgInBatch      = false;
    features.supportsBookmarkingConcept          = false;
    features.supportsAutoBookmarking             = false;
    features.supportsMsgIdBasedBookmark          = false;
    features.supportsMsgIndexBasedBookmark       = false;
    features.supportsMsgDateTimeBasedBookmark    = false;
    features.supportsServerSideBookmarkPersistance = false;
    features.supportsSubscribingUsingBookmark    = false;
    features.supportsTopicIndexBasedBookmark     = false;
    features.supportsDerivingBookmarkFromMsg     = false;
    features.supportsDiscoveryOfAvailableTopics  = false;
    features.supportsGlobPatternTopicNames       = false;
    features.supportsSubscriptionMsgArrivalDelayRequests = false;
    features.supportsSubscriptionEndOfDataEvent  = false;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CFIXServerPubSubClient::GetTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_fixConfig.topics.begin();
    while ( i != m_fixConfig.topics.end() )
    {
        if ( topicName == (*i)->topicName )
            return (*i);
        ++i;
    }
    return PUBSUB::CPubSubClientTopicConfigPtr();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CFIXServerPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CFIXServerPubSubClientTopicConfigPtr newTopicConfig = CFIXServerPubSubClientTopicConfig::CreateSharedObj();
    if ( !newTopicConfig.IsNULL() &&
         !m_fixConfig.defaultTopicConfig.IsNULL() &&
         newTopicConfig->LoadConfig( *m_fixConfig.defaultTopicConfig ) )
    {
        newTopicConfig->topicName = topicName;
        m_fixConfig.topics.push_back( newTopicConfig );
    }
    return newTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CFIXServerPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_fixConfig.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CFIXServerPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                            CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    CFIXServerPubSubClientTopicPtr topicAccess;
    {
        MT::CObjectScopeLock lock( this );

        topicAccess = ( GUCEF_NEW CFIXServerPubSubClientTopic( this ) )->CreateSharedPtr();
        if ( topicAccess->LoadConfig( *topicConfig ) )
        {
            m_topicMap[ topicConfig->topicName ] = topicAccess;
        }
        else
        {
            topicAccess->Shutdown();
            topicAccess.Unlink();
        }
    }

    if ( !topicAccess.IsNULL() )
    {
        TopicAccessCreatedEventData eData( topicConfig->topicName );
        NotifyObservers( TopicAccessCreatedEvent, &eData );
    }
    return topicAccess;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CFIXServerPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
        return (*i).second;
    return PUBSUB::CPubSubClientTopicPtr();
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        CFIXServerPubSubClientTopicPtr topicAccess = (*i).second;
        m_topicMap.erase( i );

        TopicAccessDestroyedEventData eData( topicName );
        NotifyObservers( TopicAccessDestroyedEvent, &eData );

        topicAccess->Shutdown();
        topicAccess.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientConfig::TPubSubClientTopicConfigPtrVector::iterator i = m_fixConfig.topics.begin();
    while ( i != m_fixConfig.topics.end() )
    {
        topicNameList.insert( (*i)->topicName );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
{GUCEF_TRACE;

    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicNameList.insert( (*i).first );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        topicAccess.insert( (*i).second );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::Connect( bool reset )
{GUCEF_TRACE;

    if ( !reset && m_tcpServerSocket.IsActive() )
        return true;

    if ( m_tcpServerSocket.IsActive() )
        m_tcpServerSocket.Close();

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClient::Connect: Starting FIX server on port " +
        CORE::ToString( (CORE::UInt32) m_fixConfig.listenPort ) );

    if ( !m_tcpServerSocket.ListenOnPort( m_fixConfig.listenPort ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT,
            "CFIXServerPubSubClient::Connect: Failed to listen on port " +
            CORE::ToString( (CORE::UInt32) m_fixConfig.listenPort ) );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    // Remove all pending connections first
    m_pendingMap.clear();

    if ( m_tcpServerSocket.IsActive() )
        m_tcpServerSocket.Close();

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;

    return m_tcpServerSocket.IsActive();
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;

    return m_tcpServerSocket.IsActive();
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXServerPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXServerPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::FIX::CFIXServerPubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    return m_fixConfig.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    return m_fixConfig.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CFIXServerPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        m_fixConfig = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg )
{GUCEF_TRACE;

    CFIXServerPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        m_fixConfig = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CFIXServerPubSubClientConfig&
CFIXServerPubSubClient::GetConfig( void ) const
{GUCEF_TRACE;

    return m_fixConfig;
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::SubscribeToPendingConnection( COMCORE::CTCPServerConnection* connection ,
                                                       CORE::UInt32 connIdx                      )
{GUCEF_TRACE;

    if ( GUCEF_NULL == connection )
        return;

    TEventCallback cbData( this, &CFIXServerPubSubClient::OnPendingConnectionData );
    SubscribeTo( connection                                ,
                 COMCORE::CTCPConnection::DataRecievedEvent ,
                 cbData                                    );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::UnsubscribeFromPendingConnection( COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    if ( GUCEF_NULL == connection )
        return;

    UnsubscribeFrom( connection );
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClient::TryScanLogon( const CORE::CDynamicBuffer& buffer     ,
                                       CFIXClientSessionFields& outFields     ,
                                       CORE::UInt32& outLogonLen              ) const
{GUCEF_TRACE;

    const char* bufPtr = static_cast< const char* >( buffer.GetConstBufferPtr() );
    CORE::UInt32 bufLen = buffer.GetDataSize();

    if ( bufLen < 15 )
        return false;  // incomplete

    // Must start with "8="
    if ( bufPtr[ 0 ] != '8' || bufPtr[ 1 ] != '=' )
        return false;

    const char* bufEnd = bufPtr + bufLen;

    // Find tag 8 value end
    const char* tag8ValStart = bufPtr + 2;
    const char* tag8ValEnd   = tag8ValStart;
    while ( tag8ValEnd < bufEnd && *tag8ValEnd != CFIXClientMessage::SOH )
        ++tag8ValEnd;
    if ( tag8ValEnd >= bufEnd )
        return false;  // incomplete

    // Locate tag 9
    const char* afterTag8 = tag8ValEnd + 1;
    const char* tag9Pos   = GUCEF_NULL;
    {
        const char* scanPos = afterTag8;
        const char* scanEnd = afterTag8 + 20;
        if ( scanEnd > bufEnd ) scanEnd = bufEnd;
        while ( scanPos + 1 < scanEnd )
        {
            if ( scanPos[ 0 ] == '9' && scanPos[ 1 ] == '=' &&
                 ( scanPos == bufPtr || *( scanPos - 1 ) == CFIXClientMessage::SOH ) )
            {
                tag9Pos = scanPos;
                break;
            }
            ++scanPos;
        }
    }
    if ( GUCEF_NULL == tag9Pos )
        return false;

    const char* bodyLenStart = tag9Pos + 2;
    const char* bodyLenEnd   = bodyLenStart;
    while ( bodyLenEnd < bufEnd && *bodyLenEnd != CFIXClientMessage::SOH )
        ++bodyLenEnd;
    if ( bodyLenEnd >= bufEnd )
        return false;

    CORE::UInt32 bodyLen = (CORE::UInt32) CFIXClientMessage::ParseUInt64Inline(
        bodyLenStart, (CORE::UInt32)( bodyLenEnd - bodyLenStart ) );

    // [S1] reject oversized
    if ( bodyLen > m_fixConfig.maxMsgSizeBytes )
        return false;

    const char* bodyStart = bodyLenEnd + 1;
    CORE::UInt64 safeEnd = (CORE::UInt64)( bodyStart - bufPtr ) +
                           (CORE::UInt64)bodyLen +
                           (CORE::UInt64)7;
    if ( safeEnd > (CORE::UInt64)bufLen )
        return false;  // incomplete

    // Find tag 10
    const char* tag10Pos = bodyStart + bodyLen;
    if ( (CORE::UInt32)( bufEnd - tag10Pos ) < 3 ||
         tag10Pos[ 0 ] != '1' || tag10Pos[ 1 ] != '0' || tag10Pos[ 2 ] != '=' )
        return false;

    const char* checksumValStart = tag10Pos + 3;
    const char* checksumValEnd   = checksumValStart;
    while ( checksumValEnd < bufEnd && *checksumValEnd != CFIXClientMessage::SOH )
        ++checksumValEnd;
    if ( checksumValEnd >= bufEnd )
        return false;

    const char* msgEnd = checksumValEnd + 1;
    CORE::UInt32 msgLen = (CORE::UInt32)( msgEnd - bufPtr );

    // Scan session fields
    if ( !CFIXClientMessage::ScanSessionFields( bufPtr, msgLen, outFields ) )
        return false;

    // Must be a Logon (MsgType=A)
    if ( GUCEF_NULL == outFields.msgTypeStart ||
         !CFIXClientMessage::FieldMatchesValue( outFields.msgTypeStart, outFields.msgTypeLen, "A" ) )
        return false;

    outLogonLen = msgLen;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::PromotePendingToTopic( CORE::UInt32 connIdx                   ,
                                                COMCORE::CTCPServerConnection* conn     ,
                                                const CFIXClientSessionFields& fields   ,
                                                CORE::UInt32 logonLen                   )
{GUCEF_TRACE;

    // Determine session key = remote SenderCompID
    CORE::CString sessionKey;
    if ( GUCEF_NULL != fields.senderStart && fields.senderLen > 0 )
        sessionKey = CORE::CString( fields.senderStart, fields.senderLen );
    else
        sessionKey = "UNKNOWN_" + CORE::ToString( connIdx );

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClient::PromotePendingToTopic: Logon identified from " +
        sessionKey + " (connIdx=" + CORE::ToString( connIdx ) + ")" );

    // Remove from pending map and unsubscribe (topic will subscribe its own events)
    TPendingMap::iterator pit = m_pendingMap.find( connIdx );
    CORE::CDynamicBuffer logonData;
    if ( pit != m_pendingMap.end() )
    {
        logonData.SetDataSize( logonLen );
        ::memcpy( logonData.GetBufferPtr(),
                  pit->second.buffer.GetConstBufferPtr(),
                  logonLen );
        UnsubscribeFromPendingConnection( conn );
        m_pendingMap.erase( pit );
    }

    // Find or create topic for this session key
    TTopicMap::iterator ti = m_topicMap.find( sessionKey );
    if ( ti == m_topicMap.end() )
    {
        // Auto-create topic
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClient::PromotePendingToTopic: Auto-creating topic for " + sessionKey );

        CFIXServerPubSubClientTopicPtr newTopic = ( GUCEF_NEW CFIXServerPubSubClientTopic( this ) )->CreateSharedPtr();
        // Apply default topic config if available
        if ( !m_fixConfig.defaultTopicConfig.IsNULL() )
            newTopic->LoadConfig( *m_fixConfig.defaultTopicConfig );

        // Override the topicName with the sessionKey
        PUBSUB::CPubSubClientTopicConfig tmpCfg;
        newTopic->SaveConfig( tmpCfg );
        tmpCfg.topicName = sessionKey;
        newTopic->LoadConfig( tmpCfg );

        m_topicMap[ sessionKey ] = newTopic;
        ti = m_topicMap.find( sessionKey );

        // Notify that a new topic was auto-created
        TopicAccessCreatedEventData eData( sessionKey );
        NotifyObservers( TopicAccessCreatedEvent, &eData );
    }

    // Attach the connection to the topic (handles session bump internally)
    ti->second->AttachConnection( conn, logonData.GetConstBufferPtr(), logonLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::OnClientConnected( CORE::CNotifier* notifier    ,
                                            const CORE::CEvent& eventId  ,
                                            CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPServerSocket::TClientConnectedEventData* connEvent =
        static_cast< COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData );
    if ( GUCEF_NULL == connEvent )
        return;

    const COMCORE::CTCPServerSocket::TConnectionInfo& connInfo = connEvent->GetData();
    if ( GUCEF_NULL == connInfo.connection )
        return;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClient::OnClientConnected: New connection index=" +
        CORE::ToString( connInfo.connectionIndex ) +
        " from " + connInfo.hostAddress.GetFirstAddressAndPortAsString() );

    // Add to pending map — waiting for Logon
    SPendingConnectionState& pending = m_pendingMap[ connInfo.connectionIndex ];
    pending.connection = connInfo.connection;
    pending.buffer.Clear();

    SubscribeToPendingConnection( connInfo.connection, connInfo.connectionIndex );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::OnClientDisconnected( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPServerSocket::TClientDisconnectedEventData* discEvent =
        static_cast< COMCORE::CTCPServerSocket::TClientDisconnectedEventData* >( eventData );
    if ( GUCEF_NULL == discEvent )
        return;

    const COMCORE::CTCPServerSocket::TDisconnectInfo& discInfo = discEvent->GetData();
    CORE::UInt32 connIdx = discInfo.connectionInfo.connectionIndex;

    // If still in pending map, just remove it
    TPendingMap::iterator pit = m_pendingMap.find( connIdx );
    if ( pit != m_pendingMap.end() )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClient::OnClientDisconnected: Pending connection " +
            CORE::ToString( connIdx ) + " disconnected before Logon" );
        if ( GUCEF_NULL != pit->second.connection )
            UnsubscribeFromPendingConnection( pit->second.connection );
        m_pendingMap.erase( pit );
    }
    // Note: if the connection was already promoted to a topic, the topic handles its own
    // DisconnectedEvent subscription and will update its state there.
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::OnServerSocketOpened( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClient::OnServerSocketOpened: Server socket opened on port " +
        CORE::ToString( (CORE::UInt32) m_fixConfig.listenPort ) );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::OnServerSocketClosed( CORE::CNotifier* notifier    ,
                                               const CORE::CEvent& eventId  ,
                                               CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXServerPubSubClient::OnServerSocketClosed" );
    m_pendingMap.clear();
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::OnServerSocketError( CORE::CNotifier* notifier    ,
                                              const CORE::CEvent& eventId  ,
                                              CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXServerPubSubClient::OnServerSocketError: Server socket error" );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClient::OnPendingConnectionData( CORE::CNotifier* notifier    ,
                                                  const CORE::CEvent& eventId  ,
                                                  CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPConnection::TDataRecievedEventData* dataEvent =
        static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData );
    if ( GUCEF_NULL == dataEvent )
        return;

    // Identify which pending connection this data belongs to
    COMCORE::CTCPServerConnection* conn =
        static_cast< COMCORE::CTCPServerConnection* >( notifier );
    if ( GUCEF_NULL == conn )
        return;

    CORE::UInt32 connIdx = conn->GetConnectionIndex();

    TPendingMap::iterator pit = m_pendingMap.find( connIdx );
    if ( pit == m_pendingMap.end() )
        return;  // not in our pending map (already promoted or unknown)

    const CORE::CDynamicBuffer& incoming = dataEvent->GetData();
    if ( incoming.GetDataSize() == 0 )
        return;

    // Append to pending buffer
    SPendingConnectionState& pending = pit->second;
    CORE::UInt32 existingSize = pending.buffer.GetDataSize();

    // [S1] guard: reject excessively large pre-Logon buffers
    if ( existingSize + incoming.GetDataSize() > m_fixConfig.maxMsgSizeBytes )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClient::OnPendingConnectionData: Pending buffer exceeded maxMsgSizeBytes for connIdx=" +
            CORE::ToString( connIdx ) + " - closing" );
        UnsubscribeFromPendingConnection( conn );
        conn->Close();
        m_pendingMap.erase( pit );
        return;
    }

    pending.buffer.SetDataSize( existingSize + incoming.GetDataSize() );
    ::memcpy( static_cast< char* >( pending.buffer.GetBufferPtr() ) + existingSize,
              incoming.GetConstBufferPtr(),
              incoming.GetDataSize() );

    // Try to find a complete Logon
    CFIXClientSessionFields fields;
    CORE::UInt32 logonLen = 0;
    if ( TryScanLogon( pending.buffer, fields, logonLen ) )
    {
        PromotePendingToTopic( connIdx, conn, fields, logonLen );
    }
    // else: wait for more data
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace FIX */
}; /* namespace PUBSUBPLUGIN */
}; /* namespace GUCEF */

/*--------------------------------------------------------------------------*/
