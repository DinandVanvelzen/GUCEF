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

#ifndef GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#include "gucefPUBSUB_CBasicPubSubMsg.h"
#define GUCEF_PUBSUB_CBASICPUBSUBMSG_H
#endif /* GUCEF_PUBSUB_CBASICPUBSUBMSG_H ? */

#ifndef GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#include "gucefPUBSUB_CPubSubBookmark.h"
#define GUCEF_PUBSUB_CPUBSUBBOOKMARK_H
#endif /* GUCEF_PUBSUB_CPUBSUBBOOKMARK_H ? */

#ifndef PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENT_H
#include "pubsubpluginFIX_CFIXPubSubClient.h"
#define PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENT_H
#endif /* PUBSUBPLUGIN_FIX_CFIXPUBSUBCLIENT_H ? */

#include "pubsubpluginFIX_CFIXPubSubClientTopic.h"

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

CFIXPubSubClientTopic::CFIXPubSubClientTopic( CFIXPubSubClient* client )
    : PUBSUB::CPubSubClientTopic( client->GetPulseGenerator() )
    , CORE::CTSharedObjCreator< CFIXPubSubClientTopic, MT::CMutex >( this )
    , m_client( client )
    , m_config()
    , m_isSubscribed( false )
    , m_lastReceivedSeqNum( 0 )
    , m_currentPublishActionId( 1 )
    , m_currentReceiveActionId( 1 )
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
}

/*-------------------------------------------------------------------------*/

CFIXPubSubClientTopic::~CFIXPubSubClientTopic()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClientTopic::Shutdown( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_client = GUCEF_NULL;
    SignalUpcomingDestruction();
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubClient*
CFIXPubSubClientTopic::GetClient( void )
{GUCEF_TRACE;

    return m_client;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::InitializeConnectivity( bool reset )
{GUCEF_TRACE;

    // Connectivity is managed at the client (TCP session) level
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::Disconnect( void )
{GUCEF_TRACE;

    m_isSubscribed = false;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::IsConnected( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        return m_client->IsConnected();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::IsSubscribed( void ) const
{GUCEF_TRACE;

    return m_isSubscribed;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::IsHealthy( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        return m_client->IsConnected() || m_client->IsInitialized();
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::IsPublishingSupported( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        return m_client->GetConfig().allowPublishing;
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::IsSubscribingSupported( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::Subscribe( void )
{GUCEF_TRACE;

    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::SubscribeStartingAtBookmark( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    // Decode the sequence number from the bookmark and request a replay
    CORE::UInt64 fromSeqNum = bookmark.GetBookmarkData().AsUInt64( 0, true );
    if ( fromSeqNum > 0 && GUCEF_NULL != m_client )
    {
        m_client->RequestReplayFrom( fromSeqNum );
    }
    m_isSubscribed = true;
    return true;
}

/*-------------------------------------------------------------------------*/

PUBSUB::CPubSubBookmark
CFIXPubSubClientTopic::GetCurrentBookmark( void )
{GUCEF_TRACE;

    PUBSUB::CPubSubBookmark bookmark( PUBSUB::CPubSubBookmark::BOOKMARK_TYPE_MSG_INDEX );
    bookmark.SetBookmarkData( CORE::CVariant( m_lastReceivedSeqNum ) );
    return bookmark;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXPubSubClientTopic::GetTopicName( void ) const
{GUCEF_TRACE;

    return m_config.topicName;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::IsMsgTypePassedByFilter( const char* msgType,
                                                  CORE::UInt32 msgTypeLen ) const
{GUCEF_TRACE;

    if ( m_config.msgTypeFilter.IsNULLOrEmpty() )
        return true;  // empty filter = accept all

    // Scan comma-separated filter list with memcmp — no CString allocation
    const char* filterPtr = m_config.msgTypeFilter.C_String();
    CORE::UInt32 filterLen = (CORE::UInt32) m_config.msgTypeFilter.Length();
    CORE::UInt32 start = 0;
    while ( start < filterLen )
    {
        CORE::UInt32 end = start;
        while ( end < filterLen && filterPtr[ end ] != ',' )
            ++end;

        // Trim leading spaces
        CORE::UInt32 tokenStart = start;
        while ( tokenStart < end && filterPtr[ tokenStart ] == ' ' )
            ++tokenStart;
        // Trim trailing spaces
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
CFIXPubSubClientTopic::OnApplicationMessage( const char* msgStart, CORE::UInt32 msgLen,
                                              const CFIXSessionFields& fields )
{GUCEF_TRACE;

    if ( !m_isSubscribed )
        return;

    // Check session-level filter — inline char comparison, no CString allocation
    if ( IsSessionLevelMsgType( fields.msgTypeStart, fields.msgTypeLen ) &&
         !m_config.includeSessionLevelMsgs )
        return;

    // Check MsgType filter — memcmp scan, no CString allocation
    if ( GUCEF_NULL != fields.msgTypeStart &&
         !IsMsgTypePassedByFilter( fields.msgTypeStart, fields.msgTypeLen ) )
        return;

    MT::CScopeMutex lock( m_lock );

    m_pubsubMsgs.resize( 1 );
    PUBSUB::CBasicPubSubMsg& msg = m_pubsubMsgs[ 0 ];
    msg.Clear();

    // MsgId: link seqNum ASCII bytes from buffer — no copy
    if ( GUCEF_NULL != fields.seqNumStart )
        msg.GetMsgId().LinkTo( fields.seqNumStart, fields.seqNumLen, GUCEF_DATATYPE_ASCII_STRING );

    // MsgIndex: inline UInt64 scalar — no heap allocation
    msg.GetMsgIndex() = CORE::CVariant( fields.seqNumVal );

    msg.GetMsgDateTime() = CORE::CDateTime::NowUTCDateTime();

    // PRIMARY PAYLOAD: FIX is a text protocol — typed as ASCII_STRING, linked, no copy
    msg.GetPrimaryPayload().LinkTo( msgStart, msgLen, GUCEF_DATATYPE_ASCII_STRING );

    // METADATA: numeric UInt32 tag keys (inline scalars) + linked ASCII_STRING value views
    // Only add fields that were actually found (null-check guards)
    // All calls use AddLinkedMetaDataKeyValuePair so value variants are stored linked (no copy)

    if ( GUCEF_NULL != fields.msgTypeStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.msgTypeStart, fields.msgTypeLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXMessage::TAG_MSG_TYPE ), valueVar );
    }
    if ( GUCEF_NULL != fields.beginStringStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.beginStringStart, fields.beginStringLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXMessage::TAG_BEGIN_STRING ), valueVar );
    }
    if ( GUCEF_NULL != fields.senderStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.senderStart, fields.senderLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXMessage::TAG_SENDER_COMP_ID ), valueVar );
    }
    if ( GUCEF_NULL != fields.targetStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.targetStart, fields.targetLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXMessage::TAG_TARGET_COMP_ID ), valueVar );
    }
    if ( GUCEF_NULL != fields.seqNumStart )
    {
        CORE::CVariant valueVar;
        valueVar.LinkTo( fields.seqNumStart, fields.seqNumLen, GUCEF_DATATYPE_ASCII_STRING );
        msg.AddLinkedMetaDataKeyValuePair(
            CORE::CVariant( (CORE::UInt32)CFIXMessage::TAG_MSG_SEQ_NUM ), valueVar );
    }

    msg.SetReceiveActionId( m_currentReceiveActionId );
    ++m_currentReceiveActionId;

    m_lastReceivedSeqNum = fields.seqNumVal;

    // Build refs vector and notify synchronously
    // IMPORTANT: linked views into msgStart are valid because ProcessReceiveBuffer
    // performs the memmove compaction AFTER this call returns
    m_pubsubMsgsRefs.clear();
    m_pubsubMsgsRefs.push_back( &m_pubsubMsgs[ 0 ] );
    NotifyObservers( MsgsRecievedEvent, &m_pubsubMsgsRefs );
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::Publish( CORE::UInt64& publishActionId,
                                 const PUBSUB::CIPubSubMsg& msg,
                                 bool notify )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !m_client->GetConfig().allowPublishing )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClientTopic::Publish: Publishing not enabled" );
        return false;
    }

    // Use the primary payload as a raw FIX string if available
    CORE::CString rawFix;
    const CORE::CVariant& payload = msg.GetPrimaryPayload();
    if ( !payload.IsNULL() )
    {
        rawFix = payload.AsString( CORE::CString::Empty, true );
    }

    if ( rawFix.IsNULLOrEmpty() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CFIXPubSubClientTopic::Publish: No primary payload to send" );
        publishActionId = m_currentPublishActionId;
        ++m_currentPublishActionId;

        m_publishFailureActionIds.clear();
        m_publishFailureActionIds.push_back( publishActionId );
        if ( notify )
            NotifyObservers( MsgsPublishFailureEvent, &m_publishFailureActionEventData );
        return false;
    }

    publishActionId = m_currentPublishActionId;
    ++m_currentPublishActionId;

    bool sendOk = m_client->SendRawFix( rawFix );

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
CFIXPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CIPubSubMsg& msg )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        m_client->SaveSequenceBookmark();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::AcknowledgeReceipt( const PUBSUB::CPubSubBookmark& bookmark )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_client )
        m_client->SaveSequenceBookmark();
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::SaveConfig( PUBSUB::CPubSubClientTopicConfig& config ) const
{GUCEF_TRACE;

    config = m_config;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CFIXPubSubClientTopic::LoadConfig( const PUBSUB::CPubSubClientTopicConfig& config )
{GUCEF_TRACE;

    return m_config.LoadConfig( config );
}

/*-------------------------------------------------------------------------*/

const CFIXPubSubClientTopicConfig&
CFIXPubSubClientTopic::GetTopicConfig( void ) const
{GUCEF_TRACE;

    return m_config;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CFIXPubSubClientTopic::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PUBSUBPLUGIN::FIX::CFIXPubSubClientTopic";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const MT::CILockable*
CFIXPubSubClientTopic::AsLockable( void ) const
{GUCEF_TRACE;

    return this;
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CFIXPubSubClientTopic::Lock( CORE::UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_lock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CFIXPubSubClientTopic::Unlock( void ) const
{GUCEF_TRACE;

    return m_lock.Unlock();
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
