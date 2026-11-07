/*
 *  matchprovidermail: gucefMULTIPLAY match provider plugin that carries matches between players as e-mail
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

#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_COM_CMIMEPARSER_H
#include "gucefCOM_CMimeParser.h"
#define GUCEF_COM_CMIMEPARSER_H
#endif /* GUCEF_COM_CMIMEPARSER_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILFILES_H
#include "matchprovidermail_CMailFiles.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILFILES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILFILES_H ? */

#include "matchprovidermail_CMailInbox.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace MULTIPLAYPLUGIN {
namespace MAIL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CMailInbox::MessageEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInbox::MessageEvent";
const CORE::CEvent CMailInbox::RefusedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInbox::RefusedEvent";
const CORE::CEvent CMailInbox::StateEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInbox::StateEvent";
const CORE::CEvent CMailInbox::NoticeEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInbox::NoticeEvent";

static const UInt32 g_maxStateFileBytes = 4 * 1024 * 1024;
static const UInt32 g_maxIdleRenewMs = 25 * 60 * 1000;
static const UInt32 g_maxMessagesPerCycle = 100;
static const char* const g_stateMagic = "GMIB 1";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMailInboxMessage::CMailInboxMessage( void )
    : CORE::CICloneable()
    , incoming()
    , message()
    , rawSize( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxMessage::CMailInboxMessage( const CMailInboxMessage& src )
    : CORE::CICloneable( src )
    , incoming( src.incoming )
    , message( src.message )
    , rawSize( src.rawSize )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxMessage::~CMailInboxMessage()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxMessage&
CMailInboxMessage::operator=( const CMailInboxMessage& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        incoming = src.incoming;
        message = src.message;
        rawSize = src.rawSize;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailInboxMessage::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailInboxMessage( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMailInboxMessage::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMailInboxMessage::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInboxMessage";
}

/*-------------------------------------------------------------------------*/

CMailInboxRefusal::CMailInboxRefusal( void )
    : CORE::CICloneable()
    , matchId()
    , revision( 0 )
    , kind( MAILKIND_UNKNOWN )
    , reason()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxRefusal::CMailInboxRefusal( const CMailInboxRefusal& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , kind( src.kind )
    , reason( src.reason )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxRefusal::~CMailInboxRefusal()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxRefusal&
CMailInboxRefusal::operator=( const CMailInboxRefusal& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        kind = src.kind;
        reason = src.reason;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailInboxRefusal::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailInboxRefusal( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMailInboxRefusal::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMailInboxRefusal::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInboxRefusal";
}

/*-------------------------------------------------------------------------*/

CMailInboxNotice::CMailInboxNotice( void )
    : CORE::CICloneable()
    , code()
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxNotice::CMailInboxNotice( const CMailInboxNotice& src )
    : CORE::CICloneable( src )
    , code( src.code )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxNotice::~CMailInboxNotice()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailInboxNotice&
CMailInboxNotice::operator=( const CMailInboxNotice& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        code = src.code;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailInboxNotice::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailInboxNotice( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMailInboxNotice::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMailInboxNotice::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailInboxNotice";
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::RegisterEvents( void )
{GUCEF_TRACE;

    MessageEvent.Initialize();
    RefusedEvent.Initialize();
    StateEvent.Initialize();
    NoticeEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMailInbox::CMailInbox( void )
    : CORE::CObservingNotifier()
    , m_settings( GUCEF_NULL )
    , m_credentials( GUCEF_NULL )
    , m_statePath()
    , m_started( false )
    , m_online( false )
    , m_nowMs( 0 )
    , m_nextConnectAt( 0 )
    , m_nextPollAt( 0 )
    , m_backoffMs( 0 )
    , m_checkRequested( false )
    , m_imap( GUCEF_NULL )
    , m_pop3( GUCEF_NULL )
    , m_retired()
    , m_retiredIsImap()
    , m_imapSettings()
    , m_pop3Settings()
    , m_step( STEP_NONE )
    , m_requestId( 0 )
    , m_supportsIdle( false )
    , m_existsAtSearch( 0 )
    , m_uidValidity( 0 )
    , m_lastUid( 0 )
    , m_seenOrder()
    , m_seen()
    , m_handled( 0 )
    , m_uids()
    , m_uidIndex( 0 )
    , m_currentUid( 0 )
    , m_currentIncoming()
    , m_currentSize( 0 )
    , m_currentInteresting( false )
    , m_actionPending( false )
    , m_moveDeletes( false )
    , m_degradedNoticeGiven( false )
    , m_currentRaw()
    , m_popUids()
    , m_popSizes()
    , m_popIndex( 0 )
    , m_popCurrentUid()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMailInbox::~CMailInbox()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    Stop();
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Configure( const CMailMatchProviderSettings* settings, COM::CIMailCredentialProvider* credentials, const CORE::CString& statePath )
{GUCEF_TRACE;

    m_settings = settings;
    m_credentials = credentials;
    m_statePath = statePath;
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::LoadState( void )
{GUCEF_TRACE;

    m_uidValidity = 0;
    m_lastUid = 0;
    m_seenOrder.clear();
    m_seen.clear();
    if ( m_statePath.IsNULLOrEmpty() )
        return true;
    CORE::CDynamicBuffer bytes;
    if ( !CMailFiles::ReadBounded( m_statePath, g_maxStateFileBytes, bytes ) || 0 == bytes.GetDataSize() )
        return true;
    const CORE::CString text( static_cast< const char* >( bytes.GetConstBufferPtr() ), bytes.GetDataSize() );
    CORE::CString::StringVector lines = text.ParseElements( '\n', false );
    if ( lines.size() < 2 || lines[ 0 ] != g_stateMagic )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "MailInbox: The state file of the mailbox is not usable and is ignored" );
        return true;
    }
    CORE::CString::StringVector position = lines[ 1 ].ParseElements( ' ', false );
    if ( position.size() >= 2 )
    {
        m_uidValidity = (UInt32) strtoul( position[ 0 ].C_String(), GUCEF_NULL, 10 );
        m_lastUid = (UInt32) strtoul( position[ 1 ].C_String(), GUCEF_NULL, 10 );
    }
    for ( size_t i=2; i<lines.size(); ++i )
    {
        CORE::CString line = lines[ i ];
        if ( line.EndsWith( "\r" ) )
            line = line.SubstrToIndex( line.Length() - 1, true );
        if ( !line.IsNULLOrEmpty() && line.Length() <= 1000 )
            AddSeen( line );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::SaveState( void )
{GUCEF_TRACE;

    if ( m_statePath.IsNULLOrEmpty() )
        return true;
    CORE::CString text = CORE::CString( g_stateMagic ) + "\n" + CORE::UInt32ToString( m_uidValidity ) + " " + CORE::UInt32ToString( m_lastUid ) + "\n";
    for ( size_t i=0; i<m_seenOrder.size(); ++i )
        text += m_seenOrder[ i ] + "\n";
    return CMailFiles::WriteAtomic( m_statePath, text.C_String(), text.ByteSize() - 1 );
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::IsSeen( const CORE::CString& id ) const
{GUCEF_TRACE;

    return m_seen.find( id ) != m_seen.end();
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::AddSeen( const CORE::CString& id )
{GUCEF_TRACE;

    if ( id.IsNULLOrEmpty() || !m_seen.insert( id ).second )
        return;
    m_seenOrder.push_back( id );
    const size_t limit = GUCEF_NULL != m_settings ? m_settings->seenSetSize : 5000;
    while ( m_seenOrder.size() > limit )
    {
        m_seen.erase( m_seenOrder.front() );
        m_seenOrder.erase( m_seenOrder.begin() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::Start( Int64 nowMs )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_settings )
        return false;
    if ( m_started )
        return true;
    m_nowMs = nowMs;
    LoadState();
    m_started = true;
    m_online = false;
    m_backoffMs = 0;
    m_nextConnectAt = nowMs;
    m_nextPollAt = nowMs;
    m_checkRequested = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Stop( void )
{GUCEF_TRACE;

    if ( m_started )
        SaveState();
    m_started = false;
    Disconnect();
    for ( size_t i=0; i<m_retired.size(); ++i )
    {
        if ( m_retiredIsImap[ i ] )
            delete static_cast< COM::CImapClient* >( m_retired[ i ] );
        else
            delete static_cast< COM::CPop3Client* >( m_retired[ i ] );
    }
    m_retired.clear();
    m_retiredIsImap.clear();
    if ( m_online )
        SetOnline( false, "stopped" );
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::IsStarted( void ) const
{GUCEF_TRACE;

    return m_started;
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::IsOnline( void ) const
{GUCEF_TRACE;

    return m_online;
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::IsIdling( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_imap && STEP_IDLE == m_step && m_imap->IsIdling();
}

/*-------------------------------------------------------------------------*/

UInt32 CMailInbox::GetUidValidity( void ) const { return m_uidValidity; }
UInt32 CMailInbox::GetLastUid( void ) const { return m_lastUid; }
UInt32 CMailInbox::GetSeenCount( void ) const { return (UInt32) m_seen.size(); }
UInt32 CMailInbox::GetHandledCount( void ) const { return m_handled; }

/*-------------------------------------------------------------------------*/

void
CMailInbox::SetOnline( bool online, const char* description, bool tellAlways )
{GUCEF_TRACE;

    if ( m_online == online && !tellAlways )
        return;
    m_online = online;
    MULTIPLAY::CMatchProviderState state;
    state.online = online;
    state.description = description;
    NotifyObservers( StateEvent, &state );
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::CheckNow( void )
{GUCEF_TRACE;

    m_checkRequested = true;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Disconnect( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_imap )
    {
        COM::CImapClient* client = m_imap;
        m_imap = GUCEF_NULL;
        UnsubscribeFrom( client );
        client->Disconnect();
        m_retired.push_back( client );
        m_retiredIsImap.push_back( true );
    }
    if ( GUCEF_NULL != m_pop3 )
    {
        COM::CPop3Client* client = m_pop3;
        m_pop3 = GUCEF_NULL;
        UnsubscribeFrom( client );
        client->Disconnect();
        m_retired.push_back( client );
        m_retiredIsImap.push_back( false );
    }
    m_step = STEP_NONE;
    m_requestId = 0;
    m_uids.clear();
    m_uidIndex = 0;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ScheduleReconnect( Int64 nowMs )
{GUCEF_TRACE;

    m_backoffMs = 0 == m_backoffMs ? m_settings->reconnectInitialMs : ( m_backoffMs * 2 > m_settings->reconnectMaxMs ? m_settings->reconnectMaxMs : m_backoffMs * 2 );
    m_nextConnectAt = nowMs + m_backoffMs;
    m_checkRequested = true;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Update( Int64 nowMs )
{GUCEF_TRACE;

    m_nowMs = nowMs;
    if ( !m_started || GUCEF_NULL == m_settings )
        return;

    // clients that were left are deleted here, not inside one of their own events
    for ( size_t i=0; i<m_retired.size(); ++i )
    {
        if ( m_retiredIsImap[ i ] )
            delete static_cast< COM::CImapClient* >( m_retired[ i ] );
        else
            delete static_cast< COM::CPop3Client* >( m_retired[ i ] );
    }
    m_retired.clear();
    m_retiredIsImap.clear();

    if ( m_settings->useImap )
    {
        if ( GUCEF_NULL == m_imap )
        {
            if ( nowMs >= m_nextConnectAt )
                Connect( nowMs );
            return;
        }
        if ( !m_online )
            return;
        if ( STEP_IDLE == m_step && ( m_checkRequested || nowMs >= m_nextPollAt ) )
        {
            // the mailbox is asked now and then also while the server is waited for: a notice that never came costs no more than the interval
            m_checkRequested = false;
            m_step = STEP_ENDING_IDLE;
            m_imap->IdleDone();
        }
        else
        if ( STEP_NONE == m_step && ( m_checkRequested || nowMs >= m_nextPollAt ) )
            ImapStartCycle();
    }
    else
    {
        if ( GUCEF_NULL == m_pop3 && nowMs >= m_nextConnectAt && ( m_checkRequested || nowMs >= m_nextPollAt ) )
            Connect( nowMs );
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Connect( Int64 nowMs )
{GUCEF_TRACE;

    m_step = STEP_NONE;
    m_requestId = 0;
    if ( m_settings->useImap )
    {
        m_imapSettings = m_settings->imap;
        if ( 0 == m_imapSettings.idleRenewInMs || m_imapSettings.idleRenewInMs > g_maxIdleRenewMs )
            m_imapSettings.idleRenewInMs = g_maxIdleRenewMs;
        m_imap = GUCEF_NEW COM::CImapClient();
        TEventCallback readyCallback( this, &CMailInbox::OnImapReady );
        SubscribeTo( m_imap, COM::CMailClientBase::ReadyEvent, readyCallback );
        TEventCallback commandCallback( this, &CMailInbox::OnImapCommand );
        SubscribeTo( m_imap, COM::CImapClient::CommandCompletedEvent, commandCallback );
        TEventCallback idleCallback( this, &CMailInbox::OnImapIdleUpdate );
        SubscribeTo( m_imap, COM::CImapClient::IdleUpdateEvent, idleCallback );
        TEventCallback errorCallback( this, &CMailInbox::OnClientError );
        SubscribeTo( m_imap, COM::CMailClientBase::ErrorEvent, errorCallback );
        TEventCallback disconnectedCallback( this, &CMailInbox::OnClientDisconnected );
        SubscribeTo( m_imap, COM::CMailClientBase::DisconnectedEvent, disconnectedCallback );
        if ( GUCEF_NULL != m_credentials )
            m_imap->SetCredentialProvider( m_credentials );
        if ( !m_imap->Connect( m_imapSettings ) )
        {
            Disconnect();
            ScheduleReconnect( nowMs );
        }
    }
    else
    {
        m_pop3Settings = m_settings->pop3;
        m_pop3 = GUCEF_NEW COM::CPop3Client();
        TEventCallback readyCallback( this, &CMailInbox::OnPop3Ready );
        SubscribeTo( m_pop3, COM::CMailClientBase::ReadyEvent, readyCallback );
        TEventCallback commandCallback( this, &CMailInbox::OnPop3Command );
        SubscribeTo( m_pop3, COM::CPop3Client::CommandCompletedEvent, commandCallback );
        TEventCallback errorCallback( this, &CMailInbox::OnClientError );
        SubscribeTo( m_pop3, COM::CMailClientBase::ErrorEvent, errorCallback );
        TEventCallback disconnectedCallback( this, &CMailInbox::OnClientDisconnected );
        SubscribeTo( m_pop3, COM::CMailClientBase::DisconnectedEvent, disconnectedCallback );
        if ( GUCEF_NULL != m_credentials )
            m_pop3->SetCredentialProvider( m_credentials );
        m_checkRequested = false;
        if ( !m_pop3->Connect( m_pop3Settings ) )
        {
            Disconnect();
            ScheduleReconnect( nowMs );
            m_nextPollAt = nowMs + m_settings->pollIntervalMs;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnClientError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COM::CMailErrorEventData* error = static_cast< const COM::CMailErrorEventData* >( eventData );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailInbox: The connection to the mailbox failed (class " << (Int32) error->errorClass << ")" );
    // every failure is told, also when the mailbox was not reachable before: it is what the player has to know
    SetOnline( false, COM::MAILERR_AUTH == error->errorClass ? "the login to the mailbox was refused" : "the mailbox cannot be reached", true );
    Disconnect();
    ScheduleReconnect( m_nowMs );
    if ( !m_settings->useImap )
        m_nextPollAt = m_nowMs + m_settings->pollIntervalMs;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a connection that ended without a word: the same as a failure. The ones we ended ourselves are no longer subscribed
    if ( m_started && ( ( GUCEF_NULL != m_imap && notifier == m_imap ) || ( GUCEF_NULL != m_pop3 && notifier == m_pop3 && STEP_NONE != m_step ) ) )
    {
        SetOnline( false, "the connection to the mailbox was lost" );
        Disconnect();
        ScheduleReconnect( m_nowMs );
        if ( !m_settings->useImap )
            m_nextPollAt = m_nowMs + m_settings->pollIntervalMs;
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnImapReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_backoffMs = 0;
    SetOnline( true, "connected" );
    m_supportsIdle = m_settings->useIdle && m_imap->HasCapability( "IDLE" );
    m_checkRequested = false;
    ImapStartCycle();
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapStartCycle( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imap )
        return;
    m_checkRequested = false;
    m_uids.clear();
    m_uidIndex = 0;
    if ( !m_supportsIdle )
    {
        m_step = STEP_NOOP;
        m_requestId = m_imap->Noop();
    }
    else
    {
        m_step = STEP_SELECT;
        m_requestId = m_imap->Select( m_settings->mailbox, false, m_uidValidity );
    }
    if ( 0 == m_requestId )
    {
        SetOnline( false, "the mailbox cannot be asked" );
        Disconnect();
        ScheduleReconnect( m_nowMs );
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnImapIdleUpdate( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COM::CImapIdleUpdate* update = static_cast< const COM::CImapIdleUpdate* >( eventData );
    if ( COM::CImapIdleUpdate::KIND_EXISTS == update->kind && STEP_IDLE == m_step && GUCEF_NULL != m_imap )
    {
        m_step = STEP_ENDING_IDLE;
        m_imap->IdleDone();
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapWait( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imap )
        return;
    m_nextPollAt = m_nowMs + m_settings->pollIntervalMs;
    SaveState();
    // mail that came while the last was handled is not announced to an IDLE that starts later: the mailbox is asked again until a round finds nothing
    if ( m_checkRequested || !m_uids.empty() || m_imap->GetSelectedMailbox().exists != m_existsAtSearch )
    {
        ImapStartCycle();
        return;
    }
    if ( m_supportsIdle )
    {
        m_step = STEP_IDLE;
        m_requestId = m_imap->Idle();
        if ( 0 == m_requestId )
        {
            m_supportsIdle = false;
            m_step = STEP_NONE;
        }
    }
    else
        m_step = STEP_NONE;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnImapCommand( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COM::CImapCommandResult* result = static_cast< const COM::CImapCommandResult* >( eventData );
    if ( GUCEF_NULL == m_imap || result->requestId != m_requestId )
        return;

    if ( STEP_ENDING_IDLE == m_step || STEP_IDLE == m_step )
    {
        // the IDLE ended: by us, or by the server
        ImapStartCycle();
        return;
    }
    if ( !result->ok )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailInbox: The mailbox refused a command in step " << (Int32) m_step << " (" << result->status << ", class " << (Int32) result->errorClass << ")" );
        // a failed copy, move or flag is not a failed mailbox: the message was handled and stays where it is
        if ( STEP_STORE == m_step || STEP_APPEND == m_step || STEP_UNFLAG_DELETED == m_step )
        {
            ImapNextUid();
            return;
        }
        if ( STEP_COPY == m_step || STEP_MOVE == m_step || STEP_FLAG_DELETED == m_step )
        {
            ImapMoveFailed();
            ImapNextUid();
            return;
        }
        if ( STEP_EXPUNGE_UID == m_step )
        {
            // the copy exists and the original is flagged: the flag goes, so that the original is the mail it was
            ImapMoveFailed();
            CORE::CString::StringVector flags;
            flags.push_back( "\\Deleted" );
            m_step = STEP_UNFLAG_DELETED;
            m_requestId = m_imap->Store( CORE::UInt32ToString( m_currentUid ), COM::IMAPSTORE_REMOVE, flags, true, true );
            if ( 0 == m_requestId )
                ImapNextUid();
            return;
        }
        SetOnline( false, "the mailbox refused a command" );
        Disconnect();
        ScheduleReconnect( m_nowMs );
        return;
    }

    switch ( m_step )
    {
        case STEP_NOOP:
        {
            m_step = STEP_SELECT;
            m_requestId = m_imap->Select( m_settings->mailbox, false, m_uidValidity );
            break;
        }
        case STEP_SELECT:
        {
            if ( result->uidValidityChanged || ( 0 != m_uidValidity && m_uidValidity != result->mailbox.uidValidity ) )
            {
                // the UIDs mean nothing any more; the set of message ids, which is what de-duplicates, stays
                GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailInbox: The mailbox is a new one, the UID position starts again" );
                m_lastUid = 0;
            }
            m_uidValidity = result->mailbox.uidValidity;
            COM::CImapSearchCriteria criteria;
            criteria.AddHeader( CMailMatchCodec::HeaderMatch, CORE::CString::Empty );
            if ( m_lastUid > 0 )
                criteria.AddUidRange( m_lastUid + 1, 0 );
            m_step = STEP_SEARCH;
            m_existsAtSearch = m_imap->GetSelectedMailbox().exists;
            m_requestId = m_imap->Search( criteria, true );
            break;
        }
        case STEP_SEARCH:
        {
            m_uids.clear();
            for ( size_t i=0; i<result->numbers.size(); ++i )
                if ( result->numbers[ i ] > m_lastUid )
                    m_uids.push_back( result->numbers[ i ] );
            std::sort( m_uids.begin(), m_uids.end() );
            if ( m_uids.size() > g_maxMessagesPerCycle )
                m_uids.resize( g_maxMessagesPerCycle );
            m_uidIndex = 0;
            ImapNextUid();
            break;
        }
        case STEP_FETCH_HEADER:
        {
            if ( result->fetched.empty() )
            {
                ImapNextUid();
                break;
            }
            ImapHandleHeader( result->fetched[ 0 ] );
            break;
        }
        case STEP_FETCH_BODY:
        {
            if ( result->fetched.empty() )
            {
                ImapNextUid();
                break;
            }
            ImapHandleBody( result->fetched[ 0 ] );
            break;
        }
        case STEP_STORE:
        {
            ImapProcessedOrNext();
            break;
        }
        case STEP_APPEND:
        case STEP_MOVE:
        case STEP_EXPUNGE_UID:
        case STEP_UNFLAG_DELETED:
        {
            ImapNextUid();
            break;
        }
        case STEP_COPY:
        {
            if ( !m_moveDeletes )
            {
                ImapNextUid();
                break;
            }
            // the copy is there: now exactly this message is flagged and expunged
            CORE::CString::StringVector flags;
            flags.push_back( "\\Deleted" );
            m_step = STEP_FLAG_DELETED;
            m_requestId = m_imap->Store( CORE::UInt32ToString( m_currentUid ), COM::IMAPSTORE_ADD, flags, true, true );
            if ( 0 == m_requestId )
            {
                ImapMoveFailed();
                ImapNextUid();
            }
            break;
        }
        case STEP_FLAG_DELETED:
        {
            m_step = STEP_EXPUNGE_UID;
            m_requestId = m_imap->UidExpunge( CORE::UInt32ToString( m_currentUid ) );
            if ( 0 == m_requestId )
            {
                // refused before it was sent: the flag is taken off again
                ImapMoveFailed();
                CORE::CString::StringVector flags;
                flags.push_back( "\\Deleted" );
                m_step = STEP_UNFLAG_DELETED;
                m_requestId = m_imap->Store( CORE::UInt32ToString( m_currentUid ), COM::IMAPSTORE_REMOVE, flags, true, true );
                if ( 0 == m_requestId )
                    ImapNextUid();
            }
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapNextUid( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imap )
        return;
    m_currentRaw.Clear();
    m_currentInteresting = false;
    if ( m_uidIndex >= m_uids.size() )
    {
        ImapWait();
        return;
    }
    m_currentUid = m_uids[ m_uidIndex++ ];
    COM::CImapFetchSpec spec;
    spec.flags = true;
    spec.size = true;
    spec.sections.push_back( "HEADER" );
    m_step = STEP_FETCH_HEADER;
    m_requestId = m_imap->Fetch( CORE::UInt32ToString( m_currentUid ), spec, true );
    if ( 0 == m_requestId )
    {
        SetOnline( false, "the mailbox cannot be asked" );
        Disconnect();
        ScheduleReconnect( m_nowMs );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::DecideOnHeaders( const COM::CMailHeaders& headers, UInt32 size, CMailIncoming& incoming, bool& markAsSeen )
{GUCEF_TRACE;

    markAsSeen = false;
    CMailMatchCodec::ReadHeaders( headers, incoming );
    if ( !incoming.valid || incoming.automatic )
        return false;
    if ( !m_settings->mailAddress.IsNULLOrEmpty() && incoming.fromAddress == m_settings->mailAddress.Lowercase() )
        return false;
    if ( !incoming.messageId.IsNULLOrEmpty() && IsSeen( incoming.messageId ) )
        return false;
    if ( size > m_settings->maxMessageBytes )
    {
        CMailInboxRefusal refusal;
        refusal.matchId = incoming.matchId;
        refusal.revision = incoming.revision;
        refusal.kind = incoming.kind;
        refusal.reason = "The message is larger than the limit and was not read";
        NotifyObservers( RefusedEvent, &refusal );
        if ( !incoming.messageId.IsNULLOrEmpty() )
            AddSeen( incoming.messageId );
        markAsSeen = true;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapHandleHeader( const COM::CImapFetchedItem& item )
{GUCEF_TRACE;

    const COM::CImapFetchSection* section = item.FindSection( "HEADER" );
    COM::CMailHeaders headers;
    UInt32 bodyOffset = 0;
    bool parsed = false;
    if ( GUCEF_NULL != section && !section->isNil && section->data.GetDataSize() > 0 )
    {
        COM::CMimeParser parser;
        parsed = parser.ParseHeaders( section->data.GetConstBufferPtr(), section->data.GetDataSize(), headers, bodyOffset );
    }
    m_currentSize = item.hasSize ? item.size : 0;
    bool markAsSeen = false;
    CMailIncoming incoming;
    if ( parsed && DecideOnHeaders( headers, m_currentSize, incoming, markAsSeen ) )
    {
        m_currentIncoming = incoming;
        m_currentInteresting = true;
        COM::CImapFetchSpec spec;
        spec.flags = true;
        spec.size = true;
        spec.sections.push_back( "" );
        m_step = STEP_FETCH_BODY;
        m_requestId = m_imap->Fetch( CORE::UInt32ToString( m_currentUid ), spec, true );
        if ( 0 == m_requestId )
        {
            SetOnline( false, "the mailbox cannot be asked" );
            Disconnect();
            ScheduleReconnect( m_nowMs );
        }
        return;
    }
    // not for us, or handled before, or too large: it is done with, and not touched
    if ( parsed && !incoming.messageId.IsNULLOrEmpty() )
        AddSeen( incoming.messageId );
    m_currentIncoming = incoming;
    ImapFinishMessage( markAsSeen, false );
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::HandleCompleteMessage( const CMailIncoming& incoming, const void* data, UInt32 size )
{GUCEF_TRACE;

    COM::CMimeLimits limits;
    limits.maxMessageSize = m_settings->maxMessageBytes;
    COM::CMimeParser parser( limits );
    CMailInboxMessage mail;
    CORE::CString error;
    if ( !parser.ParseMessage( data, size, mail.message, &error ) )
    {
        CMailInboxRefusal refusal;
        refusal.matchId = incoming.matchId;
        refusal.revision = incoming.revision;
        refusal.kind = incoming.kind;
        refusal.reason = "The message could not be read";
        NotifyObservers( RefusedEvent, &refusal );
        return false;
    }
    mail.incoming = incoming;
    mail.rawSize = size;
    ++m_handled;
    NotifyObservers( MessageEvent, &mail );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapHandleBody( const COM::CImapFetchedItem& item )
{GUCEF_TRACE;

    const COM::CImapFetchSection* section = item.FindSection( "" );
    bool handled = false;
    if ( GUCEF_NULL != section && !section->isNil && section->data.GetDataSize() > 0 )
    {
        m_currentRaw = section->data;
        if ( !m_currentIncoming.messageId.IsNULLOrEmpty() )
            AddSeen( m_currentIncoming.messageId );
        handled = HandleCompleteMessage( m_currentIncoming, m_currentRaw.GetConstBufferPtr(), m_currentRaw.GetDataSize() );
    }
    ImapFinishMessage( m_settings->markProcessedSeen, handled );
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapFinishMessage( bool markSeen, bool processed )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imap )
        return;
    if ( m_currentUid > m_lastUid )
        m_lastUid = m_currentUid;
    SaveState();
    m_actionPending = processed;
    if ( markSeen )
    {
        CORE::CString::StringVector flags;
        flags.push_back( "\\Seen" );
        m_step = STEP_STORE;
        m_requestId = m_imap->Store( CORE::UInt32ToString( m_currentUid ), COM::IMAPSTORE_ADD, flags, true, true );
        if ( 0 != m_requestId )
            return;
    }
    ImapProcessedOrNext();
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapProcessedOrNext( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_imap )
        return;
    if ( m_actionPending )
    {
        m_actionPending = false;
        if ( ImapStartProcessedAction() )
            return;
    }
    ImapNextUid();
}

/*-------------------------------------------------------------------------*/

bool
CMailInbox::ImapStartProcessedAction( void )
{GUCEF_TRACE;

    m_moveDeletes = false;
    if ( MAILPROCESSED_LEAVE == m_settings->processedAction || m_settings->processedFolder.IsNULLOrEmpty() )
        return false;

    if ( MAILPROCESSED_COPY_TO_FOLDER == m_settings->processedAction )
    {
        // a copy in the folder; the original stays
        if ( 0 == m_currentRaw.GetDataSize() )
            return false;
        CORE::CString::StringVector flags;
        flags.push_back( "\\Seen" );
        m_step = STEP_APPEND;
        m_requestId = m_imap->AppendRaw( m_settings->processedFolder, m_currentRaw.GetConstBufferPtr(), m_currentRaw.GetDataSize(), flags );
        return 0 != m_requestId;
    }

    const CORE::CString uid = CORE::UInt32ToString( m_currentUid );
    if ( m_imap->HasCapability( "MOVE" ) )
    {
        m_step = STEP_MOVE;
        m_requestId = m_imap->Move( uid, m_settings->processedFolder, true );
        if ( 0 != m_requestId )
            return true;
    }
    if ( m_imap->HasCapability( "UIDPLUS" ) )
    {
        // copy first; the original is flagged and expunged by its UID only after the copy is there
        m_moveDeletes = true;
    }
    else
    if ( !m_degradedNoticeGiven )
    {
        m_degradedNoticeGiven = true;
        Notice( "MOVE_NOT_POSSIBLE", "The mail server cannot move mail safely. Game mail is copied to the folder and also stays in the inbox. You can delete it there yourself." );
    }
    m_step = STEP_COPY;
    m_requestId = m_imap->Copy( uid, m_settings->processedFolder, true );
    return 0 != m_requestId;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::ImapMoveFailed( void )
{GUCEF_TRACE;

    m_moveDeletes = false;
    Notice( "MOVE_FAILED", "A game mail could not be moved out of the inbox. It stays in the inbox. Check that the folder for processed mail exists." );
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Notice( const char* code, const char* description )
{GUCEF_TRACE;

    CMailInboxNotice notice;
    notice.code = code;
    notice.description = description;
    NotifyObservers( NoticeEvent, &notice );
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnPop3Ready( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_backoffMs = 0;
    SetOnline( true, "connected" );
    m_popUids.clear();
    m_popSizes.clear();
    m_popIndex = 0;
    m_step = STEP_POP_UIDL;
    m_requestId = m_pop3->Uidl();
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Pop3EndSession( void )
{GUCEF_TRACE;

    SaveState();
    Disconnect();
    m_nextPollAt = m_nowMs + m_settings->pollIntervalMs;
    m_checkRequested = false;
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::Pop3NextMessage( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pop3 )
        return;
    while ( m_popIndex < m_popUids.size() && ( IsSeen( "pop3:" + m_popUids[ m_popIndex ] ) || m_popIndex >= g_maxMessagesPerCycle * 10 ) )
        ++m_popIndex;
    if ( m_popIndex >= m_popUids.size() )
    {
        Pop3EndSession();
        return;
    }
    m_popCurrentUid = m_popUids[ m_popIndex ];
    m_currentSize = m_popIndex < m_popSizes.size() ? m_popSizes[ m_popIndex ] : 0;
    if ( m_pop3->HasCapability( "TOP" ) )
    {
        m_step = STEP_POP_TOP;
        m_requestId = m_pop3->Top( (UInt32) m_popIndex + 1, 0 );
    }
    else
    {
        m_step = STEP_POP_RETR;
        m_currentInteresting = false;
        m_requestId = m_pop3->Retr( (UInt32) m_popIndex + 1 );
    }
}

/*-------------------------------------------------------------------------*/

void
CMailInbox::OnPop3Command( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COM::CPop3CommandResult* result = static_cast< const COM::CPop3CommandResult* >( eventData );
    if ( GUCEF_NULL == m_pop3 || result->requestId != m_requestId )
        return;
    if ( !result->ok && STEP_POP_DELE == m_step )
    {
        // the message was processed and is remembered; that it could not be deleted is no reason to drop the session
        Notice( "DELETE_FAILED", "A game mail could not be deleted from the mailbox. It stays there." );
        ++m_popIndex;
        Pop3NextMessage();
        return;
    }
    if ( !result->ok )
    {
        SetOnline( false, "the mailbox refused a command" );
        Disconnect();
        ScheduleReconnect( m_nowMs );
        m_nextPollAt = m_nowMs + m_settings->pollIntervalMs;
        return;
    }
    switch ( m_step )
    {
        case STEP_POP_UIDL:
        {
            m_popUids.clear();
            for ( size_t i=0; i<result->entries.size(); ++i )
            {
                // numbers are 1 to n in order; a server that says otherwise is not listened to
                if ( result->entries[ i ].number != (UInt32) i + 1 || result->entries[ i ].uid.IsNULLOrEmpty() || result->entries[ i ].uid.Length() > 70 )
                {
                    m_popUids.clear();
                    break;
                }
                m_popUids.push_back( result->entries[ i ].uid );
            }
            m_currentInteresting = false;
            m_step = STEP_POP_LIST;
            m_requestId = m_pop3->List();
            break;
        }
        case STEP_POP_LIST:
        {
            m_popSizes.assign( m_popUids.size(), 0 );
            for ( size_t i=0; i<result->entries.size(); ++i )
                if ( result->entries[ i ].number >= 1 && result->entries[ i ].number <= m_popSizes.size() )
                    m_popSizes[ result->entries[ i ].number - 1 ] = result->entries[ i ].size;
            m_popIndex = 0;
            Pop3NextMessage();
            break;
        }
        case STEP_POP_TOP:
        {
            COM::CMailHeaders headers;
            UInt32 bodyOffset = 0;
            COM::CMimeParser parser;
            bool markAsSeen = false;
            CMailIncoming incoming;
            if ( parser.ParseHeaders( result->data.GetConstBufferPtr(), result->data.GetDataSize(), headers, bodyOffset ) && DecideOnHeaders( headers, m_currentSize, incoming, markAsSeen ) )
            {
                m_currentIncoming = incoming;
                m_currentInteresting = true;
                m_step = STEP_POP_RETR;
                m_requestId = m_pop3->Retr( (UInt32) m_popIndex + 1 );
                break;
            }
            AddSeen( "pop3:" + m_popCurrentUid );
            if ( !incoming.messageId.IsNULLOrEmpty() )
                AddSeen( incoming.messageId );
            ++m_popIndex;
            Pop3NextMessage();
            break;
        }
        case STEP_POP_RETR:
        {
            COM::CMailHeaders headers;
            UInt32 bodyOffset = 0;
            COM::CMimeParser parser;
            CMailIncoming incoming = m_currentIncoming;
            bool proceed = m_currentInteresting;
            if ( !proceed )
            {
                // no TOP: the headers are in the whole message
                bool markAsSeen = false;
                proceed = parser.ParseHeaders( result->data.GetConstBufferPtr(), result->data.GetDataSize(), headers, bodyOffset ) && DecideOnHeaders( headers, result->data.GetDataSize(), incoming, markAsSeen );
            }
            bool processed = false;
            if ( proceed )
            {
                if ( !incoming.messageId.IsNULLOrEmpty() )
                    AddSeen( incoming.messageId );
                processed = HandleCompleteMessage( incoming, result->data.GetConstBufferPtr(), result->data.GetDataSize() );
            }
            AddSeen( "pop3:" + m_popCurrentUid );
            m_currentInteresting = false;
            SaveState();
            if ( processed && m_settings->pop3DeleteAfterProcessing )
            {
                // only this message, only after it was processed; the deletion is final when the session ends with QUIT
                m_step = STEP_POP_DELE;
                m_requestId = m_pop3->Dele( (UInt32) m_popIndex + 1 );
                if ( 0 != m_requestId )
                    break;
            }
            ++m_popIndex;
            Pop3NextMessage();
            break;
        }
        case STEP_POP_DELE:
        {
            ++m_popIndex;
            Pop3NextMessage();
            break;
        }
        default:
            break;
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace MAIL */
}; /* namespace MULTIPLAYPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
