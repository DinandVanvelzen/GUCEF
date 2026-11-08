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

#include <string.h>

#ifndef GUCEF_COMCORE_MACROS_H
#include "gucefCOMCORE_macros.h"
#define GUCEF_COMCORE_MACROS_H
#endif /* GUCEF_COMCORE_MACROS_H ? */

#if ( ( GUCEF_PLATFORM == GUCEF_PLATFORM_LINUX ) || ( GUCEF_PLATFORM == GUCEF_PLATFORM_ANDROID ) )
  #include <errno.h>
  #include <unistd.h>
  #include <sys/epoll.h>
  #include <sys/eventfd.h>
  #define GUCEF_COMCORE_READINESS_EPOLL
#elif ( GUCEF_PLATFORM == GUCEF_PLATFORM_MSWIN )
  #ifndef GUCEF_COMCORE_DVSOCKET_H
  #include "dvwinsock.h"              /* winsock2.h: WSAPoll, Windows Vista and later */
  #define GUCEF_COMCORE_DVSOCKET_H
  #endif /* GUCEF_COMCORE_DVSOCKET_H ? */
  #if defined( _WIN32_WINNT ) && ( _WIN32_WINNT >= 0x0600 )
    #define GUCEF_COMCORE_READINESS_WSAPOLL
  #endif
#endif

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

#ifndef GUCEF_CORE_CTASKMANAGER_H
#include "gucefCORE_CTaskManager.h"
#define GUCEF_CORE_CTASKMANAGER_H
#endif /* GUCEF_CORE_CTASKMANAGER_H ? */

#ifndef GUCEF_CORE_CTHREADPOOL_H
#include "gucefCORE_CThreadPool.h"
#define GUCEF_CORE_CTHREADPOOL_H
#endif /* GUCEF_CORE_CTHREADPOOL_H ? */

#include "gucefCOMCORE_CSocketReadinessWaiter.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace COMCORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const UInt32 CSocketReadinessWaiter::ReadinessRead = 1;
const UInt32 CSocketReadinessWaiter::ReadinessWrite = 2;
const CORE::CString CSocketReadinessWaiter::TaskType = "GUCEF::COMCORE::CSocketReadinessWaiter";
const CORE::CString CSocketReadinessWaiter::ThreadPoolName = "gucef.comcore.readiness";
const UInt32 CSocketReadinessWaiter::WaitTimeoutInMs = 100;

/*-------------------------------------------------------------------------//
//                                                                         //
//      TYPES                                                              //
//                                                                         //
//-------------------------------------------------------------------------*/

struct CSocketReadinessWaiter::SRegistration
{
    UInt64 descriptor;
    CORE::PulseGeneratorPtr owner;
    UInt32 interest;            /**< armed readiness, 0 when disarmed */
};

/*-------------------------------------------------------------------------*/

#if defined( GUCEF_COMCORE_READINESS_EPOLL )

/** epoll set with an eventfd at id 0 that ends a wait */
struct CSocketReadinessWaiter::SBackend
{
    int epollFd;
    int wakeFd;
    struct epoll_event events[ 64 ];

    SBackend( void ) : epollFd( -1 ), wakeFd( -1 ) { memset( events, 0, sizeof( events ) ); }
};

#elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

/**
 *  WSAPoll over an array that is rebuilt when registrations or their arming changed; a UDP socket bound to the
 *  loopback address, at the front of the array, ends a wait when a datagram is sent to it
 */
struct CSocketReadinessWaiter::SBackend
{
    SOCKET wakeSocket;
    SOCKADDR_IN wakeAddress;
    bool rebuild;                                       /**< guarded by the registry lock */
    GUCEF::vector< WSAPOLLFD > pollSet;                 /**< used by the waiter's task only */
    GUCEF::vector< TRegistrationId > pollSetIds;        /**< id per entry of pollSet, 0 for the wake socket */

    SBackend( void ) : wakeSocket( INVALID_SOCKET ), rebuild( true ), pollSet(), pollSetIds() { memset( &wakeAddress, 0, sizeof( wakeAddress ) ); }
};

#else

struct CSocketReadinessWaiter::SBackend
{
};

#endif

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CSocketReadinessWaiter::CSocketReadinessWaiter( void )
    : CORE::CTaskConsumer()
    , CORE::CTSharedObjCreator< CSocketReadinessWaiter, MT::CMutex >( this )
    , m_backend( GUCEF_NULL )
    , m_registryLock()
    , m_taskLock()
    , m_registrations()
    , m_enabledPulseGenerators()
    , m_lastRegistrationId( 0 )
    , m_readyIds()
    , m_pulseGeneratorsToWake()
    , m_shutdown( false )
    , m_taskRunning( false )
    , m_wakeCount( 0 )
    , m_waitCount( 0 )
{GUCEF_TRACE;

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )

    SBackend* backend = GUCEF_NEW SBackend();
    backend->epollFd = epoll_create1( EPOLL_CLOEXEC );
    backend->wakeFd = eventfd( 0, EFD_NONBLOCK | EFD_CLOEXEC );
    if ( backend->epollFd >= 0 && backend->wakeFd >= 0 )
    {
        struct epoll_event wakeEvent;
        memset( &wakeEvent, 0, sizeof( wakeEvent ) );
        wakeEvent.events = EPOLLIN;
        wakeEvent.data.u64 = 0;
        if ( 0 == epoll_ctl( backend->epollFd, EPOLL_CTL_ADD, backend->wakeFd, &wakeEvent ) )
            m_backend = backend;
    }
    if ( GUCEF_NULL == m_backend )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadinessWaiter: cannot set up epoll (errno " << errno << "), sockets are serviced at their periodic pulses only" );
        if ( backend->epollFd >= 0 )
            close( backend->epollFd );
        if ( backend->wakeFd >= 0 )
            close( backend->wakeFd );
        GUCEF_DELETE backend;
    }

    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

    SBackend* backend = GUCEF_NEW SBackend();
    int errorCode = 0;
    backend->wakeSocket = dvsocket_socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP, &errorCode );
    if ( INVALID_SOCKET != backend->wakeSocket )
    {
        u_long nonBlocking = 1;
        backend->wakeAddress.sin_family = AF_INET;
        backend->wakeAddress.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
        backend->wakeAddress.sin_port = 0;
        int addressSize = (int) sizeof( backend->wakeAddress );
        if ( 0 == ioctlsocket( backend->wakeSocket, FIONBIO, &nonBlocking )                                                    &&
             0 == bind( backend->wakeSocket, (const struct sockaddr*) &backend->wakeAddress, (int) sizeof( backend->wakeAddress ) ) &&
             0 == getsockname( backend->wakeSocket, (struct sockaddr*) &backend->wakeAddress, &addressSize )                          )
        {
            m_backend = backend;
        }
    }
    if ( GUCEF_NULL == m_backend )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadinessWaiter: cannot set up the wake socket (error " << WSAGetLastError() << "), sockets are serviced at their periodic pulses only" );
        if ( INVALID_SOCKET != backend->wakeSocket )
            closesocket( backend->wakeSocket );
        GUCEF_DELETE backend;
    }

    #endif
}

/*-------------------------------------------------------------------------*/

CSocketReadinessWaiter::~CSocketReadinessWaiter()
{GUCEF_TRACE;

    Shutdown();
    ReleaseBackend();
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::ReleaseBackend( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_registryLock );
    if ( GUCEF_NULL == m_backend )
        return;

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )
    close( m_backend->epollFd );
    close( m_backend->wakeFd );
    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )
    closesocket( m_backend->wakeSocket );
    #endif

    GUCEF_DELETE m_backend;
    m_backend = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CSocketReadinessWaiter::IsAvailable( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_registryLock );
    return GUCEF_NULL != m_backend && !m_shutdown.Load();
}

/*-------------------------------------------------------------------------*/

bool
CSocketReadinessWaiter::EnsureTaskStarted( void )
{GUCEF_TRACE;

    if ( m_taskRunning.Load() )
        return true;

    MT::CScopeMutex taskLock( m_taskLock );
    if ( m_taskRunning.Load() )
        return true;
    if ( m_shutdown.Load() )
        return false;

    CORE::ThreadPoolPtr pool = CORE::CCoreGlobal::Instance()->GetTaskManager().GetOrCreateThreadPool( ThreadPoolName );
    if ( pool.IsNULL() )
        return false;
    CSocketReadinessWaiterPtr self = CreateSharedPtr();
    CORE::CFutureResult setup = pool->SetupSingularTask( self );
    if ( setup.HasNoFuture() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadinessWaiter: pool \"" << ThreadPoolName << "\" refused the task, sockets are serviced at their periodic pulses only" );
        return false;
    }
    m_taskRunning.Store( true );
    CORE::CFutureResult started = pool->StartTaskWithConsumer( self );
    if ( started.HasNoFuture() )
    {
        m_taskRunning.Store( false );
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "SocketReadinessWaiter: pool \"" << ThreadPoolName << "\" did not start the task, sockets are serviced at their periodic pulses only" );
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CSocketReadinessWaiter::TRegistrationId
CSocketReadinessWaiter::Register( UInt64 socketDescriptor, const CORE::PulseGeneratorPtr& owner )
{GUCEF_TRACE;

    if ( owner.IsNULL() || GUCEF_NULL == m_backend || m_shutdown.Load() )
        return 0;

    // the task is started outside the registry lock: it takes the locks of the thread pool
    if ( !EnsureTaskStarted() )
        return 0;

    MT::CScopeMutex lock( m_registryLock );
    if ( GUCEF_NULL == m_backend || m_shutdown.Load() )
        return 0;

    TRegistrationId registrationId = ++m_lastRegistrationId;

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )

    // added disarmed: one shot with no readiness asked for until Arm()
    struct epoll_event socketEvent;
    memset( &socketEvent, 0, sizeof( socketEvent ) );
    socketEvent.events = EPOLLONESHOT;
    socketEvent.data.u64 = registrationId;
    if ( 0 != epoll_ctl( m_backend->epollFd, EPOLL_CTL_ADD, (int) socketDescriptor, &socketEvent ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SocketReadinessWaiter: cannot add socket " << socketDescriptor << " to the epoll set (errno " << errno << ")" );
        return 0;
    }

    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

    // disarmed entries are not in the poll set, nothing to rebuild yet

    #endif

    SRegistration* registration = GUCEF_NEW SRegistration();
    registration->descriptor = socketDescriptor;
    registration->owner = owner;
    registration->interest = 0;
    m_registrations[ registrationId ] = registration;
    return registrationId;
}

/*-------------------------------------------------------------------------*/

bool
CSocketReadinessWaiter::Arm( TRegistrationId registrationId, UInt32 interest )
{GUCEF_TRACE;

    if ( 0 == registrationId || 0 == interest )
        return false;

    MT::CScopeMutex lock( m_registryLock );
    TRegistrationMap::iterator i = m_registrations.find( registrationId );
    if ( i == m_registrations.end() || GUCEF_NULL == m_backend )
        return false;
    SRegistration* registration = (*i).second;

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )

    // the kernel takes the change while the waiter waits; readiness that is there already is reported at once
    struct epoll_event socketEvent;
    memset( &socketEvent, 0, sizeof( socketEvent ) );
    socketEvent.events = EPOLLONESHOT | EPOLLRDHUP;
    if ( 0 != ( interest & ReadinessRead ) )
        socketEvent.events |= EPOLLIN;
    if ( 0 != ( interest & ReadinessWrite ) )
        socketEvent.events |= EPOLLOUT;
    socketEvent.data.u64 = registrationId;
    if ( 0 != epoll_ctl( m_backend->epollFd, EPOLL_CTL_MOD, (int) registration->descriptor, &socketEvent ) )
        return false;
    registration->interest = interest;
    return true;

    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

    if ( registration->interest == interest )
        return true;
    registration->interest = interest;
    m_backend->rebuild = true;
    lock.EarlyUnlock();
    WakeWaiter();
    return true;

    #else

    return false;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::Unregister( TRegistrationId registrationId )
{GUCEF_TRACE;

    if ( 0 == registrationId )
        return;

    MT::CScopeMutex lock( m_registryLock );
    TRegistrationMap::iterator i = m_registrations.find( registrationId );
    if ( i == m_registrations.end() )
        return;
    SRegistration* registration = (*i).second;
    m_registrations.erase( i );

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )

    // the descriptor is still open: the owner closes it after this. An event of this id the waiter already took
    // finds no registration and is dropped
    if ( GUCEF_NULL != m_backend )
    {
        struct epoll_event socketEvent;
        memset( &socketEvent, 0, sizeof( socketEvent ) );
        epoll_ctl( m_backend->epollFd, EPOLL_CTL_DEL, (int) registration->descriptor, &socketEvent );
    }
    GUCEF_DELETE registration;

    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

    bool wasArmed = 0 != registration->interest;
    GUCEF_DELETE registration;
    if ( wasArmed && GUCEF_NULL != m_backend )
    {
        // the descriptor must leave the poll set before the owner closes it and the system reuses the handle
        m_backend->rebuild = true;
        lock.EarlyUnlock();
        WakeWaiter();
    }

    #else

    GUCEF_DELETE registration;

    #endif
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::EnableForPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( pulseGenerator.IsNULL() )
        return;
    MT::CScopeMutex lock( m_registryLock );
    if ( m_shutdown.Load() )
        return;
    m_enabledPulseGenerators[ pulseGenerator.GetPointerAlways() ] = pulseGenerator;
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::DisableForPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator )
{GUCEF_TRACE;

    if ( pulseGenerator.IsNULL() )
        return;

    // the reference is let go of outside the lock: it may be the last one
    CORE::PulseGeneratorPtr released;
    {
        MT::CScopeMutex lock( m_registryLock );
        TPulseGeneratorMap::iterator i = m_enabledPulseGenerators.find( pulseGenerator.GetPointerAlways() );
        if ( i == m_enabledPulseGenerators.end() )
            return;
        released = (*i).second;
        m_enabledPulseGenerators.erase( i );
    }
}

/*-------------------------------------------------------------------------*/

bool
CSocketReadinessWaiter::IsEnabledForPulseGenerator( const CORE::PulseGeneratorPtr& pulseGenerator ) const
{GUCEF_TRACE;

    if ( pulseGenerator.IsNULL() )
        return false;
    MT::CScopeMutex lock( m_registryLock );
    return m_enabledPulseGenerators.find( pulseGenerator.GetPointerAlways() ) != m_enabledPulseGenerators.end();
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::WakeWaiter( void )
{GUCEF_TRACE;

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )

    MT::CScopeMutex lock( m_registryLock );
    if ( GUCEF_NULL != m_backend )
    {
        UInt64 one = 1;
        ssize_t written = write( m_backend->wakeFd, &one, sizeof( one ) );
        (void) written;     // a full counter already wakes the wait
    }

    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

    MT::CScopeMutex lock( m_registryLock );
    if ( GUCEF_NULL != m_backend )
    {
        char one = 1;
        sendto( m_backend->wakeSocket, &one, 1, 0, (const struct sockaddr*) &m_backend->wakeAddress, (int) sizeof( m_backend->wakeAddress ) );
    }

    #endif
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::Shutdown( void )
{GUCEF_TRACE;

    // the references are let go of outside the lock: one may be the last
    TPulseGeneratorMap released;
    {
        MT::CScopeMutex lock( m_registryLock );
        if ( m_shutdown.Load() )
            return;
        m_shutdown.Store( true );

        TRegistrationMap::iterator i = m_registrations.begin();
        while ( i != m_registrations.end() )
        {
            #if defined( GUCEF_COMCORE_READINESS_EPOLL )
            if ( GUCEF_NULL != m_backend )
            {
                struct epoll_event socketEvent;
                memset( &socketEvent, 0, sizeof( socketEvent ) );
                epoll_ctl( m_backend->epollFd, EPOLL_CTL_DEL, (int) (*i).second->descriptor, &socketEvent );
            }
            #endif
            GUCEF_DELETE (*i).second;
            ++i;
        }
        m_registrations.clear();
        released.swap( m_enabledPulseGenerators );

        #if defined( GUCEF_COMCORE_READINESS_WSAPOLL )
        if ( GUCEF_NULL != m_backend )
            m_backend->rebuild = true;
        #endif
    }

    WakeWaiter();

    // not under the registry lock: the stop takes the locks of the thread pool and waits for the task, which takes
    // the registry lock in its wait
    MT::CScopeMutex taskLock( m_taskLock );
    if ( m_taskRunning.Load() )
    {
        // the identity of the thread that pulses, not GetDelegatorThreadId(): that one is the operating system's id of the thread
        CORE::PulseGeneratorPtr pulseGenerator = GetPulseGenerator();
        bool ownThread = !pulseGenerator.IsNULL() && pulseGenerator->GetPulseDriverThreadId() == MT::GetCurrentThreadID();
        RequestTaskToStop( !ownThread );
    }
}

/*-------------------------------------------------------------------------*/

UInt64
CSocketReadinessWaiter::GetWakeCount( void ) const
{GUCEF_TRACE;

    return m_wakeCount.Load();
}

/*-------------------------------------------------------------------------*/

UInt64
CSocketReadinessWaiter::GetWaitCount( void ) const
{GUCEF_TRACE;

    return m_waitCount.Load();
}

/*-------------------------------------------------------------------------*/

UInt32
CSocketReadinessWaiter::GetRegistrationCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_registryLock );
    return (UInt32) m_registrations.size();
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::WaitOnce( UInt32 timeoutInMs )
{GUCEF_TRACE;

    m_readyIds.clear();

    #if defined( GUCEF_COMCORE_READINESS_EPOLL )

    // the backend lives as long as this object, only ReleaseBackend() in the destructor removes it
    int readyCount = epoll_wait( m_backend->epollFd, m_backend->events, 64, (int) timeoutInMs );
    m_waitCount.FetchAdd( 1 );
    if ( readyCount < 0 )
    {
        if ( EINTR != errno )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SocketReadinessWaiter: epoll_wait failed (errno " << errno << ")" );
            MT::PrecisionDelay( timeoutInMs );
        }
        return;
    }
    for ( int n=0; n<readyCount; ++n )
    {
        TRegistrationId readyId = m_backend->events[ n ].data.u64;
        if ( 0 == readyId )
        {
            UInt64 count = 0;
            ssize_t readSize = read( m_backend->wakeFd, &count, sizeof( count ) );
            (void) readSize;
        }
        else
            m_readyIds.push_back( readyId );
    }

    #elif defined( GUCEF_COMCORE_READINESS_WSAPOLL )

    {
        MT::CScopeMutex lock( m_registryLock );
        if ( m_backend->rebuild )
        {
            m_backend->pollSet.clear();
            m_backend->pollSetIds.clear();

            WSAPOLLFD entry;
            memset( &entry, 0, sizeof( entry ) );
            entry.fd = m_backend->wakeSocket;
            entry.events = POLLRDNORM;
            m_backend->pollSet.push_back( entry );
            m_backend->pollSetIds.push_back( 0 );

            TRegistrationMap::iterator i = m_registrations.begin();
            while ( i != m_registrations.end() )
            {
                SRegistration* registration = (*i).second;
                if ( 0 != registration->interest )
                {
                    memset( &entry, 0, sizeof( entry ) );
                    entry.fd = (SOCKET) registration->descriptor;
                    if ( 0 != ( registration->interest & ReadinessRead ) )
                        entry.events |= POLLRDNORM;
                    if ( 0 != ( registration->interest & ReadinessWrite ) )
                        entry.events |= POLLWRNORM;
                    m_backend->pollSet.push_back( entry );
                    m_backend->pollSetIds.push_back( (*i).first );
                }
                ++i;
            }
            m_backend->rebuild = false;
        }
    }

    int readyCount = WSAPoll( &m_backend->pollSet[ 0 ], (ULONG) m_backend->pollSet.size(), (INT) timeoutInMs );
    m_waitCount.FetchAdd( 1 );
    if ( SOCKET_ERROR == readyCount )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "SocketReadinessWaiter: WSAPoll failed (error " << WSAGetLastError() << ")" );
        MT::PrecisionDelay( timeoutInMs );
        return;
    }
    for ( size_t n=0; n<m_backend->pollSet.size() && readyCount > 0; ++n )
    {
        if ( 0 == m_backend->pollSet[ n ].revents )
            continue;
        --readyCount;
        if ( 0 == m_backend->pollSetIds[ n ] )
        {
            char buffer[ 64 ];
            while ( recv( m_backend->wakeSocket, buffer, (int) sizeof( buffer ), 0 ) > 0 ) {}
        }
        else
            m_readyIds.push_back( m_backend->pollSetIds[ n ] );
    }

    #else

    MT::PrecisionDelay( timeoutInMs );
    m_waitCount.FetchAdd( 1 );

    #endif

    if ( m_readyIds.empty() )
        return;

    // disarm and collect the owners under the lock, one request per pulse generator; the requests are made after it
    m_pulseGeneratorsToWake.clear();
    {
        MT::CScopeMutex lock( m_registryLock );
        for ( size_t n=0; n<m_readyIds.size(); ++n )
        {
            TRegistrationMap::iterator i = m_registrations.find( m_readyIds[ n ] );
            if ( i == m_registrations.end() )
                continue;   // unregistered while the wait returned
            SRegistration* registration = (*i).second;
            if ( 0 == registration->interest )
                continue;
            registration->interest = 0;

            #if defined( GUCEF_COMCORE_READINESS_WSAPOLL )
            m_backend->rebuild = true;
            #endif

            bool known = false;
            for ( size_t p=0; p<m_pulseGeneratorsToWake.size(); ++p )
            {
                if ( m_pulseGeneratorsToWake[ p ].GetPointerAlways() == registration->owner.GetPointerAlways() )
                {
                    known = true;
                    break;
                }
            }
            if ( !known )
                m_pulseGeneratorsToWake.push_back( registration->owner );
        }
    }

    for ( size_t p=0; p<m_pulseGeneratorsToWake.size(); ++p )
    {
        if ( m_pulseGeneratorsToWake[ p ]->RequestImmediatePulseThroughDriver() )
            m_wakeCount.FetchAdd( 1 );
    }
    m_pulseGeneratorsToWake.clear();
}

/*-------------------------------------------------------------------------*/

CORE::CString
CSocketReadinessWaiter::GetType( void ) const
{GUCEF_TRACE;

    return TaskType;
}

/*-------------------------------------------------------------------------*/

bool
CSocketReadinessWaiter::OnTaskStart( CORE::CTaskPtr task )
{GUCEF_TRACE;

    return GUCEF_NULL != m_backend;
}

/*-------------------------------------------------------------------------*/

bool
CSocketReadinessWaiter::OnTaskCycle( CORE::CTaskPtr task )
{GUCEF_TRACE;

    // the task keeps its own cycle: between cycles the delegator would wait its minimal cycle delta, and readiness
    // would wait with it. The OS wait has a timeout so that a stop asked for from outside is seen
    while ( !IsDeactivationRequested() && !m_shutdown.Load() )
        WaitOnce( WaitTimeoutInMs );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CSocketReadinessWaiter::OnTaskEnded( CORE::CTaskPtr task, bool wasForced )
{GUCEF_TRACE;

    m_taskRunning.Store( false );
    CORE::CTaskConsumer::OnTaskEnded( task, wasForced );
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CSocketReadinessWaiter::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::COMCORE::CSocketReadinessWaiter";
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
