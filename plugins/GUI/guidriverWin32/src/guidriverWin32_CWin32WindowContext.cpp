/*
 *  guidriverWin32: module implementing window management for Win32
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

#include "GUIDRIVERWIN32_CWin32WindowContext.h"

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace GUIDRIVERWIN32 {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILITIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CWin32WindowContext::CWin32WindowContext( void )
    : CWindowContext()         ,
      m_guiContext()           ,
      m_id()                   ,
      m_name()                 ,
      m_window()               
{GUCEF_TRACE;

    SubscribeTo( &m_window );
}

/*-------------------------------------------------------------------------*/

CWin32WindowContext::~CWin32WindowContext()
{GUCEF_TRACE;

    Shutdown();
}

/*-------------------------------------------------------------------------*/

void
CWin32WindowContext::SetGuiContext( GUI::TGuiContextPtr& context )
{GUCEF_TRACE;

    m_guiContext = context;
}

/*-------------------------------------------------------------------------*/

GUI::TGuiContextPtr
CWin32WindowContext::GetGuiContext( void )
{GUCEF_TRACE;

    return m_guiContext;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32WindowContext::GetID( void ) const
{GUCEF_TRACE;

    return m_id;
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowContext::IsActive( void ) const
{GUCEF_TRACE;

    return false;
}

/*-------------------------------------------------------------------------*/

GUI::CString
CWin32WindowContext::GetName( void ) const
{GUCEF_TRACE;

    return m_name;
}

/*-------------------------------------------------------------------------*/

void
CWin32WindowContext::Shutdown( void )
{GUCEF_TRACE;

	CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    pulseGenerator->RequestStopOfPeriodicUpdates( this );
    UnsubscribeFrom( pulseGenerator.GetPointerAlways() );

    if ( GUCEF_NULL != m_window.GetHwnd() )
    {
        CORE::CMsWin32Window::UnregisterWindowClass( m_window.GetText() );
        m_window.WindowDestroy();
    }
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32WindowContext::GetWidth( void ) const
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
CWin32WindowContext::GetHeight( void ) const
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
CWin32WindowContext::GetProperty( const GUI::CString& propertyName ) const
{GUCEF_TRACE;

    if ( propertyName == "WINDOW" || propertyName == "HWND" )
    {
        return CORE::PointerToString( m_window.GetHwnd() );
    }
    return GUI::CString(); 
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowContext::Initialize( const GUI::CString& title                ,
                                     const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    // Do not initialize twice
    Shutdown();

    if ( !CORE::CMsWin32Window::RegisterWindowClass( GetClassTypeName() ) )
    {
        GUCEF_ERROR_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Could not register window class" );
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

        // Grab the main app pulse generator and set the update interval for the context to the desired refresh rate
        CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator(); 
        pulseGenerator->RequestPeriodicPulses( this, 1000 / videoSettings.GetFrequency() );
        SubscribeTo( pulseGenerator.GetPointerAlways() );

        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Succesfully created Win32 window" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------------*/

const CORE::CString&
CWin32WindowContext::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CORE::CString classTypeName = "GUCEF::GUIDRIVERWIN32::CWin32WindowContext";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CWin32WindowContext::OnNotify( CORE::CNotifier* notifier   ,
                                   const CORE::CEvent& eventID ,
                                   CORE::CICloneable* evenData )
{GUCEF_TRACE;

    if ( ( eventID == CORE::CPulseGenerator::PulseEvent )      ||
         ( eventID == CORE::CMsWin32Window::WindowPaintEvent ) )
    {
        // Notify that we are going to redraw the window
        NotifyObservers( WindowContextRedrawEvent );        
    }
    else
    if ( eventID == CORE::CMsWin32Window::WindowResizeEvent )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Window resize event received" );
        NotifyObservers( WindowContextSizeEvent );
    }
    else
    if ( eventID == CORE::CMsWin32Window::WindowActivationEvent )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Window activation event received" );
        NotifyObservers( WindowContextActivateEvent );
    }
    else
    if ( ( eventID == CORE::CMsWin32Window::WindowCloseEvent )    ||
         ( eventID == CORE::CMsWin32Window::WindowDestroyEvent )  )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Window close/destroy event received" );
        Shutdown();
    }
}


/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

} /* namespace GUIDRIVERWIN32 */
} /* namespace GUCEF */

/*-------------------------------------------------------------------------*/
