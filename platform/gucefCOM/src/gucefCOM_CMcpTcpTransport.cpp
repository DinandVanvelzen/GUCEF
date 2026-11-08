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

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_CIMONOTONICCLOCK_H
#include "gucefCORE_CIMonotonicClock.h"
#define GUCEF_CORE_CIMONOTONICCLOCK_H
#endif /* GUCEF_CORE_CIMONOTONICCLOCK_H ? */

#ifndef GUCEF_CORE_CRANDOM_H
#include "gucefCORE_CRandom.h"
#define GUCEF_CORE_CRANDOM_H
#endif /* GUCEF_CORE_CRANDOM_H ? */

#ifndef GUCEF_CORE_CSHA256_H
#include "gucefCORE_CSha256.h"
#define GUCEF_CORE_CSHA256_H
#endif /* GUCEF_CORE_CSHA256_H ? */

#ifndef GUCEF_CORE_CFILEACCESS_H
#include "CFileAccess.h"
#define GUCEF_CORE_CFILEACCESS_H
#endif /* GUCEF_CORE_CFILEACCESS_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefCOM_CMcpTcpTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COM {

/*-------------------------------------------------------------------------//
//                                                                         //
//      INTERNAL TYPES                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/**
 *  One connection: a stream channel that writes to the connection and that applies the checks of the TCP transport
 *  to what it receives
 */
class CMcpTcpTransport::CTcpChannel : public CMcpStreamChannel ,
                                      private CIMcpLineSink
{
    public:

    CTcpChannel( const CMcpPrincipal& principal, CMcpTcpTransport& transport, COMCORE::CTCPServerConnection& connection )
        : CMcpStreamChannel( principal, this )
        , CIMcpLineSink()
        , m_transport( &transport )
        , m_connection( &connection )
        , m_closeRequested( false )
        , m_tokenFailures( 0 )
        , m_verified( false )
    {
    }

    virtual ~CTcpChannel() GUCEF_VIRTUAL_OVERRIDE
    {
    }

    bool IsCloseRequested( void ) const
    {
        return m_closeRequested;
    }

    /** A request passed the checks of the transport: the token, when one is required */
    bool IsVerified( void ) const
    {
        return m_verified;
    }

    void Detach( void )
    {
        m_transport = GUCEF_NULL;
        m_connection = GUCEF_NULL;
        SetSink( GUCEF_NULL );
    }

    protected:

    /** Only a JSON object can be a message. The request line of an HTTP request is not one, and ends the connection */
    virtual bool OnLineReceived( const CORE::CString& line ) GUCEF_VIRTUAL_OVERRIDE
    {
        const char* text = line.C_String();
        const UInt32 size = ( line.ByteSize() > 0 ? line.ByteSize() - 1 : 0 );
        UInt32 i = 0;
        while ( i < size && ( ' ' == text[ i ] || '\t' == text[ i ] ) )
            ++i;
        if ( i < size && '{' == text[ i ] )
            return true;

        // what the peer sent is not logged: it can be anything
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport(" << GetChannelId() << "): A line that is no JSON object was received, the connection is closed" );
        RequestClose();
        return false;
    }

    virtual bool ValidateRequestHeaders( const SJsonRpcRequest& request ,
                                         const CMcpRequestMeta* meta    ,
                                         EMcpProtocolEra era            ,
                                         Int32& errorCode               ,
                                         CORE::CString& errorMessage    ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( GUCEF_NULL == m_transport || !m_transport->m_requireToken )
            return true;

        // the key is matched as a whole member name: it holds '/' and '.'
        CORE::CString given;
        if ( request.hasParams )
        {
            const CORE::CDataNode* metaNode = request.params.FindChild( "_meta" );
            if ( GUCEF_NULL != metaNode )
            {
                const CORE::CDataNode::TKeyValuePair* token = metaNode->GetAttribute( CMcpMetaKeys::GucefAuthToken );
                if ( GUCEF_NULL != token && (*token).second.IsString() )
                    given = (*token).second.AsString();
            }
        }
        if ( m_transport->IsTokenValid( given ) )
            return true;

        errorCode = MCP_ERROR_GUCEF_ACCESS_DENIED;
        errorMessage = "Access denied: a valid authentication token is required in _meta";
        ++m_tokenFailures;
        if ( m_tokenFailures >= MaxTokenFailuresPerConnection )
        {
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport(" << GetChannelId() << "): " << m_tokenFailures << " requests without a valid token, the connection is closed" );
            m_closeRequested = true;
        }
        return false;
    }

    /** The token of the request matched: the request is the authenticated principal's */
    virtual void CompleteRequestPrincipal( CMcpPrincipal& principal ) GUCEF_VIRTUAL_OVERRIDE
    {
        m_verified = true;
        if ( GUCEF_NULL != m_transport && m_transport->m_requireToken )
        {
            principal.subject = "local:tcp-token";
            principal.isAuthenticated = true;
        }
    }

    virtual bool SendMcpLine( const CORE::CString& line )
    {
        if ( GUCEF_NULL == m_connection )
            return false;
        // the message and its line end go out in one send
        CORE::CDynamicBuffer buffer;
        // bytes, not characters: CString::Length() counts characters in a UTF-8 build
        buffer.Append( line.C_String(), ( line.ByteSize() > 0 ? line.ByteSize() - 1 : 0 ) );
        buffer.Append( "\n", 1 );
        return m_connection->Send( buffer );
    }

    private:

    void RequestClose( void )
    {
        m_closeRequested = true;
        if ( GUCEF_NULL != m_transport )
            m_transport->OnChannelWantsClose( *this );
    }

    private:

    CMcpTcpTransport* m_transport;
    COMCORE::CTCPServerConnection* m_connection;
    bool m_closeRequested;
    UInt32 m_tokenFailures;
    bool m_verified;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CMcpTcpTransport::CMcpTcpTransport( void )
    : CIMcpTransport()
    , m_server( GUCEF_NULL )
    , m_socket( GUCEF_NULL )
    , m_connections()
    , m_active( false )
    , m_bindAddress( "127.0.0.1" )
    , m_port( 0 )
    , m_maxConnections( 8 )
    , m_requireToken( true )
    , m_tokenFile()
    , m_allowNonLoopback( false )
    , m_tlsSettings()
    , m_firstRequestTimeoutMs( DefaultFirstRequestTimeoutMs )
    , m_nextTimeoutCheckInMs( 0 )
    , m_pulseGenerator()
    , m_token()
{GUCEF_TRACE;

    ::memset( m_tokenDigest, 0, sizeof( m_tokenDigest ) );
}

/*-------------------------------------------------------------------------*/

CMcpTcpTransport::~CMcpTcpTransport()
{GUCEF_TRACE;

    SignalUpcomingObserverDestruction();
    UnsubscribeAllFromObserver();
    if ( m_active )
        Release();
    ::memset( m_tokenDigest, 0, sizeof( m_tokenDigest ) );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpTcpTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpTcpTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpTcpTransport::GetTransportName( void ) const
{GUCEF_TRACE;

    return TcpTransportName;
}

/*-------------------------------------------------------------------------*/

void CMcpTcpTransport::SetBindAddress( const CORE::CString& bindAddress ) {GUCEF_TRACE; m_bindAddress = bindAddress; }
void CMcpTcpTransport::SetPort( UInt16 port ) {GUCEF_TRACE; m_port = port; }
void CMcpTcpTransport::SetMaxConnections( UInt32 maxConnections ) {GUCEF_TRACE; m_maxConnections = maxConnections; }
void CMcpTcpTransport::SetRequireToken( bool requireToken ) {GUCEF_TRACE; m_requireToken = requireToken; }
void CMcpTcpTransport::SetTokenFile( const CORE::CString& tokenFile ) {GUCEF_TRACE; m_tokenFile = tokenFile; }
void CMcpTcpTransport::SetAllowNonLoopback( bool allowNonLoopback ) {GUCEF_TRACE; m_allowNonLoopback = allowNonLoopback; }
void CMcpTcpTransport::SetFirstRequestTimeoutMs( UInt32 timeoutInMs ) {GUCEF_TRACE; m_firstRequestTimeoutMs = timeoutInMs; }
void CMcpTcpTransport::SetTlsSettings( const COMCORE::CTlsServerSettings& settings ) {GUCEF_TRACE; m_tlsSettings = settings; }
const COMCORE::CTlsServerSettings& CMcpTcpTransport::GetTlsSettings( void ) const {GUCEF_TRACE; return m_tlsSettings; }

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::SetToken( const CORE::CString& token )
{GUCEF_TRACE;

    m_token = token;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpTcpTransport::GetToken( void ) const
{GUCEF_TRACE;

    return m_token;
}

/*-------------------------------------------------------------------------*/

UInt16
CMcpTcpTransport::GetBoundPort( void ) const
{GUCEF_TRACE;

    return m_active && GUCEF_NULL != m_socket ? m_socket->GetBoundPort() : 0;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpTcpTransport::GetConnectionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_connections.size();
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpTcpTransport::GetEndpointDescription( void ) const
{GUCEF_TRACE;

    return CORE::CString( m_tlsSettings.IsEnabled() ? "tls://" : "tcp://" ) + m_bindAddress + ":" + CORE::UInt32ToString( m_active ? (UInt32) GetBoundPort() : (UInt32) m_port );
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::IsLoopbackAddress( const CORE::CString& address )
{GUCEF_TRACE;

    // 127.0.0.0/8 in dotted text: four numbers, the first of which is 127
    if ( address.Length() < 7 || address.Length() > 15 )
        return false;
    CORE::CString::StringVector parts = address.ParseElements( '.', false );
    if ( 4 != parts.size() )
        return false;
    for ( size_t i=0; i<parts.size(); ++i )
    {
        if ( parts[ i ].IsNULLOrEmpty() || parts[ i ].Length() > 3 )
            return false;
        for ( UInt32 n=0; n<parts[ i ].Length(); ++n )
            if ( parts[ i ].C_String()[ n ] < '0' || parts[ i ].C_String()[ n ] > '9' )
                return false;
        if ( CORE::StringToInt32( parts[ i ] ) > 255 )
            return false;
    }
    return 127 == CORE::StringToInt32( parts[ 0 ] );
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::PrepareToken( CORE::CString& problem )
{GUCEF_TRACE;

    if ( !m_requireToken )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: No authentication token is required. Every process of this machine can use the server" );
        return true;
    }

    if ( m_token.IsNULLOrEmpty() && !m_tokenFile.IsNULLOrEmpty() )
    {
        // read only when nobody else can have put it there or read it; never through a symbolic link
        CORE::CDynamicBuffer content;
        CORE::CString fileProblem;
        bool missing = false;
        if ( CORE::ReadOwnerOnlyFile( m_tokenFile, content, fileProblem, &missing ) )
        {
            CORE::CString text( static_cast< const char* >( content.GetConstBufferPtr() ), content.GetDataSize() );
            m_token = text.Trim( true ).Trim( false );
        }
        else
        if ( !missing )
        {
            problem = "the token file is refused: " + fileProblem;
            return false;
        }
        else
        {
            // a first start: a random token, in a new file that is the owner's only from its creation on
            UInt8 random[ 32 ];
            if ( !CORE::CRandom::GetRandomBytes( random, sizeof( random ) ) )
            {
                problem = "no random bytes are available for a token";
                return false;
            }
            CORE::CString generated;
            CORE::Base64UrlEncode( random, sizeof( random ), generated, false );
            ::memset( random, 0, sizeof( random ) );

            if ( !CORE::CreateOwnerOnlyFile( m_tokenFile, generated.C_String(), ( generated.ByteSize() > 0 ? generated.ByteSize() - 1 : 0 ), fileProblem ) )
            {
                problem = "the token file cannot be made: " + fileProblem;
                return false;
            }
            m_token = generated;
        }
    }
    if ( m_token.IsNULLOrEmpty() )
    {
        problem = "a token is required and there is neither a token nor a token file";
        return false;
    }
    CORE::CSha256::Hash( m_token.C_String(), ( m_token.ByteSize() > 0 ? m_token.ByteSize() - 1 : 0 ), m_tokenDigest );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::IsTokenValid( const CORE::CString& given ) const
{GUCEF_TRACE;

    // both sides are hashed first, so that the comparison has the same duration whatever the length of the given token
    UInt8 givenDigest[ 32 ];
    CORE::CSha256::Hash( given.C_String(), ( given.ByteSize() > 0 ? given.ByteSize() - 1 : 0 ), givenDigest );
    const bool equal = CORE::CSha256::ConstantTimeEquals( givenDigest, m_tokenDigest, sizeof( givenDigest ) );
    ::memset( givenDigest, 0, sizeof( givenDigest ) );
    return equal && !given.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::Start( CMcpServer& server )
{GUCEF_TRACE;

    if ( m_active )
        return true;

    CORE::CString problem;
    if ( !IsLoopbackAddress( m_bindAddress ) && !m_tlsSettings.IsEnabled() )
    {
        if ( !m_allowNonLoopback )
        {
            problem = "McpTcpTransport: The bind address \"" + m_bindAddress + "\" is not a loopback address, which needs TLS (certificateChainPath, privateKeyPath) or allowNonLoopback for plain TCP";
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
            CORE::TCloneableString eData( problem );
            NotifyObservers( TransportErrorEvent, &eData );
            return false;
        }
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: Binding to \"" << m_bindAddress << "\", which is not a loopback address, without TLS" );
    }
    if ( !PrepareToken( problem ) )
    {
        problem = "McpTcpTransport: " + problem;
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        CORE::TCloneableString eData( problem );
        NotifyObservers( TransportErrorEvent, &eData );
        return false;
    }

    m_server = &server;
    // one connection more than allowed: a new client is always accepted, and room is made for it (OnClientConnected())
    m_socket = GUCEF_NEW COMCORE::CTCPServerSocket( server.GetPulseGenerator(), false, m_maxConnections + 1 );
    if ( !m_socket->SetBindAddress( m_bindAddress ) )
    {
        problem = "McpTcpTransport: The bind address \"" + m_bindAddress + "\" is not valid";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        CORE::TCloneableString eData( problem );
        NotifyObservers( TransportErrorEvent, &eData );
        Release();
        return false;
    }
    if ( !m_socket->SetTlsSettings( m_tlsSettings ) )
    {
        problem = "McpTcpTransport: TLS cannot be used with the certificate \"" + m_tlsSettings.GetCertificateChainPath() + "\" and the key \"" + m_tlsSettings.GetPrivateKeyPath() + "\", see the log";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        CORE::TCloneableString eData( problem );
        NotifyObservers( TransportErrorEvent, &eData );
        Release();
        return false;
    }

    typedef CORE::CTEventHandlerFunctor< CMcpTcpTransport > TEventCallback;
    TEventCallback connectedCallback( this, &CMcpTcpTransport::OnClientConnected );
    SubscribeTo( m_socket, COMCORE::CTCPServerSocket::ClientConnectedEvent, connectedCallback );
    TEventCallback errorCallback( this, &CMcpTcpTransport::OnSocketError );
    SubscribeTo( m_socket, COMCORE::CTCPServerSocket::ServerSocketErrorEvent, errorCallback );
    m_pulseGenerator = server.GetPulseGenerator();
    if ( !m_pulseGenerator.IsNULL() )
    {
        TEventCallback pulseCallback( this, &CMcpTcpTransport::OnPulse );
        SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulseCallback );
    }

    if ( !m_socket->ListenOnPort( m_port ) )
    {
        problem = "McpTcpTransport: Cannot listen on " + m_bindAddress + ":" + CORE::UInt32ToString( m_port );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        CORE::TCloneableString eData( problem );
        NotifyObservers( TransportErrorEvent, &eData );
        Release();
        return false;
    }

    m_active = true;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: Listening on " << GetEndpointDescription() );
    CORE::TCloneableString started( GetEndpointDescription() );
    NotifyObservers( StartedEvent, &started );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::Stop( bool graceful )
{GUCEF_TRACE;

    if ( m_active )
        Release();
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::CloseConnection( COMCORE::CTCPServerConnection* connection )
{GUCEF_TRACE;

    // The close ends in OnClientDisconnected(), which forgets the connection; the object is not used after this
    if ( GUCEF_NULL != connection )
        connection->Close();
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::Release( void )
{GUCEF_TRACE;

    // no new connections; the ones that are open go
    if ( GUCEF_NULL != m_socket )
        m_socket->Close();

    GUCEF::vector< COMCORE::CTCPServerConnection* > connections;
    for ( TConnectionMap::iterator i = m_connections.begin(); i != m_connections.end(); ++i )
        connections.push_back( (*i).first );
    for ( size_t i=0; i<connections.size(); ++i )
    {
        TConnectionMap::iterator found = m_connections.find( connections[ i ] );
        if ( found == m_connections.end() )
            continue;
        SConnectionRecord record = (*found).second;
        m_connections.erase( found );
        record.channel->Detach();
        if ( GUCEF_NULL != m_server )
            m_server->CloseChannel( record.channelPtr );
        UnsubscribeFrom( connections[ i ] );
        connections[ i ]->Close();
    }
    m_connections.clear();

    if ( GUCEF_NULL != m_socket )
    {
        UnsubscribeFrom( m_socket );
        GUCEF_DELETE m_socket;
        m_socket = GUCEF_NULL;
    }
    if ( !m_pulseGenerator.IsNULL() )
    {
        UnsubscribeFrom( m_pulseGenerator.GetPointerAlways() );
        m_pulseGenerator.Unlink();
    }
    const bool wasActive = m_active;
    m_active = false;
    m_server = GUCEF_NULL;
    ::memset( m_tokenDigest, 0, sizeof( m_tokenDigest ) );
    if ( wasActive )
        NotifyObservers( StoppedEvent );
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( 0 == m_firstRequestTimeoutMs || m_connections.empty() )
        return;
    const UInt64 now = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
    if ( now < m_nextTimeoutCheckInMs )
        return;
    m_nextTimeoutCheckInMs = now + ( m_firstRequestTimeoutMs / 4 < 250 ? ( m_firstRequestTimeoutMs / 4 > 10 ? m_firstRequestTimeoutMs / 4 : 10 ) : 250 );

    // a connection that holds a place without a request that passed the checks of the transport
    GUCEF::vector< COMCORE::CTCPServerConnection* > expired;
    for ( TConnectionMap::iterator i = m_connections.begin(); i != m_connections.end(); ++i )
    {
        if ( !(*i).second.held && !(*i).second.channel->IsVerified() && now - (*i).second.connectedAtInMs >= m_firstRequestTimeoutMs )
            expired.push_back( (*i).first );
    }
    for ( size_t i=0; i<expired.size(); ++i )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: A connection made no verified request within " << m_firstRequestTimeoutMs << " ms and is closed" );
        CloseConnection( expired[ i ] );
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::OnSocketError( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    const COMCORE::CTCPServerSocket::TServerSocketErrorEventData* error = static_cast< const COMCORE::CTCPServerSocket::TServerSocketErrorEventData* >( eventData );
    CORE::CString text = "McpTcpTransport: The server socket reported error " + CORE::Int32ToString( GUCEF_NULL != error ? error->GetData() : 0 );
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, text );
    CORE::TCloneableString eData( text );
    NotifyObservers( TransportErrorEvent, &eData );
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::OnClientConnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_server )
        return;
    const COMCORE::CTCPServerSocket::TClientConnectedEventData* eData = static_cast< const COMCORE::CTCPServerSocket::TClientConnectedEventData* >( eventData );
    COMCORE::CTCPServerConnection* connection = eData->GetData().connection;

    // a connection object can be reused for a new client: what was known of the old one is gone
    TConnectionMap::iterator stale = m_connections.find( connection );
    if ( stale != m_connections.end() )
    {
        SConnectionRecord old = (*stale).second;
        m_connections.erase( stale );
        old.channel->Detach();
        m_server->CloseChannel( old.channelPtr );
    }

    // not authenticated until the token of a request matched (CTcpChannel::CompleteRequestPrincipal())
    CMcpPrincipal principal( TcpTransportName, m_requireToken ? "local:tcp-unverified" : "local:tcp-anonymous", false );
    SConnectionRecord record;
    record.connection = connection;
    record.channel = GUCEF_NEW CTcpChannel( principal, *this, *connection );
    record.channelPtr = CMcpChannelPtr( record.channel );
    record.connectedAtInMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
    if ( !m_server->OpenChannel( record.channelPtr ) )
    {
        record.channel->Detach();
        connection->Close();
        return;
    }
    m_connections[ connection ] = record;

    // one too many: the oldest connection that never made a request that passed the checks makes room. A client
    // with the token is therefore always served, however many connections others hold open without one
    if ( m_connections.size() > m_maxConnections )
    {
        COMCORE::CTCPServerConnection* evict = GUCEF_NULL;
        UInt64 oldest = 0;
        for ( TConnectionMap::iterator i = m_connections.begin(); i != m_connections.end(); ++i )
        {
            if ( (*i).first == connection || (*i).second.channel->IsVerified() )
                continue;
            if ( GUCEF_NULL == evict || (*i).second.connectedAtInMs < oldest )
            {
                evict = (*i).first;
                oldest = (*i).second.connectedAtInMs;
            }
        }
        if ( GUCEF_NULL == evict )
            evict = connection;
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: More than " << m_maxConnections << " connections: the oldest without a verified request is closed" );
        if ( evict == connection )
        {
            // every place has a client that made a verified request: the new one waits for a place, as it waited in the
            // backlog of the socket before; the socket takes no further connection meanwhile (maxConnections + 1)
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: Every place has a served client: the new connection is held until one goes" );
            m_connections[ connection ].held = true;
        }
        else
        {
            CloseConnection( evict );
        }
    }

    typedef CORE::CTEventHandlerFunctor< CMcpTcpTransport > TEventCallback;
    TEventCallback dataCallback( this, &CMcpTcpTransport::OnClientData );
    SubscribeTo( connection, COMCORE::CTCPServerConnection::DataRecievedEvent, dataCallback );
    TEventCallback disconnectCallback( this, &CMcpTcpTransport::OnClientDisconnected );
    SubscribeTo( connection, COMCORE::CTCPServerConnection::DisconnectedEvent, disconnectCallback );
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::OnClientData( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    TConnectionMap::iterator found = m_connections.find( connection );
    if ( found == m_connections.end() )
        return;
    const COMCORE::CTCPServerConnection::TDataRecievedEventData* eData = static_cast< const COMCORE::CTCPServerConnection::TDataRecievedEventData* >( eventData );
    const CORE::CDynamicBuffer& received = eData->GetData();

    if ( (*found).second.held )
    {
        // kept until the connection has a place; no more than one message may wait
        SConnectionRecord& record = (*found).second;
        const UInt32 limit = ( GUCEF_NULL != m_server ? m_server->GetSettings().maxMessageBytes : 4194304 ) + 65536;
        if ( record.heldBytes.GetDataSize() + received.GetDataSize() > limit )
        {
            CloseConnection( connection );
            return;
        }
        record.heldBytes.Append( received.GetConstBufferPtr(), received.GetDataSize() );
        return;
    }

    CTcpChannel* channel = (*found).second.channel;
    CMcpChannelPtr keepAlive = (*found).second.channelPtr;
    channel->ProcessIncomingBytes( received.GetConstBufferPtr(), received.GetDataSize() );

    // the connection of a peer that broke a rule is closed once the read that showed it is done
    if ( channel->IsCloseRequested() )
        CloseConnection( connection );
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::OnChannelWantsClose( CTcpChannel& channel )
{GUCEF_TRACE;

    // the channel stops taking lines at once: the lines behind the one that broke a rule are never processed
    for ( TConnectionMap::iterator i = m_connections.begin(); i != m_connections.end(); ++i )
    {
        if ( (*i).second.channel == &channel )
        {
            if ( GUCEF_NULL != m_server )
                m_server->CloseChannel( (*i).second.channelPtr );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::OnClientDisconnected( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    COMCORE::CTCPServerConnection* connection = static_cast< COMCORE::CTCPServerConnection* >( notifier );
    TConnectionMap::iterator found = m_connections.find( connection );
    if ( found == m_connections.end() )
        return;
    SConnectionRecord record = (*found).second;
    m_connections.erase( found );
    record.channel->Detach();
    if ( GUCEF_NULL != m_server )
        m_server->CloseChannel( record.channelPtr );
    if ( !record.held )
        ReleaseHeldConnection();
}

/*-------------------------------------------------------------------------*/

void
CMcpTcpTransport::ReleaseHeldConnection( void )
{GUCEF_TRACE;

    // a place is free: a held connection gets it, with the time for its first request counted from now
    for ( TConnectionMap::iterator i = m_connections.begin(); i != m_connections.end(); ++i )
    {
        SConnectionRecord& record = (*i).second;
        if ( !record.held )
            continue;
        record.held = false;
        record.connectedAtInMs = CORE::CSystemMonotonicClock::Instance().GetMonotonicTimeInMs();
        COMCORE::CTCPServerConnection* connection = (*i).first;
        CTcpChannel* channel = record.channel;
        CMcpChannelPtr keepAlive = record.channelPtr;
        CORE::CDynamicBuffer pending;
        pending.Append( record.heldBytes.GetConstBufferPtr(), record.heldBytes.GetDataSize() );
        record.heldBytes.Clear( false );
        if ( pending.GetDataSize() > 0 )
            channel->ProcessIncomingBytes( pending.GetConstBufferPtr(), pending.GetDataSize() );
        if ( channel->IsCloseRequested() )
            CloseConnection( connection );
        return;
    }
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "type", CORE::CVariant( TcpTransportName ) );
    cfg.SetAttribute( "bindAddress", CORE::CVariant( m_bindAddress ) );
    cfg.SetAttribute( "port", (UInt32) m_port );
    cfg.SetAttribute( "maxConnections", m_maxConnections );
    cfg.SetAttribute( "requireToken", m_requireToken );
    cfg.SetAttribute( "tokenFile", CORE::CVariant( m_tokenFile ) );
    cfg.SetAttribute( "allowNonLoopback", m_allowNonLoopback );
    cfg.SetAttribute( "firstRequestTimeoutMs", m_firstRequestTimeoutMs );
    if ( m_tlsSettings.IsEnabled() )
        m_tlsSettings.SaveConfig( cfg );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpTcpTransport::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_bindAddress = cfg.GetAttributeValueOrChildValueByName( "bindAddress" ).AsString( m_bindAddress, true );
    const UInt32 port = cfg.GetAttributeValueOrChildValueByName( "port" ).AsUInt32( m_port, true );
    if ( port > 65535 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "McpTcpTransport: The port " << port << " is not valid" );
        return false;
    }
    m_port = (UInt16) port;
    m_maxConnections = cfg.GetAttributeValueOrChildValueByName( "maxConnections" ).AsUInt32( m_maxConnections, true );
    m_requireToken = cfg.GetAttributeValueOrChildValueByName( "requireToken" ).AsBool( m_requireToken, true );
    m_tokenFile = cfg.GetAttributeValueOrChildValueByName( "tokenFile" ).AsString( m_tokenFile, true );
    m_allowNonLoopback = cfg.GetAttributeValueOrChildValueByName( "allowNonLoopback" ).AsBool( m_allowNonLoopback, true );
    m_firstRequestTimeoutMs = cfg.GetAttributeValueOrChildValueByName( "firstRequestTimeoutMs" ).AsUInt32( m_firstRequestTimeoutMs, true );
    m_tlsSettings.LoadConfig( cfg );
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
