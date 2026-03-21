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

// FIX session-level MsgTypes that are normally not forwarded to subscribers
static bool IsSessionLevelMsgType( const CORE::CString& msgType )
{
    return msgType == "0" ||  // Heartbeat
           msgType == "1" ||  // TestRequest
           msgType == "2" ||  // ResendRequest
           msgType == "3" ||  // Reject
           msgType == "4" ||  // SequenceReset
           msgType == "5" ||  // Logout
           msgType == "A";    // Logon
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
CFIXPubSubClientTopic::IsMsgTypePassedByFilter( const CORE::CString& msgType ) const
{GUCEF_TRACE;

    if ( m_config.msgTypeFilter.IsNULLOrEmpty() )
        return true;  // empty filter = accept all

    // Filter is comma-separated list of MsgTypes
    const char* filterPtr = m_config.msgTypeFilter.C_String();
    CORE::UInt32 filterLen = (CORE::UInt32) m_config.msgTypeFilter.Length();
    CORE::UInt32 start = 0;
    while ( start < filterLen )
    {
        CORE::UInt32 end = start;
        while ( end < filterLen && filterPtr[ end ] != ',' )
            ++end;

        CORE::CString token( filterPtr + start, end - start );
        token = token.Trim( true );
        if ( token == msgType )
            return true;

        start = end + 1;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

void
CFIXPubSubClientTopic::OnApplicationMessage( const CFIXMessage& fixMsg )
{GUCEF_TRACE;

    if ( !m_isSubscribed )
        return;

    CORE::CString msgType = fixMsg.GetMsgType();

    // Check session-level filter
    if ( IsSessionLevelMsgType( msgType ) && !m_config.includeSessionLevelMsgs )
        return;

    // Check MsgType filter
    if ( !IsMsgTypePassedByFilter( msgType ) )
        return;

    MT::CScopeMutex lock( m_lock );

    // Build a pubsub message
    m_pubsubMsgs.resize( 1 );
    PUBSUB::CBasicPubSubMsg& msg = m_pubsubMsgs[ 0 ];

    // Set message identity fields
    msg.GetMsgId() = fixMsg.GetMsgSeqNum();
    msg.GetMsgIndex() = fixMsg.GetMsgSeqNumAsUInt64();
    msg.GetMsgDateTime() = CORE::CDateTime::NowUTCDateTime();

    // Set raw FIX bytes as primary payload
    const CORE::CString& rawMsg = fixMsg.GetRawMessage();
    msg.GetPrimaryPayload() = CORE::CVariant( rawMsg.C_String(), (CORE::UInt32) rawMsg.Length(), GUCEF_DATATYPE_BINARY_BLOB );

    // Optionally emit all FIX fields as key-value pairs
    if ( m_config.parseFieldsAsKeyValues )
    {
        const CFIXMessage::TFixFieldMap& fields = fixMsg.GetFields();
        CFIXMessage::TFixFieldMap::const_iterator i = fields.begin();
        while ( i != fields.end() )
        {
            msg.AddKeyValuePair( CORE::CVariant( i->first ), CORE::CVariant( i->second ) );
            ++i;
        }
    }

    // Always add metadata
    msg.AddMetaDataKeyValuePair( CORE::CVariant( CORE::CString( "FIX_MsgType" ) ),
                                 CORE::CVariant( msgType ) );
    msg.AddMetaDataKeyValuePair( CORE::CVariant( CORE::CString( "FIX_BeginString" ) ),
                                 CORE::CVariant( fixMsg.GetBeginString() ) );
    msg.AddMetaDataKeyValuePair( CORE::CVariant( CORE::CString( "FIX_SenderCompID" ) ),
                                 CORE::CVariant( fixMsg.GetField( CFIXMessage::TAG_SENDER_COMP_ID ) ) );
    msg.AddMetaDataKeyValuePair( CORE::CVariant( CORE::CString( "FIX_TargetCompID" ) ),
                                 CORE::CVariant( fixMsg.GetField( CFIXMessage::TAG_TARGET_COMP_ID ) ) );

    msg.SetReceiveActionId( m_currentReceiveActionId );
    ++m_currentReceiveActionId;

    m_lastReceivedSeqNum = fixMsg.GetMsgSeqNumAsUInt64();

    // Build refs vector and notify
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
