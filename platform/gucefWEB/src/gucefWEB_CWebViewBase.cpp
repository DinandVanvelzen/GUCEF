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

#ifndef GUCEF_MT_CSCOPEMUTEX_H
#include "gucefMT_CScopeMutex.h"
#define GUCEF_MT_CSCOPEMUTEX_H
#endif /* GUCEF_MT_CSCOPEMUTEX_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_WEB_WEBVIEWTYPENAMES_H
#include "gucefWEB_WebViewTypeNames.h"
#define GUCEF_WEB_WEBVIEWTYPENAMES_H
#endif /* GUCEF_WEB_WEBVIEWTYPENAMES_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#include "gucefWEB_CWebViewCommandResult.h"
#define GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H
#endif /* GUCEF_WEB_CWEBVIEWCOMMANDRESULT_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEBASE_H
#include "gucefWEB_CWebViewEngineBase.h"
#define GUCEF_WEB_CWEBVIEWENGINEBASE_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEBASE_H ? */

#ifndef GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#include "gucefWEB_CIWebViewEngineLoop.h"
#define GUCEF_WEB_CIWEBVIEWENGINELOOP_H
#endif /* GUCEF_WEB_CIWEBVIEWENGINELOOP_H ? */

#ifndef GUCEF_WEB_CWEBVIEWENGINEHOST_H
#include "gucefWEB_CWebViewEngineHost.h"
#define GUCEF_WEB_CWEBVIEWENGINEHOST_H
#endif /* GUCEF_WEB_CWEBVIEWENGINEHOST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWLOADERROR_H
#include "gucefWEB_CWebViewLoadError.h"
#define GUCEF_WEB_CWEBVIEWLOADERROR_H
#endif /* GUCEF_WEB_CWEBVIEWLOADERROR_H ? */

#ifndef GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H
#include "gucefWEB_CWebViewNavigationRequest.h"
#define GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWNAVIGATIONREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H
#include "gucefWEB_CWebViewPopupRequest.h"
#define GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWPOPUPREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H
#include "gucefWEB_CWebViewDownloadRequest.h"
#define GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWDOWNLOADREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H
#include "gucefWEB_CWebViewPermissionRequest.h"
#define GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H
#endif /* GUCEF_WEB_CWEBVIEWPERMISSIONREQUEST_H ? */

#ifndef GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H
#include "gucefWEB_CWebViewScriptDialog.h"
#define GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H
#endif /* GUCEF_WEB_CWEBVIEWSCRIPTDIALOG_H ? */

#ifndef GUCEF_WEB_CWEBVIEWCONSOLEMESSAGE_H
#include "gucefWEB_CWebViewConsoleMessage.h"
#define GUCEF_WEB_CWEBVIEWCONSOLEMESSAGE_H
#endif /* GUCEF_WEB_CWEBVIEWCONSOLEMESSAGE_H ? */

#include "gucefWEB_CWebViewBase.h"

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

namespace {

/** The scheme, host and path of an address: what may be logged of it (the query may hold codes and states) */
CString
LoggableUrl( const CString& url )
{
    Int32 end = url.HasChar( '?' );
    const Int32 fragment = url.HasChar( '#' );
    if ( fragment >= 0 && ( end < 0 || fragment < end ) )
        end = fragment;
    return end >= 0 ? url.SubstrToIndex( (UInt32) end, true ) : url;
}

TWebViewSecurityState
SecurityOfUrl( const CString& url )
{
    const CString lower = url.Lowercase();
    if ( lower.StartsWith( "https://" ) )
        return WEBVIEWSECURITY_SECURE;
    if ( lower.StartsWith( "http://" ) )
        return WEBVIEWSECURITY_INSECURE;
    return WEBVIEWSECURITY_NONE;
}

}

/*-------------------------------------------------------------------------*/

CWebViewBase::CWebViewBase( CWebViewEngineBase& engine, UInt32 viewId, const CWebViewSettings& settings, const CWebViewCapabilities& capabilities )
    : IWebView()
    , m_engine( &engine )
    , m_engineName( engine.GetEngineName() )
    , m_viewId( viewId )
    , m_settings( settings )
    , m_capabilities( capabilities )
    , m_frames()
    , m_closed( 0 )
    , m_stateLock()
    , m_url( "about:blank" )
    , m_title()
    , m_progress( 0.0f )
    , m_loading( false )
    , m_security( WEBVIEWSECURITY_NONE )
    , m_cursor( WEBVIEWCURSOR_ARROW )
    , m_width( settings.width )
    , m_height( settings.height )
    , m_scale( settings.deviceScale )
    , m_pendingNavigation( GUCEF_NULL )
    , m_decisions()
    , m_nextDecisionId( 0 )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWebViewBase::~CWebViewBase()
{GUCEF_TRACE;

    if ( GUCEF_NULL != m_pendingNavigation )
    {
        m_pendingNavigation->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_VIEW_CLOSED, "The view is gone" ), true );
        GUCEF_DELETE m_pendingNavigation;
        m_pendingNavigation = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

CWebViewEngineBase&
CWebViewBase::GetEngine( void )
{GUCEF_TRACE;

    return *m_engine;
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::Post( CWebViewCommand& command )
{GUCEF_TRACE;

    command.viewId = m_viewId;
    MT::CScopeMutex lock( m_stateLock );
    if ( 0 != m_closed.Load() || GUCEF_NULL == m_engine )
        return CWebViewEngineBase::MakeEndedFuture( WEBVIEWERROR_VIEW_CLOSED, "The view is closed" );
    return m_engine->PostCommand( command );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::PostWithoutResult( CWebViewCommand& command )
{GUCEF_TRACE;

    command.viewId = m_viewId;
    MT::CScopeMutex lock( m_stateLock );
    if ( 0 != m_closed.Load() || GUCEF_NULL == m_engine )
        return;
    m_engine->PostCommandWithoutResult( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::Navigate( const CString& url )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_NAVIGATE );
    command.text = url;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::Reload( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_RELOAD );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::Stop( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_STOP );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::GoBack( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_GO_BACK );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::GoForward( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_GO_FORWARD );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::EvaluateScript( const CString& script, const CString& world )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_EVALUATE_SCRIPT );
    command.text = script;
    command.text2 = world;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::GetCookies( const CString& urlFilter )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_GET_COOKIES );
    command.text = urlFilter;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::ClearCookies( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_CLEAR_COOKIES );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::SetCookie( const CWebViewCookie& cookie )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_SET_COOKIE );
    command.cookie = cookie;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::SetSize( UInt32 logicalWidth, UInt32 logicalHeight, Float32 deviceScale )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_SET_SIZE );
    command.number1 = logicalWidth;
    command.number2 = logicalHeight;
    command.scale = deviceScale;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::SetVisible( bool visible )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_SET_VISIBLE );
    command.flag = visible;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::SetFocus( bool focused )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_SET_FOCUS );
    command.flag = focused;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::InjectInput( const CWebViewInputEvent& inputEvent )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_INJECT_INPUT );
    command.input = inputEvent;
    PostWithoutResult( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::SetZoom( Float32 factor )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_SET_ZOOM );
    command.scale = factor;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::ResolveDecision( UInt32 decisionId, bool allow, const CString& text )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_RESOLVE_DECISION );
    command.id = decisionId;
    command.flag = allow;
    command.text = text;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::ExecuteEditCommand( TWebViewEditCommand editCommand )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_EDIT_COMMAND );
    command.number1 = (UInt32) editCommand;
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::CaptureFrame( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_CAPTURE_FRAME );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CORE::CFutureResult
CWebViewBase::Close( void )
{GUCEF_TRACE;

    CWebViewCommand command( CWebViewCommand::CMD_CLOSE );
    return Post( command );
}

/*-------------------------------------------------------------------------*/

CString
CWebViewBase::GetUrl( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_url;
}

/*-------------------------------------------------------------------------*/

CString
CWebViewBase::GetTitle( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_title;
}

/*-------------------------------------------------------------------------*/

Float32
CWebViewBase::GetLoadProgress( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_progress;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewBase::IsLoading( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_loading;
}

/*-------------------------------------------------------------------------*/

TWebViewSecurityState
CWebViewBase::GetSecurityState( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_security;
}

/*-------------------------------------------------------------------------*/

TWebViewCursorShape
CWebViewBase::GetCursorShape( void ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    return m_cursor;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewBase::IsClosed( void ) const
{GUCEF_TRACE;

    return 0 != m_closed.Load();
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::GetSize( UInt32& logicalWidth, UInt32& logicalHeight, Float32& deviceScale ) const
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    logicalWidth = m_width;
    logicalHeight = m_height;
    deviceScale = m_scale;
}

/*-------------------------------------------------------------------------*/

CWebViewFrameExchange&
CWebViewBase::GetFrameExchange( void )
{GUCEF_TRACE;

    return m_frames;
}

/*-------------------------------------------------------------------------*/

const CWebViewSettings&
CWebViewBase::GetSettings( void ) const
{GUCEF_TRACE;

    return m_settings;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewBase::GetEngineName( void ) const
{GUCEF_TRACE;

    return m_engineName;
}

/*-------------------------------------------------------------------------*/

CWebViewCapabilities
CWebViewBase::GetCapabilities( void ) const
{GUCEF_TRACE;

    return m_capabilities;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewBase::GetViewId( void ) const
{GUCEF_TRACE;

    return m_viewId;
}

/*-------------------------------------------------------------------------*/

const CString&
CWebViewBase::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CString classTypeName = "GUCEF::WEB::CWebViewBase";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::Created( void )
{GUCEF_TRACE;

    OnCreated();
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::OnCreated( void )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::OnTick( UInt64 nowMs )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::ExecuteCommand( CWebViewCommand& command )
{GUCEF_TRACE;

    if ( 0 != m_closed.Load() )
    {
        FailCommand( command, WEBVIEWERROR_VIEW_CLOSED, "The view is closed" );
        return;
    }

    switch ( command.type )
    {
        case CWebViewCommand::CMD_NAVIGATE:
        {
            if ( WEBVIEWDECISION_ALLOW != CheckNavigation( command.text, true, false, true ) )
            {
                FailCommand( command, WEBVIEWERROR_REFUSED_BY_POLICY, "The policy of the view does not allow the address" );
                return;
            }
            CancelPendingNavigation( WEBVIEWERROR_CANCELLED, "Replaced by another navigation" );
            if ( command.HasCompletion() )
                m_pendingNavigation = GUCEF_NEW CORE::CFutureResultSource( *command.GetCompletion() );
            OnNavigate( command.text );
            return;
        }
        case CWebViewCommand::CMD_RELOAD:
        {
            if ( OnReload() )
                CompleteCommand( command );
            else
                FailCommand( command, WEBVIEWERROR_NOT_SUPPORTED, "Nothing to reload" );
            return;
        }
        case CWebViewCommand::CMD_STOP:
        {
            OnStop();
            CancelPendingNavigation( WEBVIEWERROR_CANCELLED, "Stopped" );
            SetLoading( false );
            CompleteCommand( command );
            return;
        }
        case CWebViewCommand::CMD_GO_BACK:
        {
            if ( OnGoBack() )
                CompleteCommand( command );
            else
                FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "There is no page to go back to" );
            return;
        }
        case CWebViewCommand::CMD_GO_FORWARD:
        {
            if ( OnGoForward() )
                CompleteCommand( command );
            else
                FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "There is no page to go forward to" );
            return;
        }
        case CWebViewCommand::CMD_EVALUATE_SCRIPT:
        {
            if ( !m_settings.policy.IsScriptWorldAllowed( command.text2 ) )
            {
                FailCommand( command, WEBVIEWERROR_REFUSED_BY_POLICY, "Scripts of the application run in an isolated world only" );
                return;
            }
            if ( !m_capabilities.Has( CWebViewCapabilities::CAP_SCRIPT_RESULT ) )
            {
                FailCommand( command, WEBVIEWERROR_NOT_SUPPORTED, "The engine evaluates no scripts" );
                return;
            }
            OnEvaluateScript( command.text, command.text2, command );
            return;
        }
        case CWebViewCommand::CMD_GET_COOKIES:
        {
            if ( !m_capabilities.Has( CWebViewCapabilities::CAP_COOKIES ) )
            {
                CompleteCookies( command, TWebViewCookieVector() );
                return;
            }
            OnGetCookies( command.text, command );
            return;
        }
        case CWebViewCommand::CMD_CLEAR_COOKIES:
        {
            if ( !m_capabilities.Has( CWebViewCapabilities::CAP_COOKIES ) )
            {
                CompleteCommand( command );
                return;
            }
            OnClearCookies( command );
            return;
        }
        case CWebViewCommand::CMD_SET_COOKIE:
        {
            if ( !m_capabilities.Has( CWebViewCapabilities::CAP_COOKIES ) )
            {
                FailCommand( command, WEBVIEWERROR_NOT_SUPPORTED, "The engine keeps no cookies" );
                return;
            }
            if ( command.cookie.name.IsNULLOrEmpty() || command.cookie.domain.IsNULLOrEmpty() )
            {
                FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "A cookie needs a name and a domain" );
                return;
            }
            OnSetCookie( command.cookie, command );
            return;
        }
        case CWebViewCommand::CMD_SET_SIZE:
        {
            UInt32 width = command.number1;
            UInt32 height = command.number2;
            Float32 scale = command.scale;
            if ( 0 == width || 0 == height || width > CWebViewSettings::MaxSize || height > CWebViewSettings::MaxSize || !( scale >= 0.25f && scale <= 8.0f ) )
            {
                FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "The size or scale is not valid" );
                return;
            }
            {
                MT::CScopeMutex lock( m_stateLock );
                m_width = width;
                m_height = height;
                m_scale = scale;
            }
            OnSetSize( width, height, scale );
            CompleteCommand( command );
            return;
        }
        case CWebViewCommand::CMD_SET_VISIBLE:
        {
            OnSetVisible( command.flag );
            CompleteCommand( command );
            return;
        }
        case CWebViewCommand::CMD_SET_FOCUS:
        {
            OnSetFocus( command.flag );
            CompleteCommand( command );
            return;
        }
        case CWebViewCommand::CMD_INJECT_INPUT:
        {
            OnInjectInput( command.input );
            CompleteCommand( command );
            return;
        }
        case CWebViewCommand::CMD_SET_ZOOM:
        {
            if ( !( command.scale > 0.0f && command.scale <= 10.0f ) )
                FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "The zoom factor is not valid" );
            else
            if ( OnSetZoom( command.scale ) )
                CompleteCommand( command );
            else
                FailCommand( command, WEBVIEWERROR_NOT_SUPPORTED, "The engine has no zoom" );
            return;
        }
        case CWebViewCommand::CMD_RESOLVE_DECISION:
        {
            TDecisionMap::iterator i = m_decisions.find( command.id );
            if ( i == m_decisions.end() )
            {
                FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "The decision is not open (answered, timed out or never asked)" );
                return;
            }
            const TWebViewDecisionKind kind = (*i).second.kind;
            m_decisions.erase( i );
            OnDecisionResolved( command.id, kind, command.flag, command.text, false );
            CompleteCommand( command );
            return;
        }
        case CWebViewCommand::CMD_EDIT_COMMAND:
        {
            CString copied;
            if ( OnEditCommand( (TWebViewEditCommand) command.number1, copied ) )
                CompleteCommand( command, copied );
            else
                FailCommand( command, WEBVIEWERROR_NOT_SUPPORTED, "The engine cannot carry out the edit command" );
            return;
        }
        case CWebViewCommand::CMD_CAPTURE_FRAME:
        {
            IMAGE::TPixelMapPtr frame( GUCEF_NEW IMAGE::CPixelMap( GUCEF_NULL, 1, 1, IMAGE::PSF_RGBA, GUCEF::DATATYPE_UINT8 ) );
            if ( !m_frames.CopyLastPublished( *frame ) )
            {
                FailCommand( command, WEBVIEWERROR_NOT_SUPPORTED, "The view has no frame yet" );
                return;
            }
            if ( command.HasCompletion() )
            {
                CWebViewCommandResult* result = GUCEF_NEW CWebViewCommandResult();
                result->frame = frame;
                command.GetCompletion()->Complete( result, true );
            }
            return;
        }
        case CWebViewCommand::CMD_CLOSE:
        {
            CloseOnEngineThread( WEBVIEWERROR_VIEW_CLOSED );
            CompleteCommand( command );
            return;
        }
        default:
        {
            FailCommand( command, WEBVIEWERROR_INVALID_ARGUMENT, "Not a command of a view" );
            return;
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::Tick( UInt64 nowMs )
{GUCEF_TRACE;

    if ( 0 != m_closed.Load() )
        return;
    TDecisionMap::iterator i = m_decisions.begin();
    while ( i != m_decisions.end() )
    {
        if ( 0 != (*i).second.deadlineMs && nowMs >= (*i).second.deadlineMs )
        {
            const UInt32 id = (*i).first;
            const TWebViewDecisionKind kind = (*i).second.kind;
            m_decisions.erase( i );
            GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebView " << m_viewId << ": decision " << id << " was not answered in time and is refused" );
            OnDecisionResolved( id, kind, false, CString::Empty, true );
            i = m_decisions.lower_bound( id );
            continue;
        }
        ++i;
    }
    OnTick( nowMs );
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewBase::GetTickWaitMs( UInt64 nowMs ) const
{GUCEF_TRACE;

    UInt32 wait = CIWebViewEngineLoop::WaitForever;
    for ( TDecisionMap::const_iterator i=m_decisions.begin(); i!=m_decisions.end(); ++i )
    {
        const UInt64 deadline = (*i).second.deadlineMs;
        if ( 0 == deadline )
            continue;
        const UInt32 left = deadline <= nowMs ? 0 : ( deadline - nowMs > 0xFFFFFFF0 ? 0xFFFFFFF0 : (UInt32) ( deadline - nowMs ) );
        if ( left < wait )
            wait = left;
    }
    return wait;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewBase::GetOpenDecisionCount( void ) const
{GUCEF_TRACE;

    return (UInt32) m_decisions.size();
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::CloseOnEngineThread( TWebViewError reason )
{GUCEF_TRACE;

    if ( 0 != m_closed.Exchange( 1 ) )
        return;
    OnClose();
    CancelPendingNavigation( reason, "The view is closed" );
    m_decisions.clear();
    {
        MT::CScopeMutex lock( m_stateLock );
        m_engine = GUCEF_NULL;
        m_loading = false;
    }
    NotifyObservers( ClosedEvent );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::EngineStalled( void )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        m_engine = GUCEF_NULL;
        m_loading = false;
    }
    m_closed.Store( 1 );
    NotifyObservers( EngineStalledEvent );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::CompleteCommand( CWebViewCommand& command, const CString& text )
{GUCEF_TRACE;

    if ( command.HasCompletion() )
        command.GetCompletion()->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_NONE, text ), true );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::FailCommand( CWebViewCommand& command, TWebViewError error, const CString& message )
{GUCEF_TRACE;

    if ( command.HasCompletion() )
        command.GetCompletion()->Complete( GUCEF_NEW CWebViewCommandResult( error, message ), true );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::CompleteScript( CWebViewCommand& command, const CString& json )
{GUCEF_TRACE;

    const UInt32 bytes = json.IsNULLOrEmpty() ? 0 : json.ByteSize() - 1;
    if ( 0 != m_settings.maxScriptResultBytes && bytes > m_settings.maxScriptResultBytes )
    {
        FailCommand( command, WEBVIEWERROR_RESULT_TOO_LARGE, "The result of the script is larger than the view allows" );
        return;
    }
    CompleteCommand( command, json );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::CompleteCookies( CWebViewCommand& command, const TWebViewCookieVector& cookies )
{GUCEF_TRACE;

    if ( !command.HasCompletion() )
        return;
    CWebViewCommandResult* result = GUCEF_NEW CWebViewCommandResult();
    result->cookies = cookies;
    command.GetCompletion()->Complete( result, true );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::CancelPendingNavigation( TWebViewError error, const CString& message )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_pendingNavigation )
        return;
    m_pendingNavigation->Complete( GUCEF_NEW CWebViewCommandResult( error, message ), true );
    GUCEF_DELETE m_pendingNavigation;
    m_pendingNavigation = GUCEF_NULL;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewBase::HasPendingNavigation( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_pendingNavigation;
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::SetLoading( bool loading )
{GUCEF_TRACE;

    MT::CScopeMutex lock( m_stateLock );
    m_loading = loading;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewBase::CheckNavigation( const CString& url, bool mainFrame, bool userGesture, bool byApplication )
{GUCEF_TRACE;

    const TWebViewDecision decision = m_settings.policy.DecideNavigation( url, mainFrame, byApplication );
    CWebViewNavigationRequest request;
    request.url = url;
    request.mainFrame = mainFrame;
    request.userGesture = userGesture;
    request.byApplication = byApplication;
    request.decision = decision;
    TWebViewNavigationRequestEventData eventData( request );
    NotifyObservers( NavigationRequestedEvent, &eventData );

    if ( WEBVIEWDECISION_ALLOW != decision )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "WebView " << m_viewId << ": refused a navigation to " << LoggableUrl( url ) );
        if ( mainFrame )
        {
            CWebViewLoadError error;
            error.url = url;
            error.error = WEBVIEWERROR_REFUSED_BY_POLICY;
            error.message = "The address is not allowed in this view";
            TWebViewLoadErrorEventData errorData( error );
            NotifyObservers( LoadFailedEvent, &errorData );
            // a navigation of the page itself (a redirect, a link) ends the navigation that is pending; the base refuses one of the application itself
            if ( !byApplication )
                CancelPendingNavigation( WEBVIEWERROR_REFUSED_BY_POLICY, error.message );
            SetLoading( false );
        }
    }
    return decision;
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyLoadStarted( const CString& url )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        m_loading = true;
        m_progress = 0.0f;
    }
    TUrlEventData eventData( url );
    NotifyObservers( LoadStartedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyLoadCommitted( const CString& url )
{GUCEF_TRACE;

    bool urlChanged = false;
    bool securityChanged = false;
    const TWebViewSecurityState security = SecurityOfUrl( url );
    {
        MT::CScopeMutex lock( m_stateLock );
        urlChanged = m_url != url;
        m_url = url;
        securityChanged = m_security != security;
        m_security = security;
        m_loading = true;
    }
    TUrlEventData eventData( url );
    NotifyObservers( LoadCommittedEvent, &eventData );
    if ( urlChanged )
        NotifyObservers( UrlChangedEvent, &eventData );
    if ( securityChanged )
    {
        TStateEventData stateData( (Int32) security );
        NotifyObservers( SecurityStateChangedEvent, &stateData );
    }
    if ( GUCEF_NULL != m_pendingNavigation )
    {
        m_pendingNavigation->Complete( GUCEF_NEW CWebViewCommandResult( WEBVIEWERROR_NONE, url ), true );
        GUCEF_DELETE m_pendingNavigation;
        m_pendingNavigation = GUCEF_NULL;
    }
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyLoadFinished( const CString& url )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        m_loading = false;
        m_progress = 1.0f;
    }
    TProgressEventData progressData( 1.0f );
    NotifyObservers( ProgressChangedEvent, &progressData );
    TUrlEventData eventData( url );
    NotifyObservers( LoadFinishedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyLoadFailed( const CString& url, TWebViewError error, const CString& message )
{GUCEF_TRACE;

    SetLoading( false );
    CWebViewLoadError failure;
    failure.url = url;
    failure.error = error;
    failure.message = message;
    TWebViewLoadErrorEventData eventData( failure );
    NotifyObservers( LoadFailedEvent, &eventData );
    CancelPendingNavigation( error, message );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyTitleChanged( const CString& title )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        if ( m_title == title )
            return;
        m_title = title;
    }
    TTextEventData eventData( title );
    NotifyObservers( TitleChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyUrlChanged( const CString& url )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        if ( m_url == url )
            return;
        m_url = url;
    }
    TUrlEventData eventData( url );
    NotifyObservers( UrlChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyProgress( Float32 progress )
{GUCEF_TRACE;

    if ( progress < 0.0f )
        progress = 0.0f;
    if ( progress > 1.0f )
        progress = 1.0f;
    {
        MT::CScopeMutex lock( m_stateLock );
        m_progress = progress;
    }
    TProgressEventData eventData( progress );
    NotifyObservers( ProgressChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifyCursorChanged( TWebViewCursorShape shape )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        if ( m_cursor == shape )
            return;
        m_cursor = shape;
    }
    TStateEventData eventData( (Int32) shape );
    NotifyObservers( CursorChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::NotifySecurityState( TWebViewSecurityState state )
{GUCEF_TRACE;

    {
        MT::CScopeMutex lock( m_stateLock );
        if ( m_security == state )
            return;
        m_security = state;
    }
    TStateEventData eventData( (Int32) state );
    NotifyObservers( SecurityStateChangedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

bool
CWebViewBase::PublishFrame( const IMAGE::CPixelMap& frame, const CWebViewRect& dirty )
{GUCEF_TRACE;

    // at most one FrameAvailableEvent until the reader took the frame: a waiting frame is replaced silently
    const bool previousTaken = !m_frames.HasNewFrame();
    Float32 scale = 1.0f;
    {
        MT::CScopeMutex lock( m_stateLock );
        scale = m_scale;
    }
    if ( !m_frames.UpdateFromRgba( frame, dirty, scale ) )
        return false;
    if ( previousTaken )
    {
        TWebViewRectEventData eventData( dirty );
        NotifyObservers( FrameAvailableEvent, &eventData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWebViewBase::PublishFrameBgraPremultiplied( const UInt8* pixels, UInt32 strideInBytes, UInt32 width, UInt32 height, const CWebViewRect& dirty )
{GUCEF_TRACE;

    const bool previousTaken = !m_frames.HasNewFrame();
    Float32 scale = 1.0f;
    {
        MT::CScopeMutex lock( m_stateLock );
        scale = m_scale;
    }
    if ( !m_frames.UpdateFromBgraPremultiplied( pixels, strideInBytes, width, height, dirty, scale ) )
        return false;
    if ( previousTaken )
    {
        TWebViewRectEventData eventData( dirty );
        NotifyObservers( FrameAvailableEvent, &eventData );
    }
    return true;
}

/*-------------------------------------------------------------------------*/

UInt32
CWebViewBase::DeferDecision( TWebViewDecisionKind kind )
{GUCEF_TRACE;

    SDecision decision;
    decision.kind = kind;
    decision.deadlineMs = 0 == m_settings.decisionTimeoutMs ? 0 : CWebViewEngineHost::NowMs() + m_settings.decisionTimeoutMs;
    const UInt32 id = ++m_nextDecisionId;
    m_decisions[ id ] = decision;
    return id;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewBase::RequestPopup( const CString& url, UInt32& decisionId )
{GUCEF_TRACE;

    decisionId = 0;
    const TWebViewDecision decision = m_settings.policy.DecidePopup( url );
    if ( WEBVIEWDECISION_ASK == decision )
        decisionId = DeferDecision( WEBVIEWDECISIONKIND_POPUP );
    CWebViewPopupRequest request;
    request.url = url;
    request.decision = decision;
    request.decisionId = decisionId;
    TWebViewPopupRequestEventData eventData( request );
    NotifyObservers( PopupRequestedEvent, &eventData );
    return decision;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewBase::RequestDownload( const CString& url, const CString& suggestedName, UInt32& decisionId )
{GUCEF_TRACE;

    decisionId = 0;
    const TWebViewDecision decision = m_settings.policy.DecideDownload();
    if ( WEBVIEWDECISION_ASK == decision )
        decisionId = DeferDecision( WEBVIEWDECISIONKIND_DOWNLOAD );
    CWebViewDownloadRequest request;
    request.url = url;
    request.suggestedName = suggestedName;
    request.decision = decision;
    request.decisionId = decisionId;
    TWebViewDownloadRequestEventData eventData( request );
    NotifyObservers( DownloadRequestedEvent, &eventData );
    return decision;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewBase::RequestPermission( TWebViewPermissionKind kind, const CString& origin, UInt32& decisionId )
{GUCEF_TRACE;

    decisionId = 0;
    const TWebViewDecision decision = m_settings.policy.DecidePermission( kind );
    if ( WEBVIEWDECISION_ASK == decision )
        decisionId = DeferDecision( WEBVIEWDECISIONKIND_PERMISSION );
    CWebViewPermissionRequest request;
    request.kind = kind;
    request.origin = origin;
    request.decision = decision;
    request.decisionId = decisionId;
    TWebViewPermissionRequestEventData eventData( request );
    NotifyObservers( PermissionRequestedEvent, &eventData );
    return decision;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewBase::ReportCertificateError( const CWebViewCertificateError& certificateError, UInt32& decisionId )
{GUCEF_TRACE;

    decisionId = 0;
    const TWebViewDecision decision = m_settings.policy.DecideCertificateError();
    if ( WEBVIEWDECISION_ASK == decision )
        decisionId = DeferDecision( WEBVIEWDECISIONKIND_CERTIFICATE_ERROR );
    NotifySecurityState( WEBVIEWSECURITY_CERTIFICATE_ERROR );
    CWebViewCertificateError report( certificateError );
    report.decision = decision;
    report.decisionId = decisionId;
    TWebViewCertificateErrorEventData eventData( report );
    NotifyObservers( CertificateErrorEvent, &eventData );
    return decision;
}

/*-------------------------------------------------------------------------*/

TWebViewDecision
CWebViewBase::ShowScriptDialog( TWebViewScriptDialogKind kind, const CString& message, const CString& defaultPromptText, UInt32& decisionId )
{GUCEF_TRACE;

    decisionId = 0;
    const TWebViewDecision decision = m_settings.policy.DecideScriptDialog();
    if ( WEBVIEWDECISION_ASK == decision )
        decisionId = DeferDecision( WEBVIEWDECISIONKIND_SCRIPT_DIALOG );
    CWebViewScriptDialog dialog;
    dialog.kind = kind;
    dialog.message = message;
    dialog.defaultPromptText = defaultPromptText;
    dialog.decision = decision;
    dialog.decisionId = decisionId;
    TWebViewScriptDialogEventData eventData( dialog );
    NotifyObservers( ScriptDialogEvent, &eventData );
    return decision;
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::ReportConsoleMessage( TWebViewConsoleLevel level, const CString& text, const CString& source, UInt32 line )
{GUCEF_TRACE;

    if ( !m_settings.consoleMessages )
        return;
    CWebViewConsoleMessage message;
    message.level = level;
    message.text = text;
    message.source = LoggableUrl( source );
    message.line = line;
    TWebViewConsoleMessageEventData eventData( message );
    NotifyObservers( ConsoleMessageEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::ReportProcessTerminated( TWebViewTermination reason )
{GUCEF_TRACE;

    GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "WebView " << m_viewId << ": the web process of the engine ended (" << WebViewTerminationToString( reason ) << ")" );
    SetLoading( false );
    CancelPendingNavigation( WEBVIEWERROR_PROCESS_ENDED, "The web process of the engine ended" );
    TStateEventData eventData( (Int32) reason );
    NotifyObservers( ProcessTerminatedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::RequestFocusLeave( bool forward )
{GUCEF_TRACE;

    TFlagEventData eventData( forward );
    NotifyObservers( FocusLeaveRequestedEvent, &eventData );
}

/*-------------------------------------------------------------------------*/

void
CWebViewBase::RequestClose( void )
{GUCEF_TRACE;

    NotifyObservers( CloseRequestedEvent );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace WEB */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
