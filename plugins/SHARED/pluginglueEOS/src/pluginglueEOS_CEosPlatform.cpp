/*
 *  pluginglueEOS: Library that owns the Epic Online Services platform for the plugins that share it
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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#include "pluginglueEOS_CEosPlatform.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace PLUGINGLUE {
namespace EOS {

/*-------------------------------------------------------------------------//
//                                                                         //
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

const CORE::CEvent CEosPlatform::TickEvent = "GUCEF::PLUGINGLUE::EOS::CEosPlatform::TickEvent";
const CORE::CEvent CEosPlatform::LoginStateChangedEvent = "GUCEF::PLUGINGLUE::EOS::CEosPlatform::LoginStateChangedEvent";

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

const char*
EosResultToString( EEosResult result )
{GUCEF_TRACE;

    switch ( result )
    {
        case EOSRESULT_SUCCESS :            return "success";
        case EOSRESULT_CANCELLED :          return "cancelled";
        case EOSRESULT_NOT_FOUND :          return "not found";
        case EOSRESULT_NO_PERMISSION :      return "no permission";
        case EOSRESULT_INVALID_PARAMETERS : return "invalid parameters";
        case EOSRESULT_LOBBY_FULL :         return "lobby is full";
        case EOSRESULT_LIMIT_EXCEEDED :     return "a limit was exceeded";
        case EOSRESULT_NO_CONNECTION :      return "no connection";
        case EOSRESULT_TIMEOUT :            return "timed out";
        case EOSRESULT_ALREADY_PENDING :    return "already pending";
        case EOSRESULT_NOT_LOGGED_IN :      return "not logged in";
        default :                           return "failed";
    }
}

/*-------------------------------------------------------------------------*/

CEosLoginState::CEosLoginState( void )
    : CORE::CICloneable()
    , epicAccountId()
    , productUserId()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEosLoginState::CEosLoginState( const CORE::CString& epicAccount, const CORE::CString& productUser )
    : CORE::CICloneable()
    , epicAccountId( epicAccount )
    , productUserId( productUser )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CEosLoginState::IsLoggedIn( void ) const
{GUCEF_TRACE;

    return !productUserId.IsNULLOrEmpty();
}

/*-------------------------------------------------------------------------*/

bool
CEosLoginState::operator==( const CEosLoginState& other ) const
{GUCEF_TRACE;

    return epicAccountId == other.epicAccountId && productUserId == other.productUserId;
}

/*-------------------------------------------------------------------------*/

CORE::CICloneable*
CEosLoginState::Clone( void ) const
{GUCEF_TRACE;

    return GUCEF_NEW CEosLoginState( *this );
}

/*-------------------------------------------------------------------------*/

UInt64
CEosPlatform::GetMonotonicTimeInMs( void )
{GUCEF_TRACE;

    UInt64 resolution = MT::PrecisionTimerResolution();
    UInt64 ticks = MT::PrecisionTickCount();
    if ( resolution >= 1000 )
        return ticks / ( resolution / 1000 );
    if ( 0 == resolution )
        return ticks;
    return ticks * ( 1000 / resolution );
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::RegisterEvents( void )
{GUCEF_TRACE;

    TickEvent.Initialize();
    LoginStateChangedEvent.Initialize();
}

/*-------------------------------------------------------------------------*/

CEosPlatform::CEosPlatform( const SEosPlatformShim& shim, const CEosPlatformSettings& settings )
    : CORE::CObservingNotifier()
    , m_lock()
    , m_shim( shim )
    , m_settings( settings )
    , m_createdHere( false )
    , m_refCount( 0 )
    , m_loginState()
    , m_demands()
    , m_nextDemandId( 0 )
    , m_timer( DefaultTickIntervalInMs )
    , m_router( GUCEF_NULL )
{GUCEF_TRACE;

    RegisterEvents();
    SubscribeTo( &m_timer, CORE::CTimer::TimerUpdateEvent );
}

/*-------------------------------------------------------------------------*/

CEosPlatform::~CEosPlatform()
{GUCEF_TRACE;

    Shutdown();
    UnsubscribeAllFromObserver();
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatform::Create( CORE::CString& reason )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_shim.isSdkAvailable || !m_shim.isSdkAvailable( m_shim.context ) )
    {
        reason = "The Epic Online Services SDK is not part of this build";
        return false;
    }

    // A platform that is there already is used as it is: the application made it
    if ( GUCEF_NULL != m_shim.isPlatformReady && m_shim.isPlatformReady( m_shim.context ) )
        return true;

    if ( GUCEF_NULL == m_shim.createPlatform )
    {
        reason = "The Epic Online Services SDK cannot create its platform";
        return false;
    }
    if ( !m_settings.IsConfigured() )
    {
        reason = "The settings clientId, clientSecret, productId, sandboxId and deploymentId are needed";
        return false;
    }

    SEosPlatformConfig config;
    m_settings.FillPlatformConfig( config );

    char text[ 256 ];
    text[ 0 ] = '\0';
    if ( !m_shim.createPlatform( m_shim.context, &config, text, sizeof( text ) ) )
    {
        reason = '\0' == text[ 0 ] ? CORE::CString( "The Epic Online Services platform could not be created" ) : CORE::CString( text );
        return false;
    }
    m_createdHere = true;
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosPlatform: The Epic Online Services platform is created" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::Shutdown( void )
{GUCEF_TRACE;

    m_timer.SetEnabled( false );
    GUCEF_DELETE m_router;
    m_router = GUCEF_NULL;
    if ( m_createdHere && GUCEF_NULL != m_shim.releasePlatform )
    {
        m_shim.releasePlatform( m_shim.context );
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosPlatform: The Epic Online Services platform is released" );
    }
    m_createdHere = false;
}

/*-------------------------------------------------------------------------*/

const SEosPlatformShim&
CEosPlatform::GetPlatformShim( void ) const
{GUCEF_TRACE;

    return m_shim;
}

/*-------------------------------------------------------------------------*/

const CEosPlatformSettings&
CEosPlatform::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatform::IsReady( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_shim.isPlatformReady && m_shim.isPlatformReady( m_shim.context );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosPlatform::GetRefCount( void ) const
{GUCEF_TRACE;

    return m_refCount;
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatform::WasCreatedHere( void ) const
{GUCEF_TRACE;

    return m_createdHere;
}

/*-------------------------------------------------------------------------*/

void*
CEosPlatform::GetPlatformHandle( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL == m_shim.getPlatformHandle ? GUCEF_NULL : m_shim.getPlatformHandle( m_shim.context );
}

/*-------------------------------------------------------------------------*/

CEosLoginState
CEosPlatform::GetLoginState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_loginState;
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::SetLoginState( const CORE::CString& epicAccountId, const CORE::CString& productUserId )
{GUCEF_TRACE;

    CEosLoginState state( epicAccountId, productUserId );
    {
        MT::CScopeMutex lock( m_lock );
        if ( state == m_loginState )
            return;
        m_loginState = state;
    }

    if ( GUCEF_NULL != m_router )
        m_router->SetLocalUser( productUserId );
    const CORE::CString stateText = state.IsLoggedIn() ? "a product user is logged in" : "no product user is logged in";
    GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "EosPlatform: The login state changed, " << stateText );
    NotifyObservers( LoginStateChangedEvent, &state );
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::ClearLoginState( void )
{GUCEF_TRACE;

    SetLoginState( CORE::CString::Empty, CORE::CString::Empty );
}

/*-------------------------------------------------------------------------*/

UInt32
CEosPlatform::AddTickDemand( UInt32 intervalInMs )
{GUCEF_TRACE;

    SDemand demand;
    demand.id = ++m_nextDemandId;
    demand.intervalInMs = intervalInMs > 0 ? intervalInMs : DefaultTickIntervalInMs;
    m_demands.push_back( demand );
    UpdateTimer();
    return demand.id;
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::SetTickDemandInterval( UInt32 demandId, UInt32 intervalInMs )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_demands.size(); ++i )
    {
        if ( m_demands[ i ].id == demandId )
        {
            m_demands[ i ].intervalInMs = intervalInMs > 0 ? intervalInMs : DefaultTickIntervalInMs;
            UpdateTimer();
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::RemoveTickDemand( UInt32 demandId )
{GUCEF_TRACE;

    for ( size_t i=0; i<m_demands.size(); ++i )
    {
        if ( m_demands[ i ].id == demandId )
        {
            m_demands.erase( m_demands.begin() + i );
            UpdateTimer();
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CEosPlatform::GetTickDemandCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_demands.size();
}

/*-------------------------------------------------------------------------*/

UInt32
CEosPlatform::GetTickInterval( void ) const
{GUCEF_TRACE;

    if ( m_demands.empty() )
        return 0;
    UInt32 interval = m_demands[ 0 ].intervalInMs;
    for ( size_t i=1; i<m_demands.size(); ++i )
        if ( m_demands[ i ].intervalInMs < interval )
            interval = m_demands[ i ].intervalInMs;
    return interval;
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::UpdateTimer( void )
{GUCEF_TRACE;

    UInt32 interval = GetTickInterval();
    if ( 0 == interval )
    {
        if ( m_timer.GetEnabled() )
            m_timer.SetEnabled( false );
        return;
    }
    if ( m_timer.GetInterval() != interval )
        m_timer.SetInterval( interval );
    if ( !m_timer.GetEnabled() )
        m_timer.SetEnabled( true );
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::Tick( void )
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_shim.tick )
        m_shim.tick( m_shim.context );
    if ( GUCEF_NULL != m_router )
        m_router->Pump();
    NotifyObservers( TickEvent );
}

/*-------------------------------------------------------------------------*/

bool
CEosPlatform::AttachP2P( const SEosP2PShim& shim )
{GUCEF_TRACE;

    if ( !shim.IsUsable() || GUCEF_NULL != m_router )
        return false;
    m_router = GUCEF_NEW CEosP2PRouter( shim );
    m_router->SetLocalUser( GetLoginState().productUserId );
    return true;
}

/*-------------------------------------------------------------------------*/

CEosP2PRouter*
CEosPlatform::GetP2PRouter( void ) const
{GUCEF_TRACE;

    return m_router;
}

/*-------------------------------------------------------------------------*/

void
CEosPlatform::OnNotify( CORE::CNotifier* notifier                 ,
                        const CORE::CEvent& eventId               ,
                        CORE::CICloneable* eventData /* = GUCEF_NULL */ )
{GUCEF_TRACE;

    if ( notifier == &m_timer && CORE::CTimer::TimerUpdateEvent == eventId )
        Tick();
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEosPlatform::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::PLUGINGLUE::EOS::CEosPlatform";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace EOS */
}; /* namespace PLUGINGLUE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
