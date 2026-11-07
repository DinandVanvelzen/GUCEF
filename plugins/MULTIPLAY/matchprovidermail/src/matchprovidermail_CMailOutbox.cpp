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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILFILES_H
#include "matchprovidermail_CMailFiles.h"
#define GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILFILES_H
#endif /* GUCEF_MULTIPLAYPLUGIN_MAIL_CMAILFILES_H ? */

#include "matchprovidermail_CMailOutbox.h"

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

const CORE::CEvent CMailOutbox::DeliveryChangedEvent = "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailOutbox::DeliveryChangedEvent";

static const UInt32 g_maxItemFileBytes = 8 * 1024 * 1024;
static const UInt32 g_maxRecipients = 64;
static const UInt32 g_maxAddressBytes = 320;
static const char g_itemMagic[ 4 ] = { 'G', 'M', 'O', 'B' };
static const UInt8 g_itemVersion = 1;
static const char* const g_itemExtension = ".rdxout";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
MailDeliveryStateToString( TMailDeliveryState state )
{GUCEF_TRACE;

    switch ( state )
    {
        case MAILDELIVERY_QUEUED:       return "QUEUED";
        case MAILDELIVERY_SENT:         return "SENT";
        case MAILDELIVERY_DELIVERED:    return "DELIVERED";
        case MAILDELIVERY_FAILED:       return "FAILED";
        default:                        return "NONE";
    }
}

/*-------------------------------------------------------------------------*/

CMailDeliveryEvent::CMailDeliveryEvent( void )
    : CORE::CICloneable()
    , matchId()
    , revision( 0 )
    , kind( MAILKIND_TURN )
    , state( MAILDELIVERY_NONE )
    , description()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailDeliveryEvent::CMailDeliveryEvent( const CMailDeliveryEvent& src )
    : CORE::CICloneable( src )
    , matchId( src.matchId )
    , revision( src.revision )
    , kind( src.kind )
    , state( src.state )
    , description( src.description )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailDeliveryEvent::~CMailDeliveryEvent()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMailDeliveryEvent&
CMailDeliveryEvent::operator=( const CMailDeliveryEvent& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        matchId = src.matchId;
        revision = src.revision;
        kind = src.kind;
        state = src.state;
        description = src.description;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CMailDeliveryEvent::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CMailDeliveryEvent( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CMailDeliveryEvent::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CMailDeliveryEvent::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::MULTIPLAYPLUGIN::MAIL::CMailDeliveryEvent";
}

/*-------------------------------------------------------------------------*/

CMailOutboxItem::CMailOutboxItem( void )
    : id( 0 )
    , kind( MAILKIND_TURN )
    , matchId()
    , revision( 0 )
    , messageId()
    , envelopeFrom()
    , recipients()
    , awaiting()
    , packageSha256()
    , needsConfirmation( false )
    , state( MAILDELIVERY_QUEUED )
    , attempts( 0 )
    , createdAt( 0 )
    , nextAttemptAt( 0 )
    , lastSentAt( 0 )
    , message()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

static void
WriteStrings( MULTIPLAY::CMatchBinaryWriter& writer, const CORE::CString::StringVector& list )
{GUCEF_TRACE;

    writer.WriteUInt32( (UInt32) list.size() );
    for ( size_t i=0; i<list.size(); ++i )
        writer.WriteString( list[ i ] );
}

/*-------------------------------------------------------------------------*/

static bool
ReadStrings( MULTIPLAY::CMatchBinaryReader& reader, CORE::CString::StringVector& list )
{GUCEF_TRACE;

    list.clear();
    const UInt32 count = reader.ReadUInt32();
    if ( !reader.IsOk() || count > g_maxRecipients )
        return false;
    for ( UInt32 i=0; i<count; ++i )
    {
        CORE::CString text;
        if ( !reader.ReadString( text, g_maxAddressBytes ) )
            return false;
        list.push_back( text );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailOutboxItem::ToBytes( CORE::CDynamicBuffer& bytes ) const
{GUCEF_TRACE;

    bytes.Clear();
    MULTIPLAY::CMatchBinaryWriter writer( bytes );
    writer.WriteRaw( g_itemMagic, 4 );
    writer.WriteUInt8( g_itemVersion );
    writer.WriteUInt32( id );
    writer.WriteUInt8( (UInt8) kind );
    writer.WriteString( matchId );
    writer.WriteUInt32( revision );
    writer.WriteString( messageId );
    writer.WriteString( envelopeFrom );
    WriteStrings( writer, recipients );
    WriteStrings( writer, awaiting );
    writer.WriteString( packageSha256 );
    writer.WriteBool( needsConfirmation );
    writer.WriteUInt8( (UInt8) state );
    writer.WriteUInt32( attempts );
    writer.WriteInt64( createdAt );
    writer.WriteInt64( nextAttemptAt );
    writer.WriteInt64( lastSentAt );
    writer.WriteBytes( message.GetConstBufferPtr(), message.GetDataSize() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailOutboxItem::FromBytes( const void* data, UInt32 size )
{GUCEF_TRACE;

    if ( GUCEF_NULL == data || size < 8 || size > g_maxItemFileBytes )
        return false;
    MULTIPLAY::CMatchBinaryReader reader( data, size );
    char magic[ 4 ];
    if ( !reader.ReadRaw( magic, 4 ) || 0 != memcmp( magic, g_itemMagic, 4 ) || g_itemVersion != reader.ReadUInt8() )
        return false;
    CMailOutboxItem read;
    read.id = reader.ReadUInt32();
    const UInt8 kindValue = reader.ReadUInt8();
    if ( !reader.IsOk() || kindValue >= (UInt8) MAILKIND_UNKNOWN )
        return false;
    read.kind = (TMailMatchKind) kindValue;
    if ( !reader.ReadString( read.matchId, 36 ) || !MULTIPLAY::CMatchId::IsValid( read.matchId ) )
        return false;
    read.revision = reader.ReadUInt32();
    if ( !reader.ReadString( read.messageId, 998 ) || !reader.ReadString( read.envelopeFrom, g_maxAddressBytes ) )
        return false;
    if ( !ReadStrings( reader, read.recipients ) || !ReadStrings( reader, read.awaiting ) || read.recipients.empty() )
        return false;
    if ( !reader.ReadString( read.packageSha256, 64 ) )
        return false;
    read.needsConfirmation = reader.ReadBool();
    const UInt8 stateValue = reader.ReadUInt8();
    if ( !reader.IsOk() || stateValue > (UInt8) MAILDELIVERY_FAILED )
        return false;
    read.state = (TMailDeliveryState) stateValue;
    read.attempts = reader.ReadUInt32();
    read.createdAt = reader.ReadInt64();
    read.nextAttemptAt = reader.ReadInt64();
    read.lastSentAt = reader.ReadInt64();
    UInt32 messageSize = 0;
    const UInt8* message = reader.ReadBytesInPlace( messageSize, g_maxItemFileBytes );
    if ( !reader.IsAtEndAndOk() || GUCEF_NULL == message || 0 == messageSize )
        return false;
    read.message.Append( message, messageSize );
    *this = read;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::RegisterEvents( void )
{GUCEF_TRACE;

    DeliveryChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CMailOutbox::CMailOutbox( void )
    : CORE::CObservingNotifier()
    , m_settings( GUCEF_NULL )
    , m_credentials( GUCEF_NULL )
    , m_directory()
    , m_items()
    , m_nextId( 1 )
    , m_client( GUCEF_NULL )
    , m_clientReady( false )
    , m_clientFinished( false )
    , m_nowMs( 0 )
    , m_sessionBackoffMs( 0 )
    , m_nextSessionAt( 0 )
    , m_inFlight()
    , m_sessionItems()
    , m_failures()
    , m_retired()
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CMailOutbox::~CMailOutbox()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
    Shutdown();
    for ( size_t i=0; i<m_retired.size(); ++i )
        delete m_retired[ i ];
    m_retired.clear();
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::Configure( const CMailMatchProviderSettings* settings, COM::CIMailCredentialProvider* credentials, const CORE::CString& directory )
{GUCEF_TRACE;

    m_settings = settings;
    m_credentials = credentials;
    m_directory = directory;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMailOutbox::GetItemPath( UInt32 id ) const
{GUCEF_TRACE;

    CORE::CString number = CORE::UInt32ToString( id );
    while ( number.Length() < 8 )
        number = "0" + number;
    return CORE::CombinePath( m_directory, number + g_itemExtension );
}

/*-------------------------------------------------------------------------*/

bool
CMailOutbox::Load( UInt32* skipped )
{GUCEF_TRACE;

    if ( GUCEF_NULL != skipped )
        *skipped = 0;
    m_items.clear();
    if ( m_directory.IsNULLOrEmpty() )
        return false;
    CORE::CString::StringVector names;
    CMailFiles::ListFiles( m_directory, names );
    for ( size_t i=0; i<names.size(); ++i )
    {
        if ( !names[ i ].EndsWith( g_itemExtension ) )
            continue;
        CORE::CDynamicBuffer bytes;
        CMailOutboxItem item;
        if ( CMailFiles::ReadBounded( CORE::CombinePath( m_directory, names[ i ] ), g_maxItemFileBytes, bytes ) && item.FromBytes( bytes.GetConstBufferPtr(), bytes.GetDataSize() ) && 0 != item.id )
        {
            m_items[ item.id ] = item;
            if ( item.id >= m_nextId )
                m_nextId = item.id + 1;
        }
        else
        if ( GUCEF_NULL != skipped )
            ++(*skipped);
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMailOutbox::Persist( const CMailOutboxItem& item )
{GUCEF_TRACE;

    CORE::CDynamicBuffer bytes;
    return item.ToBytes( bytes ) && CMailFiles::WriteAtomic( GetItemPath( item.id ), bytes.GetConstBufferPtr(), bytes.GetDataSize() );
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::Remove( UInt32 id )
{GUCEF_TRACE;

    CORE::DeleteFile( GetItemPath( id ) );
    m_items.erase( id );
    m_failures.erase( id );
    m_sessionItems.erase( id );
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::RaiseState( const CMailOutboxItem& item, TMailDeliveryState state, const char* description )
{GUCEF_TRACE;

    CMailDeliveryEvent eventData;
    eventData.matchId = item.matchId;
    eventData.revision = item.revision;
    eventData.kind = item.kind;
    eventData.state = state;
    eventData.description = description;
    NotifyObservers( DeliveryChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

UInt32
CMailOutbox::Enqueue( CMailOutboxItem& item, Int64 nowMs )
{GUCEF_TRACE;

    if ( m_directory.IsNULLOrEmpty() || item.recipients.empty() || 0 == item.message.GetDataSize() )
        return 0;
    m_nowMs = nowMs;
    item.id = m_nextId++;
    item.createdAt = nowMs;
    item.nextAttemptAt = nowMs;
    item.state = MAILDELIVERY_QUEUED;
    item.attempts = 0;
    item.lastSentAt = 0;
    if ( item.needsConfirmation && item.awaiting.empty() )
        for ( size_t i=0; i<item.recipients.size(); ++i )
            item.awaiting.push_back( item.recipients[ i ].Lowercase() );
    if ( !Persist( item ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailOutbox: A mail could not be written to the outbox directory" );
        return 0;
    }
    m_items[ item.id ] = item;
    RaiseState( item, MAILDELIVERY_QUEUED, "in the outbox" );
    return item.id;
}

/*-------------------------------------------------------------------------*/

Int64
CMailOutbox::BackoffFor( UInt32 failures ) const
{GUCEF_TRACE;

    Int64 wait = m_settings->sendRetryInitialMs;
    for ( UInt32 i=1; i<failures && wait < (Int64) m_settings->sendRetryMaxMs; ++i )
        wait *= 2;
    return wait > (Int64) m_settings->sendRetryMaxMs ? (Int64) m_settings->sendRetryMaxMs : wait;
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::ItemFailedTemporarily( CMailOutboxItem& item, const char* description )
{GUCEF_TRACE;

    const UInt32 failures = ++m_failures[ item.id ];
    item.state = MAILDELIVERY_QUEUED;
    item.nextAttemptAt = m_nowMs + BackoffFor( failures );
    Persist( item );
    RaiseState( item, MAILDELIVERY_QUEUED, description );
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::Update( Int64 nowMs )
{GUCEF_TRACE;

    m_nowMs = nowMs;
    if ( GUCEF_NULL == m_settings )
        return;

    // sessions that are over can be deleted now, we are not in one of their events
    for ( size_t i=0; i<m_retired.size(); ++i )
        delete m_retired[ i ];
    m_retired.clear();

    // mails that nobody confirmed are sent again after the quiet period, to those who did not confirm
    TItemMap::iterator it = m_items.begin();
    while ( it != m_items.end() )
    {
        CMailOutboxItem& item = (*it).second;
        if ( MAILDELIVERY_SENT == item.state && item.needsConfirmation && !item.awaiting.empty() && item.attempts < m_settings->outboxMaxSendAttempts &&
             nowMs >= item.lastSentAt + (Int64) m_settings->outboxResendIntervalMs )
        {
            item.state = MAILDELIVERY_QUEUED;
            item.recipients = item.awaiting;
            item.nextAttemptAt = nowMs;
            Persist( item );
        }
        ++it;
    }

    bool anyDue = false;
    for ( it = m_items.begin(); it != m_items.end(); ++it )
        if ( MAILDELIVERY_QUEUED == (*it).second.state && (*it).second.nextAttemptAt <= nowMs )
        {
            anyDue = true;
            break;
        }

    if ( GUCEF_NULL == m_client )
    {
        if ( anyDue && nowMs >= m_nextSessionAt )
            StartSession( nowMs );
    }
    else
    if ( m_clientReady )
    {
        SubmitDue();
        if ( m_inFlight.empty() && !anyDue )
            EndSession();
    }
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::StartSession( Int64 nowMs )
{GUCEF_TRACE;

    m_client = GUCEF_NEW COM::CSmtpClient();
    m_clientReady = false;
    m_clientFinished = false;
    m_sessionItems.clear();
    TEventCallback readyCallback( this, &CMailOutbox::OnReady );
    SubscribeTo( m_client, COM::CMailClientBase::ReadyEvent, readyCallback );
    TEventCallback sentCallback( this, &CMailOutbox::OnSendCompleted );
    SubscribeTo( m_client, COM::CSmtpClient::SendCompletedEvent, sentCallback );
    TEventCallback errorCallback( this, &CMailOutbox::OnSessionError );
    SubscribeTo( m_client, COM::CMailClientBase::ErrorEvent, errorCallback );
    TEventCallback disconnectedCallback( this, &CMailOutbox::OnSessionDisconnected );
    SubscribeTo( m_client, COM::CMailClientBase::DisconnectedEvent, disconnectedCallback );
    if ( GUCEF_NULL != m_credentials )
        m_client->SetCredentialProvider( m_credentials );

    // every mail that is due counts as a try, also when the server cannot be reached
    for ( TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it )
        if ( MAILDELIVERY_QUEUED == (*it).second.state && (*it).second.nextAttemptAt <= nowMs )
        {
            ++(*it).second.attempts;
            m_sessionItems.insert( (*it).first );
            Persist( (*it).second );
        }

    COM::CSmtpClientSettings settings( m_settings->smtp );
    if ( !m_client->Connect( settings ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "MailOutbox: The connection to the mail server could not be started, the settings are not usable" );
        FailInFlight( nowMs, "the mail server cannot be reached" );
        EndSession();
    }
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::EndSession( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client )
        return;
    COM::CSmtpClient* client = m_client;
    m_client = GUCEF_NULL;
    m_clientReady = false;
    UnsubscribeFrom( client );
    m_inFlight.clear();
    m_sessionItems.clear();
    client->Disconnect();
    m_retired.push_back( client );
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::SubmitDue( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_client || !m_clientReady )
        return;
    for ( TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it )
    {
        CMailOutboxItem& item = (*it).second;
        if ( MAILDELIVERY_QUEUED != item.state || item.nextAttemptAt > m_nowMs )
            continue;
        bool inFlight = false;
        for ( GUCEF::map< UInt32, UInt32 >::const_iterator f = m_inFlight.begin(); f != m_inFlight.end(); ++f )
            inFlight = inFlight || (*f).second == item.id;
        if ( inFlight )
            continue;
        if ( m_sessionItems.insert( item.id ).second )
        {
            ++item.attempts;
            Persist( item );
        }
        COM::CMailAddress::TMailAddressVector recipients;
        for ( size_t r=0; r<item.recipients.size(); ++r )
            recipients.push_back( COM::CMailAddress( item.recipients[ r ] ) );
        CORE::CString error;
        const UInt32 requestId = m_client->SendRaw( COM::CMailAddress( item.envelopeFrom ), recipients, item.message.GetConstBufferPtr(), item.message.GetDataSize(), &error );
        if ( 0 == requestId )
        {
            item.state = MAILDELIVERY_FAILED;
            Persist( item );
            RaiseState( item, MAILDELIVERY_FAILED, "the mail was refused before it was sent" );
            continue;
        }
        m_inFlight[ requestId ] = item.id;
    }
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    m_clientReady = true;
    m_sessionBackoffMs = 0;
    SubmitDue();
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::OnSendCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COM::CSmtpSendResult* result = static_cast< const COM::CSmtpSendResult* >( eventData );
    GUCEF::map< UInt32, UInt32 >::iterator f = m_inFlight.find( result->requestId );
    if ( f == m_inFlight.end() )
        return;
    TItemMap::iterator it = m_items.find( (*f).second );
    m_inFlight.erase( f );
    if ( it == m_items.end() )
        return;
    CMailOutboxItem& item = (*it).second;

    if ( result->success )
    {
        // who was served, who is to be tried again, who will never get it
        CORE::CString::StringVector again;
        CORE::CString::StringVector refusedForGood;
        for ( size_t r=0; r<result->recipients.size(); ++r )
        {
            const COM::CSmtpRecipientResult& recipient = result->recipients[ r ];
            if ( recipient.accepted )
                continue;
            if ( COM::MAILERR_TEMPORARY == recipient.errorClass )
                again.push_back( recipient.address.Lowercase() );
            else
                refusedForGood.push_back( recipient.address.Lowercase() );
        }
        CORE::CString::StringVector awaiting;
        for ( size_t a=0; a<item.awaiting.size(); ++a )
        {
            bool gone = false;
            for ( size_t g=0; g<refusedForGood.size(); ++g )
                gone = gone || refusedForGood[ g ] == item.awaiting[ a ];
            if ( !gone )
                awaiting.push_back( item.awaiting[ a ] );
        }
        item.awaiting = awaiting;
        m_failures.erase( item.id );
        item.lastSentAt = m_nowMs;
        if ( !again.empty() )
        {
            item.recipients = again;
            ItemFailedTemporarily( item, "some recipients could not be served yet, the rest was" );
            return;
        }
        item.recipients = item.awaiting.empty() ? item.recipients : item.awaiting;
        if ( item.needsConfirmation && !item.awaiting.empty() )
        {
            item.state = MAILDELIVERY_SENT;
            Persist( item );
            RaiseState( item, MAILDELIVERY_SENT, refusedForGood.empty() ? "accepted by the mail server" : "accepted by the mail server for some of the recipients" );
        }
        else
        {
            const CMailOutboxItem done = item;
            Remove( item.id );
            RaiseState( done, MAILDELIVERY_SENT, "accepted by the mail server" );
        }
        return;
    }

    switch ( result->errorClass )
    {
        case COM::MAILERR_TEMPORARY:
        case COM::MAILERR_CONNECTION:
        case COM::MAILERR_TIMEOUT:
        {
            ItemFailedTemporarily( item, COM::MAILERR_TEMPORARY == result->errorClass ? "the mail server asked to try again later" : "the mail server could not be reached" );
            break;
        }
        case COM::MAILERR_AUTH:
        case COM::MAILERR_TLS:
        {
            ItemFailedTemporarily( item, "the login or the protection of the connection to the mail server failed" );
            break;
        }
        default:
        {
            item.state = MAILDELIVERY_FAILED;
            Persist( item );
            RaiseState( item, MAILDELIVERY_FAILED, "the mail server refused the mail" );
            break;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::FailInFlight( Int64 nowMs, const char* description )
{GUCEF_TRACE;

    for ( GUCEF::set< UInt32 >::const_iterator s = m_sessionItems.begin(); s != m_sessionItems.end(); ++s )
    {
        TItemMap::iterator it = m_items.find( *s );
        if ( it != m_items.end() && MAILDELIVERY_QUEUED == (*it).second.state )
            ItemFailedTemporarily( (*it).second, description );
    }
    m_inFlight.clear();
    m_sessionBackoffMs = 0 == m_sessionBackoffMs ? m_settings->sendRetryInitialMs : ( m_sessionBackoffMs * 2 > m_settings->sendRetryMaxMs ? m_settings->sendRetryMaxMs : m_sessionBackoffMs * 2 );
    m_nextSessionAt = nowMs + m_sessionBackoffMs;
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::OnSessionError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COM::CMailErrorEventData* error = static_cast< const COM::CMailErrorEventData* >( eventData );
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "MailOutbox: The session with the mail server failed (class " << (Int32) error->errorClass << ")" );
    FailInFlight( m_nowMs, COM::MAILERR_AUTH == error->errorClass ? "the login to the mail server was refused" : "the mail server cannot be reached" );
    EndSession();
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::OnSessionDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // a disconnect that nobody asked for: the mails that were on their way are tried again
    if ( GUCEF_NULL != m_client && notifier == m_client )
    {
        FailInFlight( m_nowMs, "the connection to the mail server was lost" );
        EndSession();
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CMailOutbox::Acknowledge( const CORE::CString& matchId, const CORE::CString& address, UInt32 upToRevision, bool includeInvites )
{GUCEF_TRACE;

    const CORE::CString lower = address.Lowercase();
    UInt32 ended = 0;
    GUCEF::vector< UInt32 > done;
    for ( TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it )
    {
        CMailOutboxItem& item = (*it).second;
        if ( item.matchId != matchId || !item.needsConfirmation )
            continue;
        const bool isTurn = MAILKIND_TURN == item.kind || MAILKIND_FINISH == item.kind;
        const bool isInvite = MAILKIND_INVITE == item.kind;
        if ( !( ( isTurn && item.revision <= upToRevision ) || ( isInvite && includeInvites ) ) )
            continue;
        CORE::CString::StringVector remaining;
        bool found = false;
        for ( size_t a=0; a<item.awaiting.size(); ++a )
        {
            if ( item.awaiting[ a ] == lower )
                found = true;
            else
                remaining.push_back( item.awaiting[ a ] );
        }
        if ( !found )
            continue;
        item.awaiting = remaining;
        if ( remaining.empty() )
            done.push_back( item.id );
        else
        {
            // those that remain are the ones to be served again
            CORE::CString::StringVector recipients;
            for ( size_t r=0; r<item.recipients.size(); ++r )
                if ( item.recipients[ r ].Lowercase() != lower )
                    recipients.push_back( item.recipients[ r ] );
            if ( !recipients.empty() )
                item.recipients = recipients;
            Persist( item );
        }
    }
    for ( size_t d=0; d<done.size(); ++d )
    {
        TItemMap::iterator it = m_items.find( done[ d ] );
        if ( it == m_items.end() )
            continue;
        const CMailOutboxItem finished = (*it).second;
        Remove( done[ d ] );
        RaiseState( finished, MAILDELIVERY_DELIVERED, "every recipient showed that it arrived" );
        ++ended;
    }
    return ended;
}

/*-------------------------------------------------------------------------*/

UInt32
CMailOutbox::ResendNow( const CORE::CString& matchId, UInt32 revision, Int64 nowMs )
{GUCEF_TRACE;

    m_nowMs = nowMs;
    UInt32 count = 0;
    for ( TItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it )
    {
        CMailOutboxItem& item = (*it).second;
        const bool carriesTurn = MAILKIND_TURN == item.kind || MAILKIND_FINISH == item.kind || MAILKIND_INVITE == item.kind;
        if ( item.matchId != matchId || item.revision != revision || !carriesTurn || item.awaiting.empty() )
            continue;
        item.state = MAILDELIVERY_QUEUED;
        item.recipients = item.awaiting;
        item.nextAttemptAt = nowMs;
        m_failures.erase( item.id );
        Persist( item );
        RaiseState( item, MAILDELIVERY_QUEUED, "sent again on request" );
        ++count;
    }
    return count;
}

/*-------------------------------------------------------------------------*/

TMailDeliveryState
CMailOutbox::GetState( const CORE::CString& matchId, UInt32 revision, UInt32* attempts ) const
{GUCEF_TRACE;

    const CMailOutboxItem* newest = GUCEF_NULL;
    for ( TItemMap::const_iterator it = m_items.begin(); it != m_items.end(); ++it )
    {
        const CMailOutboxItem& item = (*it).second;
        const bool carriesTurn = MAILKIND_TURN == item.kind || MAILKIND_FINISH == item.kind || MAILKIND_INVITE == item.kind;
        if ( item.matchId == matchId && item.revision == revision && carriesTurn && ( GUCEF_NULL == newest || item.id > newest->id ) )
            newest = &item;
    }
    if ( GUCEF_NULL == newest )
    {
        if ( GUCEF_NULL != attempts )
            *attempts = 0;
        return MAILDELIVERY_NONE;
    }
    if ( GUCEF_NULL != attempts )
        *attempts = newest->attempts;
    return newest->state;
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::RemoveMatch( const CORE::CString& matchId )
{GUCEF_TRACE;

    GUCEF::vector< UInt32 > ids;
    for ( TItemMap::const_iterator it = m_items.begin(); it != m_items.end(); ++it )
        if ( (*it).second.matchId == matchId )
            ids.push_back( (*it).first );
    for ( size_t i=0; i<ids.size(); ++i )
        Remove( ids[ i ] );
}

/*-------------------------------------------------------------------------*/

UInt32
CMailOutbox::RemoveUnconfirmed( const CORE::CString& matchId )
{GUCEF_TRACE;

    GUCEF::vector< UInt32 > ids;
    for ( TItemMap::const_iterator it = m_items.begin(); it != m_items.end(); ++it )
        if ( (*it).second.matchId == matchId && (*it).second.needsConfirmation )
            ids.push_back( (*it).first );
    for ( size_t i=0; i<ids.size(); ++i )
        Remove( ids[ i ] );
    return (UInt32) ids.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CMailOutbox::GetItemCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_items.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CMailOutbox::GetUnconfirmedCount( void ) const
{GUCEF_TRACE;

    UInt32 count = 0;
    for ( TItemMap::const_iterator it = m_items.begin(); it != m_items.end(); ++it )
        if ( (*it).second.needsConfirmation && !(*it).second.awaiting.empty() )
            ++count;
    return count;
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::GetItems( const CORE::CString& matchId, GUCEF::vector< CMailOutboxItem >& items ) const
{GUCEF_TRACE;

    items.clear();
    for ( TItemMap::const_iterator it = m_items.begin(); it != m_items.end(); ++it )
        if ( (*it).second.matchId == matchId )
            items.push_back( (*it).second );
}

/*-------------------------------------------------------------------------*/

void
CMailOutbox::Shutdown( void )
{GUCEF_TRACE;

    EndSession();
    for ( size_t i=0; i<m_retired.size(); ++i )
        delete m_retired[ i ];
    m_retired.clear();
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
