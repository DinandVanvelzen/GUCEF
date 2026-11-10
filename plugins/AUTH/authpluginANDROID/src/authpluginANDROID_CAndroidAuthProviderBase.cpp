/*
 *  authpluginANDROID: gucefAUTH plugin for logging in on Android devices
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

#ifndef GUCEF_AUTH_CLOCK_H
#include "gucefAUTH_Clock.h"
#define GUCEF_AUTH_CLOCK_H
#endif /* GUCEF_AUTH_CLOCK_H ? */

#include "authpluginANDROID_CAndroidAuthProviderBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace AUTHPLUGIN {
namespace ANDROIDAUTH {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

// The period of the timer that looks for cancelled and timed out requests, while any wait
static const UInt32 PendingCheckIntervalInMs = 100;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CAndroidAuthProviderBase::CPump::CPump( CAndroidAuthProviderBase& owner )
    : CORE::CObserver()
    , m_owner( owner )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthProviderBase::CPump::~CPump()
{GUCEF_TRACE;

    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CAndroidAuthProviderBase::CPump::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::AUTHPLUGIN::ANDROIDAUTH::CAndroidAuthProviderBase::CPump";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthProviderBase::CPump::OnNotify( CORE::CNotifier* notifier, const CORE::CEvent& eventId, CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CTimer::TimerUpdateEvent == eventId )
        m_owner.OnTimer();
}

/*-------------------------------------------------------------------------*/

CAndroidAuthProviderBase::CAndroidAuthProviderBase( void )
    : AUTH::CIAuthProvider()
    , IAndroidAuthResultSink()
    , m_lock()
    , m_pending()
    , m_shutDown( false )
    , m_timer( PendingCheckIntervalInMs )
    , m_pump( *this )
{GUCEF_TRACE;

    m_pump.SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent );
}

/*-------------------------------------------------------------------------*/

CAndroidAuthProviderBase::~CAndroidAuthProviderBase()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthProviderBase::Shutdown( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        m_shutDown = true;
        m_timer.SetEnabled( false );
    }
    m_pump.UnsubscribeAllFromObserver();

    // Waits for an answer that is being handed over; none arrives after this
    CAndroidAuthBridge::Instance()->ReleaseSink( this );

    MT::CScopeMutex lock( m_lock );
    for ( size_t i=0; i<m_pending.size(); ++i )
        m_pending[ i ].source.Cancel();
    m_pending.clear();
}

/*-------------------------------------------------------------------------*/

UInt32
CAndroidAuthProviderBase::GetPendingRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_pending.size();
}

/*-------------------------------------------------------------------------*/

AUTH::TAuthFailureReason
CAndroidAuthProviderBase::ToFailureReason( EAndroidAuthStatus status )
{GUCEF_TRACE;

    switch ( status )
    {
        case ANDROIDAUTH_CANCELLED :        return AUTH::AUTHFAIL_CANCELLED;
        case ANDROIDAUTH_NOT_SIGNED_IN :
        case ANDROIDAUTH_NO_CREDENTIAL :    return AUTH::AUTHFAIL_NOT_AVAILABLE;
        case ANDROIDAUTH_DENIED :           return AUTH::AUTHFAIL_DENIED;
        case ANDROIDAUTH_NETWORK :          return AUTH::AUTHFAIL_NETWORK;
        default :                           return AUTH::AUTHFAIL_INTERNAL;
    }
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthProviderBase::UpdateTimer( void )
{GUCEF_TRACE;

    bool needed = !m_pending.empty() && !m_shutDown;
    if ( needed != m_timer.GetEnabled() )
        m_timer.SetEnabled( needed );
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthProviderBase::StartRequest( bool interactive, UInt32 timeoutInMs, const CORE::CString& nonce, CORE::CFutureResult& future )
{GUCEF_TRACE;

    future = CORE::CFutureResult( CORE::TTaskStatus::TASKSTATUS_UNDEFINED );

    UInt32 requestId = 0;
    CORE::CFutureResultSource source;
    {
        MT::CScopeMutex lock( m_lock );

        CORE::CString reason;
        bool canStart = !m_shutDown && CanStart( interactive, reason );
        bool busy = canStart && !m_pending.empty();
        if ( !canStart || busy )
        {
            if ( !interactive )
            {
                const CORE::CString whyNot = busy ? CORE::CString( "a login is under way" ) : reason;
                GUCEF_LOG( CORE::LOGLEVEL_BELOW_NORMAL, "AndroidAuthProvider: " << GetProviderName() << ": no silent login: " << whyNot );
                return false;
            }
            CORE::CFutureResultSource failed;
            future = failed.GetFuture();
            if ( busy )
                FailLogin( failed, AUTH::AUTHFAIL_IN_PROGRESS, "A login is under way" );
            else
                FailLogin( failed, AUTH::AUTHFAIL_NOT_AVAILABLE, reason );
            return true;
        }

        future = source.GetFuture();
        SPending pending;
        pending.requestId = CAndroidAuthBridge::Instance()->BeginRequest( this );
        pending.source = source;
        pending.deadlineInMs = AUTH::GetMonotonicTimeInMs() + timeoutInMs;
        pending.interactive = interactive;
        pending.nonce = nonce;
        requestId = pending.requestId;
        m_pending.push_back( pending );
        UpdateTimer();
    }

    // The Java side can answer on its own thread, even before this returns: no lock is held here
    if ( !StartNative( requestId, interactive, nonce ) )
    {
        {
            MT::CScopeMutex lock( m_lock );
            for ( size_t i=0; i<m_pending.size(); ++i )
            {
                if ( m_pending[ i ].requestId == requestId )
                {
                    m_pending.erase( m_pending.begin() + i );
                    break;
                }
            }
            UpdateTimer();
        }
        CAndroidAuthBridge::Instance()->EndRequest( requestId );
        FailLogin( source, AUTH::AUTHFAIL_NOT_AVAILABLE, "The Android side did not start the login" );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthProviderBase::OnAndroidAuthResult( const SAndroidAuthResult& result )
{GUCEF_TRACE;

    SPending pending;
    bool found = false;
    {
        MT::CScopeMutex lock( m_lock );
        for ( size_t i=0; i<m_pending.size(); ++i )
        {
            if ( m_pending[ i ].requestId == result.requestId )
            {
                pending = m_pending[ i ];
                m_pending.erase( m_pending.begin() + i );
                found = true;
                break;
            }
        }
        // The timer is not touched here: this runs on a thread of the Java side and a timer belongs to the thread that
        // pumps the pulse generator. It stops itself at its next tick when nothing waits any more
    }

    // A request that was cancelled or timed out meanwhile has been ended by the timer
    if ( found )
        HandleResult( pending, result );
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthProviderBase::OnTimer( void )
{GUCEF_TRACE;

    GUCEF::vector< SPending > cancelled;
    GUCEF::vector< SPending > timedOut;
    {
        MT::CScopeMutex lock( m_lock );
        UInt64 now = AUTH::GetMonotonicTimeInMs();
        size_t i = 0;
        while ( i < m_pending.size() )
        {
            bool isCancelled = m_pending[ i ].source.IsCancellationRequested();
            bool isTimedOut = now >= m_pending[ i ].deadlineInMs;
            if ( isCancelled )
            {
                cancelled.push_back( m_pending[ i ] );
                m_pending.erase( m_pending.begin() + i );
            }
            else
            if ( isTimedOut )
            {
                timedOut.push_back( m_pending[ i ] );
                m_pending.erase( m_pending.begin() + i );
            }
            else
            {
                ++i;
            }
        }
        UpdateTimer();
    }

    // Outside the lock: the Java side may answer the cancellation at once
    CAndroidAuthBridge* bridge = CAndroidAuthBridge::Instance();
    for ( size_t c=0; c<cancelled.size() + timedOut.size(); ++c )
    {
        bool wasCancelled = c < cancelled.size();
        SPending& ended = wasCancelled ? cancelled[ c ] : timedOut[ c - cancelled.size() ];
        bridge->EndRequest( ended.requestId );
        TAndroidShim shim = bridge->GetShim();
        if ( GUCEF_NULL != shim.cancelRequest )
            shim.cancelRequest( shim.context, ended.requestId );
        if ( wasCancelled )
            FailLogin( ended.source, AUTH::AUTHFAIL_CANCELLED, "The login was cancelled" );
        else
            FailLogin( ended.source, AUTH::AUTHFAIL_TIMEOUT, "The login did not finish in time" );
    }
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace ANDROIDAUTH */
}; /* namespace AUTHPLUGIN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
