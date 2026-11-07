/*
 *  gucefCOM: GUCEF module providing communication implementations
 *  for standardized protocols
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

#include <string.h>
#include <algorithm>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

#ifndef GUCEF_COM_CIMAPVALUEPARSER_H
#include "gucefCOM_CImapValueParser.h"
#define GUCEF_COM_CIMAPVALUEPARSER_H
#endif /* GUCEF_COM_CIMAPVALUEPARSER_H ? */

#include "gucefCOM_CImapClient.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CImapClient::CommandCompletedEvent = "GUCEF::COM::CImapClient::CommandCompletedEvent";
const CORE::CEvent CImapClient::IdleStartedEvent = "GUCEF::COM::CImapClient::IdleStartedEvent";
const CORE::CEvent CImapClient::IdleUpdateEvent = "GUCEF::COM::CImapClient::IdleUpdateEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

inline UInt32
ByteLength( const CORE::CString& text )
{
    return text.ByteSize() > 0 ? text.ByteSize() - 1 : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
ReadUInt32( const CORE::CString& text )
{
    UInt64 value = 0;
    const char* p = text.C_String();
    for ( ; *p >= '0' && *p <= '9' && value < 0xFFFFFFFFull; ++p )
        value = value * 10 + (UInt64)( *p - '0' );
    return value > 0xFFFFFFFFull ? 0xFFFFFFFFu : (UInt32) value;
}

/*-------------------------------------------------------------------------*/

/** The first UID in the target mailbox of the arguments of COPYUID: "uidvalidity source-set target-set" */
UInt32
ReadCopyUid( const CORE::CString& codeArguments )
{
    CORE::CString::StringVector words = codeArguments.ParseElements( ' ', false );
    return words.size() >= 3 ? ReadUInt32( words[ 2 ] ) : 0;
}

/*-------------------------------------------------------------------------*/

bool
IsPlainTextOfStatusResponse( const CORE::CString& line )
{
    // the text of a status response is no place for a literal, a line that ends in {5} there is just text
    const char* p = line.C_String();
    const UInt32 size = ByteLength( line );
    if ( size > 0 && '+' == p[ 0 ] )
        return true;
    UInt32 i = 0;
    while ( i < size && ' ' != p[ i ] )
        ++i;
    while ( i < size && ' ' == p[ i ] )
        ++i;
    UInt32 start = i;
    while ( i < size && ' ' != p[ i ] )
        ++i;
    const CORE::CString word( p + start, i - start );
    if ( size > 0 && '*' == p[ 0 ] )
        return word.Equals( "OK", false ) || word.Equals( "NO", false ) || word.Equals( "BAD", false ) || word.Equals( "BYE", false ) || word.Equals( "PREAUTH", false );
    return word.Equals( "OK", false ) || word.Equals( "NO", false ) || word.Equals( "BAD", false );
}

/*-------------------------------------------------------------------------*/

/** Runs the steps of FetchNewSync() on the events of a client */
class CImapFetchRunner : public CORE::CObserver
{
    public:

    bool done;

    CImapFetchRunner( CImapClient& client, const CImapFetchOptions& options, CImapFetchResult& result )
        : CORE::CObserver()
        , done( false )
        , m_client( client )
        , m_options( options )
        , m_result( result )
        , m_stage( STAGE_SELECT )
        , m_pendingId( 0 )
        , m_uids()
        , m_index( 0 )
        , m_startUid( 1 )
    {
        typedef CORE::CTEventHandlerFunctor< CImapFetchRunner > TCallback;
        TCallback ready( this, &CImapFetchRunner::OnReady );
        SubscribeTo( &client, CImapClient::ReadyEvent, ready );
        TCallback completed( this, &CImapFetchRunner::OnCommandCompleted );
        SubscribeTo( &client, CImapClient::CommandCompletedEvent, completed );
        TCallback error( this, &CImapFetchRunner::OnError );
        SubscribeTo( &client, CImapClient::ErrorEvent, error );
        TCallback disconnected( this, &CImapFetchRunner::OnDisconnected );
        SubscribeTo( &client, CImapClient::DisconnectedEvent, disconnected );
    }

    virtual ~CImapFetchRunner()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::COM::CImapFetchRunner";
        return classTypeName;
    }

    private:

    enum EStage { STAGE_SELECT, STAGE_SEARCH, STAGE_FETCH, STAGE_MARK, STAGE_DELETE, STAGE_EXPUNGE };

    void Finish( bool success )
    {
        m_result.success = success;
        done = true;
    }

    void FailWith( EMailErrorClass errorClass, const CORE::CString& description )
    {
        m_result.errorClass = errorClass;
        m_result.errorDescription = description;
        Finish( false );
    }

    void OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        m_stage = STAGE_SELECT;
        const bool readOnly = !m_options.markSeen && !m_options.deleteAfterFetch;
        m_pendingId = m_client.Select( m_options.mailbox.IsNULLOrEmpty() ? CORE::CString( "INBOX" ) : m_options.mailbox, readOnly, m_options.lastUidValidity );
        if ( 0 == m_pendingId )
            FailWith( MAILERR_LOCAL, "The mailbox cannot be selected" );
    }

    void FetchNext( void )
    {
        if ( m_index < m_uids.size() )
        {
            m_stage = STAGE_FETCH;
            CImapFetchSpec spec;
            spec.flags = true;
            spec.size = true;
            spec.sections.push_back( CORE::CString() );
            m_pendingId = m_client.Fetch( CORE::UInt32ToString( m_uids[ m_index ] ), spec, true );
            return;
        }
        if ( m_uids.empty() )
        {
            Finish( true );
            return;
        }
        if ( m_options.markSeen )
        {
            m_stage = STAGE_MARK;
            CORE::CString::StringVector flags;
            flags.push_back( "\\Seen" );
            m_pendingId = m_client.Store( UidSet(), IMAPSTORE_ADD, flags, true, true );
            return;
        }
        StartDelete();
    }

    void StartDelete( void )
    {
        if ( m_options.deleteAfterFetch )
        {
            m_stage = STAGE_DELETE;
            CORE::CString::StringVector flags;
            flags.push_back( "\\Deleted" );
            m_pendingId = m_client.Store( UidSet(), IMAPSTORE_ADD, flags, true, true );
            return;
        }
        Finish( true );
    }

    CORE::CString UidSet( void ) const
    {
        // the messages that were fetched, as a list of UIDs
        CORE::CString set;
        for ( size_t i=0; i<m_result.messages.size(); ++i )
        {
            if ( !m_result.messages[ i ].parsed )
                continue;
            if ( !set.IsNULLOrEmpty() )
                set += ",";
            set += CORE::UInt32ToString( m_result.messages[ i ].uid );
        }
        return set.IsNULLOrEmpty() ? CORE::CString( "0" ) : set;
    }

    void OnCommandCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const CImapCommandResult* command = CORE::CICloneable::CloneableAs< CImapCommandResult >( eventData );
        if ( GUCEF_NULL == command || command->requestId != m_pendingId || done )
            return;
        if ( !command->ok )
        {
            FailWith( command->errorClass, "The server did not carry out a command: " + CEMailProtocolLog::RedactServerLine( command->text, false ) );
            return;
        }

        switch ( m_stage )
        {
            case STAGE_SELECT:
            {
                m_result.uidValidity = command->mailbox.uidValidity;
                m_result.uidNext = command->mailbox.uidNext;
                m_result.uidValidityChanged = command->uidValidityChanged;
                m_startUid = ( m_options.lastUidValidity != 0 && !command->uidValidityChanged ) ? m_options.lastUid + 1 : 1;
                m_stage = STAGE_SEARCH;
                CImapSearchCriteria criteria;
                criteria.AddUidRange( m_startUid, 0 );
                for ( size_t i=0; i<m_options.headerNames.size(); ++i )
                    criteria.AddHeader( m_options.headerNames[ i ], i < m_options.headerTexts.size() ? m_options.headerTexts[ i ] : CORE::CString() );
                m_pendingId = m_client.Search( criteria, true );
                if ( 0 == m_pendingId )
                    FailWith( MAILERR_LOCAL, "The search cannot be sent: " + m_client.GetLastRefusal() );
                return;
            }
            case STAGE_SEARCH:
            {
                // "UID n:*" always includes the last message, even when it is below n
                for ( size_t i=0; i<command->numbers.size(); ++i )
                    if ( command->numbers[ i ] >= m_startUid )
                        m_uids.push_back( command->numbers[ i ] );
                std::sort( m_uids.begin(), m_uids.end() );
                if ( m_options.maxMessages > 0 && m_uids.size() > m_options.maxMessages )
                    m_uids.resize( m_options.maxMessages );
                m_index = 0;
                FetchNext();
                return;
            }
            case STAGE_FETCH:
            {
                CImapFetchedMessage fetched;
                fetched.uid = m_uids[ m_index ];
                for ( size_t i=0; i<command->fetched.size(); ++i )
                {
                    const CImapFetchedItem& item = command->fetched[ i ];
                    if ( item.hasUid && item.uid != fetched.uid )
                        continue;
                    fetched.size = item.size;
                    fetched.flags = item.flags;
                    const CImapFetchSection* body = item.FindSection( "" );
                    if ( GUCEF_NULL != body )
                    {
                        CMimeParser parser( m_options.mimeLimits );
                        CORE::CString error;
                        fetched.parsed = parser.ParseMessage( body->data.GetConstBufferPtr(), body->data.GetDataSize(), fetched.message, &error );
                    }
                }
                if ( fetched.uid > m_result.highestUid )
                    m_result.highestUid = fetched.uid;
                m_result.messages.push_back( fetched );
                ++m_index;
                FetchNext();
                return;
            }
            case STAGE_MARK:
            {
                StartDelete();
                return;
            }
            case STAGE_DELETE:
            {
                if ( m_client.HasCapability( "UIDPLUS" ) )
                {
                    m_stage = STAGE_EXPUNGE;
                    m_pendingId = m_client.UidExpunge( UidSet() );
                    return;
                }
                // without UIDPLUS an EXPUNGE would remove messages that were not ours: they stay marked
                Finish( true );
                return;
            }
            case STAGE_EXPUNGE:
            {
                Finish( true );
                return;
            }
        }
    }

    void OnError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const CEMailErrorEventData* error = CORE::CICloneable::CloneableAs< CEMailErrorEventData >( eventData );
        if ( GUCEF_NULL != error )
        {
            m_result.errorClass = error->errorClass;
            m_result.errorDescription = error->description;
        }
        Finish( false );
    }

    void OnDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        if ( !done )
            FailWith( MAILERR_CONNECTION, "The connection was closed" );
    }

    private:

    CImapClient& m_client;
    const CImapFetchOptions& m_options;
    CImapFetchResult& m_result;
    EStage m_stage;
    UInt32 m_pendingId;
    GUCEF::vector< UInt32 > m_uids;
    size_t m_index;
    UInt32 m_startUid;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CImapClient::RegisterEvents( void )
{GUCEF_TRACE;

    CommandCompletedEvent.Initialize();
    IdleStartedEvent.Initialize();
    IdleUpdateEvent.Initialize();
    CEMailClientBase::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CImapClient::CImapClient( void )
    : CEMailClientBase( CORE::PulseGeneratorPtr() )
    , m_settings()
    , m_phase( PHASE_GREETING )
    , m_queue()
    , m_current( GUCEF_NULL )
    , m_tagCounter( 0 )
    , m_literalIndex( 0 )
    , m_awaitingContinuation( false )
    , m_assembly()
    , m_assemblyHasLiteral( false )
    , m_expectingLiteral( false )
    , m_responseCount( 0 )
    , m_capabilities()
    , m_selected()
    , m_mailboxSelected( false )
    , m_idling( false )
    , m_idleRenewing( false )
    , m_idleDoneSent( false )
    , m_idleEndWanted( false )
    , m_idleStartedAt( 0 )
    , m_loggedIn( false )
    , m_logoutSent( false )
    , m_lastRefusal()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CImapClient::CImapClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CEMailClientBase( pulseGenerator )
    , m_settings()
    , m_phase( PHASE_GREETING )
    , m_queue()
    , m_current( GUCEF_NULL )
    , m_tagCounter( 0 )
    , m_literalIndex( 0 )
    , m_awaitingContinuation( false )
    , m_assembly()
    , m_assemblyHasLiteral( false )
    , m_expectingLiteral( false )
    , m_responseCount( 0 )
    , m_capabilities()
    , m_selected()
    , m_mailboxSelected( false )
    , m_idling( false )
    , m_idleRenewing( false )
    , m_idleDoneSent( false )
    , m_idleEndWanted( false )
    , m_idleStartedAt( 0 )
    , m_loggedIn( false )
    , m_logoutSent( false )
    , m_lastRefusal()
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CImapClient::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();
    m_assembly.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
}

/*-------------------------------------------------------------------------*/

CImapClient::~CImapClient()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_queue.size(); ++i )
        GUCEF_DELETE m_queue[ i ];
    m_queue.clear();
    GUCEF_DELETE m_current;
    m_current = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImapClient::GetProtocolName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "IMAP";
    return name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImapClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CImapClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CImapClientSettings&
CImapClient::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

const CImapMailboxStatus&
CImapClient::GetSelectedMailbox( void ) const
{GUCEF_TRACE;

    return m_selected;
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::IsMailboxSelected( void ) const
{GUCEF_TRACE;

    return m_mailboxSelected;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CImapClient::GetLastRefusal( void ) const
{GUCEF_TRACE;

    return m_lastRefusal;
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::HasCapability( const CORE::CString& name ) const
{GUCEF_TRACE;

    return m_capabilities.find( name.Uppercase() ) != m_capabilities.end();
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::IsIdling( void ) const
{GUCEF_TRACE;

    return m_idling && !m_idleDoneSent;
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::Connect( const CImapClientSettings& settings )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED != GetState() )
        return false;
    m_settings = settings;
    m_phase = PHASE_GREETING;
    m_assembly.Clear();
    m_assemblyHasLiteral = false;
    m_expectingLiteral = false;
    m_awaitingContinuation = false;
    m_capabilities.clear();
    m_mailboxSelected = false;
    m_idling = false;
    m_idleRenewing = false;
    m_idleDoneSent = false;
    m_idleEndWanted = false;
    m_loggedIn = false;
    m_logoutSent = false;
    m_responseCount = 0;
    return BeginConnect( m_settings );
}

/*-------------------------------------------------------------------------*/

UInt32
CImapClient::Refuse( const char* reason )
{GUCEF_TRACE;

    m_lastRefusal = reason;
    return 0;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapClient::NextTag( void )
{GUCEF_TRACE;

    ++m_tagCounter;
    CORE::CString number = CORE::UInt32ToString( m_tagCounter );
    while ( number.Length() < 4 )
        number = "0" + number;
    return "A" + number;
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::IsValidFlag( const CORE::CString& flag ) const
{GUCEF_TRACE;

    const char* p = flag.C_String();
    const UInt32 size = ByteLength( flag );
    if ( 0 == size || size > 64 )
        return false;
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '$' == c || '-' == c || '_' == c || '.' == c || ( 0 == i && '\\' == c );
        if ( !ok )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CImapClient::FlagsToText( const CORE::CString::StringVector& flags )
{GUCEF_TRACE;

    CORE::CString text = "(";
    for ( size_t i=0; i<flags.size(); ++i )
    {
        if ( i > 0 )
            text += " ";
        text += flags[ i ];
    }
    return text + ")";
}

/*-------------------------------------------------------------------------*/

UInt32
CImapClient::Queue( EImapCommand command, const CImapCommandParts& parts, const CORE::CString& mailbox, UInt32 expectedUidValidity )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == GetState() )
        return Refuse( "The client is not connected" );

    SCommand* entry = GUCEF_NEW SCommand;
    entry->requestId = NextRequestId();
    entry->command = command;
    entry->internal = INTERNAL_NONE;
    entry->parts = parts;
    entry->expectedUidValidity = expectedUidValidity;
    entry->mailbox = mailbox;
    entry->silentUse = false;
    entry->result.requestId = entry->requestId;
    entry->result.command = command;
    const UInt32 id = entry->requestId;
    m_queue.push_back( entry );

    // a command ends the IDLE that is going on
    if ( m_idling && !m_idleDoneSent )
        SendDone( false );
    SendNext();
    return id;
}

/*-------------------------------------------------------------------------*/

void
CImapClient::QueueInternal( EInternal internal, const CImapCommandParts& parts )
{GUCEF_TRACE;

    SCommand* entry = GUCEF_NEW SCommand;
    entry->requestId = 0;
    entry->command = IMAPCMD_NOOP;
    entry->internal = internal;
    entry->parts = parts;
    entry->expectedUidValidity = 0;
    entry->silentUse = false;
    m_queue.insert( m_queue.begin(), entry );
    SendNext();
}

/*-------------------------------------------------------------------------*/

UInt32
CImapClient::Capability( void )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "CAPABILITY" );
    return Queue( IMAPCMD_CAPABILITY, parts );
}

UInt32
CImapClient::Noop( void )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "NOOP" );
    return Queue( IMAPCMD_NOOP, parts );
}

UInt32
CImapClient::List( const CORE::CString& reference, const CORE::CString& pattern )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "LIST " );
    if ( !parts.AddMailbox( reference ) )
        return Refuse( "The reference is not valid" );
    parts.AddWord( " " );
    if ( !parts.AddMailbox( pattern ) )
        return Refuse( "The pattern is not valid" );
    return Queue( IMAPCMD_LIST, parts );
}

UInt32
CImapClient::Select( const CORE::CString& mailbox, bool readOnly, UInt32 expectedUidValidity )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( readOnly ? "EXAMINE " : "SELECT " );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    return Queue( readOnly ? IMAPCMD_EXAMINE : IMAPCMD_SELECT, parts, mailbox, expectedUidValidity );
}

UInt32
CImapClient::Status( const CORE::CString& mailbox )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "STATUS " );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    parts.AddWord( " (MESSAGES RECENT UIDNEXT UIDVALIDITY UNSEEN)" );
    return Queue( IMAPCMD_STATUS, parts, mailbox );
}

UInt32
CImapClient::Search( const CImapSearchCriteria& criteria, bool useUid )
{GUCEF_TRACE;

    CImapCommandParts built;
    CORE::CString error;
    if ( !criteria.Build( built, error ) )
        return Refuse( "The search cannot be sent" );
    CImapCommandParts parts;
    parts.AddWord( useUid ? "UID SEARCH " : "SEARCH " );
    parts.Append( built );
    return Queue( IMAPCMD_SEARCH, parts );
}

UInt32
CImapClient::Fetch( const CORE::CString& sequenceSet, const CImapFetchSpec& spec, bool useUid )
{GUCEF_TRACE;

    if ( !CImapCommandParts::IsValidSequenceSet( sequenceSet ) )
        return Refuse( "The set of messages is not valid" );
    CORE::CString items;
    if ( !spec.Build( items ) )
        return Refuse( "The items of the fetch are not valid" );
    CImapCommandParts parts;
    parts.AddWord( ( ( useUid ? "UID FETCH " : "FETCH " ) + sequenceSet + " " + items ).C_String() );
    return Queue( IMAPCMD_FETCH, parts );
}

UInt32
CImapClient::Store( const CORE::CString& sequenceSet, EImapStoreMode mode, const CORE::CString::StringVector& flags, bool useUid, bool silent )
{GUCEF_TRACE;

    if ( !CImapCommandParts::IsValidSequenceSet( sequenceSet ) )
        return Refuse( "The set of messages is not valid" );
    for ( size_t i=0; i<flags.size(); ++i )
        if ( !IsValidFlag( flags[ i ] ) )
            return Refuse( "A flag is not valid" );
    const char* item = IMAPSTORE_ADD == mode ? "+FLAGS" : ( IMAPSTORE_REMOVE == mode ? "-FLAGS" : "FLAGS" );
    CImapCommandParts parts;
    parts.AddWord( ( ( useUid ? "UID STORE " : "STORE " ) + sequenceSet + " " + item + ( silent ? ".SILENT " : " " ) + FlagsToText( flags ) ).C_String() );
    return Queue( IMAPCMD_STORE, parts );
}

UInt32
CImapClient::Expunge( void )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "EXPUNGE" );
    return Queue( IMAPCMD_EXPUNGE, parts );
}

UInt32
CImapClient::UidExpunge( const CORE::CString& uidSet )
{GUCEF_TRACE;

    if ( !HasCapability( "UIDPLUS" ) )
        return Refuse( "The server does not offer UIDPLUS" );
    if ( !CImapCommandParts::IsValidSequenceSet( uidSet ) )
        return Refuse( "The set of messages is not valid" );
    CImapCommandParts parts;
    parts.AddWord( ( "UID EXPUNGE " + uidSet ).C_String() );
    return Queue( IMAPCMD_EXPUNGE, parts );
}

UInt32
CImapClient::Copy( const CORE::CString& sequenceSet, const CORE::CString& mailbox, bool useUid )
{GUCEF_TRACE;

    if ( !CImapCommandParts::IsValidSequenceSet( sequenceSet ) )
        return Refuse( "The set of messages is not valid" );
    CImapCommandParts parts;
    parts.AddWord( ( ( useUid ? "UID COPY " : "COPY " ) + sequenceSet + " " ).C_String() );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    return Queue( IMAPCMD_COPY, parts );
}

UInt32
CImapClient::Move( const CORE::CString& sequenceSet, const CORE::CString& mailbox, bool useUid )
{GUCEF_TRACE;

    if ( !HasCapability( "MOVE" ) )
        return Refuse( "The server does not offer MOVE" );
    if ( !CImapCommandParts::IsValidSequenceSet( sequenceSet ) )
        return Refuse( "The set of messages is not valid" );
    CImapCommandParts parts;
    parts.AddWord( ( ( useUid ? "UID MOVE " : "MOVE " ) + sequenceSet + " " ).C_String() );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    return Queue( IMAPCMD_MOVE, parts );
}

UInt32
CImapClient::Close( void )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "CLOSE" );
    return Queue( IMAPCMD_CLOSE, parts );
}

UInt32
CImapClient::CreateMailbox( const CORE::CString& mailbox )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "CREATE " );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    return Queue( IMAPCMD_CREATE, parts );
}

UInt32
CImapClient::DeleteMailbox( const CORE::CString& mailbox )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( "DELETE " );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    return Queue( IMAPCMD_DELETE, parts );
}

UInt32
CImapClient::AppendRaw( const CORE::CString& mailbox, const void* data, UInt32 size, const CORE::CString::StringVector& flags, const CORE::CString& internalDate )
{GUCEF_TRACE;

    if ( size > m_settings.maxMessageSize )
        return Refuse( "The message is larger than the settings allow" );
    for ( size_t i=0; i<flags.size(); ++i )
        if ( !IsValidFlag( flags[ i ] ) )
            return Refuse( "A flag is not valid" );
    CImapCommandParts parts;
    parts.AddWord( "APPEND " );
    if ( !parts.AddMailbox( mailbox ) )
        return Refuse( "The name of the mailbox is not valid" );
    if ( !flags.empty() )
        parts.AddWord( ( " " + FlagsToText( flags ) ).C_String() );
    if ( !internalDate.IsNULLOrEmpty() )
    {
        const char* d = internalDate.C_String();
        for ( UInt32 i=0; i<ByteLength( internalDate ); ++i )
        {
            const char c = d[ i ];
            if ( !( ( c >= '0' && c <= '9' ) || ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ' ' == c || ':' == c || '+' == c || '-' == c ) )
                return Refuse( "The date is not valid" );
        }
        parts.AddWord( ( " \"" + internalDate + "\"" ).C_String() );
    }
    parts.AddWord( " " );
    parts.AddLiteral( data, size );
    return Queue( IMAPCMD_APPEND, parts, mailbox );
}

UInt32
CImapClient::AppendMessage( const CORE::CString& mailbox, const CEMailMessage& message, const CORE::CString::StringVector& flags )
{GUCEF_TRACE;

    CMimeComposeOptions options;
    options.allowUtf8Addresses = true;
    options.maxSize = m_settings.maxMessageSize;
    CMimeComposer composer( options );
    CORE::CDynamicBuffer data;
    CORE::CString error;
    if ( !composer.Compose( message, data, error ) )
        return Refuse( "The message cannot be composed" );
    return AppendRaw( mailbox, data.GetConstBufferPtr(), data.GetDataSize(), flags );
}

UInt32
CImapClient::Idle( void )
{GUCEF_TRACE;

    if ( !HasCapability( "IDLE" ) )
        return Refuse( "The server does not offer IDLE" );
    if ( !m_mailboxSelected )
        return Refuse( "No mailbox is selected" );
    CImapCommandParts parts;
    parts.AddWord( "IDLE" );
    return Queue( IMAPCMD_IDLE, parts );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::IdleDone( void )
{GUCEF_TRACE;

    if ( m_idling && !m_idleDoneSent )
    {
        SendDone( false );
        return;
    }
    // the IDLE is on its way and the server has not confirmed it: it ends when the server has, else it would idle for ever
    if ( ( !m_idling || m_idleRenewing ) && IsIdleCommandPending() )
        m_idleEndWanted = true;
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::IsIdleCommandPending( void ) const
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_current && INTERNAL_NONE == m_current->internal && IMAPCMD_IDLE == m_current->command )
        return true;
    for ( size_t i=0; i<m_queue.size(); ++i )
        if ( INTERNAL_NONE == m_queue[ i ]->internal && IMAPCMD_IDLE == m_queue[ i ]->command )
            return true;
    return false;
}

/*-------------------------------------------------------------------------*/

void
CImapClient::SendDone( bool renewing )
{GUCEF_TRACE;

    m_idleDoneSent = true;
    m_idleRenewing = renewing;
    SetDeadline( m_settings.commandTimeoutInMs, "the end of IDLE" );
    SendCommandLine( "DONE" );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnTimerTick( UInt64 nowInMs )
{GUCEF_TRACE;

    if ( m_idling && !m_idleDoneSent && m_settings.idleRenewInMs > 0 && nowInMs - m_idleStartedAt >= m_settings.idleRenewInMs )
        SendDone( true );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::SendNext( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_current || m_queue.empty() || CLIENTSTATE_DISCONNECTED == GetState() )
        return;
    SCommand* next = m_queue.front();
    if ( PHASE_READY != m_phase && INTERNAL_NONE == next->internal )
        return;
    if ( PHASE_GREETING == m_phase )
        return;

    m_queue.erase( m_queue.begin() );
    m_current = next;
    m_current->tag = NextTag();
    m_responseCount = 0;
    SendFirstPart( m_current );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::SendFirstPart( SCommand* command )
{GUCEF_TRACE;

    m_literalIndex = 0;
    m_awaitingContinuation = false;
    if ( IMAPCMD_IDLE != command->command || INTERNAL_NONE != command->internal )
        SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
    else
        SetDeadline( m_settings.commandTimeoutInMs, "the server to start IDLE" );

    const CORE::CString line = command->tag + " " + command->parts.texts[ 0 ];
    if ( command->parts.HasLiterals() )
    {
        m_awaitingContinuation = true;
        GetLog().LogClientLine( line, false );
        GetTransport().SendLine( line );
        return;
    }
    SendCommandLine( line, false );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::SendLiteralAndContinue( void )
{GUCEF_TRACE;

    SCommand* command = m_current;
    const CORE::CDynamicBuffer& literal = command->parts.literals[ m_literalIndex ];
    SendDataBlock( literal.GetConstBufferPtr(), literal.GetDataSize(), "literal" );
    ++m_literalIndex;
    const CORE::CString& next = command->parts.texts[ m_literalIndex ];
    if ( m_literalIndex < command->parts.literals.size() )
    {
        // the next literal is announced at the end of the text that follows this one
        GetTransport().SendLine( next );
        return;
    }
    m_awaitingContinuation = false;
    GetTransport().SendLine( next );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnConnectionEstablished( void )
{GUCEF_TRACE;

    m_phase = PHASE_GREETING;
    m_assembly.Clear();
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::IsServerLineLoggable( const CORE::CString& line ) const
{GUCEF_TRACE;

    // the content of messages (and of mailboxes: subjects, addresses) stays out of the log
    if ( m_assembly.GetDataSize() > 0 )
        return false;
    const char* p = line.C_String();
    const UInt32 size = ByteLength( line );
    if ( size > 2 && '*' == p[ 0 ] && ' ' == p[ 1 ] )
    {
        UInt32 i = 2;
        while ( i < size && p[ i ] >= '0' && p[ i ] <= '9' )
            ++i;
        if ( i > 2 && i + 6 <= size && 0 == strncmp( p + i, " FETCH", 6 ) )
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnServerLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( m_expectingLiteral )
    {
        Fail( MAILERR_PROTOCOL, "The server sent a line where a literal was announced" );
        return;
    }

    const bool continuesResponse = m_assembly.GetDataSize() > 0;
    if ( m_assembly.GetDataSize() + ByteLength( line ) > m_settings.maxResponseSize )
    {
        Fail( MAILERR_PROTOCOL, "A response of the server is larger than the limit" );
        return;
    }
    m_assembly.Append( line.C_String(), ByteLength( line ), true );

    UInt32 literalSize = 0;
    bool nonSynchronizing = false;
    if ( ( continuesResponse || !IsPlainTextOfStatusResponse( line ) ) && CImapResponse::EndsWithLiteral( line, literalSize, nonSynchronizing ) )
    {
        if ( literalSize > m_settings.maxLiteralSize || m_assembly.GetDataSize() + literalSize > m_settings.maxResponseSize )
        {
            Fail( MAILERR_PROTOCOL, "The server announced a literal of " + CORE::UInt32ToString( literalSize ) + " bytes, which is more than the limit" );
            return;
        }
        m_assembly.Append( "\r\n", 2, true );
        m_expectingLiteral = true;
        m_assemblyHasLiteral = true;
        GetTransport().ExpectBytes( literalSize );
        return;
    }

    // the response is complete
    CORE::CDynamicBuffer response;
    response.Swap( m_assembly );
    const bool hadLiteral = m_assemblyHasLiteral;
    m_assemblyHasLiteral = false;
    if ( hadLiteral )
        GetLog().LogData( false, "FETCH response", response.GetDataSize() );
    HandleResponse( response );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnServerBytes( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    if ( !m_expectingLiteral )
        return;
    m_expectingLiteral = false;
    m_assembly.Append( bytes, true );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::HandleResponse( const CORE::CDynamicBuffer& buffer )
{GUCEF_TRACE;

    CImapResponse response;
    CORE::CString error;
    if ( !CImapResponse::Parse( buffer.GetConstBufferPtr(), buffer.GetDataSize(), CImapValueParser::DefaultMaxValues, response, error ) )
    {
        Fail( MAILERR_PROTOCOL, "The server sent a response that cannot be read: " + error );
        return;
    }

    if ( PHASE_GREETING == m_phase )
    {
        OnGreeting( response );
        return;
    }

    if ( CImapResponse::KIND_UNTAGGED == response.kind )
    {
        if ( ++m_responseCount > m_settings.maxResponseCount )
        {
            Fail( MAILERR_PROTOCOL, "The server sent more responses than the limit" );
            return;
        }
        OnUntagged( response );
    }
    else
    if ( CImapResponse::KIND_CONTINUATION == response.kind )
        OnContinuation( response );
    else
        OnTagged( response );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnGreeting( const CImapResponse& response )
{GUCEF_TRACE;

    if ( CImapResponse::KIND_UNTAGGED != response.kind || !response.isStatus )
    {
        Fail( MAILERR_PROTOCOL, "The server did not start with a greeting" );
        return;
    }
    if ( response.status == "BYE" )
    {
        Fail( MAILERR_CONNECTION, "The server refused the connection" );
        return;
    }
    if ( response.status != "OK" && response.status != "PREAUTH" )
    {
        Fail( MAILERR_PROTOCOL, "The greeting of the server is not valid" );
        return;
    }
    if ( GetTransport().HasBufferedData() )
    {
        Fail( MAILERR_PROTOCOL, "The server sent more than the greeting before the client spoke" );
        return;
    }
    m_loggedIn = response.status == "PREAUTH";
    m_phase = PHASE_SETUP;
    CImapCommandParts parts;
    parts.AddWord( "CAPABILITY" );
    QueueInternal( INTERNAL_CAPABILITY, parts );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::ReadCapabilities( const CORE::CString::StringVector& words )
{GUCEF_TRACE;

    m_capabilities.clear();
    for ( size_t i=0; i<words.size() && i<200; ++i )
        m_capabilities[ words[ i ].Uppercase() ] = CORE::CString();
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnUntaggedStatus( const CImapResponse& response )
{GUCEF_TRACE;

    if ( response.status == "BYE" )
    {
        if ( !m_logoutSent )
            Fail( MAILERR_CONNECTION, "The server is closing the connection: " + CEMailProtocolLog::RedactServerLine( response.text, false ) );
        return;
    }

    const bool selecting = GUCEF_NULL != m_current && ( IMAPCMD_SELECT == m_current->command || IMAPCMD_EXAMINE == m_current->command ) && INTERNAL_NONE == m_current->internal;
    if ( response.code == "CAPABILITY" )
    {
        ReadCapabilities( response.codeArguments.ParseElements( ' ', false ) );
        return;
    }
    if ( response.code == "COPYUID" && GUCEF_NULL != m_current && INTERNAL_NONE == m_current->internal && IMAPCMD_MOVE == m_current->command )
    {
        m_current->result.copyUid = ReadCopyUid( response.codeArguments );
        return;
    }
    if ( !selecting )
        return;

    CImapMailboxStatus& mailbox = m_current->result.mailbox;
    if ( response.code == "UIDVALIDITY" )
        mailbox.uidValidity = ReadUInt32( response.codeArguments );
    else
    if ( response.code == "UIDNEXT" )
        mailbox.uidNext = ReadUInt32( response.codeArguments );
    else
    if ( response.code == "UNSEEN" )
        mailbox.unseen = ReadUInt32( response.codeArguments );
    else
    if ( response.code == "READ-ONLY" )
        mailbox.readOnly = true;
    else
    if ( response.code == "PERMANENTFLAGS" )
    {
        CImapValueParser parser( response.codeArguments.C_String(), ByteLength( response.codeArguments ) );
        CImapValue list;
        if ( parser.ParseValue( list ) && list.IsList() )
            for ( UInt32 i=0; i<list.GetListSize() && i<200; ++i )
                mailbox.permanentFlags.push_back( list.GetElement( i ).GetString() );
    }
}

/*-------------------------------------------------------------------------*/

void
CImapClient::RaiseIdleUpdate( const CImapIdleUpdate& update )
{GUCEF_TRACE;

    CImapIdleUpdate copy( update );
    NotifyObservers( IdleUpdateEvent, &copy );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnUntagged( const CImapResponse& response )
{GUCEF_TRACE;

    if ( response.isStatus )
    {
        OnUntaggedStatus( response );
        return;
    }

    SCommand* command = m_current;
    const bool userCommand = GUCEF_NULL != command && INTERNAL_NONE == command->internal;
    const CImapValue& data = response.data;

    if ( response.hasNumber )
    {
        if ( response.keyword == "EXISTS" )
        {
            if ( userCommand && ( IMAPCMD_SELECT == command->command || IMAPCMD_EXAMINE == command->command ) )
                command->result.mailbox.exists = response.number;
            m_selected.exists = response.number;
            if ( m_idling )
            {
                CImapIdleUpdate update;
                update.kind = CImapIdleUpdate::KIND_EXISTS;
                update.number = response.number;
                RaiseIdleUpdate( update );
            }
        }
        else
        if ( response.keyword == "RECENT" )
        {
            if ( userCommand && ( IMAPCMD_SELECT == command->command || IMAPCMD_EXAMINE == command->command ) )
                command->result.mailbox.recent = response.number;
            m_selected.recent = response.number;
            if ( m_idling )
            {
                CImapIdleUpdate update;
                update.kind = CImapIdleUpdate::KIND_RECENT;
                update.number = response.number;
                RaiseIdleUpdate( update );
            }
        }
        else
        if ( response.keyword == "EXPUNGE" )
        {
            if ( m_selected.exists > 0 )
                --m_selected.exists;
            if ( m_idling )
            {
                CImapIdleUpdate update;
                update.kind = CImapIdleUpdate::KIND_EXPUNGE;
                update.number = response.number;
                RaiseIdleUpdate( update );
            }
        }
        else
        if ( response.keyword == "FETCH" )
        {
            CImapFetchedItem item;
            if ( !CImapFetchedItem::FromResponse( response, item ) )
            {
                Fail( MAILERR_PROTOCOL, "The server sent a FETCH response that is not valid" );
                return;
            }
            if ( userCommand && ( IMAPCMD_FETCH == command->command || IMAPCMD_STORE == command->command ) )
            {
                if ( command->result.fetched.size() >= m_settings.maxFetchResults )
                {
                    Fail( MAILERR_PROTOCOL, "The server sent more messages than the limit" );
                    return;
                }
                command->result.fetched.push_back( item );
            }
            else
            if ( m_idling && item.hasFlags )
            {
                CImapIdleUpdate update;
                update.kind = CImapIdleUpdate::KIND_FLAGS;
                update.number = item.sequence;
                update.hasUid = item.hasUid;
                update.uid = item.uid;
                update.flags = item.flags;
                RaiseIdleUpdate( update );
            }
        }
        return;
    }

    if ( response.keyword == "CAPABILITY" )
    {
        CORE::CString::StringVector words;
        for ( UInt32 i=0; i<data.GetListSize(); ++i )
            words.push_back( data.GetElement( i ).GetString() );
        ReadCapabilities( words );
        if ( userCommand && IMAPCMD_CAPABILITY == command->command )
            command->result.capabilities = words;
    }
    else
    if ( response.keyword == "LIST" || response.keyword == "LSUB" )
    {
        if ( userCommand && IMAPCMD_LIST == command->command )
        {
            if ( command->result.mailboxes.size() >= m_settings.maxFetchResults )
            {
                Fail( MAILERR_PROTOCOL, "The server sent more mailboxes than the limit" );
                return;
            }
            CImapMailboxEntry entry;
            const CImapValue& attributes = data.GetElement( 0 );
            for ( UInt32 i=0; i<attributes.GetListSize() && i<50; ++i )
                entry.attributes.push_back( attributes.GetElement( i ).GetString() );
            entry.hierarchyDelimiter = data.GetElement( 1 ).GetString();
            CORE::CString name;
            if ( !CImapMailboxName::Decode( data.GetElement( 2 ).GetString(), name ) )
                name = data.GetElement( 2 ).GetString();
            entry.name = name;
            command->result.mailboxes.push_back( entry );
        }
    }
    else
    if ( response.keyword == "SEARCH" )
    {
        if ( userCommand && IMAPCMD_SEARCH == command->command )
        {
            for ( UInt32 i=0; i<data.GetListSize(); ++i )
            {
                if ( !data.GetElement( i ).IsNumber() )
                    continue;
                if ( command->result.numbers.size() >= m_settings.maxFetchResults )
                {
                    Fail( MAILERR_PROTOCOL, "The server sent more search results than the limit" );
                    return;
                }
                command->result.numbers.push_back( (UInt32) data.GetElement( i ).GetNumber() );
            }
        }
    }
    else
    if ( response.keyword == "FLAGS" )
    {
        if ( userCommand && ( IMAPCMD_SELECT == command->command || IMAPCMD_EXAMINE == command->command ) )
        {
            const CImapValue& list = data.GetElement( 0 );
            for ( UInt32 i=0; i<list.GetListSize() && i<200; ++i )
                command->result.mailbox.flags.push_back( list.GetElement( i ).GetString() );
        }
    }
    else
    if ( response.keyword == "STATUS" )
    {
        if ( userCommand && IMAPCMD_STATUS == command->command )
        {
            const CImapValue& list = data.GetElement( 1 );
            CImapMailboxStatus& mailbox = command->result.mailbox;
            for ( UInt32 i=0; i+1<list.GetListSize(); i+=2 )
            {
                const CORE::CString key = list.GetElement( i ).GetString().Uppercase();
                const UInt32 value = (UInt32) list.GetElement( i + 1 ).GetNumber();
                if ( key == "MESSAGES" ) mailbox.exists = value;
                else if ( key == "RECENT" ) mailbox.recent = value;
                else if ( key == "UIDNEXT" ) mailbox.uidNext = value;
                else if ( key == "UIDVALIDITY" ) mailbox.uidValidity = value;
                else if ( key == "UNSEEN" ) mailbox.unseen = value;
            }
        }
    }
    // ENABLED, ESEARCH, NAMESPACE and others that were not asked for are left alone
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnContinuation( const CImapResponse& response )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_current )
    {
        Fail( MAILERR_PROTOCOL, "The server sent a continuation that was not asked for" );
        return;
    }
    if ( m_awaitingContinuation )
    {
        SendLiteralAndContinue();
        return;
    }
    if ( INTERNAL_NONE == m_current->internal && IMAPCMD_IDLE == m_current->command )
    {
        m_idling = true;
        m_idleDoneSent = false;
        m_idleStartedAt = GetTimeInMs();
        ClearDeadline();
        NotifyObservers( IdleStartedEvent );
        // a command that was given before the server was ready for IDLE ends it right away, and so does the caller who asked for the end
        const bool endWanted = m_idleEndWanted;
        m_idleEndWanted = false;
        if ( !m_queue.empty() || endWanted )
            SendDone( false );
        return;
    }
    if ( INTERNAL_AUTHENTICATE == m_current->internal && IsSaslActive() )
    {
        OnSaslChallenge( response.text );
        return;
    }
    Fail( MAILERR_PROTOCOL, "The server sent a continuation that was not asked for" );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnTagged( const CImapResponse& response )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_current || response.tag != m_current->tag )
    {
        Fail( MAILERR_PROTOCOL, "The server completed a command that was not given" );
        return;
    }
    if ( INTERNAL_NONE != m_current->internal )
    {
        OnInternalCompleted( response );
        return;
    }
    CompleteCurrent( response );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnInternalCompleted( const CImapResponse& response )
{GUCEF_TRACE;

    const EInternal internal = m_current->internal;
    GUCEF_DELETE m_current;
    m_current = GUCEF_NULL;
    ClearDeadline();
    const bool ok = response.status == "OK";

    switch ( internal )
    {
        case INTERNAL_CAPABILITY:
        {
            if ( !ok )
            {
                Fail( MAILERR_PROTOCOL, "The server did not tell its capabilities" );
                return;
            }
            ProceedAfterCapabilities();
            return;
        }
        case INTERNAL_STARTTLS:
        {
            if ( !ok )
            {
                Fail( MAILERR_TLS, "The server refused STARTTLS" );
                return;
            }
            m_capabilities.clear();
            BeginTlsUpgrade();
            return;
        }
        case INTERNAL_AUTHENTICATE:
        {
            if ( ok )
            {
                OnSaslSucceeded();
                m_loggedIn = true;
                CImapCommandParts parts;
                parts.AddWord( "CAPABILITY" );
                QueueInternal( INTERNAL_CAPABILITY, parts );
            }
            else
                OnSaslRejected( response.text, 0 );
            return;
        }
        case INTERNAL_LOGIN:
        {
            if ( !ok )
            {
                Fail( MAILERR_AUTH, "The login was refused: " + CEMailProtocolLog::RedactServerLine( response.text, false ) );
                return;
            }
            m_loggedIn = true;
            CImapCommandParts parts;
            parts.AddWord( "CAPABILITY" );
            QueueInternal( INTERNAL_CAPABILITY, parts );
            return;
        }
        default:
        {
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnTlsEstablished( void )
{GUCEF_TRACE;

    m_capabilities.clear();
    CImapCommandParts parts;
    parts.AddWord( "CAPABILITY" );
    QueueInternal( INTERNAL_CAPABILITY, parts );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::ProceedAfterCapabilities( void )
{GUCEF_TRACE;

    if ( !m_loggedIn )
    {
        const ETlsAction action = DecideTls( HasCapability( "STARTTLS" ) );
        if ( TLSACTION_FAILED == action )
            return;
        if ( TLSACTION_STARTTLS == action )
        {
            CImapCommandParts parts;
            parts.AddWord( "STARTTLS" );
            QueueInternal( INTERNAL_STARTTLS, parts );
            return;
        }
        if ( WantsLogin() )
        {
            ProceedToLogin();
            return;
        }
    }
    LoggedIn();
}

/*-------------------------------------------------------------------------*/

void
CImapClient::ProceedToLogin( void )
{GUCEF_TRACE;

    // the mechanisms the server lists as AUTH=NAME
    CORE::CString::StringVector mechanisms;
    for ( GUCEF::map< CORE::CString, CORE::CString >::const_iterator i = m_capabilities.begin(); i != m_capabilities.end(); ++i )
        if ( (*i).first.StartsWith( "AUTH=" ) )
            mechanisms.push_back( (*i).first.SubstrFromRange( 5, (*i).first.Length() ) );

    CORE::CString::StringVector supported;
    CEMailSaslFactory::GetSupportedMechanisms( supported );
    bool usable = false;
    for ( size_t i=0; i<mechanisms.size() && !usable; ++i )
        for ( size_t s=0; s<supported.size(); ++s )
            usable = usable || mechanisms[ i ].Equals( supported[ s ], false );
    if ( usable )
    {
        BeginSasl( mechanisms, HasCapability( "SASL-IR" ) );
        return;
    }

    if ( HasCapability( "LOGINDISABLED" ) )
    {
        Fail( GetTransport().IsSecure() ? MAILERR_AUTH : MAILERR_TLS, "The server does not allow LOGIN and has no mechanism that is usable" );
        return;
    }
    if ( !IsCredentialExposureAllowed() )
    {
        Fail( MAILERR_TLS, "The credentials are not sent without TLS" );
        return;
    }
    CEMailCredential credential;
    if ( !GetCredentialNow( credential, false ) || credential.secret.IsNULLOrEmpty() )
    {
        Fail( MAILERR_AUTH, "There is no credential to log in with" );
        return;
    }
    if ( MAILCRED_BEARER_TOKEN == credential.type )
    {
        Fail( MAILERR_AUTH, "The server has no authentication mechanism for an access token" );
        return;
    }

    CImapCommandParts parts;
    parts.AddWord( "LOGIN " );
    if ( !parts.AddString( credential.userName ) )
    {
        Fail( MAILERR_AUTH, "The user name cannot be sent" );
        return;
    }
    parts.AddWord( " " );
    if ( !parts.AddString( credential.secret ) )
    {
        Fail( MAILERR_AUTH, "The password cannot be sent" );
        return;
    }
    SetState( CLIENTSTATE_AUTHENTICATING );
    QueueInternal( INTERNAL_LOGIN, parts );
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse )
{GUCEF_TRACE;

    CImapCommandParts parts;
    parts.AddWord( ( "AUTHENTICATE " + mechanism + ( hasInitialResponse ? " " + base64InitialResponse : CORE::CString::Empty ) ).C_String() );
    QueueInternal( INTERNAL_AUTHENTICATE, parts );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::SendSaslResponse( const CORE::CString& base64Response )
{GUCEF_TRACE;

    SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
    return SendCommandLine( base64Response, true );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::SendSaslCancel( void )
{GUCEF_TRACE;

    SendCommandLine( "*", true );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::SendQuitCommand( void )
{GUCEF_TRACE;

    if ( m_idling && !m_idleDoneSent )
        SendCommandLine( "DONE" );
    m_logoutSent = true;
    SendCommandLine( NextTag() + " LOGOUT" );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::LoggedIn( void )
{GUCEF_TRACE;

    m_phase = PHASE_READY;
    SetReady();
    SendNext();
}

/*-------------------------------------------------------------------------*/

void
CImapClient::CompleteCurrent( const CImapResponse& response )
{GUCEF_TRACE;

    SCommand* command = m_current;
    const bool ok = response.status == "OK";

    // IDLE that was ended for the renewal starts again
    if ( IMAPCMD_IDLE == command->command )
    {
        m_idling = false;
        m_idleDoneSent = false;
        if ( m_idleRenewing && ok )
        {
            m_idleRenewing = false;
            command->tag = NextTag();
            SendFirstPart( command );
            return;
        }
        m_idleRenewing = false;
        m_idleEndWanted = false;
    }

    CImapCommandResult result = command->result;
    result.requestId = command->requestId;
    result.command = command->command;
    result.ok = ok;
    result.status = response.status;
    result.text = response.text;
    result.code = response.code;
    result.errorClass = MAILERR_NONE;
    if ( !ok )
    {
        result.errorClass = ( response.code == "UNAVAILABLE" || response.code == "INUSE" || response.code == "SERVERBUG" ) ? MAILERR_TEMPORARY : MAILERR_PERMANENT;
    }

    switch ( command->command )
    {
        case IMAPCMD_SELECT:
        case IMAPCMD_EXAMINE:
        {
            if ( ok )
            {
                result.mailbox.name = command->mailbox;
                if ( IMAPCMD_EXAMINE == command->command )
                    result.mailbox.readOnly = true;
                result.uidValidityChanged = 0 != command->expectedUidValidity && command->expectedUidValidity != result.mailbox.uidValidity;
                m_selected = result.mailbox;
                m_mailboxSelected = true;
                if ( result.uidValidityChanged )
                    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "IMAP " << m_settings.host << ": the UIDVALIDITY of the mailbox changed, the UIDs that were known mean nothing now" );
            }
            else
                m_mailboxSelected = false;
            break;
        }
        case IMAPCMD_STATUS:
        {
            result.mailbox.name = command->mailbox;
            break;
        }
        case IMAPCMD_CLOSE:
        {
            if ( ok )
                m_mailboxSelected = false;
            break;
        }
        case IMAPCMD_APPEND:
        {
            if ( ok && response.code == "APPENDUID" )
            {
                CORE::CString::StringVector words = response.codeArguments.ParseElements( ' ', false );
                if ( words.size() >= 2 )
                    result.appendUid = ReadUInt32( words[ 1 ] );
            }
            break;
        }
        case IMAPCMD_COPY:
        case IMAPCMD_MOVE:
        {
            if ( ok && response.code == "COPYUID" && 0 == result.copyUid )
                result.copyUid = ReadCopyUid( response.codeArguments );
            break;
        }
        default:
        {
            break;
        }
    }

    GUCEF_DELETE m_current;
    m_current = GUCEF_NULL;
    ClearDeadline();
    if ( !NotifyObservers( CommandCompletedEvent, &result ) )
        return;
    SendNext();
}

/*-------------------------------------------------------------------------*/

void
CImapClient::FailAllCommands( EMailErrorClass errorClass, const CORE::CString& description )
{GUCEF_TRACE;

    GUCEF::vector< SCommand* > pending;
    if ( GUCEF_NULL != m_current )
        pending.push_back( m_current );
    m_current = GUCEF_NULL;
    for ( size_t i=0; i<m_queue.size(); ++i )
        pending.push_back( m_queue[ i ] );
    m_queue.clear();
    m_idling = false;
    m_idleEndWanted = false;
    m_awaitingContinuation = false;

    for ( size_t i=0; i<pending.size(); ++i )
    {
        SCommand* command = pending[ i ];
        if ( INTERNAL_NONE == command->internal )
        {
            CImapCommandResult result;
            result.requestId = command->requestId;
            result.command = command->command;
            result.ok = false;
            result.errorClass = errorClass;
            result.text = description;
            NotifyObservers( CommandCompletedEvent, &result );
        }
        GUCEF_DELETE command;
    }
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnClientFailed( const CEMailErrorEventData& error )
{GUCEF_TRACE;

    m_mailboxSelected = false;
    FailAllCommands( error.errorClass, error.description );
}

/*-------------------------------------------------------------------------*/

void
CImapClient::OnClientClosed( void )
{GUCEF_TRACE;

    m_mailboxSelected = false;
    FailAllCommands( MAILERR_CANCELLED, "The connection was closed" );
}

/*-------------------------------------------------------------------------*/

bool
CImapClient::FetchNewSync( const CImapClientSettings& settings, const CImapFetchOptions& options, CImapFetchResult& result, CIEMailCredentialProvider* credentialProvider )
{GUCEF_TRACE;

    // everything below happens on the calling thread, on a pulse generator that this thread owns
    result = CImapFetchResult();
    CORE::PulseGeneratorPtr pulseGenerator = CreateLocalPulseGenerator();
    CImapClient client( pulseGenerator );
    CImapFetchRunner runner( client, options, result );
    if ( GUCEF_NULL != credentialProvider )
        client.SetCredentialProvider( credentialProvider );

    if ( !client.Connect( settings ) )
    {
        result.errorClass = client.GetLastError().errorClass;
        result.errorDescription = client.GetLastError().description;
        return false;
    }

    const UInt32 timeout = settings.connectTimeoutInMs + 20 * settings.commandTimeoutInMs + 5000;
    RunPulseUntil( pulseGenerator, runner.done, timeout );
    if ( !runner.done )
    {
        result.success = false;
        result.errorClass = MAILERR_TIMEOUT;
        result.errorDescription = "The fetch did not finish in time";
    }
    client.Disconnect();
    return result.success;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
