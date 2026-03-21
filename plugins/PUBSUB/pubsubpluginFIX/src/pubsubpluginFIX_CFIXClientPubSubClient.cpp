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

#include "pubsubpluginFIX_CFIXClientPubSubClient.h"

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

const CORE::CString CFIXClientPubSubClient::TypeName = "FIXClient";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CFIXClientPubSubClient::CFIXClientPubSubClient( const PUBSUB::CPubSubClientConfig& config )
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
    , m_consecutiveChecksumFailures( 0 )
    , m_lock()
    , m_initialized( false )
{GUCEF_TRACE;

    if ( !LoadConfig( config ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXClientPubSubClient: Failed to load config at construction" );
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

CFIXClientPubSubClient::~CFIXClientPubSubClient()
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
CFIXClientPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator )
{GUCEF_TRACE;

    SetPulseGenerator( newPulseGenerator, true );
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::SetPulseGenerator( CORE::PulseGeneratorPtr newPulseGenerator,
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
CFIXClientPubSubClient::RegisterEventHandlers( void )
{GUCEF_TRACE;

    // TCP socket events
    TEventCallback cbConnected( this, &CFIXClientPubSubClient::OnTcpConnected );
    SubscribeTo( &m_tcpSocket                            ,
                 COMCORE::CTCPConnection::ConnectedEvent ,
                 cbConnected                            );

    TEventCallback cbDisconnected( this, &CFIXClientPubSubClient::OnTcpDisconnected );
    SubscribeTo( &m_tcpSocket                               ,
                 COMCORE::CTCPConnection::DisconnectedEvent ,
                 cbDisconnected                             );

    TEventCallback cbDataReceived( this, &CFIXClientPubSubClient::OnTcpDataReceived );
    SubscribeTo( &m_tcpSocket                               ,
                 COMCORE::CTCPConnection::DataRecievedEvent ,
                 cbDataReceived                             );

    TEventCallback cbSocketError( this, &CFIXClientPubSubClient::OnTcpSocketError );
    SubscribeTo( &m_tcpSocket                              ,
                 COMCORE::CTCPConnection::SocketErrorEvent ,
                 cbSocketError                             );

    // Heartbeat timer
    if ( GUCEF_NULL != m_heartbeatTimer )
    {
        TEventCallback cbHeartbeat( this, &CFIXClientPubSubClient::OnHeartbeatTimerCycle );
        SubscribeTo( m_heartbeatTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     cbHeartbeat                   );
    }

    // Logon timeout timer
    if ( GUCEF_NULL != m_logonTimeoutTimer )
    {
        TEventCallback cbLogonTimeout( this, &CFIXClientPubSubClient::OnLogonTimeoutTimerCycle );
        SubscribeTo( m_logonTimeoutTimer            ,
                     CORE::CTimer::TimerUpdateEvent ,
                     cbLogonTimeout                 );
    }

    // Reconnect timer
    if ( GUCEF_NULL != m_reconnectTimer )
    {
        TEventCallback cbReconnect( this, &CFIXClientPubSubClient::OnReconnectTimerCycle );
        SubscribeTo( m_reconnectTimer               ,
                     CORE::CTimer::TimerUpdateEvent ,
                     cbReconnect                    );
    }
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::GetSupportedFeatures( PUBSUB::CPubSubClientFeatures& features ) const
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
CFIXClientPubSubClient::CreateTopicAccess( PUBSUB::CPubSubClientTopicConfigPtr topicConfig ,
                                           CORE::PulseGeneratorPtr pulseGenerator          )
{GUCEF_TRACE;

    CFIXClientPubSubClientTopicPtr topicAccess;
    {
        MT::CObjectScopeLock lock( this );

        topicAccess = ( GUCEF_NEW CFIXClientPubSubClientTopic( this ) )->CreateSharedPtr();
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
CFIXClientPubSubClient::GetTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
        return (*i).second;
    return PUBSUB::CPubSubClientTopicPtr();
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::DestroyTopicAccess( const CORE::CString& topicName )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    TTopicMap::iterator i = m_topicMap.find( topicName );
    if ( i != m_topicMap.end() )
    {
        CFIXClientPubSubClientTopicPtr topicAccess = (*i).second;
        m_topicMap.erase( i );

        TopicAccessDestroyedEventData eData( topicName );
        NotifyObservers( TopicAccessDestroyedEvent, &eData );

        topicAccess->Shutdown();
        topicAccess.Unlink();
    }
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClientTopicConfigPtr
CFIXClientPubSubClient::GetTopicConfig( const CORE::CString& topicName )
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
CFIXClientPubSubClient::GetOrCreateTopicConfig( const CORE::CString& topicName )
{GUCEF_TRACE;

    PUBSUB::CPubSubClientTopicConfigPtr preExistingConfig = GetTopicConfig( topicName );
    if ( !preExistingConfig.IsNULL() )
        return preExistingConfig;

    CFIXClientPubSubClientTopicConfigPtr newTopicConfig = CFIXClientPubSubClientTopicConfig::CreateSharedObj();
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
CFIXClientPubSubClient::GetDefaultTopicConfig( void )
{GUCEF_TRACE;

    MT::CObjectScopeLock lock( this );
    return m_fixConfig.defaultTopicConfig;
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::GetConfiguredTopicNameList( CORE::CString::StringSet& topicNameList )
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
CFIXClientPubSubClient::GetCreatedTopicAccessNameList( CORE::CString::StringSet& topicNameList )
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
CFIXClientPubSubClient::GetAllCreatedTopicAccess( PubSubClientTopicSet& topicAccess )
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
CFIXClientPubSubClient::Connect( bool reset )
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
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXClientPubSubClient::Connect: No remote addresses configured" );
        return false;
    }

    const COMCORE::CHostAddress& remoteAddr = m_fixConfig.remoteAddresses.front();
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::Connect: Connecting to " + remoteAddr.GetFirstAddressAndPortAsString() );

    m_sessionState = STATE_CONNECTING;
    bool connectOk = m_tcpSocket.ConnectTo( remoteAddr, false /* non-blocking */ );
    if ( !connectOk )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "CFIXClientPubSubClient::Connect: ConnectTo returned false" );
        m_sessionState = STATE_DISCONNECTED;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::Disconnect( void )
{GUCEF_TRACE;

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_reconnectTimer->SetEnabled( false );

    if ( m_sessionState == STATE_ACTIVE )
    {
        // Send Logout
        CORE::CAsciiString logoutMsg = CFIXClientMessage::BuildLogout(
            m_fixConfig.senderCompId, m_fixConfig.targetCompId,
            m_fixConfig.fixVersion, m_outgoingSeqNum );
        m_tcpSocket.Send( logoutMsg.C_String(), (CORE::UInt32) logoutMsg.Length() );
        ++m_outgoingSeqNum;
        m_sessionState = STATE_LOGGING_OUT;
    }

    m_tcpSocket.Close();
    m_sessionState = STATE_DISCONNECTED;
    m_receiveBuffer.Clear();
    m_consecutiveChecksumFailures = 0;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::IsConnected( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::IsSessionActive( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::IsHealthy( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE ||
           m_sessionState == STATE_CONNECTING ||
           m_sessionState == STATE_LOGGING_IN;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::IsInitialized( void ) const
{GUCEF_TRACE;

    return m_initialized;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXClientPubSubClient::GetType( void ) const
{GUCEF_TRACE;

    return TypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXClientPubSubClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::FIX::CFIXClientPubSubClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    return m_fixConfig.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::SaveConfig( PUBSUB::CPubSubClientConfig& cfg ) const
{GUCEF_TRACE;

    return m_fixConfig.SaveConfig( cfg );
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    CFIXClientPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        m_fixConfig = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::LoadConfig( const PUBSUB::CPubSubClientConfig& cfg )
{GUCEF_TRACE;

    CFIXClientPubSubClientConfig parsedCfg;
    if ( parsedCfg.LoadConfig( cfg ) )
    {
        m_fixConfig = parsedCfg;
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

CFIXClientPubSubClientConfig&
CFIXClientPubSubClient::GetConfig( void )
{GUCEF_TRACE;

    return m_fixConfig;
}

/*-------------------------------------------------------------------------*/

bool
CFIXClientPubSubClient::SendRawFix( const CORE::CAsciiString& rawMsg )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::SendRawFix: Session not active" );
        return false;
    }
    return m_tcpSocket.Send( rawMsg.C_String(), (CORE::UInt32) rawMsg.Length() );
}

/*-------------------------------------------------------------------------*/

CORE::UInt64
CFIXClientPubSubClient::GetAndIncrementOutgoingSeqNum( void )
{GUCEF_TRACE;

    return m_outgoingSeqNum++;
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::SaveSequenceBookmark( void )
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
        CFIXClientPubSubClientTopicPtr topic = m_topicMap.begin()->second;
        if ( !topic.IsNULL() )
        {
            m_fixConfig.bookmarkPersistence->StoreBookmark( m_fixConfig.pubsubIdPrefix, *this, *topic, bookmark );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::LoadSequenceBookmark( void )
{GUCEF_TRACE;

    if ( m_fixConfig.bookmarkPersistence.IsNULL() || m_topicMap.empty() )
        return;

    CFIXClientPubSubClientTopicPtr topic = m_topicMap.begin()->second;
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
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::LoadSequenceBookmark: Restored outgoing=" +
                    CORE::ToString( m_outgoingSeqNum ) + " incoming=" + CORE::ToString( m_expectedIncomingSeqNum ) );
            }
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::RequestReplayFrom( CORE::UInt64 fromSeqNum )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE )
        return;

    CORE::CAsciiString resendReq = CFIXClientMessage::BuildResendRequest(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum, fromSeqNum, 0 );
    ++m_outgoingSeqNum;
    m_tcpSocket.Send( resendReq.C_String(), (CORE::UInt32) resendReq.Length() );
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::ScheduleReconnect( void )
{GUCEF_TRACE;

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_reconnectTimer->SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::ProcessReceiveBuffer( void )
{GUCEF_TRACE;

    const char* bufPtr = static_cast< const char* >( m_receiveBuffer.GetConstBufferPtr() );
    CORE::UInt32 bufLen = m_receiveBuffer.GetDataSize();
    CORE::UInt32 processedOffset = 0;

    while ( processedOffset < bufLen )
    {
        const char* msgStart = bufPtr + processedOffset;
        CORE::UInt32 remaining = bufLen - processedOffset;

        // Need minimum bytes for a meaningful FIX header
        if ( remaining < 15 )
            break;

        // Framing sync: message must start with "8="
        if ( msgStart[ 0 ] != '8' || msgStart[ 1 ] != '=' )
        {
            // Scan forward for a field-aligned "8=" to resync framing
            CORE::UInt32 i = 1;
            bool found = false;
            while ( i + 1 < remaining )
            {
                if ( bufPtr[ processedOffset + i ]     == '8' &&
                     bufPtr[ processedOffset + i + 1 ] == '=' )
                {
                    // Must be at buffer start or preceded by SOH
                    if ( processedOffset + i == 0 ||
                         bufPtr[ processedOffset + i - 1 ] == CFIXClientMessage::SOH )
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                            "CFIXClientPubSubClient::ProcessReceiveBuffer: Framing resync - skipping " +
                            CORE::ToString( i ) + " bytes to next 8=" );
                        processedOffset += i;
                        found = true;
                        break;
                    }
                }
                ++i;
            }
            if ( !found )
            {
                processedOffset = bufLen;  // nothing recognisable left
                break;
            }
            continue;  // restart from new position
        }

        // Find end of tag 8 value (BeginString)
        const char* tag8ValStart = msgStart + 2;  // after "8="
        const char* tag8ValEnd   = tag8ValStart;
        const char* bufEnd       = bufPtr + bufLen;
        while ( tag8ValEnd < bufEnd && *tag8ValEnd != CFIXClientMessage::SOH )
            ++tag8ValEnd;
        if ( tag8ValEnd >= bufEnd )
            break;  // incomplete — wait for more data

        // [S5] Validate BeginString starts with "FIX." or "FIXT."
        CORE::UInt32 beginStringLen = (CORE::UInt32)( tag8ValEnd - tag8ValStart );
        bool validBeginString =
            ( beginStringLen >= 4 &&
              tag8ValStart[ 0 ] == 'F' && tag8ValStart[ 1 ] == 'I' &&
              tag8ValStart[ 2 ] == 'X' && tag8ValStart[ 3 ] == '.' ) ||
            ( beginStringLen >= 5 &&
              tag8ValStart[ 0 ] == 'F' && tag8ValStart[ 1 ] == 'I' &&
              tag8ValStart[ 2 ] == 'X' && tag8ValStart[ 3 ] == 'T' &&
              tag8ValStart[ 4 ] == '.' );
        if ( !validBeginString )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXClientPubSubClient::ProcessReceiveBuffer: [S5] BeginString is not FIX/FIXT - discarding framing junk" );
            // Advance past this "8=" occurrence and continue resyncing
            processedOffset = (CORE::UInt32)( tag8ValEnd - bufPtr ) + 1;
            continue;
        }

        // Locate "9=" (BodyLength) — must immediately follow tag 8's SOH in standard FIX
        const char* afterTag8 = tag8ValEnd + 1;
        const char* tag9Pos   = GUCEF_NULL;
        {
            const char* scanPos = afterTag8;
            const char* scanEnd = afterTag8 + 20;  // tag 9 must be within 20 bytes of tag 8 SOH
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
        {
            if ( afterTag8 + 20 > bufEnd )
                break;  // incomplete — might still come
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXClientPubSubClient::ProcessReceiveBuffer: Cannot locate tag 9 (BodyLength), skipping" );
            processedOffset = (CORE::UInt32)( tag8ValEnd - bufPtr ) + 1;
            continue;
        }

        // Parse BodyLength value inline
        const char* bodyLenStart = tag9Pos + 2;  // after "9="
        const char* bodyLenEnd   = bodyLenStart;
        while ( bodyLenEnd < bufEnd && *bodyLenEnd != CFIXClientMessage::SOH )
            ++bodyLenEnd;
        if ( bodyLenEnd >= bufEnd )
            break;  // incomplete

        CORE::UInt32 bodyLen = (CORE::UInt32) CFIXClientMessage::ParseUInt64Inline( bodyLenStart,
                                                                  (CORE::UInt32)( bodyLenEnd - bodyLenStart ) );

        // [S1] Max message size cap — reject giant BodyLength before jumping
        if ( bodyLen > m_fixConfig.maxMsgSizeBytes )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT,
                "CFIXClientPubSubClient::ProcessReceiveBuffer: [S1] BodyLength " +
                CORE::ToString( bodyLen ) + " exceeds maxMsgSizeBytes " +
                CORE::ToString( m_fixConfig.maxMsgSizeBytes ) + " - disconnecting" );
            Disconnect();
            return;
        }

        // bodyStart = first byte after "9=xxx\x01"
        const char* bodyStart = bodyLenEnd + 1;

        // [S2] 64-bit overflow-safe bounds check: bodyStart + bodyLen + 7 ("10=XXX\x01") <= bufLen
        CORE::UInt64 safeEnd = (CORE::UInt64)( bodyStart - bufPtr ) +
                               (CORE::UInt64)bodyLen +
                               (CORE::UInt64)7;  // "10=XXX\x01" minimum 7 bytes
        if ( safeEnd > (CORE::UInt64)bufLen )
            break;  // incomplete — wait for more data

        // O(1) jump to where "10=" should be
        const char* tag10Pos = bodyStart + bodyLen;

        // [S9] Verify the jump landed on "10=" (bounded fallback scan if not)
        bool tag10Valid = ( (CORE::UInt32)( bufEnd - tag10Pos ) >= 3 &&
                            tag10Pos[ 0 ] == '1' && tag10Pos[ 1 ] == '0' && tag10Pos[ 2 ] == '=' &&
                            ( tag10Pos == bufPtr || *( tag10Pos - 1 ) == CFIXClientMessage::SOH ) );
        if ( !tag10Valid )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXClientPubSubClient::ProcessReceiveBuffer: BodyLength jump missed tag 10 - fallback linear scan" );
            tag10Pos = GUCEF_NULL;
            // [S9] Bounded fallback scan: do not scan past bufLen-6
            const char* scanPtr = bodyStart;
            const char* scanEnd = bufEnd;
            if ( scanEnd > bufEnd - 6 )
                scanEnd = bufEnd - 6;
            while ( scanPtr < scanEnd )
            {
                if ( scanPtr[ 0 ] == '1' && scanPtr[ 1 ] == '0' && scanPtr[ 2 ] == '=' &&
                     ( scanPtr == bufPtr || *( scanPtr - 1 ) == CFIXClientMessage::SOH ) )
                {
                    tag10Pos = scanPtr;
                    break;
                }
                ++scanPtr;
            }
            if ( GUCEF_NULL == tag10Pos )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL,
                    "CFIXClientPubSubClient::ProcessReceiveBuffer: Cannot locate tag 10 - malformed message, skipping" );
                processedOffset = (CORE::UInt32)( bodyStart - bufPtr );
                continue;
            }
        }

        // Find SOH after checksum digits — determines actual msgEnd
        const char* checksumValStart = tag10Pos + 3;  // after "10="
        const char* checksumValEnd   = checksumValStart;
        while ( checksumValEnd < bufEnd && *checksumValEnd != CFIXClientMessage::SOH )
            ++checksumValEnd;
        if ( checksumValEnd >= bufEnd )
            break;  // incomplete

        const char* msgEnd = checksumValEnd + 1;  // one past final SOH
        CORE::UInt32 msgLen = (CORE::UInt32)( msgEnd - msgStart );

        // [S7] Checksum validation (enabled by default)
        if ( !m_fixConfig.disableChecksumValidation )
        {
            CORE::UInt32 byteSum = 0;
            for ( const char* p = msgStart; p < tag10Pos; ++p )
                byteSum += (CORE::UInt8)*p;
            CORE::UInt32 calcChecksum = byteSum % 256;

            CORE::UInt32 claimedChecksum = (CORE::UInt32) CFIXClientMessage::ParseUInt64Inline(
                checksumValStart, (CORE::UInt32)( checksumValEnd - checksumValStart ) );

            if ( calcChecksum != claimedChecksum )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                    "CFIXClientPubSubClient::ProcessReceiveBuffer: [S7] Checksum mismatch claimed=" +
                    CORE::ToString( claimedChecksum ) + " calc=" + CORE::ToString( calcChecksum ) +
                    " - dropping message" );
                ++m_consecutiveChecksumFailures;
                if ( m_consecutiveChecksumFailures >= m_fixConfig.maxConsecutiveChecksumFailures )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT,
                        "CFIXClientPubSubClient::ProcessReceiveBuffer: " +
                        CORE::ToString( m_consecutiveChecksumFailures ) +
                        " consecutive checksum failures - disconnecting" );
                    Disconnect();
                    return;
                }
                processedOffset += msgLen;
                continue;
            }
            m_consecutiveChecksumFailures = 0;
        }

        // Scan session fields — zero-allocation, single forward pass
        CFIXClientSessionFields fields;
        if ( !CFIXClientMessage::ScanSessionFields( msgStart, msgLen, fields ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXClientPubSubClient::ProcessReceiveBuffer: ScanSessionFields failed - dropping message" );
            processedOffset += msgLen;
            continue;
        }

        // CRITICAL: Dispatch BEFORE buffer compaction.
        // NotifyObservers is synchronous — all observers complete before we return here.
        // Linked views into msgStart remain valid for the entire notification chain.
        DispatchIncomingMessage( msgStart, msgLen, fields );

        // Check if dispatch triggered a disconnect (e.g., Logout handler closed connection)
        if ( m_sessionState == STATE_DISCONNECTED )
            return;  // buffer has been cleared, do not continue processing

        processedOffset += msgLen;
    }

    // [S8] Safety clamp: processedOffset cannot exceed bufLen
    if ( processedOffset > bufLen )
        processedOffset = bufLen;

    // Single memmove at the end — not per-message
    if ( processedOffset > 0 )
    {
        CORE::UInt32 remainingBytes = bufLen - processedOffset;
        if ( remainingBytes > 0 )
        {
            ::memmove( m_receiveBuffer.GetBufferPtr(),
                       bufPtr + processedOffset,
                       remainingBytes );
        }
        m_receiveBuffer.SetDataSize( remainingBytes );
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::DispatchIncomingMessage( const char* msgStart                  ,
                                                 CORE::UInt32 msgLen                   ,
                                                 const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    // Require at minimum a MsgType field
    if ( GUCEF_NULL == fields.msgTypeStart )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXClientPubSubClient::DispatchIncomingMessage: Missing MsgType (tag 35) - dropping" );
        return;
    }

    CORE::UInt64 incomingSeqNum = fields.seqNumVal;

    // Sequence number gap detection (skip for SequenceReset which resets seqnum)
    if ( !CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "4" ) && incomingSeqNum > 0 )
    {
        if ( incomingSeqNum > m_expectedIncomingSeqNum )
        {
            // Gap detected — request resend
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXClientPubSubClient: Sequence gap detected. Expected=" +
                CORE::ToString( m_expectedIncomingSeqNum ) +
                " Got=" + CORE::ToString( incomingSeqNum ) );

            if ( m_sessionState == STATE_ACTIVE )
            {
                CORE::CAsciiString resendReq = CFIXClientMessage::BuildResendRequest(
                    m_fixConfig.senderCompId, m_fixConfig.targetCompId,
                    m_fixConfig.fixVersion, m_outgoingSeqNum,
                    m_expectedIncomingSeqNum, incomingSeqNum - 1 );
                m_tcpSocket.Send( resendReq.C_String(), (CORE::UInt32) resendReq.Length() );
                ++m_outgoingSeqNum;
            }
            // Still process the current message unless it is a PossDup duplicate
            if ( GUCEF_NULL != fields.possDupFlagStart &&
                 CFIXClientMessage::FieldMatchesValue( fields.possDupFlagStart, fields.possDupFlagLen, "Y" ) &&
                 incomingSeqNum < m_expectedIncomingSeqNum )
            {
                return;  // duplicate — skip
            }
        }
        else if ( incomingSeqNum < m_expectedIncomingSeqNum )
        {
            // PossDupFlag check for already-processed messages
            if ( GUCEF_NULL != fields.possDupFlagStart &&
                 CFIXClientMessage::FieldMatchesValue( fields.possDupFlagStart, fields.possDupFlagLen, "Y" ) )
            {
                return;  // already processed — skip
            }
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXClientPubSubClient: Unexpected lower seqnum. Expected=" +
                CORE::ToString( m_expectedIncomingSeqNum ) +
                " Got=" + CORE::ToString( incomingSeqNum ) );
        }
        else
        {
            ++m_expectedIncomingSeqNum;
        }
    }

    // Dispatch by MsgType via direct char comparison — no CString allocation
    if      ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "A" ) ) HandleLogon( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "5" ) ) HandleLogout( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "0" ) ) HandleHeartbeat( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "1" ) ) HandleTestRequest( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "2" ) ) HandleResendRequest( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "4" ) ) HandleSequenceReset( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "3" ) ) HandleReject( msgStart, msgLen, fields );
    else
    {
        // Application message — route to all topics
        TTopicMap::iterator i = m_topicMap.begin();
        while ( i != m_topicMap.end() )
        {
            (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
            ++i;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleLogon( const char* msgStart                  ,
                                     CORE::UInt32 msgLen                   ,
                                     const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::HandleLogon: Logon accepted by counterparty" );

    m_logonTimeoutTimer->SetEnabled( false );
    m_sessionState = STATE_ACTIVE;
    m_heartbeatTimer->SetEnabled( true );

    // If counterparty requests a sequence reset (tag 141=Y)
    if ( GUCEF_NULL != fields.resetFlagStart &&
         CFIXClientMessage::FieldMatchesValue( fields.resetFlagStart, fields.resetFlagLen, "Y" ) )
    {
        m_expectedIncomingSeqNum = 1;
    }

    // Also pass Logon to topics if configured (includeSessionLevelMsgs)
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleLogout( const char* msgStart                  ,
                                      CORE::UInt32 msgLen                   ,
                                      const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::HandleLogout: Received Logout from counterparty" );

    if ( m_sessionState == STATE_ACTIVE )
    {
        // Send Logout reply
        CORE::CAsciiString logoutReply = CFIXClientMessage::BuildLogout(
            m_fixConfig.senderCompId, m_fixConfig.targetCompId,
            m_fixConfig.fixVersion, m_outgoingSeqNum );
        m_tcpSocket.Send( logoutReply.C_String(), (CORE::UInt32) logoutReply.Length() );
        ++m_outgoingSeqNum;
    }

    m_heartbeatTimer->SetEnabled( false );
    m_sessionState = STATE_LOGGING_OUT;

    // Also pass Logout to topics if configured (before closing connection)
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
        ++i;
    }

    m_tcpSocket.Close();
    m_sessionState = STATE_DISCONNECTED;

    ScheduleReconnect();
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleHeartbeat( const char* msgStart                  ,
                                         CORE::UInt32 msgLen                   ,
                                         const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    // If TestReqID is present, echo it back as a Heartbeat
    if ( GUCEF_NULL != fields.testReqIdStart && fields.testReqIdLen > 0 )
    {
        // Outgoing path: allocation acceptable (infrequent)
        CORE::CAsciiString testReqId( fields.testReqIdStart, fields.testReqIdLen );
        CORE::CAsciiString heartbeat = CFIXClientMessage::BuildHeartbeat(
            m_fixConfig.senderCompId, m_fixConfig.targetCompId,
            m_fixConfig.fixVersion, m_outgoingSeqNum, testReqId );
        m_tcpSocket.Send( heartbeat.C_String(), (CORE::UInt32) heartbeat.Length() );
        ++m_outgoingSeqNum;
    }

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleTestRequest( const char* msgStart                  ,
                                           CORE::UInt32 msgLen                   ,
                                           const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    // Reply with Heartbeat including the TestReqID
    // Outgoing path: allocation acceptable (infrequent)
    CORE::CAsciiString testReqId;
    if ( GUCEF_NULL != fields.testReqIdStart && fields.testReqIdLen > 0 )
        testReqId = CORE::CAsciiString( fields.testReqIdStart, fields.testReqIdLen );

    CORE::CAsciiString heartbeat = CFIXClientMessage::BuildHeartbeat(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum, testReqId );
    m_tcpSocket.Send( heartbeat.C_String(), (CORE::UInt32) heartbeat.Length() );
    ++m_outgoingSeqNum;

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleResendRequest( const char* msgStart                  ,
                                             CORE::UInt32 msgLen                   ,
                                             const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXClientPubSubClient::HandleResendRequest: ResendRequest received - sending GapFill SequenceReset" );

    // Parse BeginSeqNo from fields — outgoing path, allocation acceptable
    CORE::UInt64 beginSeqNo = 0;
    if ( GUCEF_NULL != fields.beginSeqNoStart && fields.beginSeqNoLen > 0 )
        beginSeqNo = CFIXClientMessage::ParseUInt64Inline( fields.beginSeqNoStart, fields.beginSeqNoLen );

    // For simplicity, respond with a SequenceReset-GapFill to catch up to current seqnum
    CORE::CAsciiString seqReset = CFIXClientMessage::BuildSequenceReset(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, beginSeqNo, m_outgoingSeqNum, true /* gapFill */ );
    m_tcpSocket.Send( seqReset.C_String(), (CORE::UInt32) seqReset.Length() );
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleSequenceReset( const char* msgStart                  ,
                                             CORE::UInt32 msgLen                   ,
                                             const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    CORE::UInt64 newSeqNo = 0;
    if ( GUCEF_NULL != fields.newSeqNoStart && fields.newSeqNoLen > 0 )
        newSeqNo = CFIXClientMessage::ParseUInt64Inline( fields.newSeqNoStart, fields.newSeqNoLen );

    if ( newSeqNo > 0 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXClientPubSubClient::HandleSequenceReset: Resetting expected incoming seq to " +
            CORE::ToString( newSeqNo ) );
        m_expectedIncomingSeqNum = newSeqNo;
    }

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::HandleReject( const char* msgStart                  ,
                                      CORE::UInt32 msgLen                   ,
                                      const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXClientPubSubClient::HandleReject: Reject received from counterparty" );

    // Also pass to topics if configured
    TTopicMap::iterator i = m_topicMap.begin();
    while ( i != m_topicMap.end() )
    {
        (*i).second->OnApplicationMessage( msgStart, msgLen, fields );
        ++i;
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::OnTcpConnected( CORE::CNotifier* notifier    ,
                                   const CORE::CEvent& eventId  ,
                                   CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::OnTcpConnected: TCP connection established, sending Logon" );

    m_sessionState = STATE_LOGGING_IN;

    // Send Logon
    CORE::CAsciiString logonMsg = CFIXClientMessage::BuildLogon(
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
CFIXClientPubSubClient::OnTcpDisconnected( CORE::CNotifier* notifier    ,
                                      const CORE::CEvent& eventId  ,
                                      CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::OnTcpDisconnected" );

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_sessionState = STATE_DISCONNECTED;
    m_receiveBuffer.Clear();
    m_consecutiveChecksumFailures = 0;

    ScheduleReconnect();
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::OnTcpDataReceived( CORE::CNotifier* notifier    ,
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
CFIXClientPubSubClient::OnTcpSocketError( CORE::CNotifier* notifier    ,
                                     const CORE::CEvent& eventId  ,
                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::OnTcpSocketError: TCP socket error" );

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );
    m_sessionState = STATE_DISCONNECTED;
    m_receiveBuffer.Clear();
    m_consecutiveChecksumFailures = 0;
    ScheduleReconnect();
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::OnHeartbeatTimerCycle( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE )
        return;

    // Send TestRequest to verify counterparty is alive
    CORE::CAsciiString testReqId = CORE::CAsciiString( "HEARTBEAT_" ) + CORE::CAsciiString( CORE::ToString( m_outgoingSeqNum ) );
    CORE::CAsciiString testReq = CFIXClientMessage::BuildTestRequest(
        m_fixConfig.senderCompId, m_fixConfig.targetCompId,
        m_fixConfig.fixVersion, m_outgoingSeqNum, testReqId );
    m_tcpSocket.Send( testReq.C_String(), (CORE::UInt32) testReq.Length() );
    ++m_outgoingSeqNum;
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::OnLogonTimeoutTimerCycle( CORE::CNotifier* notifier    ,
                                             const CORE::CEvent& eventId  ,
                                             CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_sessionState == STATE_LOGGING_IN )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXClientPubSubClient::OnLogonTimeoutTimerCycle: Logon timeout, disconnecting" );
        m_logonTimeoutTimer->SetEnabled( false );
        m_tcpSocket.Close();
        m_sessionState = STATE_DISCONNECTED;
        ScheduleReconnect();
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXClientPubSubClient::OnReconnectTimerCycle( CORE::CNotifier* notifier    ,
                                          const CORE::CEvent& eventId  ,
                                          CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_reconnectTimer->SetEnabled( false );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXClientPubSubClient::OnReconnectTimerCycle: Attempting reconnect" );
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
