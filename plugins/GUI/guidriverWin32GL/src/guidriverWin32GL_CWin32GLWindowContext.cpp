/*
 *  guidriverWin32GL: module implementing GL based window management for Win32
 *  Copyright (C) 2002 - 2011.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*-------------------------------------------------------------------------//
//                                                                         //
//      INCLUDES                                                           //
//                                                                         //
//-------------------------------------------------------------------------*/

#include <windows.h>
#include <GL/GL.h>

#ifndef GUCEF_CORE_LOGGING_H
#include "gucefCORE_Logging.h"
#define GUCEF_CORE_LOGGING_H
#endif /* GUCEF_CORE_LOGGING_H ? */

#ifndef GUCEF_CORE_CGUCEFAPPLICATION_H
#include "CGUCEFApplication.h"
#define GUCEF_CORE_CGUCEFAPPLICATION_H
#endif /* GUCEF_CORE_CGUCEFAPPLICATION_H ? */

#ifndef GUCEF_CORE_CCOREGLOBAL_H
#include "gucefCORE_CCoreGlobal.h"
#define GUCEF_CORE_CCOREGLOBAL_H
#endif /* GUCEF_CORE_CCOREGLOBAL_H ? */

#ifndef GUCEF_CORE_CPULSEGENERATOR_H
#include "gucefCORE_CPulseGenerator.h"
#define GUCEF_CORE_CPULSEGENERATOR_H
#endif /* GUCEF_CORE_CPULSEGENERATOR_H ? */

#ifndef GUCEF_GUI_CWINDOWCONTEXT_H
#include "gucefGUI_CWindowContext.h"
#define GUCEF_GUI_CWINDOWCONTEXT_H
#endif /* GUCEF_GUI_CWINDOWCONTEXT_H ? */

#include "guidriverWin32GL_CWin32GLWindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32GL {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWin32GLWindowContext::CWin32GLWindowContext( void )
    : CWindowContext()         ,
      m_guiContext()           ,
      m_id()                   ,
      m_name()                 ,
      m_window()               ,
      m_renderContext( NULL )  ,
      m_deviceContext( NULL )
{GUCEF_TRACE;

    SubscribeTo( &m_window );
}

/*-------------------------------------------------------------------------*/

CWin32GLWindowContext::~CWin32GLWindowContext()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CWin32GLWindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CWin32GLWindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32GLWindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

bool
CWin32GLWindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWin32GLWindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CWin32GLWindowContext::Shutdown( void )
{GUCEF_TRACE;

	CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->RequestStopOfPeriodicUpdates( this );
    UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
    
    if ( NULL != m_renderContext )
	{
		wglMakeCurrent( NULL, NULL ); 
		wglDeleteContext( m_renderContext );
		m_renderContext = NULL;
	}

	if ( NULL != m_deviceContext )
	{
		ReleaseDC( m_window.GetHwnd(), m_deviceContext );
		m_deviceContext = NULL;
	}

    if ( NULL != m_window.GetHwnd() )
    {
        CORE::CMsWin32Window::UnregisterWindowClass( m_window.GetText() );
        m_window.WindowDestroy();
    }
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32GLWindowContext::GetWidth( void ) const
{GUCEF_TRACE;

    int dummy;
    int width;
    if ( m_window.GetClientArea( dummy, dummy, width, dummy ) )
    {
        return (GUI::UInt32) width;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32GLWindowContext::GetHeight( void ) const
{GUCEF_TRACE;
    
    int dummy;
    int height;
    if ( m_window.GetClientArea( dummy, dummy, dummy, height ) )
    {
        return (GUI::UInt32) height;
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWin32GLWindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( propertyName == "WINDOW" )
    {
        return CORE::PointerToString( m_window.GetHwnd() );
    }
    return GUI::CString(); 
}

/*-------------------------------------------------------------------------*/

bool
CWin32GLWindowContext::Initialize( const GUI::CString& title                ,
                                   const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    // Do not initialize twice
    Shutdown();

    if ( !CORE::CMsWin32Window::RegisterWindowClass( GetClassTypeName() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Could not register window class" );
        return false;
    }

    // First create a regular Win32 window
    if ( m_window.WindowCreate( GetClassTypeName()                          ,
                                title                                       ,
                                0                                           ,
                                0                                           ,
                                videoSettings.GetResolutionWidthInPixels()  ,
                                videoSettings.GetResolutionHeightInPixels() ) )
    {
        // Display the new window
        m_window.Show();
        m_window.SendToForegound();
        m_window.GrabFocus();

	    // Now proceed with setting up the OpenGL specifics
        m_deviceContext = GetDC( m_window.GetHwnd() );
	    if ( NULL == m_deviceContext )
	    {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Could not obtain window device context" );
            Shutdown();
		    return false;
	    }

	    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	    memset( &pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor) );
	    pixelFormatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	    pixelFormatDescriptor.nVersion = 1;
	    pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	    pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	    pixelFormatDescriptor.cColorBits = 32;
	    pixelFormatDescriptor.cRedBits = 8;
	    pixelFormatDescriptor.cGreenBits = 8;
	    pixelFormatDescriptor.cBlueBits = 8;
	    pixelFormatDescriptor.cAlphaBits = 8;
	    pixelFormatDescriptor.cDepthBits = 24;
	    pixelFormatDescriptor.cStencilBits = 8;

	    int pixelFormat = ChoosePixelFormat( m_deviceContext, &pixelFormatDescriptor );
	    if ( 0 == pixelFormat )
	    {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Failed to get the desired pixel format" );
		    Shutdown();
            return false;
	    }

	    if ( FALSE == SetPixelFormat( m_deviceContext, pixelFormat, &pixelFormatDescriptor ) )
	    {
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Could not set pixel format" );
		    Shutdown();
            return false;
	    }

	    m_renderContext = wglCreateContext( m_deviceContext );
	    if ( NULL == m_renderContext )
	    { 
		    GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Could not create OpenGL rendering context" );
		    Shutdown();
            return false;
	    }

	    if ( !MakeCurrent() )
	    {
		    Shutdown();
            return false;
	    }

        // Set up the GL state.
        glClearColor(0, 0, 0, 1);
        //glEnableClientState(GL_VERTEX_ARRAY);
        //glEnableClientState(GL_COLOR_ARRAY);
        glViewport( 0, 0, videoSettings.GetResolutionWidthInPixels(), videoSettings.GetResolutionHeightInPixels() );

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, GetWidth(), GetHeight(), 0, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        ApplySwapInterval( videoSettings.GetVSyncState() );
        if ( videoSettings.GetFullscreenState() )
            m_window.SetFullscreenStatus( true );

        // Grab the main app pulse generator and set the update interval for the context to the desired refresh rate
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator(); 
        pulseGenerator->RequestPeriodicPulses( this, 1000 / videoSettings.GetFrequency() );
        SubscribeTo( pulseGenerator.GetPointerAlways() );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Succesfully created OpenGL rendering context" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

bool
CWin32GLWindowContext::MakeCurrent( void )
{
	if ( wglMakeCurrent( m_deviceContext, m_renderContext ) == FALSE )
	{
		GUCEF_ERROR_LOG( CORE::LOGLEVEL_IMPORTANT, "Win32GLWindowContext: Could not make OpenGL rendering context current" );
        return false;
	}
    GUCEF_DEBUG_LOG( CORE::LOGLEVEL_EVERYTHING, "Win32GLWindowContext: Made context " + CORE::PointerToString( m_renderContext ) + " active for device context " + CORE::PointerToString( m_deviceContext ) );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWin32GLWindowContext::ApplySwapInterval( bool vsync )
{GUCEF_TRACE;

    typedef BOOL ( WINAPI *TSwapIntervalEXT )( int );
    TSwapIntervalEXT swapIntervalEXT = (TSwapIntervalEXT) wglGetProcAddress( "wglSwapIntervalEXT" );
    if ( NULL == swapIntervalEXT )
        return false;
    return FALSE != swapIntervalEXT( vsync ? 1 : 0 );
}

/*-------------------------------------------------------------------------*/

bool
CWin32GLWindowContext::ApplyVideoSettings( const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    if ( NULL == m_window.GetHwnd() || NULL == m_renderContext )
        return false;
    if ( !MakeCurrent() )
        return false;
    ApplySwapInterval( videoSettings.GetVSyncState() );
    m_window.SetFullscreenStatus( videoSettings.GetFullscreenState() );
    if ( !videoSettings.GetFullscreenState() && videoSettings.GetResolutionWidthInPixels() > 0 && videoSettings.GetResolutionHeightInPixels() > 0 )
        m_window.ClientAreaResize( (int) videoSettings.GetResolutionWidthInPixels(), (int) videoSettings.GetResolutionHeightInPixels() );
    return true;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWin32GLWindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::GUIDRIVERWIN32GL::CWin32GLWindowContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWin32GLWindowContext::OnNotify( CORE::CNotifier* notifier   ,
                                 const CORE::CEvent& eventID ,
                                 CORE::CICloneable* evenData )
{GUCEF_TRACE;

    if ( ( eventID == CORE::CPulseGenerator::PulseEvent )      ||
         ( eventID == CORE::CMsWin32Window::WindowPaintEvent ) )
    {
        // Make sure this context is the current context
        if ( MakeCurrent() )
        {
            // Clear the screen in preparation for the redraw
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glViewport( 0, 0, GetWidth(), GetHeight() );

            // Notify that we are going to redraw the window
            NotifyObservers( WindowContextRedrawEvent );        
            
            // Swap our front and back buffers
            ::SwapBuffers( m_deviceContext );
        }
    }
    else
    if ( eventID == CORE::CMsWin32Window::WindowResizeEvent )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Window resize event received" );
        MakeCurrent();
        glViewport( 0, 0, GetWidth(), GetHeight() );
        NotifyObservers( WindowContextSizeEvent );
    }
    else
    if ( eventID == CORE::CMsWin32Window::WindowActivationEvent )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Window activation event received" );
        MakeCurrent();
        NotifyObservers( WindowContextActivateEvent );
    }
    else
    if ( ( eventID == CORE::CMsWin32Window::WindowCloseEvent )    ||
         ( eventID == CORE::CMsWin32Window::WindowDestroyEvent )  )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32GLWindowContext: Window close/destroy event received" );
        Shutdown();
    }
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32GL */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
