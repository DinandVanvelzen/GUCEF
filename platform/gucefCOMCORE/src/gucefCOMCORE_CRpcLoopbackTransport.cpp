/*
 *  gucefCOMCORE: GUCEF module providing basic communication facilities
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

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#include "gucefCORE_CTEventHandlerFunctor.h"
#define GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H
#endif /* GUCEF_CORE_CTEVENTHANDLERFUNCTOR_H ? */

#ifndef GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#include "gucefCOMCORE_CRpcLoopbackTransportListener.h"
#define GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H
#endif /* GUCEF_COMCORE_CRPCLOOPBACKTRANSPORTLISTENER_H ? */

#include "gucefCOMCORE_CRpcLoopbackTransport.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

/** The connection between the two sides of a loopback pair */
class CRpcLoopbackLink : public CORE::CTSharedObjCreator< CRpcLoopbackLink, MT::CMutex >
{
    public:

    MT::CMutex lock;
    CRpcLoopbackTransport* sides[ 2 ];

    CRpcLoopbackLink( void )
        : CORE::CTSharedObjCreator< CRpcLoopbackLink, MT::CMutex >( this )
        , lock()
    {
        sides[ 0 ] = GUCEF_NULL;
        sides[ 1 ] = GUCEF_NULL;
    }

    virtual ~CRpcLoopbackLink() {}

    CRpcLoopbackTransport* PeerOf( const CRpcLoopbackTransport* side ) const
    {
        if ( sides[ 0 ] == side )
            return sides[ 1 ];
        if ( sides[ 1 ] == side )
            return sides[ 0 ];
        return GUCEF_NULL;
    }

    void Remove( const CRpcLoopbackTransport* side )
    {
        if ( sides[ 0 ] == side )
            sides[ 0 ] = GUCEF_NULL;
        if ( sides[ 1 ] == side )
            sides[ 1 ] = GUCEF_NULL;
    }
};

/*-------------------------------------------------------------------------*/

const CORE::CString CRpcLoopbackTransport::TransportType = "loopback";

/*-------------------------------------------------------------------------*/

CRpcLoopbackTransport::CRpcLoopbackTransport( void )
    : CIRpcTransport()
    , m_notifierLock()
    , m_queueLock()
    , m_queue()
    , m_free()
    , m_link()
    , m_pulseGenerator()
    , m_task()
    , m_threadPoolName( "gucef.rpc.loopback" )
    , m_pulseObserver()
    , m_properties()
    , m_principal( TransportType, "local:in-process", true )
    , m_remoteUri()
    , m_connected( false )
    , m_closed( false )
    , m_pulsesRequested( false )
    , m_deliveryHeld( false )
{GUCEF_TRACE;

    m_properties.isReliable = true;
    m_properties.isOrdered = true;
    m_properties.isSecure = true;
    m_properties.hasFrameProtectionClasses = false;
    m_properties.maxFrameSize = 1024 * 1024;
    m_properties.latencyClass = CRpcTransportProperties::LATENCY_LOCAL;
    m_properties.profileName = TransportType;
}

/*-------------------------------------------------------------------------*/

CRpcLoopbackTransport::~CRpcLoopbackTransport()
{GUCEF_TRACE;

    m_pulseObserver.UnsubscribeAllFromObserver();
    if ( !m_pulseGenerator.IsNULL() )
        m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
    DisconnectFromPeer( true );
    if ( !m_task.IsNULL() )
        m_task->Stop();

    MT::CScopeMutex lock( m_queueLock );
    for ( UInt32 i=0; i<m_queue.size(); ++i )
        GUCEF_DELETE m_queue[ i ];
    m_queue.clear();
    for ( UInt32 i=0; i<m_free.size(); ++i )
        GUCEF_DELETE m_free[ i ];
    m_free.clear();
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::SetMaxFrameSize( UInt32 maxFrameSize )
{GUCEF_TRACE;

    m_properties.maxFrameSize = maxFrameSize;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::SetPrincipalRoles( const CORE::CString::StringSet& roles )
{GUCEF_TRACE;

    m_principal.roles = roles;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::SetThreadPoolName( const CORE::CString& poolName )
{GUCEF_TRACE;

    m_threadPoolName = poolName;
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransport::SetPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( IsConnected() )
        return false;
    m_pulseObserver.UnsubscribeAllFromObserver();
    m_pulseGenerator = pulseGenerator;
    SubscribeToPulses();
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::SubscribeToPulses( void )
{GUCEF_TRACE;

    if ( m_pulseGenerator.IsNULL() )
        return;
    typedef CORE::CTEventHandlerFunctor< CRpcLoopbackTransport > TEventCallback;
    TEventCallback callback( this, &CRpcLoopbackTransport::OnPulse );
    m_pulseObserver.SubscribeTo( m_pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent, callback );
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransport::EnsurePulseGenerator( void )
{GUCEF_TRACE;

    if ( !m_pulseGenerator.IsNULL() )
        return true;

    // a transport of its own thread: a dedicated task of the task manager, no raw thread
    m_task = CRpcProtocolTask::Start( m_threadPoolName );
    if ( m_task.IsNULL() )
        return false;
    m_pulseGenerator = m_task->GetPulseGenerator();
    SubscribeToPulses();
    return !m_pulseGenerator.IsNULL();
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::Join( CRpcLoopbackTransport& a, CRpcLoopbackTransport& b, const CORE::CUri& uriOfB, const CORE::CUri& uriOfA )
{GUCEF_TRACE;

    CRpcLoopbackLink* link = GUCEF_NEW CRpcLoopbackLink();
    CORE::CTSharedPtr< CRpcLoopbackLink, MT::CMutex > linkPtr = link->CreateSharedPtr();
    link->sides[ 0 ] = &a;
    link->sides[ 1 ] = &b;
    {
        MT::CScopeMutex lock( a.m_queueLock );
        a.m_link = linkPtr;
        a.m_remoteUri = uriOfB;
        a.m_connected = true;
        a.m_closed = false;
    }
    {
        MT::CScopeMutex lock( b.m_queueLock );
        b.m_link = linkPtr;
        b.m_remoteUri = uriOfA;
        b.m_connected = true;
        b.m_closed = false;
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransport::CreatePair( const CORE::PulseGeneratorPtr& pulseGeneratorA, const CORE::PulseGeneratorPtr& pulseGeneratorB,
                                   CIRpcTransportPtr& a, CIRpcTransportPtr& b )
{GUCEF_TRACE;

    CORE::CTSharedPtr< CRpcLoopbackTransport, MT::CMutex > sharedA( GUCEF_NEW CRpcLoopbackTransport() );
    CORE::CTSharedPtr< CRpcLoopbackTransport, MT::CMutex > sharedB( GUCEF_NEW CRpcLoopbackTransport() );
    CRpcLoopbackTransport* sideA = sharedA.GetPointerAlways();
    CRpcLoopbackTransport* sideB = sharedB.GetPointerAlways();
    a = sharedA;
    b = sharedB;
    sideA->SetPulseGenerator( pulseGeneratorA );
    sideB->SetPulseGenerator( pulseGeneratorB );
    if ( !sideA->EnsurePulseGenerator() || !sideB->EnsurePulseGenerator() )
        return false;
    Join( *sideA, *sideB, CORE::CUri( "loopback://pair-b" ), CORE::CUri( "loopback://pair-a" ) );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcLoopbackTransport::GetTransportType( void ) const
{GUCEF_TRACE;

    return TransportType;
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransport::Connect( const CORE::CUri& remote )
{GUCEF_TRACE;

    if ( IsConnected() )
        return true;
    if ( remote.GetScheme() != TransportType )
        return false;
    if ( !EnsurePulseGenerator() )
        return false;
    CORE::CString name = remote.GetAuthorityAndPath();
    if ( !CRpcLoopbackTransportListener::ConnectTo( name, *this ) )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "RpcLoopbackTransport: nobody listens on loopback://" << name );
        return false;
    }
    // ConnectedEvent on this transport's own thread, never inside the caller
    Enqueue( GUCEF_NULL, 0, false, true );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::DisconnectFromPeer( bool notifyPeer )
{GUCEF_TRACE;

    CORE::CTSharedPtr< CRpcLoopbackLink, MT::CMutex > link;
    {
        MT::CScopeMutex lock( m_queueLock );
        link = m_link;
        m_link.Unlink();
    }
    if ( link.IsNULL() )
        return;
    MT::CScopeMutex lock( link->lock );
    CRpcLoopbackTransport* peer = link->PeerOf( this );
    if ( notifyPeer && GUCEF_NULL != peer )
        peer->Enqueue( GUCEF_NULL, 0, true, false );
    link->Remove( this );
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::Close( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_queueLock );
        if ( !m_connected )
            return;
        m_connected = false;
        m_closed = true;
    }
    DisconnectFromPeer( true );

    // this side hears of it too, on its own thread
    Enqueue( GUCEF_NULL, 0, true, false );
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransport::IsConnected( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_queueLock );
    return m_connected;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::Enqueue( const void* frame, UInt32 size, bool isDisconnect, bool isConnect )
{GUCEF_TRACE;

    CORE::PulseGeneratorPtr pulseGenerator;
    {
        MT::CScopeMutex lock( m_queueLock );
        SQueuedFrame* item = GUCEF_NULL;
        if ( !m_free.empty() )
        {
            item = m_free.back();
            m_free.pop_back();
        }
        else
            item = GUCEF_NEW SQueuedFrame();
        item->bytes.SetDataSize( 0 );
        if ( size > 0 )
            item->bytes.CopyFrom( size, frame );
        item->isDisconnect = isDisconnect;
        item->isConnect = isConnect;
        m_queue.push_back( item );

        // delivery always happens in a later pulse, never inside SendFrame(): an immediate pulse request may pulse
        // right here (a driver that is not looping does), so periodic pulses are asked for while frames wait.
        // The request is repeated for every frame: it is what wakes a driver that waits for its next cycle
        if ( !m_pulseGenerator.IsNULL() )
        {
            m_pulsesRequested = true;
            m_pulseGenerator->RequestPeriodicPulses( this, 1 );
        }
    }
}

/*-------------------------------------------------------------------------*/

bool
CRpcLoopbackTransport::SendFrame( const void* frame, UInt32 size, TRpcProtectionClass protection )
{GUCEF_TRACE;

    if ( size > m_properties.maxFrameSize )
        return false;
    CORE::CTSharedPtr< CRpcLoopbackLink, MT::CMutex > link;
    {
        MT::CScopeMutex stateLock( m_queueLock );
        if ( !m_connected )
            return false;
        link = m_link;
    }
    if ( link.IsNULL() )
        return false;
    MT::CScopeMutex lock( link->lock );
    CRpcLoopbackTransport* peer = link->PeerOf( this );
    if ( GUCEF_NULL == peer )
        return false;
    peer->Enqueue( frame, size, false, false );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::OnPulse( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    TQueue batch;
    {
        MT::CScopeMutex lock( m_queueLock );
        if ( m_queue.empty() )
        {
            if ( m_pulsesRequested && !m_pulseGenerator.IsNULL() )
            {
                m_pulsesRequested = false;
                m_pulseGenerator->RequestStopOfPeriodicUpdates( this );
            }
            return;
        }

        // an accepted transport delivers nothing before its listener announced it (TransportAcceptedEvent): the
        // observer that takes it subscribes in that event, its first frame follows. The periodic pulses stay asked for
        if ( m_deliveryHeld )
            return;
        batch.swap( m_queue );
    }

    for ( UInt32 i=0; i<batch.size(); ++i )
    {
        SQueuedFrame* item = batch[ i ];
        if ( item->isConnect )
        {
            NotifyObservers( ConnectedEvent );
        }
        else if ( item->isDisconnect )
        {
            {
                MT::CScopeMutex lock( m_queueLock );
                m_connected = false;
            }
            DisconnectFromPeer( false );
            CRpcStatus why( RPC_STATUS_UNAVAILABLE, "the loopback transport was closed" );
            NotifyObservers( DisconnectedEvent, &why );
        }
        else
        {
            CRpcFrameEventData data( item->bytes.GetConstBufferPtr(), item->bytes.GetDataSize(), RPC_PROTECTION_CONFIDENTIAL );
            NotifyObservers( FrameReceivedEvent, &data );
        }
    }

    MT::CScopeMutex lock( m_queueLock );
    for ( UInt32 i=0; i<batch.size(); ++i )
    {
        if ( m_free.size() < 256 && batch[ i ]->bytes.GetBufferSize() <= 64 * 1024 )
            m_free.push_back( batch[ i ] );
        else
            GUCEF_DELETE batch[ i ];
    }
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcLoopbackTransport::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;

    return m_notifierLock.Lock( lockWaitTimeoutInMs );
}

/*-------------------------------------------------------------------------*/

MT::TLockStatus
CRpcLoopbackTransport::Unlock( void ) const
{GUCEF_TRACE;

    return m_notifierLock.Unlock();
}

/*-------------------------------------------------------------------------*/

void
CRpcLoopbackTransport::ReleaseDelivery( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_queueLock );
    m_deliveryHeld = false;
    if ( !m_queue.empty() && !m_pulseGenerator.IsNULL() )
    {
        m_pulsesRequested = true;
        m_pulseGenerator->RequestPeriodicPulses( this, 1 );
    }
}

/*-------------------------------------------------------------------------*/

const CRpcTransportProperties&
CRpcLoopbackTransport::GetProperties( void ) const
{GUCEF_TRACE;

    return m_properties;
}

/*-------------------------------------------------------------------------*/

const CRpcPrincipal&
CRpcLoopbackTransport::GetPeerPrincipal( void ) const
{GUCEF_TRACE;

    return m_principal;
}

/*-------------------------------------------------------------------------*/

CORE::PulseGeneratorPtr
CRpcLoopbackTransport::GetPulseGenerator( void ) const
{GUCEF_TRACE;

    return m_pulseGenerator;
}

/*-------------------------------------------------------------------------*/

CORE::CUri
CRpcLoopbackTransport::GetRemoteUri( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_queueLock );
    return m_remoteUri;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CRpcLoopbackTransport::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CRpcLoopbackTransport";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace COMCORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
