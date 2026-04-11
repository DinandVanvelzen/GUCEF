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

#ifndef GUCEF_CORE_CDATETIME_H
#include "gucefCORE_CDateTime.h"
#define GUCEF_CORE_CDATETIME_H
#endif /* GUCEF_CORE_CDATETIME_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef GUCEF_COMCORE_CTCPCONNECTION_H
#include "CTCPConnection.h"
#define GUCEF_COMCORE_CTCPCONNECTION_H
#endif /* GUCEF_COMCORE_CTCPCONNECTION_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#include "gucefPUBSUB_CPubSubClientSide.h"
#define GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H
#endif /* GUCEF_PUBSUB_CPUBSUBCLIENTSIDE_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H
#include "pubsubpluginFIX_CFIXServerPubSubClient.h"
#define PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_FIX_CFIXSERVERPUBSUBCLIENT_H ? */

#include "pubsubpluginFIX_CFIXServerPubSubClientTopic.h"

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
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

// FIX session-level MsgTypes that are normally not forwarded to subscribers.
// Checked inline via single-char comparison for hot-path efficiency.
static inline bool IsSessionLevelMsgType( const char* msgType, CORE::UInt32 msgTypeLen )
{
    if ( GUCEF_NULL == msgType || msgTypeLen != 1 )
        return false;
    char c = msgType[ 0 ];
    return c == '0' ||  // Heartbeat
           c == '1' ||  // TestRequest
           c == '2' ||  // ResendRequest
           c == '3' ||  // Reject
           c == '4' ||  // SequenceReset
           c == '5' ||  // Logout
           c == 'A';    // Logon
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopic::CFIXServerPubSubClientTopic( CFIXServerPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CFIXServerPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_config()
    , m_activeConnection( GUCEF_NULL )
    , m_receiveBuffer()
    , m_heartbeatTimer( GUCEF_NULL )
    , m_logonTimeoutTimer( GUCEF_NULL )
    , m_outgoingSeqNum( 1 )
    , m_expectedIncomingSeqNum( 1 )
    , m_resendRequestSentForExpectedSeq( 0 )
    , m_consecutiveChecksumFailures( 0 )
    , m_sessionState( STATE_NO_CONNECTION )
    , m_isSubscribed( false )
    , m_currentReceiveActionId( 1 )
    , m_currentPublishActionId( 1 )
    , m_lock()
    , m_pubsubMsgs()
    , m_pubsubMsgsRefs()
    , m_publishSuccessActionIds()
    , m_publishSuccessActionEventData()
    , m_publishFailureActionIds()
    , m_publishFailureActionEventData()
{GUCEF_TRACE;

    m_publishSuccessActionEventData.LinkTo( &m_publishSuccessActionIds );
    m_publishFailureActionEventData.LinkTo( &m_publishFailureActionIds );

    const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();

    m_heartbeatTimer    = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), cfg.heartbeatIntervalSecs * 1000 );
    m_logonTimeoutTimer = GUCEF_NEW CORE::CTimer( client->GetPulseGenerator(), cfg.logonTimeoutInMs );

    m_heartbeatTimer->SetEnabled( false );
    m_logonTimeoutTimer->SetEnabled( false );

    TEventCallback cbHb( this, &CFIXServerPubSubClientTopic::OnHeartbeatTimerCycle );
    SubscribeTo( m_heartbeatTimer               ,
                 CORE::CTimer::TimerUpdateEvent ,
                 cbHb                          );

    TEventCallback cbLogonTimeout( this, &CFIXServerPubSubClientTopic::OnLogonTimeoutTimerCycle );
    SubscribeTo( m_logonTimeoutTimer            ,
                 CORE::CTimer::TimerUpdateEvent ,
                 cbLogonTimeout                 );
}

/*-------------------------------------------------------------------------*/

CFIXServerPubSubClientTopic::~CFIXServerPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_heartbeatTimer )
    {
        m_heartbeatTimer->SetEnabled( false );
        GUCEF_DELETE m_heartbeatTimer;
        m_heartbeatTimer = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_logonTimeoutTimer )
    {
        m_logonTimeoutTimer->SetEnabled( false );
        GUCEF_DELETE m_logonTimeoutTimer;
        m_logonTimeoutTimer = GUCEF_NULL;
    }

    if ( GUCEF_NULL != m_activeConnection )
    {
        UnregisterConnectionEventHandlers( m_activeConnection );
        m_activeConnection = GUCEF_NULL;
    }

    MT::CScopeMutex lock( m_lock );
    m_client = GUCEF_NULL;
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CFIXServerPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    // Connectivity is managed at the client (TCP server socket) level
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    m_isSubscribed = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    return m_isSubscribed;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    return m_sessionState == STATE_ACTIVE ||
           m_sessionState == STATE_WAITING_LOGON;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        return m_client->GetConfig().allowPublishing;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark
CFIXServerPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    PUBSUB::CPubSubBookmark bookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
    bookmark.SetBookmarkData( CORE::CVariant( m_expectedIncomingSeqNum > 0 ? m_expectedIncomingSeqNum - 1 : 0 ) );
    return bookmark;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXServerPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::IsMsgTypePassedByFilter( const char* msgType ,
                                                       CORE::UInt32 msgTypeLen ) const
{GUCEF_TRACE;

    if ( m_config.msgTypeFilter.IsNULLOrEmpty() )
        return true;  // empty filter = accept all

    const char* filterPtr = m_config.msgTypeFilter.C_String();
    CORE::UInt32 filterLen = (CORE::UInt32) m_config.msgTypeFilter.Length();
    CORE::UInt32 start = 0;
    while ( start < filterLen )
    {
        CORE::UInt32 end = start;
        while ( end < filterLen && filterPtr[ end ] != ',' )
            ++end;

        CORE::UInt32 tokenStart = start;
        while ( tokenStart < end && filterPtr[ tokenStart ] == ' ' )
            ++tokenStart;
        CORE::UInt32 tokenEnd = end;
        while ( tokenEnd > tokenStart && filterPtr[ tokenEnd - 1 ] == ' ' )
            --tokenEnd;

        CORE::UInt32 tokenLen = tokenEnd - tokenStart;
        if ( tokenLen == msgTypeLen &&
             ::memcmp( filterPtr + tokenStart, msgType, msgTypeLen ) == 0 )
            return true;

        start = end + 1;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::DeliverToSubscribers( const char* msgStart                  ,
                                                    CORE::UInt32 msgLen                   ,
                                                    const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    if ( !m_isSubscribed )
        return;

    if ( IsSessionLevelMsgType( fields.msgTypeStart, fields.msgTypeLen ) &&
         !m_config.includeSessionLevelMsgs )
        return;

    if ( GUCEF_NULL != fields.msgTypeStart &&
         !IsMsgTypePassedByFilter( fields.msgTypeStart, fields.msgTypeLen ) )
        return;

    MT::CScopeMutex lock( m_lock );

    m_pubsubMsgs.resize( 1 );
    PUBSUB::CBasicPubSubMsg& msg = m_pubsubMsgs[ 0 ];
    msg.Clear();

    if ( GUCEF_NULL != fields.seqNumStart )
        msg.GetMsgId().LinkTo( fields.seqNumStart, fields.seqNumLen, GUCEF_DATATYPE_ASCII_STRING );

    msg.GetMsgIndex() = CORE::CVariant( fields.seqNumVal );
    msg.GetMsgDateTime() = CORE::CDateTime::NowUTCDateTime();
    msg.GetPrimaryPayload().LinkTo( msgStart, msgLen, GUCEF_DATATYPE_ASCII_STRING );

    if ( GUCEF_NULL != fields.msgTypeStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.msgTypeStart, fields.msgTypeLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXClientMessage::TAG_MSG_TYPE ), valueVar );
    }
    if ( GUCEF_NULL != fields.beginStringStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.beginStringStart, fields.beginStringLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXClientMessage::TAG_BEGIN_STRING ), valueVar );
    }
    if ( GUCEF_NULL != fields.senderStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.senderStart, fields.senderLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXClientMessage::TAG_SENDER_COMP_ID ), valueVar );
    }
    if ( GUCEF_NULL != fields.targetStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.targetStart, fields.targetLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXClientMessage::TAG_TARGET_COMP_ID ), valueVar );
    }
    if ( GUCEF_NULL != fields.seqNumStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.seqNumStart, fields.seqNumLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXClientMessage::TAG_MSG_SEQ_NUM ), valueVar );
    }

    msg.SetReceiveActionId( m_currentReceiveActionId );
    ++m_currentReceiveActionId;

    m_pubsubMsgsRefs.clear();
    m_pubsubMsgsRefs.push_back( &m_pubsubMsgs[ 0 ] );
    NotifyObservers( MsgsRecievedEvent, &m_pubsubMsgsRefs );
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::SendRawFix( const CORE::CAsciiString& rawMsg )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_activeConnection || m_sessionState != STATE_ACTIVE )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClientTopic::SendRawFix: No active connection or session not active for topic " + m_config.topicName );
        return false;
    }
    return m_activeConnection->Send( rawMsg.C_String(), (CORE::UInt32) rawMsg.Length() );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::RegisterConnectionEventHandlers( COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    if ( GUCEF_NULL == connection )
        return;

    TEventCallback cbData( this, &CFIXServerPubSubClientTopic::OnConnectionDataReceived );
    SubscribeTo( connection                                ,
                 COMCORE::CTCPConnection::DataRecievedEvent ,
                 cbData                                    );

    TEventCallback cbDisc( this, &CFIXServerPubSubClientTopic::OnConnectionDisconnected );
    SubscribeTo( connection                                   ,
                 COMCORE::CTCPConnection::DisconnectedEvent   ,
                 cbDisc                                       );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::UnregisterConnectionEventHandlers( COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    if ( GUCEF_NULL == connection )
        return;

    UnsubscribeFrom( connection );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::AttachConnection( COMCORE::CTCPServerConnection* connection  ,
                                                const void* logonData                      ,
                                                CORE::UInt32 logonLen                      ,
                                                const CFIXClientSessionFields& logonFields )
{GUCEF_TRACE;

    if ( GUCEF_NULL == connection )
        return;

    // Session bump: close old connection if one exists
    if ( GUCEF_NULL != m_activeConnection )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClientTopic::AttachConnection: Session bump for topic " +
            m_config.topicName + " - replacing existing connection" );

        UnregisterConnectionEventHandlers( m_activeConnection );
        m_activeConnection->Close();
        m_activeConnection = GUCEF_NULL;
    }

    // Stop any running timers
    if ( GUCEF_NULL != m_heartbeatTimer )
        m_heartbeatTimer->SetEnabled( false );
    if ( GUCEF_NULL != m_logonTimeoutTimer )
        m_logonTimeoutTimer->SetEnabled( false );

    // Reset per-connection state
    m_receiveBuffer.Clear();
    m_consecutiveChecksumFailures = 0;
    m_resendRequestSentForExpectedSeq = 0;
    m_sessionState = STATE_WAITING_LOGON;

    // Attach to new connection
    m_activeConnection = connection;

    // Pre-load and process the Logon bytes that were already read during the pending phase.
    // This must happen BEFORE RegisterConnectionEventHandlers so that if the GUCEF notification
    // system re-delivers the current DataRecievedEvent to our newly-subscribed OnConnectionDataReceived
    // handler (which happens when subscribing mid-dispatch), the HandleLogon guard below catches
    // the duplicate and the buffer is already cleared from the first pass.
    if ( GUCEF_NULL != logonData && logonLen > 0 )
    {
        m_receiveBuffer.SetDataSize( logonLen );
        ::memcpy( m_receiveBuffer.GetBufferPtr(), logonData, logonLen );
        ProcessReceiveBuffer();
    }

    // Subscribe to future events on this connection (after initial Logon already processed above)
    RegisterConnectionEventHandlers( m_activeConnection );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::ProcessReceiveBuffer( void )
{GUCEF_TRACE;

    const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();

    const char* bufPtr = static_cast< const char* >( m_receiveBuffer.GetConstBufferPtr() );
    CORE::UInt32 bufLen = m_receiveBuffer.GetDataSize();
    CORE::UInt32 processedOffset = 0;

    while ( processedOffset < bufLen )
    {
        const char* msgStart = bufPtr + processedOffset;
        CORE::UInt32 remaining = bufLen - processedOffset;

        if ( remaining < 15 )
            break;

        // Framing sync: message must start with "8="
        if ( msgStart[ 0 ] != '8' || msgStart[ 1 ] != '=' )
        {
            CORE::UInt32 i = 1;
            bool found = false;
            while ( i + 1 < remaining )
            {
                if ( bufPtr[ processedOffset + i ]     == '8' &&
                     bufPtr[ processedOffset + i + 1 ] == '=' )
                {
                    if ( processedOffset + i == 0 ||
                         bufPtr[ processedOffset + i - 1 ] == CFIXClientMessage::SOH )
                    {
                        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                            "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: Framing resync - skipping " +
                            CORE::ToString( i ) + " bytes" );
                        processedOffset += i;
                        found = true;
                        break;
                    }
                }
                ++i;
            }
            if ( !found )
            {
                processedOffset = bufLen;
                break;
            }
            continue;
        }

        // Find end of tag 8 value
        const char* tag8ValStart = msgStart + 2;
        const char* tag8ValEnd   = tag8ValStart;
        const char* bufEnd       = bufPtr + bufLen;
        while ( tag8ValEnd < bufEnd && *tag8ValEnd != CFIXClientMessage::SOH )
            ++tag8ValEnd;
        if ( tag8ValEnd >= bufEnd )
            break;

        // [S5] Validate BeginString
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
                "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: [S5] BeginString invalid - discarding" );
            processedOffset = (CORE::UInt32)( tag8ValEnd - bufPtr ) + 1;
            continue;
        }

        // Locate "9=" (BodyLength)
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
        {
            if ( afterTag8 + 20 > bufEnd )
                break;
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: Cannot locate tag 9, skipping" );
            processedOffset = (CORE::UInt32)( tag8ValEnd - bufPtr ) + 1;
            continue;
        }

        // Parse BodyLength
        const char* bodyLenStart = tag9Pos + 2;
        const char* bodyLenEnd   = bodyLenStart;
        while ( bodyLenEnd < bufEnd && *bodyLenEnd != CFIXClientMessage::SOH )
            ++bodyLenEnd;
        if ( bodyLenEnd >= bufEnd )
            break;

        CORE::UInt32 bodyLen = (CORE::UInt32) CFIXClientMessage::ParseUInt64Inline(
            bodyLenStart, (CORE::UInt32)( bodyLenEnd - bodyLenStart ) );

        // [S1] Max message size cap
        if ( bodyLen > cfg.maxMsgSizeBytes )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT,
                "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: [S1] BodyLength " +
                CORE::ToString( bodyLen ) + " exceeds maxMsgSizeBytes " +
                CORE::ToString( cfg.maxMsgSizeBytes ) + " - closing connection" );
            if ( GUCEF_NULL != m_activeConnection )
            {
                UnregisterConnectionEventHandlers( m_activeConnection );
                m_activeConnection->Close();
                m_activeConnection = GUCEF_NULL;
            }
            m_sessionState = STATE_NO_CONNECTION;
            m_receiveBuffer.Clear();
            return;
        }

        const char* bodyStart = bodyLenEnd + 1;

        // [S2] 64-bit overflow-safe bounds check
        CORE::UInt64 safeEnd = (CORE::UInt64)( bodyStart - bufPtr ) +
                               (CORE::UInt64)bodyLen +
                               (CORE::UInt64)7;
        if ( safeEnd > (CORE::UInt64)bufLen )
            break;

        // O(1) jump to tag 10
        const char* tag10Pos = bodyStart + bodyLen;
        bool tag10Valid = ( (CORE::UInt32)( bufEnd - tag10Pos ) >= 3 &&
                            tag10Pos[ 0 ] == '1' && tag10Pos[ 1 ] == '0' && tag10Pos[ 2 ] == '=' &&
                            ( tag10Pos == bufPtr || *( tag10Pos - 1 ) == CFIXClientMessage::SOH ) );
        if ( !tag10Valid )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: BodyLength jump missed tag 10 - fallback scan" );
            tag10Pos = GUCEF_NULL;
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
                    "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: Cannot locate tag 10, skipping" );
                processedOffset = (CORE::UInt32)( bodyStart - bufPtr );
                continue;
            }
        }

        const char* checksumValStart = tag10Pos + 3;
        const char* checksumValEnd   = checksumValStart;
        while ( checksumValEnd < bufEnd && *checksumValEnd != CFIXClientMessage::SOH )
            ++checksumValEnd;
        if ( checksumValEnd >= bufEnd )
            break;

        const char* msgEnd = checksumValEnd + 1;
        CORE::UInt32 msgLen = (CORE::UInt32)( msgEnd - msgStart );

        // [S7] Checksum validation
        if ( !cfg.disableChecksumValidation )
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
                    "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: [S7] Checksum mismatch claimed=" +
                    CORE::ToString( claimedChecksum ) + " calc=" + CORE::ToString( calcChecksum ) );
                ++m_consecutiveChecksumFailures;
                if ( m_consecutiveChecksumFailures >= cfg.maxConsecutiveChecksumFailures )
                {
                    GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT,
                        "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: " +
                        CORE::ToString( m_consecutiveChecksumFailures ) +
                        " consecutive checksum failures - closing connection" );
                    if ( GUCEF_NULL != m_activeConnection )
                    {
                        UnregisterConnectionEventHandlers( m_activeConnection );
                        m_activeConnection->Close();
                        m_activeConnection = GUCEF_NULL;
                    }
                    m_sessionState = STATE_NO_CONNECTION;
                    m_receiveBuffer.Clear();
                    return;
                }
                processedOffset += msgLen;
                continue;
            }
            m_consecutiveChecksumFailures = 0;
        }

        // Scan session fields
        CFIXClientSessionFields fields;
        if ( !CFIXClientMessage::ScanSessionFields( msgStart, msgLen, fields ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXServerPubSubClientTopic::ProcessReceiveBuffer: ScanSessionFields failed - dropping" );
            processedOffset += msgLen;
            continue;
        }

        // Dispatch BEFORE buffer compaction
        DispatchIncomingMessage( msgStart, msgLen, fields );

        if ( m_sessionState == STATE_NO_CONNECTION )
            return;

        processedOffset += msgLen;
    }

    // [S8] Safety clamp
    if ( processedOffset > bufLen )
        processedOffset = bufLen;

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
CFIXServerPubSubClientTopic::DispatchIncomingMessage( const char* msgStart                  ,
                                                       CORE::UInt32 msgLen                   ,
                                                       const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    if ( GUCEF_NULL == fields.msgTypeStart )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClientTopic::DispatchIncomingMessage: Missing MsgType (tag 35)" );
        return;
    }

    CORE::UInt64 incomingSeqNum = fields.seqNumVal;

    // Sequence number tracking (skip for SequenceReset)
    if ( !CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "4" ) && incomingSeqNum > 0 )
    {
        if ( incomingSeqNum > m_expectedIncomingSeqNum )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXServerPubSubClientTopic: Sequence gap. Expected=" +
                CORE::ToString( m_expectedIncomingSeqNum ) +
                " Got=" + CORE::ToString( incomingSeqNum ) );

            if ( m_sessionState == STATE_ACTIVE && GUCEF_NULL != m_activeConnection &&
                 m_resendRequestSentForExpectedSeq != m_expectedIncomingSeqNum )
            {
                const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();
                CORE::CAsciiString resendReq = CFIXClientMessage::BuildResendRequest(
                    cfg.senderCompId, CORE::CAsciiString( m_config.topicName ),
                    cfg.fixVersion, m_outgoingSeqNum,
                    m_expectedIncomingSeqNum, incomingSeqNum - 1 );
                m_activeConnection->Send( resendReq.C_String(), (CORE::UInt32) resendReq.Length() );
                ++m_outgoingSeqNum;
                m_resendRequestSentForExpectedSeq = m_expectedIncomingSeqNum;
            }

            if ( GUCEF_NULL != fields.possDupFlagStart &&
                 CFIXClientMessage::FieldMatchesValue( fields.possDupFlagStart, fields.possDupFlagLen, "Y" ) &&
                 incomingSeqNum < m_expectedIncomingSeqNum )
                return;
        }
        else if ( incomingSeqNum < m_expectedIncomingSeqNum )
        {
            if ( GUCEF_NULL != fields.possDupFlagStart &&
                 CFIXClientMessage::FieldMatchesValue( fields.possDupFlagStart, fields.possDupFlagLen, "Y" ) )
            {
                GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL,
                    "CFIXServerPubSubClientTopic: PossDup lower seqnum - already processed. Expected=" +
                    CORE::ToString( m_expectedIncomingSeqNum ) +
                    " Got=" + CORE::ToString( incomingSeqNum ) );
                return;
            }
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
                "CFIXServerPubSubClientTopic: Unexpected lower seqnum. Expected=" +
                CORE::ToString( m_expectedIncomingSeqNum ) +
                " Got=" + CORE::ToString( incomingSeqNum ) );
            return;
        }
        else
        {
            ++m_expectedIncomingSeqNum;
            m_resendRequestSentForExpectedSeq = 0;
        }
    }

    if      ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "A" ) ) HandleLogon( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "5" ) ) HandleLogout( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "0" ) ) HandleHeartbeat( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "1" ) ) HandleTestRequest( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "2" ) ) HandleResendRequest( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "4" ) ) HandleSequenceReset( msgStart, msgLen, fields );
    else if ( CFIXClientMessage::FieldMatchesValue( fields.msgTypeStart, fields.msgTypeLen, "3" ) ) HandleReject( msgStart, msgLen, fields );
    else
    {
        DeliverToSubscribers( msgStart, msgLen, fields );
    }
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleLogon( const char* msgStart                  ,
                                           CORE::UInt32 msgLen                   ,
                                           const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClientTopic::HandleLogon: Logon received for topic " + m_config.topicName );

    if ( GUCEF_NULL == m_activeConnection )
        return;

    // Guard: if the session is already active, this is a duplicate Logon arriving on an established
    // connection. This can happen as a re-dispatch artifact when the GUCEF notification system
    // delivers the current DataRecievedEvent to a handler that was subscribed mid-dispatch.
    // Sending a second Logon ACK would confuse the counterparty and corrupt sequence numbers.
    if ( m_sessionState == STATE_ACTIVE )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClientTopic::HandleLogon: Session already active for topic \"" +
            m_config.topicName + "\" - ignoring duplicate Logon" );
        // DispatchIncomingMessage already incremented m_expectedIncomingSeqNum for this Logon.
        // Undo that increment so the expected counter stays at the post-reset value (1).
        if ( m_expectedIncomingSeqNum > 1 )
            --m_expectedIncomingSeqNum;
        return;
    }

    // Stop logon timeout since Logon arrived
    if ( GUCEF_NULL != m_logonTimeoutTimer )
        m_logonTimeoutTimer->SetEnabled( false );

    const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();

    // Determine the remote CompID to use as the TargetCompID in our reply
    CORE::CAsciiString remoteCompId;
    if ( GUCEF_NULL != fields.senderStart && fields.senderLen > 0 )
        remoteCompId = CORE::CAsciiString( fields.senderStart, fields.senderLen );
    else
        remoteCompId = CORE::CAsciiString( m_config.topicName );

    // If counterparty requests sequence reset (tag 141=Y), reset our expected incoming
    if ( GUCEF_NULL != fields.resetFlagStart &&
         CFIXClientMessage::FieldMatchesValue( fields.resetFlagStart, fields.resetFlagLen, "Y" ) )
    {
        m_expectedIncomingSeqNum = 1;
    }

    // Reply with server Logon
    CORE::CAsciiString logonReply = CFIXClientMessage::BuildLogon(
        cfg.senderCompId, remoteCompId,
        cfg.fixVersion, m_outgoingSeqNum,
        cfg.heartbeatIntervalSecs, false /* no reset */ );
    m_activeConnection->Send( logonReply.C_String(), (CORE::UInt32) logonReply.Length() );
    ++m_outgoingSeqNum;

    m_sessionState = STATE_ACTIVE;

    if ( GUCEF_NULL != m_heartbeatTimer )
        m_heartbeatTimer->SetEnabled( true );

    // Also deliver Logon to subscribers if configured
    DeliverToSubscribers( msgStart, msgLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleLogout( const char* msgStart                  ,
                                            CORE::UInt32 msgLen                   ,
                                            const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClientTopic::HandleLogout: Logout received for topic " + m_config.topicName );

    if ( GUCEF_NULL != m_heartbeatTimer )
        m_heartbeatTimer->SetEnabled( false );

    // Also deliver Logout to subscribers if configured (before closing)
    DeliverToSubscribers( msgStart, msgLen, fields );

    if ( m_sessionState == STATE_ACTIVE && GUCEF_NULL != m_activeConnection )
    {
        const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();
        CORE::CAsciiString remoteCompId;
        if ( GUCEF_NULL != fields.senderStart && fields.senderLen > 0 )
            remoteCompId = CORE::CAsciiString( fields.senderStart, fields.senderLen );
        else
            remoteCompId = CORE::CAsciiString( m_config.topicName );

        CORE::CAsciiString logoutReply = CFIXClientMessage::BuildLogout(
            cfg.senderCompId, remoteCompId,
            cfg.fixVersion, m_outgoingSeqNum );
        m_activeConnection->Send( logoutReply.C_String(), (CORE::UInt32) logoutReply.Length() );
        ++m_outgoingSeqNum;
    }

    m_sessionState = STATE_LOGGING_OUT;

    if ( GUCEF_NULL != m_activeConnection )
    {
        UnregisterConnectionEventHandlers( m_activeConnection );
        m_activeConnection->Close();
        m_activeConnection = GUCEF_NULL;
    }
    m_sessionState = STATE_NO_CONNECTION;
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleHeartbeat( const char* msgStart                  ,
                                               CORE::UInt32 msgLen                   ,
                                               const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    // Echo TestReqID back if present
    if ( GUCEF_NULL != fields.testReqIdStart && fields.testReqIdLen > 0 &&
         GUCEF_NULL != m_activeConnection )
    {
        const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();
        CORE::CAsciiString remoteCompId;
        if ( GUCEF_NULL != fields.senderStart && fields.senderLen > 0 )
            remoteCompId = CORE::CAsciiString( fields.senderStart, fields.senderLen );
        else
            remoteCompId = CORE::CAsciiString( m_config.topicName );

        CORE::CAsciiString testReqId( fields.testReqIdStart, fields.testReqIdLen );
        CORE::CAsciiString heartbeat = CFIXClientMessage::BuildHeartbeat(
            cfg.senderCompId, remoteCompId,
            cfg.fixVersion, m_outgoingSeqNum, testReqId );
        m_activeConnection->Send( heartbeat.C_String(), (CORE::UInt32) heartbeat.Length() );
        ++m_outgoingSeqNum;
    }

    DeliverToSubscribers( msgStart, msgLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleTestRequest( const char* msgStart                  ,
                                                 CORE::UInt32 msgLen                   ,
                                                 const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_activeConnection )
        return;

    const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();
    CORE::CAsciiString remoteCompId;
    if ( GUCEF_NULL != fields.senderStart && fields.senderLen > 0 )
        remoteCompId = CORE::CAsciiString( fields.senderStart, fields.senderLen );
    else
        remoteCompId = CORE::CAsciiString( m_config.topicName );

    CORE::CAsciiString testReqId;
    if ( GUCEF_NULL != fields.testReqIdStart && fields.testReqIdLen > 0 )
        testReqId = CORE::CAsciiString( fields.testReqIdStart, fields.testReqIdLen );

    CORE::CAsciiString heartbeat = CFIXClientMessage::BuildHeartbeat(
        cfg.senderCompId, remoteCompId,
        cfg.fixVersion, m_outgoingSeqNum, testReqId );
    m_activeConnection->Send( heartbeat.C_String(), (CORE::UInt32) heartbeat.Length() );
    ++m_outgoingSeqNum;

    DeliverToSubscribers( msgStart, msgLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleResendRequest( const char* msgStart                  ,
                                                   CORE::UInt32 msgLen                   ,
                                                   const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    CORE::UInt64 beginSeqNo = 0;
    if ( GUCEF_NULL != fields.beginSeqNoStart && fields.beginSeqNoLen > 0 )
        beginSeqNo = CFIXClientMessage::ParseUInt64Inline( fields.beginSeqNoStart, fields.beginSeqNoLen );

    CORE::UInt64 endSeqNo = 0; // 0 = replay to end of available data (tag 16 EndSeqNo not yet in session fields)

    // Attempt out-of-band replay via the flow router / persistence side
    PUBSUB::CPubSubClient* client = GetClient();
    if ( GUCEF_NULL != client )
    {
        PUBSUB::CPubSubClientSide* parentSide = client->GetParentSide();
        if ( GUCEF_NULL != parentSide )
        {
            PUBSUB::CPubSubBookmark startBookmark = PUBSUB::CPubSubBookmark::MakeIndexKeyValueBookmark(
                "mk:fix_seq_num", CORE::CVariant( beginSeqNo ) );
            PUBSUB::CPubSubBookmark endBookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_NOT_INITIALIZED ); // 0 = to end of data
            if ( endSeqNo != 0 )
                endBookmark = PUBSUB::CPubSubBookmark::MakeIndexKeyValueBookmark( "mk:fix_seq_num", CORE::CVariant( endSeqNo ) );

            CORE::UInt64 replayRequestId = 0;
            if ( parentSide->OnReplayRequested( this, startBookmark, endBookmark, replayRequestId ) )
            {
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CFIXServerPubSubClientTopic::HandleResendRequest: "
                    "Replay initiated via router for beginSeqNo=" + CORE::ToString( beginSeqNo ) +
                    " endSeqNo=" + CORE::ToString( endSeqNo ) +
                    " replayRequestId=" + CORE::ToString( replayRequestId ) );
                DeliverToSubscribers( msgStart, msgLen, fields );
                return;
            }
        }
    }

    // Fallback: no persistence side available — send GapFill SequenceReset as before
    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClientTopic::HandleResendRequest: Replay not available, falling back to GapFill SequenceReset" );

    if ( GUCEF_NULL == m_activeConnection )
    {
        DeliverToSubscribers( msgStart, msgLen, fields );
        return;
    }

    const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();
    CORE::CAsciiString remoteCompId;
    if ( GUCEF_NULL != fields.senderStart && fields.senderLen > 0 )
        remoteCompId = CORE::CAsciiString( fields.senderStart, fields.senderLen );
    else
        remoteCompId = CORE::CAsciiString( m_config.topicName );

    CORE::CAsciiString seqReset = CFIXClientMessage::BuildSequenceReset(
        cfg.senderCompId, remoteCompId,
        cfg.fixVersion, beginSeqNo, m_outgoingSeqNum, true /* gapFill */ );
    m_activeConnection->Send( seqReset.C_String(), (CORE::UInt32) seqReset.Length() );

    DeliverToSubscribers( msgStart, msgLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleSequenceReset( const char* msgStart                  ,
                                                   CORE::UInt32 msgLen                   ,
                                                   const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    CORE::UInt64 newSeqNo = 0;
    if ( GUCEF_NULL != fields.newSeqNoStart && fields.newSeqNoLen > 0 )
        newSeqNo = CFIXClientMessage::ParseUInt64Inline( fields.newSeqNoStart, fields.newSeqNoLen );

    if ( newSeqNo > 0 )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClientTopic::HandleSequenceReset: Resetting expected incoming seq to " +
            CORE::ToString( newSeqNo ) );
        m_expectedIncomingSeqNum = newSeqNo;
        m_resendRequestSentForExpectedSeq = 0;
    }

    DeliverToSubscribers( msgStart, msgLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::HandleReject( const char* msgStart                  ,
                                            CORE::UInt32 msgLen                   ,
                                            const CFIXClientSessionFields& fields )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClientTopic::HandleReject: Reject received" );

    DeliverToSubscribers( msgStart, msgLen, fields );
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::OnConnectionDataReceived( CORE::CNotifier* notifier    ,
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

    CORE::UInt32 existingSize = m_receiveBuffer.GetDataSize();
    m_receiveBuffer.SetDataSize( existingSize + buffer.GetDataSize() );
    ::memcpy( static_cast< char* >( m_receiveBuffer.GetBufferPtr() ) + existingSize,
              buffer.GetConstBufferPtr(),
              buffer.GetDataSize() );

    ProcessReceiveBuffer();
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::OnConnectionDisconnected( CORE::CNotifier* notifier    ,
                                                        const CORE::CEvent& eventId  ,
                                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClientTopic::OnConnectionDisconnected: Connection dropped for topic " +
        m_config.topicName );

    if ( GUCEF_NULL != m_heartbeatTimer )
        m_heartbeatTimer->SetEnabled( false );
    if ( GUCEF_NULL != m_logonTimeoutTimer )
        m_logonTimeoutTimer->SetEnabled( false );

    // Unregister but do NOT delete — the server socket owns the connection object
    if ( GUCEF_NULL != m_activeConnection )
    {
        UnregisterConnectionEventHandlers( m_activeConnection );
        m_activeConnection = GUCEF_NULL;
    }

    m_sessionState = STATE_NO_CONNECTION;
    m_receiveBuffer.Clear();
    m_consecutiveChecksumFailures = 0;
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::OnHeartbeatTimerCycle( CORE::CNotifier* notifier    ,
                                                     const CORE::CEvent& eventId  ,
                                                     CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_sessionState != STATE_ACTIVE || GUCEF_NULL == m_activeConnection )
        return;

    const CFIXServerPubSubClientConfig& cfg = m_client->GetConfig();
    CORE::CAsciiString testReqId = CORE::CAsciiString( "HEARTBEAT_" ) + CORE::CAsciiString( CORE::ToString( m_outgoingSeqNum ) );
    CORE::CAsciiString testReq = CFIXClientMessage::BuildTestRequest(
        cfg.senderCompId, CORE::CAsciiString( m_config.topicName ),
        cfg.fixVersion, m_outgoingSeqNum, testReqId );
    m_activeConnection->Send( testReq.C_String(), (CORE::UInt32) testReq.Length() );
    ++m_outgoingSeqNum;
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::OnLogonTimeoutTimerCycle( CORE::CNotifier* notifier    ,
                                                        const CORE::CEvent& eventId  ,
                                                        CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( m_sessionState == STATE_WAITING_LOGON )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL,
            "CFIXServerPubSubClientTopic::OnLogonTimeoutTimerCycle: Logon timeout for topic " +
            m_config.topicName + " - closing pending connection" );

        if ( GUCEF_NULL != m_logonTimeoutTimer )
            m_logonTimeoutTimer->SetEnabled( false );

        if ( GUCEF_NULL != m_activeConnection )
        {
            UnregisterConnectionEventHandlers( m_activeConnection );
            m_activeConnection->Close();
            m_activeConnection = GUCEF_NULL;
        }
        m_sessionState = STATE_NO_CONNECTION;
        m_receiveBuffer.Clear();
    }
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::Publish( CORE::UInt64& publishActionId           ,
                                       const PUBSUB::CIPubSubMsg& msg          ,
                                       bool notify                             )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !m_client->GetConfig().allowPublishing )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXServerPubSubClientTopic::Publish: Publishing not enabled" );
        return false;
    }

    CORE::CAsciiString rawFix;
    const CORE::CVariant& payload = msg.GetPrimaryPayload();
    if ( !payload.IsNULL() )
    {
        rawFix = CORE::CAsciiString( payload.AsString( CORE::CString::Empty, true ) );
    }

    publishActionId = m_currentPublishActionId;
    ++m_currentPublishActionId;

    bool sendOk = false;
    if ( !rawFix.IsNULLOrEmpty() )
        sendOk = SendRawFix( rawFix );
    else
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXServerPubSubClientTopic::Publish: No primary payload" );
    }

    if ( notify )
    {
        if ( sendOk )
        {
            m_publishSuccessActionIds.clear();
            m_publishSuccessActionIds.push_back( publishActionId );
            NotifyObservers( MsgsPublishedEvent, &m_publishSuccessActionEventData );
        }
        else
        {
            m_publishFailureActionIds.clear();
            m_publishFailureActionIds.push_back( publishActionId );
            NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData );
        }
    }
    return sendOk;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXServerPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    return m_config.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CFIXServerPubSubClientTopicConfig&
CFIXServerPubSubClientTopic::GetTopicConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXServerPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::FIX::CFIXServerPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFIXServerPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CFIXServerPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CFIXServerPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CFIXServerPubSubClientTopic::OnReplayComplete( CORE::UInt64 replayRequestId )
{GUCEF_TRACE;

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL,
        "CFIXServerPubSubClientTopic::OnReplayComplete: Replay completed for replayRequestId=" +
        CORE::ToString( replayRequestId ) + " on topic \"" + m_config.topicName + "\"" );
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
