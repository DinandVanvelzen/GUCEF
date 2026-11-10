/*
 *  guidriverWAYLAND: GUCEF window driver for Wayland: native windows with OpenGL through EGL
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

#include <unistd.h>
#include <limits.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_DVCPPFILEUTILS_H
#include "dvcppfileutils.h"
#define GUCEF_CORE_DVCPPFILEUTILS_H
#endif /* GUCEF_CORE_DVCPPFILEUTILS_H ? */

#ifndef GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#include "pluginglueWAYLAND_CWaylandRuntime.h"
#define GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H
#endif /* GUCEF_PLUGINGLUE_WAYLAND_CWAYLANDRUNTIME_H ? */

#ifndef GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWCONTEXT_H
#include "guidriverWAYLAND_CWaylandWindowContext.h"
#define GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWCONTEXT_H
#endif /* GUCEF_GUIDRIVERWAYLAND_CWAYLANDWINDOWCONTEXT_H ? */

#include "guidriverWAYLAND_CWaylandWindowManagerImp.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWAYLAND {

using namespace PLUGINGLUE::WAYLAND;

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

/** The name of the program, what the desktop knows a window by when the application says nothing else */
CORE::CString
ProgramName( void )
{
    char path[ PATH_MAX ];
    const ssize_t length = ::readlink( "/proc/self/exe", path, sizeof( path ) - 1 );
    if ( length <= 0 )
        return "gucef";
    path[ length ] = '\0';
    return CORE::ExtractFilename( CORE::CString( path ) );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CWaylandWindowManagerImp::CWaylandWindowManagerImp( void )
    : GUI::CWindowManagerBackend()
    , m_connection( GUCEF_NULL )
    , m_runtime( GUCEF_NULL )
    , m_evaluated( false )
    , m_availability()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWaylandWindowManagerImp::CWaylandWindowManagerImp( CWaylandConnection* connection, const SWaylandGuiRuntime* runtime )
    : GUI::CWindowManagerBackend()
    , m_connection( connection )
    , m_runtime( runtime )
    , m_evaluated( false )
    , m_availability()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CWaylandWindowManagerImp::~CWaylandWindowManagerImp()
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

const SWaylandGuiRuntime&
CWaylandWindowManagerImp::GetRuntime( void ) const
{GUCEF_TRACE;

    return GUCEF_NULL != m_runtime ? *m_runtime : GetWaylandGuiRuntime();
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowManagerImp::Evaluate( void ) const
{GUCEF_TRACE;

    if ( m_evaluated )
        return;
    m_evaluated = true;

    // the session and the compositor first: on an X11 session that is the reason worth giving
    if ( GUCEF_NULL != m_connection )
    {
        m_availability = CWaylandAvailability::Decide( true, true, m_connection->IsConnected(), m_connection->GetGlobals(), false );
    }
    else
    {
        const bool sessionFound = !CWaylandAvailability::FindSessionSocket().IsNULLOrEmpty();
        const bool libraryLoaded = sessionFound && GUCEF_NULL != CWaylandRuntime::GetClientApi();
        CORE::CString problem;
        CWaylandConnection* connection = sessionFound && libraryLoaded ? CWaylandConnection::Acquire( &problem ) : GUCEF_NULL;
        if ( GUCEF_NULL != connection )
        {
            m_availability = CWaylandAvailability::Decide( true, true, true, connection->GetGlobals(), false );
            CWaylandConnection::Release( connection );
        }
        else
            m_availability = CWaylandAvailability::Decide( sessionFound, libraryLoaded, false, TWaylandGlobalList(), false );
    }

    if ( m_availability.available )
    {
        const SWaylandGuiRuntime& runtime = GetRuntime();
        if ( !runtime.hasEgl )
        {
            m_availability.available = false;
            m_availability.reason = "libEGL.so.1 is not installed";
        }
        else
        if ( !runtime.hasEglWindow )
        {
            m_availability.available = false;
            m_availability.reason = "libwayland-egl.so.1 is not installed";
        }
    }
    if ( !m_availability.available )
        GUCEF_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowManagerImp: Wayland windows are not available: " << m_availability.reason );
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowManagerImp::IsAvailable( void ) const
{GUCEF_TRACE;

    Evaluate();
    return m_availability.available;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWaylandWindowManagerImp::GetAvailabilityDescription( void ) const
{GUCEF_TRACE;

    Evaluate();
    return m_availability.available ? GUI::CString::Empty : m_availability.reason;
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWaylandWindowManagerImp::CreateWindowContext( const GUI::CString& title ,
                                               const GUI::UInt32 width   ,
                                               const GUI::UInt32 height  ,
                                               const bool fullscreen     )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );
    return CreateWindowContext( title, videoSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWaylandWindowManagerImp::CreateWindowContext( const GUI::CString& title      ,
                                               const GUI::UInt32 width        ,
                                               const GUI::UInt32 height       ,
                                               const bool fullscreen          ,
                                               const CORE::CValueList& params )
{GUCEF_TRACE;

    GUI::CVideoSettings videoSettings;
    videoSettings.SetFullscreenState( fullscreen );
    videoSettings.SetResolution( width, height, 24 );
    return CreateWindowContext( title, videoSettings, &params );
}

/*-------------------------------------------------------------------------*/

GUI::TWindowContextPtr
CWaylandWindowManagerImp::CreateWindowContext( const GUI::CString& title           ,
                                               const GUI::CVideoSettings& settings ,
                                               const CORE::CValueList* params      )
{GUCEF_TRACE;

    if ( !IsAvailable() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowManagerImp: No Wayland window can be made: " << m_availability.reason );
        return GUI::TWindowContextPtr();
    }

    CWaylandConnection* connection = m_connection;
    bool acquired = false;
    if ( GUCEF_NULL == connection )
    {
        CORE::CString problem;
        connection = CWaylandConnection::Acquire( &problem );
        if ( GUCEF_NULL == connection )
        {
            GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CWaylandWindowManagerImp: No connection to the compositor: " << problem );
            return GUI::TWindowContextPtr();
        }
        acquired = true;
    }

    SWaylandWindowParams windowParams;
    windowParams.appId = ProgramName();
    if ( GUCEF_NULL != params )
    {
        const CORE::CString appId = params->GetValueAlways( "APP_ID", "" ).AsString( CORE::CString::Empty, true );
        if ( !appId.IsNULLOrEmpty() )
            windowParams.appId = appId;
        windowParams.decorations = CDecorationPolicy::ParsePreference( params->GetValueAlways( "DECORATIONS", "" ).AsString( CORE::CString::Empty, true ) );
        windowParams.minWidth = params->GetValueAlways( "MIN_WIDTH", "0" ).AsInt32( 0, true );
        windowParams.minHeight = params->GetValueAlways( "MIN_HEIGHT", "0" ).AsInt32( 0, true );
        windowParams.maxWidth = params->GetValueAlways( "MAX_WIDTH", "0" ).AsInt32( 0, true );
        windowParams.maxHeight = params->GetValueAlways( "MAX_HEIGHT", "0" ).AsInt32( 0, true );
    }

    TWaylandWindowContextPtr windowContext( GUCEF_NEW CWaylandWindowContext( *connection, GetRuntime() ) );
    if ( !windowContext->Initialize( title, settings, windowParams ) )
    {
        windowContext.Unlink();
        if ( acquired )
            CWaylandConnection::Release( connection );
        return GUI::TWindowContextPtr();
    }
    windowContext->SetReleaseConnectionOnShutdown( acquired );
    return windowContext;
}

/*-------------------------------------------------------------------------*/

void
CWaylandWindowManagerImp::DestroyWindowContext( GUI::TWindowContextPtr& windowContext )
{GUCEF_TRACE;

    // the contexts that are handed to this manager were made by it
    CWaylandWindowContext* window = static_cast< CWaylandWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL != window )
        window->Shutdown();
}

/*-------------------------------------------------------------------------*/

bool
CWaylandWindowManagerImp::ApplyVideoSettings( GUI::TWindowContextPtr& windowContext ,
                                              const GUI::CVideoSettings& settings   )
{GUCEF_TRACE;

    CWaylandWindowContext* window = static_cast< CWaylandWindowContext* >( windowContext.GetPointerAlways() );
    if ( GUCEF_NULL == window )
        return false;
    return window->ApplyVideoSettings( settings );
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVERWAYLAND */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
