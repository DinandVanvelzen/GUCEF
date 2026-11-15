/*
 *  gucefWEB: GUCEF module providing WEB application functionality
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

#ifndef GUCEF_TIMEOUT_EXCEPTION_H
#include "gucef_timeout_exception.h"
#define GUCEF_TIMEOUT_EXCEPTION_H
#endif /* GUCEF_TIMEOUT_EXCEPTION_H ? */

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CFUTURERESULTSOURCE_H
#include "gucefCORE_CFutureResultSource.h"
#define GUCEF_CORE_CFUTURERESULTSOURCE_H
#endif /* GUCEF_CORE_CFUTURERESULTSOURCE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#include "gucefWEB_CWebViewCommandResult.h"
#define GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H
#include "gucefWEB_CWebViewSemaphoreEngineLoop.h"
#define GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H
#endif /* GUCEF_WEB_CWEBVIEWSEMAPHOREENGINELOOP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWBASE_H
#include "gucefWEB_CWebViewBase.h"
#define GUCEF_WEB_CWEBVIEWBASE_H
#endif /* GUCEF_WEB_CWEBVIEWBASE_H ? */

#ifndef GUCEF_WEB_CWEBVIEWGLOBAL_H
#include "gucefWEB_CWebViewGlobal.h"
#define GUCEF_WEB_CWEBVIEWGLOBAL_H
#endif /* GUCEF_WEB_CWEBVIEWGLOBAL_H ? */

#include "gucefWEB_CWebViewEngineBase.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace WEB {

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWebViewEngineBase::CWebViewEngineBase( const CString& engineName )
    : IWebViewEngine()
    , m_engineName( engineName )
    , m_lock()
    , m_host()
    , m_loop( GUCEF_NULL )
    , m_views()
    , m_pendingCreations()
    , m_nextViewId( 0 )
    , m_nextRequestId( 0 )
    , m_shuttingDown( 0 )
    , m_abandoned( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewEngineBase::~CWebViewEngineBase()
{GUCEF_TRACE;

    if ( !m_host.IsNULL() && m_host->IsRunning() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngine \"" << m_engineName << "\": destroyed while its engine thread runs: the derived engine must call ShutdownEngine() in its destructor" );
        Shutdown( CWebViewGlobal::Instance()->GetSettings().shutdownTimeoutMs );
    }
    if ( !m_abandoned )
    {
        GUCEF_DELETE m_loop;
        m_loop = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewEngineBase::GetEngineName( void ) const
{GUCEF_TRACE;

    return m_engineName;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewEngineBase::MakeEndedFuture( TWebViewError error, const CString& text )
{GUCEF_TRACE;

    CORE::CFutureResultSource source;
    source.Complete( GUCEF_NEW CWebViewCommandResult( error, text ), true );
    return source.GetFuture();
}

/*-------------------------------------------------------------------------*/

CIWebViewEngineLoop*
CWebViewEngineBase::CreateEngineLoop( void )
{GUCEF_TRACE;

    return GUCEF_NEW CWebViewSemaphoreEngineLoop();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::OnEngineThreadStart( void )
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineBase::OnEngineThreadStop( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::EnsureHost( CString& problem )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    if ( !m_host.IsNULL() )
        return true;
    if ( 0 != m_shuttingDown.Load() )
    {
        problem = "The engine is shutting down";
        return false;
    }
    m_loop = CreateEngineLoop();
    if ( GUCEF_NULL == m_loop )
    {
        problem = "The engine has no loop";
        return false;
    }
    const UInt32 idleWakeMs = CWebViewGlobal::Instance()->GetSettings().engineIdleWakeMs;
    m_host = ( GUCEF_NEW CWebViewEngineHost( this, m_loop, idleWakeMs ) )->CreateSharedPtr();
    if ( m_host.IsNULL() || !m_host->Start( "WebView." + m_engineName ) )
    {
        m_host.Unlink();
        GUCEF_DELETE m_loop;
        m_loop = GUCEF_NULL;
        problem = "The engine thread could not be started";
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewEngineBase::CreateViewAsync( const CWebViewSettings& settings )
{GUCEF_TRACE;

    if ( 0 != m_shuttingDown.Load() )
        return MakeEndedFuture( WEBVIEWERROR_SHUTTING_DOWN, "The web view engine \"" + m_engineName + "\" is shutting down" );
    if ( !IsAvailable() )
        return MakeEndedFuture( WEBVIEWERROR_NOT_SUPPORTED, "The web view engine \"" + m_engineName + "\" is not available: " + GetAvailabilityDescription() );
    if ( WEBVIEWMODE_OFFSCREEN == settings.mode && !GetCapabilities().Has( CWebViewCapabilities::CAP_OFFSCREEN_FRAMES ) )
        return MakeEndedFuture( WEBVIEWERROR_NOT_SUPPORTED, "The web view engine \"" + m_engineName + "\" has no offscreen frames" );
    if ( WEBVIEWMODE_NATIVEWINDOW == settings.mode && !GetCapabilities().Has( CWebViewCapabilities::CAP_NATIVE_WINDOW ) )
        return MakeEndedFuture( WEBVIEWERROR_NOT_SUPPORTED, "The web view engine \"" + m_engineName + "\" has no window of its own" );

    CString problem;
    if ( !EnsureHost( problem ) )
        return MakeEndedFuture( WEBVIEWERROR_SHUTTING_DOWN, problem );

    CWebViewSettings sanitized( settings );
    sanitized.Sanitize( false );
    CWebViewCommand command( CWebViewCommand::CMD_CREATE_VIEW );
    TWebViewEngineHostPtr host;
    {
        MT::CScopeMutex lock( m_lock );
        command.id = ++m_nextRequestId;
        m_pendingCreations[ command.id ] = sanitized;
        host = m_host;
    }
    CORE::CFutureResult future = command.CreateCompletion();
    if ( host.IsNULL() || !host->Post( command ) )
    {
        MT::CScopeMutex lock( m_lock );
        m_pendingCreations.erase( command.id );
        return MakeEndedFuture( !host.IsNULL() && host->IsStalled() ? WEBVIEWERROR_ENGINE_STALLED : WEBVIEWERROR_SHUTTING_DOWN, "The web view engine \"" + m_engineName + "\" takes no commands" );
    }
    return future;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewEngineBase::PostCommand( CWebViewCommand& command )
{GUCEF_TRACE;

    TWebViewEngineHostPtr host;
    {
        MT::CScopeMutex lock( m_lock );
        host = m_host;
    }
    if ( host.IsNULL() )
        return MakeEndedFuture( WEBVIEWERROR_SHUTTING_DOWN, "The web view engine \"" + m_engineName + "\" has no engine thread" );
    CORE::CFutureResult future = command.CreateCompletion();
    if ( !host->Post( command ) )
        return MakeEndedFuture( host->IsStalled() ? WEBVIEWERROR_ENGINE_STALLED : WEBVIEWERROR_SHUTTING_DOWN, "The web view engine \"" + m_engineName + "\" takes no commands" );
    return future;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::PostCommandWithoutResult( const CWebViewCommand& command )
{GUCEF_TRACE;

    TWebViewEngineHostPtr host;
    {
        MT::CScopeMutex lock( m_lock );
        host = m_host;
    }
    return !host.IsNULL() && host->Post( command );
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineBase::ExecuteCommand( CWebViewCommand& command )
{GUCEF_TRACE;

    switch ( command.type )
    {
        case CWebViewCommand::CMD_CREATE_VIEW:
        {
            CWebViewSettings settings;
            bool found = false;
            UInt32 viewId = 0;
            {
                MT::CScopeMutex lock( m_lock );
                TSettingsMap::iterator i = m_pendingCreations.find( command.id );
                if ( i != m_pendingCreations.end() )
                {
                    settings = (*i).second;
                    m_pendingCreations.erase( i );
                    found = true;
                    viewId = ++m_nextViewId;
                }
            }
            CORE::CFutureResultSource* completion = command.GetCompletion();
            if ( !found || 0 != m_shuttingDown.Load() )
            {
                if ( GUCEF_NULL != completion )
                    completion->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_SHUTTING_DOWN, "The view was not created" ), true );
                return;
            }
            CString problem;
            CWebViewBase* view = CreateViewOnEngineThread( settings, viewId, problem );
            if ( GUCEF_NULL == view )
            {
                GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebViewEngine \"" << m_engineName << "\": a view could not be created: " << problem );
                if ( GUCEF_NULL != completion )
                    completion->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_OTHER, problem ), true );
                return;
            }
            TWebViewPtr viewPtr( view );
            {
                MT::CScopeMutex lock( m_lock );
                m_views[ viewId ] = viewPtr;
            }
            view->Created();
            if ( GUCEF_NULL != completion )
            {
                CWebViewCommandResult* result = GUCEF_NEW CWebViewCommandResult();
                result->view = viewPtr;
                completion->Complete( result, true );
            }
            return;
        }
        case CWebViewCommand::CMD_CLOSE_ALL_VIEWS:
        {
            CloseAllViews( WEBVIEWERROR_SHUTTING_DOWN );
            if ( command.HasCompletion() )
                command.GetCompletion()->Complete( GUCEF_NEW CWebViewCommandResult(), true );
            return;
        }
        default:
        {
            TWebViewPtr viewPtr;
            {
                MT::CScopeMutex lock( m_lock );
                TViewMap::iterator i = m_views.find( command.viewId );
                if ( i != m_views.end() )
                    viewPtr = (*i).second;
            }
            if ( viewPtr.IsNULL() )
            {
                if ( command.HasCompletion() )
                    command.GetCompletion()->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_VIEW_CLOSED, "The view is closed" ), true );
                return;
            }
            static_cast< CWebViewBase* >( viewPtr.GetPointerAlways() )->ExecuteCommand( command );
            if ( CWebViewCommand::CMD_CLOSE == command.type )
            {
                MT::CScopeMutex lock( m_lock );
                m_views.erase( command.viewId );
            }
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineBase::CloseAllViews( TWebViewError reason )
{GUCEF_TRACE;

    TViewMap views;
    {
        MT::CScopeMutex lock( m_lock );
        views.swap( m_views );
    }
    for ( TViewMap::iterator i=views.begin(); i!=views.end(); ++i )
        static_cast< CWebViewBase* >( (*i).second.GetPointerAlways() )->CloseOnEngineThread( reason );
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineBase::OnEngineTick( UInt64 nowMs )
{GUCEF_TRACE;

    // views are only added and removed on this thread: the map can be walked without copying it, the lock is only
    // taken against the readers on other threads (watchdog, view count) and is not held while a view runs
    UInt32 previousId = 0;
    for ( ;; )
    {
        TWebViewPtr viewPtr;
        {
            MT::CScopeMutex lock( m_lock );
            TViewMap::iterator i = m_views.upper_bound( previousId );
            if ( i == m_views.end() )
                return;
            previousId = (*i).first;
            viewPtr = (*i).second;
        }
        static_cast< CWebViewBase* >( viewPtr.GetPointerAlways() )->Tick( nowMs );
    }
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEngineBase::GetTickWaitMs( UInt64 nowMs )
{GUCEF_TRACE;

    UInt32 wait = CIWebViewEngineLoop::WaitForever;
    MT::CScopeMutex lock( m_lock );
    for ( TViewMap::iterator i=m_views.begin(); i!=m_views.end(); ++i )
    {
        const UInt32 viewWait = static_cast< CWebViewBase* >( (*i).second.GetPointerAlways() )->GetTickWaitMs( nowMs );
        if ( viewWait < wait )
            wait = viewWait;
    }
    return wait;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::OnEngineThreadStarted( void )
{GUCEF_TRACE;

    return OnEngineThreadStart();
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineBase::OnEngineThreadStopping( void )
{GUCEF_TRACE;

    CloseAllViews( WEBVIEWERROR_SHUTTING_DOWN );
    OnEngineThreadStop();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::Shutdown( UInt32 timeoutMs )
{GUCEF_TRACE;

    if ( IsEngineThread() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngine \"" << m_engineName << "\": Shutdown() cannot be called on the engine thread" );
        return false;
    }
    m_shuttingDown.Store( 1 );
    TWebViewEngineHostPtr host;
    {
        MT::CScopeMutex lock( m_lock );
        host = m_host;
    }
    if ( host.IsNULL() )
        return true;

    const UInt64 start = CWebViewEngineHost::NowMs();
    if ( host->IsRunning() && !host->IsStalled() && !host->IsStopping() )
    {
        // the views are closed on the engine thread, where their native objects live
        CWebViewCommand command( CWebViewCommand::CMD_CLOSE_ALL_VIEWS );
        CORE::CFutureResult future = command.CreateCompletion();
        if ( host->Post( command ) )
        {
            try
            {
                future.Await( (Int32) ( timeoutMs > 0x7FFFFFFF ? 0x7FFFFFFF : timeoutMs ) );
            }
            catch ( const timeout_exception& )
            {
                GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngine \"" << m_engineName << "\": the views did not close within " << timeoutMs << " ms" );
            }
        }
    }
    const UInt64 spent = CWebViewEngineHost::NowMs() - start;
    const UInt32 left = spent >= timeoutMs ? 1 : (UInt32) ( timeoutMs - spent );
    const bool stopped = host->Stop( left );
    if ( !stopped )
    {
        // a stuck engine is left behind: the process is ending, its thread may still use the host and the loop
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "WebViewEngine \"" << m_engineName << "\": the engine thread did not stop and is abandoned" );
        m_abandoned = true;
        TViewMap views;
        {
            MT::CScopeMutex lock( m_lock );
            views.swap( m_views );
        }
        for ( TViewMap::iterator i=views.begin(); i!=views.end(); ++i )
            static_cast< CWebViewBase* >( (*i).second.GetPointerAlways() )->EngineStalled();
        return false;
    }
    {
        MT::CScopeMutex lock( m_lock );
        m_views.clear();
        m_pendingCreations.clear();
        m_host.Unlink();
    }
    GUCEF_DELETE m_loop;
    m_loop = GUCEF_NULL;
    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "WebViewEngine \"" << m_engineName << "\": shut down" );
    return true;
}

/*-------------------------------------------------------------------------*/

void
CWebViewEngineBase::ShutdownEngine( void )
{GUCEF_TRACE;

    Shutdown( CWebViewGlobal::Instance()->GetSettings().shutdownTimeoutMs );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::CanUnload( void ) const
{GUCEF_TRACE;

    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::CheckWatchdog( UInt32 watchdogMs )
{GUCEF_TRACE;

    TWebViewEngineHostPtr host;
    {
        MT::CScopeMutex lock( m_lock );
        host = m_host;
    }
    if ( host.IsNULL() )
        return false;
    bool newlyStalled = false;
    const bool stalled = host->CheckWatchdog( watchdogMs, newlyStalled );
    if ( newlyStalled )
    {
        NotifyObservers( EngineStalledEvent );
        TViewMap views;
        {
            MT::CScopeMutex lock( m_lock );
            views = m_views;
        }
        for ( TViewMap::iterator i=views.begin(); i!=views.end(); ++i )
            static_cast< CWebViewBase* >( (*i).second.GetPointerAlways() )->EngineStalled();
    }
    return stalled;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::IsStalled( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_host.IsNULL() && m_host->IsStalled();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::IsEngineThread( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_host.IsNULL() && m_host->IsEngineThread();
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewEngineBase::GetViewCount( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return (UInt32) m_views.size();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::IsShuttingDown( void ) const
{GUCEF_TRACE;

    return 0 != m_shuttingDown.Load();
}

/*-------------------------------------------------------------------------*/

bool
CWebViewEngineBase::IsRunning( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return !m_host.IsNULL() && m_host->IsRunning();
}

/*-------------------------------------------------------------------------*/

TWebViewEngineHostPtr
CWebViewEngineBase::GetHost( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_lock );
    return m_host;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewEngineBase::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewEngineBase";
    return classTypeName;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
