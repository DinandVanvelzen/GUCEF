/*
 *  gucefCORE: GUCEF module providing O/S abstraction and generic solutions
 *  Copyright (C) 2002 - 2007.  Dinand Vanvelzen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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

#include "gucefCORE_CMsWin32Window.h"

#ifdef GUCEF_MSWIN_BUILD

#ifndef GUCEF_CORE_MSWINUTILS_H
#include "gucefCORE_mswinutils.h"
#define GUCEF_CORE_MSWINUTILS_H
#endif /* GUCEF_CORE_MSWINUTILS_H ? */

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

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

namespace GUCEF {
namespace CORE {

/*-------------------------------------------------------------------------//
//                                                                         //
//      UTILTIIES                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

CMsWin32Window::CMsWin32Window( void )
    : COSWindow(),
      m_hwnd( 0 ) ,
      m_orgWinProc( 0 ) ,
      m_isFullscreen( false ) ,
      m_windowedStyle( 0 ) ,
      m_windowedRect()
{GUCEF_TRACE;

    memset( &m_windowedRect, 0, sizeof( m_windowedRect ) );

    SubscribeTo( CCoreGlobal::Instance()->GetPulseGenerator().GetPointerAlways(), CPulseGenerator::PulseEvent );
}

/*-------------------------------------------------------------------------*/

CMsWin32Window::~CMsWin32Window()
{GUCEF_TRACE;

    SetHwnd( 0 );
}

/*-------------------------------------------------------------------------*/

CString
CMsWin32Window::GetProperty( const CString& propertyName ) const
{GUCEF_TRACE;

    if ( propertyName == "WINDOW" || propertyName == "HWND" ) 
    {
        return PointerToString( GetHwnd() );
    }
    if ( propertyName == "WINDOWINT" || propertyName == "HWNDINT" ) 
    {
        Int64 hwndAsInt = (Int64) GetHwnd();
        return Int64ToString( hwndAsInt );
    }
    return CString(); 
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::WindowDestroy( void )
{
    SetHwnd( 0 );
}

/*-------------------------------------------------------------------------*/

LRESULT
CMsWin32Window::WindowProc( const HWND hWnd     ,
                            const UINT nMsg     ,
                            const WPARAM wParam ,
                            const LPARAM lParam )
{GUCEF_TRACE;

    switch( nMsg )
    {
        //case WM_COMMAND:
        //{
        //    CMsWin32Window* child = (CMsWin32Window*) LOWORD( wParam );
        //    
        //    // Check if this is a text changed notification
        //    if ( HIWORD( wParam ) == EN_CHANGE )
        //    {
        //        //NotifyObservers( TextChangedEvent );
        //        break;
        //    }
        //}
        case WM_CLOSE:
        {
            NotifyObservers( WindowCloseEvent );
            
            DestroyWindow( m_hwnd );
            m_hwnd = 0;
            break;
        }
        case WM_DESTROY:
        {
            NotifyObservers( WindowDestroyEvent );
            break;
        }
        case WM_SIZE:
        {
            NotifyObservers( WindowResizeEvent );
            break;
        }
        case WM_ACTIVATE :
        {
            if ( ( WA_ACTIVE == LOWORD( wParam ) )      ||
                 ( WA_CLICKACTIVE == LOWORD( wParam ) ) )
            {
                NotifyObservers( WindowActivationEvent );
            }
            break;
        }
        case WM_PAINT:
        {
            NotifyObservers( WindowPaintEvent );
            return GetOriginalWinProc()( hWnd   ,
                                         nMsg   , 
                                         wParam ,
                                         lParam );
        }
        default:
        {
            return GetOriginalWinProc()( hWnd   ,
                                         nMsg   , 
                                         wParam ,
                                         lParam );
        }
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::SendToForegound( void )
{
    ::SetForegroundWindow( GetHwnd() );
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::GrabFocus( void )
{
    ::SetFocus( GetHwnd() );
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::SetFullscreenStatus( bool fullscreen )
{GUCEF_TRACE;

    if ( 0 == m_hwnd || fullscreen == m_isFullscreen )
        return;

    if ( fullscreen )
    {
        MONITORINFO monitorInfo;
        memset( &monitorInfo, 0, sizeof( monitorInfo ) );
        monitorInfo.cbSize = sizeof( monitorInfo );
        if ( FALSE == ::GetMonitorInfo( ::MonitorFromWindow( m_hwnd, MONITOR_DEFAULTTONEAREST ), &monitorInfo ) )
            return;

        m_windowedStyle = ::GetWindowLongPtr( m_hwnd, GWL_STYLE );
        ::GetWindowRect( m_hwnd, &m_windowedRect );
        ::SetWindowLongPtr( m_hwnd, GWL_STYLE, ( m_windowedStyle & ~(LONG_PTR) WS_OVERLAPPEDWINDOW ) | WS_POPUP );
        const RECT& area = monitorInfo.rcMonitor;
        ::SetWindowPos( m_hwnd, HWND_TOP, area.left, area.top, area.right - area.left, area.bottom - area.top,
                        SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
    }
    else
    {
        ::SetWindowLongPtr( m_hwnd, GWL_STYLE, m_windowedStyle );
        ::SetWindowPos( m_hwnd, NULL, m_windowedRect.left, m_windowedRect.top,
                        m_windowedRect.right - m_windowedRect.left, m_windowedRect.bottom - m_windowedRect.top,
                        SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW );
    }
    m_isFullscreen = fullscreen;
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::GetFullscreenStatus( void ) const
{GUCEF_TRACE;

    return m_isFullscreen;
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::Show( void )
{
    return ::ShowWindow( GetHwnd(), SW_SHOW ) == TRUE;
}
    
/*-------------------------------------------------------------------------*/
    
bool
CMsWin32Window::Hide( void )
{
    return ::ShowWindow( GetHwnd(), SW_HIDE ) == TRUE;
}
    
/*-------------------------------------------------------------------------*/
    
bool
CMsWin32Window::Maximize( void )
{
    return ::ShowWindow( GetHwnd(), SW_SHOWMAXIMIZED ) == TRUE;
}
    
/*-------------------------------------------------------------------------*/
    
bool
CMsWin32Window::Minimize( void )
{
    return ::ShowWindow( GetHwnd(), SW_MINIMIZE ) == TRUE;
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::SetHwnd( HWND hwnd )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        ::DestroyWindow( m_hwnd );
    }
    m_hwnd = hwnd;
}

/*-------------------------------------------------------------------------*/

HWND
CMsWin32Window::GetHwnd( void ) const
{GUCEF_TRACE;

    return m_hwnd;
}

/*-------------------------------------------------------------------------*/

HWND
CMsWin32Window::GetParentHwnd( void ) const
{GUCEF_TRACE;
    
    if ( 0 != m_hwnd )
    {
        ::GetParent( m_hwnd );
    }
    return 0;
}

/*-------------------------------------------------------------------------*/

LRESULT
CMsWin32Window::WndProc( HWND hwnd     , 
                         UINT msg      ,
                         WPARAM wParam ,
                         LPARAM lParam )
{GUCEF_TRACE;

    CMsWin32Window* windowObj = 0 ;

    if ( msg == WM_NCCREATE )
    {
        windowObj = (CMsWin32Window*) ((LPCREATESTRUCT) lParam)->lpCreateParams;
        #ifdef GUCEF_32BIT
        ::SetWindowLong( hwnd, GWL_USERDATA, (LONG_PTR) windowObj );
	    #else
        ::SetWindowLongPtr( hwnd, GWLP_USERDATA, (LONG_PTR) windowObj );
        #endif

	    //If you process any messages that are sent before CreateWindowEx returns
	    //the HWND, you need something in the place of your HWND member.
	    windowObj->SetHwnd( hwnd );
    }
    else
    {
        #ifdef GUCEF_32BIT
        windowObj = (CMsWin32Window*) ::GetWindowLong( hwnd, GWL_USERDATA );
        #else
        windowObj = (CMsWin32Window*) ::GetWindowLongPtr( hwnd, GWLP_USERDATA );
        #endif
    }
    
    if ( 0 != windowObj )
    {
        return windowObj->WindowProc( hwnd, msg, wParam, lParam );
    }
    return ::DefWindowProc( hwnd, msg, wParam, lParam );
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::Close( void )
{GUCEF_TRACE;
    
    if ( 0 != m_hwnd )
    {
        ::CloseWindow( m_hwnd );
    }
}

/*-------------------------------------------------------------------------*/

int
CMsWin32Window::GetWidthOfClientAreaBorder( void ) const
{GUCEF_TRACE;
    
    if ( 0 != m_hwnd )
    {
        RECT rcClient; RECT rcWind;
        ::GetClientRect( m_hwnd, &rcClient); 
        ::GetWindowRect( m_hwnd, &rcWind); 
        return ( ( rcWind.right - rcWind.left ) - rcClient.right ) / 2; 
    }
    else
    {
        return ::GetSystemMetrics( SM_CXSIZEFRAME );
    }
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::GetWindowAreaPosition( int& x, int& y ) const
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcWindow;
        ::GetWindowRect( m_hwnd, &rcWindow );
        x = rcWindow.left;
        y = rcWindow.top;
        return true;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::GetClientAreaPosition( int& x, int& y ) const
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcClient;
        ::GetClientRect( m_hwnd, &rcClient );
        x = rcClient.left;
        y = rcClient.top;
        return true;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::GetClientArea( int& x, int& y, int& width, int& height ) const
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcClient;
        ::GetClientRect( m_hwnd, &rcClient );
        x = rcClient.left;
        y = rcClient.top;
        width = (rcClient.right - rcClient.left);
        height = (rcClient.bottom - rcClient.top);
        return true;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::GetWindowArea( int& x, int& y, int& width, int& height ) const
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcWindow;
        ::GetWindowRect( m_hwnd, &rcWindow );
        x = rcWindow.left;
        y = rcWindow.top;
        width = (rcWindow.right - rcWindow.left);
        height = (rcWindow.bottom - rcWindow.top);
        return true;
    }
    return false;    
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::GetWindowAreaRelativeToParentClientArea( Float32& x      , 
                                                         Float32& y      ,
                                                         Float32& width  , 
                                                         Float32& height ) const
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        HWND parentHwnd = ::GetParent( m_hwnd );
        if ( 0 != parentHwnd )
        {        
            RECT rcWindow;
            ::GetWindowRect( m_hwnd, &rcWindow );            
            RECT rcParentClient;
            ::GetClientRect( parentHwnd, &rcParentClient );

            Float32 thisWidth = (Float32) rcWindow.right - rcWindow.left;
            Float32 thisHeight = (Float32) rcWindow.bottom - rcWindow.top;            
            Float32 parentWidth = (Float32) rcParentClient.right - rcParentClient.left;
            Float32 parentHeight = (Float32) rcParentClient.bottom - rcParentClient.top;
            
            x = rcWindow.left / parentWidth;
            y = rcWindow.top / parentHeight;
            width = thisWidth / parentWidth;
            height = thisHeight / parentHeight;
            return true;
        }
    }
    return false; 
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::WindowAreaResize( int x, int y, int width, int height, bool repaint )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        ::MoveWindow( m_hwnd, x, y, width, height, repaint ? TRUE : FALSE );
    }
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::WindowAreaResize( int width, int height, bool repaint )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcWindow;
        ::GetWindowRect( m_hwnd, &rcWindow );

        ::MoveWindow( m_hwnd, rcWindow.left, rcWindow.top, width, height, repaint ? TRUE : FALSE );
    }
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::ClientAreaResize( int x, int y, int width, int height, bool repaint )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcClient, rcWindow;
        POINT sizeDiff;
        
        ::GetClientRect( m_hwnd, &rcClient );
        ::GetWindowRect( m_hwnd, &rcWindow );
        
        sizeDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
        sizeDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
        
        ::MoveWindow( m_hwnd                 , 
                      x + rcClient.left      , 
                      y + rcClient.top       , 
                      width + sizeDiff.x     ,
                      height + sizeDiff.y    ,
                      repaint ? TRUE : FALSE );
    }
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::ClientAreaResize( int width, int height, bool repaint )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        RECT rcClient;
        ::GetClientRect( m_hwnd, &rcClient );
        
        ClientAreaResize( rcClient.left, rcClient.top, width, height, repaint );    
    }
}

/*-------------------------------------------------------------------------*/

inline Float32
ClampFloat( Float32 valueToClamp )
{
    if ( valueToClamp < 0.0 )
    {
        return 0.0;
    }
    else
    if ( valueToClamp > 1.0 )
    {
        return 1.0;
    }
    return valueToClamp;    
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::ClientAreaResizeRelativeToParentClientArea( Float32 x      , 
                                                            Float32 y      ,
                                                            Float32 width  , 
                                                            Float32 height ,
                                                            bool repaint   )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        HWND parentWindow = ::GetParent( m_hwnd );
        if ( 0 != parentWindow )
        {
            x = ClampFloat( x );
            y = ClampFloat( y );
            width = ClampFloat( width );
            height = ClampFloat( height );
            
            RECT parentClientAreaSize;
            ::GetClientRect( parentWindow, &parentClientAreaSize );
            
            int parentWidth = parentClientAreaSize.right - parentClientAreaSize.left;
            int parentHeight = parentClientAreaSize.bottom - parentClientAreaSize.top;
            
            ClientAreaResize( (int) ( x * parentWidth )       ,
                              (int) ( y * parentHeight )      , 
                              (int) ( width * parentWidth )   ,
                              (int) ( height * parentHeight ) ,
                              repaint                         );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::WindowAreaResizeRelativeToParentClientArea( Float32 x      , 
                                                            Float32 y      ,
                                                            Float32 width  , 
                                                            Float32 height ,
                                                            bool repaint   )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        HWND parentWindow = ::GetParent( m_hwnd );
        if ( 0 != parentWindow )
        {
            x = ClampFloat( x );
            y = ClampFloat( y );
            width = ClampFloat( width );
            height = ClampFloat( height );
            
            RECT parentClientAreaSize;
            ::GetClientRect( parentWindow, &parentClientAreaSize );
            
            int parentWidth = parentClientAreaSize.right - parentClientAreaSize.left;
            int parentHeight = parentClientAreaSize.bottom - parentClientAreaSize.top;
            
            WindowAreaResize( (int) ( x * parentWidth )       ,
                              (int) ( y * parentHeight )      , 
                              (int) ( width * parentWidth )   ,
                              (int) ( height * parentHeight ) ,
                              repaint                         );
        }
    }
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::Repaint( void )
{GUCEF_TRACE;

    ::InvalidateRect( m_hwnd, NULL, TRUE );
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::SetText( const CString& text )
{GUCEF_TRACE;

    SetWindowText( m_hwnd, text.C_String() );
}

/*-------------------------------------------------------------------------*/
    
CString
CMsWin32Window::GetText( void ) const
{GUCEF_TRACE;

    int len = GetWindowTextLength( m_hwnd );
    if( len > 0 )
    {
        char* buf = (char*) GlobalAlloc( GPTR, len + 1 );
        GetWindowText( m_hwnd, buf, len + 1);

        CString returnValue( buf, len );

        GlobalFree( (HANDLE)buf );
        
        return returnValue;
    }
    return CString();
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::SetOriginalWinProc( WNDPROC originalWinProc )
{GUCEF_TRACE;

    m_orgWinProc = originalWinProc;
}

/*-------------------------------------------------------------------------*/

WNDPROC
CMsWin32Window::GetOriginalWinProc( void )
{GUCEF_TRACE;

    if ( 0 != m_orgWinProc )
    {
        return m_orgWinProc;
    }
    return DefWindowProc;
}

/*-------------------------------------------------------------------------*/

bool 
CMsWin32Window::WindowCreate( const CString& windowTitle     ,
                              const Int32 xPosition          ,
                              const Int32 yPosition          ,
                              const UInt32 width             ,
                              const UInt32 height            )
{GUCEF_TRACE;

    return WindowCreate( GetClassTypeName() ,
                         windowTitle        ,
                         xPosition          ,
                         yPosition          ,
                         width              ,
                         height             ,
                         0                  );
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::WindowCreate( const CString& windowClassName ,
                              const CString& windowTitle     ,
                              const Int32 xPosition          ,
                              const Int32 yPosition          ,
                              const UInt32 width             ,
                              const UInt32 height            ,
                              const HWND hWndParent          )
{GUCEF_TRACE;

    if ( 0 != m_hwnd )
    {
        DestroyWindow( m_hwnd );
        m_hwnd = 0;
    }

    m_hwnd = CreateWindowEx( WS_EX_APPWINDOW,
                             windowClassName.C_String(),
                             windowTitle.C_String(),
                             WS_OVERLAPPEDWINDOW,
                             xPosition, yPosition, (int)width, (int)height,
                             hWndParent, NULL, GetCurrentModuleHandle(), (LPVOID)this );
    
    if ( 0 == m_hwnd )
    {
        GUCEF_SYSTEM_LOG( LOGLEVEL_IMPORTANT, "CMsWin32Window::WindowCreate(): Failed to create a window" );
        return false;
    }

    // Resize the client area to match the requested size by default. This is desired because in situations where 
    // a specific resolution is most impportant (rendering) it tends to always relate to the client area not the window area
    ClientAreaResize( width, height );

    HookWndProc();
    
    Show();
    Repaint();                                 
    return true;
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::HookWndProc( void )
{GUCEF_TRACE;

    WNDPROC orgWinProc = (WNDPROC) ::GetWindowLongPtr( m_hwnd, GWLP_WNDPROC );
    if ( orgWinProc != (WNDPROC) WndProc )
    {
        ::SetWindowLongPtr( m_hwnd, GWLP_WNDPROC, (LONG_PTR) WndProc );
        m_orgWinProc = orgWinProc;
    }
    ::SetWindowLongPtr( m_hwnd, GWLP_USERDATA, (LONG_PTR) this );
}

/*-------------------------------------------------------------------------*/

bool
CMsWin32Window::RegisterWindowClass( const CString& windowClassName )
{GUCEF_TRACE;

    WNDCLASSEX wc;
    if ( FALSE == ::GetClassInfoEx( GetCurrentModuleHandle()   ,
                                    windowClassName.C_String() ,
                                    &wc                        ) )
    {
        wc.cbSize        = sizeof(WNDCLASSEX);
        wc.style         = 0;
        wc.lpfnWndProc   = (WNDPROC)WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = GetCurrentModuleHandle();
        wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wc.lpszMenuName  = NULL;
        wc.lpszClassName = windowClassName.C_String();
        wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

        if ( 0 == RegisterClassEx( &wc ) )
        {
            GUCEF_SYSTEM_LOG( LOGLEVEL_IMPORTANT, "CMsWin32Window::RegisterWindowClass(): Failed to register a window class with name \"" + windowClassName + "\"" );
            return false;
        }        
    }
    else
    {
        GUCEF_DEBUG_LOG( LOGLEVEL_NORMAL, "CMsWin32Window::RegisterWindowClass(): A window class is already registered with name \"" + windowClassName + "\", skipping registration" );
    }

    return true;
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::UnregisterWindowClass( const CString& windowClassName )
{
    UnregisterClassA( windowClassName.C_String(), GetCurrentModuleHandle() );
}

/*-------------------------------------------------------------------------*/

const CString&
CMsWin32Window::GetClassTypeName( void ) const
{GUCEF_TRACE;

    static CString classTypeName = "GUCEF::CORE::CMsWin32Window";
    return classTypeName;
}

/*-------------------------------------------------------------------------*/

void
CMsWin32Window::OnNotify( CNotifier* notifier    ,
                          const CEvent& eventid  ,
                          CICloneable* eventdata )
{
    if ( eventid == CPulseGenerator::PulseEvent )
    {
        ::UpdateWindow( m_hwnd );
        
        MSG msg;
	    while( ::PeekMessage( &msg, m_hwnd, 0U, 0U, PM_REMOVE ) )
	    {
		    ::TranslateMessage( &msg );
		    ::DispatchMessage( &msg );
	    }
    }
    
}

/*-------------------------------------------------------------------------*/
                         
MT::TLockStatus
CMsWin32Window::Lock( UInt32 lockWaitTimeoutInMs ) const
{GUCEF_TRACE;
    
    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------*/
                         
MT::TLockStatus
CMsWin32Window::Unlock( void ) const
{GUCEF_TRACE;
    
    return MT::LOCKSTATUS_NOT_APPLICABLE;
}

/*-------------------------------------------------------------------------//
//                                                                         //
//      NAMESPACE                                                          //
//                                                                         //
//-------------------------------------------------------------------------*/

}; /* namespace CORE */
}; /* namespace GUCEF */

/*-------------------------------------------------------------------------*/

#endif /* GUCEF_MSWIN_BUILD ? */
