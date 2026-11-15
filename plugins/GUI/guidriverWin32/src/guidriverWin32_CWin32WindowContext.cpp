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

#ifndef GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#include "gucefGUI_CWindowManagerBackend.h"
#define GUCEF_GUI_CWINDOWMANAGERBACKEND_H
#endif /* GUCEF_GUI_CWINDOWMANAGERBACKEND_H ? */

#ifndef GUCEF_CORE_DVCPPSTRINGUTILS_H
#include "dvcppstringutils.h"
#define GUCEF_CORE_DVCPPSTRINGUTILS_H
#endif /* GUCEF_CORE_DVCPPSTRINGUTILS_H ? */

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
//      GLOBAL VARS                                                        //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace {

GUI::UInt32 g_nextWindowId = 1;
bool g_dpiAwarenessDone = false;

/** DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2, which older SDK headers do not define */
const HANDLE PerMonitorAwareV2 = (HANDLE) (LONG_PTR) -4;

typedef BOOL ( WINAPI *TSetProcessDpiAwarenessContext )( HANDLE value );
typedef UINT ( WINAPI *TGetDpiForWindow )( HWND window );

/** The process is made per monitor DPI aware once, before its first window; the functions are looked up because older Windows lacks them */
void
MakeProcessDpiAware( void )
{
    if ( g_dpiAwarenessDone )
        return;
    g_dpiAwarenessDone = true;
    HMODULE user32 = ::GetModuleHandleA( "user32.dll" );
    if ( GUCEF_NULL == user32 )
        return;
    TSetProcessDpiAwarenessContext setAwareness = (TSetProcessDpiAwarenessContext) ::GetProcAddress( user32, "SetProcessDpiAwarenessContext" );
    if ( GUCEF_NULL == setAwareness )
    {
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Windows offers no per monitor DPI awareness (before Windows 10 1703): windows are scaled by Windows" );
        return;
    }
    // fails when the manifest of the application or an earlier call already decided: that choice stays
    if ( FALSE == setAwareness( PerMonitorAwareV2 ) )
        GUCEF_SYSTEM_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: The DPI awareness of the process was already set, error " << (UInt32) ::GetLastError() );
}

}

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
      m_window()               ,
      m_pulseFrequency( 0 )
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

    HWND window = m_window.GetHwnd();
    return GUCEF_NULL != window && ::GetForegroundWindow() == window;
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

    SetPulseFrequency( 0 );
    if ( GUCEF_NULL != m_window.GetHwnd() )
    {
        m_window.WindowDestroy();
        // fails while other windows of the class exist, which is as it should be
        CORE::CMsWin32Window::UnregisterWindowClass( GetClassTypeName() );
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
    if ( propertyName == "GRAPHICS_API" )
    {
        return GUI::CWindowManagerBackend::GraphicsApiNone;
    }
    if ( propertyName == "DPI" )
    {
        return CORE::UInt32ToString( GetDpi() );
    }
    if ( propertyName == "SCALE" )
    {
        return CORE::FloatToString( (Float32) GetDpi() / 96.0f );
    }
    return GUI::CString(); 
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowContext::SetSystemCursorShape( GUI::TCursorShape shape )
{GUCEF_TRACE;

    HWND window = m_window.GetHwnd();
    if ( GUCEF_NULL == window )
        return false;

    LPCTSTR resource = IDC_ARROW;
    switch ( shape )
    {
        case GUI::CURSORSHAPE_ARROW: resource = IDC_ARROW; break;
        case GUI::CURSORSHAPE_TEXT: resource = IDC_IBEAM; break;
        case GUI::CURSORSHAPE_RESIZE_ALL: resource = IDC_SIZEALL; break;
        case GUI::CURSORSHAPE_RESIZE_NS: resource = IDC_SIZENS; break;
        case GUI::CURSORSHAPE_RESIZE_EW: resource = IDC_SIZEWE; break;
        case GUI::CURSORSHAPE_RESIZE_NESW: resource = IDC_SIZENESW; break;
        case GUI::CURSORSHAPE_RESIZE_NWSE: resource = IDC_SIZENWSE; break;
        case GUI::CURSORSHAPE_HAND: resource = IDC_HAND; break;
        case GUI::CURSORSHAPE_WAIT: resource = IDC_WAIT; break;
        case GUI::CURSORSHAPE_PROGRESS: resource = IDC_APPSTARTING; break;
        case GUI::CURSORSHAPE_NOT_ALLOWED: resource = IDC_NO; break;
        default: break;
    }

    // the cursor of the window class is what Windows puts back whenever the pointer moves, so it is the class that is changed
    HCURSOR cursor = ::LoadCursor( GUCEF_NULL, resource );
    if ( GUCEF_NULL == cursor )
        return false;
    ::SetClassLongPtr( window, GCLP_HCURSOR, (LONG_PTR) cursor );
    ::SetCursor( cursor );
    return true;
}

/*-------------------------------------------------------------------------*/

GUI::UInt32
CWin32WindowContext::GetDpi( void ) const
{GUCEF_TRACE;

    HWND window = m_window.GetHwnd();
    HMODULE user32 = ::GetModuleHandleA( "user32.dll" );
    if ( GUCEF_NULL == window || GUCEF_NULL == user32 )
        return 96;
    TGetDpiForWindow getDpiForWindow = (TGetDpiForWindow) ::GetProcAddress( user32, "GetDpiForWindow" );
    if ( GUCEF_NULL == getDpiForWindow )
        return 96;
    const UINT dpi = getDpiForWindow( window );
    return 0 != dpi ? (GUI::UInt32) dpi : 96;
}

/*-------------------------------------------------------------------------*/

void
CWin32WindowContext::SetPulseFrequency( GUI::UInt32 frequency )
{GUCEF_TRACE;

    if ( frequency == m_pulseFrequency )
        return;
    CORE::PulseGeneratorPtr pulseGenerator = CORE::CCoreGlobal::Instance()->GetPulseGenerator();
    if ( 0 != m_pulseFrequency )
    {
        pulseGenerator->RequestStopOfPeriodicUpdates( this );
        UnsubscribeFrom( pulseGenerator.GetPointerAlways() );
    }
    m_pulseFrequency = frequency;
    if ( 0 != frequency )
    {
        // the redraws are paced by the pulses of the application at the frequency of the video settings
        const GUI::UInt32 interval = frequency < 1000 ? 1000 / frequency : 1;
        pulseGenerator->RequestPeriodicPulses( this, interval );
        SubscribeTo( pulseGenerator.GetPointerAlways() );
    }
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowContext::ApplyVideoSettings( const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    if ( GUCEF_NULL == m_window.GetHwnd() )
        return false;
    m_window.SetFullscreenStatus( videoSettings.GetFullscreenState() );
    if ( !videoSettings.GetFullscreenState() && videoSettings.GetResolutionWidthInPixels() > 0 && videoSettings.GetResolutionHeightInPixels() > 0 )
    {
        m_window.ClientAreaResize( (int) videoSettings.GetResolutionWidthInPixels(), (int) videoSettings.GetResolutionHeightInPixels() );
    }
    SetPulseFrequency( videoSettings.GetFrequency() );
    return true;
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowContext::Initialize( const GUI::CString& title                ,
                                     const GUI::CVideoSettings& videoSettings )
{GUCEF_TRACE;

    return Initialize( title, videoSettings, GUCEF_NULL );
}

/*-------------------------------------------------------------------------*/

bool
CWin32WindowContext::Initialize( const GUI::CString& title                ,
                                 const GUI::CVideoSettings& videoSettings ,
                                 const CORE::CValueList* params           )
{GUCEF_TRACE;

    // Do not initialize twice
    Shutdown();

    m_name = title;
    m_id = g_nextWindowId++;
    bool dpiAware = true;
    if ( GUCEF_NULL != params && params->HasKey( "DPI_AWARE" ) )
        dpiAware = params->GetValueAlways( "DPI_AWARE" ).AsBool( true, true );
    if ( dpiAware )
        MakeProcessDpiAware();

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

        // the redraws are paced by the application's pulses at the frequency of the video settings
        SetPulseFrequency( videoSettings.GetFrequency() );
        if ( videoSettings.GetFullscreenState() )
            m_window.SetFullscreenStatus( true );

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
    if ( eventID == CORE::CMsWin32Window::WindowCloseEvent )
    {
        // CMsWin32Window destroys the window right after this: whoever drew in it lets go of its surface now
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Window close event received" );
        NotifyObservers( WindowContextCloseRequestEvent );
        Shutdown();
    }
    else
    if ( eventID == CORE::CMsWin32Window::WindowDestroyEvent )
    {
        GUCEF_DEBUG_LOG( CORE::LOGLEVEL_NORMAL, "Win32WindowContext: Window destroy event received" );
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
