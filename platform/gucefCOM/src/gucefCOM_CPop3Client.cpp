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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_DVMD5UTILS_H
#include "dvmd5utils.h"
#define GUCEF_CORE_DVMD5UTILS_H
#endif /* GUCEF_CORE_DVMD5UTILS_H ? */

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#include "gucefCOM_CPop3Client.h"

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

const CORE::CEvent CPop3Client::CommandCompletedEvent = "GUCEF::COM::CPop3Client::CommandCompletedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
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
ParseNumber( const char*& p )
{
    UInt64 value = 0;
    while ( *p >= '0' && *p <= '9' )
    {
        if ( value < 0xFFFFFFFFull )
            value = value * 10 + (UInt64)( *p - '0' );
        ++p;
    }
    return value > 0xFFFFFFFFull ? 0xFFFFFFFFu : (UInt32) value;
}

/*-------------------------------------------------------------------------*/

bool
IsValidUid( const CORE::CString& uid )
{
    const UInt32 size = ByteLength( uid );
    if ( 0 == size || size > 70 )
        return false;
    const char* p = uid.C_String();
    for ( UInt32 i=0; i<size; ++i )
        if ( ( (UInt8) p[ i ] ) < 0x21 || ( (UInt8) p[ i ] ) > 0x7E )
            return false;
    return true;
}

/*-------------------------------------------------------------------------*/

/** Runs the steps of FetchNewSync() on the events of a client */
class CPop3FetchRunner : public CORE::CObserver
{
    public:

    bool done;

    CPop3FetchRunner( CPop3Client& client, const CPop3ClientSettings& settings, const CPop3FetchOptions& options, CPop3FetchResult& result )
        : CORE::CObserver()
        , done( false )
        , m_client( client )
        , m_settings( settings )
        , m_options( options )
        , m_result( result )
        , m_stage( STAGE_UIDL )
        , m_pendingId( 0 )
        , m_toFetch()
        , m_index( 0 )
    {
        typedef CORE::CTEventHandlerFunctor< CPop3FetchRunner > TCallback;
        TCallback ready( this, &CPop3FetchRunner::OnReady );
        SubscribeTo( &client, CPop3Client::ReadyEvent, ready );
        TCallback completed( this, &CPop3FetchRunner::OnCommandCompleted );
        SubscribeTo( &client, CPop3Client::CommandCompletedEvent, completed );
        TCallback error( this, &CPop3FetchRunner::OnError );
        SubscribeTo( &client, CPop3Client::ErrorEvent, error );
        TCallback disconnected( this, &CPop3FetchRunner::OnDisconnected );
        SubscribeTo( &client, CPop3Client::DisconnectedEvent, disconnected );
    }

    virtual ~CPop3FetchRunner()
    {
        UnsubscribeAllFromObserver();
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::COM::CPop3FetchRunner";
        return classTypeName;
    }

    private:

    enum EStage { STAGE_UIDL, STAGE_RETR, STAGE_DELE, STAGE_DONE };

    void Finish( bool success )
    {
        m_result.success = success;
        done = true;
    }

    void OnReady( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        m_stage = STAGE_UIDL;
        m_pendingId = m_client.Uidl();
    }

    void FetchNext( void )
    {
        if ( m_index < m_toFetch.size() )
        {
            m_stage = STAGE_RETR;
            m_pendingId = m_client.Retr( m_toFetch[ m_index ].number );
            return;
        }
        // deletions come after everything was fetched, so that a failure on the way loses no mail
        m_stage = STAGE_DELE;
        m_index = 0;
        DeleteNext();
    }

    void DeleteNext( void )
    {
        if ( m_options.deleteAfterFetch )
        {
            while ( m_index < m_result.messages.size() && !m_result.messages[ m_index ].parsed )
                ++m_index;
            if ( m_index < m_result.messages.size() )
            {
                m_pendingId = m_client.Dele( m_result.messages[ m_index ].number );
                return;
            }
        }
        m_stage = STAGE_DONE;
        Finish( true );
    }

    void OnCommandCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const CPop3CommandResult* command = CORE::CICloneable::CloneableAs< CPop3CommandResult >( eventData );
        if ( GUCEF_NULL == command || command->requestId != m_pendingId || done )
            return;

        if ( STAGE_UIDL == m_stage )
        {
            if ( !command->ok )
            {
                m_result.errorClass = MAILERR_PERMANENT;
                m_result.errorDescription = "The server does not support UIDL, which is needed to tell new messages from known ones";
                Finish( false );
                return;
            }
            UInt32 limit = m_options.maxMessages > 0 ? m_options.maxMessages : m_settings.maxMessagesPerSession;
            m_result.messageCountOnServer = (UInt32) command->entries.size();
            for ( size_t i=0; i<command->entries.size(); ++i )
            {
                const CPop3ListEntry& entry = command->entries[ i ];
                m_result.allUids.push_back( entry.uid );
                if ( m_options.knownUids.find( entry.uid ) != m_options.knownUids.end() )
                    continue;
                if ( 0 == limit || m_toFetch.size() < limit )
                    m_toFetch.push_back( entry );
            }
            m_index = 0;
            FetchNext();
            return;
        }

        if ( STAGE_RETR == m_stage )
        {
            CPop3FetchedMessage fetched;
            fetched.uid = m_toFetch[ m_index ].uid;
            fetched.number = m_toFetch[ m_index ].number;
            if ( command->ok )
            {
                CMimeParser parser( m_options.mimeLimits );
                CORE::CString error;
                fetched.parsed = parser.ParseMessage( command->data.GetConstBufferPtr(), command->data.GetDataSize(), fetched.message, &error );
            }
            m_result.messages.push_back( fetched );
            ++m_index;
            FetchNext();
            return;
        }

        if ( STAGE_DELE == m_stage )
        {
            m_result.messages[ m_index ].deleted = command->ok;
            ++m_index;
            DeleteNext();
        }
    }

    void OnError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const CMailErrorEventData* error = CORE::CICloneable::CloneableAs< CMailErrorEventData >( eventData );
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
        {
            m_result.errorClass = MAILERR_CONNECTION;
            m_result.errorDescription = "The connection was closed";
            Finish( false );
        }
    }

    private:

    CPop3Client& m_client;
    const CPop3ClientSettings& m_settings;
    const CPop3FetchOptions& m_options;
    CPop3FetchResult& m_result;
    EStage m_stage;
    UInt32 m_pendingId;
    GUCEF::vector< CPop3ListEntry > m_toFetch;
    size_t m_index;
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CPop3ListEntry::CPop3ListEntry( void )
    : number( 0 )
    , size( 0 )
    , uid()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult::CPop3CommandResult( void )
    : CORE::CICloneable()
    , requestId( 0 )
    , command( POP3CMD_NOOP )
    , ok( false )
    , errorClass( MAILERR_NONE )
    , text()
    , number( 0 )
    , messageCount( 0 )
    , totalSize( 0 )
    , entries()
    , data()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult::CPop3CommandResult( const CPop3CommandResult& src )
    : CORE::CICloneable( src )
    , requestId( src.requestId )
    , command( src.command )
    , ok( src.ok )
    , errorClass( src.errorClass )
    , text( src.text )
    , number( src.number )
    , messageCount( src.messageCount )
    , totalSize( src.totalSize )
    , entries( src.entries )
    , data( src.data )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult::~CPop3CommandResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3CommandResult&
CPop3CommandResult::operator=( const CPop3CommandResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        requestId = src.requestId;
        command = src.command;
        ok = src.ok;
        errorClass = src.errorClass;
        text = src.text;
        number = src.number;
        messageCount = src.messageCount;
        totalSize = src.totalSize;
        entries = src.entries;
        data = src.data;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPop3CommandResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPop3CommandResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CPop3CommandResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CPop3CommandResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CPop3CommandResult";
}

/*-------------------------------------------------------------------------*/

CPop3FetchOptions::CPop3FetchOptions( void )
    : knownUids()
    , maxMessages( 0 )
    , deleteAfterFetch( false )
    , mimeLimits()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchedMessage::CPop3FetchedMessage( void )
    : uid()
    , number( 0 )
    , message()
    , parsed( false )
    , deleted( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult::CPop3FetchResult( void )
    : CORE::CICloneable()
    , success( false )
    , errorClass( MAILERR_NONE )
    , errorDescription()
    , messageCountOnServer( 0 )
    , allUids()
    , messages()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult::CPop3FetchResult( const CPop3FetchResult& src )
    : CORE::CICloneable( src )
    , success( src.success )
    , errorClass( src.errorClass )
    , errorDescription( src.errorDescription )
    , messageCountOnServer( src.messageCountOnServer )
    , allUids( src.allUids )
    , messages( src.messages )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult::~CPop3FetchResult()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CPop3FetchResult&
CPop3FetchResult::operator=( const CPop3FetchResult& src )
{GUCEF_TRACE;

    if ( &src != this )
    {
        success = src.success;
        errorClass = src.errorClass;
        errorDescription = src.errorDescription;
        messageCountOnServer = src.messageCountOnServer;
        allUids = src.allUids;
        messages = src.messages;
    }
    return *this;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CPop3FetchResult::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CPop3FetchResult( *this );
}

/*-------------------------------------------------------------------------*/

const char*
CPop3FetchResult::GetCloneableTypeName( void ) const
{GUCEF_TRACE;

    return CloneableTypeName();
}

/*-------------------------------------------------------------------------*/

const char*
CPop3FetchResult::CloneableTypeName( void )
{GUCEF_TRACE;

    return "GUCEF::COM::CPop3FetchResult";
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::RegisterEvents( void )
{GUCEF_TRACE;

    CommandCompletedEvent.Initialize();
    CMailClientBase::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPop3Client::CPop3Client( void )
    : CMailClientBase( CORE::PulseGeneratorPtr() )
    , m_settings()
    , m_phase( PHASE_GREETING )
    , m_timestamp()
    , m_capabilities()
    , m_queue()
    , m_commandSent( false )
    , m_collectingLines( false )
    , m_awaitingBytes( false )
    , m_result()
    , m_collectedLines( 0 )
    , m_collectedBytes( 0 )
    , m_pendingUser()
    , m_capaKnown( false )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CPop3Client::CPop3Client( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CMailClientBase( pulseGenerator )
    , m_settings()
    , m_phase( PHASE_GREETING )
    , m_timestamp()
    , m_capabilities()
    , m_queue()
    , m_commandSent( false )
    , m_collectingLines( false )
    , m_awaitingBytes( false )
    , m_result()
    , m_collectedLines( 0 )
    , m_collectedBytes( 0 )
    , m_pendingUser()
    , m_capaKnown( false )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CPop3Client::~CPop3Client()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPop3Client::GetProtocolName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "POP3";
    return name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CPop3Client::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CPop3Client";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CPop3ClientSettings&
CPop3Client::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CPop3Client::HasCapability( const CORE::CString& name ) const
{GUCEF_TRACE;

    return m_capabilities.find( name.Uppercase() ) != m_capabilities.end();
}

/*-------------------------------------------------------------------------*/

bool
CPop3Client::Connect( const CPop3ClientSettings& settings )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED != GetState() )
        return false;
    m_settings = settings;
    m_phase = PHASE_GREETING;
    m_timestamp.Clear();
    m_capabilities.clear();
    m_queue.clear();
    m_commandSent = false;
    m_collectingLines = false;
    m_awaitingBytes = false;
    m_capaKnown = false;
    return BeginConnect( m_settings );
}

/*-------------------------------------------------------------------------*/

UInt32
CPop3Client::Queue( EPop3Command command, UInt32 number, UInt32 lines )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED == GetState() )
        return 0;
    SCommand entry;
    entry.requestId = NextRequestId();
    entry.command = command;
    entry.number = number;
    entry.lines = lines;
    m_queue.push_back( entry );
    SendNextCommand();
    return entry.requestId;
}

/*-------------------------------------------------------------------------*/

UInt32 CPop3Client::Stat( void ) { return Queue( POP3CMD_STAT, 0, 0 ); }
UInt32 CPop3Client::List( UInt32 messageNumber ) { return Queue( POP3CMD_LIST, messageNumber, 0 ); }
UInt32 CPop3Client::Uidl( UInt32 messageNumber ) { return Queue( POP3CMD_UIDL, messageNumber, 0 ); }
UInt32 CPop3Client::Retr( UInt32 messageNumber ) { return Queue( POP3CMD_RETR, messageNumber, 0 ); }
UInt32 CPop3Client::Top( UInt32 messageNumber, UInt32 lines ) { return Queue( POP3CMD_TOP, messageNumber, lines ); }
UInt32 CPop3Client::Dele( UInt32 messageNumber ) { return Queue( POP3CMD_DELE, messageNumber, 0 ); }
UInt32 CPop3Client::Rset( void ) { return Queue( POP3CMD_RSET, 0, 0 ); }
UInt32 CPop3Client::Noop( void ) { return Queue( POP3CMD_NOOP, 0, 0 ); }

/*-------------------------------------------------------------------------*/

void
CPop3Client::SendNextCommand( void )
{GUCEF_TRACE;

    if ( PHASE_READY != m_phase || CLIENTSTATE_READY != GetState() || m_commandSent || m_queue.empty() )
        return;

    const SCommand& command = m_queue.front();
    CORE::CString line;
    switch ( command.command )
    {
        case POP3CMD_STAT: line = "STAT"; break;
        case POP3CMD_LIST: line = command.number > 0 ? "LIST " + CORE::UInt32ToString( command.number ) : CORE::CString( "LIST" ); break;
        case POP3CMD_UIDL: line = command.number > 0 ? "UIDL " + CORE::UInt32ToString( command.number ) : CORE::CString( "UIDL" ); break;
        case POP3CMD_RETR: line = "RETR " + CORE::UInt32ToString( command.number ); break;
        case POP3CMD_TOP: line = "TOP " + CORE::UInt32ToString( command.number ) + " " + CORE::UInt32ToString( command.lines ); break;
        case POP3CMD_DELE: line = "DELE " + CORE::UInt32ToString( command.number ); break;
        case POP3CMD_RSET: line = "RSET"; break;
        case POP3CMD_NOOP: line = "NOOP"; break;
    }

    m_result = CPop3CommandResult();
    m_result.requestId = command.requestId;
    m_result.command = command.command;
    m_result.number = command.number;
    m_collectedLines = 0;
    m_collectedBytes = 0;
    m_commandSent = true;
    SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
    SendCommandLine( line );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::SendCapa( void )
{GUCEF_TRACE;

    m_phase = PHASE_CAPA;
    SendCommandLine( "CAPA" );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnConnectionEstablished( void )
{GUCEF_TRACE;

    m_phase = PHASE_GREETING;
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnTlsEstablished( void )
{GUCEF_TRACE;

    // what the server offered before the handshake is not trusted
    m_capabilities.clear();
    SendCapa();
    SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnServerLine( const CORE::CString& line )
{GUCEF_TRACE;

    const char* p = line.C_String();
    const UInt32 size = ByteLength( line );

    if ( m_collectingLines )
    {
        if ( 1 == size && '.' == p[ 0 ] )
        {
            m_collectingLines = false;
            if ( PHASE_CAPA == m_phase )
                OnCapabilities();
            else
                CompleteCommand( true, m_result.text );
            return;
        }
        if ( ++m_collectedLines > m_settings.maxResponseCount )
        {
            Fail( MAILERR_PROTOCOL, "The server sent more lines than the limit" );
            return;
        }
        CORE::CString content = size >= 2 && '.' == p[ 0 ] && '.' == p[ 1 ] ? CORE::CString( p + 1, size - 1 ) : line;

        if ( PHASE_CAPA == m_phase )
        {
            CORE::CString keyword = content.Trim( true ).Trim( false );
            CORE::CString parameters;
            Int32 space = keyword.HasChar( ' ' );
            if ( space > 0 )
            {
                parameters = keyword.SubstrFromRange( (UInt32) space + 1, keyword.Length() );
                keyword = keyword.SubstrToIndex( (UInt32) space, true );
            }
            if ( m_capabilities.size() < 100 )
                m_capabilities[ keyword.Uppercase() ] = parameters;
            return;
        }

        CPop3ListEntry entry;
        const char* c = content.C_String();
        entry.number = ParseNumber( c );
        if ( ' ' != *c || 0 == entry.number )
        {
            Fail( MAILERR_PROTOCOL, "The server sent a list entry that is not valid" );
            return;
        }
        ++c;
        if ( POP3CMD_UIDL == m_result.command )
        {
            entry.uid = CORE::CString( c );
            if ( !IsValidUid( entry.uid ) )
            {
                Fail( MAILERR_PROTOCOL, "The server sent a unique id that is not valid" );
                return;
            }
        }
        else
            entry.size = ParseNumber( c );
        m_result.entries.push_back( entry );
        return;
    }

    // the status line
    if ( size >= 3 && 0 == strncmp( p, "+OK", 3 ) && ( 3 == size || ' ' == p[ 3 ] ) )
    {
        OnStatus( true, size > 4 ? CORE::CString( p + 4, size - 4 ) : CORE::CString() );
        return;
    }
    if ( size >= 4 && 0 == strncmp( p, "-ERR", 4 ) && ( 4 == size || ' ' == p[ 4 ] ) )
    {
        OnStatus( false, size > 5 ? CORE::CString( p + 5, size - 5 ) : CORE::CString() );
        return;
    }
    if ( size >= 1 && '+' == p[ 0 ] && ( 1 == size || ' ' == p[ 1 ] ) )
    {
        OnContinuation( size > 2 ? CORE::CString( p + 2, size - 2 ) : CORE::CString() );
        return;
    }
    Fail( MAILERR_PROTOCOL, "The server sent a line that is not a POP3 response" );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnContinuation( const CORE::CString& text )
{GUCEF_TRACE;

    if ( PHASE_AUTH != m_phase )
    {
        Fail( MAILERR_PROTOCOL, "The server sent a continuation that was not asked for" );
        return;
    }
    OnSaslChallenge( text );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnCapabilities( void )
{GUCEF_TRACE;

    m_capaKnown = true;
    const ETlsAction action = DecideTls( HasCapability( "STLS" ) );
    if ( TLSACTION_FAILED == action )
        return;
    if ( TLSACTION_STARTTLS == action )
    {
        m_phase = PHASE_STLS;
        SendCommandLine( "STLS" );
        return;
    }
    ProceedToLogin();
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::ProceedToLogin( void )
{GUCEF_TRACE;

    if ( !WantsLogin() )
    {
        OnLoggedIn();
        return;
    }

    // SASL when the server has a mechanism that we have
    CORE::CString::StringVector serverMechanisms;
    GUCEF::map< CORE::CString, CORE::CString >::const_iterator sasl = m_capabilities.find( "SASL" );
    if ( sasl != m_capabilities.end() )
        serverMechanisms = (*sasl).second.ParseElements( ' ', false );
    CORE::CString::StringVector supported;
    CMailSaslFactory::GetSupportedMechanisms( supported );
    bool usable = false;
    for ( size_t i=0; i<serverMechanisms.size() && !usable; ++i )
        for ( size_t s=0; s<supported.size(); ++s )
            usable = usable || serverMechanisms[ i ].Equals( supported[ s ], false );
    if ( usable )
    {
        BeginSasl( serverMechanisms, true );
        return;
    }

    // USER and PASS (or APOP): the password goes over the line as it is
    CMailCredential credential;
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

    #ifndef GUCEF_CORE_NO_MD5_SUPPORT
    if ( m_settings.useApop && !m_timestamp.IsNULLOrEmpty() && m_settings.allowInsecureAuth )
    {
        UInt8 digest[ 16 ];
        CORE::md5fromstring( ( m_timestamp + credential.secret ).C_String(), digest );
        m_phase = PHASE_APOP;
        SendCommandLine( "APOP " + credential.userName + " " + CORE::Base16Encode( digest, 16 ).Lowercase(), false );
        return;
    }
    #endif /* GUCEF_CORE_NO_MD5_SUPPORT ? */

    if ( !IsCredentialExposureAllowed() )
    {
        Fail( MAILERR_TLS, "The credentials are not sent without TLS" );
        return;
    }
    m_pendingUser = credential.secret;
    m_phase = PHASE_USER;
    SetState( CLIENTSTATE_AUTHENTICATING );
    SendCommandLine( "USER " + credential.userName, false );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnLoggedIn( void )
{GUCEF_TRACE;

    m_phase = PHASE_READY;
    SetReady();
    SendNextCommand();
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnStatus( bool ok, const CORE::CString& text )
{GUCEF_TRACE;

    switch ( m_phase )
    {
        case PHASE_GREETING:
        {
            if ( !ok )
            {
                Fail( MAILERR_PERMANENT, "The server refused the connection" );
                return;
            }
            if ( GetTransport().HasBufferedData() )
            {
                Fail( MAILERR_PROTOCOL, "The server sent more than the greeting before the client spoke" );
                return;
            }
            // <timestamp> in the greeting is what APOP is made of
            Int32 open = text.HasChar( '<' );
            Int32 close = text.HasChar( '>' );
            if ( open >= 0 && close > open )
                m_timestamp = text.SubstrFromRange( (UInt32) open, (UInt32) close + 1 );
            SendCapa();
            SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
            return;
        }
        case PHASE_CAPA:
        {
            if ( ok )
                m_collectingLines = true;
            else
            {
                m_capabilities.clear();
                OnCapabilities();
            }
            return;
        }
        case PHASE_STLS:
        {
            if ( !ok )
            {
                Fail( MAILERR_TLS, "The server refused STLS" );
                return;
            }
            m_capabilities.clear();
            BeginTlsUpgrade();
            return;
        }
        case PHASE_AUTH:
        {
            if ( ok )
            {
                OnSaslSucceeded();
                OnLoggedIn();
            }
            else
                OnSaslRejected( text, 0 );
            return;
        }
        case PHASE_USER:
        {
            if ( !ok )
            {
                m_pendingUser.Clear();
                Fail( MAILERR_AUTH, "The server refused the user" );
                return;
            }
            const CORE::CString secret = m_pendingUser;
            m_pendingUser.Clear();
            m_phase = PHASE_PASS;
            SendCommandLine( "PASS " + secret, false );
            return;
        }
        case PHASE_PASS:
        case PHASE_APOP:
        {
            if ( ok )
                OnLoggedIn();
            else
                Fail( MAILERR_AUTH, "Authentication was refused: " + CMailProtocolLog::RedactServerLine( text, false ) );
            return;
        }
        case PHASE_READY:
        {
            break;
        }
    }

    if ( !m_commandSent || m_queue.empty() )
    {
        Fail( MAILERR_PROTOCOL, "The server sent a response that was not asked for" );
        return;
    }
    if ( !ok )
    {
        m_result.errorClass = MAILERR_PERMANENT;
        CompleteCommand( false, text );
        return;
    }

    const SCommand& command = m_queue.front();
    const char* c = text.C_String();
    switch ( command.command )
    {
        case POP3CMD_STAT:
        {
            m_result.messageCount = ParseNumber( c );
            if ( ' ' == *c ) ++c;
            m_result.totalSize = ParseNumber( c );
            CompleteCommand( true, text );
            return;
        }
        case POP3CMD_LIST:
        case POP3CMD_UIDL:
        {
            if ( command.number > 0 )
            {
                CPop3ListEntry entry;
                entry.number = ParseNumber( c );
                if ( ' ' == *c ) ++c;
                if ( POP3CMD_UIDL == command.command )
                    entry.uid = CORE::CString( c );
                else
                    entry.size = ParseNumber( c );
                m_result.entries.push_back( entry );
                CompleteCommand( true, text );
            }
            else
            {
                m_result.text = text;
                m_collectingLines = true;
            }
            return;
        }
        case POP3CMD_RETR:
        case POP3CMD_TOP:
        {
            m_result.text = text;
            m_awaitingBytes = true;
            GetTransport().ExpectDotTerminatedBytes( m_settings.maxResponseSize );
            return;
        }
        default:
        {
            CompleteCommand( true, text );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::Unstuff( const CORE::CDynamicBuffer& stuffed, CORE::CDynamicBuffer& message )
{GUCEF_TRACE;

    message.Clear();
    message.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    const char* p = static_cast< const char* >( stuffed.GetConstBufferPtr() );
    const UInt32 size = stuffed.GetDataSize();
    UInt32 start = 0;
    bool lineStart = true;
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( lineStart && '.' == p[ i ] )
        {
            // a dot at the start of a line is the stuffing, when another follows
            if ( i + 1 < size && '.' == p[ i + 1 ] )
            {
                if ( i > start )
                    message.Append( p + start, i - start, true );
                start = i + 1;
            }
        }
        lineStart = '\n' == p[ i ];
    }
    if ( size > start )
        message.Append( p + start, size - start, true );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnServerBytes( const CORE::CDynamicBuffer& bytes )
{GUCEF_TRACE;

    if ( !m_awaitingBytes )
        return;
    m_awaitingBytes = false;
    GetLog().LogData( false, "message", bytes.GetDataSize() );
    Unstuff( bytes, m_result.data );
    CompleteCommand( true, m_result.text );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::CompleteCommand( bool ok, const CORE::CString& text )
{GUCEF_TRACE;

    if ( m_queue.empty() )
        return;
    m_result.ok = ok;
    m_result.text = text;
    if ( !ok && MAILERR_NONE == m_result.errorClass )
        m_result.errorClass = MAILERR_PERMANENT;
    CPop3CommandResult result = m_result;
    m_queue.erase( m_queue.begin() );
    m_commandSent = false;
    m_collectingLines = false;
    m_awaitingBytes = false;
    m_result = CPop3CommandResult();
    ClearDeadline();

    if ( !NotifyObservers( CommandCompletedEvent, &result ) )
        return;
    SendNextCommand();
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::FailAllCommands( EMailErrorClass errorClass, const CORE::CString& description )
{GUCEF_TRACE;

    m_collectingLines = false;
    m_awaitingBytes = false;
    m_commandSent = false;
    GUCEF::vector< SCommand > pending;
    pending.swap( m_queue );
    for ( size_t i=0; i<pending.size(); ++i )
    {
        CPop3CommandResult result;
        result.requestId = pending[ i ].requestId;
        result.command = pending[ i ].command;
        result.number = pending[ i ].number;
        result.ok = false;
        result.errorClass = errorClass;
        result.text = description;
        if ( !NotifyObservers( CommandCompletedEvent, &result ) )
            return;
    }
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnClientFailed( const CMailErrorEventData& error )
{GUCEF_TRACE;

    FailAllCommands( error.errorClass, error.description );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::OnClientClosed( void )
{GUCEF_TRACE;

    FailAllCommands( MAILERR_CANCELLED, "The connection was closed" );
}

/*-------------------------------------------------------------------------*/

bool
CPop3Client::SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse )
{GUCEF_TRACE;

    m_phase = PHASE_AUTH;
    SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
    return SendCommandLine( "AUTH " + mechanism + ( hasInitialResponse ? " " + base64InitialResponse : CORE::CString::Empty ), false );
}

/*-------------------------------------------------------------------------*/

bool
CPop3Client::SendSaslResponse( const CORE::CString& base64Response )
{GUCEF_TRACE;

    SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
    return SendCommandLine( base64Response, true );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::SendSaslCancel( void )
{GUCEF_TRACE;

    SendCommandLine( "*", true );
}

/*-------------------------------------------------------------------------*/

void
CPop3Client::SendQuitCommand( void )
{GUCEF_TRACE;

    SendCommandLine( "QUIT" );
}

/*-------------------------------------------------------------------------*/

bool
CPop3Client::FetchNewSync( const CPop3ClientSettings& settings, const CPop3FetchOptions& options, CPop3FetchResult& result, CIMailCredentialProvider* credentialProvider )
{GUCEF_TRACE;

    // everything below happens on the calling thread, on a pulse generator that this thread owns
    result = CPop3FetchResult();
    CORE::PulseGeneratorPtr pulseGenerator = CreateLocalPulseGenerator();
    CPop3Client client( pulseGenerator );
    CPop3FetchRunner runner( client, settings, options, result );
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
