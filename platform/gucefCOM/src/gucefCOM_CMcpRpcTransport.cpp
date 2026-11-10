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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#include "gucefCOMCORE_CRpcContractBuilder.h"
#define GUCEF_COMCORE_CRPCCONTRACTBUILDER_H
#endif /* GUCEF_COMCORE_CRPCCONTRACTBUILDER_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVICEIMPL_H
#include "gucefCOMCORE_CRpcServiceImpl.h"
#define GUCEF_COMCORE_CRPCSERVICEIMPL_H
#endif /* GUCEF_COMCORE_CRPCSERVICEIMPL_H ? */

#ifndef GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#include "gucefCOMCORE_CTRpcMethodHandler.h"
#define GUCEF_COMCORE_CTRPCMETHODHANDLER_H
#endif /* GUCEF_COMCORE_CTRPCMETHODHANDLER_H ? */

#ifndef GUCEF_COMCORE_CRPCSERVERCALL_H
#include "gucefCOMCORE_CRpcServerCall.h"
#define GUCEF_COMCORE_CRPCSERVERCALL_H
#endif /* GUCEF_COMCORE_CRPCSERVERCALL_H ? */

#ifndef GUCEF_COM_CMCPSERVER_H
#include "gucefCOM_CMcpServer.h"
#define GUCEF_COM_CMCPSERVER_H
#endif /* GUCEF_COM_CMCPSERVER_H ? */

#include "gucefCOM_CMcpRpcTransport.h"

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

/** One Exchange call: the MCP channel that writes to it */
class CMcpRpcTransport::CRpcChannel : public CMcpChannel
{
    public:

    CRpcChannel( const CMcpPrincipal& principal, CMcpRpcTransport& transport, const COMCORE::CRpcServerCallPtr& call,
                 UInt32 maxPendingMessages, UInt32 maxPendingBytes )
        : CMcpChannel( CHANNEL_STREAM, principal ), m_transport( &transport ), m_call( call ), m_pending(), m_pendingHead( 0 )
        , m_pendingBytes( 0 ), m_maxPendingMessages( maxPendingMessages ), m_maxPendingBytes( maxPendingBytes ), m_overflowed( false )
    {}

    virtual ~CRpcChannel() GUCEF_VIRTUAL_OVERRIDE {}

    COMCORE::CRpcServerCallPtr GetCall( void ) const { return m_call; }

    void Detach( void ) { m_transport = GUCEF_NULL; }

    UInt32 GetPendingCount( void ) const { return (UInt32) m_pending.size() - m_pendingHead; }

    /** Credit came back: what waited goes now, in order */
    void FlushPending( void )
    {
        while ( m_pendingHead < m_pending.size() && !m_call.IsNULL() )
        {
            if ( !m_call->Write( m_pending[ m_pendingHead ].message ) )
                break;
            m_pendingBytes -= m_pending[ m_pendingHead ].bytes;
            m_pending[ m_pendingHead ].message = COMCORE::CRpcMessage();
            ++m_pendingHead;
        }

        // the written ones are taken off the front now and then, not one by one
        if ( m_pendingHead == m_pending.size() )
        {
            m_pending.clear();
            m_pendingHead = 0;
        }
        else if ( m_pendingHead > 64 && m_pendingHead * 2 > m_pending.size() )
        {
            m_pending.erase( m_pending.begin(), m_pending.begin() + m_pendingHead );
            m_pendingHead = 0;
        }
    }

    protected:

    virtual bool SendMessageText( const CORE::CString& messageText ) GUCEF_VIRTUAL_OVERRIDE
    {
        if ( m_overflowed || m_call.IsNULL() || m_call->IsDone() )
            return false;
        COMCORE::CRpcMessage message = m_call->NewResponse();
        message.SetString( 1, messageText );
        if ( 0 == GetPendingCount() && m_call->Write( message ) )
            return true;

        // no credit: the client reads slower than the server writes; it holds back only this channel, within the
        // bounds of the transport (0 for no maximum). A client beyond them does not read: its call ends with
        // RESOURCE_EXHAUSTED and the channel closes at the next pulse
        UInt32 bytes = messageText.ByteSize();
        if ( ( 0 != m_maxPendingMessages && GetPendingCount() >= m_maxPendingMessages ) ||
             ( 0 != m_maxPendingBytes && m_pendingBytes + bytes > m_maxPendingBytes ) )
        {
            GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "McpRpcTransport: channel " << GetChannelId() << " ends: " << GetPendingCount() <<
                               " messages (" << m_pendingBytes << " bytes) wait for a client that does not read them" );
            m_overflowed = true;
            m_pending.clear();
            m_pendingHead = 0;
            m_pendingBytes = 0;
            m_call->Finish( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, "the client does not read the messages of its MCP channel" ) );
            if ( GUCEF_NULL != m_transport )
                m_transport->OnChannelOverflow( *this );
            return false;
        }
        SPending pending;
        pending.message = message;
        pending.bytes = bytes;
        m_pending.push_back( pending );
        m_pendingBytes += bytes;
        return true;
    }

    virtual void OnChannelClosed( void ) GUCEF_VIRTUAL_OVERRIDE
    {
        // the server closed the channel: the call ends with OK
        if ( !m_call.IsNULL() && !m_call->IsDone() )
            m_call->Finish( COMCORE::CRpcStatus::Ok );
        if ( GUCEF_NULL != m_transport )
            m_transport->OnChannelEnded( *this );
    }

    private:

    struct SPending
    {
        COMCORE::CRpcMessage message;
        UInt32 bytes;
    };

    CMcpRpcTransport* m_transport;
    COMCORE::CRpcServerCallPtr m_call;
    GUCEF::vector< SPending > m_pending;
    UInt32 m_pendingHead;                   /**< the first one not written yet */
    UInt32 m_pendingBytes;
    UInt32 m_maxPendingMessages;
    UInt32 m_maxPendingBytes;
    bool m_overflowed;
};

/*-------------------------------------------------------------------------*/

/** The implementation of gucef.mcp.Session: every Exchange call opens a channel at the MCP server */
class CMcpRpcTransport::CSessionService : public COMCORE::CRpcServiceImpl ,
                                          public CORE::CObserver
{
    public:

    typedef COMCORE::CTRpcMethodHandler< CSessionService > THandler;
    typedef GUCEF::map< COMCORE::CRpcServerCall*, CMcpChannelPtr > TChannelMap;

    explicit CSessionService( CMcpRpcTransport& transport )
        : COMCORE::CRpcServiceImpl(), CORE::CObserver(), m_transport( &transport ), m_lock(), m_channels(), m_closing()
    {
        BindMethod( "Exchange", GUCEF_NEW THandler( this, &CSessionService::OnExchange ) );
    }

    /** At Start(): channels that ended inside a send are closed at a pulse of the server's pulse generator */
    void WatchPulses( CORE::PulseGeneratorPtr pulseGenerator )
    {
        if ( pulseGenerator.IsNULL() )
            return;
        typedef CORE::CTEventHandlerFunctor< CSessionService > TEventCallback;
        TEventCallback pulse( this, &CSessionService::OnPulse );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, pulse );
    }

    /** Inside the channel's send: the server may be in the middle of using it, it is closed at the next pulse */
    void CloseLater( CRpcChannel& channel )
    {
        MT::CScopeMutex lock( m_lock );
        m_closing.push_back( &channel );
    }

    virtual ~CSessionService() GUCEF_VIRTUAL_OVERRIDE
    {
        UnsubscribeAllFromObserver();
    }

    UInt32 GetChannelCount( void ) const { MT::CScopeMutex lock( m_lock ); return (UInt32) m_channels.size(); }

    /** Every channel closes, every call ends */
    void CloseAll( void )
    {
        TChannelMap channels;
        {
            MT::CScopeMutex lock( m_lock );
            channels.swap( m_channels );
            m_closing.clear();
        }
        TChannelMap::iterator i = channels.begin();
        for ( ; i != channels.end(); ++i )
        {
            CRpcChannel* channel = static_cast< CRpcChannel* >( (*i).second.GetPointerAlways() );
            channel->Detach();
            if ( GUCEF_NULL != m_transport->m_server )
                m_transport->m_server->CloseChannel( (*i).second );
            COMCORE::CRpcServerCallPtr call = channel->GetCall();
            if ( !call.IsNULL() && !call->IsDone() )
                call->Finish( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the MCP transport stopped" ) );
        }
        UnsubscribeAllFromObserver();
    }

    void Forget( CRpcChannel& channel )
    {
        MT::CScopeMutex lock( m_lock );
        TChannelMap::iterator i = m_channels.begin();
        for ( ; i != m_channels.end(); ++i )
        {
            if ( (*i).second.GetPointerAlways() == &channel )
            {
                m_channels.erase( i );
                return;
            }
        }
    }

    COMCORE::TRpcHandlerResult OnExchange( COMCORE::CRpcServerCall& call )
    {
        CMcpServer* server = m_transport->m_server;
        if ( GUCEF_NULL == server )
        {
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_UNAVAILABLE, "the MCP server is not running" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }

        // the MCP principal is the RPC peer: the transport set it, never a message
        const COMCORE::CRpcPrincipal& rpcPrincipal = call.GetPrincipal();
        CMcpPrincipal principal( RpcTransportName, rpcPrincipal.subject, rpcPrincipal.isAuthenticated );
        CORE::CString::StringSet::const_iterator r = rpcPrincipal.roles.begin();
        for ( ; r != rpcPrincipal.roles.end(); ++r )
            principal.scopes.push_back( *r );

        CRpcChannel* channel = GUCEF_NEW CRpcChannel( principal, *m_transport, call.GetSharedPtr(), m_transport->m_maxPendingMessages,
                                                      m_transport->m_maxPendingBytes );
        CMcpChannelPtr channelPtr( channel );
        if ( !server->OpenChannel( channelPtr ) )
        {
            channel->Detach();
            call.Fail( COMCORE::CRpcStatus( COMCORE::RPC_STATUS_RESOURCE_EXHAUSTED, "the MCP server refused the channel" ) );
            return COMCORE::RPC_HANDLER_DONE;
        }
        {
            MT::CScopeMutex lock( m_lock );
            m_channels[ &call ] = channelPtr;
        }
        typedef CORE::CTEventHandlerFunctor< CSessionService > TEventCallback;
        TEventCallback message( this, &CSessionService::OnMessage );
        TEventCallback halfClosed( this, &CSessionService::OnEnd );
        TEventCallback cancelled( this, &CSessionService::OnEnd );
        TEventCallback writable( this, &CSessionService::OnWritable );
        SubscribeTo( &call, COMCORE::CRpcServerCall::MessageEvent, message );
        SubscribeTo( &call, COMCORE::CRpcServerCall::HalfClosedEvent, halfClosed );
        SubscribeTo( &call, COMCORE::CRpcServerCall::CancelledEvent, cancelled );
        SubscribeTo( &call, COMCORE::CRpcServerCall::WritableEvent, writable );
        if ( call.HasRequest() )
            channel->ProcessIncomingMessage( call.GetRequest().GetString( 1 ) );
        return COMCORE::RPC_HANDLER_DEFERRED;
    }

    private:

    CMcpChannelPtr Find( CORE::CNotifier* notifier ) const
    {
        MT::CScopeMutex lock( m_lock );
        TChannelMap::const_iterator i = m_channels.find( static_cast< COMCORE::CRpcServerCall* >( notifier ) );
        return i != m_channels.end() ? (*i).second : CMcpChannelPtr();
    }

    void OnMessage( CORE::CNotifier* notifier, const CORE::CEvent&, CORE::CICloneable* eventData )
    {
        CMcpChannelPtr channel = Find( notifier );
        if ( channel.IsNULL() || GUCEF_NULL == eventData )
            return;
        const COMCORE::CRpcMessage& message = static_cast< COMCORE::CRpcStreamMessageEventData* >( eventData )->message;
        channel->ProcessIncomingMessage( message.GetString( 1 ) );
    }

    void OnPulse( CORE::CNotifier*, const CORE::CEvent&, CORE::CICloneable* )
    {
        GUCEF::vector< CRpcChannel* > closing;
        {
            MT::CScopeMutex lock( m_lock );
            if ( m_closing.empty() )
                return;
            closing.swap( m_closing );
        }
        for ( UInt32 c=0; c<closing.size(); ++c )
        {
            CMcpChannelPtr channel;
            COMCORE::CRpcServerCall* call = GUCEF_NULL;
            {
                MT::CScopeMutex lock( m_lock );
                TChannelMap::iterator i = m_channels.begin();
                for ( ; i != m_channels.end(); ++i )
                {
                    if ( (*i).second.GetPointerAlways() == closing[ c ] )
                    {
                        channel = (*i).second;
                        call = (*i).first;
                        m_channels.erase( i );
                        break;
                    }
                }
            }
            if ( channel.IsNULL() )
                continue;
            UnsubscribeFrom( *call );
            if ( GUCEF_NULL != m_transport->m_server )
                m_transport->m_server->CloseChannel( channel );
        }
    }

    void OnWritable( CORE::CNotifier* notifier, const CORE::CEvent&, CORE::CICloneable* )
    {
        CMcpChannelPtr channel = Find( notifier );
        if ( !channel.IsNULL() )
            static_cast< CRpcChannel* >( channel.GetPointerAlways() )->FlushPending();
    }

    /** HALF_CLOSE or CANCEL: the channel closes */
    void OnEnd( CORE::CNotifier* notifier, const CORE::CEvent&, CORE::CICloneable* )
    {
        CMcpChannelPtr channel;
        {
            MT::CScopeMutex lock( m_lock );
            TChannelMap::iterator i = m_channels.find( static_cast< COMCORE::CRpcServerCall* >( notifier ) );
            if ( i == m_channels.end() )
                return;
            channel = (*i).second;
            m_channels.erase( i );
        }
        UnsubscribeFrom( *notifier );
        if ( GUCEF_NULL != m_transport->m_server )
            m_transport->m_server->CloseChannel( channel );
        COMCORE::CRpcServerCallPtr call = static_cast< CRpcChannel* >( channel.GetPointerAlways() )->GetCall();
        if ( !call.IsNULL() && !call->IsDone() )
            call->Finish( COMCORE::CRpcStatus::Ok );
    }

    CMcpRpcTransport* m_transport;
    mutable MT::CMutex m_lock;
    TChannelMap m_channels;
    GUCEF::vector< CRpcChannel* > m_closing;
};

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CString CMcpRpcTransport::RpcTransportName = "rpc";

/*-------------------------------------------------------------------------*/

COMCORE::CRpcContractPtr
CMcpRpcTransport::GetMcpContract( void )
{GUCEF_TRACE;

    static MT::CMutex contractLock;
    static COMCORE::CRpcContractPtr* contract = GUCEF_NULL;
    MT::CScopeMutex lock( contractLock );
    if ( GUCEF_NULL != contract )
        return *contract;
    COMCORE::CRpcContractBuilder builder( "gucef.mcp", 1, 0 );
    builder.SetDoc( "MCP messages between a relay and an MCP server, carried opaquely over RPC" );
    builder.AddType( "McpMessage", "One JSON-RPC message of MCP" );
    builder.AddField( "McpMessage", 1, "json", "string" );
    builder.AddService( "Session", 1 );
    builder.AddMethod( "Session", 1, "Exchange", COMCORE::RPC_METHODKIND_BIDI_STREAM, "McpMessage", "McpMessage" );
    builder.SetMethodProtection( "Session", "Exchange", COMCORE::RPC_PROTECTION_CONFIDENTIAL, COMCORE::RPC_PROTECTION_CONFIDENTIAL );
    builder.SetMethodInitialCredit( "Session", "Exchange", 64 );
    CORE::CString error;
    COMCORE::CRpcContractPtr built = builder.Build( error );
    if ( built.IsNULL() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_CRITICAL, "McpRpcTransport: gucef.mcp 1.0 cannot be built: " << error );
        return built;
    }
    contract = GUCEF_NEW COMCORE::CRpcContractPtr( built );
    return *contract;
}

/*-------------------------------------------------------------------------*/

CMcpRpcTransport::CMcpRpcTransport( void )
    : CIMcpTransport()
    , m_server( GUCEF_NULL )
    , m_services()
    , m_contract()
    , m_service( GUCEF_NULL )
    , m_active( false )
    , m_maxPendingMessages( DefaultMaxPendingMessages )
    , m_maxPendingBytes( DefaultMaxPendingBytes )
{GUCEF_TRACE;

    m_service = GUCEF_NEW CSessionService( *this );
}

/*-------------------------------------------------------------------------*/

CMcpRpcTransport::~CMcpRpcTransport()
{GUCEF_TRACE;

    Stop( false );
    GUCEF_DELETE m_service;
    m_service = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRpcTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COM::CMcpRpcTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CMcpRpcTransport::GetTransportName( void ) const
{GUCEF_TRACE;

    return RpcTransportName;
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcTransport::SetServiceSet( const COMCORE::CRpcServiceSetPtr& services )
{GUCEF_TRACE;

    m_services = services;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpRpcTransport::GetChannelCount( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_service ? m_service->GetChannelCount() : 0;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcTransport::Start( CMcpServer& server )
{GUCEF_TRACE;

    if ( m_active )
        return true;
    if ( m_services.IsNULL() )
    {
        CORE::CString problem = "McpRpcTransport: no RPC service set to register gucef.mcp.Session on (SetServiceSet())";
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, problem );
        CORE::TCloneableString eData( problem );
        NotifyObservers( TransportErrorEvent, &eData );
        return false;
    }
    m_contract = GetMcpContract();
    m_server = &server;
    if ( m_contract.IsNULL() || !m_services->RegisterService( m_contract, "Session", m_service, COMCORE::RPC_EXECUTION_NOTIFICATION_THREAD ) )
    {
        m_server = GUCEF_NULL;
        return false;
    }
    m_service->WatchPulses( server.GetPulseGenerator() );
    m_active = true;
    CORE::TCloneableString started( GetEndpointDescription() );
    NotifyObservers( StartedEvent, &started );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcTransport::Stop( bool graceful )
{GUCEF_TRACE;

    if ( !m_active )
        return;
    m_services->UnregisterService( m_contract, "Session" );
    m_service->CloseAll();
    m_active = false;
    m_server = GUCEF_NULL;
    NotifyObservers( StoppedEvent );
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcTransport::IsActive( void ) const
{GUCEF_TRACE;

    return m_active;
}

/*-------------------------------------------------------------------------*/

CORE::CString
CMcpRpcTransport::GetEndpointDescription( void ) const
{GUCEF_TRACE;

    return "rpc:gucef.mcp.Session/Exchange";
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcTransport::SetMaxPendingMessages( UInt32 maxMessages )
{GUCEF_TRACE;

    m_maxPendingMessages = maxMessages;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpRpcTransport::GetMaxPendingMessages( void ) const
{GUCEF_TRACE;

    return m_maxPendingMessages;
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcTransport::SetMaxPendingBytes( UInt32 maxBytes )
{GUCEF_TRACE;

    m_maxPendingBytes = maxBytes;
}

/*-------------------------------------------------------------------------*/

UInt32
CMcpRpcTransport::GetMaxPendingBytes( void ) const
{GUCEF_TRACE;

    return m_maxPendingBytes;
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcTransport::OnChannelOverflow( CRpcChannel& channel )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service )
        m_service->CloseLater( channel );
}

/*-------------------------------------------------------------------------*/

void
CMcpRpcTransport::OnChannelEnded( CRpcChannel& channel )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_service )
        m_service->Forget( channel );
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcTransport::SaveConfig( CORE::CDataNode& cfg ) const
{GUCEF_TRACE;

    cfg.SetAttribute( "maxPendingMessages", m_maxPendingMessages );
    cfg.SetAttribute( "maxPendingBytes", m_maxPendingBytes );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CMcpRpcTransport::LoadConfig( const CORE::CDataNode& cfg )
{GUCEF_TRACE;

    m_maxPendingMessages = cfg.GetAttributeValueOrChildValueByName( "maxPendingMessages" ).AsUInt32( m_maxPendingMessages, true );
    m_maxPendingBytes = cfg.GetAttributeValueOrChildValueByName( "maxPendingBytes" ).AsUInt32( m_maxPendingBytes, true );
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
