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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefCOM_CMcpStdioTransport.h"

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

const CORE::CString CMcpStdioTransport::ConfigSectionName = "McpStdioTransport";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpStdioTransport::CMcpStdioTransport( void )
    : CIMcpTransport()
    , CIMcpLineSink()
    , m_server( GUCEF_NULL )
    , m_reader( GUCEF_NULL )
    , m_writer( GUCEF_NULL )
    , m_relay( GUCEF_NULL )
    , m_channel( GUCEF_NULL )
    , m_channelPtr()
    , m_active( false )
    , m_claimedStdout( false )
    , m_peerGone( false )
    , m_hasCustomStreams( false )
    , m_inputStream()
    , m_outputStream()
    , m_ignoreSigPipe( false )
    , m_scopes()
    , m_maxQueuedOutputBytes( CORE::CStandardOutputWriter::DefaultMaxQueuedBytes )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CMcpStdioTransport::~CMcpStdioTransport()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
    if ( m_active )
        Release( false );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStdioTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpStdioTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpStdioTransport::GetTransportName( void ) const
{GUCEF_TRACE;

    return StdioTransportName;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::SetStreams( CORE::CStandardInputReader::TStreamHandle input, CORE::CStandardOutputWriter::TStreamHandle output )
{GUCEF_TRACE;

    m_inputStream = input;
    m_outputStream = output;
    m_hasCustomStreams = true;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::UseStandardStreams( void )
{GUCEF_TRACE;

    m_hasCustomStreams = false;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::SetIgnoreSigPipe( bool ignoreSigPipe )
{GUCEF_TRACE;

    m_ignoreSigPipe = ignoreSigPipe;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::GetIgnoreSigPipe( void ) const
{GUCEF_TRACE;

    return m_ignoreSigPipe;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::SetScopes( const CORE::CString::StringVector& scopes )
{GUCEF_TRACE;

    m_scopes = scopes;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::SetMaxQueuedOutputBytes( UInt32 maxQueuedBytes )
{GUCEF_TRACE;

    m_maxQueuedOutputBytes = maxQueuedBytes;
}

/*-------------------------------------------------------------------------*/

CMcpChannelPtr
CMcpStdioTransport::GetChannel( void ) const
{GUCEF_TRACE;

    return m_channelPtr;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::HasPeerGone( void ) const
{GUCEF_TRACE;

    return m_peerGone;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpStdioTransport::GetEndpointDescription( void ) const
{GUCEF_TRACE;

    return "stdio";
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::Start( CMcpServer& server )
{GUCEF_TRACE;

    if ( m_active )
        return true;

    CORE::CString problem;
    if ( !m_hasCustomStreams )
    {
        // standard output carries the protocol from here on: nothing else of the process may write to it
        if ( !CORE::CStandardOutputWriter::ClaimExclusive() )
        {
            problem = "McpStdioTransport: Standard output is claimed already, the stdio transport cannot start";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            CORE::TCloneableString eData( problem );
            NotifyObservers( TransportErrorEvent, &eData );
            return false;
        }
        m_claimedStdout = true;
    }
    if ( m_ignoreSigPipe )
        CORE::CStandardOutputWriter::SetSigPipeIgnoredProcessWide( true );

    m_server = &server;
    m_peerGone = false;
    const UInt32 maxLineBytes = server.GetSettings().maxMessageBytes;

    m_writer = m_hasCustomStreams ? GUCEF_NEW CORE::CStandardOutputWriter( m_maxQueuedOutputBytes, m_outputStream )
                                  : GUCEF_NEW CORE::CStandardOutputWriter( m_maxQueuedOutputBytes );
    m_reader = m_hasCustomStreams ? GUCEF_NEW CORE::CStandardInputReader( maxLineBytes, m_inputStream )
                                  : GUCEF_NEW CORE::CStandardInputReader( maxLineBytes );

    // what the two threads report is handled on the protocol thread, in order. The relay keeps events that come from the protocol
    // thread itself for the next pulse (false): the failure of a send is reported that way, and must not tear the channel down from inside the send
    m_relay = GUCEF_NEW CORE::CTSGNotifier( server.GetPulseGenerator(), false );
    typedef CORE::CTEventHandlerFunctor< CMcpStdioTransport > TEventCallback;
    TEventCallback lineCallback( this, &CMcpStdioTransport::OnLineReceived );
    m_relay->SubscribeTo( m_reader, CORE::CStandardInputReader::LineReceivedEvent, lineCallback );
    TEventCallback tooLongCallback( this, &CMcpStdioTransport::OnLineTooLong );
    m_relay->SubscribeTo( m_reader, CORE::CStandardInputReader::LineTooLongEvent, tooLongCallback );
    TEventCallback endCallback( this, &CMcpStdioTransport::OnEndOfInput );
    m_relay->SubscribeTo( m_reader, CORE::CStandardInputReader::EndOfInputEvent, endCallback );
    TEventCallback readErrorCallback( this, &CMcpStdioTransport::OnReadError );
    m_relay->SubscribeTo( m_reader, CORE::CStandardInputReader::ReadErrorEvent, readErrorCallback );
    TEventCallback brokenCallback( this, &CMcpStdioTransport::OnBrokenPipe );
    m_relay->SubscribeTo( m_writer, CORE::CStandardOutputWriter::BrokenPipeEvent, brokenCallback );
    m_relay->SubscribeTo( this, TransportErrorEvent, brokenCallback );

    CMcpPrincipal principal( StdioTransportName, "local:parent-process", true );
    principal.scopes = m_scopes;
    m_channel = GUCEF_NEW CMcpStreamChannel( principal, this );
    m_channelPtr = CMcpChannelPtr( m_channel );
    if ( !server.OpenChannel( m_channelPtr ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStdioTransport: The server did not open the channel" );
        Release( false );
        return false;
    }

    if ( !m_writer->Start() || !m_reader->Start() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStdioTransport: The reader or writer thread did not start" );
        Release( false );
        return false;
    }

    m_active = true;
    CORE::TCloneableString started( GetEndpointDescription() );
    NotifyObservers( StartedEvent, &started );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::Stop( bool graceful )
{GUCEF_TRACE;

    if ( m_active )
        Release( graceful );
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::Release( bool flush )
{GUCEF_TRACE;

    // no more input; what was queued for the output is written, for a graceful stop, before the channel goes
    if ( GUCEF_NULL != m_reader )
        m_reader->Stop();
    if ( GUCEF_NULL != m_relay )
    {
        GUCEF_DELETE m_relay;
        m_relay = GUCEF_NULL;
    }
    if ( GUCEF_NULL != m_writer )
        m_writer->Stop( flush );

    if ( !m_channelPtr.IsNULL() )
    {
        if ( GUCEF_NULL != m_server )
            m_server->CloseChannel( m_channelPtr );
        m_channel->SetSink( GUCEF_NULL );
        m_channelPtr = CMcpChannelPtr();
        m_channel = GUCEF_NULL;
    }

    GUCEF_DELETE m_reader;
    m_reader = GUCEF_NULL;
    GUCEF_DELETE m_writer;
    m_writer = GUCEF_NULL;
    if ( m_claimedStdout )
    {
        CORE::CStandardOutputWriter::ReleaseClaim();
        m_claimedStdout = false;
    }

    const bool wasActive = m_active;
    m_active = false;
    m_server = GUCEF_NULL;
    if ( wasActive )
        NotifyObservers( StoppedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::SendMcpLine( const CORE::CString& line )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_writer || m_peerGone )
        return false;
    if ( m_writer->WriteLine( line ) )
        return true;

    // the queue is full or the pipe broke: the transport ends, from the next pulse and not from inside this send
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStdioTransport: A message could not be written to the output, the transport ends" );
    CORE::TCloneableString eData( CORE::CString( "the output queue is full or the output broke" ) );
    NotifyObservers( TransportErrorEvent, &eData );
    return false;
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::OnLineReceived( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* line = static_cast< const CORE::TCloneableString* >( eventData );
    if ( GUCEF_NULL != line && GUCEF_NULL != m_channel && !m_peerGone )
        m_channel->ProcessIncomingLine( line->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::OnLineTooLong( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableUInt32* size = static_cast< const CORE::TCloneableUInt32* >( eventData );
    if ( GUCEF_NULL != size && GUCEF_NULL != m_channel && !m_peerGone )
        m_channel->ProcessMessageTooLarge( size->GetData() );
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::OnEndOfInput( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpStdioTransport: The input ended" );
    OnPeerGone( "the input ended" );
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::OnReadError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const CORE::TCloneableString* what = static_cast< const CORE::TCloneableString* >( eventData );
    CORE::CString reason = "the input failed: " + ( GUCEF_NULL != what ? what->GetData() : CORE::CString::Empty );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStdioTransport: " << reason );
    CORE::TCloneableString eData( reason );
    NotifyObservers( TransportErrorEvent, &eData );
    OnPeerGone( reason );
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::OnBrokenPipe( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    // from the writer when the pipe broke, or from this transport when a send failed
    if ( notifier != this )
    {
        const CORE::TCloneableString* what = static_cast< const CORE::TCloneableString* >( eventData );
        CORE::CString reason = "the output broke: " + ( GUCEF_NULL != what ? what->GetData() : CORE::CString::Empty );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpStdioTransport: " << reason );
        CORE::TCloneableString eData( reason );
        NotifyObservers( TransportErrorEvent, &eData );
    }
    OnPeerGone( "the output ended" );
}

/*-------------------------------------------------------------------------*/

void
CMcpStdioTransport::OnPeerGone( const CORE::CString& reason )
{GUCEF_TRACE;

    if ( m_peerGone || GUCEF_NULL == m_server )
        return;
    m_peerGone = true;

    // nothing is read or sent from here on, and the requests in flight are cancelled
    CMcpServer* server = m_server;
    server->CloseChannel( m_channelPtr );
    if ( server->GetSettings().stopOnStdinClosed )
        server->RequestShutdown( "stdio: " + reason );
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "type", CORE::CVariant( StdioTransportName ) );
    cfg.SetAttribute( "ignoreSigPipe", m_ignoreSigPipe );
    cfg.SetAttribute( "maxQueuedOutputBytes", m_maxQueuedOutputBytes );
    CORE::CString scopes;
    for ( size_t i=0; i<m_scopes.size(); ++i )
    {
        if ( i > 0 )
            scopes += ',';
        scopes += m_scopes[ i ];
    }
    cfg.SetAttribute( "scopes", CORE::CVariant( scopes ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpStdioTransport::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_ignoreSigPipe = cfg.GetAttributeValueOrChildValueByName( "ignoreSigPipe" ).AsBool( m_ignoreSigPipe, true );
    m_maxQueuedOutputBytes = cfg.GetAttributeValueOrChildValueByName( "maxQueuedOutputBytes" ).AsUInt32( m_maxQueuedOutputBytes, true );
    CORE::CString::StringVector scopes;
    if ( cfg.GetStringListByName( "scopes", scopes ) )
        m_scopes = scopes;
    return true;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COM */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
