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

#ifndef GUCEF_MT_DVMTOSWRAP_H
#include "gucefMT_dvmtoswrap.h"
#define GUCEF_MT_DVMTOSWRAP_H
#endif /* GUCEF_MT_DVMTOSWRAP_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#include "authpluginANDROID_CAndroidAuthBridge.h"

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

// How long ReleaseSink() waits for a delivery that is under way
static const UInt32 ReleaseWaitLimitInMs = 5000;

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

IAndroidAuthResultSink::IAndroidAuthResultSink( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

IAndroidAuthResultSink::~IAndroidAuthResultSink()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthBridge::CLauncher::CLauncher( void )
    : CORE::CIBrowserLauncherHandler()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthBridge::CLauncher::~CLauncher()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthBridge::CLauncher::OpenUrl( const CORE::CString& url )
{GUCEF_TRACE;

    TAndroidShim shim = CAndroidAuthBridge::Instance()->GetShim();
    if ( GUCEF_NULL == shim.openCustomTab )
        return false;
    // The url is not logged, it can carry a state or a nonce
    return shim.openCustomTab( shim.context, url.C_String() );
}

/*-------------------------------------------------------------------------*/

CAndroidAuthBridge::CAndroidAuthBridge( void )
    : m_lock()
    , m_shim()
    , m_requests()
    , m_nextRequestId( 0 )
    , m_deliveriesInFlight( 0 )
    , m_redirectReceiver( GUCEF_NULL )
    , m_launcher()
    , m_previousLauncher( GUCEF_NULL )
    , m_launcherInstalled( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthBridge::~CAndroidAuthBridge()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CAndroidAuthBridge*
CAndroidAuthBridge::Instance( void )
{GUCEF_TRACE;

    // Lives as long as the plugin: the Java side can answer at any time
    static CAndroidAuthBridge bridge;
    return &bridge;
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthBridge::SetShim( const TAndroidShim& shim )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_shim = shim;
}

/*-------------------------------------------------------------------------*/

TAndroidShim
CAndroidAuthBridge::GetShim( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_shim;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthBridge::HasShim( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return GUCEF_NULL != m_shim.googleSignIn || GUCEF_NULL != m_shim.amazonSignIn || GUCEF_NULL != m_shim.openCustomTab;
}

/*-------------------------------------------------------------------------*/

UInt32
CAndroidAuthBridge::BeginRequest( IAndroidAuthResultSink* sink )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    do
    {
        ++m_nextRequestId;
    }
    while ( 0 == m_nextRequestId || m_requests.find( m_nextRequestId ) != m_requests.end() );
    m_requests[ m_nextRequestId ] = sink;
    return m_nextRequestId;
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthBridge::EndRequest( UInt32 requestId )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    m_requests.erase( requestId );
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthBridge::ReleaseSink( IAndroidAuthResultSink* sink )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_lock );
        TRequestMap::iterator i = m_requests.begin();
        while ( i != m_requests.end() )
        {
            if ( (*i).second == sink )
                m_requests.erase( i++ );
            else
                ++i;
        }
    }

    // A delivery that took the sink before it was released is still running: wait for it
    for ( UInt32 waited=0; waited<ReleaseWaitLimitInMs; waited+=2 )
    {
        {
            MT::CScopeMutex lock( m_lock );
            if ( 0 == m_deliveriesInFlight )
                return;
        }
        MT::ThreadDelay( 2 );
    }
    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthBridge: A delivery of an answer did not finish in time while a provider was released" );
}

/*-------------------------------------------------------------------------*/

UInt32
CAndroidAuthBridge::GetRequestCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_requests.size();
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthBridge::DeliverResult( const SAndroidAuthResult& result )
{GUCEF_TRACE;

    IAndroidAuthResultSink* sink = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        TRequestMap::iterator i = m_requests.find( result.requestId );
        if ( i == m_requests.end() )
            return false;
        sink = (*i).second;
        m_requests.erase( i );
        ++m_deliveriesInFlight;
    }

    // Outside the lock: the sink takes locks of its own
    sink->OnAndroidAuthResult( result );

    MT::CScopeMutex lock( m_lock );
    --m_deliveriesInFlight;
    return true;
}

/*-------------------------------------------------------------------------*/

CAndroidRedirectReceiver*
CAndroidAuthBridge::SetRedirectReceiver( CAndroidRedirectReceiver* receiver )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    CAndroidRedirectReceiver* previous = m_redirectReceiver;
    m_redirectReceiver = receiver;
    return previous;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthBridge::DeliverRedirect( const CORE::CString& redirectUrl )
{GUCEF_TRACE;

    CAndroidRedirectReceiver* receiver = GUCEF_NULL;
    {
        MT::CScopeMutex lock( m_lock );
        receiver = m_redirectReceiver;
        if ( GUCEF_NULL != receiver )
            ++m_deliveriesInFlight;
    }
    if ( GUCEF_NULL == receiver )
    {
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "AndroidAuthBridge: A redirect arrived and nothing takes redirects" );
        return false;
    }

    bool taken = receiver->OnAuthRedirect( redirectUrl );

    MT::CScopeMutex lock( m_lock );
    --m_deliveriesInFlight;
    return taken;
}

/*-------------------------------------------------------------------------*/

bool
CAndroidAuthBridge::InstallBrowserLauncher( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( m_launcherInstalled )
        return false;
    m_previousLauncher = CORE::SetBrowserLauncherHandler( &m_launcher );
    m_launcherInstalled = true;
    return true;
}

/*-------------------------------------------------------------------------*/

void
CAndroidAuthBridge::RemoveBrowserLauncher( void )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_launcherInstalled )
        return;
    // Someone may have replaced ours since: theirs stays
    if ( CORE::GetBrowserLauncherHandler() == &m_launcher )
        CORE::SetBrowserLauncherHandler( m_previousLauncher );
    m_previousLauncher = GUCEF_NULL;
    m_launcherInstalled = false;
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
