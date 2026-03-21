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

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#include "CTCPConnection.h"
#define GUCEF_COMCORE_CTCPCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "pubsubpluginFIX_CFIXPubSubClient.h"

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

const CORE::CString CFIXPubSubClient::TypeName = "FIX";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFIXPubSubClient::CFIXPubSubClient( const PUBSUB::CPubSubClientConfig& config )
    : PUBSUB::CPubSubClient( config.pulseGenerator )
    , m_fixConfig()
    , m_tcpSocket( config.pulseGenerator.IsNULL()
                       ? CORE::CCoreGlobal::Instance()->GetPulseGenerator()
                       : config.pulseGenerator
                   , false )
    , m_receiveBuffer()
    , m_topicMap()
    , m_heartbeatTimer( GUCEF_NULL )
    , m_logonTimeoutTimer( GUCEF_NULL )
    , m_reconnectTimer( GUCEF_NULL )
    , m_outgoingSeqNum( 1 )
    , m_expectedIncomingSeqNum( 1 )
    , m_sessionState( STATE_DISCONNECTED )
    , m_lock()
    , m_initialized( false )
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXPubSubClient: Failed to load config at construction" );
    }

    if ( m_fixConfig.pulseGenerator.IsNULL() )
        m_fixConfig.pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();

    m_heartbeatTimer    = GUCEF_NEW CORE::CTimer( m_fixConfig.pulseGenerator, m_fixConfig.heartbeatIntervalSecs * 1000 );
    m_logonTimeoutTimer = GUCEF_NEW CORE::CTimer( m_fixConfig.pulseGenerator, m_fixConfig.logonTimeoutInMs );
    m_reconnectTimer    = GUCEF_NEW CORE::CTimer( m_fixConfig.pulseGenerator, m_fixConfig.reconnectDelayInMs > 0 ? m_fixConfig.reconnectDelayInMs : 5000 );

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_reconnectTimer->SetEnabled( false );

    RegisterEventHandlers();
    m_initialized = true;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClient::~CFIXPubSubClient()
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

    GUCEF_DELETE m_heartbeatTimer;
    m_heartbeatTimer = GUCEF_NULL;

    GUCEF_DELETE m_logonTimeoutTimer;
    m_logonTimeoutTimer = GUCEF_NULL;

    GUCEF_DELETE m_reconnectTimer;
    m_reconnectTimer = GUCEF_NULL;

    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator,
                                      bool includeTopics )
{GUCEF_TRACE;

    CORE::CTSGNotifier::SetPulseGenerator( newPulseGenerator );
    m_fixConfig.pulseGenerator = newPulseGenerator;

    // Note: CTCPClientSocket pulse generator is set at construction time and cannot be changed

    if ( GUCEF_NULL != m_heartbeatTimer )
        m_heartbeatTimer->SetPulseGenerator( newPulseGenerator );
    if ( GUCEF_NULL != m_logonTimeoutTimer )
        m_logonTimeoutTimer->SetPulseGenerator( newPulseGenerator );
    if ( GUCEF_NULL != m_reconnectTimer )
        m_reconnectTimer->SetPulseGenerator( newPulseGenerator );

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

void
CFIXPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // TCP socket events
    TEventCallback cbConnected( this, &CFIXPubSubClient::OnTcpConnected );
    SubscribeTo( &m_tcpSocket                            ,
                 COMCORE::CTCPConnection::ConnectedEvent ,
                 cbConnected                            );

    TEventCallback cbDisconnected( this, &CFIXPubSubClient::OnTcpDisconnected );
    SubscribeTo( &m_tcpSocket                               ,
                 COMCORE::CTCPConnection::DisconnectedEvent ,
                 cbDisconnected                             );

    TEventCallback cbDataReceived( this, &CFIXPubSubClient::OnTcpDataReceived );
    SubscribeTo( &m_tcpSocket                               ,
                 COMCORE::CTCPConnection::DataRecievedEvent ,
                 cbDataReceived                             );

    TEventCallback cbSocketError( this, &CFIXPubSubClient::OnTcpSocketError );
    SubscribeTo( &m_tcpSocket                              ,
                 COMCORE::CTCPConnection::SocketErrorEvent ,
                 cbSocketError                             );

    // Heartbeat timer
    if ( GUCEF_NULL != m_heartbeatTimer )
    {
        TEventCallback cbHeartbeat( this, &CFIXPubSubClient::OnHeartbeatTimerCycle );
        SubscribeTo( m_heartbeatTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     cbHeartbeat                   );
    }

    // Logon timeout timer
    if ( GUCEF_NULL != m_logonTimeoutTimer )
    {
        TEventCallback cbLogonTimeout( this, &CFIXPubSubClient::OnLogonTimeoutTimerCycle );
        SubscribeTo( m_logonTimeoutTimer            ,
                     CORE::CTimer::TimerUpdateEvent ,
                     cbLogonTimeout                 );
    }

    // Reconnect timer
    if ( GUCEF_NULL != m_reconnectTimer )
    {
        TEventCallback cbReconnect( this, &CFIXPubSubClient::OnReconnectTimerCycle );
        SubscribeTo( m_reconnectTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     cbReconnect                    );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
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
    features.supportsAutoReconnect               = true;
    features.supportsAckUsingBookmark            = true;
    features.supportsSubscriberMsgReceivedAck    = true;
    features.supportsAutoMsgReceivedAck          = false;
    features.supportsAbsentMsgReceivedAck        = false;
    features.supportsAckUsingLastMsgInBatch      = true;
    features.supportsBookmarkingConcept          = true;
    features.supportsAutoBookmarking             = false;
    features.supportsMsgIdBasedBookmark          = true;
    features.supportsMsgIndexBasedBookmark       = true;
    features.supportsMsgDateTimeBasedBookmark    = false;
    features.supportsServerSideBookmarkPersistance = false;
    features.supportsSubscribingUsingBookmark    = true;
    features.supportsTopicIndexBasedBookmark     = false;
    features.supportsDerivingBookmarkFromMsg     = false;
    features.supportsDiscoveryOfAvailableTopics  = false;
    features.supportsGlobPatternTopicNames       = false;
    features.supportsSubscriptionMsgArrivalDelayRequests = false;
    features.supportsSubscriptionEndOfDataEvent  = false;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicBasicPtr
CFIXPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig,
                                      CORE::PulseGeneratorPtr pulseGenerator )
{GUCEF_TRACE;

    CFIXPubSubClientTopicPtr topicAccess;
    {
        MT::CObjectScopeLock lock( this );

        topicAccess = ( GUCEF_NEW CFIXPubSubClientTopic( this ) )->CreateSharedPtr();
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
CFIXPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
        return (*i).second;
    return PUBSUB::CPubSubClientTopicPtr();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        CFIXPubSubClientTopicPtr topicAccess = (*i).second;
        m_topicMap.erase( i );

        TopicAccessDestroyedEventData eData( topicName );
        NotifyObservers( TopicAccessDestroyedEvent, &eData );

        topicAccess->Shutdown();
        topicAccess.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CFIXPubSubClient::GetTopicConfig( const CORE::CString& topicName )
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
CFIXPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CFIXPubSubClientTopicConfigPtr newTopicConfig = CFIXPubSubClientTopicConfig::CreateSharedObj();
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
CFIXPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_fixConfig.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
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
CFIXPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
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
CFIXPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
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
CFIXPubSubClient::Connect( bool reset )
{GUCEF_TRACE;

    if ( !reset && m_sessionState == STATE_ACTIVE )
        return true;

    if ( m_sessionState != STATE_DISCONNECTED )
        Disconnect();

    // Load bookmark to restore sequence numbers
    LoadSequenceBookmark();

    if ( m_fixConfig.resetSeqNumOnLogon || reset )
    {
        m_outgoingSeqNum = 1;
        m_expectedIncomingSeqNum = 1;
    }

    // Get remote address from config
    if ( m_fixConfig.remoteAddresses.empty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXPubSubClient::Connect: No remote addresses configured" );
        return false;
    }

    const COMCORE::CHostAddress& remoteAddr = m_fixConfig.remoteAddresses.front();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::Connect: Connecting to " + remoteAddr.GetFirstAddressAndPortAsString() );

    m_sessionState = STATE_CONNECTING;
    bool connectOk = m_tcpSocket.ConnectTo( remoteAddr, false /* non-blocking */ );
    if ( !connectOk )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXPubSubClient::Connect: ConnectTo returned false" );
        m_sessionState = STATE_DISCONNECTED;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_reconnectTimer->SetEnabled( false );

    if ( m_sessionState == STATE_ACTIVE )
    {
        // Send Logout
        CORE::CString logoutMsg = CFIXMessage::BuildLogout(
            m_fixConfig.senderCompId, m_fixConfig.targetCompId,
            m_fixConfig.fixVersion, m_outgoingSeqNum );
        m_tcpSocket.Send( logoutMsg.C_String(), (CORE::UInt32) logoutMsg.Length() );
        ++m_outgoingSeqNum;
        m_sessionState = STATE_LOGGING_OUT;
    }

    m_tcpSocket.Close();
    m_sessionState = STATE_DISCONNECTED;
    m_receiveBuffer.Clear();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::IsSessionActive( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE ||
           m_sessionState == STATE_CONNECTING ||
           m_sessionState == STATE_LOGGING_IN;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::FIX::CFIXPubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    return m_fixConfig.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    return m_fixConfig.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CFIXPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        m_fixConfig = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg )
{GUCEF_TRACE;

    CFIXPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        m_fixConfig = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientConfig&
CFIXPubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_fixConfig;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClient::SendRawFix( const CORE::CString& rawMsg )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::SendRawFix: Session not active" );
        return false;
    }
    return m_tcpSocket.Send( rawMsg.C_String(), (CORE::UInt32) rawMsg.Length() );
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CFIXPubSubClient::GetAndIncrementOutgoingSeqNum( void )
{GUCEF_TRACE;

    return m_outgoingSeqNum++;
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::SaveSequenceBookmark( void )
{GUCEF_TRACE;

    if ( m_fixConfig.bookmarkPersistence.IsNULL() )
        return;

    // Encode both sequence numbers: 8 bytes each, little-endian
    CORE::CDynamicBuffer bookmarkData( 16, true );
    bookmarkData.SetDataSize( 16 );
    CORE::UInt64* pData = static_cast< CORE::UInt64* >( bookmarkData.GetBufferPtr() );
    pData[ 0 ] = m_outgoingSeqNum;
    pData[ 1 ] = m_expectedIncomingSeqNum;

    PUBSUB::CPubSubBookmark bookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
    bookmark.SetBookmarkData( CORE::CVariant( bookmarkData.GetConstBufferPtr(), 16, GUCEF_DATATYPE_BINARY_BLOB ) );
    bookmark.GetBookmarkDateTime() = CORE::CDateTime::NowUTCDateTime();

    if ( !m_topicMap.empty() )
    {
        CFIXPubSubClientTopicPtr topic = m_topicMap.begin()->second;
        if ( !topic.IsNULL() )
        {
            m_fixConfig.bookmarkPersistence->StoreBookmark( m_fixConfig.pubsubIdPrefix, *this, *topic, bookmark );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::LoadSequenceBookmark( void )
{GUCEF_TRACE;

    if ( m_fixConfig.bookmarkPersistence.IsNULL() || m_topicMap.empty() )
        return;

    CFIXPubSubClientTopicPtr topic = m_topicMap.begin()->second;
    if ( topic.IsNULL() )
        return;

    PUBSUB::CPubSubBookmark bookmark;
    if ( m_fixConfig.bookmarkPersistence->GetLatestBookmark( m_fixConfig.pubsubIdPrefix, *this, *topic, bookmark ) )
    {
        const CORE::CVariant& data = bookmark.GetBookmarkData();
        if ( data.ByteSize() >= 16 )
        {
            const CORE::UInt64* pData = static_cast< const CORE::UInt64* >( data.AsVoidPtr() );
            if ( GUCEF_NULL != pData )
            {
                m_outgoingSeqNum         = pData[ 0 ];
                m_expectedIncomingSeqNum = pData[ 1 ];
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::LoadSequenceBookmark: Restored outgoing=" +
                    CORE::ToString( m_outgoingSeqNum ) + " incoming=" + CORE::ToString( m_expectedIncomingSeqNum ) );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::RequestReplayFrom( CORE::UInt64 fromSeqNum )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE )
        return;

    CORE::CString resendReq = CFIXMessage::BuildResendRequest(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum, fromSeqNum, 0 );
    ++m_outgoingSeqNum;
    m_tcpSocket.Send( resendReq.C_String(), (CORE::UInt32) resendReq.Length() );
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::ScheduleReconnect( void )
{GUCEF_TRACE;

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_reconnectTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::ProcessReceiveBuffer( void )
{GUCEF_TRACE;

    // Search for complete FIX messages (terminated by "10=xxx\x01") in m_receiveBuffer
    // A complete FIX message ends with the CheckSum field: "10=NNN\x01"
    while ( true )
    {
        const char* bufPtr = static_cast< const char* >( m_receiveBuffer.GetConstBufferPtr() );
        CORE::UInt32 bufLen = m_receiveBuffer.GetDataSize();

        if ( bufLen == 0 )
            break;

        // Find "10=" followed by digits and SOH
        const char* msgEnd = GUCEF_NULL;
        for ( CORE::UInt32 i = 0; i + 5 < bufLen; ++i )
        {
            if ( bufPtr[i] == '1' && bufPtr[i+1] == '0' && bufPtr[i+2] == '=' )
            {
                // Found "10=", now find the terminating SOH
                CORE::UInt32 j = i + 3;
                while ( j < bufLen && bufPtr[j] != CFIXMessage::SOH )
                    ++j;
                if ( j < bufLen && bufPtr[j] == CFIXMessage::SOH )
                {
                    msgEnd = bufPtr + j + 1; // one past the terminating SOH
                    break;
                }
            }
        }

        if ( GUCEF_NULL == msgEnd )
            break; // No complete message yet

        // Extract the complete message
        CORE::UInt32 msgLen = (CORE::UInt32)( msgEnd - bufPtr );
        CORE::CString rawMsg( bufPtr, msgLen );

        // Advance the buffer past this message
        CORE::UInt32 remaining = bufLen - msgLen;
        if ( remaining > 0 )
        {
            ::memmove( m_receiveBuffer.GetBufferPtr(), bufPtr + msgLen, remaining );
        }
        m_receiveBuffer.SetDataSize( remaining );

        DispatchIncomingMessage( rawMsg );
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::DispatchIncomingMessage( const CORE::CString& rawMsg )
{GUCEF_TRACE;

    CFIXMessage fixMsg;
    if ( !CFIXMessage::Parse( rawMsg, fixMsg ) )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::DispatchIncomingMessage: Failed to parse FIX message" );
        return;
    }

    CORE::UInt64 incomingSeqNum = fixMsg.GetMsgSeqNumAsUInt64();
    CORE::CString msgType = fixMsg.GetMsgType();

    // Sequence number gap detection (skip for SequenceReset which is allowed to reset)
    if ( msgType != "4" && incomingSeqNum > 0 )
    {
        if ( incomingSeqNum > m_expectedIncomingSeqNum )
        {
            // Gap detected - request resend
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient: Sequence gap detected. Expected=" +
                CORE::ToString( m_expectedIncomingSeqNum ) + " Got=" + CORE::ToString( incomingSeqNum ) );

            if ( m_sessionState == STATE_ACTIVE )
            {
                CORE::CString resendReq = CFIXMessage::BuildResendRequest(
                    m_fixConfig.senderCompId, m_fixConfig.targetCompId,
                    m_fixConfig.fixVersion, m_outgoingSeqNum,
                    m_expectedIncomingSeqNum, incomingSeqNum - 1 );
                m_tcpSocket.Send( resendReq.C_String(), (CORE::UInt32) resendReq.Length() );
                ++m_outgoingSeqNum;
            }
            // Still process the current message if it's not a PossDupFlag scenario
            if ( fixMsg.HasField( CFIXMessage::TAG_POSS_DUP_FLAG ) &&
                 fixMsg.GetField( CFIXMessage::TAG_POSS_DUP_FLAG ) == "Y" &&
                 incomingSeqNum < m_expectedIncomingSeqNum )
            {
                return; // duplicate, skip
            }
        }
        else if ( incomingSeqNum < m_expectedIncomingSeqNum )
        {
            // PossDupFlag check
            if ( fixMsg.HasField( CFIXMessage::TAG_POSS_DUP_FLAG ) &&
                 fixMsg.GetField( CFIXMessage::TAG_POSS_DUP_FLAG ) == "Y" )
            {
                return; // already processed, skip
            }
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient: Unexpected lower seqnum. Expected=" +
                CORE::ToString( m_expectedIncomingSeqNum ) + " Got=" + CORE::ToString( incomingSeqNum ) );
        }
        else
        {
            ++m_expectedIncomingSeqNum;
        }
    }

    // Dispatch by MsgType
    if      ( msgType == "A" ) HandleLogon( fixMsg );
    else if ( msgType == "5" ) HandleLogout( fixMsg );
    else if ( msgType == "0" ) HandleHeartbeat( fixMsg );
    else if ( msgType == "1" ) HandleTestRequest( fixMsg );
    else if ( msgType == "2" ) HandleResendRequest( fixMsg );
    else if ( msgType == "4" ) HandleSequenceReset( fixMsg );
    else if ( msgType == "3" ) HandleReject( fixMsg );
    else
    {
        // Application message - route to topic
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            (*i).second->OnApplicationMessage( fixMsg );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleLogon( const CFIXMessage& msg )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::HandleLogon: Logon accepted by counterparty" );

    m_logonTimeoutTimer->SetEnabled( false );
    m_sessionState = STATE_ACTIVE;
    m_heartbeatTimer->SetEnabled( true );

    // If counterparty requests a seq reset
    if ( msg.HasField( CFIXMessage::TAG_RESET_SEQ_NUM ) &&
         msg.GetField( CFIXMessage::TAG_RESET_SEQ_NUM ) == "Y" )
    {
        m_expectedIncomingSeqNum = 1;
    }

    // Also pass Logon to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msg );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleLogout( const CFIXMessage& msg )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::HandleLogout: Received Logout from counterparty" );

    if ( m_sessionState == STATE_ACTIVE )
    {
        // Send Logout reply
        CORE::CString logoutReply = CFIXMessage::BuildLogout(
            m_fixConfig.senderCompId, m_fixConfig.targetCompId,
            m_fixConfig.fixVersion, m_outgoingSeqNum );
        m_tcpSocket.Send( logoutReply.C_String(), (CORE::UInt32) logoutReply.Length() );
        ++m_outgoingSeqNum;
    }

    m_heartbeatTimer->SetEnabled( false );
    m_sessionState = STATE_LOGGING_OUT;
    m_tcpSocket.Close();
    m_sessionState = STATE_DISCONNECTED;

    // Also pass Logout to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msg );
        ++i;
    }

    ScheduleReconnect();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleHeartbeat( const CFIXMessage& msg )
{GUCEF_TRACE;

    // If TestReqID is present, echo it back as a Heartbeat
    if ( msg.HasField( CFIXMessage::TAG_TEST_REQ_ID ) )
    {
        CORE::CString testReqId = msg.GetField( CFIXMessage::TAG_TEST_REQ_ID );
        CORE::CString heartbeat = CFIXMessage::BuildHeartbeat(
            m_fixConfig.senderCompId, m_fixConfig.targetCompId,
            m_fixConfig.fixVersion, m_outgoingSeqNum, testReqId );
        m_tcpSocket.Send( heartbeat.C_String(), (CORE::UInt32) heartbeat.Length() );
        ++m_outgoingSeqNum;
    }

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msg );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleTestRequest( const CFIXMessage& msg )
{GUCEF_TRACE;

    // Reply with Heartbeat including the TestReqID
    CORE::CString testReqId = msg.GetField( CFIXMessage::TAG_TEST_REQ_ID );
    CORE::CString heartbeat = CFIXMessage::BuildHeartbeat(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum, testReqId );
    m_tcpSocket.Send( heartbeat.C_String(), (CORE::UInt32) heartbeat.Length() );
    ++m_outgoingSeqNum;

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msg );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleResendRequest( const CFIXMessage& msg )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::HandleResendRequest: ResendRequest received - sending GapFill SequenceReset" );

    // For simplicity, respond with a SequenceReset-GapFill to catch up to current seqnum
    CORE::UInt64 beginSeqNo = CORE::StringToUInt64( msg.GetField( CFIXMessage::TAG_BEGIN_SEQ_NO ) );
    CORE::CString seqReset = CFIXMessage::BuildSequenceReset(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, beginSeqNo, m_outgoingSeqNum, true /* gapFill */ );
    m_tcpSocket.Send( seqReset.C_String(), (CORE::UInt32) seqReset.Length() );
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleSequenceReset( const CFIXMessage& msg )
{GUCEF_TRACE;

    CORE::UInt64 newSeqNo = CORE::StringToUInt64( msg.GetField( CFIXMessage::TAG_NEW_SEQ_NO ) );
    if ( newSeqNo > 0 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::HandleSequenceReset: Resetting expected incoming seq to " + CORE::ToString( newSeqNo ) );
        m_expectedIncomingSeqNum = newSeqNo;
    }

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msg );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::HandleReject( const CFIXMessage& msg )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::HandleReject: Reject received from counterparty" );

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msg );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnTcpConnected( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::OnTcpConnected: TCP connection established, sending Logon" );

    m_sessionState = STATE_LOGGING_IN;

    // Send Logon
    CORE::CString logonMsg = CFIXMessage::BuildLogon(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum,
        m_fixConfig.heartbeatIntervalSecs, m_fixConfig.resetSeqNumOnLogon );
    m_tcpSocket.Send( logonMsg.C_String(), (CORE::UInt32) logonMsg.Length() );
    ++m_outgoingSeqNum;

    // Start logon timeout
    m_logonTimeoutTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnTcpDisconnected( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::OnTcpDisconnected" );

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_sessionState = STATE_DISCONNECTED;
    m_receiveBuffer.Clear();

    // Notify topics of disconnection
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        // Topics notice disconnect via IsConnected() returning false
        ++i;
    }

    ScheduleReconnect();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnTcpDataReceived( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPConnection::TDataRecievedEventData* dataEvent =
        static_cast< COMCORE::CTCPConnection::TDataRecievedEventData* >( eventData );
    if ( GUCEF_NULL == dataEvent )
        return;

    const CORE::CDynamicBuffer& buffer = dataEvent->GetData();
    if ( buffer.GetDataSize() == 0 )
        return;

    // Append to receive buffer
    CORE::UInt32 existingSize = m_receiveBuffer.GetDataSize();
    m_receiveBuffer.SetDataSize( existingSize + buffer.GetDataSize() );
    ::memcpy( static_cast< char* >( m_receiveBuffer.GetBufferPtr() ) + existingSize,
              buffer.GetConstBufferPtr(),
              buffer.GetDataSize() );

    ProcessReceiveBuffer();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnTcpSocketError( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::OnTcpSocketError: TCP socket error" );

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_sessionState = STATE_DISCONNECTED;
    m_receiveBuffer.Clear();
    ScheduleReconnect();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnHeartbeatTimerCycle( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE )
        return;

    // Send TestRequest to verify counterparty is alive
    CORE::CString testReqId = "HEARTBEAT_" + CORE::ToString( m_outgoingSeqNum );
    CORE::CString testReq = CFIXMessage::BuildTestRequest(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum, testReqId );
    m_tcpSocket.Send( testReq.C_String(), (CORE::UInt32) testReq.Length() );
    ++m_outgoingSeqNum;
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnLogonTimeoutTimerCycle( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_sessionState == STATE_LOGGING_IN )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::OnLogonTimeoutTimerCycle: Logon timeout, disconnecting" );
        m_logonTimeoutTimer->SetEnabled( false );
        m_tcpSocket.Close();
        m_sessionState = STATE_DISCONNECTED;
        ScheduleReconnect();
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClient::OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_reconnectTimer->SetEnabled( false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClient::OnReconnectTimerCycle: Attempting reconnect" );
    Connect( false );
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
