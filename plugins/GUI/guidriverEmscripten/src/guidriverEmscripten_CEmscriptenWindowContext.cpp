/*
 *  guidriverEmscripten: GUCEF window driver for web browsers: a canvas with WebGL 2
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
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_CORE_CVALUELIST_H
#include "CValueList.h"
#define GUCEF_CORE_CVALUELIST_H
#endif /* GUCEF_CORE_CVALUELIST_H ? */

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

#include "guidriverEmscripten_CEmscriptenWindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

/* JavaScript helpers, of the page the application runs in */

EM_JS( int, guidriverEmscripten_CanvasExists, ( const char* selector ), {
    return document.querySelector( UTF8ToString( selector ) ) ? 1 : 0;
} );

EM_JS( void, guidriverEmscripten_SetCanvasCursor, ( const char* selector, int visible ), {
    var canvas = document.querySelector( UTF8ToString( selector ) );
    if ( canvas ) canvas.style.cursor = visible ? "" : "none";
} );

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVEREMSCRIPTEN {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUI::UInt32 g_nextWindowId = 1;

/*-------------------------------------------------------------------------*/

/** The function GL_GETPROCADDRESS hands out; WebGL functions are linked in, renderers on the web need no loader */
void*
GetProcAddress( const char* name )
{
    return ::emscripten_webgl_get_proc_address( name );
}

}

/*-------------------------------------------------------------------------//
//                                                                         //
//      IMPLEMENTATION                                                     //
//                                                                         //
//-------------------------------------------------------------------------*/

CEmscriptenWindowContext::CEmscriptenWindowContext( void )
    : GUI::CWindowContext()
    , m_guiContext()
    , m_id( g_nextWindowId++ )
    , m_name()
    , m_canvas()
    , m_width( 0 )
    , m_height( 0 )
    , m_webglContext( 0 )
    , m_fullscreen( false )
    , m_drivesRedraws( false )
{GUCEF_TRACE;

}

/*-------------------------------------------------------------------------*/

CEmscriptenWindowContext::~CEmscriptenWindowContext()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenWindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CEmscriptenWindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CEmscriptenWindowContext::GetWidth( void ) const
{GUCEF_TRACE;

    return m_width;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CEmscriptenWindowContext::GetHeight( void ) const
{GUCEF_TRACE;

    return m_height;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CEmscriptenWindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return 0 != m_webglContext;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CEmscriptenWindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CEmscriptenWindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( "WINDOW" == propertyName )
        return m_canvas;
    if ( "GL_FRAMEBUFFER" == propertyName )
        return "0";
    if ( "GL_GETPROCADDRESS" == propertyName )
        return CORE::PointerToString( (const void*) &GetProcAddress );
    if ( "GL_API" == propertyName )
        return "GLES";
    if ( "GL_VERSION" == propertyName )
        return "3.0";
    return GUI::CString::Empty;
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowContext::SetSystemCursorVisible( bool visible )
{GUCEF_TRACE;

    if ( m_canvas.IsNULLOrEmpty() )
        return false;
    guidriverEmscripten_SetCanvasCursor( m_canvas.C_String(), visible ? 1 : 0 );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowContext::Initialize( const GUI::CString& title                ,
                                      const GUI::CVideoSettings& videoSettings ,
                                      const CORE::CValueList* params           )
{GUCEF_TRACE;

    Shutdown();
    m_name = title;
    m_canvas = GUCEF_NULL != params ? params->GetValueAlways( "CANVAS", "#canvas" ).AsString() : GUI::CString( "#canvas" );
    if ( m_canvas.IsNULLOrEmpty() )
        m_canvas = "#canvas";

    if ( 0 == guidriverEmscripten_CanvasExists( m_canvas.C_String() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: The page has no canvas \"" + m_canvas + "\"" );
        return false;
    }

    EmscriptenWebGLContextAttributes attributes;
    ::emscripten_webgl_init_context_attributes( &attributes );
    attributes.majorVersion = 2;
    attributes.minorVersion = 0;
    attributes.alpha = EM_FALSE;
    attributes.depth = EM_TRUE;
    attributes.stencil = EM_TRUE;
    attributes.antialias = EM_FALSE;
    attributes.premultipliedAlpha = EM_TRUE;
    attributes.preserveDrawingBuffer = EM_FALSE;
    attributes.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = ::emscripten_webgl_create_context( m_canvas.C_String(), &attributes );
    if ( context <= 0 )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: The browser gives no WebGL 2 context for \"" + m_canvas + "\"" );
        return false;
    }
    m_webglContext = (CORE::Int64) context;
    if ( !MakeCurrent() )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: The WebGL context could not be made current" );
        Shutdown();
        return false;
    }

    if ( !Resize( videoSettings.GetResolutionWidthInPixels(), videoSettings.GetResolutionHeightInPixels() ) )
    {
        Shutdown();
        return false;
    }
    if ( !title.IsNULLOrEmpty() )
        ::emscripten_set_window_title( title.C_String() );

    if ( videoSettings.GetFrequency() > 0 )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestPeriodicPulses( this, 1000 / videoSettings.GetFrequency() );
        SubscribeTo( pulseGenerator.GetPointerAlways(), CORE::CPulseGenerator::PulseEvent );
        m_drivesRedraws = true;
    }
    if ( videoSettings.GetFullscreenState() )
        SetFullscreen( true );

    GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: Made a WebGL 2 window of " + CORE::UInt32ToString( m_width ) + " x " +
                      CORE::UInt32ToString( m_height ) + " in canvas \"" + m_canvas + "\"" );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowContext::Resize( GUI::UInt32 width, GUI::UInt32 height )
{GUCEF_TRACE;

    if ( 0 == m_webglContext )
        return false;
    if ( 0 == width || 0 == height )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: A window needs a size" );
        return false;
    }
    if ( width == m_width && height == m_height )
        return true;

    if ( EMSCRIPTEN_RESULT_SUCCESS != ::emscripten_set_canvas_element_size( m_canvas.C_String(), (int) width, (int) height ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: The size of canvas \"" + m_canvas + "\" could not be set" );
        return false;
    }
    const bool resized = 0 != m_width;
    m_width = width;
    m_height = height;
    if ( resized )
        NotifyObservers( WindowContextSizeEvent );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowContext::SetFullscreen( bool fullscreen )
{GUCEF_TRACE;

    if ( 0 == m_webglContext )
        return false;
    if ( fullscreen == m_fullscreen )
        return true;

    EMSCRIPTEN_RESULT result = EMSCRIPTEN_RESULT_SUCCESS;
    if ( fullscreen )
    {
        EmscriptenFullscreenStrategy strategy;
        memset( &strategy, 0, sizeof( strategy ) );
        strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
        strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
        strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST;

        // outside the handler of an input event the browser refuses, then the request waits for the next one
        result = ::emscripten_request_fullscreen_strategy( m_canvas.C_String(), EM_TRUE, &strategy );
    }
    else
    {
        result = ::emscripten_exit_fullscreen();
    }
    if ( EMSCRIPTEN_RESULT_SUCCESS != result && EMSCRIPTEN_RESULT_DEFERRED != result )
    {
        GUCEF_WARNING_LOG( CORE::LOGLEVEL_NORMAL, "CEmscriptenWindowContext: The browser refused to " + CORE::CString( fullscreen ? "enter" : "leave" ) +
                           " full screen (" + CORE::Int32ToString( result ) + ")" );
        return false;
    }
    m_fullscreen = fullscreen;
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CEmscriptenWindowContext::MakeCurrent( void )
{GUCEF_TRACE;

    if ( 0 == m_webglContext )
        return false;
    return EMSCRIPTEN_RESULT_SUCCESS == ::emscripten_webgl_make_context_current( (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE) m_webglContext );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenWindowContext::RequestRedraw( void )
{GUCEF_TRACE;

    if ( !MakeCurrent() )
        return;

    ::glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    ::glViewport( 0, 0, (GLsizei) m_width, (GLsizei) m_height );
    ::glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    ::glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    NotifyObservers( WindowContextRedrawEvent );
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenWindowContext::Shutdown( void )
{GUCEF_TRACE;

    if ( m_drivesRedraws )
    {
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
        m_drivesRedraws = false;
    }
    if ( m_fullscreen )
    {
        ::emscripten_exit_fullscreen();
        m_fullscreen = false;
    }
    if ( 0 != m_webglContext )
    {
        if ( !m_canvas.IsNULLOrEmpty() )
            guidriverEmscripten_SetCanvasCursor( m_canvas.C_String(), 1 );
        ::emscripten_webgl_destroy_context( (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE) m_webglContext );
        m_webglContext = 0;
    }
    m_width = m_height = 0;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CEmscriptenWindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static const CORE::CString classTypeName = "GUCEF::GUIDRIVEREMSCRIPTEN::CEmscriptenWindowContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CEmscriptenWindowContext::OnNotify( CORE::CNotifier* notifier    ,
                                    const CORE::CEvent& eventId  ,
                                    CORE::CICloneable* eventData )
{GUCEF_TRACE;

    if ( CORE::CPulseGenerator::PulseEvent == eventId )
        RequestRedraw();
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace GUIDRIVEREMSCRIPTEN */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
