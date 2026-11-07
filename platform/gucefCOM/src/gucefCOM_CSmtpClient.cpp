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

#ifndef GUCEF_CORE_COBSERVER_H
#include "CObserver.h"
#define GUCEF_CORE_COBSERVER_H
#endif /* GUCEF_CORE_COBSERVER_H ? */

#ifndef GUCEF_COM_CMIMECODEC_H
#include "gucefCOM_CMimeCodec.h"
#define GUCEF_COM_CMIMECODEC_H
#endif /* GUCEF_COM_CMIMECODEC_H ? */

#ifndef GUCEF_COM_CMIMECOMPOSER_H
#include "gucefCOM_CMimeComposer.h"
#define GUCEF_COM_CMIMECOMPOSER_H
#endif /* GUCEF_COM_CMIMECOMPOSER_H ? */

#include "gucefCOM_CSmtpClient.h"

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

const CORE::CEvent CSmtpClient::SendCompletedEvent = "GUCEF::COM::CSmtpClient::SendCompletedEvent";

static const UInt32 MaxReplyLines = 1000;
static const UInt32 MaxReplyBytes = 64 * 1024;

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

CORE::CString
ToUpperToken( const CORE::CString& text )
{
    return text.Uppercase().Trim( true ).Trim( false );
}

/*-------------------------------------------------------------------------*/

/** Collects the outcome of a message for the blocking use of the client */
class CSmtpSyncSink : public CORE::CObserver
{
    public:

    bool done;
    CSmtpSendResult result;

    CSmtpSyncSink( void )
        : CORE::CObserver()
        , done( false )
        , result()
    {
    }

    virtual ~CSmtpSyncSink()
    {
        UnsubscribeAllFromObserver();
    }

    void Attach( CSmtpClient& client )
    {
        typedef CORE::CTEventHandlerFunctor< CSmtpSyncSink > TCallback;
        TCallback sendCompleted( this, &CSmtpSyncSink::OnSendCompleted );
        SubscribeTo( &client, CSmtpClient::SendCompletedEvent, sendCompleted );
    }

    void OnSendCompleted( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
    {
        const CSmtpSendResult* data = CORE::CICloneable::CloneableAs< CSmtpSendResult >( eventData );
        if ( GUCEF_NULL != data )
        {
            result = *data;
            done = true;
        }
    }

    virtual const CORE::CString& GetClassTypeName( void ) const GUCEF_VIRTUAL_OVERRIDE
    {
        static const CORE::CString classTypeName = "GUCEF::COM::CSmtpSyncSink";
        return classTypeName;
    }
};

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

void
CSmtpClient::RegisterEvents( void )
{GUCEF_TRACE;

    SendCompletedEvent.Initialize();
    CEMailClientBase::RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSmtpClient::CSmtpClient( void )
    : CEMailClientBase( CORE::PulseGeneratorPtr() )
    , m_settings()
    , m_receivedText()
    , m_reply()
    , m_expected()
    , m_extensions()
    , m_triedHelo( false )
    , m_authenticated( false )
    , m_isHelo( false )
    , m_replyBytes( 0 )
    , m_jobs()
    , m_current( GUCEF_NULL )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

CSmtpClient::CSmtpClient( const CORE::PulseGeneratorPtr& pulseGenerator )
    : CEMailClientBase( pulseGenerator )
    , m_settings()
    , m_receivedText()
    , m_reply()
    , m_expected()
    , m_extensions()
    , m_triedHelo( false )
    , m_authenticated( false )
    , m_isHelo( false )
    , m_replyBytes( 0 )
    , m_jobs()
    , m_current( GUCEF_NULL )
{GUCEF_TRACE;

    Initialize();
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::Initialize( void )
{GUCEF_TRACE;

    RegisterEvents();
}

/*-------------------------------------------------------------------------*/

CSmtpClient::~CSmtpClient()
{GUCEF_TRACE;

    for ( size_t i=0; i<m_jobs.size(); ++i )
        GUCEF_DELETE m_jobs[ i ];
    m_jobs.clear();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSmtpClient::GetProtocolName( void ) const
{GUCEF_TRACE;

    static const CORE::CString name = "SMTP";
    return name;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSmtpClient::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CSmtpClient";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CSmtpClientSettings&
CSmtpClient::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClient::Connect( const CSmtpClientSettings& settings )
{GUCEF_TRACE;

    if ( CLIENTSTATE_DISCONNECTED != GetState() )
        return false;

    m_settings = settings;
    m_reply = CSmtpReply();
    m_replyBytes = 0;
    m_expected.clear();
    m_extensions.clear();
    m_triedHelo = false;
    m_authenticated = false;
    m_isHelo = false;
    return BeginConnect( m_settings );
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpClient::GetPendingCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_jobs.size();
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClient::HasExtension( const CORE::CString& name ) const
{GUCEF_TRACE;

    return m_extensions.find( ToUpperToken( name ) ) != m_extensions.end();
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpClient::GetServerMaxSize( void ) const
{GUCEF_TRACE;

    GUCEF::map< CORE::CString, CORE::CString >::const_iterator i = m_extensions.find( "SIZE" );
    if ( i == m_extensions.end() )
        return 0;
    UInt64 value = 0;
    const char* digits = (*i).second.C_String();
    for ( ; *digits >= '0' && *digits <= '9' && value < 0xFFFFFFFFull; ++digits )
        value = value * 10 + (UInt64)( *digits - '0' );
    return value > 0xFFFFFFFFull ? 0xFFFFFFFFu : (UInt32) value;
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::PrepareData( const void* data, UInt32 size, CORE::CDynamicBuffer& stuffed )
{GUCEF_TRACE;

    stuffed.Clear();
    stuffed.SetGrowthStrategy( CORE::CDynamicBuffer::GROWTHSTRATEGY_GEOMETRIC );
    const char* p = static_cast< const char* >( data );

    char chunk[ 4096 ];
    UInt32 used = 0;
    bool lineStart = true;
    UInt32 i = 0;
    while ( i < size )
    {
        if ( used + 4 > sizeof( chunk ) )
        {
            stuffed.Append( chunk, used, true );
            used = 0;
        }
        const char c = p[ i ];
        if ( '\r' == c || '\n' == c )
        {
            if ( '\r' == c && i + 1 < size && '\n' == p[ i + 1 ] )
                ++i;
            chunk[ used++ ] = '\r';
            chunk[ used++ ] = '\n';
            lineStart = true;
            ++i;
            continue;
        }
        if ( lineStart && '.' == c )
            chunk[ used++ ] = '.';
        chunk[ used++ ] = c;
        lineStart = false;
        ++i;
    }
    if ( !lineStart )
    {
        chunk[ used++ ] = '\r';
        chunk[ used++ ] = '\n';
    }
    chunk[ used++ ] = '.';
    chunk[ used++ ] = '\r';
    chunk[ used++ ] = '\n';
    stuffed.Append( chunk, used, true );
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSmtpClient::SanitizeHeloName( const CORE::CString& name )
{GUCEF_TRACE;

    const char* p = name.C_String();
    const UInt32 size = ByteLength( name );
    if ( 0 == size || size > 255 )
        return "localhost";
    for ( UInt32 i=0; i<size; ++i )
    {
        const char c = p[ i ];
        const bool ok = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || '.' == c || '-' == c || '[' == c || ']' == c || ':' == c;
        if ( !ok )
            return "localhost";
    }
    return name;
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::Expect( EStep step )
{GUCEF_TRACE;

    m_expected.push_back( step );
    UpdateDeadline();
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::UpdateDeadline( void )
{GUCEF_TRACE;

    if ( m_expected.empty() )
    {
        if ( CLIENTSTATE_READY == GetState() )
            ClearDeadline();
        return;
    }
    if ( CLIENTSTATE_CONNECTING == GetState() && STEP_GREETING == m_expected.front() )
        return;     // the connect deadline is running
    if ( STEP_DATA_END == m_expected.front() )
        SetDeadline( m_settings.dataTimeoutInMs, "the reply to the end of the message data" );
    else
        SetDeadline( m_settings.commandTimeoutInMs, "a reply of the server" );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnConnectionEstablished( void )
{GUCEF_TRACE;

    m_expected.clear();
    m_expected.push_back( STEP_GREETING );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::SendEhlo( void )
{GUCEF_TRACE;

    m_extensions.clear();
    m_isHelo = false;
    SendCommandLine( "EHLO " + SanitizeHeloName( m_settings.heloName ) );
    Expect( STEP_EHLO );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnServerLine( const CORE::CString& line )
{GUCEF_TRACE;

    UInt32 code = 0;
    bool isLast = false;
    CORE::CString text;
    if ( !CSmtpReply::ParseLine( line, code, isLast, text ) )
    {
        Fail( MAILERR_PROTOCOL, "The server sent a line that is not an SMTP reply" );
        return;
    }
    if ( !m_reply.lines.empty() && code != m_reply.code )
    {
        Fail( MAILERR_PROTOCOL, "The lines of a multi line reply have different codes" );
        return;
    }
    m_reply.code = code;

    CORE::CString enhanced, rest;
    if ( m_reply.lines.empty() && CSmtpReply::SplitEnhancedCode( text, code, enhanced, rest ) )
    {
        m_reply.enhancedCode = enhanced;
        text = rest;
    }
    m_replyBytes += ByteLength( text );
    if ( m_reply.lines.size() >= MaxReplyLines || m_replyBytes > MaxReplyBytes )
    {
        Fail( MAILERR_PROTOCOL, "A reply of the server is larger than the limit" );
        return;
    }
    m_reply.lines.push_back( text );
    if ( !isLast )
        return;

    CSmtpReply reply = m_reply;
    m_reply = CSmtpReply();
    m_replyBytes = 0;
    OnReply( reply );

    // a server that says more than was asked for is out of step with us, or is not what it seems
    if ( m_expected.empty() && CLIENTSTATE_DISCONNECTED != GetState() && GetTransport().HasBufferedData() )
        Fail( MAILERR_PROTOCOL, "The server sent more than was asked for" );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::ParseExtensions( const CSmtpReply& reply )
{GUCEF_TRACE;

    m_extensions.clear();
    // the first line is the greeting of the server, the others are the extensions
    for ( size_t i=1; i<reply.lines.size(); ++i )
    {
        CORE::CString line = reply.lines[ i ].Trim( true ).Trim( false );
        if ( line.IsNULLOrEmpty() )
            continue;
        CORE::CString keyword = line;
        CORE::CString parameters;
        Int32 space = line.HasChar( ' ' );
        if ( space > 0 )
        {
            keyword = line.SubstrToIndex( (UInt32) space, true );
            parameters = line.SubstrFromRange( (UInt32) space + 1, line.Length() );
        }
        keyword = keyword.Uppercase();

        // the older form "AUTH=PLAIN LOGIN" is the same extension
        if ( keyword.StartsWith( "AUTH=" ) )
        {
            parameters = keyword.SubstrFromRange( 5, keyword.Length() ) + " " + parameters;
            keyword = "AUTH";
        }
        GUCEF::map< CORE::CString, CORE::CString >::iterator existing = m_extensions.find( keyword );
        if ( existing != m_extensions.end() )
            (*existing).second += " " + parameters;
        else
            m_extensions[ keyword ] = parameters;
        if ( m_extensions.size() > 100 )
            break;
    }
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnReply( const CSmtpReply& reply )
{GUCEF_TRACE;

    if ( m_expected.empty() )
    {
        if ( 421 == reply.code )
            Fail( MAILERR_TEMPORARY, "The server is closing the connection: " + CEMailProtocolLog::RedactServerLine( reply.GetText(), false ), reply.code, reply.enhancedCode );
        else
            Fail( MAILERR_PROTOCOL, "The server sent a reply that was not asked for" );
        return;
    }

    const EStep step = m_expected.front();
    m_expected.erase( m_expected.begin() );

    if ( 421 == reply.code )
    {
        Fail( MAILERR_TEMPORARY, "The server is closing the connection: " + CEMailProtocolLog::RedactServerLine( reply.GetText(), false ), reply.code, reply.enhancedCode );
        return;
    }

    switch ( step )
    {
        case STEP_GREETING:
        {
            if ( 220 == reply.code )
            {
                // the client has not said a word yet: there is nothing that can follow the greeting
                if ( GetTransport().HasBufferedData() )
                {
                    Fail( MAILERR_PROTOCOL, "The server sent more than the greeting before the client spoke" );
                    return;
                }
                SendEhlo();
            }
            else
                Fail( CSmtpReply::GetErrorClass( reply.code ), "The server refused the connection", reply.code, reply.enhancedCode );
            return;
        }
        case STEP_EHLO:
        {
            OnEhloReply( reply, false );
            return;
        }
        case STEP_HELO:
        {
            OnEhloReply( reply, true );
            return;
        }
        case STEP_STARTTLS:
        {
            if ( 220 != reply.code )
            {
                Fail( MAILERR_TLS, "The server refused STARTTLS", reply.code, reply.enhancedCode );
                return;
            }
            m_extensions.clear();       // what was offered in plain text means nothing from now on
            BeginTlsUpgrade();
            return;
        }
        case STEP_AUTH:
        {
            if ( 235 == reply.code )
            {
                OnSaslSucceeded();
                m_authenticated = true;
                SetReady();
                ProcessQueue();
            }
            else
            if ( 334 == reply.code )
            {
                OnSaslChallenge( reply.lines.empty() ? CORE::CString::Empty : reply.lines[ 0 ] );
            }
            else
            {
                OnSaslRejected( reply.GetText(), reply.code );
            }
            UpdateDeadline();
            return;
        }
        case STEP_MAIL:
        {
            if ( GUCEF_NULL == m_current )
                break;
            if ( 2 == reply.code / 100 )
            {
                if ( !( HasExtension( "PIPELINING" ) && m_settings.usePipelining ) )
                    SendNextRecipient();
            }
            else
            {
                m_current->mailFailed = true;
                SetJobFailure( CSmtpReply::GetErrorClass( reply.code ), &reply, "The server refused the sender" );
                if ( !( HasExtension( "PIPELINING" ) && m_settings.usePipelining ) )
                    CompleteCurrentJob( false, &reply );
            }
            UpdateDeadline();
            return;
        }
        case STEP_RCPT:
        {
            if ( GUCEF_NULL == m_current )
                break;
            CSmtpRecipientResult& result = m_current->results[ m_current->rcptReplies ];
            result.replyCode = reply.code;
            result.enhancedCode = reply.enhancedCode;
            result.replyText = reply.GetText();
            result.accepted = 250 == reply.code || 251 == reply.code;
            result.errorClass = result.accepted ? MAILERR_NONE : CSmtpReply::GetErrorClass( reply.code );
            ++m_current->rcptReplies;

            if ( m_current->rcptReplies >= m_current->recipients.size() )
                OnRecipientsDone();
            else
            if ( !( HasExtension( "PIPELINING" ) && m_settings.usePipelining ) )
                SendNextRecipient();
            UpdateDeadline();
            return;
        }
        case STEP_DATA:
        {
            if ( GUCEF_NULL == m_current )
                break;
            if ( 354 != reply.code )
            {
                SetJobFailure( CSmtpReply::GetErrorClass( reply.code ), &reply, "The server refused to take the message data" );
                SendCommandLine( "RSET" );
                Expect( STEP_RSET );
                return;
            }
            CORE::CDynamicBuffer stuffed;
            PrepareData( m_current->data.GetConstBufferPtr(), m_current->data.GetDataSize(), stuffed );
            SendDataBlock( stuffed.GetConstBufferPtr(), stuffed.GetDataSize(), "message data" );
            Expect( STEP_DATA_END );
            return;
        }
        case STEP_DATA_END:
        {
            if ( GUCEF_NULL == m_current )
                break;
            if ( 2 == reply.code / 100 )
                CompleteCurrentJob( true, &reply );
            else
            {
                SetJobFailure( CSmtpReply::GetErrorClass( reply.code ), &reply, "The server did not accept the message" );
                CompleteCurrentJob( false, &reply );
            }
            UpdateDeadline();
            return;
        }
        case STEP_RSET:
        {
            if ( GUCEF_NULL != m_current )
                CompleteCurrentJob( false, GUCEF_NULL );
            UpdateDeadline();
            return;
        }
    }
    Fail( MAILERR_PROTOCOL, "The server sent a reply that does not fit the conversation" );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnEhloReply( const CSmtpReply& reply, bool isHelo )
{GUCEF_TRACE;

    if ( 250 != reply.code )
    {
        // a server that does not know EHLO is asked with HELO, once
        if ( !isHelo && !m_triedHelo && ( 500 == reply.code || 501 == reply.code || 502 == reply.code || 504 == reply.code ) )
        {
            m_triedHelo = true;
            SendCommandLine( "HELO " + SanitizeHeloName( m_settings.heloName ) );
            Expect( STEP_HELO );
            return;
        }
        Fail( CSmtpReply::GetErrorClass( reply.code ), "The server did not accept our greeting", reply.code, reply.enhancedCode );
        return;
    }

    m_isHelo = isHelo;
    if ( isHelo )
        m_extensions.clear();
    else
        ParseExtensions( reply );

    // the TLS policy decides before anything else happens, and again after the handshake
    const ETlsAction action = DecideTls( HasExtension( "STARTTLS" ) );
    if ( TLSACTION_FAILED == action )
        return;
    if ( TLSACTION_STARTTLS == action )
    {
        SendCommandLine( "STARTTLS" );
        Expect( STEP_STARTTLS );
        return;
    }
    ProceedToLogin();
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnTlsEstablished( void )
{GUCEF_TRACE;

    // the capabilities that were offered before the handshake are not trusted: they are asked for again
    SendEhlo();
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::ProceedToLogin( void )
{GUCEF_TRACE;

    if ( WantsLogin() && !m_authenticated )
    {
        CORE::CString::StringVector mechanisms;
        GUCEF::map< CORE::CString, CORE::CString >::const_iterator auth = m_extensions.find( "AUTH" );
        if ( auth != m_extensions.end() )
            mechanisms = (*auth).second.ParseElements( ' ', false );
        if ( mechanisms.empty() )
        {
            Fail( MAILERR_AUTH, "The server does not offer authentication" );
            return;
        }
        BeginSasl( mechanisms, true );
        return;
    }
    SetReady();
    ProcessQueue();
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClient::SendSaslStart( const CORE::CString& mechanism, bool hasInitialResponse, const CORE::CString& base64InitialResponse )
{GUCEF_TRACE;

    const bool sent = SendCommandLine( "AUTH " + mechanism + ( hasInitialResponse ? " " + base64InitialResponse : CORE::CString::Empty ), false );
    Expect( STEP_AUTH );
    return sent;
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClient::SendSaslResponse( const CORE::CString& base64Response )
{GUCEF_TRACE;

    const bool sent = SendCommandLine( base64Response, true );
    Expect( STEP_AUTH );
    return sent;
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::SendSaslCancel( void )
{GUCEF_TRACE;

    SendCommandLine( "*", true );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::SendQuitCommand( void )
{GUCEF_TRACE;

    SendCommandLine( "QUIT" );
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpClient::SendMessage( const CEMailMessage& message, CORE::CString* error )
{GUCEF_TRACE;

    CORE::CString reason;
    CMimeComposeOptions options;
    options.allowUtf8Addresses = m_settings.allowSmtpUtf8;
    options.maxSize = m_settings.maxMessageSize;
    CMimeComposer composer( options );
    CORE::CDynamicBuffer data;
    if ( !composer.Compose( message, data, reason ) )
    {
        if ( GUCEF_NULL != error )
            *error = reason;
        return 0;
    }

    CEMailAddress::TEMailAddressVector recipients;
    message.GetEnvelopeRecipients( recipients );
    const CEMailAddress& sender = message.sender.IsEmpty() ? message.from[ 0 ] : message.sender;

    bool needsUtf8 = sender.IsInternational();
    for ( size_t i=0; i<recipients.size(); ++i )
        needsUtf8 = needsUtf8 || recipients[ i ].IsInternational();
    return QueueJob( sender.GetAddress(), recipients, data.GetConstBufferPtr(), data.GetDataSize(), needsUtf8, error );
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpClient::SendRaw( const CEMailAddress& envelopeFrom, const CEMailAddress::TEMailAddressVector& recipients, const void* data, UInt32 size, CORE::CString* error )
{GUCEF_TRACE;

    bool needsUtf8 = envelopeFrom.IsInternational();
    for ( size_t i=0; i<recipients.size(); ++i )
        needsUtf8 = needsUtf8 || recipients[ i ].IsInternational();
    return QueueJob( envelopeFrom.GetAddress(), recipients, data, size, needsUtf8, error );
}

/*-------------------------------------------------------------------------*/

UInt32
CSmtpClient::QueueJob( const CORE::CString& envelopeFrom, const CEMailAddress::TEMailAddressVector& recipients, const void* data, UInt32 size, bool needsUtf8, CORE::CString* error )
{GUCEF_TRACE;

    #define GUCEF_COM_SMTP_REFUSE( text ) { if ( GUCEF_NULL != error ) *error = text; return 0; }

    if ( CLIENTSTATE_DISCONNECTED == GetState() )
        GUCEF_COM_SMTP_REFUSE( "The client is not connected" )
    if ( recipients.empty() )
        GUCEF_COM_SMTP_REFUSE( "The message has no recipients" )
    if ( recipients.size() > m_settings.maxRecipients )
        GUCEF_COM_SMTP_REFUSE( "The message has more recipients than the settings allow" )
    if ( size > m_settings.maxMessageSize )
        GUCEF_COM_SMTP_REFUSE( "The message is larger than the settings allow" )

    // the addresses of the envelope end up in commands: only valid addresses are used
    CEMailAddress from;
    if ( !envelopeFrom.IsNULLOrEmpty() )
    {
        from.SetAddress( envelopeFrom );
        if ( !from.IsValid( m_settings.allowSmtpUtf8 ) )
            GUCEF_COM_SMTP_REFUSE( "The sender is not a valid address" )
    }

    SJob* job = GUCEF_NEW SJob;
    job->requestId = NextRequestId();
    job->envelopeFrom = from.GetAddress();
    job->needsUtf8 = needsUtf8;
    job->isEightBit = false;
    job->mailFailed = false;
    job->rcptSent = 0;
    job->rcptReplies = 0;
    job->hasFailure = false;

    for ( size_t i=0; i<recipients.size(); ++i )
    {
        if ( !recipients[ i ].IsValid( m_settings.allowSmtpUtf8 ) )
        {
            GUCEF_DELETE job;
            GUCEF_COM_SMTP_REFUSE( "A recipient is not a valid address" )
        }
        job->recipients.push_back( recipients[ i ].GetAddress() );
        CSmtpRecipientResult result;
        result.address = recipients[ i ].GetAddress();
        job->results.push_back( result );
    }

    // the data: no NUL, and note whether 8 bit
    const char* bytes = static_cast< const char* >( data );
    for ( UInt32 i=0; i<size; ++i )
    {
        if ( '\0' == bytes[ i ] )
        {
            GUCEF_DELETE job;
            GUCEF_COM_SMTP_REFUSE( "The message data has a NUL byte" )
        }
        if ( ( (UInt8) bytes[ i ] ) >= 0x80 )
            job->isEightBit = true;
    }
    if ( size > 0 )
        job->data.CopyFrom( size, data );

    const UInt32 requestId = job->requestId;
    m_jobs.push_back( job );
    ProcessQueue();
    return requestId;

    #undef GUCEF_COM_SMTP_REFUSE
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::ProcessQueue( void )
{GUCEF_TRACE;

    if ( CLIENTSTATE_READY != GetState() || GUCEF_NULL != m_current || m_jobs.empty() )
        return;
    m_current = m_jobs.front();
    StartJob();
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::SetJobFailure( EMailErrorClass errorClass, const CSmtpReply* reply, const char* description )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_current || m_current->hasFailure )
        return;
    m_current->hasFailure = true;
    m_current->failure.errorClass = errorClass;
    m_current->failure.errorDescription = description;
    if ( GUCEF_NULL != reply )
    {
        m_current->failure.replyCode = reply->code;
        m_current->failure.enhancedCode = reply->enhancedCode;
        m_current->failure.replyText = reply->GetText();
    }
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::StartJob( void )
{GUCEF_TRACE;

    SJob* job = m_current;

    const UInt32 serverMax = GetServerMaxSize();
    if ( serverMax > 0 && job->data.GetDataSize() > serverMax )
    {
        SetJobFailure( MAILERR_LOCAL, GUCEF_NULL, "The message is larger than the server accepts" );
        CompleteCurrentJob( false, GUCEF_NULL );
        return;
    }
    if ( job->needsUtf8 && !( m_settings.allowSmtpUtf8 && HasExtension( "SMTPUTF8" ) ) )
    {
        SetJobFailure( MAILERR_LOCAL, GUCEF_NULL, "An address is international and the server does not offer SMTPUTF8" );
        CompleteCurrentJob( false, GUCEF_NULL );
        return;
    }
    if ( job->isEightBit && !HasExtension( "8BITMIME" ) )
    {
        SetJobFailure( MAILERR_LOCAL, GUCEF_NULL, "The message has 8 bit data and the server does not offer 8BITMIME" );
        CompleteCurrentJob( false, GUCEF_NULL );
        return;
    }

    CORE::CString mail = "MAIL FROM:<" + job->envelopeFrom + ">";
    if ( HasExtension( "SIZE" ) )
        mail += " SIZE=" + CORE::UInt32ToString( job->data.GetDataSize() );
    if ( job->isEightBit )
        mail += " BODY=8BITMIME";
    if ( job->needsUtf8 )
        mail += " SMTPUTF8";

    SendCommandLine( mail );
    Expect( STEP_MAIL );

    if ( HasExtension( "PIPELINING" ) && m_settings.usePipelining )
    {
        for ( size_t i=0; i<job->recipients.size(); ++i )
        {
            SendCommandLine( "RCPT TO:<" + job->recipients[ i ] + ">" );
            Expect( STEP_RCPT );
            ++job->rcptSent;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::SendNextRecipient( void )
{GUCEF_TRACE;

    SJob* job = m_current;
    if ( job->rcptSent >= job->recipients.size() )
        return;
    SendCommandLine( "RCPT TO:<" + job->recipients[ job->rcptSent ] + ">" );
    ++job->rcptSent;
    Expect( STEP_RCPT );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnRecipientsDone( void )
{GUCEF_TRACE;

    SJob* job = m_current;
    if ( job->mailFailed )
    {
        // the sender was refused: there is no transaction to reset
        CompleteCurrentJob( false, GUCEF_NULL );
        return;
    }

    UInt32 accepted = 0, rejected = 0;
    const CSmtpRecipientResult* firstRejection = GUCEF_NULL;
    for ( size_t i=0; i<job->results.size(); ++i )
    {
        if ( job->results[ i ].accepted )
            ++accepted;
        else
        {
            ++rejected;
            if ( GUCEF_NULL == firstRejection )
                firstRejection = &job->results[ i ];
        }
    }

    if ( 0 == accepted || ( rejected > 0 && m_settings.failIfAnyRecipientRejected ) )
    {
        CSmtpReply reply;
        if ( GUCEF_NULL != firstRejection )
        {
            reply.code = firstRejection->replyCode;
            reply.enhancedCode = firstRejection->enhancedCode;
            reply.lines.push_back( firstRejection->replyText );
        }
        SetJobFailure( GUCEF_NULL != firstRejection ? firstRejection->errorClass : MAILERR_PERMANENT, &reply, 0 == accepted ? "The server accepted none of the recipients" : "A recipient was refused" );
        SendCommandLine( "RSET" );
        Expect( STEP_RSET );
        return;
    }

    SendCommandLine( "DATA" );
    Expect( STEP_DATA );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::CompleteCurrentJob( bool success, const CSmtpReply* finalReply )
{GUCEF_TRACE;

    SJob* job = m_current;
    m_current = GUCEF_NULL;
    CompleteJob( job, success, finalReply );
    ProcessQueue();
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::CompleteJob( SJob* job, bool success, const CSmtpReply* finalReply )
{GUCEF_TRACE;

    // out of the queue first: an observer may do anything
    for ( size_t i=0; i<m_jobs.size(); ++i )
    {
        if ( m_jobs[ i ] == job )
        {
            m_jobs.erase( m_jobs.begin() + i );
            break;
        }
    }

    CSmtpSendResult result;
    if ( job->hasFailure && !success )
        result = job->failure;
    result.requestId = job->requestId;
    result.success = success;
    result.recipients = job->results;
    if ( success )
    {
        result.errorClass = MAILERR_NONE;
        if ( GUCEF_NULL != finalReply )
        {
            result.replyCode = finalReply->code;
            result.enhancedCode = finalReply->enhancedCode;
            result.replyText = finalReply->GetText();
        }
    }
    else
    if ( !job->hasFailure )
    {
        result.errorClass = MAILERR_PROTOCOL;
        result.errorDescription = "The message could not be sent";
    }
    GUCEF_DELETE job;

    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "SMTP " << m_settings.host << ": message " << result.requestId << ( success ? " accepted" : " not sent: " ) << ( success ? CORE::CString() : result.errorDescription ) );
    NotifyObservers( SendCompletedEvent, &result );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::FailAllJobs( EMailErrorClass errorClass, const CORE::CString& description )
{GUCEF_TRACE;

    m_current = GUCEF_NULL;
    while ( !m_jobs.empty() )
    {
        SJob* job = m_jobs.front();
        job->hasFailure = true;
        job->failure.errorClass = errorClass;
        job->failure.errorDescription = description;
        CompleteJob( job, false, GUCEF_NULL );
    }
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnClientFailed( const CEMailErrorEventData& error )
{GUCEF_TRACE;

    m_expected.clear();
    FailAllJobs( error.errorClass, error.description );
}

/*-------------------------------------------------------------------------*/

void
CSmtpClient::OnClientClosed( void )
{GUCEF_TRACE;

    m_expected.clear();
    FailAllJobs( MAILERR_CANCELLED, "The connection was closed before the message was sent" );
}

/*-------------------------------------------------------------------------*/

bool
CSmtpClient::SendMessageSync( const CSmtpClientSettings& settings, const CEMailMessage& message, CSmtpSendResult& result, CIEMailCredentialProvider* credentialProvider )
{GUCEF_TRACE;

    // everything below happens on the calling thread, on a pulse generator that this thread owns
    CORE::PulseGeneratorPtr pulseGenerator = CreateLocalPulseGenerator();
    CSmtpClient client( pulseGenerator );
    CSmtpSyncSink sink;
    sink.Attach( client );
    if ( GUCEF_NULL != credentialProvider )
        client.SetCredentialProvider( credentialProvider );

    result = CSmtpSendResult();
    if ( !client.Connect( settings ) )
    {
        result.errorClass = client.GetLastError().errorClass;
        result.errorDescription = client.GetLastError().description;
        return false;
    }
    CORE::CString error;
    if ( 0 == client.SendMessage( message, &error ) )
    {
        client.Disconnect();
        result.errorClass = MAILERR_LOCAL;
        result.errorDescription = error;
        return false;
    }

    const UInt32 timeout = settings.connectTimeoutInMs + settings.dataTimeoutInMs + 4 * settings.commandTimeoutInMs + 5000;
    RunPulseUntil( pulseGenerator, sink.done, timeout );
    if ( !sink.done )
    {
        result.errorClass = MAILERR_TIMEOUT;
        result.errorDescription = "The message was not sent in time";
    }
    else
        result = sink.result;
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
